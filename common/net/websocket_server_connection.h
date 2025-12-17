#pragma once

#include "common/json/json-forwards.h"
#include "common/net/tcp_server.h"
#include "common/types.h"

#include "websocketpp/config/core.hpp"
#include "websocketpp/server.hpp"

namespace EQ
{
	namespace Net
	{
		typedef websocketpp::server<websocketpp::config::core> websocket_server;
		typedef websocketpp::connection<websocketpp::config::core> websocket_connection;
		typedef websocket_server::message_ptr websocket_message_ptr;

		class WebsocketServer;
		class WebsocketServerConnection
		{
		public:
			WebsocketServerConnection(WebsocketServer *parent, 
				std::shared_ptr<TCPConnection> connection, 
				std::shared_ptr<websocket_connection> ws_connection);
			~WebsocketServerConnection();

			std::string GetID() const;
			bool IsAuthorized() const;
			std::string GetAccountName() const;
			uint32 GetAccountID() const;
			int GetStatus() const;
			std::string RemoteIP() const;
			int RemotePort() const;
		private:
			std::shared_ptr<websocket_connection> GetWebsocketConnection();
			std::shared_ptr<TCPConnection> GetTCPConnection();
			void OnMessage(websocketpp::connection_hdl hdl, websocket_message_ptr msg);
			void SendResponse(const Json::Value &response, double time_elapsed);
			void SetAuthorized(bool v, const std::string account_name, uint32 account_id, int status);

			struct Impl;
			std::unique_ptr<Impl> _impl;

			friend class WebsocketServer;
		};
	}
}
