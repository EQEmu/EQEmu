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

#include "common/discord/discord.h"
#include "common/types.h"
#include "world/ucs.h"

#include <string>

class WorldBoot {
public:
	static void GMSayHookCallBackProcessWorld(uint16 log_category, const char *func, std::string message);
	static bool HandleCommandInput(int argc, char **argv);
	static bool LoadServerConfig();
	static bool LoadDatabaseConnections();
	static void RegisterLoginservers();
	static bool DatabaseLoadRoutines(int argc, char **argv);
	static void CheckForPossibleConfigurationIssues();
	static void Shutdown();
	static void SendDiscordMessage(int webhook_id, const std::string& message);
	static void DiscordWebhookMessageHandler(uint16 log_category, int webhook_id, const std::string &message)
	{
		std::string message_prefix = fmt::format(
			"[**{}**] **World** ",
			Logs::LogCategoryName[log_category]
		);

		SendDiscordMessage(webhook_id, message_prefix + Discord::FormatDiscordMessage(log_category, message));
	};
};
