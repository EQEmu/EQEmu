#ifdef LUA_EQEMU

#include "lua_door.h"

#include "zone/doors.h"
#include "zone/lua_mob.h"

#include "luabind/luabind.hpp"
#include "luabind/object.hpp"


void Lua_Door::SetDoorName(const char *name) {
	Lua_Safe_Call_Void();
	self->SetDoorName(name);
}

const char *Lua_Door::GetDoorName() {
	Lua_Safe_Call_String();
	return self->GetDoorName();
}

float Lua_Door::GetX() {
	Lua_Safe_Call_Real();
	return self->GetPosition().x;
}

float Lua_Door::GetY() {
	Lua_Safe_Call_Real();
	return self->GetPosition().y;
}

float Lua_Door::GetZ() {
	Lua_Safe_Call_Real();
	return self->GetPosition().z;
}

float Lua_Door::GetHeading() {
	Lua_Safe_Call_Real();
	return self->GetPosition().w;
}

void Lua_Door::SetX(float x) {
	Lua_Safe_Call_Void();
	auto position = self->GetPosition();
	position.x = x;
	self->SetPosition(position);
}

void Lua_Door::SetY(float y) {
	Lua_Safe_Call_Void();
	auto position = self->GetPosition();
	position.y = y;
	self->SetPosition(position);
}

void Lua_Door::SetZ(float z) {
	Lua_Safe_Call_Void();
	auto position = self->GetPosition();
	position.z = z;
	self->SetPosition(position);
}

void Lua_Door::SetHeading(float h) {
	Lua_Safe_Call_Void();
	auto position = self->GetPosition();
	position.w = h;
	self->SetPosition(position);
}

void Lua_Door::SetLocation(float x, float y, float z) {
	Lua_Safe_Call_Void();
	self->SetLocation(x, y, z);
}

uint32 Lua_Door::GetDoorDBID() {
	Lua_Safe_Call_Int();
	return self->GetDoorDBID();
}

uint32 Lua_Door::GetDoorID() {
	Lua_Safe_Call_Int();
	return self->GetDoorID();
}

void Lua_Door::SetSize(uint32 sz) {
	Lua_Safe_Call_Void();
	self->SetSize(sz);
}

uint32 Lua_Door::GetSize() {
	Lua_Safe_Call_Int();
	return self->GetSize();
}

void Lua_Door::SetIncline(uint32 incline) {
	Lua_Safe_Call_Void();
	self->SetIncline(incline);
}

uint32 Lua_Door::GetIncline() {
	Lua_Safe_Call_Int();
	return self->GetIncline();
}

void Lua_Door::SetOpenType(uint32 type) {
	Lua_Safe_Call_Void();
	self->SetOpenType(type);
}

uint32 Lua_Door::GetOpenType() {
	Lua_Safe_Call_Int();
	return self->GetOpenType();
}

void Lua_Door::SetDisableTimer(bool flag) {
	Lua_Safe_Call_Void();
	self->SetDisableTimer(flag);
}

bool Lua_Door::GetDisableTimer() {
	Lua_Safe_Call_Bool();
	return self->GetDisableTimer();
}

void Lua_Door::SetLockPick(uint32 pick) {
	Lua_Safe_Call_Void();
	self->SetLockpick(pick);
}

uint32 Lua_Door::GetLockPick() {
	Lua_Safe_Call_Int();
	return self->GetLockpick();
}

void Lua_Door::SetKeyItem(uint32 key) {
	Lua_Safe_Call_Void();
	self->SetKeyItem(key);
}

uint32 Lua_Door::GetKeyItem() {
	Lua_Safe_Call_Int();
	return self->GetKeyItem();
}

void Lua_Door::SetNoKeyring(int type) {
	Lua_Safe_Call_Void();
	self->SetNoKeyring(type);
}

int Lua_Door::GetNoKeyring() {
	Lua_Safe_Call_Int();
	return self->GetNoKeyring();
}

void Lua_Door::CreateDatabaseEntry() {
	Lua_Safe_Call_Void();
	self->CreateDatabaseEntry();
}

void Lua_Door::ForceOpen(Lua_Mob sender) {
	Lua_Safe_Call_Void();
	self->ForceOpen(sender);
}

void Lua_Door::ForceOpen(Lua_Mob sender, bool alt_mode) {
	Lua_Safe_Call_Void();
	self->ForceOpen(sender, alt_mode);
}

void Lua_Door::ForceClose(Lua_Mob sender) {
	Lua_Safe_Call_Void();
	self->ForceClose(sender);
}

void Lua_Door::ForceClose(Lua_Mob sender, bool alt_mode) {
	Lua_Safe_Call_Void();
	self->ForceClose(sender, alt_mode);
}

uint32 Lua_Door::GetID() {
	Lua_Safe_Call_Int();
	return self->GetID();
}

uint8 Lua_Door::GetTriggerDoorID() {
	Lua_Safe_Call_Int();
	return self->GetTriggerDoorID();
}

uint8 Lua_Door::GetTriggerType() {
	Lua_Safe_Call_Int();
	return self->GetTriggerType();
}

bool Lua_Door::IsLDoNDoor() {
	Lua_Safe_Call_Bool();
	return self->IsLDoNDoor();
}

uint32 Lua_Door::GetClientVersionMask() {
	Lua_Safe_Call_Int();
	return self->GetClientVersionMask();
}

int Lua_Door::GetDoorParam() {
	Lua_Safe_Call_Int();
	return self->GetDoorParam();
}

bool Lua_Door::HasDestinationZone() {
	Lua_Safe_Call_Bool();
	return self->HasDestinationZone();
}

bool Lua_Door::IsDestinationZoneSame() {
	Lua_Safe_Call_Bool();
	return self->IsDestinationZoneSame();
}

bool Lua_Door::IsDoorBlacklisted() {
	Lua_Safe_Call_Bool();
	return self->IsDoorBlacklisted();
}

std::string Lua_Door::GetDestinationZoneName() {
	Lua_Safe_Call_String();
	return self->GetDestinationZoneName();
}

int Lua_Door::GetDestinationInstanceID() {
	Lua_Safe_Call_Int();
	return self->GetDestinationInstanceID();
}

float Lua_Door::GetDestinationX() {
	Lua_Safe_Call_Real();
	return self->GetDestinationX();
}

float Lua_Door::GetDestinationY() {
	Lua_Safe_Call_Real();
	return self->GetDestinationY();
}

float Lua_Door::GetDestinationZ() {
	Lua_Safe_Call_Real();
	return self->GetDestinationZ();
}

float Lua_Door::GetDestinationHeading() {
	Lua_Safe_Call_Real();
	return self->GetDestinationHeading();
}

int Lua_Door::GetDzSwitchID() {
	Lua_Safe_Call_Int();
	return self->GetDzSwitchID();
}

int Lua_Door::GetInvertState() {
	Lua_Safe_Call_Int();
	return self->GetInvertState();
}

void Lua_Door::SetInvertState(int invert_state) {
	Lua_Safe_Call_Void();
	self->SetInvertState(invert_state);
}

uint32 Lua_Door::GetGuildID() {
	Lua_Safe_Call_Int();
	return self->GetGuildID();
}

luabind::scope lua_register_door() {
	return luabind::class_<Lua_Door, Lua_Entity>("Door")
	.def(luabind::constructor<>())
	.property("null", &Lua_Door::Null)
	.property("valid", &Lua_Door::Valid)
	.def("CreateDatabaseEntry", (void(Lua_Door::*)(void))&Lua_Door::CreateDatabaseEntry)
	.def("ForceClose", (void(Lua_Door::*)(Lua_Mob))&Lua_Door::ForceClose)
	.def("ForceClose", (void(Lua_Door::*)(Lua_Mob,bool))&Lua_Door::ForceClose)
	.def("ForceOpen", (void(Lua_Door::*)(Lua_Mob))&Lua_Door::ForceOpen)
	.def("ForceOpen", (void(Lua_Door::*)(Lua_Mob,bool))&Lua_Door::ForceOpen)
	.def("GetDisableTimer", (bool(Lua_Door::*)(void))&Lua_Door::GetDisableTimer)
	.def("GetClientVersionMask", (uint32(Lua_Door::*)(void))&Lua_Door::GetClientVersionMask)
	.def("GetDestinationHeading", (float(Lua_Door::*)(void))&Lua_Door::GetDestinationHeading)
	.def("GetDestinationInstanceID", (int(Lua_Door::*)(void))&Lua_Door::GetDestinationInstanceID)
	.def("GetDestinationX", (float(Lua_Door::*)(void))&Lua_Door::GetDestinationX)
	.def("GetDestinationY", (float(Lua_Door::*)(void))&Lua_Door::GetDestinationY)
	.def("GetDestinationZ", (float(Lua_Door::*)(void))&Lua_Door::GetDestinationZ)
	.def("GetDestinationZoneName", (std::string(Lua_Door::*)(void))&Lua_Door::GetDestinationZoneName)
	.def("GetDoorDBID", (uint32(Lua_Door::*)(void))&Lua_Door::GetDoorDBID)
	.def("GetDoorID", (uint32(Lua_Door::*)(void))&Lua_Door::GetDoorID)
	.def("GetDoorName", (const char*(Lua_Door::*)(void))&Lua_Door::GetDoorName)
	.def("GetDoorParam", (int(Lua_Door::*)(void))&Lua_Door::GetDoorParam)
	.def("GetDzSwitchID", (int(Lua_Door::*)(void))&Lua_Door::GetDzSwitchID)
	.def("GetGuildID", (uint32(Lua_Door::*)(void))&Lua_Door::GetGuildID)
	.def("GetHeading", (float(Lua_Door::*)(void))&Lua_Door::GetHeading)
	.def("GetID", (uint32(Lua_Door::*)(void))&Lua_Door::GetID)
	.def("GetIncline", (uint32(Lua_Door::*)(void))&Lua_Door::GetIncline)
	.def("GetInvertState", (int(Lua_Door::*)(void))&Lua_Door::GetInvertState)
	.def("GetKeyItem", (uint32(Lua_Door::*)(void))&Lua_Door::GetKeyItem)
	.def("GetLockPick", (uint32(Lua_Door::*)(void))&Lua_Door::GetLockPick)
	.def("GetNoKeyring", (int(Lua_Door::*)(void))&Lua_Door::GetNoKeyring)
	.def("GetOpenType", (uint32(Lua_Door::*)(void))&Lua_Door::GetOpenType)
	.def("GetSize", (uint32(Lua_Door::*)(void))&Lua_Door::GetSize)
	.def("GetTriggerDoorID", (uint8(Lua_Door::*)(void))&Lua_Door::GetTriggerDoorID)
	.def("GetTriggerType", (uint8(Lua_Door::*)(void))&Lua_Door::GetTriggerType)
	.def("GetX", (float(Lua_Door::*)(void))&Lua_Door::GetX)
	.def("GetY", (float(Lua_Door::*)(void))&Lua_Door::GetY)
	.def("GetZ", (float(Lua_Door::*)(void))&Lua_Door::GetZ)
	.def("HasDestinationZone", (bool(Lua_Door::*)(void))&Lua_Door::HasDestinationZone)
	.def("IsDestinationZoneSame", (bool(Lua_Door::*)(void))&Lua_Door::IsDestinationZoneSame)
	.def("IsDoorBlacklisted", (bool(Lua_Door::*)(void))&Lua_Door::IsDoorBlacklisted)
	.def("IsLDoNDoor", (bool(Lua_Door::*)(void))&Lua_Door::IsLDoNDoor)
	.def("SetDisableTimer", (void(Lua_Door::*)(bool))&Lua_Door::SetDisableTimer)
	.def("SetDoorName", (void(Lua_Door::*)(const char*))&Lua_Door::SetDoorName)
	.def("SetHeading", (void(Lua_Door::*)(float))&Lua_Door::SetHeading)
	.def("SetIncline", (void(Lua_Door::*)(uint32))&Lua_Door::SetIncline)
	.def("SetInvertState", (void(Lua_Door::*)(int))&Lua_Door::SetInvertState)
	.def("SetKeyItem", (void(Lua_Door::*)(uint32))&Lua_Door::SetKeyItem)
	.def("SetLocation", (void(Lua_Door::*)(float,float,float))&Lua_Door::SetLocation)
	.def("SetLockPick", (void(Lua_Door::*)(uint32))&Lua_Door::SetLockPick)
	.def("SetNoKeyring", (void(Lua_Door::*)(int))&Lua_Door::SetNoKeyring)
	.def("SetOpenType", (void(Lua_Door::*)(uint32))&Lua_Door::SetOpenType)
	.def("SetSize", (void(Lua_Door::*)(uint32))&Lua_Door::SetSize)
	.def("SetX", (void(Lua_Door::*)(float))&Lua_Door::SetX)
	.def("SetY", (void(Lua_Door::*)(float))&Lua_Door::SetY)
	.def("SetZ", (void(Lua_Door::*)(float))&Lua_Door::SetZ);
}

#endif
