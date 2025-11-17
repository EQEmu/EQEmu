#include "dynamic_zone_lockout.h"
#include "strings.h"
#include "rulesys.h"
#include "util/uuid.h"
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <cereal/types/chrono.hpp>

DzLockout::DzLockout(std::string uuid, std::string expedition, std::string event, uint64_t expire_time, uint32_t duration)
	: m_uuid(std::move(uuid))
	, m_name(std::move(expedition))
	, m_event(std::move(event))
	, m_expire_time(std::chrono::system_clock::from_time_t(expire_time))
	, m_duration(duration)
{
	m_is_replay = m_event == ReplayTimer;
}

DzLockout::DzLockout(std::string_view name, BaseDynamicZoneLockoutsRepository::DynamicZoneLockouts&& lockout)
	: m_uuid(std::move(lockout.from_expedition_uuid))
	, m_name(name)
	, m_event(std::move(lockout.event_name))
	, m_expire_time(std::chrono::system_clock::from_time_t(lockout.expire_time))
	, m_duration(lockout.duration)
{
	m_is_replay = m_event == ReplayTimer;
}

DzLockout DzLockout::Create(const std::string& expedition, const std::string& event, uint32_t seconds, std::string uuid)
{
	seconds = static_cast<uint32_t>(seconds * RuleR(Expedition, LockoutDurationMultiplier));

	if (uuid.empty())
	{
		uuid = EQ::Util::UUID::Generate().ToString();
	}

	DzLockout lockout{uuid, expedition, event, 0, seconds};
	lockout.Reset(); // sets expire time
	return lockout;
}

uint32_t DzLockout::GetSecondsRemaining() const
{
	auto now = std::chrono::system_clock::now();
	if (m_expire_time > now)
	{
		auto remaining = m_expire_time - now;
		return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(remaining).count());
	}
	return 0;
}

DzLockout::TimeStrings DzLockout::GetTimeRemainingStrs() const
{
	auto seconds = GetSecondsRemaining();
	return DzLockout::TimeStrings{
		fmt::format_int(seconds / 86400).str(),     // days
		fmt::format_int(seconds / 3600 % 24).str(), // hours
		fmt::format_int(seconds / 60 % 60).str(),   // minutes
		fmt::format_int(seconds % 60).str()         // seconds
	};
}

bool DzLockout::IsSame(const DzLockout& other) const
{
	return other.IsSame(m_name, m_event);
}

bool DzLockout::IsSame(const std::string& expedition, const std::string& event) const
{
	return m_name == expedition && m_event == event;
}

void DzLockout::AddLockoutTime(int seconds)
{
	seconds = static_cast<int>(seconds * RuleR(Expedition, LockoutDurationMultiplier));

	auto new_duration = std::max(0, static_cast<int>(m_duration.count()) + seconds);

	auto start_time = m_expire_time - m_duration;
	m_duration = std::chrono::seconds(new_duration);
	m_expire_time = start_time + m_duration;
}

template <typename T>
void DzLockout::serialize(T& archive)
{
	archive(m_is_replay, m_uuid, m_name, m_event, m_duration, m_expire_time);
}

template void DzLockout::serialize(cereal::BinaryOutputArchive&);
template void DzLockout::serialize(cereal::BinaryInputArchive&);
