#include "discord.h"

#include "common/eqemu_logsys.h"
#include "common/events/player_event_logs.h"
#include "common/http/httplib.h"
#include "common/json/json.h"
#include "common/strings.h"

#include "cereal/archives/binary.hpp"
#include "cereal/archives/json.hpp"

constexpr int MAX_RETRIES = 10;

void Discord::SendWebhookMessage(const std::string &message, const std::string &webhook_url)
{
	if (!ValidateWebhookUrl(webhook_url)) {
		return;
	}

	// split
	auto s = Strings::Split(webhook_url, '/');

	// url
	std::string base_url = fmt::format("{}//{}", s[0], s[2]);
	std::string endpoint = Strings::Replace(webhook_url, base_url, "");

	// client
	httplib::Client cli(base_url);
	cli.set_connection_timeout(0, 15000000); // 15 sec
	cli.set_read_timeout(15, 0); // 15 seconds
	cli.set_write_timeout(15, 0); // 15 seconds

	// payload
	Json::Value p;
	p["content"] = message;
	std::stringstream payload;
	payload << p;

	bool retry       = true;
	int  retries     = 0;
	int  retry_timer = 1000;
	while (retry) {
		if (auto res = cli.Post(endpoint, payload.str(), "application/json")) {
			if (res->status != 200 && res->status != 204) {
				LogError("[Discord Client] Code [{}] Error [{}]", res->status, res->body);
			}
			if (res->status == 429) {
				if (!res->body.empty()) {
					std::stringstream ss(res->body);
					Json::Value       response;

					try {
						ss >> response;
					}
					catch (std::exception const &ex) {
						LogDiscord("JSON serialization failure [{}] via [{}]", ex.what(), res->body);
					}

					retry_timer = Strings::ToInt(response["retry_after"].asString()) + 500;
				}

				LogDiscord("Rate limited... retrying message in [{}ms]", retry_timer);
				std::this_thread::sleep_for(std::chrono::milliseconds(retry_timer + 500));
			}
			if (res->status == 204) {
				retry = false;
			}
			if (retries > MAX_RETRIES) {
				LogDiscord("Retries exceeded for message [{}]", message);
				retry = false;
			}

			retries++;
		}
	}
}

void Discord::SendPlayerEventMessage(
	const PlayerEvent::PlayerEventContainer &e,
	const std::string &webhook_url
)
{
	if (!ValidateWebhookUrl(webhook_url)) {
		return;
	}

	auto s = Strings::Split(webhook_url, '/');

	// url
	std::string base_url = fmt::format("{}//{}", s[0], s[2]);
	std::string endpoint = Strings::Replace(webhook_url, base_url, "");

	// client
	httplib::Client cli(base_url);
	cli.set_connection_timeout(0, 15000000); // 15 sec
	cli.set_read_timeout(15, 0); // 15 seconds
	cli.set_write_timeout(15, 0); // 15 seconds

	std::string payload = PlayerEventLogs::GetDiscordPayloadFromEvent(e);
	if (payload.empty()) {
		return;
	}

	bool retry       = true;
	int  retries     = 0;
	int  retry_timer = 1000;
	while (retry) {
		if (auto res = cli.Post(endpoint, payload, "application/json")) {
			if (res->status != 200 && res->status != 204) {
				LogError("Code [{}] Error [{}]", res->status, res->body);
			}
			if (res->status == 429) {
				if (!res->body.empty()) {
					std::stringstream ss(res->body);
					Json::Value       response;

					try {
						ss >> response;
					}
					catch (std::exception const &ex) {
						LogDiscord("JSON serialization failure [{}] via [{}]", ex.what(), res->body);
					}

					retry_timer = Strings::ToInt(response["retry_after"].asString()) + 500;
				}

				LogDiscord("Rate limited... retrying message in [{}ms]", retry_timer);
				std::this_thread::sleep_for(std::chrono::milliseconds(retry_timer + 500));
			}
			if (res->status == 204) {
				retry = false;
			}
			if (retries > MAX_RETRIES) {
				LogDiscord("Retries exceeded for player event message");
				retry = false;
			}

			retries++;
		}
	}
}

std::string Discord::FormatDiscordMessage(uint16 category_id, const std::string &message)
{
	if (category_id == Logs::LogCategory::MySQLQuery) {
		return fmt::format("```sql\n{}\n```", message);
	}

	return message + "\n";
}

bool Discord::ValidateWebhookUrl(const std::string &webhook_url)
{
	// validate
	if (webhook_url.empty()) {
		LogDiscord("[webhook_url] is empty");
		return false;
	}

	// validate
	if (!Strings::Contains(webhook_url, "http://") && !Strings::Contains(webhook_url, "https://")) {
		LogDiscord("[webhook_url] [{}] does not contain a valid http/s prefix.", webhook_url);
		return false;
	}

	return true;
}
