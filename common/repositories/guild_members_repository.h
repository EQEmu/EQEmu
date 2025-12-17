#pragma once

#include "common/repositories/base/base_guild_members_repository.h"

#include "common/database.h"
#include "common/strings.h"

class GuildMembersRepository : public BaseGuildMembersRepository {
public:

	struct GuildMembersWithTributeOnStruct {
		int32_t     char_id;
		uint32_t    guild_id;
		uint8_t     tribute_enable;
		std::string char_name;
		uint32_t    char_level;
	};

	struct GuildMembershipStatsStruct {
		uint32 leaders;
		uint32 senior_officers;
		uint32 officers;
		uint32 senior_members;
		uint32 members;
		uint32 junior_members;
		uint32 initates;
		uint32 recruits;
		uint32 tribute_enabled;
	};

	static int UpdateMemberRank(Database &db, uint32 char_id, uint32 rank_id)
	{
		const auto guild_members = GetWhere(db, fmt::format("char_id = '{}'", char_id));
		if (guild_members.empty()) {
			return 0;
		}

		auto m = guild_members[0];
		m.rank_ = rank_id;

		return UpdateOne(db, m);
	}

	static int UpdateEnabled(Database &db, uint32 guild_id, uint32 char_id, uint32 enabled)
	{
		const auto guild_members = GetWhere(db, fmt::format("char_id = '{}' AND guild_id = '{}'", char_id, guild_id));
		if (guild_members.empty()) {
			return 0;
		}

		auto m = guild_members[0];
		m.tribute_enable = enabled ? 1 : 0;

		return UpdateOne(db, m);
	}

	static std::vector<GuildMembersWithTributeOnStruct> GetMembersWithTributeOn(Database &db, uint32 guild_id)
	{
		std::vector<GuildMembersWithTributeOnStruct> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT gm.char_id, gm.guild_id, gm.tribute_enable, cd.`name`, cd.`level`, cd.deleted_at "
				"FROM guild_members gm JOIN character_data cd ON cd.id = gm.char_id "
				"WHERE ISNULL(cd.deleted_at) AND gm.tribute_enable = 1 AND gm.guild_id = '{}';",
				guild_id
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GuildMembersWithTributeOnStruct e{};

			e.char_id        = static_cast<int32_t>(atoi(row[0]));
			e.guild_id       = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.tribute_enable = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.char_name      = row[3] ? row[3] : "";
			e.char_level     = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static int UpdateFavor(Database &db, uint32 guild_id, uint32 char_id, uint32 favor)
	{
		const auto guild_members = GetWhere(db, fmt::format("char_id = '{}' AND guild_id = '{}'", char_id, guild_id));
		if (guild_members.empty()) {
			return 0;
		}

		auto m = guild_members[0];
		m.total_tribute = favor;
		m.last_tribute  = time(nullptr);

		return UpdateOne(db, m);
	}

	static int UpdateOnline(Database &db, uint32 char_id, bool status)
	{
		const auto guild_members = GetWhere(db, fmt::format("char_id = '{}'", char_id));
		if (guild_members.empty()) {
			return 0;
		}

		auto m = guild_members[0];
		m.online = status ? 1 : 0;

		return UpdateOne(db, m);
	}

	static int UpdateNote(Database &db, uint32 char_id, std::string &public_note)
	{
		const auto guild_members = GetWhere(db, fmt::format("char_id = '{}'", char_id));
		if (guild_members.empty()) {
			return 0;
		}

		auto m = guild_members[0];
		m.public_note = public_note;

		return UpdateOne(db, m);
	}

	static GuildMembershipStatsStruct GetGuildMembershipStats(Database &db, uint32 guild_id)
	{
		std::string query = fmt::format(
			"SELECT "
			"SUM(CASE WHEN gm.`rank` = 1 THEN 1 ELSE 0 END) AS RANK1, "
			"SUM(CASE WHEN gm.`rank` = 2 THEN 1 ELSE 0 END) AS RANK2, "
			"SUM(CASE WHEN gm.`rank` = 3 THEN 1 ELSE 0 END) AS RANK3, "
			"SUM(CASE WHEN gm.`rank` = 4 THEN 1 ELSE 0 END) AS RANK4, "
			"SUM(CASE WHEN gm.`rank` = 5 THEN 1 ELSE 0 END) AS RANK5, "
			"SUM(CASE WHEN gm.`rank` = 6 THEN 1 ELSE 0 END) AS RANK6, "
			"SUM(CASE WHEN gm.`rank` = 7 THEN 1 ELSE 0 END) AS RANK7, "
			"SUM(CASE WHEN gm.`rank` = 8 THEN 1 ELSE 0 END) AS RANK8, "
			"SUM(CASE WHEN gm.tribute_enable = 1 THEN 1 ELSE 0 END) AS TRIBUTE "
			"FROM guild_members gm "
			"WHERE gm.guild_id = '{}';",
			guild_id
		);

		GuildMembershipStatsStruct gmss{};

		auto results = db.QueryDatabase(query);
		if (!results.Success()) {
			return gmss;
		}

		for (auto row = results.begin(); row != results.end(); ++row) {

			gmss.leaders         = static_cast<uint32_t>(Strings::ToUnsignedInt(row[0]));
			gmss.senior_officers = static_cast<uint32_t>(Strings::ToUnsignedInt(row[1]));
			gmss.officers        = static_cast<uint32_t>(Strings::ToUnsignedInt(row[2]));
			gmss.senior_members  = static_cast<uint32_t>(Strings::ToUnsignedInt(row[3]));
			gmss.members         = static_cast<uint32_t>(Strings::ToUnsignedInt(row[4]));
			gmss.junior_members  = static_cast<uint32_t>(Strings::ToUnsignedInt(row[5]));
			gmss.initates        = static_cast<uint32_t>(Strings::ToUnsignedInt(row[6]));
			gmss.recruits        = static_cast<uint32_t>(Strings::ToUnsignedInt(row[7]));
			gmss.tribute_enabled = static_cast<uint32_t>(Strings::ToUnsignedInt(row[8]));
		}

		return gmss;
	}

	static int UpdateBankerFlag(Database &db, uint32 char_id, bool status)
	{
		const auto guild_members = GetWhere(db, fmt::format("char_id = {}", char_id));
		if (guild_members.empty()) {
			return 0;
		}

		auto m = guild_members[0];
		m.banker = status ? 1 : 0;

		return UpdateOne(db, m);
	}

	static int UpdateAltFlag(Database &db, uint32 char_id, bool status)
	{
		const auto guild_members = GetWhere(db, fmt::format("char_id = {}", char_id));
		if (guild_members.empty()) {
			return 0;
		}

		auto m = guild_members[0];
		m.alt = status ? 1 : 0;

		return UpdateOne(db, m);
	}

	static void ClearOnlineStatus(Database &db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET `online` = 0 "
				"WHERE `online` = 1;",
				TableName()
			)
		);
	}
};
