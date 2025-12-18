#include "common/features.h"

#ifdef EMBPERL_XS_CLASSES

#include "zone/embperl.h"
#include "zone/spawn2.h"

void Perl_Spawn_Depop(Spawn2* self)
{
	self->Depop();
}

void Perl_Spawn_Disable(Spawn2* self)
{
	return self->Disable();
}

void Perl_Spawn_Enable(Spawn2* self)
{
	return self->Enable();
}

void Perl_Spawn_ForceDespawn(Spawn2* self)
{
	self->ForceDespawn();
}

uint32 Perl_Spawn_GetCurrentNPCID(Spawn2* self)
{
	return self->CurrentNPCID();
}

float Perl_Spawn_GetHeading(Spawn2* self)
{
	return self->GetHeading();
}

uint32 Perl_Spawn_GetID(Spawn2* self)
{
	return self->GetID();
}

uint32 Perl_Spawn_GetKillCount(Spawn2* self)
{
	return self->GetKillCount();
}

uint32 Perl_Spawn_GetRespawnTimer(Spawn2* self)
{
	return self->RespawnTimer();
}

uint32 Perl_Spawn_GetSpawnCondition(Spawn2* self)
{
	return self->GetSpawnCondition();
}

uint32 Perl_Spawn_GetSpawnGroupID(Spawn2* self)
{
	return self->SpawnGroupID();
}

uint32 Perl_Spawn_GetVariance(Spawn2* self)
{
	return self->GetVariance();
}

float Perl_Spawn_GetX(Spawn2* self)
{
	return self->GetX();
}

float Perl_Spawn_GetY(Spawn2* self)
{
	return self->GetY();
}

float Perl_Spawn_GetZ(Spawn2* self)
{
	return self->GetZ();
}

bool Perl_Spawn_IsEnabled(Spawn2* self)
{
	return self->Enabled();
}

bool Perl_Spawn_IsNPCPointerValid(Spawn2* self)
{
	return self->NPCPointerValid();
}

void Perl_Spawn_LoadGrid(Spawn2* self)
{
	self->LoadGrid();
}

void Perl_Spawn_Repop(Spawn2* self)
{
	self->Repop();
}

void Perl_Spawn_Repop(Spawn2* self, uint32 delay)
{
	self->Repop(delay);
}

void Perl_Spawn_Reset(Spawn2* self)
{
	self->Reset();
}

void Perl_Spawn_SetCurrentNPCID(Spawn2* self, uint32 npc_id)
{
	self->SetCurrentNPCID(npc_id);
}

void Perl_Spawn_SetNPCPointer(Spawn2* self, NPC* n)
{
	self->SetNPCPointer(n);
}

void Perl_Spawn_SetRespawnTimer(Spawn2* self, uint32 new_respawn_time)
{
	self->SetRespawnTimer(new_respawn_time);
}

void Perl_Spawn_SetTimer(Spawn2* self, uint32 duration)
{
	self->SetTimer(duration);
}

void Perl_Spawn_SetVariance(Spawn2* self, uint32 new_variance)
{
	self->SetVariance(new_variance);
}

void perl_register_spawn()
{
	perl::interpreter perl(PERL_GET_THX);

	auto package = perl.new_class<Spawn2>("Spawn");
	package.add("Depop", (void(*)(Spawn2*))&Perl_Spawn_Depop);
	package.add("Disable", (void(*)(Spawn2*))&Perl_Spawn_Disable);
	package.add("Enable", (void(*)(Spawn2*))&Perl_Spawn_Enable);
	package.add("ForceDespawn", (void(*)(Spawn2*))&Perl_Spawn_ForceDespawn);
	package.add("GetCurrentNPCID", (uint32(*)(Spawn2*))&Perl_Spawn_GetCurrentNPCID);
	package.add("GetHeading", (float(*)(Spawn2*))&Perl_Spawn_GetHeading);
	package.add("GetID", (uint32(*)(Spawn2*))&Perl_Spawn_GetID);
	package.add("GetKillCount", (uint32(*)(Spawn2*))&Perl_Spawn_GetKillCount);
	package.add("GetRespawnTimer", (uint32(*)(Spawn2*))&Perl_Spawn_GetRespawnTimer);
	package.add("GetSpawnCondition", (uint32(*)(Spawn2*))&Perl_Spawn_GetSpawnCondition);
	package.add("GetSpawnGroupID", (uint32(*)(Spawn2*))&Perl_Spawn_GetSpawnGroupID);
	package.add("GetVariance", (uint32(*)(Spawn2*))&Perl_Spawn_GetVariance);
	package.add("GetX", (float(*)(Spawn2*))&Perl_Spawn_GetX);
	package.add("GetY", (float(*)(Spawn2*))&Perl_Spawn_GetY);
	package.add("GetZ", (float(*)(Spawn2*))&Perl_Spawn_GetZ);
	package.add("IsEnabled", (bool(*)(Spawn2*))&Perl_Spawn_IsEnabled);
	package.add("IsNPCPointerValid", (bool(*)(Spawn2*))&Perl_Spawn_IsNPCPointerValid);
	package.add("LoadGrid", (void(*)(Spawn2*))&Perl_Spawn_LoadGrid);
	package.add("Repop", (void(*)(Spawn2*))&Perl_Spawn_Repop);
	package.add("Repop", (void(*)(Spawn2*, uint32))&Perl_Spawn_Repop);
	package.add("Reset", (void(*)(Spawn2*))&Perl_Spawn_Reset);
	package.add("SetCurrentNPCID", (void(*)(Spawn2*, uint32))&Perl_Spawn_SetCurrentNPCID);
	package.add("SetNPCPointer", (void(*)(Spawn2*, NPC*))&Perl_Spawn_SetNPCPointer);
	package.add("SetRespawnTimer", (void(*)(Spawn2*, uint32))&Perl_Spawn_SetRespawnTimer);
	package.add("SetTimer", (void(*)(Spawn2*, uint32))&Perl_Spawn_SetTimer);
	package.add("SetVariance", (void(*)(Spawn2*, uint32))&Perl_Spawn_SetVariance);
}

#endif //EMBPERL_XS_CLASSES
