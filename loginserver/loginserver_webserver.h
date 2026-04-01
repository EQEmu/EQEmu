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

#include "common/http/httplib.h"
#include "common/json/json.h"

namespace LoginserverWebserver {

	class TokenManager {

	public:
		TokenManager() = default;

		struct Token {
			int id;
			std::string token;
			bool can_read;
			bool can_write;
			std::string user_agent;
			std::string remote_address;
		};

		std::map<std::string, Token> loaded_api_tokens{};

		void LoadApiTokens();
		static bool TokenExists(const std::string &token);
		Token GetToken(const std::string &token);
		static Token CheckApiAuthorizationHeaders(const httplib::Request &request);
		static bool AuthCanRead(const httplib::Request &request, httplib::Response &res);
		static bool AuthCanWrite(const httplib::Request &request, httplib::Response &res);
	};

	void RegisterRoutes(httplib::Server &api);
	void SendResponse(const Json::Value &payload, httplib::Response &res);
	static Json::Value ParseRequestBody(const httplib::Request &request);
};
