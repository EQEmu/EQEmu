#include "../../client.h"

void SetTexture(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set texture [Texture] [Helmet Texture]");
		return;
	}

	const uint8 texture        = Strings::ToUnsignedInt(sep->arg[2]);
	const uint8 helmet_texture = (
		sep->IsNumber(3) ?
		Strings::ToUnsignedInt(sep->arg[3]) :
		0
	);

	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	if (IsPlayerRace(t->GetModel())) { // Player Races Wear Armor, so Wearchange is sent instead
		for (
			int texture_slot = EQ::textures::textureBegin;
			texture_slot <= EQ::textures::LastTintableTexture;
			texture_slot++
		) {
			t->SendTextureWC(texture_slot, texture);
		}
	} else { // Non-Player Races only need Illusion Packets to be sent for texture
		t->SendIllusionPacket(
			AppearanceStruct{
				.gender_id = t->GetGender(),
				.helmet_texture = helmet_texture,
				.race_id = t->GetModel(),
				.texture = texture,
			}
		);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Texture Changed for {} | Texture: {}{}",
			c->GetTargetDescription(t, TargetDescriptionType::UCSelf),
			texture,
			(
				IsPlayerRace(t->GetModel()) ?
				"" :
				fmt::format(
					" Helmet Texture: {}",
					helmet_texture
				)
			)
		).c_str()
	);
}
