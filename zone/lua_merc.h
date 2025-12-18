#pragma once

#ifdef LUA_EQEMU

#include "zone/lua_mob.h"

class Merc;
class Lua_Group;
class Lua_Merc;
class Lua_Mob;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_merc();

class Lua_Merc : public Lua_Mob
{
	typedef Merc NativeType;
public:
	Lua_Merc() { SetLuaPtrData(nullptr); }
	Lua_Merc(Merc *d) { SetLuaPtrData(reinterpret_cast<Entity*>(d)); }
	virtual ~Lua_Merc() { }

	operator Merc*() {
		return reinterpret_cast<Merc*>(GetLuaPtrData());
	}

	uint32 GetCostFormula();
	Lua_Group GetGroup();
	int GetHatedCount();
	float GetMaxMeleeRangeToTarget(Lua_Mob target);
	uint32 GetMercenaryCharacterID();
	uint32 GetMercenaryID();
	uint32 GetMercenaryNameType();
	Lua_Client GetMercenaryOwner();
	uint32 GetMercenarySubtype();
	uint32 GetMercenaryTemplateID();
	uint32 GetMercenaryType();
	Lua_Mob GetOwner();
	Lua_Mob GetOwnerOrSelf();
	uint8 GetProficiencyID();
	uint8 GetStance();
	uint8 GetTierID();
	bool HasOrMayGetAggro();
	bool IsMercenaryCaster();
	bool IsMercenaryCasterCombatRange(Lua_Mob target);
	bool IsSitting();
	bool IsStanding();
	void ScaleStats(int scale_percentage);
	void ScaleStats(int scale_percentage, bool set_to_max);
	void SendPayload(int payload_id, std::string payload_value);
	void SetTarget(Lua_Mob target);
	void Signal(int signal_id);
	void Sit();
	void Stand();
	bool Suspend();
	bool UseDiscipline(uint16 spell_id, uint16 target_id);
};

#endif // LUA_EQEMU
