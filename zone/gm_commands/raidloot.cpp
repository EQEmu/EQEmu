#include "zone/client.h"
#include "zone/groups.h"
#include "zone/raids.h"

void command_raidloot(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #raidloot [All|GroupLeader|RaidLeader|Selected]");
		return;
	}

	auto client_raid = c->GetRaid();
	if (!client_raid) {
		c->Message(Chat::White, "You must be in a Raid to use this command.");
		return;
	}

	if (!client_raid->IsLeader(c)) {
		c->Message(Chat::White, "You must be the Raid Leader to use this command.");
		return;
	}

	std::string raid_loot_type  = Strings::ToLower(sep->arg[1]);
	bool        is_all          = raid_loot_type.find("all") != std::string::npos;
	bool        is_group_leader = raid_loot_type.find("groupleader") != std::string::npos;
	bool        is_raid_leader  = raid_loot_type.find("raidleader") != std::string::npos;
	bool        is_selected     = raid_loot_type.find("selected") != std::string::npos;
	if (
		!is_all &&
		!is_group_leader &&
		!is_raid_leader &&
		!is_selected
		) {
		c->Message(Chat::White, "Usage: #raidloot [All|GroupLeader|RaidLeader|Selected]");
		return;
	}

	std::map<uint32, std::string> loot_types = {
		{RaidLootTypes::All,         "All"},
		{RaidLootTypes::GroupLeader, "GroupLeader"},
		{RaidLootTypes::RaidLeader,  "RaidLeader"},
		{RaidLootTypes::Selected,    "Selected"}
	};

	uint32 loot_type;
	if (is_all) {
		loot_type = RaidLootTypes::All;
	}
	else if (is_group_leader) {
		loot_type = RaidLootTypes::GroupLeader;
	}
	else if (is_raid_leader) {
		loot_type = RaidLootTypes::RaidLeader;
	}
	else if (is_selected) {
		loot_type = RaidLootTypes::Selected;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Loot type changed to {} ({}).",
			loot_types[loot_type],
			loot_type
		).c_str()
	);
}
