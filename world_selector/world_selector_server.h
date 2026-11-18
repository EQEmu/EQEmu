#pragma once

#include "world_selector/selector_state.h"

#include <map>
#include <string>

#include <uv.h>

namespace EQ::Net::MultiWorldSelector {

class WorldSelectorServer {
public:
	explicit WorldSelectorServer(Config config);
	~WorldSelectorServer();

	bool Start(std::string &error);
	int Run();
	void Stop();

private:
	struct UpstreamSession {
		uv_udp_t socket{};
		WorldSelectorServer *owner{};
		ClientEndpoint client;
		sockaddr_in backend{};
	};

	static void AllocateBuffer(uv_handle_t *handle, std::size_t suggested_size, uv_buf_t *buffer);
	static void OnControlDatagram(
		uv_udp_t *handle,
		ssize_t bytes_read,
		const uv_buf_t *buffer,
		const sockaddr *address,
		unsigned int flags
	);
	static void OnWorldDatagram(
		uv_udp_t *handle,
		ssize_t bytes_read,
		const uv_buf_t *buffer,
		const sockaddr *address,
		unsigned int flags
	);
	static void OnBackendDatagram(
		uv_udp_t *handle,
		ssize_t bytes_read,
		const uv_buf_t *buffer,
		const sockaddr *address,
		unsigned int flags
	);

	void ProcessControlDatagram(const std::uint8_t *data, std::size_t size, const sockaddr *address);
	void ProcessWorldDatagram(const char *data, std::size_t size, const sockaddr *address);
	void ProcessBackendDatagram(UpstreamSession &session, const char *data, std::size_t size, const sockaddr *address);
	bool OpenUpstream(const ClientEndpoint &client, const SessionRoute &route, std::string &error);
	void CloseSession(const ClientEndpoint &client);
	void ExpireState();

	Config m_config;
	SelectorState m_state;
	uv_loop_t *m_loop{};
	uv_udp_t m_control_socket{};
	uv_udp_t m_world_socket{};
	uv_timer_t m_cleanup_timer{};
	uv_signal_t m_interrupt_signal{};
	uv_signal_t m_terminate_signal{};
	bool m_started{false};
	bool m_stopping{false};
	bool m_control_initialized{false};
	bool m_world_initialized{false};
	bool m_timer_initialized{false};
	bool m_interrupt_initialized{false};
	bool m_terminate_initialized{false};
	std::map<ClientEndpoint, UpstreamSession *> m_upstreams;
};

bool ValidateConfig(const Config &config, std::string &error);

} // namespace EQ::Net::MultiWorldSelector
