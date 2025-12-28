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

#include "dns.h"

#include "common/eqemu_logsys.h"
#include "common/event/event_loop.h"
#include "common/event/task_scheduler.h"

#include "uv.h"

namespace EQ::Net {

struct DNSBaton
{
	dns_callback_t cb;
	bool ipv6;
};

void DNSLookup(const std::string& addr, int port, bool ipv6, dns_callback_t cb)
{
	addrinfo hints = {};
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	uv_loop_t* loop = EQ::EventLoop::Get().Handle();

	uv_getaddrinfo_t* resolver = new uv_getaddrinfo_t();
	memset(resolver, 0, sizeof(uv_getaddrinfo_t));
	std::string port_str = std::to_string(port);
	DNSBaton* baton = new DNSBaton();
	baton->cb = std::move(cb);
	baton->ipv6 = ipv6;
	resolver->data = baton;

	uv_getaddrinfo(loop, resolver, [](uv_getaddrinfo_t* req, int status, addrinfo* result)
	{
		DNSBaton* baton = static_cast<DNSBaton*>(req->data);

		dns_callback_t dns_callback = std::move(baton->cb);
		bool ipv6 = baton->ipv6;

		delete baton;
		delete req;

		if (status < 0)
		{
			dns_callback({});
			return;
		}

		char addr[40] = {};

		if (ipv6)
		{
			uv_ip6_name(reinterpret_cast<sockaddr_in6*>(result->ai_addr), addr, 40);
		}
		else
		{
			uv_ip4_name(reinterpret_cast<sockaddr_in*>(result->ai_addr), addr, 40);
		}

		uv_freeaddrinfo(result);

		dns_callback(addr);
	}, addr.c_str(), port_str.c_str(), &hints);
}

std::string DNSLookupSync(const std::string& addr, int port, bool ipv6 /* = false */)
{
	EQ::Event::TaskScheduler task_runner;

	auto res = task_runner.Enqueue(
		[addr, port, ipv6]() -> std::string
	{
		bool running = true;
		std::string ret;

		EQ::Net::DNSLookup(
			addr, port, ipv6, [&](const std::string& addr) {
				ret = addr;
				running = !addr.empty();

				return ret;
			}
		);

		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

		auto& loop = EQ::EventLoop::Get();
		while (running) {
			if (!ret.empty()) {
				running = false;
			}

			std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
			if (std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() > 1500) {
				LogInfo(
					"Deadline exceeded [{}]",
					1500
				);
				running = false;
			}

			loop.Process();
		}

		return ret;
	});

	return res.get();
}


} // namespace EQ::Net
