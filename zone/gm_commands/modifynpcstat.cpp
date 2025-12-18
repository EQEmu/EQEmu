#include "zone/client.h"

std::map<std::string, std::string> GetModifyNPCStatMap()
{
	std::map<std::string, std::string> identifiers_map = {
		{ "ac", "Armor Class" },
		{ "accuracy", "Accuracy" },
		{ "aggro", "Aggro" },
		{ "agi", "Agility" },
		{ "assist", "Assist" },
		{ "atk", "Attack" },
		{ "attack_count", "Attack Count" },
		{ "attack_delay", "Attack Delay" },
		{ "attack_speed", "Attack Speed" },
		{ "avoidance", "Avoidance" },
		{ "cha", "Charisma" },
		{ "cr", "Cold Resist" },
		{ "dex", "Dexterity" },
		{ "dr", "Disease Resist" },
		{ "fr", "Fire Resist" },
		{ "healscale", "Heal Scale" },
		{ "heroic_strikethrough", "Heroic Strikethrough" },
		{ "hp_regen", "HP Regen" },
		{ "hp_regen_per_second", "HP Regen Per Second" },
		{ "int", "Intelligence" },
		{ "_int", "Intelligence" },
		{ "keeps_sold_items", "Keeps Sold Items" },
		{ "level", "Level" },
		{ "loottable_id", "Loottable ID" },
		{ "mana_regen", "Mana Regen" },
		{ "max_hit", "Maximum Damage" },
		{ "max_hp", "Maximum Hit Points" },
		{ "min_hit", "Minimum Damage" },
		{ "mr", "Magic Resist" },
		{ "npc_spells_id", "NPC Spells ID" },
		{ "npc_spells_effects_id", "NPC Spells Effects ID" },
		{ "phr", "Physical Resist" },
		{ "pr", "Poison Resist" },
		{ "runspeed", "Run Speed" },
		{ "see_invis", "See Invisible" },
		{ "see_invis_undead", "See Invisible vs. Undead" },
		{ "see_hide", "See Hide" },
		{ "see_improved_hide", "See Improved Hide" },
		{ "slow_mitigation", "Slow Mitigation" },
		{ "special_attacks", "Special Attacks" },
		{ "special_abilities", "Special Abilities" },
		{ "spellscale", "Spell Scale" },
		{ "sta", "Stamina" },
		{ "str", "Strength" },
		{ "trackable", "Trackable" },
		{ "wis", "Wisdom" }
	};

	return identifiers_map;
}

std::string GetModifyNPCStatDescription(std::string stat)
{
	if (GetModifyNPCStatMap().find(stat) != GetModifyNPCStatMap().end()) {
		return GetModifyNPCStatMap().find(stat)->second;
	}

	return std::string();
}

void ListModifyNPCStatMap(Client *c)
{
	for (const auto& s : GetModifyNPCStatMap()) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: #modifynpcstat {} [Value] - Modifies an NPC's {}",
				s.first,
				s.second
			).c_str()
		);
	}
}

void command_modifynpcstat(Client *c, const Seperator *sep)
{
	auto arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #modifynpcstat [Stat] [Value]");
		ListModifyNPCStatMap(c);
		return;
	}

	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	auto target = c->GetTarget()->CastToNPC();

	const std::string& stat  = sep->arg[1] ? sep->arg[1] : "";
	const std::string& value = sep->arg[2] ? sep->arg[2] : "";

	if (stat.empty() || value.empty()) {
		c->Message(Chat::White, "Usage: #modifynpcstat [Stat] [Value]");
		ListModifyNPCStatMap(c);
		return;
	}

	auto stat_description = GetModifyNPCStatDescription(stat);
	if (!stat_description.length()) {
		c->Message(
			Chat::White,
			fmt::format(
				"Stat '{}' does not exist.",
				stat
			).c_str()
		);
		return;
	}

	target->ModifyNPCStat(stat, value);

	c->Message(
		Chat::White,
		fmt::format(
			"Stat Modified | Target: {}",
			c->GetTargetDescription(target)
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Stat Modified | Stat: {} ({}) Value: {}",
			GetModifyNPCStatDescription(stat),
			stat,
			value
		).c_str()
	);
}
