
#pragma once

#include "common/types.h"
#include "common/spdat.h"
#include "common/emu_constants.h"

#include "cereal/cereal.hpp"

#define	HIGHEST_RESIST 9 //Max resist type value
#define MAX_SPELL_PROJECTILE 10 //Max amount of spell projectiles that can be active by a single mob.

/* macros for IsAttackAllowed, IsBeneficialAllowed */
#define _CLIENT(x) (x && x->IsClient() && !x->CastToClient()->IsBecomeNPC())
#define _NPC(x) (x && x->IsNPC() && !x->CastToMob()->GetOwnerID())
#define _BECOMENPC(x) (x && x->IsClient() && x->CastToClient()->IsBecomeNPC())
#define _CLIENTCORPSE(x) (x && x->IsCorpse() && x->CastToCorpse()->IsPlayerCorpse() && !x->CastToCorpse()->IsBecomeNPCCorpse())
#define _NPCCORPSE(x) (x && x->IsCorpse() && (x->CastToCorpse()->IsNPCCorpse() || x->CastToCorpse()->IsBecomeNPCCorpse()))
#define _CLIENTPET(x) (x && x->CastToMob()->GetOwner() && x->CastToMob()->GetOwner()->IsClient())

//LOS Parameters:
#define HEAD_POSITION 0.9f	//ratio of GetSize() where NPCs see from
#define SEE_POSITION 0.5f	//ratio of GetSize() where NPCs try to see for LOS

namespace Archetype {
	constexpr uint8 Hybrid = 1;
	constexpr uint8 Caster = 2;
	constexpr uint8 Melee  = 3;
};

#define DMG_BLOCKED		-1
#define DMG_PARRIED		-2
#define DMG_RIPOSTED		-3
#define DMG_DODGED		-4
#define DMG_INVULNERABLE	-5
#define DMG_RUNE		-6

//Spell specialization parameters, not sure of a better place for them
#define SPECIALIZE_FIZZLE 11		//% fizzle chance reduce at 200 specialized

//these are large right now because the x,y,z coords of the zone
//lines do not make a lot of sense
//Maximum distance from a zone point given that the request didnt
//know what zone that the line was for
#define ZONEPOINT_NOZONE_RANGE 40000.0f
//Maximum distance from a zone point if zone was specified
#define ZONEPOINT_ZONE_RANGE 40000.0f

#define AURA_HARDCAP		2
#define WEAPON_STANCE_TYPE_MAX 2


#define SHIELD_ABILITY_RECAST_TIME 180

typedef enum {	//focus types
	focusSpellHaste = 1,				//@Fc, SPA: 127, SpellEffect::IncreaseSpellHaste,				On Caster, cast time mod pct, base: pct
	focusSpellDuration,					//@Fc, SPA: 128, SpellEffect::IncreaseSpellDuration,			On Caster, spell duration mod pct, base: pct
	focusRange,							//@Fc, SPA: 129, SpellEffect::IncreaseRange,					On Caster, spell range mod pct, base: pct
	focusReagentCost,					//@Fc, SPA: 131, SpellEffect::ReduceReagentCost,				On Caster, do not consume reagent pct chance, base: min pct, limit: max pct
	focusManaCost,						//@Fc, SPA: 132, SpellEffect::ReduceManaCost,					On Caster, reduce mana cost by pct, base: min pct, limt: max pct
	focusImprovedHeal,					//@Fc, SPA: 125, SpellEffect::ImprovedHeal,					On Caster, spell healing mod pct, base: min pct, limit: max pct
	focusImprovedDamage,				//@Fc, SPA: 124, SpellEffect::ImprovedDamage,					On Caster, spell damage mod pct, base: min pct, limit: max pct
	focusImprovedDamage2,				//@Fc, SPA: 461, SpellEffect::ImprovedDamage2,				On Caster, spell damage mod pct, base: min pct, limit: max pct
	focusFcDamagePctCrit,				//@Fc, SPA: 302, SpellEffect::FcDamagePctCrit,				On Caster, spell damage mod pct, base: min pct, limit: max pct
	focusPetPower,						//@Fc, SPA: 167, SpellEffect::PetPowerIncrease,				On Caster, pet power mod, base: value
	focusResistRate,					//@Fc, SPA: 126, SpellEffect::SpellResistReduction,			On Caster, casted spell resist mod pct, base: min pct, limit: max pct
	focusSpellHateMod,					//@Fc, SPA: 130, SpellEffect::SpellHateMod,					On Caster, spell hate mod pct, base: min pct, limit: max pct
	focusTriggerOnCast,					//@Fc, SPA: 339, SpellEffect::TriggerOnCast,					On Caster, cast on spell use, base: chance pct limit: spellid
	focusSpellVulnerability,			//@Fc, SPA: 296, SpellEffect::FcSpellVulnerability,			On Target, spell damage taken mod pct, base: min pct, limit: max pct
	focusFcSpellDamagePctIncomingPC,	//@Fc, SPA: 483, SpellEffect::Fc_Spell_Damage_Pct_IncomingPC, On Target, spell damage taken mod pct, base: min pct, limit: max pct
	focusTwincast,						//@Fc, SPA: 399, SpellEffect::FcTwincast,						On Caster, chance cast spell twice, base: chance pct
	focusSympatheticProc,				//@Fc, SPA: 383, SpellEffect::SympatheticProc,				On Caster, cast on spell use, base: variable proc chance on cast time, limit: spellid
	focusFcDamageAmt,					//@Fc, SPA: 286, SpellEffect::FcDamageAmt,					On Caster, spell damage mod flat amt, base: amt
	focusFcDamageAmt2,					//@Fc, SPA: 462, SpellEffect::FcDamageAmt2,					On Caster, spell damage mod flat amt, base: amt
	focusFcDamageAmtCrit,				//@Fc, SPA: 303, SpellEffect::FFcDamageAmtCrit,				On Caster, spell damage mod flat amt, base: amt
	focusSpellDurByTic,					//@Fc, SPA: 287, SpellEffect::SpellDurationIncByTic,			On Caster, spell buff duration mod, base: tics
	focusSwarmPetDuration,				//@Fc, SPA: 398, SpellEffect::SwarmPetDuration,				On Caster, swarm pet duration mod, base: milliseconds
	focusReduceRecastTime,				//@Fc, SPA: 310, SpellEffect::ReduceReuseTimer,				On Caster, disc reuse time mod, base: milliseconds
	focusBlockNextSpell,				//@Fc, SPA: 335, SpellEffect::BlockNextSpellFocus,			On Caster, chance to block next spell, base: chance
	focusFcHealPctIncoming,				//@Fc, SPA: 393, SpellEffect::FcHealPctIncoming,   			On Target, heal received mod pct, base: pct
	focusFcDamageAmtIncoming,			//@Fc, SPA: 297, SpellEffect::FcDamageAmtIncoming,			On Target, damage taken flat amt, base: amt
	focusFcSpellDamageAmtIncomingPC,	//@Fc, SPA: 484, SpellEffect::Fc_Spell_Damage_Amt_IncomingPC,	On Target, damage taken flat amt, base: amt
	focusFcCastSpellOnLand,				//@Fc, SPA: 481, SpellEffect::Fc_Cast_Spell_On_Land,			On Target, cast spell if hit by spell, base: chance pct, limit: spellid
	focusFcHealAmtIncoming,				//@Fc, SPA: 394, SpellEffect::FcHealAmtIncoming,				On Target, heal received mod flat amt, base: amt
	focusFcBaseEffects,					//@Fc, SPA: 413, SpellEffect::FcBaseEffects,					On Caster, base spell effectiveness mod pct, base: pct
	focusIncreaseNumHits,				//@Fc, SPA: 421, SpellEffect::FcIncreaseNumHits,				On Caster, numhits mod flat amt, base: amt
	focusFcLimitUse,					//@Fc, SPA: 420, SpellEffect::FcLimitUse,						On Caster, numhits mod pct, base: pct
	focusFcMute,						//@Fc, SPA: 357, SpellEffect::FcMute,							On Caster, prevents spell casting, base: chance pct
	focusFcTimerRefresh,				//@Fc, SPA: 389, SpellEffect::FcTimerRefresh,					On Caster, reset spell recast timer, base: 1
	focusFcTimerLockout,				//@Fc, SPA: 390, SpellEffect::FcTimerLockout,					On Caster, set a spell to be on recast timer, base: recast duration milliseconds
	focusFcStunTimeMod,					//@Fc, SPA: 133, SpellEffect::FcStunTimeMod,					On Caster, stun time mod pct, base: chance pct
	focusFcResistIncoming,				//@Fc, SPA: 510, SpellEffect::Fc_Resist_Incoming,				On Target, resist modifier, base: amt
	focusFcAmplifyMod,					//@Fc, SPA: 507, SpellEffect::Fc_Amplify_Mod,					On Caster, damage-heal-dot mod pct, base: pct
	focusFcAmplifyAmt,					//@Fc, SPA: 508, SpellEffect::Fc_Amplify_Amt,					On Caster, damage-heal-dot mod flat amt, base: amt
	focusFcCastTimeMod2,				//@Fc, SPA: 500, SpellEffect::Fc_CastTimeMod2,				On Caster, cast time mod pct, base: pct
	focusFcCastTimeAmt,					//@Fc, SPA: 501, SpellEffect::Fc_CastTimeAmt,					On Caster, cast time mod flat amt, base: milliseconds
	focusFcHealPctCritIncoming,			//@Fc, SPA: 395, SpellEffect::FcHealPctCritIncoming,			On Target, spell healing mod pct, base: pct
	focusFcHealAmt,						//@Fc, SPA: 392, SpellEffect::FcHealAmt,						On Caster, spell healing mod flat amt, base: amt
	focusFcHealAmtCrit,					//@Fc, SPA: 396, SpellEffect::FcHealAmtCrit,					On Caster, spell healing mod flat amt, base: amt
} focusType; //Any new FocusType needs to be added to the Mob::IsFocus function
#define HIGHEST_FOCUS	focusFcHealAmtCrit //Should always be last focusType in enum

typedef enum {	//fear states
	fearStateNotFeared = 0,
	fearStateRunning,		//I am running, hoping to find a grid at my WP
	fearStateRunningForever,	//can run straight until spell ends
	fearStateGrid,			//I am allready on a fear grid
	fearStateStuck			//I cannot move somehow...
} FearState;

// This is actually FlyMode, from MQ2
enum GravityBehavior {
	Ground,
	Flying,
	Levitating,
	Water,
	Floating, // boat
	LevitateWhileRunning
};

class Trade;
enum TradeState {
	TradeNone,
	Trading,
	TradeAccepted,
	TradeCompleting
};

enum class NumHit {		  // Numhits type
	IncomingHitAttempts = 1,  // Attempted incoming melee attacks (hit or miss) on YOU.
	OutgoingHitAttempts = 2,  // Attempted outgoing melee attacks (hit or miss) on YOUR TARGET.
	IncomingSpells = 3,       // Incoming detrimental spells
	OutgoingSpells = 4,       // Outgoing detrimental spells
	OutgoingHitSuccess = 5,   // Successful outgoing melee attack HIT on YOUR TARGET.
	IncomingHitSuccess = 6,   // Successful incoming melee attack HIT on YOU.
	MatchingSpells = 7,       // Any casted spell matching/triggering a focus effect.
	IncomingDamage = 8,       // Successful incoming spell or melee dmg attack on YOU
	ReflectSpell = 9,	 // Incoming Reflected spells.
	DefensiveSpellProcs = 10, // Defensive buff procs
	OffensiveSpellProcs = 11  // Offensive buff procs
};

enum class PlayerState : uint32 {
	None = 0,
	Open = 1,
	WeaponSheathed = 2,
	Aggressive = 4,
	ForcedAggressive = 8,
	InstrumentEquipped = 16,
	Stunned = 32,
	PrimaryWeaponEquipped = 64,
	SecondaryWeaponEquipped = 128
};

enum class LootResponse : uint8 {
	SomeoneElse = 0,
	Normal = 1,
	NotAtThisTime = 2,
	Normal2 = 3, // acts exactly the same as Normal, maybe group vs ungroup? No idea
	Hostiles = 4,
	TooFar = 5,
	LootAll = 6 // SoD+
};

enum class LootRequestType : uint8 {
	Forbidden = 0,
	GMPeek,
	GMAllowed,
	Self,
	AllowedPVE,
	AllowedPVPAll,
	AllowedPVPSingle, // can make this 'AllowedPVPVariable' and allow values between 1 and EQ::invtype::POSSESSIONS_SIZE
	AllowedPVPDefined,
};

enum class KilledByTypes : uint8 {
	Killed_NPC = 0,
	Killed_DUEL = 1,
	Killed_PVP = 2
};

namespace Journal {
	enum class SpeakMode : uint8 {
		Raw = 0,	// this just uses the raw message
		Say = 1,	// prints with "%1 says,%2 '%3'" if in another language else "%1 says '%2'"
		Shout = 2,	// prints with "%1 shouts,%2 '%3'" if in another language else "%1 shouts '%2'"
		EmoteAlt = 3,	// prints "%2", this should just be the same as raw ...
		Emote = 4,	// prints "%1 %2" if message doesn't start with "\" or "@", else "%1%2"
		Group = 5	// prints "%1 tells the group,%2 '%3'"
	};

	enum class Mode : uint8 {
		None = 0,
		Log1 = 1, // 1 and 2 log to journal
		Log2 = 2, // our current code uses 2
	};

	struct Options {
		SpeakMode speak_mode;
		Mode journal_mode;
		int8 language;
		uint32 message_type;
		uint32 target_spawn_id; // who the message is talking to (limits journaling)
	};
};

//this is our internal representation of the BUFF struct, can put whatever we want in it
struct Buffs_Struct {
	uint16	spellid;
	uint8	casterlevel;
	uint16	casterid;		// Maybe change this to a pointer sometime, but gotta make sure it's 0'd when it no longer points to anything
	char	caster_name[64];
	int32	ticsremaining;
	uint32	counters;
	uint32	hit_number; //the number of physical hits this buff can take before it fades away, lots of druid armor spells take advantage of this mixed with powerful effects
	uint32	melee_rune;
	uint32	magic_rune;
	uint32	dot_rune;
	int32	caston_x;
	int32	caston_y;
	int32	caston_z;
	int32	ExtraDIChance;
	int16	RootBreakChance; //Not saved to dbase
	uint32	instrument_mod;
	int32	virus_spread_time; //time till next attempted viral spread
	bool	persistant_buff;
	bool	client; //True if the caster is a client
	bool	UpdateClient;

	// cereal
	template<class Archive>
	void serialize(Archive &ar)
	{

		std::string caster_name_str(caster_name);
		if (Archive::is_saving::value) {
			caster_name_str = std::string(caster_name);
		}

		ar(
			CEREAL_NVP(spellid),
			CEREAL_NVP(casterlevel),
			CEREAL_NVP(casterid),
			CEREAL_NVP(caster_name_str),
			CEREAL_NVP(ticsremaining),
			CEREAL_NVP(counters),
			CEREAL_NVP(hit_number),
			CEREAL_NVP(melee_rune),
			CEREAL_NVP(magic_rune),
			CEREAL_NVP(dot_rune),
			CEREAL_NVP(caston_x),
			CEREAL_NVP(caston_y),
			CEREAL_NVP(caston_z),
			CEREAL_NVP(ExtraDIChance),
			CEREAL_NVP(RootBreakChance),
			CEREAL_NVP(instrument_mod),
			CEREAL_NVP(virus_spread_time),
			CEREAL_NVP(persistant_buff),
			CEREAL_NVP(client),
			CEREAL_NVP(UpdateClient)
		);

		// Copy back into caster_name after deserialization
		if (Archive::is_loading::value) {
			strncpy(caster_name, caster_name_str.c_str(), sizeof(caster_name));
			caster_name[sizeof(caster_name) - 1] = '\0'; // Ensure null termination
		}
	}
};

struct StatBonuses {
	int32	AC;
	int64	HP;
	int64	HPRegen;
	int64	ManaRegen;
	int64	EnduranceRegen;
	int64	Mana;
	int64	Endurance;
	int32	ATK;
	//would it be worth it to create a Stat_Struct?
	int32	STR;
	int32	STRCapMod;
	int32	HeroicSTR;
	int32	STA;
	int32	STACapMod;
	int32	HeroicSTA;
	int32	DEX;
	int32	DEXCapMod;
	int32	HeroicDEX;
	int32	AGI;
	int32	AGICapMod;
	int32	HeroicAGI;
	int32	INT;
	int32	INTCapMod;
	int32	HeroicINT;
	int32	WIS;
	int32	WISCapMod;
	int32	HeroicWIS;
	int32	CHA;
	int32	CHACapMod;
	int32	HeroicCHA;
	int32	MR;
	int32	MRCapMod;
	int32	HeroicMR;
	int32	FR;
	int32	FRCapMod;
	int32	HeroicFR;
	int32	CR;
	int32	CRCapMod;
	int32	HeroicCR;
	int32	PR;
	int32	PRCapMod;
	int32	HeroicPR;
	int32	DR;
	int32	DRCapMod;
	int32	HeroicDR;
	int32	Corrup;
	int32	CorrupCapMod;
	int32	HeroicCorrup;
	uint16	DamageShieldSpellID;
	int		DamageShield;						// this is damage done to mobs that attack this
	DmgShieldType	DamageShieldType;
	int		SpellDamageShield;
	int		SpellShield;
	int		ReverseDamageShield;				// this is damage done to the mob when it attacks
	uint16	ReverseDamageShieldSpellID;
	DmgShieldType	ReverseDamageShieldType;
	int		movementspeed;
	int32	haste;
	int32	hastetype2;
	int32	hastetype3;
	int32	inhibitmelee;
	int32	increase_archery;
	float	AggroRange;							// when calculate just replace original value with this
	float	AssistRange;
	int32	skillmod[EQ::skills::HIGHEST_SKILL + 1];
	int32	skillmodmax[EQ::skills::HIGHEST_SKILL + 1];
	int		effective_casting_level;
	int		adjusted_casting_skill;				// SPA 112 for fizzles
	int		reflect[3];					// chance to reflect incoming spell [0]=Chance [1]=Resist Mod [2]= % of Base Dmg
	uint32	singingMod;
	uint32	Amplification;						// stacks with singingMod
	uint32	brassMod;
	uint32	percussionMod;
	uint32	windMod;
	uint32	stringedMod;
	uint32	songModCap;
	int32	hatemod;
	int64	EnduranceReduction;

	int32	StrikeThrough;						// PoP: Strike Through %
	int32	MeleeMitigation;					//i = Shielding
	int32	MeleeMitigationEffect;				//i = Spell Effect Melee Mitigation
	int32	CriticalHitChance[EQ::skills::HIGHEST_SKILL + 2];	//i
	int32	CriticalSpellChance;				//i
	int32	SpellCritDmgIncrease;				//i
	int32	SpellCritDmgIncNoStack;				// increase
	int32	DotCritDmgIncrease;					//i
	int32	CriticalHealChance;					//i
	int32	CriticalHealOverTime;				//i
	int32	CriticalDoTChance;					//i
	int32	CrippBlowChance;					//
	int32	AvoidMeleeChance;					//AvoidMeleeChance/10 == % chance i = Avoidance (item mod)
	int32	AvoidMeleeChanceEffect;				//AvoidMeleeChance Spell Effect
	int32	RiposteChance;						//i
	int32	DodgeChance;						//i
	int32	ParryChance;						//i
	int32	DualWieldChance;					//i
	int32	DoubleAttackChance;					//i
	int32	TripleAttackChance;					//i
	int32   DoubleRangedAttack;				//i
	int32	ResistSpellChance;					//i
	int32	ResistFearChance;					//i
	bool	Fearless;							//i
	bool	IsFeared;							//i
	bool	IsBlind;							//i
	int32	StunResist;							//i
	int32	MeleeSkillCheck;					//i
	uint8	MeleeSkillCheckSkill;
	int32	HitChance;							//HitChance/15 == % increase i = Accuracy (Item: Accuracy)
	int32	HitChanceEffect[EQ::skills::HIGHEST_SKILL + 2];	//Spell effect Chance to Hit, straight percent increase
	int32	DamageModifier[EQ::skills::HIGHEST_SKILL + 2];	//i
	int32	DamageModifier2[EQ::skills::HIGHEST_SKILL + 2];	//i
	int32	DamageModifier3[EQ::skills::HIGHEST_SKILL + 2];	//i
	int32	MinDamageModifier[EQ::skills::HIGHEST_SKILL + 2]; //i
	int32	ProcChance;							// ProcChance/10 == % increase i = CombatEffects
	int32	ProcChanceSPA;						// ProcChance from spell effects
	int32	ExtraAttackChance[2];				// base chance(w/ 2H weapon)=0, amt of extra attacks=1
	int32	ExtraAttackChancePrimary[2];		// base chance=0, , amt of extra attacks=1
	int32	ExtraAttackChanceSecondary[2];		// base chance=0, , amt of extra attacks=1
	int32	DoubleMeleeRound[2];				// base chance=0, damage mod=1
	int32	DoTShielding;
	int32	DivineSaveChance[2];				// Second Chance (base1 = chance, base2 = spell on trigger)
	uint32	DeathSave[4];						// Death Pact [0](value = 1 partial 2 = full) [1]=slot [2]=LvLimit [3]=HealAmt
	int32	FlurryChance;
	int32	Accuracy[EQ::skills::HIGHEST_SKILL + 2];			//Accuracy/15 == % increase	[Spell Effect: Accuracy)
	int32	HundredHands;						//extra haste, stacks with all other haste	i
	int32	MeleeLifetap;						//i
	int32	Vampirism;							//i
	int32	HealRate;							// Spell effect that influences effectiveness of heals
	int64	PercentMaxHPChange;					// base: Max HP change by percentage value from spell effect/item worn effect/aa
	int16	SkillDmgTaken[EQ::skills::HIGHEST_SKILL + 2];		// All Skills + -1
	int32	HealAmt;							// Item Effect
	int32	SpellDmg;							// Item Effect
	int32	Clairvoyance;						// Item Effect
	int32	DSMitigation;						// Item Effect
	int32	DSMitigationOffHand;				// Lowers damage shield from off hand attacks.
	uint32	SpellTriggers[MAX_SPELL_TRIGGER];	// Innate/Spell/Item Spells that trigger when you cast
	uint32	SpellOnKill[MAX_SPELL_TRIGGER*3];	// Chance to proc after killing a mob
	uint32	SpellOnDeath[MAX_SPELL_TRIGGER*2];	// Chance to have effect cast when you die
	int32	CritDmgMod[EQ::skills::HIGHEST_SKILL + 2];		// All Skills + -1
	int32	CritDmgModNoStack[EQ::skills::HIGHEST_SKILL + 2];// Critical melee damage modifier by percent, does not stack.
	int32	SkillReuseTime[EQ::skills::HIGHEST_SKILL + 1];	// Reduces skill timers
	int32	SkillDamageAmount[EQ::skills::HIGHEST_SKILL + 2];	// All Skills + -1
	int32	TwoHandBluntBlock;					// chance to block when wielding two hand blunt weapon
	uint32	ItemManaRegenCap;					// Increases the amount of mana you have can over the cap(aa effect)
	int32	GravityEffect;						// Indictor of spell effect
	bool	AntiGate;							// spell effect that prevents gating
	bool	MagicWeapon;						// spell effect that makes weapon magical
	int32	IncreaseBlockChance;				// overall block chance modifier
	uint32	PersistantCasting;					// chance to continue casting through a stun
	int	XPRateMod;							//i
	int		HPPercCap[2];						//Spell effect that limits you to being healed/regening beyond a % of your max
	int		ManaPercCap[2];						// ^^ 0 = % Cap 1 = Flat Amount Cap
	int		EndPercCap[2];						// ^^
	bool	ImmuneToFlee;						// Bypass the fleeing flag
	uint32	VoiceGraft;							// Stores the ID of the mob with which to talk through
	int32	SpellProcChance;					// chance to proc from sympathetic spell effects
	int32	CharmBreakChance;					// chance to break charm
	int32	SongRange;							// increases range of beneficial bard songs
	uint32	HPToManaConvert;					// Uses HP to cast spells at specific conversion
	int32	FocusEffects[HIGHEST_FOCUS+1];		// Stores the focus effectid for each focustype you have.
	int16	FocusEffectsWorn[HIGHEST_FOCUS+1];	// Optional to allow focus effects to be applied additively from worn slot, limits do not apply
	int32	FocusEffectsWornWithLimits[HIGHEST_FOCUS + 1];// Optional to allow focus effects to be applied additively from worn slot, limits apply
	bool	NegateEffects;						// Check if you contain a buff with negate effect. (only spellbonuses)
	int32	SkillDamageAmount2[EQ::skills::HIGHEST_SKILL + 2];	// Adds skill specific damage
	uint32	NegateAttacks[3];					// 0 = bool HasEffect 1 = Buff Slot 2 = Max damage absorbed per hit
	uint32	MitigateMeleeRune[4];				// 0 = Mitigation value 1 = Buff Slot 2 = Max mitigation per hit 3 = Rune Amt
	uint32	MeleeThresholdGuard[3];				// 0 = Mitigation value 1 = Buff Slot 2 = Min damage to trigger.
	uint32	SpellThresholdGuard[3];				// 0 = Mitigation value 1 = Buff Slot 2 = Min damage to trigger.
	uint32	MitigateSpellRune[4];				// 0 = Mitigation value 1 = Buff Slot 2 = Max mitigation per spell 3 = Rune Amt
	uint32	MitigateDotRune[4];					// 0 = Mitigation value 1 = Buff Slot 2 = Max mitigation per tick 3 = Rune Amt
	bool	TriggerMeleeThreshold;				// Has Melee Threshhold
	bool	TriggerSpellThreshold;				// Has Spell Threshhold
	uint32	ManaAbsorbPercentDamage;			// 0 = Mitigation value
	int32	EnduranceAbsorbPercentDamage[2];	// 0 = Mitigation value 1 = Percent Endurance drain per HP lost
	int32	ShieldBlock;						// Chance to Shield Block
	int32	BlockBehind;						// Chance to Block Behind (with our without shield)
	bool	CriticalRegenDecay;					// increase critical regen chance, decays based on spell level cast
	bool	CriticalHealDecay;					// increase critical heal chance, decays based on spell level cast
	bool	CriticalDotDecay;					// increase critical dot chance, decays based on spell level cast
	bool	DivineAura;							// invulnerability
	bool	DistanceRemoval;					// Check if Cancle if Moved effect is present
	int32	ImprovedTaunt[3];					// 0 = Max Level 1 = Aggro modifier 2 = buff slot
	int8	Root[2];							// The lowest buff slot a root can be found. [0] = Bool if has root [1] = buff slot
	int32	FrenziedDevastation;				// base1= AArank(used) base2= chance increase spell criticals + all DD spells 2x mana.
	uint32	AbsorbMagicAtt[2];					// 0 = magic rune value 1 = buff slot
	uint32	MeleeRune[2];						// 0 = rune value 1 = buff slot
	bool	NegateIfCombat;						// Bool Drop buff if cast or melee
	int8	Screech;							// -1 = Will be blocked if another Screech is +(1)
	int32	AlterNPCLevel;						// amount of lvls +/-
	int32	AStacker[2];						// For buff stack blocking 0=Exists 1=Effect_value
	int32	BStacker[2];						// For buff stack blocking 0=Exists 1=Effect_value
	int32	CStacker[2];						// For buff stack blocking 0=Exists 1=Effect_value
	int32	DStacker[2];						// For buff stack blocking 0=Exists 1=Effect_value
	bool	BerserkSPA;							// berserk effect
	int32	Metabolism;							// Food/drink consumption rates.
	bool	Sanctuary;							// Sanctuary effect, lowers place on hate list until cast on others.
	int32   FactionModPct;						// Modifies amount of faction gained.
	bool	LimitToSkill[EQ::skills::HIGHEST_SKILL + 3];		// Determines if we need to search for a skill proc.
	int32  SkillProc[MAX_SKILL_PROCS];			// Max number of spells containing skill_procs.
	int32  SkillProcSuccess[MAX_SKILL_PROCS];	// Max number of spells containing skill_procs_success.
	int32   SpellProc[MAX_AA_PROCS];		// Max number of spells containing melee spell procs.
	int32   RangedProc[MAX_AA_PROCS];	    // Max number of spells containing ranged spell procs.
	int32   DefensiveProc[MAX_AA_PROCS];	// Max number of spells containing defensive spell procs.
	bool	Proc_Timer_Modifier;				// Used to check if this exists, to avoid any further unnncessary checks.
	uint32  PC_Pet_Rampage[2];					// 0= % chance to rampage, 1=damage modifier
	uint32  PC_Pet_AE_Rampage[2];				// 0= % chance to AE rampage, 1=damage modifier
	uint32  PC_Pet_Flurry;						// Percent chance flurry from double attack
	int32   Attack_Accuracy_Max_Percent;		// Increase ATK accuracy by percent.
	int32   AC_Mitigation_Max_Percent;			// Increase AC mitigation by percent
	int32   AC_Avoidance_Max_Percent;			// Increase AC avoidance by percent
	int32   Damage_Taken_Position_Mod[2];		// base = percent melee damage reduction base2 0=back 1=front. [0]Back[1]Front
	int32   Melee_Damage_Position_Mod[2];		// base = percent melee damage increase base2 0=back 1=front. [0]Back[1]Front
	int32   Damage_Taken_Position_Amt[2];		// base = flat amt melee damage reduction base2 0=back 1=front. [0]Back[1]Front
	int32   Melee_Damage_Position_Amt[2];		// base = flat amt melee damage increase base2 0=back 1=front. [0]Back[1]Front
	int32   Double_Backstab_Front;				// base = percent chance to double back stab front
	int32   DS_Mitigation_Amount;				// base = flat amt DS mitigation. Negative value to reduce
	int32	DS_Mitigation_Percentage;			// base = percent amt of DS mitigation. Negative value to reduce
	int32   Pet_Crit_Melee_Damage_Pct_Owner;	// base = percent mod for pet critcal damage from owner
	int32	Pet_Add_Atk;						// base = Pet ATK bonus from owner
	int32	ItemEnduranceRegenCap;				// modify endurance regen cap
	int32   WeaponStance[WEAPON_STANCE_TYPE_MAX +1];// base = trigger spell id, base2 = 0 is 2h, 1 is shield, 2 is dual wield, [0]spid 2h, [1]spid shield, [2]spid DW
	bool	ZoneSuspendMinion;					// base 1 allows suspended minions to zone
	bool	CompleteHealBuffBlocker;			// Use in SPA 101 to prevent recast of complete heal from this effect till blocker buff is removed.
	int32	Illusion;							// illusion spell id
	uint8	invisibility;						// invisibility level
	uint8	invisibility_verse_undead;			// IVU level
	uint8	invisibility_verse_animal;			// IVA level
	int32	ShieldTargetSpa[2];                 // [0] base = % mitigation amount, [1] buff slot
	uint32  ReduceSkill[EQ::skills::HIGHEST_SKILL + 2]; //reduce value of a skill by percentage
	int64	FlatMaxHPChange;					// base: Max HP change by a flat amount value from spell effect/item worn effect/aa


	// AAs
	int32	TrapCircumvention;					// reduce chance to trigger a trap.
	uint16  SecondaryForte;						// allow a second skill to be specialized with a cap of this value.
	int32	ShieldDuration;						// extends duration of /shield ability
	int32	ExtendedShielding;					// extends range of /shield ability
	int8	Packrat;							// weight reduction for items, 1 point = 10%
	uint8	BuffSlotIncrease;					// Increases number of available buff slots
	uint32	DelayDeath;							// how far below 0 hp you can go
	int8	BaseMovementSpeed;					// Adjust base run speed, does not stack with other movement bonuses.
	uint8	IncreaseRunSpeedCap;				// Increase max run speed above cap.
	int32	DoubleSpecialAttack;				// Chance to to perform a double special attack (ie flying kick 2x)
	int32	SkillAttackProc[MAX_CAST_ON_SKILL_USE];					// [0] chance to proc [2] spell on [1] skill usage
	bool	HasSkillAttackProc[EQ::skills::HIGHEST_SKILL + 1]; //check if any skill proc is present before assessing for all skill procs
	uint8	FrontalStunResist;					// Chance to resist a frontal stun
	int32	BindWound;							// Increase amount of HP by percent.
	int32	MaxBindWound;						// Increase max amount of HP you can bind wound.
	int32	ChannelChanceSpells;				// Modify chance to channel a spell.
	int32	ChannelChanceItems;					// Modify chance to channel a items.
	uint8	SeeInvis;							// See Invs.
	uint8	TripleBackstab;						// Chance to triple backstab
	bool	FrontalBackstabMinDmg;				// Allow frontal backstabs for min damage
	uint8	FrontalBackstabChance;				// Chance to backstab from the front for full damage
	uint8	ConsumeProjectile;					// Chance to not consume arrow.
	uint8	ForageAdditionalItems;				// Chance to forage another item.
	uint8	SalvageChance;						// Chance to salvage a tradeskill components on fail.
	uint32	ArcheryDamageModifier;				// Increase Archery Damage by percent
	bool	SecondaryDmgInc;					// Allow off hand weapon to recieve damage bonus.
	uint32	GiveDoubleAttack;					// Allow classes to double attack with a specified chance.
	int32	SlayUndead[2];						// Allow classes to do extra damage verse undead.(base1 = rate, base2 = damage mod)
	int32	PetCriticalHit;						// Allow pets to critical hit with % value.
	int32	PetAvoidance;						// Pet avoidance chance.
	int32	CombatStability;					// Melee damage mitigation.
	int32	DoubleRiposte;						// Chance to double riposte
	int32	GiveDoubleRiposte[3];				// 0=Regular Chance, 1=Skill Attack Chance, 2=Skill
	uint32	RaiseSkillCap[EQ::skills::HIGHEST_SKILL + 1];		// Raise a specific skill cap (base1= value, base2=skill)
	int32	Ambidexterity;						// Increase chance to duel wield by adding bonus 'skill'.
	int32	PetMaxHP;							// Increase the max hp of your pet.
	int32	PetFlurry;							// Chance for pet to flurry.
	uint8	MasteryofPast;						// Can not fizzle spells below this level specified in value.
	bool	GivePetGroupTarget;					// All pets to recieve group buffs. (Pet Affinity)
	int32	RootBreakChance;					// Chance root will break;
	int32	UnfailingDivinity;					// Improves chance that DI will fire + increase partial heal.
	int32	ItemHPRegenCap;						// Increase item regen cap.
	int32	SEResist[MAX_RESISTABLE_EFFECTS*2];	// Resist chance by specific spell effects.
	int32	OffhandRiposteFail;					// chance for opponent to fail riposte with offhand attack.
	int32	ItemATKCap;							// Raise item attack cap
	int32	FinishingBlow[2];					// Chance to do a finishing blow for specified damage amount.
	uint32	FinishingBlowLvl[2];				// Sets max level an NPC can be affected by FB. (base1 = lv, base2= hit point ratio)
	int32	ShieldEquipDmgMod;					// Increases weapon's base damage by base1 % when shield is equipped (indirectly increasing hate)
	bool	TriggerOnCastRequirement;			// Triggers off various different conditions defined as emum SpellRestrictions
	int8	StunBashChance;						// chance to stun with bash.
	int8	IncreaseChanceMemwipe;				// increases chance to memory wipe
	int8	CriticalMend;						// chance critical monk mend
	int32	ImprovedReclaimEnergy;				// Modifies amount of mana returned from reclaim energy
	uint32	HeadShot[2];						// Headshot AA (Massive dmg vs humaniod w/ archery) 0= ? 1= Dmg
	uint8	HSLevel[2];							// Max Level Headshot will be effective at. and chance mod
	uint32	Assassinate[2];						// Assassinate AA (Massive dmg vs humaniod w/ assassinate) 0= ? 1= Dmg
	uint8	AssassinateLevel[2];				// Max Level Assassinate will be effective at.
	int32	PetMeleeMitigation;					// Add AC to owner's pet.
	int		IllusionPersistence;				// 1=Causes illusions not to fade when zoning 2=Allow to persist after death.
	uint16	extra_xtargets;						// extra xtarget entries
	bool	ShroudofStealth;					// rogue improved invisiblity
	uint16  ReduceFallDamage;					// reduce fall damage by percent
	int32	ReduceTradeskillFail[EQ::skills::HIGHEST_SKILL + 1]; // Reduces chance for trade skills to fail by percent.
	uint8	TradeSkillMastery;					// Allow number of tradeskills to exceed 200 skill.
	int16	NoBreakAESneak;						// Percent value
	int16	FeignedCastOnChance;				// Percent Value
	bool	PetCommands[PetCommand::Max];		// SPA 267
	int	FeignedMinionChance;				// SPA 281 base1 = chance, just like normal FD
	int	GrantForage; // affects max skill of forage as well as granting non-forage classes forage
	int aura_slots;
	int trap_slots;
	bool hunger; // Song of Sustenance -- min caps to 3500
	int64 heroic_max_hp;
	int64 heroic_max_mana;
	int64 heroic_max_end;
	int64 heroic_hp_regen;
	int64 heroic_mana_regen;
	int64 heroic_end_regen;
	int32 heroic_str_shield_ac;
	int32 heroic_str_melee_damage;
	int32 heroic_agi_avoidance;
	int32 heroic_dex_ranged_damage;
};

// StatBonus Indexes
namespace SBIndex {
	constexpr uint16 BUFFSTACKER_EXISTS                     = 0; // SPA 446-449
	constexpr uint16 BUFFSTACKER_VALUE                      = 1; // SPA 446-449
	constexpr uint16 EXTRA_ATTACK_CHANCE                    = 0; // SPA 266,498,499
	constexpr uint16 EXTRA_ATTACK_NUM_ATKS                  = 1; // SPA 266,498,499
	constexpr uint16 DIVINE_SAVE_CHANCE                     = 0; // SPA 232
	constexpr uint16 DIVINE_SAVE_SPELL_TRIGGER_ID           = 1; // SPA 232
	constexpr uint16 DEATH_SAVE_TYPE                        = 0; // SPA 150
	constexpr uint16 DEATH_SAVE_BUFFSLOT                    = 1; // SPA 150
	constexpr uint16 DEATH_SAVE_MIN_LEVEL_FOR_HEAL          = 2; // SPA 150
	constexpr uint16 DEATH_SAVE_HEAL_AMT                    = 3; // SPA 150
	constexpr uint16 RESOURCE_PERCENT_CAP                   = 0; // SPA 408-410
	constexpr uint16 RESOURCE_AMOUNT_CAP                    = 1; // SPA 408-419
	constexpr uint16 NEGATE_ATK_EXISTS                      = 0; // SPA 163
	constexpr uint16 NEGATE_ATK_BUFFSLOT                    = 1; // SPA 163
	constexpr uint16 NEGATE_ATK_MAX_DMG_ABSORB_PER_HIT      = 2; // SPA 163
	constexpr uint16 MITIGATION_RUNE_PERCENT                = 0; // SPA 161,162,450
	constexpr uint16 MITIGATION_RUNE_BUFFSLOT               = 1; // SPA 161,162,450
	constexpr uint16 MITIGATION_RUNE_MAX_DMG_ABSORB_PER_HIT = 2; // SPA 161,162,450
	constexpr uint16 MITIGATION_RUNE_MAX_HP_AMT             = 3; // SPA 161,162,450
	constexpr uint16 THRESHOLDGUARD_MITIGATION_PERCENT      = 0; // SPA 451,452
	constexpr uint16 THRESHOLDGUARD_BUFFSLOT                = 1; // SPA 451,452
	constexpr uint16 THRESHOLDGUARD_MIN_DMG_TO_TRIGGER      = 2; // SPA 451,452
	constexpr uint16 ENDURANCE_ABSORD_MITIGIATION           = 0; // SPA 521
	constexpr uint16 ENDURANCE_ABSORD_DRAIN_PER_HP          = 1; // SPA 521
	constexpr uint16 IMPROVED_TAUNT_MAX_LVL                 = 0; // SPA 444
	constexpr uint16 IMPROVED_TAUNT_AGGRO_MOD               = 1; // SPA 444
	constexpr uint16 IMPROVED_TAUNT_BUFFSLOT                = 2; // SPA 444
	constexpr uint16 ROOT_EXISTS                            = 0; // SPA 99
	constexpr uint16 ROOT_BUFFSLOT                          = 1; // SPA 99
	constexpr uint16 RUNE_AMOUNT                            = 0; // SPA 55
	constexpr uint16 RUNE_BUFFSLOT                          = 1; // SPA 78
	constexpr uint16 POSITION_BACK							= 0; // SPA 503-506
	constexpr uint16 POSITION_FRONT							= 1; // SPA 503-506
	constexpr uint16 PET_RAMPAGE_CHANCE                     = 0; // SPA 464,465
	constexpr uint16 PET_RAMPAGE_DMG_MOD                    = 1; // SPA 465,465
	constexpr uint16 SKILLATK_PROC_SPELL_ID                 = 0; // SPA 288
	constexpr uint16 SKILLATK_PROC_CHANCE                   = 1; // SPA 288
	constexpr uint16 SKILLATK_PROC_SKILL                    = 2; // SPA 288
	constexpr uint16 SLAYUNDEAD_DMG_MOD                     = 0; // SPA 219
	constexpr uint16 SLAYUNDEAD_RATE_MOD                    = 1; // SPA 219
	constexpr uint16 DOUBLE_RIPOSTE_CHANCE                  = 0; // SPA 223
	constexpr uint16 DOUBLE_RIPOSTE_SKILL_ATK_CHANCE        = 1; // SPA 223
	constexpr uint16 DOUBLE_RIPOSTE_SKILL                   = 2; // SPA 223
	constexpr uint16 FINISHING_EFFECT_PROC_CHANCE           = 0; // SPA 278, 439, 217
	constexpr uint16 FINISHING_EFFECT_DMG                   = 1; // SPA 278, 439, 217
	constexpr uint16 FINISHING_EFFECT_LEVEL_MAX             = 0; // SPA 440, 345, 346
	constexpr uint16 FINISHING_EFFECT_LEVEL_CHANCE_BONUS    = 1; // SPA 345, 346
	constexpr uint16 FINISHING_BLOW_LEVEL_HP_RATIO			= 1; // SPA 440
	constexpr uint16 DOUBLE_MELEE_ROUND_CHANCE              = 0; // SPA 471
	constexpr uint16 DOUBLE_MELEE_ROUND_DMG_BONUS			= 1; // SPA 471
	constexpr uint16 REFLECT_CHANCE                         = 0; // SPA 158
	constexpr uint16 REFLECT_RESISTANCE_MOD                 = 1; // SPA 158
	constexpr uint16 REFLECT_DMG_EFFECTIVENESS              = 2; // SPA 158
	constexpr uint16 COMBAT_PROC_ORIGIN_ID                  = 0; // SPA
	constexpr uint16 COMBAT_PROC_SPELL_ID                   = 1; // SPA
	constexpr uint16 COMBAT_PROC_RATE_MOD                   = 2; // SPA
	constexpr uint16 COMBAT_PROC_REUSE_TIMER                = 3; // SPA
	constexpr uint16 SHIELD_TARGET_MITIGATION_PERCENT       = 0; // SPA 463
	constexpr uint16 SHIELD_TARGET_BUFFSLOT                 = 1; // SPA 463
};


typedef struct
{
	int32 spellID;
	uint16 chance;
	int32 base_spellID;
	int level_override;
	uint32 proc_reuse_time;
} tProc;


struct WeaponStance_Struct {
	bool enabled;
	bool spellbonus_enabled;
	bool itembonus_enabled;
	bool aabonus_enabled;
	int spellbonus_buff_spell_id;
	int itembonus_buff_spell_id;
	int aabonus_buff_spell_id;
};

constexpr uint16 WEAPON_STANCE_TYPE_2H = 0;
constexpr uint16 WEAPON_STANCE_TYPE_SHIELD = 1;
constexpr uint16 WEAPON_STANCE_TYPE_DUAL_WIELD = 2;

typedef struct
{
	uint16 increment;
	uint16 hit_increment;
	uint16 target_id;
	int32 wpn_dmg;
	float origin_x;
	float origin_y;
	float origin_z;
	float tlast_x;
	float tlast_y;
	uint32 ranged_id;
	uint32 ammo_id;
	int ammo_slot;
	uint8 skill;
	float speed_mod;
	bool disable_procs;
} tProjatk;

//eventually turn this into a typedef and
//make DoAnim take it instead of int, to enforce its use.
enum {	//type arguments to DoAnim
	animKick				= 1,
	anim1HPiercing			= 2,	//might be piercing?
	anim2HSlashing			= 3,
	anim2HWeapon			= 4,
	anim1HWeapon			= 5,
	animDualWield			= 6,
	animTailRake			= 7,	//slam & Dpunch too
	animHand2Hand			= 8,
	animShootBow			= 9,
	animRoundKick			= 11,
	animSwarmAttack			= 20,	//dunno about this one..
	animFlyingKick			= 45,
	animTigerClaw			= 46,
	animEagleStrike			= 47,

};

enum {
	SKILLUP_UNKNOWN = 0,
	SKILLUP_SUCCESS = 1,
	SKILLUP_FAILURE = 2
};

enum {
	GridCircular,
	GridRandom10,
	GridRandom,
	GridPatrol,
	GridOneWayRepop,
	GridRand5LoS,
	GridOneWayDepop,
	GridCenterPoint,
	GridRandomCenterPoint,
	GridRandomPath
};

typedef enum {
	SingleTarget,	// causes effect to spell_target
	AETarget,			// causes effect in aerange of target + target
	AECaster,			// causes effect in aerange of 'this'
	GroupSpell,		// causes effect to caster + target's group
	CAHateList,		// causes effect to all people on caster's hate list within some range
	DirectionalAE,
	Beam,
	TargetRing,
	CastActUnknown
} CastAction_type;


struct MercType {
	uint32	Type;
	uint32	ClientVersion;
};

struct MercData {
	uint32	MercTemplateID;
	uint32	MercType;				// From dbstr_us.txt - Apprentice (330000100), Journeyman (330000200), Master (330000300)
	uint32	MercSubType;			// From dbstr_us.txt - 330020105^23^Race: Guktan<br>Type: Healer<br>Confidence: High<br>Proficiency: Apprentice, Tier V...
	uint32	CostFormula;			// To determine cost to client
	uint32	ClientVersion;				// Only send valid mercs per expansion
	uint32	NPCID;
};

namespace EQ
{
	class ItemInstance;
}

class Mob;
// All data associated with a single trade
class Trade
{
public:
	Trade(Mob* in_owner);
	virtual ~Trade();

	void Reset();

	// Initiate a trade with another mob
	// Also puts other mob into trader mode with this mob
	void Start(uint32 mob_id, bool initiate_with=true);

	// Mob the owner is trading with
	Mob* With();

	// Add item from cursor slot to trade bucket (automatically does bag data too)
	void AddEntity(uint16 trade_slot_id, uint32 stack_size);


public:
	// Object state
	TradeState state;
	int32 pp;
	int32 gp;
	int32 sp;
	int32 cp;

private:
	// Send item data for trade item to other person involved in trade
	void SendItemData(const EQ::ItemInstance* inst, int16 dest_slot_id);

	uint32 with_id;
	Mob* owner;
public:
	Mob *GetOwner() const;
};

struct ExtraAttackOptions {
	ExtraAttackOptions()
		: damage_percent(1.0f), damage_flat(0),
		armor_pen_percent(0.0f), armor_pen_flat(0),
		crit_percent(1.0f), crit_flat(0.0f),
		hate_percent(1.0f), hate_flat(0), hit_chance(0),
		melee_damage_bonus_flat(0), skilldmgtaken_bonus_flat(0),
		range_percent(0)
	{ }

	float damage_percent;
	int64 damage_flat;
	float armor_pen_percent;
	int armor_pen_flat;
	float crit_percent;
	float crit_flat;
	float hate_percent;
	int hate_flat;
	int hit_chance;
	int melee_damage_bonus_flat;
	int skilldmgtaken_bonus_flat;
	int range_percent;
};

struct DamageTable {
	int32 max_extra; // max extra damage
	int32 chance; // chance not to apply?
	int32 minusfactor; // difficulty of rolling
};

struct DamageHitInfo {
	//uint16 attacker; // id
	//uint16 defender; // id
	int64 base_damage;
	int64 min_damage;
	int64 damage_done;
	int offense;
	int tohit;
	int hand;
	EQ::skills::SkillType skill;
};

struct DataBucketCache
{
	uint64_t      bucket_id;
	std::string   bucket_key;
	std::string   bucket_value;
	uint32_t      bucket_expires;
};

// Defines based on the RoF2 Client
#define PET_HEALTHREPORT	0	// 0x00 - /pet health or Pet Window
#define PET_LEADER			1	// 0x01 - /pet leader or Pet Window
#define PET_ATTACK			2	// 0x02 - /pet attack or Pet Window
#define PET_QATTACK			3	// 0x03 - /pet qattack or Pet Window
#define PET_FOLLOWME		4	// 0x04 - /pet follow or Pet Window
#define PET_GUARDHERE		5	// 0x05 - /pet guard or Pet Window
#define PET_SIT				6	// 0x06 - /pet sit or Pet Window
#define PET_SITDOWN			7	// 0x07 - /pet sit on
#define PET_STANDUP			8	// 0x08 - /pet sit off
#define PET_STOP			9	// 0x09 - /pet stop or Pet Window - Not implemented
#define PET_STOP_ON			10	// 0x0a - /pet stop on - Not implemented
#define PET_STOP_OFF		11	// 0x0b - /pet stop off - Not implemented
#define PET_TAUNT			12	// 0x0c - /pet taunt or Pet Window
#define PET_TAUNT_ON		13	// 0x0d - /pet taunt on
#define PET_TAUNT_OFF		14	// 0x0e - /pet taunt off
#define PET_HOLD			15	// 0x0f - /pet hold or Pet Window, won't add to hate list unless attacking
#define PET_HOLD_ON			16	// 0x10 - /pet hold on
#define PET_HOLD_OFF		17	// 0x11 - /pet hold off
#define PET_GHOLD			18	// 0x12 - /pet ghold, will never add to hate list unless told to
#define PET_GHOLD_ON		19	// 0x13 - /pet ghold on
#define PET_GHOLD_OFF		20	// 0x14 - /pet ghold off
#define PET_SPELLHOLD		21	// 0x15 - /pet no cast or /pet spellhold or Pet Window
#define PET_SPELLHOLD_ON	22	// 0x16 - /pet spellhold on
#define PET_SPELLHOLD_OFF	23	// 0x17 - /pet spellhold off
#define PET_FOCUS			24	// 0x18 - /pet focus or Pet Window
#define PET_FOCUS_ON		25	// 0x19 - /pet focus on
#define PET_FOCUS_OFF		26	// 0x1a - /pet focus off
#define PET_FEIGN			27	// 0x1b - /pet feign
#define PET_BACKOFF			28	// 0x1c - /pet back off
#define PET_GETLOST			29	// 0x1d - /pet get lost
#define PET_GUARDME			30	// 0x1e - Same as /pet follow, but different message in older clients - define not from client /pet target in modern clients but doesn't send packet
#define PET_REGROUP			31	// 0x1f - /pet regroup, acts like classic hold. Stops attack and moves back to guard/you but doesn't clear hate list
#define PET_REGROUP_ON		32	// 0x20 - /pet regroup on, turns on regroup
#define PET_REGROUP_OFF		33	// 0x21 - /pet regroup off, turns off regroup
#define PET_MAXCOMMANDS		PET_REGROUP_OFF + 1

// can change the state of these buttons with a packet
#define PET_BUTTON_SIT			0
#define PET_BUTTON_STOP			1
#define PET_BUTTON_REGROUP		2
#define PET_BUTTON_FOLLOW		3
#define PET_BUTTON_GUARD		4
#define PET_BUTTON_TAUNT		5
#define PET_BUTTON_HOLD			6
#define PET_BUTTON_GHOLD		7
#define PET_BUTTON_FOCUS		8
#define PET_BUTTON_SPELLHOLD	9

enum eStandingPetOrder { SPO_Follow, SPO_Sit, SPO_Guard, SPO_FeignDeath };

typedef enum {
	petFamiliar,		//only listens to /pet get lost
	petAnimation,		//does not listen to any commands
	petOther,
	petCharmed,
	petNPCFollow,
	petTargetLock,			//remain active as long something is on the hatelist. Don't listen to any commands
	petNone = 0xFF // not a pet
} PetTypeOld;
