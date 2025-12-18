/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2005 EQEMu Development Team (http://eqemu.org)

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

#include "ptimer.h"

#include "common/database.h"
#include "common/strings.h"
#include "common/timer.h"

#ifdef _WINDOWS
#include <winsock2.h>
#include <windows.h>
int gettimeofday (timeval *tp, ...);
#else
#include <sys/time.h>
#endif

#if EQDEBUG > 10
#define DEBUG_PTIMERS
#endif


/*
Persistent timers, By Father Nitwit

The idea of persistent timers is timers which follow a player
through zoning. You use operations on them much like regular
timers: Start, Check, Clear but they also provide methods
to store them in the DB: Load and Store.

All durations are in seconds.

Each persistent timer is attached to a character, and given
a specific type. A given character can only have one timer
of each type. While the type is just an arbitrary number,
please record what you are using it for in the enum for
pTimerType at the top of ptimer.h, and give it a UNIQUE number.

There should be little need to directly access ptimers. Each
client has a facility called p_timers which should handle
most of what you need. The idea is that instead of making
your own PersistentTimer, you use the methods on p_timers:
Start, Check, Clear, GetRemainingTime to access them. You
starting a timer which does not exist will create it. If
you need to do more than that with your timer, you should
still use p_timers, just use the Get() method to get direct
access to the PersistentTimer. All timers in the p_timers
list will automatically be loaded and stored to the database.
You should never need to call any Load or Store methods.

To get to p_timers when you are not in the Client:: scope,
use client->GetPTimers(). to access timers.

To use ptimers, you need to create the table below in your DB:

Schema:

CREATE TABLE timers (
	char_id INT(11) NOT NULL,
	type MEDIUMINT UNSIGNED NOT NULL,
	start INT UNSIGNED NOT NULL,
	duration INT UNSIGNED NOT NULL,
	enable TINYINT NOT NULL,
	PRIMARY KEY(char_id, type)
);


*/


//#define DEBUG_PTIMERS


PersistentTimer *PersistentTimer::LoadTimer(Database *db, uint32 char_id, pTimerType type) {
	PersistentTimer *p;
	p = new PersistentTimer(char_id, type, 0);
	if(p->Load(db))
		return(p);
	delete p;
	return(nullptr);
}

PersistentTimer::PersistentTimer(uint32 char_id, pTimerType type, uint32 in_timer_time) {
	_char_id = char_id;
	_type = type;

	timer_time = in_timer_time;
	start_time = get_current_time();
	if (timer_time == 0) {
		enabled = false;
	} else {
		enabled = true;
	}
#ifdef DEBUG_PTIMERS
	printf("New timer: char %lu of type %u at %lu for %lu seconds.\n", (unsigned long)_char_id, _type, (unsigned long)start_time, (unsigned long)timer_time);
#endif
}

PersistentTimer::PersistentTimer(uint32 char_id, pTimerType type, uint32 in_start_time, uint32 in_timer_time, bool in_enable) {
	_char_id = char_id;
	_type = type;

	timer_time = in_timer_time;
	start_time = in_start_time;
	enabled = in_enable;
#ifdef DEBUG_PTIMERS
	printf("New stored timer: char %lu of type %u at %lu for %lu seconds.\n", (unsigned long)_char_id, _type, (unsigned long)start_time, (unsigned long)timer_time);
#endif
}

bool PersistentTimer::Load(Database *db) {

#ifdef DEBUG_PTIMERS
	printf("Loading timer: char %lu of type %u\n", (unsigned long)_char_id, _type);
#endif
    std::string query = StringFormat("SELECT start, duration, enable "
                                    "FROM timers WHERE char_id=%lu AND type=%u",
                                    (unsigned long)_char_id, _type);
    auto results = db->QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	if (results.RowCount() != 1)
        return false;

	auto row = results.begin();

    start_time = strtoul(row[0], nullptr, 10);
    timer_time = strtoul(row[1], nullptr, 10);
    enabled = (row[2][0] == '1');

    return true;
}

bool PersistentTimer::Store(Database *db) {
	if(Expired(db, false))	//dont need to store expired timers.
		return true;

	std::string query = StringFormat("REPLACE INTO timers "
                                    " (char_id, type, start, duration, enable) "
                                    " VALUES (%lu, %u, %lu, %lu, %d)",
                                    (unsigned long)_char_id, _type, (unsigned long)start_time,
                                    (unsigned long)timer_time, enabled ? 1: 0);

#ifdef DEBUG_PTIMERS
	printf("Storing timer: char %lu of type %u: '%s'\n", (unsigned long)_char_id, _type, query.c_str());
#endif
    auto results = db->QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	return true;
}

bool PersistentTimer::Clear(Database *db) {

    std::string query = StringFormat("DELETE FROM timers "
                                    "WHERE char_id = %lu AND type = %u ",
                                    (unsigned long)_char_id, _type);
#ifdef DEBUG_PTIMERS
	printf("Clearing timer: char %lu of type %u: '%s'\n", (unsigned long)_char_id, _type, query.c_str());
#endif

    auto results = db->QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	return true;

}

/* This function checks if the timer triggered */
bool PersistentTimer::Expired(Database *db, bool iReset) {
	uint32 current_time = get_current_time();
	if (current_time-start_time >= timer_time) {
		if (enabled && iReset) {
			start_time = current_time; // Reset timer
		} else if(enabled) {
			Clear(db);	//remove it from DB too
		}
		return(true);
	}

	return(false);
}

/* This function set the timer and restart it */
void PersistentTimer::Start(uint32 set_timer_time) {
	start_time = get_current_time();
	enabled = true;
	if (set_timer_time != 0) {
		timer_time = set_timer_time;
	}
#ifdef DEBUG_PTIMERS
	printf("Starting timer: char %lu of type %u at %lu for %lu seconds.\n", (unsigned long)_char_id, _type, (unsigned long)start_time, (unsigned long)timer_time);
#endif
}

// This timer updates the timer without restarting it
void PersistentTimer::SetTimer(uint32 set_timer_time) {
	// If we were disabled before => restart the timer
	timer_time = set_timer_time;
	if (!enabled) {
		start_time = get_current_time();
		enabled = true;
	}
#ifdef DEBUG_PTIMERS
	printf("Setting timer: char %lu of type %u at %lu for %lu seconds.\n", (unsigned long)_char_id, _type, (unsigned long)start_time, (unsigned long)timer_time);
#endif
}

uint32 PersistentTimer::GetRemainingTime() {
	if (enabled) {
		uint32 current_time = get_current_time();
		if (current_time-start_time > timer_time)
			return 0;
		else
			return (start_time + timer_time) - current_time;
	}
	else {
		return 0xFFFFFFFF;
	}
}


uint32 PersistentTimer::get_current_time() {
	timeval tv;
	gettimeofday(&tv, nullptr);
	return(tv.tv_sec);
}

PTimerList::PTimerList(uint32 char_id) {
	_char_id = char_id;
}

PTimerList::~PTimerList() {
	std::map<pTimerType, PersistentTimer *>::iterator s;
	s = _list.begin();
	while(s != _list.end()) {
		if(s->second != nullptr)
			delete s->second;
		++s;
	}
}


bool PTimerList::Load(Database *db) {

	for (auto timerIterator = _list.begin(); timerIterator != _list.end(); ++timerIterator)
		if(timerIterator->second != nullptr)
			delete timerIterator->second;
	_list.clear();

#ifdef DEBUG_PTIMERS
	printf("Loading all timers for char %lu\n", (unsigned long)_char_id);
#endif
	std::string query = StringFormat("SELECT type, start, duration, enable "
                                    "FROM timers WHERE char_id = %lu",
                                    (unsigned long)_char_id);
    auto results = db->QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	pTimerType type;
	uint32 start_time, timer_time;
	bool enabled;

	PersistentTimer *cur;

    for (auto row = results.begin(); row != results.end(); ++row) {
		type = Strings::ToUnsignedInt(row[0]);
		start_time = Strings::ToUnsignedInt(row[1]);
		timer_time = Strings::ToUnsignedInt(row[2]);
		enabled = (row[3][0] == '1');

		//if it expired allready, dont bother.
		cur = new PersistentTimer(_char_id, type, start_time, timer_time, enabled);
		if(!cur->Expired(nullptr))
			_list[type] = cur;
		else
			delete cur;
	}

	return true;
}

bool PTimerList::Store(Database *db) {
#ifdef DEBUG_PTIMERS
	printf("Storing all timers for char %lu\n", (unsigned long)_char_id);
#endif

	std::map<pTimerType, PersistentTimer *>::iterator s;
	s = _list.begin();
	bool res = true;
	while(s != _list.end()) {
		if(s->second != nullptr) {
#ifdef DEBUG_PTIMERS
	printf("Storing timer %u for char %lu\n", s->first, (unsigned long)_char_id);
#endif
			if(!s->second->Store(db))
				res = false;
		}
		++s;
	}
	return(res);
}

bool PTimerList::Clear(Database *db) {
	_list.clear();

	std::string query = StringFormat("DELETE FROM timers WHERE char_id=%lu ", (unsigned long)_char_id);
#ifdef DEBUG_PTIMERS
	printf("Storing all timers for char %lu: '%s'\n", (unsigned long)_char_id, query.c_str());
#endif
    auto results = db->QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	return true;
}

void PTimerList::Start(pTimerType type, uint32 duration) {
	if(_list.count(type) == 1 && _list[type] != nullptr) {
		_list[type]->Start(duration);
	} else {
		_list[type] = new PersistentTimer(_char_id, type, duration);
	}
}

void PTimerList::Clear(Database *db, pTimerType type) {
	if(_list.count(type) == 1) {
		if(_list[type] != nullptr) {
			_list[type]->Clear(db);
			delete _list[type];
		}
		_list.erase(type);
	}
}

bool PTimerList::Expired(Database *db, pTimerType type, bool reset) {
	if(_list.count(type) != 1)
		return(true);
	if(_list[type] == nullptr)
		return(true);
	return(_list[type]->Expired(db, reset));
}

bool PTimerList::Enabled(pTimerType type) {
	if(_list.count(type) != 1)
		return(false);
	if(_list[type] == nullptr)
		return(false);
	return(_list[type]->Enabled());
}

void PTimerList::Enable(pTimerType type) {
	if(_list.count(type) == 1 && _list[type] != nullptr)
		_list[type]->Enable();
}

void PTimerList::Disable(pTimerType type) {
	if(_list.count(type) == 1 && _list[type] != nullptr)
		_list[type]->Disable();
}

uint32 PTimerList::GetRemainingTime(pTimerType type) {
	if(_list.count(type) != 1)
		return(0);
	if(_list[type] == nullptr)
		return(0);
	return(_list[type]->GetRemainingTime());
}

PersistentTimer *PTimerList::Get(pTimerType type) {
	if(_list.count(type) != 1)
		return(nullptr);
	return(_list[type]);
}

void PTimerList::ToVector(std::vector< std::pair<pTimerType, PersistentTimer *> > &out) {

	std::pair<pTimerType, PersistentTimer *> p;

	std::map<pTimerType, PersistentTimer *>::iterator s;
	s = _list.begin();
	while(s != _list.end()) {
		if(s->second != nullptr) {
			p.first = s->first;
			p.second = s->second;
			out.push_back(p);
		}
		++s;
	}
}

bool PTimerList::ClearOffline(Database *db, uint32 char_id, pTimerType type) {

	std::string query = StringFormat("DELETE FROM timers WHERE char_id=%lu AND type=%u ",(unsigned long)char_id, type);

#ifdef DEBUG_PTIMERS
	printf("Clearing timer (offline): char %lu of type %u: '%s'\n", (unsigned long)char_id, type, query.c_str());
#endif
    auto results = db->QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	return true;
}
