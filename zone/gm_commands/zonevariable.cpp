#include "zone/client.h"

void command_zonevariable(Client *c, const Seperator *sep)
{
	const uint16 arguments = sep->argnum;

	if (!arguments) {
		c->Message(Chat::White, "Usage: #zonevariable clear - Clear all zone variables");
		c->Message(Chat::White, "Usage: #zonevariable delete [Variable Name] - Delete a zone variable");
		c->Message(Chat::White, "Usage: #zonevariable set [Variable Name] [Variable Value] - Set a zone variable");
		c->Message(Chat::White, "Usage: #zonevariable view [Variable Name] - View a zone variable");
		c->Message(Chat::White, "Note: You can have spaces in variable names and values by wrapping in double quotes like this");
		c->Message(Chat::White, "Example: #zonevariable set \"Test Variable\" \"Test Value\"");
		c->Message(Chat::White, "Note: Variable Value is optional and can be set to empty by not providing a value");
		return;
	}

	const char* action = arguments >= 1 ? sep->arg[1] : "";
	const bool is_clear  = !strcasecmp(action, "clear");
	const bool is_delete = !strcasecmp(action, "delete");
	const bool is_set    = !strcasecmp(action, "set");
	const bool is_view   = !strcasecmp(action, "view");

	if (!is_clear && !is_delete && !is_set && !is_view) {
		c->Message(Chat::White, "Usage: #zonevariable clear - Clear all zone variables");
		c->Message(Chat::White, "Usage: #zonevariable delete [Variable Name] - Delete a zone variable");
		c->Message(Chat::White, "Usage: #zonevariable set [Variable Name] [Variable Value] - Set a zone variable");
		c->Message(Chat::White, "Usage: #zonevariable view [Variable Name] - View a zone variable");
		c->Message(Chat::White, "Note: You can have spaces in variable names and values by wrapping in double quotes like this");
		c->Message(Chat::White, "Example: #zonevariable set \"Test Variable\" \"Test Value\"");
		c->Message(Chat::White, "Note: Variable Value is optional and can be set to empty by not providing a value");
		return;
	}

	if (is_clear) {
		const bool cleared = zone->ClearVariables();
		c->Message(Chat::White, cleared ? "Cleared all zone variables." : "There are no zone variables to clear.");
		return;
	}

	if (is_delete) {
		const std::string variable_name = arguments >= 2 ? sep->argplus[2] : "";
		if (variable_name.empty() || !zone->VariableExists(variable_name)) {
			c->Message(Chat::White, fmt::format("A zone variable named '{}' does not exist.", variable_name).c_str());
			return;
		}

		zone->DeleteVariable(variable_name);
		c->Message(Chat::White, fmt::format("Deleted a zone variable named '{}'.", variable_name).c_str());
		return;
	}

	if (is_set) {
		const std::string variable_name  = arguments >= 2 ? sep->arg[2] : "";
		const std::string variable_value = arguments >= 3 ? sep->arg[3] : "";
		zone->SetVariable(variable_name, variable_value);
		c->Message(Chat::White, fmt::format("Set a zone variable named '{}' to a value of '{}'.", variable_name, variable_value).c_str());
		return;
	}

	if (is_view) {
		const auto& l = zone->GetVariables();
		const std::string search_criteria = arguments >= 2 ? sep->argplus[2] : "";

		uint32 variable_count  = 0;
		uint32 variable_number = 1;

		for (const auto& e : l) {
			if (search_criteria.empty() || Strings::Contains(Strings::ToLower(e), Strings::ToLower(search_criteria))) {
				c->Message(Chat::White, fmt::format(
					"Variable {} | Name: {} Value: {} | {}",
					variable_number,
					e,
					zone->GetVariable(e),
					Saylink::Silent(fmt::format("#zonevariable delete {}", e), "Delete")
				).c_str());

				variable_count++;
				variable_number++;
			}
		}

		if (!variable_count) {
			c->Message(Chat::White, fmt::format(
				"There are no zone variables{}.",
				(!search_criteria.empty() ? fmt::format(" matching '{}'", search_criteria) : "")
			).c_str());
			return;
		}

		c->Message(Chat::White, fmt::format(
			"There {} {} zone variable{}{}, would you like to {} zone variables?",
			variable_count != 1 ? "are" : "is",
			variable_count,
			variable_count != 1 ? "s" : "",
			(!search_criteria.empty() ? fmt::format(" matching '{}'", search_criteria) : ""),
			Saylink::Silent("#zonevariable clear", "clear")
		).c_str());
	}
}
