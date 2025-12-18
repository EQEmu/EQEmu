#include "world_event_scheduler.h"

#include "common/rulesys.h"
#include "common/server_reload_types.h"
#include "common/servertalk.h"

#include <ctime>

void WorldEventScheduler::Process(ZSList *zs_list)
{
	std::time_t time = std::time(nullptr);
	std::tm     *now = std::localtime(&time);

	// once a minute polling
	if (m_last_polled_minute != now->tm_min) {

		// refresh; world polls and tells zones if they should update if there is a change
		if (CheckIfEventsChanged()) {
			LogSchedulerDetail("Event changes detected, forcing zones to refresh their schedules...");
			auto pack = new ServerPacket(ServerOP_UpdateSchedulerEvents, 0);
			zs_list->SendPacket(pack);
			safe_delete(pack);
		}

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

		for (auto &e: m_events) {
			// discard uninteresting events as its less work to calculate time on events we don't care about
			// different processes are interested in different events
			if (e.event_type != ServerEvents::EVENT_TYPE_BROADCAST &&
				e.event_type != ServerEvents::EVENT_TYPE_RELOAD_WORLD) {
				continue;
			}

			// validate event is ready to activate and run it
			if (ValidateEventReadyToActivate(e)) {
				if (e.event_type == ServerEvents::EVENT_TYPE_BROADCAST) {
					LogScheduler("Sending broadcast [{}]", e.event_data.c_str());
					zs_list->SendEmoteMessage(
						0,
						0,
						AccountStatus::Player,
						Chat::Yellow,
						e.event_data.c_str()
					);
				}

				if (e.event_type == ServerEvents::EVENT_TYPE_RELOAD_WORLD) {
					LogScheduler("Sending reload world event [{}]", e.event_data.c_str());

					zs_list->SendServerReload(ServerReload::Type::WorldRepop, nullptr);
				}
			}
		}

		m_last_polled_minute = now->tm_min;
	}
}
