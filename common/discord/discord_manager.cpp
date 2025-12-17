#include "discord_manager.h"

#include "common/discord/discord.h"
#include "common/events/player_event_logs.h"

void DiscordManager::QueueWebhookMessage(uint32 webhook_id, const std::string &message)
{
	webhook_queue_lock.lock();
	webhook_message_queue[webhook_id].emplace_back(message);
	webhook_queue_lock.unlock();
}

constexpr int MAX_MESSAGE_LENGTH = 1900;

void DiscordManager::ProcessMessageQueue()
{
	if (webhook_message_queue.empty()) {
		return;
	}

	webhook_queue_lock.lock();
	for (auto &q: webhook_message_queue) {
		LogDiscord("Processing [{}] messages in queue for webhook ID [{}]...", q.second.size(), q.first);

		if (q.first >= MAX_DISCORD_WEBHOOK_ID) {
			LogDiscord("Out of bounds webhook ID [{}] max [{}]", q.first, MAX_DISCORD_WEBHOOK_ID);
			continue;
		}

		auto        webhook  = EQEmuLogSys::Instance()->GetDiscordWebhooks()[q.first];
		std::string message;

		for (auto &m: q.second) {
			// next message would become too large
			bool next_message_too_large = ((int) m.length() + (int) message.length()) > MAX_MESSAGE_LENGTH;
			if (next_message_too_large) {
				Discord::SendWebhookMessage(
					message,
					webhook.webhook_url
				);
				message.clear();
			}

			message += m;

			// one single message was too large
			// this should rarely happen but the message will need to be split
			if ((int) message.length() > MAX_MESSAGE_LENGTH) {
				for (unsigned mi = 0; mi < message.length(); mi += MAX_MESSAGE_LENGTH) {
					Discord::SendWebhookMessage(
						message.substr(mi, MAX_MESSAGE_LENGTH),
						webhook.webhook_url
					);
				}
				message.clear();
			}
		}
		// final flush
		if (!message.empty()) {
			Discord::SendWebhookMessage(
				message,
				webhook.webhook_url
			);
		}
	}
	webhook_message_queue.clear();
	webhook_queue_lock.unlock();
}

void DiscordManager::QueuePlayerEventMessage(const PlayerEvent::PlayerEventContainer& e)
{
	auto w = PlayerEventLogs::Instance()->GetDiscordWebhookUrlFromEventType(e.player_event_log.event_type_id);
	if (!w.empty()) {
		Discord::SendPlayerEventMessage(e, w);
	}
}
