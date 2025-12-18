#include "common/features.h"

#ifdef EMBPERL_XS_CLASSES

#include "zone/embperl.h"
#include "zone/npc.h"

void Perl_NPC_SignalNPC(NPC* self, int signal_id) // @categories Script Utility
{
	self->SignalNPC(signal_id);
}

int Perl_NPC_CheckNPCFactionAlly(NPC* self, int32 faction_id) // @categories Faction
{
	return self->CheckNPCFactionAlly(faction_id);
}

void Perl_NPC_AddItem(NPC* self, uint32 item_id) // @categories Inventory and Items
{
	self->AddItem(item_id, 0);
}

void Perl_NPC_AddItem(NPC* self, uint32 item_id, uint16 charges) // @categories Inventory and Items
{
	self->AddItem(item_id, charges);
}

void Perl_NPC_AddItem(NPC* self, uint32 item_id, uint16 charges, bool equip_item) // @categories Inventory and Items
{
	self->AddItem(item_id, charges, equip_item);
}

void Perl_NPC_AddItem(NPC* self, uint32 item_id, uint16 charges, bool equip_item, uint32 aug1) // @categories Inventory and Items
{
	self->AddItem(item_id, charges, equip_item, aug1);
}

void Perl_NPC_AddItem(NPC* self, uint32 item_id, uint16 charges, bool equip_item, uint32 aug1, uint32 aug2) // @categories Inventory and Items
{
	self->AddItem(item_id, charges, equip_item, aug1, aug2);
}

void Perl_NPC_AddItem(NPC* self, uint32 item_id, uint16 charges, bool equip_item, uint32 aug1, uint32 aug2, uint32 aug3) // @categories Inventory and Items
{
	self->AddItem(item_id, charges, equip_item, aug1, aug2, aug3);
}

void Perl_NPC_AddItem(NPC* self, uint32 item_id, uint16 charges, bool equip_item, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4) // @categories Inventory and Items
{
	self->AddItem(item_id, charges, equip_item, aug1, aug2, aug3, aug4);
}

void Perl_NPC_AddItem(NPC* self, uint32 item_id, uint16 charges, bool equip_item, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5) // @categories Inventory and Items
{
	self->AddItem(item_id, charges, equip_item, aug1, aug2, aug3, aug4, aug5);
}

void Perl_NPC_AddItem(NPC* self, uint32 item_id, uint16 charges, bool equip_item, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5, uint32 aug6) // @categories Inventory and Items
{
	self->AddItem(item_id, charges, equip_item, aug1, aug2, aug3, aug4, aug5, aug6);
}

void Perl_NPC_AddLootTable(NPC* self) // @categories Inventory and Items
{
	self->AddLootTable();
}

void Perl_NPC_AddLootTable(NPC* self, uint32 loottable_id) // @categories Inventory and Items
{
	self->AddLootTable(loottable_id);
}

void Perl_NPC_RemoveItem(NPC* self, uint32 item_id) // @categories Inventory and Items
{
	self->RemoveItem(item_id);
}

void Perl_NPC_RemoveItem(NPC* self, uint32 item_id, uint16 quantity) // @categories Inventory and Items
{
	self->RemoveItem(item_id, quantity);
}

void Perl_NPC_RemoveItem(NPC* self, uint32 item_id, uint16 quantity, uint16 slot_id) // @categories Inventory and Items
{
	self->RemoveItem(item_id, quantity, slot_id);
}

void Perl_NPC_ClearLootItems(NPC* self) // @categories Inventory and Items
{
	self->ClearLootItems();
}

void Perl_NPC_AddLootCash(NPC* self, uint32 copper, uint32 silver, uint32 gold, uint32 platinum) // @categories Currency and Points
{
	self->AddLootCash(copper, silver, gold, platinum);
}

void Perl_NPC_RemoveLootCash(NPC* self) // @categories Currency and Points
{
	self->RemoveLootCash();
}

uint32_t Perl_NPC_CountLoot(NPC* self) // @categories Inventory and Items
{
	return self->CountLoot();
}

uint32_t Perl_NPC_GetLoottableID(NPC* self) // @categories Stats and Attributes
{
	return self->GetLoottableID();
}

uint32_t Perl_NPC_GetCopper(NPC* self) // @categories Currency and Points
{
	return self->GetCopper();
}

uint32_t Perl_NPC_GetSilver(NPC* self) // @categories Currency and Points
{
	return self->GetSilver();
}

uint32_t Perl_NPC_GetGold(NPC* self) // @categories Currency and Points
{
	return self->GetGold();
}

uint32_t Perl_NPC_GetPlatinum(NPC* self) // @categories Currency and Points
{
	return self->GetPlatinum();
}

void Perl_NPC_SetCopper(NPC* self, uint32 amt) // @categories Currency and Points
{
	self->SetCopper(amt);
}

void Perl_NPC_SetSilver(NPC* self, uint32 amt) // @categories Currency and Points
{
	self->SetSilver(amt);
}

void Perl_NPC_SetGold(NPC* self, uint32 amt) // @categories Currency and Points
{
	self->SetGold(amt);
}

void Perl_NPC_SetPlatinum(NPC* self, uint32 amt) // @categories Currency and Points
{
	self->SetPlatinum(amt);
}

void Perl_NPC_SetGrid(NPC* self, int grid) // @categories Script Utility
{
	self->SetGrid(grid);
}

void Perl_NPC_SetSaveWaypoint(NPC* self, uint16 wp) // @categories Script Utility
{
	self->SetSaveWaypoint(wp);
}

void Perl_NPC_SetSp2(NPC* self, uint32 set_spawn_group_id) // @categories Spawns
{
	self->SetSpawnGroupId(set_spawn_group_id);
}

int Perl_NPC_GetWaypointMax(NPC* self) // @categories Script Utility
{
	return self->GetWaypointMax();
}

int Perl_NPC_GetGrid(NPC* self) // @categories Script Utility, Spawns
{
	return self->GetGrid();
}

uint32_t Perl_NPC_GetSp2(NPC* self) // @categories Spawns
{
	return self->GetSpawnGroupId();
}

int Perl_NPC_GetNPCFactionID(NPC* self) // @categories Faction, Stats and Attributes
{
	return self->GetNPCFactionID();
}

int Perl_NPC_GetPrimaryFaction(NPC* self) // @categories Faction, Stats and Attributes
{
	return self->GetPrimaryFaction();
}

int64_t Perl_NPC_GetNPCHate(NPC* self, Mob* mob) // @categories Hate and Aggro
{
	return self->GetNPCHate(mob);
}

bool Perl_NPC_IsOnHatelist(NPC* self, Mob* mob) // @categories Hate and Aggro
{
	return self->IsOnHatelist(mob);
}

void Perl_NPC_RemoveFromHateList(NPC* self, Mob* mob) // @categories Hate and Aggro
{
	self->RemoveFromHateList(mob);
	self->RemoveFromRampageList(mob);
}

void Perl_NPC_SetNPCFactionID(NPC* self, int faction_id) // @categories Faction
{
	self->SetNPCFactionID(faction_id);
}

uint32_t Perl_NPC_GetMaxDMG(NPC* self) // @categories Stats and Attributes
{
	return self->GetMaxDMG();
}

uint32_t Perl_NPC_GetMinDMG(NPC* self)
{
	return self->GetMinDMG();
}

bool Perl_NPC_IsAnimal(NPC* self) // @categories Stats and Attributes
{
	return self->IsAnimal();
}

int Perl_NPC_GetPetSpellID(NPC* self) // @categories Stats and Attributes, Pet
{
	return self->GetPetSpellID();
}

void Perl_NPC_SetPetSpellID(NPC* self, uint16 amount) // @categories Pet
{
	self->SetPetSpellID(amount);
}

uint32_t Perl_NPC_GetMaxDamage(NPC* self, uint8 target_level) // @categories Stats and Attributes
{
	return self->GetMaxDamage(target_level);
}

void Perl_NPC_SetTaunting(NPC* self, bool on) // @categories Script Utility
{
	self->SetTaunting(on);
}

bool Perl_NPC_IsTaunting(NPC* self)
{
	return self->IsTaunting();
}

void Perl_NPC_PickPocket(NPC* self, Client* thief) // @categories Skills and Recipes
{
	self->PickPocket(thief);
}

void Perl_NPC_StartSwarmTimer(NPC* self, uint32 duration) // @categories Script Utility, Pet
{
	self->StartSwarmTimer(duration);
}

void Perl_NPC_DoClassAttacks(NPC* self, Mob* target) // @categories Script Utility
{
	self->DoClassAttacks(target);
}

int Perl_NPC_GetMaxWp(NPC* self)
{
	return self->GetMaxWp();
}

void Perl_NPC_DisplayWaypointInfo(NPC* self, Client* client) // @categories Script Utility
{
	self->DisplayWaypointInfo(client);
}

void Perl_NPC_CalculateNewWaypoint(NPC* self) // @categories Script Utility
{
	self->CalculateNewWaypoint();
}

void Perl_NPC_AssignWaypoints(NPC* self, uint32 grid_id) // @categories Script Utility
{
	self->AssignWaypoints(grid_id);
}

void Perl_NPC_SetWaypointPause(NPC* self) // @categories Script Utility
{
	self->SetWaypointPause();
}

void Perl_NPC_UpdateWaypoint(NPC* self, int wp_index) // @categories Script Utility
{
	self->UpdateWaypoint(wp_index);
}

void Perl_NPC_StopWandering(NPC* self) // @categories Script Utility
{
	self->StopWandering();
}

void Perl_NPC_ResumeWandering(NPC* self) // @categories Script Utility
{
	self->ResumeWandering();
}

void Perl_NPC_PauseWandering(NPC* self, int pause_time) // @categories Script Utility
{
	self->PauseWandering(pause_time);
}

void Perl_NPC_MoveTo(NPC* self, float x, float y, float z) // @categories Script Utility
{
	auto position = glm::vec4(x, y, z, 0);
	self->MoveTo(position, false);
}

void Perl_NPC_MoveTo(NPC* self, float x, float y, float z, float h) // @categories Script Utility
{
	auto position = glm::vec4(x, y, z, h);
	self->MoveTo(position, false);
}

void Perl_NPC_MoveTo(NPC* self, float x, float y, float z, float h, bool save) // @categories Script Utility
{
	auto position = glm::vec4(x, y, z, h);
	self->MoveTo(position, save);
}

void Perl_NPC_NextGuardPosition(NPC* self) // @categories Script Utility
{
	self->NextGuardPosition();
}

void Perl_NPC_SaveGuardSpot(NPC* self) // @categories Script Utility
{
	self->SaveGuardSpot();
}

void Perl_NPC_SaveGuardSpot(NPC* self, bool clear) // @categories Script Utility
{
	self->SaveGuardSpot(clear);
}

void Perl_NPC_SaveGuardSpot(NPC* self, float x, float y, float z, float heading) // @categories Script Utility
{
	self->SaveGuardSpot(glm::vec4(x, y, z, heading));
}

bool Perl_NPC_IsGuarding(NPC* self) // @categories Script Utility
{
	return self->IsGuarding();
}

void Perl_NPC_AI_SetRoambox(NPC* self, float distance, float max_x, float min_x, float max_y, float min_y) // @categories Script Utility
{
	self->AI_SetRoambox(distance, max_x, min_x, max_y, min_y);
}

void Perl_NPC_AI_SetRoambox(NPC* self, float distance, float max_x, float min_x, float max_y, float min_y, uint32 max_delay) // @categories Script Utility
{
	self->AI_SetRoambox(distance, max_x, min_x, max_y, min_y, max_delay);
}

void Perl_NPC_AI_SetRoambox(NPC* self, float distance, float max_x, float min_x, float max_y, float min_y, uint32 max_delay, uint32 min_delay) // @categories Script Utility
{
	self->AI_SetRoambox(distance, max_x, min_x, max_y, min_y, max_delay, min_delay);
}

uint32_t Perl_NPC_GetNPCSpellsID(NPC* self) // @categories Stats and Attributes
{
	return self->GetNPCSpellsID();
}

uint32_t Perl_NPC_GetSpawnPointID(NPC* self) // @categories Spawns
{
	return self->GetSpawnPointID();
}

float Perl_NPC_GetSpawnPointX(NPC* self) // @categories Spawns
{
	return self->GetSpawnPoint().x;
}

float Perl_NPC_GetSpawnPointY(NPC* self) // @categories Spawns
{
	return self->GetSpawnPoint().y;
}

float Perl_NPC_GetSpawnPointZ(NPC* self) // @categories Spawns
{
	return self->GetSpawnPoint().z;
}

float Perl_NPC_GetSpawnPointH(NPC* self) // @categories Spawns
{
	return self->GetSpawnPoint().w;
}

float Perl_NPC_GetGuardPointX(NPC* self) // @categories Script Utility, Spawns
{
	return self->GetGuardPoint().x;
}

float Perl_NPC_GetGuardPointY(NPC* self) // @categories Script Utility, Spawns
{
	return self->GetGuardPoint().y;
}

float Perl_NPC_GetGuardPointZ(NPC* self) // @categories Script Utility, Spawns
{
	return self->GetGuardPoint().z;
}

void Perl_NPC_SetPrimSkill(NPC* self, int skill_id) // @categories Stats and Attributes
{
	self->SetPrimSkill(skill_id);
}

void Perl_NPC_SetSecSkill(NPC* self, int skill_id) // @categories Stats and Attributes
{
	self->SetSecSkill(skill_id);
}

uint32_t Perl_NPC_GetPrimSkill(NPC* self) // @categories Stats and Attributes
{
	return self->GetPrimSkill();
}

uint32_t Perl_NPC_GetSecSkill(NPC* self) // @categories Stats and Attributes
{
	return self->GetSecSkill();
}

uint32_t Perl_NPC_GetSwarmOwner(NPC* self) // @categories Pet
{
	return self->GetSwarmOwner();
}

uint32_t Perl_NPC_GetSwarmTarget(NPC* self) // @categories Pet
{
	return self->GetSwarmTarget();
}

void Perl_NPC_SetSwarmTarget(NPC* self, int target_id) // @categories Pet
{
	self->SetSwarmTarget(target_id);
}

void Perl_NPC_ModifyNPCStat(NPC* self, std::string stat, std::string value) // @categories Stats and Attributes
{
	self->ModifyNPCStat(stat, value);
}

float Perl_NPC_GetNPCStat(NPC* self, std::string stat) // @categories Stats and Attributes
{
	return self->GetNPCStat(stat);
}

void Perl_NPC_AddSpellToNPCList(NPC* self, int16 priority, uint16_t spell_id, uint32 type, int mana_cost, int recast_delay, int16 resist_adjust) // @categories Spells and Disciplines, Script Utility
{
	self->AddSpellToNPCList(priority, spell_id, type, mana_cost, recast_delay, resist_adjust, 0, 0);
}

void Perl_NPC_AddSpellToNPCList(NPC* self, int16 priority, uint16_t spell_id, uint32 type, int mana_cost, int recast_delay, int16 resist_adjust, int8 min_hp, int8 max_hp) // @categories Spells and Disciplines, Script Utility
{
	self->AddSpellToNPCList(priority, spell_id, type, mana_cost, recast_delay, resist_adjust, min_hp, max_hp);
}

void Perl_NPC_RemoveSpellFromNPCList(NPC* self, uint16_t spell_id) // @categories Spells and Disciplines
{
	self->RemoveSpellFromNPCList(spell_id);
}

void Perl_NPC_SetSpellFocusDMG(NPC* self, int new_spell_focus_dmg) // @categories Stats and Attributes
{
	self->SetSpellFocusDMG(new_spell_focus_dmg);
}

int Perl_NPC_GetSpellFocusDMG(NPC* self) // @categories Spells and Disciplines
{
	return self->GetSpellFocusDMG();
}

void Perl_NPC_SetSpellFocusHeal(NPC* self, int new_spell_focus_heal) // @categories Stats and Attributes
{
	self->SetSpellFocusHeal(new_spell_focus_heal);
}

int Perl_NPC_GetSpellFocusHeal(NPC* self) // @categories Spells and Disciplines
{
	return self->GetSpellFocusHeal();
}

float Perl_NPC_GetSlowMitigation(NPC* self) // @categories Stats and Attributes
{
	return self->GetSlowMitigation();
}

float Perl_NPC_GetAttackSpeed(NPC* self) // @categories Stats and Attributes
{
	return self->GetAttackSpeed();
}

int Perl_NPC_GetAttackDelay(NPC* self) // @categories Stats and Attributes
{
	return self->GetAttackDelay();
}

int Perl_NPC_GetAccuracyRating(NPC* self) // @categories Stats and Attributes
{
	return self->GetAccuracyRating();
}

int Perl_NPC_GetAvoidanceRating(NPC* self) // @categories Stats and Attributes
{
	return self->GetAvoidanceRating();
}

uint32_t Perl_NPC_GetSpawnKillCount(NPC* self) // @categories Spawns
{
	return self->GetSpawnKillCount();
}

int Perl_NPC_GetScore(NPC* self) // @categories Script Utility
{
	return self->GetScore();
}

void Perl_NPC_MerchantOpenShop(NPC* self) // @categories Script Utility
{
	self->MerchantOpenShop();
}

void Perl_NPC_MerchantCloseShop(NPC* self) // @categories Script Utility
{
	self->MerchantCloseShop();
}

void Perl_NPC_AddMeleeProc(NPC* self, uint16_t spell_id, uint16_t chance) // @categories Script Utility
{
	self->AddProcToWeapon(spell_id, true, chance);
}

void Perl_NPC_AddRangedProc(NPC* self, uint16_t spell_id, uint16_t chance) // @categories Script Utility
{
	self->AddRangedProc(spell_id, chance);
}

void Perl_NPC_AddDefensiveProc(NPC* self, uint16_t spell_id, uint16_t chance) // @categories Script Utility
{
	self->AddDefensiveProc(spell_id, chance);
}

void Perl_NPC_RemoveMeleeProc(NPC* self, uint16_t spell_id) // @categories Script Utility
{
	self->RemoveProcFromWeapon(spell_id, false);
}

void Perl_NPC_RemoveRangedProc(NPC* self, uint16_t spell_id) // @categories Script Utility
{
	self->RemoveRangedProc(spell_id, false);
}

void Perl_NPC_RemoveDefensiveProc(NPC* self, uint16_t spell_id) // @categories Script Utility
{
	self->RemoveDefensiveProc(spell_id, false);
}

void Perl_NPC_ChangeLastName(NPC* self, std::string name) // @categories Script Utility
{
	self->ChangeLastName(name);
}

void Perl_NPC_ClearLastName(NPC* self) // @categories Script Utility
{
	self->ClearLastName();
}

bool Perl_NPC_GetCombatState(NPC* self) // @categories Script Utility
{
	return self->GetCombatEvent();
}

void Perl_NPC_SetSimpleRoamBox(NPC* self, float box_size) // @categories Script Utility
{
	self->SetSimpleRoamBox(box_size);
}

void Perl_NPC_SetSimpleRoamBox(NPC* self, float box_size, float move_distance) // @categories Script Utility
{
	self->SetSimpleRoamBox(box_size, move_distance);
}

void Perl_NPC_SetSimpleRoamBox(NPC* self, float box_size, float move_distance, int move_delay) // @categories Script Utility
{
	self->SetSimpleRoamBox(box_size, move_distance, move_delay);
}

void Perl_NPC_RecalculateSkills(NPC* self) // @categories Skills and Recipes
{
	self->RecalculateSkills();
}

bool Perl_NPC_IsRaidTarget(NPC* self)
{
	return self->IsRaidTarget();
}

bool Perl_NPC_HasItem(NPC* self, uint32 item_id) // @categories Script Utility
{
	return self->HasItem(item_id);
}

uint32 Perl_NPC_CountItem(NPC* self, uint32 item_id)
{
	return self->CountItem(item_id);
}

uint32_t Perl_NPC_GetLootItemIDBySlot(NPC* self, uint16 loot_slot)
{
	return self->GetLootItemIDBySlot(loot_slot);
}

int Perl_NPC_GetFirstLootSlotByItemID(NPC* self, uint32 item_id)
{
	return self->GetFirstLootSlotByItemID(item_id);
}

float Perl_NPC_GetHealScale(NPC* self) // @categories Stats and Attributes
{
	return self->GetHealScale();
}

float Perl_NPC_GetSpellScale(NPC* self) // @categories Stats and Attributes
{
	return self->GetSpellScale();
}

perl::array Perl_NPC_GetLootList(NPC* self) // @categories Script Utility
{
	perl::array result;
	auto npc_items = self->GetLootList();
	for (int item_id : npc_items)
	{
		result.push_back(item_id);
	}
	return result;
}

void Perl_NPC_AddAISpellEffect(NPC* self, int spell_effect_id, int base_value, int limit_value, int max_value) // @categories Spells and Disciplines
{
	self->AddSpellEffectToNPCList(spell_effect_id, base_value, limit_value, max_value, true);
}

void Perl_NPC_RemoveAISpellEffect(NPC* self, int spell_effect_id) // @categories Spells and Disciplines
{
	self->RemoveSpellEffectFromNPCList(spell_effect_id, true);
}

bool Perl_NPC_HasAISpellEffect(NPC* self, int spell_effect_id) // @categories Spells and Disciplines
{
	return self->HasAISpellEffect(spell_effect_id);
}

bool Perl_NPC_IsRareSpawn(NPC* self)
{
	return self->IsRareSpawn();
}

void Perl_NPC_ReloadSpells(NPC* self)
{
	self->ReloadSpells();
}

void Perl_NPC_SendPayload(NPC* self, int payload_id) // @categories Script Utility
{
	self->SendPayload(payload_id);
}

void Perl_NPC_SendPayload(NPC* self, int payload_id, std::string payload_value) // @categories Script Utility
{
	self->SendPayload(payload_id, payload_value);
}

bool Perl_NPC_GetKeepsSoldItems(NPC* self)
{
	return self->GetKeepsSoldItems();
}

void Perl_NPC_SetKeepsSoldItems(NPC* self, bool keeps_sold_items)
{
	self->SetKeepsSoldItems(keeps_sold_items);
}

bool Perl_NPC_IsLDoNTrapped(NPC* self)
{
	return self->IsLDoNTrapped();
}

void Perl_NPC_SetLDoNTrapped(NPC* self, bool is_trapped)
{
	self->SetLDoNTrapped(is_trapped);
}

uint8 Perl_NPC_GetLDoNTrapType(NPC* self)
{
	return self->GetLDoNTrapType();
}

void Perl_NPC_SetLDoNTrapType(NPC* self, uint8 trap_type)
{
	self->SetLDoNTrapType(trap_type);
}

uint16 Perl_NPC_GetLDoNTrapSpellID(NPC* self)
{
	return self->GetLDoNTrapSpellID();
}

void Perl_NPC_SetLDoNTrapSpellID(NPC* self, uint16 spell_id)
{
	self->SetLDoNTrapSpellID(spell_id);
}

bool Perl_NPC_IsLDoNLocked(NPC* self)
{
	return self->IsLDoNLocked();
}

void Perl_NPC_SetLDoNLocked(NPC* self, bool is_locked)
{
	self->SetLDoNLocked(is_locked);
}

uint16 Perl_NPC_GetLDoNLockedSkill(NPC* self)
{
	return self->GetLDoNLockedSkill();
}

void Perl_NPC_SetLDoNLockedSkill(NPC* self, uint16 skill_value)
{
	self->SetLDoNLockedSkill(skill_value);
}

bool Perl_NPC_IsLDoNTrapDetected(NPC* self)
{
	return self->IsLDoNTrapDetected();
}

void Perl_NPC_SetLDoNTrapDetected(NPC* self, bool is_detected)
{
	self->SetLDoNTrapDetected(is_detected);
}

void Perl_NPC_ScaleNPC(NPC* self, uint8 npc_level)
{
	self->ScaleNPC(npc_level, true);
}

void Perl_NPC_ScaleNPC(NPC* self, uint8 npc_level, bool override_special_abilities)
{
	self->ScaleNPC(npc_level, true, override_special_abilities);
}

bool Perl_NPC_IsUnderwaterOnly(NPC* self) // @categories Script Utility
{
	return self->IsUnderwaterOnly();
}

bool Perl_NPC_HasSpecialAbilities(NPC* self) // @categories Script Utility
{
	return self->HasSpecialAbilities();
}

bool Perl_NPC_GetNPCAggro(NPC* self) // @categories Script Utility
{
	return self->GetNPCAggro();
}

void Perl_NPC_SetNPCAggro(NPC* self, bool in_npc_aggro) // @categories Script Utility
{
	self->SetNPCAggro(in_npc_aggro);
}

uint32 Perl_NPC_GetNPCSpellsEffectsID(NPC* self)
{
	return self->GetNPCSpellsEffectsID();
}

void Perl_NPC_DescribeSpecialAbilities(NPC* self, Client* c)
{
	self->DescribeSpecialAbilities(c);
}

bool Perl_NPC_IsMultiQuestEnabled(NPC* self)
{
	return self->IsMultiQuestEnabled();
}

void Perl_NPC_MultiQuestEnable(NPC* self)
{
	self->MultiQuestEnable();
}

bool Perl_NPC_IsResumedFromZoneSuspend(NPC* self)
{
	return self->IsResumedFromZoneSuspend();
}

bool Perl_NPC_CheckHandin(
	NPC* self,
	Client* c,
	perl::reference handin_ref,
	perl::reference required_ref,
	perl::array items_ref
)
{
	perl::hash handin   = handin_ref;
	perl::hash required = required_ref;

	std::map<std::string, uint32>   handin_map;
	std::map<std::string, uint32>   required_map;
	std::vector<EQ::ItemInstance *> items;

	for (auto e: handin) {
		if (!e.first) {
			continue;
		}

		if (Strings::EqualFold(e.first, "0")) {
			continue;
		}

		LogNpcHandinDetail("Handin key [{}] value [{}]", e.first, handin.at(e.first).c_str());

		const uint32 count = static_cast<uint32>(handin.at(e.first));
		handin_map[e.first] = count;
	}

	for (auto e: required) {
		if (!e.first) {
			continue;
		}

		if (Strings::EqualFold(e.first, "0")) {
			continue;
		}

		LogNpcHandinDetail("Required key [{}] value [{}]", e.first, required.at(e.first).c_str());

		const uint32 count = static_cast<uint32>(required.at(e.first));
		required_map[e.first] = count;
	}

	for (auto e : items_ref) {
		EQ::ItemInstance* i = static_cast<EQ::ItemInstance*>(e);
		if (!i) {
			continue;
		}

		items.emplace_back(i);

		LogNpcHandinDetail(
			"Item instance [{}] ({}) UUID ({}) added to handin list",
			i->GetItem()->Name,
			i->GetItem()->ID,
			i->GetSerialNumber()
		);
	}

	return self->CheckHandin(c, handin_map, required_map, items);
}

void Perl_NPC_ReturnHandinItems(NPC *self, Client* c)
{
	self->ReturnHandinItems(c);
}

Spawn2* Perl_NPC_GetSpawn(NPC* self)
{
	return self->GetSpawn();
}

void Perl_NPC_SetNPCTintIndex(NPC* self, uint32 index)
{
	self->SetNPCTintIndex(index);
}

uint32 Perl_NPC_GetNPCTintIndex(NPC* self)
{
	return self->GetNPCTintIndex();
}

void perl_register_npc()
{
	perl::interpreter perl(PERL_GET_THX);

	auto package = perl.new_class<NPC>("NPC");
	package.add_base_class("Mob");
	package.add("AI_SetRoambox", (void(*)(NPC*, float, float, float, float, float))&Perl_NPC_AI_SetRoambox);
	package.add("AI_SetRoambox", (void(*)(NPC*, float, float, float, float, float, uint32))&Perl_NPC_AI_SetRoambox);
	package.add("AI_SetRoambox", (void(*)(NPC*, float, float, float, float, float, uint32, uint32))&Perl_NPC_AI_SetRoambox);
	package.add("AddAISpell", (void(*)(NPC*, int16, uint16, uint32, int, int, int16))&Perl_NPC_AddSpellToNPCList);
	package.add("AddAISpell", (void(*)(NPC*, int16, uint16, uint32, int, int, int16, int8, int8))&Perl_NPC_AddSpellToNPCList);
	package.add("AddAISpellEffect", &Perl_NPC_AddAISpellEffect);
	package.add("AddCash", &Perl_NPC_AddLootCash);
	package.add("AddDefensiveProc", &Perl_NPC_AddDefensiveProc);
	package.add("AddItem", (void(*)(NPC*, uint32))&Perl_NPC_AddItem);
	package.add("AddItem", (void(*)(NPC*, uint32, uint16))&Perl_NPC_AddItem);
	package.add("AddItem", (void(*)(NPC*, uint32, uint16, bool))&Perl_NPC_AddItem);
	package.add("AddItem", (void(*)(NPC*, uint32, uint16, bool, uint32))&Perl_NPC_AddItem);
	package.add("AddItem", (void(*)(NPC*, uint32, uint16, bool, uint32, uint32))&Perl_NPC_AddItem);
	package.add("AddItem", (void(*)(NPC*, uint32, uint16, bool, uint32, uint32, uint32))&Perl_NPC_AddItem);
	package.add("AddItem", (void(*)(NPC*, uint32, uint16, bool, uint32, uint32, uint32, uint32))&Perl_NPC_AddItem);
	package.add("AddItem", (void(*)(NPC*, uint32, uint16, bool, uint32, uint32, uint32, uint32, uint32))&Perl_NPC_AddItem);
	package.add("AddItem", (void(*)(NPC*, uint32, uint16, bool, uint32, uint32, uint32, uint32, uint32, uint32))&Perl_NPC_AddItem);
	package.add("AddLootTable", (void(*)(NPC*))&Perl_NPC_AddLootTable);
	package.add("AddLootTable", (void(*)(NPC*, uint32))&Perl_NPC_AddLootTable);
	package.add("AddMeleeProc", &Perl_NPC_AddMeleeProc);
	package.add("AddRangedProc", &Perl_NPC_AddRangedProc);
	package.add("AssignWaypoints", &Perl_NPC_AssignWaypoints);
	package.add("CalculateNewWaypoint", &Perl_NPC_CalculateNewWaypoint);
	package.add("ChangeLastName", &Perl_NPC_ChangeLastName);
	package.add("CheckNPCFactionAlly", &Perl_NPC_CheckNPCFactionAlly);
	package.add("CheckHandin", &Perl_NPC_CheckHandin);
	package.add("ClearItemList", &Perl_NPC_ClearLootItems);
	package.add("ClearLastName", &Perl_NPC_ClearLastName);
	package.add("CountItem", &Perl_NPC_CountItem);
	package.add("CountLoot", &Perl_NPC_CountLoot);
	package.add("DescribeSpecialAbilities", &Perl_NPC_DescribeSpecialAbilities);
	package.add("DisplayWaypointInfo", &Perl_NPC_DisplayWaypointInfo);
	package.add("DoClassAttacks", &Perl_NPC_DoClassAttacks);
	package.add("GetAccuracyRating", &Perl_NPC_GetAccuracyRating);
	package.add("GetAttackDelay", &Perl_NPC_GetAttackDelay);
	package.add("GetAttackSpeed", &Perl_NPC_GetAttackSpeed);
	package.add("GetAvoidanceRating", &Perl_NPC_GetAvoidanceRating);
	package.add("GetCombatState", &Perl_NPC_GetCombatState);
	package.add("GetCopper", &Perl_NPC_GetCopper);
	package.add("GetFirstSlotByItemID", &Perl_NPC_GetFirstLootSlotByItemID);
	package.add("GetGold", &Perl_NPC_GetGold);
	package.add("GetGrid", &Perl_NPC_GetGrid);
	package.add("GetGuardPointX", &Perl_NPC_GetGuardPointX);
	package.add("GetGuardPointY", &Perl_NPC_GetGuardPointY);
	package.add("GetGuardPointZ", &Perl_NPC_GetGuardPointZ);
	package.add("GetHealScale", &Perl_NPC_GetHealScale);
	package.add("GetItemIDBySlot", &Perl_NPC_GetLootItemIDBySlot);
	package.add("GetKeepsSoldItems", &Perl_NPC_GetKeepsSoldItems);
	package.add("GetLDoNLockedSkill", &Perl_NPC_GetLDoNLockedSkill);
	package.add("GetLDoNTrapType", &Perl_NPC_GetLDoNTrapType);
	package.add("GetLDoNTrapSpellID", &Perl_NPC_GetLDoNTrapSpellID);
	package.add("GetLootList", &Perl_NPC_GetLootList);
	package.add("GetLoottableID", &Perl_NPC_GetLoottableID);
	package.add("GetMaxDMG", &Perl_NPC_GetMaxDMG);
	package.add("GetMaxDamage", &Perl_NPC_GetMaxDamage);
	package.add("GetMaxWp", &Perl_NPC_GetMaxWp);
	package.add("GetMinDMG", &Perl_NPC_GetMinDMG);
	package.add("GetNPCAggro", &Perl_NPC_GetNPCAggro);
	package.add("GetNPCFactionID", &Perl_NPC_GetNPCFactionID);
	package.add("GetNPCHate", &Perl_NPC_GetNPCHate);
	package.add("GetNPCSpellsEffectsID", &Perl_NPC_GetNPCSpellsEffectsID);
	package.add("GetNPCSpellsID", &Perl_NPC_GetNPCSpellsID);
	package.add("GetNPCStat", &Perl_NPC_GetNPCStat);
	package.add("GetNPCTintIndex", &Perl_NPC_GetNPCTintIndex);
	package.add("GetPetSpellID", &Perl_NPC_GetPetSpellID);
	package.add("GetPlatinum", &Perl_NPC_GetPlatinum);
	package.add("GetPrimSkill", &Perl_NPC_GetPrimSkill);
	package.add("GetPrimaryFaction", &Perl_NPC_GetPrimaryFaction);
	package.add("GetScore", &Perl_NPC_GetScore);
	package.add("GetSecSkill", &Perl_NPC_GetSecSkill);
	package.add("GetSilver", &Perl_NPC_GetSilver);
	package.add("GetSlowMitigation", &Perl_NPC_GetSlowMitigation);
	package.add("GetSp2", &Perl_NPC_GetSp2);
	package.add("GetSpawn", &Perl_NPC_GetSpawn);
	package.add("GetSpawnKillCount", &Perl_NPC_GetSpawnKillCount);
	package.add("GetSpawnPointH", &Perl_NPC_GetSpawnPointH);
	package.add("GetSpawnPointID", &Perl_NPC_GetSpawnPointID);
	package.add("GetSpawnPointX", &Perl_NPC_GetSpawnPointX);
	package.add("GetSpawnPointY", &Perl_NPC_GetSpawnPointY);
	package.add("GetSpawnPointZ", &Perl_NPC_GetSpawnPointZ);
	package.add("GetSpellFocusDMG", &Perl_NPC_GetSpellFocusDMG);
	package.add("GetSpellFocusHeal", &Perl_NPC_GetSpellFocusHeal);
	package.add("GetSpellScale", &Perl_NPC_GetSpellScale);
	package.add("GetSwarmOwner", &Perl_NPC_GetSwarmOwner);
	package.add("GetSwarmTarget", &Perl_NPC_GetSwarmTarget);
	package.add("GetWaypointMax", &Perl_NPC_GetWaypointMax);
	package.add("HasAISpellEffect", &Perl_NPC_HasAISpellEffect);
	package.add("HasSpecialAbilities", &Perl_NPC_HasSpecialAbilities);
	package.add("HasItem", &Perl_NPC_HasItem);
	package.add("IsAnimal", &Perl_NPC_IsAnimal);
	package.add("IsGuarding", &Perl_NPC_IsGuarding);
	package.add("IsLDoNLocked", &Perl_NPC_IsLDoNLocked);
	package.add("IsLDoNTrapped", &Perl_NPC_IsLDoNTrapped);
	package.add("IsLDoNTrapDetected", &Perl_NPC_IsLDoNTrapDetected);
	package.add("IsMultiQuestEnabled", &Perl_NPC_IsMultiQuestEnabled);
	package.add("IsOnHatelist", &Perl_NPC_IsOnHatelist);
	package.add("IsRaidTarget", &Perl_NPC_IsRaidTarget);
	package.add("IsRareSpawn", &Perl_NPC_IsRareSpawn);
	package.add("IsResumedFromZoneSuspend", &Perl_NPC_IsResumedFromZoneSuspend);
	package.add("IsTaunting", &Perl_NPC_IsTaunting);
	package.add("IsUnderwaterOnly", (bool(*)(NPC*))&Perl_NPC_IsUnderwaterOnly);
	package.add("MerchantCloseShop", &Perl_NPC_MerchantCloseShop);
	package.add("MerchantOpenShop", &Perl_NPC_MerchantOpenShop);
	package.add("ModifyNPCStat", &Perl_NPC_ModifyNPCStat);
	package.add("MoveTo", (void(*)(NPC*, float, float, float))&Perl_NPC_MoveTo);
	package.add("MoveTo", (void(*)(NPC*, float, float, float, float))&Perl_NPC_MoveTo);
	package.add("MoveTo", (void(*)(NPC*, float, float, float, float, bool))&Perl_NPC_MoveTo);
	package.add("MultiQuestEnable", &Perl_NPC_MultiQuestEnable);
	package.add("NextGuardPosition", &Perl_NPC_NextGuardPosition);
	package.add("PauseWandering", &Perl_NPC_PauseWandering);
	package.add("PickPocket", &Perl_NPC_PickPocket);
	package.add("RecalculateSkills", &Perl_NPC_RecalculateSkills);
	package.add("ReloadSpells", &Perl_NPC_ReloadSpells);
	package.add("RemoveAISpell", &Perl_NPC_RemoveSpellFromNPCList);
	package.add("RemoveAISpellEffect", &Perl_NPC_RemoveAISpellEffect);
	package.add("RemoveCash", &Perl_NPC_RemoveLootCash);
	package.add("RemoveDefensiveProc", &Perl_NPC_RemoveDefensiveProc);
	package.add("RemoveFromHateList", &Perl_NPC_RemoveFromHateList);
	package.add("RemoveItem", (void(*)(NPC*, uint32))&Perl_NPC_RemoveItem);
	package.add("RemoveItem", (void(*)(NPC*, uint32, uint16))&Perl_NPC_RemoveItem);
	package.add("RemoveItem", (void(*)(NPC*, uint32, uint16, uint16))&Perl_NPC_RemoveItem);
	package.add("RemoveMeleeProc", &Perl_NPC_RemoveMeleeProc);
	package.add("RemoveRangedProc", &Perl_NPC_RemoveRangedProc);
	package.add("ResumeWandering", &Perl_NPC_ResumeWandering);
	package.add("ReturnHandinItems", &Perl_NPC_ReturnHandinItems);
	package.add("SaveGuardSpot", (void(*)(NPC*))&Perl_NPC_SaveGuardSpot);
	package.add("SaveGuardSpot", (void(*)(NPC*, bool))&Perl_NPC_SaveGuardSpot);
	package.add("SaveGuardSpot", (void(*)(NPC*, float, float, float, float))&Perl_NPC_SaveGuardSpot);
	package.add("ScaleNPC", (void(*)(NPC*, uint8))&Perl_NPC_ScaleNPC);
	package.add("ScaleNPC", (void(*)(NPC*, uint8, bool))&Perl_NPC_ScaleNPC);
	package.add("SendPayload", (void(*)(NPC*, int))&Perl_NPC_SendPayload);
	package.add("SendPayload", (void(*)(NPC*, int, std::string))&Perl_NPC_SendPayload);
	package.add("SetCopper", &Perl_NPC_SetCopper);
	package.add("SetKeepsSoldItems", &Perl_NPC_SetKeepsSoldItems);
	package.add("SetLDoNLocked", &Perl_NPC_SetLDoNLocked);
	package.add("SetLDoNLockedSkill", &Perl_NPC_SetLDoNLockedSkill);
	package.add("SetLDoNTrapped", &Perl_NPC_SetLDoNTrapped);
	package.add("SetLDoNTrapDetected", &Perl_NPC_SetLDoNTrapDetected);
	package.add("SetLDoNTrapSpellID", &Perl_NPC_SetLDoNTrapSpellID);
	package.add("SetLDoNTrapType", &Perl_NPC_SetLDoNTrapType);
	package.add("SetNPCAggro", &Perl_NPC_SetNPCAggro);
	package.add("SetGold", &Perl_NPC_SetGold);
	package.add("SetGrid", &Perl_NPC_SetGrid);
	package.add("SetNPCFactionID", &Perl_NPC_SetNPCFactionID);
	package.add("SetNPCTintIndex", &Perl_NPC_SetNPCTintIndex);
	package.add("SetPetSpellID", &Perl_NPC_SetPetSpellID);
	package.add("SetPlatinum", &Perl_NPC_SetPlatinum);
	package.add("SetPrimSkill", &Perl_NPC_SetPrimSkill);
	package.add("SetSaveWaypoint", &Perl_NPC_SetSaveWaypoint);
	package.add("SetSecSkill", &Perl_NPC_SetSecSkill);
	package.add("SetSilver", &Perl_NPC_SetSilver);
	package.add("SetSimpleRoamBox", (void(*)(NPC*, float))&Perl_NPC_SetSimpleRoamBox);
	package.add("SetSimpleRoamBox", (void(*)(NPC*, float, float))&Perl_NPC_SetSimpleRoamBox);
	package.add("SetSimpleRoamBox", (void(*)(NPC*, float, float, int))&Perl_NPC_SetSimpleRoamBox);
	package.add("SetSp2", &Perl_NPC_SetSp2);
	package.add("SetSpellFocusDMG", &Perl_NPC_SetSpellFocusDMG);
	package.add("SetSpellFocusHeal", &Perl_NPC_SetSpellFocusHeal);
	package.add("SetSwarmTarget", &Perl_NPC_SetSwarmTarget);
	package.add("SetTaunting", &Perl_NPC_SetTaunting);
	package.add("SetWaypointPause", &Perl_NPC_SetWaypointPause);
	package.add("SignalNPC", &Perl_NPC_SignalNPC);
	package.add("StartSwarmTimer", &Perl_NPC_StartSwarmTimer);
	package.add("StopWandering", &Perl_NPC_StopWandering);
	package.add("UpdateWaypoint", &Perl_NPC_UpdateWaypoint);
}

#endif //EMBPERL_XS_CLASSES
