#include "../../client.h"
#include "../../titles.h"

void SetTitleSuffix(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #set title_suffix [Title Suffix]");
		c->Message(Chat::White, "Note: Use \"-1\" to remove title.");
		return;
	}

	const bool is_remove = !strcasecmp(sep->argplus[2], "-1");
	std::string suffix = is_remove ? "" : sep->argplus[2];

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	if (suffix.size() > 31) {
		c->Message(Chat::White, "Title suffix must be 31 characters or less.");
		return;
	}

	if (!suffix.empty()) {
		Strings::FindReplace(suffix, "_", " ");
	}

	if (is_remove) {
		t->SetTitleSuffix(suffix);
	} else {
		title_manager.CreateNewPlayerSuffix(t, suffix);
	}

	t->Save();

	c->Message(
		Chat::White,
		fmt::format(
			"Title suffix has been {}{} for {}{}",
			is_remove ? "removed" : "changed",
			!is_remove ? " and saved" : "",
			c->GetTargetDescription(t),
			(
				is_remove ?
				"." :
				fmt::format(
					" to '{}'.",
					suffix
				)
			)
		).c_str()
	);
}
