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

void bot_command_actionable(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_actionable", sep->arg[0], "actionable")) {
		c->Message(Chat::White, "note: Lists actionable command arguments and use descriptions");

		return;
	}

	BotCommandHelpParams p;

	p.description = { "Lists actionable command arguments and use descriptions." };
	p.notes = {
		"[target] - uses the command on the target. Some commands will default to target if no actionable is selected.",
		"[byname] [name] - selects a bot by name their name.",
		"[ownergroup] - selects all bots in the owner's group.",
		"[ownerraid] - selects all bots in the owner's raid.",
		"[targetgroup] - selects all bots in the target's group.",
		"[namesgroup] [name] - selects all bots in [name]'s group.",
		"[healrotation] [name] - selects all member and target bots of a heal rotation where [name] is a member.",
		"[healrotationmembers] [name] - selects all member bots of a heal rotation where [name] is a member.",
		"[healrotationtargets] [name] - selects all target bots of a heal rotation where [name] is a member.",
		"[mmr] - selects all bots that are currently at max melee range.",
		"[byclass] - selects all bots of the chosen class.",
		"[byrace] - selects all bots of the chosen race.",
		"[spawned] - selects all spawned bots.",
		"[all] - selects all spawned bots.",
		"<br>",
		"You may only select your own bots."
	};

	std::string popup_text = c->SendBotCommandHelpWindow(p);
	popup_text = DialogueWindow::Table(popup_text);

	c->SendPopupToClient(sep->arg[0], popup_text.c_str());

	return;
}
