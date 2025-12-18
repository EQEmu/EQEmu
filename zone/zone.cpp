/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2003 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "zone.h"

#include "common/data_verification.h"
#include "common/eqemu_logsys.h"
#include "common/features.h"
#include "common/repositories/alternate_currency_repository.h"
#include "common/repositories/buyer_repository.h"
#include "common/repositories/character_exp_modifiers_repository.h"
#include "common/repositories/criteria/content_filter_criteria.h"
#include "common/repositories/graveyard_repository.h"
#include "common/repositories/ldon_trap_entries_repository.h"
#include "common/repositories/ldon_trap_templates_repository.h"
#include "common/repositories/level_exp_mods_repository.h"
#include "common/repositories/merc_stance_entries_repository.h"
#include "common/repositories/merchantlist_repository.h"
#include "common/repositories/npc_emotes_repository.h"
#include "common/repositories/object_repository.h"
#include "common/repositories/respawn_times_repository.h"
#include "common/repositories/rule_sets_repository.h"
#include "common/repositories/trader_repository.h"
#include "common/repositories/zone_state_spawns_repository.h"
#include "common/rulesys.h"
#include "common/seperator.h"
#include "common/serverinfo.h"
#include "common/strings.h"
#include "zone/dynamic_zone.h"
#include "zone/guild_mgr.h"
#include "zone/map.h"
#include "zone/mob_movement_manager.h"
#include "zone/npc_scale_manager.h"
#include "zone/npc.h"
#include "zone/object.h"
#include "zone/pathfinder_null.h"
#include "zone/petitions.h"
#include "zone/quest_parser_collection.h"
#include "zone/spawn2.h"
#include "zone/spawngroup.h"
#include "zone/water_map.h"
#include "zone/worldserver.h"
#include "zone/zone_config.h"
#include "zone/zone_reload.h"

#include <cfloat>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>

extern bool staticzone;
extern QuestParserCollection* parse;
extern uint32 numclients;
extern WorldServer worldserver;
extern Zone* zone;
extern NpcScaleManager* npc_scale_manager;

volatile bool is_zone_loaded = false;
Zone* zone = 0;

void UpdateWindowTitle(char* iNewTitle);

bool Zone::Bootup(uint32 iZoneID, uint32 iInstanceID, bool is_static) {
	const char* zonename = ZoneName(iZoneID);

	if (iZoneID == 0 || zonename == 0)
		return false;
	if (zone != 0 || is_zone_loaded) {
		std::cerr << "Error: Zone::Bootup call when zone already booted!" << std::endl;
		worldserver.SetZoneData(0);
		return false;
	}

	LogInfo("Booting [{}] ([{}]:[{}])", zonename, iZoneID, iInstanceID);

	numclients = 0;
	zone = new Zone(iZoneID, iInstanceID, zonename);

	//init the zone, loads all the data, etc
	if (!zone->Init(is_static)) {
		safe_delete(zone);
		std::cerr << "Zone->Init failed" << std::endl;
		worldserver.SetZoneData(0);
		return false;
	}

	std::string tmp;
	if (database.GetVariable("loglevel", tmp)) {
		int log_levels[4];
		int tmp_i = Strings::ToInt(tmp);
		if (tmp_i>9){ //Server is using the new code
			for(int i=0;i<4;i++){
				if (((int)tmp[i]>=48) && ((int)tmp[i]<=57))
					log_levels[i]=(int)tmp[i]-48; //get the value to convert it to an int from the ascii value
				else
					log_levels[i]=0; //set to zero on a bogue char
			}
			zone->loglevelvar = log_levels[0];
			LogInfo("General logging level: [{}]", zone->loglevelvar);
			zone->merchantvar = log_levels[1];
			LogInfo("Merchant logging level: [{}]", zone->merchantvar);
			zone->tradevar = log_levels[2];
			LogInfo("Trade logging level: [{}]", zone->tradevar);
			zone->lootvar = log_levels[3];
			LogInfo("Loot logging level: [{}]", zone->lootvar);
		}
		else {
			zone->loglevelvar = uint8(tmp_i); //continue supporting only command logging (for now)
			zone->merchantvar = 0;
			zone->tradevar = 0;
			zone->lootvar = 0;
		}
	}

	is_zone_loaded = true;

	worldserver.SetZoneData(iZoneID, iInstanceID);
	if(iInstanceID != 0)
	{
		auto pack = new ServerPacket(ServerOP_AdventureZoneData, sizeof(uint16));
		*((uint16*)pack->pBuffer) = iInstanceID;
		worldserver.SendPacket(pack);
		delete pack;
	}

	LogInfo("Zone server [{}] listening on port [{}]", zonename, ZoneConfig::get()->ZonePort);
	LogInfo("Zone bootup type [{}] short_name [{}] zone_id [{}] instance_id [{}]",
		(is_static) ? "Static" : "Dynamic", zonename, iZoneID, iInstanceID);
	UpdateWindowTitle(nullptr);

	// Dynamic zones need to Sync here.
	// Static zones sync when they connect in worldserver.cpp.
	// Static zones cannot sync here as request is ignored by worldserver.
	if (!is_static) {
		zone->GetTimeSync();
	}

	zone->RequestUCSServerStatus();
	zone->StartShutdownTimer();

	DataBucket::LoadZoneCache(&database, iZoneID, iInstanceID);

	/*
	 * Set Logging
	 */
	EQEmuLogSys::Instance()->StartFileLogs(StringFormat("%s_version_%u_inst_id_%u_port_%u", zone->GetShortName(), zone->GetInstanceVersion(), zone->GetInstanceID(), ZoneConfig::get()->ZonePort));

	return true;
}

//this really loads the objects into entity_list
bool Zone::LoadZoneObjects()
{
	const auto &l = ObjectRepository::GetWhere(
		content_db,
		fmt::format(
			"zoneid = {} AND (version = {} OR version = -1) {}",
			zoneid,
			instanceversion,
			ContentFilterCriteria::apply()
		)
	);

	for (const auto &e : l) {
		if (e.type == ObjectTypes::StaticLocked) {
			const std::string &zone_short_name = ZoneName(e.zoneid, false);

			if (zone_short_name.empty()) {
				continue;
			}

			auto d = DoorsRepository::NewEntity();

			d.zone                = zone_short_name;
			d.id                  = 1000000000 + e.id;
			d.doorid              = -1;
			d.pos_x               = e.xpos;
			d.pos_y               = e.ypos;
			d.pos_z               = e.zpos;
			d.heading             = e.heading;
			d.name                = Strings::Replace(e.objectname, "_ACTORDEF", "");
			d.dest_zone           = "NONE";
			d.incline             = e.incline;
			d.client_version_mask = 0xFFFFFFFF;

			if (e.size_percentage == 0) {
				d.size = 100;
			}

			switch (d.opentype = e.solid_type)
			{
				case 0:
					d.opentype = 31;
					break;
				case 1:
					d.opentype = 9;
					break;
			}

			auto door = new Doors(d);
			entity_list.AddDoor(door);
		}

		Object_Struct data    = {0};
		uint32        id      = 0;
		uint32        icon    = 0;
		uint32        type    = 0;
		uint32        itemid  = 0;
		uint32        idx     = 0;
		int16         charges = 0;

		id = e.id;

		data.zone_id = e.zoneid;
		data.x       = e.xpos;
		data.y       = e.ypos;
		data.z       = e.zpos;
		data.heading = e.heading;

		itemid  = e.itemid;
		charges = e.charges;
		type    = e.type;
		icon    = e.icon;

		data.object_type = type;
		data.linked_list_addr[0] = 0;
		data.linked_list_addr[1] = 0;

		strn0cpy(data.object_name, e.objectname.c_str(), sizeof(data.object_name));

		data.solid_type = e.solid_type;
		data.incline    = e.incline;
		data.unknown024 = e.unknown24;
		data.unknown076 = e.unknown76;
		data.size       = e.size;
		data.tilt_x     = e.tilt_x;
		data.tilt_y     = e.tilt_y;
		data.unknown084 = 0;


		glm::vec3 position;
		position.x = data.x;
		position.y = data.y;
		position.z = data.z;

		if (zone->HasMap()) {
			data.z = zone->zonemap->FindBestZ(position, nullptr);
		}

		EQ::ItemInstance *inst = nullptr;
		// tradeskill containers do not have an itemid of 0
		if (!itemid) {
			// Generic tradeskill container
			inst = new EQ::ItemInstance(ItemInstWorldContainer);
		} else {
			// Groundspawn object
			inst = database.CreateItem(itemid);
		}

		if (!inst && type != ObjectTypes::Temporary) {
			inst = new EQ::ItemInstance(ItemInstWorldContainer);
		}

		// Load child objects if container
		if (inst && inst->IsType(EQ::item::ItemClassBag)) {
			database.LoadWorldContainer(id, inst);
		}

		auto object = new Object(id, type, icon, data, inst);

		object->SetDisplayName(e.display_name.c_str());

		entity_list.AddObject(object, false);

		if (type == ObjectTypes::Temporary && itemid) {
			entity_list.RemoveObject(object->GetID());
		}

		safe_delete(inst);
	}

	LogInfo("Loaded [{}] world objects", Strings::Commify(l.size()));

	return true;
}

bool Zone::IsSpecialBindLocation(const glm::vec4& location)
{
	glm::vec2 corner1;
	glm::vec2 corner2;
	switch (GetZoneID()) {
		case Zones::NORTHKARANA:
			corner1 = glm::vec2(-234, -741);
			corner2 = glm::vec2(-127, -525);
			break;
		case Zones::OASIS:
			corner1 = glm::vec2(90,  656);
			corner2 = glm::vec2(-58, 471);
			break;
		case Zones::FIELDOFBONE:
			corner1 = glm::vec2(265,  -2213);
			corner2 = glm::vec2(-506, -1255);
			break;
		case Zones::FIRIONA:
			corner1 = glm::vec2(1065, -2609);
			corner2 = glm::vec2(3511, -4534);
			break;
		case Zones::FRONTIERMTNS:
			corner1 = glm::vec2(1554, -2106);
			corner2 = glm::vec2(1206, -2333);
			break;
		case Zones::OVERTHERE:
			corner1 = glm::vec2(3937, 3614);
			corner2 = glm::vec2(2034, 2324);
			break;
		case Zones::ICECLAD:
			corner1 = glm::vec2(3937, 3614);
			corner2 = glm::vec2(510,  5365);
			break;
		default:
			return false;
	}
	return IsWithinAxisAlignedBox(glm::vec2(location.x, location.y), corner1, corner2);
}

//this also just loads into entity_list, not really into zone
bool Zone::LoadGroundSpawns() {
	GroundSpawns g{};

	content_db.LoadGroundSpawns(zoneid, GetInstanceVersion(), &g);

	uint32 added = 0;

	for (uint16 slot_id = 0; slot_id < 50; slot_id++) {
		if (EQ::ValueWithin(g.spawn[slot_id].item_id, 1, (SAYLINK_ITEM_ID - 1))) {
			auto inst = database.CreateItem(g.spawn[slot_id].item_id);

			const uint32 max_allowed = g.spawn[slot_id].max_allowed;

			if (inst) {
				for (uint32 i = 0; i < max_allowed; i++) {
					auto object = new Object(
						inst,
						g.spawn[slot_id].name,
						g.spawn[slot_id].max_x,
						g.spawn[slot_id].min_x,
						g.spawn[slot_id].max_y,
						g.spawn[slot_id].min_y,
						g.spawn[slot_id].max_z,
						g.spawn[slot_id].heading,
						g.spawn[slot_id].respawn_timer,
						g.spawn[slot_id].fix_z
					);

					entity_list.AddObject(object, false);

					added++;
				}

				safe_delete(inst);
			}
		}
	}

	LogInfo(
		"Loaded [{}] Ground Spawn{}",
		Strings::Commify(added),
		added != 1 ? "s" : ""
	);

	return(true);
}

int Zone::SaveTempItem(uint32 merchantid, uint32 npcid, uint32 item, int32 charges, bool sold) {

	LogInventory("[{}] [{}] charges of [{}]", ((sold) ? "Sold" : "Bought"),
		charges, item);
	// Iterate past main items.
	// If the item being transacted is in this list, return 0;
	std::list<MerchantList> merlist = merchanttable[merchantid];
	std::list<MerchantList>::const_iterator itr;
	uint32 temp_slot_index = 1;
	for (itr = merlist.begin(); itr != merlist.end(); ++itr) {
		MerchantList ml = *itr;
		if (ml.item == item) {
			return 0;
		}

		// Account for merchant lists with gaps in them.
		if (ml.slot >= temp_slot_index) {
			temp_slot_index = ml.slot + 1;
		}
	}

	LogInventory("Searching Temporary List.  Main list ended at [{}]", temp_slot_index-1);

	// Now search the temporary list.
	std::list<TempMerchantList> tmp_merlist = tmpmerchanttable[npcid];
	std::list<TempMerchantList>::const_iterator tmp_itr;
	TempMerchantList ml;
	uint32 first_empty_slot = 0; // Save 1st vacant slot while searching..
	bool found = false;

	for (tmp_itr = tmp_merlist.begin(); tmp_itr != tmp_merlist.end(); ++tmp_itr) {
		ml = *tmp_itr;

		if (ml.item == item) {
			found = true;
			LogInventory("Item found in temp list at [{}] with [{}] charges", ml.origslot, ml.charges);
			break;
		}
	}

	if (found) {
		tmp_merlist.clear();
		std::list<TempMerchantList> oldtmp_merlist = tmpmerchanttable[npcid];
		for (tmp_itr = oldtmp_merlist.begin(); tmp_itr != oldtmp_merlist.end(); ++tmp_itr) {
			TempMerchantList ml2 = *tmp_itr;
			if(ml2.item != item)
				tmp_merlist.push_back(ml2);
			else {
				if (sold) {
					LogInventory("Total charges is [{}] + [{}] charges", ml.charges, charges);
					ml.charges = ml.charges + charges;
				}
				else {
					ml.charges = charges;
					LogInventory("new charges is [{}] charges", ml.charges);
				}

				if (!ml.origslot) {
					ml.origslot = ml.slot;
				}
				if (ml.charges > 0) {
					database.SaveMerchantTemp(npcid, ml.origslot, GetZoneID(), GetInstanceID(), item, ml.charges);
					tmp_merlist.push_back(ml);
				} else {
					database.DeleteMerchantTemp(npcid, ml.origslot, GetZoneID(), GetInstanceID());
				}
			}
		}

		tmpmerchanttable[npcid] = tmp_merlist;
		return ml.slot;
	}
	else {
		if (charges < 0) { //sanity check only, shouldnt happen
			charges = 0x7FFF;
		}

		// Find an ununsed db slot #
		std::list<int> slots;
		TempMerchantList ml3;
		for (tmp_itr = tmp_merlist.begin(); tmp_itr != tmp_merlist.end(); ++tmp_itr) {
			ml3 = *tmp_itr;
			slots.push_back(ml3.origslot);
		}
		slots.sort();
		std::list<int>::const_iterator slots_itr;
		uint32 first_empty_slot = 0;
		uint32 idx = temp_slot_index;
		for (slots_itr = slots.begin(); slots_itr != slots.end(); ++slots_itr) {
			if (!first_empty_slot && *slots_itr > idx) {
				LogInventory("Popped [{}]", *slots_itr);
				LogInventory("First Gap Found at [{}]", idx);
				break;
			}

			++idx;
		}

		first_empty_slot = idx;

		// Find an ununsed mslot
		slots.clear();
		for (tmp_itr = tmp_merlist.begin(); tmp_itr != tmp_merlist.end(); ++tmp_itr) {
			ml3 = *tmp_itr;
			slots.push_back(ml3.slot);
		}
		slots.sort();
		uint32 first_empty_mslot=0;
		idx = temp_slot_index;
		for (slots_itr = slots.begin(); slots_itr != slots.end(); ++slots_itr) {
			if (!first_empty_mslot && *slots_itr > idx) {
				LogInventory("Popped [{}]", *slots_itr);
				LogInventory("First Gap Found at [{}]", idx);
				break;
			}

			++idx;
		}

		first_empty_mslot = idx;

		database.SaveMerchantTemp(npcid, first_empty_slot, GetZoneID(), GetInstanceID(), item, charges);
		tmp_merlist = tmpmerchanttable[npcid];
		TempMerchantList ml2;
		ml2.charges = charges;
		LogInventory("Adding slot [{}] with [{}] charges.", first_empty_mslot, charges);
		ml2.item = item;
		ml2.npcid = npcid;
		ml2.slot = first_empty_mslot;
		ml2.origslot = first_empty_slot;
		tmp_merlist.push_back(ml2);
		tmpmerchanttable[npcid] = tmp_merlist;
		return ml2.slot;
	}
}

uint32 Zone::GetTempMerchantQuantity(uint32 NPCID, uint32 Slot) {

	std::list<TempMerchantList> TmpMerchantList = tmpmerchanttable[NPCID];
	std::list<TempMerchantList>::const_iterator Iterator;

	for (Iterator = TmpMerchantList.begin(); Iterator != TmpMerchantList.end(); ++Iterator)
		if ((*Iterator).slot == Slot) {
			LogInventory("Slot [{}] has [{}] charges.", Slot, (*Iterator).charges);
			return (*Iterator).charges;
		}

	return 0;
}

void Zone::LoadTempMerchantData()
{
	auto results = content_db.QueryDatabase(
		fmt::format(
			SQL(
				SELECT
				DISTINCT npc_types.id
				FROM
					npc_types
				JOIN spawnentry ON spawnentry.npcID = npc_types.id
				JOIN spawn2 ON spawn2.spawngroupID = spawnentry.spawngroupID
				WHERE
				spawn2.zone = '{}'
				AND spawn2.version = {}
			),
			GetShortName(),
			GetInstanceVersion()
		)
	);

    if (!results.Success() || results.RowCount() == 0) {
        return;
	}

	std::vector<std::string> npc_ids;
	for (auto row = results.begin(); row != results.end(); ++row) {
		npc_ids.push_back(row[0]);
	}

	results = database.QueryDatabase(
		fmt::format(
			SQL(
				SELECT
				npcid,
				slot,
				charges,
				itemid
				FROM merchantlist_temp
				WHERE npcid IN ({})
				AND zone_id = {}
				AND instance_id = {}
			),
			Strings::Implode(", ", npc_ids),
			GetZoneID(), GetInstanceID()
		)
	);

	LogInfo("Loaded [{}] temporary merchant entries", Strings::Commify(results.RowCount()));

	std::map<uint32, std::list<TempMerchantList> >::iterator temp_merchant_table_entry;

	uint32 npc_id = 0;
	for (auto row = results.begin(); row != results.end(); ++row) {
		TempMerchantList temp_merchant_list;
		temp_merchant_list.npcid = Strings::ToUnsignedInt(row[0]);
		if (npc_id != temp_merchant_list.npcid) {
			temp_merchant_table_entry = tmpmerchanttable.find(temp_merchant_list.npcid);
			if (temp_merchant_table_entry == tmpmerchanttable.end()) {
				std::list<TempMerchantList> empty;
				tmpmerchanttable[temp_merchant_list.npcid] = empty;
				temp_merchant_table_entry = tmpmerchanttable.find(temp_merchant_list.npcid);
			}
			npc_id = temp_merchant_list.npcid;
		}

		temp_merchant_list.slot     = Strings::ToUnsignedInt(row[1]);
		temp_merchant_list.charges  = Strings::ToUnsignedInt(row[2]);
		temp_merchant_list.item     = Strings::ToUnsignedInt(row[3]);
		temp_merchant_list.origslot = temp_merchant_list.slot;

		LogMerchants(
			"Loading merchant temp items npc_id [{}] slot [{}] charges [{}] item [{}] origslot [{}]",
			npc_id,
			temp_merchant_list.slot,
			temp_merchant_list.charges,
			temp_merchant_list.item,
			temp_merchant_list.origslot
		);

		temp_merchant_table_entry->second.push_back(temp_merchant_list);
	}
}

void Zone::LoadNewMerchantData(uint32 merchantid) {

	std::list<MerchantList> merchant_list;

	const auto& l = MerchantlistRepository::GetWhere(
		content_db,
		fmt::format(
			"merchantid = {} {} ORDER BY slot",
			merchantid,
			ContentFilterCriteria::apply()
		)
	);

	if (l.empty()) {
		return;
	}

	for (const auto& e : l) {
		MerchantList ml;
		ml.id                = merchantid;
		ml.item              = e.item;
		ml.slot              = e.slot;
		ml.faction_required  = e.faction_required;
		ml.level_required    = e.level_required;
		ml.min_status        = e.min_status;
		ml.max_status        = e.max_status;
		ml.alt_currency_cost = e.alt_currency_cost;
		ml.classes_required  = e.classes_required;
		ml.probability       = e.probability;
		ml.bucket_name       = e.bucket_name;
		ml.bucket_value      = e.bucket_value;
		ml.bucket_comparison = e.bucket_comparison;
		merchant_list.push_back(ml);
	}

	merchanttable[merchantid] = merchant_list;
}

void Zone::LoadMerchants()
{
	const auto& l = MerchantlistRepository::GetWhere(
		content_db,
		fmt::format(
			SQL(
				`merchantid` IN (
					SELECT `merchant_id` FROM `npc_types` WHERE `id` IN (
						SELECT `npcID` FROM `spawnentry` WHERE `spawngroupID` IN (
							SELECT `spawngroupID` FROM `spawn2` WHERE `zone` = '{}' AND (`version` = {} OR `version` = -1)
						)
					)
				)
				{}
				ORDER BY `merchantlist`.`slot`
			),
			GetShortName(),
			GetInstanceVersion(),
			ContentFilterCriteria::apply()
		)
	);

	LogInfo("Loaded [{}] merchant lists", Strings::Commify(l.size()));

	if (l.empty()) {
		LogDebug("No Merchant Data found for [{}]", GetShortName());
		return;
	}

	std::map<uint32, std::list<MerchantList>>::iterator ml;
	uint32 npc_id = 0;

	for (const auto& e : l) {
		if (npc_id != e.merchantid) {
			ml = merchanttable.find(e.merchantid);
			if (ml == merchanttable.end()) {
				std::list<MerchantList> empty;
				merchanttable[e.merchantid] = empty;
				ml = merchanttable.find(e.merchantid);
			}

			npc_id = e.merchantid;
		}

		bool found = false;
		for (const auto &m : ml->second) {
			if (m.item == e.merchantid) {
				found = true;
				break;
			}
		}

		if (found) {
			continue;
		}

		ml->second.push_back(
			MerchantList{
				.id = static_cast<uint32>(e.merchantid),
				.slot = e.slot,
				.item = static_cast<uint32>(e.item),
				.faction_required = e.faction_required,
				.level_required = static_cast<int8>(e.level_required),
				.min_status = e.min_status,
				.max_status = e.max_status,
				.alt_currency_cost = e.alt_currency_cost,
				.classes_required = static_cast<uint32>(e.classes_required),
				.probability = static_cast<uint8>(e.probability),
				.bucket_name = e.bucket_name,
				.bucket_value = e.bucket_value,
				.bucket_comparison = e.bucket_comparison
			}
		);
	}
}

void Zone::LoadMercenaryTemplates()
{
	std::list<MercStanceInfo> mercenary_stances;

	merc_templates.clear();

	const auto& l = MercStanceEntriesRepository::GetAllOrdered(database);
	if (l.empty()) {
		return;
	}

	for (const auto& e : l) {
		MercStanceInfo t{
			.ProficiencyID = e.proficiency_id,
			.ClassID = static_cast<uint8>(e.class_id),
			.StanceID = e.stance_id,
			.IsDefault = static_cast<uint8>(e.isdefault)
		};

		mercenary_stances.push_back(t);
	}

	const std::string& query = SQL(
		SELECT DISTINCT MTem.merc_template_id, MTyp.dbstring
		AS merc_type_id, MTem.dbstring
		AS merc_subtype_id, MTyp.race_id, MS.class_id, MTyp.proficiency_id, MS.tier_id, 0
		AS CostFormula, MTem.clientversion, MTem.merc_npc_type_id
		FROM merc_types MTyp, merc_templates MTem, merc_subtypes MS
		WHERE MTem.merc_type_id = MTyp.merc_type_id AND MTem.merc_subtype_id = MS.merc_subtype_id
		ORDER BY MTyp.race_id, MS.class_id, MTyp.proficiency_id
	);
	auto results = database.QueryDatabase(query);
	if (!results.Success() || !results.RowCount()) {
		return;
	}

	for (auto row: results) {
		MercTemplate t{
			.MercTemplateID = Strings::ToUnsignedInt(row[0]),
			.MercType = Strings::ToUnsignedInt(row[1]),
			.MercSubType = Strings::ToUnsignedInt(row[2]),
			.RaceID = static_cast<uint16>(Strings::ToUnsignedInt(row[3])),
			.ClassID = static_cast<uint8>(Strings::ToUnsignedInt(row[4])),
			.MercNPCID = Strings::ToUnsignedInt(row[9]),
			.ProficiencyID = static_cast<uint8>(Strings::ToUnsignedInt(row[5])),
			.TierID = static_cast<uint8>(Strings::ToUnsignedInt(row[6])),
			.CostFormula = static_cast<uint8>(Strings::ToUnsignedInt(row[7])),
			.ClientVersion = Strings::ToUnsignedInt(row[8])
		};

		for (int i = 0; i < MaxMercStanceID; i++) {
			t.Stances[i] = 0;
		}

		int stance_index = 0;

		for (auto i = mercenary_stances.begin(); i != mercenary_stances.end(); ++i) {
			if (i->ClassID != t.ClassID || i->ProficiencyID != t.ProficiencyID) {
				continue;
			}

			zone->merc_stance_list[t.MercTemplateID].push_back((*i));
			t.Stances[stance_index] = i->StanceID;
			++stance_index;
		}

		merc_templates[t.MercTemplateID] = t;
	}
}

void Zone::LoadLevelEXPMods()
{
	level_exp_mod.clear();

	const auto& l = LevelExpModsRepository::All(database);

	for (const auto& e : l) {
		level_exp_mod[e.level].ExpMod   = e.exp_mod;
		level_exp_mod[e.level].AAExpMod = e.aa_exp_mod;
	}
}

void Zone::LoadMercenarySpells()
{
	merc_spells_list.clear();

	const std::string& query = SQL(
		SELECT msl.class_id, msl.proficiency_id, msle.spell_id, msle.spell_type,
		msle.stance_id, msle.minlevel, msle.maxlevel, msle.slot, msle.procChance
		FROM merc_spell_lists msl, merc_spell_list_entries msle
		WHERE msle.merc_spell_list_id = msl.merc_spell_list_id
		ORDER BY msl.class_id, msl.proficiency_id, msle.spell_type, msle.minlevel, msle.slot
	);

	auto results = database.QueryDatabase(query);
	if (!results.Success() || !results.RowCount()) {
		return;
	}

	for (auto row: results) {
		const uint32 class_id = Strings::ToUnsignedInt(row[0]);

		merc_spells_list[class_id].push_back(
			MercSpellEntry{
				.proficiencyid = static_cast<uint8>(Strings::ToUnsignedInt(row[1])),
				.spellid = static_cast<uint16>(Strings::ToUnsignedInt(row[2])),
				.type = Strings::ToUnsignedInt(row[3]),
				.stance = static_cast<int16>(Strings::ToInt(row[4])),
				.minlevel = static_cast<uint8>(Strings::ToUnsignedInt(row[5])),
				.maxlevel = static_cast<uint8>(Strings::ToUnsignedInt(row[6])),
				.slot = static_cast<int16>(Strings::ToInt(row[7])),
				.proc_chance = static_cast<uint16>(Strings::ToUnsignedInt(row[8]))
			}
		);
	}
}

bool Zone::IsLoaded() {
	return is_zone_loaded;
}

void Zone::Shutdown(bool quiet)
{
	if (!is_zone_loaded) {
		return;
	}

	DataBucket::DeleteCachedBuckets(DataBucketLoadType::Zone, zone->GetZoneID(), zone->GetInstanceID());
	// save and kick all clients
	for (auto c : entity_list.GetClientList()) {
		c.second->Save();
		c.second->WorldKick();
	}

	if (m_save_zone_state && RuleB(Zone, StateSavingOnShutdown) && zone && zone->IsLoaded()) {
		SaveZoneState();
	}

	entity_list.StopMobAI();

	std::map<uint32, NPCType *>::iterator itr;
	while (!npctable.empty()) {
		itr = npctable.begin();
		delete itr->second;
		itr->second = nullptr;
		npctable.erase(itr);
	}

	while (!merctable.empty()) {
		itr = merctable.begin();
		delete itr->second;
		itr->second = nullptr;
		merctable.erase(itr);
	}

	adventure_entry_list_flavor.clear();

	std::map<uint32, LDoNTrapTemplate *>::iterator itr4;
	while (!ldon_trap_list.empty()) {
		itr4 = ldon_trap_list.begin();
		delete itr4->second;
		itr4->second = nullptr;
		ldon_trap_list.erase(itr4);
	}
	ldon_trap_entry_list.clear();

	LogInfo(
		"Zone [{}] zone_id [{}] version [{}] instance_id [{}]",
		GetShortName(),
		GetZoneID(),
		GetInstanceVersion(),
		GetInstanceID()
	);
	PetitionList::Instance()->ClearPetitions();
	SetZoneHasCurrentTime(false);
	if (!quiet) {
		LogInfo(
			"Zone [{}] zone_id [{}] version [{}] instance_id [{}] Going to sleep",
			GetShortName(),
			GetZoneID(),
			GetInstanceVersion(),
			GetInstanceID()
		);
	}

	is_zone_loaded = false;

	ResetAuth();
	safe_delete(zone);
	entity_list.ClearAreas();
	parse->ReloadQuests(true);
	UpdateWindowTitle(nullptr);

	EQEmuLogSys::Instance()->CloseFileLogs();

	if (RuleB(Zone, KillProcessOnDynamicShutdown)) {
		LogInfo("Shutting down");
		EQ::EventLoop::Get().Shutdown();
	}
}

void Zone::LoadZoneDoors()
{
	auto door_entries = content_db.LoadDoors(GetShortName(), GetInstanceVersion());
	if (door_entries.empty()) {
		LogInfo("No doors loaded");
		return;
	}

	for (const auto &entry : door_entries) {
		auto d = new Doors(entry);
		entity_list.AddDoor(d);
		LogDoorsDetail("Door added to entity list, db id: [{}], door_id: [{}]", entry.id, entry.doorid);
	}
}

Zone::Zone(uint32 in_zoneid, uint32 in_instanceid, const char* in_short_name)
: initgrids_timer(10000),
  autoshutdown_timer((RuleI(Zone, AutoShutdownDelay))),
  clientauth_timer(AUTHENTICATION_TIMEOUT * 1000),
  spawn2_timer(1000),
  hot_reload_timer(1000),
  qglobal_purge_timer(30000),
  m_safe_points(0.0f, 0.0f, 0.0f, 0.0f),
  m_graveyard(0.0f, 0.0f, 0.0f, 0.0f)
{
	zoneid = in_zoneid;
	instanceid = in_instanceid;
	instanceversion = database.GetInstanceVersion(instanceid);
	pers_instance = false;
	zonemap = nullptr;
	watermap = nullptr;
	pathing = nullptr;
	qGlobals = nullptr;
	default_ruleset = 0;

	is_zone_time_localized = false;
	quest_idle_override = false;

	loglevelvar = 0;
	merchantvar = 0;
	tradevar = 0;
	lootvar = 0;

	short_name = strcpy(new char[strlen(in_short_name)+1], in_short_name);
	strlwr(short_name);
	memset(file_name, 0, sizeof(file_name));

	long_name         = 0;
	aggroedmobs       = 0;
	m_graveyard_id    = 0;
	pgraveyard_zoneid = 0;
	m_max_clients     = 0;
	pvpzone           = false;

	SetIdleWhenEmpty(true);
	SetSecondsBeforeIdle(60);
	SetSaveZoneState(false);

	if (database.GetServerType() == 1) {
		pvpzone = true;
	}

	auto z = GetZoneVersionWithFallback(ZoneID(short_name), instanceversion);
	if (z) {
		long_name = strcpy(new char[strlen(z->long_name.c_str()) + 1], z->long_name.c_str());

		m_safe_points.x = z->safe_x;
		m_safe_points.y = z->safe_y;
		m_safe_points.z = z->safe_z;
		m_safe_points.w = z->safe_heading;
		m_graveyard_id = z->graveyard_id;
		m_max_clients  = z->maxclients;

		SetIdleWhenEmpty(z->idle_when_empty);
		SetSecondsBeforeIdle(z->seconds_before_idle);

		if (z->file_name.empty()) {
			strcpy(file_name, short_name);
		}
		else {
			strcpy(file_name, z->file_name.c_str());
		}
	}

	if (graveyard_id() > 0) {
		LogDebug("Graveyard ID is [{}]", graveyard_id());
		const auto& e = GraveyardRepository::FindOne(content_db, graveyard_id());

		if (e.id) {
			pgraveyard_zoneid = e.zone_id;

			m_graveyard.x = e.x;
			m_graveyard.y = e.y;
			m_graveyard.z = e.z;
			m_graveyard.w = e.heading;

			LogDebug("Loaded a graveyard for zone [{}]: graveyard zoneid is [{}] at [{}]", short_name, graveyard_zoneid(), to_string(m_graveyard).c_str());
		}
		else {
			LogError("Unable to load the graveyard id [{}] for zone [{}]", graveyard_id(), short_name);
		}
	}
	if (long_name == 0) {
		long_name = strcpy(new char[18], "Long zone missing");
	}
	Weather_Timer = new Timer(60000);
	Weather_Timer->Start();
	LogDebug("The next weather check for zone: [{}] will be in [{}] seconds", short_name, Weather_Timer->GetRemainingTime()/1000);
	zone_weather              = EQ::constants::WeatherTypes::None;
	weather_intensity         = 0;
	blocked_spells            = nullptr;
	zone_total_blocked_spells = 0;
	zone_has_current_time     = false;

	Instance_Shutdown_Timer = nullptr;
	bool is_perma = false;
	if(instanceid > 0)
	{
		uint32 rem = database.GetTimeRemainingInstance(instanceid, is_perma);

		if(!is_perma)
		{
			if(rem < 150) //give some leeway to people who are zoning in 2.5 minutes to finish zoning in and get ported out
				rem = 150;
			Instance_Timer = new Timer(rem * 1000);
		}
		else
		{
			pers_instance = true;
			Instance_Timer = nullptr;
		}
	}
	else
	{
		Instance_Timer = nullptr;
	}
	adv_data = nullptr;
	map_name = nullptr;
	Instance_Warning_timer = nullptr;
	did_adventure_actions = false;
	database.QGlobalPurge();

	if(zoneid == Zones::GUILDHALL)
		GuildBanks = new GuildBankManager;
	else
		GuildBanks = nullptr;

	m_ucss_available = false;
	m_last_ucss_update = 0;

	mMovementManager = &MobMovementManager::Get();

	SetQuestHotReloadQueued(false);
}

Zone::~Zone() {
	LogInfo("Zone destructor called for zone [{}]", short_name);

	spawn2_list.Clear();
	if (worldserver.Connected()) {
		worldserver.SetZoneData(0);
	}

	for (auto &e: npc_emote_list) {
		safe_delete(e);
	}
	npc_emote_list.clear();

	zone_point_list.Clear();
	entity_list.Clear();
	parse->ReloadQuests();
	ClearBlockedSpells();

	safe_delete_array(short_name);
	safe_delete_array(long_name);
	safe_delete(Weather_Timer);
	safe_delete(zonemap);
	safe_delete(watermap);
	safe_delete(pathing);
	safe_delete(Instance_Timer);
	safe_delete(Instance_Shutdown_Timer);
	safe_delete(Instance_Warning_timer);
	safe_delete(qGlobals);
	safe_delete_array(adv_data);
	safe_delete_array(map_name);
	safe_delete(GuildBanks);
}

//Modified for timezones.
bool Zone::Init(bool is_static) {
	SetStaticZone(is_static);

	//load the zone config file.
	if (!LoadZoneCFG(GetShortName(), GetInstanceVersion())) { // try loading the zone name...
		LoadZoneCFG(
			GetFileName(),
			GetInstanceVersion()
		);
	} // if that fails, try the file name, then load defaults

	if (RuleManager::Instance()->GetActiveRulesetID() != default_ruleset) {
		std::string r_name = RuleSetsRepository::GetRuleSetName(database, default_ruleset);
		if (r_name.size() > 0) {
			RuleManager::Instance()->LoadRules(&database, r_name, false);
		}
	}

	if (!map_name) {
		LogError("No map name found for zone [{}]", GetShortName());
		return false;
	}

	zonemap  = Map::LoadMapFile(map_name);
	watermap = WaterMap::LoadWaterMapfile(map_name);
	pathing  = IPathfinder::Load(map_name);

	LogInfo("Loading timezone data");
	zone_time.setEQTimeZone(content_db.GetZoneTimezone(zoneid, GetInstanceVersion()));

	LoadLDoNTraps();
	LoadLDoNTrapEntries();

	LoadDynamicZoneTemplates();
	DynamicZone::CacheAllFromDatabase();

	content_db.LoadGlobalLoot();

	npc_scale_manager->LoadScaleData();

	LoadGrids();

	if (RuleB(Zone, LevelBasedEXPMods)) {
		LoadLevelEXPMods();
	}

	RespawnTimesRepository::ClearExpiredRespawnTimers(database);

	// Loading zone variables so they're available for things like encounter_load
	if (RuleB(Zone, StateSavingOnShutdown)) {
		zone->LoadZoneVariablesState();
	}

	// make sure that anything that needs to be loaded prior to scripts is loaded before here
	// this is to ensure that the scripts have access to the data they need
	parse->ReloadQuests(true);

	spawn_conditions.LoadSpawnConditions(short_name, instanceid);

	content_db.LoadStaticZonePoints(&zone_point_list, short_name, GetInstanceVersion());

	if (!content_db.LoadSpawnGroups(short_name, GetInstanceVersion(), &spawn_group_list)) {
		LogError("Loading spawn groups failed");
		return false;
	}

	content_db.PopulateZoneSpawnList(zoneid, spawn2_list, GetInstanceVersion());
	SetSaveZoneState(true);

	database.LoadCharacterCorpses(zoneid, instanceid);

	content_db.LoadTraps(short_name, GetInstanceVersion());

	LogInfo("Loading adventure flavor text");
	LoadAdventureFlavor();
	LoadGroundSpawns();
	LoadZoneObjects();
	LoadZoneDoors();
	LoadZoneBlockedSpells();
	LoadVeteranRewards();
	LoadAlternateCurrencies();
	LoadNPCEmotes(&npc_emote_list);
	LoadAlternateAdvancement();
	LoadBaseData();
	LoadMerchants();
	LoadTempMerchantData();

	// Merc data
	if (RuleB(Mercs, AllowMercs)) {
		LoadMercenaryTemplates();
		LoadMercenarySpells();
	}

	PetitionList::Instance()->ClearPetitions();
	PetitionList::Instance()->ReadDatabase();

	guild_mgr.LoadGuilds();

	LogInfo("Zone booted successfully zone_id [{}] time_offset [{}]", zoneid, zone_time.getEQTimeZone());

	// logging origination information
	EQEmuLogSys::Instance()->origination_info.zone_short_name = zone->short_name;
	EQEmuLogSys::Instance()->origination_info.zone_long_name  = zone->long_name;
	EQEmuLogSys::Instance()->origination_info.instance_id     = zone->instanceid;

	return true;
}

void Zone::ReloadStaticData() {
	LogInfo("Reloading Zone Static Data");
	entity_list.RemoveAllObjects(); //Ground spawns are also objects we clear list then fill it
	entity_list.RemoveAllDoors(); //Some objects are also doors so clear list before filling

	if (!content_db.LoadStaticZonePoints(&zone_point_list, GetShortName(), GetInstanceVersion())) {
		LogError("Loading static zone points failed");
	}

	LogInfo("Reloading traps");
	entity_list.RemoveAllTraps();
	if (!content_db.LoadTraps(GetShortName(), GetInstanceVersion()))
	{
		LogError("Reloading traps failed");
	}

	LogInfo("Reloading ground spawns");
	if (!LoadGroundSpawns())
	{
		LogError("Reloading ground spawns failed. continuing");
	}

	LogInfo("Reloading World Objects from DB");
	if (!LoadZoneObjects())
	{
		LogError("Reloading World Objects failed. continuing");
	}

	LoadZoneDoors();
	entity_list.RespawnAllDoors();

	LoadVeteranRewards();
	LoadAlternateCurrencies();
	LoadNPCEmotes(&npc_emote_list);

	//load the zone config file.
	if (!LoadZoneCFG(GetShortName(), GetInstanceVersion())) { // try loading the zone name...
		LoadZoneCFG(
			GetFileName(),
			GetInstanceVersion()
		);
	} // if that fails, try the file name, then load defaults

	WorldContentService::Instance()->SetExpansionContext()->ReloadContentFlags();


	LogInfo("Zone Static Data Reloaded");
}

bool Zone::LoadZoneCFG(const char* filename, uint16 instance_version)
{
	auto z = ZoneStore::Instance()->GetZoneWithFallback(ZoneID(filename), instance_version);

	if (!z) {
		LogError("Failed to load zone data for [{}] instance_version [{}]", filename, instance_version);
		return false;
	}

	memset(&newzone_data, 0, sizeof(NewZone_Struct));
	map_name = nullptr;
	map_name = new char[100];
	newzone_data.zone_id = zoneid;

	strcpy(map_name, "default");

	newzone_data.ztype = z->ztype;
	zone_type = newzone_data.ztype;

	// fog:red
	newzone_data.fog_red[0] = z->fog_red;
	newzone_data.fog_red[1] = z->fog_red2;
	newzone_data.fog_red[2] = z->fog_red3;
	newzone_data.fog_red[3] = z->fog_red4;

	// fog:blue
	newzone_data.fog_blue[0] = z->fog_blue;
	newzone_data.fog_blue[1] = z->fog_blue2;
	newzone_data.fog_blue[2] = z->fog_blue3;
	newzone_data.fog_blue[3] = z->fog_blue4;

	// fog:green
	newzone_data.fog_green[0] = z->fog_green;
	newzone_data.fog_green[1] = z->fog_green2;
	newzone_data.fog_green[2] = z->fog_green3;
	newzone_data.fog_green[3] = z->fog_green4;

	// fog:minclip
	newzone_data.fog_minclip[0] = z->fog_minclip;
	newzone_data.fog_minclip[1] = z->fog_minclip2;
	newzone_data.fog_minclip[2] = z->fog_minclip3;
	newzone_data.fog_minclip[3] = z->fog_minclip4;

	// fog:maxclip
	newzone_data.fog_maxclip[0] = z->fog_maxclip;
	newzone_data.fog_maxclip[1] = z->fog_maxclip2;
	newzone_data.fog_maxclip[2] = z->fog_maxclip3;
	newzone_data.fog_maxclip[3] = z->fog_maxclip4;

	// rain_chance
	newzone_data.rain_chance[0] = z->rain_chance1;
	newzone_data.rain_chance[1] = z->rain_chance2;
	newzone_data.rain_chance[2] = z->rain_chance3;
	newzone_data.rain_chance[3] = z->rain_chance4;

	// rain_duration
	newzone_data.rain_duration[0] = z->rain_duration1;
	newzone_data.rain_duration[1] = z->rain_duration2;
	newzone_data.rain_duration[2] = z->rain_duration3;
	newzone_data.rain_duration[3] = z->rain_duration4;

	// snow_chance
	newzone_data.snow_chance[0] = z->snow_chance1;
	newzone_data.snow_chance[1] = z->snow_chance2;
	newzone_data.snow_chance[2] = z->snow_chance3;
	newzone_data.snow_chance[3] = z->snow_chance4;

	// snow_duration
	newzone_data.snow_duration[0] = z->snow_duration1;
	newzone_data.snow_duration[1] = z->snow_duration2;
	newzone_data.snow_duration[2] = z->snow_duration3;
	newzone_data.snow_duration[3] = z->snow_duration4;

	// misc
	newzone_data.fog_density               = z->fog_density;
	newzone_data.sky                       = z->sky;
	newzone_data.zone_exp_multiplier       = z->zone_exp_multiplier;
	newzone_data.safe_x                    = z->safe_x;
	newzone_data.safe_y                    = z->safe_y;
	newzone_data.safe_z                    = z->safe_z;
	newzone_data.safe_heading              = z->safe_heading;
	newzone_data.underworld                = z->underworld;
	newzone_data.minclip                   = z->minclip;
	newzone_data.maxclip                   = z->maxclip;
	newzone_data.time_type                 = z->time_type;
	newzone_data.gravity                   = z->gravity;
	newzone_data.fast_regen_hp             = z->fast_regen_hp > 0 ? z->fast_regen_hp : 180;
	newzone_data.fast_regen_mana           = z->fast_regen_mana > 0 ? z->fast_regen_mana : 180;
	newzone_data.fast_regen_endurance      = z->fast_regen_endurance > 0 ? z->fast_regen_endurance : 180;
	newzone_data.npc_aggro_max_dist        = z->npc_max_aggro_dist;
	newzone_data.underworld_teleport_index = z->underworld_teleport_index;
	newzone_data.lava_damage               = z->lava_damage;
	newzone_data.min_lava_damage           = z->min_lava_damage;
	newzone_data.suspend_buffs             = z->suspendbuffs;

	// local attributes
	can_bind              = z->canbind != 0;
	is_city               = z->canbind == 2;
	can_combat            = z->cancombat != 0;
	can_levitate          = z->canlevitate != 0;
	can_castoutdoor       = z->castoutdoor != 0;
	is_hotzone            = z->hotzone != 0;
	m_client_update_range = z->client_update_range;
	default_ruleset       = z->ruleset;
	allow_mercs           = true;
	m_graveyard_id        = z->graveyard_id;
	m_max_clients         = z->maxclients;

	SetIdleWhenEmpty(z->idle_when_empty);
	SetSecondsBeforeIdle(z->seconds_before_idle);

	// safe coordinates
	m_safe_points.x = z->safe_x;
	m_safe_points.y = z->safe_y;
	m_safe_points.z = z->safe_z;
	m_safe_points.w = z->safe_heading;

	if (!z->map_file_name.empty()) {
		strcpy(map_name, z->map_file_name.c_str());
	}
	else {
		strcpy(map_name, z->short_name.c_str());
	}

	// overwrite with our internal variables
	strcpy(newzone_data.zone_short_name, GetShortName());
	strcpy(newzone_data.zone_long_name, GetLongName());
	strcpy(newzone_data.zone_short_name2, GetShortName());

	LogInfo(
		"Successfully loaded zone headers for zone [{}] long_name [{}] version [{}] instance_id [{}]",
		GetShortName(),
		GetLongName(),
		GetInstanceVersion(),
		instance_version
	);

	return true;
}

bool Zone::SaveZoneCFG()
{
	return content_db.SaveZoneCFG(GetZoneID(), GetInstanceVersion(), &newzone_data);
}

void Zone::AddAuth(ServerZoneIncomingClient_Struct* szic) {
	auto zca = new ZoneClientAuth_Struct;
	memset(zca, 0, sizeof(ZoneClientAuth_Struct));
	zca->ip = szic->ip;
	zca->wid = szic->wid;
	zca->accid = szic->accid;
	zca->admin = szic->admin;
	zca->charid = szic->charid;
	zca->lsid = szic->lsid;
	zca->tellsoff = szic->tellsoff;
	strn0cpy(zca->charname, szic->charname, sizeof(zca->charname));
	strn0cpy(zca->lskey, szic->lskey, sizeof(zca->lskey));
	zca->stale = false;
	client_auth_list.Insert(zca);
}

void Zone::RemoveAuth(const char* iCharName, const char* iLSKey)
{
	LinkedListIterator<ZoneClientAuth_Struct*> iterator(client_auth_list);

	iterator.Reset();
	while (iterator.MoreElements()) {
		ZoneClientAuth_Struct* zca = iterator.GetData();
		if (strcasecmp(zca->charname, iCharName) == 0 && strcasecmp(zca->lskey, iLSKey) == 0) {
			iterator.RemoveCurrent();
			return;
		}
		iterator.Advance();
	}
}

void Zone::RemoveAuth(uint32 lsid)
{
	LinkedListIterator<ZoneClientAuth_Struct*> iterator(client_auth_list);

	iterator.Reset();
	while (iterator.MoreElements()) {
		ZoneClientAuth_Struct* zca = iterator.GetData();
		if (zca->lsid == lsid) {
			iterator.RemoveCurrent();
			continue;
		}
		iterator.Advance();
	}
}

void Zone::ResetAuth()
{
	LinkedListIterator<ZoneClientAuth_Struct*> iterator(client_auth_list);

	iterator.Reset();
	while (iterator.MoreElements()) {
		iterator.RemoveCurrent();
	}
}

bool Zone::GetAuth(uint32 iIP, const char* iCharName, uint32* oWID, uint32* oAccID, uint32* oCharID, int16* oStatus, char* oLSKey, bool* oTellsOff) {
	LinkedListIterator<ZoneClientAuth_Struct*> iterator(client_auth_list);

	iterator.Reset();
	while (iterator.MoreElements()) {
		ZoneClientAuth_Struct* zca = iterator.GetData();
		if (strcasecmp(zca->charname, iCharName) == 0) {
				if(oWID)
				*oWID = zca->wid;
				if(oAccID)
				*oAccID = zca->accid;
				if(oCharID)
				*oCharID = zca->charid;
				if(oStatus)
				*oStatus = zca->admin;
				if(oTellsOff)
				*oTellsOff = zca->tellsoff;
				zca->stale = true;
			return true;
		}
		iterator.Advance();
	}
	return false;
}

uint32 Zone::CountAuth() {
	LinkedListIterator<ZoneClientAuth_Struct*> iterator(client_auth_list);

	int x = 0;
	iterator.Reset();
	while (iterator.MoreElements()) {
		x++;
		iterator.Advance();
	}
	return x;
}

bool Zone::Process() {
	spawn_conditions.Process();

	if (spawn2_timer.Check()) {
		LinkedListIterator<Spawn2 *> iterator(spawn2_list);

		EQ::InventoryProfile::CleanDirty();

		iterator.Reset();
		while (iterator.MoreElements()) {
			if (iterator.GetData()->Process()) {
				iterator.Advance();
			}
			else {
				iterator.RemoveCurrent();
			}
		}

		if (adv_data && !did_adventure_actions) {
			DoAdventureActions();
		}
	}

	if (hot_reload_timer.Check() && IsQuestHotReloadQueued()) {

		LogHotReloadDetail("Hot reload timer check...");

		bool perform_reload = true;

		if (RuleB(HotReload, QuestsRepopWhenPlayersNotInCombat)) {
			for (auto &it : entity_list.GetClientList()) {
				auto client = it.second;
				if (client->GetAggroCount() > 0) {
					perform_reload = false;
					break;
				}
			}
		}

		if (perform_reload) {
			ZoneReload::HotReloadQuests();
		}
	}

	if(initgrids_timer.Check()) {
		//delayed grid loading stuff.
		initgrids_timer.Disable();
		LinkedListIterator<Spawn2*> iterator(spawn2_list);

		iterator.Reset();
		while (iterator.MoreElements()) {
			iterator.GetData()->LoadGrid();
			iterator.Advance();
		}
	}

	if (!staticzone) {
		if (autoshutdown_timer.Check()) {
			ResetShutdownTimer();
			if (numclients == 0) {
				return false;
			}
		}
	}

	if(GetInstanceID() > 0)
	{
		if(Instance_Timer != nullptr && Instance_Shutdown_Timer == nullptr)
		{
			if(Instance_Timer->Check())
			{
				auto dz = GetDynamicZone();
				if (dz)
				{
					dz->RemoveAllMembers(); // entity list will teleport clients out immediately
				}

				// instance shutting down, move corpses to graveyard or non-instanced zone at same coords
				entity_list.MovePlayerCorpsesToGraveyard(true);
				entity_list.GateAllClientsToSafeReturn();
				database.DeleteInstance(GetInstanceID());
				Instance_Shutdown_Timer = new Timer(20000); //20 seconds
			}

			if(adv_data == nullptr)
			{
				if(Instance_Warning_timer == nullptr)
				{
					uint32 rem_time = Instance_Timer->GetRemainingTime();
					uint32_t minutes_warning = 0;
					if(rem_time < 60000 && rem_time > 55000)
					{
						minutes_warning = 1;
					}
					else if(rem_time < 300000 && rem_time > 295000)
					{
						minutes_warning = 5;
					}
					else if(rem_time < 900000 && rem_time > 895000)
					{
						minutes_warning = 15;
					}

					if (minutes_warning > 0)
					{
						// expedition expire warnings are handled by world
						auto expedition = DynamicZone::FindExpeditionByZone(GetZoneID(), GetInstanceID());
						if (!expedition)
						{
							entity_list.ExpeditionWarning(minutes_warning);
							Instance_Warning_timer = new Timer(10000);
						}
					}
				}
				else if(Instance_Warning_timer->Check())
				{
					safe_delete(Instance_Warning_timer);
				}
			}
		}
		else if(Instance_Shutdown_Timer != nullptr)
		{
			if(Instance_Shutdown_Timer->Check())
			{
				StartShutdownTimer();
				return false;
			}
		}
	}

	if(Weather_Timer->Check())
	{
		Weather_Timer->Disable();
		ChangeWeather();
	}

	if(qGlobals)
	{
		if(qglobal_purge_timer.Check())
		{
			qGlobals->PurgeExpiredGlobals();
		}
	}

	if (clientauth_timer.Check()) {
		LinkedListIterator<ZoneClientAuth_Struct*> iterator2(client_auth_list);

		iterator2.Reset();
		while (iterator2.MoreElements()) {
			if (iterator2.GetData()->stale)
				iterator2.RemoveCurrent();
			else {
				iterator2.GetData()->stale = true;
				iterator2.Advance();
			}
		}
	}

	const bool has_timer_event = parse->ZoneHasQuestSub(EVENT_TIMER);

	for (auto e : zone_timers) {
		if (e.timer_.Enabled() && e.timer_.Check()) {
			if (has_timer_event) {
				parse->EventZone(EVENT_TIMER, this, e.name);
			}
		}
	}

	if (!m_zone_signals.empty()) {
		int signal_id = m_zone_signals.front();
		m_zone_signals.pop_front();

		if (parse->ZoneHasQuestSub(EVENT_SIGNAL)) {
			parse->EventZone(EVENT_SIGNAL, this, std::to_string(signal_id), 0);
		}
	}

	mMovementManager->Process();

	return true;
}

void Zone::ChangeWeather()
{
	if(!HasWeather())
	{
		Weather_Timer->Disable();
		return;
	}

	int chance = zone->random.Int(0, 3);
	auto rain_chance = zone->newzone_data.rain_chance[chance];
	auto rain_duration = zone->newzone_data.rain_duration[chance];
	auto snow_chance = zone->newzone_data.snow_chance[chance];
	auto snow_duration = zone->newzone_data.snow_duration[chance];
	uint32 weather_timer = 0;
	auto temporary_weather = static_cast<uint8>(zone->random.Int(0, 100));
	uint8 duration = 0;
	auto temporary_old_weather = zone->zone_weather;
	bool changed = false;

	if (temporary_old_weather == EQ::constants::WeatherTypes::None) {
		if (rain_chance || snow_chance) {
			auto intensity = static_cast<uint8>(zone->random.Int(1, 10));
			if (rain_chance > snow_chance || rain_chance == snow_chance) { // Rain
				if (rain_chance >= temporary_weather) {
					if (!rain_duration) {
						duration = 1;
					} else {
						duration = rain_duration * 3; //Duration is 1 EQ hour which is 3 earth minutes.
					}

					weather_timer = (duration * 60) * 1000;
					Weather_Timer->Start(weather_timer);
					zone->zone_weather = EQ::constants::WeatherTypes::Raining;
					zone->weather_intensity = intensity;
					changed = true;
				}
			} else { // Snow
				if (snow_chance >= temporary_weather) {
					if (!snow_duration) {
						duration = 1;
					} else {
						duration = snow_duration * 3; //Duration is 1 EQ hour which is 3 earth minutes.
					}

					weather_timer = (duration * 60) * 1000;
					Weather_Timer->Start(weather_timer);
					zone->zone_weather = EQ::constants::WeatherTypes::Snowing;
					zone->weather_intensity = intensity;
					changed = true;
				}
			}
		}
	} else {
		changed = true;
		//We've had weather, now taking a break
		if (temporary_old_weather == EQ::constants::WeatherTypes::Raining) {
			if (!rain_duration) {
				duration = 1;
			} else {
				duration = rain_duration * 3; //Duration is 1 EQ hour which is 3 earth minutes.
			}

			weather_timer = (duration * 60) * 1000;
			Weather_Timer->Start(weather_timer);
			zone->weather_intensity = 0;
		} else if (temporary_old_weather == EQ::constants::WeatherTypes::Snowing) {
			if (!snow_duration) {
				duration = 1;
			} else {
				duration = snow_duration * 3; //Duration is 1 EQ hour which is 3 earth minutes.
			}

			weather_timer = (duration * 60) * 1000;
			Weather_Timer->Start(weather_timer);
			zone->weather_intensity = 0;
		}
	}

	if (!changed) {
		if (!weather_timer) {
			uint32 weather_timer_rule = RuleI(Zone, WeatherTimer);
			weather_timer = weather_timer_rule * 1000;
			Weather_Timer->Start(weather_timer);
		}
		LogDebug("The next weather check for zone: [{}] will be in [{}] seconds", zone->GetShortName(), Weather_Timer->GetRemainingTime()/1000);
	} else {
		LogDebug("The weather for zone: [{}] has changed. Old weather was = [{}]. New weather is = [{}] The next check will be in [{}] seconds. Rain chance: [{}], Rain duration: [{}], Snow chance [{}], Snow duration: [{}]", zone->GetShortName(), temporary_old_weather, zone_weather,Weather_Timer->GetRemainingTime()/1000,rain_chance,rain_duration,snow_chance,snow_duration);
		weatherSend();
		if (!zone->weather_intensity) {
			zone->zone_weather = EQ::constants::WeatherTypes::None;
		}
	}
}

bool Zone::HasWeather()
{
	auto rain_chance_one = zone->newzone_data.rain_chance[0];
	auto rain_chance_two = zone->newzone_data.rain_chance[1];
	auto rain_chance_three = zone->newzone_data.rain_chance[2];
	auto rain_chance_four = zone->newzone_data.rain_chance[3];

	auto snow_chance_one = zone->newzone_data.snow_chance[0];
	auto snow_chance_two = zone->newzone_data.snow_chance[1];
	auto snow_chance_three = zone->newzone_data.snow_chance[2];
	auto snow_chance_four = zone->newzone_data.snow_chance[3];

	if (
		!rain_chance_one &&
		!rain_chance_two &&
		!rain_chance_three &&
		!rain_chance_four &&
		!snow_chance_one &&
		!snow_chance_two &&
		!snow_chance_three &&
		!snow_chance_four
	) {
		return false;
	} else {
		return true;
	}
}

void Zone::StartShutdownTimer(uint32 set_time)
{
	// if we pass in the default value, we should pull from the zone and use it is different
	std::string loaded_from = "rules";
	if (set_time == (RuleI(Zone, AutoShutdownDelay))) {
		auto delay = content_db.getZoneShutDownDelay(
			GetZoneID(),
			GetInstanceVersion()
		);
		if (delay != RuleI(Zone, AutoShutdownDelay)) {
			set_time = delay;
			loaded_from = "zone table";
		}
	}

	if (set_time != autoshutdown_timer.GetDuration()) {
		LogInfo(
			"Reset to [{}] {} from original remaining time [{}] duration [{}] zone [{}]",
			Strings::SecondsToTime(set_time, true),
			!loaded_from.empty() ? fmt::format("(Loaded from [{}])", loaded_from) : "",
			Strings::SecondsToTime(autoshutdown_timer.GetRemainingTime(), true),
			Strings::SecondsToTime(autoshutdown_timer.GetDuration(), true),
			zone->GetZoneDescription()
		);
	}

	autoshutdown_timer.SetTimer(set_time);
}

void Zone::ResetShutdownTimer() {
	LogInfo(
		"Reset to [{}] from original remaining time [{}] duration [{}] zone [{}]",
		Strings::SecondsToTime(autoshutdown_timer.GetDuration(), true),
		Strings::SecondsToTime(autoshutdown_timer.GetRemainingTime(), true),
		Strings::SecondsToTime(autoshutdown_timer.GetDuration(), true),
		zone->GetZoneDescription()
	);

	autoshutdown_timer.Start(autoshutdown_timer.GetDuration(), true);
}

void Zone::StopShutdownTimer() {
	LogInfo("Stopping zone shutdown timer");
	autoshutdown_timer.Disable();
}

bool Zone::Depop(bool StartSpawnTimer) {
	std::map<uint32,NPCType *>::iterator itr;
	entity_list.Depop(StartSpawnTimer);
	entity_list.ClearTrapPointers();
	entity_list.UpdateAllTraps(false);
	/* Refresh npctable (cache), getting current info from database. */
	while(!npctable.empty()) {
		itr = npctable.begin();
		delete itr->second;
		itr->second = nullptr;
		npctable.erase(itr);
	}

	// clear spell cache
	database.ClearNPCSpells();
	database.ClearBotSpells();

	zone->spawn_group_list.ReloadSpawnGroups();

	return true;
}

void Zone::ClearNPCTypeCache(int id) {
	if (id <= 0) {
		auto iter = npctable.begin();
		while (iter != npctable.end()) {
			delete iter->second;
			iter->second = nullptr;
			++iter;
		}
		npctable.clear();
	}
	else {
		auto iter = npctable.begin();
		while (iter != npctable.end()) {
			if (iter->first == (uint32)id) {
				delete iter->second;
				iter->second = nullptr;
				npctable.erase(iter);
				return;
			}
			++iter;
		}
	}
}

void Zone::Repop(bool is_forced)
{
	if (!Depop()) {
		return;
	}

	if (is_forced) {
		ClearSpawnTimers();
	}

	LinkedListIterator<Spawn2 *> iterator(spawn2_list);

	iterator.Reset();
	while (iterator.MoreElements()) {
		iterator.RemoveCurrent();
	}

	npc_scale_manager->LoadScaleData();

	entity_list.ClearTrapPointers();

	quest_manager.ClearAllTimers();

	StopAllTimers();

	LogInfo("Loading spawn groups");
	if (!content_db.LoadSpawnGroups(short_name, GetInstanceVersion(), &spawn_group_list)) {
		LogError("Loading spawn groups failed");
	}

	spawn_conditions.LoadSpawnConditions(short_name, instanceid);

	if (RuleB(Zone, StateSavingOnShutdown)) {
		ClearZoneState(zoneid, instanceid);
	}

	if (!content_db.PopulateZoneSpawnList(zoneid, spawn2_list, GetInstanceVersion())) {
		LogDebug("Error in Zone::Repop: database.PopulateZoneSpawnList failed");
	}

	LoadGrids();

	initgrids_timer.Start();

	entity_list.UpdateAllTraps(true, true);
}

void Zone::GetTimeSync()
{
	if (!zone_has_current_time) {
		LogInfo("Requesting world time");
		auto pack = new ServerPacket(ServerOP_GetWorldTime, 1);
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void Zone::SetDate(uint16 year, uint8 month, uint8 day, uint8 hour, uint8 minute)
{
	if (worldserver.Connected()) {
		auto pack = new ServerPacket(ServerOP_SetWorldTime, sizeof(eqTimeOfDay));
		eqTimeOfDay* eqtod = (eqTimeOfDay*)pack->pBuffer;
		eqtod->start_eqtime.minute=minute;
		eqtod->start_eqtime.hour=hour;
		eqtod->start_eqtime.day=day;
		eqtod->start_eqtime.month=month;
		eqtod->start_eqtime.year=year;
		eqtod->start_realtime=time(0);
		printf("Setting master date on world server to: %d-%d-%d %d:%d (%d)\n", year, month, day, hour, minute, (int)eqtod->start_realtime);
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void Zone::SetTime(uint8 hour, uint8 minute, bool update_world /*= true*/)
{
	if (worldserver.Connected()) {
		auto pack = new ServerPacket(ServerOP_SetWorldTime, sizeof(eqTimeOfDay));
		eqTimeOfDay* eq_time_of_day = (eqTimeOfDay*)pack->pBuffer;

		zone_time.GetCurrentEQTimeOfDay(time(0), &eq_time_of_day->start_eqtime);

		eq_time_of_day->start_eqtime.minute = minute;
		eq_time_of_day->start_eqtime.hour = hour + 1;
		eq_time_of_day->start_realtime = time(0);

		/* By Default we update worlds time, but we can optionally no update world which updates the rest of the zone servers */
		if (update_world){
			LogInfo("Setting master time on world server to: {}:{} ({})\n", hour, minute, (int)eq_time_of_day->start_realtime);
			worldserver.SendPacket(pack);

			/* Set Time Localization Flag */
			zone->is_zone_time_localized = false;
		}
		/* When we don't update world, we are localizing ourselves, we become disjointed from normal syncs and set time locally */
		else{

			LogInfo("Setting zone localized time...");

			zone->zone_time.SetCurrentEQTimeOfDay(eq_time_of_day->start_eqtime, eq_time_of_day->start_realtime);
			auto outapp = new EQApplicationPacket(OP_TimeOfDay, sizeof(TimeOfDay_Struct));
			TimeOfDay_Struct* time_of_day = (TimeOfDay_Struct*)outapp->pBuffer;
			zone->zone_time.GetCurrentEQTimeOfDay(time(0), time_of_day);
			entity_list.QueueClients(0, outapp, false);
			safe_delete(outapp);

			/* Set Time Localization Flag */
			zone->is_zone_time_localized = true;
		}

		safe_delete(pack);
	}
}

ZonePoint* Zone::GetClosestZonePoint(const glm::vec3& location, uint32 to, Client* client, float max_distance) {
	LinkedListIterator<ZonePoint*> iterator(zone_point_list);
	ZonePoint* closest_zp = nullptr;

	if (!client) {
		return closest_zp;
	}

	float closest_dist = FLT_MAX;
	float max_distance2 = max_distance * max_distance;
	iterator.Reset();
	while(iterator.MoreElements())
	{
		ZonePoint* zp = iterator.GetData();
		uint32 mask_test = client->ClientVersionBit();
		if (!(zp->client_version_mask & mask_test)) {
			iterator.Advance();
			continue;
		}

		if (zp->target_zone_id == to)
		{
            auto dist = Distance(glm::vec2(zp->x, zp->y), glm::vec2(location));
			if ((zp->x == 999999 || zp->x == -999999) && (zp->y == 999999 || zp->y == -999999))
				dist = 0;

			if (dist < closest_dist)
			{
				closest_zp = zp;
				closest_dist = dist;
			}
		}
		iterator.Advance();
	}

	// if we have a water map and it says we're in a zoneline, lets assume it's just a really big zone line
	// this shouldn't open up any exploits since those situations are detected later on
	if ((client && zone->HasWaterMap() && !zone->watermap->InZoneLine(glm::vec3(client->GetPosition()))) || (!zone->HasWaterMap() && closest_dist > 400.0f && closest_dist < max_distance2))
	{
		if (!client->cheat_manager.GetExemptStatus(Port)) {
			client->cheat_manager.CheatDetected(MQZoneUnknownDest, location);
		}

		LogInfo("WARNING: Closest zone point for zone id [{}] is [{}], you might need to update your zone_points table if you dont arrive at the right spot", to, closest_dist);
		LogInfo("<Real Zone Points>. [{}]", to_string(location).c_str());
	}

	if(closest_dist > max_distance2)
		closest_zp = nullptr;

	if(!closest_zp)
		closest_zp = GetClosestZonePointWithoutZone(location.x, location.y, location.z, client);

	return closest_zp;
}

ZonePoint* Zone::GetClosestZonePoint(const glm::vec3& location, const char* to_name, Client* client, float max_distance) {
	if(to_name == nullptr)
		return GetClosestZonePointWithoutZone(location.x, location.y, location.z, client, max_distance);
	return GetClosestZonePoint(location, ZoneID(to_name), client, max_distance);
}

ZonePoint* Zone::GetClosestZonePointWithoutZone(float x, float y, float z, Client* client, float max_distance) {
	LinkedListIterator<ZonePoint*> iterator(zone_point_list);
	ZonePoint* closest_zp = nullptr;
	float closest_dist = FLT_MAX;
	float max_distance2 = max_distance*max_distance;
	iterator.Reset();
	while(iterator.MoreElements())
	{
		ZonePoint* zp = iterator.GetData();
		uint32 mask_test = client->ClientVersionBit();

		if(!(zp->client_version_mask & mask_test))
		{
			iterator.Advance();
			continue;
		}

		float delta_x = zp->x - x;
		float delta_y = zp->y - y;
		if(zp->x == 999999 || zp->x == -999999)
			delta_x = 0;
		if(zp->y == 999999 || zp->y == -999999)
			delta_y = 0;

		float dist = delta_x*delta_x+delta_y*delta_y;///*+(zp->z-z)*(zp->z-z)*/;
		if (dist < closest_dist)
		{
			closest_zp = zp;
			closest_dist = dist;
		}
		iterator.Advance();
	}
	if(closest_dist > max_distance2)
		closest_zp = nullptr;

	return closest_zp;
}

bool ZoneDatabase::LoadStaticZonePoints(LinkedList<ZonePoint *> *zone_point_list, const char *zonename, uint32 version)
{
	zone_point_list->Clear();
	zone->numzonepoints = 0;
	zone->virtual_zone_point_list.clear();

	auto zone_points = ZonePointsRepository::GetWhere(content_db,
		fmt::format(
			"zone = '{}' AND (version = {} OR version = -1) {} ORDER BY number",
			zonename,
			version,
			ContentFilterCriteria::apply()
		)
	);

	for (auto &zone_point : zone_points) {
		auto zp = new ZonePoint;

		zp->x                    = zone_point.x;
		zp->y                    = zone_point.y;
		zp->z                    = zone_point.z;
		zp->target_x             = zone_point.target_x;
		zp->target_y             = zone_point.target_y;
		zp->target_z             = zone_point.target_z;
		zp->target_zone_id       = zone_point.target_zone_id;
		zp->heading              = zone_point.heading;
		zp->target_heading       = zone_point.target_heading;
		zp->number               = zone_point.number;
		zp->target_zone_instance = zone_point.target_instance;
		zp->client_version_mask  = zone_point.client_version_mask;
		zp->is_virtual           = zone_point.is_virtual > 0;
		zp->height               = zone_point.height;
		zp->width                = zone_point.width;

		LogZonePoints(
			"Loading ZP x [{}] y [{}] z [{}] heading [{}] target x y z zone_id instance_id [{}] [{}] [{}] [{}] [{}] number [{}] is_virtual [{}] height [{}] width [{}]",
			zp->x,
			zp->y,
			zp->z,
			zp->heading,
			zp->target_x,
			zp->target_y,
			zp->target_z,
			zp->target_zone_id,
			zp->target_zone_instance,
			zp->number,
			zp->is_virtual ? "true" : "false",
			zp->height,
			zp->width
		);

		if (zone_point.is_virtual) {
			zone->virtual_zone_point_list.emplace_back(zone_point);
			safe_delete(zp);
			continue;
		}

		zone_point_list->Insert(zp);
		zone->numzonepoints++;
	}

	LogInfo("Loaded [{}] zone_points", Strings::Commify(std::to_string(zone_points.size())));

	return true;
}

bool ZoneDatabase::GetDecayTimes(npcDecayTimes_Struct *npcCorpseDecayTimes)
{
	const std::string query =
	    "SELECT varname, value FROM variables WHERE varname LIKE 'decaytime%%' ORDER BY varname";
	auto results = QueryDatabase(query);
	if (!results.Success())
		return false;

	int index = 0;
	for (auto row = results.begin(); row != results.end(); ++row, ++index) {
		Seperator sep(row[0]);
		npcCorpseDecayTimes[index].minlvl = Strings::ToInt(sep.arg[1]);
		npcCorpseDecayTimes[index].maxlvl = Strings::ToInt(sep.arg[2]);

		npcCorpseDecayTimes[index].seconds = std::min(24 * 60 * 60, Strings::ToInt(row[1]));
	}

	LogInfo("Loaded [{}] decay timers", Strings::Commify(results.RowCount()));

	return true;
}

void Zone::weatherSend(Client *client)
{
	auto outapp = new EQApplicationPacket(OP_Weather, 8);
	if (zone_weather > EQ::constants::WeatherTypes::None) {
		outapp->pBuffer[0] = zone_weather - 1;
	}

	if (zone_weather > EQ::constants::WeatherTypes::None) {
		outapp->pBuffer[4] = zone->weather_intensity;
	}

	if (client) {
		client->QueuePacket(outapp);
	} else {
		entity_list.QueueClients(0, outapp);
	}

	safe_delete(outapp);
}

bool Zone::HasGraveyard() {
	bool Result = false;

	if(graveyard_zoneid() > 0)
		Result = true;

	return Result;
}

void Zone::LoadZoneBlockedSpells()
{
	if (!blocked_spells) {
		zone_total_blocked_spells = content_db.GetBlockedSpellsCount(GetZoneID());
		if (zone_total_blocked_spells > 0) {
			blocked_spells = new ZoneSpellsBlocked[zone_total_blocked_spells];
			if (!content_db.LoadBlockedSpells(zone_total_blocked_spells, blocked_spells, GetZoneID())) {
				LogError(
					"Failed to load blocked spells for {} ({}).",
					ZoneStore::Instance()->GetZoneName(GetZoneID(), true),
					GetZoneID()
				);
				ClearBlockedSpells();
			}
		}

		LogInfo(
			"Loaded [{}] blocked spells(s) for {} ({}).",
			Strings::Commify(zone_total_blocked_spells),
			ZoneStore::Instance()->GetZoneName(GetZoneID(), true),
			GetZoneID()
		);
	}
}

void Zone::ClearBlockedSpells()
{
	safe_delete_array(blocked_spells);

	zone_total_blocked_spells = 0;
}

bool Zone::IsSpellBlocked(uint32 spell_id, const glm::vec3 &location)
{
	if (blocked_spells) {
		bool exception = false;
		bool block_all = false;

		for (int x = 0; x < GetZoneTotalBlockedSpells(); x++) {
			if (blocked_spells[x].spellid == spell_id) {
				exception = true;
			}

			if (blocked_spells[x].spellid == 0) {
				block_all = true;
			}
		}

		// If all spells are blocked and this is an exception, it is not blocked
		if (block_all && exception) {
			return false;
		}

		for (int x = 0; x < GetZoneTotalBlockedSpells(); x++) {
			// Spellid of 0 matches all spells
			if (0 != blocked_spells[x].spellid && spell_id != blocked_spells[x].spellid) {
				continue;
			}

			switch (blocked_spells[x].type) {
				case ZoneBlockedSpellTypes::ZoneWide: {
					return true;
					break;
				}
				case ZoneBlockedSpellTypes::Region: {
					if (IsWithinAxisAlignedBox(
						location,
						blocked_spells[x].m_Location - blocked_spells[x].m_Difference,
						blocked_spells[x].m_Location + blocked_spells[x].m_Difference
					)) {
						return true;
					}
					break;
				}
				default: {
					continue;
					break;
				}
			}
		}
	}

	return false;
}

const char *Zone::GetSpellBlockedMessage(uint32 spell_id, const glm::vec3 &location)
{
	if (blocked_spells) {
		for (int x = 0; x < GetZoneTotalBlockedSpells(); x++) {
			if (spell_id != blocked_spells[x].spellid && blocked_spells[x].spellid != 0) {
				continue;
			}
			switch (blocked_spells[x].type) {
				case ZoneBlockedSpellTypes::ZoneWide: {
					return blocked_spells[x].message;
					break;
				}
				case ZoneBlockedSpellTypes::Region: {
					if (IsWithinAxisAlignedBox(
						location,
						blocked_spells[x].m_Location - blocked_spells[x].m_Difference,
						blocked_spells[x].m_Location + blocked_spells[x].m_Difference
					)) {
						return blocked_spells[x].message;
					}
					break;
				}
				default: {
					continue;
					break;
				}
			}
		}
	}
	return "Error: Message String Not Found\0";
}

void Zone::SetInstanceTimer(uint32 new_duration)
{
	if(Instance_Timer)
	{
		Instance_Timer->Start(new_duration * 1000);
	}
}

void Zone::LoadLDoNTraps()
{
	const auto& l = LdonTrapTemplatesRepository::All(content_db);

	for (const auto& e : l) {
		auto t = new LDoNTrapTemplate;

		t->id       = e.id;
		t->type     = static_cast<LDoNChestTypes>(e.type);
		t->spell_id = static_cast<uint32>(e.spell_id);
		t->skill    = e.skill;
		t->locked   = e.locked;

		ldon_trap_list[e.id] = t;
	}
}

void Zone::LoadLDoNTrapEntries()
{
	const auto& l = LdonTrapEntriesRepository::All(content_db);

	for (const auto& e : l) {
		auto t = new LDoNTrapTemplate;

		auto i = ldon_trap_list.find(e.trap_id);
		if (i == ldon_trap_list.end()) {
			continue;
		}

		t = ldon_trap_list[e.trap_id];

		std::list<LDoNTrapTemplate*> tl;

		auto ei = ldon_trap_entry_list.find(e.id);
		if (ei != ldon_trap_entry_list.end()) {
			tl = ldon_trap_entry_list[e.id];
		}

		tl.emplace_back(t);

		ldon_trap_entry_list[e.id] = tl;
	}
}

void Zone::LoadVeteranRewards()
{
	VeteranRewards.clear();

	InternalVeteranReward current_reward;
	current_reward.claim_id = 0;

    const std::string query = "SELECT claim_id, name, item_id, charges "
                            "FROM veteran_reward_templates "
                            "WHERE reward_slot < 8 and claim_id > 0 "
                            "ORDER by claim_id, reward_slot";
    auto results = content_db.QueryDatabase(query);
    if (!results.Success()) {
        return;
    }

	LogInfo("Loaded [{}] veteran reward(s)", Strings::Commify(results.RowCount()));

	int index = 0;
    for (auto row = results.begin(); row != results.end(); ++row, ++index)
    {
        uint32 claim = Strings::ToInt(row[0]);

        if(claim != current_reward.claim_id)
        {
            if(current_reward.claim_id != 0)
            {
                current_reward.claim_count = index;
                current_reward.number_available = 1;
                VeteranRewards.push_back(current_reward);
            }

            index = 0;
            memset(&current_reward, 0, sizeof(InternalVeteranReward));
            current_reward.claim_id = claim;
        }

        strcpy(current_reward.items[index].item_name, row[1]);
        current_reward.items[index].item_id = Strings::ToInt(row[2]);
        current_reward.items[index].charges = Strings::ToInt(row[3]);
    }

    if(current_reward.claim_id != 0)
    {
        current_reward.claim_count = index;
        current_reward.number_available = 1;
        VeteranRewards.push_back(current_reward);
    }

}

void Zone::LoadAlternateCurrencies()
{
	AlternateCurrencies.clear();

	const auto& l = AlternateCurrencyRepository::All(content_db);

	if (l.empty()) {
		return;
	}

	AltCurrencyDefinition_Struct c;

	for (const auto &e : l) {
		c.id      = e.id;
		c.item_id = e.item_id;
		AlternateCurrencies.push_back(c);
	}

	LogInfo(
		"Loaded [{}] Alternate Currenc{}",
		Strings::Commify(l.size()),
		l.size() != 1 ? "ies" : "y"
	);
}

void Zone::UpdateQGlobal(uint32 qid, QGlobal newGlobal)
{
	if(newGlobal.npc_id != 0)
		return;

	if(newGlobal.char_id != 0)
		return;

	if(newGlobal.zone_id == GetZoneID() || newGlobal.zone_id == 0)
	{
		if(qGlobals)
		{
			qGlobals->AddGlobal(qid, newGlobal);
		}
		else
		{
			qGlobals = new QGlobalCache();
			qGlobals->AddGlobal(qid, newGlobal);
		}
	}
}

void Zone::DeleteQGlobal(std::string name, uint32 npcID, uint32 charID, uint32 zoneID)
{
	if(qGlobals)
	{
		qGlobals->RemoveGlobal(name, npcID, charID, zoneID);
	}
}

void Zone::LoadAdventureFlavor()
{
	const std::string query = "SELECT id, text FROM adventure_template_entry_flavor";
	auto results = content_db.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

    for (auto row = results.begin(); row != results.end(); ++row) {
        uint32 id = Strings::ToInt(row[0]);
        adventure_entry_list_flavor[id] = row[1];
    }

	LogInfo("Loaded [{}] adventure text entries", Strings::Commify(results.RowCount()));
}

void Zone::DoAdventureCountIncrease()
{
	ServerZoneAdventureDataReply_Struct *sr = (ServerZoneAdventureDataReply_Struct*)adv_data;
	if(sr->count < sr->total)
	{
		sr->count++;
		auto pack = new ServerPacket(ServerOP_AdventureCountUpdate, sizeof(uint16));
		*((uint16*)pack->pBuffer) = instanceid;
		worldserver.SendPacket(pack);
		delete pack;
	}
}

void Zone::DoAdventureAssassinationCountIncrease()
{
	ServerZoneAdventureDataReply_Struct *sr = (ServerZoneAdventureDataReply_Struct*)adv_data;
	if(sr->assa_count < RuleI(Adventure, NumberKillsForBossSpawn))
	{
		sr->assa_count++;
		auto pack = new ServerPacket(ServerOP_AdventureAssaCountUpdate, sizeof(uint16));
		*((uint16*)pack->pBuffer) = instanceid;
		worldserver.SendPacket(pack);
		delete pack;
	}
}

void Zone::DoAdventureActions()
{
	const auto* ds = (ServerZoneAdventureDataReply_Struct*) adv_data;
	if (ds->type == Adventure_Collect) {
		int count = (ds->total - ds->count) * 25 / RuleI(Adventure, LDoNLootCountModifier);
		entity_list.AddLootToNPCS(ds->data_id, count);
		did_adventure_actions = true;
	} else if (ds->type == Adventure_Assassinate) {
		if (ds->assa_count >= RuleI(Adventure, NumberKillsForBossSpawn)) {
			const auto* d = content_db.LoadNPCTypesData(ds->data_id);
			if (d) {
				NPC* npc = new NPC(
					d,
					nullptr,
					glm::vec4(ds->assa_x, ds->assa_y, ds->assa_z, ds->assa_h),
					GravityBehavior::Water
				);

				npc->AddLootTable();

				if (npc->DropsGlobalLoot()) {
					npc->CheckGlobalLootTables();
				}

				entity_list.AddNPC(npc);
				npc->Shout("Rarrrgh!");
				did_adventure_actions = true;
			}
		}
	}
	else
	{
		did_adventure_actions = true;
	}

}

void Zone::LoadNPCEmotes(std::vector<NPC_Emote_Struct*>* v)
{
	for (auto &e: *v) {
		safe_delete(e);
	}

	v->clear();

	const auto& l = NpcEmotesRepository::All(content_db);

	for (const auto& e : l) {
		auto n = new NPC_Emote_Struct;

		n->emoteid = e.emoteid;
		n->event_  = e.event_;
		n->type    = e.type;

		strn0cpy(n->text, e.text.c_str(), sizeof(n->text));

		v->push_back(n);
	}

	LogInfo(
		"Loaded [{}] NPC Emote{}",
		Strings::Commify(l.size()),
		l.size() != 1 ? "s" : ""
	);

}

void Zone::ClearSpawnTimers()
{
	LinkedListIterator<Spawn2 *> iterator(spawn2_list);

	iterator.Reset();

	std::vector<uint32> respawn_ids;

	while (iterator.MoreElements()) {
		respawn_ids.emplace_back(iterator.GetData()->spawn2_id);

		iterator.Advance();
	}

	if (!respawn_ids.empty()) {
		RespawnTimesRepository::DeleteWhere(
			database,
			fmt::format(
				"`instance_id` = {} AND `id` IN ({})",
				GetInstanceID(),
				Strings::Join(respawn_ids, ", ")
			)
		);
	}
}

uint32 Zone::GetSpawnKillCount(uint32 in_spawnid) {
	LinkedListIterator<Spawn2*> iterator(spawn2_list);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(iterator.GetData()->GetID() == in_spawnid)
		{
			return(iterator.GetData()->killcount);
		}
		iterator.Advance();
	}
	return 0;
}

bool Zone::IsWaterZone(float z)
{

	switch (GetZoneID()) {
		case Zones::KEDGE:
			return true;
		case Zones::POWATER:
			if (z < 0.0f) {
				return true;
			}
			return false;
		default:
			return false;
	}
}

void Zone::SetIsHotzone(bool is_hotzone)
{
	Zone::is_hotzone = is_hotzone;
}

void Zone::RequestUCSServerStatus() {
	auto outapp = new ServerPacket(ServerOP_UCSServerStatusRequest, sizeof(UCSServerStatus_Struct));
	auto ucsss = (UCSServerStatus_Struct*)outapp->pBuffer;
	ucsss->available = 0;
	ucsss->port = Config->ZonePort;
	ucsss->unused = 0;
	worldserver.SendPacket(outapp);
	safe_delete(outapp);
}

void Zone::SetUCSServerAvailable(bool ucss_available, uint32 update_timestamp) {
	if (m_last_ucss_update == update_timestamp && m_ucss_available != ucss_available) {
		m_ucss_available = false;
		RequestUCSServerStatus();
		return;
	}
	if (m_last_ucss_update < update_timestamp)
		m_ucss_available = ucss_available;
}

bool Zone::IsQuestHotReloadQueued() const
{
	return quest_hot_reload_queued;
}

void Zone::SetQuestHotReloadQueued(bool in_quest_hot_reload_queued)
{
	quest_hot_reload_queued = in_quest_hot_reload_queued;
}

void Zone::LoadGrids()
{
	zone_grids        = GridRepository::GetZoneGrids(content_db, GetZoneID());
	zone_grid_entries = GridEntriesRepository::GetZoneGridEntries(content_db, GetZoneID());

	LogInfo(
		"Loaded [{}] grids and [{}] grid_entries",
		Strings::Commify(zone_grids.size()),
		Strings::Commify(zone_grid_entries.size())
	);
}

Timer Zone::GetInitgridsTimer()
{
	return initgrids_timer;
}

uint32 Zone::GetInstanceTimeRemaining() const
{
	return instance_time_remaining;
}

void Zone::SetInstanceTimeRemaining(uint32 instance_time_remaining)
{
	Zone::instance_time_remaining = instance_time_remaining;
}

bool Zone::IsZone(uint32 zone_id, uint16 instance_id) const
{
	return (zoneid == zone_id && instanceid == instance_id);
}

DynamicZone* Zone::GetDynamicZone()
{
	if (GetInstanceID() == 0)
	{
		return nullptr;
	}

	// todo: cache dynamic zone id on zone later for faster lookup
	for (const auto& dz_iter : zone->dynamic_zone_cache)
	{
		if (dz_iter.second->IsSameDz(GetZoneID(), GetInstanceID()))
		{
			return dz_iter.second.get();
		}
	}

	return nullptr;
}

uint32 Zone::GetCurrencyID(uint32 item_id)
{
	if (!item_id) {
		return 0;
	}

	for (const auto& alternate_currency : AlternateCurrencies) {
		if (item_id == alternate_currency.item_id) {
			return alternate_currency.id;
		}
	}

	return 0;
}

uint32 Zone::GetCurrencyItemID(uint32 currency_id)
{
	if (!currency_id) {
		return 0;
	}

	for (const auto& alternate_currency : AlternateCurrencies) {
		if (currency_id == alternate_currency.id) {
			return alternate_currency.item_id;
		}
	}

	return 0;
}

std::string Zone::GetZoneDescription()
{
	if (!IsLoaded()) {
		return fmt::format(
			"PID ({})",
			EQ::GetPID()
		);
	}

	return fmt::format(
		"PID ({}) {} ({}){}{}",
		EQ::GetPID(),
		GetLongName(),
		GetZoneID(),
		(
			GetInstanceID() ?
			fmt::format(
				" (Instance ID {})",
				GetInstanceID()
			) :
			""
		),
		(
			GetInstanceVersion() ?
			fmt::format(
				" (Version {})",
				GetInstanceVersion()
			) :
			""
		)
	);
}

void Zone::SendReloadMessage(std::string reload_type)
{
	worldserver.SendEmoteMessage(
		0,
		0,
		AccountStatus::GMAdmin,
		Chat::Yellow,
		fmt::format(
			"{} reloaded for {}.",
			reload_type,
			GetZoneDescription()
		).c_str()
	);
}

void Zone::SendDiscordMessage(int webhook_id, const std::string& message)
{
	if (worldserver.Connected()) {
		auto pack = new ServerPacket(ServerOP_DiscordWebhookMessage, sizeof(DiscordWebhookMessage_Struct) + 1);
		auto *q   = (DiscordWebhookMessage_Struct *) pack->pBuffer;

		strn0cpy(q->message, message.c_str(), 2000);
		q->webhook_id = webhook_id;

		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void Zone::SendDiscordMessage(const std::string& webhook_name, const std::string &message)
{
	bool not_found = true;

	for (int i= 0; i < MAX_DISCORD_WEBHOOK_ID; i++) {
		auto &w = EQEmuLogSys::Instance()->GetDiscordWebhooks()[i];
		if (w.webhook_name == webhook_name) {
			SendDiscordMessage(w.id, message + "\n");
			not_found = false;
		}
	}

	if (not_found) {
		LogDiscord("Did not find valid webhook by webhook name [{}]", webhook_name);
	}
}

void Zone::LoadDynamicZoneTemplates()
{
	dz_template_cache.clear();
	auto dz_templates = DynamicZoneTemplatesRepository::All(content_db);
	for (const auto& dz_template : dz_templates)
	{
		dz_template_cache[dz_template.id] = dz_template;
	}
}

std::string Zone::GetAAName(int aa_id)
{
	if (!aa_id) {
		return std::string();
	}

	int current_aa_id = 0;

	const auto& r = aa_ranks.find(aa_id);
	if (
		r != aa_ranks.end() &&
		r->second.get()->base_ability
	) {
		current_aa_id = r->second.get()->base_ability->id;
	}

	if (current_aa_id) {
		const auto& a = aa_abilities.find(current_aa_id);
		if (a != aa_abilities.end()) {
			return a->second.get()->name;
		} else {
			for (const auto& b : aa_abilities) {
				if (b.second.get()->first->id == aa_id) {
					return b.second.get()->name;
				}
			}
		}
	}

	return std::string();
}

bool Zone::CompareDataBucket(uint8 comparison_type, const std::string& bucket, const std::string& value)
{
	if (!ComparisonType::IsValid(comparison_type)) {
		return false;
	}

	if (EQ::ValueWithin(comparison_type, ComparisonType::Equal, ComparisonType::NotEqual)) {
		const bool is_equal = value == bucket;

		return comparison_type == ComparisonType::Equal ? is_equal : !is_equal;
	} else if (EQ::ValueWithin(comparison_type, ComparisonType::GreaterOrEqual, ComparisonType::LesserOrEqual)) {
		if (!Strings::IsNumber(value) || !Strings::IsNumber(bucket)) {
			return false;
		}

		const int64 p = Strings::ToBigInt(value);
		const int64 b = Strings::ToBigInt(bucket);

		const bool is_greater_or_equal = p >= b;
		const bool is_lesser_or_equal  = p <= b;

		return comparison_type == ComparisonType::GreaterOrEqual ? is_greater_or_equal : is_lesser_or_equal;
	} else if (EQ::ValueWithin(comparison_type, ComparisonType::Greater, ComparisonType::Lesser)) {
		if (!Strings::IsNumber(value) || !Strings::IsNumber(bucket)) {
			return false;
		}

		const bool is_greater = Strings::ToBigInt(value) > Strings::ToBigInt(bucket);

		return comparison_type == ComparisonType::Greater ? is_greater : !is_greater;
	} else if (EQ::ValueWithin(comparison_type, ComparisonType::Any, ComparisonType::NotAny)) {
		const auto& values = Strings::Split(bucket, "|");
		if (values.empty()) {
			return false;
		}

		const bool is_any = std::find(values.begin(), values.end(), value) != values.end();

		return comparison_type == ComparisonType::Any ? is_any : !is_any;
	} else if (EQ::ValueWithin(comparison_type, ComparisonType::Between, ComparisonType::NotBetween)) {
		if (!Strings::IsNumber(value)) {
			return false;
		}

		const auto& values = Strings::Split(bucket, "|");
		if (values.empty()) {
			return false;
		}

		if (!Strings::IsNumber(values[0]) || !Strings::IsNumber(values[1])) {
			return false;
		}

		const bool is_between = EQ::ValueWithin(
			Strings::ToBigInt(value),
			Strings::ToBigInt(values[0]),
			Strings::ToBigInt(values[1])
		);

		return comparison_type == ComparisonType::Between ? is_between : !is_between;
	}

	return false;
}

void Zone::ReloadContentFlags()
{
	worldserver.SendReload(ServerReload::Type::ContentFlags);
}

void Zone::ClearEXPModifier(Client* c)
{
	exp_modifiers.erase(c->CharacterID());
}

void Zone::ClearEXPModifierByCharacterID(const uint32 character_id)
{
	exp_modifiers.erase(character_id);
}

float Zone::GetAAEXPModifier(Client* c)
{
	const auto& l = exp_modifiers.find(c->CharacterID());
	if (l == exp_modifiers.end()) {
		return 1.0f;
	}

	const auto& v = l->second;

	return v.aa_modifier;
}

float Zone::GetAAEXPModifierByCharacterID(const uint32 character_id)
{
	const auto& l = exp_modifiers.find(character_id);
	if (l == exp_modifiers.end()) {
		return 1.0f;
	}

	const auto& v = l->second;

	return v.aa_modifier;
}

float Zone::GetEXPModifier(Client* c)
{
	const auto& l = exp_modifiers.find(c->CharacterID());
	if (l == exp_modifiers.end()) {
		return 1.0f;
	}

	const auto& v = l->second;

	return v.exp_modifier;
}

float Zone::GetEXPModifierByCharacterID(const uint32 character_id)
{
	const auto& l = exp_modifiers.find(character_id);
	if (l == exp_modifiers.end()) {
		return 1.0f;
	}

	const auto& v = l->second;

	return v.exp_modifier;
}

void Zone::SetAAEXPModifier(Client* c, float aa_modifier)
{
	auto l = exp_modifiers.find(c->CharacterID());
	if (l == exp_modifiers.end()) {
		return;
	}

	auto& m = l->second;

	m.aa_modifier = aa_modifier;

	CharacterExpModifiersRepository::SetEXPModifier(
		database,
		c->CharacterID(),
		GetZoneID(),
		GetInstanceVersion(),
		m
	);
}

void Zone::SetAAEXPModifierByCharacterID(const uint32 character_id, float aa_modifier)
{
	auto l = exp_modifiers.find(character_id);
	if (l == exp_modifiers.end()) {
		return;
	}

	auto& m = l->second;

	m.aa_modifier = aa_modifier;

	CharacterExpModifiersRepository::SetEXPModifier(
		database,
		character_id,
		GetZoneID(),
		GetInstanceVersion(),
		m
	);
}

void Zone::SetEXPModifier(Client* c, float exp_modifier)
{
	auto l = exp_modifiers.find(c->CharacterID());
	if (l == exp_modifiers.end()) {
		return;
	}

	auto& m = l->second;

	m.exp_modifier = exp_modifier;

	CharacterExpModifiersRepository::SetEXPModifier(
		database,
		c->CharacterID(),
		GetZoneID(),
		GetInstanceVersion(),
		m
	);
}

void Zone::SetEXPModifierByCharacterID(const uint32 character_id, float exp_modifier)
{
	auto l = exp_modifiers.find(character_id);
	if (l == exp_modifiers.end()) {
		return;
	}

	auto& m = l->second;

	m.exp_modifier = exp_modifier;

	CharacterExpModifiersRepository::SetEXPModifier(
		database,
		character_id,
		GetZoneID(),
		GetInstanceVersion(),
		m
	);
}

bool Zone::IsIdleWhenEmpty() const
{
	return m_idle_when_empty;
}

void Zone::SetIdleWhenEmpty(bool idle_when_empty)
{
	Zone::m_idle_when_empty = idle_when_empty;
}

uint32 Zone::GetSecondsBeforeIdle() const
{
	return m_seconds_before_idle;
}

void Zone::SetSecondsBeforeIdle(uint32 seconds_before_idle)
{
	Zone::m_seconds_before_idle = seconds_before_idle;
}

bool Zone::DoesAlternateCurrencyExist(uint32 currency_id)
{
	return std::any_of(
		AlternateCurrencies.begin(),
		AlternateCurrencies.end(),
		[&](const auto& c) {
			return c.id == currency_id;
		}
	);
}

std::string Zone::GetBucket(const std::string& bucket_name)
{
	DataBucketKey k = {};
	k.zone_id     = zoneid;
	k.instance_id = instanceid;
	k.key         = bucket_name;

	return DataBucket::GetData(&database, k).value;
}

void Zone::SetBucket(const std::string& bucket_name, const std::string& bucket_value, const std::string& expiration)
{
	DataBucketKey k = {};
	k.zone_id     = zoneid;
	k.instance_id = instanceid;
	k.key         = bucket_name;
	k.expires     = expiration;
	k.value       = bucket_value;

	DataBucket::SetData(&database, k);
}

void Zone::DeleteBucket(const std::string& bucket_name)
{
	DataBucketKey k = {};
	k.zone_id     = zoneid;
	k.instance_id = instanceid;
	k.key         = bucket_name;

	DataBucket::DeleteData(&database, k);
}

std::string Zone::GetBucketExpires(const std::string& bucket_name)
{
	DataBucketKey k = {};
	k.zone_id     = zoneid;
	k.instance_id = instanceid;
	k.key         = bucket_name;

	return DataBucket::GetDataExpires(&database, k);
}

std::string Zone::GetBucketRemaining(const std::string& bucket_name)
{
	DataBucketKey k = {};
	k.zone_id     = zoneid;
	k.instance_id = instanceid;
	k.key         = bucket_name;

	return DataBucket::GetDataRemaining(&database, k);
}

void Zone::DisableRespawnTimers()
{
	LinkedListIterator<Spawn2 *> e(spawn2_list);

	e.Reset();

	while (e.MoreElements()) {
		e.GetData()->SetRespawnTimer(std::numeric_limits<uint32_t>::max());
		e.Advance();
	}
}

bool Zone::ClearVariables()
{
	if (m_zone_variables.empty()) {
		return false;
	}

	m_zone_variables.clear();
	return true;
}

bool Zone::DeleteVariable(const std::string& variable_name)
{
	if (m_zone_variables.empty() || variable_name.empty()) {
		return false;
	}

	auto v = m_zone_variables.find(variable_name);
	if (v == m_zone_variables.end()) {
		return false;
	}

	m_zone_variables.erase(v);

	return true;
}

std::string Zone::GetVariable(const std::string& variable_name)
{
	if (m_zone_variables.empty() || variable_name.empty()) {
		return std::string();
	}

	const auto& v = m_zone_variables.find(variable_name);

	return v != m_zone_variables.end() ? v->second : std::string();
}

std::vector<std::string> Zone::GetVariables()
{
	std::vector<std::string> l;

	if (m_zone_variables.empty()) {
		return l;
	}

	l.reserve(m_zone_variables.size());

	for (const auto& v : m_zone_variables) {
		l.emplace_back(v.first);
	}

	return l;
}

void Zone::SetVariable(const std::string& variable_name, const std::string& variable_value)
{
	if (variable_name.empty()) {
		return;
	}

	m_zone_variables[variable_name] = variable_value;
}

bool Zone::VariableExists(const std::string& variable_name)
{
	if (m_zone_variables.empty() || variable_name.empty()) {
		return false;
	}

	return m_zone_variables.find(variable_name) != m_zone_variables.end();
}

void Zone::ReloadMaps()
{
	zonemap  = Map::LoadMapFile(map_name);
	watermap = WaterMap::LoadWaterMapfile(map_name);
	pathing  = IPathfinder::Load(map_name);
}

uint32 Zone::GetTimerDuration(std::string name)
{
	if (!IsLoaded() || zone_timers.empty()) {
		return 0;
	}

	const auto& e = std::find_if(
		zone_timers.begin(),
		zone_timers.end(),
		[&name](ZoneTimer e) {
			return e.name == name;
		}
	);

	return e != zone_timers.end() ? e->timer_.GetDuration() : 0;
}

uint32 Zone::GetTimerRemainingTime(std::string name)
{
	if (!IsLoaded() || zone_timers.empty()) {
		return 0;
	}

	const auto& e = std::find_if(
		zone_timers.begin(),
		zone_timers.end(),
		[&name](ZoneTimer e) {
			return e.name == name;
		}
	);

	return e != zone_timers.end() ? e->timer_.GetRemainingTime() : 0;
}

bool Zone::HasTimer(std::string name)
{
	if (!IsLoaded() || zone_timers.empty()) {
		return false;
	}

	const auto& e = std::find_if(
		zone_timers.begin(),
		zone_timers.end(),
		[&name](ZoneTimer e) {
			return e.name == name;
		}
	);

	return e != zone_timers.end();
}

bool Zone::IsPausedTimer(std::string name)
{
	if (!IsLoaded() || paused_zone_timers.empty()) {
		return false;
	}

	const auto& e = std::find_if(
		paused_zone_timers.begin(),
		paused_zone_timers.end(),
		[&name](PausedZoneTimer e) {
			return e.name == name;
		}
	);

	return e != paused_zone_timers.end();
}

void Zone::PauseTimer(std::string name)
{
	if (
		!IsLoaded() ||
		zone_timers.empty() ||
		!HasTimer(name) ||
		IsPausedTimer(name)
	) {
		return;
	}

	uint32 remaining_time = 0;

	const bool has_pause_event = parse->ZoneHasQuestSub(EVENT_TIMER_PAUSE);

	if (!zone_timers.empty()) {
		for (auto e = zone_timers.begin(); e != zone_timers.end(); e++) {
			if (e->name == name) {
				remaining_time = e->timer_.GetRemainingTime();

				zone_timers.erase(e);

				const std::string& export_string = fmt::format(
					"{} {}",
					name,
					remaining_time
				);

				LogQuests(
					"Pausing timer [{}] with [{}] ms remaining",
					name,
					remaining_time
				);

				paused_zone_timers.emplace_back(
					PausedZoneTimer{
						.name = name,
						.remaining_time = remaining_time
					}
				);

				if (has_pause_event) {
					parse->EventZone(EVENT_TIMER_PAUSE, this, export_string);
				}

				break;
			}
		}
	}
}

void Zone::ResumeTimer(std::string name)
{
	if (
		!IsLoaded() ||
		paused_zone_timers.empty() ||
		!IsPausedTimer(name)
	) {
		return;
	}

	uint32 remaining_time = 0;

	if (!paused_zone_timers.empty()) {
		for (auto e = paused_zone_timers.begin(); e != paused_zone_timers.end(); e++) {
			if (e->name == name) {
				remaining_time = e->remaining_time;

				paused_zone_timers.erase(e);

				if (!remaining_time) {
					LogQuests("Paused timer [{}] not found or has expired.", name);
					return;
				}

				const std::string& export_string = fmt::format(
					"{} {}",
					name,
					remaining_time
				);

				LogQuests(
					"Creating a new timer and resuming [{}] with [{}] ms remaining",
					name,
					remaining_time
				);

				zone_timers.emplace_back(ZoneTimer(name, remaining_time));

				if (parse->ZoneHasQuestSub(EVENT_TIMER_RESUME)) {
					parse->EventZone(EVENT_TIMER_RESUME, this, export_string);
				}

				break;
			}
		}
	}
}

void Zone::SetTimer(std::string name, uint32 duration)
{
	if (!IsLoaded() || HasTimer(name)) {
		return;
	}

	zone_timers.emplace_back(ZoneTimer(name, duration));

	if (parse->ZoneHasQuestSub(EVENT_TIMER_START)) {
		const std::string& export_string = fmt::format("{} {}", name, duration);
		parse->EventZone(EVENT_TIMER_START, this, export_string);
	}
}

void Zone::StopTimer(std::string name)
{
	if (
		!IsLoaded() ||
		zone_timers.empty() ||
		!HasTimer(name) ||
		IsPausedTimer(name)
	) {
		return;
	}

	const bool has_stop_event = parse->ZoneHasQuestSub(EVENT_TIMER_STOP);

	for (auto e = zone_timers.begin(); e != zone_timers.end(); e++) {
		if (e->name == name) {
			if (has_stop_event) {
				parse->EventZone(EVENT_TIMER_STOP, this, name);
			}

			zone_timers.erase(e);
			break;
		}
	}
}

void Zone::StopAllTimers()
{
	if (!IsLoaded() || zone_timers.empty()) {
		return;
	}

	const bool has_stop_event = parse->ZoneHasQuestSub(EVENT_TIMER_STOP);

	for (auto e = zone_timers.begin(); e != zone_timers.end();) {
		if (has_stop_event) {
			parse->EventZone(EVENT_TIMER_STOP, this, e->name);
		}

		e = zone_timers.erase(e);
	}
}

void Zone::Signal(int signal_id)
{
	m_zone_signals.push_back(signal_id);
}

void Zone::SendPayload(int payload_id, std::string payload_value)
{
	if (parse->ZoneHasQuestSub(EVENT_PAYLOAD)) {
		const auto& export_string = fmt::format("{} {}", payload_id, payload_value);

		parse->EventZone(EVENT_PAYLOAD, this, export_string, 0);
	}
}

std::vector<std::string> Zone::GetPausedTimers()
{
	std::vector<std::string> v;

	if (!paused_zone_timers.empty()) {
		for (auto e = paused_zone_timers.begin(); e != paused_zone_timers.end(); e++) {
			v.emplace_back(e->name);
		}
	}

	return v;
}

std::vector<std::string> Zone::GetTimers()
{
	std::vector<std::string> v;

	if (!zone_timers.empty()) {
		for (auto e = zone_timers.begin(); e != zone_timers.end(); e++) {
			v.emplace_back(e->name);
		}
	}

	return v;
}
