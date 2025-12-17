#pragma once

#include "tcp_server.h"

#include <map>
#include <memory>

struct ServerChannelMessage_Struct;

namespace EQ
{
	namespace Net
	{
		enum ConsoleConnectionStatus
		{
			ConsoleStatusWaitingForLogin,
			ConsoleStatusWaitingForPassword,
			ConsoleStatusLoggedIn,
			ConsoleStatusFailedLogin
		};

		const int MaxConsoleLineLength = 512;
		const int ConsoleLoginStatus = 50;
		class ConsoleServer;
		class ConsoleServerConnection
		{
		public:
			ConsoleServerConnection(ConsoleServer *parent, std::shared_ptr<TCPConnection> connection);
			~ConsoleServerConnection();

			std::string GetUUID() const { return m_uuid; }
			void ClearBuffer() { m_cursor = 0; }
			void Close() { m_connection->Disconnect(); }

			void SendClear();
			void Send(const std::string &msg);
			void SendLine(const std::string &line);
			void SendNewLine();
			void SendPrompt();
			ConsoleConnectionStatus Status() const { return m_status; }
			std::string UserName() const { return m_user; }
			int UserId() const { return m_user_id; }
			int Admin() const { return m_admin; }

			bool AcceptMessages() const { return m_accept_messages; }
			void SetAcceptMessages(bool v) { m_accept_messages = v; }
			void QueueMessage(const std::string &msg);
			bool SendChannelMessage(const ServerChannelMessage_Struct* scm, std::function<void(void)> onTell);
		private:
			void OnRead(TCPConnection* c, const unsigned char* data, size_t sz);
			void OnDisconnect(TCPConnection* c);
			void ProcessCommand(const std::string &cmd);

			ConsoleServer *m_parent;
			std::shared_ptr<TCPConnection> m_connection;
			std::string m_uuid;
			ConsoleConnectionStatus m_status;
			std::string m_user;
			int m_user_id;
			int m_admin;
			bool m_accept_messages;

			size_t m_cursor;
			char m_line[MaxConsoleLineLength];
		};
	}
}
