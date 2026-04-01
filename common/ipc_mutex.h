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

#include <string>

namespace EQ {

	//! Interprocess Named Binary Semaphore (Mutex)
	/*!
		Important to note: while this can be used to synchronize processes, it is not in itself re-entrant or thread-safe
		and thus should be used from one thread and non-recursively. It was intended to be a simple synchronization method
		for our MemoryMappedFile loading.
	*/
	class IPCMutex {
		struct Implementation;
	public:
		//! Constructor
		/*!
			Creates a named binary semaphore, basically a semaphore that is init S <- 1
		\param name The name of this mutex.
		*/
		IPCMutex(const std::string& name);

		//! Destructor
		~IPCMutex();

		//! Lock the mutex
		/*!
			Same basic function as P(): for(;;) { if(S > 0) { S -= 1; break; } }
		*/
		bool Lock();

		//! Unlocks the mutex
		/*!
			Same basic function as V(): S += 1;
		*/
		bool Unlock();
	private:
		IPCMutex(const IPCMutex&);
		const IPCMutex& operator=(const IPCMutex&);

		bool locked_; //!< Whether this mutex is locked or not
		Implementation *imp_;
	};
}
