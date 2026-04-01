/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "zone/bot_command.h"

void bot_command_find_aliases(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_find_aliases", sep->arg[0], "findaliases"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s [alias | command]", sep->arg[0]);
		return;
	}

	auto find_iter = bot_command_aliases.find(sep->arg[1]);
	if (find_iter == bot_command_aliases.end()) {
		c->Message(Chat::White, "No bot commands or aliases match '%s'", sep->arg[1]);
		return;
	}

	auto command_iter = bot_command_list.find(find_iter->second);
	if (find_iter->second.empty() || command_iter == bot_command_list.end()) {
		c->Message(Chat::White, "An unknown condition has occurred...");
		return;
	}

	c->Message(Chat::White, "Available bot command aliases for '%s':", command_iter->first.c_str());

	int bot_command_aliases_shown = 0;
	for (auto alias_iter : bot_command_aliases) {
		if (strcasecmp(find_iter->second.c_str(), alias_iter.second.c_str()) || c->Admin() < command_iter->second->access)
			continue;

		c->Message(
			Chat::White,
			fmt::format(
				"^{}",
				alias_iter.first
			).c_str()
		);

		++bot_command_aliases_shown;
	}
	c->Message(Chat::White, "%d bot command alias%s listed.", bot_command_aliases_shown, bot_command_aliases_shown != 1 ? "es" : "");
}
