#include "zone/client.h"

void command_dye(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;

	if (arguments == 0) {
		c->Message(Chat::White, "Command Syntax: #dye help | #dye [slot] [red] [green] [blue] [use_tint]");
		return;
	}

	uint8 slot     = 0;
	uint8 red      = 255;
	uint8 green    = 255;
	uint8 blue     = 255;
	uint8 use_tint = 255;

	std::vector<std::string> dye_slots = {
		"Helmet",
		"Chest",
		"Arms",
		"Wrist",
		"Hands",
		"Legs",
		"Feet"
	};

	if (arguments == 1 && !strcasecmp(sep->arg[1], "help")) {
		int                      slot_id = 0;
		std::vector<std::string> slot_messages;
		c->Message(Chat::White, "Command Syntax: #dye help | #dye [slot] [red] [green] [blue] [use_tint]");
		c->Message(Chat::White, "Red, Green, and Blue go from 0 to 255.");

		for (const auto &slot : dye_slots) {
			slot_messages.emplace_back(fmt::format("({}) {}", slot_id, slot));
			slot_id++;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"{} {}",
				"Slots are as follows:",
				Strings::Implode(", ", slot_messages)
			).c_str()
		);
		return;
	}

	if (arguments >= 1 && sep->IsNumber(1)) {
		slot = Strings::ToInt(sep->arg[1]);
	}

	if (arguments >= 2 && sep->IsNumber(2)) {
		red = Strings::ToInt(sep->arg[2]);
	}

	if (arguments >= 3 && sep->IsNumber(3)) {
		green = Strings::ToInt(sep->arg[3]);
	}

	if (arguments >= 4 && sep->IsNumber(4)) {
		blue = Strings::ToInt(sep->arg[4]);
	}

	if (arguments >= 5 && sep->IsNumber(5)) {
		use_tint = Strings::ToInt(sep->arg[5]);
	}

	if (RuleB(Command, DyeCommandRequiresDyes)) {
		uint32 dye_item_id = 32557;
		if (c->CountItem(dye_item_id) >= 1) {
			c->RemoveItem(dye_item_id);
		}
		else {
			EQ::SayLinkEngine linker;
			linker.SetLinkType(EQ::saylink::SayLinkItemData);
			const EQ::ItemData *dye_item = database.GetItem(dye_item_id);
			linker.SetItemData(dye_item);
			c->Message(Chat::White, fmt::format("This command requires a {} to use.", linker.GenerateLink()).c_str());
			return;
		}
	}

	c->DyeArmorBySlot(slot, red, green, blue, use_tint);
}

