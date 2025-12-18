#pragma once

#ifdef LUA_EQEMU

#include "zone/lua_ptr.h"

class Lua_ItemInst;
class Lua_Item;

namespace EQ
{
	class InventoryProfile;
}

namespace luabind {
	struct scope;
}

luabind::scope lua_register_inventory();

// This class should be deprecated due to the nature of inventory actions.
// Direct manipulation of the inventory system bypasses the client management
// of database calls and can lead to lost items, duplicated items and/or
// desync'd inventories, if not handled correctly.

class Lua_Inventory : public Lua_Ptr<EQ::InventoryProfile>
{
	typedef EQ::InventoryProfile NativeType;
public:
	Lua_Inventory() : Lua_Ptr(nullptr) { }
	Lua_Inventory(EQ::InventoryProfile *d) : Lua_Ptr(d) { }
	virtual ~Lua_Inventory() { }

	operator EQ::InventoryProfile*() {
		return reinterpret_cast<EQ::InventoryProfile*>(GetLuaPtrData());
	}

	Lua_ItemInst GetItem(int slot_id);
	Lua_ItemInst GetItem(int slot_id, int bag_slot);
	int PutItem(int slot_id, Lua_ItemInst item);
	int PushCursor(Lua_ItemInst item);
	bool SwapItem(int source_slot, int destination_slot);
	bool DeleteItem(int slot_id);
	bool DeleteItem(int slot_id, int quantity);
	bool CheckNoDrop(int slot_id);
	uint32 CountAugmentEquippedByID(uint32 item_id);
	uint32 CountItemEquippedByID(uint32 item_id);
	Lua_ItemInst PopItem(int slot_id);
	bool HasAugmentEquippedByID(uint32 item_id);
	bool HasItemEquippedByID(uint32 item_id);
	int HasItem(int item_id);
	int HasItem(int item_id, int quantity);
	int HasItem(int item_id, int quantity, int where);
	bool HasSpaceForItem(Lua_Item item, int quantity);
	int HasItemByUse(int use);
	int HasItemByUse(int use, uint8 quantity);
	int HasItemByUse(int use, uint8 quantity, uint8 where);
	int HasItemByLoreGroup(uint32 loregroup);
	int HasItemByLoreGroup(uint32 loregroup, int where);
	int FindFreeSlot(bool for_bag, bool try_cursor);
	int FindFreeSlot(bool for_bag, bool try_cursor, int min_size);
	int FindFreeSlot(bool for_bag, bool try_cursor, int min_size, bool is_arrow);
	int CalcSlotId(int slot_id);
	int CalcSlotId(int slot_id, int bag_slot);
	int CalcBagIdx(int slot_id);
	int CalcSlotFromMaterial(int material);
	int CalcMaterialFromSlot(int equipslot);
	bool CanItemFitInContainer(Lua_Item item, Lua_Item container);
	bool SupportsContainers(int slot_id);
	int GetSlotByItemInst(Lua_ItemInst inst);
	luabind::object GetAugmentIDsBySlotID(lua_State* L, int16 slot_id);
};

#endif // LUA_EQEMU
