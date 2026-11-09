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
#include "console.h"

#include "common/md5.h"
#include "common/repositories/character_parcels_repository.h"
#include "common/strings.h"
#include "common/util/uuid.h"
#include "common/zone_store.h"
#include "world/clientlist.h"
#include "world/cliententry.h"
#include "world/eqemu_api_world_data_service.h"
#include "world/login_server_list.h"
#include "world/login_server.h"
#include "world/world_config.h"
#include "world/world_console_connection.h"
#include "world/worlddb.h"
#include "world/zonelist.h"
#include "world/zoneserver.h"

#include "fmt/format.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <unordered_map>

/**
 * @param username
 * @param password
 * @return
 */
struct EQ::Net::ConsoleLoginStatus CheckLogin(const std::string &username, const std::string &password)
{
	struct EQ::Net::ConsoleLoginStatus ret;

	std::string prefix   = "eqemu";
	std::string raw_user = "";

	ParseAccountString(username, raw_user, prefix);

	ret.account_id = database.CheckLogin(raw_user.c_str(), password.c_str(), prefix.c_str());

	if (ret.account_id == 0) {
		return ret;
	}

	const std::string& account_name = database.GetAccountName(ret.account_id);

	ret.account_name = account_name;
	ret.status       = database.GetAccountStatus(ret.account_id);
	return ret;
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleNull(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
}


/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleApi(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	Json::Value root;
	Json::Value response;

	BenchTimer timer;
	timer.reset();

	std::string method = args.empty() ? "" : args[0];

	if (method.empty()) {
		root["execution_time"] = std::to_string(timer.elapsed());
		root["method"]         = method;
		root["data"]           = response;
		root["error"]          = "No method specified";

		std::stringstream payload;
		payload << root;
		connection->SendLine(payload.str());
		return;
	}

	// Safe to call now that args[0] is known to exist
	EQEmuApiWorldDataService::get(response, args);

	root["execution_time"] = std::to_string(timer.elapsed());
	root["method"]         = method;
	root["data"]           = response;

	std::stringstream payload;
	payload << root;
	connection->SendLine(payload.str());
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleWhoami(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	connection->SendLine(fmt::format("You are logged in as '{0}'", connection->UserName()));
	connection->SendLine(fmt::format("You are known as '*{0}'", connection->UserName()));
	connection->SendLine(fmt::format("AccessLevel: '{0}'", connection->Admin()));
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleZoneStatus(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	WorldConsoleTCPConnection console_connection(connection);
	ZSList::Instance()->SendZoneStatus(0, connection->Admin(), &console_connection);
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleWho(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	Who_All_Struct whom;
	memset(&whom, 0, sizeof(whom));
	whom.lvllow   = 0xFFFF;
	whom.lvlhigh  = 0xFFFF;
	whom.wclass   = 0xFFFF;
	whom.wrace    = 0xFFFF;
	whom.gmlookup = 0xFFFF;

	for (auto &arg : args) {
		if (strcasecmp(arg.c_str(), "gm") == 0) {
			whom.gmlookup = 1;
		}
		else if (Strings::IsNumber(arg)) {
			if (whom.lvllow == 0xFFFF) {
				whom.lvllow  = Strings::ToInt(arg);
				whom.lvlhigh = whom.lvllow;
			}
			else if (Strings::ToInt(arg) > int(whom.lvllow)) {
				whom.lvlhigh = Strings::ToInt(arg);
			}
			else {
				whom.lvllow = Strings::ToInt(arg);
			}
		}
		else {
			strn0cpy(whom.whom, arg.c_str(), sizeof(whom.whom));
		}
	}

	WorldConsoleTCPConnection console_connection(connection);
	ClientList::Instance()->ConsoleSendWhoAll(0, connection->Admin(), &whom, &console_connection);
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleUptime(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 1) {
		return;
	}

	if (Strings::IsNumber(args[0]) && Strings::ToInt(args[0]) > 0) {
		auto                pack = new ServerPacket(ServerOP_Uptime, sizeof(ServerUptime_Struct));
		ServerUptime_Struct *sus = (ServerUptime_Struct *) pack->pBuffer;
		snprintf(sus->adminname, sizeof(sus->adminname), "*%s", connection->UserName().c_str());
		sus->zoneserverid = Strings::ToInt(args[0]);
		ZoneServer *zs = ZSList::Instance()->FindByID(sus->zoneserverid);
		if (zs) {
			zs->SendPacket(pack);
		}
		else {
			connection->SendLine("Zoneserver not found.");
		}
		delete pack;
	}
	else {
		WorldConsoleTCPConnection console_connection(connection);
		ZSList::ShowUpTime(&console_connection);
	}
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleMd5(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 1) {
		return;
	}

	uint8 md5[16];
	MD5::Generate((const uchar *) args[0].c_str(), strlen(args[0].c_str()), md5);

	connection->SendLine(
		StringFormat(
			"MD5: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
			md5[0],
			md5[1],
			md5[2],
			md5[3],
			md5[4],
			md5[5],
			md5[6],
			md5[7],
			md5[8],
			md5[9],
			md5[10],
			md5[11],
			md5[12],
			md5[13],
			md5[14],
			md5[15]
		)
	);
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleEmote(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 3) {
		return;
	}

	auto join_args = args;
	join_args.erase(join_args.begin(), join_args.begin() + 2);

	if (strcasecmp(args[0].c_str(), "world") == 0) {
		ZSList::Instance()->SendEmoteMessageRaw(
			0,
			0,
			AccountStatus::Player,
			Strings::ToInt(args[1]),
			Strings::Join(join_args, " ").c_str()
		);
	}
	else {
		ZoneServer *zs = ZSList::Instance()->FindByName(args[0].c_str());
		if (zs != 0) {
			zs->SendEmoteMessageRaw(
				0,
				0,
				AccountStatus::Player,
				Strings::ToInt(args[1]),
				Strings::Join(join_args, " ").c_str()
			);
		}
		else {
			ZSList::Instance()->SendEmoteMessageRaw(
				args[0].c_str(),
				0,
				AccountStatus::Player,
				Strings::ToInt(args[1]),
				Strings::Join(join_args, " ").c_str()
			);
		}
	}
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleAcceptMessages(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 1) {
		connection->SendLine("Usage: acceptmessages [on/off]");
		return;
	}

	if (strcasecmp(args[0].c_str(), "on") == 0) {
		connection->SetAcceptMessages(true);
	}
	else if (strcasecmp(args[0].c_str(), "off") == 0) {
		connection->SetAcceptMessages(false);
	}
	else {
		connection->SendLine("Usage: acceptmessages [on/off]");
	}
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleTell(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 2) {
		return;
	}

	char tmpname[64];
	tmpname[0] = '*';
	strcpy(&tmpname[1], connection->UserName().c_str());
	std::string to = args[0];

	auto join_args = args;
	join_args.erase(join_args.begin(), join_args.begin() + 1);

	ZSList::Instance()->SendChannelMessage(tmpname, to.c_str(), ChatChannel_Tell, 0, Strings::Join(join_args, " ").c_str());
}

namespace {

using SupportChatClock = std::chrono::steady_clock;

struct PendingPlayerSupportTell {
	std::string console_id;
	uint32 account_id;
	uint32 character_id;
	std::unique_ptr<ServerPacket> packet;
	SupportChatClock::time_point expires;
	// Spire's answer, or "timeout". It waits here while the sender is zoning.
	std::string decision;
};

// How long an answer waits for a sender who is zoning.
constexpr auto SupportTellZoningGrace = std::chrono::seconds(60);

std::unordered_map<std::string, PendingPlayerSupportTell> pending_support_tells;

// Zones print channel messages through printf, so a literal % must be doubled.
std::string EscapeChannelText(const std::string &text)
{
	std::string escaped;
	escaped.reserve(text.size());
	for (const auto c : text) {
		escaped.push_back(c);
		if (c == '%') {
			escaped.push_back('%');
		}
	}
	return escaped;
}

void RejectPlayerSupportTell(ClientListEntry &sender, const std::string &decision)
{
	const char *message = "Game Master messaging is unavailable right now. Please try again.";
	if (decision == "no_petition") {
		message = "Your account has no active petition. Use #petition [message] or /guidehelp to file one.";
	} else if (decision == "messages_off") {
		message = "Your petition is in the queue. A Game Master will turn on messages when they are ready to reply.";
	} else if (decision == "invalid") {
		message = "Your message was empty or too long. Please shorten it and try again.";
	} else if (decision == "timeout") {
		message = "Game Master could not confirm your message. Please try again.";
	}
	const auto from = fmt::format("*{}", EQ::Net::PlayerSupportConsoleName);
	ZSList::Instance()->SendChannelMessageRaw(from.c_str(), sender.name(), ChatChannel_Tell, 0, message);
}

// Delivers the answer to the sender. Returns false while the sender is
// zoning, so the answer can wait for them instead of being lost. Statuses
// past InZone (such as offline trading on some servers) count as gone.
bool FinishPlayerSupportTell(const PendingPlayerSupportTell &tell)
{
	auto *sender = ClientList::Instance()->FindCLEByCharacterID(tell.character_id);
	if (!sender || sender->AccountID() != tell.account_id || sender->Online() < CLE_Status::Zoning ||
		sender->Online() > CLE_Status::InZone) {
		return true;
	}
	if (sender->Online() != CLE_Status::InZone || !sender->Server()) {
		return false;
	}
	if (tell.decision == "accepted") {
		const auto *original = reinterpret_cast<const ServerChannelMessage_Struct *>(tell.packet->pBuffer);
		const auto text = EscapeChannelText(original->message);
		ServerPacket echo(ServerOP_ChannelMessage, sizeof(ServerChannelMessage_Struct) + text.size() + 1);
		auto *message = reinterpret_cast<ServerChannelMessage_Struct *>(echo.pBuffer);
		memcpy(message, original, sizeof(ServerChannelMessage_Struct));
		memcpy(message->message, text.c_str(), text.size() + 1);
		strn0cpy(message->deliverto, sender->name(), sizeof(message->deliverto));
		message->chan_num = ChatChannel_TellEcho;
		message->noreply = true;
		sender->Server()->SendPacket(&echo);
	} else {
		RejectPlayerSupportTell(*sender, tell.decision);
	}
	return true;
}

}

void QueuePlayerSupportTell(EQ::Net::ConsoleServerConnection *connection, ClientListEntry &sender, ServerPacket &packet)
{
	if (!connection || !connection->IsSupportChat()) {
		// Petitions are one per account, and any of its characters may reply.
		const auto result = database.QueryDatabase(fmt::format(
			"SELECT p.dib FROM petitions p "
			"JOIN account acct ON acct.name = p.accountname "
			"WHERE acct.id = {} LIMIT 1",
			sender.AccountID()
		));
		RejectPlayerSupportTell(sender, result.Success() && result.RowCount() == 0 ? "no_petition" : "unavailable");
		return;
	}

	const auto *message = reinterpret_cast<const ServerChannelMessage_Struct *>(packet.pBuffer);
	const auto available = packet.size - offsetof(ServerChannelMessage_Struct, message);
	const auto *end = static_cast<const char *>(std::memchr(message->message, '\0', available));
	if (!end || end == message->message || end - message->message > 4096) {
		RejectPlayerSupportTell(sender, "invalid");
		return;
	}
	// A few unanswered messages per character, so one player cannot fill the queue.
	const auto character_pending = std::count_if(
		pending_support_tells.begin(), pending_support_tells.end(),
		[&sender](const auto &pending) { return pending.second.character_id == sender.CharID(); }
	);
	if (pending_support_tells.size() >= 128 || character_pending >= 3) {
		RejectPlayerSupportTell(sender, "unavailable");
		return;
	}

	const auto request_id = EQ::Util::UUID::Generate().ToString();
	pending_support_tells.emplace(request_id, PendingPlayerSupportTell{
		connection->GetUUID(), sender.AccountID(), sender.CharID(),
		std::unique_ptr<ServerPacket>(packet.Copy()), SupportChatClock::now() + std::chrono::seconds(8)
	});
	Json::Value event;
	event["kind"] = "tell";
	event["request_id"] = request_id;
	event["account_id"] = sender.AccountID();
	event["character_id"] = sender.CharID();
	event["character"] = sender.name();
	event["text"] = std::string(message->message, end);
	Json::StreamWriterBuilder writer;
	writer["indentation"] = "";
	connection->SendLine("SPIRE_SUPPORT " + Json::writeString(writer, event));
}

void ProcessPlayerSupportTells()
{
	if (pending_support_tells.empty()) {
		return;
	}
	const auto now = SupportChatClock::now();
	static auto next_check = now;
	if (now < next_check) {
		return;
	}
	next_check = now + std::chrono::seconds(1);
	for (auto it = pending_support_tells.begin(); it != pending_support_tells.end();) {
		auto &tell = it->second;
		if (tell.decision.empty() && tell.expires <= now) {
			tell.decision = "timeout";
		}
		if (!tell.decision.empty() && (FinishPlayerSupportTell(tell) || tell.expires + SupportTellZoningGrace <= now)) {
			it = pending_support_tells.erase(it);
		} else {
			++it;
		}
	}
}

static void ConsoleSupportChat(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &,
	const std::vector<std::string> &args
)
{
	Json::Value response;
	if (args.size() == 1 && args[0] == "listen") {
		response["kind"] = connection->EnableSupportChat() ? "ready" : "busy";
		response["protocol"] = 2;
	} else if (!connection->IsSupportChat()) {
		return;
	} else if (args.size() == 3 && args[0] == "decision") {
		auto it = pending_support_tells.find(args[1]);
		// The first answer stands; a late one cannot replace a timeout.
		if (it != pending_support_tells.end() && it->second.console_id == connection->GetUUID() && it->second.decision.empty()) {
			it->second.decision = args[2];
			if (FinishPlayerSupportTell(it->second)) {
				pending_support_tells.erase(it);
			}
		}
		return;
	} else if (args.size() == 1 && args[0] == "ping") {
		response["kind"] = "pong";
	} else if (args.size() == 5 && args[0] == "send" && args[1].size() <= 64) {
		response["request_id"] = args[1];
		response["kind"] = "unreachable";
		const auto account_id = Strings::ToUnsignedInt(args[2]);
		const auto character_id = Strings::ToUnsignedInt(args[3]);
		const auto &hex = args[4];
		std::string message;
		const std::string digits = "0123456789abcdef";
		if (!hex.empty() && hex.size() <= 400 && hex.size() % 2 == 0) {
			for (size_t i = 0; i < hex.size(); i += 2) {
				const auto high = digits.find(hex[i]);
				const auto low = digits.find(hex[i + 1]);
				if (high == std::string::npos || low == std::string::npos || high * 16 + low < 32 || high * 16 + low > 126) {
					message.clear();
					break;
				}
				message.push_back(static_cast<char>(high * 16 + low));
			}
		}
		auto *recipient = character_id ? ClientList::Instance()->FindCLEByCharacterID(character_id) : nullptr;
		if (!message.empty() && recipient && account_id && recipient->AccountID() == account_id &&
			recipient->Online() == CLE_Status::InZone && recipient->Server() && !recipient->TellsOff()) {
			const auto from = fmt::format("*{}", EQ::Net::PlayerSupportConsoleName);
			ZSList::Instance()->SendChannelMessageRaw(from.c_str(), recipient->name(), ChatChannel_Tell, 0, EscapeChannelText(message).c_str());
			response["kind"] = "sent";
		}
	} else {
		return;
	}
	Json::StreamWriterBuilder writer;
	writer["indentation"] = "";
	connection->SendLine("SPIRE_SUPPORT " + Json::writeString(writer, response));
}

static uint16 SupportZonePort(ZoneServer *zone_server)
{
	if (!zone_server || (zone_server->GetIP() != "127.0.0.1" && zone_server->GetIP() != "::ffff:127.0.0.1")) {
		return 0;
	}
	return zone_server->GetCPort();
}

static void SendSupportMoveResult(EQ::Net::ConsoleServerConnection *connection, const char *status, uint16 port = 0)
{
	connection->SendLine(fmt::format("{{\"status\":\"{}\",\"port\":{}}}", status, port));
}

static void ConsoleSupportLocation(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &,
	const std::vector<std::string> &args
)
{
	if (!connection->IsLocalAdmin() || args.size() != 2) {
		SendSupportMoveResult(connection, "stale");
		return;
	}
	const auto character_id = Strings::ToUnsignedInt(args[0]);
	const auto account_id = Strings::ToUnsignedInt(args[1]);
	auto *client = character_id ? ClientList::Instance()->FindCLEByCharacterID(character_id) : nullptr;
	if (!account_id || (client && client->AccountID() != account_id)) {
		SendSupportMoveResult(connection, "stale");
	} else if (!client || client->Online() < CLE_Status::Zoning) {
		SendSupportMoveResult(connection, "offline");
	} else if (client->Online() != CLE_Status::InZone || !client->Server() || client->Server()->IsBootingUp()) {
		SendSupportMoveResult(connection, "busy");
	} else if (const auto port = SupportZonePort(client->Server())) {
		SendSupportMoveResult(connection, "zone", port);
	} else {
		SendSupportMoveResult(connection, "unavailable");
	}
}

static void ConsoleSupportMove(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &,
	const std::vector<std::string> &args
)
{
	auto reply = [connection](const char *status, uint16 port = 0) {
		SendSupportMoveResult(connection, status, port);
	};
	const bool player_position = args.size() == 11;
	if (!connection->IsLocalAdmin() || (args.size() != 6 && !player_position) ||
		!std::all_of(args.begin(), args.begin() + (player_position ? 7 : 6), [](const auto &arg) {
			return !arg.empty() && arg.find_first_not_of("0123456789") == std::string::npos;
		})) {
		reply("stale");
		return;
	}
	const auto character_id = Strings::ToUnsignedInt(args[0]);
	const auto account_id = Strings::ToUnsignedInt(args[1]);
	const auto corpse_id = Strings::ToUnsignedInt(args[2]);
	const auto source_zone = Strings::ToUnsignedInt(args[3]);
	const auto source_instance = Strings::ToUnsignedInt(args[4]);
	const auto destination = Strings::ToUnsignedInt(args[5]);
	const auto *target = ZoneStore::Instance()->GetZone(destination);
	if (!character_id || !account_id || !source_zone || source_zone > UINT16_MAX || source_instance > UINT16_MAX ||
		(corpse_id == 0 && args[2] != "0") || (source_instance == 0 && args[4] != "0") ||
		!target || target->version != 0) {
		reply("stale");
		return;
	}
	uint32 destination_instance = 0;
	float x = target->safe_x, y = target->safe_y, z = target->safe_z, heading = target->safe_heading;
	if (player_position) {
		destination_instance = Strings::ToUnsignedInt(args[6]);
		auto read_float = [](const std::string &value, float &coordinate) {
			try {
				size_t parsed = 0;
				coordinate = std::stof(value, &parsed);
				return parsed == value.size() && std::isfinite(coordinate);
			} catch (const std::exception &) {
				return false;
			}
		};
		if (destination_instance > UINT16_MAX || (destination_instance == 0 && args[6] != "0") ||
			!read_float(args[7], x) || !read_float(args[8], y) || !read_float(args[9], z) || !read_float(args[10], heading)) {
			reply("stale");
			return;
		}
		if (corpse_id) {
			// A corpse moved to its owner follows the owner's live position.
			auto *owner = ClientList::Instance()->FindCLEByCharacterID(character_id);
			if (!owner || owner->AccountID() != account_id || owner->Online() != CLE_Status::InZone ||
				owner->zone() != destination || owner->instance() != destination_instance || !SupportZonePort(owner->Server())) {
				reply("busy");
				return;
			}
		} else if (destination_instance != 0) {
			// Character moves use GM coordinates in the base zone only.
			reply("stale");
			return;
		}
	}

	ZoneServer *source = nullptr;
	if (corpse_id) {
		source = source_instance ? ZSList::Instance()->FindByInstanceID(source_instance)
		                         : ZSList::Instance()->FindByZoneID(source_zone);
	} else {
		auto *client = ClientList::Instance()->FindCLEByCharacterID(character_id);
		if (client && client->Online() >= CLE_Status::Zoning) {
			if (client->AccountID() != account_id || client->zone() != source_zone ||
				client->instance() != source_instance) {
				reply("stale");
				return;
			}
			if (client->Online() != CLE_Status::InZone || !client->Server()) {
				reply("busy");
				return;
			}
			source = client->Server();
		}
	}
	if (source) {
		if (source->IsBootingUp() || source->GetZoneID() != source_zone ||
			source->GetInstanceID() != source_instance) {
			reply("busy");
		} else if (const auto port = SupportZonePort(source)) {
			reply("zone", port);
		} else {
			reply("unavailable");
		}
		return;
	}

	const auto position = fmt::format(
		"zone_id = {}, x = {}, y = {}, z = {}, heading = {}",
		destination, x, y, z, heading
	);
	// Upstream EQEmu's TransactionBegin() returns void, so check the query itself.
	if (!corpse_id && !database.QueryDatabase("START TRANSACTION").Success()) {
		reply("unavailable");
		return;
	}
	auto result = corpse_id ? database.QueryDatabase(fmt::format(
		"UPDATE character_corpses SET {}, instance_id = {} WHERE id = {} AND charid = {} "
		"AND zone_id = {} AND instance_id = {} AND is_buried = 0 "
		"AND EXISTS (SELECT 1 FROM character_data cd WHERE cd.id = {} AND cd.account_id = {})",
		position, destination_instance, corpse_id, character_id, source_zone, source_instance, character_id, account_id
	)) : database.QueryDatabase(fmt::format(
		"UPDATE character_data SET {}, zone_instance = 0 WHERE id = {} AND account_id = {} "
		"AND zone_id = {} AND zone_instance = {} AND deleted_at IS NULL",
		position, character_id, account_id, source_zone, source_instance
	));
	if (!result.Success() || result.RowsAffected() != 1) {
		if (!corpse_id) {
			database.TransactionRollback();
		}
		reply("stale");
		return;
	}
	if (!corpse_id) {
		const auto queued = database.QueryDatabase(fmt::format(
			"INSERT INTO data_buckets (`key`, `value`, expires, character_id) "
			"VALUES ('spire:character-move-notice', '1', 0, {}) "
			"ON DUPLICATE KEY UPDATE `value` = VALUES(`value`), expires = VALUES(expires)",
			character_id
		));
		if (!queued.Success()) {
			database.TransactionRollback();
			reply("unavailable");
			return;
		}
		if (!database.TransactionCommit().Success()) {
			database.TransactionRollback();
			reply("unconfirmed");
			return;
		}
	} else if (!player_position) {
		auto *zone = ZSList::Instance()->FindByZoneID(destination);
		if (zone && !zone->IsBootingUp()) {
			ServerPacket packet(ServerOP_SpawnPlayerCorpse, sizeof(SpawnPlayerCorpse_Struct));
			auto *spawn = reinterpret_cast<SpawnPlayerCorpse_Struct *>(packet.pBuffer);
			spawn->player_corpse_id = corpse_id;
			spawn->zone_id = destination;
			zone->SendPacket(&packet);
		}
	}
	reply("saved");
}

static void ConsoleParcelNotify(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &,
	const std::vector<std::string> &args
)
{
	if (!connection->IsLocalAdmin() || args.size() < 2 || args.size() > 21) {
		connection->SendLine("invalid");
		return;
	}
	const auto character_id = Strings::ToUnsignedInt(args[0]);
	auto *recipient = character_id ? ClientList::Instance()->FindCLEByCharacterID(character_id) : nullptr;
	if (!recipient || recipient->Online() != CLE_Status::InZone || !recipient->Server()) {
		connection->SendLine("offline");
		return;
	}

	std::vector<std::string> parcel_ids;
	for (size_t i = 1; i < args.size(); ++i) {
		const auto id = Strings::ToUnsignedInt(args[i]);
		if (!id) {
			connection->SendLine("invalid");
			return;
		}
		parcel_ids.push_back(std::to_string(id));
	}
	const auto parcels = CharacterParcelsRepository::GetWhere(
		database, fmt::format("char_id = {} AND id IN ({})", character_id, Strings::Join(parcel_ids, ","))
	);
	for (const auto &parcel : parcels) {
		ServerPacket packet(ServerOP_ParcelDelivery, sizeof(Parcel_Struct));
		auto *delivery = reinterpret_cast<Parcel_Struct *>(packet.pBuffer);
		*delivery = {};
		delivery->item_slot = parcel.slot_id;
		strn0cpy(delivery->send_to, recipient->name(), sizeof(delivery->send_to));
		recipient->Server()->SendPacket(&packet);
	}
	connection->SendLine(parcels.empty() ? "gone" : "ok");
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleBroadcast(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 1) {
		return;
	}

	char tmpname[64];
	tmpname[0] = '*';
	strcpy(&tmpname[1], connection->UserName().c_str());
	ZSList::Instance()->SendChannelMessage(tmpname, 0, ChatChannel_Broadcast, 0, Strings::Join(args, " ").c_str());
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleGMSay(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 1) {
		return;
	}

	char tmpname[64];
	tmpname[0] = '*';
	strcpy(&tmpname[1], connection->UserName().c_str());
	ZSList::Instance()->SendChannelMessage(tmpname, 0, ChatChannel_GMSAY, 0, Strings::Join(args, " ").c_str());
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleGuildSay(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 1) {
		return;
	}

	auto from = args[0];
	auto guild_id = Strings::IsNumber(args[1]) ? Strings::ToUnsignedInt(args[1]) : 0;
	if (!guild_id) {
		return;
	}

	auto join_args = args;
	join_args.erase(join_args.begin(), join_args.begin() + 2);

	auto message = fmt::format(
		"{} tells the guild, '{}'",
		from,
		Strings::Join(join_args, " ")
	);

	ZSList::Instance()->SendEmoteMessage(0, guild_id, AccountStatus::Player, Chat::Guild, message.c_str());
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleOOC(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 1) {
		return;
	}

	char tmpname[64];
	tmpname[0] = '*';
	strcpy(&tmpname[1], connection->UserName().c_str());
	ZSList::Instance()->SendChannelMessage(tmpname, 0, ChatChannel_OOC, 0, Strings::Join(args, " ").c_str());
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleAuction(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 1) {
		return;
	}

	char tmpname[64];
	tmpname[0] = '*';
	strcpy(&tmpname[1], connection->UserName().c_str());
	ZSList::Instance()->SendChannelMessage(tmpname, 0, ChatChannel_Auction, 0, Strings::Join(args, " ").c_str());
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleKick(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 1) {
		return;
	}

	char tmpname[64];
	tmpname[0] = '*';
	strcpy(&tmpname[1], connection->UserName().c_str());
	auto pack = new ServerPacket;
	pack->opcode  = ServerOP_KickPlayer;
	pack->size    = sizeof(ServerKickPlayer_Struct);
	pack->pBuffer = new uchar[pack->size];
	ServerKickPlayer_Struct *skp = (ServerKickPlayer_Struct *) pack->pBuffer;
	strcpy(skp->adminname, tmpname);
	strcpy(skp->name, args[0].c_str());
	skp->adminrank = connection->Admin();
	ZSList::Instance()->SendPacket(pack);
	delete pack;
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleLock(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	WorldConfig::LockWorld();
	if (LoginServerList::Instance()->Connected()) {
		LoginServerList::Instance()->SendStatus();
		connection->SendLine("World locked.");
	}
	else {
		connection->SendLine("World locked, but login server not connected.");
	}
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleUnlock(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	WorldConfig::UnlockWorld();
	if (LoginServerList::Instance()->Connected()) {
		LoginServerList::Instance()->SendStatus();
		connection->SendLine("World unlocked.");
	}
	else {
		connection->SendLine("World unlocked, but login server not connected.");
	}
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleZoneShutdown(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 1) {
		connection->SendLine("Usage: zoneshutdown zoneshortname");
		return;
	}

	if (args[0].length() == 0) {
		connection->SendLine("Usage: zoneshutdown zoneshortname");
	}
	else {
		char tmpname[64];
		tmpname[0] = '*';
		strcpy(&tmpname[1], connection->UserName().c_str());

		auto pack = new ServerPacket;
		pack->size    = sizeof(ServerZoneStateChange_Struct);
		pack->pBuffer = new uchar[pack->size];
		memset(pack->pBuffer, 0, sizeof(ServerZoneStateChange_Struct));
		auto *s = (ServerZoneStateChange_Struct *) pack->pBuffer;
		pack->opcode = ServerOP_ZoneShutdown;
		strcpy(s->admin_name, tmpname);
		if (Strings::IsNumber(args[0])) {
			s->zone_server_id = Strings::ToInt(args[0]);
		}
		else {
			s->zone_id = ZoneID(args[0].c_str());
		}

		ZoneServer *zs = 0;
		if (s->zone_server_id != 0) {
			zs = ZSList::Instance()->FindByID(s->zone_server_id);
		}
		else if (s->zone_id != 0) {
			zs = ZSList::Instance()->FindByName(ZoneName(s->zone_id));
		}
		else {
			connection->SendLine("Error: ZoneShutdown: neither ID nor name specified");
		}

		if (zs == 0) {
			connection->SendLine("Error: ZoneShutdown: zoneserver not found");
		}
		else {
			zs->SendPacket(pack);
		}

		delete pack;
	}
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleZoneBootup(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 2) {
		return;
	}

	if (args[1].length() == 0 || !Strings::IsNumber(args[0])) {
		connection->SendLine("Usage: zonebootup ZoneServerID# zoneshortname");
	}
	else {
		char tmpname[64];
		tmpname[0] = '*';
		strcpy(&tmpname[1], connection->UserName().c_str());

		if (args.size() > 2) {
			ZSList::Instance()->SOPZoneBootup(
				tmpname,
				Strings::ToInt(args[0]),
				args[1].c_str(),
				(bool) (strcasecmp(args[1].c_str(), "static") == 0));
		}
		else {
			ZSList::Instance()->SOPZoneBootup(tmpname, Strings::ToInt(args[0]), args[1].c_str(), false);
		}
	}
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleZoneLock(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 1) {
		return;
	}

	if (strcasecmp(args[0].c_str(), "list") == 0) {
		WorldConsoleTCPConnection console_connection(connection);
		ZSList::Instance()->ListLockedZones(0, &console_connection);
	}
	else if (strcasecmp(args[0].c_str(), "lock") == 0 && connection->Admin() >= 101) {
		if (args.size() < 2) {
			return;
		}

		uint16 tmp = ZoneID(args[1].c_str());
		if (tmp) {
			if (ZSList::Instance()->SetLockedZone(tmp, true)) {
				ZSList::Instance()->SendEmoteMessage(
					0,
					0,
					AccountStatus::QuestTroupe,
					Chat::Yellow,
					fmt::format(
						"Zone locked: {}",
						ZoneName(tmp)
					).c_str()
				);
			}
			else {
				connection->SendLine("Failed to change lock");
			}
		}
		else {
			connection->SendLine("Usage: #zonelock lock [zonename]");
		}
	}
	else if (strcasecmp(args[0].c_str(), "unlock") == 0 && connection->Admin() >= 101) {
		if (args.size() < 2) {
			return;
		}

		uint16 tmp = ZoneID(args[1].c_str());
		if (tmp) {
			if (ZSList::Instance()->SetLockedZone(tmp, false)) {
				ZSList::Instance()->SendEmoteMessage(
					0,
					0,
					AccountStatus::QuestTroupe,
					Chat::Yellow,
					fmt::format(
						"Zone unlocked: {}",
						ZoneName(tmp)
					).c_str()
				);
			}
			else {
				connection->SendLine("Failed to change lock");
			}
		}
		else {
			connection->SendLine("Usage: #zonelock unlock [zonename]");
		}
	}
	else {
		connection->SendLine("#zonelock sub-commands");
		connection->SendLine("  list");
		if (connection->Admin() >= 101) {
			connection->SendLine("  lock [zonename]");
			connection->SendLine("  unlock [zonename]");
		}
	}
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleFlag(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 2) {
		return;
	}

	if (args[1].length() == 0 || !Strings::IsNumber(args[0])) {
		connection->SendLine("Usage: flag [status] [accountname]");
	}
	else {
		if (Strings::ToInt(args[0]) > connection->Admin()) {
			connection->SendLine("You cannot set people's status to higher than your own");
		}
		else if (!database.SetAccountStatus(args[1].c_str(), Strings::ToInt(args[0]))) {
			connection->SendLine("Unable to flag account!");
		}
		else {
			connection->SendLine("Account Flaged");
		}
	}
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleSetPass(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() != 2) {
		connection->SendLine("Format: setpass accountname password");
	}
	else {
		std::string prefix = "eqemu";
		std::string raw_user = "";

		ParseAccountString(args[0], raw_user, prefix);

		auto account_id = database.GetAccountIDByName(raw_user, prefix);

		if (!account_id) {
			connection->SendLine("Error: Account not found");
		}
	}
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleVersion(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	connection->SendLine(StringFormat("Current version information."));
	connection->SendLine(StringFormat("  %s", CURRENT_VERSION));
	connection->SendLine(StringFormat("  Compiled on: %s at %s", COMPILE_DATE, COMPILE_TIME));
	connection->SendLine(StringFormat("  Last modified on: %s", LAST_MODIFIED));
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleWorldShutdown(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() == 2) {
		int32 time, interval;
		if (Strings::IsNumber(args[0]) && Strings::IsNumber(args[1]) && ((time = Strings::ToInt(args[0])) > 0) &&
			((interval = Strings::ToInt(args[1])) > 0)) {
			ZSList::Instance()->WorldShutDown(time, interval);
		}
		else {
			connection->SendLine("Usage: worldshutdown [now] [disable] ([time] [interval])");
		}
	}
	else if (args.size() == 1) {
		if (strcasecmp(args[0].c_str(), "now") == 0) {
			ZSList::Instance()->WorldShutDown(0, 0);
		}
		else if (strcasecmp(args[0].c_str(), "disable") == 0) {
			connection->SendLine("[SYSTEM] World shutdown has been aborted.");
			ZSList::Instance()->SendEmoteMessage(
				0,
				0,
				AccountStatus::Player,
				Chat::Yellow,
				"[SYSTEM] World shutdown has been aborted."
			);
			ZSList::Instance()->shutdowntimer->Disable();
			ZSList::Instance()->reminder->Disable();
		}
		else {
			connection->SendLine("Usage: worldshutdown [now] [disable] ([time] [interval])");
		}
	}
	else {
		connection->SendLine("Usage: worldshutdown [now] [disable] ([time] [interval])");
	}
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleIpLookup(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (!args.empty()) {
		WorldConsoleTCPConnection console_connection(connection);
		ClientList::Instance()->SendCLEList(connection->Admin(), nullptr, &console_connection, args[0].c_str());
	}
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleSignalCharByName(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 2) {
		return;
	}

	connection->SendLine(StringFormat("Signal Sent to %s with ID %i", (char *) args[0].c_str(), Strings::ToInt(args[1])));
	uint32 message_len = strlen((char *) args[0].c_str()) + 1;
	auto pack = new ServerPacket(ServerOP_CZSignal, sizeof(CZSignal_Struct) + message_len);
	CZSignal_Struct* CZS = (CZSignal_Struct*) pack->pBuffer;
	uint8 update_type = CZUpdateType_ClientName;
	int update_identifier = 0;
	CZS->update_type = update_type;
	CZS->update_identifier = update_identifier;
	CZS->signal_id = Strings::ToInt(args[1]);
	strn0cpy(CZS->client_name, (char *) args[0].c_str(), 64);
	ZSList::Instance()->SendPacket(pack);
	safe_delete(pack);
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleReloadWorld(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	connection->SendLine("Reloading World...");
	ZSList::Instance()->SendServerReload(ServerReload::Type::WorldRepop, nullptr);
}

auto debounce_reload = std::chrono::system_clock::now();

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleReloadZoneQuests(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.empty()) {
		connection->SendLine("[zone_short_name] required as argument");
		return;
	}

	// if now is within 1 second, return
	if (std::chrono::system_clock::now() - debounce_reload < std::chrono::seconds(1)) {
		debounce_reload = std::chrono::system_clock::now();
		return;
	}

	debounce_reload = std::chrono::system_clock::now();

	std::string zone_short_name = args[0];

	connection->SendLine(fmt::format("Reloading Zone [{}]...", zone_short_name));

	auto pack               = new ServerPacket(ServerOP_HotReloadQuests, sizeof(HotReloadQuestsStruct));
	auto *hot_reload_quests = (HotReloadQuestsStruct *) pack->pBuffer;
	strn0cpy(hot_reload_quests->zone_short_name, (char *) zone_short_name.c_str(), 200);

	ZSList::Instance()->SendPacket(pack);
	safe_delete(pack);
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleQuit(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	connection->SendLine("Exiting...");
	connection->Close();
}

void ConsoleCrossZoneCastSpell(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
) {
	if (args.size() < 3) {
		connection->SendLine("czcast character [character_id] [spell_id]");
		connection->SendLine("czcast expedition [expedition_id] [spell_id]");
		connection->SendLine("czcast group [group_id] [spell_id]");
		connection->SendLine("czcast guild [guild_id] [spell_id]");
		connection->SendLine("czcast name [character_name] [spell_id]");
		connection->SendLine("czcast raid [raid_id] [spell_id]");
		return;
	}

	const auto& type = Strings::ToLower(args[0]);

	const auto is_character  = type == "character";
	const auto is_expedition = type == "expedition";
	const auto is_group      = type == "group";
	const auto is_guild      = type == "guild";
	const auto is_name       = type == "name";
	const auto is_raid       = type == "raid";

	if (
		!is_character &&
		!is_expedition &&
		!is_group &&
		!is_guild &&
		!is_name &
		!is_raid
	) {
		connection->SendLine("czcast character [character_id] [spell_id]");
		connection->SendLine("czcast expedition [expedition_id] [spell_id]");
		connection->SendLine("czcast group [group_id] [spell_id]");
		connection->SendLine("czcast guild [guild_id] [spell_id]");
		connection->SendLine("czcast name [character_name] [spell_id]");
		connection->SendLine("czcast raid [raid_id] [spell_id]");
		return;
	}

	std::string name;
	int         update_identifier = 0;

	if (!is_name) {
		if (Strings::IsNumber(args[1])) {
			update_identifier = Strings::ToInt(args[1]);
		}

		if (!update_identifier) {
			connection->SendLine(fmt::format("Identifier is invalid for '{}'.", type));
			return;
		}
	} else if (is_name) {
		if (!Strings::IsNumber(args[1])) {
			name = Strings::UcFirst(Strings::ToLower(args[1]));
		}

		if (name.empty()) {
			connection->SendLine("Empty name is invalid.");
			return;
		}
	}

	const auto spell_id = Strings::IsNumber(args[2]) ? Strings::ToUnsignedInt(args[2]) : 0;

	if (!spell_id) {
		connection->SendLine("Spell ID is invalid.");
		return;
	}

	uint8 update_type;

	if (is_character) {
		update_type = CZUpdateType_Character;
	} else if (is_expedition) {
		update_type = CZUpdateType_Expedition;
	} else if (is_group) {
		update_type = CZUpdateType_Group;
	} else if (is_guild) {
		update_type = CZUpdateType_Guild;
	} else if (is_name) {
		update_type = CZUpdateType_ClientName;
	} else if (is_raid) {
		update_type = CZUpdateType_Raid;
	}

	auto pack = new ServerPacket(ServerOP_CZSpell, sizeof(CZSpell_Struct));
	auto* CZS = (CZSpell_Struct*) pack->pBuffer;

	CZS->update_type       = update_type;
	CZS->update_subtype    = CZSpellUpdateSubtype_Cast;
	CZS->update_identifier = update_identifier;
	CZS->spell_id          = spell_id;

	strn0cpy(CZS->client_name, name.c_str(), sizeof(CZS->client_name));

	ZSList::Instance()->SendPacket(pack);
	safe_delete(pack);

	connection->SendLine(
		fmt::format(
			"Casting spell ID {} across zones by {} with an identifier of {}.",
			spell_id,
			type,
			!is_name ? std::to_string(update_identifier) : name
		)
	);
}

void ConsoleWorldWideCastSpell(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
) {
	if (args.size() < 1) {
		connection->SendLine("wwcast [spell_id]");
		connection->SendLine("wwcast [spell_id] [min_status]");
		connection->SendLine("wwcast [spell_id] [min_status] [max_status]");
		return;
	}

	const auto spell_id = Strings::IsNumber(args[0]) ? Strings::ToUnsignedInt(args[0]) : 0;

	if (!spell_id) {
		connection->SendLine("Spell ID 0 is invalid.");
		return;
	}

	uint8 min_status = AccountStatus::Player;
	uint8 max_status = AccountStatus::Player;

	if (args.size() >= 2 && Strings::IsNumber(args[1])) {
		min_status = static_cast<uint8>(Strings::ToUnsignedInt(args[1]));
	}

	if (args.size() >= 3 && Strings::IsNumber(args[2])) {
		max_status = static_cast<uint8>(Strings::ToUnsignedInt(args[2]));
	}

	auto pack = new ServerPacket(ServerOP_WWSpell, sizeof(WWSpell_Struct));
	auto* WWS = (WWSpell_Struct*) pack->pBuffer;

	WWS->update_type = WWSpellUpdateType_Cast;
	WWS->spell_id    = spell_id;
	WWS->min_status  = min_status;
	WWS->max_status  = max_status;

	ZSList::Instance()->SendPacket(pack);
	safe_delete(pack);

	connection->SendLine(
		fmt::format(
			"Casting spell ID {} world wide for players with a status between {} and {}.",
			spell_id,
			min_status,
			max_status
		)
	);
}

void ConsoleCrossZoneMove(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
) {
	if (args.size() < 3) {
		connection->SendLine("czmove character [character_id] [instance_id]");
		connection->SendLine("czmove character [character_id] [zone_short_name]");
		connection->SendLine("czmove expedition [expedition_id] [instance_id]");
		connection->SendLine("czmove expedition [expedition_id] [zone_short_name]");
		connection->SendLine("czmove group [group_id] [instance_id]");
		connection->SendLine("czmove group [group_id] [zone_short_name]");
		connection->SendLine("czmove guild [guild_id] [instance_id]");
		connection->SendLine("czmove guild [guild_id] [zone_short_name]");
		connection->SendLine("czmove name [character_name] [instance_id]");
		connection->SendLine("czmove name [character_name] [zone_short_name]");
		connection->SendLine("czmove raid [raid_id] [instance_id]");
		connection->SendLine("czmove raid [raid_id] [zone_short_name]");
		return;
	}

	const auto& type = Strings::ToLower(args[0]);

	const auto is_character  = type == "character";
	const auto is_expedition = type == "expedition";
	const auto is_group      = type == "group";
	const auto is_guild      = type == "guild";
	const auto is_name       = type == "name";
	const auto is_raid       = type == "raid";

	if (
		!is_character &&
		!is_expedition &&
		!is_group &&
		!is_guild &&
		!is_name &
		!is_raid
	) {
		connection->SendLine("czmove character [character_id] [instance_id]");
		connection->SendLine("czmove character [character_id] [zone_short_name]");
		connection->SendLine("czmove expedition [expedition_id] [instance_id]");
		connection->SendLine("czmove expedition [expedition_id] [zone_short_name]");
		connection->SendLine("czmove group [group_id] [instance_id]");
		connection->SendLine("czmove group [group_id] [zone_short_name]");
		connection->SendLine("czmove guild [guild_id] [instance_id]");
		connection->SendLine("czmove guild [guild_id] [zone_short_name]");
		connection->SendLine("czmove name [character_name] [instance_id]");
		connection->SendLine("czmove name [character_name] [zone_short_name]");
		connection->SendLine("czmove raid [raid_id] [instance_id]");
		connection->SendLine("czmove raid [raid_id] [zone_short_name]");
		return;
	}

	std::string name;
	int         update_identifier = 0;

	if (!is_name) {
		if (Strings::IsNumber(args[1])) {
			update_identifier = Strings::ToInt(args[1]);
		}

		if (!update_identifier) {
			connection->SendLine(fmt::format("Identifier invalid for '{}'.", type));
			return;
		}
	} else if (is_name) {
		if (!Strings::IsNumber(args[1])) {
			name = Strings::UcFirst(Strings::ToLower(args[1]));
		}

		if (name.empty()) {
			connection->SendLine("Empty name is invalid.");
			return;
		}
	}

	const auto&  zone_short_name = !Strings::IsNumber(args[2]) ? args[2] : "";
	const uint16 instance_id     = Strings::IsNumber(args[2]) ? static_cast<uint16>(Strings::ToUnsignedInt(args[2])) : 0;

	const auto& z = !zone_short_name.empty() ? ZoneStore::Instance()->GetZone(zone_short_name) : nullptr;

	if (z && !z->id) {
		connection->SendLine(fmt::format("No zone with the short name '{}' exists.", zone_short_name));
		return;
	}

	uint8 update_type;

	if (is_character) {
		update_type = CZUpdateType_Character;
	} else if (is_expedition) {
		update_type = CZUpdateType_Expedition;
	} else if (is_group) {
		update_type = CZUpdateType_Group;
	} else if (is_guild) {
		update_type = CZUpdateType_Guild;
	} else if (is_name) {
		update_type = CZUpdateType_ClientName;
	} else if (is_raid) {
		update_type = CZUpdateType_Raid;
	}

	auto pack = new ServerPacket(ServerOP_CZMove, sizeof(CZMove_Struct));
	auto m = (CZMove_Struct*) pack->pBuffer;

	if (!name.empty()) {
		m->client_name = name;
	}

	m->instance_id       = instance_id;
	m->update_identifier = update_identifier;
	m->update_type       = update_type;
	m->update_subtype    = !instance_id ? CZMoveUpdateSubtype_MoveZone : CZMoveUpdateSubtype_MoveZoneInstance;

	if (!zone_short_name.empty()) {
		m->zone_short_name = zone_short_name;
	}

	ZSList::Instance()->SendPacket(pack);
	safe_delete(pack);

	connection->SendLine(
		fmt::format(
			"Moving player(s) to {} by {} with an identifier of {}.",
			!instance_id ? fmt::format("{} ({})", z->long_name, z->short_name) : fmt::format("Instance ID {}", instance_id),
			type,
			!is_name ? std::to_string(update_identifier) : name
		)
	);
}

void ConsoleWorldWideMove(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
) {
	if (args.size() < 1) {
		connection->SendLine("wwmove [instance_id]");
		connection->SendLine("wwmove [instance_id] [min_status]");
		connection->SendLine("wwmove [instance_id] [min_status] [max_status]");
		connection->SendLine("wwmove [zone_short_name]");
		connection->SendLine("wwmove [zone_short_name] [min_status]");
		connection->SendLine("wwmove [zone_short_name] [min_status] [max_status]");
		return;
	}
\
	const auto&  zone_short_name = !Strings::IsNumber(args[2]) ? args[2] : "";
	const uint16 instance_id     = Strings::IsNumber(args[2]) ? static_cast<uint16>(Strings::ToUnsignedInt(args[2])) : 0;

	const auto& z = !zone_short_name.empty() ? ZoneStore::Instance()->GetZone(zone_short_name) : nullptr;

	if (z && !z->id) {
		connection->SendLine(fmt::format("No zone with the short name '{}' exists.", zone_short_name));
		return;
	}

	uint8 min_status = AccountStatus::Player;
	uint8 max_status = AccountStatus::Player;

	if (args.size() >= 2 && Strings::IsNumber(args[1])) {
		min_status = static_cast<uint8>(Strings::ToUnsignedInt(args[1]));
	}

	if (args.size() >= 3 && Strings::IsNumber(args[2])) {
		max_status = static_cast<uint8>(Strings::ToUnsignedInt(args[2]));
	}

	auto pack = new ServerPacket(ServerOP_WWMove, sizeof(WWMove_Struct));
	auto* WWM = (WWMove_Struct*) pack->pBuffer;

	WWM->update_type = !instance_id ? WWMoveUpdateType_MoveZone : WWMoveUpdateType_MoveZoneInstance;
	WWM->instance_id = instance_id;
	WWM->min_status  = min_status;
	WWM->max_status  = max_status;

	strn0cpy(WWM->zone_short_name, zone_short_name.c_str(), sizeof(WWM->zone_short_name));

	ZSList::Instance()->SendPacket(pack);
	safe_delete(pack);

	connection->SendLine(
		fmt::format(
			"Moving player(s) to {} for players with a status between {} and {}.",
			!instance_id ? fmt::format("{} ({})", z->long_name, z->short_name) : fmt::format("Instance ID {}", instance_id),
			min_status,
			max_status
		)
	);
}

void ConsoleWWMarquee(
	EQ::Net::ConsoleServerConnection* connection,
	const std::string& command,
	const std::vector<std::string>& args
)
{
	if (args.size() < 2) {
		connection->SendLine("Usage: wwmarquee <type> <message>");
		return;
	}

	const uint32 type    = Strings::IsNumber(args[0]) ? Strings::ToUnsignedInt(args[0]) : 0;
	std::string  message = Strings::Join(std::vector<std::string>(args.begin() + 1, args.end()), " ");
	if (message.empty()) {
		connection->SendLine("Message cannot be empty.");
		return;
	}

	auto pack = new ServerPacket(ServerOP_WWMarquee, sizeof(WWMarquee_Struct));
	auto* wwm = (WWMarquee_Struct*)pack->pBuffer;

	wwm->type       = type;
	wwm->priority   = 510;
	wwm->fade_in    = 0;
	wwm->fade_out   = 0;
	wwm->duration   = 5000;
	wwm->min_status = AccountStatus::Player;
	wwm->max_status = AccountStatus::Player;

	strn0cpy(wwm->message, message.c_str(), sizeof(wwm->message));

	ZSList::Instance()->SendPacket(pack);
	safe_delete(pack);

	connection->SendLine(fmt::format("Sent world marquee type {}: {}", type, message));
}

/**
 * @param console
 */
void RegisterConsoleFunctions(std::unique_ptr<EQ::Net::ConsoleServer>& console)
{
	console->RegisterLogin(std::bind(CheckLogin, std::placeholders::_1, std::placeholders::_2));
	console->RegisterCall("acceptmessages", 50, "acceptmessages [on/off]", std::bind(ConsoleAcceptMessages, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("api", 200, "api", std::bind(ConsoleApi, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("auction", 50, "auction [message]", std::bind(ConsoleAuction, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("broadcast", 50, "broadcast [message]", std::bind(ConsoleBroadcast, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("czcast", 50, "czcast [type] [identifier] [spell_id]", std::bind(ConsoleCrossZoneCastSpell, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("czmove", 50, "czmove [type] [identifier] [instance_id|zone_short_name] - instance_id and zone_short_name are interchangeable", std::bind(ConsoleCrossZoneMove, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("echo", 50, "echo [on/off]", std::bind(ConsoleNull, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("emote", 50, "emote [zonename or charname or world] [type] [message]", std::bind(ConsoleEmote, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("flag", 200, "flag [status] [accountname]", std::bind(ConsoleFlag, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("gmsay", 50, "gmsay [message]", std::bind(ConsoleGMSay, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("guildsay", 50, "guildsay [Character Name] [Guild ID] [Message]", std::bind(ConsoleGuildSay, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("iplookup", 50, "iplookup [name]", std::bind(ConsoleIpLookup, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("kick", 150, "kick [charname]", std::bind(ConsoleKick, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("lock", 150, "lock", std::bind(ConsoleLock, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("lsreconnect", 50, "LSReconnect", std::bind(ConsoleNull, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("md5", 50, "md5", std::bind(ConsoleMd5, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("ooc", 50, "ooc [message]", std::bind(ConsoleOOC, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("ping", 50, "ping", std::bind(ConsoleNull, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("parcelnotify", 200, "parcelnotify [character id] [parcel ids]", ConsoleParcelNotify);
	console->RegisterCall("reloadworld", 200, "reloadworld", std::bind(ConsoleReloadWorld, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("reloadzonequests", 200, "reloadzonequests [zone_short_name]", std::bind(ConsoleReloadZoneQuests, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("setpass", 200, "setpass [account_name] [new_password]", std::bind(ConsoleSetPass, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("signalcharbyname", 50, "signalcharbyname charname ID", std::bind(ConsoleSignalCharByName, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("supportchat", 200, "supportchat", ConsoleSupportChat);
	console->RegisterCall("supportmove", 200, "supportmove [character] [account] [corpse or 0] [source zone] [source instance] [destination] [instance x y z heading]", ConsoleSupportMove);
	console->RegisterCall("supportlocation", 200, "supportlocation [character] [account]", ConsoleSupportLocation);
	console->RegisterCall("tell", 50, "tell [name] [message]", std::bind(ConsoleTell, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("unlock", 150, "unlock", std::bind(ConsoleUnlock, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("uptime", 50, "uptime [zone_server_id]", std::bind(ConsoleUptime, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("version", 50, "version", std::bind(ConsoleVersion, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("who", 50, "who", std::bind(ConsoleWho, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("whoami", 50, "whoami", std::bind(ConsoleWhoami, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("worldshutdown", 200, "worldshutdown", std::bind(ConsoleWorldShutdown, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("wwcast", 50, "wwcast [spell_id] [min_status] [max_status] - min_status and max_status are optional", std::bind(ConsoleWorldWideCastSpell, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall(
		"wwmarquee",
		50,
		"wwmarquee <type> <message>",
		std::bind(ConsoleWWMarquee, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
	);
	console->RegisterCall("wwmove", 50, "wwmove [instance_id|zone_short_name] [min_status] [max_status] -  min_status and max_status are optional, instance_id and zone_short_name are interchangeable", std::bind(ConsoleWorldWideMove, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("zonebootup", 150, "zonebootup [zone_server_id] [zone_short_name]", std::bind(ConsoleZoneBootup, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("zonelock", 150, "zonelock [list|lock|unlock] [zone_short_name]", std::bind(ConsoleZoneLock, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("zoneshutdown", 150, "zoneshutdown [zone_short_name or zone_server_id]", std::bind(ConsoleZoneShutdown, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("zonestatus", 50, "zonestatus", std::bind(ConsoleZoneStatus, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("quit", 50, "quit", std::bind(ConsoleQuit, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("exit", 50, "exit", std::bind(ConsoleQuit, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}
