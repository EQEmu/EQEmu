#include "../../client.h"

void SetName(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #set name [Name]");
		return;
	}

	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		auto t = c->GetTarget()->CastToClient();

		std::string new_name = sep->arg[2];
		std::string old_name = t->GetCleanName();

		if (t->ChangeFirstName(new_name, c->GetCleanName())) {
			c->Message(
				Chat::White,
				fmt::format(
					"Successfully renamed {} to {}",
					old_name,
					new_name
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Unable to rename {}. Check that the new name '{}' isn't already taken (Including Pet Names), or isn't invalid",
				old_name,
				new_name
			).c_str()
		);
	}
}
