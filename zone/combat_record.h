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
