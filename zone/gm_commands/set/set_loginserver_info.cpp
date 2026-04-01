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

void SetLoginserverInfo(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 3) {
		c->Message(Chat::White, "Usage: #set lsinfo [Email] [Password]");
		return;
	}

	const std::string& email    = sep->arg[2];
	const std::string& password = sep->arg[3];

	auto pack = new ServerPacket(ServerOP_LSAccountUpdate, sizeof(LoginserverAccountUpdate));

	auto s = (LoginserverAccountUpdate *) pack->pBuffer;

	s->user_account_id = c->LSAccountID();
	strn0cpy(s->user_account_name, c->AccountName(), 30);
	strn0cpy(s->user_email, email.c_str(), 100);
	strn0cpy(s->user_account_password, password.c_str(), 50);

	worldserver.SendPacket(pack);
	safe_delete(pack);

	c->Message(Chat::White, "Your email and local loginserver password have been set.");
}
