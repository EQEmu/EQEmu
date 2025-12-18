#include "zone/client.h"

void command_removeitem(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #removeitem [Item ID] [Amount]");
		return;
	}

	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	auto item_id = Strings::ToInt(sep->arg[1]);
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

	auto item_link = database.CreateItemLink(item_id);
	auto amount = sep->IsNumber(2) ? Strings::ToUnsignedInt(sep->arg[2]) : 1;
	auto item_count = target->CountItem(item_id);
	if (item_count) {
		if (item_count >= amount) {
			target->RemoveItem(item_id, amount);

			c->Message(
				Chat::White,
				fmt::format(
					"Removed {} {} ({}) from {}.",
					amount,
					item_link,
					item_id,
					c->GetTargetDescription(target)
				).c_str()
			);
		} else {
			target->RemoveItem(item_id, item_count);

			c->Message(
				Chat::White,
				fmt::format(
					"Removed {} {} ({}) from {} because {} did not have {} {} ({}).",
					item_count,
					item_link,
					item_id,
					c->GetTargetDescription(target),
					c == target ? "you" : "they",
					amount,
					item_link,
					item_id
				).c_str()
			);
		}
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"Could not find any {} ({}) to delete from {}.",
				database.CreateItemLink(item_id),
				item_id,
				c->GetTargetDescription(target)
			).c_str()
		);
	}
}

