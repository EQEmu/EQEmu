#include "zone/client.h"

void command_feature(Client *c, const Seperator *sep)
{
	std::string command         = sep->arg[0] ? sep->arg[0] : "";
	const bool  is_size_alias   = sep->arg[0] && Strings::Contains(command, "#size");
	const bool  is_nested_alias = is_size_alias;

	const auto arguments = sep->argnum;
	if ((arguments < 2 || !sep->IsNumber(2)) && !is_nested_alias) {
		SendFeatureSubCommands(c);
		return;
	}

	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	const bool is_beard       = !strcasecmp(sep->arg[1], "beard");
	const bool is_beard_color = !strcasecmp(sep->arg[1], "beardcolor");
	const bool is_details     = !strcasecmp(sep->arg[1], "details");
	const bool is_eyes        = !strcasecmp(sep->arg[1], "eyes");
	const bool is_face        = !strcasecmp(sep->arg[1], "face");
	const bool is_gender      = !strcasecmp(sep->arg[1], "gender");
	const bool is_hair        = !strcasecmp(sep->arg[1], "hair");
	const bool is_hair_color  = !strcasecmp(sep->arg[1], "haircolor");
	const bool is_helm        = !strcasecmp(sep->arg[1], "helm");
	const bool is_heritage    = !strcasecmp(sep->arg[1], "heritage");
	const bool is_race        = !strcasecmp(sep->arg[1], "race");
	const bool is_size        = !strcasecmp(sep->arg[1], "size") || is_size_alias;
	const bool is_tattoo      = !strcasecmp(sep->arg[1], "tattoo");
	const bool is_texture     = !strcasecmp(sep->arg[1], "texture");

	if (
		!is_beard &&
		!is_beard_color &&
		!is_details &&
		!is_eyes &&
		!is_face &&
		!is_gender &&
		!is_hair &&
		!is_hair_color &&
		!is_helm &&
		!is_heritage &&
		!is_race &&
		!is_size &&
		!is_tattoo &&
		!is_texture
	) {
		SendFeatureSubCommands(c);
		return;
	}

	FaceChange_Struct f{
		.haircolor = t->GetHairColor(),
		.beardcolor = t->GetBeardColor(),
		.eyecolor1 = t->GetEyeColor1(),
		.eyecolor2 = t->GetEyeColor2(),
		.hairstyle = t->GetHairStyle(),
		.beard = t->GetBeard(),
		.face = t->GetLuclinFace(),
		.drakkin_heritage = t->GetDrakkinHeritage(),
		.drakkin_tattoo = t->GetDrakkinTattoo(),
		.drakkin_details = t->GetDrakkinDetails(),
	};

	uint8  gender       = t->GetGender();
	uint8  helm_texture = t->GetHelmTexture();
	uint16 race         = t->GetModel();
	float  size         = t->GetSize();
	uint8  texture      = t->GetTexture();

	std::string feature_changed;
	float value_changed = 0.0f;

	if (is_beard) {
		f.beard = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));
		feature_changed = "Beard";
		value_changed = f.beard;
	} else if (is_beard_color) {
		f.beardcolor = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));
		feature_changed = "Beard Color";
		value_changed = f.beardcolor;
	} else if (is_details) {
		if (t->GetRace() != Race::Drakkin) {
			c->Message(Chat::White, "You must target a Drakkin to use this command.");
			return;
		}

		f.drakkin_details = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));
		feature_changed = "Drakkin Details";
		value_changed = static_cast<float>(f.drakkin_details);
	} else if (is_eyes) {
		f.eyecolor1 = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));
		feature_changed = "Eyes";
		value_changed = f.eyecolor1; // eyecolor2 isn't used
	} else if (is_face) {
		f.face = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));
		feature_changed = "Face";
		value_changed = f.face;
	} else if (is_gender) {
		gender = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));
		feature_changed = "Gender";
		value_changed = gender;
	} else if (is_hair) {
		f.hairstyle = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));
		feature_changed = "Hair";
		value_changed = f.hairstyle;
	} else if (is_hair_color) {
		f.haircolor = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));
		feature_changed = "Hair Color";
		value_changed = f.haircolor;
	} else if (is_helm) {
		helm_texture = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));
		feature_changed = "Helmet Texture";
		value_changed = helm_texture;
	} else if (is_heritage) {
		if (t->GetRace() != Race::Drakkin) {
			c->Message(Chat::White, "You must target a Drakkin to use this command.");
			return;
		}

		f.drakkin_heritage = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));
		feature_changed = "Drakkin Heritage";
		value_changed = static_cast<float>(f.drakkin_heritage);
	} else if (is_race) {
		race = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[2]));
		feature_changed = "Race";
		value_changed = race;
	} else if (is_size) {
		// handle aliased input
		if (is_size_alias) {
			c->Message(Chat::White, "Usage: #feature size [Size] - Change your or your target's Size temporarily (Valid values are 0 to 255, decimal increments are allowed.)");
			if (sep->arg[1] && Strings::IsFloat(sep->arg[1])) {
				size = Strings::ToFloat(sep->arg[1]);
			}
		}
		else {
			size = Strings::ToFloat(sep->arg[2]);
		}

		if (size < 0 || size > 255) {
			c->Message(Chat::White, "Usage: #feature size [Size] - Change your or your target's Size temporarily (Valid values are 0 to 255, decimal increments are allowed.)");
			return;
		}

		feature_changed = "Size";
		value_changed = size;
	} else if (is_tattoo) {
		if (t->GetRace() != Race::Drakkin) {
			c->Message(Chat::White, "You must target a Drakkin to use this command.");
			return;
		}

		f.drakkin_tattoo = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));
		feature_changed = "Drakkin Tattoos";
		value_changed = static_cast<float>(f.drakkin_tattoo);
	} else if (is_texture) {
		texture = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));
		feature_changed = "Texture";
		value_changed = texture;
	}

	// For now face number is not set through SetFace. This is because the
	// client may not update face features after being set to an invalid face
	// until a specific valid face number is re-sent (needs more research)
	if (
		!is_face &&
		!is_gender &&
		!is_helm &&
		!is_race &&
		!is_size &&
		!is_texture
	) {
		t->SetFaceAppearance(f);
	} else {
		t->SendIllusionPacket(
			AppearanceStruct{
				.beard = f.beard,
				.beard_color = f.beardcolor,
				.drakkin_details = f.drakkin_details,
				.drakkin_heritage = f.drakkin_heritage,
				.drakkin_tattoo = f.drakkin_tattoo,
				.eye_color_one = t->GetEyeColor1(),
				.eye_color_two = t->GetEyeColor2(),
				.face = f.face,
				.gender_id = gender,
				.hair = f.hairstyle,
				.hair_color = f.haircolor,
				.helmet_texture = helm_texture,
				.race_id = race,
				.size = size,
				.texture = texture,
			}
		);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} set for {} to {}.",
			feature_changed,
			c->GetTargetDescription(t),
			(
				is_size ?
				fmt::format(
					"{:.2f}",
					value_changed
				) :
				fmt::format(
					"{}",
					static_cast<uint8>(value_changed)
				)
			)
		).c_str()
	);
}

void SendFeatureSubCommands(Client *c)
{
	const std::string& feature_save_link = Saylink::Silent("#npcedit featuresave");

	c->Message(Chat::White, "Usage: #feature beard [Beard] - Change your or your target's Beard");
	c->Message(Chat::White, "Usage: #feature beardcolor [Beard Color] - Change your or your target's Beard Color");
	c->Message(Chat::White, "Usage: #feature details [Details] - Change your or your target's Drakkin Details");
	c->Message(Chat::White, "Usage: #feature eyes [Eye Color] - Change your or your target's Eyes");
	c->Message(Chat::White, "Usage: #feature face [Face] - Change your or your target's Face");
	c->Message(Chat::White, "Usage: #feature gender [Gender] - Change your or your target's Gender");
	c->Message(Chat::White, "Usage: #feature hair [Hair] - Change your or your target's Hair");
	c->Message(Chat::White, "Usage: #feature haircolor [Hair Color] - Change your or your target's Hair Color");
	c->Message(Chat::White, "Usage: #feature helm [Helmet Texture] - Change your or your target's Helmet Texture");
	c->Message(Chat::White, "Usage: #feature heritage [Heritage] - Change your or your target's Drakkin Heritage");
	c->Message(Chat::White, "Usage: #feature race [Race ID] - Change your or your target's Race");
	c->Message(Chat::White, "Usage: #feature size [Size] - Change your or your target's Size (Valid values are 0 to 255, decimal increments are allowed.)");
	c->Message(Chat::White, "Usage: #feature tattoo [Tattoo] - Change your or your target's Drakkin Tattoos");
	c->Message(Chat::White, "Usage: #feature texture [Texture] - Change your or your target's Texture");
	c->Message(
		Chat::White,
		fmt::format(
			"Note: All features are temporary. If your target is an NPC, you can save these features to the database using {}.",
			feature_save_link
		).c_str()
	);
}
