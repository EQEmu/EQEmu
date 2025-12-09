#include "../../client.h"

void SetTemporaryName(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #set temporary_name [New Name]");
		c->Message(Chat::White, "Note: This command requires a target, even if the target is yourself.");
		return;
	}

	if (!c->GetTarget()) {
		c->Message(Chat::White, "Usage: #set temporary_name [New Name]");
		c->Message(Chat::White, "Note: This command requires a target, even if the target is yourself.");
		return;
	}

	Mob* t = c->GetTarget();

	const std::string& temporary_name = strlen(sep->arg[2]) ? sep->arg[2] : "";

	t->TempName(temporary_name.c_str());

	if (temporary_name.length()) {
		const std::string& current_name   = t->GetName();

		c->Message(
			Chat::White,
			fmt::format(
				"Renamed {} to {} temporarily.",
				current_name,
				temporary_name
			).c_str()
		);
	} else {
		c->Message(Chat::White, "Restored your target's original name.");
	}
}
