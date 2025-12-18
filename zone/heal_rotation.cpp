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

#include "heal_rotation.h"

#include "zone/bot.h"

#define SAFE_HP_RATIO_CLOTH 95.0f
#define SAFE_HP_RATIO_LEATHER 90.0f
#define SAFE_HP_RATIO_CHAIN 80.0f
#define SAFE_HP_RATIO_PLATE 75.0f

#define CRITICAL_HP_RATIO_CLOTH 45.0f
#define CRITICAL_HP_RATIO_LEATHER 40.0f
#define CRITICAL_HP_RATIO_CHAIN 35.0f
#define CRITICAL_HP_RATIO_PLATE 30.0f

HealRotation::HealRotation(Bot* hr_creator, uint32 interval_ms, bool fast_heals, bool adaptive_targeting, bool casting_override)
{
	m_member_pool.push_back(hr_creator);

	m_creation_time_ms = Timer::GetCurrentTime();
	m_last_heal_time_ms = m_creation_time_ms;
	m_interval_ms = ((interval_ms >= CASTING_CYCLE_MINIMUM_INTERVAL) ? (interval_ms) : (CASTING_CYCLE_MINIMUM_INTERVAL));
	m_next_cast_time_ms = m_creation_time_ms;
	m_next_poke_time_ms = m_creation_time_ms;
	m_healing_stats_begin_ms = m_creation_time_ms;
	m_fast_heals = fast_heals;
	m_adaptive_targeting = adaptive_targeting;
	m_casting_override = casting_override;
	m_casting_target_poke = true;
	m_active_heal_target = false;

	ResetArmorTypeHPLimits();

	m_is_active = false;

	m_consumed = false;

	m_hot_target = nullptr;
	m_hot_active = false;
}

void HealRotation::SetIntervalMS(uint32 interval_ms)
{
	if (interval_ms > CASTING_CYCLE_MAXIMUM_INTERVAL)
		interval_ms = CASTING_CYCLE_MAXIMUM_INTERVAL;
	else if (interval_ms < CASTING_CYCLE_MINIMUM_INTERVAL)
		interval_ms = CASTING_CYCLE_MINIMUM_INTERVAL;

	m_interval_ms = interval_ms;
}

void HealRotation::SetIntervalS(uint32 interval_s)
{
	interval_s *= 1000;
	if (interval_s > CASTING_CYCLE_MAXIMUM_INTERVAL)
		interval_s = CASTING_CYCLE_MAXIMUM_INTERVAL;
	else if (interval_s < CASTING_CYCLE_MINIMUM_INTERVAL)
		interval_s = CASTING_CYCLE_MINIMUM_INTERVAL;

	m_interval_ms = interval_s;
}

bool HealRotation::AddMemberToPool(Bot* hr_member)
{
	if (!hr_member)
		return false;
	if (!IsHealRotationMemberClass(hr_member->GetClass()))
		return false;
	if (m_member_pool.size() >= RuleI(Bots, HealRotationMaxMembers))
		return false;

	for (auto find_iter : m_member_pool) {
		if (find_iter == hr_member)
			return false;
	}

	m_member_pool.push_back(hr_member);
	valid_state();

	return true;
}

bool HealRotation::AddTargetToPool(Mob* hr_target)
{
	if (!hr_target)
		return false;
	if (!valid_state())
		return false;
	if (!IsHealRotationTargetMobType(hr_target))
		return false;
	if (m_target_pool.size() >= RuleI(Bots, HealRotationMaxTargets))
		return false;

	for (auto find_iter : m_target_pool) {
		if (find_iter == hr_target)
			return false;
	}

	m_target_pool.push_back(hr_target);

	return true;
}

bool HealRotation::RemoveMemberFromPool(Bot* hr_member)
{
	if (!hr_member)
		return true;

	for (auto member_iter : m_member_pool) {
		if (member_iter != hr_member)
			continue;

		m_member_is_casting.erase(hr_member);
		m_member_pool.remove(hr_member);
		valid_state();
		return true;
	}
	return false;
}

bool HealRotation::RemoveTargetFromPool(Mob* hr_target)
{
	if (!hr_target)
		return true;
	if (!valid_state())
		return true;

	for (auto target_iter : m_target_pool) {
		if (target_iter != hr_target)
			continue;

		if (m_hot_target == hr_target) {
			m_hot_target = nullptr;
			m_hot_active = false;
		}

		m_target_healing_stats_2.erase(hr_target);
		m_target_healing_stats_1.erase(hr_target);
		m_target_pool.remove(hr_target);
		m_casting_target_poke = false;
		bias_targets();
		return true;
	}

	return false;
}

bool HealRotation::ClearMemberPool()
{
	m_is_active = false;
	m_cycle_pool.clear();
	m_casting_target_poke = false;
	m_active_heal_target = false;

	if (!ClearTargetPool())
		LogError("failed to clear m_target_pool (size: [{}])", m_target_pool.size());

	auto& clear_list = const_cast<const std::list<Bot*>&>(m_member_pool);
	for (auto member_iter : clear_list)
		member_iter->LeaveHealRotationMemberPool();

	return true;
}

bool HealRotation::ClearTargetPool()
{
	m_hot_target = nullptr;
	m_hot_active = false;
	m_is_active = false;

	auto& clear_list = const_cast<const std::list<Mob*>&>(m_target_pool);
	for (auto target_iter : clear_list)
		target_iter->LeaveHealRotationTargetPool();

	//m_casting_target_poke = false;
	//bias_targets();

	// strange crash point...
	// bias_targets() should be returning on m_target_pool.empty()
	// and setting this two properties as below
	m_casting_target_poke = true;
	m_active_heal_target = false;
	// instead, the list retains mob shared_ptrs and
	// attempts to process them - and crashes program
	// predominate when adaptive_healing = true
	// (shared_ptr now has a delayed gc action? this did work before...)

	return m_target_pool.empty();
}

bool HealRotation::SetHOTTarget(Mob* hot_target)
{
	if (!hot_target || !IsTargetInPool(hot_target))
		return false;

	m_hot_target = hot_target;
	m_hot_active = true;

	return true;
}

bool HealRotation::ClearHOTTarget()
{
	m_hot_target = nullptr;
	m_hot_active = false;

	return true;
}

bool HealRotation::Start()
{
	m_is_active = false;
	if (m_member_pool.empty() || m_target_pool.empty()) {
		validate_hot();
		return false;
	}

	m_cycle_pool = m_member_pool;
	m_is_active = true;

	return true;
}

bool HealRotation::Stop()
{
	m_is_active = false;
	m_active_heal_target = false;
	m_cycle_pool.clear();

	return true;
}

Bot* HealRotation::CastingMember()
{
	if (!m_is_active && !m_hot_active)
		return nullptr;

	if (m_cycle_pool.empty()) {
		cycle_refresh();

		if (m_cycle_pool.empty())
			return nullptr;
	}

	return m_cycle_pool.front();
}

bool HealRotation::PokeCastingTarget()
{
	if (m_hot_target && m_hot_active)
		return true;

	if (!m_is_active)
		return false;

	uint32 current_time = Timer::GetCurrentTime();

	if (current_time < m_next_poke_time_ms) {
		auto hr_target = CastingTarget();
		if (hr_target && hr_target->DontHealMeBefore() > current_time)
			m_next_poke_time_ms = current_time;
		else
			return m_active_heal_target;
	}

	m_next_poke_time_ms = (current_time + POKE_PROPAGATION_DELAY);

	if (m_healing_stats_begin_ms + HEALING_STATS_RESET_INTERVAL <= current_time)
		StartNewTargetHealingStatsCycle(current_time);

	m_casting_target_poke = false;
	bias_targets();

	return m_active_heal_target;
}

Mob* HealRotation::CastingTarget()
{
	if (m_hot_target && m_hot_active)
		return m_hot_target;

	if (!m_is_active)
		return nullptr;
	if (!m_active_heal_target)
		return nullptr;

	return m_target_pool.front();
}

bool HealRotation::AdvanceRotation(bool use_interval)
{
	m_cycle_pool.pop_front();
	m_next_cast_time_ms = Timer::GetCurrentTime();
	if (use_interval) {
		m_next_poke_time_ms = m_next_cast_time_ms;
		m_next_cast_time_ms += m_interval_ms;
	}
	else {
		m_next_cast_time_ms += ADVANCE_ROTATION_MINIMUM_INTERVAL;
	}

	if (m_cycle_pool.empty())
		cycle_refresh();

	return (!m_cycle_pool.empty());
}

bool HealRotation::IsMemberInPool(Bot* hr_member)
{
	if (!hr_member)
		return false;
	if (m_member_pool.empty())
		return false;

	for (auto find_iter : m_member_pool) {
		if (find_iter == hr_member)
			return true;
	}

	return false;
}

bool HealRotation::IsTargetInPool(Mob* hr_target)
{
	if (!hr_target)
		return false;
	if (m_target_pool.empty())
		return false;

	for (auto find_iter : m_target_pool) {
		if (find_iter == hr_target)
			return true;
	}

	return false;
}

bool HealRotation::IsHOTTarget(Mob* hot_target)
{
	if (!hot_target)
		return false;
	if (m_hot_target != hot_target)
		return false;

	return true;
}

void HealRotation::SetMemberIsCasting(Bot* hr_member, bool flag)
{
	if (!hr_member)
		return;
	if (!IsMemberInPool(hr_member))
		return;

	m_member_is_casting[hr_member] = flag;
}

bool HealRotation::MemberIsCasting(Bot* hr_member)
{
	if (!hr_member)
		return false;
	if (m_member_is_casting.find(hr_member) == m_member_is_casting.end())
		return false;

	return m_member_is_casting[hr_member];
}

void HealRotation::UpdateTargetHealingStats(Mob* hr_target)
{
	if (!hr_target)
		return;
	if (!IsTargetInPool(hr_target))
		return;

	m_last_heal_time_ms = Timer::GetCurrentTime();

	m_target_healing_stats_1[hr_target].last_heal_time_ms = m_last_heal_time_ms;
	++m_target_healing_stats_1[hr_target].heal_count;
}

void HealRotation::StartNewTargetHealingStatsCycle(uint32 current_time)
{
	m_target_healing_stats_2 = m_target_healing_stats_1;
	m_target_healing_stats_1.clear();

	m_healing_stats_begin_ms = current_time;
}

uint32 HealRotation::HealCount(Mob* hr_target)
{
	if (!hr_target)
		return 0;

	uint32 heal_count = 0;
	if (m_target_healing_stats_1.find(hr_target) != m_target_healing_stats_1.end())
		heal_count += m_target_healing_stats_1[hr_target].heal_count;

	return heal_count;
}

uint32 HealRotation::ExtendedHealCount(Mob* hr_target)
{
	if (!hr_target)
		return 0;

	uint32 heal_count = 0;
	if (m_target_healing_stats_1.find(hr_target) != m_target_healing_stats_1.end())
		heal_count += m_target_healing_stats_1[hr_target].heal_count;
	if (m_target_healing_stats_2.find(hr_target) != m_target_healing_stats_2.end())
		heal_count += m_target_healing_stats_2[hr_target].heal_count;

	return heal_count;
}

float HealRotation::HealFrequency(Mob* hr_target)
{
	if (!hr_target)
		return 0.0f;

	float time_base = 0;
	uint32 heal_count = 0;
	if (m_target_healing_stats_1.find(hr_target) != m_target_healing_stats_1.end()) {
		heal_count += m_target_healing_stats_1[hr_target].heal_count;
		time_base = (Timer::GetCurrentTime() - m_target_healing_stats_1[hr_target].last_heal_time_ms);
	}

	time_base /= 1000;
	if (!time_base)
		time_base = HEALING_STATS_RESET_INTERVAL_S;

	if (heal_count)
		return ((float)1 / (time_base / heal_count));
	else
		return ((float)1 / time_base);
}

float HealRotation::ExtendedHealFrequency(Mob* hr_target)
{
	if (!hr_target)
		return 0.0f;

	uint32 current_time = Timer::GetCurrentTime();
	uint32 heal_count = 0;
	float time_base = 0;
	if (m_target_healing_stats_1.find(hr_target) != m_target_healing_stats_1.end()) {
		heal_count += m_target_healing_stats_1[hr_target].heal_count;
		time_base = (current_time - m_target_healing_stats_1[hr_target].last_heal_time_ms + HEALING_STATS_RESET_INTERVAL);
	}
	if (m_target_healing_stats_2.find(hr_target) != m_target_healing_stats_2.end()) {
		heal_count += m_target_healing_stats_2[hr_target].heal_count;
		time_base = (current_time - m_target_healing_stats_2[hr_target].last_heal_time_ms);
	}

	time_base /= 1000;
	if (!time_base)
		time_base = (HEALING_STATS_RESET_INTERVAL_S * 2);

	if (heal_count)
		return ((float)1 / (time_base / heal_count));
	else
		return ((float)1 / time_base);
}

HealingStats* HealRotation::TargetHealingStats1(Mob* hr_target)
{
	if (!hr_target)
		return nullptr;
	if (m_target_healing_stats_1.find(hr_target) == m_target_healing_stats_1.end())
		return nullptr;

	return &m_target_healing_stats_1[hr_target];
}

HealingStats* HealRotation::TargetHealingStats2(Mob* hr_target)
{
	if (!hr_target)
		return nullptr;
	if (m_target_healing_stats_2.find(hr_target) == m_target_healing_stats_2.end())
		return nullptr;

	return &m_target_healing_stats_2[hr_target];
}

bool HealRotation::SetArmorTypeSafeHPRatio(uint8 armor_type, float hp_ratio)
{
	if (armor_type >= ARMOR_TYPE_COUNT)
		return false;
	if (hp_ratio < CRITICAL_HP_RATIO_ABS || hp_ratio > SAFE_HP_RATIO_ABS)
		return false;
	if (hp_ratio < m_critical_hp_ratio[armor_type])
		return false;

	m_safe_hp_ratio[armor_type] = hp_ratio;

	return true;
}

bool HealRotation::SetArmorTypeCriticalHPRatio(uint8 armor_type, float hp_ratio)
{
	if (armor_type >= ARMOR_TYPE_COUNT)
		return false;
	if (hp_ratio < CRITICAL_HP_RATIO_ABS || hp_ratio > SAFE_HP_RATIO_ABS)
		return false;
	if (hp_ratio > m_safe_hp_ratio[armor_type])
		return false;

	m_critical_hp_ratio[armor_type] = hp_ratio;

	return true;
}

float HealRotation::ArmorTypeSafeHPRatio(uint8 armor_type)
{
	if (armor_type < ARMOR_TYPE_COUNT)
		return m_safe_hp_ratio[armor_type];
	else
		return m_safe_hp_ratio[ARMOR_TYPE_UNKNOWN];
}

float HealRotation::ArmorTypeCriticalHPRatio(uint8 armor_type)
{
	if (armor_type < ARMOR_TYPE_COUNT)
		return m_critical_hp_ratio[armor_type];
	else
		return m_critical_hp_ratio[ARMOR_TYPE_UNKNOWN];
}

void HealRotation::ResetArmorTypeHPLimits()
{
	m_safe_hp_ratio[ARMOR_TYPE_UNKNOWN] = SAFE_HP_RATIO_BASE;
	m_safe_hp_ratio[ARMOR_TYPE_CLOTH] = SAFE_HP_RATIO_CLOTH;
	m_safe_hp_ratio[ARMOR_TYPE_LEATHER] = SAFE_HP_RATIO_LEATHER;
	m_safe_hp_ratio[ARMOR_TYPE_CHAIN] = SAFE_HP_RATIO_CHAIN;
	m_safe_hp_ratio[ARMOR_TYPE_PLATE] = SAFE_HP_RATIO_PLATE;

	m_critical_hp_ratio[ARMOR_TYPE_UNKNOWN] = CRITICAL_HP_RATIO_BASE;
	m_critical_hp_ratio[ARMOR_TYPE_CLOTH] = CRITICAL_HP_RATIO_CLOTH;
	m_critical_hp_ratio[ARMOR_TYPE_LEATHER] = CRITICAL_HP_RATIO_LEATHER;
	m_critical_hp_ratio[ARMOR_TYPE_CHAIN] = CRITICAL_HP_RATIO_CHAIN;
	m_critical_hp_ratio[ARMOR_TYPE_PLATE] = CRITICAL_HP_RATIO_PLATE;
}

bool HealRotation::valid_state()
{
	m_member_pool.remove(nullptr);
	m_member_pool.remove_if([](Mob* l) {return (!IsHealRotationMemberClass(l->GetClass())); });

	cycle_refresh();

	if (m_member_pool.empty() && !m_consumed) { // Consumes HealRotation at this point
		m_consumed = true;
		ClearTargetPool();
	}

	return (!m_member_pool.empty());
}

void HealRotation::cycle_refresh()
{
	m_is_active = false;
	m_cycle_pool.clear();
	if (m_member_pool.empty())
		return;

	m_cycle_pool = m_member_pool;

	m_is_active = true;
}

bool HealRotation::healable_target(bool use_class_at, bool critical_only)
{
	if (m_target_pool.empty())
		return false;

	auto healable_target = m_target_pool.front();
	if (!healable_target)
		return false;
	if (healable_target->DontHealMeBefore() > Timer::GetCurrentTime())
		return false;
	if (healable_target->GetAppearance() == eaDead)
		return false;

	if (use_class_at) {
		if (critical_only && healable_target->GetHPRatio() > m_critical_hp_ratio[ClassArmorType(healable_target->GetClass())])
			return false;
		if (healable_target->GetHPRatio() > m_safe_hp_ratio[ClassArmorType(healable_target->GetClass())])
			return false;
		if (healable_target->IsBerserk() && (healable_target->GetClass() == Class::Warrior || healable_target->GetClass() == Class::Berserker)) {
			if (healable_target->GetHPRatio() <= RuleI(Combat, BerserkerFrenzyEnd) && healable_target->GetHPRatio() > m_critical_hp_ratio[ClassArmorType(healable_target->GetClass())])
				return false;
		}
	}
	else {
		if (critical_only && healable_target->GetHPRatio() > CRITICAL_HP_RATIO_BASE)
			return false;
		if (healable_target->GetHPRatio() > SAFE_HP_RATIO_BASE)
			return false;
		if (healable_target->IsBerserk() && (healable_target->GetClass() == Class::Warrior || healable_target->GetClass() == Class::Berserker)) {
			if (healable_target->GetHPRatio() <= RuleI(Combat, BerserkerFrenzyEnd) && healable_target->GetHPRatio() > CRITICAL_HP_RATIO_BASE)
				return false;
		}
	}

	return true;
}

void HealRotation::bias_targets()
{
#define LT_HPRATIO(l, r) (l->GetHPRatio() < r->GetHPRatio())
#define LT_ARMTYPE(l, r) (ClassArmorType(l->GetClass()) < ClassArmorType(r->GetClass()))

#define EQ_ALIVE(l, r) (l->GetAppearance() != eaDead && r->GetAppearance() != eaDead)
#define EQ_READY(l, r, ct) (l->DontHealMeBefore() <= ct && r->DontHealMeBefore() <= ct)
#define EQ_TANK(l, r) ((l->HasGroup() && l->GetGroup()->AmIMainTank(l->GetCleanName())) && (r->HasGroup() && r->GetGroup()->AmIMainTank(r->GetCleanName())))
#define EQ_HEALER(l, r) (IsHealRotationMemberClass(l->GetClass()) && IsHealRotationMemberClass(r->GetClass()))
#define EQ_ARMTYPE(l, r) (ClassArmorType(l->GetClass()) == ClassArmorType(r->GetClass()))
#define EQ_ATCRIT(l, r) (l->GetHPRatio() <= (*l->TargetOfHealRotation())->ArmorTypeCriticalHPRatio(ClassArmorType(l->GetClass())) && \
	r->GetHPRatio() <= (*r->TargetOfHealRotation())->ArmorTypeCriticalHPRatio(ClassArmorType(r->GetClass())))
#define EQ_ATWOUND(l, r) (l->GetHPRatio() <= (*l->TargetOfHealRotation())->ArmorTypeSafeHPRatio(ClassArmorType(l->GetClass())) && \
	r->GetHPRatio() <= (*r->TargetOfHealRotation())->ArmorTypeSafeHPRatio(ClassArmorType(r->GetClass())))

#define GT_ALIVE(l, r) (l->GetAppearance() != eaDead && r->GetAppearance() == eaDead)
#define GT_READY(l, r, ct) (l->DontHealMeBefore() <= ct && r->DontHealMeBefore() > ct)
#define GT_TANK(l, r) ((l->HasGroup() && l->GetGroup()->AmIMainTank(l->GetCleanName())) && (!r->HasGroup() || !r->GetGroup()->AmIMainTank(r->GetCleanName())))
#define GT_HEALER(l, r) (IsHealRotationMemberClass(l->GetClass()) && !IsHealRotationMemberClass(r->GetClass()))
#define GT_HEALFREQ(l, r) (l->HealRotationHealFrequency() > r->HealRotationHealFrequency())
#define GT_HEALCNT(l, r) (l->HealRotationHealCount() > r->HealRotationHealCount())
#define GT_ATCRIT(l, r) (l->GetHPRatio() <= (*l->TargetOfHealRotation())->ArmorTypeCriticalHPRatio(ClassArmorType(l->GetClass())) && \
	r->GetHPRatio() > (*r->TargetOfHealRotation())->ArmorTypeCriticalHPRatio(ClassArmorType(r->GetClass())))
#define GT_XHEALFREQ(l, r) (l->HealRotationExtendedHealFrequency() > r->HealRotationExtendedHealFrequency())
#define GT_XHEALCNT(l, r) (l->HealRotationExtendedHealCount() > r->HealRotationExtendedHealCount())
#define GT_ATWOUND(l, r) (l->GetHPRatio() <= (*l->TargetOfHealRotation())->ArmorTypeSafeHPRatio(ClassArmorType(l->GetClass())) && \
	r->GetHPRatio() > (*r->TargetOfHealRotation())->ArmorTypeSafeHPRatio(ClassArmorType(r->GetClass())))

	if (m_target_pool.empty()) {
		m_casting_target_poke = true;
		m_active_heal_target = false;
		return;
	}

	// attempt to clear invalid target pool entries
	m_target_pool.remove(nullptr);
	m_target_pool.remove_if([](Mob* l) {
		try { return (!IsHealRotationTargetMobType(l)); }
		catch (...) { return true; }
	});

	uint32 sort_type = 0; // debug

	while (m_target_pool.size() > 1 && !m_casting_target_poke && !m_adaptive_targeting) { // standard behavior
		sort_type = 1;
		m_target_pool.sort([](Mob* l, Mob* r) {
			if (GT_ALIVE(l, r))
				return true;
			uint32 current_time = Timer::GetCurrentTime();
			if (EQ_ALIVE(l, r) && GT_READY(l, r, current_time))
				return true;
			if (EQ_ALIVE(l, r) && EQ_READY(l, r, current_time) && GT_TANK(l, r))
				return true;
			if (EQ_ALIVE(l, r) && EQ_READY(l, r, current_time) && EQ_TANK(l, r) && LT_HPRATIO(l, r))
				return true;

			return false;
		});
		if (m_target_pool.front()->HasGroup() && m_target_pool.front()->GetGroup()->AmIMainTank(m_target_pool.front()->GetCleanName()) && healable_target(false))
			break;

		sort_type = 2;
		m_target_pool.sort([](Mob* l, Mob* r) {
			if (GT_ALIVE(l, r))
				return true;
			uint32 current_time = Timer::GetCurrentTime();
			if (EQ_ALIVE(l, r) && GT_READY(l, r, current_time))
				return true;
			if (EQ_ALIVE(l, r) && EQ_READY(l, r, current_time) && GT_HEALER(l, r))
				return true;
			if (EQ_ALIVE(l, r) && EQ_READY(l, r, current_time) && EQ_HEALER(l, r) && LT_HPRATIO(l, r))
				return true;

			return false;
		});
		if (IsHealRotationMemberClass(m_target_pool.front()->GetClass()) && healable_target(false))
			break;

		sort_type = 3; // default
		m_target_pool.sort([](const Mob* l, const Mob* r) {
			if (GT_ALIVE(l, r))
				return true;
			uint32 current_time = Timer::GetCurrentTime();
			if (EQ_ALIVE(l, r) && GT_READY(l, r, current_time))
				return true;
			if (EQ_ALIVE(l, r) && EQ_READY(l, r, current_time) && LT_HPRATIO(l, r))
				return true;

			return false;
		});

		break;
	}

	while (m_target_pool.size() > 1 && !m_casting_target_poke && m_adaptive_targeting) { // adaptive targeting behavior
		sort_type = 101;
		m_target_pool.sort([](Mob* l, Mob* r) {
			if (GT_ALIVE(l, r))
				return true;
			uint32 current_time = Timer::GetCurrentTime();
			if (EQ_ALIVE(l, r) && GT_READY(l, r, current_time))
				return true;
			if (EQ_ALIVE(l, r) && EQ_READY(l, r, current_time) && GT_HEALFREQ(l, r))
				return true;

			return false;
		});
		if (healable_target(true, true))
			break;

		sort_type = 102;
		m_target_pool.sort([](Mob* l, Mob* r) {
			if (GT_ALIVE(l, r))
				return true;
			uint32 current_time = Timer::GetCurrentTime();
			if (EQ_ALIVE(l, r) && GT_READY(l, r, current_time))
				return true;
			if (EQ_ALIVE(l, r) && EQ_READY(l, r, current_time) && GT_HEALCNT(l, r))
				return true;

			return false;
		});
		if (healable_target(true, true))
			break;

		sort_type = 103;
		m_target_pool.sort([](Mob* l, Mob* r) {
			if (GT_ALIVE(l, r))
				return true;
			uint32 current_time = Timer::GetCurrentTime();
			if (EQ_ALIVE(l, r) && GT_READY(l, r, current_time))
				return true;
			if (EQ_ALIVE(l, r) && EQ_READY(l, r, current_time) && GT_TANK(l, r))
				return true;
			if (EQ_ALIVE(l, r) && EQ_READY(l, r, current_time) && EQ_TANK(l, r) && LT_HPRATIO(l, r))
				return true;

			return false;
		});
		if (m_target_pool.front()->HasGroup() && m_target_pool.front()->GetGroup()->AmIMainTank(m_target_pool.front()->GetCleanName()) && healable_target(true, true))
			break;

		sort_type = 104;
		m_target_pool.sort([](const Mob* l, const Mob* r) {
			if (GT_ALIVE(l, r))
				return true;
			uint32 current_time = Timer::GetCurrentTime();
			if (EQ_ALIVE(l, r) && GT_READY(l, r, current_time))
				return true;
			if (EQ_ALIVE(l, r) && EQ_READY(l, r, current_time) && GT_HEALER(l, r))
				return true;
			if (EQ_ALIVE(l, r) && EQ_READY(l, r, current_time) && EQ_HEALER(l, r) && LT_HPRATIO(l, r))
				return true;

			return false;
		});
		if (IsHealRotationMemberClass(m_target_pool.front()->GetClass()) && healable_target(true, true))
			break;

		sort_type = 105;
		m_target_pool.sort([](const Mob* l, const Mob* r) {
			if (GT_ALIVE(l, r))
				return true;
			uint32 current_time = Timer::GetCurrentTime();
			if (EQ_ALIVE(l, r) && GT_READY(l, r, current_time))
				return true;
			if (EQ_ALIVE(l, r) && EQ_READY(l, r, current_time) && GT_ATCRIT(l, r))
				return true;
			if (EQ_ALIVE(l, r) && EQ_READY(l, r, current_time) && EQ_ATCRIT(l, r) && LT_ARMTYPE(l, r))
				return true;
			if (EQ_ALIVE(l, r) && EQ_READY(l, r, current_time) && EQ_ATCRIT(l, r) && EQ_ARMTYPE(l, r) && LT_HPRATIO(l, r))
				return true;

			return false;
		});
		if (healable_target(true, true))
			break;

		sort_type = 106;
		m_target_pool.sort([](Mob* l, Mob* r) {
			if (GT_ALIVE(l, r))
				return true;
			uint32 current_time = Timer::GetCurrentTime();
			if (EQ_ALIVE(l, r) && GT_READY(l, r, current_time))
				return true;
			if (EQ_ALIVE(l, r) && EQ_READY(l, r, current_time) && GT_XHEALFREQ(l, r))
				return true;

			return false;
		});
		if (healable_target(true))
			break;

		sort_type = 107;
		m_target_pool.sort([](Mob* l, Mob* r) {
			if (GT_ALIVE(l, r))
				return true;
			uint32 current_time = Timer::GetCurrentTime();
			if (EQ_ALIVE(l, r) && GT_READY(l, r, current_time))
				return true;
			if (EQ_ALIVE(l, r) && EQ_READY(l, r, current_time) && GT_XHEALCNT(l, r))
				return true;

			return false;
		});
		if (healable_target(true))
			break;

		sort_type = 108;
		m_target_pool.sort([](const Mob* l, const Mob* r) {
			if (GT_ALIVE(l, r))
				return true;
			uint32 current_time = Timer::GetCurrentTime();
			if (EQ_ALIVE(l, r) && GT_READY(l, r, current_time))
				return true;
			if (EQ_ALIVE(l, r) && EQ_READY(l, r, current_time) && GT_ATWOUND(l, r))
				return true;
			if (EQ_ALIVE(l, r) && EQ_READY(l, r, current_time) && EQ_ATWOUND(l, r) && LT_ARMTYPE(l, r))
				return true;
			if (EQ_ALIVE(l, r) && EQ_READY(l, r, current_time) && EQ_ATWOUND(l, r) && EQ_ARMTYPE(l, r) && LT_HPRATIO(l, r))
				return true;

			return false;
		});
		if (healable_target())
			break;

		sort_type = 109; // default
		m_target_pool.sort([](const Mob* l, const Mob* r) {
			if (GT_ALIVE(l, r))
				return true;
			uint32 current_time = Timer::GetCurrentTime();
			if (EQ_ALIVE(l, r) && GT_READY(l, r, current_time))
				return true;
			if (EQ_ALIVE(l, r) && EQ_READY(l, r, current_time) && LT_HPRATIO(l, r))
				return true;

			return false;
		});

		break;
	}

	m_active_heal_target = healable_target(false);
	if (!m_active_heal_target)
		m_active_heal_target = healable_target();

	m_casting_target_poke = true;

#if (EQDEBUG >= 12)
	LogError("HealRotation::bias_targets() - *** Post-processing state ***");
	LogError("HealRotation Settings:");
	LogError("HealRotation::m_interval_ms = [{}]", m_interval_ms);
	LogError("HealRotation::m_next_cast_time_ms = [{}] (current_time: [{}], time_diff: [{}])", m_next_cast_time_ms, Timer::GetCurrentTime(), ((int32)Timer::GetCurrentTime() - (int32)m_next_cast_time_ms));
	LogError("HealRotation::m_next_poke_time_ms = [{}] (current_time: [{}], time_diff: [{}])", m_next_poke_time_ms, Timer::GetCurrentTime(), ((int32)Timer::GetCurrentTime() - (int32)m_next_poke_time_ms));
	LogError("HealRotation::m_fast_heals = [{}]", ((m_fast_heals) ? ("true") : ("false")));
	LogError("HealRotation::m_adaptive_targeting = [{}]", ((m_adaptive_targeting) ? ("true") : ("false")));
	LogError("HealRotation::m_casting_override = [{}]", ((m_casting_override) ? ("true") : ("false")));
	LogError("HealRotation::m_casting_target_poke = [{}]", ((m_casting_target_poke) ? ("true") : ("false")));
	LogError("HealRotation::m_active_heal_target = [{}]", ((m_active_heal_target) ? ("true") : ("false")));
	LogError("HealRotation::m_is_active = [{}]", ((m_is_active) ? ("true") : ("false")));
	LogError("HealRotation::m_member_list.size() = [{}]", m_member_pool.size());
	LogError("HealRotation::m_cycle_list.size() = [{}]", m_cycle_pool.size());
	LogError("HealRotation::m_target_list.size() = [{}]", m_target_pool.size());
	if (m_member_pool.size()) { LogError("(std::shared_ptr<HealRotation>::use_count() = [{}]", m_member_pool.front()->MemberOfHealRotation()->use_count()); }
	else { LogError("(std::shared_ptr<HealRotation>::use_count() = unknown (0)"); }
	LogError("HealRotation Members:");
	int member_index = 0;
	for (auto mlist_iter : m_member_pool) {
		if (!mlist_iter) { continue; }
		LogError("([{}]) [{}] (hrcast: [{}])", (++member_index), mlist_iter->GetCleanName(), ((mlist_iter->AmICastingForHealRotation())?('T'):('F')));
	}
	if (!member_index) { LogError("(0) None"); }
	LogError("HealRotation Cycle:");
	int cycle_index = 0;
	for (auto clist_iter : m_cycle_pool) {
		if (!clist_iter) { continue; }
		LogError("([{}]) [{}]", (++cycle_index), clist_iter->GetCleanName());
	}
	if (!cycle_index) { LogError("(0) None"); }
	LogError("HealRotation Targets: (sort type: [{}])", sort_type);
	int target_index = 0;

	for (auto tlist_iter : m_target_pool) {
		if (!tlist_iter) { continue; }
		LogError("([{}]) [{}] (hp: [{}], at: [{}], dontheal: [{}], crit(base): [{}]([{}]), safe(base): [{}]([{}]), hcnt(ext): [{}]([{}]), hfreq(ext): [{}]([{}]))",
			(++target_index), tlist_iter->GetCleanName(),
			tlist_iter->GetHPRatio(),
			ClassArmorType(tlist_iter->GetClass()),
			((tlist_iter->DontHealMeBefore() > Timer::GetCurrentTime()) ? ('T') : ('F')),
			((tlist_iter->GetHPRatio()>m_critical_hp_ratio[ClassArmorType(tlist_iter->GetClass())]) ? ('F') : ('T')),
			((tlist_iter->GetHPRatio()>m_critical_hp_ratio[ARMOR_TYPE_UNKNOWN]) ? ('F') : ('T')),
			((tlist_iter->GetHPRatio()>m_safe_hp_ratio[ClassArmorType(tlist_iter->GetClass())]) ? ('T') : ('F')),
			((tlist_iter->GetHPRatio()>m_safe_hp_ratio[ARMOR_TYPE_UNKNOWN]) ? ('T') : ('F')),
			tlist_iter->HealRotationHealCount(),
			tlist_iter->HealRotationExtendedHealCount(),
			tlist_iter->HealRotationHealFrequency(),
			tlist_iter->HealRotationExtendedHealFrequency());
	}
	if (!target_index) { LogError("(0) None (hp: 0.0\%, at: 0, dontheal: F, crit(base): F(F), safe(base): F(F), hcnt(ext): 0(0), hfreq(ext): 0.0(0.0))"); }
#endif
}

void HealRotation::validate_hot()
{
	if (!m_hot_target) {
		m_hot_active = false;
		return;
	}

	if (!IsTargetInPool(m_hot_target)) {
		m_hot_target = nullptr;
		m_hot_active = false;
	}
}

bool IsHealRotationMemberClass(uint8 class_id)
{
	switch (class_id) {
	case Class::Cleric:
	case Class::Druid:
	case Class::Shaman:
		return true;
	default:
		return false;
	}
}

bool IsHealRotationTargetMobType(Mob* target_mob)
{
	if (!target_mob)
		return false;
	if (!target_mob->IsClient() && !target_mob->IsBot() && !target_mob->IsPet())
		return false;
	if (target_mob->IsPet() && (!target_mob->GetOwner() || (!target_mob->GetOwner()->IsClient() && !target_mob->GetOwner()->IsBot())))
		return false;

	return true;
}
