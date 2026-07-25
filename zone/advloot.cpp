#include "advloot.h"

#include "client.h"
#include "corpse.h"
#include "entity.h"
#include "groups.h"
#include "raids.h"
#include "zonedb.h"
#include "../common/data_bucket.h"
#include "../common/strings.h"
#include "../common/eqemu_logsys.h"   // pulls fmt/format.h, used by StatusText
#include "zone.h"                     // zone->random for the roll

#include <vector>
#include <ctime>   // roll deadlines

AdvLootManager advloot_manager;

// Raid ids and group ids come from different counters, so a raid key is offset to keep the two from
// colliding in m_modes / AdvLootShare::group_id.
static const uint32 ADVLOOT_RAID_KEY_BASE = 0x40000000;

AdvLootMode AdvLootManager::GetMode(uint32 group_id) const
{
	if (!group_id) {
		return ADVLOOT_MODE_FFA;   // solo is always free-for-all
	}
	const auto it = m_modes.find(group_id);
	return (it == m_modes.end()) ? ADVLOOT_MODE_FFA : it->second;
}

void AdvLootManager::SetMode(uint32 group_id, AdvLootMode mode)
{
	if (!group_id) {
		return;
	}
	m_modes[group_id] = mode;
}

uint32 AdvLootManager::GroupKeyFor(Client *c)
{
	if (!c) {
		return 0;
	}
	if (Raid *r = c->GetRaid()) {
		return ADVLOOT_RAID_KEY_BASE + r->GetID();
	}
	if (Group *g = c->GetGroup()) {
		return g->GetID();
	}
	return 0;
}

// The master looter. Raids already carry a looter concept (RaidLootType + per-member is_looter, which
// NPC::Death uses to assign rights), so mirror it rather than inventing a second rule. Groups have no
// native looter concept at all, so the group leader is the master looter.
bool AdvLootManager::IsMasterLooter(Client *c)
{
	if (!c) {
		return false;
	}
	if (Raid *r = c->GetRaid()) {
		const uint32 idx = r->GetPlayerIndex(c);
		if (idx < MAX_RAID_MEMBERS) {
			return r->members[idx].is_raid_leader || r->members[idx].is_looter;
		}
		return false;
	}
	if (Group *g = c->GetGroup()) {
		return g->IsLeader(c);
	}
	return true;   // solo: you are your own master looter
}

AdvLootShare *AdvLootManager::Get(uint16 corpse_id, uint16 lootslot)
{
	const auto it = m_shares.find(Key(corpse_id, lootslot));
	return (it == m_shares.end()) ? nullptr : &it->second;
}

AdvLootShare *AdvLootManager::Ensure(Client *c, uint16 corpse_id, uint16 lootslot, uint32 item_id)
{
	const uint32 gk = GroupKeyFor(c);
	if (!gk || GetMode(gk) == ADVLOOT_MODE_FFA) {
		return nullptr;   // not under group control
	}

	AdvLootShare &s = m_shares[Key(corpse_id, lootslot)];
	if (!s.corpse_id) {
		s.corpse_id = corpse_id;
		s.lootslot  = lootslot;
		s.item_id   = item_id;
		s.group_id  = gk;
	}
	return &s;
}

bool AdvLootManager::CanLoot(Client *c, uint16 corpse_id, uint16 lootslot, uint32 item_id, std::string &why)
{
	if (!c) {
		return false;
	}

	const uint32      gk   = GroupKeyFor(c);
	const AdvLootMode mode = GetMode(gk);
	if (mode == ADVLOOT_MODE_FFA) {
		return true;   // native behaviour: rights alone decide
	}

	AdvLootShare *s = Ensure(c, corpse_id, lootslot, item_id);
	if (!s) {
		return true;
	}

	if (s->free_grab) {
		return true;   // master looter released it to first-come-first-served
	}

	if (s->awarded_to) {
		if (s->awarded_to == c->CharacterID()) {
			return true;
		}
		why = "That item has already been awarded to someone else.";
		return false;
	}

	if (mode == ADVLOOT_MODE_MASTER) {
		if (IsMasterLooter(c)) {
			return true;
		}
		why = "Your group is on Master Looter; only the master looter can take that.";
		return false;
	}

	// Need/Greed: nobody takes it until the roll has produced a winner.
	why = "That item is still up for Need/Greed. Mark ND/GD/NO and wait for the roll.";
	return false;
}

void AdvLootManager::Vote(Client *c, uint16 corpse_id, uint16 lootslot, uint32 item_id, uint8_t vote)
{
	if (!c) {
		return;
	}
	AdvLootShare *s = Ensure(c, corpse_id, lootslot, item_id);
	if (!s || s->awarded_to) {
		return;
	}
	s->votes[c->CharacterID()] = vote;

	const char *label = (vote == ADVLOOT_VOTE_NEED)  ? "NEED"
	                  : (vote == ADVLOOT_VOTE_GREED) ? "GREED" : "PASS";
	const EQ::ItemData *d = database.GetItem(s->item_id);
	c->Message(Chat::Yellow, "You mark %s on %s.", label, d ? d->Name : "that item");

	NotifyGroup(s->group_id);
}

// Resolution order: NEED outranks GREED outranks SELL. Within the winning tier the item is rolled
// for at random. If nobody needs or greeds but someone chose Sell, the item is sold and the coin is
// split evenly between everyone who chose Sell. All passes (or no votes) leaves it on the corpse.
bool AdvLootManager::Resolve(Client *c, uint16 corpse_id, uint16 lootslot)
{
	AdvLootShare *s = Get(corpse_id, lootslot);
	if (!s || s->awarded_to) {
		return false;
	}
	s->roll_ends = 0;   // the roll is closing now, however it ends

	std::vector<uint32> need, greed, sell;
	for (const auto &v : s->votes) {
		if (v.second == ADVLOOT_VOTE_NEED)  { need.push_back(v.first);  }
		if (v.second == ADVLOOT_VOTE_GREED) { greed.push_back(v.first); }
		if (v.second == ADVLOOT_VOTE_SELL)  { sell.push_back(v.first);  }
	}

	const EQ::ItemData *d    = database.GetItem(s->item_id);
	const char         *iname = d ? d->Name : "the item";

	// Nobody wants to keep it but somebody wants it sold -> sell, and split the coin between EVERYONE
	// entitled to roll on it, not merely the people who voted sell. A 20-man raid divides by 20.
	//
	// The entitled set is the corpse's allowed-looter list, which NPC::Death already built from the
	// killer / group / RaidLootType. Using it means a raid on LeaderOnly correctly divides by one, and
	// we never have to re-derive who "the group" is.
	if (need.empty() && greed.empty() && !sell.empty()) {
		Corpse   *corpse = entity_list.GetCorpseByID(corpse_id);
		LootItem *li     = corpse ? corpse->GetItem(lootslot) : nullptr;
		if (corpse && li && d) {
			std::vector<uint32> entitled;
			for (int i = 0; i < MAX_LOOTERS; i++) {
				const int cid = corpse->GetAllowedLooter(i);
				if (cid) { entitled.push_back((uint32) cid); }
			}
			if (entitled.empty()) { entitled = sell; }   // unrestricted corpse: the sellers split it

			const uint64 total = Client::AdvLootSellValue(d, li->charges);
			const uint64 share = total / entitled.size();

			database.DeleteItemOffCharacterCorpse(corpse->GetCorpseDBID(), li->equip_slot, li->item_id);
			corpse->RemoveItem(lootslot);

			// Divide by the full entitled count even if some of them are offline or have zoned --
			// otherwise being away would silently inflate everyone else's cut.
			for (uint32 cid : entitled) {
				if (Client *sc = entity_list.GetClientByCharID(cid)) {
					sc->AddMoneyToPP(share, true);
					sc->Message(
						Chat::Yellow, "%s is sold for %s, split %d ways; your share is %s.",
						iname,
						Strings::Money(total / 1000, (total / 100) % 10,
						               (total / 10) % 10, total % 10).c_str(),
						(int) entitled.size(),
						Strings::Money(share / 1000, (share / 100) % 10,
						               (share / 10) % 10, share % 10).c_str()
					);
				}
			}
			const uint32 gid = s->group_id;
			ForgetSlot(corpse_id, lootslot);
			NotifyGroup(gid);
			return true;
		}
	}

	const std::vector<uint32> &pool = !need.empty() ? need : greed;
	if (pool.empty()) {
		if (c) {
			c->Message(Chat::Yellow, "Nobody wanted %s -- it stays on the corpse.", iname);
		}
		NotifyGroup(s->group_id);
		return false;
	}

	const uint32 winner = pool[zone->random.Int(0, (int) pool.size() - 1)];
	s->awarded_to       = winner;

	Client           *wc = entity_list.GetClientByCharID(winner);
	const std::string nm = wc ? wc->GetCleanName() : "someone";
	const char       *ty = need.empty() ? "greed" : "need";

	entity_list.MessageGroup(
		wc ? wc : c, true, Chat::Yellow,
		"%s wins %s on %s.", nm.c_str(), ty, iname
	);

	NotifyGroup(s->group_id);
	return true;
}

// Every zone tick: close out any roll whose timer has expired. Collected first because Resolve can
// erase the share (the sell path), which would invalidate the iterator.
void AdvLootManager::Process()
{
	if (m_shares.empty()) {
		return;
	}
	const uint32 now = (uint32) time(nullptr);

	std::vector<std::pair<uint16, uint16>> due;
	for (const auto &e : m_shares) {
		if (e.second.roll_ends && now >= e.second.roll_ends && !e.second.awarded_to) {
			due.emplace_back(e.second.corpse_id, e.second.lootslot);
		}
	}
	for (const auto &p : due) {
		Resolve(nullptr, p.first, p.second);
	}
}

// A member's standing rule becomes their opening vote the moment the item drops, and starts the
// clock. Anyone can still override it by clicking Need/Greed/Pass before the timer expires.
void AdvLootManager::SeedRule(Client *c, uint16 corpse_id, uint16 lootslot, uint32 item_id, int rule)
{
	if (!c || rule <= 0) {
		return;
	}
	AdvLootShare *s = Ensure(c, corpse_id, lootslot, item_id);
	if (!s || s->awarded_to) {
		return;
	}

	uint8_t vote = ADVLOOT_VOTE_NONE;
	if      (rule == 1) { vote = ADVLOOT_VOTE_NEED;  }   // Always Need
	else if (rule == 2) { vote = ADVLOOT_VOTE_GREED; }   // Always Greed
	else if (rule == 4) { vote = ADVLOOT_VOTE_SELL;  }   // Always Sell
	if (!vote) {
		return;
	}

	s->votes[c->CharacterID()] = vote;
	if (!s->roll_ends) {
		s->roll_ends = (uint32) time(nullptr) + ADVLOOT_ROLL_SECONDS;
	}
}

bool AdvLootManager::Award(Client *ml, uint16 corpse_id, uint16 lootslot, const char *player)
{
	if (!ml || !IsMasterLooter(ml)) {
		if (ml) {
			ml->Message(Chat::Red, "Only the master looter can award items.");
		}
		return false;
	}

	AdvLootShare *s = Get(corpse_id, lootslot);
	if (!s) {
		return false;
	}

	Client *target = entity_list.GetClientByName(player);
	if (!target) {
		ml->Message(Chat::Red, "Could not find %s in this zone.", player ? player : "that player");
		return false;
	}

	Corpse *corpse = entity_list.GetCorpseByID(corpse_id);
	if (corpse && !corpse->CanPlayerLoot(target->CharacterID())) {
		ml->Message(Chat::Red, "%s has no loot rights on that corpse.", target->GetCleanName());
		return false;
	}

	s->awarded_to = target->CharacterID();

	const EQ::ItemData *d = database.GetItem(s->item_id);
	ml->Message(Chat::Yellow, "You award %s to %s.", d ? d->Name : "the item", target->GetCleanName());
	target->Message(Chat::Yellow, "%s awards you %s. It is yours to loot.",
	                ml->GetCleanName(), d ? d->Name : "an item");

	NotifyGroup(s->group_id);
	return true;
}

bool AdvLootManager::SetFreeGrab(Client *ml, uint16 corpse_id, uint16 lootslot)
{
	if (!ml || !IsMasterLooter(ml)) {
		if (ml) {
			ml->Message(Chat::Red, "Only the master looter can do that.");
		}
		return false;
	}

	AdvLootShare *s = Get(corpse_id, lootslot);
	if (!s) {
		return false;
	}
	s->free_grab = true;

	const EQ::ItemData *d = database.GetItem(s->item_id);
	entity_list.MessageGroup(ml, true, Chat::Yellow,
		"%s releases %s -- first come, first served.", ml->GetCleanName(), d ? d->Name : "an item");

	NotifyGroup(s->group_id);
	return true;
}

uint8_t AdvLootManager::VoteOf(Client *viewer, uint16 corpse_id, uint16 lootslot)
{
	AdvLootShare *s = Get(corpse_id, lootslot);
	if (!s || !viewer) {
		return ADVLOOT_VOTE_NONE;
	}
	const auto it = s->votes.find(viewer->CharacterID());
	return (it == s->votes.end()) ? ADVLOOT_VOTE_NONE : it->second;
}

std::string AdvLootManager::StatusText(Client *viewer, uint16 corpse_id, uint16 lootslot)
{
	AdvLootShare *s = Get(corpse_id, lootslot);
	if (!s) {
		return "";
	}
	if (s->free_grab) {
		return "Free Grab";
	}
	if (s->awarded_to) {
		Client *w = entity_list.GetClientByCharID(s->awarded_to);
		if (viewer && s->awarded_to == viewer->CharacterID()) {
			return "Yours";
		}
		return fmt::format("Won by {}", w ? w->GetCleanName() : "another");
	}
	if (GetMode(s->group_id) == ADVLOOT_MODE_MASTER) {
		return "Master Looter";
	}
	if (s->roll_ends) {
		const uint32 now  = (uint32) time(nullptr);
		const int    left = (s->roll_ends > now) ? (int) (s->roll_ends - now) : 0;
		return fmt::format("Rolling {}s ({})", left, (int) s->votes.size());
	}
	return fmt::format("Rolling {}", (int) s->votes.size());
}

void AdvLootManager::ForgetSlot(uint16 corpse_id, uint16 lootslot)
{
	m_shares.erase(Key(corpse_id, lootslot));
}

void AdvLootManager::ForgetCorpse(uint16 corpse_id)
{
	for (auto it = m_shares.begin(); it != m_shares.end(); ) {
		it = (it->second.corpse_id == corpse_id) ? m_shares.erase(it) : std::next(it);
	}
}

// A shared decision changed, so every member's list is stale. Push to the whole group/raid.
void AdvLootManager::NotifyGroup(uint32 group_id)
{
	if (!group_id) {
		return;
	}

	if (group_id >= ADVLOOT_RAID_KEY_BASE) {
		Raid *r = entity_list.GetRaidByID(group_id - ADVLOOT_RAID_KEY_BASE);
		if (!r) {
			return;
		}
		for (const auto &m : r->members) {
			if (m.member && m.member->IsClient()) {
				m.member->CastToClient()->SendAdvLootData();
			}
		}
		return;
	}

	Group *g = entity_list.GetGroupByID(group_id);
	if (!g) {
		return;
	}
	for (auto *m : g->members) {
		if (m && m->IsClient()) {
			m->CastToClient()->SendAdvLootData();
		}
	}
}
