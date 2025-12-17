#pragma once

#include "event_loop.h"

#include <functional>

namespace EQ {
	class Timer
	{
	public:
		Timer(std::function<void(Timer *)> cb)
		{
			m_timer = nullptr;
			m_cb = cb;
		}

		Timer(uint64_t duration_ms, bool repeats, std::function<void(Timer *)> cb)
		{
			m_timer = nullptr;
			m_cb = cb;
			Start(duration_ms, repeats);
		}
	
		~Timer()
		{
			Stop();
		}

		void Start(uint64_t duration_ms, bool repeats) {
			auto loop = EventLoop::Get().Handle();
			if (!m_timer) {
				m_timer = new uv_timer_t;
				memset(m_timer, 0, sizeof(uv_timer_t));
				uv_timer_init(loop, m_timer);
				m_timer->data = this;

				if (repeats) {
					uv_timer_start(m_timer, [](uv_timer_t *handle) {
						Timer *t = (Timer*)handle->data;
						t->Execute();
					}, duration_ms, duration_ms);
				}
				else {
					uv_timer_start(m_timer, [](uv_timer_t *handle) {
						Timer *t = (Timer*)handle->data;
						t->Stop();
						t->Execute();
					}, duration_ms, 0);
				}
			}
		}

		void Stop() {
			if (m_timer) {
				uv_close((uv_handle_t*)m_timer, [](uv_handle_t* handle) {
					delete (uv_timer_t *)handle;
				});
				m_timer = nullptr;
			}
		}
	private:
		void Execute() {
			m_cb(this);
		}
	
		uv_timer_t *m_timer;
		std::function<void(Timer*)> m_cb;
	};
}
