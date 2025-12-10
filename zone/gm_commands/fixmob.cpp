#include "../client.h"

void SendFixMobSubCommands(Client* c)
{
	c->Message(Chat::White, "Usage: #fixmob beard [Next|Previous]");
	c->Message(Chat::White, "Usage: #fixmob beard_color [Next|Previous]");
	c->Message(Chat::White, "Usage: #fixmob drakkin_details [Next|Previous]");
	c->Message(Chat::White, "Usage: #fixmob drakkin_heritage [Next|Previous]");
	c->Message(Chat::White, "Usage: #fixmob drakkin_tattoo [Next|Previous]");
	c->Message(Chat::White, "Usage: #fixmob face [Next|Previous]");
	c->Message(Chat::White, "Usage: #fixmob gender [Next|Previous]");
	c->Message(Chat::White, "Usage: #fixmob hair [Next|Previous]");
	c->Message(Chat::White, "Usage: #fixmob hair_color [Next|Previous]");
	c->Message(Chat::White, "Usage: #fixmob helm [Next|Previous]");
	c->Message(Chat::White, "Usage: #fixmob race [Next|Previous]");
	c->Message(Chat::White, "Usage: #fixmob texture [Next|Previous]");
}

void command_fixmob(Client *c, const Seperator *sep)
{
	const int arguments = sep->argnum;
	if (arguments < 2) {
		SendFixMobSubCommands(c);
		return;
	}

	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	const std::string& type      = sep->arg[1];
	const std::string& move_type = Strings::ToLower(sep->arg[2]);

	std::string change_type;
	std::string change_value;

	const bool is_next     = move_type[0] == 'n';
	const bool is_previous = move_type[0] == 'p';

	const uint32 adjustment = is_next ? 1 : -1;

	uint16 race_id          = t->GetRace();
	uint8  gender_id        = t->GetGender();
	uint8  texture          = UINT8_MAX;
	uint8  helm_texture     = UINT8_MAX;
	uint8  hair_color       = t->GetHairColor();
	uint8  beard_color      = t->GetBeardColor();
	uint8  eye_color_1      = t->GetEyeColor1();
	uint8  eye_color_2      = t->GetEyeColor2();
	uint8  hair_style       = t->GetHairStyle();
	uint8  face             = t->GetLuclinFace();
	uint8  beard_style      = t->GetBeard();
	uint8  drakkin_heritage = t->GetDrakkinHeritage();
	uint8  drakkin_tattoo   = t->GetDrakkinTattoo();
	uint8  drakkin_details  = t->GetDrakkinDetails();

	const bool is_beard            = Strings::EqualFold(type, "beard");
	const bool is_beard_color      = Strings::EqualFold(type, "beard_color");
	const bool is_drakkin_details  = Strings::EqualFold(type, "drakkin_details");
	const bool is_drakkin_heritage = Strings::EqualFold(type, "drakkin_heritage");
	const bool is_drakkin_tattoo   = Strings::EqualFold(type, "drakkin_tattoo");
	const bool is_face             = Strings::EqualFold(type, "face");
	const bool is_gender           = Strings::EqualFold(type, "gender");
	const bool is_hair             = Strings::EqualFold(type, "hair");
	const bool is_hair_color       = Strings::EqualFold(type, "hair_color");
	const bool is_helm             = Strings::EqualFold(type, "helm");
	const bool is_race             = Strings::EqualFold(type, "race");
	const bool is_texture          = Strings::EqualFold(type, "texture");

	if (
		!is_beard &&
		!is_beard_color &&
		!is_drakkin_details &&
		!is_drakkin_heritage &&
		!is_drakkin_tattoo &&
		!is_face &&
		!is_gender &&
		!is_hair &&
		!is_hair_color &&
		!is_helm &&
		!is_race &&
		!is_texture
	) {
		SendFixMobSubCommands(c);
		return;
	}

	if (is_race) {
		if (race_id == Race::Human && is_previous) {
			race_id = RuleI(NPC, MaxRaceID);
		} else if (race_id >= RuleI(NPC, MaxRaceID) && is_next) {
			race_id = Race::Human;
		} else {
			race_id += adjustment;
		}

		change_type  = "Race";
		change_value = std::to_string(race_id);
	} else if (is_gender) {
		if (gender_id == Gender::Male && is_previous) {
			gender_id = Gender::Neuter;
		} else if (gender_id >= Gender::Neuter && is_next) {
			gender_id = Gender::Male;
		} else {
			gender_id += adjustment;
		}

		change_type  = "Gender";
		change_value = std::to_string(gender_id);
	} else if (is_texture) {
		texture = t->GetTexture();

		if (texture == 0 && is_previous) {
			texture = 25;
		} else if (texture >= 25 && is_next) {
			texture = 0;
		} else {
			texture += adjustment;
		}

		change_type  = "Texture";
		change_value = std::to_string(texture);
	} else if (is_helm) {
		helm_texture   = t->GetHelmTexture();
		if (helm_texture == 0 && is_previous) {
			helm_texture = 25;
		} else if (helm_texture >= 25 && is_next) {
			helm_texture = 0;
		} else {
			helm_texture += adjustment;
		}

		change_type  = "Helm Texture";
		change_value = std::to_string(helm_texture);
	} else if (is_face) {
		if (face == 0 && is_previous) {
			face = 87;
		} else if (face >= 87 && is_next) {
			face = 0;
		} else {
			face += adjustment;
		}

		change_type  = "Face";
		change_value = std::to_string(face);
	} else if (is_hair) {
		if (hair_style == 0 && is_previous) {
			hair_style = 8;
		} else if (hair_style >= 8 && is_next) {
			hair_style = 0;
		} else {
			hair_style += adjustment;
		}

		change_type  = "Hair Style";
		change_value = std::to_string(hair_style);
	} else if (is_hair_color) {
		if (hair_color == 0 && is_previous) {
			hair_color = 24;
		} else if (hair_color >= 24 && is_next) {
			hair_color = 0;
		} else {
			hair_color += adjustment;
		}

		change_type  = "Hair Color";
		change_value = std::to_string(hair_color);
	} else if (is_beard) {
		if (beard_style == 0 && is_previous) {
			beard_style = 11;
		} else if (beard_style >= 11 && is_next) {
			beard_style = 0;
		} else {
			beard_style += adjustment;
		}

		change_type  = "Beard Style";
		change_value = std::to_string(beard_style);
	} else if (is_beard_color) {
		if (beard_color == 0 && is_previous) {
			beard_color = 24;
		} else if (beard_color >= 24 && is_next) {
			beard_color = 0;
		} else {
			beard_color += adjustment;
		}

		change_type  = "Beard Color";
		change_value = std::to_string(beard_color);
	} else if (is_drakkin_heritage) {
		if (drakkin_heritage == 0 && is_previous) {
			drakkin_heritage = 6;
		} else if (drakkin_heritage >= 6 && is_next) {
			drakkin_heritage = 0;
		} else {
			drakkin_heritage += adjustment;
		}

		change_type  = "Drakkin Heritage";
		change_value = std::to_string(drakkin_heritage);
	} else if (is_drakkin_tattoo) {
		if (drakkin_tattoo == 0 && is_previous) {
			drakkin_tattoo = 8;
		} else if (drakkin_tattoo >= 8 && is_next) {
			drakkin_tattoo = 0;
		} else {
			drakkin_tattoo += adjustment;
		}

		change_type  = "Drakkin Tattoo";
		change_value = std::to_string(drakkin_tattoo);
	} else if (is_drakkin_details) {
		if (drakkin_details == 0 && is_previous) {
			drakkin_details = 7;
		} else if (drakkin_details >= 7 && is_next) {
			drakkin_details = 0;
		} else {
			drakkin_details += adjustment;
		}

		change_type  = "Drakkin Details";
		change_value = std::to_string(drakkin_details);
	}

	switch (race_id) {
		case Race::Barbarian: {
			if (face > 10) {
				face -= ((drakkin_tattoo - 1) * 10);
			}

			face += (drakkin_tattoo * 10);
			break;
		}
		case Race::Erudite: {
			if (face > 10) {
				face -= ((hair_style - 1) * 10);
			}

			face += (hair_style * 10);
			break;
		}
		case Race::HighElf:
		case Race::DarkElf:
		case Race::HalfElf: {
			if (face > 10) {
				face -= ((beard_style - 1) * 10);
			}

			face += (beard_style * 10);
			break;
		}
		default: {
			break;
		}
	}

	t->SendIllusionPacket(
		AppearanceStruct{
			.beard = beard_style,
			.beard_color = beard_color,
			.drakkin_details = drakkin_details,
			.drakkin_heritage = drakkin_heritage,
			.drakkin_tattoo = drakkin_tattoo,
			.eye_color_one = eye_color_1,
			.eye_color_two = eye_color_2,
			.face = face,
			.gender_id = gender_id,
			.hair = hair_style,
			.hair_color = hair_color,
			.helmet_texture = helm_texture,
			.race_id = race_id,
			.texture = texture,
		}
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Appearance for {} | Type: {} Value: {}",
			c->GetTargetDescription(t, TargetDescriptionType::UCSelf),
			change_type,
			change_value
		).c_str()
	);
}

