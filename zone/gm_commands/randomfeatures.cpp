#include "zone/client.h"

void command_randomfeatures(Client *c, const Seperator *sep)
{
	if (!c->GetTarget()) {
		c->Message(Chat::White, "You must have a target to use this command.");
		return;
	}

	auto target = c->GetTarget();

	if (target->RandomizeFeatures()) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} had {} features randomized.",
				c->GetTargetDescription(target, TargetDescriptionType::UCYou),
				c == target ? "have" : "had",
				c == target ? "your" : "their"
			).c_str()
		);
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} not a player race, {} race is {} ({}).",
				c->GetTargetDescription(target, TargetDescriptionType::UCYou),
				c == target ? "are" : "is",
				c == target ? "your" : "their",
				GetRaceIDName(target->GetRace()),
				target->GetRace()
			).c_str()
		);
	}
}

