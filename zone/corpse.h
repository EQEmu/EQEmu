
#pragma once

#include "common/loot.h"
#include "common/repositories/character_corpses_repository.h"
#include "zone/client.h"
#include "zone/mob.h"

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
		uint16 spell_id,
		EQ::skills::SkillType attack_skill,
		KilledByTypes killed_by = KilledByTypes::Killed_NPC,
		bool is_buff_tic = false
	) { return true; }

	virtual void Damage(
		Mob *from,
		int64 damage,
		uint16 spell_id,
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
	void CastRezz(uint16 spell_id, Mob *caster);
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
