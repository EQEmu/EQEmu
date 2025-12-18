#pragma once

#ifdef LUA_EQEMU

#include "zone/lua_entity.h"

class Doors;
class Lua_Mob;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_door();

class Lua_Door : public Lua_Entity
{
	typedef Doors NativeType;
public:
	Lua_Door() { }
	Lua_Door(Doors *d) { SetLuaPtrData(reinterpret_cast<Entity*>(d)); }
	virtual ~Lua_Door() { }

	operator Doors*() {
		void *d = GetLuaPtrData();
		if(d) {
			return reinterpret_cast<Doors*>(d);
		}

		return nullptr;
	}

	void SetDoorName(const char *name);
	const char *GetDoorName();
	float GetX();
	float GetY();
	float GetZ();
	float GetHeading();
	void SetX(float x);
	void SetY(float y);
	void SetZ(float z);
	void SetHeading(float h);
	void SetLocation(float x, float y, float z);
	uint32 GetDoorDBID();
	uint32 GetDoorID();
	void SetSize(uint32 sz);
	uint32 GetSize();
	void SetIncline(uint32 incline);
	uint32 GetIncline();
	void SetOpenType(uint32 type);
	uint32 GetOpenType();
	void SetDisableTimer(bool flag);
	bool GetDisableTimer();
	void SetLockPick(uint32 pick);
	uint32 GetLockPick();
	void SetKeyItem(uint32 key);
	uint32 GetKeyItem();
	void SetNoKeyring(int type);
	int GetNoKeyring();
	void CreateDatabaseEntry();
	void ForceOpen(Lua_Mob sender);
	void ForceOpen(Lua_Mob sender, bool alt_mode);
	void ForceClose(Lua_Mob sender);
	void ForceClose(Lua_Mob sender, bool alt_mode);
	uint32 GetID();
	uint8 GetTriggerDoorID();
	uint8 GetTriggerType();
	bool IsLDoNDoor();
	uint32 GetClientVersionMask();
	int GetDoorParam();
	bool HasDestinationZone();
	bool IsDestinationZoneSame();
	bool IsDoorBlacklisted();
	std::string GetDestinationZoneName();
	int GetDestinationInstanceID();
	float GetDestinationX();
	float GetDestinationY();
	float GetDestinationZ();
	float GetDestinationHeading();
	int GetDzSwitchID();
	int GetInvertState();
	void SetInvertState(int invert_state);
	uint32 GetGuildID();
};

#endif // LUA_EQEMU
