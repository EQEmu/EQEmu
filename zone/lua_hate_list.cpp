#ifdef LUA_EQEMU

#include "lua_hate_list.h"

#include "zone/lua_mob.h"
#include "zone/masterentity.h"

#include "luabind/iterator_policy.hpp"
#include "luabind/luabind.hpp"

Lua_Mob Lua_HateEntry::GetEnt() {
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->entity_on_hatelist);
}

void Lua_HateEntry::SetEnt(Lua_Mob e) {
	Lua_Safe_Call_Void();
	self->entity_on_hatelist = e;
}

int64 Lua_HateEntry::GetDamage() {
	Lua_Safe_Call_Int();
	return self->hatelist_damage;
}

void Lua_HateEntry::SetDamage(int64 value) {
	Lua_Safe_Call_Void();
	self->hatelist_damage = value;
}

int64 Lua_HateEntry::GetHate() {
	Lua_Safe_Call_Int();
	return self->stored_hate_amount;
}

void Lua_HateEntry::SetHate(int64 value) {
	Lua_Safe_Call_Void();
	self->stored_hate_amount = value;
}

bool Lua_HateEntry::GetFrenzy() {
	Lua_Safe_Call_Bool();
	return self->is_entity_frenzy;
}

void Lua_HateEntry::SetFrenzy(bool value) {
	Lua_Safe_Call_Void();
	self->is_entity_frenzy = value;
}

luabind::scope lua_register_hate_entry() {
	return luabind::class_<Lua_HateEntry>("HateEntry")
	.property("null", &Lua_HateEntry::Null)
	.property("valid", &Lua_HateEntry::Valid)
	.property("damage", &Lua_HateEntry::GetDamage, &Lua_HateEntry::SetDamage)
	.property("ent", &Lua_HateEntry::GetEnt, &Lua_HateEntry::SetEnt)
	.property("frenzy", &Lua_HateEntry::GetFrenzy, &Lua_HateEntry::SetFrenzy)
	.property("hate", &Lua_HateEntry::GetHate, &Lua_HateEntry::SetHate);
}

luabind::scope lua_register_hate_list() {
	return luabind::class_<Lua_HateList>("HateList")
	.def_readwrite("entries", &Lua_HateList::entries, luabind::return_stl_iterator);
}

#endif // LUA_EQEMU
