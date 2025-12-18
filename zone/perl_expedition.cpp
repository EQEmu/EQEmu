#include "common/features.h"

#ifdef EMBPERL_XS_CLASSES

#include "common/zone_store.h"
#include "zone/dynamic_zone.h"
#include "zone/embperl.h"

void Perl_Expedition_AddLockout(DynamicZone* self, std::string event_name, uint32_t seconds)
{
	self->AddLockout(event_name, seconds);
}

void Perl_Expedition_AddLockoutDuration(DynamicZone* self, std::string event_name, int seconds)
{
	self->AddLockoutDuration(event_name, seconds);
}

void Perl_Expedition_AddLockoutDuration(DynamicZone* self, std::string event_name, int seconds, bool members_only)
{
	self->AddLockoutDuration(event_name, seconds, members_only);
}

void Perl_Expedition_AddReplayLockout(DynamicZone* self, uint32_t seconds)
{
	self->AddLockout(DzLockout::ReplayTimer, seconds);
}

void Perl_Expedition_AddReplayLockoutDuration(DynamicZone* self, int seconds)
{
	self->AddLockoutDuration(DzLockout::ReplayTimer, seconds);;
}

void Perl_Expedition_AddReplayLockoutDuration(DynamicZone* self, int seconds, bool members_only)
{
	self->AddLockoutDuration(DzLockout::ReplayTimer, seconds, members_only);
}

uint32_t Perl_Expedition_GetID(DynamicZone* self)
{
	return self->GetID();
}

uint16_t Perl_Expedition_GetInstanceID(DynamicZone* self)
{
	return self->GetInstanceID();
}

std::string Perl_Expedition_GetLeaderName(DynamicZone* self)
{
	return self->GetLeaderName();
}

perl::reference Perl_Expedition_GetLockouts(DynamicZone* self)
{
	perl::hash table;
	const auto& lockouts = self->GetLockouts();
	for (const auto& lockout : lockouts)
	{
		table[lockout.Event()] = lockout.GetSecondsRemaining();
	}
	return perl::reference(table);
}

std::string Perl_Expedition_GetLootEventByNPCTypeID(DynamicZone* self, uint32_t npc_type_id)
{
	return self->GetLootEvent(npc_type_id, DzLootEvent::Type::NpcType);
}

std::string Perl_Expedition_GetLootEventBySpawnID(DynamicZone* self, uint32_t spawn_id)
{
	return self->GetLootEvent(spawn_id, DzLootEvent::Type::Entity);
}

uint32_t Perl_Expedition_GetMemberCount(DynamicZone* self)
{
	return self->GetMemberCount();
}

perl::reference Perl_Expedition_GetMembers(DynamicZone* self)
{
	perl::hash table;
	for (const auto& member : self->GetMembers())
	{
		table[member.name] = member.id;
	}
	return perl::reference(table);
}

std::string Perl_Expedition_GetName(DynamicZone* self)
{
	return self->GetName();
}

uint32_t Perl_Expedition_GetSecondsRemaining(DynamicZone* self)
{
	return self->GetSecondsRemaining();
}

std::string Perl_Expedition_GetUUID(DynamicZone* self)
{
	return self->GetUUID();
}

uint16_t Perl_Expedition_GetZoneID(DynamicZone* self)
{
	return self->GetZoneID();
}

std::string Perl_Expedition_GetZoneName(DynamicZone* self)
{
	return ZoneName(self->GetZoneID());
}

uint32_t Perl_Expedition_GetZoneVersion(DynamicZone* self)
{
	return self->GetZoneVersion();
}

bool Perl_Expedition_HasLockout(DynamicZone* self, std::string event_name)
{
	return self->HasLockout(event_name);
}

bool Perl_Expedition_HasReplayLockout(DynamicZone* self)
{
	return self->HasReplayLockout();
}

bool Perl_Expedition_IsLocked(DynamicZone* self)
{
	return self->IsLocked();
}

void Perl_Expedition_RemoveCompass(DynamicZone* self)
{
	self->SetCompass(0, 0, 0, 0, true);
}

void Perl_Expedition_RemoveLockout(DynamicZone* self, std::string event_name)
{
	self->RemoveLockout(event_name);
}

void Perl_Expedition_SetCompass(DynamicZone* self, perl::scalar zone, float x, float y, float z)
{
	uint32_t zone_id = zone.is_string() ? ZoneID(zone.c_str()) : zone.as<uint32_t>();
	self->SetCompass(zone_id, x, y, z, true);
}

void Perl_Expedition_SetLocked(DynamicZone* self, bool locked)
{
	self->SetLocked(locked, true);
}

void Perl_Expedition_SetLocked(DynamicZone* self, bool locked, int lock_msg)
{
	self->SetLocked(locked, true, static_cast<DzLockMsg>(lock_msg));
}

void Perl_Expedition_SetLocked(DynamicZone* self, bool locked, int lock_msg, uint32_t color)
{
	self->SetLocked(locked, true, static_cast<DzLockMsg>(lock_msg), color);
}

void Perl_Expedition_SetLootEventByNPCTypeID(DynamicZone* self, uint32_t npc_type_id, std::string event_name)
{
	self->SetLootEvent(npc_type_id, event_name, DzLootEvent::Type::NpcType);
}

void Perl_Expedition_SetLootEventBySpawnID(DynamicZone* self, uint32_t entity_id, std::string event_name)
{
	self->SetLootEvent(entity_id, event_name, DzLootEvent::Type::Entity);
}

void Perl_Expedition_SetReplayLockoutOnMemberJoin(DynamicZone* self, bool enable)
{
	self->SetReplayOnJoin(enable, true);
}

void Perl_Expedition_SetSafeReturn(DynamicZone* self, perl::scalar zone, float x, float y, float z, float heading)
{
	uint32_t zone_id = zone.is_string() ? ZoneID(zone.c_str()) : zone.as<uint32_t>();
	self->SetSafeReturn(zone_id, x, y, z, heading, true);
}

void Perl_Expedition_SetSecondsRemaining(DynamicZone* self, uint32_t seconds_remaining)
{
	self->SetSecondsRemaining(seconds_remaining);
}

void Perl_Expedition_SetSwitchID(DynamicZone* self, int dz_switch_id)
{
	self->SetSwitchID(dz_switch_id, true);
}

void Perl_Expedition_SetZoneInLocation(DynamicZone* self, float x, float y, float z, float heading)
{
	self->SetZoneInLocation(x, y, z, heading, true);
}

void Perl_Expedition_UpdateLockoutDuration(DynamicZone* self, std::string event_name, uint32_t seconds)
{
	self->UpdateLockoutDuration(event_name, seconds);
}

void Perl_Expedition_UpdateLockoutDuration(DynamicZone* self, std::string event_name, uint32_t seconds, bool members_only)
{
	self->UpdateLockoutDuration(event_name, seconds, members_only);
}

void perl_register_expedition()
{
	perl::interpreter perl(PERL_GET_THX);

	auto package = perl.new_class<DynamicZone>("Expedition");
	package.add("AddLockout", &Perl_Expedition_AddLockout);
	package.add("AddLockoutDuration", (void(*)(DynamicZone*, std::string, int))&Perl_Expedition_AddLockoutDuration);
	package.add("AddLockoutDuration", (void(*)(DynamicZone*, std::string, int, bool))&Perl_Expedition_AddLockoutDuration);
	package.add("AddReplayLockout", &Perl_Expedition_AddReplayLockout);
	package.add("AddReplayLockoutDuration", (void(*)(DynamicZone*, int))&Perl_Expedition_AddReplayLockoutDuration);
	package.add("AddReplayLockoutDuration", (void(*)(DynamicZone*, int, bool))&Perl_Expedition_AddReplayLockoutDuration);
	package.add("GetDynamicZoneID", &Perl_Expedition_GetID);
	package.add("GetID", &Perl_Expedition_GetID);
	package.add("GetInstanceID", &Perl_Expedition_GetInstanceID);
	package.add("GetLeaderName", &Perl_Expedition_GetLeaderName);
	package.add("GetLockouts", &Perl_Expedition_GetLockouts);
	package.add("GetLootEventByNPCTypeID", &Perl_Expedition_GetLootEventByNPCTypeID);
	package.add("GetLootEventBySpawnID", &Perl_Expedition_GetLootEventBySpawnID);
	package.add("GetMemberCount", &Perl_Expedition_GetMemberCount);
	package.add("GetMembers", &Perl_Expedition_GetMembers);
	package.add("GetName", &Perl_Expedition_GetName);
	package.add("GetSecondsRemaining", &Perl_Expedition_GetSecondsRemaining);
	package.add("GetUUID", &Perl_Expedition_GetUUID);
	package.add("GetZoneID", &Perl_Expedition_GetZoneID);
	package.add("GetZoneName", &Perl_Expedition_GetZoneName);
	package.add("GetZoneVersion", &Perl_Expedition_GetZoneVersion);
	package.add("HasLockout", &Perl_Expedition_HasLockout);
	package.add("HasReplayLockout", &Perl_Expedition_HasReplayLockout);
	package.add("IsLocked", &Perl_Expedition_IsLocked);
	package.add("RemoveCompass", &Perl_Expedition_RemoveCompass);
	package.add("RemoveLockout", &Perl_Expedition_RemoveLockout);
	package.add("SetCompass", &Perl_Expedition_SetCompass);
	package.add("SetLocked", (void(*)(DynamicZone*, bool))&Perl_Expedition_SetLocked);
	package.add("SetLocked", (void(*)(DynamicZone*, bool, int))&Perl_Expedition_SetLocked);
	package.add("SetLocked", (void(*)(DynamicZone*, bool, int, uint32_t))&Perl_Expedition_SetLocked);
	package.add("SetLootEventByNPCTypeID", &Perl_Expedition_SetLootEventByNPCTypeID);
	package.add("SetLootEventBySpawnID", &Perl_Expedition_SetLootEventBySpawnID);
	package.add("SetReplayLockoutOnMemberJoin", &Perl_Expedition_SetReplayLockoutOnMemberJoin);
	package.add("SetSafeReturn", &Perl_Expedition_SetSafeReturn);
	package.add("SetSecondsRemaining", &Perl_Expedition_SetSecondsRemaining);
	package.add("SetSwitchID", &Perl_Expedition_SetSwitchID);
	package.add("SetZoneInLocation", &Perl_Expedition_SetZoneInLocation);
	package.add("UpdateLockoutDuration", (void(*)(DynamicZone*, std::string, uint32_t))&Perl_Expedition_UpdateLockoutDuration);
	package.add("UpdateLockoutDuration", (void(*)(DynamicZone*, std::string, uint32_t, bool))&Perl_Expedition_UpdateLockoutDuration);
}

void perl_register_expedition_lock_messages()
{
	perl::interpreter perl(PERL_GET_THX);

	auto package = perl.new_package("ExpeditionLockMessage");
	package.add_const("None", static_cast<int>(DzLockMsg::None));
	package.add_const("Close", static_cast<int>(DzLockMsg::Close));
	package.add_const("Begin", static_cast<int>(DzLockMsg::Begin));
}

#endif //EMBPERL_XS_CLASSES
