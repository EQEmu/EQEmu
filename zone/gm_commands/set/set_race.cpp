#include "../../client.h"

void SetRace(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: #set race [0-{}, 2253-2259] (0 for back to normal)",
				RuleI(NPC, MaxRaceID)
			).c_str()
		);

		return;
	}

	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	const uint16 race_id = Strings::ToUnsignedInt(sep->arg[2]);

	if (
		!EQ::ValueWithin(race_id, Race::Doug, RuleI(NPC, MaxRaceID)) &&
		!EQ::ValueWithin(race_id, 2253, 2259)
	) {
		c->Message(
			Chat::White,
			fmt::format(
				"Usage: #race [0-{}, 2253-2259] (0 for back to normal)",
				RuleI(NPC, MaxRaceID)
			).c_str()
		);

		return;
	}

	t->SendIllusionPacket(
		AppearanceStruct{
			.race_id = race_id,
			.size = t->GetSize(),
		}
	);

	if (race_id == Race::Doug) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} been returned to {} base race.",
				c->GetTargetDescription(t, TargetDescriptionType::UCYou),
				c == t ? "have" : "has",
				c == t ? "your" : "their"
			).c_str()
		);
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} {} now temporarily a(n) {} ({}).",
			c->GetTargetDescription(t, TargetDescriptionType::UCYou),
			c == t ? "are" : "is",
			GetRaceIDName(race_id),
			race_id
		).c_str()
	);
}
