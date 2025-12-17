#pragma once

#include "common/events/player_events.h"
#include "common/repositories/player_event_logs_repository.h"
#include "common/types.h"

#include <string>

class Discord
{
public:
	static void SendWebhookMessage(const std::string& message, const std::string& webhook_url);
	static std::string FormatDiscordMessage(uint16 category_id, const std::string& message);
	static void SendPlayerEventMessage(const PlayerEvent::PlayerEventContainer& e, const std::string &webhook_url);
	static bool ValidateWebhookUrl(const std::string &webhook_url);
};
