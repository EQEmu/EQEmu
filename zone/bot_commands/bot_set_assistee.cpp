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

void bot_command_set_assistee(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_set_assistee", sep->arg[0], "setassistee")) {
		c->Message(Chat::White, "note: Sets your bots to assist your target in addition to yourself.");

		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		BotCommandHelpParams p;

		p.description = { "Sets your bots to assist your target in addition to yourself." };
		p.notes =
		{
			"- Your target must be another player in your group or raid.",
			"- This needs to be set on every zone/camp you do.",
			"- If a Raid or Group assist is set and you do not want your bots to auto assist that person, set yourself as the assistee."
		};

		std::string popup_text = c->SendBotCommandHelpWindow(p);
		popup_text = DialogueWindow::Table(popup_text);

		c->SendPopupToClient(sep->arg[0], popup_text.c_str());

		return;
	}

	Mob* assistee = c->GetTarget();

	if (assistee && assistee->IsClient() && c->IsInGroupOrRaid(assistee)) {
		c->SetAssistee(assistee->CastToClient()->CharacterID());
		c->Message(Chat::Green, "Your bots will now assist %s.", assistee->GetCleanName());
		
		return;
	}

	c->Message(Chat::Yellow, "You can only set your bots to assist clients that are in your group or raid.");

	return;
}
