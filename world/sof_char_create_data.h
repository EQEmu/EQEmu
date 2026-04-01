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
#pragma once

#pragma pack(push)
#pragma pack(1)

struct RaceClassAllocation {
	unsigned int Index;
	unsigned int BaseStats[7];
	unsigned int DefaultPointAllocation[7];
};

struct RaceClassCombos {
	unsigned int ExpansionRequired;
	unsigned int Race;
	unsigned int Class;
	unsigned int Deity;
	unsigned int AllocationIndex;
	unsigned int Zone;
};

/*struct SoFCCData {
	unsigned char Unknown;
	unsigned int RaceClassStatEntryCount;
	SoFCCRaceClassData RCData[109];
	unsigned int Unknown2;
	SoFCCStartZoneData StartZoneData[641];
};
*/

#pragma pack(pop)
