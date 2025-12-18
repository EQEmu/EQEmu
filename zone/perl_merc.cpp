#include "common/features.h"

#ifdef EMBPERL_XS_CLASSES

#include "zone/embperl.h"
#include "zone/merc.h"

uint32 Perl_Merc_GetCostFormula(Merc* self)
{
	return self->GetCostFormula();
}

Group* Perl_Merc_GetGroup(Merc* self)
{
	return self->GetGroup();
}

int Perl_Merc_GetHatedCount(Merc* self)
{
	return self->GetHatedCount();
}

float Perl_Merc_GetMaxMeleeRangeToTarget(Merc* self, Mob* target)
{
	return self->GetMaxMeleeRangeToTarget(target);
}

uint32 Perl_Merc_GetMercenaryCharacterID(Merc* self)
{
	return self->GetMercenaryCharacterID();
}

uint32 Perl_Merc_GetMercenaryID(Merc* self)
{
	return self->GetMercenaryID();
}

uint32 Perl_Merc_GetMercenaryNameType(Merc* self)
{
	return self->GetMercNameType();
}

Client* Perl_Merc_GetMercenaryOwner(Merc* self)
{
	return self->GetMercenaryOwner();
}

uint32 Perl_Merc_GetMercenarySubtype(Merc* self)
{
	return self->GetMercenarySubType();
}

uint32 Perl_Merc_GetMercenaryTemplateID(Merc* self)
{
	return self->GetMercenaryTemplateID();
}

uint32 Perl_Merc_GetMercenaryType(Merc* self)
{
	return self->GetMercenaryType();
}

Mob* Perl_Merc_GetOwner(Merc* self)
{
	return self->GetOwner();
}

Mob* Perl_Merc_GetOwnerOrSelf(Merc* self)
{
	return self->GetOwnerOrSelf();
}

uint8 Perl_Merc_GetProficiencyID(Merc* self)
{
	return self->GetProficiencyID();
}

uint8 Perl_Merc_GetStance(Merc* self)
{
	return self->GetStance();
}

uint8 Perl_Merc_GetTierID(Merc* self)
{
	return self->GetTierID();
}

bool Perl_Merc_HasOrMayGetAggro(Merc* self)
{
	return self->HasOrMayGetAggro();
}

bool Perl_Merc_IsMercenaryCaster(Merc* self)
{
	return self->IsMercCaster();
}

bool Perl_Merc_IsMercenaryCasterCombatRange(Merc* self, Mob* target)
{
	return self->IsMercCasterCombatRange(target);
}

bool Perl_Merc_IsSitting(Merc* self)
{
	return self->IsSitting();
}

bool Perl_Merc_IsStanding(Merc* self)
{
	return self->IsStanding();
}

void Perl_Merc_ScaleStats(Merc* self, int scale_percentage)
{
	self->ScaleStats(scale_percentage);
}

void Perl_Merc_ScaleStats(Merc* self, int scale_percentage, bool set_to_max)
{
	self->ScaleStats(scale_percentage, set_to_max);
}

void Perl_Merc_SendPayload(Merc* self, int payload_id, std::string payload_value)
{
	self->SendPayload(payload_id, payload_value);
}

void Perl_Merc_SetTarget(Merc* self, Mob* target)
{
	self->SetTarget(target);
}

void Perl_Merc_Signal(Merc* self, int signal_id)
{
	self->Signal(signal_id);
}

void Perl_Merc_Sit(Merc* self)
{
	self->Sit();
}

void Perl_Merc_Stand(Merc* self)
{
	self->Stand();
}

bool Perl_Merc_Suspend(Merc* self)
{
	return self->Suspend();
}

bool Perl_Merc_UseDiscipline(Merc* self, uint16 spell_id, uint16 target_id)
{
	return self->UseDiscipline(spell_id, target_id);
}

void perl_register_merc()
{
	perl::interpreter state(PERL_GET_THX);

	auto package = state.new_class<Merc>("Merc");
	package.add_base_class("NPC");
	package.add("GetCostFormula", &Perl_Merc_GetCostFormula);
	package.add("GetGroup", &Perl_Merc_GetGroup);
	package.add("GetHatedCount", &Perl_Merc_GetHatedCount);
	package.add("GetMaxMeleeRangeToTarget", &Perl_Merc_GetMaxMeleeRangeToTarget);
	package.add("GetMercenaryCharacterID", &Perl_Merc_GetMercenaryCharacterID);
	package.add("GetMercenaryID", &Perl_Merc_GetMercenaryID);
	package.add("GetMercenaryNameType", &Perl_Merc_GetMercenaryNameType);
	package.add("GetMercenaryOwner", &Perl_Merc_GetMercenaryOwner);
	package.add("GetMercenarySubtype", &Perl_Merc_GetMercenarySubtype);
	package.add("GetMercenaryTemplateID", &Perl_Merc_GetMercenaryTemplateID);
	package.add("GetMercenaryType", &Perl_Merc_GetMercenaryType);
	package.add("GetOwner", &Perl_Merc_GetOwner);
	package.add("GetOwnerOrSelf", &Perl_Merc_GetOwnerOrSelf);
	package.add("GetProficiencyID", &Perl_Merc_GetProficiencyID);
	package.add("GetStance", &Perl_Merc_GetStance);
	package.add("GetTierID", &Perl_Merc_GetTierID);
	package.add("HasOrMayGetAggro", &Perl_Merc_HasOrMayGetAggro);
	package.add("IsMercenaryCaster", &Perl_Merc_IsMercenaryCaster);
	package.add("IsMercenaryCasterCombatRange", &Perl_Merc_IsMercenaryCasterCombatRange);
	package.add("IsSitting", &Perl_Merc_IsSitting);
	package.add("IsStanding", &Perl_Merc_IsStanding);
	package.add("ScaleStats", (void(*)(Merc*, int))&Perl_Merc_ScaleStats);
	package.add("ScaleStats", (void(*)(Merc*, int, bool))&Perl_Merc_ScaleStats);
	package.add("SendPayload", &Perl_Merc_SendPayload);
	package.add("SetTarget", &Perl_Merc_SetTarget);
	package.add("Signal", &Perl_Merc_Signal);
	package.add("Sit", &Perl_Merc_Sit);
	package.add("Stand", &Perl_Merc_Stand);
	package.add("Suspend", &Perl_Merc_Suspend);
	package.add("UseDiscipline", &Perl_Merc_UseDiscipline);
}

#endif //EMBPERL_XS_CLASSES
