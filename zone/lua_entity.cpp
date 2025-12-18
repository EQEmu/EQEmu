#ifdef LUA_EQEMU

#include "lua_entity.h"

#include "zone/entity.h"
#include "zone/lua_bot.h"
#include "zone/lua_client.h"
#include "zone/lua_corpse.h"
#include "zone/lua_door.h"
#include "zone/lua_merc.h"
#include "zone/lua_mob.h"
#include "zone/lua_npc.h"
#include "zone/lua_object.h"

#include "luabind/luabind.hpp"

bool Lua_Entity::IsClient() {
	Lua_Safe_Call_Bool();
	return self->IsClient();
}

bool Lua_Entity::IsNPC() {
	Lua_Safe_Call_Bool();
	return self->IsNPC();
}

bool Lua_Entity::IsMob() {
	Lua_Safe_Call_Bool();
	return self->IsMob();
}

bool Lua_Entity::IsMerc() {
	Lua_Safe_Call_Bool();
	return self->IsMerc();
}

bool Lua_Entity::IsCorpse() {
	Lua_Safe_Call_Bool();
	return self->IsCorpse();
}

bool Lua_Entity::IsPlayerCorpse() {
	Lua_Safe_Call_Bool();
	return self->IsPlayerCorpse();
}

bool Lua_Entity::IsNPCCorpse() {
	Lua_Safe_Call_Bool();
	return self->IsNPCCorpse();
}

bool Lua_Entity::IsObject() {
	Lua_Safe_Call_Bool();
	return self->IsObject();
}

bool Lua_Entity::IsDoor() {
	Lua_Safe_Call_Bool();
	return self->IsDoor();
}

bool Lua_Entity::IsTrap() {
	Lua_Safe_Call_Bool();
	return self->IsTrap();
}

bool Lua_Entity::IsBeacon() {
	Lua_Safe_Call_Bool();
	return self->IsBeacon();
}

bool Lua_Entity::IsEncounter() {
	Lua_Safe_Call_Bool();
	return self->IsEncounter();
}

bool Lua_Entity::IsBot() {
	Lua_Safe_Call_Bool();
	return self->IsBot();
}

bool Lua_Entity::IsAura() {
	Lua_Safe_Call_Bool();
	return self->IsAura();
}

bool Lua_Entity::IsOfClientBot() {
	Lua_Safe_Call_Bool();
	return self->IsOfClientBot();
}

bool Lua_Entity::IsOfClientBotMerc() {
	Lua_Safe_Call_Bool();
	return self->IsOfClientBotMerc();
}

int Lua_Entity::GetID() {
	Lua_Safe_Call_Bool();
	return self->GetID();
}

Lua_Client Lua_Entity::CastToClient() {
	void *d = GetLuaPtrData();
	Client *m = reinterpret_cast<Client*>(d);
	return Lua_Client(m);
}

Lua_NPC Lua_Entity::CastToNPC() {
	void *d = GetLuaPtrData();
	NPC *m = reinterpret_cast<NPC*>(d);
	return Lua_NPC(m);
}

Lua_Mob Lua_Entity::CastToMob() {
	void *d = GetLuaPtrData();
	Mob *m = reinterpret_cast<Mob*>(d);
	return Lua_Mob(m);
}

Lua_Corpse Lua_Entity::CastToCorpse() {
	void *d = GetLuaPtrData();
	Corpse *m = reinterpret_cast<Corpse*>(d);
	return Lua_Corpse(m);
}

Lua_Object Lua_Entity::CastToObject() {
	void *d = GetLuaPtrData();
	Object *m = reinterpret_cast<Object*>(d);
	return Lua_Object(m);
}

Lua_Door Lua_Entity::CastToDoor() {
	void *d = GetLuaPtrData();
	Doors *m = reinterpret_cast<Doors*>(d);
	return Lua_Door(m);
}

Lua_Bot Lua_Entity::CastToBot() {
	void *d = GetLuaPtrData();
	Bot *b = reinterpret_cast<Bot*>(d);
	return Lua_Bot(b);
}

Lua_Merc Lua_Entity::CastToMerc() {
	void *d = GetLuaPtrData();
	Merc *m = reinterpret_cast<Merc*>(d);
	return Lua_Merc(m);
}

luabind::scope lua_register_entity() {
	return luabind::class_<Lua_Entity>("Entity")
	.def(luabind::constructor<>())
	.property("null", &Lua_Entity::Null)
	.property("valid", &Lua_Entity::Valid)
	.def("CastToBot", &Lua_Entity::CastToBot)
	.def("CastToClient", &Lua_Entity::CastToClient)
	.def("CastToCorpse", &Lua_Entity::CastToCorpse)
	.def("CastToDoor", &Lua_Entity::CastToDoor)
	.def("CastToMerc", &Lua_Entity::CastToMerc)
	.def("CastToMob", &Lua_Entity::CastToMob)
	.def("CastToNPC", &Lua_Entity::CastToNPC)
	.def("CastToObject", &Lua_Entity::CastToObject)
	.def("GetID", &Lua_Entity::GetID)
	.def("IsAura", &Lua_Entity::IsAura)
	.def("IsBeacon", &Lua_Entity::IsBeacon)
	.def("IsBot", &Lua_Entity::IsBot)
	.def("IsClient", &Lua_Entity::IsClient)
	.def("IsCorpse", &Lua_Entity::IsCorpse)
	.def("IsDoor", &Lua_Entity::IsDoor)
	.def("IsEncounter", &Lua_Entity::IsEncounter)
	.def("IsMerc", &Lua_Entity::IsMerc)
	.def("IsMob", &Lua_Entity::IsMob)
	.def("IsNPC", &Lua_Entity::IsNPC)
	.def("IsNPCCorpse", &Lua_Entity::IsNPCCorpse)
	.def("IsObject", &Lua_Entity::IsObject)
	.def("IsOfClientBot", &Lua_Entity::IsOfClientBot)
	.def("IsOfClientBotMerc", &Lua_Entity::IsOfClientBotMerc)
	.def("IsPlayerCorpse", &Lua_Entity::IsPlayerCorpse)
	.def("IsTrap", &Lua_Entity::IsTrap);
}

#endif
