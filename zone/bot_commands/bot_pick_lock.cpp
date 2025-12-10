#include "../bot_command.h"
#include "../doors.h"

void bot_command_pick_lock(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_pick_lock", sep->arg[0], "picklock"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s", sep->arg[0]);
		c->Message(Chat::White, "requires one of the following bot classes:");
		c->Message(Chat::White, "Rogue(5) or Bard(40)");
		return;
	}

	std::vector<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	float pick_lock_value = 0.0f;
	ActionableBots::Filter_ByHighestPickLock(c, sbl, pick_lock_value);
	if (sbl.empty()) {
		c->Message(Chat::White, "No bots are capable of performing this action");
		return;
	}

	Bot* my_bot = sbl.front();

	my_bot->InterruptSpell();
	my_bot->RaidGroupSay("Attempting to pick the lock.");

	std::list<Doors*> door_list;
	entity_list.GetDoorsList(door_list);

	int door_count = 0, open_count = 0;
	for (auto door_iter : door_list) {
		if (!door_iter || door_iter->IsDoorOpen())
			continue;

		glm::tvec4<float, glm::highp> diff = (c->GetPosition() - door_iter->GetPosition());

		float curdist = ((diff.x * diff.x) + (diff.y * diff.y));
		float curelev = (diff.z * diff.z);
#if (EQDEBUG >= 11)
		if (curdist <= 130 && curelev <= 65 && curelev >= 25) // 2D limit is '130' (x^2 + y^2), 1D theoretically should be '65' (z^2)
			Log(Logs::Detail, Logs::Doors, "bot_command_pick_lock(): DoorID: %i - Elevation difference failure within theoretical limit (%f <= 65.0)", door_iter->GetDoorID(), curelev);
#endif
		if (curelev >= 25 || curdist > 130) // changed curelev from '10' to '25' - requiring diff.z to be less than '5'
			continue;

		++door_count;
		if (pick_lock_value >= door_iter->GetLockpick()) {
			door_iter->ForceOpen(my_bot);
			++open_count;
		}
		else {
			my_bot->RaidGroupSay("I am not skilled enough for this lock.");
		}
	}
	c->Message(Chat::White, "%i door%s attempted - %i door%s successful", door_count, ((door_count != 1) ? ("s") : ("")), open_count, ((open_count != 1) ? ("s") : ("")));
}
