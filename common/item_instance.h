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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  04111-1307  USA
*/

// @merth notes:
// These classes could be optimized with database reads/writes by storing
// a status flag indicating how object needs to interact with database

#pragma once

#include "common/bodytypes.h"
#include "common/deity.h"
#include "common/eq_constants.h"
#include "common/evolving_items.h"
#include "common/item_data.h"
#include "common/memory_buffer.h"
#include "common/repositories/character_evolving_items_repository.h"
#include "common/timer.h"

#include <map>

class ItemParse;			// Parses item packets
class EvolveInfo;			// Stores information about an evolving item family


// Specifies usage type for item inside EQ::ItemInstance
enum ItemInstTypes
{
	ItemInstNormal = 0,
	ItemInstWorldContainer
};

typedef enum {
	byFlagIgnore,	//do not consider this flag
	byFlagSet,		//apply action if the flag is set
	byFlagNotSet	//apply action if the flag is NOT set
} byFlagSetting;

enum OrnamentationAugmentTypes {
	StandardOrnamentation = 20,
	SpecialOrnamentation = 21
};

class SharedDatabase;

// ########################################
// Class: EQ::ItemInstance
//	Base class for an instance of an item
//	An item instance encapsulates item data + data specific
//	to an item instance (includes dye, augments, charges, etc)
namespace EQ
{
	class InventoryProfile;

	class ItemInstance {
	public:
		/////////////////////////
		// Methods
		/////////////////////////

		// Constructors/Destructor
		ItemInstance(const ItemData* item = nullptr, int16 charges = 0);

		ItemInstance(SharedDatabase *db, uint32 item_id, int16 charges = 0);

		ItemInstance(ItemInstTypes use_type);

		ItemInstance(const ItemInstance& copy);

		~ItemInstance();

		// Query item type
		bool IsType(item::ItemClass item_class) const;

		bool IsClassCommon() const;
		bool IsClassBag() const;
		bool IsClassBook() const;

		// Can item be stacked?
		bool IsStackable() const;
		bool IsCharged() const;

		// Can item be equipped by/at?
		bool IsEquipable(uint16 race, uint16 class_) const;
		bool IsClassEquipable(uint16 class_) const;
		bool IsRaceEquipable(uint16 race) const;
		bool IsEquipable(int16 slot_id) const;

		//
		// Augments
		//
		bool IsAugmentable() const;
		bool AvailableWearSlot(uint32 aug_wear_slots) const;
		int8 AvailableAugmentSlot(int32 augment_type) const;
		bool IsAugmentSlotAvailable(int32 augment_type, uint8 slot) const;
		inline int GetAugmentType() const { return m_item ? m_item->AugType : 0; }
		inline uint32 GetAugmentRestriction() const { return m_item ? m_item->AugRestrict : 0; }

		inline bool IsExpendable() const { return ((m_item) ? ((m_item->Click.Type == item::ItemEffectExpendable) || (m_item->ItemType == item::ItemTypePotion)) : false); }

		//
		// Contents
		//
		ItemInstance* GetItem(uint8 slot) const;
		uint32 GetItemID(uint8 slot) const;
		inline const ItemInstance* operator[](uint8 slot) const { return GetItem(slot); }
		void PutItem(uint8 slot, const ItemInstance& inst);
		void PutItem(SharedDatabase *db, uint8 slot, uint32 item_id) { return; } // not defined anywhere...
		void DeleteItem(uint8 slot);
		ItemInstance* PopItem(uint8 index);
		void Clear();
		void ClearByFlags(byFlagSetting is_nodrop, byFlagSetting is_norent);
		uint8 FirstOpenSlot() const;
		uint8 GetTotalItemCount() const;
		bool IsNoneEmptyContainer();
		std::map<uint8, ItemInstance*>* GetContents() { return &m_contents; }

		//
		// Augments
		//
		ItemInstance* GetAugment(uint8 augment_index) const;
		uint32 GetAugmentItemID(uint8 augment_index) const;
		void PutAugment(uint8 slot, const ItemInstance& inst);
		void PutAugment(SharedDatabase *db, uint8 slot, uint32 item_id);
		void DeleteAugment(uint8 slot);
		ItemInstance* RemoveAugment(uint8 index);
		bool IsAugmented();
		bool ContainsAugmentByID(uint32 item_id);
		int CountAugmentByID(uint32 item_id);
		bool IsOrnamentationAugment(EQ::ItemInstance* augment) const;
		ItemInstance* GetOrnamentationAugment() const;
		bool UpdateOrnamentationInfo();
		static bool CanTransform(const ItemData *ItemToTry, const ItemData *Container, bool AllowAll = false);

		// Has attack/delay?
		bool IsWeapon() const;
		bool IsAmmo() const;

		// Accessors
		const uint32 GetID() const { return ((m_item) ? m_item->ID : 0); }
		const uint32 GetItemScriptID() const { return ((m_item) ? m_item->ScriptFileID : 0); }
		const ItemData* GetItem() const;
		const ItemData* GetUnscaledItem() const;

		const uint8 GetItemType() const { return m_item ? m_item->ItemType : 255; } // Return 255 so you know there's no valid item

		int16 GetCharges() const				{ return m_charges; }
		void SetCharges(int16 charges)			{ m_charges = charges; }

		uint32 GetPrice() const					{ return m_price; }
		void SetPrice(uint32 price)				{ m_price = price; }

		void SetColor(uint32 color)				{ m_color = color; }
		uint32 GetColor() const					{ return m_color; }

		uint32 GetMerchantSlot() const			{ return m_merchantslot; }
		void SetMerchantSlot(uint32 slot)		{ m_merchantslot = slot; }

		int32 GetMerchantCount() const			{ return m_merchantcount; }
		void SetMerchantCount(int32 count)		{ m_merchantcount = count; }

		int16 GetCurrentSlot() const			{ return m_currentslot; }
		void SetCurrentSlot(int16 curr_slot)	{ m_currentslot = curr_slot; }

		// Is this item already attuned?
		bool IsAttuned() const					{ return m_attuned; }
		void SetAttuned(bool flag)				{ m_attuned = flag; }

		std::string GetCustomDataString() const;
		std::string GetCustomData(const std::string &identifier);
		void SetCustomDataString(const std::string& str);
		void SetCustomData(const std::string &identifier, const std::string& value);
		void SetCustomData(const std::string &identifier, int value);
		void SetCustomData(const std::string &identifier, float value);
		void SetCustomData(const std::string &identifier, bool value);
		void DeleteCustomData(const std::string& identifier);

		// Allows treatment of this object as though it were a pointer to m_item
		operator bool() const { return (m_item != nullptr); }

		// Compare inner Item_Struct of two ItemInstance objects
		bool operator==(const ItemInstance& right) const { return (this->m_item == right.m_item); }
		bool operator!=(const ItemInstance& right) const { return (this->m_item != right.m_item); }

		// Clone current item
		ItemInstance* Clone() const;

		bool IsSlotAllowed(int16 slot_id) const;

		bool IsDroppable(bool recurse = true) const;

		bool IsScaling() const				{ return m_scaling; }
		uint32 GetExp() const				{ return m_exp; }
		void SetExp(uint32 exp)				{ m_exp = exp; }
		void AddExp(uint32 exp)				{ m_exp += exp; }
		void SetScaling(bool v)				{ m_scaling = v; }
		uint32 GetOrnamentationIcon() const							{ return m_ornamenticon; }
		void SetOrnamentIcon(uint32 ornament_icon)					{ m_ornamenticon = ornament_icon; }
		uint32 GetOrnamentationIDFile() const						{ return m_ornamentidfile; }
		void SetOrnamentationIDFile(uint32 ornament_idfile)			{ m_ornamentidfile = ornament_idfile; }
		uint32 GetNewIDFile() const						            { return m_new_id_file; }
		void SetNewIDFile(uint32 new_id_file)			            { m_new_id_file = new_id_file; }
		uint32 GetOrnamentHeroModel(int32 material_slot = -1) const;
		void SetOrnamentHeroModel(uint32 ornament_hero_model)		{ m_ornament_hero_model = ornament_hero_model; }
		uint32 GetRecastTimestamp() const							{ return m_recast_timestamp; }
		void SetRecastTimestamp(uint32 in)							{ m_recast_timestamp = in; }

		void Initialize(SharedDatabase *db = nullptr);
		void ScaleItem();

		std::string Serialize(int16 slot_id) const { InternalSerializedItem_Struct s; s.slot_id = slot_id; s.inst = (const void*)this; std::string ser; ser.assign((char*)&s, sizeof(InternalSerializedItem_Struct)); return ser; }
		void Serialize(OutBuffer& ob, int16 slot_id) const { InternalSerializedItem_Struct isi; isi.slot_id = slot_id; isi.inst = (const void*)this; ob.write((const char*)&isi, sizeof(isi)); }

		inline int32 GetSerialNumber() const { return m_SerialNumber; }
		inline void SetSerialNumber(int32 id) { m_SerialNumber = id; }

		std::map<std::string, ::Timer>& GetTimers() const { return m_timers; }
		void SetTimer(std::string name, uint32 time);
		void SetTimer(std::string name, uint32 time) const;
		void StopTimer(std::string name);
		void ClearTimers();

		int GetTaskDeliveredCount() const { return m_task_delivered_count; }
		void SetTaskDeliveredCount(int count) { m_task_delivered_count = count; }
		// This function should only be used by trade return apis
		// Removes delivered task items from stack count and returns remaining count
		// Return value should be used to determine if an item still exists (for stackable and non-stackable)
		int RemoveTaskDeliveredItems();

		// Get a total of a stat, including augs
		// These functions should be used in place of other code manually totaling
		// to centralize where it is done to make future changes easier (ex. whenever powersources come around)
		// and to minimize errors. CalcItemBonuses however doesn't use these in interest of performance
		// by default these do not recurse into augs
		int GetItemArmorClass(bool augments = false) const;
		int GetItemElementalDamage(int &magic, int &fire, int &cold, int &poison, int &disease, int &chromatic, int &prismatic, int &physical, int &corruption, bool augments = false) const;
		// These two differ in the fact that they're quick checks (they are checked BEFORE the one above
		int GetItemElementalFlag(bool augments = false) const;
		int GetItemElementalDamage(bool augments = false) const;
		int GetItemRecommendedLevel(bool augments = false) const;
		int GetItemRequiredLevel(bool augments = false) const;
		int GetItemWeaponDamage(bool augments = false) const;
		int GetItemBackstabDamage(bool augments = false) const;
		// these two are just quick checks
		int GetItemBaneDamageBody(bool augments = false) const;
		int GetItemBaneDamageRace(bool augments = false) const;
		int GetItemBaneDamageBody(uint8 against, bool augments = false) const;
		int GetItemBaneDamageRace(uint16 against, bool augments = false) const;
		int GetItemMagical(bool augments = false) const;
		int GetItemHP(bool augments = false) const;
		int GetItemMana(bool augments = false) const;
		int GetItemManaRegen(bool augments = false) const;
		int GetItemEndur(bool augments = false) const;
		int GetItemAttack(bool augments = false) const;
		int GetItemStr(bool augments = false) const;
		int GetItemSta(bool augments = false) const;
		int GetItemDex(bool augments = false) const;
		int GetItemAgi(bool augments = false) const;
		int GetItemInt(bool augments = false) const;
		int GetItemWis(bool augments = false) const;
		int GetItemCha(bool augments = false) const;
		int GetItemMR(bool augments = false) const;
		int GetItemFR(bool augments = false) const;
		int GetItemCR(bool augments = false) const;
		int GetItemPR(bool augments = false) const;
		int GetItemDR(bool augments = false) const;
		int GetItemCorrup(bool augments = false) const;
		int GetItemHeroicStr(bool augments = false) const;
		int GetItemHeroicSta(bool augments = false) const;
		int GetItemHeroicDex(bool augments = false) const;
		int GetItemHeroicAgi(bool augments = false) const;
		int GetItemHeroicInt(bool augments = false) const;
		int GetItemHeroicWis(bool augments = false) const;
		int GetItemHeroicCha(bool augments = false) const;
		int GetItemHeroicMR(bool augments = false) const;
		int GetItemHeroicFR(bool augments = false) const;
		int GetItemHeroicCR(bool augments = false) const;
		int GetItemHeroicPR(bool augments = false) const;
		int GetItemHeroicDR(bool augments = false) const;
		int GetItemHeroicCorrup(bool augments = false) const;
		int GetItemHaste(bool augments = false) const;
		int GetItemRegen(bool augments = false) const;
		int GetItemDamageShield(bool augments = false) const;
		int GetItemDSMitigation(bool augments = false) const;
		int GetItemHealAmt(bool augments = false) const;
		int GetItemSpellDamage(bool augments = false) const;
		int GetItemClairvoyance(bool augments = false) const;
		int GetItemSkillsStat(EQ::skills::SkillType skill, bool augments = false) const;
		uint32 GetItemGuildFavor() const;
		std::vector<uint32> GetAugmentIDs() const;
		std::vector<std::string> GetAugmentNames() const;
		static void AddGUIDToMap(uint64 existing_serial_number);
		static void ClearGUIDMap();

		// evolving items stuff
		CharacterEvolvingItemsRepository::CharacterEvolvingItems &GetEvolvingDetails() const { return m_evolving_details; }

		int8             GetEvolveLvl() const { if (GetItem()) { return GetItem()->EvolvingLevel; } return false; }
		bool             IsEvolving() const { if (GetItem()) { return GetItem()->EvolvingItem; } return false; }
		int8             GetMaxEvolveLvl() const { if (GetItem()) { return GetItem()->EvolvingMax; } return false; }
		bool             GetEvolveActivated() const { return m_evolving_details.activated ? true : false; }
		bool             GetEvolveEquipped() const { return m_evolving_details.equipped ? true : false; }
		double           GetEvolveProgression() const { return m_evolving_details.progression; }
		uint64           GetEvolveUniqueID() const { return m_evolving_details.id; }
		uint32           GetEvolveCharID() const { return m_evolving_details.character_id; }
		uint32           GetEvolveItemID() const { return m_evolving_details.item_id; }
		uint32           GetEvolveLoreID() const { if (GetItem()) { return GetItem()->EvolvingID; } return false; }
		uint64           GetEvolveCurrentAmount() const { return m_evolving_details.current_amount; }
		uint32           GetEvolveFinalItemID() const { return m_evolving_details.final_item_id; }
		uint32           GetAugmentEvolveUniqueID(uint8 augment_index) const;
		void             SetEvolveEquipped(const bool in) const;
		void             SetEvolveActivated(const bool in) const { m_evolving_details.activated = in; }
		void             SetEvolveProgression(const double in) const { m_evolving_details.progression = in; }
		void             SetEvolveUniqueID(const uint64 in) const { m_evolving_details.id = in; }
		void             SetEvolveCharID(const uint32 in) const { m_evolving_details.character_id = in; }
		void             SetEvolveItemID(const uint32 in) const { m_evolving_details.item_id = in; }
		void             SetEvolveCurrentAmount(const uint64 in) const { m_evolving_details.current_amount = in; }
		void             SetEvolveAddToCurrentAmount(const uint64 in) const { m_evolving_details.current_amount += in; }
		void             SetEvolveFinalItemID(const uint32 in) const { m_evolving_details.final_item_id = in; }
		bool             TransferOwnership(Database& db, const uint32 to_char_id) const;
		void             CalculateEvolveProgression() const { m_evolving_details.progression = EvolvingItemsManager::Instance()->CalculateProgression(GetEvolveCurrentAmount(), GetID()); }

	protected:
		//////////////////////////
		// Protected Members
		//////////////////////////
		friend class InventoryProfile;

		std::map<uint8, ItemInstance*>::const_iterator _cbegin() { return m_contents.cbegin(); }
		std::map<uint8, ItemInstance*>::const_iterator _cend() { return m_contents.cend(); }

		void _PutItem(uint8 index, ItemInstance* inst) { m_contents[index] = inst; }

		ItemInstTypes    m_use_type{ItemInstNormal};// Usage type for item
		const ItemData * m_item{nullptr};           // Ptr to item data
		int16            m_charges{0};              // # of charges for chargeable items
		uint32           m_price{0};                // Bazaar /trader price
		uint32           m_color{0};
		uint32           m_merchantslot{0};
		int16            m_currentslot{0};
		bool             m_attuned{false};
		int32            m_merchantcount{1};//number avaliable on the merchant, -1=unlimited
		int32            m_SerialNumber{0}; // Unique identifier for this instance of an item. Needed for Bazaar.
		uint32           m_exp{0};
		int8             m_evolveLvl{0};
		ItemData *       m_scaledItem{nullptr};
		bool             m_scaling{false};
		uint32           m_ornamenticon{0};
		uint32           m_ornamentidfile{0};
		uint32           m_new_id_file{0};
		uint32           m_ornament_hero_model{0};
		uint32           m_recast_timestamp{0};
		int              m_task_delivered_count{0};
		mutable CharacterEvolvingItemsRepository::CharacterEvolvingItems  m_evolving_details{};

		// Items inside of this item (augs or contents) {};
		std::map<uint8, ItemInstance*>         m_contents {}; // Zero-based index: min=0, max=9
		std::map<std::string, std::string>     m_custom_data {};
		mutable std::map<std::string, ::Timer> m_timers {};
	};
}
