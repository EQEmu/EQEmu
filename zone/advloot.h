#ifndef ADVLOOT_H
#define ADVLOOT_H

#include <cstdint>
#include <map>
#include <string>

class Client;
class Corpse;

// ================================================================= AoTv4 group/raid shared loot
// The personal-loot half of Advanced Loot lives in client_packet.cpp (SendAdvLootData / AdvLootSlot /
// HandleAdvLootSay). This file owns only the GROUP decision layer, modelled on live:
//
//   Free Grab     -- first eligible looter to click it wins (this is also the FFA default)
//   Need/Greed    -- members mark ND / GD / NO; need outranks greed, random within the winning tier
//   Direct Award  -- the master looter hands it to a named player
//   Leave         -- master looter drops it from the shared list; it stays on the corpse
//
// EQEmu already decides WHO has loot rights: NPC::Death calls Corpse::AllowPlayerLoot for the killer,
// every group member, or the raid per RaidLootType, and Corpse::CanPlayerLoot gates it. We do not
// duplicate any of that -- this layer only decides, among people who already have rights, who may
// take a given item right now. Enforcement is a single check in Client::AdvLootSlot.
//
// State is per-zone and in memory: a corpse is a zone entity and shared decisions do not outlive it.

enum AdvLootMode : uint8_t {
	ADVLOOT_MODE_FFA       = 0,   // no group control; anyone with rights loots (EQEmu's native behaviour)
	ADVLOOT_MODE_MASTER    = 1,   // only the master looter may loot, or award to someone
	ADVLOOT_MODE_NEEDGREED = 2,   // ND/GD/NO roll decides
};

enum AdvLootVote : uint8_t {
	ADVLOOT_VOTE_NONE  = 0,
	ADVLOOT_VOTE_NEED  = 1,
	ADVLOOT_VOTE_GREED = 2,
	ADVLOOT_VOTE_PASS  = 3,
	ADVLOOT_VOTE_SELL  = 4,   // sell it and split the coin between everyone who chose sell
};

// How long a group roll stays open before it resolves itself.
static const uint32 ADVLOOT_ROLL_SECONDS = 30;

struct AdvLootShare {
	uint16                        corpse_id  = 0;
	uint16                        lootslot   = 0;
	uint32                        item_id    = 0;
	uint32                        group_id   = 0;   // group (or raid) whose members may vote
	bool                          free_grab  = false;
	uint32                        awarded_to = 0;   // character_id, 0 = undecided
	uint32                        roll_ends  = 0;   // unix time the roll auto-resolves (0 = not open)
	std::map<uint32, uint8_t>     votes;            // character_id -> AdvLootVote
};

class AdvLootManager {
public:
	// group loot mode (per group id; raids reuse the raid id space via MakeGroupKey)
	AdvLootMode GetMode(uint32 group_id) const;
	void        SetMode(uint32 group_id, AdvLootMode mode);

	// The group id that governs this client's loot decisions, or 0 when solo (solo always behaves FFA).
	static uint32 GroupKeyFor(Client *c);
	static bool   IsMasterLooter(Client *c);

	// Returns the share record for a corpse slot, creating it if the item is under group control.
	// nullptr means "not shared" -> personal loot rules apply unchanged.
	AdvLootShare *Get(uint16 corpse_id, uint16 lootslot);
	AdvLootShare *Ensure(Client *c, uint16 corpse_id, uint16 lootslot, uint32 item_id);

	// May this client take this item right now? Fills 'why' with a player-facing reason when false.
	bool CanLoot(Client *c, uint16 corpse_id, uint16 lootslot, uint32 item_id, std::string &why);

	void Vote(Client *c, uint16 corpse_id, uint16 lootslot, uint32 item_id, uint8_t vote);
	bool Resolve(Client *c, uint16 corpse_id, uint16 lootslot);            // run the roll now
	bool Award(Client *ml, uint16 corpse_id, uint16 lootslot, const char *player);
	bool SetFreeGrab(Client *ml, uint16 corpse_id, uint16 lootslot);

	// A short status string for the loot list ("Need/Greed", "Free Grab", "Won by Bob", "").
	std::string StatusText(Client *viewer, uint16 corpse_id, uint16 lootslot);
	uint8_t     VoteOf(Client *viewer, uint16 corpse_id, uint16 lootslot);

	// Called every zone tick: resolves any roll whose 30s timer has run out.
	void Process();

	// Apply a member's standing AN/AG/Sell rule as their opening vote and open the roll timer.
	// Solo (no group) has no roll at all -- AN and AG simply auto-loot, handled by the caller.
	void SeedRule(Client *c, uint16 corpse_id, uint16 lootslot, uint32 item_id, int rule);

	void ForgetCorpse(uint16 corpse_id);   // corpse looted out / decayed
	void ForgetSlot(uint16 corpse_id, uint16 lootslot);

	// Push a refreshed loot list to every member of the group that owns this share.
	void NotifyGroup(uint32 group_id);

private:
	static uint32 Key(uint16 corpse_id, uint16 lootslot) {
		return ((uint32) corpse_id << 16) | lootslot;
	}

	std::map<uint32, AdvLootShare> m_shares;
	std::map<uint32, AdvLootMode>  m_modes;
};

extern AdvLootManager advloot_manager;

#endif
