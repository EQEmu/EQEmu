#include "common/features.h"
#include "client.h"

#ifdef EMBPERL_XS_CLASSES

#include "zone/embperl.h"

int Perl_QuestItemData_GetMinimumStatus(EQ::ItemData* self)
{
	return self->MinStatus;
}

int Perl_QuestItemData_GetItemClass(EQ::ItemData* self)
{
	return self->ItemClass;
}

const char* Perl_QuestItemData_GetName(EQ::ItemData* self)
{
	return self->Name;
}

const char* Perl_QuestItemData_GetLore(EQ::ItemData* self)
{
	return self->Lore;
}

const char* Perl_QuestItem_GetComment(EQ::ItemData* self)
{
	return self->Comment;
}

const char* Perl_QuestItemData_GetIDFile(EQ::ItemData* self)
{
	return self->IDFile;
}

uint32 Perl_QuestItemData_GetID(EQ::ItemData* self)
{
	return self->ID;
}

int Perl_QuestItemData_GetWeight(EQ::ItemData* self)
{
	return self->Weight;
}

int Perl_QuestItemData_GetNoRent(EQ::ItemData* self)
{
	return self->NoRent;
}

int Perl_QuestItemData_GetNoDrop(EQ::ItemData* self)
{
	return self->NoDrop;
}

int Perl_QuestItemData_GetSize(EQ::ItemData* self)
{
	return self->Size;
}

uint32 Perl_QuestItemData_GetSlots(EQ::ItemData* self)
{
	return self->Slots;
}

uint32 Perl_QuestItemData_GetPrice(EQ::ItemData* self)
{
	return self->Price;
}

uint32 Perl_QuestItemData_GetIcon(EQ::ItemData* self)
{
	return self->Icon;
}

int32 Perl_QuestItemData_GetLoreGroup(EQ::ItemData* self)
{
	return self->LoreGroup;
}

bool Perl_QuestItemData_GetLoreFlag(EQ::ItemData* self)
{
	return self->LoreFlag;
}

bool Perl_QuestItemData_GetPendingLoreFlag(EQ::ItemData* self)
{
	return self->PendingLoreFlag;
}

bool Perl_QuestItemData_GetArtifactFlag(EQ::ItemData* self)
{
	return self->ArtifactFlag;
}

bool Perl_QuestItemData_GetSummonedFlag(EQ::ItemData* self)
{
	return self->SummonedFlag;
}

int Perl_QuestItemData_GetFVNoDrop(EQ::ItemData* self)
{
	return self->FVNoDrop;
}

uint32 Perl_QuestItemData_GetFavor(EQ::ItemData* self)
{
	return self->Favor;
}

uint32 Perl_QuestItemData_GetGuildFavor(EQ::ItemData* self)
{
	return self->GuildFavor;
}

uint32 Perl_QuestItemData_GetPointType(EQ::ItemData* self)
{
	return self->PointType;
}

int Perl_QuestItemData_GetBagType(EQ::ItemData* self)
{
	return self->BagType;
}

int Perl_QuestItemData_GetBagSlots(EQ::ItemData* self)
{
	return self->BagSlots;
}

int Perl_QuestItemData_GetBagSize(EQ::ItemData* self)
{
	return self->BagSize;
}

int Perl_QuestItemData_GetBagWeightReduction(EQ::ItemData* self)
{
	return self->BagWR;
}

bool Perl_QuestItemData_GetBenefitFlag(EQ::ItemData* self)
{
	return self->BenefitFlag;
}

bool Perl_QuestItemData_GetTradeskills(EQ::ItemData* self)
{
	return self->Tradeskills;
}

int Perl_QuestItemData_GetCR(EQ::ItemData* self)
{
	return self->CR;
}

int Perl_QuestItemData_GetDR(EQ::ItemData* self)
{
	return self->DR;
}

int Perl_QuestItemData_GetPR(EQ::ItemData* self)
{
	return self->PR;
}

int Perl_QuestItemData_GetMR(EQ::ItemData* self)
{
	return self->MR;
}

int Perl_QuestItemData_GetFR(EQ::ItemData* self)
{
	return self->FR;
}

int Perl_QuestItemData_GetSTR(EQ::ItemData* self)
{
	return self->AStr;
}

int Perl_QuestItemData_GetSTA(EQ::ItemData* self)
{
	return self->ASta;
}

int Perl_QuestItemData_GetAGI(EQ::ItemData* self)
{
	return self->AAgi;
}

int Perl_QuestItemData_GetDEX(EQ::ItemData* self)
{
	return self->ADex;
}

int Perl_QuestItemData_GetCHA(EQ::ItemData* self)
{
	return self->ACha;
}

int Perl_QuestItemData_GetINT(EQ::ItemData* self)
{
	return self->AInt;
}

int Perl_QuestItemData_GetAWis(EQ::ItemData* self)
{
	return self->AWis;
}

int Perl_QuestItemData_GetHP(EQ::ItemData* self)
{
	return self->HP;
}

int Perl_QuestItemData_GetMana(EQ::ItemData* self)
{
	return self->Mana;
}

int Perl_QuestItemData_GetAC(EQ::ItemData* self)
{
	return self->AC;
}

uint32 Perl_QuestItemData_GetDeity(EQ::ItemData* self)
{
	return self->Deity;
}

int Perl_QuestItemData_GetSkillModifierValue(EQ::ItemData* self)
{
	return self->SkillModValue;
}

uint32 Perl_QuestItemData_GetSkillModifierType(EQ::ItemData* self)
{
	return self->SkillModType;
}

uint32 Perl_QuestItemData_GetBaneDamageRace(EQ::ItemData* self)
{
	return self->BaneDmgRace;
}

int Perl_QuestItemData_GetBaneDamageAmount(EQ::ItemData* self)
{
	return self->BaneDmgAmt;
}

uint32 Perl_QuestItemData_GetBaneDamageBody(EQ::ItemData* self)
{
	return self->BaneDmgBody;
}

bool Perl_QuestItemData_GetMagic(EQ::ItemData* self)
{
	return self->Magic;
}

int Perl_QuestItemData_GetCastTime_(EQ::ItemData* self)
{
	return self->CastTime_;
}

int Perl_QuestItemData_GetReqLevel(EQ::ItemData* self)
{
	return self->ReqLevel;
}

uint32 Perl_QuestItemData_GetBardSkillType(EQ::ItemData* self)
{
	return self->BardType;
}

int Perl_QuestItemData_GetBardSkillValue(EQ::ItemData* self)
{
	return self->BardValue;
}

int Perl_QuestItemData_GetLight(EQ::ItemData* self)
{
	return self->Light;
}

int Perl_QuestItemData_GetDelay(EQ::ItemData* self)
{
	return self->Delay;
}

int Perl_QuestItemData_GetRecLevel(EQ::ItemData* self)
{
	return self->RecLevel;
}

int Perl_QuestItemData_GetRecSkill(EQ::ItemData* self)
{
	return self->RecSkill;
}

int Perl_QuestItemData_GetElementalDamageType(EQ::ItemData* self)
{
	return self->ElemDmgType;
}

int Perl_QuestItemData_GetElementalDamageAmount(EQ::ItemData* self)
{
	return self->ElemDmgAmt;
}

int Perl_QuestItemData_GetRange(EQ::ItemData* self)
{
	return self->Range;
}

uint32 Perl_QuestItemData_GetDamage(EQ::ItemData* self)
{
	return self->Damage;
}

uint32 Perl_QuestItemData_GetColor(EQ::ItemData* self)
{
	return self->Color;
}

uint32 Perl_QuestItemData_GetClasses(EQ::ItemData* self)
{
	return self->Classes;
}

uint32 Perl_QuestItemData_GetRaces(EQ::ItemData* self)
{
	return self->Races;
}

int Perl_QuestItemData_GetMaximumCharges(EQ::ItemData* self)
{
	return self->MaxCharges;
}

int Perl_QuestItemData_GetItemType(EQ::ItemData* self)
{
	return self->ItemType;
}

int Perl_QuestItemData_GetMaterial(EQ::ItemData* self)
{
	return self->Material;
}

double Perl_QuestItemData_GetSellRate(EQ::ItemData* self)
{
	return self->SellRate;
}

uint32 Perl_QuestItemData_GetFulfilment(EQ::ItemData* self)
{
	return self->Fulfilment;
}

int Perl_QuestItemData_GetCastTime(EQ::ItemData* self)
{
	return self->CastTime;
}

uint32 Perl_QuestItemData_GetEliteMaterial(EQ::ItemData* self)
{
	return self->EliteMaterial;
}

int Perl_QuestItemData_GetProcRate(EQ::ItemData* self)
{
	return self->ProcRate;
}

int Perl_QuestItemData_GetCombatEffects(EQ::ItemData* self)
{
	return self->CombatEffects;
}

int Perl_QuestItemData_GetShielding(EQ::ItemData* self)
{
	return self->Shielding;
}

int Perl_QuestItemData_GetStunResist(EQ::ItemData* self)
{
	return self->StunResist;
}

int Perl_QuestItemData_GetStrikeThrough(EQ::ItemData* self)
{
	return self->StrikeThrough;
}

uint32 Perl_QuestItemData_GetExtraDamageSkill(EQ::ItemData* self)
{
	return self->ExtraDmgSkill;
}

uint32 Perl_QuestItemData_GetExtraDamageAmount(EQ::ItemData* self)
{
	return self->ExtraDmgAmt;
}

int Perl_QuestItemData_GetSpellShield(EQ::ItemData* self)
{
	return self->SpellShield;
}

int Perl_QuestItemData_GetAvoidance(EQ::ItemData* self)
{
	return self->Avoidance;
}

int Perl_QuestItemData_GetAccuracy(EQ::ItemData* self)
{
	return self->Accuracy;
}

uint32 Perl_QuestItemData_GetCharmFileID(EQ::ItemData* self)
{
	return self->CharmFileID;
}

int Perl_QuestItemData_GetFactionModifier1(EQ::ItemData* self)
{
	return self->FactionMod1;
}

int Perl_QuestItemData_GetFactionModifier2(EQ::ItemData* self)
{
	return self->FactionMod2;
}

int Perl_QuestItemData_GetFactionModifier3(EQ::ItemData* self)
{
	return self->FactionMod3;
}

int Perl_QuestItemData_GetFactionModifier4(EQ::ItemData* self)
{
	return self->FactionMod4;
}

int Perl_QuestItemData_GetFactionAmount1(EQ::ItemData* self)
{
	return self->FactionAmt1;
}

int Perl_QuestItemData_GetFactionAmount2(EQ::ItemData* self)
{
	return self->FactionAmt2;
}

int Perl_QuestItemData_GetFactionAmount3(EQ::ItemData* self)
{
	return self->FactionAmt3;
}

int Perl_QuestItemData_GetFactionAmount4(EQ::ItemData* self)
{
	return self->FactionAmt4;
}

const char* Perl_QuestItemData_GetCharmFile(EQ::ItemData* self)
{
	return self->CharmFile;
}

uint32 Perl_QuestItemData_GetAugmentType(EQ::ItemData* self)
{
	return self->AugType;
}

int Perl_QuestItemData_GetAugmentSlotType(EQ::ItemData* self, uint8 slot_id)
{
	if (!EQ::ValueWithin(slot_id, EQ::invaug::SOCKET_BEGIN, EQ::invaug::SOCKET_END)) {
		return 0;
	}

	return self->AugSlotType[slot_id];
}

int Perl_QuestItemData_GetAugmentSlotVisible(EQ::ItemData* self, uint8 slot_id)
{
	if (!EQ::ValueWithin(slot_id, EQ::invaug::SOCKET_BEGIN, EQ::invaug::SOCKET_END)) {
		return 0;
	}

	return self->AugSlotVisible[slot_id];
}

int Perl_QuestItemData_GetAugmentSlotUnk2(EQ::ItemData* self, uint8 slot_id)
{
	if (!EQ::ValueWithin(slot_id, EQ::invaug::SOCKET_BEGIN, EQ::invaug::SOCKET_END)) {
		return 0;
	}

	return self->AugSlotUnk2[slot_id];
}

uint32 Perl_QuestItemData_GetLDoNTheme(EQ::ItemData* self)
{
	return self->LDoNTheme;
}

uint32 Perl_QuestItemData_GetLDoNPrice(EQ::ItemData* self)
{
	return self->LDoNPrice;
}

uint32 Perl_QuestItemData_GetLDoNSold(EQ::ItemData* self)
{
	return self->LDoNSold;
}

uint32 Perl_QuestItemData_GetBaneDamageRaceAmount(EQ::ItemData* self)
{
	return self->BaneDmgRaceAmt;
}

uint32 Perl_QuestItemData_GetAugmentRestrict(EQ::ItemData* self)
{
	return self->AugRestrict;
}

int32 Perl_QuestItemData_GetEndurance(EQ::ItemData* self)
{
	return self->Endur;
}

int32 Perl_QuestItemData_GetDOTShielding(EQ::ItemData* self)
{
	return self->DotShielding;
}

int32 Perl_QuestItemData_GetAttack(EQ::ItemData* self)
{
	return self->Attack;
}

int32 Perl_QuestItemData_GetRegen(EQ::ItemData* self)
{
	return self->Regen;
}

int32 Perl_QuestItemData_GetManaRegen(EQ::ItemData* self)
{
	return self->ManaRegen;
}

int32 Perl_QuestItemData_GetEnduranceRegen(EQ::ItemData* self)
{
	return self->EnduranceRegen;
}

int32 Perl_QuestItemData_GetHaste(EQ::ItemData* self)
{
	return self->Haste;
}

int32 Perl_QuestItemData_GetDamageShield(EQ::ItemData* self)
{
	return self->DamageShield;
}

uint32 Perl_QuestItemData_GetRecastDelay(EQ::ItemData* self)
{
	return self->RecastDelay;
}

uint32 Perl_QuestItemData_GetRecastType(EQ::ItemData* self)
{
	return self->RecastType;
}

uint32 Perl_QuestItemData_GetAugmentDistiller(EQ::ItemData* self)
{
	return self->AugDistiller;
}

bool Perl_QuestItemData_GetAttuneable(EQ::ItemData* self)
{
	return self->Attuneable;
}

bool Perl_QuestItemData_GetNoPet(EQ::ItemData* self)
{
	return self->NoPet;
}

bool Perl_QuestItemData_GetPotionBelt(EQ::ItemData* self)
{
	return self->PotionBelt;
}

bool Perl_QuestItemData_GetStackable(EQ::ItemData* self)
{
	return self->Stackable;
}

bool Perl_QuestItemData_GetNoTransfer(EQ::ItemData* self)
{
	return self->NoTransfer;
}

bool Perl_QuestItemData_GetQuestItemFlag(EQ::ItemData* self)
{
	return self->QuestItemFlag;
}

int Perl_QuestItemData_GetStackSize(EQ::ItemData* self)
{
	return self->StackSize;
}

int Perl_QuestItemData_GetPotionBeltSlots(EQ::ItemData* self)
{
	return self->PotionBeltSlots;
}

int Perl_QuestItemData_GetClickEffect(EQ::ItemData* self)
{
	return self->Click.Effect;
}

int Perl_QuestItemData_GetClickType(EQ::ItemData* self)
{
	return self->Click.Type;
}

int Perl_QuestItemData_GetClickLevel(EQ::ItemData* self)
{
	return self->Click.Level;
}

int Perl_QuestItemData_GetClickLevel2(EQ::ItemData* self)
{
	return self->Click.Level2;
}

int Perl_QuestItemData_GetProcEffect(EQ::ItemData* self)
{
	return self->Proc.Effect;
}

int Perl_QuestItemData_GetProcType(EQ::ItemData* self)
{
	return self->Proc.Type;
}

int Perl_QuestItemData_GetProcLevel(EQ::ItemData* self)
{
	return self->Proc.Level;
}

int Perl_QuestItemData_GetProcLevel2(EQ::ItemData* self)
{
	return self->Proc.Level2;
}

int Perl_QuestItemData_GetWornEffect(EQ::ItemData* self)
{
	return self->Worn.Effect;
}

int Perl_QuestItemData_GetWornType(EQ::ItemData* self)
{
	return self->Worn.Type;
}

int Perl_QuestItemData_GetWornLevel(EQ::ItemData* self)
{
	return self->Worn.Level;
}

int Perl_QuestItemData_GetWornLevel2(EQ::ItemData* self)
{
	return self->Worn.Level2;
}

int Perl_QuestItemData_GetFocusEffect(EQ::ItemData* self)
{
	return self->Focus.Effect;
}

int Perl_QuestItemData_GetFocusType(EQ::ItemData* self)
{
	return self->Focus.Type;
}

int Perl_QuestItemData_GetFocusLevel(EQ::ItemData* self)
{
	return self->Focus.Level;
}

int Perl_QuestItemData_GetFocusLevel2(EQ::ItemData* self)
{
	return self->Focus.Level2;
}

int Perl_QuestItemData_GetScrollEffect(EQ::ItemData* self)
{
	return self->Scroll.Effect;
}

int Perl_QuestItemData_GetScrollType(EQ::ItemData* self)
{
	return self->Scroll.Type;
}

int Perl_QuestItemData_GetScrollLevel(EQ::ItemData* self)
{
	return self->Scroll.Level;
}

int Perl_QuestItemData_GetScrollLevel2(EQ::ItemData* self)
{
	return self->Scroll.Level2;
}

int Perl_QuestItemData_GetBardEffect(EQ::ItemData* self)
{
	return self->Bard.Effect;
}

int Perl_QuestItemData_GetBardType(EQ::ItemData* self)
{
	return self->Bard.Type;
}

int Perl_QuestItemData_GetBardLevel(EQ::ItemData* self)
{
	return self->Bard.Level;
}

int Perl_QuestItemData_GetBardLevel2(EQ::ItemData* self)
{
	return self->Bard.Level2;
}

int Perl_QuestItemData_GetBook(EQ::ItemData* self)
{
	return self->Book;
}

uint32 Perl_QuestItemData_GetBookType(EQ::ItemData* self)
{
	return self->BookType;
}

const char* Perl_QuestItemData_GetFilename(EQ::ItemData* self)
{
	return self->Filename;
}

int Perl_QuestItemData_GetCorruption(EQ::ItemData* self)
{
	return self->SVCorruption;
}

uint32 Perl_QuestItemData_GetPurity(EQ::ItemData* self)
{
	return self->Purity;
}

uint32 Perl_QuestItemData_GetBackstabDamage(EQ::ItemData* self)
{
	return self->BackstabDmg;
}

uint32 Perl_QuestItemData_GetDSMitigation(EQ::ItemData* self)
{
	return self->DSMitigation;
}

int Perl_QuestItemData_GetHeroicSTR(EQ::ItemData* self)
{
	return self->HeroicStr;
}

int Perl_QuestItemData_GetHeroicINT(EQ::ItemData* self)
{
	return self->HeroicInt;
}

int Perl_QuestItemData_GetHeroicWIS(EQ::ItemData* self)
{
	return self->HeroicWis;
}

int Perl_QuestItemData_GetHeroicAGI(EQ::ItemData* self)
{
	return self->HeroicAgi;
}

int Perl_QuestItemData_GetHeroicDEX(EQ::ItemData* self)
{
	return self->HeroicDex;
}

int Perl_QuestItemData_GetHeroicSTA(EQ::ItemData* self)
{
	return self->HeroicSta;
}

int Perl_QuestItemData_GetHeroicCHA(EQ::ItemData* self)
{
	return self->HeroicCha;
}

int Perl_QuestItemData_GetHeroicMR(EQ::ItemData* self)
{
	return self->HeroicMR;
}

int Perl_QuestItemData_GetHeroicFR(EQ::ItemData* self)
{
	return self->HeroicFR;
}

int Perl_QuestItemData_GetHeroicCR(EQ::ItemData* self)
{
	return self->HeroicCR;
}

int Perl_QuestItemData_GetHeroicDR(EQ::ItemData* self)
{
	return self->HeroicDR;
}

int Perl_QuestItemData_GetHeroicPR(EQ::ItemData* self)
{
	return self->HeroicPR;
}

int Perl_QuestItemData_GetHeroicCorruption(EQ::ItemData* self)
{
	return self->HeroicSVCorrup;
}

int Perl_QuestItemData_GetHealAmount(EQ::ItemData* self)
{
	return self->HealAmt;
}

int Perl_QuestItemData_GetSpellDamage(EQ::ItemData* self)
{
	return self->SpellDmg;
}

uint32 Perl_QuestItemData_GetLDoNSellBackRate(EQ::ItemData* self)
{
	return self->LDoNSellBackRate;
}

uint32 Perl_QuestItemData_GetScriptFileID(EQ::ItemData* self)
{
	return self->ScriptFileID;
}

int Perl_QuestItemData_GetExpendableArrow(EQ::ItemData* self)
{
	return self->ExpendableArrow;
}

uint32 Perl_QuestItemData_GetClairvoyance(EQ::ItemData* self)
{
	return self->Clairvoyance;
}

const char* Perl_QuestItemData_GetClickName(EQ::ItemData* self)
{
	return self->ClickName;
}

const char* Perl_QuestItemData_GetProcName(EQ::ItemData* self)
{
	return self->ProcName;
}

const char* Perl_QuestItemData_GetWornName(EQ::ItemData* self)
{
	return self->WornName;
}

const char* Perl_QuestItemData_GetFocusName(EQ::ItemData* self)
{
	return self->FocusName;
}

const char* Perl_QuestItemData_GetScrollName(EQ::ItemData* self)
{
	return self->ScrollName;
}

void perl_register_questitem_data()
{
	perl::interpreter perl(PERL_GET_THX);

	auto package = perl.new_class<EQ::ItemData>("QuestItemData");
	package.add("GetAGI", &Perl_QuestItemData_GetAGI);
	package.add("GetAC", &Perl_QuestItemData_GetAC);
	package.add("GetCHA", &Perl_QuestItemData_GetCHA);
	package.add("GetDEX", &Perl_QuestItemData_GetDEX);
	package.add("GetINT", &Perl_QuestItemData_GetINT);
	package.add("GetSTA", &Perl_QuestItemData_GetSTA);
	package.add("GetSTR", &Perl_QuestItemData_GetSTR);
	package.add("GetAWis", &Perl_QuestItemData_GetAWis);
	package.add("GetAccuracy", &Perl_QuestItemData_GetAccuracy);
	package.add("GetArtifactFlag", &Perl_QuestItemData_GetArtifactFlag);
	package.add("GetAttack", &Perl_QuestItemData_GetAttack);
	package.add("GetAttuneable", &Perl_QuestItemData_GetAttuneable);
	package.add("GetAugmentDistiller", &Perl_QuestItemData_GetAugmentDistiller);
	package.add("GetAugmentRestrict", &Perl_QuestItemData_GetAugmentRestrict);
	package.add("GetAugmentSlotType", &Perl_QuestItemData_GetAugmentSlotType);
	package.add("GetAugmentSlotUnk2", &Perl_QuestItemData_GetAugmentSlotUnk2);
	package.add("GetAugmentSlotVisible", &Perl_QuestItemData_GetAugmentSlotVisible);
	package.add("GetAugmentType", &Perl_QuestItemData_GetAugmentType);
	package.add("GetAvoidance", &Perl_QuestItemData_GetAvoidance);
	package.add("GetBackstabDamage", &Perl_QuestItemData_GetBackstabDamage);
	package.add("GetBagSize", &Perl_QuestItemData_GetBagSize);
	package.add("GetBagSlots", &Perl_QuestItemData_GetBagSlots);
	package.add("GetBagType", &Perl_QuestItemData_GetBagType);
	package.add("GetBagWeightReduction", &Perl_QuestItemData_GetBagWeightReduction);
	package.add("GetBaneDamageAmount", &Perl_QuestItemData_GetBaneDamageAmount);
	package.add("GetBaneDamageBody", &Perl_QuestItemData_GetBaneDamageBody);
	package.add("GetBaneDamageRace", &Perl_QuestItemData_GetBaneDamageRace);
	package.add("GetBaneDamageRaceAmount", &Perl_QuestItemData_GetBaneDamageRaceAmount);
	package.add("GetBardEffect", &Perl_QuestItemData_GetBardEffect);
	package.add("GetBardLevel", &Perl_QuestItemData_GetBardLevel);
	package.add("GetBardLevel2", &Perl_QuestItemData_GetBardLevel2);
	package.add("GetBardType", &Perl_QuestItemData_GetBardType);
	package.add("GetBardSkillType", &Perl_QuestItemData_GetBardSkillType);
	package.add("GetBardSkillValue", &Perl_QuestItemData_GetBardSkillValue);
	package.add("GetBenefitFlag", &Perl_QuestItemData_GetBenefitFlag);
	package.add("GetBook", &Perl_QuestItemData_GetBook);
	package.add("GetBookType", &Perl_QuestItemData_GetBookType);
	package.add("GetCR", &Perl_QuestItemData_GetCR);
	package.add("GetCastTime", &Perl_QuestItemData_GetCastTime);
	package.add("GetCastTime_", &Perl_QuestItemData_GetCastTime_);
	package.add("GetCharmFile", &Perl_QuestItemData_GetCharmFile);
	package.add("GetCharmFileID", &Perl_QuestItemData_GetCharmFileID);
	package.add("GetClairvoyance", &Perl_QuestItemData_GetClairvoyance);
	package.add("GetClasses", &Perl_QuestItemData_GetClasses);
	package.add("GetClickName", &Perl_QuestItemData_GetClickName);
	package.add("GetClickEffect", &Perl_QuestItemData_GetClickEffect);
	package.add("GetClickLevel", &Perl_QuestItemData_GetClickLevel);
	package.add("GetClickLevel2", &Perl_QuestItemData_GetClickLevel2);
	package.add("GetClickType", &Perl_QuestItemData_GetClickType);
	package.add("GetColor", &Perl_QuestItemData_GetColor);
	package.add("GetCombatEffects", &Perl_QuestItemData_GetCombatEffects);
	package.add("GetComment", &Perl_QuestItem_GetComment);
	package.add("GetCorruption", &Perl_QuestItemData_GetCorruption);
	package.add("GetDR", &Perl_QuestItemData_GetDR);
	package.add("GetDSMitigation", &Perl_QuestItemData_GetDSMitigation);
	package.add("GetDamage", &Perl_QuestItemData_GetDamage);
	package.add("GetDamageShield", &Perl_QuestItemData_GetDamageShield);
	package.add("GetDeity", &Perl_QuestItemData_GetDeity);
	package.add("GetDelay", &Perl_QuestItemData_GetDelay);
	package.add("GetDOTShielding", &Perl_QuestItemData_GetDOTShielding);
	package.add("GetElementalDamageAmount", &Perl_QuestItemData_GetElementalDamageAmount);
	package.add("GetElementalDamageType", &Perl_QuestItemData_GetElementalDamageType);
	package.add("GetEliteMaterial", &Perl_QuestItemData_GetEliteMaterial);
	package.add("GetEndurance", &Perl_QuestItemData_GetEndurance);
	package.add("GetEnduranceRegen", &Perl_QuestItemData_GetEnduranceRegen);
	package.add("GetExpendableArrow", &Perl_QuestItemData_GetExpendableArrow);
	package.add("GetExtraDamageAmount", &Perl_QuestItemData_GetExtraDamageAmount);
	package.add("GetExtraDamageSkill", &Perl_QuestItemData_GetExtraDamageSkill);
	package.add("GetFR", &Perl_QuestItemData_GetFR);
	package.add("GetFVNoDrop", &Perl_QuestItemData_GetFVNoDrop);
	package.add("GetFactionAmount1", &Perl_QuestItemData_GetFactionAmount1);
	package.add("GetFactionAmount2", &Perl_QuestItemData_GetFactionAmount2);
	package.add("GetFactionAmount3", &Perl_QuestItemData_GetFactionAmount3);
	package.add("GetFactionAmount4", &Perl_QuestItemData_GetFactionAmount4);
	package.add("GetFactionModifier1", &Perl_QuestItemData_GetFactionModifier1);
	package.add("GetFactionModifier2", &Perl_QuestItemData_GetFactionModifier2);
	package.add("GetFactionModifier3", &Perl_QuestItemData_GetFactionModifier3);
	package.add("GetFactionModifier4", &Perl_QuestItemData_GetFactionModifier4);
	package.add("GetFavor", &Perl_QuestItemData_GetFavor);
	package.add("GetFilename", &Perl_QuestItemData_GetFilename);
	package.add("GetFocusEffect", &Perl_QuestItemData_GetFocusEffect);
	package.add("GetFocusName", &Perl_QuestItemData_GetFocusName);
	package.add("GetFocusLevel", &Perl_QuestItemData_GetFocusLevel);
	package.add("GetFocusLevel2", &Perl_QuestItemData_GetFocusLevel2);
	package.add("GetFocusType", &Perl_QuestItemData_GetFocusType);
	package.add("GetFulfilment", &Perl_QuestItemData_GetFulfilment);
	package.add("GetGuildFavor", &Perl_QuestItemData_GetGuildFavor);
	package.add("GetHP", &Perl_QuestItemData_GetHP);
	package.add("GetHaste", &Perl_QuestItemData_GetHaste);
	package.add("GetHealAmount", &Perl_QuestItemData_GetHealAmount);
	package.add("GetHeroicAGI", &Perl_QuestItemData_GetHeroicAGI);
	package.add("GetHeroicCR", &Perl_QuestItemData_GetHeroicCR);
	package.add("GetHeroicCHA", &Perl_QuestItemData_GetHeroicCHA);
	package.add("GetHeroicCorruption", &Perl_QuestItemData_GetHeroicCorruption);
	package.add("GetHeroicDR", &Perl_QuestItemData_GetHeroicDR);
	package.add("GetHeroicDEX", &Perl_QuestItemData_GetHeroicDEX);
	package.add("GetHeroicFR", &Perl_QuestItemData_GetHeroicFR);
	package.add("GetHeroicINT", &Perl_QuestItemData_GetHeroicINT);
	package.add("GetHeroicMR", &Perl_QuestItemData_GetHeroicMR);
	package.add("GetHeroicPR", &Perl_QuestItemData_GetHeroicPR);
	package.add("GetHeroicSTA", &Perl_QuestItemData_GetHeroicSTA);
	package.add("GetHeroicSTR", &Perl_QuestItemData_GetHeroicSTR);
	package.add("GetHeroicWIS", &Perl_QuestItemData_GetHeroicWIS);
	package.add("GetID", &Perl_QuestItemData_GetID);
	package.add("GetIDFile", &Perl_QuestItemData_GetIDFile);
	package.add("GetIcon", &Perl_QuestItemData_GetIcon);
	package.add("GetItemClass", &Perl_QuestItemData_GetItemClass);
	package.add("GetItemType", &Perl_QuestItemData_GetItemType);
	package.add("GetLDoNPrice", &Perl_QuestItemData_GetLDoNPrice);
	package.add("GetLDoNSellBackRate", &Perl_QuestItemData_GetLDoNSellBackRate);
	package.add("GetLDoNSold", &Perl_QuestItemData_GetLDoNSold);
	package.add("GetLDoNTheme", &Perl_QuestItemData_GetLDoNTheme);
	package.add("GetLight", &Perl_QuestItemData_GetLight);
	package.add("GetLore", &Perl_QuestItemData_GetLore);
	package.add("GetLoreFlag", &Perl_QuestItemData_GetLoreFlag);
	package.add("GetLoreGroup", &Perl_QuestItemData_GetLoreGroup);
	package.add("GetMR", &Perl_QuestItemData_GetMR);
	package.add("GetMagic", &Perl_QuestItemData_GetMagic);
	package.add("GetMana", &Perl_QuestItemData_GetMana);
	package.add("GetManaRegen", &Perl_QuestItemData_GetManaRegen);
	package.add("GetMaterial", &Perl_QuestItemData_GetMaterial);
	package.add("GetMaximumCharges", &Perl_QuestItemData_GetMaximumCharges);
	package.add("GetMinimumStatus", &Perl_QuestItemData_GetMinimumStatus);
	package.add("GetName", &Perl_QuestItemData_GetName);
	package.add("GetNoDrop", &Perl_QuestItemData_GetNoDrop);
	package.add("GetNoPet", &Perl_QuestItemData_GetNoPet);
	package.add("GetNoRent", &Perl_QuestItemData_GetNoRent);
	package.add("GetNoTransfer", &Perl_QuestItemData_GetNoTransfer);
	package.add("GetPR", &Perl_QuestItemData_GetPR);
	package.add("GetPendingLoreFlag", &Perl_QuestItemData_GetPendingLoreFlag);
	package.add("GetPointType", &Perl_QuestItemData_GetPointType);
	package.add("GetPotionBelt", &Perl_QuestItemData_GetPotionBelt);
	package.add("GetPotionBeltSlots", &Perl_QuestItemData_GetPotionBeltSlots);
	package.add("GetPrice", &Perl_QuestItemData_GetPrice);
	package.add("GetProcEffect", &Perl_QuestItemData_GetProcEffect);
	package.add("GetProcName", &Perl_QuestItemData_GetProcName);
	package.add("GetProcRate", &Perl_QuestItemData_GetProcRate);
	package.add("GetProcLevel", &Perl_QuestItemData_GetProcLevel);
	package.add("GetProcLevel2", &Perl_QuestItemData_GetProcLevel2);
	package.add("GetProcType", &Perl_QuestItemData_GetProcType);
	package.add("GetPurity", &Perl_QuestItemData_GetPurity);
	package.add("GetQuestItemFlag", &Perl_QuestItemData_GetQuestItemFlag);
	package.add("GetRaces", &Perl_QuestItemData_GetRaces);
	package.add("GetRange", &Perl_QuestItemData_GetRange);
	package.add("GetRecLevel", &Perl_QuestItemData_GetRecLevel);
	package.add("GetRecSkill", &Perl_QuestItemData_GetRecSkill);
	package.add("GetRecastDelay", &Perl_QuestItemData_GetRecastDelay);
	package.add("GetRecastType", &Perl_QuestItemData_GetRecastType);
	package.add("GetRegen", &Perl_QuestItemData_GetRegen);
	package.add("GetReqLevel", &Perl_QuestItemData_GetReqLevel);
	package.add("GetScriptFileID", &Perl_QuestItemData_GetScriptFileID);
	package.add("GetScrollEffect", &Perl_QuestItemData_GetScrollEffect);
	package.add("GetScrollName", &Perl_QuestItemData_GetScrollName);
	package.add("GetScrollLevel", &Perl_QuestItemData_GetScrollLevel);
	package.add("GetScrollLevel2", &Perl_QuestItemData_GetScrollLevel2);
	package.add("GetScrollType", &Perl_QuestItemData_GetScrollType);
	package.add("GetSellRate", &Perl_QuestItemData_GetSellRate);
	package.add("GetShielding", &Perl_QuestItemData_GetShielding);
	package.add("GetSize", &Perl_QuestItemData_GetSize);
	package.add("GetSkillModifierType", &Perl_QuestItemData_GetSkillModifierType);
	package.add("GetSkillModifierValue", &Perl_QuestItemData_GetSkillModifierValue);
	package.add("GetSlots", &Perl_QuestItemData_GetSlots);
	package.add("GetSpellDamage", &Perl_QuestItemData_GetSpellDamage);
	package.add("GetSpellShield", &Perl_QuestItemData_GetSpellShield);
	package.add("GetStackSize", &Perl_QuestItemData_GetStackSize);
	package.add("GetStackable", &Perl_QuestItemData_GetStackable);
	package.add("GetStrikeThrough", &Perl_QuestItemData_GetStrikeThrough);
	package.add("GetStunResist", &Perl_QuestItemData_GetStunResist);
	package.add("GetSummonedFlag", &Perl_QuestItemData_GetSummonedFlag);
	package.add("GetTradeskills", &Perl_QuestItemData_GetTradeskills);
	package.add("GetWeight", &Perl_QuestItemData_GetWeight);
	package.add("GetWornName", &Perl_QuestItemData_GetWornName);
	package.add("GetWornEffect", &Perl_QuestItemData_GetWornEffect);
	package.add("GetWornLevel", &Perl_QuestItemData_GetWornLevel);
	package.add("GetWornLevel2", &Perl_QuestItemData_GetWornLevel2);
	package.add("GetWornType", &Perl_QuestItemData_GetWornType);
}

#endif //EMBPERL_XS_CLASSES
