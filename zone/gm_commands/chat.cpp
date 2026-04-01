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
#include "zone/worldserver.h"

extern WorldServer worldserver;

void command_chat(Client *c, const Seperator *sep)
{
	auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #chat [Channel ID] [Message]");
		return;
	}

	auto channel_id = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[1]));
	std::string message = sep->argplus[2];
	if (!worldserver.SendChannelMessage(0, 0, channel_id, 0, Language::CommonTongue, Language::MaxValue, message.c_str())) {
		c->Message(Chat::White, "World server is disconnected.");
	}
}

