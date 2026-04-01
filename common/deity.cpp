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
#include "deity.h"

uint32 Deity::GetBitmask(uint32 deity_id)
{
	return IsValid(deity_id) ? deity_bitmasks[deity_id] : Deity::Bitmask::All;
}

std::string Deity::GetName(uint32 deity_id)
{
	return IsValid(deity_id) ? deity_names[deity_id] : "UNKNOWN DEITY";
}

bool Deity::IsValid(uint32 deity_id)
{
	return deity_names.find(deity_id) != deity_names.end();
}
