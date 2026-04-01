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

#include "database.h"

#include "common/eqemu_logsys.h"
#include "common/strings.h"

// this function does not delete the ServerPacket, so it must be handled at call site
void QSDatabase::GeneralQueryReceive(ServerPacket *pack)
{
	/*
		These are general queries passed from anywhere in zone instead of packing structures and breaking them down again and again
	*/
	auto queryBuffer = new char[pack->ReadUInt32() + 1];
	pack->ReadString(queryBuffer);

	std::string query(queryBuffer);
	auto        results = QueryDatabase(query);
	if (!results.Success()) {
		LogInfo("Failed Delete Log Record Insert: [{}]", results.ErrorMessage().c_str());
		LogInfo("[{}]", query.c_str());
	}

	safe_delete_array(queryBuffer);
}
