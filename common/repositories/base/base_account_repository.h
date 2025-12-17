/**
 * DO NOT MODIFY THIS FILE
 *
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to the repository extending the base.
 * Any modifications to base repositories are to be made by the generator only
 *
 * @generator ./utils/scripts/generators/repository-generator.pl
 * @docs https://docs.eqemu.dev/developer/repositories
 */

#pragma once

#include "common/database.h"
#include "common/strings.h"

#include <ctime>

class BaseAccountRepository {
public:
	struct Account {
		int32_t     id;
		std::string name;
		std::string charname;
		std::string auto_login_charname;
		uint32_t    sharedplat;
		std::string password;
		int32_t     status;
		std::string ls_id;
		uint32_t    lsaccount_id;
		uint8_t     gmspeed;
		int8_t      invulnerable;
		int8_t      flymode;
		int8_t      ignore_tells;
		uint8_t     revoked;
		uint32_t    karma;
		std::string minilogin_ip;
		int8_t      hideme;
		uint8_t     rulesflag;
		time_t      suspendeduntil;
		uint32_t    time_creation;
		std::string ban_reason;
		std::string suspend_reason;
		std::string crc_eqgame;
		std::string crc_skillcaps;
		std::string crc_basedata;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"name",
			"charname",
			"auto_login_charname",
			"sharedplat",
			"password",
			"status",
			"ls_id",
			"lsaccount_id",
			"gmspeed",
			"invulnerable",
			"flymode",
			"ignore_tells",
			"revoked",
			"karma",
			"minilogin_ip",
			"hideme",
			"rulesflag",
			"suspendeduntil",
			"time_creation",
			"ban_reason",
			"suspend_reason",
			"crc_eqgame",
			"crc_skillcaps",
			"crc_basedata",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"name",
			"charname",
			"auto_login_charname",
			"sharedplat",
			"password",
			"status",
			"ls_id",
			"lsaccount_id",
			"gmspeed",
			"invulnerable",
			"flymode",
			"ignore_tells",
			"revoked",
			"karma",
			"minilogin_ip",
			"hideme",
			"rulesflag",
			"UNIX_TIMESTAMP(suspendeduntil)",
			"time_creation",
			"ban_reason",
			"suspend_reason",
			"crc_eqgame",
			"crc_skillcaps",
			"crc_basedata",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(Strings::Implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("account");
	}

	static std::string BaseSelect()
	{
		return fmt::format(
			"SELECT {} FROM {}",
			SelectColumnsRaw(),
			TableName()
		);
	}

	static std::string BaseInsert()
	{
		return fmt::format(
			"INSERT INTO {} ({}) ",
			TableName(),
			ColumnsRaw()
		);
	}

	static Account NewEntity()
	{
		Account e{};

		e.id                  = 0;
		e.name                = "";
		e.charname            = "";
		e.auto_login_charname = "";
		e.sharedplat          = 0;
		e.password            = "";
		e.status              = 0;
		e.ls_id               = "eqemu";
		e.lsaccount_id        = 0;
		e.gmspeed             = 0;
		e.invulnerable        = 0;
		e.flymode             = 0;
		e.ignore_tells        = 0;
		e.revoked             = 0;
		e.karma               = 0;
		e.minilogin_ip        = "";
		e.hideme              = 0;
		e.rulesflag           = 0;
		e.suspendeduntil      = 0;
		e.time_creation       = 0;
		e.ban_reason          = "";
		e.suspend_reason      = "";
		e.crc_eqgame          = "";
		e.crc_skillcaps       = "";
		e.crc_basedata        = "";

		return e;
	}

	static Account GetAccount(
		const std::vector<Account> &accounts,
		int account_id
	)
	{
		for (auto &account : accounts) {
			if (account.id == account_id) {
				return account;
			}
		}

		return NewEntity();
	}

	static Account FindOne(
		Database& db,
		int account_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				account_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Account e{};

			e.id                  = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.name                = row[1] ? row[1] : "";
			e.charname            = row[2] ? row[2] : "";
			e.auto_login_charname = row[3] ? row[3] : "";
			e.sharedplat          = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.password            = row[5] ? row[5] : "";
			e.status              = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.ls_id               = row[7] ? row[7] : "eqemu";
			e.lsaccount_id        = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.gmspeed             = row[9] ? static_cast<uint8_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.invulnerable        = row[10] ? static_cast<int8_t>(atoi(row[10])) : 0;
			e.flymode             = row[11] ? static_cast<int8_t>(atoi(row[11])) : 0;
			e.ignore_tells        = row[12] ? static_cast<int8_t>(atoi(row[12])) : 0;
			e.revoked             = row[13] ? static_cast<uint8_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.karma               = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.minilogin_ip        = row[15] ? row[15] : "";
			e.hideme              = row[16] ? static_cast<int8_t>(atoi(row[16])) : 0;
			e.rulesflag           = row[17] ? static_cast<uint8_t>(strtoul(row[17], nullptr, 10)) : 0;
			e.suspendeduntil      = strtoll(row[18] ? row[18] : "-1", nullptr, 10);
			e.time_creation       = row[19] ? static_cast<uint32_t>(strtoul(row[19], nullptr, 10)) : 0;
			e.ban_reason          = row[20] ? row[20] : "";
			e.suspend_reason      = row[21] ? row[21] : "";
			e.crc_eqgame          = row[22] ? row[22] : "";
			e.crc_skillcaps       = row[23] ? row[23] : "";
			e.crc_basedata        = row[24] ? row[24] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int account_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				account_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Account &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[2] + " = '" + Strings::Escape(e.charname) + "'");
		v.push_back(columns[3] + " = '" + Strings::Escape(e.auto_login_charname) + "'");
		v.push_back(columns[4] + " = " + std::to_string(e.sharedplat));
		v.push_back(columns[5] + " = '" + Strings::Escape(e.password) + "'");
		v.push_back(columns[6] + " = " + std::to_string(e.status));
		v.push_back(columns[7] + " = '" + Strings::Escape(e.ls_id) + "'");
		v.push_back(columns[8] + " = " + std::to_string(e.lsaccount_id));
		v.push_back(columns[9] + " = " + std::to_string(e.gmspeed));
		v.push_back(columns[10] + " = " + std::to_string(e.invulnerable));
		v.push_back(columns[11] + " = " + std::to_string(e.flymode));
		v.push_back(columns[12] + " = " + std::to_string(e.ignore_tells));
		v.push_back(columns[13] + " = " + std::to_string(e.revoked));
		v.push_back(columns[14] + " = " + std::to_string(e.karma));
		v.push_back(columns[15] + " = '" + Strings::Escape(e.minilogin_ip) + "'");
		v.push_back(columns[16] + " = " + std::to_string(e.hideme));
		v.push_back(columns[17] + " = " + std::to_string(e.rulesflag));
		v.push_back(columns[18] + " = FROM_UNIXTIME(" + (e.suspendeduntil > 0 ? std::to_string(e.suspendeduntil) : "null") + ")");
		v.push_back(columns[19] + " = " + std::to_string(e.time_creation));
		v.push_back(columns[20] + " = '" + Strings::Escape(e.ban_reason) + "'");
		v.push_back(columns[21] + " = '" + Strings::Escape(e.suspend_reason) + "'");
		v.push_back(columns[22] + " = '" + Strings::Escape(e.crc_eqgame) + "'");
		v.push_back(columns[23] + " = '" + Strings::Escape(e.crc_skillcaps) + "'");
		v.push_back(columns[24] + " = '" + Strings::Escape(e.crc_basedata) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Account InsertOne(
		Database& db,
		Account e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back("'" + Strings::Escape(e.charname) + "'");
		v.push_back("'" + Strings::Escape(e.auto_login_charname) + "'");
		v.push_back(std::to_string(e.sharedplat));
		v.push_back("'" + Strings::Escape(e.password) + "'");
		v.push_back(std::to_string(e.status));
		v.push_back("'" + Strings::Escape(e.ls_id) + "'");
		v.push_back(std::to_string(e.lsaccount_id));
		v.push_back(std::to_string(e.gmspeed));
		v.push_back(std::to_string(e.invulnerable));
		v.push_back(std::to_string(e.flymode));
		v.push_back(std::to_string(e.ignore_tells));
		v.push_back(std::to_string(e.revoked));
		v.push_back(std::to_string(e.karma));
		v.push_back("'" + Strings::Escape(e.minilogin_ip) + "'");
		v.push_back(std::to_string(e.hideme));
		v.push_back(std::to_string(e.rulesflag));
		v.push_back("FROM_UNIXTIME(" + (e.suspendeduntil > 0 ? std::to_string(e.suspendeduntil) : "null") + ")");
		v.push_back(std::to_string(e.time_creation));
		v.push_back("'" + Strings::Escape(e.ban_reason) + "'");
		v.push_back("'" + Strings::Escape(e.suspend_reason) + "'");
		v.push_back("'" + Strings::Escape(e.crc_eqgame) + "'");
		v.push_back("'" + Strings::Escape(e.crc_skillcaps) + "'");
		v.push_back("'" + Strings::Escape(e.crc_basedata) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<Account> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back("'" + Strings::Escape(e.charname) + "'");
			v.push_back("'" + Strings::Escape(e.auto_login_charname) + "'");
			v.push_back(std::to_string(e.sharedplat));
			v.push_back("'" + Strings::Escape(e.password) + "'");
			v.push_back(std::to_string(e.status));
			v.push_back("'" + Strings::Escape(e.ls_id) + "'");
			v.push_back(std::to_string(e.lsaccount_id));
			v.push_back(std::to_string(e.gmspeed));
			v.push_back(std::to_string(e.invulnerable));
			v.push_back(std::to_string(e.flymode));
			v.push_back(std::to_string(e.ignore_tells));
			v.push_back(std::to_string(e.revoked));
			v.push_back(std::to_string(e.karma));
			v.push_back("'" + Strings::Escape(e.minilogin_ip) + "'");
			v.push_back(std::to_string(e.hideme));
			v.push_back(std::to_string(e.rulesflag));
			v.push_back("FROM_UNIXTIME(" + (e.suspendeduntil > 0 ? std::to_string(e.suspendeduntil) : "null") + ")");
			v.push_back(std::to_string(e.time_creation));
			v.push_back("'" + Strings::Escape(e.ban_reason) + "'");
			v.push_back("'" + Strings::Escape(e.suspend_reason) + "'");
			v.push_back("'" + Strings::Escape(e.crc_eqgame) + "'");
			v.push_back("'" + Strings::Escape(e.crc_skillcaps) + "'");
			v.push_back("'" + Strings::Escape(e.crc_basedata) + "'");

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<Account> All(Database& db)
	{
		std::vector<Account> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Account e{};

			e.id                  = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.name                = row[1] ? row[1] : "";
			e.charname            = row[2] ? row[2] : "";
			e.auto_login_charname = row[3] ? row[3] : "";
			e.sharedplat          = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.password            = row[5] ? row[5] : "";
			e.status              = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.ls_id               = row[7] ? row[7] : "eqemu";
			e.lsaccount_id        = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.gmspeed             = row[9] ? static_cast<uint8_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.invulnerable        = row[10] ? static_cast<int8_t>(atoi(row[10])) : 0;
			e.flymode             = row[11] ? static_cast<int8_t>(atoi(row[11])) : 0;
			e.ignore_tells        = row[12] ? static_cast<int8_t>(atoi(row[12])) : 0;
			e.revoked             = row[13] ? static_cast<uint8_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.karma               = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.minilogin_ip        = row[15] ? row[15] : "";
			e.hideme              = row[16] ? static_cast<int8_t>(atoi(row[16])) : 0;
			e.rulesflag           = row[17] ? static_cast<uint8_t>(strtoul(row[17], nullptr, 10)) : 0;
			e.suspendeduntil      = strtoll(row[18] ? row[18] : "-1", nullptr, 10);
			e.time_creation       = row[19] ? static_cast<uint32_t>(strtoul(row[19], nullptr, 10)) : 0;
			e.ban_reason          = row[20] ? row[20] : "";
			e.suspend_reason      = row[21] ? row[21] : "";
			e.crc_eqgame          = row[22] ? row[22] : "";
			e.crc_skillcaps       = row[23] ? row[23] : "";
			e.crc_basedata        = row[24] ? row[24] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Account> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Account> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Account e{};

			e.id                  = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.name                = row[1] ? row[1] : "";
			e.charname            = row[2] ? row[2] : "";
			e.auto_login_charname = row[3] ? row[3] : "";
			e.sharedplat          = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.password            = row[5] ? row[5] : "";
			e.status              = row[6] ? static_cast<int32_t>(atoi(row[6])) : 0;
			e.ls_id               = row[7] ? row[7] : "eqemu";
			e.lsaccount_id        = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.gmspeed             = row[9] ? static_cast<uint8_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.invulnerable        = row[10] ? static_cast<int8_t>(atoi(row[10])) : 0;
			e.flymode             = row[11] ? static_cast<int8_t>(atoi(row[11])) : 0;
			e.ignore_tells        = row[12] ? static_cast<int8_t>(atoi(row[12])) : 0;
			e.revoked             = row[13] ? static_cast<uint8_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.karma               = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.minilogin_ip        = row[15] ? row[15] : "";
			e.hideme              = row[16] ? static_cast<int8_t>(atoi(row[16])) : 0;
			e.rulesflag           = row[17] ? static_cast<uint8_t>(strtoul(row[17], nullptr, 10)) : 0;
			e.suspendeduntil      = strtoll(row[18] ? row[18] : "-1", nullptr, 10);
			e.time_creation       = row[19] ? static_cast<uint32_t>(strtoul(row[19], nullptr, 10)) : 0;
			e.ban_reason          = row[20] ? row[20] : "";
			e.suspend_reason      = row[21] ? row[21] : "";
			e.crc_eqgame          = row[22] ? row[22] : "";
			e.crc_skillcaps       = row[23] ? row[23] : "";
			e.crc_basedata        = row[24] ? row[24] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static int DeleteWhere(Database& db, const std::string &where_filter)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int Truncate(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int64 GetMaxId(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COALESCE(MAX({}), 0) FROM {}",
				PrimaryKey(),
				TableName()
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static int64 Count(Database& db, const std::string &where_filter = "")
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COUNT(*) FROM {} {}",
				TableName(),
				(where_filter.empty() ? "" : "WHERE " + where_filter)
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static std::string BaseReplace()
	{
		return fmt::format(
			"REPLACE INTO {} ({}) ",
			TableName(),
			ColumnsRaw()
		);
	}

	static int ReplaceOne(
		Database& db,
		const Account &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back("'" + Strings::Escape(e.charname) + "'");
		v.push_back("'" + Strings::Escape(e.auto_login_charname) + "'");
		v.push_back(std::to_string(e.sharedplat));
		v.push_back("'" + Strings::Escape(e.password) + "'");
		v.push_back(std::to_string(e.status));
		v.push_back("'" + Strings::Escape(e.ls_id) + "'");
		v.push_back(std::to_string(e.lsaccount_id));
		v.push_back(std::to_string(e.gmspeed));
		v.push_back(std::to_string(e.invulnerable));
		v.push_back(std::to_string(e.flymode));
		v.push_back(std::to_string(e.ignore_tells));
		v.push_back(std::to_string(e.revoked));
		v.push_back(std::to_string(e.karma));
		v.push_back("'" + Strings::Escape(e.minilogin_ip) + "'");
		v.push_back(std::to_string(e.hideme));
		v.push_back(std::to_string(e.rulesflag));
		v.push_back("FROM_UNIXTIME(" + (e.suspendeduntil > 0 ? std::to_string(e.suspendeduntil) : "null") + ")");
		v.push_back(std::to_string(e.time_creation));
		v.push_back("'" + Strings::Escape(e.ban_reason) + "'");
		v.push_back("'" + Strings::Escape(e.suspend_reason) + "'");
		v.push_back("'" + Strings::Escape(e.crc_eqgame) + "'");
		v.push_back("'" + Strings::Escape(e.crc_skillcaps) + "'");
		v.push_back("'" + Strings::Escape(e.crc_basedata) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseReplace(),
				Strings::Implode(",", v)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int ReplaceMany(
		Database& db,
		const std::vector<Account> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back("'" + Strings::Escape(e.charname) + "'");
			v.push_back("'" + Strings::Escape(e.auto_login_charname) + "'");
			v.push_back(std::to_string(e.sharedplat));
			v.push_back("'" + Strings::Escape(e.password) + "'");
			v.push_back(std::to_string(e.status));
			v.push_back("'" + Strings::Escape(e.ls_id) + "'");
			v.push_back(std::to_string(e.lsaccount_id));
			v.push_back(std::to_string(e.gmspeed));
			v.push_back(std::to_string(e.invulnerable));
			v.push_back(std::to_string(e.flymode));
			v.push_back(std::to_string(e.ignore_tells));
			v.push_back(std::to_string(e.revoked));
			v.push_back(std::to_string(e.karma));
			v.push_back("'" + Strings::Escape(e.minilogin_ip) + "'");
			v.push_back(std::to_string(e.hideme));
			v.push_back(std::to_string(e.rulesflag));
			v.push_back("FROM_UNIXTIME(" + (e.suspendeduntil > 0 ? std::to_string(e.suspendeduntil) : "null") + ")");
			v.push_back(std::to_string(e.time_creation));
			v.push_back("'" + Strings::Escape(e.ban_reason) + "'");
			v.push_back("'" + Strings::Escape(e.suspend_reason) + "'");
			v.push_back("'" + Strings::Escape(e.crc_eqgame) + "'");
			v.push_back("'" + Strings::Escape(e.crc_skillcaps) + "'");
			v.push_back("'" + Strings::Escape(e.crc_basedata) + "'");

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseReplace(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}
};
