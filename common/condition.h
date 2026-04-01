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

#include "common/mutex.h"
#include "common/platform/posix/include_pthreads.h"
#include "common/platform/win/include_windows.h"

//Sombody, someday needs to figure out how to implement a condition
//system on windows...


class Condition {
	private:
#ifdef WIN32
		enum {
			SignalEvent = 0,
			BroadcastEvent,
			_eventCount
		};

		HANDLE m_events[_eventCount];
		uint32 m_waiters;
		CRITICAL_SECTION CSMutex;
#else
		pthread_cond_t cond;
		pthread_mutex_t mutex;
#endif
	public:
		Condition();
		void Signal();
		void SignalAll();
		void Wait();
//		bool TimedWait(unsigned long usec);
		~Condition();
};
