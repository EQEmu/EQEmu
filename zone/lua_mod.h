#pragma once

#include "common/repositories/bug_reports_repository.h"

#include <string>

struct lua_State;

class LuaParser;
class LuaMod
{
public:
	LuaMod(lua_State *ls, LuaParser *lp, const std::string &package_name) {
		L = ls;
		parser_ = lp;
		package_name_ = package_name;
		Init();
	}
	~LuaMod() { }
	void Init();

	void MeleeMitigation(Mob *self, Mob *attacker, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault);
	void ApplyDamageTable(Mob *self, DamageHitInfo &hit, bool &ignoreDefault);
	void AvoidDamage(Mob *self, Mob *other, DamageHitInfo &hit, bool &returnValue, bool &ignoreDefault);
	void CheckHitChance(Mob *self, Mob* other, DamageHitInfo &hit, bool &returnValue, bool &ignoreDefault);
	void CommonOutgoingHitSuccess(Mob *self, Mob* other, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault);
	void TryCriticalHit(Mob *self, Mob *defender, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault);
	void GetRequiredAAExperience(Client *self, uint32 &returnValue, bool &ignoreDefault);
	void GetEXPForLevel(Client *self, uint16 level, uint32 &returnValue, bool &ignoreDefault);
	void IsImmuneToSpell(Mob *self, Mob* caster, uint16 spell_id, bool &return_value, bool &ignore_default);
	void GetExperienceForKill(Client *self, Mob *against, uint64 &returnValue, bool &ignoreDefault);
	void CalcSpellEffectValue_formula(Mob *self, uint32 formula, int64 base_value, int64 max_value, int caster_level, uint16 spell_id, int ticsremaining, int64 &returnValue, bool &ignoreDefault);
	void UpdatePersonalFaction(Mob *self, int32 npc_value, int32 faction_id, int32 current_value, int32 temp, int32 this_faction_min, int32 this_faction_max, int32 &return_value, bool &ignore_default);
	void RegisterBug(Client *self, BaseBugReportsRepository::BugReports bug, bool &ignore_default);
	void CommonDamage(Mob *self, Mob* attacker, int64 value, uint16 spell_id, int skill_used, bool avoidable, int8 buff_slot, bool buff_tic, int special, int64 &return_value, bool &ignore_default);
	void HealDamage(Mob *self, Mob* caster, uint64 value, uint16 spell_id, uint64 &return_value, bool &ignore_default);
	void SetEXP(Mob *self, ExpSource exp_source, uint64 current_exp, uint64 set_exp, bool is_rezz_exp, uint64 &return_value, bool &ignore_default);
	void SetAAEXP(Mob *self, ExpSource exp_source, uint64 current_aa_exp, uint64 set_aa_exp, bool is_rezz_exp, uint64 &return_value, bool &ignore_default);
private:
	LuaParser *parser_;
	lua_State *L;
	std::string package_name_;

	bool m_has_melee_mitigation;
	bool m_has_apply_damage_table;
	bool m_has_avoid_damage;
	bool m_has_check_hit_chance;
	bool m_has_common_outgoing_hit_success;
	bool m_has_try_critical_hit;
	bool m_has_get_required_aa_experience;
	bool m_has_get_exp_for_level;
	bool m_has_get_experience_for_kill;
	bool m_has_calc_spell_effect_value_formula;
	bool m_has_register_bug;
	bool m_has_common_damage;
	bool m_has_heal_damage;
	bool m_has_set_exp;
	bool m_has_set_aa_exp;
	bool m_has_is_immune_to_spell;
	bool m_has_update_personal_faction;
};
