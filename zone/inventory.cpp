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

#include "client.h"

#include "common/eqemu_logsys.h"
#include "common/events/player_event_logs.h"
#include "common/evolving_items.h"
#include "common/repositories/character_corpse_items_repository.h"
#include "common/strings.h"
#include "zone/bot.h"
#include "zone/queryserv.h"
#include "zone/quest_parser_collection.h"
#include "zone/worldserver.h"
#include "zone/zonedb.h"

extern WorldServer worldserver;
extern QueryServ  *QServ;

// @merth: this needs to be touched up
uint32 Client::NukeItem(uint32 itemnum, uint8 where_to_check) {
	if (itemnum == 0)
		return 0;
	uint32 x = 0;
	EQ::ItemInstance *cur = nullptr;

	int i;
	if(where_to_check & invWhereWorn) {
		for (i = EQ::invslot::EQUIPMENT_BEGIN; i <= EQ::invslot::EQUIPMENT_END; i++) {
			if (GetItemIDAt(i) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(i) != INVALID_ID)) {
				cur = m_inv.GetItem(i);
				if(cur && cur->GetItem()->Stackable) {
					x += cur->GetCharges();
				} else {
					x++;
				}

				DeleteItemInInventory(i, 0, ((((uint64)1 << i) & GetInv().GetLookup()->PossessionsBitmask) != 0));
			}
		}
	}

	if(where_to_check & invWhereCursor) {
		if (GetItemIDAt(EQ::invslot::slotCursor) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(EQ::invslot::slotCursor) != INVALID_ID)) {
			cur = m_inv.GetItem(EQ::invslot::slotCursor);
			if(cur && cur->GetItem()->Stackable) {
				x += cur->GetCharges();
			} else {
				x++;
			}

			DeleteItemInInventory(EQ::invslot::slotCursor, 0, true);
		}

		for (i = EQ::invbag::CURSOR_BAG_BEGIN; i <= EQ::invbag::CURSOR_BAG_END; i++) {
			if (GetItemIDAt(i) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(i) != INVALID_ID)) {
				cur = m_inv.GetItem(i);
				if(cur && cur->GetItem()->Stackable) {
					x += cur->GetCharges();
				} else {
					x++;
				}

				DeleteItemInInventory(i, 0, true);
			}
		}
	}

	if(where_to_check & invWherePersonal) {
		for (i = EQ::invslot::GENERAL_BEGIN; i <= EQ::invslot::GENERAL_END; i++) {
			if (GetItemIDAt(i) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(i) != INVALID_ID)) {
				cur = m_inv.GetItem(i);
				if(cur && cur->GetItem()->Stackable) {
					x += cur->GetCharges();
				} else {
					x++;
				}

				DeleteItemInInventory(i, 0, ((((uint64)1 << i) & GetInv().GetLookup()->PossessionsBitmask) != 0));
			}
		}

		for (i = EQ::invbag::GENERAL_BAGS_BEGIN; i <= EQ::invbag::GENERAL_BAGS_END; i++) {
			if (GetItemIDAt(i) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(i) != INVALID_ID)) {
				cur = m_inv.GetItem(i);
				if(cur && cur->GetItem()->Stackable) {
					x += cur->GetCharges();
				} else {
					x++;
				}

				DeleteItemInInventory(i, 0, ((((uint64)1 << (EQ::invslot::GENERAL_BEGIN + ((i - EQ::invbag::GENERAL_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT))) & GetInv().GetLookup()->PossessionsBitmask) != 0));
			}
		}
	}

	if(where_to_check & invWhereBank) {
		for (i = EQ::invslot::BANK_BEGIN; i <= EQ::invslot::BANK_END; i++) {
			if (GetItemIDAt(i) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(i) != INVALID_ID)) {
				cur = m_inv.GetItem(i);
				if(cur && cur->GetItem()->Stackable) {
					x += cur->GetCharges();
				} else {
					x++;
				}

				DeleteItemInInventory(i, 0, ((i - EQ::invslot::BANK_BEGIN) >= GetInv().GetLookup()->InventoryTypeSize.Bank));
			}
		}

		for (i = EQ::invbag::BANK_BAGS_BEGIN; i <= EQ::invbag::BANK_BAGS_END; i++) {
			if (GetItemIDAt(i) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(i) != INVALID_ID)) {
				cur = m_inv.GetItem(i);
				if(cur && cur->GetItem()->Stackable) {
					x += cur->GetCharges();
				} else {
					x++;
				}

				DeleteItemInInventory(i, 0, (((i - EQ::invbag::BANK_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT) >= GetInv().GetLookup()->InventoryTypeSize.Bank));
			}
		}
	}

	if(where_to_check & invWhereSharedBank) {
		for (i = EQ::invslot::SHARED_BANK_BEGIN; i <= EQ::invslot::SHARED_BANK_END; i++) {
			if (GetItemIDAt(i) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(i) != INVALID_ID)) {
				cur = m_inv.GetItem(i);
				if(cur && cur->GetItem()->Stackable) {
					x += cur->GetCharges();
				} else {
					x++;
				}

				DeleteItemInInventory(i, 0, true);
			}
		}

		for (i = EQ::invbag::SHARED_BANK_BAGS_BEGIN; i <= EQ::invbag::SHARED_BANK_BAGS_END; i++) {
			if (GetItemIDAt(i) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(i) != INVALID_ID)) {
				cur = m_inv.GetItem(i);
				if(cur && cur->GetItem()->Stackable) {
					x += cur->GetCharges();
				} else {
					x++;
				}

				DeleteItemInInventory(i, 0, true);
			}
		}
	}

	return x;
}


bool Client::CheckLoreConflict(const EQ::ItemData* item)
{
	if (!item) { return false; }
	if (!item->LoreFlag) { return false; }
	if (item->LoreGroup == 0) { return false; }

	if (item->LoreGroup == -1) // Standard lore items; look everywhere except the shared bank, return the result
		return (m_inv.HasItem(item->ID, 0, ~invWhereSharedBank) != INVALID_INDEX);

	// If the item has a lore group, we check for other items with the same group and return the result
	return (m_inv.HasItemByLoreGroup(item->LoreGroup, ~invWhereSharedBank) != INVALID_INDEX);
}

bool Client::SummonItem(uint32 item_id, int16 charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5, uint32 aug6, bool attuned, uint16 to_slot, uint32 ornament_icon, uint32 ornament_idfile, uint32 ornament_hero_model) {
	const EQ::ItemData* item = database.GetItem(item_id);

	// make sure the item exists
	if(item == nullptr) {
		Message(
			Chat::Red,
			fmt::format(
				"Item {} does not exist.",
				item_id
			).c_str()
		);
		LogInventory(
			"Player [{}] on account [{}] attempted to create an item with an invalid id.\n"
			"Item [{}], Aug1: [{}], Aug2: [{}], Aug3: [{}], Aug4: [{}], Aug5: [{}], Aug6: [{}])\n",
			GetName(),
			account_name,
			item_id,
			aug1,
			aug2,
			aug3,
			aug4,
			aug5,
			aug6
		);
		return false;
	}
	// check that there is not a lore conflict between base item and existing inventory
	else if(CheckLoreConflict(item)) {
		// DuplicateLoreMessage(item_id);
		Message(
			Chat::Red,
			fmt::format(
				"You already have a lore {} ({}) in your inventory.",
				database.CreateItemLink(item_id),
				item_id
			).c_str()
		);
		return false;
	}
	// check to make sure we are augmenting an augmentable item
	else if (((!item->IsClassCommon()) || (item->AugType > 0)) && (aug1 | aug2 | aug3 | aug4 | aug5 | aug6)) {
		Message(Chat::Red, "You can not augment an augment or a non-common class item.");
		LogInventory(
			"Player [{}] on account [{}] attempted to augment an augment or a non-common class item.\n"
			"Item [{}], Aug1: [{}], Aug2: [{}], Aug3: [{}], Aug4: [{}], Aug5: [{}], Aug5: [{}])\n",
			GetName(),
			account_name,
			item->ID,
			aug1,
			aug2,
			aug3,
			aug4,
			aug5,
			aug6
		);
		return false;
	}

	// This code is ready to implement once the item load code is changed to process the 'minstatus' field.
	// Checking #iteminfo in-game verfies that item->MinStatus is set to '0' regardless of field value.
	// An optional sql script will also need to be added, once this goes live, to allow changing of the min status.

	// check to make sure we are a GM if the item is GM-only
	/*
	else if(item->MinStatus && ((Admin() < item->MinStatus) || (Admin() < RuleI(GM, MinStatusToSummonItem)))) {
		Message(Chat::Red, "You are not a GM or do not have the status to summon this item.");
		LogInventory("Player [{}] on account [{}] attempted to create a GM-only item with a status of [{}].\n"Item [{}], Aug1: [{}], Aug2: [{}], Aug3: [{}], Aug4: [{}], Aug5: [{}], Aug6: [{}], MinStatus: [{}])\n",
			GetName(), account_name, Admin(), item->ID, aug1, aug2, aug3, aug4, aug5, aug6, item->MinStatus);

		return false;
	}
	*/

	uint32 augments[EQ::invaug::SOCKET_COUNT] = { aug1, aug2, aug3, aug4, aug5, aug6 };
	uint32 classes = item->Classes;
	uint32 races = item->Races;
	uint32 slots = item->Slots;
	bool enforce_wearable = RuleB(Inventory, EnforceAugmentWear);
	bool enforce_restrictions = RuleB(Inventory, EnforceAugmentRestriction);
	bool enforce_usable = RuleB(Inventory, EnforceAugmentUsability);
	for (int iter = EQ::invaug::SOCKET_BEGIN; iter <= EQ::invaug::SOCKET_END; ++iter) {
		int augment_slot = iter + 1;
		const EQ::ItemData* augtest = database.GetItem(augments[iter]);
		if(augtest == nullptr) {
			if(augments[iter]) {
				Message(
					Chat::Red,
					fmt::format(
						"Augment {} in Augment Slot {} does not exist.",
						augments[iter],
						augment_slot
					).c_str()
				);
				LogInventory(
					"Player [{}] on account [{}] attempted to create an augment (Aug[{}]) with an invalid id.\n"
					"Item [{}], Aug1: [{}], Aug2: [{}], Aug3: [{}], Aug4: [{}], Aug5: [{}], Aug6: [{}])\n",
					GetName(),
					account_name,
					augment_slot,
					item->ID,
					aug1,
					aug2,
					aug3,
					aug4,
					aug5,
					aug6
				);

				return false;
			}
		}
		else {
			// check that there is not a lore conflict between augment and existing inventory
			if(CheckLoreConflict(augtest)) {
				// DuplicateLoreMessage(augtest->ID);
				Message(
					Chat::Red,
					fmt::format(
						"You already have a lore {} ({}) in your inventory.",
						database.CreateItemLink(augtest->ID),
						augtest->ID
					).c_str()
				);

				return false;
			}
			// check that augment is an actual augment
			else if(augtest->AugType == 0) {
				Message(
					Chat::Red,
					fmt::format(
						"{} ({}) in Augment Slot {} is not an actual augment.",
						database.CreateItemLink(augtest->ID),
						augtest->ID,
						augment_slot
					).c_str()
				);
				LogInventory(
					"Player [{}] on account [{}] attempted to use a non-augment item (Augment Slot [{}]) as an augment.\n"
					"Item [{}], Aug1: [{}], Aug2: [{}], Aug3: [{}], Aug4: [{}], Aug5: [{}], Aug6: [{}])\n",
					GetName(),
					account_name,
					item->ID,
					augment_slot,
					aug1,
					aug2,
					aug3,
					aug4,
					aug5,
					aug6
				);

				return false;
			}

			// Same as GM check above

			// check to make sure we are a GM if the augment is GM-only
			/*
			else if(augtest->MinStatus && ((Admin() < augtest->MinStatus) || (Admin() < RuleI(GM, MinStatusToSummonItem)))) {
				Message(Chat::Red, "You are not a GM or do not have the status to summon this augment.");
				LogInventory("Player [{}] on account [{}] attempted to create a GM-only augment (Aug[{}]) with a status of [{}].\n(Item: [{}], Aug1: [{}], Aug2: [{}], Aug3: [{}], Aug4: [{}], Aug5: [{}], MinStatus: [{}])\n",
					GetName(), account_name, augment_slot, Admin(), item->ID, aug1, aug2, aug3, aug4, aug5, aug6, item->MinStatus);

				return false;
			}
			*/

			// check for augment type allowance
			if(enforce_wearable) {
				if ((item->AugSlotType[iter] == EQ::item::AugTypeNone) || !(((uint32)1 << (item->AugSlotType[iter] - 1)) & augtest->AugType)) {
					Message(
						Chat::Red,
						fmt::format(
							"Augment {} ({}) in Augment Slot {} is not capable of being socketed in to {} ({}).",
							database.CreateItemLink(augments[iter]),
							augments[iter],
							augment_slot,
							database.CreateItemLink(item->ID),
							item->ID
						).c_str()
					);
					LogInventory(
						"Player [{}] on account [{}] attempted to augment an item with an unacceptable augment type (Aug[{}]).\n"
						"Item [{}], Aug1: [{}], Aug2: [{}], Aug3: [{}], Aug4: [{}], Aug5: [{}], Aug6: [{}])\n",
						GetName(),
						account_name,
						augment_slot,
						item->ID,
						aug1,
						aug2,
						aug3,
						aug4,
						aug5,
						aug6
					);

					return false;
				}

				if (!RuleB(Items, SummonItemAllowInvisibleAugments) && item->AugSlotVisible[iter] == 0) {
					Message(
						Chat::Red,
						fmt::format(
							"{} ({}) has not evolved enough to accept {} ({}) in Augment Slot {}.",
							database.CreateItemLink(item->ID),
							item->ID,
							database.CreateItemLink(augments[iter]),
							augments[iter],
							augment_slot
						).c_str()
					);
					LogInventory(
						"Player [{}] on account [{}] attempted to augment an unevolved item with augment type (Aug[{}]).\n"
						"Item [{}], Aug1: [{}], Aug2: [{}], Aug3: [{}], Aug4: [{}], Aug5: [{}], Aug6: [{}])\n",
						GetName(),
						account_name,
						augment_slot,
						item->ID,
						aug1,
						aug2,
						aug3,
						aug4,
						aug5,
						aug6
					);

					return false;
				}
			}

			// check for augment to item restriction
			if (enforce_restrictions) {
				bool is_restricted = IsAugmentRestricted(item->ItemType, augtest->AugRestrict);

				if (is_restricted) {
					Message(
						Chat::Red,
						fmt::format(
							"{} ({}) in Augment Slot {} is restricted from being augmented in to {} ({}).",
							database.CreateItemLink(augments[iter]),
							augments[iter],
							augment_slot,
							database.CreateItemLink(item->ID),
							item->ID
						).c_str()
					);
					LogInventory(
						"Player [{}] on account [{}] attempted to augment an item with a restricted augment (Aug[{}]).\n"
						"Item [{}], Aug1: [{}], Aug2: [{}], Aug3: [{}], Aug4: [{}], Aug5: [{}], Aug6: [{}])\n",
						GetName(),
						account_name,
						augment_slot,
						item->ID,
						aug1,
						aug2,
						aug3,
						aug4,
						aug5,
						aug6
					);

					return false;
				}
			}

			if(enforce_usable) {
				// check for class usability
				if(item->Classes && !(classes &= augtest->Classes)) {
					Message(
						Chat::Red,
						fmt::format(
							"{} ({}) in Augment Slot {} will result in an item unusable by any class.",
							database.CreateItemLink(augments[iter]),
							augments[iter],
							augment_slot
						).c_str()
					);
					LogInventory(
						"Player [{}] on account [{}] attempted to create an item unusable by any class.\n"
						"Item [{}], Aug1: [{}], Aug2: [{}], Aug3: [{}], Aug4: [{}], Aug5: [{}], Aug6: [{}])\n",
						GetName(),
						account_name,
						item->ID,
						aug1,
						aug2,
						aug3,
						aug4,
						aug5,
						aug6
					);

					return false;
				}

				// check for race usability
				if(item->Races && !(races &= augtest->Races)) {
					Message(
						Chat::Red,
						fmt::format(
							"{} ({}) in Augment Slot {} will result in an item unusable by any race.",
							database.CreateItemLink(augments[iter]),
							augments[iter],
							augment_slot
						).c_str()
					);
					LogInventory(
						"Player [{}] on account [{}] attempted to create an item unusable by any race.\n"
						"Item [{}], Aug1: [{}], Aug2: [{}], Aug3: [{}], Aug4: [{}], Aug5: [{}], Aug6: [{}])\n",
						GetName(),
						account_name,
						item->ID,
						aug1,
						aug2,
						aug3,
						aug4,
						aug5,
						aug6
					);

					return false;
				}

				// check for slot usability
				if(item->Slots && !(slots &= augtest->Slots)) {
					Message(
						Chat::Red,
						fmt::format(
							"{} ({}) in Augment Slot {} will result in an item unusable in any slot.",
							database.CreateItemLink(augments[iter]),
							augments[iter],
							augment_slot
						).c_str()
					);
					LogInventory(
						"Player [{}] on account [{}] attempted to create an item unusable in any slot.\n"
						"Item [{}], Aug1: [{}], Aug2: [{}], Aug3: [{}], Aug4: [{}], Aug5: [{}], Aug6: [{}])\n",
						GetName(),
						account_name,
						item->ID,
						aug1,
						aug2,
						aug3,
						aug4,
						aug5,
						aug6
					);

					return false;
				}
			}
		}
	}

	// validation passed..so, set the charges and create the actual item

	// if the item is stackable and the charge amount is -1 or 0 then set to 1 charge.
	// removed && item->MaxCharges == 0 if -1 or 0 was passed max charges is irrelevant
	if(charges <= 0 && item->Stackable) {
		charges = 1;
	} else if(charges == -1) { // if the charges is -1, then no charge value was passed in set to max charges
		charges = item->MaxCharges;
	}

	// in any other situation just use charges as passed

	EQ::ItemInstance* inst = database.CreateItem(item, charges);
	auto timestamps = database.GetItemRecastTimestamps(CharacterID());
	if (inst) {
		const auto* d = inst->GetItem();
		if (d->RecastDelay) {
			if (d->RecastType != RECAST_TYPE_UNLINKED_ITEM) {
				inst->SetRecastTimestamp(timestamps.count(d->RecastType) ? timestamps.at(d->RecastType) : 0);
			}
			else {
				inst->SetRecastTimestamp(timestamps.count(d->ID) ? timestamps.at(d->ID) : 0);
			}
		}
	}

	if(inst == nullptr) {
		Message(Chat::Red, "An unknown server error has occurred and your item was not created.");
		// this goes to logfile since this is a major error
		LogError(
			"Player [{}] on account [{}] encountered an unknown item creation error.\n"
			"Item [{}], Aug1: [{}], Aug2: [{}], Aug3: [{}], Aug4: [{}], Aug5: [{}], Aug6: [{}])\n",
			GetName(),
			account_name,
			item->ID,
			aug1,
			aug2,
			aug3,
			aug4,
			aug5,
			aug6
		);

		return false;
	}

	// add any validated augments
	for (int iter = EQ::invaug::SOCKET_BEGIN; iter <= EQ::invaug::SOCKET_END; ++iter) {
		if(augments[iter]) {
			inst->PutAugment(&database, iter, augments[iter]);
		}
	}

	// attune item
	if(attuned && inst->GetItem()->Attuneable) {
		inst->SetAttuned(true);
	}

	inst->SetOrnamentIcon(ornament_icon);
	inst->SetOrnamentationIDFile(ornament_idfile);
	inst->SetOrnamentHeroModel(ornament_hero_model);

	// check to see if item is usable in requested slot
	if (enforce_usable && (to_slot >= EQ::invslot::EQUIPMENT_BEGIN && to_slot <= EQ::invslot::EQUIPMENT_END)) {
		uint32 slottest = to_slot;
		if(!(slots & ((uint32)1 << slottest))) {
			Message(
				Chat::White,
				fmt::format(
					"{} ({}) cannot be equipped in {} ({}), moving to cursor.",
					database.CreateItemLink(item->ID),
					item->ID,
					EQ::invslot::GetInvPossessionsSlotName(to_slot),
					to_slot
				).c_str()
			);
			LogInventory(
				"Player [{}] on account [{}] attempted to equip an item unusable in slot [{}] - moved to cursor.\n"
				"Item [{}], Aug1: [{}], Aug2: [{}], Aug3: [{}], Aug4: [{}], Aug5: [{}], Aug6: [{}])\n",
				GetName(),
				account_name,
				to_slot,
				item->ID,
				aug1,
				aug2,
				aug3,
				aug4,
				aug5,
				aug6
			);
			to_slot = EQ::invslot::slotCursor;
		}
	}

	if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::ITEM_CREATION)) {
		auto e = PlayerEvent::ItemCreationEvent{};
		e.item_id      = item->ID;
		e.item_name    = item->Name;
		e.to_slot      = to_slot;
		e.charges      = charges;
		e.augment_1_id = aug1;
		e.augment_2_id = aug2;
		e.augment_3_id = aug3;
		e.augment_4_id = aug4;
		e.augment_5_id = aug5;
		e.augment_6_id = aug6;
		e.attuned      = attuned;

		RecordPlayerEventLog(PlayerEvent::ITEM_CREATION, e);
	}

	// put item into inventory
	if (to_slot == EQ::invslot::slotCursor) {
		PushItemOnCursor(*inst);
		SendItemPacket(EQ::invslot::slotCursor, inst, ItemPacketLimbo);
	} else {
		PutItemInInventory(to_slot, *inst, true);
	}

	m_external_handin_items_returned.emplace_back(inst->GetItem()->ID);

	safe_delete(inst);

	// discover item and any augments
	if (
		RuleB(Character, EnableDiscoveredItems) &&
		!GetGM() &&
		!IsDiscovered(item_id)
	) {
		DiscoverItem(item_id);
	}

	return true;
}

// Drop item from inventory to ground (generally only dropped from SLOT_CURSOR)
void Client::DropItem(int16 slot_id, bool recurse)
{
	LogInventory(
		"[{}] (char_id: [{}]) Attempting to drop item from slot [{}] on the ground",
		GetCleanName(),
		CharacterID(),
		slot_id
	);

	if (GetInv().CheckNoDrop(slot_id, recurse) && !CanTradeFVNoDropItem()) {
		auto invalid_drop = m_inv.GetItem(slot_id);
		if (!invalid_drop) {
			LogInventory("Error in InventoryProfile::CheckNoDrop() - returned 'true' for empty slot");
		}
		else {
			if (EQEmuLogSys::Instance()->log_settings[Logs::Inventory].is_category_enabled) {
				LogInventory("DropItem() Hack detected - full item parse:");
				LogInventory("depth: 0, Item: [{}] (id: [{}]), IsDroppable: [{}]",
					(invalid_drop->GetItem() ? invalid_drop->GetItem()->Name : "null data"), invalid_drop->GetID(), (invalid_drop->IsDroppable(false) ? "true" : "false"));

				for (auto iter1 : *invalid_drop->GetContents()) { // depth 1
					LogInventory("-depth: 1, Item: [{}] (id: [{}]), IsDroppable: [{}]",
						(iter1.second->GetItem() ? iter1.second->GetItem()->Name : "null data"), iter1.second->GetID(), (iter1.second->IsDroppable(false) ? "true" : "false"));

					for (auto iter2 : *iter1.second->GetContents()) { // depth 2
						LogInventory("--depth: 2, Item: [{}] (id: [{}]), IsDroppable: [{}]",
							(iter2.second->GetItem() ? iter2.second->GetItem()->Name : "null data"), iter2.second->GetID(), (iter2.second->IsDroppable(false) ? "true" : "false"));
					}
				}
			}
		}

		std::string message = fmt::format(
			"Tried to drop an item on the ground that was no-drop! item_name [{}] item_id ({})",
			invalid_drop->GetItem()->Name,
			invalid_drop->GetItem()->ID
		);

		invalid_drop = nullptr;
		RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});
		GetInv().DeleteItem(slot_id);
		return;
	}

	// Take control of item in client inventory
	auto* inst = m_inv.PopItem(slot_id);
	if (inst) {
		if (EQEmuLogSys::Instance()->log_settings[Logs::Inventory].is_category_enabled) {
			LogInventory("DropItem() Processing - full item parse:");
			LogInventory(
				"depth: 0, Item: [{}] (id: [{}]), IsDroppable: [{}]",
				(inst->GetItem() ? inst->GetItem()->Name : "null data"),
				inst->GetID(),
				(inst->IsDroppable(false) ? "true" : "false")
			);

			if (!inst->IsDroppable(false)) {
				LogError("Non-droppable item being processed for drop by [{}]", GetCleanName());
			}

			for (auto iter1 : *inst->GetContents()) { // depth 1
				LogInventory(
					"-depth: 1, Item: [{}] (id: [{}]), IsDroppable: [{}]",
					(iter1.second->GetItem() ? iter1.second->GetItem()->Name : "null data"),
					iter1.second->GetID(),
					(iter1.second->IsDroppable(false) ? "true" : "false")
				);

				if (!iter1.second->IsDroppable(false)) {
					LogError("Non-droppable item being processed for drop by [{}]", GetCleanName());
				}

				for (auto iter2 : *iter1.second->GetContents()) { // depth 2
					LogInventory(
						"--depth: 2, Item: [{}] (id: [{}]), IsDroppable: [{}]",
						(iter2.second->GetItem() ? iter2.second->GetItem()->Name : "null data"),
						iter2.second->GetID(),
						(iter2.second->IsDroppable(false) ? "true" : "false")
					);

					if (!iter2.second->IsDroppable(false)) {
						LogError("Non-droppable item being processed for drop by [{}]", GetCleanName());
					}
				}
			}
		}


		int i = 0;

		if (inst && PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::DROPPED_ITEM)) {
			auto e = PlayerEvent::DroppedItemEvent{
				.item_id      = inst->GetID(),
				.augment_1_id = inst->GetAugmentItemID(0),
				.augment_2_id = inst->GetAugmentItemID(1),
				.augment_3_id = inst->GetAugmentItemID(2),
				.augment_4_id = inst->GetAugmentItemID(3),
				.augment_5_id = inst->GetAugmentItemID(4),
				.augment_6_id = inst->GetAugmentItemID(5),
				.item_name    = inst->GetItem()->Name,
				.slot_id      = slot_id,
				.charges      = (uint32) inst->GetCharges()
			};
			RecordPlayerEventLog(PlayerEvent::DROPPED_ITEM, e);
		}

		if (parse->ItemHasQuestSub(inst, EVENT_DROP_ITEM)) {
			parse->EventItem(EVENT_DROP_ITEM, this, inst, nullptr, "", slot_id);
			if (i != 0) {
				LogInventory("Item drop handled by [EVENT_DROP_ITEM]");
				safe_delete(inst);
			}
		}

		if (parse->PlayerHasQuestSub(EVENT_DROP_ITEM_CLIENT)) {
			std::vector<std::any> args = { inst };

			i = parse->EventPlayer(EVENT_DROP_ITEM_CLIENT, this, "", slot_id, &args);
			if (i != 0) {
				LogInventory("Item drop handled by [EVENT_DROP_ITEM_CLIENT]");
				safe_delete(inst);
			}
		}
	} else {
		// Item doesn't exist in inventory!
		LogInventory("DropItem() - No item found in slot [{}]", slot_id);
		Message(
			Chat::Red,
			fmt::format(
				"Error: Item not found in slot {}.",
				slot_id
			).c_str()
		);
		return;
	}

	// Save client inventory change to database
	if (slot_id == EQ::invslot::slotCursor) {
		SendCursorBuffer();
		auto s = m_inv.cursor_cbegin(), e = m_inv.cursor_cend();
		database.SaveCursor(CharacterID(), s, e);
	} else {
		database.SaveInventory(CharacterID(), nullptr, slot_id);
	}

	if (!inst) {
		return;
	}

	// Package as zone object
	auto object = new Object(this, inst);
	entity_list.AddObject(object, true);
	object->StartDecay();

	LogInventory("[{}] dropped [{}] from slot [{}]", GetCleanName(), inst->GetItem()->Name, slot_id);

	safe_delete(inst);
}

// Drop inst
void Client::DropInst(const EQ::ItemInstance* inst)
{
	if (!inst) {
		// Item doesn't exist in inventory!
		Message(Chat::Red, "Error: Item not found");
		return;
	}


	if (inst->GetItem()->NoDrop == 0)
	{
		Message(Chat::Red, "This item is NODROP. Deleting.");
		return;
	}

	// Package as zone object
	auto object = new Object(this, inst);
	entity_list.AddObject(object, true);
	object->StartDecay();
}

// Returns a slot's item ID (returns INVALID_ID if not found)
int32 Client::GetItemIDAt(int16 slot_id) {
	if (slot_id <= EQ::invslot::POSSESSIONS_END && slot_id >= EQ::invslot::POSSESSIONS_BEGIN) {
		if ((((uint64)1 << slot_id) & GetInv().GetLookup()->PossessionsBitmask) == 0)
			return INVALID_ID;
	}
	else if (slot_id <= EQ::invbag::GENERAL_BAGS_END && slot_id >= EQ::invbag::GENERAL_BAGS_BEGIN) {
		auto temp_slot = EQ::invslot::GENERAL_BEGIN + ((slot_id - EQ::invbag::GENERAL_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT);
		if ((((uint64)1 << temp_slot) & GetInv().GetLookup()->PossessionsBitmask) == 0)
			return INVALID_ID;
	}
	else if (slot_id <= EQ::invslot::BANK_END && slot_id >= EQ::invslot::BANK_BEGIN) {
		if ((slot_id - EQ::invslot::BANK_BEGIN) >= GetInv().GetLookup()->InventoryTypeSize.Bank)
			return INVALID_ID;
	}
	else if (slot_id <= EQ::invbag::BANK_BAGS_END && slot_id >= EQ::invbag::BANK_BAGS_BEGIN) {
		auto temp_slot = (slot_id - EQ::invbag::BANK_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT;
		if (temp_slot >= GetInv().GetLookup()->InventoryTypeSize.Bank)
			return INVALID_ID;
	}

	const EQ::ItemInstance* inst = m_inv[slot_id];
	if (inst)
		return inst->GetItem()->ID;

	// None found
	return INVALID_ID;
}

// Returns an augment's ID that's in an item (returns INVALID_ID if not found)
// Pass in the slot ID of the item and which augslot you want to check (0-5)
int32 Client::GetAugmentIDAt(int16 slot_id, uint8 augslot) {
	if (slot_id <= EQ::invslot::POSSESSIONS_END && slot_id >= EQ::invslot::POSSESSIONS_BEGIN) {
		if ((((uint64)1 << slot_id) & GetInv().GetLookup()->PossessionsBitmask) == 0)
			return INVALID_ID;
	}
	else if (slot_id <= EQ::invbag::GENERAL_BAGS_END && slot_id >= EQ::invbag::GENERAL_BAGS_BEGIN) {
		auto temp_slot = EQ::invslot::GENERAL_BEGIN + ((slot_id - EQ::invbag::GENERAL_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT);
		if ((((uint64)1 << temp_slot) & GetInv().GetLookup()->PossessionsBitmask) == 0)
			return INVALID_ID;
	}
	else if (slot_id <= EQ::invslot::BANK_END && slot_id >= EQ::invslot::BANK_BEGIN) {
		if ((slot_id - EQ::invslot::BANK_BEGIN) >= GetInv().GetLookup()->InventoryTypeSize.Bank)
			return INVALID_ID;
	}
	else if (slot_id <= EQ::invbag::BANK_BAGS_END && slot_id >= EQ::invbag::BANK_BAGS_BEGIN) {
		auto temp_slot = (slot_id - EQ::invbag::BANK_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT;
		if (temp_slot >= GetInv().GetLookup()->InventoryTypeSize.Bank)
			return INVALID_ID;
	}

	const EQ::ItemInstance* inst = m_inv[slot_id];
	if (inst && inst->GetAugmentItemID(augslot)) {
		return inst->GetAugmentItemID(augslot);
	}

	// None found
	return INVALID_ID;
}

void Client::SendCursorBuffer()
{
	// Temporary work-around for the RoF+ Client Buffer
	// Instead of dealing with client moving items in cursor buffer,
	// we can just send the next item in the cursor buffer to the cursor.
	if (ClientVersion() < EQ::versions::ClientVersion::RoF) { return; }
	if (GetInv().CursorEmpty()) { return; }

	auto test_inst = GetInv().GetCursorItem();
	if (test_inst == nullptr) { return; }
	auto test_item = test_inst->GetItem();
	if (test_item == nullptr) { return; }

	bool lore_pass = true;
	if (test_item->LoreGroup == -1) {
		lore_pass = (m_inv.HasItem(test_item->ID, 0, ~(invWhereSharedBank | invWhereCursor)) == INVALID_INDEX);
	}
	else if (test_item->LoreGroup != 0) {
		lore_pass = (m_inv.HasItemByLoreGroup(test_item->LoreGroup, ~(invWhereSharedBank | invWhereCursor)) == INVALID_INDEX);
	}

	if (!lore_pass) {
		LogInventory("([{}]) Duplicate lore items are not allowed - destroying item [{}](id:[{}]) on cursor",
			GetName(), test_item->Name, test_item->ID);
		MessageString(Chat::Loot, 290);

		if (parse->ItemHasQuestSub(test_inst, EVENT_DESTROY_ITEM)) {
			parse->EventItem(EVENT_DESTROY_ITEM, this, test_inst, nullptr, "", 0);
		}

		if (parse->PlayerHasQuestSub(EVENT_DESTROY_ITEM_CLIENT)) {
			std::vector<std::any> args = { test_inst };
			parse->EventPlayer(EVENT_DESTROY_ITEM_CLIENT, this, "", 0, &args);
		}

		DeleteItemInInventory(EQ::invslot::slotCursor);
		SendCursorBuffer();
	}
	else {
		SendItemPacket(EQ::invslot::slotCursor, test_inst, ItemPacketLimbo);
	}
}

// Remove item from inventory
void Client::DeleteItemInInventory(int16 slot_id, int16 quantity, bool client_update, bool update_db) {
	#if (EQDEBUG >= 5)
		LogDebug("DeleteItemInInventory([{}], [{}], [{}])", slot_id, quantity, (client_update) ? "true":"false");
	#endif

	// Added 'IsSlotValid(slot_id)' check to both segments of client packet processing.
	// - cursor queue slots were slipping through and crashing client
	if(!m_inv[slot_id]) {
		// Make sure the client deletes anything in this slot to match the server.
		if(client_update && IsValidSlot(slot_id)) {
			EQApplicationPacket* outapp;
			outapp = new EQApplicationPacket(OP_DeleteItem, sizeof(DeleteItem_Struct));
			DeleteItem_Struct* delitem	= (DeleteItem_Struct*)outapp->pBuffer;
			delitem->from_slot			= slot_id;
			delitem->to_slot			= 0xFFFFFFFF;
			delitem->number_in_stack	= 0xFFFFFFFF;
			QueuePacket(outapp);
			safe_delete(outapp);
		}
		return;
	}

	uint64 evolve_id = m_inv[slot_id]->GetEvolveUniqueID();
	bool   isDeleted = m_inv.DeleteItem(slot_id, quantity);
	if (isDeleted && evolve_id && (slot_id > EQ::invslot::TRADE_END || slot_id < EQ::invslot::TRADE_BEGIN)) {
		CharacterEvolvingItemsRepository::SoftDelete(database, evolve_id);
	}

	const EQ::ItemInstance* inst = nullptr;
	if (slot_id == EQ::invslot::slotCursor) {
		auto s = m_inv.cursor_cbegin(), e = m_inv.cursor_cend();
		if(update_db)
			database.SaveCursor(character_id, s, e);
	}
	else {
		// Save change to database
		inst = m_inv[slot_id];
		if(update_db)
			database.SaveInventory(character_id, inst, slot_id);
	}

	if(client_update && IsValidSlot(slot_id)) {
		EQApplicationPacket* outapp = nullptr;
		if(inst) {
			if (!inst->IsStackable() && !isDeleted) {
				// Non stackable item with charges = Item with clicky spell effect ? Delete a charge.
				outapp = new EQApplicationPacket(OP_DeleteCharge, sizeof(MoveItem_Struct));
			}
			else {
				// Stackable, arrows, etc ? Delete one from the stack
				outapp = new EQApplicationPacket(OP_DeleteItem, sizeof(MoveItem_Struct));
			}

			DeleteItem_Struct* delitem	= (DeleteItem_Struct*)outapp->pBuffer;
			delitem->from_slot			= slot_id;
			delitem->to_slot			= 0xFFFFFFFF;
			delitem->number_in_stack	= 0xFFFFFFFF;

			for(int loop=0;loop<quantity;loop++)
				QueuePacket(outapp);
			safe_delete(outapp);
		}
		else {
			outapp = new EQApplicationPacket(OP_MoveItem, sizeof(MoveItem_Struct));
			MoveItem_Struct* delitem	= (MoveItem_Struct*)outapp->pBuffer;
			delitem->from_slot			= slot_id;
			delitem->to_slot			= 0xFFFFFFFF;
			delitem->number_in_stack	= 0xFFFFFFFF;

			QueuePacket(outapp);
			safe_delete(outapp);
		}
	}
}

bool Client::PushItemOnCursor(const EQ::ItemInstance& inst, bool client_update)
{
	LogInventory("Putting item [{}] ([{}]) on the cursor", inst.GetItem()->Name, inst.GetItem()->ID);

	EvolvingItemsManager::Instance()->DoLootChecks(CharacterID(), EQ::invslot::slotCursor, inst);
	m_inv.PushCursor(inst);

	if (client_update) {
		SendItemPacket(EQ::invslot::slotCursor, &inst, ItemPacketLimbo);
	}

	auto s = m_inv.cursor_cbegin(), e = m_inv.cursor_cend();
	return database.SaveCursor(CharacterID(), s, e);
}

// Puts an item into the person's inventory
// Any items already there will be removed from user's inventory
// (Also saves changes back to the database: this may be optimized in the future)
// client_update: Sends packet to client
bool Client::PutItemInInventory(int16 slot_id, const EQ::ItemInstance& inst, bool client_update) {
	LogInventory("Putting item [{}] ([{}]) into slot [{}]", inst.GetItem()->Name, inst.GetItem()->ID, slot_id);

	if (slot_id == EQ::invslot::slotCursor) { // don't trust macros before conditional statements...
		return PushItemOnCursor(inst, client_update);
	}

	EvolvingItemsManager::Instance()->DoLootChecks(CharacterID(), slot_id, inst);
	m_inv.PutItem(slot_id, inst);

	if (client_update)
	{
		SendItemPacket(slot_id, &inst, ((slot_id == EQ::invslot::slotCursor) ? ItemPacketLimbo : ItemPacketTrade));
		//SendWearChange(EQ::InventoryProfile::CalcMaterialFromSlot(slot_id));
	}

	CalcBonuses();

	if (slot_id == EQ::invslot::slotCursor) {
		auto s = m_inv.cursor_cbegin(), e = m_inv.cursor_cend();
		return database.SaveCursor(CharacterID(), s, e);
	}

	return database.SaveInventory(CharacterID(), &inst, slot_id);

	//CalcBonuses(); // this never fires??
	// a lot of wasted checks and calls coded above...
}

void Client::PutLootInInventory(int16 slot_id, const EQ::ItemInstance &inst, LootItem** bag_item_data)
{
	LogInventory("Putting loot item [{}] ([{}]) into slot [{}]", inst.GetItem()->Name, inst.GetItem()->ID, slot_id);

	bool cursor_empty = m_inv.CursorEmpty();

	EvolvingItemsManager::Instance()->DoLootChecks(CharacterID(), slot_id, inst);

	if (slot_id == EQ::invslot::slotCursor) {
		m_inv.PushCursor(inst);
		auto s = m_inv.cursor_cbegin(), e = m_inv.cursor_cend();
		database.SaveCursor(CharacterID(), s, e);
	}
	else {
		m_inv.PutItem(slot_id, inst);
		database.SaveInventory(CharacterID(), &inst, slot_id);
	}

	// Subordinate items in cursor buffer must be sent via ItemPacketSummonItem or we just overwrite the visible cursor and desync the client
	if (slot_id == EQ::invslot::slotCursor && !cursor_empty) {
		// RoF+ currently has a specialized cursor handler
		if (ClientVersion() < EQ::versions::ClientVersion::RoF)
			SendItemPacket(slot_id, &inst, ItemPacketLimbo);
	}
	else {
		SendLootItemInPacket(&inst, slot_id);
	}

	if (bag_item_data) {
		for (int index = EQ::invbag::SLOT_BEGIN; index <= EQ::invbag::SLOT_END; ++index) {
			if (bag_item_data[index] == nullptr)
				continue;

			const EQ::ItemInstance *bagitem = database.CreateItem(
				bag_item_data[index]->item_id,
				bag_item_data[index]->charges,
				bag_item_data[index]->aug_1,
				bag_item_data[index]->aug_2,
				bag_item_data[index]->aug_3,
				bag_item_data[index]->aug_4,
				bag_item_data[index]->aug_5,
				bag_item_data[index]->aug_6,
				bag_item_data[index]->attuned,
				bag_item_data[index]->custom_data,
				bag_item_data[index]->ornamenticon,
				bag_item_data[index]->ornamentidfile,
				bag_item_data[index]->ornament_hero_model
				);

			// Dump bag contents to cursor in the event that owning bag is not the first cursor item
			// (This assumes that the data passed is correctly associated..no safety checks are implemented)
			if (slot_id == EQ::invslot::slotCursor && !cursor_empty) {
				LogInventory("Putting bag loot item [{}] ([{}]) into slot [{}] (non-empty cursor override)",
					inst.GetItem()->Name, inst.GetItem()->ID, EQ::invslot::slotCursor);

				PutLootInInventory(EQ::invslot::slotCursor, *bagitem);
			}
			else {
				auto bag_slot = EQ::InventoryProfile::CalcSlotId(slot_id, index);

				LogInventory("Putting bag loot item [{}] ([{}]) into slot [{}] (bag slot [{}])",
					inst.GetItem()->Name, inst.GetItem()->ID, bag_slot, index);

				PutLootInInventory(bag_slot, *bagitem);
			}
			safe_delete(bagitem);
		}
	}

	CalcBonuses();
}
bool Client::TryStacking(EQ::ItemInstance* item, uint8 type, bool try_worn, bool try_cursor) {
	if(!item || !item->IsStackable() || item->GetCharges()>=item->GetItem()->StackSize)
		return false;
	int16 i;
	uint32 item_id = item->GetItem()->ID;
	for (i = EQ::invslot::GENERAL_BEGIN; i <= EQ::invslot::GENERAL_END; i++) {
		if ((((uint64)1 << i) & GetInv().GetLookup()->PossessionsBitmask) == 0)
			continue;

		EQ::ItemInstance* tmp_inst = m_inv.GetItem(i);
		if(tmp_inst && tmp_inst->GetItem()->ID == item_id && tmp_inst->GetCharges() < tmp_inst->GetItem()->StackSize){
			MoveItemCharges(*item, i, type);
			CalcBonuses();
			if (item->GetCharges()) { // we didn't get them all
				return AutoPutLootInInventory(*item, try_worn, try_cursor, 0);
			}
			return true;
		}
	}
	for (i = EQ::invslot::GENERAL_BEGIN; i <= EQ::invslot::GENERAL_END; i++) {
		if ((((uint64)1 << i) & GetInv().GetLookup()->PossessionsBitmask) == 0)
			continue;

		for (uint8 j = EQ::invbag::SLOT_BEGIN; j <= EQ::invbag::SLOT_END; j++) {
			uint16 slotid = EQ::InventoryProfile::CalcSlotId(i, j);
			EQ::ItemInstance* tmp_inst = m_inv.GetItem(slotid);

			if(tmp_inst && tmp_inst->GetItem()->ID == item_id && tmp_inst->GetCharges() < tmp_inst->GetItem()->StackSize) {
				MoveItemCharges(*item, slotid, type);
				CalcBonuses();
				if (item->GetCharges()) { // we didn't get them all
					return AutoPutLootInInventory(*item, try_worn, try_cursor, 0);
				}
				return true;
			}
		}
	}
	return false;
}

// Locate an available space in inventory to place an item
// and then put the item there
// The change will be saved to the database
bool Client::AutoPutLootInInventory(EQ::ItemInstance& inst, bool try_worn, bool try_cursor, LootItem** bag_item_data)
{
	// #1: Try to auto equip
	if (try_worn && inst.IsEquipable(GetBaseRace(), GetClass()) && inst.GetItem()->ReqLevel <= level && (!inst.GetItem()->Attuneable || inst.IsAttuned()) && inst.GetItem()->ItemType != EQ::item::ItemTypeAugmentation) {
		for (int16 i = EQ::invslot::EQUIPMENT_BEGIN; i <= EQ::invslot::EQUIPMENT_END; i++) {
			if ((((uint64)1 << i) & GetInv().GetLookup()->PossessionsBitmask) == 0)
				continue;

			if (!m_inv[i]) {
				if (i == EQ::invslot::slotPrimary && inst.IsWeapon()) { // If item is primary slot weapon
					if (inst.GetItem()->IsType2HWeapon()) { // and uses 2hs \ 2hb \ 2hp
						if (m_inv[EQ::invslot::slotSecondary]) { // and if secondary slot is not empty
							continue; // Can't auto-equip
						}
					}
				}
				if( i == EQ::invslot::slotPrimary && m_inv[EQ::invslot::slotSecondary] ) {
					uint8 instrument = m_inv[EQ::invslot::slotSecondary]->GetItem()->ItemType;
					if(
							instrument == EQ::item::ItemTypeWindInstrument ||
							instrument == EQ::item::ItemTypeStringedInstrument ||
							instrument == EQ::item::ItemTypeBrassInstrument ||
							instrument == EQ::item::ItemTypePercussionInstrument
							) {
						LogInventory("Cannot equip a primary item with [{}] already in the secondary.", m_inv[EQ::invslot::slotSecondary]->GetItem()->Name);
						continue; // Do not auto-equip Primary when instrument is in Secondary
					}
				}
				if (i == EQ::invslot::slotSecondary && m_inv[EQ::invslot::slotPrimary]) { // check to see if primary slot is a two hander
					uint8 instrument = inst.GetItem()->ItemType;
					if(
							instrument == EQ::item::ItemTypeWindInstrument ||
							instrument == EQ::item::ItemTypeStringedInstrument ||
							instrument == EQ::item::ItemTypeBrassInstrument ||
							instrument == EQ::item::ItemTypePercussionInstrument
							) {
						LogInventory("Cannot equip a secondary instrument with [{}] already in the primary.", m_inv[EQ::invslot::slotPrimary]->GetItem()->Name);
						continue; // Do not auto-equip instrument in Secondary when Primary is equipped.
					}

					uint8 use = m_inv[EQ::invslot::slotPrimary]->GetItem()->ItemType;
					if(use == EQ::item::ItemType2HSlash || use == EQ::item::ItemType2HBlunt || use == EQ::item::ItemType2HPiercing) {
						continue;
					}
				}
				if (i == EQ::invslot::slotSecondary && inst.IsWeapon() && !CanThisClassDualWield()) {
					continue;
				}

				if (inst.IsEquipable(i)) { // Equippable at this slot?
					//send worn to everyone...
					PutLootInInventory(i, inst);
					uint8 worn_slot_material = EQ::InventoryProfile::CalcMaterialFromSlot(i);
					if (worn_slot_material != EQ::textures::materialInvalid) {
						SendWearChange(worn_slot_material);
					}

					if (parse->ItemHasQuestSub(&inst, EVENT_EQUIP_ITEM)) {
						parse->EventItem(EVENT_EQUIP_ITEM, this, &inst, nullptr, "", i);
					}

					return true;
				}
			}
		}
	}

	// #2: Stackable item?
	if (inst.IsStackable()) {
		if (TryStacking(&inst, ItemPacketTrade, try_worn, try_cursor))
			return true;
	}

	// #3: put it in inventory
	bool is_arrow = (inst.GetItem()->ItemType == EQ::item::ItemTypeArrow) ? true : false;
	int16 slot_id = m_inv.FindFreeSlot(inst.IsClassBag(), try_cursor, inst.GetItem()->Size, is_arrow);
	if (slot_id != INVALID_INDEX) {
		PutLootInInventory(slot_id, inst, bag_item_data);
		return true;
	}

	return false;
}

// helper function for AutoPutLootInInventory
void Client::MoveItemCharges(EQ::ItemInstance &from, int16 to_slot, uint8 type)
{
	EQ::ItemInstance *tmp_inst = m_inv.GetItem(to_slot);

	if(tmp_inst && tmp_inst->GetCharges() < tmp_inst->GetItem()->StackSize) {
		// this is how much room is left on the item we're stacking onto
		int charge_slots_left = tmp_inst->GetItem()->StackSize - tmp_inst->GetCharges();
		// this is how many charges we can move from the looted item to
		// the item in the inventory
		int charges_to_move = (from.GetCharges() < charge_slots_left) ? from.GetCharges() : charge_slots_left;

		tmp_inst->SetCharges(tmp_inst->GetCharges() + charges_to_move);
		from.SetCharges(from.GetCharges() - charges_to_move);
		SendLootItemInPacket(tmp_inst, to_slot);
		if (to_slot == EQ::invslot::slotCursor) {
			auto s = m_inv.cursor_cbegin(), e = m_inv.cursor_cend();
			database.SaveCursor(CharacterID(), s, e);
		}
		else {
			database.SaveInventory(CharacterID(), tmp_inst, to_slot);
		}
	}
}

#if 0
// TODO: needs clean-up to save references
bool MakeItemLink(char* &ret_link, const ItemData *item, uint32 aug0, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5, uint8 evolving, uint8 evolvedlevel) {
	//we're sending back the entire "link", minus the null characters & item name
	//that way, we can use it for regular links & Task links
	//note: initiator needs to pass us ret_link

	/*
	--- Usage ---
	Chat: "%c" "%s" "%s" "%c", 0x12, ret_link, inst->GetItem()->name, 0x12
	Task: "<a WndNotify=\"27," "%s" "\">" "%s" "</a>", ret_link, inst->GetItem()->name
	<a WndNotify="27,00960F000000000000000000000000000000000000000">Master's Book of Wood Elven Culture</a>
	http://eqitems.13th-floor.org/phpBB2/viewtopic.php?p=510#510
	*/

	if (!item) //have to have an item to make the link
		return false;

	//format:
	//0	itemid	aug1	aug2	aug3	aug4	aug5	evolving?	loregroup	evolved level	hash
	//0	00000	00000	00000	00000	00000	00000	0			0000		0				00000000
	//length:
	//1	5		5		5		5		5		5		1			4			1				8		= 45
	//evolving item info: http://eqitems.13th-floor.org/phpBB2/viewtopic.php?t=145#558

	//int hash = GetItemLinkHash(inst);	//eventually this will work (currently crashes zone), but for now we'll skip the extra overhead
	int hash = NOT_USED;

	// Tested with UF and RoF..there appears to be a problem with using non-augment arguments below...
	// Currently, enabling them causes misalignments in what the client expects. I haven't looked
	// into it further to determine the cause..but, the function is setup to accept the parameters.
	// Note: some links appear with '00000' in front of the name..so, it's likely we need to send
	// some additional information when certain parameters are true
	//switch (GetClientVersion()) {
	switch (0) {
	case EQClientRoF2:
		// This operator contains 14 parameter masks..but, only 13 parameter values.
		// Even so, the client link appears ok... Need to figure out the discrepancy
		MakeAnyLenString(&ret_link, "%1X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%1X" "%1X" "%04X" "%1X" "%05X" "%08X",
			0,
			item->ID,
			aug0,
			aug1,
			aug2,
			aug3,
			aug4,
			aug5,
			0,//evolving,
			0,//item->LoreGroup,
			0,//evolvedlevel,
			0,
			hash
			);
		return true;
	case EQClientRoF:
		MakeAnyLenString(&ret_link, "%1X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%1X" "%04X" "%1X" "%05X" "%08X",
			0,
			item->ID,
			aug0,
			aug1,
			aug2,
			aug3,
			aug4,
			aug5,
			0,//evolving,
			0,//item->LoreGroup,
			0,//evolvedlevel,
			0,
			hash
			);
		return true;
	case EQClientUnderfoot:
	case EQClientSoD:
	case EQClientSoF:
		MakeAnyLenString(&ret_link, "%1X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%1X" "%04X" "%1X" "%05X" "%08X",
			0,
			item->ID,
			aug0,
			aug1,
			aug2,
			aug3,
			aug4,
			0,//evolving,
			0,//item->LoreGroup,
			0,//evolvedlevel,
			0,
			hash
			);
		return true;
	case EQClientTitanium:
		MakeAnyLenString(&ret_link, "%1X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%1X" "%04X" "%1X" "%08X",
			0,
			item->ID,
			aug0,
			aug1,
			aug2,
			aug3,
			aug4,
			0,//evolving,
			0,//item->LoreGroup,
			0,//evolvedlevel,
			hash
			);
		return true;
	case EQClient62:
	default:
		return false;
	}
}
#endif

int Client::GetItemLinkHash(const EQ::ItemInstance* inst) {
#if 0
	//pre-Titanium: http://eqitems.13th-floor.org/phpBB2/viewtopic.php?t=70&postdays=0&postorder=asc
	//Titanium: http://eqitems.13th-floor.org/phpBB2/viewtopic.php?t=145
	if (!inst)	//have to have an item to make the hash
		return 0;

	const EQ::ItemData* item = inst->GetItem();
	char* hash_str = 0;
	/*register */int hash = 0;

	//now the fun part, since different types of items use different hashes...
	if (item->ItemClass == 0 && item->CharmFileID) {	//charm
		MakeAnyLenString(&hash_str, "%d%s-1-1-1-1-1%d %d %d %d %d %d %d %d %d",
			item->ID,
			item->Name,
			item->Light,
			item->Icon,
			item->Price,
			item->Size,
			item->Weight,
			item->ItemClass,
			item->ItemType,
			item->Favor,
			item->GuildFavor);
	} else if (item->ItemClass == 2) {	//book
		MakeAnyLenString(&hash_str, "%d%s%d%d%09X",
			item->ID,
			item->Name,
			item->Weight,
			item->BookType,
			item->Price);
	} else if (item->ItemClass == 1) {	//bag
		MakeAnyLenString(&hash_str, "%d%s%x%d%09X%d",
			item->ID,
			item->Name,
			item->BagSlots,
			item->BagWR,
			item->Price,
			item->Weight);
	} else {	//everything else
		MakeAnyLenString(&hash_str, "%d%s-1-1-1-1-1%d %d %d %d %d %d %d %d %d %d %d %d %d",
			item->ID,
			item->Name,
			item->Mana,
			item->HP,
			item->Favor,
			item->Light,
			item->Icon,
			item->Price,
			item->Weight,
			item->ReqLevel,
			item->Size,
			item->ItemClass,
			item->ItemType,
			item->AC,
			item->GuildFavor);
	}

	//this currently crashes zone, so someone feel free to fix this so we can work with hashes:
	//*** glibc detected *** double free or corruption (out): 0xb2403470 ***

	/*
	while (*hash_str != '\0') {
		register int c = toupper(*hash_str);

		asm volatile("\
			imul $31, %1, %1;\
			movzx %%ax, %%edx;\
			addl %%edx, %1;\
			movl %1, %0;\
			"
			:"=r"(hash)
			:"D"(hash), "a"(c)
			:"%edx"
			);

		// This is what the inline asm is doing:
		// hash *= 0x1f;
		// hash += (int)c;

		hash_str++;
	}
	*/

	safe_delete_array(hash_str);
	return hash;
#endif
	return 0;
}

// This appears to still be in use... The core of this should be incorporated into class EQ::SayLinkEngine
void Client::SendItemLink(const EQ::ItemInstance* inst, bool send_to_all)
{
/*

this stuff is old, live dosent do this anymore. they send a much smaller
packet with the item number in it, but I cant seem to find it right now

*/
	if (!inst)
		return;

	const EQ::ItemData* item = inst->GetItem();
	const char* name2 = &item->Name[0];
	auto outapp = new EQApplicationPacket(OP_ItemLinkText, strlen(name2) + 68);
	char buffer2[135] = {0};
	char itemlink[135] = {0};
	sprintf(itemlink,"%c0%06u0%05u-%05u-%05u-%05u-%05u00000000%c",
		0x12,
		item->ID,
		inst->GetAugmentItemID(0),
		inst->GetAugmentItemID(1),
		inst->GetAugmentItemID(2),
		inst->GetAugmentItemID(3),
		inst->GetAugmentItemID(4),
		0x12);
	sprintf(buffer2,"%c%c%c%c%c%c%c%c%c%c%c%c%s",0x00,0x00,0x00,0x00,0xD3,0x01,0x00,0x00,0x1E,0x01,0x00,0x00,itemlink);
	memcpy(outapp->pBuffer,buffer2,outapp->size);
	QueuePacket(outapp);
	safe_delete(outapp);
	if (send_to_all==false)
		return;
	const char* charname = GetName();
	outapp = new EQApplicationPacket(OP_ItemLinkText,strlen(itemlink)+14+strlen(charname));
	char buffer3[150] = {0};
	sprintf(buffer3,"%c%c%c%c%c%c%c%c%c%c%c%c%6s%c%s",0x00,0x00,0x00,0x00,0xD2,0x01,0x00,0x00,0x00,0x00,0x00,0x00,charname,0x00,itemlink);
	memcpy(outapp->pBuffer,buffer3,outapp->size);
	entity_list.QueueCloseClients(CastToMob(),outapp,true,200,0,false);
	safe_delete(outapp);
}

void Client::SendLootItemInPacket(const EQ::ItemInstance* inst, int16 slot_id)
{
	SendItemPacket(slot_id,inst, ItemPacketTrade);
}

bool Client::IsValidSlot(uint32 slot) {
	if (slot <= EQ::invslot::POSSESSIONS_END && slot >= EQ::invslot::POSSESSIONS_BEGIN) {
		return ((((uint64)1 << slot) & GetInv().GetLookup()->PossessionsBitmask) != 0);
	}
	else if (slot <= EQ::invbag::GENERAL_BAGS_END && slot >= EQ::invbag::GENERAL_BAGS_BEGIN) {
		auto temp_slot = EQ::invslot::GENERAL_BEGIN + ((slot - EQ::invbag::GENERAL_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT);
		return ((((uint64)1 << temp_slot) & GetInv().GetLookup()->PossessionsBitmask) != 0);
	}
	else if (slot <= EQ::invslot::BANK_END && slot >= EQ::invslot::BANK_BEGIN) {
		return ((slot - EQ::invslot::BANK_BEGIN) < GetInv().GetLookup()->InventoryTypeSize.Bank);
	}
	else if (slot <= EQ::invbag::BANK_BAGS_END && slot >= EQ::invbag::BANK_BAGS_BEGIN) {
		auto temp_slot = (slot - EQ::invbag::BANK_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT;
		return (temp_slot < GetInv().GetLookup()->InventoryTypeSize.Bank);
	}
	else if (
		(slot == (uint32)INVALID_INDEX) ||
		(slot == (uint32)EQ::invslot::slotCursor) ||
		(slot <= EQ::invbag::CURSOR_BAG_END && slot >= EQ::invbag::CURSOR_BAG_BEGIN) ||
		(slot <= EQ::invslot::TRIBUTE_END && slot >= EQ::invslot::TRIBUTE_BEGIN) ||
		(slot <= EQ::invslot::GUILD_TRIBUTE_END && slot >= EQ::invslot::GUILD_TRIBUTE_BEGIN) ||
		(slot <= EQ::invslot::SHARED_BANK_END && slot >= EQ::invslot::SHARED_BANK_BEGIN) ||
		(slot <= EQ::invbag::SHARED_BANK_BAGS_END && slot >= EQ::invbag::SHARED_BANK_BAGS_BEGIN) ||
		(slot <= EQ::invslot::TRADE_END && slot >= EQ::invslot::TRADE_BEGIN) ||
		(slot <= EQ::invslot::WORLD_END && slot >= EQ::invslot::WORLD_BEGIN)
	) {
		return true;
	}

	return false;
}

bool Client::IsBankSlot(uint32 slot)
{
	if ((slot >= EQ::invslot::BANK_BEGIN && slot <= EQ::invslot::BANK_END) ||
		(slot >= EQ::invbag::BANK_BAGS_BEGIN && slot <= EQ::invbag::BANK_BAGS_END) ||
		(slot >= EQ::invslot::SHARED_BANK_BEGIN && slot <= EQ::invslot::SHARED_BANK_END) ||
		(slot >= EQ::invbag::SHARED_BANK_BAGS_BEGIN && slot <= EQ::invbag::SHARED_BANK_BAGS_END))
	{
		return true;
	}

	return false;
}

// Moves items around both internally and in the database
// In the future, this can be optimized by pushing all changes through one database REPLACE call
bool Client::SwapItem(MoveItem_Struct* move_in) {

	uint32 src_slot_check = move_in->from_slot;
	uint32 dst_slot_check = move_in->to_slot;
	uint32 stack_count_check = move_in->number_in_stack;

	if(!IsValidSlot(src_slot_check)){
		// SoF+ sends a Unix timestamp (should be int32) for src and dst slots every 10 minutes for some reason.
		if(src_slot_check < 2147483647)
			Message(Chat::Red, "Warning: Invalid slot move from slot %u to slot %u with %u charges!", src_slot_check, dst_slot_check, stack_count_check);
		LogInventory("Invalid slot move from slot [{}] to slot [{}] with [{}] charges!", src_slot_check, dst_slot_check, stack_count_check);
		return false;
	}

	if(!IsValidSlot(dst_slot_check)) {
		// SoF+ sends a Unix timestamp (should be int32) for src and dst slots every 10 minutes for some reason.
		if(src_slot_check < 2147483647)
			Message(Chat::Red, "Warning: Invalid slot move from slot %u to slot %u with %u charges!", src_slot_check, dst_slot_check, stack_count_check);
		LogInventory("Invalid slot move from slot [{}] to slot [{}] with [{}] charges!", src_slot_check, dst_slot_check, stack_count_check);
		return false;
	}

	if (move_in->from_slot == move_in->to_slot) { // Item summon, no further processing needed
		if (ClientVersion() >= EQ::versions::ClientVersion::RoF) { return true; } // Can't do RoF+

		if (move_in->to_slot == EQ::invslot::slotCursor) {
			auto test_inst = m_inv.GetItem(EQ::invslot::slotCursor);
			if (test_inst == nullptr) { return true; }
			auto test_item = test_inst->GetItem();
			if (test_item == nullptr) { return true; }
			if (!test_item->LoreFlag) { return true; }

			bool lore_pass = true;
			if (test_item->LoreGroup == -1) {
				lore_pass = (m_inv.HasItem(test_item->ID, 0, ~(invWhereSharedBank | invWhereCursor)) == INVALID_INDEX);
			}
			else if (test_item->LoreGroup != 0) {
				lore_pass = (m_inv.HasItemByLoreGroup(test_item->LoreGroup, ~(invWhereSharedBank | invWhereCursor)) == INVALID_INDEX);
			}

			if (!lore_pass) {
				LogInventory("([{}]) Duplicate lore items are not allowed - destroying item [{}](id:[{}]) on cursor",
					GetName(), test_item->Name, test_item->ID);
				MessageString(Chat::Loot, 290);

				if (parse->ItemHasQuestSub(test_inst, EVENT_DESTROY_ITEM)) {
					parse->EventItem(EVENT_DESTROY_ITEM, this, test_inst, nullptr, "", 0);
				}

				if (parse->PlayerHasQuestSub(EVENT_DESTROY_ITEM_CLIENT)) {
					std::vector<std::any> args = { test_inst };
					parse->EventPlayer(EVENT_DESTROY_ITEM_CLIENT, this, "", 0, &args);
				}

				DeleteItemInInventory(EQ::invslot::slotCursor, 0, true);

				if (test_inst && PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::ITEM_DESTROY)) {
					auto e = PlayerEvent::DestroyItemEvent{
						.item_id      = test_inst->GetItem()->ID,
						.item_name    = test_inst->GetItem()->Name,
						.charges      = test_inst->GetCharges(),
						.augment_1_id = test_inst->GetAugmentItemID(0),
						.augment_2_id = test_inst->GetAugmentItemID(1),
						.augment_3_id = test_inst->GetAugmentItemID(2),
						.augment_4_id = test_inst->GetAugmentItemID(3),
						.augment_5_id = test_inst->GetAugmentItemID(4),
						.augment_6_id = test_inst->GetAugmentItemID(5),
						.attuned      = test_inst->IsAttuned(),
						.reason       = "Duplicate lore item"
					};

					RecordPlayerEventLog(PlayerEvent::ITEM_DESTROY, e);
				}

			}
		}
		return true;
	}

	if (move_in->to_slot == (uint32)INVALID_INDEX) {
		if (move_in->from_slot == (uint32)EQ::invslot::slotCursor) {
			LogInventory("Client destroyed item from cursor slot [{}]", move_in->from_slot);

			EQ::ItemInstance *inst = m_inv.GetItem(EQ::invslot::slotCursor);

			if (inst) {
				if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::ITEM_DESTROY)) {
					auto e = PlayerEvent::DestroyItemEvent{
						.item_id      = inst->GetItem()->ID,
						.item_name    = inst->GetItem()->Name,
						.charges      = inst->GetCharges(),
						.augment_1_id = inst->GetAugmentItemID(0),
						.augment_2_id = inst->GetAugmentItemID(1),
						.augment_3_id = inst->GetAugmentItemID(2),
						.augment_4_id = inst->GetAugmentItemID(3),
						.augment_5_id = inst->GetAugmentItemID(4),
						.augment_6_id = inst->GetAugmentItemID(5),
						.attuned      = inst->IsAttuned(),
						.reason       = "Client destroy cursor"
					};

					RecordPlayerEventLog(PlayerEvent::ITEM_DESTROY, e);
				}

				if (parse->ItemHasQuestSub(inst, EVENT_DESTROY_ITEM)) {
					parse->EventItem(EVENT_DESTROY_ITEM, this, inst, nullptr, "", 0);
				}

				if (parse->PlayerHasQuestSub(EVENT_DESTROY_ITEM_CLIENT)) {
					std::vector<std::any> args = { inst };
					parse->EventPlayer(EVENT_DESTROY_ITEM_CLIENT, this, "", 0, &args);
				}
			}

			DeleteItemInInventory(move_in->from_slot);
			SendCursorBuffer();

			return true; // Item destroyed by client
		}
		else {
			LogInventory("Deleted item from slot [{}] as a result of an inventory container tradeskill combine", move_in->from_slot);
			DeleteItemInInventory(move_in->from_slot);
			return true; // Item deletion
		}
	}

	if (auto_attack) {
		if (move_in->from_slot == EQ::invslot::slotPrimary || move_in->from_slot == EQ::invslot::slotSecondary || move_in->from_slot == EQ::invslot::slotRange)
			SetAttackTimer();
		else if (move_in->to_slot == EQ::invslot::slotPrimary || move_in->to_slot == EQ::invslot::slotSecondary || move_in->to_slot == EQ::invslot::slotRange)
			SetAttackTimer();
	}

	// Step 1: Variables
	int16 src_slot_id = (int16)move_in->from_slot;
	int16 dst_slot_id = (int16)move_in->to_slot;

	if(IsBankSlot(src_slot_id) || IsBankSlot(dst_slot_id) || IsBankSlot(src_slot_check) || IsBankSlot(dst_slot_check)) {
		uint32 distance = 0;
		NPC *banker = entity_list.GetClosestBanker(this, distance);

		if(!banker || distance > USE_NPC_RANGE2)
		{
			auto message = fmt::format(
				"Player tried to make use of a banker (items) but banker [{}] is "
				"non-existent or too far away [{}] units",
				banker ? banker->GetName() : "UNKNOWN NPC", distance
			);
			RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});

			Kick("Inventory desync");	// Kicking player to avoid item loss do to client and server inventories not being sync'd
			return false;
		}
	}

	//Setup
	uint32 srcitemid = 0;
	uint32 dstitemid = 0;
	EQ::ItemInstance* src_inst = m_inv.GetItem(src_slot_id);
	EQ::ItemInstance* dst_inst = m_inv.GetItem(dst_slot_id);
	if (src_inst){
		LogInventory("Src slot [{}] has item [{}] ([{}]) with [{}] charges in it", src_slot_id, src_inst->GetItem()->Name, src_inst->GetItem()->ID, src_inst->GetCharges());
		srcitemid = src_inst->GetItem()->ID;
		//SetTint(dst_slot_id,src_inst->GetColor());
		if (src_inst->GetCharges() > 0 && (src_inst->GetCharges() < (int16)move_in->number_in_stack || move_in->number_in_stack > src_inst->GetItem()->StackSize))
		{
			Message(Chat::Red,"Error: Insufficient number in stack.");
			return false;
		}
	}
	if (dst_inst) {
		LogInventory("Dest slot [{}] has item [{}] ([{}]) with [{}] charges in it", dst_slot_id, dst_inst->GetItem()->Name, dst_inst->GetItem()->ID, dst_inst->GetCharges());
		dstitemid = dst_inst->GetItem()->ID;
	}
	if (IsBuyer() && srcitemid > 0) {
		CheckIfMovedItemIsPartOfBuyLines(srcitemid);
	}

	if (IsTrader() && srcitemid>0){
		EQ::ItemInstance* srcbag;
		EQ::ItemInstance* dstbag;
		uint32 srcbagid =0;
		uint32 dstbagid = 0;

		if (src_slot_id >= EQ::invbag::GENERAL_BAGS_BEGIN && src_slot_id <= EQ::invbag::GENERAL_BAGS_END) {
			srcbag = m_inv.GetItem(EQ::InventoryProfile::CalcSlotId(src_slot_id));
			if (srcbag)
				srcbagid = srcbag->GetItem()->ID;
		}
		if (dst_slot_id >= EQ::invbag::GENERAL_BAGS_BEGIN && dst_slot_id <= EQ::invbag::GENERAL_BAGS_END) {
			dstbag = m_inv.GetItem(EQ::InventoryProfile::CalcSlotId(dst_slot_id));
			if (dstbag)
				dstbagid = dstbag->GetItem()->ID;
		}
		if ((srcbagid && srcbag->GetItem()->BagType == EQ::item::BagTypeTradersSatchel) ||
		    (dstbagid && dstbag->GetItem()->BagType == EQ::item::BagTypeTradersSatchel) ||
		    (srcitemid && src_inst && src_inst->GetItem()->BagType == EQ::item::BagTypeTradersSatchel) ||
		    (dstitemid && dst_inst && dst_inst->GetItem()->BagType == EQ::item::BagTypeTradersSatchel)) {
			TraderEndTrader();
			Message(Chat::Red,"You cannot move your Trader Satchels, or items inside them, while Trading.");
		}
	}

	// Step 2: Validate item in from_slot
	// After this, we can assume src_inst is a valid ptr
	if (!src_inst && (src_slot_id < EQ::invslot::WORLD_BEGIN || src_slot_id > EQ::invslot::WORLD_END)) {
		if (dst_inst) {
			// If there is no source item, but there is a destination item,
			// move the slots around before deleting the invalid source slot item,
			// which is now in the destination slot.
			move_in->from_slot = dst_slot_check;
			move_in->to_slot = src_slot_check;
			move_in->number_in_stack = dst_inst->GetCharges();
			if(!SwapItem(move_in)) { LogInventory("Recursive SwapItem call failed due to non-existent destination item (charid: [{}], fromslot: [{}], toslot: [{}])", CharacterID(), src_slot_id, dst_slot_id); }
		}

		return false;
	}
	//verify shared bank transactions in the database
	if (
		src_inst &&
		(
			EQ::ValueWithin(src_slot_id, EQ::invslot::SHARED_BANK_BEGIN, EQ::invslot::SHARED_BANK_END) ||
			EQ::ValueWithin(src_slot_id, EQ::invbag::SHARED_BANK_BAGS_BEGIN, EQ::invbag::SHARED_BANK_BAGS_END)
		)
	) {
		if(!database.VerifyInventory(account_id, src_slot_id, src_inst)) {
			LogError("Player [{}] on account [{}] was found exploiting the shared bank.\n", GetName(), account_name);
			DeleteItemInInventory(dst_slot_id,0,true);
			return(false);
		}

		if (EQ::ValueWithin(src_slot_id, EQ::invslot::SHARED_BANK_BEGIN, EQ::invslot::SHARED_BANK_END) && src_inst->IsClassBag()){
			for (uint8 idx = EQ::invbag::SLOT_BEGIN; idx <= EQ::invbag::SLOT_END; idx++) {
				const EQ::ItemInstance* baginst = src_inst->GetItem(idx);
				if (baginst && !database.VerifyInventory(account_id, EQ::InventoryProfile::CalcSlotId(src_slot_id, idx), baginst)){
					DeleteItemInInventory(EQ::InventoryProfile::CalcSlotId(src_slot_id, idx), 0, false);
				}
			}
		}
	}

	if (
		dst_inst &&
		(
			EQ::ValueWithin(dst_slot_id, EQ::invslot::SHARED_BANK_BEGIN, EQ::invslot::SHARED_BANK_END) ||
			EQ::ValueWithin(dst_slot_id, EQ::invbag::SHARED_BANK_BAGS_BEGIN, EQ::invbag::SHARED_BANK_BAGS_END)
		)
	) {
		if(!database.VerifyInventory(account_id, dst_slot_id, dst_inst)) {
			LogError("Player [{}] on account [{}] was found exploting the shared bank.\n", GetName(), account_name);
			DeleteItemInInventory(src_slot_id,0,true);
			return(false);
		}

		if (EQ::ValueWithin(dst_slot_id, EQ::invslot::SHARED_BANK_BEGIN, EQ::invslot::SHARED_BANK_END) && dst_inst->IsClassBag()){
			for (uint8 idx = EQ::invbag::SLOT_BEGIN; idx <= EQ::invbag::SLOT_END; idx++) {
				const EQ::ItemInstance* baginst = dst_inst->GetItem(idx);
				if (baginst && !database.VerifyInventory(account_id, EQ::InventoryProfile::CalcSlotId(dst_slot_id, idx), baginst)){
					DeleteItemInInventory(EQ::InventoryProfile::CalcSlotId(dst_slot_id, idx), 0, false);
				}
			}
		}
	}


	// Check for No Drop Hacks
	Mob* with = trade->With();
	if (
		(
			(
				with &&
				with->IsClient() &&
				!with->CastToClient()->IsBecomeNPC() &&
				EQ::ValueWithin(dst_slot_id, EQ::invslot::TRADE_BEGIN, EQ::invslot::TRADE_END)
			) ||
			EQ::ValueWithin(dst_slot_id, EQ::invslot::SHARED_BANK_BEGIN, EQ::invslot::SHARED_BANK_END) ||
			EQ::ValueWithin(dst_slot_id, EQ::invbag::SHARED_BANK_BAGS_BEGIN, EQ::invbag::SHARED_BANK_BAGS_END)
		) &&
		GetInv().CheckNoDrop(src_slot_id) &&
		!CanTradeFVNoDropItem()
	) {
		auto ndh_inst = m_inv[src_slot_id];
		std::string ndh_item_data;
		if (ndh_inst == nullptr) {
			ndh_item_data.append("[nullptr on EQ::ItemInstance*]");
		}
		else {
			auto ndh_item = ndh_inst->GetItem();
			if (ndh_item == nullptr) {
				ndh_item_data.append("[nullptr on ItemData*]");
			}
			else {
				ndh_item_data.append(StringFormat("name=%s", ndh_item->Name));
				ndh_item_data.append(StringFormat(", id=%u", ndh_item->ID));
				ndh_item_data.append(StringFormat(", nodrop=%s(%u)", (ndh_item->NoDrop == 0 ? "true" : "false"), ndh_item->NoDrop));
			}
		}
		LogError("WorldKick() of Player [{}](id:[{}], acct:[{}]) due to 'NoDrop Hack' detection >> SlotID:[{}], ItemData:[{}]",
			GetName(), CharacterID(), AccountID(), src_slot_id, ndh_item_data.c_str());
		ndh_inst = nullptr;

		DeleteItemInInventory(src_slot_id);
		WorldKick();
		return false;
	}

	// Step 3: Check for interaction with World Container (tradeskills)
	if(m_tradeskill_object != nullptr) {
		if (src_slot_id >= EQ::invslot::WORLD_BEGIN && src_slot_id <= EQ::invslot::WORLD_END) {
			// Picking up item from world container
			EQ::ItemInstance* inst = m_tradeskill_object->PopItem(EQ::InventoryProfile::CalcBagIdx(src_slot_id));
			if (inst) {
				PutItemInInventory(dst_slot_id, *inst, false);
				safe_delete(inst);
			}

			return true;
		}
		else if (dst_slot_id >= EQ::invslot::WORLD_BEGIN && dst_slot_id <= EQ::invslot::WORLD_END) {
			// Putting item into world container, which may swap (or pile onto) with existing item
			uint8 world_idx = EQ::InventoryProfile::CalcBagIdx(dst_slot_id);
			EQ::ItemInstance* world_inst = m_tradeskill_object->PopItem(world_idx);

			// Case 1: No item in container, unidirectional "Put"
			if (world_inst == nullptr) {
				m_tradeskill_object->PutItem(world_idx, src_inst);
				m_inv.DeleteItem(src_slot_id);
			}
			else {
				const EQ::ItemData* world_item = world_inst->GetItem();
				const EQ::ItemData* src_item = src_inst->GetItem();
				if (world_item && src_item) {
					// Case 2: Same item on cursor, stacks, transfer of charges needed
					if ((world_item->ID == src_item->ID) && src_inst->IsStackable()) {
						int16 world_charges = world_inst->GetCharges();
						int16 src_charges = src_inst->GetCharges();

						// Fill up destination stack as much as possible
						world_charges += src_charges;
						if (world_charges > world_inst->GetItem()->StackSize) {
							src_charges = world_charges - world_inst->GetItem()->StackSize;
							world_charges = world_inst->GetItem()->StackSize;
						}
						else {
							src_charges = 0;
						}

						world_inst->SetCharges(world_charges);
						m_tradeskill_object->PutItem(world_idx, world_inst);
						m_tradeskill_object->Save();

						if (src_charges == 0) {
							m_inv.DeleteItem(src_slot_id); // DB remove will occur below
						}
						else {
							src_inst->SetCharges(src_charges);
						}
					}
					else {
						// Case 3: Swap the item on user with item in world container
						// World containers don't follow normal rules for swapping
						EQ::ItemInstance* inv_inst = m_inv.PopItem(src_slot_id);
						m_tradeskill_object->PutItem(world_idx, inv_inst);
						m_inv.PutItem(src_slot_id, *world_inst);
						safe_delete(inv_inst);
					}
				}
			}

			safe_delete(world_inst);
			if (src_slot_id == EQ::invslot::slotCursor)
			{
				if (dstitemid == 0)
				{
					SendCursorBuffer();
				}
				auto s = m_inv.cursor_cbegin(), e = m_inv.cursor_cend();
				database.SaveCursor(character_id, s, e);
			}
			else
			{
				database.SaveInventory(character_id, m_inv[src_slot_id], src_slot_id);
			}

			return true;
		}
	}

	// Step 4: Check for entity trade
	if (dst_slot_id >= EQ::invslot::TRADE_BEGIN && dst_slot_id <= EQ::invslot::TRADE_END) {
		if (src_slot_id != EQ::invslot::slotCursor) {
			Kick("Trade with non-cursor item");
			return false;
		}
		if (with) {
			LogInventory("Trade item move from slot [{}] to slot [{}] (trade with [{}])", src_slot_id, dst_slot_id, with->GetName());
			// Fill Trade list with items from cursor
			if (!m_inv[EQ::invslot::slotCursor]) {
				Message(Chat::Red, "Error: Cursor item not located on server!");
				return false;
			}

			trade->AddEntity(dst_slot_id, move_in->number_in_stack);
			if (dstitemid == 0)
			{
				SendCursorBuffer();
			}

			return true;
		} else {
			SummonItem(src_inst->GetID(), src_inst->GetCharges());
			DeleteItemInInventory(EQ::invslot::slotCursor);

			return true;
		}
	}

	bool all_to_stack = false;
	// Step 5: Swap (or stack) items
	if (move_in->number_in_stack > 0) {
		// Determine if charged items can stack
		if(src_inst && !src_inst->IsStackable()) {
			LogInventory("Move from [{}] to [{}] with stack size [{}]. [{}] is not a stackable item. (charname: [{}])", src_slot_id, dst_slot_id, move_in->number_in_stack, src_inst->GetItem()->Name, GetName());
			return false;
		}

		if (dst_inst) {
			if(src_inst->GetID() != dst_inst->GetID()) {
				LogInventory("Move from [{}] to [{}] with stack size [{}]. Incompatible item types: [{}] != [{}]", src_slot_id, dst_slot_id, move_in->number_in_stack, src_inst->GetID(), dst_inst->GetID());
				return(false);
			}
			if(dst_inst->GetCharges() < dst_inst->GetItem()->StackSize) {
				//we have a chance of stacking.
				LogInventory("Move from [{}] to [{}] with stack size [{}]. dest has [{}]/[{}] charges", src_slot_id, dst_slot_id, move_in->number_in_stack, dst_inst->GetCharges(), dst_inst->GetItem()->StackSize);
				// Charges can be emptied into dst
				uint16 usedcharges = dst_inst->GetItem()->StackSize - dst_inst->GetCharges();
				if (usedcharges > move_in->number_in_stack)
					usedcharges = move_in->number_in_stack;

				dst_inst->SetCharges(dst_inst->GetCharges() + usedcharges);
				src_inst->SetCharges(src_inst->GetCharges() - usedcharges);

				// Depleted all charges?
				if (src_inst->GetCharges() < 1)
				{
					LogInventory("Dest ([{}]) now has [{}] charges, source ([{}]) was entirely consumed. ([{}] moved)", dst_slot_id, dst_inst->GetCharges(), src_slot_id, usedcharges);
					database.SaveInventory(CharacterID(),nullptr,src_slot_id);
					m_inv.DeleteItem(src_slot_id);
					all_to_stack = true;
				} else {
					LogInventory("Dest ([{}]) now has [{}] charges, source ([{}]) has [{}] ([{}] moved)", dst_slot_id, dst_inst->GetCharges(), src_slot_id, src_inst->GetCharges(), usedcharges);
				}
			} else {
				LogInventory("Move from [{}] to [{}] with stack size [{}]. Exceeds dest maximum stack size: [{}]/[{}]", src_slot_id, dst_slot_id, move_in->number_in_stack, (src_inst->GetCharges()+dst_inst->GetCharges()), dst_inst->GetItem()->StackSize);
				return false;
			}
		}
		else {
			// Nothing in destination slot: split stack into two
			if ((int16)move_in->number_in_stack >= src_inst->GetCharges()) {
				// Move entire stack
				EQ::InventoryProfile::SwapItemFailState fail_state = EQ::InventoryProfile::swapInvalid;
				if (!m_inv.SwapItem(src_slot_id, dst_slot_id, fail_state)) { return false; }
				LogInventory("Move entire stack from [{}] to [{}] with stack size [{}]. Dest empty", src_slot_id, dst_slot_id, move_in->number_in_stack);
			}
			else {
				// Split into two
				src_inst->SetCharges(src_inst->GetCharges() - move_in->number_in_stack);
				LogInventory("Split stack of [{}] ([{}]) from slot [{}] to [{}] with stack size [{}]. Src keeps [{}]", src_inst->GetItem()->Name, src_inst->GetItem()->ID, src_slot_id, dst_slot_id, move_in->number_in_stack, src_inst->GetCharges());
				EQ::ItemInstance* inst = database.CreateItem(src_inst->GetItem(), move_in->number_in_stack);
				m_inv.PutItem(dst_slot_id, *inst);
				safe_delete(inst);
			}
		}
	}
	else {
		// Not dealing with charges - just do direct swap
		if (src_inst && (dst_slot_id <= EQ::invslot::EQUIPMENT_END) && dst_slot_id >= EQ::invslot::EQUIPMENT_BEGIN) {
			if (src_inst->GetItem()->Attuneable) {
				src_inst->SetAttuned(true);
			}
			if (src_inst->IsAugmented()) {
				for (int i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
					if (src_inst->GetAugment(i)) {
						if (src_inst->GetAugment(i)->GetItem()->Attuneable) {
							src_inst->GetAugment(i)->SetAttuned(true);
						}
					}
				}
			}
			SetMaterial(dst_slot_id,src_inst->GetItem()->ID);
		}

		EQ::InventoryProfile::SwapItemFailState fail_state = EQ::InventoryProfile::swapInvalid;
		if (!m_inv.SwapItem(src_slot_id, dst_slot_id, fail_state, GetBaseRace(), GetBaseClass(), GetDeity(), GetLevel())) {
			const char* fail_message = "The selected slot was invalid.";
			if (fail_state == EQ::InventoryProfile::swapRaceClass || fail_state == EQ::InventoryProfile::swapDeity)
				fail_message = "Your class, deity and/or race may not equip that item.";
			else if (fail_state == EQ::InventoryProfile::swapLevel)
				fail_message = "You are not sufficient level to use this item.";

			if (fail_message)
				Message(Chat::Red, "%s", fail_message);

			return false;
		}

		LogInventory("Moving entire item from slot [{}] to slot [{}]", src_slot_id, dst_slot_id);
		if (src_inst->IsStackable() &&
			dst_slot_id >= EQ::invbag::GENERAL_BAGS_BEGIN &&
			dst_slot_id <= EQ::invbag::GENERAL_BAGS_END
			)	{
			EQ::ItemInstance *bag = nullptr;
			bag = m_inv.GetItem(EQ::InventoryProfile::CalcSlotId(dst_slot_id));
			if (bag) {
				if (bag->GetItem()->BagType == EQ::item::BagTypeTradersSatchel) {
					PutItemInInventory(dst_slot_id, *src_inst, true);
					//This resets the UF client to recognize the new serial item of the placed item
					//if it came from a stack without having to close the trader window and re-open.
					//It is not required for the RoF2 client.
					if (ClientVersion() < EQ::versions::ClientVersion::RoF2) {
						auto outapp  = new EQApplicationPacket(OP_Trader, sizeof(TraderBuy_Struct));
						auto data    = (TraderBuy_Struct *) outapp->pBuffer;
						data->action = BazaarBuyItem;
						FastQueuePacket(&outapp);
					}
				}
			}
		}

		if (src_slot_id <= EQ::invslot::EQUIPMENT_END) {
			if(src_inst) {
				if (parse->ItemHasQuestSub(src_inst, EVENT_UNEQUIP_ITEM)) {
					parse->EventItem(EVENT_UNEQUIP_ITEM, this, src_inst, nullptr, "", src_slot_id);
				}

				if (parse->PlayerHasQuestSub(EVENT_UNEQUIP_ITEM_CLIENT)) {
					const auto& export_string = fmt::format(
						"{} {}",
						src_inst->IsStackable() ? src_inst->GetCharges() : 1,
						src_slot_id
					);

					parse->EventPlayer(EVENT_UNEQUIP_ITEM_CLIENT, this, export_string, src_inst->GetItem()->ID);
				}
			}

			if (dst_inst) {
				if (parse->ItemHasQuestSub(dst_inst, EVENT_EQUIP_ITEM)) {
					parse->EventItem(EVENT_EQUIP_ITEM, this, dst_inst, nullptr, "", src_slot_id);
				}

				if (parse->PlayerHasQuestSub(EVENT_EQUIP_ITEM_CLIENT)) {
					const auto& export_string = fmt::format(
						"{} {}",
						dst_inst->IsStackable() ? dst_inst->GetCharges() : 1,
						src_slot_id
					);

					parse->EventPlayer(EVENT_EQUIP_ITEM_CLIENT, this, export_string, dst_inst->GetItem()->ID);
				}
			}
		}

		if (dst_slot_id <= EQ::invslot::EQUIPMENT_END) {
			if (dst_inst) {
				if (parse->ItemHasQuestSub(dst_inst, EVENT_UNEQUIP_ITEM)) {
					parse->EventItem(EVENT_UNEQUIP_ITEM, this, dst_inst, nullptr, "", dst_slot_id);
				}

				if (parse->PlayerHasQuestSub(EVENT_UNEQUIP_ITEM_CLIENT)) {
					const auto& export_string = fmt::format(
						"{} {}",
						dst_inst->IsStackable() ? dst_inst->GetCharges() : 1,
						dst_slot_id
					);

					std::vector<std::any> args = { dst_inst };

					parse->EventPlayer(EVENT_UNEQUIP_ITEM_CLIENT, this, export_string, dst_inst->GetItem()->ID, &args);
				}
			}

			if (src_inst) {
				if (parse->ItemHasQuestSub(src_inst, EVENT_EQUIP_ITEM)) {
					parse->EventItem(EVENT_EQUIP_ITEM, this, src_inst, nullptr, "", dst_slot_id);
				}

				if (parse->PlayerHasQuestSub(EVENT_EQUIP_ITEM_CLIENT)) {
					const auto& export_string = fmt::format(
						"{} {}",
						src_inst->IsStackable() ? src_inst->GetCharges() : 1,
						dst_slot_id
					);

					std::vector<std::any> args = { src_inst };

					parse->EventPlayer(EVENT_EQUIP_ITEM_CLIENT, this, export_string, src_inst->GetItem()->ID, &args);
				}
			}
		}
	}

	int matslot = SlotConvert2(dst_slot_id);
	if (dst_slot_id <= EQ::invslot::EQUIPMENT_END) {// on Titanium and ROF2 /showhelm works even if sending helm slot
		SendWearChange(matslot);
	}

	// Step 7: Save change to the database
	if (src_slot_id == EQ::invslot::slotCursor) {
		// If not swapping another item to cursor and stacking items were depleted
		if (dstitemid == 0 || all_to_stack == true)
		{
			SendCursorBuffer();
		}
		auto s = m_inv.cursor_cbegin(), e = m_inv.cursor_cend();
		database.SaveCursor(character_id, s, e);
	}
	else {
		database.SaveInventory(character_id, m_inv.GetItem(src_slot_id), src_slot_id);
	}

	if (dst_slot_id == EQ::invslot::slotCursor) {
		auto s = m_inv.cursor_cbegin(), e = m_inv.cursor_cend();
		database.SaveCursor(character_id, s, e);
	}
	else {
		database.SaveInventory(character_id, m_inv.GetItem(dst_slot_id), dst_slot_id);
	}

	// Step 8: Re-calc stats
	CalcBonuses();
	ApplyWeaponsStance();
	return true;
}

void Client::SwapItemResync(MoveItem_Struct* move_slots) {
	// wow..this thing created a helluva memory leak...
	// with any luck..this won't be needed in the future

	// resync the 'from' and 'to' slots on an as-needed basis
	// Not as effective as the full process, but less intrusive to gameplay
	LogInventory("Inventory desyncronization. (charname: [{}], source: [{}], destination: [{}])", GetName(), move_slots->from_slot, move_slots->to_slot);
	Message(Chat::Yellow, "Inventory Desyncronization detected: Resending slot data...");

	if (move_slots->from_slot >= EQ::invslot::EQUIPMENT_BEGIN && move_slots->from_slot <= EQ::invbag::CURSOR_BAG_END) {
		int16 resync_slot = (EQ::InventoryProfile::CalcSlotId(move_slots->from_slot) == INVALID_INDEX) ? move_slots->from_slot : EQ::InventoryProfile::CalcSlotId(move_slots->from_slot);
		if (IsValidSlot(resync_slot) && resync_slot != INVALID_INDEX) {
			// This prevents the client from crashing when closing any 'phantom' bags
			const EQ::ItemData* token_struct = database.GetItem(22292); // 'Copper Coin'
			EQ::ItemInstance* token_inst = database.CreateItem(token_struct, 1);

			SendItemPacket(resync_slot, token_inst, ItemPacketTrade);

			if(m_inv[resync_slot]) { SendItemPacket(resync_slot, m_inv[resync_slot], ItemPacketTrade); }
			else {
				auto outapp = new EQApplicationPacket(OP_DeleteItem, sizeof(DeleteItem_Struct));
				DeleteItem_Struct* delete_slot	= (DeleteItem_Struct*)outapp->pBuffer;
				delete_slot->from_slot			= resync_slot;
				delete_slot->to_slot			= 0xFFFFFFFF;
				delete_slot->number_in_stack	= 0xFFFFFFFF;

				QueuePacket(outapp);
				safe_delete(outapp);
			}
			safe_delete(token_inst);
			Message(Chat::Lime, "Source slot %i resyncronized.", move_slots->from_slot);
		}
		else { Message(Chat::Red, "Could not resyncronize source slot %i.", move_slots->from_slot); }
	}
	else {
		int16 resync_slot = (EQ::InventoryProfile::CalcSlotId(move_slots->from_slot) == INVALID_INDEX) ? move_slots->from_slot : EQ::InventoryProfile::CalcSlotId(move_slots->from_slot);
		if (IsValidSlot(resync_slot) && resync_slot != INVALID_INDEX) {
			if(m_inv[resync_slot]) {
				const EQ::ItemData* token_struct = database.GetItem(22292); // 'Copper Coin'
				EQ::ItemInstance* token_inst = database.CreateItem(token_struct, 1);

				SendItemPacket(resync_slot, token_inst, ItemPacketTrade);
				SendItemPacket(resync_slot, m_inv[resync_slot], ItemPacketTrade);

				safe_delete(token_inst);
				Message(Chat::Lime, "Source slot %i resyncronized.", move_slots->from_slot);
			}
			else { Message(Chat::Red, "Could not resyncronize source slot %i.", move_slots->from_slot); }
		}
		else { Message(Chat::Red, "Could not resyncronize source slot %i.", move_slots->from_slot); }
	}

	if (move_slots->to_slot >= EQ::invslot::EQUIPMENT_BEGIN && move_slots->to_slot <= EQ::invbag::CURSOR_BAG_END) {
		int16 resync_slot = (EQ::InventoryProfile::CalcSlotId(move_slots->to_slot) == INVALID_INDEX) ? move_slots->to_slot : EQ::InventoryProfile::CalcSlotId(move_slots->to_slot);
		if (IsValidSlot(resync_slot) && resync_slot != INVALID_INDEX) {
			const EQ::ItemData* token_struct = database.GetItem(22292); // 'Copper Coin'
			EQ::ItemInstance* token_inst = database.CreateItem(token_struct, 1);

			SendItemPacket(resync_slot, token_inst, ItemPacketTrade);

			if(m_inv[resync_slot]) { SendItemPacket(resync_slot, m_inv[resync_slot], ItemPacketTrade); }
			else {
				auto outapp = new EQApplicationPacket(OP_DeleteItem, sizeof(DeleteItem_Struct));
				DeleteItem_Struct* delete_slot	= (DeleteItem_Struct*)outapp->pBuffer;
				delete_slot->from_slot			= resync_slot;
				delete_slot->to_slot			= 0xFFFFFFFF;
				delete_slot->number_in_stack	= 0xFFFFFFFF;

				QueuePacket(outapp);
				safe_delete(outapp);
			}
			safe_delete(token_inst);
			Message(Chat::Lime, "Destination slot %i resyncronized.", move_slots->to_slot);
		}
		else { Message(Chat::Red, "Could not resyncronize destination slot %i.", move_slots->to_slot); }
	}
	else {
		int16 resync_slot = (EQ::InventoryProfile::CalcSlotId(move_slots->to_slot) == INVALID_INDEX) ? move_slots->to_slot : EQ::InventoryProfile::CalcSlotId(move_slots->to_slot);
		if (IsValidSlot(resync_slot) && resync_slot != INVALID_INDEX) {
			if(m_inv[resync_slot]) {
				const EQ::ItemData* token_struct = database.GetItem(22292); // 'Copper Coin'
				EQ::ItemInstance* token_inst = database.CreateItem(token_struct, 1);

				SendItemPacket(resync_slot, token_inst, ItemPacketTrade);
				SendItemPacket(resync_slot, m_inv[resync_slot], ItemPacketTrade);

				safe_delete(token_inst);
				Message(Chat::Lime, "Destination slot %i resyncronized.", move_slots->to_slot);
			}
			else { Message(Chat::Red, "Could not resyncronize destination slot %i.", move_slots->to_slot); }
		}
		else { Message(Chat::Red, "Could not resyncronize destination slot %i.", move_slots->to_slot); }
	}
}

void Client::DyeArmor(EQ::TintProfile* dye){
	int16 slot=0;
	for (int i = EQ::textures::textureBegin; i <= EQ::textures::LastTintableTexture; i++) {
		if ((m_pp.item_tint.Slot[i].Color & 0x00FFFFFF) != (dye->Slot[i].Color & 0x00FFFFFF)) {
			slot = m_inv.HasItem(32557, 1, invWherePersonal);
			if (slot != INVALID_INDEX){
				DeleteItemInInventory(slot,1,true);
				uint8 slot2=SlotConvert(i);
				EQ::ItemInstance* inst = m_inv.GetItem(slot2);
				if(inst){
					uint32 armor_color = ((uint32)dye->Slot[i].Red << 16) | ((uint32)dye->Slot[i].Green << 8) | ((uint32)dye->Slot[i].Blue);
					inst->SetColor(armor_color);
					database.SaveCharacterMaterialColor(CharacterID(), i, armor_color);
					database.SaveInventory(CharacterID(),inst,slot2);
					if(dye->Slot[i].UseTint)
						m_pp.item_tint.Slot[i].UseTint = 0xFF;
					else
						m_pp.item_tint.Slot[i].UseTint=0x00;
				}
				m_pp.item_tint.Slot[i].Blue=dye->Slot[i].Blue;
				m_pp.item_tint.Slot[i].Red=dye->Slot[i].Red;
				m_pp.item_tint.Slot[i].Green=dye->Slot[i].Green;
				SendWearChange(i);
			}
			else{
				Message(Chat::Red,"Could not locate A Vial of Prismatic Dye.");
				return;
			}
		}
	}
	auto outapp = new EQApplicationPacket(OP_Dye, 0);
	QueuePacket(outapp);
	safe_delete(outapp);

}

void Client::DyeArmorBySlot(uint8 slot, uint8 red, uint8 green, uint8 blue, uint8 use_tint) {
	uint8 item_slot = SlotConvert(slot);
	EQ::ItemInstance* item_instance = m_inv.GetItem(item_slot);
	if (item_instance) {
		uint32 armor_color = ((uint32)red << 16) | ((uint32)green << 8) | ((uint32)blue);
		item_instance->SetColor(armor_color);
		database.SaveCharacterMaterialColor(CharacterID(), slot, armor_color);
		database.SaveInventory(CharacterID(), item_instance, item_slot);
		m_pp.item_tint.Slot[slot].UseTint = (use_tint ? 0xFF : 0x00);
	}
	m_pp.item_tint.Slot[slot].Red = red;
	m_pp.item_tint.Slot[slot].Green = green;
	m_pp.item_tint.Slot[slot].Blue = blue;
	SendWearChange(slot);
}

bool Client::DecreaseByID(uint32 type, int16 quantity) {
	const EQ::ItemData* TempItem = nullptr;
	EQ::ItemInstance* ins = nullptr;
	int x;
	int num = 0;

	for (x = EQ::invslot::POSSESSIONS_BEGIN; x <= EQ::invslot::POSSESSIONS_END; ++x) {
		if (num >= quantity)
			break;
		if ((((uint64)1 << x) & GetInv().GetLookup()->PossessionsBitmask) == 0)
			continue;

		TempItem = nullptr;
		ins = GetInv().GetItem(x);
		if (ins)
			TempItem = ins->GetItem();
		if (TempItem && TempItem->ID == type)
			num += ins->GetCharges();
	}

	for (x = EQ::invbag::GENERAL_BAGS_BEGIN; x <= EQ::invbag::GENERAL_BAGS_END; ++x) {
		if (num >= quantity)
			break;
		if ((((uint64)1 << (EQ::invslot::GENERAL_BEGIN + ((x - EQ::invbag::GENERAL_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT))) & GetInv().GetLookup()->PossessionsBitmask) == 0)
			continue;

		TempItem = nullptr;
		ins = GetInv().GetItem(x);
		if (ins)
			TempItem = ins->GetItem();
		if (TempItem && TempItem->ID == type)
			num += ins->GetCharges();
	}

	for (x = EQ::invbag::CURSOR_BAG_BEGIN; x <= EQ::invbag::CURSOR_BAG_END; ++x) {
		if (num >= quantity)
			break;

		TempItem = nullptr;
		ins = GetInv().GetItem(x);
		if (ins)
			TempItem = ins->GetItem();
		if (TempItem && TempItem->ID == type)
			num += ins->GetCharges();
	}

	if (num < quantity)
		return false;


	for (x = EQ::invslot::POSSESSIONS_BEGIN; x <= EQ::invslot::POSSESSIONS_END; ++x) {
		if (quantity < 1)
			break;
		if ((((uint64)1 << x) & GetInv().GetLookup()->PossessionsBitmask) == 0)
			continue;

		TempItem = nullptr;
		ins = GetInv().GetItem(x);
		if (ins)
			TempItem = ins->GetItem();
		if (TempItem && TempItem->ID != type)
			continue;

		if (ins->GetCharges() < quantity) {
			quantity -= ins->GetCharges();
			DeleteItemInInventory(x, quantity, true);
		}
		else {
			DeleteItemInInventory(x, quantity, true);
			quantity = 0;
		}
	}

	for (x = EQ::invbag::GENERAL_BAGS_BEGIN; x <= EQ::invbag::GENERAL_BAGS_END; ++x) {
		if (quantity < 1)
			break;
		if ((((uint64)1 << (EQ::invslot::GENERAL_BEGIN + ((x - EQ::invbag::GENERAL_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT))) & GetInv().GetLookup()->PossessionsBitmask) == 0)
			continue;

		TempItem = nullptr;
		ins = GetInv().GetItem(x);
		if (ins)
			TempItem = ins->GetItem();
		if (TempItem && TempItem->ID != type)
			continue;

		if (ins->GetCharges() < quantity) {
			quantity -= ins->GetCharges();
			DeleteItemInInventory(x, quantity, true);
		}
		else {
			DeleteItemInInventory(x, quantity, true);
			quantity = 0;
		}
	}

	for (x = EQ::invbag::CURSOR_BAG_BEGIN; x <= EQ::invbag::CURSOR_BAG_END; ++x) {
		if (quantity < 1)
			break;

		TempItem = nullptr;
		ins = GetInv().GetItem(x);
		if (ins)
			TempItem = ins->GetItem();
		if (TempItem && TempItem->ID != type)
			continue;

		if (ins->GetCharges() < quantity) {
			quantity -= ins->GetCharges();
			DeleteItemInInventory(x, quantity, true);
		}
		else {
			DeleteItemInInventory(x, quantity, true);
			quantity = 0;
		}
	}

	return true;
}

static bool IsSummonedBagID(uint32 item_id)
{
	switch (item_id) {
	case 17147: // "Spiritual Prismatic Pack"
	case 17303: // "Spirit Pouch"
	case 17304: // "Dimensional Pocket"
	case 17305: // "Dimensional Hole"
	case 17306: // "Glowing Backpack"
	case 17307: // "Quiver of Marr"
	case 17308: // "Bandoleer of Luclin"
	case 17309: // "Pouch of Quellious"
	case 17310: // "Phantom Satchel"
	case 17510: // "Glowing Chest"
	case 17900: // "Grandmaster's Satchel"
	case 57260: // "Glowing Backpack"
	case 57261: // "Pouch of Quellious"
	case 57262: // "Phantom Satchel"
	case 60224: // "Faded-Glyph Tablet"
	case 95199: // "Beginner Artisan Satchel"
	case 95200: // "Apprentice Artisan Satchel"
	case 95201: // "Freshman Artisan Satchel"
	case 95202: // "Journeyman Artisan Satchel"
	case 95203: // "Expert Artisan Satchel"
	case 95204: // "Master Artisan Satchel"
	//case 96960: // "Artisan Satchel" - no 12-slot disenchanted bags
		return true;
	default:
		return false;
	}
}

static uint32 GetDisenchantedBagID(uint8 bag_slots)
{
	switch (bag_slots) {
	case 4:
		return 77772; // "Small Disenchanted Backpack"
	case 6:
		return 77774; // "Disenchanted Backpack"
	case 8:
		return 77776; // "Large Disenchanted Backpack"
	case 10:
		return 77778; // "Huge Disenchanted Backpack"
	default:
		return 0; // no suitable conversions
	}
}

static bool CopyBagContents(EQ::ItemInstance* new_bag, const EQ::ItemInstance* old_bag)
{
	if (!new_bag || !old_bag) { return false; }
	if (new_bag->GetItem()->BagSlots < old_bag->GetItem()->BagSlots) { return false; }

	// pre-check for size comparisons
	for (auto bag_slot = 0; bag_slot < old_bag->GetItem()->BagSlots; ++bag_slot) {
		if (!old_bag->GetItem(bag_slot)) { continue; }
		if (old_bag->GetItem(bag_slot)->GetItem()->Size > new_bag->GetItem()->BagSize) {
			LogInventory("Copy Bag Contents: Failure due to [{}] is larger than size capacity of [{}] ([{}] > [{}])",
				old_bag->GetItem(bag_slot)->GetItem()->Name, new_bag->GetItem()->Name, old_bag->GetItem(bag_slot)->GetItem()->Size, new_bag->GetItem()->BagSize);
			return false;
		}
	}

	for (auto bag_slot = 0; bag_slot < old_bag->GetItem()->BagSlots; ++bag_slot) {
		if (!old_bag->GetItem(bag_slot)) { continue; }
		new_bag->PutItem(bag_slot, *(old_bag->GetItem(bag_slot)));
	}

	return true;
}

void Client::DisenchantSummonedBags(bool client_update)
{
	for (auto slot_id = EQ::invslot::GENERAL_BEGIN; slot_id <= EQ::invslot::GENERAL_END; ++slot_id) {
		if ((((uint64)1 << slot_id) & GetInv().GetLookup()->PossessionsBitmask) == 0)
			continue; // not usable this session - will be disenchanted once player logs in on client that doesn't exclude affected slots

		auto inst = m_inv[slot_id];
		if (!inst) { continue; }
		if (!IsSummonedBagID(inst->GetItem()->ID)) { continue; }
		if (!inst->GetItem()->IsClassBag()) { continue; }
		if (inst->GetTotalItemCount() == 1) { continue; }

		auto new_id = GetDisenchantedBagID(inst->GetItem()->BagSlots);
		if (!new_id) { continue; }
		auto new_item = database.GetItem(new_id);
		if (!new_item) { continue; }
		auto new_inst = database.CreateBaseItem(new_item);
		if (!new_inst) { continue; }

		if (CopyBagContents(new_inst, inst)) {
			LogInventory("Disenchant Summoned Bags: Replacing [{}] with [{}] in slot [{}]", inst->GetItem()->Name, new_inst->GetItem()->Name, slot_id);
			PutItemInInventory(slot_id, *new_inst, client_update);
		}
		safe_delete(new_inst);
	}

	for (auto slot_id = EQ::invslot::BANK_BEGIN; slot_id <= EQ::invslot::BANK_END; ++slot_id) {
		if ((slot_id - EQ::invslot::BANK_BEGIN) >= GetInv().GetLookup()->InventoryTypeSize.Bank)
			continue;

		auto inst = m_inv[slot_id];
		if (!inst) { continue; }
		if (!IsSummonedBagID(inst->GetItem()->ID)) { continue; }
		if (!inst->GetItem()->IsClassBag()) { continue; }
		if (inst->GetTotalItemCount() == 1) { continue; }

		auto new_id = GetDisenchantedBagID(inst->GetItem()->BagSlots);
		if (!new_id) { continue; }
		auto new_item = database.GetItem(new_id);
		if (!new_item) { continue; }
		auto new_inst = database.CreateBaseItem(new_item);
		if (!new_inst) { continue; }

		if (CopyBagContents(new_inst, inst)) {
			LogInventory("Disenchant Summoned Bags: Replacing [{}] with [{}] in slot [{}]", inst->GetItem()->Name, new_inst->GetItem()->Name, slot_id);
			PutItemInInventory(slot_id, *new_inst, client_update);
		}
		safe_delete(new_inst);
	}

	for (auto slot_id = EQ::invslot::SHARED_BANK_BEGIN; slot_id <= EQ::invslot::SHARED_BANK_END; ++slot_id) {
		auto inst = m_inv[slot_id];
		if (!inst) { continue; }
		if (!IsSummonedBagID(inst->GetItem()->ID)) { continue; }
		if (!inst->GetItem()->IsClassBag()) { continue; }
		if (inst->GetTotalItemCount() == 1) { continue; }

		auto new_id = GetDisenchantedBagID(inst->GetItem()->BagSlots);
		if (!new_id) { continue; }
		auto new_item = database.GetItem(new_id);
		if (!new_item) { continue; }
		auto new_inst = database.CreateBaseItem(new_item);
		if (!new_inst) { continue; }

		if (CopyBagContents(new_inst, inst)) {
			LogInventory("Disenchant Summoned Bags: Replacing [{}] with [{}] in slot [{}]", inst->GetItem()->Name, new_inst->GetItem()->Name, slot_id);
			PutItemInInventory(slot_id, *new_inst, client_update);
		}
		safe_delete(new_inst);
	}

	while (!m_inv.CursorEmpty()) {
		auto inst = m_inv[EQ::invslot::slotCursor];
		if (!inst) { break; }
		if (!IsSummonedBagID(inst->GetItem()->ID)) { break; }
		if (!inst->GetItem()->IsClassBag()) { break; }
		if (inst->GetTotalItemCount() == 1) { break; }

		auto new_id = GetDisenchantedBagID(inst->GetItem()->BagSlots);
		if (!new_id) { break; }
		auto new_item = database.GetItem(new_id);
		if (!new_item) { break; }
		auto new_inst = database.CreateBaseItem(new_item);
		if (!new_inst) { break; }

		if (CopyBagContents(new_inst, inst)) {
			LogInventory("Disenchant Summoned Bags: Replacing [{}] with [{}] in slot [{}]", inst->GetItem()->Name, new_inst->GetItem()->Name, EQ::invslot::slotCursor);
			std::list<EQ::ItemInstance*> local;
			local.push_front(new_inst);
			m_inv.PopItem(EQ::invslot::slotCursor);
			safe_delete(inst);

			while (!m_inv.CursorEmpty()) {
				auto limbo_inst = m_inv.PopItem(EQ::invslot::slotCursor);
				if (limbo_inst == nullptr) { continue; }
				local.push_back(limbo_inst);
			}

			for (auto iter = local.begin(); iter != local.end(); ++iter) {
				auto cur_inst = *iter;
				if (cur_inst == nullptr) { continue; }
				m_inv.PushCursor(*cur_inst);
				safe_delete(cur_inst);
			}
			local.clear();

			auto s = m_inv.cursor_cbegin(), e = m_inv.cursor_cend();
			database.SaveCursor(CharacterID(), s, e);
		}
		else {
			safe_delete(new_inst); // deletes disenchanted bag if not used
		}

		break;
	}
}

void Client::RemoveNoRent(bool client_update)
{
	for (auto slot_id = EQ::invslot::EQUIPMENT_BEGIN; slot_id <= EQ::invslot::EQUIPMENT_END; ++slot_id) {
		if ((((uint64)1 << slot_id) & GetInv().GetLookup()->PossessionsBitmask) == 0)
			continue;

		auto inst = m_inv[slot_id];
		if(inst && !inst->GetItem()->NoRent) {
			LogInventory("NoRent Timer Lapse: Deleting [{}] from slot [{}]", inst->GetItem()->Name, slot_id);
			DeleteItemInInventory(slot_id, 0, client_update);
		}
	}

	for (auto slot_id = EQ::invslot::GENERAL_BEGIN; slot_id <= EQ::invslot::GENERAL_END; ++slot_id) {
		if ((((uint64)1 << slot_id) & GetInv().GetLookup()->PossessionsBitmask) == 0)
			continue;

		auto inst = m_inv[slot_id];
		if (inst && !inst->GetItem()->NoRent) {
			LogInventory("NoRent Timer Lapse: Deleting [{}] from slot [{}]", inst->GetItem()->Name, slot_id);
			DeleteItemInInventory(slot_id, 0, client_update);
		}
	}

	for (auto slot_id = EQ::invbag::GENERAL_BAGS_BEGIN; slot_id <= EQ::invbag::CURSOR_BAG_END; ++slot_id) {
		auto temp_slot = EQ::invslot::GENERAL_BEGIN + ((slot_id - EQ::invbag::GENERAL_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT);
		if ((((uint64)1 << temp_slot) & GetInv().GetLookup()->PossessionsBitmask) == 0)
			continue;

		auto inst = m_inv[slot_id];
		if(inst && !inst->GetItem()->NoRent) {
			LogInventory("NoRent Timer Lapse: Deleting [{}] from slot [{}]", inst->GetItem()->Name, slot_id);
			DeleteItemInInventory(slot_id, 0, client_update);
		}
	}

	for (auto slot_id = EQ::invslot::BANK_BEGIN; slot_id <= EQ::invslot::BANK_END; ++slot_id) {
		if ((slot_id - EQ::invslot::BANK_BEGIN) >= GetInv().GetLookup()->InventoryTypeSize.Bank)
			continue;

		auto inst = m_inv[slot_id];
		if(inst && !inst->GetItem()->NoRent) {
			LogInventory("NoRent Timer Lapse: Deleting [{}] from slot [{}]", inst->GetItem()->Name, slot_id);
			DeleteItemInInventory(slot_id); // Can't delete from client Bank slots
		}
	}

	for (auto slot_id = EQ::invbag::BANK_BAGS_BEGIN; slot_id <= EQ::invbag::BANK_BAGS_END; ++slot_id) {
		auto temp_slot = (slot_id - EQ::invbag::BANK_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT;
		if (temp_slot >= GetInv().GetLookup()->InventoryTypeSize.Bank)
			continue;

		auto inst = m_inv[slot_id];
		if(inst && !inst->GetItem()->NoRent) {
			LogInventory("NoRent Timer Lapse: Deleting [{}] from slot [{}]", inst->GetItem()->Name, slot_id);
			DeleteItemInInventory(slot_id); // Can't delete from client Bank Container slots
		}
	}

	for (auto slot_id = EQ::invslot::SHARED_BANK_BEGIN; slot_id <= EQ::invslot::SHARED_BANK_END; ++slot_id) {
		auto inst = m_inv[slot_id];
		if(inst && !inst->GetItem()->NoRent) {
			LogInventory("NoRent Timer Lapse: Deleting [{}] from slot [{}]", inst->GetItem()->Name, slot_id);
			DeleteItemInInventory(slot_id); // Can't delete from client Shared Bank slots
		}
	}

	for (auto slot_id = EQ::invbag::SHARED_BANK_BAGS_BEGIN; slot_id <= EQ::invbag::SHARED_BANK_BAGS_END; ++slot_id) {
		auto inst = m_inv[slot_id];
		if(inst && !inst->GetItem()->NoRent) {
			LogInventory("NoRent Timer Lapse: Deleting [{}] from slot [{}]", inst->GetItem()->Name, slot_id);
			DeleteItemInInventory(slot_id); // Can't delete from client Shared Bank Container slots
		}
	}

	if (!m_inv.CursorEmpty()) {
		std::list<EQ::ItemInstance*> local;

		while (!m_inv.CursorEmpty()) {
			auto inst = m_inv.PopItem(EQ::invslot::slotCursor);
			if (inst == nullptr) { continue; }
			local.push_back(inst);
		}

		for (auto iter = local.begin(); iter != local.end(); ++iter) {
			auto inst = *iter;
			if (inst == nullptr) { continue; }
			if (!inst->GetItem()->NoRent) {
				LogInventory("NoRent Timer Lapse: Deleting [{}] from `Limbo`", inst->GetItem()->Name);
			}
			else {
				m_inv.PushCursor(*inst);
			}
			safe_delete(inst);
		}
		local.clear();

		auto s = m_inv.cursor_cbegin(), e = m_inv.cursor_cend();
		database.SaveCursor(CharacterID(), s, e);
	}
}

// Two new methods to alleviate perpetual login desyncs
void Client::RemoveDuplicateLore()
{
	for (auto slot_id : GetInventorySlots()) {
		if ((((uint64) 1 << slot_id) & GetInv().GetLookup()->PossessionsBitmask) == 0) {
			continue;
		}

		// ignore shared bank slots
		if (slot_id >= EQ::invslot::SHARED_BANK_BEGIN && slot_id <= EQ::invslot::SHARED_BANK_END) {
			continue;
		}

		if (slot_id >= EQ::invbag::SHARED_BANK_BAGS_BEGIN && slot_id <= EQ::invbag::SHARED_BANK_BAGS_END) {
			continue;
		}

		// slot gets handled in a queue
		if (slot_id == EQ::invslot::slotCursor) {
			continue;
		}

		// temporarily move the item off of the slot
		auto inst = m_inv.PopItem(slot_id);
		if (!inst) {
			continue;
		}

		if (CheckLoreConflict(inst->GetItem())) {
			LogError(
				"Lore Duplication Error | Deleting [{}] ({}) from slot [{}] client [{}]",
				inst->GetItem()->Name,
				inst->GetItem()->ID,
				slot_id,
				GetCleanName()
			);
			database.SaveInventory(character_id, nullptr, slot_id);
			safe_delete(inst);
		}

		// if no lore conflict, put the item back in the slot
		m_inv.PushItem(slot_id, inst);
	}

	if (!m_inv.CursorEmpty()) {
		std::list<EQ::ItemInstance*> local_1;
		std::list<EQ::ItemInstance*> local_2;

		while (!m_inv.CursorEmpty()) {
			auto inst = m_inv.PopItem(EQ::invslot::slotCursor);
			if (!inst) {
				continue;
			}
			local_1.push_back(inst);
		}

		for (auto inst: local_1) {
			if (!inst) {
				continue;
			}
			if (CheckLoreConflict(inst->GetItem())) {
				LogError(
					"Lore Duplication Error | Deleting [{}] ({}) from `Limbo` client [{}]",
					inst->GetItem()->Name,
					inst->GetItem()->ID,
					GetCleanName()
				);
				safe_delete(inst);
			}
			else {
				local_2.push_back(inst);
			}
		}
		local_1.clear();

		for (auto inst: local_2) {
			if (!inst) {
				continue;
			}
			if (!inst->GetItem()->LoreFlag ||
				((inst->GetItem()->LoreGroup == -1) &&
				 (m_inv.HasItem(inst->GetID(), 0, invWhereCursor) == INVALID_INDEX)) ||
				(inst->GetItem()->LoreGroup && (~inst->GetItem()->LoreGroup) &&
				 (m_inv.HasItemByLoreGroup(inst->GetItem()->LoreGroup, invWhereCursor) == INVALID_INDEX))
				) {
				m_inv.PushCursor(*inst);
			}
			else {
				LogError(
					"Lore Duplication Error | Deleting [{}] ({}) from `Limbo` client [{}]",
					inst->GetItem()->Name,
					inst->GetItem()->ID,
					GetCleanName()
				);
			}
			safe_delete(inst);
		}
		local_2.clear();

		auto s = m_inv.cursor_cbegin(), e = m_inv.cursor_cend();
		database.SaveCursor(CharacterID(), s, e);
	}
}

void Client::MoveSlotNotAllowed(bool client_update)
{
	for (auto slot_id = EQ::invslot::EQUIPMENT_BEGIN; slot_id <= EQ::invslot::EQUIPMENT_END; ++slot_id) {
		if(m_inv[slot_id] && !m_inv[slot_id]->IsSlotAllowed(slot_id)) {
			auto inst = m_inv.PopItem(slot_id);
			bool is_arrow = (inst->GetItem()->ItemType == EQ::item::ItemTypeArrow) ? true : false;
			int16 free_slot_id = m_inv.FindFreeSlot(inst->IsClassBag(), true, inst->GetItem()->Size, is_arrow);
			LogInventory("Slot Assignment Error: Moving [{}] from slot [{}] to [{}]", inst->GetItem()->Name, slot_id, free_slot_id);
			PutItemInInventory(free_slot_id, *inst, client_update);
			database.SaveInventory(character_id, nullptr, slot_id);
			safe_delete(inst);
		}
	}

	// added this check to move any client-based excluded slots
	//for (auto slot_id = EQ::invslot::POSSESSIONS_BEGIN; slot_id <= EQ::invslot::POSSESSIONS_END; ++slot_id) {
	//	if (((uint64)1 << slot_id) & GetInv().GetLookup()->PossessionsBitmask != 0)
	//		continue;

	//	if (m_inv[slot_id]) { // this is currently dangerous for bag-based movements since limbo does not save bag slots
	//		auto inst = m_inv.PopItem(slot_id);
	//		bool is_arrow = (inst->GetItem()->ItemType == EQ::item::ItemTypeArrow) ? true : false;
	//		int16 free_slot_id = m_inv.FindFreeSlot(inst->IsClassBag(), true, inst->GetItem()->Size, is_arrow);
	//		LogInventory("Slot Assignment Error: Moving [{}] from slot [{}] to [{}]", inst->GetItem()->Name, slot_id, free_slot_id);
	//		PutItemInInventory(free_slot_id, *inst, client_update);
	//		database.SaveInventory(character_id, nullptr, slot_id);
	//		safe_delete(inst);
	//	}
	//}

	// No need to check inventory, cursor, bank or shared bank since they allow max item size and containers
	// Code can be added to check item size vs. container size, but it is left to attrition for now.
}

// these functions operate with a material slot, which is from 0 to 8
uint32 Client::GetEquippedItemFromTextureSlot(uint8 material_slot) const
{
	int16 inventory_slot;

	const EQ::ItemInstance *item;

	if (material_slot > EQ::textures::LastTexture) {
		return 0;
	}

	inventory_slot = EQ::InventoryProfile::CalcSlotFromMaterial(material_slot);
	if (inventory_slot == INVALID_INDEX) {
		return 0;
	}

	item = m_inv.GetItem(inventory_slot);

	if (item && item->GetItem()) {
		return item->GetItem()->ID;
	}

	return 0;
}

uint32 Client::GetEquipmentColor(uint8 material_slot) const
{
	if (material_slot > EQ::textures::LastTexture)
		return 0;

	const EQ::ItemData *item = database.GetItem(GetEquippedItemFromTextureSlot(material_slot));
	if(item != nullptr)
		return ((m_pp.item_tint.Slot[material_slot].UseTint) ? m_pp.item_tint.Slot[material_slot].Color : item->Color);

	return 0;
}

// Send an item packet (including all subitems of the item)
void Client::SendItemPacket(int16 slot_id, const EQ::ItemInstance* inst, ItemPacketType packet_type)
{
	if (!inst) {
		return;
	}

	if (!eqs) {
		return;
	}

	if (packet_type != ItemPacketMerchant) {
		if (slot_id <= EQ::invslot::POSSESSIONS_END && slot_id >= EQ::invslot::POSSESSIONS_BEGIN) {
			if ((((uint64)1 << slot_id) & GetInv().GetLookup()->PossessionsBitmask) == 0) {
				LogError("Item not sent to merchant : slot [{}]", slot_id);
				return;
			}
		}
		else if (slot_id <= EQ::invbag::GENERAL_BAGS_END && slot_id >= EQ::invbag::GENERAL_BAGS_BEGIN) {
			auto temp_slot = EQ::invslot::GENERAL_BEGIN + ((slot_id - EQ::invbag::GENERAL_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT);
			if ((((uint64)1 << temp_slot) & GetInv().GetLookup()->PossessionsBitmask) == 0) {
				LogError("Item not sent to merchant2 : slot [{}]", slot_id);
				return;
			}
		}
		else if (slot_id <= EQ::invslot::BANK_END && slot_id >= EQ::invslot::BANK_BEGIN) {
			if ((slot_id - EQ::invslot::BANK_BEGIN) >= GetInv().GetLookup()->InventoryTypeSize.Bank) {
				LogError("Item not sent to merchant3 : slot [{}]", slot_id);
				return;
			}
		}
		else if (slot_id <= EQ::invbag::BANK_BAGS_END && slot_id >= EQ::invbag::BANK_BAGS_BEGIN) {
			auto temp_slot = (slot_id - EQ::invbag::BANK_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT;
			if (temp_slot >= GetInv().GetLookup()->InventoryTypeSize.Bank) {
				LogError("Item not sent to merchant4 : slot [{}]", slot_id);
				return;
			}
		}
	}

	// Serialize item into |-delimited string (Titanium- uses '|' delimiter .. newer clients use pure data serialization)
	std::string packet = inst->Serialize(slot_id);

	EmuOpcode opcode = OP_Unknown;
	EQApplicationPacket* outapp = nullptr;
	ItemPacket_Struct* itempacket = nullptr;

	// Construct packet
	opcode = (packet_type==ItemPacketViewLink) ? OP_ItemLinkResponse : OP_ItemPacket;
	outapp = new EQApplicationPacket(opcode, packet.length()+sizeof(ItemPacket_Struct));
	itempacket = (ItemPacket_Struct*)outapp->pBuffer;
	memcpy(itempacket->SerializedItem, packet.c_str(), packet.length());
	itempacket->PacketType = packet_type;

#if EQDEBUG >= 9
		DumpPacket(outapp);
#endif
	FastQueuePacket(&outapp);
}

static int16 BandolierSlotToWeaponSlot(int BandolierSlot)
{
	switch (BandolierSlot)
	{
	case bandolierPrimary:
		return EQ::invslot::slotPrimary;
	case bandolierSecondary:
		return EQ::invslot::slotSecondary;
	case bandolierRange:
		return EQ::invslot::slotRange;
	default:
		return EQ::invslot::slotAmmo;
	}
}

void Client::CreateBandolier(const EQApplicationPacket *app)
{
	// Store bandolier set with the number and name passed by the client, along with the items that are currently
	// in the players weapon slots.

	BandolierCreate_Struct *bs = (BandolierCreate_Struct*)app->pBuffer;

	LogInventory("Char: [{}] Creating Bandolier Set [{}], Set Name: [{}]", GetName(), bs->Number, bs->Name);
	strcpy(m_pp.bandoliers[bs->Number].Name, bs->Name);

	const EQ::ItemInstance* InvItem = nullptr;
	const EQ::ItemData *BaseItem = nullptr;
	int16 WeaponSlot = 0;

	database.DeleteCharacterBandolier(CharacterID(), bs->Number);

	for(int BandolierSlot = bandolierPrimary; BandolierSlot <= bandolierAmmo; BandolierSlot++) {
		WeaponSlot = BandolierSlotToWeaponSlot(BandolierSlot);
		InvItem = GetInv()[WeaponSlot];
		if(InvItem) {
			BaseItem = InvItem->GetItem();
			LogInventory("Char: [{}] adding item [{}] to slot [{}]", GetName(),BaseItem->Name, WeaponSlot);
			m_pp.bandoliers[bs->Number].Items[BandolierSlot].ID = BaseItem->ID;
			m_pp.bandoliers[bs->Number].Items[BandolierSlot].Icon = BaseItem->Icon;
			strncpy(m_pp.bandoliers[bs->Number].Items[BandolierSlot].Name, BaseItem->Name, sizeof(m_pp.bandoliers[bs->Number].Items[BandolierSlot].Name));
			database.SaveCharacterBandolier(CharacterID(), bs->Number, BandolierSlot, m_pp.bandoliers[bs->Number].Items[BandolierSlot].ID, m_pp.bandoliers[bs->Number].Items[BandolierSlot].Icon, bs->Name);
		}
		else {
			LogInventory("Char: [{}] no item in slot [{}]", GetName(), WeaponSlot);
			m_pp.bandoliers[bs->Number].Items[BandolierSlot].ID = 0;
			m_pp.bandoliers[bs->Number].Items[BandolierSlot].Icon = 0;
			m_pp.bandoliers[bs->Number].Items[BandolierSlot].Name[0] = '\0';
		}
	}
}

void Client::RemoveBandolier(const EQApplicationPacket *app)
{
	BandolierDelete_Struct *bds = (BandolierDelete_Struct*)app->pBuffer;
	LogInventory("Char: [{}] removing set", GetName(), bds->Number);
	memset(m_pp.bandoliers[bds->Number].Name, 0, 32);
	for(int i = bandolierPrimary; i <= bandolierAmmo; i++) {
		m_pp.bandoliers[bds->Number].Items[i].ID = 0;
		m_pp.bandoliers[bds->Number].Items[i].Icon = 0;
	}
	database.DeleteCharacterBandolier(CharacterID(), bds->Number);
}

void Client::SetBandolier(const EQApplicationPacket *app)
{
	// Swap the weapons in the given bandolier set into the character's weapon slots and return
	// any items currently in the weapon slots to inventory.

	BandolierSet_Struct *bss = (BandolierSet_Struct*)app->pBuffer;
	LogInventory("Char: [{}] activating set [{}]", GetName(), bss->Number);
	int16 slot = 0;
	int16 WeaponSlot = 0;
	EQ::ItemInstance *BandolierItems[4]; // Temporary holding area for the weapons we pull out of their inventory

	// First we pull the items for this bandolier set out of their inventory, this makes space to put the
	// currently equipped items back.
	for(int BandolierSlot = bandolierPrimary; BandolierSlot <= bandolierAmmo; BandolierSlot++) {
		// If this bandolier set has an item in this position
		if(m_pp.bandoliers[bss->Number].Items[BandolierSlot].ID) {
			WeaponSlot = BandolierSlotToWeaponSlot(BandolierSlot);

			// Check if the player has the item specified in the bandolier set on them.
			//
			slot = m_inv.HasItem(m_pp.bandoliers[bss->Number].Items[BandolierSlot].ID, 1,
							invWhereWorn|invWherePersonal);

			// removed 'invWhereCursor' argument from above and implemented slots 30, 331-340 checks here
			if (slot == INVALID_INDEX) {
				if (m_inv.GetItem(EQ::invslot::slotCursor)) {
					// Below used to check charges but for some reason
					// m_inv.GetItem(EQ::invslot::slotCursor)->GetCharges()
					// is returning 0.  We know it exists, so removed check.
					// TODO: Why is 0 being returned for slotCursor.
					if (m_inv.GetItem(EQ::invslot::slotCursor)->GetItem()->ID == m_pp.bandoliers[bss->Number].Items[BandolierSlot].ID) {
						slot = EQ::invslot::slotCursor;
					}
					else if (m_inv.GetItem(EQ::invslot::slotCursor)->GetItem()->ItemClass == 1) {
						for(int16 CursorBagSlot = EQ::invbag::CURSOR_BAG_BEGIN; CursorBagSlot <= EQ::invbag::CURSOR_BAG_END; CursorBagSlot++) {
							if (m_inv.GetItem(CursorBagSlot)) {
								if (m_inv.GetItem(CursorBagSlot)->GetItem()->ID == m_pp.bandoliers[bss->Number].Items[BandolierSlot].ID &&
									m_inv.GetItem(CursorBagSlot)->GetCharges() >= 1) { // ditto
										slot = CursorBagSlot;
										break;
								}
							}
						}
					}
				}
			}

			// if the player has this item in their inventory,
			if (slot != INVALID_INDEX) {
				// Pull the item out of the inventory
				BandolierItems[BandolierSlot] = m_inv.PopItem(slot);
				// If ammo with charges, only take one charge out to put in the range slot, that is what
				// the client does.

				if(((BandolierSlot == bandolierAmmo) || (BandolierSlot == bandolierRange)) &&
					BandolierItems[BandolierSlot] && BandolierItems[BandolierSlot]->IsStackable()){
					int Charges = BandolierItems[BandolierSlot]->GetCharges();
					// If there is more than one charge
					if(Charges > 1) {
						BandolierItems[BandolierSlot]->SetCharges(Charges-1);
						// Take one charge out and put the rest back
						m_inv.PutItem(slot, *BandolierItems[BandolierSlot]);
						database.SaveInventory(character_id, BandolierItems[BandolierSlot], slot);
						BandolierItems[BandolierSlot]->SetCharges(1);
					}
					else { // Remove the item from the inventory
						database.SaveInventory(character_id, 0, slot);
					}
				}
				else { // Remove the item from the inventory
					database.SaveInventory(character_id, 0, slot);
				}
			}
			else { // The player doesn't have the required weapon with them.
				BandolierItems[BandolierSlot] = 0;
				if (slot == INVALID_INDEX) {
					LogInventory("Character does not have required bandolier item for slot [{}]", WeaponSlot);
					EQ::ItemInstance *InvItem = m_inv.PopItem(WeaponSlot);
					if(InvItem) {
						// If there was an item in that weapon slot, put it in the inventory
						LogInventory("returning item [{}] in weapon slot [{}] to inventory",
						InvItem->GetItem()->Name, WeaponSlot);
						LogInventory("returning item [{}] in weapon slot [{}] to inventory", InvItem->GetItem()->Name, WeaponSlot);
						if (MoveItemToInventory(InvItem)) {
							database.SaveInventory(character_id, 0, WeaponSlot);
							LogError("returning item [{}] in weapon slot [{}] to inventory", InvItem->GetItem()->Name, WeaponSlot);
						}
						else {
							LogError("Char: [{}], ERROR returning [{}] to inventory", GetName(), InvItem->GetItem()->Name);
						}
						safe_delete(InvItem);
					}
				}
			}
		}
	}

	// Now we move the required weapons into the character weapon slots, and return any items we are replacing
	// back to inventory.
	//
	for(int BandolierSlot = bandolierPrimary; BandolierSlot <= bandolierAmmo; BandolierSlot++) {

		// Find the inventory slot corresponding to this bandolier slot

		WeaponSlot = BandolierSlotToWeaponSlot(BandolierSlot);

		// if there is an item in this Bandolier slot ?
		if(m_pp.bandoliers[bss->Number].Items[BandolierSlot].ID) {
			// if the player has this item in their inventory, and it is not already where it needs to be
			if(BandolierItems[BandolierSlot]) {
				// Pull the item that we are going to replace
				EQ::ItemInstance *InvItem = m_inv.PopItem(WeaponSlot);
				// Put the item specified in the bandolier where it needs to be
				m_inv.PutItem(WeaponSlot, *BandolierItems[BandolierSlot]);

				safe_delete(BandolierItems[BandolierSlot]);
				// Update the database, save the item now in the weapon slot
				database.SaveInventory(character_id, m_inv.GetItem(WeaponSlot), WeaponSlot);

				if(InvItem) {
					// If there was already an item in that weapon slot that we replaced, find a place to put it
					if (!MoveItemToInventory(InvItem)) {
						LogError("Char: [{}], ERROR returning [{}] to inventory", GetName(), InvItem->GetItem()->Name);
					}
					safe_delete(InvItem);
				}
			}
		}
		else {
			// This bandolier set has no item for this slot, so take whatever is in the weapon slot and
			// put it in the player's inventory.
			EQ::ItemInstance *InvItem = m_inv.PopItem(WeaponSlot);
			if(InvItem) {
				LogInventory("Bandolier has no item for slot [{}], returning item [{}] to inventory", WeaponSlot, InvItem->GetItem()->Name);
				// If there was an item in that weapon slot, put it in the inventory
				if (MoveItemToInventory(InvItem)) {
					database.SaveInventory(character_id, 0, WeaponSlot);
				}
				else {
					LogError("Char: [{}], ERROR returning [{}] to inventory", GetName(), InvItem->GetItem()->Name);
				}
				safe_delete(InvItem);
			}
		}
	}

	if (RuleI(Character, BandolierSwapDelay) > 0) {
		bandolier_throttle_timer.Start(RuleI(Character, BandolierSwapDelay));
	}

	// finally, recalculate any stat bonuses from the item change
	CalcBonuses();
}

bool Client::MoveItemToInventory(EQ::ItemInstance *ItemToReturn, bool UpdateClient) {

	// This is a support function for Client::SetBandolier, however it can be used anywhere it's functionality is required.
	//
	// When the client moves items around as Bandolier sets are activated, it does not send details to the
	// server of what item it has moved to which slot. It assumes the server knows what it will do.
	//
	// The standard EQEmu auto inventory routines do not behave as the client does when manipulating bandoliers.
	// The client will look in each main inventory slot. If it finds a bag in a slot, it will then look inside
	// the bag for a free slot.
	//
	// This differs from the standard EQEmu method of looking in all 8 inventory slots first to find an empty slot, and
	// then going back and looking in bags. There are also other differences related to how it moves stackable items back
	// to inventory.
	//
	// Rather than alter the current auto inventory behaviour, just in case something
	// depends on current behaviour, this routine operates the same as the client when moving items back to inventory when
	// swapping bandolier sets.

	if (!ItemToReturn) {
		return false;
	}

	LogInventory("Char: [{}] Returning [{}] to inventory", GetName(), ItemToReturn->GetItem()->Name);

	uint32 ItemID = ItemToReturn->GetItem()->ID;

	// If the item is stackable (ammo in range slot), try stacking it with other items of the same type
	//
	if(ItemToReturn->IsStackable()) {

		for (int16 i = EQ::invslot::GENERAL_BEGIN; i <= EQ::invslot::slotCursor; i++) { // changed slot max to 30 from 29. client will stack into slot 30 (bags too) before moving.
			if ((((uint64)1 << i) & GetInv().GetLookup()->PossessionsBitmask) == 0)
				continue;

			EQ::ItemInstance* InvItem = m_inv.GetItem(i);

			if(InvItem && (InvItem->GetItem()->ID == ItemID) && (InvItem->GetCharges() < InvItem->GetItem()->StackSize)) {

				int ChargeSlotsLeft = InvItem->GetItem()->StackSize - InvItem->GetCharges();

				int ChargesToMove = ItemToReturn->GetCharges() < ChargeSlotsLeft ? ItemToReturn->GetCharges() : ChargeSlotsLeft;

				InvItem->SetCharges(InvItem->GetCharges() + ChargesToMove);

				if(UpdateClient)
					SendItemPacket(i, InvItem, ItemPacketTrade);

				database.SaveInventory(character_id, m_inv.GetItem(i), i);

				ItemToReturn->SetCharges(ItemToReturn->GetCharges() - ChargesToMove);

				if(!ItemToReturn->GetCharges())
					return true;
			}
			// If there is a bag in this slot, look inside it.
			//
			if (InvItem && InvItem->IsClassBag()) {

				int16 BaseSlotID = EQ::InventoryProfile::CalcSlotId(i, EQ::invbag::SLOT_BEGIN);

				uint8 BagSize=InvItem->GetItem()->BagSlots;

				uint8 BagSlot;
				for (BagSlot = EQ::invbag::SLOT_BEGIN; BagSlot < BagSize; BagSlot++) {
					InvItem = m_inv.GetItem(BaseSlotID + BagSlot);
					if (InvItem && (InvItem->GetItem()->ID == ItemID) &&
						(InvItem->GetCharges() < InvItem->GetItem()->StackSize)) {

						int ChargeSlotsLeft = InvItem->GetItem()->StackSize - InvItem->GetCharges();

						int ChargesToMove = ItemToReturn->GetCharges() < ChargeSlotsLeft ? ItemToReturn->GetCharges() : ChargeSlotsLeft;

						InvItem->SetCharges(InvItem->GetCharges() + ChargesToMove);

						if(UpdateClient)
							SendItemPacket(BaseSlotID + BagSlot, m_inv.GetItem(BaseSlotID + BagSlot), ItemPacketTrade);

						database.SaveInventory(character_id, m_inv.GetItem(BaseSlotID + BagSlot), BaseSlotID + BagSlot);

						ItemToReturn->SetCharges(ItemToReturn->GetCharges() - ChargesToMove);

						if(!ItemToReturn->GetCharges())
							return true;
					}
				}
			}
		}
	}

	// We have tried stacking items, now just try and find an empty slot.

	for (int16 i = EQ::invslot::GENERAL_BEGIN; i <= EQ::invslot::slotCursor; i++) { // changed slot max to 30 from 29. client will move into slot 30 (bags too) before pushing onto cursor.
		if ((((uint64)1 << i) & GetInv().GetLookup()->PossessionsBitmask) == 0)
			continue;

		EQ::ItemInstance* InvItem = m_inv.GetItem(i);

		if (!InvItem) {
			// Found available slot in personal inventory
			m_inv.PutItem(i, *ItemToReturn);

			if(UpdateClient)
				SendItemPacket(i, ItemToReturn, ItemPacketTrade);

			database.SaveInventory(character_id, m_inv.GetItem(i), i);

			LogInventory("Char: [{}] Storing in main inventory slot [{}]", GetName(), i);

			return true;
		}
		if (InvItem->IsClassBag() && EQ::InventoryProfile::CanItemFitInContainer(ItemToReturn->GetItem(), InvItem->GetItem())) {

			int16 BaseSlotID = EQ::InventoryProfile::CalcSlotId(i, EQ::invbag::SLOT_BEGIN);

			uint8 BagSize=InvItem->GetItem()->BagSlots;

			for (uint8 BagSlot = EQ::invbag::SLOT_BEGIN; BagSlot < BagSize; BagSlot++) {

				InvItem = m_inv.GetItem(BaseSlotID + BagSlot);

				if (!InvItem) {
					// Found available slot within bag
					m_inv.PutItem(BaseSlotID + BagSlot, *ItemToReturn);

					if(UpdateClient)
						SendItemPacket(BaseSlotID + BagSlot, ItemToReturn, ItemPacketTrade);

					database.SaveInventory(character_id, m_inv.GetItem(BaseSlotID + BagSlot), BaseSlotID + BagSlot);

					LogInventory("Char: [{}] Storing in bag slot [{}]", GetName(), BaseSlotID + BagSlot);

					return true;
				}
			}
		}
	}

	// Store on the cursor
	//
	LogInventory("Char: [{}] No space, putting on the cursor", GetName());

	PushItemOnCursor(*ItemToReturn, UpdateClient);

	return true;
}

bool Client::InterrogateInventory(Client* requester, bool log, bool silent, bool allowtrip, bool& error, bool autolog)
{
	if (!requester)
		return false;

	std::map<int16, const EQ::ItemInstance*> instmap;

	// build reference map
	for (int16 index = EQ::invslot::POSSESSIONS_BEGIN; index <= EQ::invslot::POSSESSIONS_END; ++index) {
		auto inst = m_inv[index];
		if (inst == nullptr) { continue; }
		instmap[index] = inst;
	}
	for (int16 index = EQ::invslot::TRIBUTE_BEGIN; index <= EQ::invslot::TRIBUTE_END; ++index) {
		auto inst = m_inv[index];
		if (inst == nullptr) { continue; }
		instmap[index] = inst;
	}
	for (int16 index = EQ::invslot::GUILD_TRIBUTE_BEGIN; index <= EQ::invslot::GUILD_TRIBUTE_END; ++index) {
		auto inst = m_inv[index];
		if (inst == nullptr) { continue; }
		instmap[index] = inst;
	}
	for (int16 index = EQ::invslot::BANK_BEGIN; index <= EQ::invslot::BANK_END; ++index) {
		auto inst = m_inv[index];
		if (inst == nullptr) { continue; }
		instmap[index] = inst;
	}
	for (int16 index = EQ::invslot::SHARED_BANK_BEGIN; index <= EQ::invslot::SHARED_BANK_END; ++index) {
		auto inst = m_inv[index];
		if (inst == nullptr) { continue; }
		instmap[index] = inst;
	}
	for (int16 index = EQ::invslot::TRADE_BEGIN; index <= EQ::invslot::TRADE_END; ++index) {
		auto inst = m_inv[index];
		if (inst == nullptr) { continue; }
		instmap[index] = inst;
	}

	auto tsobject = GetTradeskillObject();
	if (tsobject != nullptr) {
		for (int16 index = EQ::invslot::SLOT_BEGIN; index < EQ::invtype::WORLD_SIZE; ++index) {
			auto inst = tsobject->GetItem(index);
			if (inst == nullptr) { continue; }
			instmap[EQ::invslot::WORLD_BEGIN + index] = inst;
		}
	}

	int limbo = 0;
	for (auto cursor_itr = m_inv.cursor_cbegin(); cursor_itr != m_inv.cursor_cend(); ++cursor_itr, ++limbo) {
		// m_inv.cursor_begin() is referenced as SlotCursor in MapPossessions above
		if (cursor_itr == m_inv.cursor_cbegin())
			continue;

		instmap[EQ::invbag::CURSOR_BAG_BEGIN + limbo] = *cursor_itr;
	}

	// call InterrogateInventory_ for error check
	for (auto instmap_itr = instmap.begin(); (instmap_itr != instmap.end()) && (!error); ++instmap_itr) {
		InterrogateInventory_(true, requester, instmap_itr->first, INVALID_INDEX, instmap_itr->second, nullptr, log, silent, error, 0);
	}

	if (autolog && error && (!log))
		log = true;

	if (log) {
		LogError("Client::InterrogateInventory() called for [{}] by [{}] with an error state of [{}]", GetName(), requester->GetName(), (error ? "TRUE" : "FALSE"));
	}
	if (!silent) {
		requester->Message(Chat::Default, "--- Inventory Interrogation Report for %s (requested by: %s, error state: %s) ---", GetName(), requester->GetName(), (error ? "TRUE" : "FALSE"));
	}

	// call InterrogateInventory_ for report
	for (auto instmap_itr = instmap.begin(); (instmap_itr != instmap.end()); ++instmap_itr) {
		InterrogateInventory_(false, requester, instmap_itr->first, INVALID_INDEX, instmap_itr->second, nullptr, log, silent, error, 0);
	}

	if (error) {
		Message(Chat::Red, "An error has been discovered in your inventory!");
		Message(Chat::Red, "Do not log out, zone or re-arrange items until this");
		Message(Chat::Red, "issue has been resolved or item loss may occur!");

		if (allowtrip)
			TripInterrogateInvState();
	}

	if (log) {
		LogError("Target interrogate inventory flag: [{}]", (GetInterrogateInvState() ? "TRUE" : "FALSE"));
	}
	if (!silent) {
		requester->Message(Chat::Default, "Target interrogation flag: %s", (GetInterrogateInvState() ? "TRUE" : "FALSE"));
		requester->Message(Chat::Default, "--- End of Interrogation Report ---");
	}

	instmap.clear();

	return true;
}

void Client::InterrogateInventory_(bool errorcheck, Client* requester, int16 head, int16 index, const EQ::ItemInstance* inst, const EQ::ItemInstance* parent, bool log, bool silent, bool &error, int depth)
{
	if (depth >= 10) {
		LogDebug("[CLIENT] Client::InterrogateInventory_() - Recursion count has exceeded the maximum allowable (You have a REALLY BIG PROBLEM!!)");
		return;
	}

	if (errorcheck) {
		if (InterrogateInventory_error(head, index, inst, parent, depth)) {
			error = true;
		}
		else {
			if (inst) {
				for (int16 sub = EQ::invbag::SLOT_BEGIN; (sub <= EQ::invbag::SLOT_END) && (!error); ++sub) { // treat any EQ::ItemInstance as having the max internal slots available
					if (inst->GetItem(sub))
						InterrogateInventory_(true, requester, head, sub, inst->GetItem(sub), inst, log, silent, error, depth + 1);
				}
			}
		}
	}
	else {
		bool localerror = InterrogateInventory_error(head, index, inst, parent, depth);
		std::string i;
		std::string p;
		std::string e;

		if (inst) { i = StringFormat("%s (id: %u, cls: %u, aug_t: %u)", inst->GetItem()->Name, inst->GetItem()->ID, inst->GetItem()->ItemClass, inst->GetItem()->AugType); }
		else { i = "NONE"; }
		if (parent) { p = StringFormat("%s (id: %u, cls: %u, aug_t: %u), index: %i", parent->GetItem()->Name, parent->GetItem()->ID, parent->GetItem()->ItemClass, parent->GetItem()->AugType, index); }
		else { p = "NONE"; }
		if (localerror) { e = " [ERROR]"; }
		else { e = ""; }

		if (log) {
			LogError("Head: [{}], Depth: [{}], Instance: [{}], Parent: [{}][{}]",
				head, depth, i.c_str(), p.c_str(), e.c_str());
		}
		if (!silent) {
			requester->Message(Chat::Gray, "%i:%i - inst: %s - parent: %s%s",
				head, depth, i.c_str(), p.c_str(), e.c_str());
		}

		if (inst) {
			for (int16 sub = EQ::invbag::SLOT_BEGIN; (sub <= EQ::invbag::SLOT_END); ++sub) {
				if (inst->GetItem(sub))
					InterrogateInventory_(false, requester, head, sub, inst->GetItem(sub), inst, log, silent, error, depth + 1);
			}
		}
	}

	return;
}

bool Client::InterrogateInventory_error(int16 head, int16 index, const EQ::ItemInstance* inst, const EQ::ItemInstance* parent, int depth)
{
	// very basic error checking - can be elaborated upon if more in-depth testing is needed...

	if (
		EQ::ValueWithin(head, EQ::invslot::EQUIPMENT_BEGIN, EQ::invslot::EQUIPMENT_END) ||
		EQ::ValueWithin(head, EQ::invslot::TRIBUTE_BEGIN, EQ::invslot::TRIBUTE_END) ||
		EQ::ValueWithin(head, EQ::invslot::GUILD_TRIBUTE_BEGIN, EQ::invslot::GUILD_TRIBUTE_END) ||
		EQ::ValueWithin(head, EQ::invslot::WORLD_BEGIN, EQ::invslot::WORLD_END) ||
		EQ::ValueWithin(head, EQ::invbag::CURSOR_BAG_BEGIN, EQ::invbag::CURSOR_BAG_END)
	) {
		switch (depth)
		{
		case 0: // requirement: inst is extant
			if (!inst)
				return true;
			break;
		case 1: // requirement: parent is common and inst is augment
			if ((!parent) || (!inst))
				return true;
			if (!parent->IsType(EQ::item::ItemClassCommon))
				return true;
			if (index > EQ::invaug::SOCKET_END)
				return true;
			break;
		default: // requirement: none (something bad happened...)
			return true;
		}
	}
	else if (
		(head >= EQ::invslot::GENERAL_BEGIN && head <= EQ::invslot::GENERAL_END) ||
		(head == EQ::invslot::slotCursor) ||
		(head >= EQ::invslot::BANK_BEGIN && head <= EQ::invslot::BANK_END) ||
		(head >= EQ::invslot::SHARED_BANK_BEGIN && head <= EQ::invslot::SHARED_BANK_END) ||
		(head >= EQ::invslot::TRADE_BEGIN && head <= EQ::invslot::TRADE_END)) {
		switch (depth)
		{
		case 0: // requirement: inst is extant
			if (!inst)
				return true;
			break;
		case 1: // requirement: parent is common and inst is augment ..or.. parent is container and inst is extant
			if ((!parent) || (!inst))
				return true;
			if (parent->IsType(EQ::item::ItemClassBag))
				break;
			if (parent->IsClassBook())
				return true;
			if (parent->IsClassCommon()) {
				if (!(inst->GetItem()->AugType > 0))
					return true;
				if (index > EQ::invaug::SOCKET_END)
					return true;
			}
			break;
		case 2: // requirement: parent is common and inst is augment
			if ((!parent) || (!inst))
				return true;
			if (parent->IsType(EQ::item::ItemClassBag))
				return true;
			if (parent->IsClassBook())
				return true;
			if (parent->IsClassCommon()) {
				if (!(inst->GetItem()->AugType > 0))
					return true;
				if (index > EQ::invaug::SOCKET_END)
					return true;
			}
			break;
		default: // requirement: none (something bad happened again...)
			return true;
		}
	}
	else {
		return true;
	}

	return false;
}

void EQ::InventoryProfile::SetCustomItemData(uint32 character_id, int16 slot_id, const std::string &identifier, const std::string &value) {
	EQ::ItemInstance *inst = GetItem(slot_id);
	if(inst) {
		inst->SetCustomData(identifier, value);
		database.SaveInventory(character_id, inst, slot_id);
	}
}

void EQ::InventoryProfile::SetCustomItemData(uint32 character_id, int16 slot_id, const std::string &identifier, int value) {
	EQ::ItemInstance *inst = GetItem(slot_id);
	if(inst) {
		inst->SetCustomData(identifier, value);
		database.SaveInventory(character_id, inst, slot_id);
	}
}

void EQ::InventoryProfile::SetCustomItemData(uint32 character_id, int16 slot_id, const std::string &identifier, float value) {
	EQ::ItemInstance *inst = GetItem(slot_id);
	if(inst) {
		inst->SetCustomData(identifier, value);
		database.SaveInventory(character_id, inst, slot_id);
	}
}

void EQ::InventoryProfile::SetCustomItemData(uint32 character_id, int16 slot_id, const std::string &identifier, bool value) {
	EQ::ItemInstance *inst = GetItem(slot_id);
	if(inst) {
		inst->SetCustomData(identifier, value);
		database.SaveInventory(character_id, inst, slot_id);
	}
}

std::string EQ::InventoryProfile::GetCustomItemData(int16 slot_id, const std::string &identifier) {
	EQ::ItemInstance *inst = GetItem(slot_id);
	if(inst) {
		return inst->GetCustomData(identifier);
	}
	return "";
}

const int EQ::InventoryProfile::GetItemStatValue(uint32 item_id, const std::string &identifier) {
	if (identifier.empty()) {
		return 0;
	}

	const auto* inst = database.CreateItem(item_id);
	if (!inst) {
		return 0;
	}

	const auto* item = inst->GetItem();
	if (!item) {
		return 0;
	}

	int stat = 0;

	if (Strings::EqualFold(identifier, "itemclass")) {
		stat = static_cast<int>(item->ItemClass);
	}

	if (Strings::EqualFold(identifier, "id")) {
		stat = static_cast<int>(item->ID);
	}

	if (Strings::EqualFold(identifier, "idfile")) {
		stat = Strings::IsNumber(&item->IDFile[2]) ? Strings::ToInt(&item->IDFile[2]) : 0;
	}

	if (Strings::EqualFold(identifier, "weight")) {
		stat = static_cast<int>(item->Weight);
	}

	if (Strings::EqualFold(identifier, "norent")) {
		stat = static_cast<int>(item->NoRent);
	}

	if (Strings::EqualFold(identifier, "nodrop")) {
		stat = static_cast<int>(item->NoDrop);
	}

	if (Strings::EqualFold(identifier, "size")) {
		stat = static_cast<int>(item->Size);
	}

	if (Strings::EqualFold(identifier, "slots")) {
		stat = static_cast<int>(item->Slots);
	}

	if (Strings::EqualFold(identifier, "price")) {
		stat = static_cast<int>(item->Price);
	}

	if (Strings::EqualFold(identifier, "icon")) {
		stat = static_cast<int>(item->Icon);
	}

	if (Strings::EqualFold(identifier, "loregroup")) {
		stat = static_cast<int>(item->LoreGroup);
	}

	if (Strings::EqualFold(identifier, "loreflag")) {
		stat = static_cast<int>(item->LoreFlag);
	}

	if (Strings::EqualFold(identifier, "pendingloreflag")) {
		stat = static_cast<int>(item->PendingLoreFlag);
	}

	if (Strings::EqualFold(identifier, "artifactflag")) {
		stat = static_cast<int>(item->ArtifactFlag);
	}

	if (Strings::EqualFold(identifier, "summonedflag")) {
		stat = static_cast<int>(item->SummonedFlag);
	}

	if (Strings::EqualFold(identifier, "fvnodrop")) {
		stat = static_cast<int>(item->FVNoDrop);
	}

	if (Strings::EqualFold(identifier, "favor")) {
		stat = static_cast<int>(item->Favor);
	}

	if (Strings::EqualFold(identifier, "guildfavor")) {
		stat = static_cast<int>(item->GuildFavor);
	}

	if (Strings::EqualFold(identifier, "pointtype")) {
		stat = static_cast<int>(item->PointType);
	}

	if (Strings::EqualFold(identifier, "bagtype")) {
		stat = static_cast<int>(item->BagType);
	}

	if (Strings::EqualFold(identifier, "bagslots")) {
		stat = static_cast<int>(item->BagSlots);
	}

	if (Strings::EqualFold(identifier, "bagsize")) {
		stat = static_cast<int>(item->BagSize);
	}

	if (Strings::EqualFold(identifier, "bagwr")) {
		stat = static_cast<int>(item->BagWR);
	}

	if (Strings::EqualFold(identifier, "benefitflag")) {
		stat = static_cast<int>(item->BenefitFlag);
	}

	if (Strings::EqualFold(identifier, "tradeskills")) {
		stat = static_cast<int>(item->Tradeskills);
	}

	if (Strings::EqualFold(identifier, "cr")) {
		stat = static_cast<int>(item->CR);
	}

	if (Strings::EqualFold(identifier, "dr")) {
		stat = static_cast<int>(item->DR);
	}

	if (Strings::EqualFold(identifier, "pr")) {
		stat = static_cast<int>(item->PR);
	}

	if (Strings::EqualFold(identifier, "mr")) {
		stat = static_cast<int>(item->MR);
	}

	if (Strings::EqualFold(identifier, "fr")) {
		stat = static_cast<int>(item->FR);
	}

	if (Strings::EqualFold(identifier, "astr")) {
		stat = static_cast<int>(item->AStr);
	}

	if (Strings::EqualFold(identifier, "asta")) {
		stat = static_cast<int>(item->ASta);
	}

	if (Strings::EqualFold(identifier, "aagi")) {
		stat = static_cast<int>(item->AAgi);
	}

	if (Strings::EqualFold(identifier, "adex")) {
		stat = static_cast<int>(item->ADex);
	}

	if (Strings::EqualFold(identifier, "acha")) {
		stat = static_cast<int>(item->ACha);
	}

	if (Strings::EqualFold(identifier, "aint")) {
		stat = static_cast<int>(item->AInt);
	}

	if (Strings::EqualFold(identifier, "awis")) {
		stat = static_cast<int>(item->AWis);
	}

	if (Strings::EqualFold(identifier, "hp")) {
		stat = static_cast<int>(item->HP);
	}

	if (Strings::EqualFold(identifier, "mana")) {
		stat = static_cast<int>(item->Mana);
	}

	if (Strings::EqualFold(identifier, "ac")) {
		stat = static_cast<int>(item->AC);
	}

	if (Strings::EqualFold(identifier, "deity")) {
		stat = static_cast<int>(item->Deity);
	}

	if (Strings::EqualFold(identifier, "skillmodvalue")) {
		stat = static_cast<int>(item->SkillModValue);
	}

	if (Strings::EqualFold(identifier, "skillmodtype")) {
		stat = static_cast<int>(item->SkillModType);
	}

	if (Strings::EqualFold(identifier, "banedmgrace")) {
		stat = static_cast<int>(item->BaneDmgRace);
	}

	if (Strings::EqualFold(identifier, "banedmgamt")) {
		stat = static_cast<int>(item->BaneDmgAmt);
	}

	if (Strings::EqualFold(identifier, "banedmgbody")) {
		stat = static_cast<int>(item->BaneDmgBody);
	}

	if (Strings::EqualFold(identifier, "magic")) {
		stat = static_cast<int>(item->Magic);
	}

	if (Strings::EqualFold(identifier, "casttime_")) {
		stat = static_cast<int>(item->CastTime_);
	}

	if (Strings::EqualFold(identifier, "reqlevel")) {
		stat = static_cast<int>(item->ReqLevel);
	}

	if (Strings::EqualFold(identifier, "bardtype")) {
		stat = static_cast<int>(item->BardType);
	}

	if (Strings::EqualFold(identifier, "bardvalue")) {
		stat = static_cast<int>(item->BardValue);
	}

	if (Strings::EqualFold(identifier, "light")) {
		stat = static_cast<int>(item->Light);
	}

	if (Strings::EqualFold(identifier, "delay")) {
		stat = static_cast<int>(item->Delay);
	}

	if (Strings::EqualFold(identifier, "reclevel")) {
		stat = static_cast<int>(item->RecLevel);
	}

	if (Strings::EqualFold(identifier, "recskill")) {
		stat = static_cast<int>(item->RecSkill);
	}

	if (Strings::EqualFold(identifier, "elemdmgtype")) {
		stat = static_cast<int>(item->ElemDmgType);
	}

	if (Strings::EqualFold(identifier, "elemdmgamt")) {
		stat = static_cast<int>(item->ElemDmgAmt);
	}

	if (Strings::EqualFold(identifier, "range")) {
		stat = static_cast<int>(item->Range);
	}

	if (Strings::EqualFold(identifier, "damage")) {
		stat = static_cast<int>(item->Damage);
	}

	if (Strings::EqualFold(identifier, "color")) {
		stat = static_cast<int>(item->Color);
	}

	if (Strings::EqualFold(identifier, "classes")) {
		stat = static_cast<int>(item->Classes);
	}

	if (Strings::EqualFold(identifier, "races")) {
		stat = static_cast<int>(item->Races);
	}

	if (Strings::EqualFold(identifier, "maxcharges")) {
		stat = static_cast<int>(item->MaxCharges);
	}

	if (Strings::EqualFold(identifier, "itemtype")) {
		stat = static_cast<int>(item->ItemType);
	}

	if (Strings::EqualFold(identifier, "material")) {
		stat = static_cast<int>(item->Material);
	}

	if (Strings::EqualFold(identifier, "casttime")) {
		stat = static_cast<int>(item->CastTime);
	}

	if (Strings::EqualFold(identifier, "elitematerial")) {
		stat = static_cast<int>(item->EliteMaterial);
	}

	if (Strings::EqualFold(identifier, "herosforgemodel")) {
		stat = static_cast<int>(item->HerosForgeModel);
	}

	if (Strings::EqualFold(identifier, "procrate")) {
		stat = static_cast<int>(item->ProcRate);
	}

	if (Strings::EqualFold(identifier, "combateffects")) {
		stat = static_cast<int>(item->CombatEffects);
	}

	if (Strings::EqualFold(identifier, "shielding")) {
		stat = static_cast<int>(item->Shielding);
	}

	if (Strings::EqualFold(identifier, "stunresist")) {
		stat = static_cast<int>(item->StunResist);
	}

	if (Strings::EqualFold(identifier, "strikethrough")) {
		stat = static_cast<int>(item->StrikeThrough);
	}

	if (Strings::EqualFold(identifier, "extradmgskill")) {
		stat = static_cast<int>(item->ExtraDmgSkill);
	}

	if (Strings::EqualFold(identifier, "extradmgamt")) {
		stat = static_cast<int>(item->ExtraDmgAmt);
	}

	if (Strings::EqualFold(identifier, "spellshield")) {
		stat = static_cast<int>(item->SpellShield);
	}

	if (Strings::EqualFold(identifier, "avoidance")) {
		stat = static_cast<int>(item->Avoidance);
	}

	if (Strings::EqualFold(identifier, "accuracy")) {
		stat = static_cast<int>(item->Accuracy);
	}

	if (Strings::EqualFold(identifier, "charmfileid")) {
		stat = static_cast<int>(item->CharmFileID);
	}

	if (Strings::EqualFold(identifier, "factionmod1")) {
		stat = static_cast<int>(item->FactionMod1);
	}

	if (Strings::EqualFold(identifier, "factionmod2")) {
		stat = static_cast<int>(item->FactionMod2);
	}

	if (Strings::EqualFold(identifier, "factionmod3")) {
		stat = static_cast<int>(item->FactionMod3);
	}

	if (Strings::EqualFold(identifier, "factionmod4")) {
		stat = static_cast<int>(item->FactionMod4);
	}

	if (Strings::EqualFold(identifier, "factionamt1")) {
		stat = static_cast<int>(item->FactionAmt1);
	}

	if (Strings::EqualFold(identifier, "factionamt2")) {
		stat = static_cast<int>(item->FactionAmt2);
	}

	if (Strings::EqualFold(identifier, "factionamt3")) {
		stat = static_cast<int>(item->FactionAmt3);
	}

	if (Strings::EqualFold(identifier, "factionamt4")) {
		stat = static_cast<int>(item->FactionAmt4);
	}

	if (Strings::EqualFold(identifier, "augtype")) {
		stat = static_cast<int>(item->AugType);
	}

	if (Strings::EqualFold(identifier, "ldontheme")) {
		stat = static_cast<int>(item->LDoNTheme);
	}

	if (Strings::EqualFold(identifier, "ldonprice")) {
		stat = static_cast<int>(item->LDoNPrice);
	}

	if (Strings::EqualFold(identifier, "ldonsold")) {
		stat = static_cast<int>(item->LDoNSold);
	}

	if (Strings::EqualFold(identifier, "banedmgraceamt")) {
		stat = static_cast<int>(item->BaneDmgRaceAmt);
	}

	if (Strings::EqualFold(identifier, "augrestrict")) {
		stat = static_cast<int>(item->AugRestrict);
	}

	if (Strings::EqualFold(identifier, "endur")) {
		stat = static_cast<int>(item->Endur);
	}

	if (Strings::EqualFold(identifier, "dotshielding")) {
		stat = static_cast<int>(item->DotShielding);
	}

	if (Strings::EqualFold(identifier, "attack")) {
		stat = static_cast<int>(item->Attack);
	}

	if (Strings::EqualFold(identifier, "regen")) {
		stat = static_cast<int>(item->Regen);
	}

	if (Strings::EqualFold(identifier, "manaregen")) {
		stat = static_cast<int>(item->ManaRegen);
	}

	if (Strings::EqualFold(identifier, "enduranceregen")) {
		stat = static_cast<int>(item->EnduranceRegen);
	}

	if (Strings::EqualFold(identifier, "haste")) {
		stat = static_cast<int>(item->Haste);
	}

	if (Strings::EqualFold(identifier, "damageshield")) {
		stat = static_cast<int>(item->DamageShield);
	}

	if (Strings::EqualFold(identifier, "recastdelay")) {
		stat = static_cast<int>(item->RecastDelay);
	}

	if (Strings::EqualFold(identifier, "recasttype")) {
		stat = static_cast<int>(item->RecastType);
	}

	if (Strings::EqualFold(identifier, "augdistiller")) {
		stat = static_cast<int>(item->AugDistiller);
	}

	if (Strings::EqualFold(identifier, "attuneable")) {
		stat = static_cast<int>(item->Attuneable);
	}

	if (Strings::EqualFold(identifier, "nopet")) {
		stat = static_cast<int>(item->NoPet);
	}

	if (Strings::EqualFold(identifier, "potionbelt")) {
		stat = static_cast<int>(item->PotionBelt);
	}

	if (Strings::EqualFold(identifier, "stackable")) {
		stat = static_cast<int>(item->Stackable);
	}

	if (Strings::EqualFold(identifier, "notransfer")) {
		stat = static_cast<int>(item->NoTransfer);
	}

	if (Strings::EqualFold(identifier, "questitemflag")) {
		stat = static_cast<int>(item->QuestItemFlag);
	}

	if (Strings::EqualFold(identifier, "stacksize")) {
		stat = static_cast<int>(item->StackSize);
	}

	if (Strings::EqualFold(identifier, "potionbeltslots")) {
		stat = static_cast<int>(item->PotionBeltSlots);
	}

	if (Strings::EqualFold(identifier, "book")) {
		stat = static_cast<int>(item->Book);
	}

	if (Strings::EqualFold(identifier, "booktype")) {
		stat = static_cast<int>(item->BookType);
	}

	if (Strings::EqualFold(identifier, "svcorruption")) {
		stat = static_cast<int>(item->SVCorruption);
	}

	if (Strings::EqualFold(identifier, "purity")) {
		stat = static_cast<int>(item->Purity);
	}

	if (Strings::EqualFold(identifier, "backstabdmg")) {
		stat = static_cast<int>(item->BackstabDmg);
	}

	if (Strings::EqualFold(identifier, "dsmitigation")) {
		stat = static_cast<int>(item->DSMitigation);
	}

	if (Strings::EqualFold(identifier, "heroicstr")) {
		stat = static_cast<int>(item->HeroicStr);
	}

	if (Strings::EqualFold(identifier, "heroicint")) {
		stat = static_cast<int>(item->HeroicInt);
	}

	if (Strings::EqualFold(identifier, "heroicwis")) {
		stat = static_cast<int>(item->HeroicWis);
	}

	if (Strings::EqualFold(identifier, "heroicagi")) {
		stat = static_cast<int>(item->HeroicAgi);
	}

	if (Strings::EqualFold(identifier, "heroicdex")) {
		stat = static_cast<int>(item->HeroicDex);
	}

	if (Strings::EqualFold(identifier, "heroicsta")) {
		stat = static_cast<int>(item->HeroicSta);
	}

	if (Strings::EqualFold(identifier, "heroiccha")) {
		stat = static_cast<int>(item->HeroicCha);
	}

	if (Strings::EqualFold(identifier, "heroicmr")) {
		stat = static_cast<int>(item->HeroicMR);
	}

	if (Strings::EqualFold(identifier, "heroicfr")) {
		stat = static_cast<int>(item->HeroicFR);
	}

	if (Strings::EqualFold(identifier, "heroiccr")) {
		stat = static_cast<int>(item->HeroicCR);
	}

	if (Strings::EqualFold(identifier, "heroicdr")) {
		stat = static_cast<int>(item->HeroicDR);
	}

	if (Strings::EqualFold(identifier, "heroicpr")) {
		stat = static_cast<int>(item->HeroicPR);
	}

	if (Strings::EqualFold(identifier, "heroicsvcorrup")) {
		stat = static_cast<int>(item->HeroicSVCorrup);
	}

	if (Strings::EqualFold(identifier, "healamt")) {
		stat = static_cast<int>(item->HealAmt);
	}

	if (Strings::EqualFold(identifier, "spelldmg")) {
		stat = static_cast<int>(item->SpellDmg);
	}

	if (Strings::EqualFold(identifier, "ldonsellbackrate")) {
		stat = static_cast<int>(item->LDoNSellBackRate);
	}

	if (Strings::EqualFold(identifier, "scriptfileid")) {
		stat = static_cast<int>(item->ScriptFileID);
	}

	if (Strings::EqualFold(identifier, "expendablearrow")) {
		stat = static_cast<int>(item->ExpendableArrow);
	}

	if (Strings::EqualFold(identifier, "clairvoyance")) {
		stat = static_cast<int>(item->Clairvoyance);
	}

	if (Strings::EqualFold(identifier, "clickeffect")) {
		stat = static_cast<int>(item->Click.Effect);
	}

	if (Strings::EqualFold(identifier, "clicktype")) {
		stat = static_cast<int>(item->Click.Type);
	}

	if (Strings::EqualFold(identifier, "clicklevel")) {
		stat = static_cast<int>(item->Click.Level);
	}

	if (Strings::EqualFold(identifier, "clicklevel2")) {
		stat = static_cast<int>(item->Click.Level2);
	}

	if (Strings::EqualFold(identifier, "proceffect")) {
		stat = static_cast<int>(item->Proc.Effect);
	}

	if (Strings::EqualFold(identifier, "proctype")) {
		stat = static_cast<int>(item->Proc.Type);
	}

	if (Strings::EqualFold(identifier, "proclevel")) {
		stat = static_cast<int>(item->Proc.Level);
	}

	if (Strings::EqualFold(identifier, "proclevel2")) {
		stat = static_cast<int>(item->Proc.Level2);
	}

	if (Strings::EqualFold(identifier, "worneffect")) {
		stat = static_cast<int>(item->Worn.Effect);
	}

	if (Strings::EqualFold(identifier, "worntype")) {
		stat = static_cast<int>(item->Worn.Type);
	}

	if (Strings::EqualFold(identifier, "wornlevel")) {
		stat = static_cast<int>(item->Worn.Level);
	}

	if (Strings::EqualFold(identifier, "wornlevel2")) {
		stat = static_cast<int>(item->Worn.Level2);
	}

	if (Strings::EqualFold(identifier, "focuseffect")) {
		stat = static_cast<int>(item->Focus.Effect);
	}

	if (Strings::EqualFold(identifier, "focustype")) {
		stat = static_cast<int>(item->Focus.Type);
	}

	if (Strings::EqualFold(identifier, "focuslevel")) {
		stat = static_cast<int>(item->Focus.Level);
	}

	if (Strings::EqualFold(identifier, "focuslevel2")) {
		stat = static_cast<int>(item->Focus.Level2);
	}

	if (Strings::EqualFold(identifier, "scrolleffect")) {
		stat = static_cast<int>(item->Scroll.Effect);
	}

	if (Strings::EqualFold(identifier, "scrolltype")) {
		stat = static_cast<int>(item->Scroll.Type);
	}

	if (Strings::EqualFold(identifier, "scrolllevel")) {
		stat = static_cast<int>(item->Scroll.Level);
	}

	if (Strings::EqualFold(identifier, "scrolllevel2")) {
		stat = static_cast<int>(item->Scroll.Level2);
	}

	if (Strings::EqualFold(identifier, "augslot1type")) {
		stat = static_cast<int>(item->AugSlotType[0]);
	}

	if (Strings::EqualFold(identifier, "augslot2type")) {
		stat = static_cast<int>(item->AugSlotType[1]);
	}

	if (Strings::EqualFold(identifier, "augslot3type")) {
		stat = static_cast<int>(item->AugSlotType[2]);
	}

	if (Strings::EqualFold(identifier, "augslot4type")) {
		stat = static_cast<int>(item->AugSlotType[3]);
	}

	if (Strings::EqualFold(identifier, "augslot5type")) {
		stat = static_cast<int>(item->AugSlotType[4]);
	}

	if (Strings::EqualFold(identifier, "augslot6type")) {
		stat = static_cast<int>(item->AugSlotType[5]);
	}

	if (Strings::EqualFold(identifier, "augslot1visible")) {
		stat = static_cast<int>(item->AugSlotVisible[0]);
	}

	if (Strings::EqualFold(identifier, "augslot2visible")) {
		stat = static_cast<int>(item->AugSlotVisible[1]);
	}

	if (Strings::EqualFold(identifier, "augslot3visible")) {
		stat = static_cast<int>(item->AugSlotVisible[2]);
	}

	if (Strings::EqualFold(identifier, "augslot4visible")) {
		stat = static_cast<int>(item->AugSlotVisible[3]);
	}

	if (Strings::EqualFold(identifier, "augslot5visible")) {
		stat = static_cast<int>(item->AugSlotVisible[4]);
	}

	if (Strings::EqualFold(identifier, "augslot6visible")) {
		stat = static_cast<int>(item->AugSlotVisible[5]);
	}

	safe_delete(inst);
	return stat;
}

// Returns a slot's item ID (returns INVALID_ID if not found)
int32 Bot::GetItemIDAt(int16 slot_id) {
	if (slot_id <= EQ::invslot::POSSESSIONS_END && slot_id >= EQ::invslot::POSSESSIONS_BEGIN) {
		if ((((uint64)1 << slot_id) & GetInv().GetLookup()->PossessionsBitmask) == 0) {
			return INVALID_ID;
		}
	}
	else if (slot_id <= EQ::invbag::GENERAL_BAGS_END && slot_id >= EQ::invbag::GENERAL_BAGS_BEGIN) {
		auto temp_slot = EQ::invslot::GENERAL_BEGIN + ((slot_id - EQ::invbag::GENERAL_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT);
		if ((((uint64)1 << temp_slot) & GetInv().GetLookup()->PossessionsBitmask) == 0) {
			return INVALID_ID;
		}
	}
	else if (slot_id <= EQ::invslot::BANK_END && slot_id >= EQ::invslot::BANK_BEGIN) {
		if ((slot_id - EQ::invslot::BANK_BEGIN) >= GetInv().GetLookup()->InventoryTypeSize.Bank) {
			return INVALID_ID;
		}
	}
	else if (slot_id <= EQ::invbag::BANK_BAGS_END && slot_id >= EQ::invbag::BANK_BAGS_BEGIN) {
		auto temp_slot = (slot_id - EQ::invbag::BANK_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT;
		if (temp_slot >= GetInv().GetLookup()->InventoryTypeSize.Bank) {
			return INVALID_ID;
		}
	}

	const EQ::ItemInstance* inst = m_inv[slot_id];
	if (inst)
		return inst->GetItem()->ID;

	// None found
	return INVALID_ID;
}

// Returns an augment's ID that's in an item (returns INVALID_ID if not found)
// Pass in the slot ID of the item and which augslot you want to check (0-5)
int32 Bot::GetAugmentIDAt(int16 slot_id, uint8 augslot) {
	if (slot_id <= EQ::invslot::POSSESSIONS_END && slot_id >= EQ::invslot::POSSESSIONS_BEGIN) {
		if ((((uint64)1 << slot_id) & GetInv().GetLookup()->PossessionsBitmask) == 0) {
			return INVALID_ID;
		}
	}
	else if (slot_id <= EQ::invbag::GENERAL_BAGS_END && slot_id >= EQ::invbag::GENERAL_BAGS_BEGIN) {
		auto temp_slot = EQ::invslot::GENERAL_BEGIN + ((slot_id - EQ::invbag::GENERAL_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT);
		if ((((uint64)1 << temp_slot) & GetInv().GetLookup()->PossessionsBitmask) == 0) {
			return INVALID_ID;
		}
	}
	else if (slot_id <= EQ::invslot::BANK_END && slot_id >= EQ::invslot::BANK_BEGIN) {
		if ((slot_id - EQ::invslot::BANK_BEGIN) >= GetInv().GetLookup()->InventoryTypeSize.Bank) {
			return INVALID_ID;
		}
	}
	else if (slot_id <= EQ::invbag::BANK_BAGS_END && slot_id >= EQ::invbag::BANK_BAGS_BEGIN) {
		auto temp_slot = (slot_id - EQ::invbag::BANK_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT;
		if (temp_slot >= GetInv().GetLookup()->InventoryTypeSize.Bank) {
			return INVALID_ID;
		}
	}

	const EQ::ItemInstance* inst = m_inv[slot_id];
	if (inst && inst->GetAugmentItemID(augslot)) {
		return inst->GetAugmentItemID(augslot);
	}

	// None found
	return INVALID_ID;
}

bool Client::IsAugmentRestricted(uint8 item_type, uint32 augment_restriction)
{
	switch (augment_restriction) {
		case EQ::item::AugRestrictionAny:
			break;
		case EQ::item::AugRestrictionArmor:
			switch (item_type) {
				case EQ::item::ItemTypeArmor:
					break;
				default:
					return true;
			}
			break;
		case EQ::item::AugRestrictionWeapons:
			switch (item_type) {
				case EQ::item::ItemType1HSlash:
				case EQ::item::ItemType1HBlunt:
				case EQ::item::ItemType1HPiercing:
				case EQ::item::ItemTypeMartial:
				case EQ::item::ItemType2HSlash:
				case EQ::item::ItemType2HBlunt:
				case EQ::item::ItemType2HPiercing:
				case EQ::item::ItemTypeBow:
					break;
				default:
					return true;
			}
			break;
		case EQ::item::AugRestriction1HWeapons:
			switch (item_type) {
				case EQ::item::ItemType1HSlash:
				case EQ::item::ItemType1HBlunt:
				case EQ::item::ItemType1HPiercing:
				case EQ::item::ItemTypeMartial:
					break;
				default:
					return true;
			}
			break;
		case EQ::item::AugRestriction2HWeapons:
			switch (item_type) {
				case EQ::item::ItemType2HSlash:
				case EQ::item::ItemType2HBlunt:
				case EQ::item::ItemType2HPiercing:
				case EQ::item::ItemTypeBow:
					break;
				default:
					return true;
			}
			break;
		case EQ::item::AugRestriction1HSlash:
			switch (item_type) {
				case EQ::item::ItemType1HSlash:
					break;
				default:
					return true;
			}
			break;
		case EQ::item::AugRestriction1HBlunt:
			switch (item_type) {
				case EQ::item::ItemType1HBlunt:
					break;
				default:
					return true;
			}
			break;
		case EQ::item::AugRestrictionPiercing:
			switch (item_type) {
				case EQ::item::ItemType1HPiercing:
					break;
				default:
					return true;
			}
			break;
		case EQ::item::AugRestrictionHandToHand:
			switch (item_type) {
				case EQ::item::ItemTypeMartial:
					break;
				default:
					return true;
			}
			break;
		case EQ::item::AugRestriction2HSlash:
			switch (item_type) {
				case EQ::item::ItemType2HSlash:
					break;
				default:
					return true;
			}
			break;
		case EQ::item::AugRestriction2HBlunt:
			switch (item_type) {
				case EQ::item::ItemType2HBlunt:
					break;
				default:
					return true;
			}
			break;
		case EQ::item::AugRestriction2HPierce:
			switch (item_type) {
				case EQ::item::ItemType2HPiercing:
					break;
				default:
					return true;
			}
			break;
		case EQ::item::AugRestrictionBows:
			switch (item_type) {
				case EQ::item::ItemTypeBow:
					break;
				default:
					return true;
			}
			break;
		case EQ::item::AugRestrictionShields:
			switch (item_type) {
				case EQ::item::ItemTypeShield:
					break;
				default:
					return true;
			}
			break;
		case EQ::item::AugRestriction1HSlash1HBluntOrHandToHand:
			switch (item_type) {
				case EQ::item::ItemType1HSlash:
				case EQ::item::ItemType1HBlunt:
				case EQ::item::ItemTypeMartial:
					break;
				default:
					return true;
			}
			break;
		case EQ::item::AugRestriction1HBluntOrHandToHand:
			switch (item_type) {
				case EQ::item::ItemType1HBlunt:
				case EQ::item::ItemTypeMartial:
					break;
				default:
					return true;
			}
			break;
		case EQ::item::AugRestrictionUnknown1:
		case EQ::item::AugRestrictionUnknown2:
		case EQ::item::AugRestrictionUnknown3:
		default:
			return true;
	}

	return false;
}

void Client::SummonItemIntoInventory(
	uint32 item_id,
	int16 charges,
	uint32 aug1,
	uint32 aug2,
	uint32 aug3,
	uint32 aug4,
	uint32 aug5,
	uint32 aug6,
	bool is_attuned
)
{
	auto *inst = database.CreateItem(
		item_id,
		charges,
		aug1,
		aug2,
		aug3,
		aug4,
		aug5,
		aug6,
		is_attuned
	);

	if (!inst) {
		return;
	}

	// Try stacking first if the item is stackable, then fall back to finding a free slot
	if (!PutItemInInventoryWithStacking(inst)) {
		// PutItemInInventoryWithStacking failed, fall back to original behavior
		const bool  is_arrow = inst->GetItem()->ItemType == EQ::item::ItemTypeArrow;
		const int16 slot_id  = m_inv.FindFreeSlot(
			inst->IsClassBag(),
			true,
			inst->GetItem()->Size,
			is_arrow
		);

		SummonItem(
			item_id,
			charges,
			aug1,
			aug2,
			aug3,
			aug4,
			aug5,
			aug6,
			is_attuned,
			slot_id
		);
	}

	safe_delete(inst);
}

bool Client::HasItemOnCorpse(uint32 item_id)
{
	auto corpses = CharacterCorpsesRepository::GetWhere(database, fmt::format("charid = {}", CharacterID()));
	if (corpses.empty()) {
		return false;
	}

	std::vector<uint32> corpse_ids;
	corpse_ids.reserve(corpses.size());

	for (auto &corpse : corpses) {
		corpse_ids.push_back(corpse.id);
	}

	auto items = CharacterCorpseItemsRepository::GetWhere(
		database,
		fmt::format(
			"corpse_id IN ({})",
			Strings::Join(corpse_ids, ",")
		)
	);

	for (auto &item : items) {
		if (item.item_id == item_id) {
			return true;
		}
		if (item.aug_1 == item_id || item.aug_2 == item_id ||
			item.aug_3 == item_id || item.aug_4 == item_id ||
			item.aug_5 == item_id || item.aug_6 == item_id) {
			return true;
		}
	}

	return false;
}

bool Client::PutItemInInventoryWithStacking(EQ::ItemInstance *inst)
{
	auto free_id = GetInv().FindFirstFreeSlotThatFitsItem(inst->GetItem());
	if (inst->IsStackable()) {
		if (TryStacking(inst, ItemPacketTrade, true, false)) {
			return true;
		}
	}
	if (free_id != INVALID_INDEX) {
		if (PutItemInInventory(free_id, *inst, true)) {
			return true;
		}
	}
	return false;
};

bool Client::FindNumberOfFreeInventorySlotsWithSizeCheck(std::vector<BuyerLineTradeItems_Struct> items)
{
	uint32 count = 0;
	for (int16         i = EQ::invslot::GENERAL_BEGIN; i <= EQ::invslot::GENERAL_END; i++) {
		if ((((uint64) 1 << i) & GetInv().GetLookup()->PossessionsBitmask) == 0) {
			continue;
		}

		EQ::ItemInstance *inv_item = GetInv().GetItem(i);

		if (!inv_item) {
			// Found available slot in personal inventory.  Fits all sizes
			count++;
		}

		if (count >= items.size()) {
			return true;
		}

		if (inv_item->IsClassBag()) {
			for (auto const& item:items) {
				auto item_tmp = database.GetItem(item.item_id);
				if (EQ::InventoryProfile::CanItemFitInContainer(item_tmp, inv_item->GetItem())) {
					int16 base_slot_id = EQ::InventoryProfile::CalcSlotId(i, EQ::invbag::SLOT_BEGIN);
					uint8 bag_size     = inv_item->GetItem()->BagSlots;

					for (uint8 bag_slot = EQ::invbag::SLOT_BEGIN; bag_slot < bag_size; bag_slot++) {
						auto bag_item = GetInv().GetItem(base_slot_id + bag_slot);
						if (!bag_item) {
							// Found a bag slot that fits the item
							count++;
						}
					}

					if (count >= items.size()) {
						return true;
					}
				}
			}
		}
	}
	return false;
};
