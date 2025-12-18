#include "common/features.h"

#ifdef EMBPERL_XS_CLASSES

#include "zone/doors.h"
#include "zone/embperl.h"

uint32_t Perl_Doors_GetDoorDBID(Doors* self) // @categories Doors
{
	return self->GetDoorDBID();
}

uint32_t Perl_Doors_GetDoorID(Doors* self) // @categories Doors
{
	return self->GetDoorID();
}

uint32_t Perl_Doors_GetID(Doors* self) // @categories Doors
{
	return self->GetEntityID();
}

float Perl_Doors_GetX(Doors* self) // @categories Doors
{
	return self->GetPosition().x;
}

float Perl_Doors_GetY(Doors* self) // @categories Doors
{
	return self->GetPosition().y;
}

float Perl_Doors_GetZ(Doors* self) // @categories Doors
{
	return self->GetPosition().z;
}

float Perl_Doors_GetHeading(Doors* self) // @categories Doors
{
	return self->GetPosition().w;
}

uint32_t Perl_Doors_GetOpenType(Doors* self)  // @categories Doors
{
	return self->GetOpenType();
}

uint32_t Perl_Doors_GetLockPick(Doors* self) // @categories Doors, Skills and Recipes
{
	return self->GetLockpick();
}

uint32_t Perl_Doors_GetKeyItem(Doors* self) // @categories Doors
{
	return self->GetKeyItem();
}

uint8_t Perl_Doors_GetNoKeyring(Doors* self) // @categories Doors
{
	return self->GetNoKeyring();
}

int Perl_Doors_GetIncline(Doors* self) // @categories Doors
{
	return self->GetIncline();
}

uint32_t Perl_Doors_GetSize(Doors* self) // @categories Doors
{
	return self->GetSize();
}

void Perl_Doors_SetOpenType(Doors* self, uint32_t open_type) // @categories Doors
{
	self->SetOpenType(open_type);
}

void Perl_Doors_SetLockPick(Doors* self, uint32_t lockpick_type) // @categories Doors
{
	self->SetLockpick(lockpick_type);
}

void Perl_Doors_SetKeyItem(Doors* self, uint32_t key_item_id) // @categories Doors
{
	self->SetKeyItem(key_item_id);
}

void Perl_Doors_SetNoKeyring(Doors* self, uint8_t no_key_ring) // @categories Doors
{
	self->SetNoKeyring(no_key_ring);
}

void Perl_Doors_SetIncline(Doors* self, uint32_t incline) // @categories Doors
{
	self->SetIncline(incline);
}

void Perl_Doors_SetSize(Doors* self, uint32_t size) // @categories Doors
{
	self->SetSize(size);
}

void Perl_Doors_SetLocation(Doors* self, float x, float y, float z) // @categories Doors
{
	self->SetLocation(x, y, z);
}

void Perl_Doors_SetX(Doors* self, float x) // @categories Doors
{
	auto position = self->GetPosition();
	position.x = x;
	self->SetPosition(position);
}

void Perl_Doors_SetY(Doors* self, float y) // @categories Doors
{
	auto position = self->GetPosition();
	position.y = y;
	self->SetPosition(position);
}

void Perl_Doors_SetZ(Doors* self, float z) // @categories Doors
{
	auto position = self->GetPosition();
	position.z = z;
	self->SetPosition(position);
}

void Perl_Doors_SetHeading(Doors* self, float heading) // @categories Doors
{
	auto position = self->GetPosition();
	position.w = heading;
	self->SetPosition(position);
}

void Perl_Doors_SetModelName(Doors* self, const char* name) // @categories Doors
{
	self->SetDoorName(name);
}

std::string Perl_Doors_GetModelName(Doors* self) // @categories Doors
{
	return self->GetDoorName();
}

void Perl_Doors_CreateDatabaseEntry(Doors* self) // @categories Doors
{
	self->CreateDatabaseEntry();
}

void Perl_Doors_ForceClose(Doors* self, Mob* sender)
{
	self->ForceClose(sender);
}

void Perl_Doors_ForceClose(Doors* self, Mob* sender, bool alt_mode)
{
	self->ForceClose(sender, alt_mode);
}

void Perl_Doors_ForceOpen(Doors* self, Mob* sender)
{
	self->ForceOpen(sender);
}

void Perl_Doors_ForceOpen(Doors* self, Mob* sender, bool alt_mode)
{
	self->ForceOpen(sender, alt_mode);
}

bool Perl_Doors_GetDisableTimer(Doors* self)
{
	return self->GetDisableTimer();
}

void Perl_Doors_SetDisableTimer(Doors* self, bool disable_timer)
{
	self->SetDisableTimer(disable_timer);
}

uint8 Perl_Doors_GetTriggerDoorID(Doors* self)
{
	return self->GetTriggerDoorID();
}

uint8 Perl_Doors_GetTriggerType(Doors* self)
{
	return self->GetTriggerType();
}

bool Perl_Doors_IsLDoNDoor(Doors* self)
{
	return self->IsLDoNDoor();
}

uint32 Perl_Doors_GetClientVersionMask(Doors* self)
{
	return self->GetClientVersionMask();
}

int Perl_Doors_GetDoorParam(Doors* self)
{
	return self->GetDoorParam();
}

bool Perl_Doors_HasDestinationZone(Doors* self)
{
	return self->HasDestinationZone();
}

bool Perl_Doors_IsDestinationZoneSame(Doors* self)
{
	return self->IsDestinationZoneSame();
}

bool Perl_Doors_IsDoorBlacklisted(Doors* self)
{
	return self->IsDoorBlacklisted();
}

std::string Perl_Doors_GetDestinationZoneName(Doors* self)
{
	return self->GetDestinationZoneName();
}

int Perl_Doors_GetDestinationInstanceID(Doors* self)
{
	return self->GetDestinationInstanceID();
}

float Perl_Doors_GetDestinationX(Doors* self)
{
	return self->GetDestinationX();
}

float Perl_Doors_GetDestinationY(Doors* self)
{
	return self->GetDestinationY();
}

float Perl_Doors_GetDestinationZ(Doors* self)
{
	return self->GetDestinationZ();
}

float Perl_Doors_GetDestinationHeading(Doors* self)
{
	return self->GetDestinationHeading();
}

int Perl_Doors_GetDzSwitchID(Doors* self)
{
	return self->GetDzSwitchID();
}

int Perl_Doors_GetInvertState(Doors* self)
{
	return self->GetInvertState();
}

void Perl_Doors_SetInvertState(Doors* self, int invert_state)
{
	self->SetInvertState(invert_state);
}

uint32 Perl_Doors_GetGuildID(Doors* self)
{
	return self->GetGuildID();
}

void perl_register_doors()
{
	perl::interpreter perl(PERL_GET_THX);

	auto package = perl.new_class<Doors>("Doors");
	package.add("CreateDatabaseEntry", &Perl_Doors_CreateDatabaseEntry);
	package.add("ForceClose", (void(*)(Doors*, Mob*))&Perl_Doors_ForceClose);
	package.add("ForceClose", (void(*)(Doors*, Mob*, bool))&Perl_Doors_ForceClose);
	package.add("ForceOpen", (void(*)(Doors*, Mob*))&Perl_Doors_ForceOpen);
	package.add("ForceOpen", (void(*)(Doors*, Mob*, bool))&Perl_Doors_ForceOpen);
	package.add("GetClientVersionMask", &Perl_Doors_GetClientVersionMask);
	package.add("GetDestinationHeading", &Perl_Doors_GetDestinationHeading);
	package.add("GetDestinationInstanceID", &Perl_Doors_GetDestinationInstanceID);
	package.add("GetDestinationX", &Perl_Doors_GetDestinationX);
	package.add("GetDestinationY", &Perl_Doors_GetDestinationY);
	package.add("GetDestinationZ", &Perl_Doors_GetDestinationZ);
	package.add("GetDestinationZoneName", &Perl_Doors_GetDestinationZoneName);
	package.add("GetDisableTimer", &Perl_Doors_GetDisableTimer);
	package.add("GetDoorDBID", &Perl_Doors_GetDoorDBID);
	package.add("GetDoorID", &Perl_Doors_GetDoorID);
	package.add("GetDoorParam", &Perl_Doors_GetDoorParam);
	package.add("GetDzSwitchID", &Perl_Doors_GetDzSwitchID);
	package.add("GetGuildID", &Perl_Doors_GetGuildID);
	package.add("GetHeading", &Perl_Doors_GetHeading);
	package.add("GetID", &Perl_Doors_GetID);
	package.add("GetIncline", &Perl_Doors_GetIncline);
	package.add("GetInvertState", &Perl_Doors_GetInvertState);
	package.add("GetKeyItem", &Perl_Doors_GetKeyItem);
	package.add("GetLockPick", &Perl_Doors_GetLockPick);
	package.add("GetModelName", &Perl_Doors_GetModelName);
	package.add("GetNoKeyring", &Perl_Doors_GetNoKeyring);
	package.add("GetOpenType", &Perl_Doors_GetOpenType);
	package.add("GetSize", &Perl_Doors_GetSize);
	package.add("GetTriggerDoorID", &Perl_Doors_GetTriggerDoorID);
	package.add("GetTriggerType", &Perl_Doors_GetTriggerType);
	package.add("GetX", &Perl_Doors_GetX);
	package.add("GetY", &Perl_Doors_GetY);
	package.add("GetZ", &Perl_Doors_GetZ);
	package.add("HasDestinationZone", &Perl_Doors_HasDestinationZone);
	package.add("IsDestinationZoneSame", &Perl_Doors_IsDestinationZoneSame);
	package.add("IsDoorBlacklisted", &Perl_Doors_IsDoorBlacklisted);
	package.add("IsLDoNDoor", &Perl_Doors_IsLDoNDoor);
	package.add("SetDisableTimer", &Perl_Doors_SetDisableTimer);
	package.add("SetHeading", &Perl_Doors_SetHeading);
	package.add("SetIncline", &Perl_Doors_SetIncline);
	package.add("SetInvertState", &Perl_Doors_SetInvertState);
	package.add("SetKeyItem", &Perl_Doors_SetKeyItem);
	package.add("SetLocation", &Perl_Doors_SetLocation);
	package.add("SetLockPick", &Perl_Doors_SetLockPick);
	package.add("SetModelName", &Perl_Doors_SetModelName);
	package.add("SetNoKeyring", &Perl_Doors_SetNoKeyring);
	package.add("SetOpenType", &Perl_Doors_SetOpenType);
	package.add("SetSize", &Perl_Doors_SetSize);
	package.add("SetX", &Perl_Doors_SetX);
	package.add("SetY", &Perl_Doors_SetY);
	package.add("SetZ", &Perl_Doors_SetZ);
}

#endif //EMBPERL_XS_CLASSES
