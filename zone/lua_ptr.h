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

#include "lua.hpp"
#include "luabind/luabind.hpp"

#ifndef EQEMU_UNSAFE_LUA
#define Lua_Safe_Call_Void() if(!d_) { return; } NativeType *self = reinterpret_cast<NativeType*>(d_)
#define Lua_Safe_Call_Bool() if(!d_) { return false; } NativeType *self = reinterpret_cast<NativeType*>(d_)
#define Lua_Safe_Call_Int() if(!d_) { return 0; } NativeType *self = reinterpret_cast<NativeType*>(d_)
#define Lua_Safe_Call_Real() if(!d_) { return 0.0; } NativeType *self = reinterpret_cast<NativeType*>(d_)
#define Lua_Safe_Call_String() if(!d_) { return ""; } NativeType *self = reinterpret_cast<NativeType*>(d_)
#define Lua_Safe_Call_Class(type) if(!d_) { return type(); } NativeType *self = reinterpret_cast<NativeType*>(d_)
#else
#define Lua_Safe_Call_Void() NativeType *self = reinterpret_cast<NativeType*>(d_)
#define Lua_Safe_Call_Bool() NativeType *self = reinterpret_cast<NativeType*>(d_)
#define Lua_Safe_Call_Int() NativeType *self = reinterpret_cast<NativeType*>(d_)
#define Lua_Safe_Call_Real() NativeType *self = reinterpret_cast<NativeType*>(d_)
#define Lua_Safe_Call_String() NativeType *self = reinterpret_cast<NativeType*>(d_)
#define Lua_Safe_Call_Class(type) NativeType *self = reinterpret_cast<NativeType*>(d_)
#endif

template<typename T>
class Lua_Ptr
{
public:
	Lua_Ptr() {
	}
	
	Lua_Ptr(T *d) : d_(d) {
	}
	
	~Lua_Ptr() {
	}
	
	T *GetLuaPtrData() {
		return d_;
	}
	
	void SetLuaPtrData(T *d) {
		d_ = d;
	}
	
	bool Null() {
		return d_ == nullptr;
	}
	
	bool Valid() {
		return d_ != nullptr;
	}
	
protected:
	T *d_;
};

#endif // LUA_EQEMU
