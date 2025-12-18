/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#pragma once

#include "common/repositories/lootdrop_entries_repository.h"
#include "common/repositories/lootdrop_repository.h"
#include "common/repositories/loottable_entries_repository.h"
#include "common/repositories/loottable_repository.h"
#include "common/repositories/npc_faction_entries_repository.h"
#include "common/rulesys.h"
#include "common/zone_store.h"
#include "zone/mob.h"
#include "zone/qglobals.h"
#include "zone/zonedb.h"
#include "zone/zonedump.h"

#include <deque>
#include <list>

typedef struct {
	float	min_x;
	float	max_x;
	float	min_y;
	float	max_y;
	float	min_z;
	float	max_z;
	bool	say;
	bool	proximity_set;
} NPCProximity;

struct AISpells_Struct {
	uint32	type;			// 0 = never, must be one (and only one) of the defined values
	uint16	spellid;		// <= 0 = no spell
	int16	manacost;		// -1 = use spdat, -2 = no cast time
	uint32	time_cancast;	// when we can cast this spell next
	int32	recast_delay;
	int16	priority;
	int16	resist_adjust;
	int8	min_hp; // >0 won't cast if HP is below
	int8	max_hp; // >0 won't cast if HP is above
};

struct AISpellsEffects_Struct {
	uint16	spelleffectid;
	int32	base_value;
	int32	limit;
	int32	max_value;
};

struct AISpellsVar_Struct {
	uint32	fail_recast;
	uint32	engaged_no_sp_recast_min;
	uint32	engaged_no_sp_recast_max;
	uint8	engaged_beneficial_self_chance;
	uint8	engaged_beneficial_other_chance;
	uint8	engaged_detrimental_chance;
	uint32	pursue_no_sp_recast_min;
	uint32	pursue_no_sp_recast_max;
	uint8	pursue_detrimental_chance;
	uint32	idle_no_sp_recast_min;
	uint32	idle_no_sp_recast_max;
	uint8	idle_beneficial_chance;
};

struct Roambox {
	float  max_x;
	float  max_y;
	float  min_x;
	float  min_y;
	float  distance;
	float  dest_x;
	float  dest_y;
	float  dest_z;
	uint32 delay;
	uint32 min_delay;
};

class SwarmPet;
class Client;
class Group;
class Raid;
class Spawn2;
class Aura;

namespace EQ
{
	struct ItemData;
}

class NPC : public Mob
{
public:
	static NPC* SpawnNPC(const char* spawncommand, const glm::vec4& position, Client* client = nullptr);
	static bool	SpawnZoneController();

	// loot recording / simulator
	bool IsRecordLootStats() const;
	void SetRecordLootStats(bool record_loot_stats);
	const std::vector<uint32> &GetRolledItems() const;
	int GetRolledItemCount(uint32 item_id);

	NPC(const NPCType* npc_type_data, Spawn2* respawn, const glm::vec4& position, GravityBehavior iflymode, bool IsCorpse = false);

	virtual ~NPC();

	static NPC *SpawnNodeNPC(std::string name, std::string last_name, const glm::vec4 &position);
	static void SpawnGridNodeNPC(const glm::vec4 &position, int32 grid_id, int32 grid_number, int32 zoffset);
	static NPC * SpawnZonePointNodeNPC(std::string name, const glm::vec4 &position);

	//abstract virtual function implementations requird by base abstract class
	virtual bool Death(Mob* killer_mob, int64 damage, uint16 spell_id, EQ::skills::SkillType attack_skill, KilledByTypes killed_by = KilledByTypes::Killed_NPC, bool is_buff_tic = false);
	virtual void Damage(Mob* from, int64 damage, uint16 spell_id, EQ::skills::SkillType attack_skill, bool avoidable = true, int8 buffslot = -1, bool iBuffTic = false, eSpecialAttacks special = eSpecialAttacks::None);
	bool Attack(Mob* other, int Hand = EQ::invslot::slotPrimary, bool FromRiposte = false, bool IsStrikethrough = false,
		bool IsFromSpell = false, ExtraAttackOptions *opts = nullptr) override;
	virtual bool HasRaid() { return false; }
	virtual bool HasGroup() { return false; }
	virtual Raid* GetRaid() { return 0; }
	virtual Group* GetGroup() { return 0; }

	virtual bool IsNPC() const { return true; }

	virtual bool Process();
	virtual void	AI_Init();
	virtual void	AI_Start(uint32 iMoveDelay = 0);
	virtual void	AI_Stop();
	void			AI_DoMovement();
	void			AI_SetupNextWaypoint();
	bool			AI_AddNPCSpells(uint32 iDBSpellsID);
	bool			AI_AddNPCSpellsEffects(uint32 iDBSpellsEffectsID);
	virtual bool	AI_EngagedCastCheck();
	bool			AI_HasSpells() { return HasAISpell; }
	bool			AI_HasSpellsEffects() { return HasAISpellEffects; }
	void			ApplyAISpellEffects(StatBonuses* newbon);

	virtual bool	AI_PursueCastCheck();
	virtual bool	AI_IdleCastCheck();
	virtual void	AI_Event_SpellCastFinished(bool iCastSucceeded, uint16 slot);

	bool AICheckCloseBeneficialSpells(NPC* caster, uint8 chance, float cast_range, uint32 spell_types);
	void AIYellForHelp(Mob* sender, Mob* attacker);

	void LevelScale();

	virtual void SetTarget(Mob* mob);
	virtual uint16 GetSkill(EQ::skills::SkillType skill_num) const { if (skill_num <= EQ::skills::HIGHEST_SKILL) { return skills[skill_num]; } return 0; }

	virtual void CalcBonuses();
	virtual int GetCurrentBuffSlots() const { return RuleI(Spells, MaxBuffSlotsNPC); }
	virtual int GetCurrentSongSlots() const { return RuleI(Spells, MaxSongSlotsNPC); }
	virtual int GetCurrentDiscSlots() const { return RuleI(Spells, MaxDiscSlotsNPC); }
	virtual int GetMaxBuffSlots() const { return RuleI(Spells, MaxBuffSlotsNPC); }
	virtual int GetMaxSongSlots() const { return RuleI(Spells, MaxSongSlotsNPC); }
	virtual int GetMaxDiscSlots() const { return RuleI(Spells, MaxDiscSlotsNPC); }
	virtual int GetMaxTotalSlots() const { return RuleI(Spells, MaxTotalSlotsNPC); }
	virtual int GetPetMaxTotalSlots() const { return RuleI(Spells, MaxTotalSlotsPET); }
	virtual void InitializeBuffSlots();
	virtual void UninitializeBuffSlots();

	virtual void	SetAttackTimer();
	virtual void	RangedAttack(Mob* other);
	virtual void	ThrowingAttack(Mob* other) { }
	int32 GetNumberOfAttacks() const { return attack_count; }
	void DoRangedAttackDmg(Mob* other, bool Launch = true, int16 damage_mod = 0, int16 chance_mod = 0, EQ::skills::SkillType skill = EQ::skills::SkillArchery, float speed = 4.0f, const char *IDFile = nullptr);
	bool	IsFactionListAlly(uint32 other_faction);
	bool	IsGuard();
	FACTION_VALUE CheckNPCFactionAlly(int32 other_faction);
	virtual FACTION_VALUE GetReverseFactionCon(Mob* iOther);

	void	GoToBind(uint8 bind_number = 0)	{ GMMove(m_SpawnPoint.x, m_SpawnPoint.y, m_SpawnPoint.z, m_SpawnPoint.w); }
	void	Gate(uint8 bind_number = 0);

	void	GetPetState(SpellBuff_Struct *buffs, uint32 *items, char *name);
	void	SetPetState(SpellBuff_Struct *buffs, uint32 *items);
	virtual void SpellProcess();
	virtual void FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho);

	// loot
	void AddItem(const EQ::ItemData *item, uint16 charges, bool equip_item = true);
	void AddItem(
		uint32 item_id,
		uint16 charges,
		bool equip_item = true,
		uint32 augment_one = 0,
		uint32 augment_two = 0,
		uint32 augment_three = 0,
		uint32 augment_four = 0,
		uint32 augment_five = 0,
		uint32 augment_six = 0
	);
	void AddLootTable();
	void AddLootTable(uint32 loottable_id, bool is_global = false);
	void AddLootDropTable(uint32 lootdrop_id, uint8 drop_limit, uint8 min_drop);
	void CheckGlobalLootTables();
	void RemoveItem(uint32 item_id, uint16 quantity = 0, uint16 slot = 0);
	void CheckTrivialMinMaxLevelDrop(Mob *killer);
	void ClearLootItems();
	inline const LootItems &GetLootItems() { return m_loot_items; }
	LootItem *GetItem(int slot_id);
	void AddLootCash(uint32 in_copper, uint32 in_silver, uint32 in_gold, uint32 in_platinum);
	void RemoveLootCash();
	void QueryLoot(Client *to, bool is_pet_query = false);
	bool HasItem(uint32 item_id);
	uint32 CountItem(uint32 item_id);
	uint32 GetLootItemIDBySlot(uint16 loot_slot);
	uint16 GetFirstLootSlotByItemID(uint32 item_id);
	std::vector<int> GetLootList();
	uint32 CountLoot();
	inline uint32 GetLoottableID() const { return m_loottable_id; }
	inline bool DropsGlobalLoot() const { return !m_skip_global_loot; }
	inline uint32 GetCopper() const { return m_loot_copper; }
	inline uint32 GetSilver() const { return m_loot_silver; }
	inline uint32 GetGold() const { return m_loot_gold; }
	inline uint32 GetPlatinum() const { return m_loot_platinum; }
	inline void SetCopper(uint32 amt) { m_loot_copper = amt; }
	inline void SetSilver(uint32 amt) { m_loot_silver = amt; }
	inline void SetGold(uint32 amt) { m_loot_gold = amt; }
	inline void SetPlatinum(uint32 amt) { m_loot_platinum = amt; }

	void DescribeAggro(Client *to_who, Mob *mob, bool verbose);
	virtual void UpdateEquipmentLight();
	virtual int64 CalcMaxMana();
	void SetGrid(int32 grid_){ grid=grid_; }
	void SetSpawnGroupId(uint32 sg2){ spawn_group_id =sg2; }
	void SetWaypointMax(uint16 wp_){ wp_m=wp_; }
	void SetSaveWaypoint(uint16 wp_){ save_wp=wp_; }

	uint16 GetWaypointMax() const { return wp_m; }
	int32 GetGrid() const { return grid; }
	Spawn2* GetSpawn() { return respawn2 ? respawn2 : nullptr; }
	uint32 GetSpawnGroupId() const { return spawn_group_id; }
	uint32 GetSpawnPointID() const;

	glm::vec4 const GetSpawnPoint() const { return m_SpawnPoint; }
	glm::vec4 const GetGuardPoint() const { return m_GuardPoint; }
	EmuAppearance GetGuardPointAnim() const { return guard_anim; }
	void SaveGuardPointAnim(EmuAppearance anim) { guard_anim = anim; }

	uint8 GetPrimSkill()	const { return prim_melee_type; }
	uint8 GetSecSkill()	const { return sec_melee_type; }
	uint8 GetRangedSkill() const { return ranged_type; }
	void SetPrimSkill(uint8 skill_type)	{ prim_melee_type = skill_type; }
	void SetSecSkill(uint8 skill_type)	{ sec_melee_type = skill_type; }
	void SetRangedSkill(uint8 skill_type)	{ ranged_type = skill_type; }

	uint32	MerchantType;
	bool	merchant_open;
	inline void	MerchantOpenShop() { merchant_open = true; }
	inline void	MerchantCloseShop() { merchant_open = false; }
	inline bool	IsMerchantOpen() { return merchant_open; }
	inline uint8 GetGreedPercent() { return NPCTypedata->greed; }
	inline bool GetParcelMerchant() { return NPCTypedata->is_parcel_merchant; }
	void	Depop(bool start_spawn_timer = false);
	void	Stun(int duration);
	void	UnStun();
	uint32	GetSwarmOwner();
	uint32	GetSwarmTarget();
	void	SetSwarmTarget(int target_id = 0);
	void	DepopSwarmPets();
	void	TryDepopTargetLockedPets(Mob* current_target);
	void	PetOnSpawn(NewSpawn_Struct* ns);

	void	SignalNPC(int _signal_id);
	void	SendPayload(int payload_id, std::string payload_value = std::string());

	inline int32 GetNPCFactionID() const
	{ return npc_faction_id; }

	inline int32 GetPrimaryFaction() const
	{ return primary_faction; }

	inline int32 GetFactionAmount() const
	{ return faction_amount; }

	int64 GetNPCHate(Mob *in_ent)
	{ return hate_list.GetEntHateAmount(in_ent); }

	bool IsOnHatelist(Mob *p)
	{ return hate_list.IsEntOnHateList(p); }

	void SetNPCFactionID(int32 in)
	{
		npc_faction_id = in;
		content_db.GetFactionIDsForNPC(npc_faction_id, &faction_list, &primary_faction);
	}

    glm::vec4 m_SpawnPoint;

	uint32	GetMaxDMG() const {return max_dmg;}
	uint32	GetMinDMG() const {return min_dmg;}
	int GetBaseDamage() const { return base_damage; }
	int GetMinDamage() const { return min_damage; }
	float GetSlowMitigation() const { return slow_mitigation; }
	float	GetAttackSpeed() const {return attack_speed;}
	int		GetAttackDelay() const {return attack_delay;}
	bool	IsAnimal() const { return(bodytype == BodyType::Animal); }
	uint16	GetPetSpellID() const {return pet_spell_id;}
	void	SetPetSpellID(uint16 amt) {pet_spell_id = amt;}
	uint32	GetMaxDamage(uint8 tlevel);
	void	SetTaunting(bool is_taunting);
	bool	IsTaunting() const { return taunting; }
	void	PickPocket(Client* thief);
	void	Disarm(Client* client, int chance);
	void	StartSwarmTimer(uint32 duration) { swarm_timer.Start(duration); }
	void	DisableSwarmTimer() { swarm_timer.Disable(); }

	void AddLootDrop(
		const EQ::ItemData *item2,
		LootdropEntriesRepository::LootdropEntries loot_drop,
		bool wear_change = false,
		uint32 augment_one = 0,
		uint32 augment_two = 0,
		uint32 augment_three = 0,
		uint32 augment_four = 0,
		uint32 augment_five = 0,
		uint32 augment_six = 0
	);

	bool MeetsLootDropLevelRequirements(LootdropEntriesRepository::LootdropEntries loot_drop, bool verbose=false);

	void CheckSignal();

	virtual void DoClassAttacks(Mob *target);
	inline bool IsNotTargetableWithHotkey() const { return no_target_hotkey; }
	int64 GetNPCHPRegen() const { return hp_regen + itembonuses.HPRegen + spellbonuses.HPRegen; }
	inline const char* GetAmmoIDfile() const { return ammo_idfile; }

	void ModifyStatsOnCharm(bool remove_charm, Mob* charmer);

	//waypoint crap
	int					GetMaxWp() const { return max_wp; }
	void				DisplayWaypointInfo(Client *client);
	void				CalculateNewWaypoint();
	void				AssignWaypoints(int32 grid_id, int start_wp = 0);
	void				SetWaypointPause();
	void				UpdateWaypoint(int wp_index);

	// quest wandering commands
	void				StopWandering();
	void				ResumeWandering();
	void				PauseWandering(int pausetime);
	void				MoveTo(const glm::vec4& position, bool saveguardspot);
	void				GetClosestWaypoints(std::list<wplist> &wp_list, int count, const glm::vec3& location);
	int					GetClosestWaypoint(const glm::vec3& location);

	uint32				GetEquippedItemFromTextureSlot(uint8 material_slot) const;	// returns item id
	uint32				GetEquipmentMaterial(uint8 material_slot) const;

	void				NextGuardPosition();
	void				SaveGuardSpot(bool ClearGuardSpot = false);
	void				SaveGuardSpot(const glm::vec4 &pos);
	inline bool			IsGuarding() const { return(m_GuardPoint.w != 0); }
	void				SaveGuardSpotCharm();

	void DescribeSpecialAbilities(Client* c);

	uint16 GetMeleeTexture1() const;
	uint16 GetMeleeTexture2() const;

	void RestoreGuardSpotCharm();

	void AI_SetRoambox(
		float max_distance,
		float roam_distance_variance,
		uint32 delay = 2500,
		uint32 min_delay = 2500
	);

	void				AI_SetRoambox(float distance, float max_x, float min_x, float max_y, float min_y, uint32 delay = 2500, uint32 min_delay = 2500);

	//mercenary stuff
	void	LoadMercenaryTypes();
	void	LoadMercenaries();
	std::list<MercType> GetMercenaryTypesList() {return mercTypeList; };
	std::list<MercType> GetMercenaryTypesList( uint32 expansion );
	std::list<MercData> GetMercenariesList() {return mercDataList; };
	std::list<MercData> GetMercenariesList( uint32 expansion );
	int		GetNumMercenaryTypes() { return static_cast<int>(mercTypeList.size()); };
	int		GetNumMercenaryTypes( uint32 expansion );
	int		GetNumberOfMercenaries() { return static_cast<int>(mercDataList.size()); };
	int		GetNumberOfMercenaries( uint32 expansion );

	inline bool GetNPCAggro() const { return npc_aggro; }
	inline void SetNPCAggro(bool in_npc_aggro) { npc_aggro = in_npc_aggro; }

	inline void GiveNPCTypeData(NPCType *ours) { NPCTypedata_ours = ours; }
	inline const uint32 GetNPCSpellsID()	const { return npc_spells_id; }
	inline const uint32 GetNPCSpellsEffectsID()	const { return npc_spells_effects_id; }

	float GetProximityMinX();
	float GetProximityMaxX();
	float GetProximityMinY();
	float GetProximityMaxY();
	float GetProximityMinZ();
	float GetProximityMaxZ();
	bool  IsProximitySet();

	NPCProximity* proximity;
	Spawn2*	respawn2;
	QGlobalCache *GetQGlobals() { return qGlobals; }
	QGlobalCache *CreateQGlobals() { qGlobals = new QGlobalCache(); return qGlobals; }

	SwarmPet *GetSwarmInfo() { return (swarmInfoPtr); }
	void SetSwarmInfo(SwarmPet *mSwarmInfo) { swarmInfoPtr = mSwarmInfo; }

	int32	GetAccuracyRating() const { return (accuracy_rating); }
	void	SetAccuracyRating(int32 d) { accuracy_rating = d;}
	int32	GetAvoidanceRating() const { return (avoidance_rating); }
	void	SetAvoidanceRating(int32 d) { avoidance_rating = d;}
	int32 GetRawAC() const { return AC; }

	float	GetNPCStat(const std::string& stat);
	void	ModifyNPCStat(const std::string& stat, const std::string& value);
	virtual void SetLevel(uint8 in_level, bool command = false);

	bool IsLDoNTrapped() const { return ldon_trapped; }
	void SetLDoNTrapped(bool n) { ldon_trapped = n; }

	uint8 GetLDoNTrapType() const { return ldon_trap_type; }
	void SetLDoNTrapType(uint8 n) { ldon_trap_type = n; }

	uint16 GetLDoNTrapSpellID() const { return ldon_spell_id; }
	void SetLDoNTrapSpellID(uint16 n) { ldon_spell_id = n; }

	bool IsLDoNLocked() const { return ldon_locked; }
	void SetLDoNLocked(bool n) { ldon_locked = n; }

	uint16 GetLDoNLockedSkill() const { return ldon_locked_skill; }
	void SetLDoNLockedSkill(uint16 n) { ldon_locked_skill = n; }

	bool IsLDoNTrapDetected() const { return ldon_trap_detected; }
	void SetLDoNTrapDetected(bool n) { ldon_trap_detected = n; }

	const bool GetCombatEvent() const { return combat_event; }
	void SetCombatEvent(bool b) { combat_event = b; }

	/* Only allows players that killed corpse to loot */
	const bool HasPrivateCorpse() const { return NPCTypedata_ours ? NPCTypedata_ours->private_corpse : NPCTypedata->private_corpse; }

	virtual const bool IsUnderwaterOnly() const { return m_is_underwater_only; }
	virtual const bool IsQuestNPC() const { return m_is_quest_npc; }
	const char* GetRawNPCTypeName() const { return NPCTypedata_ours ? NPCTypedata_ours->name : NPCTypedata->name; }

	virtual int GetKillExpMod() const { return NPCTypedata_ours ? NPCTypedata_ours->exp_mod : NPCTypedata->exp_mod; }

	void ChangeLastName(std::string last_name);
	void ClearLastName();

	bool GetDepop() { return p_depop; }

	void NPCSlotTexture(uint8 slot, uint32 texture);	// Sets new material values for slots

	uint32 GetAdventureTemplate() const { return adventure_template_id; }
	void AddSpellToNPCList(int16 iPriority, uint16 iSpellID, uint32 iType, int16 iManaCost, int32 iRecastDelay, int16 iResistAdjust, int8 min_hp, int8 max_hp);
	void AddSpellEffectToNPCList(uint16 iSpellEffectID, int32 base_value, int32 limit, int32 max_value, bool apply_bonus = false);
	void RemoveSpellFromNPCList(uint16 spell_id);
	void RemoveSpellEffectFromNPCList(uint16 iSpellEffectID, bool apply_bonus = false);
	bool HasAISpellEffect(uint16 spell_effect_id);
	Timer *GetRefaceTimer() const { return reface_timer; }
	const uint32 GetAltCurrencyType() const { return NPCTypedata->alt_currency_type; }

	NPC_Emote_Struct* GetNPCEmote(uint32 emote_id, uint8 event_);
	void DoNPCEmote(uint8 event_, uint32 emote_id, Mob* t = nullptr);
	bool CanTalk();
	void DoQuestPause(Mob* m);
	bool FacesTarget();

	inline void SetSpellScale(float amt)		{ spellscale = amt; }
	inline float GetSpellScale()				{ return spellscale; }

	inline void SetHealScale(float amt)		{ healscale = amt; }
	inline float GetHealScale()					{ return healscale; }

	inline void SetSpellFocusDMG(int32 NewSpellFocusDMG) {SpellFocusDMG = NewSpellFocusDMG;}
	inline int32 GetSpellFocusDMG() const { return SpellFocusDMG;}

	inline void SetSpellFocusHeal(int32 NewSpellFocusHeal) {SpellFocusHeal = NewSpellFocusHeal;}
	inline int32 GetSpellFocusHeal() const {return SpellFocusHeal;}

	uint32	GetSpawnKillCount();
	int	GetScore();
	void	AISpellsList(Client *c);
	uint16 GetInnateProcSpellID() const { return innate_proc_spell_id; }

	uint32	GetHeroForgeModel() const { return herosforgemodel; }
	void	SetHeroForgeModel(uint32 model) { herosforgemodel = model; }

	bool IsRaidTarget() const { return raid_target; };
	bool IsRareSpawn() const { return rare_spawn; }
	void ResetHPUpdateTimer() { send_hp_update_timer.Start(); }

	bool IgnoreDespawn() { return ignore_despawn; }

	void SetSimpleRoamBox(float box_size, float move_distance = 0, int move_delay = 0);

	float GetRoamboxMaxX() const;
	float GetRoamboxMaxY() const;
	float GetRoamboxMinX() const;
	float GetRoamboxMinY() const;
	float GetRoamboxDistance() const;
	float GetRoamboxDestinationX() const;
	float GetRoamboxDestinationY() const;
	float GetRoamboxDestinationZ() const;
	uint32 GetRoamboxDelay() const;
	uint32 GetRoamboxMinDelay() const;

	inline uint8 GetArmTexture() { return armtexture; }
	inline uint8 GetBracerTexture() { return bracertexture; }
	inline uint8 GetHandTexture() { return handtexture; }
	inline uint8 GetFeetTexture() { return feettexture; }
	inline uint8 GetLegTexture() { return legtexture; }

	inline int GetCharmedAccuracy() { return charm_accuracy_rating; }
	inline int GetCharmedArmorClass() { return charm_ac; }
	inline int GetCharmedAttack() { return charm_atk; }
	inline int GetCharmedAttackDelay() { return charm_attack_delay; }
	inline int GetCharmedAvoidance() { return charm_avoidance_rating; }
	inline int GetCharmedMaxDamage() { return charm_max_dmg; }
	inline int GetCharmedMinDamage() { return charm_min_dmg; }

	inline bool GetAlwaysAggro() { return always_aggro; }
	inline bool GetNPCAggro() { return npc_aggro; }
	inline bool GetIgnoreDespawn() { return ignore_despawn; }
	inline bool GetSkipGlobalLoot() { return m_skip_global_loot; }

	std::unique_ptr<Timer> AIautocastspell_timer;

	virtual int GetStuckBehavior() const { return NPCTypedata_ours ? NPCTypedata_ours->stuck_behavior : NPCTypedata->stuck_behavior; }

	inline bool IsSkipAutoScale() const { return m_skip_auto_scale; }

	void ScaleNPC(uint8 npc_level, bool always_scale = false, bool override_special_abilities = false);

	uint32 GetNPCTintIndex() { return m_npc_tint_id; }
	void SetNPCTintIndex(uint32 index);

	void RecalculateSkills();
	void ReloadSpells();

	void SendPositionToClients();

	bool CanPathTo(float x, float y, float z);

	void DoNpcToNpcAggroScan();

	// hand-ins
	bool CanPetTakeItem(const EQ::ItemInstance *inst);

	struct HandinEntry {
		std::string      item_id            = "0";
		uint32           count              = 0;
		EQ::ItemInstance *item              = nullptr;
		bool             is_multiquest_item = false; // state
	};

	struct HandinMoney {
		uint32 platinum = 0;
		uint32 gold     = 0;
		uint32 silver   = 0;
		uint32 copper   = 0;
	};

	struct Handin {
		std::vector<HandinEntry> original_items = {}; // this is what the player originally handed in, never modified
		std::vector<HandinEntry> items          = {}; // items can be removed from this set as successful handins are made
		HandinMoney              original_money = {}; // this is what the player originally handed in, never modified
		HandinMoney              money          = {}; // money can be removed from this set as successful handins are made
	};

	// NPC Hand-in
	bool IsMultiQuestEnabled() { return m_multiquest_enabled; }
	void MultiQuestEnable() { m_multiquest_enabled = true; }
	bool IsGuildmasterForClient(Client *c);
	bool CheckHandin(
		Client *c,
		std::map<std::string, uint32> handin,
		std::map<std::string, uint32> required,
		std::vector<EQ::ItemInstance *> items
	);
	Handin ReturnHandinItems(Client *c);
	void ResetHandin();
	void ResetMultiQuest();
	bool HasProcessedHandinReturn() { return m_has_processed_handin_return; }
	bool HandinStarted() { return m_handin_started; }

	// zone state save
	inline void SetQueuedToCorpse() { m_queued_for_corpse = true; }
	inline bool IsQueuedForCorpse() const { return m_queued_for_corpse; }
	inline void SetResumedFromZoneSuspend(bool state = true) { m_resumed_from_zone_suspend = state; }
	inline bool IsResumedFromZoneSuspend() const { return m_resumed_from_zone_suspend; }

	inline void LoadBuffsFromState(std::vector<Buffs_Struct> in_buffs) {
		int i = 0;
		for (auto &b: in_buffs) {
			buffs[i].spellid     = b.spellid;
			buffs[i].casterlevel = b.casterlevel;
			buffs[i].casterid    = b.casterid;
			strncpy(buffs[i].caster_name, b.caster_name, 64);
			buffs[i].ticsremaining     = b.ticsremaining;
			buffs[i].counters          = b.counters;
			buffs[i].hit_number        = b.hit_number;
			buffs[i].melee_rune        = b.melee_rune;
			buffs[i].magic_rune        = b.magic_rune;
			buffs[i].dot_rune          = b.dot_rune;
			buffs[i].caston_x          = b.caston_x;
			buffs[i].caston_y          = b.caston_y;
			buffs[i].caston_z          = b.caston_z;
			buffs[i].ExtraDIChance     = b.ExtraDIChance;
			buffs[i].RootBreakChance   = b.RootBreakChance;
			buffs[i].instrument_mod    = b.instrument_mod;
			buffs[i].virus_spread_time = b.virus_spread_time;
			buffs[i].persistant_buff   = b.persistant_buff;
			buffs[i].client            = b.client;
			buffs[i].UpdateClient      = b.UpdateClient;
			i++;
		}
		CalcBonuses();
	}

protected:

	void HandleRoambox();

	const NPCType*	NPCTypedata;
	NPCType*	NPCTypedata_ours;	//special case for npcs with uniquely created data.

	friend class EntityList;
	friend class Aura;

	int32  grid;
	uint32 spawn_group_id;
	uint16 wp_m;

	// loot
	uint32    m_loot_copper;
	uint32    m_loot_silver;
	uint32    m_loot_gold;
	uint32    m_loot_platinum;
	LootItems m_loot_items;

	// zone state
	bool m_resumed_from_zone_suspend = false;
	bool m_queued_for_corpse         = false; // this is to check for corpse creation on zone state restore

	// this is a timer that protects a NPC from having double assignment of loot
	// this is to prevent a player from killing a NPC and then zoning out and back in to get loot again
	// if loot was to be assigned via script again, this protects double assignment for a short time
	Timer m_resumed_from_zone_suspend_shutoff_timer = {};

	std::list<NpcFactionEntriesRepository::NpcFactionEntries> faction_list;

	int32	npc_faction_id;
	int32	primary_faction;
	int32	faction_amount;

	Timer	attacked_timer;		//running while we are being attacked (damaged)
	Timer	swarm_timer;
	Timer	monkattack_timer;	//additional timer for tiger claw usage
	Timer	classattack_timer;
	Timer	knightattack_timer;
	Timer	assist_timer;		//ask for help from nearby mobs
	Timer	qglobal_purge_timer;

	bool	combat_event;	//true if we are in combat, false otherwise
	Timer	send_hp_update_timer;
	Timer	enraged_timer;
	Timer *reface_timer;

	uint32	npc_spells_id;
	uint8	casting_spell_AIindex;

	std::vector<AISpells_Struct> AIspells;
	bool HasAISpell;
	virtual bool AICastSpell(Mob* tar, uint8 iChance, uint32 iSpellTypes, bool bInnates = false);
	virtual bool AIDoSpellCast(int32 i, Mob* tar, int32 mana_cost, uint32* oDontDoAgainBefore = 0);
	AISpellsVar_Struct AISpellVar;
	int64 GetFocusEffect(focusType type, uint16 spell_id, Mob *caster = nullptr, bool from_buff_tic = false) override;
	uint16 innate_proc_spell_id;

	uint32	npc_spells_effects_id;
	std::vector<AISpellsEffects_Struct> AIspellsEffects;
	bool HasAISpellEffects;

	uint32	max_dmg;
	uint32	min_dmg;
	int		base_damage;
	int		min_damage;
	int32	accuracy_rating;
	int32	avoidance_rating;
	int16	attack_count;
	uint64	npc_mana;
	float	spellscale;
	float	healscale;
	int32 SpellFocusDMG;
	int32 SpellFocusHeal;

	// stats to switch back to after charm wears off
	// could probably pick a better name, but these probably aren't taken so ...
	int default_ac;
	int default_min_dmg;
	int default_max_dmg;
	int default_attack_delay;
	int default_accuracy_rating;
	int default_avoidance_rating;
	int default_atk;
	char default_special_abilities[512];

	// when charmed, switch to these
	int charm_ac;
	int charm_min_dmg;
	int charm_max_dmg;
	int charm_attack_delay;
	int charm_accuracy_rating;
	int charm_avoidance_rating;
	int charm_atk;

	//pet crap:
	uint16	pet_spell_id;
	bool	taunting;
	Timer	taunt_timer;		//for pet taunting

	bool npc_aggro;

	std::deque<int> signal_q;

	//waypoint crap:
	std::vector<wplist> Waypoints;
	int max_wp;
	int save_wp;
	glm::vec4 m_GuardPoint;
	glm::vec4 m_GuardPointSaved;
	EmuAppearance guard_anim;

	Roambox m_roambox = {};

	uint16	skills[EQ::skills::HIGHEST_SKILL + 1];

	uint32	equipment[EQ::invslot::EQUIPMENT_COUNT];	//this is an array of item IDs

	uint32	herosforgemodel;			//this is the Hero Forge Armor Model (i.e 63 or 84 or 203)
	uint32	d_melee_texture1;
	//this is an item Material value
	uint32	d_melee_texture2;			//this is an item Material value (offhand)
	const char*	ammo_idfile;			//this determines projectile graphic "IT###" (see item field 'idfile')
	uint8	prim_melee_type;			//Sets the Primary Weapon attack message and animation
	uint8	sec_melee_type;				//Sets the Secondary Weapon attack message and animation
	uint8   ranged_type;				//Sets the Ranged Weapon attack message and animation

	SwarmPet *swarmInfoPtr;

	bool ldon_trapped;
	uint8 ldon_trap_type;
	uint16 ldon_spell_id;
	bool ldon_locked;
	uint16 ldon_locked_skill;
	bool ldon_trap_detected;
	QGlobalCache *qGlobals;
	uint32 adventure_template_id;

	bool m_is_underwater_only = false;
	bool m_is_quest_npc = false;

	//mercenary stuff
	std::list<MercType> mercTypeList;
	std::list<MercData> mercDataList;

	bool raid_target;
	bool ignore_despawn; //NPCs with this set to 1 will ignore the despawn value in spawngroup

	// NPC Hand-in
	bool m_multiquest_enabled          = false;
	bool m_handin_started              = false;
	bool m_has_processed_handin_return = false;

	// this is the working handin data from the player
	// items can be decremented from this as each successful
	// check is ran in scripts, the remainder is what is returned
	Handin m_hand_in = {};
public:
	const Handin GetHandin() { return m_hand_in; }

private:
	uint32              m_loottable_id;
	bool                m_skip_global_loot;
	bool                m_skip_auto_scale;
	bool                p_depop;
	bool                m_record_loot_stats;
	std::vector<uint32> m_rolled_items = {};
};
