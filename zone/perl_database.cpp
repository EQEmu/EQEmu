#include "common/features.h"

#ifdef EMBPERL_XS_CLASSES

#include "zone/embperl.h"
#include "zone/perl_database.h"
#include "zone/zonedb.h"

// Perl takes ownership of returned objects allocated with new and deletes
// them via the DESTROY method when the last perl reference goes out of scope

void Perl_Database::Destroy(Perl_Database* ptr)
{
	delete ptr;
}

Perl_Database* Perl_Database::Connect()
{
	return new Perl_Database();
}

Perl_Database* Perl_Database::Connect(Connection type)
{
	return new Perl_Database(type);
}

Perl_Database* Perl_Database::Connect(Connection type, bool connect)
{
	return new Perl_Database(type, connect);
}

Perl_Database* Perl_Database::Connect(const char* host, const char* user, const char* pass, const char* db, uint32_t port)
{
	return new Perl_Database(host, user, pass, db, port);
}

Perl_MySQLPreparedStmt* Perl_Database::Prepare(std::string query)
{
	return m_db ? new Perl_MySQLPreparedStmt(m_db->Prepare(std::move(query))) : nullptr;
}

void Perl_Database::Close()
{
	m_db.reset();
}

// ---------------------------------------------------------------------------

void Perl_MySQLPreparedStmt::Destroy(Perl_MySQLPreparedStmt* ptr)
{
	delete ptr;
}

void Perl_MySQLPreparedStmt::Close()
{
	m_stmt.reset();
}

void Perl_MySQLPreparedStmt::Execute()
{
	if (m_stmt)
	{
		m_res = m_stmt->Execute();
	}
}

void Perl_MySQLPreparedStmt::Execute(perl::array args)
{
	// passes all script args as strings
	if (m_stmt)
	{
		std::vector<mysql::PreparedStmt::param_t> inputs;
		for (const perl::scalar& arg : args)
		{
			if (arg.is_null())
			{
				inputs.emplace_back(nullptr);
			}
			else
			{
				inputs.emplace_back(arg.c_str());
			}
		}
		m_res = m_stmt->Execute(inputs);
	}
}

void Perl_MySQLPreparedStmt::SetOptions(perl::hash hash)
{
	if (m_stmt)
	{
		mysql::StmtOptions opts = m_stmt->GetOptions();
		if (hash.exists("buffer_results"))
		{
			opts.buffer_results = hash["buffer_results"].as<bool>();
		}
		if (hash.exists("use_max_length"))
		{
			opts.use_max_length = hash["use_max_length"].as<bool>();
		}
		m_stmt->SetOptions(opts);
	}
}

static void PushValue(PerlInterpreter* my_perl, SV* sv, const mysql::StmtColumn& col)
{
	if (col.IsNull())
	{
		sv_setsv(sv, &PL_sv_undef);
		return;
	}

	switch (col.Type())
	{
	case MYSQL_TYPE_TINY:
	case MYSQL_TYPE_SHORT:
	case MYSQL_TYPE_INT24:
	case MYSQL_TYPE_LONG:
	case MYSQL_TYPE_LONGLONG:
	case MYSQL_TYPE_BIT:
		if (col.IsUnsigned())
		{
			sv_setuv(sv, col.Get<UV>().value());
		}
		else
		{
			sv_setiv(sv, col.Get<IV>().value());
		}
		break;
	case MYSQL_TYPE_FLOAT:
	case MYSQL_TYPE_DOUBLE:
		sv_setnv(sv, col.Get<NV>().value());
		break;
	case MYSQL_TYPE_TIME:
	case MYSQL_TYPE_DATE:
	case MYSQL_TYPE_DATETIME:
	case MYSQL_TYPE_TIMESTAMP:
		{
			std::string str = col.GetStr().value();
			sv_setpvn(sv, str.data(), str.size());
		}
		break;
	default: // string types, push raw buffer to avoid copy
		{
			std::string_view str = col.GetStrView().value();
			sv_setpvn(sv, str.data(), str.size());
		}
		break;
	}
}

perl::array Perl_MySQLPreparedStmt::FetchArray()
{
	auto row = m_stmt ? m_stmt->Fetch() : mysql::StmtRow();
	if (!row)
	{
		return perl::array();
	}

	// perf: bypass perlbind operator[]/push and use cache to limit SV allocs
	dTHX;
	AV* av = static_cast<AV*>(m_row_array);
	for (const mysql::StmtColumn& col : row)
	{
		SV** sv = av_fetch(av, col.Index(), true);
		PushValue(my_perl, *sv, col);
	}

	SvREFCNT_inc(av); // return a ref to our cache (no copy)
	return perl::array(std::move(av));
}

perl::reference Perl_MySQLPreparedStmt::FetchArrayRef()
{
	perl::array array = FetchArray();
	return array.size() == 0 ? perl::reference() : perl::reference(array);
}

perl::reference Perl_MySQLPreparedStmt::FetchHashRef()
{
	auto row = m_stmt ? m_stmt->Fetch() : mysql::StmtRow();
	if (!row)
	{
		return perl::reference();
	}

	// perf: bypass perlbind operator[] and use cache to limit SV allocs
	dTHX;
	HV* hv = static_cast<HV*>(m_row_hash);
	for (const mysql::StmtColumn& col : row)
	{
		SV** sv = hv_fetch(hv, col.Name().c_str(), static_cast<I32>(col.Name().size()), true);
		PushValue(my_perl, *sv, col);
	}

	SvREFCNT_inc(hv); // return a ref to our cache (no copy)
	return perl::reference(std::move(hv));
}

int Perl_MySQLPreparedStmt::ColumnCount()
{
	return m_res.ColumnCount();
}

uint64_t Perl_MySQLPreparedStmt::LastInsertID()
{
	return m_res.LastInsertID();
}

uint64_t Perl_MySQLPreparedStmt::RowCount()
{
	return m_res.RowCount();
}

uint64_t Perl_MySQLPreparedStmt::RowsAffected()
{
	return m_res.RowsAffected();
}

void perl_register_database()
{
	perl::interpreter perl(PERL_GET_THX);

	{
		auto package = perl.new_class<Perl_Database>("Database");
		package.add_const("Default", static_cast<int>(QuestDB::Connection::Default));
		package.add_const("Content", static_cast<int>(QuestDB::Connection::Content));
		package.add("DESTROY", &Perl_Database::Destroy);
		package.add("new", static_cast<Perl_Database*(*)()>(&Perl_Database::Connect));
		package.add("new", static_cast<Perl_Database*(*)(QuestDB::Connection)>(&Perl_Database::Connect));
		package.add("new", static_cast<Perl_Database*(*)(QuestDB::Connection, bool)>(&Perl_Database::Connect));
		package.add("new", static_cast<Perl_Database*(*)(const char*, const char*, const char*, const char*, uint32_t)>(&Perl_Database::Connect));
		package.add("close", &Perl_Database::Close);
		package.add("prepare", &Perl_Database::Prepare);
	}

	{
		auto package = perl.new_class<Perl_MySQLPreparedStmt>("MySQLPreparedStmt");
		package.add("DESTROY", &Perl_MySQLPreparedStmt::Destroy);
		package.add("close", &Perl_MySQLPreparedStmt::Close);
		package.add("execute", static_cast<void(Perl_MySQLPreparedStmt::*)()>(&Perl_MySQLPreparedStmt::Execute));
		package.add("execute", static_cast<void(Perl_MySQLPreparedStmt::*)(perl::array)>(&Perl_MySQLPreparedStmt::Execute));
		package.add("fetch", &Perl_MySQLPreparedStmt::FetchArray);
		package.add("fetch_array", &Perl_MySQLPreparedStmt::FetchArray);
		package.add("fetch_arrayref", &Perl_MySQLPreparedStmt::FetchArrayRef);
		package.add("fetch_hashref", &Perl_MySQLPreparedStmt::FetchHashRef);
		package.add("insert_id", &Perl_MySQLPreparedStmt::LastInsertID);
		package.add("num_fields", &Perl_MySQLPreparedStmt::ColumnCount);
		package.add("num_rows", &Perl_MySQLPreparedStmt::RowCount);
		package.add("rows_affected", &Perl_MySQLPreparedStmt::RowsAffected);
		package.add("set_options", &Perl_MySQLPreparedStmt::SetOptions);
	}
}

#endif // EMBPERL_XS_CLASSES
