//
// Created by dannu on 4/21/2026.
//

#pragma once


#include "common/emu_versions.h"
#include "common/patches/client_version.h"
#include "common/patches/IBuff.h"
#include "common/patches/IMessage.h"

#include "zone/client.h"
#include "zone/mob.h"

// store all _generic_ static functions for the different patches here
namespace ClientPatch {

using ClientList = std::unordered_map<uint16, Client*>;
template<typename Obj> using ComponentGetter = std::function<Obj*(const Client*)>;
using SendPredicate = std::function<bool(Client*)>;
using MutatePacket = std::function<void(std::unique_ptr<EQApplicationPacket>&, Client*)>;

template <typename Fun, typename Obj, typename... Args>
	requires std::is_member_function_pointer_v<Fun>
static void QueuePacket(Client* c, Fun fun, Obj* obj, Args&&... args)
{
	if (obj != nullptr) {
		std::unique_ptr<EQApplicationPacket> app = std::invoke(fun, obj, std::forward<Args>(args)...);
		if (app)
			c->QueuePacket(app.get());
	}
}

// packet generator queue functions
static auto QueueClients(Mob* sender, bool ignore_sender = false, bool ackreq = true)
{
	return [=]<typename Fun, typename Obj, typename... Args>(Fun fun, const ComponentGetter<Obj>& component, Args&&... args)
			requires std::is_member_function_pointer_v<Fun>
	{
		std::array<std::unique_ptr<EQApplicationPacket>, EQ::versions::ClientVersionCount> build_packets;

		for (auto [_, ent] : entity_list.GetClientList()) {
			if (!ignore_sender || ent != sender) {
				auto& packet = build_packets.at(static_cast<uint32_t>(ent->ClientVersion()));
				if (!packet)
					if (auto comp = component(ent); comp != nullptr)
						packet = std::invoke(fun, comp, std::forward<Args>(args)...);

				if (packet)
					ent->QueuePacket(packet.get(), ackreq, Client::CLIENT_CONNECTED);
			}
		}
	};
}

static auto QueueCloseClients(
	Mob* sender, bool ignore_sender = false, float distance = 200,
	Mob* skipped_mob = nullptr, bool is_ack_required = true,
	eqFilterType filter = FilterNone)
{
	if (distance <= 0) distance = static_cast<float>(zone->GetClientUpdateRange());

	return [=]<typename Fun, typename Obj, typename... Args>(Fun fun, const ComponentGetter<Obj>& component, Args&&... args)
			requires std::is_member_function_pointer_v<Fun>
	{
		if (sender == nullptr) {
			QueueClients(sender, ignore_sender, is_ack_required)(fun, component, std::forward<Args>(args)...);
		} else {
			float distance_squared = distance * distance;
			std::array<std::unique_ptr<EQApplicationPacket>, EQ::versions::ClientVersionCount> build_packets;

			for (auto& [_, mob] : sender->GetCloseMobList(distance)) {
				if (mob && mob->IsClient()) {
					Client* client = mob->CastToClient();
					if ((!ignore_sender || client != sender)
						&& client != skipped_mob
						&& DistanceSquared(client->GetPosition(), sender->GetPosition()) < distance_squared
						&& client->Connected()
						&& client->ShouldGetPacket(sender, filter))
					{
						auto& packet = build_packets.at(static_cast<uint32_t>(client->ClientVersion()));
						if (!packet)
							if (auto comp = component(client); comp != nullptr)
								packet = std::invoke(fun, comp, std::forward<Args>(args)...);

						if (packet)
							client->QueuePacket(packet.get(), is_ack_required, Client::CLIENT_CONNECTED);
					}
				}
			}
		}
	};
}

template <typename Fun, typename Obj, typename... Args>
static void FastQueuePacket(Client* c, Fun fun, Obj* obj, Args&&... args)
	requires std::is_member_function_pointer_v<Fun>
{
	if (obj != nullptr) {
		std::unique_ptr<EQApplicationPacket> app = std::invoke(fun, obj, std::forward<Args>(args)...);
		if (app) {
			// FastQueuePacket specifically takes lifetime management of packet, so release here
			EQApplicationPacket* packet = app.release();
			c->FastQueuePacket(&packet);
		}
	}
}

static auto QueueClientsByTarget(Mob* sender, bool ackreq, bool HoTT, const SendPredicate& should_send, const MutatePacket& mutate)
{
	return [=]<typename Fun, typename Obj, typename... Args>(Fun fun, const ComponentGetter<Obj> component, Args&&... args)
			requires std::is_member_function_pointer_v<Fun>
	{
		if (sender != nullptr) {
			std::array<std::unique_ptr<EQApplicationPacket>, EQ::versions::ClientVersionCount> build_packets;

			for (auto [_, c] : entity_list.GetClientList()) {
				Mob* Target = c->GetTarget();
				if ((Target == sender || (HoTT && Target != nullptr && Target->GetTarget() == sender)) &&
					(Target == c || should_send(c))) {
					auto& packet = build_packets.at(static_cast<uint32_t>(c->ClientVersion()));
					if (!packet)
						if (auto comp = component(c); comp != nullptr)
							packet = std::invoke(fun, comp, std::forward<Args>(args)...);

					mutate(packet, c);

					if (packet)
						c->QueuePacket(packet.get(), ackreq, Client::CLIENT_CONNECTED);
				}
			}
		}
	};
}

} // namespace ClientPatch

// Helpers for the Message interface to send message packets
namespace Message {

// this can return nullptr when the component doesn't exist for the version
static std::function GetComponent = [](const Client* c) -> IMessage* {
	return GetMessageComponent(c->GetClientVersion()).get();
};

// Helper functions to wrap the packet construction in sends
template <AllConstChar... Args>
	requires (sizeof...(Args) <= 9)
void MessageString(Client* c, uint32_t type, uint32_t id, Args&&... args)
{
	if constexpr (sizeof...(Args) == 0) {
		ClientPatch::QueuePacket(c, &IMessage::Simple, GetComponent(c), type, id);
	} else {
		std::array<const char*, 9> a = {args...};
		ClientPatch::QueuePacket(c, &IMessage::Formatted, GetComponent(c), type, id, a);
	}
}

static auto CloseMessageString(
	Mob* sender, bool ignore_sender = false, float distance = 200.f,
	Mob* skipped_mob = nullptr, bool is_ack_required = true,
	eqFilterType filter = FilterNone)
{
	return [=]<AllConstChar... Args>(uint32_t type, uint32_t id, Args&&... args)
			requires (sizeof...(Args) <= 9)
	{
		auto queue_close_clients = ClientPatch::QueueCloseClients(sender, ignore_sender, distance, skipped_mob,
			is_ack_required, filter);

		if constexpr (sizeof...(Args) == 0) {
			return queue_close_clients(&IMessage::Simple, GetComponent, type, id);
		} else {
			std::array<const char*, 9> a = {args...};
			return queue_close_clients(&IMessage::Formatted, GetComponent, type, id, a);
		}
	};
}

inline void InterruptSpell(Client* c, uint32_t message, uint32_t spawn_id, const char* spell_link)
{
	ClientPatch::QueuePacket(c, &IMessage::InterruptSpell, GetComponent(c), message, spawn_id, spell_link);
}

inline void InterruptSpellOther(Mob* sender, uint32_t message, uint32_t spawn_id, const char* name,
	const char* spell_link)
{
	ClientPatch::QueueCloseClients(sender, true, RuleI(Range, SongMessages), nullptr, true,
		sender->IsClient() ? FilterPCSpells : FilterNPCSpells)(
		&IMessage::InterruptSpellOther, GetComponent, sender, message, spawn_id, name, spell_link);
}

} // namespace Message

// helper functions to handle sending buffs
namespace Buff {

static std::function GetComponent = [](const Client* c) -> IBuff* {
	return GetBuffComponent(c->GetClientVersion()).get();
};

static bool ShouldSendTargetBuffs(Client* c)
{
	// this function checks for server rules against LAA and GM status to determine if a buffs packet should be sent
	// to a client (c) for targeted mobs
	if (c->GetGM() || RuleB(Spells, AlwaysSendTargetsBuffs)) { // this rule bypasses LAA abilities, always return true
		if (c->GetGM()) {
			if (!c->EntityVariableExists(SEE_BUFFS_FLAG)) { // This flag just ensures that the following message is only sent once
				c->Message(Chat::White,
					"Your GM flag allows you to always see your targets' buffs.");
				c->SetEntityVariable(SEE_BUFFS_FLAG, "1");
			}
		}
		return true;
	}

	if (c->IsRaidGrouped()) {
		Raid* raid = c->GetRaid();
		if (raid) {
			uint32 gid = raid->GetGroup(c);
			if (gid < MAX_RAID_GROUPS && raid->GroupCount(gid) >= 3) {
				if (raid->GetLeadershipAA(groupAAInspectBuffs, gid))
					return true;
			}
		}
	} else {
		Group* group = c->GetGroup();
		if (group && group->GroupCount() >= 3) {
			if (group->GetLeadershipAA(groupAAInspectBuffs)) {
				return true;
			}
		}
	}

	return false;
}

inline void SendFullBuffRefresh(Mob* sender, bool remove = false, bool ackreq = true)
{
	bool suspended = zone->BuffTimersSuspended();
	std::vector<uint32_t> slots;

	// first, send to self if self is client
	if (sender->IsClient()) {
		Client* c = sender->CastToClient();
		ClientPatch::FastQueuePacket(c, &IBuff::RefreshBuffs, GetComponent(c), OP_RefreshBuffs, sender, false, suspended, slots);
	}

	// next, send to owner if self is a pet to a client
	if (sender->IsPet() && sender->GetOwner()->IsClient()) {
		if (Mob* owner = sender->GetOwner(); owner != nullptr && owner->IsClient()) {
			Client* c = owner->CastToClient();
			ClientPatch::FastQueuePacket(c, &IBuff::RefreshBuffs, GetComponent(c), OP_RefreshPetBuffs, sender, false, suspended, slots);
		}
	}

	// finally send to all clients targeting the mob, will need to mutate the packet to set the type
	auto mutate = [sender](std::unique_ptr<EQApplicationPacket>& packet, Client* c) {
		GetComponent(c)->SetRefreshType(packet, sender, c);
	};

	ClientPatch::QueueClientsByTarget(sender, ackreq, false, ShouldSendTargetBuffs, mutate)(
		&IBuff::RefreshBuffs, GetComponent, OP_RefreshTargetBuffs, sender, false, suspended, slots);

	// if we have remove set, this will clear  any target windows that shouldn't see the buffs
	if (remove)
		ClientPatch::QueueClientsByTarget(sender, ackreq, true,
			[](Client* c) { return !ShouldSendTargetBuffs(c); }, mutate)(
			&IBuff::RefreshBuffs, GetComponent, OP_RefreshTargetBuffs, sender, true, suspended, slots);
}

inline void SendSingleBuffChange(Mob* sender, const Buffs_Struct& buff, int slot, bool remove = false, bool ackreq = true)
{
	bool suspended = zone->BuffTimersSuspended();
	std::vector slots = { static_cast<uint32_t>(slot) };

	// first, send to self if self is client, which takes the definition and the refresh
	if (sender->IsClient()) {
		Client* c = sender->CastToClient();
		ClientPatch::FastQueuePacket(c, &IBuff::BuffDefinition, GetComponent(c), sender, buff,  slot, remove);
		ClientPatch::FastQueuePacket(c, &IBuff::RefreshBuffs, GetComponent(c), OP_RefreshBuffs, sender, remove, suspended, slots);
	}

	// the rest of the buff packets do not take the definition, only the refresh
	if (sender->IsPet() && sender->GetOwner()->IsClient()) {
		if (Mob* owner = sender->GetOwner(); owner != nullptr && owner->IsClient()) {
			Client* c = owner->CastToClient();
			ClientPatch::FastQueuePacket(c, &IBuff::RefreshBuffs, GetComponent(c), OP_RefreshPetBuffs, sender, remove, suspended, slots);
		}
	}

	auto mutate = [sender](std::unique_ptr<EQApplicationPacket>& packet, Client* c) {
		GetComponent(c)->SetRefreshType(packet, sender, c);
	};

	ClientPatch::QueueClientsByTarget(sender, ackreq, false, ShouldSendTargetBuffs, mutate)(
		&IBuff::RefreshBuffs, GetComponent, OP_RefreshTargetBuffs, sender, remove, suspended, slots);

	// the client doesn't automatically do this for some reason (RoF2? I think this is in TOB)
	// TODO: hook this up to QueueClients, or figure out if there's another missing packet to display the fade text
	if (remove && sender->IsClient())
	{
		const char *fadetext = spells[buff.spellid].spell_fades;
		auto outapp = std::make_unique<EQApplicationPacket>(OP_ColoredText, sizeof(ColoredText_Struct) +  strlen(fadetext));
		ColoredText_Struct *bfm = (ColoredText_Struct *) outapp->pBuffer;
		bfm->color = Chat::Spells;
		memcpy(bfm->msg, fadetext, strlen(fadetext));
		sender->CastToClient()->QueuePacket(outapp.get());
	}
}

} // namespace Buff
