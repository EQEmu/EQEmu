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

#include "common/repositories/base/base_character_parcels_repository.h"

#include "common/database.h"
#include "common/strings.h"

class CharacterParcelsRepository: public BaseCharacterParcelsRepository {
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
	 * ParcelsRepository::GetByZoneAndVersion(int zone_id, int zone_version)
	 * ParcelsRepository::GetWhereNeverExpires()
	 * ParcelsRepository::GetWhereXAndY()
	 * ParcelsRepository::DeleteWhereXAndY()
	 *
	 * Most of the above could be covered by base methods, but if you as a developer
	 * find yourself re-using logic for other parts of the code, its best to just make a
	 * method that can be re-used easily elsewhere especially if it can use a base repository
	 * method and encapsulate filters there
	 */

	// Custom extended repository methods here
	struct ParcelCountAndCharacterName
	{
		std::string character_name;
		uint32      char_id;
		uint32      parcel_count;
	};

	static std::vector<ParcelCountAndCharacterName> GetParcelCountAndCharacterName(Database &db, const std::string &character_name)
	{
		std::vector<ParcelCountAndCharacterName> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT c.name, COUNT(p.id), c.id FROM character_data c "
				"LEFT JOIN character_parcels p ON p.char_id = c.id "
				"WHERE c.name = '{}' "
				"LIMIT 1",
				character_name)
		);

		all_entries.reserve(results.RowCount());

		for(auto row = results.begin(); row != results.end(); ++row) {
			ParcelCountAndCharacterName e {};

			e.character_name = row[0] ? row[0] : "";
			e.parcel_count   = row[1] ? Strings::ToUnsignedInt(row[1]) : 0;
			e.char_id        = row[2] ? Strings::ToUnsignedInt(row[2]) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static int GetNextFreeParcelSlot(Database& db, const uint32 character_id, const uint32 max_slots)
	{
		const auto& l = CharacterParcelsRepository::GetWhere(
			db,
			fmt::format(
				"char_id = '{}' ORDER BY slot_id ASC",
				character_id
			)
		);

		if (l.empty()) {
			return PARCEL_BEGIN_SLOT;
		}

		for (uint32 i = PARCEL_BEGIN_SLOT; i <= max_slots; i++) {
			auto it = std::find_if(
				l.cbegin(),
				l.cend(),
				[&](const auto &x) {
					return x.slot_id == i;
				}
			);

			if (it == l.end()) {
				return i;
			}
		}

		return INVALID_INDEX;
	}
};
