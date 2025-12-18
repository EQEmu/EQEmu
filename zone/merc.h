#pragma once

#include "zone/npc.h"

class Client;
class Corpse;
class Group;
class Mob;
class Raid;
struct MercTemplate;
struct NPCType;
struct NewSpawn_Struct;

namespace EQ
{
	struct ItemData;
}

#define MAXMERCS 1
#define TANK 1
#define HEALER 2
#define MELEEDPS 9
#define CASTERDPS 12

#define NO_MERC_ID 0
#define MERC_STATE_NORMAL 5
#define MERC_STATE_SUSPENDED 1
#define NOT_SUSPENDED_TIME 0

const int MercAISpellRange = 100; // TODO: Write a method that calcs what the merc's spell range is based on spell, equipment, AA, whatever and replace this

struct MercSpell {
	uint16 spellid; // <= 0 = no spell
	uint32 type; // 0 = never, must be one (and only one) of the defined values
	int16 stance; // 0 = all, + = only this stance, - = all except this stance
	int16 slot;
	uint16 proc_chance;
	uint32 time_cancast; // when we can cast this spell next
};

struct MercTimer {
	uint16 timerid; // EndurTimerIndex
	uint8 timertype; // 1 = spell, 2 = disc
	uint16 spellid; // <= 0 = no spell
	uint32 time_cancast; // when we can cast this spell next
};

class Merc : public NPC {
public:
	Merc(const NPCType* d, float x, float y, float z, float heading);
	virtual ~Merc();

	//abstract virtual function implementations requird by base abstract class
	virtual bool Death(Mob* killer_mob, int64 damage, uint16 spell_id, EQ::skills::SkillType attack_skill, uint8 killed_by = 0, bool is_buff_tic = false);
	virtual void Damage(Mob* from, int64 damage, uint16 spell_id, EQ::skills::SkillType attack_skill, bool avoidable = true, int8 buffslot = -1, bool iBuffTic = false, eSpecialAttacks special = eSpecialAttacks::None);
	virtual bool Attack(Mob* other, int Hand = EQ::invslot::slotPrimary, bool FromRiposte = false, bool IsStrikethrough = false,
	bool IsFromSpell = false, ExtraAttackOptions *opts = nullptr);
	virtual bool HasRaid() { return false; }
	virtual bool HasGroup() { return (GetGroup() ? true : false); }
	virtual Raid* GetRaid() { return 0; }
	virtual Group* GetGroup() { return entity_list.GetGroupByMob(this); }

	// Mob AI Virtual Override Methods
	virtual void AI_Start(int32 iMoveDelay = 0);
	virtual void AI_Stop();
	virtual void AI_Process();

	//virtual bool AICastSpell(Mob* tar, int8 iChance, uint32 iSpellTypes);
	virtual bool AICastSpell(int8 iChance, uint32 iSpellTypes);
	virtual bool AIDoSpellCast(uint16 spellid, Mob* tar, int32 mana_cost, uint32* oDontDoAgainBefore = 0);
	virtual bool AI_EngagedCastCheck();
	//virtual bool AI_PursueCastCheck();
	virtual bool AI_IdleCastCheck();

	virtual bool Process();

	// Static Merc Group Methods
	static bool AddMercToGroup(Merc* merc, Group* group);
	static bool RemoveMercFromGroup(Merc* merc, Group* group);
	void ProcessClientZoneChange(Client* mercOwner);
	static void MercGroupSay(Mob *speaker, const char *msg, ...);
	Corpse* GetGroupMemberCorpse();

	// Merc Spell Casting Methods
	int8 GetChanceToCastBySpellType(uint32 spellType);
	void SetSpellRecastTimer(uint16 timer_id, uint16 spellid, uint32 recast_delay);
	void SetDisciplineRecastTimer(uint16 timer_id, uint16 spellid, uint32 recast_delay);
	void SetSpellTimeCanCast(uint16 spellid, uint32 recast_delay);
	static int32 GetSpellRecastTimer(Merc *caster, uint16 timer_id);
	static bool CheckSpellRecastTimers(Merc *caster, uint16 spellid);
	static int32 GetDisciplineRecastTimer(Merc *caster, uint16 timer_id);
	static bool CheckDisciplineRecastTimers(Merc *caster, uint16 spell_id, uint16 timer_id);
	static int32 GetDisciplineRemainingTime(Merc *caster, uint16 timer_id);
	static std::list<MercSpell> GetMercSpellsForSpellEffect(Merc* caster, int spellEffect);
	static std::list<MercSpell> GetMercSpellsForSpellEffectAndTargetType(Merc* caster, int spellEffect, SpellTargetType targetType);
	static std::list<MercSpell> GetMercSpellsBySpellType(Merc* caster, uint32 spellType);
	static MercSpell GetFirstMercSpellBySpellType(Merc* caster, uint32 spellType);
	static MercSpell GetFirstMercSpellForSingleTargetHeal(Merc* caster);
	static MercSpell GetMercSpellBySpellID(Merc* caster, uint16 spellid);
	static MercSpell GetBestMercSpellForVeryFastHeal(Merc* caster);
	static MercSpell GetBestMercSpellForFastHeal(Merc* caster);
	static MercSpell GetBestMercSpellForHealOverTime(Merc* caster);
	static MercSpell GetBestMercSpellForPercentageHeal(Merc* caster);
	static MercSpell GetBestMercSpellForRegularSingleTargetHeal(Merc* caster);
	static MercSpell GetBestMercSpellForGroupHealOverTime(Merc* caster);
	static MercSpell GetBestMercSpellForGroupCompleteHeal(Merc* caster);
	static MercSpell GetBestMercSpellForGroupHeal(Merc* caster);
	static MercSpell GetBestMercSpellForAETaunt(Merc* caster);
	static MercSpell GetBestMercSpellForTaunt(Merc* caster);
	static MercSpell GetBestMercSpellForHate(Merc* caster);
	static MercSpell GetBestMercSpellForCure(Merc* caster, Mob* target);
	static MercSpell GetBestMercSpellForStun(Merc* caster);
	static MercSpell GetBestMercSpellForAENuke(Merc* caster, Mob* target);
	static MercSpell GetBestMercSpellForTargetedAENuke(Merc* caster, Mob* target);
	static MercSpell GetBestMercSpellForPBAENuke(Merc* caster, Mob* target);
	static MercSpell GetBestMercSpellForAERainNuke(Merc* caster, Mob* target);
	static MercSpell GetBestMercSpellForNuke(Merc* caster);
	static MercSpell GetBestMercSpellForNukeByTargetResists(Merc* caster, Mob* target);
	static bool CheckAENuke(Merc* caster, Mob* tar, uint16 spell_id, uint8 &numTargets);
	static bool GetNeedsCured(Mob *tar);
	bool HasOrMayGetAggro();
	bool UseDiscipline(int32 spell_id, int32 target);

	bool IsMerc() const override { return true; }
	bool IsOfClientBotMerc() const override { return true; }

	virtual void FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho);
	static Merc* LoadMercenary(Client *c, MercTemplate* merc_template, uint32 merchant_id, bool updateFromDB = false);
	void UpdateMercInfo(Client *c);
	void UpdateMercStats(Client *c, bool setmax = false);
	void UpdateMercAppearance();
	virtual void UpdateEquipmentLight();
	void AddItem(uint8 slot, uint32 item_id);
	static const char *GetRandomName();
	bool Spawn(Client *owner);
	bool Suspend();
	bool Unsuspend(bool setMaxStats);
	bool MercJoinClientGroup();
	void Zone();
	virtual void Depop();
	virtual bool Save();
	bool GetDepop() { return p_depop; }

	bool IsDead() { return GetHP() < 0;};
	bool IsMedding() { return _medding; };
	bool IsSuspended() { return _suspended; };

	void Signal(int signal_id);
	void SendPayload(int payload_id, std::string payload_value);

	static uint32 CalcPurchaseCost( uint32 templateID , uint8 level, uint8 currency_type = 0);
	static uint32 CalcUpkeepCost( uint32 templateID , uint8 level, uint8 currency_type = 0);

	// "GET" Class Methods
	virtual Mob* GetOwner();
	Client* GetMercenaryOwner();
	virtual Mob* GetOwnerOrSelf();
	uint32 GetMercenaryID() { return _MercID; }
	uint32 GetMercenaryCharacterID( ) { return owner_char_id; }
	uint32 GetMercenaryTemplateID() { return _MercTemplateID; }
	uint32 GetMercenaryType() { return _MercType; }
	uint32 GetMercenarySubType() { return _MercSubType; }
	uint8 GetProficiencyID() { return _ProficiencyID; }
	uint8 GetTierID() { return _TierID; }
	uint32 GetCostFormula() { return _CostFormula; }
	uint32 GetMercNameType() { return _NameType; }
	uint8 GetStance() { return _currentStance; }
	int GetHatedCount() { return _hatedCount; }

	inline const uint8 GetClientVersion() const { return _OwnerClientVersion; }

	virtual void SetTarget(Mob* mob);
	bool HasSkill(EQ::skills::SkillType skill_id) const;
	bool CanHaveSkill(EQ::skills::SkillType skill_id) const;
	uint16 MaxSkill(EQ::skills::SkillType skillid, uint16 class_, uint16 level) const;
	inline uint16 MaxSkill(EQ::skills::SkillType skillid) const { return MaxSkill(skillid, GetClass(), GetLevel()); }
	virtual void DoClassAttacks(Mob *target);
	void CheckHateList();
	bool CheckTaunt();
	bool CheckAETaunt();
	bool CheckConfidence();
	bool TryHide();

	// stat functions
	virtual void ScaleStats(int scalepercent, bool setmax = false);
	virtual void CalcBonuses();
	int64 GetEndurance() const {return cur_end;} //This gets our current endurance
	inline uint8 GetEndurancePercent() { return (uint8)((float)cur_end / (float)max_end * 100.0f); }
	inline virtual int32 GetATK() const { return ATK; }
	inline virtual int32 GetATKBonus() const { return itembonuses.ATK + spellbonuses.ATK; }
	int32 GetRawACNoShield(int &shield_ac) const;

	inline virtual int32 GetSTR() const { return STR; }
	inline virtual int32 GetSTA() const { return STA; }
	inline virtual int32 GetDEX() const { return DEX; }
	inline virtual int32 GetAGI() const { return AGI; }
	inline virtual int32 GetINT() const { return INT; }
	inline virtual int32 GetWIS() const { return WIS; }
	inline virtual int32 GetCHA() const { return CHA; }
	inline virtual int32 GetMR() const { return MR; }
	inline virtual int32 GetFR() const { return FR; }
	inline virtual int32 GetDR() const { return DR; }
	inline virtual int32 GetPR() const { return PR; }
	inline virtual int32 GetCR() const { return CR; }
	inline virtual int32 GetCorrup() const { return Corrup; }

	inline virtual int32 GetHeroicSTR() const { return itembonuses.HeroicSTR; }
	inline virtual int32 GetHeroicSTA() const { return itembonuses.HeroicSTA; }
	inline virtual int32 GetHeroicDEX() const { return itembonuses.HeroicDEX; }
	inline virtual int32 GetHeroicAGI() const { return itembonuses.HeroicAGI; }
	inline virtual int32 GetHeroicINT() const { return itembonuses.HeroicINT; }
	inline virtual int32 GetHeroicWIS() const { return itembonuses.HeroicWIS; }
	inline virtual int32 GetHeroicCHA() const { return itembonuses.HeroicCHA; }
	inline virtual int32 GetHeroicMR() const { return itembonuses.HeroicMR; }
	inline virtual int32 GetHeroicFR() const { return itembonuses.HeroicFR; }
	inline virtual int32 GetHeroicDR() const { return itembonuses.HeroicDR; }
	inline virtual int32 GetHeroicPR() const { return itembonuses.HeroicPR; }
	inline virtual int32 GetHeroicCR() const { return itembonuses.HeroicCR; }
	inline virtual int32 GetHeroicCorrup() const { return itembonuses.HeroicCorrup; }
	// Mod2
	inline virtual int32 GetShielding() const { return itembonuses.MeleeMitigation; }
	inline virtual int32 GetSpellShield() const { return itembonuses.SpellShield; }
	inline virtual int32 GetDoTShield() const { return itembonuses.DoTShielding; }
	inline virtual int32 GetStunResist() const { return itembonuses.StunResist; }
	inline virtual int32 GetStrikeThrough() const { return itembonuses.StrikeThrough; }
	inline virtual int32 GetAvoidance() const { return itembonuses.AvoidMeleeChance; }
	inline virtual int32 GetAccuracy() const { return itembonuses.HitChance; }
	inline virtual int32 GetCombatEffects() const { return itembonuses.ProcChance; }
	inline virtual int32 GetDS() const { return itembonuses.DamageShield; }
	// Mod3
	inline int32 GetHealAmt() const override { return itembonuses.HealAmt; }
	inline int32 GetSpellDmg() const override { return itembonuses.SpellDmg; }
	inline virtual int32 GetClair() const { return itembonuses.Clairvoyance; }
	inline virtual int32 GetDSMit() const { return itembonuses.DSMitigation; }

	inline virtual int32 GetSingMod() const { return itembonuses.singingMod; }
	inline virtual int32 GetBrassMod() const { return itembonuses.brassMod; }
	inline virtual int32 GetPercMod() const { return itembonuses.percussionMod; }
	inline virtual int32 GetStringMod() const { return itembonuses.stringedMod; }
	inline virtual int32 GetWindMod() const { return itembonuses.windMod; }

	inline virtual int32 GetDelayDeath() const { return aabonuses.DelayDeath + spellbonuses.DelayDeath + itembonuses.DelayDeath; }

	// "SET" Class Methods
	void SetMercData (uint32 templateID );
	void SetMercID( uint32 mercID ) { _MercID = mercID; }
	void SetMercCharacterID( uint32 ownerID ) { owner_char_id = ownerID; }
	void SetMercTemplateID( uint32 templateID ) { _MercTemplateID = templateID; }
	void SetMercType( uint32 type ) { _MercType = type; }
	void SetMercSubType( uint32 subtype ) { _MercSubType = subtype; }
	void SetProficiencyID( uint8 proficiency_id ) { _ProficiencyID = proficiency_id; }
	void SetTierID( uint8 tier_id ) { _TierID = tier_id; }
	void SetCostFormula( uint8 costformula ) { _CostFormula = costformula; }
	void SetMercNameType( uint8 nametype ) { _NameType = nametype; }
	void SetClientVersion(uint8 clientVersion) { _OwnerClientVersion = clientVersion; }
	void SetSuspended(bool suspended) { _suspended = suspended; }
	void SetStance(uint8 stance_id) { _currentStance = stance_id; }
	void SetHatedCount( int count ) { _hatedCount = count; }

	void Sit();
	void Stand();
	bool IsSitting() const override;
	bool IsStanding();

	// Merc-specific functions
	bool IsMercCaster() { return (GetClass() == HEALER || GetClass() == CASTERDPS); }
	bool IsMercCasterCombatRange(Mob *target);
	virtual float GetMaxMeleeRangeToTarget(Mob* target);
	virtual void MercMeditate(bool isSitting);
	bool FindTarget();

protected:
	int64 GetFocusEffect(focusType type, uint16 spell_id, bool from_buff_tic = false);

	std::vector<MercSpell> merc_spells;
	std::map<uint32,MercTimer> timers;

	Timer evade_timer; // can be moved to pTimers at some point

	uint16 skills[EQ::skills::HIGHEST_SKILL + 1];
	uint32 equipment[EQ::invslot::EQUIPMENT_COUNT]; //this is an array of item IDs
	uint32 d_melee_texture1; //this is an item Material value
	uint32 d_melee_texture2; //this is an item Material value (offhand)
	uint8 prim_melee_type; //Sets the Primary Weapon attack message and animation
	uint8 sec_melee_type; //Sets the Secondary Weapon attack message and animation

private:

	int32 CalcAC();
	int32 CalcATK();
	//int CalcHaste();

	int32 CalcSTR();
	int32 CalcSTA();
	int32 CalcDEX();
	int32 CalcAGI();
	int32 CalcINT();
	int32 CalcWIS();
	int32 CalcCHA();

	int32 CalcMR();
	int32 CalcFR();
	int32 CalcDR();
	int32 CalcPR();
	int32 CalcCR();
	int32 CalcCorrup();
	int64 CalcMaxHP();
	int64 CalcBaseHP();
	int64 CalcHPRegen();
	int64 CalcHPRegenCap();
	int64 CalcMaxMana();
	int64 CalcBaseMana();
	int64 CalcManaRegen();
	int64 CalcBaseManaRegen();
	int64 CalcManaRegenCap();
	void CalcMaxEndurance(); //This calculates the maximum endurance we can have
	int64 CalcBaseEndurance(); //Calculates Base End
	int64 GetMaxEndurance() const {return max_end;} //This gets our endurance from the last CalcMaxEndurance() call
	int64 CalcEnduranceRegen(); //Calculates endurance regen used in DoEnduranceRegen()
	int64 CalcEnduranceRegenCap();
	void SetEndurance(int32 newEnd); //This sets the current endurance to the new value
	void DoEnduranceUpkeep(); //does the endurance upkeep
	void CalcRestState();

	int GroupLeadershipAAHealthEnhancement();
	int GroupLeadershipAAManaEnhancement();
	int GroupLeadershipAAHealthRegeneration();
	int GroupLeadershipAAOffenseEnhancement();

	float GetDefaultSize();

	bool LoadMercenarySpells();
	bool CheckStance(int16 stance);
	std::vector<MercSpell> GetMercSpells() { return merc_spells; }

	// Private "base stats" Members
	int32 base_mana;
	int32 base_end;
	int32 _baseAC;
	uint32 _baseSTR;
	uint32 _baseSTA;
	uint32 _baseDEX;
	uint32 _baseAGI;
	uint32 _baseINT;
	uint32 _baseWIS;
	uint32 _baseCHA;
	uint32 _baseATK;
	uint32 _baseRace; // Necessary to preserve the race otherwise mercs get their race updated in the db when they get an illusion.
	uint8 _baseGender; // Merc gender. Necessary to preserve the original value otherwise it can be changed by illusions.
	uint32 _baseMR;
	uint32 _baseCR;
	uint32 _baseDR;
	uint32 _baseFR;
	uint32 _basePR;
	uint32 _baseCorrup;
	uint32 RestRegenHP;
	uint32 RestRegenMana;
	uint32 RestRegenEndurance;

	uint32 _MercID;
	uint32 _MercTemplateID;
	uint32 _MercType;
	uint32 _MercSubType;
	uint8 _ProficiencyID;
	uint8 _TierID;
	uint8 _CostFormula;
	uint8 _NameType;
	uint8 _OwnerClientVersion;
	uint8 _currentStance;

	EQ::InventoryProfile m_inv;
	int64 max_end;
	int64 cur_end;
	bool _medding;
	bool _suspended;
	bool p_depop;
	bool _check_confidence;
	bool _lost_confidence;
	int _hatedCount;
	uint32 owner_char_id;
	Timer endupkeep_timer;
	Timer rest_timer;
	Timer confidence_timer;
	Timer check_target_timer;
};
