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

#include "tob.h"
#include "tob_structs.h"

#include "common/global_define.h"
#include "common/eqemu_config.h"
#include "common/eqemu_logsys.h"
#include "common/opcodemgr.h"

#include "common/eq_stream_ident.h"
#include "common/crc32.h"

#include "common/eq_packet_structs.h"
#include "common/misc_functions.h"
#include "common/strings.h"
#include "common/inventory_profile.h"
#include "common/rulesys.h"
#include "common/path_manager.h"
#include "common/classes.h"
#include "common/packet_dump.h"
#include "common/races.h"
#include "common/raid.h"
#include "world/sof_char_create_data.h"
#include "zone/client.h"
#include "zone/mob.h"
#include "zone/string_ids.h"

#include <iostream>
#include <sstream>
#include <numeric>
#include <cassert>
#include <cinttypes>
#include <set>

namespace TOB
{
	static const char* name = "TOB";
	static OpcodeManager* opcodes = nullptr;
	static Strategy struct_strategy;

	void SerializeItem(SerializeBuffer &buffer, const EQ::ItemInstance* inst, int16 slot_id, uint8 depth, ItemPacketType packet_type);

	// message link converters
	static void ServerToTOBConvertLinks(std::string& message_out, const std::string& message_in);
	static void TOBToServerConvertLinks(std::string& message_out, const std::string& message_in);

	// SpawnAppearance
	static inline uint32 ServerToTOBSpawnAppearanceType(uint32 server_type);
	static inline uint32 TOBToServerSpawnAppearanceType(uint32 tob_type);

	// server to client inventory location converters
	static inline structs::InventorySlot_Struct ServerToTOBSlot(uint32 server_slot);
	static inline structs::InventorySlot_Struct ServerToTOBCorpseSlot(uint32 server_corpse_slot);
	static inline uint32 ServerToTOBCorpseMainSlot(uint32 server_corpse_slot);
	static inline structs::TypelessInventorySlot_Struct ServerToTOBTypelessSlot(uint32 server_slot, int16 server_type);

	// client to server inventory location converters
	static inline uint32 TOBToServerSlot(structs::InventorySlot_Struct tob_slot);
	static inline uint32 TOBToServerCorpseSlot(structs::InventorySlot_Struct tob_corpse_slot);
	static inline uint32 TOBToServerCorpseMainSlot(uint32 tob_corpse_slot);
	static inline uint32 TOBToServerTypelessSlot(structs::TypelessInventorySlot_Struct tob_slot, int16 tob_type);
	static inline structs::InventorySlot_Struct TOBCastingInventorySlotToInventorySlot(structs::CastSpellInventorySlot_Struct tob_slot);
	static inline structs::CastSpellInventorySlot_Struct TOBInventorySlotToCastingInventorySlot(structs::InventorySlot_Struct tob_slot);

	// Item packet types
	static item::ItemPacketType ServerToTOBItemPacketType(ItemPacketType tob_type);

	// casting slots
	static inline spells::CastingSlot ServerToTOBCastingSlot(EQ::spells::CastingSlot slot);
	static inline EQ::spells::CastingSlot TOBToServerCastingSlot(spells::CastingSlot slot);

	// buff slots
	static inline int TOBToServerBuffSlot(int index);

	void Register(EQStreamIdentifier& into)
	{
		//create our opcode manager if we havent already
		if (opcodes == nullptr) {

			std::string opfile = fmt::format("{}/patch_{}.conf", PathManager::Instance()->GetPatchPath(), name);

			//load up the opcode manager.
			//TODO: figure out how to support shared memory with multiple patches...
			opcodes = new RegularOpcodeManager();
			if (!opcodes->LoadOpcodes(opfile.c_str())) {
				LogNetcode("[OPCODES] Error loading opcodes file [{}]. Not registering patch [{}]", opfile.c_str(), name);
				return;
			}
		}

		//ok, now we have what we need to register.

		EQStreamInterface::Signature signature;
		std::string pname;

		//register our world signature.
		pname = std::string(name) + "_world";
		signature.ignore_eq_opcode = 0;
		signature.first_length = sizeof(structs::LoginInfo_Struct);
		signature.first_eq_opcode = opcodes->EmuToEQ(OP_SendLoginInfo);
		into.RegisterPatch(signature, pname.c_str(), &opcodes, &struct_strategy);

		//register our zone signature.
		pname = std::string(name) + "_zone";
		signature.ignore_eq_opcode = opcodes->EmuToEQ(OP_AckPacket);
		signature.first_length = sizeof(structs::ClientZoneEntry_Struct);
		signature.first_eq_opcode = opcodes->EmuToEQ(OP_ZoneEntry);
		into.RegisterPatch(signature, pname.c_str(), &opcodes, &struct_strategy);
		LogNetcode("[StreamIdentify] Registered patch [{}]", name);
	}

	void Reload()
	{
		//we have a big problem to solve here when we switch back to shared memory
		//opcode managers because we need to change the manager pointer, which means
		//we need to go to every stream and replace it's manager.

		if (opcodes != nullptr) {
			std::string opfile = fmt::format("{}/patch_{}.conf", PathManager::Instance()->GetPatchPath(), name);
			if (!opcodes->ReloadOpcodes(opfile.c_str())) {
				LogNetcode("[OPCODES] Error reloading opcodes file [{}] for patch [{}]", opfile.c_str(), name);
				return;
			}
			LogNetcode("[OPCODES] Reloaded opcodes for patch [{}]", name);
		}
	}

	Strategy::Strategy() : StructStrategy()
	{
		//all opcodes default to passthrough.
#include "ss_register.h"
#include "tob_ops.h"

	}

	std::string Strategy::Describe() const
	{
		std::string r;
		r += "Patch ";
		r += name;
		return(r);
	}

	const EQ::versions::ClientVersion Strategy::ClientVersion() const
	{
		return EQ::versions::ClientVersion::TOB;
	}

#include "ss_define.h"

	// ENCODE methods
	ENCODE(OP_AAExpUpdate) {
		ENCODE_LENGTH_EXACT(AltAdvStats_Struct);
		SETUP_DIRECT_ENCODE(AltAdvStats_Struct, structs::AltAdvStats_Struct);

		//later we should change the underlying server to use this more accurate value
		//and encode the 330 in the other patches
		eq->experience = emu->experience * 100000 / 330;

		OUT(unspent);
		OUT(percentage);

		FINISH_ENCODE();
	}

	ENCODE(OP_Action) {
		ENCODE_LENGTH_EXACT(Action_Struct);
		SETUP_DIRECT_ENCODE(Action_Struct, structs::MissileHitInfo);

		//This is mostly figured out; there's two unknowns, only unknown1 is read by the client
		OUT(target);
		OUT(source);
		eq->spell_id = emu->spell;
		eq->effect_type = emu->effect_flag;
		eq->effective_casting_level = 0; //if you set this to != 0 it will use this level instead of calculating it
		eq->unknown1 = 0;
		eq->unknown2 = 0;
		eq->damage = 0; //client doesn't read this but live sends it here, can just set 0
		eq->modifier = 1.0f + (emu->instrument_mod - 10) / 10.0f;
		OUT(force);
		OUT(hit_heading);
		OUT(hit_pitch);
		eq->skill = emu->type;
		OUT(level);

		FINISH_ENCODE();
	}

	ENCODE(OP_Animation)
	{
		ENCODE_LENGTH_EXACT(Animation_Struct);
		SETUP_DIRECT_ENCODE(Animation_Struct, structs::Animation_Struct);

		OUT(spawnid);
		OUT(action);
		OUT(speed);

		FINISH_ENCODE();
	}

	ENCODE(OP_ApplyPoison)
	{
		ENCODE_LENGTH_EXACT(ApplyPoison_Struct);
		SETUP_DIRECT_ENCODE(ApplyPoison_Struct, structs::ApplyPoison_Struct);

		eq->inventorySlot = ServerToTOBTypelessSlot(emu->inventorySlot, EQ::invtype::typePossessions);
		OUT(success);

		FINISH_ENCODE();
	}

	ENCODE(OP_AugmentInfo)
	{
		ENCODE_LENGTH_EXACT(AugmentInfo_Struct);
		SETUP_DIRECT_ENCODE(AugmentInfo_Struct, structs::AugmentInfo_Struct);

		OUT(itemid);
		OUT(window);
		strn0cpy(eq->augment_info, emu->augment_info, 64);

		FINISH_ENCODE();
	}

	ENCODE(OP_Begging)
	{
		ENCODE_LENGTH_EXACT(BeggingResponse_Struct);
		SETUP_DIRECT_ENCODE(BeggingResponse_Struct, structs::BeggingResponse_Struct);

		OUT(Unknown00);
		OUT(Unknown04);
		OUT(Unknown08);
		eq->Result = static_cast<uint8>(emu->Result);
		OUT(Amount);
		eq->StringSize = 0; // set this to 0, but it's a string size
		eq->Lucky = 0; // set to 1 to message a lucky beg

		FINISH_ENCODE();
	}

	ENCODE(OP_BeginCast)
	{
		ENCODE_LENGTH_EXACT(BeginCast_Struct);
		SETUP_DIRECT_ENCODE(BeginCast_Struct, structs::BeginCast_Struct);

		OUT(spell_id);
		OUT(caster_id);
		OUT(cast_time);
		eq->unknown0e = 1; //not sure what this is; but its usually 1 on live

		FINISH_ENCODE();
	}

	ENCODE(OP_BlockedBuffs)
	{
		// Blocked buffs are a major change. They are stored in a resizable array in TOB, so this sends size, then
		// spells, then the final two bools -- see 0x140202750
		SETUP_VAR_ENCODE(BlockedBuffs_Struct);

		// size is uint32 + count * int32 + uint8 + uint8
		uint32 sz = 6 + emu->Count * 4;
		__packet->size = sz;
		__packet->pBuffer = new unsigned char[sz];
		memset(__packet->pBuffer, 0, sz);

		__packet->WriteUInt32(emu->Count);
		for (int i = 0; i < emu->Count; i++)
			__packet->WriteSInt32(emu->SpellID[i]);

		__packet->WriteUInt8(emu->Pet);
		__packet->WriteUInt8(emu->Initialise);

		FINISH_ENCODE();
	}

	ENCODE(OP_CancelTrade)
	{
		ENCODE_LENGTH_EXACT(CancelTrade_Struct);
		SETUP_DIRECT_ENCODE(CancelTrade_Struct, structs::CancelTrade_Struct);

		OUT(fromid);
		OUT(action);

		FINISH_ENCODE();
	}

	ENCODE(OP_CastSpell)
	{
		// I don't think the client handles this at all, it only sends the cast packet
		ENCODE_LENGTH_EXACT(CastSpell_Struct);
		SETUP_DIRECT_ENCODE(CastSpell_Struct, structs::CastSpell_Struct);

		eq->slot = static_cast<uint32>(ServerToTOBCastingSlot(static_cast<EQ::spells::CastingSlot>(emu->slot)));

		OUT(spell_id);
		//we should double check this cause it feels wrong
		eq->inventory_slot = TOBInventorySlotToCastingInventorySlot(ServerToTOBSlot(emu->inventoryslot));
		//OUT(inventoryslot);
		OUT(target_id);

		FINISH_ENCODE();
	}

	ENCODE(OP_ChannelMessage)
	{
		EQApplicationPacket* in = *p;
		*p = nullptr;

		ChannelMessage_Struct* emu = (ChannelMessage_Struct*)in->pBuffer;

		unsigned char* __emu_buffer = in->pBuffer;

		std::string old_message = emu->message;
		std::string new_message;
		ServerToTOBConvertLinks(new_message, old_message);

		in->size = strlen(emu->sender) + strlen(emu->targetname) + new_message.length() + 43;

		in->pBuffer = new unsigned char[in->size];

		char* OutBuffer = (char*)in->pBuffer;

		VARSTRUCT_ENCODE_STRING(OutBuffer, emu->sender);
		VARSTRUCT_ENCODE_STRING(OutBuffer, emu->targetname);
		VARSTRUCT_ENCODE_TYPE(uint64, OutBuffer, 0);	// Unknown
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->language);
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->chan_num);
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);	// Unknown
		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, 0);	// Unknown
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->skill_in_language);
		VARSTRUCT_ENCODE_STRING(OutBuffer, new_message.c_str());

		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, 0); // Unknown
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);// Unknown
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);// Unknown

		VARSTRUCT_ENCODE_STRING(OutBuffer, "");
		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, 0); // Unknown
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);// Unknown

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_CharacterCreateRequest) {
		EQApplicationPacket* in = *p;
		*p = nullptr;

		// need to calculate the size of the buffer
		uint8* inptr = in->pBuffer;
		inptr += sizeof(uint8);

		uint32 allocs = *(uint32*)inptr; // allocations count
		inptr += sizeof(uint32) + sizeof(RaceClassAllocation) * allocs;

		uint32 combos = *(uint32*)inptr; // combos count

		SerializeBuffer buf(sizeof(uint8) + 2 * sizeof(uint32) +
			allocs * sizeof(structs::RaceClassAllocation) +
			combos * sizeof(structs::RaceClassCombos));

		// write the modified contents to the buffer
		buf.WriteUInt8(in->ReadUInt8());

		buf.WriteUInt32(in->ReadUInt32()); // allocations
		for (uint32 i = 0; i < allocs; i++) {
			// RaceClassAllocations is 15 uint32's
			for (int j = 0; j < 15; ++j)
				buf.WriteUInt32(in->ReadUInt32());
		}

		buf.WriteUInt32(in->ReadUInt32()); // combos
		for (uint32 i = 0; i < combos; ++i) {
			buf.WriteUInt64(in->ReadUInt32()); // expansion required -- only actual conversion
			for (int j = 0; j < 5; ++j)
				buf.WriteUInt32(in->ReadUInt32());
		}

		// will need to delete this after we swap, or it will leak
		uchar* emu_buffer = in->pBuffer;

		// swap into in
		in->size = buf.size();
		in->pBuffer = new uint8[buf.size()];
		memcpy(in->pBuffer, buf.buffer(), buf.size());

		delete[] emu_buffer;

		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_CharInventory) {
		//consume the packet
		EQApplicationPacket* in = *p;
		*p = nullptr;

		if (!in->size) {
			in->size = 4;
			in->pBuffer = new uchar[in->size];
			memset(in->pBuffer, 0, in->size);

			dest->FastQueuePacket(&in, ack_req);
			return;
		}

		//store away the emu struct
		uchar* __emu_buffer = in->pBuffer;

		int item_count = in->size / sizeof(EQ::InternalSerializedItem_Struct);
		if (!item_count || (in->size % sizeof(EQ::InternalSerializedItem_Struct)) != 0) {
			LogNetcode("[STRUCTS] Wrong size on outbound {}: Got {}, expected multiple of {}",
				opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(EQ::InternalSerializedItem_Struct));

			delete in;
			return;
		}

		EQ::InternalSerializedItem_Struct* eq = (EQ::InternalSerializedItem_Struct*)in->pBuffer;
		SerializeBuffer buffer;
		buffer.WriteUInt32(item_count);

		for (int index = 0; index < item_count; ++index, ++eq) {
			SerializeItem(buffer, (const EQ::ItemInstance*)eq->inst, eq->slot_id, 0, ItemPacketCharInventory);
		}

		in->size = buffer.size();
		in->pBuffer = new unsigned char[buffer.size()];
		memcpy(in->pBuffer, buffer.buffer(), buffer.size());

		delete[] __emu_buffer;

		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_ClickObjectAction)
	{
		ENCODE_LENGTH_EXACT(ClickObjectAction_Struct);
		SETUP_DIRECT_ENCODE(ClickObjectAction_Struct, structs::ClickObjectAction_Struct);

		OUT(drop_id);
		eq->unknown04 = -1;
		eq->unknown08 = -1;
		OUT(type);
		OUT(icon);
		eq->unknown16 = 0;
		OUT_str(object_name);

		FINISH_ENCODE();
	}

	ENCODE(OP_ClientUpdate)
	{
		ENCODE_LENGTH_EXACT(PlayerPositionUpdateServer_Struct);
		SETUP_DIRECT_ENCODE(PlayerPositionUpdateServer_Struct, structs::PlayerPositionUpdateServer_Struct);

		OUT(spawn_id);
		OUT(vehicle_id);
		eq->position.x = emu->x_pos;
		eq->position.y = emu->y_pos;
		eq->position.z = emu->z_pos;
		eq->position.heading = emu->heading;
		eq->position.deltaX = emu->delta_x;
		eq->position.deltaY = emu->delta_y;
		eq->position.deltaZ = emu->delta_z;
		eq->position.deltaHeading = emu->delta_heading;
		eq->position.animation = emu->animation;

		FINISH_ENCODE();
	}

	ENCODE(OP_Consider)
	{
		ENCODE_LENGTH_EXACT(Consider_Struct);
		SETUP_DIRECT_ENCODE(Consider_Struct, structs::Consider_Struct);

		OUT(playerid);
		OUT(targetid);
		OUT(faction);
		OUT(level);

		FINISH_ENCODE();
	}

	ENCODE(OP_Damage) {
		SETUP_DIRECT_ENCODE(CombatDamage_Struct, structs::CombatDamage_Struct);

		OUT(target);
		OUT(source);
		OUT(type);
		OUT(spellid);
		OUT(damage);
		OUT(force);
		OUT(hit_heading);
		OUT(hit_pitch);
		OUT(special);

		FINISH_ENCODE();
	}

	ENCODE(OP_Death)
	{
		ENCODE_LENGTH_EXACT(Death_Struct);
		SETUP_DIRECT_ENCODE(Death_Struct, structs::Death_Struct);

		OUT(spawn_id);
		OUT(killer_id);
		OUT(spell_id);
		OUT(attack_skill);
		OUT(damage);

		//This is a hack, we need to actually fix the ordering in source as this wont respect filters etc
		if (emu->attack_skill != 231) {
			auto combat_packet = new EQApplicationPacket(OP_Damage, sizeof(structs::CombatDamage_Struct));
			structs::CombatDamage_Struct* cds = (structs::CombatDamage_Struct*)combat_packet->pBuffer;

			cds->target = emu->spawn_id;
			cds->source = emu->killer_id;
			cds->type = emu->attack_skill;
			cds->damage = emu->damage;
			cds->spellid = -1;

			dest->FastQueuePacket(&combat_packet, ack_req);
		}

		FINISH_ENCODE();
	}

	ENCODE(OP_DeleteCharge)
	{
		Log(Logs::Detail, Logs::Netcode, "TOB::ENCODE(OP_DeleteCharge)");

		ENCODE_FORWARD(OP_MoveItem);
	}

	ENCODE(OP_DeleteItem)
	{
		ENCODE_LENGTH_EXACT(DeleteItem_Struct);
		SETUP_DIRECT_ENCODE(DeleteItem_Struct, structs::DeleteItem_Struct);

		eq->from_slot = ServerToTOBSlot(emu->from_slot);
		eq->to_slot = ServerToTOBSlot(emu->to_slot);
		OUT(number_in_stack);

		FINISH_ENCODE();
	}

	ENCODE(OP_DeleteSpawn)
	{
		ENCODE_LENGTH_EXACT(DeleteSpawn_Struct);
		SETUP_DIRECT_ENCODE(DeleteSpawn_Struct, structs::DeleteSpawn_Struct);

		OUT(spawn_id);
		eq->unknown04 = 1;	// Observed

		FINISH_ENCODE();
	}

	ENCODE(OP_DisciplineUpdate)
	{
		ENCODE_LENGTH_EXACT(Disciplines_Struct);
		SETUP_DIRECT_ENCODE(Disciplines_Struct, structs::Disciplines_Struct);

		memcpy(&eq->values, &emu->values, sizeof(Disciplines_Struct));

		FINISH_ENCODE();
	}

	ENCODE(OP_ExpansionInfo)
	{
		ENCODE_LENGTH_EXACT(ExpansionInfo_Struct);
		SETUP_DIRECT_ENCODE(ExpansionInfo_Struct, structs::ExpansionInfo_Struct);

		OUT(Expansions);

		FINISH_ENCODE();
	}

	ENCODE(OP_ExpUpdate)
	{
		SETUP_DIRECT_ENCODE(ExpUpdate_Struct, structs::ExpUpdate_Struct);

		//later we should change the underlying server to use this more accurate value
		//and encode the 330 in the other patches
		eq->exp = emu->exp * 100000 / 330;

		FINISH_ENCODE();
	}

	ENCODE(OP_GMTraining) {
		ENCODE_LENGTH_EXACT(GMTrainee_Struct);
		SETUP_DIRECT_ENCODE(GMTrainee_Struct, structs::GMTrainee_Struct);

		OUT(npcid);
		OUT(playerid);

		for (int i = 0; i < 100; ++i) {
			OUT(skills[i]);
		}

		eq->unknown408[0] = 1; // this tells the client to update the GM window
		eq->unknown408[1] = 0xC9; // these appear to be languages, but it's only testing if this is non-zero
		eq->unknown408[2] = 0xC9;
		eq->unknown408[3] = 0xC9;
		eq->unknown408[4] = 0xC9;
		eq->unknown408[5] = 0xC9;
		eq->unknown408[6] = 0xC9;
		eq->unknown408[7] = 0xC9;
		eq->unknown408[8] = 0xC9;
		eq->unknown408[9] = 0xC9;
		eq->unknown408[10] = 0xC9;
		eq->unknown408[11] = 0xC9;
		eq->unknown408[12] = 0xC9;
		eq->unknown408[13] = 0xC9;
		eq->unknown408[14] = 0xC9;
		eq->unknown408[15] = 0xC9;
		eq->unknown408[16] = 0xC9;
		eq->unknown408[17] = 0xC9;
		eq->unknown408[18] = 0xC9;
		eq->unknown408[19] = 0xC9;
		eq->unknown408[20] = 0xC9;
		eq->unknown408[21] = 0xC9;
		eq->unknown408[22] = 0xC9;
		eq->unknown408[23] = 0xC9;
		eq->unknown408[24] = 0xC9;
		eq->unknown408[25] = 0xC9;
		eq->unknown408[26] = 0xC9;
		eq->unknown408[27] = 0xC9;
		eq->unknown408[28] = 0xC9;
		eq->unknown408[29] = 0xC9;
		eq->unknown408[30] = 0xC9;
		eq->unknown408[31] = 0xC9;
		eq->unknown408[32] = 0xC9;
		eq->unknown408[33] = 0xCA; //the client far as I can tell doesn't read past the 1 byte + 32 0xc9s, but still setting it to what we saw
		eq->unknown408[34] = 0x8C;
		eq->unknown408[35] = 0xEC;

		FINISH_ENCODE();
	}

	ENCODE(OP_GMTrainSkillConfirm)
	{
		ENCODE_LENGTH_EXACT(GMTrainSkillConfirm_Struct);
		SETUP_DIRECT_ENCODE(GMTrainSkillConfirm_Struct, structs::GMTrainSkillConfirm_Struct);

		OUT(SkillID);
		OUT(Cost);
		OUT(NewSkill);
		OUT_str(TrainerName);

		FINISH_ENCODE();
	}

	ENCODE(OP_GroundSpawn)
	{
		EQApplicationPacket* in = *p;
		*p = nullptr;
		Object_Struct* emu = (Object_Struct*)in->pBuffer;

		SerializeBuffer buffer;
		buffer.WriteUInt32(emu->drop_id);
		buffer.WriteString(emu->object_name);
		buffer.WriteUInt16(emu->zone_id);
		buffer.WriteUInt16(emu->zone_instance);
		buffer.WriteUInt32(emu->drop_id); //this is some other sub but it's okay to duplicate
		buffer.WriteUInt32(0); //expires
		buffer.WriteFloat(emu->heading);
		buffer.WriteFloat(emu->tilt_x);
		buffer.WriteFloat(emu->tilt_y);
		buffer.WriteFloat(emu->size != 0 && (float)emu->size < 5000.f ? (float)((float)emu->size / 100.0f) : 1.f); //size, with weird peq hack
		buffer.WriteFloat(emu->y);
		buffer.WriteFloat(emu->x);
		buffer.WriteFloat(emu->z);
		buffer.WriteFloat(emu->object_type); //weight

		auto outapp = new EQApplicationPacket(OP_GroundSpawn, buffer.size());
		outapp->WriteData(buffer.buffer(), buffer.size());
		dest->FastQueuePacket(&outapp, ack_req);

		delete in;
	}

	ENCODE(OP_HPUpdate)
	{
		SETUP_DIRECT_ENCODE(SpawnHPUpdate_Struct, structs::SpawnHPUpdate_Struct);

		OUT(spawn_id);
		OUT(cur_hp);
		OUT(max_hp);

		FINISH_ENCODE();
	}

	ENCODE(OP_Illusion)
	{
		ENCODE_LENGTH_EXACT(Illusion_Struct);
		SETUP_DIRECT_ENCODE(Illusion_Struct, structs::Illusion_Struct);

		OUT(spawnid);
		OUT_str(charname);
		OUT(race);
		OUT(gender);
		OUT(texture);
		OUT(helmtexture);
		OUT(face);
		OUT(hairstyle);
		OUT(haircolor);
		OUT(beard);
		OUT(beardcolor);
		OUT(size);
		OUT(drakkin_heritage);
		OUT(drakkin_tattoo);
		OUT(drakkin_details);

		FINISH_ENCODE();
	}

	ENCODE(OP_ItemPacket)
	{
		EQApplicationPacket* in = *p;
		*p = nullptr;
		uchar* __emu_buffer = in->pBuffer;
		ItemPacket_Struct* old_item_pkt = (ItemPacket_Struct*)__emu_buffer;

		auto type = ServerToTOBItemPacketType(old_item_pkt->PacketType);
		if (type == item::ItemPacketType::ItemPacketInvalid) {
			delete in;
			return;
		}

		switch (type)
		{
		case item::ItemPacketType::ItemPacketParcel: {
			ParcelMessaging_Struct       pms{};
			EQ::Util::MemoryStreamReader ss(reinterpret_cast<char*>(in->pBuffer), in->size);
			cereal::BinaryInputArchive   ar(ss);
			ar(pms);

			uint32 player_name_length = pms.player_name.length();
			uint32 note_length = pms.note.length();

			auto* int_struct = (EQ::InternalSerializedItem_Struct*)pms.serialized_item.data();

			SerializeBuffer buffer;
			buffer.WriteInt32((int32_t)type);
			SerializeItem(buffer, (const EQ::ItemInstance*)int_struct->inst, int_struct->slot_id, 0, old_item_pkt->PacketType);

			buffer.WriteUInt32(pms.sent_time);
			buffer.WriteLengthString(pms.player_name);
			buffer.WriteLengthString(pms.note);

			auto outapp = new EQApplicationPacket(OP_ItemPacket, buffer.size());
			outapp->WriteData(buffer.buffer(), buffer.size());
			dest->FastQueuePacket(&outapp, ack_req);
			break;
		}
		default: {
			EQ::InternalSerializedItem_Struct* int_struct = (EQ::InternalSerializedItem_Struct*)(&__emu_buffer[4]);
			SerializeBuffer buffer;
			buffer.WriteInt32((int32_t)type);
			SerializeItem(buffer, (const EQ::ItemInstance*)int_struct->inst, int_struct->slot_id, 0, old_item_pkt->PacketType);

			auto outapp = new EQApplicationPacket(OP_ItemPacket, buffer.size());
			outapp->WriteData(buffer.buffer(), buffer.size());
			dest->FastQueuePacket(&outapp, ack_req);
		}
		}

		delete in;
	}

	ENCODE(OP_LogServer) {
		SETUP_VAR_ENCODE(LogServer_Struct);
		ALLOC_LEN_ENCODE(1932);

		// pvp
		if (emu->enable_pvp) {
			*(char*)&__packet->pBuffer[0x04] = 1;
		}

		if (emu->enable_FV) {
			*(char*)&__packet->pBuffer[0x08] = 1; // RP server
			*(char*)&__packet->pBuffer[0x0a] = 1; // free loot server
		}

		// this lets you transfer no drop items in the shared bank
		*(char*)&__packet->pBuffer[0x75d] = 0;

		// disable tutorial at character create/select
		*(char*)&__packet->pBuffer[0x09] = 0;

		// this is the auto-identify flag
		*(char*)&__packet->pBuffer[0x0b] = 0;

		// not actually used in the client, has to do with name gen
		*(char*)&__packet->pBuffer[0x0c] = 1;

		// unknown languages are gibberish
		*(char*)&__packet->pBuffer[0x0d] = 1;

		// is_dev_server flags
		*(char*)&__packet->pBuffer[0x600] = 0; // is beta server
		*(char*)&__packet->pBuffer[0x601] = 0; // override allow beta buff (for any server)
		*(char*)&__packet->pBuffer[0x602] = 0; // is test server (name reservations are unavailable)
		*(char*)&__packet->pBuffer[0x603] = 0; // unused in the client

		//world short name
		strncpy((char*)&__packet->pBuffer[0x15], emu->worldshortname, 32);

		// static base HP/MP regen
		*(char*)&__packet->pBuffer[0x5ec] = 0;

		// use mail system
		if (emu->enablemail) {
			*(char*)&__packet->pBuffer[0x5f5] = 1;
		}

		// use voice macros
		if (emu->enablevoicemacros) {
			*(char*)&__packet->pBuffer[0x5f4] = 1;
		}

		// Disable character select buttons except create character
		*(char*)&__packet->pBuffer[0x5f6] = 0;

		// enable tutorial at character create/select
		*(char*)&__packet->pBuffer[0x5f8] = 1;

		// client defaults, unused
		*(int32_t*)&__packet->pBuffer[0x63c] = -1;
		*(int32_t*)&__packet->pBuffer[0x640] = 0;
		*(char*)&__packet->pBuffer[0x745] = 0;
		*(char*)&__packet->pBuffer[0x750] = 1;

		// these are always multiplied together in guild favor calcs for display, live and test send 1.0f
		*(float*)&__packet->pBuffer[0x760] = 1.0f;
		*(float*)&__packet->pBuffer[0x764] = 1.0f;

		// these are always multiplied together in non-guild favor calcs for display, live and test send 1.0f
		*(float*)&__packet->pBuffer[0x768] = 1.0f;
		*(float*)&__packet->pBuffer[0x76c] = 1.0f;

		FINISH_ENCODE();
	}

	ENCODE(OP_ManaChange) {
		ENCODE_LENGTH_EXACT(ManaChange_Struct);
		SETUP_DIRECT_ENCODE(ManaChange_Struct, structs::ManaChange_Struct);

		OUT(new_mana);
		OUT(stamina);
		OUT(spell_id);
		OUT(keepcasting);
		OUT(slot);

		FINISH_ENCODE();
	}

	ENCODE(OP_MemorizeSpell) {
		ENCODE_LENGTH_EXACT(MemorizeSpell_Struct);
		SETUP_DIRECT_ENCODE(MemorizeSpell_Struct, structs::MemorizeSpell_Struct);

		// in TOB, 2 is "finish memming" so that becomes 1 in emu and 3 is "unmem" which becomes 2
		if (emu->scribing == 1)
			eq->scribing = 2;
		else if (emu->scribing == 2)
			eq->scribing = 3;
		else if (emu->scribing == 3)
			eq->scribing = 4;
		else
			OUT(scribing);

		OUT(slot);
		OUT(spell_id);
		OUT(reduction);

		FINISH_ENCODE();
	}

	ENCODE(OP_MobHealth) {
		ENCODE_LENGTH_EXACT(SpawnHPUpdate_Struct2);
		SETUP_DIRECT_ENCODE(SpawnHPUpdate_Struct2, structs::MobHealth_Struct);

		OUT(spawn_id);
		OUT(hp);

		FINISH_ENCODE();
	}

	ENCODE(OP_MoneyOnCorpse)
	{
		ENCODE_LENGTH_EXACT(moneyOnCorpseStruct);
		SETUP_DIRECT_ENCODE(moneyOnCorpseStruct, structs::moneyOnCorpseStruct);

		// TODO: The type has changed to accomodate all kinds of loot options and actions, including advloot
		eq->type = emu->response;
		OUT(platinum);
		OUT(gold);
		OUT(silver);
		OUT(copper);
		eq->flags = 0;

		FINISH_ENCODE();
	}

	ENCODE(OP_MoveItem)
	{
		ENCODE_LENGTH_EXACT(MoveItem_Struct);
		SETUP_DIRECT_ENCODE(MoveItem_Struct, structs::MoveItem_Struct);

		Log(Logs::Detail, Logs::Netcode, "TOB::ENCODE(OP_MoveItem)");

		eq->from_slot = ServerToTOBSlot(emu->from_slot);
		eq->to_slot = ServerToTOBSlot(emu->to_slot);
		OUT(number_in_stack);

		FINISH_ENCODE();
	}

	ENCODE(OP_NewSpawn) { ENCODE_FORWARD(OP_ZoneSpawns); }

	ENCODE(OP_NewZone) {
		// zoneHeader
		EQApplicationPacket* in = *p;
		*p = nullptr;

		unsigned char* __emu_buffer = in->pBuffer;
		NewZone_Struct* emu = (NewZone_Struct*)__emu_buffer;

		SerializeBuffer buffer;
		/*
		char Shortname[];
		char Longname[];
		char WeatherType[];
		char WeatherTypeOverride[];
		char SkyType[];
		char SkyTypeOverride[];
		*/
		buffer.WriteString(emu->zone_short_name);
		buffer.WriteString(emu->zone_long_name);
		buffer.WriteString("");
		buffer.WriteString("");
		buffer.WriteString("");
		buffer.WriteString("");

		/*
		s32 ZoneType;
		s32 ZoneId; // combine id + instance
		float ZoneExpModifier;
		s32 GroupLvlExpRelated;
		s32 FilterID;
		s32 Unknown1;
		*/
		buffer.WriteInt32(emu->ztype);
		buffer.WriteUInt16(emu->zone_id);
		buffer.WriteUInt16(emu->zone_instance);
		buffer.WriteFloat(emu->zone_exp_multiplier);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);

		//float FogDensity;
		buffer.WriteFloat(emu->fog_density);

		//WeatherState state[4];
		for (int i = 0; i < 4; ++i) {
			/*
			float FogStart;
			float FogEnd;
			u8 FogRed;
			u8 FogGreen;
			u8 FogBlue;
			u8 RainChance;
			u8 RainDuration;
			u8 SnowChance;
			u8 SnowDuration;
			*/

			buffer.WriteFloat(emu->fog_minclip[i]);
			buffer.WriteFloat(emu->fog_maxclip[i]);
			buffer.WriteUInt8(emu->fog_red[i]);
			buffer.WriteUInt8(emu->fog_green[i]);
			buffer.WriteUInt8(emu->fog_blue[i]);
			buffer.WriteUInt8(emu->rain_chance[i]);
			buffer.WriteUInt8(emu->rain_duration[i]);
			buffer.WriteUInt8(emu->snow_chance[i]);
			buffer.WriteUInt8(emu->snow_duration[i]);

		}

		/*
		u8 PrecipitationType;
		float BloomIntensity;
		float ZoneGravity;
		s32 LavaDamage;
		s32 MinLavaDamage;
		*/
		buffer.WriteUInt8(emu->sky);
		buffer.WriteFloat(1.0f);
		buffer.WriteFloat(emu->gravity);
		buffer.WriteInt32(emu->lava_damage);
		buffer.WriteInt32(emu->min_lava_damage);

		/*
		s32 TimeStringID;
		s32 Unknown3;
		s32 SkyLock;
		s32 SkyLockOverride;
		*/
		buffer.WriteInt32(0);
		buffer.WriteInt32(1);
		buffer.WriteInt32(0);
		buffer.WriteInt32(-1);

		/*
		float SafeY;
		float SafeX;
		float SafeZ;
		float SafeHeading;
		float Ceiling;
		float Floor;
		*/

		buffer.WriteFloat(emu->safe_y);
		buffer.WriteFloat(emu->safe_x);
		buffer.WriteFloat(emu->safe_z);
		buffer.WriteFloat(emu->safe_heading);
		buffer.WriteFloat(emu->max_z);
		buffer.WriteFloat(emu->underworld);

		/*
		float MinClip;
		float MaxClip;
		s32 FallThroughWorldTeleportID;
		*/
		buffer.WriteFloat(emu->minclip);
		buffer.WriteFloat(emu->maxclip);
		buffer.WriteInt32(emu->underworld_teleport_index);

		/*
		s32 Unknown4;
		s32 ScriptIDHour;
		s32 ScriptIDMinute;
		s32 ScriptIDTick;
		s32 ScriptIDOnPlayerDeath;
		s32 ScriptIDOnNPCDeath;
		s32 ScriptIDPlayerEnteringZone;
		s32 ScriptIDOnZonePop;
		s32 ScriptIDNPCLoot;
		s32 Unknown4b;
		s32 ScriptIDOnFishing;
		s32 ScriptIDOnForage;
		s32 Unknown4c;
		*/
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);

		/*
		s32 NPCAgroMaxDist;
		*/
		buffer.WriteInt32(600);

		/*
		s32 ForageLow;
		s32 ForageMedium;
		s32 ForageHigh;
		s32 ForageSpecial;
		s32 FishingLow;
		s32 FishingMedium;
		s32 FishingHigh;
		s32 FishingRelated;
		*/
		buffer.WriteInt32(-1);
		buffer.WriteInt32(-1);
		buffer.WriteInt32(-1);
		buffer.WriteInt32(-1);
		buffer.WriteInt32(-1);
		buffer.WriteInt32(-1);
		buffer.WriteInt32(-1);
		buffer.WriteInt32(-1);

		/*
		s32 CanPlaceCampsite;
		s32 CanPlaceGuildBanner;
		s32 Unknown4d;
		*/

		buffer.WriteInt32(2);
		buffer.WriteInt32(2);
		buffer.WriteInt32(0);

		/*
		s32 FastRegenHP;
		s32 FastRegenMana;
		s32 FastRegenEndurance;
		*/
		buffer.WriteInt32(emu->fast_regen_hp);
		buffer.WriteInt32(emu->fast_regen_mana);
		buffer.WriteInt32(emu->fast_regen_endurance);

		/*
		u8 NewEngineZone;
		u8 SkyEnabled;
		u8 FogOnOff;
		u8 ClimateType;
		u8 bNoPlayerLight;
		*/
		buffer.WriteUInt8(0); //not sure what happens if we set this incorrectly but we probably need to add this to the zone database
		buffer.WriteUInt8(1);
		buffer.WriteUInt8(1);
		buffer.WriteUInt8(0);
		buffer.WriteUInt8(0);

		/*
		u8 bUnknown5;
		u8 bNoAttack;
		u8 bAllowPVP;
		u8 bNoEncumber;
		u8 Unknown6;
		u8 Unknown7;
		u8 Unknown7a;
		*/
		buffer.WriteUInt8(1);
		buffer.WriteUInt8(0);
		buffer.WriteUInt8(1);
		buffer.WriteUInt8(0);
		buffer.WriteUInt8(0);
		buffer.WriteUInt8(0);
		buffer.WriteUInt8(0);

		/*
		u8 bNoLevitate;
		u8 bNoBuffExpiration;
		u8 bDisallowManaStone;
		u8 bNoBind;
		u8 bNoCallOfTheHero;
		u8 bUnknown8;
		u8 bNoFear;
		u8 bUnknown9;
		*/

		buffer.WriteUInt8(0);
		buffer.WriteUInt8(emu->suspend_buffs);
		buffer.WriteUInt8(1);
		buffer.WriteUInt8(0);
		buffer.WriteUInt8(0);
		buffer.WriteUInt8(0);
		buffer.WriteUInt8(0);
		buffer.WriteUInt8(0);

		auto outapp = new EQApplicationPacket(OP_NewZone, buffer.size());
		outapp->WriteData(buffer.buffer(), buffer.size());
		dest->FastQueuePacket(&outapp, ack_req);

		delete in;
	}

	ENCODE(OP_OnLevelMessage)
	{
		EQApplicationPacket* in = *p;
		*p = nullptr;
		OnLevelMessage_Struct* emu = (OnLevelMessage_Struct*)in->pBuffer;
		SerializeBuffer buffer;

		buffer.WriteLengthString(emu->Title);
		buffer.WriteLengthString(emu->Text);
		buffer.WriteLengthString(emu->ButtonName0);
		buffer.WriteLengthString(emu->ButtonName1);
		buffer.WriteUInt8(emu->Buttons);
		buffer.WriteUInt8(emu->SoundControls);
		buffer.WriteUInt32(emu->Duration);
		buffer.WriteUInt32(emu->PopupID);
		buffer.WriteUInt32(emu->NegativeID);
		buffer.WriteUInt32(0); //seen -1 & 0
		buffer.WriteUInt32(0); //seen 0

		auto outapp = new EQApplicationPacket(OP_OnLevelMessage, buffer.size());
		outapp->WriteData(buffer.buffer(), buffer.size());
		dest->FastQueuePacket(&outapp, ack_req);

		delete in;
	}

	ENCODE(OP_PlayerProfile) {
		EQApplicationPacket* in = *p;
		*p = nullptr;

		unsigned char* __emu_buffer = in->pBuffer;
		PlayerProfile_Struct* emu = (PlayerProfile_Struct*)__emu_buffer;

		SerializeBuffer out;

		/*
		u32 crc;
		u32 length;
		*/
		out.WriteUInt32(0);
		out.WriteUInt32(0);

		//PcProfile begin
		/*
		u32 profile_type;
		u32 profile_id;
		u32 shroud_template_id;
		*/
		out.WriteUInt32(0);
		out.WriteUInt32(0);
		out.WriteUInt32(0);

		/*
		u8 gender;
		u32 race;
		u32 class;
		u8 level;
		u8 level1;
		*/
		out.WriteUInt8(emu->gender);
		out.WriteUInt32(emu->race);
		out.WriteUInt32(emu->class_);
		out.WriteUInt8(emu->level);
		out.WriteUInt8(emu->level);

		//u32 bind_count;
		out.WriteUInt32(5);

		for (int r = 0; r < 5; r++)
		{
			/*
			u32 zoneid;
			float x;
			float y;
			float z;
			float heading;
			*/
			out.WriteUInt32(emu->binds[r].zone_id);
			out.WriteFloat(emu->binds[r].x);
			out.WriteFloat(emu->binds[r].y);
			out.WriteFloat(emu->binds[r].z);
			out.WriteFloat(emu->binds[r].heading);
		}

		/*
		u32 deity;
		u32 intoxication;
		*/
		out.WriteUInt32(emu->deity);
		out.WriteUInt32(emu->intoxication);

		//u32 property_count;
		out.WriteUInt32(10); // properties count

		//u32 properties[property_count]; 
		for (int i = 0; i < 10; i++) {
			out.WriteUInt32(0);
		}

		//u32 armor_prop_count;
		out.WriteUInt32(22); //armor count
		for (int i = 0; i < 22; ++i) {
			/*
			s32 type;
			s32 variation;
			s32 material;
			s32 newArmorId;
			s32 newArmorType;
			*/
			out.WriteUInt32(0);
			out.WriteUInt32(0);
			out.WriteUInt32(0);
			out.WriteUInt32(0);
			out.WriteUInt32(0);
		}

		//u32 base_armor_prop_count;
		out.WriteUInt32(9); //base armor count
		for (int i = 0; i < 9; ++i) {
			/*
			s32 type;
			s32 variation;
			s32 material;
			s32 newArmorId;
			s32 newArmorType;
			*/
			out.WriteUInt32(0);
			out.WriteUInt32(0);
			out.WriteUInt32(0);
			out.WriteUInt32(0);
			out.WriteUInt32(0);
		}

		//u32 body_tint_count;
		out.WriteUInt32(9); //body_tint_count
		//u32 body_tints[body_tint_count];
		for (int i = 0; i < 9; ++i) {
			out.WriteUInt32(0);
		}

		//u32 equip_tint_count;
		out.WriteUInt32(9); //equip_tint_count
		//u32 equip_tints[equip_tint_count];
		for (int i = 0; i < 9; ++i) {
			out.WriteUInt32(0);
		}

		/*
		u8 hair_color;
		u8 facial_hair_color;
		u32 npc_tint_index;
		u8 eye_color1;
		u8 eye_color2;
		u8 hair_style;
		u8 facial_hair;
		u8 face;
		u8 old_face;
		u32 heritage;
		u32 tattoo;
		u32 details;
		*/
		out.WriteUInt8(emu->haircolor);
		out.WriteUInt8(emu->beardcolor);
		out.WriteUInt32(0); //npc tint index
		out.WriteUInt8(emu->eyecolor1);
		out.WriteUInt8(emu->eyecolor2);
		out.WriteUInt8(emu->hairstyle);
		out.WriteUInt8(emu->beard);
		out.WriteUInt8(emu->face);
		out.WriteUInt8(0); //old face
		out.WriteUInt32(emu->drakkin_heritage);
		out.WriteUInt32(emu->drakkin_tattoo);
		out.WriteUInt32(emu->drakkin_details);

		/*
		u8 texture_type;
		u8 material;
		u8 variation;
		*/
		out.WriteUInt8(0);
		out.WriteUInt8(0);
		out.WriteUInt8(0);

		/*
		float height;
		float width;
		float length;
		float view_height;
		*/
		out.WriteFloat(5.0f);
		out.WriteFloat(3.0f);
		out.WriteFloat(2.5f);
		out.WriteFloat(5.5f);

		/*
		u32 primary;
		u32 secondary;
		*/
		out.WriteUInt32(0);
		out.WriteUInt32(0);

		/*
		u32 practices;
		u32 base_mana;
		u32 base_hp;
		u32 base_str;
		u32 base_sta;
		u32 base_cha;
		u32 base_dex;
		u32 base_int;
		u32 base_agi;
		u32 base_wis;
		u32 base_heroic_str;
		u32 base_heroic_sta;
		u32 base_heroic_cha;
		u32 base_heroic_dex;
		u32 base_heroic_int;
		u32 base_heroic_agi;
		u32 base_heroic_wis;
		*/
		out.WriteUInt32(emu->points);
		out.WriteUInt32(emu->mana);
		out.WriteUInt32(emu->cur_hp);
		out.WriteUInt32(emu->STR);
		out.WriteUInt32(emu->STA);
		out.WriteUInt32(emu->CHA);
		out.WriteUInt32(emu->DEX);
		out.WriteUInt32(emu->INT);
		out.WriteUInt32(emu->AGI);
		out.WriteUInt32(emu->WIS);
		out.WriteUInt32(0);
		out.WriteUInt32(0);
		out.WriteUInt32(0);
		out.WriteUInt32(0);
		out.WriteUInt32(0);
		out.WriteUInt32(0);
		out.WriteUInt32(0);

		//u32 aa_count;
		out.WriteUInt32(300);
		for (int i = 0; i < 240; ++i) {
			/*
			s32 index;
			s32 points_spent;
			s32 charges_spent;
			*/
			out.WriteUInt32(emu->aa_array[i].AA);
			out.WriteUInt32(emu->aa_array[i].value);
			out.WriteUInt32(emu->aa_array[i].charges);
		}

		for (int i = 0; i < 60; ++i) {
			/*
			s32 index;
			s32 points_spent;
			s32 charges_spent;
			*/
			out.WriteUInt32(0);
			out.WriteUInt32(0);
			out.WriteUInt32(0);
		}

		/*u32 skill_count;*/
		out.WriteUInt32(100);
		//s32 skills[skill_count];
		for (int i = 0; i < 100; ++i) {
			out.WriteUInt32(emu->skills[i]);
		}

		//u32 innate_skill_count;
		out.WriteUInt32(25);
		//s32 innate_skills[innate_skill_count];
		for (int i = 0; i < 25; ++i) {
			out.WriteUInt32(emu->InnateSkills[i]);
		}

		/*
		u32 combat_ability_count;
		*/
		out.WriteUInt32(300);
		//s32 combat_abilities[combat_ability_count];
		for (int i = 0; i < 100; ++i) {
			out.WriteUInt32(emu->disciplines.values[i]);
		}

		for (int i = 0; i < 200; ++i) {
			out.WriteUInt32(0);
		}

		//u32 combat_ability_timer_count;
		out.WriteUInt32(25);
		//s32 combat_ability_timers[combat_ability_timer_count];
		for (int i = 0; i < 20; ++i) {
			out.WriteUInt32(emu->disciplines.values[i]);
		}

		for (int i = 0; i < 5; ++i) {
			out.WriteUInt32(0);
		}

		//u32 unk_ability_count;
		out.WriteUInt32(0);

		//u32 linked_spell_timer_count;
		out.WriteUInt32(25);
		//s32 linked_spell_timers[linked_spell_timer_count];
		for (int i = 0; i < 25; ++i) {
			out.WriteUInt32(0);
		}

		//u32 item_recast_timer_count;
		out.WriteUInt32(100);
		//s32 item_recast_timers[item_recast_timer_count];
		for (int i = 0; i < 100; ++i) {
			out.WriteUInt32(0);
		}

		//u32 spell_book_slot_count;
		out.WriteUInt32(1120);

		//s32 spell_book_slots[spell_book_slot_count];
		for (int i = 0; i < 720; ++i) {
			out.WriteUInt32(emu->spell_book[i]);
		}

		for (int i = 0; i < 400; ++i) {
			out.WriteUInt32(0xFFFFFFFF);
		}

		//u32 spell_gem_count;
		out.WriteUInt32(18);
		//s32 spell_gems[spell_gem_count];
		for (int i = 0; i < 12; ++i) {
			out.WriteUInt32(emu->mem_spells[i]);
		}

		for (int i = 0; i < 6; ++i) {
			out.WriteUInt32(0xFFFFFFFF);
		}

		/*
		u32 spell_recast_timer_count;
		*/
		out.WriteUInt32(15);

		//s32 spell_recast_timers[spell_recast_timer_count];
		for (int i = 0; i < 12; ++i) {
			out.WriteUInt32(emu->spellSlotRefresh[i]);
		}

		for (int i = 0; i < 3; ++i) {
			out.WriteUInt32(0);
		}

		//u8 max_allowed_spell_slots;
		out.WriteUInt8(0);

		//u32 buff_count;
		out.WriteUInt32(42);

		//PackedEQAffect buffs[buff_count];
		//todo: fix
		for (int i = 0; i < 42; ++i) {
			/*
			struct EQAffect
			{
				float modifier;
				EqGuid caster;
				u32 duration;
				u32 max_duration;
				u8 level;
				s32 spell_id;
				s32 hitcount;
				u32 flags;
				u32 viral_timer;
				u8 type;
				SlotData slots[6];
			};

			*/
			out.WriteFloat(1.0f);
			out.WriteUInt64(0);
			out.WriteUInt32(0);
			out.WriteUInt32(0);
			out.WriteUInt8(0);
			out.WriteUInt32(0xFFFFFFFF);
			out.WriteUInt32(0);
			out.WriteUInt32(0);
			out.WriteUInt32(0);
			out.WriteUInt8(0);
			//SlotData slots[6];
			for (int j = 0; j < 6; ++j) {
				/*
				s32 slot_id;
				s64 value;
				*/
				out.WriteInt32(-1);
				out.WriteUInt64(0);
			}
		}

		//Coin coin;
		/*
		u32 platinum;
		u32 gold;
		u32 silver;
		u32 copper;
		*/
		out.WriteUInt32(emu->platinum);
		out.WriteUInt32(emu->gold);
		out.WriteUInt32(emu->silver);
		out.WriteUInt32(emu->copper);

		//Coin cursor_coin;
		/*
		u32 platinum;
		u32 gold;
		u32 silver;
		u32 copper;
		*/
		out.WriteUInt32(emu->platinum_cursor);
		out.WriteUInt32(emu->gold_cursor);
		out.WriteUInt32(emu->silver_cursor);
		out.WriteUInt32(emu->copper_cursor);

		/*
		u32 disc_timer;
		u32 mend_timer;
		u32 forage_timer;
		u32 thirst;
		u32 hunger;
		*/

		out.WriteUInt32(0);
		out.WriteUInt32(0);
		out.WriteUInt32(0);
		out.WriteUInt32(emu->thirst_level);
		out.WriteUInt32(emu->hunger_level);

		//u32 aa_spent;
		out.WriteUInt32(emu->aapoints_spent);

		//u32 aa_window_count;
		out.WriteUInt32(6);
		//u32 aa_window_stats[aa_window_count];
		out.WriteUInt32(0);
		out.WriteUInt32(0);
		out.WriteUInt32(0);
		out.WriteUInt32(0);
		out.WriteUInt32(0);
		out.WriteUInt32(0);

		//u32 aa_points_unspent;
		out.WriteUInt32(emu->aapoints);

		/*
		u8 sneak;
		u8 hide;
		*/
		out.WriteUInt8(0);
		out.WriteUInt8(0);

		//u32 bandolier_count;
		out.WriteUInt32(20);

		//BandolierSet bandolier_sets[bandolier_count];
		for (int i = 0; i < 20; ++i) {
			//char name[];
			out.WriteString(emu->bandoliers[i].Name);

			//BandolierItemInfo items[4];
			for (int j = 0; j < 4; ++j) {
				//char name[];
				out.WriteString(emu->bandoliers[i].Items[j].Name);
				//s32 item_id;
				out.WriteUInt32(emu->bandoliers[i].Items[j].ID);
				//s32 icon;
				out.WriteUInt32(emu->bandoliers[i].Items[j].Icon);
			}
		}

		//u32 invslot_bitmask;
		out.WriteUInt32(0xFFFFFFFF);

		/*
		u32 basedata_hp;
		u32 basedata_mana;
		u32 basedata_endur;
		u32 basedata_mr;
		u32 basedata_fr;
		u32 basedata_cr;
		u32 basedata_pr;
		u32 basedata_dr;
		u32 basedata_corrupt;
		u32 basedata_phr;
		*/

		out.WriteUInt32(5);
		out.WriteUInt32(5);
		out.WriteUInt32(5);
		out.WriteUInt32(25);
		out.WriteUInt32(25);
		out.WriteUInt32(25);
		out.WriteUInt32(15);
		out.WriteUInt32(15);
		out.WriteUInt32(15);
		out.WriteUInt32(15);

		/*
		float basedata_walkspeed;
		float basedata_runspeed;
		*/

		out.WriteFloat(0.46f);
		out.WriteFloat(0.7f);

		/*
		u32 basedata_hpregen;
		u32 basedata_manaregen;
		u32 basedata_mountmanaregen;
		u32 basedata_endurregen;
		u32 basedata_ac;
		u32 basedata_atk;
		u32 basedata_dmg;
		u32 basedata_delay;
		u32 endurance;
		u32 heroic_type;
		*/

		out.WriteUInt32(0);
		out.WriteUInt32(0);
		out.WriteUInt32(0);
		out.WriteUInt32(0);
		out.WriteUInt32(0);
		out.WriteUInt32(0);
		out.WriteUInt32(0);
		out.WriteUInt32(0);
		out.WriteUInt32(emu->endurance);
		out.WriteUInt32(0);

		//ItemIndex keyring_item_index[6];
		for (int i = 0; i < 6; ++i) {
			/*
			s16 slot1;
			s16 slot2;
			s16 slot3;
			*/

			out.WriteInt16(-1);
			out.WriteInt16(-1);
			out.WriteInt16(-1);
		}

		/*
		u64 exp;
		u32 aa_exp;
		*/
		out.WriteUInt64(emu->exp);
		out.WriteUInt32(emu->expAA);

		//PcClient begin
		/*
		u32 character_id;
		u16 world_id;
		u16 reserved;
		*/
		out.WriteUInt32(emu->char_id);
		out.WriteUInt16(RuleI(World, Id));
		out.WriteUInt16(0);

		/*u32 name_length;*/
		/*char name[name_length];*/
		out.WriteLengthString(64, emu->name);

		//u32 last_name_length;
		//char last_name[last_name_length];
		out.WriteLengthString(32, emu->last_name);

		/*
		u32 creation_time;
		u32 account_creation_time;
		u32 last_played_time;
		u32 played_minutes;
		u32 entitled_days;
		u64 expansion_flags;
		*/

		out.WriteUInt32(emu->birthday);
		out.WriteUInt32(emu->birthday);
		out.WriteUInt32(emu->lastlogin);
		out.WriteUInt32(5000);
		out.WriteUInt32(6000);
		out.WriteUInt64(0x3FFFFFFF);

		//u32 language_count;
		out.WriteUInt32(32);
		for (int i = 0; i < 28; i++)
		{
			//u8 languages[language_count];
			out.WriteUInt8(emu->languages[i]);
		}

		for (int i = 0; i < 4; i++)
		{
			out.WriteUInt8(0);
		}

		/*
		u32 current_zone; // combine id + instance
		float current_x;
		float current_y;
		float current_z;
		float current_heading;
		*/
		out.WriteUInt16(emu->zone_id);
		out.WriteUInt16(emu->zoneInstance);
		out.WriteFloat(emu->x);
		out.WriteFloat(emu->y);
		out.WriteFloat(emu->z);
		out.WriteFloat(emu->heading);

		/*
		u8 animation;
		u8 pvp;
		u8 anon;
		u8 gm;
		*/
		out.WriteUInt8(100);
		out.WriteUInt8(emu->pvp);
		out.WriteUInt8(emu->anon);
		out.WriteUInt8(emu->gm);

		/*
		u64 guild_id;
		u8 guild_show_sprite;
		u8 status;
		*/

		out.WriteUInt64(emu->guild_id);
		out.WriteUInt8(0);
		out.WriteUInt8(5);

		//Coin coin;
		out.WriteUInt32(emu->platinum);
		out.WriteUInt32(emu->gold);
		out.WriteUInt32(emu->silver);
		out.WriteUInt32(emu->copper);

		//Coin bank;
		out.WriteUInt32(emu->platinum_bank);
		out.WriteUInt32(emu->gold_bank);
		out.WriteUInt32(emu->silver_bank);
		out.WriteUInt32(emu->copper_bank);

		//u32 bank_shared_plat;
		out.WriteUInt32(emu->platinum_shared);

		//u32 claim_count;
		out.WriteUInt32(0);
		//Claim claims[claim_count];

		//Tribute tribute;
		/*
		u32 BenefitTimer;
		s32 unknown1;
		s64 current_favor;
		s64 all_time_favor;
		u8 unknown;
		u8 unknown;
		*/
		out.WriteUInt32(600000);
		out.WriteInt32(-1);
		out.WriteUInt64(emu->tribute_points);
		out.WriteUInt64(emu->career_tribute_points);
		out.WriteUInt8(0);
		out.WriteUInt8(0);

		//u32 tribute_benefit_count
		out.WriteUInt32(5);

		//TributeBenefit tribute_benefits[tribute_benefit_count];
		for (int i = 0; i < 5; ++i) {
			/*
			s32 benefit_id;
			s32 benefit_tier;
			*/

			out.WriteUInt32(emu->tributes[i].tribute);
			out.WriteUInt32(emu->tributes[i].tier);
		}

		//u32 trophy_tribute_benefit_count;
		out.WriteUInt32(10);

		//TributeBenefit trophy_tribute_benefit[trophy_tribute_benefit_count];
		for (int i = 0; i < 10; ++i) {
			/*
			s32 benefit_id;
			s32 benefit_tier;
			*/

			out.WriteUInt32(0xFFFFFFFF);
			out.WriteUInt32(0);
		}

		// u32 BankItems type
		// u32 BankItems data1
		// u32 BankItems data2
		out.WriteUInt32(0);
		out.WriteUInt32(0);
		out.WriteUInt32(0);

		// s32 ActiveSharedTaskID
		// bool IsMonsterMission
		// s32 TaskID
		// s32 MovingStartTime
		// s32 InitialStartTime
		out.WriteInt32(0);
		out.WriteUInt8(0);
		out.WriteInt32(0);
		out.WriteInt32(0);
		out.WriteInt32(0);

		for (int i = 0; i < 2; ++i) {
			// these are the element active booleans, will want to set them all individually later
			for (int j = 0; j < 10; ++j)
				out.WriteUInt8(0);
		}

		for (int i = 0; i < 2; ++i) {
			// these are the current count integers, will want to set them all individually later
			for (int j = 0; j < 10; ++j)
				out.WriteUInt32(0);
		}

		// float RewardAdjustment
		out.WriteFloat(0.f);

		// monster mission count (set to 0 for now)
		// monster mission packets are as follows:
		//   s32 template id
		//   s32 min
		//   s32 max
		//   s32 num selected
		//   bool can select
		//   string template name
		out.WriteUInt32(0);


		/*
		u32 good_points_available;
		u32 good_points_earned;
		u32 bad_points_available;
		u32 bad_points_earned;
		*/

		out.WriteUInt32(emu->currentRadCrystals);
		out.WriteUInt32(emu->careerRadCrystals);
		out.WriteUInt32(emu->currentEbonCrystals);
		out.WriteUInt32(emu->careerEbonCrystals);

		/*
		u32 momentum_balance;
		u32 loyalty_reward_balance;
		u32 parcel_status;
		*/

		out.WriteUInt32(0);
		out.WriteUInt32(0);
		out.WriteUInt32(0);

		/*
		u32 vehicle_name_length;
		char vehicle_name[vehicle_name_length];
		*/

		out.WriteUInt32(64);
		for (int i = 0; i < 64; ++i) {
			out.WriteUInt8(0);
		}

		/*
		u8 super_pkill;
		u8 unclone;
		u8 dead;
		*/

		out.WriteUInt8(0);
		out.WriteUInt8(0);
		out.WriteUInt8(0);

		/*
		u32 ld_timer;
		u32 spell_interrupt_count;
		u8 autosplit;
		u8 tells_off;
		*/

		out.WriteUInt32(0);
		out.WriteUInt32(0);
		out.WriteUInt8(emu->autosplit);
		out.WriteUInt8(0);

		/*
		u8 gm_invis;
		u32 kill_me;
		u8 cheater_ld_flag;
		u8 norent;
		u8 corpse;
		u8 client_gm_flag_set;
		u32 mentor_pct;
		*/

		out.WriteUInt8(0);
		out.WriteUInt32(0);
		out.WriteUInt8(0);
		out.WriteUInt8(0);
		out.WriteUInt8(0);
		out.WriteUInt8(0);
		out.WriteUInt32(0);

		//RaidData raid;
		/*
		u32 main_assist1;
		u32 main_assist2;
		u32 main_assist3;
		char main_assist_name1[];
		char main_assist_name2[];
		char main_assist_name3[];
		u32 main_marker1;
		u32 main_marker2;
		u32 main_marker3;
		u32 master_looter;
		*/

		out.WriteUInt32(0);
		out.WriteUInt32(0);
		out.WriteUInt32(0);
		out.WriteUInt8(0);
		out.WriteUInt8(0);
		out.WriteUInt8(0);
		out.WriteUInt32(0);
		out.WriteUInt32(0);
		out.WriteUInt32(0);
		out.WriteUInt32(0);

		//u32 unique_player_id;
		out.WriteUInt32(emu->char_id);

		//LdonData ldon_data;
		/*
		u32 count;
		u32 ldon_categories[count];
		u32 ldon_points_available;
		*/

		out.WriteUInt32(6);
		out.WriteUInt32(0);
		out.WriteUInt32(emu->ldon_points_guk);
		out.WriteUInt32(emu->ldon_points_mir);
		out.WriteUInt32(emu->ldon_points_mmc);
		out.WriteUInt32(emu->ldon_points_ruj);
		out.WriteUInt32(emu->ldon_points_tak);
		out.WriteUInt32(emu->ldon_points_available);

		//u32 air_supply;
		out.WriteUInt32(emu->air_remaining);

		//PvPData pvp_data;
		/*
		u32 kills;
		u32 deaths;
		u32 current_points;
		u32 career_points;
		u32 best_kill_streak;
		u32 worst_death_streak;
		u32 current_kill_streak;
		*/

		out.WriteUInt32(emu->PVPKills);
		out.WriteUInt32(emu->PVPDeaths);
		out.WriteUInt32(emu->PVPCurrentPoints);
		out.WriteUInt32(emu->PVPCareerPoints);
		out.WriteUInt32(emu->PVPBestKillStreak);
		out.WriteUInt32(emu->PVPWorstDeathStreak);
		out.WriteUInt32(emu->PVPCurrentKillStreak);

		//PvPKill last_kill;
		/*
		char name[];
		u32 unk;
		u32 race;
		u32 class;
		u32 zone;
		u32 time;
		u32 points;
		u32 level;
		*/
		out.WriteString(emu->PVPLastKill.Name);
		out.WriteUInt32(0);
		out.WriteUInt32(emu->PVPLastKill.Race);
		out.WriteUInt32(emu->PVPLastKill.Class);
		out.WriteUInt32(emu->PVPLastKill.Zone);
		out.WriteUInt32(emu->PVPLastKill.Time);
		out.WriteUInt32(emu->PVPLastKill.Points);
		out.WriteUInt32(emu->PVPLastKill.Level);

		//PvPDeath last_death;
		/*
		char name[];
		u32 level;
		u32 race;
		u32 class;
		u32 zone;
		u32 time;
		u32 points;
		*/

		out.WriteString(emu->PVPLastDeath.Name);
		out.WriteUInt32(emu->PVPLastDeath.Level);
		out.WriteUInt32(emu->PVPLastDeath.Race);
		out.WriteUInt32(emu->PVPLastDeath.Class);
		out.WriteUInt32(emu->PVPLastDeath.Zone);
		out.WriteUInt32(emu->PVPLastDeath.Time);
		out.WriteUInt32(emu->PVPLastDeath.Points);

		/*
		u32 kills_in_past_24_hours;
		*/

		out.WriteUInt32(emu->PVPNumberOfKillsInLast24Hours);

		//u32 kill_list_count;
		out.WriteUInt32(50);

		//PvPKill kill_list[kill_list_count];
		for (int i = 0; i < 50; ++i) {
			/*
			char name[];
			u32 unknown1;
			u32 race;
			u32 class;
			u32 zone;
			u32 time;
			u32 points;
			u32 level;
			*/
			out.WriteString(emu->PVPRecentKills[i].Name);
			out.WriteUInt32(0);
			out.WriteUInt32(emu->PVPRecentKills[i].Race);
			out.WriteUInt32(emu->PVPRecentKills[i].Class);
			out.WriteUInt32(emu->PVPRecentKills[i].Zone);
			out.WriteUInt32(emu->PVPRecentKills[i].Time);
			out.WriteUInt32(emu->PVPRecentKills[i].Points);
			out.WriteUInt32(emu->PVPRecentKills[i].Level);
		}

		/*
		u32 pvp_infamy_level;
		u32 pvp_vitality;
		*/
		out.WriteUInt32(0);
		out.WriteUInt32(0);

		/*
		u32 cursor_krono;
		u32 krono;
		*/
		out.WriteUInt32(0);
		out.WriteUInt32(0);

		/*
		u8 autoconsent_group;
		u8 autoconsent_raid;
		u8 autoconsent_guild;
		u8 autoconsent_fellowship;
		*/

		out.WriteUInt8(emu->groupAutoconsent);
		out.WriteUInt8(emu->raidAutoconsent);
		out.WriteUInt8(emu->guildAutoconsent);
		out.WriteUInt8(1);

		/*
		u8 private_for_eq_players;
		u32 main_level;
		u8 show_helm;
		u32 downtime;
		*/
		out.WriteUInt8(1);
		out.WriteUInt32(emu->level);
		out.WriteUInt8(emu->showhelm);
		out.WriteUInt32(emu->RestTimer);

		//AltCurrency alt_currency;
		/*
		u64 alt_currency_str_length;
		char alt_currency_string[alt_currency_str_length];
		*/
		out.WriteUInt64(1);
		out.WriteUInt8(0x31);

		//u32 completed_event_subcomponent_count;
		out.WriteUInt32(0);
		//AchivementSubComponentData completed_event_subcomponents[completed_event_subcomponent_count];
		// s32 component id
		// s32 requirement id
		// u32 requirement type
		// s32 count
		// u32 unk

		//u32 inprogress_event_subcomponent_count;
		out.WriteUInt32(0);
		//AchivementSubComponentData inprogress_event_subcomponents[inprogress_event_subcomponent_count];
		// s32 component id
		// s32 requirement id
		// u32 requirement type
		// s32 count
		// u32 unk

		/*
		u64 merc_aa_exp;
		u32 merc_aa_points;
		u32 merc_aa_spent;
		*/
		out.WriteUInt64(0);
		out.WriteUInt32(0);
		out.WriteUInt32(0);

		//u32 starting_city_zone_id;
		//we don't actually support this yet
		out.WriteUInt32(394);

		/*
		u8 use_advanced_looting;
		u8 is_master_loot_candidate;
		*/

		out.WriteUInt8(1);
		out.WriteUInt8(1);

		//alchemy_bonus_list_count
		out.WriteUInt32(0);
		//AlchemyBonusSkillData alchemy_bonus_list[alchemy_bonus_list_count];
		// s32 skill id
		// s32 bonus points

		//u32 persona_count;
		out.WriteUInt32(0);
		//PersonaEquipmentSet persona_equipment_set[persona_count];
		// u32
		// u32 (length)
		//    u32
		//    u32
		//    u32
		// (there is a way to set two more u32's here, but the client seems to not ever reach that code)

		//u8 term;
		out.WriteUInt8(0);

		auto outapp = new EQApplicationPacket(OP_PlayerProfile, out.length());
		outapp->WriteData(out.buffer(), out.length());
		outapp->SetWritePosition(4);
		outapp->WriteUInt32(outapp->size - 9);
		CRC32::SetEQChecksum(outapp->pBuffer, outapp->size - 1, 8);
		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	ENCODE(OP_RecipeAutoCombine)
	{
		ENCODE_LENGTH_EXACT(RecipeAutoCombine_Struct);
		SETUP_DIRECT_ENCODE(RecipeAutoCombine_Struct, structs::RecipeAutoCombine_Struct);

		OUT(object_type);
		OUT(some_id);
		eq->container_slot = ServerToTOBSlot(emu->unknown1);
		structs::InventorySlot_Struct TOBSlot;
		TOBSlot.Type = 8;	// Observed
		TOBSlot.Slot = 0xffff;
		TOBSlot.SubIndex = 0xffff;
		TOBSlot.AugIndex = 0xffff;
		TOBSlot.Padding2 = 0;
		eq->unknown_slot = TOBSlot;
		OUT(recipe_id);
		OUT(reply_code);

		FINISH_ENCODE();
	}

	ENCODE(OP_RemoveBlockedBuffs) { ENCODE_FORWARD(OP_BlockedBuffs); }

	ENCODE(OP_RespondAA)
	{
		SETUP_DIRECT_ENCODE(AATable_Struct, structs::AATable_Struct);

		eq->aa_spent = emu->aa_spent;
		// These fields may need to be correctly populated at some point
		eq->aapoints_assigned[0] = emu->aa_spent;
		eq->aapoints_assigned[1] = 0;
		eq->aapoints_assigned[2] = 0;
		eq->aapoints_assigned[3] = 0;
		eq->aapoints_assigned[4] = 0;
		eq->aapoints_assigned[5] = 0;

		for (uint32 i = 0; i < structs::MAX_PP_AA_ARRAY; ++i)
		{
			eq->aa_list[i].AA = emu->aa_list[i].AA;
			eq->aa_list[i].value = emu->aa_list[i].value;
			eq->aa_list[i].charges = emu->aa_list[i].charges;
		}

		FINISH_ENCODE();
	}

	ENCODE(OP_RequestClientZoneChange)
	{
		ENCODE_LENGTH_EXACT(RequestClientZoneChange_Struct);
		SETUP_DIRECT_ENCODE(RequestClientZoneChange_Struct, structs::RequestClientZoneChange_Struct);

		OUT(zone_id);
		OUT(instance_id);
		OUT(y);
		OUT(x);
		OUT(z);
		OUT(heading);
		eq->type = 0x0b;
		eq->unknown004 = 0xffffffff;
		eq->unknown172 = 0x0168b500;

		FINISH_ENCODE();
	}

	ENCODE(OP_SendAATable)
	{
		EQApplicationPacket* in = *p;
		*p = nullptr;
		AARankInfo_Struct* emu = (AARankInfo_Struct*)in->pBuffer;

		std::vector<int32> skill;
		std::vector<int32> points;
		in->SetReadPosition(sizeof(AARankInfo_Struct) + emu->total_effects * sizeof(AARankEffect_Struct));
		for (auto i = 0; i < emu->total_prereqs; ++i) {
			skill.push_back(in->ReadUInt32());
			points.push_back(in->ReadUInt32());
		}

		SerializeBuffer buffer;

		/*
		s32 AbilityId;
		u8 ShowInAbilityWindow;
		s32 ShortName;
		s32 ShortName2;
		s32 Name;
		s32 Desc;
		*/

		buffer.WriteUInt32(emu->id); // Index
		buffer.WriteUInt8(1);
		buffer.WriteInt32(emu->upper_hotkey_sid);
		buffer.WriteInt32(emu->lower_hotkey_sid);
		buffer.WriteInt32(emu->title_sid);
		buffer.WriteInt32(emu->desc_sid);

		/*
		s32 MinLevel;
		s32 Cost;
		s32 GroupID;
		s32 CurrentRank;
		*/
		buffer.WriteInt32(emu->level_req);
		buffer.WriteInt32(emu->cost);
		buffer.WriteUInt32(emu->seq);
		buffer.WriteUInt32(emu->current_level);

		/*
		u32 PrereqSkillCount;
		s32 PrereqSkills[PrereqSkillCount];
		u32 PrereqLevelCount;
		s32 PrereqLevels[PrereqLevelCount];
		*/

		if (emu->total_prereqs) {
			buffer.WriteUInt32(emu->total_prereqs);
			for (auto& e : skill)
				buffer.WriteInt32(e);
			buffer.WriteUInt32(emu->total_prereqs);
			for (auto& e : points)
				buffer.WriteInt32(e);
		}
		else {
			buffer.WriteUInt32(1);
			buffer.WriteUInt32(0);
			buffer.WriteUInt32(1);
			buffer.WriteUInt32(0);
		}

		/*
		u32 Type;
		s32 SpellId;
		*/
		buffer.WriteInt32(emu->type);
		buffer.WriteInt32(emu->spell);

		/*
		u32 TimerIdCount;
		s32 TimerIds[TimerIdCount];
		s32 ReuseTimer;
		u32 Classes;
		*/
		buffer.WriteInt32(1);
		buffer.WriteInt32(emu->spell_type);
		buffer.WriteInt32(emu->spell_refresh);
		buffer.WriteInt32(emu->classes);

		/*
		s32 MaxRank;
		s32 PrevAbilityId;
		s32 NextAbilityId;
		s32 TotalPoints;
		*/

		buffer.WriteInt32(emu->max_level);
		buffer.WriteInt32(emu->prev_id);
		buffer.WriteInt32(emu->next_id);
		buffer.WriteInt32(emu->total_cost);

		/*
		u8 bRefund;
		s32 QuestOnly;
		u8 bIgnoreDeLevel;
		*/
		buffer.WriteUInt8(0);
		buffer.WriteUInt32(emu->grant_only);
		buffer.WriteUInt8(0);

		/*
		s32 Charges;
		s32 Expansion;
		s32 Category;
		*/
		buffer.WriteUInt32(emu->charges);
		buffer.WriteInt32(emu->expansion);
		buffer.WriteInt32(emu->category);

		/*
		u8 bShroud;
		u8 bBetaOnly;
		u8 bResetOnDeath;
		u8 AutoGrant;
		*/
		buffer.WriteUInt8(0);
		buffer.WriteUInt8(0);
		buffer.WriteUInt8(0);
		buffer.WriteUInt8(0);

		/*
		s32 AutoGrantExpansion;
		s32 Unknown098;
		u8 Unknown09C;
		u8 unk TOB added
		*/

		buffer.WriteInt32(emu->expansion);
		buffer.WriteInt32(0);
		buffer.WriteUInt8(0);
		buffer.WriteUInt8(0);

		//u32 TotalEffects;
		buffer.WriteUInt32(emu->total_effects);
		in->SetReadPosition(sizeof(AARankInfo_Struct));
		for (auto i = 0; i < emu->total_effects; ++i) {
			auto skill_id = in->ReadUInt32();
			auto base1 = in->ReadUInt32();
			auto base2 = in->ReadUInt32();
			auto slot = in->ReadUInt32();

			/*
			u32 effect_id;
			s64 base1;
			s64 base2;
			u32 slot;
			*/

			buffer.WriteUInt32(skill_id);
			buffer.WriteInt64(base1);
			buffer.WriteInt64(base2);
			buffer.WriteUInt32(slot);
		}

		auto outapp = new EQApplicationPacket(OP_SendAATable, buffer.size());
		outapp->WriteData(buffer.buffer(), buffer.size());
		dest->FastQueuePacket(&outapp, ack_req);

		delete in;
	}

	ENCODE(OP_SendCharInfo) {
		ENCODE_LENGTH_ATLEAST(CharacterSelect_Struct);
		SETUP_VAR_ENCODE(CharacterSelect_Struct);

		// Zero-character count shunt
		if (emu->CharCount == 0) {
			ALLOC_VAR_ENCODE(structs::CharacterSelect_Struct, sizeof(structs::CharacterSelect_Struct));
			eq->CharCount = emu->CharCount;

			FINISH_ENCODE();
			return;
		}

		unsigned char* emu_ptr = __emu_buffer;
		emu_ptr += sizeof(CharacterSelect_Struct);
		CharacterSelectEntry_Struct* emu_cse = (CharacterSelectEntry_Struct*)nullptr;

		size_t names_length = 0;
		size_t character_count = 0;
		for (; character_count < emu->CharCount; ++character_count) {
			emu_cse = (CharacterSelectEntry_Struct*)emu_ptr;
			names_length += strlen(emu_cse->Name);
			emu_ptr += sizeof(CharacterSelectEntry_Struct);
		}

		size_t total_length = sizeof(structs::CharacterSelect_Struct)
			+ character_count * sizeof(structs::CharacterSelectEntry_Struct)
			+ names_length;

		ALLOC_VAR_ENCODE(structs::CharacterSelect_Struct, total_length);
		structs::CharacterSelectEntry_Struct* eq_cse = (structs::CharacterSelectEntry_Struct*)nullptr;

		eq->CharCount = character_count;

		emu_ptr = __emu_buffer;
		emu_ptr += sizeof(CharacterSelect_Struct);

		unsigned char* eq_ptr = __packet->pBuffer;
		eq_ptr += sizeof(structs::CharacterSelect_Struct);

		for (int counter = 0; counter < character_count; ++counter) {
			emu_cse = (CharacterSelectEntry_Struct*)emu_ptr;
			eq_cse = (structs::CharacterSelectEntry_Struct*)eq_ptr; // base address

			strcpy(eq_cse->Name, emu_cse->Name);
			eq_ptr += strlen(emu_cse->Name);

			eq_cse = (structs::CharacterSelectEntry_Struct*)eq_ptr;
			eq_cse->Name[0] = '\0';

			eq_cse->Class = emu_cse->Class;
			eq_cse->Race = emu_cse->Race;
			eq_cse->Level = emu_cse->Level;
			eq_cse->ShroudClass = emu_cse->ShroudClass;
			eq_cse->ShroudRace = emu_cse->ShroudRace;
			eq_cse->Zone = emu_cse->Zone;
			eq_cse->Instance = emu_cse->Instance;
			eq_cse->Gender = emu_cse->Gender;
			eq_cse->Face = emu_cse->Face;

			for (int equip_index = 0; equip_index < EQ::textures::materialCount; equip_index++) {
				eq_cse->Equip[equip_index].Material = emu_cse->Equip[equip_index].Material; // type
				eq_cse->Equip[equip_index].Unknown1 = emu_cse->Equip[equip_index].Unknown1; // variation
				eq_cse->Equip[equip_index].EliteMaterial = emu_cse->Equip[equip_index].EliteModel; // material
				eq_cse->Equip[equip_index].HeroForgeModel = emu_cse->Equip[equip_index].HerosForgeModel; // new armor id
				eq_cse->Equip[equip_index].Material2 = emu_cse->Equip[equip_index].Unknown2; // new armor type
				eq_cse->Equip[equip_index].Color = emu_cse->Equip[equip_index].Color; // tint
			}

			eq_cse->TextureType = 255;
			eq_cse->HeadType = 0;
			eq_cse->DrakkinTattoo = emu_cse->DrakkinTattoo;
			eq_cse->DrakkinDetails = emu_cse->DrakkinDetails;
			eq_cse->Deity = emu_cse->Deity;
			eq_cse->PrimaryIDFile = emu_cse->PrimaryIDFile;
			eq_cse->SecondaryIDFile = emu_cse->SecondaryIDFile;
			eq_cse->HairColor = emu_cse->HairColor;
			eq_cse->BeardColor = emu_cse->BeardColor;
			eq_cse->EyeColor1 = emu_cse->EyeColor1;
			eq_cse->EyeColor2 = emu_cse->EyeColor2;
			eq_cse->HairStyle = emu_cse->HairStyle;
			eq_cse->Beard = emu_cse->Beard;
			eq_cse->PreFTP = 1;
			eq_cse->Tutorial = emu_cse->Tutorial;
			eq_cse->DrakkinHeritage = emu_cse->DrakkinHeritage;
			eq_cse->GoHome = emu_cse->GoHome;
			eq_cse->LastLogin = emu_cse->LastLogin;
			eq_cse->TooHighLevel = 0;
			eq_cse->Usable = emu_cse->Enabled; // this doesn't seem to do anything
			eq_cse->Shrouded = 0;
			eq_cse->Unknown = 0;
			eq_cse->CharacterId = 0;

			emu_ptr += sizeof(CharacterSelectEntry_Struct);
			eq_ptr += sizeof(structs::CharacterSelectEntry_Struct);
		}

		DumpPacket(__packet);

		FINISH_ENCODE();
	}

	ENCODE(OP_SendMaxCharacters) {
		ENCODE_LENGTH_EXACT(MaxCharacters_Struct);
		SETUP_DIRECT_ENCODE(MaxCharacters_Struct, structs::MaxCharacters_Struct);

		*eq = {0};
		eq->total_character_slots = 8;
		eq->marketplace_character_slots = 0;
		eq->unknown008 = -1;
		eq->head_start_button = 0;
		eq->heroic_related = 0x0003;
		eq->heroic_50_count = 0;
		eq->heroic_100_count = 0;
		eq->disable_character_creation = 0; // this works, but it soft-locks the UI for some reason, needs to be fixed
		eq->monthly_claim = -1;
		eq->marketplace_related = 0;
		eq->add_marketplace_chars = 0;
		eq->add_unknown = 0;
		eq->legacy_characters_ruleset = 0;
		eq->num_max_characters = 0;
		eq->num_personas_available = 10;
		eq->has_de_ranger = 1; // this is probably an expansion flag only

		FINISH_ENCODE();
	}

	ENCODE(OP_SendMembership) {
		ENCODE_LENGTH_EXACT(Membership_Struct);
		SETUP_DIRECT_ENCODE(Membership_Struct, structs::Membership_Struct);

		eq->membership = emu->membership;
		eq->races = emu->races;
		eq->classes = emu->classes;
		eq->entrysize = 33;
		eq->entries[0] = -1;  // Max AA Restriction
		eq->entries[1] = -1;  // Max Level Restriction
		eq->entries[2] = -1;  // Max Char Slots per Account (not used by client?)
		eq->entries[3] = -1;  // SpellTier
		eq->entries[4] = -1;  // Main Inventory Size
		eq->entries[5] = -1;  // Max Platinum per level
		eq->entries[6] = 1;   // Send Mail
		eq->entries[7] = 1;   // Use Parcels
		eq->entries[8] = 1;   // Loyalty
		eq->entries[9] = -1;  // Merc Tiers
		eq->entries[10] = 1;  // Housing
		eq->entries[11] = -1; // Shared Bank Slots
		eq->entries[12] = -1; // Max Journal Quests
		eq->entries[13] = 1;  // CreateGuild
		eq->entries[14] = 1;  // Bazaar
		eq->entries[15] = 1;  // Barter
		eq->entries[16] = 1;  // Chat
		eq->entries[17] = 1;  // Petition
		eq->entries[18] = 1;  // Advertising
		eq->entries[19] = -1; // UseItem
		eq->entries[20] = -1; // StartingCity
		eq->entries[21] = 1;  // Ornament
		eq->entries[22] = 0;  // HeroicCharacter
		eq->entries[23] = 0;  // AutoGrantAA
		eq->entries[24] = 0;  // MountKeyRingSlots
		eq->entries[25] = 0;  // IllusionKeyRingSlots
		eq->entries[26] = 0;  // FamiliarKeyRingSlots
		eq->entries[27] = 0;  // FamiliarAutoLeave
		eq->entries[28] = 0;  // HeroForgeKeyRingSlots
		eq->entries[29] = 0;  // DragonHoardSlots
		eq->entries[30] = 0;  // TeleportKeyRingSlots
		eq->entries[31] = 0;  // PersonalDepotSlots
		eq->entries[32] = 0;

		FINISH_ENCODE();
	}

	ENCODE(OP_SendMembershipDetails) {
		ENCODE_LENGTH_EXACT(Membership_Details_Struct);
		SETUP_DIRECT_ENCODE(Membership_Details_Struct, structs::Membership_Details_Struct);

		int32 settings[96][3] = {
			{ 0, 0, 250 }, { 1, 0, 1000 }, { 0, 1, -1 }, { 1, 1, -1 }, 
			{ 0, 2, 2 }, { 2, 0, -1 }, { 3, 0, -1 }, { 1, 2, 4 }, 
			{ 0, 3, 1 }, { 2, 1, -1 }, { 3, 1, -1 }, { 1, 3, 1 }, 
			{ 0, 4, -1 }, { 2, 2, -1 }, { 3, 2, -1 }, { 1, 4, -1 }, 
			{ 0, 5, -1 }, { 2, 3, -1 }, { 3, 3, -1 }, { 1, 5, -1 }, 
			{ 0, 6, 0 }, { 2, 4, -1 }, { 3, 4, -1 }, { 1, 6, 0 }, 
			{ 0, 7, 1 }, { 2, 5, -1 }, { 3, 5, -1 }, { 1, 7, 1 }, 
			{ 0, 8, 1 }, { 2, 6, 1 }, { 3, 6, 1 }, { 1, 8, 1 }, 
			{ 0, 9, 5 }, { 2, 7, 1 }, { 3, 7, 1 }, { 1, 9, 5 }, 
			{ 0, 10, 0 }, { 2, 8, 1 }, { 3, 8, 1 }, { 0, 11, -1 }, 
			{ 1, 10, 1 }, { 2, 9, -1 }, { 3, 9, -1 }, { 0, 12, -1 }, 
			{ 1, 11, -1 }, { 2, 10, 1 }, { 3, 10, 1 }, { 0, 13, 0 }, 
			{ 1, 12, -1 }, { 2, 11, -1 }, { 3, 11, -1 }, { 0, 14, 0 }, 
			{ 1, 13, 1 }, { 2, 12, -1 }, { 3, 12, -1 }, { 0, 15, 0 }, 
			{ 1, 14, 0 }, { 2, 13, 1 }, { 3, 13, 1 }, { 0, 16, 0 }, 
			{ 1, 15, 0 }, { 2, 14, 1 }, { 3, 14, 1 }, { 0, 17, 0 }, 
			{ 1, 16, 1 }, { 2, 15, 1 }, { 3, 15, 1 }, { 0, 18, 0 }, 
			{ 1, 17, 0 }, { 2, 16, 1 }, { 3, 16, 1 }, { 0, 19, 0 }, 
			{ 1, 18, 0 }, { 2, 17, 1 }, { 3, 17, 1 }, { 0, 20, 0 }, 
			{ 1, 19, 0 }, { 2, 18, 1 }, { 3, 18, 1 }, { 0, 21, 0 }, 
			{ 1, 20, 0 }, { 2, 19, -1 }, { 3, 19, -1 }, { 0, 22, 0 }, 
			{ 1, 21, 0 }, { 2, 20, -1 }, { 3, 20, -1 }, { 2, 21, 0 }, 
			{ 0, 23, 0 }, { 1, 22, 0 }, { 3, 21, 0 }, { 2, 22, 0 }, 
			{ 1, 23, 0 }, { 3, 22, 0 }, { 2, 23, 0 }, { 3, 23, 0 }
		};

		uint32 races[17][2] = {
			{ 1, 131071 },
			{ 333, 131071 },
			{ 90287, 131071 },
			{ 90289, 16 },
			{ 90290, 32 },
			{ 90291, 64 },
			{ 90292, 128 },
			{ 90293, 256 },
			{ 90294, 512 },
			{ 90295, 1024 },
			{ 90296, 2048 },
			{ 90297, 8192 },
			{ 90298, 16384 },
			{ 90299, 32768 },
			{ 90300, 65536 },
			{ 2012271, 131071 },
			{ 2012277, 131071 }
		};
		
		uint32 classes[17][2] = {
			{ 1, 131071 },
			{ 333, 131071 },
			{ 90287, 131071 },
			{ 90301, 8 },
			{ 90302, 16 },
			{ 90303, 32 },
			{ 90304, 64 },
			{ 90305, 128 },
			{ 90306, 256 },
			{ 90307, 1024 },
			{ 90308, 2048 },
			{ 90309, 8192 },
			{ 90310, 16384 },
			{ 90311, 32768 },
			{ 90312, 65536 },
			{ 2012271, 131071 },
			{ 2012277, 131071 }
		};

		eq->membership_setting_count = 96;

		for (int i = 0; i < 96; ++i) {
			eq->settings[i].setting_index = (int8)settings[i][0];
			eq->settings[i].setting_id = settings[i][1];
			eq->settings[i].setting_value = settings[i][2];
		}

		eq->class_entry_count = 17;
		for (int i = 0; i < 17; ++i) {
			eq->membership_classes[i].purchase_id = classes[i][0];
			eq->membership_classes[i].bitwise_entry = classes[i][1];
		}

		eq->race_entry_count = 17;
		for (int i = 0; i < 17; ++i) {
			eq->membership_races[i].purchase_id = races[i][0];
			eq->membership_races[i].bitwise_entry = races[i][1];
		}

		eq->exit_url_length = 0;

		FINISH_ENCODE();
	}	

	ENCODE(OP_SendZonepoints)
	{
		SETUP_VAR_ENCODE(ZonePoints);
		ALLOC_VAR_ENCODE(structs::ZonePoints, sizeof(structs::ZonePoints) + sizeof(structs::ZonePoint_Entry) * (emu->count + 1));

		eq->count = emu->count;
		for (uint32 i = 0; i < emu->count; ++i)
		{
			eq->zpe[i].iterator = emu->zpe[i].iterator;
			eq->zpe[i].x = emu->zpe[i].x;
			eq->zpe[i].y = emu->zpe[i].y;
			eq->zpe[i].z = emu->zpe[i].z;
			eq->zpe[i].heading = emu->zpe[i].heading;
			eq->zpe[i].zoneid = emu->zpe[i].zoneid;
			eq->zpe[i].zoneinstance = emu->zpe[i].zoneinstance;
		}

		FINISH_ENCODE();
	}

	ENCODE(OP_ShopPlayerBuy)
	{
		ENCODE_LENGTH_EXACT(Merchant_Sell_Struct);
		SETUP_DIRECT_ENCODE(Merchant_Sell_Struct, structs::Merchant_Sell_Response_Struct);

		OUT(npcid);
		OUT(playerid);
		OUT(itemslot);
		OUT(quantity);
		OUT(price);

		FINISH_ENCODE();
	}

	ENCODE(OP_ShopPlayerSell)
	{
		ENCODE_LENGTH_EXACT(Merchant_Purchase_Struct);
		SETUP_DIRECT_ENCODE(Merchant_Purchase_Struct, structs::Merchant_Purchase_Response_Struct);

		OUT(npcid);
		eq->inventory_slot = ServerToTOBTypelessSlot(emu->itemslot, EQ::invtype::typePossessions);
		OUT(quantity);
		OUT(price);

		FINISH_ENCODE();
	}

	ENCODE(OP_ShopRequest)
	{
		ENCODE_LENGTH_EXACT(MerchantClick_Struct);
		SETUP_DIRECT_ENCODE(MerchantClick_Struct, structs::MerchantClickResponse_Struct);

		if (emu->command == 0) {
			OUT(player_id);
			eq->npc_id = 0;
		}
		else {
			OUT(npc_id);
			OUT(player_id);
			OUT(rate);
			OUT(tab_display);
			eq->unknown028 = 256;
		}

		FINISH_ENCODE();
	}

	ENCODE(OP_SkillUpdate)
	{
		ENCODE_LENGTH_EXACT(SkillUpdate_Struct);
		SETUP_DIRECT_ENCODE(SkillUpdate_Struct, structs::SkillUpdate_Struct);

		OUT(skillId);
		OUT(value);
		eq->active = 1;

		FINISH_ENCODE();
	}

	ENCODE(OP_SpecialMesg)
	{
		EQApplicationPacket* in = *p;
		*p = nullptr;

		SerializeBuffer buf(in->size);
		buf.WriteInt8(in->ReadUInt8()); // speak mode
		buf.WriteInt8(in->ReadUInt8()); // journal mode
		buf.WriteInt8(in->ReadUInt8()); // language
		buf.WriteInt32(in->ReadUInt32()); // message type
		buf.WriteInt32(in->ReadUInt32()); // target spawn id

		std::string name;
		in->ReadString(name); // NPC names max out at 63 chars

		buf.WriteString(name);

		buf.WriteInt32(in->ReadUInt32()); // loc
		buf.WriteInt32(in->ReadUInt32());
		buf.WriteInt32(in->ReadUInt32());

		std::string old_message;
		std::string new_message;

		in->ReadString(old_message);

		ServerToTOBConvertLinks(new_message, old_message);

		buf.WriteString(new_message);

		auto outapp = new EQApplicationPacket(OP_SpecialMesg, std::move(buf));

		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	ENCODE(OP_SpawnAppearance)
	{
		EQApplicationPacket* in = *p;
		*p = nullptr;
	
		unsigned char* emu_buffer = in->pBuffer;
	
		SpawnAppearance_Struct* sas = (SpawnAppearance_Struct*)emu_buffer;
	
		if (sas->type != AppearanceType::Size)
		{
			//tob struct is different than rof2's but the idea is the same
			//we will probably want to better implement TOB's structure later
			auto outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(structs::SpawnAppearance_Struct));
			structs::SpawnAppearance_Struct *eq = (structs::SpawnAppearance_Struct*)outapp->pBuffer;
	
			eq->spawn_id = sas->spawn_id;
			eq->type = ServerToTOBSpawnAppearanceType(sas->type);
			eq->parameter = sas->parameter;
	
			dest->FastQueuePacket(&outapp, ack_req);
			delete in;
			return;
		}
	
		auto outapp = new EQApplicationPacket(OP_ChangeSize, sizeof(structs::ChangeSize_Struct));
	
		structs::ChangeSize_Struct* css = (structs::ChangeSize_Struct*)outapp->pBuffer;
	
		css->EntityID = sas->spawn_id;
		css->Size = (float)sas->parameter;
		css->CameraOffset = 0;
		css->AnimationSpeedRelated = 1.0f;
	
		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	ENCODE(OP_SpawnDoor)
	{
		SETUP_VAR_ENCODE(Door_Struct);
		int door_count = __packet->size / sizeof(Door_Struct);
		int total_length = door_count * sizeof(structs::Door_Struct);
		ALLOC_VAR_ENCODE(structs::Door_Struct, total_length);

		int r;
		for (r = 0; r < door_count; r++) {
			strncpy(eq[r].name, emu[r].name, 32);
			eq[r].DefaultY = emu[r].yPos;
			eq[r].DefaultX = emu[r].xPos;
			eq[r].DefaultZ = emu[r].zPos;
			eq[r].DefaultHeading = emu[r].heading;
			eq[r].DefaultDoorAngle = emu[r].incline;
			eq[r].Y = emu[r].yPos;
			eq[r].X = emu[r].xPos;
			eq[r].Z = emu[r].zPos;
			eq[r].Heading = emu[r].heading;
			//there's a door angle here but im not sure if / what we set it to since ive literally never seen it as anything but 0 on live
			//based on pattern it probably is supposed to match the default angle?
			//I'm not 100% sure this is a float it might be a uint32
			eq[r].DoorAngle = emu[r].incline;
			eq[r].ScaleFactor = emu[r].size;
			eq[r].Id = emu[r].doorId;
			eq[r].Type = emu[r].opentype;
			eq[r].State = emu[r].state_at_spawn;
			eq[r].DefaultState = emu[r].invert_state;
			eq[r].Param = emu[r].door_param;
			eq[r].bVisible = 1;
			eq[r].bUsable = 1;
		}

		FINISH_ENCODE();
	}

	ENCODE(OP_Stun)
	{
		ENCODE_LENGTH_EXACT(Stun_Struct);
		SETUP_DIRECT_ENCODE(Stun_Struct, structs::Stun_Struct);

		OUT(duration);
		eq->unknown005 = 163;
		eq->unknown006 = 67;

		FINISH_ENCODE();
	}

	ENCODE(OP_WearChange)
	{
		ENCODE_LENGTH_EXACT(WearChange_Struct);
		SETUP_DIRECT_ENCODE(WearChange_Struct, structs::WearChange_Struct);

		OUT(spawn_id);
		eq->wear_slot_id = emu->wear_slot_id;
		eq->armor_id = emu->material;
		eq->variation = emu->unknown06;
		eq->material = emu->elite_material;
		eq->new_armor_id = emu->hero_forge_model;
		eq->new_armor_type = emu->unknown18;
		eq->color = emu->color.Color;

		FINISH_ENCODE();
	}

	ENCODE(OP_ZoneChange)
	{
		ENCODE_LENGTH_EXACT(ZoneChange_Struct);
		SETUP_DIRECT_ENCODE(ZoneChange_Struct, structs::ZoneChange_Struct);

		OUT_str(char_name);
		OUT(zoneID);
		OUT(instanceID);
		OUT(y);
		OUT(x);
		OUT(z);
		OUT(zone_reason);
		OUT(success);

		if (eq->success < 0)
			eq->success -= 1;

		FINISH_ENCODE();
	}

	ENCODE(OP_ZoneEntry) { ENCODE_FORWARD(OP_ZoneSpawns); }

	ENCODE(OP_ZonePlayerToBind)
	{
		SETUP_VAR_ENCODE(ZonePlayerToBind_Struct);
		ALLOC_LEN_ENCODE(sizeof(structs::ZonePlayerToBind_Struct) + strlen(emu->zone_name));

		__packet->SetWritePosition(0);
		__packet->WriteUInt16(emu->bind_zone_id);
		__packet->WriteUInt16(emu->bind_instance_id);
		__packet->WriteFloat(emu->x);
		__packet->WriteFloat(emu->y);
		__packet->WriteFloat(emu->z);
		__packet->WriteFloat(emu->heading);
		__packet->WriteString(emu->zone_name);
		__packet->WriteUInt32(60);
		__packet->WriteUInt32(0);
		__packet->WriteUInt32(51);
		__packet->WriteUInt32(41);

		FINISH_ENCODE();
	}

	ENCODE(OP_ZoneSpawns)
	{
		EQApplicationPacket* in = *p;
		*p = nullptr;

		//store away the emu struct
		unsigned char* __emu_buffer = in->pBuffer;
		Spawn_Struct* emu = (Spawn_Struct*)__emu_buffer;

		int entrycount = in->size / sizeof(Spawn_Struct);
		if (entrycount == 0 || (in->size % sizeof(Spawn_Struct)) != 0) {
			LogNetcode("[STRUCTS] Wrong size on outbound [{}]: Got [{}], expected multiple of [{}]", opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(Spawn_Struct));
			delete in;
			return;
		}
		
		for (int i = 0; i < entrycount; i++, emu++) {
			SerializeBuffer buffer;

			auto SpawnSize = emu->size;
			if (!((emu->NPC == 0) || (emu->race <= Race::Gnome) || (emu->race == Race::Iksar) ||
				(emu->race == Race::VahShir) || (emu->race == Race::Froglok2) || (emu->race == Race::Drakkin))
				)
			{
				if (emu->size == 0)
				{
					emu->size = 6;
					SpawnSize = 6;
				}
			}

			if (SpawnSize == 0)
			{
				SpawnSize = 3;
			}

			/*
			char Name[];
			u32 SpawnId;
			u8 Level;
			float MeleeRadius;
			*/
			buffer.WriteString(emu->name);
			buffer.WriteUInt32(emu->spawnId);
			buffer.WriteUInt8(emu->level);
			if (emu->DestructibleObject) //bounding radius: we should consider supporting this officially in the future
			{
				buffer.WriteFloat(10.0f);
			}
			else
			{

				buffer.WriteFloat(SpawnSize - 0.7f);
			}

			buffer.WriteFloat(1.0f); // This has something to do with collisions, generally between 1.0-1.1

			/*
			EqGuid HashKey; -- this is actually uint64 in the client
			*/
			buffer.WriteUInt32(emu->CharacterGuid.Id);
			buffer.WriteUInt16(emu->CharacterGuid.WorldId);
			buffer.WriteUInt16(0);

			/*
			u8 Type;
			ActorFlags Flags;
			*/
			buffer.WriteUInt8(emu->NPC);

			structs::Spawn_Struct_Bitfields flags;
			memset(&flags, 0, sizeof(structs::Spawn_Struct_Bitfields));

			flags.gender = emu->gender;
			flags.ispet = emu->is_pet;
			flags.afk = emu->afk;
			flags.anon = emu->anon;
			flags.gm = emu->gm;
			flags.sneak = 0;
			flags.lfg = emu->lfg;
			flags.invis = emu->invis; //we need to implement this
			flags.linkdead = 0; //on live I often see this as 1 for npcs, maybe consider adding this in the future
			flags.showhelm = emu->showhelm;
			flags.trader = emu->trader ? 1 : 0;
			flags.targetable = 1;
			flags.targetable_with_hotkey = emu->targetable_with_hotkey ? 1 : 0;
			flags.showname = emu->show_name;
			flags.buyer = emu->buyer ? 1 : 0;
			flags.title = strlen(emu->title) > 0 ? 1 : 0;
			flags.suffix = strlen(emu->suffix) > 0 ? 1 : 0;

			if (emu->DestructibleObject || emu->class_ == Class::LDoNTreasure) {
				flags.interactiveobject = 1;
			}

			//write flags
			//buffer.WriteStructure(flags);
			for (int j = 0; j < 5; ++j) {
				buffer.WriteUInt8(flags.raw[j]);
			}

			/*
			float EmitterScalingRadius;
			s32 DefaultEmitterID;
			*/

			//we don't support this atm; wouldn't be hard to add I don't think
			//RoF also supports this so might be worth implementing.
			buffer.WriteFloat(1.0f);
			buffer.WriteInt32(-1);

			if (emu->DestructibleObject || emu->class_ == Class::LDoNTreasure) // flags & interactiveobject
			{
				/*
				char InteractiveObjectModelName[];
				char InteractiveObjectName[];
				char InteractiveObjectOtherName[];
				*/

				buffer.WriteString(emu->DestructibleModel);
				buffer.WriteString(emu->DestructibleName2);
				buffer.WriteString(emu->DestructibleString);

				/*
				s32 CurrIOState;
				s32 ObjectAnimationID;
				*/
				buffer.WriteUInt32(emu->DestructibleAppearance);
				buffer.WriteUInt32(emu->DestructibleUnk1);

				/*
				s32 SoundId[10];
				*/
				buffer.WriteUInt32(emu->DestructibleID1);
				buffer.WriteUInt32(emu->DestructibleID2);
				buffer.WriteUInt32(emu->DestructibleID3);
				buffer.WriteUInt32(emu->DestructibleID4);
				buffer.WriteUInt32(emu->DestructibleUnk2);
				buffer.WriteUInt32(emu->DestructibleUnk3);
				buffer.WriteUInt32(emu->DestructibleUnk4);
				buffer.WriteUInt32(emu->DestructibleUnk5);
				buffer.WriteUInt32(emu->DestructibleUnk6);
				buffer.WriteUInt32(emu->DestructibleUnk7);
				/*
				s8 Collidable;
				s8 ObjectType;
				*/
				buffer.WriteUInt8(emu->DestructibleUnk8);
				buffer.WriteUInt8(emu->DestructibleUnk9);
			}

			/*
			u8 PropertyCount;
			u32 Properties[PropertyCount];
			*/
			//We don't actually support multiple body types yet, but we should consider it in the future

			if (!emu->DestructibleObject)
			{
				buffer.WriteUInt8(1);
				buffer.WriteUInt32(emu->bodytype);
			}
			else
			{
				buffer.WriteUInt8(0);
			}

			/*
			u8 HPCurrentPct;
			*/
			buffer.WriteUInt8(emu->curHp);

			/*
			s8 HairColor;
			s8 FacialHairColor;
			s8 EyeColor1;
			s8 EyeColor2;
			s8 HairStyle;
			s8 FacialHair;
			s32 Heritage;
			s32 Tattoo;
			s32 Details;
			*/

			buffer.WriteUInt8(emu->haircolor);
			buffer.WriteUInt8(emu->beardcolor);
			buffer.WriteUInt8(emu->eyecolor1);
			buffer.WriteUInt8(emu->eyecolor2);
			buffer.WriteUInt8(emu->hairstyle);
			buffer.WriteUInt8(emu->beard);
			buffer.WriteUInt32(emu->drakkin_heritage);
			buffer.WriteUInt32(emu->drakkin_tattoo);
			buffer.WriteUInt32(emu->drakkin_details);

			/*
			s8 TextureType;
			s8 Material;
			s8 Variation;
			s8 HeadType;
			*/
			buffer.WriteUInt8(emu->equip_chest2);
			buffer.WriteUInt8(0);
			buffer.WriteUInt8(0);
			buffer.WriteUInt8(emu->helm);

			/*
			float Height;
			s8 FaceStyle;
			float MyWalkSpeed;
			float RunSpeed;
			s32 Race;
			*/

			buffer.WriteFloat(emu->size);
			buffer.WriteInt8(emu->face);
			buffer.WriteFloat(emu->walkspeed);
			buffer.WriteFloat(emu->runspeed);
			buffer.WriteInt32(emu->race);

			/*
			u8 HoldingAnimation;
			u32 Deity;
			EqGuid GuildID;
			u32 Class;
			*/

			buffer.WriteUInt8(0);
			buffer.WriteUInt32(emu->deity);
			if (emu->NPC) {
				buffer.WriteInt32(-1);
				buffer.WriteUInt32(0);
			}
			else { //guilds will probably need a ton of work
				buffer.WriteUInt32(emu->guildID);
				buffer.WriteUInt32(0);
			}
			buffer.WriteUInt32(emu->class_);

			/*
			u8 PvP;
			u8 StandState;
			u8 Light;
			u8 GravityBehavior;
			*/

			buffer.WriteUInt8(0);
			buffer.WriteUInt8(emu->StandState);
			buffer.WriteUInt8(emu->light);
			buffer.WriteUInt8(emu->flymode);

			/*
			char LastName[];
			*/
			buffer.WriteString(emu->lastName);

			/*
			u8 bGuildShowAnim;
			u8 bTempPet;
			u32 MasterID;
			u8 FindBits;
			*/

			buffer.WriteUInt8(emu->guild_show);
			buffer.WriteUInt8(0);
			buffer.WriteUInt32(emu->petOwnerId);
			buffer.WriteUInt8(0);

			/*
			u32 PlayerState;
			u32 NpcTintIndex;
			u32 PrimaryTintIndex;
			u32 SecondaryTintIndex;
			*/

			buffer.WriteUInt32(emu->PlayerState);
			buffer.WriteUInt32(0);
			buffer.WriteUInt32(0);
			buffer.WriteUInt32(0);

			/*
			u32 EncounterLockState;
			u64 LockID;
			*/

			buffer.WriteUInt32(0);
			buffer.WriteUInt64(0);

			//u32 SeeInvis[3];
			if (emu->NPC == 1) {
				buffer.WriteUInt32(0);
				buffer.WriteUInt32(0);
				buffer.WriteUInt32(0);
			}

			/*
			s32 Primary;
			s32 Secondary;
			*/
			buffer.WriteUInt32(0xffffffff);
			buffer.WriteUInt32(0xffffffff);

			if ((emu->NPC == 0) || (emu->race <= Race::Gnome) || (emu->race == Race::Iksar) ||
				(emu->race == Race::VahShir) || (emu->race == Race::Froglok2) || (emu->race == Race::Drakkin)
				)
			{
				/*
				u32 ArmorColor[9];
				*/
				for (int k = EQ::textures::textureBegin; k < EQ::textures::materialCount; ++k)
				{
					buffer.WriteUInt32(emu->equipment_tint.Slot[k].Color);
				}

				/*
				Armor Armor[9];
				*/
				for (int k = EQ::textures::textureBegin; k < EQ::textures::materialCount; k++) {
					buffer.WriteUInt32(emu->equipment.Slot[k].Material);
					buffer.WriteUInt32(emu->equipment.Slot[k].Unknown1);
					buffer.WriteUInt32(emu->equipment.Slot[k].EliteModel);
					buffer.WriteUInt32(emu->equipment.Slot[k].HerosForgeModel);
					buffer.WriteUInt32(emu->equipment.Slot[k].Unknown2);
				}
			}
			else
			{
				//Armor Armor[3];
				buffer.WriteUInt32(0);
				buffer.WriteUInt32(0);
				buffer.WriteUInt32(0);
				buffer.WriteUInt32(0);
				buffer.WriteUInt32(0);

				buffer.WriteUInt32(emu->equipment.Primary.Material);
				buffer.WriteUInt32(0);
				buffer.WriteUInt32(0);
				buffer.WriteUInt32(0);
				buffer.WriteUInt32(0);

				buffer.WriteUInt32(emu->equipment.Secondary.Material);
				buffer.WriteUInt32(0);
				buffer.WriteUInt32(0);
				buffer.WriteUInt32(0);
				buffer.WriteUInt32(0);
			}

			//u8 CPhysicsData[20];
			structs::Spawn_Struct_Position position;
			memset(&position, 0, sizeof(structs::Spawn_Struct_Position));

			position.y = emu->y;
			position.deltaZ = emu->deltaZ;
			position.deltaX = emu->deltaX;
			position.x = emu->x;
			position.heading = emu->heading;
			position.deltaHeading = emu->deltaHeading;
			position.z = emu->z;
			position.animation = emu->animation;
			position.deltaY = emu->deltaY;

			//buffer.WriteStructure(position);
			for (int j = 0; j < 5; ++j) {
				buffer.WriteUInt32(position.raw[j]);
			}

			/*
			if(Flags.title) {
				char Title[];
			}
			*/
			if (flags.title) {
				buffer.WriteString(emu->title);
			}

			/*
			if(Flags.suffix) {
				char Suffix[];
			}
			*/
			if (flags.suffix) {
				buffer.WriteString(emu->suffix);
			}

			/*
			u32 Unknown0x0164;
			s32 SplineID;
			*/

			buffer.WriteUInt32(0);
			buffer.WriteUInt32(0);

			/*
			u8 Mercenary;
			*/
			buffer.WriteUInt8(emu->IsMercenary);

			/*
				char realEstateItemGuid[];
				s32 RealEstateID;
				s32 RealEstateItemId;
			*/

			buffer.WriteString("0000000000000000");
			buffer.WriteInt32(-1);
			buffer.WriteInt32(-1);

			/*
			s32 MercId;
			s32 ContractorID;
			u32 Birthdate;
			u8 bAlwaysShowAura;
			*/

			buffer.WriteInt32(0);
			buffer.WriteInt32(0);
			buffer.WriteUInt32(0);
			buffer.WriteUInt8(0);

			/*
			u32 physicsEffectCount;
			PhysicsEffect physicsEffects[physicsEffectCount];
			*/
			buffer.WriteUInt32(0);

			//s32 SpawnStatus[6];
			buffer.WriteUInt32(0);
			buffer.WriteUInt32(0);
			buffer.WriteUInt32(0);
			buffer.WriteUInt32(0);
			buffer.WriteUInt32(0);
			buffer.WriteUInt32(0);

			if (flags.interactiveobject && emu->DestructibleUnk9 == 4) {
				/*
				s32 BannerIndex0;
				s32 BannerIndex1;
				s32 BannerTint0;
				s32 BannerTint1;
				*/
				buffer.WriteUInt32(0);
				buffer.WriteUInt32(0);
				buffer.WriteUInt32(0);
				buffer.WriteUInt32(0);
			}

			auto outapp = new EQApplicationPacket(OP_ZoneEntry, buffer.size());
			outapp->WriteData(buffer.buffer(), buffer.size());
			dest->FastQueuePacket(&outapp, ack_req);
		}

		delete in;
	}

	// DECODE methods
	DECODE(OP_Animation)
	{
		DECODE_LENGTH_EXACT(structs::Animation_Struct);
		SETUP_DIRECT_DECODE(Animation_Struct, structs::Animation_Struct);

		IN(spawnid);
		IN(action);
		IN(speed);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ApplyPoison)
	{
		DECODE_LENGTH_EXACT(structs::ApplyPoison_Struct);
		SETUP_DIRECT_DECODE(ApplyPoison_Struct, structs::ApplyPoison_Struct);

		emu->inventorySlot = TOBToServerTypelessSlot(eq->inventorySlot, invtype::typePossessions);
		IN(success);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ApproveName)
	{
		DECODE_LENGTH_EXACT(structs::NameApproval_Struct);
		SETUP_DIRECT_DECODE(NameApproval_Struct, structs::NameApproval_Struct);

		IN_str(name);
		IN(race_id);
		IN(class_id);

		// TODO: expand the approval logic to include the rest of the TOB struct values (and remove the direct translation here)

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_AugmentInfo)
	{
		DECODE_LENGTH_EXACT(structs::AugmentInfo_Struct);
		SETUP_DIRECT_DECODE(AugmentInfo_Struct, structs::AugmentInfo_Struct);

		IN(itemid);
		IN(window);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_AugmentItem)
	{
		DECODE_LENGTH_EXACT(structs::AugmentItem_Struct);
		SETUP_DIRECT_DECODE(AugmentItem_Struct, structs::AugmentItem_Struct);

		emu->container_slot = TOBToServerSlot(eq->container_slot);
		emu->augment_slot = TOBToServerSlot(eq->augment_slot);
		emu->container_index = eq->container_index;
		emu->augment_index = eq->augment_index;
		emu->dest_inst_id = eq->dest_inst_id;
		emu->augment_action = eq->augment_action;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_BlockedBuffs)
	{
		uint32 count = __packet->ReadUInt32();
		std::vector<int32> blocked_spell_ids;
		blocked_spell_ids.reserve(count);
		for (int i = 0; i < count; ++i)
			blocked_spell_ids.push_back(static_cast<int32>(__packet->ReadUInt32()));

		bool pet = __packet->ReadUInt8() == 1;
		bool init = __packet->ReadUInt8() == 1;

		__packet->SetReadPosition(0); // reset the packet read to pass it along

		__packet->size = sizeof(BlockedBuffs_Struct);
		__packet->pBuffer = new unsigned char[__packet->size]{};
		BlockedBuffs_Struct* emu = (BlockedBuffs_Struct*)__packet->pBuffer;

		memset(emu->SpellID, -1, sizeof(emu->SpellID));
		for (int i = 0; i < count; ++i)
			emu->SpellID[i] = blocked_spell_ids[i];

		emu->Count = count;
		emu->Pet = pet;
		emu->Initialise = init;
	}

	DECODE(OP_BuffRemoveRequest)
	{
		// This is to cater for the fact that short buff box buffs start at 30 as opposed to 25 in prior clients.
		//
		DECODE_LENGTH_EXACT(BuffRemoveRequest_Struct);
		SETUP_DIRECT_DECODE(BuffRemoveRequest_Struct, BuffRemoveRequest_Struct);

		emu->SlotID = TOBToServerBuffSlot(eq->SlotID);

		IN(EntityID);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_CastSpell)
	{
		DECODE_LENGTH_EXACT(structs::CastSpell_Struct);
		SETUP_DIRECT_DECODE(CastSpell_Struct, structs::CastSpell_Struct);

		emu->slot = static_cast<uint32>(TOBToServerCastingSlot(static_cast<spells::CastingSlot>(eq->slot)));

		IN(spell_id);
		emu->inventoryslot = -1;
		IN(target_id);
		IN(y_pos);
		IN(x_pos);
		IN(z_pos);
		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ChannelMessage)
	{
		unsigned char* __eq_buffer = __packet->pBuffer;

		char* InBuffer = (char*)__eq_buffer;

		char Sender[64];
		char Target[64];

		VARSTRUCT_DECODE_STRING(Sender, InBuffer);
		VARSTRUCT_DECODE_STRING(Target, InBuffer);

		//packet seems the same as rof2 with 4 more empty bytes before language
		InBuffer += 8;

		uint32 Language = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);
		uint32 Channel = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);

		InBuffer += 5;

		uint32 Skill = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);

		// this has a size limit of 11k in the client
		std::string old_message = InBuffer;
		std::string new_message;
		TOBToServerConvertLinks(new_message, old_message);

		// there are 15 bytes after this, part of which is an unk string, check the ENCODE for the layout

		__packet->size = sizeof(ChannelMessage_Struct) + new_message.length() + 1;
		__packet->pBuffer = new unsigned char[__packet->size];
		ChannelMessage_Struct* emu = (ChannelMessage_Struct*)__packet->pBuffer;

		strn0cpy(emu->targetname, Target, sizeof(emu->targetname));
		strn0cpy(emu->sender, Target, sizeof(emu->sender));
		emu->language = Language;
		emu->chan_num = Channel;
		emu->skill_in_language = Skill;
		strcpy(emu->message, new_message.c_str());

		delete[] __eq_buffer;
	}

	DECODE(OP_CharacterCreate) {
		DECODE_LENGTH_EXACT(structs::CharCreate_Struct);
		SETUP_DIRECT_DECODE(CharCreate_Struct, structs::CharCreate_Struct);

		IN(gender);
		IN(race);
		IN(class_);
		IN(deity);
		IN(start_zone);
		IN(haircolor);
		IN(beard);
		IN(beardcolor);
		IN(hairstyle);
		IN(face);
		IN(eyecolor1);
		IN(eyecolor2);
		IN(drakkin_heritage);
		IN(drakkin_tattoo);
		IN(drakkin_details);
		IN(STR);
		IN(STA);
		IN(AGI);
		IN(DEX);
		IN(WIS);
		IN(INT);
		IN(CHA);
		IN(tutorial);

		// TODO: can handle the heroic type here as well (new member)

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ClickDoor)
	{
		DECODE_LENGTH_EXACT(structs::ClickDoor_Struct);
		SETUP_DIRECT_DECODE(ClickDoor_Struct, structs::ClickDoor_Struct);

		IN(doorid);
		IN(player_id);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ClientUpdate)
	{
		// for some odd reason, there is an extra byte on the end of this on occasion..
		DECODE_LENGTH_ATLEAST(structs::PlayerPositionUpdateClient_Struct);
		SETUP_DIRECT_DECODE(PlayerPositionUpdateClient_Struct, structs::PlayerPositionUpdateClient_Struct);

		IN(spawn_id);
		IN(vehicle_id);
		IN(sequence);
		emu->x_pos = eq->position.x;
		emu->y_pos = eq->position.y;
		emu->z_pos = eq->position.z;
		emu->heading = eq->position.heading;
		emu->delta_x = eq->position.delta_x;
		emu->delta_y = eq->position.delta_y;
		emu->delta_z = eq->position.delta_z;
		emu->delta_heading = eq->position.delta_heading;
		emu->animation = eq->position.animation;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_Consider)
	{
		DECODE_LENGTH_EXACT(structs::Consider_Struct);
		SETUP_DIRECT_DECODE(Consider_Struct, structs::Consider_Struct);

		IN(playerid);
		IN(targetid);
		IN(faction);
		IN(level);
		//emu->cur_hp = 1;
		//emu->max_hp = 2;
		//emu->pvpcon = 0;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ConsiderCorpse) { DECODE_FORWARD(OP_Consider); }

	DECODE(OP_CorpseDrag)
	{
		std::string CorpseName;
		__packet->ReadLengthString(CorpseName);

		std::string DraggerName;
		__packet->ReadLengthString(DraggerName);

		__packet->SetReadPosition(0);
		__packet->size = sizeof(CorpseDrag_Struct);
		__packet->pBuffer = new unsigned char[__packet->size]{};
		auto* emu = reinterpret_cast<CorpseDrag_Struct*>(__packet->pBuffer);

		strncpy(emu->CorpseName, CorpseName.c_str(), 64);
		strncpy(emu->DraggerName, DraggerName.c_str(), 64);
	}

	DECODE(OP_DeleteItem)
	{
		DECODE_LENGTH_EXACT(structs::DeleteItem_Struct);
		SETUP_DIRECT_DECODE(DeleteItem_Struct, structs::DeleteItem_Struct);

		emu->from_slot = TOBToServerSlot(eq->from_slot);
		emu->to_slot = TOBToServerSlot(eq->to_slot);
		IN(number_in_stack);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_EnterWorld)
	{
		DECODE_LENGTH_EXACT(structs::EnterWorld_Struct);
		SETUP_DIRECT_DECODE(EnterWorld_Struct, structs::EnterWorld_Struct);

		memcpy(emu->name, eq->name, sizeof(emu->name));
		emu->return_home = 0;
		emu->tutorial = 0;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GMTraining)
	{
		DECODE_LENGTH_EXACT(structs::GMTrainee_Struct);
		SETUP_DIRECT_DECODE(GMTrainee_Struct, structs::GMTrainee_Struct);

		IN(npcid);
		IN(playerid);

		for (int i = 0; i < 100; ++i) {
			IN(skills[i]);
		}

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GroupDisband)
	{
		DECODE_LENGTH_EXACT(structs::GroupGeneric_Struct);
		SETUP_DIRECT_DECODE(GroupGeneric_Struct, structs::GroupGeneric_Struct);

		memcpy(emu->name1, eq->name1, sizeof(emu->name1));
		memcpy(emu->name2, eq->name2, sizeof(emu->name2));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GroupInvite)
	{
		DECODE_LENGTH_EXACT(structs::GroupGeneric_Struct);
		SETUP_DIRECT_DECODE(GroupGeneric_Struct, structs::GroupGeneric_Struct);

		memcpy(emu->name1, eq->name1, sizeof(emu->name1));
		memcpy(emu->name2, eq->name2, sizeof(emu->name2));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GroupInvite2)
	{
		DECODE_FORWARD(OP_GroupInvite);
	}

	DECODE(OP_LootItem)
	{
		DECODE_LENGTH_EXACT(structs::LootingItem_Struct);
		SETUP_DIRECT_DECODE(LootingItem_Struct, structs::LootingItem_Struct);

		Log(Logs::Detail, Logs::Netcode, "TOB::DECODE(OP_LootItem)");

		IN(lootee);
		IN(looter);
		emu->slot_id = TOBToServerCorpseMainSlot(eq->slot_id);
		IN(auto_loot);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_MemorizeSpell) {
		DECODE_LENGTH_EXACT(structs::MemorizeSpell_Struct);
		SETUP_DIRECT_DECODE(MemorizeSpell_Struct, structs::MemorizeSpell_Struct);

		// TOB sends status 1 here to let the server know that it's started memming, but doesn't want a response
		if (eq->scribing == 1) {
			// TODO: There should be a timer set here to detect short-mem cheats, and then checked when the 2 packet is sent
			// The previous detection will still happen on scribing == 2, the new client just handles it better
			__packet->SetOpcode(OP_Unknown);
			return;
		}

		// in TOB, 2 is "finish memming" so that becomes 1 in emu and 3 is "unmem" which becomes 2
		if (eq->scribing == 2)
			emu->scribing = 1;
		else if (eq->scribing == 3)
			emu->scribing = 2;
		else if (eq->scribing == 4)
			emu->scribing = 3;
		else
			IN(scribing);

		IN(slot);
		IN(spell_id);
		IN(reduction);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_MoveItem)
	{
		DECODE_LENGTH_EXACT(structs::MoveItem_Struct);
		SETUP_DIRECT_DECODE(MoveItem_Struct, structs::MoveItem_Struct);

		Log(Logs::Detail, Logs::Netcode, "TOB::DECODE(OP_MoveItem)");

		emu->from_slot = TOBToServerSlot(eq->from_slot);
		emu->to_slot = TOBToServerSlot(eq->to_slot);
		IN(number_in_stack);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_RemoveBlockedBuffs) { DECODE_FORWARD(OP_BlockedBuffs); }

	DECODE(OP_SetServerFilter)
	{
		DECODE_LENGTH_EXACT(structs::SetServerFilter_Struct);
		SETUP_DIRECT_DECODE(SetServerFilter_Struct, structs::SetServerFilter_Struct);

		int r;
		for (r = 0; r < 29; r++) {
			// Size 69 in TOB
			IN(filters[r]);
		}

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ShopPlayerBuy)
	{
		DECODE_LENGTH_EXACT(structs::Merchant_Sell_Request_Struct);
		SETUP_DIRECT_DECODE(Merchant_Sell_Struct, structs::Merchant_Sell_Request_Struct);

		IN(npcid);
		IN(playerid);
		IN(itemslot);
		IN(quantity);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ShopPlayerSell)
	{
		DECODE_LENGTH_EXACT(structs::Merchant_Purchase_Request_Struct);
		SETUP_DIRECT_DECODE(Merchant_Purchase_Struct, structs::Merchant_Purchase_Request_Struct);

		IN(npcid);
		emu->itemslot = TOBToServerTypelessSlot(eq->inventory_slot, invtype::typePossessions);
		IN(quantity);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ShopRequest)
	{
		DECODE_LENGTH_EXACT(structs::MerchantClickRequest_Struct);
		SETUP_DIRECT_DECODE(MerchantClick_Struct, structs::MerchantClickRequest_Struct);

		IN(npc_id);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_SpawnAppearance) {
		DECODE_LENGTH_EXACT(structs::SpawnAppearance_Struct);
		SETUP_DIRECT_DECODE(SpawnAppearance_Struct, structs::SpawnAppearance_Struct);

		IN(spawn_id);
		emu->type = TOBToServerSpawnAppearanceType(eq->type);
		IN(parameter);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_TradeSkillCombine)
	{
		DECODE_LENGTH_EXACT(structs::NewCombine_Struct);
		SETUP_DIRECT_DECODE(NewCombine_Struct, structs::NewCombine_Struct);

		emu->container_slot = TOBToServerSlot(eq->container_slot);
		emu->guildtribute_slot = TOBToServerSlot(eq->guildtribute_slot); // this should only return INVALID_INDEX until implemented

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_WearChange)
	{
		DECODE_LENGTH_EXACT(structs::WearChange_Struct);
		SETUP_DIRECT_DECODE(WearChange_Struct, structs::WearChange_Struct);

		IN(spawn_id);
		emu->wear_slot_id = eq->wear_slot_id;
		emu->material = eq->armor_id;
		emu->unknown06 = eq->variation;
		emu->elite_material = eq->material;
		emu->hero_forge_model = eq->new_armor_id;
		emu->unknown18 = eq->new_armor_type;
		emu->color.Color = eq->color;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ZoneEntry)
	{
		DECODE_LENGTH_EXACT(structs::ClientZoneEntry_Struct);
		SETUP_DIRECT_DECODE(ClientZoneEntry_Struct, structs::ClientZoneEntry_Struct);

		IN_str(char_name);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ZoneChange)
	{
		DECODE_LENGTH_EXACT(structs::ZoneChange_Struct);
		SETUP_DIRECT_DECODE(ZoneChange_Struct, structs::ZoneChange_Struct);

		memcpy(emu->char_name, eq->char_name, sizeof(emu->char_name));
		IN(zoneID);
		IN(instanceID);
		IN(y);
		IN(x);
		IN(z)
			IN(zone_reason);
		IN(success);

		FINISH_DIRECT_DECODE();
	}

	//Naive version but should work well enough for now
	int ExtractIDFile(const std::string& input) {
		std::string number;
		for (char ch : input) {
			if (std::isdigit(static_cast<unsigned char>(ch))) {
				number += ch;
			}
		}

		if (number.empty()) {
			return 0;
		}

		return std::stoi(number);
	}

	// Helper Functions
	void SerializeItemDefinition(SerializeBuffer& buffer, const EQ::ItemData* item) {
		//u8 Type;
		buffer.WriteUInt8(item->ItemClass);
		//char Name[];
		buffer.WriteString(item->Name);

		//char LoreName[];
		buffer.WriteString(item->Lore);

		//we need to parse id file
		//s32 IDFile;
		int32 idfile = ExtractIDFile(item->IDFile);
		buffer.WriteUInt32(idfile);

		//s32 IDFile2;
		buffer.WriteUInt32(0); //unsupported atm

		/*
		ibs.id = item->ID;
		ibs.weight = item->Weight;
		ibs.norent = item->NoRent;
		ibs.nodrop = item->NoDrop;
		ibs.attune = item->Attuneable;
		ibs.size = item->Size;
		ibs.slots = item->Slots;
		ibs.price = item->Price;
		ibs.icon = item->Icon;
		*/

		//s32 ItemNumber;
		buffer.WriteUInt32(item->ID);
		
		//s32 Weight;
		buffer.WriteInt32(item->Weight);
		
		//bool NoRent;
		buffer.WriteUInt8(item->NoRent);

		//bool IsDroppable;
		buffer.WriteUInt8(item->NoDrop);

		//bool Attuneable;
		buffer.WriteUInt8(item->Attuneable);

		//u8 Size;
		buffer.WriteUInt8(item->Size);

		//u32 EquipSlots;
		buffer.WriteUInt32(item->Slots);

		//u32 Cost;
		buffer.WriteUInt32(item->Price);

		//s32 IconNumber;
		buffer.WriteInt32(item->Icon);

		//bool eGMRequirement;
		buffer.WriteUInt8(0); //unsupported atm

		//bool Tradeskills;
		buffer.WriteUInt8(0); //unsupported atm

		//s8 SvCold;
		//s8 SvDisease;
		//s8 SvPoison;
		//s8 SvMagic;
		//s8 SvFire;
		//s8 SvCorruption;
		buffer.WriteInt8(item->CR);
		buffer.WriteInt8(item->DR);
		buffer.WriteInt8(item->PR);
		buffer.WriteInt8(item->MR);
		buffer.WriteInt8(item->FR);
		buffer.WriteInt8(item->SVCorruption);
		
		//s8 STR;
		//s8 STA;
		//s8 AGI;
		//s8 DEX;
		//s8 CHA;
		//s8 INT;
		//s8 WIS;
		buffer.WriteInt8(item->AStr);
		buffer.WriteInt8(item->ASta);
		buffer.WriteInt8(item->AAgi);
		buffer.WriteInt8(item->ADex);
		buffer.WriteInt8(item->ACha);
		buffer.WriteInt8(item->AInt);
		buffer.WriteInt8(item->AWis);

		//s32 HP;
		//s32 Mana;
		//s32 Endurance;
		//s32 AC;
		buffer.WriteInt32(item->HP);
		buffer.WriteInt32(item->Mana);
		buffer.WriteInt32(item->Endur);
		buffer.WriteInt32(item->AC);

		//s32 HPRegen;
		//s32 ManaRegen;
		//s32 EnduranceRegen;
		buffer.WriteInt32(item->Regen);
		buffer.WriteInt32(item->ManaRegen);
		buffer.WriteInt32(item->EnduranceRegen);
		
		//u32 Classes;
		//u32 Races;
		//u32 Deity;
		buffer.WriteUInt32(item->Classes);
		buffer.WriteUInt32(item->Races);
		buffer.WriteUInt32(item->Deity);

		//u32 SkillModValue;
		//u32 SkillModMax;
		//s32 SkillModType;
		//s32 SkillModBonus;
		buffer.WriteInt32(item->SkillModValue);
		buffer.WriteUInt32(item->SkillModMax);
		buffer.WriteInt32(item->SkillModType);
		buffer.WriteInt32(0); //unsupported atm
		
		//s32 BaneDMGRace;
		//s32 BaneDMGBodyType;
		//s32 BaneDMGRaceValue;
		//s32 BaneDMGBodyTypeValue;
		buffer.WriteInt32(item->BaneDmgRace);
		buffer.WriteInt32(item->BaneDmgBody);
		buffer.WriteInt32(item->BaneDmgRaceAmt);
		buffer.WriteInt32(item->BaneDmgAmt);

		//bool Magic;
		buffer.WriteUInt8(item->Magic);

		//s32 FoodDuration;
		buffer.WriteInt32(item->CastTime_);

		//s32 RequiredLevel;
		buffer.WriteInt32(item->ReqLevel > 125 ? 125 : item->ReqLevel);

		//s32 RecommendedLevel;
		buffer.WriteInt32(item->RecLevel > 125 ? 125 : item->RecLevel);

		//s32 InstrumentType;
		//s32 InstrumentMod;
		buffer.WriteInt32(item->BardType);
		buffer.WriteInt32(item->BardValue);

		//u8 Light;
		buffer.WriteUInt8(item->Light);

		//u8 Delay;
		buffer.WriteUInt8(item->Delay);

		//u8 ElementalFlag;
		//u8 ElementalDamage;
		buffer.WriteUInt8(item->ElemDmgType);
		buffer.WriteUInt8(item->ElemDmgAmt);

		//u8 Range;
		buffer.WriteUInt8(item->Range);

		//u32 Damage;
		buffer.WriteUInt32(item->Damage);

		//u32 MaterialTintIndex;
		//u32 Prestige;
		buffer.WriteUInt32(item->Color);
		buffer.WriteUInt32(0); //unsupported atm
		
		//u8 ItemClass;
		buffer.WriteUInt8(item->ItemType);

		//ArmorProperties properties;
		//s32 Type;
		//s32 Material;
		//s32 Variation;
		//s32 NewArmorId;
		//s32 NewArmorType;
		buffer.WriteUInt32(item->Material); // this isn't labeled well, material is material *type*
		buffer.WriteUInt32(0); //unsupported atm
		buffer.WriteUInt32(item->EliteMaterial);
		buffer.WriteUInt32(item->HerosForgeModel);
		buffer.WriteUInt32(0); //unsupported atm
		
		//float MerchantGreedMod;
		buffer.WriteFloat(item->SellRate);

		//s32 DmgBonusSkill;
		//s32 DmgBonusValue;
		buffer.WriteInt32(item->ExtraDmgSkill);
		buffer.WriteInt32(item->ExtraDmgAmt);

		//s32 ScriptID;
		//char CharmFile[];
		buffer.WriteUInt32(item->CharmFileID);
		buffer.WriteString(item->CharmFile);

		//s32 AugType;
		//u32 AugSkinTypeMask;
		//u32 AugRestrictions;
		buffer.WriteUInt32(item->AugType);
		buffer.WriteInt32(-1); //unsupported atm
		buffer.WriteUInt32(item->AugRestrict);

		//ItemAugmentationSocket AugData[6];
		for (int j = 0; j < 6; ++j) {
			/*
			s32 Type;
			bool Visible;
			bool Infusible;
			*/

			buffer.WriteInt32(item->AugSlotType[j]);
			buffer.WriteUInt8(item->AugSlotVisible[j]);
			buffer.WriteUInt8(item->AugSlotUnk2[j]); //not entirely supported atm
		}
		
		//s32 LDType;
		//s32 LDTheme;
		//s32 LDCost;
		//s32 PointBuyBackPercent;
		//s32 NeedAdventureCompleted;
		buffer.WriteUInt32(item->PointType);
		buffer.WriteUInt32(item->LDoNTheme);
		buffer.WriteUInt32(item->LDoNPrice);
		buffer.WriteUInt32(item->LDoNSellBackRate);
		buffer.WriteUInt32(item->LDoNSold);

		//u8 ContainerType;
		//u8 Slots;
		//u8 SizeCapacity;
		//u8 WeightReduction;
		buffer.WriteUInt8(item->BagType);
		buffer.WriteUInt8(item->BagSlots);
		buffer.WriteUInt8(item->BagSize);
		buffer.WriteUInt8(item->BagWR);

		//u8 BookType;
		//u8 BookLang;
		//char BookFile[];
		buffer.WriteUInt8(item->Book);
		buffer.WriteUInt8(item->BookType); //doesn't match the name for eqlib
		buffer.WriteString(item->Filename);

		//s32 Lore;
		buffer.WriteInt32(item->LoreGroup);

		//bool Artifact;
		buffer.WriteUInt8(item->ArtifactFlag);

		//s32 Favor;
		buffer.WriteUInt32(item->Favor);

		//bool bIsFVNoDrop;
		buffer.WriteUInt8(item->FVNoDrop);

		//s32 Attack;
		//s32 Haste;
		buffer.WriteInt32(item->Attack);
		buffer.WriteInt32(item->Haste);

		//s32 GuildFavor;
		buffer.WriteUInt32(item->GuildFavor);

		//s32 SolventItemID;
		buffer.WriteUInt32(item->AugDistiller);
		
		//s32 AnimationOverride;
		buffer.WriteInt32(-1); //unsupported atm

		//u32 PaletteTintIndex;
		buffer.WriteInt32(0); //unsupported atm

		//bool bNoPetGive;
		buffer.WriteUInt8(item->NoPet);

		//bool bSomeProfile;
		buffer.WriteUInt8(0); //unsupported atm

		//u32 StackSize;
		buffer.WriteUInt32(item->ID == PARCEL_MONEY_ITEM_ID ? 0x7FFFFFFF : ((item->Stackable ? item->StackSize : 0)));
		
		//bool bNoStorage;
		buffer.WriteUInt8(item->NoTransfer);

		//bool Expendable;
		uint8 expendable = item->ExpendableArrow;

		if (item->ItemType == EQ::item::ItemTypeFishingPole && item->SubType == 0) {
			expendable = 1;
		}

		buffer.WriteUInt8(expendable);

		//u8 SpellDataSkillMask[78];
		for (int j = 0; j < 78; ++j) {
			buffer.WriteUInt8(0); // TODO: collection of ints for bitfield for each skill required to use. reads 19 ints byte by byte in the client, leave like this for further investigation
		}


		/* There are a static 7 spell data entries on an item:
			Clicky
			Proc
			Worn
			Focus
			Scroll
			Focus2
			Blessing
		 */

		/* SpellData:
			s32 SpellId;
			u8 RequiredLevel;
			u8 EffectType;
			s32 EffectiveCasterLevel;
			s32 MaxCharges;
			s32 CastTime;
			s32 RecastTime;
			s32 RecastType;
			s32 ProcRate;
			char OverrideName[];
			s32 OverrideDesc;
		*/

		//SpellData SpellDataClicky;
		buffer.WriteInt32(item->Click.Effect);
		buffer.WriteUInt8(item->Click.Level2);
		buffer.WriteUInt8(item->Click.Type);
		buffer.WriteInt32(item->Click.Level);
		buffer.WriteInt32(item->MaxCharges);
		buffer.WriteInt32(item->CastTime);
		buffer.WriteInt32(item->RecastDelay);
		buffer.WriteInt32(item->RecastType);
		buffer.WriteInt32(0); //unsupported atm
		if (strlen(item->ClickName) > 0) {
			buffer.WriteString(item->ClickName);
		}
		else {
			buffer.WriteString("");
		}
		buffer.WriteInt32(0); //unsupported atm

		//SpellData SpellDataProc;
		buffer.WriteInt32(item->Proc.Effect);
		buffer.WriteUInt8(item->Proc.Level2);
		buffer.WriteUInt8(item->Proc.Type);
		buffer.WriteInt32(item->Proc.Level);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0); //unsupported atm; even live sets this to 0 for procs
		if (strlen(item->ProcName) > 0) {
			buffer.WriteString(item->ProcName);
		}
		else {
			buffer.WriteString("");
		}
		buffer.WriteInt32(0); //unsupported atm

		//SpellData SpellDataWorn;
		buffer.WriteInt32(item->Worn.Effect);
		buffer.WriteUInt8(item->Worn.Level2);
		buffer.WriteUInt8(item->Worn.Type);
		buffer.WriteInt32(item->Worn.Level);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		if (strlen(item->WornName) > 0) {
			buffer.WriteString(item->WornName);
		}
		else {
			buffer.WriteString("");
		}
		buffer.WriteInt32(0); //unsupported atm

		//SpellData SpellDataFocus;
		buffer.WriteInt32(item->Focus.Effect);
		buffer.WriteUInt8(item->Focus.Level2);
		buffer.WriteUInt8(item->Focus.Type);
		buffer.WriteInt32(item->Focus.Level);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		if (strlen(item->FocusName) > 0) {
			buffer.WriteString(item->FocusName);
		}
		else {
			buffer.WriteString("");
		}
		buffer.WriteInt32(0); //unsupported atm

		//SpellData SpellDataScroll;
		buffer.WriteInt32(item->Scroll.Effect);
		buffer.WriteUInt8(item->Scroll.Level2);
		buffer.WriteUInt8(item->Scroll.Type);
		buffer.WriteInt32(item->Scroll.Level);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		if (strlen(item->ScrollName) > 0) {
			buffer.WriteString(item->ScrollName);
		}
		else {
			buffer.WriteString("");
		}
		buffer.WriteInt32(0); //unsupported atm

		//SpellData SpellDataFocus2; //unsupported atm
		buffer.WriteInt32(-1);
		buffer.WriteUInt8(0);
		buffer.WriteUInt8(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteString("");
		buffer.WriteInt32(0);

		//SpellData SpellDataBlessing; //unsupported atm
		buffer.WriteInt32(-1);
		buffer.WriteUInt8(0);
		buffer.WriteUInt8(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteInt32(0);
		buffer.WriteString("");
		buffer.WriteInt32(0);		
		
		//s32 RightClickScriptID;
		buffer.WriteInt32(0); //unsupported atm

		//bool QuestItem;
		buffer.WriteInt8(item->QuestItemFlag);

		//s32 MaxPower;
		buffer.WriteInt32(0); //unsupported atm

		//s32 Purity;
		buffer.WriteInt32(item->Purity);

		//s32 BackstabDamage;
		buffer.WriteInt32(item->BackstabDmg);

		//s32 HeroicSTR;
		//s32 HeroicINT;
		//s32 HeroicWIS;
		//s32 HeroicAGI;
		//s32 HeroicDEX;
		//s32 HeroicSTA;
		//s32 HeroicCHA;
		buffer.WriteInt32(item->HeroicStr);
		buffer.WriteInt32(item->HeroicInt);
		buffer.WriteInt32(item->HeroicWis);
		buffer.WriteInt32(item->HeroicAgi);
		buffer.WriteInt32(item->HeroicDex);
		buffer.WriteInt32(item->HeroicSta);
		buffer.WriteInt32(item->HeroicCha);

		//s32 HealAmount;
		//s32 SpellDamage;
		//s32 Clairvoyance;
		buffer.WriteInt32(item->HealAmt);
		buffer.WriteInt32(item->SpellDmg);
		buffer.WriteInt32(item->Clairvoyance);

		//s32 SubClass;
		buffer.WriteInt32(item->SubType);

		//bool bLoginRegReqItem;
		buffer.WriteUInt8(0); //unsupported atm

		//s32 ItemLaunchScriptID;
		buffer.WriteInt32(0); //unsupported atm

		//bool Heirloom;
		buffer.WriteUInt8(0); //unsupported atm

		//s32 Placeable;
		buffer.WriteInt32(0); //unsupported atm

		//bool bPlaceableIgnoreCollisions;
		buffer.WriteUInt8(0);

		//s32 PlacementType;
		buffer.WriteInt32(0); //unsupported atm

		//s32 RealEstateDefID;
		buffer.WriteInt32(0); //unsupported atm

		//float PlaceableScaleRangeMin;
		//float PlaceableScaleRangeMax;
		buffer.WriteFloat(0.0f); //unsupported atm
		buffer.WriteFloat(0.0f); //unsupported atm

		//s32 RealEstateUpkeepID;
		buffer.WriteInt32(0); //unsupported atm

		//s32 MaxPerRealEstate;
		buffer.WriteInt32(-1); //unsupported atm

		//char HousepetFileName[];
		buffer.WriteString(""); //unsupported atm

		//bool bInteractiveObject;
		buffer.WriteUInt8(0); //unsupported atm

		//s32 TrophyBenefitID;
		buffer.WriteInt32(-1); //unsupported atm

		//bool bDisablePlacementRotation;
		//bool bDisableFreePlacement;
		buffer.WriteUInt8(0); //unsupported atm
		buffer.WriteUInt8(0); //unsupported atm

		//s32 NpcRespawnInterval;
		buffer.WriteInt32(0); //unsupported atm

		//float PlaceableDefScale;
		//float PlaceableDefHeading;
		//float PlaceableDefPitch;
		//float PlaceableDefRoll;
		buffer.WriteFloat(0.0f);
		buffer.WriteFloat(0.0f);
		buffer.WriteFloat(0.0f);
		buffer.WriteFloat(0.0f);

		//u8 SocketSubClassCount;
		//s32 SocketSubClass[SocketSubClassCount];
		buffer.WriteUInt8(0); //unsupported atm
		
		//bool Collectible;
		buffer.WriteUInt8(0); //unsupported atm

		//bool NoDestroy;
		buffer.WriteUInt8(0); //unsupported atm
		
		//bool bNoNPC;
		buffer.WriteUInt8(0); //unsupported atm

		//bool NoZone;
		buffer.WriteUInt8(0); //unsupported atm

		//s32 MakerId;
		buffer.WriteInt32(0); //unsupported atm

		//bool NoGround;
		buffer.WriteUInt8(0); //unsupported atm

		//bool bNoLoot;
		buffer.WriteUInt8(0); //unsupported atm

		//bool MarketPlace;
		buffer.WriteUInt8(0); //unsupported atm

		//bool bFreeSlot;
		buffer.WriteUInt8(0); //unsupported atm

		//bool bAutoUse;
		buffer.WriteUInt8(0); //unsupported atm

		//s32 Unknown0x0e4;
		buffer.WriteInt32(-1); //unsupported atm

		//s32 MinLuck;
		//s32 MaxLuck;
		buffer.WriteUInt32(0); //unsupported atm
		buffer.WriteUInt32(0); //unsupported atm

		//s32 LoreEquipped;
		buffer.WriteUInt32(0); //unsupported atm
	}

	void SerializeItem(SerializeBuffer& buffer, const EQ::ItemInstance* inst, int16 slot_id_in, uint8 depth, ItemPacketType packet_type) {
		const EQ::ItemData* item = inst->GetUnscaledItem();

		//char ItemGUID[];
		auto item_guid = fmt::format("{:016}", inst->GetSerialNumber());
		buffer.WriteString(item_guid);

		//u32 StackCount;
		auto stacksize =
			item->ID == PARCEL_MONEY_ITEM_ID ? inst->GetPrice() : (inst->IsStackable() ? ((inst->GetCharges() > 1000)
				? 0xFFFFFFFF : inst->GetCharges()) : 1);
		buffer.WriteUInt32(stacksize);

		structs::InventorySlot_Struct slot_id{};
		switch (packet_type) {
		case ItemPacketLoot:
			slot_id = ServerToTOBCorpseSlot(slot_id_in);
			break;
		default:
			slot_id = ServerToTOBSlot(slot_id_in);
			break;
		}

		//u32 slot_type;
		buffer.WriteUInt32(inst->GetMerchantSlot() ? invtype::typeMerchant : slot_id.Type);
		//s16 main_slot;
		//s16 sub_slot;
		//s16 aug_slot;
		buffer.WriteInt16(inst->GetMerchantSlot() ? inst->GetMerchantSlot() : slot_id.Slot);
		buffer.WriteInt16(inst->GetMerchantSlot() ? 0xffff : slot_id.SubIndex);
		buffer.WriteInt16(inst->GetMerchantSlot() ? 0xffff : slot_id.AugIndex);

		//u64 price;
		buffer.WriteUInt64(inst->GetPrice());

		//u32 MerchantQuantity;
		buffer.WriteUInt32(inst->GetMerchantSlot() ? inst->GetMerchantCount() : 1);

		//u32 ScriptIndex;
		buffer.WriteUInt32(inst->IsScaling() ? (inst->GetExp() / 100) : 0);

		//u64 MerchantSlot;
		buffer.WriteUInt64(inst->GetMerchantSlot() ? inst->GetMerchantSlot() : inst->GetSerialNumber());

		//u32 LastCastTime;
		buffer.WriteUInt32(inst->GetRecastTimestamp());

		//s32 Charges;
		auto charges = (inst->IsStackable() ? (item->MaxCharges ? 1 : 0) : ((inst->GetCharges() > 254)
			? -1
			: inst->GetCharges()));

		buffer.WriteInt32(charges);

		//s32 NoDropFlag;
		buffer.WriteInt32(inst->IsAttuned() ? 1 : 0);

		//s32 Power;
		buffer.WriteInt32(0);

		//s32 AugFlag;
		buffer.WriteInt32(0);

		//bool bConvertable;
		buffer.WriteInt8(0);

		//u32 ConvertItemNameLength;
		buffer.WriteInt32(0);

		//char ConvertItemName[ConvertItemNameLength];

		//u32 ConvertItemID;
		buffer.WriteInt32(0);

		//u32 Open;
		buffer.WriteInt32(0);

		//bool EvolvingItem;
		buffer.WriteInt8(item->EvolvingItem);

		//EvoData evoData;
		if (item->EvolvingItem > 0) {
			//s32 GroupId;
			buffer.WriteInt32(0);

			//s32 EvolvingCurrentLevel;
			buffer.WriteInt32(item->EvolvingLevel);

			//double EvolvingExpPct;
			buffer.WriteDouble(0.0);
			
			//s32 EvolvingMaxLevel;
			buffer.WriteInt32(item->EvolvingMax);

			//s32 LastEquipped;
			buffer.WriteInt32(0);
		}

		uint32 ornamentation_icon = (inst->GetOrnamentationIcon() ? inst->GetOrnamentationIcon() : 0);
		uint32 hero_model = 0;

		//s32 ActorTag1;
		//s32 ActorTag2;
		if (inst->GetOrnamentationIDFile()) {
			hero_model = inst->GetOrnamentHeroModel(EQ::InventoryProfile::CalcMaterialFromSlot(slot_id_in));

			buffer.WriteInt32(inst->GetOrnamentationIDFile());
			buffer.WriteInt32(inst->GetOrnamentationIDFile());
		}
		else {
			buffer.WriteInt32(0);
			buffer.WriteInt32(0);
		}

		//s32 OrnamentationIcon;
		//s32 ArmorType;
		//s32 NewArmorID;
		//u32 Tint;
		buffer.WriteInt32(ornamentation_icon);
		buffer.WriteInt32(-1);
		buffer.WriteInt32(hero_model);
		buffer.WriteInt32(0);

		//bool bCopied;
		buffer.WriteUInt8(0);

		//s32 RealEstateID;
		buffer.WriteInt32(-1);
		//s32 RespawnTime;
		buffer.WriteInt32(0);

		//ItemDefinition Item;
		SerializeItemDefinition(buffer, item);

		//u32 SubContentSize;
		int16 SubSlotNumber = EQ::invbag::SLOT_INVALID;
		
		if (slot_id_in <= EQ::invslot::GENERAL_END && slot_id_in >= EQ::invslot::GENERAL_BEGIN)
			SubSlotNumber = EQ::invbag::GENERAL_BAGS_BEGIN + (slot_id_in - EQ::invslot::GENERAL_BEGIN) * EQ::invbag::SLOT_COUNT;
		else if (slot_id_in == EQ::invslot::slotCursor)
			SubSlotNumber = EQ::invbag::CURSOR_BAG_BEGIN;
		else if (slot_id_in <= EQ::invslot::BANK_END && slot_id_in >= EQ::invslot::BANK_BEGIN)
			SubSlotNumber = EQ::invbag::BANK_BAGS_BEGIN + (slot_id_in - EQ::invslot::BANK_BEGIN) * EQ::invbag::SLOT_COUNT;
		else if (slot_id_in <= EQ::invslot::SHARED_BANK_END && slot_id_in >= EQ::invslot::SHARED_BANK_BEGIN)
			SubSlotNumber = EQ::invbag::SHARED_BANK_BAGS_BEGIN + (slot_id_in - EQ::invslot::SHARED_BANK_BEGIN) * EQ::invbag::SLOT_COUNT;
		else
			SubSlotNumber = slot_id_in; // not sure if this is the best way to handle this..leaving for now

		if (SubSlotNumber != EQ::invbag::SLOT_INVALID) {
			std::vector<std::pair<int, EQ::ItemInstance*>> subitems;
			for (uint32 index = EQ::invbag::SLOT_BEGIN; index <= EQ::invbag::SLOT_END; ++index) {
				EQ::ItemInstance* sub = inst->GetItem(index);
				if (sub != nullptr)
					subitems.emplace_back(index, sub);
			}

			buffer.WriteUInt32(subitems.size());

			// This must be guaranteed to have subitem_count members, where the index is the correct index. The client doesn't loop through all slots here
			for (const auto& [index, subitem] : subitems) {
				buffer.WriteUInt32(index);
				SerializeItem(buffer, subitem, SubSlotNumber, depth + 1, packet_type);
			}
		} else
			buffer.WriteUInt32(0); // no subitems, client needs to know that

		//bool bCollected;
		buffer.WriteInt8(0); //unsupported atm
		//u64 DontKnow;
		buffer.WriteInt64(0); //unsupported atm
		//s32 Luck;
		buffer.WriteInt32(0); //unsupported atm
	}

	static void ServerToTOBConvertLinks(std::string& message_out, const std::string& message_in)
	{
		if (message_in.find('\x12') == std::string::npos) {
			message_out = message_in;
			return;
		}

		std::vector<std::string> segments = Strings::Split(message_in, '\x12');
		for (size_t segment_iter = 0; segment_iter < segments.size(); ++segment_iter) {
			if (segment_iter & 1) {
				auto etag = std::stoi(segments[segment_iter].substr(0, 1));

				switch (etag) {
				case 0: {
					size_t index = 1;
					std::string item_id = segments[segment_iter].substr(index, 5);
					index += 5;

					std::string aug1 = segments[segment_iter].substr(index, 5);
					index += 5;

					std::string aug2 = segments[segment_iter].substr(index, 5);
					index += 5;

					std::string aug3 = segments[segment_iter].substr(index, 5);
					index += 5;

					std::string aug4 = segments[segment_iter].substr(index, 5);
					index += 5;

					std::string aug5 = segments[segment_iter].substr(index, 5);
					index += 5;

					std::string aug6 = segments[segment_iter].substr(index, 5);
					index += 5;

					std::string is_evolving = segments[segment_iter].substr(index, 1);
					index += 1;

					std::string evolutionGroup = segments[segment_iter].substr(index, 4);
					index += 4;

					std::string evolutionLevel = segments[segment_iter].substr(index, 2);
					index += 2;

					std::string ornamentationIconID = segments[segment_iter].substr(index, 5);
					index += 5;

					std::string itemHash = segments[segment_iter].substr(index, 8);
					index += 8;

					std::string text = segments[segment_iter].substr(index);

					message_out.push_back('\x12');
					message_out.push_back('0'); //etag item
					message_out.append(item_id);
					message_out.append(aug1);
					message_out.append("00000");
					message_out.append(aug2);
					message_out.append("00000");
					message_out.append(aug3);
					message_out.append("00000");
					message_out.append(aug4);
					message_out.append("00000");
					message_out.append(aug5);
					message_out.append("00000");
					message_out.append(aug6);
					message_out.append("00000");
					message_out.append(is_evolving);
					message_out.append(evolutionGroup);
					message_out.append(evolutionLevel);
					message_out.append(ornamentationIconID);
					message_out.append("00000");
					message_out.append(itemHash);
					message_out.append(text);
					message_out.push_back('\x12');

					break;
				}
				default:
					//unsupported etag right now; just pass it as is
					message_out.push_back('\x12');
					message_out.append(segments[segment_iter]);
					message_out.push_back('\x12');
					break;
				}
			} else {
				message_out.append(segments[segment_iter]);
			}
		}
	}

	static void TOBToServerConvertLinks(std::string& message_out, const std::string& message_in) {
		message_out = message_in;
	}

	static inline uint32 ServerToTOBSpawnAppearanceType(uint32 server_type) {
		switch (server_type)
		{
		case AppearanceType::WhoLevel:
			return structs::TOBAppearance::WhoLevel;
		case AppearanceType::MaxHealth:
			return structs::TOBAppearance::MaxHealth;
		case AppearanceType::Invisibility:
			return structs::TOBAppearance::Invisibility;
		case AppearanceType::PVP:
			return structs::TOBAppearance::PVP;
		case AppearanceType::Light:
			return structs::TOBAppearance::Light;
		case AppearanceType::Animation:
			return structs::TOBAppearance::Animation;
		case AppearanceType::Sneak:
			return structs::TOBAppearance::Sneak;
		case AppearanceType::SpawnID:
			return structs::TOBAppearance::SpawnID;
		case AppearanceType::Health:
			return structs::TOBAppearance::Health;
		case AppearanceType::Linkdead:
			return structs::TOBAppearance::Linkdead;
		case AppearanceType::FlyMode:
			return structs::TOBAppearance::FlyMode;
		case AppearanceType::GM:
			return structs::TOBAppearance::GM;
		case AppearanceType::Anonymous:
			return structs::TOBAppearance::Anonymous;
		case AppearanceType::GuildID:
			return structs::TOBAppearance::GuildID;
		case AppearanceType::AFK:
			return structs::TOBAppearance::AFK;
		case AppearanceType::Pet:
			return structs::TOBAppearance::Pet;
		case AppearanceType::Summoned:
			return structs::TOBAppearance::Summoned;
		case AppearanceType::SetType:
			return structs::TOBAppearance::NPCName;
		case AppearanceType::CancelSneakHide:
			return structs::TOBAppearance::CancelSneakHide;
		case AppearanceType::AreaHealthRegen:
			return structs::TOBAppearance::AreaHealthRegen;
		case AppearanceType::AreaManaRegen:
			return structs::TOBAppearance::AreaManaRegen;
		case AppearanceType::AreaEnduranceRegen:
			return structs::TOBAppearance::AreaEnduranceRegen;
		case AppearanceType::FreezeBeneficialBuffs:
			return structs::TOBAppearance::FreezeBeneficialBuffs;
		case AppearanceType::NPCTintIndex:
			return structs::TOBAppearance::NPCTintIndex;
		case AppearanceType::ShowHelm:
			return structs::TOBAppearance::ShowHelm;
		case AppearanceType::DamageState:
			return structs::TOBAppearance::DamageState;
		case AppearanceType::TextureType:
			return structs::TOBAppearance::TextureType;
		case AppearanceType::GuildShow:
			return structs::TOBAppearance::GuildShow;
		case AppearanceType::OfflineMode:
			return structs::TOBAppearance::OfflineMode;
		default:
			return structs::TOBAppearance::None;
		}
	}

	static inline uint32 TOBToServerSpawnAppearanceType(uint32 tob_type) {
		switch (tob_type)
		{
		case structs::TOBAppearance::WhoLevel:
			return AppearanceType::WhoLevel;
		case structs::TOBAppearance::MaxHealth:
			return AppearanceType::MaxHealth;
		case structs::TOBAppearance::Invisibility:
			return AppearanceType::Invisibility;
		case structs::TOBAppearance::PVP:
			return AppearanceType::PVP;
		case structs::TOBAppearance::Light:
			return AppearanceType::Light;
		case structs::TOBAppearance::Animation:
			return AppearanceType::Animation;
		case structs::TOBAppearance::Sneak:
			return AppearanceType::Sneak;
		case structs::TOBAppearance::SpawnID:
			return AppearanceType::SpawnID;
		case structs::TOBAppearance::Health:
			return AppearanceType::Health;
		case structs::TOBAppearance::Linkdead:
			return AppearanceType::Linkdead;
		case structs::TOBAppearance::FlyMode:
			return AppearanceType::FlyMode;
		case structs::TOBAppearance::GM:
			return AppearanceType::GM;
		case structs::TOBAppearance::Anonymous:
			return AppearanceType::Anonymous;
		case structs::TOBAppearance::GuildID:
			return AppearanceType::GuildID;
		case structs::TOBAppearance::AFK:
			return AppearanceType::AFK;
		case structs::TOBAppearance::Pet:
			return AppearanceType::Pet;
		case structs::TOBAppearance::Summoned:
			return AppearanceType::Summoned;
		case structs::TOBAppearance::SetType:
			return AppearanceType::NPCName;
		case structs::TOBAppearance::CancelSneakHide:
			return AppearanceType::CancelSneakHide;
		case structs::TOBAppearance::AreaHealthRegen:
			return AppearanceType::AreaHealthRegen;
		case structs::TOBAppearance::AreaManaRegen:
			return AppearanceType::AreaManaRegen;
		case structs::TOBAppearance::AreaEnduranceRegen:
			return AppearanceType::AreaEnduranceRegen;
		case structs::TOBAppearance::FreezeBeneficialBuffs:
			return AppearanceType::FreezeBeneficialBuffs;
		case structs::TOBAppearance::NPCTintIndex:
			return AppearanceType::NPCTintIndex;
		case structs::TOBAppearance::ShowHelm:
			return AppearanceType::ShowHelm;
		case structs::TOBAppearance::DamageState:
			return AppearanceType::DamageState;
		case structs::TOBAppearance::TextureType:
			return AppearanceType::TextureType;
		case structs::TOBAppearance::GuildShow:
			return AppearanceType::GuildShow;
		case structs::TOBAppearance::OfflineMode:
			return AppearanceType::OfflineMode;
		default:
			return AppearanceType::Die;
		}
	}

	static inline structs::InventorySlot_Struct ServerToTOBSlot(uint32 server_slot)
	{
		structs::InventorySlot_Struct TOBSlot;
		TOBSlot.Type = invtype::TYPE_INVALID;
		TOBSlot.Slot = invslot::SLOT_INVALID;
		TOBSlot.SubIndex = invbag::SLOT_INVALID;
		TOBSlot.AugIndex = invaug::SOCKET_INVALID;

		uint32 TempSlot = EQ::invslot::SLOT_INVALID;

		if (server_slot < EQ::invtype::POSSESSIONS_SIZE) {
			TOBSlot.Type = invtype::typePossessions;

			if (server_slot == EQ::invslot::slotCursor) {
				TOBSlot.Slot = invslot::slotCursor;
			}
			else 
			{
				TOBSlot.Slot = server_slot;
			}
		}

		else if (server_slot <= EQ::invbag::CURSOR_BAG_END && server_slot >= EQ::invbag::GENERAL_BAGS_BEGIN) {
			TempSlot = server_slot - EQ::invbag::GENERAL_BAGS_BEGIN;

			TOBSlot.Type = invtype::typePossessions;
			TOBSlot.Slot = invslot::GENERAL_BEGIN + (TempSlot / EQ::invbag::SLOT_COUNT);
			TOBSlot.SubIndex = TempSlot - ((TOBSlot.Slot - invslot::GENERAL_BEGIN) * EQ::invbag::SLOT_COUNT);
		}

		else if (server_slot <= EQ::invslot::TRIBUTE_END && server_slot >= EQ::invslot::TRIBUTE_BEGIN) {
			TOBSlot.Type = invtype::typeTribute;
			TOBSlot.Slot = server_slot - EQ::invslot::TRIBUTE_BEGIN;
		}

		else if (server_slot <= EQ::invslot::GUILD_TRIBUTE_END && server_slot >= EQ::invslot::GUILD_TRIBUTE_BEGIN) {
			TOBSlot.Type = invtype::typeGuildTribute;
			TOBSlot.Slot = server_slot - EQ::invslot::GUILD_TRIBUTE_BEGIN;
		}

		else if (server_slot == EQ::invslot::SLOT_TRADESKILL_EXPERIMENT_COMBINE) {
			TOBSlot.Type = invtype::typeWorld;
		}

		else if (server_slot <= EQ::invslot::BANK_END && server_slot >= EQ::invslot::BANK_BEGIN) {
			TOBSlot.Type = invtype::typeBank;
			TOBSlot.Slot = server_slot - EQ::invslot::BANK_BEGIN;
		}

		else if (server_slot <= EQ::invbag::BANK_BAGS_END && server_slot >= EQ::invbag::BANK_BAGS_BEGIN) {
			TempSlot = server_slot - EQ::invbag::BANK_BAGS_BEGIN;

			TOBSlot.Type = invtype::typeBank;
			TOBSlot.Slot = TempSlot / EQ::invbag::SLOT_COUNT;
			TOBSlot.SubIndex = TempSlot - (TOBSlot.Slot * EQ::invbag::SLOT_COUNT);
		}

		else if (server_slot <= EQ::invslot::SHARED_BANK_END && server_slot >= EQ::invslot::SHARED_BANK_BEGIN) {
			TOBSlot.Type = invtype::typeSharedBank;
			TOBSlot.Slot = server_slot - EQ::invslot::SHARED_BANK_BEGIN;
		}

		else if (server_slot <= EQ::invbag::SHARED_BANK_BAGS_END && server_slot >= EQ::invbag::SHARED_BANK_BAGS_BEGIN) {
			TempSlot = server_slot - EQ::invbag::SHARED_BANK_BAGS_BEGIN;

			TOBSlot.Type = invtype::typeSharedBank;
			TOBSlot.Slot = TempSlot / EQ::invbag::SLOT_COUNT;
			TOBSlot.SubIndex = TempSlot - (TOBSlot.Slot * EQ::invbag::SLOT_COUNT);
		}

		else if (server_slot <= EQ::invslot::TRADE_END && server_slot >= EQ::invslot::TRADE_BEGIN) {
			TOBSlot.Type = invtype::typeTrade;
			TOBSlot.Slot = server_slot - EQ::invslot::TRADE_BEGIN;
		}

		else if (server_slot <= EQ::invbag::TRADE_BAGS_END && server_slot >= EQ::invbag::TRADE_BAGS_BEGIN) {
			TempSlot = server_slot - EQ::invbag::TRADE_BAGS_BEGIN;

			TOBSlot.Type = invtype::typeTrade;
			TOBSlot.Slot = TempSlot / EQ::invbag::SLOT_COUNT;
			TOBSlot.SubIndex = TempSlot - (TOBSlot.Slot * EQ::invbag::SLOT_COUNT);
		}

		else if (server_slot <= EQ::invslot::WORLD_END && server_slot >= EQ::invslot::WORLD_BEGIN) {
			TOBSlot.Type = invtype::typeWorld;
			TOBSlot.Slot = server_slot - EQ::invslot::WORLD_BEGIN;
		}

		Log(Logs::Detail, Logs::Netcode, fmt::format("Convert Server Slot {} to TOB Slot [{}, {}, {}, {}]",
			server_slot, TOBSlot.Type, TOBSlot.Slot, TOBSlot.SubIndex, TOBSlot.AugIndex).c_str());

		return TOBSlot;
	}

	static inline structs::InventorySlot_Struct ServerToTOBCorpseSlot(uint32 server_corpse_slot)
	{
		structs::InventorySlot_Struct TOBSlot;
		TOBSlot.Type = invtype::TYPE_INVALID;
		TOBSlot.Slot = ServerToTOBCorpseMainSlot(server_corpse_slot);
		TOBSlot.SubIndex = invbag::SLOT_INVALID;
		TOBSlot.AugIndex = invaug::SOCKET_INVALID;

		if (TOBSlot.Slot != invslot::SLOT_INVALID)
			TOBSlot.Type = invtype::typeCorpse;

		Log(Logs::Detail, Logs::Netcode, fmt::format("Convert Server Corpse Slot {} to TOB Corpse Slot [{}, {}, {}, {}]",
			server_corpse_slot, TOBSlot.Type, TOBSlot.Slot, TOBSlot.SubIndex, TOBSlot.AugIndex).c_str());

		return TOBSlot;
	}
	
	static inline uint32 ServerToTOBCorpseMainSlot(uint32 server_corpse_slot)
	{
		uint32 TOBSlot = invslot::SLOT_INVALID;

		if (server_corpse_slot <= EQ::invslot::CORPSE_END && server_corpse_slot >= EQ::invslot::CORPSE_BEGIN) {
			TOBSlot = server_corpse_slot;
		}

		LogNetcode("Convert Server Corpse Slot [{}] to TOB Corpse Main Slot [{}]", server_corpse_slot, TOBSlot);

		return TOBSlot;
	}

	static inline structs::TypelessInventorySlot_Struct ServerToTOBTypelessSlot(uint32 server_slot, int16 server_type)
	{
		structs::TypelessInventorySlot_Struct TOBSlot;
		TOBSlot.Slot = invslot::SLOT_INVALID;
		TOBSlot.SubIndex = invbag::SLOT_INVALID;
		TOBSlot.AugIndex = invaug::SOCKET_INVALID;

		uint32 TempSlot = EQ::invslot::SLOT_INVALID;

		if (server_type == EQ::invtype::typePossessions) {
			if (server_slot < EQ::invtype::POSSESSIONS_SIZE) {
				TOBSlot.Slot = server_slot;
			}

			else if (server_slot <= EQ::invbag::CURSOR_BAG_END && server_slot >= EQ::invbag::GENERAL_BAGS_BEGIN) {
				TempSlot = server_slot - EQ::invbag::GENERAL_BAGS_BEGIN;

				TOBSlot.Slot = invslot::GENERAL_BEGIN + (TempSlot / EQ::invbag::SLOT_COUNT);
				TOBSlot.SubIndex = TempSlot - ((TOBSlot.Slot - invslot::GENERAL_BEGIN) * EQ::invbag::SLOT_COUNT);
			}
		}

		Log(Logs::Detail, Logs::Netcode, fmt::format("Convert Server Slot {} to TOB Typeless Slot [{}, {}, {}] (implied type: {})",
			server_slot, TOBSlot.Slot, TOBSlot.SubIndex, TOBSlot.AugIndex, server_type).c_str());

		return TOBSlot;
	}

	static inline uint32 TOBToServerSlot(structs::InventorySlot_Struct tob_slot)
	{
		if (tob_slot.AugIndex < invaug::SOCKET_INVALID || tob_slot.AugIndex >= invaug::SOCKET_COUNT) {
			Log(Logs::Detail, Logs::Netcode, fmt::format("Convert TOB Slot [{}, {}, {}, {}] to Server Slot {}",
				tob_slot.Type, tob_slot.Slot, tob_slot.SubIndex, tob_slot.AugIndex, EQ::invslot::SLOT_INVALID).c_str());

			return EQ::invslot::SLOT_INVALID;
		}

		uint32 server_slot = EQ::invslot::SLOT_INVALID;
		uint32 temp_slot = invslot::SLOT_INVALID;

		switch (tob_slot.Type) {
		case invtype::typePossessions: {
			if (tob_slot.Slot >= invslot::POSSESSIONS_BEGIN && tob_slot.Slot <= invslot::POSSESSIONS_END) {
				if (tob_slot.SubIndex == invbag::SLOT_INVALID) {
					if (tob_slot.Slot == invslot::slotCursor) {
						server_slot = EQ::invslot::slotCursor;
					} 
					else if(tob_slot.Slot == invslot::slotGeneral11 || tob_slot.Slot == invslot::slotGeneral12)
					{
						return EQ::invslot::SLOT_INVALID;
					}
					else {
						server_slot = tob_slot.Slot;
					}
				}

				else if (tob_slot.SubIndex >= invbag::SLOT_BEGIN && tob_slot.SubIndex <= invbag::SLOT_END) {
					if (tob_slot.Slot < invslot::GENERAL_BEGIN)
						return EQ::invslot::SLOT_INVALID;

					temp_slot = (tob_slot.Slot - invslot::GENERAL_BEGIN) * invbag::SLOT_COUNT;
					server_slot = EQ::invbag::GENERAL_BAGS_BEGIN + temp_slot + tob_slot.SubIndex;
				}
			}

			break;
		}
		case invtype::typeBank: {
			if (tob_slot.Slot >= invslot::SLOT_BEGIN && tob_slot.Slot < invtype::BANK_SIZE) {
				if (tob_slot.SubIndex == invbag::SLOT_INVALID) {
					server_slot = EQ::invslot::BANK_BEGIN + tob_slot.Slot;
				}

				else if (tob_slot.SubIndex >= invbag::SLOT_BEGIN && tob_slot.SubIndex <= invbag::SLOT_END) {
					temp_slot = tob_slot.Slot * invbag::SLOT_COUNT;
					server_slot = EQ::invbag::BANK_BAGS_BEGIN + temp_slot + tob_slot.SubIndex;
				}
			}

			break;
		}
		case invtype::typeSharedBank: {
			if (tob_slot.Slot >= invslot::SLOT_BEGIN && tob_slot.Slot < invtype::SHARED_BANK_SIZE) {
				if (tob_slot.SubIndex == invbag::SLOT_INVALID) {
					server_slot = EQ::invslot::SHARED_BANK_BEGIN + tob_slot.Slot;
				}

				else if (tob_slot.SubIndex >= invbag::SLOT_BEGIN && tob_slot.SubIndex <= invbag::SLOT_END) {
					temp_slot = tob_slot.Slot * invbag::SLOT_COUNT;
					server_slot = EQ::invbag::SHARED_BANK_BAGS_BEGIN + temp_slot + tob_slot.SubIndex;
				}
			}

			break;
		}
		case invtype::typeTrade: {
			if (tob_slot.Slot >= invslot::SLOT_BEGIN && tob_slot.Slot < invtype::TRADE_SIZE) {
				if (tob_slot.SubIndex == invbag::SLOT_INVALID) {
					server_slot = EQ::invslot::TRADE_BEGIN + tob_slot.Slot;
				}

				else if (tob_slot.SubIndex >= invbag::SLOT_BEGIN && tob_slot.SubIndex <= invbag::SLOT_END) {
					temp_slot = tob_slot.Slot * invbag::SLOT_COUNT;
					server_slot = EQ::invbag::TRADE_BAGS_BEGIN + temp_slot + tob_slot.SubIndex;
				}
			}

			break;
		}
		case invtype::typeWorld: {
			if (tob_slot.Slot >= invslot::SLOT_BEGIN && tob_slot.Slot < invtype::WORLD_SIZE) {
				server_slot = EQ::invslot::WORLD_BEGIN + tob_slot.Slot;
			}

			else if (tob_slot.Slot == invslot::SLOT_INVALID) {
				server_slot = EQ::invslot::SLOT_TRADESKILL_EXPERIMENT_COMBINE;
			}

			break;
		}
		case invtype::typeLimbo: {
			if (tob_slot.Slot >= invslot::SLOT_BEGIN && tob_slot.Slot < invtype::LIMBO_SIZE) {
				server_slot = EQ::invslot::slotCursor;
			}

			break;
		}
		case invtype::typeTribute: {
			if (tob_slot.Slot >= invslot::SLOT_BEGIN && tob_slot.Slot < invtype::TRIBUTE_SIZE) {
				server_slot = EQ::invslot::TRIBUTE_BEGIN + tob_slot.Slot;
			}

			break;
		}
		case invtype::typeGuildTribute: {
			if (tob_slot.Slot >= invslot::SLOT_BEGIN && tob_slot.Slot < invtype::GUILD_TRIBUTE_SIZE) {
				server_slot = EQ::invslot::GUILD_TRIBUTE_BEGIN + tob_slot.Slot;
			}

			break;
		}
		case invtype::typeCorpse: {
			if (tob_slot.Slot >= invslot::CORPSE_BEGIN && tob_slot.Slot <= invslot::CORPSE_END) {
				server_slot = tob_slot.Slot;
			}

			break;
		}
		default: {

			break;
		}
		}

		Log(Logs::Detail, Logs::Netcode, fmt::format("Convert TOB Slot [{}, {}, {}, {}] to Server Slot {}",
			tob_slot.Type, tob_slot.Slot, tob_slot.SubIndex, tob_slot.AugIndex, server_slot).c_str());

		return server_slot;
	}

	static inline uint32 TOBToServerCorpseSlot(structs::InventorySlot_Struct tob_corpse_slot)
	{
		uint32 ServerSlot = EQ::invslot::SLOT_INVALID;

		if (tob_corpse_slot.Type != invtype::typeCorpse || tob_corpse_slot.SubIndex != invbag::SLOT_INVALID || tob_corpse_slot.AugIndex != invaug::SOCKET_INVALID) {
			ServerSlot = EQ::invslot::SLOT_INVALID;
		}

		else {
			ServerSlot = TOBToServerCorpseMainSlot(tob_corpse_slot.Slot);
		}

		Log(Logs::Detail, Logs::Netcode, fmt::format("Convert TOB Slot [{}, {}, {}, {}] to Server Slot {}",
			tob_corpse_slot.Type, tob_corpse_slot.Slot, tob_corpse_slot.SubIndex, tob_corpse_slot.AugIndex, ServerSlot).c_str());

		return ServerSlot;
	}

	static inline uint32 TOBToServerCorpseMainSlot(uint32 tob_corpse_slot)
	{
		uint32 ServerSlot = EQ::invslot::SLOT_INVALID;

		if (tob_corpse_slot <= invslot::CORPSE_END && tob_corpse_slot >= invslot::CORPSE_BEGIN) {
			ServerSlot = tob_corpse_slot;
		}

		LogNetcode("Convert TOB Corpse Main Slot [{}] to Server Corpse Slot [{}]", tob_corpse_slot, ServerSlot);

		return ServerSlot;
	}

	static inline uint32 TOBToServerTypelessSlot(structs::TypelessInventorySlot_Struct tob_slot, int16 tob_type)
	{
		if (tob_slot.AugIndex < invaug::SOCKET_INVALID || tob_slot.AugIndex >= invaug::SOCKET_COUNT) {
			Log(Logs::Detail, Logs::Netcode, fmt::format("Convert TOB Typeless Slot [{}, {}, {}] (implied type: {}) to Server Slot {}",
				tob_slot.Slot, tob_slot.SubIndex, tob_slot.AugIndex, tob_type, EQ::invslot::SLOT_INVALID).c_str());

			return EQ::invslot::SLOT_INVALID;
		}

		uint32 ServerSlot = EQ::invslot::SLOT_INVALID;
		uint32 TempSlot = invslot::SLOT_INVALID;

		switch (tob_type) {
		case invtype::typePossessions: {
			if (tob_slot.Slot >= invslot::POSSESSIONS_BEGIN && tob_slot.Slot <= invslot::POSSESSIONS_END) {
				if (tob_slot.SubIndex == invbag::SLOT_INVALID) {
					ServerSlot = tob_slot.Slot;
				}

				else if (tob_slot.SubIndex >= invbag::SLOT_BEGIN && tob_slot.SubIndex <= invbag::SLOT_END) {
					if (tob_slot.Slot < invslot::GENERAL_BEGIN)
						return EQ::invslot::SLOT_INVALID;

					TempSlot = (tob_slot.Slot - invslot::GENERAL_BEGIN) * invbag::SLOT_COUNT;
					ServerSlot = EQ::invbag::GENERAL_BAGS_BEGIN + TempSlot + tob_slot.SubIndex;
				}
			}

			break;
		}
		case invtype::typeBank: {
			if (tob_slot.Slot >= invslot::SLOT_BEGIN && tob_slot.Slot < invtype::BANK_SIZE) {
				if (tob_slot.SubIndex == invbag::SLOT_INVALID) {
					ServerSlot = EQ::invslot::BANK_BEGIN + tob_slot.Slot;
				}

				else if (tob_slot.SubIndex >= invbag::SLOT_BEGIN && tob_slot.SubIndex <= invbag::SLOT_END) {
					TempSlot = tob_slot.Slot * invbag::SLOT_COUNT;
					ServerSlot = EQ::invbag::BANK_BAGS_BEGIN + TempSlot + tob_slot.SubIndex;
				}
			}

			break;
		}
		case invtype::typeSharedBank: {
			if (tob_slot.Slot >= invslot::SLOT_BEGIN && tob_slot.Slot < invtype::SHARED_BANK_SIZE) {
				if (tob_slot.SubIndex == invbag::SLOT_INVALID) {
					ServerSlot = EQ::invslot::SHARED_BANK_BEGIN + tob_slot.Slot;
				}

				else if (tob_slot.SubIndex >= invbag::SLOT_BEGIN && tob_slot.SubIndex <= invbag::SLOT_END) {
					TempSlot = tob_slot.Slot * invbag::SLOT_COUNT;
					ServerSlot = EQ::invbag::SHARED_BANK_BAGS_BEGIN + TempSlot + tob_slot.SubIndex;
				}
			}

			break;
		}
		case invtype::typeTrade: {
			if (tob_slot.Slot >= invslot::SLOT_BEGIN && tob_slot.Slot < invtype::TRADE_SIZE) {
				if (tob_slot.SubIndex == invbag::SLOT_INVALID) {
					ServerSlot = EQ::invslot::TRADE_BEGIN + tob_slot.Slot;
				}

				else if (tob_slot.SubIndex >= invbag::SLOT_BEGIN && tob_slot.SubIndex <= invbag::SLOT_END) {
					TempSlot = tob_slot.Slot * invbag::SLOT_COUNT;
					ServerSlot = EQ::invbag::TRADE_BAGS_BEGIN + TempSlot + tob_slot.SubIndex;
				}
			}

			break;
		}
		case invtype::typeWorld: {
			if (tob_slot.Slot >= invslot::SLOT_BEGIN && tob_slot.Slot < invtype::WORLD_SIZE) {
				ServerSlot = EQ::invslot::WORLD_BEGIN + tob_slot.Slot;
			}

			else if (tob_slot.Slot == invslot::SLOT_INVALID) {
				ServerSlot = EQ::invslot::SLOT_TRADESKILL_EXPERIMENT_COMBINE;
			}

			break;
		}
		case invtype::typeLimbo: {
			if (tob_slot.Slot >= invslot::SLOT_BEGIN && tob_slot.Slot < invtype::LIMBO_SIZE) {
				ServerSlot = EQ::invslot::slotCursor;
			}

			break;
		}
		case invtype::typeTribute: {
			if (tob_slot.Slot >= invslot::SLOT_BEGIN && tob_slot.Slot < invtype::TRIBUTE_SIZE) {
				ServerSlot = EQ::invslot::TRIBUTE_BEGIN + tob_slot.Slot;
			}

			break;
		}
		case invtype::typeGuildTribute: {
			if (tob_slot.Slot >= invslot::SLOT_BEGIN && tob_slot.Slot < invtype::GUILD_TRIBUTE_SIZE) {
				ServerSlot = EQ::invslot::GUILD_TRIBUTE_BEGIN + tob_slot.Slot;
			}

			break;
		}
		case invtype::typeCorpse: {
			if (tob_slot.Slot >= invslot::CORPSE_BEGIN && tob_slot.Slot <= invslot::CORPSE_END) {
				ServerSlot = tob_slot.Slot;
			}

			break;
		}
		default: {

			break;
		}
		}

		Log(Logs::Detail, Logs::Netcode, fmt::format("Convert TOB Typeless Slot [{}, {}, {}] (implied type: {}) to Server Slot {}",
			tob_slot.Slot, tob_slot.SubIndex, tob_slot.AugIndex, tob_type, ServerSlot).c_str());

		return ServerSlot;
	}

	static inline structs::InventorySlot_Struct TOBCastingInventorySlotToInventorySlot(structs::CastSpellInventorySlot_Struct tob_slot) {
		structs::InventorySlot_Struct ret;
		ret.Type = tob_slot.type;
		ret.Slot = tob_slot.slot;
		ret.SubIndex = tob_slot.sub_index;
		ret.AugIndex = tob_slot.aug_index;
		return ret;
	}

	static inline structs::CastSpellInventorySlot_Struct TOBInventorySlotToCastingInventorySlot(structs::InventorySlot_Struct tob_slot) {
		structs::CastSpellInventorySlot_Struct ret;
		ret.type = tob_slot.Type;
		ret.slot = tob_slot.Slot;
		ret.sub_index = tob_slot.SubIndex;
		ret.aug_index = tob_slot.AugIndex;
		return ret;
	}

	static item::ItemPacketType ServerToTOBItemPacketType(ItemPacketType server_type) {
		switch (server_type) {
		case ItemPacketType::ItemPacketMerchant:
			return item::ItemPacketType::ItemPacketMerchant;
		case ItemPacketType::ItemPacketTradeView:
			return item::ItemPacketType::ItemPacketTradeView;
		case ItemPacketType::ItemPacketLoot:
			return item::ItemPacketType::ItemPacketLoot;
		case ItemPacketType::ItemPacketTrade:
			return item::ItemPacketType::ItemPacketTrade;
		case ItemPacketType::ItemPacketCharInventory:
			return item::ItemPacketType::ItemPacketCharInventory;
		case ItemPacketType::ItemPacketLimbo:
			return item::ItemPacketType::ItemPacketLimbo;
		case ItemPacketType::ItemPacketWorldContainer:
			return item::ItemPacketType::ItemPacketWorldContainer;
		case ItemPacketType::ItemPacketTributeItem:
			return item::ItemPacketType::ItemPacketTributeItem;
		case ItemPacketType::ItemPacketGuildTribute:
			return item::ItemPacketType::ItemPacketGuildTribute;
		case ItemPacketType::ItemPacketCharmUpdate:
			return item::ItemPacketType::ItemPacketCharmUpdate;
		default:
			return item::ItemPacketType::ItemPacketInvalid;
		}
	}

	//This stuff isn't right because they for one removed potion belt
	//This will probably be enough to get casting working for now though
	static inline spells::CastingSlot ServerToTOBCastingSlot(EQ::spells::CastingSlot slot) {
		switch (slot) {
		case EQ::spells::CastingSlot::Gem1:
			return spells::CastingSlot::Gem1;
		case EQ::spells::CastingSlot::Gem2:
			return spells::CastingSlot::Gem2;
		case EQ::spells::CastingSlot::Gem3:
			return spells::CastingSlot::Gem3;
		case EQ::spells::CastingSlot::Gem4:
			return spells::CastingSlot::Gem4;
		case EQ::spells::CastingSlot::Gem5:
			return spells::CastingSlot::Gem5;
		case EQ::spells::CastingSlot::Gem6:
			return spells::CastingSlot::Gem6;
		case EQ::spells::CastingSlot::Gem7:
			return spells::CastingSlot::Gem7;
		case EQ::spells::CastingSlot::Gem8:
			return spells::CastingSlot::Gem8;
		case EQ::spells::CastingSlot::Gem9:
			return spells::CastingSlot::Gem9;
		case EQ::spells::CastingSlot::Gem10:
			return spells::CastingSlot::Gem10;
		case EQ::spells::CastingSlot::Gem11:
			return spells::CastingSlot::Gem11;
		case EQ::spells::CastingSlot::Gem12:
			return spells::CastingSlot::Gem12;
		case EQ::spells::CastingSlot::Item:
		case EQ::spells::CastingSlot::PotionBelt:
			return spells::CastingSlot::Item;
		case EQ::spells::CastingSlot::Discipline:
			return spells::CastingSlot::Discipline;
		case EQ::spells::CastingSlot::AltAbility:
			return spells::CastingSlot::AltAbility;
		default: // we shouldn't have any issues with other slots ... just return something
			return spells::CastingSlot::Discipline;
		}
	}

	static inline EQ::spells::CastingSlot TOBToServerCastingSlot(spells::CastingSlot slot) {
		switch (slot) {
		case spells::CastingSlot::Gem1:
			return EQ::spells::CastingSlot::Gem1;
		case spells::CastingSlot::Gem2:
			return EQ::spells::CastingSlot::Gem2;
		case spells::CastingSlot::Gem3:
			return EQ::spells::CastingSlot::Gem3;
		case spells::CastingSlot::Gem4:
			return EQ::spells::CastingSlot::Gem4;
		case spells::CastingSlot::Gem5:
			return EQ::spells::CastingSlot::Gem5;
		case spells::CastingSlot::Gem6:
			return EQ::spells::CastingSlot::Gem6;
		case spells::CastingSlot::Gem7:
			return EQ::spells::CastingSlot::Gem7;
		case spells::CastingSlot::Gem8:
			return EQ::spells::CastingSlot::Gem8;
		case spells::CastingSlot::Gem9:
			return EQ::spells::CastingSlot::Gem9;
		case spells::CastingSlot::Gem10:
			return EQ::spells::CastingSlot::Gem10;
		case spells::CastingSlot::Gem11:
			return EQ::spells::CastingSlot::Gem11;
		case spells::CastingSlot::Gem12:
			return EQ::spells::CastingSlot::Gem12;
		case spells::CastingSlot::Discipline:
			return EQ::spells::CastingSlot::Discipline;
		case spells::CastingSlot::Item:
			return EQ::spells::CastingSlot::Item;
		case spells::CastingSlot::AltAbility:
			return EQ::spells::CastingSlot::AltAbility;
		default: // we shouldn't have any issues with other slots ... just return something
			return EQ::spells::CastingSlot::Discipline;
		}
	}

	static inline int TOBToServerBuffSlot(int index)
	{
		// we're a disc
		if (index >= spells::LONG_BUFFS + spells::SHORT_BUFFS)
			return index - spells::LONG_BUFFS - spells::SHORT_BUFFS + EQ::spells::LONG_BUFFS +
			EQ::spells::SHORT_BUFFS;
		// we're a song
		if (index >= spells::LONG_BUFFS)
			return index - spells::LONG_BUFFS + EQ::spells::LONG_BUFFS;
		// we're a normal buff
		return index; // as long as we guard against bad slots server side, we should be fine
	}

struct TOBStringIDs
{
	static constexpr uint32_t DisarmedTrap = 1458; // You successfully disarmed the trap
};

uint32_t MessageComponent::ResolveID(uint32_t id) const
{
	switch (id) {
	case YOU_FLURRY:
	case BOW_DOUBLE_DAMAGE:
	case NO_INSTRUMENT_SKILL:
	case DISCIPLINE_CONLOST:
	case TGB_ON:
	case TGB_OFF:
	case DISCIPLINE_RDY:
	case SONG_NEEDS_DRUM:
	case SONG_NEEDS_WIND:
	case SONG_NEEDS_STRINGS:
	case SONG_NEEDS_BRASS:
	case YOU_CRIT_HEAL:
	case YOU_CRIT_BLAST:
	case SPELL_WORN_OFF:
	case PET_TAUNTING:
	case DISC_LEVEL_ERROR:
	case MALE_SLAYUNDEAD:
	case FEMALE_SLAYUNDEAD:
	case FINISHING_BLOW:
	case ASSASSINATES:
	case CRIPPLING_BLOW:
	case CRITICAL_HIT:
	case DEADLY_STRIKE:
	case OTHER_CRIT_HEAL:
	case OTHER_CRIT_BLAST:
	case NPC_RAMPAGE:
	case NPC_FLURRY:
	case DISCIPLINE_FEARLESS:
	case CORPSE_ITEM_LOST:
	case FATAL_BOW_SHOT:
	case CURRENT_SPELL_EFFECTS:
	case NOT_DELEGATED_MARKER:
	case STRIKETHROUGH_STRING:
	case AE_RAMPAGE:
	case DISC_LEVEL_USE_ERROR:
	case SPLIT_FAIL:
		// removed from the client
		return 0;
	case DISARMED_TRAP:
		return TOBStringIDs::DisarmedTrap;
	default:
		return Titanium::MessageComponent::ResolveID(id);
	}
}

void MessageComponent::ResolveArguments(uint32_t id, std::array<const char*, 9>& args) const
{
	switch (id) {
	case SPELL_FIZZLE:
	case MISS_NOTE:
	case SPELL_FIZZLE_OTHER:
	case MISSED_NOTE_OTHER:
		// take all arguments (spell link)
		break;
	default:
		Titanium::MessageComponent::ResolveArguments(id, args);
		break;
	}
}

std::unique_ptr<EQApplicationPacket> MessageComponent::Formatted(uint32_t color, uint32_t id,
	const FormattedArgs& args) const
{
	uint32_t string_id = ResolveID(id);
	if (string_id > 0) {
		std::array<const char*, 9> resolved_args = args;
		ResolveArguments(id, resolved_args);
		if (!resolved_args[0])
			return Simple(color, id);

		SerializeBuffer buffer(49);
		// 49 is the minimum size needed for this packet since each arg writes at least 4 bytes
		buffer.WriteUInt32(0);
		// This is a string written like the message arrays, but it seems to be discarded by the client
		buffer.WriteUInt8(0); // 0 is a zone packet, 1 is a world packet -- these are always sent from zone from here
		buffer.WriteUInt32(string_id);
		buffer.WriteUInt32(color);

		for (auto a : resolved_args) {
			if (a != nullptr) {
				std::string new_message;
				::TOB::ServerToTOBConvertLinks(new_message, a);
				buffer.WriteLengthString(new_message);
			} else
				buffer.WriteUInt32(0);
		}

		return std::make_unique<EQApplicationPacket>(OP_FormattedMessage, std::move(buffer));
	}

	return nullptr;
}

std::unique_ptr<EQApplicationPacket> MessageComponent::InterruptSpell(uint32_t message, uint32_t spawn_id,
	const char* spell_link) const
{
	auto outapp = std::make_unique<EQApplicationPacket>(OP_InterruptCast, sizeof(InterruptCast_Struct) + strlen(spell_link) + 1);
	auto ic = reinterpret_cast<InterruptCast_Struct*>(outapp->pBuffer);
	ic->messageid = ResolveID(message);
	ic->spawnid = spawn_id;
	strcpy(ic->message, spell_link);
	outapp->priority = 5;

	return outapp;
}

std::unique_ptr<EQApplicationPacket> MessageComponent::InterruptSpellOther(Mob* sender, uint32_t message, uint32_t spawn_id,
	const char* name,
	const char* spell_link) const
{
	auto outapp = std::make_unique<EQApplicationPacket>(OP_InterruptCast,
		sizeof(InterruptCast_Struct) + strlen(name) + strlen(spell_link) + 2);
	auto ic = reinterpret_cast<InterruptCast_Struct*>(outapp->pBuffer);
	ic->messageid = ResolveID(message);
	ic->spawnid = spawn_id;
	strcpy(ic->message, name);
	strcpy(&ic->message[strlen(name) + 1], spell_link);

	return outapp;
}

std::unique_ptr<EQApplicationPacket> BuffComponent::BuffDefinition(Mob* mob, const Buffs_Struct& buff, uint32_t slot, bool fade) const
{
	auto packet = std::make_unique<EQApplicationPacket>(OP_BuffDefinition, sizeof(structs::EQAffectPacket_Struct));
	auto affect = reinterpret_cast<structs::EQAffectPacket_Struct*>(packet->pBuffer);

	// base packet
	affect->entity_id = mob->GetID();
	affect->unknown004 = 0;
	affect->slot_id = ServerToPatchBuffSlot(slot);
	affect->buff_fade = fade ? 1 : 2; // 1 is remove, 2 is modify, 3 is add (only seen 1 and 2 sent)

	memset(&affect->affect, 0, sizeof(affect->affect));

	// affect slots
	for (int affect_slot = 0; affect_slot < 6; ++affect_slot) {
		// all of this is unknown, just what we've seen
		affect->affect.slots[affect_slot].slot = -1; // this is always -1
		affect->affect.slots[affect_slot].padding = 0; // this is never 0, but the values aren't clear
		affect->affect.slots[affect_slot].value = 0; // this is always 0
	}

	// affect info
	affect->affect.caster_id.Id = buff.casterid;
	affect->affect.caster_id.WorldId = RuleI(World, Id);
	affect->affect.caster_id.Reserved = 0;
	affect->affect.flags = 0;
	affect->affect.spell_id = buff.spellid;
	affect->affect.duration = buff.ticsremaining;
	affect->affect.initial_duration = buff.initialduration;
	affect->affect.hit_count = buff.hit_number;
	affect->affect.viral_timer = 0;
	affect->affect.modifier = static_cast<float>(buff.instrument_mod) / 10.f;
	affect->affect.y = static_cast<float>(buff.caston_y);
	affect->affect.x = static_cast<float>(buff.caston_x);
	affect->affect.z = static_cast<float>(buff.caston_z);
	affect->affect.type = 2;
	affect->affect.level = buff.casterlevel > 0 ? buff.casterlevel : mob->GetLevel();

	//no idea if these are right; eqlib doesn't seem to know either
	if (buff.dot_rune > 0)
		affect->affect.charges = buff.dot_rune;
	else if (buff.magic_rune > 0)
		affect->affect.charges = buff.magic_rune;
	else if (buff.melee_rune > 0)
		affect->affect.charges = buff.melee_rune;
	else if (buff.counters > 0)
		affect->affect.charges = buff.counters;

	affect->affect.activatable = 0;
	affect->affect.unknown1 = 0; //might be some timer, not sure though

	return packet;
}

std::unique_ptr<EQApplicationPacket> BuffComponent::RefreshBuffs(EmuOpcode opcode, Mob* mob, bool remove,
	bool buff_timers_suspended, const std::vector<uint32_t>& slots) const
{
	Buffs_Struct* buffs = mob->GetBuffs();

	// pre-calculate the buffer size to avoid too many grow calls
	size_t buffer_size = 13; // 13 bytes outside the list
	std::vector<uint32_t> send_slots;
	if (slots.empty()) {
		for (uint32_t slot = 0; slot < mob->GetMaxTotalSlots(); ++slot)
			if (buffs[slot].spellid > 1) {
				buffer_size += 17 + strlen(buffs[slot].caster_name); // 17 includes the null terminator
				send_slots.push_back(slot);
			}
	} else {
		for (uint32_t slot : slots)
			if (slot < mob->GetMaxTotalSlots() && buffs[slot].spellid > 1) {
				buffer_size += 17 + strlen(buffs[slot].caster_name);
				send_slots.push_back(slot);
			}
	}

	SerializeBuffer buffer(buffer_size);

	buffer.WriteUInt32(mob->GetID());
	buffer.WriteInt32(mob->GetRemainingTicTime());
	buffer.WriteUInt8(slots.empty() ? 1 : 0);			// 1 indicates all buffs on the mob (0 to add or remove a single buff)
	buffer.WriteUInt16(send_slots.size());

	for (uint32_t slot : send_slots) {
		buffer.WriteUInt32(ServerToPatchBuffSlot(slot)); // the server stores fewer buffs
		buffer.WriteInt32(remove ? -1 : buffs[slot].spellid);
		buffer.WriteUInt32(buffs[slot].ticsremaining);
		buffer.WriteUInt32(buffs[slot].hit_number);
		buffer.WriteString(buffs[slot].caster_name);
	}

	buffer.WriteUInt8(opcode == OP_RefreshPetBuffs ? 2 : 0);
	buffer.WriteUInt8(buff_timers_suspended ? 1 : 0); // bBuffTimersOnHold

	return std::make_unique<EQApplicationPacket>(opcode, std::move(buffer));
}

bool BuffComponent::NeedsWearMessage() const { return false; }

// 0 = self buff window, 1 = self target window, 2 = pet buff or target window, 4 = group, 5 = PC, 7 = NPC
void BuffComponent::SetRefreshType(std::unique_ptr<EQApplicationPacket>& packet, uint8_t refresh_type) const
{
	if (packet)
		packet->pBuffer[packet->size - 2] = refresh_type;
}

} /*TOB*/