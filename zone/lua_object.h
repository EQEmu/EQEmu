/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
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
