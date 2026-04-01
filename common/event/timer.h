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
#pragma once

#include "event_loop.h"

#include <cstring>
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
