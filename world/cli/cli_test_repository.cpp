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
#include "world/world_server_cli.h"

#include "common/repositories/instance_list_repository.h"
#include "world/worlddb.h"

void WorldserverCLI::TestRepository(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Test command";

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	/**
	 * Insert one
	 */
	auto e = InstanceListRepository::NewEntity();

	e.zone          = 999;
	e.version       = 1;
	e.is_global     = 1;
	e.start_time    = 0;
	e.duration      = 0;
	e.never_expires = 1;

	auto inserted = InstanceListRepository::InsertOne(database, e);

	LogInfo("Inserted ID is [{}] zone [{}]", inserted.id, inserted.zone);

	/**
	 * Find one
	 */
	auto f = InstanceListRepository::FindOne(database, inserted.id);

	LogInfo("Found ID is [{}] zone [{}]", f.id, f.zone);

	/**
	 * Update one
	 */
	LogInfo("Updating instance id [{}] zone [{}]", f.id, f.zone);

	int update_instance_list_count = InstanceListRepository::UpdateOne(database, f);

	f.zone = 777;

	LogInfo(
		"Updated instance id [{}] zone [{}] affected [{}]",
		f.id,
		f.zone,
		update_instance_list_count
	);


	/**
	 * Delete one
	 */
	int deleted = InstanceListRepository::DeleteOne(database, f.id);

	LogInfo("Deleting one instance [{}] deleted count [{}]", f.id, deleted);

	/**
	 * Insert many
	 */
	std::vector<InstanceListRepository::InstanceList> instance_lists;

	auto b = InstanceListRepository::NewEntity();

	b.zone          = 999;
	b.version       = 1;
	b.is_global     = 1;
	b.start_time    = 0;
	b.duration      = 0;
	b.never_expires = 1;

	for (int i = 0; i < 10; i++) {
		instance_lists.push_back(b);
	}

	/**
	 * Insert Many
	 */
	int inserted_count = InstanceListRepository::InsertMany(database, instance_lists);

	LogInfo("Bulk insertion test, inserted [{}]", inserted_count);

	for (auto &entry: InstanceListRepository::GetWhere(database, fmt::format("zone = {}", 999))) {
		LogInfo("Iterating through entry id [{}] zone [{}]", entry.id, entry.zone);
	}

	LogInfo("[Max ID] {}", InstanceListRepository::GetMaxId(database));
	LogInfo("[Count] {}", InstanceListRepository::Count(database));
	LogInfo("[Count Where] {}", InstanceListRepository::Count(database, "zone = 999"));
	LogInfo("[Count Where] {}", InstanceListRepository::Count(database, "zone = 777"));

	/**
	 * Delete where
	 */
	int deleted_count = InstanceListRepository::DeleteWhere(database, fmt::format("zone = {}", 999));

	LogInfo("Bulk deletion test, deleted [{}]", deleted_count);

}
