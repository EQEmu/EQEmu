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
#include "common/repositories/account_repository.h"
#include "zone/client.h"
#include "zone/worldserver.h"

extern WorldServer worldserver;

void SetChecksum(Client *c, const Seperator *sep)
{
	auto account = AccountRepository::FindOne(database, c->AccountID());
	if (!account.id) {
		c->Message(Chat::White, "Your account was not found!");
		return;
	}

	database.SetVariable("crc_eqgame", account.crc_eqgame);
	database.SetVariable("crc_skillcaps", account.crc_skillcaps);
	database.SetVariable("crc_basedata", account.crc_basedata);

	c->Message(Chat::White, "Attempting to reload Rules globally.");
	worldserver.SendReload(ServerReload::Type::Rules);

	c->Message(Chat::White, "Attempting to reload Variables globally.");
	worldserver.SendReload(ServerReload::Type::Variables);
}
