#include "zone/client.h"

void command_disablerecipe(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #disablerecipe [Recipe ID]");
		return;
	}

	auto recipe_id = Strings::ToUnsignedInt(sep->arg[1]);
	if (!recipe_id) {
		c->Message(Chat::White, "Usage: #disablerecipe [Recipe ID]");
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Recipe ID {} {} disabled.",
			recipe_id,
			(
				content_db.DisableRecipe(recipe_id) ?
				"successfully" :
				"failed to be"
			)
		).c_str()
	);
}
