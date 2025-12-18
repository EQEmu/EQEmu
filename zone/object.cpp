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

#include "object.h"

#include "common/events/player_event_logs.h"
#include "common/repositories/criteria/content_filter_criteria.h"
#include "common/repositories/ground_spawns_repository.h"
#include "common/repositories/object_repository.h"
#include "common/strings.h"
#include "zone/client.h"
#include "zone/entity.h"
#include "zone/mob.h"
#include "zone/queryserv.h"
#include "zone/quest_parser_collection.h"
#include "zone/worldserver.h"
#include "zone/zonedb.h"


const char DEFAULT_OBJECT_NAME[] = "IT63_ACTORDEF";
const char DEFAULT_OBJECT_NAME_SUFFIX[] = "_ACTORDEF";


extern Zone       *zone;
extern EntityList  entity_list;
extern WorldServer worldserver;
extern QueryServ  *QServ;

// Loading object from database
Object::Object(
	uint32 id,
	uint32 type,
	uint32 icon,
	const Object_Struct& object,
	const EQ::ItemInstance* inst,
	bool fix_z
) :
respawn_timer(0),
decay_timer(300000)
{
	user      = nullptr;
	last_user = nullptr;

	// Initialize members
	m_id           = id;
	m_type         = type;
	m_icon         = icon;
	m_fix_z        = fix_z;
	m_inst         = nullptr;
	m_ground_spawn = false;

	// Copy object data
	memcpy(&m_data, &object, sizeof(Object_Struct));
	if (inst) {
		m_inst = inst->Clone();
		decay_timer.Start();
	} else {
		decay_timer.Disable();
	}

	memset(m_display_name, 0, sizeof(m_display_name));

	respawn_timer.Disable();

	// Set drop_id to zero - it will be set when added to zone with SetID()
	m_data.drop_id = 0;
	m_data.size    = object.size;
	m_data.tilt_x  = object.tilt_x;
	m_data.tilt_y  = object.tilt_y;

	if (!IsFixZEnabled()) {
		FixZ();
	}
}

//creating a re-ocurring ground spawn.
Object::Object(
	const EQ::ItemInstance* inst,
	const std::string& name,
	float max_x,
	float min_x,
	float max_y,
	float min_y,
	float z,
	float heading,
	uint32 respawn_timer_,
	bool fix_z
) :
respawn_timer(respawn_timer_ * 1000),
decay_timer(300000)
{

	user           = nullptr;
	last_user      = nullptr;
	m_max_x        = max_x;
	m_max_y        = max_y;
	m_min_x        = min_x;
	m_min_y        = min_y;
	m_id           = 0;
	m_inst         = (inst) ? inst->Clone() : nullptr;
	m_type         = ObjectTypes::Temporary;
	m_icon         = 0;
	m_fix_z        = fix_z;
	m_ground_spawn = true;

	decay_timer.Disable();
	// Set as much struct data as we can
	memset(&m_data, 0, sizeof(Object_Struct));

	memset(m_display_name, 0, sizeof(m_display_name));

	m_data.heading = heading;
	m_data.z       = z;
	m_data.zone_id = zone->GetZoneID();

	respawn_timer.Disable();

	strcpy(m_data.object_name, name.c_str());

	RandomSpawn(false);

	if (!IsFixZEnabled()) {
		FixZ();
	}

	// Hardcoded portion for unknown members
	m_data.unknown024 = 0x7f001194;
	m_data.unknown076 = 0x0000d5fe;
	m_data.unknown084 = 0xFFFFFFFF;
}

// Loading object from client dropping item on ground
Object::Object(
	Client* client,
	const EQ::ItemInstance* inst
) :
respawn_timer(0),
decay_timer(300000)
{
	user = nullptr;
	last_user = nullptr;

	// Initialize members
	m_id           = 0;
	m_inst         = (inst) ? inst->Clone() : nullptr;
	m_type         = ObjectTypes::Temporary;
	m_icon         = 0;
	m_ground_spawn = false;
	m_fix_z        = false;

	// Set as much struct data as we can
	memset(&m_data, 0, sizeof(Object_Struct));

	memset(m_display_name, 0, sizeof(m_display_name));

	m_data.heading = client->GetHeading();
	m_data.x       = client->GetX();
	m_data.y       = client->GetY();

	if (client->ClientVersion() >= EQ::versions::ClientVersion::RoF2) {
		// RoF2 places items at player's Z, which is 0.625 of their height.
		m_data.z = client->GetZ() - (client->GetSize() * 0.625f);
	} else {
		m_data.z = client->GetZ();
	}

	m_data.zone_id = zone->GetZoneID();

	decay_timer.Start();
	respawn_timer.Disable();

	// Hardcoded portion for unknown members
	m_data.unknown024 = 0x7f001194;
	m_data.unknown076 = 0x0000d5fe;
	m_data.unknown084 = 0xFFFFFFFF;

	// Set object name
	if (inst) {
		const EQ::ItemData* item = inst->GetItem();
		if (item) {
			if (item->IDFile[0] == '\0') {
				strcpy(m_data.object_name, DEFAULT_OBJECT_NAME);
			} else {
				// Object name is idfile + _ACTORDEF
				uint32 len_idfile = strlen(inst->GetItem()->IDFile);
				uint32 len_copy   = sizeof(m_data.object_name) - len_idfile - 1;
				if (len_copy > sizeof(DEFAULT_OBJECT_NAME_SUFFIX)) {
					len_copy = sizeof(DEFAULT_OBJECT_NAME_SUFFIX);
				}

				memcpy(&m_data.object_name[0], inst->GetItem()->IDFile, len_idfile);
				memcpy(&m_data.object_name[len_idfile], DEFAULT_OBJECT_NAME_SUFFIX, len_copy);
			}
		} else {
			strcpy(m_data.object_name, DEFAULT_OBJECT_NAME);
		}
	}

	FixZ();
}

Object::Object(
	const EQ::ItemInstance *inst,
	float x,
	float y,
	float z,
	float heading,
	uint32 decay_time,
	bool fix_z
) :
respawn_timer(0),
decay_timer(decay_time)
{
	user      = nullptr;
	last_user = nullptr;

	// Initialize members
	m_id           = 0;
	m_inst         = (inst) ? inst->Clone() : nullptr;
	m_type         = ObjectTypes::Temporary;
	m_icon         = 0;
	m_ground_spawn = false;
	m_fix_z        = fix_z;

	// Set as much struct data as we can
	memset(&m_data, 0, sizeof(Object_Struct));

	memset(m_display_name, 0, sizeof(m_display_name));

	m_data.heading = heading;
	m_data.x       = x;
	m_data.y       = y;
	m_data.z       = z;
	m_data.zone_id = zone->GetZoneID();

	if (decay_time) {
		decay_timer.Start();
	}

	respawn_timer.Disable();

	// Hardcoded portion for unknown members
	m_data.unknown024 = 0x7f001194;
	m_data.unknown076 = 0x0000d5fe;
	m_data.unknown084 = 0xFFFFFFFF;

	// Set object name
	if (inst) {
		const EQ::ItemData *item = inst->GetItem();
		if (item) {
			if (item->IDFile[0] == '\0') {
				strcpy(m_data.object_name, DEFAULT_OBJECT_NAME);
			} else {
				// Object name is idfile + _ACTORDEF
				uint32 len_idfile = strlen(inst->GetItem()->IDFile);
				uint32 len_copy   = sizeof(m_data.object_name) - len_idfile - 1;
				if (len_copy > sizeof(DEFAULT_OBJECT_NAME_SUFFIX)) {
					len_copy = sizeof(DEFAULT_OBJECT_NAME_SUFFIX);
				}

				memcpy(&m_data.object_name[0], inst->GetItem()->IDFile, len_idfile);
				memcpy(&m_data.object_name[len_idfile], DEFAULT_OBJECT_NAME_SUFFIX, len_copy);
			}
		} else {
			strcpy(m_data.object_name, DEFAULT_OBJECT_NAME);
		}
	}

	if (!IsFixZEnabled()) {
		FixZ();
	}
}

Object::Object(
	const std::string& model,
	float x,
	float y,
	float z,
	float heading,
	uint8 type,
	uint32 decay_time
) :
respawn_timer(0),
decay_timer(decay_time)
{
	user      = nullptr;
	last_user = nullptr;

	auto* inst = new EQ::ItemInstance(ItemInstWorldContainer);

	// Initialize members
	m_id           = 0;
	m_inst         = inst->Clone();
	m_type         = type;
	m_icon         = 0;
	m_ground_spawn = false;

	// Set as much struct data as we can
	memset(&m_data, 0, sizeof(Object_Struct));
	m_data.heading = heading;
	m_data.x       = x;
	m_data.y       = y;
	m_data.z       = z;
	m_data.zone_id = zone->GetZoneID();

	memset(m_display_name, 0, sizeof(m_display_name));

	if (!IsFixZEnabled()) {
		FixZ();
	}

	if (decay_time) {
		decay_timer.Start();
	}

	respawn_timer.Disable();

	// Hardcoded portion for unknown members
	m_data.unknown024 = 0x7f001194;
	m_data.unknown076 = 0x0000d5fe;
	m_data.unknown084 = 0xFFFFFFFF;

	strcpy(m_data.object_name, !model.empty() ? model.c_str() : "IT64_ACTORDEF");

	safe_delete(inst);
}

Object::~Object()
{
	safe_delete(m_inst);

	if (user) {
		user->SetTradeskillObject(nullptr);
	}
}

void Object::SetID(uint16 set_id)
{
	// Invoke base class
	Entity::SetID(set_id);

	// Store new id as drop_id
	m_data.drop_id = (uint32)GetID();
}

// Reset state of object back to zero
void Object::ResetState()
{
	Close();

	safe_delete(m_inst);

	m_id   = 0;
	m_type = 0;
	m_icon = 0;

	memset(&m_data, 0, sizeof(Object_Struct));
}

bool Object::Save()
{
	if (m_id) { // Update existing
		content_db.UpdateObject(m_id, m_type, m_icon, m_data, m_inst);
	} else { // Doesn't yet exist, add now
		m_id = content_db.AddObject(m_type, m_icon, m_data, m_inst);
	}

	return true;
}

uint16 Object::VarSave()
{
	if (m_id) { // Update existing
		content_db.UpdateObject(m_id, m_type, m_icon, m_data, m_inst);
	} else { // Doesn't yet exist, add now
		m_id = content_db.AddObject(m_type, m_icon, m_data, m_inst);
	}

	return m_id;
}

// Remove object from database
void Object::Delete(bool reset_state)
{
	if (m_id != 0) {
		content_db.DeleteObject(m_id);
	}

	if (reset_state) {
		ResetState();
	}
}

const EQ::ItemInstance* Object::GetItem(uint8 index) {
	if (index < EQ::invtype::WORLD_SIZE) {
		return m_inst->GetItem(index);
	}

	return nullptr;
}

// Add item to object (only logical for world tradeskill containers
void Object::PutItem(uint8 index, const EQ::ItemInstance* inst)
{
	if (index > 9) {
		LogError("Invalid index specified ([{}])", index);
		return;
	}

	if (m_inst && m_inst->IsType(EQ::item::ItemClassBag)) {
		if (inst) {
			m_inst->PutItem(index, *inst);
		} else {
			m_inst->DeleteItem(index);
		}

		database.SaveWorldContainer(zone->GetZoneID(),m_id,m_inst);
		// This is _highly_ inefficient, but for now it will work: Save entire object to database
		Save();
	}
}

void Object::Close() {
	if (user) {
		last_user = user;
		// put any remaining items from the world container back into the player's inventory to avoid item loss
		// if they close the container without removing all items
		auto container = m_inst;
		if (container) {
			for (uint8 i = EQ::invbag::SLOT_BEGIN; i <= EQ::invbag::SLOT_END; i++) {
				auto inst = container->PopItem(i);
				if (inst) {
					user->MoveItemToInventory(inst, true);
				}
			}
		}

		auto outapp = new EQApplicationPacket(OP_ClearObject, sizeof(ClearObject_Struct));
		ClearObject_Struct *cos = (ClearObject_Struct *)outapp->pBuffer;
		cos->Clear = 1;
		user->QueuePacket(outapp);
		safe_delete(outapp);

		user->SetTradeskillObject(nullptr);
	}

	user = nullptr;
}

// Remove item from container
void Object::DeleteItem(uint8 index)
{
	if (m_inst && m_inst->IsType(EQ::item::ItemClassBag)) {
		m_inst->DeleteItem(index);

		// This is _highly_ inefficient, but for now it will work: Save entire object to database
		Save();
	}
}

// Pop item out of container
EQ::ItemInstance* Object::PopItem(uint8 index)
{
	EQ::ItemInstance* inst = nullptr;

	if (m_inst && m_inst->IsType(EQ::item::ItemClassBag)) {
		inst = m_inst->PopItem(index);

		// This is _highly_ inefficient, but for now it will work: Save entire object to database
		Save();
	}

	return inst;
}

void Object::CreateSpawnPacket(EQApplicationPacket* app)
{
	app->SetOpcode(OP_GroundSpawn);

	safe_delete_array(app->pBuffer);

	app->pBuffer = new uchar[sizeof(Object_Struct)];
	app->size    = sizeof(Object_Struct);

	memcpy(app->pBuffer, &m_data, sizeof(Object_Struct));
}

void Object::CreateDeSpawnPacket(EQApplicationPacket* app)
{
	app->SetOpcode(OP_ClickObject);

	safe_delete_array(app->pBuffer);

	app->pBuffer = new uchar[sizeof(ClickObject_Struct)];
	app->size    = sizeof(ClickObject_Struct);

	memset(app->pBuffer, 0, sizeof(ClickObject_Struct));

	auto co = (ClickObject_Struct*) app->pBuffer;

	co->drop_id   = m_data.drop_id;
	co->player_id = 0;
}

bool Object::Process(){
	if (m_type == ObjectTypes::Temporary && decay_timer.Enabled() && decay_timer.Check()) {
		// Send click to all clients (removes entity on client)
		auto outapp = new EQApplicationPacket(OP_ClickObject, sizeof(ClickObject_Struct));
		ClickObject_Struct* click_object = (ClickObject_Struct*)outapp->pBuffer;
		click_object->drop_id = GetID();
		entity_list.QueueClients(nullptr, outapp, false);
		safe_delete(outapp);

		// Remove object
		content_db.DeleteObject(m_id);
		return false;
	}

	if (m_ground_spawn && respawn_timer.Check()){
		RandomSpawn(true);
	}

	if (user && !entity_list.GetClientByCharID(user->CharacterID())) {
		last_user = user;

		user->SetTradeskillObject(nullptr);

		user = nullptr;
	}

	return true;
}

void Object::RandomSpawn(bool send_packet) {
	if (!m_ground_spawn) {
		return;
	}

	m_data.x = zone->random.Real(m_min_x, m_max_x);
	m_data.y = zone->random.Real(m_min_y, m_max_y);

	if (m_data.z == BEST_Z_INVALID && zone->HasMap()) {
		glm::vec3 me;

		me.x = m_data.x;
		me.y = m_data.y;
		me.z = 0;

		glm::vec3 hit;

		float best_z = zone->zonemap->FindClosestZ(me, &hit);
		if (best_z != BEST_Z_INVALID) {
			m_data.z = best_z + 0.1f;
		}
	}

	LogInfo("[{}] [{}] ([{}] [{}] [{}])", m_data.object_name, m_inst->GetID(), m_data.x, m_data.y, m_data.z);

	respawn_timer.Disable();

	if(send_packet) {
		EQApplicationPacket app;
		CreateSpawnPacket(&app);
		entity_list.QueueClients(nullptr, &app, true);
	}
}

bool Object::HandleClick(Client* sender, const ClickObject_Struct* click_object)
{
	if (m_ground_spawn) {//This is a Cool Groundspawn
		respawn_timer.Start();
	}

	if (m_type == ObjectTypes::Temporary) {
		bool cursor_delete  = false;
		bool duplicate_lore = false;

		if (m_inst && sender) {
			// if there is a lore conflict, delete the offending item from the server inventory
			// the client updates itself and takes care of sending "duplicate lore item" messages
			auto item = m_inst->GetItem();
			if (sender->CheckLoreConflict(item)) {
				duplicate_lore = true;

				const int16 lore_item_slot = sender->GetInv().HasItem(item->ID, 0, invWhereBank);
				if (lore_item_slot != INVALID_INDEX) { // if the duplicate is in the bank, delete it.
					sender->DeleteItemInInventory(lore_item_slot);
				} else { // otherwise, we delete the new one
					cursor_delete = true;
				}
			}

			if (item->RecastDelay) {
				if (item->RecastType != RECAST_TYPE_UNLINKED_ITEM) {
					m_inst->SetRecastTimestamp(
						database.GetItemRecastTimestamp(
							sender->CharacterID(),
							item->RecastType
						)
					);
				} else {
					m_inst->SetRecastTimestamp(
						database.GetItemRecastTimestamp(
							sender->CharacterID(),
							item->ID
						)
					);
				}
			}

			if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::GROUNDSPAWN_PICKUP)) {
				auto e = PlayerEvent::GroundSpawnPickupEvent{
					.item_id = item->ID,
					.item_name = item->Name,
				};
				RecordPlayerEventLogWithClient(sender, PlayerEvent::GROUNDSPAWN_PICKUP, e);
			}

			if (parse->PlayerHasQuestSub(EVENT_PLAYER_PICKUP)) {
				std::vector<std::any> args = { m_inst };

				if (parse->EventPlayer(EVENT_PLAYER_PICKUP, sender, std::to_string(item->ID), GetID(), &args)) {
					auto outapp = new EQApplicationPacket(OP_ClickObject, sizeof(ClickObject_Struct));

					memcpy(outapp->pBuffer, click_object, sizeof(ClickObject_Struct));

					auto co = (ClickObject_Struct*) outapp->pBuffer;

					co->drop_id = 0;

					entity_list.QueueClients(nullptr, outapp, false);

					safe_delete(outapp);

					sender->SetTradeskillObject(nullptr);

					user = nullptr;

					return true;
				}
			}

			if (parse->ZoneHasQuestSub(EVENT_PLAYER_PICKUP)) {
				std::vector<std::any> args = { m_inst, sender };

				if (parse->EventZone(EVENT_PLAYER_PICKUP, zone, std::to_string(item->ID), GetID(), &args)) {
					auto outapp = new EQApplicationPacket(OP_ClickObject, sizeof(ClickObject_Struct));

					memcpy(outapp->pBuffer, click_object, sizeof(ClickObject_Struct));

					auto co = (ClickObject_Struct*) outapp->pBuffer;

					co->drop_id = 0;

					entity_list.QueueClients(nullptr, outapp, false);

					safe_delete(outapp);

					sender->SetTradeskillObject(nullptr);

					user = nullptr;

					return true;
				}
			}

			// Transfer item to client
			sender->PutItemInInventory(EQ::invslot::slotCursor, *m_inst, false);
			sender->SendItemPacket(EQ::invslot::slotCursor, m_inst, ItemPacketTrade);

			sender->CheckItemDiscoverability(m_inst->GetID());

			if (cursor_delete) {    // delete the item if it's a duplicate lore. We have to do this because the client expects the item packet
				sender->DeleteItemInInventory(EQ::invslot::slotCursor, 1, true);
			}

			if (!m_ground_spawn) {
				safe_delete(m_inst);
			}

			// No longer using a tradeskill object
			sender->SetTradeskillObject(nullptr);

			user = nullptr;
		}

		// Send click to all clients (removes entity on client)
		auto outapp = new EQApplicationPacket(OP_ClickObject, sizeof(ClickObject_Struct));

		memcpy(outapp->pBuffer, click_object, sizeof(ClickObject_Struct));

		entity_list.QueueClients(nullptr, outapp, false);

		safe_delete(outapp);

		// Remove object
		content_db.DeleteObject(m_id);

		if (!m_ground_spawn) {
			entity_list.RemoveEntity(GetID());
		}

		// we have to delete the entity on click or the client desyncs
		// this is a way to immediately respawn the groundspawn after killing it and
		// deleting the item from the player
		// I believe older clients somehow sent this automatically but we are no longer with ROF2+
		if (duplicate_lore) {
			sender->Message(Chat::Yellow, "Duplicate lore item detected");
			respawn_timer.Trigger();
		}
	} else {
		// Tradeskill item
		auto outapp = new EQApplicationPacket(OP_ClickObjectAction, sizeof(ClickObjectAction_Struct));

		auto coa = (ClickObjectAction_Struct*) outapp->pBuffer;

		//TODO: there is prolly a better way to do this.
		coa->type      = m_type;
		coa->unknown16 = 0x0a;
		coa->drop_id   = click_object->drop_id;
		coa->player_id = click_object->player_id;
		coa->icon      = m_icon;

		strn0cpy(coa->object_name, m_display_name, 64);

		//if this is not the main user, send them a close and a message
		if (!user || user == sender) {
			coa->open = 0x01;
		} else {
			coa->open = 0x00;

			if (sender->ClientVersion() >= EQ::versions::ClientVersion::RoF) {
				coa->drop_id = UINT32_MAX;
				sender->Message(Chat::White, "Someone else is using that. Try again later.");
			}
		}

		sender->QueuePacket(outapp);

		safe_delete(outapp);

		//if the object allready had a user, we are done
		if (user) {
			return false;
		}

		// Starting to use this object
		sender->SetTradeskillObject(this);

		user = sender;

		// Send items inside of container

		if (m_inst && m_inst->IsType(EQ::item::ItemClassBag)) {
			//Clear out no-drop and no-rent items first if different player opens it
			if (user != last_user) {
				m_inst->ClearByFlags(byFlagSet, byFlagSet);
			}

			auto outapp = new EQApplicationPacket(OP_ClientReady, 0);

			sender->QueuePacket(outapp);

			safe_delete(outapp);

			for (uint8 i = EQ::invbag::SLOT_BEGIN; i <= EQ::invbag::SLOT_END; i++) {
				auto inst = m_inst->GetItem(i);
				if (inst) {
					sender->SendItemPacket(i, inst, ItemPacketWorldContainer);
				}
			}
		}
	}

	return true;
}

uint32 ZoneDatabase::AddObject(
	uint32 type,
	uint32 icon,
	const Object_Struct& o,
	const EQ::ItemInstance* inst
)
{
	uint32 item_id = 0;
	int16  charges = 0;

	if (inst && inst->GetItem()) {
		item_id = inst->GetItem()->ID;
		charges = inst->GetCharges();
	}

	auto e = ObjectRepository::NewEntity();

	e.zoneid     = o.zone_id;
	e.xpos       = o.x;
	e.ypos       = o.y;
	e.zpos       = o.z;
	e.heading    = o.heading;
	e.itemid     = item_id;
	e.charges    = charges;
	e.objectname = o.object_name;
	e.type       = type;
	e.icon       = icon;

	e = ObjectRepository::InsertOne(*this, e);

	if (!e.id) {
		return 0;
	}

	if (inst && inst->IsType(EQ::item::ItemClassBag)) {
		SaveWorldContainer(o.zone_id, e.id, inst);
	}

	return e.id;
}

void ZoneDatabase::UpdateObject(
	uint32 object_id,
	uint32 type,
	uint32 icon,
	const Object_Struct& o,
	const EQ::ItemInstance* inst
)
{
	uint32 item_id = 0;
	int16  charges = 0;

	if (inst && inst->GetItem()) {
		item_id = inst->GetItem()->ID;
		charges = inst->GetCharges();
	}

	if (inst && !inst->IsType(EQ::item::ItemClassBag)) {
		auto e = ObjectRepository::FindOne(*this, object_id);

		e.zoneid     = o.zone_id;
		e.xpos       = o.x;
		e.ypos       = o.y;
		e.zpos       = o.z;
		e.heading    = o.heading;
		e.itemid     = item_id;
		e.charges    = charges;
		e.objectname = o.object_name;
		e.type       = type;
		e.icon       = icon;
		e.size       = o.size;
		e.tilt_x     = o.tilt_x;
		e.tilt_y     = o.tilt_y;

		const int updated = ObjectRepository::UpdateOne(*this, e);

		if (!updated) {
			return;
		}
	}

	if (inst && inst->IsType(EQ::item::ItemClassBag)) {
		SaveWorldContainer(o.zone_id, object_id, inst);
	}
}

GroundSpawns* ZoneDatabase::LoadGroundSpawns(
	uint32 zone_id,
	int16 instance_version,
	GroundSpawns* gs
)
{
	const auto& l = GroundSpawnsRepository::GetWhere(
		*this,
		fmt::format(
			"`zoneid` = {} AND (`version` = {} OR `version` = -1) {} LIMIT 50",
			zone_id,
			instance_version,
			ContentFilterCriteria::apply()
		)
	);

	if (l.empty()) {
		return gs;
	}

	uint32 slot_id = 0;

	for (const auto& e : l) {
		gs->spawn[slot_id].name          = e.name;
		gs->spawn[slot_id].max_x         = e.max_x;
		gs->spawn[slot_id].max_y         = e.max_y;
		gs->spawn[slot_id].max_z         = e.max_z;
		gs->spawn[slot_id].min_x         = e.min_x;
		gs->spawn[slot_id].min_y         = e.min_y;
		gs->spawn[slot_id].heading       = e.heading;
		gs->spawn[slot_id].item_id       = e.item;
		gs->spawn[slot_id].max_allowed   = e.max_allowed;
		gs->spawn[slot_id].respawn_timer = e.respawn_timer;
		gs->spawn[slot_id].fix_z         = e.fix_z;

		slot_id++;
	}

	return gs;
}

void ZoneDatabase::DeleteObject(uint32 object_id)
{
	if (!object_id) {
		return;
	}

	ObjectRepository::DeleteOne(*this, object_id);
}

uint32 Object::GetDBID()
{
	return m_id;
}

uint32 Object::GetType()
{
	return m_type;
}

void Object::SetType(uint32 type)
{
	m_type = type;
	m_data.object_type = type;
}

uint32 Object::GetIcon()
{
	return m_icon;
}

float Object::GetX()
{
	return m_data.x;
}

float Object::GetY()
{
	return m_data.y;
}


float Object::GetZ()
{
	return m_data.z;
}

float Object::GetHeadingData()
{
	return m_data.heading;
}

float Object::GetTiltX()
{
	return m_data.tilt_x;
}

float Object::GetTiltY()
{
	return m_data.tilt_y;
}

void Object::SetX(float pos)
{
	m_data.x = pos;

	auto app = new EQApplicationPacket();
	auto app2 = new EQApplicationPacket();
	CreateDeSpawnPacket(app);
	CreateSpawnPacket(app2);
	entity_list.QueueClients(0, app);
	entity_list.QueueClients(0, app2);
	safe_delete(app);
	safe_delete(app2);
}

void Object::SetY(float pos)
{
	m_data.y = pos;

	auto app = new EQApplicationPacket();
	auto app2 = new EQApplicationPacket();
	CreateDeSpawnPacket(app);
	CreateSpawnPacket(app2);
	entity_list.QueueClients(0, app);
	entity_list.QueueClients(0, app2);
	safe_delete(app);
	safe_delete(app2);
}

void Object::SetTiltX(float pos)
{
	m_data.tilt_x = pos;

	auto app = new EQApplicationPacket();
	auto app2 = new EQApplicationPacket();
	CreateDeSpawnPacket(app);
	CreateSpawnPacket(app2);
	entity_list.QueueClients(0, app);
	entity_list.QueueClients(0, app2);
	safe_delete(app);
	safe_delete(app2);
}

void Object::SetTiltY(float pos)
{
	m_data.tilt_y = pos;

	auto app = new EQApplicationPacket();
	auto app2 = new EQApplicationPacket();
	CreateDeSpawnPacket(app);
	CreateSpawnPacket(app2);
	entity_list.QueueClients(0, app);
	entity_list.QueueClients(0, app2);
	safe_delete(app);
	safe_delete(app2);
}

void Object::SetDisplayName(const char *in_name)
{
	strn0cpy(m_display_name, in_name, 64);
}

void Object::Depop()
{
	auto app = new EQApplicationPacket();
	CreateDeSpawnPacket(app);
	entity_list.QueueClients(0, app);
	safe_delete(app);
	entity_list.RemoveObject(GetID());
}

void Object::Repop()
{
	auto app = new EQApplicationPacket();
	auto app2 = new EQApplicationPacket();
	CreateDeSpawnPacket(app);
	CreateSpawnPacket(app2);
	entity_list.QueueClients(0, app);
	entity_list.QueueClients(0, app2);
	safe_delete(app);
	safe_delete(app2);
}



void Object::SetZ(float pos)
{
	m_data.z = pos;

	auto app = new EQApplicationPacket();
	auto app2 = new EQApplicationPacket();
	CreateDeSpawnPacket(app);
	CreateSpawnPacket(app2);
	entity_list.QueueClients(0, app);
	entity_list.QueueClients(0, app2);
	safe_delete(app);
	safe_delete(app2);
}

void Object::SetModelName(const char* modelname)
{
	strn0cpy(m_data.object_name, modelname, sizeof(m_data.object_name)); // 32 is the max for chars in object_name, this should be safe
	auto app = new EQApplicationPacket();
	auto app2 = new EQApplicationPacket();
	CreateDeSpawnPacket(app);
	CreateSpawnPacket(app2);
	entity_list.QueueClients(0, app);
	entity_list.QueueClients(0, app2);
	safe_delete(app);
	safe_delete(app2);
}

void Object::SetSize(float size)
{
	m_data.size = size;
	auto app = new EQApplicationPacket();
	auto app2 = new EQApplicationPacket();
	CreateDeSpawnPacket(app);
	CreateSpawnPacket(app2);
	entity_list.QueueClients(0, app);
	entity_list.QueueClients(0, app2);
	safe_delete(app);
	safe_delete(app2);
}

void Object::SetSolidType(uint16 solidtype)
{
	m_data.solid_type = solidtype;
	auto app = new EQApplicationPacket();
	auto app2 = new EQApplicationPacket();
	CreateDeSpawnPacket(app);
	CreateSpawnPacket(app2);
	entity_list.QueueClients(0, app);
	entity_list.QueueClients(0, app2);
	safe_delete(app);
	safe_delete(app2);
}

float Object::GetSize()
{
	return m_data.size;
}

uint16 Object::GetSolidType()
{
	return m_data.solid_type;
}

const char* Object::GetModelName()
{
	return m_data.object_name;
}

void Object::SetIcon(uint32 icon)
{
	m_icon = icon;
}

uint32 Object::GetItemID()
{
	if (m_inst == 0)
	{
		return 0;
	}

	const EQ::ItemData* item = m_inst->GetItem();

	if (item == 0)
	{
		return 0;
	}

	return item->ID;
}

void Object::SetItemID(uint32 itemid)
{
	safe_delete(m_inst);

	if (itemid)
	{
		m_inst = database.CreateItem(itemid);
	}
}

void Object::GetObjectData(Object_Struct* Data)
{
	if (Data)
	{
		memcpy(Data, &m_data, sizeof(m_data));
	}
}

void Object::SetObjectData(Object_Struct* Data)
{
	if (Data)
	{
		memcpy(&m_data, Data, sizeof(m_data));
	}
}

void Object::GetLocation(float* x, float* y, float* z)
{
	if (x)
	{
		*x = m_data.x;
	}

	if (y)
	{
		*y = m_data.y;
	}

	if (z)
	{
		*z = m_data.z;
	}
}

void Object::SetLocation(float x, float y, float z)
{
	m_data.x = x;
	m_data.y = y;
	m_data.z = z;
	auto app = new EQApplicationPacket();
	auto app2 = new EQApplicationPacket();
	CreateDeSpawnPacket(app);
	CreateSpawnPacket(app2);
	entity_list.QueueClients(0, app);
	entity_list.QueueClients(0, app2);
	safe_delete(app);
	safe_delete(app2);
}

void Object::GetHeading(float* heading)
{
	if (heading)
	{
		*heading = m_data.heading;
	}
}

void Object::SetHeading(float heading)
{
	m_data.heading = heading;
	auto app = new EQApplicationPacket();
	auto app2 = new EQApplicationPacket();
	CreateDeSpawnPacket(app);
	CreateSpawnPacket(app2);
	entity_list.QueueClients(0, app);
	entity_list.QueueClients(0, app2);
	safe_delete(app);
	safe_delete(app2);
}

bool Object::ClearEntityVariables()
{
	if (o_EntityVariables.empty()) {
		return false;
	}

	o_EntityVariables.clear();
	return true;
}

bool Object::DeleteEntityVariable(std::string variable_name)
{
	if (o_EntityVariables.empty() || variable_name.empty()) {
		return false;
	}

	auto v = o_EntityVariables.find(variable_name);
	if (v == o_EntityVariables.end()) {
		return false;
	}

	o_EntityVariables.erase(v);
	return true;
}

std::string Object::GetEntityVariable(std::string variable_name)
{
	if (o_EntityVariables.empty() || variable_name.empty()) {
		return std::string();
	}

	const auto& v = o_EntityVariables.find(variable_name);
	if (v != o_EntityVariables.end()) {
		return v->second;
	}

	return std::string();
}

std::vector<std::string> Object::GetEntityVariables()
{
	std::vector<std::string> l;
	if (o_EntityVariables.empty()) {
		return l;
	}

	for (const auto& v : o_EntityVariables) {
		l.push_back(v.first);
	}

	return l;
}

bool Object::EntityVariableExists(std::string variable_name)
{
	if (o_EntityVariables.empty() || variable_name.empty()) {
		return false;
	}

	const auto& v = o_EntityVariables.find(variable_name);
	if (v != o_EntityVariables.end()) {
		return true;
	}

	return false;
}

void Object::SetEntityVariable(std::string variable_name, std::string variable_value)
{
	if (variable_name.empty()) {
		return;
	}

	o_EntityVariables[variable_name] = variable_value;
}

void Object::FixZ()
{
	float best_z = BEST_Z_INVALID;
	if (zone->zonemap != nullptr) {
		auto dest = glm::vec3(m_data.x, m_data.y, m_data.z + 5);
		best_z = zone->zonemap->FindBestZ(dest, nullptr);
		if (best_z != BEST_Z_INVALID) {
			m_data.z = best_z;
		}
	}
}

std::string ObjectType::GetName(uint32 object_type)
{
	return IsValid(object_type) ? object_types[object_type] : "UNKNOWN OBJECT TYPE";
}

bool ObjectType::IsValid(uint32 object_type)
{
	return object_types.find(object_type) != object_types.end();
}
