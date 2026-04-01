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

#include "common/mutex.h"
#include "common/mysql_request_result.h"
#include "common/types.h"

#include "mysql.h"
#include <mutex>

#define CR_SERVER_GONE_ERROR    2006
#define CR_SERVER_LOST          2013

namespace mysql { class PreparedStmt; }

class DBcore {
public:
	enum eStatus {
		Closed, Connected, Error
	};

	DBcore();
	~DBcore();
	eStatus GetStatus() { return pStatus; }
	MySQLRequestResult QueryDatabase(const char *query, uint32 querylen, bool retryOnFailureOnce = true);
	MySQLRequestResult QueryDatabase(const std::string& query, bool retryOnFailureOnce = true);
	MySQLRequestResult QueryDatabaseMulti(const std::string &query);
	void TransactionBegin();
	MySQLRequestResult TransactionCommit();
	void TransactionRollback();
	std::string Escape(const std::string& s);
	uint32 DoEscapeString(char *tobuf, const char *frombuf, uint32 fromlen);
	void ping();

	const std::string &GetOriginHost() const;
	void SetOriginHost(const std::string &origin_host);

	bool DoesTableExist(const std::string& table_name);

	void SetMySQL(const DBcore &o)
	{
		mysql      = o.mysql;
		mysqlOwner = false;
	}
	void SetMutex(Mutex *mutex);

	// only safe on connections shared with other threads if results buffered
	// unsafe to use off main thread due to internal server logging
	// throws std::runtime_error on failure
	mysql::PreparedStmt Prepare(std::string query);

protected:
	bool Open(
		const char *iHost,
		const char *iUser,
		const char *iPassword,
		const char *iDatabase,
		uint32 iPort,
		uint32 *errnum = 0,
		char *errbuf = 0,
		bool iCompress = false,
		bool iSSL = false
	);

private:
	bool Open(uint32 *errnum = nullptr, char *errbuf = nullptr);

	MYSQL*  mysql;
	bool    mysqlOwner;
	Mutex   *m_mutex;
	eStatus pStatus;

	std::mutex m_query_lock{};

	std::string origin_host;

	char   *pHost;
	char   *pUser;
	char   *pPassword;
	char   *pDatabase;
	bool   pCompress;
	uint32 pPort;
	bool   pSSL;

	// allows multiple queries to be executed within the same query
	// do not use this under normal operation
	// we use this during database migrations only currently
	void SetMultiStatementsOn()
	{
		mysql_set_server_option(mysql, MYSQL_OPTION_MULTI_STATEMENTS_ON);
	}

	// disables multiple statements to be executed in one query
	void SetMultiStatementsOff()
	{
		mysql_set_server_option(mysql, MYSQL_OPTION_MULTI_STATEMENTS_OFF);
	}
};
