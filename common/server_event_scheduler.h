#pragma once

#include "common/repositories/server_scheduled_events_repository.h"

#include <ctime>
#include <chrono>

namespace ServerEvents {
	static const std::string EVENT_TYPE_HOT_ZONE_ACTIVE     = "hot_zone_activate";
	static const std::string EVENT_TYPE_BROADCAST           = "broadcast";
	static const std::string EVENT_TYPE_RELOAD_WORLD        = "reload_world";
	static const std::string EVENT_TYPE_RULE_CHANGE         = "rule_change";
	static const std::string EVENT_TYPE_CONTENT_FLAG_CHANGE = "content_flag_change";
} // namespace ServerEvents

class ServerEventScheduler {
public:
	virtual ~ServerEventScheduler();
	ServerEventScheduler();
	ServerEventScheduler *SetDatabase(Database *db);
	void LoadScheduledEvents();
	bool CheckIfEventsChanged();

protected:

	// events directly from the database
	std::vector<ServerScheduledEventsRepository::ServerScheduledEvents> m_events;

	// used to track only when it is convenient to undo an action from an active event
	// typically there should be two separate events to turn something on / off
	// hotzones use this right now simply to keep us from toggling off the hotzone
	// every minute we trigger and then immediately turning it right back on
	std::vector<ServerScheduledEventsRepository::ServerScheduledEvents> m_active_events;

	// simple ticker used to determine when the last polled minute was so that when the minute
	// changes we fire checking the scheduler
	int m_last_polled_minute;

	// validates an event is currently active or not
	bool ValidateEventReadyToActivate(ServerScheduledEventsRepository::ServerScheduledEvents &e);

	// is event active
	bool IsEventActive(ServerScheduledEventsRepository::ServerScheduledEvents &e);

	// remove active event
	bool RemoveActiveEvent(ServerScheduledEventsRepository::ServerScheduledEvents &e);

	// build time object from event
	std::tm BuildStartTimeFromEvent(ServerScheduledEventsRepository::ServerScheduledEvents &e, tm *now);
	std::tm BuildEndTimeFromEvent(ServerScheduledEventsRepository::ServerScheduledEvents &e, tm *now);

	// reference to database
	Database *m_database;
	bool ValidateDatabaseConnection();
};
