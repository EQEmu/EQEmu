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
#include "dbcore.h"

#include "common/eqemu_logsys.h"
#include "common/misc_functions.h"
#include "common/mysql_stmt.h"
#include "common/strings.h"
#include "common/timer.h"
#include "common/types.h"

#include "mysqld_error.h"

#ifdef _EQDEBUG
#define DEBUG_MYSQL_QUERIES 0
#else
#define DEBUG_MYSQL_QUERIES 0
#endif

DBcore::DBcore()
	: mysql(mysql_init(nullptr))
	, m_mutex(std::make_shared<Mutex>())
{
}

DBcore::~DBcore()
{
	/**
	 * This prevents us from doing a double free in multi-tenancy setups where we
	 * are re-using the default database connection pointer when we dont have an
	 * external configuration setup ex: (content_database)
	 */
	if (mysqlOwner) {
		mysql_close(mysql);
	}
}

// Sends the MySQL server a keepalive
void DBcore::ping()
{
	if (!m_mutex->try_lock())
	{
		// well, if's it's locked, someone's using it. If someone's using it, it doesnt need a keepalive
		return;
	}

	mysql_ping(mysql);
	m_mutex->unlock();
}

MySQLRequestResult DBcore::QueryDatabase(const std::string& query, bool retryOnFailureOnce)
{
	auto r = QueryDatabase(query.c_str(), query.length(), retryOnFailureOnce);
	return r;
}

bool DBcore::DoesTableExist(const std::string& table_name)
{
	auto results = QueryDatabase(fmt::format("SHOW TABLES LIKE '{}'", table_name));

	return results.RowCount() > 0;
}

MySQLRequestResult DBcore::QueryDatabase(const char *query, uint32 querylen, bool retryOnFailureOnce)
{
	BenchTimer timer;
	timer.reset();

	std::scoped_lock lock(*m_mutex);

	if (m_strict_transaction_active && m_strict_transaction_failed) {
		return {};
	}

	// Reconnect if we are not connected before hand.
	if (pStatus != Connected) {
		if (m_strict_transaction_active) {
			m_strict_transaction_failed = true;
			return {};
		}
		Open();
	}

	// request query. != 0 indicates some kind of error.
	if (mysql_real_query(mysql, query, querylen) != 0) {
		unsigned int errorNumber = mysql_errno(mysql);
		if (m_strict_transaction_active) {
			m_strict_transaction_failed = true;
			retryOnFailureOnce = false;
		}

		if (errorNumber == CR_SERVER_GONE_ERROR) {
			pStatus = Error;
		}

		// error appears to be a disconnect error, may need to try again.
		if (errorNumber == CR_SERVER_LOST || errorNumber == CR_SERVER_GONE_ERROR) {

			if (retryOnFailureOnce) {
				LogInfo("Database Error: Lost connection, attempting to recover");
				MySQLRequestResult requestResult = QueryDatabase(query, querylen, false);

				if (requestResult.Success()) {
					LogInfo("Reconnection to database successful");
					return requestResult;
				}

			}

			pStatus = Error;

			auto errorBuffer = new char[MYSQL_ERRMSG_SIZE];

			snprintf(errorBuffer, MYSQL_ERRMSG_SIZE, "#%i: %s", mysql_errno(mysql), mysql_error(mysql));

			return MySQLRequestResult(nullptr, 0, 0, 0, 0, (uint32) mysql_errno(mysql), errorBuffer);
		}

		auto errorBuffer = new char[MYSQL_ERRMSG_SIZE];
		snprintf(errorBuffer, MYSQL_ERRMSG_SIZE, "#%i: %s", mysql_errno(mysql), mysql_error(mysql));

		/**
		 * Error logging
		 */
		if (mysql_errno(mysql) > 0 && query[0] != '\0') {
			LogMySQLError("MySQL Error ({}) [{}] Query [{}]", mysql_errno(mysql), mysql_error(mysql), query);
		}

		return MySQLRequestResult(nullptr, 0, 0, 0, 0, mysql_errno(mysql), errorBuffer);
	}

	// successful query. get results.
	MYSQL_RES *res     = mysql_store_result(mysql);
	uint32    rowCount = 0;

	if (res != nullptr) {
		rowCount = (uint32) mysql_num_rows(res);
	}

	MySQLRequestResult requestResult(
		res,
		(uint32) mysql_affected_rows(mysql),
		rowCount,
		(uint32) mysql_field_count(mysql),
		(uint32) mysql_insert_id(mysql)
	);

	if (EQEmuLogSys::Instance()->log_settings[Logs::MySQLQuery].is_category_enabled == 1) {
		if ((strncasecmp(query, "select", 6) == 0)) {
			LogMySQLQuery(
				"{0} -- ({1} row{2} returned) ({3}s)",
				query,
				requestResult.RowCount(),
				requestResult.RowCount() == 1 ? "" : "s",
				std::to_string(timer.elapsed())
			);
		}
		else {
			LogMySQLQuery(
				"{0} -- ({1} row{2} affected) ({3}s)",
				query,
				requestResult.RowsAffected(),
				requestResult.RowsAffected() == 1 ? "" : "s",
				std::to_string(timer.elapsed())
			);
		}
	}

	return requestResult;
}

void DBcore::TransactionBegin()
{
	if (m_strict_transaction_active) {
		m_strict_transaction_failed = true;
		return;
	}
	QueryDatabase("START TRANSACTION");
}

MySQLRequestResult DBcore::TransactionCommit()
{
	if (m_strict_transaction_active) {
		m_strict_transaction_failed = true;
		return {};
	}
	return QueryDatabase("COMMIT");
}

void DBcore::TransactionRollback()
{
	if (m_strict_transaction_active) {
		TransactionRollbackStrict();
		return;
	}
	QueryDatabase("ROLLBACK");
}

MySQLRequestResult DBcore::TransactionBeginStrict()
{
	if (m_strict_transaction_active) {
		return {};
	}

	auto result = QueryDatabase("START TRANSACTION", 17, false);
	if (result.Success()) {
		m_strict_transaction_active = true;
		m_strict_transaction_failed = false;
	}
	return result;
}

MySQLRequestResult DBcore::TransactionCommitStrict()
{
	if (!m_strict_transaction_active) {
		return {};
	}
	if (m_strict_transaction_failed) {
		TransactionRollbackStrict();
		return {};
	}

	auto result = QueryDatabase("COMMIT", 6, false);
	m_strict_transaction_active = false;
	m_strict_transaction_failed = false;
	return result;
}

void DBcore::TransactionRollbackStrict()
{
	if (!m_strict_transaction_active) {
		return;
	}

	// A failed strict statement suppresses all later statements. Temporarily
	// clear that latch so a still-live session can receive ROLLBACK.
	m_strict_transaction_failed = false;
	QueryDatabase("ROLLBACK", 8, false);
	m_strict_transaction_active = false;
	m_strict_transaction_failed = false;
}

void DBcore::TransactionFailStrict()
{
	if (m_strict_transaction_active) {
		m_strict_transaction_failed = true;
	}
}

bool DBcore::TransactionStrictFailed() const
{
	return m_strict_transaction_active && m_strict_transaction_failed;
}

uint32 DBcore::DoEscapeString(char *tobuf, const char *frombuf, uint32 fromlen)
{
//	No good reason to lock the DB, we only need it in the first place to check char encoding.
//	LockMutex lock(&MDatabase);
	return mysql_real_escape_string(mysql, tobuf, frombuf, fromlen);
}

bool DBcore::Open(
	const char *iHost,
	const char *iUser,
	const char *iPassword,
	const char *iDatabase,
	uint32 iPort,
	uint32 *errnum,
	char *errbuf,
	bool iCompress,
	bool iSSL
)
{
	std::scoped_lock lock(*m_mutex);

	m_host = iHost;
	m_user = iUser;
	m_password = iPassword;
	m_database = iDatabase;
	pCompress = iCompress;
	pPort     = iPort;
	pSSL      = iSSL;
	return Open(errnum, errbuf);
}

bool DBcore::Open(uint32 *errnum, char *errbuf)
{
	// Expects m_mutex to already be locked.

	if (errbuf) {
		errbuf[0] = 0;
	}

	if (GetStatus() == Connected) {
		return true;
	}
	if (GetStatus() == Error) {
		mysql_close(mysql);
		mysql_init(mysql);        // Initialize structure again
	}
	if (m_host.empty()) {
		return false;
	}
	/*
	Added CLIENT_FOUND_ROWS flag to the connect
	otherwise DB update calls would say 0 rows affected when the value already equalled
	what the function was tring to set it to, therefore the function would think it failed
	*/
	uint32 flags = CLIENT_FOUND_ROWS;
	if (pCompress) {
		flags |= CLIENT_COMPRESS;
	}

	//todo: we need to revisit this ssl handling later
	//the whole connect code is ancient and tls is starting to come as a default requirement for many db setups
	if (pSSL) {
		flags |= CLIENT_SSL;
	}
	else {
		int off = 0;
		mysql_options(mysql, MYSQL_OPT_SSL_ENFORCE, &off);
		mysql_options(mysql, MYSQL_OPT_SSL_VERIFY_SERVER_CERT, &off);
	}
	if (mysql_real_connect(mysql, m_host.c_str(), m_user.c_str(), m_password.c_str(), m_database.c_str(), pPort, nullptr, flags)) {
		pStatus = Connected;

		SetOriginHost(m_host);

		return true;
	}
	else {
		if (errnum) {
			*errnum = mysql_errno(mysql);
		}
		if (errbuf) {
			snprintf(errbuf, MYSQL_ERRMSG_SIZE, "#%i: %s", mysql_errno(mysql), mysql_error(mysql));
		}
		pStatus = Error;
		return false;
	}
}

const std::string &DBcore::GetOriginHost() const
{
	return origin_host;
}

void DBcore::SetOriginHost(const std::string& originHost)
{
	DBcore::origin_host = originHost;
}

std::string DBcore::Escape(const std::string& s)
{
	const std::size_t s_len = s.length();
	std::vector<char> temp((s_len * 2) + 1, '\0');
	mysql_real_escape_string(mysql, temp.data(), s.c_str(), s_len);

	return temp.data();
}

void DBcore::SetMutex(const std::shared_ptr<Mutex>& mutex)
{
	DBcore::m_mutex = mutex;
}

// executes multiple statements in one query
// do not use this in application logic
// this was built and maintained for database migrations only
MySQLRequestResult DBcore::QueryDatabaseMulti(const std::string &query)
{
	SetMultiStatementsOn();

	BenchTimer timer;
	timer.reset();

	std::scoped_lock lock(*m_mutex);

	// Reconnect if we are not connected before hand.
	if (pStatus != Connected) {
		Open();
	}
	auto r = MySQLRequestResult{};

	int status = mysql_real_query(mysql, query.c_str(), query.length());

	// A failed multi-statement query has no usable result set to drain.
	if (status != 0) {
		const auto error_number = mysql_errno(mysql);
		const std::string error_message = mysql_error(mysql);

		if (error_number == CR_SERVER_GONE_ERROR) {
			pStatus = Error;
		}

		// error logging
		if (error_number > 0 && !query.empty() && error_number != 1065) {
			std::string error_raw   = fmt::format("{}", error_message);
			std::string mysql_err   = Strings::Trim(error_raw);
			std::string clean_query = Strings::Replace(query, "\n", "");
			LogMySQLError("[{}] ({}) query [{}]", mysql_err, error_number, clean_query);
		}

		r = MySQLRequestResult(
			nullptr,
			(uint32) mysql_affected_rows(mysql),
			0,
			(uint32) mysql_field_count(mysql),
			(uint32) mysql_insert_id(mysql)
		);
		r.SetErrorMessage(error_message);
		r.SetErrorNumber(error_number);

		SetMultiStatementsOff();
		return r;
	}

	int index = 0;

	// there could be a query with a semicolon in the actual data, this is best effort for
	// logging / display purposes
	// rare that we see this when this is only used in DDL statements
	auto pieces = Strings::Split(query, ";");

	// process each statement result
	do {
		MYSQL_RES *res = mysql_store_result(mysql);
		const uint32 row_count = res ? (uint32) mysql_num_rows(res) : 0;

		// QueryDatabaseMulti callers consume status and metadata only. Keep the
		// wrapper non-owning so the current result can be freed exactly once
		// before mysql_next_result advances the multi-statement protocol.
		r = MySQLRequestResult(
			nullptr,
			(uint32) mysql_affected_rows(mysql),
			row_count,
			(uint32) mysql_field_count(mysql),
			(uint32) mysql_insert_id(mysql)
		);

		if (index < pieces.size()) {
			auto piece = pieces[index];
			LogMySQLQuery(
				"{} -- ({} row{} affected) ({}s)",
				piece,
				r.RowsAffected(),
				r.RowsAffected() == 1 ? "" : "s",
				std::to_string(timer.elapsed())
			);
		}

		if (res) {
			mysql_free_result(res);
			res = nullptr;
		}

		// more results? -1 = no, >0 = error, 0 = yes (keep looping)
		if ((status = mysql_next_result(mysql)) > 0) {
			const auto error_number = mysql_errno(mysql);
			const std::string error_message = mysql_error(mysql);
			if (error_number > 0) {
				LogMySQLError("[{}] [{}]", error_number, error_message);
			}

			// error logging
			r.SetErrorMessage(error_message);
			r.SetErrorNumber(error_number);

			SetMultiStatementsOff();

			// we handle errors elsewhere
			return r;
		}

		index++;
	} while (status == 0);

	SetMultiStatementsOff();

	return r;
}

mysql::PreparedStmt DBcore::Prepare(std::string query)
{
	return mysql::PreparedStmt(*mysql, std::move(query), *m_mutex);
}
