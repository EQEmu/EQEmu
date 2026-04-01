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
#include "common/repositories/character_data_repository.h"
#include "zone/client.h"

void SetAnon(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set anon [Anonymous Flag]");
		c->Message(Chat::White, "Usage: #set anon [Character ID] [Anonymous Flag]");
		c->Message(Chat::White, "Note: 0 = Not Anonymous, 1 = Anonymous, 2 = Roleplaying");
		return;
	}

	if (arguments == 2) {
		const uint8 anon_flag = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));

		auto t = c;
		if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM()) {
			t = c->GetTarget()->CastToClient();
		}

		std::string anon_setting;
		if (anon_flag == Anonymity::NotAnonymous) {
			anon_setting = "no longer Anonymous or Roleplaying";
		} else if (anon_flag == Anonymity::Anonymous) {
			anon_setting = "now Anonymous";
		} else if (anon_flag == Anonymity::Roleplaying) {
			anon_setting = "now Roleplaying";
		} else {
			c->Message(Chat::White, "Usage: #set anon [Anonymous Flag]");
			c->Message(Chat::White, "Usage: #set anon [Character ID] [Anonymous Flag]");
			c->Message(Chat::White, "Note: 0 = Not Anonymous, 1 = Anonymous, 2 = Roleplaying");
			return;
		}

		c->SetAnon(anon_flag);

		c->Message(
			Chat::White,
			fmt::format(
				"{} {} {}.",
				c->GetTargetDescription(t, TargetDescriptionType::UCYou),
				c == t ? "are" : "is",
				anon_setting
			).c_str()
		);
	} else if (arguments == 3) {
		const int   character_id = Strings::ToInt(sep->arg[2]);
		const uint8 anon_flag    = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[3]));

		auto e = CharacterDataRepository::FindOne(content_db, character_id);
		if (!e.id) {
			c->Message(
				Chat::White,
				fmt::format(
					"Character ID {} does not exist or is invalid.",
					character_id
				).c_str()
			);

			return;
		}

		e.anon = anon_flag;

		auto updated = CharacterDataRepository::UpdateOne(content_db, e);

		if (!updated) {
			c->Message(
				Chat::White,
				fmt::format(
					"Failed to change Anonymous Flag for {} ({}).",
					e.name,
					character_id
				).c_str()
			);

			return;
		}

		std::string anon_setting;
		if (anon_flag == Anonymity::NotAnonymous) {
			anon_setting = "no longer Anonymous or Roleplaying";
		} else if (anon_flag == Anonymity::Anonymous) {
			anon_setting = "now Anonymous";
		} else if (anon_flag == Anonymity::Roleplaying) {
			anon_setting = "now Roleplaying";
		} else {
			c->Message(Chat::White, "Usage: #set anon [Anonymous Flag]");
			c->Message(Chat::White, "Usage: #set anon [Character ID] [Anonymous Flag]");
			c->Message(Chat::White, "Note: 0 = Not Anonymous, 1 = Anonymous, 2 = Roleplaying");
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"{} ({}) is {}.",
				e.name,
				character_id,
				anon_setting
			).c_str()
		);
	}
}
