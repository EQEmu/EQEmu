#ifdef LUA_EQEMU

#include "lua_npc.h"

#include "zone/lua_client.h"
#include "zone/lua_item.h"
#include "zone/lua_iteminst.h"
#include "zone/lua_spawn.h"
#include "zone/npc.h"

#include "lua.hpp"
#include "luabind/iterator_policy.hpp"
#include "luabind/luabind.hpp"

struct Lua_NPC_Loot_List {
	std::vector<uint32> entries;
};

void Lua_NPC::Signal(int signal_id) {
	Lua_Safe_Call_Void();
	self->SignalNPC(signal_id);
}

int Lua_NPC::CheckNPCFactionAlly(int faction) {
	Lua_Safe_Call_Int();
	return static_cast<int>(self->CheckNPCFactionAlly(faction));
}

void Lua_NPC::AddItem(int item_id, int charges) {
	Lua_Safe_Call_Void();
	self->AddItem(item_id, charges);
}

void Lua_NPC::AddItem(int item_id, int charges, bool equip) {
	Lua_Safe_Call_Void();
	self->AddItem(item_id, charges, equip);
}

void Lua_NPC::AddItem(int item_id, int charges, bool equip, int aug1) {
	Lua_Safe_Call_Void();
	self->AddItem(item_id, charges, equip, aug1);
}

void Lua_NPC::AddItem(int item_id, int charges, bool equip, int aug1, int aug2) {
	Lua_Safe_Call_Void();
	self->AddItem(item_id, charges, equip, aug1, aug2);
}

void Lua_NPC::AddItem(int item_id, int charges, bool equip, int aug1, int aug2, int aug3) {
	Lua_Safe_Call_Void();
	self->AddItem(item_id, charges, equip, aug1, aug2, aug3);
}

void Lua_NPC::AddItem(int item_id, int charges, bool equip, int aug1, int aug2, int aug3, int aug4) {
	Lua_Safe_Call_Void();
	self->AddItem(item_id, charges, equip, aug1, aug2, aug3, aug4);
}

void Lua_NPC::AddItem(int item_id, int charges, bool equip, int aug1, int aug2, int aug3, int aug4, int aug5) {
	Lua_Safe_Call_Void();
	self->AddItem(item_id, charges, equip, aug1, aug2, aug3, aug4, aug5);
}

void Lua_NPC::AddItem(int item_id, int charges, bool equip, int aug1, int aug2, int aug3, int aug4, int aug5, int aug6) {
	Lua_Safe_Call_Void();
	self->AddItem(item_id, charges, equip, aug1, aug2, aug3, aug4, aug5, aug6);
}

void Lua_NPC::AddLootTable() {
	Lua_Safe_Call_Void();
	self->AddLootTable();
}

void Lua_NPC::AddLootTable(int id) {
	Lua_Safe_Call_Void();
	self->AddLootTable(id);
}

void Lua_NPC::RemoveItem(int item_id) {
	Lua_Safe_Call_Void();
	self->RemoveItem(item_id);
}

void Lua_NPC::RemoveItem(int item_id, int quantity) {
	Lua_Safe_Call_Void();
	self->RemoveItem(item_id, quantity);
}

void Lua_NPC::RemoveItem(int item_id, int quantity, int slot) {
	Lua_Safe_Call_Void();
	self->RemoveItem(item_id, quantity, slot);
}

void Lua_NPC::ClearLootItems() {
	Lua_Safe_Call_Void();
	self->ClearLootItems();
}

void Lua_NPC::AddLootCash(uint32 copper, uint32 silver, uint32 gold, uint32 platinum) {
	Lua_Safe_Call_Void();
	self->AddLootCash(copper, silver, gold, platinum);
}

void Lua_NPC::RemoveLootCash() {
	Lua_Safe_Call_Void();
	self->RemoveLootCash();
}

int Lua_NPC::CountLoot() {
	Lua_Safe_Call_Int();
	return self->CountLoot();
}

int Lua_NPC::GetLoottableID() {
	Lua_Safe_Call_Int();
	return self->GetLoottableID();
}

uint32 Lua_NPC::GetCopper() {
	Lua_Safe_Call_Int();
	return self->GetCopper();
}

uint32 Lua_NPC::GetSilver() {
	Lua_Safe_Call_Int();
	return self->GetSilver();
}

uint32 Lua_NPC::GetGold() {
	Lua_Safe_Call_Int();
	return self->GetGold();
}

uint32 Lua_NPC::GetPlatinum() {
	Lua_Safe_Call_Int();
	return self->GetPlatinum();
}

void Lua_NPC::SetCopper(uint32 amt) {
	Lua_Safe_Call_Void();
	self->SetCopper(amt);
}

void Lua_NPC::SetSilver(uint32 amt) {
	Lua_Safe_Call_Void();
	self->SetSilver(amt);
}

void Lua_NPC::SetGold(uint32 amt) {
	Lua_Safe_Call_Void();
	self->SetGold(amt);
}

void Lua_NPC::SetPlatinum(uint32 amt) {
	Lua_Safe_Call_Void();
	self->SetPlatinum(amt);
}

void Lua_NPC::SetGrid(int grid) {
	Lua_Safe_Call_Void();
	self->SetGrid(grid);
}

void Lua_NPC::SetSaveWaypoint(int wp) {
	Lua_Safe_Call_Void();
	self->SetSaveWaypoint(wp);
}

void Lua_NPC::SetSp2(int sg2) {
	Lua_Safe_Call_Void();
	self->SetSpawnGroupId(sg2);
}

int Lua_NPC::GetWaypointMax() {
	Lua_Safe_Call_Int();
	return self->GetWaypointMax();
}

int Lua_NPC::GetGrid() {
	Lua_Safe_Call_Int();
	return self->GetGrid();
}

uint32 Lua_NPC::GetSp2() {
	Lua_Safe_Call_Int();
	return self->GetSpawnGroupId();
}

int Lua_NPC::GetNPCFactionID() {
	Lua_Safe_Call_Int();
	return self->GetNPCFactionID();
}

int Lua_NPC::GetPrimaryFaction() {
	Lua_Safe_Call_Int();
	return self->GetPrimaryFaction();
}

int64 Lua_NPC::GetNPCHate(Lua_Mob ent) {
	Lua_Safe_Call_Int();
	return self->GetNPCHate(ent);
}

bool Lua_NPC::IsOnHatelist(Lua_Mob ent) {
	Lua_Safe_Call_Bool();
	return self->IsOnHatelist(ent);
}

void Lua_NPC::SetNPCFactionID(int id) {
	Lua_Safe_Call_Void();
	self->SetNPCFactionID(id);
}

uint32 Lua_NPC::GetMaxDMG() {
	Lua_Safe_Call_Int();
	return self->GetMaxDMG();
}

uint32 Lua_NPC::GetMinDMG() {
	Lua_Safe_Call_Int();
	return self->GetMinDMG();
}

bool Lua_NPC::IsAnimal() {
	Lua_Safe_Call_Bool();
	return self->IsAnimal();
}

int Lua_NPC::GetPetSpellID() {
	Lua_Safe_Call_Int();
	return self->GetPetSpellID();
}

void Lua_NPC::SetPetSpellID(int id) {
	Lua_Safe_Call_Void();
	self->SetPetSpellID(id);
}

uint32 Lua_NPC::GetMaxDamage(int level) {
	Lua_Safe_Call_Int();
	return self->GetMaxDamage(level);
}

void Lua_NPC::SetTaunting(bool t) {
	Lua_Safe_Call_Void();
	self->SetTaunting(t);
}

bool Lua_NPC::IsTaunting() {
	Lua_Safe_Call_Bool();
	return self->IsTaunting();
}

void Lua_NPC::PickPocket(Lua_Client thief) {
	Lua_Safe_Call_Void();
	self->PickPocket(thief);
}

void Lua_NPC::StartSwarmTimer(uint32 duration) {
	Lua_Safe_Call_Void();
	self->StartSwarmTimer(duration);
}

void Lua_NPC::DoClassAttacks(Lua_Mob target) {
	Lua_Safe_Call_Void();
	self->DoClassAttacks(target);
}

int Lua_NPC::GetMaxWp() {
	Lua_Safe_Call_Int();
	return self->GetMaxWp();
}

void Lua_NPC::DisplayWaypointInfo(Lua_Client client) {
	Lua_Safe_Call_Void();
	self->DisplayWaypointInfo(client);
}

void Lua_NPC::CalculateNewWaypoint() {
	Lua_Safe_Call_Void();
	self->CalculateNewWaypoint();
}

void Lua_NPC::AssignWaypoints(int grid) {
	Lua_Safe_Call_Void();
	self->AssignWaypoints(grid);
}

void Lua_NPC::SetWaypointPause() {
	Lua_Safe_Call_Void();
	self->SetWaypointPause();
}

void Lua_NPC::UpdateWaypoint(int wp) {
	Lua_Safe_Call_Void();
	self->UpdateWaypoint(wp);
}

void Lua_NPC::StopWandering() {
	Lua_Safe_Call_Void();
	self->StopWandering();
}

void Lua_NPC::ResumeWandering() {
	Lua_Safe_Call_Void();
	self->ResumeWandering();
}

void Lua_NPC::PauseWandering(int pause_time) {
	Lua_Safe_Call_Void();
	self->PauseWandering(pause_time);
}

void Lua_NPC::MoveTo(float x, float y, float z, float h, bool save) {
	Lua_Safe_Call_Void();
	auto position = glm::vec4(x, y, z, h);
	self->MoveTo(position, save);
}

void Lua_NPC::NextGuardPosition() {
	Lua_Safe_Call_Void();
	self->NextGuardPosition();
}

void Lua_NPC::SaveGuardSpot() {
	Lua_Safe_Call_Void();
	self->SaveGuardSpot();
}

void Lua_NPC::SaveGuardSpot(bool clear) {
	Lua_Safe_Call_Void();
	self->SaveGuardSpot(clear);
}

void Lua_NPC::SaveGuardSpot(float x, float y, float z, float heading) {
	Lua_Safe_Call_Void();
	self->SaveGuardSpot(glm::vec4(x, y, z, heading));
}

bool Lua_NPC::IsGuarding() {
	Lua_Safe_Call_Bool();
	return self->IsGuarding();
}

void Lua_NPC::AI_SetRoambox(float dist, float max_x, float min_x, float max_y, float min_y) {
	Lua_Safe_Call_Void();
	self->AI_SetRoambox(dist, max_x, min_x, max_y, min_y);
}

void Lua_NPC::AI_SetRoambox(float dist, float max_x, float min_x, float max_y, float min_y, uint32 delay, uint32 mindelay) {
	Lua_Safe_Call_Void();
	self->AI_SetRoambox(dist, max_x, min_x, max_y, min_y, delay, mindelay);
}

void Lua_NPC::SetFollowID(int id) {
	Lua_Safe_Call_Void();
	self->SetFollowID(id);
}

void Lua_NPC::SetFollowDistance(int dist) {
	Lua_Safe_Call_Void();
	self->SetFollowDistance(dist);
}

void Lua_NPC::SetFollowCanRun(bool v) {
	Lua_Safe_Call_Void();
	self->SetFollowCanRun(v);
}

int Lua_NPC::GetFollowID() {
	Lua_Safe_Call_Int();
	return self->GetFollowID();
}

int Lua_NPC::GetFollowDistance() {
	Lua_Safe_Call_Int();
	return self->GetFollowDistance();
}

bool Lua_NPC::GetFollowCanRun() {
	Lua_Safe_Call_Bool();
	return self->GetFollowCanRun();
}

uint32 Lua_NPC::GetNPCSpellsID() {
	Lua_Safe_Call_Int();
	return self->GetNPCSpellsID();
}

int Lua_NPC::GetSpawnPointID() {
	Lua_Safe_Call_Int();
	return self->GetSpawnPointID();
}

float Lua_NPC::GetSpawnPointX() {
	Lua_Safe_Call_Real();
	return self->GetSpawnPoint().x;
}

float Lua_NPC::GetSpawnPointY() {
	Lua_Safe_Call_Real();
	return self->GetSpawnPoint().y;
}

float Lua_NPC::GetSpawnPointZ() {
	Lua_Safe_Call_Real();
	return self->GetSpawnPoint().z;
}

float Lua_NPC::GetSpawnPointH() {
	Lua_Safe_Call_Real();
	return self->GetSpawnPoint().w;
}

float Lua_NPC::GetGuardPointX() {
	Lua_Safe_Call_Real();
	return self->GetGuardPoint().x;
}

float Lua_NPC::GetGuardPointY() {
	Lua_Safe_Call_Real();
	return self->GetGuardPoint().y;
}

float Lua_NPC::GetGuardPointZ() {
	Lua_Safe_Call_Real();
	return self->GetGuardPoint().z;
}

void Lua_NPC::SetPrimSkill(int skill_id) {
	Lua_Safe_Call_Void();
	self->SetPrimSkill(skill_id);
}

void Lua_NPC::SetSecSkill(int skill_id) {
	Lua_Safe_Call_Void();
	self->SetSecSkill(skill_id);
}

int Lua_NPC::GetPrimSkill() {
	Lua_Safe_Call_Int();
	return self->GetPrimSkill();
}

int Lua_NPC::GetSecSkill() {
	Lua_Safe_Call_Int();
	return self->GetSecSkill();
}

int Lua_NPC::GetSwarmOwner() {
	Lua_Safe_Call_Int();
	return self->GetSwarmOwner();
}

int Lua_NPC::GetSwarmTarget() {
	Lua_Safe_Call_Int();
	return self->GetSwarmTarget();
}

void Lua_NPC::SetSwarmTarget(int target) {
	Lua_Safe_Call_Void();
	self->SetSwarmTarget(target);
}

void Lua_NPC::ModifyNPCStat(std::string stat, std::string value) {
	Lua_Safe_Call_Void();
	self->ModifyNPCStat(stat, value);
}

void Lua_NPC::AddAISpell(int priority, int spell_id, int type, int mana_cost, int recast_delay, int resist_adjust) {
	Lua_Safe_Call_Void();
	self->AddSpellToNPCList(priority, spell_id, type, mana_cost, recast_delay, resist_adjust, 0, 0);
}

void Lua_NPC::AddAISpell(int priority, int spell_id, int type, int mana_cost, int recast_delay, int resist_adjust, int min_hp, int max_hp) {
	Lua_Safe_Call_Void();
	self->AddSpellToNPCList(priority, spell_id, type, mana_cost, recast_delay, resist_adjust, min_hp, max_hp);
}

void Lua_NPC::RemoveAISpell(int spell_id) {
	Lua_Safe_Call_Void();
	self->RemoveSpellFromNPCList(spell_id);
}

void Lua_NPC::SetSpellFocusDMG(int focus) {
	Lua_Safe_Call_Void();
	self->SetSpellFocusDMG(focus);
}

void Lua_NPC::SetSpellFocusHeal(int focus) {
	Lua_Safe_Call_Void();
	self->SetSpellFocusHeal(focus);
}

int Lua_NPC::GetSpellFocusDMG() {
	Lua_Safe_Call_Int();
	return self->GetSpellFocusDMG();
}

int Lua_NPC::GetSpellFocusHeal() {
	Lua_Safe_Call_Int();
	return self->GetSpellFocusHeal();
}

float Lua_NPC::GetSlowMitigation() {
	Lua_Safe_Call_Real();
	return self->GetSlowMitigation();
}

float Lua_NPC::GetAttackSpeed() {
	Lua_Safe_Call_Real();
	return self->GetAttackSpeed();
}

int Lua_NPC::GetAttackDelay() {
	Lua_Safe_Call_Int();
	return self->GetAttackDelay();
}

int Lua_NPC::GetAccuracyRating() {
	Lua_Safe_Call_Int();
	return self->GetAccuracyRating();
}

int Lua_NPC::GetSpawnKillCount() {
	Lua_Safe_Call_Int();
	return self->GetSpawnKillCount();
}

int Lua_NPC::GetScore() {
	Lua_Safe_Call_Int();
	return self->GetScore();
}

void Lua_NPC::MerchantOpenShop() {
	Lua_Safe_Call_Void();
	self->MerchantOpenShop();
}

void Lua_NPC::MerchantCloseShop() {
	Lua_Safe_Call_Void();
	self->MerchantCloseShop();
}

int Lua_NPC::GetRawAC() {
	Lua_Safe_Call_Int();
	return self->GetRawAC();
}

int Lua_NPC::GetAvoidanceRating()
{
	Lua_Safe_Call_Int();
	return self->GetAvoidanceRating();
}

void Lua_NPC::SetSimpleRoamBox(float box_size)
{
	Lua_Safe_Call_Void();
	self->SetSimpleRoamBox(box_size);
}

void Lua_NPC::SetSimpleRoamBox(float box_size, float move_distance)
{
	Lua_Safe_Call_Void();
	self->SetSimpleRoamBox(box_size, move_distance);
}

void Lua_NPC::SetSimpleRoamBox(float box_size, float move_distance, int move_delay)
{
	Lua_Safe_Call_Void();
	self->SetSimpleRoamBox(box_size, move_distance, move_delay);
}

void Lua_NPC::RecalculateSkills()
{
	Lua_Safe_Call_Void();
	self->RecalculateSkills();
}

bool Lua_NPC::IsRaidTarget()
{
	Lua_Safe_Call_Bool();
	return self->IsRaidTarget();
}

void Lua_NPC::ChangeLastName(std::string last_name)
{
	Lua_Safe_Call_Void();
	self->ChangeLastName(last_name);
}

void Lua_NPC::ClearLastName()
{
	Lua_Safe_Call_Void();
	self->ClearLastName();
}

bool Lua_NPC::HasItem(uint32 item_id)
{
	Lua_Safe_Call_Bool();
	return self->HasItem(item_id);
}

uint32 Lua_NPC::CountItem(uint32 item_id)
{
	Lua_Safe_Call_Int();
	return self->CountItem(item_id);
}

uint32 Lua_NPC::GetLootItemIDBySlot(uint16 loot_slot)
{
	Lua_Safe_Call_Int();
	return self->GetLootItemIDBySlot(loot_slot);
}

uint16 Lua_NPC::GetFirstLootSlotByItemID(uint32 item_id)
{
	Lua_Safe_Call_Int();
	return self->GetFirstLootSlotByItemID(item_id);
}

float Lua_NPC::GetHealScale()
{
	Lua_Safe_Call_Real();
	return self->GetHealScale();
}

float Lua_NPC::GetSpellScale()
{
	Lua_Safe_Call_Real();
	return self->GetSpellScale();
}

Lua_NPC_Loot_List Lua_NPC::GetLootList(lua_State* L) {
	Lua_Safe_Call_Class(Lua_NPC_Loot_List);
	Lua_NPC_Loot_List ret;
	auto loot_list = self->GetLootList();

	for (auto item_id : loot_list) {
		ret.entries.push_back(item_id);
	}

	return ret;
}

void Lua_NPC::AddAISpellEffect(int spell_effect_id, int base_value, int limit_value, int max_value)
{
	Lua_Safe_Call_Void();
	self->AddSpellEffectToNPCList(spell_effect_id, base_value, limit_value, max_value, true);
}

void Lua_NPC::RemoveAISpellEffect(int spell_effect_id)
{
	Lua_Safe_Call_Void();
	self->RemoveSpellEffectFromNPCList(spell_effect_id, true);
}

bool Lua_NPC::HasAISpellEffect(int spell_effect_id)
{
	Lua_Safe_Call_Bool();
	return self->HasAISpellEffect(spell_effect_id);
}

float Lua_NPC::GetNPCStat(std::string stat)
{
	Lua_Safe_Call_Real();
	return self->GetNPCStat(stat);
}

bool Lua_NPC::IsRareSpawn()
{
	Lua_Safe_Call_Bool();
	return self->IsRareSpawn();
}

void Lua_NPC::ReloadSpells()
{
	Lua_Safe_Call_Void();
	self->ReloadSpells();
}

void Lua_NPC::SendPayload(int payload_id) {
	Lua_Safe_Call_Void();
	self->SendPayload(payload_id);
}

void Lua_NPC::SendPayload(int payload_id, std::string payload_value) {
	Lua_Safe_Call_Void();
	self->SendPayload(payload_id, payload_value);
}

bool Lua_NPC::GetKeepsSoldItems() {
	Lua_Safe_Call_Bool();
	return self->GetKeepsSoldItems();
}

void Lua_NPC::SetKeepsSoldItems(bool keeps_sold_items) {
	Lua_Safe_Call_Void();
	self->SetKeepsSoldItems(keeps_sold_items);
}

bool Lua_NPC::IsLDoNTrapped() {
	Lua_Safe_Call_Bool();
	return self->IsLDoNTrapped();
}

void Lua_NPC::SetLDoNTrapped(bool is_trapped) {
	Lua_Safe_Call_Void();
	self->SetLDoNTrapped(is_trapped);
}

uint8 Lua_NPC::GetLDoNTrapType() {
	Lua_Safe_Call_Int();
	return self->GetLDoNTrapType();
}

void Lua_NPC::SetLDoNTrapType(uint8 trap_type) {
	Lua_Safe_Call_Void();
	self->SetLDoNTrapType(trap_type);
}

uint16 Lua_NPC::GetLDoNTrapSpellID() {
	Lua_Safe_Call_Int();
	return self->GetLDoNTrapSpellID();
}

void Lua_NPC::SetLDoNTrapSpellID(uint16 spell_id) {
	Lua_Safe_Call_Void();
	self->SetLDoNTrapSpellID(spell_id);
}

bool Lua_NPC::IsLDoNLocked() {
	Lua_Safe_Call_Bool();
	return self->IsLDoNLocked();
}

void Lua_NPC::SetLDoNLocked(bool is_locked) {
	Lua_Safe_Call_Void();
	self->SetLDoNLocked(is_locked);
}

uint16 Lua_NPC::GetLDoNLockedSkill() {
	Lua_Safe_Call_Int();
	return self->GetLDoNLockedSkill();
}

void Lua_NPC::SetLDoNLockedSkill(uint16 skill_value) {
	Lua_Safe_Call_Void();
	self->SetLDoNLockedSkill(skill_value);
}

bool Lua_NPC::IsLDoNTrapDetected() {
	Lua_Safe_Call_Bool();
	return self->IsLDoNTrapDetected();
}

void Lua_NPC::SetLDoNTrapDetected(bool is_detected) {
	Lua_Safe_Call_Void();
	self->SetLDoNTrapDetected(is_detected);
}

void Lua_NPC::ScaleNPC(uint8 npc_level)
{
	Lua_Safe_Call_Void();
	self->ScaleNPC(npc_level, true);
}

void Lua_NPC::ScaleNPC(uint8 npc_level, bool override_special_abilities)
{
	Lua_Safe_Call_Void();
	self->ScaleNPC(npc_level, true, override_special_abilities);
}

bool Lua_NPC::IsUnderwaterOnly() {
	Lua_Safe_Call_Bool();
	return self->IsUnderwaterOnly();
}

bool Lua_NPC::HasSpecialAbilities() {
	Lua_Safe_Call_Bool();
	return self->HasSpecialAbilities();
}

void Lua_NPC::DeleteBucket(std::string bucket_name)
{
	Lua_Safe_Call_Void();
	self->DeleteBucket(bucket_name);
}

std::string Lua_NPC::GetBucket(std::string bucket_name)
{
	Lua_Safe_Call_String();
	return self->GetBucket(bucket_name);
}

std::string Lua_NPC::GetBucketExpires(std::string bucket_name)
{
	Lua_Safe_Call_String();
	return self->GetBucketExpires(bucket_name);
}

std::string Lua_NPC::GetBucketRemaining(std::string bucket_name)
{
	Lua_Safe_Call_String();
	return self->GetBucketRemaining(bucket_name);
}

void Lua_NPC::SetBucket(std::string bucket_name, std::string bucket_value)
{
	Lua_Safe_Call_Void();
	self->SetBucket(bucket_name, bucket_value);
}

void Lua_NPC::SetBucket(std::string bucket_name, std::string bucket_value, std::string expiration)
{
	Lua_Safe_Call_Void();
	self->SetBucket(bucket_name, bucket_value, expiration);
}

bool Lua_NPC::GetNPCAggro()
{
	Lua_Safe_Call_Bool();
	return self->GetNPCAggro();
}

void Lua_NPC::SetNPCAggro(bool in_npc_aggro)
{
	Lua_Safe_Call_Void();
	self->SetNPCAggro(in_npc_aggro);
}

uint32 Lua_NPC::GetNPCSpellsEffectsID()
{
	Lua_Safe_Call_Int();
	return self->GetNPCSpellsEffectsID();
}

void Lua_NPC::DescribeSpecialAbilities(Lua_Client c)
{
	Lua_Safe_Call_Void();
	self->DescribeSpecialAbilities(c);
}

bool Lua_NPC::IsMultiQuestEnabled()
{
	Lua_Safe_Call_Bool();
	return self->IsMultiQuestEnabled();
}

void Lua_NPC::MultiQuestEnable()
{
	Lua_Safe_Call_Void();
	self->MultiQuestEnable();
}

bool Lua_NPC::LuaCheckHandin(
	Lua_Client c,
	luabind::adl::object handin_table,
	luabind::adl::object required_table,
	luabind::adl::object items_table
)
{
	Lua_Safe_Call_Bool();

	if (
		luabind::type(handin_table) != LUA_TTABLE ||
		luabind::type(required_table) != LUA_TTABLE ||
		luabind::type(items_table) != LUA_TTABLE
		) {
		return false;
	}

	std::map<std::string, uint32>   handin_map;
	std::map<std::string, uint32>   required_map;
	std::vector<EQ::ItemInstance *> items;

	for (luabind::iterator i(handin_table), end; i != end; i++) {
		std::string key;
		if (luabind::type(i.key()) == LUA_TSTRING) {
			key = luabind::object_cast<std::string>(i.key());
		}
		else if (luabind::type(i.key()) == LUA_TNUMBER) {
			key = fmt::format("{}", luabind::object_cast<int>(i.key()));
		}
		else {
			LogError("Handin key type [{}] not supported", luabind::type(i.key()));
		}

		if (!key.empty()) {
			handin_map[key] = luabind::object_cast<uint32>(handin_table[i.key()]);
			LogNpcHandinDetail("Handin key [{}] value [{}]", key, handin_map[key]);
		}
	}

	for (luabind::iterator i(required_table), end; i != end; i++) {
		std::string key;
		if (luabind::type(i.key()) == LUA_TSTRING) {
			key = luabind::object_cast<std::string>(i.key());
		}
		else if (luabind::type(i.key()) == LUA_TNUMBER) {
			key = fmt::format("{}", luabind::object_cast<int>(i.key()));
		}
		else {
			LogError("Required key type [{}] not supported", luabind::type(i.key()));
		}

		if (!key.empty()) {
			required_map[key] = luabind::object_cast<uint32>(required_table[i.key()]);
			LogNpcHandinDetail("Required key [{}] value [{}]", key, required_map[key]);
		}
	}

	for (luabind::iterator i(items_table), end; i != end; i++) {
		auto item = luabind::object_cast<Lua_ItemInst>(items_table[i.key()]);

		if (item && item.GetItem()) {
			LogNpcHandinDetail(
				"Item instance [{}] ({}) UUID ({}) added to handin list",
				item.GetName(),
				item.GetID(),
				item.GetSerialNumber()
			);

			items.emplace_back(item);
		}
	}

	return self->CheckHandin(c, handin_map, required_map, items);
}

void Lua_NPC::ReturnHandinItems(Lua_Client c)
{
	Lua_Safe_Call_Void();
	self->ReturnHandinItems(c);
}

Lua_Spawn Lua_NPC::GetSpawn(lua_State* L)
{
	Lua_Safe_Call_Class(Lua_Spawn);
	return Lua_Spawn(self->GetSpawn());
}

bool Lua_NPC::IsResumedFromZoneSuspend()
{
	Lua_Safe_Call_Bool();
	return self->IsResumedFromZoneSuspend();
}

void Lua_NPC::SetNPCTintIndex(uint32 index)
{
	Lua_Safe_Call_Void();
	self->SetNPCTintIndex(index);
}

uint32 Lua_NPC::GetNPCTintIndex()
{
	Lua_Safe_Call_Int();
	return self->GetNPCTintIndex();
}

luabind::scope lua_register_npc() {
	return luabind::class_<Lua_NPC, Lua_Mob>("NPC")
	.def(luabind::constructor<>())
	.def("AI_SetRoambox", (void(Lua_NPC::*)(float,float,float,float,float))&Lua_NPC::AI_SetRoambox)
	.def("AI_SetRoambox", (void(Lua_NPC::*)(float,float,float,float,float,uint32,uint32))&Lua_NPC::AI_SetRoambox)
	.def("AddAISpell", (void(Lua_NPC::*)(int,int,int,int,int,int))&Lua_NPC::AddAISpell)
	.def("AddAISpell", (void(Lua_NPC::*)(int,int,int,int,int,int,int,int))&Lua_NPC::AddAISpell)
	.def("AddAISpellEffect", (void(Lua_NPC::*)(int,int,int,int))&Lua_NPC::AddAISpellEffect)
	.def("AddCash", (void(Lua_NPC::*)(uint32,uint32,uint32,uint32))&Lua_NPC::AddLootCash)
	.def("AddItem", (void(Lua_NPC::*)(int,int))&Lua_NPC::AddItem)
	.def("AddItem", (void(Lua_NPC::*)(int,int,bool))&Lua_NPC::AddItem)
	.def("AddItem", (void(Lua_NPC::*)(int,int,bool,int))&Lua_NPC::AddItem)
	.def("AddItem", (void(Lua_NPC::*)(int,int,bool,int,int))&Lua_NPC::AddItem)
	.def("AddItem", (void(Lua_NPC::*)(int,int,bool,int,int,int))&Lua_NPC::AddItem)
	.def("AddItem", (void(Lua_NPC::*)(int,int,bool,int,int,int,int))&Lua_NPC::AddItem)
	.def("AddItem", (void(Lua_NPC::*)(int,int,bool,int,int,int,int,int))&Lua_NPC::AddItem)
	.def("AddItem", (void(Lua_NPC::*)(int,int,bool,int,int,int,int,int,int))&Lua_NPC::AddItem)
	.def("AddLootTable", (void(Lua_NPC::*)(int))&Lua_NPC::AddLootTable)
	.def("AddLootTable", (void(Lua_NPC::*)(void))&Lua_NPC::AddLootTable)
	.def("AssignWaypoints", (void(Lua_NPC::*)(int))&Lua_NPC::AssignWaypoints)
	.def("CalculateNewWaypoint", (void(Lua_NPC::*)(void))&Lua_NPC::CalculateNewWaypoint)
	.def("ChangeLastName", (void(Lua_NPC::*)(std::string))&Lua_NPC::ChangeLastName)
	.def("CheckHandin", (bool(Lua_NPC::*)(Lua_Client,luabind::adl::object,luabind::adl::object,luabind::adl::object))&Lua_NPC::LuaCheckHandin)
	.def("CheckNPCFactionAlly", (int(Lua_NPC::*)(int))&Lua_NPC::CheckNPCFactionAlly)
	.def("ClearItemList", (void(Lua_NPC::*)(void))&Lua_NPC::ClearLootItems)
	.def("ClearLastName", (void(Lua_NPC::*)(void))&Lua_NPC::ClearLastName)
	.def("CountItem", (uint32(Lua_NPC::*)(uint32))&Lua_NPC::CountItem)
	.def("CountLoot", (int(Lua_NPC::*)(void))&Lua_NPC::CountLoot)
	.def("DeleteBucket", (void(Lua_NPC::*)(std::string))&Lua_NPC::DeleteBucket)
	.def("DescribeSpecialAbilities", (void(Lua_NPC::*)(Lua_Client))&Lua_NPC::DescribeSpecialAbilities)
	.def("DisplayWaypointInfo", (void(Lua_NPC::*)(Lua_Client))&Lua_NPC::DisplayWaypointInfo)
	.def("DoClassAttacks", (void(Lua_NPC::*)(Lua_Mob))&Lua_NPC::DoClassAttacks)
	.def("GetAccuracyRating", (int(Lua_NPC::*)(void))&Lua_NPC::GetAccuracyRating)
	.def("GetAttackDelay", (int(Lua_NPC::*)(void))&Lua_NPC::GetAttackDelay)
	.def("GetAttackSpeed", (float(Lua_NPC::*)(void))&Lua_NPC::GetAttackSpeed)
	.def("GetAvoidanceRating", &Lua_NPC::GetAvoidanceRating)
	.def("GetBucket", (std::string(Lua_NPC::*)(std::string))&Lua_NPC::GetBucket)
	.def("GetBucketExpires", (std::string(Lua_NPC::*)(std::string))&Lua_NPC::GetBucketExpires)
	.def("GetBucketRemaining", (std::string(Lua_NPC::*)(std::string))&Lua_NPC::GetBucketRemaining)
	.def("GetCopper", (uint32(Lua_NPC::*)(void))&Lua_NPC::GetCopper)
	.def("GetFirstSlotByItemID", (uint16(Lua_NPC::*)(uint32))&Lua_NPC::GetFirstLootSlotByItemID)
	.def("GetFollowCanRun", (bool(Lua_NPC::*)(void))&Lua_NPC::GetFollowCanRun)
	.def("GetFollowDistance", (int(Lua_NPC::*)(void))&Lua_NPC::GetFollowDistance)
	.def("GetFollowID", (int(Lua_NPC::*)(void))&Lua_NPC::GetFollowID)
	.def("GetGold", (uint32(Lua_NPC::*)(void))&Lua_NPC::GetGold)
	.def("GetGrid", (int(Lua_NPC::*)(void))&Lua_NPC::GetGrid)
	.def("GetGuardPointX", (float(Lua_NPC::*)(void))&Lua_NPC::GetGuardPointX)
	.def("GetGuardPointY", (float(Lua_NPC::*)(void))&Lua_NPC::GetGuardPointY)
	.def("GetGuardPointZ", (float(Lua_NPC::*)(void))&Lua_NPC::GetGuardPointZ)
	.def("GetHealScale", (float(Lua_NPC::*)(void))&Lua_NPC::GetHealScale)
	.def("GetItemIDBySlot", (uint32(Lua_NPC::*)(uint16)) &Lua_NPC::GetLootItemIDBySlot)
	.def("GetKeepsSoldItems", (bool(Lua_NPC::*)(void))&Lua_NPC::GetKeepsSoldItems)
	.def("GetLootList", (Lua_NPC_Loot_List(Lua_NPC::*)(lua_State* L))&Lua_NPC::GetLootList)
	.def("GetLoottableID", (int(Lua_NPC::*)(void))&Lua_NPC::GetLoottableID)
	.def("GetMaxDMG", (uint32(Lua_NPC::*)(void))&Lua_NPC::GetMaxDMG)
	.def("GetMaxDamage", (uint32(Lua_NPC::*)(int))&Lua_NPC::GetMaxDamage)
	.def("GetMaxWp", (int(Lua_NPC::*)(void))&Lua_NPC::GetMaxWp)
	.def("GetMinDMG", (uint32(Lua_NPC::*)(void))&Lua_NPC::GetMinDMG)
	.def("GetLDoNLockedSkill", (uint16(Lua_NPC::*)(void))&Lua_NPC::GetLDoNLockedSkill)
	.def("GetLDoNTrapType", (uint8(Lua_NPC::*)(void))&Lua_NPC::GetLDoNTrapType)
	.def("GetLDoNTrapSpellID", (uint16(Lua_NPC::*)(void))&Lua_NPC::GetLDoNTrapSpellID)
	.def("GetNPCAggro", (bool(Lua_NPC::*)(void))&Lua_NPC::GetNPCAggro)
	.def("GetNPCFactionID", (int(Lua_NPC::*)(void))&Lua_NPC::GetNPCFactionID)
	.def("GetNPCHate", (int64(Lua_NPC::*)(Lua_Mob))&Lua_NPC::GetNPCHate)
	.def("GetNPCSpellsEffectsID", (uint32(Lua_NPC::*)(void))&Lua_NPC::GetNPCSpellsEffectsID)
	.def("GetNPCSpellsID", (uint32(Lua_NPC::*)(void))&Lua_NPC::GetNPCSpellsID)
	.def("GetNPCStat", (float(Lua_NPC::*)(std::string))&Lua_NPC::GetNPCStat)
	.def("GetNPCTintIndex", (uint32(Lua_NPC::*)(void))&Lua_NPC::GetNPCTintIndex)
	.def("GetPetSpellID", (int(Lua_NPC::*)(void))&Lua_NPC::GetPetSpellID)
	.def("GetPlatinum", (uint32(Lua_NPC::*)(void))&Lua_NPC::GetPlatinum)
	.def("GetPrimSkill", (int(Lua_NPC::*)(void))&Lua_NPC::GetPrimSkill)
	.def("GetPrimaryFaction", (int(Lua_NPC::*)(void))&Lua_NPC::GetPrimaryFaction)
	.def("GetRawAC", (int(Lua_NPC::*)(void))&Lua_NPC::GetRawAC)
	.def("GetScore", (int(Lua_NPC::*)(void))&Lua_NPC::GetScore)
	.def("GetSecSkill", (int(Lua_NPC::*)(void))&Lua_NPC::GetSecSkill)
	.def("GetSilver", (uint32(Lua_NPC::*)(void))&Lua_NPC::GetSilver)
	.def("GetSlowMitigation", (int(Lua_NPC::*)(void))&Lua_NPC::GetSlowMitigation)
	.def("GetSp2", (uint32(Lua_NPC::*)(void))&Lua_NPC::GetSp2)
	.def("GetSpawn", (Lua_Spawn(Lua_NPC::*)(void))&Lua_NPC::GetSpawn)
	.def("GetSpawnKillCount", (int(Lua_NPC::*)(void))&Lua_NPC::GetSpawnKillCount)
	.def("GetSpawnPointH", (float(Lua_NPC::*)(void))&Lua_NPC::GetSpawnPointH)
	.def("GetSpawnPointID", (int(Lua_NPC::*)(void))&Lua_NPC::GetSpawnPointID)
	.def("GetSpawnPointX", (float(Lua_NPC::*)(void))&Lua_NPC::GetSpawnPointX)
	.def("GetSpawnPointY", (float(Lua_NPC::*)(void))&Lua_NPC::GetSpawnPointY)
	.def("GetSpawnPointZ", (float(Lua_NPC::*)(void))&Lua_NPC::GetSpawnPointZ)
	.def("GetSpellFocusDMG", (void(Lua_NPC::*)(int))&Lua_NPC::GetSpellFocusDMG)
	.def("GetSpellFocusHeal", (void(Lua_NPC::*)(int))&Lua_NPC::GetSpellFocusHeal)
	.def("GetSpellScale", (float(Lua_NPC::*)(void))&Lua_NPC::GetSpellScale)
	.def("GetSwarmOwner", (int(Lua_NPC::*)(void))&Lua_NPC::GetSwarmOwner)
	.def("GetSwarmTarget", (int(Lua_NPC::*)(void))&Lua_NPC::GetSwarmTarget)
	.def("GetWaypointMax", (int(Lua_NPC::*)(void))&Lua_NPC::GetWaypointMax)
	.def("HasAISpellEffect", (bool(Lua_NPC::*)(int))&Lua_NPC::HasAISpellEffect)
	.def("HasItem", (bool(Lua_NPC::*)(uint32))&Lua_NPC::HasItem)
	.def("IsAnimal", (bool(Lua_NPC::*)(void))&Lua_NPC::IsAnimal)
	.def("IsGuarding", (bool(Lua_NPC::*)(void))&Lua_NPC::IsGuarding)
	.def("IsLDoNLocked", (bool(Lua_NPC::*)(void))&Lua_NPC::IsLDoNLocked)
	.def("IsLDoNTrapped", (bool(Lua_NPC::*)(void))&Lua_NPC::IsLDoNTrapped)
	.def("IsLDoNTrapDetected", (bool(Lua_NPC::*)(void))&Lua_NPC::IsLDoNTrapDetected)
	.def("IsMultiQuestEnabled", (bool(Lua_NPC::*)(void))&Lua_NPC::IsMultiQuestEnabled)
	.def("IsOnHatelist", (bool(Lua_NPC::*)(Lua_Mob))&Lua_NPC::IsOnHatelist)
	.def("IsRaidTarget", (bool(Lua_NPC::*)(void))&Lua_NPC::IsRaidTarget)
	.def("IsRareSpawn", (bool(Lua_NPC::*)(void))&Lua_NPC::IsRareSpawn)
	.def("IsResumedFromZoneSuspend",(bool(Lua_NPC::*)(void))&Lua_NPC::IsResumedFromZoneSuspend)
	.def("IsTaunting", (bool(Lua_NPC::*)(void))&Lua_NPC::IsTaunting)
	.def("IsUnderwaterOnly", (bool(Lua_NPC::*)(void))&Lua_NPC::IsUnderwaterOnly)
	.def("MerchantCloseShop", (void(Lua_NPC::*)(void))&Lua_NPC::MerchantCloseShop)
	.def("MerchantOpenShop", (void(Lua_NPC::*)(void))&Lua_NPC::MerchantOpenShop)
	.def("ModifyNPCStat", (void(Lua_NPC::*)(std::string,std::string))&Lua_NPC::ModifyNPCStat)
	.def("MoveTo", (void(Lua_NPC::*)(float,float,float,float,bool))&Lua_NPC::MoveTo)
	.def("MultiQuestEnable", &Lua_NPC::MultiQuestEnable)
	.def("NextGuardPosition", (void(Lua_NPC::*)(void))&Lua_NPC::NextGuardPosition)
	.def("PauseWandering", (void(Lua_NPC::*)(int))&Lua_NPC::PauseWandering)
	.def("PickPocket", (void(Lua_NPC::*)(Lua_Client))&Lua_NPC::PickPocket)
	.def("RecalculateSkills", (void(Lua_NPC::*)(void))&Lua_NPC::RecalculateSkills)
	.def("ReloadSpells", (void(Lua_NPC::*)(void))&Lua_NPC::ReloadSpells)
	.def("RemoveAISpell", (void(Lua_NPC::*)(int))&Lua_NPC::RemoveAISpell)
	.def("RemoveAISpellEffect", (void(Lua_NPC::*)(int))&Lua_NPC::RemoveAISpellEffect)
	.def("RemoveCash", (void(Lua_NPC::*)(void))&Lua_NPC::RemoveLootCash)
	.def("RemoveItem", (void(Lua_NPC::*)(int))&Lua_NPC::RemoveItem)
	.def("RemoveItem", (void(Lua_NPC::*)(int,int))&Lua_NPC::RemoveItem)
	.def("RemoveItem", (void(Lua_NPC::*)(int,int,int))&Lua_NPC::RemoveItem)
	.def("ResumeWandering", (void(Lua_NPC::*)(void))&Lua_NPC::ResumeWandering)
	.def("ReturnHandinItems", (void(Lua_NPC::*)(Lua_Client))&Lua_NPC::ReturnHandinItems)
	.def("SaveGuardSpot", (void(Lua_NPC::*)(void))&Lua_NPC::SaveGuardSpot)
	.def("SaveGuardSpot", (void(Lua_NPC::*)(bool))&Lua_NPC::SaveGuardSpot)
	.def("SaveGuardSpot", (void(Lua_NPC::*)(float,float,float,float))&Lua_NPC::SaveGuardSpot)
	.def("ScaleNPC", (void(Lua_NPC::*)(uint8))&Lua_NPC::ScaleNPC)
	.def("ScaleNPC", (void(Lua_NPC::*)(uint8,bool))&Lua_NPC::ScaleNPC)
	.def("SendPayload", (void(Lua_NPC::*)(int))&Lua_NPC::SendPayload)
	.def("SendPayload", (void(Lua_NPC::*)(int,std::string))&Lua_NPC::SendPayload)
	.def("SetBucket", (void(Lua_NPC::*)(std::string,std::string))&Lua_NPC::SetBucket)
	.def("SetBucket", (void(Lua_NPC::*)(std::string,std::string,std::string))&Lua_NPC::SetBucket)
	.def("SetCopper", (void(Lua_NPC::*)(uint32))&Lua_NPC::SetCopper)
	.def("SetFollowCanRun", (void(Lua_NPC::*)(bool))&Lua_NPC::SetFollowCanRun)
	.def("SetFollowDistance", (void(Lua_NPC::*)(int))&Lua_NPC::SetFollowDistance)
	.def("SetFollowID", (void(Lua_NPC::*)(int))&Lua_NPC::SetFollowID)
	.def("SetGold", (void(Lua_NPC::*)(uint32))&Lua_NPC::SetGold)
	.def("SetGrid", (void(Lua_NPC::*)(int))&Lua_NPC::SetGrid)
	.def("SetKeepsSoldItems", (void(Lua_NPC::*)(bool))&Lua_NPC::SetKeepsSoldItems)
	.def("SetLDoNLocked", (void(Lua_NPC::*)(bool))&Lua_NPC::SetLDoNLocked)
	.def("SetLDoNLockedSkill", (void(Lua_NPC::*)(uint16))&Lua_NPC::SetLDoNLockedSkill)
	.def("SetLDoNTrapped", (void(Lua_NPC::*)(bool))&Lua_NPC::SetLDoNTrapped)
	.def("SetLDoNTrapDetected", (void(Lua_NPC::*)(bool))&Lua_NPC::SetLDoNTrapDetected)
	.def("SetLDoNTrapSpellID", (void(Lua_NPC::*)(uint16))&Lua_NPC::SetLDoNTrapSpellID)
	.def("SetLDoNTrapType", (void(Lua_NPC::*)(uint8))&Lua_NPC::SetLDoNTrapType)
	.def("SetNPCAggro", (void(Lua_NPC::*)(bool))&Lua_NPC::SetNPCAggro)
	.def("SetNPCFactionID", (void(Lua_NPC::*)(int))&Lua_NPC::SetNPCFactionID)
	.def("SetNPCTintIndex", &Lua_NPC::SetNPCTintIndex)
	.def("SetPetSpellID", (void(Lua_NPC::*)(int))&Lua_NPC::SetPetSpellID)
	.def("SetPlatinum", (void(Lua_NPC::*)(uint32))&Lua_NPC::SetPlatinum)
	.def("SetPrimSkill", (void(Lua_NPC::*)(int))&Lua_NPC::SetPrimSkill)
	.def("SetSaveWaypoint", (void(Lua_NPC::*)(int))&Lua_NPC::SetSaveWaypoint)
	.def("SetSecSkill", (void(Lua_NPC::*)(int))&Lua_NPC::SetSecSkill)
	.def("SetSilver", (void(Lua_NPC::*)(uint32))&Lua_NPC::SetSilver)
	.def("SetSimpleRoamBox", (void(Lua_NPC::*)(float))&Lua_NPC::SetSimpleRoamBox)
	.def("SetSimpleRoamBox", (void(Lua_NPC::*)(float, float))&Lua_NPC::SetSimpleRoamBox)
	.def("SetSimpleRoamBox", (void(Lua_NPC::*)(float, float, int))&Lua_NPC::SetSimpleRoamBox)
	.def("SetSp2", (void(Lua_NPC::*)(int))&Lua_NPC::SetSp2)
	.def("SetSpellFocusDMG", (void(Lua_NPC::*)(int))&Lua_NPC::SetSpellFocusDMG)
	.def("SetSpellFocusHeal", (void(Lua_NPC::*)(int))&Lua_NPC::SetSpellFocusHeal)
	.def("SetSwarmTarget", (void(Lua_NPC::*)(int))&Lua_NPC::SetSwarmTarget)
	.def("SetTaunting", (void(Lua_NPC::*)(bool))&Lua_NPC::SetTaunting)
	.def("SetWaypointPause", (void(Lua_NPC::*)(void))&Lua_NPC::SetWaypointPause)
	.def("Signal", (void(Lua_NPC::*)(int))&Lua_NPC::Signal)
	.def("StartSwarmTimer", (void(Lua_NPC::*)(uint32))&Lua_NPC::StartSwarmTimer)
	.def("StopWandering", (void(Lua_NPC::*)(void))&Lua_NPC::StopWandering)
	.def("UpdateWaypoint", (void(Lua_NPC::*)(int))&Lua_NPC::UpdateWaypoint);
}

luabind::scope lua_register_npc_loot_list() {
	return luabind::class_<Lua_NPC_Loot_List>("NPCLootList")
	.def_readwrite("entries", &Lua_NPC_Loot_List::entries, luabind::return_stl_iterator);
}

#endif // LUA_EQEMU
