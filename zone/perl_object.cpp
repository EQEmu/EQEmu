#include "common/features.h"

#ifdef EMBPERL_XS_CLASSES

#include "zone/embperl.h"
#include "zone/object.h"

bool Perl_Object_IsGroundSpawn(Object* self) // @categories Objects
{
	return self->IsGroundSpawn();
}

void Perl_Object_Close(Object* self) // @categories Objects
{
	self->Close();
}

void Perl_Object_Delete(Object* self) // @categories Objects
{
	self->Delete();
}

void Perl_Object_Delete(Object* self, bool reset_state) // @categories Objects
{
	self->Delete(reset_state);
}

void Perl_Object_StartDecay(Object* self) // @categories Objects
{
	self->StartDecay();
}

void Perl_Object_DeleteItem(Object* self, uint8_t index) // @categories Objects
{
	self->DeleteItem(index);
}

bool Perl_Object_IsObject(Object* self) // @categories Objects
{
	return self->IsObject();
}

bool Perl_Object_Save(Object* self) // @categories Objects
{
	return self->Save();
}

void Perl_Object_SetID(Object* self, uint16_t set_id) // @categories Objects
{
	self->SetID(set_id);
}

void Perl_Object_ClearUser(Object* self) // @categories Objects
{
	self->ClearUser();
}

uint32_t Perl_Object_GetDBID(Object* self) // @categories Objects
{
	return self->GetDBID();
}

uint32_t Perl_Object_GetID(Object* self) // @categories Objects
{
	return self->GetID();
}

float Perl_Object_GetX(Object* self) // @categories Objects
{
	return self->GetX();
}

float Perl_Object_GetY(Object* self) // @categories Objects
{
	return self->GetY();
}

float Perl_Object_GetZ(Object* self) // @categories Objects
{
	return self->GetZ();
}

float Perl_Object_GetHeading(Object* self) // @categories Objects
{
	return self->GetHeadingData();
}

uint32_t Perl_Object_VarSave(Object* self) // @categories Objects
{
	return self->VarSave();
}

uint32_t Perl_Object_GetType(Object* self) // @categories Objects
{
	return self->GetType();
}

void Perl_Object_SetType(Object* self, uint32_t type) // @categories Objects
{
	self->SetType(type);
}

uint32_t Perl_Object_GetIcon(Object* self) // @categories Objects
{
	return self->GetIcon();
}

void Perl_Object_SetIcon(Object* self, uint32_t icon) // @categories Objects
{
	self->SetIcon(icon);
}

uint32_t Perl_Object_GetItemID(Object* self) // @categories Objects
{
	return self->GetItemID();
}

void Perl_Object_SetItemID(Object* self, uint32_t itemid) // @categories Objects
{
	self->SetItemID(itemid);
}

void Perl_Object_SetLocation(Object* self, float x, float y, float z) // @categories Objects
{
	self->SetLocation(x, y, z);
}

void Perl_Object_SetX(Object* self, float x) // @categories Objects
{
	self->SetX(x);
}

void Perl_Object_SetY(Object* self, float y) // @categories Objects
{
	self->SetY(y);
}

void Perl_Object_SetZ(Object* self, float z) // @categories Objects
{
	self->SetZ(z);
}

void Perl_Object_SetHeading(Object* self, float heading) // @categories Objects
{
	self->SetHeading(heading);
}

void Perl_Object_SetModelName(Object* self, const char* name) // @categories Objects
{
	self->SetModelName(name);
}

std::string Perl_Object_GetModelName(Object* self) // @categories Objects
{
	return self->GetModelName();
}

void Perl_Object_Repop(Object* self) // @categories Objects
{
	self->Repop();
}

void Perl_Object_Depop(Object* self) // @categories Objects
{
	self->Depop();
}

uint32_t Perl_Object_GetSolidType(Object* self) // @categories Objects
{
	return self->GetSolidType();
}

void Perl_Object_SetSolidType(Object* self, uint16_t type) // @categories Objects
{
	self->SetSolidType(type);
}

float Perl_Object_GetSize(Object* self) // @categories Objects
{
	return self->GetSize();
}

void Perl_Object_SetSize(Object* self, float size) // @categories Objects
{
	self->SetSize(size);
}

void Perl_Object_SetTiltX(Object* self, float tilt_x) // @categories Objects
{
	self->SetTiltX(tilt_x);
}

void Perl_Object_SetTiltY(Object* self, float tilt_y) // @categories Objects
{
	self->SetTiltY(tilt_y);
}

float Perl_Object_GetTiltX(Object* self) // @categories Objects
{
	return self->GetTiltX();
}

float Perl_Object_GetTiltY(Object* self) // @categories Objects
{
	return self->GetTiltY();
}

bool Perl_Object_ClearEntityVariables(Object* self) // @categories Script Utility
{
	return self->ClearEntityVariables();
}

bool Perl_Object_DeleteEntityVariable(Object* self, std::string variable_name) // @categories Script Utility
{
	return self->DeleteEntityVariable(variable_name);
}

bool Perl_Object_EntityVariableExists(Object* self, std::string variable_name) // @categories Objects
{
	return self->EntityVariableExists(variable_name);
}

std::string Perl_Object_GetEntityVariable(Object* self, std::string variable_name) // @categories Objects
{
	return self->GetEntityVariable(variable_name);
}

perl::array Perl_Object_GetEntityVariables(Object* self) // @categories Script Utility
{
	perl::array a;

	const auto& l = self->GetEntityVariables();
	for (const auto& v : l) {
		a.push_back(v);
	}

	return a;
}

void Perl_Object_SetEntityVariable(Object* self, std::string variable_name, std::string variable_value) // @categories Objects
{
	self->SetEntityVariable(variable_name, variable_value);
}

void perl_register_object()
{
	perl::interpreter perl(PERL_GET_THX);

	auto package = perl.new_class<Object>("Object");
	package.add_base_class("Entity");
	package.add("ClearEntityVariables", &Perl_Object_ClearEntityVariables);
	package.add("ClearUser", &Perl_Object_ClearUser);
	package.add("Close", &Perl_Object_Close);
	package.add("Delete", (void(*)(Object*))&Perl_Object_Delete);
	package.add("Delete", (void(*)(Object*, bool))&Perl_Object_Delete);
	package.add("DeleteEntityVariable", &Perl_Object_DeleteEntityVariable);
	package.add("DeleteItem", &Perl_Object_DeleteItem);
	package.add("Depop", &Perl_Object_Depop);
	package.add("EntityVariableExists", &Perl_Object_EntityVariableExists);
	package.add("GetDBID", &Perl_Object_GetDBID);
	package.add("GetEntityVariable", &Perl_Object_GetEntityVariable);
	package.add("GetEntityVariables", &Perl_Object_GetEntityVariables);
	package.add("GetHeading", &Perl_Object_GetHeading);
	package.add("GetID", &Perl_Object_GetID);
	package.add("GetIcon", &Perl_Object_GetIcon);
	package.add("GetItemID", &Perl_Object_GetItemID);
	package.add("GetModelName", &Perl_Object_GetModelName);
	package.add("GetSize", &Perl_Object_GetSize);
	package.add("GetSolidType", &Perl_Object_GetSolidType);
	package.add("GetTiltX", &Perl_Object_GetTiltX);
	package.add("GetTiltY", &Perl_Object_GetTiltY);
	package.add("GetType", &Perl_Object_GetType);
	package.add("GetX", &Perl_Object_GetX);
	package.add("GetY", &Perl_Object_GetY);
	package.add("GetZ", &Perl_Object_GetZ);
	package.add("IsGroundSpawn", &Perl_Object_IsGroundSpawn);
	package.add("IsObject", &Perl_Object_IsObject);
	package.add("Repop", &Perl_Object_Repop);
	package.add("Save", &Perl_Object_Save);
	package.add("SetEntityVariable", &Perl_Object_SetEntityVariable);
	package.add("SetHeading", &Perl_Object_SetHeading);
	package.add("SetID", &Perl_Object_SetID);
	package.add("SetIcon", &Perl_Object_SetIcon);
	package.add("SetItemID", &Perl_Object_SetItemID);
	package.add("SetLocation", &Perl_Object_SetLocation);
	package.add("SetModelName", &Perl_Object_SetModelName);
	package.add("SetSize", &Perl_Object_SetSize);
	package.add("SetSolidType", &Perl_Object_SetSolidType);
	package.add("SetTiltX", &Perl_Object_SetTiltX);
	package.add("SetTiltY", &Perl_Object_SetTiltY);
	package.add("SetType", &Perl_Object_SetType);
	package.add("SetX", &Perl_Object_SetX);
	package.add("SetY", &Perl_Object_SetY);
	package.add("SetZ", &Perl_Object_SetZ);
	package.add("StartDecay", &Perl_Object_StartDecay);
	package.add("VarSave", &Perl_Object_VarSave);
}

#endif //EMBPERL_XS_CLASSES
