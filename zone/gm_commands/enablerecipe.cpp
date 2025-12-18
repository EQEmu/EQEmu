#include "zone/client.h"

void command_enablerecipe(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #enablerecipe [Recipe ID]");
		return;
	}

	auto recipe_id = Strings::ToUnsignedInt(sep->arg[1]);
	if (!recipe_id) {
		c->Message(Chat::White, "Usage: #enablerecipe [Recipe ID]");
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Recipe ID {} {} enabled.",
			recipe_id,
			(
				content_db.EnableRecipe(recipe_id) ?
				"successfully" :
				"failed to be"
			)
		).c_str()
	);
}
