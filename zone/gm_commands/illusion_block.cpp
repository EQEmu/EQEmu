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
#include "zone/client.h"
#include "zone/dialogue_window.h"

void command_illusion_block(Client* c, const Seperator* sep)
{
	int arguments = sep->argnum;
	if (!arguments || !strcasecmp(sep->arg[1], "help")) {
		BotCommandHelpParams p;

		p.description = { "Toggles whether or not you will block the illusion effects of spells cast by players or bots." };
		p.notes = {
			"- This will still allow other portions of a spell to land on you.",
			"- Spells cast by NPCs will ignore this setting and land as normal."
		};
		p.example_format = { fmt::format("{} [value]", sep->arg[0]) };
		p.examples_one =
		{
			"To enable illusion block:",
			fmt::format(
				"{} 1",
				sep->arg[0]
			)
		};
		p.examples_two =
		{
			"To disable illusion block:",
			fmt::format(
				"{} 0",
				sep->arg[0]
			)
		};

		std::string popup_text = c->SendBotCommandHelpWindow(p);
		popup_text = DialogueWindow::Table(popup_text);

		c->SendPopupToClient(sep->arg[0], popup_text.c_str());

		return;
	}

	if (sep->IsNumber(1)) {
		int set_status = atoi(sep->arg[1]);
		if (EQ::ValueWithin(set_status, 0, 1)) {
			c->SetIllusionBlock(set_status);
			c->Message(Chat::Green, "Your Illusion Block has been %s.", (set_status ? "enabled" : "disabled"));
		}
		else {
			c->Message(Chat::Yellow, "You must enter 0 for disabled or 1 for enabled.");
			return;
		}
	}
	else if (!strcasecmp(sep->arg[1], "current")) {
		c->Message(Chat::Green, "You're currently %s illusions.", (c->GetIllusionBlock() ? "blocking" : "allowing"));
	}
	else {
		c->Message(Chat::Yellow , "Incorrect argument, use %s help for a list of options.", sep->arg[0]);
	}
}
