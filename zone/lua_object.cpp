#ifdef LUA_EQEMU

#include "lua_object.h"

#include "zone/object.h"

#include "lua.hpp"
#include "luabind/luabind.hpp"

void Lua_Object::Depop() {
	Lua_Safe_Call_Void();
	self->Depop();
}

void Lua_Object::Repop() {
	Lua_Safe_Call_Void();
	self->Repop();
}

void Lua_Object::SetModelName(const char *name) {
	Lua_Safe_Call_Void();
	self->SetModelName(name);
}

const char *Lua_Object::GetModelName() {
	Lua_Safe_Call_String();
	return self->GetModelName();
}

float Lua_Object::GetX() {
	Lua_Safe_Call_Real();
	return self->GetX();
}

float Lua_Object::GetY() {
	Lua_Safe_Call_Real();
	return self->GetY();
}

float Lua_Object::GetZ() {
	Lua_Safe_Call_Real();
	return self->GetZ();
}

float Lua_Object::GetHeading() {
	Lua_Safe_Call_Real();
	float h = 0.0f;
	self->GetHeading(&h);
	return h;
}

void Lua_Object::SetX(float x) {
	Lua_Safe_Call_Void();
	self->SetX(x);
}

void Lua_Object::SetY(float y) {
	Lua_Safe_Call_Void();
	self->SetY(y);
}

void Lua_Object::SetZ(float z) {
	Lua_Safe_Call_Void();
	self->SetZ(z);
}

void Lua_Object::SetHeading(float h) {
	Lua_Safe_Call_Void();
	self->SetHeading(h);
}

void Lua_Object::SetLocation(float x, float y, float z) {
	Lua_Safe_Call_Void();
	self->SetLocation(x, y, z);
}

void Lua_Object::SetItemID(uint32 item_id) {
	Lua_Safe_Call_Void();
	self->SetItemID(item_id);
}

uint32 Lua_Object::GetItemID() {
	Lua_Safe_Call_Int();
	return self->GetItemID();
}

void Lua_Object::SetIcon(uint32 icon) {
	Lua_Safe_Call_Void();
	self->SetIcon(icon);
}

uint32 Lua_Object::GetIcon() {
	Lua_Safe_Call_Int();
	return self->GetIcon();
}

void Lua_Object::SetType(uint32 type) {
	Lua_Safe_Call_Void();
	self->SetType(type);
}

uint32 Lua_Object::GetType() {
	Lua_Safe_Call_Int();
	return self->GetType();
}

uint32 Lua_Object::GetDBID() {
	Lua_Safe_Call_Int();
	return self->GetDBID();
}

void Lua_Object::ClearUser() {
	Lua_Safe_Call_Void();
	self->ClearUser();
}

void Lua_Object::SetID(int user) {
	Lua_Safe_Call_Void();
	self->SetID(user);
}

int Lua_Object::GetID() {
	Lua_Safe_Call_Int();
	return self->GetID();
}

bool Lua_Object::Save() {
	Lua_Safe_Call_Int();
	return self->Save();
}

uint32 Lua_Object::VarSave() {
	Lua_Safe_Call_Int();
	return self->VarSave();
}

void Lua_Object::DeleteItem(int index) {
	Lua_Safe_Call_Void();
	self->DeleteItem(index);
}

void Lua_Object::StartDecay() {
	Lua_Safe_Call_Void();
	self->StartDecay();
}

void Lua_Object::Delete() {
	Lua_Safe_Call_Void();
	self->Delete();
}

void Lua_Object::Delete(bool reset_state) {
	Lua_Safe_Call_Void();
	self->Delete(reset_state);
}

bool Lua_Object::IsGroundSpawn() {
	Lua_Safe_Call_Int();
	return self->IsGroundSpawn();
}

void Lua_Object::Close() {
	Lua_Safe_Call_Void();
	self->Close();
}

std::string Lua_Object::GetEntityVariable(std::string variable_name) {
	Lua_Safe_Call_String();
	return self->GetEntityVariable(variable_name);
}

luabind::object Lua_Object::GetEntityVariables(lua_State* L) {
	auto t = luabind::newtable(L);
	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);
		auto l = self->GetEntityVariables();
		int i = 1;
		for (const auto& v : l) {
			t[i] = v;
			i++;
		}
	}

	return t;
}

bool Lua_Object::ClearEntityVariables() {
	Lua_Safe_Call_Bool();
	return self->ClearEntityVariables();
}

bool Lua_Object::DeleteEntityVariable(std::string variable_name) {
	Lua_Safe_Call_Bool();
	return self->DeleteEntityVariable(variable_name);
}

void Lua_Object::SetEntityVariable(std::string variable_name, std::string variable_value) {
	Lua_Safe_Call_Void();
	self->SetEntityVariable(variable_name, variable_value);
}

bool Lua_Object::EntityVariableExists(std::string variable_name) {
	Lua_Safe_Call_Int();
	return self->EntityVariableExists(variable_name);
}

luabind::scope lua_register_object() {
	return luabind::class_<Lua_Object, Lua_Entity>("Object")
	.def(luabind::constructor<>())
	.property("null", &Lua_Object::Null)
	.property("valid", &Lua_Object::Valid)
	.def("ClearEntityVariables", (bool(Lua_Object::*)(void))&Lua_Object::ClearEntityVariables)
	.def("ClearUser", (void(Lua_Object::*)(void))&Lua_Object::ClearUser)
	.def("Close", (void(Lua_Object::*)(void))&Lua_Object::Close)
	.def("Delete", (void(Lua_Object::*)(bool))&Lua_Object::Delete)
	.def("Delete", (void(Lua_Object::*)(void))&Lua_Object::Delete)
	.def("DeleteEntityVariable", (bool(Lua_Object::*)(std::string))&Lua_Object::DeleteEntityVariable)
	.def("DeleteItem", (void(Lua_Object::*)(int))&Lua_Object::DeleteItem)
	.def("Depop", (void(Lua_Object::*)(void))&Lua_Object::Depop)
	.def("EntityVariableExists", (bool(Lua_Object::*)(std::string))&Lua_Object::EntityVariableExists)
	.def("GetDBID", (uint32(Lua_Object::*)(void))&Lua_Object::GetDBID)
	.def("GetEntityVariable", (std::string(Lua_Object::*)(std::string))&Lua_Object::GetEntityVariable)
	.def("GetEntityVariables", (luabind::object(Lua_Object::*)(lua_State*))&Lua_Object::GetEntityVariables)
	.def("GetHeading", (float(Lua_Object::*)(void))&Lua_Object::GetHeading)
	.def("GetID", (int(Lua_Object::*)(void))&Lua_Object::GetID)
	.def("GetIcon", (uint32(Lua_Object::*)(void))&Lua_Object::GetIcon)
	.def("GetItemID", (uint32(Lua_Object::*)(void))&Lua_Object::GetItemID)
	.def("GetModelName", (const char*(Lua_Object::*)(void))&Lua_Object::GetModelName)
	.def("GetType", (uint32(Lua_Object::*)(void))&Lua_Object::GetType)
	.def("GetX", (float(Lua_Object::*)(void))&Lua_Object::GetX)
	.def("GetY", (float(Lua_Object::*)(void))&Lua_Object::GetY)
	.def("GetZ", (float(Lua_Object::*)(void))&Lua_Object::GetZ)
	.def("IsGroundSpawn", (bool(Lua_Object::*)(void))&Lua_Object::IsGroundSpawn)
	.def("Repop", (void(Lua_Object::*)(void))&Lua_Object::Repop)
	.def("Save", (bool(Lua_Object::*)(void))&Lua_Object::Save)
	.def("SetEntityVariable", (void(Lua_Object::*)(std::string,std::string))&Lua_Object::SetEntityVariable)
	.def("SetHeading", (void(Lua_Object::*)(float))&Lua_Object::SetHeading)
	.def("SetID", (void(Lua_Object::*)(int))&Lua_Object::SetID)
	.def("SetIcon", (void(Lua_Object::*)(uint32))&Lua_Object::SetIcon)
	.def("SetItemID", (void(Lua_Object::*)(uint32))&Lua_Object::SetItemID)
	.def("SetLocation", (void(Lua_Object::*)(float,float,float))&Lua_Object::SetLocation)
	.def("SetModelName", (void(Lua_Object::*)(const char*))&Lua_Object::SetModelName)
	.def("SetType", (void(Lua_Object::*)(uint32))&Lua_Object::SetType)
	.def("SetX", (void(Lua_Object::*)(float))&Lua_Object::SetX)
	.def("SetY", (void(Lua_Object::*)(float))&Lua_Object::SetY)
	.def("SetZ", (void(Lua_Object::*)(float))&Lua_Object::SetZ)
	.def("StartDecay", (void(Lua_Object::*)(void))&Lua_Object::StartDecay)
	.def("VarSave", (uint32(Lua_Object::*)(void))&Lua_Object::VarSave);
}

#endif // LUA_EQEMU
