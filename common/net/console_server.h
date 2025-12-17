#pragma once

#include "console_server_connection.h"

#include <functional>
#include <vector>

namespace EQ
{
	namespace Net
	{
		struct ConsoleLoginStatus
		{
			int status;
			int account_id;
			std::string account_name;
		};

		class ConsoleServer
		{
		public:
			typedef std::function<void(ConsoleServerConnection*, const std::string&, const std::vector<std::string>&)> ConsoleServerCallback;
			typedef std::function<struct ConsoleLoginStatus(const std::string&, const std::string&)> ConsoleServerLoginCallback;
			ConsoleServer(const std::string &addr, int port);
			~ConsoleServer();

			void RegisterCall(const std::string& command, int status_required, const std::string& help_definition, ConsoleServerCallback fn);
			void RegisterLogin(ConsoleServerLoginCallback fn);
			ConsoleServerConnection *FindByAccountName(const std::string &acct_name);
			void SendChannelMessage(const ServerChannelMessage_Struct* scm, std::function<void(void)> onTell);
		private:
			void ConnectionDisconnected(ConsoleServerConnection *c);
			void ProcessCommand(ConsoleServerConnection *c, const std::string& cmd);

			std::unique_ptr<TCPServer> m_server;

			std::map<std::string, std::unique_ptr<ConsoleServerConnection>> m_connections;

			struct ConsoleServerCommand
			{
				ConsoleServerCallback fn;
				int status_required;
				std::string help_definition;
			};

			std::map<std::string, ConsoleServerCommand> m_commands;
			ConsoleServerLoginCallback m_login;
			friend class ConsoleServerConnection;
		};
	}
}
