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

#include "common/repositories/base/base_character_exp_modifiers_repository.h"

#include "common/database.h"
#include "common/strings.h"

class CharacterExpModifiersRepository: public BaseCharacterExpModifiersRepository {
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
     * CharacterExpModifiersRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * CharacterExpModifiersRepository::GetWhereNeverExpires()
     * CharacterExpModifiersRepository::GetWhereXAndY()
     * CharacterExpModifiersRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here
	static EXPModifier GetEXPModifier(
		Database& db,
		uint32 character_id,
		uint32 zone_id,
		int16 instance_version
	)
	{
		const auto& l = CharacterExpModifiersRepository::GetWhere(
			db,
			fmt::format(
				SQL(
					`character_id` = {} AND
					(`zone_id` = {} OR `zone_id` = 0) AND
					(`instance_version` = {} OR `instance_version` = -1)
					ORDER BY `zone_id`, `instance_version` DESC
					LIMIT 1
				),
				character_id,
				zone_id,
				instance_version
			)
		);

		if (l.empty()) {
			return EXPModifier{
				.aa_modifier = 1.0f,
				.exp_modifier = 1.0f
			};
		}

		const auto& m = l.front();

		return EXPModifier{
			.aa_modifier = m.aa_modifier,
			.exp_modifier = m.exp_modifier
		};
	}

	static void SetEXPModifier(
		Database& db,
		uint32 character_id,
		uint32 zone_id,
		int16 instance_version,
		EXPModifier m
	)
	{
		CharacterExpModifiersRepository::ReplaceOne(
			db,
			CharacterExpModifiersRepository::CharacterExpModifiers{
				.character_id = static_cast<int32_t>(character_id),
				.zone_id = static_cast<int32_t>(zone_id),
				.instance_version = instance_version,
				.aa_modifier = m.aa_modifier,
				.exp_modifier = m.exp_modifier
			}
		);
	}
};
