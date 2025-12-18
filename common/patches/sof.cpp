/*	EQEMu: Everquest Server Emulator

	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "sof.h"
#include "sof_structs.h"

#include "common/crc32.h"
#include "common/eq_packet_structs.h"
#include "common/eq_stream_ident.h"
#include "common/eqemu_config.h"
#include "common/eqemu_logsys.h"
#include "common/item_instance.h"
#include "common/misc_functions.h"
#include "common/opcodemgr.h"
#include "common/path_manager.h"
#include "common/raid.h"
#include "common/rulesys.h"
#include "common/strings.h"

#include <iostream>
#include <sstream>


namespace SoF
{
	static const char *name = "SoF";
	static OpcodeManager *opcodes = nullptr;
	static Strategy struct_strategy;

	void SerializeItem(EQ::OutBuffer& ob, const EQ::ItemInstance *inst, int16 slot_id, uint8 depth);

	// server to client inventory location converters
	static inline uint32 ServerToSoFSlot(uint32 server_slot);
	static inline uint32 ServerToSoFCorpseSlot(uint32 server_corpse_slot);

	// client to server inventory location converters
	static inline uint32 SoFToServerSlot(uint32 sof_slot);
	static inline uint32 SoFToServerCorpseSlot(uint32 sof_corpse_slot);

	// server to client say link converter
	static inline void ServerToSoFSayLink(std::string &sof_saylink, const std::string &server_saylink);

	// client to server say link converter
	static inline void SoFToServerSayLink(std::string &server_saylink, const std::string &sof_saylink);

	static inline spells::CastingSlot ServerToSoFCastingSlot(EQ::spells::CastingSlot slot);
	static inline EQ::spells::CastingSlot SoFToServerCastingSlot(spells::CastingSlot slot, uint32 item_location);

	static inline int ServerToSoFBuffSlot(int index);
	static inline int SoFToServerBuffSlot(int index);

	void Register(EQStreamIdentifier &into)
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
#include "sof_ops.h"
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
		return EQ::versions::ClientVersion::SoF;
	}

#include "ss_define.h"

// ENCODE methods
	ENCODE(OP_Action)
	{
		ENCODE_LENGTH_EXACT(Action_Struct);
		SETUP_DIRECT_ENCODE(Action_Struct, structs::Action_Struct);

		OUT(target);
		OUT(source);
		OUT(level);
		OUT(instrument_mod);
		OUT(force);
		OUT(hit_heading);
		OUT(hit_pitch);
		OUT(type);
		//OUT(damage);
		OUT(spell);
		OUT(spell_level);
		OUT(effect_flag); // if this is 4, a buff icon is made

		FINISH_ENCODE();
	}

	ENCODE(OP_AdventureMerchantSell)
	{
		ENCODE_LENGTH_EXACT(Adventure_Sell_Struct);
		SETUP_DIRECT_ENCODE(Adventure_Sell_Struct, structs::Adventure_Sell_Struct);

		eq->unknown000 = 1;
		OUT(npcid);
		eq->slot = ServerToSoFSlot(emu->slot);
		OUT(charges);
		OUT(sell_price);

		FINISH_ENCODE();
	}

	ENCODE(OP_AltCurrencySell)
	{
		ENCODE_LENGTH_EXACT(AltCurrencySellItem_Struct);
		SETUP_DIRECT_ENCODE(AltCurrencySellItem_Struct, structs::AltCurrencySellItem_Struct);

		OUT(merchant_entity_id);
		eq->slot_id = ServerToSoFSlot(emu->slot_id);
		OUT(charges);
		OUT(cost);

		FINISH_ENCODE();
	}

	ENCODE(OP_ApplyPoison)
	{
		ENCODE_LENGTH_EXACT(ApplyPoison_Struct);
		SETUP_DIRECT_ENCODE(ApplyPoison_Struct, structs::ApplyPoison_Struct);

		eq->inventorySlot = ServerToSoFSlot(emu->inventorySlot);
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

	ENCODE(OP_BazaarSearch)
	{
		if (((*p)->size == sizeof(BazaarReturnDone_Struct)) || ((*p)->size == sizeof(BazaarWelcome_Struct))) {

			EQApplicationPacket *in = *p;
			*p = nullptr;
			dest->FastQueuePacket(&in, ack_req);
			return;
		}

		//consume the packet
		EQApplicationPacket *in = *p;
		*p = nullptr;

		//store away the emu struct
		unsigned char *__emu_buffer = in->pBuffer;
		BazaarSearchResults_Struct *emu = (BazaarSearchResults_Struct *)__emu_buffer;

		//determine and verify length
		int entrycount = in->size / sizeof(BazaarSearchResults_Struct);
		if (entrycount == 0 || (in->size % sizeof(BazaarSearchResults_Struct)) != 0) {
			Log(Logs::General, Logs::Netcode, "[STRUCTS] Wrong size on outbound %s: Got %d, expected multiple of %d",
				opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(BazaarSearchResults_Struct));
			delete in;
			return;
		}

		//make the EQ struct.
		in->size = sizeof(structs::BazaarSearchResults_Struct)*entrycount;
		in->pBuffer = new unsigned char[in->size];
		structs::BazaarSearchResults_Struct *eq = (structs::BazaarSearchResults_Struct *) in->pBuffer;

		//zero out the packet. We could avoid this memset by setting all fields (including unknowns)
		//in the loop.
		memset(in->pBuffer, 0, in->size);

		for (int i = 0; i<entrycount; i++, eq++, emu++) {
			eq->Beginning.Action = emu->Beginning.Action;
			eq->Beginning.Unknown001 = emu->Beginning.Unknown001;
			eq->Beginning.Unknown002 = emu->Beginning.Unknown002;
			eq->NumItems = emu->NumItems;
			eq->SerialNumber = emu->SerialNumber;
			eq->SellerID = emu->SellerID;
			eq->Cost = emu->Cost;
			eq->ItemStat = emu->ItemStat;
			strcpy(eq->ItemName, emu->ItemName);
		}

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_BecomeTrader)
	{
		ENCODE_LENGTH_EXACT(BecomeTrader_Struct);
		SETUP_DIRECT_ENCODE(BecomeTrader_Struct, structs::BecomeTrader_Struct);

		OUT(trader_id);
		OUT(action);

		FINISH_ENCODE();
	}

	ENCODE(OP_Buff)
	{
		ENCODE_LENGTH_EXACT(SpellBuffPacket_Struct);
		SETUP_DIRECT_ENCODE(SpellBuffPacket_Struct, structs::SpellBuffPacket_Struct);

		OUT(entityid);
		OUT(buff.effect_type);
		OUT(buff.level);
		OUT(buff.bard_modifier);
		OUT(buff.spellid);
		OUT(buff.duration);
		OUT(buff.counters);
		OUT(buff.player_id);
		eq->slotid = ServerToSoFBuffSlot(emu->slotid);
		OUT(bufffade);

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

	ENCODE(OP_ChannelMessage)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		ChannelMessage_Struct *emu = (ChannelMessage_Struct *)in->pBuffer;

		unsigned char *__emu_buffer = in->pBuffer;

		std::string old_message = emu->message;
		std::string new_message;
		ServerToSoFSayLink(new_message, old_message);

		in->size = sizeof(ChannelMessage_Struct) + new_message.length() + 1;

		in->pBuffer = new unsigned char[in->size];

		char *OutBuffer = (char *)in->pBuffer;

		memcpy(OutBuffer, __emu_buffer, sizeof(ChannelMessage_Struct));

		OutBuffer += sizeof(ChannelMessage_Struct);

		VARSTRUCT_ENCODE_STRING(OutBuffer, new_message.c_str());

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_CharInventory)
	{
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
			Log(Logs::General, Logs::Netcode, "[STRUCTS] Wrong size on outbound %s: Got %d, expected multiple of %d",
				opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(EQ::InternalSerializedItem_Struct));

			delete in;
			return;
		}

		EQ::InternalSerializedItem_Struct* eq = (EQ::InternalSerializedItem_Struct*)in->pBuffer;

		EQ::OutBuffer ob;
		EQ::OutBuffer::pos_type last_pos = ob.tellp();

		ob.write((const char*)&item_count, sizeof(uint32));

		for (int index = 0; index < item_count; ++index, ++eq) {
			SerializeItem(ob, (const EQ::ItemInstance*)eq->inst, eq->slot_id, 0);
			if (ob.tellp() == last_pos)
				LogNetcode("SoF::ENCODE(OP_CharInventory) Serialization failed on item slot [{}] during OP_CharInventory.  Item skipped", eq->slot_id);

			last_pos = ob.tellp();
		}

		in->size = ob.size();
		in->pBuffer = ob.detach();

		delete[] __emu_buffer;

		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_ClientUpdate)
	{
		ENCODE_LENGTH_EXACT(PlayerPositionUpdateServer_Struct);
		SETUP_DIRECT_ENCODE(PlayerPositionUpdateServer_Struct, structs::PlayerPositionUpdateServer_Struct);

		OUT(spawn_id);
		OUT(x_pos);
		OUT(delta_x);
		OUT(delta_y);
		OUT(z_pos);
		OUT(delta_heading);
		OUT(y_pos);
		OUT(delta_z);
		OUT(animation);
		OUT(heading);

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
		OUT(pvpcon);

		FINISH_ENCODE();
	}

	ENCODE(OP_Damage)
	{
		ENCODE_LENGTH_EXACT(CombatDamage_Struct);
		SETUP_DIRECT_ENCODE(CombatDamage_Struct, structs::CombatDamage_Struct);

		OUT(target);
		OUT(source);
		OUT(type);
		OUT(spellid);
		OUT(damage);
		OUT(force);
		OUT(hit_heading);
		OUT(hit_pitch);

		FINISH_ENCODE();
	}

	ENCODE(OP_DeleteCharge)
	{
		Log(Logs::Detail, Logs::Netcode, "SoF::ENCODE(OP_DeleteCharge)");

		ENCODE_FORWARD(OP_MoveItem);
	}

	ENCODE(OP_DeleteItem)
	{
		ENCODE_LENGTH_EXACT(DeleteItem_Struct);
		SETUP_DIRECT_ENCODE(DeleteItem_Struct, structs::DeleteItem_Struct);

		eq->from_slot = ServerToSoFSlot(emu->from_slot);
		eq->to_slot = ServerToSoFSlot(emu->to_slot);
		OUT(number_in_stack);

		FINISH_ENCODE();
	}

	ENCODE(OP_DeleteSpawn)
	{
		SETUP_DIRECT_ENCODE(DeleteSpawn_Struct, structs::DeleteSpawn_Struct);

		OUT(spawn_id);

		FINISH_ENCODE();
	}

	ENCODE(OP_DzChooseZone)
	{
		SETUP_VAR_ENCODE(DynamicZoneChooseZone_Struct);

		SerializeBuffer buf;
		buf.WriteUInt32(emu->client_id);
		buf.WriteUInt32(emu->count);

		for (uint32 i = 0; i < emu->count; ++i)
		{
			buf.WriteUInt16(emu->choices[i].dz_zone_id);
			buf.WriteUInt16(emu->choices[i].dz_instance_id);
			buf.WriteUInt32(emu->choices[i].unknown_id1);
			buf.WriteUInt32(emu->choices[i].dz_type);
			buf.WriteUInt32(emu->choices[i].unknown_id2);
			buf.WriteString(emu->choices[i].description);
			buf.WriteString(emu->choices[i].leader_name);
		}

		__packet->size = buf.size();
		__packet->pBuffer = new unsigned char[__packet->size];
		memcpy(__packet->pBuffer, buf.buffer(), __packet->size);

		FINISH_ENCODE();
	}

	ENCODE(OP_DzExpeditionEndsWarning)
	{
		ENCODE_LENGTH_EXACT(ExpeditionExpireWarning);
		SETUP_DIRECT_ENCODE(ExpeditionExpireWarning, structs::ExpeditionExpireWarning);

		OUT(minutes_remaining);

		FINISH_ENCODE();
	}

	ENCODE(OP_DzExpeditionInfo)
	{
		ENCODE_LENGTH_EXACT(DynamicZoneInfo_Struct);
		SETUP_DIRECT_ENCODE(DynamicZoneInfo_Struct, structs::DynamicZoneInfo_Struct);

		OUT(client_id);
		OUT(assigned);
		OUT(max_players);
		strn0cpy(eq->dz_name, emu->dz_name, sizeof(eq->dz_name));
		strn0cpy(eq->leader_name, emu->leader_name, sizeof(eq->leader_name));

		FINISH_ENCODE();
	}

	ENCODE(OP_DzExpeditionInvite)
	{
		ENCODE_LENGTH_EXACT(ExpeditionInvite_Struct);
		SETUP_DIRECT_ENCODE(ExpeditionInvite_Struct, structs::ExpeditionInvite_Struct);

		OUT(client_id);
		strn0cpy(eq->inviter_name, emu->inviter_name, sizeof(eq->inviter_name));
		strn0cpy(eq->expedition_name, emu->expedition_name, sizeof(eq->expedition_name));
		OUT(swapping);
		strn0cpy(eq->swap_name, emu->swap_name, sizeof(eq->swap_name));
		OUT(dz_zone_id);
		OUT(dz_instance_id);

		FINISH_ENCODE();
	}

	ENCODE(OP_DzExpeditionLockoutTimers)
	{
		SETUP_VAR_ENCODE(ExpeditionLockoutTimers_Struct);

		SerializeBuffer buf;
		buf.WriteUInt32(emu->client_id);
		buf.WriteUInt32(emu->count);
		for (uint32 i = 0; i < emu->count; ++i)
		{
			buf.WriteString(emu->timers[i].expedition_name);
			buf.WriteUInt32(emu->timers[i].seconds_remaining);
			buf.WriteInt32(emu->timers[i].event_type);
			buf.WriteString(emu->timers[i].event_name);
		}

		__packet->size = buf.size();
		__packet->pBuffer = new unsigned char[__packet->size];
		memcpy(__packet->pBuffer, buf.buffer(), __packet->size);

		FINISH_ENCODE();
	}

	ENCODE(OP_DzSetLeaderName)
	{
		ENCODE_LENGTH_EXACT(DynamicZoneLeaderName_Struct);
		SETUP_DIRECT_ENCODE(DynamicZoneLeaderName_Struct, structs::DynamicZoneLeaderName_Struct);

		OUT(client_id);
		strn0cpy(eq->leader_name, emu->leader_name, sizeof(eq->leader_name));

		FINISH_ENCODE();
	}

	ENCODE(OP_DzMemberList)
	{
		SETUP_VAR_ENCODE(DynamicZoneMemberList_Struct);

		SerializeBuffer buf;
		buf.WriteUInt32(emu->client_id);
		buf.WriteUInt32(emu->member_count);
		for (uint32 i = 0; i < emu->member_count; ++i)
		{
			buf.WriteString(emu->members[i].name);
			buf.WriteUInt8(emu->members[i].online_status);
		}

		__packet->size = buf.size();
		__packet->pBuffer = new unsigned char[__packet->size];
		memcpy(__packet->pBuffer, buf.buffer(), __packet->size);

		FINISH_ENCODE();
	}

	ENCODE(OP_DzMemberListName)
	{
		ENCODE_LENGTH_EXACT(DynamicZoneMemberListName_Struct);
		SETUP_DIRECT_ENCODE(DynamicZoneMemberListName_Struct, structs::DynamicZoneMemberListName_Struct);

		OUT(client_id);
		OUT(add_name);
		strn0cpy(eq->name, emu->name, sizeof(eq->name));

		FINISH_ENCODE();
	}

	ENCODE(OP_DzMemberListStatus)
	{
		auto emu = reinterpret_cast<DynamicZoneMemberList_Struct*>((*p)->pBuffer);
		if (emu->member_count == 1)
		{
			ENCODE_FORWARD(OP_DzMemberList);
		}
	}

	ENCODE(OP_Emote)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		Emote_Struct *emu = (Emote_Struct *)in->pBuffer;

		unsigned char *__emu_buffer = in->pBuffer;

		std::string old_message = emu->message;
		std::string new_message;
		ServerToSoFSayLink(new_message, old_message);

		//if (new_message.length() > 512) // length restricted in packet building function due vari-length name size (no nullterm)
		//	new_message = new_message.substr(0, 512);

		in->size = new_message.length() + 5;
		in->pBuffer = new unsigned char[in->size];

		char *OutBuffer = (char *)in->pBuffer;

		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->type);
		VARSTRUCT_ENCODE_STRING(OutBuffer, new_message.c_str());

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_ExpansionInfo)
	{
		ENCODE_LENGTH_EXACT(ExpansionInfo_Struct);
		SETUP_DIRECT_ENCODE(ExpansionInfo_Struct, structs::ExpansionInfo_Struct);

		OUT(Expansions);

		FINISH_ENCODE();
	}

	ENCODE(OP_FormattedMessage)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		FormattedMessage_Struct *emu = (FormattedMessage_Struct *)in->pBuffer;

		unsigned char *__emu_buffer = in->pBuffer;

		char *old_message_ptr = (char *)in->pBuffer;
		old_message_ptr += sizeof(FormattedMessage_Struct);

		std::string old_message_array[9];

		for (int i = 0; i < 9; ++i) {
			if (*old_message_ptr == 0) { break; }
			old_message_array[i] = old_message_ptr;
			old_message_ptr += old_message_array[i].length() + 1;
		}

		uint32 new_message_size = 0;
		std::string new_message_array[9];

		for (int i = 0; i < 9; ++i) {
			if (old_message_array[i].length() == 0) { break; }
			ServerToSoFSayLink(new_message_array[i], old_message_array[i]);
			new_message_size += new_message_array[i].length() + 1;
		}

		in->size = sizeof(FormattedMessage_Struct) + new_message_size + 1;
		in->pBuffer = new unsigned char[in->size];

		char *OutBuffer = (char *)in->pBuffer;

		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->unknown0);
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->string_id);
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->type);

		for (int i = 0; i < 9; ++i) {
			if (new_message_array[i].length() == 0) { break; }
			VARSTRUCT_ENCODE_STRING(OutBuffer, new_message_array[i].c_str());
		}

		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, 0);

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_GroundSpawn)
	{
		ENCODE_LENGTH_EXACT(Object_Struct);
		SETUP_DIRECT_ENCODE(Object_Struct, structs::Object_Struct);

		OUT(drop_id);
		OUT(zone_id);
		OUT(zone_instance);
		OUT(heading);
		OUT(x);
		OUT(y);
		OUT(z);
		OUT_str(object_name);
		OUT(object_type);
		OUT(spawn_id);

		/*fill in some unknowns with observed values, hopefully it will help */
		eq->unknown020 = 0;
		eq->unknown024 = 0;
		eq->size = 1;	//This forces all objects to standard size for now
		eq->unknown088 = 0;
		memset(eq->unknown096, 0xFF, sizeof(eq->unknown096));

		FINISH_ENCODE();
	}

	ENCODE(OP_GuildMemberList)
	{
		//consume the packet
		EQApplicationPacket *in = *p;
		*p = nullptr;

		//store away the emu struct
		unsigned char *__emu_buffer = in->pBuffer;
		Internal_GuildMembers_Struct *emu = (Internal_GuildMembers_Struct *)in->pBuffer;

		//make a new EQ buffer.
		uint32 pnl = strlen(emu->player_name);
		uint32 length = sizeof(structs::GuildMembers_Struct) + pnl +
			emu->count*sizeof(structs::GuildMemberEntry_Struct)
			+ emu->name_length + emu->note_length;
		in->pBuffer = new uint8[length];
		in->size = length;
		//no memset since we fill every byte.

		uint8 *buffer;
		buffer = in->pBuffer;

		//easier way to setup GuildMembers_Struct
		//set prefix name
		strcpy((char *)buffer, emu->player_name);
		buffer += pnl;
		*buffer = '\0';
		buffer++;

		//add member count.
		*((uint32 *)buffer) = htonl(emu->count);
		buffer += sizeof(uint32);

		if (emu->count > 0) {
			Internal_GuildMemberEntry_Struct *emu_e = emu->member;
			const char *emu_name = (const char *)(__emu_buffer +
				sizeof(Internal_GuildMembers_Struct)+ //skip header
				emu->count * sizeof(Internal_GuildMemberEntry_Struct)	//skip static length member data
				);
			const char *emu_note = (emu_name +
				emu->name_length + //skip name contents
				emu->count	//skip string terminators
				);

			structs::GuildMemberEntry_Struct *e = (structs::GuildMemberEntry_Struct *) buffer;

			uint32 r;
			for (r = 0; r < emu->count; r++, emu_e++) {

				//the order we set things here must match the struct

				//nice helper macro
				/*#define SlideStructString(field, str) \
				strcpy(e->field, str.c_str()); \
				e = (GuildMemberEntry_Struct *) ( ((uint8 *)e) + str.length() )*/
#define SlideStructString(field, str) \
			{ \
				int sl = strlen(str); \
				memcpy(e->field, str, sl+1); \
				e = (structs::GuildMemberEntry_Struct *) ( ((uint8 *)e) + sl ); \
				str += sl + 1; \
			}
#define PutFieldN(field) e->field = htonl(emu_e->field)

				SlideStructString(name, emu_name);
				PutFieldN(level);
				PutFieldN(banker);
				PutFieldN(class_);
				PutFieldN(rank);
				PutFieldN(time_last_on);
				PutFieldN(tribute_enable);
				PutFieldN(total_tribute);
				PutFieldN(last_tribute);
				e->unknown_one = htonl(1);
				SlideStructString(public_note, emu_note);
				e->zoneinstance = 0;
				e->zone_id = htons(emu_e->zone_id);

#undef SlideStructString
#undef PutFieldN

				e++;
			}
		}

		delete[] __emu_buffer;

		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_Illusion)
	{
		ENCODE_LENGTH_EXACT(Illusion_Struct);
		SETUP_DIRECT_ENCODE(Illusion_Struct, structs::Illusion_Struct);

		OUT(spawnid);
		OUT_str(charname);
		OUT(race);
		OUT(unknown006[0]);
		OUT(unknown006[1]);
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

	ENCODE(OP_InspectRequest)
	{
		ENCODE_LENGTH_EXACT(Inspect_Struct);
		SETUP_DIRECT_ENCODE(Inspect_Struct, structs::Inspect_Struct);

		OUT(TargetID);
		OUT(PlayerID);

		FINISH_ENCODE();
	}

	ENCODE(OP_ItemLinkResponse) { ENCODE_FORWARD(OP_ItemPacket); }

	ENCODE(OP_ItemPacket)
	{
		//consume the packet
		EQApplicationPacket* in = *p;
		*p = nullptr;

		//store away the emu struct
		uchar* __emu_buffer = in->pBuffer;

		EQ::InternalSerializedItem_Struct* int_struct = (EQ::InternalSerializedItem_Struct*)(&__emu_buffer[4]);

		EQ::OutBuffer ob;
		EQ::OutBuffer::pos_type last_pos = ob.tellp();

		ob.write((const char*)__emu_buffer, 4);

		SerializeItem(ob, (const EQ::ItemInstance*)int_struct->inst, int_struct->slot_id, 0);
		if (ob.tellp() == last_pos) {
			LogNetcode("SoF::ENCODE(OP_ItemPacket) Serialization failed on item slot [{}]", int_struct->slot_id);
			delete in;
			return;
		}

		in->size = ob.size();
		in->pBuffer = ob.detach();

		delete[] __emu_buffer;

		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_ItemVerifyReply)
	{
		ENCODE_LENGTH_EXACT(ItemVerifyReply_Struct);
		SETUP_DIRECT_ENCODE(ItemVerifyReply_Struct, structs::ItemVerifyReply_Struct);

		eq->slot = ServerToSoFSlot(emu->slot);
		OUT(spell);
		OUT(target);

		FINISH_ENCODE();
	}

	ENCODE(OP_LeadershipExpUpdate)
	{
		SETUP_DIRECT_ENCODE(LeadershipExpUpdate_Struct, structs::LeadershipExpUpdate_Struct);

		OUT(group_leadership_exp);
		OUT(group_leadership_points);
		OUT(raid_leadership_exp);
		OUT(raid_leadership_points);

		FINISH_ENCODE();
	}

	ENCODE(OP_LogServer)
	{
		ENCODE_LENGTH_EXACT(LogServer_Struct);
		SETUP_DIRECT_ENCODE(LogServer_Struct, structs::LogServer_Struct);

		strcpy(eq->worldshortname, emu->worldshortname);

		OUT(enablevoicemacros);
		OUT(enablemail);
		OUT(enable_pvp);
		OUT(enable_FV);

		// These next two need to be set like this for the Tutorial Button to work.
		eq->unknown263[0] = 0;
		eq->unknown263[2] = 1;

		FINISH_ENCODE();
	}

	ENCODE(OP_LootItem)
	{
		ENCODE_LENGTH_EXACT(LootingItem_Struct);
		SETUP_DIRECT_ENCODE(LootingItem_Struct, structs::LootingItem_Struct);

		Log(Logs::Detail, Logs::Netcode, "SoF::ENCODE(OP_LootItem)");

		OUT(lootee);
		OUT(looter);
		eq->slot_id = ServerToSoFCorpseSlot(emu->slot_id);
		OUT(auto_loot);

		FINISH_ENCODE();
	}

	ENCODE(OP_MemorizeSpell)
	{
		ENCODE_LENGTH_EXACT(MemorizeSpell_Struct);
		SETUP_DIRECT_ENCODE(MemorizeSpell_Struct, structs::MemorizeSpell_Struct);

		// Since HT/LoH are translated up, we need to translate down only for memSpellSpellbar case
		if (emu->scribing == 3)
			eq->slot = static_cast<uint32>(ServerToSoFCastingSlot(static_cast<EQ::spells::CastingSlot>(emu->slot)));
		else
			OUT(slot);
		OUT(spell_id);
		OUT(scribing);

		FINISH_ENCODE();
	}

	ENCODE(OP_MoveItem)
	{
		ENCODE_LENGTH_EXACT(MoveItem_Struct);
		SETUP_DIRECT_ENCODE(MoveItem_Struct, structs::MoveItem_Struct);

		Log(Logs::Detail, Logs::Netcode, "SoF::ENCODE(OP_MoveItem)");

		eq->from_slot = ServerToSoFSlot(emu->from_slot);
		eq->to_slot = ServerToSoFSlot(emu->to_slot);
		OUT(number_in_stack);

		FINISH_ENCODE();
	}

	ENCODE(OP_NewSpawn) { ENCODE_FORWARD(OP_ZoneSpawns); }

	ENCODE(OP_NewZone)
	{
		SETUP_DIRECT_ENCODE(NewZone_Struct, structs::NewZone_Struct);

		OUT_str(char_name);
		OUT_str(zone_short_name);
		OUT_str(zone_long_name);
		OUT(ztype);
		int r;
		for (r = 0; r < 4; r++) {
			OUT(fog_red[r]);
			OUT(fog_green[r]);
			OUT(fog_blue[r]);
			OUT(fog_minclip[r]);
			OUT(fog_maxclip[r]);
		}
		OUT(gravity);
		OUT(time_type);
		for (r = 0; r < 4; r++) {
			OUT(rain_chance[r]);
		}
		for (r = 0; r < 4; r++) {
			OUT(rain_duration[r]);
		}
		for (r = 0; r < 4; r++) {
			OUT(snow_chance[r]);
		}
		for (r = 0; r < 4; r++) {
			OUT(snow_duration[r]);
		}
		for (r = 0; r < 32; r++) {
			eq->unknown537[r] = 0xFF;	//observed
		}
		OUT(sky);
		OUT(zone_exp_multiplier);
		OUT(safe_y);
		OUT(safe_x);
		OUT(safe_z);
		OUT(max_z);
		OUT(underworld);
		OUT(minclip);
		OUT(maxclip);
		OUT_str(zone_short_name2);
		OUT(zone_id);
		OUT(zone_instance);
		OUT(suspend_buffs);
		OUT(fast_regen_hp);
		OUT(fast_regen_mana);
		OUT(fast_regen_endurance);
		OUT(underworld_teleport_index);

		/*fill in some unknowns with observed values, hopefully it will help */
		eq->unknown796 = -1;
		eq->unknown840 = 600;
		OUT(lava_damage);
		OUT(min_lava_damage);
		eq->unknown884 = 1;
		eq->unknown885 = 0;
		eq->unknown886 = 1;
		eq->unknown887 = 0;
		eq->unknown888 = 0;
		eq->unknown889 = 0;
		eq->fall_damage = 0;	// 0 = Fall Damage on, 1 = Fall Damage off
		eq->unknown891 = 0;
		eq->unknown904 = 2;
		eq->unknown908 = 2;

		FINISH_ENCODE();
	}

	ENCODE(OP_OnLevelMessage)
	{
		ENCODE_LENGTH_EXACT(OnLevelMessage_Struct);
		SETUP_DIRECT_ENCODE(OnLevelMessage_Struct, structs::OnLevelMessage_Struct);

		OUT_str(Title);
		OUT_str(Text);
		OUT(Buttons);
		OUT(Duration);
		OUT(PopupID);

		eq->unknown4236 = 0x00000000;
		eq->unknown4240 = 0xffffffff;

		FINISH_ENCODE();
	}

	ENCODE(OP_PetBuffWindow)
	{
		ENCODE_LENGTH_EXACT(PetBuff_Struct);
		SETUP_DIRECT_ENCODE(PetBuff_Struct, PetBuff_Struct);

		OUT(petid);
		OUT(buffcount);

		int EQBuffSlot = 0; // do we really want to shuffle them around like this?

		for (uint32 EmuBuffSlot = 0; EmuBuffSlot < PET_BUFF_COUNT; ++EmuBuffSlot)
		{
			if (emu->spellid[EmuBuffSlot])
			{
				eq->spellid[EQBuffSlot] = emu->spellid[EmuBuffSlot];
				eq->ticsremaining[EQBuffSlot++] = emu->ticsremaining[EmuBuffSlot];
			}
		}

		FINISH_ENCODE();
	}

	ENCODE(OP_PlayerProfile)
	{
		SETUP_DIRECT_ENCODE(PlayerProfile_Struct, structs::PlayerProfile_Struct);

		uint32 r;

		eq->available_slots = 0xffffffff;
		memset(eq->unknown4184, 0xff, sizeof(eq->unknown4184));
		memset(eq->unknown04396, 0xff, sizeof(eq->unknown04396));

		//	OUT(checksum);
		OUT(gender);
		OUT(race);
		OUT(class_);
		//	OUT(unknown00016);
		OUT(level);
		eq->level1 = emu->level;
		//	OUT(unknown00022[2]);
		for (r = 0; r < 5; r++) {
			OUT(binds[r].zone_id);
			OUT(binds[r].x);
			OUT(binds[r].y);
			OUT(binds[r].z);
			OUT(binds[r].heading);
		}
		OUT(deity);
		OUT(intoxication);

		OUT_array(spellSlotRefresh, spells::SPELL_GEM_COUNT);
		eq->spellSlotRefresh[9] = 0; // 10th slot is not valid in this release

		OUT(abilitySlotRefresh);
		OUT(points); // Relocation Test
		//	OUT(unknown0166[4]);
		OUT(haircolor);
		OUT(beardcolor);
		OUT(eyecolor1);
		OUT(eyecolor2);
		OUT(hairstyle);
		OUT(beard);
		//	OUT(unknown00178[10]);
		for (r = EQ::textures::textureBegin; r < EQ::textures::materialCount; r++) {
			eq->equipment.Slot[r].Material = emu->item_material.Slot[r].Material;
			eq->equipment.Slot[r].Unknown1 = 0;
			eq->equipment.Slot[r].EliteMaterial = 0;
			//eq->colors[r].color = emu->colors[r].color;
		}
		for (r = 0; r < 7; r++) {
			OUT(item_tint.Slot[r].Color);
		}
		//	OUT(unknown00224[48]);
		//NOTE: new client supports 300 AAs, our internal rep/PP
		//only supports 240..
		for (r = 0; r < MAX_PP_AA_ARRAY; r++) {
			OUT(aa_array[r].AA);
			OUT(aa_array[r].value);
			OUT(aa_array[r].charges);
		}
		//	OUT(unknown02220[4]);
		OUT(mana);
		OUT(cur_hp);
		OUT(STR);
		OUT(STA);
		OUT(CHA);
		OUT(AGI);
		OUT(INT);
		OUT(DEX);
		OUT(WIS);
		OUT(face);
		//	OUT(unknown02264[47]);

		if (spells::SPELLBOOK_SIZE <= EQ::spells::SPELLBOOK_SIZE) {
			for (uint32 r = 0; r < spells::SPELLBOOK_SIZE; r++) {
				if (emu->spell_book[r] <= spells::SPELL_ID_MAX)
					eq->spell_book[r] = emu->spell_book[r];
				else
					eq->spell_book[r] = 0xFFFFFFFFU;
			}
		}
		else {
			for (uint32 r = 0; r < EQ::spells::SPELLBOOK_SIZE; r++) {
				if (emu->spell_book[r] <= spells::SPELL_ID_MAX)
					eq->spell_book[r] = emu->spell_book[r];
				else
					eq->spell_book[r] = 0xFFFFFFFFU;
			}
			// invalidate the rest of the spellbook slots
			memset(&eq->spell_book[EQ::spells::SPELLBOOK_SIZE], 0xFF, (sizeof(uint32) * (spells::SPELLBOOK_SIZE - EQ::spells::SPELLBOOK_SIZE)));
		}

		//	OUT(unknown4184[128]);

		OUT_array(mem_spells, spells::SPELL_GEM_COUNT);
		eq->mem_spells[9] = 0xFFFFFFFFU; // 10th slot is not valid in this release

		//	OUT(unknown04396[32]);
		OUT(platinum);
		OUT(gold);
		OUT(silver);
		OUT(copper);
		OUT(platinum_cursor);
		OUT(gold_cursor);
		OUT(silver_cursor);
		OUT(copper_cursor);

		OUT_array(skills, structs::MAX_PP_SKILL);	// 1:1 direct copy (100 dword)
		OUT_array(InnateSkills, structs::MAX_PP_INNATE_SKILL);  // 1:1 direct copy (25 dword)

		//	OUT(unknown04760[236]);
		OUT(toxicity);
		OUT(thirst_level);
		OUT(hunger_level);
		for (r = 0; r < structs::BUFF_COUNT; r++) {
			OUT(buffs[r].effect_type);
			OUT(buffs[r].level);
			OUT(buffs[r].bard_modifier);
			OUT(buffs[r].unknown003);
			OUT(buffs[r].spellid);
			OUT(buffs[r].duration);
			OUT(buffs[r].counters);
			OUT(buffs[r].player_id);
		}
		for (r = 0; r < structs::MAX_PP_DISCIPLINES; r++) {
			OUT(disciplines.values[r]);
		}
		OUT_array(recastTimers, structs::MAX_RECAST_TYPES);
		//	OUT(unknown08124[360]);
		OUT(endurance);
		OUT(aapoints_spent);
		OUT(aapoints);

		//	OUT(unknown06160[4]);

		// Copy bandoliers where server and client indices converge
		for (r = 0; r < EQ::profile::BANDOLIERS_SIZE && r < profile::BANDOLIERS_SIZE; ++r) {
			OUT_str(bandoliers[r].Name);
			for (uint32 k = 0; k < profile::BANDOLIER_ITEM_COUNT; ++k) { // Will need adjusting if 'server != client' is ever true
				OUT(bandoliers[r].Items[k].ID);
				OUT(bandoliers[r].Items[k].Icon);
				OUT_str(bandoliers[r].Items[k].Name);
			}
		}
		// Nullify bandoliers where server and client indices diverge, with a client bias
		for (r = EQ::profile::BANDOLIERS_SIZE; r < profile::BANDOLIERS_SIZE; ++r) {
			eq->bandoliers[r].Name[0] = '\0';
			for (uint32 k = 0; k < profile::BANDOLIER_ITEM_COUNT; ++k) { // Will need adjusting if 'server != client' is ever true
				eq->bandoliers[r].Items[k].ID = 0;
				eq->bandoliers[r].Items[k].Icon = 0;
				eq->bandoliers[r].Items[k].Name[0] = '\0';
			}
		}

		//	OUT(unknown07444[5120]);

		// Copy potion belt where server and client indices converge
		for (r = 0; r < EQ::profile::POTION_BELT_SIZE && r < profile::POTION_BELT_SIZE; ++r) {
			OUT(potionbelt.Items[r].ID);
			OUT(potionbelt.Items[r].Icon);
			OUT_str(potionbelt.Items[r].Name);
		}
		// Nullify potion belt where server and client indices diverge, with a client bias
		for (r = EQ::profile::POTION_BELT_SIZE; r < profile::POTION_BELT_SIZE; ++r) {
			eq->potionbelt.Items[r].ID = 0;
			eq->potionbelt.Items[r].Icon = 0;
			eq->potionbelt.Items[r].Name[0] = '\0';
		}

		//	OUT(unknown12852[8]);
		//	OUT(unknown12864[76]);

		OUT_str(name);
		OUT_str(last_name);
		OUT(guild_id);
		OUT(birthday);
		OUT(lastlogin);
		OUT(timePlayedMin);
		OUT(pvp);
		OUT(anon);
		OUT(gm);
		OUT(guildrank);
		OUT(guildbanker);
		//	OUT(unknown13054[12]);
		OUT(exp);
		//	OUT(unknown13072[8]);
		OUT(timeentitledonaccount);
		OUT_array(languages, structs::MAX_PP_LANGUAGE);
		//	OUT(unknown13109[7]);
		OUT(y); //reversed x and y
		OUT(x);
		OUT(z);
		OUT(heading);
		//	OUT(unknown13132[4]);
		OUT(platinum_bank);
		OUT(gold_bank);
		OUT(silver_bank);
		OUT(copper_bank);
		OUT(platinum_shared);
		//	OUT(unknown13156[84]);
		OUT(expansions);
		//eq->expansions = 16383;
		//	OUT(unknown13244[12]);
		OUT(autosplit);
		//	OUT(unknown13260[16]);
		OUT(zone_id);
		OUT(zoneInstance);
		for (r = 0; r < structs::MAX_GROUP_MEMBERS; r++) {
			OUT_str(groupMembers[r]);
		}
		strcpy(eq->groupLeader, emu->groupMembers[0]);
		//	OUT_str(groupLeader);
		//	OUT(unknown13728[660]);
		OUT(entityid);
		OUT(leadAAActive);
		//	OUT(unknown14392[4]);
		OUT(ldon_points_guk);
		OUT(ldon_points_mir);
		OUT(ldon_points_mmc);
		OUT(ldon_points_ruj);
		OUT(ldon_points_tak);
		OUT(ldon_points_available);
		//	OUT(unknown14420[132]);
		OUT(tribute_time_remaining);
		OUT(career_tribute_points);
		//	OUT(unknown7208);
		OUT(tribute_points);
		//	OUT(unknown7216);
		OUT(tribute_active);
		for (r = 0; r < structs::MAX_PLAYER_TRIBUTES; r++) {
			OUT(tributes[r].tribute);
			OUT(tributes[r].tier);
		}
		//	OUT(unknown14616[8]);
		OUT(group_leadership_exp);
		//	OUT(unknown14628);
		OUT(raid_leadership_exp);
		OUT(group_leadership_points);
		OUT(raid_leadership_points);
		OUT_array(leader_abilities.ranks, structs::MAX_LEADERSHIP_AA_ARRAY);
		//	OUT(unknown14772[128]);
		OUT(air_remaining);
		OUT(PVPKills);
		OUT(PVPDeaths);
		OUT(PVPCurrentPoints);
		OUT(PVPCareerPoints);
		OUT(PVPBestKillStreak);
		OUT(PVPWorstDeathStreak);
		OUT(PVPCurrentKillStreak);
		//	OUT(unknown17892[4580]);
		OUT(expAA);
		//	OUT(unknown19516[40]);
		OUT(currentRadCrystals);
		OUT(careerRadCrystals);
		OUT(currentEbonCrystals);
		OUT(careerEbonCrystals);
		OUT(groupAutoconsent);
		OUT(raidAutoconsent);
		OUT(guildAutoconsent);
		//	OUT(unknown19575[5]);
		eq->level3 = emu->level;
		eq->showhelm = emu->showhelm;
		OUT(RestTimer);
		//	OUT(unknown19584[4]);
		//	OUT(unknown19588);

		const uint8 bytes[] = {
			0xa3, 0x02, 0x00, 0x00, 0x95, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00,
			0x19, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00,
			0x0F, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x1F, 0x85, 0xEB, 0x3E, 0x33, 0x33, 0x33, 0x3F,
			0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		};

		memcpy(eq->unknown12864, bytes, sizeof(bytes));

		//set the checksum...
		CRC32::SetEQChecksum(__packet->pBuffer, sizeof(structs::PlayerProfile_Struct) - 4);

		FINISH_ENCODE();
	}

	ENCODE(OP_RaidJoin)
	{
		EQApplicationPacket* inapp = *p;
		*p = nullptr;
		unsigned char* __emu_buffer = inapp->pBuffer;
		RaidCreate_Struct* emu = (RaidCreate_Struct*)__emu_buffer;

		auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(structs::RaidGeneral_Struct));
		structs::RaidGeneral_Struct* general = (structs::RaidGeneral_Struct*)outapp->pBuffer;

		general->action = raidCreate;
		general->parameter = RaidCommandAcceptInvite;
		strn0cpy(general->leader_name, emu->leader_name, sizeof(emu->leader_name));
		strn0cpy(general->player_name, emu->leader_name, sizeof(emu->leader_name));

		dest->FastQueuePacket(&outapp);

		safe_delete(inapp);

	}

	ENCODE(OP_RaidUpdate)
	{
		EQApplicationPacket* inapp = *p;
		*p = nullptr;
		unsigned char* __emu_buffer = inapp->pBuffer;
		RaidGeneral_Struct* raid_gen = (RaidGeneral_Struct*)__emu_buffer;

		switch (raid_gen->action)
		{
		case raidAdd:
		{
			RaidAddMember_Struct* emu = (RaidAddMember_Struct*)__emu_buffer;

			auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(structs::RaidAddMember_Struct));
			structs::RaidAddMember_Struct* eq = (structs::RaidAddMember_Struct*)outapp->pBuffer;

			OUT(raidGen.action);
			OUT(raidGen.parameter);
			OUT_str(raidGen.leader_name);
			OUT_str(raidGen.player_name);
			OUT(_class);
			OUT(level);
			OUT(isGroupLeader);
			OUT(flags[0]);
			OUT(flags[1]);
			OUT(flags[2]);
			OUT(flags[3]);
			OUT(flags[4]);

			dest->FastQueuePacket(&outapp);
			break;
		}
		case raidSetMotd:
		{
			RaidMOTD_Struct* emu = (RaidMOTD_Struct*)__emu_buffer;

			auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(structs::RaidMOTD_Struct));
			structs::RaidMOTD_Struct* eq = (structs::RaidMOTD_Struct*)outapp->pBuffer;

			OUT(general.action);
			OUT_str(general.player_name);
			OUT_str(general.leader_name);
			OUT_str(motd);

			dest->FastQueuePacket(&outapp);
			break;
		}
		case raidSetLeaderAbilities:
		case raidMakeLeader:
		{
			RaidLeadershipUpdate_Struct* emu = (RaidLeadershipUpdate_Struct*)__emu_buffer;

			auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(structs::RaidLeadershipUpdate_Struct));
			structs::RaidLeadershipUpdate_Struct* eq = (structs::RaidLeadershipUpdate_Struct*)outapp->pBuffer;

			OUT(action);
			OUT_str(player_name);
			OUT_str(leader_name);
			memcpy(&eq->raid, &emu->raid, sizeof(RaidLeadershipAA_Struct));

			dest->FastQueuePacket(&outapp);
			break;
		}
		case raidSetNote:
		{
			auto emu = (RaidNote_Struct*)__emu_buffer;

			auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(structs::RaidNote_Struct));
			auto eq = (structs::RaidNote_Struct*)outapp->pBuffer;

			OUT(general.action);
			OUT_str(general.leader_name);
			OUT_str(general.player_name);
			OUT_str(note);

			dest->FastQueuePacket(&outapp);
			break;
		}
		case raidNoRaid:
		{
			dest->QueuePacket(inapp);
			break;
		}
		default:
		{
			RaidGeneral_Struct* emu = (RaidGeneral_Struct*)__emu_buffer;

			auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(structs::RaidGeneral_Struct));
			structs::RaidGeneral_Struct* eq = (structs::RaidGeneral_Struct*)outapp->pBuffer;

			OUT(action);
			OUT(parameter);
			OUT_str(leader_name);
			OUT_str(player_name);

			dest->FastQueuePacket(&outapp);
			break;
		}
		}
		safe_delete(inapp);
	}

	ENCODE(OP_ReadBook)
	{
		ENCODE_LENGTH_ATLEAST(BookText_Struct);
		SETUP_DIRECT_ENCODE(BookText_Struct, structs::BookRequest_Struct);

		if (emu->window == 0xFF)
			eq->window = 0xFFFFFFFF;
		else
			eq->window = emu->window;
		OUT(type);
		eq->invslot = ServerToSoFSlot(emu->invslot);
		OUT(target_id);
		OUT(can_cast);
		OUT(can_scribe);
		strn0cpy(eq->txtfile, emu->booktext, sizeof(eq->txtfile));

		FINISH_ENCODE();
	}

	ENCODE(OP_SendAATable)
	{
		EQApplicationPacket *inapp = *p;
		*p = nullptr;
		AARankInfo_Struct *emu = (AARankInfo_Struct*)inapp->pBuffer;

		auto outapp = new EQApplicationPacket(
		    OP_SendAATable, sizeof(structs::SendAA_Struct) + emu->total_effects * sizeof(structs::AA_Ability));
		structs::SendAA_Struct *eq = (structs::SendAA_Struct*)outapp->pBuffer;

		inapp->SetReadPosition(sizeof(AARankInfo_Struct));
		outapp->SetWritePosition(sizeof(structs::SendAA_Struct));

		eq->id = emu->id;
		eq->unknown004 = 1;
		eq->id = emu->id;
		eq->hotkey_sid = emu->upper_hotkey_sid;
		eq->hotkey_sid2 = emu->lower_hotkey_sid;
		eq->desc_sid = emu->desc_sid;
		eq->title_sid = emu->title_sid;
		eq->class_type = emu->level_req;
		eq->cost = emu->cost;
		eq->seq = emu->seq;
		eq->current_level = emu->current_level;
		eq->type = emu->type;
		eq->spellid = emu->spell;
		eq->spell_type = emu->spell_type;
		eq->spell_refresh = emu->spell_refresh;
		eq->classes = emu->classes;
		eq->max_level = emu->max_level;
		eq->last_id = emu->prev_id;
		eq->next_id = emu->next_id;
		eq->cost2 = emu->total_cost;
		eq->grant_only = emu->grant_only;
		eq->expendable_charges = emu->charges;
		eq->aa_expansion = emu->expansion;
		eq->special_category = emu->category;
		eq->total_abilities = emu->total_effects;

		for(auto i = 0; i < eq->total_abilities; ++i) {
			eq->abilities[i].skill_id = inapp->ReadUInt32();
			eq->abilities[i].base_value = inapp->ReadUInt32();
			eq->abilities[i].limit_value = inapp->ReadUInt32();
			eq->abilities[i].slot = inapp->ReadUInt32();
		}

		if(emu->total_prereqs > 0) {
			eq->prereq_skill = inapp->ReadUInt32();
			eq->prereq_minpoints = inapp->ReadUInt32();
		}

		dest->FastQueuePacket(&outapp);
		delete inapp;
	}

	ENCODE(OP_SendCharInfo)
	{
		ENCODE_LENGTH_ATLEAST(CharacterSelect_Struct);
		SETUP_VAR_ENCODE(CharacterSelect_Struct);

		// Zero-character count shunt
		if (emu->CharCount == 0) {
			ALLOC_VAR_ENCODE(structs::CharacterSelect_Struct, sizeof(structs::CharacterSelect_Struct));
			eq->CharCount = emu->CharCount;
			eq->TotalChars = emu->TotalChars;

			if (eq->TotalChars > constants::CHARACTER_CREATION_LIMIT)
				eq->TotalChars = constants::CHARACTER_CREATION_LIMIT;

			FINISH_ENCODE();
			return;
		}

		unsigned char *emu_ptr = __emu_buffer;
		emu_ptr += sizeof(CharacterSelect_Struct);
		CharacterSelectEntry_Struct *emu_cse = (CharacterSelectEntry_Struct *)nullptr;

		size_t names_length = 0;
		size_t character_count = 0;
		for (; character_count < emu->CharCount && character_count < constants::CHARACTER_CREATION_LIMIT; ++character_count) {
			emu_cse = (CharacterSelectEntry_Struct *)emu_ptr;
			names_length += strlen(emu_cse->Name);
			emu_ptr += sizeof(CharacterSelectEntry_Struct);
		}

		size_t total_length = sizeof(structs::CharacterSelect_Struct)
			+ character_count * sizeof(structs::CharacterSelectEntry_Struct)
			+ names_length;

		ALLOC_VAR_ENCODE(structs::CharacterSelect_Struct, total_length);
		structs::CharacterSelectEntry_Struct *eq_cse = (structs::CharacterSelectEntry_Struct *)nullptr;

		eq->CharCount = character_count;
		eq->TotalChars = emu->TotalChars;

		if (eq->TotalChars > constants::CHARACTER_CREATION_LIMIT)
			eq->TotalChars = constants::CHARACTER_CREATION_LIMIT;

		emu_ptr = __emu_buffer;
		emu_ptr += sizeof(CharacterSelect_Struct);

		unsigned char *eq_ptr = __packet->pBuffer;
		eq_ptr += sizeof(structs::CharacterSelect_Struct);

		for (int counter = 0; counter < character_count; ++counter) {
			emu_cse = (CharacterSelectEntry_Struct *)emu_ptr;
			eq_cse = (structs::CharacterSelectEntry_Struct *)eq_ptr; // base address

			eq_cse->Level = emu_cse->Level;
			eq_cse->HairStyle = emu_cse->HairStyle;
			eq_cse->Gender = emu_cse->Gender;

			strcpy(eq_cse->Name, emu_cse->Name);
			eq_ptr += strlen(emu_cse->Name);
			eq_cse = (structs::CharacterSelectEntry_Struct *)eq_ptr; // offset address (base + name length offset)
			eq_cse->Name[0] = '\0'; // (offset)eq_cse->Name[0] = (base)eq_cse->Name[strlen(emu_cse->Name)]

			eq_cse->Beard = emu_cse->Beard;
			eq_cse->HairColor = emu_cse->HairColor;
			eq_cse->Face = emu_cse->Face;

			for (int equip_index = EQ::textures::textureBegin; equip_index < EQ::textures::materialCount; equip_index++) {
				eq_cse->Equip[equip_index].Material = emu_cse->Equip[equip_index].Material;
				eq_cse->Equip[equip_index].Unknown1 = emu_cse->Equip[equip_index].Unknown1;
				eq_cse->Equip[equip_index].EliteMaterial = emu_cse->Equip[equip_index].EliteModel;
				eq_cse->Equip[equip_index].Color = emu_cse->Equip[equip_index].Color;
			}

			eq_cse->PrimaryIDFile = emu_cse->PrimaryIDFile;
			eq_cse->SecondaryIDFile = emu_cse->SecondaryIDFile;
			eq_cse->Tutorial = emu_cse->Tutorial;
			eq_cse->Unknown15 = emu_cse->Unknown15;
			eq_cse->Deity = emu_cse->Deity;
			eq_cse->Zone = emu_cse->Zone;
			eq_cse->Unknown19 = emu_cse->Unknown19;
			eq_cse->Race = emu_cse->Race;
			eq_cse->GoHome = emu_cse->GoHome;
			eq_cse->Class = emu_cse->Class;
			eq_cse->EyeColor1 = emu_cse->EyeColor1;
			eq_cse->BeardColor = emu_cse->BeardColor;
			eq_cse->EyeColor2 = emu_cse->EyeColor2;
			eq_cse->DrakkinHeritage = emu_cse->DrakkinHeritage;
			eq_cse->DrakkinTattoo = emu_cse->DrakkinTattoo;
			eq_cse->DrakkinDetails = emu_cse->DrakkinDetails;

			emu_ptr += sizeof(CharacterSelectEntry_Struct);
			eq_ptr += sizeof(structs::CharacterSelectEntry_Struct);
		}

		FINISH_ENCODE();
	}

	//hack hack hack
	ENCODE(OP_SendZonepoints)
	{
		ENCODE_LENGTH_ATLEAST(ZonePoints);

		SETUP_VAR_ENCODE(ZonePoints);
		ALLOC_VAR_ENCODE(structs::ZonePoints, __packet->size);

		memcpy(eq, emu, __packet->size);

		FINISH_ENCODE();
		//	unknown0xxx[24];
		//this is utter crap... the client is waiting for this
		//certain 0 length opcode to come after the reqclientspawn
		//stuff... so this is a dirty way to put it in there.
		// this needs to be done better

		//EQApplicationPacket hack_test(OP_PetitionUnCheckout, 0);
		//dest->QueuePacket(&hack_test);
	}

	ENCODE(OP_ShopPlayerSell)
	{
		ENCODE_LENGTH_EXACT(Merchant_Purchase_Struct);
		SETUP_DIRECT_ENCODE(Merchant_Purchase_Struct, structs::Merchant_Purchase_Struct);

		OUT(npcid);
		eq->itemslot = ServerToSoFSlot(emu->itemslot);
		OUT(quantity);
		OUT(price);

		FINISH_ENCODE();
	}

	ENCODE(OP_ShopRequest)
	{
		ENCODE_LENGTH_EXACT(MerchantClick_Struct);
		SETUP_DIRECT_ENCODE(MerchantClick_Struct, structs::MerchantClick_Struct);

		OUT(npc_id);
		OUT(player_id);
		OUT(command);
		OUT(rate);

		FINISH_ENCODE();
	}

	ENCODE(OP_SomeItemPacketMaybe)
	{
		// This Opcode is not named very well. It is used for the animation of arrows leaving the player's bow
		// and flying to the target.
		//

		ENCODE_LENGTH_EXACT(Arrow_Struct);
		SETUP_DIRECT_ENCODE(Arrow_Struct, structs::Arrow_Struct);

		OUT(src_y);
		OUT(src_x);
		OUT(src_z);
		OUT(velocity);
		OUT(launch_angle);
		OUT(tilt);
		OUT(arc);
		OUT(source_id);
		OUT(target_id);
		OUT(item_id);

		eq->unknown070 = 175; // This needs to be set to something, else we get a 1HS animation instead of ranged.

		OUT(item_type);
		OUT(skill);

		strcpy(eq->model_name, emu->model_name);

		FINISH_ENCODE();
	}

	ENCODE(OP_SpawnDoor)
	{
		SETUP_VAR_ENCODE(Door_Struct);

		int door_count = __packet->size / sizeof(Door_Struct);
		int total_length = door_count * sizeof(structs::Door_Struct);
		ALLOC_VAR_ENCODE(structs::Door_Struct, total_length);
		int r;
		for (r = 0; r < door_count; r++) {
			strcpy(eq[r].name, emu[r].name);
			eq[r].xPos = emu[r].xPos;
			eq[r].yPos = emu[r].yPos;
			eq[r].zPos = emu[r].zPos;
			eq[r].heading = emu[r].heading;
			eq[r].incline = emu[r].incline;
			eq[r].size = emu[r].size;
			eq[r].doorId = emu[r].doorId;
			eq[r].opentype = emu[r].opentype;
			eq[r].state_at_spawn = emu[r].state_at_spawn;
			eq[r].invert_state = emu[r].invert_state;
			eq[r].door_param = emu[r].door_param;
			eq[r].unknown0076 = 0;
			eq[r].unknown0077 = 1; // Both must be 1 to allow clicking doors
			eq[r].unknown0078 = 0;
			eq[r].unknown0079 = 1; // Both must be 1 to allow clicking doors
			eq[r].unknown0080 = 0;
			eq[r].unknown0081 = 0;
			eq[r].unknown0082 = 0;
		}

		FINISH_ENCODE();
	}

	ENCODE(OP_SpecialMesg)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		SerializeBuffer buf(in->size);
		buf.WriteInt8(in->ReadUInt8()); // speak mode
		buf.WriteInt8(in->ReadUInt8()); // journal mode
		buf.WriteInt8(in->ReadUInt8()); // language
		buf.WriteInt32(in->ReadUInt32()); // message type
		buf.WriteInt32(in->ReadUInt32()); // target spawn id

		std::string name;
		in->ReadString(name);

		buf.WriteString(name);

		buf.WriteInt32(in->ReadUInt32()); // loc
		buf.WriteInt32(in->ReadUInt32());
		buf.WriteInt32(in->ReadUInt32());

		std::string old_message;
		std::string new_message;

		in->ReadString(old_message);

		ServerToSoFSayLink(new_message, old_message);

		buf.WriteString(new_message);

		auto outapp = new EQApplicationPacket(OP_SpecialMesg, buf);

		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
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

	ENCODE(OP_TaskDescription)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		unsigned char *__emu_buffer = in->pBuffer;

		char *InBuffer = (char *)in->pBuffer;
		char *block_start = InBuffer;

		InBuffer += sizeof(TaskDescriptionHeader_Struct);
		uint32 title_size = strlen(InBuffer) + 1;
		InBuffer += title_size;
		InBuffer += sizeof(TaskDescriptionData1_Struct);
		uint32 description_size = strlen(InBuffer) + 1;
		InBuffer += description_size;
		InBuffer += sizeof(TaskDescriptionData2_Struct);

		uint32 reward_size = strlen(InBuffer) + 1;
		InBuffer += reward_size;

		std::string old_message = InBuffer; // start item link string
		std::string new_message;
		ServerToSoFSayLink(new_message, old_message);

		in->size = sizeof(TaskDescriptionHeader_Struct) + sizeof(TaskDescriptionData1_Struct)+
			sizeof(TaskDescriptionData2_Struct) + sizeof(TaskDescriptionTrailer_Struct)+
			title_size + description_size + reward_size + new_message.length() + 1;

		in->pBuffer = new unsigned char[in->size];

		char *OutBuffer = (char *)in->pBuffer;

		memcpy(OutBuffer, block_start, (InBuffer - block_start));
		OutBuffer += (InBuffer - block_start);

		VARSTRUCT_ENCODE_STRING(OutBuffer, new_message.c_str());

		InBuffer += strlen(InBuffer) + 1;

		memcpy(OutBuffer, InBuffer, sizeof(TaskDescriptionTrailer_Struct));

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_Track)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		unsigned char *__emu_buffer = in->pBuffer;
		Track_Struct *emu = (Track_Struct *)__emu_buffer;

		int EntryCount = in->size / sizeof(Track_Struct);

		if (EntryCount == 0 || ((in->size % sizeof(Track_Struct))) != 0)
		{
			LogNetcode("[STRUCTS] Wrong size on outbound [{}]: Got [{}], expected multiple of [{}]", opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(Track_Struct));
			delete in;
			return;
		}

		in->size = sizeof(structs::Track_Struct) * EntryCount;
		in->pBuffer = new unsigned char[in->size];
		structs::Track_Struct *eq = (structs::Track_Struct *) in->pBuffer;

		for (int i = 0; i < EntryCount; ++i, ++eq, ++emu)
		{
			OUT(entityid);
			//OUT(padding002);
			OUT(distance);
		}

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_Trader)
	{
		if ((*p)->size != sizeof(TraderBuy_Struct)) {
			EQApplicationPacket *in = *p;
			*p = nullptr;
			dest->FastQueuePacket(&in, ack_req);
			return;
		}

		ENCODE_FORWARD(OP_TraderBuy);
	}

	ENCODE(OP_TraderBuy)
	{
		ENCODE_LENGTH_EXACT(TraderBuy_Struct);
		SETUP_DIRECT_ENCODE(TraderBuy_Struct, structs::TraderBuy_Struct);

		OUT(action);
		OUT(price);
		OUT(trader_id);
		memcpy(eq->item_name, emu->item_name, sizeof(eq->item_name));
		OUT(item_id);
		OUT(quantity);
		OUT(already_sold);

		FINISH_ENCODE();
	}

	ENCODE(OP_TributeItem)
	{
		ENCODE_LENGTH_EXACT(TributeItem_Struct);
		SETUP_DIRECT_ENCODE(TributeItem_Struct, structs::TributeItem_Struct);

		eq->slot = ServerToSoFSlot(emu->slot);
		OUT(quantity);
		OUT(tribute_master_id);
		OUT(tribute_points);

		FINISH_ENCODE();
	}

	ENCODE(OP_VetRewardsAvaliable)
	{
		EQApplicationPacket *inapp = *p;
		unsigned char * __emu_buffer = inapp->pBuffer;

		uint32 count = ((*p)->Size() / sizeof(InternalVeteranReward));
		*p = nullptr;

		auto outapp_create =
		    new EQApplicationPacket(OP_VetRewardsAvaliable, (sizeof(structs::VeteranReward) * count));
		uchar *old_data = __emu_buffer;
		uchar *data = outapp_create->pBuffer;
		for (uint32 i = 0; i < count; ++i)
		{
			structs::VeteranReward *vr = (structs::VeteranReward*)data;
			InternalVeteranReward *ivr = (InternalVeteranReward*)old_data;

			vr->claim_count = ivr->claim_count;
			vr->claim_id = ivr->claim_id;
			vr->number_available = ivr->number_available;
			for (int x = 0; x < 8; ++x)
			{
				vr->items[x].item_id = ivr->items[x].item_id;
				strcpy(vr->items[x].item_name, ivr->items[x].item_name);
				vr->items[x].charges = ivr->items[x].charges;
			}

			old_data += sizeof(InternalVeteranReward);
			data += sizeof(structs::VeteranReward);
		}

		dest->FastQueuePacket(&outapp_create);
		delete inapp;
	}

	ENCODE(OP_WearChange)
	{
		ENCODE_LENGTH_EXACT(WearChange_Struct);
		SETUP_DIRECT_ENCODE(WearChange_Struct, structs::WearChange_Struct);

		OUT(spawn_id);
		OUT(material);
		OUT(unknown06);
		OUT(elite_material);
		OUT(color.Color);
		OUT(wear_slot_id);

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
		__packet->WriteUInt8(1); // save items
		__packet->WriteUInt32(0); // hp
		__packet->WriteUInt32(0); // mana
		__packet->WriteUInt32(0); // endurance

		FINISH_ENCODE();
	}

	ENCODE(OP_ZoneServerInfo)
	{
		SETUP_DIRECT_ENCODE(ZoneServerInfo_Struct, ZoneServerInfo_Struct);

		OUT_str(ip);
		OUT(port);

		FINISH_ENCODE();

		//this is SUCH bullshit to be doing from down here. but the
		// new client requires us to close immediately following this
		// packet, so do it.
		//dest->Close();
	}

	ENCODE(OP_ZoneSpawns)
	{
		//consume the packet
		EQApplicationPacket *in = *p;
		*p = nullptr;

		//store away the emu struct
		unsigned char *__emu_buffer = in->pBuffer;
		Spawn_Struct *emu = (Spawn_Struct *)__emu_buffer;

		//determine and verify length
		int entrycount = in->size / sizeof(Spawn_Struct);
		if (entrycount == 0 || (in->size % sizeof(Spawn_Struct)) != 0) {
			LogNetcode("[STRUCTS] Wrong size on outbound [{}]: Got [{}], expected multiple of [{}]", opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(Spawn_Struct));
			delete in;
			return;
		}

		//make the EQ struct.
		in->size = sizeof(structs::Spawn_Struct)*entrycount;
		in->pBuffer = new unsigned char[in->size];
		structs::Spawn_Struct *eq = (structs::Spawn_Struct *) in->pBuffer;

		//zero out the packet. We could avoid this memset by setting all fields (including unknowns)
		//in the loop.
		memset(in->pBuffer, 0, in->size);

		//do the transform...
		int r;
		int k;
		for (r = 0; r < entrycount; r++, eq++, emu++) {

			eq->showname = emu->show_name ? 1 : 0; //New Field - Toggles Name Display on or off - 0 = off, 1 = on
			eq->linkdead = 0; //New Field - Toggles LD on or off after name - 0 = off, 1 = on
			eq->statue = 0; //New Field - 1 freezes animation
			eq->showhelm = emu->showhelm;
			eq->deity = emu->deity;
			eq->drakkin_heritage = emu->drakkin_heritage;
			eq->gender = emu->gender;
			for (k = EQ::textures::textureBegin; k < EQ::textures::materialCount; k++) {
				eq->equipment.Slot[k].Material = emu->equipment.Slot[k].Material;
				eq->equipment.Slot[k].Unknown1 = emu->equipment.Slot[k].Unknown1;
				eq->equipment.Slot[k].EliteMaterial = emu->equipment.Slot[k].EliteModel;
				eq->equipment_tint.Slot[k].Color = emu->equipment_tint.Slot[k].Color;
			}
			eq->StandState = emu->StandState;
			eq->guildID = emu->guildID;
			eq->spelleffect = 0;
			eq->spelleffect2 = 0;
			eq->spelleffect3 = 0;
			eq->spelleffect4 = 0;
			eq->spelleffect5 = 0;
			eq->spelleffect6 = 0;
			eq->class_ = emu->class_;
			eq->flymode = emu->flymode;
			eq->gm = emu->gm;
			eq->helm = emu->helm;
			eq->drakkin_tattoo = emu->drakkin_tattoo;
			eq->beardcolor = emu->beardcolor;
			eq->runspeed = emu->runspeed;
			eq->light = emu->light;
			eq->level = emu->level;
			eq->PlayerState = emu->PlayerState;
			eq->lfg = emu->lfg;
			eq->hairstyle = emu->hairstyle;
			eq->haircolor = emu->haircolor;
			eq->race = emu->race;
			strcpy(eq->suffix, emu->suffix);
			eq->findable = emu->findable;
			if (emu->bodytype >= 66)
			{
				eq->bodytype = 11;      //non-targetable
				eq->showname = 0;       //no visible name
				eq->race = 127;         //invisible man
				eq->gender = 0;         //invisible men are gender 0
			}
			else
			{
				eq->bodytype = emu->bodytype;
			}
			//eq->bodytype2 = 0;
			eq->equip_chest2 = emu->equip_chest2;
			eq->curHp = emu->curHp;
			eq->invis = emu->invis;
			strcpy(eq->lastName, emu->lastName);
			eq->eyecolor1 = emu->eyecolor1;
			strcpy(eq->title, emu->title);
			eq->beard = emu->beard;
			eq->targetable = 1; //New Field - Toggle Targetable on or off - 0 = off, 1 = on
			eq->NPC = emu->NPC;
			eq->targetable_with_hotkey = emu->targetable_with_hotkey ? 1 : 0; //New Field - Toggle Targetable on or off - 0 = off, 1 = on
			eq->x = emu->x;
			eq->deltaX = emu->deltaX;
			eq->deltaY = emu->deltaY;
			eq->z = emu->z;
			eq->deltaHeading = emu->deltaHeading;
			eq->y = emu->y;
			eq->deltaZ = emu->deltaZ;
			eq->animation = emu->animation;
			eq->heading = emu->heading;
			eq->spawnId = emu->spawnId;
			eq->nonvisible = 0;
			strcpy(eq->name, emu->name);
			eq->petOwnerId = emu->petOwnerId;
			eq->pvp = 0;	// 0 = non-pvp colored name, 1 = red pvp name
			for (k = EQ::textures::textureBegin; k < EQ::textures::materialCount; k++) {
				eq->equipment_tint.Slot[k].Color = emu->equipment_tint.Slot[k].Color;
			}
			eq->anon = emu->anon;
			eq->face = emu->face;
			eq->drakkin_details = emu->drakkin_details;
			eq->size = emu->size;
			eq->walkspeed = emu->walkspeed;
			/*
			//Uncomment this section to use this hack test with NPC last names
			//Hack Test for finding more fields in the Struct:
			if (emu->lastName[0] == '*') // Test NPC!
			{
			char code = emu->lastName[1];
			size_t len = strlen(emu->lastName);
			char* sep = (char*)memchr(&emu->lastName[2], '=', len - 2);

			uint32 ofs;
			uint32 val;
			uint8 rnd = rand() & 0x0F;
			if (sep == nullptr)
			{
			ofs = 0;
			if ((emu->lastName[2] < '0') || (emu->lastName[2] > '9'))
			{
			val = rnd;
			}
			else
			{
			val = Strings::ToInt(&emu->lastName[2]);
			}
			}
			else
			{
			sep[0] = nullptr;
			ofs = Strings::ToInt(&emu->lastName[2]);
			sep[0] = '=';
			if ((sep[1] < '0') || (sep[1] > '9'))
			{
			val = rnd;
			}
			else
			{
			val = Strings::ToInt(&sep[1]);
			}
			}

			char hex[] = "0123456789ABCDEF";

			eq->lastName[len + 0] = ' ';
			eq->lastName[len + 1] = code;
			eq->lastName[len + 2] = '0' + ((ofs / 1000) % 10);
			eq->lastName[len + 3] = '0' + ((ofs / 100) % 10);
			eq->lastName[len + 4] = '0' + ((ofs / 10) % 10);
			eq->lastName[len + 5] = '0' + (ofs % 10);
			eq->lastName[len + 6] = '=';
			eq->lastName[len + 7] = '0' + ((val / 100) % 10);
			eq->lastName[len + 8] = '0' + ((val / 10) % 10);
			eq->lastName[len + 9] = '0' + (val % 10);
			eq->lastName[len + 10] = 0x00;

			switch (code)
			{
			case 'a':
			eq->unknown0001[ofs % 4] = val; break;
			case 'b':
			eq->unknown0008 = val; break;
			case 'c':
			eq->unknown0011[ofs % 3] = val; break;
			case 'd':
			eq->unknown0018[ofs % 4] = val; break;
			case 'e':
			eq->unknown0023[ofs % 4] = val; break;
			case 'f':
			eq->unknown0136 = val; break;
			case 'g':
			eq->unknown0166[ofs % 8] = val; break;
			case 'h':
			eq->unknown0175[ofs % 192] = val; break;
			case 'i':
			eq->unknown0370[ofs % 3] = val; break;
			case 'j':
			eq->unknown0374[ofs % 128] = val; break;
			case 'k':
			eq->unknown0507[ofs % 4] = val; break;
			case 'l':
			eq->unknown0512[ofs % 16] = val; break;
			case 'm':
			eq->unknown0529[ofs % 4] = val; break;
			case 'n':
			eq->unknown0539[ofs % 41] = val; break;
			case 'o':
			eq->unknown0614[ofs % 11] = val; break;
			case 'p':
			eq->unknown0626[ofs % 28] = val; break;
			case 'q':
			eq->unknown0690 = val; break;
			case 'r':
			eq->unknown0726[ofs % 4] = val; break;
			case 's':
			eq->unknown0731[ofs % 11] = val; break;
			case 't':
			eq->unknown0767[ofs % 3] = val; break;
			case 'u':
			eq->unknown0883[ofs % 4] = val; break;
			case 'v':
			eq->unknown0895[ofs % 2] = val; break;
			case 'X':
			((uint8*)eq)[ofs % 897] = val; break;
			case 'Z':
			eq->size = (float)val; break; // Test w/ size.
			}
			}*/
		}

		//kill off the emu structure and send the eq packet.
		delete[] __emu_buffer;

		//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Sending zone spawns");
		//Log.Hex(Logs::Netcode, in->pBuffer, in->size);

		dest->FastQueuePacket(&in, ack_req);
	}

// DECODE methods
	DECODE(OP_AdventureMerchantSell)
	{
		DECODE_LENGTH_EXACT(structs::Adventure_Sell_Struct);
		SETUP_DIRECT_DECODE(Adventure_Sell_Struct, structs::Adventure_Sell_Struct);

		IN(npcid);
		emu->slot = SoFToServerSlot(eq->slot);
		IN(charges);
		IN(sell_price);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_AltCurrencySell)
	{
		DECODE_LENGTH_EXACT(structs::AltCurrencySellItem_Struct);
		SETUP_DIRECT_DECODE(AltCurrencySellItem_Struct, structs::AltCurrencySellItem_Struct);

		IN(merchant_entity_id);
		emu->slot_id = SoFToServerSlot(eq->slot_id);
		IN(charges);
		IN(cost);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_AltCurrencySellSelection)
	{
		DECODE_LENGTH_EXACT(structs::AltCurrencySelectItem_Struct);
		SETUP_DIRECT_DECODE(AltCurrencySelectItem_Struct, structs::AltCurrencySelectItem_Struct);

		IN(merchant_entity_id);
		emu->slot_id = SoFToServerSlot(eq->slot_id);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ApplyPoison)
	{
		DECODE_LENGTH_EXACT(structs::ApplyPoison_Struct);
		SETUP_DIRECT_DECODE(ApplyPoison_Struct, structs::ApplyPoison_Struct);

		emu->inventorySlot = SoFToServerSlot(eq->inventorySlot);
		IN(success);

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

		emu->container_slot = SoFToServerSlot(eq->container_slot);
		emu->augment_slot = eq->augment_slot;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_BookButton)
	{
		DECODE_LENGTH_EXACT(structs::BookButton_Struct);
		SETUP_DIRECT_DECODE(BookButton_Struct, structs::BookButton_Struct);

		emu->invslot = static_cast<int16_t>(SoFToServerSlot(eq->invslot));
		IN(target_id);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_Buff)
	{
		DECODE_LENGTH_EXACT(structs::SpellBuffPacket_Struct);
		SETUP_DIRECT_DECODE(SpellBuffPacket_Struct, structs::SpellBuffPacket_Struct);

		IN(entityid);
		IN(buff.effect_type);
		IN(buff.level);
		IN(buff.bard_modifier);
		IN(buff.spellid);
		IN(buff.duration);
		IN(buff.counters);
		IN(buff.player_id);
		emu->slotid = SoFToServerBuffSlot(eq->slotid);
		IN(bufffade);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_Bug)
	{
		DECODE_LENGTH_EXACT(structs::BugReport_Struct);
		SETUP_DIRECT_DECODE(BugReport_Struct, structs::BugReport_Struct);

		emu->category_id = Bug::GetID(eq->category_name);
		memcpy(emu->category_name, eq, sizeof(structs::BugReport_Struct));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_CastSpell)
	{
		DECODE_LENGTH_EXACT(structs::CastSpell_Struct);
		SETUP_DIRECT_DECODE(CastSpell_Struct, structs::CastSpell_Struct);

		emu->slot = static_cast<uint32>(SoFToServerCastingSlot(static_cast<spells::CastingSlot>(eq->slot), eq->inventoryslot));
		IN(spell_id);
		emu->inventoryslot = SoFToServerSlot(eq->inventoryslot);
		IN(target_id);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ChannelMessage)
	{
		unsigned char *__eq_buffer = __packet->pBuffer;

		std::string old_message = (char *)&__eq_buffer[sizeof(ChannelMessage_Struct)];
		std::string new_message;
		SoFToServerSayLink(new_message, old_message);

		__packet->size = sizeof(ChannelMessage_Struct) + new_message.length() + 1;
		__packet->pBuffer = new unsigned char[__packet->size];

		ChannelMessage_Struct *emu = (ChannelMessage_Struct *)__packet->pBuffer;

		memcpy(emu, __eq_buffer, sizeof(ChannelMessage_Struct));
		strcpy(emu->message, new_message.c_str());

		delete[] __eq_buffer;
	}

	DECODE(OP_CharacterCreate)
	{
		DECODE_LENGTH_EXACT(structs::CharCreate_Struct);
		SETUP_DIRECT_DECODE(CharCreate_Struct, structs::CharCreate_Struct);

		IN(class_);
		IN(beardcolor);
		IN(beard);
		IN(hairstyle);
		IN(gender);
		IN(race);
		IN(start_zone);
		IN(haircolor);
		IN(deity);
		IN(STR);
		IN(STA);
		IN(AGI);
		IN(DEX);
		IN(WIS);
		IN(INT);
		IN(CHA);
		IN(face);
		IN(eyecolor1);
		IN(eyecolor2);
		IN(tutorial);
		IN(drakkin_heritage);
		IN(drakkin_tattoo);
		IN(drakkin_details);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ClientUpdate)
	{
		// for some odd reason, there is an extra byte on the end of this on occasion..
		DECODE_LENGTH_ATLEAST(structs::PlayerPositionUpdateClient_Struct);
		SETUP_DIRECT_DECODE(PlayerPositionUpdateClient_Struct, structs::PlayerPositionUpdateClient_Struct);

		IN(spawn_id);
		IN(sequence);
		IN(x_pos);
		IN(y_pos);
		IN(z_pos);
		IN(heading);
		IN(delta_x);
		IN(delta_y);
		IN(delta_z);
		IN(delta_heading);
		IN(animation);
		emu->vehicle_id = 0;

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

	DECODE(OP_Consume)
	{
		DECODE_LENGTH_EXACT(structs::Consume_Struct);
		SETUP_DIRECT_DECODE(Consume_Struct, structs::Consume_Struct);

		emu->slot = SoFToServerSlot(eq->slot);
		IN(auto_consumed);
		IN(type);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_DeleteItem)
	{
		DECODE_LENGTH_EXACT(structs::DeleteItem_Struct);
		SETUP_DIRECT_DECODE(DeleteItem_Struct, structs::DeleteItem_Struct);

		emu->from_slot = SoFToServerSlot(eq->from_slot);
		emu->to_slot = SoFToServerSlot(eq->to_slot);
		IN(number_in_stack);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_DzAddPlayer)
	{
		DECODE_LENGTH_EXACT(structs::ExpeditionCommand_Struct);
		SETUP_DIRECT_DECODE(ExpeditionCommand_Struct, structs::ExpeditionCommand_Struct);

		strn0cpy(emu->name, eq->name, sizeof(emu->name));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_DzChooseZoneReply)
	{
		DECODE_LENGTH_EXACT(structs::DynamicZoneChooseZoneReply_Struct);
		SETUP_DIRECT_DECODE(DynamicZoneChooseZoneReply_Struct, structs::DynamicZoneChooseZoneReply_Struct);

		emu->unknown000 = eq->unknown000;
		emu->unknown008 = eq->unknown004;
		IN(unknown_id1);
		IN(dz_zone_id);
		IN(dz_instance_id);
		IN(dz_type);
		IN(unknown_id2);
		emu->unknown028 = eq->unknown024;
		emu->unknown032 = eq->unknown028;
		emu->unknown036 = eq->unknown032;
		emu->unknown040 = eq->unknown036;
		emu->unknown044 = eq->unknown040;
		emu->unknown048 = eq->unknown044;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_DzExpeditionInviteResponse)
	{
		DECODE_LENGTH_EXACT(structs::ExpeditionInviteResponse_Struct);
		SETUP_DIRECT_DECODE(ExpeditionInviteResponse_Struct, structs::ExpeditionInviteResponse_Struct);

		IN(dz_zone_id);
		IN(dz_instance_id);
		IN(accepted);
		IN(swapping);
		strn0cpy(emu->swap_name, eq->swap_name, sizeof(emu->swap_name));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_DzMakeLeader)
	{
		DECODE_LENGTH_EXACT(structs::ExpeditionCommand_Struct);
		SETUP_DIRECT_DECODE(ExpeditionCommand_Struct, structs::ExpeditionCommand_Struct);

		strn0cpy(emu->name, eq->name, sizeof(emu->name));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_DzRemovePlayer)
	{
		DECODE_LENGTH_EXACT(structs::ExpeditionCommand_Struct);
		SETUP_DIRECT_DECODE(ExpeditionCommand_Struct, structs::ExpeditionCommand_Struct);

		strn0cpy(emu->name, eq->name, sizeof(emu->name));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_DzSwapPlayer)
	{
		DECODE_LENGTH_EXACT(structs::ExpeditionCommandSwap_Struct);
		SETUP_DIRECT_DECODE(ExpeditionCommandSwap_Struct, structs::ExpeditionCommandSwap_Struct);

		strn0cpy(emu->add_player_name, eq->add_player_name, sizeof(emu->add_player_name));
		strn0cpy(emu->rem_player_name, eq->rem_player_name, sizeof(emu->rem_player_name));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_Emote)
	{
		unsigned char *__eq_buffer = __packet->pBuffer;

		std::string old_message = (char *)&__eq_buffer[4]; // unknown01 offset
		std::string new_message;
		SoFToServerSayLink(new_message, old_message);

		__packet->size = sizeof(Emote_Struct);
		__packet->pBuffer = new unsigned char[__packet->size];

		char *InBuffer = (char *)__packet->pBuffer;

		memcpy(InBuffer, __eq_buffer, 4);
		InBuffer += 4;
		strcpy(InBuffer, new_message.substr(0, 1023).c_str());
		InBuffer[1023] = '\0';

		delete[] __eq_buffer;
	}

	DECODE(OP_FaceChange)
	{
		DECODE_LENGTH_EXACT(structs::FaceChange_Struct);
		SETUP_DIRECT_DECODE(FaceChange_Struct, structs::FaceChange_Struct);

		IN(haircolor);
		IN(beardcolor);
		IN(eyecolor1);
		IN(eyecolor2);
		IN(hairstyle);
		IN(beard);
		IN(face);
		IN(drakkin_heritage);
		IN(drakkin_tattoo);
		IN(drakkin_details);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_FindPersonRequest)
	{
		DECODE_LENGTH_EXACT(structs::FindPersonRequest_Struct);
		SETUP_DIRECT_DECODE(FindPersonRequest_Struct, structs::FindPersonRequest_Struct);

		IN(npc_id);
		IN(client_pos.x);
		IN(client_pos.y);
		IN(client_pos.z);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GroupFollow)
	{
		DECODE_LENGTH_EXACT(structs::GroupFollow_Struct);
		SETUP_DIRECT_DECODE(GroupGeneric_Struct, structs::GroupFollow_Struct);

		memcpy(emu->name1, eq->name1, sizeof(emu->name1));
		memcpy(emu->name2, eq->name2, sizeof(emu->name2));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GroupFollow2)
	{
		DECODE_LENGTH_EXACT(structs::GroupFollow_Struct);
		SETUP_DIRECT_DECODE(GroupGeneric_Struct, structs::GroupFollow_Struct);

		memcpy(emu->name1, eq->name1, sizeof(emu->name1));
		memcpy(emu->name2, eq->name2, sizeof(emu->name2));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_InspectRequest)
	{
		DECODE_LENGTH_EXACT(structs::Inspect_Struct);
		SETUP_DIRECT_DECODE(Inspect_Struct, structs::Inspect_Struct);

		IN(TargetID);
		IN(PlayerID);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ItemLinkClick)
	{
		DECODE_LENGTH_EXACT(structs::ItemViewRequest_Struct);
		SETUP_DIRECT_DECODE(ItemViewRequest_Struct, structs::ItemViewRequest_Struct);

		MEMSET_IN(ItemViewRequest_Struct);

		IN(item_id);
		int r;
		for (r = 0; r < 5; r++) {
			IN(augments[r]);
		}
		IN(link_hash);
		IN(icon);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ItemVerifyRequest)
	{
		DECODE_LENGTH_EXACT(structs::ItemVerifyRequest_Struct);
		SETUP_DIRECT_DECODE(ItemVerifyRequest_Struct, structs::ItemVerifyRequest_Struct);

		emu->slot = SoFToServerSlot(eq->slot);
		IN(target);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_LoadSpellSet)
	{
		DECODE_LENGTH_EXACT(structs::LoadSpellSet_Struct);
		SETUP_DIRECT_DECODE(LoadSpellSet_Struct, structs::LoadSpellSet_Struct);

		for (int i = 0; i < spells::SPELL_GEM_COUNT; ++i)
			IN(spell[i]);
		for (int i = spells::SPELL_GEM_COUNT; i < EQ::spells::SPELL_GEM_COUNT; ++i)
			emu->spell[i] = 0xFFFFFFFF;

		IN(unknown);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_LootItem)
	{
		DECODE_LENGTH_EXACT(structs::LootingItem_Struct);
		SETUP_DIRECT_DECODE(LootingItem_Struct, structs::LootingItem_Struct);

		Log(Logs::Detail, Logs::Netcode, "SoF::DECODE(OP_LootItem)");

		IN(lootee);
		IN(looter);
		emu->slot_id = SoFToServerCorpseSlot(eq->slot_id);
		IN(auto_loot);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_MoveItem)
	{
		DECODE_LENGTH_EXACT(structs::MoveItem_Struct);
		SETUP_DIRECT_DECODE(MoveItem_Struct, structs::MoveItem_Struct);

		Log(Logs::Detail, Logs::Netcode, "SoF::DECODE(OP_MoveItem)");

		emu->from_slot = SoFToServerSlot(eq->from_slot);
		emu->to_slot = SoFToServerSlot(eq->to_slot);
		IN(number_in_stack);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_PetCommands)
	{
		DECODE_LENGTH_EXACT(structs::PetCommand_Struct);
		SETUP_DIRECT_DECODE(PetCommand_Struct, structs::PetCommand_Struct);

		switch (eq->command)
		{
		case 1: // back off
			emu->command = 28;
			break;
		case 2: // get lost
			emu->command = 29;
			break;
		case 3: // as you were ???
			emu->command = 4; // fuck it follow
			break;
		case 4: // report HP
			emu->command = 0;
			break;
		case 5: // guard here
			emu->command = 5;
			break;
		case 6: // guard me
			emu->command = 4; // fuck it follow
			break;
		case 7: // attack
			emu->command = 2;
			break;
		case 8: // follow
			emu->command = 4;
			break;
		case 9: // sit down
			emu->command = 7;
			break;
		case 10: // stand up
			emu->command = 8;
			break;
		case 11: // taunt toggle
			emu->command = 12;
			break;
		case 12: // hold toggle
			emu->command = 15;
			break;
		case 13: // taunt on
			emu->command = 13;
			break;
		case 14: // no taunt
			emu->command = 14;
			break;
		// 15 is target, doesn't send packet
		case 16: // leader
			emu->command = 1;
			break;
		case 17: // feign
			emu->command = 27;
			break;
		case 18: // no cast toggle
			emu->command = 21;
			break;
		case 19: // focus toggle
			emu->command = 24;
			break;
		default:
			emu->command = eq->command;
		}
		IN(target);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_RaidInvite)
	{
		DECODE_LENGTH_ATLEAST(structs::RaidGeneral_Struct);

		RaidGeneral_Struct* rgs = (RaidGeneral_Struct*)__packet->pBuffer;

		switch (rgs->action)
		{
		case raidSetMotd:
		{
			SETUP_VAR_DECODE(RaidMOTD_Struct, structs::RaidMOTD_Struct, motd);

			IN(general.action);
			IN(general.parameter);
			IN_str(general.leader_name);
			IN_str(general.player_name);
			IN_str(motd);

			FINISH_VAR_DECODE();
			break;
		}
		case raidSetNote:
		{
			SETUP_VAR_DECODE(RaidNote_Struct, structs::RaidNote_Struct, note);

			IN(general.action);
			IN(general.parameter);
			IN_str(general.leader_name);
			IN_str(general.player_name);
			IN_str(note);

			FINISH_VAR_DECODE();
			break;
		}
		default:
		{
			SETUP_DIRECT_DECODE(RaidGeneral_Struct, structs::RaidGeneral_Struct);
			IN(action);
			IN(parameter);
			IN_str(leader_name);
			IN_str(player_name);

			FINISH_DIRECT_DECODE();
			break;
		}
		}
	}

	DECODE(OP_ReadBook)
	{
		DECODE_LENGTH_EXACT(structs::BookRequest_Struct);
		SETUP_DIRECT_DECODE(BookRequest_Struct, structs::BookRequest_Struct);

		IN(type);
		emu->invslot = static_cast<int16_t>(SoFToServerSlot(eq->invslot));
		IN(target_id);
		emu->window = (uint8)eq->window;
		strn0cpy(emu->txtfile, eq->txtfile, sizeof(emu->txtfile));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_Save)
	{
		DECODE_LENGTH_EXACT(structs::Save_Struct);
		SETUP_DIRECT_DECODE(Save_Struct, structs::Save_Struct);

		memcpy(emu->unknown00, eq->unknown00, sizeof(emu->unknown00));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_SetServerFilter)
	{
		DECODE_LENGTH_EXACT(structs::SetServerFilter_Struct);
		SETUP_DIRECT_DECODE(SetServerFilter_Struct, structs::SetServerFilter_Struct);

		int r;
		for (r = 0; r < 29; r++) {
			IN(filters[r]);
		}

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ShopPlayerSell)
	{
		DECODE_LENGTH_EXACT(structs::Merchant_Purchase_Struct);
		SETUP_DIRECT_DECODE(Merchant_Purchase_Struct, structs::Merchant_Purchase_Struct);

		IN(npcid);
		emu->itemslot = SoFToServerSlot(eq->itemslot);
		IN(quantity);
		IN(price);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ShopRequest)
	{
		DECODE_LENGTH_EXACT(structs::MerchantClick_Struct);
		SETUP_DIRECT_DECODE(MerchantClick_Struct, structs::MerchantClick_Struct);

		IN(npc_id);
		IN(player_id);
		IN(command);
		IN(rate);
		emu->tab_display = 0;
		emu->unknown020 = 0;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_TraderBuy)
	{
		DECODE_LENGTH_EXACT(structs::TraderBuy_Struct);
		SETUP_DIRECT_DECODE(TraderBuy_Struct, structs::TraderBuy_Struct);
		MEMSET_IN(TraderBuy_Struct);

		IN(action);
		IN(price);
		IN(trader_id);
		memcpy(emu->item_name, eq->item_name, sizeof(emu->item_name));
		IN(item_id);
		IN(quantity);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_TradeSkillCombine)
	{
		DECODE_LENGTH_EXACT(structs::NewCombine_Struct);
		SETUP_DIRECT_DECODE(NewCombine_Struct, structs::NewCombine_Struct);

		emu->container_slot = SoFToServerSlot(eq->container_slot);
		IN(guildtribute_slot);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_TributeItem)
	{
		DECODE_LENGTH_EXACT(structs::TributeItem_Struct);
		SETUP_DIRECT_DECODE(TributeItem_Struct, structs::TributeItem_Struct);

		emu->slot = SoFToServerSlot(eq->slot);
		IN(quantity);
		IN(tribute_master_id);
		IN(tribute_points);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_WearChange)
	{
		DECODE_LENGTH_EXACT(structs::WearChange_Struct);
		SETUP_DIRECT_DECODE(WearChange_Struct, structs::WearChange_Struct);

		IN(spawn_id);
		IN(material);
		IN(unknown06);
		IN(elite_material);
		IN(color.Color);
		IN(wear_slot_id);
		emu->hero_forge_model = 0;
		emu->unknown18 = 0;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_WhoAllRequest)
	{
		DECODE_LENGTH_EXACT(structs::Who_All_Struct);
		SETUP_DIRECT_DECODE(Who_All_Struct, structs::Who_All_Struct);

		memcpy(emu->whom, eq->whom, sizeof(emu->whom));
		IN(wrace);
		IN(wclass);
		IN(lvllow);
		IN(lvlhigh);
		IN(gmlookup);
		IN(guildid);
		IN(type);

		FINISH_DIRECT_DECODE();
	}

// file scope helper methods
	uint32 NextItemInstSerialNumber = 1;
	uint32 MaxInstances = 2000000000;

	static inline int32 GetNextItemInstSerialNumber()
	{
		if (NextItemInstSerialNumber >= MaxInstances)
			NextItemInstSerialNumber = 1;
		else
			NextItemInstSerialNumber++;

		return NextItemInstSerialNumber;
	}

	void SerializeItem(EQ::OutBuffer& ob, const EQ::ItemInstance *inst, int16 slot_id_in, uint8 depth)
	{
		const EQ::ItemData *item = inst->GetUnscaledItem();

		SoF::structs::ItemSerializationHeader hdr;

		hdr.stacksize = (inst->IsStackable() ? ((inst->GetCharges() > 254) ? 0xFFFFFFFF : inst->GetCharges()) : 1);
		hdr.unknown004 = 0;

		int32 slot_id = ServerToSoFSlot(slot_id_in);

		hdr.slot = (inst->GetMerchantSlot() ? inst->GetMerchantSlot() : slot_id);
		hdr.price = inst->GetPrice();
		hdr.merchant_slot = (inst->GetMerchantSlot() ? inst->GetMerchantCount() : 1);
		hdr.scaled_value = (inst->IsScaling() ? (inst->GetExp() / 100) : 0);
		hdr.instance_id = (inst->GetMerchantSlot() ? inst->GetMerchantSlot() : inst->GetSerialNumber());
		hdr.unknown028 = 0;
		hdr.last_cast_time = inst->GetRecastTimestamp();
		hdr.charges = (inst->IsStackable() ? (item->MaxCharges ? 1 : 0) : ((inst->GetCharges() > 254) ? 0xFFFFFFFF : inst->GetCharges()));
		hdr.inst_nodrop = (inst->IsAttuned() ? 1 : 0);
		hdr.unknown044 = 0;
		hdr.unknown048 = 0;
		hdr.unknown052 = 0;
		hdr.unknown056 = 0;
		hdr.unknown060 = 0;
		hdr.unknown061 = 0;
		hdr.ItemClass = item->ItemClass;

		ob.write((const char*)&hdr, sizeof(SoF::structs::ItemSerializationHeader));

		if (strlen(item->Name) > 0)
			ob.write(item->Name, strlen(item->Name));
		ob.write("\0", 1);

		if (strlen(item->Lore) > 0)
			ob.write(item->Lore, strlen(item->Lore));
		ob.write("\0", 1);

		if (strlen(item->IDFile) > 0)
			ob.write(item->IDFile, strlen(item->IDFile));
		ob.write("\0", 1);

		SoF::structs::ItemBodyStruct ibs;
		memset(&ibs, 0, sizeof(SoF::structs::ItemBodyStruct));

		ibs.id = item->ID;
		// weight is uint8 in the struct, and some weights exceed that, so capping at 255.
		ibs.weight = ((item->Weight > 255) ? 255 : item->Weight);
		ibs.norent = item->NoRent;
		ibs.nodrop = item->NoDrop;
		ibs.attune = item->Attuneable;
		ibs.size = item->Size;
		ibs.slots = item->Slots;
		ibs.price = item->Price;
		ibs.icon = item->Icon;
		ibs.unknown1 = 1;
		ibs.unknown2 = 1;
		ibs.BenefitFlag = item->BenefitFlag;
		ibs.tradeskills = item->Tradeskills;
		ibs.CR = item->CR;
		ibs.DR = item->DR;
		ibs.PR = item->PR;
		ibs.MR = item->MR;
		ibs.FR = item->FR;
		ibs.SVCorruption = item->SVCorruption;
		ibs.AStr = item->AStr;
		ibs.ASta = item->ASta;
		ibs.AAgi = item->AAgi;
		ibs.ADex = item->ADex;
		ibs.ACha = item->ACha;
		ibs.AInt = item->AInt;
		ibs.AWis = item->AWis;

		ibs.HP = item->HP;
		ibs.Mana = item->Mana;
		ibs.Endur = item->Endur;
		ibs.AC = item->AC;
		ibs.regen = item->Regen;
		ibs.mana_regen = item->ManaRegen;
		ibs.end_regen = item->EnduranceRegen;
		ibs.Classes = item->Classes;
		ibs.Races = item->Races;
		ibs.Deity = item->Deity;
		ibs.SkillModValue = item->SkillModValue;
		ibs.SkillModMax = item->SkillModMax;
		ibs.SkillModType = item->SkillModType;
		ibs.BaneDmgRace = item->BaneDmgRace;
		ibs.BaneDmgBody = item->BaneDmgBody;
		ibs.BaneDmgRaceAmt = item->BaneDmgRaceAmt;
		ibs.BaneDmgAmt = item->BaneDmgAmt;
		ibs.Magic = item->Magic;
		ibs.CastTime_ = item->CastTime_;
		ibs.ReqLevel = item->ReqLevel;
		ibs.RecLevel = item->RecLevel;
		ibs.RecSkill = item->RecSkill;
		ibs.BardType = item->BardType;
		ibs.BardValue = item->BardValue;
		ibs.Light = item->Light;
		ibs.Delay = item->Delay;
		ibs.ElemDmgType = item->ElemDmgType;
		ibs.ElemDmgAmt = item->ElemDmgAmt;
		ibs.Range = item->Range;
		ibs.Damage = item->Damage;
		ibs.Color = item->Color;
		ibs.ItemType = item->ItemType;
		ibs.Material = item->Material;
		ibs.unknown7 = 0;
		ibs.EliteMaterial = item->EliteMaterial;
		ibs.SellRate = item->SellRate;

		ibs.CombatEffects = item->CombatEffects;
		ibs.Shielding = item->Shielding;
		ibs.StunResist = item->StunResist;
		ibs.StrikeThrough = item->StrikeThrough;
		ibs.ExtraDmgSkill = item->ExtraDmgSkill;
		ibs.ExtraDmgAmt = item->ExtraDmgAmt;
		ibs.SpellShield = item->SpellShield;
		ibs.Avoidance = item->Avoidance;
		ibs.Accuracy = item->Accuracy;
		ibs.CharmFileID = item->CharmFileID;
		ibs.FactionAmt1 = item->FactionAmt1;
		ibs.FactionMod1 = item->FactionMod1;
		ibs.FactionAmt2 = item->FactionAmt2;
		ibs.FactionMod2 = item->FactionMod2;
		ibs.FactionAmt3 = item->FactionAmt3;
		ibs.FactionMod3 = item->FactionMod3;
		ibs.FactionAmt4 = item->FactionAmt4;
		ibs.FactionMod4 = item->FactionMod4;

		ob.write((const char*)&ibs, sizeof(SoF::structs::ItemBodyStruct));

		//charm text
		if (strlen(item->CharmFile) > 0)
			ob.write((const char*)item->CharmFile, strlen(item->CharmFile));
		ob.write("\0", 1);

		SoF::structs::ItemSecondaryBodyStruct isbs;
		memset(&isbs, 0, sizeof(SoF::structs::ItemSecondaryBodyStruct));

		isbs.augtype = item->AugType;
		isbs.augrestrict = item->AugRestrict;

		for (int index = invaug::SOCKET_BEGIN; index <= invaug::SOCKET_END; ++index) {
			isbs.augslots[index].type = item->AugSlotType[index];
			isbs.augslots[index].visible = item->AugSlotVisible[index];
			isbs.augslots[index].unknown = item->AugSlotUnk2[index];
		}

		isbs.ldonpoint_type = item->PointType;
		isbs.ldontheme = item->LDoNTheme;
		isbs.ldonprice = item->LDoNPrice;
		isbs.ldonsellbackrate = item->LDoNSellBackRate;
		isbs.ldonsold = item->LDoNSold;

		isbs.bagtype = item->BagType;
		isbs.bagslots = item->BagSlots;
		isbs.bagsize = item->BagSize;
		isbs.wreduction = item->BagWR;

		isbs.book = item->Book;
		isbs.booktype = item->BookType;

		ob.write((const char*)&isbs, sizeof(SoF::structs::ItemSecondaryBodyStruct));

		if (strlen(item->Filename) > 0)
			ob.write((const char*)item->Filename, strlen(item->Filename));
		ob.write("\0", 1);

		SoF::structs::ItemTertiaryBodyStruct itbs;
		memset(&itbs, 0, sizeof(SoF::structs::ItemTertiaryBodyStruct));

		itbs.loregroup = item->LoreGroup;
		itbs.artifact = item->ArtifactFlag;
		itbs.summonedflag = item->SummonedFlag;
		itbs.favor = item->Favor;
		itbs.fvnodrop = item->FVNoDrop;
		itbs.dotshield = item->DotShielding;
		itbs.atk = item->Attack;
		itbs.haste = item->Haste;
		itbs.damage_shield = item->DamageShield;
		itbs.guildfavor = item->GuildFavor;
		itbs.augdistil = item->AugDistiller;
		itbs.no_pet = item->NoPet;

		itbs.potion_belt_enabled = item->PotionBelt;
		itbs.potion_belt_slots = item->PotionBeltSlots;
		itbs.stacksize = (inst->IsStackable() ? item->StackSize : 0);
		itbs.no_transfer = item->NoTransfer;
		itbs.expendablearrow = item->ExpendableArrow;

		// Done to hack older clients to label expendable fishing poles as such
		// July 28th, 2018 patch
		if (item->ItemType == EQ::item::ItemTypeFishingPole && item->SubType == 0) {
			itbs.expendablearrow = 1;
		}

		ob.write((const char*)&itbs, sizeof(SoF::structs::ItemTertiaryBodyStruct));

		// Effect Structures Broken down to allow variable length strings for effect names
		int32 effect_unknown = 0;

		SoF::structs::ClickEffectStruct ices;
		memset(&ices, 0, sizeof(SoF::structs::ClickEffectStruct));

		ices.effect = item->Click.Effect;
		ices.level2 = item->Click.Level2;
		ices.type = item->Click.Type;
		ices.level = item->Click.Level;
		ices.max_charges = item->MaxCharges;
		ices.cast_time = item->CastTime;
		ices.recast = item->RecastDelay;
		ices.recast_type = item->RecastType;

		ob.write((const char*)&ices, sizeof(SoF::structs::ClickEffectStruct));

		if (strlen(item->ClickName) > 0)
			ob.write((const char*)item->ClickName, strlen(item->ClickName));
		ob.write("\0", 1);

		ob.write((const char*)&effect_unknown, sizeof(int32));	// clickunk7

		SoF::structs::ProcEffectStruct ipes;
		memset(&ipes, 0, sizeof(SoF::structs::ProcEffectStruct));

		ipes.effect = item->Proc.Effect;
		ipes.level2 = item->Proc.Level2;
		ipes.type = item->Proc.Type;
		ipes.level = item->Proc.Level;
		ipes.procrate = item->ProcRate;

		ob.write((const char*)&ipes, sizeof(SoF::structs::ProcEffectStruct));

		if (strlen(item->ProcName) > 0)
			ob.write((const char*)item->ProcName, strlen(item->ProcName));
		ob.write("\0", 1);

		ob.write((const char*)&effect_unknown, sizeof(int32));	// unknown5

		SoF::structs::WornEffectStruct iwes;
		memset(&iwes, 0, sizeof(SoF::structs::WornEffectStruct));

		iwes.effect = item->Worn.Effect;
		iwes.level2 = item->Worn.Level2;
		iwes.type = item->Worn.Type;
		iwes.level = item->Worn.Level;

		ob.write((const char*)&iwes, sizeof(SoF::structs::WornEffectStruct));

		if (strlen(item->WornName) > 0)
			ob.write((const char*)item->WornName, strlen(item->WornName));
		ob.write("\0", 1);

		ob.write((const char*)&effect_unknown, sizeof(int32));	// unknown6

		SoF::structs::WornEffectStruct ifes;
		memset(&ifes, 0, sizeof(SoF::structs::WornEffectStruct));

		ifes.effect = item->Focus.Effect;
		ifes.level2 = item->Focus.Level2;
		ifes.type = item->Focus.Type;
		ifes.level = item->Focus.Level;

		ob.write((const char*)&ifes, sizeof(SoF::structs::WornEffectStruct));

		if (strlen(item->FocusName) > 0)
			ob.write((const char*)item->FocusName, strlen(item->FocusName));
		ob.write("\0", 1);

		ob.write((const char*)&effect_unknown, sizeof(int32));	// unknown6

		SoF::structs::WornEffectStruct ises;
		memset(&ises, 0, sizeof(SoF::structs::WornEffectStruct));

		ises.effect = item->Scroll.Effect;
		ises.level2 = item->Scroll.Level2;
		ises.type = item->Scroll.Type;
		ises.level = item->Scroll.Level;

		ob.write((const char*)&ises, sizeof(SoF::structs::WornEffectStruct));

		if (strlen(item->ScrollName) > 0)
			ob.write((const char*)item->ScrollName, strlen(item->ScrollName));
		ob.write("\0", 1);

		ob.write((const char*)&effect_unknown, sizeof(int32));	// unknown6
		// End of Effects

		SoF::structs::ItemQuaternaryBodyStruct iqbs;
		memset(&iqbs, 0, sizeof(SoF::structs::ItemQuaternaryBodyStruct));

		iqbs.scriptfileid = item->ScriptFileID;
		iqbs.quest_item = item->QuestItemFlag;
		iqbs.unknown15 = 0xffffffff;

		iqbs.Purity = item->Purity;
		iqbs.BackstabDmg = item->BackstabDmg;
		iqbs.DSMitigation = item->DSMitigation;
		iqbs.HeroicStr = item->HeroicStr;
		iqbs.HeroicInt = item->HeroicInt;
		iqbs.HeroicWis = item->HeroicWis;
		iqbs.HeroicAgi = item->HeroicAgi;
		iqbs.HeroicDex = item->HeroicDex;
		iqbs.HeroicSta = item->HeroicSta;
		iqbs.HeroicCha = item->HeroicCha;
		iqbs.HeroicMR = item->HeroicMR;
		iqbs.HeroicFR = item->HeroicFR;
		iqbs.HeroicCR = item->HeroicCR;
		iqbs.HeroicDR = item->HeroicDR;
		iqbs.HeroicPR = item->HeroicPR;
		iqbs.HeroicSVCorrup = item->HeroicSVCorrup;
		iqbs.HealAmt = item->HealAmt;
		iqbs.SpellDmg = item->SpellDmg;

		ob.write((const char*)&iqbs, sizeof(SoF::structs::ItemQuaternaryBodyStruct));

		EQ::OutBuffer::pos_type count_pos = ob.tellp();
		uint32 subitem_count = 0;

		ob.write((const char*)&subitem_count, sizeof(uint32));

		// moved outside of loop since it is not modified within that scope
		int16 SubSlotNumber = EQ::invbag::SLOT_INVALID;

		if (slot_id_in <= EQ::invslot::slotGeneral8 && slot_id_in >= EQ::invslot::GENERAL_BEGIN)
			SubSlotNumber = EQ::invbag::GENERAL_BAGS_BEGIN + ((slot_id_in - EQ::invslot::GENERAL_BEGIN) * EQ::invbag::SLOT_COUNT);
		else if (slot_id_in <= EQ::invslot::GENERAL_END && slot_id_in >= EQ::invslot::slotGeneral9)
			SubSlotNumber = EQ::invbag::SLOT_INVALID;
		else if (slot_id_in == EQ::invslot::slotCursor)
			SubSlotNumber = EQ::invbag::CURSOR_BAG_BEGIN;
		else if (slot_id_in <= EQ::invslot::BANK_END && slot_id_in >= EQ::invslot::BANK_BEGIN)
			SubSlotNumber = EQ::invbag::BANK_BAGS_BEGIN + ((slot_id_in - EQ::invslot::BANK_BEGIN) * EQ::invbag::SLOT_COUNT);
		else if (slot_id_in <= EQ::invslot::SHARED_BANK_END && slot_id_in >= EQ::invslot::SHARED_BANK_BEGIN)
			SubSlotNumber = EQ::invbag::SHARED_BANK_BAGS_BEGIN + ((slot_id_in - EQ::invslot::SHARED_BANK_BEGIN) * EQ::invbag::SLOT_COUNT);
		else
			SubSlotNumber = slot_id_in; // not sure if this is the best way to handle this..leaving for now

		if (SubSlotNumber != EQ::invbag::SLOT_INVALID) {
			for (uint32 index = EQ::invbag::SLOT_BEGIN; index <= EQ::invbag::SLOT_END; ++index) {
				EQ::ItemInstance* sub = inst->GetItem(index);
				if (!sub)
					continue;

				ob.write((const char*)&index, sizeof(uint32));

				SerializeItem(ob, sub, SubSlotNumber, (depth + 1));
				++subitem_count;
			}

			if (subitem_count)
				ob.overwrite(count_pos, (const char*)&subitem_count, sizeof(uint32));
		}
	}

	static inline uint32 ServerToSoFSlot(uint32 server_slot)
	{
		uint32 sof_slot = invslot::SLOT_INVALID;

		if (server_slot <= EQ::invslot::slotGeneral8) {
			sof_slot = server_slot;
		}

		else if (server_slot <= EQ::invslot::CORPSE_END && server_slot >= EQ::invslot::slotCursor) {
			sof_slot = server_slot - 2;
		}

		else if (server_slot <= EQ::invbag::GENERAL_BAGS_END && server_slot >= EQ::invbag::GENERAL_BAGS_BEGIN) {
			sof_slot = server_slot - (EQ::invbag::GENERAL_BAGS_BEGIN - invbag::GENERAL_BAGS_BEGIN) - ((EQ::invbag::SLOT_COUNT - invbag::SLOT_COUNT) * ((server_slot - EQ::invbag::GENERAL_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT));;
		}

		else if (server_slot <= EQ::invbag::CURSOR_BAG_END && server_slot >= EQ::invbag::CURSOR_BAG_BEGIN) {
			sof_slot = server_slot - (EQ::invbag::CURSOR_BAG_BEGIN - invbag::CURSOR_BAG_BEGIN);
		}

		else if (server_slot <= EQ::invslot::TRIBUTE_END && server_slot >= EQ::invslot::TRIBUTE_BEGIN) {
			sof_slot = server_slot;
		}

		else if (server_slot <= EQ::invslot::GUILD_TRIBUTE_END && server_slot >= EQ::invslot::GUILD_TRIBUTE_BEGIN) {
			sof_slot = server_slot;
		}

		else if (server_slot == EQ::invslot::SLOT_TRADESKILL_EXPERIMENT_COMBINE) {
			sof_slot = server_slot;
		}

		else if (server_slot <= EQ::invslot::BANK_END && server_slot >= EQ::invslot::BANK_BEGIN) {
			sof_slot = server_slot;
		}

		else if (server_slot <= EQ::invbag::BANK_BAGS_END && server_slot >= EQ::invbag::BANK_BAGS_BEGIN) {
			sof_slot = server_slot - (EQ::invbag::BANK_BAGS_BEGIN - invbag::BANK_BAGS_BEGIN) - ((EQ::invbag::SLOT_COUNT - invbag::SLOT_COUNT) * ((server_slot - EQ::invbag::BANK_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT));
		}

		else if (server_slot <= EQ::invslot::SHARED_BANK_END && server_slot >= EQ::invslot::SHARED_BANK_BEGIN) {
			sof_slot = server_slot;
		}

		else if (server_slot <= EQ::invbag::SHARED_BANK_BAGS_END && server_slot >= EQ::invbag::SHARED_BANK_BAGS_BEGIN) {
			sof_slot = server_slot - (EQ::invbag::SHARED_BANK_BAGS_BEGIN - invbag::SHARED_BANK_BAGS_BEGIN) - ((EQ::invbag::SLOT_COUNT - invbag::SLOT_COUNT) * ((server_slot - EQ::invbag::SHARED_BANK_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT));
		}

		else if (server_slot <= EQ::invslot::TRADE_END && server_slot >= EQ::invslot::TRADE_BEGIN) {
			sof_slot = server_slot;
		}

		else if (server_slot <= EQ::invbag::TRADE_BAGS_END && server_slot >= EQ::invbag::TRADE_BAGS_BEGIN) {
			sof_slot = server_slot - (EQ::invbag::TRADE_BAGS_BEGIN - invbag::TRADE_BAGS_BEGIN) - ((EQ::invbag::SLOT_COUNT - invbag::SLOT_COUNT) * ((server_slot - EQ::invbag::TRADE_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT));
		}

		else if (server_slot <= EQ::invslot::WORLD_END && server_slot >= EQ::invslot::WORLD_BEGIN) {
			sof_slot = server_slot;
		}

		LogNetcode("Convert Server Slot [{}] to SoF Slot [{}]", server_slot, sof_slot);

		return sof_slot;
	}

	static inline uint32 ServerToSoFCorpseSlot(uint32 server_corpse_slot)
	{
		uint32 SoFSlot = invslot::SLOT_INVALID;

		if (server_corpse_slot <= EQ::invslot::slotGeneral8 && server_corpse_slot >= EQ::invslot::slotGeneral1) {
			SoFSlot = server_corpse_slot;
		}

		else if (server_corpse_slot <= EQ::invslot::CORPSE_END && server_corpse_slot >= EQ::invslot::slotCursor) {
			SoFSlot = server_corpse_slot - 2;
		}

		Log(Logs::Detail,
			Logs::Netcode,
			"Convert Server Corpse Slot %i to SoF Corpse Slot %i",
			server_corpse_slot,
			SoFSlot);

		return SoFSlot;
	}

	static inline uint32 SoFToServerSlot(uint32 sof_slot)
	{
		uint32 server_slot = EQ::invslot::SLOT_INVALID;

		if (sof_slot <= invslot::slotGeneral8) {
			server_slot = sof_slot;
		}

		else if (sof_slot <= invslot::CORPSE_END && sof_slot >= invslot::slotCursor) {
			server_slot = sof_slot + 2;
		}

		else if (sof_slot <= invbag::GENERAL_BAGS_END && sof_slot >= invbag::GENERAL_BAGS_BEGIN) {
			server_slot = sof_slot + (EQ::invbag::GENERAL_BAGS_BEGIN - invbag::GENERAL_BAGS_BEGIN) + ((EQ::invbag::SLOT_COUNT - invbag::SLOT_COUNT) * ((sof_slot - invbag::GENERAL_BAGS_BEGIN) / invbag::SLOT_COUNT));;
		}

		else if (sof_slot <= invbag::CURSOR_BAG_END && sof_slot >= invbag::CURSOR_BAG_BEGIN) {
			server_slot = sof_slot + (EQ::invbag::CURSOR_BAG_BEGIN - invbag::CURSOR_BAG_BEGIN);
		}

		else if (sof_slot <= invslot::TRIBUTE_END && sof_slot >= invslot::TRIBUTE_BEGIN) {
			server_slot = sof_slot;
		}

		else if (sof_slot <= invslot::GUILD_TRIBUTE_END && sof_slot >= invslot::GUILD_TRIBUTE_BEGIN) {
			server_slot = sof_slot;
		}

		else if (sof_slot == invslot::SLOT_TRADESKILL_EXPERIMENT_COMBINE) {
			server_slot = sof_slot;
		}

		else if (sof_slot <= invslot::BANK_END && sof_slot >= invslot::BANK_BEGIN) {
			server_slot = sof_slot;
		}

		else if (sof_slot <= invbag::BANK_BAGS_END && sof_slot >= invbag::BANK_BAGS_BEGIN) {
			server_slot = sof_slot + (EQ::invbag::BANK_BAGS_BEGIN - invbag::BANK_BAGS_BEGIN) + ((EQ::invbag::SLOT_COUNT - invbag::SLOT_COUNT) * ((sof_slot - invbag::BANK_BAGS_BEGIN) / invbag::SLOT_COUNT));;
		}

		else if (sof_slot <= invslot::SHARED_BANK_END && sof_slot >= invslot::SHARED_BANK_BEGIN) {
			server_slot = sof_slot;
		}

		else if (sof_slot <= invbag::SHARED_BANK_BAGS_END && sof_slot >= invbag::SHARED_BANK_BAGS_BEGIN) {
			server_slot = sof_slot + (EQ::invbag::SHARED_BANK_BAGS_BEGIN - invbag::SHARED_BANK_BAGS_BEGIN) + ((EQ::invbag::SLOT_COUNT - invbag::SLOT_COUNT) * ((sof_slot - invbag::SHARED_BANK_BAGS_BEGIN) / invbag::SLOT_COUNT));;
		}

		else if (sof_slot <= invslot::TRADE_END && sof_slot >= invslot::TRADE_BEGIN) {
			server_slot = sof_slot;
		}

		else if (sof_slot <= invbag::TRADE_BAGS_END && sof_slot >= invbag::TRADE_BAGS_BEGIN) {
			server_slot = sof_slot + (EQ::invbag::TRADE_BAGS_BEGIN - invbag::TRADE_BAGS_BEGIN) + ((EQ::invbag::SLOT_COUNT - invbag::SLOT_COUNT) * ((sof_slot - invbag::TRADE_BAGS_BEGIN) / invbag::SLOT_COUNT));;
		}

		else if (sof_slot <= invslot::WORLD_END && sof_slot >= invslot::WORLD_BEGIN) {
			server_slot = sof_slot;
		}

		LogNetcode("Convert SoF Slot [{}] to Server Slot [{}]", sof_slot, server_slot);

		return server_slot;
	}

	static inline uint32 SoFToServerCorpseSlot(uint32 sof_corpse_slot)
	{
		uint32 server_slot = EQ::invslot::SLOT_INVALID;

		if (sof_corpse_slot <= invslot::slotGeneral8 && sof_corpse_slot >= invslot::slotGeneral1) {
			server_slot = sof_corpse_slot;
		}

		else if (sof_corpse_slot <= invslot::CORPSE_END && sof_corpse_slot >= invslot::slotCursor) {
			server_slot = sof_corpse_slot + 2;
		}

		Log(Logs::Detail,
			Logs::Netcode,
			"Convert SoF Corpse Slot %i to Server Corpse Slot %i",
			sof_corpse_slot,
			server_slot);

		return server_slot;
	}

	static inline void ServerToSoFSayLink(std::string &sof_saylink, const std::string &server_saylink)
	{
		if ((constants::SAY_LINK_BODY_SIZE == EQ::constants::SAY_LINK_BODY_SIZE) || (server_saylink.find('\x12') == std::string::npos)) {
			sof_saylink = server_saylink;
			return;
		}

		auto segments = Strings::Split(server_saylink, '\x12');

		for (size_t segment_iter = 0; segment_iter < segments.size(); ++segment_iter) {
			if (segment_iter & 1) {
				if (segments[segment_iter].length() <= EQ::constants::SAY_LINK_BODY_SIZE) {
					sof_saylink.append(segments[segment_iter]);
					// TODO: log size mismatch error
					continue;
				}

				// Idx:  0 1     6     11    16    21    26    31    36 37   41 43    48       (Source)
				// RoF2: X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX X  XXXX XX XXXXX XXXXXXXX (56)
				// SoF:  X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX       X  XXXX  X XXXXX XXXXXXXX (50)
				// Diff:                                       ^^^^^         ^

				sof_saylink.push_back('\x12');
				sof_saylink.append(segments[segment_iter].substr(0, 31));
				sof_saylink.append(segments[segment_iter].substr(36, 5));

				if (segments[segment_iter][41] == '0')
					sof_saylink.push_back(segments[segment_iter][42]);
				else
					sof_saylink.push_back('F');

				sof_saylink.append(segments[segment_iter].substr(43));
				sof_saylink.push_back('\x12');
			}
			else {
				sof_saylink.append(segments[segment_iter]);
			}
		}
	}

	static inline void SoFToServerSayLink(std::string &server_saylink, const std::string &sof_saylink)
	{
		if ((EQ::constants::SAY_LINK_BODY_SIZE == constants::SAY_LINK_BODY_SIZE) || (sof_saylink.find('\x12') == std::string::npos)) {
			server_saylink = sof_saylink;
			return;
		}

		auto segments = Strings::Split(sof_saylink, '\x12');

		for (size_t segment_iter = 0; segment_iter < segments.size(); ++segment_iter) {
			if (segment_iter & 1) {
				if (segments[segment_iter].length() <= constants::SAY_LINK_BODY_SIZE) {
					server_saylink.append(segments[segment_iter]);
					// TODO: log size mismatch error
					continue;
				}

				// Idx:  0 1     6     11    16    21    26          31 32    36 37    42       (Source)
				// SoF:  X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX       X  XXXX  X  XXXXX XXXXXXXX (50)
				// RoF2: X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX X  XXXX XX  XXXXX XXXXXXXX (56)
				// Diff:                                       ^^^^^         ^

				server_saylink.push_back('\x12');
				server_saylink.append(segments[segment_iter].substr(0, 31));
				server_saylink.append("00000");
				server_saylink.append(segments[segment_iter].substr(31, 5));
				server_saylink.push_back('0');
				server_saylink.append(segments[segment_iter].substr(36));
				server_saylink.push_back('\x12');
			}
			else {
				server_saylink.append(segments[segment_iter]);
			}
		}
	}

	static inline spells::CastingSlot ServerToSoFCastingSlot(EQ::spells::CastingSlot slot) {
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
			case EQ::spells::CastingSlot::Item:
				return spells::CastingSlot::Item;
			case EQ::spells::CastingSlot::PotionBelt:
				return spells::CastingSlot::PotionBelt;
			case EQ::spells::CastingSlot::Discipline:
				return spells::CastingSlot::Discipline;
			case EQ::spells::CastingSlot::AltAbility:
				return spells::CastingSlot::AltAbility;
			default: // we shouldn't have any issues with other slots ... just return something
				return spells::CastingSlot::Discipline;
		}
	}

	static inline EQ::spells::CastingSlot SoFToServerCastingSlot(spells::CastingSlot slot, uint32 item_location) {
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
			case spells::CastingSlot::Ability:
				return EQ::spells::CastingSlot::Ability;
				// Tit uses 10 for item and discipline casting, but items have a valid location
			case spells::CastingSlot::Item:
				if (item_location == INVALID_INDEX)
					return EQ::spells::CastingSlot::Discipline;
				else
					return EQ::spells::CastingSlot::Item;
			case spells::CastingSlot::PotionBelt:
				return EQ::spells::CastingSlot::PotionBelt;
			case spells::CastingSlot::AltAbility:
				return EQ::spells::CastingSlot::AltAbility;
			default: // we shouldn't have any issues with other slots ... just return something
				return EQ::spells::CastingSlot::Discipline;
		}
	}

	static inline int ServerToSoFBuffSlot(int index) {
		// we're a disc
		if (index >= EQ::spells::LONG_BUFFS + EQ::spells::SHORT_BUFFS)
			return index - EQ::spells::LONG_BUFFS - EQ::spells::SHORT_BUFFS +
				   spells::LONG_BUFFS + spells::SHORT_BUFFS;
		// we're a song
		if (index >= EQ::spells::LONG_BUFFS)
			return index - EQ::spells::LONG_BUFFS + spells::LONG_BUFFS;
		// we're a normal buff
		return index; // as long as we guard against bad slots server side, we should be fine
	}

	static inline int SoFToServerBuffSlot(int index)
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
}
