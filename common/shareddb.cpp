/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.org)

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

#include "shareddb.h"

#include "common/classes.h"
#include "common/data_verification.h"
#include "common/eq_packet_structs.h"
#include "common/eqemu_config.h"
#include "common/evolving_items.h"
#include "common/faction.h"
#include "common/inventory_profile.h"
#include "common/ipc_mutex.h"
#include "common/memory_mapped_file.h"
#include "common/path_manager.h"
#include "common/repositories/account_repository.h"
#include "common/repositories/books_repository.h"
#include "common/repositories/character_corpses_repository.h"
#include "common/repositories/character_inspect_messages_repository.h"
#include "common/repositories/character_item_recast_repository.h"
#include "common/repositories/criteria/content_filter_criteria.h"
#include "common/repositories/damageshieldtypes_repository.h"
#include "common/repositories/inventory_repository.h"
#include "common/repositories/items_repository.h"
#include "common/repositories/sharedbank_repository.h"
#include "common/repositories/starting_items_repository.h"
#include "common/rulesys.h"
#include "common/strings.h"
#include "zone/client.h"

#include "fmt/format.h"
#include <algorithm>

SharedDatabase::SharedDatabase()
: Database()
{
}

SharedDatabase::SharedDatabase(const char* host, const char* user, const char* passwd, const char* database, uint32 port)
: Database(host, user, passwd, database, port)
{
}

SharedDatabase::~SharedDatabase() = default;

bool SharedDatabase::SetHideMe(uint32 account_id, uint8 hideme)
{
	auto a = AccountRepository::FindOne(*this, account_id);
	if (a.id > 0) {
		a.hideme = hideme ? 1 : 0;
		AccountRepository::UpdateOne(*this, a);
	}

	return a.id > 0;
}

uint8 SharedDatabase::GetGMSpeed(uint32 account_id)
{
	auto a = AccountRepository::FindOne(*this, account_id);
	if (a.id > 0) {
		return a.gmspeed;
	}

	return 0;
}

bool SharedDatabase::SetGMSpeed(uint32 account_id, uint8 gmspeed)
{
	auto a = AccountRepository::FindOne(*this, account_id);
	if (a.id > 0) {
		a.gmspeed = gmspeed ? 1 : 0;
		AccountRepository::UpdateOne(*this, a);
	}

	return a.id > 0;
}

bool SharedDatabase::SetGMInvul(uint32 account_id, bool gminvul)
{
	auto a = AccountRepository::FindOne(*this, account_id);
	if (a.id > 0) {
		a.invulnerable = gminvul ? 1 : 0;
		AccountRepository::UpdateOne(*this, a);
	}

	return a.id > 0;
}

bool SharedDatabase::SetGMFlymode(uint32 account_id, uint8 flymode)
{
	auto a = AccountRepository::FindOne(*this, account_id);
	if (a.id > 0) {
		a.flymode = flymode;
		AccountRepository::UpdateOne(*this, a);
	}

	return a.id > 0;
}

void SharedDatabase::SetMailKey(uint32 character_id, uint32 ip_address, uint32 mail_key)
{
	std::string full_mail_key;

	if (RuleB(Chat, EnableMailKeyIPVerification)) {
		full_mail_key = fmt::format("{:08X}{:08X}", ip_address, mail_key);
	} else {
		full_mail_key = fmt::format("{:08X}", mail_key);
	}

	auto e = CharacterDataRepository::FindOne(*this, character_id);
	if (!e.id) {
		LogError("Failed to find character_id [{}] when setting mailkey", character_id);
		return;
	}

	e.mailkey = full_mail_key;

	if (!CharacterDataRepository::UpdateOne(*this, e)) {
		LogError("Failed to set mailkey to [{}] for character_id [{}]", full_mail_key, character_id);
	}
}

SharedDatabase::MailKeys SharedDatabase::GetMailKey(uint32 character_id)
{
	auto e = CharacterDataRepository::FindOne(*this, character_id);

	if (!e.id) {
		return MailKeys{ };
	}

	return MailKeys{
		.mail_key = e.mailkey.substr(8),
		.mail_key_full = e.mailkey
	};
}

bool SharedDatabase::SaveCursor(
	uint32 char_id,
	std::list<EQ::ItemInstance*>::const_iterator& start,
	std::list<EQ::ItemInstance*>::const_iterator& end
)
{
	const int deleted = InventoryRepository::DeleteWhere(
		*this,
		fmt::format(
			"`character_id` = {} AND (`slot_id` = {} OR `slot_id` BETWEEN {} AND {})",
			char_id,
			EQ::invslot::slotCursor,
			EQ::invbag::CURSOR_BAG_BEGIN,
			EQ::invbag::CURSOR_BAG_END
		)
	);

	int16 i = EQ::invbag::CURSOR_BAG_BEGIN;
	for (auto& it = start; it != end; ++it, i++) {
		// shouldn't be anything in the queue that indexes this high
		if (i > EQ::invbag::CURSOR_BAG_END) {
			break;
		}

		const EQ::ItemInstance* inst = *it;
		const int16 use_slot = i == EQ::invbag::CURSOR_BAG_BEGIN ? EQ::invslot::slotCursor : i;
		if (!SaveInventory(char_id, inst, use_slot)) {
			return false;
		}
	}

	return true;
}

bool SharedDatabase::VerifyInventory(uint32 account_id, int16 slot_id, const EQ::ItemInstance* inst)
{
	if (!inst || !inst->GetItem()) {
		return false;
	}

	const auto& l = SharedbankRepository::GetWhere(
		*this,
		fmt::format(
			"`account_id` = {} AND `slot_id` = {} LIMIT 1",
			account_id,
			slot_id
		)
	);

	if (l.empty()) {
		return false;
	}

	const auto& e = l.front();

	uint16 expect_charges = inst->GetCharges() >= 0 ? inst->GetCharges() : std::numeric_limits<int16>::max();

	return e.item_id == inst->GetID() && e.charges == expect_charges;
}

bool SharedDatabase::SaveInventory(uint32 char_id, const EQ::ItemInstance* inst, int16 slot_id)
{
	// Don't save any Tribute slots
	if (
		EQ::ValueWithin(slot_id, EQ::invslot::GUILD_TRIBUTE_BEGIN, EQ::invslot::GUILD_TRIBUTE_END) ||
		EQ::ValueWithin(slot_id, EQ::invslot::TRIBUTE_BEGIN, EQ::invslot::TRIBUTE_END)
	) {
		return true;
	}

	if (
		EQ::ValueWithin(slot_id, EQ::invslot::SHARED_BANK_BEGIN, EQ::invslot::SHARED_BANK_END) ||
		EQ::ValueWithin(slot_id, EQ::invbag::SHARED_BANK_BAGS_BEGIN, EQ::invbag::SHARED_BANK_BAGS_END)
	) {
		if (!inst) {
			return DeleteSharedBankSlot(char_id, slot_id);
		} else {
			// Needed to clear out bag slots that 'REPLACE' in UpdateSharedBankSlot does not overwrite..otherwise, duplication occurs
			// (This requires that parent then child items be sent..which should be how they are currently passed)
			if (EQ::InventoryProfile::SupportsContainers(slot_id)) {
				DeleteSharedBankSlot(char_id, slot_id);
			}

			return UpdateSharedBankSlot(char_id, inst, slot_id);
		}
	} else if (!inst) { // All other inventory
		return DeleteInventorySlot(char_id, slot_id);
	}

	// Needed to clear out bag slots that 'REPLACE' in UpdateInventorySlot does not overwrite..otherwise, duplication occurs
	// (This requires that parent then child items be sent..which should be how they are currently passed)
	if (EQ::InventoryProfile::SupportsContainers(slot_id)) {
		DeleteInventorySlot(char_id, slot_id);
	}

	return UpdateInventorySlot(char_id, inst, slot_id);
}

bool SharedDatabase::UpdateInventorySlot(uint32 char_id, const EQ::ItemInstance* inst, int16 slot_id)
{
	if (!inst || !inst->GetItem()) {
		return false;
	}

	std::vector<uint32> augment_ids = inst->GetAugmentIDs();

	uint16 charges = inst->GetCharges() >= 0 ? inst->GetCharges() : std::numeric_limits<int16>::max();

	auto e = InventoryRepository::NewEntity();

	e.character_id        = char_id;
	e.slot_id             = slot_id;
	e.item_id             = inst->GetID();
	e.charges             = charges;
	e.color               = inst->GetColor();
	e.augment_one         = augment_ids[0];
	e.augment_two         = augment_ids[1];
	e.augment_three       = augment_ids[2];
	e.augment_four        = augment_ids[3];
	e.augment_five        = augment_ids[4];
	e.augment_six         = augment_ids[5];
	e.instnodrop          = inst->IsAttuned() ? 1 : 0;
	e.custom_data         = inst->GetCustomDataString();
	e.ornament_icon       = inst->GetOrnamentationIcon();
	e.ornament_idfile     = inst->GetOrnamentationIDFile();
	e.ornament_hero_model = inst->GetOrnamentHeroModel();
	e.guid                = inst->GetSerialNumber();

	const int replaced = InventoryRepository::ReplaceOne(*this, e);

	// Save bag contents, if slot supports bag contents
	if (inst->IsClassBag() && EQ::InventoryProfile::SupportsContainers(slot_id)) {
		// Limiting to bag slot count will get rid of 'hidden' duplicated items and 'Invalid Slot ID'
		// messages through attrition (and the modded code in SaveInventory)
		for (
			uint8 i = EQ::invbag::SLOT_BEGIN;
			i < inst->GetItem()->BagSlots && i <= EQ::invbag::SLOT_END;
			i++
		) {
			const EQ::ItemInstance* bag_inst = inst->GetItem(i);
			SaveInventory(char_id, bag_inst, EQ::InventoryProfile::CalcSlotId(slot_id, i));
		}
	}

	return replaced;
}

bool SharedDatabase::UpdateSharedBankSlot(uint32 char_id, const EQ::ItemInstance* inst, int16 slot_id)
{
	if (!inst || !inst->GetItem()) {
		return false;
	}

	std::vector<uint32> augment_ids = inst->GetAugmentIDs();

	uint16 charges = inst->GetCharges() >= 0 ? inst->GetCharges() : std::numeric_limits<int16>::max();

	const uint32 account_id = GetAccountIDByChar(char_id);

	auto e = SharedbankRepository::NewEntity();

	e.account_id          = account_id;
	e.slot_id             = slot_id;
	e.item_id             = inst->GetID();
	e.charges             = charges;
	e.color               = inst->GetColor();
	e.augment_one         = augment_ids[0];
	e.augment_two         = augment_ids[1];
	e.augment_three       = augment_ids[2];
	e.augment_four        = augment_ids[3];
	e.augment_five        = augment_ids[4];
	e.augment_six         = augment_ids[5];
	e.custom_data         = inst->GetCustomDataString();
	e.ornament_icon       = inst->GetOrnamentationIcon();
	e.ornament_idfile     = inst->GetOrnamentationIDFile();
	e.ornament_hero_model = inst->GetOrnamentHeroModel();
	e.guid                = inst->GetSerialNumber();

	const int replaced = SharedbankRepository::ReplaceOne(*this, e);

	// Save bag contents, if slot supports bag contents
	if (inst->IsClassBag() && EQ::InventoryProfile::SupportsContainers(slot_id)) {
		// Limiting to bag slot count will get rid of 'hidden' duplicated items and 'Invalid Slot ID'
		// messages through attrition (and the modded code in SaveInventory)
		for (
			uint8 i = EQ::invbag::SLOT_BEGIN;
			i < inst->GetItem()->BagSlots && i <= EQ::invbag::SLOT_END;
			i++
		) {
			const EQ::ItemInstance* bag_inst = inst->GetItem(i);
			SaveInventory(char_id, bag_inst, EQ::InventoryProfile::CalcSlotId(slot_id, i));
		}
	}

	return replaced;
}

bool SharedDatabase::DeleteInventorySlot(uint32 char_id, int16 slot_id)
{
	const int deleted = InventoryRepository::DeleteWhere(
		*this,
		fmt::format(
			"`character_id` = {} AND `slot_id` = {}",
			char_id,
			slot_id
		)
	);

	if (!deleted) {
		return false;
	}

	// Delete bag slots, if need be
	if (!EQ::InventoryProfile::SupportsContainers(slot_id)) {
		return true;
	}

	const int16 base_slot_id = EQ::InventoryProfile::CalcSlotId(slot_id, EQ::invbag::SLOT_BEGIN);

	return InventoryRepository::DeleteWhere(
		*this,
		fmt::format(
			"`character_id` = {} AND `slot_id` BETWEEN {} AND {}",
			char_id,
			base_slot_id,
			base_slot_id + (EQ::invbag::SLOT_COUNT - 1)
		)
	);
}

bool SharedDatabase::DeleteSharedBankSlot(uint32 char_id, int16 slot_id)
{
	const uint32 account_id = GetAccountIDByChar(char_id);

	const int deleted = SharedbankRepository::DeleteWhere(
		*this,
		fmt::format(
			"`account_id` = {} AND `slot_id` = {}",
			account_id,
			slot_id
		)
	);

	if (!deleted) {
		return false;
	}

	if (!EQ::InventoryProfile::SupportsContainers(slot_id)) {
		return true;
	}

	const int16 base_slot_id = EQ::InventoryProfile::CalcSlotId(slot_id, EQ::invbag::SLOT_BEGIN);

	return SharedbankRepository::DeleteWhere(
		*this,
		fmt::format(
			"`account_id` = {} AND `slot_id` BETWEEN {} AND {}",
			account_id,
			base_slot_id,
			base_slot_id + (EQ::invbag::SLOT_COUNT - 1)
		)
	);
}

int32 SharedDatabase::GetSharedPlatinum(uint32 account_id)
{
	const auto& e = AccountRepository::FindOne(*this, account_id);

	return e.sharedplat;
}

bool SharedDatabase::AddSharedPlatinum(uint32 account_id, int amount)
{
	auto e = AccountRepository::FindOne(*this, account_id);

	e.sharedplat += amount;

	return AccountRepository::UpdateOne(*this, e);
}

bool SharedDatabase::SetStartingItems(
	PlayerProfile_Struct *pp,
	EQ::InventoryProfile *inv,
	uint32 si_race,
	uint32 si_class,
	uint32 si_deity,
	uint32 si_current_zone,
	char *si_name,
	int admin_level
)
{
	const EQ::ItemData *item_data;

	const auto& l = StartingItemsRepository::GetWhere(
		*this,
		fmt::format(
			"TRUE {}",
			ContentFilterCriteria::apply()
		)
	);

	if (l.empty()) {
		return false;
	}

	std::vector<StartingItemsRepository::StartingItems> v;

	for (const auto &e : l) {
		const auto &classes = Strings::Split(e.class_list, "|");
		const auto &deities = Strings::Split(e.deity_list, "|");
		const auto &races   = Strings::Split(e.race_list, "|");
		const auto &zones   = Strings::Split(e.zone_id_list, "|");

		const std::string &all = std::to_string(0);

		if (classes[0] != all) {
			if (!Strings::Contains(classes, std::to_string(si_class))) {
				continue;
			}
		}

		if (deities[0] != all) {
			if (!Strings::Contains(deities, std::to_string(si_deity))) {
				continue;
			}
		}

		if (races[0] != all) {
			if (!Strings::Contains(races, std::to_string(si_race))) {
				continue;
			}
		}

		if (zones[0] != all) {
			if (!Strings::Contains(zones, std::to_string(si_current_zone))) {
				continue;
			}
		}

		v.emplace_back(e);
	}

	for (const auto &e : v) {
		const uint32 item_id      = e.item_id;
		const uint8  item_charges = e.item_charges;
		int32        slot         = e.inventory_slot;

		item_data = GetItem(item_id);

		if (!item_data) {
			continue;
		}

		const auto* inst = CreateItem(
			item_data,
			item_charges,
			e.augment_one,
			e.augment_two,
			e.augment_three,
			e.augment_four,
			e.augment_five,
			e.augment_six
		);

		if (slot < EQ::invslot::slotCharm) {
			slot = inv->FindFreeSlot(false, false);
		}

		inv->PutItem(slot, *inst);
		safe_delete(inst);
	}

	return true;
}


// Retrieve shared bank inventory based on either account or character
bool SharedDatabase::GetSharedBank(uint32 id, EQ::InventoryProfile *inv, bool is_charid)
{
	const uint32 account_id = is_charid ? GetAccountIDByChar(id) : id;

	if (!account_id) {
		return false;
	}

	const auto& l = SharedbankRepository::GetWhere(
		*this,
		fmt::format(
			"`account_id` = {}",
			account_id
		)
	);

	if (l.empty()) {
		return true;
	}

	for (const auto& e : l) {
		uint32 augment_ids[EQ::invaug::SOCKET_COUNT] = {
			e.augment_one,
			e.augment_two,
			e.augment_three,
			e.augment_four,
			e.augment_five,
			e.augment_six
		};

		const EQ::ItemData* item = GetItem(e.item_id);

		if (!item) {
			LogError(
				"Warning: {} [{}] has an invalid item_id [{}] in slot_id [{}]",
				is_charid ? "character_id" : "account_id",
				id,
				e.item_id,
				e.slot_id
			);
			continue;
		}

		EQ::ItemInstance* inst = CreateBaseItem(item, e.charges);
		if (!inst) {
			continue;
		}

		if (item->IsClassCommon()) {
			for (int i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
				if (augment_ids[i]) {
					inst->PutAugment(this, i, augment_ids[i]);
				}
			}
		}

		if (!e.custom_data.empty()) {
			inst->SetCustomDataString(e.custom_data);
		}

		const int16 put_slot_id = inv->PutItem(e.slot_id, *inst);
		safe_delete(inst);

		if (put_slot_id != INVALID_INDEX) {
			continue;
		}

		LogError(
			"Warning: Invalid slot_id for item in shared bank inventory for {} [{}] item_id [{}] slot_id [{}]",
			is_charid ? "character_id" : "account_id",
			id,
			e.item_id,
			e.slot_id
		);

		if (is_charid) {
			SaveInventory(id, nullptr, e.slot_id);
		}
	}

	return true;
}
// Overloaded: Retrieve character inventory based on character id (zone entry)
//bool SharedDatabase::GetInventory(uint32 char_id, EQ::InventoryProfile *inv)
bool SharedDatabase::GetInventory(Client *c)
{
	if (!c) {
		return false;
	}

	uint32                char_id = c->CharacterID();
	EQ::InventoryProfile &inv     = c->GetInv();

	// Retrieve character inventory
	auto results   = InventoryRepository::GetWhere(*this, fmt::format("`character_id` = '{}' ORDER BY `slot_id`", char_id));
	auto e_results = CharacterEvolvingItemsRepository::GetWhere(
		*this, fmt::format("`character_id` = '{}' AND `deleted_at` IS NULL", char_id)
	);

	if (results.empty()) {
		LogError("Error loading inventory for char_id {} from the database.", char_id);
		return false;
	}

	for (auto const& row: results) {
		if (row.guid != 0) {
			EQ::ItemInstance::AddGUIDToMap(row.guid);
		}
	}

	const auto timestamps  = GetItemRecastTimestamps(char_id);
	auto       cv_conflict = false;
	const auto pmask       = inv.GetLookup()->PossessionsBitmask;
	const auto bank_size   = inv.GetLookup()->InventoryTypeSize.Bank;

	std::vector<InventoryRepository::Inventory> queue{ };
	for (auto& row: results) {
		const int16  slot_id             = row.slot_id;
		const uint32 item_id             = row.item_id;
		const uint16 charges             = row.charges;
		const uint32 color               = row.color;
		const bool   instnodrop          = row.instnodrop;
		const uint32 ornament_icon       = row.ornament_icon;
		const uint32 ornament_idfile     = row.ornament_idfile;
		const uint32 ornament_hero_model = row.ornament_hero_model;

		uint32 augment_ids[EQ::invaug::SOCKET_COUNT] = {
			row.augment_one,
			row.augment_two,
			row.augment_three,
			row.augment_four,
			row.augment_five,
			row.augment_six
		};

		if (EQ::ValueWithin(slot_id, EQ::invslot::POSSESSIONS_BEGIN, EQ::invslot::POSSESSIONS_END)) {
			// Titanium thru UF check
			if (((static_cast<uint64>(1) << slot_id) & pmask) == 0) {
				cv_conflict = true;
				continue;
			}
		} else if (EQ::ValueWithin(slot_id, EQ::invbag::GENERAL_BAGS_BEGIN, EQ::invbag::GENERAL_BAGS_END)) {
			// Titanium thru UF check
			const auto parent_slot = EQ::invslot::GENERAL_BEGIN + ((slot_id - EQ::invbag::GENERAL_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT);
			if (((static_cast<uint64>(1) << parent_slot) & pmask) == 0) {
				cv_conflict = true;
				continue;
			}
		} else if (EQ::ValueWithin(slot_id, EQ::invslot::BANK_BEGIN, EQ::invslot::BANK_END)) {
			// Titanium check
			if ((slot_id - EQ::invslot::BANK_BEGIN) >= bank_size) {
				cv_conflict = true;
				continue;
			}
		} else if (EQ::ValueWithin(slot_id, EQ::invbag::BANK_BAGS_BEGIN, EQ::invbag::BANK_BAGS_END)) {
			// Titanium check
			const auto parent_index = ((slot_id - EQ::invbag::BANK_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT);
			if (parent_index >= bank_size) {
				cv_conflict = true;
				continue;
			}
		}

		auto* item = GetItem(item_id);
		if (!item) {
			LogError(
				"Warning: charid [{}] has an invalid item_id [{}] in inventory slot [{}]",
				char_id,
				item_id,
				slot_id
			);
			continue;
		}

		auto* inst = CreateBaseItem(item, charges);
		if (!inst) {
			continue;
		}

		if (!row.custom_data.empty()) {
			inst->SetCustomDataString(row.custom_data);
		}

		inst->SetOrnamentIcon(ornament_icon);
		inst->SetOrnamentationIDFile(ornament_idfile);
		inst->SetOrnamentHeroModel(item->HerosForgeModel);

		if (
			instnodrop ||
			(
				inst->GetItem()->Attuneable &&
				EQ::ValueWithin(slot_id, EQ::invslot::EQUIPMENT_BEGIN, EQ::invslot::EQUIPMENT_END)
			)
		) {
			inst->SetAttuned(true);
		}

		if (color > 0) {
			inst->SetColor(color);
		}

		if (charges == std::numeric_limits<int16>::max()) {
			inst->SetCharges(-1);
		} else if (charges == 0 && inst->IsStackable()) {
			// Stackable items need a minimum charge of 1 remain moveable.
			inst->SetCharges(1);
		} else {
			inst->SetCharges(charges);
		}

		if (item->RecastDelay) {
			if (item->RecastType != RECAST_TYPE_UNLINKED_ITEM && timestamps.count(item->RecastType)) {
				inst->SetRecastTimestamp(timestamps.at(item->RecastType));
			} else if (item->RecastType == RECAST_TYPE_UNLINKED_ITEM && timestamps.count(item->ID)) {
				inst->SetRecastTimestamp(timestamps.at(item->ID));
			} else {
				inst->SetRecastTimestamp(0);
			}
		}

		if (item->IsClassCommon()) {
			for (int i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
				if (augment_ids[i]) {
					inst->PutAugment(this, i, augment_ids[i]);
				}
			}
		}

		if (item->EvolvingItem) {
			if (slot_id >= EQ::invslot::EQUIPMENT_BEGIN && slot_id <= EQ::invslot::EQUIPMENT_END) {
				inst->SetEvolveEquipped(true);
			}

			auto t = std::ranges::find_if(
				e_results.cbegin(),
				e_results.cend(),
				[&](const CharacterEvolvingItemsRepository::CharacterEvolvingItems &x) {
					return x.item_id == item_id;
				}
			);

			if (t == std::end(e_results)) {
				auto e = CharacterEvolvingItemsRepository::NewEntity();

				e.character_id  = char_id;
				e.item_id       = item_id;
				e.equipped      = inst->GetEvolveEquipped();
				e.final_item_id = EvolvingItemsManager::Instance()->GetFinalItemID(*inst);

				auto r = CharacterEvolvingItemsRepository::InsertOne(*this, e);
				e.id = r.id;
				e_results.push_back(e);

				inst->SetEvolveUniqueID(e.id);
				inst->SetEvolveCharID(e.character_id);
				inst->SetEvolveItemID(e.item_id);
				inst->SetEvolveActivated(e.activated);
				inst->SetEvolveEquipped(e.equipped);
				inst->SetEvolveCurrentAmount(e.current_amount);
				inst->CalculateEvolveProgression();
				inst->SetEvolveFinalItemID(e.final_item_id);
			}
			else {
				inst->SetEvolveUniqueID(t->id);
				inst->SetEvolveCharID(t->character_id);
				inst->SetEvolveItemID(t->item_id);
				inst->SetEvolveActivated(t->activated);
				inst->SetEvolveEquipped(t->equipped);
				inst->SetEvolveCurrentAmount(t->current_amount);
				inst->CalculateEvolveProgression();
				inst->SetEvolveFinalItemID(t->final_item_id);
			}
		}

		int16 put_slot_id;
		// this had  || slot_id == EQ::invslot::slotCursor ??s
		if (EQ::ValueWithin(slot_id, EQ::invbag::CURSOR_BAG_BEGIN, EQ::invbag::CURSOR_BAG_END)) {
			put_slot_id = inv.PushCursor(*inst);
		} else {
			put_slot_id = inv.PutItem(slot_id, *inst);
		}

		row.guid = inst->GetSerialNumber();
		queue.push_back(row);

		safe_delete(inst);

		// Save ptr to item in inventory
		if (put_slot_id == INVALID_INDEX) {
			LogError(
				"Warning: Invalid slot_id for item in inventory for character_id [{}] item_id [{}] slot_id [{}]",
				char_id,
				item_id,
				slot_id
			);
		}
	}

	if (cv_conflict) {
		const std::string& char_name = GetCharName(char_id);
		LogError(
			"ClientVersion/Expansion conflict during inventory load at zone entry for [{}] (charid: [{}], inver: [{}], gmi: [{}])",
			char_name,
			char_id,
			EQ::versions::MobVersionName(inv.InventoryVersion()),
			(inv.GMInventory() ? "true" : "false")
		);
	}

	if (!queue.empty()) {
		InventoryRepository::ReplaceMany(*this, queue);
	}

	EQ::ItemInstance::ClearGUIDMap();

	// Retrieve shared inventory
	return GetSharedBank(char_id, &inv, true);
}

std::map<uint32, uint32> SharedDatabase::GetItemRecastTimestamps(uint32 char_id)
{
	std::map<uint32, uint32> timers;

	const auto& l = CharacterItemRecastRepository::GetWhere(
		*this,
		fmt::format(
			"`id` = {}",
			char_id
		)
	);

	if (l.empty()) {
		return timers;
	}

	for (const auto& e : l) {
		timers[e.recast_type] = e.timestamp;
	}

	return timers;
}

uint32 SharedDatabase::GetItemRecastTimestamp(uint32 char_id, uint32 recast_type)
{
	const auto& l = CharacterItemRecastRepository::GetWhere(
		*this,
		fmt::format(
			"`id` = {} AND `recast_type` = {}",
			char_id,
			recast_type
		)
	);

	return l.empty() ? 0 : l[0].timestamp;
}

void SharedDatabase::ClearOldRecastTimestamps(uint32 char_id)
{
	CharacterItemRecastRepository::DeleteWhere(
		*this,
		fmt::format(
			"`id` = {} AND `timestamp` < UNIX_TIMESTAMP()",
			char_id
		)
	);
}

void SharedDatabase::GetItemsCount(int32& item_count, uint32& max_id)
{
	max_id     = ItemsRepository::GetMaxId(*this);
	item_count = ItemsRepository::Count(*this);
}

bool SharedDatabase::LoadItems(const std::string &prefix) {
	items_mmf.reset(nullptr);

	try {
		const auto Config = EQEmuConfig::get();
		EQ::IPCMutex mutex("items");
		mutex.Lock();
		std::string file_name = fmt::format("{}/{}{}", PathManager::Instance()->GetSharedMemoryPath(), prefix, std::string("items"));
		items_mmf = std::make_unique<EQ::MemoryMappedFile>(file_name);
		items_hash = std::make_unique<EQ::FixedMemoryHashSet<EQ::ItemData>>(static_cast<uint8*>(items_mmf->Get()), items_mmf->Size());
		mutex.Unlock();

		LogInfo("Loaded [{}] items via shared memory", Strings::Commify(m_shared_items_count));
	} catch(std::exception& ex) {
		LogError("Error Loading Items: {}", ex.what());
		return false;
	}

	return true;
}

void SharedDatabase::LoadItems(void *data, uint32 size, int32 items, uint32 max_item_id)
{
	EQ::FixedMemoryHashSet<EQ::ItemData> hash(static_cast<uint8 *>(data), size, items, max_item_id);

	std::string variable_buffer;

	bool disable_attuneable          = RuleB(Items, DisableAttuneable);
	bool disable_bard_focus_effects  = RuleB(Items, DisableBardFocusEffects);
	bool disable_lore                = RuleB(Items, DisableLore);
	bool disable_no_drop             = RuleB(Items, DisableNoDrop);
	bool disable_no_pet              = RuleB(Items, DisableNoPet);
	bool disable_no_rent             = RuleB(Items, DisableNoRent);
	bool disable_no_transfer         = RuleB(Items, DisableNoTransfer);
	bool disable_potion_belt         = RuleB(Items, DisablePotionBelt);
	bool disable_spell_focus_effects = RuleB(Items, DisableSpellFocusEffects);

	// Old Variable Code
	if (GetVariable("disablelore", variable_buffer)) {
		if (variable_buffer == "1") {
			disable_lore = true;
		}
	}

	if (GetVariable("disablenodrop", variable_buffer)) {
		if (variable_buffer == "1") {
			disable_no_drop = true;
		}
	}

	if (GetVariable("disablenorent", variable_buffer)) {
		if (variable_buffer == "1") {
			disable_no_rent = true;
		}
	}


	if (GetVariable("disablenotransfer", variable_buffer)) {
		if (variable_buffer == "1") {
			disable_no_transfer = true;
		}
	}

	EQ::ItemData item;

	const auto& l = ItemsRepository::All(*this);

	if (l.empty()) {
		return;
	}

	for (const auto& e : l) {
		memset(&item, 0, sizeof(EQ::ItemData));

		// Unique Identifier
		item.ID = e.id;

		// Minimum Status
		item.MinStatus = static_cast<uint8>(e.minstatus);

		// Name, Lore, and Comment
		strn0cpy(item.Name, e.Name.c_str(), sizeof(item.Name));
		strn0cpy(item.Lore, e.lore.c_str(), sizeof(item.Lore));
		strn0cpy(item.Comment, e.comment.c_str(), sizeof(item.Comment));

		// Flags
		item.ArtifactFlag    = e.artifactflag;
		item.Attuneable      = !disable_attuneable && e.attuneable;
		item.BenefitFlag     = e.benefitflag;
		item.FVNoDrop        = e.fvnodrop;
		item.Magic           = e.magic;
		item.NoDrop          = disable_no_drop ? std::numeric_limits<uint8>::max() : e.nodrop;
		item.NoPet           = !disable_no_pet && e.nopet;
		item.NoRent          = disable_no_rent ? std::numeric_limits<uint8>::max() : e.norent;
		item.NoTransfer      = !disable_no_transfer && e.notransfer;
		item.PendingLoreFlag = e.pendingloreflag;
		item.QuestItemFlag   = e.questitemflag;
		item.Stackable       = e.stackable;
		item.Tradeskills     = e.tradeskills;
		item.SummonedFlag    = e.summonedflag;

		// Lore
		item.LoreGroup = disable_lore ? 0 : e.loregroup;
		item.LoreFlag  = !disable_lore && item.LoreGroup != 0;

		// Type
		item.AugType  = e.augtype;
		item.ItemType = static_cast<uint8>(e.itemtype);
		item.SubType  = e.subtype;

		// Miscellaneous
		item.ExpendableArrow = e.expendablearrow;
		item.Light           = EQ::Clamp(e.light, -128, 127);
		item.MaxCharges      = e.maxcharges;
		item.Size            = static_cast<uint8>(e.size);
		item.StackSize       = e.stacksize;
		item.Weight          = e.weight;

		// Potion Belt
		item.PotionBelt      = !disable_potion_belt && e.potionbelt;
		item.PotionBeltSlots = disable_potion_belt ? 0 : static_cast<uint8>(e.potionbeltslots);

		// Merchant
		item.Favor      = e.favor;
		item.GuildFavor = e.guildfavor;
		item.Price      = e.price;
		item.SellRate   = e.sellrate;

		// Display
		item.Color           = e.color;
		item.EliteMaterial   = e.elitematerial;
		item.HerosForgeModel = e.herosforgemodel;
		item.Icon            = e.icon;
		strn0cpy(item.IDFile, e.idfile.c_str(), sizeof(item.IDFile));
		item.Material = e.material;

		// Resists
		item.CR           = EQ::Clamp(e.cr, -128, 127);
		item.DR           = EQ::Clamp(e.dr, -128, 127);
		item.FR           = EQ::Clamp(e.fr, -128, 127);
		item.MR           = EQ::Clamp(e.mr, -128, 127);
		item.PR           = EQ::Clamp(e.pr, -128, 127);
		item.SVCorruption = EQ::Clamp(e.svcorruption, -128, 127);

		// Heroic Resists
		item.HeroicCR       = e.heroic_cr;
		item.HeroicDR       = e.heroic_dr;
		item.HeroicFR       = e.heroic_fr;
		item.HeroicMR       = e.heroic_mr;
		item.HeroicPR       = e.heroic_pr;
		item.HeroicSVCorrup = e.heroic_svcorrup;

		// Stats
		item.AAgi = EQ::Clamp(e.aagi, -128, 127);
		item.ACha = EQ::Clamp(e.acha, -128, 127);
		item.ADex = EQ::Clamp(e.adex, -128, 127);
		item.AInt = EQ::Clamp(e.aint, -128, 127);
		item.ASta = EQ::Clamp(e.asta, -128, 127);
		item.AStr = EQ::Clamp(e.astr, -128, 127);
		item.AWis = EQ::Clamp(e.awis, -128, 127);

		// Heroic Stats
		item.HeroicAgi = e.heroic_agi;
		item.HeroicCha = e.heroic_cha;
		item.HeroicDex = e.heroic_dex;
		item.HeroicInt = e.heroic_int;
		item.HeroicSta = e.heroic_sta;
		item.HeroicStr = e.heroic_str;
		item.HeroicWis = e.heroic_wis;

		// Health, Mana, and Endurance
		item.HP             = e.hp;
		item.Regen          = e.regen;
		item.Mana           = e.mana;
		item.ManaRegen      = e.manaregen;
		item.Endur          = e.endur;
		item.EnduranceRegen = e.enduranceregen;

		// Bane Damage
		item.BaneDmgAmt     = e.banedmgamt;
		item.BaneDmgBody    = e.banedmgbody;
		item.BaneDmgRace    = e.banedmgrace;
		item.BaneDmgRaceAmt = e.banedmgraceamt;

		// Elemental Damage
		item.ElemDmgType = static_cast<uint8>(e.elemdmgtype);
		item.ElemDmgAmt  = static_cast<uint8>(e.elemdmgamt);

		// Combat
		item.BackstabDmg = e.backstabdmg;
		item.Damage      = e.damage;
		item.Delay       = static_cast<uint8>(e.delay);
		item.Range       = static_cast<uint8>(e.range_);

		// Combat Stats
		item.AC           = e.ac;
		item.Accuracy     = EQ::Clamp(e.accuracy, -128, 127);
		item.Attack       = e.attack;
		item.Avoidance    = EQ::Clamp(e.avoidance, -128, 127);
		item.Clairvoyance = e.clairvoyance;
		item.CombatEffects = Strings::IsNumber(e.combateffects) ? static_cast<int8>(EQ::Clamp(Strings::ToInt(e.combateffects), -128, 127)) : 0;
		item.DamageShield  = e.damageshield;
		item.DotShielding  = e.dotshielding;
		item.DSMitigation  = e.dsmitigation;
		item.Haste         = e.haste;
		item.HealAmt       = e.healamt;
		item.Purity        = e.purity;
		item.Shielding     = EQ::Clamp(e.shielding, -128, 127);
		item.SpellDmg      = e.spelldmg;
		item.SpellShield   = EQ::Clamp(e.spellshield, -128, 127);
		item.StrikeThrough = EQ::Clamp(e.strikethrough, -128, 127);
		item.StunResist    = EQ::Clamp(e.stunresist, -128, 127);

		// Restrictions
		item.AugRestrict = e.augrestrict;
		item.Classes     = e.classes;
		item.Deity       = e.deity;
		item.ItemClass   = static_cast<uint8>(e.itemclass);
		item.Races       = e.races;
		item.RecLevel    = static_cast<uint8>(e.reclevel);
		item.RecSkill    = static_cast<uint8>(e.recskill);
		item.ReqLevel    = static_cast<uint8>(e.reqlevel);
		item.Slots       = e.slots;

		// Skill Modifier
		item.SkillModValue = e.skillmodvalue;
		item.SkillModMax   = e.skillmodmax;
		item.SkillModType  = e.skillmodtype;

		// Extra Damage Skill
		item.ExtraDmgSkill = e.extradmgskill;
		item.ExtraDmgAmt   = e.extradmgamt;

		// Bard
		item.BardType  = e.bardtype;
		item.BardValue = e.bardvalue;

		// Faction
		item.FactionAmt1 = e.factionamt1;
		item.FactionMod1 = e.factionmod1;
		item.FactionAmt2 = e.factionamt2;
		item.FactionMod2 = e.factionmod2;
		item.FactionAmt3 = e.factionamt3;
		item.FactionMod3 = e.factionmod3;
		item.FactionAmt4 = e.factionamt4;
		item.FactionMod4 = e.factionmod4;

		// Augment Distiller
		item.AugDistiller = e.augdistiller;

		// Augment Slots
		item.AugSlotType[0]    = static_cast<uint8>(e.augslot1type);
		item.AugSlotVisible[0] = static_cast<uint8>(e.augslot1visible);
		item.AugSlotType[1]    = static_cast<uint8>(e.augslot2type);
		item.AugSlotVisible[1] = static_cast<uint8>(e.augslot2visible);
		item.AugSlotType[2]    = static_cast<uint8>(e.augslot3type);
		item.AugSlotVisible[2] = static_cast<uint8>(e.augslot3visible);
		item.AugSlotType[3]    = static_cast<uint8>(e.augslot4type);
		item.AugSlotVisible[3] = static_cast<uint8>(e.augslot4visible);
		item.AugSlotType[4]    = static_cast<uint8>(e.augslot5type);
		item.AugSlotVisible[4] = static_cast<uint8>(e.augslot5visible);
		item.AugSlotType[5]    = static_cast<uint8>(e.augslot6type);
		item.AugSlotVisible[5] = static_cast<uint8>(e.augslot6visible);

		// Augment Unknowns
		for (uint8 i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
			item.AugSlotUnk2[i] = 0;
		}

		// LDoN
		item.LDoNTheme        = e.ldontheme;
		item.LDoNPrice        = e.ldonprice;
		item.LDoNSellBackRate = e.ldonsellbackrate;
		item.LDoNSold         = e.ldonsold;
		item.PointType        = e.pointtype;

		// Bag
		item.BagSize  = static_cast<uint8>(e.bagsize);
		item.BagSlots = EQ::Clamp(e.bagslots, 0, static_cast<int>(EQ::invbag::SLOT_COUNT));
		item.BagType  = static_cast<uint8>(e.bagtype);
		item.BagWR    = EQ::Clamp(e.bagwr, 0, 100);

		// Bard Effect
		item.Bard.Effect = disable_bard_focus_effects ? 0 : e.bardeffect;
		item.Bard.Type   = disable_bard_focus_effects ? 0 : static_cast<uint8>(e.bardtype);
		item.Bard.Level  = disable_bard_focus_effects ? 0 : static_cast<uint8>(e.bardlevel);
		item.Bard.Level2 = disable_bard_focus_effects ? 0 : static_cast<uint8>(e.bardlevel2);

		// Book
		item.Book     = static_cast<uint8>(e.book);
		item.BookType = e.booktype;

		// Click Effect
		item.CastTime     = e.casttime;
		item.CastTime_    = e.casttime_;
		item.Click.Effect = e.clickeffect;
		item.Click.Type   = static_cast<uint8>(e.clicktype);
		item.Click.Level  = static_cast<uint8>(e.clicklevel);
		item.Click.Level2 = static_cast<uint8>(e.clicklevel2);
		strn0cpy(item.ClickName, e.clickname.c_str(), sizeof(item.ClickName));
		item.RecastDelay = e.recastdelay;
		item.RecastType  = e.recasttype;

		// Focus Effect
		item.Focus.Effect = disable_spell_focus_effects ? 0 : e.focuseffect;
		item.Focus.Type   = disable_spell_focus_effects ? 0 : static_cast<uint8>(e.focustype);
		item.Focus.Level  = disable_spell_focus_effects ? 0 : static_cast<uint8>(e.focuslevel);
		item.Focus.Level2 = disable_spell_focus_effects ? 0 : static_cast<uint8>(e.focuslevel2);
		strn0cpy(item.FocusName, disable_spell_focus_effects ? "" : e.focusname.c_str(), sizeof(item.FocusName));

		// Proc Effect
		item.Proc.Effect = e.proceffect;
		item.Proc.Type   = static_cast<uint8>(e.proctype);
		item.Proc.Level  = static_cast<uint8>(e.proclevel);
		item.Proc.Level2 = static_cast<uint8>(e.proclevel2);
		strn0cpy(item.ProcName, e.procname.c_str(), sizeof(item.ProcName));
		item.ProcRate = e.procrate;

		// Scroll Effect
		item.Scroll.Effect = e.scrolleffect;
		item.Scroll.Type   = static_cast<uint8>(e.scrolltype);
		item.Scroll.Level  = static_cast<uint8>(e.scrolllevel);
		item.Scroll.Level2 = static_cast<uint8>(e.scrolllevel2);
		strn0cpy(item.ScrollName, e.scrollname.c_str(), sizeof(item.ScrollName));

		// Worn Effect
		item.Worn.Effect = e.worneffect;
		item.Worn.Type   = static_cast<uint8>(e.worntype);
		item.Worn.Level  = static_cast<uint8>(e.wornlevel);
		item.Worn.Level2 = static_cast<uint8>(e.wornlevel2);
		strn0cpy(item.WornName, e.wornname.c_str(), sizeof(item.WornName));

		// Evolving Item
		item.EvolvingID    = e.evoid;
		item.EvolvingItem  = static_cast<uint8>(e.evoitem);
		item.EvolvingLevel = static_cast<uint8>(e.evolvinglevel);
		item.EvolvingMax   = static_cast<uint8>(e.evomax);

		// Scripting
		item.CharmFileID = Strings::IsNumber(e.charmfileid) ? Strings::ToUnsignedInt(e.charmfileid) : 0;
		strn0cpy(item.CharmFile, e.charmfile.c_str(), sizeof(item.CharmFile));
		strn0cpy(item.Filename, e.filename.c_str(), sizeof(item.Filename));
		item.ScriptFileID = e.scriptfileid;

		try {
			hash.insert(item.ID, item);
		} catch (std::exception &ex) {
			LogError("Database::LoadItems: {}", ex.what());
			break;
		}
	}
}

const EQ::ItemData *SharedDatabase::GetItem(uint32 id) const
{
	if (id == 0) {
		return nullptr;
	}

	if (!items_hash || id > items_hash->max_key()) {
		return nullptr;
	}

	if (items_hash->exists(id)) {
		return &(items_hash->at(id));
	}

	return nullptr;
}

const EQ::ItemData* SharedDatabase::IterateItems(uint32* id) const
{
	if(!items_hash || !id) {
		return nullptr;
	}

	for(;;) {
		if(*id > items_hash->max_key()) {
			break;
		}

		if(items_hash->exists(*id)) {
			return &(items_hash->at((*id)++));
		} else {
			++(*id);
		}
	}

	return nullptr;
}

Book_Struct SharedDatabase::GetBook(const std::string& text_file)
{
	const auto& l = BooksRepository::GetWhere(
		*this,
		fmt::format(
			"`name` = '{}'",
			Strings::Escape(text_file)
		)
	);

	Book_Struct b;

	if (l.empty()) {
		return b;
	}

	const auto& e = l.front();

	b.language = e.language;
	b.text     = e.txtfile;

	return b;
}

// Create appropriate EQ::ItemInstance class
EQ::ItemInstance* SharedDatabase::CreateItem(
	uint32 item_id,
	int16 charges,
	uint32 aug1,
	uint32 aug2,
	uint32 aug3,
	uint32 aug4,
	uint32 aug5,
	uint32 aug6,
	bool attuned,
	const std::string& custom_data,
	uint32 ornamenticon,
	uint32 ornamentidfile,
	uint32 ornament_hero_model
) {
	EQ::ItemInstance* inst = nullptr;

	const EQ::ItemData* item = GetItem(item_id);
	if (item) {
		inst = CreateBaseItem(item, charges);

		if (!inst) {
			LogError("Error: valid item data returned a null reference for EQ::ItemInstance creation in SharedDatabase::CreateItem()");
			LogError("Item Data = ID: {}, Name: {}, Charges: {}", item->ID, item->Name, charges);
			return nullptr;
		}

		inst->PutAugment(this, 0, aug1);
		inst->PutAugment(this, 1, aug2);
		inst->PutAugment(this, 2, aug3);
		inst->PutAugment(this, 3, aug4);
		inst->PutAugment(this, 4, aug5);
		inst->PutAugment(this, 5, aug6);
		inst->SetAttuned(attuned);
		inst->SetCustomDataString(custom_data);
		inst->SetOrnamentIcon(ornamenticon);
		inst->SetOrnamentationIDFile(ornamentidfile);
		inst->SetOrnamentHeroModel(ornament_hero_model);
	}

	return inst;
}


// Create appropriate EQ::ItemInstance class
EQ::ItemInstance* SharedDatabase::CreateItem(
	const EQ::ItemData* item,
	int16 charges,
	uint32 aug1,
	uint32 aug2,
	uint32 aug3,
	uint32 aug4,
	uint32 aug5,
	uint32 aug6,
	bool attuned,
	const std::string& custom_data,
	uint32 ornamenticon,
	uint32 ornamentidfile,
	uint32 ornament_hero_model
) {
	EQ::ItemInstance* inst = nullptr;
	if (item) {
		inst = CreateBaseItem(item, charges);

		if (!inst) {
			LogError("Error: valid item data returned a null reference for EQ::ItemInstance creation in SharedDatabase::CreateItem()");
			LogError("Item Data = ID: {}, Name: {}, Charges: {}", item->ID, item->Name, charges);
			return nullptr;
		}

		inst->PutAugment(this, 0, aug1);
		inst->PutAugment(this, 1, aug2);
		inst->PutAugment(this, 2, aug3);
		inst->PutAugment(this, 3, aug4);
		inst->PutAugment(this, 4, aug5);
		inst->PutAugment(this, 5, aug6);
		inst->SetAttuned(attuned);
		inst->SetCustomDataString(custom_data);
		inst->SetOrnamentIcon(ornamenticon);
		inst->SetOrnamentationIDFile(ornamentidfile);
		inst->SetOrnamentHeroModel(ornament_hero_model);
	}

	return inst;
}

EQ::ItemInstance* SharedDatabase::CreateBaseItem(const EQ::ItemData* item, int16 charges) {
	EQ::ItemInstance* inst = nullptr;
	if (item) {
		// if maxcharges is -1 that means it is an unlimited use item.
		// set it to 1 charge so that it is usable on creation
		if (charges == 0 && item->MaxCharges == -1) {
			charges = 1;
		}

		// Stackable items need a minimum charge of 1 to remain moveable.
		if (charges <= 0 && item->Stackable) {
			charges = 1;
		}

		inst = new EQ::ItemInstance(item, charges);

		if (!inst) {
			LogError("Error: valid item data returned a null reference for EQ::ItemInstance creation in SharedDatabase::CreateBaseItem()");
			LogError("Item Data = ID: {}, Name: {}, Charges: {}", item->ID, item->Name, charges);
			return nullptr;
		}

		if (item->CharmFileID != 0 || item->LoreGroup >= 1000) {
			inst->Initialize(this);
		}
	}

	return inst;
}

int SharedDatabase::DeleteStalePlayerCorpses() {
	return (
		RuleB(Zone, EnableShadowrest) ?
		CharacterCorpsesRepository::BuryDecayedCorpses(*this) :
		CharacterCorpsesRepository::DeleteWhere(
			*this,
			fmt::format(
				"(UNIX_TIMESTAMP() - UNIX_TIMESTAMP(time_of_death)) > {} AND time_of_death != 0",
				RuleI(Character, CorpseDecayTime) / 1000
			)
		)
	);
}

bool SharedDatabase::GetCommandSettings(std::map<std::string, std::pair<uint8, std::vector<std::string>>> &command_settings)
{
	command_settings.clear();

	const std::string& query = "SELECT `command`, `access`, `aliases` FROM `command_settings`";
	auto results = QueryDatabase(query);
	if (!results.Success() || !results.RowCount()) {
		return false;
	}

	for (auto row : results) {
		command_settings[row[0]].first = Strings::ToUnsignedInt(row[1]);
		if (row[2][0] == 0) {
			continue;
		}

		std::vector<std::string> aliases = Strings::Split(row[2], '|');
		for (const auto& e : aliases) {
			if (e.empty()) {
				continue;
			}

			command_settings[row[0]].second.push_back(e);
		}
	}

	return true;
}

template<typename T1, typename T2>
inline std::vector<std::string> join_pair(
	const std::string &glue,
	const std::pair<char, char> &encapsulation,
	const std::vector<std::pair<T1, T2>> &src
)
{
	if (src.empty()) {
		return {};
	}

	std::vector<std::string> output;

	for (const std::pair<T1, T2> &src_iter: src) {
		output.emplace_back(

			fmt::format(
				"{}{}{}{}{}{}{}",
				encapsulation.first,
				src_iter.first,
				encapsulation.second,
				glue,
				encapsulation.first,
				src_iter.second,
				encapsulation.second
			)
		);
	}

	return output;
}

template<typename T>
inline std::string
ImplodePair(const std::string &glue, const std::pair<char, char> &encapsulation, const std::vector<T> &src)
{
	if (src.empty()) {
		return {};
	}
	std::ostringstream oss;
	for (const T &src_iter: src) {
		oss << encapsulation.first << src_iter << encapsulation.second << glue;
	}
	std::string output(oss.str());
	output.resize(output.size() - glue.size());
	return output;
}

bool SharedDatabase::UpdateInjectedCommandSettings(const std::vector<std::pair<std::string, uint8>> &injected)
{
	if (injected.size()) {
		const std::string query = fmt::format(
			"REPLACE INTO `command_settings`(`command`, `access`) VALUES {}",
			ImplodePair(
				",",
				std::pair<char, char>('(', ')'),
				join_pair(",", std::pair<char, char>('\'', '\''), injected)
			)
		);

		auto results = QueryDatabase(query);
		if (!results.Success()) {
			return false;
		}

		LogInfo(
			"[{}] New Command{} Added",
			injected.size(),
			injected.size() != 1 ? "s" : ""
		);
	}

	return true;
}

bool SharedDatabase::UpdateOrphanedCommandSettings(const std::vector<std::string> &orphaned)
{
	if (orphaned.size()) {
		std::string query = fmt::format(
			"DELETE FROM `command_settings` WHERE `command` IN ({})",
			ImplodePair(",", std::pair<char, char>('\'', '\''), orphaned)
		);

		auto results = QueryDatabase(query);
		if (!results.Success()) {
			return false;
		}

		query = fmt::format(
			"DELETE FROM `command_subsettings` WHERE `parent_command` IN ({})",
			ImplodePair(",", std::pair<char, char>('\'', '\''), orphaned)
		);

		auto results_two = QueryDatabase(query);
		if (!results_two.Success()) {
			return false;
		}

		LogInfo(
			"{} Orphaned Command{} Deleted | {} Orphaned Subcommand{} Deleted",
			orphaned.size(),
			orphaned.size() != 1 ? "s" : "",
			results_two.RowsAffected(),
			results_two.RowsAffected() != 1 ? "s" : ""
		);
	}

	return true;
}

bool SharedDatabase::GetCommandSubSettings(std::vector<CommandSubsettingsRepository::CommandSubsettings> &command_subsettings)
{
	command_subsettings.clear();

	const auto& l = CommandSubsettingsRepository::GetAll(*this);

	if (l.empty()) {
		return false;
	}

	command_subsettings.reserve(l.size());

	for (const auto& e : l) {
		command_subsettings.emplace_back(e);
	}

	return true;
}

void SharedDatabase::LoadDamageShieldTypes(SPDat_Spell_Struct* s)
{
	const auto& l = DamageshieldtypesRepository::All(*this);

	if (l.empty()) {
		return;
	}

	for (const auto& e : l) {
		s[e.spellid].damage_shield_type = e.type;
	}
}

const EvolveInfo* SharedDatabase::GetEvolveInfo(uint32 loregroup) {
	return nullptr;	// nothing here for now... database and/or sharemem pulls later
}

int SharedDatabase::GetMaxSpellID() {
	const std::string query = "SELECT MAX(id) FROM spells_new";
	auto results = QueryDatabase(query);
    if (!results.Success()) {
        return -1;
    }

    auto& row = results.begin();

	return Strings::ToInt(row[0]);
}

bool SharedDatabase::LoadSpells(const std::string &prefix, int32 *records, const SPDat_Spell_Struct **sp) {
	spells_mmf.reset(nullptr);

	try {
		const auto Config = EQEmuConfig::get();
		EQ::IPCMutex mutex("spells");
		mutex.Lock();

		std::string file_name = fmt::format("{}/{}{}", PathManager::Instance()->GetSharedMemoryPath(), prefix, std::string("spells"));
		spells_mmf = std::make_unique<EQ::MemoryMappedFile>(file_name);
		LogInfo("Loading [{}]", file_name);
		*records = *static_cast<uint32*>(spells_mmf->Get());
		*sp = reinterpret_cast<const SPDat_Spell_Struct*>(static_cast<char*>(spells_mmf->Get()) + 4);
		mutex.Unlock();

		LogInfo("Loaded [{}] spells via shared memory", Strings::Commify(m_shared_spells_count));
	}
	catch(std::exception& ex) {
		LogError("Error Loading Spells: {}", ex.what());
		return false;
	}

	return true;
}

void SharedDatabase::LoadSpells(void *data, int max_spells) {
	*static_cast<uint32*>(data) = max_spells;
	SPDat_Spell_Struct *sp = reinterpret_cast<SPDat_Spell_Struct*>(static_cast<char*>(data) + sizeof(uint32));

	const std::string query = "SELECT * FROM spells_new ORDER BY id ASC";
    auto results = QueryDatabase(query);
    if (!results.Success()) {
        return;
    }

    if(results.ColumnCount() <= SPELL_LOAD_FIELD_COUNT) {
		LogSpells("Fatal error loading spells: Spell field count < SPELL_LOAD_FIELD_COUNT([{}])", SPELL_LOAD_FIELD_COUNT);
		return;
    }

	int counter = 0;

    for (auto& row = results.begin(); row != results.end(); ++row) {
	    const int tempid = Strings::ToInt(row[0]);
        if(tempid >= max_spells) {
			LogSpells("Non fatal error: spell.id >= max_spells, ignoring");
			continue;
		}

        ++counter;
        sp[tempid].id = tempid;
        strn0cpy(sp[tempid].name, row[1], sizeof(sp[tempid].name));
        strn0cpy(sp[tempid].player_1, row[2], sizeof(sp[tempid].player_1));
		strn0cpy(sp[tempid].teleport_zone, row[3], sizeof(sp[tempid].teleport_zone));
		strn0cpy(sp[tempid].you_cast, row[4], sizeof(sp[tempid].you_cast));
		strn0cpy(sp[tempid].other_casts, row[5], sizeof(sp[tempid].other_casts));
		strn0cpy(sp[tempid].cast_on_you, row[6], sizeof(sp[tempid].cast_on_you));
		strn0cpy(sp[tempid].cast_on_other, row[7], sizeof(sp[tempid].cast_on_other));
		strn0cpy(sp[tempid].spell_fades, row[8], sizeof(sp[tempid].spell_fades));

		sp[tempid].range = Strings::ToFloat(row[9]);
		sp[tempid].aoe_range = Strings::ToFloat(row[10]);
		sp[tempid].push_back = Strings::ToFloat(row[11]);
		sp[tempid].push_up = Strings::ToFloat(row[12]);
		sp[tempid].cast_time=Strings::ToUnsignedInt(row[13]);
		sp[tempid].recovery_time=Strings::ToUnsignedInt(row[14]);
		sp[tempid].recast_time=Strings::ToUnsignedInt(row[15]);
		sp[tempid].buff_duration_formula=Strings::ToUnsignedInt(row[16]);
		sp[tempid].buff_duration=Strings::ToUnsignedInt(row[17]);
		sp[tempid].aoe_duration=Strings::ToUnsignedInt(row[18]);
		sp[tempid].mana=Strings::ToInt(row[19]);

		int y=0;
		for(y=0; y< EFFECT_COUNT;y++)
			sp[tempid].base_value[y]=Strings::ToInt(row[20+y]); // effect_base_value

		for(y=0; y < EFFECT_COUNT; y++)
			sp[tempid].limit_value[y]=Strings::ToInt(row[32+y]); // effect_limit_value

		for(y=0; y< EFFECT_COUNT;y++)
			sp[tempid].max_value[y]=Strings::ToInt(row[44+y]);

		for(y=0; y< 4;y++)
			sp[tempid].component[y]=Strings::ToInt(row[58+y]);

		for(y=0; y< 4;y++)
			sp[tempid].component_count[y]=Strings::ToInt(row[62+y]);

		for(y=0; y< 4;y++)
			sp[tempid].no_expend_reagent[y]=Strings::ToInt(row[66+y]);

		for(y=0; y< EFFECT_COUNT;y++)
			sp[tempid].formula[y]=Strings::ToUnsignedInt(row[70+y]);

		sp[tempid].good_effect=Strings::ToInt(row[83]);
		sp[tempid].activated=Strings::ToInt(row[84]);
		sp[tempid].resist_type=Strings::ToInt(row[85]);

		for(y=0; y< EFFECT_COUNT;y++)
			sp[tempid].effect_id[y]=Strings::ToInt(row[86+y]);

		sp[tempid].target_type = static_cast<SpellTargetType>(Strings::ToInt(row[98]));
		sp[tempid].base_difficulty=Strings::ToInt(row[99]);

		int tmp_skill = Strings::ToInt(row[100]);

		if (tmp_skill < 0 || tmp_skill > EQ::skills::HIGHEST_SKILL)
			sp[tempid].skill = EQ::skills::SkillBegging; /* not much better we can do. */ // can probably be changed to client-based 'SkillNone' once activated
        else
			sp[tempid].skill = static_cast<EQ::skills::SkillType>(tmp_skill);

		sp[tempid].zone_type=Strings::ToInt(row[101]);
		sp[tempid].environment_type=Strings::ToInt(row[102]);
		sp[tempid].time_of_day=Strings::ToInt(row[103]);

		for(y=0; y < Class::PLAYER_CLASS_COUNT;y++)
			sp[tempid].classes[y]=Strings::ToInt(row[104+y]);

		sp[tempid].casting_animation=Strings::ToInt(row[120]);
		sp[tempid].spell_affect_index=Strings::ToInt(row[123]);
		sp[tempid].disallow_sit=Strings::ToInt(row[124]);
		sp[tempid].deity_agnostic=Strings::ToInt(row[125]);

		for (y = 0; y < 16; y++)
			sp[tempid].deities[y]=Strings::ToInt(row[126+y]);

		sp[tempid].new_icon=Strings::ToInt(row[144]);
		sp[tempid].uninterruptable=Strings::ToBool(row[146]);
		sp[tempid].resist_difficulty=Strings::ToInt(row[147]);
		sp[tempid].unstackable_dot = Strings::ToBool(row[148]);
		sp[tempid].recourse_link = Strings::ToUnsignedInt(row[150]);
		sp[tempid].no_partial_resist = Strings::ToBool(row[151]);

		sp[tempid].short_buff_box = Strings::ToInt(row[154]);
		sp[tempid].description_id = Strings::ToInt(row[155]);
		sp[tempid].type_description_id = Strings::ToInt(row[156]);
		sp[tempid].effect_description_id = Strings::ToInt(row[157]);

		sp[tempid].npc_no_los = Strings::ToBool(row[159]);
		sp[tempid].feedbackable = Strings::ToBool(row[160]);
		sp[tempid].reflectable = Strings::ToBool(row[161]);
		sp[tempid].bonus_hate=Strings::ToInt(row[162]);

		sp[tempid].ldon_trap = Strings::ToBool(row[165]);
		sp[tempid].endurance_cost= Strings::ToInt(row[166]);
		sp[tempid].timer_id= Strings::ToInt(row[167]);
		sp[tempid].is_discipline = Strings::ToBool(row[168]);
		sp[tempid].hate_added= Strings::ToInt(row[173]);
		sp[tempid].endurance_upkeep=Strings::ToInt(row[174]);
		sp[tempid].hit_number_type = Strings::ToInt(row[175]);
		sp[tempid].hit_number = Strings::ToInt(row[176]);
		sp[tempid].pvp_resist_base= Strings::ToInt(row[177]);
		sp[tempid].pvp_resist_per_level= Strings::ToInt(row[178]);
		sp[tempid].pvp_resist_cap= Strings::ToInt(row[179]);
		sp[tempid].spell_category= Strings::ToInt(row[180]);
		sp[tempid].pvp_duration = Strings::ToInt(row[181]);
		sp[tempid].pvp_duration_cap = Strings::ToInt(row[182]);
		sp[tempid].pcnpc_only_flag= Strings::ToInt(row[183]);
		sp[tempid].cast_not_standing = Strings::ToInt(row[184]) != 0;
		sp[tempid].can_mgb= Strings::ToBool(row[185]);
		sp[tempid].dispel_flag = Strings::ToInt(row[186]);
		sp[tempid].min_resist = Strings::ToInt(row[189]);
		sp[tempid].max_resist = Strings::ToInt(row[190]);
		sp[tempid].viral_targets = Strings::ToInt(row[191]);
		sp[tempid].viral_timer = Strings::ToInt(row[192]);
		sp[tempid].nimbus_effect = Strings::ToInt(row[193]);
		sp[tempid].directional_start = Strings::ToFloat(row[194]);
		sp[tempid].directional_end = Strings::ToFloat(row[195]);
		sp[tempid].sneak = Strings::ToBool(row[196]);
		sp[tempid].not_focusable = Strings::ToBool(row[197]);
		sp[tempid].no_detrimental_spell_aggro = Strings::ToBool(row[198]);
		sp[tempid].suspendable = Strings::ToBool(row[200]);
		sp[tempid].viral_range = Strings::ToInt(row[201]);
		sp[tempid].song_cap = Strings::ToInt(row[202]);
		sp[tempid].no_block = Strings::ToInt(row[205]);
		sp[tempid].spell_group=Strings::ToInt(row[207]);
		sp[tempid].rank = Strings::ToInt(row[208]);
		sp[tempid].no_resist=Strings::ToInt(row[209]);
		sp[tempid].cast_restriction = Strings::ToInt(row[211]);
		sp[tempid].allow_rest = Strings::ToBool(row[212]);
		sp[tempid].can_cast_in_combat = Strings::ToBool(row[213]);
		sp[tempid].can_cast_out_of_combat = Strings::ToBool(row[214]);
		sp[tempid].override_crit_chance = Strings::ToInt(row[217]);
		sp[tempid].aoe_max_targets = Strings::ToInt(row[218]);
		sp[tempid].no_heal_damage_item_mod = Strings::ToInt(row[219]);
		sp[tempid].caster_requirement_id = Strings::ToInt(row[220]);
		sp[tempid].spell_class = Strings::ToInt(row[221]);
		sp[tempid].spell_subclass = Strings::ToInt(row[222]);
		sp[tempid].persist_death = Strings::ToBool(row[224]);
		sp[tempid].min_distance = Strings::ToFloat(row[227]);
		sp[tempid].min_distance_mod = Strings::ToFloat(row[228]);
		sp[tempid].max_distance = Strings::ToFloat(row[229]);
		sp[tempid].max_distance_mod = Strings::ToFloat(row[230]);
		sp[tempid].min_range = Strings::ToFloat(row[231]);
		sp[tempid].no_remove = Strings::ToBool(row[232]);
		sp[tempid].damage_shield_type = 0;
	}

	LoadDamageShieldTypes(sp);
}

void SharedDatabase::LoadCharacterInspectMessage(uint32 character_id, InspectMessage_Struct* s)
{
	const auto& e = CharacterInspectMessagesRepository::FindOne(*this, character_id);

	memset(s, '\0', sizeof(InspectMessage_Struct));

	if (!e.id) {
		return;
	}

	memcpy(s, e.inspect_message.c_str(), sizeof(InspectMessage_Struct));
}

void SharedDatabase::SaveCharacterInspectMessage(uint32 character_id, const InspectMessage_Struct* s)
{
	auto e = CharacterInspectMessagesRepository::NewEntity();

	e.id              = character_id;
	e.inspect_message = s->text;

	if (!CharacterInspectMessagesRepository::ReplaceOne(*this, e)) {
		LogError("Failed to save character inspect message of [{}] for character_id [{}]", s->text, character_id);
	}
}

uint32 SharedDatabase::GetSpellsCount()
{
	auto results = QueryDatabase("SELECT count(*) FROM spells_new");
	if (!results.Success() || !results.RowCount()) {
		return 0;
	}

	auto& row = results.begin();

	if (row[0]) {
		return Strings::ToUnsignedInt(row[0]);
	}

	return 0;
}

uint32 SharedDatabase::GetItemsCount()
{
	return ItemsRepository::Count(*this);
}

void SharedDatabase::SetSharedItemsCount(uint32 shared_items_count)
{
	SharedDatabase::m_shared_items_count = shared_items_count;
}

void SharedDatabase::SetSharedSpellsCount(uint32 shared_spells_count)
{
	SharedDatabase::m_shared_spells_count = shared_spells_count;
}
