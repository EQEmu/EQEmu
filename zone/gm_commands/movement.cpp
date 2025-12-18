#include "zone/client.h"
#include "zone/mob_movement_manager.h"

void command_movement(Client *c, const Seperator *sep)
{
	const int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #movement [clear|packet|rotate|run|stats|stop|walk]");
		return;
	}

	const bool is_clear  = !strcasecmp(sep->arg[1], "clear");
	const bool is_packet = !strcasecmp(sep->arg[1], "packet");
	const bool is_rotate = !strcasecmp(sep->arg[1], "rotate");
	const bool is_run    = !strcasecmp(sep->arg[1], "run");
	const bool is_stats  = !strcasecmp(sep->arg[1], "stats");
	const bool is_stop   = !strcasecmp(sep->arg[1], "stop");
	const bool is_walk   = !strcasecmp(sep->arg[1], "walk");

	if (
		!is_clear &&
		!is_packet &&
		!is_rotate &&
		!is_run &&
		!is_stats &&
		!is_stop &&
		!is_walk
	) {
		c->Message(Chat::White, "Usage: #movement [clear|packet|rotate|run|stats|stop|walk]");
		return;
	}

	auto &m = MobMovementManager::Get();

	if (is_clear) {
		m.ClearStats();
	} else if (is_packet) {
		Mob *t = c->GetTarget();
		if (!t) {
			c->Message(Chat::White, "You must have a target to use this command.");
			return;
		}

		const float x         = sep->IsNumber(2) ? Strings::ToFloat(sep->arg[2]) : 0.0f;
		const float y         = sep->IsNumber(3) ? Strings::ToFloat(sep->arg[3]) : 0.0f;
		const float z         = sep->IsNumber(4) ? Strings::ToFloat(sep->arg[4]) : 0.0f;
		const float heading   = sep->IsNumber(5) ? Strings::ToFloat(sep->arg[5]) : 0.0f;
		const int   animation = sep->IsNumber(6) ? Strings::ToInt(sep->arg[6]) : 0.0f;

		m.SendCommandToClients(t, x, y, z, heading, animation, ClientRangeAny);
	} else if (is_rotate) {
		Mob *t = c->GetTarget();
		if (!t) {
			c->Message(Chat::White, "You must have a target to use this command.");
			return;
		}

		t->RotateToWalking(t->CalculateHeadingToTarget(c->GetX(), c->GetY()));
	} else if (is_run) {
		Mob *t = c->GetTarget();
		if (!t) {
			c->Message(Chat::White, "You must have a target to use this command.");
			return;
		}

		t->RunTo(c->GetX(), c->GetY(), c->GetZ());
	} else if (is_stats) {
		m.DumpStats(c);
	} else if (is_stop) {
		Mob *t = c->GetTarget();
		if (!t) {
			c->Message(Chat::White, "You must have a target to use this command.");
			return;
		}

		t->StopNavigation();
	} else if (is_walk) {
		Mob *t = c->GetTarget();
		if (!t) {
			c->Message(Chat::White, "You must have a target to use this command.");
			return;
		}

		t->WalkTo(c->GetX(), c->GetY(), c->GetZ());
	}
}
