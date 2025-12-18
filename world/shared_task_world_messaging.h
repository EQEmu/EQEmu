#pragma once

#include "common/eqemu_logsys.h"
#include "common/repositories/tasks_repository.h"
#include "common/servertalk.h"
#include "common/shared_tasks.h"
#include "common/tasks.h"
#include "common/types.h"

class SharedTaskWorldMessaging
{
public:
	static void HandleZoneMessage(ServerPacket *pack);
};
