#pragma once

#ifdef LUA_EQEMU

#include "zone/lua_entity.h"

class Object;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_object();

class Lua_Object : public Lua_Entity
{
	typedef Object NativeType;
public:
	Lua_Object() { SetLuaPtrData(nullptr); }
	Lua_Object(Object *d) { SetLuaPtrData(reinterpret_cast<Entity*>(d)); }
	virtual ~Lua_Object() { }

	operator Object*() {
		void *d = GetLuaPtrData();
		if(d) {
			return reinterpret_cast<Object*>(d);
		}

		return nullptr;
	}

	void Depop();
	void Repop();
	void SetModelName(const char *name);
	const char *GetModelName();
	float GetX();
	float GetY();
	float GetZ();
	float GetHeading();
	void SetX(float x);
	void SetY(float y);
	void SetZ(float z);
	void SetHeading(float h);
	void SetLocation(float x, float y, float z);
	void SetItemID(uint32 item_id);
	uint32 GetItemID();
	void SetIcon(uint32 icon);
	uint32 GetIcon();
	void SetType(uint32 type);
	uint32 GetType();
	uint32 GetDBID();
	void ClearUser();
	void SetID(int user);
	int GetID();
	bool Save();
	uint32 VarSave();
	void DeleteItem(int index);
	void StartDecay();
	void Delete();
	void Delete(bool reset_state);
	bool IsGroundSpawn();
	void Close();
	bool ClearEntityVariables();
	bool DeleteEntityVariable(std::string variable_name);
	std::string GetEntityVariable(std::string variable_name);
	luabind::object GetEntityVariables(lua_State* L);
	void SetEntityVariable(std::string variable_name, std::string variable_value);
	bool EntityVariableExists(std::string variable_name);
};

#endif // LUA_EQEMU
