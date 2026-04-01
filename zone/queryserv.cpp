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
#include "queryserv.h"

#include "common/events/player_event_logs.h"
#include "common/servertalk.h"
#include "common/strings.h"
#include "zone/worldserver.h"

extern WorldServer worldserver;
extern QueryServ*  QServ;

QueryServ::QueryServ()
{
}

QueryServ::~QueryServ()
{
}

void QueryServ::SendQuery(std::string Query)
{
	auto pack = new ServerPacket(ServerOP_QSSendQuery, Query.length() + 5);
	pack->WriteUInt32(Query.length()); /* Pack Query String Size so it can be dynamically broken out at queryserv */
	pack->WriteString(Query.c_str());  /* Query */
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QueryServ::Connect()
{
	m_connection = std::make_unique<EQ::Net::ServertalkClient>(
		Config->QSHost,
		Config->QSPort,
		false,
		"Zone",
		Config->SharedKey
	);
	m_connection->OnMessage(std::bind(&QueryServ::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
	m_connection->OnConnect([this](EQ::Net::ServertalkClient* client)
	{
		m_is_qs_connected = true;
		LogInfo(
			"Query Server connection established to [{}] [{}]",
			client->Handle()->RemoteIP(),
			client->Handle()->RemotePort()
		);
	});

	LogInfo(
		"New Query Server connection to [{}:{}]",
		Config->QSHost,
		Config->QSPort
	);
}

bool QueryServ::SendPacket(ServerPacket* pack)
{
	if (m_connection.get() == nullptr) {
		Connect();
	}

	if (!m_connection.get()) {
		return false;
	}

	if (m_is_qs_connected) {
		m_connection->SendPacket(pack);
		return true;
	}

	return false;
}

void QueryServ::HandleMessage(uint16 opcode, const EQ::Net::Packet& p)
{
	ServerPacket tpack(opcode, p);
	auto         pack = &tpack;

	switch (opcode) {
	case ServerOP_SendPlayerEventSettings:
		{
			if (pack->size < sizeof(ServerSendPlayerEvent_Struct)) {
				LogError("ServerOP_SendPlayerEventSettings: packet too small");
				break;
			}

			auto*          data          = reinterpret_cast<const ServerSendPlayerEvent_Struct*>(pack->pBuffer);
			const uint32_t expected_size = sizeof(ServerSendPlayerEvent_Struct) + data->cereal_size;

			if (pack->size < expected_size) {
				LogError(
					"ServerOP_SendPlayerEventSettings: packet size mismatch, expected {}, got {}", expected_size,
					pack->size
				);
				break;
			}

			std::vector<PlayerEventLogSettingsRepository::PlayerEventLogSettings> settings;

			try {
				EQ::Util::MemoryStreamReader ms(const_cast<char*>(data->cereal_data), data->cereal_size);
				cereal::BinaryInputArchive   ar(ms);
				ar(settings);
			}
			catch (const std::exception& ex) {
				LogError("Failed to deserialize PlayerEventLogSettings: {}", ex.what());
				break;
			}

			PlayerEventLogs::Instance()->LoadPlayerEventSettingsFromQS(settings);
			LogInfo("Loaded {} PlayerEventLogSettings from queryserv", settings.size());
			break;
		}

	default:
		{
			LogInfo("Unknown ServerOP Received [{}]", opcode);
			break;
		}
	}
}
