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

void command_ipban(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #ipban [IP]");
		return;
	}

	std::string ip = sep->arg[1];
	if (ip.empty()) {
		c->Message(Chat::White, "Usage: #ipban [IP]");
		return;
	}

	if (database.AddBannedIP(ip, c->GetName())) {
		c->Message(
			Chat::White,
			fmt::format(
				"IP '{}' has been successfully banned.",
				ip
			).c_str()
		);
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"IP '{}' has failed to be banned, the IP address may already be in the table.",
				ip
			).c_str()
		);
	}
}

