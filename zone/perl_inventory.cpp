#include "common/features.h"

#ifdef EMBPERL_XS_CLASSES

#include "zone/client.h"
#include "zone/embperl.h"

bool Perl_Inventory_CanItemFitInContainer(EQ::InventoryProfile* self, EQ::ItemInstance* item_to_check, EQ::ItemInstance* container_to_check)
{
	const EQ::ItemData* item_data = item_to_check->GetItem();
	const EQ::ItemData* container_data = container_to_check->GetItem();
	return self->CanItemFitInContainer(item_data, container_data);
}

bool Perl_Inventory_CheckNoDrop(EQ::InventoryProfile* self, int16_t slot_id)
{
	return self->CheckNoDrop(slot_id);
}

bool Perl_Inventory_DeleteItem(EQ::InventoryProfile* self, int16_t slot_id)
{
	return self->DeleteItem(slot_id);
}

bool Perl_Inventory_DeleteItem(EQ::InventoryProfile* self, int16_t slot_id, uint8_t quantity)
{
	return self->DeleteItem(slot_id, quantity);
}

int Perl_Inventory_FindFreeSlot(EQ::InventoryProfile* self, bool is_for_bag, bool try_cursor)
{
	return self->FindFreeSlot(is_for_bag, try_cursor);
}

int Perl_Inventory_FindFreeSlot(EQ::InventoryProfile* self, bool is_for_bag, bool try_cursor, uint8_t min_size)
{
	return self->FindFreeSlot(is_for_bag, try_cursor, min_size);
}

int Perl_Inventory_FindFreeSlot(EQ::InventoryProfile* self, bool is_for_bag, bool try_cursor, uint8_t min_size, bool is_arrow)
{
	return self->FindFreeSlot(is_for_bag, try_cursor, min_size, is_arrow);
}

int Perl_Inventory_GetBagIndex(EQ::InventoryProfile* self, int16_t slot_id)
{
	return self->CalcBagIdx(slot_id);
}

EQ::ItemInstance* Perl_Inventory_GetItem(EQ::InventoryProfile* self, int16_t slot_id)
{
	return self->GetItem(slot_id);
}

int Perl_Inventory_GetMaterialFromSlot(EQ::InventoryProfile* self, int16_t slot_id)
{
	return self->CalcMaterialFromSlot(slot_id);
}

int Perl_Inventory_GetSlotByItemInst(EQ::InventoryProfile* self, EQ::ItemInstance* item)
{
	return self->GetSlotByItemInst(item);
}

int Perl_Inventory_GetSlotFromMaterial(EQ::InventoryProfile* self, uint8_t material)
{
	return self->CalcSlotFromMaterial(material);
}

int Perl_Inventory_GetSlotID(EQ::InventoryProfile* self, int16_t slot_id)
{
	return self->CalcSlotId(slot_id);
}

int Perl_Inventory_GetSlotID(EQ::InventoryProfile* self, int16_t slot_id, uint8_t bag_index)
{
	return self->CalcSlotId(slot_id, bag_index);
}

int Perl_Inventory_HasItem(EQ::InventoryProfile* self, uint32_t item_id)
{
	return self->HasItem(item_id);
}

int Perl_Inventory_HasItem(EQ::InventoryProfile* self, uint32_t item_id, uint8_t quantity)
{
	return self->HasItem(item_id, quantity);
}

int Perl_Inventory_HasItem(EQ::InventoryProfile* self, uint32_t item_id, uint8_t quantity, uint8_t where_to_look)
{
	return self->HasItem(item_id, quantity, where_to_look);
}

int Perl_Inventory_HasItemByLoreGroup(EQ::InventoryProfile* self, uint32_t loregroup)
{
	return self->HasItemByLoreGroup(loregroup);
}

int Perl_Inventory_HasItemByLoreGroup(EQ::InventoryProfile* self, uint32_t loregroup, uint8_t where_to_look)
{
	return self->HasItemByLoreGroup(loregroup, where_to_look);
}

int Perl_Inventory_HasItemByUse(EQ::InventoryProfile* self, uint8_t item_use, uint8_t quantity)
{
	return self->HasItemByUse(item_use, quantity);
}

int Perl_Inventory_HasItemByUse(EQ::InventoryProfile* self, uint8_t item_use, uint8_t quantity, uint8_t where_to_look)
{
	return self->HasItemByUse(item_use, quantity, where_to_look);
}

bool Perl_Inventory_HasSpaceForItem(EQ::InventoryProfile* self, EQ::ItemInstance* item_to_check, uint8_t quantity)
{
	const EQ::ItemData* item_data = item_to_check->GetItem();
	return self->HasSpaceForItem(item_data, quantity);
}

EQ::ItemInstance* Perl_Inventory_PopItem(EQ::InventoryProfile* self, int16_t slot_id)
{
	return self->PopItem(slot_id);
}

bool Perl_Inventory_SupportsContainers(EQ::InventoryProfile* self, int16_t slot_id)
{
	return self->SupportsContainers(slot_id);
}

bool Perl_Inventory_SwapItem(EQ::InventoryProfile* self, int16_t source_slot_id, int16_t destination_slot_id)
{
	EQ::InventoryProfile::SwapItemFailState fail_state = EQ::InventoryProfile::swapInvalid;
	return self->SwapItem(source_slot_id, destination_slot_id, fail_state);
}

int Perl_Inventory_PushCursor(EQ::InventoryProfile* self, EQ::ItemInstance* item)
{
	return self->PushCursor(*item);
}

int Perl_Inventory_PutItem(EQ::InventoryProfile* self, int16_t slot_id, EQ::ItemInstance* item)
{
	return self->PutItem(slot_id, *item);
}

bool Perl_Inventory_HasAugmentEquippedByID(EQ::InventoryProfile* self, uint32_t item_id)
{
	return self->HasAugmentEquippedByID(item_id);
}

uint32 Perl_Inventory_CountAugmentEquippedByID(EQ::InventoryProfile* self, uint32_t item_id)
{
	return self->CountAugmentEquippedByID(item_id);
}

bool Perl_Inventory_HasItemEquippedByID(EQ::InventoryProfile* self, uint32_t item_id)
{
	return self->HasItemEquippedByID(item_id);
}

uint32 Perl_Inventory_CountItemEquippedByID(EQ::InventoryProfile* self, uint32_t item_id)
{
	return self->CountItemEquippedByID(item_id);
}

perl::array Perl_Inventory_GetAugmentIDsBySlotID(EQ::InventoryProfile* self, int16 slot_id)
{
	perl::array result;
	auto augments = self->GetAugmentIDsBySlotID(slot_id);

	for (int i = 0; i < augments.size(); ++i) {
		result.push_back(augments[i]);
	}

	return result;
}

void perl_register_inventory()
{
	perl::interpreter perl(PERL_GET_THX);

	auto package = perl.new_class<EQ::InventoryProfile>("Inventory");
	package.add("CanItemFitInContainer", &Perl_Inventory_CanItemFitInContainer);
	package.add("CountAugmentEquippedByID", &Perl_Inventory_CountAugmentEquippedByID);
	package.add("CountItemEquippedByID", &Perl_Inventory_CountItemEquippedByID);
	package.add("CheckNoDrop", &Perl_Inventory_CheckNoDrop);
	package.add("DeleteItem", (bool(*)(EQ::InventoryProfile*, int16_t))&Perl_Inventory_DeleteItem);
	package.add("DeleteItem", (bool(*)(EQ::InventoryProfile*, int16_t, uint8_t))&Perl_Inventory_DeleteItem);
	package.add("FindFreeSlot", (int(*)(EQ::InventoryProfile*, bool, bool))&Perl_Inventory_FindFreeSlot);
	package.add("FindFreeSlot", (int(*)(EQ::InventoryProfile*, bool, bool, uint8_t))&Perl_Inventory_FindFreeSlot);
	package.add("FindFreeSlot", (int(*)(EQ::InventoryProfile*, bool, bool, uint8_t, bool))&Perl_Inventory_FindFreeSlot);
	package.add("GetAugmentIDsBySlotID", &Perl_Inventory_GetAugmentIDsBySlotID);
	package.add("GetBagIndex", &Perl_Inventory_GetBagIndex);
	package.add("GetItem", &Perl_Inventory_GetItem);
	package.add("GetMaterialFromSlot", &Perl_Inventory_GetMaterialFromSlot);
	package.add("GetSlotByItemInst", &Perl_Inventory_GetSlotByItemInst);
	package.add("GetSlotFromMaterial", &Perl_Inventory_GetSlotFromMaterial);
	package.add("GetSlotID", (int(*)(EQ::InventoryProfile*, int16_t))&Perl_Inventory_GetSlotID);
	package.add("GetSlotID", (int(*)(EQ::InventoryProfile*, int16_t, uint8_t))&Perl_Inventory_GetSlotID);
	package.add("HasAugmentEquippedByID", &Perl_Inventory_HasAugmentEquippedByID);
	package.add("HasItem", (int(*)(EQ::InventoryProfile*, uint32_t))&Perl_Inventory_HasItem);
	package.add("HasItem", (int(*)(EQ::InventoryProfile*, uint32_t, uint8_t))&Perl_Inventory_HasItem);
	package.add("HasItem", (int(*)(EQ::InventoryProfile*, uint32_t, uint8_t, uint8_t))&Perl_Inventory_HasItem);
	package.add("HasItemByLoreGroup", (int(*)(EQ::InventoryProfile*, uint32_t))&Perl_Inventory_HasItemByLoreGroup);
	package.add("HasItemByLoreGroup", (int(*)(EQ::InventoryProfile*, uint32_t, uint8_t))&Perl_Inventory_HasItemByLoreGroup);
	package.add("HasItemByUse", (int(*)(EQ::InventoryProfile*, uint8_t, uint8_t))&Perl_Inventory_HasItemByUse);
	package.add("HasItemByUse", (int(*)(EQ::InventoryProfile*, uint8_t, uint8_t, uint8_t))&Perl_Inventory_HasItemByUse);
	package.add("HasItemEquippedByID", &Perl_Inventory_HasItemEquippedByID);
	package.add("HasSpaceForItem", &Perl_Inventory_HasSpaceForItem);
	package.add("PopItem", &Perl_Inventory_PopItem);
	package.add("PushCursor", &Perl_Inventory_PushCursor);
	package.add("PutItem", &Perl_Inventory_PutItem);
	package.add("SupportsContainers", &Perl_Inventory_SupportsContainers);
	package.add("SwapItem", &Perl_Inventory_SwapItem);
}

#endif //EMBPERL_XS_CLASSES
