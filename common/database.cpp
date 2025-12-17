/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2015 EQEMu Development Team (http://eqemulator.net)

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

#include "common/data_verification.h"
#include "common/database_schema.h"
#include "common/database.h"
#include "common/eq_packet_structs.h"
#include "common/events/player_event_logs.h"
#include "common/extprofile.h"
#include "common/global_define.h"
#include "common/http/httplib.h"
#include "common/http/uri.h"
#include "common/repositories/account_repository.h"
#include "common/repositories/adventure_stats_repository.h"
#include "common/repositories/bot_data_repository.h"
#include "common/repositories/buyer_repository.h"
#include "common/repositories/character_bind_repository.h"
#include "common/repositories/character_data_repository.h"
#include "common/repositories/character_languages_repository.h"
#include "common/repositories/character_leadership_abilities_repository.h"
#include "common/repositories/character_parcels_repository.h"
#include "common/repositories/character_pet_name_repository.h"
#include "common/repositories/character_skills_repository.h"
#include "common/repositories/data_buckets_repository.h"
#include "common/repositories/group_id_repository.h"
#include "common/repositories/group_leaders_repository.h"
#include "common/repositories/guild_members_repository.h"
#include "common/repositories/instance_list_repository.h"
#include "common/repositories/inventory_snapshots_repository.h"
#include "common/repositories/ip_exemptions_repository.h"
#include "common/repositories/merchantlist_temp_repository.h"
#include "common/repositories/name_filter_repository.h"
#include "common/repositories/npc_types_repository.h"
#include "common/repositories/raid_details_repository.h"
#include "common/repositories/raid_members_repository.h"
#include "common/repositories/reports_repository.h"
#include "common/repositories/trader_repository.h"
#include "common/repositories/variables_repository.h"
#include "common/repositories/zone_repository.h"
#include "common/rulesys.h"
#include "common/strings.h"
#include "common/zone_store.h"

#include "mysqld_error.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iomanip>
#include <iostream>
#include <map>
#include <algorithm>

// Disgrace: for windows compile
#ifdef _WINDOWS
#include <windows.h>
#define snprintf	_snprintf
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#else
#include "unix.h"
#include <netinet/in.h>
#include <sys/time.h>
#endif

extern Client client;

Database::Database() { }

Database::~Database() { }

Database::Database(
	const std::string& host,
	const std::string& user,
	const std::string& password,
	const std::string& database,
	uint32 port
)
{
	Connect(host, user, password, database, port);
}

bool Database::Connect(
	const std::string& host,
	const std::string& user,
	const std::string& password,
	const std::string& database,
	uint32 port,
	std::string connection_label
)
{
	uint32 error_number = 0;
	char   error_buffer[MYSQL_ERRMSG_SIZE];

	if (!Open(host.c_str(), user.c_str(), password.c_str(), database.c_str(), port, &error_number, error_buffer)) {
		LogError("Connection [{}] Failed to connect to database Error [{}]", connection_label, error_buffer);
		return false;
	} else {
		LogInfo("Connected to database [{}] [{}] @ [{}:{}]", connection_label, database, host, port);
		return true;
	}
}

uint32 Database::CheckLogin(
	const std::string& name,
	const std::string& password,
	const std::string& loginserver,
	int16* status
)
{
	if (name.length() >= 50 || password.length() >= 50) {
		return 0;
	}

	const auto& l = AccountRepository::GetWhere(
		*this,
		fmt::format(
			"`name` = '{}' AND `ls_id` = '{}' AND `password` IS NOT NULL AND LENGTH(`password`) > 0 AND (`password` = '{}' OR `password` = MD5('{}'))",
			Strings::Escape(name),
			Strings::Escape(loginserver),
			Strings::Escape(password),
			Strings::Escape(password)
		)
	);

	if (l.empty()) {
		return 0;
	}

	auto& e = l.front();

	if (status) {
		*status = e.status;
	}

	return e.id;
}

bool Database::CheckBannedIPs(const std::string& login_ip)
{
	const std::string& query = fmt::format(
		"SELECT `ip_address` FROM `banned_ips` WHERE `ip_address` = '{}'",
		Strings::Escape(login_ip)
	);

	auto results = QueryDatabase(query);
	if (!results.Success() || results.RowCount()) {
		return true;
	}

	return false;
}

bool Database::AddBannedIP(const std::string& banned_ip, const std::string& notes)
{
	auto query = fmt::format(
		"INSERT INTO banned_ips SET `ip_address` = '{}', `notes` = '{}'",
		Strings::Escape(banned_ip),
		Strings::Escape(notes)
	);

	auto results = QueryDatabase(query);

	return results.Success();
}

bool Database::CheckGMIPs(const std::string& login_ip, uint32 account_id)
{
	auto query = fmt::format(
		"SELECT `name`, `account_id`, `ip_address` FROM `gm_ips` WHERE `ip_address` = '{}' AND `account_id` = {}",
		Strings::Escape(login_ip),
		account_id
	);

	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	return results.RowCount();
}

void Database::LoginIP(uint32 account_id, const std::string& login_ip)
{
	auto query = fmt::format(
		"INSERT INTO account_ip SET `accid` = {}, `ip` = '{}' ON DUPLICATE KEY UPDATE count = (count + 1), lastused = NOW()",
		account_id,
		Strings::Escape(login_ip)
	);

	QueryDatabase(query);
}

int16 Database::GetAccountStatus(uint32 account_id)
{
	auto e = AccountRepository::FindOne(*this, account_id);

	if (e.suspendeduntil > 0 && e.suspendeduntil < std::time(nullptr)) {
		e.status         = 0;
		e.suspendeduntil = 0;
		e.suspend_reason = "";

		AccountRepository::UpdateOne(*this, e);
	}

	return e.status;
}

uint32 Database::CreateAccount(
	const std::string& name,
	const std::string& password,
	int16 status,
	const std::string& loginserver,
	uint32 lsaccount_id
)
{
	auto e = AccountRepository::NewEntity();

	e.name          = name;
	e.status        = status;
	e.ls_id         = loginserver;
	e.lsaccount_id  = lsaccount_id;
	e.time_creation = std::time(nullptr);

	if (!password.empty()) {
		e.password = password;
	}

	LogInfo("Account attempting to be created loginserver [{}] name [{}] status [{}]", loginserver, name, status);

	e = AccountRepository::InsertOne(*this, e);

	if (!e.id) {
		return 0;
	}

	return e.id;
}

bool Database::SetLocalPassword(uint32 account_id, const std::string& password)
{
	return AccountRepository::UpdatePassword(*this, account_id, password);
}

bool Database::SetAccountStatus(const std::string& account_name, int16 status)
{
	LogInfo("Account [{}] is attempting to be set to status [{}]", account_name, status);

	auto l = AccountRepository::GetWhere(
		*this,
		fmt::format(
			"`name` = '{}'",
			Strings::Escape(account_name)
		)
	);

	if (l.empty()) {
		return false;
	}

	auto& e = l.front();

	e.status = status;

	return AccountRepository::UpdateOne(*this, e);
}

bool Database::ReserveName(uint32 account_id, const std::string& name)
{
	const std::string& where_filter = fmt::format(
		"`name` = '{}'",
		Strings::Escape(name)
	);

	if (RuleB(Bots, Enabled)) {
		const auto& b = BotDataRepository::GetWhere(*this, where_filter);

		if (!b.empty()) {
			LogInfo("Account [{}] requested name [{}] but name is already taken by a bot", account_id, name);
			return false;
		}
	}

	const auto& c = CharacterDataRepository::GetWhere(*this, where_filter);

	if (!c.empty()) {
		LogInfo("Account [{}] requested name [{}] but name is already taken by a character", account_id, name);
		return false;
	}

	const auto& n = NpcTypesRepository::GetWhere(*this, where_filter);

	if (!n.empty()) {
		LogInfo("Account [{}] requested name [{}] but name is already taken by an NPC", account_id, name);
		return false;
	}

	const auto& p = CharacterPetNameRepository::GetWhere(*this, where_filter);
	if (!p.empty()) {
		LogInfo("Account [{}] requested name [{}] but name is already taken by an Pet", account_id, name);
		return false;
	}

	auto e = CharacterDataRepository::NewEntity();

	e.account_id = account_id;
	e.name       = name;

	e = CharacterDataRepository::InsertOne(*this, e);

	if (!e.id) {
		return false;
	}

	const uint32 guild_id   = RuleI(Character, DefaultGuild);
	const uint8  guild_rank = EQ::Clamp(RuleI(Character, DefaultGuildRank), 0, 8);
	if (guild_id != 0) {
		if (e.id) {
			auto g = GuildMembersRepository::NewEntity();

			g.char_id  = e.id;
			g.guild_id = guild_id;
			g.rank_    = guild_rank;

			GuildMembersRepository::InsertOne(*this, g);
		}
	}

	return true;
}

bool Database::DeleteCharacter(const std::string& name)
{
	if (name.empty()) {
		LogInfo("Request to delete without a name.");
		return false;
	}

	const auto& l = CharacterDataRepository::GetWhere(
		*this,
		fmt::format(
			"`name` = '{}'",
			Strings::Escape(name)
		)
	);

	if (l.empty()) {
		return false;
	}

	auto& e = l.front();

	if (!e.id) {
		return false;
	}

	std::string delete_type = "hard-deleted";
	if (RuleB(Character, SoftDeletes)) {
		delete_type = "soft-deleted";

		std::string query = fmt::format(
			SQL(
				UPDATE character_data SET name = SUBSTRING(CONCAT(name, '-deleted-', UNIX_TIMESTAMP()), 1, 64), deleted_at = NOW()
				WHERE id = {}
			),
			e.id
		);

		QueryDatabase(query);

		if (RuleB(Bots, Enabled)) {
			query = fmt::format(
				SQL(
					UPDATE bot_data SET name = SUBSTRING(CONCAT(name, '-deleted-', UNIX_TIMESTAMP()), 1, 64) WHERE owner_id = {}
				),
				e.id
			);

			QueryDatabase(query);

			LogInfo(
				"[DeleteCharacter] character_name [{}] ({}) bots are being [{}]",
				e.name,
				e.id,
				delete_type
			);
		}

		return true;
	}

	for (const auto &t : DatabaseSchema::GetCharacterTables()) {
		const std::string& table_name               = t.first;
		const std::string& character_id_column_name = t.second;

		QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				table_name,
				character_id_column_name,
				e.id
			)
		);
	}

	LogInfo("character_name [{}] ({}) is being [{}]", e.name, e.id, delete_type);

	return true;
}

bool Database::SaveCharacterCreate(uint32 character_id, uint32 account_id, PlayerProfile_Struct *pp)
{
	auto c = CharacterDataRepository::NewEntity();

	c.id                      = character_id;
	c.account_id              = account_id;
	c.name                    = pp->name;
	c.last_name               = pp->last_name;
	c.gender                  = pp->gender;
	c.race                    = pp->race;
	c.class_                  = pp->class_;
	c.level                   = pp->level;
	c.deity                   = pp->deity;
	c.birthday                = pp->birthday;
	c.last_login              = pp->lastlogin;
	c.time_played             = pp->timePlayedMin;
	c.pvp_status              = pp->pvp;
	c.level2                  = pp->level2;
	c.anon                    = pp->anon;
	c.gm                      = pp->gm;
	c.intoxication            = pp->intoxication;
	c.hair_color              = pp->haircolor;
	c.beard_color             = pp->beardcolor;
	c.eye_color_1             = pp->eyecolor1;
	c.eye_color_2             = pp->eyecolor2;
	c.hair_style              = pp->hairstyle;
	c.beard                   = pp->beard;
	c.ability_time_seconds    = pp->ability_time_seconds;
	c.ability_number          = pp->ability_number;
	c.ability_time_minutes    = pp->ability_time_minutes;
	c.ability_time_hours      = pp->ability_time_hours;
	c.title                   = pp->title;
	c.suffix                  = pp->suffix;
	c.exp                     = pp->exp;
	c.points                  = pp->points;
	c.mana                    = pp->mana;
	c.cur_hp                  = pp->cur_hp;
	c.str                     = pp->STR;
	c.sta                     = pp->STA;
	c.cha                     = pp->CHA;
	c.dex                     = pp->DEX;
	c.int_                    = pp->INT;
	c.agi                     = pp->AGI;
	c.wis                     = pp->WIS;
	c.face                    = pp->face;
	c.y                       = pp->y;
	c.x                       = pp->x;
	c.z                       = pp->z;
	c.heading                 = pp->heading;
	c.pvp2                    = pp->pvp2;
	c.pvp_type                = pp->pvptype;
	c.autosplit_enabled       = pp->autosplit;
	c.zone_change_count       = pp->zone_change_count;
	c.drakkin_heritage        = pp->drakkin_heritage;
	c.drakkin_tattoo          = pp->drakkin_tattoo;
	c.drakkin_details         = pp->drakkin_details;
	c.toxicity                = pp->toxicity;
	c.hunger_level            = pp->hunger_level;
	c.thirst_level            = pp->thirst_level;
	c.ability_up              = pp->ability_up;
	c.zone_id                 = pp->zone_id;
	c.zone_instance           = pp->zoneInstance;
	c.leadership_exp_on       = pp->leadAAActive;
	c.ldon_points_guk         = pp->ldon_points_guk;
	c.ldon_points_mir         = pp->ldon_points_mir;
	c.ldon_points_mmc         = pp->ldon_points_mmc;
	c.ldon_points_ruj         = pp->ldon_points_ruj;
	c.ldon_points_tak         = pp->ldon_points_tak;
	c.ldon_points_available   = pp->ldon_points_available;
	c.tribute_time_remaining  = pp->tribute_time_remaining;
	c.show_helm               = pp->showhelm;
	c.career_tribute_points   = pp->career_tribute_points;
	c.tribute_points          = pp->tribute_points;
	c.tribute_active          = pp->tribute_active;
	c.endurance               = pp->endurance;
	c.group_leadership_exp    = pp->group_leadership_exp;
	c.raid_leadership_exp     = pp->raid_leadership_exp;
	c.group_leadership_points = pp->group_leadership_points;
	c.raid_leadership_points  = pp->raid_leadership_points;
	c.air_remaining           = pp->air_remaining;
	c.pvp_kills               = pp->PVPKills;
	c.pvp_deaths              = pp->PVPDeaths;
	c.pvp_current_points      = pp->PVPCurrentPoints;
	c.pvp_career_points       = pp->PVPCareerPoints;
	c.pvp_best_kill_streak    = pp->PVPBestKillStreak;
	c.pvp_worst_death_streak  = pp->PVPWorstDeathStreak;
	c.pvp_current_kill_streak = pp->PVPCurrentKillStreak;
	c.aa_points_spent         = pp->aapoints_spent;
	c.aa_exp                  = pp->expAA;
	c.aa_points               = pp->aapoints;
	c.group_auto_consent      = pp->groupAutoconsent;
	c.raid_auto_consent       = pp->raidAutoconsent;
	c.guild_auto_consent      = pp->guildAutoconsent;
	c.RestTimer               = pp->RestTimer;

	CharacterDataRepository::ReplaceOne(*this, c);

	std::vector<CharacterBindRepository::CharacterBind> character_binds;

	character_binds.reserve(5);

	auto b = CharacterBindRepository::NewEntity();

	b.id = character_id;

	for (uint8 slot_id = 0; slot_id < 5; slot_id++) {
		b.zone_id     = pp->binds[slot_id].zone_id;
		b.x           = pp->binds[slot_id].x;
		b.y           = pp->binds[slot_id].y;
		b.z           = pp->binds[slot_id].z;
		b.heading     = pp->binds[slot_id].heading;
		b.slot        = slot_id;

		character_binds.emplace_back(b);
	}

	CharacterBindRepository::ReplaceMany(*this, character_binds);

	if (RuleB(Character, GrantHoTTOnCreate)) {
		CharacterLeadershipAbilitiesRepository::InsertOne(
			*this,
			CharacterLeadershipAbilitiesRepository::CharacterLeadershipAbilities{
				.id = character_id,
				.slot = LeadershipAbilitySlot::HealthOfTargetsTarget,
				.rank_ = 1
			}
		);
	}

	std::vector<CharacterSkillsRepository::CharacterSkills> character_skills;

	character_skills.reserve(MAX_PP_SKILL);

	for (uint16 slot_id = 0; slot_id < MAX_PP_SKILL; slot_id++) {
		character_skills.emplace_back(
			CharacterSkillsRepository::CharacterSkills{
				.id = character_id,
				.skill_id = slot_id,
				.value = static_cast<uint16_t>(pp->skills[slot_id])
			}
		);
	}

	CharacterSkillsRepository::ReplaceMany(*this, character_skills);

	std::vector<CharacterLanguagesRepository::CharacterLanguages> character_languages;

	character_languages.reserve(MAX_PP_LANGUAGE);

	for (uint16 slot_id = 0; slot_id < MAX_PP_LANGUAGE; slot_id++) {
		character_languages.emplace_back(
			CharacterLanguagesRepository::CharacterLanguages{
				.id = character_id,
				.lang_id = slot_id,
				.value = static_cast<uint16_t>(pp->languages[slot_id])
			}
		);
	}

	CharacterLanguagesRepository::ReplaceMany(*this, character_languages);

	return true;
}

uint32 Database::GetCharacterID(const std::string& name)
{
	const auto& l = CharacterDataRepository::GetWhere(
		*this,
		fmt::format(
			"`name` = '{}'",
			Strings::Escape(name)
		)
	);

	if (l.empty()) {
		return 0;
	}

	auto& e = l.front();

	return e.id;
}

uint32 Database::GetAccountIDByChar(const std::string& name, uint32* character_id)
{
	const auto& l = CharacterDataRepository::GetWhere(
		*this,
		fmt::format(
			"`name` = '{}'",
			Strings::Escape(name)
		)
	);

	if (l.empty()) {
		return 0;
	}

	auto& e = l.front();

	if (character_id) {
		*character_id = e.id;
	}

	return e.account_id;
}

uint32 Database::GetAccountIDByChar(uint32 character_id)
{
	const auto& e = CharacterDataRepository::FindOne(*this, character_id);

	return e.id ? e.account_id : 0;
}

uint32 Database::GetAccountIDByName(const std::string& account_name, const std::string& loginserver, int16* status, uint32* lsaccount_id)
{
	if (!isAlphaNumeric(account_name.c_str())) {
		return 0;
	}

	const auto& l = AccountRepository::GetWhere(
		*this,
		fmt::format(
			"`name` = '{}' AND `ls_id` = '{}' LIMIT 1",
			Strings::Escape(account_name),
			Strings::Escape(loginserver)
		)
	);

	if (l.empty()) {
		return 0;
	}

	auto& e = l.front();

	if (status) {
		*status = e.status;
	}

	if (lsaccount_id) {
		*lsaccount_id = e.lsaccount_id;
	}

	return e.id;
}

const std::string Database::GetAccountName(uint32 account_id, uint32* lsaccount_id)
{
	const auto& e = AccountRepository::FindOne(*this, account_id);

	if (!e.id) {
		return std::string();
	}

	if (lsaccount_id) {
		*lsaccount_id = e.lsaccount_id;
	}

	return e.name;
}

const std::string Database::GetCharName(uint32 character_id)
{
	const auto& e = CharacterDataRepository::FindOne(*this, character_id);

	return e.id ? e.name : std::string();
}

const std::string Database::GetCharNameByID(uint32 character_id)
{
	const auto& e = CharacterDataRepository::FindOne(*this, character_id);

	return e.id ? e.name : std::string();
}

const std::string Database::GetNPCNameByID(uint32 npc_id)
{
	const auto& e = NpcTypesRepository::FindOne(*this, npc_id);

	return e.id ? e.name : std::string();
}

template<typename InputIterator, typename OutputIterator>
inline auto CleanMobName(InputIterator first, InputIterator last, OutputIterator result)
{
	for (; first != last; ++first) {
		if (*first == '_') {
			*result = ' ';
		}
		else if (isalpha(*first) || *first == '`') {
			*result = *first;
		}
	}
	return result;
}

const std::string Database::GetCleanNPCNameByID(uint32 npc_id)
{
	const auto& e = NpcTypesRepository::FindOne(*this, npc_id);

	if (!e.id) {
		return std::string();
	}

	std::string mob_name = e.name;
	std::string result;

	CleanMobName(mob_name.begin(), mob_name.end(), std::back_inserter(result));

	return result;
}

bool Database::LoadVariables()
{
	const auto& l = VariablesRepository::GetWhere(
		*this,
		fmt::format(
			"UNIX_TIMESTAMP(`ts`) >= {}",
			varcache.last_update
		)
	);

	if (l.empty()) {
		return true;
	}

	LockMutex lock(&Mvarcache);

	for (const auto& e : l) {
		varcache.last_update = std::time(nullptr);

		varcache.Add(Strings::ToLower(e.varname), e.value);
	}

	LogInfo(
		"Loaded [{}] Variable{}",
		Strings::Commify(l.size()),
		l.size() != 1 ? "s" : ""
	);

	return true;
}

bool Database::GetVariable(const std::string& name, std::string& value)
{
	LockMutex lock(&Mvarcache);

	if (name.empty()) {
		return false;
	}

	auto v = varcache.Get(Strings::ToLower(name));

	if (v) {
		value = *v;
		return true;
	}

	return false;
}

bool Database::SetVariable(const std::string& name, const std::string& value)
{
	auto l = VariablesRepository::GetWhere(
		*this,
		fmt::format(
			"`varname` = '{}'",
			Strings::Escape(name)
		)
	);

	if (l.empty()) {
		return false;
	}

	auto& variable = l.front();

	variable.value = value;

	const int updated = VariablesRepository::UpdateOne(*this, variable);

	if (updated) {
		LoadVariables();

		return true;
	}

	auto e = VariablesRepository::NewEntity();

	e.varname = name;
	e.value   = value;

	e = VariablesRepository::InsertOne(*this, e);

	if (!e.id) {
		return false;
	}

	LoadVariables();

	return true;
}

void Database::SetAccountCRCField(uint32 account_id, const std::string& field_name, uint64 checksum)
{
	auto e = AccountRepository::FindOne(*this, account_id);

	if (Strings::EqualFold(field_name, "crc_basedata")) {
		e.crc_basedata = checksum;
	} else if (Strings::EqualFold(field_name, "crc_eqgame")) {
		e.crc_eqgame = checksum;
	} else if (Strings::EqualFold(field_name, "crc_skillcaps")) {
		e.crc_skillcaps = checksum;
	}

	AccountRepository::UpdateOne(*this, e);
}

uint8 Database::GetPEQZone(uint32 zone_id, uint32 version)
{
	auto z = GetZoneVersionWithFallback(zone_id, version);

	return z ? z->peqzone : 0;
}

bool Database::CheckNameFilter(const std::string& name, bool surname)
{
	// the minimum 4 is enforced by the client too
	if (name.empty() || name.size() < 4) {
		return false;
	}

	// Given name length is enforced by the client too
	if (!surname && name.size() > 15) {
		return false;
	}

	for (size_t i = 0; i < name.size(); i++) {
		if (!isalpha(name[i])) {
			return false;
		}
	}

	char c = '\0';
	uint8 num_c = 0;
	for (size_t x = 0; x < name.size(); ++x) {
		if (name[x] == c) {
			num_c++;
		} else {
			num_c = 1;
			c = name[x];
		}

		if (num_c > 2) {
			return false;
		}
	}

	const auto& l = NameFilterRepository::All(*this);

	if (l.empty()) {
		return true;
	}

	for (const auto& e : l) {
		if (Strings::Contains(Strings::ToLower(name), Strings::ToLower(e.name))) {
			return false;
		}
	}

	return true;
}

bool Database::AddToNameFilter(const std::string& name)
{
	auto e = NameFilterRepository::NewEntity();

	e.name = name;

	return NameFilterRepository::InsertOne(*this, e).id;
}

uint32 Database::GetAccountIDFromLSID(
	const std::string& in_loginserver_id,
	uint32 in_loginserver_account_id,
	char* in_account_name,
	int16* in_status
)
{
	const auto& l = AccountRepository::GetWhere(
		*this,
		fmt::format(
			"`lsaccount_id` = {} AND `ls_id` = '{}'",
			in_loginserver_account_id,
			in_loginserver_id
		)
	);

	if (l.empty()) {
		return 0;
	}

	auto& e = l.front();

	if (in_account_name) {
		strcpy(in_account_name, e.name.c_str());
	}

	if (in_status) {
		*in_status = e.status;
	}

	return e.id;
}

void Database::ClearMerchantTemp()
{
	MerchantlistTempRepository::ClearTemporaryMerchantLists(*this);
}

bool Database::UpdateName(const std::string& old_name, const std::string& new_name)
{
	LogInfo("Renaming [{}] to [{}]", old_name, new_name);

	auto l = CharacterDataRepository::GetWhere(
		*this,
		fmt::format(
			"`name` = '{}'",
			Strings::Escape(old_name)
		)
	);

	if (l.empty()) {
		return false;
	}

	auto& e = l.front();

	e.name = new_name;

	return CharacterDataRepository::UpdateOne(*this, e);
}

bool Database::UpdateNameByID(const int character_id, const std::string& new_name)
{
	LogInfo("Renaming [{}] to [{}]", character_id, new_name);

	auto l = CharacterDataRepository::GetWhere(
		*this,
		fmt::format(
			"`id` = {}",
			character_id
		)
	);

	if (l.empty()) {
		return false;
	}

	auto& e = l.front();

	e.name = new_name;

	return CharacterDataRepository::UpdateOne(*this, e);
}

bool Database::IsNameUsed(const std::string& name)
{
	if (RuleB(Bots, Enabled)) {
		const auto& bot_data = BotDataRepository::GetWhere(
			*this,
			fmt::format(
				"`name` = '{}'",
				Strings::Escape(name)
			)
		);

		if (!bot_data.empty()) {
			return true;
		}
	}

	const auto& character_data = CharacterDataRepository::GetWhere(
		*this,
		fmt::format(
			"`name` = '{}'",
			Strings::Escape(name)
		)
	);

	return !character_data.empty();
}

// Players cannot have the same name as a pet vanity name, or memory corruption occurs.
bool Database::IsPetNameUsed(const std::string& name)
{
	const auto& pet_name_data = CharacterPetNameRepository::GetWhere(
		*this,
		fmt::format(
			"`name` = '{}'",
			Strings::Escape(name)
		)
	);

	return !pet_name_data.empty();
}

uint32 Database::GetServerType()
{
	const auto& l = VariablesRepository::GetWhere(*this, "`varname` = 'ServerType' LIMIT 1");

	if (l.empty()) {
		return 0;
	}

	auto& e = l.front();

	return Strings::ToUnsignedInt(e.value);
}

bool Database::MoveCharacterToZone(uint32 character_id, uint32 zone_id)
{
	auto e = CharacterDataRepository::FindOne(*this, character_id);

	e.zone_id = zone_id;

	e.x = e.y = e.z = -1;

	return CharacterDataRepository::UpdateOne(*this, e);
}

bool Database::MoveCharacterToZone(const std::string& name, uint32 zone_id)
{
	auto l = CharacterDataRepository::GetWhere(
		*this,
		fmt::format(
			"`name` = '{}'",
			Strings::Escape(name)
		)
	);

	if (l.empty()) {
		return false;
	}

	auto& e = l.front();

	e.zone_id = zone_id;

	e.x = e.y = e.z = -1;

	return CharacterDataRepository::UpdateOne(*this, e);
}

bool Database::UpdateLiveChar(const std::string& name, uint32 account_id)
{
	auto e = AccountRepository::FindOne(*this, account_id);

	if (!e.id) {
		return false;
	}

	e.charname = name;

	return AccountRepository::UpdateOne(*this, e);
}

const std::string Database::GetLiveChar(uint32 account_id)
{
	auto e = AccountRepository::FindOne(*this, account_id);

	return e.id ? e.charname : std::string();
}

void Database::SetLFP(uint32 character_id, bool is_lfp)
{
	auto e = CharacterDataRepository::FindOne(*this, character_id);

	e.lfp = is_lfp ? 1 : 0;

	CharacterDataRepository::UpdateOne(*this, e);
}

void Database::SetLoginFlags(uint32 character_id, bool is_lfp, bool is_lfg, uint8 ingame)
{
	auto e = CharacterDataRepository::FindOne(*this, character_id);

	e.ingame = ingame;
	e.lfg    = is_lfg ? 1 : 0;
	e.lfp    = is_lfp ? 1 : 0;

	CharacterDataRepository::UpdateOne(*this, e);
}

void Database::SetLFG(uint32 character_id, bool is_lfg)
{
	auto e = CharacterDataRepository::FindOne(*this, character_id);

	e.lfg = is_lfg ? 1 : 0;

	CharacterDataRepository::UpdateOne(*this, e);
}

void Database::SetIngame(uint32 character_id, uint8 ingame)
{
	auto e = CharacterDataRepository::FindOne(*this, character_id);

	e.ingame = ingame;

	CharacterDataRepository::UpdateOne(*this, e);
}

void Database::AddReport(const std::string& who, const std::string& against, const std::string& lines)
{
	auto e = ReportsRepository::NewEntity();

	e.name          = Strings::Escape(who);
	e.reported      = Strings::Escape(against);
	e.reported_text = Strings::Escape(lines);

	ReportsRepository::InsertOne(*this, e);
}

void Database::ClearAllGroups()
{
	GroupIdRepository::ClearAllGroups(*this);
}

void Database::ClearGroup(uint32 group_id) {
	ClearGroupLeader(group_id);

	if (!group_id) {
		//clear all groups
		ClearAllGroups();
		return;
	}

	//clear a specific group
	GroupIdRepository::DeleteWhere(
		*this,
		fmt::format(
			"`group_id` = {}",
			group_id
		)
	);
}

uint32 Database::GetGroupID(const std::string& name)
{
	const auto& l = GroupIdRepository::GetWhere(
		*this,
		fmt::format(
			"`name` = '{}'",
			Strings::Escape(name)
		)
	);

	if (l.empty()) {
		return 0;
	}

	auto& e = l.front();

	return e.group_id;
}

std::string Database::GetGroupLeaderForLogin(const std::string& character_name)
{
	const auto& g = GroupIdRepository::GetWhere(
		*this,
		fmt::format(
			"`name` = '{}'",
			Strings::Escape(character_name)
		)
	);

	if (g.empty()) {
		return std::string();
	}

	auto& group = g.front();

	const uint32 group_id = group.group_id;

	const auto& e = GroupLeadersRepository::FindOne(*this, group_id);

	return e.gid ? e.leadername : std::string();
}

void Database::SetGroupLeaderName(uint32 group_id, const std::string &name)
{
    auto e       = GroupLeadersRepository::FindOne(*this, group_id);

    e.leadername = name;

    if (e.gid) {
        GroupLeadersRepository::UpdateOne(*this, e);
        return;
    }

    e.gid            = group_id;
    e.marknpc        = std::string();
    e.leadershipaa   = std::string();
    e.maintank       = std::string();
    e.assist         = std::string();
    e.puller         = std::string();
    e.mentoree       = std::string();
    e.mentor_percent = 0;

    GroupLeadersRepository::ReplaceOne(*this, e);
}

std::string Database::GetGroupLeaderName(uint32 group_id)
{
	const std::string& query = fmt::format(
		"SELECT `leadername` FROM `group_leaders` WHERE `gid` = {}",
		group_id
	);

	auto results = QueryDatabase(query);

	if (!results.Success() || !results.RowCount()) {
		return std::string();
	}

	auto& row = results.begin();

	return row[0];
}

char* Database::GetGroupLeadershipInfo(
	uint32 group_id,
	char* leaderbuf,
	char* maintank,
	char* assist,
	char* puller,
	char* marknpc,
	char* mentoree,
	int* mentor_percent,
	GroupLeadershipAA_Struct* GLAA
)
{
	auto e = GroupLeadersRepository::FindOne(*this, group_id);

	if (!e.gid) {
		if (leaderbuf) {
			strcpy(leaderbuf, "UNKNOWN");
		}

		if (maintank) {
			maintank[0] = '\0';
		}

		if (assist) {
			assist[0] = '\0';
		}

		if (puller) {
			puller[0] = '\0';
		}

		if (marknpc) {
			marknpc[0] = '\0';
		}

		if (mentoree) {
			mentoree[0] = '\0';
		}

		if (mentor_percent) {
			*mentor_percent = 0;
		}

		return leaderbuf;
	}

	if (leaderbuf) {
		strcpy(leaderbuf, e.leadername.c_str());
	}

	if (maintank) {
		strcpy(maintank, e.maintank.c_str());
	}

	if (assist) {
		strcpy(assist, e.assist.c_str());
	}

	if (puller) {
		strcpy(puller, e.puller.c_str());
	}

	if (marknpc) {
		strcpy(marknpc, e.marknpc.c_str());
	}

	if (mentoree) {
		strcpy(mentoree, e.mentoree.c_str());
	}

	if (mentor_percent) {
		*mentor_percent = e.mentor_percent;
	}
	if(GLAA && e.leadershipaa.length() == sizeof(GroupLeadershipAA_Struct)) {
		Decode(e.leadershipaa);
		memcpy(GLAA, e.leadershipaa.data(), sizeof(GroupLeadershipAA_Struct));
	}

	return leaderbuf;
}

void Database::ClearAllGroupLeaders()
{
	GroupLeadersRepository::ClearAllGroupLeaders(*this);
}

void Database::ClearGroupLeader(uint32 group_id)
{
	if (!group_id) {
		ClearAllGroupLeaders();
		return;
	}

	GroupLeadersRepository::DeleteOne(*this, group_id);
}

uint8 Database::GetAgreementFlag(uint32 account_id)
{
	const auto& e = AccountRepository::FindOne(*this, account_id);

	return !e.id ? 0 : e.rulesflag;
}

void Database::SetAgreementFlag(uint32 account_id)
{
	auto e = AccountRepository::FindOne(*this, account_id);

	e.rulesflag = 1;

	AccountRepository::UpdateOne(*this, e);
}

void Database::ClearRaid(uint32 raid_id)
{
	if (!raid_id) {
		ClearAllRaids();
		return;
	}

	RaidMembersRepository::DeleteWhere(
		*this,
		fmt::format(
			"`raidid` = {}",
			raid_id
		)
	);
}

void Database::ClearAllRaids()
{
	RaidMembersRepository::ClearAllRaids(*this);
}

void Database::ClearAllRaidDetails()
{
	RaidDetailsRepository::ClearAllRaidDetails(*this);
}

void Database::ClearRaidDetails(uint32 raid_id) {

	if (!raid_id) {
		ClearAllRaidDetails();
		return;
	}

	RaidDetailsRepository::DeleteWhere(
		*this,
		fmt::format(
			"`raidid` = {}",
			raid_id
		)
	);
}

void Database::PurgeAllDeletedDataBuckets()
{
	DataBucketsRepository::DeleteWhere(
		*this,
		fmt::format(
			"(`expires` < {} AND `expires` > 0)",
			std::time(nullptr)
		)
	);
}

uint32 Database::GetRaidID(const std::string& name)
{
	const auto& l = RaidMembersRepository::GetWhere(
		*this,
		fmt::format(
			"`name` = '{}'",
			Strings::Escape(name)
		)
	);

	if (l.empty()) {
		return 0;
	}

	auto& e = l.front();

	return e.raidid;
}

const std::string Database::GetRaidLeaderName(uint32 raid_id)
{
	const auto& l = RaidMembersRepository::GetWhere(
		*this,
		fmt::format(
			"`raidid` = {} AND `israidleader` = 1",
			raid_id
		)
	);

	if (l.empty()) {
		LogDebug("Unable to get Raid Leader Name for Raid ID [{}]", raid_id);
		return "UNKNOWN";
	}

	auto& e = l.front();

	return e.name;
}

void Database::GetGroupLeadershipInfo(
	uint32 group_id,
	uint32 raid_id,
	char* maintank,
	char* assist,
	char* puller,
	char* marknpc,
	char* mentoree,
	int* mentor_percent,
	GroupLeadershipAA_Struct* GLAA
)
{
	const std::string& query = fmt::format(
		SQL(
			SELECT `maintank`, `assist`, `puller`, `marknpc`, `mentoree`, `mentor_percent`, `leadershipaa`
			FROM `raid_leaders`
			WHERE `gid` = {} AND `rid` = {}
		),
		group_id,
		raid_id
	);

	auto results = QueryDatabase(query);

	if (!results.Success() || !results.RowCount()) {
		if (maintank) {
			maintank[0] = '\0';
		}

		if (assist) {
			assist[0] = '\0';
		}

		if (puller) {
			puller[0] = '\0';
		}

		if (marknpc) {
			marknpc[0] = '\0';
		}

		if (mentoree) {
			mentoree[0] = '\0';
		}

		if (mentor_percent) {
			*mentor_percent = 0;
		}

		return;
	}

	auto& row = results.begin();

	if (maintank) {
		strcpy(maintank, row[0]);
	}

	if (assist) {
		strcpy(assist, row[1]);
	}

	if (puller) {
		strcpy(puller, row[2]);
	}

	if (marknpc) {
		strcpy(marknpc, row[3]);
	}

	if (mentoree) {
		strcpy(mentoree, row[4]);
	}

	if (mentor_percent) {
		*mentor_percent = Strings::ToInt(row[5]);
	}

	if (GLAA && results.LengthOfColumn(6) == sizeof(GroupLeadershipAA_Struct)) {
		memcpy(GLAA, row[6], sizeof(GroupLeadershipAA_Struct));
	}
}

void Database::GetRaidLeadershipInfo(
	uint32 raid_id,
	char* maintank,
	char* assist,
	char* puller,
	char* marknpc,
	RaidLeadershipAA_Struct* RLAA
)
{
	std::string query = fmt::format(
		SQL(
			SELECT `maintank`, `assist`, `puller`, `marknpc`, `leadershipaa`
			FROM `raid_leaders`
			WHERE `gid` = {} AND `rid` = {}
		),
		std::numeric_limits<uint32>::max(),
		raid_id
	);

	auto results = QueryDatabase(query);

	if (!results.Success() || !results.RowCount()) {
		if (maintank) {
			maintank[0] = '\0';
		}

		if (assist) {
			assist[0] = '\0';
		}

		if (puller) {
			puller[0] = '\0';
		}

		if (marknpc) {
			marknpc[0] = '\0';
		}

		return;
	}

	auto& row = results.begin();

	if (maintank) {
		strcpy(maintank, row[0]);
	}

	if (assist) {
		strcpy(assist, row[1]);
	}

	if (puller) {
		strcpy(puller, row[2]);
	}

	if (marknpc) {
		strcpy(marknpc, row[3]);
	}

	if (RLAA && results.LengthOfColumn(4) == sizeof(RaidLeadershipAA_Struct)) {
		memcpy(RLAA, row[4], sizeof(RaidLeadershipAA_Struct));
	}
}

void Database::SetRaidGroupLeaderInfo(uint32 group_id, uint32 raid_id)
{
	std::string query = fmt::format(
		"UPDATE `raid_leaders` SET `leadershipaa` = '' WHERE `gid` = {} AND `rid` = {}",
		group_id,
		raid_id
	);

	auto results = QueryDatabase(query);

	if (results.RowsAffected() != 0) {
		return;
	}

	query = fmt::format(
		SQL(
			REPLACE INTO `raid_leaders` (`gid`, `rid`, `marknpc`, `leadershipaa`, `maintank`, `assist`, `puller`, `mentoree`, `mentor_percent`) VALUES
			({}, {}, '', '', '', '', '', '', 0)
		),
		group_id,
		raid_id
	);

	QueryDatabase(query);
}

void Database::ClearAllRaidLeaders()
{
	QueryDatabase("DELETE FROM `raid_leaders`");
}

void Database::ClearRaidLeader(uint32 group_id, uint32 raid_id)
{
	if (!raid_id) {
		ClearAllRaidLeaders();
		return;
	}

	const std::string& query = fmt::format(
		"DELETE FROM `raid_leaders` WHERE `gid` = {} AND `rid` = {}",
		group_id,
		raid_id
	);

	QueryDatabase(query);
}

void Database::UpdateAdventureStatsEntry(uint32 character_id, uint8 theme_id, bool is_win, bool is_remove)
{
	AdventureStatsRepository::UpdateAdventureStatsEntry(*this, character_id, theme_id, is_win, is_remove);
}

bool Database::GetAdventureStats(uint32 character_id, AdventureStats_Struct* as)
{
	const auto& e = AdventureStatsRepository::FindOne(*this, character_id);

	if (!e.player_id) {
		return false;
	}

	as->success.guk = e.guk_wins;
	as->failure.guk = e.guk_losses;
	as->success.mir = e.mir_wins;
	as->failure.mir = e.mir_losses;
	as->success.mmc = e.mmc_wins;
	as->failure.mmc = e.mmc_losses;
	as->success.ruj = e.ruj_wins;
	as->failure.ruj = e.ruj_losses;
	as->success.tak = e.tak_wins;
	as->failure.tak = e.tak_losses;

	as->failure.total = (
		as->failure.guk +
		as->failure.mir +
		as->failure.mmc +
		as->failure.ruj +
		as->failure.tak
	);

	as->success.total = (
		as->success.guk +
		as->success.mir +
		as->success.mmc +
		as->success.ruj +
		as->success.tak
	);

	return true;
}

uint32 Database::GetGuildIDByCharID(uint32 character_id)
{
	const auto& e = GuildMembersRepository::FindOne(*this, character_id);

	return e.char_id ? e.guild_id : 0;
}

uint32 Database::GetGroupIDByCharID(uint32 character_id)
{
	const auto& e = GroupIdRepository::GetWhere(
		*this,
		fmt::format(
			"`character_id` = {}",
			character_id
		)
	);

	return e.size() == 1 ? e.front().group_id : 0;
}

uint32 Database::GetRaidIDByCharID(uint32 character_id)
{

	const auto& e = RaidMembersRepository::GetWhere(
		*this,
		fmt::format(
			"`charid` = {}",
			character_id
		)
	);

	return e.size() == 1 ? e.front().raidid : 0;
}

int64 Database::CountInvSnapshots()
{
	return InventorySnapshotsRepository::CountInventorySnapshots(*this);
}

void Database::ClearInvSnapshots(bool from_now)
{
	uint32 delete_time = time(nullptr);

	if (!from_now) {
		delete_time -= RuleI(Character, InvSnapshotHistoryD) * 86400;
	}

	InventorySnapshotsRepository::DeleteWhere(
		*this,
		fmt::format(
			"`time_index` <= {}",
			delete_time
		)
	);
}

struct TimeOfDay_Struct Database::LoadTime(time_t& realtime)
{
	TimeOfDay_Struct t{ };

	const std::string& query = "SELECT `minute`, `hour`, `day`, `month`, `year`, `realtime` FROM `eqtime` LIMIT 1";
	auto results = QueryDatabase(query);

	if (!results.Success() || !results.RowCount()) {
		LogInfo("Loading EQ time of day failed. Using defaults");
		t.minute = 0;
		t.hour   = 9;
		t.day    = 1;
		t.month  = 1;
		t.year   = 3100;
		realtime = time(nullptr);
		return t;
	}

	auto& row = results.begin();

	uint8  hour      = Strings::ToUnsignedInt(row[1]);
	time_t realtime_ = Strings::ToBigInt(row[5]);
	if (RuleI(World, BootHour) > 0 && RuleI(World, BootHour) <= 24) {
		hour      = RuleI(World, BootHour);
		realtime_ = time(nullptr);
	}

	t.minute = Strings::ToUnsignedInt(row[0]);
	t.hour   = hour;
	t.day    = Strings::ToUnsignedInt(row[2]);
	t.month  = Strings::ToUnsignedInt(row[3]);
	t.year   = Strings::ToUnsignedInt(row[4]);
	realtime = realtime_;

	LogEqTime("Setting hour to [{}]", hour);

	return t;
}

bool Database::SaveTime(int8 minute, int8 hour, int8 day, int8 month, int16 year)
{
	const std::string& query = fmt::format(
		"UPDATE `eqtime` SET `minute` = {}, `hour` = {}, `day` = {}, `month` = {}, `year` = {}, `realtime` = {} LIMIT 1",
		minute,
		hour,
		day,
		month,
		year,
		std::time(nullptr)
	);

	auto results = QueryDatabase(query);

	return results.Success();
}

int Database::GetIPExemption(const std::string& account_ip)
{
	const auto& l = IpExemptionsRepository::GetWhere(
		*this,
		fmt::format(
			"`exemption_ip` = '{}'",
			Strings::Escape(account_ip)
		)
	);

	if (l.empty()) {
		return RuleI(World, MaxClientsPerIP);
	}

	auto& e = l.front();

	return e.exemption_amount;
}

void Database::SetIPExemption(const std::string& account_ip, int exemption_amount)
{
	auto l = IpExemptionsRepository::GetWhere(
		*this,
		fmt::format(
			"`exemption_ip` = '{}'",
			Strings::Escape(account_ip)
		)
	);

	if (l.empty()) {
		auto e = IpExemptionsRepository::NewEntity();

		e.exemption_ip     = account_ip;
		e.exemption_amount = exemption_amount;

		IpExemptionsRepository::InsertOne(*this, e);

		return;
	}

	auto& e = l.front();

	e.exemption_amount = exemption_amount;

	IpExemptionsRepository::UpdateOne(*this, e);
}

int Database::GetInstanceID(uint32 character_id, uint32 zone_id)
{
	const auto& l = InstanceListRepository::GetWhere(
		*this,
		fmt::format(
			"`zone` = {} AND `id` IN (SELECT `id` FROM `instance_list_player` WHERE `charid` = {})",
			zone_id,
			character_id
		)
	);

	if (l.empty()) {
		return 0;
	}

	auto& e = l.front();

	return e.id;
}

bool Database::CopyCharacter(
	const std::string& source_character_name,
	const std::string& destination_character_name,
	const std::string& destination_account_name
)
{
	const auto& characters = CharacterDataRepository::GetWhere(
		*this,
		fmt::format(
			"`name` = '{}' AND `deleted_at` IS NULL LIMIT 1",
			Strings::Escape(source_character_name)
		)
	);

	if (characters.empty()) {
		LogError("No character found with name [{}]", source_character_name);
		return false;
	}

	auto& character = characters.front();

	const uint32 source_character_id = character.id;

	const auto& accounts = AccountRepository::GetWhere(
		*this,
		fmt::format(
			"`name` = '{}' LIMIT 1",
			Strings::Escape(destination_account_name)
		)
	);

	if (accounts.empty()) {
		LogError("No account found with name [{}]", destination_account_name);
		return false;
	}

	auto& account = accounts.front();

	const int destination_account_id = account.id;

	const int64 new_character_id = (CharacterDataRepository::GetMaxId(*this) + 1);

	// validate destination name doesn't exist already
	const auto& destination_characters = CharacterDataRepository::GetWhere(
		*this,
		fmt::format(
			"`name` = '{}' AND `deleted_at` IS NULL LIMIT 1",
			Strings::Escape(destination_character_name)
		)
	);
	if (!destination_characters.empty()) {
		LogError("Character [{}] already exists", destination_character_name);
		return false;
	}

	std::vector<std::string> tables_to_zero_id = {
		"keyring",
		"data_buckets",
		"character_evolving_items",
		"character_instance_safereturns",
		"character_expedition_lockouts",
		"character_instance_lockouts",
		"character_parcels",
		"character_tribute",
		"player_titlesets",
	};

	std::vector<std::string> ignore_tables = {
		"guilds",
	};

	size_t total_rows_copied = 0;

	TransactionBegin();

	for (const auto &t : DatabaseSchema::GetCharacterTables()) {
		const std::string& table_name               = t.first;
		const std::string& character_id_column_name = t.second;

		if (Strings::Contains(ignore_tables, table_name)) {
			continue;
		}

		auto results = QueryDatabase(
			fmt::format(
				"SHOW COLUMNS FROM {}",
				table_name
			)
		);

		if (!results.Success()) {
			LogError("Transaction failed [{}] rolling back", results.ErrorMessage());
			TransactionRollback();
			return false;
		}

		std::vector<std::string> columns      = {};
		int                      column_count = 0;

		for (auto row : results) {
			columns.emplace_back(row[0]);
			column_count++;
		}

		results = QueryDatabase(
			fmt::format(
				"SELECT {} FROM {} WHERE {} = {}",
				Strings::Implode(",", Strings::Wrap(columns, "`")),
				table_name,
				character_id_column_name,
				source_character_id
			)
		);

		if (!results.Success()) {
			LogError("Transaction failed [{}] rolling back", results.ErrorMessage());
			TransactionRollback();
			return false;
		}

		std::vector<std::vector<std::string>> new_rows;

		for (auto row : results) {
			std::vector<std::string> new_values   = {};

			for (int column_index = 0; column_index < column_count; column_index++) {
				const std::string& column = columns[column_index];
				std::string value = row[column_index] ? row[column_index] : "null";

				if (column == "id" && Strings::Contains(tables_to_zero_id, table_name)) {
					value = "0";
				}

				if (column == character_id_column_name) {
					value = std::to_string(new_character_id);
				}

				if (column == "name" && table_name == "character_data") {
					value = destination_character_name;
				}

				if (column == "account_id" && table_name == "character_data") {
					value = std::to_string(destination_account_id);
				}

				if (!Strings::IsNumber(value)) {
					value = Strings::Escape(value);
				}

				new_values.emplace_back(value);
			}

			new_rows.emplace_back(new_values);
		}

		std::vector<std::string> insert_rows;

		for (auto &r : new_rows) {
			const std::string& insert_row = "(" + Strings::Implode(",", Strings::Wrap(r, "'")) + ")";
			insert_rows.emplace_back(insert_row);
		}

		if (!insert_rows.empty()) {
			QueryDatabase(
				fmt::format(
					"DELETE FROM {} WHERE {} = {}",
					table_name,
					character_id_column_name,
					new_character_id
				)
			);

			auto insert = QueryDatabase(
				fmt::format(
					"INSERT INTO {} ({}) VALUES {}",
					table_name,
					Strings::Implode(",", Strings::Wrap(columns, "`")),
					Strings::Implode(",", insert_rows)
				)
			);

			size_t rows_copied = insert_rows.size(); // Rows copied for this table
			total_rows_copied += rows_copied; // Increment grand total

			LogInfo("Copying table [{}] rows [{}]", table_name, Strings::Commify(rows_copied));

			if (!insert.ErrorMessage().empty()) {
				LogError("Error copying table [{}] [{}]", table_name, insert.ErrorMessage());
				TransactionRollback();
				return false;
			}
		}
	}

	auto r = TransactionCommit();
	if (!r.Success()) {
		LogError("Transaction failed [{}] rolling back", r.ErrorMessage());
		return false;
	}

	LogInfo(
		"Character [{}] copied to [{}] total rows [{}]",
		source_character_name,
		destination_character_name,
		Strings::Commify(total_rows_copied)
	);

	return true;
}

void Database::SourceDatabaseTableFromUrl(const std::string& table_name, const std::string& url)
{
	try {
		uri request_uri(url);

		LogHTTP(
			"parsing url [{}] path [{}] host [{}] query_string [{}] protocol [{}] port [{}]",
			url,
			request_uri.get_path(),
			request_uri.get_host(),
			request_uri.get_query(),
			request_uri.get_scheme(),
			request_uri.get_port()
		);

		if (!DoesTableExist(table_name)) {
			LogMySQLQuery("Table [{}] does not exist. Downloading and installing...", table_name);

			// http get request
			httplib::Client cli(
				fmt::format(
					"{}://{}",
					request_uri.get_scheme(),
					request_uri.get_host()
				)
			);

			cli.set_connection_timeout(0, 60000000); // 60 sec
			cli.set_read_timeout(60, 0); // 60 seconds
			cli.set_write_timeout(60, 0); // 60 seconds

			int sourced_queries = 0;

			if (auto res = cli.Get(request_uri.get_path())) {
				if (res->status == 200) {
					for (auto& s : Strings::Split(res->body, ';')) {
						if (!Strings::Trim(s).empty()) {
							auto results = QueryDatabase(s);
							if (!results.ErrorMessage().empty()) {
								LogError("Error sourcing SQL [{}]", results.ErrorMessage());
								return;
							}

							sourced_queries++;
						}
					}
				}
			} else {
				LogError("Error retrieving URL [{}]", url);
			}

			LogMySQLQuery(
				"Table [{}] installed. Sourced [{}] queries",
				table_name,
				sourced_queries
			);
		}
	} catch (std::invalid_argument iae) {
		LogError("URI parser error [{}]", iae.what());
	}
}

void Database::SourceSqlFromUrl(const std::string& url)
{
	try {
		uri request_uri(url);

		LogHTTPDetail(
			"parsing url [{}] path [{}] host [{}] query_string [{}] protocol [{}] port [{}]",
			url,
			request_uri.get_path(),
			request_uri.get_host(),
			request_uri.get_query(),
			request_uri.get_scheme(),
			request_uri.get_port()
		);

		LogInfo("Downloading and installing from [{}]", url);

		// http get request
		httplib::Client cli(
			fmt::format(
				"{}://{}",
				request_uri.get_scheme(),
				request_uri.get_host()
			)
		);

		cli.set_connection_timeout(0, 60000000); // 60 sec
		cli.set_read_timeout(60, 0); // 60 seconds
		cli.set_write_timeout(60, 0); // 60 seconds

		if (auto res = cli.Get(request_uri.get_path())) {
			if (res->status == 200) {
				auto results = QueryDatabaseMulti(res->body);
				if (!results.ErrorMessage().empty()) {
					LogError("Error sourcing SQL [{}]", results.ErrorMessage());
					return;
				}
			}

			if (res->status == 404) {
				LogError("Error retrieving URL [{}]", url);
			}
		} else {
			LogError("Error retrieving URL [{}]", url);
		}
	} catch (std::invalid_argument iae) {
		LogError("URI parser error [{}]", iae.what());
	}
}

void Database::Encode(std::string &in)
{
	for(int i = 0; i < in.length(); i++) {
		in.at(i) += char('0');
	}
};

void Database::Decode(std::string &in)
{
	for(int i = 0; i < in.length(); i++) {
		in.at(i) -= char('0');
	}
};

void Database::PurgeCharacterParcels()
{
	auto filter  = fmt::format("sent_date < (NOW() - INTERVAL {} DAY)", RuleI(Parcel, ParcelPruneDelay));
	auto results = CharacterParcelsRepository::GetWhere(*this, filter);
	auto prune   = CharacterParcelsRepository::DeleteWhere(*this, filter);

	PlayerEvent::ParcelDelete                  pd{};
	PlayerEventLogsRepository::PlayerEventLogs pel{};
	pel.event_type_id   = PlayerEvent::PARCEL_DELETE;
	pel.event_type_name = PlayerEvent::EventName[pel.event_type_id];
	std::stringstream ss;
	for (auto const   &r: results) {
		pd.from_name    = r.from_name;
		pd.item_id      = r.item_id;
		pd.augment_1_id = r.aug_slot_1;
		pd.augment_2_id = r.aug_slot_2;
		pd.augment_3_id = r.aug_slot_3;
		pd.augment_4_id = r.aug_slot_4;
		pd.augment_5_id = r.aug_slot_5;
		pd.augment_6_id = r.aug_slot_6;
		pd.note         = r.note;
		pd.quantity     = r.quantity;
		pd.sent_date    = r.sent_date;
		pd.char_id      = r.char_id;
		{
			cereal::JSONOutputArchiveSingleLine ar(ss);
			pd.serialize(ar);
		}

		pel.event_data = ss.str();
		pel.created_at = std::time(nullptr);

		PlayerEventLogs::Instance()->AddToQueue(pel);

		ss.str("");
		ss.clear();
	}

	LogInfo(
		"Purged <yellow>[{}] parcels that were over <yellow>[{}] days old.",
		results.size(),
		RuleI(Parcel, ParcelPruneDelay)
	);
}

void Database::ClearGuildOnlineStatus()
{
	GuildMembersRepository::ClearOnlineStatus(*this);
}

void Database::ClearTraderDetails()
{
	TraderRepository::Truncate(*this);
}

void Database::ClearBuyerDetails()
{
	BuyerRepository::DeleteBuyer(*this, 0);
}

uint64_t Database::GetNextTableId(const std::string &table_name)
{
	auto results = QueryDatabase(fmt::format("SHOW TABLE STATUS LIKE '{}'", table_name));

	for (auto row: results) {
		for (int row_index = 0; row_index < results.ColumnCount(); row_index++) {
			std::string field_name = Strings::ToLower(results.FieldName(row_index));
			if (field_name == "auto_increment") {
				std::string value = row[row_index] ? row[row_index] : "null";
				return Strings::ToUnsignedBigInt(value, 1);
			}
		}
	}

	return 1;
}
