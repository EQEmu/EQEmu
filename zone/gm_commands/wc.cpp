#include "common/data_verification.h"
#include "zone/client.h"

void command_wc(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (
		arguments < 2 ||
		!sep->IsNumber(1) ||
		!sep->IsNumber(2)
	) {
		c->Message(Chat::White, "Usage: #wc [Slot ID] [Material]");
		c->Message(Chat::White, "Usage: #wc [Slot ID] [Material] [Hero Forge Model] [Elite Material]");
		return;
	}

	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	const uint8  slot_id          = Strings::ToUnsignedInt(sep->arg[1]);
	const uint32 texture          = Strings::ToUnsignedInt(sep->arg[2]);
	uint32       hero_forge_model = 0;
	uint32       elite_material   = 0;

	if (arguments >= 3 && sep->IsNumber(3)) {
		hero_forge_model = Strings::ToUnsignedInt(sep->arg[3]);

		if (EQ::ValueWithin(hero_forge_model, 1, 999)) { // Shorthand Hero Forge ID. Otherwise use the value the user entered.
			hero_forge_model = (hero_forge_model * 100) + slot_id;
		}
	}

	if (arguments >= 4 && sep->IsNumber(4)) {
		elite_material = Strings::ToUnsignedInt(sep->arg[4]);
	}

	t->SendTextureWC(
		slot_id,
		texture,
		hero_forge_model,
		elite_material
	);
}
