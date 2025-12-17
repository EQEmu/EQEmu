#include "zoneserver.h"

#include "common/discord/discord_manager.h"
#include "common/eqemu_logsys.h"
#include "common/events/player_event_logs.h"
#include "common/global_define.h"
#include "common/repositories/player_event_logs_repository.h"

ZoneServer::ZoneServer(
	std::shared_ptr<EQ::Net::ServertalkServerConnection> in_connection,
	EQ::Net::ConsoleServer *in_console
)
	: m_connection(in_connection)
{

	m_connection->OnMessage(std::bind(&ZoneServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
	m_console = in_console;
}

ZoneServer::~ZoneServer()
{
}

void ZoneServer::HandleMessage(uint16 opcode, const EQ::Net::Packet &p)
{
	ServerPacket tpack(opcode, p);
	auto         pack = &tpack;

	switch (opcode) {
		case ServerOP_PlayerEvent: {
			auto n = PlayerEvent::PlayerEventContainer{};
			auto s = reinterpret_cast<ServerSendPlayerEvent_Struct *>(pack->pBuffer);
			EQ::Util::MemoryStreamReader ss(s->cereal_data, s->cereal_size);
			cereal::BinaryInputArchive archive(ss);
			archive(n);

			PlayerEventLogs::Instance()->AddToQueue(n.player_event_log);

			DiscordManager::Instance()->QueuePlayerEventMessage(n);
			break;
		}
		default: {
			LogInfo("Unknown ServerOP Received [{}]", opcode);
			break;
		}
	}
}

void ZoneServer::SendPlayerEventLogSettings()
{
	EQ::Net::DynamicPacket                                                dyn_pack;
	std::vector<PlayerEventLogSettingsRepository::PlayerEventLogSettings> settings(
		PlayerEventLogs::Instance()->GetSettings(),
		PlayerEventLogs::Instance()->GetSettings() + PlayerEvent::EventType::MAX
	);

	dyn_pack.PutSerialize(0, settings);

	auto packet_size = sizeof(ServerSendPlayerEvent_Struct) + dyn_pack.Length();

	auto pack = std::make_unique<ServerPacket>(
		ServerOP_SendPlayerEventSettings,
		static_cast<uint32_t>(packet_size)
	);

	auto* buf        = reinterpret_cast<ServerSendPlayerEvent_Struct*>(pack->pBuffer);
	buf->cereal_size = static_cast<uint32_t>(dyn_pack.Length());
	memcpy(buf->cereal_data, dyn_pack.Data(), dyn_pack.Length());

	SendPacket(pack.release());
}
