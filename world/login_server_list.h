#pragma once

#include "common/eq_packet_structs.h"
#include "common/mutex.h"
#include "common/queue.h"
#include "common/servertalk.h"
#include "common/timer.h"

#include <list>

class LoginServer;

class LoginServerList{
public:
	LoginServerList();
	~LoginServerList();

	void	Add(const char*, uint16, const char*, const char*, bool);
	bool	SendStatus();
	bool	SendPacket(ServerPacket *pack);
	bool	SendAccountUpdate(ServerPacket *pack);
	bool	Connected();
	size_t GetServerCount() const { return m_list.size(); }

	static LoginServerList* Instance()
	{
		static LoginServerList instance;
		return &instance;
	}

protected:
	std::list<std::unique_ptr<LoginServer>> m_list;
};
