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
#include "eql_config.h"

#include "common/strings.h"
#include "common/zone_store.h"
#include "world/launcher_link.h"
#include "world/launcher_list.h"
#include "world/worlddb.h"

#include <cstdlib>
#include <cstring>

extern LauncherList launcher_list;

EQLConfig::EQLConfig(const char *launcher_name)
: m_name(launcher_name)
{
	LoadSettings();
}

void EQLConfig::LoadSettings() {

	LauncherZone tmp;
	char namebuf[128];
	database.DoEscapeString(namebuf, m_name.c_str(), m_name.length()&0x3F);	//limit len to 64
	namebuf[127] = '\0';

    std::string query = StringFormat("SELECT dynamics FROM launcher WHERE name = '%s'", namebuf);
    auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		LogError("EQLConfig::LoadSettings: {}", results.ErrorMessage().c_str());
	}
    else {
        auto row = results.begin();
        m_dynamics = Strings::ToInt(row[0]);
    }

	query = StringFormat("SELECT zone, port FROM launcher_zones WHERE launcher = '%s'", namebuf);
	results = database.QueryDatabase(query);
	if (!results.Success()) {
        LogError("EQLConfig::LoadSettings: {}", results.ErrorMessage().c_str());
        return;
    }

    LauncherZone zs;
    for (auto row = results.begin(); row != results.end(); ++row) {
        zs.name = row[0];
		zs.port = Strings::ToInt(row[1]);
		m_zones[zs.name] = zs;
    }

}

void EQLConfig::GetZones(std::vector<LauncherZone> &result) {
	std::map<std::string, LauncherZone>::iterator cur, end;
	cur = m_zones.begin();
	end = m_zones.end();
	for(; cur != end; ++cur) {
		result.push_back(cur->second);
	}
}

void EQLConfig::StartZone(Const_char *zone_ref) {
	LauncherLink *ll = launcher_list.Get(m_name.c_str());
	if(ll == nullptr)
		return;
	ll->StartZone(zone_ref);
}

int EQLConfig::GetDynamicCount() const {
	return(m_dynamics);
}
