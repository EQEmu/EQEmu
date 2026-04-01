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
#include "common/data_bucket.h"
#include "zone/client.h"

void command_devtools(Client *c, const Seperator *sep)
{
	const uint16 arguments = sep->argnum;
	if (arguments != 2) {
		c->ShowDevToolsMenu();
		return;
	}

	const std::string& type = sep->arg[1];
	const bool toggle = Strings::ToBool(sep->arg[2]);

	if (Strings::EqualFold(type, "menu")) {
		c->SetDevToolsEnabled(toggle);
	} else if (Strings::EqualFold(type, "window")) {
		c->SetDisplayMobInfoWindow(toggle);
	}

	c->ShowDevToolsMenu();
}

