#include "zone/client.h"
#include "zone/corpse.h"

void command_save(Client *c, const Seperator *sep)
{
	if (
		!c->GetTarget() ||
		(
			c->GetTarget() &&
			!c->GetTarget()->IsClient() &&
			!c->GetTarget()->IsPlayerCorpse()
		)
	) {
		c->Message(Chat::White, "You must target a player or player corpse to use this command.");
		return;
	}

	auto target = c->GetTarget();

	if (target->IsClient()) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} been {} saved.",
				c->GetTargetDescription(target, TargetDescriptionType::UCYou),
				c == target ? "have" : "has",
				target->CastToClient()->Save(2) ? "successfully" : "failed to be"
			).c_str()
		);
	} else if (target->IsPlayerCorpse()) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} has been {} saved.",
				c->GetTargetDescription(target),
				target->CastToMob()->Save() ? "successfully" : "failed to be"
			).c_str()
		);
	}
}

