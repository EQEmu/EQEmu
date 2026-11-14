#include "world/achievement_state_update_manager.h"

#include "common/eq_packet.h"
#include "common/eqemu_logsys.h"
#include "common/rulesys.h"
#include "common/servertalk.h"
#include "world/cliententry.h"
#include "world/clientlist.h"
#include "world/dynamic_zone.h"
#include "world/shared_task_manager.h"
#include "world/worlddb.h"
#include "world/zoneserver.h"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <fmt/format.h>
#include <string>
#include <utility>

static uint32_t ParseUInt32(const char *value)
{
	return value ? static_cast<uint32_t>(std::strtoul(value, nullptr, 10)) : 0;
}

static bool SameRequest(
	const AchievementStateUpdates::Request &left,
	const AchievementStateUpdates::Request &right
)
{
	return
		left.target_id == right.target_id &&
		left.achievement_id == right.achievement_id &&
		left.component_id == right.component_id &&
		left.value == right.value &&
		left.version == right.version &&
		left.target_type == right.target_type &&
		left.operation == right.operation &&
		left.component_type == right.component_type;
}

bool AchievementStateUpdateManager::Queue(const AchievementStateUpdates::Request &request)
{
	if (!AchievementStateUpdates::IsValidRequest(request)) {
		LogError("Rejected an invalid achievement state update request");
		return false;
	}

	std::vector<uint32_t> character_ids;
	const auto resolution = ResolveTargets(
		request.target_type,
		request.target_id,
		character_ids
	);
	if (resolution == TargetResolution::RetryableFailure) {
		LogError(
			"Deferring achievement state update after target type [{}], ID [{}] "
			"could not be resolved",
			static_cast<uint32_t>(request.target_type),
			request.target_id
		);
		return RetainForRetry(request);
	}

	NormalizeTargets(character_ids);
	if (
		resolution == TargetResolution::Missing ||
		character_ids.empty()
	) {
		LogError(
			"Achievement state update target type [{}], ID [{}] has no player members",
			static_cast<uint32_t>(request.target_type),
			request.target_id
		);
		return false;
	}

	if (!Persist(request, character_ids)) {
		return RetainForRetry(request, std::move(character_ids), true);
	}

	Wake(character_ids);
	return true;
}

AchievementStateUpdateManager::TargetResolution
AchievementStateUpdateManager::ResolveTargets(
	AchievementStateUpdates::TargetType target_type,
	uint64_t target_id,
	std::vector<uint32_t> &character_ids
) const
{
	using AchievementStateUpdates::TargetType;

	switch (target_type) {
	case TargetType::Character: {
		const auto character_id = static_cast<uint32_t>(target_id);
		auto result = database.QueryDatabase(fmt::format(
			"SELECT id FROM character_data WHERE id = {} LIMIT 1",
			character_id
		));
		if (!result.Success()) {
			return TargetResolution::RetryableFailure;
		}
		if (result.RowCount() == 1) {
			character_ids.push_back(character_id);
		}
		break;
	}
	case TargetType::Group: {
		auto result = database.QueryDatabase(fmt::format(
			"SELECT character_id FROM group_id "
			"WHERE group_id = {} AND character_id > 0 AND bot_id = 0 AND merc_id = 0",
			target_id
		));
		if (!result.Success()) {
			return TargetResolution::RetryableFailure;
		}
		for (auto row : result) {
			const auto character_id = ParseUInt32(row[0]);
			if (character_id) {
				character_ids.push_back(character_id);
			}
		}
		break;
	}
	case TargetType::Raid: {
		auto result = database.QueryDatabase(fmt::format(
			"SELECT charid FROM raid_members "
			"WHERE raidid = {} AND charid > 0 AND bot_id = 0",
			target_id
		));
		if (!result.Success()) {
			return TargetResolution::RetryableFailure;
		}
		for (auto row : result) {
			const auto character_id = ParseUInt32(row[0]);
			if (character_id) {
				character_ids.push_back(character_id);
			}
		}
		break;
	}
	case TargetType::DynamicZone: {
		const auto dynamic_zone = DynamicZone::FindDynamicZoneByID(
			static_cast<uint32_t>(target_id)
		);
		if (!dynamic_zone) {
			return TargetResolution::Missing;
		}
		for (const auto &member : dynamic_zone->GetMembers()) {
			if (member.id) {
				character_ids.push_back(member.id);
			}
		}

		std::vector<ClientListEntry *> clients;
		clients.reserve(ClientList::Instance()->GetClientCount());
		ClientList::Instance()->GetClients("", clients);
		for (const auto *client : clients) {
			if (
				client &&
				client->CharID() &&
				client->GetOnline() == CLE_Status::InZone &&
				dynamic_zone->IsSameDz(client->zone(), client->instance())
			) {
				character_ids.push_back(client->CharID());
			}
		}
		break;
	}
	case TargetType::SharedTask: {
		const auto shared_task = SharedTaskManager::Instance()->FindSharedTaskById(
			static_cast<int64_t>(target_id)
		);
		if (!shared_task) {
			return TargetResolution::Missing;
		}
		for (const auto &member : shared_task->GetMembers()) {
			if (member.character_id) {
				character_ids.push_back(member.character_id);
			}
		}
		break;
	}
	}

	return character_ids.empty()
		? TargetResolution::Missing
		: TargetResolution::Resolved;
}

void AchievementStateUpdateManager::NormalizeTargets(
	std::vector<uint32_t> &character_ids
)
{
	std::sort(character_ids.begin(), character_ids.end());
	character_ids.erase(
		std::unique(character_ids.begin(), character_ids.end()),
		character_ids.end()
	);
}

bool AchievementStateUpdateManager::RetainForRetry(
	const AchievementStateUpdates::Request &request,
	std::vector<uint32_t> character_ids,
	bool targets_resolved
)
{
	NormalizeTargets(character_ids);
	const auto duplicate = std::find_if(
		m_retry_requests.begin(),
		m_retry_requests.end(),
		[&](const RetryRequest &pending) {
			return
				pending.targets_resolved == targets_resolved &&
				SameRequest(pending.request, request) &&
				(!targets_resolved || pending.character_ids == character_ids);
		}
	);
	if (duplicate != m_retry_requests.end()) {
		return true;
	}

	constexpr size_t retry_capacity = 1024;
	if (m_retry_requests.size() >= retry_capacity) {
		LogError(
			"Achievement state update retry queue is full; dropping target type [{}], ID [{}]",
			static_cast<uint32_t>(request.target_type),
			request.target_id
		);
		return false;
	}

	m_retry_requests.push_back({
		.request = request,
		.character_ids = std::move(character_ids),
		.targets_resolved = targets_resolved
	});
	return true;
}

void AchievementStateUpdateManager::ProcessRetries()
{
	constexpr size_t retry_batch_size = 64;
	const auto request_count = std::min(
		m_retry_requests.size(),
		retry_batch_size
	);

	for (size_t i = 0; i < request_count; ++i) {
		auto pending = std::move(m_retry_requests.front());
		m_retry_requests.pop_front();

		if (!pending.targets_resolved) {
			const auto resolution = ResolveTargets(
				pending.request.target_type,
				pending.request.target_id,
				pending.character_ids
			);
			if (resolution == TargetResolution::RetryableFailure) {
				RetainForRetry(pending.request);
				continue;
			}

			NormalizeTargets(pending.character_ids);
			if (
				resolution == TargetResolution::Missing ||
				pending.character_ids.empty()
			) {
				LogError(
					"Discarding deferred achievement state update because target type "
					"[{}], ID [{}] no longer has player members",
					static_cast<uint32_t>(pending.request.target_type),
					pending.request.target_id
				);
				continue;
			}
			pending.targets_resolved = true;
		}

		if (!Persist(pending.request, pending.character_ids)) {
			RetainForRetry(
				pending.request,
				std::move(pending.character_ids),
				true
			);
			continue;
		}

		Wake(pending.character_ids);
	}
}

bool AchievementStateUpdateManager::Persist(
	const AchievementStateUpdates::Request &request,
	const std::vector<uint32_t> &character_ids
) const
{
	std::string values;
	for (const auto character_id : character_ids) {
		if (!values.empty()) {
			values += ", ";
		}
		values += fmt::format(
			"({}, {}, {}, {}, {}, {}, {}, {}, {}, {}, 0, UNIX_TIMESTAMP(), 0, '')",
			character_id,
			static_cast<uint32_t>(request.target_type),
			request.target_id,
			static_cast<uint32_t>(request.operation),
			request.achievement_id,
			static_cast<uint32_t>(request.component_type),
			request.component_id,
			request.value,
			request.version,
			static_cast<uint32_t>(AchievementStateUpdates::Status::Pending)
		);
	}

	if (!database.TransactionBeginStrict().Success()) {
		LogError("Failed to begin achievement state update transaction");
		return false;
	}

	const auto insert = database.QueryDatabase(fmt::format(
		"INSERT INTO character_achievement_pending_updates "
		"(character_id, source_target_type, source_target_id, operation, "
		"achievement_id, component_type, component_id, requested_value, "
		"`version`, status, attempt_count, created_at, last_attempt_at, "
		"last_error) VALUES {}",
		values
	));
	if (!insert.Success()) {
		database.TransactionRollbackStrict();
		LogError(
			"Failed to persist achievement state update for target type [{}], ID [{}]",
			static_cast<uint32_t>(request.target_type),
			request.target_id
		);
		return false;
	}

	if (!database.TransactionCommitStrict().Success()) {
		LogError(
			"Failed to commit achievement state update for target type [{}], ID [{}]",
			static_cast<uint32_t>(request.target_type),
			request.target_id
		);
		return false;
	}

	return true;
}

void AchievementStateUpdateManager::Wake(
	const std::vector<uint32_t> &character_ids
) const
{
	ServerPacket packet(
		ServerOP_CZAchievementStateUpdateWake,
		sizeof(ServerCharacterID_Struct)
	);
	for (const auto character_id : character_ids) {
		const auto character = ClientList::Instance()->FindCLEByCharacterID(character_id);
		if (
			!character ||
			character->GetOnline() != CLE_Status::InZone ||
			!character->Server()
		) {
			continue;
		}

		const ServerCharacterID_Struct wake{character_id};
		std::memcpy(packet.pBuffer, &wake, sizeof(wake));
		character->Server()->SendPacket(&packet);
	}
}

void AchievementStateUpdateManager::Process()
{
	if (
		!m_retry_timer.Check() ||
		!RuleB(Achievements, EnableAchievements)
	) {
		return;
	}

	ProcessRetries();

	constexpr uint32_t batch_size = 256;
	auto result = database.QueryDatabase(fmt::format(
		"SELECT DISTINCT character_id "
		"FROM character_achievement_pending_updates "
		"WHERE (status = {} OR (status = {} "
		"AND last_attempt_at + {} <= UNIX_TIMESTAMP())) "
		"AND character_id > {} "
		"ORDER BY character_id LIMIT {}",
		static_cast<uint32_t>(AchievementStateUpdates::Status::Pending),
		static_cast<uint32_t>(AchievementStateUpdates::Status::Processing),
		AchievementStateUpdates::ProcessingLeaseSeconds,
		m_retry_cursor,
		batch_size
	));
	if (!result.Success()) {
		LogError("Failed to scan pending achievement state updates");
		return;
	}

	std::vector<uint32_t> character_ids;
	character_ids.reserve(result.RowCount());
	for (auto row : result) {
		const auto character_id = ParseUInt32(row[0]);
		if (character_id) {
			character_ids.push_back(character_id);
			m_retry_cursor = character_id;
		}
	}

	Wake(character_ids);
	if (result.RowCount() < batch_size) {
		m_retry_cursor = 0;
	}
}
