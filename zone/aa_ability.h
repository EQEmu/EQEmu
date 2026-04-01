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

#include "zone/aa_rank_effects.h"
#include "zone/aa_rank.h"

#include <memory>
#include <unordered_map>
#include <vector>

class Mob;

namespace AA {

class Ability
{
public:
	Ability() { }
	~Ability() { }

	Rank *GetRankByPointsSpent(int current_level);
	int GetMaxLevel(Mob *who);

	uint32 id;
	std::string name;
	int category;
	int classes;
	int races;
	int deities;
	int drakkin_heritage;
	int status;
	bool grant_only;
	bool reset_on_death;
	bool auto_grant_enabled;
	int type;
	int charges;
	int first_rank_id;
	Rank *first;
};

} // namespace AA
