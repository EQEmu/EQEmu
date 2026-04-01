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
#pragma once

#include "common/json/json.h"
#include "common/net/servertalk_server_connection.h"

#include <functional>
#include <map>
#include <string>

class WebInterface
{
public:
	typedef std::function<void(WebInterface *, const std::string&, const std::string&, const Json::Value&)> WebInterfaceCall;
	WebInterface(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection);
	~WebInterface();

	std::string GetUUID() const { return m_connection->GetUUID(); }
	void SendResponse(const std::string &id, const Json::Value &response);
	void SendError(const std::string &message);
	void SendError(const std::string &message, const std::string &id);
	void SendEvent(const Json::Value &value);
	void AddCall(const std::string &method, WebInterfaceCall call);
private:
	void OnCall(uint16 opcode, EQ::Net::Packet &p);
	void Send(const Json::Value &value);

	std::shared_ptr<EQ::Net::ServertalkServerConnection> m_connection;
	std::map<std::string, WebInterfaceCall> m_calls;
};

class WebInterfaceList
{
public:
	WebInterfaceList();
	~WebInterfaceList();

	void AddConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection);
	void RemoveConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection);
	void SendResponse(const std::string &uuid, std::string &id, const Json::Value &response);
	void SendEvent(const Json::Value &value);
	void SendError(const std::string &uuid, const std::string &message);
	void SendError(const std::string &uuid, const std::string &message, const std::string &id);

	static WebInterfaceList* Instance()
	{
		static WebInterfaceList instance;
		return &instance;
	}
private:
	std::map<std::string, std::unique_ptr<WebInterface>> m_interfaces;
};
