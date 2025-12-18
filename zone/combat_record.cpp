#include "combat_record.h"

#include "common/eqemu_logsys.h"
#include "common/strings.h"

void CombatRecord::Start(const std::string& in_mob_name)
{
	m_start_time      = std::time(nullptr);
	m_end_time        = 0;
	m_damage_received = 0;
	m_heal_received   = 0;
	m_mob_name        = in_mob_name;
}


void CombatRecord::Stop()
{
	m_end_time = std::time(nullptr);

	double time_in_combat = TimeInCombat();

	LogCombatRecord(
		"[Summary] Mob [{}] [Received] DPS [{:.0f}] Heal/s [{:.0f}] Duration [{}] ({}s)",
		m_mob_name,
		GetDamageReceivedPerSecond(),
		GetHealedReceivedPerSecond(),
		time_in_combat > 0 ? Strings::SecondsToTime(time_in_combat) : "",
		time_in_combat
	);
}

bool CombatRecord::InCombat() const
{
	return m_start_time > 0;
}

void CombatRecord::ProcessHPEvent(int64 hp, int64 current_hp)
{
	// damage
	if (hp < current_hp) {
		m_damage_received = m_damage_received + std::llabs(current_hp - hp);
	}

	// heal
	if (hp > current_hp && current_hp > 0) {
		m_heal_received = m_heal_received + std::llabs(current_hp - hp);
	}

	LogCombatRecordDetail(
		"damage_received [{}] heal_received [{}] current_hp [{}] hp [{}] calc [{}]",
		m_damage_received,
		m_heal_received,
		current_hp,
		hp,
		std::llabs(current_hp - hp)
	);
}

double CombatRecord::TimeInCombat() const
{
	return m_end_time > m_start_time ? difftime(m_end_time, m_start_time) : 0;
}

float CombatRecord::GetDamageReceivedPerSecond() const
{
	double time_in_combat = TimeInCombat();
	return time_in_combat > 0 ? (m_damage_received / time_in_combat) : m_damage_received;
}

float CombatRecord::GetHealedReceivedPerSecond() const
{
	double time_in_combat = TimeInCombat();
	return time_in_combat > 0 ? (m_heal_received / time_in_combat) : m_heal_received;
}

time_t CombatRecord::GetStartTime() const
{
	return m_start_time;
}

time_t CombatRecord::GetEndTime() const
{
	return m_end_time;
}

int64 CombatRecord::GetDamageReceived() const
{
	return m_damage_received;
}

int64 CombatRecord::GetHealingReceived() const
{
	return m_heal_received;
}
