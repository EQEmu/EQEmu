#include "world/world_server_cli.h"

#include "common/events/player_event_logs.h"
#include "common/json/json.h"
#include "world/worlddb.h"

void WorldserverCLI::EtlGetSettings(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Displays server player event logs that are etl enabled";

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	Json::Value etl_settings;
	Json::Value player_events;

	PlayerEventLogs::Instance()->SetDatabase(&database)->Init();
	auto event_settings = PlayerEventLogs::Instance()->GetSettings();
	auto etl_details    = PlayerEventLogs::Instance()->GetEtlSettings();

	for (int i = PlayerEvent::GM_COMMAND; i < PlayerEvent::EventType::MAX; i++) {
		player_events["event_id"]    = event_settings[i].id;
		player_events["enabled"]     = event_settings[i].event_enabled ? true : false;
		player_events["retention"]   = event_settings[i].retention_days;
		player_events["discord_id"]  = event_settings[i].discord_webhook_id;
		player_events["etl_enabled"] = event_settings[i].etl_enabled ? true : false;
		player_events["table_name"]  = "";

		auto it = etl_details.find(static_cast<PlayerEvent::EventType>(event_settings[i].id));

		if (it != std::end(etl_details)) {
			player_events["table_name"]  = it->second.table_name;
			player_events["etl_enabled"] = it->second.enabled;
		}

		etl_settings["etl_settings"].append(player_events);
	}

	std::stringstream payload;
	payload << etl_settings;

	std::cout << payload.str() << std::endl;
}
