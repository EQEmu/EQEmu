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