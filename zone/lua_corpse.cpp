#ifdef LUA_EQEMU

#include "lua_corpse.h"

#include "zone/corpse.h"
#include "zone/lua_client.h"

#include "lua.hpp"
#include "luabind/iterator_policy.hpp"
#include "luabind/luabind.hpp"

struct Lua_Corpse_Loot_List {
	std::vector<uint32> entries;
};

uint32 Lua_Corpse::GetCharID() {
	Lua_Safe_Call_Int();
	return self->GetCharID();
}

uint32 Lua_Corpse::GetDecayTime() {
	Lua_Safe_Call_Int();
	return self->GetDecayTime();
}

void Lua_Corpse::Lock() {
	Lua_Safe_Call_Void();
	self->Lock();
}

void Lua_Corpse::UnLock() {
	Lua_Safe_Call_Void();
	self->UnLock();
}

bool Lua_Corpse::IsLocked() {
	Lua_Safe_Call_Bool();
	return self->IsLocked();
}

void Lua_Corpse::ResetLooter() {
	Lua_Safe_Call_Void();
	self->ResetLooter();
}

uint32 Lua_Corpse::GetDBID() {
	Lua_Safe_Call_Int();
	return self->GetCorpseDBID();
}

bool Lua_Corpse::IsRezzed() {
	Lua_Safe_Call_Bool();
	return self->IsRezzed();
}

const char* Lua_Corpse::GetOwnerName() {
	Lua_Safe_Call_String();
	return self->GetOwnerName();
}

bool Lua_Corpse::Save() {
	Lua_Safe_Call_Bool();
	return self->Save();
}

void Lua_Corpse::Delete() {
	Lua_Safe_Call_Void();
	self->Delete();
}

void Lua_Corpse::Bury() {
	Lua_Safe_Call_Void();
	self->Bury();
}

void Lua_Corpse::Depop() {
	Lua_Safe_Call_Void();
	self->Depop();
}

uint32 Lua_Corpse::CountItems() {
	Lua_Safe_Call_Int();
	return self->CountItems();
}

void Lua_Corpse::AddItem(uint32 itemnum, uint16 charges) {
	Lua_Safe_Call_Void();
	self->AddItem(itemnum, charges);
}

void Lua_Corpse::AddItem(uint32 itemnum, uint16 charges, int16 slot) {
	Lua_Safe_Call_Void();
	self->AddItem(itemnum, charges, slot);
}

void Lua_Corpse::AddItem(uint32 itemnum, uint16 charges, int16 slot, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5) {
	Lua_Safe_Call_Void();
	self->AddItem(itemnum, charges, slot, aug1, aug2, aug3, aug4, aug5);
}

uint32 Lua_Corpse::GetWornItem(int16 equipSlot) {
	Lua_Safe_Call_Int();
	return self->GetWornItem(equipSlot);
}

void Lua_Corpse::RemoveItem(uint16 lootslot) {
	Lua_Safe_Call_Void();
	self->RemoveItem(lootslot);
}

void Lua_Corpse::SetCash(uint32 copper, uint32 silver, uint32 gold, uint32 platinum) {
	Lua_Safe_Call_Void();
	self->SetCash(copper, silver, gold, platinum);
}

void Lua_Corpse::RemoveLootCash() {
	Lua_Safe_Call_Void();
	self->RemoveCash();
}

bool Lua_Corpse::IsEmpty() {
	Lua_Safe_Call_Bool();
	return self->IsEmpty();
}

void Lua_Corpse::ResetDecayTimer() {
	Lua_Safe_Call_Void();
	self->ResetDecayTimer();
}

void Lua_Corpse::SetDecayTimer(uint32 decaytime) {
	Lua_Safe_Call_Void();
	self->SetDecayTimer(decaytime);
}

bool Lua_Corpse::CanMobLoot(int charid) {
	Lua_Safe_Call_Bool();
	return self->CanPlayerLoot(charid);
}

void Lua_Corpse::AllowMobLoot(Lua_Mob them, uint8 slot) {
	Lua_Safe_Call_Void();
	self->AllowPlayerLoot(them, slot);
}

bool Lua_Corpse::Summon(Lua_Client client, bool spell, bool checkdistance) {
	Lua_Safe_Call_Bool();
	return self->Summon(client, spell, checkdistance);
}

uint32 Lua_Corpse::GetCopper() {
	Lua_Safe_Call_Int();
	return self->GetCopper();
}

uint32 Lua_Corpse::GetSilver() {
	Lua_Safe_Call_Int();
	return self->GetSilver();
}

uint32 Lua_Corpse::GetGold() {
	Lua_Safe_Call_Int();
	return self->GetGold();
}

uint32 Lua_Corpse::GetPlatinum() {
	Lua_Safe_Call_Int();
	return self->GetPlatinum();
}

void Lua_Corpse::AddLooter(Lua_Mob who) {
	Lua_Safe_Call_Void();
	self->AddLooter(who);
}

bool Lua_Corpse::HasItem(uint32 item_id) {
	Lua_Safe_Call_Bool();
	return self->HasItem(item_id);
}

uint32 Lua_Corpse::CountItem(uint32 item_id) {
	Lua_Safe_Call_Int();
	return self->CountItem(item_id);
}

uint32 Lua_Corpse::GetItemIDBySlot(uint16 loot_slot) {
	Lua_Safe_Call_Int();
	return self->GetItemIDBySlot(loot_slot);
}

uint16 Lua_Corpse::GetFirstLootSlotByItemID(uint32 item_id) {
	Lua_Safe_Call_Int();
	return self->GetFirstLootSlotByItemID(item_id);
}

void Lua_Corpse::RemoveItemByID(uint32 item_id) {
	Lua_Safe_Call_Void();
	self->RemoveItemByID(item_id);
}

void Lua_Corpse::RemoveItemByID(uint32 item_id, int quantity) {
	Lua_Safe_Call_Void();
	self->RemoveItemByID(item_id, quantity);
}

Lua_Corpse_Loot_List Lua_Corpse::GetLootList(lua_State* L) {
	Lua_Safe_Call_Class(Lua_Corpse_Loot_List);
	Lua_Corpse_Loot_List ret;
	auto loot_list = self->GetLootList();

	for (auto item_id : loot_list) {
		ret.entries.push_back(item_id);
	}

	return ret;
}

luabind::scope lua_register_corpse() {
	return luabind::class_<Lua_Corpse, Lua_Mob>("Corpse")
	.def(luabind::constructor<>())
	.property("null", &Lua_Corpse::Null)
	.property("valid", &Lua_Corpse::Valid)
	.def("AddItem", (void(Lua_Corpse::*)(uint32, uint16))&Lua_Corpse::AddItem)
	.def("AddItem", (void(Lua_Corpse::*)(uint32, uint16, int16))&Lua_Corpse::AddItem)
	.def("AddItem", (void(Lua_Corpse::*)(uint32, uint16, int16, uint32, uint32, uint32, uint32, uint32))&Lua_Corpse::AddItem)
	.def("AddLooter", (void(Lua_Corpse::*)(Lua_Mob))&Lua_Corpse::AddLooter)
	.def("AllowMobLoot", (void(Lua_Corpse::*)(Lua_Mob, uint8))&Lua_Corpse::AllowMobLoot)
	.def("Bury", (void(Lua_Corpse::*)(void))&Lua_Corpse::Bury)
	.def("CanMobLoot", (bool(Lua_Corpse::*)(int))&Lua_Corpse::CanMobLoot)
	.def("CountItem", (uint32(Lua_Corpse::*)(uint32))&Lua_Corpse::CountItem)
	.def("CountItems", (uint32(Lua_Corpse::*)(void))&Lua_Corpse::CountItems)
	.def("Delete", (void(Lua_Corpse::*)(void))&Lua_Corpse::Delete)
	.def("Depop", (void(Lua_Corpse::*)(void))&Lua_Corpse::Depop)
	.def("GetCharID", (uint32(Lua_Corpse::*)(void))&Lua_Corpse::GetCharID)
	.def("GetCopper", (uint32(Lua_Corpse::*)(void))&Lua_Corpse::GetCopper)
	.def("GetDBID", (uint32(Lua_Corpse::*)(void))&Lua_Corpse::GetDBID)
	.def("GetDecayTime", (uint32(Lua_Corpse::*)(void))&Lua_Corpse::GetDecayTime)
	.def("GetFirstSlotByItemID", (uint16(Lua_Corpse::*)(uint32))&Lua_Corpse::GetFirstLootSlotByItemID)
	.def("GetGold", (uint32(Lua_Corpse::*)(void))&Lua_Corpse::GetGold)
	.def("GetItemIDBySlot", (uint32(Lua_Corpse::*)(uint16))&Lua_Corpse::GetItemIDBySlot)
	.def("GetLootList", (Lua_Corpse_Loot_List(Lua_Corpse::*)(lua_State* L))&Lua_Corpse::GetLootList)
	.def("GetOwnerName", (const char *(Lua_Corpse::*)(void))&Lua_Corpse::GetOwnerName)
	.def("GetPlatinum", (uint32(Lua_Corpse::*)(void))&Lua_Corpse::GetPlatinum)
	.def("GetSilver", (uint32(Lua_Corpse::*)(void))&Lua_Corpse::GetSilver)
	.def("GetWornItem", (uint32(Lua_Corpse::*)(int16))&Lua_Corpse::GetWornItem)
	.def("HasItem", (bool(Lua_Corpse::*)(uint32))&Lua_Corpse::HasItem)
	.def("IsEmpty", (bool(Lua_Corpse::*)(void))&Lua_Corpse::IsEmpty)
	.def("IsLocked", (bool(Lua_Corpse::*)(void))&Lua_Corpse::IsLocked)
	.def("IsRezzed", (bool(Lua_Corpse::*)(void))&Lua_Corpse::IsRezzed)
	.def("Lock", (void(Lua_Corpse::*)(void))&Lua_Corpse::Lock)
	.def("RemoveCash", (void(Lua_Corpse::*)(void))&Lua_Corpse::RemoveLootCash)
	.def("RemoveItem", (void(Lua_Corpse::*)(uint16))&Lua_Corpse::RemoveItem)
	.def("RemoveItemByID", (void(Lua_Corpse::*)(uint32))&Lua_Corpse::RemoveItemByID)
	.def("RemoveItemByID", (void(Lua_Corpse::*)(uint32,int))&Lua_Corpse::RemoveItemByID)
	.def("ResetDecayTimer", &Lua_Corpse::ResetDecayTimer)
	.def("ResetLooter", (void(Lua_Corpse::*)(void))&Lua_Corpse::ResetLooter)
	.def("Save", (bool(Lua_Corpse::*)(void))&Lua_Corpse::Save)
	.def("SetCash", (void(Lua_Corpse::*)(uint32, uint32, uint32, uint32))&Lua_Corpse::SetCash)
	.def("SetDecayTimer", (void(Lua_Corpse::*)(uint32))&Lua_Corpse::SetDecayTimer)
	.def("Summon", (bool(Lua_Corpse::*)(Lua_Client, bool, bool))&Lua_Corpse::Summon)
	.def("UnLock", (void(Lua_Corpse::*)(void))&Lua_Corpse::UnLock);

}

luabind::scope lua_register_corpse_loot_list() {
	return luabind::class_<Lua_Corpse_Loot_List>("CorpseLootList")
	.def_readwrite("entries", &Lua_Corpse_Loot_List::entries, luabind::return_stl_iterator);
}

#endif // LUA_EQEMU
