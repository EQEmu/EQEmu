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
