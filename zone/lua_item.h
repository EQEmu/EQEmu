#pragma once

#ifdef LUA_EQEMU

#include "common/types.h"
#include "zone/lua_ptr.h"

namespace EQ
{
	struct ItemData;
}

namespace luabind {
	struct scope;
}

luabind::scope lua_register_item();

class Lua_Item : public Lua_Ptr<const EQ::ItemData>
{
	typedef const EQ::ItemData NativeType;
public:
	Lua_Item(uint32 item_id);
	Lua_Item() : Lua_Ptr(nullptr) { }
	Lua_Item(const EQ::ItemData *d) : Lua_Ptr(d) { }
	virtual ~Lua_Item() { }

	operator const EQ::ItemData*() {
		return reinterpret_cast<const EQ::ItemData*>(GetLuaPtrData());
	}

	int GetMinStatus();
	int GetItemClass();
	const char *GetName();
	const char *GetLore();
	const char *GetComment();
	const char *GetIDFile();
	uint32 GetID();
	int GetWeight();
	int GetNoRent();
	int GetNoDrop();
	int GetSize();
	uint32 GetSlots();
	uint32 GetPrice();
	uint32 GetIcon();
	int32 GetLoreGroup();
	bool GetLoreFlag();
	bool GetPendingLoreFlag();
	bool GetArtifactFlag();
	bool GetSummonedFlag();
	int GetFVNoDrop();
	uint32 GetFavor();
	uint32 GetGuildFavor();
	uint32 GetPointType();
	int GetBagType();
	int GetBagSlots();
	int GetBagSize();
	int GetBagWR();
	bool GetBenefitFlag();
	bool GetTradeskills();
	int GetCR();
	int GetDR();
	int GetPR();
	int GetMR();
	int GetFR();
	int GetAStr();
	int GetASta();
	int GetAAgi();
	int GetADex();
	int GetACha();
	int GetAInt();
	int GetAWis();
	int GetHP();
	int GetMana();
	int GetAC();
	uint32 GetDeity();
	int GetSkillModValue();
	uint32 GetSkillModType();
	uint32 GetBaneDmgRace();
	int GetBaneDmgAmt();
	uint32 GetBaneDmgBody();
	bool GetMagic();
	int GetCastTime_();
	int GetReqLevel();
	uint32 GetBardType();
	int GetBardValue();
	int GetLight();
	int GetDelay();
	int GetRecLevel();
	int GetRecSkill();
	int GetElemDmgType();
	int GetElemDmgAmt();
	int GetRange();
	uint32 GetDamage();
	uint32 GetColor();
	uint32 GetClasses();
	uint32 GetRaces();
	int GetMaxCharges();
	int GetItemType();
	int GetMaterial();
	double GetSellRate();
	uint32 GetFulfilment();
	int GetCastTime();
	uint32 GetEliteMaterial();
	int GetProcRate();
	int GetCombatEffects();
	int GetShielding();
	int GetStunResist();
	int GetStrikeThrough();
	uint32 GetExtraDmgSkill();
	uint32 GetExtraDmgAmt();
	int GetSpellShield();
	int GetAvoidance();
	int GetAccuracy();
	uint32 GetCharmFileID();
	int GetFactionMod1();
	int GetFactionMod2();
	int GetFactionMod3();
	int GetFactionMod4();
	int GetFactionAmt1();
	int GetFactionAmt2();
	int GetFactionAmt3();
	int GetFactionAmt4();
	const char *GetCharmFile();
	uint32 GetAugType();
	int GetAugSlotType(uint8 slot_id);
	int GetAugSlotVisible(uint8 slot_id);
	int GetAugSlotUnk2(uint8 slot_id);
	uint32 GetLDoNTheme();
	uint32 GetLDoNPrice();
	uint32 GetLDoNSold();
	uint32 GetBaneDmgRaceAmt();
	uint32 GetAugRestrict();
	int32 GetEndur();
	int32 GetDotShielding();
	int32 GetAttack();
	int32 GetRegen();
	int32 GetManaRegen();
	int32 GetEnduranceRegen();
	int32 GetHaste();
	int32 GetDamageShield();
	uint32 GetRecastDelay();
	uint32 GetRecastType();
	uint32 GetAugDistiller();
	bool GetAttuneable();
	bool GetNoPet();
	bool GetPotionBelt();
	bool GetStackable();
	bool GetNoTransfer();
	bool GetQuestItemFlag();
	int GetStackSize();
	int GetPotionBeltSlots();
	int GetClick_Effect();
	int GetClick_Type();
	int GetClick_Level();
	int GetClick_Level2();
	int GetProc_Effect();
	int GetProc_Type();
	int GetProc_Level();
	int GetProc_Level2();
	int GetWorn_Effect();
	int GetWorn_Type();
	int GetWorn_Level();
	int GetWorn_Level2();
	int GetFocus_Effect();
	int GetFocus_Type();
	int GetFocus_Level();
	int GetFocus_Level2();
	int GetScroll_Effect();
	int GetScroll_Type();
	int GetScroll_Level();
	int GetScroll_Level2();
	int GetBard_Effect();
	int GetBard_Type();
	int GetBard_Level();
	int GetBard_Level2();
	int GetBook();
	uint32 GetBookType();
	const char *GetFilename();
	int GetSVCorruption();
	uint32 GetPurity();
	uint32 GetBackstabDmg();
	uint32 GetDSMitigation();
	int GetHeroicStr();
	int GetHeroicInt();
	int GetHeroicWis();
	int GetHeroicAgi();
	int GetHeroicDex();
	int GetHeroicSta();
	int GetHeroicCha();
	int GetHeroicMR();
	int GetHeroicFR();
	int GetHeroicCR();
	int GetHeroicDR();
	int GetHeroicPR();
	int GetHeroicSVCorrup();
	int GetHealAmt();
	int GetSpellDmg();
	uint32 GetLDoNSellBackRate();
	uint32 GetScriptFileID();
	int GetExpendableArrow();
	uint32 GetClairvoyance();
	const char *GetClickName();
	const char *GetProcName();
	const char *GetWornName();
	const char *GetFocusName();
	const char *GetScrollName();
};

#endif // LUA_EQEMU
