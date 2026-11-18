#include "world_selector/world_selector_server.h"

#include <algorithm>
#include <csignal>
#include <cstring>
#include <iostream>
#include <memory>
#include <vector>

namespace EQ::Net::MultiWorldSelector {
namespace {

struct DatagramSend {
	uv_udp_send_t request{};
	std::vector<char> payload;
};

bool ParseSocketAddress(const std::string &address, std::uint16_t port, sockaddr_in &result)
{
	return uv_ip4_addr(address.c_str(), port, &result) == 0;
}

std::string SocketAddress(const sockaddr *address)
{
	if (address == nullptr || address->sa_family != AF_INET) {
		return {};
	}

	char text[INET_ADDRSTRLEN]{};
	if (uv_ip4_name(reinterpret_cast<const sockaddr_in *>(address), text, sizeof(text)) != 0) {
		return {};
	}
	return text;
}

std::uint16_t SocketPort(const sockaddr *address)
{
	return address != nullptr && address->sa_family == AF_INET
		? ntohs(reinterpret_cast<const sockaddr_in *>(address)->sin_port)
		: 0;
}

bool SameSocketAddress(const sockaddr_in &expected, const sockaddr *actual)
{
	if (actual == nullptr || actual->sa_family != AF_INET) {
		return false;
	}
	const auto *actual_ipv4 = reinterpret_cast<const sockaddr_in *>(actual);
	return expected.sin_port == actual_ipv4->sin_port &&
		expected.sin_addr.s_addr == actual_ipv4->sin_addr.s_addr;
}

bool SendDatagram(uv_udp_t *socket, const char *data, std::size_t size, const sockaddr *destination)
{
	auto send = std::make_unique<DatagramSend>();
	send->payload.assign(data, data + size);
	send->request.data = send.get();
	auto buffer = uv_buf_init(send->payload.data(), static_cast<unsigned int>(send->payload.size()));
	const auto result = uv_udp_send(
		&send->request,
		socket,
		&buffer,
		1,
		destination,
		[](uv_udp_send_t *request, int status) {
			std::unique_ptr<DatagramSend> completed(static_cast<DatagramSend *>(request->data));
			if (status < 0) {
				std::cerr << "UDP relay send failed: " << uv_strerror(status) << '\n';
			}
		}
	);

	if (result < 0) {
		std::cerr << "Unable to queue UDP relay send: " << uv_strerror(result) << '\n';
		return false;
	}

	send.release();
	return true;
}

} // namespace

bool ValidateConfig(const Config &config, std::string &error)
{
	if (!config.enabled) {
		return true;
	}
	if (!IsLoopbackIPv4(config.control_bind)) {
		error = "control_bind must be a numeric IPv4 loopback address (127.0.0.0/8)";
		return false;
	}
	if (config.control_port == 0 || config.world_port == 0 ||
		config.selection_timeout_seconds == 0 || config.session_timeout_seconds == 0) {
		error = "ports and timeout values must be non-zero";
		return false;
	}

	sockaddr_in parsed{};
	if (!ParseSocketAddress(config.control_bind, config.control_port, parsed) ||
		!ParseSocketAddress(config.world_bind, config.world_port, parsed) ||
		!ParseSocketAddress(config.upstream_bind, 0, parsed)) {
		error = "control_bind, world_bind, and upstream_bind must be numeric IPv4 addresses";
		return false;
	}
	if (config.worlds.empty()) {
		error = "at least one World backend must be configured";
		return false;
	}

	for (const auto &[name, backend] : config.worlds) {
		if (!IsValidWorldShortName(name) || backend.port == 0 ||
			!ParseSocketAddress(backend.host, backend.port, parsed)) {
			error = "invalid World backend entry: " + name;
			return false;
		}
		if (backend.host == config.world_bind && backend.port == config.world_port) {
			error = "World backend would loop back into the selector: " + name;
			return false;
		}
	}

	return true;
}

WorldSelectorServer::WorldSelectorServer(Config config)
:	m_config(std::move(config)),
	m_state(m_config),
	m_loop(uv_default_loop())
{
}

WorldSelectorServer::~WorldSelectorServer() = default;

bool WorldSelectorServer::Start(std::string &error)
{
	if (m_started) {
		return true;
	}
	if (!ValidateConfig(m_config, error)) {
		return false;
	}

	sockaddr_in control_address{};
	sockaddr_in world_address{};
	ParseSocketAddress(m_config.control_bind, m_config.control_port, control_address);
	ParseSocketAddress(m_config.world_bind, m_config.world_port, world_address);

	if (const auto result = uv_udp_init(m_loop, &m_control_socket); result < 0) {
		error = std::string("unable to initialize selector control socket: ") + uv_strerror(result);
		return false;
	}
	m_control_initialized = true;
	if (const auto result = uv_udp_bind(
		&m_control_socket,
		reinterpret_cast<const sockaddr *>(&control_address),
		0
	); result < 0) {
		error = "unable to bind selector control socket to " + m_config.control_bind + ":" +
			std::to_string(m_config.control_port) + ": " + uv_strerror(result);
		Stop();
		uv_run(m_loop, UV_RUN_NOWAIT);
		return false;
	}
	m_control_socket.data = this;

	if (const auto result = uv_udp_init(m_loop, &m_world_socket); result < 0) {
		error = std::string("unable to initialize selector World socket: ") + uv_strerror(result);
		Stop();
		uv_run(m_loop, UV_RUN_NOWAIT);
		return false;
	}
	m_world_initialized = true;
	if (const auto result = uv_udp_bind(
		&m_world_socket,
		reinterpret_cast<const sockaddr *>(&world_address),
		0
	); result < 0) {
		error = "unable to bind selector World socket to " + m_config.world_bind + ":" +
			std::to_string(m_config.world_port) + ": " + uv_strerror(result);
		Stop();
		uv_run(m_loop, UV_RUN_NOWAIT);
		return false;
	}
	m_world_socket.data = this;

	if (const auto result = uv_timer_init(m_loop, &m_cleanup_timer); result < 0) {
		error = std::string("unable to initialize cleanup timer: ") + uv_strerror(result);
		Stop();
		uv_run(m_loop, UV_RUN_NOWAIT);
		return false;
	}
	m_timer_initialized = true;
	m_cleanup_timer.data = this;

	if (const auto result = uv_signal_init(m_loop, &m_interrupt_signal); result < 0) {
		error = std::string("unable to initialize interrupt handler: ") + uv_strerror(result);
		Stop();
		uv_run(m_loop, UV_RUN_NOWAIT);
		return false;
	}
	m_interrupt_initialized = true;
	m_interrupt_signal.data = this;

	if (const auto result = uv_signal_init(m_loop, &m_terminate_signal); result < 0) {
		error = std::string("unable to initialize termination handler: ") + uv_strerror(result);
		Stop();
		uv_run(m_loop, UV_RUN_NOWAIT);
		return false;
	}
	m_terminate_initialized = true;
	m_terminate_signal.data = this;

	if (uv_udp_recv_start(&m_control_socket, AllocateBuffer, OnControlDatagram) < 0 ||
		uv_udp_recv_start(&m_world_socket, AllocateBuffer, OnWorldDatagram) < 0) {
		error = "unable to start UDP receive handlers";
		Stop();
		uv_run(m_loop, UV_RUN_NOWAIT);
		return false;
	}

	uv_timer_start(
		&m_cleanup_timer,
		[](uv_timer_t *timer) {
			static_cast<WorldSelectorServer *>(timer->data)->ExpireState();
		},
		1000,
		1000
	);
	uv_signal_start(
		&m_interrupt_signal,
		[](uv_signal_t *signal, int) {
			static_cast<WorldSelectorServer *>(signal->data)->Stop();
		},
		SIGINT
	);
	uv_signal_start(
		&m_terminate_signal,
		[](uv_signal_t *signal, int) {
			static_cast<WorldSelectorServer *>(signal->data)->Stop();
		},
		SIGTERM
	);

	m_started = true;
	std::cout << "Selector control listening on " << m_config.control_bind << ':' << m_config.control_port << '\n';
	std::cout << "EQ World relay listening on " << m_config.world_bind << ':' << m_config.world_port << '\n';
	return true;
}

int WorldSelectorServer::Run()
{
	return uv_run(m_loop, UV_RUN_DEFAULT);
}

void WorldSelectorServer::Stop()
{
	if (m_stopping) {
		return;
	}
	m_stopping = true;

	std::vector<ClientEndpoint> clients;
	clients.reserve(m_upstreams.size());
	for (const auto &[client, session] : m_upstreams) {
		clients.push_back(client);
	}
	for (const auto &client : clients) {
		CloseSession(client);
	}

	auto close_handle = [](uv_handle_t *handle) {
		if (!uv_is_closing(handle)) {
			uv_close(handle, nullptr);
		}
	};
	if (m_control_initialized) {
		uv_udp_recv_stop(&m_control_socket);
		close_handle(reinterpret_cast<uv_handle_t *>(&m_control_socket));
	}
	if (m_world_initialized) {
		uv_udp_recv_stop(&m_world_socket);
		close_handle(reinterpret_cast<uv_handle_t *>(&m_world_socket));
	}
	if (m_timer_initialized) {
		uv_timer_stop(&m_cleanup_timer);
		close_handle(reinterpret_cast<uv_handle_t *>(&m_cleanup_timer));
	}
	if (m_interrupt_initialized) {
		uv_signal_stop(&m_interrupt_signal);
		close_handle(reinterpret_cast<uv_handle_t *>(&m_interrupt_signal));
	}
	if (m_terminate_initialized) {
		uv_signal_stop(&m_terminate_signal);
		close_handle(reinterpret_cast<uv_handle_t *>(&m_terminate_signal));
	}
	std::cout << "Selector shutdown requested; in-memory routes cleared\n";
}

void WorldSelectorServer::AllocateBuffer(uv_handle_t *, std::size_t suggested_size, uv_buf_t *buffer)
{
	const auto size = std::max<std::size_t>(suggested_size, 2048);
	buffer->base = new char[size];
	buffer->len  = static_cast<unsigned int>(size);
}

void WorldSelectorServer::OnControlDatagram(
	uv_udp_t *handle,
	ssize_t bytes_read,
	const uv_buf_t *buffer,
	const sockaddr *address,
	unsigned int
)
{
	std::unique_ptr<char[]> storage(buffer->base);
	if (bytes_read > 0 && address != nullptr) {
		static_cast<WorldSelectorServer *>(handle->data)->ProcessControlDatagram(
			reinterpret_cast<const std::uint8_t *>(buffer->base),
			static_cast<std::size_t>(bytes_read),
			address
		);
	}
}

void WorldSelectorServer::OnWorldDatagram(
	uv_udp_t *handle,
	ssize_t bytes_read,
	const uv_buf_t *buffer,
	const sockaddr *address,
	unsigned int
)
{
	std::unique_ptr<char[]> storage(buffer->base);
	if (bytes_read > 0 && address != nullptr) {
		static_cast<WorldSelectorServer *>(handle->data)->ProcessWorldDatagram(
			buffer->base,
			static_cast<std::size_t>(bytes_read),
			address
		);
	}
}

void WorldSelectorServer::OnBackendDatagram(
	uv_udp_t *handle,
	ssize_t bytes_read,
	const uv_buf_t *buffer,
	const sockaddr *address,
	unsigned int
)
{
	std::unique_ptr<char[]> storage(buffer->base);
	auto *session = static_cast<UpstreamSession *>(handle->data);
	if (bytes_read > 0 && address != nullptr && session != nullptr) {
		session->owner->ProcessBackendDatagram(
			*session,
			buffer->base,
			static_cast<std::size_t>(bytes_read),
			address
		);
	}
}

void WorldSelectorServer::ProcessControlDatagram(
	const std::uint8_t *data,
	std::size_t size,
	const sockaddr *address
)
{
	const auto source = SocketAddress(address);
	if (!IsLoopbackIPv4(source)) {
		std::cerr << "Ignored selector control packet from non-loopback source " << source << '\n';
		return;
	}

	ControlSelection selection;
	if (!DecodeControlPacket(data, size, selection)) {
		std::cerr << "Ignored malformed selector control packet from " << source << '\n';
		return;
	}
	if (!m_state.AddSelection(selection, SelectorState::Clock::now())) {
		std::cerr << "Ignored selector control packet for unknown World '"
			<< selection.world_short_name << "'\n";
		return;
	}

	std::cout << "Selection " << selection.client_ip << " -> " << selection.world_short_name
		<< " (login source port hint " << selection.login_source_port << ")\n";
}

void WorldSelectorServer::ProcessWorldDatagram(const char *data, std::size_t size, const sockaddr *address)
{
	ClientEndpoint client{SocketAddress(address), SocketPort(address)};
	if (client.address.empty() || client.port == 0) {
		return;
	}

	auto upstream = m_upstreams.find(client);
	const bool is_session_request = size >= 14 && static_cast<std::uint8_t>(data[0]) == 0 &&
		static_cast<std::uint8_t>(data[1]) == 0x01;
	if (upstream != m_upstreams.end() && is_session_request && m_state.HasExactPendingSelection(client)) {
		std::cout << "Replacing existing World route for reselected client "
			<< client.address << ':' << client.port << '\n';
		CloseSession(client);
		upstream = m_upstreams.end();
	}

	if (upstream == m_upstreams.end()) {
		// Only an EQ session request may consume a pending login selection.
		if (!is_session_request) {
			return;
		}

		const auto route = m_state.AssignSession(client, SelectorState::Clock::now());
		if (!route) {
			std::cerr << "No unexpired World selection for " << client.address << ':' << client.port << '\n';
			return;
		}

		std::string error;
		if (!OpenUpstream(client, *route, error)) {
			std::cerr << error << '\n';
			m_state.RemoveSession(client);
			return;
		}
		upstream = m_upstreams.find(client);
		std::cout << "Session " << client.address << ':' << client.port << " -> "
			<< route->world_short_name << " (" << route->backend.host << ':' << route->backend.port << ")\n";
	}

	m_state.TouchSession(client, SelectorState::Clock::now());
	SendDatagram(
		&upstream->second->socket,
		data,
		size,
		reinterpret_cast<const sockaddr *>(&upstream->second->backend)
	);
}

void WorldSelectorServer::ProcessBackendDatagram(
	UpstreamSession &session,
	const char *data,
	std::size_t size,
	const sockaddr *address
)
{
	if (!SameSocketAddress(session.backend, address)) {
		std::cerr << "Ignored UDP packet from unexpected backend " << SocketAddress(address) << ':'
			<< SocketPort(address) << '\n';
		return;
	}

	sockaddr_in client_address{};
	if (!ParseSocketAddress(session.client.address, session.client.port, client_address)) {
		return;
	}
	m_state.TouchSession(session.client, SelectorState::Clock::now());
	SendDatagram(
		&m_world_socket,
		data,
		size,
		reinterpret_cast<const sockaddr *>(&client_address)
	);
}

bool WorldSelectorServer::OpenUpstream(
	const ClientEndpoint &client,
	const SessionRoute &route,
	std::string &error
)
{
	auto session = std::make_unique<UpstreamSession>();
	session->owner  = this;
	session->client = client;
	if (!ParseSocketAddress(route.backend.host, route.backend.port, session->backend)) {
		error = "invalid backend address for World " + route.world_short_name;
		return false;
	}

	const auto initialized = uv_udp_init(m_loop, &session->socket);
	if (initialized < 0) {
		error = std::string("unable to initialize upstream socket: ") + uv_strerror(initialized);
		return false;
	}
	session->socket.data = session.get();

	sockaddr_in source{};
	ParseSocketAddress(m_config.upstream_bind, 0, source);
	const auto bound = uv_udp_bind(&session->socket, reinterpret_cast<const sockaddr *>(&source), 0);
	if (bound < 0) {
		error = std::string("unable to bind ephemeral upstream socket: ") + uv_strerror(bound);
		uv_close(
			reinterpret_cast<uv_handle_t *>(&session->socket),
			[](uv_handle_t *handle) {
				delete static_cast<UpstreamSession *>(handle->data);
			}
		);
		session.release();
		return false;
	}

	const auto receiving = uv_udp_recv_start(&session->socket, AllocateBuffer, OnBackendDatagram);
	if (receiving < 0) {
		error = std::string("unable to receive backend replies: ") + uv_strerror(receiving);
		uv_close(
			reinterpret_cast<uv_handle_t *>(&session->socket),
			[](uv_handle_t *handle) {
				delete static_cast<UpstreamSession *>(handle->data);
			}
		);
		session.release();
		return false;
	}

	m_upstreams.emplace(client, session.release());
	return true;
}

void WorldSelectorServer::CloseSession(const ClientEndpoint &client)
{
	const auto session = m_upstreams.find(client);
	if (session == m_upstreams.end()) {
		m_state.RemoveSession(client);
		return;
	}

	auto *upstream = session->second;
	m_upstreams.erase(session);
	m_state.RemoveSession(client);
	uv_udp_recv_stop(&upstream->socket);
	if (!uv_is_closing(reinterpret_cast<uv_handle_t *>(&upstream->socket))) {
		uv_close(
			reinterpret_cast<uv_handle_t *>(&upstream->socket),
			[](uv_handle_t *handle) {
				delete static_cast<UpstreamSession *>(handle->data);
			}
		);
	}
}

void WorldSelectorServer::ExpireState()
{
	for (const auto &client : m_state.Expire(SelectorState::Clock::now())) {
		std::cout << "Expired World session " << client.address << ':' << client.port << '\n';
		CloseSession(client);
	}
}

} // namespace EQ::Net::MultiWorldSelector
