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
#include "common/types.h"
#include "zone/masterentity.h"
#include "zone/aa_ability.h"

AA::Rank *AA::Ability::GetRankByPointsSpent(int current_level) {
	if(current_level == 0)
		return nullptr;

	if(!first)
		return nullptr;

	int i = 1;
	Rank *current = first;
	while(current->next) {
		if(i == current_level) {
			break;
		}

		i++;
		current = current->next;
	}

	return current;
}

int AA::Ability::GetMaxLevel(Mob *who) {
	int max_level = 0;
	Rank *current = first;
	while(current) {
		if(!who->CanUseAlternateAdvancementRank(current)) {
			return max_level;
		}

		max_level++;
		current = current->next;
	}

	return max_level;
}
