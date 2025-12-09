#include "../../client.h"

void SetGenderPermanent(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set gender_permanent [Gender ID]");
		c->Message(Chat::White, "Genders: 0 = Male, 1 = Female, 2 = Neuter");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const uint8 gender_id = Strings::ToInt(sep->arg[2]);
	if (!EQ::ValueWithin(gender_id, Gender::Male, Gender::Neuter)) {
		c->Message(Chat::White, "Usage: #set gender_permanent [Gender ID]");
		c->Message(Chat::White, "Genders: 0 = Male, 1 = Female, 2 = Neuter");
		return;
	}

	LogInfo("Gender changed by {} for {} to {} ({})",
		c->GetCleanName(),
		c->GetTargetDescription(t),
		GetGenderName(gender_id),
		gender_id
	);

	t->SetBaseGender(gender_id);
	t->Save();
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
