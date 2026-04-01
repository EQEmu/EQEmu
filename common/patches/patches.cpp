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
#include "patches.h"

#include "common/patches/rof.h"
#include "common/patches/rof2.h"
#include "common/patches/sod.h"
#include "common/patches/sof.h"
#include "common/patches/titanium.h"
#include "common/patches/uf.h"


void RegisterAllPatches(EQStreamIdentifier &into)
{
	Titanium::Register(into);
	SoF::Register(into);
	SoD::Register(into);
	UF::Register(into);
	RoF::Register(into);
	RoF2::Register(into);
}

void ReloadAllPatches()
{
	Titanium::Reload();
	SoF::Reload();
	SoD::Reload();
	UF::Reload();
	RoF::Reload();
	RoF2::Reload();
}
