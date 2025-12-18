#ifdef LUA_EQEMU

#include "lua_merc.h"

#include "zone/lua_client.h"
#include "zone/lua_group.h"
#include "zone/lua_item.h"
#include "zone/lua_iteminst.h"
#include "zone/lua_mob.h"
#include "zone/merc.h"

#include "lua.hpp"
#include "luabind/luabind.hpp"

uint32 Lua_Merc::GetCostFormula()
{
	Lua_Safe_Call_Int();
	return self->GetCostFormula();
}

Lua_Group Lua_Merc::GetGroup()
{
	Lua_Safe_Call_Class(Lua_Group);
	return self->GetGroup();
}

int Lua_Merc::GetHatedCount()
{
	Lua_Safe_Call_Int();
	return self->GetHatedCount();
}

float Lua_Merc::GetMaxMeleeRangeToTarget(Lua_Mob target)
{
	Lua_Safe_Call_Real();
	return self->GetMaxMeleeRangeToTarget(target);
}

uint32 Lua_Merc::GetMercenaryCharacterID()
{
	Lua_Safe_Call_Int();
	return self->GetMercenaryCharacterID();
}

uint32 Lua_Merc::GetMercenaryID()
{
	Lua_Safe_Call_Int();
	return self->GetMercenaryID();
}

uint32 Lua_Merc::GetMercenaryNameType()
{
	Lua_Safe_Call_Int();
	return self->GetMercNameType();
}

Lua_Client Lua_Merc::GetMercenaryOwner()
{
	Lua_Safe_Call_Class(Lua_Client);
	return Lua_Client(self->GetMercenaryOwner());
}

uint32 Lua_Merc::GetMercenarySubtype()
{
	Lua_Safe_Call_Int();
	return self->GetMercenarySubType();
}

uint32 Lua_Merc::GetMercenaryTemplateID()
{
	Lua_Safe_Call_Int();
	return self->GetMercenaryTemplateID();
}

uint32 Lua_Merc::GetMercenaryType()
{
	Lua_Safe_Call_Int();
	return self->GetMercenaryType();
}

Lua_Mob Lua_Merc::GetOwner()
{
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetOwner());
}

Lua_Mob Lua_Merc::GetOwnerOrSelf()
{
	Lua_Safe_Call_Class(Lua_Mob);
	return Lua_Mob(self->GetOwnerOrSelf());
}

uint8 Lua_Merc::GetProficiencyID()
{
	Lua_Safe_Call_Int();
	return self->GetProficiencyID();
}

uint8 Lua_Merc::GetStance()
{
	Lua_Safe_Call_Int();
	return self->GetStance();
}

uint8 Lua_Merc::GetTierID()
{
	Lua_Safe_Call_Int();
	return self->GetTierID();
}

bool Lua_Merc::HasOrMayGetAggro()
{
	Lua_Safe_Call_Bool();
	return self->HasOrMayGetAggro();
}

bool Lua_Merc::IsMercenaryCaster()
{
	Lua_Safe_Call_Bool();
	return self->IsMercCaster();
}

bool Lua_Merc::IsMercenaryCasterCombatRange(Lua_Mob target)
{
	Lua_Safe_Call_Bool();
	return self->IsMercCasterCombatRange(target);
}

bool Lua_Merc::IsSitting()
{
	Lua_Safe_Call_Bool();
	return self->IsSitting();
}

bool Lua_Merc::IsStanding()
{
	Lua_Safe_Call_Bool();
	return self->IsStanding();
}

void Lua_Merc::ScaleStats(int scale_percentage)
{
	Lua_Safe_Call_Void();
	self->ScaleStats(scale_percentage);
}

void Lua_Merc::ScaleStats(int scale_percentage, bool set_to_max)
{
	Lua_Safe_Call_Void();
	self->ScaleStats(scale_percentage, set_to_max);
}

void Lua_Merc::SendPayload(int payload_id, std::string payload_value)
{
	Lua_Safe_Call_Void();
	self->SendPayload(payload_id, payload_value);
}

void Lua_Merc::SetTarget(Lua_Mob target)
{
	Lua_Safe_Call_Void();
	self->SetTarget(target);
}

void Lua_Merc::Signal(int signal_id)
{
	Lua_Safe_Call_Void();
	self->Signal(signal_id);
}

void Lua_Merc::Sit()
{
	Lua_Safe_Call_Void();
	self->Sit();
}

void Lua_Merc::Stand()
{
	Lua_Safe_Call_Void();
	self->Stand();
}

bool Lua_Merc::Suspend()
{
	Lua_Safe_Call_Bool();
	return self->Suspend();
}

bool Lua_Merc::UseDiscipline(uint16 spell_id, uint16 target_id)
{
	Lua_Safe_Call_Bool();
	return self->UseDiscipline(spell_id, target_id);
}

luabind::scope lua_register_merc() {
	return luabind::class_<Lua_Merc, Lua_Mob>("Merc")
	.def(luabind::constructor<>())
	.def("GetCostFormula", &Lua_Merc::GetCostFormula)
	.def("GetGroup", &Lua_Merc::GetGroup)
	.def("GetHatedCount", &Lua_Merc::GetHatedCount)
	.def("GetMaxMeleeRangeToTarget", &Lua_Merc::GetMaxMeleeRangeToTarget)
	.def("GetMercenaryCharacterID", &Lua_Merc::GetMercenaryCharacterID)
	.def("GetMercenaryID", &Lua_Merc::GetMercenaryID)
	.def("GetMercenaryNameType", &Lua_Merc::GetMercenaryNameType)
	.def("GetMercenaryOwner", &Lua_Merc::GetMercenaryOwner)
	.def("GetMercenarySubtype", &Lua_Merc::GetMercenarySubtype)
	.def("GetMercenaryTemplateID", &Lua_Merc::GetMercenaryTemplateID)
	.def("GetMercenaryType", &Lua_Merc::GetMercenaryType)
	.def("GetOwner", &Lua_Merc::GetOwner)
	.def("GetOwnerOrSelf", &Lua_Merc::GetOwnerOrSelf)
	.def("GetProficiencyID", &Lua_Merc::GetProficiencyID)
	.def("GetStance", &Lua_Merc::GetStance)
	.def("GetTierID", &Lua_Merc::GetTierID)
	.def("HasOrMayGetAggro", &Lua_Merc::HasOrMayGetAggro)
	.def("IsMercenaryCaster", &Lua_Merc::IsMercenaryCaster)
	.def("IsMercenaryCasterCombatRange", &Lua_Merc::IsMercenaryCasterCombatRange)
	.def("IsSitting", &Lua_Merc::IsSitting)
	.def("IsStanding", &Lua_Merc::IsStanding)
	.def("ScaleStats", (void(Lua_Merc::*)(int))&Lua_Merc::ScaleStats)
	.def("ScaleStats", (void(Lua_Merc::*)(int,bool))&Lua_Merc::ScaleStats)
	.def("SendPayload", &Lua_Merc::SendPayload)
	.def("SetTarget", &Lua_Merc::SetTarget)
	.def("Signal", &Lua_Merc::Signal)
	.def("Sit", &Lua_Merc::Sit)
	.def("Stand", &Lua_Merc::Stand)
	.def("Suspend", &Lua_Merc::Suspend)
	.def("UseDiscipline", &Lua_Merc::UseDiscipline);
}

#endif
