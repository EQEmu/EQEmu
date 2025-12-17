#pragma once

#include "common/eqemu_logsys.h"

#include "uv.h"

#include <array>
#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

constexpr size_t UDP_BUFFER_SIZE = 512;

struct EmbeddedContext {
	size_t pool_index;
	class SendBufferPool* pool;
};

class SendBufferPool {
public:
	explicit SendBufferPool(size_t initial_capacity = 64)
		: m_capacity(initial_capacity), m_head(0)
	{
		LogNetClient("[SendBufferPool] Initializing with capacity [{}]", (int)m_capacity);

		m_pool.reserve(m_capacity);
		m_locks = std::make_unique<std::atomic_bool[]>(m_capacity);

		for (size_t i = 0; i < m_capacity; ++i) {
			auto* req = new PooledUdpSend();
			req->context.pool_index = i;
			req->context.pool = this;
			req->uv_req.data = &req->context;

			m_pool.emplace_back(std::unique_ptr<PooledUdpSend>(req));
			m_locks[i].store(false, std::memory_order_relaxed);
		}
	}

	std::optional<std::tuple<uv_udp_send_t*, char*, EmbeddedContext*>> acquire() {
		size_t cap = m_capacity.load(std::memory_order_acquire);
		for (size_t i = 0; i < cap; ++i) {
			size_t index = m_head.fetch_add(1, std::memory_order_relaxed) % cap;
			bool expected = false;
			if (m_locks[index].compare_exchange_strong(expected, true)) {
				auto* req = m_pool[index].get();
				LogNetClientDetail("[SendBufferPool] Acquired [{}]", index);
				return std::make_tuple(&req->uv_req, req->buffer.data(), &req->context);
			}
		}

		LogNetClient("[SendBufferPool] Growing from [{}] to [{}]", cap, cap * 2);
		grow();
		return acquireAfterGrowth();
	}

	void release(EmbeddedContext* ctx) {
		if (!ctx || ctx->pool != this || ctx->pool_index >= m_capacity.load(std::memory_order_acquire)) {
			LogNetClient("[SendBufferPool] Invalid context release [{}]", ctx ? ctx->pool_index : -1);
			return;
		}
		m_locks[ctx->pool_index].store(false, std::memory_order_release);
		LogNetClientDetail("[SendBufferPool] Released [{}]", ctx->pool_index);
	}

private:
	struct PooledUdpSend {
		uv_udp_send_t                     uv_req;
		std::array<char, UDP_BUFFER_SIZE> buffer;
		EmbeddedContext                   context;
	};

	std::vector<std::unique_ptr<PooledUdpSend>> m_pool;
	std::unique_ptr<std::atomic_bool[]> m_locks;
	std::atomic<size_t> m_capacity;
	std::atomic<size_t> m_head;
	std::mutex m_grow_mutex;

	void grow() {
		std::lock_guard<std::mutex> lock(m_grow_mutex);

		size_t old_cap = m_capacity.load(std::memory_order_acquire);
		size_t new_cap = old_cap * 2;

		m_pool.reserve(new_cap);
		for (size_t i = old_cap; i < new_cap; ++i) {
			auto* req = new PooledUdpSend();
			req->context.pool_index = i;
			req->context.pool = this;
			req->uv_req.data = &req->context;

			m_pool.emplace_back(std::unique_ptr<PooledUdpSend>(req));
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

		LogNetClient("[SendBufferPool] Grew to [{}] from [{}]", new_cap, old_cap);
	}

	std::optional<std::tuple<uv_udp_send_t*, char*, EmbeddedContext*>> acquireAfterGrowth() {
		size_t cap = m_capacity.load(std::memory_order_acquire);
		for (size_t i = 0; i < cap; ++i) {
			size_t index = m_head.fetch_add(1, std::memory_order_relaxed) % cap;
			bool expected = false;
			if (m_locks[index].compare_exchange_strong(expected, true)) {
				auto* req = m_pool[index].get();
				LogNetClient("[SendBufferPool] Acquired after grow [{}]", index);
				return std::make_tuple(&req->uv_req, req->buffer.data(), &req->context);
			}
		}
		return std::nullopt;
	}
};
