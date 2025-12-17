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

#pragma once

#include "common/types.h"

#define CALIBRATE_LOOPS 3
#define SLEEP_TIME 10	//in ms

/*

	This class implementes the highest possibly prescision timer
	which is avaliable on the current archetecture.

	On intel, this uses the rdtsc instruction to get the actual
	clock cycle count, and elsewhere it falls back to gettimeofday

	All calculations are carried out in 64 bit integers.
*/

class RDTSC_Timer {
public:
	RDTSC_Timer();
	RDTSC_Timer(bool start_it);

	void start();	//start the timer
	virtual void stop();	//stop the timer
	double getDuration();	//returns the number of miliseconds elapsed

	//access functions
	int64 getTicks() { return(_end - _start); }
	static int64 ticksPerMS() { return(_ticsperms); }

protected:
	static int64 rdtsc();

	int64 _start;
	int64 _end;

protected:
	static void init();
	static bool _inited;
	static int64 _ticsperms;
};

//this is a timer which can be started and stoped many times.
//each time it contributes its counter to a sum, whic is used
//to find net duration.
class RDTSC_Collector : public RDTSC_Timer {
public:
	RDTSC_Collector();
	RDTSC_Collector(bool start_it);

	void reset();

	void stop();	//stop the timer

	double getTotalDuration();	//returns the number of miliseconds elapsed
	double getAverage();

	int64 getTotalTicks() { return(_sum); }
	int64 getCount() { return(_count); }

protected:
	int64 _sum;
	int64 _count;
};
