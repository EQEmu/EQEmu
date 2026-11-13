#pragma once

#include "common/multi_world_selector.h"

#include <chrono>
#include <cstdint>
#include <deque>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace EQ::Net::MultiWorldSelector {

struct ClientEndpoint {
	std::string address;
	std::uint16_t port{0};

	bool operator<(const ClientEndpoint &other) const
	{
		return address < other.address || (address == other.address && port < other.port);
	}

	bool operator==(const ClientEndpoint &other) const = default;
};

struct SessionRoute {
	std::string world_short_name;
	WorldBackend backend;
};

class SelectorState {
public:
	using Clock = std::chrono::steady_clock;
	using TimePoint = Clock::time_point;

	explicit SelectorState(Config config);

	bool AddSelection(const ControlSelection &selection, TimePoint now);
	bool HasExactPendingSelection(const ClientEndpoint &client) const;
	std::optional<SessionRoute> AssignSession(const ClientEndpoint &client, TimePoint now);
	std::optional<SessionRoute> GetSession(const ClientEndpoint &client) const;
	void TouchSession(const ClientEndpoint &client, TimePoint now);
	void RemoveSession(const ClientEndpoint &client);
	std::vector<ClientEndpoint> Expire(TimePoint now);

	std::size_t PendingSelectionCount() const;
	std::size_t SessionCount() const;

private:
	struct PendingSelection {
		std::uint16_t login_source_port{0};
		std::string world_short_name;
		TimePoint created_at;
	};

	struct Session {
		SessionRoute route;
		TimePoint last_activity;
	};

	Config m_config;
	std::map<std::string, std::deque<PendingSelection>> m_pending;
	std::map<ClientEndpoint, Session> m_sessions;
};

} // namespace EQ::Net::MultiWorldSelector
