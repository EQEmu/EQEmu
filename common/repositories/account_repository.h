#pragma once

#include "common/repositories/base/base_account_repository.h"

#include "common/database.h"
#include "common/strings.h"

class AccountRepository: public BaseAccountRepository {
public:

    /**
     * This file was auto generated and can be modified and extended upon
     *
     * Base repository methods are automatically
     * generated in the "base" version of this repository. The base repository
     * is immutable and to be left untouched, while methods in this class
     * are used as extension methods for more specific persistence-layer
     * accessors or mutators.
     *
     * Base Methods (Subject to be expanded upon in time)
     *
     * Note: Not all tables are designed appropriately to fit functionality with all base methods
     *
     * InsertOne
     * UpdateOne
     * DeleteOne
     * FindOne
     * GetWhere(std::string where_filter)
     * DeleteWhere(std::string where_filter)
     * InsertMany
     * All
     *
     * Example custom methods in a repository
     *
     * AccountRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * AccountRepository::GetWhereNeverExpires()
     * AccountRepository::GetWhereXAndY()
     * AccountRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here
	static int16 GetAccountStatus(Database& db, const uint32 account_id)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT `status`, TIMESTAMPDIFF(SECOND, NOW(), `suspendeduntil`) FROM `{}` WHERE `{}` = {}",
				TableName(),
				PrimaryKey(),
				account_id
			)
		);

		if (!results.Success() || !results.RowCount()) {
			return 0;
		}

		auto row = results.begin();

		int16 status    = static_cast<int16>(Strings::ToInt(row[0]));
		int   date_diff = 0;

		if (row[1]) {
			date_diff = Strings::ToInt(row[1]);
		}

		if (date_diff > 0) {
			status = -1;
		}

		return status;
	}

	static bool UpdatePassword(Database& db, const uint32 account_id, const std::string& password)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET `password` = MD5('{}') WHERE `{}` = {}",
				TableName(),
				password,
				PrimaryKey(),
				account_id
			)
		);

		return results.Success();
	}

	static std::string GetAutoLoginCharacterNameByAccountID(Database& db, const uint32 account_id)
	{
		return AccountRepository::FindOne(db, account_id).auto_login_charname;
	}

	static bool SetAutoLoginCharacterNameByAccountID(Database& db, const uint32 account_id, const std::string& character_name)
	{
		auto e = AccountRepository::FindOne(db, account_id);

		if (!e.id) {
			return false;
		}

		e.auto_login_charname = character_name;

		return AccountRepository::UpdateOne(db, e);
	}
};
