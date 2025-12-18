/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2019 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include "api_service.h"

#include "common/eqemu_logsys.h"
#include "common/net/websocket_server.h"
#include "zone/client.h"
#include "zone/corpse.h"
#include "zone/doors.h"
#include "zone/entity.h"
#include "zone/object.h"
#include "zone/zone.h"
#include "zone/zonedb.h"

#include <memory>

extern Zone *zone;

/**
 * @param connection
 * @param username
 * @param password
 * @return
 */
EQ::Net::WebsocketLoginStatus CheckLogin(
	EQ::Net::WebsocketServerConnection *connection,
	const std::string &username,
	const std::string &password
)
{
	EQ::Net::WebsocketLoginStatus ret;
	ret.logged_in = false;


	std::string prefix = "eqemu";

	ret.account_id = database.CheckLogin(username.c_str(), password.c_str(), prefix.c_str());

	if (ret.account_id == 0) {
		return ret;
	}

	ret.account_name = database.GetAccountName(static_cast<uint32>(ret.account_id));
	ret.logged_in    = true;
	ret.status       = database.GetAccountStatus(ret.account_id);
	return ret;
}

/**
 * @param connection
 * @param params
 * @return
 */
Json::Value ApiGetPacketStatistics(EQ::Net::WebsocketServerConnection *connection, Json::Value params)
{
	if (zone->GetZoneID() == 0) {
		throw EQ::Net::WebsocketException("Zone must be loaded to invoke this call");
	}

	Json::Value response;
	auto        &list = entity_list.GetClientList();

	for (auto &iter : list) {
		auto client                = iter.second;
		auto connection            = client->Connection();
		auto opts                  = connection->GetManager()->GetOptions();
		auto eqs_stats             = connection->GetStats();
		auto &stats                = eqs_stats.ReliableStreamStats;
		auto now                   = EQ::Net::Clock::now();
		auto sec_since_stats_reset = std::chrono::duration_cast<std::chrono::duration<double>>(
			now - stats.created
		).count();

		Json::Value row;

		row["client_id"]                = client->GetID();
		row["client_name"]              = client->GetCleanName();
		row["seconds_since_reset"]      = sec_since_stats_reset;
		row["sent_bytes"]               = stats.sent_bytes;
		row["receive_bytes"]            = stats.recv_bytes;
		row["min_ping"]                 = stats.min_ping;
		row["max_ping"]                 = stats.max_ping;
		row["last_ping"]                = stats.last_ping;
		row["average_ping"]             = stats.avg_ping;
		row["realtime_receive_packets"] = stats.recv_packets;
		row["realtime_sent_packets"]    = stats.sent_packets;
		row["sync_recv_packets"]        = stats.sync_recv_packets;
		row["sync_sent_packets"]        = stats.sync_sent_packets;
		row["sync_remote_recv_packets"] = stats.sync_remote_recv_packets;
		row["sync_remote_sent_packets"] = stats.sync_remote_sent_packets;
		row["packet_loss_in"]           = (100.0 * (1.0 - static_cast<double>(stats.sync_recv_packets) /
														  static_cast<double>(stats.sync_remote_sent_packets)));
		row["packet_loss_out"]          = (100.0 * (1.0 - static_cast<double>(stats.sync_remote_recv_packets) /
														  static_cast<double>(stats.sync_sent_packets)));
		row["resent_packets"]           = stats.resent_packets;
		row["resent_fragments"]         = stats.resent_fragments;
		row["resent_non_fragments"]     = stats.resent_full;
		row["dropped_datarate_packets"] = stats.dropped_datarate_packets;

		Json::Value sent_packet_types;

		for (auto i = 0; i < _maxEmuOpcode; ++i) {
			auto count = eqs_stats.SentCount[i];
			if (count > 0) {
				sent_packet_types[OpcodeNames[i]] = count;
			}
		}

		Json::Value receive_packet_types;

		for (auto i = 0; i < _maxEmuOpcode; ++i) {
			auto count = eqs_stats.RecvCount[i];
			if (count > 0) {
				receive_packet_types[OpcodeNames[i]] = count;
			}
		}

		row["sent_packet_types"]    = sent_packet_types;
		row["receive_packet_types"] = receive_packet_types;

		response.append(row);
	}

	return response;
}

Json::Value ApiGetOpcodeList(EQ::Net::WebsocketServerConnection *connection, Json::Value params)
{
	if (zone->GetZoneID() == 0) {
		throw EQ::Net::WebsocketException("Zone must be loaded to invoke this call");
	}

	Json::Value response;
	for (auto   i = 0; i < _maxEmuOpcode; ++i) {
		Json::Value row = OpcodeNames[i];

		response.append(row);
	}

	return response;
}

Json::Value ApiGetNpcListDetail(EQ::Net::WebsocketServerConnection *connection, Json::Value params)
{
	if (zone->GetZoneID() == 0) {
		throw EQ::Net::WebsocketException("Zone must be loaded to invoke this call");
	}

	Json::Value response;
	auto        &list = entity_list.GetNPCList();

	for (auto &iter : list) {
		auto        npc = iter.second;
		Json::Value row;

		/**
		 * Main
		 */
		row["id"]         = npc->GetID();
		row["clean_name"] = npc->GetCleanName();
		row["x"]          = npc->GetX();
		row["y"]          = npc->GetY();
		row["z"]          = npc->GetZ();
		row["heading"]    = npc->GetHeading();

		/**
		 * Rest
		 */
		row["accuracy_rating"]               = npc->GetAccuracyRating();
		row["ai_has_spells"]                 = npc->AI_HasSpells();
		row["ai_has_spells_effects"]         = npc->AI_HasSpellsEffects();
		row["ammo_id_file"]                  = npc->GetAmmoIDfile();
		row["attack_delay"]                  = npc->GetAttackDelay();
		row["attack_speed"]                  = npc->GetAttackSpeed();
		row["avoidance_rating"]              = npc->GetAvoidanceRating();
		row["base_damage"]                   = npc->GetBaseDamage();
		row["combat_event"]                  = npc->GetCombatEvent();
		row["copper"]                        = npc->GetCopper();
		row["count_loot"]                    = npc->CountLoot();
		row["drops_global_loot"]             = npc->DropsGlobalLoot();
		row["gold"]                          = npc->GetGold();
		row["grid"]                          = npc->GetGrid();
		row["has_private_corpse"]            = npc->HasPrivateCorpse();
		row["is_animal"]                     = npc->IsAnimal();
		row["is_guarding"]                   = npc->IsGuarding();
		row["is_ldon_locked"]                = npc->IsLDoNLocked();
		row["is_ldon_trap_detected"]         = npc->IsLDoNTrapDetected();
		row["is_ldon_trapped"]               = npc->IsLDoNTrapped();
		row["is_merchant_open"]              = npc->IsMerchantOpen();
		row["is_not_targetable_with_hotkey"] = npc->IsNotTargetableWithHotkey();
		row["is_proximity_set"]              = npc->IsProximitySet();
		row["is_taunting"]                   = npc->IsTaunting();
		row["ldon_locked_skill"]             = npc->GetLDoNLockedSkill();
		row["ldon_trap_spell_id"]            = npc->GetLDoNTrapSpellID();
		row["ldon_trap_type"]                = npc->GetLDoNTrapType();
		row["loottable_id"]                  = npc->GetLoottableID();
		row["max_dmg"]                       = npc->GetMaxDMG();
		row["max_wp"]                        = npc->GetMaxWp();
		row["min_damage"]                    = npc->GetMinDamage();
		row["min_dmg"]                       = npc->GetMinDMG();
		row["npc_spells_effects_id"]         = npc->GetNPCSpellsEffectsID();
		row["npc_spells_id"]                 = npc->GetNPCSpellsID();
		row["npchp_regen"]                   = npc->GetNPCHPRegen();
		row["num_merc_types"]                = npc->GetNumMercenaryTypes();
		row["num_mercs"]                     = npc->GetNumberOfMercenaries();
		row["number_of_attacks"]             = npc->GetNumberOfAttacks();
		row["pet_spell_id"]                  = npc->GetPetSpellID();
		row["platinum"]                      = npc->GetPlatinum();
		row["prim_skill"]                    = npc->GetPrimSkill();

		if (npc->IsProximitySet()) {
			row["proximity_max_x"] = npc->GetProximityMaxX();
			row["proximity_max_y"] = npc->GetProximityMaxY();
			row["proximity_max_z"] = npc->GetProximityMaxZ();
			row["proximity_min_x"] = npc->GetProximityMinX();
			row["proximity_min_y"] = npc->GetProximityMinY();
			row["proximity_min_z"] = npc->GetProximityMinZ();
		}

		row["ranged_skill"]    = npc->GetRangedSkill();
		row["raw_ac"]          = npc->GetRawAC();
		row["sec_skill"]       = npc->GetSecSkill();
		row["silver"]          = npc->GetSilver();
		row["slow_mitigation"] = npc->GetSlowMitigation();
		row["spawn_group_id"]  = npc->GetSpawnGroupId();
		row["swarm_owner"]     = npc->GetSwarmOwner();
		row["swarm_target"]    = npc->GetSwarmTarget();
		row["waypoint_max"]    = npc->GetWaypointMax();
		row["npc_aggro"]       = npc->GetNPCAggro();

		response.append(row);
	}

	return response;
}

Json::Value ApiGetDoorListDetail(EQ::Net::WebsocketServerConnection *connection, Json::Value params)
{
	if (zone->GetZoneID() == 0) {
		throw EQ::Net::WebsocketException("Zone must be loaded to invoke this call");
	}

	Json::Value response;
	auto        &door_list = entity_list.GetDoorsList();

	for (auto itr : door_list) {
		Doors *door = itr.second;

		Json::Value row;

		row["door_name"]           = door->GetDoorName();
		row["client_version_mask"] = door->GetClientVersionMask();
		row["disable_timer"]       = door->GetDisableTimer();
		row["door_db_id"]          = door->GetDoorDBID();
		row["door_id"]             = door->GetDoorID();
		row["door_param"]          = door->GetDoorParam();
		row["entity_id"]           = door->GetEntityID();
		row["guild_id"]            = door->GetGuildID();
		row["incline"]             = door->GetIncline();
		row["invert_state"]        = door->GetInvertState();
		row["is_door"]             = door->IsDoor();
		row["is_door_open"]        = door->IsDoorOpen();
		row["is_ldon_door"]        = door->IsLDoNDoor();
		row["key_item"]            = door->GetKeyItem();
		row["lockpick"]            = door->GetLockpick();
		row["no_keyring"]          = door->GetNoKeyring();
		row["open_type"]           = door->GetOpenType();
		row["size"]                = door->GetSize();
		row["trigger_door_id"]     = door->GetTriggerDoorID();
		row["trigger_type"]        = door->GetTriggerType();
		row["x"]                   = door->GetX();
		row["y"]                   = door->GetY();
		row["z"]                   = door->GetZ();

		response.append(row);
	}

	return response;
}

Json::Value ApiGetCorpseListDetail(EQ::Net::WebsocketServerConnection *connection, Json::Value params)
{
	if (zone->GetZoneID() == 0) {
		throw EQ::Net::WebsocketException("Zone must be loaded to invoke this call");
	}

	Json::Value response;
	auto        &corpse_list = entity_list.GetCorpseList();

	for (auto itr : corpse_list) {
		auto corpse = itr.second;

		Json::Value row;

		row["char_id"]              = corpse->GetCharID();
		row["copper"]               = corpse->GetCopper();
		row["corpse_dbid"]          = corpse->GetCorpseDBID();
		row["count_items"]          = corpse->CountItems();
		row["decay_time"]           = corpse->GetDecayTime();
		row["gold"]                 = corpse->GetGold();
		row["is_become_npc_corpse"] = corpse->IsBecomeNPCCorpse();
		row["is_being_looted"]      = corpse->IsBeingLooted();
		row["is_corpse"]            = corpse->IsCorpse();
		row["is_locked"]            = corpse->IsLocked();
		row["is_npc_corpse"]        = corpse->IsNPCCorpse();
		row["is_player_corpse"]     = corpse->IsPlayerCorpse();
		row["is_rezzed"]            = corpse->IsRezzed();
		row["owner_name"]           = corpse->GetOwnerName();
		row["platinum"]             = corpse->GetPlatinum();
		row["player_kill_item"]     = corpse->GetPlayerKillItem();
		row["rez_exp"]              = corpse->GetRezExp();
		row["rez_time"]             = corpse->GetRezTime();
		row["silver"]               = corpse->GetSilver();

		response.append(row);
	}

	return response;
}

Json::Value ApiGetObjectListDetail(EQ::Net::WebsocketServerConnection *connection, Json::Value params)
{
	if (zone->GetZoneID() == 0) {
		throw EQ::Net::WebsocketException("Zone must be loaded to invoke this call");
	}

	Json::Value response;
	auto        &list = entity_list.GetObjectList();

	for (auto &iter : list) {
		auto object = iter.second;

		Json::Value row;

		row["display_name"]    = object->GetDisplayName();
		row["dbid"]            = object->GetDBID();
		row["heading_data"]    = object->GetHeadingData();
		row["icon"]            = object->GetIcon();
		row["is_ground_spawn"] = object->IsGroundSpawn();
		row["item_id"]         = object->GetItemID();
		row["model_name"]      = object->GetModelName();
		row["size"]            = object->GetSize();
		row["solid_type"]      = object->GetSolidType();
		row["tilt_x"]          = object->GetTiltX();
		row["tilt_y"]          = object->GetTiltY();
		row["type"]            = object->GetType();
		row["x"]               = object->GetX();
		row["y"]               = object->GetY();
		row["z"]               = object->GetZ();

		response.append(row);
	}

	return response;
}

Json::Value ApiGetMobListDetail(EQ::Net::WebsocketServerConnection *connection, Json::Value params)
{
	if (zone->GetZoneID() == 0) {
		throw EQ::Net::WebsocketException("Zone must be loaded to invoke this call");
	}

	Json::Value response;
	auto        &list = entity_list.GetMobList();

	for (auto &iter : list) {
		auto mob = iter.second;

		Json::Value row;

		/**
		 * Main
		 */
		row["id"]         = mob->GetID();
		row["clean_name"] = mob->GetCleanName();
		row["x"]          = mob->GetX();
		row["y"]          = mob->GetY();
		row["z"]          = mob->GetZ();
		row["heading"]    = mob->GetHeading();

		/**
		 * Rest
		 */
		row["ac"]                          = mob->GetAC();
		row["ac_softcap"]                  = mob->GetACSoftcap();
		row["ac_sum"]                      = mob->ACSum();
		row["active_light_type"]           = mob->GetActiveLightType();
		row["aggro_range"]                 = mob->GetAggroRange();
		row["allow_beneficial"]            = mob->GetAllowBeneficial();
		row["animation"]                   = mob->GetAnimation();
		row["assist_range"]                = mob->GetAssistRange();
		row["aura_slots"]                  = mob->GetAuraSlots();
		row["base_fear_speed"]             = mob->GetBaseFearSpeed();
		row["base_runspeed"]               = mob->GetBaseRunspeed();
		row["base_size"]                   = mob->GetBaseSize();
		row["base_walkspeed"]              = mob->GetBaseWalkspeed();
		row["beard"]                       = mob->GetBeard();
		row["beard_color"]                 = mob->GetBeardColor();
		row["best_melee_skill"]            = mob->GetBestMeleeSkill();
		row["calc_fear_resist_chance"]     = mob->CalcFearResistChance();
		row["calc_resist_chance_bonus"]    = mob->CalcResistChanceBonus();
		row["can_block_spell"]             = mob->CanBlockSpell();
		row["can_facestab"]                = mob->CanFacestab();
		row["casted_spell_inv_slot"]       = mob->GetCastedSpellInvSlot();
		row["casting_spell_id"]            = mob->CastingSpellID();
		row["charmed"]                     = mob->Charmed();
		row["check_last_los_state"]        = mob->CheckLastLosState();
		row["class"]                       = mob->GetClass();
		row["class_level_factor"]          = mob->GetClassLevelFactor();
		row["class_race_ac_bonus"]         = mob->GetClassRaceACBonus();
		row["compute_defense"]             = mob->compute_defense();
		row["count_dispellable_buffs"]     = mob->CountDispellableBuffs();
		row["cripp_blow_chance"]           = mob->GetCrippBlowChance();
		row["cur_wp"]                      = mob->GetCurWp();
		row["cwp"]                         = mob->GetCWP();
		row["cwpp"]                        = mob->GetCWPP();
		row["divine_aura"]                 = mob->DivineAura();
		row["dont_buff_me_before"]         = mob->DontBuffMeBefore();
		row["dont_cure_me_before"]         = mob->DontCureMeBefore();
		row["dont_dot_me_before"]          = mob->DontDotMeBefore();
		row["dont_heal_me_before"]         = mob->DontHealMeBefore();
		row["dont_root_me_before"]         = mob->DontRootMeBefore();
		row["dont_snare_me_before"]        = mob->DontSnareMeBefore();
		row["drakkin_details"]             = mob->GetDrakkinDetails();
		row["drakkin_heritage"]            = mob->GetDrakkinHeritage();
		row["drakkin_tattoo"]              = mob->GetDrakkinTattoo();
		row["emote_id"]                    = mob->GetEmoteID();
		row["equipment_light_type"]        = mob->GetEquipmentLightType();
		row["eye_color1"]                  = mob->GetEyeColor1();
		row["eye_color2"]                  = mob->GetEyeColor2();
		row["fear_speed"]                  = mob->GetFearSpeed();
		row["flurry_chance"]               = mob->GetFlurryChance();
		row["follow_can_run"]              = mob->GetFollowCanRun();
		row["follow_distance"]             = mob->GetFollowDistance();
		row["follow_id"]                   = mob->GetFollowID();
		row["gender"]                      = mob->GetGender();
		row["hair_color"]                  = mob->GetHairColor();
		row["hair_style"]                  = mob->GetHairStyle();
		row["has_active_song"]             = mob->HasActiveSong();
		row["has_assist_aggro"]            = mob->HasAssistAggro();
		row["has_died"]                    = mob->HasDied();
		row["has_disc_buff"]               = mob->HasDiscBuff();
		row["has_endur_upkeep"]            = mob->HasEndurUpkeep();
		row["has_free_aura_slots"]         = mob->HasFreeAuraSlots();
		row["has_free_trap_slots"]         = mob->HasFreeTrapSlots();
		row["has_mgb"]                     = mob->HasMGB();
		row["has_numhits"]                 = mob->HasNumhits();
		row["has_pet"]                     = mob->HasPet();
		row["has_pet_affinity"]            = mob->HasPetAffinity();
		row["has_primary_aggro"]           = mob->HasPrimaryAggro();
		row["has_project_illusion"]        = mob->HasProjectIllusion();
		row["has_projectile_attack"]       = mob->HasProjectileAttack();
		row["has_shield_equipped"]         = mob->HasShieldEquipped();
		row["has_special_abilities"]       = mob->HasSpecialAbilities();
		row["has_tar_reflection"]          = mob->HasTargetReflection();
		row["has_temp_pets_active"]        = mob->HasTempPetsActive();
		row["has_two_hand_blunt_equipped"] = mob->HasTwoHandBluntEquipped();
		row["has_two_hander_equipped"]     = mob->HasTwoHanderEquipped();
		row["hate_summon"]                 = mob->HateSummon();
		row["helm_texture"]                = mob->GetHelmTexture();
		row["hp"]                          = mob->GetHP();
		row["improved_taunt"]              = mob->ImprovedTaunt();
		row["innate_light_type"]           = mob->GetInnateLightType();
		row["is_ai_controlled"]            = mob->IsAIControlled();
		row["is_amnesiad"]                 = mob->IsAmnesiad();
		row["is_animation"]                = mob->IsAnimation();
		row["is_blind"]                    = mob->IsBlind();
		row["is_casting"]                  = mob->IsCasting();
		row["is_charmed"]                  = mob->IsCharmed();
		row["is_destructible_object"]      = mob->IsDestructibleObject();
		row["is_engaged"]                  = mob->IsEngaged();
		row["is_enraged"]                  = mob->IsEnraged();
		row["is_familiar"]                 = mob->IsFamiliar();
		row["is_feared"]                   = mob->IsFeared();
		row["is_findable"]                 = mob->IsFindable();
		row["is_focused"]                  = mob->IsFocused();
		row["is_g_held"]                   = mob->IsGHeld();
		row["is_grouped"]                  = mob->IsGrouped();
		row["is_held"]                     = mob->IsHeld();
		row["is_looting"]                  = mob->IsLooting();
		row["is_melee_disabled"]           = mob->IsMeleeDisabled();
		row["is_mezzed"]                   = mob->IsMezzed();
		row["is_moved"]                    = mob->IsMoved();
		row["is_moving"]                   = mob->IsMoving();
		row["is_no_cast"]                  = mob->IsNoCast();
		row["is_off_hand_atk"]             = mob->IsOffHandAtk();
		row["is_pet_owner_client"]         = mob->IsPetOwnerClient();
		row["is_pet_regroup"]              = mob->IsPetRegroup();
		row["is_pet_stop"]                 = mob->IsPetStop();
		row["is_pseudo_rooted"]            = mob->IsPseudoRooted();
		row["is_raid_grouped"]             = mob->IsRaidGrouped();
		row["is_rare_spawn"]               = mob->IsRareSpawn();
		row["is_roamer"]                   = mob->IsRoamer();
		row["is_rooted"]                   = mob->IsRooted();
		row["is_running"]                  = mob->IsRunning();
		row["is_silenced"]                 = mob->IsSilenced();
		row["is_stunned"]                  = mob->IsStunned();
		row["is_tar_lock_pet"]             = mob->IsTargetLockPet();
		row["is_tarable"]                  = mob->IsTargetable();
		row["is_tared"]                    = mob->IsTargeted();
		row["is_temp_pet"]                 = mob->IsTempPet();
		row["is_trackable"]                = mob->IsTrackable();
		row["item_hp_bonuses"]             = mob->GetItemHPBonuses();
		row["last_name"]                   = mob->GetLastName();
		row["level"]                       = mob->GetLevel();
		row["luclin_face"]                 = mob->GetLuclinFace();
		row["mana"]                        = mob->GetMana();
		row["mana_percent"]                = mob->GetManaPercent();
		row["mana_ratio"]                  = mob->GetManaRatio();
		row["max_hp"]                      = mob->GetMaxHP();
		row["max_mana"]                    = mob->GetMaxMana();
		row["melee_mitigation"]            = mob->GetMeleeMitigation();
		row["mitigation_ac"]               = mob->GetMitigationAC();
		row["movespeed"]                   = mob->GetMovespeed();
		row["name"]                        = mob->GetName();
		row["next_hp_event"]               = mob->GetNextHPEvent();
		row["next_inc_hp_event"]           = mob->GetNextIncHPEvent();
		row["npc_assist_cap"]              = mob->NPCAssistCap();
		row["npc_type_id"]                 = mob->GetNPCTypeID();
		row["orig_level"]                  = mob->GetOrigLevel();
		row["orig_name"]                   = mob->GetOrigName();
		row["owner_id"]                    = mob->GetOwnerID();
		row["pet_id"]                      = mob->GetPetID();
		row["pet_power"]                   = mob->GetPetPower();
		row["pet_tar_lock_id"]             = mob->GetPetTargetLockID();
		row["qglobal"]                     = mob->GetQglobal();
		row["race"]                        = mob->GetRace();
		row["run_anim_speed"]              = mob->GetRunAnimSpeed();
		row["sanctuary"]                   = mob->Sanctuary();
		row["see_hide"]                    = mob->SeeHide();
		row["see_improved_hide"]           = mob->SeeImprovedHide();
		row["see_invisible"]               = mob->SeeInvisible();
		row["see_invisible_undead"]        = mob->SeeInvisibleUndead();
		row["size"]                        = mob->GetSize();
		row["slow_mitigation"]             = mob->GetSlowMitigation();
		row["snared_amount"]               = mob->GetSnaredAmount();
		row["spawned"]                     = mob->Spawned();
		row["spell_hp_bonuses"]            = mob->GetSpellHPBonuses();
		row["spell_light_type"]            = mob->GetSpellLightType();
		row["spell_power_distance_mod"]    = mob->GetSpellPowerDistanceMod();
		row["spell_x"]                     = mob->GetSpellX();
		row["spell_y"]                     = mob->GetSpellY();
		row["spell_z"]                     = mob->GetSpellZ();
		row["tar_ring_x"]                  = mob->GetTargetRingX();
		row["tar_ring_y"]                  = mob->GetTargetRingY();
		row["tar_ring_z"]                  = mob->GetTargetRingZ();
		row["temp_pet_count"]              = mob->GetTempPetCount();
		row["texture"]                     = mob->GetTexture();
		row["trap_slots"]                  = mob->GetTrapSlots();
		row["try_death_save"]              = mob->TryDeathSave();
		row["try_divine_save"]             = mob->TryDivineSave();
		row["try_spell_on_death"]          = mob->TrySpellOnDeath();
		row["update_active_light"]         = mob->UpdateActiveLight();
		row["wander_type"]                 = mob->GetWanderType();

		response.append(row);
	}

	return response;
}

Json::Value ApiGetClientListDetail(EQ::Net::WebsocketServerConnection *connection, Json::Value params)
{
	if (zone->GetZoneID() == 0) {
		throw EQ::Net::WebsocketException("Zone must be loaded to invoke this call");
	}

	Json::Value response;
	auto        &list = entity_list.GetClientList();

	for (auto &iter : list) {
		auto client = iter.second;

		Json::Value row;

		/**
		 * Main
		 */
		row["id"]         = client->GetID();
		row["clean_name"] = client->GetCleanName();
		row["x"]          = client->GetX();
		row["y"]          = client->GetY();
		row["z"]          = client->GetZ();
		row["heading"]    = client->GetHeading();

		/**
		 * Rest
		 */
		row["aa_percent"]                              = client->GetAAPercent();
		row["aa_points"]                               = client->GetAAPoints();
		row["aaxp"]                                    = client->GetAAXP();
		row["account_age"]                             = client->GetAccountAge();
		row["account_creation"]                        = client->GetAccountCreation();
		row["account_id"]                              = client->AccountID();
		row["account_name"]                            = client->AccountName();
		row["act_agi"]                                 = client->GetActAGI();
		row["act_cha"]                                 = client->GetActCHA();
		row["act_dex"]                                 = client->GetActDEX();
		row["act_int"]                                 = client->GetActINT();
		row["act_sta"]                                 = client->GetActSTA();
		row["act_str"]                                 = client->GetActSTR();
		row["act_wis"]                                 = client->GetActWIS();
		row["active_task_count"]                       = client->GetActiveTaskCount();
		row["admin"]                                   = client->Admin();
		row["aggro_count"]                             = client->GetAggroCount();
		row["aggro_meter_available"]                   = client->AggroMeterAvailable();
		row["all_money"]                               = client->GetAllMoney();
		row["anon"]                                    = client->GetAnon();
		row["atk_rating"]                              = client->GetATKRating();
		row["auto_attack_enabled"]                     = client->AutoAttackEnabled();
		row["auto_fire_enabled"]                       = client->AutoFireEnabled();
		row["auto_split_enabled"]                      = client->AutoSplitEnabled();
		row["base_agi"]                                = client->GetBaseAGI();
		row["base_beard"]                              = client->GetBaseBeard();
		row["base_beard_color"]                        = client->GetBaseBeardColor();
		row["base_cha"]                                = client->GetBaseCHA();
		row["base_class"]                              = client->GetBaseClass();
		row["base_corrup"]                             = client->GetBaseCorrup();
		row["base_details"]                            = client->GetBaseDetails();
		row["base_dex"]                                = client->GetBaseDEX();
		row["base_eye_color"]                          = client->GetBaseEyeColor();
		row["base_face"]                               = client->GetBaseFace();
		row["base_gender"]                             = client->GetBaseGender();
		row["base_hair_color"]                         = client->GetBaseHairColor();
		row["base_hair_style"]                         = client->GetBaseHairStyle();
		row["base_heritage"]                           = client->GetBaseHeritage();
		row["base_hp"]                                 = client->GetBaseHP();
		row["base_int"]                                = client->GetBaseINT();
		row["base_ph_r"]                               = client->GetBasePhR();
		row["base_race"]                               = client->GetBaseRace();
		row["base_sta"]                                = client->GetBaseSTA();
		row["base_str"]                                = client->GetBaseSTR();
		row["base_tattoo"]                             = client->GetBaseTattoo();
		row["base_wis"]                                = client->GetBaseWIS();
		row["become_npc_level"]                        = client->GetBecomeNPCLevel();
		row["boat_id"]                                 = client->GetBoatID();
		row["calc_atk"]                                = client->CalcATK();
		row["calc_base_mana"]                          = client->CalcBaseMana();
		row["calc_current_weight"]                     = client->CalcCurrentWeight();
		row["calc_endurance_regen_cap"]                = client->CalcEnduranceRegenCap();
		row["calc_hp_regen_cap"]                       = client->CalcHPRegenCap();
		row["calc_mana_regen_cap"]                     = client->CalcManaRegenCap();
		row["calc_max_mana"]                           = client->CalcMaxMana();
		row["can_fast_regen"]                          = client->CanFastRegen();
		row["can_fish"]                                = client->CanFish();
		row["can_med_on_horse"]                        = client->CanMedOnHorse();
		row["carried_money"]                           = client->GetCarriedMoney();
		row["char_max_level_from_bucket"]              = client->GetCharMaxLevelFromBucket();
		row["char_max_level_from_q_global"]            = client->GetCharMaxLevelFromQGlobal();
		row["character_id"]                            = client->CharacterID();
		row["check_can_unsuspend_merc"]                = client->CheckCanUnsuspendMerc();
		row["check_double_attack"]                     = client->CheckDoubleAttack();
		row["check_double_ranged_attack"]              = client->CheckDoubleRangedAttack();
		row["check_dual_wield"]                        = client->CheckDualWield();
		row["check_trade_non_droppable"]               = client->CheckTradeNonDroppable();
		row["check_triple_attack"]                     = client->CheckTripleAttack();
		row["client_max_level"]                        = client->GetClientMaxLevel();
		row["client_version_bit"]                      = client->ClientVersionBit();
		row["connected"]                               = client->Connected();
		row["copper"]                                  = client->GetCopper();
		row["corpse_count"]                            = client->GetCorpseCount();
		row["duel_tar"]                                = client->GetDuelTarget();
		row["ebon_crystals"]                           = client->GetEbonCrystals();
		row["endurance"]                               = client->GetEndurance();
		row["endurance_percent"]                       = client->GetEndurancePercent();
		row["exp"]                                     = client->GetEXP();
		row["face"]                                    = client->GetFace();
		row["feigned"]                                 = client->GetFeigned();
		row["gm"]                                      = client->GetGM();
		row["gm_speed"]                                = client->GetGMSpeed();
		row["gold"]                                    = client->GetGold();
		row["group_exp"]                               = client->GetGroupEXP();
		row["group_leadership_aa_health_enhancement"]  = client->GroupLeadershipAAHealthEnhancement();
		row["group_leadership_aa_health_regeneration"] = client->GroupLeadershipAAHealthRegeneration();
		row["group_leadership_aa_mana_enhancement"]    = client->GroupLeadershipAAManaEnhancement();
		row["group_leadership_aa_offense_enhancement"] = client->GroupLeadershipAAOffenseEnhancement();
		row["group_points"]                            = client->GetGroupPoints();
		row["guild_id"]                                = client->GuildID();
		row["guild_rank"]                              = client->GuildRank();
		row["has_adventure_data"]                      = client->HasAdventureData();
		row["hide_me"]                                 = client->GetHideMe();
		row["horse_id"]                                = client->GetHorseId();
		row["hunger"]                                  = client->GetHunger();
		row["hungry"]                                  = client->Hungry();
		row["in_zone"]                                 = client->InZone();
		row["instance_id"]                             = client->GetInstanceID();
		row["interrogate_inv_state"]                   = client->GetInterrogateInvState();
		row["ip"]                                      = client->GetIP();
		row["is_become_npc"]                           = client->IsBecomeNPC();
		row["is_buyer"]                                = client->IsBuyer();
		row["is_dead"]                                 = client->IsDead();
		row["is_dragging_corpse"]                      = client->IsDraggingCorpse();
		row["is_dueling"]                              = client->IsDueling();
		row["is_guild_banker"]                         = client->IsGuildBanker();
		row["is_hovering_for_respawn"]                 = client->IsHoveringForRespawn();
		row["is_in_a_guild"]                           = client->IsInAGuild();
		row["is_ld"]                                   = client->IsLD();
		row["is_leadership_exp_on"]                    = client->IsLeadershipEXPOn();
		row["is_lfp"]                                  = client->IsLFP();
		row["is_medding"]                              = client->IsMedding();
		row["is_on_adventure"]                         = client->IsOnAdventure();
		row["is_rezz_pending"]                         = client->IsRezzPending();
		row["is_sitting"]                              = client->IsSitting();
		row["is_starved"]                              = client->IsStarved();
		row["is_tracking"]                             = client->IsTracking();
		row["is_trader"]                               = client->IsTrader();
		row["is_unconscious"]                          = client->IsUnconscious();
		row["ldon_losses"]                             = client->GetLDoNLosses();
		row["ldon_wins"]                               = client->GetLDoNWins();
		row["last_inv_snapshot_time"]                  = client->GetLastInvSnapshotTime();
		row["last_name"]                               = client->GetLastName();
		row["level2"]                                  = client->GetLevel2();
		row["level_regen"]                             = client->LevelRegen();
		row["ls_account_id"]                           = client->LSAccountID();
		row["max_endurance"]                           = client->GetMaxEndurance();
		row["max_x_tars"]                              = client->GetMaxXTargets();
		row["merc_id"]                                 = client->GetMercenaryID();
		row["merc_only_or_no_group"]                   = client->MercOnlyOrNoGroup();
		row["merc_slot"]                               = client->GetMercSlot();
		row["next_inv_snapshot_time"]                  = client->GetNextInvSnapshotTime();
		row["num_mercs"]                               = client->GetNumberOfMercenaries();
		row["pending_adventure_create"]                = client->GetPendingAdventureCreate();
		row["pending_adventure_door_click"]            = client->GetPendingAdventureDoorClick();
		row["pending_adventure_leave"]                 = client->GetPendingAdventureLeave();
		row["pending_adventure_request"]               = client->GetPendingAdventureRequest();
		row["pending_guild_invitation"]                = client->GetPendingGuildInvitation();
		row["platinum"]                                = client->GetPlatinum();
		row["port"]                                    = client->GetPort();
		row["primary_skill_value"]                     = client->GetPrimarySkillValue();
		row["proximity_x"]                             = client->ProximityX();
		row["proximity_y"]                             = client->ProximityY();
		row["proximity_z"]                             = client->ProximityZ();
		row["pvp_points"]                              = client->GetPVPPoints();
		row["radiant_crystals"]                        = client->GetRadiantCrystals();
		row["raid_exp"]                                = client->GetRaidEXP();
		row["raid_points"]                             = client->GetRaidPoints();
		row["raw_item_ac"]                             = client->GetRawItemAC();
		row["required_aa_experience"]                  = client->GetRequiredAAExperience();
		row["revoked"]                                 = client->GetRevoked();
		row["run_mode"]                                = client->GetRunMode();
		row["save_currency"]                           = client->SaveCurrency();
		row["save_task_state"]                         = client->SaveTaskState();
		row["silver"]                                  = client->GetSilver();
		row["skill_points"]                            = client->GetSkillPoints();
		row["spent_aa"]                                = client->GetSpentAA();
		row["tgb"]                                     = client->TGB();
		row["thirst"]                                  = client->GetThirst();
		row["thirsty"]                                 = client->Thirsty();
		row["total_atk"]                               = client->GetTotalATK();
		row["total_seconds_played"]                    = client->GetTotalSecondsPlayed();
		row["weight"]                                  = client->GetWeight();
		row["wid"]                                     = client->GetWID();
		row["x_tarting_available"]                     = client->XTargettingAvailable();

		response.append(row);
	}

	return response;
}

Json::Value ApiGetZoneAttributes(EQ::Net::WebsocketServerConnection *connection, Json::Value params)
{
	if (zone->GetZoneID() == 0) {
		throw EQ::Net::WebsocketException("Zone must be loaded to invoke this call");
	}

	Json::Value response;
	Json::Value row;

	row["aggro_limit_reached"]     = zone->AggroLimitReached();
	row["allow_mercs"]             = zone->AllowMercs();
	row["buff_timers_suspended"]   = zone->BuffTimersSuspended();
	row["can_bind"]                = zone->CanBind();
	row["can_cast_outdoor"]        = zone->CanCastOutdoor();
	row["can_do_combat"]           = zone->CanDoCombat();
	row["can_levitate"]            = zone->CanLevitate();
	row["count_auth"]              = zone->CountAuth();
	row["count_spawn2"]            = zone->CountSpawn2();
	row["file_name"]               = zone->GetFileName();
	row["graveyard_id"]            = zone->graveyard_id();
	row["graveyard_zoneid"]        = zone->graveyard_zoneid();
	row["has_graveyard"]           = zone->HasGraveyard();
	row["has_map"]                 = zone->HasMap();
	row["has_water_map"]           = zone->HasWaterMap();
	row["has_weather"]             = zone->HasWeather();
	row["instance_id"]             = zone->GetInstanceID();
	row["instance_version"]        = zone->GetInstanceVersion();
	row["instant_grids"]           = zone->InstantGrids();
	row["is_city"]                 = zone->IsCity();
	row["is_hotzone"]              = zone->IsHotzone();
	row["is_instance_persistent"]  = zone->IsInstancePersistent();
	row["is_pvp_zone"]             = zone->IsPVPZone();
	row["is_static_zone"]          = zone->IsStaticZone();
	row["is_ucs_server_available"] = zone->IsUCSServerAvailable();
	row["long_name"]               = zone->GetLongName();
	row["max_clients"]             = zone->GetMaxClients();
	row["mobs_aggro_count"]        = zone->MobsAggroCount();
	row["save_zone_cfg"]           = zone->SaveZoneCFG();
	row["short_name"]              = zone->GetShortName();
	row["total_blocked_spells"]    = zone->GetZoneTotalBlockedSpells();
	row["zone_id"]                 = zone->GetZoneID();
	row["zone_type"]               = zone->GetZoneType();

	response.append(row);
	return response;
}

Json::Value ApiGetLogsysCategories(EQ::Net::WebsocketServerConnection *connection, Json::Value params)
{
	if (!zone || (zone && zone->GetZoneID() == 0)) {
		throw EQ::Net::WebsocketException("Zone must be loaded to invoke this call");
	}

	Json::Value response;

	for (int i = 1; i < Logs::LogCategory::MaxCategoryID; i++) {
		Json::Value row;

		row["log_category_id"]          = i;
		row["log_category_description"] = Logs::LogCategoryName[i];
		row["log_to_console"]           = EQEmuLogSys::Instance()->log_settings[i].log_to_console;
		row["log_to_file"]              = EQEmuLogSys::Instance()->log_settings[i].log_to_file;
		row["log_to_gmsay"]             = EQEmuLogSys::Instance()->log_settings[i].log_to_gmsay;

		response.append(row);
	}

	return response;
}

Json::Value ApiSetLoggingLevel(EQ::Net::WebsocketServerConnection *connection, Json::Value params)
{
	if (zone->GetZoneID() == 0) {
		throw EQ::Net::WebsocketException("Zone must be loaded to invoke this call");
	}

	Json::Value response;

	auto logging_category = params[0].asInt();
	auto logging_level    = params[1].asInt();

	response["status"] = "Category doesn't exist";

	LogInfo("Logging category is [{}] and level is [{}]",
		logging_category,
		logging_level
	);

	if (logging_category < Logs::LogCategory::MaxCategoryID &&
		logging_category > Logs::LogCategory::None
		) {
		EQEmuLogSys::Instance()->log_settings[logging_category].log_to_console = logging_level;
		response["status"] = "Category log level updated";
	}

	if (logging_level > 0) {
		EQEmuLogSys::Instance()->log_settings[logging_category].is_category_enabled = 1;
	}
	else {
		EQEmuLogSys::Instance()->log_settings[logging_category].is_category_enabled = 0;
	}

	return response;
}

void RegisterApiLogEvent(std::unique_ptr<EQ::Net::WebsocketServer> &server)
{
	EQEmuLogSys::Instance()->SetConsoleHandler(
		[&](uint16 log_category, const std::string &msg) {
			Json::Value data;
			data["log_category"] = log_category;
			data["msg"]          = msg;
			server->DispatchEvent(EQ::Net::SubscriptionEventLog, data, 50);
		}
	);
}

void RegisterApiService(std::unique_ptr<EQ::Net::WebsocketServer> &server)
{
	server->SetLoginHandler(CheckLogin);
	server->SetMethodHandler("get_packet_statistics", &ApiGetPacketStatistics, 50);
	server->SetMethodHandler("get_opcode_list", &ApiGetOpcodeList, 50);
	server->SetMethodHandler("get_npc_list_detail", &ApiGetNpcListDetail, 50);
	server->SetMethodHandler("get_door_list_detail", &ApiGetDoorListDetail, 50);
	server->SetMethodHandler("get_corpse_list_detail", &ApiGetCorpseListDetail, 50);
	server->SetMethodHandler("get_object_list_detail", &ApiGetObjectListDetail, 50);
	server->SetMethodHandler("get_mob_list_detail", &ApiGetMobListDetail, 50);
	server->SetMethodHandler("get_client_list_detail", &ApiGetClientListDetail, 50);
	server->SetMethodHandler("get_zone_attributes", &ApiGetZoneAttributes, 50);
	server->SetMethodHandler("get_logsys_categories", &ApiGetLogsysCategories, 50);
	server->SetMethodHandler("set_logging_level", &ApiSetLoggingLevel, 50);

	RegisterApiLogEvent(server);
}
