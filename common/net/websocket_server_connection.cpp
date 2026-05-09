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
#include "websocket_server_connection.h"

#include "common/net/websocket_server.h"
#include "common/timer.h"
#include "common/util/uuid.h"

#include "fmt/format.h"
#include <sstream>

struct EQ::Net::WebsocketServerConnection::Impl {
	WebsocketServer *parent;
	std::shared_ptr<TCPConnection> connection;
	std::shared_ptr<websocket_connection> ws_connection;
	std::string id;
	bool authorized;
	std::string account_name;
	uint32 account_id;
	int status;
};

EQ::Net::WebsocketServerConnection::WebsocketServerConnection(WebsocketServer *parent, 
	std::shared_ptr<TCPConnection> connection,
	std::shared_ptr<websocket_connection> ws_connection)
{
	_impl = std::make_unique<Impl>();
	_impl->parent = parent;
	_impl->connection = connection;
	_impl->id = EQ::Util::UUID::Generate().ToString();
	_impl->authorized = false;
	_impl->account_id = 0;
	_impl->status = 0;
	_impl->ws_connection = ws_connection;
	_impl->ws_connection->set_message_handler(std::bind(&WebsocketServerConnection::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
	_impl->ws_connection->start();

	connection->OnDisconnect([this](EQ::Net::TCPConnection *connection) {
		_impl->parent->ReleaseConnection(this);
	});
	
	connection->OnRead([this](EQ::Net::TCPConnection *c, const unsigned char *buffer, size_t buffer_size) {
		_impl->ws_connection->read_all((const char*)buffer, buffer_size);
	});

	connection->Start();
}

EQ::Net::WebsocketServerConnection::~WebsocketServerConnection()
{
}

std::string EQ::Net::WebsocketServerConnection::GetID() const
{
	return _impl->id;
}

bool EQ::Net::WebsocketServerConnection::IsAuthorized() const
{
	return _impl->authorized;
}

std::string EQ::Net::WebsocketServerConnection::GetAccountName() const
{
	return _impl->account_name;
}

uint32 EQ::Net::WebsocketServerConnection::GetAccountID() const
{
	return _impl->account_id;
}

int EQ::Net::WebsocketServerConnection::GetStatus() const
{
	return _impl->status;
}

std::string EQ::Net::WebsocketServerConnection::RemoteIP() const
{
	return _impl->connection->RemoteIP();
}

int EQ::Net::WebsocketServerConnection::RemotePort() const
{
	return _impl->connection->RemotePort();
}

std::shared_ptr<EQ::Net::websocket_connection> EQ::Net::WebsocketServerConnection::GetWebsocketConnection()
{
	return _impl->ws_connection;
}

std::shared_ptr<EQ::Net::TCPConnection> EQ::Net::WebsocketServerConnection::GetTCPConnection()
{
	return _impl->connection;
}

void EQ::Net::WebsocketServerConnection::OnMessage(websocketpp::connection_hdl hdl, websocket_message_ptr msg)
{
	BenchTimer timer;
	timer.reset();

	if (msg->get_opcode() == websocketpp::frame::opcode::text) {
		try {
			auto &payload = msg->get_payload();

			std::stringstream ss(payload);
			Json::Value root;

			ss >> root;

			auto method = root["method"].asString();
			auto params = root["params"];
			std::string id = "";

			auto idNode = root["id"];
			if (!idNode.isNull() && idNode.isString()) {
				id = idNode.asString();
			}

			Json::Value response;
			response["type"] = "method";
			response["data"] = _impl->parent->HandleRequest(this, method, params);
			response["method"] = method;
			if(id != "") {
				response["id"] = id;
			}

			SendResponse(response, timer.elapsed());
		}
		catch (std::exception &ex) {
			Json::Value error;
			error["type"] = "method";
			error["error"] = fmt::format("{0}", ex.what());
			SendResponse(error, timer.elapsed());
		}
	}
}

void EQ::Net::WebsocketServerConnection::SendResponse(const Json::Value &response, double time_elapsed)
{
	Json::Value root = response;
	root["execution_time"] = std::to_string(time_elapsed);

	std::stringstream payload;
	payload << root;

	_impl->ws_connection->send(payload.str());
}

void EQ::Net::WebsocketServerConnection::SetAuthorized(bool v, const std::string account_name, uint32 account_id, int status)
{
	_impl->authorized = v;
	_impl->account_name = account_name;
	_impl->account_id = account_id;
	_impl->status = status;
}
