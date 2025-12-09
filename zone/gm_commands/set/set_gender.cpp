#include "../../client.h"

void SetGender(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set gender [Gender ID]");
		c->Message(Chat::White, "Genders: 0 = Male, 1 = Female, 2 = Neuter");
		return;
	}

	Mob *t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	const uint8 gender_id = Strings::ToUnsignedInt(sep->arg[2]);
	if (!EQ::ValueWithin(gender_id, Gender::Male, Gender::Neuter)) {
		c->Message(Chat::White, "Usage: #set gender [Gender ID]");
		c->Message(Chat::White, "Genders: 0 = Male, 1 = Female, 2 = Neuter");
		return;
	}

	t->SendIllusionPacket(
		AppearanceStruct{
			.gender_id = gender_id,
			.race_id = t->GetRace(),
			.size = t->GetSize(),
		}
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Gender changed for {} to {} ({}).",
			c->GetTargetDescription(t),
			GetGenderName(gender_id),
			gender_id
		).c_str()
	);
}
