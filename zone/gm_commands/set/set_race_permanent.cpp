#include "../../client.h"

void SetRacePermanent(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set race_permanent [Race ID]");
		c->Message(
			Chat::White,
			"NOTE: Not all models are global. If a model is not global, it will appear as a Human on character select and in zones without the model."
		);

		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const uint16 race_id   = Strings::ToUnsignedInt(sep->arg[2]);
	const uint8  gender_id = Mob::GetDefaultGender(race_id, t->GetBaseGender());

	LogInfo("Race changed by {} for {} to {} ({})",
		c->GetCleanName(),
		c->GetTargetDescription(t),
		GetRaceIDName(race_id),
		race_id
	);

	t->SetBaseRace(race_id);
	t->SetBaseGender(gender_id);
	t->Save();
	t->SendIllusionPacket(
		AppearanceStruct{
			.gender_id = gender_id,
			.race_id = race_id,
			.size = t->GetSize()
		}
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Race changed for {} to {} ({}).",
			c->GetTargetDescription(t),
			GetRaceIDName(race_id),
			race_id
		).c_str()
	);
}
