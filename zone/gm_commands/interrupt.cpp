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

void command_interrupt(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;

	uint16 interrupt_message = 0x01b7, interrupt_color = 0x0121;

	if (arguments >= 1 && sep->IsNumber(1)) {
		interrupt_message = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[1]));
	}

	if (arguments == 2 && sep->IsNumber(2)) {
		interrupt_color = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[2]));
	}

	c->InterruptSpell(interrupt_message, interrupt_color);
}

