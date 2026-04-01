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

#include "common/eq_packet_structs.h"

#include <string>

//Struct
struct eqTimeOfDay
{
	TimeOfDay_Struct start_eqtime;
	time_t	start_realtime;
};

//Class Def
class EQTime
{
public:
	//Constructor/destructor
	EQTime(TimeOfDay_Struct start_eq, time_t start_real);
	EQTime();
	~EQTime() = default;

	//Get functions
	int GetCurrentEQTimeOfDay( TimeOfDay_Struct *eqTimeOfDay ) { return(GetCurrentEQTimeOfDay(time(nullptr), eqTimeOfDay)); }
	int GetCurrentEQTimeOfDay( time_t timeConvert, TimeOfDay_Struct *eqTimeOfDay );
	TimeOfDay_Struct getStartEQTime() { return eqTime.start_eqtime; }
	time_t getStartRealTime() { return eqTime.start_realtime; }
	uint32 getEQTimeZone() { return timezone; }
	uint32 getEQTimeZoneHr() { return timezone/60; }
	uint32 getEQTimeZoneMin() { return timezone%60; }
	bool IsDayTime();
	bool IsNightTime();
	bool IsInbetweenTime(uint8 min_time, uint8 max_time);

	//Set functions
	int SetCurrentEQTimeOfDay(TimeOfDay_Struct start_eq, time_t start_real);
	void setEQTimeZone(int32 in_timezone) { timezone=in_timezone; }

	//Time math/logic functions
	static bool IsTimeBefore(TimeOfDay_Struct *base, TimeOfDay_Struct *test);	//is test before base
	static void AddMinutes(uint32 minutes, TimeOfDay_Struct *to);

	static void ToString(TimeOfDay_Struct *t, std::string &str);

private:
	//This is our reference clock.
	eqTimeOfDay eqTime;
	//This is our tz offset
	int32 timezone;
};
