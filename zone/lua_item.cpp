#ifdef LUA_EQEMU

#include "lua_item.h"

#include "zone/masterentity.h"

#include "luabind/luabind.hpp"

Lua_Item::Lua_Item(uint32 item_id) {
	const EQ::ItemData *t = database.GetItem(item_id);
	SetLuaPtrData(t);
}

int Lua_Item::GetMinStatus() {
	Lua_Safe_Call_Int();
	return self->MinStatus;
}

int Lua_Item::GetItemClass() {
	Lua_Safe_Call_Int();
	return self->ItemClass;
}

const char *Lua_Item::GetName() {
	Lua_Safe_Call_String();
	return self->Name;
}

const char *Lua_Item::GetLore() {
	Lua_Safe_Call_String();
	return self->Lore;
}

const char *Lua_Item::GetComment() {
	Lua_Safe_Call_String();
	return self->Comment;
}

const char *Lua_Item::GetIDFile() {
	Lua_Safe_Call_String();
	return self->IDFile;
}

uint32 Lua_Item::GetID() {
	Lua_Safe_Call_Int();
	return self->ID;
}

int Lua_Item::GetWeight() {
	Lua_Safe_Call_Int();
	return self->Weight;
}

int Lua_Item::GetNoRent() {
	Lua_Safe_Call_Int();
	return self->NoRent;
}

int Lua_Item::GetNoDrop() {
	Lua_Safe_Call_Int();
	return self->NoDrop;
}

int Lua_Item::GetSize() {
	Lua_Safe_Call_Int();
	return self->Size;
}

uint32 Lua_Item::GetSlots() {
	Lua_Safe_Call_Int();
	return self->Slots;
}

uint32 Lua_Item::GetPrice() {
	Lua_Safe_Call_Int();
	return self->Price;
}

uint32 Lua_Item::GetIcon() {
	Lua_Safe_Call_Int();
	return self->Icon;
}

int32 Lua_Item::GetLoreGroup() {
	Lua_Safe_Call_Int();
	return self->LoreGroup;
}

bool Lua_Item::GetLoreFlag() {
	Lua_Safe_Call_Bool();
	return self->LoreFlag;
}

bool Lua_Item::GetPendingLoreFlag() {
	Lua_Safe_Call_Bool();
	return self->PendingLoreFlag;
}

bool Lua_Item::GetArtifactFlag() {
	Lua_Safe_Call_Bool();
	return self->ArtifactFlag;
}

bool Lua_Item::GetSummonedFlag() {
	Lua_Safe_Call_Bool();
	return self->SummonedFlag;
}

int Lua_Item::GetFVNoDrop() {
	Lua_Safe_Call_Int();
	return self->FVNoDrop;
}

uint32 Lua_Item::GetFavor() {
	Lua_Safe_Call_Int();
	return self->Favor;
}

uint32 Lua_Item::GetGuildFavor() {
	Lua_Safe_Call_Int();
	return self->GuildFavor;
}

uint32 Lua_Item::GetPointType() {
	Lua_Safe_Call_Int();
	return self->PointType;
}

int Lua_Item::GetBagType() {
	Lua_Safe_Call_Int();
	return self->BagType;
}

int Lua_Item::GetBagSlots() {
	Lua_Safe_Call_Int();
	return self->BagSlots;
}

int Lua_Item::GetBagSize() {
	Lua_Safe_Call_Int();
	return self->BagSize;
}

int Lua_Item::GetBagWR() {
	Lua_Safe_Call_Int();
	return self->BagWR;
}

bool Lua_Item::GetBenefitFlag() {
	Lua_Safe_Call_Bool();
	return self->BenefitFlag;
}

bool Lua_Item::GetTradeskills() {
	Lua_Safe_Call_Bool();
	return self->Tradeskills;
}

int Lua_Item::GetCR() {
	Lua_Safe_Call_Int();
	return self->CR;
}

int Lua_Item::GetDR() {
	Lua_Safe_Call_Int();
	return self->DR;
}

int Lua_Item::GetPR() {
	Lua_Safe_Call_Int();
	return self->PR;
}

int Lua_Item::GetMR() {
	Lua_Safe_Call_Int();
	return self->MR;
}

int Lua_Item::GetFR() {
	Lua_Safe_Call_Int();
	return self->FR;
}

int Lua_Item::GetAStr() {
	Lua_Safe_Call_Int();
	return self->AStr;
}

int Lua_Item::GetASta() {
	Lua_Safe_Call_Int();
	return self->ASta;
}

int Lua_Item::GetAAgi() {
	Lua_Safe_Call_Int();
	return self->AAgi;
}

int Lua_Item::GetADex() {
	Lua_Safe_Call_Int();
	return self->ADex;
}

int Lua_Item::GetACha() {
	Lua_Safe_Call_Int();
	return self->ACha;
}

int Lua_Item::GetAInt() {
	Lua_Safe_Call_Int();
	return self->AInt;
}

int Lua_Item::GetAWis() {
	Lua_Safe_Call_Int();
	return self->AWis;
}

int Lua_Item::GetHP() {
	Lua_Safe_Call_Int();
	return self->HP;
}

int Lua_Item::GetMana() {
	Lua_Safe_Call_Int();
	return self->Mana;
}

int Lua_Item::GetAC() {
	Lua_Safe_Call_Int();
	return self->AC;
}

uint32 Lua_Item::GetDeity() {
	Lua_Safe_Call_Int();
	return self->Deity;
}

int Lua_Item::GetSkillModValue() {
	Lua_Safe_Call_Int();
	return self->SkillModValue;
}

uint32 Lua_Item::GetSkillModType() {
	Lua_Safe_Call_Int();
	return self->SkillModType;
}

uint32 Lua_Item::GetBaneDmgRace() {
	Lua_Safe_Call_Int();
	return self->BaneDmgRace;
}

int Lua_Item::GetBaneDmgAmt() {
	Lua_Safe_Call_Int();
	return self->BaneDmgAmt;
}

uint32 Lua_Item::GetBaneDmgBody() {
	Lua_Safe_Call_Int();
	return self->BaneDmgBody;
}

bool Lua_Item::GetMagic() {
	Lua_Safe_Call_Bool();
	return self->Magic;
}

int Lua_Item::GetCastTime_() {
	Lua_Safe_Call_Int();
	return self->CastTime_;
}

int Lua_Item::GetReqLevel() {
	Lua_Safe_Call_Int();
	return self->ReqLevel;
}

uint32 Lua_Item::GetBardType() {
	Lua_Safe_Call_Int();
	return self->BardType;
}

int Lua_Item::GetBardValue() {
	Lua_Safe_Call_Int();
	return self->BardValue;
}

int Lua_Item::GetLight() {
	Lua_Safe_Call_Int();
	return self->Light;
}

int Lua_Item::GetDelay() {
	Lua_Safe_Call_Int();
	return self->Delay;
}

int Lua_Item::GetRecLevel() {
	Lua_Safe_Call_Int();
	return self->RecLevel;
}

int Lua_Item::GetRecSkill() {
	Lua_Safe_Call_Int();
	return self->RecSkill;
}

int Lua_Item::GetElemDmgType() {
	Lua_Safe_Call_Int();
	return self->ElemDmgType;
}

int Lua_Item::GetElemDmgAmt() {
	Lua_Safe_Call_Int();
	return self->ElemDmgAmt;
}

int Lua_Item::GetRange() {
	Lua_Safe_Call_Int();
	return self->Range;
}

uint32 Lua_Item::GetDamage() {
	Lua_Safe_Call_Int();
	return self->Damage;
}

uint32 Lua_Item::GetColor() {
	Lua_Safe_Call_Int();
	return self->Color;
}

uint32 Lua_Item::GetClasses() {
	Lua_Safe_Call_Int();
	return self->Classes;
}

uint32 Lua_Item::GetRaces() {
	Lua_Safe_Call_Int();
	return self->Races;
}

int Lua_Item::GetMaxCharges() {
	Lua_Safe_Call_Int();
	return self->MaxCharges;
}

int Lua_Item::GetItemType() {
	Lua_Safe_Call_Int();
	return self->ItemType;
}

int Lua_Item::GetMaterial() {
	Lua_Safe_Call_Int();
	return self->Material;
}

double Lua_Item::GetSellRate() {
	Lua_Safe_Call_Real();
	return self->SellRate;
}

uint32 Lua_Item::GetFulfilment() {
	Lua_Safe_Call_Int();
	return self->Fulfilment;
}

int Lua_Item::GetCastTime() {
	Lua_Safe_Call_Int();
	return self->CastTime;
}

uint32 Lua_Item::GetEliteMaterial() {
	Lua_Safe_Call_Int();
	return self->EliteMaterial;
}

int Lua_Item::GetProcRate() {
	Lua_Safe_Call_Int();
	return self->ProcRate;
}

int Lua_Item::GetCombatEffects() {
	Lua_Safe_Call_Int();
	return self->CombatEffects;
}

int Lua_Item::GetShielding() {
	Lua_Safe_Call_Int();
	return self->Shielding;
}

int Lua_Item::GetStunResist() {
	Lua_Safe_Call_Int();
	return self->StunResist;
}

int Lua_Item::GetStrikeThrough() {
	Lua_Safe_Call_Int();
	return self->StrikeThrough;
}

uint32 Lua_Item::GetExtraDmgSkill() {
	Lua_Safe_Call_Int();
	return self->ExtraDmgSkill;
}

uint32 Lua_Item::GetExtraDmgAmt() {
	Lua_Safe_Call_Int();
	return self->ExtraDmgAmt;
}

int Lua_Item::GetSpellShield() {
	Lua_Safe_Call_Int();
	return self->SpellShield;
}

int Lua_Item::GetAvoidance() {
	Lua_Safe_Call_Int();
	return self->Avoidance;
}

int Lua_Item::GetAccuracy() {
	Lua_Safe_Call_Int();
	return self->Accuracy;
}

uint32 Lua_Item::GetCharmFileID() {
	Lua_Safe_Call_Int();
	return self->CharmFileID;
}

int Lua_Item::GetFactionMod1() {
	Lua_Safe_Call_Int();
	return self->FactionMod1;
}

int Lua_Item::GetFactionMod2() {
	Lua_Safe_Call_Int();
	return self->FactionMod2;
}

int Lua_Item::GetFactionMod3() {
	Lua_Safe_Call_Int();
	return self->FactionMod3;
}

int Lua_Item::GetFactionMod4() {
	Lua_Safe_Call_Int();
	return self->FactionMod4;
}

int Lua_Item::GetFactionAmt1() {
	Lua_Safe_Call_Int();
	return self->FactionAmt1;
}

int Lua_Item::GetFactionAmt2() {
	Lua_Safe_Call_Int();
	return self->FactionAmt2;
}

int Lua_Item::GetFactionAmt3() {
	Lua_Safe_Call_Int();
	return self->FactionAmt3;
}

int Lua_Item::GetFactionAmt4() {
	Lua_Safe_Call_Int();
	return self->FactionAmt4;
}

const char *Lua_Item::GetCharmFile() {
	Lua_Safe_Call_String();
	return self->CharmFile;
}

uint32 Lua_Item::GetAugType() {
	Lua_Safe_Call_Int();
	return self->AugType;
}

int Lua_Item::GetAugSlotType(uint8 slot_id) {
	Lua_Safe_Call_Int();

	if (!EQ::ValueWithin(slot_id, EQ::invaug::SOCKET_BEGIN, EQ::invaug::SOCKET_END)) {
		return 0;
	}

	return self->AugSlotType[slot_id];
}

int Lua_Item::GetAugSlotVisible(uint8 slot_id) {
	Lua_Safe_Call_Int();

	if (!EQ::ValueWithin(slot_id, EQ::invaug::SOCKET_BEGIN, EQ::invaug::SOCKET_END)) {
		return 0;
	}

	return self->AugSlotVisible[slot_id];
}

int Lua_Item::GetAugSlotUnk2(uint8 slot_id) {
	Lua_Safe_Call_Int();

	if (!EQ::ValueWithin(slot_id, EQ::invaug::SOCKET_BEGIN, EQ::invaug::SOCKET_END)) {
		return 0;
	}

	return self->AugSlotUnk2[slot_id];
}

uint32 Lua_Item::GetLDoNTheme() {
	Lua_Safe_Call_Int();
	return self->LDoNTheme;
}

uint32 Lua_Item::GetLDoNPrice() {
	Lua_Safe_Call_Int();
	return self->LDoNPrice;
}

uint32 Lua_Item::GetLDoNSold() {
	Lua_Safe_Call_Int();
	return self->LDoNSold;
}

uint32 Lua_Item::GetBaneDmgRaceAmt() {
	Lua_Safe_Call_Int();
	return self->BaneDmgRaceAmt;
}

uint32 Lua_Item::GetAugRestrict() {
	Lua_Safe_Call_Int();
	return self->AugRestrict;
}

int32 Lua_Item::GetEndur() {
	Lua_Safe_Call_Int();
	return self->Endur;
}

int32 Lua_Item::GetDotShielding() {
	Lua_Safe_Call_Int();
	return self->DotShielding;
}

int32 Lua_Item::GetAttack() {
	Lua_Safe_Call_Int();
	return self->Attack;
}

int32 Lua_Item::GetRegen() {
	Lua_Safe_Call_Int();
	return self->Regen;
}

int32 Lua_Item::GetManaRegen() {
	Lua_Safe_Call_Int();
	return self->ManaRegen;
}

int32 Lua_Item::GetEnduranceRegen() {
	Lua_Safe_Call_Int();
	return self->EnduranceRegen;
}

int32 Lua_Item::GetHaste() {
	Lua_Safe_Call_Int();
	return self->Haste;
}

int32 Lua_Item::GetDamageShield() {
	Lua_Safe_Call_Int();
	return self->DamageShield;
}

uint32 Lua_Item::GetRecastDelay() {
	Lua_Safe_Call_Int();
	return self->RecastDelay;
}

uint32 Lua_Item::GetRecastType() {
	Lua_Safe_Call_Int();
	return self->RecastType;
}

uint32 Lua_Item::GetAugDistiller() {
	Lua_Safe_Call_Int();
	return self->AugDistiller;
}

bool Lua_Item::GetAttuneable() {
	Lua_Safe_Call_Bool();
	return self->Attuneable;
}

bool Lua_Item::GetNoPet() {
	Lua_Safe_Call_Bool();
	return self->NoPet;
}

bool Lua_Item::GetPotionBelt() {
	Lua_Safe_Call_Bool();
	return self->PotionBelt;
}

bool Lua_Item::GetStackable() {
	Lua_Safe_Call_Bool();
	return self->Stackable;
}

bool Lua_Item::GetNoTransfer() {
	Lua_Safe_Call_Bool();
	return self->NoTransfer;
}

bool Lua_Item::GetQuestItemFlag() {
	Lua_Safe_Call_Bool();
	return self->QuestItemFlag;
}

int Lua_Item::GetStackSize() {
	Lua_Safe_Call_Int();
	return self->StackSize;
}

int Lua_Item::GetPotionBeltSlots() {
	Lua_Safe_Call_Int();
	return self->PotionBeltSlots;
}

int Lua_Item::GetClick_Effect() {
	Lua_Safe_Call_Int();
	return self->Click.Effect;
}

int Lua_Item::GetClick_Type() {
	Lua_Safe_Call_Int();
	return self->Click.Type;
}

int Lua_Item::GetClick_Level() {
	Lua_Safe_Call_Int();
	return self->Click.Level;
}

int Lua_Item::GetClick_Level2() {
	Lua_Safe_Call_Int();
	return self->Click.Level2;
}

int Lua_Item::GetProc_Effect() {
	Lua_Safe_Call_Int();
	return self->Proc.Effect;
}

int Lua_Item::GetProc_Type() {
	Lua_Safe_Call_Int();
	return self->Proc.Type;
}

int Lua_Item::GetProc_Level() {
	Lua_Safe_Call_Int();
	return self->Proc.Level;
}

int Lua_Item::GetProc_Level2() {
	Lua_Safe_Call_Int();
	return self->Proc.Level2;
}

int Lua_Item::GetWorn_Effect() {
	Lua_Safe_Call_Int();
	return self->Worn.Effect;
}

int Lua_Item::GetWorn_Type() {
	Lua_Safe_Call_Int();
	return self->Worn.Type;
}

int Lua_Item::GetWorn_Level() {
	Lua_Safe_Call_Int();
	return self->Worn.Level;
}

int Lua_Item::GetWorn_Level2() {
	Lua_Safe_Call_Int();
	return self->Worn.Level2;
}

int Lua_Item::GetFocus_Effect() {
	Lua_Safe_Call_Int();
	return self->Focus.Effect;
}

int Lua_Item::GetFocus_Type() {
	Lua_Safe_Call_Int();
	return self->Focus.Type;
}

int Lua_Item::GetFocus_Level() {
	Lua_Safe_Call_Int();
	return self->Focus.Level;
}

int Lua_Item::GetFocus_Level2() {
	Lua_Safe_Call_Int();
	return self->Focus.Level2;
}

int Lua_Item::GetScroll_Effect() {
	Lua_Safe_Call_Int();
	return self->Scroll.Effect;
}

int Lua_Item::GetScroll_Type() {
	Lua_Safe_Call_Int();
	return self->Scroll.Type;
}

int Lua_Item::GetScroll_Level() {
	Lua_Safe_Call_Int();
	return self->Scroll.Level;
}

int Lua_Item::GetScroll_Level2() {
	Lua_Safe_Call_Int();
	return self->Scroll.Level2;
}

int Lua_Item::GetBard_Effect() {
	Lua_Safe_Call_Int();
	return self->Bard.Effect;
}

int Lua_Item::GetBard_Type() {
	Lua_Safe_Call_Int();
	return self->Bard.Type;
}

int Lua_Item::GetBard_Level() {
	Lua_Safe_Call_Int();
	return self->Bard.Level;
}

int Lua_Item::GetBard_Level2() {
	Lua_Safe_Call_Int();
	return self->Bard.Level2;
}

int Lua_Item::GetBook() {
	Lua_Safe_Call_Int();
	return self->Book;
}

uint32 Lua_Item::GetBookType() {
	Lua_Safe_Call_Int();
	return self->BookType;
}

const char *Lua_Item::GetFilename() {
	Lua_Safe_Call_String();
	return self->Filename;
}

int Lua_Item::GetSVCorruption() {
	Lua_Safe_Call_Int();
	return self->SVCorruption;
}

uint32 Lua_Item::GetPurity() {
	Lua_Safe_Call_Int();
	return self->Purity;
}

uint32 Lua_Item::GetBackstabDmg() {
	Lua_Safe_Call_Int();
	return self->BackstabDmg;
}

uint32 Lua_Item::GetDSMitigation() {
	Lua_Safe_Call_Int();
	return self->DSMitigation;
}

int Lua_Item::GetHeroicStr() {
	Lua_Safe_Call_Int();
	return self->HeroicStr;
}

int Lua_Item::GetHeroicInt() {
	Lua_Safe_Call_Int();
	return self->HeroicInt;
}

int Lua_Item::GetHeroicWis() {
	Lua_Safe_Call_Int();
	return self->HeroicWis;
}

int Lua_Item::GetHeroicAgi() {
	Lua_Safe_Call_Int();
	return self->HeroicAgi;
}

int Lua_Item::GetHeroicDex() {
	Lua_Safe_Call_Int();
	return self->HeroicDex;
}

int Lua_Item::GetHeroicSta() {
	Lua_Safe_Call_Int();
	return self->HeroicSta;
}

int Lua_Item::GetHeroicCha() {
	Lua_Safe_Call_Int();
	return self->HeroicCha;
}

int Lua_Item::GetHeroicMR() {
	Lua_Safe_Call_Int();
	return self->HeroicMR;
}

int Lua_Item::GetHeroicFR() {
	Lua_Safe_Call_Int();
	return self->HeroicFR;
}

int Lua_Item::GetHeroicCR() {
	Lua_Safe_Call_Int();
	return self->HeroicCR;
}

int Lua_Item::GetHeroicDR() {
	Lua_Safe_Call_Int();
	return self->HeroicDR;
}

int Lua_Item::GetHeroicPR() {
	Lua_Safe_Call_Int();
	return self->HeroicPR;
}

int Lua_Item::GetHeroicSVCorrup() {
	Lua_Safe_Call_Int();
	return self->HeroicSVCorrup;
}

int Lua_Item::GetHealAmt() {
	Lua_Safe_Call_Int();
	return self->HealAmt;
}

int Lua_Item::GetSpellDmg() {
	Lua_Safe_Call_Int();
	return self->SpellDmg;
}

uint32 Lua_Item::GetLDoNSellBackRate() {
	Lua_Safe_Call_Int();
	return self->LDoNSellBackRate;
}

uint32 Lua_Item::GetScriptFileID() {
	Lua_Safe_Call_Int();
	return self->ScriptFileID;
}

int Lua_Item::GetExpendableArrow() {
	Lua_Safe_Call_Int();
	return self->ExpendableArrow;
}

uint32 Lua_Item::GetClairvoyance() {
	Lua_Safe_Call_Int();
	return self->Clairvoyance;
}

const char *Lua_Item::GetClickName() {
	Lua_Safe_Call_String();
	return self->ClickName;
}

const char *Lua_Item::GetProcName() {
	Lua_Safe_Call_String();
	return self->ProcName;
}

const char *Lua_Item::GetWornName() {
	Lua_Safe_Call_String();
	return self->WornName;
}

const char *Lua_Item::GetFocusName() {
	Lua_Safe_Call_String();
	return self->FocusName;
}

const char *Lua_Item::GetScrollName() {
	Lua_Safe_Call_String();
	return self->ScrollName;
}

luabind::scope lua_register_item() {
	return luabind::class_<Lua_Item>("Item")
	.def(luabind::constructor<>())
	.def(luabind::constructor<uint32>())
	.def("null", &Lua_Item::Null)
	.def("valid", &Lua_Item::Valid)
	.def("AAgi", &Lua_Item::GetAAgi)
	.def("AC", &Lua_Item::GetAC)
	.def("ACha", &Lua_Item::GetACha)
	.def("ADex", &Lua_Item::GetADex)
	.def("AInt", &Lua_Item::GetAInt)
	.def("ASta", &Lua_Item::GetASta)
	.def("AStr", &Lua_Item::GetAStr)
	.def("AWis", &Lua_Item::GetAWis)
	.def("Accuracy", &Lua_Item::GetAccuracy)
	.def("ArtifactFlag", &Lua_Item::GetArtifactFlag)
	.def("Attack", &Lua_Item::GetAttack)
	.def("Attuneable", &Lua_Item::GetAttuneable)
	.def("AugDistiller", &Lua_Item::GetAugDistiller)
	.def("AugRestrict", &Lua_Item::GetAugRestrict)
	.def("AugSlotType", &Lua_Item::GetAugSlotType)
	.def("AugSlotUnk2", &Lua_Item::GetAugSlotUnk2)
	.def("AugSlotVisible", &Lua_Item::GetAugSlotVisible)
	.def("AugType", &Lua_Item::GetAugType)
	.def("Avoidance", &Lua_Item::GetAvoidance)
	.def("BackstabDmg", &Lua_Item::GetBackstabDmg)
	.def("BagSize", &Lua_Item::GetBagSize)
	.def("BagSlots", &Lua_Item::GetBagSlots)
	.def("BagType", &Lua_Item::GetBagType)
	.def("BagWR", &Lua_Item::GetBagWR)
	.def("BaneDmgAmt", &Lua_Item::GetBaneDmgAmt)
	.def("BaneDmgBody", &Lua_Item::GetBaneDmgBody)
	.def("BaneDmgRace", &Lua_Item::GetBaneDmgRace)
	.def("BaneDmgRaceAmt", &Lua_Item::GetBaneDmgRaceAmt)
	.def("BardType", &Lua_Item::GetBardType)
	.def("BardValue", &Lua_Item::GetBardValue)
	.def("Bard_Effect", &Lua_Item::GetBard_Effect)
	.def("Bard_Level", &Lua_Item::GetBard_Level)
	.def("Bard_Level2", &Lua_Item::GetBard_Level2)
	.def("Bard_Type", &Lua_Item::GetBard_Type)
	.def("BenefitFlag", &Lua_Item::GetBenefitFlag)
	.def("Book", &Lua_Item::GetBook)
	.def("BookType", &Lua_Item::GetBookType)
	.def("CR", &Lua_Item::GetCR)
	.def("CastTime", &Lua_Item::GetCastTime)
	.def("CastTime_", &Lua_Item::GetCastTime_)
	.def("CharmFile", &Lua_Item::GetCharmFile)
	.def("CharmFileID", &Lua_Item::GetCharmFileID)
	.def("Clairvoyance", &Lua_Item::GetClairvoyance)
	.def("Classes", &Lua_Item::GetClasses)
	.def("ClickName", &Lua_Item::GetClickName)
	.def("Click_Effect", &Lua_Item::GetClick_Effect)
	.def("Click_Level", &Lua_Item::GetClick_Level)
	.def("Click_Level2", &Lua_Item::GetClick_Level2)
	.def("Click_Type", &Lua_Item::GetClick_Type)
	.def("Color", &Lua_Item::GetColor)
	.def("CombatEffects", &Lua_Item::GetCombatEffects)
	.def("Comment", &Lua_Item::GetComment)
	.def("DR", &Lua_Item::GetDR)
	.def("DSMitigation", &Lua_Item::GetDSMitigation)
	.def("Damage", &Lua_Item::GetDamage)
	.def("DamageShield", &Lua_Item::GetDamageShield)
	.def("Deity", &Lua_Item::GetDeity)
	.def("Delay", &Lua_Item::GetDelay)
	.def("DotShielding", &Lua_Item::GetDotShielding)
	.def("ElemDmgAmt", &Lua_Item::GetElemDmgAmt)
	.def("ElemDmgType", &Lua_Item::GetElemDmgType)
	.def("EliteMaterial", &Lua_Item::GetEliteMaterial)
	.def("Endur", &Lua_Item::GetEndur)
	.def("EnduranceRegen", &Lua_Item::GetEnduranceRegen)
	.def("ExpendableArrow", &Lua_Item::GetExpendableArrow)
	.def("ExtraDmgAmt", &Lua_Item::GetExtraDmgAmt)
	.def("ExtraDmgSkill", &Lua_Item::GetExtraDmgSkill)
	.def("FR", &Lua_Item::GetFR)
	.def("FVNoDrop", &Lua_Item::GetFVNoDrop)
	.def("FactionAmt1", &Lua_Item::GetFactionAmt1)
	.def("FactionAmt2", &Lua_Item::GetFactionAmt2)
	.def("FactionAmt3", &Lua_Item::GetFactionAmt3)
	.def("FactionAmt4", &Lua_Item::GetFactionAmt4)
	.def("FactionMod1", &Lua_Item::GetFactionMod1)
	.def("FactionMod2", &Lua_Item::GetFactionMod2)
	.def("FactionMod3", &Lua_Item::GetFactionMod3)
	.def("FactionMod4", &Lua_Item::GetFactionMod4)
	.def("Favor", &Lua_Item::GetFavor)
	.def("Filename", &Lua_Item::GetFilename)
	.def("FocusName", &Lua_Item::GetFocusName)
	.def("Focus_Effect", &Lua_Item::GetFocus_Effect)
	.def("Focus_Level", &Lua_Item::GetFocus_Level)
	.def("Focus_Level2", &Lua_Item::GetFocus_Level2)
	.def("Focus_Type", &Lua_Item::GetFocus_Type)
	.def("Fulfilment", &Lua_Item::GetFulfilment)
	.def("GuildFavor", &Lua_Item::GetGuildFavor)
	.def("HP", &Lua_Item::GetHP)
	.def("Haste", &Lua_Item::GetHaste)
	.def("HealAmt", &Lua_Item::GetHealAmt)
	.def("HeroicAgi", &Lua_Item::GetHeroicAgi)
	.def("HeroicCR", &Lua_Item::GetHeroicCR)
	.def("HeroicCha", &Lua_Item::GetHeroicCha)
	.def("HeroicDR", &Lua_Item::GetHeroicDR)
	.def("HeroicDex", &Lua_Item::GetHeroicDex)
	.def("HeroicFR", &Lua_Item::GetHeroicFR)
	.def("HeroicInt", &Lua_Item::GetHeroicInt)
	.def("HeroicMR", &Lua_Item::GetHeroicMR)
	.def("HeroicPR", &Lua_Item::GetHeroicPR)
	.def("HeroicSVCorrup", &Lua_Item::GetHeroicSVCorrup)
	.def("HeroicSta", &Lua_Item::GetHeroicSta)
	.def("HeroicStr", &Lua_Item::GetHeroicStr)
	.def("HeroicWis", &Lua_Item::GetHeroicWis)
	.def("ID", &Lua_Item::GetID)
	.def("IDFile", &Lua_Item::GetIDFile)
	.def("Icon", &Lua_Item::GetIcon)
	.def("ItemClass", &Lua_Item::GetItemClass)
	.def("ItemType", &Lua_Item::GetItemType)
	.def("LDoNPrice", &Lua_Item::GetLDoNPrice)
	.def("LDoNSellBackRate", &Lua_Item::GetLDoNSellBackRate)
	.def("LDoNSold", &Lua_Item::GetLDoNSold)
	.def("LDoNTheme", &Lua_Item::GetLDoNTheme)
	.def("Light", &Lua_Item::GetLight)
	.def("Lore", &Lua_Item::GetLore)
	.def("LoreFlag", &Lua_Item::GetLoreFlag)
	.def("LoreGroup", &Lua_Item::GetLoreGroup)
	.def("MR", &Lua_Item::GetMR)
	.def("Magic", &Lua_Item::GetMagic)
	.def("Mana", &Lua_Item::GetMana)
	.def("ManaRegen", &Lua_Item::GetManaRegen)
	.def("Material", &Lua_Item::GetMaterial)
	.def("MaxCharges", &Lua_Item::GetMaxCharges)
	.def("MinStatus", &Lua_Item::GetMinStatus)
	.def("Name", &Lua_Item::GetName)
	.def("NoDrop", &Lua_Item::GetNoDrop)
	.def("NoPet", &Lua_Item::GetNoPet)
	.def("NoRent", &Lua_Item::GetNoRent)
	.def("NoTransfer", &Lua_Item::GetNoTransfer)
	.def("PR", &Lua_Item::GetPR)
	.def("PendingLoreFlag", &Lua_Item::GetPendingLoreFlag)
	.def("PointType", &Lua_Item::GetPointType)
	.def("PotionBelt", &Lua_Item::GetPotionBelt)
	.def("PotionBeltSlots", &Lua_Item::GetPotionBeltSlots)
	.def("Price", &Lua_Item::GetPrice)
	.def("ProcName", &Lua_Item::GetProcName)
	.def("ProcRate", &Lua_Item::GetProcRate)
	.def("Proc_Effect", &Lua_Item::GetProc_Effect)
	.def("Proc_Level", &Lua_Item::GetProc_Level)
	.def("Proc_Level2", &Lua_Item::GetProc_Level2)
	.def("Proc_Type", &Lua_Item::GetProc_Type)
	.def("Purity", &Lua_Item::GetPurity)
	.def("QuestItemFlag", &Lua_Item::GetQuestItemFlag)
	.def("Races", &Lua_Item::GetRaces)
	.def("Range", &Lua_Item::GetRange)
	.def("RecLevel", &Lua_Item::GetRecLevel)
	.def("RecSkill", &Lua_Item::GetRecSkill)
	.def("RecastDelay", &Lua_Item::GetRecastDelay)
	.def("RecastType", &Lua_Item::GetRecastType)
	.def("Regen", &Lua_Item::GetRegen)
	.def("ReqLevel", &Lua_Item::GetReqLevel)
	.def("SVCorruption", &Lua_Item::GetSVCorruption)
	.def("ScriptFileID", &Lua_Item::GetScriptFileID)
	.def("ScrollName", &Lua_Item::GetScrollName)
	.def("Scroll_Effect", &Lua_Item::GetScroll_Effect)
	.def("Scroll_Level", &Lua_Item::GetScroll_Level)
	.def("Scroll_Level2", &Lua_Item::GetScroll_Level2)
	.def("Scroll_Type", &Lua_Item::GetScroll_Type)
	.def("SellRate", &Lua_Item::GetSellRate)
	.def("Shielding", &Lua_Item::GetShielding)
	.def("Size", &Lua_Item::GetSize)
	.def("SkillModType", &Lua_Item::GetSkillModType)
	.def("SkillModValue", &Lua_Item::GetSkillModValue)
	.def("Slots", &Lua_Item::GetSlots)
	.def("SpellDmg", &Lua_Item::GetSpellDmg)
	.def("SpellShield", &Lua_Item::GetSpellShield)
	.def("StackSize", &Lua_Item::GetStackSize)
	.def("Stackable", &Lua_Item::GetStackable)
	.def("StrikeThrough", &Lua_Item::GetStrikeThrough)
	.def("StunResist", &Lua_Item::GetStunResist)
	.def("SummonedFlag", &Lua_Item::GetSummonedFlag)
	.def("Tradeskills", &Lua_Item::GetTradeskills)
	.def("Weight", &Lua_Item::GetWeight)
	.def("WornName", &Lua_Item::GetWornName)
	.def("Worn_Effect", &Lua_Item::GetWorn_Effect)
	.def("Worn_Level", &Lua_Item::GetWorn_Level)
	.def("Worn_Level2", &Lua_Item::GetWorn_Level2)
	.def("Worn_Type", &Lua_Item::GetWorn_Type);
}

#endif // LUA_EQEMU
