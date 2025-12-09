#include "../../client.h"
#include "../../../common/data_verification.h"

void SetFlymode(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set flymode [Flymode ID]");

		for (const auto& e : EQ::constants::GetFlyModeMap()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Flymode {} | {}",
					e.first,
					e.second
				).c_str()
			);
		}

		return;
	}

	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	const int8 flymode_id = Strings::ToInt(sep->arg[2]);
	if (
		!EQ::ValueWithin(
			flymode_id,
			EQ::constants::GravityBehavior::Ground,
			EQ::constants::GravityBehavior::LevitateWhileRunning
		)
	) {
		c->Message(Chat::White, "Usage: #set flymode [Flymode ID]");

		for (const auto& e : EQ::constants::GetFlyModeMap()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Flymode {} | {}",
					e.first,
					e.second
				).c_str()
			);
		}

		return;
	}

	t->SetFlyMode(static_cast<GravityBehavior>(flymode_id));
	t->SendAppearancePacket(AppearanceType::FlyMode, flymode_id);

	const uint32 account = c->AccountID();

	database.SetGMFlymode(account, flymode_id);

	c->Message(
		Chat::White,
		fmt::format(
			"Fly Mode for {} is now {} ({}).",
			c->GetTargetDescription(t),
			EQ::constants::GetFlyModeName(flymode_id),
			flymode_id
		).c_str()
	);
}
