#include "common/data_verification.h"
#include "zone/client.h"
#include "zone/corpse.h"

void command_npcloot(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || (!c->GetTarget()->IsNPC() && !c->GetTarget()->IsCorpse())) {
		c->Message(Chat::White, "You must target an NPC or a Corpse to use this command.");
		return;
	}

	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #npcloot add [Item ID] [Charges] [Equip] [Augment 1 ID] [Augment 2 ID] [Augment 3 ID] [Augment 4 ID] [Augment 5 ID] [Augment 6 ID] - Adds the specified item to an NPC's loot");
		c->Message(Chat::White, "Usage: #npcloot money [Platinum] [Gold] [Silver] [Copper] - Set an NPC's current money");
		c->Message(Chat::White, "Usage: #npcloot remove [All|Item ID] - Remove loot from an NPC by ID or remove all loot");
		c->Message(Chat::White, "Usage: #npcloot show - Shows target NPC's or Corpse's current loot");
		return;
	}

	bool is_add = !strcasecmp(sep->arg[1], "add");
	bool is_money = !strcasecmp(sep->arg[1], "money");
	bool is_remove = !strcasecmp(sep->arg[1], "remove");
	bool is_show = !strcasecmp(sep->arg[1], "show");

	if (
		!is_add &&
		!is_money &&
		!is_remove &&
		!is_show
	) {
		c->Message(Chat::White, "Usage: #npcloot add [Item ID] [Charges] [Equip] [Augment 1 ID] [Augment 2 ID] [Augment 3 ID] [Augment 4 ID] [Augment 5 ID] [Augment 6 ID] - Adds the specified item to an NPC's loot");
		c->Message(Chat::White, "Usage: #npcloot money [Platinum] [Gold] [Silver] [Copper] - Set an NPC's current money");
		c->Message(Chat::White, "Usage: #npcloot remove [All|Item ID] - Remove loot from an NPC by ID or remove all loot");
		c->Message(Chat::White, "Usage: #npcloot show - Shows target NPC's or Corpse's current loot");
		return;
	}

	if (is_add) {
		if (!c->GetTarget()->IsNPC() || !sep->IsNumber(2)) {
			c->Message(Chat::White, "Usage: #npcloot add [Item ID] [Charges] [Equip] [Augment 1 ID] [Augment 2 ID] [Augment 3 ID] [Augment 4 ID] [Augment 5 ID] [Augment 6 ID] - Adds the specified item to an NPC's loot");
			return;
		}

		auto item_id = Strings::ToUnsignedInt(sep->arg[2]);
		auto item_charges = sep->IsNumber(3) ? static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[3])) : 1;
		bool equip_item = arguments >= 4 ? atobool(sep->arg[4]) : false;
		auto augment_one_id = sep->IsNumber(5) ? Strings::ToUnsignedInt(sep->arg[5]) : 0;
		auto augment_two_id = sep->IsNumber(6) ? Strings::ToUnsignedInt(sep->arg[6]) : 0;
		auto augment_three_id = sep->IsNumber(7) ? Strings::ToUnsignedInt(sep->arg[7]) : 0;
		auto augment_four_id = sep->IsNumber(8) ? Strings::ToUnsignedInt(sep->arg[8]) : 0;
		auto augment_five_id = sep->IsNumber(9) ? Strings::ToUnsignedInt(sep->arg[9]) : 0;
		auto augment_six_id = sep->IsNumber(10) ? Strings::ToUnsignedInt(sep->arg[10]) : 0;

		auto item_data = database.GetItem(item_id);

		if (!item_data) {
			c->Message(
				Chat::White,
				fmt::format(
					"Item ID {} could not be found",
					item_id
				).c_str()
			);
			return;
		}

		auto target = c->GetTarget();

		target->CastToNPC()->AddItem(
			item_id,
			item_charges,
			equip_item,
			augment_one_id,
			augment_two_id,
			augment_three_id,
			augment_four_id,
			augment_five_id,
			augment_six_id
		);

		auto item = database.CreateItem(
			item_id,
			item_charges,
			augment_one_id,
			augment_two_id,
			augment_three_id,
			augment_four_id,
			augment_five_id,
			augment_six_id
		);

		EQ::SayLinkEngine linker;
		linker.SetLinkType(EQ::saylink::SayLinkItemInst);
		linker.SetItemInst(item);

		auto item_link = linker.GenerateLink();

		c->Message(
			Chat::White,
			fmt::format(
				"Added {} ({}) to {}.",
				item_link,
				item_id,
				c->GetTargetDescription(target)
			).c_str()
		);
	} else if (is_money) {
		if (!c->GetTarget()->IsNPC()) {
			c->Message(Chat::White, "You must target an NPC to use this command.");
			return;
		}

		auto target = c->GetTarget()->CastToNPC();

		if (sep->IsNumber(2)) {
			uint16 platinum = EQ::Clamp(Strings::ToInt(sep->arg[2]), 0, 65535);
			uint16 gold = sep->IsNumber(3) ? EQ::Clamp(Strings::ToInt(sep->arg[3]), 0, 65535) : 0;
			uint16 silver = sep->IsNumber(4) ? EQ::Clamp(Strings::ToInt(sep->arg[4]), 0, 65535) : 0;
			uint16 copper = sep->IsNumber(5) ? EQ::Clamp(Strings::ToInt(sep->arg[5]), 0, 65535) : 0;
			target->AddLootCash(
				copper,
				silver,
				gold,
				platinum
			);

			auto money_string = (
				(
					copper ||
					silver ||
					gold ||
					platinum
				) ?
					Strings::Money(
						platinum,
						gold,
						silver,
						copper
					) :
				"no money"
			);

			c->Message(
				Chat::White,
				fmt::format(
					"{} now has {}.",
					c->GetTargetDescription(target),
					money_string
				).c_str()
			);
		} else {
			c->Message(Chat::White, "Usage: #npcloot money [Platinum] [Gold] [Silver] [Copper] - Set an NPC's current money");
		}
	} else if (is_remove) {
		if (!c->GetTarget()->IsNPC()) {
			c->Message(Chat::White, "You must target an NPC to use this command.");
			return;
		}

		auto target = c->GetTarget()->CastToNPC();

		bool is_remove_all = !strcasecmp(sep->arg[2], "all");
		if (is_remove_all) {
			auto loot_list = target->GetLootList();
			auto total_item_count = 0;
			for (const auto& item_id : loot_list) {
				auto item_count = target->CountItem(item_id);

				target->RemoveItem(item_id);

				c->Message(
					Chat::White,
					fmt::format(
						"Removed {} {} ({}) from {}.",
						item_count,
						database.CreateItemLink(item_id),
						item_id,
						c->GetTargetDescription(target)
					).c_str()
				);

				total_item_count += item_count;
			}

			if (!total_item_count) {
				c->Message(
					Chat::White,
					fmt::format(
						"{} has no items to remove.",
						c->GetTargetDescription(target)
					).c_str()
				);
			} else {
				c->Message(
					Chat::White,
					fmt::format(
						"{} Item{} removed from {}.",
						total_item_count,
						total_item_count != 1 ? "s" : "",
						c->GetTargetDescription(target)
					).c_str()
				);
			}
		} else {
			if (sep->IsNumber(2)) {
				auto item_id = Strings::ToUnsignedInt(sep->arg[2]);
				auto item_count = target->CountItem(item_id);
				if (item_count) {
					target->RemoveItem(item_id);
					c->Message(
						Chat::White,
						fmt::format(
							"Removed {} {} ({}) from {}.",
							item_count,
							database.CreateItemLink(item_id),
							item_id,
							c->GetTargetDescription(target)
						).c_str()
					);
				} else {
					c->Message(
						Chat::White,
						fmt::format(
							"{} does not have any {} ({}).",
							c->GetTargetDescription(target),
							database.CreateItemLink(item_id),
							item_id
						).c_str()
					);
				}
			} else {
				c->Message(Chat::White, "Usage: #npcloot remove [All|Item ID] - Remove loot from an NPC by ID or remove all loot");
			}
		}
	} else if (is_show) {
		if (c->GetTarget()->IsNPC()) {
			c->GetTarget()->CastToNPC()->QueryLoot(c);
		} else if (c->GetTarget()->IsCorpse()) {
			c->GetTarget()->CastToCorpse()->QueryLoot(c);
		}
	}
}

