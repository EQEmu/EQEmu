#include "zone_event_scheduler.h"

#include <ctime>

void ZoneEventScheduler::Process(Zone *zone, WorldContentService *content_service)
{
	std::time_t time = std::time(nullptr);
	std::tm     *now = std::localtime(&time);

	// once a minute polling
	if (m_last_polled_minute != now->tm_min) {
		int month = (now->tm_mon + 1);
		int year  = (now->tm_year + 1900);

		LogSchedulerDetail(
			"Polling year [{}] month [{}] day [{}] hour [{}] minute [{}]",
			year,
			month,
			now->tm_mday,
			now->tm_hour,
			now->tm_min
		);

		// because stored active events could have a reference of time that has been changed since
		// the time has been updated, we need to make sure we update internal fields so that
		// the scheduler can properly end events if we set a new end date
		SyncEventDataWithActiveEvents();

		// active events
		for (auto &e: m_active_events) {
			LogSchedulerDetail("Looping active event [{}]", e.description);

			// if event becomes no longer active
			if (!ValidateEventReadyToActivate(e)) {
				LogSchedulerDetail("Looping active event validated [{}]", e.event_type);
				if (e.event_type == ServerEvents::EVENT_TYPE_HOT_ZONE_ACTIVE) {
					LogScheduler("Deactivating event [{}] disabling hotzone status", e.description);
					if (Strings::SearchDelim(e.event_data, zone->GetShortName()) != std::string::npos) {
						zone->SetIsHotzone(false);
					}
					RemoveActiveEvent(e);
				}

				if (e.event_type == ServerEvents::EVENT_TYPE_RULE_CHANGE) {
					LogScheduler("Deactivating event [{}] resetting rules to normal", e.description);
					RuleManager::Instance()->LoadRules(m_database, RuleManager::Instance()->GetActiveRuleset(), true);

					// force active events clear and reapply all active events because we reset the entire state
					// ideally if we could revert only the state of which was originally set we would only remove one active event
					m_active_events.clear();
				}

				if (e.event_type == ServerEvents::EVENT_TYPE_CONTENT_FLAG_CHANGE) {
					auto flag_name = e.event_data;
					if (!flag_name.empty()) {
						LogScheduler("Deactivating event [{}] resetting content flags", e.description);
						content_service->ReloadContentFlags();
					}

					// force active events clear and reapply all active events because we reset the entire state
					// ideally if we could revert only the state of which was originally set we would only remove one active event
					m_active_events.clear();
				}
			}
		}

		// check for active
		for (auto &e: m_events) {

			// discard uninteresting events as its less work to calculate time on events we don't care about
			// different processes are interested in different events
			if (
				e.event_type != ServerEvents::EVENT_TYPE_HOT_ZONE_ACTIVE &&
				e.event_type != ServerEvents::EVENT_TYPE_CONTENT_FLAG_CHANGE &&
				e.event_type != ServerEvents::EVENT_TYPE_RULE_CHANGE
				) {
				continue;
			}

			// the scheduler as of today manipulates events in memory and is preferred to be that way
			// the scheduler changes temporary "state" in the server for a period of time for things such as
			// hotzone activation, content flag activation, rule value activation
			// when these events expire, the events become untoggled in memory
			// there can be support for one-time events that are more suitable to run from worlds scheduler
			// such as broadcasts, reloads
			if (ValidateEventReadyToActivate(e) && !IsEventActive(e)) {
				if (e.event_type == ServerEvents::EVENT_TYPE_HOT_ZONE_ACTIVE) {
					if (Strings::SearchDelim(e.event_data, zone->GetShortName()) != std::string::npos) {
						zone->SetIsHotzone(true);
						LogScheduler("Activating Event [{}] Enabling zone as hotzone", e.description);
					}
					m_active_events.push_back(e);
				}

				if (e.event_type == ServerEvents::EVENT_TYPE_RULE_CHANGE) {
					auto params     = Strings::Split(e.event_data, '=');
					auto rule_key   = params[0];
					auto rule_value = params[1];
					if (!rule_key.empty() && !rule_value.empty()) {
						LogScheduler(
							"Activating Event [{}] scheduled rule change, setting rule [{}] to [{}]",
							e.description,
							rule_key,
							rule_value
						);
						RuleManager::Instance()->SetRule(rule_key, rule_value, nullptr, false, true);
					}
					m_active_events.push_back(e);
				}

				if (e.event_type == ServerEvents::EVENT_TYPE_CONTENT_FLAG_CHANGE) {
					auto flag_name = e.event_data;
					if (!flag_name.empty()) {
						LogScheduler(
							"Activating Event [{}] scheduled content flag change, setting flag [{}] to enabled",
							e.description,
							flag_name
						);

						// add new flag entity to stack
						auto flags = content_service->GetContentFlags();
						auto f     = ContentFlagsRepository::NewEntity();
						f.flag_name = flag_name;
						f.enabled   = 1;
						flags.push_back(f);

						content_service->SetContentFlags(flags);
						m_active_events.push_back(e);
					}
				}
			}
		}

		m_last_polled_minute = now->tm_min;
	}
}

// because stored active events could have a reference of time that has been changed since
// the time has been updated, we need to make sure we update internal fields so that
// the scheduler can properly end events if we set a new end date
void ZoneEventScheduler::SyncEventDataWithActiveEvents()
{
	for (auto &a: m_active_events) {
		for (auto &e: m_events) {
			if (e.id == a.id) {
				a.description     = e.description;
				a.event_type      = e.event_type;
				a.event_data      = e.event_data;
				a.minute_start    = e.minute_start;
				a.hour_start      = e.hour_start;
				a.day_start       = e.day_start;
				a.month_start     = e.month_start;
				a.year_start      = e.year_start;
				a.minute_end      = e.minute_end;
				a.hour_end        = e.hour_end;
				a.day_end         = e.day_end;
				a.month_end       = e.month_end;
				a.year_end        = e.year_end;
				a.cron_expression = e.cron_expression;
				a.created_at      = e.created_at;
				a.deleted_at      = e.deleted_at;
			}
		}
	}
}
