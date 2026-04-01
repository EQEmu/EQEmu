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

void CatchSignal(int sig_num);

void command_shutdown(Client *c, const Seperator *sep)
{
	const int arguments = sep->argnum;
	if (!arguments) {
		c->Message(
			Chat::White,
			fmt::format(
				"Using this command will shut down your current zone. Please {} this is what you want to do.",
				Saylink::Silent("#shutdown confirm", "confirm")
			).c_str()
		);
		return;
	}

	const bool is_confirm = !strcasecmp(sep->arg[1], "confirm");

	if (is_confirm) {
		c->Message(Chat::White, "Shutting down your current zone.");
		CatchSignal(2);
	}
}
