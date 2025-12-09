#include "../../client.h"

void SetHeroModel(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set hero_model [Hero Model]");
		c->Message(Chat::White, "Usage: #set hero_model [Hero Model] [Slot]");
		c->Message(
			Chat::White,
			fmt::format(
				"Example: {}",
				Saylink::Silent("#heromodel 63")
			).c_str()
		);
		return;
	}

	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	uint32 hero_forge_model = Strings::ToUnsignedInt(sep->arg[2]);

	if (arguments > 2) {
		const uint8 slot = Strings::ToUnsignedInt(sep->arg[3]);
		c->GetTarget()->SendTextureWC(slot, 0, hero_forge_model, 0, 0, 0);
	} else {
		if (!hero_forge_model) {
			c->Message(Chat::White, "Hero's Forge Model must be greater than 0.");
			return;
		}

		// Conversion to simplify the command arguments
		// Hero's Forge model is actually model * 1000 + texture * 100 + wearslot
		// Hero's Forge Model slot 7 is actually for Robes, but it still needs to use wearslot 1 in the packet
		hero_forge_model *= 100;

		for (uint8 slot = 0; slot < 7; slot++) {
			c->GetTarget()->SendTextureWC(slot, 0, (hero_forge_model + slot), 0, 0, 0);
		}
	}
}
