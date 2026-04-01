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

#include "common/repositories/titles_repository.h"

#include <vector>

class Client;
class EQApplicationPacket;

class TitleManager
{
public:
	TitleManager();

	bool LoadTitles();

	EQApplicationPacket* MakeTitlesPacket(Client* c);
	std::string GetPrefix(int title_id);
	std::string GetSuffix(int title_id);
	std::vector<TitlesRepository::Titles> GetEligibleTitles(Client* c);
	bool IsNewAATitleAvailable(int aa_points, int class_id);
	bool IsNewTradeSkillTitleAvailable(int t, int skill_value);
	void CreateNewPlayerTitle(Client* c, std::string title);
	void CreateNewPlayerSuffix(Client* c, std::string suffix);
	bool HasTitle(Client* c, uint32 title_id);
	inline const std::vector<TitlesRepository::Titles>& GetTitles() { return titles; }

protected:
	std::vector<TitlesRepository::Titles> titles;
};

extern TitleManager title_manager;
