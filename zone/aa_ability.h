/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2015 EQEMu Development Team (http://eqemulator.net)

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
