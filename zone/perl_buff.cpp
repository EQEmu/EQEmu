#include "common/features.h"

#ifdef EMBPERL_XS_CLASSES

#include "zone/common.h"
#include "zone/embperl.h"

uint16 Perl_Buff_GetCasterID(Buffs_Struct* self)
{
	return self->casterid;
}

uint8 Perl_Buff_GetCasterLevel(Buffs_Struct* self)
{
	return self->casterlevel;
}

std::string Perl_Buff_GetCasterName(Buffs_Struct* self)
{
	return self->caster_name;
}

int Perl_Buff_GetCastOnX(Buffs_Struct* self)
{
	return self->caston_x;
}

int Perl_Buff_GetCastOnY(Buffs_Struct* self)
{
	return self->caston_y;
}

int Perl_Buff_GetCastOnZ(Buffs_Struct* self)
{
	return self->caston_z;
}

uint32 Perl_Buff_GetCounters(Buffs_Struct* self)
{
	return self->counters;
}

uint32 Perl_Buff_GetDOTRune(Buffs_Struct* self)
{
	return self->dot_rune;
}

int Perl_Buff_GetExtraDIChance(Buffs_Struct* self)
{
	return self->ExtraDIChance;
}

uint32 Perl_Buff_GetInstrumentModifier(Buffs_Struct* self)
{
	return self->instrument_mod;
}

uint32 Perl_Buff_GetMagicRune(Buffs_Struct* self)
{
	return self->magic_rune;
}

uint32 Perl_Buff_GetMeleeRune(Buffs_Struct* self)
{
	return self->melee_rune;
}

uint32 Perl_Buff_GetNumberOfHits(Buffs_Struct* self)
{
	return self->hit_number;
}

int16 Perl_Buff_GetRootBreakChance(Buffs_Struct* self)
{
	return self->RootBreakChance;
}

uint16 Perl_Buff_GetSpellID(Buffs_Struct* self)
{
	return self->spellid;
}

int Perl_Buff_GetTicsRemaining(Buffs_Struct* self)
{
	return self->ticsremaining;
}

int Perl_Buff_GetVirusSpreadTime(Buffs_Struct* self)
{
	return self->virus_spread_time;
}

bool Perl_Buff_IsCasterClient(Buffs_Struct* self)
{
	return self->client;
}

bool Perl_Buff_IsPersistentBuff(Buffs_Struct* self)
{
	return self->persistant_buff;
}

bool Perl_Buff_SendsClientUpdate(Buffs_Struct* self)
{
	return self->UpdateClient;
}

void perl_register_buff()
{
	perl::interpreter state(PERL_GET_THX);

	auto package = state.new_class<Buffs_Struct>("Buff");
	package.add("GetCasterID", &Perl_Buff_GetCasterID);
	package.add("GetCasterLevel", &Perl_Buff_GetCasterLevel);
	package.add("GetCasterName", &Perl_Buff_GetCasterName);
	package.add("GetCastOnX", &Perl_Buff_GetCastOnX);
	package.add("GetCastOnY", &Perl_Buff_GetCastOnY);
	package.add("GetCastOnZ", &Perl_Buff_GetCastOnZ);
	package.add("GetCounters", &Perl_Buff_GetCounters);
	package.add("GetDOTRune", &Perl_Buff_GetDOTRune);
	package.add("GetExtraDIChance", &Perl_Buff_GetExtraDIChance);
	package.add("GetInstrumentModifier", &Perl_Buff_GetInstrumentModifier);
	package.add("GetMagicRune", &Perl_Buff_GetMagicRune);
	package.add("GetMeleeRune", &Perl_Buff_GetMeleeRune);
	package.add("GetNumberOfHits", &Perl_Buff_GetNumberOfHits);
	package.add("GetRootBreakChance", &Perl_Buff_GetRootBreakChance);
	package.add("GetSpellID", &Perl_Buff_GetSpellID);
	package.add("GetTicsRemaining", &Perl_Buff_GetTicsRemaining);
	package.add("GetVirusSpreadTime", &Perl_Buff_GetVirusSpreadTime);
	package.add("IsCasterClient", &Perl_Buff_IsCasterClient);
	package.add("IsPersistentBuff", &Perl_Buff_IsPersistentBuff);
	package.add("SendsClientUpdate", &Perl_Buff_SendsClientUpdate);
}

#endif //EMBPERL_XS_CLASSES
