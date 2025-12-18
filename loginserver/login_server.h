#pragma once

#include "common/json_config.h"
#include "loginserver/client_manager.h"
#include "loginserver/loginserver_webserver.h"
#include "loginserver/options.h"
#include "loginserver/world_server_manager.h"

struct LoginServer {
public:

	LoginServer() : server_manager(nullptr)
	{

	}

	EQ::JsonConfigFile                 config;
	LoginserverWebserver::TokenManager *token_manager{};
	Options                            options;
	WorldServerManager                 *server_manager;
	ClientManager                      *client_manager{};
};
