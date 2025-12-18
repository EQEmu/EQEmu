#include "common/features.h"

#ifdef EMBPERL_XS_CLASSES

#include "zone/embperl.h"
#include "zone/zone.h"

bool Perl_Zone_BuffTimersSuspended(Zone* self)
{
	return self->BuffTimersSuspended();
}

bool Perl_Zone_BypassesExpansionCheck(Zone* self)
{
	return ZoneStore::Instance()->GetZoneBypassExpansionCheck(self->GetZoneID(), self->GetInstanceVersion());
}

bool Perl_Zone_CanBind(Zone* self)
{
	return self->CanBind();
}

bool Perl_Zone_CanCastOutdoor(Zone* self)
{
	return self->CanCastOutdoor();
}

bool Perl_Zone_CanDoCombat(Zone* self)
{
	return self->CanDoCombat();
}

bool Perl_Zone_CanLevitate(Zone* self)
{
	return self->CanLevitate();
}

void Perl_Zone_ClearSpawnTimers(Zone* self)
{
	self->ClearSpawnTimers();
}

void Perl_Zone_Depop(Zone* self)
{
	self->Depop();
}

void Perl_Zone_Depop(Zone* self, bool start_spawn_timers)
{
	self->Depop(start_spawn_timers);
}

void Perl_Zone_Despawn(Zone* self, uint32 spawngroup_id)
{
	self->Despawn(spawngroup_id);
}

void Perl_Zone_DisableRespawnTimers(Zone* self)
{
	self->DisableRespawnTimers();
}

float Perl_Zone_GetAAEXPModifier(Zone* self, Client* c)
{
	return self->GetAAEXPModifier(c);
}

float Perl_Zone_GetAAEXPModifierByCharacterID(Zone* self, uint32 character_id)
{
	return self->GetAAEXPModifierByCharacterID(character_id);
}

std::string Perl_Zone_GetContentFlags(Zone* self)
{
	return ZoneStore::Instance()->GetZoneContentFlags(self->GetZoneID(), self->GetInstanceVersion());
}

std::string Perl_Zone_GetContentFlagsDisabled(Zone* self)
{
	return ZoneStore::Instance()->GetZoneContentFlagsDisabled(self->GetZoneID(), self->GetInstanceVersion());
}

float Perl_Zone_GetExperienceMultiplier(Zone* self)
{
	return ZoneStore::Instance()->GetZoneExperienceMultiplier(self->GetZoneID(), self->GetInstanceVersion());
}

int8 Perl_Zone_GetExpansion(Zone* self)
{
	return ZoneStore::Instance()->GetZoneExpansion(self->GetZoneID(), self->GetInstanceVersion());
}

float Perl_Zone_GetEXPModifier(Zone* self, Client* c)
{
	return self->GetEXPModifier(c);
}

float Perl_Zone_GetEXPModifierByCharacterID(Zone* self, uint32 character_id)
{
	return self->GetEXPModifierByCharacterID(character_id);
}

int Perl_Zone_GetFastRegenEndurance(Zone* self)
{
	return ZoneStore::Instance()->GetZoneFastRegenEndurance(self->GetZoneID(), self->GetInstanceVersion());
}

int Perl_Zone_GetFastRegenHP(Zone* self)
{
	return ZoneStore::Instance()->GetZoneFastRegenHP(self->GetZoneID(), self->GetInstanceVersion());
}

int Perl_Zone_GetFastRegenMana(Zone* self)
{
	return ZoneStore::Instance()->GetZoneFastRegenMana(self->GetZoneID(), self->GetInstanceVersion());
}

std::string Perl_Zone_GetFileName(Zone* self)
{
	return self->GetFileName();
}

std::string Perl_Zone_GetFlagNeeded(Zone* self)
{
	return ZoneStore::Instance()->GetZoneFlagNeeded(self->GetZoneID(), self->GetInstanceVersion());
}

uint8 Perl_Zone_GetFogBlue(Zone* self, uint8 slot = 0)
{
	return ZoneStore::Instance()->GetZoneFogBlue(self->GetZoneID(), slot, self->GetInstanceVersion());
}

float Perl_Zone_GetFogDensity(Zone* self)
{
	return ZoneStore::Instance()->GetZoneFogDensity(self->GetZoneID(), self->GetInstanceVersion());
}

uint8 Perl_Zone_GetFogGreen(Zone* self, uint8 slot = 0)
{
	return ZoneStore::Instance()->GetZoneFogGreen(self->GetZoneID(), slot, self->GetInstanceVersion());
}

float Perl_Zone_GetFogMaximumClip(Zone* self, uint8 slot = 0)
{
	return ZoneStore::Instance()->GetZoneFogMaximumClip(self->GetZoneID(), slot, self->GetInstanceVersion());
}

float Perl_Zone_GetFogMinimumClip(Zone* self, uint8 slot = 0)
{
	return ZoneStore::Instance()->GetZoneFogMinimumClip(self->GetZoneID(), slot, self->GetInstanceVersion());
}

uint8 Perl_Zone_GetFogRed(Zone* self, uint8 slot = 0)
{
	return ZoneStore::Instance()->GetZoneFogRed(self->GetZoneID(), slot, self->GetInstanceVersion());
}

float Perl_Zone_GetGraveyardHeading(Zone* self)
{
	return self->GetGraveyardPoint().w;
}

uint32 Perl_Zone_GetGraveyardID(Zone* self)
{
	return self->graveyard_id();
}

float Perl_Zone_GetGraveyardX(Zone* self)
{
	return self->GetGraveyardPoint().x;
}

float Perl_Zone_GetGraveyardY(Zone* self)
{
	return self->GetGraveyardPoint().y;
}

float Perl_Zone_GetGraveyardZ(Zone* self)
{
	return self->GetGraveyardPoint().z;
}

uint32 Perl_Zone_GetGraveyardZoneID(Zone* self)
{
	return self->graveyard_zoneid();
}

float Perl_Zone_GetGravity(Zone* self)
{
	return ZoneStore::Instance()->GetZoneGravity(self->GetZoneID(), self->GetInstanceVersion());
}

uint32 Perl_Zone_GetInstanceID(Zone* self)
{
	return self->GetInstanceID();
}

uint8 Perl_Zone_GetInstanceType(Zone* self)
{
	return ZoneStore::Instance()->GetZoneInstanceType(self->GetZoneID(), self->GetInstanceVersion());
}

uint16 Perl_Zone_GetInstanceVersion(Zone* self)
{
	return self->GetInstanceVersion();
}

uint32 Perl_Zone_GetInstanceTimeRemaining(Zone* self)
{
	return self->GetInstanceTimeRemaining();
}

int Perl_Zone_GetLavaDamage(Zone* self)
{
	return ZoneStore::Instance()->GetZoneLavaDamage(self->GetZoneID(), self->GetInstanceVersion());
}

std::string Perl_Zone_GetLongName(Zone* self)
{
	return self->GetLongName();
}

float Perl_Zone_GetMaximumClip(Zone* self)
{
	return ZoneStore::Instance()->GetZoneMaximumClip(self->GetZoneID(), self->GetInstanceVersion());
}

int8 Perl_Zone_GetMaximumExpansion(Zone* self)
{
	return ZoneStore::Instance()->GetZoneMaximumExpansion(self->GetZoneID(), self->GetInstanceVersion());
}

uint8 Perl_Zone_GetMaximumLevel(Zone* self)
{
	return ZoneStore::Instance()->GetZoneMaximumLevel(self->GetZoneID(), self->GetInstanceVersion());
}

uint32 Perl_Zone_GetMaxClients(Zone* self)
{
	return self->GetMaxClients();
}

float Perl_Zone_GetMinimumClip(Zone* self)
{
	return ZoneStore::Instance()->GetZoneMinimumClip(self->GetZoneID(), self->GetInstanceVersion());
}

int8 Perl_Zone_GetMinimumExpansion(Zone* self)
{
	return ZoneStore::Instance()->GetZoneMinimumExpansion(self->GetZoneID(), self->GetInstanceVersion());
}

uint8 Perl_Zone_GetMinimumLevel(Zone* self)
{
	return ZoneStore::Instance()->GetZoneMinimumLevel(self->GetZoneID(), self->GetInstanceVersion());
}

int Perl_Zone_GetMinimumLavaDamage(Zone* self)
{
	return ZoneStore::Instance()->GetZoneMinimumLavaDamage(self->GetZoneID(), self->GetInstanceVersion());
}

uint8 Perl_Zone_GetMinimumStatus(Zone* self)
{
	return ZoneStore::Instance()->GetZoneMinimumStatus(self->GetZoneID(), self->GetInstanceVersion());
}

std::string Perl_Zone_GetNote(Zone* self)
{
	return ZoneStore::Instance()->GetZoneNote(self->GetZoneID(), self->GetInstanceVersion());
}

int Perl_Zone_GetNPCMaximumAggroDistance(Zone* self)
{
	return ZoneStore::Instance()->GetZoneNPCMaximumAggroDistance(self->GetZoneID(), self->GetInstanceVersion());
}

int8 Perl_Zone_GetPEQZone(Zone* self)
{
	return ZoneStore::Instance()->GetZonePEQZone(self->GetZoneID(), self->GetInstanceVersion());
}

int Perl_Zone_GetRainChance(Zone* self, uint8 slot = 0)
{
	return ZoneStore::Instance()->GetZoneRainChance(self->GetZoneID(), slot, self->GetInstanceVersion());
}

int Perl_Zone_GetRainDuration(Zone* self, uint8 slot = 0)
{
	return ZoneStore::Instance()->GetZoneRainDuration(self->GetZoneID(), slot, self->GetInstanceVersion());
}

uint32 Perl_Zone_GetRuleSet(Zone* self)
{
	return ZoneStore::Instance()->GetZoneRuleSet(self->GetZoneID(), self->GetInstanceVersion());
}

float Perl_Zone_GetSafeHeading(Zone* self)
{
	return self->GetSafePoint().w;
}

float Perl_Zone_GetSafeX(Zone* self)
{
	return self->GetSafePoint().x;
}

float Perl_Zone_GetSafeY(Zone* self)
{
	return self->GetSafePoint().y;
}

float Perl_Zone_GetSafeZ(Zone* self)
{
	return self->GetSafePoint().z;
}

std::string Perl_Zone_GetShortName(Zone* self)
{
	return self->GetShortName();
}

uint32 Perl_Zone_GetSecondsBeforeIdle(Zone* self)
{
	return self->GetSecondsBeforeIdle();
}

uint64 Perl_Zone_GetShutdownDelay(Zone* self)
{
	return ZoneStore::Instance()->GetZoneShutdownDelay(self->GetZoneID(), self->GetInstanceVersion());
}

uint8 Perl_Zone_GetSky(Zone* self)
{
	return ZoneStore::Instance()->GetZoneSky(self->GetZoneID(), self->GetInstanceVersion());
}

int8 Perl_Zone_GetSkyLock(Zone* self)
{
	return ZoneStore::Instance()->GetZoneSkyLock(self->GetZoneID(), self->GetInstanceVersion());
}

int Perl_Zone_GetSnowChance(Zone* self, uint8 slot = 0)
{
	return ZoneStore::Instance()->GetZoneSnowChance(self->GetZoneID(), slot, self->GetInstanceVersion());
}

int Perl_Zone_GetSnowDuration(Zone* self, uint8 slot = 0)
{
	return ZoneStore::Instance()->GetZoneSnowDuration(self->GetZoneID(), slot, self->GetInstanceVersion());
}

uint8 Perl_Zone_GetTimeType(Zone* self)
{
	return ZoneStore::Instance()->GetZoneTimeType(self->GetZoneID(), self->GetInstanceVersion());
}

int Perl_Zone_GetTimeZone(Zone* self)
{
	return ZoneStore::Instance()->GetZoneTimeZone(self->GetZoneID(), self->GetInstanceVersion());
}

std::string Perl_Zone_GetZoneDescription(Zone* self)
{
	return self->GetZoneDescription();
}

uint32 Perl_Zone_GetZoneID(Zone* self)
{
	return self->GetZoneID();
}

uint8 Perl_Zone_GetZoneType(Zone* self)
{
	return self->GetZoneType();
}

float Perl_Zone_GetUnderworld(Zone* self)
{
	return ZoneStore::Instance()->GetZoneUnderworld(self->GetZoneID(), self->GetInstanceVersion());
}

int Perl_Zone_GetUnderworldTeleportIndex(Zone* self)
{
	return ZoneStore::Instance()->GetZoneUnderworldTeleportIndex(self->GetZoneID(), self->GetInstanceVersion());
}

float Perl_Zone_GetWalkSpeed(Zone* self)
{
	return ZoneStore::Instance()->GetZoneWalkSpeed(self->GetZoneID(), self->GetInstanceVersion());
}

uint8 Perl_Zone_GetZoneZType(Zone* self)
{
	return ZoneStore::Instance()->GetZoneZType(self->GetZoneID(), self->GetInstanceVersion());
}

int Perl_Zone_GetZoneTotalBlockedSpells(Zone* self)
{
	return self->GetZoneTotalBlockedSpells();
}

bool Perl_Zone_HasGraveyard(Zone* self)
{
	return self->HasGraveyard();
}

bool Perl_Zone_HasMap(Zone* self)
{
	return self->HasMap();
}

bool Perl_Zone_HasWaterMap(Zone* self)
{
	return self->HasWaterMap();
}

bool Perl_Zone_HasWeather(Zone* self)
{
	return self->HasWeather();
}

bool Perl_Zone_IsCity(Zone* self)
{
	return self->IsCity();
}

bool Perl_Zone_IsHotzone(Zone* self)
{
	return self->IsHotzone();
}

bool Perl_Zone_IsInstancePersistent(Zone* self)
{
	return self->IsInstancePersistent();
}

bool Perl_Zone_IsIdleWhenEmpty(Zone* self)
{
	return self->IsIdleWhenEmpty();
}

bool Perl_Zone_IsPVPZone(Zone* self)
{
	return self->IsPVPZone();
}

bool Perl_Zone_IsRaining(Zone* self)
{
	return self->IsRaining();
}

bool Perl_Zone_IsSnowing(Zone* self)
{
	return self->IsSnowing();
}

bool Perl_Zone_IsSpecialBindLocation(Zone* self, float x, float y, float z, float heading)
{
	return self->IsSpecialBindLocation(glm::vec4(x, y, z, heading));
}

bool Perl_Zone_IsSpellBlocked(Zone* self, uint32 spell_id, float x, float y, float z)
{
	return self->IsSpellBlocked(spell_id, glm::vec3(x, y, z));
}

bool Perl_Zone_IsStaticZone(Zone* self)
{
	return self->IsStaticZone();
}

bool Perl_Zone_IsUCSServerAvailable(Zone* self)
{
	return self->IsUCSServerAvailable();
}

bool Perl_Zone_IsWaterZone(Zone* self, float z)
{
	return self->IsWaterZone(z);
}

void Perl_Zone_Repop(Zone* self)
{
	self->Repop();
}

void Perl_Zone_Repop(Zone* self, bool is_forced)
{
	self->Repop(is_forced);
}

void Perl_Zone_SetAAEXPModifier(Zone* self, Client* c, float aa_modifier)
{
	self->SetAAEXPModifier(c, aa_modifier);
}

void Perl_Zone_SetAAEXPModifierByCharacterID(Zone* self, uint32 character_id, float aa_modifier)
{
	self->SetAAEXPModifierByCharacterID(character_id, aa_modifier);
}

void Perl_Zone_SetEXPModifier(Zone* self, Client* c, float exp_modifier)
{
	self->SetAAEXPModifier(c, exp_modifier);
}

void Perl_Zone_SetEXPModifierByCharacterID(Zone* self, uint32 character_id, float exp_modifier)
{
	self->SetAAEXPModifierByCharacterID(character_id, exp_modifier);
}

void Perl_Zone_SetInstanceTimer(Zone* self, uint32 new_duration)
{
	self->SetInstanceTimer(new_duration);
}

void Perl_Zone_SetInstanceTimeRemaining(Zone* self, uint32 time_remaining)
{
	self->SetInstanceTimeRemaining(time_remaining);
}

void Perl_Zone_SetIsHotzone(Zone* self, bool is_hotzone)
{
	self->SetIsHotzone(is_hotzone);
}

void Perl_Zone_ShowZoneGlobalLoot(Zone* self, Client* c)
{
	self->ShowZoneGlobalLoot(c);
}

void Perl_Zone_SetBucket(Zone* self, const std::string bucket_name, const std::string bucket_value)
{
	self->SetBucket(bucket_name, bucket_value);
}

void Perl_Zone_SetBucket(Zone* self, const std::string bucket_name, const std::string bucket_value, const std::string expiration)
{
	self->SetBucket(bucket_name, bucket_value, expiration);
}

void Perl_Zone_DeleteBucket(Zone* self, const std::string bucket_name)
{
	self->DeleteBucket(bucket_name);
}

std::string Perl_Zone_GetBucket(Zone* self, const std::string bucket_name)
{
	return self->GetBucket(bucket_name);
}

std::string Perl_Zone_GetBucketExpires(Zone* self, const std::string bucket_name)
{
	return self->GetBucketExpires(bucket_name);
}

std::string Perl_Zone_GetBucketRemaining(Zone* self, const std::string bucket_name)
{
	return self->GetBucketRemaining(bucket_name);
}

bool Perl_Zone_ClearVariables(Zone* self)
{
	return self->ClearVariables();
}

bool Perl_Zone_DeleteVariable(Zone* self, const std::string variable_name)
{
	return self->DeleteVariable(variable_name);
}

std::string Perl_Zone_GetVariable(Zone* self, const std::string variable_name)
{
	return self->GetVariable(variable_name);
}

perl::array Perl_Zone_GetVariables(Zone* self)
{
	perl::array a;

	const auto& l = self->GetVariables();
	for (const auto& v : l) {
		a.push_back(v);
	}

	return a;
}

void Perl_Zone_SetVariable(Zone* self, const std::string variable_name, const std::string variable_value)
{
	self->SetVariable(variable_name, variable_value);
}

bool Perl_Zone_VariableExists(Zone* self, const std::string variable_name)
{
	return self->VariableExists(variable_name);
}

uint32 Perl_Zone_GetTimerDuration(Zone* self, std::string name)
{
	return self->GetTimerDuration(name);
}

uint32 Perl_Zone_GetTimerRemainingTime(Zone* self, std::string name)
{
	return self->GetTimerRemainingTime(name);
}

bool Perl_Zone_HasTimer(Zone* self, std::string name)
{
	return self->HasTimer(name);
}

bool Perl_Zone_IsPausedTimer(Zone* self, std::string name)
{
	return self->IsPausedTimer(name);
}

void Perl_Zone_PauseTimer(Zone* self, std::string name)
{
	self->PauseTimer(name);
}

void Perl_Zone_ResumeTimer(Zone* self, std::string name)
{
	self->ResumeTimer(name);
}

void Perl_Zone_SetTimer(Zone* self, std::string name, uint32 duration)
{
	self->SetTimer(name, duration);
}

void Perl_Zone_StopTimer(Zone* self, std::string name)
{
	self->StopTimer(name);
}

void Perl_Zone_StopAllTimers(Zone* self)
{
	self->StopAllTimers();
}

void Perl_Zone_SendPayload(Zone* self, int payload_id, std::string payload_value)
{
	self->SendPayload(payload_id, payload_value);
}

void Perl_Zone_Signal(Zone* self, int signal_id)
{
	self->Signal(signal_id);
}

perl::array Perl_Zone_GetPausedTimers(Zone* self)
{
	perl::array a;

	const auto& l = self->GetPausedTimers();

	if (!l.empty()) {
		a.reserve(l.size());

		for (const auto& v : l) {
			a.push_back(v);
		}
	}

	return a;
}

perl::array Perl_Zone_GetTimers(Zone* self)
{
	perl::array a;

	const auto& l = self->GetTimers();

	if (!l.empty()) {
		a.reserve(l.size());

		for (const auto& v : l) {
			a.push_back(v);
		}
	}

	return a;
}

void perl_register_zone()
{
	perl::interpreter perl(PERL_GET_THX);

	auto package = perl.new_class<Zone>("Zone");
	package.add("BuffTimersSuspended", &Perl_Zone_BuffTimersSuspended);
	package.add("BypassesExpansionCheck", &Perl_Zone_BypassesExpansionCheck);
	package.add("CanBind", &Perl_Zone_CanBind);
	package.add("CanCastOutdoor", &Perl_Zone_CanCastOutdoor);
	package.add("CanDoCombat", &Perl_Zone_CanDoCombat);
	package.add("CanLevitate", &Perl_Zone_CanLevitate);
	package.add("ClearSpawnTimers", &Perl_Zone_ClearSpawnTimers);
	package.add("ClearVariables", &Perl_Zone_ClearVariables);
	package.add("DeleteBucket", &Perl_Zone_DeleteBucket);
	package.add("DeleteVariable", &Perl_Zone_DeleteVariable);
	package.add("Depop", (void(*)(Zone*))&Perl_Zone_Depop);
	package.add("Depop", (void(*)(Zone*, bool))&Perl_Zone_Depop);
	package.add("Despawn", &Perl_Zone_Despawn);
	package.add("DisableRespawnTimers", &Perl_Zone_DisableRespawnTimers);
	package.add("GetAAEXPModifier", &Perl_Zone_GetAAEXPModifier);
	package.add("GetAAEXPModifierByCharacterID", &Perl_Zone_GetAAEXPModifierByCharacterID);
	package.add("GetBucket", &Perl_Zone_GetBucket);
	package.add("GetBucketExpires", &Perl_Zone_GetBucketExpires);
	package.add("GetBucketRemaining", &Perl_Zone_GetBucketRemaining);
	package.add("GetContentFlags", &Perl_Zone_GetContentFlags);
	package.add("GetContentFlagsDisabled", &Perl_Zone_GetContentFlagsDisabled);
	package.add("GetExperienceMultiplier", &Perl_Zone_GetExperienceMultiplier);
	package.add("GetExpansion", &Perl_Zone_GetExpansion);
	package.add("GetEXPModifier", &Perl_Zone_GetEXPModifier);
	package.add("GetEXPModifierByCharacterID", &Perl_Zone_GetEXPModifierByCharacterID);
	package.add("GetFastRegenEndurance", &Perl_Zone_GetFastRegenEndurance);
	package.add("GetFastRegenHP", &Perl_Zone_GetFastRegenHP);
	package.add("GetFastRegenMana", &Perl_Zone_GetFastRegenMana);
	package.add("GetFileName", &Perl_Zone_GetFileName);
	package.add("GetFlagNeeded", &Perl_Zone_GetFlagNeeded);
	package.add("GetFogBlue", (uint8(*)(Zone*))&Perl_Zone_GetFogBlue);
	package.add("GetFogBlue", (uint8(*)(Zone*, uint8))&Perl_Zone_GetFogBlue);
	package.add("GetFogDensity", &Perl_Zone_GetFogDensity);
	package.add("GetFogGreen", (uint8(*)(Zone*))&Perl_Zone_GetFogGreen);
	package.add("GetFogGreen", (uint8(*)(Zone*, uint8))&Perl_Zone_GetFogGreen);
	package.add("GetFogMaximumClip", (float(*)(Zone*))&Perl_Zone_GetFogMaximumClip);
	package.add("GetFogMaximumClip", (float(*)(Zone*, uint8))&Perl_Zone_GetFogMaximumClip);
	package.add("GetFogMinimumClip", (float(*)(Zone*))&Perl_Zone_GetFogMinimumClip);
	package.add("GetFogMinimumClip", (float(*)(Zone*, uint8))&Perl_Zone_GetFogMinimumClip);
	package.add("GetFogRed", (uint8(*)(Zone*))&Perl_Zone_GetFogRed);
	package.add("GetFogRed", (uint8(*)(Zone*, uint8))&Perl_Zone_GetFogRed);
	package.add("GetGraveyardHeading", &Perl_Zone_GetGraveyardHeading);
	package.add("GetGraveyardID", &Perl_Zone_GetGraveyardID);
	package.add("GetGraveyardX", &Perl_Zone_GetGraveyardX);
	package.add("GetGraveyardY", &Perl_Zone_GetGraveyardY);
	package.add("GetGraveyardZ", &Perl_Zone_GetGraveyardZ);
	package.add("GetGraveyardZoneID", &Perl_Zone_GetGraveyardZoneID);
	package.add("GetGravity", &Perl_Zone_GetGravity);
	package.add("GetInstanceID", &Perl_Zone_GetInstanceID);
	package.add("GetInstanceType", &Perl_Zone_GetInstanceType);
	package.add("GetInstanceVersion", &Perl_Zone_GetInstanceVersion);
	package.add("GetInstanceTimeRemaining", &Perl_Zone_GetInstanceTimeRemaining);
	package.add("GetLavaDamage", &Perl_Zone_GetLavaDamage);
	package.add("GetLongName", &Perl_Zone_GetLongName);
	package.add("GetMaximumClip", &Perl_Zone_GetMaximumClip);
	package.add("GetMaximumExpansion", &Perl_Zone_GetMaximumExpansion);
	package.add("GetMaximumLevel", &Perl_Zone_GetMaximumLevel);
	package.add("GetMaxClients", &Perl_Zone_GetMaxClients);
	package.add("GetMinimumClip", &Perl_Zone_GetMinimumClip);
	package.add("GetMinimumExpansion", &Perl_Zone_GetMinimumExpansion);
	package.add("GetMinimumLevel", &Perl_Zone_GetMinimumLevel);
	package.add("GetMinimumLavaDamage", &Perl_Zone_GetMinimumLavaDamage);
	package.add("GetMinimumStatus", &Perl_Zone_GetMinimumStatus);
	package.add("GetNote", &Perl_Zone_GetNote);
	package.add("GetNPCMaximumAggroDistance", &Perl_Zone_GetNPCMaximumAggroDistance);
	package.add("GetPausedTimers", &Perl_Zone_GetPausedTimers);
	package.add("GetPEQZone", &Perl_Zone_GetPEQZone);
	package.add("GetRainChance", (int(*)(Zone*))&Perl_Zone_GetRainChance);
	package.add("GetRainChance", (int(*)(Zone*, uint8))&Perl_Zone_GetRainChance);
	package.add("GetRainDuration", (int(*)(Zone*))&Perl_Zone_GetRainDuration);
	package.add("GetRainDuration", (int(*)(Zone*, uint8))&Perl_Zone_GetRainDuration);
	package.add("GetRuleSet", &Perl_Zone_GetRuleSet);
	package.add("GetSafeHeading", &Perl_Zone_GetSafeHeading);
	package.add("GetSafeX", &Perl_Zone_GetSafeX);
	package.add("GetSafeY", &Perl_Zone_GetSafeY);
	package.add("GetSafeZ", &Perl_Zone_GetSafeZ);
	package.add("GetShortName", &Perl_Zone_GetShortName);
	package.add("GetSecondsBeforeIdle", &Perl_Zone_GetSecondsBeforeIdle);
	package.add("GetShutdownDelay", &Perl_Zone_GetShutdownDelay);
	package.add("GetSky", &Perl_Zone_GetSky);
	package.add("GetSkyLock", &Perl_Zone_GetSkyLock);
	package.add("GetSnowChance", (int(*)(Zone*))&Perl_Zone_GetSnowChance);
	package.add("GetSnowChance", (int(*)(Zone*, uint8))&Perl_Zone_GetSnowChance);
	package.add("GetSnowDuration", (int(*)(Zone*))&Perl_Zone_GetSnowDuration);
	package.add("GetSnowDuration", (int(*)(Zone*, uint8))&Perl_Zone_GetSnowDuration);
	package.add("GetTimeType", &Perl_Zone_GetTimeType);
	package.add("GetTimeZone", &Perl_Zone_GetTimeZone);
	package.add("GetTimerDuration", &Perl_Zone_GetTimerDuration);
	package.add("GetTimerRemainingTime", &Perl_Zone_GetTimerRemainingTime);
	package.add("GetTimers", &Perl_Zone_GetTimers);
	package.add("GetZoneDescription", &Perl_Zone_GetZoneDescription);
	package.add("GetZoneID", &Perl_Zone_GetZoneID);
	package.add("GetZoneType", &Perl_Zone_GetZoneType);
	package.add("GetUnderworld", &Perl_Zone_GetUnderworld);
	package.add("GetUnderworldTeleportIndex", &Perl_Zone_GetUnderworldTeleportIndex);
	package.add("GetVariable", &Perl_Zone_GetVariable);
	package.add("GetVariables", &Perl_Zone_GetVariables);
	package.add("GetWalkSpeed", &Perl_Zone_GetWalkSpeed);
	package.add("GetZoneZType", &Perl_Zone_GetZoneZType);
	package.add("GetZoneTotalBlockedSpells", &Perl_Zone_GetZoneTotalBlockedSpells);
	package.add("HasGraveyard", &Perl_Zone_HasGraveyard);
	package.add("HasMap", &Perl_Zone_HasMap);
	package.add("HasTimer", &Perl_Zone_HasTimer);
	package.add("HasWaterMap", &Perl_Zone_HasWaterMap);
	package.add("HasWeather", &Perl_Zone_HasWeather);
	package.add("IsCity", &Perl_Zone_IsCity);
	package.add("IsHotzone", &Perl_Zone_IsHotzone);
	package.add("IsInstancePersistent", &Perl_Zone_IsInstancePersistent);
	package.add("IsIdleWhenEmpty", &Perl_Zone_IsIdleWhenEmpty);
	package.add("IsPausedTimer", &Perl_Zone_IsPausedTimer);
	package.add("IsPVPZone", &Perl_Zone_IsPVPZone);
	package.add("IsRaining", &Perl_Zone_IsRaining);
	package.add("IsSnowing", &Perl_Zone_IsSnowing);
	package.add("IsSpecialBindLocation", &Perl_Zone_IsSpecialBindLocation);
	package.add("IsSpellBlocked", &Perl_Zone_IsSpellBlocked);
	package.add("IsStaticZone", &Perl_Zone_IsStaticZone);
	package.add("IsUCSServerAvailable", &Perl_Zone_IsUCSServerAvailable);
	package.add("IsWaterZone", &Perl_Zone_IsWaterZone);
	package.add("PauseTimer", &Perl_Zone_PauseTimer);
	package.add("Repop", (void(*)(Zone*))&Perl_Zone_Repop);
	package.add("Repop", (void(*)(Zone*, bool))&Perl_Zone_Repop);
	package.add("ResumeTimer", &Perl_Zone_ResumeTimer);
	package.add("SendPayload", &Perl_Zone_SendPayload);
	package.add("SetAAEXPModifier", &Perl_Zone_SetAAEXPModifier);
	package.add("SetAAEXPModifierByCharacterID", &Perl_Zone_SetAAEXPModifierByCharacterID);
	package.add("SetBucket", (void(*)(Zone*, const std::string, const std::string))&Perl_Zone_SetBucket);
	package.add("SetBucket", (void(*)(Zone*, const std::string, const std::string, const std::string))&Perl_Zone_SetBucket);
	package.add("SetEXPModifier", &Perl_Zone_SetEXPModifier);
	package.add("SetEXPModifierByCharacterID", &Perl_Zone_SetEXPModifierByCharacterID);
	package.add("SetInstanceTimer", &Perl_Zone_SetInstanceTimer);
	package.add("SetInstanceTimeRemaining", &Perl_Zone_SetInstanceTimeRemaining);
	package.add("SetIsHotzone", &Perl_Zone_SetIsHotzone);
	package.add("SetTimer", &Perl_Zone_SetTimer);
	package.add("SetVariable", &Perl_Zone_SetVariable);
	package.add("Signal", &Perl_Zone_Signal);
	package.add("StopTimer", &Perl_Zone_StopTimer);
	package.add("StopAllTimers", &Perl_Zone_StopAllTimers);
	package.add("ShowZoneGlobalLoot", &Perl_Zone_ShowZoneGlobalLoot);
	package.add("VariableExists", &Perl_Zone_VariableExists);
}

#endif //EMBPERL_XS_CLASSES

