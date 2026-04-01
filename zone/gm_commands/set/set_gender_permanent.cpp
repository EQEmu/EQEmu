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

void SetGenderPermanent(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set gender_permanent [Gender ID]");
		c->Message(Chat::White, "Genders: 0 = Male, 1 = Female, 2 = Neuter");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const uint8 gender_id = Strings::ToInt(sep->arg[2]);
	if (!EQ::ValueWithin(gender_id, Gender::Male, Gender::Neuter)) {
		c->Message(Chat::White, "Usage: #set gender_permanent [Gender ID]");
		c->Message(Chat::White, "Genders: 0 = Male, 1 = Female, 2 = Neuter");
		return;
	}

	LogInfo("Gender changed by {} for {} to {} ({})",
		c->GetCleanName(),
		c->GetTargetDescription(t),
		GetGenderName(gender_id),
		gender_id
	);

	t->SetBaseGender(gender_id);
	t->Save();
	t->SendIllusionPacket(
		AppearanceStruct{
			.gender_id = gender_id,
			.race_id = t->GetRace(),
			.size = t->GetSize(),
		}
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Gender changed for {} to {} ({}).",
			c->GetTargetDescription(t),
			GetGenderName(gender_id),
			gender_id
		).c_str()
	);
}
