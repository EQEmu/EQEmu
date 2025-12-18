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

#include "titanium.h"
#include "titanium_structs.h"

#include "common/crc32.h"
#include "common/eq_packet_structs.h"
#include "common/eq_stream_ident.h"
#include "common/eqemu_config.h"
#include "common/eqemu_logsys.h"
#include "common/guilds.h"
#include "common/item_instance.h"
#include "common/misc_functions.h"
#include "common/opcodemgr.h"
#include "common/path_manager.h"
#include "common/races.h"
#include "common/raid.h"
#include "common/rulesys.h"
#include "common/strings.h"

#include <sstream>


namespace Titanium
{
	static const char *name = "Titanium";
	static OpcodeManager *opcodes = nullptr;
	static Strategy struct_strategy;

	void SerializeItem(EQ::OutBuffer& ob, const EQ::ItemInstance *inst, int16 slot_id_in, uint8 depth);

	// server to client inventory location converters
	static inline int16 ServerToTitaniumSlot(uint32 server_slot);
	static inline int16 ServerToTitaniumCorpseSlot(uint32 server_corpse_slot);

	// client to server inventory location converters
	static inline uint32 TitaniumToServerSlot(int16 titanium_slot);
	static inline uint32 TitaniumToServerCorpseSlot(int16 titanium_corpse_slot);

	// server to client say link converter
	static inline void ServerToTitaniumSayLink(std::string &titanium_saylink, const std::string &server_saylink);

	// client to server say link converter
	static inline void TitaniumToServerSayLink(std::string &server_saylink, const std::string &titanium_saylink);

	static inline spells::CastingSlot ServerToTitaniumCastingSlot(EQ::spells::CastingSlot slot);
	static inline EQ::spells::CastingSlot TitaniumToServerCastingSlot(spells::CastingSlot slot, uint32 item_location);

	static inline int ServerToTitaniumBuffSlot(int index);
	static inline int TitaniumToServerBuffSlot(int index);

	void Register(EQStreamIdentifier &into)
	{
		auto Config = EQEmuConfig::get();
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
#include "titanium_ops.h"
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
		return EQ::versions::ClientVersion::Titanium;
	}

#include "ss_define.h"

// ENCODE methods
	EAT_ENCODE(OP_GuildMemberLevelUpdate); // added ;

	EAT_ENCODE(OP_ZoneServerReady); // added ;

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
		eq->slot = ServerToTitaniumSlot(emu->slot);
		OUT(charges);
		OUT(sell_price);

		FINISH_ENCODE();
	}

	ENCODE(OP_ApplyPoison)
	{
		ENCODE_LENGTH_EXACT(ApplyPoison_Struct);
		SETUP_DIRECT_ENCODE(ApplyPoison_Struct, structs::ApplyPoison_Struct);

		eq->inventorySlot = ServerToTitaniumSlot(emu->inventorySlot);
		OUT(success);

		FINISH_ENCODE();
	}

	ENCODE(OP_BazaarSearch)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		uint32 action = *(uint32 *)in->pBuffer;

		switch (action) {
			case BazaarSearch: {
				LogTrading(
					"Encode OP_BazaarSearch(Ti) BazaarSearch action <green>[{}]",
					action
				);
				std::vector<BazaarSearchResultsFromDB_Struct> results {};
				auto bsms = (BazaarSearchMessaging_Struct *)in->pBuffer;
				EQ::Util::MemoryStreamReader ss(
					reinterpret_cast<char *>(bsms->payload),
					in->size - sizeof(BazaarSearchMessaging_Struct)
				);
				cereal::BinaryInputArchive ar(ss);
				ar(results);

				auto  size    = results.size() * sizeof(structs::BazaarSearchResults_Struct);
				auto  buffer  = new uchar[size];
				uchar *bufptr = buffer;
				memset(buffer, 0, size);

				for (auto row = results.begin(); row != results.end(); ++row) {
					VARSTRUCT_ENCODE_TYPE(uint32, bufptr, structs::TiBazaarTraderBuyerActions::BazaarSearch);
					VARSTRUCT_ENCODE_TYPE(uint32, bufptr, row->trader_entity_id);
					bufptr += 4;
					VARSTRUCT_ENCODE_TYPE(int32, bufptr, row->item_id);
					VARSTRUCT_ENCODE_TYPE(int32, bufptr, row->serial_number);
					bufptr += 4;
					if (row->stackable) {
						strn0cpy(
							reinterpret_cast<char *>(bufptr),
							fmt::format("{}({})", row->item_name.c_str(), row->charges).c_str(),
							64
						);
					}
					else {
						strn0cpy(
							reinterpret_cast<char *>(bufptr),
							fmt::format("{}({})", row->item_name.c_str(), row->count).c_str(),
							64
						);
					}
					bufptr += 64;
					VARSTRUCT_ENCODE_TYPE(uint32, bufptr, row->cost);
					VARSTRUCT_ENCODE_TYPE(uint32, bufptr, row->item_stat);
				}

				auto outapp = new EQApplicationPacket(OP_BazaarSearch, size);
				memcpy(outapp->pBuffer, buffer, size);
				dest->FastQueuePacket(&outapp);

				safe_delete(outapp);
				safe_delete_array(buffer);
				safe_delete(in);
				break;
			}
			case BazaarInspect:
			case WelcomeMessage: {
				LogTrading(
					"Encode OP_BazaarSearch(Ti) BazaarInspect/WelcomeMessage action <green>[{}]",
					action
				);
				dest->FastQueuePacket(&in, ack_req);
				break;
			}
			default: {
				LogTrading(
					"Encode OP_BazaarSearch(Ti) unhandled action <red>[{}]",
					action
				);
				dest->FastQueuePacket(&in, ack_req);
			}
		}
	}

	ENCODE(OP_BecomeTrader)
	{
		uint32 action = *(uint32 *)(*p)->pBuffer;

		switch (action)
		{
			case TraderOff:
			{
				ENCODE_LENGTH_EXACT(BecomeTrader_Struct);
				SETUP_DIRECT_ENCODE(BecomeTrader_Struct, structs::BecomeTrader_Struct);
				LogTrading(
					"Encode OP_BecomeTrader(Ti) TraderOff action <green>[{}] entity_id <green>[{}] trader_name "
					"<green>[{}]",
					emu->action,
					emu->entity_id,
					emu->trader_name
				);
				eq->action    = structs::TiBazaarTraderBuyerActions::Zero;
				eq->entity_id = emu->entity_id;
				FINISH_ENCODE();
				break;
			}
			case TraderOn:
			{
				ENCODE_LENGTH_EXACT(BecomeTrader_Struct);
				SETUP_DIRECT_ENCODE(BecomeTrader_Struct, structs::BecomeTrader_Struct);
				LogTrading(
					"Encode OP_BecomeTrader(Ti) TraderOn action <green>[{}] entity_id <green>[{}] trader_name "
					"<green>[{}]",
					emu->action,
					emu->entity_id,
					emu->trader_name
				);
				eq->action    = structs::TiBazaarTraderBuyerActions::BeginTraderMode;
				eq->entity_id = emu->entity_id;
				strn0cpy(eq->trader_name, emu->trader_name, sizeof(eq->trader_name));
				FINISH_ENCODE();
				break;
			}
			default:
			{
				LogTrading(
					"Encode OP_BecomeTrader(Ti) unhandled action <red>[{}] Sending packet as is.",
					action
				);
				EQApplicationPacket *in = *p;
				*p = nullptr;
				dest->FastQueuePacket(&in, ack_req);
			}
		}
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
		eq->slotid = ServerToTitaniumBuffSlot(emu->slotid);
		OUT(bufffade);

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
		ServerToTitaniumSayLink(new_message, old_message);

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

		//store away the emu struct
		uchar* __emu_buffer = in->pBuffer;

		int itemcount = in->size / sizeof(EQ::InternalSerializedItem_Struct);
		if (itemcount == 0 || (in->size % sizeof(EQ::InternalSerializedItem_Struct)) != 0) {
			Log(Logs::General, Logs::Netcode, "[STRUCTS] Wrong size on outbound %s: Got %d, expected multiple of %d",
				opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(EQ::InternalSerializedItem_Struct));
			delete in;
			return;
		}

		EQ::InternalSerializedItem_Struct* eq = (EQ::InternalSerializedItem_Struct*)in->pBuffer;

		//do the transform...
		EQ::OutBuffer ob;
		EQ::OutBuffer::pos_type last_pos = ob.tellp();

		for (int r = 0; r < itemcount; r++, eq++) {
			SerializeItem(ob, (const EQ::ItemInstance*)eq->inst, ServerToTitaniumSlot(eq->slot_id), 0);
			if (ob.tellp() == last_pos)
				LogNetcode("Titanium::ENCODE(OP_CharInventory) Serialization failed on item slot [{}] during OP_CharInventory.  Item skipped", eq->slot_id);

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
		Log(Logs::Detail, Logs::Netcode, "Titanium::ENCODE(OP_DeleteCharge)");

		ENCODE_FORWARD(OP_MoveItem);
	}

	ENCODE(OP_DeleteItem)
	{
		ENCODE_LENGTH_EXACT(DeleteItem_Struct);
		SETUP_DIRECT_ENCODE(DeleteItem_Struct, structs::DeleteItem_Struct);

		eq->from_slot = ServerToTitaniumSlot(emu->from_slot);
		eq->to_slot = ServerToTitaniumSlot(emu->to_slot);
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
		ServerToTitaniumSayLink(new_message, old_message);

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
			ServerToTitaniumSayLink(new_message_array[i], old_message_array[i]);
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
		// We are not encoding the spawn_id field here, but it doesn't appear to matter.
		//
		EQApplicationPacket *in = *p;
		*p = nullptr;

		//store away the emu struct
		unsigned char *__emu_buffer = in->pBuffer;
		Object_Struct *emu = (Object_Struct *)__emu_buffer;

		in->size = strlen(emu->object_name) + sizeof(structs::Object_Struct) - 1;
		in->pBuffer = new unsigned char[in->size];

		structs::Object_Struct *eq = (structs::Object_Struct *) in->pBuffer;

		eq->drop_id = emu->drop_id;
		eq->heading = emu->heading;
		eq->linked_list_addr[0] = 0;
		eq->linked_list_addr[1] = 0;
		strcpy(eq->object_name, emu->object_name);
		eq->object_type = emu->object_type;
		eq->spawn_id = 0;
		eq->unknown008[0] = 0;
		eq->unknown008[1] = 0;
		eq->unknown020 = 0;
		eq->unknown024 = 0;
		eq->unknown076 = 0;
		eq->unknown084 = 0xffffffff;
		eq->z = emu->z;
		eq->x = emu->x;
		eq->y = emu->y;
		eq->zone_id = emu->zone_id;
		eq->zone_instance = emu->zone_instance;


		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_SetGuildRank)
	{
		ENCODE_LENGTH_EXACT(GuildSetRank_Struct);
		SETUP_DIRECT_ENCODE(GuildSetRank_Struct, structs::GuildSetRank_Struct);

		eq->unknown00 = 0;
		eq->unknown04 = 0;

		//Translate older ranks to new values* /
		switch (emu->rank) {
			case GUILD_SENIOR_MEMBER:
			case GUILD_MEMBER:
			case GUILD_JUNIOR_MEMBER:
			case GUILD_INITIATE:
			case GUILD_RECRUIT: {
				eq->rank = GUILD_MEMBER_TI;
				break;
			}
			case GUILD_OFFICER:
			case GUILD_SENIOR_OFFICER: {
				eq->rank = GUILD_OFFICER_TI;
				break;
			}
			case GUILD_LEADER: {
				eq->rank = GUILD_LEADER_TI;
				break;
			}
			default: {
				eq->rank = GUILD_RANK_NONE_TI;
				break;
			}
		}

		memcpy(eq->member_name, emu->member_name, sizeof(eq->member_name));
		OUT(banker);

		FINISH_ENCODE();
	}

	ENCODE(OP_SpawnAppearance)
	{
		ENCODE_LENGTH_EXACT(SpawnAppearance_Struct);
		SETUP_DIRECT_ENCODE(SpawnAppearance_Struct, structs::SpawnAppearance_Struct);

		OUT(spawn_id);
		OUT(type);
		OUT(parameter);
		switch (emu->type) {
			case AppearanceType::GuildRank: {
				//Translate new ranks to old values* /
				switch (emu->parameter) {
					case GUILD_SENIOR_MEMBER:
					case GUILD_MEMBER:
					case GUILD_JUNIOR_MEMBER:
					case GUILD_INITIATE:
					case GUILD_RECRUIT: {
						eq->parameter = GUILD_MEMBER_TI;
						break;
					}
					case GUILD_OFFICER:
					case GUILD_SENIOR_OFFICER: {
						eq->parameter = GUILD_OFFICER_TI;
						break;
					}
					case GUILD_LEADER: {
						eq->parameter = GUILD_LEADER_TI;
						break;
					}
				}
				break;
			}
			case AppearanceType::GuildShow: {
				FAIL_ENCODE();
				return;
			}
			default: {
				break;
			}
		}

		FINISH_ENCODE();
	}

	ENCODE(OP_GuildsList)
	{
		EQApplicationPacket* in = *p;
		*p = nullptr;

		GuildsListMessaging_Struct   glms{};
		EQ::Util::MemoryStreamReader ss(reinterpret_cast<char *>(in->pBuffer), in->size);
		cereal::BinaryInputArchive   ar(ss);
		ar(glms);

		auto outapp = new EQApplicationPacket(OP_GuildsList, sizeof(structs::GuildsList_Struct));
		auto out    = (structs::GuildsList_Struct *) outapp->pBuffer;

		for (auto const& g : glms.guild_detail) {
			if (g.guild_id < Titanium::constants::MAX_GUILD_ID) {
				strn0cpy(out->Guilds[g.guild_id].name, g.guild_name.c_str(), sizeof(out->Guilds[g.guild_id].name));
			}
		}

		dest->FastQueuePacket(&outapp);
	}

	ENCODE(OP_GuildMemberAdd)
	{
		ENCODE_LENGTH_EXACT(GuildMemberAdd_Struct)
		SETUP_DIRECT_ENCODE(GuildMemberAdd_Struct, structs::GuildMemberAdd_Struct)

		OUT(guild_id)
		OUT(level)
		OUT(class_)
		switch (emu->rank_) {
			case GUILD_SENIOR_MEMBER:
			case GUILD_MEMBER:
			case GUILD_JUNIOR_MEMBER:
			case GUILD_INITIATE:
			case GUILD_RECRUIT: {
				eq->rank_ = GUILD_MEMBER_TI;
				break;
			}
			case GUILD_OFFICER:
			case GUILD_SENIOR_OFFICER: {
				eq->rank_ = GUILD_OFFICER_TI;
				break;
			}
			case GUILD_LEADER: {
				eq->rank_ = GUILD_LEADER_TI;
				break;
			}
		}
		OUT(zone_id)
		OUT(last_on)
		OUT_str(player_name)

		FINISH_ENCODE()
	}

	ENCODE(OP_GuildMemberRankAltBanker)
	{
		ENCODE_LENGTH_EXACT(GuildMemberRank_Struct)
		SETUP_DIRECT_ENCODE(GuildMemberRank_Struct, structs::GuildMemberRank_Struct)

		OUT(guild_id)
		OUT(alt_banker)
		OUT_str(player_name)

		switch (emu->rank_) {
			case GUILD_SENIOR_MEMBER:
			case GUILD_MEMBER:
			case GUILD_JUNIOR_MEMBER:
			case GUILD_INITIATE:
			case GUILD_RECRUIT: {
				eq->rank_ = GUILD_MEMBER_TI;
				break;
			}
			case GUILD_OFFICER:
			case GUILD_SENIOR_OFFICER: {
				eq->rank_ = GUILD_OFFICER_TI;
				break;
			}
			case GUILD_LEADER: {
				eq->rank_ = GUILD_LEADER_TI;
				break;
			}
		}

		FINISH_ENCODE()
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
				/* Translate new ranks to older values */
				SlideStructString(name, emu_name);
				PutFieldN(level);
				PutFieldN(banker);
				PutFieldN(class_);
				switch (emu_e->rank) {
					case GUILD_SENIOR_MEMBER:
					case GUILD_MEMBER:
					case GUILD_JUNIOR_MEMBER:
					case GUILD_INITIATE:
					case GUILD_RECRUIT: {
						e->rank = htonl(GUILD_MEMBER_TI);
						break;
					}
					case GUILD_OFFICER:
					case GUILD_SENIOR_OFFICER: {
						e->rank = htonl(GUILD_OFFICER_TI);
						break;
					}
					case GUILD_LEADER: {
						e->rank = htonl(GUILD_LEADER_TI);
						break;
					}
					default: {
						e->rank = htonl(GUILD_MEMBER_TI);
						break;
					}
				}
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

	ENCODE(OP_SendGuildTributes)
	{
		ENCODE_LENGTH_ATLEAST(structs::GuildTributeAbility_Struct);
		SETUP_VAR_ENCODE(GuildTributeAbility_Struct);
		ALLOC_VAR_ENCODE(structs::GuildTributeAbility_Struct, sizeof(GuildTributeAbility_Struct) + strlen(emu->ability.name));

		eq->guild_id = emu->guild_id;;
		strncpy(eq->ability.name, emu->ability.name, strlen(emu->ability.name));
		eq->ability.tribute_id = emu->ability.tribute_id;
		eq->ability.tier_count = emu->ability.tier_count;
		for (int i = 0; i < ntohl(emu->ability.tier_count); i++) {
			eq->ability.tiers[i].cost = emu->ability.tiers[i].cost;
			eq->ability.tiers[i].level = emu->ability.tiers[i].level;
			eq->ability.tiers[i].tribute_item_id = emu->ability.tiers[i].tribute_item_id;
		}
		FINISH_ENCODE();
	}

	ENCODE(OP_GuildTributeDonateItem)
	{
		SETUP_DIRECT_ENCODE(GuildTributeDonateItemReply_Struct, structs::GuildTributeDonateItemReply_Struct);

		Log(Logs::Detail, Logs::Netcode, "UF::ENCODE(OP_GuildTributeDonateItem)");

		OUT(quantity);
		OUT(favor);
		eq->unknown8 = 0;
		eq->slot = ServerToTitaniumSlot(emu->slot);

		FINISH_ENCODE();
	}

	ENCODE(OP_Illusion)
	{
		ENCODE_LENGTH_EXACT(Illusion_Struct);
		SETUP_DIRECT_ENCODE(Illusion_Struct, structs::Illusion_Struct);

		OUT(spawnid);
		OUT_str(charname);

		if (emu->race > 474)
			eq->race = 1;
		else
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
		/*
		//Test code for identifying the structure
		uint8 ofs;
		uint8 val;
		ofs = emu->texture;
		val = emu->face;
		((uint8*)eq)[ofs % 168] = val;
		*/

		FINISH_ENCODE();
	}

	ENCODE(OP_InspectAnswer)
	{
		ENCODE_LENGTH_EXACT(InspectResponse_Struct);
		SETUP_DIRECT_ENCODE(InspectResponse_Struct, structs::InspectResponse_Struct);

		OUT(TargetID);
		OUT(playerid);

		for (int i = EQ::invslot::slotCharm; i <= EQ::invslot::slotWaist; ++i) {
			strn0cpy(eq->itemnames[i], emu->itemnames[i], sizeof(eq->itemnames[i]));
			OUT(itemicons[i]);
		}

		// move ammo down to last element in titanium array
		strn0cpy(eq->itemnames[invslot::slotAmmo], emu->itemnames[EQ::invslot::slotAmmo], sizeof(eq->itemnames[invslot::slotAmmo]));
		eq->itemicons[invslot::slotAmmo] = emu->itemicons[EQ::invslot::slotAmmo];

		strn0cpy(eq->text, emu->text, sizeof(eq->text));

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

		SerializeItem(ob, (const EQ::ItemInstance*)int_struct->inst, ServerToTitaniumSlot(int_struct->slot_id), 0);
		if (ob.tellp() == last_pos) {
			LogNetcode("Titanium::ENCODE(OP_ItemPacket) Serialization failed on item slot [{}]", int_struct->slot_id);
			delete in;
			return;
		}

		in->size = ob.size();
		in->pBuffer = ob.detach();

		delete[] __emu_buffer;

		dest->FastQueuePacket(&in, ack_req);
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

	ENCODE(OP_LFGuild)
	{
		struct bit_mask_conversion {
			uint32	titanium_mask;
			uint32	rof2_mask;
		};

		std::vector<bit_mask_conversion> bit_mask = {
			{.titanium_mask = 2, .rof2_mask = 256},
			{.titanium_mask = 4, .rof2_mask = 32768},
			{.titanium_mask = 8, .rof2_mask = 65536},
			{.titanium_mask = 16, .rof2_mask = 4},
			{.titanium_mask = 32, .rof2_mask = 64},
			{.titanium_mask = 64, .rof2_mask = 16384},
			{.titanium_mask = 128, .rof2_mask = 8192},
			{.titanium_mask = 256, .rof2_mask = 128},
			{.titanium_mask = 512, .rof2_mask = 2048},
			{.titanium_mask = 1024, .rof2_mask = 8},
			{.titanium_mask = 2048, .rof2_mask = 16},
			{.titanium_mask = 4096, .rof2_mask = 512},
			{.titanium_mask = 8192, .rof2_mask = 32},
			{.titanium_mask = 16384, .rof2_mask = 1024},
			{.titanium_mask = 32768, .rof2_mask = 2},
			{.titanium_mask = 65536, .rof2_mask = 4096},
		};

		EQApplicationPacket *in = *p;

		uint32 Command = in->ReadUInt32();

		if (Command == 1) {
			ENCODE_LENGTH_EXACT(LFGuild_GuildToggle_Struct);
			SETUP_DIRECT_ENCODE(LFGuild_GuildToggle_Struct, structs::LFGuild_GuildToggle_Struct);

			OUT(Command);
			OUT_str(Comment);
			OUT(FromLevel);
			OUT(ToLevel);
			OUT(AACount);
			OUT(TimeZone);
			OUT(Toggle);
			OUT(TimePosted);
			OUT_str(Name);

			uint32 emu_bitmask = emu->Classes;
			uint32 ti_bitmask = 0;
			for (auto const& b : bit_mask) {
				(emu_bitmask & b.rof2_mask) != 0 ? ti_bitmask |= b.titanium_mask : ti_bitmask &= ~b.titanium_mask;
			}
			eq->Classes = ti_bitmask;

			FINISH_ENCODE();
			return;
		}

		*p = nullptr;
		if (Command != 0)
		{
			dest->FastQueuePacket(&in, ack_req);
			return;
		}

		auto outapp = new EQApplicationPacket(OP_LFGuild, sizeof(structs::LFGuild_PlayerToggle_Struct));

		memcpy(outapp->pBuffer, in->pBuffer, sizeof(structs::LFGuild_PlayerToggle_Struct));

		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	ENCODE(OP_LootItem)
	{
		ENCODE_LENGTH_EXACT(LootingItem_Struct);
		SETUP_DIRECT_ENCODE(LootingItem_Struct, structs::LootingItem_Struct);

		Log(Logs::Detail, Logs::Netcode, "Titanium::ENCODE(OP_LootItem)");

		OUT(lootee);
		OUT(looter);
		eq->slot_id = ServerToTitaniumCorpseSlot(emu->slot_id);
		OUT(auto_loot);

		FINISH_ENCODE();
	}

	ENCODE(OP_ManaChange)
	{
		ENCODE_LENGTH_EXACT(ManaChange_Struct);
		SETUP_DIRECT_ENCODE(ManaChange_Struct, structs::ManaChange_Struct);

		OUT(new_mana);
		OUT(stamina);
		OUT(spell_id);
		OUT(keepcasting);

		FINISH_ENCODE();
	}

	ENCODE(OP_MemorizeSpell)
	{
		ENCODE_LENGTH_EXACT(MemorizeSpell_Struct);
		SETUP_DIRECT_ENCODE(MemorizeSpell_Struct, structs::MemorizeSpell_Struct);

		// Since HT/LoH are translated up, we need to translate down only for memSpellSpellbar case
		if (emu->scribing == 3)
			eq->slot = static_cast<uint32>(ServerToTitaniumCastingSlot(static_cast<EQ::spells::CastingSlot>(emu->slot)));
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

		Log(Logs::Detail, Logs::Netcode, "Titanium::ENCODE(OP_MoveItem)");

		eq->from_slot = ServerToTitaniumSlot(emu->from_slot);
		eq->to_slot = ServerToTitaniumSlot(emu->to_slot);
		OUT(number_in_stack);

		FINISH_ENCODE();
	}

	ENCODE(OP_NewSpawn) { ENCODE_FORWARD(OP_ZoneSpawns); }

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
		OUT(abilitySlotRefresh);
		OUT(haircolor);
		OUT(beardcolor);
		OUT(eyecolor1);
		OUT(eyecolor2);
		OUT(hairstyle);
		OUT(beard);
		//	OUT(unknown00178[10]);
		for (r = EQ::textures::textureBegin; r < EQ::textures::materialCount; r++) {
			OUT(item_material.Slot[r].Material);
			OUT(item_tint.Slot[r].Color);
		}
		//	OUT(unknown00224[48]);
		for (r = 0; r < structs::MAX_PP_AA_ARRAY; r++) {
			OUT(aa_array[r].AA);
			OUT(aa_array[r].value);
		}
		//	OUT(unknown02220[4]);
		OUT(points);
		OUT(mana);
		OUT(cur_hp);
		OUT(STR);
		OUT(STA);
		OUT(CHA);
		OUT(DEX);
		OUT(INT);
		OUT(AGI);
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

		//	OUT(unknown4184[448]);
		OUT_array(mem_spells, spells::SPELL_GEM_COUNT);
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
		OUT_array(InnateSkills, structs::MAX_PP_INNATE_SKILL);	// 1:1 direct copy (25 dword)

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
		//	OUT(unknown05008[360]);
		//	OUT_array(recastTimers, structs::MAX_RECAST_TYPES);
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
		switch (emu->guildrank) {
			case GUILD_SENIOR_MEMBER:
			case GUILD_MEMBER:
			case GUILD_JUNIOR_MEMBER:
			case GUILD_INITIATE:
			case GUILD_RECRUIT: {
				eq->guildrank = GUILD_MEMBER_TI;
				break;
			}
			case GUILD_OFFICER:
			case GUILD_SENIOR_OFFICER: {
				eq->guildrank = GUILD_OFFICER_TI;
				break;
			}
			case GUILD_LEADER: {
				eq->guildrank = GUILD_LEADER_TI;
				break;
			}
			default: {
				eq->guildrank = GUILD_RANK_NONE_TI;
				break;
			}
		}
		OUT(guildbanker);
		//	OUT(unknown13054[8]);
		OUT(exp);
		//	OUT(unknown13072[12]);
		OUT(timeentitledonaccount);
		OUT_array(languages, structs::MAX_PP_LANGUAGE);
		//	OUT(unknown13109[7]);
		OUT(x);
		OUT(y);
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
		//	OUT(unknown14932[4580]);
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
		//	OUT(unknown19584[4]);
		//	OUT(unknown19588);


		const uint8 bytes[] = {
			0x78, 0x03, 0x00, 0x00, 0x1A, 0x04, 0x00, 0x00, 0x1A, 0x04, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00,
			0x19, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00,
			0x0F, 0x00, 0x00, 0x00, 0x1F, 0x85, 0xEB, 0x3E, 0x33, 0x33, 0x33, 0x3F, 0x09, 0x00, 0x00, 0x00,
			0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x14
		};
		memcpy(eq->unknown12864, bytes, sizeof(bytes));

		//set the checksum...
		CRC32::SetEQChecksum(__packet->pBuffer, sizeof(structs::PlayerProfile_Struct) - 4);

		FINISH_ENCODE();
	}

	ENCODE(OP_MarkRaidNPC)
	{
		ENCODE_LENGTH_EXACT(MarkNPC_Struct);
		SETUP_DIRECT_ENCODE(MarkNPC_Struct, MarkNPC_Struct);

		EQApplicationPacket* outapp = new EQApplicationPacket(OP_MarkNPC, sizeof(MarkNPC_Struct));
		MarkNPC_Struct* mnpcs = (MarkNPC_Struct*)outapp->pBuffer;
		mnpcs->TargetID = emu->TargetID;
		mnpcs->Number = emu->Number;
		dest->QueuePacket(outapp);
		safe_delete(outapp);

		FINISH_ENCODE();
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
		// no apparent slot translation needed
		EQApplicationPacket *in = *p;
		*p = nullptr;

		unsigned char *__emu_buffer = in->pBuffer;

		BookText_Struct *emu_BookText_Struct = (BookText_Struct *)__emu_buffer;

		in->size = sizeof(structs::BookText_Struct) + strlen(emu_BookText_Struct->booktext);
		in->pBuffer = new unsigned char[in->size];

		structs::BookText_Struct *eq_BookText_Struct = (structs::BookText_Struct*)in->pBuffer;

		eq_BookText_Struct->window = emu_BookText_Struct->window;
		eq_BookText_Struct->type = emu_BookText_Struct->type;
		strcpy(eq_BookText_Struct->booktext, emu_BookText_Struct->booktext);

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_RespondAA)
	{
		ENCODE_LENGTH_EXACT(AATable_Struct);
		SETUP_DIRECT_ENCODE(AATable_Struct, structs::AATable_Struct);

		unsigned int r;
		for (r = 0; r < structs::MAX_PP_AA_ARRAY; r++) {
			OUT(aa_list[r].AA);
			OUT(aa_list[r].value);
		}

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
		SETUP_DIRECT_ENCODE(CharacterSelect_Struct, structs::CharacterSelect_Struct);

		unsigned char *emu_ptr = __emu_buffer;
		emu_ptr += sizeof(CharacterSelect_Struct);
		CharacterSelectEntry_Struct *emu_cse = (CharacterSelectEntry_Struct *)nullptr;

		for (size_t index = 0; index < 10; ++index) {
			memset(eq->Name[index], 0, 64);
		}

		// Non character-indexed packet fields
		eq->Unknown830[0] = 0;
		eq->Unknown830[1] = 0;
		eq->Unknown0962[0] = 0;
		eq->Unknown0962[1] = 0;

		size_t char_index = 0;
		for (; char_index < emu->CharCount && char_index < 8; ++char_index) {
			emu_cse = (CharacterSelectEntry_Struct *)emu_ptr;

			eq->Race[char_index] = emu_cse->Race;
			if (eq->Race[char_index] > 474)
				eq->Race[char_index] = 1;

			for (int index = 0; index < EQ::textures::materialCount; ++index) {
				eq->CS_Colors[char_index].Slot[index].Color = emu_cse->Equip[index].Color;
			}

			eq->BeardColor[char_index] = emu_cse->BeardColor;
			eq->HairStyle[char_index] = emu_cse->HairStyle;

			for (int index = 0; index < EQ::textures::materialCount; ++index) {
				eq->Equip[char_index].Slot[index].Material = emu_cse->Equip[index].Material;
			}

			eq->SecondaryIDFile[char_index] = emu_cse->SecondaryIDFile;
			eq->Unknown820[char_index] = (uint8)0xFF;
			eq->Deity[char_index] = emu_cse->Deity;
			eq->GoHome[char_index] = emu_cse->GoHome;
			eq->Tutorial[char_index] = emu_cse->Tutorial;
			eq->Beard[char_index] = emu_cse->Beard;
			eq->Unknown902[char_index] = (uint8)0xFF;
			eq->PrimaryIDFile[char_index] = emu_cse->PrimaryIDFile;
			eq->HairColor[char_index] = emu_cse->HairColor;
			eq->Zone[char_index] = emu_cse->Zone;
			eq->Class[char_index] = emu_cse->Class;
			eq->Face[char_index] = emu_cse->Face;

			memcpy(eq->Name[char_index], emu_cse->Name, 64);

			eq->Gender[char_index] = emu_cse->Gender;
			eq->EyeColor1[char_index] = emu_cse->EyeColor1;
			eq->EyeColor2[char_index] = emu_cse->EyeColor2;
			eq->Level[char_index] = emu_cse->Level;

			emu_ptr += sizeof(CharacterSelectEntry_Struct);
		}

		for (; char_index < 10; ++char_index) {
			eq->Race[char_index] = 0;

			for (int index = 0; index < EQ::textures::materialCount; ++index) {
				eq->CS_Colors[char_index].Slot[index].Color = 0;
			}

			eq->BeardColor[char_index] = 0;
			eq->HairStyle[char_index] = 0;

			for (int index = 0; index < EQ::textures::materialCount; ++index) {
				eq->Equip[char_index].Slot[index].Material = 0;
			}

			eq->SecondaryIDFile[char_index] = 0;
			eq->Unknown820[char_index] = (uint8)0xFF;
			eq->Deity[char_index] = 0;
			eq->GoHome[char_index] = 0;
			eq->Tutorial[char_index] = 0;
			eq->Beard[char_index] = 0;
			eq->Unknown902[char_index] = (uint8)0xFF;
			eq->PrimaryIDFile[char_index] = 0;
			eq->HairColor[char_index] = 0;
			eq->Zone[char_index] = 0;
			eq->Class[char_index] = 0;
			eq->Face[char_index] = 0;

			strncpy(eq->Name[char_index], "<none>", 6);

			eq->Gender[char_index] = 0;
			eq->EyeColor1[char_index] = 0;
			eq->EyeColor2[char_index] = 0;
			eq->Level[char_index] = 0;
		}

		FINISH_ENCODE();
	}

	ENCODE(OP_SetFace)
	{
		auto emu = reinterpret_cast<FaceChange_Struct*>((*p)->pBuffer);

		EQApplicationPacket outapp(OP_Illusion, sizeof(structs::Illusion_Struct));
		auto buf = reinterpret_cast<structs::Illusion_Struct*>(outapp.pBuffer);

		buf->spawnid     = emu->entity_id;
		buf->race        = -1; // unchanged
		buf->gender      = -1; // unchanged
		buf->texture     = -1; // unchanged
		buf->helmtexture = -1; // unchanged
		buf->face        = emu->face;
		buf->hairstyle   = emu->hairstyle;
		buf->haircolor   = emu->haircolor;
		buf->beard       = emu->beard;
		buf->beardcolor  = emu->beardcolor;
		buf->size        = 0.0f; // unchanged

		safe_delete(*p); // not using the original packet

		dest->QueuePacket(&outapp, ack_req);
	}

	ENCODE(OP_ShopPlayerSell)
	{
		ENCODE_LENGTH_EXACT(Merchant_Purchase_Struct);
		SETUP_DIRECT_ENCODE(Merchant_Purchase_Struct, structs::Merchant_Purchase_Struct);

		OUT(npcid);
		eq->itemslot = ServerToTitaniumSlot(emu->itemslot);
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
		in->ReadString(name); // NPC names max out at 63 chars

		buf.WriteString(name);

		buf.WriteInt32(in->ReadUInt32()); // loc
		buf.WriteInt32(in->ReadUInt32());
		buf.WriteInt32(in->ReadUInt32());

		std::string old_message;
		std::string new_message;

		in->ReadString(old_message);

		ServerToTitaniumSayLink(new_message, old_message);

		buf.WriteString(new_message);

		auto outapp = new EQApplicationPacket(OP_SpecialMesg, buf);

		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
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
		ServerToTitaniumSayLink(new_message, old_message);

		in->size = sizeof(TaskDescriptionHeader_Struct) + sizeof(TaskDescriptionData1_Struct) +
			sizeof(TaskDescriptionData2_Struct) + sizeof(TaskDescriptionTrailer_Struct) +
			title_size + description_size + reward_size + new_message.length() + 1;

		in->pBuffer = new unsigned char[in->size];

		char *OutBuffer = (char *)in->pBuffer;

		memcpy(OutBuffer, block_start, (InBuffer - block_start));
		OutBuffer += (InBuffer - block_start);

		VARSTRUCT_ENCODE_STRING(OutBuffer, new_message.c_str());

		InBuffer += strlen(InBuffer) + 1;

		memcpy(OutBuffer, InBuffer, sizeof(TaskDescriptionTrailer_Struct));
		// we have an extra DWORD in the trailer struct, client should ignore it so w/e

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
		auto action = *(uint32 *) (*p)->pBuffer;

		switch (action) {
			case TraderOn: {
				ENCODE_LENGTH_EXACT(Trader_ShowItems_Struct);
				SETUP_DIRECT_ENCODE(Trader_ShowItems_Struct, structs::Trader_ShowItems_Struct);
				LogTrading(
					"Encode OP_Trader BeginTraderMode action <green>[{}]",
					action
				);

				eq->action = structs::TiBazaarTraderBuyerActions::BeginTraderMode;
				OUT(entity_id);

				FINISH_ENCODE();
				break;
			}
			case TraderOff: {
				ENCODE_LENGTH_EXACT(Trader_ShowItems_Struct);
				SETUP_DIRECT_ENCODE(Trader_ShowItems_Struct, structs::Trader_ShowItems_Struct);
				LogTrading(
					"Encode OP_Trader EndTraderMode action <green>[{}]",
					action
				);

				eq->action = structs::TiBazaarTraderBuyerActions::EndTraderMode;
				OUT(entity_id);

				FINISH_ENCODE();
				break;
			}
			case ListTraderItems: {
				ENCODE_LENGTH_EXACT(Trader_Struct);
				SETUP_DIRECT_ENCODE(Trader_Struct, structs::Trader_Struct);
				LogTrading(
					"Encode OP_Trader ListTraderItems action <green>[{}]",
					action
				);

				eq->action = structs::TiBazaarTraderBuyerActions::ListTraderItems;
				std::copy_n(emu->items, UF::invtype::BAZAAR_SIZE, eq->item_id);
				std::copy_n(emu->item_cost, UF::invtype::BAZAAR_SIZE, eq->item_cost);

				FINISH_ENCODE();
				break;
			}
			case BuyTraderItem: {
				ENCODE_LENGTH_EXACT(TraderBuy_Struct);
				SETUP_DIRECT_ENCODE(TraderBuy_Struct, structs::TraderBuy_Struct);
				LogTrading(
					"Encode OP_Trader item_id <green>[{}] price <green>[{}] quantity <green>[{}] trader_id <green>[{}]",
					eq->item_id,
					eq->price,
					eq->quantity,
					eq->trader_id
				);

				eq->action = structs::TiBazaarTraderBuyerActions::BuyTraderItem;
				OUT(price);
				OUT(trader_id);
				OUT(item_id);
				OUT(already_sold);
				OUT(quantity);
				strn0cpy(eq->item_name, emu->item_name, sizeof(eq->item_name));

				FINISH_ENCODE();
				break;
			}
			case ItemMove: {
				LogTrading(
					"Encode OP_Trader ItemMove action <green>[{}]",
					action
				);
				EQApplicationPacket *in = *p;
				*p = nullptr;
				dest->FastQueuePacket(&in, ack_req);
				break;
			}
			default: {
				EQApplicationPacket *in = *p;
				*p = nullptr;

				dest->FastQueuePacket(&in, ack_req);
				LogError("Unknown Encode OP_Trader action <red>{} received.  Unhandled.", action);
			}
		}
	}

	ENCODE(OP_TraderBuy)
	{
		ENCODE_LENGTH_EXACT(TraderBuy_Struct);
		SETUP_DIRECT_ENCODE(TraderBuy_Struct, structs::TraderBuy_Struct);
		LogTrading(
			"Encode OP_TraderBuy item_id <green>[{}] price <green>[{}] quantity <green>[{}] trader_id <green>[{}]",
			emu->item_id,
			emu->price,
			emu->quantity,
			emu->trader_id
		);

		OUT(action);
		OUT(price);
		OUT(trader_id);
		OUT(item_id);
		OUT(already_sold);
		OUT(quantity);
		OUT_str(item_name);

		FINISH_ENCODE();
	}

	ENCODE(OP_TraderShop)
	{
		auto action = *(uint32 *)(*p)->pBuffer;

		switch (action) {
			case ClickTrader: {
				ENCODE_LENGTH_EXACT(TraderClick_Struct);
				SETUP_DIRECT_ENCODE(TraderClick_Struct, structs::TraderClick_Struct);
				LogTrading(
					"ClickTrader action <green>[{}] trader_id <green>[{}]",
					action,
					emu->TraderID
				);

				eq->action    = 0;
				eq->trader_id = emu->TraderID;
				eq->approval  = emu->Approval;

				FINISH_ENCODE();
				break;
			}
			case BuyTraderItem: {
				ENCODE_LENGTH_EXACT(TraderBuy_Struct);
				SETUP_DIRECT_ENCODE(TraderBuy_Struct, structs::TraderBuy_Struct);
				LogTrading(
					"Encode OP_TraderShop item_id <green>[{}] price <green>[{}] quantity <green>[{}] trader_id <green>[{}]",
					eq->item_id,
					eq->price,
					eq->quantity,
					eq->trader_id
				);

				eq->action = structs::TiBazaarTraderBuyerActions::BuyTraderItem;
				OUT(price);
				OUT(trader_id);
				OUT(item_id);
				OUT(already_sold);
				OUT(quantity);
				strn0cpy(eq->item_name, emu->item_name, sizeof(eq->item_name));

				FINISH_ENCODE();
				break;
			}
			default: {
				EQApplicationPacket *in = *p;
				*p = nullptr;

				dest->FastQueuePacket(&in, ack_req);
				LogError("Unknown Encode OP_TraderShop action <red>[{}] received.  Unhandled.", action);
			}
		}
	}

	ENCODE(OP_TributeItem)
	{
		ENCODE_LENGTH_EXACT(TributeItem_Struct);
		SETUP_DIRECT_ENCODE(TributeItem_Struct, structs::TributeItem_Struct);

		eq->slot = ServerToTitaniumSlot(emu->slot);
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

			vr->claim_id = ivr->claim_id;
			vr->item.item_id = ivr->items[0].item_id;
			strcpy(vr->item.item_name, ivr->items[0].item_name);

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
		OUT(color.Color);
		OUT(wear_slot_id);

		FINISH_ENCODE();
	}

	ENCODE(OP_ZoneEntry) { ENCODE_FORWARD(OP_ZoneSpawns); }

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
			//		eq->unknown0000 = emu->unknown0000;
			eq->gm = emu->gm;
			//		eq->unknown0003 = emu->unknown0003;
			eq->aaitle = emu->aaitle;
			//		eq->unknown0004 = emu->unknown0004;
			eq->anon = emu->anon;
			eq->face = emu->face;
			strcpy(eq->name, emu->name);
			eq->deity = emu->deity;
			//		eq->unknown0073 = emu->unknown0073;
			eq->size = emu->size;
			//		eq->unknown0079 = emu->unknown0079;
			eq->NPC = emu->NPC;
			eq->invis = emu->invis;
			eq->haircolor = emu->haircolor;
			eq->curHp = emu->curHp;
			eq->max_hp = emu->max_hp;
			eq->findable = emu->findable;
			//		eq->unknown0089[5] = emu->unknown0089[5];
			eq->deltaHeading = emu->deltaHeading;
			eq->x = emu->x;
			//		eq->padding0054 = emu->padding0054;
			eq->y = emu->y;
			eq->animation = emu->animation;
			//		eq->padding0058 = emu->padding0058;
			eq->z = emu->z;
			eq->deltaY = emu->deltaY;
			eq->deltaX = emu->deltaX;
			eq->heading = emu->heading;
			//		eq->padding0066 = emu->padding0066;
			eq->deltaZ = emu->deltaZ;
			//		eq->padding0070 = emu->padding0070;
			eq->eyecolor1 = emu->eyecolor1;
			//		eq->unknown0115[24] = emu->unknown0115[24];
			eq->showhelm = emu->showhelm;
			//		eq->unknown0140[4] = emu->unknown0140[4];
			eq->is_npc = emu->is_npc;
			eq->hairstyle = emu->hairstyle;

			//if(emu->gender == 1){
			//	eq->hairstyle = eq->hairstyle == 0xFF ? 0 : eq->hairstyle;
			//}

			eq->beardcolor = emu->beardcolor;
			//		eq->unknown0147[4] = emu->unknown0147[4];
			eq->level = emu->level;
			eq->PlayerState = emu->PlayerState;
			eq->beard = emu->beard;
			strcpy(eq->suffix, emu->suffix);
			eq->petOwnerId = emu->petOwnerId;
			switch (emu->guildrank) {
				case GUILD_SENIOR_MEMBER:
				case GUILD_MEMBER:
				case GUILD_JUNIOR_MEMBER:
				case GUILD_INITIATE:
				case GUILD_RECRUIT: {
					eq->guildrank = GUILD_MEMBER_TI;
					break;
				}
				case GUILD_OFFICER:
				case GUILD_SENIOR_OFFICER: {
					eq->guildrank = GUILD_OFFICER_TI;
					break;
				}
				case GUILD_LEADER: {
					eq->guildrank = GUILD_LEADER_TI;
					break;
				}
				default: {
					break;
				}
			}
			//		eq->unknown0194[3] = emu->unknown0194[3];
			for (k = EQ::textures::textureBegin; k < EQ::textures::materialCount; k++) {
				eq->equipment.Slot[k].Material = emu->equipment.Slot[k].Material;
				eq->equipment_tint.Slot[k].Color = emu->equipment_tint.Slot[k].Color;
			}
			for (k = 0; k < 8; k++) {
				eq->set_to_0xFF[k] = 0xFF;
			}

			eq->runspeed = emu->runspeed;
			eq->afk = emu->afk;
			eq->guildID = emu->guildID;
			strcpy(eq->title, emu->title);
			//		eq->unknown0274 = emu->unknown0274;
			eq->helm = emu->helm;
			if (emu->race > 474)
				eq->race = 1;
			else
				eq->race = emu->race;
			//		eq->unknown0288 = emu->unknown0288;
			strcpy(eq->lastName, emu->lastName);
			eq->walkspeed = emu->walkspeed;
			//		eq->unknown0328 = emu->unknown0328;
			eq->is_pet = emu->is_pet;
			eq->light = emu->light;
			eq->class_ = emu->class_;
			eq->eyecolor2 = emu->eyecolor2;
			//		eq->unknown0333 = emu->unknown0333;
			eq->flymode = emu->flymode;
			eq->gender = emu->gender;
			eq->bodytype = emu->bodytype;
			//		eq->unknown0336[3] = emu->unknown0336[3];
			eq->equip_chest2 = emu->equip_chest2;
			eq->spawnId = emu->spawnId;
			//		eq->unknown0344[4] = emu->unknown0344[4];
			eq->lfg = emu->lfg;

			/*
			if (emu->face == 99)	      {eq->face = 0;}
			if (emu->eyecolor1 == 99)  {eq->eyecolor1 = 0;}
			if (emu->eyecolor2 == 99)  {eq->eyecolor2 = 0;}
			if (emu->hairstyle == 99)  {eq->hairstyle = 0;}
			if (emu->haircolor == 99)  {eq->haircolor = 0;}
			if (emu->beard == 99)      {eq->beard = 0;}
			if (emu->beardcolor == 99) {eq->beardcolor = 0;}
			*/

		}

		//kill off the emu structure and send the eq packet.
		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

// DECODE methods
	DECODE(OP_AdventureMerchantSell)
	{
		DECODE_LENGTH_EXACT(structs::Adventure_Sell_Struct);
		SETUP_DIRECT_DECODE(Adventure_Sell_Struct, structs::Adventure_Sell_Struct);

		IN(npcid);
		emu->slot = TitaniumToServerSlot(eq->slot);
		IN(charges);
		IN(sell_price);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ApplyPoison)
	{
		DECODE_LENGTH_EXACT(structs::ApplyPoison_Struct);
		SETUP_DIRECT_DECODE(ApplyPoison_Struct, structs::ApplyPoison_Struct);

		emu->inventorySlot = TitaniumToServerSlot(eq->inventorySlot);
		IN(success);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_AugmentItem)
	{
		DECODE_LENGTH_EXACT(structs::AugmentItem_Struct);
		SETUP_DIRECT_DECODE(AugmentItem_Struct, structs::AugmentItem_Struct);

		emu->container_slot = TitaniumToServerSlot(eq->container_slot);
		emu->augment_slot = eq->augment_slot;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_BazaarSearch)
	{
		uint32 action = *(uint32 *) __packet->pBuffer;

		switch (action) {
			case structs::TiBazaarTraderBuyerActions::BazaarSearch: {
				DECODE_LENGTH_EXACT(structs::BazaarSearch_Struct);
				SETUP_DIRECT_DECODE(BazaarSearchCriteria_Struct, structs::BazaarSearch_Struct);

				emu->action           = eq->Beginning.Action;
				emu->item_stat        = eq->ItemStat;
				emu->max_cost         = eq->MaxPrice;
				emu->min_cost         = eq->MinPrice;
				emu->max_level        = eq->MaxLlevel;
				emu->min_level        = eq->Minlevel;
				emu->race             = eq->Race;
				emu->slot             = eq->Slot;
				emu->type             = eq->Type == UINT32_MAX ? UINT8_MAX : eq->Type;
				emu->trader_entity_id = eq->TraderID;
				emu->trader_id        = 0;
				emu->_class           = eq->Class_;
				emu->search_scope     = eq->TraderID > 0 ? NonRoFBazaarSearchScope : Local_Scope;
				emu->max_results      = RuleI(Bazaar, MaxSearchResults);
				strn0cpy(emu->item_name, eq->Name, sizeof(emu->item_name));

				FINISH_DIRECT_DECODE();
				break;
			}
			case structs::TiBazaarTraderBuyerActions::BazaarInspect: {
				SETUP_DIRECT_DECODE(BazaarInspect_Struct, structs::BazaarInspect_Struct);

				IN(action);
				memcpy(emu->player_name, eq->player_name, sizeof(emu->player_name));
				IN(serial_number);

				FINISH_DIRECT_DECODE();
				break;
			}
			case structs::TiBazaarTraderBuyerActions::WelcomeMessage: {
				break;
			}
			default: {
				LogTrading("(Ti) Unhandled action <red>[{}]", action);
			}
		}
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
		emu->slotid = TitaniumToServerBuffSlot(eq->slotid);
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

		emu->slot = static_cast<uint32>(TitaniumToServerCastingSlot(static_cast<spells::CastingSlot>(eq->slot), eq->inventoryslot));
		IN(spell_id);
		emu->inventoryslot = TitaniumToServerSlot(eq->inventoryslot);
		IN(target_id);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ChannelMessage)
	{
		unsigned char *__eq_buffer = __packet->pBuffer;

		std::string old_message = (char *)&__eq_buffer[sizeof(ChannelMessage_Struct)];
		std::string new_message;
		TitaniumToServerSayLink(new_message, old_message);

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
		IN(haircolor);
		IN(gender);
		IN(race);
		IN(start_zone);
		IN(hairstyle);
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

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ClientUpdate)
	{
		// for some odd reason, there is an extra byte on the end of this on occasion.. (copied from SoF..not sure if applies to Ti - TODO: check)
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

	DECODE(OP_Consume)
	{
		DECODE_LENGTH_EXACT(structs::Consume_Struct);
		SETUP_DIRECT_DECODE(Consume_Struct, structs::Consume_Struct);

		emu->slot = TitaniumToServerSlot(eq->slot);
		IN(auto_consumed);
		IN(type);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_DeleteItem)
	{
		DECODE_LENGTH_EXACT(structs::DeleteItem_Struct);
		SETUP_DIRECT_DECODE(DeleteItem_Struct, structs::DeleteItem_Struct);

		emu->from_slot = TitaniumToServerSlot(eq->from_slot);
		emu->to_slot =TitaniumToServerSlot(eq->to_slot);
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
		TitaniumToServerSayLink(new_message, old_message);

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

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GuildDemote)
	{
		DECODE_LENGTH_EXACT(structs::GuildDemoteStruct);
		SETUP_DIRECT_DECODE(GuildDemoteStruct, structs::GuildDemoteStruct);

		memcpy(emu->name, eq->name, sizeof(emu->name));
		memcpy(emu->target, eq->target, sizeof(emu->target));
		emu->rank = GUILD_MEMBER;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GuildTributeDonateItem)
	{
		DECODE_LENGTH_EXACT(structs::GuildTributeDonateItemRequest_Struct);
		SETUP_DIRECT_DECODE(GuildTributeDonateItemRequest_Struct, structs::GuildTributeDonateItemRequest_Struct);

		Log(Logs::Detail, Logs::Netcode, "UF::DECODE(OP_GuildTributeDonateItem)");

		IN(quantity);
		IN(tribute_master_id);
		IN(guild_id);

		emu->slot = TitaniumToServerSlot(eq->slot);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_InspectAnswer)
	{
		DECODE_LENGTH_EXACT(structs::InspectResponse_Struct);
		SETUP_DIRECT_DECODE(InspectResponse_Struct, structs::InspectResponse_Struct);

		IN(TargetID);
		IN(playerid);

		for (int i = invslot::slotCharm; i <= invslot::slotWaist; ++i) {
			strn0cpy(emu->itemnames[i], eq->itemnames[i], sizeof(emu->itemnames[i]));
			IN(itemicons[i]);
		}

		// move ammo up to last element in server array
		strn0cpy(emu->itemnames[EQ::invslot::slotAmmo], eq->itemnames[invslot::slotAmmo], sizeof(emu->itemnames[EQ::invslot::slotAmmo]));
		emu->itemicons[EQ::invslot::slotAmmo] = eq->itemicons[invslot::slotAmmo];

		// nullify power source element in server array
		strn0cpy(emu->itemnames[EQ::invslot::slotPowerSource], "", sizeof(emu->itemnames[EQ::invslot::slotPowerSource]));
		emu->itemicons[EQ::invslot::slotPowerSource] = 0xFFFFFFFF;

		strn0cpy(emu->text, eq->text, sizeof(emu->text));

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

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_LFGuild)
	{
		struct bit_mask_conversion {
			uint32	titanium_mask;
			uint32	rof2_mask;
		};

		std::vector<bit_mask_conversion> bit_mask = {
			{.titanium_mask = 2, .rof2_mask = 256},
			{.titanium_mask = 4, .rof2_mask = 32768},
			{.titanium_mask = 8, .rof2_mask = 65536},
			{.titanium_mask = 16, .rof2_mask = 4},
			{.titanium_mask = 32, .rof2_mask = 64},
			{.titanium_mask = 64, .rof2_mask = 16384},
			{.titanium_mask = 128, .rof2_mask = 8192},
			{.titanium_mask = 256, .rof2_mask = 128},
			{.titanium_mask = 512, .rof2_mask = 2048},
			{.titanium_mask = 1024, .rof2_mask = 8},
			{.titanium_mask = 2048, .rof2_mask = 16},
			{.titanium_mask = 4096, .rof2_mask = 512},
			{.titanium_mask = 8192, .rof2_mask = 32},
			{.titanium_mask = 16384, .rof2_mask = 1024},
			{.titanium_mask = 32768, .rof2_mask = 2},
			{.titanium_mask = 65536, .rof2_mask = 4096},
		};

		uint32 Command = __packet->ReadUInt32();

		if (Command == 3) {
			DECODE_LENGTH_EXACT(structs::LFGuild_SearchPlayer_Struct);
			SETUP_DIRECT_DECODE(LFGuild_SearchPlayer_Struct, structs::LFGuild_SearchPlayer_Struct);

			IN(Command);
			IN(Unknown04);
			IN(FromLevel);
			IN(ToLevel);
			IN(MinAA);
			IN(TimeZone);

			uint32 new_bitmask = 0;
			uint32 ti_bitmask = eq->Classes;
			for (auto const& b : bit_mask) {
				(ti_bitmask & b.titanium_mask) != 0 ? new_bitmask |= b.rof2_mask : new_bitmask &= ~b.rof2_mask;
			}
			emu->Classes = new_bitmask;
			FINISH_DIRECT_DECODE();
			return;
		}

		if (Command == 1) {
			DECODE_LENGTH_EXACT(structs::LFGuild_GuildToggle_Struct);
			SETUP_DIRECT_DECODE(LFGuild_GuildToggle_Struct, structs::LFGuild_GuildToggle_Struct);

			IN(Command);
			IN_str(Comment);
			IN(FromLevel);
			IN(ToLevel);
			IN(AACount);
			IN(TimeZone);
			IN(Toggle);
			IN(TimePosted);
			IN_str(Name);

			uint32 new_bitmask = 0;
			uint32 ti_bitmask = eq->Classes;
			for (auto const& b : bit_mask) {
				(ti_bitmask & b.titanium_mask) != 0 ? new_bitmask |= b.rof2_mask : new_bitmask &= ~b.rof2_mask;
			}
			emu->Classes = new_bitmask;
			FINISH_DIRECT_DECODE();
			return;
		}

		if (Command != 0)
			return;

		SETUP_DIRECT_DECODE(LFGuild_PlayerToggle_Struct, structs::LFGuild_PlayerToggle_Struct);
		memcpy(emu, eq, sizeof(structs::LFGuild_PlayerToggle_Struct));
		memset(emu->Unknown612, 0, sizeof(emu->Unknown612));

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

		Log(Logs::Detail, Logs::Netcode, "Titanium::DECODE(OP_LootItem)");

		IN(lootee);
		IN(looter);
		emu->slot_id = TitaniumToServerCorpseSlot(eq->slot_id);
		IN(auto_loot);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_MoveItem)
	{
		DECODE_LENGTH_EXACT(structs::MoveItem_Struct);
		SETUP_DIRECT_DECODE(MoveItem_Struct, structs::MoveItem_Struct);

		Log(Logs::Detail, Logs::Netcode, "Titanium::DECODE(OP_MoveItem)");

		emu->from_slot = TitaniumToServerSlot(eq->from_slot);
		emu->to_slot = TitaniumToServerSlot(eq->to_slot);
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

			auto len = 0;
			if (__packet->size < sizeof(structs::RaidMOTD_Struct)) {
				len = __packet->size - sizeof(structs::RaidGeneral_Struct);
			}
			else {
				len = sizeof(eq->motd);
			}

			strn0cpy(emu->motd, eq->motd, len > 1024 ? 1024 : len);
			emu->motd[len - 1] = '\0';

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
		// no apparent slot translation needed
		DECODE_LENGTH_ATLEAST(structs::BookRequest_Struct);
		SETUP_DIRECT_DECODE(BookRequest_Struct, structs::BookRequest_Struct);

		IN(window);
		IN(type);
		strn0cpy(emu->txtfile, eq->txtfile, sizeof(emu->txtfile));

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
		emu->itemslot = TitaniumToServerSlot(eq->itemslot);
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

	DECODE(OP_Trader)
	{
		auto action = (uint32) __packet->pBuffer[0];

		switch (action) {
			case structs::TiBazaarTraderBuyerActions::BeginTraderMode: {
				DECODE_LENGTH_EXACT(structs::BeginTrader_Struct);
				SETUP_DIRECT_DECODE(ClickTrader_Struct, structs::BeginTrader_Struct);
				LogTrading(
					"Decode OP_Trader BeginTraderMode action <red>[{}]",
					action
				);

				emu->action      = TraderOn;
				emu->unknown_004 = 0;
				std::copy_n(eq->serial_number, Titanium::invtype::BAZAAR_SIZE, emu->serial_number);
				std::copy_n(eq->cost, Titanium::invtype::BAZAAR_SIZE, emu->item_cost);

				FINISH_DIRECT_DECODE();
				break;
			}
			case structs::TiBazaarTraderBuyerActions::EndTraderMode: {
				DECODE_LENGTH_EXACT(structs::Trader_ShowItems_Struct);
				SETUP_DIRECT_DECODE(Trader_ShowItems_Struct, structs::Trader_ShowItems_Struct);
				LogTrading(
					"Decode OP_Trader(Ti) EndTraderMode action <red>[{}]",
					action
				);

				emu->action = TraderOff;
				IN(entity_id);

				FINISH_DIRECT_DECODE();
				break;
			}
			case structs::TiBazaarTraderBuyerActions::PriceUpdate:
			case structs::TiBazaarTraderBuyerActions::ItemMove:
			case structs::TiBazaarTraderBuyerActions::EndTransaction:
			case structs::TiBazaarTraderBuyerActions::ListTraderItems: {
				LogTrading(
					"Decode OP_Trader(Ti) Price/ItemMove/EndTransaction/ListTraderItems action <red>[{}]",
					action
				);
				break;
			}
			case structs::TiBazaarTraderBuyerActions::ReconcileItems: {
				break;
			}
			default: {
				LogError("Unhandled(Ti) action <red>[{}] received.", action);
			}
		}
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

	DECODE(OP_TraderShop)
	{
		DECODE_LENGTH_EXACT(structs::TraderClick_Struct);
		SETUP_DIRECT_DECODE(TraderClick_Struct, structs::TraderClick_Struct);
		LogTrading(
			"(Ti) action <green>[{}] trader_id <green>[{}] approval <green>[{}]",
			eq->action,
			eq->trader_id,
			eq->approval
		);

		emu->Code     = ClickTrader;
		emu->TraderID = eq->trader_id;
		emu->Approval = eq->approval;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_TradeSkillCombine)
	{
		DECODE_LENGTH_EXACT(structs::NewCombine_Struct);
		SETUP_DIRECT_DECODE(NewCombine_Struct, structs::NewCombine_Struct);

		emu->container_slot = TitaniumToServerSlot(eq->container_slot);
		IN(guildtribute_slot);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_TributeItem)
	{
		DECODE_LENGTH_EXACT(structs::TributeItem_Struct);
		SETUP_DIRECT_DECODE(TributeItem_Struct, structs::TributeItem_Struct);

		emu->slot = TitaniumToServerSlot(eq->slot);
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
		IN(color.Color);
		IN(wear_slot_id);
		emu->unknown06 = 0;
		emu->elite_material = 0;
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
		emu->type = 3;

		FINISH_DIRECT_DECODE();
	}

// file scope helper methods
	void SerializeItem(EQ::OutBuffer& ob, const EQ::ItemInstance *inst, int16 slot_id_in, uint8 depth) {
		const char *protection      = "\\\\\\\\\\";
		const EQ::ItemData *item = inst->GetUnscaledItem();

		ob << StringFormat(
			"%.*s%s",
			(depth ? (depth - 1) : 0),
			protection,
			(depth ? "\"" : "")); // For leading quotes (and protection) if a subitem;

		// Instance data
		ob << itoa((inst->IsStackable() ? inst->GetCharges() : 0)); // stack count
		ob << '|' << itoa(0); // unknown
		ob << '|' << itoa((!inst->GetMerchantSlot() ? slot_id_in : inst->GetMerchantSlot())); // inst slot/merchant slot
		ob << '|' << itoa(inst->GetPrice()); // merchant price
		ob << '|' << itoa((!inst->GetMerchantSlot() ? 1 : inst->GetMerchantCount())); // inst count/merchant count
		ob << '|' << itoa((inst->IsScaling() ? (inst->GetExp() / 100) : 0)); // inst experience
		ob << '|' << itoa((!inst->GetMerchantSlot() ? inst->GetSerialNumber()
			: inst->GetMerchantSlot())); // merchant serial number
		ob << '|' << itoa(inst->GetRecastTimestamp()); // recast timestamp
		ob << '|' << itoa(((inst->IsStackable() ? ((inst->GetItem()->ItemType == EQ::item::ItemTypePotion) ? 1 : 0)
			: inst->GetCharges()))); // charge count
		ob << '|' << itoa((inst->IsAttuned() ? 1 : 0)); // inst attuned
		ob << '|' << itoa(0); // unknown
		ob << '|';

		ob << StringFormat("%.*s\"", depth, protection); // Quotes (and protection, if needed) around static data

		// Item data
		ob << itoa(item->ItemClass);
		ob << '|' << item->Name;
		ob << '|' << item->Lore;
		ob << '|' << item->IDFile;
		ob << '|' << itoa(item->ID);
		ob << '|' << itoa(((item->Weight > 255) ? 255 : item->Weight));

		ob << '|' << itoa(item->NoRent);
		ob << '|' << itoa(item->NoDrop);
		ob << '|' << itoa(item->Size);
		ob << '|' << itoa(Catch22(SwapBits21And22(item->Slots)));
		ob << '|' << itoa(item->Price);
		ob << '|' << itoa(item->Icon);
		ob << '|' << "0";
		ob << '|' << "0";
		ob << '|' << itoa(item->BenefitFlag);
		ob << '|' << itoa(item->Tradeskills);

		ob << '|' << itoa(item->CR);
		ob << '|' << itoa(item->DR);
		ob << '|' << itoa(item->PR);
		ob << '|' << itoa(item->MR);
		ob << '|' << itoa(item->FR);

		ob << '|' << itoa(item->AStr);
		ob << '|' << itoa(item->ASta);
		ob << '|' << itoa(item->AAgi);
		ob << '|' << itoa(item->ADex);
		ob << '|' << itoa(item->ACha);
		ob << '|' << itoa(item->AInt);
		ob << '|' << itoa(item->AWis);

		ob << '|' << itoa(item->HP);
		ob << '|' << itoa(item->Mana);
		ob << '|' << itoa(item->AC);
		ob << '|' << itoa(item->Deity);

		ob << '|' << itoa(item->SkillModValue);
		ob << '|' << itoa(item->SkillModMax);
		ob << '|' << itoa(item->SkillModType);

		ob << '|' << itoa(item->BaneDmgRace);
		if (item->BaneDmgAmt > 255)
			ob << '|' << "255";
		else
			ob << '|' << itoa(item->BaneDmgAmt);
		ob << '|' << itoa(item->BaneDmgBody);

		ob << '|' << itoa(item->Magic);
		ob << '|' << itoa(item->CastTime_);
		ob << '|' << itoa(item->ReqLevel);
		ob << '|' << itoa(item->BardType);
		ob << '|' << itoa(item->BardValue);
		ob << '|' << itoa(item->Light);
		ob << '|' << itoa(item->Delay);

		ob << '|' << itoa(item->RecLevel);
		ob << '|' << itoa(item->RecSkill);

		ob << '|' << itoa(item->ElemDmgType);
		ob << '|' << itoa(item->ElemDmgAmt);

		ob << '|' << itoa(item->Range);
		ob << '|' << itoa(item->Damage);

		ob << '|' << itoa(item->Color);
		ob << '|' << itoa(item->Classes);
		ob << '|' << itoa(item->Races);
		ob << '|' << "0";

		ob << '|' << itoa(item->MaxCharges);
		ob << '|' << itoa(item->ItemType);
		ob << '|' << itoa(item->Material);
		ob << '|' << StringFormat("%f", item->SellRate);

		ob << '|' << "0";
		ob << '|' << itoa(item->CastTime_);
		ob << '|' << "0";

		ob << '|' << itoa(item->ProcRate);
		ob << '|' << itoa(item->CombatEffects);
		ob << '|' << itoa(item->Shielding);
		ob << '|' << itoa(item->StunResist);
		ob << '|' << itoa(item->StrikeThrough);
		ob << '|' << itoa(item->ExtraDmgSkill);
		ob << '|' << itoa(item->ExtraDmgAmt);
		ob << '|' << itoa(item->SpellShield);
		ob << '|' << itoa(item->Avoidance);
		ob << '|' << itoa(item->Accuracy);

		ob << '|' << itoa(item->CharmFileID);

		ob << '|' << itoa(item->FactionMod1);
		ob << '|' << itoa(item->FactionMod2);
		ob << '|' << itoa(item->FactionMod3);
		ob << '|' << itoa(item->FactionMod4);

		ob << '|' << itoa(item->FactionAmt1);
		ob << '|' << itoa(item->FactionAmt2);
		ob << '|' << itoa(item->FactionAmt3);
		ob << '|' << itoa(item->FactionAmt4);

		ob << '|' << item->CharmFile;

		ob << '|' << itoa(item->AugType);

		ob << '|' << itoa(item->AugSlotType[0]);
		ob << '|' << itoa(item->AugSlotVisible[0]);
		ob << '|' << itoa(item->AugSlotType[1]);
		ob << '|' << itoa(item->AugSlotVisible[1]);
		ob << '|' << itoa(item->AugSlotType[2]);
		ob << '|' << itoa(item->AugSlotVisible[2]);
		ob << '|' << itoa(item->AugSlotType[3]);
		ob << '|' << itoa(item->AugSlotVisible[3]);
		ob << '|' << itoa(item->AugSlotType[4]);
		ob << '|' << itoa(item->AugSlotVisible[4]);

		ob << '|' << itoa(item->LDoNTheme);
		ob << '|' << itoa(item->LDoNPrice);
		ob << '|' << itoa(item->LDoNSold);

		ob << '|' << itoa(item->BagType);
		ob << '|' << itoa(item->BagSlots);
		ob << '|' << itoa(item->BagSize);
		ob << '|' << itoa(item->BagWR);

		ob << '|' << itoa(item->Book);
		ob << '|' << itoa(item->BookType);

		ob << '|' << item->Filename;

		ob << '|' << itoa(item->BaneDmgRaceAmt);
		ob << '|' << itoa(item->AugRestrict);
		ob << '|' << itoa(item->LoreGroup);
		ob << '|' << itoa(item->PendingLoreFlag);
		ob << '|' << itoa(item->ArtifactFlag);
		ob << '|' << itoa(item->SummonedFlag);

		ob << '|' << itoa(item->Favor);
		ob << '|' << itoa(item->FVNoDrop);
		ob << '|' << itoa(item->Endur);
		ob << '|' << itoa(item->DotShielding);
		ob << '|' << itoa(item->Attack);
		ob << '|' << itoa(item->Regen);
		ob << '|' << itoa(item->ManaRegen);
		ob << '|' << itoa(item->EnduranceRegen);
		ob << '|' << itoa(item->Haste);
		ob << '|' << itoa(item->DamageShield);
		ob << '|' << itoa(item->RecastDelay);
		ob << '|' << itoa(item->RecastType);
		ob << '|' << itoa(item->GuildFavor);

		ob << '|' << itoa(item->AugDistiller);

		ob << '|' << "0"; // unknown
		ob << '|' << "0"; // unknown
		ob << '|' << itoa(item->Attuneable);
		ob << '|' << itoa(item->NoPet);
		ob << '|' << "0"; // unknown
		ob << '|' << itoa(item->PointType);

		ob << '|' << itoa(item->PotionBelt);
		ob << '|' << itoa(item->PotionBeltSlots);
		ob << '|' << itoa(item->StackSize);
		ob << '|' << itoa(item->NoTransfer);
		ob << '|' << itoa(item->Stackable);

		ob << '|' << itoa(item->Click.Effect);
		ob << '|' << itoa(item->Click.Type);
		ob << '|' << itoa(item->Click.Level2);
		ob << '|' << itoa(item->Click.Level);
		ob << '|' << "0"; // Click name

		ob << '|' << itoa(item->Proc.Effect);
		ob << '|' << itoa(item->Proc.Type);
		ob << '|' << itoa(item->Proc.Level2);
		ob << '|' << itoa(item->Proc.Level);
		ob << '|' << "0"; // Proc name

		ob << '|' << itoa(item->Worn.Effect);
		ob << '|' << itoa(item->Worn.Type);
		ob << '|' << itoa(item->Worn.Level2);
		ob << '|' << itoa(item->Worn.Level);
		ob << '|' << "0"; // Worn name

		ob << '|' << itoa(item->Focus.Effect);
		ob << '|' << itoa(item->Focus.Type);
		ob << '|' << itoa(item->Focus.Level2);
		ob << '|' << itoa(item->Focus.Level);
		ob << '|' << "0"; // Focus name

		ob << '|' << itoa(item->Scroll.Effect);
		ob << '|' << itoa(item->Scroll.Type);
		ob << '|' << itoa(item->Scroll.Level2);
		ob << '|' << itoa(item->Scroll.Level);
		ob << '|' << "0"; // Scroll name

		ob << StringFormat("%.*s\"", depth, protection); // Quotes (and protection, if needed) around static data

		// Sub data
		for (int index = EQ::invbag::SLOT_BEGIN; index <= invbag::SLOT_END; ++index) {
			ob << '|';

			EQ::ItemInstance *sub = inst->GetItem(index);
			if (!sub)
				continue;

			SerializeItem(ob, sub, 0, (depth + 1));
		}

		ob << StringFormat(
			"%.*s%s",
			(depth ? (depth - 1) : 0),
			protection,
			(depth ? "\"" : "")); // For trailing quotes (and protection) if a subitem;

		if (!depth)
			ob.write("\0", 1);
	}

	static inline int16 ServerToTitaniumSlot(uint32 server_slot) {
		int16 titanium_slot = invslot::SLOT_INVALID;

		if (server_slot <= EQ::invslot::slotWaist) {
			titanium_slot = server_slot;
		}
		else if (server_slot == EQ::invslot::slotAmmo) {
			titanium_slot = server_slot - 1;
		}
		else if (server_slot <= EQ::invslot::slotGeneral8 && server_slot >= EQ::invslot::slotGeneral1) {
			titanium_slot = server_slot - 1;
		}
		else if (server_slot <= (EQ::invslot::POSSESSIONS_COUNT + EQ::invslot::slotWaist) &&
				 server_slot >= EQ::invslot::slotCursor) {
			titanium_slot = server_slot - 3;
		}
		else if (server_slot == (EQ::invslot::POSSESSIONS_COUNT + EQ::invslot::slotAmmo)) {
			titanium_slot = server_slot - 4;
		}
		else if (server_slot <= EQ::invbag::GENERAL_BAGS_END &&
				 server_slot >= EQ::invbag::GENERAL_BAGS_BEGIN) {
			titanium_slot = server_slot - (EQ::invbag::GENERAL_BAGS_BEGIN - invbag::GENERAL_BAGS_BEGIN) - ((EQ::invbag::SLOT_COUNT - invbag::SLOT_COUNT) * ((server_slot - EQ::invbag::GENERAL_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT));
		}
		else if (server_slot <= EQ::invbag::CURSOR_BAG_END && server_slot >= EQ::invbag::CURSOR_BAG_BEGIN) {
			titanium_slot = server_slot - (EQ::invbag::CURSOR_BAG_BEGIN - invbag::CURSOR_BAG_BEGIN);
		}
		else if (server_slot <= EQ::invslot::TRIBUTE_END && server_slot >= EQ::invslot::TRIBUTE_BEGIN) {
			titanium_slot = server_slot;
		}
		else if (server_slot <= EQ::invslot::GUILD_TRIBUTE_END &&
				 server_slot >= EQ::invslot::GUILD_TRIBUTE_BEGIN) {
			titanium_slot = server_slot;
		}
		else if (server_slot == EQ::invslot::SLOT_TRADESKILL_EXPERIMENT_COMBINE) {
			titanium_slot = server_slot;
		}
		else if (server_slot <= EQ::invslot::BANK_END && server_slot >= EQ::invslot::BANK_BEGIN) {
			titanium_slot = server_slot;
		}
		else if (server_slot <= EQ::invbag::BANK_BAGS_END && server_slot >= EQ::invbag::BANK_BAGS_BEGIN) {
			titanium_slot = server_slot - (EQ::invbag::BANK_BAGS_BEGIN - invbag::BANK_BAGS_BEGIN) - ((EQ::invbag::SLOT_COUNT - invbag::SLOT_COUNT) * ((server_slot - EQ::invbag::BANK_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT));
		}
		else if (server_slot <= EQ::invslot::SHARED_BANK_END && server_slot >= EQ::invslot::SHARED_BANK_BEGIN) {
			titanium_slot = server_slot;
		}
		else if (server_slot <= EQ::invbag::SHARED_BANK_BAGS_END &&
				 server_slot >= EQ::invbag::SHARED_BANK_BAGS_BEGIN) {
			titanium_slot = server_slot - (EQ::invbag::SHARED_BANK_BAGS_BEGIN - invbag::SHARED_BANK_BAGS_BEGIN) - ((EQ::invbag::SLOT_COUNT - invbag::SLOT_COUNT) * ((server_slot - EQ::invbag::SHARED_BANK_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT));
		}
		else if (server_slot <= EQ::invslot::TRADE_END && server_slot >= EQ::invslot::TRADE_BEGIN) {
			titanium_slot = server_slot;
		}
		else if (server_slot <= EQ::invbag::TRADE_BAGS_END && server_slot >= EQ::invbag::TRADE_BAGS_BEGIN) {
			titanium_slot = server_slot - (EQ::invbag::TRADE_BAGS_BEGIN - invbag::TRADE_BAGS_BEGIN) - ((EQ::invbag::SLOT_COUNT - invbag::SLOT_COUNT) * ((server_slot - EQ::invbag::TRADE_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT));
		}
		else if (server_slot <= EQ::invslot::WORLD_END && server_slot >= EQ::invslot::WORLD_BEGIN) {
			titanium_slot = server_slot;
		}

		LogNetcode("Convert Server Slot [{}] to Titanium Slot [{}]", server_slot, titanium_slot);

		return titanium_slot;
	}


	static inline int16 ServerToTitaniumCorpseSlot(uint32 server_corpse_slot) {
		int16 titanium_slot = invslot::SLOT_INVALID;

		if (server_corpse_slot <= EQ::invslot::slotGeneral8 && server_corpse_slot >= EQ::invslot::slotGeneral1) {
			titanium_slot = server_corpse_slot - 1;
		}

		else if (server_corpse_slot <= (EQ::invslot::POSSESSIONS_COUNT + EQ::invslot::slotWaist) &&
				 server_corpse_slot >= EQ::invslot::slotCursor) {
			titanium_slot = server_corpse_slot - 3;
		}

		else if (server_corpse_slot == (EQ::invslot::POSSESSIONS_COUNT + EQ::invslot::slotAmmo)) {
			titanium_slot = server_corpse_slot - 4;
		}

		Log(Logs::Detail,
			Logs::Netcode,
			"Convert Server Corpse Slot %i to Titanium Corpse Slot %i",
			server_corpse_slot,
			titanium_slot);

		return titanium_slot;
	}

	static inline uint32 TitaniumToServerSlot(int16 titanium_slot) {
		uint32 server_slot = EQ::invslot::SLOT_INVALID;

		if (titanium_slot <= invslot::slotWaist) {
			server_slot = titanium_slot;
		}
		else if (titanium_slot == invslot::slotAmmo) {
			server_slot = titanium_slot + 1;
		}
		else if (titanium_slot <= invslot::slotGeneral8 && titanium_slot >= invslot::slotGeneral1) {
			server_slot = titanium_slot + 1;
		}
		else if (titanium_slot <= (invslot::POSSESSIONS_COUNT + invslot::slotWaist) &&
				 titanium_slot >= invslot::slotCursor) {
			server_slot = titanium_slot + 3;
		}
		else if (titanium_slot == (invslot::POSSESSIONS_COUNT + invslot::slotAmmo)) {
			server_slot = titanium_slot + 4;
		}
		else if (titanium_slot <= invbag::GENERAL_BAGS_END && titanium_slot >= invbag::GENERAL_BAGS_BEGIN) {
			server_slot = titanium_slot + (EQ::invbag::GENERAL_BAGS_BEGIN - invbag::GENERAL_BAGS_BEGIN) + ((EQ::invbag::SLOT_COUNT - invbag::SLOT_COUNT) * ((titanium_slot - invbag::GENERAL_BAGS_BEGIN) / invbag::SLOT_COUNT));
		}
		else if (titanium_slot <= invbag::CURSOR_BAG_END && titanium_slot >= invbag::CURSOR_BAG_BEGIN) {
			server_slot = titanium_slot + (EQ::invbag::CURSOR_BAG_BEGIN - invbag::CURSOR_BAG_BEGIN);
		}
		else if (titanium_slot <= invslot::TRIBUTE_END && titanium_slot >= invslot::TRIBUTE_BEGIN) {
			server_slot = titanium_slot;
		}
		else if (titanium_slot == invslot::SLOT_TRADESKILL_EXPERIMENT_COMBINE) {
			server_slot = titanium_slot;
		}
		else if (titanium_slot <= invslot::GUILD_TRIBUTE_END && titanium_slot >= invslot::GUILD_TRIBUTE_BEGIN) {
			server_slot = titanium_slot;
		}
		else if (titanium_slot <= invslot::BANK_END && titanium_slot >= invslot::BANK_BEGIN) {
			server_slot = titanium_slot;
		}
		else if (titanium_slot <= invbag::BANK_BAGS_END && titanium_slot >= invbag::BANK_BAGS_BEGIN) {
			server_slot = titanium_slot + (EQ::invbag::BANK_BAGS_BEGIN - invbag::BANK_BAGS_BEGIN) + ((EQ::invbag::SLOT_COUNT - invbag::SLOT_COUNT) * ((titanium_slot - invbag::BANK_BAGS_BEGIN) / invbag::SLOT_COUNT));
		}
		else if (titanium_slot <= invslot::SHARED_BANK_END && titanium_slot >= invslot::SHARED_BANK_BEGIN) {
			server_slot = titanium_slot;
		}
		else if (titanium_slot <= invbag::SHARED_BANK_BAGS_END && titanium_slot >= invbag::SHARED_BANK_BAGS_BEGIN) {
			server_slot = titanium_slot + (EQ::invbag::SHARED_BANK_BAGS_BEGIN - invbag::SHARED_BANK_BAGS_BEGIN) + ((EQ::invbag::SLOT_COUNT - invbag::SLOT_COUNT) * ((titanium_slot - invbag::SHARED_BANK_BAGS_BEGIN) / invbag::SLOT_COUNT));
		}
		else if (titanium_slot <= invslot::TRADE_END && titanium_slot >= invslot::TRADE_BEGIN) {
			server_slot = titanium_slot;
		}
		else if (titanium_slot <= invbag::TRADE_BAGS_END && titanium_slot >= invbag::TRADE_BAGS_BEGIN) {
			server_slot = titanium_slot + (EQ::invbag::TRADE_BAGS_BEGIN - invbag::TRADE_BAGS_BEGIN) + ((EQ::invbag::SLOT_COUNT - invbag::SLOT_COUNT) * ((titanium_slot - invbag::TRADE_BAGS_BEGIN) / invbag::SLOT_COUNT));
		}
		else if (titanium_slot <= invslot::WORLD_END && titanium_slot >= invslot::WORLD_BEGIN) {
			server_slot = titanium_slot;
		}

		LogNetcode("Convert Titanium Slot [{}] to Server Slot [{}]", titanium_slot, server_slot);

		return server_slot;
	}

	static inline uint32 TitaniumToServerCorpseSlot(int16 titanium_corpse_slot)
	{
		uint32 server_slot = EQ::invslot::SLOT_INVALID;

		if (titanium_corpse_slot <= invslot::slotGeneral8 && titanium_corpse_slot >= invslot::slotGeneral1) {
			server_slot = titanium_corpse_slot + 1;
		}

		else if (titanium_corpse_slot <= (invslot::POSSESSIONS_COUNT + invslot::slotWaist) && titanium_corpse_slot >= invslot::slotCursor) {
			server_slot = titanium_corpse_slot + 3;
		}

		else if (titanium_corpse_slot == (invslot::POSSESSIONS_COUNT + invslot::slotAmmo)) {
			server_slot = titanium_corpse_slot + 4;
		}

		LogNetcode("Convert Titanium Corpse Slot [{}] to Server Corpse Slot [{}]", titanium_corpse_slot, server_slot);

		return server_slot;
	}

	static inline void ServerToTitaniumSayLink(std::string &titanium_saylink, const std::string &server_saylink)
	{
		if ((constants::SAY_LINK_BODY_SIZE == EQ::constants::SAY_LINK_BODY_SIZE) || (server_saylink.find('\x12') == std::string::npos)) {
			titanium_saylink = server_saylink;
			return;
		}

		auto segments = Strings::Split(server_saylink, '\x12');

		for (size_t segment_iter = 0; segment_iter < segments.size(); ++segment_iter) {
			if (segment_iter & 1) {
				if (segments[segment_iter].length() <= EQ::constants::SAY_LINK_BODY_SIZE) {
					titanium_saylink.append(segments[segment_iter]);
					// TODO: log size mismatch error
					continue;
				}

				// Idx:  0 1     6     11    16    21    26    31    36 37   41 43    48       (Source)
				// RoF2: X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX X  XXXX XX XXXXX XXXXXXXX (56)
				// 6.2:  X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX       X  XXXX  X       XXXXXXXX (45)
				// Diff:                                       ^^^^^         ^  ^^^^^

				titanium_saylink.push_back('\x12');
				titanium_saylink.append(segments[segment_iter].substr(0, 31));
				titanium_saylink.append(segments[segment_iter].substr(36, 5));

				if (segments[segment_iter][41] == '0')
					titanium_saylink.push_back(segments[segment_iter][42]);
				else
					titanium_saylink.push_back('F');

				titanium_saylink.append(segments[segment_iter].substr(48));
				titanium_saylink.push_back('\x12');
			}
			else {
				titanium_saylink.append(segments[segment_iter]);
			}
		}
	}

	static inline void TitaniumToServerSayLink(std::string &server_saylink, const std::string &titanium_saylink)
	{
		if ((EQ::constants::SAY_LINK_BODY_SIZE == constants::SAY_LINK_BODY_SIZE) || (titanium_saylink.find('\x12') == std::string::npos)) {
			server_saylink = titanium_saylink;
			return;
		}

		auto segments = Strings::Split(titanium_saylink, '\x12');

		for (size_t segment_iter = 0; segment_iter < segments.size(); ++segment_iter) {
			if (segment_iter & 1) {
				if (segments[segment_iter].length() <= constants::SAY_LINK_BODY_SIZE) {
					server_saylink.append(segments[segment_iter]);
					// TODO: log size mismatch error
					continue;
				}

				// Idx:  0 1     6     11    16    21    26          31 32    36       37       (Source)
				// 6.2:  X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX       X  XXXX  X        XXXXXXXX (45)
				// RoF2: X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX X  XXXX XX  XXXXX XXXXXXXX (56)
				// Diff:                                       ^^^^^         ^   ^^^^^

				server_saylink.push_back('\x12');
				server_saylink.append(segments[segment_iter].substr(0, 31));
				server_saylink.append("00000");
				server_saylink.append(segments[segment_iter].substr(31, 5));
				server_saylink.push_back('0');
				server_saylink.push_back(segments[segment_iter][36]);
				server_saylink.append("00000");
				server_saylink.append(segments[segment_iter].substr(37));
				server_saylink.push_back('\x12');
			}
			else {
				server_saylink.append(segments[segment_iter]);
			}
		}
	}

	static inline spells::CastingSlot ServerToTitaniumCastingSlot(EQ::spells::CastingSlot slot) {
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

	static inline EQ::spells::CastingSlot TitaniumToServerCastingSlot(spells::CastingSlot slot, uint32 item_location) {
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

	static inline int ServerToTitaniumBuffSlot(int index)
	{
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

	static inline int TitaniumToServerBuffSlot(int index)
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
} /*Titanium*/
