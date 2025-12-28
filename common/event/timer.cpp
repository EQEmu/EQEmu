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

#include "common/event/timer.h"
#include "uv.h"

namespace EQ {

Timer::Timer(callback_t cb)
	: m_cb(std::move(cb))
{
}

Timer::Timer(uint64_t duration_ms, bool repeats, callback_t cb)
	: m_cb(std::move(cb))
{
	Start(duration_ms, repeats);
}

Timer::~Timer()
{
	Stop();
}

void Timer::Start(uint64_t duration_ms, bool repeats)
{
	if (!m_timer)
	{
		uv_loop_t* loop = EventLoop::Get().Handle();

		m_timer = std::make_unique<uv_timer_t>();
		memset(m_timer.get(), 0, sizeof(uv_timer_t));

		uv_timer_init(loop, m_timer.get());
		m_timer->data = this;

		if (repeats)
		{
			uv_timer_start(m_timer.get(), [](uv_timer_t* handle)
			{
				Timer* t = static_cast<Timer*>(handle->data);
				t->Execute();
			}, duration_ms, duration_ms);
		}
		else
		{
			uv_timer_start(m_timer.get(),
				[](uv_timer_t* handle)
			{
				Timer* t = static_cast<Timer*>(handle->data);
				t->Stop();
				t->Execute();
			}, duration_ms, 0);
		}
	}
}

void Timer::Stop()
{
	if (m_timer)
	{
		uv_close(reinterpret_cast<uv_handle_t*>(m_timer.release()),
			[](uv_handle_t* handle)
		{
			delete reinterpret_cast<uv_timer_t*>(handle);
		});
	}
}

void Timer::Execute()
{
	m_cb(this);
}

} // namespace EQ
