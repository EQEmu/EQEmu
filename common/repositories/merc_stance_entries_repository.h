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

#include "common/repositories/base/base_merc_stance_entries_repository.h"

#include "common/database.h"
#include "common/strings.h"

class MercStanceEntriesRepository: public BaseMercStanceEntriesRepository {
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
     * MercStanceEntriesRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * MercStanceEntriesRepository::GetWhereNeverExpires()
     * MercStanceEntriesRepository::GetWhereXAndY()
     * MercStanceEntriesRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here
	static std::vector<MercStanceEntriesRepository::MercStanceEntries> GetAllOrdered(Database& db)
	{
		std::vector<MercStanceEntriesRepository::MercStanceEntries> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT {} FROM {} ORDER BY `class_id`, `proficiency_id`, `stance_id`",
				SelectColumnsRaw(),
				TableName()
			)
		);

		if (!results.Success() || !results.RowCount()) {
			return v;
		}

		v.reserve(results.RowCount());

		auto e = MercStanceEntriesRepository::NewEntity();

		for (auto row : results) {
			e.merc_stance_entry_id = Strings::ToUnsignedInt(row[0]);
			e.class_id             = Strings::ToUnsignedInt(row[1]);
			e.proficiency_id       = static_cast<uint8_t>(Strings::ToUnsignedInt(row[2]));
			e.stance_id            = static_cast<uint8_t>(Strings::ToUnsignedInt(row[3]));
			e.isdefault            = static_cast<int8_t>(Strings::ToInt(row[4]));

			v.emplace_back(e);
		}

		return v;
	}
};
