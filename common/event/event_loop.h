#pragma once

#include "common/platform/win/include_windows.h" // uv.h is going to include it so lets do it first.
#include "uv.h"  // FIXME: hide this

namespace EQ
{
	class EventLoop
	{
	public:
		static EventLoop &Get() {
			static thread_local EventLoop inst;
			return inst;
		}

		~EventLoop() {
			uv_loop_close(&m_loop);
		}

		void Process() {
			uv_run(&m_loop, UV_RUN_NOWAIT);
		}

		void Run() {
			uv_run(&m_loop, UV_RUN_DEFAULT);
		}

		void Shutdown() {
			uv_stop(&m_loop);
		}

		uv_loop_t* Handle() { return &m_loop; }

	private:
		EventLoop() {
			memset(&m_loop, 0, sizeof(uv_loop_t));
			uv_loop_init(&m_loop);
		}
		
		EventLoop(const EventLoop&);
		EventLoop& operator=(const EventLoop&);
	
		uv_loop_t m_loop;
	};
}
