
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
