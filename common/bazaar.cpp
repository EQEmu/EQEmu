#include "bazaar.h"

#include "common/item_instance.h"
#include "common/repositories/trader_repository.h"

#include <memory>

std::vector<BazaarSearchResultsFromDB_Struct>
Bazaar::GetSearchResults(
	Database &db,
	Database &content_db,
	BazaarSearchCriteria_Struct search,
	uint32 char_zone_id,
	int32 char_zone_instance_id
)
{
	LogTrading(
		"Searching for items with search criteria - item_name [{}] min_cost [{}] max_cost [{}] min_level [{}] "
		"max_level [{}] max_results [{}] prestige [{}] augment [{}] trader_entity_id [{}] trader_id [{}] "
		"search_scope [{}] char_zone_id [{}], char_zone_instance_id [{}]",
		search.item_name,
		search.min_cost,
		search.max_cost,
		search.min_level,
		search.max_level,
		search.max_results,
		search.prestige,
		search.augment,
		search.trader_entity_id,
		search.trader_id,
		search.search_scope,
		char_zone_id,
		char_zone_instance_id
	);

	static std::map<uint8, uint32> item_slot_searches_new = {
		{EQ::invslot::slotCharm,       1},
		{EQ::invslot::slotEar1,        2},
		{EQ::invslot::slotHead,        4},
		{EQ::invslot::slotFace,        8},
		{EQ::invslot::slotEar2,        16},
		{EQ::invslot::slotNeck,        32},
		{EQ::invslot::slotShoulders,   64},
		{EQ::invslot::slotArms,        128},
		{EQ::invslot::slotBack,        256},
		{EQ::invslot::slotWrist1,      512},
		{EQ::invslot::slotWrist2,      1024},
		{EQ::invslot::slotRange,       2048},
		{EQ::invslot::slotHands,       4096},
		{EQ::invslot::slotPrimary,     8192},
		{EQ::invslot::slotSecondary,   16384},
		{EQ::invslot::slotFinger1,     32768},
		{EQ::invslot::slotFinger2,     65536},
		{EQ::invslot::slotChest,       131072},
		{EQ::invslot::slotLegs,        262144},
		{EQ::invslot::slotFeet,        524288},
		{EQ::invslot::slotWaist,       1048576},
		{EQ::invslot::slotPowerSource, 2097152},
		{EQ::invslot::slotAmmo,        4194304},
	};

	struct ItemSearchType {
		EQ::item::ItemType type;
		std::string        condition;
	};

	std::vector<ItemSearchType> item_search_types_new = {
		{EQ::item::ItemType::ItemTypeBook,                 " AND (items.itemclass = 2 or items.itemclass = 31)"},
		{EQ::item::ItemType::ItemTypeContainer,            " AND (items.itemclass = 1 or items.itemclass = 67)"},
		{EQ::item::ItemType::ItemTypeAllEffects,           " AND (items.scrolleffect > 0 && items.scrolleffect < 65000)"},
		{EQ::item::ItemType::ItemTypeUnknown9,             " AND items.worneffect = 998"},
		{EQ::item::ItemType::ItemTypeUnknown10,            " AND (items.worneffect >= 1298 && items.worneffect <= 1307)"},
		{EQ::item::ItemType::ItemTypeFocusEffect,          " AND items.focuseffect > 0"},
		{EQ::item::ItemType::ItemTypeArmor,                " AND items.itemtype = 10"},
		{EQ::item::ItemType::ItemType1HBlunt,              " AND items.itemtype = 3"},
		{EQ::item::ItemType::ItemType1HPiercing,           " AND items.itemtype = 2"},
		{EQ::item::ItemType::ItemType1HSlash,              " AND items.itemtype = 0"},
		{EQ::item::ItemType::ItemType2HBlunt,              " AND items.itemtype = 4"},
		{EQ::item::ItemType::ItemType2HSlash,              " AND items.itemtype = 1"},
		{EQ::item::ItemType::ItemTypeBow,                  " AND items.itemtype = 5"},
		{EQ::item::ItemType::ItemTypeShield,               " AND items.itemtype = 8"},
		{EQ::item::ItemType::ItemTypeMisc,                 " AND items.itemtype = 11"},
		{EQ::item::ItemType::ItemTypeFood,                 " AND items.itemtype = 14"},
		{EQ::item::ItemType::ItemTypeDrink,                " AND items.itemtype = 15"},
		{EQ::item::ItemType::ItemTypeLight,                " AND items.itemtype = 16"},
		{EQ::item::ItemType::ItemTypeCombinable,           " AND items.itemtype = 17"},
		{EQ::item::ItemType::ItemTypeBandage,              " AND items.itemtype = 18"},
		{EQ::item::ItemType::ItemTypeSmallThrowing,        " AND (items.itemtype = 19 OR items.itemtype = 7)"},
		{EQ::item::ItemType::ItemTypeSpell,                " AND items.itemtype = 20"},
		{EQ::item::ItemType::ItemTypePotion,               " AND items.itemtype = 21"},
		{EQ::item::ItemType::ItemTypeBrassInstrument,      " AND items.itemtype = 25"},
		{EQ::item::ItemType::ItemTypeWindInstrument,       " AND items.itemtype = 23"},
		{EQ::item::ItemType::ItemTypeStringedInstrument,   " AND items.itemtype = 24"},
		{EQ::item::ItemType::ItemTypePercussionInstrument, " AND items.itemtype = 26"},
		{EQ::item::ItemType::ItemTypeArrow,                " AND items.itemtype = 27"},
		{EQ::item::ItemType::ItemTypeJewelry,              " AND items.itemtype = 29"},
		{EQ::item::ItemType::ItemTypeNote,                 " AND items.itemtype = 32"},
		{EQ::item::ItemType::ItemTypeKey,                  " AND items.itemtype = 33"},
		{EQ::item::ItemType::ItemType2HPiercing,           " AND items.itemtype = 35"},
		{EQ::item::ItemType::ItemTypeAlcohol,              " AND items.itemtype = 38"},
		{EQ::item::ItemType::ItemTypeMartial,              " AND items.itemtype = 45"},
		{EQ::item::ItemType::ItemTypeAugmentation,         " AND items.itemtype = 54"},
		{EQ::item::ItemType::ItemTypeAlternateAbility,     " AND items.itemtype = 57"},
		{EQ::item::ItemType::ItemTypeCount,                " AND items.itemtype = 65"},
		{EQ::item::ItemType::ItemTypeCollectible,          " AND items.itemtype = 66"}
	};

	// item stat searches
	struct ItemStatSearch {
		std::string           query_string;
		EQ::skills::SkillType skill_type;
	};

	std::map<uint32, ItemStatSearch> item_stat_searches_new = {
		{STAT_AC,                  {" items.ac" ,          static_cast<EQ::skills::SkillType>(0)} },
		{STAT_AGI,                 {" items.aagi",         static_cast<EQ::skills::SkillType>(0)} },
		{STAT_CHA,                 {" items.acha",         static_cast<EQ::skills::SkillType>(0)} },
		{STAT_DEX,                 {" items.adex",         static_cast<EQ::skills::SkillType>(0)} },
		{STAT_INT,                 {" items.aint",         static_cast<EQ::skills::SkillType>(0)} },
		{STAT_STA,                 {" items.asta",         static_cast<EQ::skills::SkillType>(0)} },
		{STAT_STR,                 {" items.astr",         static_cast<EQ::skills::SkillType>(0)} },
		{STAT_WIS,                 {" items.awis",         static_cast<EQ::skills::SkillType>(0)} },
		{STAT_COLD,                {" items.cr",           static_cast<EQ::skills::SkillType>(0)} },
		{STAT_DISEASE,             {" items.dr",           static_cast<EQ::skills::SkillType>(0)} },
		{STAT_FIRE,                {" items.fr",           static_cast<EQ::skills::SkillType>(0)} },
		{STAT_MAGIC,               {" items.mr",           static_cast<EQ::skills::SkillType>(0)} },
		{STAT_POISON,              {" items.pr",           static_cast<EQ::skills::SkillType>(0)} },
		{STAT_HP,                  {" items.hp",           static_cast<EQ::skills::SkillType>(0)} },
		{STAT_MANA,                {" items.mana",         static_cast<EQ::skills::SkillType>(0)} },
		{STAT_ENDURANCE,           {" items.endur",        static_cast<EQ::skills::SkillType>(0)} },
		{STAT_ATTACK,              {" items.attack",       static_cast<EQ::skills::SkillType>(0)} },
		{STAT_HP_REGEN,            {" items.regen",        static_cast<EQ::skills::SkillType>(0)} },
		{STAT_MANA_REGEN,          {" items.manaregen",    static_cast<EQ::skills::SkillType>(0)} },
		{STAT_HASTE,               {" items.haste",        static_cast<EQ::skills::SkillType>(0)} },
		{STAT_DAMAGE_SHIELD,       {" items.damageshield", static_cast<EQ::skills::SkillType>(0)} },
		{STAT_DS_MITIGATION,       {" items.dsmitigation", static_cast<EQ::skills::SkillType>(0)} },
		{STAT_HEAL_AMOUNT,         {" items.healamt",      static_cast<EQ::skills::SkillType>(0)} },
		{STAT_SPELL_DAMAGE,        {" items.spelldmg",     static_cast<EQ::skills::SkillType>(0)} },
		{STAT_CLAIRVOYANCE,        {" items.clairvoyance", static_cast<EQ::skills::SkillType>(0)} },
		{STAT_HEROIC_AGILITY,      {" items.heroic_agi",   static_cast<EQ::skills::SkillType>(0)} },
		{STAT_HEROIC_CHARISMA,     {" items.heroic_cha",   static_cast<EQ::skills::SkillType>(0)} },
		{STAT_HEROIC_DEXTERITY,    {" items.heroic_dex",   static_cast<EQ::skills::SkillType>(0)} },
		{STAT_HEROIC_INTELLIGENCE, {" items.heroic_int",   static_cast<EQ::skills::SkillType>(0)} },
		{STAT_HEROIC_STAMINA,      {" items.heroic_sta",   static_cast<EQ::skills::SkillType>(0)} },
		{STAT_HEROIC_STRENGTH,     {" items.heroic_str",   static_cast<EQ::skills::SkillType>(0)} },
		{STAT_HEROIC_WISDOM,       {" items.heroic_wis",   static_cast<EQ::skills::SkillType>(0)} },
		{STAT_BASH,                {" items.skillmodvalue", EQ::skills::SkillBash}                },
		{STAT_BACKSTAB,            {" items.backstabdmg",   EQ::skills::SkillBackstab}            },
		{STAT_DRAGON_PUNCH,        {" items.skillmodvalue", EQ::skills::SkillDragonPunch}         },
		{STAT_EAGLE_STRIKE,        {" items.skillmodvalue", EQ::skills::SkillEagleStrike}         },
		{STAT_FLYING_KICK,         {" items.skillmodvalue", EQ::skills::SkillFlyingKick}          },
		{STAT_KICK,                {" items.skillmodvalue", EQ::skills::SkillKick}                },
		{STAT_ROUND_KICK,          {" items.skillmodvalue", EQ::skills::SkillRoundKick}           },
		{STAT_TIGER_CLAW,          {" items.skillmodvalue", EQ::skills::SkillTigerClaw}           },
		{STAT_FRENZY,              {" items.skillmodvalue", EQ::skills::SkillFrenzy}              },
	};

	bool        convert = false;
	std::string search_criteria_trader("TRUE");
	std::string field_criteria_items("FALSE");
	std::string where_criteria_items(" TRUE ");

	if (search.search_scope == NonRoFBazaarSearchScope) {
		search_criteria_trader.append(
			fmt::format(
				" AND trader.char_entity_id = {} AND trader.char_zone_id = {} AND trader.char_zone_instance_id = {}",
				search.trader_entity_id,
				Zones::BAZAAR,
				char_zone_instance_id
			)
		);
	}
	else if (search.search_scope == Local_Scope) {
		search_criteria_trader.append(fmt::format(
			" AND trader.char_zone_id = {} AND trader.char_zone_instance_id = {}",
			char_zone_id,
			char_zone_instance_id)
		);
	}
	else if (search.trader_id > 0) {
		if (RuleB(Bazaar, UseAlternateBazaarSearch)) {
			if (search.trader_id >= TraderRepository::TRADER_CONVERT_ID) {
				convert = true;
				search_criteria_trader.append(fmt::format(
					" AND trader.char_zone_id = {} AND trader.char_zone_instance_id = {}",
					Zones::BAZAAR,
					search.trader_id - TraderRepository::TRADER_CONVERT_ID)
				);
			}
			else {
				search_criteria_trader.append(fmt::format(" AND trader.char_id = {}", search.trader_id));
			}
		}
		else {
			search_criteria_trader.append(fmt::format(" AND trader.char_id = {}", search.trader_id));
		}
	}

	if (search.min_cost != 0) {
		search_criteria_trader.append(fmt::format(" AND trader.item_cost >= {}", search.min_cost * 1000));
	}
	if (search.max_cost != 0) {
		search_criteria_trader.append(fmt::format(" AND trader.item_cost <= {}", (uint64) search.max_cost * 1000));
	}

	if (search.slot != std::numeric_limits<uint32>::max()) {
		if (item_slot_searches_new.contains(search.slot)) {
			where_criteria_items.append(
				fmt::format(" AND items.slots & {0} = {0}", item_slot_searches_new[search.slot]));
		}
	}

	if (search.type != std::numeric_limits<uint32>::max()) {
		for (auto const &[type, condition]: item_search_types_new) {
			if (type == search.type) {
				where_criteria_items.append(condition);
				break;
			}
		}
	}

	if (search.race != std::numeric_limits<uint32>::max()) {
		where_criteria_items.append(
			fmt::format(" AND items.races & {0} = {0}", GetPlayerRaceBit(GetRaceIDFromPlayerRaceValue(search.race))));
	}

	if (search._class != std::numeric_limits<uint32>::max()) {
		where_criteria_items.append(fmt::format(" AND items.classes & {0} = {0}", GetPlayerClassBit(search._class)));
	}

	if (search.item_stat != std::numeric_limits<uint32>::max()) {
		if (item_stat_searches_new.contains(search.item_stat)) {
			field_criteria_items = fmt::format("{}", item_stat_searches_new[search.item_stat].query_string);
			if (item_stat_searches_new[search.item_stat].skill_type) {
				where_criteria_items.append(
					fmt::format(" AND items.skillmodtype = {} ", item_stat_searches_new[search.item_stat].skill_type));
			}
			else {
				where_criteria_items.append(
					fmt::format(" AND {} > 0 ", item_stat_searches_new[search.item_stat].query_string));
			}
		}
	}

	if (search.augment) {
		where_criteria_items.append(fmt::format(
			" AND (items.augslot1type = {0} OR "
			"items.augslot2type = {0} OR "
			"items.augslot3type = {0} OR "
			"items.augslot4type = {0} OR "
			"items.augslot5type = {0} OR "
			"items.augslot6type = {0})",
			search.augment)
		);
	}

	if (search.min_level != 1) {
		where_criteria_items.append(fmt::format(" AND items.reclevel >= {}", search.min_level));
	}

	if (search.max_level != 100) {
		where_criteria_items.append(fmt::format(" AND items.reclevel <= {}", search.max_level));
	}

	std::vector<BazaarSearchResultsFromDB_Struct> all_entries;
	std::vector<std::string>                      trader_items_ids{};

	auto const trader_results = TraderRepository::GetBazaarTraderDetails(db, search_criteria_trader);
	if (trader_results.empty()) {
		LogTradingDetail("Bazaar - No traders found in bazaar search.");
		return all_entries;
	}

	for (auto const &i: trader_results) {
		trader_items_ids.push_back(std::to_string(i.trader.item_id));
	}

	auto const item_results = ItemsRepository::GetItemsForBazaarSearch(
		content_db,
		trader_items_ids,
		std::string(search.item_name),
		field_criteria_items,
		where_criteria_items,
		search.max_results
	);

	if (item_results.empty()) {
		LogTradingDetail("Bazaar - No items found in bazaar search.");
		return all_entries;
	}

	all_entries.reserve(trader_results.size());

	for (auto const& t:trader_results) {
		if (!item_results.contains(t.trader.item_id)) {
			continue;
		}

		BazaarSearchResultsFromDB_Struct r{};
		r.count                   = 1;
		r.trader_id               = t.trader.char_id;
		r.serial_number           = t.trader.item_sn;
		r.cost                    = t.trader.item_cost;
		r.slot_id                 = t.trader.slot_id;
		r.charges                 = t.trader.item_charges;
		r.stackable               = item_results.at(t.trader.item_id).stackable;
		r.icon_id                 = item_results.at(t.trader.item_id).icon;
		r.trader_zone_id          = t.trader.char_zone_id;
		r.trader_zone_instance_id = t.trader.char_zone_instance_id;
		r.trader_entity_id        = t.trader.char_entity_id;
		r.serial_number_RoF       = fmt::format("{:016}\0", t.trader.item_sn);
		r.item_name               = fmt::format("{:.63}\0", item_results.at(t.trader.item_id).name);
		r.trader_name             = fmt::format("{:.63}\0", t.trader_name);
		r.item_stat               = item_results.at(t.trader.item_id).stats;

		if (RuleB(Bazaar, UseAlternateBazaarSearch)) {
			if (convert ||
				char_zone_id != Zones::BAZAAR ||
				(char_zone_id == Zones::BAZAAR && r.trader_zone_instance_id != char_zone_instance_id)
				) {
				r.trader_id = TraderRepository::TRADER_CONVERT_ID + r.trader_zone_instance_id;
				}
		}

		all_entries.push_back(r);
	}

	if (all_entries.size() > search.max_results) {
		all_entries.resize(search.max_results);
	}

	LogTrading("Returning [{}] items from search results", all_entries.size());

	return all_entries;
}
