#include "web_interface.h"

#include "common/json/json.h"
#include "world/web_interface_eqw.h"

#include <sstream>

WebInterface::WebInterface(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection)
{
	m_connection = connection;
	m_connection->OnMessage(ServerOP_WebInterfaceCall, std::bind(&WebInterface::OnCall, this, std::placeholders::_1, std::placeholders::_2));
	RegisterEQW(this);
}

WebInterface::~WebInterface()
{
}

void WebInterface::OnCall(uint16 opcode, EQ::Net::Packet &p)
{
	Json::Value root;
	try {
		auto json_str = p.GetCString(0);
		std::stringstream ss(json_str);
		ss >> root;
	}
	catch (std::exception &) {
		SendError("Could not parse request");
		return;
	}

	std::string method;
	Json::Value params;
	std::string id;

	try {
		method = root["method"].asString();
		if (method.length() == 0) {
			SendError("Invalid request: method not supplied");
			return;
		}
	}
	catch (std::exception &) {
		SendError("Invalid request: method not supplied");
		return;
	}

	//optional "params" -> Json::Value
	try {
		params = root["params"];
	}
	catch (std::exception &) {
		params = nullptr;
	}
	
	//optional "id" needs to be string
	try {
		id = root["id"].asString();
	}
	catch (std::exception &) {
		id = "";
	}

	//check for registered method
	auto iter = m_calls.find(method);
	if (iter == m_calls.end()) {
		//if not exist then error
		SendError("Invalid request: method not found", id);
		return;
	}

	iter->second(this, method, id, params);
}

void WebInterface::Send(const Json::Value &value)
{
	try {
		std::stringstream ss;
		ss << value;

		EQ::Net::DynamicPacket p;
		p.PutString(0, ss.str());
		m_connection->Send(ServerOP_WebInterfaceCall, p);
	}
	catch (std::exception &) {
		//Log error
	}
}

void WebInterface::SendError(const std::string &message)
{
	Json::Value error;
	error["error"] = Json::Value();
	error["error"]["message"] = message;

	Send(error);
}

void WebInterface::SendError(const std::string &message, const std::string &id)
{
	Json::Value error;
	error["id"] = id;
	error["error"] = Json::Value();
	error["error"]["message"] = message;

	Send(error);
}

void WebInterface::SendEvent(const Json::Value &value)
{
	try {
		std::stringstream ss;
		ss << value;

		EQ::Net::DynamicPacket p;
		p.PutString(0, ss.str());
		m_connection->Send(ServerOP_WebInterfaceEvent, p);
	}
	catch (std::exception &) {
		//Log error
	}
}

void WebInterface::AddCall(const std::string &method, WebInterfaceCall call)
{
	m_calls.emplace(std::make_pair(method, call));
}

void WebInterface::SendResponse(const std::string &id, const Json::Value &response)
{
	Json::Value out;
	if(!id.empty())
		out["id"] = id;
	out["response"] = response;

	Send(out);
}

WebInterfaceList::WebInterfaceList()
{
}

WebInterfaceList::~WebInterfaceList()
{
}

void WebInterfaceList::AddConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection)
{
	m_interfaces.emplace(std::make_pair(connection->GetUUID(), std::make_unique<WebInterface>(connection)));
}

void WebInterfaceList::RemoveConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection)
{
	auto iter = m_interfaces.find(connection->GetUUID());
	if (iter != m_interfaces.end()) {
		m_interfaces.erase(iter);
		return;
	}
}

void WebInterfaceList::SendResponse(const std::string &uuid, std::string &id, const Json::Value &response) {
	auto iter = m_interfaces.find(uuid);
	if (iter != m_interfaces.end()) {
		iter->second->SendResponse(id, response);
	}
}

void WebInterfaceList::SendEvent(const Json::Value &value) {
	for (auto &i : m_interfaces) {
		i.second->SendEvent(value);
	}
}

void WebInterfaceList::SendError(const std::string &uuid, const std::string &message) {
	auto iter = m_interfaces.find(uuid);
	if (iter != m_interfaces.end()) {
		iter->second->SendError(message);
	}
}

void WebInterfaceList::SendError(const std::string &uuid, const std::string &message, const std::string &id) {
	auto iter = m_interfaces.find(uuid);
	if (iter != m_interfaces.end()) {
		iter->second->SendError(message, id);
	}
}
