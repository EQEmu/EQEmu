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
