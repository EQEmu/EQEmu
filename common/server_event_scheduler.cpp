#include "server_event_scheduler.h"

#include "common/cron/croncpp.h"
#include "common/database.h"
#include "common/strings.h"

#include <chrono>
#include <iostream>
#include <time.h>

ServerEventScheduler::ServerEventScheduler()
{
	m_last_polled_minute = -1;
	m_events             = {};
	m_active_events      = {};
}

ServerEventScheduler::~ServerEventScheduler() = default;

void ServerEventScheduler::LoadScheduledEvents()
{
	if (!ValidateDatabaseConnection()) {
		return;
	}

	std::time_t time = std::time(nullptr);
	std::tm     *now = std::localtime(&time);

	m_events = ServerScheduledEventsRepository::GetWhere(*m_database, "deleted_at is null");
	for (auto &e: m_events) {

		auto start = BuildStartTimeFromEvent(e, now);
		auto end   = BuildEndTimeFromEvent(e, now);

		// data excluded from output because it can be very large

		LogScheduler(
			"Loaded Event ({}) [{}] type [{}] start [{}/{}/{} {:02}:{:02}:00] end [{}/{}/{} {:02}:{:02}:00] cron [{}] created [{}]",
			e.id,
			e.description,
			e.event_type,
			start.tm_mon + 1,
			start.tm_mday,
			start.tm_year + 1900,
			start.tm_hour,
			start.tm_min,
			end.tm_mon + 1,
			end.tm_mday,
			end.tm_year + 1900,
			end.tm_hour,
			end.tm_min,
			e.cron_expression,
			e.created_at
		);
	}

	LogScheduler("Loaded scheduled events [{}]", m_events.size());
}

// checks to see if event is ready to be activated
bool ServerEventScheduler::ValidateEventReadyToActivate(
	ServerScheduledEventsRepository::ServerScheduledEvents &e
)
{

	// if there is a cron expression, it will try to parse it first before falling back to
	// alternative time logic
	if (!e.cron_expression.empty()) {
		try {
			auto        cron      = cron::make_cron<cron::cron_standard_traits>(e.cron_expression);
			std::time_t cron_now  = std::time(nullptr);
			std::time_t cron_next = cron::cron_next(cron, cron_now);

			// we have to pad our now window just a tad so we don't miss the cron window
			if ((cron_now + 10) >= cron_next) {
				LogScheduler("Cron time has been met! Event scheduling ({}) [{}]", e.id, e.description);
				return true;
			}

			LogSchedulerDetail("Cron now [{}] cron next [{}]\n", cron_now, cron_next);
		}
		catch (cron::bad_cronexpr const &ex) {
			LogScheduler(
				"Error: Cron expression error [{}] see [https://github.com/mariusbancila/croncpp#cron-expressions]",
				ex.what()
			);
		}

		return false;
	}

	std::time_t time            = std::time(nullptr);
	std::tm     *now            = std::localtime(&time);
	time_t      now_time_unix   = mktime(now);
	auto        start           = BuildStartTimeFromEvent(e, now);
	auto        end             = BuildEndTimeFromEvent(e, now);
	time_t      start_time_unix = mktime(&start);

	bool doesnt_end = (
		e.year_end == 0 &&
		e.month_end == 0 &&
		e.day_end == 0 &&
		e.hour_end == 0 &&
		e.minute_end == 0
	);

	time_t end_time_unix;
	if (!doesnt_end) {
		end_time_unix = mktime(&end);
	}

	if (now_time_unix >= start_time_unix && (doesnt_end || now_time_unix < end_time_unix)) {
		LogSchedulerDetail(
			"now_time [{}] start_time [{}] doesnt_end [{}] end_time [{}]",
			now_time_unix,
			start_time_unix,
			doesnt_end ? "true" : "false",
			end_time_unix
		);
		return true;
	}

	return false;
}

ServerEventScheduler *ServerEventScheduler::SetDatabase(Database *db)
{
	m_database = db;

	return this;
}

bool ServerEventScheduler::ValidateDatabaseConnection()
{
	if (!m_database) {
		LogError("No database connection");
		return false;
	}

	return true;
}

// in this function we simply look at events we have internally and events
// in the database and determine if any edits have been made
// this helps inform decisions to tell all zones to reload their events
bool ServerEventScheduler::CheckIfEventsChanged()
{
	if (!m_database) {
		return false;
	}

	auto events = ServerScheduledEventsRepository::GetWhere(*m_database, "deleted_at is null");

	// first check if the size changed, if it did this is the easiest step
	if (m_events.size() != events.size()) {
		LogSchedulerDetail("[CheckIfEventsChanged] Event size has changed");
		m_events = events;
		return true;
	}

	// compare fields of database fields to internal events to see if any fields changed
	for (auto &e: m_events) {
		for (auto &dbe: events) {
			if (dbe.id == e.id) {
				if (
					dbe.description != e.description ||
					dbe.event_type != e.event_type ||
					dbe.event_data != e.event_data ||
					dbe.minute_start != e.minute_start ||
					dbe.hour_start != e.hour_start ||
					dbe.day_start != e.day_start ||
					dbe.month_start != e.month_start ||
					dbe.year_start != e.year_start ||
					dbe.minute_end != e.minute_end ||
					dbe.hour_end != e.hour_end ||
					dbe.day_end != e.day_end ||
					dbe.month_end != e.month_end ||
					dbe.year_end != e.year_end ||
					dbe.cron_expression != e.cron_expression ||
					dbe.created_at != e.created_at ||
					dbe.deleted_at != e.deleted_at
					) {
					LogSchedulerDetail("Field change detected");
					m_events = events;
					return true;
				}
			}
		}
	}

	return false;
}

// checks if event is active
bool ServerEventScheduler::IsEventActive(ServerScheduledEventsRepository::ServerScheduledEvents &e)
{
	for (auto &a: m_active_events) {
		if (a.id == e.id) {
			return true;
		}
	}

	return false;
}

bool ServerEventScheduler::RemoveActiveEvent(ServerScheduledEventsRepository::ServerScheduledEvents &e)
{
	m_active_events.erase(
		std::remove_if(
			m_active_events.begin(),
			m_active_events.end(),
			[&](ServerScheduledEventsRepository::ServerScheduledEvents const &active_event) {
				return active_event.id == e.id;
			}
		),
		m_active_events.end());

	return false;
}

std::tm ServerEventScheduler::BuildStartTimeFromEvent(
	ServerScheduledEventsRepository::ServerScheduledEvents &e,
	std::tm *now
)
{
	struct tm time{};
	time.tm_year  = ((e.year_start > 0) ? e.year_start - 1900 : now->tm_year);
	time.tm_mon   = ((e.month_start > 0) ? e.month_start - 1 : now->tm_mon);
	time.tm_mday  = ((e.day_start > 0) ? e.day_start : now->tm_mday);
	time.tm_hour  = ((e.hour_start > 0) ? e.hour_start : now->tm_hour);
	time.tm_min   = ((e.minute_start > 0) ? e.minute_start : now->tm_min);
	time.tm_sec   = 0;
	time.tm_isdst = now->tm_isdst;

	return time;
}

std::tm ServerEventScheduler::BuildEndTimeFromEvent(
	ServerScheduledEventsRepository::ServerScheduledEvents &e,
	std::tm *now
)
{
	struct tm time{};
	time.tm_year  = ((e.year_end > 0) ? e.year_end - 1900 : now->tm_year);
	time.tm_mon   = ((e.month_end > 0) ? e.month_end - 1 : now->tm_mon);
	time.tm_mday  = ((e.day_end > 0) ? e.day_end : now->tm_mday);
	time.tm_hour  = ((e.hour_end > 0) ? e.hour_end : now->tm_hour);
	time.tm_min   = ((e.minute_end > 0) ? e.minute_end : now->tm_min);
	time.tm_sec   = 0;
	time.tm_isdst = now->tm_isdst;

	return time;
}
