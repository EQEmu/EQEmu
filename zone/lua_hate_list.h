#pragma once

#ifdef LUA_EQEMU

#include "zone/lua_ptr.h"

class Lua_Mob;
struct struct_HateList;

luabind::scope lua_register_hate_entry();
luabind::scope lua_register_hate_list();

class Lua_HateEntry : public Lua_Ptr<struct_HateList>
{
	typedef struct_HateList NativeType;
public:
	Lua_HateEntry() : Lua_Ptr(nullptr) { }
	Lua_HateEntry(struct_HateList *d) : Lua_Ptr(d) { }
	virtual ~Lua_HateEntry() { }

	Lua_Mob GetEnt();
	void SetEnt(Lua_Mob e);
	int64 GetDamage();
	void SetDamage(int64 value);
	int64 GetHate();
	void SetHate(int64 value);
	bool GetFrenzy();
	void SetFrenzy(bool value);
};

struct Lua_HateList
{
	std::vector<Lua_HateEntry> entries;
};

#endif // LUA_EQEMU
