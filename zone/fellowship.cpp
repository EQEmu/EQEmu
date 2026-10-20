#include "common/eq_packet.h"
#include "common/eqemu_logsys.h"
#include "common/fellowship.h"
#include "common/rulesys.h"
#include "common/servertalk.h"
#include "common/strings.h"
#include "zone/client.h"
#include "zone/entity.h"
#include "zone/fellowship_manager.h"
#include "zone/position.h"
#include "zone/string_ids.h"
#include "zone/zone.h"
#include "zone/zonedb.h"

#include <cstring>
#include <ctime>
#include <iterator>
#include <string>

// TOB+ fellowships: the zone checks what only a zone can see, then asks world to do the work

namespace {
	// seconds left on the rejoin cooldown, 0 when there is none
	uint32 GetRejoinSecondsRemaining(Client *c)
	{
		if (c->GetBucket(Fellowship::REJOIN_BUCKET).empty()) {
			return 0;
		}

		const int remaining = Strings::ToInt(c->GetBucketRemaining(Fellowship::REJOIN_BUCKET));

		return remaining > 0 ? static_cast<uint32>(remaining) : 0;
	}

	// whole minutes, rounded up so "0 hours and 0 minutes" never shows
	void SplitCooldown(uint32 seconds, std::string &hours, std::string &minutes)
	{
		const uint32 total_minutes = (seconds + 59) / 60;

		hours   = std::to_string(total_minutes / 60);
		minutes = std::to_string(total_minutes % 60);
	}

	// true (and the message sent) when the character is still on the rejoin cooldown
	bool RefuseIfOnCooldown(Client *c)
	{
		const uint32 seconds = GetRejoinSecondsRemaining(c);
		if (!seconds) {
			return false;
		}

		std::string hours;
		std::string minutes;
		SplitCooldown(seconds, hours, minutes);

		c->MessageString(Chat::Red, FELLOWSHIP_SELF_COOLDOWN, hours.c_str(), minutes.c_str());
		LogFellowshipsDetail("[{}] is on the fellowship rejoin cooldown for [{}] seconds", c->GetCleanName(), seconds);

		return true;
	}

	void InviteMember(Client *c, ZoneFellowshipManager *manager, const FellowshipInfo *fellowship, uint32 target_character_id)
	{
		auto *target = entity_list.GetClientByCharID(target_character_id);
		if (!target || target == c) {
			LogFellowshipsDetail("[{}] invited character [{}], who is not another player in this zone", c->GetCleanName(), target_character_id);
			c->MessageString(Chat::Red, FELLOWSHIP_ADD_FAILED);
			return;
		}

		// only a TOB+ client can answer the prompt
		if (!target->IsFellowshipEnabled()) {
			LogFellowshipsDetail("[{}] invited [{}], whose client has no fellowships", c->GetCleanName(), target->GetCleanName());
			c->MessageString(Chat::Red, FELLOWSHIP_ADD_FAILED);
			return;
		}

		if (manager->GetFellowshipIDByCharacterID(target->CharacterID())) {
			LogFellowshipsDetail("[{}] invited [{}], who is already in a fellowship", c->GetCleanName(), target->GetCleanName());
			c->MessageString(Chat::Red, FELLOWSHIP_TARGET_IN_FELLOWSHIP);
			return;
		}

		if (fellowship->members.size() >= Fellowship::MAX_MEMBERS) {
			LogFellowshipsDetail("[{}] invited [{}] to a full fellowship [{}]", c->GetCleanName(), target->GetCleanName(), fellowship->id);
			c->MessageString(Chat::Red, FELLOWSHIP_FULL);
			return;
		}

		const uint32 seconds = GetRejoinSecondsRemaining(target);
		if (seconds) {
			std::string hours;
			std::string minutes;
			SplitCooldown(seconds, hours, minutes);

			LogFellowshipsDetail("[{}] invited [{}], who is on the rejoin cooldown for [{}] seconds", c->GetCleanName(), target->GetCleanName(), seconds);
			c->MessageString(Chat::Red, FELLOWSHIP_TARGET_COOLDOWN, target->GetCleanName(), hours.c_str(), minutes.c_str());
			return;
		}

		manager->RecordInvite(target->CharacterID(), fellowship->id);

		auto outapp = new EQApplicationPacket(OP_FellowshipInvite, sizeof(FellowshipInvite_Struct));
		auto out    = reinterpret_cast<FellowshipInvite_Struct *>(outapp->pBuffer);
		out->target_character_id  = target->CharacterID();
		out->inviter_character_id = c->CharacterID();
		out->fellowship_id        = fellowship->id;
		target->FastQueuePacket(&outapp);
	}

	// one campsite per fellowship, and enough members near the one placing it
	void CreateCampsite(
		Client *c,
		ZoneFellowshipManager *manager,
		const FellowshipInfo *fellowship,
		const FellowshipAction_Struct *in
	)
	{
		if (!zone) {
			return;
		}

		if (!Fellowship::FindCampfireType(in->campfire_type)) {
			LogFellowshipsDetail("[{}] asked for unknown campfire type [{}]", c->GetCleanName(), in->campfire_type);
			c->MessageString(Chat::Red, FELLOWSHIP_CAMPFIRE_FAILED);
			return;
		}

		if (fellowship->campsite) {
			LogFellowshipsDetail("[{}] already has a campsite", c->GetCleanName());
			c->MessageString(Chat::Red, FELLOWSHIP_CAMPFIRE_FAILED);
			return;
		}

		const float range_squared = Fellowship::CAMPSITE_RANGE * Fellowship::CAMPSITE_RANGE;
		uint32      nearby        = 0;
		for (const auto &[id, other] : entity_list.GetClientList()) {
			if (!other || other->GetFellowshipID() != fellowship->id || !other->IsFellowshipEnabled()) {
				continue;
			}

			if (DistanceSquared(other->GetPosition(), c->GetPosition()) <= range_squared) {
				nearby++;
			}
		}

		const int minimum = RuleI(Character, FellowshipCampsiteMinimumMembers);
		if (minimum > 0 && nearby < static_cast<uint32>(minimum)) {
			LogFellowships("[{}] has [{}] members nearby, [{}] needed for a campsite", c->GetCleanName(), nearby, minimum);
			c->MessageString(Chat::Red, FELLOWSHIP_NOT_ENOUGH_NEARBY);
			return;
		}

		ServerFellowshipAction_Struct out{};
		out.action        = FellowshipCreateCampsite;
		out.character_id  = c->CharacterID();
		out.fellowship_id = fellowship->id;
		out.campfire_type = in->campfire_type;
		out.zone_id       = static_cast<uint16>(zone->GetZoneID());
		out.instance_id   = static_cast<uint16>(zone->GetInstanceID());
		out.x             = c->GetX();
		out.y             = c->GetY();
		out.z             = c->GetZ();
		out.heading       = c->GetHeading();

		manager->SendAction(out);
	}
}

bool Client::IsFellowshipEnabled() const
{
	return RuleB(Character, EnableFellowships) && ClientVersion() >= EQ::versions::ClientVersion::TOB;
}

void Client::Handle_OP_FellowshipAction(const EQApplicationPacket *app)
{
	if (!IsFellowshipEnabled()) {
		LogFellowshipsDetail("Ignoring OP_FellowshipAction from [{}]: fellowships are off for this client", GetCleanName());
		return;
	}

	if (app->size != sizeof(FellowshipAction_Struct)) {
		LogNetcode("Received invalid sized OP_FellowshipAction: got [{}], expected [{}]", app->size, sizeof(FellowshipAction_Struct));
		return;
	}

	const auto in = reinterpret_cast<const FellowshipAction_Struct *>(app->pBuffer);
	if (in->action < FellowshipCreate || in->action > FellowshipToggleExpShare) {
		LogFellowshipsDetail("[{}] sent OP_FellowshipAction with unknown action [{}]", GetCleanName(), in->action);
		return;
	}

	auto *manager = ZoneFellowshipManager::Instance();

	const uint32         fellowship_id = manager->GetFellowshipIDByCharacterID(CharacterID());
	const FellowshipInfo *fellowship   = fellowship_id ? manager->GetFellowship(fellowship_id) : nullptr;
	const bool           is_leader     = fellowship && fellowship->leader_id == CharacterID();

	ServerFellowshipAction_Struct out{};
	out.action              = in->action;
	out.character_id        = CharacterID();
	out.fellowship_id       = fellowship_id;
	out.target_character_id = in->target_character_id;

	switch (in->action) {
		case FellowshipCreate: {
			if (fellowship) {
				LogFellowshipsDetail("[{}] tried to create a fellowship while in fellowship [{}]", GetCleanName(), fellowship_id);
				return;
			}

			if (RefuseIfOnCooldown(this)) {
				return;
			}

			manager->SendAction(out);
			return;
		}
		case FellowshipAcceptInvite: {
			// the invite is gone after this, whatever the outcome
			const uint32 invite_fellowship_id = manager->TakeInvite(CharacterID());
			if (!invite_fellowship_id) {
				return;
			}

			if (fellowship) {
				LogFellowshipsDetail("[{}] accepted an invite while in fellowship [{}]", GetCleanName(), fellowship_id);
				return;
			}

			if (!manager->GetFellowship(invite_fellowship_id)) {
				LogFellowshipsDetail("[{}] accepted an invite to fellowship [{}], which no longer exists", GetCleanName(), invite_fellowship_id);
				return;
			}

			if (RefuseIfOnCooldown(this)) {
				return;
			}

			out.fellowship_id = invite_fellowship_id;
			manager->SendAction(out);
			return;
		}
		case FellowshipDeclineInvite: {
			manager->ClearInvite(CharacterID());
			return;
		}
		default: {
			break;
		}
	}

	// everything else needs a fellowship
	if (!fellowship) {
		LogFellowshipsDetail("[{}] sent fellowship action [{}] without being in a fellowship", GetCleanName(), in->action);
		return;
	}

	switch (in->action) {
		case FellowshipSetMotd: {
			if (!is_leader) {
				LogFellowshipsDetail("[{}] set the MOTD without being the leader", GetCleanName());
				return;
			}

			if (strnlen(in->motd, sizeof(in->motd)) >= sizeof(in->motd)) {
				LogFellowshipsDetail("[{}] sent an unterminated MOTD", GetCleanName());
				return;
			}

			strn0cpy(out.motd, in->motd, sizeof(out.motd));
			manager->SendAction(out);
			return;
		}
		case FellowshipInvite: {
			InviteMember(this, manager, fellowship, in->target_character_id);
			return;
		}
		case FellowshipRemoveMember: {
			if (!is_leader || in->target_character_id == CharacterID() || !fellowship->FindMember(in->target_character_id)) {
				LogFellowshipsDetail(
					"[{}] removed character [{}] (leader [{}])",
					GetCleanName(),
					in->target_character_id,
					is_leader
				);
				return;
			}

			manager->SendAction(out);
			return;
		}
		case FellowshipPromoteLeader: {
			if (!is_leader || !fellowship->FindMember(in->target_character_id)) {
				LogFellowshipsDetail(
					"[{}] promoted character [{}] (leader [{}])",
					GetCleanName(),
					in->target_character_id,
					is_leader
				);
				return;
			}

			if (in->target_character_id == CharacterID()) {
				MessageString(Chat::Red, FELLOWSHIP_ALREADY_LEADER);
				return;
			}

			manager->SendAction(out);
			return;
		}
		case FellowshipCreateCampsite: {
			// any member may place it, counted around themselves
			CreateCampsite(this, manager, fellowship, in);
			return;
		}
		case FellowshipDestroyCampsite: {
			// any member may destroy it
			if (!fellowship->campsite) {
				LogFellowshipsDetail("[{}] destroyed a campsite, but fellowship [{}] has none", GetCleanName(), fellowship_id);
				MessageString(Chat::Red, FELLOWSHIP_NO_CAMPSITE);
				return;
			}

			manager->SendAction(out);
			return;
		}
		case FellowshipLeave: {
			manager->SendAction(out);
			return;
		}
		case FellowshipDisband: {
			if (!is_leader) {
				LogFellowshipsDetail("[{}] disbanded without being the leader", GetCleanName());
				return;
			}

			manager->SendAction(out);
			return;
		}
		case FellowshipToggleExpShare: {
			if (
				in->fellowship_id != fellowship_id ||
				!fellowship->FindMember(in->target_character_id) ||
				(!is_leader && in->target_character_id != CharacterID())
			) {
				LogFellowshipsDetail(
					"[{}] toggled XP sharing of character [{}] for fellowship [{}] (leader [{}])",
					GetCleanName(),
					in->target_character_id,
					in->fellowship_id,
					is_leader
				);
				return;
			}

			manager->SendAction(out);
			return;
		}
		default: {
			return;
		}
	}
}

void Client::Handle_OP_FellowshipCampfireListRequest(const EQApplicationPacket *app)
{
	if (!IsFellowshipEnabled()) {
		LogFellowshipsDetail("Ignoring OP_FellowshipCampfireListRequest from [{}]: fellowships are off for this client", GetCleanName());
		return;
	}

	if (app->size != sizeof(FellowshipCampfireListRequest_Struct)) {
		LogNetcode(
			"Received invalid sized OP_FellowshipCampfireListRequest: got [{}], expected [{}]",
			app->size,
			sizeof(FellowshipCampfireListRequest_Struct)
		);
		return;
	}

	// the client asks after every roster and from its Refresh button; Live refuses a second one within ~2 s
	if (m_fellowship_list_timer.Enabled() && !m_fellowship_list_timer.Check(false)) {
		LogFellowshipsDetail("[{}] asked for the campfire list too soon", GetCleanName());
		MessageString(Chat::Skills, ACTION_NOT_NOW_TRY_AGAIN);
		return;
	}

	m_fellowship_list_timer.Start(Fellowship::LIST_RATE_LIMIT_MS);

	// only_usable and category change nothing while there is one campfire that needs no kit
	const uint32 count = static_cast<uint32>(std::size(Fellowship::CAMPFIRE_TYPES));

	auto outapp = new EQApplicationPacket(
		OP_FellowshipCampfireList,
		sizeof(FellowshipCampfireList_Struct) + count * sizeof(FellowshipCampfireListEntry_Struct)
	);
	auto list    = reinterpret_cast<FellowshipCampfireList_Struct *>(outapp->pBuffer);
	auto entries = reinterpret_cast<FellowshipCampfireListEntry_Struct *>(outapp->pBuffer + sizeof(FellowshipCampfireList_Struct));

	list->count = count;
	for (uint32 i = 0; i < count; i++) {
		entries[i].is_usable      = 1;
		entries[i].campfire_type  = Fellowship::CAMPFIRE_TYPES[i].id;
		entries[i].description_id = Fellowship::CAMPFIRE_TYPES[i].description_id;
		strn0cpy(entries[i].name, Fellowship::CAMPFIRE_TYPES[i].name, sizeof(entries[i].name));
	}

	FastQueuePacket(&outapp);
}

// the Fellowship Registration Insignia: teleport to the campsite
void Client::UseFellowshipInsignia()
{
	// same gate as a spell click; the bar must come back
	if (IsStunned() || IsMezzed() || IsFeared() || IsCasting()) {
		SendSpellBarEnable(SPELL_FELLOWSHIP_CALLING);
		return;
	}

	auto *manager = ZoneFellowshipManager::Instance();

	const uint32         fellowship_id = manager->GetFellowshipIDByCharacterID(CharacterID());
	const FellowshipInfo *fellowship   = fellowship_id ? manager->GetFellowship(fellowship_id) : nullptr;
	if (!fellowship) {
		MessageString(Chat::Red, FELLOWSHIP_NOT_MEMBER);
		return;
	}

	if (!fellowship->campsite || fellowship->campsite->expires_at <= static_cast<uint32>(std::time(nullptr))) {
		MessageString(Chat::Red, FELLOWSHIP_NO_CAMPSITE);
		return;
	}

	const FellowshipCampsite campsite = *fellowship->campsite;

	// the one query on a user click: an instance that is gone takes its campsite with it
	if (campsite.instance_id && database.CheckInstanceExpired(campsite.instance_id)) {
		LogFellowshipsDetail("[{}] used the insignia but instance [{}] of fellowship [{}] has expired", GetCleanName(), campsite.instance_id, fellowship_id);
		MessageString(Chat::Red, FELLOWSHIP_CAMPSITE_UNREACHABLE);
		manager->SendCampsiteExpired(fellowship_id);
		return;
	}

	MovePC(campsite.zone_id, campsite.instance_id, campsite.x, campsite.y, campsite.z, campsite.heading, 0, ZoneSolicited);
}
