#pragma once

#include "common/timer.h"
#include "common/types.h"
#include "world/adventure_template.h"

#include <cstdlib>
#include <list>
#include <string>

enum AdventureStatus
{
	AS_WaitingForZoneIn,
	AS_WaitingForPrimaryEndTime,
	AS_WaitingForSecondaryEndTime,
	AS_Finished,
};

enum AdventureWinStatus
{
	AWS_Win,
	AWS_SecondPlace,
	AWS_Lose
};

struct AdventureZones
{
	std::string zone;
	int version;
};

struct AdventureZoneIn
{
	int zone_id;
	int door_id;
};

struct AdventureFinishEvent
{
	std::string name;
	bool win;
	int points;
	int theme;
};

struct LeaderboardInfo
{
	std::string name;
	uint32 wins;
	uint32 guk_wins;
	uint32 mir_wins;
	uint32 mmc_wins;
	uint32 ruj_wins;
	uint32 tak_wins;
	uint32 losses;
	uint32 guk_losses;
	uint32 mir_losses;
	uint32 mmc_losses;
	uint32 ruj_losses;
	uint32 tak_losses;
};

class Adventure
{
public:
	Adventure(AdventureTemplate *t);
	Adventure(AdventureTemplate *t, int in_count, int in_assassination_count, AdventureStatus in_status, uint16 in_instance_id, uint32 time_left);
	~Adventure();
	bool Process();
	bool IsActive();
	void AddPlayer(std::string character_name, bool add_client_to_instance = true);
	void RemovePlayer(std::string character_name);
	bool PlayerExists(const std::string& character_name);
	bool CreateInstance();
	void IncrementCount();
	void IncrementAssassinationCount();
	void Finished(AdventureWinStatus ws);
	void SetStatus(AdventureStatus new_status);
	void SendAdventureMessage(uint32 type, const char *msg);
	void MoveCorpsesToGraveyard();

	uint16 GetInstanceID() const { return instance_id; }
	const AdventureTemplate *GetTemplate() const { return adventure_template; }
	AdventureStatus GetStatus() const { return status; }
	std::list<std::string> GetPlayers() { return players; }
	int GetCount() const { return count; }
	int GetAssassinationCount() const { return assassination_count; }
	uint32 GetRemainingTime() const { if(current_timer) { return (current_timer->GetRemainingTime() / 1000); } else { return 0; } }
protected:
	int id;
	int count;
	int assassination_count;
	AdventureTemplate *adventure_template;
	AdventureStatus status;
	std::list<std::string> players;
	Timer *current_timer;
	int instance_id;
};
