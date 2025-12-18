#pragma once

#ifdef LUA_EQEMU

#include "common/types.h"
#include "zone/lua_ptr.h"

class Lua_Item;

namespace EQ
{
	class ItemInstance;
}

namespace luabind {
	struct scope;
}

luabind::scope lua_register_iteminst();

class Lua_ItemInst : public Lua_Ptr<EQ::ItemInstance>
{
	typedef EQ::ItemInstance NativeType;
public:
	Lua_ItemInst(int item_id);
	Lua_ItemInst(int item_id, int charges);
	Lua_ItemInst() : Lua_Ptr(nullptr), cloned_(false) { }
	Lua_ItemInst(EQ::ItemInstance *d) : Lua_Ptr(d), cloned_(false) { }
	Lua_ItemInst(EQ::ItemInstance *d, bool cloned) : Lua_Ptr(d), cloned_(cloned) { }
	Lua_ItemInst& operator=(const Lua_ItemInst& o);
	Lua_ItemInst(const Lua_ItemInst& o);
	virtual ~Lua_ItemInst();

	operator EQ::ItemInstance*() {
		return reinterpret_cast<EQ::ItemInstance*>(GetLuaPtrData());
	}

	bool IsType(int item_class);
	bool IsStackable();
	bool IsEquipable(uint16 race_bitmask, uint16 class_bitmask);
	bool IsEquipable(int16 slot_id);
	bool IsAugmentable();
	int GetAugmentType();
	bool IsExpendable();
	Lua_ItemInst GetItem(uint8 slot_id);
	Lua_Item GetItem();
	Lua_Item GetUnscaledItem();
	uint32 GetItemID(uint8 slot_id);
	uint8 GetTotalItemCount();
	Lua_ItemInst GetAugment(uint8 slot_id);
	uint32 GetAugmentItemID(uint8 slot_id);
	bool IsAugmented();
	bool IsWeapon();
	bool IsAmmo();
	uint32 GetID();
	uint32 GetItemScriptID();
	int GetCharges();
	void SetCharges(int charges);
	uint32 GetPrice();
	void SetPrice(uint32 price);
	void SetColor(uint32 color);
	uint32 GetColor();
	bool IsInstNoDrop();
	bool IsAttuned();
	void SetInstNoDrop(bool flag);
	void SetAttuned(bool flag);
	std::string GetCustomDataString();
	void SetCustomData(const std::string &identifier, const std::string &value);
	void SetCustomData(const std::string &identifier, int value);
	void SetCustomData(const std::string &identifier, float value);
	void SetCustomData(const std::string &identifier, bool value);
	std::string GetCustomData(const std::string& identifier);
	void DeleteCustomData(const std::string& identifier);
	void SetScaling(bool v);
	void SetScale(double scale_factor);
	uint32 GetExp();
	void SetExp(uint32 exp);
	void AddExp(uint32 exp);
	int8 GetMaxEvolveLvl();
	Lua_ItemInst Clone();
	void SetTimer(std::string name, uint32 time);
	void StopTimer(std::string name);
	void ClearTimers();
	bool ContainsAugmentByID(uint32 item_id);
	int CountAugmentByID(uint32 item_id);
	int GetTaskDeliveredCount();
	int RemoveTaskDeliveredItems();
	std::string GetName();
	int GetSerialNumber();
	void ItemSay(const char* text);
	void ItemSay(const char* text, uint8 language_id);
	luabind::object GetAugmentIDs(lua_State* L);
	std::string GetItemLink();
	void AddEvolveAmount(uint64 amount);
	uint32 GetAugmentEvolveUniqueID(uint8 slot_id);
	bool GetEvolveActivated();
	uint64 GetEvolveAmount();
	uint32 GetEvolveCharacterID();
	bool GetEvolveEquipped();
	uint32 GetEvolveFinalItemID();
	uint32 GetEvolveItemID();
	int8 GetEvolveLevel();
	uint32 GetEvolveLoreID();
	float GetEvolveProgression();
	uint64 GetEvolveUniqueID();
	bool IsEvolving();
	void SetEvolveAmount(uint64 amount);
	void SetEvolveProgression(float amount);

private:
	bool cloned_;
};

#endif // LUA_EQEMU
