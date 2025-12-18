#pragma once

#include "common/content/world_content_service.h"
#include "common/server_event_scheduler.h"
#include "zone/zone.h"

class ZoneEventScheduler : public ServerEventScheduler {
public:
	void Process(Zone *zone, WorldContentService *content_service);
	void SyncEventDataWithActiveEvents();

	static ZoneEventScheduler* Instance()
	{
		static ZoneEventScheduler instance;
		return &instance;
	}
};
