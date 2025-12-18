/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.org)

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

#include "zone/mob.h"

#define CASTING_CYCLE_MINIMUM_INTERVAL 1000
#define CASTING_CYCLE_MINIMUM_INTERVAL_S 1
#define CASTING_CYCLE_MAXIMUM_INTERVAL 30000
#define CASTING_CYCLE_MAXIMUM_INTERVAL_S 30
#define CASTING_CYCLE_DEFAULT_INTERVAL 5000
#define CASTING_CYCLE_DEFAULT_INTERVAL_S 5
#define POKE_PROPAGATION_DELAY 250
#define ADVANCE_ROTATION_MINIMUM_INTERVAL 100
#define HEALING_STATS_RESET_INTERVAL 60000
#define HEALING_STATS_RESET_INTERVAL_S 60

#define SAFE_HP_RATIO_ABS 100.0f
#define SAFE_HP_RATIO_BASE 95.0f

#define CRITICAL_HP_RATIO_ABS 0.0f
#define CRITICAL_HP_RATIO_BASE 30.0f

struct HealingStats
{
	uint32 last_heal_time_ms;
	uint32 heal_count;
};

// Both members and targets use a shared_ptr to keep track of their HealRotation instance
class HealRotation
{
public:
	HealRotation(Bot* hr_creator, uint32 interval_ms = CASTING_CYCLE_DEFAULT_INTERVAL, bool fast_heals = false, bool adaptive_targeting = false, bool casting_override = false);
	HealRotation(HealRotation* allocator_shunt) {}; // use should be limited to the shared_ptr<HealRotation> memory allocation call

	void SetIntervalMS(uint32 interval_ms);
	void SetIntervalS(uint32 interval_s);
	void SetFastHeals(bool flag) { m_fast_heals = flag; }
	void SetAdaptiveTargeting(bool flag) { m_adaptive_targeting = flag; }
	void SetCastingOverride(bool flag) { m_casting_override = flag; }
	bool AddMemberToPool(Bot* hr_member);
	bool AddTargetToPool(Mob* hr_target);

	uint32 CreationTimeMS() { return m_creation_time_ms; }
	uint32 LastHealTimeMS() { return m_last_heal_time_ms; }
	uint32 IntervalMS() { return m_interval_ms; }
	uint32 IntervalS() { return (m_interval_ms / 1000); }
	bool FastHeals() { return m_fast_heals; }
	bool AdaptiveTargeting() { return m_adaptive_targeting; }
	bool CastingOverride() { return m_casting_override; }
	bool RemoveMemberFromPool(Bot* hr_member);
	bool RemoveTargetFromPool(Mob* hr_target);

	bool ClearMemberPool();
	bool ClearTargetPool();

	Mob* HOTTarget() { return m_hot_target; }
	bool SetHOTTarget(Mob* hot_target);
	bool ClearHOTTarget();

	bool Start();
	bool Stop();

	bool IsActive() { return m_is_active; }
	bool IsHOTActive() { return m_hot_active; }
	bool CastingReady() { return (Timer::GetCurrentTime() >= m_next_cast_time_ms); }
	Bot* CastingMember();
	bool PokeCastingTarget();
	Mob* CastingTarget();
	bool AdvanceRotation(bool use_interval = true);

	std::list<Bot*>* MemberList() { return &m_member_pool; }
	std::list<Bot*>* CycleList() { return &m_cycle_pool; }
	std::list<Mob*>* TargetList() { return &m_target_pool; }

	bool IsMemberInPool(Bot* hr_member);
	bool IsTargetInPool(Mob* hr_target);
	bool IsHOTTarget(Mob* hot_target);

	void SetMemberIsCasting(Bot* hr_member, bool flag = true);
	bool MemberIsCasting(Bot* hr_member);

	void UpdateTargetHealingStats(Mob* hr_target);
	void StartNewTargetHealingStatsCycle(uint32 current_time);
	uint32 HealCount(Mob* hr_target);
	uint32 ExtendedHealCount(Mob* hr_target);
	float HealFrequency(Mob* hr_target);
	float ExtendedHealFrequency(Mob* hr_target);
	HealingStats* TargetHealingStats1(Mob* hr_target);
	HealingStats* TargetHealingStats2(Mob* hr_target);

	bool SetArmorTypeSafeHPRatio(uint8 armor_type, float hp_ratio);
	bool SetArmorTypeCriticalHPRatio(uint8 armor_type, float hp_ratio);

	float ArmorTypeSafeHPRatio(uint8 armor_type);
	float ArmorTypeCriticalHPRatio(uint8 armor_type);

	void ResetArmorTypeHPLimits();

private:
	bool valid_state();
	void cycle_refresh();
	bool healable_target(bool use_class_at = true, bool critical_only = false);
	void bias_targets();
	void validate_hot();

	uint32 m_creation_time_ms;
	uint32 m_last_heal_time_ms;
	uint32 m_interval_ms;
	uint32 m_next_cast_time_ms;
	uint32 m_next_poke_time_ms;
	uint32 m_healing_stats_begin_ms;
	bool m_fast_heals;
	bool m_adaptive_targeting;
	bool m_casting_override;
	bool m_casting_target_poke;
	bool m_active_heal_target;

	bool m_is_active;

	bool m_consumed;

	Mob* m_hot_target;
	bool m_hot_active;

	std::list<Bot*> m_member_pool;
	std::list<Bot*> m_cycle_pool;
	std::list<Mob*> m_target_pool;

	std::map<Bot*, bool> m_member_is_casting;

	std::map<Mob*, HealingStats> m_target_healing_stats_1;
	std::map<Mob*, HealingStats> m_target_healing_stats_2;

	float m_safe_hp_ratio[ARMOR_TYPE_COUNT];
	float m_critical_hp_ratio[ARMOR_TYPE_COUNT];
};

bool IsHealRotationMemberClass(uint8 class_id);
bool IsHealRotationTargetMobType(Mob* target_mob);
