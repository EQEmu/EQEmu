#ifdef LUA_EQEMU

#include "lua_stat_bonuses.h"

#include "common/data_verification.h"

#include "luabind/luabind.hpp"


int32 Lua_StatBonuses::GetAC() const {
	Lua_Safe_Call_Int();
	return self->AC;
}

int64 Lua_StatBonuses::GetHP() const {
	Lua_Safe_Call_Int();
	return self->HP;
}

int64 Lua_StatBonuses::GetFlatMaxHPChange() const {
	Lua_Safe_Call_Int();
	return self->FlatMaxHPChange;
}

int64 Lua_StatBonuses::GetHPRegen() const {
	Lua_Safe_Call_Int();
	return self->HPRegen;
}

int64 Lua_StatBonuses::GetMaxHP() const {
	Lua_Safe_Call_Int();
	return self->PercentMaxHPChange;
}

int64 Lua_StatBonuses::GetManaRegen() const {
	Lua_Safe_Call_Int();
	return self->ManaRegen;
}

int64 Lua_StatBonuses::GetEnduranceRegen() const {
	Lua_Safe_Call_Int();
	return self->EnduranceRegen;
}

int64 Lua_StatBonuses::GetMana() const {
	Lua_Safe_Call_Int();
	return self->Mana;
}

int64 Lua_StatBonuses::GetEndurance() const {
	Lua_Safe_Call_Int();
	return self->Endurance;
}

int32 Lua_StatBonuses::GetATK() const {
	Lua_Safe_Call_Int();
	return self->ATK;
}

int32 Lua_StatBonuses::GetSTR() const {
	Lua_Safe_Call_Int();
	return self->STR;
}

int32 Lua_StatBonuses::GetSTRCapMod() const {
	Lua_Safe_Call_Int();
	return self->STRCapMod;
}

int32 Lua_StatBonuses::GetHeroicSTR() const {
	Lua_Safe_Call_Int();
	return self->HeroicSTR;
}

int32 Lua_StatBonuses::GetSTA() const {
	Lua_Safe_Call_Int();
	return self->STA;
}

int32 Lua_StatBonuses::GetSTACapMod() const {
	Lua_Safe_Call_Int();
	return self->STACapMod;
}

int32 Lua_StatBonuses::GetHeroicSTA() const {
	Lua_Safe_Call_Int();
	return self->HeroicSTA;
}

int32 Lua_StatBonuses::GetDEX() const {
	Lua_Safe_Call_Int();
	return self->DEX;
}

int32 Lua_StatBonuses::GetDEXCapMod() const {
	Lua_Safe_Call_Int();
	return self->DEXCapMod;
}

int32 Lua_StatBonuses::GetHeroicDEX() const {
	Lua_Safe_Call_Int();
	return self->HeroicDEX;
}

int32 Lua_StatBonuses::GetAGI() const {
	Lua_Safe_Call_Int();
	return self->AGI;
}

int32 Lua_StatBonuses::GetAGICapMod() const {
	Lua_Safe_Call_Int();
	return self->AGICapMod;
}

int32 Lua_StatBonuses::GetHeroicAGI() const {
	Lua_Safe_Call_Int();
	return self->HeroicAGI;
}

int32 Lua_StatBonuses::GetINT() const {
	Lua_Safe_Call_Int();
	return self->INT;
}

int32 Lua_StatBonuses::GetINTCapMod() const {
	Lua_Safe_Call_Int();
	return self->INTCapMod;
}

int32 Lua_StatBonuses::GetHeroicINT() const {
	Lua_Safe_Call_Int();
	return self->HeroicINT;
}

int32 Lua_StatBonuses::GetWIS() const {
	Lua_Safe_Call_Int();
	return self->WIS;
}

int32 Lua_StatBonuses::GetWISCapMod() const {
	Lua_Safe_Call_Int();
	return self->WISCapMod;
}

int32 Lua_StatBonuses::GetHeroicWIS() const {
	Lua_Safe_Call_Int();
	return self->HeroicWIS;
}

int32 Lua_StatBonuses::GetCHA() const {
	Lua_Safe_Call_Int();
	return self->CHA;
}

int32 Lua_StatBonuses::GetCHACapMod() const {
	Lua_Safe_Call_Int();
	return self->CHACapMod;
}

int32 Lua_StatBonuses::GetHeroicCHA() const {
	Lua_Safe_Call_Int();
	return self->HeroicCHA;
}

int32 Lua_StatBonuses::GetMR() const {
	Lua_Safe_Call_Int();
	return self->MR;
}

int32 Lua_StatBonuses::GetMRCapMod() const {
	Lua_Safe_Call_Int();
	return self->MRCapMod;
}

int32 Lua_StatBonuses::GetHeroicMR() const {
	Lua_Safe_Call_Int();
	return self->HeroicMR;
}

int32 Lua_StatBonuses::GetFR() const {
	Lua_Safe_Call_Int();
	return self->FR;
}

int32 Lua_StatBonuses::GetFRCapMod() const {
	Lua_Safe_Call_Int();
	return self->FRCapMod;
}

int32 Lua_StatBonuses::GetHeroicFR() const {
	Lua_Safe_Call_Int();
	return self->HeroicFR;
}

int32 Lua_StatBonuses::GetCR() const {
	Lua_Safe_Call_Int();
	return self->CR;
}

int32 Lua_StatBonuses::GetCRCapMod() const {
	Lua_Safe_Call_Int();
	return self->CRCapMod;
}

int32 Lua_StatBonuses::GetHeroicCR() const {
	Lua_Safe_Call_Int();
	return self->HeroicCR;
}

int32 Lua_StatBonuses::GetPR() const {
	Lua_Safe_Call_Int();
	return self->PR;
}

int32 Lua_StatBonuses::GetPRCapMod() const {
	Lua_Safe_Call_Int();
	return self->PRCapMod;
}

int32 Lua_StatBonuses::GetHeroicPR() const {
	Lua_Safe_Call_Int();
	return self->HeroicPR;
}

int32 Lua_StatBonuses::GetDR() const {
	Lua_Safe_Call_Int();
	return self->DR;
}

int32 Lua_StatBonuses::GetDRCapMod() const {
	Lua_Safe_Call_Int();
	return self->DRCapMod;
}

int32 Lua_StatBonuses::GetHeroicDR() const {
	Lua_Safe_Call_Int();
	return self->HeroicDR;
}

int32 Lua_StatBonuses::GetCorrup() const {
	Lua_Safe_Call_Int();
	return self->Corrup;
}

int32 Lua_StatBonuses::GetCorrupCapMod() const {
	Lua_Safe_Call_Int();
	return self->CorrupCapMod;
}

int32 Lua_StatBonuses::GetHeroicCorrup() const {
	Lua_Safe_Call_Int();
	return self->HeroicCorrup;
}

uint16 Lua_StatBonuses::GetDamageShieldSpellID() const {
	Lua_Safe_Call_Int();
	return self->DamageShieldSpellID;
}

int Lua_StatBonuses::GetDamageShield() const {
	Lua_Safe_Call_Int();
	return self->DamageShield;
}

int Lua_StatBonuses::GetDamageShieldType() const {
	Lua_Safe_Call_Int();
	return self->DamageShieldType;
}

int Lua_StatBonuses::GetSpellDamageShield() const {
	Lua_Safe_Call_Int();
	return self->SpellDamageShield;
}

int Lua_StatBonuses::GetSpellShield() const {
	Lua_Safe_Call_Int();
	return self->SpellShield;
}

int Lua_StatBonuses::GetReverseDamageShield() const {
	Lua_Safe_Call_Int();
	return self->ReverseDamageShield;
}

uint16 Lua_StatBonuses::GetReverseDamageShieldSpellID() const {
	Lua_Safe_Call_Int();
	return self->ReverseDamageShieldSpellID;
}

int Lua_StatBonuses::GetReverseDamageShieldType() const {
	Lua_Safe_Call_Int();
	return self->ReverseDamageShieldType;
}

int Lua_StatBonuses::Getmovementspeed() const {
	Lua_Safe_Call_Int();
	return self->movementspeed;
}

int32 Lua_StatBonuses::Gethaste() const {
	Lua_Safe_Call_Int();
	return self->haste;
}

int32 Lua_StatBonuses::Gethastetype2() const {
	Lua_Safe_Call_Int();
	return self->hastetype2;
}

int32 Lua_StatBonuses::Gethastetype3() const {
	Lua_Safe_Call_Int();
	return self->hastetype3;
}

int32 Lua_StatBonuses::Getinhibitmelee() const {
	Lua_Safe_Call_Int();
	return self->inhibitmelee;
}

float Lua_StatBonuses::GetAggroRange() const {
	Lua_Safe_Call_Real();
	return self->AggroRange;
}

float Lua_StatBonuses::GetAssistRange() const {
	Lua_Safe_Call_Real();
	return self->AssistRange;
}

int32 Lua_StatBonuses::Getskillmod(int idx) const {
	Lua_Safe_Call_Int();
	return self->skillmod[idx];
}

int32 Lua_StatBonuses::Getskillmodmax(int idx) const {
	Lua_Safe_Call_Int();
	return self->skillmodmax[idx];
}

int Lua_StatBonuses::Geteffective_casting_level() const {
	Lua_Safe_Call_Int();
	return self->effective_casting_level;
}

int Lua_StatBonuses::Getadjusted_casting_skill() const {
	Lua_Safe_Call_Int();
	return self->adjusted_casting_skill;
}

int Lua_StatBonuses::Getreflect_chance() const {
	Lua_Safe_Call_Int();
	return self->reflect[SBIndex::REFLECT_CHANCE];
}

uint32 Lua_StatBonuses::GetsingingMod() const {
	Lua_Safe_Call_Int();
	return self->singingMod;
}

uint32 Lua_StatBonuses::GetAmplification() const {
	Lua_Safe_Call_Int();
	return self->Amplification;
}

uint32 Lua_StatBonuses::GetbrassMod() const {
	Lua_Safe_Call_Int();
	return self->brassMod;
}

uint32 Lua_StatBonuses::GetpercussionMod() const {
	Lua_Safe_Call_Int();
	return self->percussionMod;
}

uint32 Lua_StatBonuses::GetwindMod() const {
	Lua_Safe_Call_Int();
	return self->windMod;
}

uint32 Lua_StatBonuses::GetstringedMod() const {
	Lua_Safe_Call_Int();
	return self->stringedMod;
}

uint32 Lua_StatBonuses::GetsongModCap() const {
	Lua_Safe_Call_Int();
	return self->songModCap;
}

int32 Lua_StatBonuses::Gethatemod() const {
	Lua_Safe_Call_Int();
	return self->hatemod;
}

int64 Lua_StatBonuses::GetEnduranceReduction() const {
	Lua_Safe_Call_Int();
	return self->EnduranceReduction;
}

int32 Lua_StatBonuses::GetStrikeThrough() const {
	Lua_Safe_Call_Int();
	return self->StrikeThrough;
}

int32 Lua_StatBonuses::GetMeleeMitigation() const {
	Lua_Safe_Call_Int();
	return self->MeleeMitigation;
}

int32 Lua_StatBonuses::GetMeleeMitigationEffect() const {
	Lua_Safe_Call_Int();
	return self->MeleeMitigationEffect;
}

int32 Lua_StatBonuses::GetCriticalHitChance(int idx) const {
	Lua_Safe_Call_Int();
	return self->CriticalHitChance[idx];
}

int32 Lua_StatBonuses::GetCriticalSpellChance() const {
	Lua_Safe_Call_Int();
	return self->CriticalSpellChance;
}

int32 Lua_StatBonuses::GetSpellCritDmgIncrease() const {
	Lua_Safe_Call_Int();
	return self->SpellCritDmgIncrease;
}

int32 Lua_StatBonuses::GetSpellCritDmgIncNoStack() const {
	Lua_Safe_Call_Int();
	return self->SpellCritDmgIncNoStack;
}

int32 Lua_StatBonuses::GetDotCritDmgIncrease() const {
	Lua_Safe_Call_Int();
	return self->DotCritDmgIncrease;
}

int32 Lua_StatBonuses::GetCriticalHealChance() const {
	Lua_Safe_Call_Int();
	return self->CriticalHealChance;
}

int32 Lua_StatBonuses::GetCriticalHealOverTime() const {
	Lua_Safe_Call_Int();
	return self->CriticalHealOverTime;
}

int32 Lua_StatBonuses::GetCriticalDoTChance() const {
	Lua_Safe_Call_Int();
	return self->CriticalDoTChance;
}

int32 Lua_StatBonuses::GetCrippBlowChance() const {
	Lua_Safe_Call_Int();
	return self->CrippBlowChance;
}

int32 Lua_StatBonuses::GetAvoidMeleeChance() const {
	Lua_Safe_Call_Int();
	return self->AvoidMeleeChance;
}

int32 Lua_StatBonuses::GetAvoidMeleeChanceEffect() const {
	Lua_Safe_Call_Int();
	return self->AvoidMeleeChanceEffect;
}

int32 Lua_StatBonuses::GetRiposteChance() const {
	Lua_Safe_Call_Int();
	return self->RiposteChance;
}

int32 Lua_StatBonuses::GetDodgeChance() const {
	Lua_Safe_Call_Int();
	return self->DodgeChance;
}

int32 Lua_StatBonuses::GetParryChance() const {
	Lua_Safe_Call_Int();
	return self->ParryChance;
}

int32 Lua_StatBonuses::GetDualWieldChance() const {
	Lua_Safe_Call_Int();
	return self->DualWieldChance;
}

int32 Lua_StatBonuses::GetDoubleAttackChance() const {
	Lua_Safe_Call_Int();
	return self->DoubleAttackChance;
}

int32 Lua_StatBonuses::GetTripleAttackChance() const {
	Lua_Safe_Call_Int();
	return self->TripleAttackChance;
}

int32 Lua_StatBonuses::GetDoubleRangedAttack() const {
	Lua_Safe_Call_Int();
	return self->DoubleRangedAttack;
}

int32 Lua_StatBonuses::GetResistSpellChance() const {
	Lua_Safe_Call_Int();
	return self->ResistSpellChance;
}

int32 Lua_StatBonuses::GetResistFearChance() const {
	Lua_Safe_Call_Int();
	return self->ResistFearChance;
}

bool Lua_StatBonuses::GetFearless() const {
	Lua_Safe_Call_Bool();
	return self->Fearless;
}

bool Lua_StatBonuses::GetIsFeared() const {
	Lua_Safe_Call_Bool();
	return self->IsFeared;
}

bool Lua_StatBonuses::GetIsBlind() const {
	Lua_Safe_Call_Bool();
	return self->IsBlind;
}

int32 Lua_StatBonuses::GetStunResist() const {
	Lua_Safe_Call_Int();
	return self->StunResist;
}

int32 Lua_StatBonuses::GetMeleeSkillCheck() const {
	Lua_Safe_Call_Int();
	return self->MeleeSkillCheck;
}

uint8 Lua_StatBonuses::GetMeleeSkillCheckSkill() const {
	Lua_Safe_Call_Int();
	return self->MeleeSkillCheckSkill;
}

int32 Lua_StatBonuses::GetHitChance() const {
	Lua_Safe_Call_Int();
	return self->HitChance;
}

int32 Lua_StatBonuses::GetHitChanceEffect(int idx) const {
	Lua_Safe_Call_Int();
	return self->HitChanceEffect[idx];
}

int32 Lua_StatBonuses::GetDamageModifier(int idx) const {
	Lua_Safe_Call_Int();
	return self->DamageModifier[idx];
}

int32 Lua_StatBonuses::GetDamageModifier2(int idx) const {
	Lua_Safe_Call_Int();
	return self->DamageModifier2[idx];
}

int32 Lua_StatBonuses::GetMinDamageModifier(int idx) const {
	Lua_Safe_Call_Int();
	return self->MinDamageModifier[idx];
}

int32 Lua_StatBonuses::GetProcChance() const {
	Lua_Safe_Call_Int();
	return self->ProcChance;
}

int32 Lua_StatBonuses::GetProcChanceSPA() const {
	Lua_Safe_Call_Int();
	return self->ProcChanceSPA;
}

int32 Lua_StatBonuses::GetExtraAttackChance() const {
	Lua_Safe_Call_Int();
	return self->ExtraAttackChance[0];
}

int32 Lua_StatBonuses::GetDoTShielding() const {
	Lua_Safe_Call_Int();
	return self->DoTShielding;
}

int32 Lua_StatBonuses::GetFlurryChance() const {
	Lua_Safe_Call_Int();
	return self->FlurryChance;
}

int32 Lua_StatBonuses::GetHundredHands() const {
	Lua_Safe_Call_Int();
	return self->HundredHands;
}

int32 Lua_StatBonuses::GetMeleeLifetap() const {
	Lua_Safe_Call_Int();
	return self->MeleeLifetap;
}

int32 Lua_StatBonuses::GetVampirism() const {
	Lua_Safe_Call_Int();
	return self->Vampirism;
}

int32 Lua_StatBonuses::GetHealRate() const {
	Lua_Safe_Call_Int();
	return self->HealRate;
}

int32 Lua_StatBonuses::GetMaxHPChange() const {
	Lua_Safe_Call_Int();
	return self->PercentMaxHPChange;
}

int32 Lua_StatBonuses::GetHealAmt() const {
	Lua_Safe_Call_Int();
	return self->HealAmt;
}

int32 Lua_StatBonuses::GetSpellDmg() const {
	Lua_Safe_Call_Int();
	return self->SpellDmg;
}

int32 Lua_StatBonuses::GetClairvoyance() const {
	Lua_Safe_Call_Int();
	return self->Clairvoyance;
}

int32 Lua_StatBonuses::GetDSMitigation() const {
	Lua_Safe_Call_Int();
	return self->DSMitigation;
}

int32 Lua_StatBonuses::GetDSMitigationOffHand() const {
	Lua_Safe_Call_Int();
	return self->DSMitigationOffHand;
}

int32 Lua_StatBonuses::GetTwoHandBluntBlock() const {
	Lua_Safe_Call_Int();
	return self->TwoHandBluntBlock;
}

uint32 Lua_StatBonuses::GetItemManaRegenCap() const {
	Lua_Safe_Call_Int();
	return self->ItemManaRegenCap;
}

int32 Lua_StatBonuses::GetGravityEffect() const {
	Lua_Safe_Call_Int();
	return self->GravityEffect;
}

bool Lua_StatBonuses::GetAntiGate() const {
	Lua_Safe_Call_Bool();
	return self->AntiGate;
}

bool Lua_StatBonuses::GetMagicWeapon() const {
	Lua_Safe_Call_Bool();
	return self->MagicWeapon;
}

int32 Lua_StatBonuses::GetIncreaseBlockChance() const {
	Lua_Safe_Call_Int();
	return self->IncreaseBlockChance;
}

uint32 Lua_StatBonuses::GetPersistantCasting() const {
	Lua_Safe_Call_Int();
	return self->PersistantCasting;
}

int Lua_StatBonuses::GetXPRateMod() const {
	Lua_Safe_Call_Int();
	return self->XPRateMod;
}

bool Lua_StatBonuses::GetBlockNextSpell() const {
	Lua_Safe_Call_Bool();
	return false; // Bonus no longer used due to effect being a focus
}

bool Lua_StatBonuses::GetImmuneToFlee() const {
	Lua_Safe_Call_Bool();
	return self->ImmuneToFlee;
}

uint32 Lua_StatBonuses::GetVoiceGraft() const {
	Lua_Safe_Call_Int();
	return self->VoiceGraft;
}

int32 Lua_StatBonuses::GetSpellProcChance() const {
	Lua_Safe_Call_Int();
	return self->SpellProcChance;
}

int32 Lua_StatBonuses::GetCharmBreakChance() const {
	Lua_Safe_Call_Int();
	return self->CharmBreakChance;
}

int32 Lua_StatBonuses::GetSongRange() const {
	Lua_Safe_Call_Int();
	return self->SongRange;
}

uint32 Lua_StatBonuses::GetHPToManaConvert() const {
	Lua_Safe_Call_Int();
	return self->HPToManaConvert;
}

bool Lua_StatBonuses::GetNegateEffects() const {
	Lua_Safe_Call_Bool();
	return self->NegateEffects;
}

bool Lua_StatBonuses::GetTriggerMeleeThreshold() const {
	Lua_Safe_Call_Bool();
	return self->TriggerMeleeThreshold;
}

bool Lua_StatBonuses::GetTriggerSpellThreshold() const {
	Lua_Safe_Call_Bool();
	return self->TriggerSpellThreshold;
}

int32 Lua_StatBonuses::GetShieldBlock() const {
	Lua_Safe_Call_Int();
	return self->ShieldBlock;
}

int32 Lua_StatBonuses::GetBlockBehind() const {
	Lua_Safe_Call_Int();
	return self->BlockBehind;
}

bool Lua_StatBonuses::GetCriticalRegenDecay() const {
	Lua_Safe_Call_Bool();
	return self->CriticalRegenDecay;
}

bool Lua_StatBonuses::GetCriticalHealDecay() const {
	Lua_Safe_Call_Bool();
	return self->CriticalHealDecay;
}

bool Lua_StatBonuses::GetCriticalDotDecay() const {
	Lua_Safe_Call_Bool();
	return self->CriticalDotDecay;
}

bool Lua_StatBonuses::GetDivineAura() const {
	Lua_Safe_Call_Bool();
	return self->DivineAura;
}

bool Lua_StatBonuses::GetDistanceRemoval() const {
	Lua_Safe_Call_Bool();
	return self->DistanceRemoval;
}

int32 Lua_StatBonuses::GetFrenziedDevastation() const {
	Lua_Safe_Call_Int();
	return self->FrenziedDevastation;
}

bool Lua_StatBonuses::GetNegateIfCombat() const {
	Lua_Safe_Call_Bool();
	return self->NegateIfCombat;
}

int8 Lua_StatBonuses::GetScreech() const {
	Lua_Safe_Call_Int();
	return self->Screech;
}

int32 Lua_StatBonuses::GetAlterNPCLevel() const {
	Lua_Safe_Call_Int();
	return self->AlterNPCLevel;
}

bool Lua_StatBonuses::GetBerserkSPA() const {
	Lua_Safe_Call_Bool();
	return self->BerserkSPA;
}

int32 Lua_StatBonuses::GetMetabolism() const {
	Lua_Safe_Call_Int();
	return self->Metabolism;
}

bool Lua_StatBonuses::GetSanctuary() const {
	Lua_Safe_Call_Bool();
	return self->Sanctuary;
}

int32 Lua_StatBonuses::GetFactionModPct() const {
	Lua_Safe_Call_Int();
	return self->FactionModPct;
}

uint32 Lua_StatBonuses::GetPC_Pet_Flurry() const {
	Lua_Safe_Call_Int();
	return self->PC_Pet_Flurry;
}

int8 Lua_StatBonuses::GetPackrat() const {
	Lua_Safe_Call_Int();
	return self->Packrat;
}

uint8 Lua_StatBonuses::GetBuffSlotIncrease() const {
	Lua_Safe_Call_Int();
	return self->BuffSlotIncrease;
}

uint32 Lua_StatBonuses::GetDelayDeath() const {
	Lua_Safe_Call_Int();
	return self->DelayDeath;
}

int8 Lua_StatBonuses::GetBaseMovementSpeed() const {
	Lua_Safe_Call_Int();
	return self->BaseMovementSpeed;
}

uint8 Lua_StatBonuses::GetIncreaseRunSpeedCap() const {
	Lua_Safe_Call_Int();
	return self->IncreaseRunSpeedCap;
}

int32 Lua_StatBonuses::GetDoubleSpecialAttack() const {
	Lua_Safe_Call_Int();
	return self->DoubleSpecialAttack;
}

uint8 Lua_StatBonuses::GetFrontalStunResist() const {
	Lua_Safe_Call_Int();
	return self->FrontalStunResist;
}

int32 Lua_StatBonuses::GetBindWound() const {
	Lua_Safe_Call_Int();
	return self->BindWound;
}

int32 Lua_StatBonuses::GetMaxBindWound() const {
	Lua_Safe_Call_Int();
	return self->MaxBindWound;
}

int32 Lua_StatBonuses::GetChannelChanceSpells() const {
	Lua_Safe_Call_Int();
	return self->ChannelChanceSpells;
}

int32 Lua_StatBonuses::GetChannelChanceItems() const {
	Lua_Safe_Call_Int();
	return self->ChannelChanceItems;
}

uint8 Lua_StatBonuses::GetSeeInvis() const {
	Lua_Safe_Call_Int();
	return self->SeeInvis;
}

uint8 Lua_StatBonuses::GetTripleBackstab() const {
	Lua_Safe_Call_Int();
	return self->TripleBackstab;
}

bool Lua_StatBonuses::GetFrontalBackstabMinDmg() const {
	Lua_Safe_Call_Bool();
	return self->FrontalBackstabMinDmg;
}

uint8 Lua_StatBonuses::GetFrontalBackstabChance() const {
	Lua_Safe_Call_Int();
	return self->FrontalBackstabChance;
}

uint8 Lua_StatBonuses::GetConsumeProjectile() const {
	Lua_Safe_Call_Int();
	return self->ConsumeProjectile;
}

uint8 Lua_StatBonuses::GetForageAdditionalItems() const {
	Lua_Safe_Call_Int();
	return self->ForageAdditionalItems;
}

uint8 Lua_StatBonuses::GetSalvageChance() const {
	Lua_Safe_Call_Int();
	return self->SalvageChance;
}

uint32 Lua_StatBonuses::GetArcheryDamageModifier() const {
	Lua_Safe_Call_Int();
	return self->ArcheryDamageModifier;
}

bool Lua_StatBonuses::GetSecondaryDmgInc() const {
	Lua_Safe_Call_Bool();
	return self->SecondaryDmgInc;
}

uint32 Lua_StatBonuses::GetGiveDoubleAttack() const {
	Lua_Safe_Call_Int();
	return self->GiveDoubleAttack;
}

int32 Lua_StatBonuses::GetPetCriticalHit() const {
	Lua_Safe_Call_Int();
	return self->PetCriticalHit;
}

int32 Lua_StatBonuses::GetPetAvoidance() const {
	Lua_Safe_Call_Int();
	return self->PetAvoidance;
}

int32 Lua_StatBonuses::GetCombatStability() const {
	Lua_Safe_Call_Int();
	return self->CombatStability;
}

int32 Lua_StatBonuses::GetDoubleRiposte() const {
	Lua_Safe_Call_Int();
	return self->DoubleRiposte;
}

int32 Lua_StatBonuses::GetAmbidexterity() const {
	Lua_Safe_Call_Int();
	return self->Ambidexterity;
}

int32 Lua_StatBonuses::GetPetMaxHP() const {
	Lua_Safe_Call_Int();
	return self->PetMaxHP;
}

int32 Lua_StatBonuses::GetPetFlurry() const {
	Lua_Safe_Call_Int();
	return self->PetFlurry;
}

uint8 Lua_StatBonuses::GetMasteryofPast() const {
	Lua_Safe_Call_Int();
	return self->MasteryofPast;
}

bool Lua_StatBonuses::GetGivePetGroupTarget() const {
	Lua_Safe_Call_Bool();
	return self->GivePetGroupTarget;
}

int32 Lua_StatBonuses::GetRootBreakChance() const {
	Lua_Safe_Call_Int();
	return self->RootBreakChance;
}

int32 Lua_StatBonuses::GetUnfailingDivinity() const {
	Lua_Safe_Call_Int();
	return self->UnfailingDivinity;
}

int32 Lua_StatBonuses::GetItemHPRegenCap() const {
	Lua_Safe_Call_Int();
	return self->ItemHPRegenCap;
}

int32 Lua_StatBonuses::GetOffhandRiposteFail() const {
	Lua_Safe_Call_Int();
	return self->OffhandRiposteFail;
}

int32 Lua_StatBonuses::GetItemATKCap() const {
	Lua_Safe_Call_Int();
	return self->ItemATKCap;
}

int32 Lua_StatBonuses::GetShieldEquipDmgMod() const {
	Lua_Safe_Call_Int();
	return self->ShieldEquipDmgMod;
}

bool Lua_StatBonuses::GetTriggerOnValueAmount() const {
	Lua_Safe_Call_Bool();
	return self->TriggerOnCastRequirement;
}

int8 Lua_StatBonuses::GetStunBashChance() const {
	Lua_Safe_Call_Int();
	return self->StunBashChance;
}

int8 Lua_StatBonuses::GetIncreaseChanceMemwipe() const {
	Lua_Safe_Call_Int();
	return self->IncreaseChanceMemwipe;
}

int8 Lua_StatBonuses::GetCriticalMend() const {
	Lua_Safe_Call_Int();
	return self->CriticalMend;
}

int32 Lua_StatBonuses::GetImprovedReclaimEnergy() const {
	Lua_Safe_Call_Int();
	return self->ImprovedReclaimEnergy;
}

int32 Lua_StatBonuses::GetPetMeleeMitigation() const {
	Lua_Safe_Call_Int();
	return self->PetMeleeMitigation;
}

bool Lua_StatBonuses::GetIllusionPersistence() const {
	Lua_Safe_Call_Bool();
	return self->IllusionPersistence;
}

uint16 Lua_StatBonuses::Getextra_xtargets() const {
	Lua_Safe_Call_Int();
	return self->extra_xtargets;
}

bool Lua_StatBonuses::GetShroudofStealth() const {
	Lua_Safe_Call_Bool();
	return self->ShroudofStealth;
}

uint16 Lua_StatBonuses::GetReduceFallDamage() const {
	Lua_Safe_Call_Int();
	return self->ReduceFallDamage;
}

uint8 Lua_StatBonuses::GetTradeSkillMastery() const {
	Lua_Safe_Call_Int();
	return self->TradeSkillMastery;
}

int16 Lua_StatBonuses::GetNoBreakAESneak() const {
	Lua_Safe_Call_Int();
	return self->NoBreakAESneak;
}

int16 Lua_StatBonuses::GetFeignedCastOnChance() const {
	Lua_Safe_Call_Int();
	return self->FeignedCastOnChance;
}

int32 Lua_StatBonuses::GetDivineSaveChance(int idx) const {
	Lua_Safe_Call_Int();
	return self->DivineSaveChance[idx];
}

uint32 Lua_StatBonuses::GetDeathSave(int idx) const {
	Lua_Safe_Call_Int();
	return self->DeathSave[idx];
}

int32 Lua_StatBonuses::GetAccuracy(int idx) const {
	Lua_Safe_Call_Int();
	return self->Accuracy[idx];
}

int16 Lua_StatBonuses::GetSkillDmgTaken(int idx) const {
	Lua_Safe_Call_Int();
	return self->SkillDmgTaken[idx];
}

uint32 Lua_StatBonuses::GetSpellTriggers(int idx) const {
	Lua_Safe_Call_Int();
	return self->SpellTriggers[idx];
}

uint32 Lua_StatBonuses::GetSpellOnKill(int idx) const {
	Lua_Safe_Call_Int();
	return self->SpellOnKill[idx];
}

uint32 Lua_StatBonuses::GetSpellOnDeath(int idx) const {
	Lua_Safe_Call_Int();
	return self->SpellOnDeath[idx];
}

int32 Lua_StatBonuses::GetCritDmgMod(int idx) const {
	Lua_Safe_Call_Int();
	return self->CritDmgMod[idx];
}

int32 Lua_StatBonuses::GetSkillReuseTime(int idx) const {
	Lua_Safe_Call_Int();
	return self->SkillReuseTime[idx];
}

int32 Lua_StatBonuses::GetSkillDamageAmount(int idx) const {
	Lua_Safe_Call_Int();
	if (!EQ::ValueWithin(idx, ALL_SKILLS, EQ::skills::HIGHEST_SKILL)) {
		return 0;
	}

	return idx == ALL_SKILLS ? self->SkillDamageAmount[EQ::skills::HIGHEST_SKILL + 1] : self->SkillDamageAmount[idx];
}

int Lua_StatBonuses::GetHPPercCap(int idx) const {
	Lua_Safe_Call_Int();
	return self->HPPercCap[idx];
}

int Lua_StatBonuses::GetManaPercCap(int idx) const {
	Lua_Safe_Call_Int();
	return self->ManaPercCap[idx];
}

int Lua_StatBonuses::GetEndPercCap(int idx) const {
	Lua_Safe_Call_Int();
	return self->EndPercCap[idx];
}

uint8 Lua_StatBonuses::GetFocusEffects(int idx) const {
	Lua_Safe_Call_Int();
	return self->FocusEffects[idx];
}

int16 Lua_StatBonuses::GetFocusEffectsWorn(int idx) const {
	Lua_Safe_Call_Int();
	return self->FocusEffectsWorn[idx];
}

int32 Lua_StatBonuses::GetSkillDamageAmount2(int idx) const {
	Lua_Safe_Call_Int();
	return self->SkillDamageAmount2[idx];
}

uint32 Lua_StatBonuses::GetNegateAttacks(int idx) const {
	Lua_Safe_Call_Int();
	return self->NegateAttacks[idx];
}

uint32 Lua_StatBonuses::GetMitigateMeleeRune(int idx) const {
	Lua_Safe_Call_Int();
	return self->MitigateMeleeRune[idx];
}

uint32 Lua_StatBonuses::GetMeleeThresholdGuard(int idx) const {
	Lua_Safe_Call_Int();
	return self->MeleeThresholdGuard[idx];
}

uint32 Lua_StatBonuses::GetSpellThresholdGuard(int idx) const {
	Lua_Safe_Call_Int();
	return self->SpellThresholdGuard[idx];
}

uint32 Lua_StatBonuses::GetMitigateSpellRune(int idx) const {
	Lua_Safe_Call_Int();
	return self->MitigateSpellRune[idx];
}

uint32 Lua_StatBonuses::GetMitigateDotRune(int idx) const {
	Lua_Safe_Call_Int();
	return self->MitigateDotRune[idx];
}

uint32 Lua_StatBonuses::GetManaAbsorbPercentDamage(int idx) const {
	Lua_Safe_Call_Int();
	return self->ManaAbsorbPercentDamage;
}

int32 Lua_StatBonuses::GetImprovedTaunt(int idx) const {
	Lua_Safe_Call_Int();
	return self->ImprovedTaunt[idx];
}

int8 Lua_StatBonuses::GetRoot(int idx) const {
	Lua_Safe_Call_Int();
	return self->Root[idx];
}

uint32 Lua_StatBonuses::GetAbsorbMagicAtt(int idx) const {
	Lua_Safe_Call_Int();
	return self->AbsorbMagicAtt[idx];
}

uint32 Lua_StatBonuses::GetMeleeRune(int idx) const {
	Lua_Safe_Call_Int();
	return self->MeleeRune[idx];
}

int32 Lua_StatBonuses::GetAStacker(int idx) const {
	Lua_Safe_Call_Int();
	return self->AStacker[idx];
}

int32 Lua_StatBonuses::GetBStacker(int idx) const {
	Lua_Safe_Call_Int();
	return self->BStacker[idx];
}

int32 Lua_StatBonuses::GetCStacker(int idx) const {
	Lua_Safe_Call_Int();
	return self->CStacker[idx];
}

int32 Lua_StatBonuses::GetDStacker(int idx) const {
	Lua_Safe_Call_Int();
	return self->DStacker[idx];
}

bool Lua_StatBonuses::GetLimitToSkill(int idx) const {
	Lua_Safe_Call_Bool();
	return self->LimitToSkill[idx];
}

uint32 Lua_StatBonuses::GetSkillProc(int idx) const {
	Lua_Safe_Call_Int();
	return self->SkillProc[idx];
}

uint32 Lua_StatBonuses::GetSkillProcSuccess(int idx) const {
	Lua_Safe_Call_Int();
	return self->SkillProcSuccess[idx];
}

uint32 Lua_StatBonuses::GetPC_Pet_Rampage(int idx) const {
	Lua_Safe_Call_Int();
	return self->PC_Pet_Rampage[idx];
}

int32 Lua_StatBonuses::GetSkillAttackProc(int idx) const {
	Lua_Safe_Call_Int();
	return self->SkillAttackProc[idx];
}

int32 Lua_StatBonuses::GetSlayUndead(int idx) const {
	Lua_Safe_Call_Int();
	return self->SlayUndead[idx];
}

int32 Lua_StatBonuses::GetGiveDoubleRiposte(int idx) const {
	Lua_Safe_Call_Int();
	return self->GiveDoubleRiposte[idx];
}

uint32 Lua_StatBonuses::GetRaiseSkillCap(int idx) const {
	Lua_Safe_Call_Int();
	return self->RaiseSkillCap[idx];
}

int32 Lua_StatBonuses::GetSEResist(int idx) const {
	Lua_Safe_Call_Int();
	return self->SEResist[idx];
}

int32 Lua_StatBonuses::GetFinishingBlow(int idx) const {
	Lua_Safe_Call_Int();
	return self->FinishingBlow[idx];
}

uint32 Lua_StatBonuses::GetFinishingBlowLvl(int idx) const {
	Lua_Safe_Call_Int();
	return self->FinishingBlowLvl[idx];
}

uint32 Lua_StatBonuses::GetHeadShot(int idx) const {
	Lua_Safe_Call_Int();
	return self->HeadShot[idx];
}

uint8 Lua_StatBonuses::GetHSLevel(int idx) const {
	Lua_Safe_Call_Int();
	return self->HSLevel[idx];
}

uint32 Lua_StatBonuses::GetAssassinate(int idx) const {
	Lua_Safe_Call_Int();
	return self->Assassinate[idx];
}

uint8 Lua_StatBonuses::GetAssassinateLevel(int idx) const {
	Lua_Safe_Call_Int();
	return self->AssassinateLevel[idx];
}

int32 Lua_StatBonuses::GetReduceTradeskillFail(int idx) const {
	Lua_Safe_Call_Int();
	return self->ReduceTradeskillFail[idx];
}

luabind::scope lua_register_stat_bonuses() {
	return luabind::class_<Lua_StatBonuses>("StatBonuses")
	.def(luabind::constructor<>())
	.def("AbsorbMagicAtt", &Lua_StatBonuses::GetAbsorbMagicAtt)
	.def("AC", &Lua_StatBonuses::GetAC)
	.def("Accuracy", &Lua_StatBonuses::GetAccuracy)
	.def("adjusted_casting_skill", &Lua_StatBonuses::Getadjusted_casting_skill)
	.def("AggroRange", &Lua_StatBonuses::GetAggroRange)
	.def("AGI", &Lua_StatBonuses::GetAGI)
	.def("AGICapMod", &Lua_StatBonuses::GetAGICapMod)
	.def("AlterNPCLevel", &Lua_StatBonuses::GetAlterNPCLevel)
	.def("Ambidexterity", &Lua_StatBonuses::GetAmbidexterity)
	.def("Amplification", &Lua_StatBonuses::GetAmplification)
	.def("AntiGate", &Lua_StatBonuses::GetAntiGate)
	.def("ArcheryDamageModifier", &Lua_StatBonuses::GetArcheryDamageModifier)
	.def("Assassinate", &Lua_StatBonuses::GetAssassinate)
	.def("AssassinateLevel", &Lua_StatBonuses::GetAssassinateLevel)
	.def("AssistRange", &Lua_StatBonuses::GetAssistRange)
	.def("AStacker", &Lua_StatBonuses::GetAStacker)
	.def("ATK", &Lua_StatBonuses::GetATK)
	.def("AvoidMeleeChance", &Lua_StatBonuses::GetAvoidMeleeChance)
	.def("AvoidMeleeChanceEffect", &Lua_StatBonuses::GetAvoidMeleeChanceEffect)
	.def("BaseMovementSpeed", &Lua_StatBonuses::GetBaseMovementSpeed)
	.def("BerserkSPA", &Lua_StatBonuses::GetBerserkSPA)
	.def("BindWound", &Lua_StatBonuses::GetBindWound)
	.def("BlockBehind", &Lua_StatBonuses::GetBlockBehind)
	.def("BlockNextSpell", &Lua_StatBonuses::GetBlockNextSpell)
	.def("brassMod", &Lua_StatBonuses::GetbrassMod)
	.def("BStacker", &Lua_StatBonuses::GetBStacker)
	.def("BuffSlotIncrease", &Lua_StatBonuses::GetBuffSlotIncrease)
	.def("CHA", &Lua_StatBonuses::GetCHA)
	.def("CHACapMod", &Lua_StatBonuses::GetCHACapMod)
	.def("ChannelChanceItems", &Lua_StatBonuses::GetChannelChanceItems)
	.def("ChannelChanceSpells", &Lua_StatBonuses::GetChannelChanceSpells)
	.def("CharmBreakChance", &Lua_StatBonuses::GetCharmBreakChance)
	.def("Clairvoyance", &Lua_StatBonuses::GetClairvoyance)
	.def("CombatStability", &Lua_StatBonuses::GetCombatStability)
	.def("ConsumeProjectile", &Lua_StatBonuses::GetConsumeProjectile)
	.def("Corrup", &Lua_StatBonuses::GetCorrup)
	.def("CorrupCapMod", &Lua_StatBonuses::GetCorrupCapMod)
	.def("CR", &Lua_StatBonuses::GetCR)
	.def("CRCapMod", &Lua_StatBonuses::GetCRCapMod)
	.def("CrippBlowChance", &Lua_StatBonuses::GetCrippBlowChance)
	.def("CritDmgMod", &Lua_StatBonuses::GetCritDmgMod)
	.def("CriticalDoTChance", &Lua_StatBonuses::GetCriticalDoTChance)
	.def("CriticalDotDecay", &Lua_StatBonuses::GetCriticalDotDecay)
	.def("CriticalHealChance", &Lua_StatBonuses::GetCriticalHealChance)
	.def("CriticalHealDecay", &Lua_StatBonuses::GetCriticalHealDecay)
	.def("CriticalHealOverTime", &Lua_StatBonuses::GetCriticalHealOverTime)
	.def("CriticalHitChance", &Lua_StatBonuses::GetCriticalHitChance)
	.def("CriticalMend", &Lua_StatBonuses::GetCriticalMend)
	.def("CriticalRegenDecay", &Lua_StatBonuses::GetCriticalRegenDecay)
	.def("CriticalSpellChance", &Lua_StatBonuses::GetCriticalSpellChance)
	.def("CStacker", &Lua_StatBonuses::GetCStacker)
	.def("DamageModifier", &Lua_StatBonuses::GetDamageModifier)
	.def("DamageModifier2", &Lua_StatBonuses::GetDamageModifier2)
	.def("DamageShield", &Lua_StatBonuses::GetDamageShield)
	.def("DamageShieldSpellID", &Lua_StatBonuses::GetDamageShieldSpellID)
	.def("DamageShieldType", &Lua_StatBonuses::GetDamageShieldType)
	.def("DeathSave", &Lua_StatBonuses::GetDeathSave)
	.def("DelayDeath", &Lua_StatBonuses::GetDelayDeath)
	.def("DEX", &Lua_StatBonuses::GetDEX)
	.def("DEXCapMod", &Lua_StatBonuses::GetDEXCapMod)
	.def("DistanceRemoval", &Lua_StatBonuses::GetDistanceRemoval)
	.def("DivineAura", &Lua_StatBonuses::GetDivineAura)
	.def("DivineSaveChance", &Lua_StatBonuses::GetDivineSaveChance)
	.def("DodgeChance", &Lua_StatBonuses::GetDodgeChance)
	.def("DotCritDmgIncrease", &Lua_StatBonuses::GetDotCritDmgIncrease)
	.def("DoTShielding", &Lua_StatBonuses::GetDoTShielding)
	.def("DoubleAttackChance", &Lua_StatBonuses::GetDoubleAttackChance)
	.def("DoubleRangedAttack", &Lua_StatBonuses::GetDoubleRangedAttack)
	.def("DoubleRiposte", &Lua_StatBonuses::GetDoubleRiposte)
	.def("DoubleSpecialAttack", &Lua_StatBonuses::GetDoubleSpecialAttack)
	.def("DR", &Lua_StatBonuses::GetDR)
	.def("DRCapMod", &Lua_StatBonuses::GetDRCapMod)
	.def("DSMitigation", &Lua_StatBonuses::GetDSMitigation)
	.def("DSMitigationOffHand", &Lua_StatBonuses::GetDSMitigationOffHand)
	.def("DStacker", &Lua_StatBonuses::GetDStacker)
	.def("DualWieldChance", &Lua_StatBonuses::GetDualWieldChance)
	.def("effective_casting_level", &Lua_StatBonuses::Geteffective_casting_level)
	.def("EndPercCap", &Lua_StatBonuses::GetEndPercCap)
	.def("Endurance", &Lua_StatBonuses::GetEndurance)
	.def("EnduranceReduction", &Lua_StatBonuses::GetEnduranceReduction)
	.def("EnduranceRegen", &Lua_StatBonuses::GetEnduranceRegen)
	.def("extra_xtargets", &Lua_StatBonuses::Getextra_xtargets)
	.def("ExtraAttackChance", &Lua_StatBonuses::GetExtraAttackChance)
	.def("FactionModPct", &Lua_StatBonuses::GetFactionModPct)
	.def("Fearless", &Lua_StatBonuses::GetFearless)
	.def("FeignedCastOnChance", &Lua_StatBonuses::GetFeignedCastOnChance)
	.def("FinishingBlow", &Lua_StatBonuses::GetFinishingBlow)
	.def("FinishingBlowLvl", &Lua_StatBonuses::GetFinishingBlowLvl)
	.def("FlatMaxHPChange", &Lua_StatBonuses::GetFlatMaxHPChange)
	.def("FlurryChance", &Lua_StatBonuses::GetFlurryChance)
	.def("FocusEffects", &Lua_StatBonuses::GetFocusEffects)
	.def("FocusEffectsWorn", &Lua_StatBonuses::GetFocusEffectsWorn)
	.def("ForageAdditionalItems", &Lua_StatBonuses::GetForageAdditionalItems)
	.def("FR", &Lua_StatBonuses::GetFR)
	.def("FRCapMod", &Lua_StatBonuses::GetFRCapMod)
	.def("FrenziedDevastation", &Lua_StatBonuses::GetFrenziedDevastation)
	.def("FrontalBackstabChance", &Lua_StatBonuses::GetFrontalBackstabChance)
	.def("FrontalBackstabMinDmg", &Lua_StatBonuses::GetFrontalBackstabMinDmg)
	.def("FrontalStunResist", &Lua_StatBonuses::GetFrontalStunResist)
	.def("GiveDoubleAttack", &Lua_StatBonuses::GetGiveDoubleAttack)
	.def("GiveDoubleRiposte", &Lua_StatBonuses::GetGiveDoubleRiposte)
	.def("GivePetGroupTarget", &Lua_StatBonuses::GetGivePetGroupTarget)
	.def("GravityEffect", &Lua_StatBonuses::GetGravityEffect)
	.def("haste", &Lua_StatBonuses::Gethaste)
	.def("hastetype2", &Lua_StatBonuses::Gethastetype2)
	.def("hastetype3", &Lua_StatBonuses::Gethastetype3)
	.def("hatemod", &Lua_StatBonuses::Gethatemod)
	.def("HeadShot", &Lua_StatBonuses::GetHeadShot)
	.def("HealAmt", &Lua_StatBonuses::GetHealAmt)
	.def("HealRate", &Lua_StatBonuses::GetHealRate)
	.def("HeroicAGI", &Lua_StatBonuses::GetHeroicAGI)
	.def("HeroicCHA", &Lua_StatBonuses::GetHeroicCHA)
	.def("HeroicCorrup", &Lua_StatBonuses::GetHeroicCorrup)
	.def("HeroicCR", &Lua_StatBonuses::GetHeroicCR)
	.def("HeroicDEX", &Lua_StatBonuses::GetHeroicDEX)
	.def("HeroicDR", &Lua_StatBonuses::GetHeroicDR)
	.def("HeroicFR", &Lua_StatBonuses::GetHeroicFR)
	.def("HeroicINT", &Lua_StatBonuses::GetHeroicINT)
	.def("HeroicMR", &Lua_StatBonuses::GetHeroicMR)
	.def("HeroicPR", &Lua_StatBonuses::GetHeroicPR)
	.def("HeroicSTA", &Lua_StatBonuses::GetHeroicSTA)
	.def("HeroicSTR", &Lua_StatBonuses::GetHeroicSTR)
	.def("HeroicWIS", &Lua_StatBonuses::GetHeroicWIS)
	.def("HitChance", &Lua_StatBonuses::GetHitChance)
	.def("HitChanceEffect", &Lua_StatBonuses::GetHitChanceEffect)
	.def("HP", &Lua_StatBonuses::GetHP)
	.def("HPPercCap", &Lua_StatBonuses::GetHPPercCap)
	.def("HPRegen", &Lua_StatBonuses::GetHPRegen)
	.def("HPToManaConvert", &Lua_StatBonuses::GetHPToManaConvert)
	.def("HSLevel", &Lua_StatBonuses::GetHSLevel)
	.def("HundredHands", &Lua_StatBonuses::GetHundredHands)
	.def("IllusionPersistence", &Lua_StatBonuses::GetIllusionPersistence)
	.def("ImmuneToFlee", &Lua_StatBonuses::GetImmuneToFlee)
	.def("ImprovedReclaimEnergy", &Lua_StatBonuses::GetImprovedReclaimEnergy)
	.def("ImprovedTaunt", &Lua_StatBonuses::GetImprovedTaunt)
	.def("IncreaseBlockChance", &Lua_StatBonuses::GetIncreaseBlockChance)
	.def("IncreaseChanceMemwipe", &Lua_StatBonuses::GetIncreaseChanceMemwipe)
	.def("IncreaseRunSpeedCap", &Lua_StatBonuses::GetIncreaseRunSpeedCap)
	.def("inhibitmelee", &Lua_StatBonuses::Getinhibitmelee)
	.def("INT", &Lua_StatBonuses::GetINT)
	.def("INTCapMod", &Lua_StatBonuses::GetINTCapMod)
	.def("IsBlind", &Lua_StatBonuses::GetIsBlind)
	.def("IsFeared", &Lua_StatBonuses::GetIsFeared)
	.def("ItemATKCap", &Lua_StatBonuses::GetItemATKCap)
	.def("ItemHPRegenCap", &Lua_StatBonuses::GetItemHPRegenCap)
	.def("ItemManaRegenCap", &Lua_StatBonuses::GetItemManaRegenCap)
	.def("LimitToSkill", &Lua_StatBonuses::GetLimitToSkill)
	.def("MagicWeapon", &Lua_StatBonuses::GetMagicWeapon)
	.def("Mana", &Lua_StatBonuses::GetMana)
	.def("ManaAbsorbPercentDamage", &Lua_StatBonuses::GetManaAbsorbPercentDamage)
	.def("ManaPercCap", &Lua_StatBonuses::GetManaPercCap)
	.def("ManaRegen", &Lua_StatBonuses::GetManaRegen)
	.def("MasteryofPast", &Lua_StatBonuses::GetMasteryofPast)
	.def("MaxBindWound", &Lua_StatBonuses::GetMaxBindWound)
	.def("MaxHP", &Lua_StatBonuses::GetMaxHP)
	.def("MaxHPChange", &Lua_StatBonuses::GetMaxHPChange)
	.def("MeleeLifetap", &Lua_StatBonuses::GetMeleeLifetap)
	.def("MeleeMitigation", &Lua_StatBonuses::GetMeleeMitigation)
	.def("MeleeMitigationEffect", &Lua_StatBonuses::GetMeleeMitigationEffect)
	.def("MeleeRune", &Lua_StatBonuses::GetMeleeRune)
	.def("MeleeSkillCheck", &Lua_StatBonuses::GetMeleeSkillCheck)
	.def("MeleeSkillCheckSkill", &Lua_StatBonuses::GetMeleeSkillCheckSkill)
	.def("MeleeThresholdGuard", &Lua_StatBonuses::GetMeleeThresholdGuard)
	.def("Metabolism", &Lua_StatBonuses::GetMetabolism)
	.def("MinDamageModifier", &Lua_StatBonuses::GetMinDamageModifier)
	.def("MitigateDotRune", &Lua_StatBonuses::GetMitigateDotRune)
	.def("MitigateMeleeRune", &Lua_StatBonuses::GetMitigateMeleeRune)
	.def("MitigateSpellRune", &Lua_StatBonuses::GetMitigateSpellRune)
	.def("movementspeed", &Lua_StatBonuses::Getmovementspeed)
	.def("MR", &Lua_StatBonuses::GetMR)
	.def("MRCapMod", &Lua_StatBonuses::GetMRCapMod)
	.def("NegateAttacks", &Lua_StatBonuses::GetNegateAttacks)
	.def("NegateEffects", &Lua_StatBonuses::GetNegateEffects)
	.def("NegateIfCombat", &Lua_StatBonuses::GetNegateIfCombat)
	.def("NoBreakAESneak", &Lua_StatBonuses::GetNoBreakAESneak)
	.def("OffhandRiposteFail", &Lua_StatBonuses::GetOffhandRiposteFail)
	.def("Packrat", &Lua_StatBonuses::GetPackrat)
	.def("ParryChance", &Lua_StatBonuses::GetParryChance)
	.def("PC_Pet_Flurry", &Lua_StatBonuses::GetPC_Pet_Flurry)
	.def("PC_Pet_Rampage", &Lua_StatBonuses::GetPC_Pet_Rampage)
	.def("percussionMod", &Lua_StatBonuses::GetpercussionMod)
	.def("PersistantCasting", &Lua_StatBonuses::GetPersistantCasting)
	.def("PetAvoidance", &Lua_StatBonuses::GetPetAvoidance)
	.def("PetCriticalHit", &Lua_StatBonuses::GetPetCriticalHit)
	.def("PetFlurry", &Lua_StatBonuses::GetPetFlurry)
	.def("PetMaxHP", &Lua_StatBonuses::GetPetMaxHP)
	.def("PetMeleeMitigation", &Lua_StatBonuses::GetPetMeleeMitigation)
	.def("PR", &Lua_StatBonuses::GetPR)
	.def("PRCapMod", &Lua_StatBonuses::GetPRCapMod)
	.def("ProcChance", &Lua_StatBonuses::GetProcChance)
	.def("ProcChanceSPA", &Lua_StatBonuses::GetProcChanceSPA)
	.def("RaiseSkillCap", &Lua_StatBonuses::GetRaiseSkillCap)
	.def("ReduceFallDamage", &Lua_StatBonuses::GetReduceFallDamage)
	.def("ReduceTradeskillFail", &Lua_StatBonuses::GetReduceTradeskillFail)
	.def("reflect_chance", &Lua_StatBonuses::Getreflect_chance)
	.def("ResistFearChance", &Lua_StatBonuses::GetResistFearChance)
	.def("ResistSpellChance", &Lua_StatBonuses::GetResistSpellChance)
	.def("ReverseDamageShield", &Lua_StatBonuses::GetReverseDamageShield)
	.def("ReverseDamageShieldSpellID", &Lua_StatBonuses::GetReverseDamageShieldSpellID)
	.def("ReverseDamageShieldType", &Lua_StatBonuses::GetReverseDamageShieldType)
	.def("RiposteChance", &Lua_StatBonuses::GetRiposteChance)
	.def("Root", &Lua_StatBonuses::GetRoot)
	.def("RootBreakChance", &Lua_StatBonuses::GetRootBreakChance)
	.def("SalvageChance", &Lua_StatBonuses::GetSalvageChance)
	.def("Sanctuary", &Lua_StatBonuses::GetSanctuary)
	.def("Screech", &Lua_StatBonuses::GetScreech)
	.def("SecondaryDmgInc", &Lua_StatBonuses::GetSecondaryDmgInc)
	.def("SeeInvis", &Lua_StatBonuses::GetSeeInvis)
	.def("SEResist", &Lua_StatBonuses::GetSEResist)
	.def("ShieldBlock", &Lua_StatBonuses::GetShieldBlock)
	.def("ShieldEquipDmgMod", &Lua_StatBonuses::GetShieldEquipDmgMod)
	.def("ShroudofStealth", &Lua_StatBonuses::GetShroudofStealth)
	.def("singingMod", &Lua_StatBonuses::GetsingingMod)
	.def("SkillAttackProc", &Lua_StatBonuses::GetSkillAttackProc)
	.def("SkillDamageAmount", &Lua_StatBonuses::GetSkillDamageAmount)
	.def("SkillDamageAmount2", &Lua_StatBonuses::GetSkillDamageAmount2)
	.def("SkillDmgTaken", &Lua_StatBonuses::GetSkillDmgTaken)
	.def("skillmod", &Lua_StatBonuses::Getskillmod)
	.def("skillmodmax", &Lua_StatBonuses::Getskillmodmax)
	.def("SkillProc", &Lua_StatBonuses::GetSkillProc)
	.def("SkillProcSuccess", &Lua_StatBonuses::GetSkillProcSuccess)
	.def("SkillReuseTime", &Lua_StatBonuses::GetSkillReuseTime)
	.def("SlayUndead", &Lua_StatBonuses::GetSlayUndead)
	.def("songModCap", &Lua_StatBonuses::GetsongModCap)
	.def("SongRange", &Lua_StatBonuses::GetSongRange)
	.def("SpellCritDmgIncNoStack", &Lua_StatBonuses::GetSpellCritDmgIncNoStack)
	.def("SpellCritDmgIncrease", &Lua_StatBonuses::GetSpellCritDmgIncrease)
	.def("SpellDamageShield", &Lua_StatBonuses::GetSpellDamageShield)
	.def("SpellDmg", &Lua_StatBonuses::GetSpellDmg)
	.def("SpellOnDeath", &Lua_StatBonuses::GetSpellOnDeath)
	.def("SpellOnKill", &Lua_StatBonuses::GetSpellOnKill)
	.def("SpellProcChance", &Lua_StatBonuses::GetSpellProcChance)
	.def("SpellShield", &Lua_StatBonuses::GetSpellShield)
	.def("SpellThresholdGuard", &Lua_StatBonuses::GetSpellThresholdGuard)
	.def("SpellTriggers", &Lua_StatBonuses::GetSpellTriggers)
	.def("STA", &Lua_StatBonuses::GetSTA)
	.def("STACapMod", &Lua_StatBonuses::GetSTACapMod)
	.def("STR", &Lua_StatBonuses::GetSTR)
	.def("STRCapMod", &Lua_StatBonuses::GetSTRCapMod)
	.def("StrikeThrough", &Lua_StatBonuses::GetStrikeThrough)
	.def("stringedMod", &Lua_StatBonuses::GetstringedMod)
	.def("StunBashChance", &Lua_StatBonuses::GetStunBashChance)
	.def("StunResist", &Lua_StatBonuses::GetStunResist)
	.def("TradeSkillMastery", &Lua_StatBonuses::GetTradeSkillMastery)
	.def("TriggerMeleeThreshold", &Lua_StatBonuses::GetTriggerMeleeThreshold)
	.def("TriggerOnValueAmount", &Lua_StatBonuses::GetTriggerOnValueAmount)
	.def("TriggerSpellThreshold", &Lua_StatBonuses::GetTriggerSpellThreshold)
	.def("TripleAttackChance", &Lua_StatBonuses::GetTripleAttackChance)
	.def("TripleBackstab", &Lua_StatBonuses::GetTripleBackstab)
	.def("TwoHandBluntBlock", &Lua_StatBonuses::GetTwoHandBluntBlock)
	.def("UnfailingDivinity", &Lua_StatBonuses::GetUnfailingDivinity)
	.def("Vampirism", &Lua_StatBonuses::GetVampirism)
	.def("VoiceGraft", &Lua_StatBonuses::GetVoiceGraft)
	.def("windMod", &Lua_StatBonuses::GetwindMod)
	.def("WIS", &Lua_StatBonuses::GetWIS)
	.def("WISCapMod", &Lua_StatBonuses::GetWISCapMod)
	.def("XPRateMod", &Lua_StatBonuses::GetXPRateMod);
}

#endif // LUA_EQEMU
