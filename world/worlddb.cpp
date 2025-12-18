/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

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

#include "worlddb.h"

#include "common/eq_packet_structs.h"
#include "common/inventory_profile.h"
#include "common/repositories/character_bind_repository.h"
#include "common/repositories/character_data_repository.h"
#include "common/repositories/character_instance_safereturns_repository.h"
#include "common/repositories/character_material_repository.h"
#include "common/repositories/criteria/content_filter_criteria.h"
#include "common/repositories/inventory_repository.h"
#include "common/repositories/start_zones_repository.h"
#include "common/rulesys.h"
#include "common/strings.h"
#include "common/zone_store.h"
#include "world/sof_char_create_data.h"

#include <cstdlib>
#include <vector>

WorldDatabase database;
WorldDatabase content_db;
extern std::vector<RaceClassAllocation> character_create_allocations;
extern std::vector<RaceClassCombos> character_create_race_class_combos;

void WorldDatabase::GetCharSelectInfo(uint32 account_id, EQApplicationPacket **out_app, uint32 client_version_bit)
{
	EQ::versions::ClientVersion
		   client_version  = EQ::versions::ConvertClientVersionBitToClientVersion(client_version_bit);
	size_t character_limit = EQ::constants::StaticLookup(client_version)->CharacterCreationLimit;

	if (character_limit > EQ::constants::CHARACTER_CREATION_LIMIT) {
		character_limit = EQ::constants::CHARACTER_CREATION_LIMIT;
	}

	// Force Titanium clients to use '8'
	if (client_version == EQ::versions::ClientVersion::Titanium) {
		character_limit = 8;
	}

	auto characters = CharacterDataRepository::GetWhere(
		database,
		fmt::format(
			"`account_id` = {} AND `deleted_at` IS NULL ORDER BY `name` LIMIT {}",
			account_id,
			character_limit
		)
	);

	size_t character_count = characters.size();
	if (characters.empty()) {
		*out_app = new EQApplicationPacket(OP_SendCharInfo, sizeof(CharacterSelect_Struct));
		auto *cs = (CharacterSelect_Struct *) (*out_app)->pBuffer;
		cs->CharCount  = 0;
		cs->TotalChars = character_limit;
		return;
	}

	std::vector<uint32_t> character_ids;
	for (auto &e: characters) {
		character_ids.push_back(e.id);
	}

	const auto& inventories = InventoryRepository::GetWhere(
		*this,
		fmt::format(
			"`character_id` IN ({}) AND `slot_id` BETWEEN {} AND {}",
			Strings::Join(character_ids, ","),
			EQ::invslot::slotHead,
			EQ::invslot::slotFeet
		)
	);

	const auto& character_binds = CharacterBindRepository::GetWhere(
		*this,
		fmt::format(
			"`id` IN ({}) ORDER BY id, slot",
			Strings::Join(character_ids, ",")
		)
	);

	const auto& character_materials = CharacterMaterialRepository::GetWhere(
		*this,
		fmt::format(
			"`id` IN ({}) ORDER BY id, slot",
			Strings::Join(character_ids, ",")
		)
	);

	size_t packet_size = sizeof(CharacterSelect_Struct) + (sizeof(CharacterSelectEntry_Struct) * character_count);
	*out_app = new EQApplicationPacket(OP_SendCharInfo, packet_size);

	unsigned char *buff_ptr = (*out_app)->pBuffer;
	auto          *cs       = (CharacterSelect_Struct *) buff_ptr;

	cs->CharCount  = character_count;
	cs->TotalChars = character_limit;

	buff_ptr += sizeof(CharacterSelect_Struct);
	for (auto &e: characters) {
		auto                 *cse = (CharacterSelectEntry_Struct *) buff_ptr;
		PlayerProfile_Struct pp;
		EQ::InventoryProfile inv;

		pp.SetPlayerProfileVersion(EQ::versions::ConvertClientVersionToMobVersion(client_version));
		inv.SetInventoryVersion(client_version);
		inv.SetGMInventory(true); // charsel can not interact with items..but, no harm in setting to full expansion support

		uint32 character_id = e.id;
		uint8  has_home     = 0;
		uint8  has_bind     = 0;

		memset(&pp, 0, sizeof(PlayerProfile_Struct));
		memset(cse->Name, 0, sizeof(cse->Name));
		strcpy(cse->Name, e.name.c_str());
		cse->Class       = e.class_;
		cse->Race        = e.race;
		cse->Level       = e.level;
		cse->ShroudClass = cse->Class;
		cse->ShroudRace  = cse->Race;
		cse->Zone        = e.zone_id;
		cse->Instance    = 0;
		cse->Gender      = e.gender;
		cse->Face        = e.face;

		for (auto &s: cse->Equip) {
			s.Material        = 0;
			s.Unknown1        = 0;
			s.EliteModel      = 0;
			s.HerosForgeModel = 0;
			s.Unknown2        = 0;
			s.Color           = 0;
		}

		cse->Unknown15       = 0xFF;
		cse->Unknown19       = 0xFF;
		cse->DrakkinTattoo   = e.drakkin_tattoo;
		cse->DrakkinDetails  = e.drakkin_details;
		cse->Deity           = e.deity;
		cse->PrimaryIDFile   = 0;                            // Processed Below
		cse->SecondaryIDFile = 0;                        // Processed Below
		cse->HairColor       = e.hair_color;
		cse->BeardColor      = e.beard_color;
		cse->EyeColor1       = e.eye_color_1;
		cse->EyeColor2       = e.eye_color_2;
		cse->HairStyle       = e.hair_style;
		cse->Beard           = e.beard;
		cse->GoHome          = 0;                                // Processed Below
		cse->Tutorial        = 0;                                // Processed Below
		cse->DrakkinHeritage = e.drakkin_heritage;
		cse->Unknown1        = 0;
		cse->Enabled         = 1;
		cse->LastLogin       = e.last_login;            // RoF2 value: 1212696584
		cse->Unknown2        = 0;

		if (RuleB(World, EnableReturnHomeButton)) {
			int now = time(nullptr);
			if (now - e.last_login >= RuleI(World, MinOfflineTimeToReturnHome)) {
				cse->GoHome = 1;
			}
		}

		if (RuleB(World, EnableTutorialButton) && (cse->Level <= RuleI(World, MaxLevelForTutorial))) {
			cse->Tutorial = 1;
		}

		// binds
		int bind_count = 0;
		for (auto &bind : character_binds) {
			if (bind.id != e.id) {
				continue;
			}

			if (bind.slot == 4) {
				has_home = 1;
				pp.binds[4].zone_id     = bind.zone_id;
				pp.binds[4].instance_id = bind.instance_id;
				pp.binds[4].x           = bind.x;
				pp.binds[4].y           = bind.y;
				pp.binds[4].z           = bind.z;
				pp.binds[4].heading     = bind.heading;
			}

			if (bind.slot == 0) {
				has_bind = 1;
			}

			bind_count++;
		}

		if (has_home == 0 || has_bind == 0) {
			const auto &start_zones = StartZonesRepository::GetWhere(
				content_db,
				fmt::format(
					"`player_class` = {} AND `player_deity` = {} AND `player_race` = {} {}",
					cse->Class,
					cse->Deity,
					cse->Race,
					ContentFilterCriteria::apply().c_str()
				)
			);

			if (!start_zones.empty()) {
				pp.binds[4].zone_id = start_zones[0].zone_id;
				pp.binds[4].x       = start_zones[0].x;
				pp.binds[4].y       = start_zones[0].y;
				pp.binds[4].z       = start_zones[0].z;
				pp.binds[4].heading = start_zones[0].heading;

				if (start_zones[0].bind_id != 0) {
					pp.binds[4].zone_id = start_zones[0].bind_id;
					auto z = GetZone(pp.binds[4].zone_id);
					if (z) {
						pp.binds[4].x       = z->safe_x;
						pp.binds[4].y       = z->safe_y;
						pp.binds[4].z       = z->safe_z;
						pp.binds[4].heading = z->safe_heading;
					}
				} else {
					pp.binds[4].zone_id = start_zones[0].zone_id;
					pp.binds[4].x       = start_zones[0].x;
					pp.binds[4].y       = start_zones[0].y;
					pp.binds[4].z       = start_zones[0].z;
					pp.binds[4].heading = start_zones[0].heading;

					if (pp.binds[4].x == 0 && pp.binds[4].y == 0 && pp.binds[4].z == 0 && pp.binds[4].heading == 0) {
						auto zone = GetZone(pp.binds[4].zone_id);
						if (zone) {
							pp.binds[4].x       = zone->safe_x;
							pp.binds[4].y       = zone->safe_y;
							pp.binds[4].z       = zone->safe_z;
							pp.binds[4].heading = zone->safe_heading;
						}
					}
				}
			}
			else {
				LogError("No start zone found for class [{}] deity [{}] race [{}]", cse->Class, cse->Deity, cse->Race);
			}


			pp.binds[0] = pp.binds[4];

			// If we don't have home set, set it
			if (has_home == 0) {
				auto bind = CharacterBindRepository::NewEntity();
				bind.id          = character_id;
				bind.zone_id     = pp.binds[4].zone_id;
				bind.instance_id = 0;
				bind.x           = pp.binds[4].x;
				bind.y           = pp.binds[4].y;
				bind.z           = pp.binds[4].z;
				bind.heading     = pp.binds[4].heading;
				bind.slot        = 4;
				CharacterBindRepository::ReplaceOne(*this, bind);
			}

			// If we don't have regular bind set, set it
			if (has_bind == 0) {
				auto bind = CharacterBindRepository::NewEntity();
				bind.id          = character_id;
				bind.zone_id     = pp.binds[0].zone_id;
				bind.instance_id = 0;
				bind.x           = pp.binds[0].x;
				bind.y           = pp.binds[0].y;
				bind.z           = pp.binds[0].z;
				bind.heading     = pp.binds[0].heading;
				bind.slot        = 0;
				CharacterBindRepository::ReplaceOne(*this, bind);
			}
		}

		// If our bind count is less than 5, then we have null data that needs to be filled in
		if (bind_count < 5) {
			// we know that home and main bind must be valid here, so we don't check those
			// we also use home to fill in the null data like live does.

			std::vector<CharacterBindRepository::CharacterBind> binds;

			for (int i = 1; i < 4; i++) {
				if (pp.binds[i].zone_id != 0) { // we assume 0 is the only invalid one ...
					continue;
				}

				auto bind = CharacterBindRepository::NewEntity();

				bind.slot        = i;
				bind.id          = character_id;
				bind.zone_id     = pp.binds[4].zone_id;
				bind.instance_id = 0;
				bind.x           = pp.binds[4].x;
				bind.y           = pp.binds[4].y;
				bind.z           = pp.binds[4].z;
				bind.heading     = pp.binds[4].heading;
				binds.emplace_back(bind);
			}

			CharacterBindRepository::ReplaceMany(*this, binds);
		}

		for (auto &cm : character_materials) {
			pp.item_tint.Slot[cm.slot].Red     = cm.red;
			pp.item_tint.Slot[cm.slot].Green   = cm.green;
			pp.item_tint.Slot[cm.slot].Blue    = cm.blue;
			pp.item_tint.Slot[cm.slot].UseTint = cm.use_tint;
			pp.item_tint.Slot[cm.slot].Color   = cm.color;
		}

		if (GetCharSelInventory(inventories, e, &inv)) {
			const EQ::ItemData     *item          = nullptr;
			const EQ::ItemInstance *inst          = nullptr;
			int16                  inventory_slot = 0;

			for (uint32 matslot = EQ::textures::textureBegin; matslot < EQ::textures::materialCount; matslot++) {
				inventory_slot = EQ::InventoryProfile::CalcSlotFromMaterial(matslot);
				if (inventory_slot == INVALID_INDEX) { continue; }
				inst = inv.GetItem(inventory_slot);
				if (inst == nullptr) {
					continue;
				}

				item = inst->GetItem();
				if (item == nullptr) {
					continue;
				}

				if (matslot > 6) {
					uint32 item_id_file = 0;
					// Weapon Models
					if (inst->GetOrnamentationIDFile() != 0) {
						item_id_file = inst->GetOrnamentationIDFile();
						cse->Equip[matslot].Material = item_id_file;
					}
					else {
						if (strlen(item->IDFile) > 2) {
							item_id_file = Strings::ToInt(&item->IDFile[2]);
							cse->Equip[matslot].Material = item_id_file;
						}
					}

					if (matslot == EQ::textures::weaponPrimary) {
						cse->PrimaryIDFile = item_id_file;
					}
					else {
						cse->SecondaryIDFile = item_id_file;
					}
				}
				else {
					// Armor Materials/Models
					uint32 color = (
						pp.item_tint.Slot[matslot].UseTint ?
							pp.item_tint.Slot[matslot].Color :
							inst->GetColor()
					);
					cse->Equip[matslot].Material        = item->Material;
					cse->Equip[matslot].EliteModel      = item->EliteMaterial;
					cse->Equip[matslot].HerosForgeModel = inst->GetOrnamentHeroModel(matslot);
					cse->Equip[matslot].Color           = color;
				}
			}
		}
		else {
			printf("Error loading inventory for %s\n", cse->Name);
		}
		buff_ptr += sizeof(CharacterSelectEntry_Struct);
	}
}

int WorldDatabase::MoveCharacterToBind(int character_id, uint8 bind_number)
{
	/*  if an invalid bind point is specified, use the primary bind */
	if (bind_number > 4)
		bind_number = 0;

	std::string query = fmt::format(
		SQL(
			SELECT
			zone_id, instance_id, x, y, z, heading
			FROM
			character_bind
			WHERE
			id = {}
			AND
			slot = {}
			LIMIT 1
		),
		character_id,
		bind_number
	);
	auto results = database.QueryDatabase(query);
	if(!results.Success() || results.RowCount() == 0) {
		return 0;
	}

	int zone_id, instance_id;
	double x, y, z, heading;
	for (auto row = results.begin(); row != results.end(); ++row) {
		zone_id = Strings::ToInt(row[0]);
		instance_id = Strings::ToInt(row[1]);
		x = Strings::ToFloat(row[2]);
		y = Strings::ToFloat(row[3]);
		z = Strings::ToFloat(row[4]);
		heading = Strings::ToFloat(row[5]);
	}

	query = fmt::format(
		SQL(
			UPDATE
			`character_data`
			SET
			`zone_id` = {},
			`zone_instance` = {},
			`x` = {},
			`y` = {},
			`z` = {},
			`heading` = {}
			WHERE `id` = {}
		),
		zone_id,
		instance_id,
		x,
		y,
		z,
		heading,
		character_id
	);

	results = database.QueryDatabase(query);
	if(!results.Success()) {
		return 0;
	}

	return zone_id;
}

int WorldDatabase::MoveCharacterToInstanceSafeReturn(
	int character_id,
	int instance_zone_id,
	int instance_id
)
{
	int zone_id = 0;

	// only moves if safe return is for specified zone instance
	auto entries = CharacterInstanceSafereturnsRepository::GetWhere(
		database,
		fmt::format(
			"character_id = {} AND instance_zone_id = {} AND instance_id = {} AND safe_zone_id > 0",
			character_id, instance_zone_id, instance_id
		)
	);

	if (!entries.empty()) {
		auto entry = entries.front();

		auto results = QueryDatabase(
			fmt::format(
				SQL(
					UPDATE character_data
					SET zone_id = {}, zone_instance = 0, x = {}, y = {}, z = {}, heading = {}
						WHERE id = {};
				),
				entry.safe_zone_id,
				entry.safe_x,
				entry.safe_y,
				entry.safe_z,
				entry.safe_heading,
				character_id
			)
		);

		if (results.Success() && results.RowsAffected() > 0) {
			zone_id = entry.safe_zone_id;
		}
	}

	if (zone_id == 0) {
		zone_id = MoveCharacterToBind(character_id);
	}

	return zone_id;
}

bool WorldDatabase::GetStartZone(
	PlayerProfile_Struct *pp,
	CharCreate_Struct *p_char_create_struct,
	bool is_titanium
)
{
	// SoF doesn't send the player_choice field in character creation, it now sends the real zoneID instead.
	//
	// For SoF, search for an entry in start_zones with a matching zone_id, class, race and deity.
	//
	// For now, if no row matching row is found, send them to Crescent Reach, as that is probably the most likely
	// reason for no match being found.
	//
	if (!pp || !p_char_create_struct) {
		return false;
	}

	pp->x                    = 0;
	pp->y                    = 0;
	pp->z                    = 0;
	pp->heading              = 0;
	pp->zone_id              = 0;
	pp->binds[0].x           = 0;
	pp->binds[0].y           = 0;
	pp->binds[0].z           = 0;
	pp->binds[0].zone_id     = 0;
	pp->binds[0].instance_id = 0;

	// see if we have an entry for start_zone. We can support both titanium & SOF+ by having two entries per class/race/deity combo with different zone_ids
	std::string query;

	if (is_titanium) {
		// Titanium sends player choice (starting city) instead of a zone id
		query = StringFormat(
			"SELECT x, y, z, heading, start_zone, bind_id, bind_x, bind_y, bind_z FROM start_zones WHERE player_choice = %i "
			"AND player_class = %i AND player_deity = %i AND player_race = %i %s",
			p_char_create_struct->start_zone,
			p_char_create_struct->class_,
			p_char_create_struct->deity,
			p_char_create_struct->race,
			ContentFilterCriteria::apply().c_str()
		);
		LogInfo("Titanium Start zone query: [{}]\n", query.c_str());
	}
	else {
		query = StringFormat(
			"SELECT x, y, z, heading, start_zone, bind_id, bind_x, bind_y, bind_z FROM start_zones WHERE zone_id = %i "
			"AND player_class = %i AND player_deity = %i AND player_race = %i %s",
			p_char_create_struct->start_zone,
			p_char_create_struct->class_,
			p_char_create_struct->deity,
			p_char_create_struct->race,
			ContentFilterCriteria::apply().c_str()
		);
		LogInfo("SoF Start zone query: [{}]\n", query.c_str());
	}

    auto results = QueryDatabase(query);
	if(!results.Success()) {
		return false;
	}

	LogInfo("SoF Start zone query: [{}]\n", query.c_str());

	if (results.RowCount() == 0) {
		printf("No start_zones entry in database, using defaults\n");
		is_titanium ? SetTitaniumDefaultStartZone(pp, p_char_create_struct) : SetSoFDefaultStartZone(pp, p_char_create_struct);
    }
    else {
		LogInfo("Found starting location in start_zones");
		auto row = results.begin();
		pp->x                = Strings::ToFloat(row[0]);
		pp->y                = Strings::ToFloat(row[1]);
		pp->z                = Strings::ToFloat(row[2]);
		pp->heading          = Strings::ToFloat(row[3]);
		pp->zone_id          = Strings::ToInt(row[4]);
		pp->binds[0].zone_id = Strings::ToInt(row[5]);
		pp->binds[0].x       = Strings::ToFloat(row[6]);
		pp->binds[0].y       = Strings::ToFloat(row[7]);
		pp->binds[0].z       = Strings::ToFloat(row[8]);
		pp->binds[0].heading = Strings::ToFloat(row[3]);
	}

	if (
		pp->x == 0 &&
		pp->y == 0 &&
		pp->z == 0 &&
		pp->heading == 0
	) {
		auto zone = GetZone(pp->zone_id);
		if (zone) {
			pp->x       = zone->safe_x;
			pp->y       = zone->safe_y;
			pp->z       = zone->safe_z;
			pp->heading = zone->safe_heading;
		}
	}

	if (
		pp->binds[0].x == 0 &&
		pp->binds[0].y == 0 &&
		pp->binds[0].z == 0 &&
		pp->binds[0].heading == 0
	) {
		auto zone = GetZone(pp->binds[0].zone_id);
		if (zone) {
			pp->binds[0].x       = zone->safe_x;
			pp->binds[0].y       = zone->safe_y;
			pp->binds[0].z       = zone->safe_z;
			pp->binds[0].heading = zone->safe_heading;
		}
	}

	return true;
}

void WorldDatabase::SetSoFDefaultStartZone(PlayerProfile_Struct* in_pp, CharCreate_Struct* in_cc){
	int sof_start_zone_id = RuleI(World, SoFStartZoneID);
	if (sof_start_zone_id > 0) {
			in_pp->zone_id = sof_start_zone_id;
			in_cc->start_zone = in_pp->zone_id;
	}
	else if (in_cc->start_zone == RuleI(World, TutorialZoneID)) {
		in_pp->zone_id = in_cc->start_zone;
	}
	else {
		in_pp->x = in_pp->binds[0].x = -51.0f;
		in_pp->y = in_pp->binds[0].y = -20.0f;
		in_pp->z = in_pp->binds[0].z = 0.79f;
		in_pp->heading = in_pp->binds[0].heading = 0.0f;
		in_pp->zone_id = in_pp->binds[0].zone_id = Zones::CRESCENT; // Crescent Reach.
	}
}

void WorldDatabase::SetTitaniumDefaultStartZone(PlayerProfile_Struct* in_pp, CharCreate_Struct* in_cc)
{
	int titanium_start_zone_id = RuleI(World, TitaniumStartZoneID);
	if (titanium_start_zone_id > 0) {
		in_pp->zone_id = titanium_start_zone_id;
		in_pp->binds[0].zone_id = titanium_start_zone_id;
	} else {
		switch (in_cc->start_zone)
		{
			case StartZoneIndex::Odus:
			{
				if (in_cc->deity == Deity::CazicThule) // Cazic-Thule Erudites go to Paineel
				{
					in_pp->zone_id = Zones::PAINEEL; // paineel
					in_pp->binds[0].zone_id = Zones::PAINEEL;
				}
				else
				{
					in_pp->zone_id = Zones::ERUDNEXT;	// erudnext
					in_pp->binds[0].zone_id = Zones::TOX;	// tox
				}
				break;
			}
			case StartZoneIndex::Qeynos:
			{
				in_pp->zone_id = Zones::QEYNOS2;	// qeynos2
				in_pp->binds[0].zone_id = Zones::QEYNOS2;	// qeynos2
				break;
			}
			case StartZoneIndex::Halas:
			{
				in_pp->zone_id = Zones::HALAS;	// halas
				in_pp->binds[0].zone_id = Zones::EVERFROST;	// everfrost
				break;
			}
			case StartZoneIndex::Rivervale:
			{
				in_pp->zone_id = Zones::RIVERVALE;	// rivervale
				in_pp->binds[0].zone_id = Zones::KITHICOR;	// kithicor
				break;
			}
			case StartZoneIndex::Freeport:
			{
				in_pp->zone_id = Zones::FREPORTW;	// freportw
				in_pp->binds[0].zone_id = Zones::FREPORTW;	// freportw
				break;
			}
			case StartZoneIndex::Neriak:
			{
				in_pp->zone_id = Zones::NERIAKA;	// neriaka
				in_pp->binds[0].zone_id = Zones::NEKTULOS;	// nektulos
				break;
			}
			case StartZoneIndex::Grobb:
			{
				in_pp->zone_id = Zones::GROBB;	// grobb
				in_pp->binds[0].zone_id = Zones::INNOTHULE;	// innothule
				break;
			}
			case StartZoneIndex::Oggok:
			{
				in_pp->zone_id = Zones::OGGOK;	// oggok
				in_pp->binds[0].zone_id = Zones::FEERROTT;	// feerrott
				break;
			}
			case StartZoneIndex::Kaladim:
			{
				in_pp->zone_id = Zones::KALADIMA;	// kaladima
				in_pp->binds[0].zone_id = Zones::BUTCHER;	// butcher
				break;
			}
			case StartZoneIndex::GreaterFaydark:
			{
				in_pp->zone_id = Zones::GFAYDARK;	// gfaydark
				in_pp->binds[0].zone_id = Zones::GFAYDARK;	// gfaydark
				break;
			}
			case StartZoneIndex::Felwithe:
			{
				in_pp->zone_id = Zones::FELWITHEA;	// felwithea
				in_pp->binds[0].zone_id = Zones::GFAYDARK;	// gfaydark
				break;
			}
			case StartZoneIndex::Akanon:
			{
				in_pp->zone_id = Zones::AKANON;	// akanon
				in_pp->binds[0].zone_id = Zones::STEAMFONT;	// steamfont
				break;
			}
			case StartZoneIndex::Cabilis:
			{
				in_pp->zone_id = Zones::CABWEST;	// cabwest
				in_pp->binds[0].zone_id = Zones::FIELDOFBONE;	// fieldofbone
				break;
			}
			case StartZoneIndex::SharVahl:
			{
				in_pp->zone_id = Zones::SHARVAHL;	// sharvahl
				in_pp->binds[0].zone_id = Zones::SHARVAHL;	// sharvahl
				break;
			}
			case StartZoneIndex::RatheMtn:
			{
				in_pp->zone_id = Zones::RATHEMTN;	// rathemtn
				in_pp->binds[0].zone_id = Zones::RATHEMTN;	// rathemtn
				break;
			}
		}
	}
}

void WorldDatabase::GetLauncherList(std::vector<std::string> &rl) {
	rl.clear();

    const std::string query = "SELECT name FROM launcher";
    auto results = QueryDatabase(query);
    if (!results.Success()) {
        LogError("WorldDatabase::GetLauncherList: {}", results.ErrorMessage().c_str());
        return;
    }

    for (auto row = results.begin(); row != results.end(); ++row)
        rl.push_back(row[0]);

}

bool WorldDatabase::GetCharacterLevel(const char *name, int &level)
{
	std::string query = StringFormat("SELECT level FROM character_data WHERE name = '%s'", name);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
        LogError("WorldDatabase::GetCharacterLevel: {}", results.ErrorMessage().c_str());
        return false;
	}

	if (results.RowCount() == 0)
        return false;

    auto row = results.begin();
    level = Strings::ToInt(row[0]);

    return true;
}

bool WorldDatabase::LoadCharacterCreateAllocations()
{
	character_create_allocations.clear();

	std::string query = "SELECT * FROM char_create_point_allocations ORDER BY id";
	auto results = QueryDatabase(query);
	if (!results.Success())
        return false;

    for (auto row = results.begin(); row != results.end(); ++row) {
        RaceClassAllocation allocate;
		allocate.Index = Strings::ToInt(row[0]);
		allocate.BaseStats[0] = Strings::ToInt(row[1]);
		allocate.BaseStats[3] = Strings::ToInt(row[2]);
		allocate.BaseStats[1] = Strings::ToInt(row[3]);
		allocate.BaseStats[2] = Strings::ToInt(row[4]);
		allocate.BaseStats[4] = Strings::ToInt(row[5]);
		allocate.BaseStats[5] = Strings::ToInt(row[6]);
		allocate.BaseStats[6] = Strings::ToInt(row[7]);
		allocate.DefaultPointAllocation[0] = Strings::ToInt(row[8]);
		allocate.DefaultPointAllocation[3] = Strings::ToInt(row[9]);
		allocate.DefaultPointAllocation[1] = Strings::ToInt(row[10]);
		allocate.DefaultPointAllocation[2] = Strings::ToInt(row[11]);
		allocate.DefaultPointAllocation[4] = Strings::ToInt(row[12]);
		allocate.DefaultPointAllocation[5] = Strings::ToInt(row[13]);
		allocate.DefaultPointAllocation[6] = Strings::ToInt(row[14]);

		character_create_allocations.push_back(allocate);
    }

	return true;
}

bool WorldDatabase::LoadCharacterCreateCombos()
{
	character_create_race_class_combos.clear();

	std::string query = "SELECT * FROM char_create_combinations ORDER BY race, class, deity, start_zone";
	auto results = QueryDatabase(query);
	if (!results.Success())
        return false;

	for (auto row = results.begin(); row != results.end(); ++row) {
		RaceClassCombos combo;
		combo.AllocationIndex = Strings::ToInt(row[0]);
		combo.Race = Strings::ToInt(row[1]);
		combo.Class = Strings::ToInt(row[2]);
		combo.Deity = Strings::ToInt(row[3]);
		combo.Zone = Strings::ToInt(row[4]);
		combo.ExpansionRequired = Strings::ToInt(row[5]);

		character_create_race_class_combos.push_back(combo);
	}

	return true;
}

bool WorldDatabase::GetCharSelInventory(
	const std::vector<InventoryRepository::Inventory> &inventories,
	const CharacterDataRepository::CharacterData &character,
	EQ::InventoryProfile *inv
)
{
	if (inventories.empty() || !character.id || !inv) {
		return false;
	}

	for (const auto& e : inventories) {
		if (e.character_id != character.id) {
			continue;
		}

		switch (e.slot_id) {
			case EQ::invslot::slotFace:
			case EQ::invslot::slotEar2:
			case EQ::invslot::slotNeck:
			case EQ::invslot::slotShoulders:
			case EQ::invslot::slotBack:
			case EQ::invslot::slotFinger1:
			case EQ::invslot::slotFinger2:
				continue;
			default:
				break;
		}


		uint32 augment_ids[EQ::invaug::SOCKET_COUNT] = {
			e.augment_one,
			e.augment_two,
			e.augment_three,
			e.augment_four,
			e.augment_five,
			e.augment_six
		};

		const EQ::ItemData* item = content_db.GetItem(e.item_id);
		if (!item) {
			continue;
		}

		EQ::ItemInstance *inst = content_db.CreateBaseItem(item, e.charges);

		if (!inst) {
			continue;
		}

		inst->SetCharges(e.charges);

		if (e.color > 0) {
			inst->SetColor(e.color);
		}

		if (item->IsClassCommon()) {
			for (int i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
				if (augment_ids[i]) {
					inst->PutAugment(this, i, augment_ids[i]);
				}
			}
		}

		inst->SetAttuned(e.instnodrop);

		if (!e.custom_data.empty()) {
			inst->SetCustomDataString(e.custom_data);
		}

		if (e.ornament_icon != 0 || e.ornament_idfile != 0 || e.ornament_hero_model != 0) {
			inst->SetOrnamentIcon(e.ornament_icon);
			inst->SetOrnamentationIDFile(e.ornament_idfile);
			inst->SetOrnamentHeroModel(e.ornament_hero_model);
		} else if (item->HerosForgeModel > 0) {
			inst->SetOrnamentHeroModel(item->HerosForgeModel);
		}

		inv->PutItem(e.slot_id, *inst);

		safe_delete(inst);
	}

	return true;
}
