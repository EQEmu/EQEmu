/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "common/events/player_events.h"
#include "common/repositories/player_event_logs_repository.h"
#include "common/types.h"

#include <map>
#include <mutex>
#include <vector>

class DiscordManager
{
public:
	void QueueWebhookMessage(uint32 webhook_id, const std::string& message);
	void ProcessMessageQueue();
	void QueuePlayerEventMessage(const PlayerEvent::PlayerEventContainer& e);

	static DiscordManager* Instance()
	{
		static DiscordManager instance;
		return &instance;
	}
private:
	std::mutex webhook_queue_lock{};
	std::map<uint32, std::vector<std::string>> webhook_message_queue{};
};
