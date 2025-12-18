#include "common/features.h"

#ifdef EMBPERL_XS_CLASSES

#include "common/spdat.h"
#include "zone/embperl.h"

int Perl_Spell_GetActivated(SPDat_Spell_Struct* self)
{
	return self->activated;
}

bool Perl_Spell_GetAllowRest(SPDat_Spell_Struct* self)
{
	return self->allow_rest;
}

uint32_t Perl_Spell_GetAOEDuration(SPDat_Spell_Struct* self)
{
	return self->aoe_duration;
}

int Perl_Spell_GetAOEMaxTargets(SPDat_Spell_Struct* self)
{
	return self->aoe_max_targets;
}

float Perl_Spell_GetAOERange(SPDat_Spell_Struct* self)
{
	return self->aoe_range;
}

int Perl_Spell_GetBaseDifficulty(SPDat_Spell_Struct* self)
{
	return self->base_difficulty;
}

int Perl_Spell_GetBaseValue(SPDat_Spell_Struct* self, uint8_t slot)
{
	return self->base_value[slot];
}

int Perl_Spell_GetBonusHate(SPDat_Spell_Struct* self)
{
	return self->bonus_hate;
}

uint32_t Perl_Spell_GetBuffDuration(SPDat_Spell_Struct* self)
{
	return self->buff_duration;
}

uint32_t Perl_Spell_GetBuffDurationFormula(SPDat_Spell_Struct* self)
{
	return self->buff_duration_formula;
}

bool Perl_Spell_GetCanCastInCombat(SPDat_Spell_Struct* self)
{
	return self->can_cast_in_combat;
}

bool Perl_Spell_GetCanCastOutOfCombat(SPDat_Spell_Struct* self)
{
	return self->can_cast_out_of_combat;
}

bool Perl_Spell_GetCanMGB(SPDat_Spell_Struct* self)
{
	return self->can_mgb;
}

bool Perl_Spell_GetCastNotStanding(SPDat_Spell_Struct* self)
{
	return self->cast_not_standing;
}

std::string Perl_Spell_GetCastOnOther(SPDat_Spell_Struct* self)
{
	return self->cast_on_other;
}

std::string Perl_Spell_GetCastOnYou(SPDat_Spell_Struct* self)
{
	return self->cast_on_you;
}

int Perl_Spell_GetCastRestriction(SPDat_Spell_Struct* self)
{
	return self->cast_restriction;
}

uint32_t Perl_Spell_GetCastTime(SPDat_Spell_Struct* self)
{
	return self->cast_time;
}

int Perl_Spell_GetCasterRequirementID(SPDat_Spell_Struct* self)
{
	return self->caster_requirement_id;
}

int Perl_Spell_GetCastingAnimation(SPDat_Spell_Struct* self)
{
	return self->casting_animation;
}

int Perl_Spell_GetClasses(SPDat_Spell_Struct* self, uint8_t slot)
{
	return self->classes[slot];
}

int Perl_Spell_GetComponent(SPDat_Spell_Struct* self, uint8_t slot)
{
	return self->component[slot];
}

int Perl_Spell_GetComponentCount(SPDat_Spell_Struct* self, uint8_t slot)
{
	return self->component_count[slot];
}

int Perl_Spell_GetDeities(SPDat_Spell_Struct* self, uint8_t slot)
{
	return self->deities[slot];
}

int Perl_Spell_GetDeityAgnostic(SPDat_Spell_Struct* self)
{
	return self->deity_agnostic;
}

int Perl_Spell_GetDescriptionID(SPDat_Spell_Struct* self)
{
	return self->description_id;
}

float Perl_Spell_GetDirectionalEnd(SPDat_Spell_Struct* self)
{
	return self->directional_end;
}

float Perl_Spell_GetDirectionalStart(SPDat_Spell_Struct* self)
{
	return self->directional_start;
}

int Perl_Spell_GetDisallowSit(SPDat_Spell_Struct* self)
{
	return self->disallow_sit;
}

int Perl_Spell_GetDispelFlag(SPDat_Spell_Struct* self)
{
	return self->dispel_flag;
}

int Perl_Spell_GetEffectDescriptionID(SPDat_Spell_Struct* self)
{
	return self->effect_description_id;
}

int Perl_Spell_GetEffectID(SPDat_Spell_Struct* self, uint8_t slot)
{
	return self->effect_id[slot];
}

int Perl_Spell_GetEnduranceCost(SPDat_Spell_Struct* self)
{
	return self->endurance_cost;
}

int Perl_Spell_GetEnduranceUpkeep(SPDat_Spell_Struct* self)
{
	return self->endurance_upkeep;
}

int Perl_Spell_GetEnvironmentType(SPDat_Spell_Struct* self)
{
	return self->environment_type;
}

bool Perl_Spell_GetFeedbackable(SPDat_Spell_Struct* self)
{
	return self->feedbackable;
}

int Perl_Spell_GetFormula(SPDat_Spell_Struct* self, uint8_t slot)
{
	return self->formula[slot];
}

int Perl_Spell_GetGoodEffect(SPDat_Spell_Struct* self)
{
	return self->good_effect;
}

int Perl_Spell_GetHateAdded(SPDat_Spell_Struct* self)
{
	return self->hate_added;
}

int Perl_Spell_GetHitNumber(SPDat_Spell_Struct* self)
{
	return self->hit_number;
}

int Perl_Spell_GetHitNumberType(SPDat_Spell_Struct* self)
{
	return self->hit_number_type;
}

int Perl_Spell_GetID(SPDat_Spell_Struct* self)
{
	return self->id;
}

bool Perl_Spell_GetIsDiscipline(SPDat_Spell_Struct* self)
{
	return self->is_discipline;
}

bool Perl_Spell_GetLDoNTrap(SPDat_Spell_Struct* self)
{
	return self->ldon_trap;
}

int Perl_Spell_GetLimitValue(SPDat_Spell_Struct* self, uint8_t slot)
{
	return self->limit_value[slot];
}

int Perl_Spell_GetMana(SPDat_Spell_Struct* self)
{
	return self->mana;
}

float Perl_Spell_GetMaxDistance(SPDat_Spell_Struct* self)
{
	return self->max_distance;
}

float Perl_Spell_GetMaxDistanceMod(SPDat_Spell_Struct* self)
{
	return self->max_distance_mod;
}

int Perl_Spell_GetMaxResist(SPDat_Spell_Struct* self)
{
	return self->max_resist;
}

int Perl_Spell_GetMaxValue(SPDat_Spell_Struct* self, uint8_t slot)
{
	return self->max_value[slot];
}

float Perl_Spell_GetMinDistance(SPDat_Spell_Struct* self)
{
	return self->min_distance;
}

float Perl_Spell_GetMinDistanceMod(SPDat_Spell_Struct* self)
{
	return self->min_distance_mod;
}

float Perl_Spell_GetMinRange(SPDat_Spell_Struct* self)
{
	return self->min_range;
}

int Perl_Spell_GetMinResist(SPDat_Spell_Struct* self)
{
	return self->min_resist;
}

std::string Perl_Spell_GetName(SPDat_Spell_Struct* self)
{
	return self->name;
}

int Perl_Spell_GetNewIcon(SPDat_Spell_Struct* self)
{
	return self->new_icon;
}

int Perl_Spell_GetNimbusEffect(SPDat_Spell_Struct* self)
{
	return self->nimbus_effect;
}

bool Perl_Spell_GetNoBlock(SPDat_Spell_Struct* self)
{
	return self->no_block;
}

bool Perl_Spell_GetNoDetrimentalSpellAggro(SPDat_Spell_Struct* self)
{
	return self->no_detrimental_spell_aggro;
}

int Perl_Spell_GetNoExpendReagent(SPDat_Spell_Struct* self, uint8_t slot)
{
	return self->no_expend_reagent[slot];
}

int Perl_Spell_GetNoHealDamageItemMod(SPDat_Spell_Struct* self)
{
	return self->no_heal_damage_item_mod;
}

bool Perl_Spell_GetNoPartialResist(SPDat_Spell_Struct* self)
{
	return self->no_partial_resist;
}

bool Perl_Spell_GetNoRemove(SPDat_Spell_Struct* self)
{
	return self->no_remove;
}

int Perl_Spell_GetNoResist(SPDat_Spell_Struct* self)
{
	return self->no_resist;
}

bool Perl_Spell_GetNotFocusable(SPDat_Spell_Struct* self)
{
	return self->not_focusable;
}

bool Perl_Spell_GetNPCNoLOS(SPDat_Spell_Struct* self)
{
	return self->npc_no_los;
}

std::string Perl_Spell_GetOtherCasts(SPDat_Spell_Struct* self)
{
	return self->other_casts;
}

int Perl_Spell_GetOverrideCritChance(SPDat_Spell_Struct* self)
{
	return self->override_crit_chance;
}

int Perl_Spell_GetPCNPCOnlyFlag(SPDat_Spell_Struct* self)
{
	return self->pcnpc_only_flag;
}

bool Perl_Spell_GetPersistDeath(SPDat_Spell_Struct* self)
{
	return self->persist_death;
}

std::string Perl_Spell_GetPlayer_1(SPDat_Spell_Struct* self)
{
	return self->player_1;
}

float Perl_Spell_GetPushBack(SPDat_Spell_Struct* self)
{
	return self->push_back;
}

float Perl_Spell_GetPushUp(SPDat_Spell_Struct* self)
{
	return self->push_up;
}

int Perl_Spell_GetPVPDuration(SPDat_Spell_Struct* self)
{
	return self->pvp_duration;
}

int Perl_Spell_GetPVPDurationCap(SPDat_Spell_Struct* self)
{
	return self->pvp_duration_cap;
}

int Perl_Spell_GetPVPResistBase(SPDat_Spell_Struct* self)
{
	return self->pvp_resist_base;
}

int Perl_Spell_GetPVPResistCap(SPDat_Spell_Struct* self)
{
	return self->pvp_resist_cap;
}

int Perl_Spell_GetPVPResistPerLevel(SPDat_Spell_Struct* self)
{
	return self->pvp_resist_per_level;
}

float Perl_Spell_GetRange(SPDat_Spell_Struct* self)
{
	return self->range;
}

int Perl_Spell_GetRank(SPDat_Spell_Struct* self)
{
	return self->rank;
}

uint32_t Perl_Spell_GetRecastTime(SPDat_Spell_Struct* self)
{
	return self->recast_time;
}

int Perl_Spell_GetRecourseLink(SPDat_Spell_Struct* self)
{
	return self->recourse_link;
}

uint32_t Perl_Spell_GetRecoveryTime(SPDat_Spell_Struct* self)
{
	return self->recovery_time;
}

bool Perl_Spell_GetReflectable(SPDat_Spell_Struct* self)
{
	return self->reflectable;
}

int Perl_Spell_GetResistDifficulty(SPDat_Spell_Struct* self)
{
	return self->resist_difficulty;
}

int Perl_Spell_GetResistType(SPDat_Spell_Struct* self)
{
	return self->resist_type;
}

int Perl_Spell_GetShortBuffBox(SPDat_Spell_Struct* self)
{
	return self->short_buff_box;
}

int Perl_Spell_GetSkill(SPDat_Spell_Struct* self)
{
	return self->skill;
}

bool Perl_Spell_GetSneak(SPDat_Spell_Struct* self)
{
	return self->sneak;
}

int Perl_Spell_GetSongCap(SPDat_Spell_Struct* self)
{
	return self->song_cap;
}

int Perl_Spell_GetSpellAffectIndex(SPDat_Spell_Struct* self)
{
	return self->spell_affect_index;
}

int Perl_Spell_GetSpellCategory(SPDat_Spell_Struct* self)
{
	return self->spell_category;
}

int Perl_Spell_GetSpellClass(SPDat_Spell_Struct* self)
{
	return self->spell_class;
}

std::string Perl_Spell_GetSpellFades(SPDat_Spell_Struct* self)
{
	return self->spell_fades;
}

int Perl_Spell_GetSpellGroup(SPDat_Spell_Struct* self)
{
	return self->spell_group;
}

int Perl_Spell_GetSpellSubclass(SPDat_Spell_Struct* self)
{
	return self->spell_subclass;
}

bool Perl_Spell_GetSuspendable(SPDat_Spell_Struct* self)
{
	return self->suspendable;
}

int Perl_Spell_GetTargetType(SPDat_Spell_Struct* self)
{
	return self->target_type;
}

std::string Perl_Spell_GetTeleportZone(SPDat_Spell_Struct* self)
{
	return self->teleport_zone;
}

int Perl_Spell_GetTimeOfDay(SPDat_Spell_Struct* self)
{
	return self->time_of_day;
}

int Perl_Spell_GetTimerID(SPDat_Spell_Struct* self)
{
	return self->timer_id;
}

int Perl_Spell_GetTypeDescriptionID(SPDat_Spell_Struct* self)
{
	return self->type_description_id;
}

bool Perl_Spell_GetUninterruptable(SPDat_Spell_Struct* self)
{
	return self->uninterruptable;
}

bool Perl_Spell_GetUnstackableDOT(SPDat_Spell_Struct* self)
{
	return self->unstackable_dot;
}

int Perl_Spell_GetViralRange(SPDat_Spell_Struct* self)
{
	return self->viral_range;
}

int Perl_Spell_GetViralTargets(SPDat_Spell_Struct* self)
{
	return self->viral_targets;
}

int Perl_Spell_GetViralTimer(SPDat_Spell_Struct* self)
{
	return self->viral_timer;
}

std::string Perl_Spell_GetYouCast(SPDat_Spell_Struct* self)
{
	return self->you_cast;
}

int Perl_Spell_GetZoneType(SPDat_Spell_Struct* self)
{
	return self->zone_type;
}

void perl_register_spell()
{
	perl::interpreter perl(PERL_GET_THX);

	auto package = perl.new_class<SPDat_Spell_Struct>("Spell");
	package.add("GetActivated", &Perl_Spell_GetActivated);
	package.add("GetAllowRest", &Perl_Spell_GetAllowRest);
	package.add("GetAOEDuration", &Perl_Spell_GetAOEDuration);
	package.add("GetAOEMaxTargets", &Perl_Spell_GetAOEMaxTargets);
	package.add("GetAOERange", &Perl_Spell_GetAOERange);
	package.add("GetBaseDifficulty", &Perl_Spell_GetBaseDifficulty);
	package.add("GetBaseValue", &Perl_Spell_GetBaseValue);
	package.add("GetBonusHate", &Perl_Spell_GetBonusHate);
	package.add("GetBuffDuration", &Perl_Spell_GetBuffDuration);
	package.add("GetBuffDurationFormula", &Perl_Spell_GetBuffDurationFormula);
	package.add("GetCanCastInCombat", &Perl_Spell_GetCanCastInCombat);
	package.add("GetCanCastOutOfCombat", &Perl_Spell_GetCanCastOutOfCombat);
	package.add("GetCanMGB", &Perl_Spell_GetCanMGB);
	package.add("GetCastNotStanding", &Perl_Spell_GetCastNotStanding);
	package.add("GetCastOnOther", &Perl_Spell_GetCastOnOther);
	package.add("GetCastOnYou", &Perl_Spell_GetCastOnYou);
	package.add("GetCastRestriction", &Perl_Spell_GetCastRestriction);
	package.add("GetCastTime", &Perl_Spell_GetCastTime);
	package.add("GetCasterRequirementID", &Perl_Spell_GetCasterRequirementID);
	package.add("GetCastingAnimation", &Perl_Spell_GetCastingAnimation);
	package.add("GetClasses", &Perl_Spell_GetClasses);
	package.add("GetComponent", &Perl_Spell_GetComponent);
	package.add("GetComponentCount", &Perl_Spell_GetComponentCount);
	package.add("GetDeities", &Perl_Spell_GetDeities);
	package.add("GetDeityAgnostic", &Perl_Spell_GetDeityAgnostic);
	package.add("GetDescriptionID", &Perl_Spell_GetDescriptionID);
	package.add("GetDirectionalEnd", &Perl_Spell_GetDirectionalEnd);
	package.add("GetDirectionalStart", &Perl_Spell_GetDirectionalStart);
	package.add("GetDisallowSit", &Perl_Spell_GetDisallowSit);
	package.add("GetDispelFlag", &Perl_Spell_GetDispelFlag);
	package.add("GetEffectDescriptionID", &Perl_Spell_GetEffectDescriptionID);
	package.add("GetEffectID", &Perl_Spell_GetEffectID);
	package.add("GetEnduranceCost", &Perl_Spell_GetEnduranceCost);
	package.add("GetEnduranceUpkeep", &Perl_Spell_GetEnduranceUpkeep);
	package.add("GetEnvironmentType", &Perl_Spell_GetEnvironmentType);
	package.add("GetFeedbackable", &Perl_Spell_GetFeedbackable);
	package.add("GetFormula", &Perl_Spell_GetFormula);
	package.add("GetGoodEffect", &Perl_Spell_GetGoodEffect);
	package.add("GetHateAdded", &Perl_Spell_GetHateAdded);
	package.add("GetHitNumber", &Perl_Spell_GetHitNumber);
	package.add("GetHitNumberType", &Perl_Spell_GetHitNumberType);
	package.add("GetID", &Perl_Spell_GetID);
	package.add("GetIsDiscipline", &Perl_Spell_GetIsDiscipline);
	package.add("GetLDoNTrap", &Perl_Spell_GetLDoNTrap);
	package.add("GetLimitValue", &Perl_Spell_GetLimitValue);
	package.add("GetMana", &Perl_Spell_GetMana);
	package.add("GetMaxDistance", &Perl_Spell_GetMaxDistance);
	package.add("GetMaxDistanceMod", &Perl_Spell_GetMaxDistanceMod);
	package.add("GetMaxResist", &Perl_Spell_GetMaxResist);
	package.add("GetMaxValue", &Perl_Spell_GetMaxValue);
	package.add("GetMinDistance", &Perl_Spell_GetMinDistance);
	package.add("GetMinDistanceMod", &Perl_Spell_GetMinDistanceMod);
	package.add("GetMinRange", &Perl_Spell_GetMinRange);
	package.add("GetMinResist", &Perl_Spell_GetMinResist);
	package.add("GetName", &Perl_Spell_GetName);
	package.add("GetNewIcon", &Perl_Spell_GetNewIcon);
	package.add("GetNimbusEffect", &Perl_Spell_GetNimbusEffect);
	package.add("GetNoBlock", &Perl_Spell_GetNoBlock);
	package.add("GetNoDetrimentalSpellAggro", &Perl_Spell_GetNoDetrimentalSpellAggro);
	package.add("GetNoExpendReagent", &Perl_Spell_GetNoExpendReagent);
	package.add("GetNoHealDamageItemMod", &Perl_Spell_GetNoHealDamageItemMod);
	package.add("GetNoPartialResist", &Perl_Spell_GetNoPartialResist);
	package.add("GetNoRemove", &Perl_Spell_GetNoRemove);
	package.add("GetNoResist", &Perl_Spell_GetNoResist);
	package.add("GetNotFocusable", &Perl_Spell_GetNotFocusable);
	package.add("GetNPCNoLOS", &Perl_Spell_GetNPCNoLOS);
	package.add("GetOtherCasts", &Perl_Spell_GetOtherCasts);
	package.add("GetOverrideCritChance", &Perl_Spell_GetOverrideCritChance);
	package.add("GetPCNPCOnlyFlag", &Perl_Spell_GetPCNPCOnlyFlag);
	package.add("GetPersistDeath", &Perl_Spell_GetPersistDeath);
	package.add("GetPlayer_1", &Perl_Spell_GetPlayer_1);
	package.add("GetPushBack", &Perl_Spell_GetPushBack);
	package.add("GetPushUp", &Perl_Spell_GetPushUp);
	package.add("GetPVPDuration", &Perl_Spell_GetPVPDuration);
	package.add("GetPVPDurationCap", &Perl_Spell_GetPVPDurationCap);
	package.add("GetPVPResistBase", &Perl_Spell_GetPVPResistBase);
	package.add("GetPVPResistCap", &Perl_Spell_GetPVPResistCap);
	package.add("GetPVPResistPerLevel", &Perl_Spell_GetPVPResistPerLevel);
	package.add("GetRange", &Perl_Spell_GetRange);
	package.add("GetRank", &Perl_Spell_GetRank);
	package.add("GetRecastTime", &Perl_Spell_GetRecastTime);
	package.add("GetRecourseLink", &Perl_Spell_GetRecourseLink);
	package.add("GetRecoveryTime", &Perl_Spell_GetRecoveryTime);
	package.add("GetReflectable", &Perl_Spell_GetReflectable);
	package.add("GetResistDifficulty", &Perl_Spell_GetResistDifficulty);
	package.add("GetResistType", &Perl_Spell_GetResistType);
	package.add("GetShortBuffBox", &Perl_Spell_GetShortBuffBox);
	package.add("GetSkill", &Perl_Spell_GetSkill);
	package.add("GetSneak", &Perl_Spell_GetSneak);
	package.add("GetSongCap", &Perl_Spell_GetSongCap);
	package.add("GetSpellAffectIndex", &Perl_Spell_GetSpellAffectIndex);
	package.add("GetSpellCategory", &Perl_Spell_GetSpellCategory);
	package.add("GetSpellClass", &Perl_Spell_GetSpellClass);
	package.add("GetSpellFades", &Perl_Spell_GetSpellFades);
	package.add("GetSpellGroup", &Perl_Spell_GetSpellGroup);
	package.add("GetSpellSubclass", &Perl_Spell_GetSpellSubclass);
	package.add("GetSuspendable", &Perl_Spell_GetSuspendable);
	package.add("GetTargetType", &Perl_Spell_GetTargetType);
	package.add("GetTeleportZone", &Perl_Spell_GetTeleportZone);
	package.add("GetTimeOfDay", &Perl_Spell_GetTimeOfDay);
	package.add("GetTimerID", &Perl_Spell_GetTimerID);
	package.add("GetTypeDescriptionID", &Perl_Spell_GetTypeDescriptionID);
	package.add("GetUninterruptable", &Perl_Spell_GetUninterruptable);
	package.add("GetUnstackableDOT", &Perl_Spell_GetUnstackableDOT);
	package.add("GetViralRange", &Perl_Spell_GetViralRange);
	package.add("GetViralTargets", &Perl_Spell_GetViralTargets);
	package.add("GetViralTimer", &Perl_Spell_GetViralTimer);
	package.add("GetYouCast", &Perl_Spell_GetYouCast);
	package.add("GetZoneType", &Perl_Spell_GetZoneType);
}

#endif //EMBPERL_XS_CLASSES
