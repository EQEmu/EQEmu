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
#include "zone/petitions.h"

void command_delpetition(Client *c, const Seperator *sep)
{
    if (sep->arg[1][0] == 0 ||
        !sep->IsNumber(1)) {
        c->Message(
            Chat::White,
            "Usage: #delpetition (petition number) "
            "Type #listpetition for a list"
        );
        return;
    }

    const auto id = Strings::ToInt(sep->arg[1]);

    if (id <= 0) {
        return;
    }

    auto *petitions = PetitionList::Instance();

    petitions->ReadDatabase();

    if (petitions->DeletePetition(
            static_cast<uint32>(id)) != 0) {
        c->Message(
            Chat::Red,
            "Petition %i could not be deleted.",
            id
        );
        return;
    }

    c->Message(
        Chat::White,
        "Petition %i deleted.",
        id
    );

    LogInfo(
        "Delete petition request from [{}], "
        "petition number [{}]",
        c->GetName(),
        id
    );
}

