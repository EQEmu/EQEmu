#include "zone/client.h"

void command_mysql(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #mysql [Help|Query] [SQL Query]");
		return;
	}

	bool is_help = !strcasecmp(sep->arg[1], "help");
	bool is_query = !strcasecmp(sep->arg[1], "query");
	if (
		!is_help &&
		!is_query
	) {
		c->Message(Chat::White, "Usage: #mysql [Help|Query] [SQL Query]");
		return;
	}

	if (is_help) {
		c->Message(Chat::White, "Usage: #mysql query \"Query goes here quoted\"");
		c->Message(Chat::White, "Note: To use 'LIKE \"%%something%%\"  replace the %% with a #");
		c->Message(Chat::White, "Example: #mysql query \"SELECT * FROM items WHERE `name` LIKE \"#Apple#\"");
		return;
	} else if (is_query) {
		if (arguments < 2) {
			c->Message(Chat::White, "Usage: #mysql query \"Query goes here quoted\"");
			c->Message(Chat::White, "Note: To use 'LIKE \"%%something%%\"  replace the %% with a #");
			c->Message(Chat::White, "Example: #mysql query \"SELECT * FROM items WHERE `name` LIKE \"#Apple#\"");
			return;
		}

		std::string query = sep->arg[2];
		Strings::FindReplace(query, "#", "%");
		auto results = database.QueryDatabase(query);
		if (!results.Success()) {
			return;
		}

		query = sep->arg[2];
		Strings::FindReplace(query, "#", "%%");

		c->Message(
			Chat::White,
			fmt::format(
				"Running Query: '{}'",
				query
			).c_str()
		);

		std::vector<std::string> lines;
		for (auto row : results) {
			for (
				int row_index = 0;
				row_index < results.ColumnCount();
				row_index++
			) {
				lines.emplace_back(
					fmt::format(
						"{} | {} ",
						results.FieldName(row_index),
						(
							row[row_index] ?
							(
								strlen(row[row_index]) ?
								row[row_index] :
								"Empty String"
							) :
							"NULL"
						)
					)
				);
			}
		}

		for (auto line : lines) {
			c->Message(
				Chat::White,
				line.c_str()
			);
		}
	}
}

