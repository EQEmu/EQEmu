#include "loginserver_webserver.h"

#include "common/json/json.h"
#include "common/repositories/login_api_tokens_repository.h"
#include "common/strings.h"
#include "loginserver/account_management.h"
#include "loginserver/login_server.h"
#include "loginserver/world_server_manager.h"

extern LoginServer server;

namespace LoginserverWebserver {

	constexpr static int HTTP_RESPONSE_OK           = 200;
	constexpr static int HTTP_RESPONSE_BAD_REQUEST  = 400;
	constexpr static int HTTP_RESPONSE_UNAUTHORIZED = 401;

	void RegisterRoutes(httplib::Server &api)
	{
		server.token_manager = new LoginserverWebserver::TokenManager;
		server.token_manager->LoadApiTokens();

		api.Get(
			"/v1/servers/list", [](const httplib::Request &request, httplib::Response &res) {
				if (!LoginserverWebserver::TokenManager::AuthCanRead(request, res)) {
					return;
				}

				Json::Value     response;
				auto            iter = server.server_manager->GetWorldServers().begin();
				for (const auto &s: server.server_manager->GetWorldServers()) {
					Json::Value row;
					row["server_long_name"]    = s->GetServerLongName();
					row["server_short_name"]   = s->GetServerShortName();
					row["server_list_type_id"] = s->GetServerListID();
					row["server_status"]       = s->GetStatus();
					row["zones_booted"]        = s->GetZonesBooted();
					row["local_ip"]            = s->GetLocalIP();
					row["remote_ip"]           = s->GetRemoteIP();
					row["players_online"]      = s->GetPlayersOnline();
					row["world_id"]            = s->GetServerId();

					response.append(row);
				}

				LoginserverWebserver::SendResponse(response, res);
			}
		);

		api.Post(
			"/v1/account/create", [](const httplib::Request &request, httplib::Response &res) {
				if (!LoginserverWebserver::TokenManager::AuthCanWrite(request, res)) {
					return;
				}

				Json::Value req      = LoginserverWebserver::ParseRequestBody(request);
				std::string username = req.get("username", "").asString();
				std::string password = req.get("password", "").asString();
				std::string email    = req.get("email", "").asString();

				Json::Value response;
				if (username.empty() || password.empty()) {
					response["error"] = "Username or password not set";
					res.status = HTTP_RESPONSE_BAD_REQUEST;
					LoginserverWebserver::SendResponse(response, res);
					return;
				}

				LoginAccountContext c;
				c.username = username;
				c.password = password;
				c.email    = email;

				int32 account_created_id = AccountManagement::CreateLoginServerAccount(c);
				if (account_created_id > 0) {
					response["message"]            = "Account created successfully!";
					response["data"]["account_id"] = account_created_id;
				}
				else if (account_created_id == -1) {
					res.status = HTTP_RESPONSE_BAD_REQUEST;
					response["error"] = "Account already exists!";
				}
				else {
					res.status = HTTP_RESPONSE_BAD_REQUEST;
					response["error"] = "Account failed to create!";
				}

				LoginserverWebserver::SendResponse(response, res);
			}
		);

		api.Post(
			"/v1/account/create/external", [](const httplib::Request &request, httplib::Response &res) {
				if (!LoginserverWebserver::TokenManager::AuthCanWrite(request, res)) {
					return;
				}

				Json::Value req              = LoginserverWebserver::ParseRequestBody(request);
				std::string username         = req.get("username", "").asString();
				std::string password         = req.get("password", "").asString();
				std::string email            = req.get("email", "").asString();
				uint32      login_account_id = req.get("login_account_id", "").asInt();

				Json::Value response;
				if (username.empty() || password.empty()) {
					res.status = HTTP_RESPONSE_BAD_REQUEST;
					response["error"] = "Username or password not set";
					LoginserverWebserver::SendResponse(response, res);
					return;
				}

				LoginAccountContext c;
				c.username           = username;
				c.password           = password;
				c.email              = email;
				c.source_loginserver = "eqemu";
				c.login_account_id   = login_account_id;

				int32 account_created_id = AccountManagement::CreateLoginServerAccount(c);

				if (account_created_id > 0) {
					response["message"]            = "Account created successfully!";
					response["data"]["account_id"] = account_created_id;
				}
				else if (account_created_id == -1) {
					res.status = HTTP_RESPONSE_BAD_REQUEST;
					response["error"] = "Account already exists!";
				}
				else {
					res.status = HTTP_RESPONSE_BAD_REQUEST;
					response["error"] = "Account failed to create!";
				}

				LoginserverWebserver::SendResponse(response, res);
			}
		);

		api.Post(
			"/v1/account/credentials/validate/local", [](const httplib::Request &request, httplib::Response &res) {
				if (!LoginserverWebserver::TokenManager::AuthCanRead(request, res)) {
					return;
				}

				Json::Value req      = LoginserverWebserver::ParseRequestBody(request);
				std::string username = req.get("username", "").asString();
				std::string password = req.get("password", "").asString();

				Json::Value response;
				if (username.empty() || password.empty()) {
					res.status = HTTP_RESPONSE_BAD_REQUEST;
					response["error"] = "Username or password not set";
					LoginserverWebserver::SendResponse(response, res);
					return;
				}

				LoginAccountContext c;
				c.username = username;
				c.password = password;

				uint32 login_account_id = AccountManagement::CheckLoginserverUserCredentials(c);
				if (login_account_id > 0) {
					response["message"]            = "Credentials valid!";
					response["data"]["account_id"] = login_account_id;
				}
				else {
					res.status = HTTP_RESPONSE_BAD_REQUEST;
					response["error"] = "Credentials invalid!";
				}

				LoginserverWebserver::SendResponse(response, res);
			}
		);

		api.Post(
			"/v1/account/credentials/update/local", [](const httplib::Request &request, httplib::Response &res) {
				if (!LoginserverWebserver::TokenManager::AuthCanWrite(request, res)) {
					return;
				}

				Json::Value req      = LoginserverWebserver::ParseRequestBody(request);
				std::string username = req.get("username", "").asString();
				std::string password = req.get("password", "").asString();

				Json::Value response;
				if (username.empty() || password.empty()) {
					res.status = HTTP_RESPONSE_BAD_REQUEST;
					response["error"] = "Username or password not set";
					LoginserverWebserver::SendResponse(response, res);
					return;
				}

				LoginAccountContext c;
				c.username = username;
				c.password = password;

				auto a = LoginAccountsRepository::GetAccountFromContext(database, c);
				if (!a.id) {
					res.status = HTTP_RESPONSE_BAD_REQUEST;
					response["error"] = "Failed to find associated loginserver account!";
					LoginserverWebserver::SendResponse(response, res);
					return;
				}

				bool success = AccountManagement::UpdateLoginserverUserCredentials(c);
				if (success) {
					response["message"] = "Loginserver account credentials updated!";
				}
				else {
					res.status = HTTP_RESPONSE_BAD_REQUEST;
					response["error"] = "Failed to update loginserver account credentials!";
				}

				LoginserverWebserver::SendResponse(response, res);
			}
		);

		api.Post(
			"/v1/account/credentials/update/external", [](const httplib::Request &request, httplib::Response &res) {
				if (!LoginserverWebserver::TokenManager::AuthCanWrite(request, res)) {
					return;
				}

				Json::Value req      = LoginserverWebserver::ParseRequestBody(request);
				std::string username = req.get("username", "").asString();
				std::string password = req.get("password", "").asString();

				Json::Value response;
				if (username.empty() || password.empty()) {
					response["error"] = "Username or password not set";
					LoginserverWebserver::SendResponse(response, res);
					return;
				}

				std::string source_loginserver = "eqemu";

				LoginAccountContext c;
				c.username           = username;
				c.password           = password;
				c.source_loginserver = source_loginserver;

				auto a = LoginAccountsRepository::GetAccountFromContext(database, c);
				if (!a.id) {
					response["error"] = "Failed to find associated loginserver account!";
					LoginserverWebserver::SendResponse(response, res);
					return;
				}

				bool success = AccountManagement::UpdateLoginserverUserCredentials(c);
				if (success) {
					response["message"] = "Loginserver account credentials updated!";
				}
				else {
					response["error"] = "Failed to update loginserver account credentials!";
				}

				LoginserverWebserver::SendResponse(response, res);
			}
		);

		api.Post(
			"/v1/account/credentials/validate/external", [](const httplib::Request &request, httplib::Response &res) {
				if (!LoginserverWebserver::TokenManager::AuthCanRead(request, res)) {
					return;
				}

				Json::Value req      = LoginserverWebserver::ParseRequestBody(request);
				std::string username = req.get("username", "").asString();
				std::string password = req.get("password", "").asString();

				Json::Value response;
				if (username.empty() || password.empty()) {
					response["error"] = "Username or password not set";
					LoginserverWebserver::SendResponse(response, res);
					return;
				}

				LoginAccountContext c;
				c.username = username;
				c.password = password;
				uint32 account_id = AccountManagement::CheckExternalLoginserverUserCredentials(c);

				if (account_id > 0) {
					response["message"]            = "Credentials valid!";
					response["data"]["account_id"] = account_id;
				}
				else {
					response["error"] = "Credentials invalid!";
					res.status = HTTP_RESPONSE_BAD_REQUEST;
				}

				LoginserverWebserver::SendResponse(response, res);
			}
		);

		api.Get(
			"/probes/healthcheck", [](const httplib::Request &request, httplib::Response &res) {
				Json::Value response;
				uint32      login_response = AccountManagement::HealthCheckUserLogin();

				response["status"] = login_response;
				if (login_response == 0) {
					response["message"] = "Process unresponsive, exiting...";
					LogInfo("Probes healthcheck unresponsive, exiting...");
				}

				LoginserverWebserver::SendResponse(response, res);
				if (login_response == 0) {
					std::exit(0);
				}
			}
		);
	}

	void SendResponse(const Json::Value &payload, httplib::Response &res)
	{
		if (res.get_header_value("response_set") == "true") {
			res.set_header("response_set", "");
			return;
		}

		std::stringstream response_payload;

		if (payload.empty()) {
			Json::Value response;
			response["message"] = "There were no results found";
			response_payload << response;
			res.set_content(response_payload.str(), "application/json");
			return;
		}

		response_payload << payload;
		res.set_content(response_payload.str(), "application/json");
	}

	Json::Value ParseRequestBody(const httplib::Request &request)
	{
		Json::Value request_body;

		try {
			std::stringstream ss;
			ss.str(request.body);
			ss >> request_body;
		}
		catch (std::exception &) {
			request_body["error"] = "Payload invalid";

			return request_body;
		}

		return request_body;
	}

	bool LoginserverWebserver::TokenManager::AuthCanRead(const httplib::Request &request, httplib::Response &res)
	{
		LoginserverWebserver::TokenManager::Token
			user_token = LoginserverWebserver::TokenManager::CheckApiAuthorizationHeaders(request);

		if (!user_token.can_read) {
			Json::Value       response;
			std::stringstream response_payload;
			response["message"] = "Authorization token is either invalid or cannot read!";
			response_payload << response;

			res.status = HTTP_RESPONSE_UNAUTHORIZED;
			res.set_content(response_payload.str(), "application/json");
			res.set_header("response_set", "true");

			LogWarning(
				"AuthCanRead access failure remote_address [{}] user_agent [{}]",
				user_token.remote_address,
				user_token.user_agent
			);

			return false;
		}

		return true;
	}

	bool LoginserverWebserver::TokenManager::AuthCanWrite(const httplib::Request &request, httplib::Response &res)
	{
		LoginserverWebserver::TokenManager::Token
			user_token = LoginserverWebserver::TokenManager::CheckApiAuthorizationHeaders(request);

		if (!user_token.can_write) {
			Json::Value       response;
			std::stringstream response_payload;
			response["message"] = "Authorization token is either invalid or cannot write!";
			response_payload << response;

			res.status = HTTP_RESPONSE_UNAUTHORIZED;
			res.set_content(response_payload.str(), "application/json");
			res.set_header("response_set", "true");

			LogWarning(
				"AuthCanWrite access failure remote_address [{}] user_agent [{}]",
				user_token.remote_address,
				user_token.user_agent
			);

			return false;
		}

		return true;
	}

	LoginserverWebserver::TokenManager::Token
	LoginserverWebserver::TokenManager::CheckApiAuthorizationHeaders(
		const httplib::Request &request
	)
	{
		std::string authorization_key;

		LoginserverWebserver::TokenManager::Token user_token{};

		for (const auto &header: request.headers) {
			auto header_key   = header.first;
			auto header_value = header.second;
			if (header_key == "Authorization") {
				authorization_key = header.second;
				Strings::FindReplace(authorization_key, "Bearer ", "");
				if (LoginserverWebserver::TokenManager::TokenExists(authorization_key)) {
					user_token = server.token_manager->GetToken(authorization_key);
				}
			}

			if (header_key == "REMOTE_ADDR") {
				user_token.remote_address = header.second;
			}

			if (header_key == "User-Agent") {
				user_token.user_agent = header.second;
			}
		}

		LogDebug(
			"Authentication Request | remote_address [{}] user_agent [{}] authorization_key [{}] request_path [{}]",
			user_token.remote_address,
			user_token.user_agent,
			authorization_key,
			request.path
		);

		return user_token;
	}

	void TokenManager::LoadApiTokens()
	{
		int token_count = 0;

		for (auto &t: LoginApiTokensRepository::GetWhere(database, "TRUE ORDER BY id ASC")) {
			LoginserverWebserver::TokenManager::Token td;
			td.id        = t.id;
			td.token     = t.token;
			td.can_write = t.can_write;
			td.can_read  = t.can_read;

			server.token_manager->loaded_api_tokens.emplace(std::make_pair(td.token, td));
			token_count++;
		}

		LogInfo("Loaded [{}] API token(s)", token_count);
	}

	bool TokenManager::TokenExists(const std::string &token)
	{
		auto it = server.token_manager->loaded_api_tokens.find(token);

		return !(it == server.token_manager->loaded_api_tokens.end());
	}

	LoginserverWebserver::TokenManager::Token TokenManager::GetToken(
		const std::string &token
	)
	{
		auto iter = server.token_manager->loaded_api_tokens.find(token);
		if (iter != server.token_manager->loaded_api_tokens.end()) {
			return iter->second;
		}

		return {};
	}
}
