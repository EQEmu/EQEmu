#ifdef LUA_EQEMU

#include "lua_iteminst.h"

#include "zone/lua_item.h"
#include "zone/masterentity.h"

#include "luabind/luabind.hpp"
#include "luabind/object.hpp"

Lua_ItemInst::Lua_ItemInst(int item_id) {
	SetLuaPtrData(database.CreateItem(item_id));
	cloned_ = true;
}

Lua_ItemInst::Lua_ItemInst(int item_id, int charges) {
	SetLuaPtrData(database.CreateItem(item_id, charges));
	cloned_ = true;
}

Lua_ItemInst::~Lua_ItemInst() {
	if (cloned_) {
		EQ::ItemInstance *ptr = GetLuaPtrData();
		delete ptr;
	}
}

Lua_ItemInst& Lua_ItemInst::operator=(const Lua_ItemInst& o) {
	if(o.cloned_) {
		cloned_ = true;
		d_ = new EQ::ItemInstance(*o.d_);
	} else {
		cloned_ = false;
		d_ = o.d_;
	}
	return *this;
}

Lua_ItemInst::Lua_ItemInst(const Lua_ItemInst& o) {
	if(o.cloned_) {
		cloned_ = true;
		d_ = new EQ::ItemInstance(*o.d_);
	} else {
		cloned_ = false;
		d_ = o.d_;
	}
}

bool Lua_ItemInst::IsType(int item_class) {
	Lua_Safe_Call_Bool();
	return self->IsType(static_cast<EQ::item::ItemClass>(item_class));
}

bool Lua_ItemInst::IsStackable() {
	Lua_Safe_Call_Bool();
	return self->IsStackable();
}

bool Lua_ItemInst::IsEquipable(uint16 race_bitmask, uint16 class_bitmask) {
	Lua_Safe_Call_Bool();
	return self->IsEquipable(race_bitmask, class_bitmask);
}

bool Lua_ItemInst::IsEquipable(int16 slot_id) {
	Lua_Safe_Call_Bool();
	return self->IsEquipable(slot_id);
}

bool Lua_ItemInst::IsAugmentable() {
	Lua_Safe_Call_Bool();
	return self->IsAugmentable();
}

int Lua_ItemInst::GetAugmentType() {
	Lua_Safe_Call_Int();
	return self->GetAugmentType();
}

bool Lua_ItemInst::IsExpendable() {
	Lua_Safe_Call_Bool();
	return self->IsExpendable();
}

Lua_ItemInst Lua_ItemInst::GetItem(uint8 slot_id) {
	Lua_Safe_Call_Class(Lua_ItemInst);
	return Lua_ItemInst(self->GetItem(slot_id));
}

Lua_Item Lua_ItemInst::GetItem() {
	Lua_Safe_Call_Class(Lua_Item);
	return Lua_Item(self->GetItem());
}

Lua_Item Lua_ItemInst::GetUnscaledItem() {
	Lua_Safe_Call_Class(Lua_Item);
	return self->GetUnscaledItem();
}

uint32 Lua_ItemInst::GetItemID(uint8 slot_id) {
	Lua_Safe_Call_Int();
	return self->GetItemID(slot_id);
}

uint8 Lua_ItemInst::GetTotalItemCount() {
	Lua_Safe_Call_Int();
	return self->GetTotalItemCount();
}

Lua_ItemInst Lua_ItemInst::GetAugment(uint8 slot_id) {
	Lua_Safe_Call_Class(Lua_ItemInst);
	return self->GetAugment(slot_id);
}

uint32 Lua_ItemInst::GetAugmentItemID(uint8 slot_id) {
	Lua_Safe_Call_Int();
	return self->GetAugmentItemID(slot_id);
}

bool Lua_ItemInst::IsAugmented() {
	Lua_Safe_Call_Bool();
	return self->IsAugmented();
}

bool Lua_ItemInst::IsWeapon() {
	Lua_Safe_Call_Bool();
	return self->IsWeapon();
}

bool Lua_ItemInst::IsAmmo() {
	Lua_Safe_Call_Bool();
	return self->IsAmmo();
}

uint32 Lua_ItemInst::GetID() {
	Lua_Safe_Call_Int();
	return self->GetID();
}

uint32 Lua_ItemInst::GetItemScriptID() {
	Lua_Safe_Call_Int();
	return self->GetItemScriptID();
}

int Lua_ItemInst::GetCharges() {
	Lua_Safe_Call_Int();
	return self->GetCharges();
}

void Lua_ItemInst::SetCharges(int charges) {
	Lua_Safe_Call_Void();
	return self->SetCharges(charges);
}

uint32 Lua_ItemInst::GetPrice() {
	Lua_Safe_Call_Int();
	return self->GetPrice();
}

void Lua_ItemInst::SetPrice(uint32 price) {
	Lua_Safe_Call_Void();
	return self->SetPrice(price);
}

void Lua_ItemInst::SetColor(uint32 color) {
	Lua_Safe_Call_Void();
	return self->SetColor(color);
}

uint32 Lua_ItemInst::GetColor() {
	Lua_Safe_Call_Int();
	return self->GetColor();
}

bool Lua_ItemInst::IsInstNoDrop() {
	Lua_Safe_Call_Bool();
	return self->IsAttuned();
}

bool Lua_ItemInst::IsAttuned() {
	Lua_Safe_Call_Bool();
	return self->IsAttuned();
}

void Lua_ItemInst::SetInstNoDrop(bool flag) {
	Lua_Safe_Call_Void();
	return self->SetAttuned(flag);
}

void Lua_ItemInst::SetAttuned(bool flag) {
	Lua_Safe_Call_Void();
	return self->SetAttuned(flag);
}

std::string Lua_ItemInst::GetCustomDataString() {
	Lua_Safe_Call_String();
	return self->GetCustomDataString();
}

void Lua_ItemInst::SetCustomData(const std::string &identifier, const std::string& value) {
	Lua_Safe_Call_Void();
	self->SetCustomData(identifier, value);
}

void Lua_ItemInst::SetCustomData(const std::string& identifier, int value) {
	Lua_Safe_Call_Void();
	self->SetCustomData(identifier, value);
}

void Lua_ItemInst::SetCustomData(const std::string& identifier, float value) {
	Lua_Safe_Call_Void();
	self->SetCustomData(identifier, value);
}

void Lua_ItemInst::SetCustomData(const std::string& identifier, bool value) {
	Lua_Safe_Call_Void();
	self->SetCustomData(identifier, value);
}

std::string Lua_ItemInst::GetCustomData(const std::string& identifier) {
	Lua_Safe_Call_String();
	return self->GetCustomData(identifier);
}

void Lua_ItemInst::DeleteCustomData(const std::string& identifier) {
	Lua_Safe_Call_Void();
	self->DeleteCustomData(identifier);
}

void Lua_ItemInst::SetScale(double scale_factor) {
	Lua_Safe_Call_Void();
	self->SetExp((int)(scale_factor*10000+.5));
}

void Lua_ItemInst::SetScaling(bool v) {
	Lua_Safe_Call_Void();
	self->SetScaling(v);
}

uint32 Lua_ItemInst::GetExp() {
	Lua_Safe_Call_Int();
	return self->GetExp();
}

void Lua_ItemInst::SetExp(uint32 exp) {
	Lua_Safe_Call_Void();
	self->SetExp(exp);
}

void Lua_ItemInst::AddExp(uint32 exp) {
	Lua_Safe_Call_Void();
	self->AddExp(exp);
}

int8 Lua_ItemInst::GetMaxEvolveLvl() {
	Lua_Safe_Call_Int();
	return self->GetMaxEvolveLvl();
}

Lua_ItemInst Lua_ItemInst::Clone() {
	Lua_Safe_Call_Class(Lua_ItemInst);
	return Lua_ItemInst(self->Clone(), true);
}

void Lua_ItemInst::SetTimer(std::string name, uint32 time) {
	Lua_Safe_Call_Void();
	self->SetTimer(name, time);
}

void Lua_ItemInst::StopTimer(std::string name) {
	Lua_Safe_Call_Void();
	self->StopTimer(name);
}

void Lua_ItemInst::ClearTimers() {
	Lua_Safe_Call_Void();
	self->ClearTimers();
}

bool Lua_ItemInst::ContainsAugmentByID(uint32 item_id) {
	Lua_Safe_Call_Bool();
	return self->ContainsAugmentByID(item_id);
}

int Lua_ItemInst::CountAugmentByID(uint32 item_id) {
	Lua_Safe_Call_Int();
	return self->CountAugmentByID(item_id);
}

int Lua_ItemInst::GetTaskDeliveredCount() {
	Lua_Safe_Call_Int();
	return self->GetTaskDeliveredCount();
}

int Lua_ItemInst::RemoveTaskDeliveredItems() {
	Lua_Safe_Call_Int();
	return self->RemoveTaskDeliveredItems();
}

std::string Lua_ItemInst::GetName() {
	Lua_Safe_Call_String();
	return self->GetItem()->Name;
}

void Lua_ItemInst::ItemSay(const char* text) // @categories Inventory and Items
{
	Lua_Safe_Call_Void();
	quest_manager.GetInitiator()->ChannelMessageSend(self->GetItem()->Name, 0, ChatChannel_Say, Language::CommonTongue, Language::MaxValue, text);
}

void Lua_ItemInst::ItemSay(const char* text, uint8 language_id) // @categories Inventory and Items
{
	Lua_Safe_Call_Void();
	quest_manager.GetInitiator()->ChannelMessageSend(self->GetItem()->Name, 0, ChatChannel_Say, language_id, Language::MaxValue, text);
}

luabind::object Lua_ItemInst::GetAugmentIDs(lua_State* L)
{
	auto lua_table = luabind::newtable(L);

	if (d_) {
		auto self = reinterpret_cast<NativeType*>(d_);

		const auto& augment_ids = self->GetAugmentIDs();

		int index = 1;

		for (auto augment_id : augment_ids) {
			lua_table[index] = augment_id;
			index++;
		}
	}

	return lua_table;
}

std::string Lua_ItemInst::GetItemLink()
{
	Lua_Safe_Call_String();

	EQ::SayLinkEngine linker;
	linker.SetLinkType(EQ::saylink::SayLinkItemInst);
	linker.SetItemInst(self);

	return linker.GenerateLink();
}

void Lua_ItemInst::AddEvolveAmount(uint64 amount)
{
	Lua_Safe_Call_Void();
	self->SetEvolveAddToCurrentAmount(amount);
}

uint32 Lua_ItemInst::GetAugmentEvolveUniqueID(uint8 slot_id)
{
	Lua_Safe_Call_Int();
	return self->GetAugmentEvolveUniqueID(slot_id);
}

bool Lua_ItemInst::GetEvolveActivated()
{
	Lua_Safe_Call_Bool();
	return self->GetEvolveActivated();
}

uint64 Lua_ItemInst::GetEvolveAmount()
{
	Lua_Safe_Call_Int();
	return self->GetEvolveCurrentAmount();
}

uint32 Lua_ItemInst::GetEvolveCharacterID()
{
	Lua_Safe_Call_Int();
	return self->GetEvolveCharID();
}

bool Lua_ItemInst::GetEvolveEquipped()
{
	Lua_Safe_Call_Bool();
	return self->GetEvolveEquipped();
}

uint32 Lua_ItemInst::GetEvolveFinalItemID()
{
	Lua_Safe_Call_Int();
	return self->GetEvolveFinalItemID();
}

uint32 Lua_ItemInst::GetEvolveItemID()
{
	Lua_Safe_Call_Int();
	return self->GetEvolveItemID();
}

int8 Lua_ItemInst::GetEvolveLevel()
{
	Lua_Safe_Call_Int();
	return self->GetEvolveLvl();
}

uint32 Lua_ItemInst::GetEvolveLoreID()
{
	Lua_Safe_Call_Int();
	return self->GetEvolveLoreID();
}

float Lua_ItemInst::GetEvolveProgression()
{
	Lua_Safe_Call_Real();
	return self->GetEvolveProgression();
}

uint64 Lua_ItemInst::GetEvolveUniqueID()
{
	Lua_Safe_Call_Int();
	return self->GetEvolveUniqueID();
}

bool Lua_ItemInst::IsEvolving()
{
	Lua_Safe_Call_Bool();
	return self->IsEvolving();
}

void Lua_ItemInst::SetEvolveAmount(uint64 amount)
{
	Lua_Safe_Call_Void();
	self->SetEvolveCurrentAmount(amount);
}

void Lua_ItemInst::SetEvolveProgression(float amount)
{
	Lua_Safe_Call_Void();
	self->SetEvolveProgression(amount);
}

int Lua_ItemInst::GetSerialNumber()
{
	Lua_Safe_Call_Int();
	return self->GetSerialNumber();
}

luabind::scope lua_register_iteminst() {
	return luabind::class_<Lua_ItemInst>("ItemInst")
	.def(luabind::constructor<>())
	.def(luabind::constructor<int>())
	.def(luabind::constructor<int,int>())
	.property("null", &Lua_ItemInst::Null)
	.property("valid", &Lua_ItemInst::Valid)
	.def("AddExp", (void(Lua_ItemInst::*)(uint32))&Lua_ItemInst::AddExp)
	.def("AddEvolveAmount", (void(Lua_ItemInst::*)(uint64))&Lua_ItemInst::AddEvolveAmount)
	.def("ClearTimers", (void(Lua_ItemInst::*)(void))&Lua_ItemInst::ClearTimers)
	.def("Clone", (Lua_ItemInst(Lua_ItemInst::*)(void))&Lua_ItemInst::Clone)
	.def("ContainsAugmentByID", (bool(Lua_ItemInst::*)(uint32))&Lua_ItemInst::ContainsAugmentByID)
	.def("CountAugmentByID", (int(Lua_ItemInst::*)(uint32))&Lua_ItemInst::CountAugmentByID)
	.def("DeleteCustomData", (void(Lua_ItemInst::*)(const std::string &))&Lua_ItemInst::DeleteCustomData)
	.def("GetAugment", (Lua_ItemInst(Lua_ItemInst::*)(int))&Lua_ItemInst::GetAugment)
	.def("GetAugmentEvolveUniqueID", (uint32(Lua_ItemInst::*)(uint8))&Lua_ItemInst::GetAugmentEvolveUniqueID)
	.def("GetAugmentIDs", (luabind::object(Lua_ItemInst::*)(lua_State*))&Lua_ItemInst::GetAugmentIDs)
	.def("GetAugmentItemID", (uint32(Lua_ItemInst::*)(int))&Lua_ItemInst::GetAugmentItemID)
	.def("GetAugmentType", (int(Lua_ItemInst::*)(void))&Lua_ItemInst::GetAugmentType)
	.def("GetCharges", (int(Lua_ItemInst::*)(void))&Lua_ItemInst::GetCharges)
	.def("GetColor", (uint32(Lua_ItemInst::*)(void))&Lua_ItemInst::GetColor)
	.def("GetCustomData", (std::string(Lua_ItemInst::*)(const std::string &))&Lua_ItemInst::GetCustomData)
	.def("GetCustomDataString", (std::string(Lua_ItemInst::*)(void))&Lua_ItemInst::GetCustomDataString)
	.def("GetExp", (uint32(Lua_ItemInst::*)(void))&Lua_ItemInst::GetExp)
	.def("GetEvolveActivated", (bool(Lua_ItemInst::*)(void))&Lua_ItemInst::GetEvolveActivated)
	.def("GetEvolveAmount", (uint64(Lua_ItemInst::*)(void))&Lua_ItemInst::GetEvolveAmount)
	.def("GetEvolveCharacterID", (uint32(Lua_ItemInst::*)(void))&Lua_ItemInst::GetEvolveCharacterID)
	.def("GetEvolveEquipped", (bool(Lua_ItemInst::*)(void))&Lua_ItemInst::GetEvolveEquipped)
	.def("GetEvolveFinalItemID", (uint64(Lua_ItemInst::*)(void))&Lua_ItemInst::GetEvolveFinalItemID)
	.def("GetEvolveItemID", (uint32(Lua_ItemInst::*)(void))&Lua_ItemInst::GetEvolveItemID)
	.def("GetEvolveLevel", (int8(Lua_ItemInst::*)(void))&Lua_ItemInst::GetEvolveLevel)
	.def("GetEvolveLoreID", (uint32(Lua_ItemInst::*)(void))&Lua_ItemInst::GetEvolveLoreID)
	.def("GetEvolveProgression", (float(Lua_ItemInst::*)(void))&Lua_ItemInst::GetEvolveProgression)
	.def("GetEvolveUniqueID", (uint64(Lua_ItemInst::*)(void))&Lua_ItemInst::GetEvolveUniqueID)
	.def("GetID", (uint32(Lua_ItemInst::*)(void))&Lua_ItemInst::GetID)
	.def("GetItem", (Lua_Item(Lua_ItemInst::*)(void))&Lua_ItemInst::GetItem)
	.def("GetItem", (Lua_ItemInst(Lua_ItemInst::*)(uint8))&Lua_ItemInst::GetItem)
	.def("GetItemID", (uint32(Lua_ItemInst::*)(int))&Lua_ItemInst::GetItemID)
	.def("GetItemLink", (std::string(Lua_ItemInst::*)(void))&Lua_ItemInst::GetItemLink)
	.def("GetItemScriptID", (uint32(Lua_ItemInst::*)(void))&Lua_ItemInst::GetItemScriptID)
	.def("GetMaxEvolveLvl", (int(Lua_ItemInst::*)(void))&Lua_ItemInst::GetMaxEvolveLvl)
	.def("GetName", (std::string(Lua_ItemInst::*)(void))&Lua_ItemInst::GetName)
	.def("GetSerialNumber", (int(Lua_ItemInst::*)(void))&Lua_ItemInst::GetSerialNumber)
	.def("GetPrice", (uint32(Lua_ItemInst::*)(void))&Lua_ItemInst::GetPrice)
	.def("GetTaskDeliveredCount", &Lua_ItemInst::GetTaskDeliveredCount)
	.def("GetTotalItemCount", (uint8(Lua_ItemInst::*)(void))&Lua_ItemInst::GetTotalItemCount)
	.def("GetUnscaledItem", (Lua_ItemInst(Lua_ItemInst::*)(int))&Lua_ItemInst::GetUnscaledItem)
	.def("IsAmmo", (bool(Lua_ItemInst::*)(void))&Lua_ItemInst::IsAmmo)
	.def("IsAttuned", (bool(Lua_ItemInst::*)(void))&Lua_ItemInst::IsAttuned)
	.def("IsAugmentable", (bool(Lua_ItemInst::*)(void))&Lua_ItemInst::IsAugmentable)
	.def("IsAugmented", (bool(Lua_ItemInst::*)(void))&Lua_ItemInst::IsAugmented)
	.def("IsEquipable", (bool(Lua_ItemInst::*)(int16))&Lua_ItemInst::IsEquipable)
	.def("IsEquipable", (bool(Lua_ItemInst::*)(uint16,uint16))&Lua_ItemInst::IsEquipable)
	.def("IsEvolving", (bool(Lua_ItemInst::*)(void))&Lua_ItemInst::IsEvolving)
	.def("IsExpendable", (bool(Lua_ItemInst::*)(void))&Lua_ItemInst::IsExpendable)
	.def("IsInstNoDrop", (bool(Lua_ItemInst::*)(void))&Lua_ItemInst::IsInstNoDrop)
	.def("IsStackable", (bool(Lua_ItemInst::*)(void))&Lua_ItemInst::IsStackable)
	.def("IsType", (bool(Lua_ItemInst::*)(int))&Lua_ItemInst::IsType)
	.def("IsWeapon", (bool(Lua_ItemInst::*)(void))&Lua_ItemInst::IsWeapon)
	.def("ItemSay", (void(Lua_ItemInst::*)(const char*))&Lua_ItemInst::ItemSay)
	.def("ItemSay", (void(Lua_ItemInst::*)(const char*, uint8))&Lua_ItemInst::ItemSay)
	.def("RemoveTaskDeliveredItems", &Lua_ItemInst::RemoveTaskDeliveredItems)
	.def("SetAttuned", (void(Lua_ItemInst::*)(bool))&Lua_ItemInst::SetAttuned)
	.def("SetCharges", (void(Lua_ItemInst::*)(int))&Lua_ItemInst::SetCharges)
	.def("SetColor", (void(Lua_ItemInst::*)(uint32))&Lua_ItemInst::SetColor)
	.def("SetCustomData", (void(Lua_ItemInst::*)(const std::string&,bool))&Lua_ItemInst::SetCustomData)
	.def("SetCustomData", (void(Lua_ItemInst::*)(const std::string&,float))&Lua_ItemInst::SetCustomData)
	.def("SetCustomData", (void(Lua_ItemInst::*)(const std::string&,int))&Lua_ItemInst::SetCustomData)
	.def("SetCustomData", (void(Lua_ItemInst::*)(const std::string&,const std::string&))&Lua_ItemInst::SetCustomData)
	.def("SetEvolveAmount", (void(Lua_ItemInst::*)(uint64))&Lua_ItemInst::SetEvolveAmount)
	.def("SetEvolveProgression", (void(Lua_ItemInst::*)(float))&Lua_ItemInst::SetEvolveProgression)
	.def("SetExp", (void(Lua_ItemInst::*)(uint32))&Lua_ItemInst::SetExp)
	.def("SetInstNoDrop", (void(Lua_ItemInst::*)(bool))&Lua_ItemInst::SetInstNoDrop)
	.def("SetPrice", (void(Lua_ItemInst::*)(uint32))&Lua_ItemInst::SetPrice)
	.def("SetScale", (void(Lua_ItemInst::*)(double))&Lua_ItemInst::SetScale)
	.def("SetScaling", (void(Lua_ItemInst::*)(bool))&Lua_ItemInst::SetScaling)
	.def("SetTimer", (void(Lua_ItemInst::*)(std::string,uint32))&Lua_ItemInst::SetTimer)
	.def("StopTimer", (void(Lua_ItemInst::*)(std::string))&Lua_ItemInst::StopTimer);
}

#endif
