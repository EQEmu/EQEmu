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
