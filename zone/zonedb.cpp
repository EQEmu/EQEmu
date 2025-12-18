#include "zonedb.h"

#include "common/eqemu_logsys.h"
#include "common/extprofile.h"
#include "common/repositories/account_repository.h"
#include "common/repositories/blocked_spells_repository.h"
#include "common/repositories/character_alt_currency_repository.h"
#include "common/repositories/character_alternate_abilities_repository.h"
#include "common/repositories/character_auras_repository.h"
#include "common/repositories/character_bandolier_repository.h"
#include "common/repositories/character_bind_repository.h"
#include "common/repositories/character_buffs_repository.h"
#include "common/repositories/character_corpse_items_repository.h"
#include "common/repositories/character_corpses_repository.h"
#include "common/repositories/character_currency_repository.h"
#include "common/repositories/character_data_repository.h"
#include "common/repositories/character_data_repository.h"
#include "common/repositories/character_disciplines_repository.h"
#include "common/repositories/character_evolving_items_repository.h"
#include "common/repositories/character_exp_modifiers_repository.h"
#include "common/repositories/character_item_recast_repository.h"
#include "common/repositories/character_languages_repository.h"
#include "common/repositories/character_leadership_abilities_repository.h"
#include "common/repositories/character_material_repository.h"
#include "common/repositories/character_memmed_spells_repository.h"
#include "common/repositories/character_pet_buffs_repository.h"
#include "common/repositories/character_pet_info_repository.h"
#include "common/repositories/character_pet_inventory_repository.h"
#include "common/repositories/character_potionbelt_repository.h"
#include "common/repositories/character_skills_repository.h"
#include "common/repositories/character_spells_repository.h"
#include "common/repositories/character_tribute_repository.h"
#include "common/repositories/criteria/content_filter_criteria.h"
#include "common/repositories/merc_buffs_repository.h"
#include "common/repositories/merc_inventory_repository.h"
#include "common/repositories/merc_subtypes_repository.h"
#include "common/repositories/merchantlist_temp_repository.h"
#include "common/repositories/mercs_repository.h"
#include "common/repositories/npc_types_repository.h"
#include "common/repositories/npc_types_tint_repository.h"
#include "common/repositories/object_contents_repository.h"
#include "common/repositories/respawn_times_repository.h"
#include "common/repositories/spawn2_disabled_repository.h"
#include "common/repositories/trader_repository.h"
#include "common/repositories/zone_repository.h"
#include "common/rulesys.h"
#include "common/strings.h"
#include "zone/aura.h"
#include "zone/client.h"
#include "zone/corpse.h"
#include "zone/groups.h"
#include "zone/merc.h"
#include "zone/zone.h"

#include "fmt/format.h"
#include <ctime>
#include <iostream>

extern Zone* zone;

ZoneDatabase database;
ZoneDatabase content_db;

ZoneDatabase::ZoneDatabase()
: SharedDatabase()
{
	ZDBInitVars();
}

ZoneDatabase::ZoneDatabase(const char* host, const char* user, const char* passwd, const char* database, uint32 port)
: SharedDatabase(host, user, passwd, database, port)
{
	ZDBInitVars();
}

void ZoneDatabase::ZDBInitVars() {
	npc_spellseffects_cache = 0;
	npc_spellseffects_loadtried = 0;
	max_faction = 0;
	faction_array = nullptr;
}

ZoneDatabase::~ZoneDatabase() {
	if (npc_spellseffects_cache) {
		for (int x = 0; x <= npc_spellseffects_maxid; x++) {
			safe_delete_array(npc_spellseffects_cache[x]);
		}
		safe_delete_array(npc_spellseffects_cache);
	}
	safe_delete_array(npc_spellseffects_loadtried);

	if (faction_array != nullptr) {
		for (int x = 0; x <= max_faction; x++) {
			if (faction_array[x] != 0)
				safe_delete(faction_array[x]);
		}
		safe_delete_array(faction_array);
	}
}

bool ZoneDatabase::SaveZoneCFG(uint32 zone_id, uint16 instance_version, NewZone_Struct* zd)
{
	const auto& l = ZoneRepository::GetWhere(
		*this,
		fmt::format(
			"`zoneidnumber` = {} AND `version` = {}",
			zone_id,
			instance_version
		)
	);

	if (l.empty()) {
		return false;
	}

	auto e = l.front();

	e.underworld          = zd->underworld;
	e.minclip             = zd->minclip;
	e.maxclip             = zd->maxclip;
	e.fog_minclip         = zd->fog_minclip[0];
	e.fog_maxclip         = zd->fog_maxclip[0];
	e.fog_blue            = zd->fog_blue[0];
	e.fog_red             = zd->fog_red[0];
	e.fog_green           = zd->fog_green[0];
	e.sky                 = zd->sky;
	e.ztype               = zd->ztype;
	e.zone_exp_multiplier = zd->zone_exp_multiplier;
	e.safe_x              = zd->safe_x;
	e.safe_y              = zd->safe_y;
	e.safe_z              = zd->safe_z;
	e.safe_heading        = zd->safe_heading;

	return ZoneRepository::UpdateOne(*this, e);
}

void ZoneDatabase::UpdateRespawnTime(uint32 spawn2_id, uint16 instance_id, uint32 time_left)
{
	timeval tv;
	gettimeofday(&tv, nullptr);
	uint32 current_time = tv.tv_sec;

	/*	If we pass timeleft as 0 that means we clear from respawn time
			otherwise we update with a REPLACE INTO
	*/

	if (!time_left) {
		RespawnTimesRepository::DeleteWhere(
			*this,
			fmt::format(
				"`id` = {} AND `instance_id` = {}",
				spawn2_id,
				instance_id
			)
		);
		return;
	}

	RespawnTimesRepository::ReplaceOne(
		*this,
		RespawnTimesRepository::RespawnTimes{
			.id = static_cast<int32_t>(spawn2_id),
			.start = static_cast<int32_t>(current_time),
			.duration = static_cast<int32_t>(time_left),
			.expire_at = current_time + time_left,
			.instance_id = static_cast<int16_t>(instance_id)
		}
	);
}

//Gets the respawn time left in the database for the current spawn id
uint32 ZoneDatabase::GetSpawnTimeLeft(uint32 spawn2_id, uint16 instance_id)
{
	timeval tv;
	gettimeofday(&tv, nullptr);

	return RespawnTimesRepository::GetTimeRemaining(*this, spawn2_id, instance_id, tv.tv_sec);
}

void ZoneDatabase::UpdateSpawn2Status(uint32 id, uint8 new_status, uint32 instance_id)
{
	auto spawns = Spawn2DisabledRepository::GetWhere(
		*this,
		fmt::format("spawn2_id = {} and instance_id = {}", id, instance_id)
	);
	if (!spawns.empty()) {
		auto spawn = spawns[0];
		// 1 = enabled 0 = disabled
		spawn.disabled    = new_status ? 0 : 1;
		spawn.instance_id = instance_id;
		Spawn2DisabledRepository::UpdateOne(*this, spawn);
		return;
	}

	auto spawn = Spawn2DisabledRepository::NewEntity();
	spawn.spawn2_id   = id;
	spawn.instance_id = instance_id;
	spawn.disabled    = new_status ? 0 : 1;
	Spawn2DisabledRepository::InsertOne(*this, spawn);
}

bool ZoneDatabase::SetSpecialAttkFlag(uint8 id, const char* flag) {

	std::string query = StringFormat("UPDATE npc_types SET npcspecialattks='%s' WHERE id = %i;", flag, id);
    auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	return results.RowsAffected() != 0;
}

void ZoneDatabase::LoadWorldContainer(uint32 parent_id, EQ::ItemInstance* container)
{
	if (!container) {
		return;
	}

	const auto& l = ObjectContentsRepository::GetWhere(
		database,
		fmt::format(
			"`parentid` = {}",
			parent_id
		)
	);

	for (const auto& e : l) {
		const uint32 augments[EQ::invaug::SOCKET_COUNT] = {
			e.augslot1,
			e.augslot2,
			e.augslot3,
			e.augslot4,
			e.augslot5,
			static_cast<uint32>(e.augslot6)
		};

		auto inst = database.CreateItem(e.itemid, e.charges);

		if (inst && inst->GetItem()->IsClassCommon()) {
			for (int i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
				if (augments[i]) {
					inst->PutAugment(&database, i, augments[i]);
				}
			}

			container->PutItem(e.bagidx, *inst);
		}

		safe_delete(inst);
	}
}

void ZoneDatabase::SaveWorldContainer(uint32 zone_id, uint32 parent_id, const EQ::ItemInstance* container)
{
	if (!container) {
		return;
	}

	DeleteWorldContainer(parent_id, zone_id);

	for (uint8 index = EQ::invbag::SLOT_BEGIN; index <= EQ::invbag::SLOT_END; index++) {
		auto inst = container->GetItem(index);
		if (!inst) {
			continue;
		}

		uint32 augments[EQ::invaug::SOCKET_COUNT] = { 0, 0, 0, 0, 0, 0 };

		if (inst->IsType(EQ::item::ItemClassCommon)) {
			for (int i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
				auto augment = inst->GetAugment(i);
				augments[i] = (augment && augment->GetItem()) ? augment->GetItem()->ID : 0;
			}
		}

		ObjectContentsRepository::ReplaceOne(
			database,
			ObjectContentsRepository::ObjectContents{
				.zoneid = zone_id,
				.parentid = parent_id,
				.bagidx = index,
				.itemid = inst->GetItem()->ID,
				.charges = inst->GetCharges(),
				.droptime = std::time(nullptr),
				.augslot1 = augments[0],
				.augslot2 = augments[1],
				.augslot3 = augments[2],
				.augslot4 = augments[3],
				.augslot5 = augments[4],
				.augslot6 = static_cast<int32_t>(augments[5])
			}
		);
	}
}

void ZoneDatabase::DeleteWorldContainer(uint32 parent_id, uint32 zone_id)
{
	ObjectContentsRepository::DeleteWhere(
		database,
		fmt::format(
			"`parentid` = {} AND `zoneid` = {}",
			parent_id,
			zone_id
		)
	);
}

std::unique_ptr<EQ::ItemInstance> ZoneDatabase::LoadSingleTraderItem(uint32 char_id, int serial_number)
{
	auto results = TraderRepository::GetWhere(
		database,
		fmt::format(
			"`char_id` = '{}' AND `item_sn` = '{}' ORDER BY slot_id",
			char_id,
			serial_number
		)
	);

	if (results.empty()) {
		LogTrading("Could not find item serial number {} for character id {}", serial_number, char_id);
		return nullptr;
	}

	int item_id = results.at(0).item_id;
	int charges = results.at(0).item_charges;
	int cost    = results.at(0).item_cost;

	const EQ::ItemData *item = database.GetItem(item_id);
	if (!item) {
		LogTrading("Unable to create item.");
		return nullptr;
	}

	if (item->NoDrop == 0) {
		return nullptr;
	}

	std::unique_ptr<EQ::ItemInstance> inst(
		database.CreateItem(
			item_id,
			charges,
			results.at(0).aug_slot_1,
			results.at(0).aug_slot_2,
			results.at(0).aug_slot_3,
			results.at(0).aug_slot_4,
			results.at(0).aug_slot_5,
			results.at(0).aug_slot_6
		)
	);
	if (!inst) {
		LogTrading("Unable to create item instance.");
		return nullptr;
	}

	inst->SetCharges(charges);
	inst->SetSerialNumber(serial_number);
	inst->SetMerchantSlot(serial_number);
	inst->SetPrice(cost);

	if (inst->IsStackable()) {
		inst->SetMerchantCount(charges);
	}

	return std::move(inst);
}

void ZoneDatabase::UpdateTraderItemPrice(int char_id, uint32 item_id, uint32 charges, uint32 new_price) {

	LogTrading("ZoneDatabase::UpdateTraderPrice([{}], [{}], [{}], [{}])", char_id, item_id, charges, new_price);
	const EQ::ItemData *item = database.GetItem(item_id);

	if(!item) {
		return;
	}

	if (new_price == 0) {
		LogTrading("Removing Trader items from the DB for char_id [{}], item_id [{}]", char_id, item_id);

		auto results = TraderRepository::DeleteWhere(
			database,
			fmt::format(
				"`char_id` = '{}' AND `item_id` = {}",
				char_id,
				item_id
			)
		);
		if (!results) {
			LogDebug("[CLIENT] Failed to remove trader item(s): [{}] for char_id: [{}]",
					 item_id,
					 char_id
			);
		}

		return;
	}

	if (!item->Stackable) {
		auto results = TraderRepository::UpdateItem(database, char_id, new_price, item_id, charges);
		if (!results) {
			LogTrading(
				"Failed to update price for trader item [{}] for char_id: [{}]",
				item_id,
				char_id
			);
		}
		return;
	}

	auto results = TraderRepository::UpdateItem(database, char_id, new_price, item_id, 0);
	if (!results) {
		LogTrading(
			"Failed to update price for trader item [{}] for char_id: [{}]",
			item_id,
			char_id
		);
	}
}

void ZoneDatabase::AddBuyLine(uint32 CharID, uint32 BuySlot, uint32 ItemID, const char* ItemName, uint32 Quantity, uint32 Price) {
	std::string query = StringFormat("REPLACE INTO buyer VALUES(%i, %i, %i, \"%s\", %i, %i)",
                                    CharID, BuySlot, ItemID, ItemName, Quantity, Price);
    auto results = QueryDatabase(query);
	if (!results.Success())
		LogDebug("[CLIENT] Failed to save buline item: [{}] for char_id: [{}], the error was: [{}]\n", ItemID, CharID, results.ErrorMessage().c_str());

}

void ZoneDatabase::RemoveBuyLine(uint32 CharID, uint32 BuySlot) {
	std::string query = StringFormat("DELETE FROM buyer WHERE charid = %i AND buyslot = %i", CharID, BuySlot);
    auto results = QueryDatabase(query);
	if (!results.Success())
		LogDebug("[CLIENT] Failed to delete buyslot [{}] for charid: [{}], the error was: [{}]\n", BuySlot, CharID, results.ErrorMessage().c_str());

}

void ZoneDatabase::UpdateBuyLine(uint32 CharID, uint32 BuySlot, uint32 Quantity) {
	if(Quantity <= 0) {
		RemoveBuyLine(CharID, BuySlot);
		return;
	}

	std::string query = StringFormat(
		"UPDATE buyer SET quantity = %i WHERE charid = %i AND buyslot = %i",
		Quantity,
		CharID,
		BuySlot
	);

	auto results = QueryDatabase(query);
	if (!results.Success()) {
		LogTrading(
			"Failed to update quantity in buyslot [{}] for charid [{}], the error was [{}]\n",
			BuySlot,
			CharID,
			results.ErrorMessage().c_str()
		);
	}

}

#define StructDist(in, f1, f2) (uint32(&in->f2)-uint32(&in->f1))

bool ZoneDatabase::LoadCharacterData(uint32 character_id, PlayerProfile_Struct* pp, ExtendedProfile_Struct* m_epp){
	const auto& e = CharacterDataRepository::FindOne(database, character_id);
	if (!e.id) {
		return false;
	}

	strcpy(pp->name, e.name.c_str());
	strcpy(pp->last_name, e.last_name.c_str());
	strcpy(pp->title, e.title.c_str());
	strcpy(pp->suffix, e.suffix.c_str());

	pp->gender                   = e.gender;
	pp->race                     = e.race;
	pp->class_                   = e.class_;
	pp->level                    = e.level;
	pp->deity                    = e.deity;
	pp->birthday                 = e.birthday;
	pp->lastlogin                = e.last_login;
	pp->timePlayedMin            = e.time_played;
	pp->pvp                      = e.pvp_status;
	pp->level2                   = e.level2;
	pp->anon                     = e.anon;
	pp->gm                       = e.gm;
	pp->intoxication             = e.intoxication;
	pp->haircolor                = e.hair_color;
	pp->beardcolor               = e.beard_color;
	pp->eyecolor1                = e.eye_color_1;
	pp->eyecolor2                = e.eye_color_2;
	pp->hairstyle                = e.hair_style;
	pp->beard                    = e.beard;
	pp->ability_time_seconds     = e.ability_time_seconds;
	pp->ability_number           = e.ability_number;
	pp->ability_time_minutes     = e.ability_time_minutes;
	pp->ability_time_hours       = e.ability_time_hours;
	pp->exp                      = e.exp;
	pp->points                   = e.points;
	pp->mana                     = e.mana;
	pp->cur_hp                   = e.cur_hp;
	pp->STR                      = e.str;
	pp->STA                      = e.sta;
	pp->CHA                      = e.cha;
	pp->DEX                      = e.dex;
	pp->INT                      = e.int_;
	pp->AGI                      = e.agi;
	pp->WIS                      = e.wis;
	pp->face                     = e.face;
	pp->y                        = e.y;
	pp->x                        = e.x;
	pp->z                        = e.z;
	pp->heading                  = e.heading;
	pp->pvp2                     = e.pvp2;
	pp->pvptype                  = e.pvp_type;
	pp->autosplit                = e.autosplit_enabled;
	pp->zone_change_count        = e.zone_change_count;
	pp->drakkin_heritage         = e.drakkin_heritage;
	pp->drakkin_tattoo           = e.drakkin_tattoo;
	pp->drakkin_details          = e.drakkin_details;
	pp->toxicity                 = e.toxicity;
	pp->hunger_level             = e.hunger_level;
	pp->thirst_level             = e.thirst_level;
	pp->ability_up               = e.ability_up;
	pp->zone_id                  = e.zone_id;
	pp->zoneInstance             = e.zone_instance;
	pp->leadAAActive             = e.leadership_exp_on;
	pp->ldon_points_guk          = e.ldon_points_guk;
	pp->ldon_points_mir          = e.ldon_points_mir;
	pp->ldon_points_mmc          = e.ldon_points_mmc;
	pp->ldon_points_ruj          = e.ldon_points_ruj;
	pp->ldon_points_tak          = e.ldon_points_tak;
	pp->ldon_points_available    = e.ldon_points_available;
	pp->tribute_time_remaining   = e.tribute_time_remaining;
	pp->showhelm                 = e.show_helm;
	pp->career_tribute_points    = e.career_tribute_points;
	pp->tribute_points           = e.tribute_points;
	pp->tribute_active           = e.tribute_active;
	pp->endurance                = e.endurance;
	pp->group_leadership_exp     = e.group_leadership_exp;
	pp->raid_leadership_exp      = e.raid_leadership_exp;
	pp->group_leadership_points  = e.group_leadership_points;
	pp->raid_leadership_points   = e.raid_leadership_points;
	pp->air_remaining            = e.air_remaining;
	pp->PVPKills                 = e.pvp_kills;
	pp->PVPDeaths                = e.pvp_deaths;
	pp->PVPCurrentPoints         = e.pvp_current_points;
	pp->PVPCareerPoints          = e.pvp_career_points;
	pp->PVPBestKillStreak        = e.pvp_best_kill_streak;
	pp->PVPWorstDeathStreak      = e.pvp_worst_death_streak;
	pp->PVPCurrentKillStreak     = e.pvp_current_kill_streak;
	pp->aapoints_spent           = e.aa_points_spent;
	pp->expAA                    = e.aa_exp;
	pp->aapoints                 = e.aa_points;
	pp->groupAutoconsent         = e.group_auto_consent;
	pp->raidAutoconsent          = e.raid_auto_consent;
	pp->guildAutoconsent         = e.guild_auto_consent;
	pp->RestTimer                = e.RestTimer;
	pp->char_id                  = e.id;
	m_epp->aa_effects            = e.e_aa_effects;
	m_epp->perAA                 = e.e_percent_to_aa;
	m_epp->expended_aa           = e.e_expended_aa_spent;
	m_epp->last_invsnapshot_time = e.e_last_invsnapshot;
	m_epp->next_invsnapshot_time = m_epp->last_invsnapshot_time + (RuleI(Character, InvSnapshotMinIntervalM) * 60);

	return true;
}

bool ZoneDatabase::LoadCharacterFactionValues(uint32 character_id, faction_map & val_list) {
	std::string query = StringFormat("SELECT `faction_id`, `current_value` FROM `faction_values` WHERE `char_id` = %i", character_id);
	auto results = database.QueryDatabase(query);
	for (auto& row = results.begin(); row != results.end(); ++row) { val_list[Strings::ToInt(row[0])] = Strings::ToInt(row[1]); }
	return true;
}

bool ZoneDatabase::LoadCharacterMemmedSpells(uint32 character_id, PlayerProfile_Struct* pp)
{
	const auto& l = CharacterMemmedSpellsRepository::GetWhere(
		database,
		fmt::format(
			"`id` = {} ORDER BY `slot_id`",
			character_id
		)
	);

	for (int i = 0; i < EQ::spells::SPELL_GEM_COUNT; i++) { // Initialize Spells
		pp->mem_spells[i] = UINT32_MAX;
	}

	for (const auto& e : l) {
		if (e.slot_id < EQ::spells::SPELL_GEM_COUNT && IsValidSpell(e.spell_id)) {
			pp->mem_spells[e.slot_id] = e.spell_id;
		}
	}

	return true;
}

bool ZoneDatabase::LoadCharacterSpellBook(uint32 character_id, PlayerProfile_Struct* pp)
{
	const auto& l = CharacterSpellsRepository::GetWhere(
		database,
		fmt::format(
			"`id` = {} ORDER BY `slot_id`",
			character_id
		)
	);

	memset(pp->spell_book, UINT8_MAX, (sizeof(uint32) * EQ::spells::SPELLBOOK_SIZE));

	// We have the ability to block loaded spells by max id on a per-client basis..
	// but, we do not have to ability to keep players from using older clients after
	// they have scribed spells on a newer one that exceeds the older one's limit.
	// Load them all so that server actions are valid..but, nix them in translators.

	for (const auto& e : l) {
		if (!EQ::ValueWithin(e.slot_id, 0, EQ::spells::SPELLBOOK_SIZE)) {
			continue;
		}

		if (!IsValidSpell(e.spell_id)) {
			continue;
		}

		pp->spell_book[e.slot_id] = e.spell_id;
	}

	return true;
}

bool ZoneDatabase::LoadCharacterLanguages(uint32 character_id, PlayerProfile_Struct* pp)
{
	const auto& l = CharacterLanguagesRepository::GetWhere(
		database,
		fmt::format(
			"`id` = {} ORDER BY `lang_id`",
			character_id
		)
	);

	if (l.empty()) {
		return false;
	}

	for (int i = 0; i < MAX_PP_LANGUAGE; ++i) { // Initialize Languages
		pp->languages[i] = 0;
	}

	for (const auto& e : l) {
		if (EQ::ValueWithin(e.lang_id, Language::CommonTongue, Language::Unknown27)) {
			pp->languages[e.lang_id] = e.value;
		}
	}

	return true;
}

bool ZoneDatabase::LoadCharacterLeadershipAbilities(uint32 character_id, PlayerProfile_Struct* pp)
{
	const auto& l = CharacterLeadershipAbilitiesRepository::GetWhere(
		database,
		fmt::format(
			"`id` = {}",
			character_id
		)
	);

	for (const auto& e : l) {
		pp->leader_abilities.ranks[e.slot] = e.rank_;
	}

	return true;
}

bool ZoneDatabase::LoadCharacterDisciplines(Client* c)
{
	if (!c) {
		return false;
	}

	const auto& l = CharacterDisciplinesRepository::GetWhere(
		database, fmt::format(
			"`id` = {} ORDER BY `slot_id`",
			c->CharacterID()
		)
	);

	if (l.empty()) {
		return false;
	}

	for (int slot_id = 0; slot_id < MAX_PP_DISCIPLINES; slot_id++) {
		c->GetPP().disciplines.values[slot_id] = 0;
	}

	for (const auto& e : l) {
		if (IsValidSpell(e.disc_id) && e.slot_id < MAX_PP_DISCIPLINES) {
			c->GetPP().disciplines.values[e.slot_id] = e.disc_id;
		}
	}

	c->SendDisciplineUpdate();

	return true;
}

bool ZoneDatabase::LoadCharacterSkills(uint32 character_id, PlayerProfile_Struct* pp)
{
	const auto& l = CharacterSkillsRepository::GetWhere(
		*this,
		fmt::format(
			"`id` = {} ORDER BY `skill_id`",
			character_id
		)
	);

	for (int i = 0; i < MAX_PP_SKILL; ++i) { // Initialize Skills
		pp->skills[i] = 0;
	}

	for (const auto& e : l) {
		if (e.skill_id < MAX_PP_SKILL) {
			pp->skills[e.skill_id] = e.value;
		}
	}

	return true;
}

bool ZoneDatabase::LoadCharacterCurrency(uint32 character_id, PlayerProfile_Struct* pp)
{
	const auto& e = CharacterCurrencyRepository::FindOne(*this, character_id);
	if (!e.id) {
		return false;
	}

	pp->platinum            = e.platinum;
	pp->platinum_bank       = e.platinum_bank;
	pp->platinum_cursor     = e.platinum_cursor;
	pp->gold                = e.gold;
	pp->gold_bank           = e.gold_bank;
	pp->gold_cursor         = e.gold_cursor;
	pp->silver              = e.silver;
	pp->silver_bank         = e.silver_bank;
	pp->silver_cursor       = e.silver_cursor;
	pp->copper              = e.copper;
	pp->copper_bank         = e.copper_bank;
	pp->copper_cursor       = e.copper_cursor;
	pp->currentRadCrystals  = e.radiant_crystals;
	pp->careerRadCrystals   = e.career_radiant_crystals;
	pp->currentEbonCrystals = e.ebon_crystals;
	pp->careerEbonCrystals  = e.career_ebon_crystals;

	return true;
}

bool ZoneDatabase::LoadCharacterMaterialColor(uint32 character_id, PlayerProfile_Struct* pp)
{
	const auto& l = CharacterMaterialRepository::GetWhere(
		database,
		fmt::format(
			"`id` = {} LIMIT 9",
			character_id
		)
	);

	for (const auto& e : l) {
		pp->item_tint.Slot[e.slot].Blue    = e.blue;
		pp->item_tint.Slot[e.slot].Green   = e.green;
		pp->item_tint.Slot[e.slot].Red     = e.red;
		pp->item_tint.Slot[e.slot].UseTint = e.use_tint;
	}

	return true;
}

bool ZoneDatabase::LoadCharacterBandolier(uint32 character_id, PlayerProfile_Struct* pp)
{
	const auto& l = CharacterBandolierRepository::GetWhere(
		database,
		fmt::format(
			"`id` = {} LIMIT {}",
			character_id,
			EQ::profile::BANDOLIERS_SIZE
		)
	);

	for (int i = 0; i < EQ::profile::BANDOLIERS_SIZE; i++) {
		pp->bandoliers[i].Name[0] = '\0';

		for (int si = 0; si < EQ::profile::BANDOLIER_ITEM_COUNT; si++) {
			pp->bandoliers[i].Items[si].ID   = 0;
			pp->bandoliers[i].Items[si].Icon = 0;

			pp->bandoliers[i].Items[si].Name[0] = '\0';
		}
	}

	for (const auto& e : l) {
		const auto* item_data = database.GetItem(e.item_id);
		if (item_data) {
			pp->bandoliers[e.bandolier_id].Items[e.bandolier_slot].ID   = item_data->ID;
			pp->bandoliers[e.bandolier_id].Items[e.bandolier_slot].Icon = e.icon;

			strncpy(
				pp->bandoliers[e.bandolier_id].Items[e.bandolier_slot].Name,
				item_data->Name,
				64
			);
		} else {
			pp->bandoliers[e.bandolier_id].Items[e.bandolier_slot].ID   = 0;
			pp->bandoliers[e.bandolier_id].Items[e.bandolier_slot].Icon = 0;

			pp->bandoliers[e.bandolier_id].Items[e.bandolier_slot].Name[0] = '\0';
		}

		strncpy(pp->bandoliers[e.bandolier_id].Name, e.bandolier_name.c_str(), 32);
	}

	return true;
}

void ZoneDatabase::LoadCharacterTribute(Client* c){
	const auto& l = CharacterTributeRepository::GetWhere(database, fmt::format("character_id = {}", c->CharacterID()));

	for (auto& t : c->GetPP().tributes) {
		t.tier    = 0;
		t.tribute = TRIBUTE_NONE;
	}

	auto i = 0;

	for (const auto& e : l) {
		if (e.tribute != TRIBUTE_NONE) {
			c->GetPP().tributes[i].tier    = e.tier;
			c->GetPP().tributes[i].tribute = e.tribute;
			i++;
		}
	}
}

bool ZoneDatabase::LoadCharacterPotionBelt(uint32 character_id, PlayerProfile_Struct *pp)
{
	const auto& l = CharacterPotionbeltRepository::GetWhere(
		database,
		fmt::format(
			"`id` = {} LIMIT {}",
			character_id,
			EQ::profile::POTION_BELT_SIZE
		)
	);

	for (int i = 0; i < EQ::profile::POTION_BELT_SIZE; i++) { // Initialize Potion Belt
		pp->potionbelt.Items[i].Icon = 0;
		pp->potionbelt.Items[i].ID   = 0;
		pp->potionbelt.Items[i].Name[0] = '\0';
	}

	for (const auto& e : l) {
		const auto* item_data = database.GetItem(e.item_id);
		if (!item_data) {
			continue;
		}

		pp->potionbelt.Items[e.potion_id].ID   = item_data->ID;
		pp->potionbelt.Items[e.potion_id].Icon = e.icon;

		strncpy(pp->potionbelt.Items[e.potion_id].Name, item_data->Name, 64);
	}

	return true;
}

bool ZoneDatabase::LoadCharacterBindPoint(uint32 character_id, PlayerProfile_Struct *pp)
{
	const auto& l = CharacterBindRepository::GetWhere(
		database,
		fmt::format(
			"`id` = {} LIMIT 5",
			character_id
		)
	);

	if (l.empty()) {
		return true;
	}

	for (const auto& e : l) {
		if (!EQ::ValueWithin(e.slot, 0, 4)) {
			continue;
		}

		pp->binds[e.slot].zone_id     = e.zone_id;
		pp->binds[e.slot].instance_id = e.instance_id;
		pp->binds[e.slot].x           = e.x;
		pp->binds[e.slot].y           = e.y;
		pp->binds[e.slot].z           = e.z;
		pp->binds[e.slot].heading     = e.heading;
	}

	return true;
}

bool ZoneDatabase::SaveCharacterLanguage(uint32 character_id, uint32 language_id, uint32 value)
{
	return CharacterLanguagesRepository::ReplaceOne(
		*this,
		CharacterLanguagesRepository::CharacterLanguages{
			.id = character_id,
			.lang_id = static_cast<uint16_t>(language_id),
			.value = static_cast<uint16_t>(value)
		}
	);
}

bool ZoneDatabase::SaveCharacterMaterialColor(uint32 character_id, uint8 slot_id, uint32 color)
{
	const uint8 red   = (color & 0x00FF0000) >> 16;
	const uint8 green = (color & 0x0000FF00) >> 8;
	const uint8 blue  = (color & 0x000000FF);

	return CharacterMaterialRepository::ReplaceOne(
		*this,
		CharacterMaterialRepository::CharacterMaterial{
			.id       = character_id,
			.slot     = slot_id,
			.blue     = blue,
			.green    = green,
			.red      = red,
			.use_tint = UINT8_MAX,
			.color    = color
		}
	);
}

bool ZoneDatabase::SaveCharacterSkill(uint32 character_id, uint32 skill_id, uint32 value)
{
	return CharacterSkillsRepository::ReplaceOne(
		*this,
		CharacterSkillsRepository::CharacterSkills{
			.id = character_id,
			.skill_id = static_cast<uint16_t>(skill_id),
			.value = static_cast<uint16_t>(value)
		}
	);
}

bool ZoneDatabase::SaveCharacterDiscipline(uint32 character_id, uint32 slot_id, uint32 disc_id)
{
	return CharacterDisciplinesRepository::ReplaceOne(
		*this,
		CharacterDisciplinesRepository::CharacterDisciplines{
			.id = character_id,
			.slot_id = static_cast<uint16_t>(slot_id),
			.disc_id = static_cast<uint16_t>(disc_id)
		}
	);
}

void ZoneDatabase::SaveCharacterTribute(Client* c)
{
	std::vector<CharacterTributeRepository::CharacterTribute> tributes = {};
	CharacterTributeRepository::CharacterTribute tribute = {};

	uint32 tribute_count = 0;
	for (auto& t : c->GetPP().tributes) {
		if (t.tribute != TRIBUTE_NONE) {
			tribute_count++;
		}
	}

	tributes.reserve(tribute_count);

	for (auto& t : c->GetPP().tributes) {
		if (t.tribute != TRIBUTE_NONE) {
			tribute.character_id = c->CharacterID();
			tribute.tier         = t.tier;
			tribute.tribute      = t.tribute;

			tributes.emplace_back(tribute);
		}
	}

	CharacterTributeRepository::DeleteWhere(database, fmt::format("character_id = {}", c->CharacterID()));

	if (tribute_count > 0) {
		CharacterTributeRepository::InsertMany(database, tributes);
	}
}

bool ZoneDatabase::SaveCharacterBandolier(
	uint32 character_id,
	uint8 bandolier_id,
	uint8 bandolier_slot,
	uint32 item_id,
	uint32 icon,
	const char* bandolier_name
)
{
	return CharacterBandolierRepository::ReplaceOne(
		*this,
		CharacterBandolierRepository::CharacterBandolier{
			.id = character_id,
			.bandolier_id = bandolier_id,
			.bandolier_slot = bandolier_slot,
			.item_id = item_id,
			.icon = icon,
			.bandolier_name = bandolier_name
		}
	);
}

bool ZoneDatabase::SaveCharacterPotionBelt(uint32 character_id, uint8 potion_id, uint32 item_id, uint32 icon)
{
	return CharacterPotionbeltRepository::ReplaceOne(
		*this,
		CharacterPotionbeltRepository::CharacterPotionbelt{
			.id = character_id,
			.potion_id = potion_id,
			.item_id = item_id,
			.icon = icon
		}
	);
}

bool ZoneDatabase::SaveCharacterLeadershipAbilities(uint32 character_id, PlayerProfile_Struct* pp)
{
	std::vector<CharacterLeadershipAbilitiesRepository::CharacterLeadershipAbilities> v;

	auto e = CharacterLeadershipAbilitiesRepository::NewEntity();

	for (int slot_id = 0; slot_id < MAX_LEADERSHIP_AA_ARRAY; slot_id++) {
		if (pp->leader_abilities.ranks[slot_id] > 0) {
			e.id    = character_id;
			e.slot  = slot_id;
			e.rank_ = pp->leader_abilities.ranks[slot_id];

			v.emplace_back(e);
		}
	}

	return CharacterLeadershipAbilitiesRepository::ReplaceMany(*this, v);
}

bool ZoneDatabase::SaveCharacterData(
	Client* c,
	PlayerProfile_Struct* pp,
	ExtendedProfile_Struct* m_epp
) {
	if (!c) {
		return false;
	}

	/* If this is ever zero - the client hasn't fully loaded and potentially crashed during zone */
	if (c->AccountID() <= 0) {
		return false;
	}

	clock_t t = std::clock(); /* Function timer start */

	auto e = CharacterDataRepository::FindOne(database, c->CharacterID());
	if (!e.id) {
		return false;
	}

	e.id                      = c->CharacterID();
	e.account_id              = c->AccountID();
	e.name                    = pp->name;
	e.last_name               = pp->last_name;
	e.gender                  = pp->gender;
	e.race                    = pp->race;
	e.class_                  = pp->class_;
	e.level                   = pp->level;
	e.deity                   = pp->deity;
	e.birthday                = pp->birthday;
	e.last_login              = pp->lastlogin;
	e.time_played             = pp->timePlayedMin;
	e.pvp_status              = pp->pvp;
	e.level2                  = pp->level2;
	e.anon                    = pp->anon;
	e.gm                      = pp->gm;
	e.intoxication            = pp->intoxication;
	e.hair_color              = pp->haircolor;
	e.beard_color             = pp->beardcolor;
	e.eye_color_1             = pp->eyecolor1;
	e.eye_color_2             = pp->eyecolor2;
	e.hair_style              = pp->hairstyle;
	e.beard                   = pp->beard;
	e.ability_time_seconds    = pp->ability_time_seconds;
	e.ability_number          = pp->ability_number;
	e.ability_time_minutes    = pp->ability_time_minutes;
	e.ability_time_hours      = pp->ability_time_hours;
	e.title                   = pp->title;
	e.suffix                  = pp->suffix;
	e.exp                     = pp->exp;
	e.exp_enabled             = c->IsEXPEnabled();
	e.points                  = pp->points;
	e.mana                    = pp->mana;
	e.cur_hp                  = pp->cur_hp;
	e.str                     = pp->STR;
	e.sta                     = pp->STA;
	e.cha                     = pp->CHA;
	e.dex                     = pp->DEX;
	e.int_                    = pp->INT;
	e.agi                     = pp->AGI;
	e.wis                     = pp->WIS;
	e.face                    = pp->face;
	e.y                       = pp->y;
	e.x                       = pp->x;
	e.z                       = pp->z;
	e.heading                 = pp->heading;
	e.pvp2                    = pp->pvp2;
	e.pvp_type                = pp->pvptype;
	e.autosplit_enabled       = pp->autosplit;
	e.zone_change_count       = pp->zone_change_count;
	e.drakkin_heritage        = pp->drakkin_heritage;
	e.drakkin_tattoo          = pp->drakkin_tattoo;
	e.drakkin_details         = pp->drakkin_details;
	e.toxicity                = pp->toxicity;
	e.hunger_level            = pp->hunger_level;
	e.thirst_level            = pp->thirst_level;
	e.ability_up              = pp->ability_up;
	e.zone_id                 = pp->zone_id;
	e.zone_instance           = pp->zoneInstance;
	e.leadership_exp_on       = pp->leadAAActive;
	e.ldon_points_guk         = pp->ldon_points_guk;
	e.ldon_points_mir         = pp->ldon_points_mir;
	e.ldon_points_mmc         = pp->ldon_points_mmc;
	e.ldon_points_ruj         = pp->ldon_points_ruj;
	e.ldon_points_tak         = pp->ldon_points_tak;
	e.ldon_points_available   = pp->ldon_points_available;
	e.tribute_time_remaining  = pp->tribute_time_remaining;
	e.show_helm               = pp->showhelm;
	e.career_tribute_points   = pp->career_tribute_points;
	e.tribute_points          = pp->tribute_points;
	e.tribute_active          = pp->tribute_active;
	e.endurance               = pp->endurance;
	e.group_leadership_exp    = pp->group_leadership_exp;
	e.raid_leadership_exp     = pp->raid_leadership_exp;
	e.group_leadership_points = pp->group_leadership_points;
	e.raid_leadership_points  = pp->raid_leadership_points;
	e.air_remaining           = pp->air_remaining;
	e.pvp_kills               = pp->PVPKills;
	e.pvp_deaths              = pp->PVPDeaths;
	e.pvp_current_points      = pp->PVPCurrentPoints;
	e.pvp_career_points       = pp->PVPCareerPoints;
	e.pvp_best_kill_streak    = pp->PVPBestKillStreak;
	e.pvp_worst_death_streak  = pp->PVPWorstDeathStreak;
	e.pvp_current_kill_streak = pp->PVPCurrentKillStreak;
	e.aa_points_spent         = pp->aapoints_spent;
	e.aa_exp                  = pp->expAA;
	e.aa_points               = pp->aapoints;
	e.group_auto_consent      = pp->groupAutoconsent;
	e.raid_auto_consent       = pp->raidAutoconsent;
	e.guild_auto_consent      = pp->guildAutoconsent;
	e.RestTimer               = pp->RestTimer;
	e.e_aa_effects            = m_epp->aa_effects;
	e.e_percent_to_aa         = m_epp->perAA;
	e.e_expended_aa_spent     = m_epp->expended_aa;
	e.e_last_invsnapshot      = m_epp->last_invsnapshot_time;
	e.mailkey                 = c->GetMailKeyFull();
	e.illusion_block          = c->GetIllusionBlock();

	const int replaced = CharacterDataRepository::ReplaceOne(database, e);

	if (!replaced) {
		LogError("Failed to save character data for [{}] ID [{}].", c->GetCleanName(), c->CharacterID());
		return false;
	}

	LogDebug(
		"ZoneDatabase::SaveCharacterData [{}], done Took [{}] seconds",
		c->CharacterID(),
		((float)(std::clock() - t)) / CLOCKS_PER_SEC
	);
	return true;
}

bool ZoneDatabase::SaveCharacterCurrency(uint32 character_id, PlayerProfile_Struct* pp)
{
	ZeroPlayerProfileCurrency(pp);

	auto e = CharacterCurrencyRepository::NewEntity();

	return CharacterCurrencyRepository::ReplaceOne(
		*this,
		CharacterCurrencyRepository::CharacterCurrency{
			.id                      = character_id,
			.platinum                = static_cast<uint32_t>(pp->platinum),
			.gold                    = static_cast<uint32_t>(pp->gold),
			.silver                  = static_cast<uint32_t>(pp->silver),
			.copper                  = static_cast<uint32_t>(pp->copper),
			.platinum_bank           = static_cast<uint32_t>(pp->platinum_bank),
			.gold_bank               = static_cast<uint32_t>(pp->gold_bank),
			.silver_bank             = static_cast<uint32_t>(pp->silver_bank),
			.copper_bank             = static_cast<uint32_t>(pp->copper_bank),
			.platinum_cursor         = static_cast<uint32_t>(pp->platinum_cursor),
			.gold_cursor             = static_cast<uint32_t>(pp->gold_cursor),
			.silver_cursor           = static_cast<uint32_t>(pp->silver_cursor),
			.copper_cursor           = static_cast<uint32_t>(pp->copper_cursor),
			.radiant_crystals        = pp->currentRadCrystals,
			.career_radiant_crystals = pp->careerRadCrystals,
			.ebon_crystals           = pp->currentEbonCrystals,
			.career_ebon_crystals    = pp->careerEbonCrystals
		}
	);
}

bool ZoneDatabase::SaveCharacterMemorizedSpell(uint32 character_id, uint32 spell_id, uint32 slot_id){
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	return CharacterMemmedSpellsRepository::ReplaceOne(
		*this,
		CharacterMemmedSpellsRepository::CharacterMemmedSpells{
			.id = character_id,
			.slot_id = static_cast<uint16_t>(slot_id),
			.spell_id = static_cast<uint16_t>(spell_id)
		}
	);
}

bool ZoneDatabase::SaveCharacterSpell(uint32 character_id, uint32 spell_id, uint32 slot_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	return CharacterSpellsRepository::ReplaceOne(
		*this,
		CharacterSpellsRepository::CharacterSpells{
			.id = character_id,
			.slot_id = static_cast<uint16_t>(slot_id),
			.spell_id = static_cast<uint16_t>(spell_id)
		}
	);
}

bool ZoneDatabase::DeleteCharacterSpell(uint32 character_id, uint32 slot_id)
{
	return CharacterSpellsRepository::DeleteWhere(
		*this,
		fmt::format(
			"`id` = {} AND `slot_id` = {}",
			character_id,
			slot_id
		)
	);
}

bool ZoneDatabase::DeleteCharacterDiscipline(uint32 character_id, uint32 slot_id)
{
	return CharacterDisciplinesRepository::DeleteWhere(
		*this,
		fmt::format(
			"`id` = {} AND `slot_id` = {}",
			character_id,
			slot_id
		)
	);
}

bool ZoneDatabase::DeleteCharacterBandolier(uint32 character_id, uint32 bandolier_id)
{
	return CharacterBandolierRepository::DeleteWhere(
		*this,
		fmt::format(
			"`id` = {} AND `bandolier_id` = {}",
			character_id,
			bandolier_id
		)
	);
}

bool ZoneDatabase::DeleteCharacterLeadershipAbilities(uint32 character_id)
{
	return CharacterLeadershipAbilitiesRepository::DeleteOne(*this, character_id);
}

bool ZoneDatabase::DeleteCharacterAAs(uint32 character_id)
{
	return CharacterAlternateAbilitiesRepository::DeleteWhere(
		*this,
		fmt::format(
			"`id` = {} AND `aa_id` NOT IN (SELECT a.first_rank_id FROM aa_ability a WHERE a.grant_only != 0)",
			character_id
		)
	);
}

bool ZoneDatabase::DeleteCharacterMaterialColor(uint32 character_id)
{
	return CharacterMaterialRepository::DeleteOne(*this, character_id);
}

bool ZoneDatabase::DeleteCharacterMemorizedSpell(uint32 character_id, uint32 slot_id)
{
	return CharacterMemmedSpellsRepository::DeleteWhere(
		*this,
		fmt::format(
			"`id` = {} AND `slot_id` = {}",
			character_id,
			slot_id
		)
	);
}

bool ZoneDatabase::NoRentExpired(const std::string& name)
{
	const uint32 seconds = CharacterDataRepository::GetSecondsSinceLastLogin(*this, name);

	return seconds > 1800;
}

bool ZoneDatabase::SaveCharacterInvSnapshot(uint32 character_id) {
	uint32 time_index = time(nullptr);
	std::string query = StringFormat(
		"INSERT "
		"INTO"
		" `inventory_snapshots` "
		"(`time_index`,"
		" `charid`,"
		" `slotid`,"
		" `itemid`,"
		" `charges`,"
		" `color`,"
		" `augslot1`,"
		" `augslot2`,"
		" `augslot3`,"
		" `augslot4`,"
		" `augslot5`,"
		" `augslot6`,"
		" `instnodrop`,"
		" `custom_data`,"
		" `ornamenticon`,"
		" `ornamentidfile`,"
		" `ornament_hero_model`,"
		" `guid`"
		") "
		"SELECT"
		" %u,"
		" `character_id`,"
		" `slot_id`,"
		" `item_id`,"
		" `charges`,"
		" `color`,"
		" `augment_one`,"
		" `augment_two`,"
		" `augment_three`,"
		" `augment_four`,"
		" `augment_five`,"
		" `augment_six`,"
		" `instnodrop`,"
		" `custom_data`,"
		" `ornament_icon`,"
		" `ornament_idfile`,"
		" `ornament_hero_model`,"
		" `guid` "
		"FROM"
		" `inventory` "
		"WHERE"
		" `character_id` = %u",
		time_index,
		character_id
	);
	auto results = database.QueryDatabase(query);
	LogInventory("[{}] ([{}])", character_id, (results.Success() ? "pass" : "fail"));
	return results.Success();
}

int ZoneDatabase::CountCharacterInvSnapshots(uint32 character_id) {
	std::string query = StringFormat(
		"SELECT"
		" COUNT(*) "
		"FROM "
		"("
		"SELECT * FROM"
		" `inventory_snapshots` a "
		"WHERE"
		" `charid` = %u "
		"GROUP BY"
		" `time_index`"
		") b",
		character_id
	);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return -1;

	auto& row = results.begin();

	int64 count = Strings::ToBigInt(row[0]);
	if (count > 2147483647)
		return -2;
	if (count < 0)
		return -3;

	return count;
}

void ZoneDatabase::ClearCharacterInvSnapshots(uint32 character_id, bool from_now) {
	uint32 del_time = time(nullptr);
	if (!from_now) { del_time -= RuleI(Character, InvSnapshotHistoryD) * 86400; }

	std::string query = StringFormat(
		"DELETE "
		"FROM"
		" `inventory_snapshots` "
		"WHERE"
		" `charid` = %u "
		"AND"
		" `time_index` <= %lu",
		character_id,
		(unsigned long)del_time
	);
	QueryDatabase(query);
}

void ZoneDatabase::ListCharacterInvSnapshots(uint32 character_id, std::list<std::pair<uint32, int>> &is_list) {
	std::string query = StringFormat(
		"SELECT"
		" `time_index`,"
		" COUNT(*) "
		"FROM"
		" `inventory_snapshots` "
		"WHERE"
		" `charid` = %u "
		"GROUP BY"
		" `time_index` "
		"ORDER BY"
		" `time_index` "
		"DESC",
		character_id
	);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return;

	for (auto row : results)
		is_list.emplace_back(std::pair<uint32, int>(Strings::ToUnsignedInt(row[0]), Strings::ToInt(row[1])));
}

bool ZoneDatabase::ValidateCharacterInvSnapshotTimestamp(uint32 character_id, uint32 timestamp) {
	if (!character_id || !timestamp)
		return false;

	std::string query = StringFormat(
		"SELECT"
		" * "
		"FROM"
		" `inventory_snapshots` "
		"WHERE"
		" `charid` = %u "
		"AND"
		" `time_index` = %u "
		"LIMIT 1",
		character_id,
		timestamp
	);
	auto results = QueryDatabase(query);

	if (!results.Success() || results.RowCount() == 0)
		return false;

	return true;
}

void ZoneDatabase::ParseCharacterInvSnapshot(uint32 character_id, uint32 timestamp, std::list<std::pair<int16, uint32>> &parse_list) {
	std::string query = StringFormat(
		"SELECT"
		" `slotid`,"
		" `itemid` "
		"FROM"
		" `inventory_snapshots` "
		"WHERE"
		" `charid` = %u "
		"AND"
		" `time_index` = %u "
		"ORDER BY"
		" `slotid`",
		character_id,
		timestamp
	);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return;

	for (auto row : results)
		parse_list.emplace_back(std::pair<int16, uint32>(Strings::ToInt(row[0]), Strings::ToUnsignedInt(row[1])));
}

void ZoneDatabase::DivergeCharacterInvSnapshotFromInventory(uint32 character_id, uint32 timestamp, std::list<std::pair<int16, uint32>> &compare_list) {
	std::string query = StringFormat(
		"SELECT"
		" slotid,"
		" itemid "
		"FROM"
		" `inventory_snapshots` "
		"WHERE"
		" `time_index` = %u "
		"AND"
		" `charid` = %u "
		"AND"
		" `slotid` NOT IN "
		"("
		"SELECT"
		" a.`slotid` "
		"FROM"
		" `inventory_snapshots` a "
		"JOIN"
		" `inventory` b "
		"USING"
		" (`slot_id`, `item_id`) "
		"WHERE"
		" a.`time_index` = %u "
		"AND"
		" a.`charid` = %u "
		"AND"
		" b.`character_id` = %u"
		")",
		timestamp,
		character_id,
		timestamp,
		character_id,
		character_id
	);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return;

	for (auto row : results)
		compare_list.emplace_back(std::pair<int16, uint32>(Strings::ToInt(row[0]), Strings::ToUnsignedInt(row[1])));
}

void ZoneDatabase::DivergeCharacterInventoryFromInvSnapshot(uint32 character_id, uint32 timestamp, std::list<std::pair<int16, uint32>> &compare_list) {
	std::string query = StringFormat(
		"SELECT"
		" `slotid`,"
		" `itemid` "
		"FROM"
		" `inventory` "
		"WHERE"
		" `character_id` = %u "
		"AND"
		" `slotid` NOT IN "
		"("
		"SELECT"
		" a.`slotid` "
		"FROM"
		" `inventory` a "
		"JOIN"
		" `inventory_snapshots` b "
		"USING"
		" (`slotid`, `itemid`) "
		"WHERE"
		" b.`time_index` = %u "
		"AND"
		" b.`charid` = %u "
		"AND"
		" a.`character_id` = %u"
		")",
		character_id,
		timestamp,
		character_id,
		character_id
	);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return;

	for (auto row : results)
		compare_list.emplace_back(std::pair<int16, uint32>(Strings::ToInt(row[0]), Strings::ToUnsignedInt(row[1])));
}

bool ZoneDatabase::RestoreCharacterInvSnapshot(uint32 character_id, uint32 timestamp) {
	// we should know what we're doing by the time we call this function..but,
	// this is to prevent inventory deletions where no timestamp entries exists
	if (!ValidateCharacterInvSnapshotTimestamp(character_id, timestamp)) {
		LogError("called for id: [{}] without valid snapshot entries @ [{}]", character_id, timestamp);
		return false;
	}

	std::string query = StringFormat(
		"DELETE "
		"FROM"
		" `inventory` "
		"WHERE"
		" `character_id` = %u",
		character_id
	);
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;

	query = StringFormat(
		"INSERT "
		"INTO"
		" `inventory` "
		"(`character_id`,"
		" `slot_id`,"
		" `item_id`,"
		" `charges`,"
		" `color`,"
		" `augment_one`,"
		" `augment_two`,"
		" `augment_three`,"
		" `augment_four`,"
		" `augment_five`,"
		" `augment_six`,"
		" `instnodrop`,"
		" `custom_data`,"
		" `ornament_icon`,"
		" `ornament_idfile`,"
		" `ornament_hero_model`,"
		" `guid` "
		") "
		"SELECT"
		" `charid`,"
		" `slotid`,"
		" `itemid`,"
		" `charges`,"
		" `color`,"
		" `augslot1`,"
		" `augslot2`,"
		" `augslot3`,"
		" `augslot4`,"
		" `augslot5`,"
		" `augslot6`,"
		" `instnodrop`,"
		" `custom_data`,"
		" `ornamenticon`,"
		" `ornamentidfile`,"
		" `ornament_hero_model`, "
		" `guid` "
		"FROM"
		" `inventory_snapshots` "
		"WHERE"
		" `charid` = %u "
		"AND"
		" `time_index` = %u",
		character_id,
		timestamp
	);
	results = database.QueryDatabase(query);

	LogInventory("[{}] snapshot for [{}] @ [{}]",
		(results.Success() ? "restored" : "failed to restore"), character_id, timestamp);

	return results.Success();
}

const NPCType *ZoneDatabase::LoadNPCTypesData(uint32 npc_type_id, bool bulk_load /*= false*/)
{
	const NPCType *npc = nullptr;

	/* If there is a cached NPC entry, load it */
	auto itr = zone->npctable.find(npc_type_id);
	if (itr != zone->npctable.end()) {
		return itr->second;
	}

	std::string filter = fmt::format("id = {}", npc_type_id);

	if (bulk_load) {
		LogDebug("Performing bulk NPC Types load");

		filter = fmt::format(
			SQL(
				id IN (
					select npcID from spawnentry where spawngroupID IN (
						select spawngroupID from spawn2 where `zone` = '{}' and (`version` = {} OR `version` = -1)
					)
				)
			),
			zone->GetShortName(),
			zone->GetInstanceVersion()
		);
	}

	std::vector<uint32> npc_ids;
	std::vector<uint32> npc_faction_ids;
	std::vector<uint32> loottable_ids;

	for (NpcTypesRepository::NpcTypes &n : NpcTypesRepository::GetWhere((Database &) content_db, filter)) {
		NPCType *t;
		t = new NPCType;
		memset(t, 0, sizeof *t);

		t->npc_id = n.id;

		strn0cpy(t->name, n.name.c_str(), 50);

		t->level              = n.level;
		t->race               = n.race;
		t->class_             = n.class_;
		t->max_hp             = n.hp;
		t->current_hp         = n.hp;
		t->Mana               = n.mana;
		t->gender             = n.gender;
		t->texture            = n.texture;
		t->helmtexture        = n.helmtexture;
		t->herosforgemodel    = n.herosforgemodel;
		t->size               = n.size;
		t->loottable_id       = n.loottable_id;
		t->merchanttype       = n.merchant_id;
		t->alt_currency_type  = n.alt_currency_id;
		t->adventure_template = n.adventure_template_id;
		t->trap_template      = n.trap_template;
		t->attack_speed       = n.attack_speed;
		t->STR                = n.STR;
		t->STA                = n.STA;
		t->DEX                = n.DEX;
		t->AGI                = n.AGI;
		t->INT                = n._INT;
		t->WIS                = n.WIS;
		t->CHA                = n.CHA;
		t->MR                 = n.MR;
		t->CR                 = n.CR;
		t->DR                 = n.DR;
		t->FR                 = n.FR;
		t->PR                 = n.PR;
		t->Corrup             = n.Corrup;
		t->PhR                = n.PhR;
		t->min_dmg            = n.mindmg;
		t->max_dmg            = n.maxdmg;
		t->attack_count       = n.attack_count;
		t->is_parcel_merchant = n.is_parcel_merchant ? true : false;
		t->greed              = n.greed;
		t->m_npc_tint_id      = n.npc_tint_id;

		if (!n.special_abilities.empty()) {
			strn0cpy(t->special_abilities, n.special_abilities.c_str(), 512);
		}
		else {
			t->special_abilities[0] = '\0';
		}

		if (n.loottable_id > 0) {
			// check if we already have this loottable_id before inserting it
			if (std::find(loottable_ids.begin(), loottable_ids.end(), n.loottable_id) == loottable_ids.end()) {
				loottable_ids.emplace_back(n.loottable_id);
			}
		}

		t->npc_spells_id         = n.npc_spells_id;
		t->npc_spells_effects_id = n.npc_spells_effects_id;
		t->d_melee_texture1      = n.d_melee_texture1;
		t->d_melee_texture2      = n.d_melee_texture2;
		strn0cpy(t->ammo_idfile, n.ammo_idfile.c_str(), 30);
		t->prim_melee_type = n.prim_melee_type;
		t->sec_melee_type  = n.sec_melee_type;
		t->ranged_type     = n.ranged_type;
		t->runspeed        = n.runspeed;
		t->findable        = n.findable != 0;
		t->is_quest_npc    = n.isquest != 0;
		t->trackable       = n.trackable != 0;
		t->hp_regen        = n.hp_regen_rate;
		t->mana_regen      = n.mana_regen_rate;

		// set default value for aggroradius
		t->aggroradius = (int32) n.aggroradius;
		if (t->aggroradius <= 0) {
			t->aggroradius = 70;
		}

		t->assistradius = (int32) n.assistradius;
		if (t->assistradius <= 0) {
			t->assistradius = t->aggroradius;
		}

		if (n.bodytype > 0) {
			t->bodytype = n.bodytype;
		}
		else {
			t->bodytype = 0;
		}

		// facial features
		t->npc_faction_id   = n.npc_faction_id;
		t->luclinface       = n.face;
		t->hairstyle        = n.luclin_hairstyle;
		t->haircolor        = n.luclin_haircolor;
		t->eyecolor1        = n.luclin_eyecolor;
		t->eyecolor2        = n.luclin_eyecolor2;
		t->beardcolor       = n.luclin_beardcolor;
		t->beard            = n.luclin_beard;
		t->drakkin_heritage = n.drakkin_heritage;
		t->drakkin_tattoo   = n.drakkin_tattoo;
		t->drakkin_details  = n.drakkin_details;

		if (t->npc_faction_id > 0) {
			if (
				std::find(
					npc_faction_ids.begin(),
					npc_faction_ids.end(),
					t->npc_faction_id
				) == npc_faction_ids.end()
			) {
				npc_faction_ids.emplace_back(t->npc_faction_id);
			}
		}

		// armor tint
		uint32 armor_tint_id = n.armortint_id;
		t->armor_tint.Head.Color = (n.armortint_red & 0xFF) << 16;
		t->armor_tint.Head.Color |= (n.armortint_green & 0xFF) << 8;
		t->armor_tint.Head.Color |= (n.armortint_blue & 0xFF);
		t->armor_tint.Head.Color |= (t->armor_tint.Head.Color) ? (0xFF << 24) : 0;

		if (armor_tint_id != 0) {

			std::string armortint_query = StringFormat(
				"SELECT red1h, grn1h, blu1h, "
				"red2c, grn2c, blu2c, "
				"red3a, grn3a, blu3a, "
				"red4b, grn4b, blu4b, "
				"red5g, grn5g, blu5g, "
				"red6l, grn6l, blu6l, "
				"red7f, grn7f, blu7f, "
				"red8x, grn8x, blu8x, "
				"red9x, grn9x, blu9x "
				"FROM npc_types_tint WHERE id = %d",
				armor_tint_id
			);

			auto armortint_results = QueryDatabase(armortint_query);
			if (!armortint_results.Success() || armortint_results.RowCount() == 0) {
				armor_tint_id = 0;
			}
			else {
				auto& armorTint_row = armortint_results.begin();

				for (int index = EQ::textures::textureBegin; index <= EQ::textures::LastTexture; index++) {
					t->armor_tint.Slot[index].Color = Strings::ToInt(armorTint_row[index * 3]) << 16;
					t->armor_tint.Slot[index].Color |= Strings::ToInt(armorTint_row[index * 3 + 1]) << 8;
					t->armor_tint.Slot[index].Color |= Strings::ToInt(armorTint_row[index * 3 + 2]);
					t->armor_tint.Slot[index].Color |= (t->armor_tint.Slot[index].Color)
						? (0xFF << 24) : 0;
				}
			}
		}
		// Try loading npc_types tint fields if armor tint is 0 or query failed to get results
		if (armor_tint_id == 0) {
			for (int index = EQ::textures::armorChest; index < EQ::textures::materialCount; index++) {
				t->armor_tint.Slot[index].Color = t->armor_tint.Slot[0].Color; // odd way to 'zero-out' the array...
			}
		}

		t->see_invis        = n.see_invis;
		t->see_invis_undead = n.see_invis_undead != 0;    // Set see_invis_undead flag

		if (!RuleB(NPC, DisableLastNames) && !n.lastname.empty()) {
			strn0cpy(t->lastname, n.lastname.c_str(), sizeof(t->lastname));
		}

		t->qglobal                = n.qglobal != 0;    // qglobal
		t->AC                     = n.AC;
		t->npc_aggro              = n.npc_aggro != 0;
		t->spawn_limit            = n.spawn_limit;
		t->see_hide               = n.see_hide != 0;
		t->see_improved_hide      = n.see_improved_hide != 0;
		t->ATK                    = n.ATK;
		t->accuracy_rating        = n.Accuracy;
		t->avoidance_rating       = n.Avoidance;
		t->slow_mitigation        = n.slow_mitigation;
		t->maxlevel               = n.maxlevel;
		t->scalerate              = n.scalerate;
		t->private_corpse         = n.private_corpse != 0;
		t->unique_spawn_by_name   = n.unique_spawn_by_name != 0;
		t->underwater             = n.underwater != 0;
		t->emoteid                = n.emoteid;
		t->spellscale             = n.spellscale;
		t->healscale              = n.healscale;
		t->no_target_hotkey       = n.no_target_hotkey != 0;
		t->raid_target            = n.raid_target != 0;
		t->attack_delay           = n.attack_delay * 100; // TODO: fix DB
		t->light                  = (n.light & 0x0F);
		t->armtexture             = n.armtexture;
		t->bracertexture          = n.bracertexture;
		t->handtexture            = n.handtexture;
		t->legtexture             = n.legtexture;
		t->feettexture            = n.feettexture;
		t->ignore_despawn         = n.ignore_despawn != 0;
		t->show_name              = n.show_name != 0;
		t->untargetable           = n.untargetable != 0;
		t->charm_ac               = n.charm_ac;
		t->charm_min_dmg          = n.charm_min_dmg;
		t->charm_max_dmg          = n.charm_max_dmg;
		t->charm_attack_delay     = n.charm_attack_delay * 100; // TODO: fix DB
		t->charm_accuracy_rating  = n.charm_accuracy_rating;
		t->charm_avoidance_rating = n.charm_avoidance_rating;
		t->charm_atk              = n.charm_atk;
		t->skip_global_loot       = n.skip_global_loot != 0;
		t->rare_spawn             = n.rare_spawn != 0;
		t->stuck_behavior         = n.stuck_behavior;
		t->use_model              = n.model;
		t->flymode                = n.flymode;
		t->always_aggro           = n.always_aggro != 0;
		t->exp_mod                = n.exp_mod;
		t->skip_auto_scale        = false; // hardcoded here for now
		t->hp_regen_per_second    = n.hp_regen_per_second;
		t->heroic_strikethrough   = n.heroic_strikethrough;
		t->faction_amount         = n.faction_amount;
		t->keeps_sold_items       = n.keeps_sold_items;
		t->multiquest_enabled     = n.multiquest_enabled != 0;

		// If NPC with duplicate NPC id already in table,
		// free item we attempted to add.
		if (zone->npctable.find(t->npc_id) != zone->npctable.end()) {
			std::cerr << "Error loading duplicate NPC " << t->npc_id << std::endl;
			delete t;
			return nullptr;
		}

		zone->npctable[t->npc_id] = t;
		npc = t;

		// If NPC ID is not in npc_ids, add to vector
		if (!std::count(npc_ids.begin(), npc_ids.end(), t->npc_id)) {
			npc_ids.emplace_back(t->npc_id);
		}
	}

	if (!npc_faction_ids.empty()) {
		zone->LoadNPCFactions(npc_faction_ids);
		zone->LoadNPCFactionAssociations(npc_faction_ids);
	}

	zone->LoadLootTables(loottable_ids);

	return npc;
}

const NPCType* ZoneDatabase::GetMercenaryType(uint32 merc_npc_type_id, uint16 race_id, uint32 owner_level)
{
	const uint32 merc_type_id = merc_npc_type_id * 100 + owner_level;

	auto i = zone->merctable.find(merc_type_id);
	if (i != zone->merctable.end()) {
		return i->second;
	}

	if (!merc_npc_type_id) {
		return nullptr;
	}

	const std::string& query = fmt::format(
		SQL(
			SELECT
			m_stats.merc_npc_type_id,
			'' AS name,
			m_stats.level,
			m_types.race_id,
			m_subtypes.class_id,
			m_stats.hp,
			m_stats.mana,
			0 AS gender,
			m_armorinfo.texture,
			m_armorinfo.helmtexture,
			m_stats.attack_delay,
			m_stats.STR,
			m_stats.STA,
			m_stats.DEX,
			m_stats.AGI,
			m_stats._INT,
			m_stats.WIS,
			m_stats.CHA,
			m_stats.MR,
			m_stats.CR,
			m_stats.DR,
			m_stats.FR,
			m_stats.PR,
			m_stats.Corrup,
			m_stats.mindmg,
			m_stats.maxdmg,
			m_stats.attack_count,
			m_stats.special_abilities,
			m_weaponinfo.d_melee_texture1,
			m_weaponinfo.d_melee_texture2,
			m_weaponinfo.prim_melee_type,
			m_weaponinfo.sec_melee_type,
			m_stats.runspeed,
			m_stats.hp_regen_rate,
			m_stats.mana_regen_rate,
			1 AS bodytype,
			m_armorinfo.armortint_id,
			m_armorinfo.armortint_red,
			m_armorinfo.armortint_green,
			m_armorinfo.armortint_blue,
			m_stats.AC,
			m_stats.ATK,
			m_stats.Accuracy,
			m_stats.statscale,
			m_stats.spellscale,
			m_stats.healscale
			FROM merc_stats m_stats
			INNER JOIN merc_armorinfo m_armorinfo
			ON m_stats.merc_npc_type_id = m_armorinfo.merc_npc_type_id
			AND m_armorinfo.minlevel <= m_stats.level AND m_armorinfo.maxlevel >= m_stats.level
			INNER JOIN merc_weaponinfo m_weaponinfo
			ON m_stats.merc_npc_type_id = m_weaponinfo.merc_npc_type_id
			AND m_weaponinfo.minlevel <= m_stats.level AND m_weaponinfo.maxlevel >= m_stats.level
			INNER JOIN merc_templates m_templates
			ON m_templates.merc_npc_type_id = m_stats.merc_npc_type_id
			INNER JOIN merc_types m_types
			ON m_templates.merc_type_id = m_types.merc_type_id
			INNER JOIN merc_subtypes m_subtypes
			ON m_templates.merc_subtype_id = m_subtypes.merc_subtype_id
			WHERE m_templates.merc_npc_type_id = {} AND m_types.race_id = {} AND m_stats.clientlevel = {}
		),
		merc_npc_type_id,
		race_id,
		owner_level
	);

	auto results = QueryDatabase(query);
	if (!results.Success() || !results.RowCount()) {
		return nullptr;
	}

	const NPCType* n = nullptr;

	auto row = results.begin();

	NPCType* t = new NPCType;

	memset(t, 0, sizeof *t);

	t->npc_id = Strings::ToInt(row[0]);

	strn0cpy(t->name, row[1], sizeof(t->name));

	t->level        = Strings::ToInt(row[2]);
	t->race         = Strings::ToInt(row[3]);
	t->class_       = Strings::ToInt(row[4]);
	t->max_hp       = Strings::ToInt(row[5]);
	t->current_hp   = t->max_hp;
	t->Mana         = Strings::ToInt(row[6]);
	t->gender       = Strings::ToInt(row[7]);
	t->texture      = Strings::ToInt(row[8]);
	t->helmtexture  = Strings::ToInt(row[9]);
	t->attack_delay = Strings::ToInt(row[10]) * 100; // TODO: fix DB
	t->STR          = Strings::ToInt(row[11]);
	t->STA          = Strings::ToInt(row[12]);
	t->DEX          = Strings::ToInt(row[13]);
	t->AGI          = Strings::ToInt(row[14]);
	t->INT          = Strings::ToInt(row[15]);
	t->WIS          = Strings::ToInt(row[16]);
	t->CHA          = Strings::ToInt(row[17]);
	t->MR           = Strings::ToInt(row[18]);
	t->CR           = Strings::ToInt(row[19]);
	t->DR           = Strings::ToInt(row[20]);
	t->FR           = Strings::ToInt(row[21]);
	t->PR           = Strings::ToInt(row[22]);
	t->Corrup       = Strings::ToInt(row[23]);
	t->min_dmg      = Strings::ToInt(row[24]);
	t->max_dmg      = Strings::ToInt(row[25]);
	t->attack_count = Strings::ToInt(row[26]);

	if (row[27]) {
		strn0cpy(t->special_abilities, row[27], sizeof(t->special_abilities));
	} else {
		t->special_abilities[0] = '\0';
	}

	t->d_melee_texture1 = Strings::ToUnsignedInt(row[28]);
	t->d_melee_texture2 = Strings::ToUnsignedInt(row[29]);
	t->prim_melee_type  = Strings::ToInt(row[30]);
	t->sec_melee_type   = Strings::ToInt(row[31]);
	t->runspeed         = Strings::ToFloat(row[32]);
	t->hp_regen         = Strings::ToInt(row[33]);
	t->mana_regen       = Strings::ToInt(row[34]);
	t->aggroradius      = RuleI(Mercs, AggroRadius);
	t->bodytype         = row[35] && strlen(row[35]) ? static_cast<uint8>(Strings::ToUnsignedInt(row[35])) : 1;

	uint32 armor_tint_id = Strings::ToInt(row[36]);

	t->armor_tint.Slot[0].Color = (Strings::ToInt(row[37]) & 0xFF) << 16;
	t->armor_tint.Slot[0].Color |= (Strings::ToInt(row[38]) & 0xFF) << 8;
	t->armor_tint.Slot[0].Color |= (Strings::ToInt(row[39]) & 0xFF);
	t->armor_tint.Slot[0].Color |= (t->armor_tint.Slot[0].Color) ? (0xFF << 24) : 0;

	if (armor_tint_id == 0) {
		for (int index = EQ::textures::armorChest; index <= EQ::textures::LastTexture; index++) {
			t->armor_tint.Slot[index].Color = t->armor_tint.Slot[0].Color;
		}
	} else if (t->armor_tint.Slot[0].Color == 0) {
		const auto& e = NpcTypesTintRepository::FindOne(*this, armor_tint_id);
		if (!e.id) {
			armor_tint_id = 0;
		} else {
			t->armor_tint.Slot[EQ::textures::armorHead].Color = e.red1h << 16;
			t->armor_tint.Slot[EQ::textures::armorHead].Color = e.grn1h << 8;
			t->armor_tint.Slot[EQ::textures::armorHead].Color = e.blu1h;

			t->armor_tint.Slot[EQ::textures::armorChest].Color = e.red2c << 16;
			t->armor_tint.Slot[EQ::textures::armorChest].Color = e.grn2c << 8;
			t->armor_tint.Slot[EQ::textures::armorChest].Color = e.blu2c;

			t->armor_tint.Slot[EQ::textures::armorArms].Color = e.red3a << 16;
			t->armor_tint.Slot[EQ::textures::armorArms].Color = e.grn3a << 8;
			t->armor_tint.Slot[EQ::textures::armorArms].Color = e.blu3a;

			t->armor_tint.Slot[EQ::textures::armorWrist].Color = e.red4b << 16;
			t->armor_tint.Slot[EQ::textures::armorWrist].Color = e.grn4b << 8;
			t->armor_tint.Slot[EQ::textures::armorWrist].Color = e.blu4b;

			t->armor_tint.Slot[EQ::textures::armorHands].Color = e.red5g << 16;
			t->armor_tint.Slot[EQ::textures::armorHands].Color = e.grn5g << 8;
			t->armor_tint.Slot[EQ::textures::armorHands].Color = e.blu5g;

			t->armor_tint.Slot[EQ::textures::armorLegs].Color = e.red6l << 16;
			t->armor_tint.Slot[EQ::textures::armorLegs].Color = e.grn6l << 8;
			t->armor_tint.Slot[EQ::textures::armorLegs].Color = e.blu6l;

			t->armor_tint.Slot[EQ::textures::armorFeet].Color = e.red7f << 16;
			t->armor_tint.Slot[EQ::textures::armorFeet].Color = e.grn7f << 8;
			t->armor_tint.Slot[EQ::textures::armorFeet].Color = e.blu7f;

			t->armor_tint.Slot[EQ::textures::weaponPrimary].Color = e.red8x << 16;
			t->armor_tint.Slot[EQ::textures::weaponPrimary].Color = e.grn8x << 8;
			t->armor_tint.Slot[EQ::textures::weaponPrimary].Color = e.blu8x;

			t->armor_tint.Slot[EQ::textures::weaponSecondary].Color = e.red9x << 16;
			t->armor_tint.Slot[EQ::textures::weaponSecondary].Color = e.grn9x << 8;
			t->armor_tint.Slot[EQ::textures::weaponSecondary].Color = e.blu9x;
		}
	}

	t->AC               = Strings::ToInt(row[40]);
	t->ATK              = Strings::ToInt(row[41]);
	t->accuracy_rating  = Strings::ToInt(row[42]);
	t->scalerate        = Strings::ToInt(row[43]);
	t->spellscale       = Strings::ToInt(row[44]);
	t->healscale        = Strings::ToInt(row[45]);
	t->skip_global_loot = true;
	t->skip_auto_scale  = true;

	// If Merc with duplicate NPC id already in table,
	// free item we attempted to add.
	if (zone->merctable.find(merc_type_id) != zone->merctable.end()) {
		delete t;
		return nullptr;
	}

	zone->merctable[merc_type_id] = t;
	n = t;

	return n;
}

bool ZoneDatabase::LoadMercenaryInfo(Client* c)
{
	const auto& l = MercsRepository::GetWhere(
		*this,
		fmt::format(
			"`OwnerCharacterID` = {} ORDER BY `Slot`",
			c->CharacterID()
		)
	);

	if (l.empty()) {
		return false;
	}

	for (const auto& e : l) {
		if (e.Slot >= MAXMERCS) {
			continue;
		}

		auto& m = c->GetMercInfo(e.Slot);

		strn0cpy(m.merc_name, e.Name.c_str(), sizeof(m.merc_name));

		m.mercid             = e.MercID;
		m.slot               = e.Slot;
		m.MercTemplateID     = e.TemplateID;
		m.SuspendedTime      = e.SuspendedTime;
		m.IsSuspended        = e.IsSuspended;
		m.MercTimerRemaining = e.TimerRemaining;
		m.Gender             = e.Gender;
		m.MercSize           = e.MercSize;
		m.State              = 5;
		m.Stance             = e.StanceID;
		m.hp                 = e.HP;
		m.mana               = e.Mana;
		m.endurance          = e.Endurance;
		m.face               = e.Face;
		m.luclinHairStyle    = e.LuclinHairStyle;
		m.luclinHairColor    = e.LuclinHairColor;
		m.luclinEyeColor     = e.LuclinEyeColor;
		m.luclinEyeColor2    = e.LuclinEyeColor2;
		m.luclinBeardColor   = e.LuclinBeardColor;
		m.luclinBeard        = e.LuclinBeard;
		m.drakkinHeritage    = e.DrakkinHeritage;
		m.drakkinTattoo      = e.DrakkinTattoo;
		m.drakkinDetails     = e.DrakkinDetails;
	}

	return true;
}

bool ZoneDatabase::LoadCurrentMercenary(Client* c)
{
	const uint8 mercenary_slot = c->GetMercSlot();

	if (mercenary_slot > MAXMERCS) {
		return false;
	}

	const auto& e = MercsRepository::GetMercenaryBySlot(*this, c);

	if (!e.MercID) {
		return false;
	}

	auto& m = c->GetMercInfo(mercenary_slot);

	strn0cpy(m.merc_name, e.Name.c_str(), sizeof(m.merc_name));

	m.mercid             = e.MercID;
	m.slot               = e.Slot;
	m.MercTemplateID     = e.TemplateID;
	m.SuspendedTime      = e.SuspendedTime;
	m.IsSuspended        = e.IsSuspended;
	m.MercTimerRemaining = e.TimerRemaining;
	m.Gender             = e.Gender;
	m.MercSize           = e.MercSize;
	m.State              = e.StanceID;
	m.hp                 = e.HP;
	m.mana               = e.Mana;
	m.endurance          = e.Endurance;
	m.face               = e.Face;
	m.luclinHairStyle    = e.LuclinHairStyle;
	m.luclinHairColor    = e.LuclinHairColor;
	m.luclinEyeColor     = e.LuclinEyeColor;
	m.luclinEyeColor2    = e.LuclinEyeColor2;
	m.luclinBeardColor   = e.LuclinBeardColor;
	m.luclinBeard        = e.LuclinBeard;
	m.drakkinHeritage    = e.DrakkinHeritage;
	m.drakkinTattoo      = e.DrakkinTattoo;
	m.drakkinDetails     = e.DrakkinDetails;

	return true;
}

bool ZoneDatabase::SaveMercenary(Merc* m)
{
	Client* c = m->GetMercenaryOwner();

	if (!c) {
		return false;
	}

	if (!m->GetMercenaryID()) {
		auto e = MercsRepository::NewEntity();

		e.OwnerCharacterID = m->GetMercenaryCharacterID();
		e.Slot             = (c->GetNumberOfMercenaries() - 1);
		e.Name             = m->GetCleanName();
		e.TemplateID       = m->GetMercenaryTemplateID();
		e.SuspendedTime    = c->GetMercInfo().SuspendedTime;
		e.IsSuspended      = m->IsSuspended();
		e.TimerRemaining   = c->GetMercInfo().MercTimerRemaining;
		e.Gender           = m->GetGender();
		e.MercSize         = m->GetSize();
		e.StanceID         = m->GetStance();
		e.HP               = m->GetHP();
		e.Mana             = m->GetMana();
		e.Endurance        = m->GetEndurance();
		e.Face             = m->GetLuclinFace();
		e.LuclinHairStyle  = m->GetHairStyle();
		e.LuclinHairColor  = m->GetHairColor();
		e.LuclinEyeColor   = m->GetEyeColor1();
		e.LuclinEyeColor2  = m->GetEyeColor2();
		e.LuclinBeardColor = m->GetBeardColor();
		e.LuclinBeard      = m->GetBeard();
		e.DrakkinHeritage  = m->GetDrakkinHeritage();
		e.DrakkinTattoo    = m->GetDrakkinTattoo();
		e.DrakkinDetails   = m->GetDrakkinDetails();

		e = MercsRepository::InsertOne(*this, e);

		if (!e.MercID) {
			c->Message(Chat::Red, "Unable to save mercenary to the database.");
			return false;
		}

		m->SetMercID(e.MercID);
		m->UpdateMercInfo(c);

		database.SaveMercenaryBuffs(m);

		return true;
	}

	auto e = MercsRepository::FindOne(*this, m->GetMercenaryID());

	e.OwnerCharacterID = m->GetMercenaryCharacterID();
	e.Slot             = (c->GetNumberOfMercenaries() - 1);
	e.Name             = m->GetCleanName();
	e.TemplateID       = m->GetMercenaryTemplateID();
	e.SuspendedTime    = c->GetMercInfo().SuspendedTime;
	e.IsSuspended      = m->IsSuspended();
	e.TimerRemaining   = c->GetMercInfo().MercTimerRemaining;
	e.Gender           = m->GetGender();
	e.MercSize         = m->GetSize();
	e.StanceID         = m->GetStance();
	e.HP               = m->GetHP();
	e.Mana             = m->GetMana();
	e.Endurance        = m->GetEndurance();
	e.Face             = m->GetLuclinFace();
	e.LuclinHairStyle  = m->GetHairStyle();
	e.LuclinHairColor  = m->GetHairColor();
	e.LuclinEyeColor   = m->GetEyeColor1();
	e.LuclinEyeColor2  = m->GetEyeColor2();
	e.LuclinBeardColor = m->GetBeardColor();
	e.LuclinBeard      = m->GetBeard();
	e.DrakkinHeritage  = m->GetDrakkinHeritage();
	e.DrakkinTattoo    = m->GetDrakkinTattoo();
	e.DrakkinDetails   = m->GetDrakkinDetails();

	const int updated = MercsRepository::UpdateOne(*this, e);

	if (!updated) {
		c->Message(Chat::Red, "Unable to save mercenary to the database.");
		return false;
	}

	m->UpdateMercInfo(c);

	database.SaveMercenaryBuffs(m);

	return true;
}

void ZoneDatabase::SaveMercenaryBuffs(Merc* m)
{
	auto buffs = m->GetBuffs();

	MercBuffsRepository::DeleteWhere(
		*this,
		fmt::format(
			"`MercID` = {}",
			m->GetMercenaryID()
		)
	);

	std::vector<MercBuffsRepository::MercBuffs> v;

	auto e = MercBuffsRepository::NewEntity();

	for (uint32 slot_id = 0; slot_id <= BUFF_COUNT; slot_id++) {
		if (!IsValidSpell(buffs[slot_id].spellid)) {
			continue;
		}

		e.MercId             = m->GetMercenaryID();
		e.SpellId            = buffs[slot_id].spellid;
		e.CasterLevel        = buffs[slot_id].casterlevel;
		e.DurationFormula    = spells[buffs[slot_id].spellid].buff_duration_formula;
		e.TicsRemaining      = buffs[slot_id].ticsremaining;
		e.PoisonCounters     = CalculatePoisonCounters(buffs[slot_id].spellid) > 0 ? buffs[slot_id].counters : 0;
		e.DiseaseCounters    = CalculateDiseaseCounters(buffs[slot_id].spellid) > 0 ? buffs[slot_id].counters : 0;
		e.CurseCounters      = CalculateCurseCounters(buffs[slot_id].spellid) > 0 ? buffs[slot_id].counters : 0;
		e.CorruptionCounters = CalculateCorruptionCounters(buffs[slot_id].spellid) > 0 ? buffs[slot_id].counters : 0;
		e.HitCount           = buffs[slot_id].hit_number;
		e.MeleeRune          = buffs[slot_id].melee_rune;
		e.MagicRune          = buffs[slot_id].magic_rune;
		e.dot_rune           = buffs[slot_id].dot_rune;
		e.caston_x           = buffs[slot_id].caston_x;
		e.caston_y           = buffs[slot_id].caston_y;
		e.caston_z           = buffs[slot_id].caston_z;
		e.Persistent         = buffs[slot_id].persistant_buff ? 1 : 0;
		e.ExtraDIChance      = buffs[slot_id].ExtraDIChance;

		v.emplace_back(e);
	}

	if (!v.empty()) {
		MercBuffsRepository::InsertMany(*this, v);
	}
}

void ZoneDatabase::LoadMercenaryBuffs(Merc* m)
{
	auto      buffs     = m->GetBuffs();
	const int max_slots = m->GetMaxBuffSlots();

	const auto& l = MercBuffsRepository::GetWhere(
		*this,
		fmt::format(
			"`MercID` = {}",
			m->GetMercenaryID()
		)
	);

	if (l.empty()) {
		return;
	}

	uint32 slot_id = 0;

	for (const auto& e : l) {
		if (slot_id == BUFF_COUNT) {
			break;
		}

		buffs[slot_id].spellid         = e.SpellId;
		buffs[slot_id].casterlevel     = e.CasterLevel;
		buffs[slot_id].ticsremaining   = e.TicsRemaining;
		buffs[slot_id].hit_number      = e.HitCount;
		buffs[slot_id].melee_rune      = e.MeleeRune;
		buffs[slot_id].magic_rune      = e.MagicRune;
		buffs[slot_id].dot_rune        = e.dot_rune;
		buffs[slot_id].caston_x        = e.caston_x;
		buffs[slot_id].caston_y        = e.caston_y;
		buffs[slot_id].caston_z        = e.caston_z;
		buffs[slot_id].casterid        = 0;
		buffs[slot_id].ExtraDIChance   = e.ExtraDIChance;
		buffs[slot_id].persistant_buff = e.Persistent;

		if (CalculatePoisonCounters(buffs[slot_id].spellid) > 0) {
			buffs[slot_id].counters = e.PoisonCounters;
		}

		if (CalculateDiseaseCounters(buffs[slot_id].spellid) > 0) {
			buffs[slot_id].counters = e.DiseaseCounters;
		}

		if (CalculateCurseCounters(buffs[slot_id].spellid) > 0) {
			buffs[slot_id].counters = e.CurseCounters;
		}

		if (CalculateCorruptionCounters(buffs[slot_id].spellid) > 0) {
			buffs[slot_id].counters = e.CorruptionCounters;
		}
	}

	MercBuffsRepository::DeleteWhere(
		*this,
		fmt::format(
			"`MercID` = {}",
			m->GetMercenaryID()
		)
	);
}

bool ZoneDatabase::DeleteMercenary(uint32 mercenary_id)
{
	if (!mercenary_id) {
		return false;
	}

	MercBuffsRepository::DeleteWhere(
		*this,
		fmt::format(
			"`MercID` = {}",
			mercenary_id
		)
	);

	const int deleted = MercsRepository::DeleteOne(*this, mercenary_id);
	if (!deleted) {
		return false;
	}

	return true;
}

void ZoneDatabase::LoadMercenaryEquipment(Merc* m)
{
	const int merc_subtype_id = MercSubtypesRepository::GetSubtype(*this, m->GetClass(), m->GetTierID());

	const auto& l = MercInventoryRepository::GetWhere(
		*this,
		fmt::format(
			"`merc_subtype_id` = {} AND `min_level` <= {} AND `max_level` >= {}",
			merc_subtype_id,
			m->GetLevel(),
			m->GetLevel()
		)
	);

	if (l.empty()) {
		return;
	}

	uint32 item_count = 0;

	for (const auto& e: l) {
		if (item_count == EQ::invslot::EQUIPMENT_COUNT) {
			break;
		}

		if (!e.item_id) {
			continue;
		}

		m->AddItem(item_count, e.item_id);

		item_count++;
	}
}

void ZoneDatabase::SaveMerchantTemp(
	uint32 npc_id,
	uint32 slot_id,
	uint32 zone_id,
	uint32 instance_id,
	uint32 item_id,
	uint32 charges
)
{
	auto e = MerchantlistTempRepository::NewEntity();

	e.npcid       = npc_id;
	e.slot        = slot_id;
	e.zone_id     = zone_id;
	e.instance_id = instance_id;
	e.itemid      = item_id;
	e.charges     = charges;

	MerchantlistTempRepository::ReplaceOne(*this, e);
}

void ZoneDatabase::DeleteMerchantTemp(uint32 npc_id, uint32 slot_id, uint32 zone_id, uint32 instance_id)
{
	MerchantlistTempRepository::DeleteWhere(
		*this,
		fmt::format(
			"`npcid` = {} AND `slot` = {} AND `zone_id` = {} AND `instance_id` = {}",
			npc_id,
			slot_id,
			zone_id,
			instance_id
		)
	);
}

uint32 ZoneDatabase::GetZoneTimezone(uint32 zone_id, uint32 instance_version)
{
	const auto& l = ZoneRepository::GetWhere(
		*this,
		fmt::format(
			"`zoneidnumber` = {} AND (`version` = {} OR `version` = 0) ORDER BY `version` DESC",
			zone_id,
			instance_version
		)
	);

	if (l.empty()) {
		return 0;
	}

	return l[0].id ? l[0].timezone : 0;
}

bool ZoneDatabase::SetZoneTimezone(uint32 zone_id, uint32 instance_version, uint32 timezone)
{
	return ZoneRepository::SetTimeZone(*this, zone_id, instance_version, timezone);
}

void ZoneDatabase::RefreshGroupFromDB(Client *client){
	if (!client) {
		return;
	}

	Group *group = client->GetGroup();

	if (!group) {
		return;
	}

	auto outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupUpdate2_Struct));
	GroupUpdate2_Struct* gu = (GroupUpdate2_Struct*)outapp->pBuffer;
	gu->action = groupActUpdate;

	strcpy(gu->yourname, client->GetName());
	GetGroupLeadershipInfo(group->GetID(), gu->leadersname, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &gu->leader_aas);
	gu->NPCMarkerID = group->GetNPCMarkerID();

	int index = 0;

	auto query = fmt::format(
		"SELECT name FROM group_id WHERE group_id = {}",
		group->GetID()
	);
	auto results = QueryDatabase(query);

	if (results.Success()) {
		for (auto row : results) {
			if (index >= 6) {
				continue;
			}

            if (!strcmp(client->GetName(), row[0])) {
				continue;
			}

			strcpy(gu->membername[index], row[0]);
			index++;
		}
	}

	client->QueuePacket(outapp);
	safe_delete(outapp);

	if (client->ClientVersion() >= EQ::versions::ClientVersion::SoD) {
		group->NotifyMainTank(client, 1);
		group->NotifyPuller(client, 1);
	}

	group->NotifyMainAssist(client, 1);
	group->NotifyMarkNPC(client);
	group->NotifyAssistTarget(client);
	group->NotifyTankTarget(client);
	group->NotifyPullerTarget(client);
	group->SendMarkedNPCsToMember(client);
}

int64 ZoneDatabase::GetBlockedSpellsCount(uint32 zone_id)
{
	return BlockedSpellsRepository::Count(
		*this,
		fmt::format(
			"zoneid = {} {}",
			zone_id,
			ContentFilterCriteria::apply()
		)
	);
}

bool ZoneDatabase::LoadBlockedSpells(int64 blocked_spells_count, ZoneSpellsBlocked* into, uint32 zone_id)
{
	LogInfo("Loading Blocked Spells from database for {} ({}).", ZoneStore::Instance()->GetZoneName(zone_id, true), zone_id);

	const auto& l = BlockedSpellsRepository::GetWhere(
		*this,
		fmt::format(
			"zoneid = {} {} ORDER BY id ASC",
			zone_id,
			ContentFilterCriteria::apply()
		)
	);

	if (l.empty()) {
		return true;
	}

	int64 i = 0;

	for (const auto& e : l) {
		if (i >= blocked_spells_count) {
			LogError(
				"Blocked spells count of {} exceeded for {} ({}).",
				blocked_spells_count,
				ZoneStore::Instance()->GetZoneName(zone_id, true),
				zone_id
			);
			break;
		}

		memset(&into[i], 0, sizeof(ZoneSpellsBlocked));
		into[i].spellid      = e.spellid;
		into[i].type         = e.type;
		into[i].m_Location   = glm::vec3(e.x, e.y, e.z);
		into[i].m_Difference = glm::vec3(e.x_diff, e.y_diff, e.z_diff);
		strn0cpy(into[i].message, e.message.c_str(), sizeof(into[i].message));
		i++;
	}

	return true;
}

int ZoneDatabase::getZoneShutDownDelay(uint32 zoneID, uint32 version)
{
	auto z = GetZoneVersionWithFallback(zoneID, version);

    return z ? z->shutdowndelay : RuleI(Zone, AutoShutdownDelay);
}

uint32 ZoneDatabase::GetKarma(uint32 account_id)
{
	const auto& e = AccountRepository::FindOne(*this, account_id);

	return !e.id ? 0 : e.karma;
}

void ZoneDatabase::UpdateKarma(uint32 account_id, uint32 amount)
{
	auto e = AccountRepository::FindOne(*this, account_id);
	if (!e.id) {
		return;
	}

	e.karma = amount;

	AccountRepository::UpdateOne(*this, e);
}

void ZoneDatabase::ListAllInstances(Client* client, uint32 character_id)
{
	if (!client) {
		return;
	}

	std::string query = fmt::format(
		"SELECT instance_list.id, zone, version, start_time, duration, never_expires "
		"FROM instance_list JOIN instance_list_player "
		"ON instance_list.id = instance_list_player.id "
		"WHERE instance_list_player.charid = {}",
		character_id
	);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	auto character_name = database.GetCharNameByID(character_id);
	bool is_same_client = client->CharacterID() == character_id;
	if (character_name.empty()) {
		client->Message(
			Chat::White,
			fmt::format(
				"Character ID '{}' does not exist.",
				character_id
			).c_str()
		);
		return;
	}

	if (!results.RowCount()) {
		client->Message(
			Chat::White,
			fmt::format(
				"{} not in any Instances.",
				(
					is_same_client ?
					"You are" :
					fmt::format(
						"{} ({}) is",
						character_name,
						character_id
					)
				)
			).c_str()
		);
		return;
	}

	client->Message(
		Chat::White,
		fmt::format(
			"{} in the following Instances.",
			(
				is_same_client ?
				"You are" :
				fmt::format(
					"{} ({}) is",
					character_name,
					character_id
				)
			)
		).c_str()
	);

	uint32 instance_count = 0;
	for (auto row : results) {
		auto instance_id = Strings::ToUnsignedInt(row[0]);
		auto zone_id = Strings::ToUnsignedInt(row[1]);
		auto version = Strings::ToUnsignedInt(row[2]);
		auto start_time = Strings::ToUnsignedInt(row[3]);
		auto duration = Strings::ToUnsignedInt(row[4]);
		auto never_expires = Strings::ToInt(row[5]) ? true : false;
		std::string remaining_time_string = "Never";
		timeval time_value;
		gettimeofday(&time_value, nullptr);
		auto current_time = time_value.tv_sec;
		auto remaining_time = ((start_time + duration) - current_time);
		if (!never_expires) {
			if (remaining_time > 0) {
				remaining_time_string = Strings::SecondsToTime(remaining_time);
			} else {
				remaining_time_string = "Already Expired";
			}
		}

		client->Message(
			Chat::White,
			fmt::format("Instance {} | Zone: {} ({}){}",
				instance_id,
				ZoneLongName(zone_id),
				zone_id,
				(
					version ?
					fmt::format(
						" Version: {}",
						version
					) :
					""
				)
			).c_str()
		);

		client->Message(
			Chat::White,
			fmt::format(
				"Instance {} | Expires: {}",
				instance_id,
				remaining_time_string,
				remaining_time
			).c_str()
		);

		instance_count++;
	}

	client->Message(
		Chat::White,
		fmt::format(
			"{} in {} Instance{}.",
			(
				is_same_client ?
				"You are" :
				fmt::format(
					"{} ({}) is",
					character_name,
					character_id
				)
			),
			instance_count,
			instance_count != 1 ? "s" : ""
		).c_str()
	);
}

void ZoneDatabase::QGlobalPurge()
{
	const std::string query = "DELETE FROM quest_globals WHERE expdate < UNIX_TIMESTAMP()";
	database.QueryDatabase(query);
}

void ZoneDatabase::LoadAltCurrencyValues(uint32 char_id, std::map<uint32, uint32> &currency)
{
	const auto& l = CharacterAltCurrencyRepository::GetWhere(
		*this,
		fmt::format(
			"`char_id` = {}",
			char_id
		)
	);

	if (l.empty()) {
		return;
	}

	for (const auto& e : l) {
		currency[e.currency_id] = e.amount;
	}
}

void ZoneDatabase::UpdateAltCurrencyValue(uint32 char_id, uint32 currency_id, uint32 value)
{
	auto e = CharacterAltCurrencyRepository::NewEntity();

	e.char_id     = char_id;
	e.currency_id = currency_id;
	e.amount      = value;

	CharacterAltCurrencyRepository::ReplaceOne(*this, e);
}

void ZoneDatabase::SaveBuffs(Client *client)
{
	CharacterBuffsRepository::DeleteWhere(
		database,
		fmt::format(
			"`character_id` = {}",
			client->CharacterID()
		)
	);

	auto      buffs          = client->GetBuffs();
	const int max_buff_slots = client->GetMaxBuffSlots();

	std::vector<CharacterBuffsRepository::CharacterBuffs> v;

	auto e = CharacterBuffsRepository::NewEntity();

	uint32 character_buff_count = 0;

	for (int slot_id = 0; slot_id < max_buff_slots; slot_id++) {
		if (!IsValidSpell(buffs[slot_id].spellid)) {
			continue;
		}

		character_buff_count++;
	}

	v.reserve(character_buff_count);

	for (int slot_id = 0; slot_id < max_buff_slots; slot_id++) {
		if (!IsValidSpell(buffs[slot_id].spellid)) {
			continue;
		}

		e.character_id   = client->CharacterID();
		e.slot_id        = slot_id;
		e.spell_id       = buffs[slot_id].spellid;
		e.caster_level   = buffs[slot_id].casterlevel;
		e.caster_name    = buffs[slot_id].caster_name;
		e.ticsremaining  = buffs[slot_id].ticsremaining;
		e.counters       = buffs[slot_id].counters;
		e.numhits        = buffs[slot_id].hit_number;
		e.melee_rune     = buffs[slot_id].melee_rune;
		e.magic_rune     = buffs[slot_id].magic_rune;
		e.persistent     = buffs[slot_id].persistant_buff;
		e.dot_rune       = buffs[slot_id].dot_rune;
		e.caston_x       = buffs[slot_id].caston_x;
		e.caston_y       = buffs[slot_id].caston_y;
		e.caston_z       = buffs[slot_id].caston_z;
		e.ExtraDIChance  = buffs[slot_id].ExtraDIChance;
		e.instrument_mod = buffs[slot_id].instrument_mod;

		v.emplace_back(e);
	}

	if (!v.empty()) {
		CharacterBuffsRepository::ReplaceMany(database, v);
	}
}

void ZoneDatabase::LoadBuffs(Client *client)
{
	auto buffs          = client->GetBuffs();
	int  max_buff_slots = client->GetMaxBuffSlots();

	for (int slot_id = 0; slot_id < max_buff_slots; ++slot_id) {
		buffs[slot_id].spellid = SPELL_UNKNOWN;
	}

	const auto& l = CharacterBuffsRepository::GetWhere(
		*this,
		fmt::format(
			"`character_id` = {}",
			client->CharacterID()
		)
	);

	if (l.empty()) {
		return;
	}

	for (const auto& e : l) {
		if (e.slot_id >= max_buff_slots) {
			continue;
		}

		if (!IsValidSpell(e.spell_id)) {
			continue;
		}

		Client* c = entity_list.GetClientByName(e.caster_name.c_str());

		buffs[e.slot_id].spellid = e.spell_id;
		buffs[e.slot_id].casterlevel = e.caster_level;

		if (c) {
			buffs[e.slot_id].casterid = c->GetID();
			buffs[e.slot_id].client   = true;

			strncpy(buffs[e.slot_id].caster_name, c->GetName(), 64);
		} else {
			buffs[e.slot_id].casterid = 0;
			buffs[e.slot_id].client   = false;

			strncpy(buffs[e.slot_id].caster_name, "", 64);
		}

		buffs[e.slot_id].ticsremaining     = e.ticsremaining;
		buffs[e.slot_id].counters          = e.counters;
		buffs[e.slot_id].hit_number        = e.numhits;
		buffs[e.slot_id].melee_rune        = e.melee_rune;
		buffs[e.slot_id].magic_rune        = e.magic_rune;
		buffs[e.slot_id].persistant_buff   = e.persistent ? true : false;
		buffs[e.slot_id].dot_rune          = e.dot_rune;
		buffs[e.slot_id].caston_x          = e.caston_x;
		buffs[e.slot_id].caston_y          = e.caston_y;
		buffs[e.slot_id].caston_z          = e.caston_z;
		buffs[e.slot_id].ExtraDIChance     = e.ExtraDIChance;
		buffs[e.slot_id].RootBreakChance   = 0;
		buffs[e.slot_id].virus_spread_time = 0;
		buffs[e.slot_id].UpdateClient      = false;
		buffs[e.slot_id].instrument_mod    = e.instrument_mod;
	}

	// We load up to the most our client supports
	max_buff_slots = EQ::spells::StaticLookup(client->ClientVersion())->LongBuffs;

	for (int slot_id = 0; slot_id < max_buff_slots; ++slot_id) {
		if (!IsValidSpell(buffs[slot_id].spellid)) {
			continue;
		}

		if (IsEffectInSpell(buffs[slot_id].spellid, SpellEffect::Charm)) {
			buffs[slot_id].spellid = SPELL_UNKNOWN;
			break;
		}

		if (IsEffectInSpell(buffs[slot_id].spellid, SpellEffect::Illusion)) {
			if (buffs[slot_id].persistant_buff) {
				break;
			}

			buffs[slot_id].spellid = SPELL_UNKNOWN;
			break;
		}
	}
}

void ZoneDatabase::SaveAuras(Client *c)
{
	CharacterAurasRepository::DeleteOne(database, c->CharacterID());

	std::vector<CharacterAurasRepository::CharacterAuras> v;

	auto e = CharacterAurasRepository::NewEntity();

	const auto& auras = c->GetAuraMgr();

	for (int slot_id = 0; slot_id < auras.count; ++slot_id) {
		Aura* a = auras.auras[slot_id].aura;
		if (a && a->AuraZones()) {
			e.id       = c->CharacterID();
			e.slot     = slot_id;
			e.spell_id = a->GetAuraID();

			v.emplace_back(e);
		}
	}

	if (!v.empty()) {
		CharacterAurasRepository::ReplaceMany(database, v);
	}
}

void ZoneDatabase::LoadAuras(Client *c)
{
	const auto& l = CharacterAurasRepository::GetWhere(
		database,
		fmt::format(
			"`id` = {} ORDER BY `slot`",
			c->CharacterID()
		)
	);

	if (l.empty()) {
		return;
	}

	for (const auto& e : l) {
		c->MakeAura(e.spell_id);
	}
}

void ZoneDatabase::SavePetInfo(Client *client)
{
	PetInfo* p = nullptr;

	std::vector<CharacterPetInfoRepository::CharacterPetInfo> pet_infos;
	auto pet_info = CharacterPetInfoRepository::NewEntity();

	std::vector<CharacterPetBuffsRepository::CharacterPetBuffs> pet_buffs;
	auto pet_buff = CharacterPetBuffsRepository::NewEntity();

	std::vector<CharacterPetInventoryRepository::CharacterPetInventory> inventory;
	auto item = CharacterPetInventoryRepository::NewEntity();

	for (int pet_info_type = PetInfoType::Current; pet_info_type <= PetInfoType::Suspended; pet_info_type++) {
		p = client->GetPetInfo(pet_info_type);
		if (!p) {
			continue;
		}

		pet_info.char_id  = client->CharacterID();
		pet_info.pet      = pet_info_type;
		pet_info.petname  = p->Name;
		pet_info.petpower = p->petpower;
		pet_info.spell_id = p->SpellID;
		pet_info.hp       = p->HP;
		pet_info.mana     = p->Mana;
		pet_info.size     = p->size;
		pet_info.taunting = p->taunting ? 1 : 0;

		pet_infos.push_back(pet_info);

		uint32 pet_buff_count = 0;

		const uint32 max_slots = (
			RuleI(Spells, MaxTotalSlotsPET) > PET_BUFF_COUNT ?
			PET_BUFF_COUNT :
			RuleI(Spells, MaxTotalSlotsPET)
		);

		for (int slot_id = 0; slot_id < max_slots; slot_id++) {
			if (!IsValidSpell(p->Buffs[slot_id].spellid)) {
				continue;
			}

			pet_buff_count++;
		}

		pet_buffs.reserve(pet_buff_count);

		for (int slot_id = 0; slot_id < max_slots; slot_id++) {
			if (!IsValidSpell(p->Buffs[slot_id].spellid)) {
				continue;
			}

			pet_buff.char_id        = client->CharacterID();
			pet_buff.pet            = pet_info_type;
			pet_buff.slot           = slot_id;
			pet_buff.spell_id       = p->Buffs[slot_id].spellid;
			pet_buff.caster_level   = p->Buffs[slot_id].level;
			pet_buff.ticsremaining  = p->Buffs[slot_id].duration;
			pet_buff.counters       = p->Buffs[slot_id].counters;
			pet_buff.instrument_mod = p->Buffs[slot_id].bard_modifier;

			pet_buffs.push_back(pet_buff);
		}

		uint32 pet_inventory_count = 0;

		for (
			int slot_id = EQ::invslot::EQUIPMENT_BEGIN;
			slot_id <= EQ::invslot::EQUIPMENT_END;
			slot_id++
		) {
			if (!p->Items[slot_id]) {
				continue;
			}

			pet_inventory_count++;
		}

		inventory.reserve(pet_inventory_count);

		for (
			int slot_id = EQ::invslot::EQUIPMENT_BEGIN;
			slot_id <= EQ::invslot::EQUIPMENT_END;
			slot_id++
		) {
			if (!p->Items[slot_id]) {
				continue;
			}

			item.char_id = client->CharacterID();
			item.pet     = pet_info_type;
			item.slot    = slot_id;
			item.item_id = p->Items[slot_id];

			inventory.push_back(item);
		}
	}

	CharacterPetInfoRepository::DeleteWhere(
		database,
		fmt::format(
			"`char_id` = {}",
			client->CharacterID()
		)
	);

	if (!pet_infos.empty()) {
		CharacterPetInfoRepository::InsertMany(database, pet_infos);
	}

	CharacterPetBuffsRepository::DeleteWhere(
		database,
		fmt::format(
			"`char_id` = {}",
			client->CharacterID()
		)
	);

	if (!pet_buffs.empty()) {
		CharacterPetBuffsRepository::InsertMany(database, pet_buffs);
	}

	CharacterPetInventoryRepository::DeleteWhere(
		database,
		fmt::format(
			"`char_id` = {}",
			client->CharacterID()
		)
	);

	if (!inventory.empty()) {
		CharacterPetInventoryRepository::InsertMany(database, inventory);
	}
}

void ZoneDatabase::RemoveTempFactions(Client *client) {

	std::string query = StringFormat("DELETE FROM faction_values "
                                    "WHERE temp = 1 AND char_id = %u",
                                    client->CharacterID());
	QueryDatabase(query);
}

void ZoneDatabase::UpdateItemRecast(uint32 character_id, uint32 recast_type, uint32 timestamp)
{
	CharacterItemRecastRepository::ReplaceOne(
		*this,
		CharacterItemRecastRepository::CharacterItemRecast{
			.id = character_id,
			.recast_type = recast_type,
			.timestamp = timestamp,
		}
	);
}

void ZoneDatabase::DeleteItemRecast(uint32 character_id, uint32 recast_type)
{
	CharacterItemRecastRepository::DeleteWhere(
		*this,
		fmt::format(
			"`id` = {} AND `recast_type` = {}",
			character_id,
			recast_type
		)
	);
}

void ZoneDatabase::LoadPetInfo(Client *client)
{
	// Load current pet and suspended pet
	auto pet_info           = client->GetPetInfo(PetInfoType::Current);
	auto suspended_pet_info = client->GetPetInfo(PetInfoType::Suspended);

	memset(pet_info, 0, sizeof(PetInfo));
	memset(suspended_pet_info, 0, sizeof(PetInfo));

	const auto& info = CharacterPetInfoRepository::GetWhere(
		database,
		fmt::format(
			"`char_id` = {}",
			client->CharacterID()
		)
	);

	if (info.empty()) {
		return;
	}

	PetInfo* p;

	for (const auto& e : info) {
		if (e.pet == PetInfoType::Current) {
			p = pet_info;
		} else if (e.pet == PetInfoType::Suspended) {
			p = suspended_pet_info;
		} else {
			continue;
		}

		strn0cpy(p->Name, e.petname.c_str(), sizeof(p->Name));

		p->petpower = e.petpower;
		p->SpellID  = e.spell_id;
		p->HP       = e.hp;
		p->Mana     = e.mana;
		p->size     = e.size;
		p->taunting = e.taunting;
	}

	const auto& buffs = CharacterPetBuffsRepository::GetWhere(
		database,
		fmt::format(
			"`char_id` = {}",
			client->CharacterID()
		)
	);

	if (!buffs.empty()) {
		for (const auto& e : buffs) {
			if (e.pet == PetInfoType::Current) {
				p = pet_info;
			} else if (e.pet == PetInfoType::Suspended) {
				p = suspended_pet_info;
			} else {
				continue;
			}

			if (e.slot >= RuleI(Spells, MaxTotalSlotsPET)) {
				continue;
			}

			if (!IsValidSpell(e.spell_id)) {
				continue;
			}

			p->Buffs[e.slot].spellid       = e.spell_id;
			p->Buffs[e.slot].level         = e.caster_level;
			p->Buffs[e.slot].player_id     = 0;
			p->Buffs[e.slot].effect_type   = BuffEffectType::Buff;
			p->Buffs[e.slot].duration      = e.ticsremaining;
			p->Buffs[e.slot].counters      = e.counters;
			p->Buffs[e.slot].bard_modifier = e.instrument_mod;
		}
	}

	const auto& inventory = CharacterPetInventoryRepository::GetWhere(
		database,
		fmt::format(
			"`char_id` = {}",
			client->CharacterID()
		)
	);

	if (!inventory.empty()) {
		for (const auto& e : inventory) {
			if (e.pet == PetInfoType::Current) {
				p = pet_info;
			} else if (e.pet == PetInfoType::Suspended) {
				p = suspended_pet_info;
			} else {
				continue;
			}

			if (!EQ::ValueWithin(e.slot, EQ::invslot::EQUIPMENT_BEGIN, EQ::invslot::EQUIPMENT_END)) {
				continue;
			}

			p->Items[e.slot] = e.item_id;
		}
	}
}

bool ZoneDatabase::GetFactionData(FactionMods* fm, uint32 class_mod, uint32 race_mod, uint32 deity_mod, int32 faction_id) {
	if (faction_id <= 0 || faction_id > (int32) max_faction)
		return false;

	if (faction_array[faction_id] == 0){
		return false;
	}

	fm->base = faction_array[faction_id]->base;
	fm->min = faction_array[faction_id]->min; // The lowest your personal earned faction can go - before race/class/deity adjustments.
	fm->max = faction_array[faction_id]->max; // The highest your personal earned faction can go - before race/class/deity adjustments.

	if(class_mod > 0) {
		char str[32];
		sprintf(str, "c%u", class_mod);

		std::map<std::string, int16>::const_iterator iter = faction_array[faction_id]->mods.find(str);
		if(iter != faction_array[faction_id]->mods.end()) {
			fm->class_mod = iter->second;
		} else {
			fm->class_mod = 0;
		}
	} else {
		fm->class_mod = 0;
	}

	if(race_mod > 0) {
		char str[32];
		sprintf(str, "r%u", race_mod);

		auto iter = faction_array[faction_id]->mods.find(str);
		if(iter != faction_array[faction_id]->mods.end()) {
			fm->race_mod = iter->second;
		} else {
			fm->race_mod = 0;
		}
	} else {
		fm->race_mod = 0;
	}

	if(deity_mod > 0) {
		char str[32];
		sprintf(str, "d%u", deity_mod);

		auto iter = faction_array[faction_id]->mods.find(str);
		if(iter != faction_array[faction_id]->mods.end()) {
			fm->deity_mod = iter->second;
		} else {
			fm->deity_mod = 0;
		}
	} else {
		fm->deity_mod = 0;
	}

	return true;
}

//o--------------------------------------------------------------
//| Name: GetFactionName; Dec. 16
//o--------------------------------------------------------------
//| Notes: Retrieves the name of the specified faction .Returns false on failure.
//o--------------------------------------------------------------
bool ZoneDatabase::GetFactionName(int32 faction_id, char* name, uint32 buflen) {
	if ((faction_id <= 0) || faction_id > int32(max_faction) ||(faction_array[faction_id] == 0))
		return false;
	if (faction_array[faction_id]->name[0] != 0) {
		strn0cpy(name, faction_array[faction_id]->name, buflen);
		return true;
	}
	return false;

}

std::string ZoneDatabase::GetFactionName(int32 faction_id)
{
	std::string faction_name;
	if (
		faction_id <= 0 ||
		 faction_id > static_cast<int>(max_faction) ||
		 !faction_array[faction_id]
	) {
		return faction_name;
	}

	faction_name = faction_array[faction_id]->name;

	return faction_name;
}

//o--------------------------------------------------------------
//| Name: SetCharacterFactionLevel; Dec. 20, 2001
//o--------------------------------------------------------------
//| Purpose: Update characters faction level with specified faction_id to specified value. Returns false on failure.
//o--------------------------------------------------------------
bool ZoneDatabase::SetCharacterFactionLevel(uint32 char_id, int32 faction_id, int32 value, uint8 temp, faction_map &val_list)
{

	std::string query;

	if(temp == 2)
		temp = 0;

	if(temp == 3)
		temp = 1;

	query = StringFormat("INSERT INTO `faction_values` "
						"(`char_id`, `faction_id`, `current_value`, `temp`) "
						"VALUES (%i, %i, %i, %i) "
						"ON DUPLICATE KEY UPDATE `current_value`=%i,`temp`=%i",
						char_id, faction_id, value, temp, value, temp);
    auto results = QueryDatabase(query);

	if (!results.Success())
		return false;
	else
		val_list[faction_id] = value;

	return true;
}

bool ZoneDatabase::LoadFactionData()
{
	std::string query("SELECT MAX(`id`) FROM `faction_list`");

	auto faction_max_results = QueryDatabase(query);
	if (!faction_max_results.Success() || faction_max_results.RowCount() == 0) {
		return false;
	}

    auto& fmr_row = faction_max_results.begin();
	if (fmr_row[0] == nullptr) {
		return false;
	}

	max_faction = Strings::ToUnsignedInt(fmr_row[0]);
	faction_array = new Faction *[max_faction + 1];

	memset(faction_array, 0, (sizeof(Faction*) * (max_faction + 1)));

	std::vector<std::string> faction_ids;

	// load factions
    query = "SELECT `id`, `name`, `base` FROM `faction_list`";

    auto faction_results = QueryDatabase(query);
    if (!faction_results.Success()) {
        return false;
    }

	for (auto fr_row : faction_results) {

		uint32 index = Strings::ToUnsignedInt(fr_row[0]);
		if (index > max_faction) {
			Log(Logs::General, Logs::Error, "Faction '%u' is out-of-bounds for faction array size!", index);
			continue;
		}

		// this should never hit since `id` is keyed..but, it alleviates any risk of lost pointers
		if (faction_array[index] != nullptr) {
			Log(Logs::General, Logs::Error, "Faction '%u' has already been assigned! (Duplicate Entry)", index);
			continue;
		}

		faction_array[index] = new Faction;
		strn0cpy(faction_array[index]->name, fr_row[1], 50);
		faction_array[index]->base = Strings::ToInt(fr_row[2]);
		faction_array[index]->min = MIN_PERSONAL_FACTION;
		faction_array[index]->max = MAX_PERSONAL_FACTION;

		faction_ids.push_back(fr_row[0]);
	}

	LogInfo("Loaded [{}] faction(s)", Strings::Commify(std::to_string(faction_ids.size())));

	const std::string faction_id_criteria(Strings::Implode(",", faction_ids));

	// load faction mins/maxes
	query = fmt::format("SELECT `client_faction_id`, `min`, `max` FROM `faction_base_data` WHERE `client_faction_id` IN ({})", faction_id_criteria);

	auto base_results = QueryDatabase(query);
	if (base_results.Success()) {

		for (auto br_row : base_results) {

			uint32 index = Strings::ToUnsignedInt(br_row[0]);
			if (index > max_faction) {
				LogError("Faction [{}] is out-of-bounds for faction array size in Base adjustment!", index);
				continue;
			}

			if (faction_array[index] == nullptr) {
				LogError("Faction [{}] does not exist for Base adjustment!", index);
				continue;
			}

			faction_array[index]->min = Strings::ToInt(br_row[1]);
			faction_array[index]->max = Strings::ToInt(br_row[2]);
		}

		LogInfo("Loaded [{}] faction base(s)", Strings::Commify(std::to_string(base_results.RowCount())));
	}
	else {
		LogInfo("Unable to load Faction Base data...");
	}

	// load race, class and deity modifiers
	query = fmt::format("SELECT `faction_id`, `mod`, `mod_name` FROM `faction_list_mod` WHERE `faction_id` IN ({})", faction_id_criteria);

	auto modifier_results = QueryDatabase(query);
	if (modifier_results.Success()) {

		for (auto mr_row : modifier_results) {

			uint32 index = Strings::ToUnsignedInt(mr_row[0]);
			if (index > max_faction) {
				Log(Logs::General, Logs::Error, "Faction '%u' is out-of-bounds for faction array size in Modifier adjustment!", index);
				continue;
			}

			if (faction_array[index] == nullptr) {
				Log(Logs::General, Logs::Error, "Faction '%u' does not exist for Modifier adjustment!", index);
				continue;
			}

			faction_array[index]->mods[mr_row[2]] = Strings::ToInt(mr_row[1]);
		}

		LogInfo("Loaded [{}] faction modifier(s)", Strings::Commify(std::to_string(modifier_results.RowCount())));
	}
	else {
		LogError("Unable to load Faction Modifier data");
	}

	return true;
}

bool ZoneDatabase::GetFactionIDsForNPC(
	uint32 npc_faction_id,
	std::list<NpcFactionEntriesRepository::NpcFactionEntries> *faction_list,
	int32* primary_faction
)
{
	if (npc_faction_id <= 0) {
		faction_list->clear();

		if (primary_faction) {
			*primary_faction = npc_faction_id;
		}

		return true;
	}

	const auto& npcf = zone->GetNPCFaction(npc_faction_id);
	if (!npcf) {
		LogError("No NPC faction entry for [{}]", npc_faction_id);
		return false;
	}

	const auto& l = zone->GetNPCFactionEntries(npc_faction_id);

	if (primary_faction) {
		*primary_faction = npcf->primaryfaction;
	}

	faction_list->clear();

	for (const auto& e: l) {
		faction_list->emplace_back(e);
	}

	return true;
}

uint32 ZoneDatabase::SendCharacterCorpseToGraveyard(
	uint32 corpse_id,
	uint32 zone_id,
	uint16 instance_id,
	glm::vec4& position
)
{
	position.x += zone->random.Real(-20, 20);
	position.y += zone->random.Real(-20, 20);

	return CharacterCorpsesRepository::SendToGraveyard(*this, corpse_id, zone_id, instance_id, position);
}

void ZoneDatabase::SendCharacterCorpseToNonInstance(uint32 corpse_id)
{
	CharacterCorpsesRepository::SendToNonInstance(*this, corpse_id);
}

uint32 ZoneDatabase::GetCharacterCorpseDecayTimer(uint32 corpse_id)
{
	return CharacterCorpsesRepository::GetDecayTimer(*this, corpse_id);
}

uint32 ZoneDatabase::UpdateCharacterCorpse(
	uint32 corpse_id,
	uint32 character_id,
	const std::string& name,
	uint32 zone_id,
	uint16 instance_id,
	const CharacterCorpseEntry& c,
	const glm::vec4& position,
	uint32 guild_consent_id,
	bool is_resurrected
)
{
	auto e = CharacterCorpsesRepository::FindOne(*this, corpse_id);

	e.charname         = name;
	e.zone_id          = zone_id;
	e.instance_id      = instance_id;
	e.charid           = character_id;
	e.x                = position.x;
	e.y                = position.y;
	e.z                = position.z;
	e.heading          = position.w;
	e.guild_consent_id = guild_consent_id;
	e.is_locked        = c.locked;
	e.exp              = c.exp;
	e.size             = c.size;
	e.level            = c.level;
	e.race             = c.race;
	e.gender           = c.gender;
	e.class_           = c.class_;
	e.deity            = c.deity;
	e.texture          = c.texture;
	e.helm_texture     = c.helmtexture;
	e.copper           = c.copper;
	e.silver           = c.silver;
	e.gold             = c.gold;
	e.platinum         = c.plat;
	e.hair_color       = c.haircolor;
	e.beard_color      = c.beardcolor;
	e.eye_color_1      = c.eyecolor1;
	e.eye_color_2      = c.eyecolor2;
	e.hair_style       = c.hairstyle;
	e.face             = c.face;
	e.beard            = c.beard;
	e.drakkin_heritage = c.drakkin_heritage;
	e.drakkin_details  = c.drakkin_details;
	e.drakkin_tattoo   = c.drakkin_tattoo;
	e.wc_1             = c.item_tint.Head.Color;
	e.wc_2             = c.item_tint.Chest.Color;
	e.wc_3             = c.item_tint.Arms.Color;
	e.wc_4             = c.item_tint.Wrist.Color;
	e.wc_5             = c.item_tint.Hands.Color;
	e.wc_6             = c.item_tint.Legs.Color;
	e.wc_7             = c.item_tint.Feet.Color;
	e.wc_8             = c.item_tint.Primary.Color;
	e.wc_9             = c.item_tint.Secondary.Color;

	CharacterCorpsesRepository::UpdateOne(*this, e);

	return corpse_id;
}

uint32 ZoneDatabase::UpdateCharacterCorpseConsent(uint32 character_id, uint32 guild_consent_id)
{
	return CharacterCorpsesRepository::SetGuildConsentID(*this, character_id, guild_consent_id);
}

void ZoneDatabase::MarkCorpseAsResurrected(uint32 corpse_id)
{
	CharacterCorpsesRepository::ResurrectCorpse(*this, corpse_id);
}

uint32 ZoneDatabase::SaveCharacterCorpse(
	uint32 character_id,
	const std::string& name,
	uint32 zone_id,
	uint16 instance_id,
	const CharacterCorpseEntry& c,
	const glm::vec4& position,
	uint32 guild_consent_id
)
{
	auto e = CharacterCorpsesRepository::NewEntity();

	e.charname         = name;
	e.zone_id          = zone_id;
	e.instance_id      = instance_id;
	e.charid           = character_id;
	e.x                = position.x;
	e.y                = position.y;
	e.z                = position.z;
	e.heading          = position.w;
	e.guild_consent_id = guild_consent_id;
	e.time_of_death    = std::time(nullptr);
	e.is_locked        = c.locked;
	e.exp              = c.exp;
	e.size             = c.size;
	e.level            = c.level;
	e.race             = c.race;
	e.gender           = c.gender;
	e.class_           = c.class_;
	e.deity            = c.deity;
	e.texture          = c.texture;
	e.helm_texture     = c.helmtexture;
	e.copper           = c.copper;
	e.silver           = c.silver;
	e.gold             = c.gold;
	e.platinum         = c.plat;
	e.hair_color       = c.haircolor;
	e.beard_color      = c.beardcolor;
	e.eye_color_1      = c.eyecolor1;
	e.eye_color_2      = c.eyecolor2;
	e.hair_style       = c.hairstyle;
	e.face             = c.face;
	e.beard            = c.beard;
	e.drakkin_heritage = c.drakkin_heritage;
	e.drakkin_tattoo   = c.drakkin_tattoo;
	e.drakkin_details  = c.drakkin_details;
	e.wc_1             = c.item_tint.Head.Color;
	e.wc_2             = c.item_tint.Chest.Color;
	e.wc_3             = c.item_tint.Arms.Color;
	e.wc_4             = c.item_tint.Wrist.Color;
	e.wc_5             = c.item_tint.Hands.Color;
	e.wc_6             = c.item_tint.Legs.Color;
	e.wc_7             = c.item_tint.Feet.Color;
	e.wc_8             = c.item_tint.Primary.Color;
	e.wc_9             = c.item_tint.Secondary.Color;
	e.killed_by        = c.killed_by;
	e.rezzable         = c.rezzable;
	e.rez_time         = c.rez_time;

	e = CharacterCorpsesRepository::InsertOne(*this, e);

	std::vector<CharacterCorpseItemsRepository::CharacterCorpseItems> v;

	v.reserve(c.items.size());

	auto ci = CharacterCorpseItemsRepository::NewEntity();

	for (const auto& i : c.items) {
		ci.corpse_id           = e.id;
		ci.equip_slot          = i.equip_slot;
		ci.item_id             = i.item_id;
		ci.charges             = i.charges;
		ci.aug_1               = i.aug_1;
		ci.aug_2               = i.aug_2;
		ci.aug_3               = i.aug_3;
		ci.aug_4               = i.aug_4;
		ci.aug_5               = i.aug_5;
		ci.aug_6               = i.aug_6;
		ci.attuned             = i.attuned;
		ci.custom_data         = i.custom_data;
		ci.ornamenticon        = i.ornamenticon;
		ci.ornamentidfile      = i.ornamentidfile;
		ci.ornament_hero_model = i.ornament_hero_model;

		v.emplace_back(ci);
	}

	if (!v.empty()) {
		CharacterCorpseItemsRepository::ReplaceMany(*this, v);
	}

	return e.id;
}

uint32 ZoneDatabase::GetCharacterBuriedCorpseCount(uint32 character_id)
{
	return CharacterCorpsesRepository::Count(
		*this,
		fmt::format(
			"`charid` = {} AND `is_buried` = 1",
			character_id
		)
	);
}

int64 ZoneDatabase::GetCharacterCorpseCount(uint32 character_id)
{
	return CharacterCorpsesRepository::Count(
		*this,
		fmt::format(
			"`charid` = {}",
			character_id
		)
	);
}

uint32 ZoneDatabase::GetCharacterCorpseID(uint32 character_id, uint8 corpse_limit)
{
	const auto& l = CharacterCorpsesRepository::GetWhere(
		*this,
		fmt::format(
			"`charid` = {} LIMIT {}, 1",
			character_id,
			corpse_limit
		)
	);

	return l.empty() ? 0 : l[0].id;
}

uint32 ZoneDatabase::GetCharacterCorpseItemAt(uint32 corpse_id, uint16 slot_id)
{
	LogCorpsesDetail("corpse_id [{}] slot_id [{}]", corpse_id, slot_id);

	Corpse* c = LoadCharacterCorpse(corpse_id);
	uint32 item_id = 0;

	if (c) {
		item_id = c->GetWornItem(slot_id);
		c->DepopPlayerCorpse();
	}

	return item_id;
}

Corpse* ZoneDatabase::SummonBuriedCharacterCorpses(
	uint32 character_id,
	uint32 zone_id,
	uint16 instance_id,
	const glm::vec4& position
)
{
	Corpse* c = nullptr;

	const auto& l = CharacterCorpsesRepository::GetWhere(
		*this,
		fmt::format(
			"`charid` = {} AND `is_buried` = 1 ORDER BY `time_of_death` LIMIT 1",
			character_id
		)
	);

	for (const auto& e : l) {
		c = Corpse::LoadCharacterCorpse(e, position);
		if (!c) {
			continue;
		}

		entity_list.AddCorpse(c);
		c->SetDecayTimer(RuleI(Character, CorpseDecayTime));
		c->Spawn();

		if (!UnburyCharacterCorpse(c->GetCorpseDBID(), zone_id, instance_id, position)) {
			LogError("Unable to unbury a summoned player corpse_id [{}] for character_id [{}]", e.id, character_id);
		}
	}

	return c;
}

bool ZoneDatabase::SummonAllCharacterCorpses(
	uint32 character_id,
	uint32 zone_id,
	uint16 instance_id,
	const glm::vec4& position
)
{
	Corpse* c = nullptr;
	int64 corpse_count = 0;

	auto l = CharacterCorpsesRepository::GetWhere(
		*this,
		fmt::format(
			"`charid` = {}",
			character_id
		)
	);

	for (auto& e : l) {
		e.zone_id          = zone_id;
		e.instance_id      = instance_id;
		e.x                = position.x;
		e.y                = position.y;
		e.z                = position.z;
		e.heading          = position.w;
		e.is_buried        = 0;
		e.was_at_graveyard = 0;

		c = Corpse::LoadCharacterCorpse(e, position);
		if (c) {
			entity_list.AddCorpse(c);
			c->SetDecayTimer(RuleI(Character, CorpseDecayTime));
			c->Spawn();
			++corpse_count;
		} else {
			LogError("Unable to construct a player corpse_id [{}] for character_id [{}]", e.id, character_id);
		}
	}

	if (!l.empty()) {
		CharacterCorpsesRepository::ReplaceMany(*this, l);
	}

	return corpse_count > 0;
}

int64 ZoneDatabase::CountCharacterCorpses(uint32 character_id)
{
	return CharacterCorpsesRepository::Count(
		*this,
		fmt::format(
			"`charid` = {}",
			character_id
		)
	);
}

int64 ZoneDatabase::CountCharacterCorpsesByZoneID(uint32 character_id, uint32 zone_id)
{
	return CharacterCorpsesRepository::Count(
		*this,
		fmt::format(
			"`charid` = {} AND `zone_id` = {}",
			character_id,
			zone_id
		)
	);
}

bool ZoneDatabase::UnburyCharacterCorpse(uint32 corpse_id, uint32 zone_id, uint16 instance_id, const glm::vec4& position)
{
	return CharacterCorpsesRepository::UnburyCorpse(*this, corpse_id, zone_id, instance_id, position);
}

Corpse* ZoneDatabase::LoadCharacterCorpse(uint32 corpse_id)
{
	if (!corpse_id) {
		return nullptr;
	}

	const auto &e = CharacterCorpsesRepository::FindOne(*this, corpse_id);
	if (!e.id) {
		return nullptr;
	}

	auto c = Corpse::LoadCharacterCorpse(e, glm::vec4(e.x, e.y, e.z, e.heading));

	entity_list.AddCorpse(c);

	return c;
}

bool ZoneDatabase::LoadCharacterCorpses(uint32 zone_id, uint16 instance_id)
{
	const auto& l = CharacterCorpsesRepository::GetWhere(
		*this,
		fmt::format(
			"`zone_id` = {} AND `instance_id` = {}{}",
			zone_id,
			instance_id,
			RuleB(Zone, EnableShadowrest) ? " AND `is_buried` = 0" : ""
		)
	);

	for (const auto &e: l) {
		glm::vec4 position = glm::vec4(e.x, e.y, e.z, e.heading);
		entity_list.AddCorpse(Corpse::LoadCharacterCorpse(e, position));
	}

	return true;
}

uint32 ZoneDatabase::GetFirstCorpseID(uint32 character_id)
{
	const auto& l = CharacterCorpsesRepository::GetWhere(
		*this,
		fmt::format(
			"`charid` = {} AND `is_buried` = 0 ORDER BY `time_of_death` LIMIT 1",
			character_id
		)
	);

	if (l.empty()) {
		return 0;
	}

	return l[0].id;
}

bool ZoneDatabase::DeleteItemOffCharacterCorpse(uint32 corpse_id, uint32 slot_id, uint32 item_id)
{
	return CharacterCorpseItemsRepository::DeleteWhere(
		*this,
		fmt::format(
			"`corpse_id` = {} AND `equip_slot` = {} AND `item_id` = {}",
			corpse_id,
			slot_id,
			item_id
		)
	);
}

bool ZoneDatabase::BuryCharacterCorpse(uint32 corpse_id)
{
	return CharacterCorpsesRepository::BuryCorpse(*this, corpse_id);
}

bool ZoneDatabase::BuryAllCharacterCorpses(uint32 character_id)
{
	const auto& l = CharacterCorpsesRepository::GetWhere(
		*this,
		fmt::format(
			"`charid` = {}",
			character_id
		)
	);

	if (l.empty()) {
		return false;
	}

	for (const auto& e : l) {
		BuryCharacterCorpse(e.id);
	}

	return true;
}

bool ZoneDatabase::DeleteCharacterCorpse(uint32 corpse_id)
{
	return CharacterCorpsesRepository::DeleteOne(*this, corpse_id);
}

void ZoneDatabase::UpdateGMStatus(uint32 account_id, int new_status)
{
	auto e = AccountRepository::FindOne(*this, account_id);
	if (!e.id) {
		return;
	}

	e.status = new_status;

	AccountRepository::UpdateOne(*this, e);
}

void ZoneDatabase::SaveCharacterBinds(Client *c)
{
	std::vector<CharacterBindRepository::CharacterBind> v;

	auto e = CharacterBindRepository::NewEntity();

	uint32 bind_count = 0;
	for (const auto &b : c->GetPP().binds) {
		if (b.zone_id) {
			bind_count++;
		}
	}

	v.reserve(bind_count);

	int slot_id = 0;

	for (const auto &b : c->GetPP().binds) {
		if (b.zone_id) {
			e.id          = c->CharacterID();
			e.zone_id     = b.zone_id;
			e.instance_id = b.instance_id;
			e.x           = b.x;
			e.y           = b.y;
			e.z           = b.z;
			e.heading     = b.heading;
			e.slot        = slot_id;

			v.emplace_back(e);

			slot_id++;
		}
	}

	if (bind_count > 0) {
		CharacterBindRepository::ReplaceMany(database, v);
	}
}

void ZoneDatabase::ZeroPlayerProfileCurrency(PlayerProfile_Struct* pp)
{
	if (pp->copper < 0) {
		pp->copper = 0;
	}

	if (pp->silver < 0) {
		pp->silver = 0;
	}

	if (pp->gold < 0) {
		pp->gold = 0;
	}

	if (pp->platinum < 0) {
		pp->platinum = 0;
	}

	if (pp->copper_bank < 0) {
		pp->copper_bank = 0;
	}

	if (pp->silver_bank < 0) {
		pp->silver_bank = 0;
	}

	if (pp->gold_bank < 0) {
		pp->gold_bank = 0;
	}

	if (pp->platinum_bank < 0) {
		pp->platinum_bank = 0;
	}

	if (pp->platinum_cursor < 0) {
		pp->platinum_cursor = 0;
	}

	if (pp->gold_cursor < 0) {
		pp->gold_cursor = 0;
	}

	if (pp->silver_cursor < 0) {
		pp->silver_cursor = 0;
	}

	if (pp->copper_cursor < 0) {
		pp->copper_cursor = 0;
	}
}

float ZoneDatabase::GetAAEXPModifierByCharID(
	uint32 character_id,
	uint32 zone_id,
	int16 instance_version
)
{
	EXPModifier m = CharacterExpModifiersRepository::GetEXPModifier(
		database,
		character_id,
		zone_id,
		instance_version
	);

	return m.aa_modifier;
}

float ZoneDatabase::GetEXPModifierByCharID(
	uint32 character_id,
	uint32 zone_id,
	int16 instance_version
)
{
	EXPModifier m = CharacterExpModifiersRepository::GetEXPModifier(
		database,
		character_id,
		zone_id,
		instance_version
	);

	return m.exp_modifier;
}

void ZoneDatabase::SetAAEXPModifierByCharID(
	uint32 character_id,
	uint32 zone_id,
	float aa_modifier,
	int16 instance_version
)
{
	CharacterExpModifiersRepository::SetEXPModifier(
		database,
		character_id,
		zone_id,
		instance_version,
		EXPModifier{
			.aa_modifier = aa_modifier,
			.exp_modifier = zone->GetEXPModifierByCharacterID(character_id)
		}
	);
}

void ZoneDatabase::SetEXPModifierByCharID(
	uint32 character_id,
	uint32 zone_id,
	float exp_modifier,
	int16 instance_version
)
{
	CharacterExpModifiersRepository::SetEXPModifier(
		database,
		character_id,
		zone_id,
		instance_version,
		EXPModifier{
			.aa_modifier = zone->GetAAEXPModifierByCharacterID(character_id),
			.exp_modifier = exp_modifier
		}
	);
}

void ZoneDatabase::LoadCharacterEXPModifier(Client* c)
{
	if (!zone) {
		return;
	}

	EXPModifier m = CharacterExpModifiersRepository::GetEXPModifier(
		*this,
		c->CharacterID(),
		zone->GetZoneID(),
		zone->GetInstanceVersion()
	);

	zone->exp_modifiers[c->CharacterID()] = m;
}

void ZoneDatabase::SaveCharacterEXPModifier(Client* c)
{
	if (!zone) {
		return;
	}

	EXPModifier m = zone->exp_modifiers[c->CharacterID()];

	CharacterExpModifiersRepository::ReplaceOne(
		*this,
		CharacterExpModifiersRepository::CharacterExpModifiers{
			.character_id = static_cast<int32_t>(c->CharacterID()),
			.zone_id = static_cast<int32_t>(zone->GetZoneID()),
			.instance_version = zone->GetInstanceVersion(),
			.aa_modifier = m.aa_modifier,
			.exp_modifier = m.exp_modifier
		}
	);
}

void ZoneDatabase::LoadCharacterTitleSets(Client* c)
{
	if (!zone || !c) {
		return;
	}

	const auto& l = PlayerTitlesetsRepository::GetWhere(
		*this,
		fmt::format(
			"`char_id` = {}",
			c->CharacterID()
		)
	);

	if (l.empty()) {
		return;
	}

	const uint32 character_id = c->CharacterID();

	for (const auto& e : l) {
		c->EnableTitle(e.title_set, false);
	}
}
