#include "queryserv.h"

#include "common/eqemu_logsys.h"
#include "common/md5.h"
#include "common/packet_dump.h"
#include "world/clientlist.h"
#include "world/world_config.h"
#include "world/zonelist.h"

QueryServConnection::QueryServConnection()
{
}

void QueryServConnection::AddConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection)
{
	//Set handlers
	connection->OnMessage(ServerOP_QueryServGeneric, std::bind(&QueryServConnection::HandleGenericMessage, this, std::placeholders::_1, std::placeholders::_2));
	connection->OnMessage(ServerOP_LFGuildUpdate, std::bind(&QueryServConnection::HandleLFGuildUpdateMessage, this, std::placeholders::_1, std::placeholders::_2));
	m_streams.emplace(std::make_pair(connection->GetUUID(), connection));
}

void QueryServConnection::RemoveConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection)
{
	auto iter = m_streams.find(connection->GetUUID());
	if (iter != m_streams.end()) {
		m_streams.erase(iter);
	}
}

void QueryServConnection::HandleGenericMessage(uint16_t opcode, EQ::Net::Packet &p) {
	uint32 ZoneID = p.GetUInt32(0);
	uint16 InstanceID = p.GetUInt32(4);
	ServerPacket pack(opcode, p);
	ZSList::Instance()->SendPacket(ZoneID, InstanceID, &pack);
}

void QueryServConnection::HandleLFGuildUpdateMessage(uint16_t opcode, EQ::Net::Packet &p) {
	ServerPacket pack(opcode, p);
	ZSList::Instance()->SendPacket(&pack);
}

bool QueryServConnection::SendPacket(ServerPacket* pack)
{
	for (auto &stream : m_streams) {
		stream.second->SendPacket(pack);
	}

	return true;
}

