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

#include "common/types.h"

#include <map>
#include <vector>

namespace AA {

class Ability;
class Rank
{
public:
	Rank() { }
	~Rank() { }

	uint32 id;
	int upper_hotkey_sid;
	int lower_hotkey_sid;
	int title_sid;
	int desc_sid;
	int cost;
	int level_req;
	int spell;
	int spell_type;
	int recast_time;
	int prev_id;
	Rank *prev;
	int next_id;
	Rank *next;
	int current_value;
	int expansion;
	int total_cost;
	Ability *base_ability;
	std::vector<RankEffect> effects;
	std::map<int, int> prereqs;
};

} // namespace AA
