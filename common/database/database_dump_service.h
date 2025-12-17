/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
*/

#pragma once

#include <string>

class DatabaseDumpService {
public:
	void DatabaseDump();
	bool IsDumpAllTables() const;
	void SetDumpAllTables(bool dump_all_tables);
	bool IsDumpWithNoData() const;
	void SetDumpWithNoData(bool dump_with_no_data);
	bool IsDumpSystemTables() const;
	void SetDumpSystemTables(bool dump_system_tables);
	bool IsDumpContentTables() const;
	void SetDumpContentTables(bool dump_content_tables);
	bool IsDumpPlayerTables() const;
	void SetDumpPlayerTables(bool dump_player_tables);
	bool IsDumpLoginServerTables() const;
	void SetDumpLoginServerTables(bool dump_login_server_tables);
	bool IsDumpTableLock() const;
	void SetDumpTableLock(bool dump_table_lock);
	bool IsDumpWithCompression() const;
	void SetDumpWithCompression(bool dump_with_compression);
	const std::string &GetDumpPath() const;
	void SetDumpPath(const std::string &dump_path);
	const std::string &GetDumpFileName() const;
	void SetDumpFileName(const std::string &dump_file_name);
	bool IsDumpOutputToConsole() const;
	void SetDumpOutputToConsole(bool dump_output_to_console);
	bool IsDumpDropTableSyntaxOnly() const;
	void SetDumpDropTableSyntaxOnly(bool dump_drop_table_syntax_only);
	bool IsDumpStateTables() const;
	void SetDumpStateTables(bool dump_state_tables);
	bool IsDumpBotTables() const;
	void SetDumpBotTables(bool dump_bot_tables);
	bool IsDumpMercTables() const;
	void SetDumpMercTables(bool dump_bot_tables);

	void SetDumpStaticInstanceData(bool b);
	bool IsDumpStaticInstanceData();

private:
	bool        dump_all_tables             = false;
	bool        dump_state_tables           = false;
	bool        dump_system_tables          = false;
	bool        dump_content_tables         = false;
	bool        dump_player_tables          = false;
	bool        dump_login_server_tables    = false;
	bool        dump_with_no_data           = false;
	bool        dump_table_lock             = false;
	bool        dump_with_compression       = false;
	bool        dump_output_to_console      = false;
	bool        dump_drop_table_syntax_only = false;
	bool        dump_bot_tables             = false;
	bool        dump_merc_tables            = false;
	bool        dump_static_instance_data   = false;

	std::string dump_path;
	std::string dump_file_name;

	bool IsMySQLInstalled();
	std::string GetMySQLVersion();
	std::string GetBaseMySQLDumpCommand();
	std::string GetPlayerTablesList();
	std::string GetBotTablesList();
	std::string GetMercTablesList();
	std::string GetSystemTablesList();
	std::string GetStateTablesList();
	std::string GetContentTablesList();
	std::string GetLoginTableList();
	bool IsTarAvailable();
	bool Is7ZipAvailable();
	bool HasCompressionBinary();
	std::string GetDumpFileNameWithPath();
	std::string GetSetDumpPath();
	void RemoveSqlBackup();
	void BuildCredentialsFile();
	void RemoveCredentialsFile();
};
