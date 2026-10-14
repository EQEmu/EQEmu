/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "common/loot.h"
#include "common/repositories/character_corpses_repository.h"
#include "zone/client.h"
#include "zone/mob.h"

#include <map>
#include <set>
#include <vector>

class EQApplicationPacket;
class Group;
class NPC;
class Raid;

struct ExtraAttackOptions;
struct NPCType;

namespace EQ {
	class ItemInstance;
}

#define MAX_LOOTERS 72

// OP_AdvLoot subtypes (u16, first in both directions)
enum AdvLootSubtype : uint16 {
	AdvLootSync         = 3,
	AdvLootLeave        = 7,
	AdvLootLoot         = 8,
	AdvLootLootResult   = 9,
	AdvLootAsk          = 0xa,
	AdvLootChoice       = 0xb,
	AdvLootGive         = 0xe,
	AdvLootClearPending = 0x10,
	AdvLootState        = 0x11,
	AdvLootRoll         = 0x12,
	AdvLootRollResult   = 0x13,
};

enum AdvLootResult : uint16 {
	AdvLootResultLooted = 1,
	AdvLootResultLore   = 3, // client prints 371
	AdvLootResultNoRoom = 5, // client prints 372
	AdvLootResultGone   = 7, // client prints 11318
};

// OP_AdvLootMisc subtypes (client to server)
enum AdvLootMiscSubtype : uint16 {
	AdvLootMiscFreeGrab = 0xf,
};

void SendAdvLootResult(Client *c, uint16 result, uint32 sequence);

class Corpse : public Mob {
public:

	static void SendEndLootErrorPacket(Client *client);
	static void SendLootReqErrorPacket(Client *client, LootResponse response = LootResponse::NotAtThisTime);

	Corpse(
		NPC *npc,
		LootItems *item_list,
		uint32 npc_type_id,
		const NPCType **npc_type_data,
		uint32 decay_time = 600000
	);
	Corpse(Client *c, int32 rez_exp, KilledByTypes killed_by = KilledByTypes::Killed_NPC);
	Corpse(
		uint32 corpse_id,
		uint32 character_id,
		const char *character_name,
		LootItems *item_list,
		uint32 copper,
		uint32 silver,
		uint32 gold,
		uint32 platinum,
		const glm::vec4 &position,
		float size,
		uint8 gender,
		uint16 race,
		uint8 class_,
		uint8 deity,
		uint8 level,
		uint8 texture,
		uint8 helm_texture,
		uint32 rez_exp,
		uint32 gm_rez_exp,
		KilledByTypes killed_by,
		bool is_rezzable,
		uint32 rez_remaining_time,
		bool was_at_graveyard = false
	);

	~Corpse();

	/* Corpse: General */
	virtual bool Death(
		Mob *killer_mob,
		int64 damage,
		int32 spell_id,
		EQ::skills::SkillType attack_skill,
		KilledByTypes killed_by = KilledByTypes::Killed_NPC,
		bool is_buff_tic = false
	) { return true; }

	virtual void Damage(
		Mob *from,
		int64 damage,
		int32 spell_id,
		EQ::skills::SkillType attack_skill,
		bool avoidable = true,
		int8 buffslot = -1,
		bool iBuffTic = false,
		eSpecialAttacks special = eSpecialAttacks::None
	) { }

	bool Attack(
		Mob *other, int Hand = EQ::invslot::slotPrimary, bool FromRiposte = false, bool IsStrikethrough = true,
		bool IsFromSpell = false, ExtraAttackOptions *opts = nullptr
	) override
	{
		return false;
	}

	virtual bool HasRaid() { return false; }
	virtual bool HasGroup() { return false; }
	virtual Raid *GetRaid() { return nullptr; }
	virtual Group *GetGroup() { return nullptr; }
	inline uint32 GetCorpseDBID() { return m_corpse_db_id; }
	inline char *GetOwnerName() { return corpse_name; }
	bool IsEmpty() const;
	bool IsCorpse() const { return true; }
	bool IsPlayerCorpse() const { return m_is_player_corpse; }
	bool IsNPCCorpse() const { return !m_is_player_corpse; }
	bool IsBecomeNPCCorpse() const { return m_become_npc; }
	virtual void DepopNPCCorpse();
	virtual void DepopPlayerCorpse();
	bool Process();
	bool Save();

	uint32 GetCharID() { return m_character_id; }

	uint32 SetCharID(uint32 iCharID)
	{
		if (IsPlayerCorpse()) {
			return (m_character_id = iCharID);
		}
		return 0xFFFFFFFF;
	};

	uint32 GetDecayTime()
	{
		if (!m_corpse_decay_timer.Enabled()) {
			return 0xFFFFFFFF;
		}
		else {
			return m_corpse_decay_timer.GetRemainingTime();
		}
	}
	uint32 GetRezTime()
	{
		if (!m_corpse_rezzable_timer.Enabled()) {
			return 0;
		}
		else {
			return m_corpse_rezzable_timer.GetRemainingTime();
		}
	}

	void ResetDecayTimer();
	void SetDecayTimer(uint32 decay_time);
	void SetConsentGroupID(uint32 group_id) { if (IsPlayerCorpse()) { m_consented_group_id = group_id; }}
	void SetConsentRaidID(uint32 raid_id) { if (IsPlayerCorpse()) { m_consented_raid_id = raid_id; }}
	void SetConsentGuildID(uint32 guild_id) { if (IsPlayerCorpse()) { m_consented_guild_id = guild_id; }}
	void AddConsentName(const std::string& consent_player_name);
	void RemoveConsentName(const std::string& consent_player_name);
	void SendWorldSpawnPlayerCorpseInZone(uint32_t zone_id);
	bool MovePlayerCorpseToGraveyard();
	bool MovePlayerCorpseToNonInstance();

	void Delete();
	void Bury();
	void CalcCorpseName();
	void LoadPlayerCorpseDecayTime(uint32 dbid);

	/* Corpse: Items */
	uint32 GetWornItem(int16 equip_slot) const;
	LootItem *GetItem(uint16 lootslot, LootItem **bag_item_data = 0);
	void SetPlayerKillItemID(int32 pk_item_id) { m_player_kill_item = pk_item_id; }
	int32 GetPlayerKillItem() { return m_player_kill_item; }
	void RemoveItem(uint16 lootslot);
	void RemoveItem(LootItem *item_data);
	void RemoveItemByID(uint32 item_id, int quantity = 1);
	void AddItem(
		uint32 itemnum,
		uint16 charges,
		int16 slot = 0,
		uint32 aug1 = 0,
		uint32 aug2 = 0,
		uint32 aug3 = 0,
		uint32 aug4 = 0,
		uint32 aug5 = 0,
		uint32 aug6 = 0,
		bool attuned = false,
		const std::string &custom_data = std::string(),
		uint32 ornamenticon = 0,
		uint32 ornamentidfile = 0,
		uint32 ornament_hero_model = 0
	);

	/* Corpse: Coin */
	void SetCash(uint32 in_copper, uint32 in_silver, uint32 in_gold, uint32 in_platinum);
	void RemoveCash();
	uint32 GetCopper() { return m_copper; }
	uint32 GetSilver() { return m_silver; }
	uint32 GetGold() { return m_gold; }
	uint32 GetPlatinum() { return m_platinum; }

	/* Corpse: Resurrection */
	bool IsRezzed() { return m_rez; }
	void IsRezzed(bool in_rez) { m_rez = in_rez; }
	void CastRezz(int32 spell_id, Mob *caster);
	void CompleteResurrection(bool timer_expired = false);
	bool IsRezzable() { return m_is_rezzable; }
	void SetRezTimer(bool initial_timer = false);

	/* Corpse: Loot */
	void QueryLoot(Client *to);
	bool HasItem(uint32 item_id);
	uint32 CountItem(uint32 item_id);
	uint32 GetItemIDBySlot(uint16 loot_slot);
	uint16 GetFirstLootSlotByItemID(uint32 item_id);
	std::vector<int> GetLootList();
	inline const LootItems &GetLootItems() { return m_item_list; }
	void LootCorpseItem(Client *c, const EQApplicationPacket *app);
	void EndLoot(Client *c, const EQApplicationPacket *app);
	void MakeLootRequestPackets(Client *c, const EQApplicationPacket *app);
	void AllowPlayerLoot(Mob *them, uint8 slot);
	void AddLooter(Mob *who);
	uint32 CountItems();
	bool CanPlayerLoot(int character_id);

	/* Corpse: Advanced Loot (TOB+; zone/corpse_advloot.cpp) */
	void StartAdvancedLoot(Client *killer);
	bool IsAdvancedLootFor(Client *c);
	bool HasAdvancedLootItems();
	bool HasAdvancedLootItemsFor(Client *c);
	bool HasAdvancedLootTransaction(uint32 transaction_id);
	void SendAdvancedLootItems(Client *c);
	void AdvancedLootItem(Client *c, uint32 item_id, uint32 sequence);
	void AdvancedLeaveItem(Client *c, uint32 item_id);
	void AdvancedGiveItem(Client *c, uint32 transaction_id, uint32 item_id, uint16 quantity, bool leave, const std::string &to_name);
	void AdvancedChoice(Client *c, uint32 transaction_id, uint32 item_id, uint32 choice);
	void AdvancedAsk(Client *c, uint32 transaction_id, uint32 item_id, uint32 state);
	void AdvancedSetState(Client *c, uint32 transaction_id, uint32 state);
	void AdvancedRoll(Client *c, uint32 transaction_id);
	void AdvancedFreeGrab(Client *c, uint32 transaction_id);
	void EndAdvancedLoot();
	static void SendAdvancedLootSync(Client *c);
	static void AnnounceAdvancedLootMasterLooter(Client *to, const std::string &name);
	static Corpse *FindAdvancedLootTransaction(uint32 transaction_id, Client *master_looter, Client *recipient = nullptr);

	inline void Lock() { m_is_locked = true; }
	inline void UnLock() { m_is_locked = false; }
	inline bool IsLocked() { return m_is_locked; }
	inline void ResetLooter()
	{
		m_being_looted_by_entity_id = 0xFFFFFFFF;
		m_loot_request_type         = LootRequestType::Forbidden;
	}
	inline bool IsBeingLooted() { return (m_being_looted_by_entity_id != 0xFFFFFFFF); }
	inline bool IsBeingLootedBy(Client *c) { return m_being_looted_by_entity_id == c->GetID(); }

	/* Mob */
	void FillSpawnStruct(NewSpawn_Struct *ns, Mob *ForWho);
	bool Summon(Client *c, bool spell, bool CheckDistance);
	void Spawn();

	char corpse_name[64];
	uint32 GetEquippedItemFromTextureSlot(uint8 material_slot) const;
	uint32 GetEquipmentColor(uint8 material_slot) const;
	inline int64 GetRezExp() { return m_rezzed_experience; }
	inline int64 GetGMRezExp() { return m_gm_rezzed_experience; }
	uint8 GetKilledBy() { return m_killed_by_type; }
	uint32 GetRemainingRezTime() { return m_remaining_rez_time; }

	virtual void UpdateEquipmentLight();

	void CheckIsOwnerOnline();
	void SetOwnerOnline(bool value) { m_is_owner_online = value; }
	bool GetOwnerOnline() { return m_is_owner_online; }

	static Corpse *LoadCharacterCorpse(
		const CharacterCorpsesRepository::CharacterCorpses &cc,
		const glm::vec4 &position
	);

	void SyncEntityVariablesToCorpseDB();

protected:
	void MoveItemToCorpse(Client *client, EQ::ItemInstance *inst, int16 equipSlot, std::list<uint32> &removedList);

private:
	bool HasLootLoreConflict(Client *c, EQ::ItemInstance *inst);
	bool LootPreventedByEvents(Client *c, EQ::ItemInstance *inst);
	void GiveLootToClient(
		Client *c,
		EQ::ItemInstance *inst,
		const EQ::ItemData *item,
		LootItem *item_data,
		LootItem **bag_item_data,
		bool auto_loot,
		bool remove_by_pointer = false
	);
	struct AdvLootTransaction {
		const LootItem *item = nullptr;
		uint32 item_id  = 0;
		uint32 state    = 0;
		Timer  timer;
	};

	LootItem *FindAdvancedLootItem(uint32 item_id, uint32 owner_character_id, bool skip_asked = false);
	uint32 GetAdvancedLootOwner(const LootItem *item);
	void GetAdvancedLootBagContents(LootItem *item, LootItem **bag_item_data);
	bool IsAdvancedLootItemListed(const LootItem *item);
	bool IsAdvancedLootItemInAsk(const LootItem *item);
	uint32 DropAdvancedLootAsks(const LootItem *item);
	Client *GetAdvancedLootMasterLooter();
	std::vector<Client *> GetAdvancedLootRecipients();
	void QueueAdvancedLootRecipients(const EQApplicationPacket *app);
	void SendAdvancedLootTransaction(Client *c, uint32 transaction_id, AdvLootTransaction &t);
	void SendAdvancedLootState(uint32 transaction_id, AdvLootTransaction &t);
	void SendAdvancedLootStateTo(Client *c, uint32 transaction_id, AdvLootTransaction &t);
	void ResolveAdvancedLootRoll(uint32 transaction_id);
	void ProcessAdvancedLoot();
	void CheckAdvancedLootDone();
	void ForgetAdvancedLootItem(const LootItem *item);
	bool IsAdvancedLootPresent(Client *c);
	void MarkAdvancedLootPresent(Client *c);

	std::vector<uint32>                    m_advloot_recipients;         // characters with this corpse in their window
	uint32                                 m_advloot_group_id = 0;       // shared group loot
	uint32                                 m_advloot_raid_id  = 0;       // shared raid loot; both 0 = one personal owner
	bool IsAdvancedLootShared() const { return m_advloot_group_id || m_advloot_raid_id; }
	bool m_private_corpse = false; // NPC private_corpse: looter rights last until decay
	// keyed by LootItem pointer: NPC corpse loot slots are only assigned by classic loot sessions
	std::map<const LootItem *, uint32>     m_advloot_owner;              // item -> character it is reserved for
	std::set<const LootItem *>             m_advloot_left;               // items left on the corpse
	std::map<const LootItem *, std::map<uint32, uint32>> m_advloot_choices; // item -> character -> need/greed/no
	std::map<uint32, AdvLootTransaction>   m_advloot_transactions;       // master looter asks by transaction id
	std::map<uint32, std::string>          m_advloot_names;              // recipient character -> name (rolls while out of zone)
	std::set<uint32>                       m_advloot_present;            // recipients who were near the kill or visited the corpse

	bool                     m_is_player_corpse;
	bool                     m_is_corpse_changed;
	bool                     m_is_locked;
	int32                    m_player_kill_item;
	uint32                   m_corpse_db_id;
	uint32                   m_character_id;
	uint32                   m_consented_group_id = 0;
	uint32                   m_consented_raid_id  = 0;
	uint32                   m_consented_guild_id = 0;
	LootItems                m_item_list;
	uint32                   m_copper;
	uint32                   m_silver;
	uint32                   m_gold;
	uint32                   m_platinum;
	bool                     m_player_corpse_depop;
	uint32                   m_being_looted_by_entity_id;
	uint64                   m_rezzed_experience;
	uint64                   m_gm_rezzed_experience;
	uint64                   m_gm_exp;
	bool                     m_rez;
	bool                     m_become_npc;
	int                      m_allowed_looters[MAX_LOOTERS];
	Timer                    m_corpse_decay_timer;
	Timer                    m_corpse_rezzable_timer;
	Timer                    m_corpse_delay_timer;
	Timer                    m_corpse_graveyard_timer;
	Timer                    m_loot_cooldown_timer;
	Timer                    m_check_owner_online_timer;
	Timer                    m_check_rezzable_timer;
	uint8                    m_killed_by_type;
	bool                     m_is_rezzable;
	EQ::TintProfile          m_item_tint;
	uint32                   m_remaining_rez_time;
	bool                     m_is_owner_online;
	std::vector<std::string> m_consented_player_names;
	LootRequestType          m_loot_request_type;
	uint32                   m_account_id;
};
