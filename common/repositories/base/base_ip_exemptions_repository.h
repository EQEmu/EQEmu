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

class BaseIpExemptionsRepository {
public:
	struct IpExemptions {
		int32_t     exemption_id;
		std::string exemption_ip;
		int32_t     exemption_amount;
	};

	static std::string PrimaryKey()
	{
		return std::string("exemption_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"exemption_id",
			"exemption_ip",
			"exemption_amount",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"exemption_id",
			"exemption_ip",
			"exemption_amount",
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
		return std::string("ip_exemptions");
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

	static IpExemptions NewEntity()
	{
		IpExemptions e{};

		e.exemption_id     = 0;
		e.exemption_ip     = "";
		e.exemption_amount = 0;

		return e;
	}

	static IpExemptions GetIpExemptions(
		const std::vector<IpExemptions> &ip_exemptionss,
		int ip_exemptions_id
	)
	{
		for (auto &ip_exemptions : ip_exemptionss) {
			if (ip_exemptions.exemption_id == ip_exemptions_id) {
				return ip_exemptions;
			}
		}

		return NewEntity();
	}

	static IpExemptions FindOne(
		Database& db,
		int ip_exemptions_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				ip_exemptions_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			IpExemptions e{};

			e.exemption_id     = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.exemption_ip     = row[1] ? row[1] : "";
			e.exemption_amount = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int ip_exemptions_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				ip_exemptions_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const IpExemptions &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.exemption_ip) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.exemption_amount));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.exemption_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static IpExemptions InsertOne(
		Database& db,
		IpExemptions e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.exemption_id));
		v.push_back("'" + Strings::Escape(e.exemption_ip) + "'");
		v.push_back(std::to_string(e.exemption_amount));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.exemption_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<IpExemptions> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.exemption_id));
			v.push_back("'" + Strings::Escape(e.exemption_ip) + "'");
			v.push_back(std::to_string(e.exemption_amount));

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

	static std::vector<IpExemptions> All(Database& db)
	{
		std::vector<IpExemptions> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			IpExemptions e{};

			e.exemption_id     = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.exemption_ip     = row[1] ? row[1] : "";
			e.exemption_amount = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<IpExemptions> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<IpExemptions> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			IpExemptions e{};

			e.exemption_id     = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.exemption_ip     = row[1] ? row[1] : "";
			e.exemption_amount = row[2] ? static_cast<int32_t>(atoi(row[2])) : 0;

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
		const IpExemptions &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.exemption_id));
		v.push_back("'" + Strings::Escape(e.exemption_ip) + "'");
		v.push_back(std::to_string(e.exemption_amount));

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
		const std::vector<IpExemptions> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.exemption_id));
			v.push_back("'" + Strings::Escape(e.exemption_ip) + "'");
			v.push_back(std::to_string(e.exemption_amount));

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
