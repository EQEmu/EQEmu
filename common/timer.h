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

#include "common/types.h"

// Disgrace: for windows compile
#ifdef _WINDOWS
struct timeval;
int gettimeofday (timeval *tp, ...);
#endif

#include <chrono>

class Timer
{
public:
	Timer();
	Timer(uint32 timer_time, bool iUseAcurateTiming = false);
	Timer(uint32 start, uint32 timer, bool iUseAcurateTiming);
	~Timer() { }

	bool Check(bool iReset = true);
	void Enable();
	void Disable();
	void Start(uint32 set_timer_time=0, bool ChangeResetTimer = true);
	void SetTimer(uint32 set_timer_time=0);
	uint32 GetRemainingTime() const;
	inline const uint32& GetTimerTime()		{ return timer_time; }
	inline const uint32& GetSetAtTrigger()	{ return set_at_trigger; }
	void Trigger();
	void SetAtTrigger(uint32 set_at_trigger, bool iEnableIfDisabled = false, bool ChangeTimerTime = false);

	inline bool Enabled() { return enabled; }
	inline uint32 GetStartTime() { return(start_time); }
	inline uint32 GetDuration() { return(timer_time); }

	static const uint32 SetCurrentTime();
	static const uint32 RollForward(uint32 seconds);
	static const uint32 GetCurrentTime();
	static const uint32 GetTimeSeconds();

private:
	uint32	start_time;
	uint32	timer_time;
	bool	enabled;
	uint32	set_at_trigger;

	// Tells the timer to be more acurate about happening every X ms.
	// Instead of Check() setting the start_time = now,
	// it it sets it to start_time += timer_time
	bool	pUseAcurateTiming;
};

/* Wrapper around chrono to make adding simple time based benching easy
 * ex:
 * void foo() {
 * ...
 * BenchTimer timer;
 * ... (expensive work here)
 * auto dur = timer.elapsed();
 * std::cout << "foo() took " << dur << seconds" << std::endl;
 * ...
 * }
 * */

struct BenchTimer
{
	typedef std::chrono::high_resolution_clock clock;

	BenchTimer() : start_time(clock::now()) {}
	void reset() { start_time = clock::now(); }
	// this is seconds
	double elapsed() { return std::chrono::duration<double> (clock::now() - start_time).count(); }
	std::chrono::milliseconds::rep elapsedMilliseconds() { return std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - start_time).count(); }
	std::chrono::microseconds::rep elapsedMicroseconds() { return std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - start_time).count(); }
	std::chrono::nanoseconds::rep elapsedNanoseconds() { return std::chrono::duration_cast<std::chrono::nanoseconds>(clock::now() - start_time).count(); }
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
};
