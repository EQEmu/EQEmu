#include "../../client.h"
#include "../../object.h"

void SendShowInventorySubCommands(Client* c) {
	c->Message(Chat::White, "Usage: #show inventory equip - Shows items in Equipment slots");
	c->Message(Chat::White, "Usage: #show inventory gen - Shows items in General slots");
	c->Message(Chat::White, "Usage: #show inventory cursor - Shows items in Cursor slots");
	c->Message(Chat::White, "Usage: #show inventory poss - Shows items in Equipment, General, and Cursor slots");
	c->Message(Chat::White, "Usage: #show inventory limbo - Shows items in Limbo slots");
	c->Message(Chat::White, "Usage: #show inventory curlim - Shows items in Cursor and Limbo slots");
	c->Message(Chat::White, "Usage: #show inventory trib - Shows items in Tribute slots");
	c->Message(Chat::White, "Usage: #show inventory bank - Shows items in Bank slots");
	c->Message(Chat::White, "Usage: #show inventory shbank - Shows items in Shared Bank slots");
	c->Message(Chat::White, "Usage: #show inventory allbank - Shows items in Bank and Shared Bank slots");
	c->Message(Chat::White, "Usage: #show inventory trade - Shows items in Trade slots");
	c->Message(Chat::White, "Usage: #show inventory world - Shows items in World slots");
	c->Message(Chat::White, "Usage: #show inventory all - Shows items in all slots");
}

void ShowInventory(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		SendShowInventorySubCommands(c);
		return;
	}

	// this can be cleaned up once inventory is cleaned up
	enum {
		peekNone       = 0x0000,
		peekEquip      = 0x0001,
		peekGen        = 0x0002,
		peekCursor     = 0x0004,
		peekLimbo      = 0x0008,
		peekTrib       = 0x0010,
		peekBank       = 0x0020,
		peekShBank     = 0x0040,
		peekTrade      = 0x0080,
		peekWorld      = 0x0100,
		peekOutOfScope = (peekWorld * 2)
	};

	static const int16 scope_range[][2] = {
		{ EQ::invslot::EQUIPMENT_BEGIN,   EQ::invslot::EQUIPMENT_END },
		{ EQ::invslot::GENERAL_BEGIN,     EQ::invslot::GENERAL_END },
		{ EQ::invslot::slotCursor,        EQ::invslot::slotCursor },
		{ EQ::invslot::SLOT_INVALID,      EQ::invslot::SLOT_INVALID },
		{ EQ::invslot::TRIBUTE_BEGIN,     EQ::invslot::TRIBUTE_END },
		{ EQ::invslot::BANK_BEGIN,        EQ::invslot::BANK_END },
		{ EQ::invslot::SHARED_BANK_BEGIN, EQ::invslot::SHARED_BANK_END },
		{ EQ::invslot::TRADE_BEGIN,       EQ::invslot::TRADE_END },
		{ EQ::invslot::SLOT_BEGIN,        (EQ::invtype::WORLD_SIZE - 1) }
	};

	static const bool scope_bag[] = {
		false, // Equip
		true, // General
		true, // Cursor
		true, // Cursor Limbo
		false, // Tribute
		true, // Bank
		true, // Shared Bank
		true, // Trade
		true // World
	};

	int scope_mask = peekNone;

	const bool is_all          = !strcasecmp(sep->arg[2], "all");
	const bool is_all_bank     = !strcasecmp(sep->arg[2], "allbank");
	const bool is_bank         = !strcasecmp(sep->arg[2], "bank");
	const bool is_cursor       = !strcasecmp(sep->arg[2], "cursor");
	const bool is_cursor_limbo = !strcasecmp(sep->arg[2], "curlimbo");
	const bool is_equipment    = !strcasecmp(sep->arg[2], "equip");
	const bool is_general      = !strcasecmp(sep->arg[2], "gen");
	const bool is_limbo        = !strcasecmp(sep->arg[2], "limbo");
	const bool is_possessions  = !strcasecmp(sep->arg[2], "poss");
	const bool is_shared_bank  = !strcasecmp(sep->arg[2], "shbank");
	const bool is_trade        = !strcasecmp(sep->arg[2], "trade");
	const bool is_tribute      = !strcasecmp(sep->arg[2], "trib");
	const bool is_world        = !strcasecmp(sep->arg[2], "world");

	if (is_all) {
		scope_mask = (peekOutOfScope - 1);
	} else if (is_all_bank) {
		scope_mask |= (peekBank | peekShBank);
	} else if (is_bank) {
		scope_mask |= peekBank;
	} else if (is_cursor) {
		scope_mask |= peekCursor;
	} else if (is_cursor_limbo) {
		scope_mask |= (peekCursor | peekLimbo);
	} else if (is_equipment) {
		scope_mask |= peekEquip;
	} else if (is_general) {
		scope_mask |= peekGen;
	} else if (is_limbo) {
		scope_mask |= peekLimbo;
	} else if (is_possessions) {
		scope_mask |= (peekEquip | peekGen | peekCursor);
	} else if (is_shared_bank) {
		scope_mask |= peekShBank;
	} else if (is_tribute) {
		scope_mask |= peekTrib;
	} else if (is_trade) {
		scope_mask |= peekTrade;
	} else if (is_world) {
		scope_mask |= peekWorld;
	} else {
		SendShowInventorySubCommands(c);
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const EQ::ItemInstance *inst_main = nullptr;
	const EQ::ItemInstance *inst_sub  = nullptr;
	const EQ::ItemInstance *inst_aug  = nullptr;
	const EQ::ItemData     *item_data = nullptr;

	EQ::SayLinkEngine linker;
	linker.SetLinkType(EQ::saylink::SayLinkItemInst);

	c->Message(
		Chat::White,
		fmt::format(
			"Displaying inventory of {}.",
			c->GetTargetDescription(t)
		).c_str()
	);

	auto o           = t->GetTradeskillObject();
	auto found_items = false;

	for (int scope_index = 0, scope_bit = peekEquip; scope_bit < peekOutOfScope; ++scope_index, scope_bit <<= 1) {
		if (scope_bit & ~scope_mask) {
			continue;
		}

		if (scope_bit & peekWorld) {
			if (!o) {
				c->Message(Chat::White, "No world Tradeskill object selected.");
				continue;
			} else {
				c->Message(
					Chat::White,
					fmt::format(
						"[World Object] Database ID: {} Entity ID: {}",
						o->GetDBID(),
						o->GetID()
					).c_str()
				);
			}
		}

		for (int16 index_main = scope_range[scope_index][0]; index_main <= scope_range[scope_index][1]; ++index_main) {
			if (index_main == EQ::invslot::SLOT_INVALID) {
				continue;
			}

			inst_main = (
				(scope_bit & peekWorld) ?
				o->GetItem(index_main) :
				t->GetInv().GetItem(index_main)
			);

			if (inst_main) {
				found_items = true;
				item_data  = inst_main->GetItem();
			} else {
				item_data = nullptr;
			}

			linker.SetItemInst(inst_main);

			if (item_data && inst_main) {
				//auto inst = c->GetInv().GetItem(scope_bit & peekWorld ? EQ::invslot::WORLD_BEGIN + index_main : index_main);
				c->Message(
					Chat::White,
					fmt::format(
						"Slot {} | {} ({}/{}){}",
						scope_bit & peekWorld ? EQ::invslot::WORLD_BEGIN + index_main : index_main,
						linker.GenerateLink(),
						item_data->ID,
						inst_main->GetSerialNumber(),
						inst_main->IsStackable() && inst_main->GetCharges() > 0 ?
						fmt::format(
							" (Stack of {})",
							inst_main->GetCharges()
						) :
						""
					).c_str()
				);
			}

			if (inst_main && inst_main->IsClassCommon()) {
				for (uint8 augment_index = EQ::invaug::SOCKET_BEGIN; augment_index <= EQ::invaug::SOCKET_END; ++augment_index) {
					inst_aug = inst_main->GetItem(augment_index);
					if (!inst_aug) { // extant only
						continue;
					}

					item_data = inst_aug->GetItem();
					linker.SetItemInst(inst_aug);

					c->Message(
						Chat::White,
						fmt::format(
							"Slot {} (Augment Slot {}) | {} ({}){}",
							((scope_bit & peekWorld) ? (EQ::invslot::WORLD_BEGIN + index_main) : index_main),
							augment_index,
							linker.GenerateLink(),
							item_data->ID,
							(
								inst_aug->IsStackable() && inst_aug->GetCharges() > 0 ?
								fmt::format(
									" (Stack of {})",
									inst_aug->GetCharges()
								) :
								""
							)
						).c_str()
					);

				}
			}

			if (!scope_bag[scope_index] || !(inst_main && inst_main->IsClassBag())) {
				continue;
			}

			for (uint8 sub_index = EQ::invbag::SLOT_BEGIN; sub_index <= EQ::invbag::SLOT_END; ++sub_index) {
				inst_sub = inst_main->GetItem(sub_index);
				if (!inst_sub) { // extant only
					continue;
				}

				item_data = inst_sub->GetItem();
				linker.SetItemInst(inst_sub);

				c->Message(
					Chat::White,
					fmt::format(
						"Slot {} Bag Slot {}/{} | {} ({}/{}){}",
						(
							(scope_bit & peekWorld) ?
							INVALID_INDEX :
							EQ::InventoryProfile::CalcSlotId(index_main, sub_index)
						),
						((scope_bit & peekWorld) ? (EQ::invslot::WORLD_BEGIN + index_main) : index_main),
						sub_index,
						linker.GenerateLink(),
						item_data->ID,
						inst_sub->GetSerialNumber(),
						(
							inst_sub->IsStackable() && inst_sub->GetCharges() > 0 ?
							fmt::format(
								" (Stack of {})",
								inst_sub->GetCharges()
							) :
							""
						)
					).c_str()
				);

				if (inst_sub->IsClassCommon()) {
					for (uint8 augment_index = EQ::invaug::SOCKET_BEGIN; augment_index <= EQ::invaug::SOCKET_END; ++augment_index) {
						inst_aug = inst_sub->GetItem(augment_index);
						if (!inst_aug) { // extant only
							continue;
						}

						item_data = inst_aug->GetItem();
						linker.SetItemInst(inst_aug);

						c->Message(
							Chat::White,
							fmt::format(
								"Slot {} Bag Slot {} (Augment Slot {}) | {} ({}){}",
								(
									(scope_bit & peekWorld) ?
									INVALID_INDEX :
									EQ::InventoryProfile::CalcSlotId(index_main,sub_index)
								),
								sub_index,
								augment_index,
								linker.GenerateLink(),
								item_data->ID,
								(
									inst_sub->IsStackable() && inst_sub->GetCharges() > 0 ?
									fmt::format(
										" (Stack of {})",
										inst_sub->GetCharges()
									) :
									""
								)
							).c_str()
						);
					}
				}
			}
		}

		if (scope_bit & peekLimbo) {
			int       limboIndex = 0;
			for (auto it = t->GetInv().cursor_cbegin(); (it != t->GetInv().cursor_cend()); ++it, ++limboIndex) {
				if (it == t->GetInv().cursor_cbegin()) {
					continue;
				}

				inst_main = *it;
				if (inst_main) {
					found_items = true;
					item_data  = inst_main->GetItem();
				} else {
					item_data = nullptr;
				}

				linker.SetItemInst(inst_main);

				if (item_data) {
					c->Message(
						Chat::White,
						fmt::format(
							"Slot {} | {} ({}){}",
							(14000 + limboIndex),
							item_data->ID,
							linker.GenerateLink(),
							(
								inst_main->IsStackable() && inst_main->GetCharges() > 0 ?
								fmt::format(
									" (Stack of {})",
									inst_main->GetCharges()
								) :
								""
							)
						).c_str()
					);
				}

				if (inst_main && inst_main->IsClassCommon()) {
					for (uint8 augment_index = EQ::invaug::SOCKET_BEGIN; augment_index <= EQ::invaug::SOCKET_END; ++augment_index) {
						inst_aug = inst_main->GetItem(augment_index);
						if (!inst_aug) { // extant only
							continue;
						}

						item_data = inst_aug->GetItem();
						linker.SetItemInst(inst_aug);

						c->Message(
							Chat::White,
							fmt::format(
								"Slot {} (Augment Slot {}) | {} ({}){}",
								(14000 + limboIndex),
								augment_index,
								linker.GenerateLink(),
								item_data->ID,
								(
									inst_aug->IsStackable() && inst_aug->GetCharges() > 0 ?
									fmt::format(
										" (Stack of {})",
										inst_aug->GetCharges()
									) :
									""
								)
							).c_str()
						);
					}
				}

				if (!scope_bag[scope_index] || !(inst_main && inst_main->IsClassBag())) {
					continue;
				}

				for (uint8 sub_index = EQ::invbag::SLOT_BEGIN; sub_index <= EQ::invbag::SLOT_END; ++sub_index) {
					inst_sub = inst_main->GetItem(sub_index);
					if (!inst_sub) {
						continue;
					}

					item_data = (inst_sub == nullptr) ? nullptr : inst_sub->GetItem();

					linker.SetItemInst(inst_sub);

					if (item_data) {
						c->Message(
							Chat::White,
							fmt::format(
								"Slot {} Bag Slot {} | {} ({}){}",
								(14000 + limboIndex),
								sub_index,
								linker.GenerateLink(),
								item_data->ID,
								(
									inst_sub->IsStackable() && inst_sub->GetCharges() > 0 ?
									fmt::format(
										" (Stack of {})",
										inst_sub->GetCharges()
									) :
									""
								)
							).c_str()
						);
					}

					if (inst_sub->IsClassCommon()) {
						for (uint8 augment_index = EQ::invaug::SOCKET_BEGIN;
							augment_index <= EQ::invaug::SOCKET_END;
							++augment_index) {
							inst_aug = inst_sub->GetItem(augment_index);
							if (!inst_aug) { // extant only
								continue;
							}

							item_data = inst_aug->GetItem();
							linker.SetItemInst(inst_aug);

							c->Message(
								Chat::White,
								fmt::format(
									"Slot {} Bag Slot {} (Augment Slot {}) | {} ({}){}",
									(14000 + limboIndex),
									sub_index,
									augment_index,
									linker.GenerateLink(),
									item_data->ID,
									(
										inst_sub->IsStackable() && inst_sub->GetCharges() > 0 ?
										fmt::format(
											" (Stack of {})",
											inst_sub->GetCharges()
										) :
										""
									)
								).c_str()
							);
						}
					}
				}
			}
		}
	}

	if (!found_items) {
		c->Message(Chat::White, "No items found.");
	}
}