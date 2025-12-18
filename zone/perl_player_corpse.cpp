#include "common/features.h"

#ifdef EMBPERL_XS_CLASSES

#include "zone/corpse.h"
#include "zone/embperl.h"

uint32_t Perl_Corpse_GetCharID(Corpse* self) // @categories Account and Character, Corpse
{
	return self->GetCharID();
}

uint32_t Perl_Corpse_GetDecayTime(Corpse* self) // @categories Script Utility, Corpse
{
	return self->GetDecayTime();
}

void Perl_Corpse_Lock(Corpse* self) // @categories Corpse
{
	self->Lock();
}

void Perl_Corpse_UnLock(Corpse* self) // @categories Corpse
{
	self->UnLock();
}

bool Perl_Corpse_IsLocked(Corpse* self) // @categories Corpse
{
	return self->IsLocked();
}

void Perl_Corpse_ResetLooter(Corpse* self) // @categories Corpse
{
	self->ResetLooter();
}

uint32_t Perl_Corpse_GetDBID(Corpse* self) // @categories Script Utility, Corpse
{
	return self->GetCorpseDBID();
}

std::string Perl_Corpse_GetOwnerName(Corpse* self) // @categories Account and Character, Corpse
{
	return self->GetOwnerName();
}

void Perl_Corpse_ResetDecayTimer(Corpse* self) // @categories Corpse
{
	self->ResetDecayTimer();
}

void Perl_Corpse_SetDecayTimer(Corpse* self, uint32_t decay_time) // @categories Corpse
{
	self->SetDecayTimer(decay_time);
}

bool Perl_Corpse_IsEmpty(Corpse* self) // @categories Inventory and Items, Corpse
{
	return self->IsEmpty();
}

void Perl_Corpse_AddItem(Corpse* self, uint32 item_id, uint16 charges) // @categories Inventory and Items, Corpse
{
	self->AddItem(item_id, charges);
}

void Perl_Corpse_AddItem(Corpse* self, uint32 item_id, uint16 charges, uint16 slot) // @categories Inventory and Items, Corpse
{
	self->AddItem(item_id, charges, slot);
}

uint32_t Perl_Corpse_GetWornItem(Corpse* self, uint16_t equip_slot) // @categories Inventory and Items, Corpse
{
	return self->GetWornItem(equip_slot);
}

void Perl_Corpse_RemoveItem(Corpse* self, uint16_t loot_slot) // @categories Inventory and Items, Corpse
{
	self->RemoveItem(loot_slot);
}

void Perl_Corpse_SetCash(Corpse* self, uint16 copper, uint16 silver, uint16 gold, uint16 platinum) // @categories Currency and Points, Corpse
{
	self->SetCash(copper, silver, gold, platinum);
}

void Perl_Corpse_RemoveLootCash(Corpse* self) // @categories Currency and Points, Corpse
{
	self->RemoveCash();
}

uint32_t Perl_Corpse_CountItems(Corpse* self) // @categories Inventory and Items, Corpse
{
	return self->CountItems();
}

void Perl_Corpse_Delete(Corpse* self) // @categories Corpse
{
	self->Delete();
}

uint32_t Perl_Corpse_GetCopper(Corpse* self) // @categories Currency and Points, Corpse
{
	return self->GetCopper();
}

uint32_t Perl_Corpse_GetSilver(Corpse* self) // @categories Currency and Points, Corpse
{
	return self->GetSilver();
}

uint32_t Perl_Corpse_GetGold(Corpse* self)// @categories Currency and Points, Corpse
{
	return self->GetGold();
}

uint32_t Perl_Corpse_GetPlatinum(Corpse* self) // @categories Currency and Points, Corpse
{
	return self->GetPlatinum();
}

void Perl_Corpse_Summon(Corpse* self, Client* client, bool is_spell) // @categories Corpse
{
	self->Summon(client, is_spell, true);
}

void Perl_Corpse_CastRezz(Corpse* self, uint16_t spell_id, Mob* caster) // @categories Spells and Disciplines, Corpse
{
	self->CastRezz(spell_id, caster);
}

void Perl_Corpse_CompleteRezz(Corpse* self) // @categories Spells and Disciplines, Corpse
{
	self->CompleteResurrection();
}

bool Perl_Corpse_CanMobLoot(Corpse* self, int character_id) // @categories Script Utility, Corpse
{
	return self->CanPlayerLoot(character_id);
}

void Perl_Corpse_AllowMobLoot(Corpse* self, Mob* them, uint8_t slot) // @categories Account and Character, Corpse
{
	self->AllowPlayerLoot(them, slot);
}

void Perl_Corpse_AddLooter(Corpse* self, Mob* who) // @categories Account and Character, Corpse
{
	self->AddLooter(who);
}

bool Perl_Corpse_IsRezzed(Corpse* self) // @categories Corpse
{
	return self->IsRezzed();
}

bool Perl_Corpse_HasItem(Corpse* self, uint32_t item_id) // @categories Script Utility
{
	return self->HasItem(item_id);
}

uint32 Perl_Corpse_CountItem(Corpse* self, uint32_t item_id) // @categories Script Utility
{
	return self->CountItem(item_id);
}

uint32_t Perl_Corpse_GetLootItemIDBySlot(Corpse* self, uint16_t loot_slot) // @categories Script Utility
{
	return self->GetItemIDBySlot(loot_slot);
}

int Perl_Corpse_GetFirstLootSlotByItemID(Corpse* self, uint32_t item_id) // @categories Script Utility
{
	return self->GetFirstLootSlotByItemID(item_id);
}

void Perl_Corpse_RemoveItemByID(Corpse* self, uint32_t item_id) // @categories Script Utility
{
	self->RemoveItemByID(item_id);
}

void Perl_Corpse_RemoveItemByID(Corpse* self, uint32_t item_id, int quantity) // @categories Script Utility
{
	self->RemoveItemByID(item_id);
}

perl::array Perl_Corpse_GetLootList(Corpse* self) // @categories Script Utility
{
	perl::array result;

	auto corpse_items = self->GetLootList();
	for (int i = 0; i < corpse_items.size(); ++i)
	{
		result.push_back(corpse_items[i]);
	}

	return result;
}

void perl_register_corpse()
{
	perl::interpreter perl(PERL_GET_THX);

	auto package = perl.new_class<Corpse>("Corpse");
	package.add_base_class("Mob");
	package.add("AddItem", (void(*)(Corpse*, uint32, uint16))&Perl_Corpse_AddItem);
	package.add("AddItem", (void(*)(Corpse*, uint32, uint16, uint16))&Perl_Corpse_AddItem);
	package.add("AddLooter", &Perl_Corpse_AddLooter);
	package.add("AllowMobLoot", &Perl_Corpse_AllowMobLoot);
	package.add("CanMobLoot", &Perl_Corpse_CanMobLoot);
	package.add("CastRezz", &Perl_Corpse_CastRezz);
	package.add("CompleteRezz", &Perl_Corpse_CompleteRezz);
	package.add("CountItem", &Perl_Corpse_CountItem);
	package.add("CountItems", &Perl_Corpse_CountItems);
	package.add("Delete", &Perl_Corpse_Delete);
	package.add("GetCharID", &Perl_Corpse_GetCharID);
	package.add("GetCopper", &Perl_Corpse_GetCopper);
	package.add("GetDBID", &Perl_Corpse_GetDBID);
	package.add("GetDecayTime", &Perl_Corpse_GetDecayTime);
	package.add("GetFirstSlotByItemID", &Perl_Corpse_GetFirstLootSlotByItemID);
	package.add("GetGold", &Perl_Corpse_GetGold);
	package.add("GetItemIDBySlot", &Perl_Corpse_GetLootItemIDBySlot);
	package.add("GetLootList", &Perl_Corpse_GetLootList);
	package.add("GetOwnerName", &Perl_Corpse_GetOwnerName);
	package.add("GetPlatinum", &Perl_Corpse_GetPlatinum);
	package.add("GetSilver", &Perl_Corpse_GetSilver);
	package.add("GetWornItem", &Perl_Corpse_GetWornItem);
	package.add("HasItem", &Perl_Corpse_HasItem);
	package.add("IsEmpty", &Perl_Corpse_IsEmpty);
	package.add("IsLocked", &Perl_Corpse_IsLocked);
	package.add("IsRezzed", &Perl_Corpse_IsRezzed);
	package.add("Lock", &Perl_Corpse_Lock);
	package.add("RemoveCash", &Perl_Corpse_RemoveLootCash);
	package.add("RemoveItem", &Perl_Corpse_RemoveItem);
	package.add("RemoveItemByID", (void(*)(Corpse*, uint32_t))&Perl_Corpse_RemoveItemByID);
	package.add("RemoveItemByID", (void(*)(Corpse*, uint32_t, int))&Perl_Corpse_RemoveItemByID);
	package.add("ResetDecayTimer", &Perl_Corpse_ResetDecayTimer);
	package.add("ResetLooter", &Perl_Corpse_ResetLooter);
	package.add("SetCash", &Perl_Corpse_SetCash);
	package.add("SetDecayTimer", &Perl_Corpse_SetDecayTimer);
	package.add("Summon", &Perl_Corpse_Summon);
	package.add("UnLock", &Perl_Corpse_UnLock);
}

#endif //EMBPERL_XS_CLASSES

