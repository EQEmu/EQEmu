/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2005 EQEMu Development Team (http://eqemulator.net)

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

#include "client.h"

#include "common/eqemu_logsys.h"
#include "common/events/player_event_logs.h"
#include "common/repositories/character_peqzone_flags_repository.h"
#include "common/repositories/zone_flags_repository.h"
#include "common/rulesys.h"
#include "common/strings.h"
#include "zone/bot.h"
#include "zone/dynamic_zone.h"
#include "zone/queryserv.h"
#include "zone/quest_parser_collection.h"
#include "zone/string_ids.h"
#include "zone/worldserver.h"
#include "zone/zone.h"

extern QueryServ* QServ;
extern WorldServer worldserver;
extern Zone* zone;

void Client::Handle_OP_ZoneChange(const EQApplicationPacket *app) {
	if (RuleB(Bots, Enabled)) {
		Bot::ProcessClientZoneChange(this);
	}

	bZoning = true;
	if (app->size != sizeof(ZoneChange_Struct)) {
		LogDebug("Wrong size: OP_ZoneChange, size=[{}], expected [{}]", app->size, sizeof(ZoneChange_Struct));
		return;
	}

#if EQDEBUG >= 5
	LogDebug("Zone request from [{}]", GetName());
	DumpPacket(app);
#endif
	auto* zc = (ZoneChange_Struct*)app->pBuffer;

	LogZoning(
		"Client [{}] char_name [{}] zoning to [{}] ({}) instance_id [{}] x [{}] y [{}] z [{}] zone_reason [{}] success [{}] zone_mode [{}] ({})",
		GetCleanName(),
		zc->char_name,
		ZoneName(zc->zoneID) ? ZoneName(zc->zoneID) : "Unknown",
		zc->zoneID,
		zc->instanceID,
		zc->x,
		zc->y,
		zc->z,
		zc->zone_reason,
		zc->success,
		GetZoneModeString(zone_mode),
		int(zone_mode)
	);

	WorldContentService::Instance()->HandleZoneRoutingMiddleware(zc);

	uint16 target_zone_id = 0;
	auto target_instance_id = zc->instanceID;
	ZonePoint* zone_point = nullptr;

	//figure out where they are going.
	if (zc->zoneID == 0) {
		//client dosent know where they are going...
		//try to figure it out for them.

		switch(zone_mode) {
			case EvacToSafeCoords:
			case ZoneToSafeCoords:
				//going to safe coords, but client dosent know where?
				//assume it is this zone for now.
				target_zone_id = zone->GetZoneID();
				break;
			case GMSummon:
			case GMHiddenSummon:
			case ZoneSolicited: //we told the client to zone somewhere, so we know where they are going.
				target_zone_id = zonesummon_id;
				break;
			case GateToBindPoint:
			case ZoneToBindPoint:
				target_zone_id = m_pp.binds[0].zone_id;
				target_instance_id = m_pp.binds[0].instance_id;
				break;
			case ZoneUnsolicited: //client came up with this on its own.
				zone_point = zone->GetClosestZonePointWithoutZone(GetX(), GetY(), GetZ(), this, ZONEPOINT_NOZONE_RANGE);
				if (zone_point) {
					//we found a zone point, which is a reasonable distance away
					//assume that is the one were going with.
					target_zone_id = zone_point->target_zone_id;
					target_instance_id = zone_point->target_zone_instance;
				} else {
					//unable to find a zone point... is there anything else
					//that can be a valid un-zolicited zone request?

					Message(Chat::Red, "Invalid unsolicited zone request.");
					LogError("Zoning [{}]: Invalid unsolicited zone request to zone id [{}]", GetName(), target_zone_id);
					cheat_manager.CheatDetected(GetBindZoneID() == target_zone_id ? MQGate : MQZone, glm::vec3(zc->x, zc->y, zc->z));
					SendZoneCancel(zc);
					return;
				}
				break;
			default:
				break;
		};
	} else {
		// This is to allow both 6.2 and Titanium clients to perform a proper zoning of the client when evac/succor
		// WildcardX 27 January 2008
		if (zone_mode == EvacToSafeCoords && zonesummon_id) {
			target_zone_id = zonesummon_id;
		} else {
			target_zone_id = zc->zoneID;
		}

		//if we are zoning to a specific zone unsolicied,
		//then until otherwise determined, they must be zoning
		//on a zone line.
		if (zone_mode == ZoneUnsolicited) {
			if (target_zone_id == zone->GetZoneID()) {
				SendZoneCancel(zc);
				return;
			}

			zone_point = zone->GetClosestZonePoint(glm::vec3(GetPosition()), target_zone_id, this, ZONEPOINT_ZONE_RANGE);
			//if we didnt get a zone point, or its to a different zone,
			//then we assume this is invalid.
			if (!zone_point || zone_point->target_zone_id != target_zone_id) {
				LogError("Zoning [{}]: Invalid unsolicited zone request to zone id [{}]", GetName(), target_zone_id);
				cheat_manager.CheatDetected(GetBindZoneID() == target_zone_id ? MQGate : MQZone, glm::vec3(zc->x, zc->y, zc->z));
				SendZoneCancel(zc);
				return;
			}
		}
	}

	if (target_instance_id) {
		LogZoning(
			"Client [{}] attempting zone to instance_id [{}] zone [{}] ({})",
			GetCleanName(),
			target_instance_id,
			ZoneName(target_zone_id) ? ZoneName(target_zone_id) : "Unknown",
			target_zone_id
		);

		//make sure we are in it and it's unexpired.
		if (!database.VerifyInstanceAlive(target_instance_id, CharacterID())) {
			Message(
				Chat::Red,
				fmt::format(
					"Instance ID {} was expired or you were not a member of it.",
					target_instance_id
				).c_str()
			);
			LogZoning("Client [{}] Invalid zone instance request to zone id [{}]", GetCleanName(), target_zone_id);
			SendZoneCancel(zc);
			return;
		}

		if (!database.VerifyZoneInstance(target_zone_id, target_instance_id)) {
			Message(
				Chat::Red,
				fmt::format(
					"Instance ID was {}, this does not match Zone ID {}.",
					target_instance_id,
					target_zone_id
				).c_str()
			);
			LogZoning("Client [{}] Invalid zone instance request to zone id [{}]", GetCleanName(), target_zone_id);
			SendZoneCancel(zc);
			return;
		}
	}

	/* Check for Valid Zone */
	auto* target_zone_name = ZoneName(target_zone_id);
	if (!target_zone_name) {
		//invalid zone...
		Message(Chat::Red, "Invalid target zone ID.");
		LogError("Zoning [{}]: Unable to get zone name for zone id [{}]", GetName(), target_zone_id);
		SendZoneCancel(zc);
		return;
	}

	auto target_instance_version = database.GetInstanceVersion(target_instance_id);
	auto zone_data = GetZoneVersionWithFallback(
		ZoneID(target_zone_name),
		target_instance_version
	);
	if (!zone_data) {
		Message(Chat::Red, "Invalid target zone while getting safe points.");
		LogError("Zoning [{}]: Unable to get safe coordinates for zone [{}]", GetName(), target_zone_name);
		SendZoneCancel(zc);
		return;
	}

	float safe_x, safe_y, safe_z, safe_heading;
	int16 min_status = AccountStatus::Player;
	uint8 min_level  = 0;

	LogZoning("Loaded zone flag [{}]", zone_data->flag_needed);

	safe_x       = zone_data->safe_x;
	safe_y       = zone_data->safe_y;
	safe_z       = zone_data->safe_z;
	safe_heading = zone_data->safe_heading;
	min_status   = zone_data->min_status;
	min_level    = zone_data->min_level;

	if (parse->PlayerHasQuestSub(EVENT_ZONE)) {
		const auto& export_string = fmt::format(
			"{} {} {} {} {} {}",
			zone->GetZoneID(),
			zone->GetInstanceID(),
			zone->GetInstanceVersion(),
			target_zone_id,
			target_instance_id,
			target_instance_version
		);

		if (parse->EventPlayer(EVENT_ZONE, this, export_string, 0) != 0) {
			SendZoneCancel(zc);
			return;
		}
	}

	if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::ZONING)) {
		auto e = PlayerEvent::ZoningEvent{};
		e.from_zone_long_name   = zone->GetLongName();
		e.from_zone_short_name  = zone->GetShortName();
		e.from_zone_id          = zone->GetZoneID();
		e.from_instance_id      = zone->GetInstanceID();
		e.from_instance_version = zone->GetInstanceVersion();
		e.to_zone_long_name     = ZoneLongName(target_zone_id);
		e.to_zone_short_name    = ZoneName(target_zone_id);
		e.to_zone_id            = target_zone_id;
		e.to_instance_id        = target_instance_id;
		e.to_instance_version   = target_instance_version;

		RecordPlayerEventLog(PlayerEvent::ZONING, e);
	}

	//handle circumvention of zone restrictions
	//we need the value when creating the outgoing packet as well.
	uint8 ignore_restrictions = zonesummon_ignorerestrictions;
	zonesummon_ignorerestrictions = 0;

	auto target_x = 0.0f, target_y = 0.0f, target_z = 0.0f, target_heading = 0.0f;
	switch (zone_mode) {
		case EvacToSafeCoords:
		case ZoneToSafeCoords:
			LogDebug(
				"Zoning [{}] to safe coords ([{}], [{}], [{}], [{}]) in [{}] ([{}])",
				GetName(),
				safe_x,
				safe_y,
				safe_z,
				safe_heading,
				target_zone_name,
				target_zone_id
			);
			target_x = safe_x;
			target_y = safe_y;
			target_z = safe_z;
			target_heading = safe_heading;
			break;
		case GMSummon:
		case GMHiddenSummon:
			target_x = m_ZoneSummonLocation.x;
			target_y = m_ZoneSummonLocation.y;
			target_z = m_ZoneSummonLocation.z;
			target_heading = m_ZoneSummonLocation.w;
			ignore_restrictions = 1;
			break;
		case GateToBindPoint:
			target_x = m_pp.binds[0].x;
			target_y = m_pp.binds[0].y;
			target_z = m_pp.binds[0].z;
			target_heading = m_pp.binds[0].heading;
			break;
		case ZoneToBindPoint:
			target_x = m_pp.binds[0].x;
			target_y = m_pp.binds[0].y;
			target_z = m_pp.binds[0].z;
			target_heading = m_pp.binds[0].heading;
			ignore_restrictions = 1;	//can always get to our bind point? seems exploitable
			break;
		case ZoneSolicited: //we told the client to zone somewhere, so we know where they are going.
			//recycle zonesummon variables
			target_x = m_ZoneSummonLocation.x;
			target_y = m_ZoneSummonLocation.y;
			target_z = m_ZoneSummonLocation.z;
			target_heading = m_ZoneSummonLocation.w;
			break;
		case ZoneUnsolicited: //client came up with this on its own.
			//client requested a zoning... what are the cases when this could happen?

			//Handle zone point case:
			if (zone_point) {
				//they are zoning using a valid zone point, figure out coords

				//999999 is a placeholder for 'same as where they were from'
				target_x = zone_point->target_x == 999999 ? GetX() : zone_point->target_x;
				target_y = zone_point->target_y == 999999 ? GetY() : zone_point->target_y;
				target_z = zone_point->target_z == 999999 ? GetZ() : zone_point->target_z;
				target_heading = zone_point->target_heading == 999 ? GetHeading() : zone_point->target_heading;
				break;
			}

			//for now, there are no other cases...

			//could not find a valid reason for them to be zoning, stop it.
			cheat_manager.CheatDetected(GetBindZoneID() == target_zone_id ? MQGate : MQZone, glm::vec3(zc->x, zc->y, zc->z));

			LogError("Zoning [{}]: Invalid unsolicited zone request to zone id [{}]. Not near a zone point", GetName(), target_zone_name);
			SendZoneCancel(zc);
			return;
		default:
			break;
	};

	auto zoning_message = ZoningMessage::ZoneSuccess;

	// Check Minimum Status, Minimum Level, Maximum Level, and Zone Flag
	if (
		!ignore_restrictions &&
		!CanEnterZone(ZoneName(target_zone_id), target_instance_version)
	) {
		zoning_message = ZoningMessage::ZoneNoExperience;
	}

	//TODO: ADVENTURE ENTRANCE CHECK

	// Expansion checks and routing
	if (WorldContentService::Instance()->GetCurrentExpansion() >= Expansion::Classic && !GetGM()) {
		bool meets_zone_expansion_check = false;

		auto z = ZoneStore::Instance()->GetZoneWithFallback(ZoneID(target_zone_name), 0);
		if (z->expansion <= WorldContentService::Instance()->GetCurrentExpansion() || z->bypass_expansion_check) {
			meets_zone_expansion_check = true;
		}

		LogZoning(
			"Checking zone request [{}] for expansion [{}] ({}) success [{}]",
			target_zone_name,
			(WorldContentService::Instance()->GetCurrentExpansion()),
			WorldContentService::Instance()->GetCurrentExpansionName(),
			meets_zone_expansion_check ? "true" : "false"
		);

		if (!meets_zone_expansion_check) {
			zoning_message = ZoningMessage::ZoneNoExpansion;
		}
	}

	if (WorldContentService::Instance()->GetCurrentExpansion() >= Expansion::Classic && GetGM()) {
		LogInfo("[{}] Bypassing zone expansion checks because GM flag is set", GetCleanName());
		Message(Chat::White, "Your GM flag allows you to bypass zone expansion checks.");
	}

	if (zoning_message == ZoningMessage::ZoneSuccess) {
		DoZoneSuccess(zc, target_zone_id, target_instance_id, target_x, target_y, target_z, target_heading, ignore_restrictions);
	} else {
		LogError("Zoning [{}]: Rules prevent this char from zoning into [{}]", GetName(), target_zone_name);
		SendZoneError(zc, zoning_message);
	}
}

void Client::SendZoneCancel(ZoneChange_Struct *zc) {
	//effectively zone them right back to where they were
	//unless we find a better way to stop the zoning process.
	cheat_manager.SetExemptStatus(Port, true);
	EQApplicationPacket *outapp = nullptr;
	outapp = new EQApplicationPacket(OP_ZoneChange, sizeof(ZoneChange_Struct));
	ZoneChange_Struct *zc2 = (ZoneChange_Struct*)outapp->pBuffer;

	LogZoning(
		"(zs) Client [{}] char_name [{}] zoning to [{}] ({}) cancelled instance_id [{}] x [{}] y [{}] z [{}] zone_reason [{}] success [{}]",
		GetCleanName(),
		zc->char_name,
		ZoneName(zc2->zoneID) ? ZoneName(zc2->zoneID) : "Unknown",
		zc->zoneID,
		zc->instanceID,
		zc->x,
		zc->y,
		zc->z,
		zc->zone_reason,
		zc->success
	);

	strn0cpy(zc2->char_name, zc->char_name, 64);
	zc2->zoneID = zone->GetZoneID();
	zc2->success = 1;
	zc2->instanceID = zone->GetInstanceID();

	// this fixes an issue where when we do a zone cancel what often ends up happening is we are sending
	// the client the wrong coordinates to zone back to. Often times it is the x,y,z of the destination zone
	// because we saved the destination x,y,z on the client profile before we rejected the zone request.
	// we're using rewind location because it should be where the client relatively was before we rejected the zone request.
	// it also prevents the client from getting caught up in a zone loop because if we sent them exactly back to where they
	// originated the request we could end up in a situation where the client is caught in a zone loop.
	m_Position.x = m_RewindLocation.x;
	m_Position.y = m_RewindLocation.y;
	m_Position.z = m_RewindLocation.z;
	zc2->x       = m_Position.x;
	zc2->y       = m_Position.y;
	zc2->z       = m_Position.z;

	LogZoning(
		"(zc2) Client [{}] char_name [{}] zoning to [{}] ({}) cancelled instance_id [{}] x [{}] y [{}] z [{}] zone_reason [{}] success [{}]",
		GetCleanName(),
		zc2->char_name,
		ZoneName(zc2->zoneID) ? ZoneName(zc2->zoneID) : "Unknown",
		zc2->zoneID,
		zc2->instanceID,
		zc2->x,
		zc2->y,
		zc2->z,
		zc2->zone_reason,
		zc2->success
	);

	outapp->priority = 6;
	FastQueuePacket(&outapp);

	//reset to unsolicited.
	zone_mode = ZoneUnsolicited;
	// reset since we're not zoning anymore
	bZoning = false;
	// remove save position lock
	m_lock_save_position = false;
}

void Client::SendZoneError(ZoneChange_Struct *zc, int8 err)
{
	LogError("Zone [{}] is not available because target wasn't found or character insufficent level", zc->zoneID);
	cheat_manager.SetExemptStatus(Port, true);
	EQApplicationPacket *outapp = nullptr;
	outapp = new EQApplicationPacket(OP_ZoneChange, sizeof(ZoneChange_Struct));
	ZoneChange_Struct *zc2 = (ZoneChange_Struct*)outapp->pBuffer;
	strcpy(zc2->char_name, zc->char_name);
	zc2->zoneID = zc->zoneID;
	zc2->success = err;
	outapp->priority = 6;
	FastQueuePacket(&outapp);

	LogZoning(
		"Client [{}] char_name [{}] zoning to [{}] ({}) (error) instance_id [{}] x [{}] y [{}] z [{}] zone_reason [{}] success [{}]",
		GetCleanName(),
		zc2->char_name,
		ZoneName(zc2->zoneID) ? ZoneName(zc2->zoneID) : "Unknown",
		zc2->zoneID,
		zc2->instanceID,
		zc2->x,
		zc2->y,
		zc2->z,
		zc2->zone_reason,
		zc2->success
	);

	//reset to unsolicited.
	zone_mode = ZoneUnsolicited;
	// reset since we're not zoning anymore
	bZoning = false;
	// remove save position lock
	m_lock_save_position = false;
}

void Client::DoZoneSuccess(ZoneChange_Struct *zc, uint16 zone_id, uint32 instance_id, float dest_x, float dest_y, float dest_z, float dest_h, int8 ignore_r) {
	//this is called once the client is fully allowed to zone here
	//it takes care of all the activities which occur when a client zones out

	SendLogoutPackets();

	/* Dont clear aggro until the zone is successful */
	entity_list.RemoveFromHateLists(this);

	if(GetPet())
		entity_list.RemoveFromHateLists(GetPet());

	if (GetPendingDzInviteID() != 0)
	{
		// live re-invites if client zoned with a pending invite, save pending invite info in world
		if (auto dz = DynamicZone::FindDynamicZoneByID(GetPendingDzInviteID(), DynamicZoneType::Expedition))
		{
			dz->SendWorldPendingInvite(m_dz_invite, GetName());
		}
	}

	LogZoning(
		"Client [{}] char_name [{}] zoning to [{}] ({}) instance_id [{}] x [{}] y [{}] z [{}] zone_reason [{}] success [{}]",
		GetCleanName(),
		zc->char_name,
		ZoneName(zone_id) ? ZoneName(zone_id) : "Unknown",
		zone_id,
		instance_id,
		dest_x,
		dest_y,
		dest_z,
		zc->zone_reason,
		zc->success
	);

	//set the player's coordinates in the new zone so they have them
	//when they zone into it
	m_Position.x = dest_x; //these coordinates will now be saved when ~client is called
	m_Position.y = dest_y;
	m_Position.z = dest_z;
	m_Position.w = dest_h; // Cripp: fix for zone heading
	m_pp.heading = dest_h;
	m_pp.zone_id = zone_id;
	m_pp.zoneInstance = instance_id;

	// save character position
	m_pp.x       = m_Position.x;
	m_pp.y       = m_Position.y;
	m_pp.z       = m_Position.z;
	m_pp.heading = m_Position.w;
	SaveCharacterData();

	m_lock_save_position = true;

	if (zone_id == zone->GetZoneID() && instance_id == zone->GetInstanceID()) {
		// No need to ask worldserver if we're zoning to ourselves (most
		// likely to a bind point), also fixes a bug since the default response was failure
		auto outapp = new EQApplicationPacket(OP_ZoneChange, sizeof(ZoneChange_Struct));
		ZoneChange_Struct* zc2 = (ZoneChange_Struct*) outapp->pBuffer;
		strcpy(zc2->char_name, GetName());
		zc2->zoneID = zone_id;
		zc2->instanceID = instance_id;
		zc2->success = 1;
		outapp->priority = 6;
		FastQueuePacket(&outapp);
	} else {
		// vesuvias - zoneing to another zone so we need to the let the world server
		//handle things with the client for a while
		auto pack = new ServerPacket(ServerOP_ZoneToZoneRequest, sizeof(ZoneToZone_Struct));
		ZoneToZone_Struct *ztz = (ZoneToZone_Struct *)pack->pBuffer;
		ztz->response = 0;
		ztz->current_zone_id = zone->GetZoneID();
		ztz->current_instance_id = zone->GetInstanceID();
		ztz->requested_zone_id = zone_id;
		ztz->requested_instance_id = instance_id;
		ztz->admin = admin;
		ztz->ignorerestrictions = ignore_r;
		strcpy(ztz->name, GetName());
		ztz->guild_id = GuildID();
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}

	//reset to unsolicited.
	zone_mode = ZoneUnsolicited;
	m_ZoneSummonLocation = glm::vec4();
	zonesummon_id = 0;
	zonesummon_ignorerestrictions = 0;

	// this simply resets the zone shutdown timer
	zone->ResetShutdownTimer();
}

void Client::MovePC(const char* zonename, float x, float y, float z, float heading, uint8 ignorerestrictions, ZoneMode zm) {
	ProcessMovePC(ZoneID(zonename), 0, x, y, z, heading, ignorerestrictions, zm);
}

//designed for in zone moving
void Client::MovePC(float x, float y, float z, float heading, uint8 ignorerestrictions, ZoneMode zm) {
	ProcessMovePC(zone->GetZoneID(), zone->GetInstanceID(), x, y, z, heading, ignorerestrictions, zm);
}

void Client::MovePC(uint32 zoneID, float x, float y, float z, float heading, uint8 ignorerestrictions, ZoneMode zm) {
	ProcessMovePC(zoneID, 0, x, y, z, heading, ignorerestrictions, zm);
}

void Client::MovePC(uint32 zoneID, uint32 instanceID, float x, float y, float z, float heading, uint8 ignorerestrictions, ZoneMode zm){
	ProcessMovePC(zoneID, instanceID, x, y, z, heading, ignorerestrictions, zm);
}

void Client::MoveZone(const char *zone_short_name, const glm::vec4 &location) {
	const bool use_coordinates = (
		location.x != 0.0f ||
		location.y != 0.0f ||
		location.z != 0.0f ||
		location.w != 0.0f
	);

	const ZoneMode zone_type = use_coordinates ? ZoneSolicited : ZoneToSafeCoords;

	ProcessMovePC(ZoneID(zone_short_name), 0, location.x, location.y, location.z, location.w, 3, zone_type);
}

void Client::MoveZoneGroup(const char *zone_short_name, const glm::vec4 &location) {
	Group* g = GetGroup();
	if (!g) {
		MoveZone(zone_short_name, location);
	} else {
		for (const auto& gm : g->members) {
			if (gm && gm->IsClient()) {
				Client* c = gm->CastToClient();
				c->MoveZone(zone_short_name, location);
			}
		}
	}
}

void Client::MoveZoneRaid(const char *zone_short_name, const glm::vec4 &location) {
	Raid* r = GetRaid();
	if (!r) {
		MoveZone(zone_short_name, location);
	} else {
		for (const auto& rm : r->members) {
			if (rm.member && rm.member->IsClient()) {
				Client* c = rm.member->CastToClient();
				c->MoveZone(zone_short_name, location);
			}
		}
	}
}

void Client::MoveZoneInstance(uint16 instance_id, const glm::vec4 &location) {
	if (!database.CheckInstanceByCharID(instance_id, CharacterID())) {
		database.AddClientToInstance(instance_id, CharacterID());
	}

	const bool use_coordinates = (
		location.x != 0.0f ||
		location.y != 0.0f ||
		location.z != 0.0f ||
		location.w != 0.0f
	);

	const ZoneMode zone_type = use_coordinates ? ZoneSolicited : ZoneToSafeCoords;

	ProcessMovePC(database.GetInstanceZoneID(instance_id), instance_id, location.x, location.y, location.z, location.w, 3, zone_type);
}

void Client::MoveZoneInstanceGroup(uint16 instance_id, const glm::vec4 &location) {
	Group* g = GetGroup();
	if (!g) {
		MoveZoneInstance(instance_id, location);
	} else {
		for (const auto& gm : g->members) {
			if (gm && gm->IsClient()) {
				Client* c = gm->CastToClient();
				c->MoveZoneInstance(instance_id, location);
			}
		}
	}
}

void Client::MoveZoneInstanceRaid(uint16 instance_id, const glm::vec4 &location) {
	Raid* r = GetRaid();
	if (!r) {
		MoveZoneInstance(instance_id, location);
	} else {
		for (const auto& rm : r->members) {
			if (rm.member && rm.member->IsClient()) {
				Client* c = rm.member->CastToClient();
				c->MoveZoneInstance(instance_id, location);
			}
		}
	}
}

void Client::ProcessMovePC(uint32 zoneID, uint32 instance_id, float x, float y, float z, float heading, uint8 ignorerestrictions, ZoneMode zm)
{
	// From what I have read, dragged corpses should stay with the player for Intra-zone summons etc, but we can
	// implement that later.
	ClearDraggedCorpses();

	// Added to ensure that if a player is moved (ported, gmmove, etc) and they are an active trader or buyer, they will
	// be removed from future transactions.
	if (IsTrader()) {
		TraderEndTrader();
	}

	if (IsBuyer()) {
		ToggleBuyerMode(false);
	}

	if(zoneID == 0)
		zoneID = zone->GetZoneID();

	if(zoneID == zone->GetZoneID() && instance_id == zone->GetInstanceID()) {
		// TODO: Determine if this condition is necessary.
		if(IsAIControlled()) {
			GMMove(x, y, z);
			return;
		}

		if(zm != SummonPC && GetPetID() != 0) {
			//if they have a pet and they are staying in zone, move with them
			Mob *p = GetPet();
			if(p != nullptr){
				p->SetPetOrder(PetOrder::Follow);
				p->GMMove(x+15, y, z);	//so it dosent have to run across the map.
			}
		}
	}

	switch(zm) {
		case GateToBindPoint:
			ZonePC(zoneID, instance_id, x, y, z, heading, ignorerestrictions, zm);
			break;
		case EvacToSafeCoords:
		case ZoneToSafeCoords:
			ZonePC(zoneID, instance_id, x, y, z, heading, ignorerestrictions, zm);
			break;
		case GMSummon:
			Message(Chat::Yellow, "You have been summoned by a GM!");
			ZonePC(zoneID, instance_id, x, y, z, heading, ignorerestrictions, zm);
			break;
		case GMHiddenSummon:
			ZonePC(zoneID, instance_id, x, y, z, heading, ignorerestrictions, zm);
			break;
		case ZoneToBindPoint:
			ZonePC(zoneID, instance_id, x, y, z, heading, ignorerestrictions, zm);
			break;
		case ZoneSolicited:
			ZonePC(zoneID, instance_id, x, y, z, heading, ignorerestrictions, zm);
			break;
		case SummonPC:
			MessageString(Chat::Yellow, PLAYER_SUMMONED);
			ZonePC(zoneID, instance_id, x, y, z, heading, ignorerestrictions, zm);
			break;
		case Rewind:
			Message(Chat::Yellow, "Rewinding to previous location.");
			ZonePC(zoneID, instance_id, x, y, z, heading, ignorerestrictions, zm);
			break;
		default:
			LogError("Received a request to perform an unsupported client zone operation");
			break;
	}
}

void Client::ZonePC(uint32 zoneID, uint32 instance_id, float x, float y, float z, float heading, uint8 ignorerestrictions, ZoneMode zm) {
	bool ReadyToZone = true;
	int iZoneNameLength = 0;
	const char*	pShortZoneName = nullptr;
	char* pZoneName = nullptr;

	pShortZoneName = ZoneName(zoneID);

	auto zd = GetZoneVersionWithFallback(zoneID, zone->GetInstanceVersion());
	if (zd) {
		pZoneName = strcpy(new char[zd->long_name.length() + 1], zd->long_name.c_str());
	}

	auto r = WorldContentService::Instance()->FindZone(zoneID, instance_id);
	if (r.zone_id) {
		zoneID      = r.zone_id;
		instance_id = r.instance.id;
		LogZoning(
			"Client caught HandleZoneRoutingMiddleware [{}] zone_id [{}] instance_id [{}] x [{}] y [{}] z [{}] heading [{}] ignorerestrictions [{}] zone_mode [{}]",
			GetCleanName(),
			zoneID,
			instance_id,
			x,
			y,
			z,
			heading,
			ignorerestrictions,
			static_cast<int>(zm)
		);

		// If we are zoning to the same zone, we need to use the current instance ID if it is not specified.
		if (WorldContentService::Instance()->IsInPublicStaticInstance(instance_id) && zoneID == zone->GetZoneID() && instance_id == 0) {
			instance_id = zone->GetInstanceID();
		}
	}

	// zone sharding
	if (zoneID == zd->zoneidnumber &&
		instance_id == 0 &&
		zd->shard_at_player_count > 0) {

		bool found_shard = false;
		auto results     = CharacterDataRepository::GetInstanceZonePlayerCounts(database, zoneID);

		LogZoning("Zone sharding results count [{}]", results.size());

		uint64_t shard_instance_duration = 3155760000;

		for (auto &e: results) {
			LogZoning(
				"Zone sharding results [{}] ({}) instance_id [{}] player_count [{}]",
				ZoneName(e.zone_id) ? ZoneName(e.zone_id) : "Unknown",
				e.zone_id,
				e.instance_id,
				e.player_count
			);

			if (e.player_count < zd->shard_at_player_count) {
				instance_id = e.instance_id;

				database.AddClientToInstance(instance_id, CharacterID());

				LogZoning(
					"Client [{}] attempting zone to sharded zone > instance_id [{}] zone [{}] ({})",
					GetCleanName(),
					instance_id,
					ZoneName(zoneID) ? ZoneName(zoneID) : "Unknown",
					zoneID
				);

				found_shard = true;
				break;
			}
		}

		if (!found_shard) {
			uint16 new_instance_id = 0;
			database.GetUnusedInstanceID(new_instance_id);
			database.CreateInstance(new_instance_id, zoneID, zd->version, shard_instance_duration);
			database.AddClientToInstance(new_instance_id, CharacterID());
			instance_id = new_instance_id;
			LogZoning(
				"Client [{}] creating new sharded zone > instance_id [{}] zone [{}] ({})",
				GetCleanName(),
				new_instance_id,
				ZoneName(zoneID) ? ZoneName(zoneID) : "Unknown",
				zoneID
			);
		}
	}

	// passed from zone shard request to normal zone
	if (instance_id == -1) {
		instance_id = 0;
	}

	LogInfo(
		"Client [{}] zone_id [{}] instance_id [{}] x [{}] y [{}] z [{}] heading [{}] ignorerestrictions [{}] zone_mode [{}]",
		GetCleanName(),
		zoneID,
		instance_id,
		x,
		y,
		z,
		heading,
		ignorerestrictions,
		static_cast<int>(zm)
	);

	cheat_manager.SetExemptStatus(Port, true);

	if(!pZoneName) {
		Message(Chat::Red, "Invalid zone number specified");
		safe_delete_array(pZoneName);
		return;
	}
	iZoneNameLength = strlen(pZoneName);
	glm::vec4 zone_safe_point;
	switch(zm) {
		case EvacToSafeCoords:
		case ZoneToSafeCoords:
			zone_safe_point = zone->GetSafePoint();
			x = zone_safe_point.x;
			y = zone_safe_point.y;
			z = zone_safe_point.z;
			heading = zone_safe_point.w;
			break;
		case GMSummon:
		case GMHiddenSummon:
			m_Position = glm::vec4(x, y, z, heading);
			m_ZoneSummonLocation = m_Position;
			zonesummon_id = zoneID;
			zonesummon_ignorerestrictions = 1;
			break;
		case ZoneSolicited:
			m_ZoneSummonLocation = glm::vec4(x, y, z, heading);
			zonesummon_id = zoneID;
			zonesummon_ignorerestrictions = ignorerestrictions;
			break;
		case GateToBindPoint:
			x = m_Position.x = m_pp.binds[0].x;
			y = m_Position.y = m_pp.binds[0].y;
			z = m_Position.z = m_pp.binds[0].z;
			heading = m_pp.binds[0].heading;
			break;
		case ZoneToBindPoint:
			x = m_Position.x = m_pp.binds[0].x;
			y = m_Position.y = m_pp.binds[0].y;
			z = m_Position.z = m_pp.binds[0].z;
			heading = m_pp.binds[0].heading;
			zonesummon_ignorerestrictions = 1;
			LogDebug("Player [{}] has died and will be zoned to bind point in zone: [{}] at LOC x=[{}], y=[{}], z=[{}], heading=[{}]",
					GetName(), pZoneName, m_pp.binds[0].x, m_pp.binds[0].y, m_pp.binds[0].z, m_pp.binds[0].heading);
			break;
		case SummonPC:
			m_ZoneSummonLocation = glm::vec4(x, y, z, heading);
			m_Position = m_ZoneSummonLocation;
			break;
		case Rewind:
			LogDebug("[{}] has requested a /rewind from [{}], [{}], [{}], to [{}], [{}], [{}] in [{}]", GetName(),
					m_Position.x, m_Position.y, m_Position.z,
					m_RewindLocation.x, m_RewindLocation.y, m_RewindLocation.z, zone->GetShortName());
			m_ZoneSummonLocation = glm::vec4(x, y, z, heading);
			m_Position = m_ZoneSummonLocation;
			break;
		default:
			LogError("Client::ZonePC() received a reguest to perform an unsupported client zone operation");
			ReadyToZone = false;
			break;
	}

	if (ReadyToZone)
	{
		//if client is looting, we need to send an end loot
		if (IsLooting())
		{
			Entity* entity = entity_list.GetID(entity_id_being_looted);
			if (entity == 0)
			{
				LogError("OP_EndLootRequest: Corpse with id of {} not found for {}.", entity_id_being_looted, GetCleanName());
				if (ClientVersion() >= EQ::versions::ClientVersion::SoD)
					Corpse::SendEndLootErrorPacket(this);
				else
					Corpse::SendLootReqErrorPacket(this);
			}
			else if (!entity->IsCorpse())
			{
				LogError("OP_EndLootRequest: Entity with id of {} was not corpse for {}.", entity_id_being_looted, GetCleanName());
				Corpse::SendLootReqErrorPacket(this);
			}
			else
			{
				Corpse::SendEndLootErrorPacket(this);
				entity->CastToCorpse()->EndLoot(this, nullptr);
			}
			SetLooting(0);
		}

		zone_mode = zm;
		if (zm == ZoneToBindPoint) {
			auto outapp = new EQApplicationPacket(OP_ZonePlayerToBind,
							      sizeof(ZonePlayerToBind_Struct) + iZoneNameLength);
			ZonePlayerToBind_Struct* gmg = (ZonePlayerToBind_Struct*) outapp->pBuffer;

			// If we are SoF and later and are respawning from hover, we want the real zone ID, else zero to use the old hack.
			//
			if(zone->GetZoneID() == zoneID) {
				if ((ClientVersionBit() & EQ::versions::maskSoFAndLater) && (!RuleB(Character, RespawnFromHover) || !IsHoveringForRespawn()))
					gmg->bind_zone_id = 0;
				else
					gmg->bind_zone_id = zoneID;
			} else {
				gmg->bind_zone_id = zoneID;
			}

			gmg->x = x;
			gmg->y = y;
			gmg->z = z;
			gmg->heading = heading;
			strcpy(gmg->zone_name, pZoneName);

			outapp->priority = 6;
			FastQueuePacket(&outapp);
		}
		else if (zm == ZoneSolicited || zm == ZoneToSafeCoords) {
			auto outapp = new EQApplicationPacket(OP_RequestClientZoneChange, sizeof(RequestClientZoneChange_Struct));
			RequestClientZoneChange_Struct* gmg = (RequestClientZoneChange_Struct*) outapp->pBuffer;

			gmg->zone_id = zoneID;
			gmg->x = x;
			gmg->y = y;
			gmg->z = z;
			gmg->heading = heading;
			gmg->instance_id = instance_id;
			gmg->type = 0x01;				//an observed value, not sure of meaning

			LogZoning(
				"Player [{}] has requested zoning to zone_id [{}] instance_id [{}] x [{}] y [{}] z [{}] heading [{}]",
				GetCleanName(),
				zoneID,
				instance_id,
				x,
				y,
				z,
				heading
			);

			outapp->priority = 6;
			FastQueuePacket(&outapp);
		}
		else if(zm == EvacToSafeCoords) {
			auto outapp = new EQApplicationPacket(OP_RequestClientZoneChange, sizeof(RequestClientZoneChange_Struct));
			RequestClientZoneChange_Struct* gmg = (RequestClientZoneChange_Struct*) outapp->pBuffer;

			// if we are in the same zone we want to evac to, client will not send OP_ZoneChange back to do an actual
			// zoning of the client, so we have to send a viable zoneid that the client *could* zone to to make it believe
			// we are leaving the zone, even though we are not. We have to do this because we are missing the correct op code
			// and struct that should be used for evac/succor.
			// 213 is Plane of War
			// 76 is orignial Plane of Hate
			// WildcardX 27 January 2008. Tested this for 6.2 and Titanium clients.

			if (GetZoneID() == 1) {
				gmg->zone_id = 2;
			} else {
				gmg->zone_id = 1;
			}

			gmg->x = x;
			gmg->y = y;
			gmg->z = z;
			gmg->heading = heading;
			gmg->instance_id = instance_id;
			gmg->type = 0x01;				// '0x01' was an observed value for the type field, not sure of meaning

			// we hide the real zoneid we want to evac/succor to here
			zonesummon_id = zoneID;

			outapp->priority = 6;
			FastQueuePacket(&outapp);
		}
		else {
			if(zoneID == GetZoneID()) {
				//properly handle proximities
				entity_list.ProcessMove(this, glm::vec3(m_Position));
				m_Proximity = glm::vec3(m_Position);

				//send out updates to people in zone.
				SentPositionPacket(0.0f, 0.0f, 0.0f, 0.0f, 0);
			}

			auto outapp =
			    new EQApplicationPacket(OP_RequestClientZoneChange, sizeof(RequestClientZoneChange_Struct));
			RequestClientZoneChange_Struct* gmg = (RequestClientZoneChange_Struct*) outapp->pBuffer;

			gmg->zone_id = zoneID;
			gmg->x = x;
			gmg->y = y;
			gmg->z = z;
			gmg->heading = heading;
			gmg->instance_id = instance_id;
			gmg->type = 0x01;	//an observed value, not sure of meaning
			outapp->priority = 6;
			FastQueuePacket(&outapp);
		}

		LogDebug("Player [{}] has requested a zoning to LOC x=[{}], y=[{}], z=[{}], heading=[{}] in zoneid=[{}]", GetName(), x, y, z, heading, zoneID);
		//Clear zonesummon variables if we're zoning to our own zone
		//Client wont generate a zone change packet to the server in this case so
		//They aren't needed and it keeps behavior on next zone attempt from being undefined.
		if(zoneID == zone->GetZoneID() && instance_id == zone->GetInstanceID())
		{
			if(zm != EvacToSafeCoords && zm != ZoneToSafeCoords && zm != ZoneToBindPoint)
			{
				m_ZoneSummonLocation = glm::vec4();
				zonesummon_id = 0;
				zonesummon_ignorerestrictions = 0;
				zone_mode = ZoneUnsolicited;
			}
		}
	}

	safe_delete_array(pZoneName);
}

void Client::GoToSafeCoords(uint16 zone_id, uint16 instance_id)
{
	if (zone_id == 0) {
		zone_id = zone->GetZoneID();
	}

	MovePC(zone_id, instance_id, 0.0f, 0.0f, 0.0f, 0.0f, 0, ZoneToSafeCoords);
}


void Mob::Gate(uint8 bind_number) {
	GoToBind(bind_number);
	if (RuleB(NPC, NPCHealOnGate) && IsNPC() && GetHPRatio() <= RuleR(NPC, NPCHealOnGateAmount)) {
		auto HealAmount = (RuleR(NPC, NPCHealOnGateAmount) / 100);
		SetHP(int64(GetMaxHP() * HealAmount));
	}
}

void Client::Gate(uint8 bind_number) {
	Mob::Gate(bind_number);
}

void NPC::Gate(uint8 bind_number) {
	entity_list.MessageCloseString(this, true, RuleI(Range, SpellMessages), Chat::Spells, GATES, GetCleanName());

	Mob::Gate(bind_number);
}

void Client::SetBindPoint(int bind_number, int to_zone, int to_instance, const glm::vec3 &location)
{
	if (bind_number < 0 || bind_number >= 4) {
		bind_number = 0;
	}

	if (to_zone == -1) {
		m_pp.binds[bind_number].zone_id     = zone->GetZoneID();
		m_pp.binds[bind_number].instance_id = (zone->GetInstanceID() != 0 && zone->IsInstancePersistent())? zone->GetInstanceID() : 0;
		m_pp.binds[bind_number].x           = m_Position.x;
		m_pp.binds[bind_number].y           = m_Position.y;
		m_pp.binds[bind_number].z           = m_Position.z;
		m_pp.binds[bind_number].heading     = GetHeading();
	}
	else {
		m_pp.binds[bind_number].zone_id     = to_zone;
		m_pp.binds[bind_number].instance_id = to_instance;
		m_pp.binds[bind_number].x           = location.x;
		m_pp.binds[bind_number].y           = location.y;
		m_pp.binds[bind_number].z           = location.z;
		m_pp.binds[bind_number].heading     = GetHeading();
	}

	database.SaveCharacterBinds(this);
}

void Client::SetBindPoint2(int bind_number, int to_zone, int to_instance, const glm::vec4 &location)
{
	if (bind_number < 0 || bind_number >= 4)
		bind_number = 0;

	if (to_zone == -1) {
		m_pp.binds[bind_number].zone_id = zone->GetZoneID();
		m_pp.binds[bind_number].instance_id = (zone->GetInstanceID() != 0 && zone->IsInstancePersistent()) ? zone->GetInstanceID() : 0;
		m_pp.binds[bind_number].x = m_Position.x;
		m_pp.binds[bind_number].y = m_Position.y;
		m_pp.binds[bind_number].z = m_Position.z;
		m_pp.binds[bind_number].heading = m_Position.w;
	} else {
		m_pp.binds[bind_number].zone_id = to_zone;
		m_pp.binds[bind_number].instance_id = to_instance;
		m_pp.binds[bind_number].x = location.x;
		m_pp.binds[bind_number].y = location.y;
		m_pp.binds[bind_number].z = location.z;
		m_pp.binds[bind_number].heading = location.w;
	}

	database.SaveCharacterBinds(this);
}

void Client::GoToBind(uint8 bind_number) {
	// if the bind number is invalid, use the primary bind
	if(bind_number > 4)
		bind_number = 0;

	// move the client, which will zone them if needed.
	// ignore restrictions on the zone request..?
	if(bind_number == 0)
		MovePC(
			m_pp.binds[0].zone_id,
			m_pp.binds[0].instance_id,
			0.0f,
			0.0f,
			0.0f,
			0.0f,
			1,
			GateToBindPoint
		);
	else
		MovePC(
			m_pp.binds[bind_number].zone_id,
			m_pp.binds[bind_number].instance_id,
			m_pp.binds[bind_number].x,
			m_pp.binds[bind_number].y,
			m_pp.binds[bind_number].z,
			m_pp.binds[bind_number].heading,
			1
		);
}

void Client::GoToDeath() {
	MovePC(m_pp.binds[0].zone_id, m_pp.binds[0].instance_id, 0.0f, 0.0f, 0.0f, 0.0f, 1, ZoneToBindPoint);
}

void Client::ClearZoneFlag(uint32 zone_id)
{
	if (!HasZoneFlag(zone_id)) {
		return;
	}

	zone_flags.erase(zone_id);

	ZoneFlagsRepository::DeleteWhere(
		database,
		fmt::format(
			"`charID` = {} AND `zoneID` = {}",
			CharacterID(),
			zone_id
		)
	);
}

bool Client::HasZoneFlag(uint32 zone_id) const
{
	return zone_flags.find(zone_id) != zone_flags.end();
}

void Client::LoadZoneFlags()
{
	const auto& l = ZoneFlagsRepository::GetWhere(
		database,
		fmt::format(
			"`charID` = {}",
			CharacterID()
		)
	);

	zone_flags.clear();

	for (const auto& e : l) {
		zone_flags.insert(e.zoneID);
	}
}

void Client::SendZoneFlagInfo(Client* to) const
{
	if (zone_flags.empty()) {
		to->Message(
			Chat::White,
			fmt::format(
				"{} {} no zone flags.",
				to->GetTargetDescription(const_cast<Mob*>(CastToMob()), TargetDescriptionType::UCYou),
				to == this ? "have" : "has"
			).c_str()
		);
		return;
	}

	to->Message(
		Chat::White,
		fmt::format(
			"{} {} the following zone flags:",
			to->GetTargetDescription(const_cast<Mob*>(CastToMob()), TargetDescriptionType::UCYou),
			to == this ? "have" : "has"
		).c_str()
	);

	uint32 flag_count = 0;

	for (const auto& zone_id : zone_flags) {
		const uint32 flag_number = (flag_count + 1);

		const auto& z = GetZone(zone_id);

		if (z) {
			const std::string& flag_name = z->flag_needed;

			to->Message(
				Chat::White,
				fmt::format(
					"Flag {} | Zone: {} ({}) ID: {}{}",
					flag_number,
					z->long_name,
					z->short_name,
					zone_id,
					(
						!flag_name.empty() ?
						fmt::format(
							" Flag Required: {}",
							flag_name
						) :
						""
					)
				).c_str()
			);
			flag_count++;
		}
	}

	to->Message(
		Chat::White,
		fmt::format(
			"{} Zone flag{} found for {}.",
			flag_count,
			flag_count != 1 ? "s" : "",
			to->GetTargetDescription(const_cast<Mob*>(CastToMob()))
		).c_str()
	);
}

void Client::SetZoneFlag(uint32 zone_id)
{
	if (HasZoneFlag(zone_id)) {
		return;
	}

	zone_flags.insert(zone_id);

	ZoneFlagsRepository::InsertOne(
		database,
		ZoneFlagsRepository::ZoneFlags{
			.charID = static_cast<int32_t>(CharacterID()),
			.zoneID = static_cast<int32_t>(zone_id)
		}
	);
}

void Client::ClearPEQZoneFlag(uint32 zone_id) {
	if (!HasPEQZoneFlag(zone_id)) {
		return;
	}

	peqzone_flags.erase(zone_id);

	if (!CharacterPeqzoneFlagsRepository::DeleteFlag(database, CharacterID(), zone_id)) {
		LogError("MySQL Error while trying to clear PEQZone flag for [{}]", GetName());
	}
}

bool Client::HasPEQZoneFlag(uint32 zone_id) const {
	return peqzone_flags.find(zone_id) != peqzone_flags.end();
}

void Client::LoadPEQZoneFlags() {
	const auto l = CharacterPeqzoneFlagsRepository::GetWhere(
		database,
		fmt::format(
			"id = {}",
			CharacterID()
		)
	);
	if (l.empty()) {
		return;
	}

	peqzone_flags.clear();

	for (const auto& f : l) {
		peqzone_flags.insert(f.zone_id);
	}
}

void Client::SendPEQZoneFlagInfo(Client *to) const {
	if (peqzone_flags.empty()) {
		to->Message(
			Chat::White,
			fmt::format(
				"{} {} no PEQZone Flags.",
				to == this ? "You" : GetName(),
				to == this ? "have" : "has"
			).c_str()
		);
		return;
	}

	to->Message(
		Chat::White,
		fmt::format(
			"{} {} the following PEQZone Flags:",
			to == this ? "You" : GetName(),
			to == this ? "have" : "has"
		).c_str()
	);

	int flag_count = 0;
	for (const auto& zone_id : peqzone_flags) {
		int flag_number = (flag_count + 1);
		std::string zone_short_name = ZoneName(zone_id, true);
		if (zone_short_name != "UNKNOWN") {
			std::string zone_long_name = ZoneLongName(zone_id);
			to->Message(
				Chat::White,
				fmt::format(
					"Flag {} | Zone: {} ({}) ID: {}",
					flag_number,
					zone_long_name,
					zone_short_name,
					zone_id
				).c_str()
			);
			flag_count++;
		}
	}

	to->Message(
		Chat::White,
		fmt::format(
			"{} {} {} PEQZone Flags.",
			to == this ? "You" : GetName(),
			to == this ? "have" : "has",
			flag_count
		).c_str()
	);
}

void Client::SetPEQZoneFlag(uint32 zone_id) {
	if (HasPEQZoneFlag(zone_id)) {
		return;
	}

	peqzone_flags.insert(zone_id);

	auto f = CharacterPeqzoneFlagsRepository::NewEntity();

	f.id = CharacterID();
	f.zone_id = zone_id;

	if (!CharacterPeqzoneFlagsRepository::InsertOne(database, f).id) {
		LogError("MySQL Error while trying to set zone flag for [{}]", GetName());
	}
}

bool Client::CanEnterZone(const std::string& zone_short_name, int16 instance_version) {
	//check some critial rules to see if this char needs to be booted from the zone
	//only enforce rules here which are serious enough to warrant being kicked from
	//the zone

	if (Admin() >= RuleI(GM, MinStatusToZoneAnywhere)) {
		return true;
	}

	auto z = GetZoneVersionWithFallback(
		zone_short_name.empty() ? ZoneID(zone->GetShortName()) : ZoneID(zone_short_name),
		instance_version == -1 ? zone->GetInstanceVersion() : instance_version
	);

	if (!z) {
		return false;
	}

	if (!GetGM() && GetLevel() < z->min_level) {
		LogInfo(
			"Character [{}] does not meet minimum level requirement ([{}] < [{}])!",
			GetCleanName(),
			GetLevel(),
			z->min_level
		);
		return false;
	}

	if (!GetGM() && GetLevel() > z->max_level) {
		LogInfo(
			"Character [{}] does not meet maximum level requirement ([{}] > [{}])!",
			GetCleanName(),
			GetLevel(),
			z->max_level
		);
		return false;
	}

	if (Admin() < z->min_status) {
		LogInfo(
			"Character [{}] does not meet minimum status requirement ([{}] < [{}])!",
			GetCleanName(),
			Admin(),
			z->min_status
		);
		return false;
	}

	if (
		!GetGM() &&
		!z->flag_needed.empty() &&
		Strings::IsNumber(z->flag_needed) &&
		Strings::ToBool(z->flag_needed) &&
		!HasZoneFlag(z->zoneidnumber)
	) {
		LogInfo(
			"Character [{}] does not have the flag to be in this zone [{}]!",
			GetCleanName(),
			z->flag_needed
		);
		return false;
	}

	return true;
}
