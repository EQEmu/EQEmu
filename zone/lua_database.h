#pragma once

#ifdef LUA_EQEMU

#include "common/mysql_stmt.h"
#include "zone/quest_db.h"

#include "luabind/object.hpp"

namespace luabind { struct scope; }
luabind::scope lua_register_database();

class Lua_MySQLPreparedStmt;

class Lua_Database : public QuestDB
{
public:
	using QuestDB::QuestDB;

	void Close();
	Lua_MySQLPreparedStmt* Prepare(lua_State*, std::string query);
};

class Lua_MySQLPreparedStmt
{
public:
	Lua_MySQLPreparedStmt(lua_State* L, mysql::PreparedStmt&& stmt)
		: m_stmt(std::make_unique<mysql::PreparedStmt>(std::move(stmt)))
		, m_row_array(luabind::newtable(L))
		, m_row_hash(luabind::newtable(L)) {}

	void Close();
	void Execute(lua_State*);
	void Execute(lua_State*, luabind::object args);
	void SetOptions(luabind::object table_opts);
	luabind::object FetchArray(lua_State*);
	luabind::object FetchHash(lua_State*);

	// StmtResult functions accessible through this class to simplify api
	int ColumnCount();
	uint64_t LastInsertID();
	uint64_t RowCount();
	uint64_t RowsAffected();

private:
	std::unique_ptr<mysql::PreparedStmt> m_stmt;
	mysql::StmtResult m_res = {};
	luabind::object m_row_array; // perf: table cache for fetches
	luabind::object m_row_hash;
};

#endif // LUA_EQEMU
