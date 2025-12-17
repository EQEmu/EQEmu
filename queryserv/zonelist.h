#pragma once

#include "common/types.h"
#include "queryserv/zoneserver.h"

#include <list>
#include <memory>
#include <string>
#include <vector>

class WorldTCPConnection;

class ZSList {
public:
	std::list<std::unique_ptr<ZoneServer>>& GetZsList() { return zone_server_list; }
	void                                    Add(ZoneServer* zoneserver);
	void                                    Remove(const std::string& uuid);
	void                                    SendPlayerEventLogSettings();

	static ZSList* Instance()
	{
		static ZSList instance;
		return &instance;
	}

private:
	std::list<std::unique_ptr<ZoneServer>> zone_server_list;
};
