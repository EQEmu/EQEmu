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
#pragma once

#include "common/repositories/dynamic_zone_templates_repository.h"
#include "common/timer.h"

#include <memory>
#include <unordered_map>
#include <vector>

extern class DynamicZoneManager dynamic_zone_manager;

class DynamicZone;
struct DynamicZoneMember;
class ServerPacket;

class DynamicZoneManager
{
public:
	DynamicZoneManager();

	void CacheAllFromDatabase();
	void CacheNewDynamicZone(ServerPacket* pack);
	DynamicZone* TryCreate(DynamicZone& dz_request, const std::vector<DynamicZoneMember>& members);
	void HandleZoneMessage(ServerPacket* pack);
	void LoadTemplates();
	void Process();
	void PurgeExpiredDynamicZones();
	const auto& GetTemplates() const { return m_dz_templates; }

	std::unordered_map<uint32_t, std::unique_ptr<DynamicZone>> dynamic_zone_cache;

private:
	void SendBulkMemberStatuses(uint32_t zone_id, uint16_t inst_id);

	Timer m_process_throttle_timer{};
	std::unordered_map<uint32_t, DynamicZoneTemplatesRepository::DynamicZoneTemplates> m_dz_templates;
};
