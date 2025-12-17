/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "inventory_profile.h"

#include "common/data_verification.h"
#include "common/evolving_items.h"
#include "common/rulesys.h"
#include "common/shareddb.h"
#include "common/strings.h"

#include <limits.h>

int32 next_item_serial_number = 1;
std::unordered_set<uint64> guids{};

static inline int32 GetNextItemInstSerialNumber()
{
	// The Bazaar relies on each item a client has up for Trade having a unique
	// identifier. This 'SerialNumber' is sent in Serialized item packets and
	// is used in Bazaar packets to identify the item a player is buying or inspecting.
	//
	// E.g. A trader may have 3 Five dose cloudy potions, each with a different number of remaining charges
	// up for sale with different prices.
	//
	// NextItemInstSerialNumber is the next one to hand out.
	//
	// It is very unlikely to reach 2,147,483,647. Maybe we should call abort(), rather than wrapping back to 1.
	if (next_item_serial_number >= INT32_MAX) {
		next_item_serial_number = 1;
	}
	else {
		next_item_serial_number++;
	}

	while (guids.contains(next_item_serial_number)) {
		next_item_serial_number++;
	}

	return next_item_serial_number;
}

//
// class EQ::ItemInstance
//
EQ::ItemInstance::ItemInstance(const ItemData* item, int16 charges) {

	if (item) {
		m_item = new ItemData(*item);
	}

	m_charges = charges;

	if (m_item && m_item->IsClassCommon()) {
		m_color = m_item->Color;
	}

	if (IsEvolving()) {
		SetTimer("evolve", RuleI(EvolvingItems, DelayUponEquipping));
	}

	m_SerialNumber  = GetNextItemInstSerialNumber();
}

EQ::ItemInstance::ItemInstance(SharedDatabase *db, uint32 item_id, int16 charges) {

	m_item     = db->GetItem(item_id);

	if (m_item) {
		m_item = new ItemData(*m_item);
	}

	m_charges = charges;

	if (m_item && m_item->IsClassCommon()) {
		m_color = m_item->Color;
	} else {
		m_color = 0;
	}

	if (IsEvolving()) {
		SetTimer("evolve", RuleI(EvolvingItems, DelayUponEquipping));
	}

	m_SerialNumber  = GetNextItemInstSerialNumber();
}

EQ::ItemInstance::ItemInstance(ItemInstTypes use_type) {
	m_use_type     = use_type;
}

// Make a copy of an EQ::ItemInstance object
EQ::ItemInstance::ItemInstance(const ItemInstance& copy)
{
	m_use_type = copy.m_use_type;

	if (copy.m_item) {
		m_item = new ItemData(*copy.m_item);
	} else {
		m_item = nullptr;
	}

	m_charges       = copy.m_charges;
	m_price         = copy.m_price;
	m_color         = copy.m_color;
	m_merchantslot  = copy.m_merchantslot;
	m_currentslot   = copy.m_currentslot;
	m_attuned       = copy.m_attuned;
	m_merchantcount = copy.m_merchantcount;

	// Copy container contents
	for (auto it = copy.m_contents.begin(); it != copy.m_contents.end(); ++it) {
		ItemInstance* inst_old = it->second;
		ItemInstance* inst_new = nullptr;

		if (inst_old) {
			inst_new = inst_old->Clone();
		}

		if (inst_new) {
			m_contents[it->first] = inst_new;
		}
	}

	std::map<std::string, std::string>::const_iterator iter;
	for (iter = copy.m_custom_data.begin(); iter != copy.m_custom_data.end(); ++iter) {
		m_custom_data[iter->first] = iter->second;
	}

	m_SerialNumber = copy.m_SerialNumber;
	m_custom_data  = copy.m_custom_data;
	m_timers       = copy.m_timers;

	m_exp       = copy.m_exp;
	m_evolveLvl = copy.m_evolveLvl;

	if (copy.m_scaledItem) {
		m_scaledItem = new ItemData(*copy.m_scaledItem);
	} else {
		m_scaledItem = nullptr;
	}

	m_evolving_details    = copy.m_evolving_details;
	m_scaling             = copy.m_scaling;
	m_ornamenticon        = copy.m_ornamenticon;
	m_ornamentidfile      = copy.m_ornamentidfile;
	m_ornament_hero_model = copy.m_ornament_hero_model;
	m_recast_timestamp    = copy.m_recast_timestamp;
	m_new_id_file         = copy.m_new_id_file;
}

// Clean up container contents
EQ::ItemInstance::~ItemInstance()
{
	Clear();
	safe_delete(m_item);
	safe_delete(m_scaledItem);
}

// Query item type
bool EQ::ItemInstance::IsType(item::ItemClass item_class) const
{
	// IsType(<ItemClassTypes>) does not protect against 'm_item = nullptr'

	// Check usage type
	if (m_use_type == ItemInstWorldContainer && item_class == item::ItemClassBag) {
		return true;
	}

	if (!m_item) {
		return false;
	}

	return (m_item->ItemClass == item_class);
}

bool EQ::ItemInstance::IsClassCommon() const
{
	return (m_item && m_item->IsClassCommon());
}

bool EQ::ItemInstance::IsClassBag() const
{
	return (m_item && m_item->IsClassBag());
}

bool EQ::ItemInstance::IsClassBook() const
{
	return (m_item && m_item->IsClassBook());
}

// Is item stackable?
bool EQ::ItemInstance::IsStackable() const
{
	return (m_item && m_item->Stackable);
}

bool EQ::ItemInstance::IsCharged() const
{
	if (!m_item) {
		return false;
	}

	if (m_item->MaxCharges > 1) {
		return true;
	} else {
		return false;
	}
}

// Can item be equipped?
bool EQ::ItemInstance::IsEquipable(uint16 race, uint16 class_) const
{
	if (!m_item || !m_item->Slots) {
		return false;
	}

	return m_item->IsEquipable(race, class_);
}

// Can item be equipped by Class?
bool EQ::ItemInstance::IsClassEquipable(uint16 class_) const
{
	if (!m_item || !m_item->Slots) {
		return false;
	}

	return m_item->IsClassEquipable(class_);
}

// Can item be equipped by Race?
bool EQ::ItemInstance::IsRaceEquipable(uint16 race) const
{
	if (!m_item || !m_item->Slots) {
		return false;
	}

	return m_item->IsRaceEquipable(race);
}

// Can equip at this slot?
bool EQ::ItemInstance::IsEquipable(int16 slot_id) const
{
	if (!m_item || !m_item->Slots) {
		return false;
	}

	if (slot_id < EQ::invslot::EQUIPMENT_BEGIN || slot_id > EQ::invslot::EQUIPMENT_END) {
		return false;
	}

	return ((m_item->Slots & (1 << slot_id)) != 0);
}

bool EQ::ItemInstance::IsAugmentable() const
{
	if (!m_item) {
		return false;
	}

	for (int index = invaug::SOCKET_BEGIN; index <= invaug::SOCKET_END; ++index) {
		if (m_item->AugSlotType[index] != 0) {
			return true;
		}
	}

	return false;
}

bool EQ::ItemInstance::AvailableWearSlot(uint32 aug_wear_slots) const {
	if (!m_item || !m_item->IsClassCommon()) {
		return false;
	}

	int index = invslot::EQUIPMENT_BEGIN;
	for (; index <= invslot::EQUIPMENT_END; ++index) {
		if (m_item->Slots & (1 << index)) {
			if (aug_wear_slots & (1 << index)) {
				break;
			}
		}
	}

	return (index <= EQ::invslot::EQUIPMENT_END);
}

int8 EQ::ItemInstance::AvailableAugmentSlot(int32 augment_type) const
{
	if (!m_item || !m_item->IsClassCommon()) {
		return INVALID_INDEX;
	}

	for (int16 slot_id = invaug::SOCKET_BEGIN; slot_id <= invaug::SOCKET_END; ++slot_id) {
		if (IsAugmentSlotAvailable(augment_type, slot_id)) {
			return slot_id;
		}
	}

	return INVALID_INDEX;
}

bool EQ::ItemInstance::IsAugmentSlotAvailable(int32 augment_type, uint8 slot) const
{
	if (!m_item || !m_item->IsClassCommon() || GetItem(slot)) {
		return false;
	}

	return (
		(
			augment_type == -1 ||
			(
				m_item->AugSlotType[slot] &&
				((1 << (m_item->AugSlotType[slot] - 1)) & augment_type)
			)
		) &&
		(
			RuleB(Items, AugmentItemAllowInvisibleAugments) ||
			m_item->AugSlotVisible[slot]
		)
	);
}

// Retrieve item inside container
EQ::ItemInstance* EQ::ItemInstance::GetItem(uint8 index) const
{
	auto it = m_contents.find(index);
	if (it != m_contents.end()) {
		return it->second;
	}

	return nullptr;
}

uint32 EQ::ItemInstance::GetItemID(uint8 slot) const
{
	const auto item = GetItem(slot);
	if (item) {
		return item->GetID();
	}

	return 0;
}

void EQ::ItemInstance::PutItem(uint8 index, const ItemInstance& inst)
{
	// Clean up item already in slot (if exists)
	DeleteItem(index);

	// Delegate to internal method
	_PutItem(index, inst.Clone());
}

// Remove item inside container
void EQ::ItemInstance::DeleteItem(uint8 index)
{
	ItemInstance* inst = PopItem(index);
	safe_delete(inst);
}

// Remove item from container without memory delete
// Hands over memory ownership to client of this function call
EQ::ItemInstance* EQ::ItemInstance::PopItem(uint8 index)
{
	auto iter = m_contents.find(index);
	if (iter != m_contents.end()) {
		ItemInstance* inst = iter->second;
		m_contents.erase(index);
		return inst; // Return pointer that needs to be deleted (or otherwise managed)
	}

	return nullptr;
}

// Remove all items from container
void EQ::ItemInstance::Clear()
{
	// Destroy container contents
	for (auto iter = m_contents.begin(); iter != m_contents.end(); ++iter) {
		safe_delete(iter->second);
	}
	m_contents.clear();
}

// Remove all items from container
void EQ::ItemInstance::ClearByFlags(byFlagSetting is_nodrop, byFlagSetting is_norent)
{
	// TODO: This needs work...

	// Destroy container contents
	std::map<uint8, ItemInstance*>::const_iterator cur, end, del;
	cur = m_contents.begin();
	end = m_contents.end();
	for (; cur != end;) {
		ItemInstance* inst = cur->second;
		if (inst == nullptr) {
			cur = m_contents.erase(cur);
			continue;
		}

		const ItemData* item = inst->GetItem();
		if (item == nullptr) {
			cur = m_contents.erase(cur);
			continue;
		}

		del = cur;
		++cur;

		switch (is_nodrop) {
		case byFlagSet:
			if (item->NoDrop == 0) {
				safe_delete(inst);
				m_contents.erase(del->first);
				continue;
			}
			// no 'break;' deletes 'byFlagNotSet' type - can't add at the moment because it really *breaks* the process somewhere
		case byFlagNotSet:
			if (item->NoDrop != 0) {
				safe_delete(inst);
				m_contents.erase(del->first);
				continue;
			}
		default:
			break;
		}

		switch (is_norent) {
		case byFlagSet:
			if (item->NoRent == 0) {
				safe_delete(inst);
				m_contents.erase(del->first);
				continue;
			}
			// no 'break;' deletes 'byFlagNotSet' type - can't add at the moment because it really *breaks* the process somewhere
		case byFlagNotSet:
			if (item->NoRent != 0) {
				safe_delete(inst);
				m_contents.erase(del->first);
				continue;
			}
		default:
			break;
		}
	}
}

uint8 EQ::ItemInstance::FirstOpenSlot() const
{
	if (!m_item)
		return INVALID_INDEX;

	uint8 slots = m_item->BagSlots, i;
	for (i = invbag::SLOT_BEGIN; i < slots; i++) {
		if (!GetItem(i))
			break;
	}

	return (i < slots) ? i : INVALID_INDEX;
}

uint8 EQ::ItemInstance::GetTotalItemCount() const
{
	if (!m_item) {
		return 0;
	}

	uint8 item_count = 1;

	if (!m_item->IsClassBag()) {
		return item_count;
	}

	for (int index = invbag::SLOT_BEGIN; index < m_item->BagSlots; ++index) {
		if (GetItem(index)) {
			++item_count;
		}
	}

	return item_count;
}

bool EQ::ItemInstance::IsNoneEmptyContainer()
{
	if (!m_item || !m_item->IsClassBag())
		return false;

	for (int index = invbag::SLOT_BEGIN; index < m_item->BagSlots; ++index) {
		if (GetItem(index))
			return true;
	}

	return false;
}

// Retrieve augment inside item
EQ::ItemInstance* EQ::ItemInstance::GetAugment(uint8 augment_index) const
{
	if (m_item && m_item->IsClassCommon()) {
		return GetItem(augment_index);
	}

	return nullptr;
}

bool EQ::ItemInstance::IsOrnamentationAugment(EQ::ItemInstance* augment) const
{
	if (!m_item || !m_item->IsClassCommon() || !augment) {
		return false;
	}

	const auto augment_item = augment->GetItem();
	if (!augment_item) {
		return false;
	}

	const std::string& idfile = augment_item->IDFile;

	if (
		EQ::ValueWithin(
			augment->GetAugmentType(),
			OrnamentationAugmentTypes::StandardOrnamentation,
			OrnamentationAugmentTypes::SpecialOrnamentation
		) ||
		(
			idfile != "IT63" &&
			idfile != "IT64"
		) ||
		augment_item->HerosForgeModel
	) {
		return true;
	}

	return false;
}

EQ::ItemInstance* EQ::ItemInstance::GetOrnamentationAugment() const
{
	if (!m_item || !m_item->IsClassCommon()) {
		return nullptr;
	}

	for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; i++) {
		const auto augment = GetAugment(i);
		if (augment && IsOrnamentationAugment(augment)) {
			return augment;
		}
	}

	return nullptr;
}

uint32 EQ::ItemInstance::GetOrnamentHeroModel(int32 material_slot) const
{
	// Not a Hero Forge item.
	if (m_ornament_hero_model == 0) {
		return 0;
	}

	// Item is using an explicit Hero Forge ID
	if (m_ornament_hero_model >= 1000) {
		return m_ornament_hero_model;
	}

	// Item is using a shorthand ID
	return (m_ornament_hero_model * 100) + material_slot;
}

bool EQ::ItemInstance::UpdateOrnamentationInfo()
{
	if (!m_item || !m_item->IsClassCommon()) {
		return false;
	}

	const auto augment = GetOrnamentationAugment();

	if (augment) {
		const auto augment_item = GetOrnamentationAugment()->GetItem();

		if (augment_item) {
			SetOrnamentIcon(augment_item->Icon);
			SetOrnamentHeroModel(augment_item->HerosForgeModel);

			if (strlen(augment_item->IDFile) > 2) {
				SetOrnamentationIDFile(Strings::ToUnsignedInt(&augment_item->IDFile[2]));
			} else {
				SetOrnamentationIDFile(0);
			}

			return true;
		}
	}

	SetOrnamentIcon(0);
	SetOrnamentHeroModel(0);
	SetOrnamentationIDFile(0);

	return false;
}

bool EQ::ItemInstance::CanTransform(const ItemData *ItemToTry, const ItemData *Container, bool AllowAll) {
	if (!ItemToTry || !Container) return false;

	if (ItemToTry->ItemType == item::ItemTypeArrow || strnlen(Container->CharmFile, 30) == 0)
		return false;

	if (AllowAll && strncasecmp(Container->CharmFile, "ITEMTRANSFIGSHIELD", 18) && strncasecmp(Container->CharmFile, "ITEMTransfigBow", 15)) {
		switch (ItemToTry->ItemType) {
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 35:
			case 45:
				return true;
		}
	}

	static std::map<std::string, int> types;
	types["itemtransfig1hp"] = 2;
	types["itemtransfig1hs"] = 0;
	types["itemtransfig2hb"] = 4;
	types["itemtransfig2hp"] = 35;
	types["itemtransfig2hs"] = 1;
	types["itemtransfigblunt"] = 3;
	types["itemtransfig1hb"] = 3;
	types["itemtransfigbow"] = 5;
	types["itemtransfighth"] = 45;
	types["itemtransfigshield"] = 8;
	types["itemtransfigslashing"] = 0;

	auto i = types.find(MakeLowerString(Container->CharmFile));
	if (i != types.end() && i->second == ItemToTry->ItemType)
		return true;

	static std::map<std::string, int> typestwo;
	typestwo["itemtransfigblunt"] = 4;
	typestwo["itemtransfigslashing"] = 1;

	i = typestwo.find(MakeLowerString(Container->CharmFile));
	if (i != typestwo.end() && i->second == ItemToTry->ItemType)
		return true;

	return false;
}

uint32 EQ::ItemInstance::GetAugmentItemID(uint8 augment_index) const
{
	if (!m_item || !m_item->IsClassCommon()) {
		return 0;
	}

	return GetItemID(augment_index);
}

// Add an augment to the item
void EQ::ItemInstance::PutAugment(uint8 slot, const ItemInstance& augment)
{
	if (!m_item || !m_item->IsClassCommon())
		return;

	PutItem(slot, augment);
}

void EQ::ItemInstance::PutAugment(SharedDatabase *db, uint8 slot, uint32 item_id)
{
	if (item_id == 0) { return; }
	if (db == nullptr) { return; /* TODO: add log message for nullptr */ }

	const ItemInstance* aug = db->CreateItem(item_id);
	if (aug) {
		PutAugment(slot, *aug);
		safe_delete(aug);
	}
}

// Remove augment from item and destroy it
void EQ::ItemInstance::DeleteAugment(uint8 index)
{
	if (!m_item || !m_item->IsClassCommon())
		return;

	DeleteItem(index);
}

// Remove augment from item and return it
EQ::ItemInstance* EQ::ItemInstance::RemoveAugment(uint8 index)
{
	if (!m_item || !m_item->IsClassCommon())
		return nullptr;

	return PopItem(index);
}

bool EQ::ItemInstance::IsAugmented()
{
	if (!m_item || !m_item->IsClassCommon()) {
		return false;
	}

	for (uint8 slot_id = invaug::SOCKET_BEGIN; slot_id <= invaug::SOCKET_END; ++slot_id) {
		if (GetAugmentItemID(slot_id)) {
			return true;
		}
	}

	return false;
}

bool EQ::ItemInstance::ContainsAugmentByID(uint32 item_id)
{
	if (!m_item || !m_item->IsClassCommon()) {
		return false;
	}

	if (!item_id) {
		return false;
	}

	for (uint8 augment_slot = invaug::SOCKET_BEGIN; augment_slot <= invaug::SOCKET_END; ++augment_slot) {
		if (GetAugmentItemID(augment_slot) == item_id) {
			return true;
		}
	}

	return false;
}

int EQ::ItemInstance::CountAugmentByID(uint32 item_id)
{
	int quantity = 0;
	if (!m_item || !m_item->IsClassCommon()) {
		return quantity;
	}

	if (!item_id) {
		return quantity;
	}

	for (uint8 augment_slot = invaug::SOCKET_BEGIN; augment_slot <= invaug::SOCKET_END; ++augment_slot) {
		if (GetAugmentItemID(augment_slot) == item_id) {
			quantity++;
		}
	}

	return quantity;
}

// Has attack/delay?
bool EQ::ItemInstance::IsWeapon() const
{
	if (!m_item || !m_item->IsClassCommon())
		return false;

	if (m_item->ItemType == item::ItemTypeArrow && m_item->Damage != 0)
		return true;
	else
		return ((m_item->Damage != 0) && (m_item->Delay != 0));
}

bool EQ::ItemInstance::IsAmmo() const
{
	if (!m_item)
		return false;

	if ((m_item->ItemType == item::ItemTypeArrow) ||
		(m_item->ItemType == item::ItemTypeLargeThrowing) ||
		(m_item->ItemType == item::ItemTypeSmallThrowing)
		) {
		return true;
	}

	return false;

}

const EQ::ItemData* EQ::ItemInstance::GetItem() const
{
	if (!m_item)
		return nullptr;

	if (m_scaledItem)
		return m_scaledItem;

	return m_item;
}

const EQ::ItemData* EQ::ItemInstance::GetUnscaledItem() const
{
	// No operator calls and defaults to nullptr
	return m_item;
}

std::string EQ::ItemInstance::GetCustomDataString() const {
	std::string ret_val;
	auto iter = m_custom_data.begin();
	while (iter != m_custom_data.end()) {
		if (ret_val.length() > 0) {
			ret_val += "^";
		}
		ret_val += iter->first;
		ret_val += "^";
		ret_val += iter->second;
		++iter;

		if (ret_val.length() > 0) {
			ret_val += "^";
		}
	}
	return ret_val;
}

void EQ::ItemInstance::SetCustomDataString(const std::string& str)
{
	auto components = Strings::Split(str, "^");
	auto value_count = components.size() / 2;

	for (auto i = 0; i < value_count; i++) {
		auto identifier = components[i * 2];
		auto value = components[(i * 2) + 1];

		SetCustomData(identifier, value);
	}
}

std::string EQ::ItemInstance::GetCustomData(const std::string& identifier) {
	std::map<std::string, std::string>::const_iterator iter = m_custom_data.find(identifier);
	if (iter != m_custom_data.end()) {
		return iter->second;
	}

	return "";
}

void EQ::ItemInstance::SetCustomData(const std::string& identifier, const std::string& value) {
	DeleteCustomData(identifier);
	m_custom_data[identifier] = value;
}

void EQ::ItemInstance::SetCustomData(const std::string& identifier, int value) {
	DeleteCustomData(identifier);
	std::stringstream ss;
	ss << value;
	m_custom_data[identifier] = ss.str();
}

void EQ::ItemInstance::SetCustomData(const std::string& identifier, float value) {
	DeleteCustomData(identifier);
	std::stringstream ss;
	ss << value;
	m_custom_data[identifier] = ss.str();
}

void EQ::ItemInstance::SetCustomData(const std::string& identifier, bool value) {
	DeleteCustomData(identifier);
	std::stringstream ss;
	ss << value;
	m_custom_data[identifier] = ss.str();
}

void EQ::ItemInstance::DeleteCustomData(const std::string& identifier) {
	auto iter = m_custom_data.find(identifier);
	if (iter != m_custom_data.end()) {
		m_custom_data.erase(iter);
	}
}

// Clone a type of EQ::ItemInstance object
// c++ doesn't allow a polymorphic copy constructor,
// so we have to resort to a polymorphic Clone()
EQ::ItemInstance* EQ::ItemInstance::Clone() const
{
	// Pseudo-polymorphic copy constructor
	return new ItemInstance(*this);
}

bool EQ::ItemInstance::IsSlotAllowed(int16 slot_id) const {
	if (!m_item) { return false; }
	else if (InventoryProfile::SupportsContainers(slot_id)) { return true; }
	else if (m_item->Slots & (1 << slot_id)) { return true; }
	else if (slot_id > invslot::EQUIPMENT_END) { return true; } // why do we call 'InventoryProfile::SupportsContainers' with this here?
	else { return false; }
}

bool EQ::ItemInstance::IsDroppable(bool recurse) const
{
	if (!m_item) {
		return false;
	}
	/*if (m_ornamentidfile) // not implemented
		return false;*/
	if (m_attuned) {
		return false;
	}

	if (RuleI(World, FVNoDropFlag) == FVNoDropFlagRule::Enabled && m_item->FVNoDrop == 0) {
		return true;
	}

	if (m_item->NoDrop == 0) {
		return false;
	}

	if (recurse) {
		for (auto iter: m_contents) {
			if (!iter.second) {
				continue;
			}

			if (!iter.second->IsDroppable(recurse)) {
				return false;
			}
		}
	}

	return true;
}

void EQ::ItemInstance::Initialize(SharedDatabase *db) {
	// if there's no actual item, don't do anything
	if (!m_item) {
		return;
	}

	// initialize scaling items
	if (m_item->CharmFileID != 0) {
		m_scaling = true;
		ScaleItem();
	}

	// initialize evolving items
	else if (db && m_item->LoreGroup >= 1000) {
		// not complete yet
	}
}

void EQ::ItemInstance::ScaleItem() {
	if (!m_item)
		return;

	if (m_scaledItem) {
		memcpy(m_scaledItem, m_item, sizeof(ItemData));
	}
	else {
		m_scaledItem = new ItemData(*m_item);
	}

	float Mult = (float)(GetExp()) / 10000;	// scaling is determined by exp, with 10,000 being full stats

	m_scaledItem->AStr = (int8)((float)m_item->AStr*Mult);
	m_scaledItem->ASta = (int8)((float)m_item->ASta*Mult);
	m_scaledItem->AAgi = (int8)((float)m_item->AAgi*Mult);
	m_scaledItem->ADex = (int8)((float)m_item->ADex*Mult);
	m_scaledItem->AInt = (int8)((float)m_item->AInt*Mult);
	m_scaledItem->AWis = (int8)((float)m_item->AWis*Mult);
	m_scaledItem->ACha = (int8)((float)m_item->ACha*Mult);

	m_scaledItem->MR = (int8)((float)m_item->MR*Mult);
	m_scaledItem->PR = (int8)((float)m_item->PR*Mult);
	m_scaledItem->DR = (int8)((float)m_item->DR*Mult);
	m_scaledItem->CR = (int8)((float)m_item->CR*Mult);
	m_scaledItem->FR = (int8)((float)m_item->FR*Mult);

	m_scaledItem->HP = (int32)((float)m_item->HP*Mult);
	m_scaledItem->Mana = (int32)((float)m_item->Mana*Mult);
	m_scaledItem->AC = (int32)((float)m_item->AC*Mult);

	// check these..some may not need to be modified (really need to check all stats/bonuses)
	//m_scaledItem->SkillModValue = (int32)((float)m_item->SkillModValue*Mult);
	//m_scaledItem->BaneDmgAmt = (int8)((float)m_item->BaneDmgAmt*Mult);	// watch (10 entries with charmfileid)
	m_scaledItem->BardValue = (int32)((float)m_item->BardValue*Mult);		// watch (no entries with charmfileid)
	m_scaledItem->ElemDmgAmt = (uint8)((float)m_item->ElemDmgAmt*Mult);		// watch (no entries with charmfileid)
	m_scaledItem->Damage = (uint32)((float)m_item->Damage*Mult);			// watch

	m_scaledItem->CombatEffects = (int8)((float)m_item->CombatEffects*Mult);
	m_scaledItem->Shielding = (int8)((float)m_item->Shielding*Mult);
	m_scaledItem->StunResist = (int8)((float)m_item->StunResist*Mult);
	m_scaledItem->StrikeThrough = (int8)((float)m_item->StrikeThrough*Mult);
	m_scaledItem->ExtraDmgAmt = (uint32)((float)m_item->ExtraDmgAmt*Mult);
	m_scaledItem->SpellShield = (int8)((float)m_item->SpellShield*Mult);
	m_scaledItem->Avoidance = (int8)((float)m_item->Avoidance*Mult);
	m_scaledItem->Accuracy = (int8)((float)m_item->Accuracy*Mult);

	m_scaledItem->FactionAmt1 = (int32)((float)m_item->FactionAmt1*Mult);
	m_scaledItem->FactionAmt2 = (int32)((float)m_item->FactionAmt2*Mult);
	m_scaledItem->FactionAmt3 = (int32)((float)m_item->FactionAmt3*Mult);
	m_scaledItem->FactionAmt4 = (int32)((float)m_item->FactionAmt4*Mult);

	m_scaledItem->Endur = (uint32)((float)m_item->Endur*Mult);
	m_scaledItem->DotShielding = (uint32)((float)m_item->DotShielding*Mult);
	m_scaledItem->Attack = (uint32)((float)m_item->Attack*Mult);
	m_scaledItem->Regen = (uint32)((float)m_item->Regen*Mult);
	m_scaledItem->ManaRegen = (uint32)((float)m_item->ManaRegen*Mult);
	m_scaledItem->EnduranceRegen = (uint32)((float)m_item->EnduranceRegen*Mult);
	m_scaledItem->Haste = (uint32)((float)m_item->Haste*Mult);
	m_scaledItem->DamageShield = (uint32)((float)m_item->DamageShield*Mult);

	m_scaledItem->Purity = (uint32)((float)m_item->Purity*Mult);
	m_scaledItem->BackstabDmg = (uint32)((float)m_item->BackstabDmg*Mult);
	m_scaledItem->DSMitigation = (uint32)((float)m_item->DSMitigation*Mult);
	m_scaledItem->HeroicStr = (int32)((float)m_item->HeroicStr*Mult);
	m_scaledItem->HeroicInt = (int32)((float)m_item->HeroicInt*Mult);
	m_scaledItem->HeroicWis = (int32)((float)m_item->HeroicWis*Mult);
	m_scaledItem->HeroicAgi = (int32)((float)m_item->HeroicAgi*Mult);
	m_scaledItem->HeroicDex = (int32)((float)m_item->HeroicDex*Mult);
	m_scaledItem->HeroicSta = (int32)((float)m_item->HeroicSta*Mult);
	m_scaledItem->HeroicCha = (int32)((float)m_item->HeroicCha*Mult);
	m_scaledItem->HeroicMR = (int32)((float)m_item->HeroicMR*Mult);
	m_scaledItem->HeroicFR = (int32)((float)m_item->HeroicFR*Mult);
	m_scaledItem->HeroicCR = (int32)((float)m_item->HeroicCR*Mult);
	m_scaledItem->HeroicDR = (int32)((float)m_item->HeroicDR*Mult);
	m_scaledItem->HeroicPR = (int32)((float)m_item->HeroicPR*Mult);
	m_scaledItem->HeroicSVCorrup = (int32)((float)m_item->HeroicSVCorrup*Mult);
	m_scaledItem->HealAmt = (int32)((float)m_item->HealAmt*Mult);
	m_scaledItem->SpellDmg = (int32)((float)m_item->SpellDmg*Mult);
	m_scaledItem->Clairvoyance = (uint32)((float)m_item->Clairvoyance*Mult);

	m_scaledItem->CharmFileID = 0;	// this stops the client from trying to scale the item itself.
}

void EQ::ItemInstance::SetTimer(std::string name, uint32 time) {
	Timer t(time);
	t.Start(time, false);
	m_timers[name] = t;
}

void EQ::ItemInstance::StopTimer(std::string name) {
	auto iter = m_timers.find(name);
	if(iter != m_timers.end()) {
		m_timers.erase(iter);
	}
}

void EQ::ItemInstance::ClearTimers() {
	m_timers.clear();
}

int EQ::ItemInstance::GetItemArmorClass(bool augments) const
{
	int ac = 0;
	const auto item = GetItem();
	if (item) {
		ac = item->AC;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					ac += GetAugment(i)->GetItemArmorClass();
	}
	return ac;
}

int EQ::ItemInstance::GetItemElementalDamage(int &magic, int &fire, int &cold, int &poison, int &disease, int &chromatic, int &prismatic, int &physical, int &corruption, bool augments) const
{
	const auto item = GetItem();
	if (item) {
		switch (item->ElemDmgType) {
		case RESIST_MAGIC:
			magic += item->ElemDmgAmt;
			break;
		case RESIST_FIRE:
			fire += item->ElemDmgAmt;
			break;
		case RESIST_COLD:
			cold += item->ElemDmgAmt;
			break;
		case RESIST_POISON:
			poison += item->ElemDmgAmt;
			break;
		case RESIST_DISEASE:
			disease += item->ElemDmgAmt;
			break;
		case RESIST_CHROMATIC:
			chromatic += item->ElemDmgAmt;
			break;
		case RESIST_PRISMATIC:
			prismatic += item->ElemDmgAmt;
			break;
		case RESIST_PHYSICAL:
			physical += item->ElemDmgAmt;
			break;
		case RESIST_CORRUPTION:
			corruption += item->ElemDmgAmt;
			break;
		}

		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					GetAugment(i)->GetItemElementalDamage(magic, fire, cold, poison, disease, chromatic, prismatic, physical, corruption);
	}
	return magic + fire + cold + poison + disease + chromatic + prismatic + physical + corruption;
}

int EQ::ItemInstance::GetItemElementalFlag(bool augments) const
{
	int flag = 0;
	const auto item = GetItem();
	if (item) {
		flag = item->ElemDmgType;
		if (flag)
			return flag;

		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i) {
				if (GetAugment(i))
					flag = GetAugment(i)->GetItemElementalFlag();
				if (flag)
					return flag;
			}
		}
	}
	return flag;
}

int EQ::ItemInstance::GetItemElementalDamage(bool augments) const
{
	int64 damage = 0;
	const auto item = GetItem();
	if (item) {
		damage = item->ElemDmgAmt;
		if (damage)
			return damage;

		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i) {
				if (GetAugment(i))
					damage = GetAugment(i)->GetItemElementalDamage();
				if (damage)
					return damage;
			}
		}
	}
	return damage;
}

int EQ::ItemInstance::GetItemRecommendedLevel(bool augments) const
{
	int level = 0;
	const auto item = GetItem();
	if (item) {
		level = item->RecLevel;

		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i) {
				int temp = 0;
				if (GetAugment(i)) {
					temp = GetAugment(i)->GetItemRecommendedLevel();
					if (temp > level)
						level = temp;
				}
			}
		}
	}

	return level;
}

int EQ::ItemInstance::GetItemRequiredLevel(bool augments) const
{
	int level = 0;
	const auto item = GetItem();
	if (item) {
		level = item->ReqLevel;

		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i) {
				int temp = 0;
				if (GetAugment(i)) {
					temp = GetAugment(i)->GetItemRequiredLevel();
					if (temp > level)
						level = temp;
				}
			}
		}
	}

	return level;
}

int EQ::ItemInstance::GetItemWeaponDamage(bool augments) const
{
	int64 damage = 0;
	const auto item = GetItem();
	if (item) {
		damage = item->Damage;

		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					damage += GetAugment(i)->GetItemWeaponDamage();
		}
	}
	return damage;
}

int EQ::ItemInstance::GetItemBackstabDamage(bool augments) const
{
	int64 damage = 0;
	const auto item = GetItem();
	if (item) {
		damage = item->BackstabDmg;

		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					damage += GetAugment(i)->GetItemBackstabDamage();
		}
	}
	return damage;
}

int EQ::ItemInstance::GetItemBaneDamageBody(bool augments) const
{
	int body = 0;
	const auto item = GetItem();
	if (item) {
		body = item->BaneDmgBody;
		if (body)
			return body;

		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i)) {
					body = GetAugment(i)->GetItemBaneDamageBody();
					if (body)
						return body;
				}
		}
	}
	return body;
}

int EQ::ItemInstance::GetItemBaneDamageRace(bool augments) const
{
	int race = Race::Doug;
	const auto item = GetItem();
	if (item) {
		race = item->BaneDmgRace;
		if (race)
			return race;

		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i)) {
					race = GetAugment(i)->GetItemBaneDamageRace();
					if (race)
						return race;
				}
		}
	}
	return race;
}

int EQ::ItemInstance::GetItemBaneDamageBody(uint8 against, bool augments) const
{
	int64 damage = 0;
	const auto item = GetItem();
	if (item) {
		if (item->BaneDmgBody == against)
			damage += item->BaneDmgAmt;

		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					damage += GetAugment(i)->GetItemBaneDamageBody(against);
		}
	}
	return damage;
}

int EQ::ItemInstance::GetItemBaneDamageRace(uint16 against, bool augments) const
{
	int64 damage = 0;
	const auto item = GetItem();
	if (item) {
		if (item->BaneDmgRace == against)
			damage += item->BaneDmgRaceAmt;

		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					damage += GetAugment(i)->GetItemBaneDamageRace(against);
		}
	}
	return damage;
}

int EQ::ItemInstance::GetItemMagical(bool augments) const
{
	const auto item = GetItem();
	if (item) {
		if (item->Magic)
			return 1;

		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i) && GetAugment(i)->GetItemMagical())
					return 1;
		}
	}
	return 0;
}

int EQ::ItemInstance::GetItemHP(bool augments) const
{
	int hp = 0;
	const auto item = GetItem();
	if (item) {
		hp = item->HP;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					hp += GetAugment(i)->GetItemHP();
	}
	return hp;
}

int EQ::ItemInstance::GetItemMana(bool augments) const
{
	int mana = 0;
	const auto item = GetItem();
	if (item) {
		mana = item->Mana;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					mana += GetAugment(i)->GetItemMana();
	}
	return mana;
}

int EQ::ItemInstance::GetItemEndur(bool augments) const
{
	int endur = 0;
	const auto item = GetItem();
	if (item) {
		endur = item->Endur;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					endur += GetAugment(i)->GetItemEndur();
	}
	return endur;
}

int EQ::ItemInstance::GetItemAttack(bool augments) const
{
	int atk = 0;
	const auto item = GetItem();
	if (item) {
		atk = item->Attack;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					atk += GetAugment(i)->GetItemAttack();
	}
	return atk;
}

int EQ::ItemInstance::GetItemStr(bool augments) const
{
	int str = 0;
	const auto item = GetItem();
	if (item) {
		str = item->AStr;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					str += GetAugment(i)->GetItemStr();
	}
	return str;
}

int EQ::ItemInstance::GetItemSta(bool augments) const
{
	int sta = 0;
	const auto item = GetItem();
	if (item) {
		sta = item->ASta;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					sta += GetAugment(i)->GetItemSta();
	}
	return sta;
}

int EQ::ItemInstance::GetItemDex(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->ADex;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemDex();
	}
	return total;
}

int EQ::ItemInstance::GetItemAgi(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->AAgi;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemAgi();
	}
	return total;
}

int EQ::ItemInstance::GetItemInt(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->AInt;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemInt();
	}
	return total;
}

int EQ::ItemInstance::GetItemWis(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->AWis;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemWis();
	}
	return total;
}

int EQ::ItemInstance::GetItemCha(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->ACha;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemCha();
	}
	return total;
}

int EQ::ItemInstance::GetItemMR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->MR;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemMR();
	}
	return total;
}

int EQ::ItemInstance::GetItemFR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->FR;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemFR();
	}
	return total;
}

int EQ::ItemInstance::GetItemCR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->CR;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemCR();
	}
	return total;
}

int EQ::ItemInstance::GetItemPR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->PR;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemPR();
	}
	return total;
}

int EQ::ItemInstance::GetItemDR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->DR;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemDR();
	}
	return total;
}

int EQ::ItemInstance::GetItemCorrup(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->SVCorruption;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemCorrup();
	}
	return total;
}

int EQ::ItemInstance::GetItemHeroicStr(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicStr;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicStr();
	}
	return total;
}

int EQ::ItemInstance::GetItemHeroicSta(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicSta;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicSta();
	}
	return total;
}

int EQ::ItemInstance::GetItemHeroicDex(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicDex;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicDex();
	}
	return total;
}

int EQ::ItemInstance::GetItemHeroicAgi(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicAgi;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicAgi();
	}
	return total;
}

int EQ::ItemInstance::GetItemHeroicInt(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicInt;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicInt();
	}
	return total;
}

int EQ::ItemInstance::GetItemHeroicWis(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicWis;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicWis();
	}
	return total;
}

int EQ::ItemInstance::GetItemHeroicCha(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicCha;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicCha();
	}
	return total;
}

int EQ::ItemInstance::GetItemHeroicMR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicMR;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicMR();
	}
	return total;
}

int EQ::ItemInstance::GetItemHeroicFR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicFR;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicFR();
	}
	return total;
}

int EQ::ItemInstance::GetItemHeroicCR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicCR;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicCR();
	}
	return total;
}

int EQ::ItemInstance::GetItemHeroicPR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicPR;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicPR();
	}
	return total;
}

int EQ::ItemInstance::GetItemHeroicDR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicDR;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicDR();
	}
	return total;
}

int EQ::ItemInstance::GetItemHeroicCorrup(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicSVCorrup;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicCorrup();
	}
	return total;
}

int EQ::ItemInstance::GetItemHaste(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->Haste;
		if (augments)
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i)
				if (GetAugment(i)) {
					int temp = GetAugment(i)->GetItemHaste();
					if (temp > total)
						total = temp;
				}
	}
	return total;
}

int EQ::ItemInstance::RemoveTaskDeliveredItems()
{
	int count = IsStackable() ? GetCharges() : 1;
	count -= GetTaskDeliveredCount();
	if (IsStackable())
	{
		SetCharges(count);
	}
	SetTaskDeliveredCount(0);
	return count;
}

uint32 EQ::ItemInstance::GetItemGuildFavor() const
{
	uint32 total = 0;
	const auto item = GetItem();
	if (item) {
		return total = item->GuildFavor;
	}
	return 0;
}

std::vector<uint32> EQ::ItemInstance::GetAugmentIDs() const
{
	std::vector<uint32> augments;

	for (uint8 slot_id = invaug::SOCKET_BEGIN; slot_id <= invaug::SOCKET_END; slot_id++) {
		augments.push_back(GetAugment(slot_id) ? GetAugmentItemID(slot_id) : 0);
	}

	return augments;
}

std::vector<std::string> EQ::ItemInstance::GetAugmentNames() const
{
	std::vector<std::string> augment_names;

	for (uint8 slot_id = invaug::SOCKET_BEGIN; slot_id <= invaug::SOCKET_END; slot_id++) {
		const auto augment = GetAugment(slot_id);
		augment_names.push_back(augment ? augment->GetItem()->Name : "");
	}

	return augment_names;
}

int EQ::ItemInstance::GetItemRegen(bool augments) const
{
	int        stat = 0;
	const auto item = GetItem();
	if (item) {
		stat = item->Regen;
		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i) {
				if (GetAugment(i)) {
					stat += GetAugment(i)->GetItemRegen();
				}
			}
		}
	}
	return stat;
}

int EQ::ItemInstance::GetItemManaRegen(bool augments) const
{
	int        stat = 0;
	const auto item = GetItem();
	if (item) {
		stat = item->ManaRegen;
		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i) {
				if (GetAugment(i)) {
					stat += GetAugment(i)->GetItemManaRegen();
				}
			}
		}
	}
	return stat;
}

int EQ::ItemInstance::GetItemDamageShield(bool augments) const
{
	int        stat = 0;
	const auto item = GetItem();
	if (item) {
		stat = item->DamageShield;
		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i) {
				if (GetAugment(i)) {
					stat += GetAugment(i)->GetItemDamageShield();
				}
			}
		}
	}
	return stat;
}

int EQ::ItemInstance::GetItemDSMitigation(bool augments) const
{
	int        stat = 0;
	const auto item = GetItem();
	if (item) {
		stat = item->DSMitigation;
		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i) {
				if (GetAugment(i)) {
					stat += GetAugment(i)->GetItemDSMitigation();
				}
			}
		}
	}
	return stat;
}

int EQ::ItemInstance::GetItemHealAmt(bool augments) const
{
	int        stat = 0;
	const auto item = GetItem();
	if (item) {
		stat = item->HealAmt;
		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i) {
				if (GetAugment(i)) {
					stat += GetAugment(i)->GetItemHealAmt();
				}
			}
		}
	}
	return stat;
}

int EQ::ItemInstance::GetItemSpellDamage(bool augments) const
{
	int        stat = 0;
	const auto item = GetItem();
	if (item) {
		stat = item->SpellDmg;
		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i) {
				if (GetAugment(i)) {
					stat += GetAugment(i)->GetItemSpellDamage();
				}
			}
		}
	}
	return stat;
}

int EQ::ItemInstance::GetItemClairvoyance(bool augments) const
{
	int        stat = 0;
	const auto item = GetItem();
	if (item) {
		stat = item->Clairvoyance;
		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i) {
				if (GetAugment(i)) {
					stat += GetAugment(i)->GetItemClairvoyance();
				}
			}
		}
	}
	return stat;
}

int EQ::ItemInstance::GetItemSkillsStat(EQ::skills::SkillType skill, bool augments) const
{
	int        stat = 0;
	const auto item = GetItem();
	if (item) {
		stat = item->ExtraDmgSkill == skill ? item->ExtraDmgAmt : 0;
		if (augments) {
			for (int i = invaug::SOCKET_BEGIN; i <= invaug::SOCKET_END; ++i) {
				if (GetAugment(i)) {
					stat += GetAugment(i)->GetItemSkillsStat(skill);
				}
			}
		}
	}
	return stat;
}

void EQ::ItemInstance::AddGUIDToMap(uint64 existing_serial_number)
{
	guids.emplace(existing_serial_number);
}

void EQ::ItemInstance::ClearGUIDMap()
{
	guids.clear();
}

bool EQ::ItemInstance::TransferOwnership(Database &db, const uint32 to_char_id) const
{
	if (!to_char_id || !IsEvolving()) {
		return false;
	}

	SetEvolveCharID(to_char_id);
	CharacterEvolvingItemsRepository::UpdateCharID(db, GetEvolveUniqueID(), to_char_id);
	return true;
}

uint32 EQ::ItemInstance::GetAugmentEvolveUniqueID(uint8 augment_index) const
{
	if (!m_item || !m_item->IsClassCommon()) {
		return 0;
	}

	const auto item = GetItem(augment_index);
	if (item) {
		return item->GetEvolveUniqueID();
	}

	return 0;
}

void EQ::ItemInstance::SetTimer(std::string name, uint32 time) const{
	Timer t(time);
	t.Start(time, false);
	m_timers[name] = t;
}

void EQ::ItemInstance::SetEvolveEquipped(const bool in) const
{
	if (!IsEvolving()) {
		return;
	}

	m_evolving_details.equipped = in;
	if (in && !GetTimers().contains("evolve")) {
		SetTimer("evolve", RuleI(EvolvingItems, DelayUponEquipping));
		return;
	}

	if (in) {
		GetTimers().at("evolve").SetTimer(RuleI(EvolvingItems, DelayUponEquipping));
		return;
	}

	GetTimers().at("evolve").Disable();
}
