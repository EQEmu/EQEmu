#include "websocket_server.h"

#include "common/emu_constants.h"
#include "common/event/event_loop.h"
#include "common/event/timer.h"

#include "fmt/format.h"
#include <array>
#include <map>
#include <unordered_set>

struct MethodHandlerEntry
{
	MethodHandlerEntry() {
		status = 0;
	}

	MethodHandlerEntry(EQ::Net::WebsocketServer::MethodHandler h, int s) {
		handler = h;
		status = s;
	}

	EQ::Net::WebsocketServer::MethodHandler handler;
	int status;
};

struct EQ::Net::WebsocketServer::Impl
{
	std::unique_ptr<TCPServer> server;
	std::unique_ptr<EQ::Timer> ping_timer;
	std::map<std::shared_ptr<websocket_connection>, std::unique_ptr<WebsocketServerConnection>> connections;
	std::map<std::string, MethodHandlerEntry> methods;
	websocket_server ws_server;
	LoginHandler login_handler;
	std::array<std::unordered_set<WebsocketServerConnection*>, SubscriptionEventMax> subscriptions;
};

EQ::Net::WebsocketServer::WebsocketServer(const std::string &addr, int port)
{
	_impl = std::make_unique<Impl>();
	_impl->server = std::make_unique<EQ::Net::TCPServer>();
	_impl->server->Listen(addr, port, false, [this](std::shared_ptr<EQ::Net::TCPConnection> connection) {
		auto wsc = _impl->ws_server.get_connection();
		WebsocketServerConnection *c = new WebsocketServerConnection(this, connection, wsc);
		_impl->connections.insert(std::make_pair(wsc, std::unique_ptr<WebsocketServerConnection>(c)));
	});

	_impl->ws_server.set_write_handler(
		[this](websocketpp::connection_hdl hdl, char const *data, size_t size) -> websocketpp::lib::error_code {
		auto c = _impl->ws_server.get_con_from_hdl(hdl);
		auto iter = _impl->connections.find(c);
		if (iter != _impl->connections.end()) {
			iter->second->GetTCPConnection()->Write(data, size);
		}
		
		return websocketpp::lib::error_code();
	});

	_impl->ping_timer = std::make_unique<EQ::Timer>(5000, true, [this](EQ::Timer *t) {
		auto iter = _impl->connections.begin();

		while (iter != _impl->connections.end()) {
			try {
				auto &connection = iter->second;
				connection->GetWebsocketConnection()->ping("keepalive");
			}
			catch (std::exception &) {
				iter->second->GetTCPConnection()->Disconnect();
			}

			iter++;
		}
	});

	_impl->methods.insert(std::make_pair("login", MethodHandlerEntry(std::bind(&WebsocketServer::Login, this, std::placeholders::_1, std::placeholders::_2), 0)));
	_impl->methods.insert(std::make_pair("subscribe", MethodHandlerEntry(std::bind(&WebsocketServer::Subscribe, this, std::placeholders::_1, std::placeholders::_2), 0)));
	_impl->methods.insert(std::make_pair("unsubscribe", MethodHandlerEntry(std::bind(&WebsocketServer::Unsubscribe, this, std::placeholders::_1, std::placeholders::_2), 0)));
	_impl->login_handler = [](const WebsocketServerConnection* connection, const std::string& user, const std::string& pass) {
		WebsocketLoginStatus ret;
		ret.account_name = "admin";
		
		if (connection->RemoteIP() == "127.0.0.1" || connection->RemoteIP() == "::") {
			ret.logged_in = true;
			return ret;
		}

		ret.logged_in = false;
		return ret;
	};

	_impl->ws_server.clear_access_channels(websocketpp::log::alevel::all);
}

EQ::Net::WebsocketServer::~WebsocketServer()
{
}

void EQ::Net::WebsocketServer::ReleaseConnection(WebsocketServerConnection *connection)
{
	UnsubscribeAll(connection);

	_impl->connections.erase(connection->GetWebsocketConnection());
}

Json::Value EQ::Net::WebsocketServer::HandleRequest(WebsocketServerConnection *connection, const std::string &method, const Json::Value &params)
{
	Json::Value err;
	if (method != "login") {
		if (!connection->IsAuthorized()) {
			throw WebsocketException("Not logged in");
		}
	}

	auto iter = _impl->methods.find(method);
	if (iter != _impl->methods.end()) {
		auto &s = iter->second;
		if (s.status > connection->GetStatus()) {
			throw WebsocketException("Status too low");
		}

		return s.handler(connection, params);
	}

	throw WebsocketException("Unknown Method");
}

void EQ::Net::WebsocketServer::SetMethodHandler(const std::string &method, MethodHandler handler, int required_status)
{
	//Reserved method names
	if (method == "subscribe" || 
		method == "unsubscribe" || 
		method == "login") {
		return;
	}

	_impl->methods[method] = MethodHandlerEntry(handler, required_status);
}

void EQ::Net::WebsocketServer::SetLoginHandler(LoginHandler handler)
{
	_impl->login_handler = handler;
}

void EQ::Net::WebsocketServer::DispatchEvent(WebsocketSubscriptionEvent evt, Json::Value data, int required_status)
{
	try {
		Json::Value event_obj;
		event_obj["type"] = "event";
		event_obj["event"] = (int)evt;
		event_obj["data"] = data;

		std::stringstream payload;
		payload << event_obj;

		for (auto &iter : _impl->connections) {
			auto &c = iter.second;

			if (c->GetStatus() >= required_status && IsSubscribed(c.get(), evt)) {
				c->GetWebsocketConnection()->send(payload.str());
			}
		}
	}
	catch (std::exception &) {
	}
}

Json::Value EQ::Net::WebsocketServer::Login(WebsocketServerConnection *connection, const Json::Value &params)
{
	Json::Value ret;

	try {
		Json::Value ret;

		auto user = params[0].asString();
		auto pass = params[1].asString();
		
		auto r = _impl->login_handler(connection, user, pass);

		if (r.logged_in) {
			connection->SetAuthorized(true, r.account_name, r.account_id, AccountStatus::Max);
			ret["status"] = "Ok";
		}
		else if (user == "admin" && (connection->RemoteIP() == "127.0.0.1" || connection->RemoteIP() == "::")) {
			r.logged_in = true;
			r.account_id = 0;
			connection->SetAuthorized(true, r.account_name, r.account_id, AccountStatus::Max);
			ret["status"] = "Ok";
		}
		else {
			connection->SetAuthorized(false, "", 0, 0);
			ret["status"] = "Not Authorized";
		}

		return ret;
	}
	catch (std::exception &) {
		throw WebsocketException("Unable to process login request");
	}
}

Json::Value EQ::Net::WebsocketServer::Subscribe(WebsocketServerConnection *connection, const Json::Value &params)
{
	Json::Value ret;

	try {
		auto evt = params[0].asInt();
		if (evt < 0 || evt >= SubscriptionEventMax) {
			throw WebsocketException("Not a valid subscription");
		}

		DoSubscribe(connection, (WebsocketSubscriptionEvent)evt);
		ret["status"] = "Ok";
		return ret;
	}
	catch (WebsocketException &ex) {
		throw ex;
	}
	catch (std::exception &) {
		throw WebsocketException("Unable to process unsubscribe request");
	}
}

Json::Value EQ::Net::WebsocketServer::Unsubscribe(WebsocketServerConnection *connection, const Json::Value &params)
{
	Json::Value ret;

	try {
		auto evt = params[0].asInt();
		if (evt < 0 || evt >= SubscriptionEventMax) {
			throw WebsocketException("Not a valid subscription");
		}

		DoUnsubscribe(connection, (WebsocketSubscriptionEvent)evt);
		ret["status"] = "Ok";
		return ret;
	}
	catch (WebsocketException &ex) {
		throw ex;
	}
	catch (std::exception &) {
		throw WebsocketException("Unable to process unsubscribe request");
	}
}

void EQ::Net::WebsocketServer::DoSubscribe(WebsocketServerConnection *connection, WebsocketSubscriptionEvent sub) {
	auto &s = _impl->subscriptions[sub];

	auto iter = s.find(connection);
	if (iter == s.end()) {
		s.insert(connection);
	}
}

void EQ::Net::WebsocketServer::DoUnsubscribe(WebsocketServerConnection *connection, WebsocketSubscriptionEvent sub) {
	auto &s = _impl->subscriptions[sub];
	s.erase(connection);
}

bool EQ::Net::WebsocketServer::IsSubscribed(WebsocketServerConnection *connection, WebsocketSubscriptionEvent sub) {
	auto &s = _impl->subscriptions[sub];
	return s.count(connection) == 1;
}

void EQ::Net::WebsocketServer::UnsubscribeAll(WebsocketServerConnection *connection) {
	for (auto i = 0; i < SubscriptionEventMax; ++i) {
		DoUnsubscribe(connection, (WebsocketSubscriptionEvent)i);
	}
}
