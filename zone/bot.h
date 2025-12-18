/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.org)

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

#include "common/misc_functions.h"
#include "common/zone_store.h"
#include "zone/bot_structs.h"
#include "zone/client.h"
#include "zone/corpse.h"
#include "zone/groups.h"
#include "zone/guild_mgr.h"
#include "zone/heal_rotation.h"
#include "zone/mob.h"
#include "zone/pets.h"
#include "zone/raids.h"
#include "zone/string_ids.h"
#include "zone/worldserver.h"
#include "zone/zonedb.h"

constexpr uint32 BOT_KEEP_ALIVE_INTERVAL = 5000; // 5 seconds

constexpr uint32 MAG_EPIC_1_0 = 28034;

extern WorldServer worldserver;

constexpr int NegativeItemReuse = -1; // Unlinked timer for items

constexpr uint8 SumWater               = 1;
constexpr uint8 SumFire                = 2;
constexpr uint8 SumAir                 = 3;
constexpr uint8 SumEarth               = 4;
constexpr uint8 MonsterSum             = 5;
constexpr uint8 SumMageMultiElement    = 6;

// nHSND	negative Healer/Slower/Nuker/Doter
// pH		positive Healer
// pS		positive Slower
// pHS		positive Healer/Slower
// pN		positive Nuker
// pHN		positive Healer/Nuker
// pSN		positive Slower/Nuker
// pHSN		positive Healer/Slower/Nuker
// pD		positive Doter
// pHD		positive Healer/Doter
// pSD		positive Slower/Doter
// pHSD		positive Healer/Slower/Doter
// pND		positive Nuker/Doter
// pHND		positive Healer/Nuker/Doter
// pSND		positive Slower/Nuker/Doter
// pHSND	positive Healer/Slower/Nuker/Doter
// cntHSND	count Healer/Slower/Nuker/Doter
enum BotCastingChanceConditional : uint8
{
	nHSND = 0,
	pH,
	pS,
	pHS,
	pN,
	pHN,
	pSN,
	pHSN,
	pD,
	pHD,
	pSD,
	pHSD,
	pND,
	pHND,
	pSND,
	pHSND,
	cntHSND = 16
};

namespace BotSettingCategories {
	constexpr uint8 BaseSetting                       = 0;
	constexpr uint8 SpellHold                         = 1;
	constexpr uint8 SpellDelay                        = 2;
	constexpr uint8 SpellMinThreshold                 = 3;
	constexpr uint8 SpellMaxThreshold                 = 4;
	constexpr uint8 SpellTypeResistLimit              = 5;
	constexpr uint8 SpellTypeAggroCheck               = 6;
	constexpr uint8 SpellTypeMinManaPct               = 7;
	constexpr uint8 SpellTypeMaxManaPct               = 8;
	constexpr uint8 SpellTypeMinHPPct                 = 9;
	constexpr uint8 SpellTypeMaxHPPct                 = 10;	
	constexpr uint8 SpellTypeIdlePriority             = 11;
	constexpr uint8 SpellTypeEngagedPriority          = 12;
	constexpr uint8 SpellTypePursuePriority           = 13;
	constexpr uint8 SpellTypeAEOrGroupTargetCount     = 14;
	constexpr uint8 SpellTypeAnnounceCast             = 15;

	constexpr uint16 START                            = BotSettingCategories::BaseSetting;
	constexpr uint16 START_NO_BASE                    = BotSettingCategories::SpellHold;
	constexpr uint16 START_CLIENT                     = BotSettingCategories::SpellDelay;
	constexpr uint16 END_CLIENT                       = BotSettingCategories::SpellMaxThreshold;
	constexpr uint16 END                              = BotSettingCategories::SpellTypeAnnounceCast;
};

static std::map<uint8, std::string> bot_setting_category_names = {
	{ BotSettingCategories::BaseSetting,                   "Base Setting" },
	{ BotSettingCategories::SpellHold,                     "Spell Holds" },
	{ BotSettingCategories::SpellDelay,                    "Spell Delays" },
	{ BotSettingCategories::SpellMinThreshold,             "Spell Minimum Thresholds" },
	{ BotSettingCategories::SpellMaxThreshold,             "Spell Maximum Thresholds" },
	{ BotSettingCategories::SpellTypeResistLimit,          "Spell Resist Limits" },
	{ BotSettingCategories::SpellTypeAggroCheck,           "Spell Aggro Checks" },
	{ BotSettingCategories::SpellTypeMinManaPct,           "Spell Min Mana Percent" },
	{ BotSettingCategories::SpellTypeMaxManaPct,           "Spell Max Mana Percent" },
	{ BotSettingCategories::SpellTypeMinHPPct,             "Spell Min HP Percent" },
	{ BotSettingCategories::SpellTypeMaxHPPct,             "Spell Max HP Percent" },
	{ BotSettingCategories::SpellTypeIdlePriority,         "Spell Idle Priority" },
	{ BotSettingCategories::SpellTypeEngagedPriority,      "Spell Engaged Priority" },
	{ BotSettingCategories::SpellTypePursuePriority,       "Spell Pursue Priority" },
	{ BotSettingCategories::SpellTypeAEOrGroupTargetCount, "Spell Target Counts" },
	{ BotSettingCategories::SpellTypeAnnounceCast,         "Spell Announce Casts" }
};

static std::map<uint8, std::string> bot_setting_category_short_names = {
	{ BotSettingCategories::BaseSetting,                   "BaseSetting" },
	{ BotSettingCategories::SpellHold,                     "SpellHolds" },
	{ BotSettingCategories::SpellDelay,                    "SpellDelays" },
	{ BotSettingCategories::SpellMinThreshold,             "SpellMinThresholds" },
	{ BotSettingCategories::SpellMaxThreshold,             "SpellMaxThresholds" },
	{ BotSettingCategories::SpellTypeResistLimit,          "SpellResistLimits" },
	{ BotSettingCategories::SpellTypeAggroCheck,           "SpellAggroChecks" },
	{ BotSettingCategories::SpellTypeMinManaPct,           "SpellMinManaPct" },
	{ BotSettingCategories::SpellTypeMaxManaPct,           "SpellMaxManaPct" },
	{ BotSettingCategories::SpellTypeMinHPPct,             "SpellMinHPPct" },
	{ BotSettingCategories::SpellTypeMaxHPPct,             "SpellMaxHPPct" },
	{ BotSettingCategories::SpellTypeIdlePriority,         "SpellIdlePriority" },
	{ BotSettingCategories::SpellTypeEngagedPriority,      "SpellEngagedPriority" },
	{ BotSettingCategories::SpellTypePursuePriority,       "SpellPursuePriority" },
	{ BotSettingCategories::SpellTypeAEOrGroupTargetCount, "SpellTargetCounts" },
	{ BotSettingCategories::SpellTypeAnnounceCast,         "SpellAnnounceCasts" }
};

namespace BotPriorityCategories {
	constexpr uint8 Idle                              = 0;
	constexpr uint8 Engaged                           = 1;
	constexpr uint8 Pursue                            = 2;

	constexpr uint16 START                            = BotPriorityCategories::Idle;
	constexpr uint16 END                              = BotPriorityCategories::Pursue; // Increment as needed
};

namespace BotBaseSettings {
	constexpr uint16 ExpansionBitmask                 = 0;
	constexpr uint16 ShowHelm                         = 1;
	constexpr uint16 FollowDistance                   = 2;
	constexpr uint16 StopMeleeLevel                   = 3;	
	constexpr uint16 EnforceSpellSettings             = 4;
	constexpr uint16 RangedSetting                    = 5;
	constexpr uint16 PetSetTypeSetting                = 6;
	constexpr uint16 BehindMob                        = 7;
	constexpr uint16 DistanceRanged                   = 8;
	constexpr uint16 IllusionBlock                    = 9;
	constexpr uint16 MaxMeleeRange                    = 10;
	constexpr uint16 MedInCombat                      = 11;
	constexpr uint16 SitHPPct                         = 12;
	constexpr uint16 SitManaPct                       = 13;

	constexpr uint16 START_ALL                        = ExpansionBitmask;
	constexpr uint16 START                            = BotBaseSettings::ShowHelm; // Everything above this cannot be copied, changed or viewed by players
	constexpr uint16 END                              = BotBaseSettings::SitManaPct; // Increment as needed
};

static std::map<uint16, std::string> botBaseSettings_names = {
	{ BotBaseSettings::ExpansionBitmask,             "ExpansionBitmask" },
	{ BotBaseSettings::ShowHelm,                     "ShowHelm" },
	{ BotBaseSettings::FollowDistance,               "FollowDistance" },
	{ BotBaseSettings::StopMeleeLevel,               "StopMeleeLevel" },
	{ BotBaseSettings::EnforceSpellSettings,         "EnforceSpellSettings" },
	{ BotBaseSettings::RangedSetting,                "RangedSetting" },
	{ BotBaseSettings::PetSetTypeSetting,            "PetSetTypeSetting" },
	{ BotBaseSettings::BehindMob,                    "BehindMob" },
	{ BotBaseSettings::DistanceRanged,               "DistanceRanged" },
	{ BotBaseSettings::IllusionBlock,                "IllusionBlock" },
	{ BotBaseSettings::MaxMeleeRange,                "MaxMeleeRange" },
	{ BotBaseSettings::MedInCombat,                  "MedInCombat" },
	{ BotBaseSettings::SitHPPct,                     "SitHPPct" },
	{ BotBaseSettings::SitManaPct,                   "SitManaPct" }
};

namespace CommandedSubTypes {
	constexpr uint16 SingleTarget       = 1;
	constexpr uint16 GroupTarget        = 2;
	constexpr uint16 AETarget           = 3;
	constexpr uint16 SeeInvis           = 4;
	constexpr uint16 Invis              = 5;
	constexpr uint16 InvisUndead        = 6;
	constexpr uint16 InvisAnimals       = 7;
	constexpr uint16 Shrink             = 8;
	constexpr uint16 Grow               = 9;
	constexpr uint16 Selo               = 10;

	constexpr uint16 START              = CommandedSubTypes::SingleTarget;
	constexpr uint16 END                = CommandedSubTypes::Selo;
};

static std::map<uint16, std::string> botSubType_names = {
	{ CommandedSubTypes::SingleTarget,              "SingleTarget" },
	{ CommandedSubTypes::GroupTarget,               "GroupTarget" },
	{ CommandedSubTypes::AETarget,                  "AETarget" },
	{ CommandedSubTypes::SeeInvis,                  "SeeInvis" },
	{ CommandedSubTypes::Invis,                     "Invis" },
	{ CommandedSubTypes::InvisUndead,               "InvisUndead" },
	{ CommandedSubTypes::InvisAnimals,              "InvisAnimals" },
	{ CommandedSubTypes::Shrink,                    "Shrink" },
	{ CommandedSubTypes::Grow,                      "Grow" },
	{ CommandedSubTypes::Selo,                      "Selo" }
};

namespace BotAnimEmpathy {
	constexpr uint8 Guard       		= 1;
	constexpr uint8 Attack           	= 2;
	constexpr uint8 BackOff             = 3;
};

class Bot : public NPC {
	friend class Mob;
public:
	// Class enums
	enum BotTradeType {	// types of trades a bot can do
		BotTradeClientNormal,
		BotTradeClientNoDropNoTrade
	};

	enum SpellTypeIndex : uint32 {
		spellTypeIndexNuke,
		spellTypeIndexHeal,
		spellTypeIndexRoot,
		spellTypeIndexBuff,
		spellTypeIndexEscape,
		spellTypeIndexPet,
		spellTypeIndexLifetap,
		spellTypeIndexSnare,
		spellTypeIndexDot,
		spellTypeIndexDispel,
		spellTypeIndexInCombatBuff,
		spellTypeIndexMez,
		spellTypeIndexCharm,
		spellTypeIndexSlow,
		spellTypeIndexDebuff,
		spellTypeIndexCure,
		spellTypeIndexResurrect,
		spellTypeIndexHateRedux,
		spellTypeIndexInCombatBuffSong,
		spellTypeIndexOutOfCombatBuffSong,
		spellTypeIndexPreCombatBuff,
		spellTypeIndexPreCombatBuffSong
	};

	static const uint32 SPELL_TYPE_FIRST = spellTypeIndexNuke;
	static const uint32 SPELL_TYPE_LAST = spellTypeIndexPreCombatBuffSong;
	static const uint32 SPELL_TYPE_COUNT = SPELL_TYPE_LAST + 1;

	// Class Constructors
	Bot(NPCType *npcTypeData, Client* botOwner);
	Bot(uint32 botID, uint32 botOwnerCharacterID, uint32 botSpellsID, double totalPlayTime, uint32 lastZoneId, NPCType *npcTypeData);

	//abstract virtual override function implementations requird by base abstract class
	bool Death(Mob* killer_mob, int64 damage, uint16 spell_id, EQ::skills::SkillType attack_skill, KilledByTypes killed_by = KilledByTypes::Killed_NPC, bool is_buff_tic = false) override;
	void Damage(Mob* from, int64 damage, uint16 spell_id, EQ::skills::SkillType attack_skill, bool avoidable = true, int8 buffslot = -1,
		bool iBuffTic = false, eSpecialAttacks special = eSpecialAttacks::None) override;

	bool HasRaid() final { return GetRaid() != nullptr; }
	bool HasGroup() final { return GetGroup() != nullptr; }
	Raid* GetRaid() final { return entity_list.GetRaidByBot(this); }
	Group* GetGroup() final { return entity_list.GetGroupByMob(this); }
	Group* GetGroupByLeaderName() { return entity_list.GetGroupByLeaderName(GetName()); }

	// Common, but informal "interfaces" with Client object
	uint32 CharacterID() const { return GetBotID(); }
	inline bool IsInAGuild() const { return (_guildId != GUILD_NONE && _guildId != 0); }
	inline bool IsInGuild(uint32 in_gid) const { return (in_gid == _guildId && IsInAGuild()); }
	inline uint32 GuildID() const { return _guildId; }
	inline uint8	GuildRank()	const { return _guildRank; }

	// Class Methods
	bool IsValidRaceClassCombo();
	static bool IsValidRaceClassCombo(uint16 r, uint8 c);
	bool IsValidName();
	static bool IsValidName(std::string& name);
	bool Spawn(Client* botCharacterOwner);
	void SetLevel(uint8 in_level, bool command = false) override;
	void FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho) override;
	bool Process() override;
	void FinishTrade(Client* client, BotTradeType trade_type, int16 chosen_slot = INVALID_INDEX);
	bool Save() override;
	void Depop();
	void CalcBotStats(bool showtext = true);
	uint16 BotGetSpells(int spellslot) { return AIBot_spells[spellslot].spellid; }
	uint32 BotGetSpellType(int spellslot) { return AIBot_spells[spellslot].type; }
	uint16 BotGetSpellPriority(int spellslot) { return AIBot_spells[spellslot].priority; }
	const std::vector<BotSpells_wIndex>& BotGetSpellsByType(uint16 spell_type) const;
	float GetProcChances(float ProcBonus, uint16 hand) override;
	int GetHandToHandDamage(void) override;
	bool TryFinishingBlow(Mob *defender, int64 &damage) override;
	void DoRiposte(Mob* defender) override;
	inline int32 GetATK() { return ATK + itembonuses.ATK + spellbonuses.ATK + ((GetSTR() + GetSkill(EQ::skills::SkillOffense)) * 9 / 10); }
	inline int32 GetATKBonus() const override { return itembonuses.ATK + spellbonuses.ATK; }
	uint32 GetTotalATK();
	uint32 GetATKRating();
	uint16 GetPrimarySkillValue();
	uint16	MaxSkill(EQ::skills::SkillType skillid, uint16 class_, uint16 level) const;
	inline	uint16	MaxSkill(EQ::skills::SkillType skillid) { return MaxSkill(skillid, GetClass(), GetLevel()); }
	int GetBaseSkillDamage(EQ::skills::SkillType skill, Mob *target = nullptr) override;
	void DoSpecialAttackDamage(Mob *who, EQ::skills::SkillType skill, int32 max_damage, int32 min_damage = 1, int32 hate_override = -1, int ReuseTime = 10, bool HitChance = false);
	void DoClassAttacks(Mob *target, bool IsRiposte=false);
	void CalcBonuses() override;

	void MakePet(uint16 spell_id, const char* pettype, const char *petname = nullptr) override;
	FACTION_VALUE GetReverseFactionCon(Mob* iOther) override;
	inline bool IsPet() override { return false; }
	bool IsNPC() const override { return false; }
	Mob* GetOwner() override;
	Mob* GetOwnerOrSelf() override;
	inline bool HasOwner() override { return GetBotOwner() != nullptr; }
	int64 CalcMaxMana() override;
	void SetAttackTimer() override;
	uint64 GetClassHPFactor();
	int64 CalcMaxHP() override;
	bool DoFinishedSpellSingleTarget(uint16 spell_id, Mob* spellTarget, EQ::spells::CastingSlot slot, bool &stopLogic);
	bool DoFinishedSpellGroupTarget(uint16 spell_id, Mob* spellTarget, EQ::spells::CastingSlot slot, bool &stopLogic);
	void Camp(bool save_to_database = true);
	void SetTarget(Mob* mob) override;
	void Zone();
	bool IsAtRange(Mob* target);
	void ChangeBotRangedWeapons(bool is_ranged);
	void Sit();
	void Stand();
	bool IsSitting() const override;
	bool IsStanding();
	int GetWalkspeed() { return (int)((float)_GetWalkSpeed() * 1.785714285f); } // 1.25 / 0.7 = 1.7857142857142857142857142857143
	int GetRunspeed() { return (int)((float)_GetRunSpeed() * 1.785714285f); }
	void WalkTo(float x, float y, float z) override;
	void RunTo(float x, float y, float z) override;

	bool GetGuardFlag() const { return m_guard_flag; }
	void SetGuardFlag(bool flag = true) { m_guard_flag = flag; }
	bool GetHoldFlag() const { return m_hold_flag; }
	void SetHoldFlag(bool flag = true) { m_hold_flag = flag; }
	bool GetAttackFlag() const { return m_attack_flag; }
	void SetAttackFlag(bool flag = true) { m_attack_flag = flag; }
	bool GetCombatRoundForAlerts() const { return m_combat_round_alert_flag; }
	bool GetAttackingFlag() const { return m_attacking_flag; }
	bool GetPullFlag() const { return m_pull_flag; }
	void SetPullFlag(bool flag = true) { m_pull_flag = flag; }
	bool GetPullingFlag() const { return m_pulling_flag; }
	bool GetReturningFlag() const { return m_returning_flag; }
	bool GetIsUsingItemClick() { return is_using_item_click; }
	void SetIsUsingItemClick(bool flag = true) { is_using_item_click = flag; }
	bool UseDiscipline(uint32 spell_id, uint32 target);
	uint8 GetNumberNeedingHealedInGroup(Mob* tar, uint16 spell_type, uint16 spell_id, float range);
	bool GetNeedsCured(Mob *tar);
	bool GetNeedsHateRedux(Mob *tar);
	bool HasOrMayGetAggro(bool SitAggro, uint32 spell_id = 0);
	void SetDefaultBotStance();
	void SetSurname(std::string_view bot_surname);
	void SetTitle(std::string_view bot_title);
	void SetSuffix(std::string_view bot_suffix);
	std::string GetSurname() const { return _surname; }
	std::string GetTitle() const { return _title; }
	std::string GetSuffix() const { return _suffix; }
	inline virtual int32	GetMaxStat();
	inline virtual int32	GetMaxResist();
	inline virtual int32	GetMaxSTR();
	inline virtual int32	GetMaxSTA();
	inline virtual int32	GetMaxDEX();
	inline virtual int32	GetMaxAGI();
	inline virtual int32	GetMaxINT();
	inline virtual int32	GetMaxWIS();
	inline virtual int32	GetMaxCHA();
	inline virtual int32	GetMaxMR();
	inline virtual int32	GetMaxPR();
	inline virtual int32	GetMaxDR();
	inline virtual int32	GetMaxCR();
	inline virtual int32	GetMaxFR();
	inline virtual int32	GetMaxCorrup();
	int32	CalcATK();
	int32	CalcSTR();
	int32	CalcSTA();
	int32	CalcDEX();
	int32	CalcAGI();
	int32	CalcINT();
	int32	CalcWIS();
	int32	CalcCHA();
	int32	CalcMR();
	int32	CalcFR();
	int32	CalcDR();
	int32	CalcPR();
	int32	CalcCR();
	int32	CalcCorrup();
	int64	CalcHPRegenCap() final;
	int64	CalcManaRegenCap() final;
	int32	LevelRegen();
	int64	CalcHPRegen();
	int64	CalcManaRegen();
	uint32	CalcCurrentWeight();
	int		GroupLeadershipAAHealthEnhancement();
	int		GroupLeadershipAAManaEnhancement();
	int 	GroupLeadershipAAHealthRegeneration();
	int		GroupLeadershipAAOffenseEnhancement();
	void CalcRestState();

	int64 CalcMaxEndurance();
	int64 CalcBaseEndurance();
	int64 CalcEnduranceRegen();
	int64 GetEndurance()	const override {return cur_end;}
	int64 GetMaxEndurance() const override {return max_end;}
	int64 CalcEnduranceRegenCap();
	inline uint8 GetEndurancePercent() override { return (uint8)((float)cur_end / (float)max_end * 100.0f); }
	void SetEndurance(int32 newEnd) override;
	void DoEnduranceUpkeep();

	void TryItemClick(uint16 slot_id);
	EQ::ItemInstance* GetClickItem(uint16 slot_id);
	void DoItemClick(const EQ::ItemData* inst, uint16 slot_id);

	bool AI_AddBotSpells(uint32 bot_spell_id);
	void AddSpellToBotList(
		int16 iPriority,
		uint16 iSpellID,
		uint32 iType,
		int16 iManaCost,
		int32 iRecastDelay,
		int16 iResistAdjust,
		uint8 min_level,
		uint8 max_level,
		int8 min_hp,
		int8 max_hp,
		std::string bucket_name,
		std::string bucket_value,
		uint8 bucket_comparison
	);

	void AddSpellToBotEnforceList(
		int16 iPriority,
		uint16 iSpellID,
		uint32 iType,
		int16 iManaCost,
		int32 iRecastDelay,
		int16 iResistAdjust,
		uint8 min_level,
		uint8 max_level,
		int8 min_hp,
		int8 max_hp,
		std::string bucket_name,
		std::string bucket_value,
		uint8 bucket_comparison
	);

	void AI_Bot_Event_SpellCastFinished(bool iCastSucceeded, uint16 slot);

	// AI Methods
	bool AICastSpell(Mob* tar, uint8 chance, uint16 spell_type, uint16 sub_target_type = UINT16_MAX, uint16 sub_type = UINT16_MAX);
	bool AttemptAICastSpell(uint16 spell_type, Mob* tar = nullptr);
	bool AttemptAACastSpell(Mob* tar, uint16 spell_id, AA::Rank* rank);
	bool AttemptForcedCastSpell(Mob* tar, uint16 spell_id, bool is_disc = false);
	bool AttemptCloseBeneficialSpells(uint16 spell_type);
	bool AI_EngagedCastCheck() override;
	bool AI_PursueCastCheck() override;
	bool AI_IdleCastCheck() override;
	bool AIHealRotation(Mob* tar, bool useFastHeals);
	bool GetPauseAI() const { return _pauseAI; }
	void SetPauseAI(bool pause_flag) { _pauseAI = pause_flag; }
	bool IsCommandedSpell() const { return _commandedSpell; }
	void SetCommandedSpell(bool value) { _commandedSpell = value;  }
	bool IsPullingSpell() const { return _pullingSpell; }
	void SetPullingSpell(bool value) { _pullingSpell = value; }
	
	void SetGuardMode();
	void SetHoldMode();

	bool IsValidSpellRange(uint16 spell_id, Mob* tar);

	// Bot AI Methods
	void AI_Bot_Init();
	void AI_Bot_Start(uint32 iMoveDelay = 0);

	// Mob AI Virtual Override Methods
	void AI_Process() final;
	void AI_Stop() final;

	// Mob Spell Virtual Override Methods
	void SpellProcess() override;
	int32 GetActSpellDuration(uint16 spell_id, int32 duration) override;
	float GetAOERange(uint16 spell_id) override;
	virtual bool SpellEffect(Mob* caster, uint16 spell_id, float partial = 100);
	void DoBuffTic(const Buffs_Struct &buff, int slot, Mob* caster = nullptr) override;
	virtual bool CastSpell(uint16 spell_id, uint16 target_id, EQ::spells::CastingSlot slot = EQ::spells::CastingSlot::Item, int32 casttime = -1, int32 mana_cost = -1, uint32* oSpellWillFinish = 0,
						uint32 item_slot = 0xFFFFFFFF, int16 *resist_adjust = nullptr, uint32 aa_id = 0);
	bool SpellOnTarget(
			uint16 spell_id,
			Mob* spelltar,
			int reflect_effectiveness = 0,
			bool use_resist_adjust = false,
			int16 resist_adjust = 0,
			bool isproc = false,
			int level_override = -1,
			int duration_override = 0,
			bool disable_buff_overwrite = false
	) final;
	bool IsImmuneToSpell(uint16 spell_id, Mob *caster) override;
	virtual bool DetermineSpellTargets(uint16 spell_id, Mob *&spell_target, Mob *&ae_center, CastAction_type &CastAction, EQ::spells::CastingSlot slot);
	virtual bool DoCastSpell(uint16 spell_id, uint16 target_id, EQ::spells::CastingSlot slot = EQ::spells::CastingSlot::Item, int32 casttime = -1, int32 mana_cost = -1,
						uint32* oSpellWillFinish = nullptr, uint32 item_slot = 0xFFFFFFFF, uint32 aa_id = 0);
	inline int64 GetFocusEffect(focusType type, uint16 spell_id, Mob *caster = nullptr, bool from_buff_tic = false) override
		{ return Mob::GetFocusEffect(type, spell_id, caster, from_buff_tic); }
	inline bool Attack(Mob* other, int Hand = EQ::invslot::slotPrimary, bool FromRiposte = false, bool IsStrikethrough = false,
		bool IsFromSpell = false, ExtraAttackOptions *opts = nullptr) override
			{ return Mob::Attack(other, Hand, FromRiposte, IsStrikethrough, IsFromSpell, opts); }
	void DoAttackRounds(Mob* target, int hand);

	bool BotPassiveCheck();
	bool ValidStateCheck(Mob* other, bool same_raid_group = false);
	Raid* GetStoredRaid() { return _storedRaid; }
	void SetStoredRaid(Raid* stored_raid) { _storedRaid = stored_raid; }
	bool GetVerifiedRaid() { return _verifiedRaid; }
	void SetVerifiedRaid(bool status) { _verifiedRaid = status; }
	uint16 GetTempSpellType() { return _tempSpellType; }
	void SetTempSpellType(uint16 spell_type) { _tempSpellType = spell_type; }
	bool IsMobEngagedByAnyone(Mob* tar);
	void SetBotTimers(std::vector<BotTimer> timers) { bot_timers = timers; }

	// Targeting
	std::vector<Mob*> GatherSpellTargets(bool entireRaid = false, Mob* target = nullptr, bool no_clients = false, bool no_bots = false);
	bool HasValidAETarget(Bot* caster, uint16 spell_id, uint16 spell_type, Mob* tar);
	void SetHasLoS(bool has_los) { _hasLoS = has_los; }
	bool HasLoS() const { return _hasLoS; }
	bool IsValidMezTarget(Mob* owner, Mob* npc, uint16 spell_id);

	// Cast checks
	bool PrecastChecks(Mob* tar, uint16 spell_type);	
	bool CastChecks(uint16 spell_id, Mob* tar, uint16 spell_type, bool prechecks = false, bool ae_check = false);
	bool IsImmuneToBotSpell(uint16 spell_id, Mob* caster);
	bool CanCastSpellType(uint16 spell_type, uint16 spell_id, Mob* tar);
	bool BotHasEnoughMana(uint16 spell_id);
	bool IsTargetAlreadyReceivingSpell(Mob* tar, uint16 spell_id);
	bool DoResistCheck(Mob* target, uint16 spell_id, int32 resist_limit);
	bool DoResistCheckBySpellType(Mob* tar, uint16 spell_id, uint16 spell_type);
	bool IsValidTargetType(uint16 spell_id, int target_type, uint8 body_type);

	// Spell checks
	static bool IsValidBotSpellType(uint16 spell_type);
	uint16 GetPetBotSpellType(uint16 spell_type);

	// Movement checks
	bool PlotBotPositionAroundTarget(const FindPositionInput& input);
	std::vector<Mob*> GetSpellTargetList(bool entire_raid = false);
	void SetSpellTargetList(std::vector<Mob*> spell_target_list) { _spell_target_list = spell_target_list; }
	std::vector<Mob*> GetGroupSpellTargetList() { return _group_spell_target_list; }
	void SetGroupSpellTargetList(std::vector<Mob*> spell_target_list) { _group_spell_target_list = spell_target_list; }
	std::vector<Mob*> GetBuffTargets(Mob* spellTarget);

	// Bot settings
	void LoadDefaultBotSettings();
	int GetDefaultSetting(uint16 setting_category, uint16 setting_type, uint8 stance = Stance::Balanced);
	int GetDefaultBotBaseSetting(uint16 bot_setting, uint8 stance = Stance::Balanced);
	bool GetDefaultSpellTypeHold(uint16 spell_type, uint8 stance = Stance::Balanced);
	uint16 GetDefaultSpellTypePriority(uint16 spell_type, uint8 priority_type, uint8 bot_class, uint8 stance = Stance::Balanced);
	uint16 GetDefaultSpellTypeIdlePriority(uint16 spell_type, uint8 bot_class, uint8 stance = Stance::Balanced);
	uint16 GetDefaultSpellTypeEngagedPriority(uint16 spell_type, uint8 bot_class, uint8 stance = Stance::Balanced);
	uint16 GetDefaultSpellTypePursuePriority(uint16 spell_type, uint8 bot_class, uint8 stance = Stance::Balanced);
	uint16 GetDefaultSpellTypeResistLimit(uint16 spell_type, uint8 stance = Stance::Balanced);
	bool GetDefaultSpellTypeAggroCheck(uint16 spell_type, uint8 stance = Stance::Balanced);
	uint8 GetDefaultSpellTypeMinManaLimit(uint16 spell_type, uint8 stance = Stance::Balanced);
	uint8 GetDefaultSpellTypeMaxManaLimit(uint16 spell_type, uint8 stance = Stance::Balanced);
	uint8 GetDefaultSpellTypeMinHPLimit(uint16 spell_type, uint8 stance = Stance::Balanced);
	uint8 GetDefaultSpellTypeMaxHPLimit(uint16 spell_type, uint8 stance = Stance::Balanced);
	uint16 GetDefaultSpellTypeAnnounceCast(uint16 spell_type, uint8 stance = Stance::Balanced);
	uint16 GetDefaultSpellTypeAEOrGroupTargetCount(uint16 spell_type, uint8 stance = Stance::Balanced);

	static bool IsValidBotBaseSetting(uint16 setting_type);
	static std::string GetBotSettingCategoryName(uint16 setting_type);
	uint16 GetBaseSettingIDByShortName(std::string setting_string);
	int GetBotBaseSetting(uint16 bot_setting);
	void SetBotBaseSetting(uint16 bot_setting, int setting_value);
	int GetSetting(uint16 setting_category, uint16 setting_type);
	void SetBotSetting(uint8 setting_type, uint16 bot_setting, int setting_value);
	void CopyBotSpellSettings(Bot* to);
	void ResetBotSpellSettings();

	void CopyBotBlockedBuffs(Bot* to);
	void CopyBotBlockedPetBuffs(Bot* to);	
	void CleanBotBlockedBuffs();
	void ClearBotBlockedBuffs() { bot_blocked_buffs.clear(); }
	bool IsBlockedBuff(int32 spell_id) override;
	bool IsBlockedPetBuff(int32 spell_id) override;
	void SetBotBlockedBuff(uint16 spell_id, bool block);
	void SetBotBlockedPetBuff(uint16 spell_id, bool block);
	std::vector<BotBlockedBuffs> GetBotBlockedBuffs() { return bot_blocked_buffs; }
	void SetBotBlockedBuffs(std::vector<BotBlockedBuffs> blocked_buffs) { bot_blocked_buffs = blocked_buffs; }

	void SetBotSpellRecastTimer(uint16 spell_type, Mob* spelltar, bool pre_cast = false);
	uint16 GetSpellTypePriority(uint16 spell_type, uint8 priority_type);
	void SetSpellTypePriority(uint16 spell_type, uint8 priority_type, uint16 priority);
	inline uint16 GetSpellTypeResistLimit(uint16 spell_type) const { return m_bot_spell_settings[spell_type].resist_limit; }
	inline void SetSpellTypeResistLimit(uint16 spell_type, uint16 value) { m_bot_spell_settings[spell_type].resist_limit = value; }
	inline bool GetSpellTypeAggroCheck(uint16 spell_type) const { return m_bot_spell_settings[spell_type].aggro_check; }
	inline void SetSpellTypeAggroCheck(uint16 spell_type, bool value) { m_bot_spell_settings[spell_type].aggro_check = value; }
	uint8 GetHPRatioForSpellType(uint16 spell_type, Mob* tar);
	inline uint8 GetSpellTypeMinManaLimit(uint16 spell_type) const { return m_bot_spell_settings[spell_type].min_mana_pct; }
	inline uint8 GetSpellTypeMaxManaLimit(uint16 spell_type) const { return m_bot_spell_settings[spell_type].max_mana_pct; }
	inline void SetSpellTypeMinManaLimit(uint16 spell_type, uint8 value) { m_bot_spell_settings[spell_type].min_mana_pct = value; }
	inline void SetSpellTypeMaxManaLimit(uint16 spell_type, uint8 value) { m_bot_spell_settings[spell_type].max_mana_pct = value; }
	inline uint8 GetSpellTypeMinHPLimit(uint16 spell_type) const { return m_bot_spell_settings[spell_type].min_hp_pct; }
	inline uint8 GetSpellTypeMaxHPLimit(uint16 spell_type) const { return m_bot_spell_settings[spell_type].max_hp_pct; }
	inline void SetSpellTypeMinHPLimit(uint16 spell_type, uint8 value) { m_bot_spell_settings[spell_type].min_hp_pct = value; }
	inline void SetSpellTypeMaxHPLimit(uint16 spell_type, uint8 value) { m_bot_spell_settings[spell_type].max_hp_pct = value; }
	inline uint16 GetSpellTypeAEOrGroupTargetCount(uint16 spell_type) const { return m_bot_spell_settings[spell_type].ae_or_group_target_count; }
	inline void SetSpellTypeAEOrGroupTargetCount(uint16 spell_type, uint16 value) { m_bot_spell_settings[spell_type].ae_or_group_target_count = value; }
	inline uint16 GetSpellTypeAnnounceCast(uint16 spell_type) const { return m_bot_spell_settings[spell_type].announce_cast; }
	inline void SetSpellTypeAnnounceCast(uint16 spell_type, uint16 value) { m_bot_spell_settings[spell_type].announce_cast = value; }
	inline bool GetSpellTypeHold(uint16 spell_type) const { return m_bot_spell_settings[spell_type].hold; }
	inline void SetSpellTypeHold(uint16 spell_type, bool value) { m_bot_spell_settings[spell_type].hold = value; }
	inline uint16 GetSpellTypeDelay(uint16 spell_type) const { return m_bot_spell_settings[spell_type].delay; }
	inline void SetSpellTypeDelay(uint16 spell_type, uint16 delay_value) { m_bot_spell_settings[spell_type].delay = delay_value; }
	inline uint8 GetSpellTypeMinThreshold(uint16 spell_type) const { return m_bot_spell_settings[spell_type].min_threshold; }
	inline void SetSpellTypeMinThreshold(uint16 spell_type, uint8 threshold_value) { m_bot_spell_settings[spell_type].min_threshold = threshold_value; }
	inline uint8 GetSpellTypeMaxThreshold(uint16 spell_type) const { return m_bot_spell_settings[spell_type].max_threshold; }
	inline void SetSpellTypeMaxThreshold(uint16 spell_type, uint8 threshold_value) { m_bot_spell_settings[spell_type].max_threshold = threshold_value; }
	inline bool SpellTypeRecastCheck(uint16 spellType) { return !m_bot_spell_settings[spellType].recast_timer.GetRemainingTime(); }
	void SetSpellTypeRecastTimer(uint16 spell_type, uint32 recast_time) { m_bot_spell_settings[spell_type].recast_timer.Start(recast_time); }
	inline bool SpellTypeAIDelayCheck(uint16 spellType) { return !m_bot_spell_settings[spellType].ai_delay.GetRemainingTime(); }
	void SetSpellTypeAITimer(uint16 spell_type, uint32 recast_time) { m_bot_spell_settings[spell_type].ai_delay.Start(recast_time); }
	bool GetUltimateSpellTypeHold(uint16 spell_type, Mob* tar);
	uint16 GetDefaultSpellTypeDelay(uint16 spell_type, uint8 stance = Stance::Balanced);
	uint8 GetDefaultSpellTypeMinThreshold(uint16 spell_type, uint8 stance = Stance::Balanced);
	uint8 GetDefaultSpellTypeMaxThreshold(uint16 spell_type, uint8 stance = Stance::Balanced);
	uint16 GetUltimateSpellTypeDelay(uint16 spell_type, Mob* tar);
	bool GetUltimateSpellTypeRecastCheck(uint16 spell_type, Mob* tar);
	uint8 GetUltimateSpellTypeMinThreshold(uint16 spell_type, Mob* tar);
	uint8 GetUltimateSpellTypeMaxThreshold(uint16 spell_type, Mob* tar);
	void SetIllusionBlock(bool value) { _illusionBlock = value; }
	bool GetIllusionBlock() const override { return _illusionBlock; }
	bool GetShowHelm() const { return _showHelm; }
	void SetShowHelm(bool show_helm) { _showHelm = show_helm; }
	bool GetBehindMob() const { return _behindMobStatus; }
	void SetBehindMob(bool value) { _behindMobStatus = value; }
	bool GetMaxMeleeRange() const { return _maxMeleeRangeStatus; }
	void SetMaxMeleeRange(bool value) { _maxMeleeRangeStatus = value; }	
	uint8 GetStopMeleeLevel() const { return _stopMeleeLevel; }
	void SetStopMeleeLevel(uint8 level) { _stopMeleeLevel = level; }
	uint32 GetBotDistanceRanged() const { return _distanceRanged; }
	void SetBotDistanceRanged(uint32 distance) { _distanceRanged = distance; }
	bool GetMedInCombat() const { return _medInCombat; }
	void SetMedInCombat(bool value) { _medInCombat = value; }
	uint8 GetSitHPPct() const { return _SitHPPct; }
	void SetSitHPPct(uint8 value) { _SitHPPct = value; }
	uint8 GetSitManaPct() const { return _SitManaPct; }
	void SetSitManaPct(uint8 value) { _SitManaPct = value; }

	// Spell lists
	std::list<BotSpellTypeOrder> GetSpellTypesPrioritized(uint8 priority_type);
	static uint16 GetParentSpellType(uint16 spell_type);
	static bool IsValidSpellTypeBySpellID(uint16 spell_type, uint16 spell_id);
	inline uint16 GetCastedSpellType() const { return _castedSpellType; }
	void SetCastedSpellType(uint16 spell_type);
	bool IsValidSpellTypeSubType(uint16 spell_type, uint16 sub_type, uint16 spell_id);
	static bool IsValidBotSpellCategory(uint8 setting_type);
	static std::string GetBotSpellCategoryName(uint8 setting_type);
	static std::string GetBotSpellCategoryShortName(uint8 setting_type);
	static uint16 GetBotSpellCategoryIDByShortName(std::string setting_string);
	void AssignBotSpellsToTypes(std::vector<BotSpells>& AIBot_spells, std::unordered_map<uint16, std::vector<BotSpells_wIndex>>& AIBot_spells_by_type);
	uint16 GetSpellByAA(int id, AA::Rank*& rank);

	// Spell Type
	static uint16 GetSpellTypeIDByShortName(std::string spellType_string);
	static std::string GetSpellTypeNameByID(uint16 spell_type);
	static std::string GetSpellTypeShortNameByID(uint16 spell_type);
	bool IsValidSubType(uint16 sub_type);
	static std::string GetSubTypeNameByID(uint16 sub_type);

	[[nodiscard]] int GetMaxBuffSlots() const final { return EQ::spells::LONG_BUFFS; }
	[[nodiscard]] int GetMaxSongSlots() const final { return EQ::spells::SHORT_BUFFS; }
	[[nodiscard]] int GetMaxDiscSlots() const final { return EQ::spells::DISC_BUFFS; }
	[[nodiscard]] int GetMaxTotalSlots() const final { return EQ::spells::TOTAL_BUFFS; }

	bool CheckDataBucket(std::string bucket_name, const std::string& bucket_value, uint8 bucket_comparison);

	// Bot Equipment & Inventory Class Methods
	void BotTradeAddItem(const EQ::ItemInstance* inst, uint16 slot_id, bool save_to_database = true);
	void EquipBot();
	bool CheckLoreConflict(const EQ::ItemData* item);
	void UpdateEquipmentLight() override
		{
			m_Light.Type[EQ::lightsource::LightEquipment] = m_inv.FindBrightestLightType();
			m_Light.Level[EQ::lightsource::LightEquipment] = EQ::lightsource::TypeToLevel(m_Light.Type[EQ::lightsource::LightEquipment]);
		}

	inline EQ::InventoryProfile& GetInv() override { return m_inv; }

	// Static Class Methods
	static Bot* LoadBot(uint32 botID);
	static uint32 SpawnedBotCount(const uint32 owner_id, uint8 class_id = Class::None);
	static void LevelBotWithClient(Client* client, uint8 level, bool sendlvlapp);

	static bool IsBotAttackAllowed(Mob* attacker, Mob* target, bool& hasRuleDefined);
	static Bot* GetBotByBotClientOwnerAndBotName(Client* c, const std::string& botName);
	static void ProcessBotGroupInvite(Client* c, std::string const& botName);
	static void ProcessBotGroupDisband(Client* c, const std::string& botName);
	static void BotOrderCampAll(Client* c, uint8 class_id = Class::None);
	static void ProcessBotInspectionRequest(Bot* inspectedBot, Client* client);
	static void LoadAndSpawnAllZonedBots(Client* bot_owner);
	static bool GroupHasBot(Group* group);
	static Bot* GetFirstBotInGroup(Group* group);
	static void ProcessClientZoneChange(Client* botOwner);
	static void ProcessBotOwnerRefDelete(Mob* botOwner); // Removes a Client* reference when the Client object is destroyed

	//Raid methods
	static void ProcessRaidInvite(Mob* invitee, Client* invitor, bool group_invite = false);
	static void RemoveBotFromRaid(Bot* bot);
	inline void SetDirtyAutoHaters() { m_dirtyautohaters = true; }
	static void CreateBotRaid(Mob* invitee, Client* invitor, bool group_invite, Raid* raid);
	static void
	ProcessBotGroupAdd(Group* group, Raid* raid, Client* client = nullptr, bool new_raid = false, bool initial = false);


	static std::list<BotSpell> GetBotSpellsForSpellEffect(Bot* caster, uint16 spell_type, int spell_effect);
	static std::list<BotSpell> GetBotSpellsForSpellEffectAndTargetType(Bot* caster, uint16 spell_type, int spell_effect, SpellTargetType target_type);
	static std::list<BotSpell> GetBotSpellsBySpellType(Bot* caster, uint16 spell_type);
	static std::vector<BotSpell_wPriority> GetPrioritizedBotSpellsBySpellType(Bot* caster, uint16 spell_type, Mob* tar, bool AE = false, uint16 sub_target_type = UINT16_MAX, uint16 sub_type = UINT16_MAX);

	static BotSpell GetFirstBotSpellBySpellType(Bot* caster, uint16 spell_type);
	BotSpell GetSpellByHealType(uint16 spell_type, Mob* tar);
	static BotSpell GetBestBotSpellForVeryFastHeal(Bot* caster, Mob* tar, uint16 spell_type = BotSpellTypes::RegularHeal);
	static BotSpell GetBestBotSpellForFastHeal(Bot* caster, Mob* tar, uint16 spell_type = BotSpellTypes::RegularHeal);
	static BotSpell GetBestBotSpellForHealOverTime(Bot* caster, Mob* tar, uint16 spell_type = BotSpellTypes::RegularHeal);
	static BotSpell GetBestBotSpellForPercentageHeal(Bot* caster, Mob* tar, uint16 spell_type = BotSpellTypes::RegularHeal);
	static BotSpell GetBestBotSpellForRegularSingleTargetHeal(Bot* caster, Mob* tar, uint16 spell_type = BotSpellTypes::RegularHeal);
	static BotSpell GetFirstBotSpellForSingleTargetHeal(Bot* caster, Mob* tar, uint16 spell_type = BotSpellTypes::RegularHeal);
	static BotSpell GetBestBotSpellForGroupHealOverTime(Bot* caster, Mob* tar, uint16 spell_type = BotSpellTypes::RegularHeal);
	static BotSpell GetBestBotSpellForGroupCompleteHeal(Bot* caster, Mob* tar, uint16 spell_type = BotSpellTypes::RegularHeal);
	static BotSpell GetBestBotSpellForGroupHeal(Bot* caster, Mob* tar, uint16 spell_type = BotSpellTypes::RegularHeal);

	static Mob* GetFirstIncomingMobToMez(Bot* caster, uint16 spell_id, uint16 spell_type, bool AE);
	static BotSpell GetBestBotSpellForMez(Bot* caster, uint16 spell_type = BotSpellTypes::Mez);
	static BotSpell GetBestBotMagicianPetSpell(Bot* caster, uint16 spell_type = BotSpellTypes::Pet);
	static std::string GetBotMagicianPetType(Bot* caster);
	static BotSpell GetBestBotSpellForNukeByTargetType(Bot* caster, SpellTargetType target_type, uint16 spell_type, bool AE = false, Mob* tar = nullptr);
	static BotSpell GetBestBotSpellForStunByTargetType(Bot* caster, SpellTargetType target_type, uint16 spell_type, bool AE = false, Mob* tar = nullptr);
	static BotSpell GetBestBotWizardNukeSpellByTargetResists(Bot* caster, Mob* target, uint16 spell_type);
	static BotSpell GetDebuffBotSpell(Bot* caster, Mob* target, uint16 spell_type);
	static BotSpell GetBestBotSpellForCure(Bot* caster, Mob* target, uint16 spell_type);
	static BotSpell GetBestBotSpellForResistDebuff(Bot* caster, Mob* target, uint16 spell_type);
	static BotSpell GetBestBotSpellForNukeByBodyType(Bot* caster, uint8 body_type, uint16 spell_type, bool AE = false, Mob* tar = nullptr);
	static BotSpell GetBestBotSpellForRez(Bot* caster, Mob* target, uint16 spell_type);
	static BotSpell GetBestBotSpellForCharm(Bot* caster, Mob* target, uint16 spell_type);

	static NPCType *CreateDefaultNPCTypeStructForBot(
		const std::string& botName,
		const std::string& botLastName,
		uint8 botLevel,
		uint16 botRace,
		uint8 botClass,
		uint8 gender
	);

	void AddBotStartingItems(uint16 race_id, uint8 class_id);

	// Static Bot Group Methods
	static bool AddBotToGroup(Bot* bot, Group* group);
	static bool RemoveBotFromGroup(Bot* bot, Group* group);
	void RaidGroupSay(const char *msg, ...);

	// "GET" Class Methods
	uint32 GetBotID() const { return _botID; }
	uint32 GetBotOwnerCharacterID() const { return _botOwnerCharacterID; }
	uint32 GetBotSpellID() const { return npc_spells_id; }
	Mob* GetBotOwner() { return this->_botOwner; }
	uint32 GetBotRangedValue();
	EQ::ItemInstance* GetBotItem(uint16 slot_id);
	bool GetSpawnStatus() { return _spawnStatus; }
	uint8 GetPetChooserID() { return _petChooserID; }
	bool HasControllablePet(uint8 ranks_required = 0);
	bool IsBotRanged() { return _botRangedSetting; }
	bool IsBotCharmer() { return _botCharmer; }
	bool IsBot() const override { return true; }
	bool IsOfClientBot() const override { return true; }
	bool IsOfClientBotMerc() const override { return true; }

	bool GetRangerAutoWeaponSelect() { return _rangerAutoWeaponSelect; }
	uint8 GetBotStance() { return _botStance; }
	static bool IsValidBotStance(uint8 stance);
	uint8 GetChanceToCastBySpellType(uint16 spell_type);
	bool IsGroupHealer() const { return m_CastingRoles.GroupHealer; }
	bool IsGroupSlower() const { return m_CastingRoles.GroupSlower; }
	bool IsGroupNuker() const { return m_CastingRoles.GroupNuker; }
	bool IsGroupDoter() const { return m_CastingRoles.GroupDoter; }
	static void UpdateGroupCastingRoles(const Group* group, bool disband = false);

	bool IsBotCaster() { return IsCasterClass(GetClass()); }
	bool IsBotHybrid() { return IsHybridClass(GetClass()); }
	bool IsBotINTCaster() { return IsINTCasterClass(GetClass()); }
	bool IsBotWISCaster() { return IsWISCasterClass(GetClass()); }
	bool IsBotSpellFighter() { return IsSpellFighterClass(GetClass()); }
	bool IsBotFighter() { return IsFighterClass(GetClass()); }
	bool IsBotNonSpellFighter() { return (GetClass() == Class::Bard ? false : IsNonSpellFighterClass(GetClass())); }
	uint8 GetBotClass() { return GetClass(); }
	int GetRawACNoShield(int &shield_ac);

	// new heal rotation code
	bool CreateHealRotation(uint32 cycle_duration_ms = 5000, bool fast_heals = false, bool adaptive_targeting = false, bool casting_override = false);
	bool DestroyHealRotation();
	bool JoinHealRotationMemberPool(std::shared_ptr<HealRotation>* heal_rotation);
	bool LeaveHealRotationMemberPool();

	bool IsHealRotationMember() { return (m_member_of_heal_rotation.use_count() && m_member_of_heal_rotation.get()); }
	bool UseHealRotationFastHeals();
	bool UseHealRotationAdaptiveTargeting();

	bool IsHealRotationActive();
	bool IsHealRotationReady();
	bool IsHealRotationCaster();
	bool HealRotationPokeTarget();
	Mob* HealRotationTarget();
	bool AdvanceHealRotation(bool use_interval = true);

	bool IsMyHealRotationSet();
	bool AmICastingForHealRotation();
	void SetMyCastingForHealRotation(bool flag = true);

	std::shared_ptr<HealRotation>* MemberOfHealRotation() { return &m_member_of_heal_rotation; }

	inline int32	GetSTR()	const override { return STR; }
	inline int32	GetSTA()	const override { return STA; }
	inline int32	GetDEX()	const override { return DEX; }
	inline int32	GetAGI()	const override { return AGI; }
	inline int32	GetINT()	const override { return INT; }
	inline int32	GetWIS()	const override { return WIS; }
	inline int32	GetCHA()	const override { return CHA; }
	inline int32	GetMR()	const override { return MR; }
	inline int32	GetFR()	const override { return FR; }
	inline int32	GetDR()	const override { return DR; }
	inline int32	GetPR()	const override { return PR; }
	inline int32	GetCR()	const override { return CR; }
	inline int32	GetCorrup()	const override { return Corrup; }
	//Heroic
	inline int32	GetHeroicSTR()	const override { return itembonuses.HeroicSTR; }
	inline int32	GetHeroicSTA()	const override { return itembonuses.HeroicSTA; }
	inline int32	GetHeroicDEX()	const override { return itembonuses.HeroicDEX; }
	inline int32	GetHeroicAGI()	const override { return itembonuses.HeroicAGI; }
	inline int32	GetHeroicINT()	const override { return itembonuses.HeroicINT; }
	inline int32	GetHeroicWIS()	const override { return itembonuses.HeroicWIS; }
	inline int32	GetHeroicCHA()	const override { return itembonuses.HeroicCHA; }
	inline int32	GetHeroicMR()	const override { return itembonuses.HeroicMR; }
	inline int32	GetHeroicFR()	const override { return itembonuses.HeroicFR; }
	inline int32	GetHeroicDR()	const override { return itembonuses.HeroicDR; }
	inline int32	GetHeroicPR()	const override { return itembonuses.HeroicPR; }
	inline int32	GetHeroicCR()	const override { return itembonuses.HeroicCR; }
	inline virtual int32	GetHeroicCorrup()	const { return itembonuses.HeroicCorrup; }
	// Mod2
	inline virtual int32	GetShielding()		const { return itembonuses.MeleeMitigation; }
	inline virtual int32	GetSpellShield()	const { return itembonuses.SpellShield; }
	inline virtual int32	GetDoTShield()		const { return itembonuses.DoTShielding; }
	inline virtual int32	GetStunResist()		const { return itembonuses.StunResist; }
	inline virtual int32	GetStrikeThrough()	const { return itembonuses.StrikeThrough; }
	inline virtual int32	GetAvoidance()		const { return itembonuses.AvoidMeleeChance; }
	inline virtual int32	GetAccuracy()		const { return itembonuses.HitChance; }
	inline virtual int32	GetCombatEffects()	const { return itembonuses.ProcChance; }
	inline virtual int32	GetDS()				const { return itembonuses.DamageShield; }
	// Mod3
	inline int32 GetHealAmt() const override { return itembonuses.HealAmt; }
	inline int32 GetSpellDmg() const override { return itembonuses.SpellDmg; }
	inline virtual int32	GetClair()			const { return itembonuses.Clairvoyance; }
	inline virtual int32	GetDSMit()			const { return itembonuses.DSMitigation; }

	inline virtual int32	GetSingMod()		const { return itembonuses.singingMod; }
	inline virtual int32	GetBrassMod()		const { return itembonuses.brassMod; }
	inline virtual int32	GetPercMod()		const { return itembonuses.percussionMod; }
	inline virtual int32	GetStringMod()		const { return itembonuses.stringedMod; }
	inline virtual int32	GetWindMod()		const { return itembonuses.windMod; }

	inline virtual int32	GetDelayDeath()		const { return aabonuses.DelayDeath + spellbonuses.DelayDeath + itembonuses.DelayDeath; }

	inline InspectMessage_Struct& GetInspectMessage() { return _botInspectMessage; }
	inline const InspectMessage_Struct& GetInspectMessage() const { return _botInspectMessage; }

	// "Quest API" Methods
	bool HasBotSpellEntry(uint16 spell_id);
	bool CanUseBotSpell(uint16 spell_id);
	void ApplySpell(int spell_id, int duration = 0, int level = -1, ApplySpellType apply_type = ApplySpellType::Solo, bool allow_pets = false, bool is_raid_group_only = true);
	void BreakInvis();
	void Escape();
	void Fling(float value, float target_x, float target_y, float target_z, bool ignore_los = false, bool clip_through_walls = false, bool calculate_speed = false);
	std::vector<Mob*> GetApplySpellList(ApplySpellType apply_type, bool allow_pets, bool is_raid_group_only);
	int32 GetItemIDAt(int16 slot_id);
	int32 GetAugmentIDAt(int16 slot_id, uint8 augslot);
	int32 GetRawItemAC();
	void SendSpellAnim(uint16 targetid, uint16 spell_id);
	void SetSpellDuration(int spell_id, int duration = 0, int level = -1, ApplySpellType apply_type = ApplySpellType::Solo, bool allow_pets = false, bool is_raid_group_only = true);

	// "SET" Class Methods
	void SetBotSpellID(uint32 newSpellID);
	void SetSpawnStatus(bool spawnStatus) { _spawnStatus = spawnStatus; }
	void SetPetChooserID(uint8 id) { _petChooserID = id; }
	void SetBotRangedSetting(bool value) { _botRangedSetting = value; }
	void SetBotCharmer(bool c) { _botCharmer = c; }
	void SetBotOwner(Mob* botOwner) { this->_botOwner = botOwner; }
	void SetRangerAutoWeaponSelect(bool enable) { GetClass() == Class::Ranger ? _rangerAutoWeaponSelect = enable : _rangerAutoWeaponSelect = false; }
	void SetBotStance(uint8 stance_id) { _botStance = Stance::IsValid(stance_id) ? stance_id : Stance::Passive; }
	uint32 GetSpellRecastTimer(uint16 spell_id = 0);
	bool CheckSpellRecastTimer(uint16 spell_id = 0);
	uint32 GetSpellRecastRemainingTime(uint16 spell_id = 0);
	void SetSpellRecastTimer(uint16 spell_id, int32 recast_delay = 0);
	uint32 CalcSpellRecastTimer(uint16 spell_id);
	uint32 GetDisciplineReuseTimer(uint16 spell_id = 0);
	bool CheckDisciplineReuseTimer(uint16 spell_id = 0);
	uint32 GetDisciplineReuseRemainingTime(uint16 spell_id = 0);
	void SetDisciplineReuseTimer(uint16 spell_id, int32 reuse_timer = 0);
	uint32 GetItemReuseTimer(uint32 item_id = 0);
	bool CheckItemReuseTimer(uint32 item_id = 0);
	void SetItemReuseTimer(uint32 item_id, uint32 reuse_timer = 0);
	void ClearDisciplineReuseTimer(uint16 spell_id = 0);
	void ClearItemReuseTimer(uint32 item_id = 0);
	void ClearSpellRecastTimer(uint16 spell_id = 0);
	uint32 GetItemReuseRemainingTime(uint32 item_id = 0);
	void ClearExpiredTimers();
	void SetBeardColor(uint8 value) { beardcolor = value; }
	void SetBeard(uint8 value) { beard = value; }
	void SetEyeColor1(uint8 value) { eyecolor1 = value; }
	void SetEyeColor2(uint8 value) { eyecolor2 = value; }
	void SetLuclinFace(uint8 value) { luclinface = value; }
	void SetHairColor(uint8 value) { haircolor = value; }
	void SetHairStyle(uint8 value) { hairstyle = value; }
	void SetDrakkinDetails(uint32 value) { drakkin_details = value; }
	void SetDrakkinHeritage(uint32 value) { drakkin_heritage = value; }
	void SetDrakkinTattoo(uint32 value) { drakkin_tattoo = value; }
	bool DyeArmor(int16 slot_id, uint32 rgb, bool all_flag = false, bool save_flag = true);

	int GetExpansionBitmask();
	void SetExpansionBitmask(int expansionBitmask);

	void ListBotSpells(uint8 min_level);

	std::string GetHPString(int8 min_hp, int8 max_hp);

	bool AddBotSpellSetting(uint16 spell_id, BotSpellSetting* bs);
	bool DeleteBotSpellSetting(uint16 spell_id);
	BotSpellSetting* GetBotSpellSetting(uint16 spell_id);
	void ListBotSpellSettings();
	void LoadBotSpellSettings();
	bool UpdateBotSpellSetting(uint16 spell_id, BotSpellSetting* bs);
	void SetBotEnforceSpellSetting(bool enforceSpellSettings);
	bool GetBotEnforceSpellSetting() { return _enforceSpellSettings; }

	// Class Destructors
	~Bot() override;

	// Publicized private functions
	static NPCType *FillNPCTypeStruct(
		uint32 botSpellsID,
		const std::string& botName,
		const std::string& botLastName,
		uint8 botLevel,
		uint16 botRace,
		uint8 botClass,
		uint8 gender,
		float size,
		uint32 face,
		uint32 hairStyle,
		uint32 hairColor,
		uint32 eyeColor,
		uint32 eyeColor2,
		uint32 beard,
		uint32 beardColor,
		uint32 drakkinHeritage,
		uint32 drakkinTattoo,
		uint32 drakkinDetails,
		int32 hp,
		int32 mana,
		int32 mr,
		int32 cr,
		int32 dr,
		int32 fr,
		int32 pr,
		int32 corrup,
		int32 ac,
		uint32 str,
		uint32 sta,
		uint32 dex,
		uint32 agi,
		uint32 _int,
		uint32 wis,
		uint32 cha,
		uint32 attack
	);
	void BotRemoveEquipItem(uint16 slot_id);
	void RemoveBotItemBySlot(uint16 slot_id
);
	void AddBotItem(
		uint16 slot_id,
		uint32 item_id,
		int16 charges = -1,
		bool attuned = false,
		uint32 augment_one = 0,
		uint32 augment_two = 0,
		uint32 augment_three = 0,
		uint32 augment_four = 0,
		uint32 augment_five = 0,
		uint32 augment_six = 0
	);
	uint32 CountBotItem(uint32 item_id);
	std::map<uint16, uint32> GetBotItemSlots();
	uint32 GetBotItemBySlot(uint16 slot_id);
	int16 HasBotItem(uint32 item_id);
	void RemoveBotItem(uint32 item_id);
	uint32 GetTotalPlayTime();

	// New accessors for BotDatabase access
	bool DeleteBot();
	std::vector<BotTimer> GetBotTimers() { return bot_timers; }
	uint32 GetLastZoneID() const { return _lastZoneId; }
	int32 GetBaseAC() const { return _baseAC; }
	int32 GetBaseATK() const { return _baseATK; }
	int32 GetBaseSTR() const { return _baseSTR; }
	int32 GetBaseSTA() const { return _baseSTA; }
	int32 GetBaseCHA() const { return _baseCHA; }
	int32 GetBaseDEX() const { return _baseDEX; }
	int32 GetBaseINT() const { return _baseINT; }
	int32 GetBaseAGI() const { return _baseAGI; }
	int32 GetBaseWIS() const { return _baseWIS; }
	int32 GetBaseFR() const { return _baseFR; }
	int32 GetBaseCR() const { return _baseCR; }
	int32 GetBaseMR() const { return _baseMR; }
	int32 GetBasePR() const { return _basePR; }
	int32 GetBaseDR() const { return _baseDR; }
	int32 GetBaseCorrup() const { return _baseCorrup; }

	void Signal(int signal_id);
	void SendPayload(int payload_id, std::string payload_value = std::string());
	void OwnerMessage(const std::string& message);

	//Raid additions
	Raid* p_raid_instance;

	static uint8 spell_casting_chances[SPELL_TYPE_COUNT][Class::PLAYER_CLASS_COUNT][Stance::AEBurn][cntHSND];

	bool BotCastMez(Mob* tar, uint8 bot_class, BotSpell& bot_spell, uint16 spell_type);
	bool BotCastHeal(Mob* tar, uint8 bot_class, BotSpell& bot_spell, uint16 spell_type);
	bool BotCastNuke(Mob* tar, uint8 bot_class, BotSpell& bot_spell, uint16 spell_type);
	bool BotCastPet(Mob* tar, uint8 bot_class, BotSpell& bot_spell, uint16 spell_type);
	bool BotCastCure(Mob* tar, uint8 bot_class, BotSpell& bot_spell, uint16 spell_type);

	bool CheckIfIncapacitated();
	bool IsAIProcessValid(const Client* bot_owner, const Group* bot_group, const Raid* raid);

	Client* SetLeashOwner(Client* bot_owner, Group* bot_group, Raid* raid, uint32 r_group) const;
	Mob* SetFollowMob(Client* leash_owner);

	Mob* GetBotTarget(Client* bot_owner);
	void SetOwnerTarget(Client* bot_owner);
	bool IsValidTarget(
		Client* bot_owner,
		Client* leash_owner,
		float lo_distance,
		float leash_distance,
		Mob* tar,
		float tar_distance
	);
	bool TargetValidation(Mob* other);

	bool PullingFlagChecks(Client* bot_owner);
	bool ReturningFlagChecks(Client* bot_owner, Mob* leash_owner, float fm_distance);
	void BotPullerProcess(Client* bot_owner, Raid* raid);

	// Combat Checks
	CombatRangeOutput EvaluateCombatRange(const CombatRangeInput& input);

	void SetBerserkState();
	bool CheckIfCasting(float fm_distance);
	void HealRotationChecks();

	void SetCombatJitter();
	bool DoCombatPositioning(const CombatPositioningInput& input);
	void RunToGoalWithJitter(glm::vec3 Goal);
	bool RequiresLoSForPositioning();
	bool HasRequiredLoSForPositioning(Mob* tar);

	// Try Combat Methods
	bool TryEvade(Mob* tar);
	bool TryFacingTarget(Mob* tar);
	bool TryPursueTarget(float leash_distance);
	bool TryMeditate();
	bool TryAutoDefend(Client* bot_owner, float leash_distance);
	bool TryIdleChecks(float fm_distance);
	bool TryNonCombatMovementChecks(Client* bot_owner, const Mob* follow_mob);
	void DoOutOfCombatChecks(Client* bot_owner, Mob* follow_mob, float leash_distance, float fm_distance);
	bool TryBardMovementCasts();
	bool BotRangedAttack(Mob* other, bool can_double_attack = false);
	bool CheckDoubleRangedAttack();

	// Public "Refactor" Methods
	static bool CheckCampSpawnConditions(Client* c);
	static bool CheckHighEnoughLevelForBots(Client* c, uint8 bot_class = Class::None);
	static bool CheckCreateLimit(Client* c, uint32 bot_count, uint8 bot_class = Class::None);
	static bool CheckSpawnLimit(Client* c, uint8 bot_class = Class::None);

protected:
	void BotMeditate(bool is_sitting);
	bool CheckBotDoubleAttack(bool triple_attack = false);
	bool CheckTripleAttack();
	void PerformTradeWithClient(int16 begin_slot_id, int16 end_slot_id, Client* client, int16 chosen_slot = INVALID_INDEX);
	bool AIDoSpellCast(int32 i, Mob* tar, int32 mana_cost, uint32* oDontDoAgainBefore = nullptr) override;

	BotCastingRoles& GetCastingRoles() { return m_CastingRoles; }
	void SetGroupHealer(bool flag = true) { m_CastingRoles.GroupHealer = flag; }
	void SetGroupSlower(bool flag = true) { m_CastingRoles.GroupSlower = flag; }
	void SetGroupNuker(bool flag = true) { m_CastingRoles.GroupNuker = flag; }
	void SetGroupDoter(bool flag = true) { m_CastingRoles.GroupDoter = flag; }
	std::deque<int> bot_signal_q;

	std::vector<BotSpells> AIBot_spells;
	std::vector<BotSpells> AIBot_spells_enforced;
	std::unordered_map<uint16, std::vector<BotSpells_wIndex>> AIBot_spells_by_type;

	std::vector<BotTimer> bot_timers;
	std::vector<BotBlockedBuffs> bot_blocked_buffs;

private:
	// Class Members
	uint32 _botID;
	uint32 _botOwnerCharacterID;
	bool _spawnStatus;
	Mob* _botOwner;
	bool _botCharmer;
	uint8 _petChooserID;
	bool berserk;
	EQ::InventoryProfile m_inv;
	double _lastTotalPlayTime;
	time_t _startTotalPlayTime;
	uint32 _guildId;
	uint8 _guildRank;
	std::string _guildName;
	std::string _surname;
	std::string _title;
	std::string _suffix;
	uint32 _lastZoneId;
	bool _rangerAutoWeaponSelect;
	uint8 _botStance;
	unsigned int RestRegenHP;
	unsigned int RestRegenMana;
	unsigned int RestRegenEndurance;
	Timer rest_timer;
	Timer m_ping_timer;
	int32	base_end;
	int32	cur_end;
	int32	max_end;
	int32	end_regen;

	Timer m_rogue_evade_timer; // Rogue evade timer
	Timer m_monk_evade_timer; // Monk evade FD timer
	Timer m_auto_defend_timer;
	Timer m_auto_save_timer;

	Timer m_combat_jitter_timer;

	bool m_dirtyautohaters;
	bool m_guard_flag;
	bool m_hold_flag;
	bool m_attack_flag;
	bool m_combat_round_alert_flag;
	bool m_attacking_flag;
	bool m_pull_flag;
	bool m_pulling_flag;
	bool m_returning_flag;
	bool is_using_item_click;

	BotCastingRoles m_CastingRoles;

	std::map<uint16, BotSpellSetting> bot_spell_settings;

	std::shared_ptr<HealRotation> m_member_of_heal_rotation;

	InspectMessage_Struct _botInspectMessage;
	bool _pauseAI;

	int _expansionBitmask;
	bool _enforceSpellSettings;
	bool _showHelm;
	bool _botRangedSetting;
	uint8 _stopMeleeLevel;
	uint32 _distanceRanged;
	bool _behindMobStatus;
	bool _maxMeleeRangeStatus;
	bool _medInCombat;
	uint8 _SitHPPct;
	uint8 _SitManaPct;
	uint16 _castedSpellType;
	bool _hasLoS;
	bool _commandedSpell;
	bool _pullingSpell;
	
	bool _illusionBlock;
	std::vector<BotSpellSettings> m_bot_spell_settings;
	std::vector<Mob*> _spell_target_list;
	std::vector<Mob*> _group_spell_target_list;
	Raid* _storedRaid;
	bool _verifiedRaid;
	uint16 _tempSpellType; // this is used to check the spell type being cast against ^spellannouncecasts status

	// Private "base stats" Members
	int32 _baseMR;
	int32 _baseCR;
	int32 _baseDR;
	int32 _baseFR;
	int32 _basePR;
	int32 _baseCorrup;
	int32 _baseAC;
	int32 _baseSTR;
	int32 _baseSTA;
	int32 _baseDEX;
	int32 _baseAGI;
	int32 _baseINT;
	int32 _baseWIS;
	int32 _baseCHA;
	int32 _baseATK;
	uint16 _baseRace;	// Necessary to preserve the race otherwise bots get their race updated in the db when they get an illusion.
	uint8 _baseGender;	// Bots gender. Necessary to preserve the original value otherwise it can be changed by illusions.

	// Class Methods
	void LoadAAs();
	void GenerateBaseStats();
	void GenerateAppearance();
	int32 GenerateBaseHitPoints();
	int32 GenerateBaseManaPoints();
	void GenerateSpecialAttacks();
	void SetBotID(uint32 botID);
	void SetCombatRoundForAlerts(bool flag = true) { m_combat_round_alert_flag = flag; }
	void SetAttackingFlag(bool flag = true) { m_attacking_flag = flag; }
	void SetPullingFlag(bool flag = true) { m_pulling_flag = flag; }
	void SetReturningFlag(bool flag = true) { m_returning_flag = flag; }

	// Private "Inventory" Methods
	void GetBotItems(EQ::InventoryProfile &inv);
	void BotAddEquipItem(uint16 slot_id, uint32 item_id);

	// Private "Pet" Methods
	bool LoadPet();	// Load and spawn bot pet if there is one
	bool SavePet();	// Save and depop bot pet if there is one
	bool DeletePet();

	public:

	int32 CalcItemATKCap() final;
};

bool IsSpellInBotList(DBbotspells_Struct* spell_list, uint16 spell_id);
