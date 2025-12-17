#include "player_event_discord_formatter.h"

#include "common/json/json_archive_single_line.h"
#include "common/repositories/character_data_repository.h"

#include "cereal/archives/json.hpp"
#include "cereal/types/vector.hpp"
#include "fmt/format.h"
#include "fmt/ranges.h"

#include <vector>

std::string PlayerEventDiscordFormatter::GetCurrentTimestamp()
{
	time_t now;
	time(&now);
	char buf[sizeof "2011-10-08T07:07:09Z"];
	strftime(buf, sizeof buf, "%FT%TZ", gmtime(&now));
	std::string timestamp = buf;
	return timestamp;
}

void PlayerEventDiscordFormatter::BuildDiscordField(
	std::vector<DiscordField> *f,
	const std::string &name,
	const std::string &value,
	bool is_inline
)
{
	if (value.empty()) {
		return;
	}

	f->emplace_back(
		DiscordField{
			.name = name,
			.value = value,
			.is_inline = is_inline,
		}
	);
}

void PlayerEventDiscordFormatter::BuildBaseEmbed(
	std::vector<DiscordEmbed> *e,
	const std::vector<DiscordField> &f,
	const PlayerEvent::PlayerEventContainer c
)
{
	auto d = DiscordEmbed{};
	d.fields = f;
	d.author = DiscordAuthor{
		.name = fmt::format(
			"[Player Event] {}",
			PlayerEvent::EventName[c.player_event_log.event_type_id]
		),
	};
	// d.timestamp = GetCurrentTimestamp()

	e->emplace_back(d);
}

std::string PlayerEventDiscordFormatter::FormatEventSay(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::SayEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildDiscordField(&f, "Message", e.message);
	BuildDiscordField(&f, "Target", e.target);

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);

	auto root = BuildDiscordWebhook(c, embeds);

	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatGMCommand(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::GMCommandEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildDiscordField(&f, "Message", e.message);
	if (e.target != "NONE") {
		BuildDiscordField(&f, "Target", e.target);
	}

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);

	auto root = BuildDiscordWebhook(c, embeds);

	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatWithNodata(const PlayerEvent::PlayerEventContainer &c)
{
	std::vector<DiscordField> f = {};
	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatMerchantPurchaseEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::MerchantPurchaseEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildDiscordField(&f, "Merchant", fmt::format("{} ({}) NPC ID ({})", e.merchant_name, e.merchant_type, e.npc_id));
	BuildDiscordField(&f, "Item", fmt::format("{} ({}) x({})", e.item_name, e.item_id, e.charges));
	BuildDiscordField(
		&f,
		"Cost",
		fmt::format(
			":moneybag: {}",
			Strings::Money((e.cost / 1000), (e.cost / 100) % 10, (e.cost / 10) % 10, e.cost % 10)));
	BuildDiscordField(
		&f,
		"Player Balance",
		fmt::format(
			":moneybag: [{}] \n:gem: Currency [{}]",
			Strings::Commify(std::to_string(e.player_money_balance)),
			e.player_currency_balance
		)
	);
	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatMerchantSellEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::MerchantSellEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildDiscordField(&f, "Merchant", fmt::format("{} ({}) NPC ID ({})", e.merchant_name, e.merchant_type, e.npc_id));
	BuildDiscordField(&f, "Item", fmt::format("{} ({}) x({})", e.item_name, e.item_id, e.charges));
	BuildDiscordField(
		&f,
		"Cost",
		fmt::format(
			":moneybag: {}",
			Strings::Money((e.cost / 1000), (e.cost / 100) % 10, (e.cost / 10) % 10, e.cost % 10)));
	BuildDiscordField(
		&f,
		"Player Balance",
		fmt::format(
			":moneybag: [{}] \n:gem: Currency [{}]",
			Strings::Commify(std::to_string(e.player_money_balance)),
			e.player_currency_balance
		)
	);
	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatZoningEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::ZoningEvent &e
)
{
	std::string instance_id_info;
	std::string instance_version_info;

	if (e.to_instance_id > 0 || e.from_instance_id > 0) {
		instance_id_info = fmt::format("Instance ID: ({}) :arrow_right: ({})", e.from_instance_id, e.to_instance_id);
	}

	if (e.from_instance_version > 0 || e.to_instance_version > 0) {
		instance_version_info = fmt::format(
			"Instance Version: ({}) :arrow_right: ({})",
			e.from_instance_version,
			e.to_instance_version
		);
	}

	std::vector<DiscordField> f = {};
	BuildDiscordField(
		&f,
		"Zoning Information",
		fmt::format(
			"Zone: {} ({}) ({}) :arrow_right: {} ({}) ({})\n{}\n{}",
			e.from_zone_long_name,
			e.from_zone_short_name,
			e.from_zone_id,
			e.to_zone_long_name,
			e.to_zone_short_name,
			e.to_zone_id,
			instance_id_info,
			instance_version_info
		)
	);

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);

	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatAAGainedEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::AAGainedEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildDiscordField(
		&f,
		"Points Gained",
		fmt::format(
			"AA Gained ({})",
			e.aa_gained
		)
	);

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatAAPurchasedEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::AAPurchasedEvent &e
)
{
	std::string aa_info;
	if (e.aa_previous_id != -1 || e.aa_next_id != -1) {
		aa_info = fmt::format("AA Previous ID ({}) \nAA Next ID ({})", e.aa_previous_id, e.aa_next_id);
	}

	std::vector<DiscordField> f = {};
	BuildDiscordField(
		&f,
		"AA Purchased",
		fmt::format(
			"AA ID ({}) \nAA Cost ({}) \n{}",
			e.aa_id, e.aa_cost, aa_info
		)
	);

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatForageSuccessEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::ForageSuccessEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildDiscordField(
		&f,
		"Foraged Item",
		fmt::format(
			"Item ID ({}) \nItem Name ({})",
			e.item_id, e.item_name
		)
	);

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatDestroyItemEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::DestroyItemEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildDiscordField(
		&f,
		"Destroyed Item",
		fmt::format(
			"{} ({}) \nCharges ({}) \nReason ({})",
			e.item_name, e.item_id, e.charges, e.reason
		)
	);

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatLevelGainedEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::LevelGainedEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildDiscordField(
		&f,
		"Level Information",
		fmt::format(
			"From ({}) > ({}) \nLevels Gained ({})",
			e.from_level, e.to_level, e.levels_gained
		)
	);

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatLevelLostEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::LevelLostEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildDiscordField(
		&f,
		"Level Information",
		fmt::format(
			"From ({}) > ({}) \nLevels Lost ({})",
			e.from_level, e.to_level, e.levels_lost
		)
	);

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatLootItemEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::LootItemEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildDiscordField(
		&f,
		"Looted Item",
		fmt::format(
			"{} ({})\nCharges: {}\nNPC: {} ({})",
			e.item_name,
			e.item_id,
			e.charges,
			e.corpse_name,
			e.npc_id
		)
	);

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}


std::string PlayerEventDiscordFormatter::FormatGroundSpawnPickupEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::GroundSpawnPickupEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildDiscordField(
		&f,
		"Picked Up Item",
		fmt::format(
			"{} ({})",
			e.item_name, e.item_id
		)
	);

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatSkillUpEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::SkillUpEvent &e
)
{
	std::string target_info;
	if (e.against_who.length()) {
		if (e.against_who == c.player_event.character_name) {
			target_info = fmt::format("Target: Self");
		}
		else {
			target_info = fmt::format("Target: {}", e.against_who);
		}
	}

	std::vector<DiscordField> f = {};
	BuildDiscordField(
		&f,
		"Skill Information",
		fmt::format(
			"Skill: {} \nLevel: ({}/{}) \n{}",
			e.skill_id, e.value, e.max_skill, target_info
		)
	);

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatTaskAcceptEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::TaskAcceptEvent &e
)
{

	std::vector<DiscordField> f = {};
	BuildDiscordField(
		&f,
		"Task Information",
		fmt::format(
			"{} ({}) \n {} ({})",
			e.task_name, e.task_id, e.npc_name, e.npc_id
		)
	);
	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatTaskCompleteEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::TaskCompleteEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildDiscordField(
		&f,
		"Task Information",
		fmt::format(
			"{} ({}) \nActivity ID ({}) \nDone ({})",
			e.task_name, e.task_id, e.activity_id, e.done_count
		)
	);
	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatTaskUpdateEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::TaskUpdateEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildDiscordField(
		&f,
		"Task Information",
		fmt::format(
			"{} ({}) \nActivity ID ({}) \nDone ({})",
			e.task_name, e.task_id, e.activity_id, e.done_count
		)
	);
	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatResurrectAcceptEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::ResurrectAcceptEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildDiscordField(
		&f,
		"Resurrect Information",
		fmt::format(
			"From: {} \nSpell: {} ({})",
			e.resurrecter_name, e.spell_name, e.spell_id
		)
	);
	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatCombineEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::CombineEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildDiscordField(
		&f,
		"Combine Information",
		fmt::format(
			"{} ({}) \n Made ({})",
			e.recipe_name, e.recipe_id, e.made_count
		)
	);
	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatFishSuccessEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::FishSuccessEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildDiscordField(
		&f,
		"Fishing Information",
		fmt::format(
			"{} ({})",
			e.item_name, e.item_id
		)
	);
	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatDeathEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::DeathEvent &e
)
{
	std::string killer_info;
	if (e.killer_id) {
		killer_info = fmt::format("Killer: {} ({})", e.killer_name, e.killer_id);
	}

	std::string spell_info;
	if (e.spell_id < MAX_SPELL_DB_ID_VAL) {
		spell_info = fmt::format("Spell: {} ({})", e.spell_name, e.spell_id);
	}

	std::string skill_info;
	if (e.skill_id) {
		skill_info = fmt::format("Skill: {} ({})", e.skill_name, e.skill_id);
	}

	std::vector<DiscordField> f = {};
	BuildDiscordField(
		&f,
		"Death Information",
		fmt::format(
			"{} \nDamage: {} \n {} \n {}",
			killer_info, e.damage, spell_info, skill_info
		)
	);
	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatNPCHandinEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::HandinEvent &e
)
{
	std::string handin_items_info;
	if (!e.handin_items.empty()) {
		for (const auto &h: e.handin_items) {
			handin_items_info += fmt::format(
				"{} ({}){}{}\n",
				h.item_name,
				h.item_id,
				h.charges > 1 ? fmt::format(" Charges: {}", h.charges) : "",
				h.attuned ? " (Attuned)" : ""
			);

			for (int i = 0; i < h.augment_ids.size(); i++) {
				if (!h.augment_names[i].empty()) {
					const uint8 slot_id = (i + 1);
					handin_items_info += fmt::format(
						"Augment {}: {} ({})\n",
						slot_id,
						h.augment_names[i],
						h.augment_ids[i]
					);
				}
			}
		}
	}

	std::string return_items_info;
	if (!e.return_items.empty()) {
		for (const auto &r: e.return_items) {
			return_items_info += fmt::format(
				"{} ({}){}{}\n",
				r.item_name,
				r.item_id,
				r.charges > 1 ? fmt::format(" Charges: {}", r.charges) : "",
				r.attuned ? " (Attuned)" : ""
			);

			for (int i = 0; i < r.augment_ids.size(); i++) {
				if (!r.augment_names[i].empty()) {
					const uint8 slot_id = (i + 1);
					return_items_info += fmt::format(
						"Augment {}: {} ({})\n",
						slot_id,
						r.augment_names[i],
						r.augment_ids[i]
					);
				}
			}
		}
	}

	std::string handin_money_info;
	if (e.handin_money.platinum) {
		handin_money_info += fmt::format(
			":moneybag: {} Platinum\n",
			Strings::Commify(std::to_string(e.handin_money.platinum))
		);
	}

	if (e.handin_money.gold) {
		handin_money_info += fmt::format(
			":moneybag: {} Gold\n",
			Strings::Commify(std::to_string(e.handin_money.gold))
		);
	}

	if (e.handin_money.silver) {
		handin_money_info += fmt::format(
			":moneybag: {} Silver\n",
			Strings::Commify(std::to_string(e.handin_money.silver))
		);
	}

	if (e.handin_money.copper) {
		handin_money_info += fmt::format(
			":moneybag: {} Copper",
			Strings::Commify(std::to_string(e.handin_money.copper))
		);
	}


	std::string return_money_info;
	if (e.return_money.platinum) {
		return_money_info += fmt::format(
			":moneybag: {} Platinum\n",
			Strings::Commify(std::to_string(e.return_money.platinum))
		);
	}

	if (e.return_money.gold) {
		return_money_info += fmt::format(
			":moneybag: {} Gold\n",
			Strings::Commify(std::to_string(e.return_money.gold))
		);
	}

	if (e.return_money.silver) {
		return_money_info += fmt::format(
			":moneybag: {} Silver\n",
			Strings::Commify(std::to_string(e.return_money.silver))
		);
	}

	if (e.return_money.copper) {
		return_money_info += fmt::format(
			":moneybag: {} Copper",
			Strings::Commify(std::to_string(e.return_money.copper))
		);
	}

	std::string npc_info = fmt::format(
		"{} ({})\n",
		e.npc_name,
		e.npc_id
	);

	npc_info += fmt::format(
		"Is Quest Handin: {}",
		e.is_quest_handin ? "Yes" : "No"
	);

	std::vector<DiscordField> f = {};


	BuildDiscordField(&f, "NPC", npc_info);

	if (!handin_items_info.empty()) {
		BuildDiscordField(&f, "Handin Items", handin_items_info);
	}

	if (!handin_money_info.empty()) {
		BuildDiscordField(&f, "Handin Money", handin_money_info);
	}

	if (!return_items_info.empty()) {
		BuildDiscordField(&f, "Return Items", return_items_info);
	}

	if (!return_money_info.empty()) {
		BuildDiscordField(&f, "Return Money", return_money_info);
	}

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatDiscoverItemEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::DiscoverItemEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildDiscordField(
		&f,
		"Discovered Item",
		fmt::format(
			"{} ({})",
			e.item_name, e.item_id
		)
	);

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatDroppedItemEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::DroppedItemEvent &e
)
{
	std::vector<DiscordField> f = {};
	BuildDiscordField(
		&f,
		"Dropped Item",
		fmt::format(
			"{} ({})\nSlot: {} ({})",
			e.item_name,
			e.item_id,
			EQ::invslot::GetInvPossessionsSlotName(e.slot_id),
			e.slot_id
		)
	);

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatSplitMoneyEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::SplitMoneyEvent &e
)
{
	std::string money_info;
	if (e.platinum) {
		money_info += fmt::format(
			":moneybag: {} Platinum\n",
			Strings::Commify(std::to_string(e.platinum))
		);
	}

	if (e.gold) {
		money_info += fmt::format(
			":moneybag: {} Gold\n",
			Strings::Commify(std::to_string(e.gold))
		);
	}

	if (e.silver) {
		money_info += fmt::format(
			":moneybag: {} Silver\n",
			Strings::Commify(std::to_string(e.silver))
		);
	}

	if (e.copper) {
		money_info += fmt::format(
			":moneybag: {} Copper\n",
			Strings::Commify(std::to_string(e.copper))
		);
	}

	money_info += fmt::format(
		":moneybag: [{}]",
		Strings::Commify(std::to_string(e.player_money_balance))
	);

	std::vector<DiscordField> f = {};
	BuildDiscordField(
		&f,
		"Split Money",
		money_info
	);

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatTraderPurchaseEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::TraderPurchaseEvent &e
)
{
	std::string purchase_info;

	purchase_info += fmt::format(
		"Item: {} ({})\n",
		e.item_name,
		e.item_id
	);

	purchase_info += fmt::format(
		"Trader: {} ({})\n",
		e.trader_name,
		e.trader_id
	);

	purchase_info += fmt::format(
		"Price: {} Amount: {} Total: {}\n",
		Strings::Commify(std::to_string(e.price)),
		e.charges,
		Strings::Commify(std::to_string(e.total_cost))
	);

	purchase_info += fmt::format(
		":moneybag: [{}]\n",
		Strings::Commify(std::to_string(e.player_money_balance))
	);

	std::vector<DiscordField> f = {};
	BuildDiscordField(
		&f,
		"Purchase Information",
		purchase_info
	);

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatTraderSellEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::TraderSellEvent &e
)
{
	std::string sell_info;

	sell_info += fmt::format(
		"Item: {} ({})\n",
		e.item_name,
		e.item_id
	);

	sell_info += fmt::format(
		"Buyer: {} ({})\n",
		e.buyer_name,
		e.buyer_id
	);

	sell_info += fmt::format(
		"Price: {} Amount: {} Total: {}\n",
		Strings::Commify(std::to_string(e.price)),
		e.charges,
		Strings::Commify(std::to_string(e.total_cost))
	);

	sell_info += fmt::format(
		":moneybag: [{}]\n",
		Strings::Commify(std::to_string(e.player_money_balance))
	);

	std::vector<DiscordField> f = {};
	BuildDiscordField(
		&f,
		"Sale Information",
		sell_info
	);

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

std::string PlayerEventDiscordFormatter::FormatTradeEvent(
	const PlayerEvent::PlayerEventContainer &c,
	const PlayerEvent::TradeEvent &e
)
{
	std::string character_1_item_info;
	if (!e.character_1_give_items.empty()) {
		for (const auto &i: e.character_1_give_items) {
			std::string augment_info;
			if (i.augment_1_id > 0) {
				augment_info += fmt::format(
					"Augment 1: {} ({})",
					i.augment_1_name,
					i.augment_1_id
				);
			}

			if (i.augment_2_id > 0) {
				augment_info += fmt::format(
					"Augment 2: {} ({})",
					i.augment_2_name,
					i.augment_2_id
				);
			}

			if (i.augment_3_id > 0) {
				augment_info += fmt::format(
					"Augment 3: {} ({})",
					i.augment_3_name,
					i.augment_3_id
				);
			}

			if (i.augment_4_id > 0) {
				augment_info += fmt::format(
					"Augment 4: {} ({})\n",
					i.augment_4_name,
					i.augment_4_id
				);
			}

			if (i.augment_5_id > 0) {
				augment_info += fmt::format(
					"Augment 5: {} ({})\n",
					i.augment_5_name,
					i.augment_5_id
				);
			}

			if (i.augment_6_id > 0) {
				augment_info += fmt::format(
					"Augment 6: {} ({})",
					i.augment_6_name,
					i.augment_6_id
				);
			}

			character_1_item_info += fmt::format(
				"{} ({}){}\nSlot: {} ({}){}\n{}",
				i.item_name,
				i.item_id,
				i.charges > 1 ? fmt::format(" Charges: {}", i.charges) : "",
				EQ::invslot::GetInvPossessionsSlotName(i.slot),
				i.slot,
				i.in_bag ? " (Bagged Item)" : "",
				augment_info
			);
		}
	}

	std::string character_2_item_info;
	if (!e.character_2_give_items.empty()) {
		for (const auto &i: e.character_2_give_items) {
			std::string augment_info;
			if (i.augment_1_id > 0) {
				augment_info += fmt::format(
					"Augment 1: {} ({})",
					i.augment_1_name,
					i.augment_1_id
				);
			}

			if (i.augment_2_id > 0) {
				augment_info += fmt::format(
					"Augment 2: {} ({})",
					i.augment_2_name,
					i.augment_2_id
				);
			}

			if (i.augment_3_id > 0) {
				augment_info += fmt::format(
					"Augment 3: {} ({})",
					i.augment_3_name,
					i.augment_3_id
				);
			}

			if (i.augment_4_id > 0) {
				augment_info += fmt::format(
					"Augment 4: {} ({})\n",
					i.augment_4_name,
					i.augment_4_id
				);
			}

			if (i.augment_5_id > 0) {
				augment_info += fmt::format(
					"Augment 5: {} ({})\n",
					i.augment_5_name,
					i.augment_5_id
				);
			}

			if (i.augment_6_id > 0) {
				augment_info += fmt::format(
					"Augment 6: {} ({})",
					i.augment_6_name,
					i.augment_6_id
				);
			}

			character_2_item_info += fmt::format(
				"{} ({}){}\nSlot: {} ({}){}\n{}\n",
				i.item_name,
				i.item_id,
				i.charges > 1 ? fmt::format(" Charges: {}", i.charges) : "",
				EQ::invslot::GetInvPossessionsSlotName(i.slot),
				i.slot,
				i.in_bag ? " (Bagged Item)" : "",
				augment_info
			);
		}
	}

	std::string character_1_money_info;
	if (e.character_1_give_money.platinum) {
		character_1_money_info += fmt::format(
			":moneybag: {} Platinum\n",
			Strings::Commify(std::to_string(e.character_1_give_money.platinum))
		);
	}

	if (e.character_1_give_money.gold) {
		character_1_money_info += fmt::format(
			":moneybag: {} Gold\n",
			Strings::Commify(std::to_string(e.character_1_give_money.gold))
		);
	}

	if (e.character_1_give_money.silver) {
		character_1_money_info += fmt::format(
			":moneybag: {} Silver\n",
			Strings::Commify(std::to_string(e.character_1_give_money.silver))
		);
	}

	if (e.character_1_give_money.copper) {
		character_1_money_info += fmt::format(
			":moneybag: {} Copper",
			Strings::Commify(std::to_string(e.character_1_give_money.copper))
		);
	}

	std::string character_2_money_info;
	if (e.character_2_give_money.platinum) {
		character_2_money_info += fmt::format(
			":moneybag: {} Platinum\n",
			Strings::Commify(std::to_string(e.character_2_give_money.platinum))
		);
	}

	if (e.character_2_give_money.gold) {
		character_2_money_info += fmt::format(
			":moneybag: {} Gold\n",
			Strings::Commify(std::to_string(e.character_2_give_money.gold))
		);
	}

	if (e.character_2_give_money.silver) {
		character_2_money_info += fmt::format(
			":moneybag: {} Silver\n",
			Strings::Commify(std::to_string(e.character_2_give_money.silver))
		);
	}

	if (e.character_2_give_money.copper) {
		character_2_money_info += fmt::format(
			":moneybag: {} Copper",
			Strings::Commify(std::to_string(e.character_2_give_money.copper))
		);
	}

	std::vector<DiscordField> f = {};

	if (!character_1_item_info.empty()) {
		BuildDiscordField(
			&f,
			"Character 1 Items",
			character_1_item_info
		);
	}

	if (!character_1_money_info.empty()) {
		BuildDiscordField(
			&f,
			"Character 1 Money",
			character_1_money_info
		);
	}

	if (!character_2_item_info.empty()) {
		BuildDiscordField(
			&f,
			"Character 2 Items",
			character_2_item_info
		);
	}

	if (!character_2_money_info.empty()) {
		BuildDiscordField(
			&f,
			"Character 2 Money",
			character_2_money_info
		);
	}

	std::vector<DiscordEmbed> embeds = {};
	BuildBaseEmbed(&embeds, f, c);
	auto root = BuildDiscordWebhook(c, embeds);
	std::stringstream ss;
	{
		cereal::JSONOutputArchiveSingleLine ar(ss);
		root.serialize(ar);
	}

	return ss.str();
}

DiscordWebhook PlayerEventDiscordFormatter::BuildDiscordWebhook(
	const PlayerEvent::PlayerEventContainer &p,
	std::vector<DiscordEmbed> &embeds
)
{
	DiscordWebhook w = DiscordWebhook{
		.embeds = embeds
	};

	std::string instance_info;
	if (p.player_event.instance_id > 0) {
		instance_info = fmt::format("Instance ID [{}]", p.player_event.instance_id);
	}

	std::string guild_info;
	if (!p.player_event.guild_name.empty()) {
		guild_info = fmt::format(":shield: **Guild** [{}] ({})", p.player_event.guild_name, p.player_event.guild_id);
	}

	std::string character = fmt::format(
		"{} ({}) {}",
		p.player_event.character_name,
		p.player_event.character_id,
		guild_info
	);

	std::string zone = fmt::format(
		"[{}] ({}) ({}) {}",
		p.player_event.zone_long_name,
		p.player_event.zone_short_name,
		p.player_event.zone_id,
		instance_info
	);

	w.content = fmt::format(":trident: **Character** {} :map: **Zone** {}", character, zone);

//	w.avatar_url = "https://cdn.discordapp.com/icons/212663220849213441/a_710698e80c111a5674e1ef716d8e3f14.webp?size=96";

	return w;
}
