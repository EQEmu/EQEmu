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
#include "zone.h"

BaseDataRepository::BaseData Zone::GetBaseData(uint8 level, uint8 class_id)
{
	for (const auto& e : m_base_data) {
		if (e.level == level && e.class_ == class_id) {
			return e;
		}
	}

	return BaseDataRepository::NewEntity();
}

void Zone::LoadBaseData()
{
	const auto& l = BaseDataRepository::All(content_db);

	m_base_data.reserve(l.size());

	for (const auto& e : l) {
		if (e.level < 1 || !IsPlayerClass(e.class_)) {
			continue;
		}

		m_base_data.emplace_back(e);
	}

	LogInfo(
		"Loaded [{}] Base Data Entr{}",
		l.size(),
		l.size() != 1 ? "ies" : "y"
	);
}

void Zone::ReloadBaseData()
{
	ClearBaseData();
	LoadBaseData();
}
