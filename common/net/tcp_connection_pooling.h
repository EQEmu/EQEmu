#pragma once

#include "common/eqemu_logsys.h"

#include "uv.h"
#include <array>
#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

namespace EQ { namespace Net { class TCPConnection; } }

constexpr size_t TCP_BUFFER_SIZE = 8192;

struct TCPWriteReq {
	uv_write_t req{};
	std::array<char, TCP_BUFFER_SIZE> buffer{};
	size_t buffer_index{};
	EQ::Net::TCPConnection* connection{};
	uint32_t magic = 0xC0FFEE;
};

class WriteReqPool {
public:
	explicit WriteReqPool(size_t initial_capacity = 512)
		: m_capacity(initial_capacity), m_head(0) {
		initialize_pool(m_capacity);
	}

	std::optional<TCPWriteReq*> acquire() {
		size_t cap = m_capacity.load(std::memory_order_acquire);

		for (size_t i = 0; i < cap; ++i) {
			size_t index = m_head.fetch_add(1, std::memory_order_relaxed) % cap;

			bool expected = false;
			if (m_locks[index].compare_exchange_strong(expected, true, std::memory_order_acquire)) {
				LogNetTCPDetail("[WriteReqPool] Acquired buffer index [{}]", index);
				return m_reqs[index].get();
			}
		}

		LogNetTCP("[WriteReqPool] Growing from [{}] to [{}]", cap, cap * 2);
		grow();
		return acquireAfterGrow();
	}

	void release(TCPWriteReq* req) {
		if (!req) return;

		const size_t index = req->buffer_index;
		const size_t cap = m_capacity.load(std::memory_order_acquire);

		if (index >= cap || m_reqs[index].get() != req) {
			std::cerr << "WriteReqPool::release - Invalid or stale pointer (index=" << index << ")\n";
			return;
		}

		m_locks[index].store(false, std::memory_order_release);
		LogNetTCPDetail("[WriteReqPool] Released buffer index [{}]", index);
	}

private:
	std::vector<std::unique_ptr<TCPWriteReq>> m_reqs;
	std::unique_ptr<std::atomic_bool[]> m_locks;
	std::atomic<size_t> m_capacity;
	std::atomic<size_t> m_head;
	std::mutex m_grow_mutex;

	void initialize_pool(size_t count) {
		m_reqs.reserve(count);
		m_locks = std::make_unique<std::atomic_bool[]>(count);

		for (size_t i = 0; i < count; ++i) {
			auto req = std::make_unique<TCPWriteReq>();
			req->buffer_index = i;
			req->req.data = req.get(); // optional: for use in libuv callbacks
			m_locks[i].store(false, std::memory_order_relaxed);
			m_reqs.emplace_back(std::move(req));
		}

		m_capacity.store(count, std::memory_order_release);
	}

	void grow() {
		std::lock_guard<std::mutex> lock(m_grow_mutex);

		const size_t old_cap = m_capacity.load(std::memory_order_acquire);
		const size_t new_cap = old_cap * 2;

		m_reqs.reserve(new_cap);
		for (size_t i = old_cap; i < new_cap; ++i) {
			auto req = std::make_unique<TCPWriteReq>();
			req->buffer_index = i;
			req->req.data = req.get(); // optional
			m_reqs.emplace_back(std::move(req));
		}

		auto new_locks = std::make_unique<std::atomic_bool[]>(new_cap);
		for (size_t i = 0; i < old_cap; ++i) {
			new_locks[i].store(m_locks[i].load(std::memory_order_acquire));
		}
		for (size_t i = old_cap; i < new_cap; ++i) {
			new_locks[i].store(false, std::memory_order_relaxed);
		}

		m_locks = std::move(new_locks);
		m_capacity.store(new_cap, std::memory_order_release);
	}

	std::optional<TCPWriteReq*> acquireAfterGrow() {
		const size_t cap = m_capacity.load(std::memory_order_acquire);

		for (size_t i = 0; i < cap; ++i) {
			bool expected = false;
			if (m_locks[i].compare_exchange_strong(expected, true, std::memory_order_acquire)) {
				LogNetTCP("[WriteReqPool] Acquired buffer index [{}] after grow", i);
				return m_reqs[i].get();
			}
		}
		return std::nullopt;
	}
};
