#include "console_server_connection.h"

#include "common/util/uuid.h"
#include "common/net/console_server.h"
#include "common/net/packet.h"
#include "common/servertalk.h"
#include "common/rulesys.h"

#include "fmt/format.h"

EQ::Net::ConsoleServerConnection::ConsoleServerConnection(ConsoleServer *parent, std::shared_ptr<TCPConnection> connection)
{
	m_parent = parent;
	m_connection = connection;
	m_uuid = EQ::Util::UUID::Generate().ToString();
	m_cursor = 0;
	memset(m_line, 0, MaxConsoleLineLength);
	m_accept_messages = false;
	m_user_id = 0;
	m_admin = AccountStatus::Player;

	m_connection->OnRead(std::bind(&ConsoleServerConnection::OnRead, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	m_connection->OnDisconnect(std::bind(&ConsoleServerConnection::OnDisconnect, this, std::placeholders::_1));
	m_connection->Start();
	ClearBuffer();
	
	auto addr = m_connection->RemoteIP();

	SendLine(fmt::format("Establishing connection from: {0}:{1}", addr, m_connection->RemotePort()));

	if (addr.find("127.0.0.1") != std::string::npos || addr.find("::0") != std::string::npos) {
		SendLine("Connection established from localhost, assuming admin");
		m_status = ConsoleStatusLoggedIn;
		m_admin = AccountStatus::Max;
		SendPrompt();
	}
	else {
		m_status = ConsoleStatusWaitingForLogin;
		Send("Username: ");
	}
}

EQ::Net::ConsoleServerConnection::~ConsoleServerConnection()
{
}

void EQ::Net::ConsoleServerConnection::SendClear()
{
	EQ::Net::DynamicPacket clear;
	clear.PutUInt8(0, 0);
	m_connection->Write((const char*)clear.Data(), clear.Length());
}

void EQ::Net::ConsoleServerConnection::Send(const std::string &msg)
{
	m_connection->Write(msg.c_str(), msg.length());
}

void EQ::Net::ConsoleServerConnection::SendLine(const std::string &line)
{
	Send(line);
	SendNewLine();
}

void EQ::Net::ConsoleServerConnection::SendNewLine()
{
	EQ::Net::DynamicPacket newline;
	newline.PutUInt8(0, 10);
	newline.PutUInt8(1, 13);
	m_connection->Write((const char*)newline.Data(), newline.Length());
}

void EQ::Net::ConsoleServerConnection::QueueMessage(const std::string &msg)
{
	if (!m_accept_messages) {
		return;
	}

	if (m_cursor == 0) {
		size_t len = m_user.length() + 2;
		for (size_t i = 0; i < len; ++i) {
			Send("\x08");
		}

		SendLine(msg);
		SendPrompt();
	}
	else {
		std::string cmd(m_line, m_line + m_cursor);
		
		size_t len = m_user.length() + 2 + cmd.length();
		for (size_t i = 0; i < len; ++i) {
			Send("\x08");
		}
		
		if (msg.length() < cmd.length()) {
			Send(msg);
			size_t blank_spaces = 2 + cmd.length() - msg.length();

			for (size_t i = 0; i < blank_spaces; ++i) {
				Send(" ");
			}

			SendNewLine();
		}
		else {
			SendLine(msg);
		}

		SendPrompt();
		Send(cmd);
	}
}

bool EQ::Net::ConsoleServerConnection::SendChannelMessage(const ServerChannelMessage_Struct* scm, std::function<void(void)> onTell) {
	if (!m_accept_messages) {
		return false;
	}

	switch (scm->chan_num) {
		case ChatChannel_Guild: {
			QueueMessage(fmt::format("{} tells the guild [{}], '{}'", scm->from, scm->guilddbid, scm->message));
			break;
		}
		case ChatChannel_Auction: {
			if (RuleB(Chat, ServerWideAuction)) {
				QueueMessage(fmt::format("{} auctions, '{}'", scm->from, scm->message));
				break;
			} else { // I think we want default action in this case?
				return false;
			}
		}
		case ChatChannel_OOC: {
			if (RuleB(Chat, ServerWideOOC)) {
				QueueMessage(fmt::format("{} says ooc, '{}'", scm->from, scm->message));
				break;
			} else { // I think we want default action in this case?
				return false;
			}
		}
		case ChatChannel_Broadcast: {
			QueueMessage(fmt::format("{} BROADCASTS, '{}'", scm->from, scm->message));
			break;
		}
		case ChatChannel_Tell: {
			QueueMessage(fmt::format("[{}] tells {}, '{}'", scm->from, scm->to, scm->message));
			if (onTell) {
				onTell();
			}

			break;
		}
		case ChatChannel_GMSAY: {
			QueueMessage(fmt::format("{} GMSAYS, '{}'", scm->from, scm->message));
			break;
		}
		default: {
			return false;
		}
	}

	return true;
}

void EQ::Net::ConsoleServerConnection::OnRead(TCPConnection *c, const unsigned char *data, size_t sz)
{
	for (size_t i = 0; i < sz; ++i) {
		unsigned char c = data[i];

		switch (c) {
		case 0:
			m_cursor = 0;
			break;
		case 10: // \n
		{
			if (m_cursor > 0) {
				std::string cmd(m_line, m_line + m_cursor);
				ProcessCommand(cmd);
				m_cursor = 0;
			}
			else {
				ProcessCommand("");
			}
		}
			break;
		case 13: // \r
			break;
		case 8:
			if (m_cursor > 0) {
				m_cursor--;
			}
			break;
		case 255:
			//255 is always followed by a character
			i++;
			if (i < sz) {
				unsigned char c = data[i];

				if (c == 255) {
					//Escaped 255
					if (m_cursor < MaxConsoleLineLength && isprint(c)) {
						m_line[m_cursor] = c;
						m_cursor++;
					}

					break;
				}

				if (c == 251 || c == 252 || c == 253 || c == 254) {
					//Option code is always followed by an extra character

					//We don't really care about negotiation tho.
					i++;
				}
			}


			break;
		default:
			if (m_cursor < MaxConsoleLineLength && isprint(c)) {
				m_line[m_cursor] = c;
				m_cursor++;
			}

			break;
		}
	}
}

void EQ::Net::ConsoleServerConnection::OnDisconnect(TCPConnection *c)
{
	m_parent->ConnectionDisconnected(this);
}

void EQ::Net::ConsoleServerConnection::ProcessCommand(const std::string &cmd)
{
	if (m_status == ConsoleStatusWaitingForLogin) {
		m_user = cmd;
		m_status = ConsoleStatusWaitingForPassword;
		Send("Password: ");
		return;
	}

	if (m_status == ConsoleStatusWaitingForPassword) {
		auto status = m_parent->m_login(m_user, cmd);
		if (status.account_id == 0) {
			m_status = ConsoleStatusFailedLogin;
			SendLine("Access denied");
			m_connection->Disconnect();
			return;
		}

		if (status.status < ConsoleLoginStatus) {
			m_status = ConsoleStatusFailedLogin;
			SendLine("Access denied");
			m_connection->Disconnect();
			return;
		}

		m_user = status.account_name;
		m_user_id = status.account_id;
		m_admin = status.status;
		m_status = ConsoleStatusLoggedIn;
		SendPrompt();
		return;
	}

	if (m_status == ConsoleStatusLoggedIn) {
		m_parent->ProcessCommand(this, cmd);
	}
}

void EQ::Net::ConsoleServerConnection::SendPrompt()
{
	Send(fmt::format("{0}> ", m_user));
}
