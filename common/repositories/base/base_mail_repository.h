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

class BaseMailRepository {
public:
	struct Mail {
		uint32_t    msgid;
		uint32_t    charid;
		int32_t     timestamp;
		std::string from;
		std::string subject;
		std::string body;
		std::string to;
		int8_t      status;
	};

	static std::string PrimaryKey()
	{
		return std::string("msgid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"msgid",
			"charid",
			"timestamp",
			"from",
			"subject",
			"body",
			"to",
			"status",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"msgid",
			"charid",
			"timestamp",
			"from",
			"subject",
			"body",
			"to",
			"status",
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
		return std::string("mail");
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

	static Mail NewEntity()
	{
		Mail e{};

		e.msgid     = 0;
		e.charid    = 0;
		e.timestamp = 0;
		e.from      = "";
		e.subject   = "";
		e.body      = "";
		e.to        = "";
		e.status    = 0;

		return e;
	}

	static Mail GetMail(
		const std::vector<Mail> &mails,
		int mail_id
	)
	{
		for (auto &mail : mails) {
			if (mail.msgid == mail_id) {
				return mail;
			}
		}

		return NewEntity();
	}

	static Mail FindOne(
		Database& db,
		int mail_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				mail_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Mail e{};

			e.msgid     = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.charid    = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.timestamp = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.from      = row[3] ? row[3] : "";
			e.subject   = row[4] ? row[4] : "";
			e.body      = row[5] ? row[5] : "";
			e.to        = row[6] ? row[6] : "";
			e.status    = row[7] ? static_cast<int8_t>(atoi(row[7])) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int mail_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				mail_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Mail &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.charid));
		v.push_back(columns[2] + " = " + std::to_string(e.timestamp));
		v.push_back(columns[3] + " = '" + Strings::Escape(e.from) + "'");
		v.push_back(columns[4] + " = '" + Strings::Escape(e.subject) + "'");
		v.push_back(columns[5] + " = '" + Strings::Escape(e.body) + "'");
		v.push_back(columns[6] + " = '" + Strings::Escape(e.to) + "'");
		v.push_back(columns[7] + " = " + std::to_string(e.status));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.msgid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Mail InsertOne(
		Database& db,
		Mail e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.msgid));
		v.push_back(std::to_string(e.charid));
		v.push_back(std::to_string(e.timestamp));
		v.push_back("'" + Strings::Escape(e.from) + "'");
		v.push_back("'" + Strings::Escape(e.subject) + "'");
		v.push_back("'" + Strings::Escape(e.body) + "'");
		v.push_back("'" + Strings::Escape(e.to) + "'");
		v.push_back(std::to_string(e.status));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.msgid = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<Mail> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.msgid));
			v.push_back(std::to_string(e.charid));
			v.push_back(std::to_string(e.timestamp));
			v.push_back("'" + Strings::Escape(e.from) + "'");
			v.push_back("'" + Strings::Escape(e.subject) + "'");
			v.push_back("'" + Strings::Escape(e.body) + "'");
			v.push_back("'" + Strings::Escape(e.to) + "'");
			v.push_back(std::to_string(e.status));

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

	static std::vector<Mail> All(Database& db)
	{
		std::vector<Mail> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Mail e{};

			e.msgid     = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.charid    = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.timestamp = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.from      = row[3] ? row[3] : "";
			e.subject   = row[4] ? row[4] : "";
			e.body      = row[5] ? row[5] : "";
			e.to        = row[6] ? row[6] : "";
			e.status    = row[7] ? static_cast<int8_t>(atoi(row[7])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Mail> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Mail> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Mail e{};

			e.msgid     = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.charid    = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.timestamp = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;
			e.from      = row[3] ? row[3] : "";
			e.subject   = row[4] ? row[4] : "";
			e.body      = row[5] ? row[5] : "";
			e.to        = row[6] ? row[6] : "";
			e.status    = row[7] ? static_cast<int8_t>(atoi(row[7])) : 0;

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
		const Mail &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.msgid));
		v.push_back(std::to_string(e.charid));
		v.push_back(std::to_string(e.timestamp));
		v.push_back("'" + Strings::Escape(e.from) + "'");
		v.push_back("'" + Strings::Escape(e.subject) + "'");
		v.push_back("'" + Strings::Escape(e.body) + "'");
		v.push_back("'" + Strings::Escape(e.to) + "'");
		v.push_back(std::to_string(e.status));

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
		const std::vector<Mail> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.msgid));
			v.push_back(std::to_string(e.charid));
			v.push_back(std::to_string(e.timestamp));
			v.push_back("'" + Strings::Escape(e.from) + "'");
			v.push_back("'" + Strings::Escape(e.subject) + "'");
			v.push_back("'" + Strings::Escape(e.body) + "'");
			v.push_back("'" + Strings::Escape(e.to) + "'");
			v.push_back(std::to_string(e.status));

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
