#include "world_console_connection.h"

#include <cstring>
#include <cstdarg>

void WorldConsoleTCPConnection::SendEmoteMessage(const char *to, uint32 to_guilddbid, int16 to_minstatus, uint32 type, const char *message, ...)
{
	va_list argptr;
	char buffer[1024];

	va_start(argptr, message);
	vsnprintf(buffer, sizeof(buffer), message, argptr);
	va_end(argptr);
	SendEmoteMessageRaw(to, to_guilddbid, to_minstatus, type, buffer);
}

void WorldConsoleTCPConnection::SendEmoteMessageRaw(const char *to, uint32 to_guilddbid, int16 to_minstatus, uint32 type, const char *message)
{
	m_connection->SendLine(message);
}
