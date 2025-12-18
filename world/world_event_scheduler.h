#pragma once

#include "common/server_event_scheduler.h"
#include "world/zonelist.h"

class WorldEventScheduler : public ServerEventScheduler {
public:
	void Process(ZSList *zs_list);

	static WorldEventScheduler* Instance()
	{
		static WorldEventScheduler instance;
		return &instance;
	}
};
