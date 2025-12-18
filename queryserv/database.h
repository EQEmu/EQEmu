#pragma once

#include "common/database.h"
#include "common/eqemu_logsys.h"
#include "common/linked_list.h"
#include "common/servertalk.h"
#include "common/types.h"

#include <map>
#include <string>
#include <vector>

#define AUTHENTICATION_TIMEOUT	60
#define INVALID_ID				0xFFFFFFFF

class QSDatabase : public Database {
public:
	void GeneralQueryReceive(ServerPacket *pack);
};
