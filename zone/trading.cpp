/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "client.h"

#include "common/bazaar.h"
#include "common/data_bucket.h"
#include "common/eq_packet_structs.h"
#include "common/eqemu_logsys.h"
#include "common/events/player_event_logs.h"
#include "common/misc_functions.h"
#include "common/repositories/buyer_buy_lines_repository.h"
#include "common/repositories/buyer_repository.h"
#include "common/repositories/trader_repository.h"
#include "common/rulesys.h"
#include "common/strings.h"
#include "zone/entity.h"
#include "zone/mob.h"
#include "zone/quest_parser_collection.h"
#include "zone/string_ids.h"
#include "zone/worldserver.h"
#include <numeric>
#include <map>

class QueryServ;

extern WorldServer worldserver;
extern QueryServ* QServ;

// The maximum amount of a single bazaar/barter transaction expressed in copper.
// Equivalent to 2 Million plat
constexpr auto MAX_TRANSACTION_VALUE = 2000000000;
// ##########################################
// Trade implementation
// ##########################################

Trade::Trade(Mob* in_owner)
{
	owner = in_owner;
	Reset();
}

Trade::~Trade()
{
	Reset();
}

void Trade::Reset()
{
	state = TradeNone;
	with_id = 0;
	pp=0; gp=0; sp=0; cp=0;
}

// Initiate a trade with another mob
// initiate_with specifies whether to start trade with other mob as well
void Trade::Start(uint32 mob_id, bool initiate_with)
{
	Reset();
	state = Trading;
	with_id = mob_id;

	// Autostart on other mob?
	if (initiate_with) {
		Mob *with = With();
		if (with) {
			with->trade->Start(owner->GetID(), false);
		}
	}
}

// Add item from a given slot to trade bucket (automatically does bag data too)
void Trade::AddEntity(uint16 trade_slot_id, uint32 stack_size) {
	// TODO: review for inventory saves / consider changing return type to bool so failure can be passed to desync handler

	if (!owner || !owner->IsClient()) {
		// This should never happen
		LogDebug("Programming error: NPC's should not call Trade::AddEntity()");
		return;
	}

	// If one party accepted the trade then an item was added, their state needs to be reset
	owner->trade->state = Trading;
	Mob* with = With();
	if (with)
		with->trade->state = Trading;

	// Item always goes into trade bucket from cursor
	Client* client = owner->CastToClient();
	EQ::ItemInstance* inst = client->GetInv().GetItem(EQ::invslot::slotCursor);

	if (!inst) {
		client->Message(Chat::Red, "Error: Could not find item on your cursor!");
		return;
	}

	EQ::ItemInstance* inst2 = client->GetInv().GetItem(trade_slot_id);

	// it looks like the original code attempted to allow stacking...
	// (it just didn't handle partial stack move actions)
	if (stack_size > 0) {
		if (!inst->IsStackable() || !inst2 || !inst2->GetItem() || (inst->GetID() != inst2->GetID()) || (stack_size > inst->GetCharges())) {
			client->Kick("Error stacking item in trade");
			return;
		}

		uint32 _stack_size = 0;

		if ((stack_size + inst2->GetCharges()) > inst2->GetItem()->StackSize) {
			_stack_size = (stack_size + inst2->GetCharges()) - inst->GetItem()->StackSize;
			inst2->SetCharges(inst2->GetItem()->StackSize);
		}
		else {
			_stack_size = inst->GetCharges() - stack_size;
			inst2->SetCharges(stack_size + inst2->GetCharges());
		}

		LogTrading("[{}] added partial item [{}] stack (qty: [{}]) to trade slot [{}]", owner->GetName(), inst->GetItem()->Name, stack_size, trade_slot_id);

		if (_stack_size > 0)
			inst->SetCharges(_stack_size);
		else
			client->DeleteItemInInventory(EQ::invslot::slotCursor);

		SendItemData(inst2, trade_slot_id);
	}
	else {
		if (inst2 && inst2->GetID()) {
			client->Kick("Attempting to add null item to trade");
			return;
		}

		SendItemData(inst, trade_slot_id);

		LogTrading("[{}] added item [{}] to trade slot [{}]", owner->GetName(), inst->GetItem()->Name, trade_slot_id);

		client->PutItemInInventory(trade_slot_id, *inst);
		client->DeleteItemInInventory(EQ::invslot::slotCursor);
	}
}

// Retrieve mob the owner is trading with
// Done like this in case 'with' mob goes LD and Mob* becomes invalid
Mob* Trade::With()
{
	return entity_list.GetMob(with_id);
}

// Private Method: Send item data for trade item to other person involved in trade
void Trade::SendItemData(const EQ::ItemInstance* inst, int16 dest_slot_id)
{
	if (inst == nullptr)
		return;

	// @merth: This needs to be redone with new item classes
	Mob* mob = With();
	if (!mob->IsClient())
		return; // Not sending packets to NPCs!

	Client* with = mob->CastToClient();
	Client* trader = owner->CastToClient();
	if (with && with->IsClient()) {
		with->SendItemPacket(dest_slot_id - EQ::invslot::TRADE_BEGIN, inst, ItemPacketTradeView);
		if (inst->GetItem()->ItemClass == 1) {
			for (uint16 i = EQ::invbag::SLOT_BEGIN; i <= EQ::invbag::SLOT_END; i++) {
				uint16 bagslot_id = EQ::InventoryProfile::CalcSlotId(dest_slot_id, i);
				const EQ::ItemInstance* bagitem = trader->GetInv().GetItem(bagslot_id);
				if (bagitem) {
					with->SendItemPacket(bagslot_id - EQ::invslot::TRADE_BEGIN, bagitem, ItemPacketTradeView);
				}
			}
		}

		//safe_delete(outapp);
	}
}

Mob *Trade::GetOwner() const
{
	return owner;
}


void Client::ResetTrade() {
	AddMoneyToPP(trade->cp, trade->sp, trade->gp, trade->pp, true);

	// step 1: process bags
	for (int16 trade_slot = EQ::invslot::TRADE_BEGIN; trade_slot <= EQ::invslot::TRADE_END; ++trade_slot) {
		const EQ::ItemInstance* inst = m_inv[trade_slot];

		if (inst && inst->IsClassBag()) {
			int16 free_slot = m_inv.FindFreeSlotForTradeItem(inst);

			if (free_slot != INVALID_INDEX) {
				PutItemInInventory(free_slot, *inst);
				SendItemPacket(free_slot, inst, ItemPacketTrade);
			}
			else {
				DropInst(inst);
			}

			DeleteItemInInventory(trade_slot);
		}
	}

	// step 2a: process stackables
	for (int16 trade_slot = EQ::invslot::TRADE_BEGIN; trade_slot <= EQ::invslot::TRADE_END; ++trade_slot) {
		EQ::ItemInstance* inst = GetInv().GetItem(trade_slot);

		if (inst && inst->IsStackable()) {
			while (true) {
				// there's no built-in safety check against an infinite loop..but, it should break on one of the conditional checks
				int16 free_slot = m_inv.FindFreeSlotForTradeItem(inst);

				if ((free_slot == EQ::invslot::slotCursor) || (free_slot == INVALID_INDEX))
					break;

				EQ::ItemInstance* partial_inst = GetInv().GetItem(free_slot);

				if (!partial_inst)
					break;

				if (partial_inst->GetID() != inst->GetID()) {
					LogDebug("[CLIENT] Client::ResetTrade() - an incompatible location reference was returned by Inventory::FindFreeSlotForTradeItem()");

					break;
				}

				if ((partial_inst->GetCharges() + inst->GetCharges()) > partial_inst->GetItem()->StackSize) {
					int16 new_charges = (partial_inst->GetCharges() + inst->GetCharges()) - partial_inst->GetItem()->StackSize;

					partial_inst->SetCharges(partial_inst->GetItem()->StackSize);
					inst->SetCharges(new_charges);
				}
				else {
					partial_inst->SetCharges(partial_inst->GetCharges() + inst->GetCharges());
					inst->SetCharges(0);
				}

				PutItemInInventory(free_slot, *partial_inst);
				SendItemPacket(free_slot, partial_inst, ItemPacketTrade);

				if (inst->GetCharges() == 0) {
					DeleteItemInInventory(trade_slot);

					break;
				}
			}
		}
	}

	// step 2b: adjust trade stack bias
	// (if any partial stacks exist before the final stack, FindFreeSlotForTradeItem() will return that slot in step 3 and an overwrite will occur)
	for (int16 trade_slot = EQ::invslot::TRADE_END; trade_slot >= EQ::invslot::TRADE_BEGIN; --trade_slot) {
		EQ::ItemInstance* inst = GetInv().GetItem(trade_slot);

		if (inst && inst->IsStackable()) {
			for (int16 bias_slot = EQ::invslot::TRADE_BEGIN; bias_slot <= EQ::invslot::TRADE_END; ++bias_slot) {
				if (bias_slot >= trade_slot)
					break;

				EQ::ItemInstance* bias_inst = GetInv().GetItem(bias_slot);

				if (!bias_inst || (bias_inst->GetID() != inst->GetID()) || (bias_inst->GetCharges() >= bias_inst->GetItem()->StackSize))
					continue;

				if ((bias_inst->GetCharges() + inst->GetCharges()) > bias_inst->GetItem()->StackSize) {
					int16 new_charges = (bias_inst->GetCharges() + inst->GetCharges()) - bias_inst->GetItem()->StackSize;

					bias_inst->SetCharges(bias_inst->GetItem()->StackSize);
					inst->SetCharges(new_charges);
				}
				else {
					bias_inst->SetCharges(bias_inst->GetCharges() + inst->GetCharges());
					inst->SetCharges(0);
				}

				if (inst->GetCharges() == 0) {
					DeleteItemInInventory(trade_slot);

					break;
				}
			}
		}
	}

	// step 3: process everything else
	for (int16 trade_slot = EQ::invslot::TRADE_BEGIN; trade_slot <= EQ::invslot::TRADE_END; ++trade_slot) {
		const EQ::ItemInstance* inst = m_inv[trade_slot];

		if (inst) {
			int16 free_slot = m_inv.FindFreeSlotForTradeItem(inst);

			if (free_slot != INVALID_INDEX) {
				PutItemInInventory(free_slot, *inst);
				SendItemPacket(free_slot, inst, ItemPacketTrade);
			}
			else {
				DropInst(inst);
			}

			DeleteItemInInventory(trade_slot);
		}
	}
}

void Client::FinishTrade(Mob* tradingWith, bool finalizer, void* event_entry, std::list<void*>* event_details) {
	if (!tradingWith) {
		return;
	}

	if (tradingWith->IsClient()) {
		Client                * other    = tradingWith->CastToClient();

		if(other) {
			LogTrading("Finishing trade with client [{}]", other->GetName());

			AddMoneyToPP(other->trade->cp, other->trade->sp, other->trade->gp, other->trade->pp, true);

			// step 1: process bags
			for (int16 trade_slot = EQ::invslot::TRADE_BEGIN; trade_slot <= EQ::invslot::TRADE_END; ++trade_slot) {
				const EQ::ItemInstance* inst = m_inv[trade_slot];

				if (inst && inst->IsClassBag()) {
					LogTrading("Giving container [{}] ([{}]) in slot [{}] to [{}]", inst->GetItem()->Name, inst->GetItem()->ID, trade_slot, other->GetName());

					// TODO: need to check bag items/augments for no drop..everything for attuned...
					if (
						inst->GetItem()->NoDrop != 0 ||
						CanTradeFVNoDropItem() ||
						other == this
						) {
						int16 free_slot = other->GetInv().FindFreeSlotForTradeItem(inst);

						if (free_slot != INVALID_INDEX) {
							if (other->PutItemInInventory(free_slot, *inst, true)) {
								inst->TransferOwnership(database, other->CharacterID());
								LogTrading("Container [{}] ([{}]) successfully transferred, deleting from trade slot", inst->GetItem()->Name, inst->GetItem()->ID);
							}
							else {
								LogTrading("Transfer of container [{}] ([{}]) to [{}] failed, returning to giver", inst->GetItem()->Name, inst->GetItem()->ID, other->GetName());
								PushItemOnCursor(*inst, true);
							}
						}
						else {
							LogTrading("[{}]'s inventory is full, returning container [{}] ([{}]) to giver", other->GetName(), inst->GetItem()->Name, inst->GetItem()->ID);
							PushItemOnCursor(*inst, true);
						}
					}
					else {
						LogTrading("Container [{}] ([{}]) is NoDrop, returning to giver", inst->GetItem()->Name, inst->GetItem()->ID);
						PushItemOnCursor(*inst, true);
					}

					DeleteItemInInventory(trade_slot);
				}
			}

			// step 2a: process stackables
			for (int16 trade_slot = EQ::invslot::TRADE_BEGIN; trade_slot <= EQ::invslot::TRADE_END; ++trade_slot) {
				EQ::ItemInstance* inst = GetInv().GetItem(trade_slot);

				if (inst && inst->IsStackable()) {
					while (true) {
						// there's no built-in safety check against an infinite loop..but, it should break on one of the conditional checks
						int16 partial_slot = other->GetInv().FindFreeSlotForTradeItem(inst);

						if ((partial_slot == EQ::invslot::slotCursor) || (partial_slot == INVALID_INDEX))
							break;

						EQ::ItemInstance* partial_inst = other->GetInv().GetItem(partial_slot);

						if (!partial_inst)
							break;

						if (partial_inst->GetID() != inst->GetID()) {
							LogTrading("[CLIENT] Client::ResetTrade() - an incompatible location reference was returned by Inventory::FindFreeSlotForTradeItem()");
							break;
						}

						int16 old_charges = inst->GetCharges();
						int16 partial_charges = partial_inst->GetCharges();

						if ((partial_inst->GetCharges() + inst->GetCharges()) > partial_inst->GetItem()->StackSize) {
							int16 new_charges = (partial_inst->GetCharges() + inst->GetCharges()) - partial_inst->GetItem()->StackSize;

							partial_inst->SetCharges(partial_inst->GetItem()->StackSize);
							inst->SetCharges(new_charges);
						}
						else {
							partial_inst->SetCharges(partial_inst->GetCharges() + inst->GetCharges());
							inst->SetCharges(0);
						}

						LogTrading("Transferring partial stack [{}] ([{}]) in slot [{}] to [{}]", inst->GetItem()->Name, inst->GetItem()->ID, trade_slot, other->GetName());

						if (other->PutItemInInventory(partial_slot, *partial_inst, true)) {
							LogTrading(
								"Partial stack [{}] ([{}]) successfully transferred, deleting [{}] charges from trade slot",
								inst->GetItem()->Name,
								inst->GetItem()->ID,
								(old_charges - inst->GetCharges())
							);
							inst->TransferOwnership(database, other->CharacterID());
						}
						else {
							LogTrading("Transfer of partial stack [{}] ([{}]) to [{}] failed, returning [{}] charges to trade slot",
									   inst->GetItem()->Name, inst->GetItem()->ID, other->GetName(), (old_charges - inst->GetCharges()));

							inst->SetCharges(old_charges);
							partial_inst->SetCharges(partial_charges);
							break;
						}

						if (inst->GetCharges() == 0) {
							DeleteItemInInventory(trade_slot);
							break;
						}
					}
				}
			}

			// step 2b: adjust trade stack bias
			// (if any partial stacks exist before the final stack, FindFreeSlotForTradeItem() will return that slot in step 3 and an overwrite will occur)
			for (int16 trade_slot = EQ::invslot::TRADE_END; trade_slot >= EQ::invslot::TRADE_BEGIN; --trade_slot) {
				EQ::ItemInstance* inst = GetInv().GetItem(trade_slot);

				if (inst && inst->IsStackable()) {
					for (int16 bias_slot = EQ::invslot::TRADE_BEGIN; bias_slot <= EQ::invslot::TRADE_END; ++bias_slot) {
						if (bias_slot >= trade_slot)
							break;

						EQ::ItemInstance* bias_inst = GetInv().GetItem(bias_slot);

						if (!bias_inst || (bias_inst->GetID() != inst->GetID()) || (bias_inst->GetCharges() >= bias_inst->GetItem()->StackSize))
							continue;

						int16 old_charges = inst->GetCharges();

						if ((bias_inst->GetCharges() + inst->GetCharges()) > bias_inst->GetItem()->StackSize) {
							int16 new_charges = (bias_inst->GetCharges() + inst->GetCharges()) - bias_inst->GetItem()->StackSize;

							bias_inst->SetCharges(bias_inst->GetItem()->StackSize);
							inst->SetCharges(new_charges);
						}
						else {
							bias_inst->SetCharges(bias_inst->GetCharges() + inst->GetCharges());
							inst->SetCharges(0);
						}

						if (inst->GetCharges() == 0) {
							DeleteItemInInventory(trade_slot);
							break;
						}
					}
				}
			}

			// step 3: process everything else
			for (int16 trade_slot = EQ::invslot::TRADE_BEGIN; trade_slot <= EQ::invslot::TRADE_END; ++trade_slot) {
				const EQ::ItemInstance* inst = m_inv[trade_slot];

				if (inst) {
					LogTrading("Giving item [{}] ([{}]) in slot [{}] to [{}]", inst->GetItem()->Name, inst->GetItem()->ID, trade_slot, other->GetName());

					// TODO: need to check bag items/augments for no drop..everything for attuned...
					if (inst->GetItem()->NoDrop != 0 || CanTradeFVNoDropItem() || other == this) {
						int16 free_slot = other->GetInv().FindFreeSlotForTradeItem(inst);

						if (free_slot != INVALID_INDEX) {
							if (other->PutItemInInventory(free_slot, *inst, true)) {
								inst->TransferOwnership(database, other->CharacterID());
								LogTrading("Item [{}] ([{}]) successfully transferred, deleting from trade slot", inst->GetItem()->Name, inst->GetItem()->ID);
							}
							else {
								LogTrading("Transfer of Item [{}] ([{}]) to [{}] failed, returning to giver", inst->GetItem()->Name, inst->GetItem()->ID, other->GetName());
								PushItemOnCursor(*inst, true);
							}
						}
						else {
							LogTrading("[{}]'s inventory is full, returning item [{}] ([{}]) to giver", other->GetName(), inst->GetItem()->Name, inst->GetItem()->ID);
							PushItemOnCursor(*inst, true);
						}
					}
					else {
						LogTrading("Item [{}] ([{}]) is NoDrop, returning to giver", inst->GetItem()->Name, inst->GetItem()->ID);
						PushItemOnCursor(*inst, true);
					}

					DeleteItemInInventory(trade_slot);
				}
			}

			//Do not reset the trade here, done by the caller.
		}
	}
	else if(tradingWith->IsNPC()) {

		bool quest_npc = false;
		if (parse->HasQuestSub(tradingWith->GetNPCTypeID(), EVENT_TRADE)) {
			quest_npc = true;
		}

		// take ownership of all trade slot items
		EQ::ItemInstance* insts[4] = { 0 };
		for (int i = EQ::invslot::TRADE_BEGIN; i <= EQ::invslot::TRADE_NPC_END; ++i) {
			insts[i - EQ::invslot::TRADE_BEGIN] = m_inv.PopItem(i);
			database.SaveInventory(CharacterID(), nullptr, i);
		}

		// copy to be filtered by task updates, null trade slots preserved for quest event arg
		std::vector<EQ::ItemInstance*> items(insts, insts + std::size(insts));

		if (RuleB(TaskSystem, EnableTaskSystem)) {
			if (UpdateTasksOnDeliver(items, *trade, tradingWith->CastToNPC())) {
				if (!tradingWith->IsMoving()) {
					tradingWith->FaceTarget(this);
				}
			}
		}

		if (!quest_npc) {
			for (auto &inst: items) {
				if (!inst || !inst->GetItem()) {
					continue;
				}

				// remove delivered task items
				if (RuleB(TaskSystem, EnableTaskSystem) && inst->GetTaskDeliveredCount() > 0) {
					int remaining = inst->RemoveTaskDeliveredItems();
					if (remaining <= 0) {
						inst = nullptr;
						continue; // all items in trade slot consumed by task update
					}
				}

				auto               with   = tradingWith->CastToNPC();
				const EQ::ItemData *item  = inst->GetItem();
				const bool         is_pet = with->IsPetOwnerOfClientBot() || with->IsCharmedPet();
				if (is_pet && with->CanPetTakeItem(inst)) {
					// pets need to look inside bags and try to equip items found there
					if (item->IsClassBag() && item->BagSlots > 0) {
						// if an item inside the bag can't be given to the pet, keep the bag
						bool       keep_bag   = false;
						int        item_count = 0;
						for (int16 bslot      = EQ::invbag::SLOT_BEGIN; bslot < item->BagSlots; bslot++) {
							const EQ::ItemInstance *baginst = inst->GetItem(bslot);
							if (baginst && baginst->GetItem() && with->CanPetTakeItem(baginst)) {
								// add item to pet's inventory
								auto lde = LootdropEntriesRepository::NewNpcEntity();
								lde.equip_item   = 1;
								lde.item_charges = static_cast<int8>(baginst->GetCharges());
								with->AddLootDrop(baginst->GetItem(), lde, true);
								inst->DeleteItem(bslot);
								item_count++;
							}
							else {
								keep_bag = true;
							}
						}

						// add item to pet's inventory
						if (!keep_bag || item_count == 0) {
							auto lde = LootdropEntriesRepository::NewNpcEntity();
							lde.equip_item   = 1;
							lde.item_charges = static_cast<int8>(inst->GetCharges());
							with->AddLootDrop(item, lde, true);
							inst = nullptr;
						}
					}
					else {
						// add item to pet's inventory
						auto lde = LootdropEntriesRepository::NewNpcEntity();
						lde.equip_item   = 1;
						lde.item_charges = static_cast<int8>(inst->GetCharges());
						with->AddLootDrop(item, lde, true);
						inst = nullptr;
					}
				}
			}
		}

		std::string currencies[] = {"copper", "silver", "gold", "platinum"};
		int32       amounts[]    = {trade->cp, trade->sp, trade->gp, trade->pp};

		for (int i = 0; i < 4; ++i) {
			parse->AddVar(
				fmt::format("{}.{}", currencies[i], tradingWith->GetNPCTypeID()),
				fmt::format("{}", amounts[i])
			);
		}

		if (tradingWith->GetAppearance() != eaDead) {
			tradingWith->FaceTarget(this);
		}

		// we cast to any to pass through the quest event system
		std::vector<std::any> item_list(items.begin(), items.end());
		for (EQ::ItemInstance *inst: items) {
			if (!inst || !inst->GetItem()) {
				continue;
			}
			item_list.emplace_back(inst);
		}

		auto handin_npc = tradingWith->CastToNPC();

		m_external_handin_money_returned = {};
		m_external_handin_items_returned = {};
		bool has_aggro = tradingWith->CheckAggro(this);
		if (parse->HasQuestSub(tradingWith->GetNPCTypeID(), EVENT_TRADE) && !has_aggro) {
			// This CheckHandin call enables eq.handin and quest::handin to recognize the hand-in context.
			// It initializes the first hand-in bucket, which is then reused for the EVENT_TRADE subroutine.
			std::map<std::string, uint32> handin = {
				{"copper",   trade->cp},
				{"silver",   trade->sp},
				{"gold",     trade->gp},
				{"platinum", trade->pp}
			};

			for (EQ::ItemInstance *inst: items) {
				if (!inst || !inst->GetItem()) {
					continue;
				}

				std::string item_id = fmt::format("{}", inst->GetItem()->ID);
				handin[item_id] += inst->GetCharges();
			}

			handin_npc->CheckHandin(this, handin, {}, items);

			parse->EventNPC(EVENT_TRADE, tradingWith->CastToNPC(), this, "", 0, &item_list);
			LogNpcHandinDetail("EVENT_TRADE triggered for NPC [{}]", tradingWith->GetNPCTypeID());
		}

		// this is a catch-all return for items that weren't consumed by the EVENT_TRADE subroutine
		// it's possible we have a quest NPC that doesn't have an EVENT_TRADE subroutine
		// we can't double fire the ReturnHandinItems() event, so we need to check if it's already been processed from EVENT_TRADE
		if (!handin_npc->HasProcessedHandinReturn()) {
			if (!handin_npc->HandinStarted()) {
				LogNpcHandinDetail("EVENT_TRADE did not process handin, calling ReturnHandinItems() for NPC [{}]", tradingWith->GetNPCTypeID());
				std::map<std::string, uint32> handin = {
					{"copper",   trade->cp},
					{"silver",   trade->sp},
					{"gold",     trade->gp},
					{"platinum", trade->pp}
				};

				for (EQ::ItemInstance *inst: items) {
					if (!inst || !inst->GetItem()) {
						continue;
					}

					std::string item_id = fmt::format("{}", inst->GetItem()->ID);
					handin[item_id] += inst->GetCharges();
				}

				handin_npc->CheckHandin(this, handin, {}, items);
			}

			if (RuleB(Items, AlwaysReturnHandins)) {
				handin_npc->ReturnHandinItems(this);
				LogNpcHandin("ReturnHandinItems called for NPC [{}]", handin_npc->GetNPCTypeID());
			}
		}

		handin_npc->ResetHandin();

		for (auto &inst: insts) {
			if (inst) {
				safe_delete(inst);
			}
		}
	}
}

bool Client::CheckTradeLoreConflict(Client* other)
{
	if (!other) {
		return true;
	}

	bool has_lore_item = false;
	std::vector<uint32> lore_item_ids;

	for (int16 index = EQ::invslot::TRADE_BEGIN; index <= EQ::invslot::TRADE_END; ++index) {
		const auto inst = m_inv[index];
		if (!inst || !inst->GetItem()) {
			continue;
		}

		if (other->CheckLoreConflict(inst->GetItem())) {
			lore_item_ids.emplace_back(inst->GetItem()->ID);

			has_lore_item = true;
		}
	}

	for (int16 index = EQ::invbag::TRADE_BAGS_BEGIN; index <= EQ::invbag::TRADE_BAGS_END; ++index) {
		const auto inst = m_inv[index];
		if (!inst || !inst->GetItem()) {
			continue;
		}

		if (other->CheckLoreConflict(inst->GetItem())) {
			lore_item_ids.emplace_back(inst->GetItem()->ID);

			has_lore_item = true;
		}
	}

	if (has_lore_item && RuleB(Character, PlayerTradingLoreFeedback)) {
		for (const uint32 lore_item_id : lore_item_ids) {
			Message(
				Chat::Red,
				fmt::format(
					"{} already has a lore {} in their inventory.",
					other->GetCleanName(),
					database.CreateItemLink(lore_item_id)
				).c_str()
			);
		}
	}

	return has_lore_item;
}

bool Client::CheckTradeNonDroppable()
{
	for (int16 index = EQ::invslot::TRADE_BEGIN; index <= EQ::invslot::TRADE_END; ++index){
		const EQ::ItemInstance* inst = m_inv[index];
		if (!inst)
			continue;

		if (!inst->IsDroppable())
			return true;
	}

	return false;
}

void Client::TraderShowItems()
{
	auto outapp = new EQApplicationPacket(OP_Trader, sizeof(Trader_Struct));
	auto data   = (Trader_Struct *) outapp->pBuffer;

	auto   trader_items = TraderRepository::GetWhere(database, fmt::format("`char_id` = '{}'", CharacterID()));
	uint32 item_limit   = trader_items.size() >= GetInv().GetLookup()->InventoryTypeSize.Bazaar ?
		GetInv().GetLookup()->InventoryTypeSize.Bazaar :
		trader_items.size();

	for (int i = 0; i < item_limit; i++) {
		data->item_cost[i] = trader_items.at(i).item_cost;
		data->items[i]     = ClientVersion() == EQ::versions::ClientVersion::RoF2 ? trader_items.at(i).item_sn
			: trader_items.at(i).item_id;
	}

	data->action = ListTraderItems;

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendTraderPacket(Client* Trader, uint32 Unknown72)
{
	if(!Trader)
		return;

	auto outapp = new EQApplicationPacket(OP_BecomeTrader, sizeof(BecomeTrader_Struct));

	BecomeTrader_Struct* bts = (BecomeTrader_Struct*)outapp->pBuffer;

	bts->action = BazaarTrader_StartTraderMode;

	bts->trader_id = Trader->CharacterID();
	bts->entity_id = Trader->GetID();
	strn0cpy(bts->trader_name, Trader->GetName(), sizeof(bts->trader_name));

	QueuePacket(outapp);


	safe_delete(outapp);
}

void Client::Trader_CustomerBrowsing(Client *Customer)
{

	auto outapp = new EQApplicationPacket(OP_Trader, sizeof(Trader_ShowItems_Struct));
	auto sis    = (Trader_ShowItems_Struct *) outapp->pBuffer;

	sis->action    = CustomerBrowsing;
	sis->entity_id = Customer->GetID();

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::TraderStartTrader(const EQApplicationPacket *app)
{
	uint32                                max_items         = GetInv().GetLookup()->InventoryTypeSize.Bazaar;
	auto                                  in                = (ClickTrader_Struct *) app->pBuffer;
	auto                                  inv               = GetTraderItems();
	bool                                  trade_items_valid = true;
	std::vector<TraderRepository::Trader> trader_items{};

	//Check inventory for no-trade items
	for (auto i = 0; i < max_items; i++) {
		if (inv->items[i] == 0 || inv->serial_number[i] == 0) {
			continue;
		}

		auto inst = FindTraderItemBySerialNumber(inv->serial_number[i]);
		if (inst) {
			if (inst->GetItem() && inst->GetItem()->NoDrop == 0) {
				Message(
					Chat::Red,
					fmt::format(
						"Item: {} is NODROP and found in a Trader's Satchel. Please remove and restart trader mode",
						inst->GetItem()->Name
					).c_str()
				);
				TraderEndTrader();
				safe_delete(inv);
				return;
			}
		}
	}

	for (uint32 i = 0; i < max_items; i++) {
		if (inv->serial_number[i] <= 0) {
			continue;
		}

		auto inst = FindTraderItemBySerialNumber(inv->serial_number[i]);
		if (!inst) {
			trade_items_valid = false;
			break;
		}

		auto it   = std::find(std::begin(in->serial_number), std::end(in->serial_number), inv->serial_number[i]);
		if (inst && it != std::end(in->serial_number)) {
			inst->SetPrice(in->item_cost[i]);
			TraderRepository::Trader trader_item{};

			trader_item.id                    = 0;
			trader_item.char_entity_id        = GetID();
			trader_item.char_id               = CharacterID();
			trader_item.char_zone_id          = GetZoneID();
			trader_item.char_zone_instance_id = GetInstanceID();
			trader_item.item_charges          = inst->GetCharges() == 0 ? 1 : inst->GetCharges();
			trader_item.item_cost             = inst->GetPrice();
			trader_item.item_id               = inst->GetID();
			trader_item.item_sn               = in->serial_number[i];
			trader_item.slot_id               = i;
			trader_item.listing_date          = time(nullptr);
			if (inst->IsAugmented()) {
				auto augs              = inst->GetAugmentIDs();
				trader_item.aug_slot_1 = augs.at(0);
				trader_item.aug_slot_2 = augs.at(1);
				trader_item.aug_slot_3 = augs.at(2);
				trader_item.aug_slot_4 = augs.at(3);
				trader_item.aug_slot_5 = augs.at(4);
				trader_item.aug_slot_6 = augs.at(5);
			}

			trader_items.emplace_back(trader_item);
			continue;
		}
		else if (inst) {
			Message(
				Chat::Red,
				fmt::format(
					"Item: {} has no price set. Please set a price and try again.",
					inst->GetItem()->Name
				).c_str()
			);
			trade_items_valid = false;
			continue;
		}
	}

	if (!trade_items_valid) {
		Message(Chat::Red, "You are not able to become a trader at this time.  Invalid item found.");
		TraderEndTrader();
		safe_delete(inv);
		return;
	}

	// AoTv4: do NOT clear the char's existing rows before relisting -- they are escrowed listings from the
	// /shop flow (AddItemsToShop), and the item lives ONLY as its row, so the stock clear-before-relist
	// would destroy real items. This native satchel path is legacy (superseded by AddItemsToShop) and
	// dormant, but leave escrow intact if it ever fires.
	TraderRepository::ReplaceMany(database, trader_items);
	safe_delete(inv);

	// This refreshes the Trader window to display the End Trader button
	if (ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		auto outapp = new EQApplicationPacket(OP_Trader, sizeof(TraderStatus_Struct));
		auto data   = (TraderStatus_Struct *) outapp->pBuffer;
		data->Code = TraderAck2;
		QueuePacket(outapp);
		safe_delete(outapp);
	}

	MessageString(Chat::Yellow, TRADER_MODE_ON);
	SetTrader(true);
	SendTraderMode(TraderOn);
	SendBecomeTraderToWorld(this, TraderOn);
	LogTrading("Trader Mode ON for Player [{}] with client version {}.", GetCleanName(), (uint32) ClientVersion());
}

void Client::TraderEndTrader(bool persist_listings)
{
	if (IsThereACustomer()) {
		auto customer = entity_list.GetClientByID(GetCustomerID());
		if (customer) {
			auto end_session = new EQApplicationPacket(OP_ShopEnd);
			customer->FastQueuePacket(&end_session);
		}
	}

	// AoTv4: NEVER delete escrowed listings when trader mode ends. A listed item lives ONLY as its `trader`
	// row (Client::AddItemsToShop removed it from the satchel), so the stock "delete this char's rows on
	// end-trader" would destroy real items -- and end-trader fires on MANY paths (zoning, camp, inventory
	// moves, invalid-item, ...), not just an intentional close. The shop must persist and stay searchable/
	// buyable. Rows are removed ONLY by an explicit unlist (PullShopItem), a sale, or login reconciliation
	// (ReclaimOfflineShop). So the stock `if (!persist_listings) DeleteWhere(...)` is disabled here.
	//   We deliberately leave char_entity_id AS-IS (the small shop-open entity id): the Bazaar's per-trader
	//   filter does an EXACT match on char_entity_id (common/bazaar.cpp); a browsing buyer with no live
	//   entity is resolved from the DB (Handle_OP_TraderShop). Offline-sale coin is banked to
	//   shop_escrow_<charid> and paid on the seller's next login (global_player.lua).
	(void) persist_listings;

	SendBecomeTraderToWorld(this, TraderOff);
	SendTraderMode(TraderOff);

	WithCustomer(0);
	SetTrader(false);
}

// AoTv4 offline shop: called on login (global_player.lua event_connect). Reconciles the owner's
// Trader's Satchel against what actually sold while they were offline, then closes the listing.
// The offline rows are stamped with kOfflineTraderEntityBase; `shop_listed_<charid>` holds the serials
// that were listed at shop-open. For each listed serial: if no row survives it fully sold (remove the
// item); if a row survives and the item is stackable, remove just the quantity that sold; a surviving
// row for a non-stackable means it's unsold (leave it). Coin from offline sales is paid separately via
// the shop_escrow_<charid> bucket (bazaar_broker.pay_escrow).
// Point this character's surviving trader rows at the CURRENT session.
//
// ⚠️ The rows are permanent escrow -- they outlive a logout on purpose -- but char_zone_id and
// char_entity_id were written when the item was listed and are stale the moment you relog. Two things
// break as a result:
//   * the /bazaar trader list grouped on those columns, so a seller appeared once PER SESSION they had
//     ever listed from (see TraderRepository::GetDistinctTraders);
//   * a purchase routes the payout to the seller's zone using char_zone_id, and a stale zone sends it
//     somewhere they are not.
//
// ⚠️ UPDATE, never DELETE. Client::ReclaimOfflineShop below ends in a blanket DeleteWhere on char_id,
// which predates the escrow rework: with items living ONLY as trader rows, that would destroy them.
// It is not called anywhere, and it should not be wired up without being rewritten first.
void Client::RefreshShopSession()
{
	// TraderRepository has DeleteWhere but no UpdateWhere, so this is a direct query -- the same thing
	// the rest of the trading code does for one-off updates.
	database.QueryDatabase(fmt::format(
		"UPDATE `trader` SET `char_zone_id` = '{}', `char_zone_instance_id` = '{}', "
		"`char_entity_id` = '{}' WHERE `char_id` = '{}'",
		GetZoneID(), GetInstanceID(), GetID(), CharacterID()
	));
}

void Client::ReclaimOfflineShop()
{
	std::string listed_key = fmt::format("shop_listed_{}", CharacterID());
	std::string listed     = DataBucket::GetData(&database, listed_key);

	if (!listed.empty()) {
		// quantity of an item_id represented by a listing/row: a stack counts its charges, anything
		// else counts as 1 unit.
		auto qty_of = [this](uint32 item_id, int32 charges) -> int32 {
			const EQ::ItemData *d = database.GetItem(item_id);
			if (d && d->Stackable) {
				return charges > 0 ? charges : 1;
			}
			return 1;
		};

		// listed at shop-open, vs. what survives now (unsold); the difference sold while offline
		std::map<uint32, int32> listed_qty;
		std::map<uint32, int32> remaining_qty;

		for (const auto &tok : Strings::Split(listed, ",")) {
			if (tok.empty()) {
				continue;
			}
			auto kv = Strings::Split(tok, ":");
			if (kv.size() < 2) {
				continue;
			}
			uint32 iid = Strings::ToUnsignedInt(kv[0]);
			int32  chg = Strings::ToInt(kv[1]);
			listed_qty[iid] += qty_of(iid, chg);
		}

		auto rows = TraderRepository::GetWhere(database, fmt::format("`char_id` = '{}'", CharacterID()));
		for (const auto &r : rows) {
			remaining_qty[r.item_id] += qty_of(r.item_id, r.item_charges);
		}

		for (const auto &kv : listed_qty) {
			uint32 item_id = kv.first;
			int32  sold    = kv.second - remaining_qty[item_id];   // 0 for unsold, >0 for what sold
			if (sold <= 0) {
				continue;
			}
			// remove `sold` units of item_id from the Trader's Satchel (matched by item_id since serials
			// are unstable across relog); stacks are decremented, non-stackables deleted whole
			for (int i = EQ::invslot::GENERAL_BEGIN; i <= EQ::invslot::GENERAL_END && sold > 0; i++) {
				const EQ::ItemInstance *bag = GetInv().GetItem(i);
				if (!bag || bag->GetItem()->BagType != EQ::item::BagTypeTradersSatchel) {
					continue;
				}
				for (int x = EQ::invbag::SLOT_BEGIN; x <= EQ::invbag::SLOT_END && sold > 0; x++) {
					uint16                  sid = EQ::InventoryProfile::CalcSlotId(i, x);
					const EQ::ItemInstance *it  = GetInv().GetItem(sid);
					if (!it || it->GetItem()->ID != item_id) {
						continue;
					}
					if (it->IsStackable()) {
						int32 have = it->GetCharges() > 0 ? it->GetCharges() : 1;
						int32 take = have < sold ? have : sold;
						DeleteItemInInventory(sid, take, true, true);
						sold -= take;
					}
					else {
						DeleteItemInInventory(sid, 0, true, true);   // 0 -> delete the whole item
						sold -= 1;
					}
					LogTrading("AoTv4 offline shop: removed sold item_id [{}] from [{}]'s satchel on login",
							   item_id, GetName());
				}
			}
		}

		DataBucket::SetData(&database, listed_key, "");    // reconciled -> clear the record
	}

	// close the offline listing; the owner re-opens to sell again (unsold goods remain in the satchel).
	// On login the player is not trading yet, so any surviving rows for them are leftover offline
	// listings -- safe to clear wholesale.
	TraderRepository::DeleteWhere(database, fmt::format("`char_id` = '{}'", CharacterID()));
}

// ===== AoTv4 permanent escrow shop ==========================================================
// A player's shop is a set of `trader` rows that are ESCROWED: when an item is added it is removed from
// the player's bags and lives only as a row, so it can never be stashed/duped. Rows are searchable via
// the native Bazaar (offline-trader style) and bought via the DB/parcel path; a sale credits the seller
// (immediately if online, else shop_escrow_<charid> paid on next login). The player manages the shop
// through the dll window's "My Shop" tab (list + pull). No Trader's Satchel involved.

// Sellable (droppable, non-container) items across the player's main inventory + its bags, as a CSV of
// "slot|item_id|name|vendor_value" -- feeds the dll "Add Items" browser. Skips bank/equipped/cursor.
// AoTv4 price book: item_id -> price (copper), persisted per character in the shopbook_<charid> bucket.
static std::map<uint32, uint32> LoadShopBook(uint32 char_id)
{
	std::map<uint32, uint32> book;
	for (const auto &tok : Strings::Split(DataBucket::GetData(&database, fmt::format("shopbook_{}", char_id)), ",")) {
		auto kv = Strings::Split(tok, ":");
		if (kv.size() >= 2) { book[Strings::ToUnsignedInt(kv[0])] = Strings::ToUnsignedInt(kv[1]); }
	}
	return book;
}

std::string Client::GetSellableInventory()
{
	std::map<uint32, uint32> book = LoadShopBook(CharacterID());   // projected prices for the Inventory tab
	std::string csv;
	auto        consider = [&](int16 slot) {
		const EQ::ItemInstance *it = GetInv().GetItem(slot);
		if (!it || !it->GetItem()) {
			return;
		}
		const EQ::ItemData *d = it->GetItem();
		if (d->NoDrop == 0 || it->IsClassBag()) {   // No-Drop can't list; don't sell containers
			return;
		}
		std::string name = d->Name;
		for (auto &ch : name) { if (ch == '|' || ch == '^') { ch = ' '; } }
		int qty = d->Stackable ? (it->GetCharges() > 0 ? it->GetCharges() : 1) : 1;   // stack size to sell
		uint32 bp = book.count(d->ID) ? book[d->ID] : 0;                              // price-book price (0 = unpriced)
		csv += fmt::format("{}|{}|{}|{}|{}|{}^", slot, d->ID, name, d->Price, qty, bp);
	};

	// AoTv4: only items placed INSIDE a Trader's Satchel are sellable -- not loose inventory or other
	// bags. Skip top-level items entirely; descend only into satchels.
	for (int16 i = EQ::invslot::GENERAL_BEGIN; i <= EQ::invslot::GENERAL_END; i++) {
		const EQ::ItemInstance *top = GetInv().GetItem(i);
		if (!top || !top->IsClassBag() || top->GetItem()->BagType != EQ::item::BagTypeTradersSatchel) {
			continue;
		}
		for (uint8 x = EQ::invbag::SLOT_BEGIN; x <= EQ::invbag::SLOT_END; x++) {
			consider(EQ::InventoryProfile::CalcSlotId(i, x));
		}
	}
	return csv;
}

// ================================ AoTv4 in-game search ("allaclone") ================================
// Backs the /search overlay window. kind = "item" | "npc" | "spell". The term is sanitized down to
// alphanumerics + space so a player's chat can never inject SQL. All lookups are read-only.

// LIST: up to 40 name matches as  "id|name^id|name^..."  (the dll renders the scrollable result list).
std::string Client::SearchList(std::string kind, std::string term)
{
	std::string t;                                   // sanitized LIKE term (alnum + space, capped)
	for (char c : term) {
		if (std::isalnum(static_cast<unsigned char>(c)) || c == ' ') t += c;
		if (t.size() >= 32) break;
	}
	// trim trailing spaces
	while (!t.empty() && t.back() == ' ') t.pop_back();
	if (t.empty()) return "";

	std::string query;
	if (kind == "item") {
		// include ALL tiers (native + Hallowed +300k + Mythic +600k) so players can find every quality;
		// order by BASE name (strip the tier prefix) so an item's native/Hallowed/Mythic rows sit together
		// and all three survive the row limit.
		query = fmt::format(
			"SELECT id, Name FROM items WHERE Name LIKE '%{}%' "
			"ORDER BY TRIM(LEADING 'Mythic ' FROM TRIM(LEADING 'Hallowed ' FROM Name)), id LIMIT 60", t);
	} else if (kind == "npc") {
		// only mobs a player can actually fight: spawned in the world, targetable (skip invisible-man
		// trigger race 127 and untargetable bodytype 11).
		query = fmt::format(
			"SELECT n.id, CONCAT(REPLACE(n.name,'_',' '),' (L',n.level,')') FROM npc_types n "
			"WHERE n.name LIKE '%{}%' AND n.race<>127 AND n.bodytype<>11 "
			"AND EXISTS (SELECT 1 FROM spawnentry se WHERE se.npcID=n.id) "
			"ORDER BY n.level, n.name LIMIT 40", t);
	} else if (kind == "spell") {
		// only real, learnable spells (classes8 = Bard learn level 1..70), deduped by name -- drops the
		// focus/AA/beta/click junk (classes8 254/255) and the many same-name rank rows.
		query = fmt::format(
			"SELECT MIN(id), name FROM spells_new WHERE name LIKE '%{}%' AND id < 10000 "
			"AND classes8 BETWEEN 1 AND 70 AND name NOT LIKE '%Rk.%' GROUP BY name ORDER BY name LIMIT 60", t);
	} else if (kind == "recipe") {
		// tradeskill recipes, deduped by name (same recipe often repeats per container).
		query = fmt::format(
			"SELECT MIN(id), name FROM tradeskill_recipe WHERE name LIKE '%{}%' AND enabled=1 "
			"GROUP BY name ORDER BY name LIMIT 60", t);
	} else {
		return "";
	}

	auto results = database.QueryDatabase(query);
	if (!results.Success()) return "";

	std::string csv;
	for (auto row : results) {
		std::string name = row[1] ? row[1] : "";
		for (auto &ch : name) { if (ch == '|' || ch == '^' || ch == '~') ch = ' '; }
		csv += fmt::format("{}|{}^", row[0], name);
	}
	return csv;
}

// AoTv4 search helpers ------------------------------------------------------------------------
// Readable spell description (db_str type 6) with #N / @N placeholders filled from the spell's effect
// base/max values. Returned cleaned of | ^ ~ so it can be embedded in a SearchDetail line. Used by both
// the spell detail and (to explain an item's clicky/focus/proc) the item detail.
// Tidy a "between X and Y" phrase once the placeholders are filled. Two things go wrong on real data:
// the two numbers can be EQUAL (a spell that does not scale, so the "range" is one value), and they
// can arrive HIGH FIRST (the template assumes base is the smaller magnitude and that is not always
// true). Both read as a bug to a player. Only the numeric span is rewritten; the surrounding wording
// is left exactly as the db_str author wrote it.
static void AoTv4FixRangePhrases(std::string &s)
{
	const std::string kBet = "between ", kAnd = " and ";
	size_t at = 0;
	while ((at = s.find(kBet, at)) != std::string::npos) {
		const size_t n1 = at + kBet.size();
		size_t e1 = n1;
		while (e1 < s.size() && isdigit((unsigned char)s[e1])) { ++e1; }
		if (e1 == n1 || s.compare(e1, kAnd.size(), kAnd) != 0) { at = n1; continue; }

		const size_t n2 = e1 + kAnd.size();
		size_t e2 = n2;
		while (e2 < s.size() && isdigit((unsigned char)s[e2])) { ++e2; }
		if (e2 == n2) { at = n1; continue; }

		const int a = Strings::ToInt(s.substr(n1, e1 - n1));
		const int b = Strings::ToInt(s.substr(n2, e2 - n2));

		// "between 2 and 2" -> "2": drop the whole span including the "between " lead-in.
		if (a == b) {
			s.replace(at, e2 - at, std::to_string(a));
			at += std::to_string(a).size();
			continue;
		}
		if (a > b) {   // "between 20 and 2" -> "between 2 and 20"
			const std::string rebuilt = std::to_string(b) + kAnd + std::to_string(a);
			s.replace(n1, e2 - n1, rebuilt);
			at = n1 + rebuilt.size();
			continue;
		}
		at = e2;
	}
}

static std::string AoTv4SpellDesc(uint32 sid)
{
	if (sid == 0) return "";
	auto r = database.QueryDatabase(fmt::format(
		"SELECT effect_base_value1, effect_base_value2, effect_base_value3, effect_base_value4, "
		"max1, max2, max3, max4 FROM spells_new WHERE id = {}", sid));
	if (!r.Success() || r.RowCount() == 0) return "";
	auto row = r.begin();
	auto I   = [&](int c) { return row[c] ? Strings::ToInt(row[c]) : 0; };
	auto dq  = database.QueryDatabase(fmt::format("SELECT value FROM db_str WHERE id = {} AND type = 6", sid));
	if (!dq.Success() || dq.RowCount() == 0 || !dq.begin()[0] || !dq.begin()[0][0]) return "";
	std::string desc = dq.begin()[0];
	int base[5] = { 0, I(0), I(1), I(2), I(3) };
	int mx[5]   = { 0, I(4), I(5), I(6), I(7) };
	auto rep = [&](const std::string &tok, int v) {
		std::string vs = std::to_string(v < 0 ? -v : v); size_t p;
		while ((p = desc.find(tok)) != std::string::npos) desc.replace(p, tok.size(), vs);
	};
	// ⚠️⚠️ maxN == 0 MEANS "NO CAP", NOT "up to zero". CalcSpellEffectValue only clamps when max is
	// non-zero, so a spell that does not scale with level leaves it at 0 -- and the stock db_str
	// template "between #1 and @1 damage" then rendered as "between 2 and 0 damage", which reads
	// backwards and looks like a data error. Fall back to the base so the range is real. 52 of the
	// 568 pool spells carrying that phrase are in this state.
	for (int n = 4; n >= 1; --n) {
		rep(fmt::format("#{}", n), base[n]);
		rep(fmt::format("@{}", n), mx[n] != 0 ? mx[n] : base[n]);
	}
	AoTv4FixRangePhrases(desc);
	for (auto &c : desc) { if (c == '|' || c == '^' || c == '~') c = ' '; }
	return desc;
}

// Word-wrap `text` into '~'-separated lines (each prefixed with `prefix`) appended to `out`.
static void AoTv4WrapInto(std::string &out, const std::string &text, const char *prefix, size_t W)
{
	size_t pos = 0;
	while (pos < text.size()) {
		size_t take = text.size() - pos;
		if (take > W) { size_t sp = text.rfind(' ', pos + W); take = (sp == std::string::npos || sp <= pos) ? W : sp - pos; }
		out += "~"; out += prefix; out += text.substr(pos, take);
		pos += take; while (pos < text.size() && text[pos] == ' ') pos++;
	}
}

// DETAIL: one row's full info, lines separated by '~' (the dll splits into display lines).
std::string Client::SearchDetail(std::string kind, uint32 id, bool include_sources)
{
	auto clean = [](std::string s) { for (auto &c : s) { if (c == '|' || c == '^' || c == '~') c = ' '; } return s; };

	if (kind == "item") {
		const EQ::ItemData *d = database.GetItem(id);
		if (!d) return "";
		std::string s = clean(d->Name);

		// --- flags line (Magic / Lore / No Trade / Attunable + our tier tag) ---
		// NOTE: never put square brackets in this text -- the RoF2 client auto-converts [Name] chat into a
		// (broken) item link. Comma-separated, native-style, no brackets.
		std::string flags;
		auto addf = [&](const char *f) { if (!flags.empty()) flags += ", "; flags += f; };
		if (d->Magic)          addf("Magic");
		if (d->LoreGroup != 0) addf("Lore");
		if (d->NoDrop == 0)    addf("No Trade");
		if (d->NoRent == 0)    addf("Temporary");
		if (d->Attuneable)     addf("Attunable");
		{ int tier = id / 300000; if (tier == 1) addf("Hallowed"); else if (tier == 2) addf("Mythic"); }
		if (!flags.empty()) s += "~" + flags;

		// --- Class / Race (65535 == all playable -> "ALL") ---
		auto maskStr = [](uint32 m, const char *const *tbl) -> std::string {
			if (m == 0 || (m & 0xFFFF) == 0xFFFF) return "ALL";
			std::string r; for (int i = 0; i < 16; ++i) if (m & (1u << i)) { if (!r.empty()) r += " "; r += tbl[i]; }
			return r.empty() ? "NONE" : r;
		};
		static const char *CLS[] = { "WAR","CLR","PAL","RNG","SHD","DRU","MNK","BRD","ROG","SHM","NEC","WIZ","MAG","ENC","BST","BER" };
		static const char *RAC[] = { "HUM","BAR","ERU","ELF","HIE","DEF","HEF","DWF","TRL","OGR","HFL","GNM","IKS","VAH","FRG","DRK" };
		s += "~Class: " + maskStr(d->Classes, CLS);
		s += "~Race: "  + maskStr(d->Races,   RAC);

		// --- equip slot names ---
		static const char *SLOT[] = { "Charm","Ear","Head","Face","Ear","Neck","Shoulders","Arms","Back",
			"Wrist","Wrist","Range","Hands","Primary","Secondary","Finger","Finger","Chest","Legs","Feet","Waist","Ammo" };
		std::string slots;
		for (int b = 0; b < 22; ++b) if (d->Slots & (1u << b)) {
			std::string nm = SLOT[b];
			if (slots.find(nm) == std::string::npos) { if (!slots.empty()) slots += " "; slots += nm; }
		}
		s += fmt::format("~Slot: {}", slots.empty() ? "NONE" : slots);

		// --- Size / Weight / (weapon) Skill ---
		static const char *SZ[] = { "TINY","SMALL","MEDIUM","LARGE","GIANT" };
		std::string sz = fmt::format("Size: {}   Weight: {:.1f}", (d->Size <= 4) ? SZ[d->Size] : "?", d->Weight / 10.0);
		const char *skill = nullptr;
		switch (d->ItemType) {
			case 0: skill = "1H Slashing"; break; case 1: skill = "2H Slashing"; break; case 2: skill = "Piercing"; break;
			case 3: skill = "1H Blunt"; break;    case 4: skill = "2H Blunt"; break;    case 5: skill = "Archery"; break;
			case 7: skill = "Throwing"; break;    case 8: skill = "Shield"; break;      case 35: skill = "2H Piercing"; break;
			case 45: skill = "Hand to Hand"; break; case 23: skill = "Wind Inst."; break; case 24: skill = "Stringed Inst."; break;
			case 25: skill = "Brass Inst."; break;  case 26: skill = "Percussion Inst."; break;
		}
		if (skill) sz += fmt::format("   Skill: {}", skill);
		s += "~" + sz;

		// --- weapon dmg / delay, or armor AC ---
		if (d->Damage) s += fmt::format("~Base Dmg: {}   Delay: {}   Ratio: {:.2f}", d->Damage, d->Delay, d->Delay ? (double)d->Damage / d->Delay : 0.0);
		if (d->ElemDmgAmt) { static const char *ED[] = { "","Magic","Fire","Cold","Poison","Disease" };
			s += fmt::format("~Elem Dmg: {} {}", d->ElemDmgAmt, (d->ElemDmgType >= 1 && d->ElemDmgType <= 5) ? ED[d->ElemDmgType] : ""); }
		if (d->AC) s += fmt::format("~AC: {}", d->AC);

		// --- HP / Mana / Endurance (only nonzero) ---
		{ std::string hm; auto add = [&](const char *n, int v) { if (v) { if (!hm.empty()) hm += "   "; hm += fmt::format("{}: {}", n, v); } };
		  add("HP", d->HP); add("Mana", d->Mana); add("End", d->Endur); if (!hm.empty()) s += "~" + hm; }

		// --- attributes (only nonzero, like the native window) ---
		{ std::string st; auto add = [&](const char *n, int v) { if (v) { if (!st.empty()) st += "  "; st += fmt::format("{} {}", n, v); } };
		  add("STR", d->AStr); add("STA", d->ASta); add("AGI", d->AAgi); add("DEX", d->ADex);
		  add("WIS", d->AWis); add("INT", d->AInt); add("CHA", d->ACha); if (!st.empty()) s += "~" + st; }

		// --- resists (only nonzero) ---
		{ std::string rs; auto add = [&](const char *n, int v) { if (v) { if (!rs.empty()) rs += "  "; rs += fmt::format("{} {}", n, v); } };
		  add("Magic", d->MR); add("Fire", d->FR); add("Cold", d->CR); add("Disease", d->DR); add("Poison", d->PR); add("Corrupt", d->SVCorruption);
		  if (!rs.empty()) s += "~Resists: " + rs; }

		if (d->SpellDmg || d->HealAmt) s += fmt::format("~Spell Dmg: {}   Heal Amt: {}", d->SpellDmg, d->HealAmt);

		// --- clicky / focus / proc / worn effects (resolve the spell name) ---
		auto spellName = [&](int sid) -> std::string {
			if (sid <= 0) return "";
			auto q = database.QueryDatabase(fmt::format("SELECT name FROM spells_new WHERE id = {}", sid));
			return (q.Success() && q.RowCount() && q.begin()[0]) ? clean(q.begin()[0]) : "";
		};
		// each effect shows its NAME then an indented description of what it does (db_str type 6)
		auto effLine = [&](int sid, const char *label) {
			std::string nm = spellName(sid); if (nm.empty()) return;
			s += "~" + std::string(label) + ": " + nm;
			std::string desc = AoTv4SpellDesc((uint32) sid);
			if (!desc.empty()) AoTv4WrapInto(s, desc, "    ", 50);
		};
		effLine(d->Click.Effect, "Effect (Click)");
		effLine(d->Proc.Effect,  "Combat Proc");
		effLine(d->Focus.Effect, "Focus");
		effLine(d->Worn.Effect,  "Worn");
		// ⚠️ WHAT THE ITEM IS, for everything that is not equipment. Every line above this point is gated
		// on a weapon/armour stat -- damage, AC, HP, resists, effects -- so a rogue poison, a tradeskill
		// component or a piece of trash produced a name and then NOTHING, which reads as a broken panel
		// rather than an item with no stats. Type, stack size, value and the tradeskill flag are the
		// facts that actually describe those.
		{
			static const std::map<int, const char *> TYPES = {
				{  0, "1H Slashing" }, {  1, "2H Slashing" }, {  2, "Piercing"    }, {  3, "1H Blunt"  },
				{  4, "2H Blunt"    }, {  5, "Bow"         }, {  8, "Shield"      }, {  9, "Scroll"    },
				{ 10, "Armor"       }, { 11, "Misc"        }, { 12, "Lockpick"    }, { 14, "Food"      },
				{ 15, "Drink"       }, { 16, "Light"       }, { 17, "Tradeskill"  }, { 18, "Bandage"   },
				{ 19, "Throwing"    }, { 20, "Spell"       }, { 21, "Potion"      }, { 23, "Wind Instrument" },
				{ 24, "Stringed Instrument" }, { 25, "Brass Instrument" }, { 26, "Percussion Instrument" },
				{ 27, "Arrow"       }, { 29, "Jewelry"     }, { 31, "Book"        }, { 32, "Note"      },
				{ 33, "Key"         }, { 34, "Coin"        }, { 36, "Fishing Pole"}, { 37, "Fishing Bait" },
				{ 38, "Alcohol"     }, { 39, "Key"         }, { 40, "Compass"     }, { 42, "Poison"    },
				{ 45, "Martial"     }, { 52, "Charm"       }, { 53, "Dye"         }, { 54, "Augmentation" },
				{ 60, "Recipe"      }, { 61, "Recipe"      }, { 63, "Alt Currency"}, { 66, "Collectible" },
				{ 67, "Container"   }
			};
			const auto ti = TYPES.find((int) d->ItemType);
			std::string tl = fmt::format("Type: {}", ti != TYPES.end() ? ti->second : "Misc");
			if (d->Tradeskills)            tl += "  (tradeskill)";
			if (d->Stackable && d->StackSize > 1) tl += fmt::format("   Stacks to {}", d->StackSize);
			if (d->MaxCharges > 0)         tl += fmt::format("   Charges: {}", d->MaxCharges);
			s += "~" + tl;

			// Value, in the coin the player actually sees.
			if (d->Price > 0) {
				const uint32 p = d->Price;
				s += fmt::format("~Value: {}p {}g {}s {}c", p / 1000, (p / 100) % 10, (p / 10) % 10, p % 10);
			}
		}

		if (d->ReqLevel) s += fmt::format("~Required Level: {}", d->ReqLevel);
		if (d->RecLevel) s += fmt::format("~Recommended Level: {}", d->RecLevel);

		// "Dropped by" -- reverse loot lookup (use the native base id; tiers roll off the same tables).
		// Skipped for the loot window: standing over a corpse, where else the thing drops is noise, and
		// this is a three table join run on every inspect.
		if (!include_sources) { return s; }
		auto dropr = database.QueryDatabase(fmt::format(
			"SELECT DISTINCT REPLACE(n.name,'_',' ') FROM npc_types n "
			"JOIN loottable_entries lte ON lte.loottable_id = n.loottable_id "
			"JOIN lootdrop_entries lde ON lde.lootdrop_id = lte.lootdrop_id "
			"WHERE lde.item_id = {} AND n.race<>127 ORDER BY n.name LIMIT 15", id % 300000));
		if (dropr.Success() && dropr.RowCount()) {
			s += "~Dropped by:";
			for (auto dr2 : dropr) { s += "~  " + clean(dr2[0] ? dr2[0] : ""); }
		}
		return s;
	}

	if (kind == "npc") {
		// only HP matters (per design); plus the zones (LONG name) it spawns in and what it drops.
		auto r = database.QueryDatabase(fmt::format(
			"SELECT REPLACE(name,'_',' '), hp FROM npc_types WHERE id = {}", id));
		if (!r.Success() || r.RowCount() == 0) return "";
		auto row = r.begin();
		std::string s = clean(row[0]);
		s += fmt::format("~Hit Points: {}", row[1]);

		auto zr = database.QueryDatabase(fmt::format(
			"SELECT DISTINCT z.long_name FROM spawn2 s "
			"JOIN spawnentry se ON se.spawngroupID = s.spawngroupID "
			"JOIN zone z ON z.short_name = s.zone WHERE se.npcID = {} LIMIT 10", id));
		if (zr.Success() && zr.RowCount()) {
			s += "~Zones:";
			for (auto zrow : zr) { s += "~  " + clean(zrow[0] ? zrow[0] : ""); }
		}

		auto lr = database.QueryDatabase(fmt::format(
			"SELECT DISTINCT i.Name FROM npc_types n "
			"JOIN loottable_entries lte ON lte.loottable_id = n.loottable_id "
			"JOIN lootdrop_entries lde ON lde.lootdrop_id = lte.lootdrop_id "
			"JOIN items i ON i.id = lde.item_id WHERE n.id = {} AND i.id < 300000 ORDER BY i.Name LIMIT 30", id));
		if (lr.Success() && lr.RowCount()) {
			s += "~Drops:";
			for (auto lrow : lr) { s += "~  " + clean(lrow[0] ? lrow[0] : ""); }
		}
		return s;
	}

	if (kind == "spell") {
		auto r = database.QueryDatabase(fmt::format(
			"SELECT name, mana, cast_time, classes8, buffduration, recast_time, `range`, resisttype, "
			"effect_base_value1, effect_base_value2, effect_base_value3, effect_base_value4, "
			"max1, max2, max3, max4 FROM spells_new WHERE id = {}", id));
		if (!r.Success() || r.RowCount() == 0) return "";
		auto row = r.begin();
		auto I   = [&](int col) { return row[col] ? Strings::ToInt(row[col]) : 0; };
		std::string s = clean(row[0] ? row[0] : "");

		// line: mana / cast / recast / range / learn level
		// ⚠️ Cast time in SECONDS, not the raw milliseconds the column holds. "Cast 2500ms" is the DB's
		// unit, not a player's -- and this string is now also the level-up picker's description, where the
		// cast time is one of the few things worth comparing between three choices at a glance.
		const double cast_s = I(2) / 1000.0;
		std::string l = fmt::format("Mana {}  Cast {:.1f}s", I(1), cast_s);
		if (I(5) > 0) l += fmt::format("  Recast {}s", I(5) / 1000);
		if (I(6) > 0) l += fmt::format("  Range {}", I(6));
		l += fmt::format("  Learn L{}", I(3));
		s += "~" + l;

		// line: resist + duration
		static const char *RES[] = { "Unresistable", "Magic", "Fire", "Cold", "Poison", "Disease",
		                             "Chromatic", "Prismatic", "Physical", "Corruption" };
		int rt = I(7), bd = I(4);
		std::string l2 = fmt::format("Resist: {}", (rt >= 0 && rt <= 9) ? RES[rt] : "?");
		if (bd > 0) l2 += fmt::format("   Duration: {} ticks (~{}s)", bd, bd * 6);
		s += "~" + l2;

		// human-readable description (db_str type 6) with #N / @N placeholders filled from the effect
		// base/max values, then word-wrapped so the scrollable detail panel shows all of it.
		auto dr = database.QueryDatabase(fmt::format("SELECT value FROM db_str WHERE id = {} AND type = 6", id));
		if (dr.Success() && dr.RowCount() && dr.begin()[0] && dr.begin()[0][0]) {
			std::string desc = dr.begin()[0];
			int base[5] = { 0, I(8),  I(9),  I(10), I(11) };
			int mx[5]   = { 0, I(12), I(13), I(14), I(15) };
			auto rep = [&](const std::string &tok, int v) {
				std::string vs = std::to_string(v < 0 ? -v : v); size_t p;
				while ((p = desc.find(tok)) != std::string::npos) desc.replace(p, tok.size(), vs);
			};
			for (int n = 4; n >= 1; --n) { rep(fmt::format("#{}", n), base[n]); rep(fmt::format("@{}", n), mx[n]); }

			// ⚠️ "%z" IS THE DURATION TOKEN, not a percent sign. Only #N and  were being filled, so a
			// description like "increasing their hit points and armor class for %z." rendered as a bare
			// "for %." and looked like a formatting bug. EQ descriptions use %z for the buff duration.
			{
				const std::string dur = (bd > 0) ? fmt::format("{} ticks", bd) : "the duration";
				size_t p;
				while ((p = desc.find("%z")) != std::string::npos) { desc.replace(p, 2, dur); }
				while ((p = desc.find("%Z")) != std::string::npos) { desc.replace(p, 2, dur); }
			}

			desc = clean(desc);
			s += "~";                                            // blank separator line
			const size_t W = 52; size_t pos = 0;
			while (pos < desc.size()) {
				size_t take = desc.size() - pos;
				if (take > W) { size_t sp = desc.rfind(' ', pos + W); take = (sp == std::string::npos || sp <= pos) ? W : sp - pos; }
				s += "~" + desc.substr(pos, take);
				pos += take; while (pos < desc.size() && desc[pos] == ' ') pos++;
			}
		}
		return s;
	}

	if (kind == "recipe") {
		auto r = database.QueryDatabase(fmt::format(
			"SELECT name, tradeskill, trivial FROM tradeskill_recipe WHERE id = {}", id));
		if (!r.Success() || r.RowCount() == 0) return "";
		auto row = r.begin();
		std::string s = clean(row[0] ? row[0] : "");
		int ts = row[1] ? Strings::ToInt(row[1]) : 0;
		const char *tsn = "Tradeskill";
		switch (ts) {
			case 55: tsn = "Fishing"; break;      case 56: tsn = "Make Poison"; break;   case 57: tsn = "Tinkering"; break;
			case 58: tsn = "Research"; break;     case 59: tsn = "Alchemy"; break;       case 60: tsn = "Baking"; break;
			case 61: tsn = "Tailoring"; break;    case 63: tsn = "Blacksmithing"; break; case 64: tsn = "Fletching"; break;
			case 65: tsn = "Brewing"; break;      case 68: tsn = "Jewelry Making"; break;case 69: tsn = "Pottery"; break;
			case 75: tsn = "Poison Making"; break;
		}
		s += fmt::format("~Skill: {}   Trivial: {}", tsn, row[2] ? Strings::ToInt(row[2]) : 0);

		// what the combine yields
		auto yq = database.QueryDatabase(fmt::format(
			"SELECT i.Name, e.successcount FROM tradeskill_recipe_entries e JOIN items i ON i.id = e.item_id "
			"WHERE e.recipe_id = {} AND e.successcount > 0 ORDER BY i.Name", id));
		if (yq.Success() && yq.RowCount()) {
			s += "~Yields:";
			for (auto y : yq) { int c = y[1] ? Strings::ToInt(y[1]) : 1; s += "~  " + clean(y[0] ? y[0] : "") + (c > 1 ? fmt::format(" x{}", c) : ""); }
		}
		// ingredients consumed
		auto cq = database.QueryDatabase(fmt::format(
			"SELECT i.Name, e.componentcount FROM tradeskill_recipe_entries e JOIN items i ON i.id = e.item_id "
			"WHERE e.recipe_id = {} AND e.componentcount > 0 ORDER BY i.Name", id));
		if (cq.Success() && cq.RowCount()) {
			s += "~Components:";
			for (auto c : cq) { int n = c[1] ? Strings::ToInt(c[1]) : 1; s += "~  " + clean(c[0] ? c[0] : "") + (n > 1 ? fmt::format(" x{}", n) : ""); }
		}
		// container it must be combined in
		auto kq = database.QueryDatabase(fmt::format(
			"SELECT DISTINCT i.Name FROM tradeskill_recipe_entries e JOIN items i ON i.id = e.item_id "
			"WHERE e.recipe_id = {} AND e.iscontainer = 1 ORDER BY i.Name LIMIT 6", id));
		if (kq.Success() && kq.RowCount()) {
			s += "~Made in:";
			for (auto k : kq) { s += "~  " + clean(k[0] ? k[0] : ""); }
		}
		return s;
	}

	return "";
}

// The player's current shop rows as "item_sn|item_id|name|unit_cost" CSV -- feeds the dll "My Shop" tab.
std::string Client::GetMyShopListing()
{
	auto        rows = TraderRepository::GetWhere(database, fmt::format("`char_id` = '{}'", CharacterID()));
	std::string csv;
	for (const auto &r : rows) {
		const EQ::ItemData *d    = database.GetItem(r.item_id);
		std::string         name = d ? d->Name : fmt::format("item {}", r.item_id);
		for (auto &ch : name) { if (ch == '|' || ch == '^') { ch = ' '; } }
		int qty = r.item_charges > 0 ? r.item_charges : 1;   // listed stack size
		csv += fmt::format("{}|{}|{}|{}|{}^", r.item_sn, r.item_id, name, r.item_cost, qty);
	}
	return csv;
}

// AoTv4 price book: persist item_id -> price and append a change-log entry (old->new, unix time).
void Client::SetItemPrice(uint32 item_id, uint32 price)
{
	if (item_id == 0) {
		return;
	}
	std::string book_key  = fmt::format("shopbook_{}", CharacterID());
	uint32      old_price = 0;
	std::string out;
	bool        found = false;
	for (const auto &tok : Strings::Split(DataBucket::GetData(&database, book_key), ",")) {
		auto kv = Strings::Split(tok, ":");
		if (kv.size() < 2) { continue; }
		uint32 iid = Strings::ToUnsignedInt(kv[0]);
		uint32 p   = Strings::ToUnsignedInt(kv[1]);
		if (iid == item_id) { old_price = p; p = price; found = true; }
		out += fmt::format("{}{}:{}", out.empty() ? "" : ",", iid, p);
	}
	if (!found) { out += fmt::format("{}{}:{}", out.empty() ? "" : ",", item_id, price); }
	DataBucket::SetData(&database, book_key, out);

	// change log: newest-first, capped at 20 entries  (item:old:new:unixtime)
	std::string log_key = fmt::format("shoplog_{}", CharacterID());
	std::string newlog  = fmt::format("{}:{}:{}:{}", item_id, old_price, price, (long) time(nullptr));
	int         kept    = 1;
	for (const auto &e : Strings::Split(DataBucket::GetData(&database, log_key), ",")) {
		if (e.empty() || kept >= 20) { continue; }
		newlog += "," + e; ++kept;
	}
	DataBucket::SetData(&database, log_key, newlog);
}

// The price book as "itemid|name|price^..." (feeds the Pricing-tab list + Inventory-tab projected prices).
std::string Client::GetPriceBook()
{
	std::string csv;
	for (const auto &tok : Strings::Split(DataBucket::GetData(&database, fmt::format("shopbook_{}", CharacterID())), ",")) {
		auto kv = Strings::Split(tok, ":");
		if (kv.size() < 2) { continue; }
		uint32              iid  = Strings::ToUnsignedInt(kv[0]);
		const EQ::ItemData *d    = database.GetItem(iid);
		std::string         name = d ? d->Name : fmt::format("item {}", iid);
		for (auto &ch : name) { if (ch == '|' || ch == '^') { ch = ' '; } }
		csv += fmt::format("{}|{}|{}^", iid, name, Strings::ToUnsignedInt(kv[1]));
	}
	return csv;
}

// Recent price changes as "itemid|name|old|new|unixtime^..." (newest first).
std::string Client::GetPriceLog()
{
	std::string csv;
	for (const auto &tok : Strings::Split(DataBucket::GetData(&database, fmt::format("shoplog_{}", CharacterID())), ",")) {
		auto kv = Strings::Split(tok, ":");
		if (kv.size() < 4) { continue; }
		uint32              iid  = Strings::ToUnsignedInt(kv[0]);
		const EQ::ItemData *d    = database.GetItem(iid);
		std::string         name = d ? d->Name : fmt::format("item {}", iid);
		for (auto &ch : name) { if (ch == '|' || ch == '^') { ch = ' '; } }
		csv += fmt::format("{}|{}|{}|{}|{}^", iid, name, kv[1], kv[2], kv[3]);
	}
	return csv;
}


// ---------------------------------------------------------------------------------------------
// AoTv4 shop history: what you have LISTED, and what has SOLD.
//
// Both follow the price log above exactly: a newest-first, capped, comma-separated data bucket, with
// a reader that resolves item names at display time. Kept as buckets rather than a table because the
// price log already proved the shape and a table would need a migration for two panels.
//
// ⚠️ CAPPED. A bucket is one string; without a cap these grow for the life of the character.
// ---------------------------------------------------------------------------------------------
static const int AOTV4_SHOP_LOG_MAX = 60;

// Append to a capped newest-first bucket.
//
// ⚠️ Uses the zone-global `database` directly rather than taking one: DataBucket wants a
// SharedDatabase*, and passing `Database&` does not convert.
static void AoTv4PushShopLog(const std::string &key, const std::string &entry)
{
	std::string out  = entry;
	int         kept = 1;
	for (const auto &e : Strings::Split(DataBucket::GetData(&database, key), ",")) {
		if (e.empty() || kept >= AOTV4_SHOP_LOG_MAX) { continue; }
		out += "," + e;
		++kept;
	}
	DataBucket::SetData(&database, key, out);
}

// Record a listing.  item:qty:price:unixtime
void Client::RecordShopListing(uint32 item_id, int32 qty, uint32 price)
{
	if (item_id == 0) { return; }
	AoTv4PushShopLog(
		fmt::format("shoplist_{}", CharacterID()),
		fmt::format("{}:{}:{}:{}", item_id, qty, price, (long) time(nullptr))
	);
}

// Record a sale.  item:qty:price:buyer:unixtime
//
// ⚠️ The buyer name is stored, not looked up later: by the time anyone opens the panel the buyer may
// be offline, in another zone, or renamed. Delimiters are stripped so a name cannot split the entry.
void Client::RecordShopSale(uint32 item_id, int32 qty, uint32 price, const std::string &buyer)
{
	if (item_id == 0) { return; }
	std::string who = buyer.empty() ? "someone" : buyer;
	for (auto &ch : who) { if (ch == ':' || ch == ',' || ch == '|' || ch == '^') { ch = ' '; } }
	AoTv4PushShopLog(
		fmt::format("shopsold_{}", CharacterID()),
		fmt::format("{}:{}:{}:{}:{}", item_id, qty, price, who, (long) time(nullptr))
	);
}

// Listings you have made, "itemid|name|qty|price|unixtime^..." (newest first).
std::string Client::GetShopListingLog()
{
	std::string csv;
	for (const auto &tok : Strings::Split(DataBucket::GetData(&database, fmt::format("shoplist_{}", CharacterID())), ",")) {
		auto kv = Strings::Split(tok, ":");
		if (kv.size() < 4) { continue; }
		uint32              iid  = Strings::ToUnsignedInt(kv[0]);
		const EQ::ItemData *d    = database.GetItem(iid);
		std::string         name = d ? d->Name : fmt::format("item {}", iid);
		for (auto &ch : name) { if (ch == '|' || ch == '^') { ch = ' '; } }
		csv += fmt::format("{}|{}|{}|{}|{}^", iid, name, kv[1], kv[2], kv[3]);
	}
	return csv;
}

// Sales you have made, "itemid|name|qty|price|buyer|unixtime^..." (newest first).
std::string Client::GetShopSoldLog()
{
	std::string csv;
	for (const auto &tok : Strings::Split(DataBucket::GetData(&database, fmt::format("shopsold_{}", CharacterID())), ",")) {
		auto kv = Strings::Split(tok, ":");
		if (kv.size() < 5) { continue; }
		uint32              iid  = Strings::ToUnsignedInt(kv[0]);
		const EQ::ItemData *d    = database.GetItem(iid);
		std::string         name = d ? d->Name : fmt::format("item {}", iid);
		for (auto &ch : name) { if (ch == '|' || ch == '^') { ch = ' '; } }
		csv += fmt::format("{}|{}|{}|{}|{}|{}^", iid, name, kv[1], kv[2], kv[3], kv[4]);
	}
	return csv;
}
// Add inventory items to the shop. `csv` = "slot:price,slot:price,..." (price is absolute copper). Each
// item is escrowed OUT of the player's bags into a `trader` row with a unique per-character item_sn.
// Returns the number added.
int Client::AddItemsToShop(std::string csv)
{
	// AoTv4: no listing while in combat. The shop escrow is death-safe (survives the roguelite wipe),
	// so a player could dodge the death penalty by dumping gear into the shop mid-fight. GetAggroCount()
	// > 0 means NPCs are actively engaged on you.
	if (GetAggroCount() > 0) {
		Message(Chat::Red, "You cannot list items in your shop while in combat.");
		return 0;
	}

	// AoTv4: hard-cap the shop at SHOP_MAX listings (server-authoritative; the dll window shows up to 80).
	const int SHOP_MAX  = 40;
	int       shop_have = (int) TraderRepository::GetWhere(
		database, fmt::format("`char_id` = '{}'", CharacterID())).size();
	if (shop_have >= SHOP_MAX) {
		Message(Chat::Red, fmt::format("Your shop is full ({} item limit).", SHOP_MAX).c_str());
		return 0;
	}

	// unique, monotonic serial per character (survives restarts, unlike runtime item serials)
	uint32      next_sn = 1;
	std::string sn_key  = fmt::format("shopsn_{}", CharacterID());
	std::string sn_prev = DataBucket::GetData(&database, sn_key);
	if (!sn_prev.empty()) {
		next_sn = (uint32) Strings::ToUnsignedInt(sn_prev);
	}

	std::map<uint32, uint32>              book = LoadShopBook(CharacterID());   // prices come from the book
	std::vector<TraderRepository::Trader> rows;
	std::vector<std::pair<int16, uint8>>  slots;   // (slot, delete_qty) -- escrowed ONLY after rows persist
	bool                                  hit_cap = false;
	for (const auto &tok : Strings::Split(csv, ",")) {
		if (tok.empty()) {
			continue;
		}
		auto kv = Strings::Split(tok, ":");
		if (kv.empty()) {
			continue;
		}
		int16  slot  = (int16) Strings::ToInt(kv[0]);
		int    want  = (kv.size() >= 2) ? Strings::ToInt(kv[1]) : 0;   // how many to list; 0 = whole stack
		const EQ::ItemInstance *it = GetInv().GetItem(slot);
		if (!it || !it->GetItem() || it->GetItem()->NoDrop == 0 || it->IsClassBag()) {
			continue;   // stale slot / No-Drop / container -- skip
		}
		uint32 price = book.count(it->GetID()) ? book[it->GetID()] : 0;   // price-book price for this item
		if (price == 0) {
			continue;   // unpriced -- set a price on the Pricing tab first
		}
		// AoTv4: the item must live INSIDE a Trader's Satchel (validate server-side, not just the dll).
		int16                   parent     = EQ::InventoryProfile::CalcSlotId(slot);
		const EQ::ItemInstance *parent_bag = (parent < 0) ? nullptr : GetInv().GetItem(parent);
		if (!parent_bag || !parent_bag->GetItem() ||
			parent_bag->GetItem()->BagType != EQ::item::BagTypeTradersSatchel) {
			continue;
		}

		// stop once the shop hits the cap (existing listings + what we've queued this batch)
		if (shop_have + (int) rows.size() >= SHOP_MAX) {
			hit_cap = true;
			break;
		}

		TraderRepository::Trader r{};
		r.id                    = 0;
		r.char_entity_id        = GetID();
		r.char_id               = CharacterID();
		r.char_zone_id          = GetZoneID();
		r.char_zone_instance_id = GetInstanceID();
		int full     = it->GetCharges() > 0 ? it->GetCharges() : 1;   // actual stack size
		int list_qty = (want <= 0 || want >= full) ? full : want;     // clamp; 0/over = whole stack
		r.item_charges          = list_qty;
		r.item_cost             = price;
		r.item_id               = it->GetID();
		r.item_sn               = next_sn++;
		r.slot_id               = (uint8) (shop_have + rows.size());   // global slot, not per-batch (avoids collisions)
		r.listing_date          = time(nullptr);
		auto augs = it->GetAugmentIDs();   // const-safe; all 0 when not augmented
		if (augs.size() >= 6) {
			r.aug_slot_1 = augs[0];
			r.aug_slot_2 = augs[1];
			r.aug_slot_3 = augs[2];
			r.aug_slot_4 = augs[3];
			r.aug_slot_5 = augs[4];
			r.aug_slot_6 = augs[5];
		}
		rows.emplace_back(r);
		uint8 del_qty = (list_qty >= full) ? 0 : (uint8) list_qty;   // 0 = delete whole item, else partial
		slots.push_back({ slot, del_qty });
	}

	if (rows.empty()) {
		return 0;
	}

	// Persist the listings FIRST; only if that succeeds do we take the items from the player's bags.
	// This ordering means a DB failure can never destroy items (worst case: nothing happens).
	if (TraderRepository::InsertMany(database, rows) == 0) {
		Message(Chat::Red, "Could not list your items right now -- nothing was taken from your bags.");
		return 0;
	}
	for (auto &s : slots) {
		DeleteItemInInventory(s.first, s.second, true, true);   // ESCROW: del_qty 0 = whole stack, else partial
	}
	DataBucket::SetData(&database, sn_key, std::to_string(next_sn));

	// History for the List Item tab. Recorded HERE, after the rows are committed and the items taken,
	// so the log can never claim a listing that did not happen.
	for (const auto &r : rows) {
		RecordShopListing(r.item_id, r.item_charges, r.item_cost);
	}

	// register as a trader so the shop shows in Bazaar search (rows alone are DB-searchable, but this
	// keeps the world's trader view consistent). Buys route through the DB/parcel path.
	SetTrader(true);
	SendBecomeTraderToWorld(this, TraderOn);

	if (hit_cap) {
		Message(Chat::Yellow, fmt::format(
			"Some items were not listed -- your shop is limited to {} items.", SHOP_MAX).c_str());
	}
	return (int) rows.size();
}

// Pull one listing back out of the shop to the player's cursor (unlist). Returns 1 on success.
int Client::PullShopItem(uint32 serial)
{
	auto row = TraderRepository::GetItemBySerialNumber(database, serial, CharacterID());
	if (!row.id) {
		Message(Chat::Yellow, "That item isn't in your shop.");
		return 0;
	}

	EQ::ItemInstance *inst = database.CreateItem(
		row.item_id, row.item_charges,
		row.aug_slot_1, row.aug_slot_2, row.aug_slot_3, row.aug_slot_4, row.aug_slot_5, row.aug_slot_6
	);
	if (!inst) {
		Message(Chat::Red, "Could not pull that item.");
		return 0;
	}
	std::string name = inst->GetItem()->Name;
	PushItemOnCursor(*inst, true);
	safe_delete(inst);

	TraderRepository::DeleteOne(database, row.id);

	// if the shop is now empty, drop trader status
	auto left = TraderRepository::GetWhere(database, fmt::format("`char_id` = '{}' LIMIT 1", CharacterID()));
	if (left.empty() && IsTrader()) {
		SetTrader(false);
		SendBecomeTraderToWorld(this, TraderOff);
	}

	Message(Chat::Yellow, fmt::format("Pulled {} from your shop to your cursor.", name).c_str());
	return 1;
}
// ============================================================================================

// AoTv4 player-shop: start Bazaar trader mode SERVER-SIDE so a "Bazaar Broker" NPC can open a
// player's shop on a click in ANY city (the native /trader window is zone-gated to the Bazaar).
// Reads the player's Trader's Satchel and prices each item at its per-item multiplier -- stored by
// the Broker popup in data bucket "shopmult_<charid>_<itemid>", else `default_mult` -- times the
// item's vendor value, then synthesizes the native ClickTrader packet so TraderStartTrader does all
// the real work (no-drop validation, DB registration, the searchable trader spawn). Buyers find the
// shop via the native global "all traders" bazaar search + parcel delivery. Returns items listed.
int Client::StartPlayerTrader(uint32 default_mult)
{
	if (IsTrader()) {
		Message(Chat::Yellow, "Your shop is already open.");
		return 0;
	}
	if (default_mult < 1) {
		default_mult = 1;
	}

	auto inv = GetTraderItems();
	auto app = new EQApplicationPacket(OP_Trader, sizeof(ClickTrader_Struct));
	auto ct  = (ClickTrader_Struct *) app->pBuffer;
	ct->action = TraderOn;

	uint32 max_items = GetInv().GetLookup()->InventoryTypeSize.Bazaar;
	int    listed    = 0;
	for (uint32 i = 0; i < max_items && i < EQ::invtype::BAZAAR_SIZE; ++i) {
		if (inv->items[i] == 0 || inv->serial_number[i] == 0) {
			continue;
		}

		const EQ::ItemData *item   = database.GetItem(inv->items[i]);
		uint32              vendor = item ? item->Price : 0;
		uint32              price  = vendor * default_mult;   // default = default_mult x vendor value (copper)

		// absolute per-item price (copper) set in the vendor window, else the default above
		std::string pb = DataBucket::GetData(&database, fmt::format("shopprice_{}_{}", CharacterID(), inv->items[i]));
		if (!pb.empty()) {
			int p = Strings::ToInt(pb);
			if (p > 0) {
				price = (uint32) p;
			}
		}

		ct->serial_number[i] = inv->serial_number[i];
		ct->item_cost[i]     = price;                // 0 -> native loop skips it
		if (ct->item_cost[i] > 0) {
			++listed;
		}
	}
	safe_delete(inv);

	TraderStartTrader(app);   // native: validates no-drop, writes trader table, spawns the searchable trader
	safe_delete(app);

	// AoTv4 offline shop: record what got listed as `item_id:charges` pairs (read back from the rows the
	// native call just wrote, so it reflects no-drop/price filtering). On the owner's next login,
	// ReclaimOfflineShop compares this against the surviving rows to remove exactly what SOLD while
	// offline. We key on item_id+quantity, NOT serial number -- item instance serials are runtime-
	// generated (item_instance.cpp) and change across a relog, so a serial from this session won't
	// match the reloaded item next login.
	{
		auto rows = TraderRepository::GetWhere(database, fmt::format("`char_id` = '{}'", CharacterID()));
		std::string csv;
		for (const auto &r : rows) {
			csv += fmt::format("{}:{},", r.item_id, r.item_charges);
		}
		DataBucket::SetData(&database, fmt::format("shop_listed_{}", CharacterID()), csv);
	}
	return listed;
}

// AoTv4 player-shop: CSV of "itemid:vendorvalue" for each item in the player's Trader's Satchel, so
// the Bazaar Broker can drive the vendor price window (which shows real platinum = vendorvalue x the
// chosen tier). Duplicates possible (multiple stacks) -- the Lua side dedups. Empty = nothing to sell.
std::string Client::GetTraderSatchelItemIDs()
{
	auto        inv       = GetTraderItems();
	uint32      max_items = GetInv().GetLookup()->InventoryTypeSize.Bazaar;
	std::string out;
	for (uint32 i = 0; i < max_items && i < EQ::invtype::BAZAAR_SIZE; ++i) {
		if (inv->items[i] == 0) {
			continue;
		}
		const EQ::ItemData *item   = database.GetItem(inv->items[i]);
		uint32              vendor = item ? item->Price : 0;
		if (!out.empty()) {
			out += ",";
		}
		out += std::to_string(inv->items[i]) + ":" + std::to_string(vendor);
	}
	safe_delete(inv);
	return out;
}

void Client::SendTraderItem(uint32 ItemID, uint16 Quantity, TraderRepository::Trader &t) {

	std::string Packet;
	int16 FreeSlotID=0;

	const EQ::ItemData* item = database.GetItem(ItemID);

	if(!item){
		LogTrading("Bogus item deleted in Client::SendTraderItem!\n");
		return;
	}

	std::unique_ptr<EQ::ItemInstance> inst(
		database.CreateItem(
			item,
			Quantity,
			t.aug_slot_1,
			t.aug_slot_2,
			t.aug_slot_3,
			t.aug_slot_4,
			t.aug_slot_5,
			t.aug_slot_6
		)
	);

	if (inst)
	{
		bool is_arrow = (inst->GetItem()->ItemType == EQ::item::ItemTypeArrow) ? true : false;
		FreeSlotID = m_inv.FindFreeSlot(false, true, inst->GetItem()->Size, is_arrow);

		if (TryStacking(inst.get(), ItemPacketTrade, true, false)) {
		}
		else {
			PutItemInInventory(FreeSlotID, *inst);
			SendItemPacket(FreeSlotID, inst.get(), ItemPacketTrade);
		}
		Save();
	}
}

void Client::SendSingleTraderItem(uint32 char_id, int serial_number)
{
	auto inst = database.LoadSingleTraderItem(char_id, serial_number);
	if (inst) {
		SendItemPacket(EQ::invslot::slotCursor, inst.get(), ItemPacketMerchant); // MainCursor?
	}
}

void Client::BulkSendTraderInventory(uint32 char_id)
{
	const EQ::ItemData *item;

	auto   trader_items = TraderRepository::GetWhere(database, fmt::format("`char_id` = '{}'", char_id));
	uint32 item_limit   = trader_items.size() >= GetInv().GetLookup()->InventoryTypeSize.Bazaar ?
		GetInv().GetLookup()->InventoryTypeSize.Bazaar :
		trader_items.size();

	for (uint32 i = 0; i < item_limit; i++) {
		if ((trader_items.at(i).item_id == 0) || (trader_items.at(i).item_cost == 0)) {
			continue;
		}
		else {
			item = database.GetItem(trader_items.at(i).item_id);
		}

		if (item && (item->NoDrop != 0)) {
			std::unique_ptr<EQ::ItemInstance> inst(
				database.CreateItem(
					trader_items.at(i).item_id,
					trader_items.at(i).item_charges,
					trader_items.at(i).aug_slot_1,
					trader_items.at(i).aug_slot_2,
					trader_items.at(i).aug_slot_3,
					trader_items.at(i).aug_slot_4,
					trader_items.at(i).aug_slot_5,
					trader_items.at(i).aug_slot_6
				)
			);
			if (inst) {
				inst->SetSerialNumber(trader_items.at(i).item_sn);
				if (trader_items.at(i).item_charges > 0) {
					inst->SetCharges(trader_items.at(i).item_charges);
				}

				if (inst->IsStackable()) {
					inst->SetMerchantCount(trader_items.at(i).item_charges);
					inst->SetMerchantSlot(trader_items.at(i).item_sn);
				}

				inst->SetPrice(trader_items.at(i).item_cost);
				SendItemPacket(EQ::invslot::slotCursor, inst.get(), ItemPacketMerchant);
//				safe_delete(inst);
			}
			else
				LogTrading("Client::BulkSendTraderInventory nullptr inst pointer");
		}
	}
}

uint32 Client::FindTraderItemSerialNumber(int32 ItemID) {

	EQ::ItemInstance* item = nullptr;
	uint16 SlotID = 0;
	for (int i = EQ::invslot::GENERAL_BEGIN; i <= EQ::invslot::GENERAL_END; i++){
		item = GetInv().GetItem(i);
		if (item && item->GetItem()->BagType == EQ::item::BagTypeTradersSatchel){
			for (int x = EQ::invbag::SLOT_BEGIN; x <= EQ::invbag::SLOT_END; x++) {
				// we already have the parent bag and a contents iterator..why not just iterate the bag!??
				SlotID = EQ::InventoryProfile::CalcSlotId(i, x);
				item = GetInv().GetItem(SlotID);
				if (item) {
					if (item->GetID() == ItemID)
						return item->GetSerialNumber();
				}
			}
		}
	}
	LogTrading("Client::FindTraderItemSerialNumber Couldn't find item! Item ID [{}]", ItemID);

	return 0;
}

EQ::ItemInstance *Client::FindTraderItemBySerialNumber(int32 SerialNumber)
{
	EQ::ItemInstance *item   = nullptr;
	int16            slot_id = 0;

	for (int16 i = EQ::invslot::GENERAL_BEGIN; i <= EQ::invslot::GENERAL_END; i++) {
		item = GetInv().GetItem(i);
		if (item && item->GetItem()->BagType == EQ::item::BagTypeTradersSatchel) {
			for (int16 x = EQ::invbag::SLOT_BEGIN; x <= EQ::invbag::SLOT_END; x++) {
				// we already have the parent bag and a contents iterator..why not just iterate the bag!??
				slot_id = EQ::InventoryProfile::CalcSlotId(i, x);
				item    = GetInv().GetItem(slot_id);
				if (item) {
					if (item->GetSerialNumber() == SerialNumber) {
						return item;
					}
				}
			}
		}
	}

	LogTrading("Couldn't find item! Serial No. was [{}]", SerialNumber);

	return nullptr;
}


GetItems_Struct *Client::GetTraderItems()
{
	const EQ::ItemInstance *item   = nullptr;
	int16                  slot_id = INVALID_INDEX;
	auto                   gis     = new GetItems_Struct{0};
	uint8                  ndx     = 0;

	for (int16 i = EQ::invslot::GENERAL_BEGIN; i <= EQ::invslot::GENERAL_END; i++) {
		if (ndx >= GetInv().GetLookup()->InventoryTypeSize.Bazaar) {
			break;
		}
		item = GetInv().GetItem(i);
		if (item && item->GetItem()->BagType == EQ::item::BagTypeTradersSatchel) {
			for (int x = EQ::invbag::SLOT_BEGIN; x <= EQ::invbag::SLOT_END; x++) {
				if (ndx >= GetInv().GetLookup()->InventoryTypeSize.Bazaar) {
					break;
				}

				slot_id = EQ::InventoryProfile::CalcSlotId(i, x);
				item    = GetInv().GetItem(slot_id);

				if (item) {
					gis->items[ndx]         = item->GetID();
					gis->serial_number[ndx] = item->GetSerialNumber();
					gis->charges[ndx]       = item->GetCharges() == 0 ? 1 : item->GetCharges();
					ndx++;
				}
			}
		}
	}
	return gis;
}

uint16 Client::FindTraderItem(int32 SerialNumber, uint16 Quantity){

	const EQ::ItemInstance* item= nullptr;
	uint16 SlotID = 0;
	for (int i = EQ::invslot::GENERAL_BEGIN; i <= EQ::invslot::GENERAL_END; i++) {
		item = GetInv().GetItem(i);
		if (item && item->GetItem()->BagType == EQ::item::BagTypeTradersSatchel){
			for (int x = EQ::invbag::SLOT_BEGIN; x <= EQ::invbag::SLOT_END; x++){
				SlotID = EQ::InventoryProfile::CalcSlotId(i, x);

				item = GetInv().GetItem(SlotID);

				if (item && item->GetSerialNumber() == SerialNumber &&
					(item->GetCharges() >= Quantity || (item->GetCharges() <= 0 && Quantity == 1)))
				{
					return SlotID;
				}
			}
		}
	}
	LogTrading("Could NOT find a match for Item: [{}] with a quantity of: [{}] on Trader: [{}]\n",
					SerialNumber , Quantity, GetName());

	return 0;
}

void Client::NukeTraderItem(
	uint16 slot,
	int16 charges,
	int16 quantity,
	Client *customer,
	uint16 trader_slot,
	int32 serial_number,
	int32 item_id
)
{
	if (!customer) {
		return;
	}

	LogTrading("NukeTraderItem(Slot <green>[{}] Charges <green>[{}] Quantity <green>[{}]", slot, charges, quantity);
	if (quantity < charges) {
		customer->SendSingleTraderItem(CharacterID(), serial_number);
		m_inv.DeleteItem(slot, quantity);
	}
	else {
		auto outapp = new EQApplicationPacket(OP_TraderDelItem, sizeof(TraderDelItem_Struct));
		auto tdis   = (TraderDelItem_Struct *) outapp->pBuffer;

		tdis->unknown_000 = 0;
		tdis->trader_id   = customer->GetID();
		tdis->item_id     = serial_number;
		tdis->unknown_012 = 0;
		customer->QueuePacket(outapp);
		safe_delete(outapp);

		m_inv.DeleteItem(slot);
	}
	// This updates the trader. Removes it from his trading bags.
	//
	const EQ::ItemInstance *Inst = m_inv[slot];
	database.SaveInventory(CharacterID(), Inst, slot);

	EQApplicationPacket *outapp2;

	if (quantity < charges) {
		outapp2 = new EQApplicationPacket(OP_DeleteItem, sizeof(MoveItem_Struct));
	}
	else {
		outapp2 = new EQApplicationPacket(OP_MoveItem, sizeof(MoveItem_Struct));
	}

	auto mis = (MoveItem_Struct *) outapp2->pBuffer;
	mis->from_slot       = slot;
	mis->to_slot         = 0xFFFFFFFF;
	mis->number_in_stack = 0xFFFFFFFF;

	if (quantity >= charges) {
		quantity = 1;
	}

	for (int i = 0; i < quantity; i++) {
		QueuePacket(outapp2);
	}

	safe_delete(outapp2);
}

void Client::FindAndNukeTraderItem(int32 serial_number, int16 quantity, Client *customer, uint16 trader_slot)
{
	const EQ::ItemInstance *item     = nullptr;
	bool                   stackable = false;
	int16                  charges   = 0;
	uint16                 slot_id   = FindTraderItem(serial_number, quantity);

	if (slot_id > 0) {
		item = GetInv().GetItem(slot_id);
		if (!item) {
			LogTrading("Could not find Item: [{}] on Trader: [{}]", serial_number, quantity, GetName());
			return;
		}

		charges   = GetInv().GetItem(slot_id)->GetCharges();
		stackable = item->IsStackable();
		if (!stackable) {
			quantity = (charges > 0) ? charges : 1;
		}

		LogTrading("FindAndNuke <green>[{}] charges <green>[{}] quantity <green>[{}]",
				   item->GetItem()->Name,
				   charges,
				   quantity
		);

		if (charges <= quantity || (charges <= 0 && quantity == 1) || !stackable) {
			DeleteItemInInventory(slot_id, quantity);
			auto   trader_items = TraderRepository::GetWhere(database, fmt::format("`char_id` = '{}'", CharacterID()));
			uint32 item_limit   = trader_items.size() >= GetInv().GetLookup()->InventoryTypeSize.Bazaar ?
				GetInv().GetLookup()->InventoryTypeSize.Bazaar :
				trader_items.size();
			uint8  count        = 0;
			bool   test_slot    = true;

			std::vector<TraderRepository::Trader> delete_queue{};
			for (int i = 0; i < item_limit; i++) {
				if (test_slot && trader_items.at(i).item_sn == serial_number) {
					delete_queue.push_back(trader_items.at(i));
					NukeTraderItem(
						slot_id,
						charges,
						quantity,
						customer,
						trader_slot,
						trader_items.at(i).item_sn,
						trader_items.at(i).item_id
					);
					test_slot = false;
				}
				else if (trader_items.at(i).item_id > 0) {
					count++;
				}
			}

			TraderRepository::DeleteMany(database, delete_queue);
			if (count == 0) {
				TraderEndTrader();
			}

			return;
		}
		else {
			TraderRepository::UpdateQuantity(database, CharacterID(), item->GetSerialNumber(), charges - quantity);
			NukeTraderItem(slot_id, charges, quantity, customer, trader_slot, item->GetSerialNumber(), item->GetID());
			return;
		}
	}
	LogTrading("Could NOT find a match for Item: <red>[{}] with a quantity of: <red>[{}] on Trader: <red>[{}]\n",
			   serial_number,
			   quantity,
			   GetName()
	);
}

void Client::ReturnTraderReq(const EQApplicationPacket *app, int16 trader_item_charges, uint32 item_id)
{
	auto tbs    = (TraderBuy_Struct *) app->pBuffer;
	auto outapp = new EQApplicationPacket(OP_TraderBuy, sizeof(TraderBuy_Struct));
	auto outtbs = (TraderBuy_Struct *) outapp->pBuffer;

	memcpy(outtbs, tbs, app->size);
	if (ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		// Convert Serial Number back to Item ID for RoF+
		outtbs->item_id = item_id;
	}
	else {
		// RoF+ requires individual price, but older clients require total price
		outtbs->price = (tbs->price * static_cast<uint32>(trader_item_charges));
	}

	outtbs->quantity     = trader_item_charges;
	// This should probably be trader ID, not customer ID as it is below.
	outtbs->trader_id    = GetID();
	outtbs->already_sold = 0;

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::TradeRequestFailed(const EQApplicationPacket *app)
{
	auto tbs    = (TraderBuy_Struct *) app->pBuffer;
	auto outapp = new EQApplicationPacket(OP_TraderBuy, sizeof(TraderBuy_Struct));
	auto outtbs = (TraderBuy_Struct *) outapp->pBuffer;

	memcpy(outtbs, tbs, app->size);
	outtbs->already_sold = 0xFFFFFFFF;
	outtbs->trader_id    = 0xFFFFFFFF;

	QueuePacket(outapp);
	safe_delete(outapp);
}

static void BazaarAuditTrail(const char *seller, const char *buyer, const char *itemName, int quantity, int totalCost, int tranType) {

	const std::string& query = fmt::format(
		"INSERT INTO `trader_audit` "
        	"(`time`, `seller`, `buyer`, `itemname`, `quantity`, `totalcost`, `trantype`) "
		"VALUES (NOW(), '{}', '{}', '{}', {}, {}, {})",
		seller,
		buyer,
		Strings::Escape(itemName),
		quantity,
		totalCost,
		tranType
	);
	database.QueryDatabase(query);
}

void Client::BuyTraderItem(TraderBuy_Struct *tbs, Client *Trader, const EQApplicationPacket *app)
{
	if (!Trader) {
		return;
	}

	if (!Trader->IsTrader()) {
		TradeRequestFailed(app);
		return;
	}

	auto outapp     = std::make_unique<EQApplicationPacket>(OP_Trader, static_cast<uint32>(sizeof(TraderBuy_Struct)));
	auto outtbs     = (TraderBuy_Struct *) outapp->pBuffer;
	outtbs->item_id = tbs->item_id;

	const EQ::ItemInstance *buy_item = nullptr;
	uint32                  item_id  = 0;

	if (ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		tbs->item_id = Strings::ToUnsignedBigInt(tbs->serial_number);
	}

	buy_item = Trader->FindTraderItemBySerialNumber(tbs->item_id);

	if (!buy_item) {
		LogTrading("Unable to find item id <red>[{}] item_sn <red>[{}] on trader", tbs->item_id, tbs->serial_number);
		TradeRequestFailed(app);
		return;
	}

	LogTrading(
		"Name: <green>[{}] IsStackable: <green>[{}] Requested Quantity: <green>[{}] Charges on Item <green>[{}]",
		buy_item->GetItem()->Name,
		buy_item->IsStackable(),
		tbs->quantity,
		buy_item->GetCharges()
	);
	// If the item is not stackable, then we can only be buying one of them.
	if (!buy_item->IsStackable()) {
		outtbs->quantity = 1; // normally you can't send more than 1 here
	}
	else {
		// Stackable items, arrows, diamonds, etc
		int32 item_charges = buy_item->GetCharges();
		// ItemCharges for stackables should not be <= 0
		if (item_charges <= 0) {
			outtbs->quantity = 1;
			// If the purchaser requested more than is in the stack, just sell them how many are actually in the stack.
		}
		else if (static_cast<uint32>(item_charges) < tbs->quantity) {
			outtbs->quantity = item_charges;
		}
		else {
			outtbs->quantity = tbs->quantity;
		}
	}

	LogTrading("Actual quantity that will be traded is <green>[{}]", outtbs->quantity);

	if ((tbs->price * outtbs->quantity) <= 0) {
        Message(Chat::Red, "Internal error. Aborting trade. Please report this to the ServerOP. Error code is 1");
        Trader->Message(
            Chat::Red,
            "Internal error. Aborting trade. Please report this to the ServerOP. Error code is 1"
        );
        LogError(
            "Bazaar: Zero price transaction between <red>[{}] and <red>[{}] aborted. Item: <red>[{}] Charges: "
            "<red>[{}] Qty <red>[{}] Price: <red>[{}]",
            GetName(),
            Trader->GetName(),
            buy_item->GetItem()->Name,
            buy_item->GetCharges(),
            tbs->quantity,
            tbs->price
        );
        TradeRequestFailed(app);
        return;
    }

	uint64 total_transaction_value = static_cast<uint64>(tbs->price) * static_cast<uint64>(outtbs->quantity);

	if (total_transaction_value > MAX_TRANSACTION_VALUE) {
		Message(
			Chat::Red,
			"That would exceed the single transaction limit of %u platinum.",
			MAX_TRANSACTION_VALUE / 1000
		);
		TradeRequestFailed(app);
		return;
	}

	// This cannot overflow assuming MAX_TRANSACTION_VALUE, checked above, is the default of 2000000000
	uint32 total_cost = tbs->price * outtbs->quantity;

	if (Trader->ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		// RoF+ uses individual item price where older clients use total price
		outtbs->price = tbs->price;
	}
	else {
		outtbs->price = total_cost;
	}

	if (!TakeMoneyFromPP(total_cost)) {
        RecordPlayerEventLog(
            PlayerEvent::POSSIBLE_HACK,
            PlayerEvent::PossibleHackEvent{
                .message = "Attempted to buy something in bazaar but did not have enough money."
            }
        );
        TradeRequestFailed(app);
        return;
    }

	LogTrading("Customer Paid: <green>[{}] in Copper", total_cost);

	uint32 platinum = total_cost / 1000;
	total_cost     -= (platinum * 1000);
	uint32 gold     = total_cost / 100;
	total_cost     -= (gold * 100);
	uint32 silver   = total_cost / 10;
	total_cost     -= (silver * 10);
	uint32 copper   = total_cost;

	Trader->AddMoneyToPP(copper, silver, gold, platinum, true);

	if (buy_item && PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::TRADER_PURCHASE)) {
		auto e = PlayerEvent::TraderPurchaseEvent{
			.item_id              = buy_item->GetID(),
			.augment_1_id         = buy_item->GetAugmentItemID(0),
			.augment_2_id         = buy_item->GetAugmentItemID(1),
			.augment_3_id         = buy_item->GetAugmentItemID(2),
			.augment_4_id         = buy_item->GetAugmentItemID(3),
			.augment_5_id         = buy_item->GetAugmentItemID(4),
			.augment_6_id         = buy_item->GetAugmentItemID(5),
			.item_name            = buy_item->GetItem()->Name,
			.trader_id            = Trader->CharacterID(),
			.trader_name          = Trader->GetCleanName(),
			.price                = tbs->price,
			.quantity             = outtbs->quantity,
			.charges              = buy_item->GetCharges(),
			.total_cost           = (tbs->price * outtbs->quantity),
			.player_money_balance = GetCarriedMoney(),
		};

		RecordPlayerEventLog(PlayerEvent::TRADER_PURCHASE, e);
	}

	if (buy_item && PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::TRADER_SELL)) {
		auto e = PlayerEvent::TraderSellEvent{
			.item_id              = buy_item->GetID(),
			.augment_1_id         = buy_item->GetAugmentItemID(0),
			.augment_2_id         = buy_item->GetAugmentItemID(1),
			.augment_3_id         = buy_item->GetAugmentItemID(2),
			.augment_4_id         = buy_item->GetAugmentItemID(3),
			.augment_5_id         = buy_item->GetAugmentItemID(4),
			.augment_6_id         = buy_item->GetAugmentItemID(5),
			.item_name            = buy_item->GetItem()->Name,
			.buyer_id             = CharacterID(),
			.buyer_name           = GetCleanName(),
			.price                = tbs->price,
			.quantity             = outtbs->quantity,
			.charges              = buy_item->GetCharges(),
			.total_cost           = (tbs->price * outtbs->quantity),
			.player_money_balance = Trader->GetCarriedMoney(),
		};

		RecordPlayerEventLogWithClient(Trader, PlayerEvent::TRADER_SELL, e);
	}

	LogTrading("Trader Received: [{}] Platinum, [{}] Gold, [{}] Silver, [{}] Copper", platinum, gold, silver, copper);
    ReturnTraderReq(app, outtbs->quantity, item_id);

    outtbs->trader_id = GetID();
    outtbs->action    = BazaarBuyItem;
	strn0cpy(outtbs->seller_name, Trader->GetCleanName(), sizeof(outtbs->seller_name));
	strn0cpy(outtbs->buyer_name, GetCleanName(), sizeof(outtbs->buyer_name));
	strn0cpy(outtbs->item_name, buy_item->GetItem()->Name, sizeof(outtbs->item_name));
	strn0cpy(
		outtbs->serial_number,
		fmt::format("{:016}", buy_item->GetSerialNumber()).c_str(),
		sizeof(outtbs->serial_number)
	);

	TraderRepository::Trader t{};
	t.item_charges = buy_item->IsStackable() ? outtbs->quantity : buy_item->GetCharges();
	t.item_id      = buy_item->GetItem()->ID;
	t.aug_slot_1   = buy_item->GetAugmentItemID(0);
	t.aug_slot_2   = buy_item->GetAugmentItemID(1);
	t.aug_slot_3   = buy_item->GetAugmentItemID(2);
	t.aug_slot_4   = buy_item->GetAugmentItemID(3);
	t.aug_slot_5   = buy_item->GetAugmentItemID(4);
	t.aug_slot_6   = buy_item->GetAugmentItemID(5);
	t.char_id      = CharacterID();
	t.slot_id      = FindNextFreeParcelSlot(CharacterID());

	SendTraderItem(
		buy_item->GetItem()->ID,
		buy_item->IsStackable() ? outtbs->quantity : buy_item->GetCharges(),
		t
	);

	if (RuleB(Bazaar, AuditTrail)) {
		BazaarAuditTrail(Trader->GetName(), GetName(), buy_item->GetItem()->Name, outtbs->quantity, outtbs->price, 0);
	}

	Trader->FindAndNukeTraderItem(tbs->item_id, outtbs->quantity, this, 0);

	if (item_id > 0 && Trader->ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		// Convert Serial Number back to ItemID for RoF+
		outtbs->item_id = item_id;
	}

	Trader->QueuePacket(outapp.get());
}

void Client::SendBazaarWelcome()
{
	const auto          results = TraderRepository::GetWelcomeData(database);
	EQApplicationPacket outapp(OP_BazaarSearch, static_cast<uint32>(sizeof(BazaarWelcome_Struct)));
	auto                data = (BazaarWelcome_Struct *) outapp.pBuffer;

	data->action             = BazaarWelcome;
	data->traders            = results.count_of_traders;
	data->items              = results.count_of_items;

	QueuePacket(&outapp);
}

void Client::SendBarterWelcome()
{
	const auto results = BuyerBuyLinesRepository::GetWelcomeData(database);
	MessageString(Chat::White, BUYER_WELCOME, std::to_string(results.count_of_buyers).c_str());
}

void Client::DoBazaarSearch(BazaarSearchCriteria_Struct search_criteria)
{
	std::vector<BazaarSearchResultsFromDB_Struct> results = Bazaar::GetSearchResults(
		database,
		content_db,
		search_criteria,
		GetZoneID(),
		GetInstanceID()
	);
	if (results.empty()) {
		SendBazaarDone(GetID());
		return;
	}

	SetTraderTransactionDate();
	std::stringstream           ss{};
	cereal::BinaryOutputArchive ar(ss);
	ar(results);

	uint32 packet_size = ss.str().length() + sizeof(BazaarSearchMessaging_Struct);
	auto   out         = new EQApplicationPacket(OP_BazaarSearch, packet_size);
	auto   data        = (BazaarSearchMessaging_Struct *) out->pBuffer;

	data->action = BazaarSearch;
	memcpy(data->payload, ss.str().data(), ss.str().length());
	FastQueuePacket(&out);

	SendBazaarDone(GetID());
	SendBazaarDeliveryCosts();
}

static void UpdateTraderCustomerItemsAdded(
	uint32 customer_id,
	std::vector<BaseTraderRepository::Trader> trader_items,
	uint32 item_id,
	uint32 item_limit
)
{
	// Send Item packets to the customer to update the Merchant window with the
	// new items for sale, and give them a message in their chat window.
	auto customer = entity_list.GetClientByID(customer_id);
	if (!customer) {
		return;
	}

	const EQ::ItemData *item = database.GetItem(item_id);
	if (!item) {
		return;
	}

	customer->Message(Chat::Red, "The Trader has put up %s for sale.", item->Name);

	for (auto const &i: trader_items) {
		if (i.item_id == item_id) {
			std::unique_ptr<EQ::ItemInstance> inst(
				database.CreateItem(
					i.item_id,
					i.item_charges,
					i.aug_slot_1,
					i.aug_slot_2,
					i.aug_slot_3,
					i.aug_slot_4,
					i.aug_slot_5,
					i.aug_slot_6
				)
			);
			if (!inst) {
				return;
			}

			inst->SetCharges(i.item_charges);
			inst->SetPrice(i.item_cost);
			inst->SetSerialNumber(i.item_sn);
			inst->SetMerchantSlot(i.item_sn);
			if (inst->IsStackable()) {
				inst->SetMerchantCount(i.item_charges);
			}

			customer->SendItemPacket(EQ::invslot::slotCursor, inst.get(), ItemPacketMerchant); // MainCursor?
			LogTrading("Sending price update for [{}], Serial No. [{}] with [{}] charges",
					   item->Name, i.item_sn, i.item_charges);
		}
	}
}

static void UpdateTraderCustomerPriceChanged(
	uint32 customer_id,
	std::vector<BaseTraderRepository::Trader> trader_items,
	uint32 item_id,
	int32 charges,
	uint32 new_price,
	uint32 item_limit
)
{
	// Send ItemPackets to update the customer's Merchant window with the new price (or remove the item if
	// the new price is 0) and inform them with a chat message.
	auto customer = entity_list.GetClientByID(customer_id);

	if (!customer) {
		return;
	}

	const EQ::ItemData *item = database.GetItem(item_id);

	if (!item) {
		return;
	}

	if (new_price == 0) {
		// If the new price is 0, remove the item(s) from the window.
		auto outapp = new EQApplicationPacket(OP_TraderDelItem, sizeof(TraderDelItem_Struct));
		auto tdis   = (TraderDelItem_Struct *) outapp->pBuffer;

		tdis->unknown_000 = 0;
		tdis->trader_id   = customer->GetID();
		tdis->unknown_012 = 0;
		customer->Message(Chat::Red, "The Trader has withdrawn the %s from sale.", item->Name);

		for (int i = 0; i < item_limit; i++) {
			if (trader_items.at(i).item_id == item_id) {
				if (customer->ClientVersion() >= EQ::versions::ClientVersion::RoF) {
					// RoF+ use Item IDs for now
					tdis->item_id = trader_items.at(i).item_id;
				}
				else {
					tdis->item_id = trader_items.at(i).item_sn;
				}
				tdis->item_id = trader_items.at(i).item_sn;
				LogTrading("Telling customer to remove item [{}] with [{}] charges and S/N [{}]",
						   item_id, charges, trader_items.at(i).item_sn);

				customer->QueuePacket(outapp);
			}
		}

		safe_delete(outapp);
		return;
	}

	LogTrading("Sending price updates to customer [{}]", customer->GetName());

	auto it = std::find_if(trader_items.begin(), trader_items.end(), [&](TraderRepository::Trader x){ return x.item_id == item->ID;});
	std::unique_ptr<EQ::ItemInstance> inst(
		database.CreateItem(
			it->item_id,
			it->item_charges,
			it->aug_slot_1,
			it->aug_slot_2,
			it->aug_slot_3,
			it->aug_slot_4,
			it->aug_slot_5,
			it->aug_slot_6
		)
	);
	if (!inst) {
		return;
	}

	if (charges > 0) {
		inst->SetCharges(charges);
	}

	inst->SetPrice(new_price);
	if (inst->IsStackable()) {
		inst->SetMerchantCount(charges);
	}

	// Let the customer know the price in the window has suddenly just changed on them.
	customer->Message(Chat::Red, "The Trader has changed the price of %s.", item->Name);

	for (int i = 0; i < item_limit; i++) {
		if ((trader_items.at(i).item_id != item_id) ||
			((!item->Stackable) && (trader_items.at(i).item_charges != charges))) {
			continue;
		}

		inst->SetSerialNumber(trader_items.at(i).item_sn);
		inst->SetMerchantSlot(trader_items.at(i).item_sn);

		LogTrading("Sending price update for [{}], Serial No. [{}] with [{}] charges",
				   item->Name, trader_items.at(i).item_sn, trader_items.at(i).item_charges);

		customer->SendItemPacket(EQ::invslot::slotCursor, inst.get(), ItemPacketMerchant); // MainCursor??
	}
//	safe_delete(inst);
}

void Client::SendBuyerResults(BarterSearchRequest_Struct& bsr)
{
	if (ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		std::string search_string(bsr.search_string);
		BuyerLineSearch_Struct results{};

		SetBarterTime();

		if (bsr.search_scope == 1) {
			// Local Buyers
			results = BuyerBuyLinesRepository::SearchBuyLines(database, search_string, 0, GetZoneID(), GetInstanceID());
		}
		else if (bsr.buyer_id) {
			// Specific Buyer
			results = BuyerBuyLinesRepository::SearchBuyLines(database, search_string, bsr.buyer_id);
		} else {
			// All Buyers
			results = BuyerBuyLinesRepository::SearchBuyLines(database, search_string);
		}

		if (results.buy_line.empty()) {
			Message(Chat::White, "No buylines could be found.");
			return;
		}

		std::string buyer_name = "ID {} not in zone.";
		if (search_string.empty()) {
			search_string = "*";
		}

		results.search_string  = std::move(search_string);
		results.transaction_id = bsr.transaction_id;
		std::stringstream ss{};
		cereal::BinaryOutputArchive ar(ss);

		{ ar(results); }

		auto packet = std::make_unique<EQApplicationPacket>(
			OP_BuyerItems,
			static_cast<uint32>(ss.str().length()) + static_cast<uint32>(sizeof(BuyerGeneric_Struct))
		);
		auto emu    = (BuyerGeneric_Struct *) packet->pBuffer;

		emu->action = Barter_BuyerSearch;
		memcpy(emu->payload, ss.str().data(), ss.str().length());

		QueuePacket(packet.get());

		ss.str("");
		ss.clear();

	}
}

void Client::ShowBuyLines(const EQApplicationPacket *app)
{
	auto bir   = (BuyerInspectRequest_Struct *) app->pBuffer;
	auto buyer = entity_list.GetClientByID(bir->buyer_id);

	if (!buyer || buyer->GetCustomerID()) {
		bir->approval = 0; // Tell the client that the Buyer is unavailable
		QueuePacket(app);
		MessageString(Chat::Yellow, TRADER_BUSY);
		return;
	}

	if (ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		SetBarterTime();
		bir->approval = buyer->WithCustomer(GetID());
		QueuePacket(app);

		auto results  = BuyerBuyLinesRepository::GetBuyLines(database, buyer->CharacterID());
		auto greeting = BuyerRepository::GetWelcomeMessage(database, buyer->GetBuyerID());

		if (greeting.length() == 0) {
			greeting = "Welcome!";
		}

		MessageString(Chat::NPCQuestSay, BUYER_GREETING, buyer->GetName(), greeting.c_str());
		const std::string name(GetName());
		buyer->SendSellerBrowsing(name);

		std::stringstream           ss{};
		cereal::BinaryOutputArchive ar(ss);

		for (auto l : results) {
			const EQ::ItemData *item = database.GetItem(l.item_id);
			l.enabled     = 1;
			l.item_icon   = item->Icon;
			l.item_toggle = 1;

			{ ar(l); }

			auto packet = std::make_unique<EQApplicationPacket>(
				OP_BuyerItems,
				static_cast<uint32>(ss.str().length()) + static_cast<uint32>(sizeof(BuyerGeneric_Struct))
			);
			auto emu    = (BuyerGeneric_Struct *) packet->pBuffer;

			emu->action = Barter_BuyerInspectBegin;
			memcpy(emu->payload, ss.str().data(), ss.str().length());

			QueuePacket(packet.get());

			ss.str("");
			ss.clear();
		}

		return;
	}
}

void Client::SellToBuyer(const EQApplicationPacket *app)
{
	if (ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		BuyerLineSellItem_Struct     sell_line{};
		auto                         in = (BuyerGeneric_Struct *) app->pBuffer;
		EQ::Util::MemoryStreamReader ss_in(
			reinterpret_cast<char *>(in->payload),
			app->size - sizeof(BuyerGeneric_Struct));
		cereal::BinaryInputArchive   ar(ss_in);
		ar(sell_line);

		sell_line.seller_name = GetCleanName();

		switch (sell_line.purchase_method) {
			case BarterInBazaar:
			case BarterByVendor: {
				auto buyer = entity_list.GetClientByID(sell_line.buyer_entity_id);
				if (!buyer) {
					SendBarterBuyerClientMessage(
						sell_line,
						Barter_SellerTransactionComplete,
						Barter_Failure,
						Barter_Failure
					);
					break;
				}

				if (sell_line.purchase_method == BarterInBazaar && buyer->IsThereACustomer()) {
					auto customer = entity_list.GetClientByID(buyer->GetCustomerID());
					if (customer) {
						customer->CancelBuyerTradeWindow();
					}
				}

				if (!DoBarterBuyerChecks(sell_line)) {
					return;
				};

				if (!DoBarterSellerChecks(sell_line)) {
					return;
				};

				BuyerRepository::UpdateTransactionDate(database, sell_line.buyer_id, time(nullptr));

				if (!FindNumberOfFreeInventorySlotsWithSizeCheck(sell_line.trade_items)) {
					LogTradingDetail("Seller {} has insufficient inventory space for {} compensation items.",
									 GetCleanName(),
									 sell_line.trade_items.size()
					);
					Message(Chat::Red, "Insufficient inventory space for the compensation items.");
					SendBarterBuyerClientMessage(
						sell_line,
						Barter_SellerTransactionComplete,
						Barter_Failure,
						Barter_Failure
					);
					return;
				}

				for (auto const &ti: sell_line.trade_items) {
					std::unique_ptr<EQ::ItemInstance> inst(
						database.CreateItem(
							ti.item_id,
							ti.item_quantity *
							sell_line.seller_quantity
						)
					);

					if (inst.get()->GetItem()) {
						buyer->RemoveItem(ti.item_id, ti.item_quantity * sell_line.seller_quantity);
						if (!PutItemInInventoryWithStacking(inst.get())) {
							Message(Chat::Red, "Error putting item in your inventory.");
							buyer->PutItemInInventoryWithStacking(inst.get());
							SendBarterBuyerClientMessage(
								sell_line,
								Barter_SellerTransactionComplete,
								Barter_Failure,
								Barter_Failure
							);
							return;
						}
					}
				}

				std::unique_ptr<EQ::ItemInstance> buy_inst(
					database.CreateItem(
						sell_line.item_id,
						sell_line.seller_quantity
					)
				);
				RemoveItem(sell_line.item_id, sell_line.seller_quantity);
				if (buy_inst->IsStackable()) {
					if (!buyer->PutItemInInventoryWithStacking(buy_inst.get())) {
						buyer->Message(Chat::Red, "Error putting item in your inventory.");
						PutItemInInventoryWithStacking(buy_inst.get());
						SendBarterBuyerClientMessage(
							sell_line,
							Barter_SellerTransactionComplete,
							Barter_Failure,
							Barter_Failure
						);
						return;
					}
				}
				else {
					for (int i = 1; i <= sell_line.seller_quantity; i++) {
						buy_inst->SetCharges(1);
						if (!buyer->PutItemInInventoryWithStacking(buy_inst.get())) {
							buyer->Message(Chat::Red, "Error putting item in your inventory.");
							PutItemInInventoryWithStacking(buy_inst.get());
							SendBarterBuyerClientMessage(
							sell_line,
								Barter_SellerTransactionComplete,
								Barter_Failure,
								Barter_Failure
							);
							return;
						}
					}
				}

				uint64 total_cost = (uint64) sell_line.item_cost * (uint64) sell_line.seller_quantity;
				AddMoneyToPP(total_cost, false);
				buyer->TakeMoneyFromPP(total_cost, false);

				if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::BARTER_TRANSACTION)) {
					PlayerEvent::BarterTransaction e{};
					e.status        = "Successful Barter Transaction";
					e.item_id       = sell_line.item_id;
					e.item_quantity = sell_line.seller_quantity;
					e.item_name     = sell_line.item_name;
					e.trade_items   = sell_line.trade_items;
					for (auto &t: e.trade_items) {
						t *= sell_line.seller_quantity;
					}
					e.total_cost  = total_cost;
					e.buyer_name  = buyer->GetCleanName();
					e.seller_name = GetCleanName();
					RecordPlayerEventLog(PlayerEvent::BARTER_TRANSACTION, e);
				}

				SendWindowUpdatesToSellerAndBuyer(sell_line);
				SendBarterBuyerClientMessage(
					sell_line,
					Barter_SellerTransactionComplete,
					Barter_Success,
					Barter_Success
				);
				buyer->SendBarterBuyerClientMessage(
					sell_line,
					Barter_BuyerTransactionComplete,
					Barter_Success,
					Barter_Success
				);
				break;
			}
			case BarterOutsideBazaar: {
				bool seller_error = false;
				auto buyer_time   = BuyerRepository::GetTransactionDate(database, sell_line.buyer_id);

				if (buyer_time > GetBarterTime()) {
					SendBarterBuyerClientMessage(
						sell_line,
						Barter_SellerTransactionComplete,
						Barter_Failure,
						Barter_DataOutOfDate
					);
					return;
				}

				if (sell_line.trade_items.size() > 0) {
					Message(Chat::Red, "You must visit the buyer directly when receiving compensation items.");
					seller_error = true;
				}

				auto buy_item_slot_id = GetInv().HasItem(
					sell_line.item_id,
					sell_line.seller_quantity,
					invWherePersonal
				);
				auto buy_item = buy_item_slot_id == INVALID_INDEX ? nullptr : GetInv().GetItem(buy_item_slot_id);
				if (!buy_item) {
					SendBarterBuyerClientMessage(
						sell_line,
						Barter_SellerTransactionComplete,
						Barter_Failure,
						Barter_SellerDoesNotHaveItem
					);
					break;
				}

				if (seller_error) {
					LogTradingDetail("Seller Error <red>[{}]  Sell/Buy Transaction Failed.",
									 seller_error
					);
					SendBarterBuyerClientMessage(
						sell_line,
						Barter_SellerTransactionComplete,
						Barter_Failure,
						Barter_Failure
					);
					return;
				}

				BuyerRepository::UpdateTransactionDate(database, sell_line.buyer_id, time(nullptr));

				auto server_packet = std::make_unique<ServerPacket>(
					ServerOP_BuyerMessaging,
					static_cast<uint32>(sizeof(BuyerMessaging_Struct))
				);

				auto data = (BuyerMessaging_Struct *) server_packet->pBuffer;

				data->action           = Barter_SellItem;
				data->buyer_entity_id  = sell_line.buyer_entity_id;
				data->buyer_id         = sell_line.buyer_id;
				data->seller_entity_id = GetID();
				data->buy_item_id      = sell_line.item_id;
				data->buy_item_qty     = sell_line.item_quantity;
				data->buy_item_cost    = sell_line.item_cost;
				data->buy_item_icon    = sell_line.item_icon;
				data->zone_id          = GetZoneID();
				data->slot             = sell_line.slot;
				data->seller_quantity  = sell_line.seller_quantity;
				data->purchase_method  = sell_line.purchase_method;
				strn0cpy(data->item_name, sell_line.item_name, sizeof(data->item_name));
				strn0cpy(data->buyer_name, sell_line.buyer_name.c_str(), sizeof(data->buyer_name));
				strn0cpy(data->seller_name, GetCleanName(), sizeof(data->seller_name));

				worldserver.SendPacket(server_packet.get());

				break;
			}
		}
	}
}

void Client::SendBuyerPacket(Client* Buyer) {

	// This is the Buyer Appearance packet. This method is called for each Buyer when a Client connects to the zone.
	//
	auto outapp = new EQApplicationPacket(OP_Barter, 13 + strlen(GetName()));

	char* Buf = (char*)outapp->pBuffer;

	VARSTRUCT_ENCODE_TYPE(uint32, Buf, Barter_BuyerAppearance);
	VARSTRUCT_ENCODE_TYPE(uint32, Buf, Buyer->GetID());
	VARSTRUCT_ENCODE_TYPE(uint32, Buf, 0x01);
	VARSTRUCT_ENCODE_STRING(Buf, GetName());

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::ToggleBuyerMode(bool status)
{
	auto outapp = std::make_unique<EQApplicationPacket>(
		OP_Barter,
		static_cast<uint32>(sizeof(BuyerSetAppearance_Struct))
	);
	auto data   = (BuyerSetAppearance_Struct *) outapp->pBuffer;

	data->action    = Barter_BuyerAppearance;
	data->entity_id = GetID();

	if (status && IsInBuyerSpace()) {
		SetBuyerID(CharacterID());

		BuyerRepository::Buyer b{};
		b.id                    = 0;
		b.char_id               = GetBuyerID();
		b.char_entity_id        = GetID();
		b.char_zone_id          = GetZoneID();
		b.char_zone_instance_id = GetInstanceID();
		b.char_name             = GetCleanName();
		b.transaction_date      = time(nullptr);
		BuyerRepository::DeleteBuyer(database, GetBuyerID());
		BuyerRepository::InsertOne(database, b);

		data->status = BuyerBarter::On;
		SetCustomerID(0);
		SendBuyerMode(true);
		SendBuyerToBarterWindow(this, Barter_AddToBarterWindow);
		Message(Chat::Yellow, "Barter Mode ON.");
	}
	else {
		data->status = BuyerBarter::Off;
		BuyerRepository::DeleteBuyer(database, GetBuyerID());
		SetCustomerID(0);
		SendBuyerToBarterWindow(this, Barter_RemoveFromBarterWindow);
		SendBuyerMode(false);
		SetBuyerID(0);
		if (!IsInBuyerSpace()) {
			Message(Chat::Red, "You must be in a Barter Stall to start Barter Mode.");
		}
		Message(Chat::Yellow, fmt::format("Barter Mode OFF. Buy lines deactivated.").c_str());
	}

	entity_list.QueueClients(this, outapp.get(), false);
}

void Client::ModifyBuyLine(const EQApplicationPacket *app)
{
	if (ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		BuyerBuyLines_Struct         bl{};
		auto                         in = (BuyerGeneric_Struct *) app->pBuffer;
		EQ::Util::MemoryStreamReader ss_in(
			reinterpret_cast<char *>(in->payload),
			app->size - sizeof(BuyerGeneric_Struct)
		);
		cereal::BinaryInputArchive   ar(ss_in);
		ar(bl);

		if (bl.buy_lines.empty()) {
			return;
		}

		BuyerRepository::UpdateTransactionDate(database, GetBuyerID(), time(nullptr));
		int64 current_total_cost = 0;
		bool  pass               = false;

		auto current_buy_lines = BuyerBuyLinesRepository::GetBuyLines(database, CharacterID());

		std::map<uint32, BuylineItemDetails_Struct> item_map;
		BuildBuyLineMapFromVector(item_map, current_buy_lines);

		current_total_cost = ValidateBuyLineCost(item_map);

		auto buy_line = bl.buy_lines.front();
		auto it       = std::find_if(
			current_buy_lines.cbegin(),
			current_buy_lines.cend(),
			[&](BuyerLineItems_Struct bl) {
				return bl.slot == buy_line.slot;
			}
		);

		if (buy_line.item_toggle) {
			current_total_cost += buy_line.item_cost * buy_line.item_quantity;
			if (it != std::end(current_buy_lines)) {
				current_total_cost -= it->item_cost * it->item_quantity;
				if (current_total_cost > GetCarriedMoney()) {
					buy_line.item_cost     = it->item_cost;
					buy_line.item_quantity = it->item_quantity;
					Message(
						Chat::Red,
						fmt::format(
							"You currently do not have sufficient funds to support your buy lines. You have {} and need {}",
							DetermineMoneyString(GetCarriedMoney()),
							DetermineMoneyString(current_total_cost)).c_str()
					);
					SendBuyLineUpdate(buy_line);
					return;
				}
				else {
					RemoveItemFromBuyLineMap(item_map, *it);
					BuildBuyLineMapFromVector(item_map, bl.buy_lines);
				}
			}
			else {
				BuildBuyLineMapFromVector(item_map, bl.buy_lines);
			}
		}
		else {
			current_total_cost -= static_cast<int64>(buy_line.item_cost) * static_cast<int64>(buy_line.item_quantity);
			std::map<uint32, BuylineItemDetails_Struct> item_map_tmp;
			BuildBuyLineMapFromVector(item_map_tmp, bl.buy_lines);
			if (ValidateBuyLineItems(item_map_tmp)) {
				pass = true;
			}
		}

		if (current_total_cost > static_cast<int64>(GetCarriedMoney())) {
			Message(
				Chat::Red,
				fmt::format(
					"You currently do not have sufficient funds to support your buy lines. You have {} and need {}",
					DetermineMoneyString(GetCarriedMoney()),
					DetermineMoneyString(current_total_cost)).c_str()
			);
			buy_line.item_toggle = 0;
			SendBuyLineUpdate(buy_line);
			return;
		}

		bool buyer_error = false;

		if (!ValidateBuyLineItems(item_map)) {
			buy_line.item_toggle = 0;
		}

		buy_line.item_icon = database.GetItem(buy_line.item_id)->Icon;
		if ((buy_line.item_toggle && it != std::end(current_buy_lines)) || pass) {
			BuyerBuyLinesRepository::ModifyBuyLine(database, buy_line, GetBuyerID());
			Message(Chat::Yellow, fmt::format("Buy line for {} modified.", buy_line.item_name).c_str());
		}
		else if (buy_line.item_toggle && it == std::end(current_buy_lines)) {
			BuyerBuyLinesRepository::CreateBuyLine(database, buy_line, GetBuyerID());
			Message(Chat::Yellow, fmt::format("Buy line for {} enabled.", buy_line.item_name).c_str());
		}
		else if (!buy_line.item_toggle) {
			BuyerBuyLinesRepository::DeleteBuyLine(database, GetBuyerID(), buy_line.slot);
			Message(Chat::Yellow, fmt::format("Buy line for {} disabled.", buy_line.item_name).c_str());
		}
		else {
			BuyerBuyLinesRepository::DeleteBuyLine(database, GetBuyerID(), buy_line.slot);
			Message(
				Chat::Yellow,
				fmt::format("Unhandled modification.  Buy line for {} disabled.", buy_line.item_name).c_str());
		}

		SendBuyLineUpdate(buy_line);

		if (IsThereACustomer()) {
			auto customer = entity_list.GetClientByID(GetCustomerID());
			if (!customer) {
				return;
			}

			auto it = std::find_if(
				current_buy_lines.cbegin(),
				current_buy_lines.cend(),
				[&](BuyerLineItems_Struct bl) {
					return bl.slot == buy_line.slot;
				}
			);
			if (it == std::end(current_buy_lines) && !buy_line.item_toggle) {
				return;
			}

			std::stringstream           ss_customer{};
			cereal::BinaryOutputArchive ar_customer(ss_customer);

			BuyerLineItems_Struct blis{};
			blis.enabled       = buy_line.enabled;
			blis.item_cost     = buy_line.item_cost;
			blis.item_icon     = buy_line.item_icon;
			blis.item_id       = buy_line.item_id;
			blis.item_quantity = buy_line.item_quantity;
			blis.item_toggle   = buy_line.item_toggle;
			blis.slot          = buy_line.slot;
			blis.item_name     = buy_line.item_name;
			for (auto const &i: buy_line.trade_items) {
				BuyerLineTradeItems_Struct bltis{};
				bltis.item_icon     = i.item_icon;
				bltis.item_id       = i.item_id;
				bltis.item_quantity = i.item_quantity;
				bltis.item_name     = i.item_name;
				blis.trade_items.push_back(bltis);
			}

			{ ar_customer(blis); }

			auto packet = std::make_unique<EQApplicationPacket>(
				OP_BuyerItems,
				static_cast<uint32>(ss_customer.str().length()) + static_cast<uint32>(sizeof(BuyerGeneric_Struct))
			);
			auto emu    = (BuyerGeneric_Struct *) packet->pBuffer;

			emu->action = Barter_BuyerInspectBegin;
			memcpy(emu->payload, ss_customer.str().data(), ss_customer.str().length());

			customer->QueuePacket(packet.get());

			ss_customer.str("");
			ss_customer.clear();
		}
	}
	return;
}

void Client::BuyerItemSearch(const EQApplicationPacket *app)
{
	auto               bis   = (BuyerItemSearch_Struct *) app->pBuffer;
	const EQ::ItemData *item = 0;
	uint32             it    = 0;

	BuyerItemSearchResults_Struct bisr{};

	while ((item = database.IterateItems(&it)) && bisr.results.size() < RuleI(Bazaar, MaxBuyerInventorySearchResults)) {
		if (!item->NoDrop) {
			continue;
		}

		auto item_name_match = std::strstr(
			Strings::ToLower(item->Name).c_str(),
			Strings::ToLower(bis->search_string).c_str()
		);

		if (item_name_match) {
			BuyerItemSearchResultEntry_Struct bisre{};
			bisre.item_id   = item->ID;
			bisre.item_icon = item->Icon;
			strn0cpy(bisre.item_name, item->Name, sizeof(bisre.item_name));
			bisr.results.push_back(bisre);
		}
	}

	bisr.action       = Barter_BuyerSearchResults;
	bisr.result_count = bisr.results.size();

	std::stringstream           ss{};
	cereal::BinaryOutputArchive ar(ss);
	{ ar(bisr); }

	uint32 packet_size = sizeof(BuyerGeneric_Struct) + ss.str().length();
	auto   outapp      = std::make_unique<EQApplicationPacket>(OP_Barter, packet_size);
	auto   emu         = (BuyerGeneric_Struct *) outapp->pBuffer;

	emu->action = Barter_BuyerSearchResults;
	memcpy(emu->payload, ss.str().data(), ss.str().length());

	QueuePacket(outapp.get());

	ss.str("");
	ss.clear();
}

const std::string &Client::GetMailKeyFull() const
{
	return m_mail_key_full;
}

const std::string &Client::GetMailKey() const
{
	return m_mail_key;
}

void Client::SendBecomeTraderToWorld(Client *trader, BazaarTraderBarterActions action)
{
	auto outapp = new ServerPacket(ServerOP_TraderMessaging, sizeof(TraderMessaging_Struct));
	auto data   = (TraderMessaging_Struct *) outapp->pBuffer;

	data->action      = action;
	data->entity_id   = trader->GetID();
	data->trader_id   = trader->CharacterID();
	data->zone_id     = trader->GetZoneID();
	data->instance_id = trader->GetInstanceID();
	strn0cpy(data->trader_name, trader->GetName(), sizeof(data->trader_name));

	worldserver.SendPacket(outapp);
	safe_delete(outapp);
}

void Client::SendBecomeTrader(BazaarTraderBarterActions action, uint32 entity_id)
{
	if (entity_id <= 0) {
		return;
	}

	auto trader = entity_list.GetClientByID(entity_id);
	if (!trader) {
		return;
	}

	auto outapp = new EQApplicationPacket(OP_BecomeTrader, sizeof(BecomeTrader_Struct));
	auto data   = (BecomeTrader_Struct *) outapp->pBuffer;

	data->action           = action;
	data->entity_id        = trader->GetID();
	data->trader_id        = trader->CharacterID();
	data->zone_id          = trader->GetZoneID();
	data->zone_instance_id = trader->GetInstanceID();
	strn0cpy(data->trader_name, trader->GetCleanName(), sizeof(data->trader_name));

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendTraderMode(BazaarTraderBarterActions status)
{
	auto outapp = new EQApplicationPacket(OP_Trader, sizeof(Trader_ShowItems_Struct));
	auto data   = (Trader_ShowItems_Struct *) outapp->pBuffer;

	data->action    = status;
	data->entity_id = GetID();

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::TraderPriceUpdate(const EQApplicationPacket *app)
{
	// Handle price updates from the Trader and update a customer browsing our stuff if necessary
	// This method also handles removing items from sale and adding them back up whilst still in
	// Trader mode.
	//
	auto tpus = (TraderPriceUpdate_Struct *) app->pBuffer;

	LogTrading(
		"Received Price Update for <green>[{}] Item Serial No. <green>[{}] New Price <green>[{}]",
		GetName(),
		tpus->SerialNumber,
		tpus->NewPrice
	);

	// Pull the items this Trader currently has for sale from the trader table.
	//
	auto   trader_items = TraderRepository::GetWhere(database, fmt::format("`char_id` = '{}'", CharacterID()));
	uint32 item_limit   = trader_items.size() >= GetInv().GetLookup()->InventoryTypeSize.Bazaar
		? GetInv().GetLookup()->InventoryTypeSize.Bazaar
		: trader_items.size();

	// The client only sends a single update with the Serial Number of the item whose price has been updated.
	// We must update the price for all the Trader's items that are identical to that one item, i.e.
	// if it is a stackable item like arrows, update the price for all stacks. If it is not stackable, then
	// update the prices for all items that have the same number of charges.
	//
	uint32 id_of_item_to_update      = 0;
	int32  charges_on_item_to_update = 0;
	uint32 old_price                 = 0;

	for (int i = 0; i < item_limit; i++) {
		if ((trader_items.at(i).item_id > 0) && (trader_items.at(i).item_sn == tpus->SerialNumber)) {
			// We found the item that the Trader wants to change the price of (or add back up for sale).
			//
			id_of_item_to_update      = trader_items.at(i).item_id;
			charges_on_item_to_update = trader_items.at(i).item_charges;
			old_price                 = trader_items.at(i).item_cost;

			LogTrading(
				"ItemID is <green>[{}] Charges is <green>[{}]",
				trader_items.at(i).item_id,
				trader_items.at(i).item_charges
			);
			break;
		}
	}

	if (id_of_item_to_update == 0) {
		// If the item is not currently in the trader table for this Trader, then they must have removed it from sale while
		// still in Trader mode. Check if the item is in their Trader Satchels, and if so, put it back up.
		// Quick Sanity check. If the item is not currently up for sale, and the new price is zero, just ack the packet
		// and do nothing.
		if (tpus->NewPrice == 0) {
			tpus->SubAction = BazaarPriceChange_RemoveItem;
			QueuePacket(app);
			return;
		}

		LogTrading("Unable to find item to update price for. Rechecking trader satchels");

		// Find what is in their Trader Satchels
		auto   newgis                 = GetTraderItems();
		uint32 id_of_item_to_add      = 0;
		int32  charges_on_item_to_add = 0;

		for (int i = 0; i < GetInv().GetLookup()->InventoryTypeSize.Bazaar; i++) {
			if ((newgis->items[i] > 0) && (newgis->serial_number[i] == tpus->SerialNumber)) {
				id_of_item_to_add      = newgis->items[i];
				charges_on_item_to_add = newgis->charges[i];

				LogTrading(
					"Found new Item to Add, ItemID is <green>[{}] Charges is <green>[{}]",
					newgis->items[i],
					newgis->charges[i]
				);
				break;
			}
		}

		const EQ::ItemData *item = nullptr;
		if (id_of_item_to_add) {
			item = database.GetItem(id_of_item_to_add);
		}

		if (!id_of_item_to_add || !item) {
			tpus->SubAction = BazaarPriceChange_Fail;
			QueuePacket(app);
			TraderEndTrader();
			safe_delete(newgis);

			LogTrading("Item not found in Trader Satchels either");
			return;
		}

		// It is a limitation of the client that if you have multiple of the same item, but with different charges,
		// although you can set different prices for them before entering Trader mode. If you Remove them and then
		// add them back whilst still in Trader mode, they all go up for the same price. We check for this situation
		// and give the Trader a warning message.
		//
		if (!item->Stackable) {
			bool same_item_with_differing_charges = false;

			for (int i = 0; i < GetInv().GetLookup()->InventoryTypeSize.Bazaar; i++) {
				if ((newgis->items[i] == id_of_item_to_add) && (newgis->charges[i] != charges_on_item_to_add)) {
					same_item_with_differing_charges = true;
					break;
				}
			}

			if (same_item_with_differing_charges) {
				Message(
					Chat::Red,
					"Warning: You have more than one %s with different charges. They have all been added for sale "
					"at the same price.",
					item->Name
				);
			}
		}

		// Now put all Items with a matching ItemID up for trade.
		//
		for (int i = 0; i < GetInv().GetLookup()->InventoryTypeSize.Bazaar; i++) {
			if (newgis->items[i] == id_of_item_to_add) {
				auto item_detail = FindTraderItemBySerialNumber(newgis->serial_number[i]);

				TraderRepository::Trader trader_item{};
				trader_item.id                    = 0;
				trader_item.char_entity_id        = GetID();
				trader_item.char_id               = CharacterID();
				trader_item.char_zone_id          = GetZoneID();
				trader_item.char_zone_instance_id = GetInstanceID();
				trader_item.item_charges          = newgis->charges[i];
				trader_item.item_cost             = tpus->NewPrice;
				trader_item.item_id               = newgis->items[i];
				trader_item.item_sn               = newgis->serial_number[i];
				trader_item.listing_date          = time(nullptr);
				if (item_detail->IsAugmented()) {
					auto augs              = item_detail->GetAugmentIDs();
					trader_item.aug_slot_1 = augs.at(0);
					trader_item.aug_slot_2 = augs.at(1);
					trader_item.aug_slot_3 = augs.at(2);
					trader_item.aug_slot_4 = augs.at(3);
					trader_item.aug_slot_5 = augs.at(4);
					trader_item.aug_slot_6 = augs.at(5);
				}
				trader_item.slot_id = i;

				TraderRepository::ReplaceOne(database, trader_item);

				trader_items.push_back(trader_item);

				LogTrading(
					"Adding new item for <green>[{}] ItemID <green>[{}] SerialNumber <green>[{}] Charges <green>[{}] "
					"Price: <green>[{}] Slot <green>[{}]",
					GetName(),
					newgis->items[i],
					newgis->serial_number[i],
					newgis->charges[i],
					tpus->NewPrice,
					i
				);
			}
		}

		// If we have a customer currently browsing, update them with the new items.
		//
		if (GetCustomerID()) {
			UpdateTraderCustomerItemsAdded(
				GetCustomerID(),
				trader_items,
				id_of_item_to_add,
				GetInv().GetLookup()->InventoryTypeSize.Bazaar
			);
		}

		safe_delete(newgis);

		// Acknowledge to the client.
		tpus->SubAction = BazaarPriceChange_AddItem;
		QueuePacket(app);

		return;
	}

	// This is a safeguard against a Trader increasing the price of an item while a customer is browsing and
	// unwittingly buying it at a higher price than they were expecting to.
	//
	if ((old_price != 0) && (tpus->NewPrice > old_price) && GetCustomerID()) {
		tpus->SubAction = BazaarPriceChange_Fail;
		QueuePacket(app);
		TraderEndTrader();
		Message(
			Chat::Red,
			"You must remove the item from sale before you can increase the price while a customer is browsing."
		);
		Message(Chat::Red, "Click 'Begin Trader' to restart Trader mode with the increased price for this item.");
		return;
	}

	// Send Acknowledgement back to the client.
	if (old_price == 0) {
		tpus->SubAction = BazaarPriceChange_AddItem;
	}
	else if (tpus->NewPrice != 0) {
		tpus->SubAction = BazaarPriceChange_UpdatePrice;
	}
	else {
		tpus->SubAction = BazaarPriceChange_RemoveItem;
	}

	QueuePacket(app);

	if (old_price == tpus->NewPrice) {
		LogTrading("The new price is the same as the old one");
		return;
	}
	// Update the price for all items we have for sale that have this ItemID and number of charges, or remove
	// them from the trader table if the new price is zero.
	//
	database.UpdateTraderItemPrice(CharacterID(), id_of_item_to_update, charges_on_item_to_update, tpus->NewPrice);

	// If a customer is browsing our goods, send them the updated prices / remove the items from the Merchant window
	if (GetCustomerID()) {
		UpdateTraderCustomerPriceChanged(
			GetCustomerID(),
			trader_items,
			id_of_item_to_update,
			charges_on_item_to_update,
			tpus->NewPrice,
			item_limit
		);
	}
}

void Client::SendBazaarDone(uint32 trader_id)
{
	auto outapp2 = new EQApplicationPacket(OP_BazaarSearch, sizeof(BazaarReturnDone_Struct));
	auto brds    = (BazaarReturnDone_Struct *) outapp2->pBuffer;

	brds->TraderID   = trader_id;
	brds->Type       = BazaarSearchDone;
	brds->Unknown008 = 0xFFFFFFFF;
	brds->Unknown012 = 0xFFFFFFFF;
	brds->Unknown016 = 0xFFFFFFFF;

	QueuePacket(outapp2);
	safe_delete(outapp2);
}

void Client::SendBulkBazaarTraders()
{
	if (ClientVersion() < EQ::versions::ClientVersion::RoF2) {
		return;
	}

	TraderRepository::BulkTraders_Struct results{};

	if (RuleB(Bazaar, UseAlternateBazaarSearch))
	{
		if (GetZoneID() == Zones::BAZAAR) {
			results = TraderRepository::GetDistinctTraders(database, GetInstanceID());
		}

		uint32 number = 1;
		auto   shards = CharacterDataRepository::GetInstanceZonePlayerCounts(database, Zones::BAZAAR);
		for (auto const &shard: shards) {
			if (GetZoneID() != Zones::BAZAAR || (GetZoneID() == Zones::BAZAAR && GetInstanceID() != shard.instance_id)) {

				TraderRepository::DistinctTraders_Struct t{};
				t.entity_id        = 0;
				t.trader_id        = TraderRepository::TRADER_CONVERT_ID + shard.instance_id;
				t.trader_name      = fmt::format("Bazaar Shard {}", number);
				t.zone_id          = Zones::BAZAAR;
				t.zone_instance_id = shard.instance_id;
				results.count += 1;
				results.name_length += t.trader_name.length() + 1;
				results.traders.push_back(t);
			}

			number++;
		}
	}
	else {
		results = TraderRepository::GetDistinctTraders(
			database,
			GetInstanceID(),
			EQ::constants::StaticLookup(ClientVersion())->BazaarTraderLimit
		);
	}

	SetTraderCount(results.count);

	auto  p_size  = 4 + 12 * results.count + results.name_length;
	auto  buffer  = std::make_unique<char[]>(p_size);
	memset(buffer.get(), 0, p_size);
	char *bufptr  = buffer.get();

	VARSTRUCT_ENCODE_TYPE(uint32, bufptr, results.count);

	for (auto t : results.traders) {
		VARSTRUCT_ENCODE_TYPE(uint16, bufptr, t.zone_id);
		VARSTRUCT_ENCODE_TYPE(uint16, bufptr, t.zone_instance_id);
		VARSTRUCT_ENCODE_TYPE(uint32, bufptr, t.trader_id);
		VARSTRUCT_ENCODE_TYPE(uint32, bufptr, t.entity_id);
		VARSTRUCT_ENCODE_STRING(bufptr, t.trader_name.c_str());
	}

	auto outapp = std::make_unique<EQApplicationPacket>(OP_TraderBulkSend, p_size);
	memcpy(outapp->pBuffer, buffer.get(), p_size);

	QueuePacket(outapp.get());
}

void Client::DoBazaarInspect(BazaarInspect_Struct &in)
{
	if (RuleB(Bazaar, UseAlternateBazaarSearch)) {
		if (in.trader_id >= TraderRepository::TRADER_CONVERT_ID) {
			auto trader = TraderRepository::GetTraderByInstanceAndSerialnumber(
				database,
				in.trader_id - TraderRepository::TRADER_CONVERT_ID,
				fmt::format("{}", in.serial_number).c_str()
			);

			if (!trader.trader_id) {
				LogTrading("Unable to convert trader id for {} and serial number {}.  Trader Buy aborted.",
					in.trader_id - TraderRepository::TRADER_CONVERT_ID,
					in.serial_number
				);
				return;
			}

			in.trader_id = trader.trader_id;
		}
	}

	auto items = TraderRepository::GetWhere(
		database, fmt::format("`char_id` = '{}' AND `item_sn` = '{}'", in.trader_id, in.serial_number)
	);

	if (items.empty()) {
		LogInfo("Failed to find item with serial number [{}]", in.serial_number);
		return;
	}

	auto &item = items.front();

	std::unique_ptr<EQ::ItemInstance> inst(
		database.CreateItem(
			item.item_id,
			item.item_charges,
			item.aug_slot_1,
			item.aug_slot_2,
			item.aug_slot_3,
			item.aug_slot_4,
			item.aug_slot_5,
			item.aug_slot_6
		)
	);

	if (inst) {
		SendItemPacket(0, inst.get(), ItemPacketViewLink);
	}
}

void Client::SendBazaarDeliveryCosts()
{
	auto outapp = std::make_unique<EQApplicationPacket>(
		OP_BazaarSearch,
		static_cast<uint32>(sizeof(BazaarDeliveryCost_Struct))
	);
	auto data   = (BazaarDeliveryCost_Struct *) outapp->pBuffer;

	data->action                = DeliveryCostUpdate;
	data->voucher_delivery_cost = RuleI(Bazaar, VoucherDeliveryCost);
	data->parcel_deliver_cost   = RuleR(Bazaar, ParcelDeliveryCostMod);

	QueuePacket(outapp.get());
}

std::string Client::DetermineMoneyString(uint64 cp)
{
	uint32 plat   = cp / 1000;
	uint32 gold   = (cp - plat * 1000) / 100;
	uint32 silver = (cp - plat * 1000 - gold * 100) / 10;
	uint32 copper = (cp - plat * 1000 - gold * 100 - silver * 10);

	if (!plat && !gold && !silver && !copper) {
		return std::string("No Money");
	}

	std::string money {};
	if (plat) {
		money += fmt::format("{}p ", plat);
	}
	if (gold) {
		money += fmt::format("{}g ", gold);
	}
	if (silver) {
		money += fmt::format("{}s ", silver);
	}
	if (copper) {
		money += fmt::format("{}c", copper);
	}

	return fmt::format("{}", money);
}

void Client::BuyTraderItemOutsideBazaar(TraderBuy_Struct *tbs, const EQApplicationPacket *app)
{
	auto in          = (TraderBuy_Struct *) app->pBuffer;
	auto trader_item = TraderRepository::GetItemBySerialNumber(database, tbs->serial_number, tbs->trader_id);
	if (!trader_item.id || GetTraderTransactionDate() < trader_item.listing_date) {
		LogTrading("Attempt to purchase an item outside of the Bazaar trader_id <red>[{}] item serial_number "
				   "<red>[{}] The Traders data was outdated.",
				   tbs->trader_id,
				   tbs->serial_number
		);
		in->method     = BazaarByParcel;
		in->sub_action = DataOutDated;
		TradeRequestFailed(app);
		return;
	}

	if (trader_item.active_transaction) {
		LogTrading("Attempt to purchase an item outside of the Bazaar trader_id <red>[{}] item serial_number "
				   "<red>[{}] The item is already within an active transaction.",
				   tbs->trader_id,
				   tbs->serial_number
		);
		in->method     = BazaarByParcel;
		in->sub_action = DataOutDated;
		TradeRequestFailed(app);
		return;
	}

	TraderRepository::UpdateActiveTransaction(database, trader_item.id, true);

	std::unique_ptr<EQ::ItemInstance> buy_item(
		database.CreateItem(
			trader_item.item_id,
			trader_item.item_charges,
			trader_item.aug_slot_1,
			trader_item.aug_slot_2,
			trader_item.aug_slot_3,
			trader_item.aug_slot_4,
			trader_item.aug_slot_5,
			trader_item.aug_slot_6
		)
	);

	if (!buy_item) {
		LogTrading("Unable to find item id <red>[{}] item_sn <red>[{}] on trader",
				   trader_item.item_id,
				   trader_item.item_sn
		);
		in->method     = BazaarByParcel;
		in->sub_action = Failed;
		TraderRepository::UpdateActiveTransaction(database, trader_item.id, false);
		TradeRequestFailed(app);
		return;
	}

	auto next_slot = FindNextFreeParcelSlot(CharacterID());
	if (next_slot == INVALID_INDEX) {
		LogTrading(
			"{} attempted to purchase {} from the bazaar with parcel delivery.  Unfortunately their parcel limit was reached.  "
			"Purchase unsuccessful.",
			GetCleanName(),
			buy_item->GetItem()->Name
		);
		in->method     = BazaarByParcel;
		in->sub_action = TooManyParcels;
		TraderRepository::UpdateActiveTransaction(database, trader_item.id, false);
		TradeRequestFailed(app);
		return;
	}

	LogTrading(
		"Name: <green>[{}] IsStackable: <green>[{}] Requested Quantity: <green>[{}] Charges on Item <green>[{}]",
		buy_item->GetItem()->Name,
		buy_item->IsStackable(),
		tbs->quantity,
		buy_item->GetCharges()
	);

	// Determine the actual quantity for the purchase
	int32 charges = static_cast<int32>(tbs->quantity);
	if (!buy_item->IsStackable()) {
		if (buy_item->GetCharges() <= 0) {
			charges = 1;
		}
		else {
			charges = buy_item->GetCharges();
		}
	} else {
		if (charges <= 0) {
			LogTrading("Rejecting purchase with zero/negative quantity [{}] for stackable item [{}]",
				charges, buy_item->GetItem()->Name);
			in->method     = BazaarByParcel;
			in->sub_action = Failed;
			TraderRepository::UpdateActiveTransaction(database, trader_item.id, false);
			TradeRequestFailed(app);
			return;
		}
		if (charges > trader_item.item_charges) {
			charges = trader_item.item_charges;
		}
		tbs->quantity = static_cast<uint32>(charges);
	}

	LogTrading(
		"Actual quantity that will be traded is <green>[{}] {}",
		tbs->quantity,
		buy_item->GetCharges() ? fmt::format("with {} charges", buy_item->GetCharges()) : ""
	);

	uint64 total_cost = static_cast<uint64>(tbs->price) * static_cast<uint64>(tbs->quantity);
	if (total_cost > MAX_TRANSACTION_VALUE) {
		Message(
			Chat::Red,
			"That would exceed the single transaction limit of %u platinum.",
			MAX_TRANSACTION_VALUE / 1000
		);
		TraderRepository::UpdateActiveTransaction(database, trader_item.id, false);
		TradeRequestFailed(app);
		return;
	}

	uint64 fee         = std::round(total_cost * RuleR(Bazaar, ParcelDeliveryCostMod));
	if (!TakeMoneyFromPP(total_cost + fee, false)) {
		RecordPlayerEventLog(
			PlayerEvent::POSSIBLE_HACK,
			PlayerEvent::PossibleHackEvent{
				.message = fmt::format(
					"{} attempted to buy {} in bazaar but did not have enough money.",
					GetCleanName(),
					buy_item->GetItem()->Name
				)
			}
		);
		in->method     = BazaarByParcel;
		in->sub_action = InsufficientFunds;
		TraderRepository::UpdateActiveTransaction(database, trader_item.id, false);
		TradeRequestFailed(app);
		return;
	}

	Message(Chat::Red, fmt::format("You paid {} for the parcel delivery.", DetermineMoneyString(fee)).c_str());
	LogTrading("Customer <green>[{}] Paid: <green>[{}] in Copper", CharacterID(), total_cost);

	if (buy_item && PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::TRADER_PURCHASE)) {
		auto e = PlayerEvent::TraderPurchaseEvent{
			.item_id              = buy_item->GetID(),
			.augment_1_id         = buy_item->GetAugmentItemID(0),
			.augment_2_id         = buy_item->GetAugmentItemID(1),
			.augment_3_id         = buy_item->GetAugmentItemID(2),
			.augment_4_id         = buy_item->GetAugmentItemID(3),
			.augment_5_id         = buy_item->GetAugmentItemID(4),
			.augment_6_id         = buy_item->GetAugmentItemID(5),
			.item_name            = buy_item->GetItem()->Name,
			.trader_id            = tbs->trader_id,
			.trader_name          = tbs->seller_name,
			.price                = tbs->price,
			.quantity             = tbs->quantity,
			.charges              = buy_item->IsStackable() ? 1 : charges,
			.total_cost           = total_cost,
			.player_money_balance = GetCarriedMoney(),
		};

		RecordPlayerEventLog(PlayerEvent::TRADER_PURCHASE, e);
	}

	CharacterParcelsRepository::CharacterParcels parcel_out{};
	parcel_out.from_name  = tbs->seller_name;
	parcel_out.note       = "Delivered from a Bazaar Purchase";
	parcel_out.sent_date  = time(nullptr);
	parcel_out.quantity   = charges;
	parcel_out.item_id    = buy_item->GetItem()->ID;
	parcel_out.aug_slot_1 = buy_item->GetAugmentItemID(0);
	parcel_out.aug_slot_2 = buy_item->GetAugmentItemID(1);
	parcel_out.aug_slot_3 = buy_item->GetAugmentItemID(2);
	parcel_out.aug_slot_4 = buy_item->GetAugmentItemID(3);
	parcel_out.aug_slot_5 = buy_item->GetAugmentItemID(4);
	parcel_out.aug_slot_6 = buy_item->GetAugmentItemID(5);
	parcel_out.char_id    = CharacterID();
	parcel_out.slot_id    = next_slot;
	parcel_out.id         = 0;

	auto result = CharacterParcelsRepository::InsertOne(database, parcel_out);
	if (!result.id) {
		LogError("Failed to add parcel to database.  From {} to {} item {} quantity {}",
				 parcel_out.from_name,
				 GetCleanName(),
				 parcel_out.item_id,
				 parcel_out.quantity
		);
		Message(Chat::Yellow, "Unable to save parcel to the database. Please contact an administrator.");
		in->method     = BazaarByParcel;
		in->sub_action = Failed;
		TraderRepository::UpdateActiveTransaction(database, trader_item.id, false);
		TradeRequestFailed(app);
		return;
	}

	ReturnTraderReq(app, tbs->quantity, buy_item->GetID());
	if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::PARCEL_SEND)) {
		PlayerEvent::ParcelSend e{};
		e.from_player_name = parcel_out.from_name;
		e.to_player_name   = GetCleanName();
		e.item_id          = parcel_out.item_id;
		e.augment_1_id     = parcel_out.aug_slot_1;
		e.augment_2_id     = parcel_out.aug_slot_2;
		e.augment_3_id     = parcel_out.aug_slot_3;
		e.augment_4_id     = parcel_out.aug_slot_4;
		e.augment_5_id     = parcel_out.aug_slot_5;
		e.augment_6_id     = parcel_out.aug_slot_6;
		e.quantity         = tbs->quantity;
		e.charges          = buy_item->IsStackable() ? 1 : charges;
		e.sent_date        = parcel_out.sent_date;

		RecordPlayerEventLog(PlayerEvent::PARCEL_SEND, e);
	}

	Parcel_Struct ps{};
	ps.item_slot = parcel_out.slot_id;
	strn0cpy(ps.send_to, GetCleanName(), sizeof(ps.send_to));

	if (trader_item.item_charges <= static_cast<int32>(tbs->quantity) || !buy_item->IsStackable()) {
		TraderRepository::DeleteOne(database, trader_item.id);
	} else {
		TraderRepository::UpdateQuantity(
			database,
			trader_item.char_id,
			trader_item.item_sn,
			trader_item.item_charges - tbs->quantity
		);
	}

	// AoTv4 shop payout: DO NOT pay the seller here. The native ServerOP_BazaarPurchase below routes to
	// the seller (any zone) and pays them via trader_pc->AddMoneyToPP -- paying here too would DOUBLE it.
	// The only gap is an OFFLINE seller (world can't route to them): that case is banked to
	// shop_escrow_<char_id> in the WORLD handler (world/zoneserver.cpp), paid out on next login.

	SendParcelDeliveryToWorld(ps);

	if (RuleB(Bazaar, AuditTrail)) {
		BazaarAuditTrail(tbs->seller_name, GetName(), buy_item->GetItem()->Name, tbs->quantity, tbs->price, 0);
	}

	auto out_server = std::make_unique<ServerPacket>(
		ServerOP_BazaarPurchase, static_cast<uint32>(sizeof(BazaarPurchaseMessaging_Struct))
	);
	auto out_data   = (BazaarPurchaseMessaging_Struct *) out_server->pBuffer;

	out_data->trader_buy_struct       = *tbs;
	out_data->buyer_id                = CharacterID();
	out_data->item_aug_1              = buy_item->GetAugmentItemID(0);
	out_data->item_aug_2              = buy_item->GetAugmentItemID(1);
	out_data->item_aug_3              = buy_item->GetAugmentItemID(2);
	out_data->item_aug_4              = buy_item->GetAugmentItemID(3);
	out_data->item_aug_5              = buy_item->GetAugmentItemID(4);
	out_data->item_aug_6              = buy_item->GetAugmentItemID(5);
	out_data->item_quantity_available = trader_item.item_charges;
	out_data->id                      = trader_item.id;
	strn0cpy(out_data->trader_buy_struct.buyer_name, GetCleanName(), sizeof(out_data->trader_buy_struct.buyer_name));

	worldserver.SendPacket(out_server.get());

	SendMoneyUpdate();
}

void Client::SetBuyerWelcomeMessage(const char *welcome_message)
{
	BuyerRepository::UpdateWelcomeMessage(database, CharacterID(), welcome_message);
}

void Client::SendBuyerGreeting(uint32 buyer_id)
{
	auto buyer = BuyerRepository::GetWhere(database, fmt::format("`char_id` = '{}'", buyer_id));
	if (buyer.empty()) {
		Message(Chat::White, "Welcome!");
		return;
	}
	Message(Chat::White, buyer.front().welcome_message.c_str());
}

void Client::SendSellerBrowsing(const std::string &browser)
{
	auto outapp = std::make_unique<EQApplicationPacket>(OP_Barter, static_cast<uint32>(sizeof(BuyerBrowsing_Struct)));
	auto eq     = (BuyerBrowsing_Struct *) outapp->pBuffer;

	eq->action = Barter_SellerBrowsing;
	strn0cpy(eq->char_name, browser.c_str(), sizeof(eq->char_name));

	QueuePacket(outapp.get());
}

void Client::SendBuyerMode(bool status)
{
	auto outapp = std::make_unique<EQApplicationPacket>(OP_Barter, 4);
	auto emu    = (BuyerGeneric_Struct *) outapp->pBuffer;

	emu->action = status ? Barter_BuyerModeOn : Barter_BuyerModeOff;

	QueuePacket(outapp.get());
}

bool Client::IsInBuyerSpace()
{
#define BUYER_DOOR_ARC_RADIUS_HIGH    91
#define BUYER_DOOR_ARC_RADIUS_LOW     71
#define BUYER_DOOR_OPEN_TYPE         155
#define TRADER_DOOR_OPEN_TYPE        153

	struct BuyerDoorDataStruct {
		uint32 door_id;
		uint32 arc_offset;
	};

	std::vector<BuyerDoorDataStruct> buyer_door_data = {
		{.door_id = 2}, {.arc_offset = 90},{.door_id = 3} ,{.arc_offset = 0} ,{.door_id = 4},  {.arc_offset = 0},
		{.door_id = 5}, {.arc_offset = 0} ,{.door_id = 6} ,{.arc_offset = 90},{.door_id = 7},  {.arc_offset = 0},
		{.door_id = 8}, {.arc_offset = 0} ,{.door_id = 9} ,{.arc_offset = 0} ,{.door_id = 10}, {.arc_offset = 0},
		{.door_id = 11},{.arc_offset = 0} ,{.door_id = 12},{.arc_offset = 0} ,{.door_id = 13}, {.arc_offset = 0},
		{.door_id = 14},{.arc_offset = 0} ,{.door_id = 15},{.arc_offset = 0} ,{.door_id = 16}, {.arc_offset = 90},
		{.door_id = 17},{.arc_offset = 0} ,{.door_id = 18},{.arc_offset = 0} ,{.door_id = 19}, {.arc_offset = 0},
		{.door_id = 20},{.arc_offset = 0} ,{.door_id = 21},{.arc_offset = 0} ,{.door_id = 22}, {.arc_offset = 0},
		{.door_id = 23},{.arc_offset = 0} ,{.door_id = 24},{.arc_offset = 0} ,{.door_id = 25}, {.arc_offset = 0},
		{.door_id = 26},{.arc_offset = 0} ,{.door_id = 27},{.arc_offset = 0} ,{.door_id = 28}, {.arc_offset = 0},
		{.door_id = 29},{.arc_offset = 90},{.door_id = 30},{.arc_offset = 0} ,{.door_id = 31}, {.arc_offset = 0},
		{.door_id = 32},{.arc_offset = 0} ,{.door_id = 33},{.arc_offset = 0} ,{.door_id = 34}, {.arc_offset = 0},
		{.door_id = 35},{.arc_offset = 0} ,{.door_id = 36},{.arc_offset = 90},{.door_id = 37}, {.arc_offset = 0},
		{.door_id = 38},{.arc_offset = 0} ,{.door_id = 39},{.arc_offset = 0} ,{.door_id = 40}, {.arc_offset = 0},
		{.door_id = 41},{.arc_offset = 0} ,{.door_id = 42},{.arc_offset = 0} ,{.door_id = 43}, {.arc_offset = 90},
		{.door_id = 44},{.arc_offset = 0} ,{.door_id = 45},{.arc_offset = 0} ,{.door_id = 46}, {.arc_offset = 0},
		{.door_id = 47},{.arc_offset = 0} ,{.door_id = 48},{.arc_offset = 0} ,{.door_id = 49}, {.arc_offset = 0},
		{.door_id = 50},{.arc_offset = 90},{.door_id = 51},{.arc_offset = 90},{.door_id = 52}, {.arc_offset = 0},
		{.door_id = 53},{.arc_offset = 0} ,{.door_id = 54},{.arc_offset = 0}, {.door_id = 55}, {.arc_offset = 0},
		{.door_id = 56},{.arc_offset = 0} ,{.door_id = 57},{.arc_offset = 0}, {.door_id = 122},{.arc_offset = 0}
	};

	auto m_location = GetPosition();

	for (auto const &d: buyer_door_data) {
		auto door = entity_list.GetDoorsByDoorID(d.door_id);
		if (door && IsWithinCircularArc(
			door->GetPosition(),
			m_location,
			d.arc_offset,
			BUYER_DOOR_ARC_RADIUS_HIGH,
			BUYER_DOOR_ARC_RADIUS_LOW
		)
			) {
			return true;
		}
	}

	for (auto const& d:entity_list.GetDoorsList()) {
		if (d.second->GetOpenType() == DoorType::BuyerStall) {
			if (IsWithinSquare(d.second->GetPosition(), d.second->GetSize(), GetPosition())) {
				return true;
			}
		}
	}

	return false;
}

void Client::CreateStartingBuyLines(const EQApplicationPacket *app)
{
	if (ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		BuyerBuyLines_Struct         bl{};
		auto                         in = (BuyerGeneric_Struct *) app->pBuffer;
		EQ::Util::MemoryStreamReader ss_in(
			reinterpret_cast<char *>(in->payload),
			app->size - sizeof(BuyerGeneric_Struct));
		cereal::BinaryInputArchive   ar(ss_in);
		ar(bl);

		if (bl.buy_lines.empty()) {
			return;
		}

		std::map<uint32, BuylineItemDetails_Struct> item_map{};

		if (!BuildBuyLineMap(item_map, bl)) {
			ToggleBuyerMode(false);
			return;
		}

		auto proposed_total_cost = ValidateBuyLineCost(item_map);
		if (proposed_total_cost == 0) {
			ToggleBuyerMode(false);
			return;
		}

		if (!ValidateBuyLineItems(item_map)) {
			ToggleBuyerMode(false);
			return;
		}

		std::stringstream           ss_out{};
		cereal::BinaryOutputArchive ar_out(ss_out);

		for (auto &b: bl.buy_lines) {
			BuyerBuyLinesRepository::CreateBuyLine(database, b, CharacterID());

			{ ar_out(b); }

			uint32 packet_size = ss_out.str().length() + sizeof(BuyerGeneric_Struct);
			auto   out         = std::make_unique<EQApplicationPacket>(OP_BuyerItems, packet_size);
			auto   data        = (BazaarSearchMessaging_Struct *) out->pBuffer;

			data->action = Barter_BuyerItemUpdate;
			memcpy(data->payload, ss_out.str().data(), ss_out.str().length());
			QueuePacket(out.get());

			ss_out.str("");
			ss_out.clear();
		}

		Message(Chat::Yellow, fmt::format("{} buy lines enabled.", bl.buy_lines.size()).c_str());
	}
}

void Client::SendBuyLineUpdate(const BuyerLineItems_Struct &buy_line)
{
	std::stringstream           ss_out{};
	cereal::BinaryOutputArchive ar_out(ss_out);

	{ ar_out(buy_line); }

	uint32 packet_size = ss_out.str().length() + sizeof(BuyerGeneric_Struct);
	auto   out         = std::make_unique<EQApplicationPacket>(OP_BuyerItems, packet_size);
	auto   data        = (BazaarSearchMessaging_Struct *) out->pBuffer;

	data->action = Barter_BuyerItemUpdate;
	memcpy(data->payload, ss_out.str().data(), ss_out.str().length());
	QueuePacket(out.get());

	ss_out.str("");
	ss_out.clear();
}

void Client::CheckIfMovedItemIsPartOfBuyLines(uint32 item_id)
{
	auto b_trade_items = BuyerTradeItemsRepository::GetTradeItems(database, GetBuyerID());
	if (b_trade_items.empty()) {
		return;
	}

	auto it = std::find_if(
		b_trade_items.cbegin(),
		b_trade_items.cend(),
		[&](const BaseBuyerTradeItemsRepository::BuyerTradeItems bti) {
			return bti.item_id == item_id;
		}
	);
	if (it != std::end(b_trade_items)) {
		auto item = GetInv().GetItem(GetInv().HasItem(item_id, 1, invWherePersonal));
		if (!item) {
			return;
		}

		Message(
			Chat::Red,
			fmt::format(
				"You moved an item ({}) that is part of an active buy line.",
				item->GetItem()->Name
			).c_str()
		);
		ToggleBuyerMode(false);
	}
}

void Client::SendWindowUpdatesToSellerAndBuyer(BuyerLineSellItem_Struct &blsi)
{
	auto buyer  = entity_list.GetClientByID(blsi.buyer_entity_id);
	auto seller = this;
	if (!buyer || !seller) {
		return;
	}

	if (blsi.item_quantity - blsi.seller_quantity <= 0) {
		auto outapp = std::make_unique<EQApplicationPacket>(
			OP_BuyerItems,
			static_cast<uint32>(sizeof(BuyerRemoveItemFromMerchantWindow_Struct))
		);
		auto data   = (BuyerRemoveItemFromMerchantWindow_Struct *) outapp->pBuffer;

		data->action      = Barter_RemoveFromMerchantWindow;
		data->buy_slot_id = blsi.slot;
		QueuePacket(outapp.get());

		std::stringstream           ss{};
		cereal::BinaryOutputArchive ar(ss);

		BuyerLineItems_Struct bl{};
		bl.enabled       = 0;
		bl.item_cost     = blsi.item_cost;
		bl.item_icon     = blsi.item_icon;
		bl.item_id       = blsi.item_id;
		bl.item_quantity = blsi.item_quantity - blsi.seller_quantity;
		bl.item_name     = blsi.item_name;
		bl.item_toggle   = 0;
		bl.slot          = blsi.slot;

		for (auto const &b: blsi.trade_items) {
			BuyerLineTradeItems_Struct blti{};
			blti.item_icon     = b.item_icon;
			blti.item_id       = b.item_id;
			blti.item_quantity = b.item_quantity;
			blti.item_name     = b.item_name;
			bl.trade_items.push_back(blti);
		}

		{ ar(bl); }

		uint32 packet_size = ss.str().length() + sizeof(BuyerGeneric_Struct);
		outapp = std::make_unique<EQApplicationPacket>(OP_BuyerItems, packet_size);
		auto emu = (BuyerGeneric_Struct *) outapp->pBuffer;

		emu->action = Barter_BuyerItemUpdate;
		memcpy(emu->payload, ss.str().data(), ss.str().length());

		buyer->QueuePacket(outapp.get());
		BuyerBuyLinesRepository::DeleteBuyLine(database, buyer->CharacterID(), blsi.slot);
	}
	else {
		std::stringstream           ss{};
		cereal::BinaryOutputArchive ar(ss);

		BuyerLineItems_Struct bli{};
		bli.enabled       = 1;
		bli.item_cost     = blsi.item_cost;
		bli.item_icon     = blsi.item_icon;
		bli.item_id       = blsi.item_id;
		bli.item_quantity = blsi.item_quantity - blsi.seller_quantity;
		bli.item_toggle   = 1;
		bli.slot          = blsi.slot;
		bli.item_name     = blsi.item_name;
		for (auto const &b: blsi.trade_items) {
			BuyerLineTradeItems_Struct blti{};
			blti.item_id       = b.item_id;
			blti.item_icon     = b.item_icon;
			blti.item_quantity = b.item_quantity;
			blti.item_name     = b.item_name;
			bli.trade_items.push_back(blti);
		}
		{ ar(bli); }

		uint32 packet_size = ss.str().length() + sizeof(BuyerGeneric_Struct);
		auto   outapp      = std::make_unique<EQApplicationPacket>(OP_BuyerItems, packet_size);
		auto   emu         = (BuyerGeneric_Struct *) outapp->pBuffer;

		emu->action = Barter_BuyerInspectBegin;
		memcpy(emu->payload, ss.str().data(), ss.str().length());

		QueuePacket(outapp.get());

		outapp = std::make_unique<EQApplicationPacket>(OP_BuyerItems, packet_size);
		emu    = (BuyerGeneric_Struct *) outapp->pBuffer;

		emu->action = Barter_BuyerItemUpdate;
		memcpy(emu->payload, ss.str().data(), ss.str().length());

		buyer->QueuePacket(outapp.get());

		BuyerBuyLinesRepository::ModifyBuyLine(database, bli, buyer->GetBuyerID());
	}
}

void Client::SendBuyerToBarterWindow(Client *buyer, uint32 action)
{
	auto server_packet = std::make_unique<ServerPacket>(
		ServerOP_BuyerMessaging,
		static_cast<uint32>(sizeof(BuyerMessaging_Struct))
	);
	auto data          = (BuyerMessaging_Struct *) server_packet->pBuffer;

	data->action          = action;
	data->zone_id         = buyer->GetZoneID();
	data->buyer_id        = buyer->GetBuyerID();
	data->buyer_entity_id = buyer->GetID();
	strn0cpy(data->buyer_name, buyer->GetCleanName(), sizeof(data->buyer_name));

	worldserver.SendPacket(server_packet.get());
}

void Client::SendBulkBazaarBuyers()
{
	auto results = BuyerRepository::All(database);

	if (results.empty()) {
		return;
	}

	auto outapp = std::make_unique<EQApplicationPacket>(
		OP_Barter,
		static_cast<uint32>(sizeof(BuyerAddBuyertoBarterWindow_Struct))
	);
	auto emu    = (BuyerAddBuyertoBarterWindow_Struct *) outapp->pBuffer;

	for (auto const &b: results) {
		auto buyer = entity_list.GetClientByCharID(b.char_id);
		emu->action          = Barter_AddToBarterWindow;
		emu->buyer_id        = b.char_id;
		emu->buyer_entity_id = buyer ? buyer->GetID() : 0;
		emu->zone_id         = buyer ? buyer->GetZoneID() : 0;
		strn0cpy(emu->buyer_name, b.char_name.c_str(), sizeof(emu->buyer_name));

		QueuePacket(outapp.get());
	}
}

void Client::SendBarterBuyerClientMessage(
	BuyerLineSellItem_Struct &blsi,
	BarterBuyerActions action,
	BarterBuyerSubActions sub_action,
	BarterBuyerSubActions error_code
)
{
	std::stringstream           ss{};
	cereal::BinaryOutputArchive ar(ss);

	blsi.sub_action = sub_action;
	blsi.error_code = error_code;

	{ ar(blsi); }

	uint32 packet_size = ss.str().length() + sizeof(BuyerGeneric_Struct);
	auto   outapp      = std::make_unique<EQApplicationPacket>(OP_BuyerItems, packet_size);
	auto   emu         = (BuyerGeneric_Struct *) outapp->pBuffer;

	emu->action = action;
	memcpy(emu->payload, ss.str().data(), ss.str().length());

	QueuePacket(outapp.get());
}

bool Client::BuildBuyLineMap(std::map<uint32, BuylineItemDetails_Struct> &item_map, BuyerBuyLines_Struct &bl)
{
	bool buyer_error = false;

	for (auto const &b: bl.buy_lines) {
		if (item_map.contains(b.item_id) && item_map[b.item_id].item_cost > 0) {
			Message(
				Chat::Red,
				fmt::format(
					"You cannot have two buy lines for the same item {}. Buy line not possible.",
					b.item_name
				).c_str()
			);
			buyer_error = true;
			break;
		}
		BuylineItemDetails_Struct t = {b.item_quantity * b.item_cost, b.item_quantity};
		item_map.emplace(b.item_id, t);
		for (auto const &i: b.trade_items) {
			if (item_map.contains(i.item_id) && item_map[i.item_id].item_cost > 0) {
				Message(
					Chat::Red,
					fmt::format(
						"You cannot buy {} and offer the same item as compensation. Buy line not possible.",
						i.item_name
					).c_str()
				);
				buyer_error = true;
				break;
			}
			if (item_map.contains(i.item_id)) {
				item_map[i.item_id].item_quantity += i.item_quantity * b.item_quantity;
				continue;
			}
			t = {0, i.item_quantity * b.item_quantity};
			item_map.emplace(i.item_id, t);
		}
	}

	if (buyer_error) {
		return false;
	}

	return true;
}

bool Client::BuildBuyLineMapFromVector(
	std::map<uint32, BuylineItemDetails_Struct> &item_map,
	std::vector<BuyerLineItems_Struct> &bl
)
{

	bool buyer_error = false;

	for (auto const &b: bl) {
		if (item_map.contains(b.item_id) && item_map[b.item_id].item_cost > 0) {
			Message(
				Chat::Red,
				fmt::format(
					"You cannot have two buy lines for the same item {}. Buy line not possible.",
					b.item_name
				).c_str()
			);
			buyer_error = true;
			break;
		}
		BuylineItemDetails_Struct t = {b.item_quantity * b.item_cost, b.item_quantity};
		item_map.emplace(b.item_id, t);
		for (auto const &i: b.trade_items) {
			if (item_map.contains(i.item_id) && item_map[i.item_id].item_cost > 0) {
				Message(
					Chat::Red,
					fmt::format(
						"You cannot buy {} and offer the same item as compensation. Buy line not possible.",
						i.item_name
					).c_str()
				);
				buyer_error = true;
				break;
			}
			if (item_map.contains(i.item_id)) {
				item_map[i.item_id].item_quantity += i.item_quantity * b.item_quantity;
				continue;
			}
			t = {0, i.item_quantity * b.item_quantity};
			item_map.emplace(i.item_id, t);
		}
	}

	if (buyer_error) {
		return false;
	}

	return true;
}

void
Client::RemoveItemFromBuyLineMap(std::map<uint32, BuylineItemDetails_Struct> &item_map, const BuyerLineItems_Struct &bl)
{
	if (item_map.contains(bl.item_id) && item_map[bl.item_id].item_cost > 0) {
		item_map.erase(bl.item_id);
	}

	for (auto const &i: bl.trade_items) {
		if (item_map.contains(i.item_id) &&
			(item_map[i.item_id].item_quantity - (i.item_quantity * bl.item_quantity)) == 0) {
			item_map.erase(i.item_id);
		}
		else if (item_map.contains(i.item_id)) {
			item_map[i.item_id].item_quantity -= i.item_quantity * bl.item_quantity;
		}
	}
}

bool Client::ValidateBuyLineItems(std::map<uint32, BuylineItemDetails_Struct> &item_map)
{
	bool buyer_error = false;

	for (auto const &i: item_map) {
		auto item = database.GetItem(i.first);
		if (!item) {
			buyer_error = true;
			break;
		}

		if (i.second.item_cost > 0) {
			auto buy_item_slot_id = GetInv().HasItem(i.first, i.second.item_quantity, invWherePersonal);
			auto buy_item         = buy_item_slot_id == INVALID_INDEX ? nullptr : GetInv().GetItem(buy_item_slot_id);
			if (buy_item && CheckLoreConflict(buy_item->GetItem())) {
				Message(
					Chat::Red,
					fmt::format(
						"You already have a {}. Purchasing another will cause a lore conflict. Buy line not possible.",
						buy_item->GetItem()->Name
					).c_str()
				);
				buyer_error = true;
				break;
			}
		}
		if (i.second.item_cost == 0) {
			if (i.second.item_quantity > 1 && CheckLoreConflict(item)) {
				Message(
					Chat::Red,
					fmt::format(
						"Your buy line requires {} {}s however the item is LORE. Buy line not possible.",
						i.second.item_quantity,
						item->Name
					).c_str()
				);
				buyer_error = true;
				break;
			}

			auto buy_item_slot_id = GetInv().HasItem(i.first, i.second.item_quantity, invWherePersonal);
			auto buy_item         = buy_item_slot_id == INVALID_INDEX ? nullptr : GetInv().GetItem(buy_item_slot_id);

			if (!buy_item) {
				Message(
					Chat::Red,
					fmt::format(
						"Your buy line(s) require a total of {} {}{} which could not be found. Buy line not possible.",
						i.second.item_quantity,
						item->Name,
						i.second.item_quantity > 1 ? "s" : ""
					).c_str()
				);
				buyer_error = true;
				break;
			}

			if (buy_item->IsAugmentable() && buy_item->IsAugmented()) {
				Message(
					Chat::Red,
					fmt::format(
						"You cannot offer {} because it is augmented. Buy line not possible.",
						buy_item->GetItem()->Name
					).c_str()
				);
				buyer_error = true;
				break;
			}

			if (!buy_item->IsDroppable()) {
				Message(
					Chat::Red,
					fmt::format(
						"You cannot offer {} because it is NoTrade. Buy line not possible.",
						buy_item->GetItem()->Name
					).c_str());
				buyer_error = true;
				break;
			}

			buyer_error = false;
		}
	}

	return !buyer_error;
}

int64 Client::ValidateBuyLineCost(std::map<uint32, BuylineItemDetails_Struct> &item_map)
{
	uint64 proposed_total_cost = std::accumulate(
		item_map.cbegin(),
		item_map.cend(),
		static_cast<uint64>(0),
		[](uint64 prev_sum, const std::pair<uint32, BuylineItemDetails_Struct> &x) {
			return prev_sum + x.second.item_cost;
		}
	);

	if (proposed_total_cost > GetCarriedMoney()) {
		Message(
			Chat::Red,
			fmt::format(
				"You currently do not have sufficient funds to support your buy lines. You have {} and need {}",
				DetermineMoneyString(GetCarriedMoney()),
				DetermineMoneyString(proposed_total_cost)).c_str()
		);
		return 0;
	}

	return proposed_total_cost;
}

bool Client::DoBarterBuyerChecks(BuyerLineSellItem_Struct &sell_line)
{
	bool buyer_error = false;
	auto buyer       = entity_list.GetClientByID(sell_line.buyer_entity_id);

	if (!buyer) {
		return false;
	}

	auto buyer_time = BuyerRepository::GetTransactionDate(database, buyer->CharacterID());
	if (buyer_time > GetBarterTime()) {
		if (sell_line.purchase_method == BarterByVendor) {
			SendBarterBuyerClientMessage(
				sell_line,
				Barter_SellerTransactionComplete,
				Barter_Success,
				Barter_DataOutOfDate
			);
			return false;
		}
		SendBarterBuyerClientMessage(sell_line, Barter_SellerTransactionComplete, Barter_Failure, Barter_DataOutOfDate);
		return false;
	}

	for (auto const &ti: sell_line.trade_items) {
		auto ti_slot_id = buyer->GetInv().HasItem(
			ti.item_id,
			ti.item_quantity * sell_line.seller_quantity,
			invWherePersonal
		);
		if (ti_slot_id == INVALID_INDEX) {
			LogTradingDetail(
				"Seller attempting to sell item <green>[{}] to buyer <green>[{}] though buyer no longer has compensation item <red>[{}]",
				sell_line.item_name,
				buyer->GetCleanName(),
				ti.item_name
			);
			buyer->Message(
				Chat::Red,
				fmt::format(
					"{} wanted to sell you {} however you no longer have compensation item {}",
					sell_line.seller_name,
					sell_line.item_name,
					ti.item_name
				).c_str());
			buyer_error = true;
			break;
		}
	}

	uint64 total_cost = (uint64) sell_line.item_cost * (uint64) sell_line.seller_quantity;
	if (!buyer->HasMoney(total_cost)) {
		LogTradingDetail(
			"Seller attempting to sell item <green>[{}] to buyer <green>[{}] though buyer does not have enough money <red>[{}]",
			sell_line.item_name,
			buyer->GetCleanName(),
			total_cost
		);
		buyer->Message(
			Chat::Red,
			fmt::format(
				"{} wanted to sell you {} however you have insufficient funds.",
				sell_line.seller_name,
				sell_line.item_name
			).c_str()
		);
		buyer_error = true;
	}

	auto buy_item_slot_id = buyer->GetInv().HasItem(
		sell_line.item_id,
		sell_line.seller_quantity,
		invWherePersonal
	);
	auto buy_item         = buy_item_slot_id == INVALID_INDEX ? nullptr : buyer->GetInv().GetItem(buy_item_slot_id);
	if (buy_item && buyer->CheckLoreConflict(buy_item->GetItem())) {
		LogTradingDetail(
			"Seller attempting to sell item <green>[{}] to buyer <green>[{}] though buyer already has the item which is LORE.",
			sell_line.item_name,
			buyer->GetCleanName()
		);
		buyer->Message(
			Chat::Red,
			fmt::format(
				"{} wanted to sell you {} however you already have the LORE item.",
				sell_line.seller_name,
				sell_line.item_name
			).c_str()
		);
		buyer_error = true;
	}

	if (buyer_error) {
		LogTradingDetail("Buyer error <red>[{}] Barter Sell/Buy Transaction Failed.", buyer_error);
		SendBarterBuyerClientMessage(sell_line, Barter_SellerTransactionComplete, Barter_Failure, Barter_Failure);
		return false;
	}

	return true;
}

bool Client::DoBarterSellerChecks(BuyerLineSellItem_Struct &sell_line)
{
	bool seller_error = false;
	auto sell_item_slot_id = GetInv().HasItem(sell_line.item_id, sell_line.seller_quantity, invWherePersonal);
	auto sell_item = sell_item_slot_id == INVALID_INDEX ? nullptr : GetInv().GetItem(sell_item_slot_id);
	if (!sell_item) {
		seller_error = true;
		LogTradingDetail("Seller no longer has item <red>[{}] to sell to buyer <red>[{}]",
						 sell_line.item_name,
						 sell_line.buyer_name
		);
		SendBarterBuyerClientMessage(
			sell_line,
			Barter_SellerTransactionComplete,
			Barter_Failure,
			Barter_SellerDoesNotHaveItem
		);
	}

	if (sell_item && sell_item->IsAugmentable() && sell_item->IsAugmented()) {
		seller_error = true;
		LogTradingDetail("Seller item <red>[{}] is augmented therefore cannot be sold.",
						 sell_line.item_name
		);
		Message(Chat::Red, "The item that you are trying to sell is augmented. Please remove augments first");
	}

	if (sell_item && !sell_item->IsDroppable()) {
		seller_error = true;
		LogTradingDetail("Seller item <red>[{}] is non-tradeable therefore cannot be sold.",
						 sell_line.item_name
		);
		Message(Chat::Red, "The item that you are trying to sell is non-tradeable and therefore cannot be sold.");
	}

	if (seller_error) {
		LogTradingDetail("Seller Error <red>[{}]  Barter Sell/Buy Transaction Failed.", seller_error);
		SendBarterBuyerClientMessage(sell_line, Barter_SellerTransactionComplete, Barter_Failure, Barter_Failure);
		return false;
	}

	return true;
}

void Client::CancelBuyerTradeWindow()
{
	auto end_session = new EQApplicationPacket(OP_Barter, sizeof(BuyerRemoveItemFromMerchantWindow_Struct));
	auto data        = reinterpret_cast<BuyerRemoveItemFromMerchantWindow_Struct *>(end_session->pBuffer);
	data->action     = Barter_BuyerInspectBegin;

	FastQueuePacket(&end_session);
}

void Client::CancelTraderTradeWindow()
{
	auto end_session = new EQApplicationPacket(OP_ShopEnd);
	FastQueuePacket(&end_session);
}
