#ifdef LUA_EQEMU

#include "lua_database.h"

#include "zone/zonedb.h"

#include "luabind/adopt_policy.hpp"
#include "luabind/luabind.hpp"

// Luabind adopts the PreparedStmt wrapper object allocated with new and deletes it via GC
// Lua GC is non-deterministic so handles should be closed explicitly to free db resources
// Script errors/exceptions will hold resources until GC deletes the wrapper object

Lua_MySQLPreparedStmt* Lua_Database::Prepare(lua_State* L, std::string query)
{
	return m_db ? new Lua_MySQLPreparedStmt(L, m_db->Prepare(std::move(query))) : nullptr;
}

void Lua_Database::Close()
{
	m_db.reset();
}

// ---------------------------------------------------------------------------

void Lua_MySQLPreparedStmt::Close()
{
	m_stmt.reset();
}

void Lua_MySQLPreparedStmt::Execute(lua_State* L)
{
	if (m_stmt)
	{
		m_res = m_stmt->Execute();
	}
}

void Lua_MySQLPreparedStmt::Execute(lua_State* L, luabind::object args)
{
	if (m_stmt)
	{
		std::vector<mysql::PreparedStmt::param_t> inputs;

		// iterate table until nil like ipairs to guarantee traversal order
		for (int i = 1, type; (type = luabind::type(args[i])) != LUA_TNIL; ++i)
		{
			switch (type)
			{
			case LUA_TBOOLEAN:
				inputs.emplace_back(luabind::object_cast<bool>(args[i]));
				break;
			case LUA_TNUMBER: // all numbers are doubles in lua before 5.3
				inputs.emplace_back(luabind::object_cast<lua_Number>(args[i]));
				break;
			case LUA_TSTRING:
				inputs.emplace_back(luabind::object_cast<const char*>(args[i]));
				break;
			case LUA_TTABLE: // let tables substitute for null since nils can't exist
				inputs.emplace_back(nullptr);
				break;
			default:
				break;
			}
		}

		m_res = m_stmt->Execute(inputs);
	}
}

void Lua_MySQLPreparedStmt::SetOptions(luabind::object table)
{
	if (m_stmt)
	{
		mysql::StmtOptions opts = m_stmt->GetOptions();
		if (luabind::type(table["buffer_results"]) == LUA_TBOOLEAN)
		{
			opts.buffer_results = luabind::object_cast<bool>(table["buffer_results"]);
		}
		if (luabind::type(table["use_max_length"]) == LUA_TBOOLEAN)
		{
			opts.use_max_length = luabind::object_cast<bool>(table["use_max_length"]);
		}
		m_stmt->SetOptions(opts);
	}
}

static void PushValue(lua_State* L, const mysql::StmtColumn& col)
{
	if (col.IsNull())
	{
		lua_pushnil(L); // clear entry in cache from any previous row
		return;
	}

	// 64-bit ints are pushed as strings since lua 5.1 only has 53-bit precision
	switch (col.Type())
	{
	case MYSQL_TYPE_TINY:
	case MYSQL_TYPE_SHORT:
	case MYSQL_TYPE_INT24:
	case MYSQL_TYPE_LONG:
	case MYSQL_TYPE_FLOAT:
	case MYSQL_TYPE_DOUBLE:
		lua_pushnumber(L, col.Get<lua_Number>().value());
		break;
	case MYSQL_TYPE_LONGLONG:
	case MYSQL_TYPE_BIT:
	case MYSQL_TYPE_TIME:
	case MYSQL_TYPE_DATE:
	case MYSQL_TYPE_DATETIME:
	case MYSQL_TYPE_TIMESTAMP:
		{
			std::string str = col.GetStr().value();
			lua_pushlstring(L, str.data(), str.size());
		}
		break;
	default: // string types, push raw buffer to avoid copy
		{
			std::string_view str = col.GetStrView().value();
			lua_pushlstring(L, str.data(), str.size());
		}
		break;
	}
}

luabind::object Lua_MySQLPreparedStmt::FetchArray(lua_State* L)
{
	auto row = m_stmt ? m_stmt->Fetch() : mysql::StmtRow();
	if (!row)
	{
		return luabind::object();
	}

	// perf: bypass luabind operator[]
	m_row_array.push(L);
	for (const mysql::StmtColumn& col : row)
	{
		PushValue(L, col);
		lua_rawseti(L, -2, col.Index() + 1);
	}
	lua_pop(L, 1);

	return m_row_array;
}

luabind::object Lua_MySQLPreparedStmt::FetchHash(lua_State* L)
{
	auto row = m_stmt ? m_stmt->Fetch() : mysql::StmtRow();
	if (!row)
	{
		return luabind::object();
	}

	// perf: bypass luabind operator[]
	m_row_hash.push(L);
	for (const mysql::StmtColumn& col : row)
	{
		PushValue(L, col);
		lua_setfield(L, -2, col.Name().c_str());
	}
	lua_pop(L, 1);

	return m_row_hash;
}

int Lua_MySQLPreparedStmt::ColumnCount()
{
	return m_res.ColumnCount();
}

uint64_t Lua_MySQLPreparedStmt::LastInsertID()
{
	return m_res.LastInsertID();
}

uint64_t Lua_MySQLPreparedStmt::RowCount()
{
	return m_res.RowCount();
}

uint64_t Lua_MySQLPreparedStmt::RowsAffected()
{
	return m_res.RowsAffected();
}

luabind::scope lua_register_database()
{
	return luabind::class_<Lua_Database>("Database")
		.enum_("constants")
		[(
			luabind::value("Default", static_cast<int>(QuestDB::Connection::Default)),
			luabind::value("Content", static_cast<int>(QuestDB::Connection::Content))
		)]
		.def(luabind::constructor<>())
		.def(luabind::constructor<QuestDB::Connection>())
		.def(luabind::constructor<QuestDB::Connection, bool>())
		.def(luabind::constructor<const char*, const char*, const char*, const char*, uint32_t>())
		.def("close", &Lua_Database::Close)
		.def("prepare", &Lua_Database::Prepare, luabind::adopt(luabind::result)),

	luabind::class_<Lua_MySQLPreparedStmt>("MySQLPreparedStmt")
		.def("close", &Lua_MySQLPreparedStmt::Close)
		.def("execute", static_cast<void(Lua_MySQLPreparedStmt::*)(lua_State*)>(&Lua_MySQLPreparedStmt::Execute))
		.def("execute", static_cast<void(Lua_MySQLPreparedStmt::*)(lua_State*, luabind::object)>(&Lua_MySQLPreparedStmt::Execute))
		.def("fetch", &Lua_MySQLPreparedStmt::FetchArray)
		.def("fetch_array", &Lua_MySQLPreparedStmt::FetchArray)
		.def("fetch_hash", &Lua_MySQLPreparedStmt::FetchHash)
		.def("insert_id", &Lua_MySQLPreparedStmt::LastInsertID)
		.def("num_fields", &Lua_MySQLPreparedStmt::ColumnCount)
		.def("num_rows", &Lua_MySQLPreparedStmt::RowCount)
		.def("rows_affected", &Lua_MySQLPreparedStmt::RowsAffected)
		.def("set_options", &Lua_MySQLPreparedStmt::SetOptions);
}

#endif // LUA_EQEMU
