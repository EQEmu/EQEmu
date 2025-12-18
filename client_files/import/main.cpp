/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2013 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "common/content/world_content_service.h"
#include "common/crash.h"
#include "common/eqemu_config.h"
#include "common/eqemu_logsys.h"
#include "common/events/player_event_logs.h"
#include "common/evolving_items.h"
#include "common/file.h"
#include "common/path_manager.h"
#include "common/platform.h"
#include "common/repositories/base_data_repository.h"
#include "common/rulesys.h"
#include "common/shareddb.h"
#include "common/strings.h"
#include "common/zone_store.h"

void ImportSpells(SharedDatabase *db);
void ImportSkillCaps(SharedDatabase *db);
void ImportBaseData(SharedDatabase *db);
void ImportDBStrings(SharedDatabase *db);

int main(int argc, char **argv) {
	RegisterExecutablePlatform(ExePlatformClientImport);
	EQEmuLogSys::Instance()->LoadLogSettingsDefaults();
	set_exception_handler();

	PathManager::Instance()->Init();

	LogInfo("Client Files Import Utility");
	if(!EQEmuConfig::LoadConfig()) {
		LogError("Unable to load configuration file.");
		return 1;
	}

	auto Config = EQEmuConfig::get();

	SharedDatabase database;
	SharedDatabase content_db;

	LogInfo("Connecting to database");
	if (!database.Connect(
		Config->DatabaseHost.c_str(),
		Config->DatabaseUsername.c_str(),
		Config->DatabasePassword.c_str(),
		Config->DatabaseDB.c_str(),
		Config->DatabasePort
	)) {
		LogError("Unable to connect to the database, cannot continue without a database connection");
		return 1;
	}

	/**
	 * Multi-tenancy: Content database
	 */
	if (!Config->ContentDbHost.empty()) {
		if (!content_db.Connect(
			Config->ContentDbHost.c_str() ,
			Config->ContentDbUsername.c_str(),
			Config->ContentDbPassword.c_str(),
			Config->ContentDbName.c_str(),
			Config->ContentDbPort
		)) {
			LogError("Cannot continue without a content database connection");
			return 1;
		}
	} else {
		content_db.SetMySQL(database);
	}

	EQEmuLogSys::Instance()->SetDatabase(&database)
		->SetLogPath(PathManager::Instance()->GetLogPath())
		->LoadLogDatabaseSettings()
		->StartFileLogs();

	ImportSpells(&content_db);
	ImportSkillCaps(&content_db);
	ImportBaseData(&content_db);
	ImportDBStrings(&database);

	EQEmuLogSys::Instance()->CloseFileLogs();

	return 0;
}

int GetSpellColumns(SharedDatabase *db) {

	const std::string query = "DESCRIBE spells_new";
	auto results = db->QueryDatabase(query);
	if(!results.Success()) {
        return 0;
    }

	return results.RowCount();
}

bool IsStringField(int i) {
	switch(i)
	{
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
		return true;
		break;
	default:
		return false;
	}
}

void ImportSpells(SharedDatabase *db) {
	LogInfo("Importing Spells");
	std::string file = fmt::format("{}/import/spells_us.txt", PathManager::Instance()->GetServerPath());
	FILE *f = fopen(file.c_str(), "r");
	if(!f) {
		LogError("Unable to open {} to read, skipping.", file);
		return;
	}

	std::string query = "DELETE FROM spells_new";
	db->QueryDatabase(query);

	int columns = GetSpellColumns(db);
	int spells_imported = 0;

	char buffer[2048];
	while(fgets(buffer, 2048, f)) {
		for(int i = 0; i < 2048; ++i) {
			if(buffer[i] == '\n') {
				buffer[i] = 0;
				break;
			}
		}

		std::string escaped = ::Strings::Escape(buffer);
		auto split = Strings::Split(escaped, '^');
		int line_columns = (int)split.size();

		std::string sql;
		if(line_columns >= columns) {
			sql = "INSERT INTO spells_new VALUES(";
			for(int i = 0; i < columns; ++i) {
				if(i != 0) {
					sql += ", '";
				} else {
					sql += "'";
				}

				if(split[i].compare("") == 0 && !IsStringField(i)) {
					sql += "0";
				}
				else {
					sql += split[i];
				}
				sql += "'";
			}

			sql += ");";
		} else {
			int i = 0;
			sql = "INSERT INTO spells_new VALUES(";
			for(; i < line_columns; ++i) {
				if(i != 0) {
					sql += ", '";
				} else {
					sql += "'";
				}

				if(split[i].compare("") == 0 && !IsStringField(i)) {
					sql += "0";
				} else {
					sql += split[i];
				}

				sql += "'";
			}

			for(; i < columns; ++i) {
				sql += ", '0'";
			}

			sql += ");";
		}

		db->QueryDatabase(sql);

		spells_imported++;
		if(spells_imported % 1000 == 0) {
			LogInfo("[{}] spells imported", spells_imported);
		}
	}

	if(spells_imported % 1000 != 0) {
		LogInfo("[{}] spells imported", spells_imported);
	}

	fclose(f);
}

void ImportSkillCaps(SharedDatabase *db) {
	LogInfo("Importing Skill Caps");

	std::string file = fmt::format("{}/import/SkillCaps.txt", PathManager::Instance()->GetServerPath());
	FILE *f = fopen(file.c_str(), "r");
	if(!f) {
		LogError("Unable to open {} to read, skipping.", file);
		return;
	}

	std::string delete_sql = "DELETE FROM skill_caps";
	db->QueryDatabase(delete_sql);

	char buffer[2048];
	while(fgets(buffer, 2048, f)) {
		auto split = Strings::Split(buffer, '^');

		if(split.size() < 4) {
			continue;
		}

		int class_id, skill_id, level, cap;
		class_id = Strings::ToInt(split[0].c_str());
		skill_id = Strings::ToInt(split[1].c_str());
		level = Strings::ToInt(split[2].c_str());
		cap = Strings::ToInt(split[3].c_str());

		std::string sql = StringFormat("INSERT INTO skill_caps(class, skillID, level, cap) VALUES(%d, %d, %d, %d)",
			class_id, skill_id, level, cap);

		db->QueryDatabase(sql);
	}

	fclose(f);
}

void ImportBaseData(SharedDatabase *db)
{
	LogInfo("Importing Base Data");

	const std::string& file_name = fmt::format("{}/import/BaseData.txt", PathManager::Instance()->GetServerPath());

	const auto& file_contents = File::GetContents(file_name);
	if (!file_contents.error.empty()) {
		LogError("{}", file_contents.error);
	}

	db->QueryDatabase("DELETE FROM base_data");

	std::vector<BaseDataRepository::BaseData> v;

	auto e = BaseDataRepository::NewEntity();

	for (const auto& line: Strings::Split(file_contents.contents, "\n")) {
		const auto& line_data = Strings::Split(line, '^');

		if (line_data.size() < 10) {
			continue;
		}

		e.level     = static_cast<uint8_t>(Strings::ToUnsignedInt(line_data[0]));
		e.class_    = static_cast<uint8_t>(Strings::ToUnsignedInt(line_data[1]));
		e.hp        = Strings::ToFloat(line_data[2]);
		e.mana      = Strings::ToFloat(line_data[3]);
		e.end       = Strings::ToFloat(line_data[4]);
		e.hp_regen  = Strings::ToFloat(line_data[5]);
		e.end_regen = Strings::ToFloat(line_data[6]);
		e.hp_fac    = Strings::ToFloat(line_data[7]);
		e.mana_fac  = Strings::ToFloat(line_data[8]);
		e.end_fac   = Strings::ToFloat(line_data[9]);

		v.emplace_back(e);
	}

	BaseDataRepository::InsertMany(*db, v);
}

void ImportDBStrings(SharedDatabase *db) {
	LogInfo("Importing DB Strings");

	std::string file = fmt::format("{}/import/dbstr_us.txt", PathManager::Instance()->GetServerPath());
	FILE *f = fopen(file.c_str(), "r");
	if(!f) {
		LogError("Unable to open {} to read, skipping.", file);
		return;
	}

	std::string delete_sql = "DELETE FROM db_str";
	db->QueryDatabase(delete_sql);

	char buffer[2048];
	bool first = true;
	while(fgets(buffer, 2048, f)) {
		if(first) {
			first = false;
			continue;
		}

		for(int i = 0; i < 2048; ++i) {
			if(buffer[i] == '\n') {
				buffer[i] = 0;
				break;
			}
		}

		auto split = Strings::Split(buffer, '^');

		if(split.size() < 2) {
			continue;
		}

		std::string sql;
		int id, type;
		std::string value;

		id = Strings::ToInt(split[0].c_str());
		type = Strings::ToInt(split[1].c_str());

		if(split.size() >= 3) {
			value = ::Strings::Escape(split[2]);
		}

		sql = StringFormat("INSERT INTO db_str(id, type, value) VALUES(%u, %u, '%s')",
						   id, type, value.c_str());

		db->QueryDatabase(sql);
	}

	fclose(f);
}
