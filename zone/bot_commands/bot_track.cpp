#include "../bot_command.h"

void bot_command_track(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_track", sep->arg[0], "track"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s (Ranger: [option=all: all | rare | local])", sep->arg[0]);
		c->Message(Chat::White, "requires one of the following bot classes:");
		c->Message(Chat::White, "Ranger(1), Druid(20) or Bard(35)");
		return;
	}

	std::string tracking_scope = sep->arg[1];

	std::vector<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	uint16 class_mask = (player_class_bitmasks[Class::Ranger] | player_class_bitmasks[Class::Druid] | player_class_bitmasks[Class::Bard]);
	ActionableBots::Filter_ByClasses(c, sbl, class_mask);

	Bot* my_bot = ActionableBots::AsSpawned_ByMinLevelAndClass(c, sbl, 1, Class::Ranger);
	if (tracking_scope.empty()) {
		if (!my_bot)
			my_bot = ActionableBots::AsSpawned_ByMinLevelAndClass(c, sbl, 20, Class::Druid);
		if (!my_bot)
			my_bot = ActionableBots::AsSpawned_ByMinLevelAndClass(c, sbl, 35, Class::Bard);
	}
	if (!my_bot) {
		c->Message(Chat::White, "No bots are capable of performing this action");
		return;
	}

	int base_distance = 0;
	bool track_named = false;
	std::string tracking_msg;
	switch (my_bot->GetClass()) {
		case Class::Ranger:
			if (!tracking_scope.compare("local")) {
				base_distance = 30;
				tracking_msg = "Local tracking...";
			}
			else if (!tracking_scope.compare("rare")) {
				base_distance = 80;
				bool track_named = true;
				tracking_msg = "Master tracking...";
			}
			else { // default to 'all'
				base_distance = 80;
				tracking_msg = "Advanced tracking...";
			}
			break;
		case Class::Druid:
			base_distance = 30;
			tracking_msg = "Local tracking...";
			break;
		case Class::Bard:
			base_distance = 20;
			tracking_msg = "Near tracking...";
			break;
		default:
			return;
	}
	if (!base_distance) {
		c->Message(Chat::White, "An unknown codition has occurred");
		return;
	}

	my_bot->InterruptSpell();
	my_bot->RaidGroupSay(tracking_msg.c_str());
	entity_list.ShowSpawnWindow(c, (c->GetLevel() * base_distance), track_named);
}
