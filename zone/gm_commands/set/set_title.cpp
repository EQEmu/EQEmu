#include "../../client.h"
#include "../../titles.h"

void SetTitle(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #set title [Title]");
		c->Message(Chat::White, "Note: Use \"-1\" to remove title.");
		return;
	}

	const bool is_remove = Strings::EqualFold(sep->argplus[2], "-1");
	std::string title = !is_remove ? sep->argplus[2] : "";

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	if (title.size() > 31) {
		c->Message(Chat::White, "Title must be 31 characters or less.");
		return;
	}

	if (!title.empty()) {
		Strings::FindReplace(title, "_", " ");
	}

	if (is_remove) {
		t->SetAATitle(title);
	} else {
		title_manager.CreateNewPlayerTitle(t, title);
	}

	t->Save();

	c->Message(
		Chat::White,
		fmt::format(
			"Title has been {}{} for {}{}",
			is_remove ? "removed" : "changed",
			!is_remove ? " and saved" : "",
			c->GetTargetDescription(t),
			(
				is_remove ?
				"." :
				fmt::format(
					" to '{}'.",
					title
				)
			)
		).c_str()
	);
}
