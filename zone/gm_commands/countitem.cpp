#include "zone/client.h"

void command_countitem(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #countitem [Item ID]");
		return;
	}

	Mob* target = c;
	if (
		c->GetTarget() &&
		(
			c->GetTarget()->IsClient() ||
			c->GetTarget()->IsNPC()
		)
	) {
		target = c->GetTarget();
	}

	auto item_id = Strings::ToUnsignedInt(sep->arg[1]);
	if (!database.GetItem(item_id)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Item ID {} could not be found.",
				item_id
			).c_str()
		);
		return;
	}

	uint16 item_count = 0;
	if (target->IsClient()) {
		item_count = target->CastToClient()->CountItem(item_id);
	} else if (target->IsNPC()) {
		item_count = target->CastToNPC()->CountItem(item_id);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} {} {} {}.",
			c->GetTargetDescription(target, TargetDescriptionType::UCYou),
			c == target ? "have" : "has",
			(
				item_count ?
				std::to_string(item_count) :
				"no"
			),
			database.CreateItemLink(item_id)
		).c_str()
	);
}

