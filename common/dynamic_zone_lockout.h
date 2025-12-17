#pragma once

#include "repositories/base/base_dynamic_zone_lockouts_repository.h"

#include <chrono>
#include <string>

class DzLockout
{
public:
	DzLockout() = default;
	DzLockout(std::string uuid, std::string expedition, std::string event, uint64_t expire_time, uint32_t duration);
	DzLockout(std::string_view name, BaseDynamicZoneLockoutsRepository::DynamicZoneLockouts&& lockout);

	static constexpr char ReplayTimer[] = "Replay Timer";

	static DzLockout Create(const std::string& expedition, const std::string& event, uint32_t seconds, std::string uuid = {});

	struct TimeStrings
	{
		std::string days;
		std::string hours;
		std::string mins;
		std::string secs;
	};

	void AddLockoutTime(int seconds);
	uint32_t GetDuration() const { return static_cast<uint32_t>(m_duration.count()); }
	uint64_t GetExpireTime() const { return std::chrono::system_clock::to_time_t(m_expire_time); }
	uint64_t GetStartTime() const { return std::chrono::system_clock::to_time_t(m_expire_time - m_duration); }
	uint32_t GetSecondsRemaining() const;
	TimeStrings GetTimeRemainingStrs() const;
	const std::string& DzName() const { return m_name; }
	const std::string& Event() const { return m_event; }
	const std::string& UUID() const { return m_uuid; }
	bool IsEvent(std::string_view event) const { return m_event == event; }
	bool IsExpired() const { return GetSecondsRemaining() == 0; }
	bool IsReplay() const { return m_is_replay; }
	bool IsSame(const DzLockout& other) const;
	bool IsSame(const std::string& expedition, const std::string& event) const;
	bool IsUUID(const std::string& uuid) const { return uuid == m_uuid; }
	void Reset() { m_expire_time = std::chrono::system_clock::now() + m_duration; }
	void SetDuration(uint32_t seconds) { m_duration = std::chrono::seconds(seconds); }
	void SetExpireTime(uint64_t expire_time) { m_expire_time = std::chrono::system_clock::from_time_t(expire_time); }
	void SetUUID(const std::string& uuid) { m_uuid = uuid; }

	template <typename T>
	void serialize(T& archive);

private:
	bool m_is_replay = false;
	std::string m_uuid; // dz received in
	std::string m_name;
	std::string m_event;
	std::chrono::seconds m_duration = {};
	std::chrono::time_point<std::chrono::system_clock> m_expire_time;
};
