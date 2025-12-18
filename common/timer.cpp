/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

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

#include "timer.h"

// Disgrace: for windows compile
#ifndef WIN32
#include <sys/time.h>
#else
#include <sys/timeb.h>
#include <WinSock2.h>
#undef GetCurrentTime
#endif

uint32 current_time = 0;
uint32 last_time = 0;

Timer::Timer() {
	timer_time = 0;
	start_time = current_time;
	set_at_trigger = timer_time;
	pUseAcurateTiming = false;
	enabled = false;
}

Timer::Timer(uint32 in_timer_time, bool iUseAcurateTiming) {
	timer_time = in_timer_time;
	start_time = current_time;
	set_at_trigger = timer_time;
	pUseAcurateTiming = iUseAcurateTiming;
	if (timer_time == 0) {
		enabled = false;
	}
	else {
		enabled = true;
	}
}

Timer::Timer(uint32 start, uint32 timer, bool iUseAcurateTiming = false) {
	timer_time = timer;
	start_time = start;
	set_at_trigger = timer_time;
	pUseAcurateTiming = iUseAcurateTiming;
	if (timer_time == 0) {
		enabled = false;
	}
	else {
		enabled = true;
	}
}

/* Reimplemented for MSVC - Bounce */
#ifdef _WINDOWS
int gettimeofday (timeval *tp, ...)
{
	timeb tb;

	ftime (&tb);

	tp->tv_sec = tb.time;
	tp->tv_usec = tb.millitm * 1000;

	return 0;
}
#endif

/* This function checks if the timer triggered */
bool Timer::Check(bool iReset)
{
	if (enabled && current_time-start_time > timer_time) {
		if (iReset) {
			if (pUseAcurateTiming)
				start_time += timer_time;
			else
				start_time = current_time; // Reset timer
			timer_time = set_at_trigger;
		}
		return true;
	}

	return false;
}

/* This function disables the timer */
void Timer::Disable() {
	enabled = false;
}

void Timer::Enable() {
	enabled = true;
}

/* This function set the timer and restart it */
void Timer::Start(uint32 set_timer_time, bool ChangeResetTimer) {
	start_time = current_time;
	enabled = true;
	if (set_timer_time != 0)
	{
		timer_time = set_timer_time;
		if (ChangeResetTimer)
			set_at_trigger = set_timer_time;
	}
}

/* This timer updates the timer without restarting it */
void Timer::SetTimer(uint32 set_timer_time) {
	/* If we were disabled before => restart the timer */
	if (!enabled) {
		start_time = current_time;
		enabled = true;
	}
	if (set_timer_time != 0) {
		timer_time = set_timer_time;
		set_at_trigger = set_timer_time;
	}
}

uint32 Timer::GetRemainingTime() const
{
	if (enabled) {
		if (current_time - start_time > timer_time) {
			return 0;
		}
		else {
			return (start_time + timer_time) - current_time;
		}
	}
	else {
		return 0xFFFFFFFF;
	}
}

void Timer::SetAtTrigger(uint32 in_set_at_trigger, bool iEnableIfDisabled, bool ChangeTimerTime) {
	set_at_trigger = in_set_at_trigger;
	if (!Enabled() && iEnableIfDisabled) {
		Enable();
	}
	if (ChangeTimerTime)
		timer_time = set_at_trigger;
}

void Timer::Trigger()
{
	enabled = true;

	timer_time = set_at_trigger;
	start_time = current_time-timer_time-1;
}

const uint32 Timer::GetCurrentTime()
{
	return current_time;
}

//just to keep all time related crap in one place... not really related to timers.
const uint32 Timer::GetTimeSeconds() {
	struct timeval read_time;

	gettimeofday(&read_time,0);
	return(read_time.tv_sec);
}

const uint32 Timer::SetCurrentTime()
{
	struct timeval read_time;
	uint32 this_time;

	gettimeofday(&read_time,0);
	this_time = read_time.tv_sec * 1000 + read_time.tv_usec / 1000;

	if (last_time == 0)
	{
		current_time = 0;
	}
	else
	{
		current_time += this_time - last_time;
	}

	last_time = this_time;

//	cerr << "Current time:" << current_time << endl;
	return current_time;
}

const uint32 Timer::RollForward(uint32 seconds)
{
	struct timeval read_time{};
	uint32 this_time;

	gettimeofday(&read_time, nullptr);
	this_time = read_time.tv_sec * 1000 + read_time.tv_usec / 1000;

	if (last_time == 0) {
		current_time = 0;
	}
	else {
		current_time += this_time - last_time;
	}

	last_time = this_time;

	// Roll forward the specified number of seconds (converted to milliseconds)
	current_time += seconds * 1000;

	return current_time;
}
