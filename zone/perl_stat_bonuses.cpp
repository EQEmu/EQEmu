#include "common/features.h"

#ifdef EMBPERL_XS_CLASSES

#include "common/data_verification.h"
#include "zone/common.h"
#include "zone/embperl.h"

int32 Perl_StatBonuses_GetAC(StatBonuses* self)
{
	return self->AC;
}

int64 Perl_StatBonuses_GetHP(StatBonuses* self)
{
	return self->HP;
}

int64 Perl_StatBonuses_GetFlatMaxHPChange(StatBonuses* self)
{
	return self->FlatMaxHPChange;
}

int64 Perl_StatBonuses_GetHPRegen(StatBonuses* self)
{
	return self->HPRegen;
}

int64 Perl_StatBonuses_GetMaxHP(StatBonuses* self)
{
	return self->PercentMaxHPChange;
}

int64 Perl_StatBonuses_GetManaRegen(StatBonuses* self)
{
	return self->ManaRegen;
}

int64 Perl_StatBonuses_GetEnduranceRegen(StatBonuses* self)
{
	return self->EnduranceRegen;
}

int64 Perl_StatBonuses_GetMana(StatBonuses* self)
{
	return self->Mana;
}

int64 Perl_StatBonuses_GetEndurance(StatBonuses* self)
{
	return self->Endurance;
}

int32 Perl_StatBonuses_GetATK(StatBonuses* self)
{
	return self->ATK;
}

int32 Perl_StatBonuses_GetSTR(StatBonuses* self)
{
	return self->STR;
}

int32 Perl_StatBonuses_GetSTRCapModifier(StatBonuses* self)
{
	return self->STRCapMod;
}

int32 Perl_StatBonuses_GetHeroicSTR(StatBonuses* self)
{
	return self->HeroicSTR;
}

int32 Perl_StatBonuses_GetSTA(StatBonuses* self)
{
	return self->STA;
}

int32 Perl_StatBonuses_GetSTACapModifier(StatBonuses* self)
{
	return self->STACapMod;
}

int32 Perl_StatBonuses_GetHeroicSTA(StatBonuses* self)
{
	return self->HeroicSTA;
}

int32 Perl_StatBonuses_GetDEX(StatBonuses* self)
{
	return self->DEX;
}

int32 Perl_StatBonuses_GetDEXCapModifier(StatBonuses* self)
{
	return self->DEXCapMod;
}

int32 Perl_StatBonuses_GetHeroicDEX(StatBonuses* self)
{
	return self->HeroicDEX;
}

int32 Perl_StatBonuses_GetAGI(StatBonuses* self)
{
	return self->AGI;
}

int32 Perl_StatBonuses_GetAGICapModifier(StatBonuses* self)
{
	return self->AGICapMod;
}

int32 Perl_StatBonuses_GetHeroicAGI(StatBonuses* self)
{
	return self->HeroicAGI;
}

int32 Perl_StatBonuses_GetINT(StatBonuses* self)
{
	return self->INT;
}

int32 Perl_StatBonuses_GetINTCapModifier(StatBonuses* self)
{
	return self->INTCapMod;
}

int32 Perl_StatBonuses_GetHeroicINT(StatBonuses* self)
{
	return self->HeroicINT;
}

int32 Perl_StatBonuses_GetWIS(StatBonuses* self)
{
	return self->WIS;
}

int32 Perl_StatBonuses_GetWISCapModifier(StatBonuses* self)
{
	return self->WISCapMod;
}

int32 Perl_StatBonuses_GetHeroicWIS(StatBonuses* self)
{
	return self->HeroicWIS;
}

int32 Perl_StatBonuses_GetCHA(StatBonuses* self)
{
	return self->CHA;
}

int32 Perl_StatBonuses_GetCHACapModifier(StatBonuses* self)
{
	return self->CHACapMod;
}

int32 Perl_StatBonuses_GetHeroicCHA(StatBonuses* self)
{
	return self->HeroicCHA;
}

int32 Perl_StatBonuses_GetMR(StatBonuses* self)
{
	return self->MR;
}

int32 Perl_StatBonuses_GetMRCapModifier(StatBonuses* self)
{
	return self->MRCapMod;
}

int32 Perl_StatBonuses_GetHeroicMR(StatBonuses* self)
{
	return self->HeroicMR;
}

int32 Perl_StatBonuses_GetFR(StatBonuses* self)
{
	return self->FR;
}

int32 Perl_StatBonuses_GetFRCapModifier(StatBonuses* self)
{
	return self->FRCapMod;
}

int32 Perl_StatBonuses_GetHeroicFR(StatBonuses* self)
{
	return self->HeroicFR;
}

int32 Perl_StatBonuses_GetCR(StatBonuses* self)
{
	return self->CR;
}

int32 Perl_StatBonuses_GetCRCapModifier(StatBonuses* self)
{
	return self->CRCapMod;
}

int32 Perl_StatBonuses_GetHeroicCR(StatBonuses* self)
{
	return self->HeroicCR;
}

int32 Perl_StatBonuses_GetPR(StatBonuses* self)
{
	return self->PR;
}

int32 Perl_StatBonuses_GetPRCapModifier(StatBonuses* self)
{
	return self->PRCapMod;
}

int32 Perl_StatBonuses_GetHeroicPR(StatBonuses* self)
{
	return self->HeroicPR;
}

int32 Perl_StatBonuses_GetDR(StatBonuses* self)
{
	return self->DR;
}

int32 Perl_StatBonuses_GetDRCapModifier(StatBonuses* self)
{
	return self->DRCapMod;
}

int32 Perl_StatBonuses_GetHeroicDR(StatBonuses* self)
{
	return self->HeroicDR;
}

int32 Perl_StatBonuses_GetCorruption(StatBonuses* self)
{
	return self->Corrup;
}

int32 Perl_StatBonuses_GetCorruptionCapModifier(StatBonuses* self)
{
	return self->CorrupCapMod;
}

int32 Perl_StatBonuses_GetHeroicCorrup(StatBonuses* self)
{
	return self->HeroicCorrup;
}

uint16 Perl_StatBonuses_GetDamageShieldSpellID(StatBonuses* self)
{
	return self->DamageShieldSpellID;
}

int Perl_StatBonuses_GetDamageShield(StatBonuses* self)
{
	return self->DamageShield;
}

int Perl_StatBonuses_GetDamageShieldType(StatBonuses* self)
{
	return self->DamageShieldType;
}

int Perl_StatBonuses_GetSpellDamageShield(StatBonuses* self)
{
	return self->SpellDamageShield;
}

int Perl_StatBonuses_GetSpellShield(StatBonuses* self)
{
	return self->SpellShield;
}

int Perl_StatBonuses_GetReverseDamageShield(StatBonuses* self)
{
	return self->ReverseDamageShield;
}

uint16 Perl_StatBonuses_GetReverseDamageShieldSpellID(StatBonuses* self)
{
	return self->ReverseDamageShieldSpellID;
}

int Perl_StatBonuses_GetReverseDamageShieldType(StatBonuses* self)
{
	return self->ReverseDamageShieldType;
}

int Perl_StatBonuses_GetMovementSpeed(StatBonuses* self)
{
	return self->movementspeed;
}

int32 Perl_StatBonuses_GetHaste(StatBonuses* self)
{
	return self->haste;
}

int32 Perl_StatBonuses_GetHasteType2(StatBonuses* self)
{
	return self->hastetype2;
}

int32 Perl_StatBonuses_GetHasteType3(StatBonuses* self)
{
	return self->hastetype3;
}

int32 Perl_StatBonuses_GetInhibitMelee(StatBonuses* self)
{
	return self->inhibitmelee;
}

float Perl_StatBonuses_GetAggroRange(StatBonuses* self)
{
	return self->AggroRange;
}

float Perl_StatBonuses_GetAssistRange(StatBonuses* self)
{
	return self->AssistRange;
}

int32 Perl_StatBonuses_GetSkillModifier(StatBonuses* self, int slot)
{
	return self->skillmod[slot];
}

int32 Perl_StatBonuses_GetSkillModifierMax(StatBonuses* self, int slot)
{
	return self->skillmodmax[slot];
}

int Perl_StatBonuses_GetEffectiveCastingLevel(StatBonuses* self)
{
	return self->effective_casting_level;
}

int Perl_StatBonuses_GetAdjustedCastingSkill(StatBonuses* self)
{
	return self->adjusted_casting_skill;
}

int Perl_StatBonuses_GetReflectChance(StatBonuses* self)
{
	return self->reflect[SBIndex::REFLECT_CHANCE];
}

uint32 Perl_StatBonuses_GetSingingModifier(StatBonuses* self)
{
	return self->singingMod;
}

uint32 Perl_StatBonuses_GetAmplification(StatBonuses* self)
{
	return self->Amplification;
}

uint32 Perl_StatBonuses_GetBrassModifier(StatBonuses* self)
{
	return self->brassMod;
}

uint32 Perl_StatBonuses_GetPercussionModifier(StatBonuses* self)
{
	return self->percussionMod;
}

uint32 Perl_StatBonuses_GetWindModifier(StatBonuses* self)
{
	return self->windMod;
}

uint32 Perl_StatBonuses_GetStringedModifier(StatBonuses* self)
{
	return self->stringedMod;
}

uint32 Perl_StatBonuses_GetSongModifierCap(StatBonuses* self)
{
	return self->songModCap;
}

int32 Perl_StatBonuses_GetHateModifier(StatBonuses* self)
{
	return self->hatemod;
}

int64 Perl_StatBonuses_GetEnduranceReduction(StatBonuses* self)
{
	return self->EnduranceReduction;
}

int32 Perl_StatBonuses_GetStrikeThrough(StatBonuses* self)
{
	return self->StrikeThrough;
}

int32 Perl_StatBonuses_GetMeleeMitigation(StatBonuses* self)
{
	return self->MeleeMitigation;
}

int32 Perl_StatBonuses_GetMeleeMitigationEffect(StatBonuses* self)
{
	return self->MeleeMitigationEffect;
}

int32 Perl_StatBonuses_GetCriticalHitChance(StatBonuses* self, int slot)
{
	return self->CriticalHitChance[slot];
}

int32 Perl_StatBonuses_GetCriticalSpellChance(StatBonuses* self)
{
	return self->CriticalSpellChance;
}

int32 Perl_StatBonuses_GetSpellCriticalDamageIncrease(StatBonuses* self)
{
	return self->SpellCritDmgIncrease;
}

int32 Perl_StatBonuses_GetSpellCriticalDamageIncreaseNoStack(StatBonuses* self)
{
	return self->SpellCritDmgIncNoStack;
}

int32 Perl_StatBonuses_GetDOTCriticalDamageIncrease(StatBonuses* self)
{
	return self->DotCritDmgIncrease;
}

int32 Perl_StatBonuses_GetCriticalHealChance(StatBonuses* self)
{
	return self->CriticalHealChance;
}

int32 Perl_StatBonuses_GetCriticalHealOverTime(StatBonuses* self)
{
	return self->CriticalHealOverTime;
}

int32 Perl_StatBonuses_GetCriticalDOTChance(StatBonuses* self)
{
	return self->CriticalDoTChance;
}

int32 Perl_StatBonuses_GetCripplingBlowChance(StatBonuses* self)
{
	return self->CrippBlowChance;
}

int32 Perl_StatBonuses_GetAvoidMeleeChance(StatBonuses* self)
{
	return self->AvoidMeleeChance;
}

int32 Perl_StatBonuses_GetAvoidMeleeChanceEffect(StatBonuses* self)
{
	return self->AvoidMeleeChanceEffect;
}

int32 Perl_StatBonuses_GetRiposteChance(StatBonuses* self)
{
	return self->RiposteChance;
}

int32 Perl_StatBonuses_GetDodgeChance(StatBonuses* self)
{
	return self->DodgeChance;
}

int32 Perl_StatBonuses_GetParryChance(StatBonuses* self)
{
	return self->ParryChance;
}

int32 Perl_StatBonuses_GetDualWieldChance(StatBonuses* self)
{
	return self->DualWieldChance;
}

int32 Perl_StatBonuses_GetDoubleAttackChance(StatBonuses* self)
{
	return self->DoubleAttackChance;
}

int32 Perl_StatBonuses_GetTripleAttackChance(StatBonuses* self)
{
	return self->TripleAttackChance;
}

int32 Perl_StatBonuses_GetDoubleRangedAttack(StatBonuses* self)
{
	return self->DoubleRangedAttack;
}

int32 Perl_StatBonuses_GetResistSpellChance(StatBonuses* self)
{
	return self->ResistSpellChance;
}

int32 Perl_StatBonuses_GetResistFearChance(StatBonuses* self)
{
	return self->ResistFearChance;
}

bool Perl_StatBonuses_GetFearless(StatBonuses* self)
{
	return self->Fearless;
}

bool Perl_StatBonuses_GetIsFeared(StatBonuses* self)
{
	return self->IsFeared;
}

bool Perl_StatBonuses_GetIsBlind(StatBonuses* self)
{
	return self->IsBlind;
}

int32 Perl_StatBonuses_GetStunResist(StatBonuses* self)
{
	return self->StunResist;
}

int32 Perl_StatBonuses_GetMeleeSkillCheck(StatBonuses* self)
{
	return self->MeleeSkillCheck;
}

uint8 Perl_StatBonuses_GetMeleeSkillCheckSkill(StatBonuses* self)
{
	return self->MeleeSkillCheckSkill;
}

int32 Perl_StatBonuses_GetHitChance(StatBonuses* self)
{
	return self->HitChance;
}

int32 Perl_StatBonuses_GetHitChanceEffect(StatBonuses* self, int slot)
{
	return self->HitChanceEffect[slot];
}

int32 Perl_StatBonuses_GetDamageModifier(StatBonuses* self, int slot)
{
	return self->DamageModifier[slot];
}

int32 Perl_StatBonuses_GetDamageModifier2(StatBonuses* self, int slot)
{
	return self->DamageModifier2[slot];
}

int32 Perl_StatBonuses_GetMinimumDamageModifier(StatBonuses* self, int slot)
{
	return self->MinDamageModifier[slot];
}

int32 Perl_StatBonuses_GetProcChance(StatBonuses* self)
{
	return self->ProcChance;
}

int32 Perl_StatBonuses_GetProcChanceSPA(StatBonuses* self)
{
	return self->ProcChanceSPA;
}

int32 Perl_StatBonuses_GetExtraAttackChance(StatBonuses* self)
{
	return self->ExtraAttackChance[0];
}

int32 Perl_StatBonuses_GetDOTShielding(StatBonuses* self)
{
	return self->DoTShielding;
}

int32 Perl_StatBonuses_GetFlurryChance(StatBonuses* self)
{
	return self->FlurryChance;
}

int32 Perl_StatBonuses_GetHundredHands(StatBonuses* self)
{
	return self->HundredHands;
}

int32 Perl_StatBonuses_GetMeleeLifetap(StatBonuses* self)
{
	return self->MeleeLifetap;
}

int32 Perl_StatBonuses_GetVampirism(StatBonuses* self)
{
	return self->Vampirism;
}

int32 Perl_StatBonuses_GetHealRate(StatBonuses* self)
{
	return self->HealRate;
}

int32 Perl_StatBonuses_GetMaxHPChange(StatBonuses* self)
{
	return self->PercentMaxHPChange;
}

int32 Perl_StatBonuses_GetHealAmt(StatBonuses* self)
{
	return self->HealAmt;
}

int32 Perl_StatBonuses_GetSpellDamage(StatBonuses* self)
{
	return self->SpellDmg;
}

int32 Perl_StatBonuses_GetClairvoyance(StatBonuses* self)
{
	return self->Clairvoyance;
}

int32 Perl_StatBonuses_GetDSMitigation(StatBonuses* self)
{
	return self->DSMitigation;
}

int32 Perl_StatBonuses_GetDSMitigationOffHand(StatBonuses* self)
{
	return self->DSMitigationOffHand;
}

int32 Perl_StatBonuses_GetTwoHandBluntBlock(StatBonuses* self)
{
	return self->TwoHandBluntBlock;
}

uint32 Perl_StatBonuses_GetItemManaRegenCap(StatBonuses* self)
{
	return self->ItemManaRegenCap;
}

int32 Perl_StatBonuses_GetGravityEffect(StatBonuses* self)
{
	return self->GravityEffect;
}

bool Perl_StatBonuses_GetAntiGate(StatBonuses* self)
{
	return self->AntiGate;
}

bool Perl_StatBonuses_GetMagicWeapon(StatBonuses* self)
{
	return self->MagicWeapon;
}

int32 Perl_StatBonuses_GetIncreaseBlockChance(StatBonuses* self)
{
	return self->IncreaseBlockChance;
}

uint32 Perl_StatBonuses_GetPersistentCasting(StatBonuses* self)
{
	return self->PersistantCasting;
}

int Perl_StatBonuses_GetXPRateModifier(StatBonuses* self)
{
	return self->XPRateMod;
}

bool Perl_StatBonuses_GetImmuneToFlee(StatBonuses* self)
{
	return self->ImmuneToFlee;
}

uint32 Perl_StatBonuses_GetVoiceGraft(StatBonuses* self)
{
	return self->VoiceGraft;
}

int32 Perl_StatBonuses_GetSpellProcChance(StatBonuses* self)
{
	return self->SpellProcChance;
}

int32 Perl_StatBonuses_GetCharmBreakChance(StatBonuses* self)
{
	return self->CharmBreakChance;
}

int32 Perl_StatBonuses_GetSongRange(StatBonuses* self)
{
	return self->SongRange;
}

uint32 Perl_StatBonuses_GetHPToManaConvert(StatBonuses* self)
{
	return self->HPToManaConvert;
}

bool Perl_StatBonuses_GetNegateEffects(StatBonuses* self)
{
	return self->NegateEffects;
}

bool Perl_StatBonuses_GetTriggerMeleeThreshold(StatBonuses* self)
{
	return self->TriggerMeleeThreshold;
}

bool Perl_StatBonuses_GetTriggerSpellThreshold(StatBonuses* self)
{
	return self->TriggerSpellThreshold;
}

int32 Perl_StatBonuses_GetShieldBlock(StatBonuses* self)
{
	return self->ShieldBlock;
}

int32 Perl_StatBonuses_GetBlockBehind(StatBonuses* self)
{
	return self->BlockBehind;
}

bool Perl_StatBonuses_GetCriticalRegenDecay(StatBonuses* self)
{
	return self->CriticalRegenDecay;
}

bool Perl_StatBonuses_GetCriticalHealDecay(StatBonuses* self)
{
	return self->CriticalHealDecay;
}

bool Perl_StatBonuses_GetCriticalDOTDecay(StatBonuses* self)
{
	return self->CriticalDotDecay;
}

bool Perl_StatBonuses_GetDivineAura(StatBonuses* self)
{
	return self->DivineAura;
}

bool Perl_StatBonuses_GetDistanceRemoval(StatBonuses* self)
{
	return self->DistanceRemoval;
}

int32 Perl_StatBonuses_GetFrenziedDevastation(StatBonuses* self)
{
	return self->FrenziedDevastation;
}

bool Perl_StatBonuses_GetNegateIfCombat(StatBonuses* self)
{
	return self->NegateIfCombat;
}

int8 Perl_StatBonuses_GetScreech(StatBonuses* self)
{
	return self->Screech;
}

int32 Perl_StatBonuses_GetAlterNPCLevel(StatBonuses* self)
{
	return self->AlterNPCLevel;
}

bool Perl_StatBonuses_GetBerserkSPA(StatBonuses* self)
{
	return self->BerserkSPA;
}

int32 Perl_StatBonuses_GetMetabolism(StatBonuses* self)
{
	return self->Metabolism;
}

bool Perl_StatBonuses_GetSanctuary(StatBonuses* self)
{
	return self->Sanctuary;
}

int32 Perl_StatBonuses_GetFactionModifierPercent(StatBonuses* self)
{
	return self->FactionModPct;
}

uint32 Perl_StatBonuses_GetPCPetFlurry(StatBonuses* self)
{
	return self->PC_Pet_Flurry;
}

int8 Perl_StatBonuses_GetPackrat(StatBonuses* self)
{
	return self->Packrat;
}

uint8 Perl_StatBonuses_GetBuffSlotIncrease(StatBonuses* self)
{
	return self->BuffSlotIncrease;
}

uint32 Perl_StatBonuses_GetDelayDeath(StatBonuses* self)
{
	return self->DelayDeath;
}

int8 Perl_StatBonuses_GetBaseMovementSpeed(StatBonuses* self)
{
	return self->BaseMovementSpeed;
}

uint8 Perl_StatBonuses_GetIncreaseRunSpeedCap(StatBonuses* self)
{
	return self->IncreaseRunSpeedCap;
}

int32 Perl_StatBonuses_GetDoubleSpecialAttack(StatBonuses* self)
{
	return self->DoubleSpecialAttack;
}

uint8 Perl_StatBonuses_GetFrontalStunResist(StatBonuses* self)
{
	return self->FrontalStunResist;
}

int32 Perl_StatBonuses_GetBindWound(StatBonuses* self)
{
	return self->BindWound;
}

int32 Perl_StatBonuses_GetMaxBindWound(StatBonuses* self)
{
	return self->MaxBindWound;
}

int32 Perl_StatBonuses_GetChannelChanceSpells(StatBonuses* self)
{
	return self->ChannelChanceSpells;
}

int32 Perl_StatBonuses_GetChannelChanceItems(StatBonuses* self)
{
	return self->ChannelChanceItems;
}

uint8 Perl_StatBonuses_GetSeeInvis(StatBonuses* self)
{
	return self->SeeInvis;
}

uint8 Perl_StatBonuses_GetTripleBackstab(StatBonuses* self)
{
	return self->TripleBackstab;
}

bool Perl_StatBonuses_GetFrontalBackstabMinimumDamage(StatBonuses* self)
{
	return self->FrontalBackstabMinDmg;
}

uint8 Perl_StatBonuses_GetFrontalBackstabChance(StatBonuses* self)
{
	return self->FrontalBackstabChance;
}

uint8 Perl_StatBonuses_GetConsumeProjectile(StatBonuses* self)
{
	return self->ConsumeProjectile;
}

uint8 Perl_StatBonuses_GetForageAdditionalItems(StatBonuses* self)
{
	return self->ForageAdditionalItems;
}

uint8 Perl_StatBonuses_GetSalvageChance(StatBonuses* self)
{
	return self->SalvageChance;
}

uint32 Perl_StatBonuses_GetArcheryDamageModifier(StatBonuses* self)
{
	return self->ArcheryDamageModifier;
}

bool Perl_StatBonuses_GetSecondaryDamageIncrease(StatBonuses* self)
{
	return self->SecondaryDmgInc;
}

uint32 Perl_StatBonuses_GetGiveDoubleAttack(StatBonuses* self)
{
	return self->GiveDoubleAttack;
}

int32 Perl_StatBonuses_GetPetCriticalHit(StatBonuses* self)
{
	return self->PetCriticalHit;
}

int32 Perl_StatBonuses_GetPetAvoidance(StatBonuses* self)
{
	return self->PetAvoidance;
}

int32 Perl_StatBonuses_GetCombatStability(StatBonuses* self)
{
	return self->CombatStability;
}

int32 Perl_StatBonuses_GetDoubleRiposte(StatBonuses* self)
{
	return self->DoubleRiposte;
}

int32 Perl_StatBonuses_GetAmbidexterity(StatBonuses* self)
{
	return self->Ambidexterity;
}

int32 Perl_StatBonuses_GetPetMaxHP(StatBonuses* self)
{
	return self->PetMaxHP;
}

int32 Perl_StatBonuses_GetPetFlurry(StatBonuses* self)
{
	return self->PetFlurry;
}

uint8 Perl_StatBonuses_GetMasteryOfPast(StatBonuses* self)
{
	return self->MasteryofPast;
}

bool Perl_StatBonuses_GetGivePetGroupTarget(StatBonuses* self)
{
	return self->GivePetGroupTarget;
}

int32 Perl_StatBonuses_GetRootBreakChance(StatBonuses* self)
{
	return self->RootBreakChance;
}

int32 Perl_StatBonuses_GetUnfailingDivinity(StatBonuses* self)
{
	return self->UnfailingDivinity;
}

int32 Perl_StatBonuses_GetItemHPRegenCap(StatBonuses* self)
{
	return self->ItemHPRegenCap;
}

int32 Perl_StatBonuses_GetOffhandRiposteFail(StatBonuses* self)
{
	return self->OffhandRiposteFail;
}

int32 Perl_StatBonuses_GetItemATKCap(StatBonuses* self)
{
	return self->ItemATKCap;
}

int32 Perl_StatBonuses_GetShieldEquipDamageModifier(StatBonuses* self)
{
	return self->ShieldEquipDmgMod;
}

bool Perl_StatBonuses_GetTriggerOnValueAmount(StatBonuses* self)
{
	return self->TriggerOnCastRequirement;
}

int8 Perl_StatBonuses_GetStunBashChance(StatBonuses* self)
{
	return self->StunBashChance;
}

int8 Perl_StatBonuses_GetIncreaseChanceMemoryWipe(StatBonuses* self)
{
	return self->IncreaseChanceMemwipe;
}

int8 Perl_StatBonuses_GetCriticalMend(StatBonuses* self)
{
	return self->CriticalMend;
}

int32 Perl_StatBonuses_GetImprovedReclaimEnergy(StatBonuses* self)
{
	return self->ImprovedReclaimEnergy;
}

int32 Perl_StatBonuses_GetPetMeleeMitigation(StatBonuses* self)
{
	return self->PetMeleeMitigation;
}

bool Perl_StatBonuses_GetIllusionPersistence(StatBonuses* self)
{
	return self->IllusionPersistence;
}

uint16 Perl_StatBonuses_GetExtraXTargets(StatBonuses* self)
{
	return self->extra_xtargets;
}

bool Perl_StatBonuses_GetShroudOfStealth(StatBonuses* self)
{
	return self->ShroudofStealth;
}

uint16 Perl_StatBonuses_GetReduceFallDamage(StatBonuses* self)
{
	return self->ReduceFallDamage;
}

uint8 Perl_StatBonuses_GetTradeSkillMastery(StatBonuses* self)
{
	return self->TradeSkillMastery;
}

int16 Perl_StatBonuses_GetNoBreakAESneak(StatBonuses* self)
{
	return self->NoBreakAESneak;
}

int16 Perl_StatBonuses_GetFeignedCastOnChance(StatBonuses* self)
{
	return self->FeignedCastOnChance;
}

int32 Perl_StatBonuses_GetDivineSaveChance(StatBonuses* self, int slot)
{
	return self->DivineSaveChance[slot];
}

uint32 Perl_StatBonuses_GetDeathSave(StatBonuses* self, int slot)
{
	return self->DeathSave[slot];
}

int32 Perl_StatBonuses_GetAccuracy(StatBonuses* self, int slot)
{
	return self->Accuracy[slot];
}

int16 Perl_StatBonuses_GetSkillDamageTaken(StatBonuses* self, int slot)
{
	return self->SkillDmgTaken[slot];
}

uint32 Perl_StatBonuses_GetSpellTriggers(StatBonuses* self, int slot)
{
	return self->SpellTriggers[slot];
}

uint32 Perl_StatBonuses_GetSpellOnKill(StatBonuses* self, int slot)
{
	return self->SpellOnKill[slot];
}

uint32 Perl_StatBonuses_GetSpellOnDeath(StatBonuses* self, int slot)
{
	return self->SpellOnDeath[slot];
}

int32 Perl_StatBonuses_GetCriticalDamageModifier(StatBonuses* self, int slot)
{
	return self->CritDmgMod[slot];
}

int32 Perl_StatBonuses_GetSkillReuseTime(StatBonuses* self, int slot)
{
	return self->SkillReuseTime[slot];
}

int32 Perl_StatBonuses_GetSkillDamageAmount(StatBonuses* self, int slot)
{
	if (!EQ::ValueWithin(slot, ALL_SKILLS, EQ::skills::HIGHEST_SKILL))
	{
		return 0;
	}

	return slot == ALL_SKILLS ? self->SkillDamageAmount[EQ::skills::HIGHEST_SKILL + 1] : self->SkillDamageAmount[slot];
}

int Perl_StatBonuses_GetHPPercentCap(StatBonuses* self, int slot)
{
	return self->HPPercCap[slot];
}

int Perl_StatBonuses_GetManaPercentCap(StatBonuses* self, int slot)
{
	return self->ManaPercCap[slot];
}

int Perl_StatBonuses_GetEndurancePercentCAp(StatBonuses* self, int slot)
{
	return self->EndPercCap[slot];
}

uint8 Perl_StatBonuses_GetFocusEffects(StatBonuses* self, int slot)
{
	return self->FocusEffects[slot];
}

int16 Perl_StatBonuses_GetFocusEffectsWorn(StatBonuses* self, int slot)
{
	return self->FocusEffectsWorn[slot];
}

int32 Perl_StatBonuses_GetSkillDamageAmount2(StatBonuses* self, int slot)
{
	return self->SkillDamageAmount2[slot];
}

uint32 Perl_StatBonuses_GetNegateAttacks(StatBonuses* self, int slot)
{
	return self->NegateAttacks[slot];
}

uint32 Perl_StatBonuses_GetMitigateMeleeRune(StatBonuses* self, int slot)
{
	return self->MitigateMeleeRune[slot];
}

uint32 Perl_StatBonuses_GetMeleeThresholdGuard(StatBonuses* self, int slot)
{
	return self->MeleeThresholdGuard[slot];
}

uint32 Perl_StatBonuses_GetSpellThresholdGuard(StatBonuses* self, int slot)
{
	return self->SpellThresholdGuard[slot];
}

uint32 Perl_StatBonuses_GetMitigateSpellRune(StatBonuses* self, int slot)
{
	return self->MitigateSpellRune[slot];
}

uint32 Perl_StatBonuses_GetMitigateDOTRune(StatBonuses* self, int slot)
{
	return self->MitigateDotRune[slot];
}

uint32 Perl_StatBonuses_GetManaAbsorbPercentDamage(StatBonuses* self, int slot)
{
	return self->ManaAbsorbPercentDamage;
}

int32 Perl_StatBonuses_GetImprovedTaunt(StatBonuses* self, int slot)
{
	return self->ImprovedTaunt[slot];
}

int8 Perl_StatBonuses_GetRoot(StatBonuses* self, int slot)
{
	return self->Root[slot];
}

uint32 Perl_StatBonuses_GetAbsorbMagicAttack(StatBonuses* self, int slot)
{
	return self->AbsorbMagicAtt[slot];
}

uint32 Perl_StatBonuses_GetMeleeRune(StatBonuses* self, int slot)
{
	return self->MeleeRune[slot];
}

int32 Perl_StatBonuses_GetAStacker(StatBonuses* self, int slot)
{
	return self->AStacker[slot];
}

int32 Perl_StatBonuses_GetBStacker(StatBonuses* self, int slot)
{
	return self->BStacker[slot];
}

int32 Perl_StatBonuses_GetCStacker(StatBonuses* self, int slot)
{
	return self->CStacker[slot];
}

int32 Perl_StatBonuses_GetDStacker(StatBonuses* self, int slot)
{
	return self->DStacker[slot];
}

bool Perl_StatBonuses_GetLimitToSkill(StatBonuses* self, int slot)
{
	return self->LimitToSkill[slot];
}

uint32 Perl_StatBonuses_GetSkillProc(StatBonuses* self, int slot)
{
	return self->SkillProc[slot];
}

uint32 Perl_StatBonuses_GetSkillProcSuccess(StatBonuses* self, int slot)
{
	return self->SkillProcSuccess[slot];
}

uint32 Perl_StatBonuses_GetPCPetRampage(StatBonuses* self, int slot)
{
	return self->PC_Pet_Rampage[slot];
}

int32 Perl_StatBonuses_GetSkillAttackProc(StatBonuses* self, int slot)
{
	return self->SkillAttackProc[slot];
}

int32 Perl_StatBonuses_GetSlayUndead(StatBonuses* self, int slot)
{
	return self->SlayUndead[slot];
}

int32 Perl_StatBonuses_GetGiveDoubleRiposte(StatBonuses* self, int slot)
{
	return self->GiveDoubleRiposte[slot];
}

uint32 Perl_StatBonuses_GetRaiseSkillCap(StatBonuses* self, int slot)
{
	return self->RaiseSkillCap[slot];
}

int32 Perl_StatBonuses_GetSEResist(StatBonuses* self, int slot)
{
	return self->SEResist[slot];
}

int32 Perl_StatBonuses_GetFinishingBlow(StatBonuses* self, int slot)
{
	return self->FinishingBlow[slot];
}

uint32 Perl_StatBonuses_GetFinishingBlowLevel(StatBonuses* self, int slot)
{
	return self->FinishingBlowLvl[slot];
}

uint32 Perl_StatBonuses_GetHeadShot(StatBonuses* self, int slot)
{
	return self->HeadShot[slot];
}

uint8 Perl_StatBonuses_GetHeadShotLevel(StatBonuses* self, int slot)
{
	return self->HSLevel[slot];
}

uint32 Perl_StatBonuses_GetAssassinate(StatBonuses* self, int slot)
{
	return self->Assassinate[slot];
}

uint8 Perl_StatBonuses_GetAssassinateLevel(StatBonuses* self, int slot)
{
	return self->AssassinateLevel[slot];
}

int32 Perl_StatBonuses_GetReduceTradeskillFail(StatBonuses* self, int slot)
{
	return self->ReduceTradeskillFail[slot];
}

void perl_register_stat_bonuses()
{
	perl::interpreter perl(PERL_GET_THX);

	auto package = perl.new_class<StatBonuses>("StatBonuses");
	package.add("GetAbsorbMagicAttack", &Perl_StatBonuses_GetAbsorbMagicAttack);
	package.add("GetAC", &Perl_StatBonuses_GetAC);
	package.add("GetAccuracy", &Perl_StatBonuses_GetAccuracy);
	package.add("GetAdjustedCastingSkill", &Perl_StatBonuses_GetAdjustedCastingSkill);
	package.add("GetAggroRange", &Perl_StatBonuses_GetAggroRange);
	package.add("GetAGI", &Perl_StatBonuses_GetAGI);
	package.add("GetAGICapModifier", &Perl_StatBonuses_GetAGICapModifier);
	package.add("GetAlterNPCLevel", &Perl_StatBonuses_GetAlterNPCLevel);
	package.add("GetAmbidexterity", &Perl_StatBonuses_GetAmbidexterity);
	package.add("GetAmplification", &Perl_StatBonuses_GetAmplification);
	package.add("GetAntiGate", &Perl_StatBonuses_GetAntiGate);
	package.add("GetArcheryDamageModifier", &Perl_StatBonuses_GetArcheryDamageModifier);
	package.add("GetAssassinate", &Perl_StatBonuses_GetAssassinate);
	package.add("GetAssassinateLevel", &Perl_StatBonuses_GetAssassinateLevel);
	package.add("GetAssistRange", &Perl_StatBonuses_GetAssistRange);
	package.add("GetAStacker", &Perl_StatBonuses_GetAStacker);
	package.add("GetATK", &Perl_StatBonuses_GetATK);
	package.add("GetAvoidMeleeChance", &Perl_StatBonuses_GetAvoidMeleeChance);
	package.add("GetAvoidMeleeChanceEffect", &Perl_StatBonuses_GetAvoidMeleeChanceEffect);
	package.add("GetBaseMovementSpeed", &Perl_StatBonuses_GetBaseMovementSpeed);
	package.add("GetBerserkSPA", &Perl_StatBonuses_GetBerserkSPA);
	package.add("GetBindWound", &Perl_StatBonuses_GetBindWound);
	package.add("GetBlockBehind", &Perl_StatBonuses_GetBlockBehind);
	package.add("GetBrassModifier", &Perl_StatBonuses_GetBrassModifier);
	package.add("GetBStacker", &Perl_StatBonuses_GetBStacker);
	package.add("GetBuffSlotIncrease", &Perl_StatBonuses_GetBuffSlotIncrease);
	package.add("GetCHA", &Perl_StatBonuses_GetCHA);
	package.add("GetCHACapModifier", &Perl_StatBonuses_GetCHACapModifier);
	package.add("GetChannelChanceItems", &Perl_StatBonuses_GetChannelChanceItems);
	package.add("GetChannelChanceSpells", &Perl_StatBonuses_GetChannelChanceSpells);
	package.add("GetCharmBreakChance", &Perl_StatBonuses_GetCharmBreakChance);
	package.add("GetClairvoyance", &Perl_StatBonuses_GetClairvoyance);
	package.add("GetCombatStability", &Perl_StatBonuses_GetCombatStability);
	package.add("GetConsumeProjectile", &Perl_StatBonuses_GetConsumeProjectile);
	package.add("GetCorruption", &Perl_StatBonuses_GetCorruption);
	package.add("GetCorruptionCapModifier", &Perl_StatBonuses_GetCorruptionCapModifier);
	package.add("GetCR", &Perl_StatBonuses_GetCR);
	package.add("GetCRCapModifier", &Perl_StatBonuses_GetCRCapModifier);
	package.add("GetCripplingBlowChance", &Perl_StatBonuses_GetCripplingBlowChance);
	package.add("GetCriticalDamageModifier", &Perl_StatBonuses_GetCriticalDamageModifier);
	package.add("GetCriticalDOTChance", &Perl_StatBonuses_GetCriticalDOTChance);
	package.add("GetCriticalDOTDecay", &Perl_StatBonuses_GetCriticalDOTDecay);
	package.add("GetCriticalHealChance", &Perl_StatBonuses_GetCriticalHealChance);
	package.add("GetCriticalHealDecay", &Perl_StatBonuses_GetCriticalHealDecay);
	package.add("GetCriticalHealOverTime", &Perl_StatBonuses_GetCriticalHealOverTime);
	package.add("GetCriticalHitChance", &Perl_StatBonuses_GetCriticalHitChance);
	package.add("GetCriticalMend", &Perl_StatBonuses_GetCriticalMend);
	package.add("GetCriticalRegenDecay", &Perl_StatBonuses_GetCriticalRegenDecay);
	package.add("GetCriticalSpellChance", &Perl_StatBonuses_GetCriticalSpellChance);
	package.add("GetCStacker", &Perl_StatBonuses_GetCStacker);
	package.add("GetDamageModifier", &Perl_StatBonuses_GetDamageModifier);
	package.add("GetDamageModifier2", &Perl_StatBonuses_GetDamageModifier2);
	package.add("GetDamageShield", &Perl_StatBonuses_GetDamageShield);
	package.add("GetDamageShieldSpellID", &Perl_StatBonuses_GetDamageShieldSpellID);
	package.add("GetDamageShieldType", &Perl_StatBonuses_GetDamageShieldType);
	package.add("GetDeathSave", &Perl_StatBonuses_GetDeathSave);
	package.add("GetDelayDeath", &Perl_StatBonuses_GetDelayDeath);
	package.add("GetDEX", &Perl_StatBonuses_GetDEX);
	package.add("GetDEXCapModifier", &Perl_StatBonuses_GetDEXCapModifier);
	package.add("GetDistanceRemoval", &Perl_StatBonuses_GetDistanceRemoval);
	package.add("GetDivineAura", &Perl_StatBonuses_GetDivineAura);
	package.add("GetDivineSaveChance", &Perl_StatBonuses_GetDivineSaveChance);
	package.add("GetDodgeChance", &Perl_StatBonuses_GetDodgeChance);
	package.add("GetDOTCriticalDamageIncrease", &Perl_StatBonuses_GetDOTCriticalDamageIncrease);
	package.add("GetDOTShielding", &Perl_StatBonuses_GetDOTShielding);
	package.add("GetDoubleAttackChance", &Perl_StatBonuses_GetDoubleAttackChance);
	package.add("GetDoubleRangedAttack", &Perl_StatBonuses_GetDoubleRangedAttack);
	package.add("GetDoubleRiposte", &Perl_StatBonuses_GetDoubleRiposte);
	package.add("GetDoubleSpecialAttack", &Perl_StatBonuses_GetDoubleSpecialAttack);
	package.add("GetDR", &Perl_StatBonuses_GetDR);
	package.add("GetDRCapModifier", &Perl_StatBonuses_GetDRCapModifier);
	package.add("GetDSMitigation", &Perl_StatBonuses_GetDSMitigation);
	package.add("GetDSMitigationOffHand", &Perl_StatBonuses_GetDSMitigationOffHand);
	package.add("GetDStacker", &Perl_StatBonuses_GetDStacker);
	package.add("GetDualWieldChance", &Perl_StatBonuses_GetDualWieldChance);
	package.add("GetEffectiveCastingLevel", &Perl_StatBonuses_GetEffectiveCastingLevel);
	package.add("GetEndurancePercentCAp", &Perl_StatBonuses_GetEndurancePercentCAp);
	package.add("GetEndurance", &Perl_StatBonuses_GetEndurance);
	package.add("GetEnduranceReduction", &Perl_StatBonuses_GetEnduranceReduction);
	package.add("GetEnduranceRegen", &Perl_StatBonuses_GetEnduranceRegen);
	package.add("GetExtraXTargets", &Perl_StatBonuses_GetExtraXTargets);
	package.add("GetExtraAttackChance", &Perl_StatBonuses_GetExtraAttackChance);
	package.add("GetFactionModifierPercent", &Perl_StatBonuses_GetFactionModifierPercent);
	package.add("GetFearless", &Perl_StatBonuses_GetFearless);
	package.add("GetFeignedCastOnChance", &Perl_StatBonuses_GetFeignedCastOnChance);
	package.add("GetFinishingBlow", &Perl_StatBonuses_GetFinishingBlow);
	package.add("GetFinishingBlowLevel", &Perl_StatBonuses_GetFinishingBlowLevel);
	package.add("GetFlurryChance", &Perl_StatBonuses_GetFlurryChance);
	package.add("GetFocusEffects", &Perl_StatBonuses_GetFocusEffects);
	package.add("GetFocusEffectsWorn", &Perl_StatBonuses_GetFocusEffectsWorn);
	package.add("GetForageAdditionalItems", &Perl_StatBonuses_GetForageAdditionalItems);
	package.add("GetFR", &Perl_StatBonuses_GetFR);
	package.add("GetFRCapModifier", &Perl_StatBonuses_GetFRCapModifier);
	package.add("GetFrenziedDevastation", &Perl_StatBonuses_GetFrenziedDevastation);
	package.add("GetFrontalBackstabChance", &Perl_StatBonuses_GetFrontalBackstabChance);
	package.add("GetFrontalBackstabMinimumDamage", &Perl_StatBonuses_GetFrontalBackstabMinimumDamage);
	package.add("GetFrontalStunResist", &Perl_StatBonuses_GetFrontalStunResist);
	package.add("GetGiveDoubleAttack", &Perl_StatBonuses_GetGiveDoubleAttack);
	package.add("GetGiveDoubleRiposte", &Perl_StatBonuses_GetGiveDoubleRiposte);
	package.add("GetGivePetGroupTarget", &Perl_StatBonuses_GetGivePetGroupTarget);
	package.add("GetGravityEffect", &Perl_StatBonuses_GetGravityEffect);
	package.add("GetHaste", &Perl_StatBonuses_GetHaste);
	package.add("GetHasteType2", &Perl_StatBonuses_GetHasteType2);
	package.add("GetHasteType3", &Perl_StatBonuses_GetHasteType3);
	package.add("GetHateModifier", &Perl_StatBonuses_GetHateModifier);
	package.add("GetHeadShot", &Perl_StatBonuses_GetHeadShot);
	package.add("GetHeadShotLevel", &Perl_StatBonuses_GetHeadShotLevel);
	package.add("GetHealAmt", &Perl_StatBonuses_GetHealAmt);
	package.add("GetHealRate", &Perl_StatBonuses_GetHealRate);
	package.add("GetHeroicAGI", &Perl_StatBonuses_GetHeroicAGI);
	package.add("GetHeroicCHA", &Perl_StatBonuses_GetHeroicCHA);
	package.add("GetHeroicCorrup", &Perl_StatBonuses_GetHeroicCorrup);
	package.add("GetHeroicCR", &Perl_StatBonuses_GetHeroicCR);
	package.add("GetHeroicDEX", &Perl_StatBonuses_GetHeroicDEX);
	package.add("GetHeroicDR", &Perl_StatBonuses_GetHeroicDR);
	package.add("GetHeroicFR", &Perl_StatBonuses_GetHeroicFR);
	package.add("GetHeroicINT", &Perl_StatBonuses_GetHeroicINT);
	package.add("GetHeroicMR", &Perl_StatBonuses_GetHeroicMR);
	package.add("GetHeroicPR", &Perl_StatBonuses_GetHeroicPR);
	package.add("GetHeroicSTA", &Perl_StatBonuses_GetHeroicSTA);
	package.add("GetHeroicSTR", &Perl_StatBonuses_GetHeroicSTR);
	package.add("GetHeroicWIS", &Perl_StatBonuses_GetHeroicWIS);
	package.add("GetHitChance", &Perl_StatBonuses_GetHitChance);
	package.add("GetHitChanceEffect", &Perl_StatBonuses_GetHitChanceEffect);
	package.add("GetHP", &Perl_StatBonuses_GetHP);
	package.add("GetHPPercentCap", &Perl_StatBonuses_GetHPPercentCap);
	package.add("GetHPRegen", &Perl_StatBonuses_GetHPRegen);
	package.add("GetHPToManaConvert", &Perl_StatBonuses_GetHPToManaConvert);
	package.add("GetHundredHands", &Perl_StatBonuses_GetHundredHands);
	package.add("GetIllusionPersistence", &Perl_StatBonuses_GetIllusionPersistence);
	package.add("GetImmuneToFlee", &Perl_StatBonuses_GetImmuneToFlee);
	package.add("GetImprovedReclaimEnergy", &Perl_StatBonuses_GetImprovedReclaimEnergy);
	package.add("GetImprovedTaunt", &Perl_StatBonuses_GetImprovedTaunt);
	package.add("GetIncreaseBlockChance", &Perl_StatBonuses_GetIncreaseBlockChance);
	package.add("GetIncreaseChanceMemoryWipe", &Perl_StatBonuses_GetIncreaseChanceMemoryWipe);
	package.add("GetIncreaseRunSpeedCap", &Perl_StatBonuses_GetIncreaseRunSpeedCap);
	package.add("GetInhibitMelee", &Perl_StatBonuses_GetInhibitMelee);
	package.add("GetINT", &Perl_StatBonuses_GetINT);
	package.add("GetINTCapModifier", &Perl_StatBonuses_GetINTCapModifier);
	package.add("GetIsBlind", &Perl_StatBonuses_GetIsBlind);
	package.add("GetIsFeared", &Perl_StatBonuses_GetIsFeared);
	package.add("GetItemATKCap", &Perl_StatBonuses_GetItemATKCap);
	package.add("GetItemHPRegenCap", &Perl_StatBonuses_GetItemHPRegenCap);
	package.add("GetItemManaRegenCap", &Perl_StatBonuses_GetItemManaRegenCap);
	package.add("GetLimitToSkill", &Perl_StatBonuses_GetLimitToSkill);
	package.add("GetMagicWeapon", &Perl_StatBonuses_GetMagicWeapon);
	package.add("GetMana", &Perl_StatBonuses_GetMana);
	package.add("GetManaAbsorbPercentDamage", &Perl_StatBonuses_GetManaAbsorbPercentDamage);
	package.add("GetManaPercentCap", &Perl_StatBonuses_GetManaPercentCap);
	package.add("GetManaRegen", &Perl_StatBonuses_GetManaRegen);
	package.add("GetMasteryOfPast", &Perl_StatBonuses_GetMasteryOfPast);
	package.add("GetMaxBindWound", &Perl_StatBonuses_GetMaxBindWound);
	package.add("GetMaxHP", &Perl_StatBonuses_GetMaxHP);
	package.add("GetMaxHPChange", &Perl_StatBonuses_GetMaxHPChange);
	package.add("GetMeleeLifetap", &Perl_StatBonuses_GetMeleeLifetap);
	package.add("GetMeleeMitigation", &Perl_StatBonuses_GetMeleeMitigation);
	package.add("GetMeleeMitigationEffect", &Perl_StatBonuses_GetMeleeMitigationEffect);
	package.add("GetMeleeRune", &Perl_StatBonuses_GetMeleeRune);
	package.add("GetMeleeSkillCheck", &Perl_StatBonuses_GetMeleeSkillCheck);
	package.add("GetMeleeSkillCheckSkill", &Perl_StatBonuses_GetMeleeSkillCheckSkill);
	package.add("GetMeleeThresholdGuard", &Perl_StatBonuses_GetMeleeThresholdGuard);
	package.add("GetMetabolism", &Perl_StatBonuses_GetMetabolism);
	package.add("GetMinimumDamageModifier", &Perl_StatBonuses_GetMinimumDamageModifier);
	package.add("GetMitigateDOTRune", &Perl_StatBonuses_GetMitigateDOTRune);
	package.add("GetMitigateMeleeRune", &Perl_StatBonuses_GetMitigateMeleeRune);
	package.add("GetMitigateSpellRune", &Perl_StatBonuses_GetMitigateSpellRune);
	package.add("GetMovementSpeed", &Perl_StatBonuses_GetMovementSpeed);
	package.add("GetMR", &Perl_StatBonuses_GetMR);
	package.add("GetMRCapModifier", &Perl_StatBonuses_GetMRCapModifier);
	package.add("GetNegateAttacks", &Perl_StatBonuses_GetNegateAttacks);
	package.add("GetNegateEffects", &Perl_StatBonuses_GetNegateEffects);
	package.add("GetNegateIfCombat", &Perl_StatBonuses_GetNegateIfCombat);
	package.add("GetNoBreakAESneak", &Perl_StatBonuses_GetNoBreakAESneak);
	package.add("GetOffhandRiposteFail", &Perl_StatBonuses_GetOffhandRiposteFail);
	package.add("GetPackrat", &Perl_StatBonuses_GetPackrat);
	package.add("GetParryChance", &Perl_StatBonuses_GetParryChance);
	package.add("GetPCPetFlurry", &Perl_StatBonuses_GetPCPetFlurry);
	package.add("GetPCPetRampage", &Perl_StatBonuses_GetPCPetRampage);
	package.add("GetPercussionModifier", &Perl_StatBonuses_GetPercussionModifier);
	package.add("GetPersistentCasting", &Perl_StatBonuses_GetPersistentCasting);
	package.add("GetPetAvoidance", &Perl_StatBonuses_GetPetAvoidance);
	package.add("GetPetCriticalHit", &Perl_StatBonuses_GetPetCriticalHit);
	package.add("GetPetFlurry", &Perl_StatBonuses_GetPetFlurry);
	package.add("GetPetMaxHP", &Perl_StatBonuses_GetPetMaxHP);
	package.add("GetPetMeleeMitigation", &Perl_StatBonuses_GetPetMeleeMitigation);
	package.add("GetPR", &Perl_StatBonuses_GetPR);
	package.add("GetPRCapModifier", &Perl_StatBonuses_GetPRCapModifier);
	package.add("GetProcChance", &Perl_StatBonuses_GetProcChance);
	package.add("GetProcChanceSPA", &Perl_StatBonuses_GetProcChanceSPA);
	package.add("GetRaiseSkillCap", &Perl_StatBonuses_GetRaiseSkillCap);
	package.add("GetReduceFallDamage", &Perl_StatBonuses_GetReduceFallDamage);
	package.add("GetReduceTradeskillFail", &Perl_StatBonuses_GetReduceTradeskillFail);
	package.add("GetReflectChance", &Perl_StatBonuses_GetReflectChance);
	package.add("GetResistFearChance", &Perl_StatBonuses_GetResistFearChance);
	package.add("GetResistSpellChance", &Perl_StatBonuses_GetResistSpellChance);
	package.add("GetReverseDamageShield", &Perl_StatBonuses_GetReverseDamageShield);
	package.add("GetReverseDamageShieldSpellID", &Perl_StatBonuses_GetReverseDamageShieldSpellID);
	package.add("GetReverseDamageShieldType", &Perl_StatBonuses_GetReverseDamageShieldType);
	package.add("GetRiposteChance", &Perl_StatBonuses_GetRiposteChance);
	package.add("GetRoot", &Perl_StatBonuses_GetRoot);
	package.add("GetRootBreakChance", &Perl_StatBonuses_GetRootBreakChance);
	package.add("GetSalvageChance", &Perl_StatBonuses_GetSalvageChance);
	package.add("GetSanctuary", &Perl_StatBonuses_GetSanctuary);
	package.add("GetScreech", &Perl_StatBonuses_GetScreech);
	package.add("GetSecondaryDamageIncrease", &Perl_StatBonuses_GetSecondaryDamageIncrease);
	package.add("GetSeeInvis", &Perl_StatBonuses_GetSeeInvis);
	package.add("GetSEResist", &Perl_StatBonuses_GetSEResist);
	package.add("GetShieldBlock", &Perl_StatBonuses_GetShieldBlock);
	package.add("GetShieldEquipDamageModifier", &Perl_StatBonuses_GetShieldEquipDamageModifier);
	package.add("GetShroudOfStealth", &Perl_StatBonuses_GetShroudOfStealth);
	package.add("GetSingingModifier", &Perl_StatBonuses_GetSingingModifier);
	package.add("GetSkillAttackProc", &Perl_StatBonuses_GetSkillAttackProc);
	package.add("GetSkillDamageAmount", &Perl_StatBonuses_GetSkillDamageAmount);
	package.add("GetSkillDamageAmount2", &Perl_StatBonuses_GetSkillDamageAmount2);
	package.add("GetSkillDamageTaken", &Perl_StatBonuses_GetSkillDamageTaken);
	package.add("GetSkillModifier", &Perl_StatBonuses_GetSkillModifier);
	package.add("GetSkillModifierMax", &Perl_StatBonuses_GetSkillModifierMax);
	package.add("GetSkillProc", &Perl_StatBonuses_GetSkillProc);
	package.add("GetSkillProcSuccess", &Perl_StatBonuses_GetSkillProcSuccess);
	package.add("GetSkillReuseTime", &Perl_StatBonuses_GetSkillReuseTime);
	package.add("GetSlayUndead", &Perl_StatBonuses_GetSlayUndead);
	package.add("GetSongModifierCap", &Perl_StatBonuses_GetSongModifierCap);
	package.add("GetSongRange", &Perl_StatBonuses_GetSongRange);
	package.add("GetSpellCriticalDamageIncreaseNoStack", &Perl_StatBonuses_GetSpellCriticalDamageIncreaseNoStack);
	package.add("GetSpellCriticalDamageIncrease", &Perl_StatBonuses_GetSpellCriticalDamageIncrease);
	package.add("GetSpellDamageShield", &Perl_StatBonuses_GetSpellDamageShield);
	package.add("GetSpellDamage", &Perl_StatBonuses_GetSpellDamage);
	package.add("GetSpellOnDeath", &Perl_StatBonuses_GetSpellOnDeath);
	package.add("GetSpellOnKill", &Perl_StatBonuses_GetSpellOnKill);
	package.add("GetSpellProcChance", &Perl_StatBonuses_GetSpellProcChance);
	package.add("GetSpellShield", &Perl_StatBonuses_GetSpellShield);
	package.add("GetSpellThresholdGuard", &Perl_StatBonuses_GetSpellThresholdGuard);
	package.add("GetSpellTriggers", &Perl_StatBonuses_GetSpellTriggers);
	package.add("GetSTA", &Perl_StatBonuses_GetSTA);
	package.add("GetSTACapModifier", &Perl_StatBonuses_GetSTACapModifier);
	package.add("GetSTR", &Perl_StatBonuses_GetSTR);
	package.add("GetSTRCapModifier", &Perl_StatBonuses_GetSTRCapModifier);
	package.add("GetStrikeThrough", &Perl_StatBonuses_GetStrikeThrough);
	package.add("GetStringedModifier", &Perl_StatBonuses_GetStringedModifier);
	package.add("GetStunBashChance", &Perl_StatBonuses_GetStunBashChance);
	package.add("GetStunResist", &Perl_StatBonuses_GetStunResist);
	package.add("GetFlatMaxHPChange", &Perl_StatBonuses_GetFlatMaxHPChange);
	package.add("GetTradeSkillMastery", &Perl_StatBonuses_GetTradeSkillMastery);
	package.add("GetTriggerMeleeThreshold", &Perl_StatBonuses_GetTriggerMeleeThreshold);
	package.add("GetTriggerOnValueAmount", &Perl_StatBonuses_GetTriggerOnValueAmount);
	package.add("GetTriggerSpellThreshold", &Perl_StatBonuses_GetTriggerSpellThreshold);
	package.add("GetTripleAttackChance", &Perl_StatBonuses_GetTripleAttackChance);
	package.add("GetTripleBackstab", &Perl_StatBonuses_GetTripleBackstab);
	package.add("GetTwoHandBluntBlock", &Perl_StatBonuses_GetTwoHandBluntBlock);
	package.add("GetUnfailingDivinity", &Perl_StatBonuses_GetUnfailingDivinity);
	package.add("GetVampirism", &Perl_StatBonuses_GetVampirism);
	package.add("GetVoiceGraft", &Perl_StatBonuses_GetVoiceGraft);
	package.add("GetWindModifier", &Perl_StatBonuses_GetWindModifier);
	package.add("GetWIS", &Perl_StatBonuses_GetWIS);
	package.add("GetWISCapModifier", &Perl_StatBonuses_GetWISCapModifier);
	package.add("GetXPRateModifier", &Perl_StatBonuses_GetXPRateModifier);
}

#endif //EMBPERL_XS_CLASSES
