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
#include "zone/groups.h"
#include "zone/raids.h"

void SetAAEXP(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 3 || !sep->IsNumber(3)) {
		c->Message(Chat::White, "Usage: #set aa_exp [aa|group|raid] [Amount]");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const std::string& aa_type = Strings::ToLower(sep->arg[2]);
	const uint32 aa_experience = Strings::ToUnsignedInt(sep->arg[3]);

	std::string group_raid_string;

	const bool is_aa    = Strings::EqualFold(aa_type, "aa");
	const bool is_group = Strings::EqualFold(aa_type, "group");
	const bool is_raid  = Strings::EqualFold(aa_type, "raid");

	if (
		!is_aa &&
		!is_group &&
		!is_raid
	) {
		c->Message(Chat::White, "Usage: #set aa_exp [aa|group|raid] [Amount]");
		return;
	}

	if (is_aa) {
		t->SetEXP(
			ExpSource::GM,
			t->GetEXP(),
			aa_experience,
			false
		);
	} else if (is_group) {
		group_raid_string = "Group ";

		t->SetLeadershipEXP(
			aa_experience,
			t->GetRaidEXP()
		);
	} else if (is_raid) {
		group_raid_string = "Raid ";

		t->SetLeadershipEXP(
			t->GetGroupEXP(),
			aa_experience
		);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} now {} {} {}AA Experience.",
			c->GetTargetDescription(t, TargetDescriptionType::UCYou),
			c == t ? "have" : "has",
			Strings::Commify(aa_experience),
			group_raid_string
		).c_str()
	);
}
