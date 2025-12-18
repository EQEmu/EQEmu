#include "zone/client.h"

void command_entityvariable(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;

	if (!arguments) {
		c->Message(Chat::White, "Usage: #entityvariable clear - Clear all entity variables on yourself or your target");
		c->Message(Chat::White, "Usage: #entityvariable delete [Variable Name] - Delete an entity variable from yourself or your target");
		c->Message(Chat::White, "Usage: #entityvariable set [Variable Name] [Variable Value] - Set an entity variable for yourself or your target");
		c->Message(Chat::White, "Usage: #entityvariable view [Variable Name] - View an entity variable on yourself or your target");
		c->Message(Chat::White, "Note: You can have spaces in variable names and values by wrapping in double quotes like this");
		c->Message(Chat::White, "Example: #entityvariable set \"Test Variable\" \"Test Value\"");
		c->Message(Chat::White, "Note: Variable Value is optional and can be set to empty by not providing a value");
		return;
	}

	const bool is_clear  = !strcasecmp(sep->arg[1], "clear");
	const bool is_delete = !strcasecmp(sep->arg[1], "delete");
	const bool is_set    = !strcasecmp(sep->arg[1], "set");
	const bool is_view   = !strcasecmp(sep->arg[1], "view");

	if (
		!is_clear &&
		!is_delete &&
		!is_set &&
		!is_view
	) {
		c->Message(Chat::White, "Usage: #entityvariable clear - Clear all entity variables on yourself or your target");
		c->Message(Chat::White, "Usage: #entityvariable delete [Variable Name] - Delete an entity variable from yourself or your target");
		c->Message(Chat::White, "Usage: #entityvariable set [Variable Name] [Variable Value] - Set an entity variable for yourself or your target");
		c->Message(Chat::White, "Usage: #entityvariable view [Variable Name] - View an entity variable on yourself or your target");
		c->Message(Chat::White, "Note: You can have spaces in variable names and values by wrapping in double quotes like this");
		c->Message(Chat::White, "Example: #entityvariable set \"Test Variable\" \"Test Value\"");
		c->Message(Chat::White, "Note: Variable Value is optional and can be set to empty by not providing a value");
		return;
	}

	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	if (is_clear) {
		const auto cleared = t->ClearEntityVariables();

		if (!cleared) {
			c->Message(
				Chat::White,
				fmt::format(
					"{} {} not have any entity variables to clear.",
					c->GetTargetDescription(t, TargetDescriptionType::UCYou),
					c == t ? "do" : "does"
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Cleared all entity variables for {}.",
				c->GetTargetDescription(t)
			).c_str()
		);
	} else if (is_delete) {
		const std::string variable_name = sep->argplus[2];

		if (!t->EntityVariableExists(variable_name)) {
			c->Message(
				Chat::White,
				fmt::format(
					"{} {} not have an entity variable named '{}'.",
					c->GetTargetDescription(t, TargetDescriptionType::UCYou),
					c == t ? "do" : "does",
					variable_name
				).c_str()
			);

			return;
		}

		t->DeleteEntityVariable(variable_name);

		c->Message(
			Chat::White,
			fmt::format(
				"Deleted an entity variable named '{}' from {}.",
				variable_name,
				c->GetTargetDescription(t)
			).c_str()
		);

		return;
	} else if (is_set) {
		const std::string variable_name  = sep->arg[2];
		const std::string variable_value = sep->arg[3];

		t->SetEntityVariable(variable_name, variable_value);

		c->Message(
			Chat::White,
			fmt::format(
				"Set an entity variable named '{}' to a value of '{}' for {}.",
				variable_name,
				variable_value,
				c->GetTargetDescription(t)
			).c_str()
		);
	} else if (is_view) {
		const auto &l = t->GetEntityVariables();

		uint32 variable_count  = 0;
		uint32 variable_number = 1;
		const std::string search_criteria = arguments ? sep->argplus[2] : "";

		for (const auto &e: l) {
			if (
				search_criteria.empty() ||
				Strings::Contains(Strings::ToLower(e), Strings::ToLower(search_criteria))
			) {
				c->Message(
					Chat::White,
					fmt::format(
						"Variable {} | Name: {} Value: {} | {}",
						variable_number,
						e,
						t->GetEntityVariable(e),
						Saylink::Silent(
							fmt::format(
								"#entityvariable delete {}",
								e
							),
							"Delete"
						)
					).c_str()
				);

				variable_count++;
				variable_number++;
			}
		}

		if (!variable_count) {
			c->Message(
				Chat::White,
				fmt::format(
					"{} {} no entity variables{}.",
					c->GetTargetDescription(t, TargetDescriptionType::UCYou),
					c == t ? "have" : "has",
					(
						!search_criteria.empty() ?
							fmt::format(
								" matching '{}'",
								search_criteria
							) :
							""
					)
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"{} {} {} entity variable{}{}, would you like to {} all of {} entity variables?",
				c->GetTargetDescription(t, TargetDescriptionType::UCYou),
				c == t ? "have" : "has",
				variable_count,
				variable_count != 1 ? "s" : "",
				(
					!search_criteria.empty() ?
						fmt::format(
							" matching '{}'",
							search_criteria
						) :
						""
				),
				Saylink::Silent(
					"#entityvariable clear",
					"clear"
				),
				c == t ? "your" : "their"
			).c_str()
		);
	}
}
