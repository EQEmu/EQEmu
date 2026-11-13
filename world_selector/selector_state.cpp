#include "world_selector/selector_state.h"

#include <algorithm>

namespace EQ::Net::MultiWorldSelector {
namespace {

bool SameRoute(const SessionRoute &left, const SessionRoute &right)
{
	return left.world_short_name == right.world_short_name && left.backend.host == right.backend.host &&
		left.backend.port == right.backend.port;
}

} // namespace

SelectorState::SelectorState(Config config)
:	m_config(std::move(config))
{
}

bool SelectorState::AddSelection(const ControlSelection &selection, TimePoint now)
{
	if (!IsValidIPv4(selection.client_ip) || selection.login_source_port == 0 ||
		m_config.worlds.find(selection.world_short_name) == m_config.worlds.end()) {
		return false;
	}

	auto &pending = m_pending[selection.client_ip];
	std::erase_if(
		pending,
		[&selection](const PendingSelection &entry) {
			return entry.login_source_port == selection.login_source_port;
		}
	);
	pending.push_back({selection.login_source_port, selection.world_short_name, now});
	return true;
}

bool SelectorState::HasExactPendingSelection(const ClientEndpoint &client) const
{
	const auto pending_by_ip = m_pending.find(client.address);
	if (pending_by_ip == m_pending.end()) {
		return false;
	}

	return std::any_of(
		pending_by_ip->second.begin(),
		pending_by_ip->second.end(),
		[&client](const PendingSelection &selection) {
			return selection.login_source_port == client.port;
		}
	);
}

std::optional<SessionRoute> SelectorState::AssignSession(const ClientEndpoint &client, TimePoint now)
{
	if (const auto existing = m_sessions.find(client); existing != m_sessions.end()) {
		existing->second.last_activity = now;
		return existing->second.route;
	}

	if (auto pending_by_ip = m_pending.find(client.address);
		pending_by_ip != m_pending.end() && !pending_by_ip->second.empty()) {
		auto &pending = pending_by_ip->second;
		auto selection = std::find_if(
			pending.begin(),
			pending.end(),
			[&client](const PendingSelection &entry) {
				return entry.login_source_port == client.port;
			}
		);

		// UDP NAT mappings can differ between login and World. If the source-port
		// hint does not match, selection order is the only correlation exposed by
		// the unmodified client and login protocol.
		if (selection == pending.end()) {
			selection = pending.begin();
		}

		const auto backend = m_config.worlds.find(selection->world_short_name);
		if (backend == m_config.worlds.end()) {
			pending.erase(selection);
			return std::nullopt;
		}

		SessionRoute route{selection->world_short_name, backend->second};
		pending.erase(selection);
		if (pending.empty()) {
			m_pending.erase(pending_by_ip);
		}

		m_sessions.emplace(client, Session{route, now});
		return route;
	}

	const auto session_timeout = std::chrono::seconds(m_config.session_timeout_seconds);
	std::optional<SessionRoute> continuation_route;
	for (const auto &[endpoint, session] : m_sessions) {
		if (endpoint.address != client.address || now - session.last_activity >= session_timeout) {
			continue;
		}

		if (!continuation_route) {
			continuation_route = session.route;
		}
		else if (!SameRoute(*continuation_route, session.route)) {
			return std::nullopt;
		}
	}

	if (!continuation_route) {
		return std::nullopt;
	}

	m_sessions.emplace(client, Session{*continuation_route, now});
	return continuation_route;
}

std::optional<SessionRoute> SelectorState::GetSession(const ClientEndpoint &client) const
{
	const auto session = m_sessions.find(client);
	return session == m_sessions.end() ? std::nullopt : std::optional<SessionRoute>{session->second.route};
}

void SelectorState::TouchSession(const ClientEndpoint &client, TimePoint now)
{
	if (auto session = m_sessions.find(client); session != m_sessions.end()) {
		session->second.last_activity = now;
	}
}

void SelectorState::RemoveSession(const ClientEndpoint &client)
{
	m_sessions.erase(client);
}

std::vector<ClientEndpoint> SelectorState::Expire(TimePoint now)
{
	const auto selection_timeout = std::chrono::seconds(m_config.selection_timeout_seconds);
	for (auto pending_by_ip = m_pending.begin(); pending_by_ip != m_pending.end();) {
		std::erase_if(
			pending_by_ip->second,
			[now, selection_timeout](const PendingSelection &entry) {
				return now - entry.created_at >= selection_timeout;
			}
		);

		if (pending_by_ip->second.empty()) {
			pending_by_ip = m_pending.erase(pending_by_ip);
		}
		else {
			++pending_by_ip;
		}
	}

	const auto session_timeout = std::chrono::seconds(m_config.session_timeout_seconds);
	std::vector<ClientEndpoint> expired_sessions;
	for (const auto &[client, session] : m_sessions) {
		if (now - session.last_activity >= session_timeout) {
			expired_sessions.push_back(client);
		}
	}

	return expired_sessions;
}

std::size_t SelectorState::PendingSelectionCount() const
{
	std::size_t count = 0;
	for (const auto &[address, pending] : m_pending) {
		count += pending.size();
	}
	return count;
}

std::size_t SelectorState::SessionCount() const
{
	return m_sessions.size();
}

} // namespace EQ::Net::MultiWorldSelector
