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

#pragma once

#include "common/types.h"

#include <map>
#include <vector>

enum : int {	//values for pTimerType
	pTimerNegativeItemReuse = -1, // these grow down basically, we will have item ID * -1 for the timer ID
	pTimerStartAdventureTimer = 1,
	pTimerSurnameChange = 2,
	pTimerFeignDeath = 3,
	pTimerSneak = 4,
	pTimerHide = 5,
	pTimerTaunt = 6,
	pTimerInstillDoubt = 7,
	pTimerFishing = 8,
	pTimerForaging = 9,
	pTimerMend = 10,
	pTimerTracking = 11,
	pTimerSenseTraps = 12,
	pTimerDisarmTraps = 13,
	pTimerDisciplineReuseStart = 14,
	pTimerDisciplineReuseEnd = 24, // client actually has 20 ids, but still no disc go that high even on live
	pTimerCombatAbility = 25,
	pTimerCombatAbility2 = 26, // RoF2+ Tiger Claw is unlinked from other monk skills, generic in case other classes ever need it
	pTimerBeggingPickPocket = 27,
	pTimerLinkedSpellReuseStart = 28,
	pTimerLinkedSpellReuseEnd = 48,
	pTimerClearXTarget = 50,

	pTimerShieldAbility = 86,

	pTimerLayHands = 87,		//these IDs are used by client too
	pTimerHarmTouch = 89,		//so dont change them

	pTimerItemStart = 100,
	pTimerItemEnd = 200, //I don't think any items would use one this high but hey, incase.
	pTimerPeqzoneReuse = 900,
	pTimerMercReuse = 901,
	pTimerMercSuspend = 902,
	pTimerAAStart = 1000,		//AA re-use timers
	pTimerAAEnd = 2999,
	pTimerAAEffectStart = 3001,	//AA effect timers
	pTimerAAEffectEnd	= 4999,

	pTimerSpellStart = 5000		//Do not put any timer IDs above this one
								//if needed, increase its starting ID
};

class Database;

typedef uint16 pTimerType;

class PersistentTimer {
public:
	static PersistentTimer *LoadTimer(Database *db, uint32 char_id, pTimerType type);

	PersistentTimer(uint32 char_id, pTimerType type, uint32 duration);
	PersistentTimer(uint32 char_id, pTimerType type, uint32 start_time, uint32 duration, bool enable);

	bool Expired(Database *db, bool iReset = true);
	void Start(uint32 set_timer_time=0);

	void SetTimer(uint32 set_timer_time=0);
	uint32 GetRemainingTime();
	inline void Enable() { enabled = true; }
	inline void Disable() { enabled = false; }
	inline const uint32 GetTimerTime() const { return timer_time; }
	inline const uint32 GetStartTime() const { return start_time; }
	inline const pTimerType GetType() const { return _type; }
	inline const uint32 GetReadyTimestamp() const { return start_time + timer_time; }

	inline bool Enabled() { return enabled; }

	bool Load(Database *db);
	bool Store(Database *db);
	bool Clear(Database *db);

protected:
	uint32 get_current_time();

	uint32	start_time;
	uint32	timer_time;
	bool	enabled;

	uint32 _char_id;
	pTimerType _type;
};

//a list of persistent timers for a specific character
class PTimerList {
public:
	PTimerList(uint32 char_id = 0);

	~PTimerList();

	bool Load(Database *db);
	bool Store(Database *db);
	bool Clear(Database *db);

	void Start(pTimerType type, uint32 duration);
	bool Expired(Database *db, pTimerType type, bool reset = true);
	void Clear(Database *db, pTimerType type);
	void Enable(pTimerType type);
	bool Enabled(pTimerType type);
	void Disable(pTimerType type);
	uint32 GetRemainingTime(pTimerType type);
	PersistentTimer *Get(pTimerType type);

	inline void SetCharID(uint32 char_id) { _char_id = char_id; }

	void ToVector(std::vector< std::pair<pTimerType, PersistentTimer *> > &out);

	//Clear a timer for a char not logged in
	//this is not defined on a char which is logged in!
	static bool ClearOffline(Database *db, uint32 char_id, pTimerType type);

	typedef std::map<pTimerType, PersistentTimer *>::iterator iterator;
	iterator begin() { return(_list.begin()); }
	iterator end() { return(_list.end()); }


	void AddTimer(pTimerType type, uint32 start_time, uint32 duration, bool enable);
protected:
	uint32 _char_id;

	std::map<pTimerType, PersistentTimer *> _list;
};
