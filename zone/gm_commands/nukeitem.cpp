#include "zone/client.h"

void command_nukeitem(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #nukeitem [Item ID] - Removes the specified Item ID from you or your player target's inventory");
		return;
	}

	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	auto item_id = Strings::ToInt(sep->arg[1]);
	auto deleted_count = target->NukeItem(item_id);
	if (deleted_count) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} ({}) deleted from {}.",
				deleted_count,
				database.CreateItemLink(item_id),
				item_id,
				c->GetTargetDescription(target)
			).c_str()
		);
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

