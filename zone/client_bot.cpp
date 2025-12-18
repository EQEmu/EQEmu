#include "client.h"

#include "zone/bot.h"

#define NO_BOT_LIMIT -1;

bool Client::GetBotOption(BotOwnerOption boo) const {
	if (boo < _booCount) {
		return bot_owner_options[boo];
	}

	return false;
}

void Client::SetBotOption(BotOwnerOption boo, bool flag) {
	if (boo < _booCount) {
		bot_owner_options[boo] = flag;
	}
}

uint32 Client::GetBotCreationLimit(uint8 class_id) {
	uint32 bot_creation_limit = RuleI(Bots, CreationLimit);

	if (GetGM() && Admin() >= RuleI(Bots, MinStatusToBypassCreateLimit)) {
		return RuleI(Bots, MinStatusBypassCreateLimit);
	}

	const auto bucket_name = fmt::format(
		"bot_creation_limit{}",
		class_id && IsPlayerClass(class_id) ?
			fmt::format("_{}", Strings::ToLower(GetClassIDName(class_id))) :
			""
	);

	auto bucket_value = GetBucket(bucket_name);

	if (!bucket_value.empty() && Strings::IsNumber(bucket_value)) {
		bot_creation_limit = Strings::ToInt(bucket_value);
	}

	if (class_id && bucket_value.empty()) {
		bot_creation_limit = NO_BOT_LIMIT;
	}

	return bot_creation_limit;
}

int Client::GetBotRequiredLevel(uint8 class_id) {
	int bot_character_level = RuleI(Bots, BotCharacterLevel);

	if (GetGM() && Admin() >= RuleI(Bots, MinStatusToBypassBotLevelRequirement)) {
		return 0;
	}

	const auto bucket_name = fmt::format(
		"bot_required_level{}",
		class_id && IsPlayerClass(class_id) ?
			fmt::format("_{}", Strings::ToLower(GetClassIDName(class_id))) :
			""
	);

	auto bucket_value = GetBucket(bucket_name);

	if (!bucket_value.empty() && Strings::IsNumber(bucket_value)) {
		bot_character_level = Strings::ToInt(bucket_value);
	}

	if (class_id && bucket_value.empty()) {
		bot_character_level = NO_BOT_LIMIT;
	}

	return bot_character_level;
}

int Client::GetBotSpawnLimit(uint8 class_id)
{
	int bot_spawn_limit = RuleI(Bots, SpawnLimit);

	if (GetGM() && Admin() >= RuleI(Bots, MinStatusToBypassSpawnLimit)) {
		return RuleI(Bots, MinStatusBypassSpawnLimit);
	}

	const auto bucket_name = fmt::format(
		"bot_spawn_limit{}",
		class_id && IsPlayerClass(class_id) ?
			fmt::format("_{}", Strings::ToLower(GetClassIDName(class_id))) :
			""
	);

	auto bucket_value = GetBucket(bucket_name);

	if (!bucket_value.empty() && Strings::IsNumber(bucket_value)) {
		bot_spawn_limit = Strings::ToInt(bucket_value);
	}

	if (class_id && bucket_value.empty()) {
		return NO_BOT_LIMIT;
	}

	if (RuleB(Bots, QuestableSpawnLimit)) {
		const auto query = fmt::format(
			"SELECT `value` FROM `quest_globals` WHERE `name` = '{}' AND `charid` = {} LIMIT 1",
			bucket_name,
			CharacterID()
		);

		auto results = database.QueryDatabase(query); // use 'database' for non-bot table calls

		if (results.Success() && results.RowCount()) {
			auto row = results.begin();
			bot_spawn_limit = Strings::ToInt(row[0]);
		}
	}

	if (!class_id) {
		const auto &zones_list = Strings::Split(RuleS(Bots, ZonesWithSpawnLimits), ",");

		if (!zones_list.empty()) {
			auto it = std::find(zones_list.begin(), zones_list.end(), std::to_string(zone->GetZoneID()));

			if (it != zones_list.end()) {
				const auto &zones_limits_list = Strings::Split(RuleS(Bots, ZoneSpawnLimits), ",");

				if (zones_list.size() == zones_limits_list.size()) {
					try {
						auto new_limit = std::stoul(zones_limits_list[std::distance(zones_list.begin(), it)]);

						if (new_limit < bot_spawn_limit) {
							bot_spawn_limit = new_limit;
						}
					} catch (const std::exception &e) {
						LogInfo("Invalid entry in Rule Bots:ZoneSpawnLimits: [{}]", e.what());
					}
				}
			}
		}

		const auto &zones_forced_list = Strings::Split(RuleS(Bots, ZonesWithForcedSpawnLimits), ",");

		if (!zones_forced_list.empty()) {
			auto it = std::find(zones_forced_list.begin(), zones_forced_list.end(), std::to_string(zone->GetZoneID()));

			if (it != zones_forced_list.end()) {
				const auto &zones_forced_limits_list = Strings::Split(RuleS(Bots, ZoneForcedSpawnLimits), ",");

				if (zones_forced_list.size() == zones_forced_limits_list.size()) {
					try {
						auto new_limit = std::stoul(zones_forced_limits_list[std::distance(zones_forced_list.begin(), it)]);

						if (new_limit != bot_spawn_limit) {
							bot_spawn_limit = new_limit;
						}
					} catch (const std::exception &e) {
						LogInfo("Invalid entry in Rule Bots:ZoneForcedSpawnLimits: [{}]", e.what());
					}
				}
			}
		}
	}

	return bot_spawn_limit;
}

void Client::SetBotCreationLimit(uint32 new_creation_limit, uint8 class_id) {
	const auto bucket_name = fmt::format(
		"bot_creation_limit{}",
		(
			class_id && IsPlayerClass(class_id) ?
			fmt::format(
				"_{}",
				Strings::ToLower(GetClassIDName(class_id))
			) :
			""
		)
	);

	SetBucket(bucket_name, std::to_string(new_creation_limit));
}

void Client::SetBotRequiredLevel(int new_required_level, uint8 class_id) {
	const auto bucket_name = fmt::format(
		"bot_required_level{}",
		(
			class_id && IsPlayerClass(class_id) ?
			fmt::format(
				"_{}",
				Strings::ToLower(GetClassIDName(class_id))
			) :
			""
		)
	);

	SetBucket(bucket_name, std::to_string(new_required_level));
}

void Client::SetBotSpawnLimit(int new_spawn_limit, uint8 class_id) {
	const auto bucket_name = fmt::format(
		"bot_spawn_limit{}",
		(
			class_id && IsPlayerClass(class_id) ?
			fmt::format(
				"_{}",
				Strings::ToLower(GetClassIDName(class_id))
			) :
			""
		)
	);

	SetBucket(bucket_name, std::to_string(new_spawn_limit));
}

void Client::CampAllBots(uint8 class_id) {
	Bot::BotOrderCampAll(this, class_id);
}

void Client::LoadDefaultBotSettings() {
	m_bot_spell_settings.clear();

	/* No longer used, left as example.
	SetBotSetting(BotSettingCategories::BaseSetting, BotBaseSettings::IllusionBlock, GetDefaultBotSettings(BotSettingCategories::BaseSetting, BotBaseSettings::IllusionBlock));
	LogBotSettingsDetail("{} says, 'Setting default {} [{}] to [{}]'", GetCleanName(), CastToBot()->GetBotSettingCategoryName(BotBaseSettings::IllusionBlock), BotBaseSettings::IllusionBlock, GetDefaultBotSettings(BotSettingCategories::BaseSetting, BotBaseSettings::IllusionBlock));
	*/

	for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
		BotSpellSettings t;

		t.spell_type    = i;
		t.short_name    = Bot::GetSpellTypeShortNameByID(i);
		t.name          = Bot::GetSpellTypeNameByID(i);
		t.delay         = GetDefaultSpellTypeDelay(i);
		t.min_threshold = GetDefaultSpellTypeMinThreshold(i);
		t.max_threshold = GetDefaultSpellTypeMaxThreshold(i);
		t.recast_timer.Start();

		m_bot_spell_settings.push_back(t);

		LogBotSettingsDetail("{} says, 'Setting defaults for {} ({}) [#{}]'", GetCleanName(), t.name, t.short_name, t.spell_type);
		LogBotSettingsDetail("{} says, 'Delay = [{}ms] | MinThreshold = [{}\%] | MaxThreshold = [{}\%]'", GetCleanName(),
							 GetDefaultSpellTypeDelay(i),
							 GetDefaultSpellTypeMinThreshold(i), GetDefaultSpellTypeMaxThreshold(i));
	}
}

int Client::GetDefaultBotSettings(uint8 setting_type, uint16 bot_setting) {
	switch (setting_type) {
		case BotSettingCategories::BaseSetting:
			return false; // only setting supported currently is illusion block
		case BotSettingCategories::SpellDelay:
			return GetDefaultSpellTypeDelay(bot_setting);
		case BotSettingCategories::SpellMinThreshold:
			return GetDefaultSpellTypeMinThreshold(bot_setting);
		case BotSettingCategories::SpellMaxThreshold:
			return GetDefaultSpellTypeMaxThreshold(bot_setting);
		default:
			return 0; // default return for any unsupported setting type
	}
}

int Client::GetBotSetting(uint8 setting_type, uint16 bot_setting) {
	switch (setting_type) {
		case BotSettingCategories::BaseSetting:
			return 0; // unused currently
		case BotSettingCategories::SpellDelay:
			return GetSpellTypeDelay(bot_setting);
		case BotSettingCategories::SpellMinThreshold:
			return GetSpellTypeMinThreshold(bot_setting);
		case BotSettingCategories::SpellMaxThreshold:
			return GetSpellTypeMaxThreshold(bot_setting);
		default:
			return 0; // default return for any unsupported setting type
	}
}

void Client::SetBotSetting(uint8 setting_type, uint16 bot_setting, uint32 setting_value) {
	switch (setting_type) {
		case BotSettingCategories::BaseSetting:
			break; // unused currently
		case BotSettingCategories::SpellDelay:
			SetSpellTypeDelay(bot_setting, setting_value);
			break;
		case BotSettingCategories::SpellMinThreshold:
			SetSpellTypeMinThreshold(bot_setting, setting_value);
			break;
		case BotSettingCategories::SpellMaxThreshold:
			SetSpellTypeMaxThreshold(bot_setting, setting_value);
			break;
	}
}

void Client::SendSpellTypePrompts(bool commanded_types, bool client_only_types) {
	if (client_only_types) {
		Message(
			Chat::Yellow,
			fmt::format(
				"You can view client spell types by {} or {}.",
				Saylink::Silent(
					fmt::format("^spelltypeids client"), "ID"
				),
				Saylink::Silent(
					fmt::format("^spelltypenames client"), "Shortname"
				)
			).c_str()
		);
	}
	else {
		Message(
			Chat::Yellow,
			fmt::format(
				"You can view spell types by {}, {}, {} or by {}, {}, {}.",
				Saylink::Silent(
					fmt::format("^spelltypeids 0-19"), "ID 0-19"
				),
				Saylink::Silent(
					fmt::format("^spelltypeids 20-39"), "20-39"
				),
				Saylink::Silent(
					fmt::format("^spelltypeids 40+"), "40+"
				),
				Saylink::Silent(
					fmt::format("^spelltypenames 0-19"), "Shortname 0-19"
				),
				Saylink::Silent(
					fmt::format("^spelltypenames 20-39"), "20-39"
				),
				Saylink::Silent(
					fmt::format("^spelltypenames 40+"), "40+"
				)
			).c_str()
		);
	}

	if (commanded_types) {
		Message(
			Chat::Yellow,
			fmt::format(
				"You can view commanded spell types by {} or {}.",
				Saylink::Silent(
					fmt::format("^spelltypeids commanded"), "ID"
				),
				Saylink::Silent(
					fmt::format("^spelltypenames commanded"), "Shortname"
				)
			).c_str()
		);
	}

	return;
}

uint16 Client::GetDefaultSpellTypeDelay(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::VeryFastHeals:
		case BotSpellTypes::PetVeryFastHeals:
			return 1500;
		case BotSpellTypes::FastHeals:
		case BotSpellTypes::PetFastHeals:
			return 2500;
		case BotSpellTypes::GroupHeals:
		case BotSpellTypes::RegularHeal:
		case BotSpellTypes::PetRegularHeals:
			return 4000;
		case BotSpellTypes::CompleteHeal:
		case BotSpellTypes::GroupCompleteHeals:
		case BotSpellTypes::PetCompleteHeals:
			return 8000;
		case BotSpellTypes::GroupHoTHeals:
		case BotSpellTypes::HoTHeals:
		case BotSpellTypes::PetHoTHeals:
			return 22000;
		case BotSpellTypes::Cure:
			return 2000;
		case BotSpellTypes::GroupCures:
			return 3000;
		case BotSpellTypes::PetCures:
			return 5000;
		default:
			return 100;
	}
}

uint8 Client::GetDefaultSpellTypeMinThreshold(uint16 spell_type) {
	switch (spell_type) {
		default:
			return 0;
	}
}

uint8 Client::GetDefaultSpellTypeMaxThreshold(uint16 spell_type) {
	uint8 client_class = GetClass();

	switch (spell_type) {
		case BotSpellTypes::VeryFastHeals:
		case BotSpellTypes::PetVeryFastHeals:
			return 25;
		case BotSpellTypes::FastHeals:
		case BotSpellTypes::PetFastHeals:
			return 40;
		case BotSpellTypes::GroupHeals:
		case BotSpellTypes::RegularHeal:
		case BotSpellTypes::PetRegularHeals:
			return 60;
		case BotSpellTypes::CompleteHeal:
		case BotSpellTypes::GroupCompleteHeals:
		case BotSpellTypes::PetCompleteHeals:
			if (client_class == Class::Necromancer || client_class == Class::Shaman) {
				return 55;
			}
			else {
				return 80;
			}
		case BotSpellTypes::GroupHoTHeals:
		case BotSpellTypes::HoTHeals:
		case BotSpellTypes::PetHoTHeals:
			if (client_class == Class::Necromancer || client_class == Class::Shaman) {
				return 70;
			}
			else {
				return 90;
			}
		case BotSpellTypes::Buff:
		case BotSpellTypes::Cure:
		case BotSpellTypes::GroupCures:
		case BotSpellTypes::PetCures:
		case BotSpellTypes::PetBuffs:
		case BotSpellTypes::PetDamageShields:
		case BotSpellTypes::PetResistBuffs:
		case BotSpellTypes::ResistBuffs:
		default:
			return 100;
	}
}
