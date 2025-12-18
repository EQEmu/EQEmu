#ifdef LUA_EQEMU

#include "lua_bot.h"

#include "zone/bot.h"
#include "zone/lua_iteminst.h"
#include "zone/lua_mob.h"
#include "zone/lua_group.h"
#include "zone/lua_item.h"

#include "lua.hpp"
#include "luabind/luabind.hpp"

void Lua_Bot::AddBotItem(uint16 slot_id, uint32 item_id) {
	Lua_Safe_Call_Void();
	self->AddBotItem(slot_id, item_id);
}

void Lua_Bot::AddBotItem(uint16 slot_id, uint32 item_id, int16 charges) {
	Lua_Safe_Call_Void();
	self->AddBotItem(slot_id, item_id, charges);
}

void Lua_Bot::AddBotItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned) {
	Lua_Safe_Call_Void();
	self->AddBotItem(slot_id, item_id, charges, attuned);
}

void Lua_Bot::AddBotItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned, uint32 augment_one) {
	Lua_Safe_Call_Void();
	self->AddBotItem(slot_id, item_id, charges, attuned, augment_one);
}

void Lua_Bot::AddBotItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned, uint32 augment_one, uint32 augment_two) {
	Lua_Safe_Call_Void();
	self->AddBotItem(slot_id, item_id, charges, attuned, augment_one, augment_two);
}

void Lua_Bot::AddBotItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned, uint32 augment_one, uint32 augment_two, uint32 augment_three) {
	Lua_Safe_Call_Void();
	self->AddBotItem(slot_id, item_id, charges, attuned, augment_one, augment_two, augment_three);
}

void Lua_Bot::AddBotItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned, uint32 augment_one, uint32 augment_two, uint32 augment_three, uint32 augment_four) {
	Lua_Safe_Call_Void();
	self->AddBotItem(slot_id, item_id, charges, attuned, augment_one, augment_two, augment_three, augment_four);
}

void Lua_Bot::AddBotItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned, uint32 augment_one, uint32 augment_two, uint32 augment_three, uint32 augment_four, uint32 augment_five) {
	Lua_Safe_Call_Void();
	self->AddBotItem(slot_id, item_id, charges, attuned, augment_one, augment_two, augment_three, augment_four, augment_five);
}

void Lua_Bot::AddBotItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned, uint32 augment_one, uint32 augment_two, uint32 augment_three, uint32 augment_four, uint32 augment_five, uint32 augment_six) {
	Lua_Safe_Call_Void();
	self->AddBotItem(slot_id, item_id, charges, attuned, augment_one, augment_two, augment_three, augment_four, augment_five, augment_six);
}

uint32 Lua_Bot::CountBotItem(uint32 item_id) {
	Lua_Safe_Call_Int();
	return self->CountBotItem(item_id);
}

Lua_Mob Lua_Bot::GetOwner() {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetOwner());
}

int16 Lua_Bot::HasBotItem(uint32 item_id) {
	Lua_Safe_Call_Int();
	return self->HasBotItem(item_id);
}

void Lua_Bot::RemoveBotItem(uint32 item_id) {
	Lua_Safe_Call_Void();
	self->RemoveBotItem(item_id);
}

Lua_ItemInst Lua_Bot::GetBotItem(uint16 slot_id) {
	Lua_Safe_Call_Class(Lua_ItemInst);
	return self->GetBotItem(slot_id);
}

uint32 Lua_Bot::GetBotItemIDBySlot(uint16 slot_id) {
	Lua_Safe_Call_Int();
	return self->GetBotItemBySlot(slot_id);
}

void Lua_Bot::Signal(int signal_id) {
	Lua_Safe_Call_Void();
	self->Signal(signal_id);
}

void Lua_Bot::OwnerMessage(std::string message) {
	Lua_Safe_Call_Void();
	self->OwnerMessage(message);
}

int Lua_Bot::GetExpansionBitmask() {
	Lua_Safe_Call_Int();
	return self->GetExpansionBitmask();
}

void Lua_Bot::SetExpansionBitmask(int expansion_bitmask) {
	Lua_Safe_Call_Void();
	self->SetExpansionBitmask(expansion_bitmask);
}

bool Lua_Bot::ReloadBotDataBuckets() {
	Lua_Safe_Call_Bool();
	return self->LoadDataBucketsCache();;
}

bool Lua_Bot::ReloadBotOwnerDataBuckets() {
	Lua_Safe_Call_Bool();
	return self->HasOwner() && self->LoadDataBucketsCache();
}

bool Lua_Bot::ReloadBotSpells() {
	Lua_Safe_Call_Bool();
	return self->AI_AddBotSpells(self->GetBotSpellID());
}

void Lua_Bot::ReloadBotSpellSettings() {
	Lua_Safe_Call_Void();
	self->LoadBotSpellSettings();
}

bool Lua_Bot::HasBotSpellEntry(uint16 spellid) {
	Lua_Safe_Call_Bool();
	return self->HasBotSpellEntry(spellid);
}

void Lua_Bot::SendPayload(int payload_id) {
	Lua_Safe_Call_Void();
	self->SendPayload(payload_id);
}

void Lua_Bot::SendPayload(int payload_id, std::string payload_value) {
	Lua_Safe_Call_Void();
	self->SendPayload(payload_id, payload_value);
}

void Lua_Bot::ApplySpell(int spell_id) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id);
}

void Lua_Bot::ApplySpell(int spell_id, int duration) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration);
}

void Lua_Bot::ApplySpell(int spell_id, int duration, int level) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration, level, ApplySpellType::Solo);
}

void Lua_Bot::ApplySpell(int spell_id, int duration, int level, bool allow_pets) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration, level, ApplySpellType::Solo, allow_pets);
}

void Lua_Bot::ApplySpellGroup(int spell_id) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, 0, -1, ApplySpellType::Group);
}

void Lua_Bot::ApplySpellGroup(int spell_id, int duration) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration, -1, ApplySpellType::Group);
}
void Lua_Bot::ApplySpellGroup(int spell_id, int duration, int level) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration, level, ApplySpellType::Group);
}

void Lua_Bot::ApplySpellGroup(int spell_id, int duration, int level, bool allow_pets) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration, level, ApplySpellType::Group, allow_pets);
}

void Lua_Bot::ApplySpellRaid(int spell_id) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, 0, -1, ApplySpellType::Raid);
}

void Lua_Bot::ApplySpellRaid(int spell_id, int duration) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration, -1, ApplySpellType::Raid, true, true);
}

void Lua_Bot::ApplySpellRaid(int spell_id, int duration, int level) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration, level, ApplySpellType::Raid, true, true);
}

void Lua_Bot::ApplySpellRaid(int spell_id, int duration, int level, bool allow_pets) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration, level, ApplySpellType::Raid, allow_pets, true);
}

void Lua_Bot::ApplySpellRaid(int spell_id, int duration, int level, bool allow_pets, bool is_raid_group_only) {
	Lua_Safe_Call_Void();
	self->ApplySpell(spell_id, duration, level, ApplySpellType::Raid, allow_pets, is_raid_group_only);
}

void Lua_Bot::SetSpellDuration(int spell_id) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id);
}

void Lua_Bot::SetSpellDuration(int spell_id, int duration) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration);
}

void Lua_Bot::SetSpellDuration(int spell_id, int duration, int level) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration, level);
}

void Lua_Bot::SetSpellDuration(int spell_id, int duration, int level, bool allow_pets) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration, level, ApplySpellType::Solo, allow_pets);
}

void Lua_Bot::SetSpellDurationGroup(int spell_id) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, 0, -1, ApplySpellType::Group);
}

void Lua_Bot::SetSpellDurationGroup(int spell_id, int duration) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration, -1, ApplySpellType::Group);
}

void Lua_Bot::SetSpellDurationGroup(int spell_id, int duration, int level) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration, level, ApplySpellType::Group);
}

void Lua_Bot::SetSpellDurationGroup(int spell_id, int duration, int level, bool allow_pets) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration, level, ApplySpellType::Group, allow_pets);
}

void Lua_Bot::SetSpellDurationRaid(int spell_id) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, 0, -1, ApplySpellType::Raid);
}

void Lua_Bot::SetSpellDurationRaid(int spell_id, int duration) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration, -1, ApplySpellType::Raid);
}

void Lua_Bot::SetSpellDurationRaid(int spell_id, int duration, int level) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration, level, ApplySpellType::Raid);
}

void Lua_Bot::SetSpellDurationRaid(int spell_id, int duration, int level, bool allow_pets) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration, level, ApplySpellType::Raid, allow_pets);
}

void Lua_Bot::SetSpellDurationRaid(int spell_id, int duration, int level, bool allow_pets, bool is_raid_group_only) {
	Lua_Safe_Call_Void();
	self->SetSpellDuration(spell_id, duration, level, ApplySpellType::Raid, allow_pets, is_raid_group_only);
}

uint32 Lua_Bot::CountAugmentEquippedByID(uint32 item_id) {
	Lua_Safe_Call_Int();
	return self->GetInv().CountAugmentEquippedByID(item_id);
}

bool Lua_Bot::HasAugmentEquippedByID(uint32 item_id) {
	Lua_Safe_Call_Bool();
	return self->GetInv().HasAugmentEquippedByID(item_id);
}

uint32 Lua_Bot::CountItemEquippedByID(uint32 item_id) {
	Lua_Safe_Call_Int();
	return self->GetInv().CountItemEquippedByID(item_id);
}

bool Lua_Bot::HasItemEquippedByID(uint32 item_id) {
	Lua_Safe_Call_Bool();
	return self->GetInv().HasItemEquippedByID(item_id);
}

void Lua_Bot::Escape() {
	Lua_Safe_Call_Void();
	self->Escape();
}

void Lua_Bot::Fling(float target_x, float target_y, float target_z) {
	Lua_Safe_Call_Void();
	self->Fling(0, target_x, target_y, target_z, false, false, true);
}

void Lua_Bot::Fling(float target_x, float target_y, float target_z, bool ignore_los) {
	Lua_Safe_Call_Void();
	self->Fling(0, target_x, target_y, target_z, ignore_los, false, true);
}

void Lua_Bot::Fling(float target_x, float target_y, float target_z, bool ignore_los, bool clip_through_walls) {
	Lua_Safe_Call_Void();
	self->Fling(0, target_x, target_y, target_z, ignore_los, clip_through_walls, true);
}

void Lua_Bot::Fling(float value, float target_x, float target_y, float target_z) {
	Lua_Safe_Call_Void();
	self->Fling(value, target_x, target_y, target_z);
}

void Lua_Bot::Fling(float value, float target_x, float target_y, float target_z, bool ignore_los) {
	Lua_Safe_Call_Void();
	self->Fling(value, target_x, target_y, target_z, ignore_los);
}

void Lua_Bot::Fling(float value, float target_x, float target_y, float target_z, bool ignore_los, bool clip_through_walls) {
	Lua_Safe_Call_Void();
	self->Fling(value, target_x, target_y, target_z, ignore_los, clip_through_walls);
}

int Lua_Bot::GetBaseSTR() {
	Lua_Safe_Call_Int();
	return self->GetBaseSTR();
}

int Lua_Bot::GetBaseSTA() {
	Lua_Safe_Call_Int();
	return self->GetBaseSTA();
}

int Lua_Bot::GetBaseCHA() {
	Lua_Safe_Call_Int();
	return self->GetBaseCHA();
}

int Lua_Bot::GetBaseDEX() {
	Lua_Safe_Call_Int();
	return self->GetBaseDEX();
}

int Lua_Bot::GetBaseINT() {
	Lua_Safe_Call_Int();
	return self->GetBaseINT();
}

int Lua_Bot::GetBaseAGI() {
	Lua_Safe_Call_Int();
	return self->GetBaseAGI();
}

int Lua_Bot::GetBaseWIS() {
	Lua_Safe_Call_Int();
	return self->GetBaseWIS();
}

Lua_Group Lua_Bot::GetGroup() {
	Lua_Safe_Call_Class(Lua_Group);
	return self->GetGroup();
}

int Lua_Bot::GetHealAmount() {
	Lua_Safe_Call_Int();
	return self->GetHealAmt();
}

int Lua_Bot::GetSpellDamage() {
	Lua_Safe_Call_Int();
	return self->GetSpellDmg();
}

int Lua_Bot::GetInstrumentMod(int spell_id) {
	Lua_Safe_Call_Int();
	return self->GetInstrumentMod(spell_id);
}

int Lua_Bot::GetRawItemAC() {
	Lua_Safe_Call_Int();
	return self->GetRawItemAC();
}

void Lua_Bot::ClearDisciplineReuseTimer() {
	Lua_Safe_Call_Void();
	return self->ClearDisciplineReuseTimer();
}

void Lua_Bot::ClearDisciplineReuseTimer(uint16 spell_id) {
	Lua_Safe_Call_Void();
	return self->ClearDisciplineReuseTimer(spell_id);
}

void Lua_Bot::ClearItemReuseTimer() {
	Lua_Safe_Call_Void();
	return self->ClearItemReuseTimer();
}

void Lua_Bot::ClearItemReuseTimer(uint32 item_id) {
	Lua_Safe_Call_Void();
	return self->ClearItemReuseTimer(item_id);
}

void Lua_Bot::ClearSpellRecastTimer() {
	Lua_Safe_Call_Void();
	return self->ClearSpellRecastTimer();
}

void Lua_Bot::ClearSpellRecastTimer(uint16 spell_id) {
	Lua_Safe_Call_Void();
	return self->ClearSpellRecastTimer(spell_id);
}

uint32 Lua_Bot::GetDisciplineReuseTimer() {
	Lua_Safe_Call_Int();
	return self->GetDisciplineReuseRemainingTime();
}

uint32 Lua_Bot::GetDisciplineReuseTimer(uint16 spell_id) {
	Lua_Safe_Call_Int();
	return self->GetDisciplineReuseRemainingTime(spell_id);
}

uint32 Lua_Bot::GetItemReuseTimer() {
	Lua_Safe_Call_Int();
	return self->GetItemReuseRemainingTime();
}

uint32 Lua_Bot::GetItemReuseTimer(uint32 item_id) {
	Lua_Safe_Call_Int();
	return self->GetItemReuseRemainingTime(item_id);
}

uint32 Lua_Bot::GetSpellRecastTimer() {
	Lua_Safe_Call_Int();
	return self->GetSpellRecastRemainingTime();
}

uint32 Lua_Bot::GetSpellRecastTimer(uint16 spell_id) {
	Lua_Safe_Call_Int();
	return self->GetSpellRecastRemainingTime(spell_id);
}

void Lua_Bot::SetDisciplineReuseTimer(uint16 spell_id) {
	Lua_Safe_Call_Void();
	return self->SetDisciplineReuseTimer(spell_id);
}

void Lua_Bot::SetDisciplineReuseTimer(uint16 spell_id, uint32 reuse_timer) {
	Lua_Safe_Call_Void();
	return self->SetDisciplineReuseTimer(spell_id, reuse_timer);
}

void Lua_Bot::SetItemReuseTimer(uint32 item_id) {
	Lua_Safe_Call_Void();
	return self->SetItemReuseTimer(item_id);
}

void Lua_Bot::SetItemReuseTimer(uint32 item_id, uint32 reuse_timer) {
	Lua_Safe_Call_Void();
	return self->SetItemReuseTimer(item_id, reuse_timer);
}

void Lua_Bot::SetSpellRecastTimer(uint16 spell_id) {
	Lua_Safe_Call_Void();
	return self->SetSpellRecastTimer(spell_id);
}

void Lua_Bot::SetSpellRecastTimer(uint16 spell_id, uint32 recast_delay) {
	Lua_Safe_Call_Void();
	return self->SetSpellRecastTimer(spell_id, recast_delay);
}

bool Lua_Bot::IsGrouped() {
	Lua_Safe_Call_Bool();
	return self->IsGrouped();
}

bool Lua_Bot::IsStanding() {
	Lua_Safe_Call_Bool();
	return self->IsStanding();
}

bool Lua_Bot::IsSitting() {
	Lua_Safe_Call_Bool();
	return self->IsSitting();
}

void Lua_Bot::Sit() {
	Lua_Safe_Call_Void();
	self->Sit();
}

void Lua_Bot::Stand() {
	Lua_Safe_Call_Void();
	self->Stand();
}

uint32 Lua_Bot::GetBotID() {
	Lua_Safe_Call_Int();
	return self->GetBotID();
}

void Lua_Bot::Camp() {
	Lua_Safe_Call_Void();
	self->Camp();
}

void Lua_Bot::Camp(bool save_to_database) {
	Lua_Safe_Call_Void();
	self->Camp(save_to_database);
}

Lua_ItemInst Lua_Bot::GetAugmentAt(int16 slot_id, uint8 augment_index)
{
	Lua_Safe_Call_Class(Lua_ItemInst);

	auto* inst = self->GetInv().GetItem(slot_id);
	if (inst) {
		return Lua_ItemInst(inst->GetAugment(augment_index));
	}

	return Lua_ItemInst();
}

int Lua_Bot::GetAugmentIDAt(int16 slot_id, uint8 augment_index) {
	Lua_Safe_Call_Int();
	return self->GetAugmentIDAt(slot_id, augment_index);
}

int Lua_Bot::GetItemIDAt(int16 slot_id) {
	Lua_Safe_Call_Int();
	return self->GetItemIDAt(slot_id);
}

Lua_ItemInst Lua_Bot::GetItemAt(int16 slot_id) // @categories Inventory and Items
{
	Lua_Safe_Call_Class(Lua_ItemInst);
	return Lua_ItemInst(self->GetInv().GetItem(slot_id));
}

void Lua_Bot::SendSpellAnim(uint16 target_id, uint16 spell_id)
{
	Lua_Safe_Call_Void();
	self->SendSpellAnim(target_id, spell_id);
}

luabind::object Lua_Bot::GetAugmentIDsBySlotID(lua_State* L, int16 slot_id) const {
	auto lua_table = luabind::newtable(L);
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto augments = self->GetInv().GetAugmentIDsBySlotID(slot_id);
		int index = 1;
		for (auto item_id : augments) {
			lua_table[index] = item_id;
			index++;
		}
	}
	return lua_table;
}

void Lua_Bot::AddItem(const luabind::object& item_table) {
	Lua_Safe_Call_Void();
	if (luabind::type(item_table) != LUA_TTABLE) {
		return;
	}

	auto item_id = luabind::object_cast<uint32>(item_table["item_id"]);
	int16 charges = luabind::object_cast<uint32>(item_table["charges"]);
	uint32 augment_one = luabind::type(item_table["augment_one"]) != LUA_TNIL ? luabind::object_cast<uint32>(item_table["augment_one"]) : 0;
	uint32 augment_two = luabind::type(item_table["augment_two"]) != LUA_TNIL ? luabind::object_cast<uint32>(item_table["augment_two"]) : 0;
	uint32 augment_three = luabind::type(item_table["augment_three"]) != LUA_TNIL ? luabind::object_cast<uint32>(item_table["augment_three"]) : 0;
	uint32 augment_four = luabind::type(item_table["augment_four"]) != LUA_TNIL ? luabind::object_cast<uint32>(item_table["augment_four"]) : 0;
	uint32 augment_five = luabind::type(item_table["augment_five"]) != LUA_TNIL ? luabind::object_cast<uint32>(item_table["augment_five"]) : 0;
	uint32 augment_six = luabind::type(item_table["augment_six"]) != LUA_TNIL ? luabind::object_cast<uint32>(item_table["augment_six"]) : 0;
	bool attuned = luabind::type(item_table["attuned"]) != LUA_TNIL ? luabind::object_cast<bool>(item_table["attuned"]) : false;
	uint16 slot_id = luabind::type(item_table["slot_id"]) != LUA_TNIL ? luabind::object_cast<uint16>(item_table["slot_id"]) : EQ::invslot::slotCursor;

	if (slot_id <= EQ::invslot::slotAmmo) {
		self->AddBotItem(
			slot_id,
			item_id,
			charges,
			attuned,
			augment_one,
			augment_two,
			augment_three,
			augment_four,
			augment_five,
			augment_six
		);
	} else {
		self->GetOwner()->CastToClient()->SummonItem(
			item_id,
			charges,
			augment_one,
			augment_two,
			augment_three,
			augment_four,
			augment_five,
			augment_six,
			attuned,
			slot_id
		);
	}
}

std::string Lua_Bot::GetClassAbbreviation() {
	Lua_Safe_Call_String();
	return GetPlayerClassAbbreviation(self->GetClass());
}

std::string Lua_Bot::GetRaceAbbreviation() {
	Lua_Safe_Call_String();
	return GetPlayerRaceAbbreviation(self->GetBaseRace());
}

void Lua_Bot::DeleteBucket(std::string bucket_name)
{
	Lua_Safe_Call_Void();
	self->DeleteBucket(bucket_name);
}

std::string Lua_Bot::GetBucket(std::string bucket_name)
{
	Lua_Safe_Call_String();
	return self->GetBucket(bucket_name);
}

std::string Lua_Bot::GetBucketExpires(std::string bucket_name)
{
	Lua_Safe_Call_String();
	return self->GetBucketExpires(bucket_name);
}

std::string Lua_Bot::GetBucketRemaining(std::string bucket_name)
{
	Lua_Safe_Call_String();
	return self->GetBucketRemaining(bucket_name);
}

void Lua_Bot::SetBucket(std::string bucket_name, std::string bucket_value)
{
	Lua_Safe_Call_Void();
	self->SetBucket(bucket_name, bucket_value);
}

void Lua_Bot::SetBucket(std::string bucket_name, std::string bucket_value, std::string expiration)
{
	Lua_Safe_Call_Void();
	self->SetBucket(bucket_name, bucket_value, expiration);
}

void Lua_Bot::DeleteBot() {
	Lua_Safe_Call_Void();
	self->DeleteBot();
}

void Lua_Bot::RaidGroupSay(const char* message) {
	Lua_Safe_Call_Void();
	self->RaidGroupSay(message);
}

luabind::scope lua_register_bot() {
	return luabind::class_<Lua_Bot, Lua_Mob>("Bot")
	.def(luabind::constructor<>())
	.def("AddBotItem", (void(Lua_Bot::*)(uint16,uint32))&Lua_Bot::AddBotItem)
	.def("AddBotItem", (void(Lua_Bot::*)(uint16,uint32,int16))&Lua_Bot::AddBotItem)
	.def("AddBotItem", (void(Lua_Bot::*)(uint16,uint32,int16,bool))&Lua_Bot::AddBotItem)
	.def("AddBotItem", (void(Lua_Bot::*)(uint16,uint32,int16,bool,uint32))&Lua_Bot::AddBotItem)
	.def("AddBotItem", (void(Lua_Bot::*)(uint16,uint32,int16,bool,uint32,uint32))&Lua_Bot::AddBotItem)
	.def("AddBotItem", (void(Lua_Bot::*)(uint16,uint32,int16,bool,uint32,uint32,uint32))&Lua_Bot::AddBotItem)
	.def("AddBotItem", (void(Lua_Bot::*)(uint16,uint32,int16,bool,uint32,uint32,uint32,uint32))&Lua_Bot::AddBotItem)
	.def("AddBotItem", (void(Lua_Bot::*)(uint16,uint32,int16,bool,uint32,uint32,uint32,uint32,uint32))&Lua_Bot::AddBotItem)
	.def("AddBotItem", (void(Lua_Bot::*)(uint16,uint32,int16,bool,uint32,uint32,uint32,uint32,uint32,uint32))&Lua_Bot::AddBotItem)
	.def("AddItem", (void(Lua_Bot::*)(luabind::adl::object))&Lua_Bot::AddItem)
	.def("ApplySpell", (void(Lua_Bot::*)(int))&Lua_Bot::ApplySpell)
	.def("ApplySpell", (void(Lua_Bot::*)(int,int))&Lua_Bot::ApplySpell)
	.def("ApplySpell", (void(Lua_Bot::*)(int,int,int))&Lua_Bot::ApplySpell)
	.def("ApplySpell", (void(Lua_Bot::*)(int,int,int,bool))&Lua_Bot::ApplySpell)
	.def("ApplySpell", (void(Lua_Bot::*)(int,int,int,bool))&Lua_Bot::ApplySpell)
	.def("ApplySpellGroup", (void(Lua_Bot::*)(int))&Lua_Bot::ApplySpellGroup)
	.def("ApplySpellGroup", (void(Lua_Bot::*)(int,int))&Lua_Bot::ApplySpellGroup)
	.def("ApplySpellGroup", (void(Lua_Bot::*)(int,int,int))&Lua_Bot::ApplySpellGroup)
	.def("ApplySpellGroup", (void(Lua_Bot::*)(int,int,int,bool))&Lua_Bot::ApplySpellGroup)
	.def("ApplySpellGroup", (void(Lua_Bot::*)(int,int,int,bool))&Lua_Bot::ApplySpellGroup)
	.def("ApplySpellRaid", (void(Lua_Bot::*)(int))&Lua_Bot::ApplySpellRaid)
	.def("ApplySpellRaid", (void(Lua_Bot::*)(int,int))&Lua_Bot::ApplySpellRaid)
	.def("ApplySpellRaid", (void(Lua_Bot::*)(int,int,int))&Lua_Bot::ApplySpellRaid)
	.def("ApplySpellRaid", (void(Lua_Bot::*)(int,int,int,bool))&Lua_Bot::ApplySpellRaid)
	.def("ApplySpellRaid", (void(Lua_Bot::*)(int,int,int,bool,bool))&Lua_Bot::ApplySpellRaid)
	.def("ApplySpellRaid", (void(Lua_Bot::*)(int,int,int,bool,bool))&Lua_Bot::ApplySpellRaid)
	.def("RaidGroupSay", (void(Lua_Bot::*)(const char*))&Lua_Bot::RaidGroupSay)
	.def("Camp", (void(Lua_Bot::*)(void))&Lua_Bot::Camp)
	.def("Camp", (void(Lua_Bot::*)(bool))&Lua_Bot::Camp)
	.def("ClearDisciplineReuseTimer", (void(Lua_Bot::*)())&Lua_Bot::ClearDisciplineReuseTimer)
	.def("ClearDisciplineReuseTimer", (void(Lua_Bot::*)(uint16))&Lua_Bot::ClearDisciplineReuseTimer)
	.def("ClearItemReuseTimer", (void(Lua_Bot::*)())&Lua_Bot::ClearItemReuseTimer)
	.def("ClearItemReuseTimer", (void(Lua_Bot::*)(uint32))&Lua_Bot::ClearItemReuseTimer)
	.def("ClearSpellRecastTimer", (void(Lua_Bot::*)())&Lua_Bot::ClearSpellRecastTimer)
	.def("ClearSpellRecastTimer", (void(Lua_Bot::*)(uint16))&Lua_Bot::ClearSpellRecastTimer)
	.def("CountAugmentEquippedByID", (uint32(Lua_Bot::*)(uint32))&Lua_Bot::CountAugmentEquippedByID)
	.def("CountBotItem", (uint32(Lua_Bot::*)(uint32))&Lua_Bot::CountBotItem)
	.def("CountItemEquippedByID", (uint32(Lua_Bot::*)(uint32))&Lua_Bot::CountItemEquippedByID)
	.def("DeleteBot", (void(Lua_Bot::*)(void))&Lua_Bot::DeleteBot)
	.def("DeleteBucket", (void(Lua_Bot::*)(std::string))&Lua_Bot::DeleteBucket)
	.def("Escape", (void(Lua_Bot::*)(void))&Lua_Bot::Escape)
	.def("Fling", (void(Lua_Bot::*)(float,float,float))&Lua_Bot::Fling)
	.def("Fling", (void(Lua_Bot::*)(float,float,float,bool))&Lua_Bot::Fling)
	.def("Fling", (void(Lua_Bot::*)(float,float,float,bool,bool))&Lua_Bot::Fling)
	.def("Fling", (void(Lua_Bot::*)(float,float,float,float))&Lua_Bot::Fling)
	.def("Fling", (void(Lua_Bot::*)(float,float,float,float,bool))&Lua_Bot::Fling)
	.def("Fling", (void(Lua_Bot::*)(float,float,float,float,bool,bool))&Lua_Bot::Fling)
	.def("GetAugmentAt", (Lua_ItemInst(Lua_Bot::*)(int16,uint8))&Lua_Bot::GetAugmentAt)
	.def("GetAugmentIDAt", (int(Lua_Bot::*)(int16,uint8))&Lua_Bot::GetAugmentIDAt)
	.def("GetAugmentIDsBySlotID", (luabind::object(Lua_Bot::*)(lua_State* L,int16))&Lua_Bot::GetAugmentIDsBySlotID)
	.def("GetBaseAGI", (int(Lua_Bot::*)(void))&Lua_Bot::GetBaseAGI)
	.def("GetBaseCHA", (int(Lua_Bot::*)(void))&Lua_Bot::GetBaseCHA)
	.def("GetBaseDEX", (int(Lua_Bot::*)(void))&Lua_Bot::GetBaseDEX)
	.def("GetBaseINT", (int(Lua_Bot::*)(void))&Lua_Bot::GetBaseINT)
	.def("GetBaseSTA", (int(Lua_Bot::*)(void))&Lua_Bot::GetBaseSTA)
	.def("GetBaseSTR", (int(Lua_Bot::*)(void))&Lua_Bot::GetBaseSTR)
	.def("GetBaseWIS", (int(Lua_Bot::*)(void))&Lua_Bot::GetBaseWIS)
	.def("GetBotID", (uint32(Lua_Bot::*)(void))&Lua_Bot::GetBotID)
	.def("GetBotItem", (Lua_ItemInst(Lua_Bot::*)(uint16))&Lua_Bot::GetBotItem)
	.def("GetBotItemIDBySlot", (uint32(Lua_Bot::*)(uint16))&Lua_Bot::GetBotItemIDBySlot)
	.def("GetDisciplineReuseTimer", (uint32(Lua_Bot::*)())&Lua_Bot::GetDisciplineReuseTimer)
	.def("GetDisciplineReuseTimer", (uint32(Lua_Bot::*)(uint16))&Lua_Bot::GetDisciplineReuseTimer)
	.def("GetBucket", (std::string(Lua_Bot::*)(std::string))&Lua_Bot::GetBucket)
	.def("GetBucketExpires", (std::string(Lua_Bot::*)(std::string))&Lua_Bot::GetBucketExpires)
	.def("GetBucketRemaining", (std::string(Lua_Bot::*)(std::string))&Lua_Bot::GetBucketRemaining)
	.def("GetClassAbbreviation", (std::string(Lua_Bot::*)(void))&Lua_Bot::GetClassAbbreviation)
	.def("GetExpansionBitmask", (int(Lua_Bot::*)(void))&Lua_Bot::GetExpansionBitmask)
	.def("GetGroup", (Lua_Group(Lua_Bot::*)(void))&Lua_Bot::GetGroup)
	.def("GetHealAmount", (int(Lua_Bot::*)(void))&Lua_Bot::GetHealAmount)
	.def("GetInstrumentMod", (int(Lua_Bot::*)(int))&Lua_Bot::GetInstrumentMod)
	.def("GetItemAt", (Lua_ItemInst(Lua_Bot::*)(int16))&Lua_Bot::GetItemAt)
	.def("GetItemIDAt", (int(Lua_Bot::*)(int16))&Lua_Bot::GetItemIDAt)
	.def("GetItemReuseTimer", (uint32(Lua_Bot::*)())&Lua_Bot::GetItemReuseTimer)
	.def("GetItemReuseTimer", (uint32(Lua_Bot::*)(uint32))&Lua_Bot::GetItemReuseTimer)
	.def("GetOwner", (Lua_Mob(Lua_Bot::*)(void))&Lua_Bot::GetOwner)
	.def("GetRaceAbbreviation", (std::string(Lua_Bot::*)(void))&Lua_Bot::GetRaceAbbreviation)
	.def("GetRawItemAC", (int(Lua_Bot::*)(void))&Lua_Bot::GetRawItemAC)
	.def("GetSpellDamage", (int(Lua_Bot::*)(void))&Lua_Bot::GetSpellDamage)
	.def("GetSpellRecastTimer", (uint32(Lua_Bot::*)())&Lua_Bot::GetSpellRecastTimer)
	.def("GetSpellRecastTimer", (uint32(Lua_Bot::*)(uint16))&Lua_Bot::GetSpellRecastTimer)
	.def("HasAugmentEquippedByID", (bool(Lua_Bot::*)(uint32))&Lua_Bot::HasAugmentEquippedByID)
	.def("HasBotItem", (int16(Lua_Bot::*)(uint32))&Lua_Bot::HasBotItem)
	.def("HasBotSpellEntry", (bool(Lua_Bot::*)(uint16))&Lua_Bot::HasBotSpellEntry)
	.def("HasItemEquippedByID", (bool(Lua_Bot::*)(uint32))&Lua_Bot::HasItemEquippedByID)
	.def("IsGrouped", (bool(Lua_Bot::*)(void))&Lua_Bot::IsGrouped)
	.def("IsSitting", (bool(Lua_Bot::*)(void))&Lua_Bot::IsSitting)
	.def("IsStanding", (bool(Lua_Bot::*)(void))&Lua_Bot::IsStanding)
	.def("OwnerMessage", (void(Lua_Bot::*)(std::string))&Lua_Bot::OwnerMessage)
	.def("ReloadBotDataBuckets", (bool(Lua_Bot::*)(void))&Lua_Bot::ReloadBotDataBuckets)
	.def("ReloadBotOwnerDataBuckets", (bool(Lua_Bot::*)(void))&Lua_Bot::ReloadBotOwnerDataBuckets)
	.def("ReloadBotSpells", (bool(Lua_Bot::*)(void))&Lua_Bot::ReloadBotSpells)
	.def("ReloadBotSpellSettings", (void(Lua_Bot::*)(void))&Lua_Bot::ReloadBotSpellSettings)
	.def("RemoveBotItem", (void(Lua_Bot::*)(uint32))&Lua_Bot::RemoveBotItem)
	.def("SendSpellAnim", (void(Lua_Bot::*)(uint16,uint16))&Lua_Bot::SendSpellAnim)
	.def("SetBucket", (void(Lua_Bot::*)(std::string,std::string))&Lua_Bot::SetBucket)
	.def("SetBucket", (void(Lua_Bot::*)(std::string,std::string,std::string))&Lua_Bot::SetBucket)
	.def("SetExpansionBitmask", (void(Lua_Bot::*)(int))&Lua_Bot::SetExpansionBitmask)
	.def("SetDisciplineReuseTimer", (void(Lua_Bot::*)(uint16))&Lua_Bot::SetDisciplineReuseTimer)
	.def("SetDisciplineReuseTimer", (void(Lua_Bot::*)(uint16, uint32))&Lua_Bot::SetDisciplineReuseTimer)
	.def("SetItemReuseTimer", (void(Lua_Bot::*)(uint32))&Lua_Bot::SetItemReuseTimer)
	.def("SetItemReuseTimer", (void(Lua_Bot::*)(uint32, uint32))&Lua_Bot::SetItemReuseTimer)
	.def("SetSpellDuration", (void(Lua_Bot::*)(int))&Lua_Bot::SetSpellDuration)
	.def("SetSpellDuration", (void(Lua_Bot::*)(int,int))&Lua_Bot::SetSpellDuration)
	.def("SetSpellDuration", (void(Lua_Bot::*)(int,int,int))&Lua_Bot::SetSpellDuration)
	.def("SetSpellDuration", (void(Lua_Bot::*)(int,int,int,bool))&Lua_Bot::SetSpellDuration)
	.def("SetSpellDurationGroup", (void(Lua_Bot::*)(int))&Lua_Bot::SetSpellDurationGroup)
	.def("SetSpellDurationGroup", (void(Lua_Bot::*)(int,int))&Lua_Bot::SetSpellDurationGroup)
	.def("SetSpellDurationGroup", (void(Lua_Bot::*)(int,int,int))&Lua_Bot::SetSpellDurationGroup)
	.def("SetSpellDurationGroup", (void(Lua_Bot::*)(int,int,int,bool))&Lua_Bot::SetSpellDurationGroup)
	.def("SetSpellDurationRaid", (void(Lua_Bot::*)(int))&Lua_Bot::SetSpellDurationRaid)
	.def("SetSpellDurationRaid", (void(Lua_Bot::*)(int,int))&Lua_Bot::SetSpellDurationRaid)
	.def("SetSpellDurationRaid", (void(Lua_Bot::*)(int,int,int))&Lua_Bot::SetSpellDurationRaid)
	.def("SetSpellDurationRaid", (void(Lua_Bot::*)(int,int,int,bool))&Lua_Bot::SetSpellDurationRaid)
	.def("SetSpellDurationRaid", (void(Lua_Bot::*)(int,int,int,bool,bool))&Lua_Bot::SetSpellDurationRaid)
	.def("SetSpellRecastTimer", (void(Lua_Bot::*)(uint16))&Lua_Bot::SetSpellRecastTimer)
	.def("SetSpellRecastTimer", (void(Lua_Bot::*)(uint16, uint32))&Lua_Bot::SetSpellRecastTimer)
	.def("SendPayload", (void(Lua_Bot::*)(int))&Lua_Bot::SendPayload)
	.def("SendPayload", (void(Lua_Bot::*)(int,std::string))&Lua_Bot::SendPayload)
	.def("Signal", (void(Lua_Bot::*)(int))&Lua_Bot::Signal)
	.def("Sit", (void(Lua_Bot::*)(void))&Lua_Bot::Sit)
	.def("Stand", (void(Lua_Bot::*)(void))&Lua_Bot::Stand);
}

#endif // LUA_EQEMU
