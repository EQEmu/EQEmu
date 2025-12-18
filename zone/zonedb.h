
#pragma once

#include "common/eq_packet_structs.h"
#include "common/eqemu_logsys.h"
#include "common/faction.h"
#include "common/races.h"
#include "common/repositories/doors_repository.h"
#include "common/repositories/npc_faction_entries_repository.h"
#include "common/shareddb.h"
#include "zone/aa_ability.h"
#include "zone/bot_database.h"
#include "zone/event_codes.h"
#include "zone/position.h"

#include <unordered_set>

class Client;
class Corpse;
class Merc;
class NPC;
class Petition;
class Spawn2;
class SpawnGroupList;
class Trap;
struct Door;
struct ExtendedProfile_Struct;
struct NPCType;
struct PlayerCorpse_Struct;
struct ZonePoint;
struct npcDecayTimes_Struct;

namespace EQ
{
	class ItemInstance;
}

template <class TYPE> class LinkedList;

//#include "doors.h"

struct wplist {
	int index;
	float x;
	float y;
	float z;
	int pause;
	float heading;
	bool centerpoint;
};

#pragma pack(1)
struct DBnpcspells_entries_Struct {
	uint16	spellid;
	uint8	minlevel;
	uint8	maxlevel;
	uint32	type;
	int16	manacost;
	int16	priority;
	int32	recast_delay;
	int16	resist_adjust;
	int8	min_hp;
	int8	max_hp;
};
#pragma pack()

#pragma pack(1)
struct DBnpcspellseffects_entries_Struct {
	int16	spelleffectid;
	uint8	minlevel;
	uint8	maxlevel;
	int32	base_value;
	int32	limit;
	int32	max_value;
};
#pragma pack()

struct DBnpcspells_Struct {
	uint32	parent_list;
	uint16	attack_proc;
	uint8	proc_chance;
	uint16	range_proc;
	int16	rproc_chance;
	uint16	defensive_proc;
	int16	dproc_chance;
	uint32	fail_recast;
	uint32	engaged_no_sp_recast_min;
	uint32	engaged_no_sp_recast_max;
	uint8	engaged_beneficial_self_chance;
	uint8	engaged_beneficial_other_chance;
	uint8	engaged_detrimental_chance;
	uint32  pursue_no_sp_recast_min;
	uint32  pursue_no_sp_recast_max;
	uint8   pursue_detrimental_chance;
	uint32  idle_no_sp_recast_min;
	uint32  idle_no_sp_recast_max;
	uint8	idle_beneficial_chance;
	std::vector<DBnpcspells_entries_Struct> entries;
};

struct DBnpcspellseffects_Struct {
	uint32	parent_list;
	uint32	numentries;
	DBnpcspellseffects_entries_Struct entries[0];
};

#pragma pack(1)
struct DBbotspells_entries_Struct {
	uint16		spellid;
	uint8		minlevel;
	uint8		maxlevel;
	uint32		type;
	int16		manacost;
	int16		priority;
	int32		recast_delay;
	int16		resist_adjust;
	int8		min_hp;
	int8		max_hp;
	std::string	bucket_name;
	std::string	bucket_value;
	uint8		bucket_comparison;
};
#pragma pack()

struct DBbotspells_Struct {
	uint32	parent_list;
	uint16	attack_proc;
	uint8	proc_chance;
	uint16	range_proc;
	int16	rproc_chance;
	uint16	defensive_proc;
	int16	dproc_chance;
	uint32	fail_recast;
	uint32	engaged_no_sp_recast_min;
	uint32	engaged_no_sp_recast_max;
	uint8	engaged_beneficial_self_chance;
	uint8	engaged_beneficial_other_chance;
	uint8	engaged_detrimental_chance;
	uint32	pursue_no_sp_recast_min;
	uint32	pursue_no_sp_recast_max;
	uint8	pursue_detrimental_chance;
	uint32	idle_no_sp_recast_min;
	uint32	idle_no_sp_recast_max;
	uint8	idle_beneficial_chance;
	std::vector<DBbotspells_entries_Struct> entries;
};

struct DBTradeskillRecipe_Struct {
	EQ::skills::SkillType tradeskill;
	int16 skill_needed;
	uint16 trivial;
	bool nofail;
	bool replace_container;
	std::vector< std::pair<uint32,uint8> > onsuccess;
	std::vector< std::pair<uint32,uint8> > onfail;
	std::vector< std::pair<uint32,uint8> > salvage;
	std::string name;
	uint8 must_learn;
	bool has_learnt;
	uint32 madecount;
	uint32 recipe_id;
	bool quest;
};

struct PetRecord {
	uint32 npc_type;	// npc_type id for the pet data to use
	bool temporary;
	int16 petpower;
	uint8 petcontrol;	// What kind of control over the pet is possible (Animation, familiar, ...)
	uint8 petnaming;		// How to name the pet (Warder, pet, random name, familiar, ...)
	bool monsterflag;	// flag for if a random monster appearance should get picked
	uint32 equipmentset;	// default equipment for the pet
};

struct AuraRecord {
	uint32 npc_type;
	char name[64]; // name shown in UI if shown and spawn name
	int spell_id;
	int distance;
	int aura_type;
	int spawn_type;
	int movement;
	int duration; // seconds some live for 90 mins (normal) others for 2 mins (traps)
	int icon; // -1 will use the buffs NEW_ICON
	int cast_time; // seconds some auras recast on a timer, most seem to be every 12 seconds
};

// Actual pet info for a client.
struct PetInfo {
	uint16	SpellID;
	int16	petpower;
	uint32	HP;
	uint32	Mana;
	float	size;
	SpellBuff_Struct	Buffs[PET_BUFF_COUNT];
	uint32	Items[EQ::invslot::EQUIPMENT_COUNT];
	char	Name[64];
	bool	taunting;
};

struct ZoneSpellsBlocked {
	uint32 spellid;
	int8 type;
	glm::vec3 m_Location;
	glm::vec3 m_Difference;
	char message[256];
};

struct TraderCharges_Struct {
	uint32 ItemID[EQ::invtype::BAZAAR_SIZE];
	int32  SerialNumber[EQ::invtype::BAZAAR_SIZE];
	uint32 ItemCost[EQ::invtype::BAZAAR_SIZE];
	int32  Charges[EQ::invtype::BAZAAR_SIZE];
};

const int MaxMercStanceID = 9;

struct MercStanceInfo {
	uint8	ProficiencyID;
	uint8	ClassID;
	uint32	StanceID;
	uint8	IsDefault;
};

struct MercTemplate {
	uint32	MercTemplateID;
	uint32	MercType;				// From dbstr_us.txt - Apprentice (330000100), Journeyman (330000200), Master (330000300)
	uint32	MercSubType;			// From dbstr_us.txt - 330020105^23^Race: Guktan<br>Type: Healer<br>Confidence: High<br>Proficiency: Apprentice, Tier V...
	uint16	RaceID;
	uint8	ClassID;
	uint32	MercNPCID;
	uint8	ProficiencyID;
	uint8	TierID;
	uint8	CostFormula;			// To determine cost to client
	uint32	ClientVersion;				// Only send valid mercs per expansion
	uint8	MercNameType;			// Determines if merc gets random name or default text
	char	MercNamePrefix[25];
	char	MercNameSuffix[25];
	uint32	Stances[MaxMercStanceID];
};

struct MercInfo {
	uint32	mercid;
	uint8	slot;
	char	merc_name[64];
	uint32	MercTemplateID;
	const	MercTemplate* myTemplate;
	uint32	SuspendedTime;
	bool	IsSuspended;
	uint32	MercTimerRemaining;
	uint8	Gender;
	float	MercSize;
	int32	State;
	uint32	Stance;
	int32	hp;
	int32	mana;
	int32	endurance;
	uint8	face;
	uint8	luclinHairStyle;
	uint8	luclinHairColor;
	uint8	luclinEyeColor;
	uint8	luclinEyeColor2;
	uint8	luclinBeardColor;
	uint8	luclinBeard;
	uint32	drakkinHeritage;
	uint32	drakkinTattoo;
	uint32	drakkinDetails;
};

struct MercSpellEntry {
	uint8	proficiencyid;
	uint16	spellid;		// <= 0 = no spell
	uint32	type;			// 0 = never, must be one (and only one) of the defined values
	int16	stance;			// 0 = all, + = only this stance, - = all except this stance
	uint8	minlevel;
	uint8	maxlevel;
	int16	slot;
	uint16	proc_chance;
	uint32	time_cancast;	// when we can cast this spell next
};

struct ClientMercEntry {
	uint32 id;
	uint32 npcid;
};

struct CharacterCorpseItemEntry
{
	uint32 item_id;
	int16 equip_slot;
	uint16 charges;
	uint16 lootslot;
	uint32 aug_1;
	uint32 aug_2;
	uint32 aug_3;
	uint32 aug_4;
	uint32 aug_5;
	uint32 aug_6;
	bool attuned;
	std::string custom_data;
	uint32 ornamenticon;
	uint32 ornamentidfile;
	uint32 ornament_hero_model;
};

struct CharacterCorpseEntry
{
	bool locked;
	uint64 exp;
	uint64 gm_exp;
	float size;
	uint8 level;
	uint32 race;
	uint8 gender;
	uint8 class_;
	uint8 deity;
	uint8 texture;
	uint8 helmtexture;
	uint32 copper;
	uint32 silver;
	uint32 gold;
	uint32 plat;
	EQ::TintProfile item_tint;
	uint8 haircolor;
	uint8 beardcolor;
	uint8 eyecolor1;
	uint8 eyecolor2;
	uint8 hairstyle;
	uint8 face;
	uint8 beard;
	uint8 killed_by;
	bool rezzable;
	uint32 rez_time;
	uint32 drakkin_heritage;
	uint32 drakkin_tattoo;
	uint32 drakkin_details;
	std::vector<CharacterCorpseItemEntry> items;
	std::string entity_variables;
};

namespace BeastlordPetData {
	struct PetStruct {
		uint16 race_id       = Race::Wolf;
		uint8  texture       = 0;
		uint8  helm_texture  = 0;
		uint8  gender        = Gender::Neuter;
		float  size_modifier = 1.0f;
		uint8  face          = 0;
	};
}

namespace NPCSpawnTypes {
	enum : uint8 {
		CreateNewSpawn,
		AddNewSpawngroup,
		UpdateAppearance,
		RemoveSpawn,
		DeleteSpawn,
		AddSpawnFromSpawngroup,
		CreateNewNPC
  };
}

namespace RaidLootTypes {
	enum : uint32 {
		RaidLeader = 1,
		GroupLeader,
		Selected,
		All
	};
}

class ZoneDatabase : public SharedDatabase {
	typedef std::list<LootItem*> ItemList;
public:
	ZoneDatabase();
	ZoneDatabase(const char* host, const char* user, const char* passwd, const char* database,uint32 port);
	virtual ~ZoneDatabase();

	/* Objects and World Containers  */
	void	LoadWorldContainer(uint32 parentid, EQ::ItemInstance* container);
	void	SaveWorldContainer(uint32 zone_id, uint32 parent_id, const EQ::ItemInstance* container);
	void	DeleteWorldContainer(uint32 parent_id,uint32 zone_id);
	uint32	AddObject(uint32 type, uint32 icon, const Object_Struct& object, const EQ::ItemInstance* inst);
	void	UpdateObject(uint32 object_id, uint32 type, uint32 icon, const Object_Struct& object, const EQ::ItemInstance* inst);
	void	DeleteObject(uint32 object_id);
	GroundSpawns*	LoadGroundSpawns(uint32 zone_id, int16 instance_version, GroundSpawns* gs);

	/* Traders  */
	void	SaveTraderItem(uint32 char_id,uint32 itemid,uint32 uniqueid, int32 charges,uint32 itemcost,uint8 slot);
	void	UpdateTraderItemCharges(int char_id, uint32 ItemInstID, int32 charges);
	void	UpdateTraderItemPrice(int char_id, uint32 item_id, uint32 charges, uint32 new_price);
	void	DeleteTraderItem(uint32 char_id);
	void	DeleteTraderItem(uint32 char_id,uint16 slot_id);

	std::unique_ptr<EQ::ItemInstance> LoadSingleTraderItem(uint32 char_id, int serial_number);
	Trader_Struct* LoadTraderItem(uint32 char_id);
	TraderCharges_Struct* LoadTraderItemWithCharges(uint32 char_id);

	/* Buyer/Barter  */
	void AddBuyLine(uint32 CharID, uint32 BuySlot, uint32 ItemID, const char *ItemName, uint32 Quantity, uint32 Price);
	void RemoveBuyLine(uint32 CharID, uint32 BuySlot);
	void UpdateBuyLine(uint32 CharID, uint32 BuySlot, uint32 Quantity);


	void UpdateGMStatus(uint32 account_id, int new_status);

	/**
	 ************************************************
	 * Character
	 ************************************************
	 */

	bool	SetServerFilters(char* name, ServerSideFilters_Struct *ssfs);
	uint32	GetServerFilters(char* name, ServerSideFilters_Struct *ssfs);

	void SaveBuffs(Client *c);
	void LoadBuffs(Client *c);
	void SaveAuras(Client *c);
	void LoadAuras(Client *c);
	void LoadPetInfo(Client *c);
	void SavePetInfo(Client *c);
	void RemoveTempFactions(Client *c);
	void UpdateItemRecast(uint32 char_id, uint32 recast_type, uint32 timestamp);
	void DeleteItemRecast(uint32 char_id, uint32 recast_type);

	bool DeleteCharacterAAs(uint32 character_id);
	bool DeleteCharacterBandolier(uint32 character_id, uint32 bandolier_id);
	bool DeleteCharacterDiscipline(uint32 character_id, uint32 slot_id);
	bool DeleteCharacterMaterialColor(uint32 character_id);
	bool DeleteCharacterLeadershipAbilities(uint32 character_id);
	bool DeleteCharacterMemorizedSpell(uint32 character_id, uint32 slot_id);
	bool DeleteCharacterSpell(uint32 character_id, uint32 slot_id);

	bool LoadCharacterBandolier(uint32 character_id, PlayerProfile_Struct* pp);
	bool LoadCharacterBindPoint(uint32 character_id, PlayerProfile_Struct* pp);
	bool LoadCharacterCurrency(uint32 character_id, PlayerProfile_Struct* pp);
	bool LoadCharacterData(uint32 character_id, PlayerProfile_Struct* pp, ExtendedProfile_Struct* m_epp);
	bool LoadCharacterDisciplines(Client* c);
	bool LoadCharacterFactionValues(uint32 character_id, faction_map & val_list);
	bool LoadCharacterLanguages(uint32 character_id, PlayerProfile_Struct* pp);
	bool LoadCharacterLeadershipAbilities(uint32 character_id, PlayerProfile_Struct* pp);
	bool LoadCharacterMaterialColor(uint32 character_id, PlayerProfile_Struct* pp);
	bool LoadCharacterMemmedSpells(uint32 character_id, PlayerProfile_Struct* pp);
	bool LoadCharacterPotionBelt(uint32 character_id, PlayerProfile_Struct* pp);
	bool LoadCharacterSkills(uint32 character_id, PlayerProfile_Struct* pp);
	bool LoadCharacterSpellBook(uint32 character_id, PlayerProfile_Struct* pp);

	bool SaveCharacterBandolier(uint32 character_id, uint8 bandolier_id, uint8 bandolier_slot, uint32 item_id, uint32 icon, const char* bandolier_name);
	bool SaveCharacterCurrency(uint32 character_id, PlayerProfile_Struct* pp);
	bool SaveCharacterData(Client* c, PlayerProfile_Struct* pp, ExtendedProfile_Struct* m_epp);
	bool SaveCharacterDiscipline(uint32 character_id, uint32 slot_id, uint32 disc_id);
	bool SaveCharacterLanguage(uint32 character_id, uint32 lang_id, uint32 value);
	bool SaveCharacterLeadershipAbilities(uint32 character_id, PlayerProfile_Struct* pp);
	bool SaveCharacterMaterialColor(uint32 character_id, uint8 slot_id, uint32 color);
	bool SaveCharacterMemorizedSpell(uint32 character_id, uint32 spell_id, uint32 slot_id);
	bool SaveCharacterPotionBelt(uint32 character_id, uint8 potion_id, uint32 item_id, uint32 icon);
	bool SaveCharacterSkill(uint32 character_id, uint32 skill_id, uint32 value);
	bool SaveCharacterSpell(uint32 character_id, uint32 spell_id, uint32 slot_id);

	void ZeroPlayerProfileCurrency(PlayerProfile_Struct* pp);

	/* EXP Modifiers */
	void LoadCharacterEXPModifier(Client* c);
	void SaveCharacterEXPModifier(Client *c);

	/* Player Title Sets */
	void LoadCharacterTitleSets(Client* c);

	float GetAAEXPModifierByCharID(uint32 character_id, uint32 zone_id, int16 instance_version = -1);
	float GetEXPModifierByCharID(uint32 character_id, uint32 zone_id, int16 instance_version = -1);
	void SetAAEXPModifierByCharID(uint32 character_id, uint32 zone_id, float aa_modifier, int16 instance_version = -1);
	void SetEXPModifierByCharID(uint32 character_id, uint32 zone_id, float exp_modifier, int16 instance_version = -1);

	/* Character Inventory  */
	bool	NoRentExpired(const std::string& name);
	bool	SaveCharacterInvSnapshot(uint32 character_id);
	int		CountCharacterInvSnapshots(uint32 character_id);
	void	ClearCharacterInvSnapshots(uint32 character_id, bool from_now = false);
	void	ListCharacterInvSnapshots(uint32 character_id, std::list<std::pair<uint32, int>> &is_list);
	bool	ValidateCharacterInvSnapshotTimestamp(uint32 character_id, uint32 timestamp);
	void	ParseCharacterInvSnapshot(uint32 character_id, uint32 timestamp, std::list<std::pair<int16, uint32>> &parse_list);
	void	DivergeCharacterInvSnapshotFromInventory(uint32 character_id, uint32 timestamp, std::list<std::pair<int16, uint32>> &compare_list);
	void	DivergeCharacterInventoryFromInvSnapshot(uint32 character_id, uint32 timestamp, std::list<std::pair<int16, uint32>> &compare_list);
	bool	RestoreCharacterInvSnapshot(uint32 character_id, uint32 timestamp);

	/* Corpses  */
	bool BuryAllCharacterCorpses(uint32 character_id);
	bool BuryCharacterCorpse(uint32 corpse_id);
	int64 CountCharacterCorpses(uint32 character_id);
	int64 CountCharacterCorpsesByZoneID(uint32 character_id, uint32 zone_id);
	bool DeleteCharacterCorpse(uint32 corpse_id);
	bool DeleteItemOffCharacterCorpse(uint32 corpse_id, uint32 slot_id, uint32 item_id);
	uint32 GetCharacterBuriedCorpseCount(uint32 character_id);
	int64 GetCharacterCorpseCount(uint32 character_id);
	uint32 GetCharacterCorpseDecayTimer(uint32 corpse_id);
	uint32 GetCharacterCorpseID(uint32 character_id, uint8 corpse_limit);
	uint32 GetCharacterCorpseItemAt(uint32 corpse_id, uint16 slot_id);
	bool GetDecayTimes(npcDecayTimes_Struct* npc_decay_times);
	uint32 GetFirstCorpseID(uint32 character_id);
	Corpse* LoadCharacterCorpse(uint32 corpse_id);
	bool LoadCharacterCorpses(uint32 zone_id, uint16 instance_id);
	void MarkCorpseAsResurrected(uint32 corpse_id);
	uint32 SaveCharacterCorpse(uint32 character_id, const std::string& name, uint32 zone_id, uint16 instance_id, const CharacterCorpseEntry& c, const glm::vec4& position, uint32 guild_consent_id);
	uint32 SendCharacterCorpseToGraveyard(uint32 corpse_id, uint32 zone_id, uint16 instance_id, glm::vec4& position);
	void SendCharacterCorpseToNonInstance(uint32 corpse_id);
	Corpse* SummonBuriedCharacterCorpses(uint32 character_id, uint32 zone_id, uint16 instance_id, const glm::vec4& position);
	bool SummonAllCharacterCorpses(uint32 character_id, uint32 zone_id, uint16 instance_id, const glm::vec4& position);
	bool UnburyCharacterCorpse(uint32 corpse_id, uint32 zone_id, uint16 instance_id, const glm::vec4& position);
	uint32 UpdateCharacterCorpse(uint32 corpse_id, uint32 character_id, const std::string& name, uint32 zone_id, uint16 instance_id, const CharacterCorpseEntry& c, const glm::vec4& position, uint32 guild_consent_id, bool is_resurrected = false);
	uint32 UpdateCharacterCorpseConsent(uint32 character_id, uint32 guild_consent_id);

	/* Faction   */
	bool		GetFactionData(FactionMods* fd, uint32 class_mod, uint32 race_mod, uint32 deity_mod, int32 faction_id); //needed for factions Dec, 16 2001
	bool		GetFactionName(int faction_id, char* name, uint32 buflen); // needed for factions Dec, 16 2001
	std::string GetFactionName(int faction_id);
	bool		GetFactionIDsForNPC(uint32 npc_faction_id, std::list<NpcFactionEntriesRepository::NpcFactionEntries>* faction_list, int32* primary_faction = 0); // improve faction handling
	bool		SetCharacterFactionLevel(uint32 char_id, int32 faction_id, int32 value, uint8 temp, faction_map &val_list); // needed for factions Dec, 16 2001
	bool		LoadFactionData();
	inline uint32 GetMaxFaction() { return max_faction; }

	/* AAs New */
	bool	LoadAlternateAdvancementAbilities(std::unordered_map<int, std::unique_ptr<AA::Ability>> &abilities,
											  std::unordered_map<int, std::unique_ptr<AA::Rank>> &ranks);
	bool	LoadAlternateAdvancement(Client *c);

	/* Zone related   */
	bool		SaveZoneCFG(uint32 zone_id, uint16 instance_version, NewZone_Struct* zd);
	bool		LoadStaticZonePoints(LinkedList<ZonePoint*>* zone_point_list,const char* zonename, uint32 version);
	int			getZoneShutDownDelay(uint32 zoneID, uint32 version);

	/* Spawns and Spawn Points  */
	bool		LoadSpawnGroups(const char* zone_name, uint16 version, SpawnGroupList* spawn_group_list);
	bool		LoadSpawnGroupsByID(int spawn_group_id, SpawnGroupList* spawn_group_list);
	bool		PopulateZoneSpawnList(uint32 zoneid, LinkedList<Spawn2*> &spawn2_list, int16 version);
	bool		CreateSpawn2(Client* c, uint32 spawngroup_id, const std::string& zone_short_name, const glm::vec4& position, uint32 respawn, uint32 variance, uint16 condition, int16 condition_value);
	void		UpdateRespawnTime(uint32 spawn2_id, uint16 instance_id,uint32 timeleft);
	uint32		GetSpawnTimeLeft(uint32 spawn2_id, uint16 instance_id);
	void        UpdateSpawn2Status(uint32 id, uint8 new_status, uint32 instance_id);

	/* Grids/Paths  */
	uint32 GetFreeGrid(uint32 zone_id);
	void DeleteWaypoint(Client* c, uint32 grid_id, uint32 number, uint32 zone_id);
	void AddWaypoint(Client* c, uint32 grid_id, uint32 number, const glm::vec4 &position, uint32 pause, uint32 zone_id);
	uint32 AddWaypointForSpawn(Client* c, uint32 spawn2_id, const glm::vec4 &position, uint32 pause, int type, int type2, uint32 zone_id);
	void ModifyGrid(Client* c, bool remove, uint32 grid_id, uint8 type = 0, uint8 type2 = 0, uint32 zone_id = 0);
	bool GridExistsInZone(uint32 zone_id, uint32 grid_id);
	int GetHighestGrid(uint32 zone_id);
	int GetHighestWaypoint(uint32 zone_id, uint32 grid_id);
	int GetRandomWaypointFromGrid(glm::vec4 &loc, uint32 zone_id, uint32 grid_id);

	/* NPCs  */

	uint32		NPCSpawnDB(uint8 command, const std::string& zone, uint32 instance_version, Client *c, NPC* n = 0, uint32 extra = 0); // 0 = Create 1 = Add; 2 = Update; 3 = Remove; 4 = Delete
	uint32		CreateNewNPCCommand(const std::string& zone, uint32 instance_version, Client* c, NPC* n, uint32 extra);
	uint32		AddNewNPCSpawnGroupCommand(const std::string& zone, uint32 instance_version, Client* c, NPC* n, uint32 in_respawn_time);
	uint32		DeleteSpawnLeaveInNPCTypeTable(const std::string& zone, Client* c, NPC* n, uint32 remove_spawngroup_id);
	uint32		DeleteSpawnRemoveFromNPCTypeTable(const std::string& zone, uint32 instance_version, Client* c, NPC* n);
	uint32		AddSpawnFromSpawnGroup(const std::string& zone, uint32 instance_version, Client* c, NPC* n, uint32 spawngroup_id);
	uint32		AddNPCTypes(const std::string& zone, uint32 instance_version, Client* c, NPC* n, uint32 spawngroup_id);
	uint32		UpdateNPCTypeAppearance(Client* c, NPC* n);
	bool		SetSpecialAttkFlag(uint8 id, const char* flag);
	bool		GetPetEntry(const std::string& pet_type, PetRecord* r);
	bool		GetPoweredPetEntry(const std::string& pet_type, int16 pet_power, PetRecord* r);
	bool		GetBasePetItems(int32 equipmentset, uint32 *items);
	BeastlordPetData::PetStruct GetBeastlordPetData(uint16 race_id);
	uint32		GetMaxNPCSpellsEffectsID();
	bool GetAuraEntry(uint16 spell_id, AuraRecord &record);
	void LoadGlobalLoot();

	DBnpcspells_Struct*				GetNPCSpells(uint32 npc_spells_id);
	DBnpcspellseffects_Struct*		GetNPCSpellsEffects(uint32 iDBSpellsEffectsID);
	void ClearNPCSpells() { npc_spells_cache.clear(); npc_spells_loadtried.clear(); }
	const NPCType* LoadNPCTypesData(uint32 id, bool bulk_load = false);

	/*Bots	*/
	DBbotspells_Struct*	GetBotSpells(uint32 bot_spell_id);
	void ClearBotSpells() { bot_spells_cache.clear(); bot_spells_loadtried.clear(); }

	/* Mercs   */
	const NPCType* GetMercenaryType(uint32 id, uint16 race_id, uint32 owner_level);
	void LoadMercenaryEquipment(Merc* m);
	void SaveMercenaryBuffs(Merc* m);
	void LoadMercenaryBuffs(Merc* m);
	bool LoadMercenaryInfo(Client* c);
	bool LoadCurrentMercenary(Client* c);
	bool SaveMercenary(Merc* m);
	bool DeleteMercenary(uint32 mercenary_id);

	/* Petitions   */
	void	DeletePetitionFromDB(Petition* wpet);
	void	UpdatePetitionToDB(Petition* wpet);
	void	InsertPetitionToDB(Petition* wpet);
	void	RefreshPetitionsFromDB();

	/* Merchants  */
	void	SaveMerchantTemp(uint32 npcid, uint32 slot, uint32 zone_id, uint32 instance_id, uint32 item, uint32 charges);
	void	DeleteMerchantTemp(uint32 npcid, uint32 slot, uint32 zone_id, uint32 instance_id);

	/* Tradeskills  */
	bool	GetTradeRecipe(const EQ::ItemInstance* container, uint8 c_type, uint32 some_id, Client* c, DBTradeskillRecipe_Struct* spec, bool* is_augmented);
	bool	GetTradeRecipe(uint32 recipe_id, uint8 c_type, uint32 some_id, Client* c, DBTradeskillRecipe_Struct* spec);
	uint32	LoadForage(uint32 zone_id, uint8 skill_level);
	uint32	LoadFishing(uint32 zone_id, uint8 skill_level, uint32 &npc_id, uint8 &npc_chance);
	void	UpdateRecipeMadecount(uint32 recipe_id, uint32 char_id, uint32 madecount);
	bool	EnableRecipe(uint32 recipe_id);
	bool	DisableRecipe(uint32 recipe_id);
	std::vector<uint32> GetRecipeComponentItemIDs(RecipeCountType count_type, uint32 recipe_id);
	int8 GetRecipeComponentCount(RecipeCountType count_type, uint32 recipe_id, uint32 item_id);

	/* Tribute  */
	bool	LoadTributes();

	/* Doors   */
	std::vector<DoorsRepository::Doors> LoadDoors(const std::string& zone_name, int16 version);
	uint32 GetDoorsCountPlusOne();
	int GetDoorsDBCountPlusOne(std::string zone_short_name, int16 version);

	/* Blocked Spells   */
	int64 GetBlockedSpellsCount(uint32 zone_id);
	bool LoadBlockedSpells(int64 blocked_spells_count, ZoneSpellsBlocked* into, uint32 zone_id);

	/* Traps   */
	bool	LoadTraps(const std::string& zone_short_name, int16 instance_version);
	bool	SetTrapData(Trap* t, bool repop = false);

	/* Time   */
	uint32	GetZoneTimezone(uint32 zoneid, uint32 version);
	bool	SetZoneTimezone(uint32 zoneid, uint32 version, uint32 tz);

	/* Group   */
	void RefreshGroupFromDB(Client *c);

	/* Instancing   */
	void ListAllInstances(Client* c, uint32 character_id);

	/* QGlobals   */
	void QGlobalPurge();

	/* Alternate Currency   */
	void LoadAltCurrencyValues(uint32 char_id, std::map<uint32, uint32> &currency);
	void UpdateAltCurrencyValue(uint32 char_id, uint32 currency_id, uint32 value);

	/*
		* Misc stuff.
		* PLEASE DO NOT ADD TO THIS COLLECTION OF CRAP UNLESS YOUR METHOD
		* REALLY HAS NO BETTER SECTION
	*/
	uint32	GetKarma(uint32 account_id);
	void	UpdateKarma(uint32 account_id, uint32 amount);

	// bot database add-on to eliminate the need for a second database connection
	BotDatabase botdb;

	static void LoadCharacterTribute(Client* c);

	static void SaveCharacterBinds(Client *c);
	static void SaveCharacterTribute(Client* c);
protected:
	void ZDBInitVars();

	uint32				max_faction;
	Faction**			faction_array;
	uint32 npc_spellseffects_maxid;
	std::unordered_map<uint32, DBnpcspells_Struct> npc_spells_cache;
	std::unordered_set<uint32> npc_spells_loadtried;
	DBnpcspellseffects_Struct** npc_spellseffects_cache;
	bool*				npc_spellseffects_loadtried;
	std::unordered_map<uint32, DBbotspells_Struct> bot_spells_cache;
	std::unordered_set<uint32> bot_spells_loadtried;

};

extern ZoneDatabase database;
extern ZoneDatabase content_db;
