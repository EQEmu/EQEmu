#include "../../client.h"

void SetAnimation(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set animation [Animation ID]");

		for (const auto& a : EQ::constants::GetSpawnAnimationMap()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Animation {} | {}",
					a.first,
					a.second
				).c_str()
			);
		}

		return;
	}

	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	const auto animation_id = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));
	if (
		!EQ::ValueWithin(
			animation_id,
			static_cast<uint8>(eaStanding),
			static_cast<uint8>(eaLooting)
		)
	) {
		c->Message(Chat::White, "Usage: #set animation [Animation ID]");

		for (const auto& a : EQ::constants::GetSpawnAnimationMap()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Animation {} | {}",
					a.first,
					a.second
				).c_str()
			);
		}

		return;
	}

	t->SetAppearance(static_cast<EmuAppearance>(animation_id), false);

	c->Message(
		Chat::White,
		fmt::format(
			"Set animation to {} ({}) for {}.",
			EQ::constants::GetSpawnAnimationName(animation_id),
			animation_id,
			c->GetTargetDescription(t)
		).c_str()
	);
}
