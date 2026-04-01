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

#include <ctime>
#include <string>

class CombatRecord {
public:
	void Start(const std::string& in_mob_name);
	void Stop();
	bool InCombat() const;
	void ProcessHPEvent(int64 hp, int64 current_hp);
	double TimeInCombat() const;
	float GetDamageReceivedPerSecond() const;
	float GetHealedReceivedPerSecond() const;
	time_t GetStartTime() const;
	time_t GetEndTime() const;
	int64 GetDamageReceived() const;
	int64 GetHealingReceived() const;
private:
	std::string m_mob_name;
	time_t      m_start_time      = 0;
	time_t      m_end_time        = 0;
	int64       m_damage_received = 0;
	int64       m_heal_received   = 0;
};
