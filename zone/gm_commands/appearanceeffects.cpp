/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "zone/client.h"

void command_appearanceeffects(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #appearanceeffects help - Display appearance effects help menu");
		c->Message(Chat::White, "Usage: #appearanceeffects remove - Remove all appearance effects saved to your target");
		c->Message(Chat::White, "Usage: #appearanceeffects set [Effect ID] [Slot ID] - Set an appearance effect saved to your target");
		c->Message(Chat::White, "Usage: #appearanceeffects view - Display all appearance effects saved to your target");
		return;
	}

	const bool is_help   = !strcasecmp(sep->arg[1], "help");
	const bool is_remove = !strcasecmp(sep->arg[1], "remove");
	const bool is_set    = !strcasecmp(sep->arg[1], "set");
	const bool is_view   = !strcasecmp(sep->arg[1], "view");

	if (is_help) {
		c->Message(Chat::White, "Usage: #appearanceeffects help - Display appearance effects help menu");
		c->Message(Chat::White, "Usage: #appearanceeffects remove - Remove all appearance effects saved to your target");
		c->Message(Chat::White, "Usage: #appearanceeffects set [Effect ID] [Slot ID] - Set an appearance effect saved to your target");
		c->Message(Chat::White, "Usage: #appearanceeffects view - Display all appearance effects saved to your target");
		return;
	}

	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	if (is_set) {
		if (arguments != 3 || !sep->IsNumber(2) || !sep->IsNumber(3)) {
			c->Message(Chat::White, "Usage: #appearanceeffects set [Effect ID] [Slot ID] - Set an appearance effect saved to your target");
			return;
		}

		const auto effect_id = Strings::ToUnsignedInt(sep->arg[2]);
		const auto slot_id   = Strings::ToUnsignedInt(sep->arg[3]);

		t->SendAppearanceEffect(
			effect_id,
			0,
			0,
			0,
			0,
			nullptr,
			slot_id,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0
		);
		c->Message(
			Chat::White,
			fmt::format(
				"Appearance Effect ID {} in slot ID {} has been set for {}.",
				effect_id,
				slot_id,
				c->GetTargetDescription(t)
			).c_str()
		);
	} else if (is_remove) {
		t->SendIllusionPacket(
			AppearanceStruct{
				.beard = t->GetBeard(),
				.beard_color = t->GetBeardColor(),
				.drakkin_details = t->GetDrakkinDetails(),
				.drakkin_heritage = t->GetDrakkinHeritage(),
				.drakkin_tattoo = t->GetDrakkinTattoo(),
				.eye_color_one = t->GetEyeColor1(),
				.eye_color_two = t->GetEyeColor2(),
				.face = t->GetLuclinFace(),
				.gender_id = t->GetGender(),
				.hair = t->GetHairStyle(),
				.hair_color = t->GetHairColor(),
				.helmet_texture = t->GetHelmTexture(),
				.race_id = t->GetRace(),
				.send_effects = false,
				.size = t->GetSize(),
				.texture = t->GetTexture(),
			}
		);
		t->ClearAppearanceEffects();
		c->Message(
			Chat::White,
			fmt::format(
				"Appearance Effects have been removed for {}.",
				c->GetTargetDescription(t)
			).c_str()
		);
	} else if (is_view) {
		t->ListAppearanceEffects(c);
	}
}
