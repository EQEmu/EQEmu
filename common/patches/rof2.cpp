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

#include "rof2.h"
#include "rof2_structs.h"

#include "common/classes.h"
#include "common/crc32.h"
#include "common/eq_packet_structs.h"
#include "common/eq_stream_ident.h"
#include "common/eqemu_config.h"
#include "common/eqemu_logsys.h"
#include "common/inventory_profile.h"
#include "common/misc_functions.h"
#include "common/opcodemgr.h"
#include "common/path_manager.h"
#include "common/races.h"
#include "common/raid.h"
#include "common/rulesys.h"
#include "common/strings.h"

#include <cassert>
#include <cinttypes>
#include <iostream>
#include <numeric>
#include <sstream>


namespace RoF2
{
	static const char *name = "RoF2";
	static OpcodeManager *opcodes = nullptr;
	static Strategy struct_strategy;

	void SerializeItem(EQ::OutBuffer& ob, const EQ::ItemInstance *inst, int16 slot_id, uint8 depth, ItemPacketType packet_type);

	// server to client inventory location converters
	static inline structs::InventorySlot_Struct ServerToRoF2Slot(uint32 server_slot);
	static inline structs::InventorySlot_Struct ServerToRoF2CorpseSlot(uint32 server_corpse_slot);
	static inline uint32 ServerToRoF2CorpseMainSlot(uint32 server_corpse_slot);
	static inline structs::TypelessInventorySlot_Struct ServerToRoF2TypelessSlot(uint32 server_slot, int16 server_type);

	// client to server inventory location converters
	static inline uint32 RoF2ToServerSlot(structs::InventorySlot_Struct rof2_slot);
	static inline uint32 RoF2ToServerCorpseSlot(structs::InventorySlot_Struct rof2_corpse_slot);
	static inline uint32 RoF2ToServerCorpseMainSlot(uint32 rof2_corpse_slot);
	static inline uint32 RoF2ToServerTypelessSlot(structs::TypelessInventorySlot_Struct rof2_slot, int16 rof2_type);

	// server to client say link converter
	static inline void ServerToRoF2SayLink(std::string &rof2_saylink, const std::string &server_saylink);

	// client to server say link converter
	static inline void RoF2ToServerSayLink(std::string &server_saylink, const std::string &rof2_saylink);

	static inline spells::CastingSlot ServerToRoF2CastingSlot(EQ::spells::CastingSlot slot);
	static inline EQ::spells::CastingSlot RoF2ToServerCastingSlot(spells::CastingSlot slot);

	static inline int ServerToRoF2BuffSlot(int index);
	static inline int RoF2ToServerBuffSlot(int index);

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
#include "rof2_ops.h"
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
		return EQ::versions::ClientVersion::RoF2;
	}

#include "ss_define.h"

// ENCODE methods


	// RoF2 Specific Encodes Begin
	ENCODE(OP_SendMembershipDetails)
	{
		ENCODE_LENGTH_EXACT(Membership_Details_Struct);
		SETUP_DIRECT_ENCODE(Membership_Details_Struct, structs::Membership_Details_Struct);

		eq->membership_setting_count = 72;
		for (uint32 i = 0; i < emu->membership_setting_count; ++i) // 66
		{
			OUT(settings[i].setting_index);
			OUT(settings[i].setting_id);
			OUT(settings[i].setting_value);
		}
		// Last 6 new settings fields are all 0s on Live as of 12/29/14

		eq->race_entry_count = emu->race_entry_count;
		for (uint32 i = 0; i < emu->race_entry_count; ++i) // 15
		{
			OUT(membership_races[i].purchase_id);
			OUT(membership_races[i].bitwise_entry);
		}

		eq->class_entry_count = emu->class_entry_count;
		for (uint32 i = 0; i < emu->class_entry_count; ++i) // 15
		{
			OUT(membership_classes[i].purchase_id);
			OUT(membership_classes[i].bitwise_entry);
		}

		eq->exit_url_length = emu->exit_url_length;
		eq->exit_url_length2 = emu->exit_url_length2;

		FINISH_ENCODE();
	}

	ENCODE(OP_SendMembership)
	{
		ENCODE_LENGTH_EXACT(Membership_Struct);
		SETUP_DIRECT_ENCODE(Membership_Struct, structs::Membership_Struct);

		eq->membership = emu->membership;
		eq->races = emu->races;
		eq->classes = emu->classes;
		eq->entrysize = 25; //emu->entrysize;

		for (uint32 i = 0; i < emu->entrysize; ++i) // 21
		{
			OUT(entries[i]);
		}
		// Last 4 new entries are 0s on Live Silver as of 12/29/14
		// Setting them each to 1 for now.
		// This removes the "Buy Now" button from aug type 21 slots on items.
		for (uint32 i = 21; i < 25; ++i) // 4
		{
			eq->entries[i] = 1;
		}


		FINISH_ENCODE();
	}

	// RoF2 Specific Encodes End


	ENCODE(OP_Action)
	{
		ENCODE_LENGTH_EXACT(Action_Struct);
		SETUP_DIRECT_ENCODE(Action_Struct, structs::ActionAlt_Struct);

		OUT(target);
		OUT(source);
		OUT(level);
		eq->unknown06 = 0;
		eq->instrument_mod = 1.0f + (emu->instrument_mod - 10) / 10.0f;
		OUT(force);
		OUT(hit_heading);
		OUT(hit_pitch);
		OUT(type);
		eq->damage = 0;
		eq->unknown31 = 0;
		OUT(spell);
		OUT(spell_level);
		OUT(effect_flag);
		eq->spell_gem = 0;
		eq->slot.Type = INVALID_INDEX;
		eq->slot.Unknown02 = 0;
		eq->slot.Slot = INVALID_INDEX;
		eq->slot.SubIndex = INVALID_INDEX;
		eq->slot.AugIndex = INVALID_INDEX;
		eq->slot.Unknown01 = 0;
		eq->item_cast_type = 0;

		FINISH_ENCODE();
	}

	ENCODE(OP_AdventureMerchantSell)
	{
		ENCODE_LENGTH_EXACT(Adventure_Sell_Struct);
		SETUP_DIRECT_ENCODE(Adventure_Sell_Struct, structs::Adventure_Sell_Struct);

		eq->unknown000 = 1;
		OUT(npcid);
		eq->inventory_slot = ServerToRoF2TypelessSlot(emu->slot, EQ::invtype::typePossessions);
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
				out_populate->entries[i].unknown00 = populate->entries[i].unknown00;
				out_populate->entries[i].currency_number2 = populate->entries[i].currency_number2;
				out_populate->entries[i].item_id = populate->entries[i].item_id;
				out_populate->entries[i].item_icon = populate->entries[i].item_icon;
				out_populate->entries[i].stack_size = populate->entries[i].stack_size;
				out_populate->entries[i].display = ((populate->entries[i].stack_size > 0) ? 1 : 0);
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
		eq->inventory_slot = ServerToRoF2TypelessSlot(emu->slot_id, EQ::invtype::typePossessions);
		OUT(charges);
		OUT(cost);

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

		eq->inventorySlot = ServerToRoF2TypelessSlot(emu->inventorySlot, EQ::invtype::typePossessions);
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
		EQApplicationPacket *in        = *p;
		*p = nullptr;

		char *buffer = (char *) in->pBuffer;
		uint32              sub_action = VARSTRUCT_DECODE_TYPE(uint32, buffer);

		switch (sub_action) {
			case Barter_BuyerAppearance: {
				auto emu = (BuyerInspectRequest_Struct *) in->pBuffer;

				auto outapp = new EQApplicationPacket(OP_Barter, sizeof(structs::Buyer_SetAppearance_Struct));
				auto eq     = (structs::Buyer_SetAppearance_Struct *) outapp->pBuffer;

				eq->action    = structs::RoF2BuyerActions::BuyerAppearance;
				eq->entity_id = emu->buyer_id;
				eq->enabled   = emu->approval;

				dest->FastQueuePacket(&outapp);
				safe_delete(in);

				break;
			}
			case Barter_BuyerItemRemove: {
				auto emu = (BuyerRemoveItem_Struct *) in->pBuffer;

				auto outapp = new EQApplicationPacket(OP_BuyerItems, sizeof(structs::BuyerRemoveItem_Struct));
				auto eq     = (structs::BuyerRemoveItem_Struct *) outapp->pBuffer;

				eq->action  = structs::RoF2BuyerActions::BuyerModifyBuyLine;
				eq->slot_id = emu->buy_slot_id;
				eq->toggle  = 0;

				dest->FastQueuePacket(&outapp);
				safe_delete(in);

				break;
			}
			case Barter_BuyerInspectBegin: {
				*(uint32 *) in->pBuffer = structs::RoF2BuyerActions::BuyerInspectBegin;
				dest->FastQueuePacket(&in);
				break;
			}
			case Barter_BuyerInspectEnd: {
				*(uint32 *) in->pBuffer = structs::RoF2BuyerActions::BuyerInspectEnd;
				dest->FastQueuePacket(&in);
				break;
			}
			case Barter_SellerBrowsing: {
				*(uint32 *) in->pBuffer = structs::RoF2BuyerActions::BuyerBrowsingBuyLine;
				dest->FastQueuePacket(&in);
				break;
			}
			case Barter_BuyerSearchResults: {
				BuyerItemSearchResults_Struct bisr{};
				auto                          emu = (BuyerGeneric_Struct *) in->pBuffer;
				EQ::Util::MemoryStreamReader  ss(
					reinterpret_cast<char *>(emu->payload),
					in->size - sizeof(BuyerGeneric_Struct)
				);
				cereal::BinaryInputArchive    ar(ss);
				ar(bisr);

				LogTradingDetail("Sending item search results <green>[{}]", bisr.result_count);

				uint32 packet_size = bisr.result_count * sizeof(structs::BuyerItemSearchResultEntry_Struct) + 8;
				auto   outapp      = std::make_unique<EQApplicationPacket>(OP_Barter, packet_size);
				auto   eq          = (char *) outapp->pBuffer;

				VARSTRUCT_ENCODE_TYPE(uint32, eq, structs::RoF2BuyerActions::BuyerSearchResults);
				VARSTRUCT_ENCODE_TYPE(uint32, eq, bisr.result_count);
				for (auto const &i: bisr.results) {
					strn0cpy(eq, i.item_name, 64);
					eq += 64;
					VARSTRUCT_ENCODE_TYPE(uint32, eq, i.item_id);
					VARSTRUCT_ENCODE_TYPE(uint32, eq, i.item_icon);
					VARSTRUCT_SKIP_TYPE(uint32, eq);
				}

				dest->QueuePacket(outapp.get());
				safe_delete(in);
				break;
			}
			default: {
				LogTradingDetail("Unhandled action <red>[{}]", sub_action);
				dest->FastQueuePacket(&in);
			}
		}
	}

	ENCODE(OP_BazaarSearch)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		uint32 action = *(uint32 *) in->pBuffer;

		switch (action) {
			case BazaarSearch: {
				LogTrading("(RoF2) BazaarSearch action <green>[{}]", action);
				std::vector<BazaarSearchResultsFromDB_Struct> results{};
				auto                                          bsms = (BazaarSearchMessaging_Struct *) in->pBuffer;
				EQ::Util::MemoryStreamReader                  ss(
					reinterpret_cast<char *>(bsms->payload),
					in->size - sizeof(BazaarSearchMessaging_Struct)
				);
				cereal::BinaryInputArchive                    ar(ss);
				ar(results);

				auto name_size = 0;
				for (auto const &i: results) {
					name_size += i.item_name.length() + 1;
				}

				auto p_size = 41 * results.size() + name_size + 14;
				auto buffer = new char[p_size];
				auto bufptr = buffer;

				VARSTRUCT_ENCODE_TYPE(uint32, bufptr, 0);
				VARSTRUCT_ENCODE_TYPE(uint16, bufptr, results[0].trader_zone_id);
				VARSTRUCT_ENCODE_TYPE(uint32, bufptr, results[0].trader_id);
				VARSTRUCT_ENCODE_TYPE(uint32, bufptr, results.size());

				for (auto i: results) {
					VARSTRUCT_ENCODE_TYPE(uint32, bufptr, i.trader_id);                          //trader ID
					VARSTRUCT_ENCODE_STRING(bufptr, i.serial_number_RoF.c_str());                //serial
					VARSTRUCT_ENCODE_TYPE(uint32, bufptr, i.cost);                               //cost
					VARSTRUCT_ENCODE_TYPE(uint32, bufptr, i.stackable ? i.charges : i.count);    //quantity
					VARSTRUCT_ENCODE_TYPE(uint32, bufptr, i.item_id);                            //ID
					VARSTRUCT_ENCODE_TYPE(uint32, bufptr, i.icon_id);                            //icon
					VARSTRUCT_ENCODE_STRING(bufptr, i.item_name.c_str());                        //name
					VARSTRUCT_ENCODE_TYPE(uint32, bufptr, i.item_stat);                          //itemstat
				}

				safe_delete_array(in->pBuffer);
				in->size    = p_size;
				in->pBuffer = (uchar*)buffer;
				dest->QueuePacket(in);
				safe_delete(in);

				break;
			}
			case BazaarInspect: {
				LogTrading("(RoF2) BazaarInspect action <green>[{}]", action);
				dest->FastQueuePacket(&in, ack_req);
				break;
			}
			case WelcomeMessage: {
				auto buffer = new char[sizeof(structs::BazaarWelcome_Struct)];
				auto emu    = (BazaarWelcome_Struct *) in->pBuffer;
				auto eq     = (structs::BazaarWelcome_Struct *) buffer;

				eq->action         = structs::RoF2BazaarTraderBuyerActions::WelcomeMessage;
				eq->num_of_traders = emu->traders;
				eq->num_of_items   = emu->items;

				safe_delete_array(in->pBuffer);
				in->SetOpcode(OP_TraderShop);
				in->size    = sizeof(structs::BazaarWelcome_Struct);
				in->pBuffer = (uchar *)buffer;

				LogTrading("(RoF2) WelcomeMessage action <green>[{}]", action);
				dest->QueuePacket(in);
				safe_delete(in);

				break;
			}
			case DeliveryCostUpdate: {
				auto data = (BazaarDeliveryCost_Struct *) in->pBuffer;
				LogTrading("(RoF2) Delivery costs updated: vouchers <green>[{}] parcel percentage <green>[{}]",
						   data->voucher_delivery_cost,
						   data->parcel_deliver_cost
				);
				data->action = 0;
				dest->FastQueuePacket(&in);
				break;
			}
			default: {
				LogTrading("(RoF2) Unhandled action <red>[{}]", action);
				dest->FastQueuePacket(&in, ack_req);
			}
		}
	}

	ENCODE(OP_BecomeTrader)
	{
		EQApplicationPacket *inapp = *p;
		*p = nullptr;

		unsigned char *__emu_buffer = inapp->pBuffer;
		auto          in            = (BecomeTrader_Struct *) __emu_buffer;

		switch (in->action) {
			case TraderOff: {
				auto emu = (BecomeTrader_Struct *) __emu_buffer;

				auto outapp = new EQApplicationPacket(OP_BecomeTrader, sizeof(structs::BecomeTrader_Struct));
				auto eq     = (structs::BecomeTrader_Struct *) outapp->pBuffer;

				eq->action    = TraderOff;
				eq->entity_id = emu->entity_id;

				LogTrading(
					"(RoF2) TraderOff action <green>[{}] for entity_id <green>[{}]",
					eq->action,
					eq->entity_id
				);
				dest->FastQueuePacket(&outapp);
				break;
			}
			case TraderOn: {
				auto emu = (BecomeTrader_Struct *) __emu_buffer;

				auto outapp = new EQApplicationPacket(OP_BecomeTrader, sizeof(structs::BecomeTrader_Struct));
				auto eq     = (structs::BecomeTrader_Struct *) outapp->pBuffer;

				eq->action    = TraderOn;
				eq->entity_id = emu->entity_id;

				LogTrading(
					"(RoF2) TraderOn action <green>[{}] for entity_id <green>[{}]",
					eq->action,
					eq->entity_id
				);
				dest->FastQueuePacket(&outapp);
				break;
			}
			case AddTraderToBazaarWindow: {
				auto emu    = (BecomeTrader_Struct *) __emu_buffer;
				auto outapp = new EQApplicationPacket(OP_TraderShop, sizeof(BecomeTrader_Struct));
				auto eq     = (BecomeTrader_Struct *) outapp->pBuffer;

				eq->action           = emu->action;
				eq->entity_id        = emu->entity_id;
				eq->trader_id        = emu->trader_id;
				eq->zone_id          = emu->zone_id;
				eq->zone_instance_id = emu->zone_instance_id;
				strn0cpy(eq->trader_name, emu->trader_name, sizeof(eq->trader_name));

				LogTrading(
					"(RoF2) AddTraderToBazaarWindow action <green>[{}] trader_id <green>[{}] entity_id <green>[{}] "
					"zone_id <green>[{}] zone_instance_id <green>[{}]",
					eq->action,
					eq->trader_id,
					eq->entity_id,
					eq->zone_id,
					eq->zone_instance_id);
				dest->FastQueuePacket(&outapp);
				break;
			}
			case RemoveTraderFromBazaarWindow: {
				auto emu    = (BecomeTrader_Struct *) __emu_buffer;
				auto outapp = new EQApplicationPacket(OP_TraderShop, sizeof(structs::BazaarWindowRemoveTrader_Struct));
				auto eq     = (structs::BazaarWindowRemoveTrader_Struct *) outapp->pBuffer;

				eq->action    = emu->action;
				eq->trader_id = emu->trader_id;

				LogTrading(
					"(RoF2) RemoveTraderFromBazaarWindow action <green>[{}] for entity_id <green>[{}]",
					eq->action,
					eq->trader_id
				);
				dest->FastQueuePacket(&outapp);
				break;
			}
			default: {
				LogTrading(
					"(RoF2) Unhandled action <red>[{}]",
					in->action
				);
				dest->QueuePacket(inapp);
			}
		}
		safe_delete(inapp);
	}

	ENCODE(OP_BeginCast)
	{
		SETUP_DIRECT_ENCODE(BeginCast_Struct, structs::BeginCast_Struct);

		OUT(spell_id);
		OUT(caster_id);
		OUT(cast_time);

		FINISH_ENCODE();
	}

	ENCODE(OP_BlockedBuffs)
	{
		ENCODE_LENGTH_EXACT(BlockedBuffs_Struct);
		SETUP_DIRECT_ENCODE(BlockedBuffs_Struct, structs::BlockedBuffs_Struct);

		for (uint32 i = 0; i < BLOCKED_BUFF_COUNT; ++i)
			eq->SpellID[i] = emu->SpellID[i];

		// -1 for the extra 10 added in RoF2. We should really be encoding for the older clients, not RoF2, but
		// we can sort that out later.

		for (uint32 i = BLOCKED_BUFF_COUNT; i < structs::BLOCKED_BUFF_COUNT; ++i)
			eq->SpellID[i] = -1;

		OUT(Count);
		OUT(Pet);
		OUT(Initialise);
		OUT(Flags);

		FINISH_ENCODE();
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
		OUT(buff.player_id);
		OUT(buff.num_hits);
		OUT(buff.y);
		OUT(buff.x);
		OUT(buff.z);
		eq->slotid = ServerToRoF2BuffSlot(emu->slotid);
		// TODO: implement slot_data stuff
		if (emu->bufffade == 1)
			eq->bufffade = 1;
		else
			eq->bufffade = 2;

		// Bit of a hack. OP_Buff appears to add/remove the buff while OP_BuffCreate adds/removes the actual buff icon
		EQApplicationPacket *outapp = nullptr;
		if (eq->bufffade == 1)
		{
			outapp = new EQApplicationPacket(OP_BuffCreate, 29);
			outapp->WriteUInt32(emu->entityid);
			outapp->WriteUInt32(0);	// tic timer
			outapp->WriteUInt8(0);		// Type of OP_BuffCreate packet ?
			outapp->WriteUInt16(1);		// 1 buff in this packet
			outapp->WriteUInt32(eq->slotid);
			outapp->WriteUInt32(0xffffffff);		// SpellID (0xffff to remove)
			outapp->WriteUInt32(0);			// Duration
			outapp->WriteUInt32(0);			// numhits
			outapp->WriteUInt8(0);		// Caster name
			outapp->WriteUInt8(0);		// Type
		}
		FINISH_ENCODE();

		if (outapp)
			dest->FastQueuePacket(&outapp);	// Send the OP_BuffCreate to remove the buff
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
		__packet->WriteUInt8(emu->all_buffs);			// 1 indicates all buffs on the player (0 to add or remove a single buff)
		__packet->WriteUInt16(emu->count);

		for (int i = 0; i < emu->count; ++i)
		{
			__packet->WriteUInt32(emu->type == 0 ? ServerToRoF2BuffSlot(emu->entries[i].buff_slot) : emu->entries[i].buff_slot);
			__packet->WriteUInt32(emu->entries[i].spell_id);
			__packet->WriteUInt32(emu->entries[i].tics_remaining);
			__packet->WriteUInt32(emu->entries[i].num_hits); // Unknown
			__packet->WriteString(emu->entries[i].caster);
		}
		__packet->WriteUInt8(emu->type); // Unknown

		FINISH_ENCODE();
	}

	ENCODE(OP_BuyerItems)
	{
		EQApplicationPacket *inapp = *p;
		*p = nullptr;

		auto action = *(uint32 *) inapp->pBuffer;

		switch (action) {
			case Barter_BuyerItemUpdate: {
				BuyerLineItems_Struct        bl{};
				auto                         emu = (BuyerGeneric_Struct *) inapp->pBuffer;
				EQ::Util::MemoryStreamReader ss(
					reinterpret_cast<char *>(emu->payload),
					inapp->size - sizeof(BuyerGeneric_Struct)
				);
				cereal::BinaryInputArchive   ar(ss);
				ar(bl);

				//packet size
				auto            packet_size = bl.item_name.length() + 1 + 34;
				for (auto const &b: bl.trade_items) {
					packet_size += b.item_name.length() + 1;
					packet_size += 12;
				}

				auto outapp         = std::make_unique<EQApplicationPacket>(OP_BuyerItems, packet_size);
				char *eq            = (char *) outapp->pBuffer;
				auto no_trade_items = bl.trade_items.size();

				VARSTRUCT_ENCODE_TYPE(uint32, eq, structs::RoF2BuyerActions::BuyerModifyBuyLine);
				VARSTRUCT_ENCODE_TYPE(uint32, eq, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, eq, bl.slot);
				VARSTRUCT_ENCODE_TYPE(uint8, eq, bl.enabled ? 1 : 0);
				VARSTRUCT_ENCODE_TYPE(uint32, eq, bl.item_id);
				VARSTRUCT_ENCODE_STRING(eq, bl.item_name.c_str());
				VARSTRUCT_ENCODE_TYPE(uint32, eq, bl.item_icon);
				VARSTRUCT_ENCODE_TYPE(uint32, eq, bl.item_quantity);
				VARSTRUCT_ENCODE_TYPE(uint8, eq, bl.item_toggle ? 1 : 0);
				VARSTRUCT_ENCODE_TYPE(uint32, eq, bl.item_cost);
				VARSTRUCT_ENCODE_TYPE(uint32, eq, no_trade_items);

				for (int i = 0; i < no_trade_items; i++) {
					VARSTRUCT_ENCODE_TYPE(uint32, eq, bl.trade_items[i].item_id);
					VARSTRUCT_ENCODE_TYPE(uint32, eq, bl.trade_items[i].item_quantity);
					VARSTRUCT_ENCODE_TYPE(uint32, eq, bl.trade_items[i].item_icon);
					VARSTRUCT_ENCODE_STRING(eq, bl.trade_items[i].item_name.c_str());
				}
				dest->QueuePacket(outapp.get());
				safe_delete(inapp);
				break;
			}
			case Barter_BuyerInspectBegin: {
				auto emu = (BuyerGeneric_Struct *) inapp->pBuffer;

				BuyerLineItems_Struct        bli{};
				EQ::Util::MemoryStreamReader ss(
					reinterpret_cast<char *>(emu->payload),
					inapp->size - sizeof(BuyerGeneric_Struct)
				);
				cereal::BinaryInputArchive   ar(ss);
				ar(bli);

				//packet size
				auto            packet_size = bli.item_name.length() + 1 + 34;
				for (auto const &b: bli.trade_items) {
					packet_size += b.item_name.length() + 1;
					packet_size += 12;
				}

				auto packet         = std::make_unique<EQApplicationPacket>(OP_BuyerItems, packet_size);
				char *eq            = (char *) packet->pBuffer;
				auto no_trade_items = bli.trade_items.size();

				VARSTRUCT_ENCODE_TYPE(uint32, eq, structs::RoF2BuyerActions::BuyerSendBuyLine);
				VARSTRUCT_ENCODE_TYPE(uint32, eq, bli.slot);
				VARSTRUCT_ENCODE_TYPE(uint32, eq, bli.slot);
				VARSTRUCT_ENCODE_TYPE(uint8, eq, bli.enabled ? 1 : 0);
				VARSTRUCT_ENCODE_TYPE(uint32, eq, bli.item_id);
				VARSTRUCT_ENCODE_STRING(eq, bli.item_name.c_str());
				VARSTRUCT_ENCODE_TYPE(uint32, eq, bli.item_icon);
				VARSTRUCT_ENCODE_TYPE(uint32, eq, bli.item_quantity);
				VARSTRUCT_ENCODE_TYPE(uint8, eq, bli.item_toggle ? 1 : 0);
				VARSTRUCT_ENCODE_TYPE(uint32, eq, bli.item_cost);
				VARSTRUCT_ENCODE_TYPE(uint32, eq, no_trade_items);

				for (auto const &i: bli.trade_items) {
					VARSTRUCT_ENCODE_TYPE(uint32, eq, i.item_id);
					VARSTRUCT_ENCODE_TYPE(uint32, eq, i.item_quantity);
					VARSTRUCT_ENCODE_TYPE(uint32, eq, i.item_icon);
					VARSTRUCT_ENCODE_STRING(eq, i.item_name.c_str());
				}
				dest->QueuePacket(packet.get());
				safe_delete(inapp);

				break;
			}
			case Barter_BuyerSearch: {
				BuyerLineSearch_Struct       bls{};
				auto                         emu = (BuyerGeneric_Struct *) inapp->pBuffer;
				EQ::Util::MemoryStreamReader ss(
					reinterpret_cast<char *>(emu->payload),
					inapp->size - sizeof(BuyerGeneric_Struct)
				);
				cereal::BinaryInputArchive   ar(ss);
				ar(bls);
				LogTrading("(RoF2) Barter_BuyerSearch action <green>[{}]", emu->action);

				//Calculate size of packet
				auto p_size = 0;
				p_size += 5 * sizeof(uint32) + 1 * sizeof(uint8);
				p_size += bls.search_string.length() + 1;
				for (auto const &b: bls.buy_line) {
					p_size += 6 * sizeof(uint32) + 2 * sizeof(uint8);
					p_size += strlen(b.item_name) + 1;
					p_size += b.buyer_name.length() + 1;
					for (auto const &d: b.trade_items) {
						if (d.item_id != 0) {
							p_size += d.item_name.length() + 1;
							p_size += 3 * sizeof(uint32);
						}
					}
					p_size += 3 * sizeof(uint32);
				}

				BuyerBuyLines_Struct bl{};
				auto outapp = std::make_unique<EQApplicationPacket>(OP_BuyerItems, p_size);
				auto eq     = (char *) outapp->pBuffer;

				VARSTRUCT_ENCODE_TYPE(uint32, eq, 1);
				VARSTRUCT_ENCODE_STRING(eq, bls.search_string.c_str());
				VARSTRUCT_ENCODE_TYPE(uint32, eq, bls.transaction_id);
				VARSTRUCT_ENCODE_TYPE(uint32, eq, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, eq, 0);
				VARSTRUCT_ENCODE_TYPE(uint8, eq, 1);
				VARSTRUCT_ENCODE_TYPE(uint32, eq, bls.no_items);
				for (auto const &b: bls.buy_line) {
					VARSTRUCT_ENCODE_TYPE(uint32, eq, b.slot);
					VARSTRUCT_ENCODE_TYPE(uint8, eq, 1);
					VARSTRUCT_ENCODE_TYPE(uint32, eq, b.item_id);
					VARSTRUCT_ENCODE_STRING(eq, b.item_name);
					VARSTRUCT_ENCODE_TYPE(uint32, eq, b.item_icon);
					VARSTRUCT_ENCODE_TYPE(uint32, eq, b.item_quantity);
					VARSTRUCT_ENCODE_TYPE(uint8, eq, 1);
					VARSTRUCT_ENCODE_TYPE(uint32, eq, b.item_cost);
					auto no_sub_items = b.trade_items.size();
					VARSTRUCT_ENCODE_TYPE(uint32, eq, no_sub_items);
					for (auto const &i: b.trade_items) {
						VARSTRUCT_ENCODE_TYPE(uint32, eq, i.item_id);
						VARSTRUCT_ENCODE_TYPE(uint32, eq, i.item_quantity);
						VARSTRUCT_ENCODE_TYPE(uint32, eq, i.item_icon);
						VARSTRUCT_ENCODE_STRING(eq, i.item_name.c_str());
					}
					VARSTRUCT_ENCODE_TYPE(uint32, eq, b.buyer_entity_id);
					VARSTRUCT_ENCODE_TYPE(uint32, eq, b.buyer_id);
					VARSTRUCT_ENCODE_TYPE(uint16, eq, b.buyer_zone_id);
					VARSTRUCT_ENCODE_TYPE(uint16, eq, b.buyer_zone_instance_id);
					VARSTRUCT_ENCODE_STRING(eq, b.buyer_name.c_str());
				}

				dest->QueuePacket(outapp.get());
				safe_delete(inapp);
				break;
			}
			case Barter_RemoveFromMerchantWindow: {
				auto emu = (BuyerRemoveItemFromMerchantWindow_Struct *) inapp->pBuffer;

				emu->action = structs::RoF2BuyerActions::BuyerSendBuyLine;
				dest->FastQueuePacket(&inapp);
				break;
			}
			case Barter_BuyerTransactionComplete:
			case Barter_SellerTransactionComplete: {
				BuyerLineSellItem_Struct     blsi{};
				auto                         emu = (BuyerGeneric_Struct *) inapp->pBuffer;
				EQ::Util::MemoryStreamReader ss(
					reinterpret_cast<char *>(emu->payload),
					inapp->size - sizeof(BuyerGeneric_Struct)
				);
				cereal::BinaryInputArchive   ar(ss);
				ar(blsi);

				//packet size
				auto            packet_size = strlen(blsi.item_name) * 2 + 2 + 48 + 30 + blsi.seller_name.length() + 1 +
											  blsi.buyer_name.length() + 1;
				for (auto const &b: blsi.trade_items) {
					packet_size += b.item_name.length() + 1;
					packet_size += 12;
				}

				auto outapp = std::make_unique<EQApplicationPacket>(OP_BuyerItems, packet_size);
				auto eq     = (char *) outapp->pBuffer;

				switch (action) {
					case Barter_BuyerTransactionComplete: {
						VARSTRUCT_ENCODE_TYPE(uint32, eq, structs::RoF2BuyerActions::BuyerBuyItem);
						break;
					}
					case Barter_SellerTransactionComplete: {
						VARSTRUCT_ENCODE_TYPE(uint32, eq, structs::RoF2BuyerActions::BuyerSellItem);
						break;
					}
				}
				VARSTRUCT_ENCODE_TYPE(uint32, eq, blsi.sub_action);
				VARSTRUCT_ENCODE_TYPE(uint32, eq, blsi.error_code);
				eq += 16;
				VARSTRUCT_ENCODE_STRING(eq, blsi.buyer_name.c_str());
				VARSTRUCT_ENCODE_STRING(eq, blsi.item_name);
				VARSTRUCT_ENCODE_STRING(eq, blsi.seller_name.c_str());
				VARSTRUCT_ENCODE_TYPE(uint32, eq, 0xFFFFFFFF);
				VARSTRUCT_ENCODE_TYPE(uint32, eq, 0xFFFFFFFF);
				eq += 1;
				VARSTRUCT_ENCODE_STRING(eq, blsi.item_name);
				eq += 9;
				VARSTRUCT_ENCODE_TYPE(uint32, eq, blsi.item_cost);
				VARSTRUCT_ENCODE_TYPE(uint32, eq, blsi.trade_items.size());

				for (auto const &i: blsi.trade_items) {
					VARSTRUCT_ENCODE_TYPE(uint32, eq, 0);
					VARSTRUCT_ENCODE_TYPE(uint32, eq, i.item_quantity);
					VARSTRUCT_ENCODE_TYPE(uint32, eq, 0);
					VARSTRUCT_ENCODE_STRING(eq, i.item_name.c_str());
				}

				VARSTRUCT_ENCODE_TYPE(uint32, eq, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, eq, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, eq, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, eq, 0xFFFFFF);
				VARSTRUCT_ENCODE_TYPE(uint32, eq, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, eq, blsi.seller_quantity);

				dest->QueuePacket(outapp.get());
				safe_delete(inapp);
				break;
			}
			default: {
				dest->FastQueuePacket(&inapp);
			}
		}
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
		ENCODE_LENGTH_EXACT(CastSpell_Struct);
		SETUP_DIRECT_ENCODE(CastSpell_Struct, structs::CastSpell_Struct);

		eq->slot = static_cast<uint32>(ServerToRoF2CastingSlot(static_cast<EQ::spells::CastingSlot>(emu->slot)));

		OUT(spell_id);
		eq->inventory_slot = ServerToRoF2Slot(emu->inventoryslot);
		//OUT(inventoryslot);
		OUT(target_id);

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
		ServerToRoF2SayLink(new_message, old_message);

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
			SerializeItem(ob, (const EQ::ItemInstance*)eq->inst, eq->slot_id, 0, ItemPacketCharInventory);
			if (ob.tellp() == last_pos)
				LogNetcode("RoF2::ENCODE(OP_CharInventory) Serialization failed on item slot [{}] during OP_CharInventory.  Item skipped", eq->slot_id);

			last_pos = ob.tellp();
		}

		in->size = ob.size();
		in->pBuffer = ob.detach();

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
		Log(Logs::Detail, Logs::Netcode, "RoF2::ENCODE(OP_DeleteCharge)");

		ENCODE_FORWARD(OP_MoveItem);
	}

	ENCODE(OP_DeleteItem)
	{
		ENCODE_LENGTH_EXACT(DeleteItem_Struct);
		SETUP_DIRECT_ENCODE(DeleteItem_Struct, structs::DeleteItem_Struct);

		eq->from_slot = ServerToRoF2Slot(emu->from_slot);
		eq->to_slot = ServerToRoF2Slot(emu->to_slot);
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
		ServerToRoF2SayLink(new_message, old_message);

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

	ENCODE(OP_EvolveItem)
	{
		EQApplicationPacket *in = *p;
		*p                      = nullptr;

		auto action = *reinterpret_cast<uint32 *>(in->pBuffer);

		switch (action) {
			case EvolvingItems::Actions::TRANSFER_WINDOW_DETAILS: {
				auto emu = reinterpret_cast<EvolveItemMessaging *>(in->pBuffer);

				EvolveXPWindowSend           e{};
				EQ::Util::MemoryStreamReader ss(emu->serialized_data, in->size - sizeof(emu->action));
				cereal::BinaryInputArchive ar(ss);
				ar(e);

				auto item_1 = static_cast<const EQ::ItemInstance *>(reinterpret_cast<EQ::InternalSerializedItem_Struct
					*>(e.serialize_item_1.data())->inst);
				auto item_2 = static_cast<const EQ::ItemInstance *>(reinterpret_cast<EQ::InternalSerializedItem_Struct
					*>(e.serialize_item_2.data())->inst);

				EQ::OutBuffer ob;

				SerializeItem(ob, item_1, 0, 0, ItemPacketMerchant);
				SerializeItem(ob, item_2, 0, 0, ItemPacketMerchant);

				auto out = std::make_unique<EQApplicationPacket>(
					OP_EvolveItem,
					sizeof(EvolveXPWindowSendDetails_Struct) + ob.size()
				);
				auto data = reinterpret_cast<EvolveXPWindowSendDetails_Struct *>(out->pBuffer);

				data->action             = e.action;
				data->compatibility      = e.compatibility;
				data->max_transfer_level = e.max_transfer_level;
				data->item1_unique_id    = e.item1_unique_id;
				data->item2_unique_id    = e.item2_unique_id;
				data->item1_present      = e.item1_present;
				data->item2_present      = e.item2_present;

				memcpy(data->serialize_data, ob.str().data(), ob.size());
				dest->QueuePacket(out.get());
				safe_delete(in);
				break;
			}
			default: {
				dest->FastQueuePacket(&in);
				break;
			}
		}
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
			ServerToRoF2SayLink(new_message_array[i], old_message_array[i]);
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

	ENCODE(OP_GMLastName)
	{
		ENCODE_LENGTH_EXACT(GMLastName_Struct);
		SETUP_DIRECT_ENCODE(GMLastName_Struct, structs::GMLastName_Struct);

		OUT_str(name);
		OUT_str(gmname);
		OUT_str(lastname);
		for (int i = 0; i<4; i++)
		{
			eq->unknown[i] = emu->unknown[i];
		}

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
		// We are not encoding the spawn_id field here, but it doesn't appear to matter.
		//
		EQApplicationPacket *in = *p;
		*p = nullptr;

		Object_Struct *emu = (Object_Struct *)in->pBuffer;

		unsigned char *__emu_buffer = in->pBuffer;

		in->size = strlen(emu->object_name) + sizeof(Object_Struct)-1;
		in->pBuffer = new unsigned char[in->size];

		char *OutBuffer = (char *)in->pBuffer;

		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->drop_id);
		VARSTRUCT_ENCODE_STRING(OutBuffer, emu->object_name);
		VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, emu->zone_id);
		VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, emu->zone_instance);
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, emu->drop_id);	// Some unique id
		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0);	// Same for all objects in the zone
		VARSTRUCT_ENCODE_TYPE(float, OutBuffer, emu->heading);
		VARSTRUCT_ENCODE_TYPE(float, OutBuffer, emu->tilt_x);	// X tilt
		VARSTRUCT_ENCODE_TYPE(float, OutBuffer, emu->tilt_y);	// Y tilt
		VARSTRUCT_ENCODE_TYPE(float, OutBuffer, emu->size != 0 && (float)emu->size < 5000.f ? (float)((float)emu->size / 100.0f) : 1.f );	// This appears to be the size field. Hackish logic because some PEQ DB items were corrupt.
		VARSTRUCT_ENCODE_TYPE(float, OutBuffer, emu->y);
		VARSTRUCT_ENCODE_TYPE(float, OutBuffer, emu->x);
		VARSTRUCT_ENCODE_TYPE(float, OutBuffer, emu->z);
		VARSTRUCT_ENCODE_TYPE(int32, OutBuffer, emu->object_type);	// Unknown, observed 0x00000014

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

			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);		// index
			VARSTRUCT_ENCODE_STRING(Buffer, gu2->yourname);	// name
			VARSTRUCT_ENCODE_TYPE(uint16, Buffer, 0);		// owner name of merc
			//VARSTRUCT_ENCODE_STRING(Buffer, "");
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);		// This is a string
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0x46);	// level
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);		// group tank flag
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);		// group assist flag
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);		// group puller flag
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);		// offline
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);		// timestamp

			int MemberNumber = 1;

			for (int i = 0; i < 5; ++i)
			{
				if (gu2->membername[i][0] == '\0')
					continue;

				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, MemberNumber++);	// index
				VARSTRUCT_ENCODE_STRING(Buffer, gu2->membername[i]);	// name
				VARSTRUCT_ENCODE_TYPE(uint16, Buffer, 0);				// merc flag
				//VARSTRUCT_ENCODE_STRING(Buffer, "");
				VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);				// owner name fo merc
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0x41);			// level
				VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);				// group tank flag
				VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);				// group assist flag
				VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);				// group puller flag
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);				// offline
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);				// timestamp
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

	ENCODE(OP_GuildBank)
	{
		auto in = *p;
		*p = nullptr;
		auto outapp = new EQApplicationPacket(OP_GuildBank, in->size + 4); // all of them are 4 bytes bigger

		// The first action in the enum was removed, everything 1 less
		// Normally we cast them to their structs, but there are so many here! will only do when it's easier
		switch (in->ReadUInt32()) {
		case 10: // GuildBankAcknowledge
			outapp->WriteUInt32(9);
			outapp->WriteUInt32(in->ReadUInt32());
			outapp->WriteUInt32(0);
			break;
		case 5: // GuildBankDeposit (ack)
			outapp->WriteUInt32(4);
			outapp->WriteUInt32(in->ReadUInt32());
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(in->ReadUInt32());
			break;
		case 1: { // GuildBankItemUpdate
			auto emu = (GuildBankItemUpdate_Struct *)in->pBuffer;
			auto eq = (structs::GuildBankItemUpdate_Struct *)outapp->pBuffer;
			eq->action = 0;
			OUT(display);
			OUT(slot_id);
			OUT(area);
			OUT(item_id);
			OUT(icon_id);
			OUT(quantity);
			OUT(permissions);
			OUT(allow_merge);
			OUT(is_useable);
			OUT_str(item_name);
			OUT_str(donator);
			OUT_str(who_for);
			break;
		}
		default:
			break;
		}
		delete in;
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

		// Guild ID
		//*((uint32*)buffer) = htonl(2);
		buffer += sizeof(uint32);

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
				e->unknown01 = 0;
				PutFieldN(total_tribute);
				PutFieldN(last_tribute);
				e->unknown_one = htonl(1);
				SlideStructString(public_note, emu_note);
				e->zoneinstance = 0;
				e->zone_id = htons(emu_e->zone_id);
				e->unknown_one2 = htonl(1);
				e->unknown04 = 0;

#undef SlideStructString
#undef PutFieldN

				e++;
			}
		}

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_GuildMemberUpdate)
	{
		SETUP_DIRECT_ENCODE(GuildMemberUpdate_Struct, structs::GuildMemberUpdate_Struct);

		OUT(GuildID);
		memcpy(eq->MemberName, emu->MemberName, sizeof(eq->MemberName));
		//OUT(ZoneID);
		//OUT(InstanceID);
		eq->InstanceID = emu->InstanceID;
		eq->ZoneID = emu->ZoneID;
		OUT(LastSeen);
		eq->Unknown76 = 0;

		FINISH_ENCODE();
	}

	ENCODE(OP_GuildsList)
	{
		EQApplicationPacket *in = *p;
		*p                      = nullptr;

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

		for (auto const &g: glms.guild_detail) {
			if (g.guild_id < RoF2::constants::MAX_GUILD_ID) {
				VARSTRUCT_ENCODE_TYPE(uint32, buf_pos, g.guild_id);
				strn0cpy((char *) buf_pos, g.guild_name.c_str(), g.guild_name.length() + 1);
				buf_pos += g.guild_name.length() + 1;
			}
		}

		safe_delete_array(in->pBuffer);

		in->pBuffer = buffer;
		in->size    = packet_size;
		dest->FastQueuePacket(&in);
	}

	ENCODE(OP_GuildTributeDonateItem)
	{
		SETUP_DIRECT_ENCODE(GuildTributeDonateItemReply_Struct, structs::GuildTributeDonateItemReply_Struct);

		Log(Logs::Detail, Logs::Netcode, "RoF2::ENCODE(OP_GuildTributeDonateItem)");

		OUT(type);
		OUT(sub_index);
		OUT(aug_index);
		OUT(quantity);
		OUT(unknown10);
		OUT(unknown20);
		OUT(favor);

		structs::InventorySlot_Struct iss;
		iss = ServerToRoF2Slot(emu->slot);

		eq->slot = iss.Slot;
		eq->sub_index = iss.SubIndex;

		FINISH_ENCODE();
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
		eq->unknown316 = -1;	// Observed

		FINISH_ENCODE();
	}

	/*ENCODE(OP_InspectAnswer)
	{
	ENCODE_LENGTH_EXACT(InspectResponse_Struct);
	SETUP_DIRECT_ENCODE(InspectResponse_Struct, structs::InspectResponse_Struct);

	OUT(TargetID);
	OUT(playerid);

	int r;
	for (r = 0; r < 21; r++) {
	strn0cpy(eq->itemnames[r], emu->itemnames[r], sizeof(eq->itemnames[r]));
	}
	// Swap last 2 slots for Arrow and Power Source
	strn0cpy(eq->itemnames[21], emu->itemnames[22], sizeof(eq->itemnames[21]));
	strn0cpy(eq->unknown_zero, emu->itemnames[21], sizeof(eq->unknown_zero));

	int k;
	for (k = 0; k < 21; k++) {
	OUT(itemicons[k]);
	}
	// Swap last 2 slots for Arrow and Power Source
	eq->itemicons[21] = emu->itemicons[22];
	eq->unknown_zero2 = emu->itemicons[21];
	strn0cpy(eq->text, emu->text, sizeof(eq->text));

	FINISH_ENCODE();
	}*/

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
		uchar             *__emu_buffer = in->pBuffer;
		ItemPacket_Struct *old_item_pkt = (ItemPacket_Struct *) __emu_buffer;

		switch(old_item_pkt->PacketType)
		{
			case ItemPacketParcel: {
				ParcelMessaging_Struct       pms{};
				EQ::Util::MemoryStreamReader ss(reinterpret_cast<char *>(in->pBuffer), in->size);
				cereal::BinaryInputArchive   ar(ss);
				ar(pms);

				uint32 player_name_length = pms.player_name.length();
				uint32 note_length        = pms.note.length();

				auto *int_struct = (EQ::InternalSerializedItem_Struct *) pms.serialized_item.data();

				EQ::OutBuffer           ob;
				EQ::OutBuffer::pos_type last_pos = ob.tellp();
				ob.write(reinterpret_cast<const char *>(&pms.packet_type), 4);

				SerializeItem(ob, (const EQ::ItemInstance *) int_struct->inst, pms.slot_id, 0, ItemPacketParcel);

				if (ob.tellp() == last_pos) {
					LogNetcode("RoF2::ENCODE(OP_ItemPacket) Serialization failed on item slot [{}]", pms.slot_id);
					safe_delete_array(__emu_buffer);
					safe_delete(in);
					return;
				}

				ob.write((const char *) &pms.sent_time, 4);
				ob.write((const char *) &player_name_length, 4);
				ob.write(pms.player_name.c_str(), pms.player_name.length());
				ob.write((const char *) &note_length, 4);
				ob.write(pms.note.c_str(), pms.note.length());

				in->size    = ob.size();
				in->pBuffer = ob.detach();

				safe_delete_array(__emu_buffer);
				dest->FastQueuePacket(&in, ack_req);

				break;
			}
            default: {
                EQ::InternalSerializedItem_Struct *int_struct = (EQ::InternalSerializedItem_Struct *)(&__emu_buffer[4]);

                EQ::OutBuffer           ob;
                EQ::OutBuffer::pos_type last_pos = ob.tellp();

                ob.write((const char *)__emu_buffer, 4);

                SerializeItem(ob, (const EQ::ItemInstance *)int_struct->inst, int_struct->slot_id, 0,
                              old_item_pkt->PacketType);
                if (ob.tellp() == last_pos) {
                    LogNetcode("RoF2::ENCODE(OP_ItemPacket) Serialization failed on item slot [{}]",
                               int_struct->slot_id);
					safe_delete_array(__emu_buffer);
					safe_delete(in);
                    return;
                }

                in->size    = ob.size();
                in->pBuffer = ob.detach();

                safe_delete_array(__emu_buffer);
                dest->FastQueuePacket(&in, ack_req);
            }
        }
    }

	ENCODE(OP_ItemPreviewRequest)
	{
		EQApplicationPacket* in = *p;
		*p = nullptr;

		uchar* in_buf = in->pBuffer;

		auto int_item = (EQ::InternalSerializedItem_Struct*) in_buf;

		EQ::OutBuffer           buf;
		EQ::OutBuffer::pos_type last_pos = buf.tellp();

		SerializeItem(buf, (const EQ::ItemInstance*) int_item->inst, int_item->slot_id, 0, ItemPacketInvalid);
		if (buf.tellp() == last_pos) {
			LogNetcode("RoF2::ENCODE(OP_ItemPreviewRequest) Serialization failed");
			safe_delete_array(in_buf);
			safe_delete(in);
			return;
		}

		in->size    = buf.size();
		in->pBuffer = buf.detach();

		safe_delete_array(in_buf);
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_ItemVerifyReply)
	{
		ENCODE_LENGTH_EXACT(ItemVerifyReply_Struct);
		SETUP_DIRECT_ENCODE(ItemVerifyReply_Struct, structs::ItemVerifyReply_Struct);

		eq->inventory_slot = ServerToRoF2Slot(emu->slot);
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

		strncpy(eq->worldshortname, emu->worldshortname, sizeof(eq->worldshortname));

		//OUT(enablevoicemacros);	// These two are lost, but must be one of the 1s in unknown[249]
		//OUT(enablemail);
		OUT(enable_pvp);
		OUT(enable_FV);

		eq->unknown016 = 1;
		eq->unknown020[0] = 1;

		eq->unknown249[0] = 1;
		eq->unknown249[1] = 1;
		eq->unknown249[8] = 1;
		eq->unknown249[9] = 1;
		eq->unknown249[12] = 1;
		eq->unknown249[14] = 1;
		eq->unknown249[15] = 1;
		eq->unknown249[16] = 1;

		eq->unknown276[0] = 1.0f;
		eq->unknown276[1] = 1.0f;
		eq->unknown276[6] = 1.0f;

		FINISH_ENCODE();
	}

	ENCODE(OP_LootItem)
	{
		ENCODE_LENGTH_EXACT(LootingItem_Struct);
		SETUP_DIRECT_ENCODE(LootingItem_Struct, structs::LootingItem_Struct);

		Log(Logs::Detail, Logs::Netcode, "RoF2::ENCODE(OP_LootItem)");

		OUT(lootee);
		OUT(looter);
		eq->slot_id = ServerToRoF2CorpseMainSlot(emu->slot_id);
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
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].Status);
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
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// MercName
			for (k = 0; k < emu->Mercs[r].StanceCount; k++)
			{
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].Stances[k].StanceIndex);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->Mercs[r].Stances[k].Stance);
			}
		}

		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

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
				PacketSize += strlen(emu->MercData[r].MercName);	// Null Terminator size already accounted for in the struct
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
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].Status);
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
				//VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// MercName
				VARSTRUCT_ENCODE_STRING(Buffer, emu->MercData[r].MercName);
				for (k = 0; k < emu->MercData[r].StanceCount; k++)
				{
					VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].Stances[k].StanceIndex);
					VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->MercData[r].Stances[k].Stance);
				}
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 1);	// MercUnk05
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

		Log(Logs::Detail, Logs::Netcode, "RoF2::ENCODE(OP_MoveItem)");

		eq->from_slot = ServerToRoF2Slot(emu->from_slot);
		eq->to_slot = ServerToRoF2Slot(emu->to_slot);
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
		eq->ZoneTimeZone = 0;
		eq->unknown571 = 0;
		eq->WaterMidi = 4;
		eq->DayMidi = 2;
		eq->NightMidi = 0;

		eq->SkyRelated2 = -1;
		eq->NPCAggroMaxDist = 600;
		eq->FilterID = 2008; // Guild Lobby observed value
		OUT(lava_damage);
		OUT(min_lava_damage);
		eq->bDisallowManaStone     = 1;
		eq->bNoBind                = 0;
		eq->bNoAttack              = 0;
		eq->bNoCallOfHero          = 0;
		eq->bNoFlux                = 0;
		eq->bNoFear                = 0;
		eq->fall_damage            = 0;	// 0 = Fall Damage on, 1 = Fall Damage off
		eq->unknown895             = 0;
		eq->can_place_campsite     = 2;
		eq->can_place_guild_banner = 2;
		eq->fishing_related        = -1;	// Set from PoK Example
		eq->forage_related         = -1;	// Set from PoK Example
		eq->b_no_levitate          = 0;
		eq->blooming               = 1.0;	// Set from PoK Example

		FINISH_ENCODE();
	}

	ENCODE(OP_OnLevelMessage)
	{
		ENCODE_LENGTH_EXACT(OnLevelMessage_Struct);
		SETUP_DIRECT_ENCODE(OnLevelMessage_Struct, structs::OnLevelMessage_Struct);

		// This packet is variable sized now, but forcing it to the old packet size for now.
		eq->Title_Count = 128;
		memcpy(eq->Title, emu->Title, sizeof(eq->Title));
		eq->Text_Count = 4096;
		memcpy(eq->Text, emu->Text, sizeof(eq->Text));
		OUT(Buttons);
		OUT(SoundControls);
		OUT(Duration);
		OUT(PopupID);
		OUT(NegativeID);
		// These two field names are used if Buttons == 1. We should add an interface to them via Perl.
		eq->ButtonName0_Count = 25;
		OUT_str(ButtonName0);
		eq->ButtonName1_Count = 25;
		OUT_str(ButtonName1);

		FINISH_ENCODE();
	}

	ENCODE(OP_PetBuffWindow)
	{
		// The format of the RoF2 packet is identical to the OP_BuffCreate packet.

		SETUP_VAR_ENCODE(PetBuff_Struct);

		uint32 sz = 12 + (17 * emu->buffcount);
		__packet->size = sz;
		__packet->pBuffer = new unsigned char[sz];
		memset(__packet->pBuffer, 0, sz);

		__packet->WriteUInt32(emu->petid);
		__packet->WriteUInt32(0);		// PlayerID ?
		__packet->WriteUInt8(1);		// 1 indicates all buffs on the pet (0 to add or remove a single buff)
		__packet->WriteUInt16(emu->buffcount);

		for (uint16 i = 0; i < PET_BUFF_COUNT; ++i)
		{
			if (emu->spellid[i])
			{
				__packet->WriteUInt32(i);
				__packet->WriteUInt32(emu->spellid[i]);
				__packet->WriteUInt32(emu->ticsremaining[i]);
				__packet->WriteUInt32(0); // num hits
				__packet->WriteString("");
			}
		}
		__packet->WriteUInt8(0); // some sort of type

		FINISH_ENCODE();
	}

	ENCODE(OP_PlayerProfile)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		unsigned char *__emu_buffer = in->pBuffer;
		PlayerProfile_Struct *emu = (PlayerProfile_Struct *)__emu_buffer;

		uint32 PacketSize = 40000;	// Calculate this later

		auto outapp = new EQApplicationPacket(OP_PlayerProfile, PacketSize);

		outapp->WriteUInt32(0);		// Checksum, we will update this later
		outapp->WriteUInt32(0);		// Checksum size, we will update this later

		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(0);		// Unknown


		outapp->WriteUInt8(emu->gender);	// Gender
		outapp->WriteUInt32(emu->race);		// Race
		outapp->WriteUInt8(emu->class_);	// Class
		outapp->WriteUInt8(emu->level);		// Level
		outapp->WriteUInt8(emu->level);		// Level1


		outapp->WriteUInt32(5);			// Bind count

		for (int r = 0; r < 5; r++)
		{
			outapp->WriteUInt32(emu->binds[r].zone_id);
			outapp->WriteFloat(emu->binds[r].x);
			outapp->WriteFloat(emu->binds[r].y);
			outapp->WriteFloat(emu->binds[r].z);
			outapp->WriteFloat(emu->binds[r].heading);
		}

		outapp->WriteUInt32(emu->deity);
		outapp->WriteUInt32(emu->intoxication);

		outapp->WriteUInt32(10);		// Unknown count

		for (int r = 0; r < 10; r++)
		{
			outapp->WriteUInt32(0);		// Unknown
		}

		outapp->WriteUInt32(22);		// Equipment count

		for (int r = EQ::textures::textureBegin; r < EQ::textures::materialCount; r++)
		{
			outapp->WriteUInt32(emu->item_material.Slot[r].Material);
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
		}

		// Write zeroes for the next 13 equipment slots

		for (int r = 0; r < 13; r++)
		{
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
		}

		outapp->WriteUInt32(EQ::textures::materialCount);		// Equipment2 count

		for (int r = EQ::textures::textureBegin; r < EQ::textures::materialCount; r++)
		{
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
		}

		outapp->WriteUInt32(EQ::textures::materialCount);		// Tint Count

		for (int r = 0; r < 7; r++)
		{
			outapp->WriteUInt32(emu->item_tint.Slot[r].Color);
		}
		// Write zeroes for extra two tint values
		outapp->WriteUInt32(0);
		outapp->WriteUInt32(0);

		outapp->WriteUInt32(EQ::textures::materialCount);		// Tint2 Count

		for (int r = 0; r < 7; r++)
		{
			outapp->WriteUInt32(emu->item_tint.Slot[r].Color);
		}
		// Write zeroes for extra two tint values
		outapp->WriteUInt32(0);
		outapp->WriteUInt32(0);

		outapp->WriteUInt8(emu->haircolor);
		outapp->WriteUInt8(emu->beardcolor);
		outapp->WriteUInt32(0);			// Unknown
		outapp->WriteUInt8(emu->eyecolor1);
		outapp->WriteUInt8(emu->eyecolor2);
		outapp->WriteUInt8(emu->hairstyle);
		outapp->WriteUInt8(emu->beard);
		outapp->WriteUInt8(emu->face);

		// Think there should be an extra byte before the drakkin stuff (referred to as oldface in client)
		// Then one of the five bytes following the drakkin stuff needs removing.

		outapp->WriteUInt32(emu->drakkin_heritage);
		outapp->WriteUInt32(emu->drakkin_tattoo);
		outapp->WriteUInt32(emu->drakkin_details);

		outapp->WriteUInt8(0);			// Unknown 0
		outapp->WriteUInt8(0xff);		// Unknown 0xff
		outapp->WriteUInt8(1);			// Unknown 1
		outapp->WriteUInt8(0xff);		// Unknown 0xff
		outapp->WriteUInt8(1);			// Unknown 1

		outapp->WriteFloat(5.0f);		// Height

		outapp->WriteFloat(3.0f);		// Unknown 3.0
		outapp->WriteFloat(2.5f);		// Unknown 2.5
		outapp->WriteFloat(5.5f);		// Unknown 5.5

		outapp->WriteUInt32(0);			// Primary ?
		outapp->WriteUInt32(0);			// Secondary ?

		outapp->WriteUInt32(emu->points);	// Unspent skill points
		outapp->WriteUInt32(emu->mana);
		outapp->WriteUInt32(emu->cur_hp);

		outapp->WriteUInt32(emu->STR);
		outapp->WriteUInt32(emu->STA);
		outapp->WriteUInt32(emu->CHA);
		outapp->WriteUInt32(emu->DEX);
		outapp->WriteUInt32(emu->INT);
		outapp->WriteUInt32(emu->AGI);
		outapp->WriteUInt32(emu->WIS);

		outapp->WriteUInt32(0);			// Unknown
		outapp->WriteUInt32(0);			// Unknown
		outapp->WriteUInt32(0);			// Unknown
		outapp->WriteUInt32(0);			// Unknown
		outapp->WriteUInt32(0);			// Unknown
		outapp->WriteUInt32(0);			// Unknown
		outapp->WriteUInt32(0);			// Unknown

		outapp->WriteUInt32(300);		// AA Count

		for (uint32 r = 0; r < MAX_PP_AA_ARRAY; r++)
		{
			outapp->WriteUInt32(emu->aa_array[r].AA);
			outapp->WriteUInt32(emu->aa_array[r].value);
			outapp->WriteUInt32(emu->aa_array[r].charges);
		}

		// Fill the other 60 AAs with zeroes

		for (uint32 r = 0; r < structs::MAX_PP_AA_ARRAY - MAX_PP_AA_ARRAY; r++)
		{
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(0);
		}

		outapp->WriteUInt32(structs::MAX_PP_SKILL);

		for (uint32 r = 0; r < structs::MAX_PP_SKILL; r++)
		{
			outapp->WriteUInt32(emu->skills[r]);
		}

		outapp->WriteUInt32(structs::MAX_PP_INNATE_SKILL);			// Innate Skills count

		for (uint32 r = 0; r < structs::MAX_PP_INNATE_SKILL; r++)
		{
			outapp->WriteUInt32(emu->InnateSkills[r]);			// Innate Skills (regen, slam, etc)
		}

		outapp->WriteUInt32(structs::MAX_PP_DISCIPLINES);	// Discipline count

		for (uint32 r = 0; r < MAX_PP_DISCIPLINES; r++)
		{
			outapp->WriteUInt32(emu->disciplines.values[r]);
		}

		// Write zeroes for the rest of the disciplines
		for (uint32 r = 0; r < structs::MAX_PP_DISCIPLINES - MAX_PP_DISCIPLINES; r++)
		{
			outapp->WriteUInt32(0);
		}

		outapp->WriteUInt32(20);			// Timestamp count

		for (uint32 r = 0; r < 20; r++)
		{
			outapp->WriteUInt32(0);
		}

		outapp->WriteUInt32(MAX_RECAST_TYPES);			// Timestamp count

		for (uint32 r = 0; r < MAX_RECAST_TYPES; r++)
		{
			outapp->WriteUInt32(emu->recastTimers[r]);
		}

		outapp->WriteUInt32(100);			// Timestamp2 count

		for (uint32 r = 0; r < 100; r++)
		{
			outapp->WriteUInt32(0);
		}

		outapp->WriteUInt32(spells::SPELLBOOK_SIZE);		// Spellbook slots

		if (spells::SPELLBOOK_SIZE <= EQ::spells::SPELLBOOK_SIZE) {
			for (uint32 r = 0; r < spells::SPELLBOOK_SIZE; r++) {
				if (emu->spell_book[r] <= spells::SPELL_ID_MAX)
					outapp->WriteUInt32(emu->spell_book[r]);
				else
					outapp->WriteUInt32(0xFFFFFFFFU);
			}
		}
		else {
			for (uint32 r = 0; r < EQ::spells::SPELLBOOK_SIZE; r++) {
				if (emu->spell_book[r] <= spells::SPELL_ID_MAX)
					outapp->WriteUInt32(emu->spell_book[r]);
				else
					outapp->WriteUInt32(0xFFFFFFFFU);
			}
			// invalidate the rest of the spellbook slots
			for (uint32 r = EQ::spells::SPELLBOOK_SIZE; r < spells::SPELLBOOK_SIZE; r++) {
				outapp->WriteUInt32(0xFFFFFFFFU);
			}
		}

		outapp->WriteUInt32(spells::SPELL_GEM_COUNT);		// Memorised spell slots

		for (uint32 r = 0; r < EQ::spells::SPELL_GEM_COUNT; r++) // write first 12
		{
			outapp->WriteUInt32(emu->mem_spells[r]);
		}
		// zeroes for the rest of the slots the other 4, which actually don't work on the client at all :D
		for (uint32 r = 0; r < spells::SPELL_GEM_COUNT - EQ::spells::SPELL_GEM_COUNT; r++)
		{
			outapp->WriteUInt32(0xFFFFFFFFU);
		}

		outapp->WriteUInt32(13);			// gem refresh counts

		for (uint32 r = 0; r < EQ::spells::SPELL_GEM_COUNT; r++)
		{
			outapp->WriteUInt32(emu->spellSlotRefresh[r]);			// spell gem refresh
		}
		outapp->WriteUInt32(0);			// also refresh -- historically HT/LoH :P

		outapp->WriteUInt8(0);			// Unknown

		outapp->WriteUInt32(structs::BUFF_COUNT);

		for (uint32 r = 0; r < BUFF_COUNT; r++)
		{
			float instrument_mod = 0.0f;
			uint8 effect_type = emu->buffs[r].effect_type;
			uint32 player_id = emu->buffs[r].player_id;;

			if (emu->buffs[r].spellid != 0xFFFF && emu->buffs[r].spellid != 0)
			{
				instrument_mod = 1.0f + (emu->buffs[r].bard_modifier - 10) / 10.0f;
				effect_type = 2;
				player_id = 0x000717fd;
			}
			else
			{
				effect_type = 0;
			}

			// this is different than the client struct for some reason :P
			// missing a few things, shuffled around
			outapp->WriteUInt8(0);		// this is an unknown
			outapp->WriteFloat(instrument_mod);
			outapp->WriteUInt32(player_id);
			outapp->WriteUInt8(0);
			outapp->WriteUInt32(emu->buffs[r].counters);
			outapp->WriteUInt32(emu->buffs[r].duration);
			outapp->WriteUInt8(emu->buffs[r].level);
			outapp->WriteUInt32(emu->buffs[r].spellid);
			outapp->WriteUInt8(effect_type);			// Only ever seen 2
			outapp->WriteUInt32(emu->buffs[r].num_hits);
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(emu->buffs[r].counters);	// Appears twice ?

			for (uint32 j = 0; j < 44; ++j)
				outapp->WriteUInt8(0);	// Unknown
		}

		for (uint32 r = 0; r < structs::BUFF_COUNT - BUFF_COUNT; r++)
		{
			// 80 bytes of zeroes
			for (uint32 j = 0; j < 20; ++j)
				outapp->WriteUInt32(0);
		}

		outapp->WriteUInt32(emu->platinum);
		outapp->WriteUInt32(emu->gold);
		outapp->WriteUInt32(emu->silver);
		outapp->WriteUInt32(emu->copper);

		outapp->WriteUInt32(emu->platinum_cursor);
		outapp->WriteUInt32(emu->gold_cursor);
		outapp->WriteUInt32(emu->silver_cursor);
		outapp->WriteUInt32(emu->copper_cursor);

		outapp->WriteUInt32(0);		// Unknown

		outapp->WriteUInt32(0);		// This is the cooldown timer for the monk 'Mend' skill. Client will add 6 minutes to this value the first time the
		// player logs in. After that it will honour whatever value we send here.

		outapp->WriteUInt32(0);		// Unknown

		outapp->WriteUInt32(emu->thirst_level);
		outapp->WriteUInt32(emu->hunger_level);

		outapp->WriteUInt32(emu->aapoints_spent);

		outapp->WriteUInt32(5);				// AA Window Tab Count
		outapp->WriteUInt32(0);				// AA Points assigned ?
		outapp->WriteUInt32(0);				// AA Points in General ?
		outapp->WriteUInt32(0);				// AA Points in Class ?
		outapp->WriteUInt32(0);				// AA Points in Archetype ?
		outapp->WriteUInt32(0);				// AA Points in Special ?
		outapp->WriteUInt32(emu->aapoints);		// AA Points unspent

		outapp->WriteUInt8(0);				// Unknown
		outapp->WriteUInt8(0);				// Unknown

		outapp->WriteUInt32(profile::BANDOLIERS_SIZE);

		// Copy bandoliers where server and client indices converge
		for (uint32 r = 0; r < EQ::profile::BANDOLIERS_SIZE && r < profile::BANDOLIERS_SIZE; ++r) {
			outapp->WriteString(emu->bandoliers[r].Name);
			for (uint32 j = 0; j < profile::BANDOLIER_ITEM_COUNT; ++j) { // Will need adjusting if 'server != client' is ever true
				outapp->WriteString(emu->bandoliers[r].Items[j].Name);
				outapp->WriteUInt32(emu->bandoliers[r].Items[j].ID);
				if (emu->bandoliers[r].Items[j].Icon) {
					outapp->WriteSInt32(emu->bandoliers[r].Items[j].Icon);
 				}
				else {
 					// If no icon, it must send -1 or Treasure Chest Icon (836) is displayed
 					outapp->WriteSInt32(-1);
 				}
 			}
 		}
		// Nullify bandoliers where server and client indices diverge, with a client bias
		for (uint32 r = EQ::profile::BANDOLIERS_SIZE; r < profile::BANDOLIERS_SIZE; ++r) {
 			outapp->WriteString("");
			for (uint32 j = 0; j < profile::BANDOLIER_ITEM_COUNT; ++j) { // Will need adjusting if 'server != client' is ever true
 				outapp->WriteString("");
 				outapp->WriteUInt32(0);
 				outapp->WriteSInt32(-1);
 			}
 		}

		outapp->WriteUInt32(profile::POTION_BELT_SIZE);

		// Copy potion belt where server and client indices converge
		for (uint32 r = 0; r < EQ::profile::POTION_BELT_SIZE && r < profile::POTION_BELT_SIZE; ++r) {
			outapp->WriteString(emu->potionbelt.Items[r].Name);
			outapp->WriteUInt32(emu->potionbelt.Items[r].ID);
			if (emu->potionbelt.Items[r].Icon) {
				outapp->WriteSInt32(emu->potionbelt.Items[r].Icon);
 			}
			else {
				// If no icon, it must send -1 or Treasure Chest Icon (836) is displayed
 				outapp->WriteSInt32(-1);
 			}
 		}
		// Nullify potion belt where server and client indices diverge, with a client bias
		for (uint32 r = EQ::profile::POTION_BELT_SIZE; r < profile::POTION_BELT_SIZE; ++r) {
			outapp->WriteString("");
			outapp->WriteUInt32(0);
			outapp->WriteSInt32(-1);
		}

		outapp->WriteSInt32(-1);	// Unknown;
		outapp->WriteSInt32(123);	// HP Total ?
		outapp->WriteSInt32(234);	// Endurance Total ?
		outapp->WriteSInt32(345);	// Mana Total ?

		// these are needed to fix display bugs
		outapp->WriteUInt32(0x19);		// base CR
		outapp->WriteUInt32(0x19);		// base FR
		outapp->WriteUInt32(0x19);		// base MR
		outapp->WriteUInt32(0xf);		// base DR
		outapp->WriteUInt32(0xf);		// base PR
		outapp->WriteUInt32(0xf);		// base PhR?
		outapp->WriteUInt32(0xf);		// base Corrup
		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(0);		// Unknown

		outapp->WriteUInt32(20);	// Unknown - Expansion count ?

		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(0);		// Unknown
		outapp->WriteUInt32(emu->endurance);
		outapp->WriteUInt32(0);		// Unknown - Observed 0x7cde - This is also seen in guild packets sent to this character.
		outapp->WriteUInt32(0);		// Unknown - Observed 0x64

		outapp->WriteUInt32(64);	// Name Length

		uint32 CurrentPosition = outapp->GetWritePosition();

		outapp->WriteString(emu->name);

		outapp->SetWritePosition(CurrentPosition + 64);

		outapp->WriteUInt32(32);	// Last Name Length

		CurrentPosition = outapp->GetWritePosition();

		outapp->WriteString(emu->last_name);

		outapp->SetWritePosition(CurrentPosition + 32);

		outapp->WriteUInt32(emu->birthday);
		outapp->WriteUInt32(emu->birthday);		// Account start date ?
		outapp->WriteUInt32(emu->lastlogin);
		outapp->WriteUInt32(emu->timePlayedMin);
		outapp->WriteUInt32(emu->timeentitledonaccount);
		outapp->WriteUInt32(emu->expansions);
		//outapp->WriteUInt32(0x0007ffff);		// Expansion bitmask

		outapp->WriteUInt32(structs::MAX_PP_LANGUAGE);

		for (uint32 r = 0; r < MAX_PP_LANGUAGE; r++)
		{
			outapp->WriteUInt8(emu->languages[r]);
		}

		for (uint32 r = 0; r < structs::MAX_PP_LANGUAGE - MAX_PP_LANGUAGE; r++)
		{
			outapp->WriteUInt8(0);
		}

		outapp->WriteUInt16(emu->zone_id);
		outapp->WriteUInt16(emu->zoneInstance);

		outapp->WriteFloat(emu->y);
		outapp->WriteFloat(emu->x);
		outapp->WriteFloat(emu->z);
		outapp->WriteFloat(emu->heading);

		outapp->WriteUInt8(0);				// Unknown
		outapp->WriteUInt8(emu->pvp);
		outapp->WriteUInt8(0);				// Unknown
		outapp->WriteUInt8(emu->gm);
		outapp->WriteUInt32(emu->guild_id);

		outapp->WriteUInt8(emu->guildrank);	// guildrank
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt8(0);			// Unknown
		outapp->WriteUInt32(0);				// Unknown

		outapp->WriteUInt64(emu->exp);		// int32 in client

		outapp->WriteUInt8(5);			// Unknown - Seen 5 on Live - Eye Height?

		outapp->WriteUInt32(emu->platinum_bank);
		outapp->WriteUInt32(emu->gold_bank);
		outapp->WriteUInt32(emu->silver_bank);
		outapp->WriteUInt32(emu->copper_bank);

		outapp->WriteUInt32(emu->platinum_shared);
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown

		outapp->WriteUInt32(0);				// Unknown

		outapp->WriteSInt32(-1);				// Unknown
		outapp->WriteSInt32(-1);				// Unknown

		outapp->WriteUInt32(emu->career_tribute_points);
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(emu->tribute_points);
		outapp->WriteUInt32(0);				// Unknown

		outapp->WriteUInt8(0);				// Unknown
		outapp->WriteUInt8(0);				// Unknown

		outapp->WriteUInt32(EQ::invtype::TRIBUTE_SIZE);

		for (uint32 r = 0; r < EQ::invtype::TRIBUTE_SIZE; r++)
		{
			outapp->WriteUInt32(emu->tributes[r].tribute);
			outapp->WriteUInt32(emu->tributes[r].tier);
		}

		outapp->WriteUInt32(10);		// Guild Tribute Count ?

		for (uint32 r = 0; r < 10; r++)
		{
			outapp->WriteUInt32(0xffffffff);
			outapp->WriteUInt32(0);
//			outapp->WriteUInt32(60);
//			outapp->WriteUInt32(1);
		}

		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown

		for (uint32 r = 0; r < 125; r++)
		{
			outapp->WriteUInt8(0);				// Unknown
		}

		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(emu->currentRadCrystals);
		outapp->WriteUInt32(emu->careerRadCrystals);
		outapp->WriteUInt32(emu->currentEbonCrystals);
		outapp->WriteUInt32(emu->careerEbonCrystals);
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown

		// Unknown String ?
		outapp->WriteUInt32(64);			// Unknown
		for (uint32 r = 0; r < 64; r++)
		{
			outapp->WriteUInt8(0);				// Unknown
		}

		outapp->WriteUInt8(0);				// Unknown
		outapp->WriteUInt8(0);				// Unknown
		outapp->WriteUInt8(0);				// Unknown

		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown

		outapp->WriteUInt8(0);				// Unknown
		outapp->WriteUInt8(0);				// Unknown
		outapp->WriteUInt8(0);				// Unknown

		outapp->WriteUInt32(0);				// Unknown

		outapp->WriteUInt8(0);				// Unknown
		outapp->WriteUInt8(0);				// Unknown
		outapp->WriteUInt8(0);				// Unknown

		outapp->WriteUInt32(0);				// Unknown

		outapp->WriteUInt8(0);				// Unknown

		outapp->WriteUInt32(0);				// Unknown

		// Unknown String ?
		outapp->WriteUInt32(64);			// Unknown
		for (uint32 r = 0; r < 64; r++)
		{
			outapp->WriteUInt8(0);				// Unknown
		}

		// Unknown String ?
		outapp->WriteUInt32(64);			// Unknown
		for (uint32 r = 0; r < 64; r++)
		{
			outapp->WriteUInt8(0);				// Unknown
		}

		outapp->WriteUInt32(0);				// Unknown

		// Block of 320 unknown bytes
		for (uint32 r = 0; r < 320; r++)
		{
			outapp->WriteUInt8(0);				// Unknown
		}

		// Block of 343 unknown bytes
		for (uint32 r = 0; r < 343; r++)
		{
			outapp->WriteUInt8(0);				// Unknown
		}

		outapp->WriteUInt32(emu->char_id);		// character_id

		outapp->WriteUInt8(emu->leadAAActive);

		outapp->WriteUInt32(6);				// Count ... of LDoN stats ?
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(emu->ldon_points_guk);
		outapp->WriteUInt32(emu->ldon_points_mir);
		outapp->WriteUInt32(emu->ldon_points_mmc);
		outapp->WriteUInt32(emu->ldon_points_ruj);
		outapp->WriteUInt32(emu->ldon_points_tak);

		outapp->WriteUInt32(emu->ldon_points_available);

		outapp->WriteDouble(emu->group_leadership_exp);
		outapp->WriteDouble(emu->raid_leadership_exp);

		outapp->WriteUInt32(emu->group_leadership_points);
		outapp->WriteUInt32(emu->raid_leadership_points);

		outapp->WriteUInt32(64);			// Group of 64 int32s follow	Group/Raid Leadership abilities ?

		for (uint32 r = 0; r < MAX_LEADERSHIP_AA_ARRAY; r++)
		{
			outapp->WriteUInt32(emu->leader_abilities.ranks[r]);
		}

		for (uint32 r = 0; r < 64 - MAX_LEADERSHIP_AA_ARRAY; r++)
		{
			outapp->WriteUInt32(0);				// Unused/unsupported Leadership abilities
		}

		outapp->WriteUInt32(emu->air_remaining);		// ?

		// PVP Stats

		outapp->WriteUInt32(emu->PVPKills);
		outapp->WriteUInt32(emu->PVPDeaths);
		outapp->WriteUInt32(emu->PVPCurrentPoints);
		outapp->WriteUInt32(emu->PVPCareerPoints);
		outapp->WriteUInt32(emu->PVPBestKillStreak);
		outapp->WriteUInt32(emu->PVPWorstDeathStreak);
		outapp->WriteUInt32(emu->PVPCurrentKillStreak);

		// Last PVP Kill

		outapp->WriteString(emu->PVPLastKill.Name);
		outapp->WriteUInt32(emu->PVPLastKill.Level);
		outapp->WriteUInt32(emu->PVPLastKill.Race);
		outapp->WriteUInt32(emu->PVPLastKill.Class);
		outapp->WriteUInt32(emu->PVPLastKill.Zone);
		outapp->WriteUInt32(emu->PVPLastKill.Time);
		outapp->WriteUInt32(emu->PVPLastKill.Points);

		// Last PVP Death

		outapp->WriteString(emu->PVPLastDeath.Name);
		outapp->WriteUInt32(emu->PVPLastDeath.Level);
		outapp->WriteUInt32(emu->PVPLastDeath.Race);
		outapp->WriteUInt32(emu->PVPLastDeath.Class);
		outapp->WriteUInt32(emu->PVPLastDeath.Zone);
		outapp->WriteUInt32(emu->PVPLastDeath.Time);
		outapp->WriteUInt32(emu->PVPLastDeath.Points);

		outapp->WriteUInt32(emu->PVPNumberOfKillsInLast24Hours);

		// Last 50 Kills
		outapp->WriteUInt32(50);
		for (uint32 r = 0; r < 50; ++r)
		{
			outapp->WriteString(emu->PVPRecentKills[r].Name);
			outapp->WriteUInt32(emu->PVPRecentKills[r].Level);
			outapp->WriteUInt32(emu->PVPRecentKills[r].Race);
			outapp->WriteUInt32(emu->PVPRecentKills[r].Class);
			outapp->WriteUInt32(emu->PVPRecentKills[r].Zone);
			outapp->WriteUInt32(emu->PVPRecentKills[r].Time);
			outapp->WriteUInt32(emu->PVPRecentKills[r].Points);
		}

		outapp->WriteUInt32(emu->expAA);
		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown

		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Krono - itemid 88888 Hard coded in client?

		outapp->WriteUInt8(emu->groupAutoconsent);
		outapp->WriteUInt8(emu->raidAutoconsent);
		outapp->WriteUInt8(emu->guildAutoconsent);
		outapp->WriteUInt8(0);				// Unknown

		outapp->WriteUInt32(emu->level);	// Level3 ?

		outapp->WriteUInt8(emu->showhelm);

		outapp->WriteUInt32(emu->RestTimer);

		outapp->WriteUInt32(1024);			// Unknown Count
		// Block of 1024 unknown bytes
		for (uint32 r = 0; r < 1024; r++)
		{
			outapp->WriteUInt8(0);				// Unknown
		}

		outapp->WriteUInt32(0);				// Unknown
		outapp->WriteUInt32(0);				// Unknown

		// Think we need 1 byte of padding at the end
		outapp->WriteUInt8(0);				// Unknown

		LogNetcode("[STRUCTS] Player Profile Packet is [{}] bytes", outapp->GetWritePosition());

		auto NewBuffer = new unsigned char[outapp->GetWritePosition()];
		memcpy(NewBuffer, outapp->pBuffer, outapp->GetWritePosition());
		safe_delete_array(outapp->pBuffer);
		outapp->pBuffer = NewBuffer;
		outapp->size = outapp->GetWritePosition();
		outapp->SetWritePosition(4);
		outapp->WriteUInt32(outapp->size - 9);

		CRC32::SetEQChecksum(outapp->pBuffer, outapp->size - 1, 8);
		//Log.Hex(Logs::Netcode, outapp->pBuffer, outapp->size);

		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
		return;
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
		eq->invslot = ServerToRoF2TypelessSlot(emu->invslot, invtype::typePossessions);
		OUT(target_id);
		OUT(can_cast);
		OUT(can_scribe);
		strn0cpy(eq->txtfile, emu->booktext, sizeof(eq->txtfile));

		FINISH_ENCODE();
	}

	ENCODE(OP_RecipeAutoCombine)
	{
		ENCODE_LENGTH_EXACT(RecipeAutoCombine_Struct);
		SETUP_DIRECT_ENCODE(RecipeAutoCombine_Struct, structs::RecipeAutoCombine_Struct);

		OUT(object_type);
		OUT(some_id);
		eq->container_slot = ServerToRoF2Slot(emu->unknown1);
		structs::InventorySlot_Struct RoF2Slot;
		RoF2Slot.Type = 8;	// Observed
		RoF2Slot.Unknown02 = 0;
		RoF2Slot.Slot = 0xffff;
		RoF2Slot.SubIndex = 0xffff;
		RoF2Slot.AugIndex = 0xffff;
		RoF2Slot.Unknown01 = 0;
		eq->unknown_slot = RoF2Slot;
		OUT(recipe_id);
		OUT(reply_code);

		FINISH_ENCODE();
	}

	ENCODE(OP_RemoveBlockedBuffs) { ENCODE_FORWARD(OP_BlockedBuffs); }

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

	ENCODE(OP_RespondAA)
	{
		SETUP_DIRECT_ENCODE(AATable_Struct, structs::AATable_Struct);

		eq->aa_spent = emu->aa_spent;
		// These fields may need to be correctly populated at some point
		eq->aapoints_assigned = emu->aa_spent;
		eq->aa_spent_general = 0;
		eq->aa_spent_archetype = 0;
		eq->aa_spent_class = 0;
		eq->aa_spent_special = 0;

		for (uint32 i = 0; i < MAX_PP_AA_ARRAY; ++i)
		{
			eq->aa_list[i].AA = emu->aa_list[i].AA;
			eq->aa_list[i].value = emu->aa_list[i].value;
			eq->aa_list[i].charges = emu->aa_list[i].charges;
		}

		FINISH_ENCODE();
	}

	ENCODE(OP_RezzRequest)
	{
		SETUP_DIRECT_ENCODE(Resurrect_Struct, structs::Resurrect_Struct);

		OUT(zone_id);
		OUT(instance_id);
		OUT(y);
		OUT(x);
		OUT(z);
		OUT_str(your_name);
		OUT_str(rezzer_name);
		OUT(spellid);
		OUT_str(corpse_name);
		OUT(action);

		FINISH_ENCODE();
	}

	ENCODE(OP_SendAATable)
	{
		EQApplicationPacket *inapp = *p;
		*p = nullptr;
		AARankInfo_Struct *emu = (AARankInfo_Struct*)inapp->pBuffer;

		// the structs::SendAA_Struct includes enough space for 1 prereq which is the min even if it has no prereqs
		auto prereq_size = emu->total_prereqs > 1 ? (emu->total_prereqs - 1) * 8 : 0;
		auto outapp = new EQApplicationPacket(OP_SendAATable, sizeof(structs::SendAA_Struct) + emu->total_effects * sizeof(structs::AA_Ability) + prereq_size);
		inapp->SetReadPosition(sizeof(AARankInfo_Struct)+emu->total_effects * sizeof(AARankEffect_Struct));


		std::vector<int32> skill;
		std::vector<int32> points;
		for(auto i = 0; i < emu->total_prereqs; ++i) {
			skill.push_back(inapp->ReadUInt32());
			points.push_back(inapp->ReadUInt32());
		}

		outapp->WriteUInt32(emu->id);
		outapp->WriteUInt8(1);
		outapp->WriteSInt32(emu->upper_hotkey_sid);
		outapp->WriteSInt32(emu->lower_hotkey_sid);
		outapp->WriteSInt32(emu->title_sid);
		outapp->WriteSInt32(emu->desc_sid);
		outapp->WriteSInt32(emu->level_req);
		outapp->WriteSInt32(emu->cost);
		outapp->WriteUInt32(emu->seq);
		outapp->WriteUInt32(emu->current_level);

		if (emu->total_prereqs) {
			outapp->WriteUInt32(emu->total_prereqs);
			for (auto &e : skill)
				outapp->WriteSInt32(e);
			outapp->WriteUInt32(emu->total_prereqs);
			for (auto &e : points)
				outapp->WriteSInt32(e);
		} else {
			outapp->WriteUInt32(1);
			outapp->WriteUInt32(0);
			outapp->WriteUInt32(1);
			outapp->WriteUInt32(0);
		}

		outapp->WriteSInt32(emu->type);
		outapp->WriteSInt32(emu->spell);
		outapp->WriteSInt32(1);
		outapp->WriteSInt32(emu->spell_type);
		outapp->WriteSInt32(emu->spell_refresh);
		outapp->WriteSInt32(emu->classes);
		outapp->WriteSInt32(emu->max_level);
		outapp->WriteSInt32(emu->prev_id);
		outapp->WriteSInt32(emu->next_id);
		outapp->WriteSInt32(emu->total_cost);
		outapp->WriteUInt8(0);
		outapp->WriteUInt8(emu->grant_only);
		outapp->WriteUInt8(0);
		outapp->WriteUInt32(emu->charges);
		outapp->WriteSInt32(emu->expansion);
		outapp->WriteSInt32(emu->category);
		outapp->WriteUInt8(0); // shroud
		outapp->WriteUInt8(0); // unknown109
		outapp->WriteUInt8(0); // loh
		outapp->WriteUInt8(0); // unknown111
		outapp->WriteUInt32(emu->total_effects);

		inapp->SetReadPosition(sizeof(AARankInfo_Struct));
		for(auto i = 0; i < emu->total_effects; ++i) {
			outapp->WriteUInt32(inapp->ReadUInt32()); // skill_id
			outapp->WriteUInt32(inapp->ReadUInt32()); // base1
			outapp->WriteUInt32(inapp->ReadUInt32()); // base2
			outapp->WriteUInt32(inapp->ReadUInt32()); // slot
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
		//eq->TotalChars = emu->TotalChars;

		//if (eq->TotalChars > consts::CHARACTER_CREATION_LIMIT)
		//	eq->TotalChars = consts::CHARACTER_CREATION_LIMIT;

		emu_ptr = __emu_buffer;
		emu_ptr += sizeof(CharacterSelect_Struct);

		unsigned char *eq_ptr = __packet->pBuffer;
		eq_ptr += sizeof(structs::CharacterSelect_Struct);

		for (int counter = 0; counter < character_count; ++counter) {
			emu_cse = (CharacterSelectEntry_Struct *)emu_ptr;
			eq_cse = (structs::CharacterSelectEntry_Struct *)eq_ptr; // base address

			strcpy(eq_cse->Name, emu_cse->Name);
			eq_ptr += strlen(emu_cse->Name);
			eq_cse = (structs::CharacterSelectEntry_Struct *)eq_ptr; // offset address (base + name length offset)
			eq_cse->Name[0] = '\0'; // (offset)eq_cse->Name[0] = (base)eq_cse->Name[strlen(emu_cse->Name)]

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
				eq_cse->Equip[equip_index].Material = emu_cse->Equip[equip_index].Material;
				eq_cse->Equip[equip_index].Unknown1 = emu_cse->Equip[equip_index].Unknown1;
				eq_cse->Equip[equip_index].EliteMaterial = emu_cse->Equip[equip_index].EliteModel;
				eq_cse->Equip[equip_index].HeroForgeModel = emu_cse->Equip[equip_index].HerosForgeModel;
				eq_cse->Equip[equip_index].Material2 = emu_cse->Equip[equip_index].Unknown2;
				eq_cse->Equip[equip_index].Color = emu_cse->Equip[equip_index].Color;
			}

			eq_cse->Unknown15 = emu_cse->Unknown15;
			eq_cse->Unknown19 = emu_cse->Unknown19;
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
			eq_cse->GoHome = emu_cse->GoHome;
			eq_cse->Tutorial = emu_cse->Tutorial;
			eq_cse->DrakkinHeritage = emu_cse->DrakkinHeritage;
			eq_cse->Unknown1 = emu_cse->Unknown1;
			eq_cse->Enabled = emu_cse->Enabled;
			eq_cse->LastLogin = emu_cse->LastLogin;
			eq_cse->Unknown2 = emu_cse->Unknown2;

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

	ENCODE(OP_SetGuildRank)
	{
		ENCODE_LENGTH_EXACT(GuildSetRank_Struct);
		SETUP_DIRECT_ENCODE(GuildSetRank_Struct, structs::GuildSetRank_Struct);

		eq->guild_id= emu->Unknown00;
		eq->rank = emu->rank;

		memcpy(eq->member_name, emu->member_name, sizeof(eq->member_name));
		OUT(banker);
		eq->unknown76 = 1;

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
		eq->inventory_slot = ServerToRoF2TypelessSlot(emu->itemslot, EQ::invtype::typePossessions);
		//OUT(itemslot);
		OUT(quantity);
		OUT(price);

		FINISH_ENCODE();
	}

	ENCODE(OP_SkillUpdate)
	{
		ENCODE_LENGTH_EXACT(SkillUpdate_Struct);
		SETUP_DIRECT_ENCODE(SkillUpdate_Struct, structs::SkillUpdate_Struct);

		OUT(skillId);
		OUT(value);
		eq->unknown08 = 1;		// Observed
		eq->unknown09 = 80;		// Observed
		eq->unknown10 = 136;	// Observed
		eq->unknown11 = 54;		// Observed

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

		strncpy(eq->model_name, emu->model_name, sizeof(eq->model_name));

		FINISH_ENCODE();
	}

	ENCODE(OP_SpawnAppearance)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		unsigned char *emu_buffer = in->pBuffer;

		SpawnAppearance_Struct *sas = (SpawnAppearance_Struct *)emu_buffer;

		if (sas->type != AppearanceType::Size)
		{
			dest->FastQueuePacket(&in, ack_req);
			return;
		}

		auto outapp = new EQApplicationPacket(OP_ChangeSize, sizeof(ChangeSize_Struct));

		ChangeSize_Struct *css = (ChangeSize_Struct *)outapp->pBuffer;

		css->EntityID = sas->spawn_id;
		css->Size = (float)sas->parameter;
		css->Unknown08 = 0;
		css->Unknown12 = 1.0f;

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
			strncpy(eq[r].name, emu[r].name, sizeof(eq[r].name));
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
			eq[r].unknown0080 = 0;
			eq[r].unknown0081 = 1; // Both must be 1 to allow clicking doors
			eq[r].unknown0082 = 0;
			eq[r].unknown0083 = 1; // Both must be 1 to allow clicking doors
			eq[r].unknown0084 = 0;
			eq[r].unknown0085 = 0;
			eq[r].unknown0086 = 0;
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

		ServerToRoF2SayLink(new_message, old_message);

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

		TaskDescriptionData1_Struct *emu_tdd1 = (TaskDescriptionData1_Struct *)InBuffer;
		emu_tdd1->StartTime = (time(nullptr) - emu_tdd1->StartTime); // RoF2 has elapsed time here rather than start time

		InBuffer += sizeof(TaskDescriptionData1_Struct);
		uint32 description_size = strlen(InBuffer) + 1;
		InBuffer += description_size;
		InBuffer += sizeof(TaskDescriptionData2_Struct);

		uint32 reward_size = strlen(InBuffer) + 1;
		InBuffer += reward_size;

		std::string old_message = InBuffer; // start item link string
		std::string new_message;
		ServerToRoF2SayLink(new_message, old_message);

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

#if 0 // original code
		EQApplicationPacket *in = *p;
		*p = nullptr;

		EQApplicationPacket *outapp = new EQApplicationPacket(OP_TaskDescription, in->size + 1);
		// Set the Write pointer as we don't know what has been done with the packet before we get it.
		in->SetReadPosition(0);
		// Copy the header
		for (int i = 0; i < 5; ++i)
			outapp->WriteUInt32(in->ReadUInt32());

		// Copy Title
		while (uint8 c = in->ReadUInt8())
			outapp->WriteUInt8(c);
		outapp->WriteUInt8(0);

		outapp->WriteUInt32(in->ReadUInt32());	// Duration
		outapp->WriteUInt32(in->ReadUInt32());	// Unknown
		uint32 StartTime = in->ReadUInt32();
		outapp->WriteUInt32(time(nullptr) - StartTime);	// RoF2 has elapsed time here rather than starttime

		// Copy the rest of the packet verbatim
		uint32 BytesLeftToCopy = in->size - in->GetReadPosition();
		memcpy(outapp->pBuffer + outapp->GetWritePosition(), in->pBuffer + in->GetReadPosition(), BytesLeftToCopy);

		delete in;
		dest->FastQueuePacket(&outapp, ack_req);
#endif
	}

	ENCODE(OP_TaskHistoryReply)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		// First we need to calculate the length of the new packet
		in->SetReadPosition(4);
		uint32 ActivityCount = in->ReadUInt32();

		uint32 Text1Length = 0;
		uint32 Text2Length = 0;
		uint32 Text3Length = 0;

		uint32 OutboundPacketSize = 8;

		for (uint32 i = 0; i < ActivityCount; ++i)
		{
			Text1Length = 0;
			Text2Length = 0;
			Text3Length = 0;

			in->ReadUInt32(); // Activity type

			// Skip past Text1
			while (in->ReadUInt8())
				++Text1Length;

			// Skip past Text2
			while (in->ReadUInt8())
				++Text2Length;

			in->ReadUInt32();
			in->ReadUInt32();
			in->ReadUInt32();
			uint32 ZoneID = in->ReadUInt32();
			in->ReadUInt32();

			// Skip past Text3
			while (in->ReadUInt8())
				++Text3Length;

			char ZoneNumber[10];

			sprintf(ZoneNumber, "%i", ZoneID);

			OutboundPacketSize += (24 + Text1Length + 1 + Text2Length + Text3Length + 1 + 7 + (strlen(ZoneNumber) * 2));
		}

		in->SetReadPosition(0);

		auto outapp = new EQApplicationPacket(OP_TaskHistoryReply, OutboundPacketSize);

		outapp->WriteUInt32(in->ReadUInt32());	// Task index
		outapp->WriteUInt32(in->ReadUInt32());	// Activity count

		for (uint32 i = 0; i < ActivityCount; ++i)
		{
			Text1Length = 0;
			Text2Length = 0;
			Text3Length = 0;

			outapp->WriteUInt32(in->ReadUInt32()); // ActivityType

			// Copy Text1
			while (uint8 c = in->ReadUInt8())
				outapp->WriteUInt8(c);

			outapp->WriteUInt8(0);	// Text1 has a null terminator

			uint32 CurrentPosition = in->GetReadPosition();

			// Determine Length of Text2
			while (in->ReadUInt8())
				++Text2Length;

			outapp->WriteUInt32(Text2Length);

			in->SetReadPosition(CurrentPosition);

			// Copy Text2
			while (uint8 c = in->ReadUInt8())
				outapp->WriteUInt8(c);

			outapp->WriteUInt32(in->ReadUInt32()); // Goalcount
			in->ReadUInt32();
			in->ReadUInt32();
			uint32 ZoneID = in->ReadUInt32();
			in->ReadUInt32();

			char ZoneNumber[10];

			sprintf(ZoneNumber, "%i", ZoneID);

			outapp->WriteUInt32(2);
			outapp->WriteUInt8(0x2d); // "-"
			outapp->WriteUInt8(0x31); // "1"

			outapp->WriteUInt32(2);
			outapp->WriteUInt8(0x2d); // "-"
			outapp->WriteUInt8(0x31); // "1"
			outapp->WriteString(ZoneNumber);

			outapp->WriteUInt32(0);

			// Copy Tex3t
			while (uint8 c = in->ReadUInt8())
				outapp->WriteUInt8(c);

			outapp->WriteUInt8(0);	// Text3 has a null terminator

			outapp->WriteUInt8(0x31); // "1"
			outapp->WriteString(ZoneNumber);
		}

		delete in;
		dest->FastQueuePacket(&outapp, ack_req);
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
			PacketSize += (13 + strlen(emu->name));

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
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->is_pet);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->is_merc);
		}

		delete[] __emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_Trader)
	{
		uint32 action = *(uint32 *) (*p)->pBuffer;

		switch (action) {
			case TraderOn: {
				ENCODE_LENGTH_EXACT(Trader_ShowItems_Struct);
				SETUP_DIRECT_ENCODE(Trader_ShowItems_Struct, structs::Trader_ShowItems_Struct);

				eq->action = structs::RoF2BazaarTraderBuyerActions::BeginTraderMode;
				OUT(entity_id);

				LogTrading("(RoF2) TraderOn action <green>[{}] entity_id <green>[{}]", action, eq->entity_id);
				FINISH_ENCODE();
				break;
			}
			case TraderOff: {
				ENCODE_LENGTH_EXACT(Trader_ShowItems_Struct);
				SETUP_DIRECT_ENCODE(Trader_ShowItems_Struct, structs::Trader_ShowItems_Struct);

				eq->action = structs::RoF2BazaarTraderBuyerActions::EndTraderMode;
				OUT(entity_id);

				LogTrading("(RoF2) TraderOff action <green>[{}] entity_id <green>[{}]", action, eq->entity_id);
				FINISH_ENCODE();
				break;
			}
			case ListTraderItems: {
				ENCODE_LENGTH_EXACT(Trader_Struct);
				SETUP_DIRECT_ENCODE(Trader_Struct, structs::ClickTrader_Struct);
				LogTrading("(RoF2)  action <green>[{}]", action);

				eq->action = structs::RoF2BazaarTraderBuyerActions::ListTraderItems;
				std::transform(
					std::begin(emu->items),
					std::end(emu->items),
					std::begin(eq->items),
					[&](const uint32 x) {
						return x;
					}
				);
				std::copy_n(
					std::begin(emu->item_cost),
					EQ::invtype::BAZAAR_SIZE,
					std::begin(eq->item_cost)
				);

				FINISH_ENCODE();
				break;
			}
			case TraderAck2: {
				LogTrading("(RoF2) TraderAck2 action");
				EQApplicationPacket *in = *p;
				*p = nullptr;

				dest->FastQueuePacket(&in);
				break;
			}
			case PriceUpdate: {
				SETUP_DIRECT_ENCODE(TraderPriceUpdate_Struct, structs::TraderPriceUpdate_Struct);
				switch (emu->SubAction) {
					case BazaarPriceChange_AddItem: {
						auto outapp = std::make_unique<EQApplicationPacket>(
							OP_Trader,
							sizeof(structs::TraderStatus_Struct)
						);

						auto data        = (structs::TraderStatus_Struct *) outapp->pBuffer;
						data->action     = emu->Action;
						data->sub_action = BazaarPriceChange_AddItem;
						LogTrading(
							"(RoF2) PriceUpdate action <green>[{}] AddItem subaction <yellow>[{}]",
							data->action,
							data->sub_action
						);

						dest->QueuePacket(outapp.get());
						break;
					}
					case BazaarPriceChange_RemoveItem: {
						auto outapp = std::make_unique<EQApplicationPacket>(
							OP_Trader,
							sizeof(structs::TraderStatus_Struct)
						);

						auto data        = (structs::TraderStatus_Struct *) outapp->pBuffer;
						data->action     = emu->Action;
						data->sub_action = BazaarPriceChange_RemoveItem;
						LogTrading(
							"(RoF2) PriceUpdate action <green>[{}] RemoveItem subaction <yellow>[{}]",
							data->action,
							data->sub_action
						);

						dest->QueuePacket(outapp.get());
						break;
					}
					case BazaarPriceChange_UpdatePrice: {
						auto outapp = std::make_unique<EQApplicationPacket>(
							OP_Trader,
							sizeof(structs::TraderStatus_Struct)
						);

						auto data        = (structs::TraderStatus_Struct *) outapp->pBuffer;
						data->action     = emu->Action;
						data->sub_action = BazaarPriceChange_UpdatePrice;
						LogTrading(
							"(RoF2) PriceUpdate action <green>[{}] UpdatePrice subaction <yellow>[{}]",
							data->action,
							data->sub_action
						);

						dest->QueuePacket(outapp.get());
						break;
					}
				}

				FINISH_ENCODE();
				break;
			}
			case BuyTraderItem: {
				EQApplicationPacket *in = *p;
				*p = nullptr;

				auto eq = (structs::TraderBuy_Struct *) in->pBuffer;
				LogTrading(
					"(RoF2) BuyTraderItem action <green>[{}] item_id <green>[{}] item_sn <green>[{}] buyer <green>[{}]",
					action,
					eq->item_id,
					eq->serial_number,
					eq->buyer_name
				);
				dest->FastQueuePacket(&in);
				break;
			}
			default: {
				LogTrading("(RoF2) action <red>[{}]", action);
				EQApplicationPacket *in = *p;
				*p = nullptr;

				dest->FastQueuePacket(&in);
			}
		}
	}

	ENCODE(OP_TraderBuy)
	{
		ENCODE_LENGTH_EXACT(TraderBuy_Struct);
		SETUP_DIRECT_ENCODE(TraderBuy_Struct, structs::TraderBuy_Struct);
		LogTrading(
			"(RoF2) item_id <green>[{}] price <green>[{}] quantity <green>[{}] trader_id <green>[{}]",
			emu->item_id,
			emu->price,
			emu->quantity,
			emu->trader_id
		);
		__packet->SetOpcode(OP_TraderShop);
		OUT(action);
		OUT(method);
		OUT(sub_action);
		OUT(trader_id);
		OUT(item_id);
		OUT(price);
		OUT(already_sold);
		OUT(quantity);
		OUT_str(buyer_name);
		OUT_str(seller_name);
		OUT_str(item_name);
		OUT_str(serial_number);

		FINISH_ENCODE();
	}

	ENCODE(OP_TraderDelItem)
	{
		ENCODE_LENGTH_EXACT(TraderDelItem_Struct);
		SETUP_DIRECT_ENCODE(TraderDelItem_Struct, structs::TraderDelItem_Struct);
		LogTrading(
			"(RoF2) trader_id <green>[{}] item_id <green>[{}]",
			emu->trader_id,
			emu->item_id
		);

		eq->TraderID = emu->trader_id;
		auto serial  = fmt::format("{:016}\n", emu->item_id);
		strn0cpy(eq->SerialNumber, serial.c_str(), sizeof(eq->SerialNumber));
		LogTrading("(RoF2) TraderID <green>[{}], SerialNumber: <green>[{}]", emu->trader_id, emu->item_id);

		FINISH_ENCODE();
	}

	ENCODE(OP_TraderShop)
	{
		auto action = *(uint32 *) (*p)->pBuffer;

		switch (action) {
			case ClickTrader: {
				ENCODE_LENGTH_EXACT(TraderClick_Struct);
				SETUP_DIRECT_ENCODE(TraderClick_Struct, structs::TraderClick_Struct);
				LogTrading(
					"(RoF2) ClickTrader action <green>[{}] trader_id <green>[{}]",
					action,
					emu->TraderID
				);

				eq->action      = structs::RoF2BazaarTraderBuyerActions::ClickTrader; // Seen on Live
				eq->trader_id   = emu->TraderID;
				eq->unknown_008 = emu->Approval;

				FINISH_ENCODE();
				break;
			}
			case structs::RoF2BazaarTraderBuyerActions::BuyTraderItem: {
				ENCODE_LENGTH_EXACT(structs::TraderBuy_Struct);
				SETUP_DIRECT_ENCODE(TraderBuy_Struct, structs::TraderBuy_Struct);
				LogTrading(
					"(RoF2) item_id <green>[{}] price <green>[{}] quantity <green>[{}] trader_id <green>[{}]",
					eq->item_id,
					eq->price,
					eq->quantity,
					eq->trader_id
				);

				OUT(action);
				OUT(method);
				OUT(trader_id);
				OUT(item_id);
				OUT(price);
				OUT(already_sold);
				OUT(quantity);
				OUT_str(buyer_name);
				OUT_str(seller_name);
				OUT_str(item_name);
				OUT_str(serial_number);

				FINISH_ENCODE();
				break;
			}
			default: {
				LogTrading("(RoF2) Unhandled action <red>[{}]", action);
				EQApplicationPacket *in = *p;
				*p = nullptr;

				dest->FastQueuePacket(&in);
			}
		}
	}

	ENCODE(OP_TributeInfo)
	{
		ENCODE_LENGTH_ATLEAST(TributeAbility_Struct);
		SETUP_VAR_ENCODE(TributeAbility_Struct);
		ALLOC_VAR_ENCODE(structs::TributeAbility_Struct, sizeof(structs::TributeAbility_Struct) + strlen(emu->name) + 1);

		OUT(tribute_id);
		OUT(tier_count);

		for (uint32 i = 0; i < MAX_TRIBUTE_TIERS; ++i)
		{
			eq->tiers[i].level = emu->tiers[i].level;
			eq->tiers[i].tribute_item_id = emu->tiers[i].tribute_item_id;
			eq->tiers[i].cost = emu->tiers[i].cost;
		}

		eq->unknown128 = 0;

		strcpy(eq->name, emu->name);

		FINISH_ENCODE();
	}

	ENCODE(OP_TributeItem)
	{
		ENCODE_LENGTH_EXACT(TributeItem_Struct);
		SETUP_DIRECT_ENCODE(TributeItem_Struct, structs::TributeItem_Struct);

		eq->inventory_slot = ServerToRoF2Slot(emu->slot);
		OUT(quantity);
		OUT(tribute_master_id);
		OUT(tribute_points);

		FINISH_ENCODE();
	}

	ENCODE(OP_VetClaimReply)
	{
		ENCODE_LENGTH_EXACT(VeteranClaim);
		SETUP_DIRECT_ENCODE(VeteranClaim, structs::VeteranClaim);

		memcpy(eq->name, emu->name, sizeof(emu->name));
		OUT(claim_id);
		OUT(action);

		FINISH_ENCODE();
	}

	ENCODE(OP_VetRewardsAvaliable)
	{
		EQApplicationPacket *inapp = *p;
		auto __emu_buffer = inapp->pBuffer;

		uint32 count = ((*p)->Size() / sizeof(InternalVeteranReward));

		// calculate size of names, note the packet DOES NOT have null termed c-strings
		std::vector<uint32> name_lengths;
		for (int i = 0; i < count; ++i) {
			InternalVeteranReward *ivr = (InternalVeteranReward *)__emu_buffer;

			for (int i = 0; i < ivr->claim_count; i++) {
				uint32 length = strnlen(ivr->items[i].item_name, 63);
				if (length)
					name_lengths.push_back(length);
			}

			__emu_buffer += sizeof(InternalVeteranReward);
		}

		uint32 packet_size = std::accumulate(name_lengths.begin(), name_lengths.end(), 0) +
				     sizeof(structs::VeteranReward) + (sizeof(structs::VeteranRewardEntry) * count) +
				     // size of name_lengths is the same as item count
				     (sizeof(structs::VeteranRewardItem) * name_lengths.size());

		// build packet now!
		auto outapp = new EQApplicationPacket(OP_VetRewardsAvaliable, packet_size);
		__emu_buffer = inapp->pBuffer;

		outapp->WriteUInt32(count);
		auto name_itr = name_lengths.begin();
		for (int i = 0; i < count; i++) {
			InternalVeteranReward *ivr = (InternalVeteranReward *)__emu_buffer;

			outapp->WriteUInt32(ivr->claim_id);
			outapp->WriteUInt32(ivr->number_available);
			outapp->WriteUInt32(ivr->claim_count);
			outapp->WriteUInt8(1); // enabled

			for (int j = 0; j < ivr->claim_count; j++) {
				assert(name_itr != name_lengths.end()); // the way it's written, it should never happen, so just assert
				outapp->WriteUInt32(*name_itr);
				outapp->WriteData(ivr->items[j].item_name, *name_itr);
				outapp->WriteUInt32(ivr->items[j].item_id);
				outapp->WriteUInt32(ivr->items[j].charges);
				++name_itr;
			}

			__emu_buffer += sizeof(InternalVeteranReward);
		}

		dest->FastQueuePacket(&outapp);
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
		OUT(hero_forge_model);
		OUT(unknown18);
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

		// The struct fields were moved around a bit, so adjust values before copying
		wars->unknown44[0] = Count;
		wars->unknown52 = 0;

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

	ENCODE(OP_ZoneChange)
	{
		ENCODE_LENGTH_EXACT(ZoneChange_Struct);
		SETUP_DIRECT_ENCODE(ZoneChange_Struct, structs::ZoneChange_Struct);

		memcpy(eq->char_name, emu->char_name, sizeof(emu->char_name));
		OUT(zoneID);
		OUT(instanceID);
		OUT(y);
		OUT(x);
		OUT(z)
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

		char *Buffer = (char *)in->pBuffer, *BufferStart;

		int r;
		int k;
		for (r = 0; r < entrycount; r++, emu++) {

			int PacketSize = 206;

			PacketSize += strlen(emu->name);
			PacketSize += strlen(emu->lastName);

			emu->title[31] = 0;
			emu->suffix[31] = 0;

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
				PacketSize += 60;

				if (emu->size == 0)
				{
					emu->size = 6;
					SpawnSize = 6;
				}
			}
			else
				PacketSize += 216;

			if (SpawnSize == 0)
			{
				SpawnSize = 3;
			}

			auto outapp = new EQApplicationPacket(OP_ZoneEntry, PacketSize);
			Buffer = (char *)outapp->pBuffer;
			BufferStart = Buffer;
			VARSTRUCT_ENCODE_STRING(Buffer, emu->name);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->spawnId);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->level);
			// actually melee range variable, this probably screws the shit out of melee ranges :D
			if (emu->DestructibleObject)
			{
				VARSTRUCT_ENCODE_TYPE(float, Buffer, 10);	// was int and 0x41200000
			}
			else
			{
				VARSTRUCT_ENCODE_TYPE(float, Buffer, SpawnSize - 0.7);	// Eye Height?
			}
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->NPC); // 0 PC, 1 NPC etc

			structs::Spawn_Struct_Bitfields *Bitfields = (structs::Spawn_Struct_Bitfields*)Buffer;

			Bitfields->gender                 = emu->gender;
			Bitfields->ispet                  = emu->is_pet;
			Bitfields->afk                    = emu->afk;
			Bitfields->anon                   = emu->anon;
			Bitfields->gm                     = emu->gm;
			Bitfields->sneak                  = 0;
			Bitfields->lfg                    = emu->lfg;
			Bitfields->invis                  = emu->invis;
			Bitfields->linkdead               = 0;
			Bitfields->showhelm               = emu->showhelm;
			Bitfields->trader                 = emu->trader ? 1 : 0;
			Bitfields->targetable             = emu->NPC ? emu->untargetable : 1;
			Bitfields->targetable_with_hotkey = emu->targetable_with_hotkey ? 1 : 0;
			Bitfields->showname               = ShowName;

			if (emu->DestructibleObject)
			{
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0x1d600000);
				Buffer = Buffer - 4;
			}

			// Not currently found
			// Bitfields->statue = 0;
			// Bitfields->buyer = 0;

			Buffer += sizeof(structs::Spawn_Struct_Bitfields);

			// actually part of bitfields
			uint8 OtherData = 0;

			if (emu->class_ == Class::LDoNTreasure) //LDoN Chest
			{
				OtherData = OtherData | 0x04;
			}
			if (strlen(emu->title)) {
				OtherData = OtherData | 16;
			}
			if (strlen(emu->suffix)) {
				OtherData = OtherData | 32;
			}
			if (emu->DestructibleObject) {
				OtherData = OtherData | 0xe1;	// Live has 0xe1 for OtherData
			}
			if (emu->buyer) {
				OtherData = OtherData | 0x01;
			}

			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, OtherData);
			// float EmitterScalingRadius

			if (emu->DestructibleObject)
			{
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0x00000000);
			}
			else
			{
				VARSTRUCT_ENCODE_TYPE(float, Buffer, -1);	// unknown3
			}
			// int DefaultEmitterID
			VARSTRUCT_ENCODE_TYPE(float, Buffer, 0);	// unknown4

			if (emu->DestructibleObject || emu->class_ == Class::LDoNTreasure)
			{
				VARSTRUCT_ENCODE_STRING(Buffer, emu->DestructibleModel);
				VARSTRUCT_ENCODE_STRING(Buffer, emu->DestructibleName2);
				VARSTRUCT_ENCODE_STRING(Buffer, emu->DestructibleString);

				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->DestructibleAppearance);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->DestructibleUnk1); // ObjectAnimationID

				// these 10 are SoundIDs
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
				VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->DestructibleUnk8); // bInteractiveObjectCollidable
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->DestructibleUnk9); // IteractiveObjectType
			}


			if (!emu->DestructibleObject)
			{
				// Setting this next field to zero will cause a crash. Looking at ShowEQ, if it is zero, the bodytype field is not
				// present. Will sort that out later.
				// This is the CharacterPropertyHash, it can have multiple fields
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

			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->equip_chest2); // InNonPCRaceIllusion
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0); // material
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0); // variation
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->helm); // headtype

			VARSTRUCT_ENCODE_TYPE(float, Buffer, emu->size);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->face);
			VARSTRUCT_ENCODE_TYPE(float, Buffer, emu->walkspeed);
			VARSTRUCT_ENCODE_TYPE(float, Buffer, emu->runspeed);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->race);

			// From MQ2: todo: create enum for this byte. Holding: Nothing=0 A RightHand Weapon=1 A Shield=2 Dual Wielding Two Weapons=3 A Spear=4 A LeftHand Weapon=5 A Two Handed Weapon=6 A bow=7
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

				/* Translate older ranks to new values */
				//switch (emu->guildrank) {
				//case 0: { VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 5);  break; }  // GUILD_MEMBER	0
				//case 1: { VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 3);  break; }  // GUILD_OFFICER	1
				//case 2: { VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 1);  break; }  // GUILD_LEADER	2
				//default: { VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->guildrank); break; }  //
				//}
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->guildrank);
			}

			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->class_);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// pvp
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->StandState);	// standstate
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->light);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->flymode);

			//VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 100); // LiveUnknown1 12/06/14 (possibly mana percent?)
			//VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 10); // LiveUnknown2 12/06/14
			//VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0); // LiveUnknown3 12/06/14

			VARSTRUCT_ENCODE_STRING(Buffer, emu->lastName);

			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);	// aatitle
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->guild_show);
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0); // TempPet

			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->petOwnerId);

			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);                 // FindBits MQ2 name
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->PlayerState);
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->npc_tint_id); // NpcTintIndex
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);                // PrimaryTintIndex
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);                // SecondaryTintIndex
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0xffffffff);       // These do something with OP_WeaponEquip1
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0xffffffff);       // ^

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

				structs::Texture_Struct *Equipment = (structs::Texture_Struct *)Buffer;

				for (k = EQ::textures::textureBegin; k < EQ::textures::materialCount; k++) {
					Equipment[k].Material = emu->equipment.Slot[k].Material;
					Equipment[k].Unknown1 = emu->equipment.Slot[k].Unknown1;
					Equipment[k].EliteMaterial = emu->equipment.Slot[k].EliteModel;
					Equipment[k].HeroForgeModel = emu->equipment.Slot[k].HerosForgeModel;
					Equipment[k].Material2 = emu->equipment.Slot[k].Unknown2;
				}

				Buffer += (sizeof(structs::Texture_Struct) * EQ::textures::materialCount);
			}
			else
			{
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);

				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->equipment.Primary.Material);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);

				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu->equipment.Secondary.Material);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
			}

			structs::Spawn_Struct_Position *Position = (structs::Spawn_Struct_Position*)Buffer;

			Position->y = emu->y;
			Position->deltaZ = emu->deltaZ;
			Position->deltaX = emu->deltaX;
			Position->x = emu->x;
			Position->heading = emu->heading;
			Position->deltaHeading = emu->deltaHeading;
			Position->z = emu->z;
			Position->animation = emu->animation;
			Position->deltaY = emu->deltaY;

			Buffer += sizeof(structs::Spawn_Struct_Position);

			if (strlen(emu->title))
			{
				VARSTRUCT_ENCODE_STRING(Buffer, emu->title);
			}

			if (strlen(emu->suffix))
			{
				VARSTRUCT_ENCODE_STRING(Buffer, emu->suffix);
			}

			// skipping two ints
			// unknown, maybe some sort of spawn ID
			// SplineID -- no idea
			Buffer += 8;
			VARSTRUCT_ENCODE_TYPE(uint8, Buffer, emu->IsMercenary);
			VARSTRUCT_ENCODE_STRING(Buffer, "0000000000000000"); // RealEstateItemGuid
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0xffffffff); // RealEstateID
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0xffffffff); // RealEstateItemID
			// 29 zero bytes follow
			// PhysicsEffects follow here ... unsure what they are but it's a count followed by a struct like {spellid, casterid, effectid, baseeffect}
			Buffer += 29;
			if (Buffer != (BufferStart + PacketSize))
			{
				LogNetcode("[ERROR] SPAWN ENCODE LOGIC PROBLEM: Buffer pointer is now [{}] from end", Buffer - (BufferStart + PacketSize));
			}
			//Log.LogDebugType(Logs::General, Logs::Netcode, "[ERROR] Sending zone spawn for %s packet is %i bytes", emu->name, outapp->size);
			//Log.Hex(Logs::Netcode, outapp->pBuffer, outapp->size);
			dest->FastQueuePacket(&outapp, ack_req);
		}

		delete in;
	}

	ENCODE(OP_CrystalCountUpdate)
	{
		ENCODE_LENGTH_EXACT(CrystalCountUpdate_Struct);
		SETUP_DIRECT_ENCODE(CrystalCountUpdate_Struct, structs::CrystalCountUpdate_Struct);

		OUT(CurrentRadiantCrystals);
		OUT(CareerRadiantCrystals);
		OUT(CurrentEbonCrystals);
		OUT(CareerEbonCrystals);

		FINISH_ENCODE();
	}

// DECODE methods

	DECODE(OP_AdventureMerchantSell)
	{
		DECODE_LENGTH_EXACT(structs::Adventure_Sell_Struct);
		SETUP_DIRECT_DECODE(Adventure_Sell_Struct, structs::Adventure_Sell_Struct);

		IN(npcid);
		emu->slot = RoF2ToServerTypelessSlot(eq->inventory_slot, invtype::typePossessions);
		IN(charges);
		IN(sell_price);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_AltCurrencySell)
	{
		DECODE_LENGTH_EXACT(structs::AltCurrencySellItem_Struct);
		SETUP_DIRECT_DECODE(AltCurrencySellItem_Struct, structs::AltCurrencySellItem_Struct);

		IN(merchant_entity_id);
		emu->slot_id = RoF2ToServerTypelessSlot(eq->inventory_slot, invtype::typePossessions);
		IN(charges);
		IN(cost);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_AltCurrencySellSelection)
	{
		DECODE_LENGTH_EXACT(structs::AltCurrencySelectItem_Struct);
		SETUP_DIRECT_DECODE(AltCurrencySelectItem_Struct, structs::AltCurrencySelectItem_Struct);

		IN(merchant_entity_id);
		emu->slot_id = RoF2ToServerTypelessSlot(eq->inventory_slot, invtype::typePossessions);

		FINISH_DIRECT_DECODE();
	}

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

		emu->inventorySlot = RoF2ToServerTypelessSlot(eq->inventorySlot, invtype::typePossessions);
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

		emu->container_slot = RoF2ToServerSlot(eq->container_slot);
		emu->augment_slot = RoF2ToServerSlot(eq->augment_slot);
		emu->container_index = eq->container_index;
		emu->augment_index = eq->augment_index;
		emu->dest_inst_id = eq->dest_inst_id;
		emu->augment_action = eq->augment_action;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_Barter)
	{
		auto action = *(uint32 *) __packet->pBuffer;

		switch (action) {
			case structs::RoF2BuyerActions::BuyerRemoveItem: {
				auto emu = (BuyerGeneric_Struct *) __packet->pBuffer;
				emu->action = Barter_BuyerItemRemove;
				LogTradingDetail("(RoF2) Buyer Remove Item");

				break;
			}
			case structs::RoF2BuyerActions::BuyerInspectBegin: {
				LogTradingDetail("(RoF2) Buyer Inspect Begin Item");

				auto emu = (BuyerGeneric_Struct *) __packet->pBuffer;
				emu->action = Barter_BuyerInspectBegin;

				break;
			}
			case structs::RoF2BuyerActions::BuyerInspectEnd: {
				LogTradingDetail("(RoF2) Buyer Inspect End Item ");

				auto emu = (BuyerGeneric_Struct *) __packet->pBuffer;
				emu->action = Barter_BuyerInspectEnd;

				break;
			}
			case structs::RoF2BuyerActions::BuyerWelcomeMessage: {
				LogTradingDetail("(RoF2) Buyer Welcome Message Update");
				SETUP_DIRECT_DECODE(BuyerWelcomeMessageUpdate_Struct, structs::BuyerWelcomeMessageUpdate_Struct);

				emu->action = Barter_WelcomeMessageUpdate;
				strn0cpy(emu->welcome_message, eq->welcome_message, sizeof(emu->welcome_message));

				FINISH_DIRECT_DECODE();
				break;
			}
			case structs::RoF2BuyerActions::BuyerItemInspect: {
				SETUP_DIRECT_DECODE(BarterItemSearchLinkRequest_Struct, structs::BarterItemSearchLinkRequest_Struct);
				LogTradingDetail("(RoF2) Seller ID <green>[{}] Inspecting Item <green>[{}] from Buyer ID <green>[{}] ",
								 eq->seller_id,
								 eq->item_id,
								 eq->buyer_id
				);

				emu->action      = Barter_BarterItemInspect;
				emu->item_id     = eq->item_id;
				emu->searcher_id = eq->seller_id;

				FINISH_DIRECT_DECODE();
				break;
			}
			default: {
				auto emu = (BuyerGeneric_Struct *) __packet->pBuffer;
				LogTradingDetail("(RoF2) Pass thru OP_Barter packet action <red>[{}]", emu->action);
			}
		}
	}

	DECODE(OP_BazaarSearch)
	{
		char *Buffer = (char *)__packet->pBuffer;

		uint8 SubAction = VARSTRUCT_DECODE_TYPE(uint8, Buffer);
		LogTrading("(RoF2) action <green>[{}]", SubAction);

		if ((SubAction != BazaarInspectItem) || (__packet->size != sizeof(structs::NewBazaarInspect_Struct)))
			return;

		SETUP_DIRECT_DECODE(NewBazaarInspect_Struct, structs::NewBazaarInspect_Struct);

		IN(Beginning.Action);
		memcpy(emu->Name, eq->Name, sizeof(emu->Name));
		IN(SerialNumber);
		LogTrading("(RoF2) action <green>[{}] serial_number <green>[{}]", eq->Beginning.Action, eq->SerialNumber);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_BlockedBuffs)
	{
		DECODE_LENGTH_EXACT(structs::BlockedBuffs_Struct);
		SETUP_DIRECT_DECODE(BlockedBuffs_Struct, structs::BlockedBuffs_Struct);

		for (uint32 i = 0; i < BLOCKED_BUFF_COUNT; ++i)
			emu->SpellID[i] = eq->SpellID[i];

		IN(Count);
		IN(Pet);
		IN(Initialise);
		IN(Flags);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_BookButton)
	{
		DECODE_LENGTH_EXACT(structs::BookButton_Struct);
		SETUP_DIRECT_DECODE(BookButton_Struct, structs::BookButton_Struct);

		emu->invslot = static_cast<int16_t>(RoF2ToServerTypelessSlot(eq->slot, invtype::typePossessions));
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
		emu->slotid = RoF2ToServerBuffSlot(eq->slotid);
		IN(bufffade);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_BuffRemoveRequest)
	{
		// This is to cater for the fact that short buff box buffs start at 30 as opposed to 25 in prior clients.
		//
		DECODE_LENGTH_EXACT(structs::BuffRemoveRequest_Struct);
		SETUP_DIRECT_DECODE(BuffRemoveRequest_Struct, structs::BuffRemoveRequest_Struct);

		emu->SlotID = RoF2ToServerBuffSlot(eq->SlotID);

		IN(EntityID);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_BuyerItems)
	{
		auto action = *(uint32 *) __packet->pBuffer;

		switch (action) {
			case structs::RoF2BuyerActions::BuyerModifyBuyLine:
			case structs::RoF2BuyerActions::BuyerBuyLine: {
				BuyerBuyLines_Struct buyer_buy_lines{};
				auto                 buffer = (char *) __packet->pBuffer;

				buyer_buy_lines.action = VARSTRUCT_DECODE_TYPE(uint32, buffer);

				buyer_buy_lines.no_items = 1;
				if (action == structs::RoF2BuyerActions::BuyerBuyLine) {
					buyer_buy_lines.no_items = VARSTRUCT_DECODE_TYPE(uint16, buffer);
				}

				buyer_buy_lines.buy_lines.reserve(buyer_buy_lines.no_items);
				for (int i = 0; i < buyer_buy_lines.no_items; i++) {
					BuyerLineItems_Struct b{};
					b.slot           = VARSTRUCT_DECODE_TYPE(uint32, buffer);
					b.enabled        = VARSTRUCT_DECODE_TYPE(uint8, buffer);
					b.item_id        = VARSTRUCT_DECODE_TYPE(uint32, buffer);
					b.item_name      = std::string(buffer, strlen(buffer));
					buffer          += strlen(buffer) + 1;
					b.item_icon      = VARSTRUCT_DECODE_TYPE(uint32, buffer);
					b.item_quantity  = VARSTRUCT_DECODE_TYPE(uint32, buffer);
					b.item_toggle    = VARSTRUCT_DECODE_TYPE(uint8, buffer);
					b.item_cost      = VARSTRUCT_DECODE_TYPE(uint32, buffer);
					auto trade_items = VARSTRUCT_DECODE_TYPE(uint32, buffer);
					buyer_buy_lines.buy_lines.push_back(b);

					if (trade_items > 0) {
						buyer_buy_lines.buy_lines[i].trade_items.reserve(trade_items);
						for (int x = 0; x < trade_items; x++) {
							BuyerLineTradeItems_Struct blti{};
							blti.item_id       = VARSTRUCT_DECODE_TYPE(uint32, buffer);
							blti.item_quantity = VARSTRUCT_DECODE_TYPE(uint32, buffer);
							blti.item_icon     = VARSTRUCT_DECODE_TYPE(uint32, buffer);
							blti.item_name     = std::string(buffer, strlen(buffer));
							buffer            += strlen(buffer) + 1;
							buyer_buy_lines.buy_lines[i].trade_items.push_back(blti);
						}
					}
					buffer += 13;
				}

				buffer = nullptr;
				std::stringstream           ss{};
				cereal::BinaryOutputArchive ar(ss);
				{
					ar(buyer_buy_lines);
				}

				auto new_size   = sizeof(BuyerGeneric_Struct) + ss.str().length();
				auto new_packet = new unsigned char[new_size];
				__packet->size    = new_size;
				__packet->pBuffer = new_packet;
				auto emu = (BuyerGeneric_Struct *) __packet->pBuffer;
				emu->action = Barter_BuyerItemUpdate;

				if (action == structs::RoF2BuyerActions::BuyerBuyLine) {
					emu->action = Barter_BuyerItemStart;
				}

				memcpy(emu->payload, ss.str().data(), ss.str().length());
				__packet->SetOpcode(OP_Barter);

				break;
			}
			case structs::RoF2BuyerActions::BuyerSellItem: {
				BuyerLineSellItem_Struct sell_item{};

				char *buffer = (char *) __packet->pBuffer;

				sell_item.action          = VARSTRUCT_DECODE_TYPE(uint32, buffer);
				sell_item.purchase_method = VARSTRUCT_DECODE_TYPE(uint32, buffer);
				buffer += 4;
				sell_item.buyer_entity_id = VARSTRUCT_DECODE_TYPE(uint32, buffer);
				sell_item.buyer_id        = VARSTRUCT_DECODE_TYPE(uint32, buffer);
				buffer += 11;
				sell_item.slot    = VARSTRUCT_DECODE_TYPE(uint32, buffer);
				sell_item.enabled = VARSTRUCT_DECODE_TYPE(uint8, buffer);
				sell_item.item_id = VARSTRUCT_DECODE_TYPE(uint32, buffer);
				VARSTRUCT_DECODE_STRING(sell_item.item_name, buffer);
				sell_item.item_icon      = VARSTRUCT_DECODE_TYPE(uint32, buffer);
				sell_item.item_quantity  = VARSTRUCT_DECODE_TYPE(uint32, buffer);
				sell_item.item_toggle    = VARSTRUCT_DECODE_TYPE(uint8, buffer);
				sell_item.item_cost      = VARSTRUCT_DECODE_TYPE(uint32, buffer);
				sell_item.no_trade_items = VARSTRUCT_DECODE_TYPE(uint32, buffer);

				if (sell_item.no_trade_items > 0) {
					sell_item.trade_items.reserve(sell_item.no_trade_items);
					for (int x = 0; x < sell_item.no_trade_items; x++) {
						BuyerLineTradeItems_Struct blti{};
						blti.item_id       = VARSTRUCT_DECODE_TYPE(uint32, buffer);
						blti.item_quantity = VARSTRUCT_DECODE_TYPE(uint32, buffer);
						blti.item_icon     = VARSTRUCT_DECODE_TYPE(uint32, buffer);
						blti.item_name     = std::string(buffer, strlen(buffer));
						buffer            += strlen(buffer) + 1;
						sell_item.trade_items.push_back(blti);
					}
				}

				if (sell_item.purchase_method) {
					sell_item.buyer_entity_id = VARSTRUCT_DECODE_TYPE(uint32, buffer);
					sell_item.buyer_id        = VARSTRUCT_DECODE_TYPE(uint32, buffer);
					sell_item.zone_id         = VARSTRUCT_DECODE_TYPE(uint32, buffer);
					sell_item.buyer_name      = std::string(buffer, strlen(buffer));
					buffer                   += sell_item.buyer_name.length() + 1;
				}
				else {
					buffer += 13;
				}

				sell_item.seller_quantity = VARSTRUCT_DECODE_TYPE(uint32, buffer);

				buffer += 4;

				buffer = nullptr;
				std::stringstream           ss{};
				cereal::BinaryOutputArchive ar(ss);
				{
					ar(sell_item);
				}

				auto new_size   = sizeof(BuyerGeneric_Struct) + ss.str().length();
				auto new_packet = new unsigned char[new_size];
				__packet->size    = new_size;
				__packet->pBuffer = new_packet;
				auto emu = (BuyerGeneric_Struct *) __packet->pBuffer;
				emu->action = Barter_SellItem;

				memcpy(emu->payload, ss.str().data(), ss.str().length());
				__packet->SetOpcode(OP_Barter);

				break;
			}
		}
	}

	DECODE(OP_CastSpell)
	{
		DECODE_LENGTH_EXACT(structs::CastSpell_Struct);
		SETUP_DIRECT_DECODE(CastSpell_Struct, structs::CastSpell_Struct);

		emu->slot = static_cast<uint32>(RoF2ToServerCastingSlot(static_cast<spells::CastingSlot>(eq->slot)));

		IN(spell_id);
		emu->inventoryslot = RoF2ToServerSlot(eq->inventory_slot);
		//IN(inventoryslot);
		IN(target_id);
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
		RoF2ToServerSayLink(new_message, old_message);

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

		emu->slot = RoF2ToServerSlot(eq->inventory_slot);
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

		emu->from_slot = RoF2ToServerSlot(eq->from_slot);
		emu->to_slot = RoF2ToServerSlot(eq->to_slot);
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
		RoF2ToServerSayLink(new_message, old_message);

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

	DECODE(OP_GMLastName)
	{
		DECODE_LENGTH_EXACT(structs::GMLastName_Struct);
		SETUP_DIRECT_DECODE(GMLastName_Struct, structs::GMLastName_Struct);

		memcpy(emu->name, eq->name, sizeof(emu->name));
		memcpy(emu->gmname, eq->gmname, sizeof(emu->gmname));
		memcpy(emu->lastname, eq->lastname, sizeof(emu->lastname));
		for (int i = 0; i<4; i++)
		{
			emu->unknown[i] = eq->unknown[i];
		}

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

	DECODE(OP_GuildBank)
	{
		// all actions are 1 off due to the removal of one of enums
		switch (__packet->ReadUInt32()) {
		case 2: {// GuildBankPromote
			DECODE_LENGTH_EXACT(structs::GuildBankPromote_Struct);
			SETUP_DIRECT_DECODE(GuildBankPromote_Struct, structs::GuildBankPromote_Struct);
			emu->Action = 3;
			IN(Unknown04);
			IN(Slot);
			IN(Slot2);
			FINISH_DIRECT_DECODE();
			return;
		}
		case 3: { // GuildBankViewItem
			DECODE_LENGTH_EXACT(structs::GuildBankViewItem_Struct);
			SETUP_DIRECT_DECODE(GuildBankViewItem_Struct, structs::GuildBankViewItem_Struct);
			emu->Action = 4;
			IN(Unknown04);
			IN(SlotID);
			IN(Area);
			IN(Unknown12);
			IN(Unknown16);
			FINISH_DIRECT_DECODE();
			return;
		}
		case 4: { // GuildBankDeposit
			__packet->WriteUInt32(5);
			return;
		}
		case 5: { // GuildBankPermissions
			DECODE_LENGTH_EXACT(structs::GuildBankPermissions_Struct);
			SETUP_DIRECT_DECODE(GuildBankPermissions_Struct, structs::GuildBankPermissions_Struct);
			emu->Action = 6;
			IN(Unknown04);
			IN(SlotID);
			IN(Unknown10);
			IN(ItemID);
			IN(Permissions);
			strn0cpy(emu->MemberName, eq->MemberName, 64);
			FINISH_DIRECT_DECODE();
			return;
		}
		case 6: { // GuildBankWithdraw
			DECODE_LENGTH_EXACT(structs::GuildBankWithdrawItem_Struct);
			SETUP_DIRECT_DECODE(GuildBankWithdrawItem_Struct, structs::GuildBankWithdrawItem_Struct);
			emu->Action = 7;
			IN(Unknown04);
			IN(SlotID);
			IN(Area);
			IN(Unknown12);
			IN(Quantity);
			FINISH_DIRECT_DECODE();
			return;
		}
		case 7: { // GuildBankSplitStacks
			DECODE_LENGTH_EXACT(structs::GuildBankWithdrawItem_Struct);
			SETUP_DIRECT_DECODE(GuildBankWithdrawItem_Struct, structs::GuildBankWithdrawItem_Struct);
			emu->Action = 8;
			IN(Unknown04);
			IN(SlotID);
			IN(Area);
			IN(Unknown12);
			IN(Quantity);
			FINISH_DIRECT_DECODE();
			return;
		}
		case 8: { // GuildBankMergeStacks
			DECODE_LENGTH_EXACT(structs::GuildBankWithdrawItem_Struct);
			SETUP_DIRECT_DECODE(GuildBankWithdrawItem_Struct, structs::GuildBankWithdrawItem_Struct);
			emu->Action = 9;
			IN(Unknown04);
			IN(SlotID);
			IN(Area);
			IN(Unknown12);
			IN(Quantity);
			FINISH_DIRECT_DECODE();
			return;
		}
		default:
			LogNetcode("Unhandled OP_GuildBank action");
			__packet->SetOpcode(OP_Unknown); /* invalidate the packet */
			return;
		}
	}

	DECODE(OP_GuildDemote)
	{
		DECODE_LENGTH_EXACT(structs::GuildDemoteStruct);
		SETUP_DIRECT_DECODE(GuildDemoteStruct, structs::GuildDemoteStruct);

		strn0cpy(emu->target, eq->target, sizeof(emu->target));
		strn0cpy(emu->name, eq->name, sizeof(emu->name));
		IN(rank);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GuildRemove)
	{
		DECODE_LENGTH_EXACT(structs::GuildCommand_Struct);
		SETUP_DIRECT_DECODE(GuildCommand_Struct, structs::GuildCommand_Struct);

		strn0cpy(emu->othername, eq->othername, sizeof(emu->othername));
		strn0cpy(emu->myname, eq->myname, sizeof(emu->myname));
		IN(guildeqid);
		IN(officer);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GuildStatus)
	{
		DECODE_LENGTH_EXACT(structs::GuildStatus_Struct);
		SETUP_DIRECT_DECODE(GuildStatus_Struct, structs::GuildStatus_Struct);

		memcpy(emu->Name, eq->Name, sizeof(emu->Name));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GuildTributeDonateItem)
	{
		DECODE_LENGTH_EXACT(structs::GuildTributeDonateItemRequest_Struct);
		SETUP_DIRECT_DECODE(GuildTributeDonateItemRequest_Struct, structs::GuildTributeDonateItemRequest_Struct);

		Log(Logs::Detail, Logs::Netcode, "RoF2::DECODE(OP_GuildTributeDonateItem)");

		IN(type);
		IN(slot);
		IN(sub_index);
		IN(aug_index);
		IN(unknown10);
		IN(quantity);
		IN(tribute_master_id);
		IN(unknown20);
		IN(guild_id);
		IN(unknown28);
		IN(unknown32);

		structs::InventorySlot_Struct iss;
		iss.Slot = eq->slot;
		iss.SubIndex = eq->sub_index;
		iss.AugIndex = eq->aug_index;
		iss.Type = eq->type;
		iss.Unknown01 = 0;
		iss.Unknown02 = 0;

		emu->slot = RoF2ToServerSlot(iss);

		FINISH_DIRECT_DECODE();
	}

	/*DECODE(OP_InspectAnswer)
	{
	DECODE_LENGTH_EXACT(structs::InspectResponse_Struct);
	SETUP_DIRECT_DECODE(InspectResponse_Struct, structs::InspectResponse_Struct);

	IN(TargetID);
	IN(playerid);

	int r;
	for (r = 0; r < 21; r++) {
	strn0cpy(emu->itemnames[r], eq->itemnames[r], sizeof(emu->itemnames[r]));
	}
	// Swap last 2 slots for Arrow and Power Source
	strn0cpy(emu->itemnames[22], eq->itemnames[21], sizeof(emu->itemnames[22]));
	strn0cpy(emu->itemnames[21], eq->unknown_zero, sizeof(emu->itemnames[21]));
	strn0cpy(emu->unknown_zero, eq->unknown_zero, sizeof(emu->unknown_zero));

	int k;
	for (k = 0; k < 21; k++) {
	IN(itemicons[k]);
	}
	// Swap last 2 slots for Arrow and Power Source
	emu->itemicons[22] = eq->itemicons[21];
	emu->itemicons[21] = eq->unknown_zero2;
	emu->unknown_zero2 = eq->unknown_zero2;
	strn0cpy(emu->text, eq->text, sizeof(emu->text));
	//emu->unknown1772 = 0;

	FINISH_DIRECT_DECODE();
	}*/

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
		for (r = EQ::invaug::SOCKET_BEGIN; r <= EQ::invaug::SOCKET_END; r++) {
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

		emu->slot = RoF2ToServerSlot(eq->inventory_slot);
		IN(target);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_LoadSpellSet)
	{
		DECODE_LENGTH_EXACT(structs::LoadSpellSet_Struct);
		SETUP_DIRECT_DECODE(LoadSpellSet_Struct, structs::LoadSpellSet_Struct);

		for (unsigned int i = 0; i < EQ::spells::SPELL_GEM_COUNT; ++i)
		{
			if (eq->spell[i] == 0)
				emu->spell[i] = 0xFFFFFFFF;
			else
				emu->spell[i] = eq->spell[i];
		}

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_LootItem)
	{
		DECODE_LENGTH_EXACT(structs::LootingItem_Struct);
		SETUP_DIRECT_DECODE(LootingItem_Struct, structs::LootingItem_Struct);

		Log(Logs::Detail, Logs::Netcode, "RoF2::DECODE(OP_LootItem)");

		IN(lootee);
		IN(looter);
		emu->slot_id = RoF2ToServerCorpseMainSlot(eq->slot_id);
		IN(auto_loot);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_MoveItem)
	{
		DECODE_LENGTH_EXACT(structs::MoveItem_Struct);
		SETUP_DIRECT_DECODE(MoveItem_Struct, structs::MoveItem_Struct);

		Log(Logs::Detail, Logs::Netcode, "RoF2::DECODE(OP_MoveItem)");

		emu->from_slot = RoF2ToServerSlot(eq->from_slot);
		emu->to_slot = RoF2ToServerSlot(eq->to_slot);
		IN(number_in_stack);

		//LogNetcode("[RoF2] MoveItem Slot from [{}] to [{}], Number [{}]", emu->from_slot, emu->to_slot, emu->number_in_stack);

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
		emu->invslot = static_cast<int16_t>(RoF2ToServerTypelessSlot(eq->invslot, invtype::typePossessions));
		IN(target_id);
		emu->window = (uint8)eq->window;
		strn0cpy(emu->txtfile, eq->txtfile, sizeof(emu->txtfile));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_RecipeAutoCombine)
	{
		DECODE_LENGTH_EXACT(structs::RecipeAutoCombine_Struct);
		SETUP_DIRECT_DECODE(RecipeAutoCombine_Struct, structs::RecipeAutoCombine_Struct);

		IN(object_type);
		IN(some_id);
		emu->unknown1 = RoF2ToServerSlot(eq->container_slot);
		IN(recipe_id);
		IN(reply_code);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_RemoveBlockedBuffs) { DECODE_FORWARD(OP_BlockedBuffs); }

	DECODE(OP_RezzAnswer)
	{
		DECODE_LENGTH_EXACT(structs::Resurrect_Struct);
		SETUP_DIRECT_DECODE(Resurrect_Struct, structs::Resurrect_Struct);

		IN(zone_id);
		IN(instance_id);
		IN(y);
		IN(x);
		IN(z);
		memcpy(emu->your_name, eq->your_name, sizeof(emu->your_name));
		memcpy(emu->rezzer_name, eq->rezzer_name, sizeof(emu->rezzer_name));
		IN(spellid);
		memcpy(emu->corpse_name, eq->corpse_name, sizeof(emu->corpse_name));
		IN(action);

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
			// Size 40 in RoF2
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
		emu->itemslot = RoF2ToServerTypelessSlot(eq->inventory_slot, invtype::typePossessions);
		//IN(itemslot);
		IN(quantity);
		IN(price);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ShopSendParcel)
	{
		DECODE_LENGTH_EXACT(structs::Parcel_Struct);
		SETUP_DIRECT_DECODE(Parcel_Struct, structs::Parcel_Struct);

		IN(npc_id);
		IN(quantity);
		IN(money_flag);
		emu->item_slot = RoF2ToServerTypelessSlot(eq->inventory_slot, invtype::typePossessions);
		strn0cpy(emu->send_to, eq->send_to, sizeof(emu->send_to));
		strn0cpy(emu->note, eq->note, sizeof(emu->note));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_Trader)
	{
		auto action = *(uint32 *)__packet->pBuffer;

		switch (action) {
			case structs::RoF2BazaarTraderBuyerActions::BeginTraderMode: {
				DECODE_LENGTH_EXACT(structs::BeginTrader_Struct);
				SETUP_DIRECT_DECODE(ClickTrader_Struct, structs::BeginTrader_Struct);
				LogTrading("(RoF2) BeginTraderMode action <green>[{}]", action);

				emu->action = TraderOn;
				std::copy_n(eq->item_cost, RoF2::invtype::BAZAAR_SIZE, emu->item_cost);
				std::transform(
					std::begin(eq->items),
					std::end(eq->items),
					std::begin(emu->serial_number),
					[&](const structs::TraderItemSerial_Struct x) {
						return Strings::ToUnsignedBigInt(x.serial_number,0);
					}
				);

				FINISH_DIRECT_DECODE();
				break;
			}
			case structs::RoF2BazaarTraderBuyerActions::EndTraderMode: {
				DECODE_LENGTH_EXACT(structs::Trader_ShowItems_Struct);
				SETUP_DIRECT_DECODE(Trader_ShowItems_Struct, structs::Trader_ShowItems_Struct);
				LogTrading("(RoF2) EndTraderMode action <green>[{}]", action);

				emu->action    = TraderOff;
				emu->entity_id = eq->entity_id;

				FINISH_DIRECT_DECODE();
				break;
			}
			case structs::RoF2BazaarTraderBuyerActions::ListTraderItems: {
				LogTrading("(RoF2) ListTraderItems action <green>[{}]", action);
				break;
			}
			case structs::RoF2BazaarTraderBuyerActions::PriceUpdate: {
				DECODE_LENGTH_EXACT(structs::TraderPriceUpdate_Struct);
				SETUP_DIRECT_DECODE(TraderPriceUpdate_Struct, structs::TraderPriceUpdate_Struct);
				LogTrading("(RoF2) PriceUpdate action <green>[{}]", action);

				emu->Action       = PriceUpdate;
				emu->SerialNumber = Strings::ToUnsignedBigInt(eq->serial_number, 0);
				if (emu->SerialNumber == 0) {
					LogTrading("(RoF2) Price change with invalid serial number <red>[{}]", eq->serial_number);
				}
				emu->NewPrice = eq->new_price;

				FINISH_DIRECT_DECODE();
				break;
			}
			default: {
				LogTrading("(RoF2) Unhandled action <red>[{}]", action);
			}
		}
	}

	DECODE(OP_TraderBuy)
	{
		DECODE_LENGTH_EXACT(structs::TraderBuy_Struct);
		SETUP_DIRECT_DECODE(TraderBuy_Struct, structs::TraderBuy_Struct);
		LogTrading(
			"(RoF2) item_id <green>[{}] price <green>[{}] quantity <green>[{}] trader_id <green>[{}]",
			eq->item_id,
			eq->price,
			eq->quantity,
			eq->trader_id
		);

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
		uint32 action = *(uint32 *)__packet->pBuffer;

		switch (action) {
			case structs::RoF2BazaarTraderBuyerActions::BazaarSearch: {
				DECODE_LENGTH_EXACT(structs::BazaarSearch_Struct);
				SETUP_DIRECT_DECODE(BazaarSearchCriteria_Struct, structs::BazaarSearch_Struct);
				LogTrading(
					"(RoF2) BazaarSearch action <green>[{}]",
					action
				);

				__packet->SetOpcode(OP_BazaarSearch);
				emu->action = BazaarSearch;
				emu->type   = eq->type == UINT32_MAX ? UINT8_MAX : eq->type;
				IN(item_stat);
				IN(max_cost);
				IN(min_cost);
				IN(max_level);
				IN(min_level);
				IN(race);
				IN(slot);
				IN(trader_id);
				IN(_class);
				IN(prestige);
				IN(search_scope);
				IN(max_results);
				IN(augment);
				IN_str(item_name);

				FINISH_DIRECT_DECODE();
				break;
			}
			case structs::RoF2BazaarTraderBuyerActions::ClickTrader: {
				DECODE_LENGTH_EXACT(structs::TraderClick_Struct);
				SETUP_DIRECT_DECODE(TraderClick_Struct, structs::TraderClick_Struct);

				emu->Code     = ClickTrader;
				emu->TraderID = eq->trader_id;
				emu->Approval = eq->unknown_008;
				LogTrading("(RoF2) ClickTrader action <green>[{}], trader_id <green>[{}], approval <green>[{}]",
						   eq->action,
						   eq->trader_id,
						   eq->unknown_008
				);
				FINISH_DIRECT_DECODE();
				break;
			}
			case structs::RoF2BazaarTraderBuyerActions::BazaarInspect: {
				DECODE_LENGTH_EXACT(structs::BazaarInspect_Struct);
				SETUP_DIRECT_DECODE(BazaarInspect_Struct, structs::BazaarInspect_Struct);

				__packet->SetOpcode(OP_BazaarSearch);
				IN(item_id);
				IN(trader_id);
				emu->action        = BazaarInspect;
				emu->serial_number = Strings::ToUnsignedInt(eq->serial_number, 0);
				if (emu->serial_number == 0) {
					LogTrading(
						"(RoF2) trader_id = <green>[{}] requested a BazaarInspect with an invalid serial number of <red>[{}]",
						eq->trader_id,
						eq->serial_number
					);
					FINISH_DIRECT_DECODE();
					return;
				}

				LogTrading("(RoF2) BazaarInspect action <green>[{}] item_id <green>[{}] serial_number <green>[{}]",
						   action,
						   eq->item_id,
						   eq->serial_number
				);
				FINISH_DIRECT_DECODE();
				break;
			}
			case structs::RoF2BazaarTraderBuyerActions::FirstOpenSearch: {
				__packet->SetOpcode(OP_BazaarSearch);
				LogTrading("(RoF2) First time opening Bazaar Search since zoning. Action <green>[{}]", action);
				break;
			}
			case structs::RoF2BazaarTraderBuyerActions::WelcomeMessage: {
				__packet->SetOpcode(OP_BazaarSearch);
				LogTrading("(RoF2) WelcomeMessage action <green>[{}]", action);
				break;
			}
			case structs::RoF2BazaarTraderBuyerActions::BuyTraderItem: {
				DECODE_LENGTH_EXACT(structs::TraderBuy_Struct);
				SETUP_DIRECT_DECODE(TraderBuy_Struct, structs::TraderBuy_Struct);
				LogTrading(
					"(RoF2) item_id <green>[{}] price <green>[{}] quantity <green>[{}] trader_id <green>[{}]",
					eq->item_id,
					eq->price,
					eq->quantity,
					eq->trader_id
				);

				__packet->SetOpcode(OP_TraderBuy);
				IN(action);
				IN(method);
				IN(trader_id);
				IN(item_id);
				IN(price);
				IN(already_sold);
				IN(quantity);
				IN_str(buyer_name);
				IN_str(seller_name);
				IN_str(item_name);
				IN_str(serial_number);

				FINISH_DIRECT_DECODE();
				break;
			}
			default: {
				LogTrading("(RoF2) Unhandled action <red>[{}]", action);
			}
			return;
		}
	}

	DECODE(OP_TradeSkillCombine)
	{
		DECODE_LENGTH_EXACT(structs::NewCombine_Struct);
		SETUP_DIRECT_DECODE(NewCombine_Struct, structs::NewCombine_Struct);

		emu->container_slot = RoF2ToServerSlot(eq->container_slot);
		emu->guildtribute_slot = RoF2ToServerSlot(eq->guildtribute_slot); // this should only return INVALID_INDEX until implemented

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_TributeItem)
	{
		DECODE_LENGTH_EXACT(structs::TributeItem_Struct);
		SETUP_DIRECT_DECODE(TributeItem_Struct, structs::TributeItem_Struct);

		emu->slot = RoF2ToServerSlot(eq->inventory_slot);
		IN(quantity);
		IN(tribute_master_id);
		IN(tribute_points);

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

	DECODE(OP_VetClaimRequest)
	{
		DECODE_LENGTH_EXACT(structs::VeteranClaim);
		SETUP_DIRECT_DECODE(VeteranClaim, structs::VeteranClaim);

		IN(claim_id);

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

	DECODE(OP_ZoneEntry)
	{
		DECODE_LENGTH_EXACT(structs::ClientZoneEntry_Struct);
		SETUP_DIRECT_DECODE(ClientZoneEntry_Struct, structs::ClientZoneEntry_Struct);

		memcpy(emu->char_name, eq->char_name, sizeof(emu->char_name));

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

	void SerializeItem(EQ::OutBuffer& ob, const EQ::ItemInstance *inst, int16 slot_id_in, uint8 depth, ItemPacketType packet_type)
	{
		const EQ::ItemData *item = inst->GetUnscaledItem();

		RoF2::structs::ItemSerializationHeader hdr;

		//sprintf(hdr.unknown000, "06e0002Y1W00");
		strn0cpy(hdr.unknown000, fmt::format("{:016}\0", inst->GetSerialNumber()).c_str(),sizeof(hdr.unknown000));

		hdr.stacksize = 1;

		if (item->ID == PARCEL_MONEY_ITEM_ID) {
			hdr.stacksize = inst->GetPrice();
		} else if (inst->IsStackable()) {
			if (inst->GetCharges() > std::numeric_limits<int16>::max()) {
				hdr.stacksize = std::numeric_limits<uint32>::max();
			} else {
				hdr.stacksize = inst->GetCharges();
			}
		}

		hdr.unknown004 = 0;

		structs::InventorySlot_Struct slot_id{};
		switch (packet_type) {
		case ItemPacketLoot:
			slot_id = ServerToRoF2CorpseSlot(slot_id_in);
			break;
		default:
			slot_id = ServerToRoF2Slot(slot_id_in);
			break;
		}

		hdr.slot_type      = (inst->GetMerchantSlot() ? invtype::typeMerchant : slot_id.Type);
		hdr.main_slot      = (inst->GetMerchantSlot() ? inst->GetMerchantSlot() : slot_id.Slot);
		hdr.sub_slot       = (inst->GetMerchantSlot() ? 0xffff : slot_id.SubIndex);
		hdr.aug_slot       = (inst->GetMerchantSlot() ? 0xffff : slot_id.AugIndex);
		hdr.price          = inst->GetPrice();
		hdr.merchant_slot  = ((inst->GetMerchantSlot() ? inst->GetMerchantCount() : 1));
		hdr.scaled_value   = (inst->IsScaling() ? (inst->GetExp() / 100) : 0);
		hdr.instance_id    = (inst->GetMerchantSlot() ? inst->GetMerchantSlot() : inst->GetSerialNumber());
		hdr.parcel_item_id = packet_type == ItemPacketParcel ? inst->GetID() : 0;
		if (item->EvolvingItem && packet_type != ItemPacketParcel && packet_type != ItemPacketMerchant) {
			hdr.instance_id    = inst->GetEvolveUniqueID() & 0xFFFFFFFF; //lower dword
			hdr.parcel_item_id = inst->GetEvolveUniqueID() >> 32;        //upper dword
		}

		hdr.last_cast_time = inst->GetRecastTimestamp();
		hdr.charges        = (inst->IsStackable() ? (item->MaxCharges ? 1 : 0) : ((inst->GetCharges() > 254)
			? 0xFFFFFFFF
			: inst->GetCharges()));
		hdr.inst_nodrop    = (inst->IsAttuned() ? 1 : 0);
		hdr.unknown044     = 0;
		hdr.unknown048     = 0;
		hdr.unknown052     = 0;
		hdr.isEvolving     = item->EvolvingItem;

		ob.write((const char*)&hdr, sizeof(RoF2::structs::ItemSerializationHeader));

		if (item->EvolvingItem > 0) {
			RoF2::structs::EvolvingItem_Struct evotop;
			inst->CalculateEvolveProgression();

			evotop.final_item_id    = inst->GetEvolveFinalItemID();
			evotop.evolve_level     = item->EvolvingLevel;
			evotop.progress         = inst->GetEvolveProgression();
			evotop.activated        = inst->GetEvolveActivated();
			evotop.evolve_max_level = item->EvolvingMax;

			ob.write((const char*)&evotop, sizeof(RoF2::structs::EvolvingItem_Struct));
		}

		/**
		 * Ornamentation
		 */
		uint32 ornamentation_icon         = (inst->GetOrnamentationIcon() ? inst->GetOrnamentationIcon() : 0);
		uint32 hero_model                 = 0;

		if (inst->GetOrnamentationIDFile()) {
			hero_model = inst->GetOrnamentHeroModel(EQ::InventoryProfile::CalcMaterialFromSlot(slot_id_in));

			char tmp[30];
			memset(tmp, 0x0, 30);
			sprintf(tmp, "IT%d", inst->GetOrnamentationIDFile());

			//Mainhand
			ob.write(tmp, strlen(tmp));
			ob.write("\0", 1);

			//Offhand
			ob.write(tmp, strlen(tmp));
			ob.write("\0", 1);
		}
		else {
			ob.write("\0", 1); // no main hand Ornamentation
			ob.write("\0", 1); // no off hand Ornamentation
		}

		RoF2::structs::ItemSerializationHeaderFinish hdrf;

		hdrf.ornamentIcon = ornamentation_icon;
		hdrf.unknowna1 = 0xffffffff;
		hdrf.ornamentHeroModel = hero_model;
		hdrf.unknown063 = 0;
		hdrf.Copied = 0;
		hdrf.unknowna4 = 0xffffffff;
		hdrf.unknowna5 = 0;
		hdrf.ItemClass = item->ItemClass;

		ob.write((const char*)&hdrf, sizeof(RoF2::structs::ItemSerializationHeaderFinish));

		if (strlen(item->Name) > 0) {
			ob.write(item->Name, strlen(item->Name));
			ob.write("\0", 1);
		}

		if (strlen(item->Lore) > 0)
			ob.write(item->Lore, strlen(item->Lore));
		ob.write("\0", 1);

		if (strlen(item->IDFile) > 0)
			ob.write(item->IDFile, strlen(item->IDFile));
		ob.write("\0", 1);

		ob.write("\0", 1);

		RoF2::structs::ItemBodyStruct ibs;
		memset(&ibs, 0, sizeof(RoF2::structs::ItemBodyStruct));

		ibs.id = item->ID;
		ibs.weight = item->Weight;
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
		ibs.SkillModType = (int8)(item->SkillModType);
		ibs.SkillModExtra = 0;
		ibs.BaneDmgRace = item->BaneDmgRace;
		ibs.BaneDmgBody = item->BaneDmgBody;
		ibs.BaneDmgRaceAmt = item->BaneDmgRaceAmt;
		ibs.BaneDmgAmt = item->BaneDmgAmt;
		ibs.Magic = item->Magic;
		ibs.CastTime_ = item->CastTime_;
		ibs.ReqLevel = ((item->ReqLevel > 100) ? 100 : item->ReqLevel);
		ibs.RecLevel = ((item->RecLevel > 100) ? 100 : item->RecLevel);
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
		ibs.Prestige = 0;
		ibs.ItemType = item->ItemType;
		ibs.Material = item->Material;
		ibs.MaterialUnknown1 = 0;
		ibs.EliteMaterial = item->EliteMaterial;
		ibs.HerosForgeModel = item->HerosForgeModel;
		ibs.MaterialUnknown2 = 0;
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

		ob.write((const char*)&ibs, sizeof(RoF2::structs::ItemBodyStruct));

		//charm text
		if (strlen(item->CharmFile) > 0)
			ob.write((const char*)item->CharmFile, strlen(item->CharmFile));
		ob.write("\0", 1);

		RoF2::structs::ItemSecondaryBodyStruct isbs;
		memset(&isbs, 0, sizeof(RoF2::structs::ItemSecondaryBodyStruct));

		isbs.augtype = item->AugType;
		isbs.augrestrict2 = -1;
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

		ob.write((const char*)&isbs, sizeof(RoF2::structs::ItemSecondaryBodyStruct));

		if (strlen(item->Filename) > 0)
			ob.write((const char*)item->Filename, strlen(item->Filename));
		ob.write("\0", 1);

		RoF2::structs::ItemTertiaryBodyStruct itbs;
		memset(&itbs, 0, sizeof(RoF2::structs::ItemTertiaryBodyStruct));

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
		itbs.unknown3 = 0xffffffff;
		itbs.unknown4 = 0;
		itbs.no_pet = item->NoPet;
		itbs.unknown5 = 0;

		itbs.potion_belt_enabled = item->PotionBelt;
		itbs.potion_belt_slots   = item->PotionBeltSlots;
		itbs.stacksize           =
			item->ID == PARCEL_MONEY_ITEM_ID ? 0x7FFFFFFF : ((inst->IsStackable() ? item->StackSize : 0));
		itbs.no_transfer         = item->NoTransfer;
		itbs.expendablearrow     = item->ExpendableArrow;

		// Done to hack older clients to label expendable fishing poles as such
		// July 28th, 2018 patch
		if (item->ItemType == EQ::item::ItemTypeFishingPole && item->SubType == 0) {
			itbs.expendablearrow = 1;
		}

		itbs.unknown8 = 0;
		itbs.unknown9 = 0;
		itbs.unknown10 = 0;
		itbs.unknown11 = 0;
		itbs.unknown12 = 0;
		itbs.unknown13 = 0;
		itbs.unknown14 = 0;

		ob.write((const char*)&itbs, sizeof(RoF2::structs::ItemTertiaryBodyStruct));

		// Effect Structures Broken down to allow variable length strings for effect names
		int32 effect_unknown = 0;

		RoF2::structs::ClickEffectStruct ices;
		memset(&ices, 0, sizeof(RoF2::structs::ClickEffectStruct));

		ices.effect = item->Click.Effect;
		ices.level2 = item->Click.Level2;
		ices.type = item->Click.Type;
		ices.level = item->Click.Level;
		ices.max_charges = item->MaxCharges;
		ices.cast_time = item->CastTime;
		ices.recast = item->RecastDelay;
		ices.recast_type = item->RecastType;

		ob.write((const char*)&ices, sizeof(RoF2::structs::ClickEffectStruct));

		if (strlen(item->ClickName) > 0)
			ob.write((const char*)item->ClickName, strlen(item->ClickName));
		ob.write("\0", 1);

		ob.write((const char*)&effect_unknown, sizeof(int32));	// clickunk7

		RoF2::structs::ProcEffectStruct ipes;
		memset(&ipes, 0, sizeof(RoF2::structs::ProcEffectStruct));

		ipes.effect = item->Proc.Effect;
		ipes.level2 = item->Proc.Level2;
		ipes.type = item->Proc.Type;
		ipes.level = item->Proc.Level;
		ipes.procrate = item->ProcRate;

		ob.write((const char*)&ipes, sizeof(RoF2::structs::ProcEffectStruct));

		if (strlen(item->ProcName) > 0)
			ob.write((const char*)item->ProcName, strlen(item->ProcName));
		ob.write("\0", 1);

		ob.write((const char*)&effect_unknown, sizeof(int32));	// unknown5

		RoF2::structs::WornEffectStruct iwes;
		memset(&iwes, 0, sizeof(RoF2::structs::WornEffectStruct));

		iwes.effect = item->Worn.Effect;
		iwes.level2 = item->Worn.Level2;
		iwes.type = item->Worn.Type;
		iwes.level = item->Worn.Level;

		ob.write((const char*)&iwes, sizeof(RoF2::structs::WornEffectStruct));

		if (strlen(item->WornName) > 0)
			ob.write((const char*)item->WornName, strlen(item->WornName));
		ob.write("\0", 1);

		ob.write((const char*)&effect_unknown, sizeof(int32));	// unknown6

		RoF2::structs::WornEffectStruct ifes;
		memset(&ifes, 0, sizeof(RoF2::structs::WornEffectStruct));

		ifes.effect = item->Focus.Effect;
		ifes.level2 = item->Focus.Level2;
		ifes.type = item->Focus.Type;
		ifes.level = item->Focus.Level;

		ob.write((const char*)&ifes, sizeof(RoF2::structs::WornEffectStruct));

		if (strlen(item->FocusName) > 0)
			ob.write((const char*)item->FocusName, strlen(item->FocusName));
		ob.write("\0", 1);

		ob.write((const char*)&effect_unknown, sizeof(int32));	// unknown6

		RoF2::structs::WornEffectStruct ises;
		memset(&ises, 0, sizeof(RoF2::structs::WornEffectStruct));

		ises.effect = item->Scroll.Effect;
		ises.level2 = item->Scroll.Level2;
		ises.type = item->Scroll.Type;
		ises.level = item->Scroll.Level;

		ob.write((const char*)&ises, sizeof(RoF2::structs::WornEffectStruct));

		if (strlen(item->ScrollName) > 0)
			ob.write((const char*)item->ScrollName, strlen(item->ScrollName));
		ob.write("\0", 1);

		ob.write((const char*)&effect_unknown, sizeof(int32));	// unknown6

		// Bard Effect?
		RoF2::structs::WornEffectStruct ibes;
		memset(&ibes, 0, sizeof(RoF2::structs::WornEffectStruct));

		ibes.effect = item->Bard.Effect;
		ibes.level2 = item->Bard.Level2;
		ibes.type = item->Bard.Type;
		ibes.level = item->Bard.Level;
		//ibes.unknown6 = 0xffffffff;

		ob.write((const char*)&ibes, sizeof(RoF2::structs::WornEffectStruct));

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

		RoF2::structs::ItemQuaternaryBodyStruct iqbs;
		memset(&iqbs, 0, sizeof(RoF2::structs::ItemQuaternaryBodyStruct));

		iqbs.scriptfileid = item->ScriptFileID;
		iqbs.quest_item = item->QuestItemFlag;
		iqbs.Power = 0;
		iqbs.Purity = item->Purity;
		iqbs.unknown16 = 0;
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

		//unknown18;	//Power Source Capacity or evolve filename?
		//evolve_string; // Some String, but being evolution related is just a guess

		iqbs.Heirloom = 0;
		iqbs.Placeable = 0;
		iqbs.unknown28 = -1;
		iqbs.unknown29 = packet_type == ItemPacketInvalid ? 0xFF : 0;
		iqbs.unknown30 = -1;
		iqbs.NoZone = 0;
		iqbs.NoGround = 0;
		iqbs.unknown37a = 0;	// (guessed position) New to RoF2
		iqbs.unknown38 = 0;
		iqbs.unknown39 = packet_type == ItemPacketInvalid ? 0 : 1;;

		ob.write((const char*)&iqbs, sizeof(RoF2::structs::ItemQuaternaryBodyStruct));

		EQ::OutBuffer::pos_type count_pos = ob.tellp();
		uint32 subitem_count = 0;

		ob.write((const char*)&subitem_count, sizeof(uint32));

		// moved outside of loop since it is not modified within that scope
		int16 SubSlotNumber = EQ::invbag::SLOT_INVALID;

		if (slot_id_in <= EQ::invslot::GENERAL_END && slot_id_in >= EQ::invslot::GENERAL_BEGIN)
			SubSlotNumber = EQ::invbag::GENERAL_BAGS_BEGIN + ((slot_id_in - EQ::invslot::GENERAL_BEGIN) * EQ::invbag::SLOT_COUNT);
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

				SerializeItem(ob, sub, SubSlotNumber, (depth + 1), packet_type);
				++subitem_count;
			}

			if (subitem_count)
				ob.overwrite(count_pos, (const char*)&subitem_count, sizeof(uint32));
		}
	}

	static inline structs::InventorySlot_Struct ServerToRoF2Slot(uint32 server_slot)
	{
		structs::InventorySlot_Struct RoF2Slot;
		RoF2Slot.Type = invtype::TYPE_INVALID;
		RoF2Slot.Unknown02 = INULL;
		RoF2Slot.Slot = invslot::SLOT_INVALID;
		RoF2Slot.SubIndex = invbag::SLOT_INVALID;
		RoF2Slot.AugIndex = invaug::SOCKET_INVALID;
		RoF2Slot.Unknown01 = INULL;

		uint32 TempSlot = EQ::invslot::SLOT_INVALID;

		if (server_slot < EQ::invtype::POSSESSIONS_SIZE) {
			RoF2Slot.Type = invtype::typePossessions;
			RoF2Slot.Slot = server_slot;
		}

		else if (server_slot <= EQ::invbag::CURSOR_BAG_END && server_slot >= EQ::invbag::GENERAL_BAGS_BEGIN) {
			TempSlot = server_slot - EQ::invbag::GENERAL_BAGS_BEGIN;

			RoF2Slot.Type = invtype::typePossessions;
			RoF2Slot.Slot = invslot::GENERAL_BEGIN + (TempSlot / EQ::invbag::SLOT_COUNT);
			RoF2Slot.SubIndex = TempSlot - ((RoF2Slot.Slot - invslot::GENERAL_BEGIN) * EQ::invbag::SLOT_COUNT);
		}

		else if (server_slot <= EQ::invslot::TRIBUTE_END && server_slot >= EQ::invslot::TRIBUTE_BEGIN) {
			RoF2Slot.Type = invtype::typeTribute;
			RoF2Slot.Slot = server_slot - EQ::invslot::TRIBUTE_BEGIN;
		}

		else if (server_slot <= EQ::invslot::GUILD_TRIBUTE_END && server_slot >= EQ::invslot::GUILD_TRIBUTE_BEGIN) {
			RoF2Slot.Type = invtype::typeGuildTribute;
			RoF2Slot.Slot = server_slot - EQ::invslot::GUILD_TRIBUTE_BEGIN;
		}

		else if (server_slot == EQ::invslot::SLOT_TRADESKILL_EXPERIMENT_COMBINE) {
			RoF2Slot.Type = invtype::typeWorld;
		}

		else if (server_slot <= EQ::invslot::BANK_END && server_slot >= EQ::invslot::BANK_BEGIN) {
			RoF2Slot.Type = invtype::typeBank;
			RoF2Slot.Slot = server_slot - EQ::invslot::BANK_BEGIN;
		}

		else if (server_slot <= EQ::invbag::BANK_BAGS_END && server_slot >= EQ::invbag::BANK_BAGS_BEGIN) {
			TempSlot = server_slot - EQ::invbag::BANK_BAGS_BEGIN;

			RoF2Slot.Type = invtype::typeBank;
			RoF2Slot.Slot = TempSlot / EQ::invbag::SLOT_COUNT;
			RoF2Slot.SubIndex = TempSlot - (RoF2Slot.Slot * EQ::invbag::SLOT_COUNT);
		}

		else if (server_slot <= EQ::invslot::SHARED_BANK_END && server_slot >= EQ::invslot::SHARED_BANK_BEGIN) {
			RoF2Slot.Type = invtype::typeSharedBank;
			RoF2Slot.Slot = server_slot - EQ::invslot::SHARED_BANK_BEGIN;
		}

		else if (server_slot <= EQ::invbag::SHARED_BANK_BAGS_END && server_slot >= EQ::invbag::SHARED_BANK_BAGS_BEGIN) {
			TempSlot = server_slot - EQ::invbag::SHARED_BANK_BAGS_BEGIN;

			RoF2Slot.Type = invtype::typeSharedBank;
			RoF2Slot.Slot = TempSlot / EQ::invbag::SLOT_COUNT;
			RoF2Slot.SubIndex = TempSlot - (RoF2Slot.Slot * EQ::invbag::SLOT_COUNT);
		}

		else if (server_slot <= EQ::invslot::TRADE_END && server_slot >= EQ::invslot::TRADE_BEGIN) {
			RoF2Slot.Type = invtype::typeTrade;
			RoF2Slot.Slot = server_slot - EQ::invslot::TRADE_BEGIN;
		}

		else if (server_slot <= EQ::invbag::TRADE_BAGS_END && server_slot >= EQ::invbag::TRADE_BAGS_BEGIN) {
			TempSlot = server_slot - EQ::invbag::TRADE_BAGS_BEGIN;

			RoF2Slot.Type = invtype::typeTrade;
			RoF2Slot.Slot = TempSlot / EQ::invbag::SLOT_COUNT;
			RoF2Slot.SubIndex = TempSlot - (RoF2Slot.Slot * EQ::invbag::SLOT_COUNT);
		}

		else if (server_slot <= EQ::invslot::WORLD_END && server_slot >= EQ::invslot::WORLD_BEGIN) {
			RoF2Slot.Type = invtype::typeWorld;
			RoF2Slot.Slot = server_slot - EQ::invslot::WORLD_BEGIN;
		}

		Log(Logs::Detail, Logs::Netcode, "Convert Server Slot %i to RoF2 Slot [%i, %i, %i, %i] (unk2: %i, unk1: %i)",
			server_slot, RoF2Slot.Type, RoF2Slot.Slot, RoF2Slot.SubIndex, RoF2Slot.AugIndex, RoF2Slot.Unknown02, RoF2Slot.Unknown01);

		return RoF2Slot;
	}

	static inline structs::InventorySlot_Struct ServerToRoF2CorpseSlot(uint32 server_corpse_slot)
	{
		structs::InventorySlot_Struct RoF2Slot;
		RoF2Slot.Type = invtype::TYPE_INVALID;
		RoF2Slot.Unknown02 = INULL;
		RoF2Slot.Slot = ServerToRoF2CorpseMainSlot(server_corpse_slot);
		RoF2Slot.SubIndex = invbag::SLOT_INVALID;
		RoF2Slot.AugIndex = invaug::SOCKET_INVALID;
		RoF2Slot.Unknown01 = INULL;

		if (RoF2Slot.Slot != invslot::SLOT_INVALID)
			RoF2Slot.Type = invtype::typeCorpse;

		Log(Logs::Detail, Logs::Netcode, "Convert Server Corpse Slot %i to RoF2 Corpse Slot [%i, %i, %i, %i] (unk2: %i, unk1: %i)",
			server_corpse_slot, RoF2Slot.Type, RoF2Slot.Slot, RoF2Slot.SubIndex, RoF2Slot.AugIndex, RoF2Slot.Unknown02, RoF2Slot.Unknown01);

		return RoF2Slot;
	}

	static inline uint32 ServerToRoF2CorpseMainSlot(uint32 server_corpse_slot)
	{
		uint32 RoF2Slot = invslot::SLOT_INVALID;

		if (server_corpse_slot <= EQ::invslot::CORPSE_END && server_corpse_slot >= EQ::invslot::CORPSE_BEGIN) {
			RoF2Slot = server_corpse_slot;
		}

		LogNetcode("Convert Server Corpse Slot [{}] to RoF2 Corpse Main Slot [{}]", server_corpse_slot, RoF2Slot);

		return RoF2Slot;
	}

	static inline structs::TypelessInventorySlot_Struct ServerToRoF2TypelessSlot(uint32 server_slot, int16 server_type)
	{
		structs::TypelessInventorySlot_Struct RoF2Slot;
		RoF2Slot.Slot = invslot::SLOT_INVALID;
		RoF2Slot.SubIndex = invbag::SLOT_INVALID;
		RoF2Slot.AugIndex = invaug::SOCKET_INVALID;
		RoF2Slot.Unknown01 = INULL;

		uint32 TempSlot = EQ::invslot::SLOT_INVALID;

		if (server_type == EQ::invtype::typePossessions) {
			if (server_slot < EQ::invtype::POSSESSIONS_SIZE) {
				RoF2Slot.Slot = server_slot;
			}

			else if (server_slot <= EQ::invbag::CURSOR_BAG_END && server_slot >= EQ::invbag::GENERAL_BAGS_BEGIN) {
				TempSlot = server_slot - EQ::invbag::GENERAL_BAGS_BEGIN;

				RoF2Slot.Slot = invslot::GENERAL_BEGIN + (TempSlot / EQ::invbag::SLOT_COUNT);
				RoF2Slot.SubIndex = TempSlot - ((RoF2Slot.Slot - invslot::GENERAL_BEGIN) * EQ::invbag::SLOT_COUNT);
			}
		}

		Log(Logs::Detail, Logs::Netcode, "Convert Server Slot %i to RoF2 Typeless Slot [%i, %i, %i] (implied type: %i, unk1: %i)",
			server_slot, RoF2Slot.Slot, RoF2Slot.SubIndex, RoF2Slot.AugIndex, server_type, RoF2Slot.Unknown01);

		return RoF2Slot;
	}

	static inline uint32 RoF2ToServerSlot(structs::InventorySlot_Struct rof2_slot)
	{
		if (rof2_slot.AugIndex < invaug::SOCKET_INVALID || rof2_slot.AugIndex >= invaug::SOCKET_COUNT) {
			Log(Logs::Detail, Logs::Netcode, "Convert RoF2 Slot [%i, %i, %i, %i] (unk2: %i, unk1: %i) to Server Slot %i",
				rof2_slot.Type, rof2_slot.Slot, rof2_slot.SubIndex, rof2_slot.AugIndex, rof2_slot.Unknown02, rof2_slot.Unknown01, EQ::invslot::SLOT_INVALID);

			return EQ::invslot::SLOT_INVALID;
		}

		uint32 server_slot = EQ::invslot::SLOT_INVALID;
		uint32 temp_slot = invslot::SLOT_INVALID;

		switch (rof2_slot.Type) {
		case invtype::typePossessions: {
			if (rof2_slot.Slot >= invslot::POSSESSIONS_BEGIN && rof2_slot.Slot <= invslot::POSSESSIONS_END) {
				if (rof2_slot.SubIndex == invbag::SLOT_INVALID) {
					server_slot = rof2_slot.Slot;
				}

				else if (rof2_slot.SubIndex >= invbag::SLOT_BEGIN && rof2_slot.SubIndex <= invbag::SLOT_END) {
					if (rof2_slot.Slot < invslot::GENERAL_BEGIN)
						return EQ::invslot::SLOT_INVALID;

					temp_slot = (rof2_slot.Slot - invslot::GENERAL_BEGIN) * invbag::SLOT_COUNT;
					server_slot = EQ::invbag::GENERAL_BAGS_BEGIN + temp_slot + rof2_slot.SubIndex;
				}
			}

			break;
		}
		case invtype::typeBank: {
			if (rof2_slot.Slot >= invslot::SLOT_BEGIN && rof2_slot.Slot < invtype::BANK_SIZE) {
				if (rof2_slot.SubIndex == invbag::SLOT_INVALID) {
					server_slot = EQ::invslot::BANK_BEGIN + rof2_slot.Slot;
				}

				else if (rof2_slot.SubIndex >= invbag::SLOT_BEGIN && rof2_slot.SubIndex <= invbag::SLOT_END) {
					temp_slot = rof2_slot.Slot * invbag::SLOT_COUNT;
					server_slot = EQ::invbag::BANK_BAGS_BEGIN + temp_slot + rof2_slot.SubIndex;
				}
			}

			break;
		}
		case invtype::typeSharedBank: {
			if (rof2_slot.Slot >= invslot::SLOT_BEGIN && rof2_slot.Slot < invtype::SHARED_BANK_SIZE) {
				if (rof2_slot.SubIndex == invbag::SLOT_INVALID) {
					server_slot = EQ::invslot::SHARED_BANK_BEGIN + rof2_slot.Slot;
				}

				else if (rof2_slot.SubIndex >= invbag::SLOT_BEGIN && rof2_slot.SubIndex <= invbag::SLOT_END) {
					temp_slot = rof2_slot.Slot * invbag::SLOT_COUNT;
					server_slot = EQ::invbag::SHARED_BANK_BAGS_BEGIN + temp_slot + rof2_slot.SubIndex;
				}
			}

			break;
		}
		case invtype::typeTrade: {
			if (rof2_slot.Slot >= invslot::SLOT_BEGIN && rof2_slot.Slot < invtype::TRADE_SIZE) {
				if (rof2_slot.SubIndex == invbag::SLOT_INVALID) {
					server_slot = EQ::invslot::TRADE_BEGIN + rof2_slot.Slot;
				}

				else if (rof2_slot.SubIndex >= invbag::SLOT_BEGIN && rof2_slot.SubIndex <= invbag::SLOT_END) {
					temp_slot = rof2_slot.Slot * invbag::SLOT_COUNT;
					server_slot = EQ::invbag::TRADE_BAGS_BEGIN + temp_slot + rof2_slot.SubIndex;
				}
			}

			break;
		}
		case invtype::typeWorld: {
			if (rof2_slot.Slot >= invslot::SLOT_BEGIN && rof2_slot.Slot < invtype::WORLD_SIZE) {
				server_slot = EQ::invslot::WORLD_BEGIN + rof2_slot.Slot;
			}

			else if (rof2_slot.Slot == invslot::SLOT_INVALID) {
				server_slot = EQ::invslot::SLOT_TRADESKILL_EXPERIMENT_COMBINE;
			}

			break;
		}
		case invtype::typeLimbo: {
			if (rof2_slot.Slot >= invslot::SLOT_BEGIN && rof2_slot.Slot < invtype::LIMBO_SIZE) {
				server_slot = EQ::invslot::slotCursor;
			}

			break;
		}
		case invtype::typeTribute: {
			if (rof2_slot.Slot >= invslot::SLOT_BEGIN && rof2_slot.Slot < invtype::TRIBUTE_SIZE) {
				server_slot = EQ::invslot::TRIBUTE_BEGIN + rof2_slot.Slot;
			}

			break;
		}
		case invtype::typeGuildTribute: {
			if (rof2_slot.Slot >= invslot::SLOT_BEGIN && rof2_slot.Slot < invtype::GUILD_TRIBUTE_SIZE) {
				server_slot = EQ::invslot::GUILD_TRIBUTE_BEGIN + rof2_slot.Slot;
			}

			break;
		}
		case invtype::typeCorpse: {
			if (rof2_slot.Slot >= invslot::CORPSE_BEGIN && rof2_slot.Slot <= invslot::CORPSE_END) {
				server_slot = rof2_slot.Slot;
			}

			break;
		}
		default: {

			break;
		}
		}

		Log(Logs::Detail, Logs::Netcode, "Convert RoF2 Slot [%i, %i, %i, %i] (unk2: %i, unk1: %i) to Server Slot %i",
			rof2_slot.Type, rof2_slot.Slot, rof2_slot.SubIndex, rof2_slot.AugIndex, rof2_slot.Unknown02, rof2_slot.Unknown01, server_slot);

		return server_slot;
	}

	static inline uint32 RoF2ToServerCorpseSlot(structs::InventorySlot_Struct rof2_corpse_slot)
	{
		uint32 ServerSlot = EQ::invslot::SLOT_INVALID;

		if (rof2_corpse_slot.Type != invtype::typeCorpse || rof2_corpse_slot.SubIndex != invbag::SLOT_INVALID || rof2_corpse_slot.AugIndex != invaug::SOCKET_INVALID) {
			ServerSlot = EQ::invslot::SLOT_INVALID;
		}

		else {
			ServerSlot = RoF2ToServerCorpseMainSlot(rof2_corpse_slot.Slot);
		}

		Log(Logs::Detail, Logs::Netcode, "Convert RoF2 Slot [%i, %i, %i, %i] (unk2: %i, unk1: %i) to Server Slot %i",
			rof2_corpse_slot.Type, rof2_corpse_slot.Slot, rof2_corpse_slot.SubIndex, rof2_corpse_slot.AugIndex, rof2_corpse_slot.Unknown02, rof2_corpse_slot.Unknown01, ServerSlot);

		return ServerSlot;
	}

	static inline uint32 RoF2ToServerCorpseMainSlot(uint32 rof2_corpse_slot)
	{
		uint32 ServerSlot = EQ::invslot::SLOT_INVALID;

		if (rof2_corpse_slot <= invslot::CORPSE_END && rof2_corpse_slot >= invslot::CORPSE_BEGIN) {
			ServerSlot = rof2_corpse_slot;
		}

		LogNetcode("Convert RoF2 Corpse Main Slot [{}] to Server Corpse Slot [{}]", rof2_corpse_slot, ServerSlot);

		return ServerSlot;
	}

	static inline uint32 RoF2ToServerTypelessSlot(structs::TypelessInventorySlot_Struct rof2_slot, int16 rof2_type)
	{
		if (rof2_slot.AugIndex < invaug::SOCKET_INVALID || rof2_slot.AugIndex >= invaug::SOCKET_COUNT) {
			Log(Logs::Detail, Logs::Netcode, "Convert RoF2 Typeless Slot [%i, %i, %i] (implied type: %i, unk1: %i) to Server Slot %i",
				rof2_slot.Slot, rof2_slot.SubIndex, rof2_slot.AugIndex, rof2_type, rof2_slot.Unknown01, EQ::invslot::SLOT_INVALID);

			return EQ::invslot::SLOT_INVALID;
		}

		uint32 ServerSlot = EQ::invslot::SLOT_INVALID;
		uint32 TempSlot = invslot::SLOT_INVALID;

		switch (rof2_type) {
		case invtype::typePossessions: {
			if (rof2_slot.Slot >= invslot::POSSESSIONS_BEGIN && rof2_slot.Slot <= invslot::POSSESSIONS_END) {
				if (rof2_slot.SubIndex == invbag::SLOT_INVALID) {
					ServerSlot = rof2_slot.Slot;
				}

				else if (rof2_slot.SubIndex >= invbag::SLOT_BEGIN && rof2_slot.SubIndex <= invbag::SLOT_END) {
					if (rof2_slot.Slot < invslot::GENERAL_BEGIN)
						return EQ::invslot::SLOT_INVALID;

					TempSlot = (rof2_slot.Slot - invslot::GENERAL_BEGIN) * invbag::SLOT_COUNT;
					ServerSlot = EQ::invbag::GENERAL_BAGS_BEGIN + TempSlot + rof2_slot.SubIndex;
				}
			}

			break;
		}
		case invtype::typeBank: {
			if (rof2_slot.Slot >= invslot::SLOT_BEGIN && rof2_slot.Slot < invtype::BANK_SIZE) {
				if (rof2_slot.SubIndex == invbag::SLOT_INVALID) {
					ServerSlot = EQ::invslot::BANK_BEGIN + rof2_slot.Slot;
				}

				else if (rof2_slot.SubIndex >= invbag::SLOT_BEGIN && rof2_slot.SubIndex <= invbag::SLOT_END) {
					TempSlot = rof2_slot.Slot * invbag::SLOT_COUNT;
					ServerSlot = EQ::invbag::BANK_BAGS_BEGIN + TempSlot + rof2_slot.SubIndex;
				}
			}

			break;
		}
		case invtype::typeSharedBank: {
			if (rof2_slot.Slot >= invslot::SLOT_BEGIN && rof2_slot.Slot < invtype::SHARED_BANK_SIZE) {
				if (rof2_slot.SubIndex == invbag::SLOT_INVALID) {
					ServerSlot = EQ::invslot::SHARED_BANK_BEGIN + rof2_slot.Slot;
				}

				else if (rof2_slot.SubIndex >= invbag::SLOT_BEGIN && rof2_slot.SubIndex <= invbag::SLOT_END) {
					TempSlot = rof2_slot.Slot * invbag::SLOT_COUNT;
					ServerSlot = EQ::invbag::SHARED_BANK_BAGS_BEGIN + TempSlot + rof2_slot.SubIndex;
				}
			}

			break;
		}
		case invtype::typeTrade: {
			if (rof2_slot.Slot >= invslot::SLOT_BEGIN && rof2_slot.Slot < invtype::TRADE_SIZE) {
				if (rof2_slot.SubIndex == invbag::SLOT_INVALID) {
					ServerSlot = EQ::invslot::TRADE_BEGIN + rof2_slot.Slot;
				}

				else if (rof2_slot.SubIndex >= invbag::SLOT_BEGIN && rof2_slot.SubIndex <= invbag::SLOT_END) {
					TempSlot = rof2_slot.Slot * invbag::SLOT_COUNT;
					ServerSlot = EQ::invbag::TRADE_BAGS_BEGIN + TempSlot + rof2_slot.SubIndex;
				}
			}

			break;
		}
		case invtype::typeWorld: {
			if (rof2_slot.Slot >= invslot::SLOT_BEGIN && rof2_slot.Slot < invtype::WORLD_SIZE) {
				ServerSlot = EQ::invslot::WORLD_BEGIN + rof2_slot.Slot;
			}

			else if (rof2_slot.Slot == invslot::SLOT_INVALID) {
				ServerSlot = EQ::invslot::SLOT_TRADESKILL_EXPERIMENT_COMBINE;
			}

			break;
		}
		case invtype::typeLimbo: {
			if (rof2_slot.Slot >= invslot::SLOT_BEGIN && rof2_slot.Slot < invtype::LIMBO_SIZE) {
				ServerSlot = EQ::invslot::slotCursor;
			}

			break;
		}
		case invtype::typeTribute: {
			if (rof2_slot.Slot >= invslot::SLOT_BEGIN && rof2_slot.Slot < invtype::TRIBUTE_SIZE) {
				ServerSlot = EQ::invslot::TRIBUTE_BEGIN + rof2_slot.Slot;
			}

			break;
		}
		case invtype::typeGuildTribute: {
			if (rof2_slot.Slot >= invslot::SLOT_BEGIN && rof2_slot.Slot < invtype::GUILD_TRIBUTE_SIZE) {
				ServerSlot = EQ::invslot::GUILD_TRIBUTE_BEGIN + rof2_slot.Slot;
			}

			break;
		}
		case invtype::typeCorpse: {
			if (rof2_slot.Slot >= invslot::CORPSE_BEGIN && rof2_slot.Slot <= invslot::CORPSE_END) {
				ServerSlot = rof2_slot.Slot;
			}

			break;
		}
		default: {

			break;
		}
		}

		Log(Logs::Detail, Logs::Netcode, "Convert RoF2 Typeless Slot [%i, %i, %i] (implied type: %i, unk1: %i) to Server Slot %i",
			rof2_slot.Slot, rof2_slot.SubIndex, rof2_slot.AugIndex, rof2_type, rof2_slot.Unknown01, ServerSlot);

		return ServerSlot;
	}

	static inline void ServerToRoF2SayLink(std::string &rof2_saylink, const std::string &server_saylink)
	{
		if ((constants::SAY_LINK_BODY_SIZE == EQ::constants::SAY_LINK_BODY_SIZE) || (server_saylink.find('\x12') == std::string::npos)) {
			rof2_saylink = server_saylink;
			return;
		}

		auto segments = Strings::Split(server_saylink, '\x12');

		for (size_t segment_iter = 0; segment_iter < segments.size(); ++segment_iter) {
			if (segment_iter & 1) {
				if (segments[segment_iter].length() <= EQ::constants::SAY_LINK_BODY_SIZE) {
					rof2_saylink.append(segments[segment_iter]);
					// TODO: log size mismatch error
					continue;
				}

				// Idx:  0 1     6     11    16    21    26    31    36 37   41 43    48       (Source)
				// RoF2: X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX X  XXXX XX XXXXX XXXXXXXX (56)
				// RoF2: X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX X  XXXX XX XXXXX XXXXXXXX (56)
				// Diff:

				rof2_saylink.push_back('\x12');
				rof2_saylink.append(segments[segment_iter]);
				rof2_saylink.push_back('\x12');
			}
			else {
				rof2_saylink.append(segments[segment_iter]);
			}
		}
	}

	static inline void RoF2ToServerSayLink(std::string &server_saylink, const std::string &rof2_saylink)
	{
		if ((EQ::constants::SAY_LINK_BODY_SIZE == constants::SAY_LINK_BODY_SIZE) || (rof2_saylink.find('\x12') == std::string::npos)) {
			server_saylink = rof2_saylink;
			return;
		}

		auto segments = Strings::Split(rof2_saylink, '\x12');

		for (size_t segment_iter = 0; segment_iter < segments.size(); ++segment_iter) {
			if (segment_iter & 1) {
				if (segments[segment_iter].length() <= constants::SAY_LINK_BODY_SIZE) {
					server_saylink.append(segments[segment_iter]);
					// TODO: log size mismatch error
					continue;
				}

				// Idx:  0 1     6     11    16    21    26    31    36 37   41 43    48       (Source)
				// RoF2: X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX X  XXXX XX XXXXX XXXXXXXX (56)
				// RoF2: X XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX XXXXX X  XXXX XX XXXXX XXXXXXXX (56)
				// Diff:

				server_saylink.push_back('\x12');
				server_saylink.append(segments[segment_iter]);
				server_saylink.push_back('\x12');
			}
			else {
				server_saylink.append(segments[segment_iter]);
			}
		}
	}

	static inline spells::CastingSlot ServerToRoF2CastingSlot(EQ::spells::CastingSlot slot)
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

	static inline EQ::spells::CastingSlot RoF2ToServerCastingSlot(spells::CastingSlot slot)
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

	// these should be optimized out for RoF2 since they should all boil down to return index :P
	// but lets leave it here for future proofing
	static inline int ServerToRoF2BuffSlot(int index)
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

	static inline int RoF2ToServerBuffSlot(int index)
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
} /*RoF2*/
