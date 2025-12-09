#include "../../client.h"
#include "../../quest_parser_collection.h"

void ShowQuestErrors(Client *c, const Seperator *sep)
{
	std::list<std::string> l;
	parse->GetErrors(l);

	if (!l.size()) {
		c->Message(Chat::White, "There are no Quest errors currently.");
		return;
	}

	c->Message(Chat::White, "Quest errors currently are as follows:");

	uint32 error_count = 0;

	for (const auto& e : l) {
		if (error_count >= RuleI(World, MaximumQuestErrors)) {
			c->Message(
				Chat::White,
				fmt::format(
					"Maximum of {} error{} shown.",
					RuleI(World, MaximumQuestErrors),
					RuleI(World, MaximumQuestErrors) != 1 ? "s" : ""
				).c_str()
			);
			break;
		}

		c->Message(Chat::White, e.c_str());

		error_count++;
	}
}
