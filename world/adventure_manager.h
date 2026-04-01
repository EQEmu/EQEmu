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

#include "common/timer.h"
#include "common/types.h"
#include "world/adventure_template.h"
#include "world/adventure.h"

#include <map>
#include <list>

class AdventureManager
{
public:
	AdventureManager();
	~AdventureManager();

	void Process();

	bool LoadAdventureTemplates();
	bool LoadAdventureEntries();
	void LoadLeaderboardInfo();

	void CalculateAdventureRequestReply(const char *data);
	void PlayerClickedDoor(const char *player, int zone_id, int door_id);
	void TryAdventureCreate(const char *data);
	void GetAdventureData(Adventure *adv);
	void GetAdventureData(const char *name);
	void LeaveAdventure(const char *name);
	void IncrementCount(uint16 instance_id);
	void IncrementAssassinationCount(uint16 instance_id);
	void DoLeaderboardRequest(const char* player, uint8 type);
	void SendAdventureFinish(AdventureFinishEvent fe);
	void AddFinishedEvent(AdventureFinishEvent fe) { finished_list.push_back(fe); Save(); }
	bool PopFinishedEvent(const char *name, AdventureFinishEvent &fe);
	void Save();

	Adventure **GetFinishedAdventures(const char *player, int &count);
	Adventure *GetActiveAdventure(const char *player);
	AdventureTemplate *GetAdventureTemplate(int theme, int id);
	AdventureTemplate *GetAdventureTemplate(int id);
	void GetZoneData(uint16 instance_id);

	static AdventureManager* Instance()
	{
		static AdventureManager instance;
		return &instance;
	}
protected:
	bool IsInExcludedZoneList(std::list<AdventureZones> excluded_zones, std::string zone_name, int version);
	bool IsInExcludedZoneInList(std::list<AdventureZoneIn> excluded_zone_ins, int zone_id, int door_object);
	void DoLeaderboardRequestWins(const char* player);
	void DoLeaderboardRequestPercentage(const char* player);
	void DoLeaderboardRequestWinsGuk(const char* player);
	void DoLeaderboardRequestPercentageGuk(const char* player);
	void DoLeaderboardRequestWinsMir(const char* player);
	void DoLeaderboardRequestPercentageMir(const char* player);
	void DoLeaderboardRequestWinsMmc(const char* player);
	void DoLeaderboardRequestPercentageMmc(const char* player);
	void DoLeaderboardRequestWinsRuj(const char* player);
	void DoLeaderboardRequestPercentageRuj(const char* player);
	void DoLeaderboardRequestWinsTak(const char* player);
	void DoLeaderboardRequestPercentageTak(const char* player);

	std::map<uint32, AdventureTemplate*> adventure_templates;
	std::map<uint32, std::list<AdventureTemplate*> > adventure_entries;
	std::list<Adventure*> adventure_list;
	std::list<AdventureFinishEvent> finished_list;
	std::list<LeaderboardInfo> leaderboard_info_wins;
	std::list<LeaderboardInfo> leaderboard_info_percentage;
	std::list<LeaderboardInfo> leaderboard_info_wins_guk;
	std::list<LeaderboardInfo> leaderboard_info_percentage_guk;
	std::list<LeaderboardInfo> leaderboard_info_wins_mir;
	std::list<LeaderboardInfo> leaderboard_info_percentage_mir;
	std::list<LeaderboardInfo> leaderboard_info_wins_mmc;
	std::list<LeaderboardInfo> leaderboard_info_percentage_mmc;
	std::list<LeaderboardInfo> leaderboard_info_wins_ruj;
	std::list<LeaderboardInfo> leaderboard_info_percentage_ruj;
	std::list<LeaderboardInfo> leaderboard_info_wins_tak;
	std::list<LeaderboardInfo> leaderboard_info_percentage_tak;
	bool leaderboard_sorted_wins;
	bool leaderboard_sorted_percentage;
	bool leaderboard_sorted_wins_guk;
	bool leaderboard_sorted_percentage_guk;
	bool leaderboard_sorted_wins_mir;
	bool leaderboard_sorted_percentage_mir;
	bool leaderboard_sorted_wins_mmc;
	bool leaderboard_sorted_percentage_mmc;
	bool leaderboard_sorted_wins_ruj;
	bool leaderboard_sorted_percentage_ruj;
	bool leaderboard_sorted_wins_tak;
	bool leaderboard_sorted_percentage_tak;
	Timer *process_timer;
	Timer *save_timer;
	Timer *leaderboard_info_timer;
};
