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
#include "zonelist.h"

#include "queryserv/zoneserver.h"
#include "common/strings.h"

void ZSList::Add(ZoneServer* zoneserver) {
	zone_server_list.emplace_back(std::unique_ptr<ZoneServer>(zoneserver));
	zoneserver->SetIsZoneConnected(true);

	zoneserver->SendPlayerEventLogSettings();
}

void ZSList::Remove(const std::string &uuid)
{
	auto iter = zone_server_list.begin();
	while (iter != zone_server_list.end()) {
		if ((*iter)->GetUUID().compare(uuid) == 0) {
			zone_server_list.erase(iter);
			return;
		}
		iter++;
	}
}

void ZSList::SendPlayerEventLogSettings()
{
	for (auto &zs : zone_server_list) {
		zs->SendPlayerEventLogSettings();
	}
}
