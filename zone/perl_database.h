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

#include "common/mysql_stmt.h"
#include "zone/quest_db.h"

class Perl_MySQLPreparedStmt;

class Perl_Database : public QuestDB
{
public:
	using QuestDB::QuestDB;

	static void Destroy(Perl_Database* ptr);
	static Perl_Database* Connect();
	static Perl_Database* Connect(Connection type);
	static Perl_Database* Connect(Connection type, bool connect);
	static Perl_Database* Connect(const char* host, const char* user, const char* pass, const char* db, uint32_t port);

	void Close();
	Perl_MySQLPreparedStmt* Prepare(std::string query);
};

class Perl_MySQLPreparedStmt
{
public:
	Perl_MySQLPreparedStmt(mysql::PreparedStmt&& stmt)
		: m_stmt(std::make_unique<mysql::PreparedStmt>(std::move(stmt))) {}

	static void Destroy(Perl_MySQLPreparedStmt* ptr);

	void Close();
	void Execute();
	void Execute(perl::array args);
	void SetOptions(perl::hash hash_opts);
	perl::array FetchArray();
	perl::reference FetchArrayRef();
	perl::reference FetchHashRef();

	// StmtResult functions accessible through this class to simplify api
	int ColumnCount();
	uint64_t LastInsertID();
	uint64_t RowCount();
	uint64_t RowsAffected();

private:
	std::unique_ptr<mysql::PreparedStmt> m_stmt;
	mysql::StmtResult m_res = {};
	perl::array m_row_array; // perf: cache for fetches
	perl::hash  m_row_hash;
};
