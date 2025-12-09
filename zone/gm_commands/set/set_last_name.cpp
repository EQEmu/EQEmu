#include "../../client.h"

void SetLastName(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #set lastname [Last Name]");
		c->Message(Chat::White, "Note: Use \"-1\" to remove last name.");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	LogInfo("#lastname request from [{}] for [{}]", c->GetCleanName(), t->GetCleanName());

	const bool is_remove = Strings::EqualFold(sep->argplus[2], "-1");
	const std::string& last_name = !is_remove ? sep->argplus[2] : "";

	if (last_name.size() > 64) {
		c->Message(Chat::White, "Last name must be 64 characters or less.");
		return;
	}

	t->ChangeLastName(last_name);

	c->Message(
		Chat::White,
		fmt::format(
			"Last name has been {}{} for {}{}",
			is_remove ? "removed" : "changed",
			!is_remove ? " and saved" : "",
			c->GetTargetDescription(t),
			(
				is_remove ?
				"." :
				fmt::format(
					" to '{}'.",
					last_name
				)
			)
		).c_str()
	);
}
