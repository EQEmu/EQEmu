#ifdef LUA_EQEMU

#include "lua_mod.h"

#include "common/spdat.h"
#include "zone/masterentity.h"
#include "zone/zone.h"
#include "zone/lua_parser.h"
#include "zone/lua_item.h"
#include "zone/lua_iteminst.h"
#include "zone/lua_mob.h"
#include "zone/lua_hate_list.h"
#include "zone/lua_client.h"
#include "zone/lua_inventory.h"
#include "zone/lua_npc.h"
#include "zone/lua_group.h"
#include "zone/lua_raid.h"
#include "zone/lua_corpse.h"
#include "zone/lua_object.h"
#include "zone/lua_door.h"
#include "zone/lua_packet.h"
#include "zone/lua_stat_bonuses.h"

#include "lua.hpp"
#include "luabind/luabind.hpp"
#include "luabind/object.hpp"


void LuaMod::Init()
{
	m_has_melee_mitigation = parser_->HasFunction("MeleeMitigation", package_name_);
	m_has_apply_damage_table = parser_->HasFunction("ApplyDamageTable", package_name_);
	m_has_avoid_damage = parser_->HasFunction("AvoidDamage", package_name_);
	m_has_check_hit_chance = parser_->HasFunction("CheckHitChance", package_name_);
	m_has_try_critical_hit = parser_->HasFunction("TryCriticalHit", package_name_);
	m_has_get_required_aa_experience = parser_->HasFunction("GetRequiredAAExperience", package_name_);
	m_has_get_exp_for_level = parser_->HasFunction("GetEXPForLevel", package_name_);
	m_has_get_experience_for_kill = parser_->HasFunction("GetExperienceForKill", package_name_);
	m_has_common_outgoing_hit_success = parser_->HasFunction("CommonOutgoingHitSuccess", package_name_);
	m_has_calc_spell_effect_value_formula = parser_->HasFunction("CalcSpellEffectValue_formula", package_name_);
	m_has_update_personal_faction = parser_->HasFunction("UpdatePersonalFaction", package_name_);
	m_has_register_bug = parser_->HasFunction("RegisterBug", package_name_);
	m_has_common_damage = parser_->HasFunction("CommonDamage", package_name_);
	m_has_heal_damage = parser_->HasFunction("HealDamage", package_name_);
	m_has_is_immune_to_spell = parser_->HasFunction("IsImmuneToSpell", package_name_);
	m_has_set_aa_exp = parser_->HasFunction("SetAAEXP", package_name_);
	m_has_set_exp = parser_->HasFunction("SetEXP", package_name_);
}

void PutDamageHitInfo(lua_State *L, luabind::adl::object &e, DamageHitInfo &hit) {
	luabind::adl::object lua_hit = luabind::newtable(L);
	lua_hit["base_damage"] = hit.base_damage;
	lua_hit["min_damage"] = hit.min_damage;
	lua_hit["damage_done"] = hit.damage_done;
	lua_hit["offense"] = hit.offense;
	lua_hit["tohit"] = hit.tohit;
	lua_hit["hand"] = hit.hand;
	lua_hit["skill"] = (int)hit.skill;
	e["hit"] = lua_hit;
}

void GetDamageHitInfo(luabind::adl::object &ret, DamageHitInfo &hit) {
	auto luaHitTable = ret["hit"];
	if (luabind::type(luaHitTable) == LUA_TTABLE) {
		auto base_damage = luaHitTable["base_damage"];
		auto min_damage = luaHitTable["min_damage"];
		auto damage_done = luaHitTable["damage_done"];
		auto offense = luaHitTable["offense"];
		auto tohit = luaHitTable["tohit"];
		auto hand = luaHitTable["hand"];
		auto skill = luaHitTable["skill"];

		if (luabind::type(base_damage) == LUA_TNUMBER) {
			hit.base_damage = luabind::object_cast<int>(base_damage);
		}

		if (luabind::type(min_damage) == LUA_TNUMBER) {
			hit.min_damage = luabind::object_cast<int>(min_damage);
		}

		if (luabind::type(damage_done) == LUA_TNUMBER) {
			hit.damage_done = luabind::object_cast<int>(damage_done);
		}

		if (luabind::type(offense) == LUA_TNUMBER) {
			hit.offense = luabind::object_cast<int>(offense);
		}

		if (luabind::type(tohit) == LUA_TNUMBER) {
			hit.tohit = luabind::object_cast<int>(tohit);
		}

		if (luabind::type(hand) == LUA_TNUMBER) {
			hit.hand = luabind::object_cast<int>(hand);
		}

		if (luabind::type(skill) == LUA_TNUMBER) {
			hit.skill = (EQ::skills::SkillType)luabind::object_cast<int>(skill);
		}
	}
}

void PutExtraAttackOptions(lua_State *L, luabind::adl::object &e, ExtraAttackOptions *opts) {
	if (opts) {
		luabind::adl::object lua_opts = luabind::newtable(L);
		lua_opts["damage_percent"] = opts->damage_percent;
		lua_opts["damage_flat"] = opts->damage_flat;
		lua_opts["armor_pen_percent"] = opts->armor_pen_percent;
		lua_opts["armor_pen_flat"] = opts->armor_pen_flat;
		lua_opts["crit_percent"] = opts->crit_percent;
		lua_opts["crit_flat"] = opts->crit_flat;
		lua_opts["hate_percent"] = opts->hate_percent;
		lua_opts["hate_flat"] = opts->hate_flat;
		lua_opts["hit_chance"] = opts->hit_chance;
		lua_opts["melee_damage_bonus_flat"] = opts->melee_damage_bonus_flat;
		lua_opts["skilldmgtaken_bonus_flat"] = opts->skilldmgtaken_bonus_flat;
		e["opts"] = lua_opts;
	}
}

void GetExtraAttackOptions(luabind::adl::object &ret, ExtraAttackOptions *opts) {
	if (opts) {
		auto luaOptsTable = ret["opts"];
		if (luabind::type(luaOptsTable) == LUA_TTABLE) {
			auto damage_percent = luaOptsTable["damage_percent"];
			auto damage_flat = luaOptsTable["damage_flat"];
			auto armor_pen_percent = luaOptsTable["armor_pen_percent"];
			auto armor_pen_flat = luaOptsTable["armor_pen_flat"];
			auto crit_percent = luaOptsTable["crit_percent"];
			auto crit_flat = luaOptsTable["crit_flat"];
			auto hate_percent = luaOptsTable["hate_percent"];
			auto hate_flat = luaOptsTable["hate_flat"];
			auto hit_chance = luaOptsTable["hit_chance"];
			auto melee_damage_bonus_flat = luaOptsTable["melee_damage_bonus_flat"];
			auto skilldmgtaken_bonus_flat = luaOptsTable["skilldmgtaken_bonus_flat"];

			if (luabind::type(damage_percent) == LUA_TNUMBER) {
				opts->damage_percent = luabind::object_cast<float>(damage_percent);
			}

			if (luabind::type(damage_flat) == LUA_TNUMBER) {
				opts->damage_flat = luabind::object_cast<int>(damage_flat);
			}

			if (luabind::type(armor_pen_percent) == LUA_TNUMBER) {
				opts->armor_pen_percent = luabind::object_cast<float>(armor_pen_percent);
			}

			if (luabind::type(armor_pen_flat) == LUA_TNUMBER) {
				opts->armor_pen_flat = luabind::object_cast<int>(armor_pen_flat);
			}

			if (luabind::type(crit_percent) == LUA_TNUMBER) {
				opts->crit_percent = luabind::object_cast<float>(crit_percent);
			}

			if (luabind::type(crit_flat) == LUA_TNUMBER) {
				opts->crit_flat = luabind::object_cast<float>(crit_flat);
			}

			if (luabind::type(hate_percent) == LUA_TNUMBER) {
				opts->hate_percent = luabind::object_cast<float>(hate_percent);
			}

			if (luabind::type(hate_flat) == LUA_TNUMBER) {
				opts->hate_flat = luabind::object_cast<int>(hate_flat);
			}

			if (luabind::type(hit_chance) == LUA_TNUMBER) {
				opts->hit_chance = luabind::object_cast<int>(hit_chance);
			}

			if (luabind::type(melee_damage_bonus_flat) == LUA_TNUMBER) {
				opts->melee_damage_bonus_flat = luabind::object_cast<int>(melee_damage_bonus_flat);
			}

			if (luabind::type(skilldmgtaken_bonus_flat) == LUA_TNUMBER) {
				opts->skilldmgtaken_bonus_flat = luabind::object_cast<int>(skilldmgtaken_bonus_flat);
			}
		}
	}
}

void LuaMod::UpdatePersonalFaction(Mob *self, int32 npc_value, int32 faction_id, int32 current_value, int32 temp, int32 this_faction_min, int32 this_faction_max, int32 &return_value, bool &ignore_default)
{
	int start = lua_gettop(L);

	try {
		if (!m_has_update_personal_faction) {
			return;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, package_name_.c_str());
		lua_getfield(L, -1, "UpdatePersonalFaction");

		Lua_Mob l_self(self);
		luabind::adl::object e = luabind::newtable(L);
		e["self"] = l_self;
		e["npc_value"] = npc_value;
		e["faction_id"] = faction_id;
		e["current_value"] = current_value;
		e["temp"] = temp;
		e["this_faction_min"] = this_faction_min;
		e["this_faction_max"] = this_faction_max;

		e.push(L);

		if (lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			parser_->AddError(error);
			lua_pop(L, 2);
			return;
		}

		if (lua_type(L, -1) == LUA_TTABLE) {
			luabind::adl::object ret(luabind::from_stack(L, -1));
			auto ignore_default_obj = ret["ignore_default"];
			if (luabind::type(ignore_default_obj) == LUA_TBOOLEAN) {
				ignore_default = ignore_default || luabind::object_cast<bool>(ignore_default_obj);
			}

			auto return_value_obj = ret["return_value"];
			if (luabind::type(return_value_obj) == LUA_TNUMBER) {
				return_value = luabind::object_cast<int64>(return_value_obj);
			}
		}
	}
	catch (std::exception &ex) {
		parser_->AddError(ex.what());
	}

	int end = lua_gettop(L);
	int n = end - start;
	if (n > 0) {
		lua_pop(L, n);
	}
}

void LuaMod::MeleeMitigation(Mob *self, Mob *attacker, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault) {
	int start = lua_gettop(L);

	try {
		if (!m_has_melee_mitigation) {
			return;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, package_name_.c_str());
		lua_getfield(L, -1, "MeleeMitigation");

		Lua_Mob l_self(self);
		Lua_Mob l_other(attacker);
		luabind::adl::object e = luabind::newtable(L);
		e["self"] = l_self;
		e["other"] = l_other;

		PutDamageHitInfo(L, e, hit);
		PutExtraAttackOptions(L, e, opts);

		e.push(L);

		if (lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			parser_->AddError(error);
			lua_pop(L, 2);
			return;
		}

		if (lua_type(L, -1) == LUA_TTABLE) {
			luabind::adl::object ret(luabind::from_stack(L, -1));
			auto IgnoreDefaultObj = ret["IgnoreDefault"];
			if (luabind::type(IgnoreDefaultObj) == LUA_TBOOLEAN) {
				ignoreDefault = ignoreDefault || luabind::object_cast<bool>(IgnoreDefaultObj);
			}

			GetDamageHitInfo(ret, hit);
			GetExtraAttackOptions(ret, opts);
		}
	}
	catch (std::exception &ex) {
		parser_->AddError(ex.what());
	}

	int end = lua_gettop(L);
	int n = end - start;
	if (n > 0) {
		lua_pop(L, n);
	}
}

void LuaMod::ApplyDamageTable(Mob *self, DamageHitInfo &hit, bool &ignoreDefault) {
	int start = lua_gettop(L);

	try {
		if (!m_has_apply_damage_table) {
			return;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, package_name_.c_str());
		lua_getfield(L, -1, "ApplyDamageTable");

		Lua_Mob l_self(self);
		luabind::adl::object e = luabind::newtable(L);
		e["self"] = l_self;

		PutDamageHitInfo(L, e, hit);
		e.push(L);

		if (lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			parser_->AddError(error);
			lua_pop(L, 2);
			return;
		}

		if (lua_type(L, -1) == LUA_TTABLE) {
			luabind::adl::object ret(luabind::from_stack(L, -1));
			auto IgnoreDefaultObj = ret["IgnoreDefault"];
			if (luabind::type(IgnoreDefaultObj) == LUA_TBOOLEAN) {
				ignoreDefault = ignoreDefault || luabind::object_cast<bool>(IgnoreDefaultObj);
			}

			GetDamageHitInfo(ret, hit);
		}
	}
	catch (std::exception &ex) {
		parser_->AddError(ex.what());
	}

	int end = lua_gettop(L);
	int n = end - start;
	if (n > 0) {
		lua_pop(L, n);
	}
}

void LuaMod::AvoidDamage(Mob *self, Mob *other, DamageHitInfo &hit, bool &returnValue, bool &ignoreDefault) {
	int start = lua_gettop(L);

	try {
		if (!m_has_avoid_damage) {
			return;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, package_name_.c_str());
		lua_getfield(L, -1, "AvoidDamage");

		Lua_Mob l_self(self);
		Lua_Mob l_other(other);
		luabind::adl::object e = luabind::newtable(L);
		e["self"] = l_self;
		e["other"] = l_other;

		PutDamageHitInfo(L, e, hit);
		e.push(L);

		if (lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			parser_->AddError(error);
			lua_pop(L, 2);
			return;
		}

		if (lua_type(L, -1) == LUA_TTABLE) {
			luabind::adl::object ret(luabind::from_stack(L, -1));
			auto IgnoreDefaultObj = ret["IgnoreDefault"];
			if (luabind::type(IgnoreDefaultObj) == LUA_TBOOLEAN) {
				ignoreDefault = ignoreDefault || luabind::object_cast<bool>(IgnoreDefaultObj);
			}

			auto returnValueObj = ret["ReturnValue"];
			if (luabind::type(returnValueObj) == LUA_TBOOLEAN) {
				returnValue = luabind::object_cast<bool>(returnValueObj);
			}

			GetDamageHitInfo(ret, hit);
		}
	}
	catch (std::exception &ex) {
		parser_->AddError(ex.what());
	}

	int end = lua_gettop(L);
	int n = end - start;
	if (n > 0) {
		lua_pop(L, n);
	}
}

void LuaMod::CheckHitChance(Mob *self, Mob* other, DamageHitInfo &hit, bool &returnValue, bool &ignoreDefault) {
	int start = lua_gettop(L);

	try {
		if (!m_has_check_hit_chance) {
			return;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, package_name_.c_str());
		lua_getfield(L, -1, "CheckHitChance");

		Lua_Mob l_self(self);
		Lua_Mob l_other(other);
		luabind::adl::object e = luabind::newtable(L);
		e["self"] = l_self;
		e["other"] = l_other;

		PutDamageHitInfo(L, e, hit);
		e.push(L);

		if (lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			parser_->AddError(error);
			lua_pop(L, 2);
			return;
		}

		if (lua_type(L, -1) == LUA_TTABLE) {
			luabind::adl::object ret(luabind::from_stack(L, -1));
			auto IgnoreDefaultObj = ret["IgnoreDefault"];
			if (luabind::type(IgnoreDefaultObj) == LUA_TBOOLEAN) {
				ignoreDefault = ignoreDefault || luabind::object_cast<bool>(IgnoreDefaultObj);
			}

			auto returnValueObj = ret["ReturnValue"];
			if (luabind::type(returnValueObj) == LUA_TBOOLEAN) {
				returnValue = luabind::object_cast<bool>(returnValueObj);
			}

			GetDamageHitInfo(ret, hit);
		}
	}
	catch (std::exception &ex) {
		parser_->AddError(ex.what());
	}

	int end = lua_gettop(L);
	int n = end - start;
	if (n > 0) {
		lua_pop(L, n);
	}
}

void LuaMod::CommonOutgoingHitSuccess(Mob *self, Mob *other, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault)
{
	int start = lua_gettop(L);

	try {
		if (!m_has_common_outgoing_hit_success) {
			return;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, package_name_.c_str());
		lua_getfield(L, -1, "CommonOutgoingHitSuccess");

		Lua_Mob l_self(self);
		Lua_Mob l_other(other);
		luabind::adl::object e = luabind::newtable(L);
		e["self"] = l_self;
		e["other"] = l_other;

		PutDamageHitInfo(L, e, hit);
		PutExtraAttackOptions(L, e, opts);
		e.push(L);

		if (lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			parser_->AddError(error);
			lua_pop(L, 2);
			return;
		}

		if (lua_type(L, -1) == LUA_TTABLE) {
			luabind::adl::object ret(luabind::from_stack(L, -1));
			auto IgnoreDefaultObj = ret["IgnoreDefault"];
			if (luabind::type(IgnoreDefaultObj) == LUA_TBOOLEAN) {
				ignoreDefault = ignoreDefault || luabind::object_cast<bool>(IgnoreDefaultObj);
			}

			GetDamageHitInfo(ret, hit);
			GetExtraAttackOptions(ret, opts);
		}
	}
	catch (std::exception &ex) {
		parser_->AddError(ex.what());
	}

	int end = lua_gettop(L);
	int n = end - start;
	if (n > 0) {
		lua_pop(L, n);
	}
}

void LuaMod::TryCriticalHit(Mob *self, Mob *defender, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault) {
	int start = lua_gettop(L);

	try {
		if (!m_has_try_critical_hit) {
			return;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, package_name_.c_str());
		lua_getfield(L, -1, "TryCriticalHit");

		Lua_Mob l_self(self);
		Lua_Mob l_other(defender);
		luabind::adl::object e = luabind::newtable(L);
		e["self"] = l_self;
		e["other"] = l_other;

		PutDamageHitInfo(L, e, hit);
		PutExtraAttackOptions(L, e, opts);
		e.push(L);

		if (lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			parser_->AddError(error);
			lua_pop(L, 2);
			return;
		}

		if (lua_type(L, -1) == LUA_TTABLE) {
			luabind::adl::object ret(luabind::from_stack(L, -1));
			auto IgnoreDefaultObj = ret["IgnoreDefault"];
			if (luabind::type(IgnoreDefaultObj) == LUA_TBOOLEAN) {
				ignoreDefault = ignoreDefault || luabind::object_cast<bool>(IgnoreDefaultObj);
			}

			GetDamageHitInfo(ret, hit);
			GetExtraAttackOptions(ret, opts);
		}
	}
	catch (std::exception &ex) {
		parser_->AddError(ex.what());
	}

	int end = lua_gettop(L);
	int n = end - start;
	if (n > 0) {
		lua_pop(L, n);
	}
}

void LuaMod::GetRequiredAAExperience(Client *self, uint32 &returnValue, bool &ignoreDefault)
{
	int start = lua_gettop(L);

	try {
		if (!m_has_get_required_aa_experience) {
			return;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, package_name_.c_str());
		lua_getfield(L, -1, "GetRequiredAAExperience");

		Lua_Client l_self(self);
		luabind::adl::object e = luabind::newtable(L);
		e["self"] = l_self;
		e.push(L);

		if (lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			parser_->AddError(error);
			lua_pop(L, 2);
			return;
		}

		if (lua_type(L, -1) == LUA_TTABLE) {
			luabind::adl::object ret(luabind::from_stack(L, -1));
			auto IgnoreDefaultObj = ret["IgnoreDefault"];
			if (luabind::type(IgnoreDefaultObj) == LUA_TBOOLEAN) {
				ignoreDefault = ignoreDefault || luabind::object_cast<bool>(IgnoreDefaultObj);
			}

			auto returnValueObj = ret["ReturnValue"];
			if (luabind::type(returnValueObj) == LUA_TNUMBER) {
				returnValue = luabind::object_cast<uint32>(returnValueObj);
			}
		}
	}
	catch (std::exception &ex) {
		parser_->AddError(ex.what());
	}

	int end = lua_gettop(L);
	int n = end - start;
	if (n > 0) {
		lua_pop(L, n);
	}
}

void LuaMod::GetEXPForLevel(Client *self, uint16 level, uint32 &returnValue, bool &ignoreDefault) {
	int start = lua_gettop(L);

	try {
		if (!m_has_get_exp_for_level) {
			return;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, package_name_.c_str());
		lua_getfield(L, -1, "GetEXPForLevel");

		Lua_Client l_self(self);
		luabind::adl::object e = luabind::newtable(L);
		e["self"] = l_self;
		e["level"] = level;
		e.push(L);

		if (lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			parser_->AddError(error);
			lua_pop(L, 2);
			return;
		}

		if (lua_type(L, -1) == LUA_TTABLE) {
			luabind::adl::object ret(luabind::from_stack(L, -1));
			auto IgnoreDefaultObj = ret["IgnoreDefault"];
			if (luabind::type(IgnoreDefaultObj) == LUA_TBOOLEAN) {
				ignoreDefault = ignoreDefault || luabind::object_cast<bool>(IgnoreDefaultObj);
			}

			auto returnValueObj = ret["ReturnValue"];
			if (luabind::type(returnValueObj) == LUA_TNUMBER) {
				returnValue = luabind::object_cast<uint32>(returnValueObj);
			}
		}
	}
	catch (std::exception &ex) {
		parser_->AddError(ex.what());
	}

	int end = lua_gettop(L);
	int n = end - start;
	if (n > 0) {
		lua_pop(L, n);
	}
}


void LuaMod::SetEXP(Mob *self, ExpSource exp_source, uint64 current_exp, uint64 set_exp, bool is_rezz_exp, uint64 &return_value, bool &ignore_default)
{
	int start = lua_gettop(L);

	try {
		if (!m_has_set_exp) {
			return;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, package_name_.c_str());
		lua_getfield(L, -1, "SetEXP");

		Lua_Mob l_self(self);
		luabind::adl::object e = luabind::newtable(L);
		e["self"] = l_self;
		e["exp_source"] = exp_source;
		e["current_exp"] = current_exp;
		e["set_exp"] = set_exp;
		e["is_rezz_exp"] = is_rezz_exp;

		e.push(L);

		if (lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			parser_->AddError(error);
			lua_pop(L, 2);
			return;
		}

		if (lua_type(L, -1) == LUA_TTABLE) {
			luabind::adl::object ret(luabind::from_stack(L, -1));
			auto ignore_default_obj = ret["ignore_default"];
			if (luabind::type(ignore_default_obj) == LUA_TBOOLEAN) {
				ignore_default = ignore_default || luabind::object_cast<bool>(ignore_default_obj);
			}

			auto return_value_obj = ret["return_value"];
			if (luabind::type(return_value_obj) == LUA_TNUMBER) {
				return_value = luabind::object_cast<int64>(return_value_obj);
			}
		}
	}
	catch (std::exception &ex) {
		parser_->AddError(ex.what());
	}

	int end = lua_gettop(L);
	int n = end - start;
	if (n > 0) {
		lua_pop(L, n);
	}
}

void LuaMod::SetAAEXP(Mob *self, ExpSource exp_source, uint64 current_aa_exp, uint64 set_aa_exp, bool is_rezz_exp, uint64 &return_value, bool &ignore_default)
{
	int start = lua_gettop(L);

	try {
		if (!m_has_set_aa_exp) {
			return;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, package_name_.c_str());
		lua_getfield(L, -1, "SetAAEXP");

		Lua_Mob l_self(self);
		luabind::adl::object e = luabind::newtable(L);
		e["self"] = l_self;
		e["exp_source"] = exp_source;
		e["current_aa_exp"] = current_aa_exp;
		e["set_aa_exp"] = set_aa_exp;
		e["is_rezz_exp"] = is_rezz_exp;

		e.push(L);

		if (lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			parser_->AddError(error);
			lua_pop(L, 2);
			return;
		}

		if (lua_type(L, -1) == LUA_TTABLE) {
			luabind::adl::object ret(luabind::from_stack(L, -1));
			auto ignore_default_obj = ret["ignore_default"];
			if (luabind::type(ignore_default_obj) == LUA_TBOOLEAN) {
				ignore_default = ignore_default || luabind::object_cast<bool>(ignore_default_obj);
			}

			auto return_value_obj = ret["return_value"];
			if (luabind::type(return_value_obj) == LUA_TNUMBER) {
				return_value = luabind::object_cast<int64>(return_value_obj);
			}
		}
	}
	catch (std::exception &ex) {
		parser_->AddError(ex.what());
	}

	int end = lua_gettop(L);
	int n = end - start;
	if (n > 0) {
		lua_pop(L, n);
	}
}

void LuaMod::GetExperienceForKill(Client *self, Mob *against, uint64 &returnValue, bool &ignoreDefault)
{
	int start = lua_gettop(L);

	try {
		if (!m_has_get_experience_for_kill) {
			return;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, package_name_.c_str());
		lua_getfield(L, -1, "GetExperienceForKill");

		Lua_Client l_self(self);
		Lua_Mob l_other(against);
		luabind::adl::object e = luabind::newtable(L);
		e["self"] = l_self;
		e["other"] = l_other;
		e.push(L);

		if (lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			parser_->AddError(error);
			lua_pop(L, 2);
			return;
		}

		if (lua_type(L, -1) == LUA_TTABLE) {
			luabind::adl::object ret(luabind::from_stack(L, -1));
			auto IgnoreDefaultObj = ret["IgnoreDefault"];
			if (luabind::type(IgnoreDefaultObj) == LUA_TBOOLEAN) {
				ignoreDefault = ignoreDefault || luabind::object_cast<bool>(IgnoreDefaultObj);
			}

			auto returnValueObj = ret["ReturnValue"];
			if (luabind::type(returnValueObj) == LUA_TNUMBER) {
				returnValue = luabind::object_cast<uint64>(returnValueObj);
			}
		}
	}
	catch (std::exception &ex) {
		parser_->AddError(ex.what());
	}

	int end = lua_gettop(L);
	int n = end - start;
	if (n > 0) {
		lua_pop(L, n);
	}
}

void LuaMod::IsImmuneToSpell(Mob *self, Mob* caster, uint16 spell_id, bool &return_value, bool &ignore_default)
{
	int start = lua_gettop(L);

	try {
		if (!m_has_is_immune_to_spell) {
			return;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, package_name_.c_str());
		lua_getfield(L, -1, "IsImmuneToSpell");

		Lua_Mob l_self(self);
		Lua_Mob l_other(caster);
		luabind::adl::object e = luabind::newtable(L);
		e["self"] = l_self;
		e["caster"] = l_other;
		e["spell_id"] = spell_id;

		e.push(L);

		if (lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			parser_->AddError(error);
			lua_pop(L, 2);
			return;
		}

		if (lua_type(L, -1) == LUA_TTABLE) {
			luabind::adl::object ret(luabind::from_stack(L, -1));
			auto ignore_default_obj = ret["ignore_default"];
			if (luabind::type(ignore_default_obj) == LUA_TBOOLEAN) {
				ignore_default = ignore_default || luabind::object_cast<bool>(ignore_default_obj);
			}

			auto return_value_obj = ret["return_value"];
			if (luabind::type(return_value_obj) == LUA_TBOOLEAN) {
				return_value = luabind::object_cast<bool>(return_value_obj);
			}
		}
	}
	catch (std::exception &ex) {
		parser_->AddError(ex.what());
	}

	int end = lua_gettop(L);
	int n = end - start;
	if (n > 0) {
		lua_pop(L, n);
	}
}

void LuaMod::CalcSpellEffectValue_formula(Mob *self, uint32 formula, int64 base_value, int64 max_value, int caster_level, uint16 spell_id, int ticsremaining, int64 &returnValue, bool &ignoreDefault)
{
	int start = lua_gettop(L);
	int64 retval = 0;

	try {
		if (!m_has_calc_spell_effect_value_formula) {
			return;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, package_name_.c_str());
		lua_getfield(L, -1, "CalcSpellEffectValue_formula");

		Lua_Mob l_self(self);
		luabind::adl::object e = luabind::newtable(L);
		e["self"] = l_self;
		e["formula"] = formula;
		e["base_value"] = base_value;
		e["max_value"] = max_value;
		e["caster_level"] = caster_level;
		e["spell_id"] = spell_id;
		e["ticsremaining"] = ticsremaining;
		e.push(L);

		if (lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			parser_->AddError(error);
			lua_pop(L, 2);
			return;
		}

		if (lua_type(L, -1) == LUA_TTABLE) {
			luabind::adl::object ret(luabind::from_stack(L, -1));
			auto IgnoreDefaultObj = ret["IgnoreDefault"];
			if (luabind::type(IgnoreDefaultObj) == LUA_TBOOLEAN) {
				ignoreDefault = ignoreDefault || luabind::object_cast<bool>(IgnoreDefaultObj);
			}

			auto returnValueObj = ret["ReturnValue"];
			if (luabind::type(returnValueObj) == LUA_TNUMBER) {
				returnValue = luabind::object_cast<int64>(returnValueObj);
			}
		}
	}
	catch (std::exception &ex) {
		parser_->AddError(ex.what());
	}

	int end = lua_gettop(L);
	int n = end - start;
	if (n > 0) {
		lua_pop(L, n);
	}
}


void LuaMod::RegisterBug(Client *self, BaseBugReportsRepository::BugReports bug, bool &ignore_default)
{
	int start = lua_gettop(L);

	try {
		if (!m_has_register_bug) {
			return;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, package_name_.c_str());
		lua_getfield(L, -1, "RegisterBug");

		Lua_Client l_self(self);
		luabind::adl::object e = luabind::newtable(L);
		e["self"]                = l_self;
		e["zone"]                = bug.zone;
		e["client_version_id"]   = bug.client_version_id;
		e["client_version_name"] = bug.client_version_name;
		e["account_id"]          = bug.account_id;
		e["character_id"]        = bug.character_id;
		e["character_name"]      = bug.character_name;
		e["reporter_spoof"]      = bug.reporter_spoof;
		e["category_id"]         = bug.category_id;
		e["category_name"]       = bug.category_name;
		e["reporter_name"]       = bug.reporter_name;
		e["ui_path"]             = bug.ui_path;
		e["pos_x"]               = bug.pos_x;
		e["pos_y"]               = bug.pos_y;
		e["pos_z"]               = bug.pos_z;
		e["heading"]             = bug.heading;
		e["time_played"]         = bug.time_played;
		e["target_id"]           = bug.target_id;
		e["target_name"]         = bug.target_name;
		e["optional_info_mask"]  = bug.optional_info_mask;
		e["_can_duplicate"]      = bug._can_duplicate;
		e["_crash_bug"]          = bug._crash_bug;
		e["_target_info"]        = bug._target_info;
		e["_character_flags"]    = bug._character_flags;
		e["_unknown_value"]      = bug._unknown_value;
		e["bug_report"]          = bug.bug_report;
		e["system_info"]         = bug.system_info;

		e.push(L);

		if (lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			parser_->AddError(error);
			lua_pop(L, 2);
			return;
		}

		if (lua_type(L, -1) == LUA_TTABLE) {
			luabind::adl::object ret(luabind::from_stack(L, -1));
			auto ignore_default_obj = ret["ignore_default"];
			if (luabind::type(ignore_default_obj) == LUA_TBOOLEAN) {
				ignore_default = ignore_default || luabind::object_cast<bool>(ignore_default_obj);
			}
		}
	}
	catch (std::exception &ex) {
		parser_->AddError(ex.what());
	}

	int end = lua_gettop(L);
	int n = end - start;
	if (n > 0) {
		lua_pop(L, n);
	}
}


void LuaMod::CommonDamage(Mob *self, Mob* attacker, int64 value, uint16 spell_id, int skill_used, bool avoidable, int8 buff_slot, bool buff_tic, int special, int64 &return_value, bool &ignore_default)
{
	int start = lua_gettop(L);

	try {
		if (!m_has_common_damage) {
			return;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, package_name_.c_str());
		lua_getfield(L, -1, "CommonDamage");

		Lua_Mob l_self(self);
		Lua_Mob l_other(attacker);
		luabind::adl::object e = luabind::newtable(L);
		e["self"] = l_self;
		e["attacker"] = l_other;
		e["value"] = value;
		e["spell_id"] = spell_id;
		e["skill_used"] = skill_used;
		e["avoidable"] = avoidable;
		e["buff_slot"] = buff_slot;
		e["buff_tic"] = buff_tic;
		e["special"] = special;

		e.push(L);

		if (lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			parser_->AddError(error);
			lua_pop(L, 2);
			return;
		}

		if (lua_type(L, -1) == LUA_TTABLE) {
			luabind::adl::object ret(luabind::from_stack(L, -1));
			auto ignore_default_obj = ret["ignore_default"];
			if (luabind::type(ignore_default_obj) == LUA_TBOOLEAN) {
				ignore_default = ignore_default || luabind::object_cast<bool>(ignore_default_obj);
			}

			auto return_value_obj = ret["return_value"];
			if (luabind::type(return_value_obj) == LUA_TNUMBER) {
				return_value = luabind::object_cast<int64>(return_value_obj);
			}
		}
	}
	catch (std::exception &ex) {
		parser_->AddError(ex.what());
	}

	int end = lua_gettop(L);
	int n = end - start;
	if (n > 0) {
		lua_pop(L, n);
	}
}


void LuaMod::HealDamage(Mob *self, Mob* caster, uint64 value, uint16 spell_id, uint64 &return_value, bool &ignore_default)
{
	int start = lua_gettop(L);

	try {
		if (!m_has_heal_damage) {
			return;
		}

		lua_getfield(L, LUA_REGISTRYINDEX, package_name_.c_str());
		lua_getfield(L, -1, "HealDamage");

		Lua_Mob l_self(self);
		Lua_Mob l_other(caster);
		luabind::adl::object e = luabind::newtable(L);
		e["self"] = l_self;
		e["caster"] = l_other;
		e["value"] = value;
		e["spell_id"] = spell_id;

		e.push(L);

		if (lua_pcall(L, 1, 1, 0)) {
			std::string error = lua_tostring(L, -1);
			parser_->AddError(error);
			lua_pop(L, 2);
			return;
		}

		if (lua_type(L, -1) == LUA_TTABLE) {
			luabind::adl::object ret(luabind::from_stack(L, -1));
			auto ignore_default_obj = ret["ignore_default"];
			if (luabind::type(ignore_default_obj) == LUA_TBOOLEAN) {
				ignore_default = ignore_default || luabind::object_cast<bool>(ignore_default_obj);
			}

			auto return_value_obj = ret["return_value"];
			if (luabind::type(return_value_obj) == LUA_TNUMBER) {
				return_value = luabind::object_cast<int64>(return_value_obj);
			}
		}
	}
	catch (std::exception &ex) {
		parser_->AddError(ex.what());
	}

	int end = lua_gettop(L);
	int n = end - start;
	if (n > 0) {
		lua_pop(L, n);
	}
}

#endif // LUA_EQEMU
