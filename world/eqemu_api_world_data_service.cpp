#include "eqemu_api_world_data_service.h"

#include "common/database_schema.h"
#include "common/server_reload_types.h"
#include "common/zone_store.h"
#include "world/cliententry.h"
#include "world/clientlist.h"
#include "world/queryserv.h"
#include "world/ucs.h"
#include "world/wguild_mgr.h"
#include "world/world_config.h"
#include "world/worlddb.h"
#include "world/zonelist.h"
#include "world/zoneserver.h"

#include "fmt/format.h"

extern WorldGuildManager guild_mgr;

void callGetZoneList(Json::Value &response)
{
	for (auto &zone: ZSList::Instance()->getZoneServerList()) {
		Json::Value row;

		if (!zone) {
			continue;
		}

		if (!zone->IsConnected()) {
			continue;
		}

		row["booting_up"]           = zone->IsBootingUp();
		row["client_address"]       = zone->GetCAddress();
		row["client_local_address"] = zone->GetCLocalAddress();
		row["client_port"]          = zone->GetCPort();
		row["compile_version"]      = zone->GetCurrentVersion();
		row["compile_date"]         = zone->GetCompileDate();
		row["compile_time"]         = zone->GetCompileTime();
		row["id"]                   = zone->GetID();
		row["instance_id"]          = zone->GetInstanceID();
		row["ip"]                   = zone->GetIP();
		row["is_static_zone"]       = zone->IsStaticZone();
		row["launch_name"]          = zone->GetLaunchName();
		row["launched_name"]        = zone->GetLaunchedName();
		row["number_players"]       = zone->NumPlayers();
		row["port"]                 = zone->GetPort();
		row["previous_zone_id"]     = zone->GetPrevZoneID();
		row["uuid"]                 = zone->GetUUID();
		row["zone_id"]              = zone->GetZoneID();
		row["zone_long_name"]       = zone->GetZoneLongName();
		row["zone_name"]            = zone->GetZoneName();
		row["zone_os_pid"]          = zone->GetZoneOSProcessID();

		response.append(row);
	}
}

void callGetDatabaseSchema(Json::Value &response)
{
	Json::Value              player_tables_json;
	std::vector<std::string> player_tables = DatabaseSchema::GetPlayerTables();
	for (const auto          &table: player_tables) {
		player_tables_json.append(table);
	}

	Json::Value              content_tables_json;
	std::vector<std::string> content_tables = DatabaseSchema::GetContentTables();
	for (const auto          &table: content_tables) {
		content_tables_json.append(table);
	}

	Json::Value              server_tables_json;
	std::vector<std::string> server_tables = DatabaseSchema::GetServerTables();
	for (const auto          &table: server_tables) {
		server_tables_json.append(table);
	}

	Json::Value              login_tables_json;
	std::vector<std::string> login_tables = DatabaseSchema::GetLoginTables();
	for (const auto          &table: login_tables) {
		login_tables_json.append(table);
	}

	Json::Value              state_tables_json;
	std::vector<std::string> state_tables = DatabaseSchema::GetStateTables();
	for (const auto          &table: state_tables) {
		state_tables_json.append(table);
	}

	Json::Value              version_tables_json;
	std::vector<std::string> version_tables = DatabaseSchema::GetVersionTables();
	for (const auto          &table: version_tables) {
		version_tables_json.append(table);
	}

	Json::Value                        character_table_columns_json;
	std::map<std::string, std::string> character_table = DatabaseSchema::GetCharacterTables();
	for (const auto                    &ctc: character_table) {
		character_table_columns_json[ctc.first] = ctc.second;
	}

	Json::Value schema;

	schema["character_table_columns"] = character_table_columns_json;
	schema["content_tables"]          = content_tables_json;
	schema["login_tables"]            = login_tables_json;
	schema["player_tables"]           = player_tables_json;
	schema["server_tables"]           = server_tables_json;
	schema["state_tables"]            = state_tables_json;
	schema["version_tables"]          = version_tables_json;

	response.append(schema);
}

void callGetClientList(Json::Value &response, const std::vector<std::string> &args)
{
	// if args has "full"
	bool full_list = false;
	if (args.size() > 1) {
		if (args[1] == "full") {
			full_list = true;
		}
	}

	ClientList::Instance()->GetClientList(response, full_list);
}

void getReloadTypes(Json::Value &response)
{
	for (auto &t: ServerReload::GetTypes()) {
		Json::Value v;

		v["command"]     = std::to_string(t);
		v["description"] = ServerReload::GetName(t);
		response.append(v);
	}
}

void getServerCounts(Json::Value &response, const std::vector<std::string> &args)
{
	response["zone_count"]   = ZSList::Instance()->GetServerListCount();
	response["client_count"] = ClientList::Instance()->GetClientCount();
}

void EQEmuApiWorldDataService::reload(Json::Value &r, const std::vector<std::string> &args)
{
	std::vector<std::string> commands{};
	commands.reserve(ServerReload::GetTypes().size());
	for (auto &c: ServerReload::GetTypes()) {
		commands.emplace_back(std::to_string(c));
	}

	std::string command = !args[1].empty() ? args[1] : "";
	if (command.empty()) {
		message(r, fmt::format("Need to provide a type ID to reload. Example(s) [{}]", Strings::Implode("|", commands)));
		return;
	}

	ServerPacket *pack = nullptr;

	bool found_command = false;

	for (auto &t: ServerReload::GetTypes()) {
		if (std::to_string(t) == command || Strings::ToLower(ServerReload::GetName(t)) == command) {
			message(r, fmt::format("Reloading [{}] globally", ServerReload::GetName(t)));
			LogInfo("Queueing reload of type [{}] to zones", ServerReload::GetName(t));
			ZSList::Instance()->QueueServerReload(t);
		}
		found_command = true;
	}

	if (!found_command) {
		message(r, fmt::format("Need to provide a type to reload. Example(s) [{}]", Strings::Implode("|", commands)));
		return;
	}
}

void EQEmuApiWorldDataService::message(Json::Value &r, const std::string &message)
{
	r["message"] = message;
}

void EQEmuApiWorldDataService::get(Json::Value &r, const std::vector<std::string> &args)
{
	const std::string &m = args[0];
	if (m == "get_zone_list") {
		callGetZoneList(r);
	}
	if (m == "get_database_schema") {
		callGetDatabaseSchema(r);
	}
	if (m == "get_client_list") {
		callGetClientList(r, args);
	}
	if (m == "get_reload_types") {
		getReloadTypes(r);
	}
	if (m == "reload") {
		reload(r, args);
	}
	if (m == "get_guild_details") {
		callGetGuildDetails(r, args);
	}
	if (m == "get_server_counts") {
		getServerCounts(r, args);
	}
	if (m == "lock_status") {
		r["locked"] = WorldConfig::get()->Locked;
	}
}

void EQEmuApiWorldDataService::callGetGuildDetails(Json::Value &response, const std::vector<std::string> &args)
{
	std::string command = !args[1].empty() ? args[1] : "";
	if (command.empty()) {
		return;
	}
	Json::Value row;

	auto guild_id = Strings::ToUnsignedInt(command);
	if (!guild_id) {
		row = "useage is: api get_guild_details ### where ### is a valid guild id.";
		return;
	}
	auto guild = guild_mgr.GetGuildByGuildID(guild_id);
	if (!guild) {
		row = fmt::format("Could not find guild id {}", guild_id);
		return;
	}

	row["guild_id"]    = command;
	row["guild_name"]  = guild->name;
	row["leader_id"]   = guild->leader;
	row["min_status"]  = guild->minstatus;
	row["motd"]        = guild->motd;
	row["motd_setter"] = guild->motd_setter;
	row["url"]         = guild->url;
	row["channel"]     = guild->channel;

	for (int i = GUILD_LEADER; i <= GUILD_RECRUIT; i++) {
		row["Ranks"][i] = guild->rank_names[i].c_str();
	}

	for (int i = 1; i <= GUILD_MAX_FUNCTIONS; i++) {
		row["functions"][i]["db_id"]      = guild->functions[i].id;
		row["functions"][i]["perm_id"]    = guild->functions[i].perm_id;
		row["functions"][i]["guild_id"]   = guild->functions[i].guild_id;
		row["functions"][i]["perm_value"] = guild->functions[i].perm_value;
	}

	row["tribute"]["favor"]          = guild->tribute.favor;
	row["tribute"]["id1"]            = guild->tribute.id_1;
	row["tribute"]["id1_tier"]       = guild->tribute.id_1_tier;
	row["tribute"]["id2"]            = guild->tribute.id_2;
	row["tribute"]["id2_tier"]       = guild->tribute.id_2_tier;
	row["tribute"]["time_remaining"] = guild->tribute.time_remaining;
	row["tribute"]["enabled"]        = guild->tribute.enabled;

	ClientList::Instance()->GetGuildClientList(response, guild_id);

	response.append(row);
}
