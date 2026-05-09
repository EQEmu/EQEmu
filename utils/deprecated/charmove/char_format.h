#ifndef CHAR_FORMAT_H
#define CHAR_FORMAT_H

#include "../common/types.h"

#define CHAR_MAGIC 0xaa77e598

#pragma pack(1)
struct CharHeader {
	uint32 char_magic;
	
	//character_ data
	char name[64];
	float x;
	float y;
	float z;
	char zone_name[32];
	uint16 zone_id;
	uint32 profile_length;
	uint32 extprofile_length;
	
	//faction_values
	uint32 faction_value_count;
	
	//inventory
	uint32 inventory_count;
	
	//sharedbank
	uint32 sharedbank_count;
	
	//quest_globals
	uint32 quest_globals_count;
	
	/*
	Variable length fields:
	byte PlayerProfile[profile_length];
	byte ExtendedProfile[extprofile_length];
	FactionValueRecord faction_values[faction_value_count];
	InventoryEntry inventory[inventory_count];
	InventoryEntry sharedbank[sharedbank_count];
	QuestGlobalEntry quest_globals[quest_globals_count];
	
	*/
};


struct FactionValueRecord {
	sint32 faction_id;
	sint32 current_value;
};

struct InventoryEntry {
	uint16	slotid;
	uint32	itemid;
	sint16	charges;
	uint32	colors;
	uint32	augs[5];
	uint8	instnodrop;
};

struct QuestGlobalEntry {
	sint32	npcid;
	sint32	zoneid;
	char	name[66];
	char	value[66];
	sint32	expdate;
};
  

#pragma pack()



#endif



