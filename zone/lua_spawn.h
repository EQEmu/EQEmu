#pragma once

#ifdef LUA_EQEMU

#include "zone/lua_ptr.h"

class Spawn2;
class Lua_NPC;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_spawn();

class Lua_Spawn : public Lua_Ptr<Spawn2>
{
	typedef Spawn2 NativeType;
public:
	Lua_Spawn() : Lua_Ptr(nullptr) { }
	Lua_Spawn(Spawn2 *d) : Lua_Ptr(d) { }
	virtual ~Lua_Spawn() { }

	operator Spawn2*() {
		return reinterpret_cast<Spawn2*>(GetLuaPtrData());
	}

	void LoadGrid();
	void Enable();
	void Disable();
	bool Enabled();
	void Reset();
	void Depop();
	void Repop();
	void Repop(uint32 delay);
	void ForceDespawn();
	uint32 GetID();
	float GetX();
	float GetY();
	float GetZ();
	float GetHeading();
	void SetRespawnTimer(uint32 newrespawntime);
	void SetVariance(uint32 newvariance);
	uint32 GetVariance();
	uint32 RespawnTimer();
	uint32 SpawnGroupID();
	uint32 CurrentNPCID();
	void SetCurrentNPCID(uint32 nid);
	uint32 GetSpawnCondition();
	bool NPCPointerValid();
	void SetNPCPointer(Lua_NPC n);
	void SetTimer(uint32 duration);
	uint32 GetKillCount();
};

#endif // LUA_EQEMU
