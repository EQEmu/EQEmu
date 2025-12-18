#ifdef LUA_EQEMU

#include "lua_group.h"

#include "common/data_verification.h"
#include "zone/groups.h"
#include "zone/lua_client.h"
#include "zone/lua_mob.h"
#include "zone/lua_npc.h"
#include "zone/masterentity.h"

#include "luabind/luabind.hpp"
#include "luabind/object.hpp"

void Lua_Group::DisbandGroup() {
	Lua_Safe_Call_Void();
	self->DisbandGroup();
}

bool Lua_Group::IsGroupMember(const char* name) {
	Lua_Safe_Call_Bool();
	return self->IsGroupMember(name);
}

bool Lua_Group::IsGroupMember(Lua_Mob c) {
	Lua_Safe_Call_Bool();
	return self->IsGroupMember(c);
}

void Lua_Group::CastGroupSpell(Lua_Mob caster, int spell_id) {
	Lua_Safe_Call_Void();
	self->CastGroupSpell(caster, spell_id);
}

void Lua_Group::SplitExp(uint64 exp, Lua_Mob other) {
	Lua_Safe_Call_Void();
	self->SplitExp(ExpSource::Quest, exp, other);
}

void Lua_Group::GroupMessage(Lua_Mob sender, const char* message) {
	Lua_Safe_Call_Void();
	self->GroupMessage(sender, Language::CommonTongue, Language::MaxValue, message);
}

void Lua_Group::GroupMessage(Lua_Mob sender, uint8 language, const char* message) {
	Lua_Safe_Call_Void();
	self->GroupMessage(sender, language, Language::MaxValue, message);
}

uint32 Lua_Group::GetTotalGroupDamage(Lua_Mob other) {
	Lua_Safe_Call_Int();
	return self->GetTotalGroupDamage(other);
}

void Lua_Group::SplitMoney(uint32 copper, uint32 silver, uint32 gold, uint32 platinum) {
	Lua_Safe_Call_Void();
	self->SplitMoney(copper, silver, gold, platinum);
}

void Lua_Group::SplitMoney(uint32 copper, uint32 silver, uint32 gold, uint32 platinum, Lua_Client splitter) {
	Lua_Safe_Call_Void();
	self->SplitMoney(copper, silver, gold, platinum, splitter);
}

void Lua_Group::SetLeader(Lua_Mob c) {
	Lua_Safe_Call_Void();
	self->SetLeader(c);
}

Lua_Mob Lua_Group::GetLeader() {
	Lua_Safe_Call_Class(Lua_Mob);
	return self->GetLeader();
}

std::string Lua_Group::GetLeaderName() {
	Lua_Safe_Call_String();
	return self->GetLeaderName();
}

bool Lua_Group::IsLeader(const char* name) {
	Lua_Safe_Call_Bool();
	return self->IsLeader(name);
}

bool Lua_Group::IsLeader(Lua_Mob c) {
	Lua_Safe_Call_Bool();
	return self->IsLeader(c);
}

int Lua_Group::GroupCount() {
	Lua_Safe_Call_Int();
	return self->GroupCount();
}

uint32 Lua_Group::GetHighestLevel() {
	Lua_Safe_Call_Int();
	return self->GetHighestLevel();
}

uint32 Lua_Group::GetLowestLevel() {
	Lua_Safe_Call_Int();
	return self->GetLowestLevel();
}

void Lua_Group::TeleportGroup(Lua_Mob sender, uint32 zone_id, uint32 instance_id, float x, float y, float z, float h) {
	Lua_Safe_Call_Void();
	self->TeleportGroup(sender, zone_id, instance_id, x, y, z, h);
}

int Lua_Group::GetID() {
	Lua_Safe_Call_Int();
	return self->GetID();
}

Lua_Mob Lua_Group::GetMember(int member_index) {
	Lua_Safe_Call_Class(Lua_Mob);

	if (!EQ::ValueWithin(member_index, 0, 5)) {
		return Lua_Mob();
	}

	return self->members[member_index];
}

uint8 Lua_Group::GetMemberRole(Lua_Mob member) {
	Lua_Safe_Call_Int();
	return self->GetMemberRole(member);
}

uint8 Lua_Group::GetMemberRole(const char* name) {
	Lua_Safe_Call_Int();
	return self->GetMemberRole(name);
}

bool Lua_Group::DoesAnyMemberHaveExpeditionLockout(std::string expedition_name, std::string event_name)
{
	Lua_Safe_Call_Bool();
	return self->AnyMemberHasDzLockout(expedition_name, event_name);
}

bool Lua_Group::DoesAnyMemberHaveExpeditionLockout(std::string expedition_name, std::string event_name, int max_check_count)
{
	Lua_Safe_Call_Bool();
	return self->AnyMemberHasDzLockout(expedition_name, event_name); // max_check_count deprecated
}

uint32 Lua_Group::GetAverageLevel() {
	Lua_Safe_Call_Int();
	return self->GetAvgLevel();
}

luabind::scope lua_register_group() {
	return luabind::class_<Lua_Group>("Group")
	.def(luabind::constructor<>())
	.property("null", &Lua_Group::Null)
	.property("valid", &Lua_Group::Valid)
	.def("CastGroupSpell", (void(Lua_Group::*)(Lua_Mob,int))&Lua_Group::CastGroupSpell)
	.def("DisbandGroup", (void(Lua_Group::*)(void))&Lua_Group::DisbandGroup)
	.def("DoesAnyMemberHaveExpeditionLockout", (bool(Lua_Group::*)(std::string, std::string))&Lua_Group::DoesAnyMemberHaveExpeditionLockout)
	.def("DoesAnyMemberHaveExpeditionLockout", (bool(Lua_Group::*)(std::string, std::string, int))&Lua_Group::DoesAnyMemberHaveExpeditionLockout)
	.def("GetAverageLevel", (uint32(Lua_Group::*)(void))&Lua_Group::GetAverageLevel)
	.def("GetHighestLevel", (uint32(Lua_Group::*)(void))&Lua_Group::GetHighestLevel)
	.def("GetID", (int(Lua_Group::*)(void))&Lua_Group::GetID)
	.def("GetLeader", (Lua_Mob(Lua_Group::*)(void))&Lua_Group::GetLeader)
	.def("GetLeaderName", (const char*(Lua_Group::*)(void))&Lua_Group::GetLeaderName)
	.def("GetLowestLevel", (uint32(Lua_Group::*)(void))&Lua_Group::GetLowestLevel)
	.def("GetMember", (Lua_Mob(Lua_Group::*)(int))&Lua_Group::GetMember)
	.def("GetMemberRole", (uint8(Lua_Group::*)(Lua_Mob))&Lua_Group::GetMemberRole)
	.def("GetMemberRole", (uint8(Lua_Group::*)(const char*))&Lua_Group::GetMemberRole)
	.def("GetTotalGroupDamage", (uint32(Lua_Group::*)(Lua_Mob))&Lua_Group::GetTotalGroupDamage)
	.def("GroupCount", (int(Lua_Group::*)(void))&Lua_Group::GroupCount)
	.def("GroupMessage", (void(Lua_Group::*)(Lua_Mob,const char*))&Lua_Group::GroupMessage)
	.def("GroupMessage", (void(Lua_Group::*)(Lua_Mob,uint8,const char*))&Lua_Group::GroupMessage)
	.def("IsGroupMember", (bool(Lua_Group::*)(const char*))&Lua_Group::IsGroupMember)
	.def("IsGroupMember", (bool(Lua_Group::*)(Lua_Mob))&Lua_Group::IsGroupMember)
	.def("IsLeader", (bool(Lua_Group::*)(const char*))&Lua_Group::IsLeader)
	.def("IsLeader", (bool(Lua_Group::*)(Lua_Mob))&Lua_Group::IsLeader)
	.def("SetLeader", (void(Lua_Group::*)(Lua_Mob))&Lua_Group::SetLeader)
	.def("SplitExp", (void(Lua_Group::*)(uint32,Lua_Mob))&Lua_Group::SplitExp)
	.def("SplitMoney", (void(Lua_Group::*)(uint32,uint32,uint32,uint32))&Lua_Group::SplitMoney)
	.def("SplitMoney", (void(Lua_Group::*)(uint32,uint32,uint32,uint32,Lua_Client))&Lua_Group::SplitMoney)
	.def("TeleportGroup", (void(Lua_Group::*)(Lua_Mob,uint32,uint32,float,float,float,float))&Lua_Group::TeleportGroup);
}

#endif // LUA_EQEMU
