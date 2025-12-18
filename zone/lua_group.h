#pragma once

#ifdef LUA_EQEMU

#include "zone/lua_ptr.h"

class Group;
class Lua_Mob;
class Lua_Client;
class Lua_Bot;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_group();

class Lua_Group : public Lua_Ptr<Group>
{
	typedef Group NativeType;
public:
	Lua_Group() : Lua_Ptr(nullptr) { }
	Lua_Group(Group *d) : Lua_Ptr(d) { }
	virtual ~Lua_Group() { }

	operator Group*() {
		return reinterpret_cast<Group*>(GetLuaPtrData());
	}

	void DisbandGroup();
	bool IsGroupMember(const char* name);
	bool IsGroupMember(Lua_Mob c);
	void CastGroupSpell(Lua_Mob caster, int spell_id);
	void SplitExp(uint64 exp, Lua_Mob other);
	void GroupMessage(Lua_Mob sender, const char* message);
	void GroupMessage(Lua_Mob sender, uint8 language_id, const char* message);
	uint32 GetTotalGroupDamage(Lua_Mob other);
	void SplitMoney(uint32 copper, uint32 silver, uint32 gold, uint32 platinum);
	void SplitMoney(uint32 copper, uint32 silver, uint32 gold, uint32 platinum, Lua_Client splitter);
	void SetLeader(Lua_Mob c);
	Lua_Mob GetLeader();
	std::string GetLeaderName();
	bool IsLeader(const char* name);
	bool IsLeader(Lua_Mob c);
	int GroupCount();
	uint32 GetAverageLevel();
	uint32 GetHighestLevel();
	uint32 GetLowestLevel();
	void TeleportGroup(Lua_Mob sender, uint32 zone_id, uint32 instance_id, float x, float y, float z, float h);
	int GetID();
	Lua_Mob GetMember(int member_index);
	uint8 GetMemberRole(Lua_Mob member);
	uint8 GetMemberRole(const char* name);
	bool DoesAnyMemberHaveExpeditionLockout(std::string expedition_name, std::string event_name);
	bool DoesAnyMemberHaveExpeditionLockout(std::string expedition_name, std::string event_name, int max_check_count);
};

#endif // LUA_EQEMU
