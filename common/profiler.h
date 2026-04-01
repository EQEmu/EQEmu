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

#ifdef EQPROFILE

#include "common/rdtsc.h"
#include "common/types.h"

class ScopedProfiler;

class GeneralProfiler {
	friend class ScopedProfiler;
public:
	inline GeneralProfiler(unsigned int _count) {
		count = _count;
		timers = new RDTSC_Collector[count];
	}
	inline virtual ~GeneralProfiler() {
		safe_delete_array(timers);
	}

	inline double getTotalDuration(unsigned int id) {
		return(id<count? timers[id].getTotalDuration() : 0);
	}

	inline double getAverage(unsigned int id) {
		return(id<count? timers[id].getAverage() : 0);
	}

	inline unsigned long long getTicks(unsigned int id) {
		return(id<count? timers[id].getTicks() : 0);
	}

	inline unsigned long long getTotalTicks(unsigned int id) {
		return(id<count? timers[id].getTotalTicks() : 0);
	}

	inline unsigned long long getCount(unsigned int id) {
		return(id<count? timers[id].getCount() : 0);
	}

	inline void reset() {
		unsigned int r;
		RDTSC_Collector *cur = timers;
		for(r = 0; r < count; r++, cur++)
			cur->reset();
	}

	RDTSC_Collector *timers;
	unsigned int count;
};

class ScopedProfiler {
public:
	inline ScopedProfiler(RDTSC_Collector *c) {
		_it = c;
		c->start();
	}
	inline ~ScopedProfiler() {
		_it->stop();
	}
protected:
	RDTSC_Collector *_it;
};


#define _GP(obj, pkg, name) ScopedProfiler __eqemu_profiler(&obj.timers[pkg::name])

#else	// else !EQPROFILE
	//no profiling, dummy functions
#define _GP(obj, pkg, name) ;

#endif
