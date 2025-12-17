/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "rdtsc.h"

#include "common/types.h"

#ifdef _WINDOWS
#include <sys/timeb.h>
#include "common/timer.h"
#else
#include <unistd.h>
#include <sys/time.h>
#endif

#ifdef i386
	#define USE_RDTSC
#endif

bool RDTSC_Timer::_inited = false;
int64 RDTSC_Timer::_ticsperms = 0;

RDTSC_Timer::RDTSC_Timer() {
	if(!_inited) {
		//find our clock rate
		RDTSC_Timer::init();
	}
	_start = 0;
	_end = 0;
}

RDTSC_Timer::RDTSC_Timer(bool start_it) {
	if(!_inited) {
		//find our clock rate
		RDTSC_Timer::init();
	}
	if(start_it)
		start();
	else {
		_start = 0;
		_end = 0;
	}
}

int64 RDTSC_Timer::rdtsc() {
	int64 res = 0;
#ifdef USE_RDTSC
#ifndef WIN64
#ifdef WIN32
	//untested!
	unsigned long highw, loww;
	__asm {
		push eax
		push edx
		rdtsc
		mov highw, eax
		mov loww, edx
		pop edx
		pop eax
	}
	res = ((int64)highw)<<32 | loww;
#else
	//gnu version
	__asm__ __volatile__ ("rdtsc" : "=A" (res));
#endif
#else
	//fall back to get time of day
	timeval t;
	gettimeofday(&t, nullptr);
	res = ((int64)t.tv_sec) * 1000 + t.tv_usec;
#endif
#endif
	return(res);
}

void RDTSC_Timer::init() {
#ifdef USE_RDTSC
	int64 before, after, sum;

	int r;
	sum = 0;
	// run an average to increase accuracy of clock rate
	for(r = 0; r < CALIBRATE_LOOPS; r++) {
		before = rdtsc();

		//sleep a know duration to figure out clock rate
#ifdef _WINDOWS
		Sleep(SLEEP_TIME);
#else
		usleep(SLEEP_TIME * 1000);	//ms * 1000
#endif

		after = rdtsc();

		sum += after - before;
	}

	//ticks per sleep / ms per sleep
	_ticsperms = (sum / CALIBRATE_LOOPS) / SLEEP_TIME;

#else
	//if using gettimeofday, this is fixed at 1000
	_ticsperms = 1000;
#endif
//	printf("Tics per milisecond: %llu \n", _ticsperms);

	_inited = true;	//only want to do this once
}

//start the timer
void RDTSC_Timer::start() {
	_start = rdtsc();
	_end = 0;
}

//stop the timer
void RDTSC_Timer::stop() {
	_end = rdtsc();
}

//calculate the elapsed duration
double RDTSC_Timer::getDuration() {
	return(((double)(getTicks())) / double(_ticsperms));
}

RDTSC_Collector::RDTSC_Collector() : RDTSC_Timer() {
	reset();
}

RDTSC_Collector::RDTSC_Collector(bool start_it) : RDTSC_Timer(start_it) {
	reset();
}

void RDTSC_Collector::stop() {
	RDTSC_Timer::stop();
	_sum += RDTSC_Timer::getTicks();
	_count++;
}

//calculate the elapsed duration
double RDTSC_Collector::getTotalDuration() {
	return(((double)(getTotalTicks())) / double(_ticsperms));
}

double RDTSC_Collector::getAverage() {
	return(((double)(getTotalTicks())) / double(_ticsperms * _count));
}

void RDTSC_Collector::reset() {
	_sum = 0;
	_count = 0;
}

