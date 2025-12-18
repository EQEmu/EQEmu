#include "ucs.h"

#include "common/eqemu_logsys.h"
#include "common/event/timer.h"
#include "common/md5.h"
#include "common/misc_functions.h"
#include "common/packet_dump.h"
#include "world/world_config.h"

UCSConnection::UCSConnection()
{
	connection = 0;
}

void UCSConnection::SetConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> inStream)
{
	if (inStream && connection && connection->Handle()) {
		LogInfo("Incoming UCS Connection while we were already connected to a UCS");
		connection->Handle()->Disconnect();
	}

	connection = inStream;
	if (connection) {
		connection->OnMessage(
			std::bind(
				&UCSConnection::ProcessPacket,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
	}
}

const std::shared_ptr<EQ::Net::ServertalkServerConnection> &UCSConnection::GetConnection() const
{
	return connection;
}

void UCSConnection::ProcessPacket(uint16 opcode, EQ::Net::Packet &p)
{
	if (!connection)
		return;

	ServerPacket tpack(opcode, p);
	ServerPacket *pack = &tpack;

	switch (opcode)
	{
		case 0:
			break;

		case ServerOP_KeepAlive:
		{
			// ignore this
			break;
		}
		case ServerOP_ZAAuth:
		{
			LogInfo("Got authentication from UCS when they are already authenticated");
			break;
		}
		default:
		{
			LogInfo("Unknown ServerOPcode from UCS {:#04x}, size [{}]", opcode, pack->size);
			DumpPacket(pack->pBuffer, pack->size);
			break;
		}
	}
}

void UCSConnection::SendPacket(ServerPacket* pack)
{
	if (!connection)
		return;

	connection->SendPacket(pack);
}

void UCSConnection::SendMessage(const char *From, const char *Message)
{
	auto pack = new ServerPacket(ServerOP_UCSMessage, strlen(From) + strlen(Message) + 2);

	char *Buffer = (char *)pack->pBuffer;

	VARSTRUCT_ENCODE_STRING(Buffer, From);
	VARSTRUCT_ENCODE_STRING(Buffer, Message);

	SendPacket(pack);
	safe_delete(pack);
}
