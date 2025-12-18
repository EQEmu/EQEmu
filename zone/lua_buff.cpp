#ifdef LUA_EQEMU

#include "lua_buff.h"

#include "lua.hpp"
#include "luabind/iterator_policy.hpp"
#include "luabind/luabind.hpp"

uint16 Lua_Buff::GetCasterID()
{
	Lua_Safe_Call_Int();
	return self->casterid;
}

uint8 Lua_Buff::GetCasterLevel()
{
	Lua_Safe_Call_Int();
	return self->casterlevel;
}

std::string Lua_Buff::GetCasterName()
{
	Lua_Safe_Call_String();
	return self->caster_name;
}

int Lua_Buff::GetCastOnX()
{
	Lua_Safe_Call_Int();
	return self->caston_x;
}

int Lua_Buff::GetCastOnY()
{
	Lua_Safe_Call_Int();
	return self->caston_y;
}

int Lua_Buff::GetCastOnZ()
{
	Lua_Safe_Call_Int();
	return self->caston_z;
}

uint32 Lua_Buff::GetCounters()
{
	Lua_Safe_Call_Int();
	return self->counters;
}

uint32 Lua_Buff::GetDOTRune()
{
	Lua_Safe_Call_Int();
	return self->dot_rune;
}

int Lua_Buff::GetExtraDIChance()
{
	Lua_Safe_Call_Int();
	return self->ExtraDIChance;
}

uint32 Lua_Buff::GetInstrumentModifier()
{
	Lua_Safe_Call_Int();
	return self->instrument_mod;
}

uint32 Lua_Buff::GetMagicRune()
{
	Lua_Safe_Call_Int();
	return self->magic_rune;
}

uint32 Lua_Buff::GetMeleeRune()
{
	Lua_Safe_Call_Int();
	return self->melee_rune;
}

uint32 Lua_Buff::GetNumberOfHits()
{
	Lua_Safe_Call_Int();
	return self->hit_number;
}

int16 Lua_Buff::GetRootBreakChance()
{
	Lua_Safe_Call_Int();
	return self->RootBreakChance;
}

uint16 Lua_Buff::GetSpellID()
{
	Lua_Safe_Call_Int();
	return self->spellid;
}

int Lua_Buff::GetTicsRemaining()
{
	Lua_Safe_Call_Int();
	return self->ticsremaining;
}

int Lua_Buff::GetVirusSpreadTime()
{
	Lua_Safe_Call_Int();
	return self->virus_spread_time;
}

bool Lua_Buff::IsCasterClient()
{
	Lua_Safe_Call_Bool();
	return self->client;
}

bool Lua_Buff::IsPersistentBuff()
{
	Lua_Safe_Call_Bool();
	return self->persistant_buff;
}

bool Lua_Buff::SendsClientUpdate()
{
	Lua_Safe_Call_Bool();
	return self->UpdateClient;
}

luabind::scope lua_register_buff() {
	return luabind::class_<Lua_Buff>("Buff")
	.def(luabind::constructor<>())
	.def("GetCasterID", &Lua_Buff::GetCasterID)
	.def("GetCasterLevel", &Lua_Buff::GetCasterLevel)
	.def("GetCasterName", &Lua_Buff::GetCasterName)
	.def("GetCastOnX", &Lua_Buff::GetCastOnX)
	.def("GetCastOnY", &Lua_Buff::GetCastOnY)
	.def("GetCastOnZ", &Lua_Buff::GetCastOnZ)
	.def("GetCounters", &Lua_Buff::GetCounters)
	.def("GetDOTRune", &Lua_Buff::GetDOTRune)
	.def("GetExtraDIChance", &Lua_Buff::GetExtraDIChance)
	.def("GetInstrumentModifier", &Lua_Buff::GetInstrumentModifier)
	.def("GetMagicRune", &Lua_Buff::GetMagicRune)
	.def("GetMeleeRune", &Lua_Buff::GetMeleeRune)
	.def("GetNumberOfHits", &Lua_Buff::GetNumberOfHits)
	.def("GetRootBreakChance", &Lua_Buff::GetRootBreakChance)
	.def("GetSpellID", &Lua_Buff::GetSpellID)
	.def("GetTicsRemaining", &Lua_Buff::GetTicsRemaining)
	.def("GetVirusSpreadTime", &Lua_Buff::GetVirusSpreadTime)
	.def("IsCasterClient", &Lua_Buff::IsCasterClient)
	.def("IsPersistentBuff", &Lua_Buff::IsPersistentBuff)
	.def("SendsClientUpdate", &Lua_Buff::SendsClientUpdate);
}

#endif // LUA_EQEMU
