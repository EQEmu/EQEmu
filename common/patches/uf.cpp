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

#include "uf.h"
#include "uf_structs.h"

#include "common/classes.h"
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

#include "cereal/types/vector.hpp"
#include <iostream>
#include <sstream>


namespace UF
{
	static const char *name = "UF";
	static OpcodeManager *opcodes = nullptr;
	static Strategy struct_strategy;

	void SerializeItem(EQ::OutBuffer& ob, const EQ::ItemInstance *inst, int16 slot_id, uint8 depth);

	// server to client inventory location converters
	static inline uint32 ServerToUFSlot(uint32 serverSlot);
	static inline uint32 ServerToUFCorpseSlot(uint32 serverCorpseSlot);

	// client to server inventory location converters
	static inline uint32 UFToServerSlot(uint32 ufSlot);
	static inline uint32 UFToServerCorpseSlot(uint32 ufCorpseSlot);

	// server to client say link converter
	static inline void ServerToUFSayLink(std::string& ufSayLink, const std::string& serverSayLink);

	// client to server say link converter
	static inline void UFToServerSayLink(std::string& serverSayLink, const std::string& ufSayLink);

	static inline spells::CastingSlot ServerToUFCastingSlot(EQ::spells::CastingSlot slot);
	static inline EQ::spells::CastingSlot UFToServerCastingSlot(spells::CastingSlot slot);

	static inline int ServerToUFBuffSlot(int index);
	static inline int UFToServerBuffSlot(int index);

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
#include "uf_ops.h"
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
		return EQ::versions::ClientVersion::UF;
	}

#include "ss_define.h"

// ENCODE methods
	ENCODE(OP_Action)
	{
		ENCODE_LENGTH_EXACT(Action_Struct);
		SETUP_DIRECT_ENCODE(Action_Struct, structs::ActionAlt_Struct);

		OUT(target);
		OUT(source);
		OUT(level);
		eq->instrument_mod = 1.0f + (emu->instrument_mod - 10) / 10.0f;
		OUT(force);
		OUT(hit_heading);
		OUT(hit_pitch);
		OUT(type);
		OUT(spell);
		OUT(spell_level);
		OUT(effect_flag);
		eq->spell_gem = 0;
		eq->slot[0] = -1; // type
		eq->slot[1] = -1; // slot
		eq->slot[2] = -1; // sub index
		eq->slot[3] = -1; // aug index
		eq->slot[4] = -1; // unknown
		eq->item_cast_type = 0;

		FINISH_ENCODE();
	}

	ENCODE(OP_AdventureMerchantSell)
	{
		ENCODE_LENGTH_EXACT(Adventure_Sell_Struct);
		SETUP_DIRECT_ENCODE(Adventure_Sell_Struct, structs::Adventure_Sell_Struct);

		eq->unknown000 = 1;
		OUT(npcid);
		eq->slot = ServerToUFSlot(emu->slot);
		OUT(charges);
		OUT(sell_price);

		FINISH_ENCODE();
	}

	ENCODE(OP_AltCurrency)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		unsigned char *emu_buffer = in->pBuffer;
		uint32 opcode = *((uint32*)emu_buffer);

		if (opcode == AlternateCurrencyMode::Populate) {
			AltCurrencyPopulate_Struct *populate = (AltCurrencyPopulate_Struct*)emu_buffer;

			auto outapp = new EQApplicationPacket(
			    OP_AltCurrency, sizeof(structs::AltCurrencyPopulate_Struct) +
						sizeof(structs::AltCurrencyPopulateEntry_Struct) * populate->count);
			structs::AltCurrencyPopulate_Struct *out_populate = (structs::AltCurrencyPopulate_Struct*)outapp->pBuffer;

			out_populate->opcode = populate->opcode;
			out_populate->count = populate->count;
			for (uint32 i = 0; i < populate->count; ++i) {
				out_populate->entries[i].currency_number = populate->entries[i].currency_number;
				out_populate->entries[i].currency_number2 = populate->entries[i].currency_number2;
				out_populate->entries[i].item_id = populate->entries[i].item_id;
				out_populate->entries[i].item_icon = populate->entries[i].item_icon;
				out_populate->entries[i].stack_size = populate->entries[i].stack_size;
				out_populate->entries[i].unknown00 = populate->entries[i].unknown00;
			}

			dest->FastQueuePacket(&outapp, ack_req);
		}
		else {
			auto outapp = new EQApplicationPacket(OP_AltCurrency, sizeof(AltCurrencyUpdate_Struct));
			memcpy(outapp->pBuffer, emu_buffer, sizeof(AltCurrencyUpdate_Struct));
			dest->FastQueuePacket(&outapp, ack_req);
		}

		//dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	ENCODE(OP_AltCurrencySell)
	{
		ENCODE_LENGTH_EXACT(AltCurrencySellItem_Struct);
		SETUP_DIRECT_ENCODE(AltCurrencySellItem_Struct, structs::AltCurrencySellItem_Struct);

		OUT(merchant_entity_id);
		eq->slot_id = ServerToUFSlot(emu->slot_id);
		OUT(charges);
		OUT(cost);

		FINISH_ENCODE();
	}

	ENCODE(OP_ApplyPoison)
	{
		ENCODE_LENGTH_EXACT(ApplyPoison_Struct);
		SETUP_DIRECT_ENCODE(ApplyPoison_Struct, structs::ApplyPoison_Struct);

		eq->inventorySlot = ServerToUFSlot(emu->inventorySlot);
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

	ENCODE(OP_Barter)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		char *Buffer = (char *)in->pBuffer;

		uint32 SubAction = VARSTRUCT_DECODE_TYPE(uint32, Buffer);

		if (SubAction != Barter_BuyerAppearance)
		{
			dest->FastQueuePacket(&in, ack_req);

			return;
		}

		unsigned char *__emu_buffer = in->pBuffer;

		in->size = 80;

		in->pBuffer = new unsigned char[in->size];

		char *OutBuffer = (char *)in->pBuffer;

		char Name[64];

		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, SubAction);
		uint32 EntityID = VARSTRUCT_DECODE_TYPE(uint32, Buffer);
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, EntityID);
		uint8 Toggle = VARSTRUCT_DECODE_TYPE(uint8, Buffer);
		VARSTRUCT_DECODE_STRING(Name, Buffer);
		VARSTRUCT_ENCODE_STRING(OutBuffer, Name);
		OutBuffer = (char *)in->pBuffer + 72;
		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, Toggle);

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_BazaarSearch)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		uint32 action = *(uint32 *)in->pBuffer;

		switch (action) {
			case BazaarSearch: {
				LogTrading(
					"Encode OP_BazaarSearch(UF) BazaarSearch action <green>[{}]",
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

				auto  size    = results.size() * sizeof(BazaarSearchResults_Struct);
				auto  buffer  = new uchar[size];
				uchar *bufptr = buffer;
				memset(buffer, 0, size);

				for (auto row = results.begin(); row != results.end(); ++row) {
					VARSTRUCT_ENCODE_TYPE(uint32, bufptr, structs::UFBazaarTraderBuyerActions::BazaarSearch);
					VARSTRUCT_ENCODE_TYPE(uint32, bufptr, row->trader_entity_id);
					strn0cpy(reinterpret_cast<char *>(bufptr), row->trader_name.c_str(), 64);
					bufptr += 64;
					VARSTRUCT_ENCODE_TYPE(uint32, bufptr, 1);
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
					"Encode OP_BazaarSearch(UF) BazaarInspect/WelcomeMessage action <green>[{}]",
					action
				);
				dest->FastQueuePacket(&in, ack_req);
				break;
			}
			default: {
				LogTrading(
					"Encode OP_BazaarSearch(UF) unhandled action <red>[{}]",
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
                    "Encode OP_BecomeTrader(UF) TraderOff action <green>[{}] entity_id <green>[{}] trader_name "
                    "<green>[{}]",
                    emu->action,
                    emu->entity_id,
                    emu->trader_name
                );
				eq->action    = structs::UFBazaarTraderBuyerActions::Zero;
				eq->entity_id = emu->entity_id;
				FINISH_ENCODE();
				break;
			}
			case TraderOn:
			{
				ENCODE_LENGTH_EXACT(BecomeTrader_Struct);
				SETUP_DIRECT_ENCODE(BecomeTrader_Struct, structs::BecomeTrader_Struct);
				LogTrading(
                    "Encode OP_BecomeTrader(UF) TraderOn action <green>[{}] entity_id <green>[{}] trader_name "
                    "<green>[{}]",
                    emu->action,
                    emu->entity_id,
                    emu->trader_name
                );
				eq->action    = structs::UFBazaarTraderBuyerActions::BeginTraderMode;
				eq->entity_id = emu->entity_id;
				strn0cpy(eq->trader_name, emu->trader_name, sizeof(eq->trader_name));
				FINISH_ENCODE();
				break;
			}
			default:
			{
				LogTrading(
					"Encode OP_BecomeTrader(UF) unhandled action <red>[{}] Sending packet as is.",
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
		// just so we're 100% sure we get a 1.0f ...
		eq->buff.bard_modifier = emu->buff.bard_modifier == 10 ? 1.0f : emu->buff.bard_modifier / 10.0f;
		OUT(buff.spellid);
		OUT(buff.duration);
		OUT(buff.num_hits);
		// TODO: implement slot_data stuff
		eq->slotid = ServerToUFBuffSlot(emu->slotid);
		OUT(bufffade);	// Live (October 2011) sends a 2 rather than 0 when a buff is created, but it doesn't seem to matter.

		FINISH_ENCODE();
	}

	ENCODE(OP_BuffCreate)
	{
		SETUP_VAR_ENCODE(BuffIcon_Struct);

		uint32 sz = 12 + (17 * emu->count) + emu->name_lengths; // 17 includes nullterm
		__packet->size = sz;
		__packet->pBuffer = new unsigned char[sz];
		memset(__packet->pBuffer, 0, sz);

		__packet->WriteUInt32(emu->entity_id);
		__packet->WriteUInt32(emu->tic_timer);
		__packet->WriteUInt8(emu->all_buffs); // 1 = all buffs, 0 = 1 buff
		__packet->WriteUInt16(emu->count);

		for (int i = 0; i < emu->count; ++i)
		{
			__packet->WriteUInt32(emu->type == 0 ? ServerToUFBuffSlot(emu->entries[i].buff_slot) : emu->entries[i].buff_slot);
			__packet->WriteUInt32(emu->entries[i].spell_id);
			__packet->WriteUInt32(emu->entries[i].tics_remaining);
			__packet->WriteUInt32(emu->entries[i].num_hits);
			__packet->WriteString(emu->entries[i].caster);
		}
		__packet->WriteUInt8(emu->type);

		FINISH_ENCODE();
		/*
		uint32 write_var32 = 60;
		uint8 write_var8 = 1;
		ss.write((const char*)&emu->entity_id, sizeof(uint32));
		ss.write((const char*)&write_var32, sizeof(uint32));
		ss.write((const char*)&write_var8, sizeof(uint8));
		ss.write((const char*)&emu->count, sizeof(uint16));
		write_var32 = 0;
		write_var8 = 0;
		for(uint16 i = 0; i < emu->count; ++i)
		{
		if(emu->entries[i].buff_slot >= 25 && emu->entries[i].buff_slot < 37)
		{
		emu->entries[i].buff_slot += 5;
		}
		else if(emu->entries[i].buff_slot >= 37)
		{
		emu->entries[i].buff_slot += 14;
		}
		ss.write((const char*)&emu->entries[i].buff_slot, sizeof(uint32));
		ss.write((const char*)&emu->entries[i].spell_id, sizeof(uint32));
		ss.write((const char*)&emu->entries[i].tics_remaining, sizeof(uint32));
		ss.write((const char*)&write_var32, sizeof(uint32));
		ss.write((const char*)&write_var8, sizeof(uint8));
		}
		ss.write((const char*)&write_var8, sizeof(uint8));
		*/
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
		ServerToUFSayLink(new_message, old_message);

		//in->size = strlen(emu->sender) + 1 + strlen(emu->targetname) + 1 + strlen(emu->message) + 1 + 36;
		in->size = strlen(emu->sender) + strlen(emu->targetname) + new_message.length() + 39;

		in->pBuffer = new unsigned char[in->size];

		char *OutBuffer = (char *)in->pBuffer;

		VARSTRUCT_ENCODE_STRING(OutBuffer, emu->sender);
		VARSTRUCT_ENCODE_STRING(OutBuffer, emu->targetname);
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);	// Unknown
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->language);
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->chan_num);
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);	// Unknown
		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, 0);	// Unknown
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->skill_in_language);
		VARSTRUCT_ENCODE_STRING(OutBuffer, new_message.c_str());

		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);	// Unknown
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);	// Unknown
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);	// Unknown
		VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, 0);	// Unknown
		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, 0);	// Unknown

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
				LogNetcode("UF::ENCODE(OP_CharInventory) Serialization failed on item slot [{}] during OP_CharInventory.  Item skipped", eq->slot_id);

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
		OUT(special);

		FINISH_ENCODE();
	}

	ENCODE(OP_DeleteCharge)
	{
		Log(Logs::Detail, Logs::Netcode, "UF::ENCODE(OP_DeleteCharge)");

		ENCODE_FORWARD(OP_MoveItem);
	}

	ENCODE(OP_DeleteItem)
	{
		ENCODE_LENGTH_EXACT(DeleteItem_Struct);
		SETUP_DIRECT_ENCODE(DeleteItem_Struct, structs::DeleteItem_Struct);

		eq->from_slot = ServerToUFSlot(emu->from_slot);
		eq->to_slot = ServerToUFSlot(emu->to_slot);
		OUT(number_in_stack);

		FINISH_ENCODE();
	}

	ENCODE(OP_DisciplineUpdate)
	{
		ENCODE_LENGTH_EXACT(Disciplines_Struct);
		SETUP_DIRECT_ENCODE(Disciplines_Struct, structs::Disciplines_Struct);

		memcpy(&eq->values, &emu->values, sizeof(Disciplines_Struct));

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
		ServerToUFSayLink(new_message, old_message);

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
			ServerToUFSayLink(new_message_array[i], old_message_array[i]);
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
		// We are not encoding the spawn_id field here, or a size but it doesn't appear to matter.
		//
		EQApplicationPacket *in = *p;
		*p = nullptr;

		Object_Struct *emu = (Object_Struct *)in->pBuffer;
		unsigned char *__emu_buffer = in->pBuffer;
		in->size = strlen(emu->object_name) + 58;
		in->pBuffer = new unsigned char[in->size];
		char *OutBuffer = (char *)in->pBuffer;

		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->drop_id);
		VARSTRUCT_ENCODE_STRING(OutBuffer, emu->object_name);
		VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, emu->zone_id);
		VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, emu->zone_instance);
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);	// Unknown, observed 0x00006762
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);	// Unknown, observer 0x7fffbb64
		VARSTRUCT_ENCODE_TYPE(float, OutBuffer, emu->heading);
		// This next field is actually a float. There is a groundspawn in freeportwest (sack of money sitting on some barrels) which requires this
		// field to be set to (float)255.0 to appear at all, and also the size field below to be 5, to be the correct size. I think SoD has the same
		// issue.
		VARSTRUCT_ENCODE_TYPE(float, OutBuffer, emu->tilt_x); //X tilt
		VARSTRUCT_ENCODE_TYPE(float, OutBuffer, emu->tilt_y);	//Y tilt
		VARSTRUCT_ENCODE_TYPE(float, OutBuffer, emu->size != 0 && (float)emu->size < 5000.f ? (float)((float)emu->size / 100.0f) : 1.f );	// This appears to be the size field. Hackish logic because some PEQ DB items were corrupt.
		VARSTRUCT_ENCODE_TYPE(float, OutBuffer, emu->y);
		VARSTRUCT_ENCODE_TYPE(float, OutBuffer, emu->x);
		VARSTRUCT_ENCODE_TYPE(float, OutBuffer, emu->z);
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->object_type);	// Unknown, observed 0x00000014
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0xffffffff);	// Unknown, observed 0xffffffff
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);	// Unknown, observed 0x00000014
		VARSTRUCT_ENCODE_TYPE(uint8, OutBuffer, 0);	// Unknown, observed 0x00

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_GroupCancelInvite)
	{
		ENCODE_LENGTH_EXACT(GroupCancel_Struct);
		SETUP_DIRECT_ENCODE(GroupCancel_Struct, structs::GroupCancel_Struct);

		memcpy(eq->name1, emu->name1, sizeof(eq->name1));
		memcpy(eq->name2, emu->name2, sizeof(eq->name2));
		OUT(toggle);

		FINISH_ENCODE();
	}

	ENCODE(OP_GroupFollow)
	{
		ENCODE_LENGTH_EXACT(GroupGeneric_Struct);
		SETUP_DIRECT_ENCODE(GroupGeneric_Struct, structs::GroupFollow_Struct);

		memcpy(eq->name1, emu->name1, sizeof(eq->name1));
		memcpy(eq->name2, emu->name2, sizeof(eq->name2));

		FINISH_ENCODE();
	}

	ENCODE(OP_GroupFollow2)
	{
		ENCODE_LENGTH_EXACT(GroupGeneric_Struct);
		SETUP_DIRECT_ENCODE(GroupGeneric_Struct, structs::GroupFollow_Struct);

		memcpy(eq->name1, emu->name1, sizeof(eq->name1));
		memcpy(eq->name2, emu->name2, sizeof(eq->name2));

		FINISH_ENCODE();
	}

	ENCODE(OP_GroupInvite)
	{
		ENCODE_LENGTH_EXACT(GroupGeneric_Struct);
		SETUP_DIRECT_ENCODE(GroupGeneric_Struct, structs::GroupInvite_Struct);

		memcpy(eq->invitee_name, emu->name1, sizeof(eq->invitee_name));
		memcpy(eq->inviter_name, emu->name2, sizeof(eq->inviter_name));

		FINISH_ENCODE();
	}

	ENCODE(OP_GroupUpdate)
	{
		//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] OP_GroupUpdate");
		EQApplicationPacket *in = *p;
		GroupJoin_Struct *gjs = (GroupJoin_Struct*)in->pBuffer;

		//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Received outgoing OP_GroupUpdate with action code %i", gjs->action);
		if ((gjs->action == groupActLeave) || (gjs->action == groupActDisband))
		{
			if ((gjs->action == groupActDisband) || !strcmp(gjs->yourname, gjs->membername))
			{
				//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Group Leave, yourname = %s, member_name = %s", gjs->yourname, gjs->member_name);

				auto outapp =
				    new EQApplicationPacket(OP_GroupDisbandYou, sizeof(structs::GroupGeneric_Struct));

				structs::GroupGeneric_Struct *ggs = (structs::GroupGeneric_Struct*)outapp->pBuffer;
				memcpy(ggs->name1, gjs->yourname, sizeof(ggs->name1));
				memcpy(ggs->name2, gjs->membername, sizeof(ggs->name1));
				dest->FastQueuePacket(&outapp);

				// Make an empty GLAA packet to clear out their useable GLAAs
				//
				outapp = new EQApplicationPacket(OP_GroupLeadershipAAUpdate, sizeof(GroupLeadershipAAUpdate_Struct));

				dest->FastQueuePacket(&outapp);

				delete in;
				return;
			}
			//if(gjs->action == groupActLeave)
			//	Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Group Leave, yourname = %s, member_name = %s", gjs->yourname, gjs->member_name);

			auto outapp =
			    new EQApplicationPacket(OP_GroupDisbandOther, sizeof(structs::GroupGeneric_Struct));

			structs::GroupGeneric_Struct *ggs = (structs::GroupGeneric_Struct*)outapp->pBuffer;
			memcpy(ggs->name1, gjs->yourname, sizeof(ggs->name1));
			memcpy(ggs->name2, gjs->membername, sizeof(ggs->name2));
			//Log.Hex(Logs::Netcode, outapp->pBuffer, outapp->size);
			dest->FastQueuePacket(&outapp);

			delete in;
			return;
		}

		if (in->size == sizeof(GroupUpdate2_Struct))
		{
			// Group Update2
			//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Struct is GroupUpdate2");

			unsigned char *__emu_buffer = in->pBuffer;
			GroupUpdate2_Struct *gu2 = (GroupUpdate2_Struct*)__emu_buffer;

			//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Yourname is %s", gu2->yourname);

			int MemberCount = 1;
			int PacketLength = 8 + strlen(gu2->leadersname) + 1 + 22 + strlen(gu2->yourname) + 1;

			for (int i = 0; i < 5; ++i)
			{
				//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Membername[%i] is %s", i,  gu2->member_name[i]);
				if (gu2->membername[i][0] != '\0')
				{
					PacketLength += (22 + strlen(gu2->membername[i]) + 1);
					++MemberCount;
				}
			}

			//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Leadername is %s", gu2->leadersname);

			auto outapp = new EQApplicationPacket(OP_GroupUpdateB, PacketLength);

			char *Buffer = (char *)outapp->pBuffer;

			// Header
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);	// Think this should be SpawnID, but it doesn't seem to matter
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, MemberCount);
			VARSTRUCT_ENCODE_STRING(Buffer, gu2->leadersname);

			// Leader
			//

			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
			VARSTRUCT_ENCODE_STRING(Buffer, gu2->yourname);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);
			//VARSTRUCT_ENCODE_STRING(Buffer, "");
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// This is a string
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0x46);	// Observed 0x41 and 0x46 here
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
			VARSTRUCT_ENCODE_TYPE(uint16, Buffer, 0);

			int MemberNumber = 1;

			for (int i = 0; i < 5; ++i)
			{
				if (gu2->membername[i][0] == '\0')
					continue;

				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, MemberNumber++);
				VARSTRUCT_ENCODE_STRING(Buffer, gu2->membername[i]);
				VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);
				//VARSTRUCT_ENCODE_STRING(Buffer, "");
				VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// This is a string
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0x41);	// Observed 0x41 and 0x46 here
				VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);	// Low byte is Main Assist Flag
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint16, Buffer, 0);
			}

			//Log.Hex(Logs::Netcode, outapp->pBuffer, outapp->size);
			dest->FastQueuePacket(&outapp);

			outapp = new EQApplicationPacket(OP_GroupLeadershipAAUpdate, sizeof(GroupLeadershipAAUpdate_Struct));

			GroupLeadershipAAUpdate_Struct *GLAAus = (GroupLeadershipAAUpdate_Struct*)outapp->pBuffer;

			GLAAus->NPCMarkerID = gu2->NPCMarkerID;
			memcpy(&GLAAus->LeaderAAs, &gu2->leader_aas, sizeof(GLAAus->LeaderAAs));

			dest->FastQueuePacket(&outapp);
			delete in;
			return;
		}
		//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Generic GroupUpdate, yourname = %s, member_name = %s", gjs->yourname, gjs->member_name);
		ENCODE_LENGTH_EXACT(GroupJoin_Struct);
		SETUP_DIRECT_ENCODE(GroupJoin_Struct, structs::GroupJoin_Struct);

		memcpy(eq->membername, emu->membername, sizeof(eq->membername));

		auto outapp =
		    new EQApplicationPacket(OP_GroupLeadershipAAUpdate, sizeof(GroupLeadershipAAUpdate_Struct));
		GroupLeadershipAAUpdate_Struct *GLAAus = (GroupLeadershipAAUpdate_Struct*)outapp->pBuffer;

		GLAAus->NPCMarkerID = emu->NPCMarkerID;

		memcpy(&GLAAus->LeaderAAs, &emu->leader_aas, sizeof(GLAAus->LeaderAAs));
		//Log.Hex(Logs::Netcode, __packet->pBuffer, __packet->size);

		FINISH_ENCODE();

		dest->FastQueuePacket(&outapp);
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
				//Translate older ranks to new values* /
				switch (emu_e->rank) {
					case GUILD_SENIOR_MEMBER:
					case GUILD_MEMBER:
					case GUILD_JUNIOR_MEMBER:
					case GUILD_INITIATE:
					case GUILD_RECRUIT: {
						emu_e->rank = GUILD_MEMBER_TI;
						break;
					}
					case GUILD_OFFICER:
					case GUILD_SENIOR_OFFICER: {
						emu_e->rank = GUILD_OFFICER_TI;
						break;
					}
					case GUILD_LEADER: {
						emu_e->rank = GUILD_LEADER_TI;
						break;
					}
					default: {
						emu_e->rank = GUILD_RANK_NONE_TI;
						break;
					}
				}
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

	ENCODE(OP_GuildsList)
	{
		EQApplicationPacket* in = *p;
		*p = nullptr;

		GuildsListMessaging_Struct   glms{};
		EQ::Util::MemoryStreamReader ss(reinterpret_cast<char *>(in->pBuffer), in->size);
		cereal::BinaryInputArchive   ar(ss);
		ar(glms);

		auto packet_size = 64 + 4 + glms.guild_detail.size() * 4 + glms.string_length;
		auto buffer      = new uchar[packet_size];
		auto buf_pos     = buffer;

		memset(buf_pos, 0, 64);
		buf_pos += 64;

		VARSTRUCT_ENCODE_TYPE(uint32, buf_pos, glms.no_of_guilds);

		for (auto const& g : glms.guild_detail) {
			if (g.guild_id < UF::constants::MAX_GUILD_ID) {
				VARSTRUCT_ENCODE_TYPE(uint32, buf_pos, g.guild_id);
				strn0cpy((char *) buf_pos, g.guild_name.c_str(), g.guild_name.length() + 1);
				buf_pos += g.guild_name.length() + 1;
			}
		}

		auto outapp = new EQApplicationPacket(OP_GuildsList);

		outapp->size    = packet_size;
		outapp->pBuffer = buffer;

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
			default: {
				eq->rank_ = GUILD_RANK_NONE_TI;
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
			default: {
				eq->rank_ = GUILD_RANK_NONE_TI;
				break;
			}
		}
		FINISH_ENCODE()
	}

	ENCODE(OP_SendGuildTributes)
	{
		ENCODE_LENGTH_ATLEAST(structs::GuildTributeAbility_Struct)
		SETUP_VAR_ENCODE(GuildTributeAbility_Struct)
		ALLOC_VAR_ENCODE(structs::GuildTributeAbility_Struct, sizeof(GuildTributeAbility_Struct) + strlen(emu->ability.name))

		eq->guild_id           = emu->guild_id;
		eq->ability.tribute_id = emu->ability.tribute_id;
		eq->ability.tier_count = emu->ability.tier_count;
		strncpy(eq->ability.name, emu->ability.name, strlen(emu->ability.name));
		for (int i = 0; i < ntohl(emu->ability.tier_count); i++) {
			eq->ability.tiers[i].cost            = emu->ability.tiers[i].cost;
			eq->ability.tiers[i].level           = emu->ability.tiers[i].level;
			eq->ability.tiers[i].tribute_item_id = emu->ability.tiers[i].tribute_item_id;
		}
		FINISH_ENCODE()
	}

	ENCODE(OP_GuildTributeDonateItem)
	{
		SETUP_DIRECT_ENCODE(GuildTributeDonateItemReply_Struct, structs::GuildTributeDonateItemReply_Struct);

		Log(Logs::Detail, Logs::Netcode, "UF::ENCODE(OP_GuildTributeDonateItem)");

		OUT(quantity)
		OUT(favor)
		eq->unknown8 = 0;
		eq->slot     = ServerToUFSlot(emu->slot);

		FINISH_ENCODE()
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

	ENCODE(OP_InspectBuffs)
	{
		ENCODE_LENGTH_EXACT(InspectBuffs_Struct);
		SETUP_DIRECT_ENCODE(InspectBuffs_Struct, structs::InspectBuffs_Struct);

		// we go over the internal 25 instead of the packet's since no entry is 0, which it will be already
		for (int i = 0; i < BUFF_COUNT; i++) {
			OUT(spell_id[i]);
			OUT(tics_remaining[i]);
		}

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
			LogNetcode("UF::ENCODE(OP_ItemPacket) Serialization failed on item slot [{}]", int_struct->slot_id);
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

		eq->slot = ServerToUFSlot(emu->slot);
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

		eq->unknown016 = 1;
		eq->unknown020[0] = 1;

		// These next two need to be set like this for the Tutorial Button to work.
		eq->unknown263[0] = 0;
		eq->unknown263[2] = 1;
		eq->unknown263[4] = 1;
		eq->unknown263[5] = 1;
		eq->unknown263[6] = 1;
		eq->unknown263[9] = 8;
		eq->unknown263[19] = 0x80;
		eq->unknown263[20] = 0x3f;
		eq->unknown263[23] = 0x80;
		eq->unknown263[24] = 0x3f;
		eq->unknown263[33] = 1;

		FINISH_ENCODE();
	}

	ENCODE(OP_LootItem)
	{
		ENCODE_LENGTH_EXACT(LootingItem_Struct);
		SETUP_DIRECT_ENCODE(LootingItem_Struct, structs::LootingItem_Struct);

		Log(Logs::Detail, Logs::Netcode, "UF::ENCODE(OP_LootItem)");

		OUT(lootee);
		OUT(looter);
		eq->slot_id = ServerToUFCorpseSlot(emu->slot_id);
		OUT(auto_loot);

		FINISH_ENCODE();
	}

	ENCODE(OP_MercenaryDataResponse)
	{
		//consume the packet
		EQApplicationPacket *in = *p;
		*p = nullptr;

		//store away the emu struct
		unsigned char *__emu_buffer = in->pBuffer;
		MercenaryMerchantList_Struct *emu = (MercenaryMerchantList_Struct *)__emu_buffer;

		char *Buffer = (char *)in->pBuffer;

		int PacketSize = sizeof(structs::MercenaryMerchantList_Struct) - 4 + emu->MercTypeCount * 4;
		PacketSize += (sizeof(structs::MercenaryListEntry_Struct) - sizeof(structs::MercenaryStance_Struct)) * emu->MercCount;

		uint32 r;
		uint32 k;
		for (r = 0; r < emu->MercCount; r++)
		{
			PacketSize += sizeof(structs::MercenaryStance_Struct) * emu->Mercs[r].StanceCount;
		}

		auto outapp = new EQApplicationPacket(OP_MercenaryDataResponse, PacketSize);
		Buffer = (char *)outapp->pBuffer;

		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercTypeCount);

		for (r = 0; r < emu->MercTypeCount; r++)
		{
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercGrades[r]);
		}

		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercCount);

		for (r = 0; r < emu->MercCount; r++)
		{
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].MercID);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].MercType);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].MercSubType);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].PurchaseCost);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].UpkeepCost);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].AltCurrencyCost);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].AltCurrencyUpkeep);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].AltCurrencyType);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->Mercs[r].MercUnk01);
			VARSTRUCT_ENCODE_TYPE(int32, Buffer, emu->Mercs[r].TimeLeft);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].MerchantSlot);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].MercUnk02);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].StanceCount);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].MercUnk03);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->Mercs[r].MercUnk04);
			for (k = 0; k < emu->Mercs[r].StanceCount; k++)
			{
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].Stances[k].StanceIndex);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].Stances[k].Stance);
			}
		}

		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	// This packet does not appear to exist in UF, but leaving it here just in case
	ENCODE(OP_MercenaryDataUpdate)
	{
		//consume the packet
		EQApplicationPacket *in = *p;
		*p = nullptr;

		//store away the emu struct
		unsigned char *__emu_buffer = in->pBuffer;
		MercenaryDataUpdate_Struct *emu = (MercenaryDataUpdate_Struct *)__emu_buffer;

		char *Buffer = (char *)in->pBuffer;

		EQApplicationPacket *outapp;

		uint32 PacketSize = 0;

		// There are 2 different sized versions of this packet depending if a merc is hired or not
		if (emu->MercStatus >= 0)
		{
			PacketSize += sizeof(structs::MercenaryDataUpdate_Struct) + (sizeof(structs::MercenaryData_Struct) - sizeof(structs::MercenaryStance_Struct)) * emu->MercCount;

			uint32 r;
			uint32 k;
			for (r = 0; r < emu->MercCount; r++)
			{
				PacketSize += sizeof(structs::MercenaryStance_Struct) * emu->MercData[r].StanceCount;
			}

			outapp = new EQApplicationPacket(OP_MercenaryDataUpdate, PacketSize);
			Buffer = (char *)outapp->pBuffer;

			VARSTRUCT_ENCODE_TYPE(int32, Buffer, emu->MercStatus);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercCount);

			for (r = 0; r < emu->MercCount; r++)
			{
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].MercID);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].MercType);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].MercSubType);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].PurchaseCost);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].UpkeepCost);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].AltCurrencyCost);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].AltCurrencyUpkeep);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].AltCurrencyType);
				VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->MercData[r].MercUnk01);
				VARSTRUCT_ENCODE_TYPE(int32, Buffer, emu->MercData[r].TimeLeft);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].MerchantSlot);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].MercUnk02);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].StanceCount);
				VARSTRUCT_ENCODE_TYPE(int32, Buffer, emu->MercData[r].MercUnk03);
				VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->MercData[r].MercUnk04);
				for (k = 0; k < emu->MercData[r].StanceCount; k++)
				{
					VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].Stances[k].StanceIndex);
					VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].Stances[k].Stance);
				}
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].MercUnk05);
			}
		}
		else
		{
			PacketSize += sizeof(structs::NoMercenaryHired_Struct);

			outapp = new EQApplicationPacket(OP_MercenaryDataUpdate, PacketSize);
			Buffer = (char *)outapp->pBuffer;

			VARSTRUCT_ENCODE_TYPE(int32, Buffer, emu->MercStatus);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercCount);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 1);
		}

		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	ENCODE(OP_MoveItem)
	{
		ENCODE_LENGTH_EXACT(MoveItem_Struct);
		SETUP_DIRECT_ENCODE(MoveItem_Struct, structs::MoveItem_Struct);

		Log(Logs::Detail, Logs::Netcode, "UF::ENCODE(OP_MoveItem)");

		eq->from_slot = ServerToUFSlot(emu->from_slot);
		eq->to_slot = ServerToUFSlot(emu->to_slot);
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

		eq->fog_density = emu->fog_density;

		/*fill in some unknowns with observed values, hopefully it will help */
		eq->unknown800 = -1;
		eq->unknown844 = 600;
		OUT(lava_damage);
		OUT(min_lava_damage);
		eq->unknown888 = 1;
		eq->unknown889 = 0;
		eq->unknown890 = 1;
		eq->unknown891 = 0;
		eq->unknown892 = 0;
		eq->unknown893 = 0;
		eq->fall_damage = 0;	// 0 = Fall Damage on, 1 = Fall Damage off
		eq->unknown895 = 0;
		eq->unknown908 = 2;
		eq->unknown912 = 2;

		FINISH_ENCODE();
	}

	ENCODE(OP_OnLevelMessage)
	{
		ENCODE_LENGTH_EXACT(OnLevelMessage_Struct);
		SETUP_DIRECT_ENCODE(OnLevelMessage_Struct, structs::OnLevelMessage_Struct);

		memcpy(eq->Title, emu->Title, sizeof(eq->Title));
		memcpy(eq->Text, emu->Text, sizeof(eq->Text));
		OUT(Buttons);
		OUT(SoundControls);
		OUT(Duration);
		OUT(PopupID);
		OUT(NegativeID);
		// These two field names are used if Buttons == 1.
		OUT_str(ButtonName0);
		OUT_str(ButtonName1);

		FINISH_ENCODE();
	}

	ENCODE(OP_PetBuffWindow)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		unsigned char *__emu_buffer = in->pBuffer;

		PetBuff_Struct *emu = (PetBuff_Struct *)__emu_buffer;

		int PacketSize = 12 + (emu->buffcount * 17);

		in->size = PacketSize;
		in->pBuffer = new unsigned char[in->size];

		char *Buffer = (char *)in->pBuffer;

		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->petid);
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
		VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 1);
		VARSTRUCT_ENCODE_TYPE(uint16, Buffer, emu->buffcount);

		for (unsigned int i = 0; i < PET_BUFF_COUNT; ++i)
		{
			if (emu->spellid[i])
			{
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, i);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->spellid[i]);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->ticsremaining[i]);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0); // numhits
				VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// This is a string. Name of the caster of the buff.
			}
		}
		VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->buffcount); /// I think this is actually some sort of type

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_PlayerProfile)
	{
		SETUP_DIRECT_ENCODE(PlayerProfile_Struct, structs::PlayerProfile_Struct);

		uint32 r;

		eq->available_slots = 0xffffffff;
		memset(eq->unknown07284, 0xff, sizeof(eq->unknown07284));

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
			eq->aa_array[r].AA = emu->aa_array[r].AA;
			eq->aa_array[r].value = emu->aa_array[r].value;
			eq->aa_array[r].charges = emu->aa_array[r].charges;
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
		//PS this needs to be figured out more; but it was 'good enough'
		for (r = 0; r < BUFF_COUNT; r++)
		{
			if (emu->buffs[r].spellid != 0xFFFF && emu->buffs[r].spellid != 0)
			{
				eq->buffs[r].bard_modifier = 1.0f + (emu->buffs[r].bard_modifier - 10) / 10.0f;
				eq->buffs[r].effect_type= 2;
				eq->buffs[r].player_id = 0x000717fd;
			}
			else
			{
				eq->buffs[r].effect_type = 0;
				eq->buffs[r].bard_modifier = 1.0f;
			}
			OUT(buffs[r].effect_type);
			OUT(buffs[r].level);
			OUT(buffs[r].unknown003);
			OUT(buffs[r].spellid);
			OUT(buffs[r].duration);
			OUT(buffs[r].num_hits);
			OUT(buffs[r].player_id);
		}
		for (r = 0; r < MAX_PP_DISCIPLINES; r++) {
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
		//Translate older ranks to new values* /
		switch (emu->guildrank) {
			case GUILD_SENIOR_MEMBER:
			case GUILD_MEMBER:
			case GUILD_JUNIOR_MEMBER:
			case GUILD_INITIATE:
			case GUILD_RECRUIT: {
				emu->guildrank = GUILD_MEMBER_TI;
				break;
			}
			case GUILD_OFFICER:
			case GUILD_SENIOR_OFFICER: {
				emu->guildrank = GUILD_OFFICER_TI;
				break;
			}
			case GUILD_LEADER: {
				emu->guildrank = GUILD_LEADER_TI;
				break;
			}
			default: {
				emu->guildrank = GUILD_RANK_NONE_TI;
				break;
			}
		}
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
		//eq->expansions = 0x1ffff;
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

		memcpy(eq->unknown18020, bytes, sizeof(bytes));

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
		eq->invslot = ServerToUFSlot(emu->invslot);
		OUT(target_id);
		OUT(can_cast);
		OUT(can_scribe);
		strn0cpy(eq->txtfile, emu->booktext, sizeof(eq->txtfile));

		FINISH_ENCODE();
	}

	ENCODE(OP_RespondAA)
	{
		SETUP_DIRECT_ENCODE(AATable_Struct, structs::AATable_Struct);

		eq->aa_spent = emu->aa_spent;
		eq->aa_assigned = emu->aa_spent;
		eq->aa_spent3 = 0;
		eq->unknown012 = 0;
		eq->unknown016 = 0;
		eq->unknown020 = 0;

		for (uint32 i = 0; i < MAX_PP_AA_ARRAY; ++i)
		{
			eq->aa_list[i].AA = emu->aa_list[i].AA;
			eq->aa_list[i].value = emu->aa_list[i].value;
			eq->aa_list[i].charges = emu->aa_list[i].charges;
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

			// Special Underfoot adjustment - field should really be 'AdditionalChars' or 'BonusChars'
			uint32 adjusted_total = eq->TotalChars - 8; // Yes, it rolls under for '< 8' - probably an int32 field
			eq->TotalChars = adjusted_total;

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

		// Special Underfoot adjustment - field should really be 'AdditionalChars' or 'BonusChars' in this client
		uint32 adjusted_total = eq->TotalChars - 8; // Yes, it rolls under for '< 8' - probably an int32 field
		eq->TotalChars = adjusted_total;

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
		SETUP_DIRECT_ENCODE(Merchant_Sell_Struct, structs::Merchant_Sell_Struct);

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
		SETUP_DIRECT_ENCODE(Merchant_Purchase_Struct, structs::Merchant_Purchase_Struct);

		OUT(npcid);
		eq->itemslot = ServerToUFSlot(emu->itemslot);
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

		eq->unknown070 = 135; // This needs to be set to something, else we get a 1HS animation instead of ranged.

		OUT(item_type);
		OUT(skill);

		strcpy(eq->model_name, emu->model_name);

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
					default: {
						eq->parameter = GUILD_RANK_NONE_TI;
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

	ENCODE(OP_SetGuildRank)
	{
		ENCODE_LENGTH_EXACT(GuildSetRank_Struct);
		SETUP_DIRECT_ENCODE(GuildSetRank_Struct, structs::GuildSetRank_Struct);

		eq->unknown00 = 0;
		eq->unknown04 = 0;

		switch (emu->rank) {
			case GUILD_SENIOR_MEMBER:
			case GUILD_MEMBER:
			case GUILD_JUNIOR_MEMBER:
			case GUILD_INITIATE:
			case GUILD_RECRUIT: {
				emu->rank = GUILD_MEMBER_TI;
				break;
			}
			case GUILD_OFFICER:
			case GUILD_SENIOR_OFFICER: {
				emu->rank = GUILD_OFFICER_TI;
				break;
			}
			case GUILD_LEADER: {
				emu->rank = GUILD_LEADER_TI;
				break;
			}
			default: {
				emu->rank = GUILD_RANK_NONE_TI;
				break;
			}
		}

		memcpy(eq->member_name, emu->member_name, sizeof(eq->member_name));
		OUT(banker);

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
		in->ReadString(name); // NPC names max out at 63 chars

		buf.WriteString(name);

		buf.WriteInt32(in->ReadUInt32()); // loc
		buf.WriteInt32(in->ReadUInt32());
		buf.WriteInt32(in->ReadUInt32());

		std::string old_message;
		std::string new_message;

		in->ReadString(old_message);

		ServerToUFSayLink(new_message, old_message);

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

	ENCODE(OP_TargetBuffs) { ENCODE_FORWARD(OP_BuffCreate); }

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
		ServerToUFSayLink(new_message, old_message);

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

		int PacketSize = 2;

		for (int i = 0; i < EntryCount; ++i, ++emu)
			PacketSize += (12 + strlen(emu->name));

		emu = (Track_Struct *)__emu_buffer;

		in->size = PacketSize;
		in->pBuffer = new unsigned char[in->size];

		char *Buffer = (char *)in->pBuffer;

		VARSTRUCT_ENCODE_TYPE(uint16, Buffer, EntryCount);

		for (int i = 0; i < EntryCount; ++i, ++emu)
		{
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->entityid);
			VARSTRUCT_ENCODE_TYPE(float, Buffer, emu->distance);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->level);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->is_npc);
			VARSTRUCT_ENCODE_STRING(Buffer, emu->name);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->is_merc);
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

				eq->action = structs::UFBazaarTraderBuyerActions::BeginTraderMode;
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

				eq->action = structs::UFBazaarTraderBuyerActions::EndTraderMode;
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

				eq->action = structs::UFBazaarTraderBuyerActions::ListTraderItems;
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

				eq->action = structs::UFBazaarTraderBuyerActions::BuyTraderItem;
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
				*p                      = nullptr;

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

				eq->action = structs::UFBazaarTraderBuyerActions::BuyTraderItem;
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

		eq->slot = ServerToUFSlot(emu->slot);
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
		for (unsigned int i = 0; i < count; ++i)
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

	ENCODE(OP_WhoAllResponse)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		char *InBuffer = (char *)in->pBuffer;

		WhoAllReturnStruct *wars = (WhoAllReturnStruct*)InBuffer;

		int Count = wars->playercount;

		auto outapp = new EQApplicationPacket(OP_WhoAllResponse, in->size + (Count * 4));

		char *OutBuffer = (char *)outapp->pBuffer;

		memcpy(OutBuffer, InBuffer, sizeof(WhoAllReturnStruct));

		OutBuffer += sizeof(WhoAllReturnStruct);
		InBuffer += sizeof(WhoAllReturnStruct);

		for (int i = 0; i < Count; ++i)
		{
			uint32 x;

			x = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);

			VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, x);

			InBuffer += 4;
			VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);
			VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0xffffffff);

			char Name[64];


			VARSTRUCT_DECODE_STRING(Name, InBuffer);	// Char Name
			VARSTRUCT_ENCODE_STRING(OutBuffer, Name);

			x = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);
			VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, x);

			VARSTRUCT_DECODE_STRING(Name, InBuffer);	// Guild Name
			VARSTRUCT_ENCODE_STRING(OutBuffer, Name);

			for (int j = 0; j < 7; ++j)
			{
				x = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);
				VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, x);
			}

			VARSTRUCT_DECODE_STRING(Name, InBuffer);		// Account
			VARSTRUCT_ENCODE_STRING(OutBuffer, Name);

			x = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);
			VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, x);
		}

		//Log.Hex(Logs::Netcode, outapp->pBuffer, outapp->size);
		dest->FastQueuePacket(&outapp);
		delete in;
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

		//Log.LogDebugType(Logs::General, Logs::Netcode, "[STRUCTS] Spawn name is [%s]", emu->name);

		emu = (Spawn_Struct *)__emu_buffer;

		//Log.LogDebugType(Logs::General, Logs::Netcode, "[STRUCTS] Spawn packet size is %i, entries = %i", in->size, entrycount);

		char *Buffer = (char *)in->pBuffer;

		int r;
		int k;
		for (r = 0; r < entrycount; r++, emu++) {

			int PacketSize = sizeof(structs::Spawn_Struct);

			PacketSize += strlen(emu->name);
			PacketSize += strlen(emu->lastName);

			if (strlen(emu->title))
				PacketSize += strlen(emu->title) + 1;

			if (strlen(emu->suffix))
				PacketSize += strlen(emu->suffix) + 1;

			if (emu->DestructibleObject || emu->class_ == Class::LDoNTreasure)
			{
				if (emu->DestructibleObject)
					PacketSize = PacketSize - 4;	// No bodytype

				PacketSize += 53;	// Fixed portion
				PacketSize += strlen(emu->DestructibleModel) + 1;
				PacketSize += strlen(emu->DestructibleName2) + 1;
				PacketSize += strlen(emu->DestructibleString) + 1;
			}

			bool ShowName = emu->show_name;
			if (emu->bodytype >= 66)
			{
				emu->race = 127;
				emu->bodytype = 11;
				emu->gender = 0;
				ShowName = 0;
			}

			float SpawnSize = emu->size;
			if (!((emu->NPC == 0) || (emu->race <= Race::Gnome) || (emu->race == Race::Iksar) ||
					(emu->race == Race::VahShir) || (emu->race == Race::Froglok2) || (emu->race == Race::Drakkin))
				)
			{
				PacketSize -= (sizeof(structs::Texture_Struct) * EQ::textures::materialCount);

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

			auto outapp = new EQApplicationPacket(OP_ZoneEntry, PacketSize);
			Buffer = (char *)outapp->pBuffer;

			VARSTRUCT_ENCODE_STRING(Buffer, emu->name);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->spawnId);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->level);

			if (emu->DestructibleObject)
			{
				VARSTRUCT_ENCODE_TYPE(float, Buffer, 10);	// was int and 0x41200000
			}
			else
			{
				VARSTRUCT_ENCODE_TYPE(float, Buffer, SpawnSize - 0.7);	// Eye Height?
			}

			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->NPC);

			structs::Spawn_Struct_Bitfields *Bitfields = (structs::Spawn_Struct_Bitfields*)Buffer;

			Bitfields->afk = 0;
			Bitfields->linkdead = 0;
			Bitfields->gender = emu->gender;

			Bitfields->invis = emu->invis;
			Bitfields->sneak = 0;
			Bitfields->lfg = emu->lfg;
			Bitfields->gm = emu->gm;
			Bitfields->anon = emu->anon;
			Bitfields->showhelm = emu->showhelm;
			Bitfields->targetable = 1;
			Bitfields->targetable_with_hotkey = emu->targetable_with_hotkey ? 1 : 0;
			Bitfields->statue = 0;
			Bitfields->trader = emu->trader ? 1 : 0;
			Bitfields->buyer = 0;

			Bitfields->showname = ShowName;

			if (emu->DestructibleObject)
			{
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0x1d600000);
				Buffer = Buffer - 4;
			}

			Bitfields->ispet = emu->is_pet;

			Buffer += sizeof(structs::Spawn_Struct_Bitfields);

			uint8 OtherData = 0;

			if (emu->class_ == Class::LDoNTreasure) //Ldon chest
			{
				OtherData = OtherData | 0x01;
			}

			if (strlen(emu->title)) {
				OtherData = OtherData | 0x04;
			}
			if (strlen(emu->suffix)) {
				OtherData = OtherData | 0x08;
			}
			if (emu->DestructibleObject) {
				OtherData = OtherData | 0xd1;	// Live has 0xe1 for OtherData
			}
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, OtherData);

			if (emu->DestructibleObject)
			{
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0x00000000);
			}
			else
			{
				VARSTRUCT_ENCODE_TYPE(float, Buffer, -1);	// unknown3
			}
			VARSTRUCT_ENCODE_TYPE(float, Buffer, 0);	// unknown4

			if (emu->DestructibleObject || emu->class_ == Class::LDoNTreasure)
			{
				VARSTRUCT_ENCODE_STRING(Buffer, emu->DestructibleModel);
				VARSTRUCT_ENCODE_STRING(Buffer, emu->DestructibleName2);
				VARSTRUCT_ENCODE_STRING(Buffer, emu->DestructibleString);

				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->DestructibleAppearance);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->DestructibleUnk1);

				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->DestructibleID1);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->DestructibleID2);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->DestructibleID3);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->DestructibleID4);

				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->DestructibleUnk2);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->DestructibleUnk3);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->DestructibleUnk4);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->DestructibleUnk5);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->DestructibleUnk6);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->DestructibleUnk7);
				VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->DestructibleUnk8);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->DestructibleUnk9);
			}

			VARSTRUCT_ENCODE_TYPE(float, Buffer, emu->size);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->face);
			VARSTRUCT_ENCODE_TYPE(float, Buffer, emu->walkspeed);
			VARSTRUCT_ENCODE_TYPE(float, Buffer, emu->runspeed);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->race);
			/*
			if(emu->bodytype >=66)
			{
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// showname
			}
			else
			{
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 1);	// showname
			}*/


			if (!emu->DestructibleObject)
			{
				// Setting this next field to zero will cause a crash. Looking at ShowEQ, if it is zero, the bodytype field is not
				// present. Will sort that out later.
				VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 1);	// This is a properties count field
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->bodytype);
			}
			else
			{
				VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);
			}

			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->curHp);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->haircolor);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->beardcolor);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->eyecolor1);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->eyecolor2);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->hairstyle);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->beard);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->drakkin_heritage);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->drakkin_tattoo);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->drakkin_details);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// ShowEQ calls this 'Holding'
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->deity);
			if (emu->NPC)
			{
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0xFFFFFFFF);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0x00000000);
			}
			else
			{
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->guildID);
				//Translate older ranks to new values* /
				switch (emu->guildrank) {
					case GUILD_SENIOR_MEMBER:
					case GUILD_MEMBER:
					case GUILD_JUNIOR_MEMBER:
					case GUILD_INITIATE:
					case GUILD_RECRUIT: {
						emu->guildrank = GUILD_MEMBER_TI;
						break;
					}
					case GUILD_OFFICER:
					case GUILD_SENIOR_OFFICER: {
						emu->guildrank = GUILD_OFFICER_TI;
						break;
					}
					case GUILD_LEADER: {
						emu->guildrank = GUILD_LEADER_TI;
						break;
					}
					default: {
						emu->guildrank = GUILD_RANK_NONE_TI;
						break;
					}
				}
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->guildrank);
			}
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->class_);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// pvp
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->StandState);	// standstate
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->light);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->flymode);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->equip_chest2); // unknown8
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0); // unknown9
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0); // unknown10
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->helm); // unknown11
			VARSTRUCT_ENCODE_STRING(Buffer, emu->lastName);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);	// aatitle
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0); // unknown12
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->petOwnerId);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0); // unknown13
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->PlayerState);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0); // unknown15
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0); // unknown16
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0); // unknown17
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0xffffffff); // unknown18
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0xffffffff); // unknown19

			structs::Spawn_Struct_Position *Position = (structs::Spawn_Struct_Position*)Buffer;

			Position->deltaX = emu->deltaX;
			Position->deltaHeading = emu->deltaHeading;
			Position->deltaY = emu->deltaY;
			Position->y = emu->y;
			Position->animation = emu->animation;
			Position->heading = emu->heading;
			Position->x = emu->x;
			Position->z = emu->z;
			Position->deltaZ = emu->deltaZ;

			Buffer += sizeof(structs::Spawn_Struct_Position);

			if ((emu->NPC == 0) || (emu->race <= Race::Gnome) || (emu->race == Race::Iksar) ||
					(emu->race == Race::VahShir) || (emu->race == Race::Froglok2) || (emu->race == Race::Drakkin)
				)
			{
				for (k = EQ::textures::textureBegin; k < EQ::textures::materialCount; ++k)
				{
					{
						VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->equipment_tint.Slot[k].Color);
					}
				}
			}
			else
			{
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);

				if (emu->equipment.Primary.Material > 99999) {
					VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 63);
				} else {
					VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->equipment.Primary.Material);
				}

				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);

				if (emu->equipment.Secondary.Material > 99999) {
					VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 63);
				} else {
					VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->equipment.Secondary.Material);
				}

				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
			}

			if ((emu->NPC == 0) || (emu->race <= Race::Gnome) || (emu->race == Race::Iksar) ||
					(emu->race == Race::VahShir) || (emu->race == Race::Froglok2) || (emu->race == Race::Drakkin)
				)
			{
				structs::Texture_Struct *Equipment = (structs::Texture_Struct *)Buffer;

				for (k = EQ::textures::textureBegin; k < EQ::textures::materialCount; k++) {
					if (emu->equipment.Slot[k].Material > 99999) {
						Equipment[k].Material = 63;
					} else {
						Equipment[k].Material = emu->equipment.Slot[k].Material;
					}
					Equipment[k].Unknown1 = emu->equipment.Slot[k].Unknown1;
					Equipment[k].EliteMaterial = emu->equipment.Slot[k].EliteModel;
				}

				Buffer += (sizeof(structs::Texture_Struct) * EQ::textures::materialCount);
			}
			if (strlen(emu->title))
			{
				VARSTRUCT_ENCODE_STRING(Buffer, emu->title);
			}

			if (strlen(emu->suffix))
			{
				VARSTRUCT_ENCODE_STRING(Buffer, emu->suffix);
			}
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0); // Unknown;
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0); // Unknown;
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->IsMercenary); //IsMercenary
			Buffer += 28; // Unknown;

			dest->FastQueuePacket(&outapp, ack_req);
		}

		delete in;
	}

// DECODE methods
	DECODE(OP_AdventureMerchantSell)
	{
		DECODE_LENGTH_EXACT(structs::Adventure_Sell_Struct);
		SETUP_DIRECT_DECODE(Adventure_Sell_Struct, structs::Adventure_Sell_Struct);

		IN(npcid);
		emu->slot = UFToServerSlot(eq->slot);
		IN(charges);
		IN(sell_price);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_AltCurrencySell)
	{
		DECODE_LENGTH_EXACT(structs::AltCurrencySellItem_Struct);
		SETUP_DIRECT_DECODE(AltCurrencySellItem_Struct, structs::AltCurrencySellItem_Struct);

		IN(merchant_entity_id);
		emu->slot_id = UFToServerSlot(eq->slot_id);
		IN(charges);
		IN(cost);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_AltCurrencySellSelection)
	{
		DECODE_LENGTH_EXACT(structs::AltCurrencySelectItem_Struct);
		SETUP_DIRECT_DECODE(AltCurrencySelectItem_Struct, structs::AltCurrencySelectItem_Struct);

		IN(merchant_entity_id);
		emu->slot_id = UFToServerSlot(eq->slot_id);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ApplyPoison)
	{
		DECODE_LENGTH_EXACT(structs::ApplyPoison_Struct);
		SETUP_DIRECT_DECODE(ApplyPoison_Struct, structs::ApplyPoison_Struct);

		emu->inventorySlot = UFToServerSlot(eq->inventorySlot);
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

		emu->container_slot = UFToServerSlot(eq->container_slot);
		emu->augment_slot = eq->augment_slot;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_BazaarSearch)
	{
		uint32 action = *(uint32 *) __packet->pBuffer;

		switch (action) {
			case structs::UFBazaarTraderBuyerActions::BazaarSearch: {
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
			case structs::UFBazaarTraderBuyerActions::BazaarInspect: {
				SETUP_DIRECT_DECODE(BazaarInspect_Struct, structs::BazaarInspect_Struct);

				IN(action);
				memcpy(emu->player_name, eq->player_name, sizeof(emu->player_name));
				IN(serial_number);

				FINISH_DIRECT_DECODE();
				break;
			}
			case structs::UFBazaarTraderBuyerActions::WelcomeMessage: {
				break;
			}
			default: {
				LogTrading("(UF) Unhandled action <red>[{}]", action);
			}
		}
	}

	DECODE(OP_BookButton)
	{
		DECODE_LENGTH_EXACT(structs::BookButton_Struct);
		SETUP_DIRECT_DECODE(BookButton_Struct, structs::BookButton_Struct);

		emu->invslot = static_cast<int16_t>(UFToServerSlot(eq->invslot));
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
		IN(buff.unknown003);
		IN(buff.spellid);
		IN(buff.duration);
		emu->slotid = UFToServerBuffSlot(eq->slotid);
		IN(slotid);
		IN(bufffade);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_BuffRemoveRequest)
	{
		// This is to cater for the fact that short buff box buffs start at 30 as opposed to 25 in prior clients.
		//
		DECODE_LENGTH_EXACT(structs::BuffRemoveRequest_Struct);
		SETUP_DIRECT_DECODE(BuffRemoveRequest_Struct, structs::BuffRemoveRequest_Struct);

		emu->SlotID = UFToServerBuffSlot(eq->SlotID);

		IN(EntityID);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_CastSpell)
	{
		DECODE_LENGTH_EXACT(structs::CastSpell_Struct);
		SETUP_DIRECT_DECODE(CastSpell_Struct, structs::CastSpell_Struct);

		emu->slot = static_cast<uint32>(UFToServerCastingSlot(static_cast<spells::CastingSlot>(eq->slot)));

		IN(spell_id);
		emu->inventoryslot = UFToServerSlot(eq->inventoryslot);
		IN(target_id);
		IN(cs_unknown1);
		IN(cs_unknown2);
		IN(y_pos);
		IN(x_pos);
		IN(z_pos);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ChannelMessage)
	{
		unsigned char *__eq_buffer = __packet->pBuffer;

		char *InBuffer = (char *)__eq_buffer;

		char Sender[64];
		char Target[64];

		VARSTRUCT_DECODE_STRING(Sender, InBuffer);
		VARSTRUCT_DECODE_STRING(Target, InBuffer);

		InBuffer += 4;

		uint32 Language = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);
		uint32 Channel = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);

		InBuffer += 5;

		uint32 Skill = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);

		std::string old_message = InBuffer;
		std::string new_message;
		UFToServerSayLink(new_message, old_message);

		//__packet->size = sizeof(ChannelMessage_Struct)+strlen(InBuffer) + 1;
		__packet->size = sizeof(ChannelMessage_Struct) + new_message.length() + 1;

		__packet->pBuffer = new unsigned char[__packet->size];
		ChannelMessage_Struct *emu = (ChannelMessage_Struct *)__packet->pBuffer;

		strn0cpy(emu->targetname, Target, sizeof(emu->targetname));
		strn0cpy(emu->sender, Target, sizeof(emu->sender));
		emu->language = Language;
		emu->chan_num = Channel;
		emu->skill_in_language = Skill;
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

		emu->slot = UFToServerSlot(eq->slot);
		IN(auto_consumed);
		IN(type);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_Damage)
	{
		DECODE_LENGTH_EXACT(structs::CombatDamage_Struct);
		SETUP_DIRECT_DECODE(CombatDamage_Struct, structs::CombatDamage_Struct);

		IN(target);
		IN(source);
		IN(type);
		IN(spellid);
		IN(damage);
		IN(hit_heading);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_DeleteItem)
	{
		DECODE_LENGTH_EXACT(structs::DeleteItem_Struct);
		SETUP_DIRECT_DECODE(DeleteItem_Struct, structs::DeleteItem_Struct);

		emu->from_slot = UFToServerSlot(eq->from_slot);
		emu->to_slot = UFToServerSlot(eq->to_slot);
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

		IN(unknown000);
		IN(unknown004);
		IN(unknown008);
		IN(unknown_id1);
		IN(dz_zone_id);
		IN(dz_instance_id);
		IN(dz_type);
		IN(unknown_id2);
		IN(unknown028);
		IN(unknown032);
		IN(unknown036);
		IN(unknown040);
		IN(unknown044);
		IN(unknown048);

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
		UFToServerSayLink(new_message, old_message);

		__packet->size = sizeof(Emote_Struct);
		__packet->pBuffer = new unsigned char[__packet->size];

		char *InBuffer = (char *)__packet->pBuffer;

		memcpy(InBuffer, __eq_buffer, 4);
		InBuffer += 4;
		strcpy(InBuffer, new_message.substr(0, 1023).c_str());
		InBuffer[1023] = '\0';

		delete[] __eq_buffer;
	}

	DECODE(OP_EnvDamage)
	{
		DECODE_LENGTH_EXACT(structs::EnvDamage2_Struct);
		SETUP_DIRECT_DECODE(EnvDamage2_Struct, structs::EnvDamage2_Struct);

		IN(id);
		IN(damage);
		IN(dmgtype);
		emu->constant = 0xFFFF;

		FINISH_DIRECT_DECODE();
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

	DECODE(OP_GroupCancelInvite)
	{
		DECODE_LENGTH_EXACT(structs::GroupCancel_Struct);
		SETUP_DIRECT_DECODE(GroupCancel_Struct, structs::GroupCancel_Struct);

		memcpy(emu->name1, eq->name1, sizeof(emu->name1));
		memcpy(emu->name2, eq->name2, sizeof(emu->name2));
		IN(toggle);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GroupDisband)
	{
		//EQApplicationPacket *in = __packet;
		//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Received incoming OP_Disband");
		//Log.Hex(Logs::Netcode, in->pBuffer, in->size);
		DECODE_LENGTH_EXACT(structs::GroupGeneric_Struct);
		SETUP_DIRECT_DECODE(GroupGeneric_Struct, structs::GroupGeneric_Struct);

		memcpy(emu->name1, eq->name1, sizeof(emu->name1));
		memcpy(emu->name2, eq->name2, sizeof(emu->name2));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GroupFollow)
	{
		//EQApplicationPacket *in = __packet;
		//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Received incoming OP_GroupFollow");
		//Log.Hex(Logs::Netcode, in->pBuffer, in->size);
		DECODE_LENGTH_EXACT(structs::GroupFollow_Struct);
		SETUP_DIRECT_DECODE(GroupGeneric_Struct, structs::GroupFollow_Struct);

		memcpy(emu->name1, eq->name1, sizeof(emu->name1));
		memcpy(emu->name2, eq->name2, sizeof(emu->name2));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GroupFollow2)
	{
		//EQApplicationPacket *in = __packet;
		//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Received incoming OP_GroupFollow2");
		//Log.Hex(Logs::Netcode, in->pBuffer, in->size);
		DECODE_LENGTH_EXACT(structs::GroupFollow_Struct);
		SETUP_DIRECT_DECODE(GroupGeneric_Struct, structs::GroupFollow_Struct);

		memcpy(emu->name1, eq->name1, sizeof(emu->name1));
		memcpy(emu->name2, eq->name2, sizeof(emu->name2));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GroupInvite)
	{
		//EQApplicationPacket *in = __packet;
		//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Received incoming OP_GroupInvite");
		//Log.Hex(Logs::Netcode, in->pBuffer, in->size);
		DECODE_LENGTH_EXACT(structs::GroupInvite_Struct);
		SETUP_DIRECT_DECODE(GroupGeneric_Struct, structs::GroupInvite_Struct);

		memcpy(emu->name1, eq->invitee_name, sizeof(emu->name1));
		memcpy(emu->name2, eq->inviter_name, sizeof(emu->name2));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GroupInvite2)
	{
		//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Received incoming OP_GroupInvite2. Forwarding");
		DECODE_FORWARD(OP_GroupInvite);
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

		emu->slot = UFToServerSlot(eq->slot);

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

		emu->slot = UFToServerSlot(eq->slot);
		IN(target);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_LoadSpellSet)
	{
		DECODE_LENGTH_EXACT(structs::LoadSpellSet_Struct);
		SETUP_DIRECT_DECODE(LoadSpellSet_Struct, structs::LoadSpellSet_Struct);

		for (unsigned int i = 0; i < EQ::spells::SPELL_GEM_COUNT; ++i)
			if (eq->spell[i] == 0)
				emu->spell[i] = 0xFFFFFFFF;
			else
				emu->spell[i] = eq->spell[i];

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_LootItem)
	{
		DECODE_LENGTH_EXACT(structs::LootingItem_Struct);
		SETUP_DIRECT_DECODE(LootingItem_Struct, structs::LootingItem_Struct);

		Log(Logs::Detail, Logs::Netcode, "UF::DECODE(OP_LootItem)");

		IN(lootee);
		IN(looter);
		emu->slot_id = UFToServerCorpseSlot(eq->slot_id);
		IN(auto_loot);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_MoveItem)
	{
		DECODE_LENGTH_EXACT(structs::MoveItem_Struct);
		SETUP_DIRECT_DECODE(MoveItem_Struct, structs::MoveItem_Struct);

		Log(Logs::Detail, Logs::Netcode, "UF::DECODE(OP_MoveItem)");

		emu->from_slot = UFToServerSlot(eq->from_slot);
		emu->to_slot = UFToServerSlot(eq->to_slot);
		IN(number_in_stack);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_PetCommands)
	{
		DECODE_LENGTH_EXACT(structs::PetCommand_Struct);
		SETUP_DIRECT_DECODE(PetCommand_Struct, structs::PetCommand_Struct);

		IN(command);
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
		emu->invslot = static_cast<int16_t>(UFToServerSlot(eq->invslot));
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

	DECODE(OP_ShopPlayerBuy)
	{
		DECODE_LENGTH_EXACT(structs::Merchant_Sell_Struct);
		SETUP_DIRECT_DECODE(Merchant_Sell_Struct, structs::Merchant_Sell_Struct);

		IN(npcid);
		IN(playerid);
		IN(itemslot);
		IN(quantity);
		IN(price);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ShopPlayerSell)
	{
		DECODE_LENGTH_EXACT(structs::Merchant_Purchase_Struct);
		SETUP_DIRECT_DECODE(Merchant_Purchase_Struct, structs::Merchant_Purchase_Struct);

		IN(npcid);
		emu->itemslot = UFToServerSlot(eq->itemslot);
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
			case structs::UFBazaarTraderBuyerActions::BeginTraderMode: {
				DECODE_LENGTH_EXACT(structs::BeginTrader_Struct);
				SETUP_DIRECT_DECODE(ClickTrader_Struct, structs::BeginTrader_Struct);
				LogTrading(
					"Decode OP_Trader BeginTraderMode action <red>[{}]",
					action
				);

				emu->action      = TraderOn;
				emu->unknown_004 = 0;
				std::copy_n(eq->serial_number, UF::invtype::BAZAAR_SIZE, emu->serial_number);
				std::copy_n(eq->cost, UF::invtype::BAZAAR_SIZE, emu->item_cost);

				FINISH_DIRECT_DECODE();
				break;
			}
			case structs::UFBazaarTraderBuyerActions::EndTraderMode: {
				DECODE_LENGTH_EXACT(structs::Trader_ShowItems_Struct);
				SETUP_DIRECT_DECODE(Trader_ShowItems_Struct, structs::Trader_ShowItems_Struct);
				LogTrading(
					"Decode OP_Trader(UF) EndTraderMode action <red>[{}]",
					action
				);

				emu->action = TraderOff;
				IN(entity_id);

				FINISH_DIRECT_DECODE();
				break;
			}
			case structs::UFBazaarTraderBuyerActions::PriceUpdate:
			case structs::UFBazaarTraderBuyerActions::ItemMove:
			case structs::UFBazaarTraderBuyerActions::EndTransaction:
			case structs::UFBazaarTraderBuyerActions::ListTraderItems: {
				LogTrading(
					"Decode OP_Trader(UF) Price/ItemMove/EndTransaction/ListTraderItems action <red>[{}]",
					action
				);
				break;
			}
			case structs::UFBazaarTraderBuyerActions::ReconcileItems: {
				break;
			}
			default: {
				LogError("Unhandled(UF) action <red>[{}] received.", action);
			}
		}
	}

	DECODE(OP_TraderBuy)
	{
		DECODE_LENGTH_EXACT(structs::TraderBuy_Struct);
		SETUP_DIRECT_DECODE(TraderBuy_Struct, structs::TraderBuy_Struct);
		LogTrading(
			"Decode OP_TraderBuy(UF) item_id <green>[{}] price <green>[{}] quantity <green>[{}] trader_id <green>[{}]",
			eq->item_id,
			eq->price,
			eq->quantity,
			eq->trader_id
		);

		emu->action = BuyTraderItem;
		IN(price);
		IN(trader_id);
		IN(item_id);
		IN(quantity);
		IN(already_sold);
		strn0cpy(emu->item_name, eq->item_name, sizeof(eq->item_name));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_TraderShop)
	{
		DECODE_LENGTH_EXACT(structs::TraderClick_Struct);
		SETUP_DIRECT_DECODE(TraderClick_Struct, structs::TraderClick_Struct);
		LogTrading(
			"(UF) action <green>[{}] trader_id <green>[{}] approval <green>[{}]",
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

		emu->container_slot = UFToServerSlot(eq->container_slot);
		IN(guildtribute_slot);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_TributeItem)
	{
		DECODE_LENGTH_EXACT(structs::TributeItem_Struct);
		SETUP_DIRECT_DECODE(TributeItem_Struct, structs::TributeItem_Struct);

		emu->slot = UFToServerSlot(eq->slot);
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

		UF::structs::ItemSerializationHeader hdr;

		hdr.stacksize = (inst->IsStackable() ? ((inst->GetCharges() > 1000) ? 0xFFFFFFFF : inst->GetCharges()) : 1);
		hdr.unknown004 = 0;

		int32 slot_id = ServerToUFSlot(slot_id_in);

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
		hdr.isEvolving = item->EvolvingItem;

		ob.write((const char*)&hdr, sizeof(UF::structs::ItemSerializationHeader));

		if (item->EvolvingItem > 0) {
			UF::structs::EvolvingItem evotop;

			evotop.unknown001 = 0;
			evotop.unknown002 = 0;
			evotop.unknown003 = 0;
			evotop.unknown004 = 0;
			evotop.evoLevel = item->EvolvingLevel;
			evotop.progress = 0;
			evotop.Activated = 1;
			evotop.evomaxlevel = item->EvolvingMax;

			ob.write((const char*)&evotop, sizeof(UF::structs::EvolvingItem));
		}

		uint16     ornament_icon = 0;
		const auto augment       = inst->GetOrnamentationAugment();

		if (augment) {
			const auto augment_item = augment->GetItem();
			ornament_icon = augment_item->Icon;

			ob.write(augment_item->IDFile, strlen(augment_item->IDFile));
		}
		else if (inst->GetOrnamentationIDFile() && inst->GetOrnamentationIcon()) {
			ornament_icon = inst->GetOrnamentationIcon();
			char tmp[30]; memset(tmp, 0x0, 30); sprintf(tmp, "IT%d", inst->GetOrnamentationIDFile());

			ob.write(tmp, strlen(tmp));
		}
		ob.write("\0", 1);

		UF::structs::ItemSerializationHeaderFinish hdrf;

		hdrf.ornamentIcon = ornament_icon;
		hdrf.unknown060 = 0; //This is Always 0.. or it breaks shit..
		hdrf.unknown061 = 0; //possibly ornament / special ornament
		hdrf.isCopied = 0; //Flag for item to be 'Copied'
		hdrf.ItemClass = item->ItemClass;

		ob.write((const char*)&hdrf, sizeof(UF::structs::ItemSerializationHeaderFinish));

		if (strlen(item->Name) > 0)
			ob.write(item->Name, strlen(item->Name));
		ob.write("\0", 1);

		if (strlen(item->Lore) > 0)
			ob.write(item->Lore, strlen(item->Lore));
		ob.write("\0", 1);

		if (strlen(item->IDFile) > 0)
			ob.write(item->IDFile, strlen(item->IDFile));
		ob.write("\0", 1);

		UF::structs::ItemBodyStruct ibs;
		memset(&ibs, 0, sizeof(UF::structs::ItemBodyStruct));

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

		ob.write((const char*)&ibs, sizeof(UF::structs::ItemBodyStruct));

		//charm text
		if (strlen(item->CharmFile) > 0)
			ob.write((const char*)item->CharmFile, strlen(item->CharmFile));
		ob.write("\0", 1);

		UF::structs::ItemSecondaryBodyStruct isbs;
		memset(&isbs, 0, sizeof(UF::structs::ItemSecondaryBodyStruct));

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

		ob.write((const char*)&isbs, sizeof(UF::structs::ItemSecondaryBodyStruct));

		if (strlen(item->Filename) > 0)
			ob.write((const char*)item->Filename, strlen(item->Filename));
		ob.write("\0", 1);

		UF::structs::ItemTertiaryBodyStruct itbs;
		memset(&itbs, 0, sizeof(UF::structs::ItemTertiaryBodyStruct));

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

		ob.write((const char*)&itbs, sizeof(UF::structs::ItemTertiaryBodyStruct));

		// Effect Structures Broken down to allow variable length strings for effect names
		int32 effect_unknown = 0;

		UF::structs::ClickEffectStruct ices;
		memset(&ices, 0, sizeof(UF::structs::ClickEffectStruct));

		ices.effect = item->Click.Effect;
		ices.level2 = item->Click.Level2;
		ices.type = item->Click.Type;
		ices.level = item->Click.Level;
		ices.max_charges = item->MaxCharges;
		ices.cast_time = item->CastTime;
		ices.recast = item->RecastDelay;
		ices.recast_type = item->RecastType;

		ob.write((const char*)&ices, sizeof(UF::structs::ClickEffectStruct));

		if (strlen(item->ClickName) > 0)
			ob.write((const char*)item->ClickName, strlen(item->ClickName));
		ob.write("\0", 1);

		ob.write((const char*)&effect_unknown, sizeof(int32));	// clickunk7

		UF::structs::ProcEffectStruct ipes;
		memset(&ipes, 0, sizeof(UF::structs::ProcEffectStruct));

		ipes.effect = item->Proc.Effect;
		ipes.level2 = item->Proc.Level2;
		ipes.type = item->Proc.Type;
		ipes.level = item->Proc.Level;
		ipes.procrate = item->ProcRate;

		ob.write((const char*)&ipes, sizeof(UF::structs::ProcEffectStruct));

		if (strlen(item->ProcName) > 0)
			ob.write((const char*)item->ProcName, strlen(item->ProcName));
		ob.write("\0", 1);

		ob.write((const char*)&effect_unknown, sizeof(int32));	// unknown5

		UF::structs::WornEffectStruct iwes;
		memset(&iwes, 0, sizeof(UF::structs::WornEffectStruct));

		iwes.effect = item->Worn.Effect;
		iwes.level2 = item->Worn.Level2;
		iwes.type = item->Worn.Type;
		iwes.level = item->Worn.Level;

		ob.write((const char*)&iwes, sizeof(UF::structs::WornEffectStruct));

		if (strlen(item->WornName) > 0)
			ob.write((const char*)item->WornName, strlen(item->WornName));
		ob.write("\0", 1);

		ob.write((const char*)&effect_unknown, sizeof(int32));	// unknown6

		UF::structs::WornEffectStruct ifes;
		memset(&ifes, 0, sizeof(UF::structs::WornEffectStruct));

		ifes.effect = item->Focus.Effect;
		ifes.level2 = item->Focus.Level2;
		ifes.type = item->Focus.Type;
		ifes.level = item->Focus.Level;

		ob.write((const char*)&ifes, sizeof(UF::structs::WornEffectStruct));

		if (strlen(item->FocusName) > 0)
			ob.write((const char*)item->FocusName, strlen(item->FocusName));
		ob.write("\0", 1);

		ob.write((const char*)&effect_unknown, sizeof(int32));	// unknown6

		UF::structs::WornEffectStruct ises;
		memset(&ises, 0, sizeof(UF::structs::WornEffectStruct));

		ises.effect = item->Scroll.Effect;
		ises.level2 = item->Scroll.Level2;
		ises.type = item->Scroll.Type;
		ises.level = item->Scroll.Level;

		ob.write((const char*)&ises, sizeof(UF::structs::WornEffectStruct));

		if (strlen(item->ScrollName) > 0)
			ob.write((const char*)item->ScrollName, strlen(item->ScrollName));
		ob.write("\0", 1);

		ob.write((const char*)&effect_unknown, sizeof(int32));	// unknown6

		// Bard Effect?
		UF::structs::WornEffectStruct ibes;
		memset(&ibes, 0, sizeof(UF::structs::WornEffectStruct));

		ibes.effect = item->Bard.Effect;
		ibes.level2 = item->Bard.Level2;
		ibes.type = item->Bard.Type;
		ibes.level = item->Bard.Level;
		//ibes.unknown6 = 0xffffffff;

		ob.write((const char*)&ibes, sizeof(UF::structs::WornEffectStruct));

		/*
		if(strlen(item->BardName) > 0)
		{
		ob.write((const char*)item->BardName, strlen(item->BardName));
		ob.write((const char*)&null_term, sizeof(uint8));
		}
		else */
		ob.write("\0", 1);

		ob.write((const char*)&effect_unknown, sizeof(int32));	// unknown6
		// End of Effects

		UF::structs::ItemQuaternaryBodyStruct iqbs;
		memset(&iqbs, 0, sizeof(UF::structs::ItemQuaternaryBodyStruct));

		iqbs.scriptfileid = item->ScriptFileID;
		iqbs.quest_item = item->QuestItemFlag;
		iqbs.unknown15 = 0;

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
		iqbs.Clairvoyance = item->Clairvoyance;
		iqbs.SubType = item->SubType;

		ob.write((const char*)&iqbs, sizeof(UF::structs::ItemQuaternaryBodyStruct));

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

	static inline uint32 ServerToUFSlot(uint32 serverSlot)
	{
		uint32 UFSlot = invslot::SLOT_INVALID;

		if (serverSlot <= EQ::invslot::slotGeneral8) {
			UFSlot = serverSlot;
		}

		else if (serverSlot <= EQ::invslot::CORPSE_END && serverSlot >= EQ::invslot::slotCursor) {
			UFSlot = serverSlot - 2;
		}

		else if (serverSlot <= EQ::invbag::GENERAL_BAGS_END && serverSlot >= EQ::invbag::GENERAL_BAGS_BEGIN) {
			UFSlot = serverSlot - (EQ::invbag::GENERAL_BAGS_BEGIN - invbag::GENERAL_BAGS_BEGIN)/*3748*/ - ((EQ::invbag::SLOT_COUNT - invbag::SLOT_COUNT) * ((serverSlot - EQ::invbag::GENERAL_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT)); // + 11;
		}

		else if (serverSlot <= EQ::invbag::CURSOR_BAG_END && serverSlot >= EQ::invbag::CURSOR_BAG_BEGIN) {
			UFSlot = serverSlot - (EQ::invbag::CURSOR_BAG_BEGIN - invbag::CURSOR_BAG_BEGIN)/*5668*/; // - 9;
		}

		else if (serverSlot <= EQ::invslot::TRIBUTE_END && serverSlot >= EQ::invslot::TRIBUTE_BEGIN) {
			UFSlot = serverSlot;
		}

		else if (serverSlot <= EQ::invslot::GUILD_TRIBUTE_END && serverSlot >= EQ::invslot::GUILD_TRIBUTE_BEGIN) {
			UFSlot = serverSlot;
		}

		else if (serverSlot == EQ::invslot::SLOT_TRADESKILL_EXPERIMENT_COMBINE) {
			UFSlot = serverSlot;
		}

		else if (serverSlot <= EQ::invslot::BANK_END && serverSlot >= EQ::invslot::BANK_BEGIN) {
			UFSlot = serverSlot;
		}

		else if (serverSlot <= EQ::invbag::BANK_BAGS_END && serverSlot >= EQ::invbag::BANK_BAGS_BEGIN) {
			UFSlot = serverSlot - (EQ::invbag::BANK_BAGS_BEGIN - invbag::BANK_BAGS_BEGIN) - ((EQ::invbag::SLOT_COUNT - invbag::SLOT_COUNT) * ((serverSlot - EQ::invbag::BANK_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT)); // + 1;
		}

		else if (serverSlot <= EQ::invslot::SHARED_BANK_END && serverSlot >= EQ::invslot::SHARED_BANK_BEGIN) {
			UFSlot = serverSlot;
		}

		else if (serverSlot <= EQ::invbag::SHARED_BANK_BAGS_END && serverSlot >= EQ::invbag::SHARED_BANK_BAGS_BEGIN) {
			UFSlot = serverSlot - (EQ::invbag::SHARED_BANK_BAGS_BEGIN - invbag::SHARED_BANK_BAGS_BEGIN) - ((EQ::invbag::SLOT_COUNT - invbag::SLOT_COUNT) * ((serverSlot - EQ::invbag::SHARED_BANK_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT)); // + 1;
		}

		else if (serverSlot <= EQ::invslot::TRADE_END && serverSlot >= EQ::invslot::TRADE_BEGIN) {
			UFSlot = serverSlot;
		}

		else if (serverSlot <= EQ::invbag::TRADE_BAGS_END && serverSlot >= EQ::invbag::TRADE_BAGS_BEGIN) {
			UFSlot = serverSlot - (EQ::invbag::TRADE_BAGS_BEGIN - invbag::TRADE_BAGS_BEGIN) - ((EQ::invbag::SLOT_COUNT - invbag::SLOT_COUNT) * ((serverSlot - EQ::invbag::TRADE_BAGS_BEGIN) / EQ::invbag::SLOT_COUNT)); // + 0;
		}

		else if (serverSlot <= EQ::invslot::WORLD_END && serverSlot >= EQ::invslot::WORLD_BEGIN) {
			UFSlot = serverSlot;
		}

		LogNetcode("Convert Server Slot [{}] to UF Slot [{}]", serverSlot, UFSlot);

		return UFSlot;
	}

	static inline uint32 ServerToUFCorpseSlot(uint32 serverCorpseSlot)
	{
		uint32 UFSlot = invslot::SLOT_INVALID;

		if (serverCorpseSlot <= EQ::invslot::slotGeneral8 && serverCorpseSlot >= EQ::invslot::slotGeneral1) {
			UFSlot = serverCorpseSlot;
		}

		else if (serverCorpseSlot <= EQ::invslot::CORPSE_END && serverCorpseSlot >= EQ::invslot::slotCursor) {
			UFSlot = serverCorpseSlot - 2;
		}

		LogNetcode("Convert Server Corpse Slot [{}] to UF Corpse Slot [{}]", serverCorpseSlot, UFSlot);

		return UFSlot;
	}

	static inline uint32 UFToServerSlot(uint32 ufSlot)
	{
		uint32 ServerSlot = EQ::invslot::SLOT_INVALID;

		if (ufSlot <= invslot::slotGeneral8) {
			ServerSlot = ufSlot;
		}

		else if (ufSlot <= invslot::CORPSE_END && ufSlot >= invslot::slotCursor) {
			ServerSlot = ufSlot + 2;
		}

		else if (ufSlot <= invbag::GENERAL_BAGS_END && ufSlot >= invbag::GENERAL_BAGS_BEGIN) {
			ServerSlot = ufSlot + (EQ::invbag::GENERAL_BAGS_BEGIN - invbag::GENERAL_BAGS_BEGIN)/*3748*/ + ((EQ::invbag::SLOT_COUNT - invbag::SLOT_COUNT) * ((ufSlot - invbag::GENERAL_BAGS_BEGIN) / invbag::SLOT_COUNT)); // - 11;
		}

		else if (ufSlot <= invbag::CURSOR_BAG_END && ufSlot >= invbag::CURSOR_BAG_BEGIN) {
			ServerSlot = ufSlot + (EQ::invbag::CURSOR_BAG_BEGIN - invbag::CURSOR_BAG_BEGIN)/*5668*/; // + 9;
		}

		else if (ufSlot <= invslot::TRIBUTE_END && ufSlot >= invslot::TRIBUTE_BEGIN) {
			ServerSlot = ufSlot;
		}

		else if (ufSlot <= invslot::GUILD_TRIBUTE_END && ufSlot >= invslot::GUILD_TRIBUTE_BEGIN) {
			ServerSlot = ufSlot;
		}

		else if (ufSlot == invslot::SLOT_TRADESKILL_EXPERIMENT_COMBINE) {
			ServerSlot = ufSlot;
		}

		else if (ufSlot <= invslot::BANK_END && ufSlot >= invslot::BANK_BEGIN) {
			ServerSlot = ufSlot;
		}

		else if (ufSlot <= invbag::BANK_BAGS_END && ufSlot >= invbag::BANK_BAGS_BEGIN) {
			ServerSlot = ufSlot + (EQ::invbag::BANK_BAGS_BEGIN - invbag::BANK_BAGS_BEGIN) + ((EQ::invbag::SLOT_COUNT - invbag::SLOT_COUNT) * ((ufSlot - invbag::BANK_BAGS_BEGIN) / invbag::SLOT_COUNT)); // - 1;
		}

		else if (ufSlot <= invslot::SHARED_BANK_END && ufSlot >= invslot::SHARED_BANK_BEGIN) {
			ServerSlot = ufSlot;
		}

		else if (ufSlot <= invbag::SHARED_BANK_BAGS_END && ufSlot >= invbag::SHARED_BANK_BAGS_BEGIN) {
			ServerSlot = ufSlot + (EQ::invbag::SHARED_BANK_BAGS_BEGIN - invbag::SHARED_BANK_BAGS_BEGIN) + ((EQ::invbag::SLOT_COUNT - invbag::SLOT_COUNT) * ((ufSlot - invbag::SHARED_BANK_BAGS_BEGIN) / invbag::SLOT_COUNT)); // - 1;
		}

		else if (ufSlot <= invslot::TRADE_END && ufSlot >= invslot::TRADE_BEGIN) {
			ServerSlot = ufSlot;
		}

		else if (ufSlot <= invbag::TRADE_BAGS_END && ufSlot >= invbag::TRADE_BAGS_BEGIN) {
			ServerSlot = ufSlot + (EQ::invbag::TRADE_BAGS_BEGIN - invbag::TRADE_BAGS_BEGIN) + ((EQ::invbag::SLOT_COUNT - invbag::SLOT_COUNT) * ((ufSlot - invbag::TRADE_BAGS_BEGIN) / invbag::SLOT_COUNT)); // - 0;
		}

		else if (ufSlot <= invslot::WORLD_END && ufSlot >= invslot::WORLD_BEGIN) {
			ServerSlot = ufSlot;
		}

		LogNetcode("Convert UF Slot [{}] to Server Slot [{}]", ufSlot, ServerSlot);

		return ServerSlot;
	}

	static inline uint32 UFToServerCorpseSlot(uint32 ufCorpseSlot)
	{
		uint32 ServerSlot = EQ::invslot::SLOT_INVALID;

		if (ufCorpseSlot <= invslot::slotGeneral8 && ufCorpseSlot >= invslot::slotGeneral1) {
			ServerSlot = ufCorpseSlot;
		}

		else if (ufCorpseSlot <= invslot::CORPSE_END && ufCorpseSlot >= invslot::slotCursor) {
			ServerSlot = ufCorpseSlot + 2;
		}

		LogNetcode("Convert UF Corpse Slot [{}] to Server Corpse Slot [{}]", ufCorpseSlot, ServerSlot);

		return ServerSlot;
	}

	static inline void ServerToUFSayLink(std::string& ufSayLink, const std::string& serverSayLink)
	{
		if ((constants::SAY_LINK_BODY_SIZE == EQ::constants::SAY_LINK_BODY_SIZE) || (serverSayLink.find('\x12') == std::string::npos)) {
			ufSayLink = serverSayLink;
			return;
		}

		auto segments = Strings::Split(serverSayLink, '\x12');

		for (size_t segment_iter = 0; segment_iter < segments.size(); ++segment_iter) {
			if (segment_iter & 1) {
				if (segments[segment_iter].length() <= EQ::constants::SAY_LINK_BODY_SIZE) {
					ufSayLink.append(segments[segment_iter]);
					// TODO: log size mismatch error
					continue;
				}

				// Idx:  0 1     6     11    16    21    26    31    36 37   41 43    48       (Source)
				// RoF2: X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX X  XXXX XX XXXXX XXXXXXXX (56)
				// SoF:  X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX       X  XXXX  X XXXXX XXXXXXXX (50)
				// Diff:                                       ^^^^^         ^

				ufSayLink.push_back('\x12');
				ufSayLink.append(segments[segment_iter].substr(0, 31));
				ufSayLink.append(segments[segment_iter].substr(36, 5));

				if (segments[segment_iter][41] == '0')
					ufSayLink.push_back(segments[segment_iter][42]);
				else
					ufSayLink.push_back('F');

				ufSayLink.append(segments[segment_iter].substr(43));
				ufSayLink.push_back('\x12');
			}
			else {
				ufSayLink.append(segments[segment_iter]);
			}
		}
	}

	static inline void UFToServerSayLink(std::string& serverSayLink, const std::string& ufSayLink)
	{
		if ((EQ::constants::SAY_LINK_BODY_SIZE == constants::SAY_LINK_BODY_SIZE) || (ufSayLink.find('\x12') == std::string::npos)) {
			serverSayLink = ufSayLink;
			return;
		}

		auto segments = Strings::Split(ufSayLink, '\x12');

		for (size_t segment_iter = 0; segment_iter < segments.size(); ++segment_iter) {
			if (segment_iter & 1) {
				if (segments[segment_iter].length() <= constants::SAY_LINK_BODY_SIZE) {
					serverSayLink.append(segments[segment_iter]);
					// TODO: log size mismatch error
					continue;
				}

				// Idx:  0 1     6     11    16    21    26          31 32    36 37    42       (Source)
				// SoF:  X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX       X  XXXX  X  XXXXX XXXXXXXX (50)
				// RoF2: X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX X  XXXX XX  XXXXX XXXXXXXX (56)
				// Diff:                                       ^^^^^         ^

				serverSayLink.push_back('\x12');
				serverSayLink.append(segments[segment_iter].substr(0, 31));
				serverSayLink.append("00000");
				serverSayLink.append(segments[segment_iter].substr(31, 5));
				serverSayLink.push_back('0');
				serverSayLink.append(segments[segment_iter].substr(36));
				serverSayLink.push_back('\x12');
			}
			else {
				serverSayLink.append(segments[segment_iter]);
			}
		}
	}

	static inline spells::CastingSlot ServerToUFCastingSlot(EQ::spells::CastingSlot slot)
	{
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

	static inline EQ::spells::CastingSlot UFToServerCastingSlot(spells::CastingSlot slot)
	{
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

	static inline int ServerToUFBuffSlot(int index)
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

	static inline int UFToServerBuffSlot(int index)
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
} /*UF*/
