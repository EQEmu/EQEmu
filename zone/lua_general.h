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

#include "luabind/luabind.hpp"

luabind::scope lua_register_general();
luabind::scope lua_register_random();
luabind::scope lua_register_events();
luabind::scope lua_register_faction();
luabind::scope lua_register_slot();
luabind::scope lua_register_material();
luabind::scope lua_register_client_version();
luabind::scope lua_register_appearance();
luabind::scope lua_register_classes();
luabind::scope lua_register_skills();
luabind::scope lua_register_bodytypes();
luabind::scope lua_register_filters();
luabind::scope lua_register_message_types();
luabind::scope lua_register_zone_types();
luabind::scope lua_register_languages();
luabind::scope lua_register_rules_const();
luabind::scope lua_register_rulei();
luabind::scope lua_register_ruler();
luabind::scope lua_register_ruleb();
luabind::scope lua_register_rules();
luabind::scope lua_register_journal_speakmode();
luabind::scope lua_register_journal_mode();
luabind::scope lua_register_exp_source();

#endif // LUA_EQEMU