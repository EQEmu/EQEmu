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
#include "common/net/websocket_server_connection.h"

#include <exception>
#include <functional>
#include <memory>

namespace EQ
{
	namespace Net
	{
		enum WebsocketSubscriptionEvent : int
		{
			SubscriptionEventNone,
			SubscriptionEventLog,
			SubscriptionEventMax
		};

		struct WebsocketLoginStatus
		{
			bool logged_in;
			std::string account_name;
			uint32 account_id;
			int status;
		};

		class WebsocketException : public std::exception
		{
		public:
			WebsocketException(const std::string &msg)
				: _msg(msg.empty() ? "Unknown Error" : msg) { }

			~WebsocketException() noexcept {}

			virtual char const *what() const noexcept override {
				return _msg.c_str();
			}
		private:
			const std::string _msg;
		};

		class WebsocketServer
		{
		public:
			typedef std::function<Json::Value(WebsocketServerConnection*, const Json::Value&)> MethodHandler;
			typedef std::function<WebsocketLoginStatus(WebsocketServerConnection*, const std::string&, const std::string&)> LoginHandler;

			WebsocketServer(const std::string &addr, int port);
			~WebsocketServer();
			
			void SetMethodHandler(const std::string& method, MethodHandler handler, int required_status);
			void SetLoginHandler(LoginHandler handler);
			void DispatchEvent(WebsocketSubscriptionEvent evt, Json::Value data = Json::Value(), int required_status = 0);
		private:
			void ReleaseConnection(WebsocketServerConnection *connection);
			Json::Value HandleRequest(WebsocketServerConnection *connection, const std::string& method, const Json::Value &params);

			Json::Value Login(WebsocketServerConnection *connection, const Json::Value &params);
			Json::Value Subscribe(WebsocketServerConnection *connection, const Json::Value &params);
			Json::Value Unsubscribe(WebsocketServerConnection *connection, const Json::Value &params);
			void DoSubscribe(WebsocketServerConnection *connection, WebsocketSubscriptionEvent sub);
			void DoUnsubscribe(WebsocketServerConnection *connection, WebsocketSubscriptionEvent sub);
			bool IsSubscribed(WebsocketServerConnection *connection, WebsocketSubscriptionEvent sub);
			void UnsubscribeAll(WebsocketServerConnection *connection);

			struct Impl;
			std::unique_ptr<Impl> _impl;

			friend class WebsocketServerConnection;
		};
	}
}
