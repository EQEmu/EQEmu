#ifdef LUA_EQEMU

#include "lua_spell.h"

#include "common/spdat.h"

#include "luabind/luabind.hpp"


Lua_Spell::Lua_Spell(int id) {
	if(IsValidSpell(id)) {
		SetLuaPtrData(&spells[id]);
	} else {
		SetLuaPtrData(nullptr);
	}
}

int Lua_Spell::GetID() {
	Lua_Safe_Call_Int();
	return self->id;
}

const char *Lua_Spell::GetName() {
	Lua_Safe_Call_String();
	return self->name;
}

const char *Lua_Spell::GetPlayer1() {
	Lua_Safe_Call_String();
	return self->player_1;
}

const char *Lua_Spell::GetTeleportZone() {
	Lua_Safe_Call_String();
	return self->teleport_zone;
}

const char *Lua_Spell::GetYouCast() {
	Lua_Safe_Call_String();
	return self->you_cast;
}

const char *Lua_Spell::GetOtherCasts() {
	Lua_Safe_Call_String();
	return self->other_casts;
}

const char *Lua_Spell::GetCastOnYou() {
	Lua_Safe_Call_String();
	return self->cast_on_you;
}

const char *Lua_Spell::GetCastOnOther() {
	Lua_Safe_Call_String();
	return self->cast_on_other;
}

const char *Lua_Spell::GetSpellFades() {
	Lua_Safe_Call_String();
	return self->spell_fades;
}

float Lua_Spell::GetRange() {
	Lua_Safe_Call_Real();
	return self->range;
}

float Lua_Spell::GetAoeRange() {
	Lua_Safe_Call_Real();
	return self->aoe_range;
}

float Lua_Spell::GetPushBack() {
	Lua_Safe_Call_Real();
	return self->push_back;
}

float Lua_Spell::GetPushUp() {
	Lua_Safe_Call_Real();
	return self->push_up;
}

uint32 Lua_Spell::GetCastTime() {
	Lua_Safe_Call_Int();
	return self->cast_time;
}

uint32 Lua_Spell::GetRecoveryTime() {
	Lua_Safe_Call_Int();
	return self->recovery_time;
}

uint32 Lua_Spell::GetRecastTime() {
	Lua_Safe_Call_Int();
	return self->recast_time;
}

uint32 Lua_Spell::GetBuffdurationFormula() {
	Lua_Safe_Call_Int();
	return self->buff_duration_formula;
}

uint32 Lua_Spell::GetBuffDuration() {
	Lua_Safe_Call_Int();
	return self->buff_duration;
}

uint32 Lua_Spell::GetAEDuration() {
	Lua_Safe_Call_Int();
	return self->aoe_duration;
}

int Lua_Spell::GetMana() {
	Lua_Safe_Call_Int();
	return self->mana;
}

int Lua_Spell::GetBase(int i) {
	Lua_Safe_Call_Int();

	if(i >= 12 || i < 0) {
		return 0;
	}

	return self->base_value[i];
}

int Lua_Spell::GetBase2(int i) {
	Lua_Safe_Call_Int();

	if(i >= 12 || i < 0) {
		return 0;
	}

	return self->limit_value[i];
}

int Lua_Spell::GetMax(int i) {
	Lua_Safe_Call_Int();

	if(i >= 12 || i < 0) {
		return 0;
	}

	return self->max_value[i];
}

int Lua_Spell::GetComponents(int i) {
	Lua_Safe_Call_Int();

	if(i >= 4 || i < 0) {
		return 0;
	}

	return self->component[i];
}

int Lua_Spell::GetComponentCounts(int i) {
	Lua_Safe_Call_Int();

	if(i >= 4 || i < 0) {
		return 0;
	}

	return self->component_count[i];
}

int Lua_Spell::GetNoexpendReagent(int i) {
	Lua_Safe_Call_Int();

	if(i >= 4 || i < 0) {
		return 0;
	}

	return self->no_expend_reagent[i];
}

int Lua_Spell::GetFormula(int i) {
	Lua_Safe_Call_Int();

	if(i >= 12 || i < 0) {
		return 0;
	}

	return self->formula[i];
}

int Lua_Spell::GetGoodEffect() {
	Lua_Safe_Call_Int();
	return self->good_effect;
}

int Lua_Spell::GetActivated() {
	Lua_Safe_Call_Int();
	return self->activated;
}

int Lua_Spell::GetResistType() {
	Lua_Safe_Call_Int();
	return self->resist_type;
}

int Lua_Spell::GetEffectID(int i) {
	Lua_Safe_Call_Int();

	if(i >= 12 || i < 0) {
		return 0;
	}

	return self->effect_id[i];
}

int Lua_Spell::GetTargetType() {
	Lua_Safe_Call_Int();
	return self->target_type;
}

int Lua_Spell::GetBaseDiff() {
	Lua_Safe_Call_Int();
	return self->base_difficulty;
}

int Lua_Spell::GetSkill() {
	Lua_Safe_Call_Int();
	return self->skill;
}

int Lua_Spell::GetZoneType() {
	Lua_Safe_Call_Int();
	return self->zone_type;
}

int Lua_Spell::GetEnvironmentType() {
	Lua_Safe_Call_Int();
	return self->environment_type;
}

int Lua_Spell::GetTimeOfDay() {
	Lua_Safe_Call_Int();
	return self->time_of_day;
}

int Lua_Spell::GetClasses(int i) {
	Lua_Safe_Call_Int();

	if(i >= 16 || i < 0) {
		return 0;
	}

	return self->classes[i];
}

int Lua_Spell::GetCastingAnim() {
	Lua_Safe_Call_Int();
	return self->casting_animation;
}

int Lua_Spell::GetSpellAffectIndex() {
	Lua_Safe_Call_Int();
	return self->spell_affect_index;
}

int Lua_Spell::GetDisallowSit() {
	Lua_Safe_Call_Int();
	return self->disallow_sit;
}

int Lua_Spell::GetDeities(int i) {
	Lua_Safe_Call_Int();

	if(i >= 16 || i < 0) {
		return 0;
	}

	return self->deities[i];
}

int Lua_Spell::GetUninterruptable() {
	Lua_Safe_Call_Int();
	return self->uninterruptable;
}

int Lua_Spell::GetResistDiff() {
	Lua_Safe_Call_Int();
	return self->resist_difficulty;
}

int Lua_Spell::GetRecourseLink() {
	Lua_Safe_Call_Int();
	return self->recourse_link;
}

int Lua_Spell::GetShortBuffBox() {
	Lua_Safe_Call_Int();
	return self->short_buff_box;
}

int Lua_Spell::GetDescNum() {
	Lua_Safe_Call_Int();
	return self->description_id;
}

int Lua_Spell::GetEffectDescNum() {
	Lua_Safe_Call_Int();
	return self->effect_description_id;
}

int Lua_Spell::GetBonusHate() {
	Lua_Safe_Call_Int();
	return self->bonus_hate;
}

int Lua_Spell::GetEndurCost() {
	Lua_Safe_Call_Int();
	return self->endurance_cost;
}

int Lua_Spell::GetEndurTimerIndex() {
	Lua_Safe_Call_Int();
	return self->endurance_upkeep;
}

int Lua_Spell::GetHateAdded() {
	Lua_Safe_Call_Int();
	return self->hate_added;
}

int Lua_Spell::GetEndurUpkeep() {
	Lua_Safe_Call_Int();
	return self->endurance_upkeep;
}

int Lua_Spell::GetNumHits() {
	Lua_Safe_Call_Int();
	return self->hit_number;
}

int Lua_Spell::GetPVPResistBase() {
	Lua_Safe_Call_Int();
	return self->pvp_resist_base;
}

int Lua_Spell::GetPVPResistCalc() {
	Lua_Safe_Call_Int();
	return self->pvp_resist_per_level;
}

int Lua_Spell::GetPVPResistCap() {
	Lua_Safe_Call_Int();
	return self->pvp_resist_cap;
}

int Lua_Spell::GetSpellCategory() {
	Lua_Safe_Call_Int();
	return self->spell_category;
}

int Lua_Spell::GetPVPDuration() {
	Lua_Safe_Call_Int();
	return self->pvp_duration;
}

int Lua_Spell::GetPVPDurationCap() {
	Lua_Safe_Call_Int();
	return self->pvp_duration_cap;
}

int Lua_Spell::GetCanMGB() {
	Lua_Safe_Call_Int();
	return self->can_mgb;
}

int Lua_Spell::GetDispelFlag() {
	Lua_Safe_Call_Int();
	return self->dispel_flag;
}

int Lua_Spell::GetMinResist() {
	Lua_Safe_Call_Int();
	return self->min_resist;
}

int Lua_Spell::GetMaxResist() {
	Lua_Safe_Call_Int();
	return self->max_resist;
}

int Lua_Spell::GetViralTargets() {
	Lua_Safe_Call_Int();
	return self->viral_targets;
}

int Lua_Spell::GetViralTimer() {
	Lua_Safe_Call_Int();
	return self->viral_timer;
}

int Lua_Spell::GetNimbusEffect() {
	Lua_Safe_Call_Int();
	return self->nimbus_effect;
}

float Lua_Spell::GetDirectionalStart() {
	Lua_Safe_Call_Real();
	return self->directional_start;
}

float Lua_Spell::GetDirectionalEnd() {
	Lua_Safe_Call_Real();
	return self->directional_end;
}

int Lua_Spell::GetSpellGroup() {
	Lua_Safe_Call_Int();
	return self->spell_group;
}

int Lua_Spell::GetPowerfulFlag() {
	Lua_Safe_Call_Int();
	return self->no_resist;
}

int Lua_Spell::GetCastRestriction() {
	Lua_Safe_Call_Int();
	return self->cast_restriction;
}

bool Lua_Spell::GetAllowRest() {
	Lua_Safe_Call_Bool();
	return self->allow_rest;
}

bool Lua_Spell::GetInCombat() {
	Lua_Safe_Call_Bool();
	return self->can_cast_in_combat;
}

bool Lua_Spell::GetOutOfCombat() {
	Lua_Safe_Call_Bool();
	return self->can_cast_out_of_combat;
}

int Lua_Spell::GetAEMaxTargets() {
	Lua_Safe_Call_Int();
	return self->aoe_max_targets;
}

int Lua_Spell::GetMaxTargets() {
	Lua_Safe_Call_Int();
	return self->no_heal_damage_item_mod;
}

bool Lua_Spell::GetPersistDeath() {
	Lua_Safe_Call_Bool();
	return self->persist_death;
}

float Lua_Spell::GetMinDist() {
	Lua_Safe_Call_Real();
	return self->min_distance;
}

float Lua_Spell::GetMinDistMod() {
	Lua_Safe_Call_Real();
	return self->min_distance_mod;
}

float Lua_Spell::GetMaxDist() {
	Lua_Safe_Call_Real();
	return self->max_distance;
}

float Lua_Spell::GetMaxDistMod() {
	Lua_Safe_Call_Real();
	return self->max_distance_mod;
}

float Lua_Spell::GetMinRange() {
	Lua_Safe_Call_Real();
	return self->min_range;
}

int Lua_Spell::GetDamageShieldType() {
	Lua_Safe_Call_Int();
	return self->damage_shield_type;
}

int Lua_Spell::GetRank() {
	Lua_Safe_Call_Int();
	return self->rank;
}

luabind::scope lua_register_spell() {
	return luabind::class_<Lua_Spell>("Spell")
	.def(luabind::constructor<>())
	.def(luabind::constructor<int>())
	.property("null", &Lua_Spell::Null)
	.property("valid", &Lua_Spell::Valid)
	.def("AEDuration", &Lua_Spell::GetAEDuration)
	.def("AEMaxTargets", &Lua_Spell::GetAEMaxTargets)
	.def("Activated", &Lua_Spell::GetActivated)
	.def("AllowRest", &Lua_Spell::GetAllowRest)
	.def("AoeRange", &Lua_Spell::GetAoeRange)
	.def("Base", &Lua_Spell::GetBase)
	.def("Base2", &Lua_Spell::GetBase2)
	.def("BaseDiff", &Lua_Spell::GetBaseDiff)
	.def("BonusHate", &Lua_Spell::GetBonusHate)
	.def("BuffDuration", &Lua_Spell::GetBuffDuration)
	.def("BuffdurationFormula", &Lua_Spell::GetBuffdurationFormula)
	.def("CanMGB", &Lua_Spell::GetCanMGB)
	.def("CastOnOther", &Lua_Spell::GetCastOnOther)
	.def("CastOnYou", &Lua_Spell::GetCastOnYou)
	.def("CastRestriction", &Lua_Spell::GetCastRestriction)
	.def("CastTime", &Lua_Spell::GetCastTime)
	.def("CastingAnim", &Lua_Spell::GetCastingAnim)
	.def("Classes", &Lua_Spell::GetClasses)
	.def("ComponentCounts", &Lua_Spell::GetComponentCounts)
	.def("Components", &Lua_Spell::GetComponents)
	.def("DamageShieldType", &Lua_Spell::GetDamageShieldType)
	.def("Deities", &Lua_Spell::GetDeities)
	.def("DescNum", &Lua_Spell::GetDescNum)
	.def("DirectionalEnd", &Lua_Spell::GetDirectionalEnd)
	.def("DirectionalStart", &Lua_Spell::GetDirectionalStart)
	.def("DisallowSit", &Lua_Spell::GetDisallowSit)
	.def("DispelFlag", &Lua_Spell::GetDispelFlag)
	.def("EffectDescNum", &Lua_Spell::GetEffectDescNum)
	.def("EffectID", &Lua_Spell::GetEffectID)
	.def("EndurCost", &Lua_Spell::GetEndurCost)
	.def("EndurTimerIndex", &Lua_Spell::GetEndurTimerIndex)
	.def("EndurUpkeep", &Lua_Spell::GetEndurUpkeep)
	.def("EnvironmentType", &Lua_Spell::GetEnvironmentType)
	.def("Formula", &Lua_Spell::GetFormula)
	.def("GoodEffect", &Lua_Spell::GetGoodEffect)
	.def("HateAdded", &Lua_Spell::GetHateAdded)
	.def("ID", &Lua_Spell::GetID)
	.def("InCombat", &Lua_Spell::GetInCombat)
	.def("Mana", &Lua_Spell::GetMana)
	.def("Max", &Lua_Spell::GetMax)
	.def("MaxDist", &Lua_Spell::GetMaxDist)
	.def("MaxDistMod", &Lua_Spell::GetMaxDistMod)
	.def("MaxResist", &Lua_Spell::GetMaxResist)
	.def("MaxTargets", &Lua_Spell::GetMaxTargets)
	.def("MinDist", &Lua_Spell::GetMinDist)
	.def("MinDistMod", &Lua_Spell::GetMinDistMod)
	.def("MinRange", &Lua_Spell::GetMinRange)
	.def("MinResist", &Lua_Spell::GetMinResist)
	.def("Name", &Lua_Spell::GetName)
	.def("NimbusEffect", &Lua_Spell::GetNimbusEffect)
	.def("NoexpendReagent", &Lua_Spell::GetNoexpendReagent)
	.def("NumHits", &Lua_Spell::GetNumHits)
	.def("OtherCasts", &Lua_Spell::GetOtherCasts)
	.def("OutOfCombat", &Lua_Spell::GetOutOfCombat)
	.def("PVPDuration", &Lua_Spell::GetPVPDuration)
	.def("PVPDurationCap", &Lua_Spell::GetPVPDurationCap)
	.def("PVPResistBase", &Lua_Spell::GetPVPResistBase)
	.def("PVPResistCalc", &Lua_Spell::GetPVPResistCalc)
	.def("PVPResistCap", &Lua_Spell::GetPVPResistCap)
	.def("PersistDeath", &Lua_Spell::GetPersistDeath)
	.def("Player1", &Lua_Spell::GetPlayer1)
	.def("PowerfulFlag", &Lua_Spell::GetPowerfulFlag)
	.def("PushBack", &Lua_Spell::GetPushBack)
	.def("PushUp", &Lua_Spell::GetPushUp)
	.def("Range", &Lua_Spell::GetRange)
	.def("Rank", &Lua_Spell::GetRank)
	.def("RecastTime", &Lua_Spell::GetRecastTime)
	.def("RecourseLink", &Lua_Spell::GetRecourseLink)
	.def("RecoveryTime", &Lua_Spell::GetRecoveryTime)
	.def("ResistDiff", &Lua_Spell::GetResistDiff)
	.def("ResistType", &Lua_Spell::GetResistType)
	.def("ShortBuffBox", &Lua_Spell::GetShortBuffBox)
	.def("Skill", &Lua_Spell::GetSkill)
	.def("SpellAffectIndex", &Lua_Spell::GetSpellAffectIndex)
	.def("SpellCategory", &Lua_Spell::GetSpellCategory)
	.def("SpellFades", &Lua_Spell::GetSpellFades)
	.def("SpellGroup", &Lua_Spell::GetSpellGroup)
	.def("TargetType", &Lua_Spell::GetTargetType)
	.def("TeleportZone", &Lua_Spell::GetTeleportZone)
	.def("TimeOfDay", &Lua_Spell::GetTimeOfDay)
	.def("Uninterruptable", &Lua_Spell::GetUninterruptable)
	.def("ViralTargets", &Lua_Spell::GetViralTargets)
	.def("ViralTimer", &Lua_Spell::GetViralTimer)
	.def("YouCast", &Lua_Spell::GetYouCast)
	.def("ZoneType", &Lua_Spell::GetZoneType);
}

#endif // LUA_EQEMU
