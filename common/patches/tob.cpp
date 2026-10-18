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
#include "common/links.h"
#include "common/say_link.h"
#include "common/misc_functions.h"
#include "common/strings.h"
#include "common/inventory_profile.h"
#include "common/rulesys.h"
#include "common/path_manager.h"
#include "common/classes.h"
#include "common/packet_dump.h"
#include "common/races.h"
#include "common/raid.h"
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

	// ---- ToB wire limits ----
	// The client copies variable-length strings into fixed-size buffers without checking the length, so these are
	// the most characters (not counting the NUL) that are sent for each buffer size.
	static constexpr size_t TOB_NAME_MAX_CHARS         = 63;    // 64-byte name buffers (characters, guilds, mercs, buyers, raid members)
	static constexpr size_t TOB_LINE_MAX_CHARS         = 127;   // 128-byte buffers (expedition names, task descriptions)
	static constexpr size_t TOB_NOTE_MAX_CHARS         = 255;   // 256-byte buffers (guild notes, LFG comments, lockout event names)
	static constexpr size_t TOB_TEXT_MAX_CHARS         = 1023;  // 1024-byte buffers (raid MOTD, emotes, task item lists)
	static constexpr size_t TOB_GUILD_NAME_MAX_CHARS   = 62;    // the client copies min(strlen + 1, 63) bytes of a guild name
	static constexpr size_t TOB_FINDABLE_NAME_MAX_CHARS      = 62;  // findable NPC name
	static constexpr size_t TOB_FINDABLE_LAST_NAME_MAX_CHARS = 30;  // findable NPC last name
	static constexpr size_t TOB_GUILD_URL_FIELD_SIZE   = 0x200;     // guild URL / channel text field in OP_GuildUpdate
	static constexpr size_t TOB_GUILD_URL_MAX_CHARS    = TOB_GUILD_URL_FIELD_SIZE - 1;
	static constexpr size_t TOB_GUILD_CHANNEL_MAX_CHARS = 127;      // the client copies 0x80 bytes of the guild channel
	static constexpr uint32 TOB_GUILD_MEMBER_LIST_MAX  = 10000;     // sanity cap on the member count of a guild member list
	static constexpr uint32 TOB_LFGUILD_MAX_RESULTS    = 1000;      // rows sent for an LFGuild search
	static constexpr uint32 TOB_LFGUILD_MAX_CLASS      = 17;        // highest class id in the client's class-name table (18 entries)
	static constexpr int32  TOB_TIMEZONE_NONE          = -1;        // LFGuild timezone: not set
	static constexpr int32  TOB_TIMEZONE_MAX           = 23;        // highest LFGuild timezone-name table index
	static constexpr uint32 TOB_TASK_HISTORY_MAX_TASKS      = 50;   // the client keeps at most 50 task history entries
	static constexpr uint32 TOB_TASK_HISTORY_MAX_ACTIVITIES = 20;   // activities per task history entry
	static constexpr uint32 TOB_SET_GROUP_TARGET_MAX_NUMBER = 3;    // the client indexes a 3 entry array with this unchecked
	static constexpr uint32 TOB_INVALID_ID             = 0xFFFFFFFF; // "none" in u32 id / type fields

	static constexpr uint16 EMU_GUILD_NONE_U16 = 0xFFFF; // "no guild" in the 16-bit guild id fields some emu packets carry
	static constexpr uint32 TOB_GUILD_RANK_MIN = 1;      // the client ignores a guild invite rank outside 1..8
	static constexpr uint32 TOB_GUILD_RANK_MAX = 8;
	static constexpr uint32 EMU_GUILD_INVITE_DECLINE_RESPONSE = 9; // OP_GuildInviteAccept response sent on decline
	static constexpr uint32 TOB_GUILD_INVITE_ACCEPT_ACTION  = 1;   // OP_GuildInviteAccept action field
	static constexpr uint32 TOB_GUILD_INVITE_DECLINE_ACTION = 2;
	// The legacy group update carries no levels, so members are shown with placeholder levels.
	static constexpr uint32 TOB_GROUP_UPDATE_SELF_LEVEL  = 0x46;
	static constexpr uint32 TOB_GROUP_UPDATE_OTHER_LEVEL = 0x41;

	// OP_GuildBank actions as the server numbers them (GuildBank* in zone/guild_mgr.h) and as ToB numbers them.
	// From Promote on ToB's numbers are one lower than the server's; the item update is 0 on the wire.
	enum EmuGuildBankAction : uint32 {
		EmuGuildBankItemUpdate  = 1,
		EmuGuildBankPromote     = 3,
		EmuGuildBankViewItem    = 4,
		EmuGuildBankDeposit     = 5,
		EmuGuildBankPermissions = 6,
		EmuGuildBankWithdraw    = 7,
		EmuGuildBankAcknowledge = 10,
	};
	enum TOBGuildBankAction : uint32 {
		TOBGuildBankItemUpdate  = 0,
		TOBGuildBankPromote     = 2,
		TOBGuildBankViewItem    = 3,
		TOBGuildBankDeposit     = 4,
		TOBGuildBankPermissions = 5,
		TOBGuildBankWithdraw    = 6, // split (7) and merge (8) follow
		TOBGuildBankSplitStacks = 7,
		TOBGuildBankMergeStacks = 8,
		TOBGuildBankAcknowledge = 9,
	};

	// OP_PickZoneWindow: the window lists zones, then one zone id for each of up to 6 group members
	static constexpr uint32 TOB_PICK_ZONE_GROUP_MEMBER_ZONES = 6;
	static constexpr uint8  TOB_PICK_ZONE_SHOW_WINDOW        = 1; // first byte of the packet: open the window

	static constexpr uint32 EMU_LFGUILD_HEADER_SIZE = 8;  // emu OP_LFGuild header: u32 command + u32 unknown
	static constexpr uint32 TOB_BUYER_LINE_TRAILER_SIZE = 13; // unused bytes the client appends to each buy line

	// Writes n zero bytes (padding / unknown fixed-width fields).
	static void TOBWriteZeros(SerializeBuffer &buf, uint32 n)
	{
		for (uint32 i = 0; i < n; ++i) { buf.WriteUInt8(0); }
	}

	// Writes n raw bytes.
	static void TOBWriteBytes(SerializeBuffer &buf, const void *data, size_t n)
	{
		auto bytes = static_cast<const uint8 *>(data);
		for (size_t i = 0; i < n; ++i) { buf.WriteUInt8(bytes[i]); }
	}

	// Writes a fixed-width, zero-padded character field. At most width - 1 characters of str are copied, so the
	// field is always NUL-terminated (same result as strn0cpy into a zeroed buffer of that width).
	static void TOBWriteFixedString(SerializeBuffer &buf, const char *str, uint32 width)
	{
		uint32 len = 0;
		while (len + 1 < width && str[len] != '\0') { ++len; }
		for (uint32 i = 0; i < len; ++i) { buf.WriteUInt8(static_cast<uint8>(str[i])); }
		TOBWriteZeros(buf, width - len);
	}

	void SerializeItem(SerializeBuffer &buffer, const EQ::ItemInstance* inst, int16 slot_id, uint8 depth, ItemPacketType packet_type);

	// message link converters
	static void ServerToTOBConvertLinks(std::string& message_out, const std::string& message_in, bool client_tags_brackets = true);
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

	// key ring size the client shows and enforces, 0 when key rings are off
	static inline int32 TOBKeyRingSlots(int ring)
	{
		if (!RuleB(Character, EnableKeyRings)) {
			return 0;
		}

		return std::clamp(RuleI(Character, KeyRingSlots), 0, static_cast<int>(EQ::invslot::KEYRING_CLIENT_SIZE[ring]));
	}

	// Store-feature claims the client checks before a key ring takes an item. The mount ring is always available;
	// the others need their claim id with a value above 0 in the player profile claims array.
	static constexpr uint32 TOB_KEYRING_CLAIMS[] = {
		2008511, // illusion
		2010564, // familiar
		2011970, // Hero's Forge
		2016968, // teleportation
		2071519, // activated items
	};

	// casting slots
	static inline spells::CastingSlot ServerToTOBCastingSlot(EQ::spells::CastingSlot slot);
	static inline EQ::spells::CastingSlot TOBToServerCastingSlot(spells::CastingSlot slot);

	// buff slots
	static inline int TOBToServerBuffSlot(int index);

	// guild ids
	static constexpr uint32 TOB_SERVER_GUILD_NONE = 0xFFFFFFFF;

	// ToB uses u64 guild ids and 0 == "no guild".
	static inline uint64 ServerToTOBGuildID(uint32 id)
	{
		return (id == TOB_SERVER_GUILD_NONE) ? 0 : static_cast<uint64>(id);
	}

	static inline uint32 TOBToServerGuildID(uint64 id)
	{
		return (id == 0 || id >= TOB_SERVER_GUILD_NONE) ? TOB_SERVER_GUILD_NONE : static_cast<uint32>(id);
	}

	// copy a fixed char field into another, always terminated
	template <size_t N, size_t M>
	static inline void TOBCopyStr(char (&dst)[N], const char (&src)[M])
	{
		strn0cpy(dst, src, std::min(N, M));
	}

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

		eq->inventorySlot = ServerToTOBSlot(emu->inventorySlot);
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
		// TODO: TOB wire format has 8 extra bytes beyond server struct (total 80 bytes):
		//   +0x48  uint32  unknown072  -- 0x37 (55) triggers hardcoded "perfected distiller" message in client;
		//                                0 causes jnz path (loc_1401EFB6D) which likely uses augment_info text
		//   +0x4C  uint32  unknown076  -- "always matches what client sends"; not echoed by current decoder

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
		eq->StringSize = 0; // TODO: set this to 0, but it's a string size
		eq->Lucky = 0; // TODO: set to 1 to message a lucky beg

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

		SerializeBuffer buffer;
		buffer.WriteUInt32(emu->Count);
		for (int i = 0; i < emu->Count; i++)
			buffer.WriteInt32(emu->SpellID[i]);

		buffer.WriteUInt8(emu->Pet);
		buffer.WriteUInt8(emu->Initialise);

		auto outapp = new EQApplicationPacket(OP_BlockedBuffs, std::move(buffer));
		dest->FastQueuePacket(&outapp, ack_req);
		delete __packet;
	}

	// ---- Bazaar / Trader / Barter ----

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

				eq->action    = structs::TOBBuyerActions::BuyerAppearance;
				eq->entity_id = emu->buyer_id;
				eq->enabled   = emu->approval;

				dest->FastQueuePacket(&outapp);
				delete in;

				break;
			}
			case Barter_BuyerItemRemove: {
				auto emu = (BuyerRemoveItem_Struct *) in->pBuffer;

				auto outapp = new EQApplicationPacket(OP_BuyerItems, sizeof(structs::BuyerRemoveItem_Struct));
				auto eq     = (structs::BuyerRemoveItem_Struct *) outapp->pBuffer;

				eq->action  = structs::TOBBuyerActions::BuyerModifyBuyLine;
				eq->slot_id = emu->buy_slot_id;
				eq->toggle  = 0;

				dest->FastQueuePacket(&outapp);
				delete in;

				break;
			}
			case Barter_BuyerInspectBegin: {
				*(uint32 *) in->pBuffer = structs::TOBBuyerActions::BuyerInspectBegin;
				dest->FastQueuePacket(&in);
				break;
			}
			case Barter_BuyerInspectEnd: {
				*(uint32 *) in->pBuffer = structs::TOBBuyerActions::BuyerInspectEnd;
				dest->FastQueuePacket(&in);
				break;
			}
			case Barter_SellerBrowsing: {
				*(uint32 *) in->pBuffer = structs::TOBBuyerActions::BuyerBrowsingBuyLine;
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

				SerializeBuffer buffer;
				buffer.WriteUInt32(structs::TOBBuyerActions::BuyerSearchResults);
				buffer.WriteUInt32(bisr.result_count);
				for (auto const &i: bisr.results) {
					TOBWriteFixedString(buffer, i.item_name, sizeof(structs::BuyerItemSearchResultEntry_Struct::item_name));
					buffer.WriteUInt32(i.item_id);
					buffer.WriteUInt32(i.item_icon);
					buffer.WriteUInt32(0);
				}

				auto outapp = std::make_unique<EQApplicationPacket>(OP_Barter, std::move(buffer));
				dest->QueuePacket(outapp.get());
				delete in;
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
				LogTrading("(TOB) BazaarSearch action <green>[{}]", action);
				std::vector<BazaarSearchResultsFromDB_Struct> results{};
				auto                                          bsms = (BazaarSearchMessaging_Struct *) in->pBuffer;
				EQ::Util::MemoryStreamReader                  ss(
					reinterpret_cast<char *>(bsms->payload),
					in->size - sizeof(BazaarSearchMessaging_Struct)
				);
				cereal::BinaryInputArchive                    ar(ss);
				ar(results);

				SerializeBuffer buffer;
				buffer.WriteUInt32(0);
				buffer.WriteUInt16(results.empty() ? 0 : results[0].trader_zone_id);
				buffer.WriteUInt32(results.empty() ? 0 : results[0].trader_id);
				buffer.WriteUInt32(results.size());

				for (auto const &i: results) {
					buffer.WriteUInt32(i.trader_id);                          //trader ID
					buffer.WriteString(i.serial_number_RoF);                  //serial
					buffer.WriteUInt32(i.cost);                               //cost
					buffer.WriteUInt32(i.stackable ? i.charges : i.count);    //quantity
					buffer.WriteUInt32(i.item_id);                            //ID
					buffer.WriteUInt32(i.icon_id);                            //icon
					buffer.WriteString(i.item_name);                          //name
					buffer.WriteUInt32(i.item_stat);                          //itemstat
					buffer.WriteFloat(0.0f);                                  //TOB only: unknown float, sent as 0
				}

				auto outapp = new EQApplicationPacket(OP_BazaarSearch, std::move(buffer));
				dest->QueuePacket(outapp);
				delete outapp;
				delete in;

				break;
			}
			case BazaarInspect: {
				LogTrading("(TOB) BazaarInspect action <green>[{}]", action);
				dest->FastQueuePacket(&in, ack_req);
				break;
			}
			case WelcomeMessage: {
				auto buffer = new char[sizeof(structs::BazaarWelcome_Struct)];
				auto emu    = (BazaarWelcome_Struct *) in->pBuffer;
				auto eq     = (structs::BazaarWelcome_Struct *) buffer;

				eq->action         = structs::TOBBazaarTraderBuyerActions::WelcomeMessage;
				eq->num_of_traders = emu->traders;
				eq->num_of_items   = emu->items;

				safe_delete_array(in->pBuffer);
				in->SetOpcode(OP_TraderShop);
				in->size    = sizeof(structs::BazaarWelcome_Struct);
				in->pBuffer = (uchar *)buffer;

				LogTrading("(TOB) WelcomeMessage action <green>[{}]", action);
				dest->QueuePacket(in);
				delete in;

				break;
			}
			case DeliveryCostUpdate: {
				auto data = (BazaarDeliveryCost_Struct *) in->pBuffer;
				LogTrading("(TOB) Delivery costs updated: vouchers <green>[{}] parcel percentage <green>[{}]",
						   data->voucher_delivery_cost,
						   data->parcel_deliver_cost
				);
				data->action = 0;
				dest->FastQueuePacket(&in);
				break;
			}
			default: {
				LogTrading("(TOB) Unhandled action <red>[{}]", action);
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
					"(TOB) TraderOff action <green>[{}] for entity_id <green>[{}]",
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
					"(TOB) TraderOn action <green>[{}] for entity_id <green>[{}]",
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
					"(TOB) AddTraderToBazaarWindow action <green>[{}] trader_id <green>[{}] entity_id <green>[{}] "
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
					"(TOB) RemoveTraderFromBazaarWindow action <green>[{}] for entity_id <green>[{}]",
					eq->action,
					eq->trader_id
				);
				dest->FastQueuePacket(&outapp);
				break;
			}
			default: {
				LogTrading(
					"(TOB) Unhandled action <red>[{}]",
					in->action
				);
				dest->QueuePacket(inapp);
			}
		}
		delete inapp;
	}

	// Body shared by the buy line update (BuyerModifyBuyLine) and buy line inspect (BuyerSendBuyLine) packets.
	static void TOBWriteBuyLine(SerializeBuffer &buffer, uint32 action, uint32 second_field, const BuyerLineItems_Struct &bl)
	{
		buffer.WriteUInt32(action);
		buffer.WriteUInt32(second_field);
		buffer.WriteUInt32(bl.slot);
		buffer.WriteUInt8(bl.enabled ? 1 : 0);
		buffer.WriteUInt32(bl.item_id);
		buffer.WriteString(bl.item_name);
		buffer.WriteUInt32(bl.item_icon);
		buffer.WriteUInt32(bl.item_quantity);
		buffer.WriteUInt8(bl.item_toggle ? 1 : 0);
		buffer.WriteUInt32(bl.item_cost);
		buffer.WriteUInt32(bl.trade_items.size());

		for (auto const &i: bl.trade_items) {
			buffer.WriteUInt32(i.item_id);
			buffer.WriteUInt32(i.item_quantity);
			buffer.WriteUInt32(i.item_icon);
			buffer.WriteString(i.item_name);
		}
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

				SerializeBuffer buffer;
				TOBWriteBuyLine(buffer, structs::TOBBuyerActions::BuyerModifyBuyLine, 0, bl);

				auto outapp = std::make_unique<EQApplicationPacket>(OP_BuyerItems, std::move(buffer));
				dest->QueuePacket(outapp.get());
				delete inapp;
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

				SerializeBuffer buffer;
				TOBWriteBuyLine(buffer, structs::TOBBuyerActions::BuyerSendBuyLine, bli.slot, bli);

				auto packet = std::make_unique<EQApplicationPacket>(OP_BuyerItems, std::move(buffer));
				dest->QueuePacket(packet.get());
				delete inapp;

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
				LogTrading("(TOB) Barter_BuyerSearch action <green>[{}]", emu->action);

				SerializeBuffer buffer;
				buffer.WriteUInt32(1);
				buffer.WriteString(bls.search_string);
				buffer.WriteUInt32(bls.transaction_id);
				buffer.WriteUInt32(0);
				buffer.WriteUInt32(0);
				buffer.WriteUInt8(1);
				buffer.WriteUInt32(bls.no_items);
				for (auto const &b: bls.buy_line) {
					buffer.WriteUInt32(b.slot);
					buffer.WriteUInt8(1);
					buffer.WriteUInt32(b.item_id);
					buffer.WriteString(b.item_name);
					buffer.WriteUInt32(b.item_icon);
					buffer.WriteUInt32(b.item_quantity);
					buffer.WriteUInt8(1);
					buffer.WriteUInt32(b.item_cost);
					buffer.WriteUInt32(b.trade_items.size());
					for (auto const &i: b.trade_items) {
						buffer.WriteUInt32(i.item_id);
						buffer.WriteUInt32(i.item_quantity);
						buffer.WriteUInt32(i.item_icon);
						buffer.WriteString(i.item_name);
					}
					buffer.WriteUInt32(b.buyer_entity_id);
					buffer.WriteUInt32(b.buyer_id);
					buffer.WriteUInt16(b.buyer_zone_id);
					buffer.WriteUInt16(b.buyer_zone_instance_id);
					buffer.WriteString(b.buyer_name);
				}

				auto outapp = std::make_unique<EQApplicationPacket>(OP_BuyerItems, std::move(buffer));
				dest->QueuePacket(outapp.get());
				delete inapp;
				break;
			}
			case Barter_RemoveFromMerchantWindow: {
				auto emu = (BuyerRemoveItemFromMerchantWindow_Struct *) inapp->pBuffer;

				emu->action = structs::TOBBuyerActions::BuyerSendBuyLine;
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

				SerializeBuffer buffer;
				switch (action) {
					case Barter_BuyerTransactionComplete: {
						buffer.WriteUInt32(structs::TOBBuyerActions::BuyerBuyItem);
						break;
					}
					case Barter_SellerTransactionComplete: {
						buffer.WriteUInt32(structs::TOBBuyerActions::BuyerSellItem);
						break;
					}
				}
				buffer.WriteUInt32(blsi.sub_action);
				buffer.WriteUInt32(blsi.error_code);
				TOBWriteZeros(buffer, 16);
				buffer.WriteString(blsi.buyer_name);
				buffer.WriteString(blsi.item_name);
				buffer.WriteString(blsi.seller_name);
				buffer.WriteUInt32(0xFFFFFFFF);
				buffer.WriteUInt32(0xFFFFFFFF);
				TOBWriteZeros(buffer, 1);
				buffer.WriteString(blsi.item_name);
				TOBWriteZeros(buffer, 9);
				buffer.WriteUInt32(blsi.item_cost);
				buffer.WriteUInt32(blsi.trade_items.size());

				for (auto const &i: blsi.trade_items) {
					buffer.WriteUInt32(0);
					buffer.WriteUInt32(i.item_quantity);
					buffer.WriteUInt32(0);
					buffer.WriteString(i.item_name);
				}

				buffer.WriteUInt32(0);
				buffer.WriteUInt32(0);
				buffer.WriteUInt32(0);
				buffer.WriteUInt32(0xFFFFFF);
				buffer.WriteUInt32(0);
				buffer.WriteUInt32(blsi.seller_quantity);

				auto outapp = std::make_unique<EQApplicationPacket>(OP_BuyerItems, std::move(buffer));
				dest->QueuePacket(outapp.get());
				delete inapp;
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

	ENCODE(OP_Charm)
	{
		SETUP_DIRECT_ENCODE(Charm_Struct, structs::Charm_Struct);
		eq->owner_id = emu->owner_id;
		eq->pet_id = emu->pet_id;
		eq->charmer_id = 0; // TOB wire format has extra spawn ID at +0x08; server struct doesn't provide it; passed to sub_1402FA570 when non-null
		eq->command = static_cast<uint8>(emu->command);
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

		SerializeBuffer buffer;
		buffer.WriteString(emu->sender);
		buffer.WriteString(emu->targetname);
		buffer.WriteUInt64(0);	// Unknown
		buffer.WriteUInt32(emu->language);
		buffer.WriteUInt32(emu->chan_num);
		buffer.WriteUInt32(0);	// Unknown
		buffer.WriteUInt8(0);	// Unknown
		buffer.WriteUInt32(emu->skill_in_language);
		buffer.WriteString(new_message);

		buffer.WriteUInt8(0); // Unknown
		buffer.WriteUInt32(0);// Unknown
		buffer.WriteUInt32(0);// Unknown

		buffer.WriteString("");
		buffer.WriteUInt8(0); // Unknown
		buffer.WriteUInt32(0);// Unknown

		in->size = buffer.size();
		in->pBuffer = new unsigned char[buffer.size()];
		memcpy(in->pBuffer, buffer.buffer(), buffer.size());

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
		// TODO: unknown24 is used by the client as ItemDefinition->ItemNumber (item DB ID for the
		// world container). Server struct field is labeled unknown24 and may not be populated.
		// TOB wire: +0x00 drop_id, +0x04 unknown04(-1), +0x08 unknown08(-1), +0x0C type(1B),
		//           +0x10 unknown16(1B, client overwrites to 10), +0x14 icon, +0x18 unknown24/ItemNumber,
		//           +0x1C object_name[64]
		OUT(unknown24);
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
		// source 0 makes ToB's hit-message formatter dereference a null spawn (crash); an unknown id is skipped
		eq->source = emu->source ? emu->source : 0xFFFF;
		OUT(type);
		OUT(spellid);
		// ToB reads damage as a signed int64. Sign-extend: blocks/parries/dodges (-1..-6)
		// were showing as 42949672xx. Damage-shield types (> 0xE7) print "N points of non-melee damage"
		// only for N >= 1, and the server sends DS damage negative (Mob::DamageShield), so flip it.
		{
			int64 dmg = static_cast<int64>(static_cast<int32>(emu->damage));
			if (emu->type > 0xE7 && dmg < 0) {
				dmg = -dmg;
			}
			eq->damage = dmg;
		}
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
		OUT(corpseid);
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
		OUT(Decay);

		FINISH_ENCODE();
	}

	ENCODE(OP_DisciplineTimer)
	{
		ENCODE_LENGTH_EXACT(DisciplineTimer_Struct);
		SETUP_DIRECT_ENCODE(DisciplineTimer_Struct, structs::DisciplineTimer_Struct);

		OUT(TimerID);
		OUT(Duration);
		OUT(Unknown08);
		eq->ServerTime = Timer::GetCurrentTime();

		FINISH_ENCODE();
	}

	ENCODE(OP_DisciplineUpdate)
	{
		ENCODE_LENGTH_EXACT(Disciplines_Struct);
		SETUP_DIRECT_ENCODE(Disciplines_Struct, structs::Disciplines_Struct);

		memcpy(&eq->values, &emu->values, sizeof(Disciplines_Struct));

		FINISH_ENCODE();
	}

	ENCODE(OP_EvolveItem)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		if (in->size < sizeof(EvolveItemMessaging)) {
			LogNetcode("TOB::ENCODE(OP_EvolveItem) packet too small [{}]", in->size);
			delete in;
			return;
		}

		auto emu = reinterpret_cast<EvolveItemMessaging *>(in->pBuffer);

		switch (emu->action) {
			case EvolvingItems::Actions::TRANSFER_WINDOW_DETAILS: {
				EvolveXPWindowSend           e{};
				EQ::Util::MemoryStreamReader ss(emu->serialized_data, in->size - sizeof(emu->action));
				cereal::BinaryInputArchive   ar(ss);
				ar(e);

				const EQ::ItemInstance *item_1 = nullptr;
				const EQ::ItemInstance *item_2 = nullptr;
				if (e.item1_present && e.serialize_item_1.size() >= sizeof(EQ::InternalSerializedItem_Struct)) {
					item_1 = static_cast<const EQ::ItemInstance *>(
						reinterpret_cast<EQ::InternalSerializedItem_Struct *>(e.serialize_item_1.data())->inst);
				}
				if (e.item2_present && e.serialize_item_2.size() >= sizeof(EQ::InternalSerializedItem_Struct)) {
					item_2 = static_cast<const EQ::ItemInstance *>(
						reinterpret_cast<EQ::InternalSerializedItem_Struct *>(e.serialize_item_2.data())->inst);
				}

				// packed header (action, both unique ids, compatibility, max level, a presence byte per item), then the
				// TOB-serialized previews of both items
				SerializeBuffer buf;
				buf.WriteUInt32(e.action);
				buf.WriteUInt64(e.item1_unique_id);
				buf.WriteUInt64(e.item2_unique_id);
				buf.WriteUInt32(e.compatibility);
				buf.WriteUInt32(e.max_transfer_level);
				buf.WriteUInt8(item_1 ? 1 : 0);
				buf.WriteUInt8(item_2 ? 1 : 0);
				if (item_1) {
					SerializeItem(buf, item_1, 0, 0, ItemPacketMerchant);
				}
				if (item_2) {
					SerializeItem(buf, item_2, 0, 0, ItemPacketMerchant);
				}

				auto outapp = new EQApplicationPacket(OP_EvolveItem, std::move(buf));
				dest->FastQueuePacket(&outapp, ack_req);
				delete in;
				break;
			}
			default: {
				// 0 (progress update) and 1 (open the transfer window)
				if (in->size != sizeof(EvolveItemToggle)) {
					LogNetcode("TOB::ENCODE(OP_EvolveItem) action [{}] wrong size [{}]", emu->action, in->size);
					delete in;
					return;
				}

				auto toggle = reinterpret_cast<EvolveItemToggle *>(in->pBuffer);
				auto outapp = new EQApplicationPacket(OP_EvolveItem, sizeof(structs::EvolveItemToggle_Struct));
				auto eq     = reinterpret_cast<structs::EvolveItemToggle_Struct *>(outapp->pBuffer);
				eq->action      = toggle->action;
				eq->unknown_004 = toggle->unknown_004;
				eq->unique_id   = toggle->unique_id;
				eq->percentage  = toggle->percentage;
				eq->activated   = toggle->activated;

				dest->FastQueuePacket(&outapp, ack_req);
				delete in;
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

	ENCODE(OP_ExpUpdate)
	{
		SETUP_DIRECT_ENCODE(ExpUpdate_Struct, structs::ExpUpdate_Struct);

		//later we should change the underlying server to use this more accurate value
		//and encode the 330 in the other patches
		eq->exp = emu->exp * 100000 / 330;

		FINISH_ENCODE();
	}

	ENCODE(OP_FactionList)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		if (in->size < sizeof(FactionList_Struct)) {
			LogNetcode("TOB::ENCODE(OP_FactionList) packet too small [{}]", in->size);
			delete in;
			return;
		}

		auto emu = reinterpret_cast<FactionList_Struct *>(in->pBuffer);
		if (emu->count > (in->size - sizeof(FactionList_Struct)) / sizeof(FactionListEntry_Struct)) {
			LogNetcode("TOB::ENCODE(OP_FactionList) count [{}] does not fit size [{}]", emu->count, in->size);
			delete in;
			return;
		}

		// u32 count, count x {u32 faction id, s32 value}
		SerializeBuffer buf;
		buf.WriteUInt32(emu->count);
		for (uint32 i = 0; i < emu->count; ++i) {
			buf.WriteUInt32(emu->entries[i].faction_id);
			buf.WriteInt32(emu->entries[i].value);
		}

		auto outapp = new EQApplicationPacket(OP_FactionList, std::move(buf));
		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	ENCODE(OP_FactionUpdate)
	{
		ENCODE_LENGTH_EXACT(FactionUpdate_Struct);
		SETUP_DIRECT_ENCODE(FactionUpdate_Struct, structs::FactionUpdate_Struct);

		OUT(show_message);
		OUT(message_type);
		OUT(faction_id);
		OUT(direction);
		OUT(old_value);
		OUT(new_value);

		FINISH_ENCODE();
	}

	ENCODE(OP_FindZoneConnections)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		if (in->size < sizeof(FindZoneConnections_Struct)) {
			LogNetcode("TOB::ENCODE(OP_FindZoneConnections) packet too small [{}]", in->size);
			delete in;
			return;
		}

		auto emu = reinterpret_cast<FindZoneConnections_Struct *>(in->pBuffer);
		if (emu->count > (in->size - sizeof(FindZoneConnections_Struct)) / sizeof(FindZoneConnection_Struct)) {
			LogNetcode("TOB::ENCODE(OP_FindZoneConnections) count [{}] does not fit size [{}]", emu->count, in->size);
			delete in;
			return;
		}

		// u32 count, count x structs::FindZoneConnection_Struct (y, x, z order for the location)
		SerializeBuffer buf;
		buf.WriteUInt32(emu->count);
		for (uint32 i = 0; i < emu->count; ++i) {
			const auto &row = emu->entries[i];
			buf.WriteUInt32(row.row_type);
			buf.WriteUInt32(row.id);
			buf.WriteUInt32(row.unknown08);
			buf.WriteUInt32(row.zone_id);
			buf.WriteUInt32(row.number);
			buf.WriteFloat(row.loc.y);
			buf.WriteFloat(row.loc.x);
			buf.WriteFloat(row.loc.z);
		}

		auto outapp = new EQApplicationPacket(OP_FindZoneConnections, std::move(buf));
		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	ENCODE(OP_Fling)
	{
		SETUP_DIRECT_ENCODE(fling_struct, structs::fling_struct);

		OUT(speed_z);
		OUT(new_y);
		OUT(new_x);
		OUT(new_z);
		eq->radius      = 0.0f;
		eq->unknown     = 0;
		OUT(travel_time);
		eq->collision   = static_cast<uint8>(emu->collision != 0 ? 1 : 0);
		// TODO: verify fall damage semantics — client stores player.408 = (TOB[29] == 0), so
		// direct copy of disable_fall_damage (1=no damage) yields player.408=0; if player.408
		// is a "damage disabled" flag this is correct, but if it is "damage enabled" flip to
		// eq->fall_damage = !emu->disable_fall_damage (needs gameplay test)
		eq->fall_damage = emu->disable_fall_damage;
		eq->z_override  = emu->unk3;

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

	ENCODE(OP_GroupCancelInvite)
	{
		constexpr uint16 GroupCancelUnknown160Value = 0x0100; // the client sends this value in the same field
		ENCODE_LENGTH_EXACT(GroupCancel_Struct);
		SETUP_DIRECT_ENCODE(GroupCancel_Struct, structs::GroupCancel_Struct);

		memcpy(eq->name1, emu->name1, sizeof(eq->name1));
		memcpy(eq->name2, emu->name2, sizeof(eq->name2));
		eq->unknown160 = GroupCancelUnknown160Value;
		OUT(toggle);

		FINISH_ENCODE();
	}

	ENCODE(OP_GroupInvite)
	{
		ENCODE_LENGTH_EXACT(GroupInvite_Struct);
		SETUP_VAR_ENCODE(GroupInvite_Struct);
		// Allocate 4 bytes beyond the struct so the client's read of group_request_id
		// at offset 168 (past the 168-byte struct) lands in valid, zeroed memory.
		// The server has no equivalent field; the client receives 0.
		ALLOC_VAR_ENCODE(structs::GroupGeneric_Struct, sizeof(structs::GroupGeneric_Struct) + sizeof(uint32));

		memcpy(eq->name1, emu->invitee_name, sizeof(eq->name1));
		memcpy(eq->name2, emu->inviter_name, sizeof(eq->name2));
		// TODO: determine what the client expects for group_request_id at offset 168 —
		// it is stored in EverQuest_GroupRequestId and may be used in the accept/decline flow.

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

	ENCODE(OP_Surname)
	{
		ENCODE_LENGTH_EXACT(SurnameReply_Struct);
		EQApplicationPacket *in = *p;
		*p = nullptr;
		auto emu = reinterpret_cast<const SurnameReply_Struct *>(in->pBuffer);

		structs::Surname_Struct header{};
		header.character.Id      = emu->character_id;
		header.character.WorldId = static_cast<uint16>(RuleI(World, Id));
		header.status            = emu->status;
		header.length            = static_cast<uint32>(strnlen(emu->lastname, sizeof(emu->lastname)));

		SerializeBuffer buffer;
		TOBWriteBytes(buffer, &header, sizeof(header));
		TOBWriteBytes(buffer, emu->lastname, header.length);

		auto outapp = new EQApplicationPacket(OP_Surname, std::move(buffer));
		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	ENCODE(OP_RezzRequest)
	{
		ENCODE_LENGTH_EXACT(Resurrect_Struct);
		SETUP_DIRECT_ENCODE(Resurrect_Struct, structs::Resurrect_Struct);
		OUT(unknown000);
		OUT(zone_id);
		OUT(instance_id);
		OUT(y);
		OUT(x);
		OUT(z);
		OUT(unknown020);
		strn0cpy(eq->your_name, emu->your_name, sizeof(eq->your_name));
		eq->unknown088 = emu->unknown088;
		strn0cpy(eq->rezzer_name, emu->rezzer_name, sizeof(eq->rezzer_name));
		OUT(spellid);
		strn0cpy(eq->corpse_name, emu->corpse_name, sizeof(eq->corpse_name));
		OUT(action);
		FINISH_ENCODE();
	}

	ENCODE(OP_RezzComplete)
	{
		ENCODE_LENGTH_EXACT(Resurrect_Struct);
		SETUP_DIRECT_ENCODE(Resurrect_Struct, structs::Resurrect_Struct);
		OUT(unknown000);
		OUT(zone_id);
		OUT(instance_id);
		OUT(y);
		OUT(x);
		OUT(z);
		OUT(unknown020);
		strn0cpy(eq->your_name, emu->your_name, sizeof(eq->your_name));
		eq->unknown088 = emu->unknown088;
		strn0cpy(eq->rezzer_name, emu->rezzer_name, sizeof(eq->rezzer_name));
		OUT(spellid);
		strn0cpy(eq->corpse_name, emu->corpse_name, sizeof(eq->corpse_name));
		OUT(action);
		FINISH_ENCODE();
	}

	// SETUP_DIRECT_ENCODE allocates sizeof(structs::X) and ENCODE_LENGTH_EXACT drops packets whose
	// size != sizeof(emu struct), so nothing reads/writes past either buffer.

	ENCODE(OP_EnduranceUpdate)
	{
		ENCODE_LENGTH_EXACT(EnduranceUpdate_Struct);
		SETUP_DIRECT_ENCODE(EnduranceUpdate_Struct, structs::EnduranceUpdate_Struct);

		OUT(spawn_id);
		OUT(cur_end);
		OUT(max_end);

		FINISH_ENCODE();
	}

	ENCODE(OP_ManaUpdate)
	{
		ENCODE_LENGTH_EXACT(ManaUpdate_Struct);
		SETUP_DIRECT_ENCODE(ManaUpdate_Struct, structs::ManaUpdate_Struct);

		OUT(spawn_id);
		OUT(cur_mana);
		OUT(max_mana);

		FINISH_ENCODE();
	}

	ENCODE(OP_MobEnduranceUpdate)
	{
		ENCODE_LENGTH_EXACT(MobEnduranceUpdate_Struct);
		SETUP_DIRECT_ENCODE(MobEnduranceUpdate_Struct, structs::MobEnduranceUpdate_Struct);

		eq->spawn_id  = static_cast<int16>(emu->spawn_id);
		eq->endurance = emu->endurance; // uint8 -> int32

		FINISH_ENCODE();
	}

	ENCODE(OP_MobManaUpdate)
	{
		ENCODE_LENGTH_EXACT(MobManaUpdate_Struct);
		SETUP_DIRECT_ENCODE(MobManaUpdate_Struct, structs::MobManaUpdate_Struct);

		eq->spawn_id = static_cast<int16>(emu->spawn_id);
		eq->mana     = emu->mana; // uint8 -> int32

		FINISH_ENCODE();
	}

	ENCODE(OP_GroupUpdate)
	{
		// Member list (OP_GroupUpdateB) layout per ToB:
		// header: u32, u32 member_count, str leader; per member: u32 index, str name, u16 merc, str owner,
		// u32 level, u8 x5 role flags, u32 offline, u64 timestamp, u32 unknown, u64 unknown.
		EQApplicationPacket *in = *p;
		GroupJoin_Struct *gjs = (GroupJoin_Struct*)in->pBuffer;

		if ((gjs->action == groupActLeave) || (gjs->action == groupActDisband)) {
			if ((gjs->action == groupActDisband) || !strcmp(gjs->yourname, gjs->membername)) {
				auto outapp = new EQApplicationPacket(OP_GroupDisbandYou, sizeof(structs::GroupGeneric_Struct));
				auto ggs = (structs::GroupGeneric_Struct*)outapp->pBuffer;
				memcpy(ggs->name1, gjs->yourname, sizeof(ggs->name1));
				memcpy(ggs->name2, gjs->membername, sizeof(ggs->name2));
				dest->FastQueuePacket(&outapp);
				delete in;
				return;
			}

			auto outapp = new EQApplicationPacket(OP_GroupDisbandOther, sizeof(structs::GroupGeneric_Struct));
			auto ggs = (structs::GroupGeneric_Struct*)outapp->pBuffer;
			memcpy(ggs->name1, gjs->yourname, sizeof(ggs->name1));
			memcpy(ggs->name2, gjs->membername, sizeof(ggs->name2));
			dest->FastQueuePacket(&outapp);
			delete in;
			return;
		}

		if (in->size == sizeof(GroupUpdate2_Struct)) {
			auto gu2 = (GroupUpdate2_Struct*)in->pBuffer;
			constexpr int other_slots = sizeof(gu2->membername) / sizeof(gu2->membername[0]);
			int member_count = 1;
			for (int i = 0; i < other_slots; ++i) {
				if (gu2->membername[i][0] != '\0') {
					++member_count;
				}
			}

			SerializeBuffer buffer;
			buffer.WriteUInt32(gu2->group_id ? gu2->group_id : structs::GROUP_ID_PLACEHOLDER);
			buffer.WriteUInt32(member_count);
			buffer.WriteString(gu2->leadersname);

			auto write_member = [&buffer](uint32 index, const char* name, uint32 level) {
				buffer.WriteUInt32(index);
				buffer.WriteString(name);
				buffer.WriteUInt16(0);  // merc flag
				buffer.WriteUInt8(0);   // merc owner name (empty string)
				buffer.WriteUInt32(level);
				for (int r = 0; r < 5; ++r)
					buffer.WriteUInt8(0); // role flags (tank, assist, puller, +2 new in ToB)
				buffer.WriteUInt32(0);  // offline
				buffer.WriteUInt64(0);  // timestamp
				buffer.WriteUInt32(0);  // unknown
				buffer.WriteUInt64(0);  // unknown
			};

			write_member(0, gu2->yourname, TOB_GROUP_UPDATE_SELF_LEVEL);
			uint32 member_number = 1;
			for (int i = 0; i < other_slots; ++i) {
				if (gu2->membername[i][0] == '\0')
					continue;
				write_member(member_number++, gu2->membername[i], TOB_GROUP_UPDATE_OTHER_LEVEL);
			}

			auto outapp = new EQApplicationPacket(OP_GroupUpdateB, std::move(buffer));
			dest->FastQueuePacket(&outapp);
			delete in;
			return;
		}

		ENCODE_LENGTH_EXACT(GroupJoin_Struct);
		SETUP_DIRECT_ENCODE(GroupJoin_Struct, structs::GroupJoin_Struct);
		memcpy(eq->membername, emu->membername, sizeof(eq->membername));
		FINISH_ENCODE();
	}

	ENCODE(OP_HPUpdate)
	{
		SETUP_DIRECT_ENCODE(SpawnHPUpdate_Struct, structs::SpawnHPUpdate_Struct);

		OUT(spawn_id);
		// self updates carry HP minus item HP (the client adds item HP back), which goes negative below that
		// amount: sign-extend, or it reaches ToB as ~4 billion and the bar sits at 100% until death
		eq->cur_hp = static_cast<int32>(emu->cur_hp);
		eq->max_hp = static_cast<int32>(emu->max_hp);

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

	ENCODE(OP_IncreaseStats)
	{
		ENCODE_LENGTH_EXACT(IncreaseStat_Struct);
		SETUP_VAR_ENCODE(IncreaseStat_Struct);
		ALLOC_VAR_ENCODE(structs::IncreaseStat_Struct, sizeof(structs::IncreaseStat_Struct));

		// entity_id is stashed in unknown13[0..1] by Client::IncStats/SetStats (zone/client.cpp)
		// because IncreaseStat_Struct has no spawn_id field. The client validates this against
		// g_pLocalPlayer->SpawnID before applying the stat.
		eq->spawn_id = *reinterpret_cast<const uint16 *>(emu->unknown13);

		if      (emu->str)  { eq->stat_type = STAT_STR; eq->value = emu->str; }
		else if (emu->sta)  { eq->stat_type = STAT_STA; eq->value = emu->sta; }
		else if (emu->agi)  { eq->stat_type = STAT_AGI; eq->value = emu->agi; }
		else if (emu->dex)  { eq->stat_type = STAT_DEX; eq->value = emu->dex; }
		else if (emu->int_) { eq->stat_type = STAT_INT; eq->value = emu->int_; }
		else if (emu->wis)  { eq->stat_type = STAT_WIS; eq->value = emu->wis; }
		else if (emu->cha)  { eq->stat_type = STAT_CHA; eq->value = emu->cha; }

		FINISH_ENCODE();
	}

	ENCODE(OP_ItemLinkResponse) { ENCODE_FORWARD(OP_ItemPacket); }

	ENCODE(OP_ItemOverflow)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		if (in->size != sizeof(ItemOverflow_Struct)) {
			LogNetcode("TOB::ENCODE(OP_ItemOverflow) wrong size [{}]", in->size);
			delete in;
			return;
		}

		auto emu = reinterpret_cast<ItemOverflow_Struct *>(in->pBuffer);

		// {u8 4, u64 item index} = row removed, {u8 7} = list cleared
		SerializeBuffer buf;
		switch (emu->action) {
			case ItemOverflowRemoved:
				buf.WriteUInt8(static_cast<uint8>(emu->action));
				buf.WriteUInt64(emu->item_index);
				break;
			case ItemOverflowClear:
				buf.WriteUInt8(static_cast<uint8>(emu->action));
				break;
			default:
				LogNetcode("TOB::ENCODE(OP_ItemOverflow) unknown action [{}]", emu->action);
				delete in;
				return;
		}

		auto outapp = new EQApplicationPacket(OP_ItemOverflow, std::move(buf));
		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	ENCODE(OP_PVPLeaderBoardDetailsReply)
	{
		ENCODE_LENGTH_EXACT(PVPLeaderBoardDetailsReply_Struct);
		SETUP_DIRECT_ENCODE(PVPLeaderBoardDetailsReply_Struct, structs::PVPLeaderBoardDetailsReply_Struct);

		strn0cpy(eq->Name, emu->Name, sizeof(eq->Name));
		memcpy(eq->Unknown064, emu->Unknown064, sizeof(eq->Unknown064));
		OUT(Level);
		OUT(Race);
		OUT(Class);
		eq->GuildID = emu->GuildID;
		OUT(TotalAA);
		OUT(Kills);
		OUT(Deaths);
		OUT(Infamy);
		OUT(Points);

		FINISH_ENCODE();
	}

	ENCODE(OP_PVPStats)
	{
		ENCODE_LENGTH_EXACT(PVPStats_Struct);
		SETUP_DIRECT_ENCODE(PVPStats_Struct, structs::PVPStats_Struct);

		OUT(Kills);
		OUT(Deaths);
		OUT(PVPPointsAvailable);
		OUT(TotalPVPPoints);
		OUT(BestKillStreak);
		OUT(WorstDeathStreak);
		OUT(CurrentKillStreak);
		OUT(Infamy);
		OUT(Vitality);

		static_assert(sizeof(eq->LastDeath) == sizeof(emu->LastDeath), "PvP entry size");
		memcpy(&eq->LastDeath, &emu->LastDeath, sizeof(eq->LastDeath));

		auto copy_entry = [](structs::PVPStatsEntry96_Struct &d, const PVPStatsEntry_Struct &e) {
			strn0cpy(d.Name, e.Name, sizeof(d.Name));
			d.Level  = e.Level;
			d.Race   = e.Race;
			d.Class  = e.Class;
			d.Zone   = e.Zone;
			d.Time   = e.Time;
			d.Points = e.Points;
		};

		copy_entry(eq->LastKill, emu->LastKill);
		constexpr size_t n_eq  = sizeof(eq->KillsLast24Hours) / sizeof(eq->KillsLast24Hours[0]);
		constexpr size_t n_emu = sizeof(emu->KillsLast24Hours) / sizeof(emu->KillsLast24Hours[0]);
		for (size_t i = 0; i < std::min(n_eq, n_emu); ++i) {
			copy_entry(eq->KillsLast24Hours[i], emu->KillsLast24Hours[i]);
		}

		FINISH_ENCODE();
	}

	ENCODE(OP_PickZoneWindow)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		if (in->size < sizeof(PickZoneWindow_Struct)) {
			LogNetcode("TOB::ENCODE(OP_PickZoneWindow) wrong size [{}]", in->size);
			delete in;
			return;
		}

		auto emu = reinterpret_cast<PickZoneWindow_Struct *>(in->pBuffer);

		int count = emu->option_count;
		const int max_entries = static_cast<int>(sizeof(emu->entries) / sizeof(emu->entries[0]));
		count = std::clamp(count, 0, max_entries);

		// serialized: u8 show, u32 session, current zone header (u32 zone, u32 players, u8 locked),
		// u32 count, count x {u32 instance, u32 zone, u32 players, u8 unknown}, 6 x u32 group member zones
		SerializeBuffer buf;
		buf.WriteUInt8(TOB_PICK_ZONE_SHOW_WINDOW);
		buf.WriteUInt32(static_cast<uint32>(emu->session_id));
		buf.WriteUInt32(count > 0 ? static_cast<uint16>(emu->entries[0].zone_id) : 0); // header only
		buf.WriteUInt32(0);
		buf.WriteUInt8(0);
		buf.WriteUInt32(static_cast<uint32>(count));
		for (int i = 0; i < count; ++i) {
			const auto &e = emu->entries[i];
			buf.WriteUInt32(static_cast<uint32>(e.instance_id));
			buf.WriteUInt32(static_cast<uint16>(e.zone_id));
			buf.WriteUInt32(static_cast<uint32>(e.player_count));
			buf.WriteUInt8(0);
		}
		for (uint32 i = 0; i < TOB_PICK_ZONE_GROUP_MEMBER_ZONES; ++i) {
			buf.WriteUInt32(0);
		}

		auto outapp = new EQApplicationPacket(OP_PickZoneWindow, std::move(buf));
		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	ENCODE(OP_ItemPacket)
	{
		EQApplicationPacket* in = *p;
		*p = nullptr;
		uchar* __emu_buffer = in->pBuffer;
		ItemPacket_Struct* old_item_pkt = (ItemPacket_Struct*)__emu_buffer;

		// ItemPacketViewLink (0) is not in the TOB OP_ItemPacket type table.
		// The client shows item windows from link clicks via OP_ItemLinkResponse (0x7805),
		// whose handler sub_1402136D0 reads [int32 type][int32 position][uint32 display_mode]
		// then serialized item, then calls CItemDisplayManager::ShowItem.
		if (old_item_pkt->PacketType == ItemPacketViewLink) {
			EQ::InternalSerializedItem_Struct* int_struct = (EQ::InternalSerializedItem_Struct*)(&__emu_buffer[4]);
			SerializeBuffer buffer;
			buffer.WriteInt32(0);  // type: display mode (0 = default)
			buffer.WriteInt32(0);  // position: window slot (0 = default/new window)
			buffer.WriteUInt32(0); // display_mode: ShowItem arg (0 = default)
			SerializeItem(buffer, (const EQ::ItemInstance*)int_struct->inst, int_struct->slot_id, 0,
				old_item_pkt->PacketType);
			auto outapp = new EQApplicationPacket(OP_ItemLinkResponse, buffer.size());
			outapp->WriteData(buffer.buffer(), buffer.size());
			dest->FastQueuePacket(&outapp, ack_req);
		} else {
			switch (const auto type = ServerToTOBItemPacketType(old_item_pkt->PacketType)) {
			case item::ItemPacketType::ItemPacketInvalid:
				break;
			case item::ItemPacketType::ItemPacketParcel: {
				ParcelMessaging_Struct pms{};
				EQ::Util::MemoryStreamReader ss(reinterpret_cast<char*>(in->pBuffer), in->size);
				cereal::BinaryInputArchive ar(ss);
				ar(pms);

				auto* int_struct = (EQ::InternalSerializedItem_Struct*)pms.serialized_item.data();

				SerializeBuffer buffer;
				buffer.WriteInt32((int32_t)type);
				SerializeItem(buffer, (const EQ::ItemInstance*)int_struct->inst, pms.slot_id, 0, old_item_pkt->PacketType);

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
				break;
			}
			}
		}

		delete in;
	}

	ENCODE(OP_ItemPreviewRequest)
	{
		EQApplicationPacket* in = *p;
		*p = nullptr;
		uchar* in_buf = in->pBuffer;
		auto int_item = (EQ::InternalSerializedItem_Struct*) in_buf;

		SerializeBuffer buf;
		SerializeItem(buf, (const EQ::ItemInstance*) int_item->inst, int_item->slot_id, 0, ItemPacketInvalid);

		auto outapp = new EQApplicationPacket(OP_ItemPreviewRequest, buf.size());
		outapp->WriteData(buf.buffer(), buf.size());
		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	ENCODE(OP_ItemRecastDelay)
	{
		SETUP_DIRECT_ENCODE(ItemRecastDelay_Struct, structs::ItemRecastDelay_Struct);

		// TODO: server struct ItemRecastDelay_Struct needs an ItemGlobalIndex (12 bytes) so the
		// client can locate the item and update its per-item recast timestamp (item+0x14).
		// Until that field is added server-side, item_global_index is zeroed and the item
		// lookup in GetItemByGlobalIndex will fail silently — SetCoreItemRecastTimer (the
		// core recast-by-type timer) will still fire correctly for valid recast_type values.
		eq->item_slot    = {};
		eq->recast_delay = emu->recast_delay;
		eq->recast_type  = emu->recast_type;
		// ignore_casting_requirement has no client equivalent (not read by TOB client)

		FINISH_ENCODE();
	}

	DECODE(OP_ItemLinkClick)
	{
		// TOB sends 84 bytes (InspectItemSafe); first 52 are layout-compatible with ItemViewRequest_Struct.
		// Extra 32 bytes (offsets 52-83) are uncharted — likely luck
		DECODE_LENGTH_ATLEAST(ItemViewRequest_Struct);
		SETUP_DIRECT_DECODE(ItemViewRequest_Struct, ItemViewRequest_Struct);
		MEMSET_IN(ItemViewRequest_Struct);

		IN(item_id);
		for (int r = EQ::invaug::SOCKET_BEGIN; r <= EQ::invaug::SOCKET_END; r++) {
			IN(augments[r]);
		}
		IN(link_hash);
		IN(icon);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ItemLinkResponse)
	{
		// TOB sends 80 bytes; EQEMu LDONItemViewRequest_Struct is 72 bytes.
		// First 72 bytes are layout-identical — extra 8 bytes (unknown072/076) are ignored.
		DECODE_LENGTH_ATLEAST(LDONItemViewRequest_Struct);
		SETUP_DIRECT_DECODE(LDONItemViewRequest_Struct, LDONItemViewRequest_Struct);
		MEMSET_IN(LDONItemViewRequest_Struct);
		IN(item_id);
		memcpy(emu->unknown004, eq->unknown004, sizeof(emu->unknown004));
		strncpy(emu->item_name, eq->item_name, sizeof(emu->item_name) - 1);
		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ItemOverflow)
	{
		// C2S: {u16 0} list, {u16 1, u64 item index} retrieve, {u16 2} retrieve all
		// (structs::ItemOverflowRequest_Struct and structs::ItemOverflowRetrieve_Struct)
		const uint32 size = __packet->size;
		__packet->SetReadPosition(0);

		const uint16 action     = __packet->ReadUInt16();
		uint64       item_index = 0;
		if (action == ItemOverflowRetrieve) {
			item_index = __packet->ReadUInt64();
		}
		const bool action_ok = action == ItemOverflowList || action == ItemOverflowRetrieve ||
			action == ItemOverflowRetrieveAll;
		if (__packet->ReadError() || !action_ok || __packet->ReadRemaining() != 0) {
			LogNetcode("TOB OP_ItemOverflow C2S action [{}] size [{}] not handled", action, size);
			__packet->SetOpcode(OP_Unknown);
			return;
		}

		unsigned char *__eq_buffer = __packet->pBuffer;
		__packet->size    = sizeof(ItemOverflow_Struct);
		__packet->pBuffer = new unsigned char[__packet->size] {};
		auto emu = reinterpret_cast<ItemOverflow_Struct *>(__packet->pBuffer);

		emu->action     = action;
		emu->item_index = item_index;

		delete[] __eq_buffer;
	}

	DECODE(OP_ItemPreviewRequest)
	{
		DECODE_LENGTH_EXACT(structs::ItemPreviewRequest_Struct);
		SETUP_DIRECT_DECODE(ItemPreview_Struct, structs::ItemPreviewRequest_Struct);
		IN(itemid);
		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ItemVerifyRequest)
	{
		DECODE_LENGTH_EXACT(structs::ItemVerifyRequest_Struct);
		SETUP_DIRECT_DECODE(ItemVerifyRequest_Struct, structs::ItemVerifyRequest_Struct);

		emu->slot = TOBToServerSlot(eq->inventory_slot);
		IN(target);

		FINISH_DIRECT_DECODE();
	}

	ENCODE(OP_ItemVerifyReply)
	{
		ENCODE_LENGTH_EXACT(ItemVerifyReply_Struct);
		SETUP_DIRECT_ENCODE(ItemVerifyReply_Struct, structs::ItemVerifyReply_Struct);

		OUT(slot);
		OUT(spell);
		OUT(target);
		// TODO: expand server struct ItemVerifyReply_Struct to support autobook-scribe — TOB wire format (20 bytes):
		//   +0x00  int32   slot        (passed as ItemGlobalIndex* to GetItemByGlobalIndex / IsHeldSlot)
		//   +0x04  uint32  spell       (client reads lower 16 bits; 0x407 = autobook-scribe path)
		//   +0x08  uint32  target
		//   +0x0C  int32   unknown0    (exit gate: handler skips without processing if < 0; send 0)
		//   +0x10  int32   recast_time (fasttime() timestamp; must be non-zero to enter autobook-scribe
		//                               path when spell==0x407; zeroed here until server provides it)
		// unknown0 and recast_time are zeroed by ALLOC_VAR_ENCODE (memset)

		FINISH_ENCODE();
	}

	ENCODE(OP_LinkedReuse)
	{
		SETUP_DIRECT_ENCODE(LinkedSpellReuseTimer_Struct, structs::LinkedSpellReuseTimer_Struct);
		OUT(timer_id);
		eq->unknown = 0;
		OUT(end_time);
		OUT(start_time);
		FINISH_ENCODE();
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

	ENCODE(OP_LootItem)
	{
		ENCODE_LENGTH_EXACT(LootingItem_Struct);
		SETUP_DIRECT_ENCODE(LootingItem_Struct, structs::LootingItem_Struct);

		OUT(lootee);
		OUT(looter);
		eq->slot_id = ServerToTOBCorpseMainSlot(emu->slot_id);
		OUT(auto_loot);
		// TODO: unknown16 appears to be quantity (items looted from a partial stack);
		// server struct has no quantity field so we cannot populate it here
		eq->unknown16 = 0;

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
		buffer.WriteInt32(emu->npc_aggro_max_dist);

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

		if (in->size < sizeof(OnLevelMessage_Struct)) {
			delete in;
			return;
		}

		OnLevelMessage_Struct* emu = (OnLevelMessage_Struct*)in->pBuffer;
		SerializeBuffer buffer;

		// bounded lengths: the emu char arrays are not guaranteed to be NUL-terminated
		buffer.WriteLengthString(strnlen(emu->Title, sizeof(emu->Title)), emu->Title);
		buffer.WriteLengthString(strnlen(emu->Text, sizeof(emu->Text)), emu->Text);
		buffer.WriteLengthString(strnlen(emu->ButtonName0, sizeof(emu->ButtonName0)), emu->ButtonName0);
		buffer.WriteLengthString(strnlen(emu->ButtonName1, sizeof(emu->ButtonName1)), emu->ButtonName1);
		buffer.WriteUInt8(emu->Buttons ? 1 : 0);   // 0 = OK only, 1 = ButtonName0/ButtonName1; >=2 shows all three on ToB
		buffer.WriteUInt8(emu->SoundControls);
		buffer.WriteUInt32(emu->Duration);          // ms, 0 = no auto-close
		buffer.WriteUInt32(emu->PopupID);           // reply {0, PopupID} on OK/left button
		buffer.WriteUInt32(emu->NegativeID);        // reply {1, NegativeID} on right button
		buffer.WriteUInt32(0);                      // TimeoutID: reply {2, id} on timeout or window close; 0 = no reply
		buffer.WriteUInt32(0);                      // chat echo channel (1..31 echoes text to chat); 0 = off

		auto outapp = new EQApplicationPacket(OP_OnLevelMessage, buffer.size());
		outapp->WriteData(buffer.buffer(), buffer.size());
		dest->FastQueuePacket(&outapp, ack_req);

		delete in;
	}

	ENCODE(OP_PickPocket)
	{
		ENCODE_LENGTH_EXACT(sPickPocket_Struct);
		SETUP_VAR_ENCODE(sPickPocket_Struct);

		uint32 nameLen = strnlen(emu->itemname, sizeof(emu->itemname));

		// structs::PickPocket_Struct, then the item name, then one terminating zero byte
		SerializeBuffer buffer;
		buffer.WriteUInt32(emu->to);
		buffer.WriteUInt32(emu->from);
		buffer.WriteUInt32(emu->myskill);
		buffer.WriteUInt8(static_cast<uint8>(emu->type));
		buffer.WriteUInt32(emu->coin);
		buffer.WriteUInt32(nameLen);
		TOBWriteBytes(buffer, emu->itemname, nameLen);
		buffer.WriteUInt8(0); // luckily

		auto outapp = new EQApplicationPacket(OP_PickPocket, std::move(buffer));
		dest->FastQueuePacket(&outapp, ack_req);
		delete __packet;
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
			out.WriteUInt32(i < EQ::textures::materialCount ? emu->item_tint.Slot[i].Color : 0);
		}

		//u32 equip_tint_count;
		out.WriteUInt32(9); //equip_tint_count
		//u32 equip_tints[equip_tint_count];
		for (int i = 0; i < 9; ++i) {
			out.WriteUInt32(i < EQ::textures::materialCount ? emu->item_tint.Slot[i].Color : 0);
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
		// the client copies 62 entries into its long buff slots 0..61, so send all of them
		out.WriteUInt32(spells::CLIENT_LONG_BUFFS);

		//PackedEQAffect buffs[buff_count];
		// these fill the client's local affect array, which the buff window checks before it will send
		// a remove request. Server long slots map 1:1; client long slots past ours stay empty.
		for (int i = 0; i < spells::CLIENT_LONG_BUFFS; ++i) {
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
			constexpr float  BardModifierScale = 10.0f;       // Buffs_Struct::bard_modifier is in tenths (10 = 1.0)
			const bool has_buff = i < EQ::spells::LONG_BUFFS && i < static_cast<int>(BUFF_COUNT) &&
				emu->buffs[i].spellid != 0 && emu->buffs[i].spellid != SPELL_UNKNOWN; // no IsValidSpell: world links this file without spells[]
			if (has_buff) {
				const auto& buff = emu->buffs[i];
				// the client multiplies most effect values by this; 0 would zero the buff
				out.WriteFloat(buff.bard_modifier > 0 ? static_cast<float>(buff.bard_modifier) / BardModifierScale : 1.0f);
				out.WriteUInt32(buff.player_id);
				out.WriteUInt16(static_cast<uint16>(RuleI(World, Id)));
				out.WriteUInt16(0);
				out.WriteUInt32(buff.duration);
				out.WriteUInt32(buff.duration); // the profile has no initial duration
				out.WriteUInt8(buff.level > 0 ? buff.level : emu->level);
				out.WriteInt32(buff.spellid);
				out.WriteInt32(buff.num_hits);
				out.WriteUInt32(0);
				out.WriteUInt32(0);
				out.WriteUInt8(0); // any non-zero type reverses the buff
			} else {
				out.WriteFloat(1.0f);
				out.WriteUInt64(0);
				out.WriteUInt32(0);
				out.WriteUInt32(0);
				out.WriteUInt8(0);
				out.WriteUInt32(TOB_INVALID_ID); // spell id of an empty affect
				out.WriteUInt32(0);
				out.WriteUInt32(0);
				out.WriteUInt32(0);
				out.WriteUInt8(0);
			}
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
		// TODO: Stat Mount / Stat Illusion / Stat Familiar. This is the selected stat item per ring, but the
		// packet the Key Ring window sends to pick one is not known yet (needs a Live capture), so send none.
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

		out.WriteUInt64(ServerToTOBGuildID(emu->guild_id)); // 0 = no guild on ToB
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
		//Claim claims[claim_count]; { u32 claim_id, s32 value }
		{
			// TOB_KEYRING_CLAIMS[i] belongs to ring i + 1 (the mount ring needs no claim)
			std::vector<uint32> claims;
			if (RuleB(Character, EnableKeyRings)) {
				for (size_t i = 0; i < std::size(TOB_KEYRING_CLAIMS); ++i) {
					if (emu->tob_keyring_unlocks & (1 << (i + 1))) {
						claims.push_back(TOB_KEYRING_CLAIMS[i]);
					}
				}
			}

			out.WriteUInt32(static_cast<uint32>(claims.size()));
			for (const auto claim_id : claims) {
				out.WriteUInt32(claim_id);
				out.WriteInt32(1);
			}
		}

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
		// slot 0 = the generic '---' theme; ToB's points-merchant buy check reads "earned" from it
		out.WriteUInt32(emu->ldon_points_available);
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
		out.WriteUInt32(0); // the client reads 6 x u32 then a u64 after the name: 32 bytes

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
			out.WriteUInt32(0); // u64 tail, see last_kill
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

	ENCODE(OP_ReadBook)
	{
		ENCODE_LENGTH_ATLEAST(BookText_Struct);
		SETUP_DIRECT_ENCODE(BookText_Struct, structs::BookRequest_Struct);

		eq->window = (emu->window == 0xFF) ? 0xFFFFFFFF : emu->window;
		OUT(type);
		OUT(target_id);
		eq->invslot = ServerToTOBTypelessSlot(emu->invslot, invtype::typePossessions);
		OUT(can_cast);  // wire 0x13 = note-path cast button
		OUT(can_scribe);  // wire 0x14 = book cast / note scribe button
		strn0cpy(eq->txtfile, emu->booktext, sizeof(eq->txtfile));

		FINISH_ENCODE();
	}

	ENCODE(OP_RecipeAutoCombine)
	{
		ENCODE_LENGTH_EXACT(RecipeAutoCombine_Struct);
		SETUP_DIRECT_ENCODE(RecipeAutoCombine_Struct, structs::RecipeAutoCombine_Struct);

		OUT(object_type);
		OUT(some_id);
		{
			structs::InventorySlot_Struct cslot = ServerToTOBSlot(emu->unknown1);
			eq->container_type       = cslot.Type;
			eq->container_slot_index = cslot.Slot;
			eq->container_subindex   = cslot.SubIndex;
			eq->container_augindex   = cslot.AugIndex;
			// Padding2 intentionally omitted — client deserializer skips it
		}
		structs::InventorySlot_Struct TOBSlot;
		TOBSlot.Type     = 8;	// Observed
		TOBSlot.Slot     = -1;
		TOBSlot.SubIndex = -1;
		TOBSlot.AugIndex = -1;
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

		for (uint32 i = 0; i < MAX_PP_AA_ARRAY; ++i)  // server struct has 240 entries; TOB expects 300, entries 240-299 remain zero
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

	// ---- guild ----
// -----------------------------------------------------------------------------------------------------


// -----------------------------------------------------------------------------------------------------
	ENCODE(OP_GuildMemberList)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		if (in->size < sizeof(Internal_GuildMembers_Struct)) {
			delete in;
			return;
		}

		const uint8 *begin = in->pBuffer;
		auto emu = reinterpret_cast<const Internal_GuildMembers_Struct *>(begin);

		uint32 count = emu->count;
		size_t fixed = sizeof(Internal_GuildMembers_Struct) + static_cast<size_t>(count) * sizeof(Internal_GuildMemberEntry_Struct);
		if (count > TOB_GUILD_MEMBER_LIST_MAX || fixed > in->size) {
			LogNetcode("TOB::ENCODE(OP_GuildMemberList) inconsistent count [{}] for size [{}]", count, in->size);
			count = 0;
			fixed = sizeof(Internal_GuildMembers_Struct);
		}

		// strings follow the fixed entries: count names, then count notes (each NUL-terminated)
		in->SetReadPosition(static_cast<uint32>(fixed));
		std::vector<std::string> names(count);
		for (auto &name : names) {
			in->ReadString(name, TOB_NAME_MAX_CHARS);
		}
		std::vector<std::string> notes(count);
		for (auto &note : notes) {
			in->ReadString(note, TOB_NOTE_MAX_CHARS);
		}

		SerializeBuffer buffer;
		// the client reads (and discards) a length-prefixed name first (the leading player name)
		buffer.WriteLengthString(std::string(emu->player_name, strnlen(emu->player_name, sizeof(emu->player_name))));

		buffer.WriteUInt64(ServerToTOBGuildID(emu->guild_id));
		buffer.WriteUInt8(0);         // unknown
		buffer.WriteUInt8(0);         // auto-evict flag
		buffer.WriteUInt32(count);

		const Internal_GuildMemberEntry_Struct *e = emu->member;
		for (uint32 i = 0; i < count; ++i, ++e) {
			buffer.WriteLengthString(names[i]);
			buffer.WriteUInt32(e->level);
			buffer.WriteUInt32(e->banker);
			buffer.WriteUInt32(e->class_);
			buffer.WriteUInt32(e->rank);
			buffer.WriteUInt32(e->time_last_on);
			buffer.WriteUInt8(e->tribute_enable ? 1 : 0);
			buffer.WriteUInt8(0);                       // unknown
			buffer.WriteUInt32(e->total_tribute);
			buffer.WriteUInt32(e->last_tribute);
			buffer.WriteUInt8(1);                       // unknown, always 1
			buffer.WriteLengthString(notes[i]);
			buffer.WriteUInt32(e->zone_id);             // non-zero = shown online
			buffer.WriteUInt8(1);                       // guild_show, always 1
			buffer.WriteUInt8(0);                       // offline mode
		}

		auto outapp = new EQApplicationPacket(OP_GuildMemberList, buffer.size());
		outapp->WriteData(buffer.buffer(), buffer.size());
		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	ENCODE(OP_GuildMemberAdd)
	{
		ENCODE_LENGTH_EXACT(GuildMemberAdd_Struct);
		SETUP_DIRECT_ENCODE(GuildMemberAdd_Struct, structs::GuildMemberAdd_Struct);

		eq->guild_id   = ServerToTOBGuildID(emu->guild_id);
		eq->level      = emu->level;
		eq->class_     = emu->class_;
		eq->rank       = emu->rank_;
		eq->guild_show = emu->guild_show ? 1 : 0;
		eq->zone_id    = emu->zone_id;
		eq->last_on    = emu->last_on;
		TOBCopyStr(eq->player_name, emu->player_name);

		FINISH_ENCODE();
	}

	ENCODE(OP_GuildMemberLevel)
	{
		// ToB also overwrites the member's class with this packet, so the zone appends a uint32 class
		// after GuildMemberLevel_Struct for ToB clients (EntityList::SendGuildMemberLevel)
		EQApplicationPacket *in = *p;
		if (in->size != sizeof(GuildMemberLevel_Struct) + sizeof(uint32)) {
			LogNetcode("Wrong size on outbound [{}] (GuildMemberLevel_Struct + class): Got [{}], expected [{}]",
				opcodes->EmuToName(in->GetOpcode()), in->size, sizeof(GuildMemberLevel_Struct) + sizeof(uint32));
			delete in;
			*p = nullptr;
			return;
		}
		uint32 class_ = 0;
		memcpy(&class_, in->pBuffer + sizeof(GuildMemberLevel_Struct), sizeof(uint32));

		SETUP_DIRECT_ENCODE(GuildMemberLevel_Struct, structs::GuildMemberLevel_Struct);
		eq->guild_id = ServerToTOBGuildID(emu->guild_id);
		TOBCopyStr(eq->player_name, emu->player_name);
		eq->level  = emu->level;
		eq->class_ = class_;
		FINISH_ENCODE();
	}

	ENCODE(OP_GuildMemberRankAltBanker)
	{
		ENCODE_LENGTH_EXACT(GuildMemberRank_Struct);
		SETUP_DIRECT_ENCODE(GuildMemberRank_Struct, structs::GuildMemberRank_Struct);

		eq->guild_id   = ServerToTOBGuildID(emu->guild_id);
		eq->rank       = emu->rank_;
		TOBCopyStr(eq->player_name, emu->player_name);
		eq->alt_banker = emu->alt_banker;
		eq->guild_show = static_cast<uint8>(emu->offline);   // emu offline flag, passed through
		eq->self_perm_flag = 0;

		FINISH_ENCODE();
	}

	ENCODE(OP_SetGuildRank)
	{
		ENCODE_LENGTH_EXACT(GuildSetRank_Struct);
		SETUP_DIRECT_ENCODE(GuildSetRank_Struct, structs::GuildMemberRank_Struct);

		eq->guild_id   = ServerToTOBGuildID(emu->Unknown00);  // the emu struct carries the guild id in Unknown00
		eq->rank       = emu->rank;
		TOBCopyStr(eq->player_name, emu->member_name);
		eq->alt_banker = emu->banker;
		eq->guild_show = 1;                                  // always 1

		FINISH_ENCODE();
	}

	ENCODE(OP_GuildMemberPublicNote)
	{
		ENCODE_LENGTH_EXACT(GuildMemberPublicNote_Struct);
		SETUP_DIRECT_ENCODE(GuildMemberPublicNote_Struct, structs::GuildMemberPublicNote_Struct);

		eq->guild_id = ServerToTOBGuildID(emu->guild_id);
		TOBCopyStr(eq->player_name, emu->player_name);
		TOBCopyStr(eq->public_note, emu->public_note);       // client strcpy's into 0x100: keep terminated

		FINISH_ENCODE();
	}

	ENCODE(OP_GuildMemberRename)
	{
		ENCODE_LENGTH_EXACT(GuildRenameMember_Struct);
		SETUP_DIRECT_ENCODE(GuildRenameMember_Struct, structs::GuildRenameMember_Struct);

		eq->guild_id = ServerToTOBGuildID(emu->guild_id);
		TOBCopyStr(eq->player_name, emu->player_name);
		TOBCopyStr(eq->new_player_name, emu->new_player_name);

		FINISH_ENCODE();
	}

	ENCODE(OP_GuildMemberDelete)
	{
		ENCODE_LENGTH_EXACT(GuildMemberDelete_Struct);
		SETUP_DIRECT_ENCODE(GuildMemberDelete_Struct, structs::GuildMemberDelete_Struct);

		eq->guild_id = ServerToTOBGuildID(emu->guild_id);
		TOBCopyStr(eq->player_name, emu->player_name);

		FINISH_ENCODE();
	}

	ENCODE(OP_GuildMemberDetails)
	{
		ENCODE_LENGTH_EXACT(GuildMemberDetails_Struct);
		SETUP_DIRECT_ENCODE(GuildMemberDetails_Struct, structs::GuildMemberDetails_Struct);

		eq->guild_id = ServerToTOBGuildID(emu->guild_id);
		TOBCopyStr(eq->player_name, emu->player_name);
		eq->zone_id      = emu->zone_id;
		eq->last_on      = emu->last_on;
		eq->offline_mode = emu->offline_mode;

		FINISH_ENCODE();
	}

	ENCODE(OP_GuildMemberUpdate)
	{
		ENCODE_LENGTH_EXACT(GuildMemberUpdate_Struct);
		SETUP_DIRECT_ENCODE(GuildMemberUpdate_Struct, structs::GuildMemberDetails_Struct);

		eq->guild_id = ServerToTOBGuildID(emu->GuildID);
		TOBCopyStr(eq->player_name, emu->MemberName);
		eq->zone_id      = emu->ZoneID;
		eq->last_on      = emu->LastSeen;
		eq->offline_mode = 0;                                // always 0

		FINISH_ENCODE();
	}

	ENCODE(OP_GuildRenameGuild)
	{
		ENCODE_LENGTH_EXACT(GuildRenameGuild_Struct);
		SETUP_DIRECT_ENCODE(GuildRenameGuild_Struct, structs::GuildRenameGuild_Struct);

		eq->guild_id = ServerToTOBGuildID(emu->guild_id);
		TOBCopyStr(eq->new_guild_name, emu->new_guild_name); // client strcpy's: keep terminated

		FINISH_ENCODE();
	}

	ENCODE(OP_GuildDeleteGuild)
	{
		ENCODE_LENGTH_EXACT(GuildDelete_Struct);
		SETUP_DIRECT_ENCODE(GuildDelete_Struct, structs::GuildDelete_Struct);

		eq->guild_id = ServerToTOBGuildID(emu->guild_id);

		FINISH_ENCODE();
	}

	ENCODE(OP_GuildsList)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		GuildsListMessaging_Struct glms{};
		try {
			EQ::Util::MemoryStreamReader ss(reinterpret_cast<char *>(in->pBuffer), in->size);
			cereal::BinaryInputArchive ar(ss);
			ar(glms);
		}
		catch (const std::exception &ex) {
			LogNetcode("TOB::ENCODE(OP_GuildsList) bad emu packet: {}", ex.what());
			delete in;
			return;
		}

		uint32 n = 0;
		for (auto const &g : glms.guild_detail) {
			if (g.guild_id != 0 && g.guild_id != TOB_SERVER_GUILD_NONE) {
				++n;
			}
		}

		SerializeBuffer buffer;
		buffer.WriteUInt32(0); // header string length, empty
		buffer.WriteUInt32(n);
		for (auto const &g : glms.guild_detail) {
			if (g.guild_id == 0 || g.guild_id == TOB_SERVER_GUILD_NONE) {
				continue;
			}
			buffer.WriteUInt64(g.guild_id);
			// keep names short enough that they stay terminated in the client's buffer
			buffer.WriteString(g.guild_name.substr(0, TOB_GUILD_NAME_MAX_CHARS));
		}

		auto outapp = new EQApplicationPacket(OP_GuildsList, buffer.size());
		outapp->WriteData(buffer.buffer(), buffer.size());
		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	ENCODE(OP_GuildMOTD)
	{
		ENCODE_LENGTH_ATLEAST(GuildMOTD_Struct);
		SETUP_DIRECT_ENCODE(GuildMOTD_Struct, structs::GuildMOTD_Struct);

		eq->guild_id = ServerToTOBGuildID(emu->unknown0);
		TOBCopyStr(eq->name, emu->name);
		TOBCopyStr(eq->setby_name, emu->setby_name);
		eq->unknown136 = emu->unknown132;
		TOBCopyStr(eq->motd, emu->motd);

		FINISH_ENCODE();
	}

	ENCODE(OP_GetGuildMOTDReply) { ENCODE_FORWARD(OP_GuildMOTD); }

	ENCODE(OP_GuildInvite)
	{
		ENCODE_LENGTH_EXACT(GuildCommand_Struct);
		SETUP_DIRECT_ENCODE(GuildCommand_Struct, structs::GuildInvite_Struct);

		TOBCopyStr(eq->othername, emu->othername);
		TOBCopyStr(eq->myname, emu->myname);
		eq->guild_id       = (emu->guildeqid == EMU_GUILD_NONE_U16) ? 0 : ServerToTOBGuildID(emu->guildeqid);
		eq->my_rank        = 0;
		eq->rank           = (emu->officer >= TOB_GUILD_RANK_MIN && emu->officer <= TOB_GUILD_RANK_MAX) ? emu->officer : TOB_GUILD_RANK_MAX;
		eq->use_guild_name = 0;                                                        // client resolves the name from guild_id

		FINISH_ENCODE();
	}

	ENCODE(OP_GuildInviteAccept)
	{
		ENCODE_LENGTH_EXACT(GuildInviteAccept_Struct);
		SETUP_DIRECT_ENCODE(GuildInviteAccept_Struct, structs::GuildInviteAccept_Struct);

		TOBCopyStr(eq->inviter, emu->inviter);
		TOBCopyStr(eq->new_member, emu->new_member);
		eq->response = emu->response;
		eq->guild_id = ServerToTOBGuildID(emu->guild_id);
		eq->action   = (emu->response >= EMU_GUILD_INVITE_DECLINE_RESPONSE) ? TOB_GUILD_INVITE_DECLINE_ACTION : TOB_GUILD_INVITE_ACCEPT_ACTION;

		FINISH_ENCODE();
	}

	// Writes structs::GuildUpdateHeader_Struct (unknown12 is always zero). name may be null (sent as all zeros);
	// otherwise the first sizeof(name) - 1 bytes are copied as they are and the last byte is forced to NUL.
	static void TOBWriteGuildUpdateHeader(SerializeBuffer &buffer, uint32 action, uint32 unknown4, uint32 unknown8,
		const char *name, uint64 guild_id)
	{
		buffer.WriteUInt32(action);
		buffer.WriteUInt32(unknown4);
		buffer.WriteUInt32(unknown8);
		buffer.WriteUInt32(0);
		constexpr size_t name_size = sizeof(structs::GuildUpdateHeader_Struct::name);
		if (name) {
			TOBWriteBytes(buffer, name, name_size - 1);
			buffer.WriteUInt8(0);
		}
		else {
			TOBWriteZeros(buffer, name_size);
		}
		buffer.WriteUInt64(guild_id);
	}

	// Layout of the emu GuildUpdate packet header. The emu side has no struct for the part every action shares.
	static constexpr uint32 EMU_GUILD_UPDATE_ACTION_OFFSET   = 0x00;
	static constexpr uint32 EMU_GUILD_UPDATE_UNKNOWN4_OFFSET = 0x04;
	static constexpr uint32 EMU_GUILD_UPDATE_UNKNOWN8_OFFSET = 0x08;
	static constexpr uint32 EMU_GUILD_UPDATE_NAME_OFFSET     = 0x0c;
	static constexpr uint32 EMU_GUILD_UPDATE_GUILD_ID_OFFSET = 0x4c;
	static constexpr uint32 EMU_GUILD_UPDATE_HEADER_SIZE     = 0x50; // the payload starts here

	// OP_GuildUpdate actions. URL and channel match the server's GuildInformationActions; ToB drops one entry from
	// the middle of that list, so rank names (server: GuildUpdateRanks) and permissions (server:
	// GuildUpdatePermissions) are numbered one lower on the wire.
	enum TOBGuildUpdateAction : uint32 {
		TOBGuildUpdateRankNames   = 3,
		TOBGuildUpdatePermissions = 4,
	};

	// GuildUpdate: ToB inserts 4 bytes before the name (0x0c -> 0x10), widens the guild id to u64
	// (0x4c u32 -> 0x50 u64), so the payload moves 0x50 -> 0x58 (+8).
	// Action numbers 4 (rank names) and 5 (permissions) become 3 and 4.
	ENCODE(OP_GuildUpdate)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		constexpr uint32 EMU_HDR = EMU_GUILD_UPDATE_HEADER_SIZE;
		constexpr uint32 TOB_HDR = sizeof(structs::GuildUpdateHeader_Struct); // 0x58
		if (in->size < EMU_HDR) {
			delete in;
			return;
		}

		const uint8 *src = in->pBuffer;
		uint32 action = 0, u4 = 0, u8_ = 0, gid = 0;
		memcpy(&action, src + EMU_GUILD_UPDATE_ACTION_OFFSET,   sizeof(action));
		memcpy(&u4,     src + EMU_GUILD_UPDATE_UNKNOWN4_OFFSET, sizeof(u4));
		memcpy(&u8_,    src + EMU_GUILD_UPDATE_UNKNOWN8_OFFSET, sizeof(u8_));
		memcpy(&gid,    src + EMU_GUILD_UPDATE_GUILD_ID_OFFSET, sizeof(gid));

		// Actions 0/1 are the guild URL / channel (GuildUpdateURLAndChannel_Struct, text at 0x50). Their
		// ToB layout is unmapped and sending them crashed the client at zone-in, so drop them.
		// The ToB guild window is NULL from character select until the first zone-in finishes, and the
		// URL/channel handlers dereference it unchecked, so only forward replies to the window-open
		// request (OP_GuildOpenGuildWindow), which the zone marks with WindowOpen.
		if (action == GuildUpdateURL || action == GuildUpdateChannel) {
			if (in->size < sizeof(GuildUpdateURLAndChannel_Struct) ||
				reinterpret_cast<GuildUpdateURLAndChannel_Struct *>(in->pBuffer)->WindowOpen == 0) {
				delete in;
				return;
			}
			auto uc = reinterpret_cast<GuildUpdateURLAndChannel_Struct *>(in->pBuffer);
			const size_t max_text = (action == GuildUpdateURL) ? TOB_GUILD_URL_MAX_CHARS : TOB_GUILD_CHANNEL_MAX_CHARS;
			const size_t text_len = std::min(strnlen(uc->Text, sizeof(uc->Text)), max_text);
			SerializeBuffer buffer;
			TOBWriteGuildUpdateHeader(buffer, action, 0, 0, nullptr, 0);
			TOBWriteBytes(buffer, uc->Text, text_len);
			TOBWriteZeros(buffer, TOB_GUILD_URL_FIELD_SIZE - text_len); // text field is zero padded to its full width
			auto outapp = new EQApplicationPacket(OP_GuildUpdate, std::move(buffer));
			dest->FastQueuePacket(&outapp, ack_req);
			delete in;
			return;
		}

		uint32 tob_action = action;
		if (action == GuildUpdateRanks)            { tob_action = TOBGuildUpdateRankNames; }
		else if (action == GuildUpdatePermissions) { tob_action = TOBGuildUpdatePermissions; }

		const uint32 payload = in->size - EMU_HDR;
		SerializeBuffer buffer;
		TOBWriteGuildUpdateHeader(buffer, tob_action, u4, u8_, reinterpret_cast<const char *>(src + EMU_GUILD_UPDATE_NAME_OFFSET), ServerToTOBGuildID(gid));
		TOBWriteBytes(buffer, src + EMU_HDR, payload);

		auto outapp = new EQApplicationPacket(OP_GuildUpdate, std::move(buffer));
		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}


	// OP_Emote 0x43cb, S2C: u32 type; char message[]
	ENCODE(OP_Emote)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		if (in->size < sizeof(uint32)) { // emu packet too small; drop
			delete in;
			return;
		}

		Emote_Struct *emu = (Emote_Struct *)in->pBuffer;
		size_t emu_msg_cap = std::min<size_t>(sizeof(emu->message), in->size - sizeof(uint32));
		std::string old_message(emu->message, strnlen(emu->message, emu_msg_cap));
		std::string new_message;
		ServerToTOBConvertLinks(new_message, old_message);
		if (new_message.length() > TOB_TEXT_MAX_CHARS)
			new_message.resize(TOB_TEXT_MAX_CHARS);

		SerializeBuffer buffer;
		buffer.WriteUInt32(emu->type);
		buffer.WriteString(new_message);
		auto outapp = new EQApplicationPacket(OP_Emote, std::move(buffer));

		delete in;
		dest->FastQueuePacket(&outapp, ack_req);
	}

	// OP_InspectRequest 0x6ea6: server -> target client forward (pre-SoF targets only)
	ENCODE(OP_InspectRequest)
	{
		ENCODE_LENGTH_EXACT(Inspect_Struct);
		SETUP_DIRECT_ENCODE(Inspect_Struct, structs::Inspect_Struct);
		OUT(TargetID);
		OUT(PlayerID);
		FINISH_ENCODE();
	}

	// OP_GMLastName 0x00a0, S2C (surname refresh for other clients): same 195-byte layout as the C2S packet
	ENCODE(OP_GMLastName)
	{
		ENCODE_LENGTH_EXACT(GMLastName_Struct);
		SETUP_DIRECT_ENCODE(GMLastName_Struct, structs::GMLastName_Struct);
		strn0cpy(eq->name, emu->name, sizeof(eq->name));
		strn0cpy(eq->gmname, emu->gmname, sizeof(eq->gmname));
		strn0cpy(eq->lastname, emu->lastname, sizeof(eq->lastname));
		// emu unknown[] is uint16 (1,1,1,1); a byte copy sent 01 00 01 and the name never refreshed
		eq->unknown192[0] = 1;
		eq->unknown192[1] = 1;
		eq->unknown192[2] = 1;
		FINISH_ENCODE();
	}

	// ---- raid + task window ----

	// String from a fixed-size emu char array; never reads past the array, truncated to max_len chars.
	static inline std::string TOBFixedStr(const char *s, size_t cap, size_t max_len)
	{
		return std::string(s, std::min(strnlen(s, cap), max_len));
	}

	// ToB raid stream header:
	//   u32 action; cstr player_name; u32 unknown1 (raid id); cstr leader_name; u32 (new, unused); u32 parameter
	static inline void TOBWriteRaidHeader(SerializeBuffer &buf, uint32 action, const std::string &player_name,
		uint32 unknown1, const std::string &leader_name, uint32 parameter)
	{
		buf.WriteUInt32(action);
		buf.WriteString(player_name);   // the client keeps at most TOB_NAME_MAX_CHARS
		buf.WriteUInt32(unknown1);
		buf.WriteString(leader_name);   // the name the sub-action acts on
		buf.WriteUInt32(0);             // new in ToB, not read by the handled actions
		buf.WriteUInt32(parameter);     // parameter (group, eqstr id, loot type, flag)
	}

	// Raid delegate ability types. The server numbers them 3 (main assist) and 4 (main marker).
	enum TOBDelegateType : uint32 {
		EmuDelegateMainAssist = 3,
		EmuDelegateMainMarker = 4,
		EmuDelegateMasterLooter = 100, // Advanced Loot master looter
		TOBDelegateMainAssist = 1, // eqstr 0x2242/0x2248
		TOBDelegateMasterLooter = 0,
		TOBDelegateMainMarker = 2, // eqstr 0x2243/0x2249
	};

	static inline uint32 ServerToTOBDelegateType(uint32 t)
	{
		switch (t) {
			case EmuDelegateMainAssist: return TOBDelegateMainAssist;
			case EmuDelegateMainMarker: return TOBDelegateMainMarker;
			case EmuDelegateMasterLooter: return TOBDelegateMasterLooter;
			default: return TOB_INVALID_ID; // emu type 5 has no ToB case; ToB has no other types
		}
	}

	static inline uint32 TOBToServerDelegateType(uint32 t)
	{
		switch (t) {
			case TOBDelegateMasterLooter: return EmuDelegateMasterLooter;
			case TOBDelegateMainAssist: return EmuDelegateMainAssist;
			case TOBDelegateMainMarker: return EmuDelegateMainMarker;
			default: return TOB_INVALID_ID; // unknown to the server; Handle_OP_RaidDelegateAbility ignores it
		}
	}

	// ---- OP_RaidUpdate (S2C 0x294a) ----
	// ToB parses this as a stream.
	ENCODE(OP_RaidUpdate)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		const uint8 *emu_buf  = in->pBuffer;
		const size_t emu_size = in->size;
		if (emu_buf == nullptr || emu_size < sizeof(uint32)) {
			delete in;
			return;
		}

		uint32 action = 0;
		memcpy(&action, emu_buf, sizeof(uint32));

		SerializeBuffer buf;

		switch (action) {
			case raidNoRaid: {
				// Zone-in sends ZoneInSendName_Struct (136 bytes, shorter than RaidGeneral_Struct).
				// ToB case 10 reads nothing past the action, but only clears the raid silently when the client
				// thinks it isn't in one; ToB keeps its raid across zones, so members were added again on every
				// zone-in. Action 4 (no longer leader, parameter 0) drops the in-raid flag without a message.
				SerializeBuffer pre;
				TOBWriteRaidHeader(pre, raidNoLongerLeader, "", 0, "", 0);
				auto pre_app = new EQApplicationPacket(OP_RaidUpdate, std::move(pre));
				dest->FastQueuePacket(&pre_app, ack_req);
				TOBWriteRaidHeader(buf, action, "", 0, "", 0);
				break;
			}
			case raidSetLeaderAbilities: {
				// ToB has no case 14 (the switch ignores it). Leadership AA would need ToB case 15's new
				// layout, which is not mapped yet. Drop.
				delete in;
				return;
			}
			case raidMakeLeader: {
				// emu RaidLeadershipUpdate_Struct: action@0, player_name@4, leader_name@68 (no unknown1).
				// ToB case 30 reads 5 header fields and no leadership AA block.
				if (emu_size < offsetof(RaidLeadershipUpdate_Struct, group)) {
					LogNetcode("TOB OP_RaidUpdate action [{}] too short ([{}] bytes)", action, emu_size);
					delete in;
					return;
				}
				auto emu = (const RaidLeadershipUpdate_Struct *)emu_buf;
				TOBWriteRaidHeader(buf, action,
					TOBFixedStr(emu->player_name, sizeof(emu->player_name), TOB_NAME_MAX_CHARS), 0,
					TOBFixedStr(emu->leader_name, sizeof(emu->leader_name), TOB_NAME_MAX_CHARS), 0);
				break;
			}
			case raidAdd: {
				if (emu_size < sizeof(RaidAddMember_Struct)) {
					LogNetcode("TOB OP_RaidUpdate raidAdd too short ([{}] bytes)", emu_size);
					delete in;
					return;
				}
				auto emu = (const RaidAddMember_Struct *)emu_buf;
				TOBWriteRaidHeader(buf, action,
					TOBFixedStr(emu->raidGen.player_name, sizeof(emu->raidGen.player_name), TOB_NAME_MAX_CHARS),
					emu->raidGen.unknown1,
					TOBFixedStr(emu->raidGen.leader_name, sizeof(emu->raidGen.leader_name), TOB_NAME_MAX_CHARS),
					emu->raidGen.parameter);                 // group number, 0xFFFFFFFF = ungrouped
				buf.WriteUInt32(static_cast<uint32>(emu->_class)); // u32 in ToB
				buf.WriteUInt8(emu->level);
				buf.WriteUInt8(emu->isGroupLeader);
				break;
			}
			case raidSetMotd: {
				if (emu_size < sizeof(RaidMOTD_Struct)) {
					LogNetcode("TOB OP_RaidUpdate raidSetMotd too short ([{}] bytes)", emu_size);
					delete in;
					return;
				}
				auto emu = (const RaidMOTD_Struct *)emu_buf;
				TOBWriteRaidHeader(buf, action,
					TOBFixedStr(emu->general.player_name, sizeof(emu->general.player_name), TOB_NAME_MAX_CHARS),
					emu->general.unknown1,
					TOBFixedStr(emu->general.leader_name, sizeof(emu->general.leader_name), TOB_NAME_MAX_CHARS),
					emu->general.parameter);
				buf.WriteString(TOBFixedStr(emu->motd, sizeof(emu->motd), TOB_TEXT_MAX_CHARS));
				break;
			}
			case raidSetNote: {
				if (emu_size < sizeof(RaidNote_Struct)) {
					LogNetcode("TOB OP_RaidUpdate raidSetNote too short ([{}] bytes)", emu_size);
					delete in;
					return;
				}
				auto emu = (const RaidNote_Struct *)emu_buf;
				TOBWriteRaidHeader(buf, action,
					TOBFixedStr(emu->general.player_name, sizeof(emu->general.player_name), TOB_NAME_MAX_CHARS),
					emu->general.unknown1,
					TOBFixedStr(emu->general.leader_name, sizeof(emu->general.leader_name), TOB_NAME_MAX_CHARS),
					emu->general.parameter);
				buf.WriteString(TOBFixedStr(emu->note, sizeof(emu->note), TOB_NAME_MAX_CHARS)); // member note
				break;
			}
			case raidCreate: {
				// Normally arrives via OP_RaidJoin; handled here too in case a RaidGeneral_Struct is used.
				if (emu_size < sizeof(RaidGeneral_Struct)) {
					delete in;
					return;
				}
				auto emu = (const RaidGeneral_Struct *)emu_buf;
				// raid id must be >= 1, or the client ignores a later disband
				TOBWriteRaidHeader(buf, action,
					TOBFixedStr(emu->player_name, sizeof(emu->player_name), TOB_NAME_MAX_CHARS),
					emu->unknown1 ? emu->unknown1 : 1,
					TOBFixedStr(emu->leader_name, sizeof(emu->leader_name), TOB_NAME_MAX_CHARS), 0);
				buf.WriteUInt8(0); // case 8 reads a trailing u8 in place of the 6th u32 (unused)
				break;
			}
			default: {
				// raidRemove2 (1), raidDisband (5), raidLock/Unlock (17/18), invite (20), loot (11/32),
				// string id (12/19/31), group changes (3/13/16) ... all use the plain header.
				if (emu_size < sizeof(RaidGeneral_Struct)) {
					LogNetcode("TOB OP_RaidUpdate action [{}] too short ([{}] bytes)", action, emu_size);
					delete in;
					return;
				}
				auto emu = (const RaidGeneral_Struct *)emu_buf;
				TOBWriteRaidHeader(buf, action,
					TOBFixedStr(emu->player_name, sizeof(emu->player_name), TOB_NAME_MAX_CHARS),
					emu->unknown1,
					TOBFixedStr(emu->leader_name, sizeof(emu->leader_name), TOB_NAME_MAX_CHARS),
					emu->parameter);
				break;
			}
		}

		auto outapp = new EQApplicationPacket(OP_RaidUpdate, buf.size());
		outapp->WriteData(buf.buffer(), buf.size());
		delete in;
		dest->FastQueuePacket(&outapp, ack_req);
	}

	// ---- OP_RaidJoin (emu-only) -> OP_RaidUpdate action 8 (raidCreate) ----
	// ToB case 8: u32 8; cstr; u32 raid_id; cstr; u32; u8.
	ENCODE(OP_RaidJoin)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		if (in->pBuffer == nullptr || in->size < sizeof(RaidCreate_Struct)) {
			LogNetcode("TOB OP_RaidJoin too short ([{}] bytes)", in->size);
			delete in;
			return;
		}
		auto emu = (const RaidCreate_Struct *)in->pBuffer;

		// The emu packet has no raid id. Any value >= 1 works for the client (it is echoed back in C2S
		// unknown1, which the server ignores); use the leader's entity id, or 1.
		uint32 raid_id = emu->leader_id ? emu->leader_id : 1;
		std::string leader = TOBFixedStr(emu->leader_name, sizeof(emu->leader_name), TOB_NAME_MAX_CHARS);

		SerializeBuffer buf;
		TOBWriteRaidHeader(buf, raidCreate, leader, raid_id, leader, RaidCommandAcceptInvite);
		buf.WriteUInt8(0);

		auto outapp = new EQApplicationPacket(OP_RaidUpdate, buf.size());
		outapp->WriteData(buf.buffer(), buf.size());
		delete in;
		dest->FastQueuePacket(&outapp, ack_req);
	}

	// ---- OP_RaidDelegateAbility 0x8ea0 ----
	ENCODE(OP_RaidDelegateAbility)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		if (in->pBuffer == nullptr || in->size < sizeof(DelegateAbility_Struct)) {
			LogNetcode("TOB OP_RaidDelegateAbility too short ([{}] bytes)", in->size);
			delete in;
			return;
		}
		auto emu = (const DelegateAbility_Struct *)in->pBuffer;

		uint32 tob_type = ServerToTOBDelegateType(emu->DelegateAbility);
		if (tob_type == TOB_INVALID_ID) { // no ToB equivalent
			delete in;
			return;
		}

		auto outapp = new EQApplicationPacket(OP_RaidDelegateAbility, sizeof(structs::DelegateAbility_Struct));
		auto eq = (structs::DelegateAbility_Struct *)outapp->pBuffer; // zeroed by the constructor
		eq->DelegateAbility = tob_type;
		eq->MemberNumber    = emu->MemberNumber;
		eq->Action          = emu->Action;
		eq->Unknown012      = emu->Unknown012;
		eq->Unknown016      = emu->Unknown016;
		// ToB compares the delegate id with the character id (PlayerProfile unique_player_id), not the spawn id;
		// the zone puts the character id in Unknown024.
		eq->EntityID        = emu->Unknown024 ? emu->Unknown024 : emu->EntityID;
		eq->Unknown024      = emu->Unknown024;
		strn0cpy(eq->Name, emu->Name, sizeof(eq->Name)); // both 64; strn0cpy always terminates

		delete in;
		dest->FastQueuePacket(&outapp, ack_req);
	}

	DECODE(OP_RaidDelegateAbility)
	{
		// C2S: u64 0; 7 x u32; cstr name. Variable length; the server wants exactly 92 bytes.
		const unsigned char *buf  = __packet->pBuffer;
		const size_t         size = __packet->size;
		const size_t fixed = sizeof(uint64) + 7 * sizeof(uint32); // 36

		if (buf == nullptr || size < fixed) {
			LogNetcode("TOB OP_RaidDelegateAbility C2S too short ([{}] bytes)", size);
			__packet->SetOpcode(OP_Unknown);
			return;
		}

		uint32 f[7];
		memcpy(f, buf + sizeof(uint64), sizeof(f));
		const char *name     = (const char *)(buf + fixed);
		size_t      name_len = strnlen(name, size - fixed); // tolerate a missing terminator

		unsigned char *__eq_buffer = __packet->pBuffer;
		__packet->size    = sizeof(DelegateAbility_Struct);
		__packet->pBuffer = new unsigned char[__packet->size] {};
		auto emu = (DelegateAbility_Struct *)__packet->pBuffer;
		emu->DelegateAbility = TOBToServerDelegateType(f[0]);
		emu->MemberNumber    = f[1];
		emu->Action          = f[2];
		emu->Unknown012      = f[3];
		emu->Unknown016      = f[4];
		emu->EntityID        = f[5];
		emu->Unknown024      = f[6];
		memcpy(emu->Name, name, std::min(name_len, sizeof(emu->Name) - 1)); // buffer is zeroed -> terminated

		delete[] __eq_buffer;
	}

	// ---- OP_TaskHistoryReply 0x50a3 (S2C) ----
	// Per activity: u32 type; cstr target; lenstr item_list; u32 goal; lenstr skill;
	// lenstr spell; cstr zones; cstr (new); u32 dz_switch_id; cstr description; cstr zones2.
	// Input is the emu format: u32 index; u32 count; count x { u32 type;
	// cstr target; cstr item_list; u32 goal; u32; u32; u32 zone_id; u32; cstr desc }.
	ENCODE(OP_TaskHistoryReply)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		in->SetReadPosition(0);
		uint32 task_index = in->ReadUInt32();
		uint32 count      = in->ReadUInt32();
		if (in->ReadError() || task_index >= TOB_TASK_HISTORY_MAX_TASKS) { // client ignores a larger index
			delete in;
			return;
		}
		if (count > TOB_TASK_HISTORY_MAX_ACTIVITIES) { // client clamps the activity count
			count = TOB_TASK_HISTORY_MAX_ACTIVITIES;
		}

		struct Act {
			uint32      type = 0;
			uint32      goal = 0;
			std::string target, items, desc, zone;
		};
		std::vector<Act> acts;
		acts.reserve(count);
		for (uint32 i = 0; i < count; ++i) {
			Act a;
			a.type = in->ReadUInt32();
			in->ReadString(a.target, TOB_NAME_MAX_CHARS);
			in->ReadString(a.items, TOB_TEXT_MAX_CHARS);
			a.goal = in->ReadUInt32();
			in->ReadUInt32();
			in->ReadUInt32();
			uint32 zone_id = in->ReadUInt32();
			in->ReadUInt32();
			in->ReadString(a.desc, TOB_LINE_MAX_CHARS);
			if (in->ReadError()) {
				break;                // truncated emu packet: send the complete activities only
			}
			a.zone = std::to_string(zone_id);
			acts.push_back(std::move(a));
		}

		SerializeBuffer buf;
		buf.WriteUInt32(task_index);
		buf.WriteUInt32(static_cast<uint32>(acts.size()));
		for (const auto &a : acts) {
			buf.WriteUInt32(a.type);
			buf.WriteString(a.target);
			buf.WriteLengthString(a.items);
			buf.WriteUInt32(a.goal);
			buf.WriteLengthString(std::string("-1")); // skill list ("-1" = none)
			buf.WriteLengthString(std::string("-1")); // spell list
			buf.WriteString(a.zone);                  // zones: cstr here (lenstr in OP_TaskActivity)
			buf.WriteString("");                      // new ToB field, same as in OP_TaskActivity
			buf.WriteUInt32(0);                       // dz_switch_id
			buf.WriteString(a.desc);
			buf.WriteString(a.zone);                  // zones2
		}

		auto outapp = new EQApplicationPacket(OP_TaskHistoryReply, buf.size());
		outapp->WriteData(buf.buffer(), buf.size());
		delete in;
		dest->FastQueuePacket(&outapp, ack_req);
	}

	// ---- OP_CompletedTasks 0x7446 (S2C), with clamps ----
	// Client loop: stops after 50 entries and copies each title with an
	// UNBOUNDED byte loop into a 64-byte field, so both limits must be enforced here.
	ENCODE(OP_CompletedTasks)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		in->SetReadPosition(0);
		uint32 count = in->ReadUInt32();
		if (in->ReadError()) {
			delete in;
			return;
		}

		struct Done {
			uint32      task_id = 0;
			uint32      completed_time = 0;
			std::string title;
		};
		std::vector<Done> tasks;
		tasks.reserve(std::min<uint32>(count, TOB_TASK_HISTORY_MAX_TASKS));
		for (uint32 i = 0; i < count && tasks.size() < TOB_TASK_HISTORY_MAX_TASKS; ++i) {
			Done t;
			t.task_id = in->ReadUInt32();
			in->ReadString(t.title, TOB_NAME_MAX_CHARS);
			t.completed_time = in->ReadUInt32();
			if (in->ReadError()) {
				break;
			}
			tasks.push_back(std::move(t));
		}

		SerializeBuffer buf;
		buf.WriteUInt32(static_cast<uint32>(tasks.size()));
		for (const auto &t : tasks) {
			buf.WriteUInt32(t.task_id);
			buf.WriteString(t.title);
			buf.WriteUInt64(static_cast<uint64>(t.completed_time));
		}

		auto outapp = new EQApplicationPacket(OP_CompletedTasks, buf.size());
		outapp->WriteData(buf.buffer(), buf.size());
		delete in;
		dest->FastQueuePacket(&outapp, ack_req);
	}


	// ---- tribute + mercenaries ----
	ENCODE(OP_TributeItem)
	{
		ENCODE_LENGTH_EXACT(TributeItem_Struct);
		SETUP_DIRECT_ENCODE(TributeItem_Struct, structs::TributeItem_Struct);

		eq->inventory_slot = ServerToTOBSlot(emu->slot);
		OUT(quantity);
		OUT(tribute_master_id);
		OUT(tribute_points);

		FINISH_ENCODE();
	}

	ENCODE(OP_MercenaryTimer)
	{
		ENCODE_LENGTH_EXACT(MercenaryStatus_Struct);
		SETUP_DIRECT_ENCODE(MercenaryStatus_Struct, structs::MercenaryStatus_Struct);

		OUT(MercEntityID);
		OUT(UpdateInterval);
		OUT(MercUnk01);
		// ToB inserted a state 5; the server's 5 ("normal"/no active merc) is 6 in ToB.
		eq->MercState = (emu->MercState >= 5) ? emu->MercState + 1 : emu->MercState;
		eq->SuspendedTime = emu->SuspendedTime;	// zero-extend unix time

		FINISH_ENCODE();
	}

	ENCODE(OP_MercenarySuspendResponse)
	{
		ENCODE_LENGTH_EXACT(SuspendMercenaryResponse_Struct);
		SETUP_DIRECT_ENCODE(SuspendMercenaryResponse_Struct, structs::SuspendMercenaryResponse_Struct);

		eq->SuspendTime = emu->SuspendTime;

		FINISH_ENCODE();
	}

	// One merc list entry. Used by OP_MercenaryDataUpdate and OP_MercenaryDataResponse.
	// 'name' may be nullptr (merchant list sends an empty name).
	static void WriteTOBMercEntry(SerializeBuffer &buffer, uint32 merc_id, uint32 merc_type, uint32 merc_sub_type,
		uint32 purchase_cost, uint32 upkeep_cost, uint32 status, uint32 alt_cost, uint32 alt_upkeep, uint32 alt_type,
		uint8 unk01, int32 time_left, uint32 merchant_slot, uint32 unk02, uint32 stance_count, int32 unk03, uint8 unk04,
		const char *name, size_t name_max, const MercenaryStance_Struct *stances)
	{
		buffer.WriteUInt32(merc_id);
		buffer.WriteUInt32(merc_type);
		buffer.WriteUInt32(merc_sub_type);
		buffer.WriteUInt32(purchase_cost);
		buffer.WriteUInt32(upkeep_cost);
		buffer.WriteUInt8(static_cast<uint8>(status));	// u8 in ToB
		buffer.WriteUInt32(alt_cost);
		buffer.WriteUInt32(alt_upkeep);
		buffer.WriteUInt32(alt_type);
		buffer.WriteUInt8(unk01);
		buffer.WriteInt32(time_left);
		buffer.WriteUInt32(merchant_slot);
		buffer.WriteUInt32(unk02);
		buffer.WriteUInt32(stance_count);
		buffer.WriteInt32(unk03);
		buffer.WriteUInt8(unk04);
		buffer.WriteUInt32(0);	// new in ToB, meaning unknown
		if (name && name_max > 0) {
			// never read past the emu array
			std::string n(name, strnlen(name, std::min<size_t>(name_max, TOB_NAME_MAX_CHARS)));
			buffer.WriteString(n);
		}
		else {
			buffer.WriteUInt8(0);
		}
		for (uint32 k = 0; k < stance_count; ++k) {
			buffer.WriteUInt32(stances[k].StanceIndex);
			buffer.WriteUInt32(stances[k].Stance);
		}
	}

	ENCODE(OP_MercenaryDataUpdate)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		SerializeBuffer buffer;

		const size_t hdr = offsetof(MercenaryDataUpdate_Struct, MercData);
		if (in->size > sizeof(NoMercenaryHired_Struct) && in->size >= hdr &&
			((MercenaryDataUpdate_Struct *)in->pBuffer)->MercStatus >= 0) {
			auto emu = (MercenaryDataUpdate_Struct *)in->pBuffer;
			// only trust as many entries as the emu buffer really holds
			uint32 max_by_size = (in->size - hdr) / sizeof(MercenaryData_Struct);
			uint32 count = std::min<uint32>(std::min<uint32>(emu->MercCount, MAX_MERC), max_by_size);

			buffer.WriteInt32(emu->MercStatus);	// index of the active merc, -1 = none
			buffer.WriteUInt32(count);
			for (uint32 r = 0; r < count; ++r) {
				const auto &m = emu->MercData[r];
				uint32 stances = std::min<uint32>(m.StanceCount, MAX_MERC_STANCES);
				WriteTOBMercEntry(buffer, m.MercID, m.MercType, m.MercSubType, m.PurchaseCost, m.UpkeepCost,
					m.Status, m.AltCurrencyCost, m.AltCurrencyUpkeep, m.AltCurrencyType, m.MercUnk01, m.TimeLeft,
					m.MerchantSlot, m.MercUnk02, stances, m.MercUnk03, m.MercUnk04, m.MercName, sizeof(m.MercName),
					m.Stances);
			}
			buffer.WriteUInt32(count ? emu->MercData[0].MercUnk05 : 1);	// trailing field (unlocked slots)
			buffer.WriteUInt32(0xb); // new ToB trailing field; 0xb is the client's own default
		}
		else if (in->size >= sizeof(NoMercenaryHired_Struct)) {
			// SendClearMercInfo: {-1, 0, 1} (or any MercStatus < 0)
			auto emu = (NoMercenaryHired_Struct *)in->pBuffer;
			buffer.WriteInt32(emu->MercStatus);
			buffer.WriteUInt32(0);
			buffer.WriteUInt32(emu->MercID);
			buffer.WriteUInt32(0xb);
		}
		else {
			LogNetcode("TOB::ENCODE(OP_MercenaryDataUpdate) short packet [{}]", in->size);
			delete in;
			return;
		}

		auto outapp = new EQApplicationPacket(OP_MercenaryDataUpdate, buffer.size());
		outapp->WriteData(buffer.buffer(), buffer.size());
		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	ENCODE(OP_MercenaryDataResponse)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		if (in->size < offsetof(MercenaryMerchantList_Struct, Mercs)) {
			LogNetcode("TOB::ENCODE(OP_MercenaryDataResponse) short packet [{}]", in->size);
			delete in;
			return;
		}

		auto emu = (MercenaryMerchantList_Struct *)in->pBuffer;
		uint32 type_count = std::min<uint32>(emu->MercTypeCount, MAX_MERC_GRADES);
		// only trust as many entries as the emu buffer really holds
		uint32 max_by_size = (in->size - offsetof(MercenaryMerchantList_Struct, Mercs)) / sizeof(MercenaryListEntry_Struct);
		uint32 count = std::min<uint32>(std::min<uint32>(emu->MercCount, MAX_MERC), max_by_size);

		SerializeBuffer buffer;
		buffer.WriteUInt32(type_count);
		for (uint32 r = 0; r < type_count; ++r) {
			buffer.WriteUInt32(emu->MercGrades[r]);
		}
		buffer.WriteUInt32(count);
		for (uint32 r = 0; r < count; ++r) {
			const auto &m = emu->Mercs[r];
			uint32 stances = std::min<uint32>(m.StanceCount, MAX_MERC_STANCES);
			WriteTOBMercEntry(buffer, m.MercID, m.MercType, m.MercSubType, m.PurchaseCost, m.UpkeepCost,
				m.Status, m.AltCurrencyCost, m.AltCurrencyUpkeep, m.AltCurrencyType, m.MercUnk01, m.TimeLeft,
				m.MerchantSlot, m.MercUnk02, stances, m.MercUnk03, m.MercUnk04, nullptr, 0, m.Stances);
		}

		auto outapp = new EQApplicationPacket(OP_MercenaryDataResponse, buffer.size());
		outapp->WriteData(buffer.buffer(), buffer.size());
		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}


	// ---- DZ, inspect buffs, veteran rewards ----

	ENCODE(OP_DzChooseZone)
	{
		EQApplicationPacket* in = *p;
		*p = nullptr;

		if (in->size < sizeof(DynamicZoneChooseZone_Struct)) {
			delete in;
			return;
		}
		auto emu = reinterpret_cast<DynamicZoneChooseZone_Struct*>(in->pBuffer);
		uint32 max_count = (in->size - sizeof(DynamicZoneChooseZone_Struct)) / sizeof(DynamicZoneChooseZoneEntry_Struct);
		uint32 count = std::min(emu->count, max_count);

		SerializeBuffer buf;
		buf.WriteUInt32(emu->client_id);
		buf.WriteUInt32(count);
		for (uint32 i = 0; i < count; ++i) {
			const auto& c = emu->choices[i];
			buf.WriteUInt16(c.dz_zone_id);       // u32 dz_id = zone | instance << 16
			buf.WriteUInt16(c.dz_instance_id);
			buf.WriteUInt32(c.unknown_id1);
			buf.WriteUInt8(static_cast<uint8>(c.dz_type)); // ToB: u8
			buf.WriteUInt32(c.unknown_id2);
			buf.WriteString(std::string(c.description, strnlen(c.description, sizeof(c.description) - 1)));
			buf.WriteString(std::string(c.leader_name, strnlen(c.leader_name, sizeof(c.leader_name) - 1)));
		}

		auto outapp = new EQApplicationPacket(OP_DzChooseZone, buf.size());
		outapp->WriteData(buf.buffer(), buf.size());
		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
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
		eq->assigned = emu->assigned ? 1 : 0;
		OUT(max_players);
		eq->unknown016 = 0;
		strn0cpy(eq->dz_name, emu->dz_name, sizeof(eq->dz_name));
		strn0cpy(eq->leader_name, emu->leader_name, sizeof(eq->leader_name));
		// the client sprintf()s these with the name as the format string
		std::replace(eq->dz_name, eq->dz_name + sizeof(eq->dz_name), '%', ' ');
		std::replace(eq->leader_name, eq->leader_name + sizeof(eq->leader_name), '%', ' ');
		eq->dz_type = 1; // expedition (emu struct has no type); lets the window auto-open like live

		FINISH_ENCODE();
	}

	ENCODE(OP_DzExpeditionInvite)
	{
		EQApplicationPacket* in = *p;
		*p = nullptr;

		if (in->size != sizeof(ExpeditionInvite_Struct)) {
			LogNetcode("TOB::ENCODE(OP_DzExpeditionInvite) wrong size [{}]", in->size);
			delete in;
			return;
		}
		auto emu = reinterpret_cast<ExpeditionInvite_Struct*>(in->pBuffer);

		SerializeBuffer buf;
		buf.WriteUInt32(emu->client_id);
		buf.WriteUInt32(emu->unknown004);
		buf.WriteLengthString(std::string());   // replay-timer warning list (emu has none)
		buf.WriteLengthString(std::string(emu->inviter_name, strnlen(emu->inviter_name, sizeof(emu->inviter_name))));
		buf.WriteLengthString(std::string(emu->expedition_name, strnlen(emu->expedition_name, sizeof(emu->expedition_name))));
		buf.WriteLengthString(std::string(emu->swap_name, strnlen(emu->swap_name, sizeof(emu->swap_name))));
		buf.WriteUInt16(emu->dz_zone_id);       // echoed back at +8 of OP_DzExpeditionInviteResponse
		buf.WriteUInt16(emu->dz_instance_id);
		buf.WriteUInt8(emu->swapping);

		auto outapp = new EQApplicationPacket(OP_DzExpeditionInvite, buf.size());
		outapp->WriteData(buf.buffer(), buf.size());
		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	ENCODE(OP_DzExpeditionLockoutTimers)
	{
		EQApplicationPacket* in = *p;
		*p = nullptr;

		if (in->size < sizeof(ExpeditionLockoutTimers_Struct)) {
			delete in;
			return;
		}
		auto emu = reinterpret_cast<ExpeditionLockoutTimers_Struct*>(in->pBuffer);
		uint32 max_count = (in->size - sizeof(ExpeditionLockoutTimers_Struct)) / sizeof(ExpeditionLockoutTimerEntry_Struct);
		uint32 count = std::min(emu->count, max_count);

		SerializeBuffer buf;
		buf.WriteUInt32(emu->client_id);
		if (count == 0) {
			// The ToB window only clears its rows when the new list has at least one entry,
			// so send one already-expired placeholder,
			// which the client never draws.
			buf.WriteUInt32(1);
			buf.WriteLengthString(std::string());
			buf.WriteUInt32(0);
			buf.WriteInt32(-1);
			buf.WriteLengthString(std::string());
		}
		else {
			buf.WriteUInt32(count);
		}
		for (uint32 i = 0; i < count; ++i) {
			const auto& t = emu->timers[i];
			// ToB: u32-length-prefixed strings
			buf.WriteLengthString(std::string(t.expedition_name, strnlen(t.expedition_name, TOB_LINE_MAX_CHARS)));
			buf.WriteUInt32(t.seconds_remaining);
			buf.WriteInt32(t.event_type);
			buf.WriteLengthString(std::string(t.event_name, strnlen(t.event_name, TOB_NOTE_MAX_CHARS)));
		}

		auto outapp = new EQApplicationPacket(OP_DzExpeditionLockoutTimers, buf.size());
		outapp->WriteData(buf.buffer(), buf.size());
		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	ENCODE(OP_DzMemberList)
	{
		EQApplicationPacket* in = *p;
		*p = nullptr;

		if (in->size < sizeof(DynamicZoneMemberList_Struct)) {
			delete in;
			return;
		}
		auto emu = reinterpret_cast<DynamicZoneMemberList_Struct*>(in->pBuffer);
		uint32 max_count = (in->size - sizeof(DynamicZoneMemberList_Struct)) / sizeof(DynamicZoneMemberEntry_Struct);
		uint32 count = std::min(emu->member_count, max_count);

		SerializeBuffer buf;
		buf.WriteUInt32(emu->client_id);
		buf.WriteUInt32(count);
		for (uint32 i = 0; i < count; ++i) {
			const auto& m = emu->members[i];
			buf.WriteString(std::string(m.name, strnlen(m.name, sizeof(m.name) - 1)));
			buf.WriteUInt8(m.online_status);
			buf.WriteUInt8(1); // "Flagged" column: flagged (members the server added passed its checks)
			buf.WriteUInt8(1); // flag known (0 shows "Pending")
		}

		// keep the emu opcode so OP_DzMemberListStatus can forward here
		auto outapp = new EQApplicationPacket(in->GetOpcode(), buf.size());
		outapp->WriteData(buf.buffer(), buf.size());
		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	ENCODE(OP_DzMemberListStatus) { ENCODE_FORWARD(OP_DzMemberList); } // ToB merges by name; any count is fine

	ENCODE(OP_DzMemberListName)
	{
		ENCODE_LENGTH_EXACT(DynamicZoneMemberListName_Struct);
		SETUP_DIRECT_ENCODE(DynamicZoneMemberListName_Struct, structs::DynamicZoneMemberListName_Struct);

		OUT(client_id);
		eq->add_name = emu->add_name ? 1 : 0;
		strn0cpy(eq->name, emu->name, sizeof(eq->name)); // the name field starts 4 bytes into the struct

		FINISH_ENCODE();
	}

	ENCODE(OP_InspectBuffs)
	{
		EQApplicationPacket* in = *p;
		*p = nullptr;

		if (in->size != sizeof(InspectBuffs_Struct) && in->size != sizeof(InspectBuffsDetailed_Struct)) {
			LogNetcode("TOB::ENCODE(OP_InspectBuffs) wrong size [{}]", in->size);
			delete in;
			return;
		}
		auto emu = reinterpret_cast<InspectBuffs_Struct*>(in->pBuffer);
		auto ext = in->size == sizeof(InspectBuffsDetailed_Struct) ? reinterpret_cast<InspectBuffsDetailed_Struct*>(in->pBuffer) : nullptr;
		constexpr size_t slots = sizeof(emu->spell_id) / sizeof(emu->spell_id[0]);

		uint32 count = 0;
		for (size_t i = 0; i < slots; ++i) {
			if (emu->spell_id[i] > 0) {
				++count;
			}
		}

		SerializeBuffer buf;
		buf.WriteUInt32(count);
		buf.WriteUInt32(ext ? ext->target_id : 0);  // target spawn id: 0 -> client uses its current target's name
		buf.WriteUInt8(ext ? ext->only_mine : 0);  // "cast by you" header flag (/inspectbuffs mine)
		for (size_t i = 0; i < slots; ++i) {
			if (emu->spell_id[i] <= 0) {
				continue;
			}
			buf.WriteUInt32(emu->spell_id[i]);
			buf.WriteInt32(emu->tics_remaining[i]); // client: seconds = ticks * 6, < 0 = permanent
			buf.WriteUInt64(ext ? ext->caster_id[i] : 0); // caster id (0 = no " [caster]" suffix)
		}

		auto outapp = new EQApplicationPacket(OP_InspectBuffs, buf.size());
		outapp->WriteData(buf.buffer(), buf.size());
		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	ENCODE(OP_VetClaimReply)
	{
		EQApplicationPacket* in = *p;
		*p = nullptr;

		if (in->size != sizeof(VeteranClaim)) {
			LogNetcode("TOB::ENCODE(OP_VetClaimReply) wrong size [{}]", in->size);
			delete in;
			return;
		}
		auto emu = reinterpret_cast<VeteranClaim*>(in->pBuffer);

		SerializeBuffer buf;
		buf.WriteLengthString(std::string(emu->name, strnlen(emu->name, sizeof(emu->name))));
		buf.WriteUInt32(emu->claim_id);
		buf.WriteUInt8(0);
		if (emu->action == 0) {          // success: let the client decrement this claim by one
			buf.WriteUInt32(1);
			buf.WriteUInt32(emu->claim_id);
			buf.WriteUInt32(1);
		} else {
			buf.WriteUInt32(0);
		}
		buf.WriteUInt32(emu->action);    // 0 ok, 1/3 unavailable, 2 not allowed, 4 need free slot (ToB)

		auto outapp = new EQApplicationPacket(OP_VetClaimReply, buf.size());
		outapp->WriteData(buf.buffer(), buf.size());
		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	ENCODE(OP_VetRewardsAvaliable)
	{
		EQApplicationPacket* in = *p;
		*p = nullptr;

		constexpr uint32 tob_max_items = 15; // fixed array in the client entry, not clamped by the client
		constexpr uint32 emu_max_items = sizeof(InternalVeteranReward::items) / sizeof(InternalVeteranRewardItem);

		uint32 count = in->size / sizeof(InternalVeteranReward);
		auto rewards = reinterpret_cast<InternalVeteranReward*>(in->pBuffer);

		SerializeBuffer buf;
		buf.WriteUInt32(count);
		for (uint32 i = 0; i < count; ++i) {
			const auto& r = rewards[i];
			uint32 items = std::min({ r.claim_count, emu_max_items, tob_max_items });

			buf.WriteUInt32(r.claim_id);
			buf.WriteUInt32(r.number_available);
			buf.WriteUInt32(items);
			buf.WriteUInt8(1);  // enabled
			buf.WriteUInt32(0); // new in ToB, purpose unknown
			for (uint32 j = 0; j < items; ++j) {
				const auto& it = r.items[j];
				// written even when the name is empty, so the stream never goes out of step
				buf.WriteLengthString(std::string(it.item_name, strnlen(it.item_name, sizeof(it.item_name) - 1)));
				buf.WriteUInt32(it.item_id);
				buf.WriteUInt32(it.charges);
			}
		}

		auto outapp = new EQApplicationPacket(OP_VetRewardsAvaliable, buf.size());
		outapp->WriteData(buf.buffer(), buf.size());
		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}


	// ---- alt currency, group make leader/follow2, guild bank, adventure merchant ----

	// Zone to client: AdvLootCorpseItems_Struct followed by item_count AdvLootCorpseItem_Struct (see
	// Corpse::SendAdvancedLootItems). The item links arrive in server format and are converted here.
	ENCODE(OP_AdvLootCorpseItems)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		if (in->size < sizeof(AdvLootCorpseItems_Struct)) {
			LogNetcode("[{}] Malformed OP_AdvLootCorpseItems ({} bytes), dropped", __FUNCTION__, in->size);
			delete in;
			return;
		}

		auto header = reinterpret_cast<const AdvLootCorpseItems_Struct *>(in->pBuffer);
		const uint64 expected_size = sizeof(AdvLootCorpseItems_Struct) + uint64(header->item_count) * sizeof(AdvLootCorpseItem_Struct);
		if (in->size != expected_size) {
			LogNetcode("[{}] Malformed OP_AdvLootCorpseItems ({} bytes for {} items), dropped", __FUNCTION__, in->size, header->item_count);
			delete in;
			return;
		}

		auto items = reinterpret_cast<const AdvLootCorpseItem_Struct *>(in->pBuffer + sizeof(AdvLootCorpseItems_Struct));

		SerializeBuffer buf;
		buf.WriteUInt8(header->present);
		buf.WriteUInt8(header->shared);
		buf.WriteUInt32(header->unlock_seconds);
		buf.WriteUInt32(header->corpse_id);
		buf.WriteUInt32(header->item_count);
		buf.WriteString(std::string(header->corpse_name, strnlen(header->corpse_name, sizeof(header->corpse_name))));

		for (uint32 i = 0; i < header->item_count; ++i) {
			const AdvLootCorpseItem_Struct &item = items[i];

			structs::AdvLootItemTrailer_Struct trailer{};
			trailer.unknown004 = structs::ADVLOOT_TRAILER_UNKNOWN004;
			trailer.unknown008 = structs::ADVLOOT_TRAILER_UNKNOWN008;

			std::string link_out;
			ServerToTOBConvertLinks(link_out, std::string(item.item_link, strnlen(item.item_link, sizeof(item.item_link))), false);

			buf.WriteUInt64(item.item_id);
			buf.WriteUInt32(item.icon);
			buf.WriteUInt32(item.shared_quantity);
			buf.WriteUInt32(item.stack_size);
			buf.WriteUInt8(0);
			buf.WriteUInt32(0);
			buf.WriteUInt32(item.personal_quantity);
			buf.WriteString(std::string(item.item_name, strnlen(item.item_name, sizeof(item.item_name))));
			buf.WriteUInt32(trailer.unknown000);
			buf.WriteUInt32(trailer.unknown004);
			buf.WriteUInt32(trailer.unknown008);
			buf.WriteUInt32(trailer.unknown012);
			buf.WriteUInt32(trailer.unknown016);
			buf.WriteLengthString(link_out);
		}

		auto outapp = new EQApplicationPacket(OP_AdvLootCorpseItems, std::move(buf));
		delete in;
		dest->FastQueuePacket(&outapp, ack_req);
	}

	// OP_AdvLoot and OP_AdvLootMasterLooter, zone to client. The zone sends one AdvLoot*_Struct per subtype
	// (eq_packet_structs.h); the wire layouts are written here.
	static constexpr uint32 TOB_ADVLOOT_RESULT_ENTRY_COUNT = 1; // entries in a loot result; the client expects one
	static constexpr uint32 TOB_ADVLOOT_ASK_CORPSE_COUNT   = 1; // corpses listed in an ask; the server asks per corpse
	static constexpr uint8  TOB_ADVLOOT_COIN_RECEIVED      = 1; // the coin message is shown to the player

	// the body when the packet is exactly one T, otherwise nullptr
	template <typename T>
	static const T *TOBAdvLootBody(const EQApplicationPacket *in)
	{
		return in->size == sizeof(T) ? reinterpret_cast<const T *>(in->pBuffer) : nullptr;
	}

	static std::string TOBAdvLootName(const char *name, size_t field_size)
	{
		return std::string(name, strnlen(name, field_size));
	}

	ENCODE(OP_AdvLoot)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		SerializeBuffer buf;
		bool valid = in->size >= sizeof(uint16);
		const uint16 subtype = valid ? in->ReadUInt16(0) : 0;

		switch (subtype) {
		case AdvLootClearPending: {
			auto b = TOBAdvLootBody<AdvLootClearPending_Struct>(in);
			if (!b) { valid = false; break; }
			buf.WriteUInt16(b->subtype);
			buf.WriteUInt32(b->corpse_id);
			buf.WriteUInt64(b->item_id);
			buf.WriteUInt32(0);
			break;
		}
		case AdvLootCoin: {
			auto b = TOBAdvLootBody<AdvLootCoin_Struct>(in);
			if (!b) { valid = false; break; }
			buf.WriteUInt16(b->subtype);
			buf.WriteUInt8(b->split);
			buf.WriteUInt64(b->copper);
			buf.WriteUInt8(TOB_ADVLOOT_COIN_RECEIVED);
			buf.WriteUInt32(0); // no bonus suffix
			break;
		}
		case AdvLootLootResult: {
			auto b = TOBAdvLootBody<AdvLootResult_Struct>(in);
			if (!b) { valid = false; break; }
			buf.WriteUInt16(b->subtype);
			buf.WriteUInt16(b->result);
			buf.WriteUInt32(TOB_ADVLOOT_RESULT_ENTRY_COUNT);
			buf.WriteUInt32(b->sequence);
			buf.WriteUInt8(0);
			break;
		}
		case AdvLootGive: {
			auto b = TOBAdvLootBody<AdvLootGiveNotice_Struct>(in);
			if (!b) { valid = false; break; }
			buf.WriteUInt16(b->subtype);
			buf.WriteUInt32(b->transaction_id);
			buf.WriteUInt32(b->corpse_id);
			buf.WriteUInt64(b->item_id);
			buf.WriteUInt16(b->quantity);
			buf.WriteUInt8(b->leave);
			buf.WriteString(TOBAdvLootName(b->player_name, sizeof(b->player_name)));
			break;
		}
		case AdvLootAsk: {
			auto b = TOBAdvLootBody<AdvLootAskNotice_Struct>(in);
			if (!b) { valid = false; break; }
			buf.WriteUInt16(b->subtype);
			buf.WriteUInt32(b->transaction_id);
			buf.WriteUInt64(b->item_id);
			buf.WriteUInt32(b->state);
			buf.WriteUInt8(0);
			buf.WriteUInt32(b->time_left);
			buf.WriteUInt32(TOB_ADVLOOT_ASK_CORPSE_COUNT);
			buf.WriteUInt32(b->corpse_id);
			buf.WriteUInt32(b->quantity);
			break;
		}
		case AdvLootState: {
			auto b = TOBAdvLootBody<AdvLootStateNotice_Struct>(in);
			if (!b) { valid = false; break; }
			buf.WriteUInt16(b->subtype);
			buf.WriteUInt32(b->transaction_id);
			buf.WriteUInt32(b->time_left);
			buf.WriteUInt32(b->state);
			buf.WriteUInt8(0);
			break;
		}
		case AdvLootRollResult: {
			if (in->size < sizeof(AdvLootRollResult_Struct)) { valid = false; break; }
			auto b = reinterpret_cast<const AdvLootRollResult_Struct *>(in->pBuffer);
			if (in->size != sizeof(AdvLootRollResult_Struct) + uint64(b->roller_count) * sizeof(AdvLootRoller_Struct)) {
				valid = false;
				break;
			}
			auto rollers = reinterpret_cast<const AdvLootRoller_Struct *>(in->pBuffer + sizeof(AdvLootRollResult_Struct));
			buf.WriteUInt16(b->subtype);
			buf.WriteUInt32(b->transaction_id);
			buf.WriteUInt32(b->best_choice);
			buf.WriteUInt8(0);
			buf.WriteUInt16(b->roller_count);
			for (uint16 i = 0; i < b->roller_count; ++i) {
				buf.WriteUInt16(rollers[i].roll);
				buf.WriteString(TOBAdvLootName(rollers[i].name, sizeof(rollers[i].name)));
			}
			break;
		}
		case AdvLootSync: {
			auto b = TOBAdvLootBody<AdvLootSyncReply_Struct>(in);
			if (!b) { valid = false; break; }
			// no corpses, no transactions, no choices; the corpses follow one by one
			buf.WriteUInt16(b->subtype);
			buf.WriteUInt32(0);
			buf.WriteString(TOBAdvLootName(b->master_looter_name, sizeof(b->master_looter_name)));
			buf.WriteUInt32(0);
			buf.WriteUInt8(0);
			buf.WriteUInt8(0);
			break;
		}
		default:
			valid = false;
			break;
		}

		if (!valid) {
			LogNetcode("[{}] Malformed OP_AdvLoot subtype [{}] ({} bytes), dropped", __FUNCTION__, subtype, in->size);
			delete in;
			return;
		}

		auto outapp = new EQApplicationPacket(OP_AdvLoot, std::move(buf));
		delete in;
		dest->FastQueuePacket(&outapp, ack_req);
	}

	ENCODE(OP_AdvLootMasterLooter)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		auto b = TOBAdvLootBody<AdvLootMasterLooter_Struct>(in);
		if (!b) {
			LogNetcode("[{}] Malformed OP_AdvLootMasterLooter ({} bytes), dropped", __FUNCTION__, in->size);
			delete in;
			return;
		}

		SerializeBuffer buf;
		buf.WriteUInt32(0);
		buf.WriteString(TOBAdvLootName(b->name, sizeof(b->name)));

		auto outapp = new EQApplicationPacket(OP_AdvLootMasterLooter, std::move(buf));
		delete in;
		dest->FastQueuePacket(&outapp, ack_req);
	}

	// Client to server: the layout depends on the subtype. A request too short for its subtype is invalidated;
	// subtypes the server has no handler for are passed on with the fields zero.
	DECODE(OP_AdvLoot)
	{
		DECODE_LENGTH_ATLEAST(structs::AdvLootSubtype_Struct);

		const unsigned char *buf  = __packet->pBuffer;
		const size_t         size = __packet->size;
		const uint16 wire_subtype = reinterpret_cast<const structs::AdvLootSubtype_Struct *>(buf)->subtype;

		AdvLootRequest_Struct request{};
		bool valid = true;

		switch (static_cast<structs::TOBAdvLootSubtype>(wire_subtype)) {
		case structs::TOBAdvLootSubtype::Sync:
			request.subtype = AdvLootSync;
			break;
		case structs::TOBAdvLootSubtype::Leave: {
			valid = size >= sizeof(structs::AdvLootLeave_Struct);
			if (valid) {
				auto eq = reinterpret_cast<const structs::AdvLootLeave_Struct *>(buf);
				request.subtype   = AdvLootLeave;
				request.item_id   = static_cast<uint32>(eq->item_id); // the client sends 64 bits; item ids fit in 32
				request.corpse_id = eq->corpse_id;
				request.quantity  = eq->quantity;
			}
			break;
		}
		case structs::TOBAdvLootSubtype::Loot: {
			valid = size >= sizeof(structs::AdvLootLoot_Struct);
			if (valid) {
				auto eq = reinterpret_cast<const structs::AdvLootLoot_Struct *>(buf);
				request.subtype   = AdvLootLoot;
				request.item_id   = static_cast<uint32>(eq->item_id);
				request.corpse_id = eq->corpse_id;
				request.quantity  = eq->quantity;
				request.sequence  = eq->sequence;
			}
			break;
		}
		case structs::TOBAdvLootSubtype::Choice: {
			valid = size >= sizeof(structs::AdvLootChoice_Struct);
			if (valid) {
				auto eq = reinterpret_cast<const structs::AdvLootChoice_Struct *>(buf);
				request.subtype        = AdvLootChoice;
				request.transaction_id = eq->transaction_id;
				request.corpse_id      = eq->corpse_id;
				request.item_id        = static_cast<uint32>(eq->item_id);
				request.choice         = eq->choice;
			}
			break;
		}
		case structs::TOBAdvLootSubtype::Ask: {
			valid = size >= sizeof(structs::AdvLootAsk_Struct);
			if (valid) {
				auto eq = reinterpret_cast<const structs::AdvLootAsk_Struct *>(buf);
				request.subtype        = AdvLootAsk;
				request.transaction_id = eq->transaction_id;
				request.item_id        = static_cast<uint32>(eq->item_id);
				request.state          = eq->state;
				request.corpse_id      = eq->corpse_id;
			}
			break;
		}
		case structs::TOBAdvLootSubtype::Give: {
			// the player name follows the fixed part
			valid = size >= sizeof(structs::AdvLootGive_Struct);
			if (valid) {
				auto eq = reinterpret_cast<const structs::AdvLootGive_Struct *>(buf);
				request.subtype        = AdvLootGive;
				request.transaction_id = eq->transaction_id;
				request.corpse_id      = eq->corpse_id;
				request.item_id        = static_cast<uint32>(eq->item_id);
				request.quantity       = eq->quantity;
				request.leave          = eq->leave != 0 ? 1 : 0;

				std::string player_name;
				__packet->SetReadPosition(sizeof(structs::AdvLootGive_Struct));
				__packet->ReadString(player_name, TOB_NAME_MAX_CHARS);
				valid = !__packet->ReadError(); // an unterminated name is malformed
				strn0cpy(request.player_name, player_name.c_str(), sizeof(request.player_name));
			}
			break;
		}
		case structs::TOBAdvLootSubtype::State: {
			valid = size >= sizeof(structs::AdvLootState_Struct);
			if (valid) {
				auto eq = reinterpret_cast<const structs::AdvLootState_Struct *>(buf);
				request.subtype        = AdvLootState;
				request.transaction_id = eq->transaction_id;
				request.state          = eq->state;
			}
			break;
		}
		case structs::TOBAdvLootSubtype::Roll: {
			valid = size >= sizeof(structs::AdvLootRoll_Struct);
			if (valid) {
				auto eq = reinterpret_cast<const structs::AdvLootRoll_Struct *>(buf);
				request.subtype        = AdvLootRoll;
				request.transaction_id = eq->transaction_id;
			}
			break;
		}
		default:
			request.subtype = wire_subtype;
			break;
		}

		if (!valid) {
			LogNetcode("TOB OP_AdvLoot subtype [{}] too short ([{}] bytes)", wire_subtype, size);
			__packet->SetOpcode(OP_Unknown);
			return;
		}

		delete[] __packet->pBuffer;
		__packet->size    = sizeof(AdvLootRequest_Struct);
		__packet->pBuffer = new unsigned char[__packet->size];
		memcpy(__packet->pBuffer, &request, sizeof(request));
	}

	DECODE(OP_AdvLootMisc)
	{
		DECODE_LENGTH_ATLEAST(structs::AdvLootSubtype_Struct);

		const unsigned char *buf  = __packet->pBuffer;
		const size_t         size = __packet->size;
		const uint16 wire_subtype = reinterpret_cast<const structs::AdvLootSubtype_Struct *>(buf)->subtype;

		AdvLootMisc_Struct request{};

		if (wire_subtype == static_cast<uint16>(structs::TOBAdvLootMiscSubtype::FreeGrab)) {
			if (size < sizeof(structs::AdvLootMiscFreeGrab_Struct)) {
				LogNetcode("TOB OP_AdvLootMisc free grab too short ([{}] bytes)", size);
				__packet->SetOpcode(OP_Unknown);
				return;
			}

			auto eq = reinterpret_cast<const structs::AdvLootMiscFreeGrab_Struct *>(buf);
			request.subtype        = AdvLootMiscFreeGrab;
			request.transaction_id = eq->transaction_id;
			request.corpse_id      = eq->corpse_id;
		}
		else {
			request.subtype = wire_subtype; // no handler on the server
		}

		delete[] __packet->pBuffer;
		__packet->size    = sizeof(AdvLootMisc_Struct);
		__packet->pBuffer = new unsigned char[__packet->size];
		memcpy(__packet->pBuffer, &request, sizeof(request));
	}

	ENCODE(OP_AltCurrency)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		if (in->size < sizeof(uint32)) {
			delete in;
			return;
		}

		const uint32 action = *reinterpret_cast<uint32 *>(in->pBuffer);

		if (action == AlternateCurrencyMode::Populate) {
			if (in->size < sizeof(AltCurrencyPopulate_Struct)) {
				delete in;
				return;
			}
			auto *emu = reinterpret_cast<AltCurrencyPopulate_Struct *>(in->pBuffer);

			// clamp the count to what is actually in the emu buffer
			const uint32 max_entries = (in->size - sizeof(AltCurrencyPopulate_Struct)) / sizeof(AltCurrencyPopulateEntry_Struct);
			const uint32 count       = std::min<uint32>(emu->count, max_entries);

			SerializeBuffer buffer;
			buffer.WriteUInt32(emu->opcode);
			buffer.WriteUInt32(count);
			for (uint32 i = 0; i < count; ++i) {
				buffer.WriteUInt32(emu->entries[i].currency_number);
				buffer.WriteUInt32(emu->entries[i].unknown00);
				buffer.WriteUInt32(emu->entries[i].currency_number2);
				buffer.WriteUInt32(emu->entries[i].item_id);
				buffer.WriteUInt32(emu->entries[i].item_icon);
				buffer.WriteUInt32(emu->entries[i].stack_size);
				buffer.WriteUInt32(0);  // sort_order: new ToB field, meaning not confirmed
				buffer.WriteUInt8((emu->entries[i].stack_size > 0) ? 1 : 0); // display
			}
			auto outapp = new EQApplicationPacket(OP_AltCurrency, std::move(buffer));
			dest->FastQueuePacket(&outapp, ack_req);
		}
		else {
			if (in->size < sizeof(AltCurrencyUpdate_Struct)) {
				delete in;
				return;
			}
			auto *emu   = reinterpret_cast<AltCurrencyUpdate_Struct *>(in->pBuffer);
			auto outapp = new EQApplicationPacket(OP_AltCurrency, sizeof(structs::AltCurrencyUpdate_Struct));
			auto *eq    = reinterpret_cast<structs::AltCurrencyUpdate_Struct *>(outapp->pBuffer);

			eq->opcode = emu->opcode;
			strn0cpy(eq->name, emu->name, sizeof(eq->name));
			eq->currency_number = emu->currency_number;
			eq->unknown072      = emu->unknown072;
			eq->delta           = 0;            // server does not know the delta; message is suppressed below
			eq->amount          = emu->amount;  // ToB offset 0x50
			eq->show_message    = 0;            // 1 = client prints "You gained %delta %name" (eqstr 0x3426)
			dest->FastQueuePacket(&outapp, ack_req);
		}

		delete in;
	}

	DECODE(OP_GroupMakeLeader)
	{
		DECODE_LENGTH_ATLEAST(structs::GroupMakeLeader_Struct);
		SETUP_DIRECT_DECODE(GroupMakeLeader_Struct, structs::GroupMakeLeader_Struct);  // emu buffer is zeroed (456)

		emu->Unknown000 = eq->unknown000;
		strn0cpy(emu->CurrentLeader, eq->current_leader, sizeof(emu->CurrentLeader));
		strn0cpy(emu->NewLeader, eq->new_leader, sizeof(emu->NewLeader));

		FINISH_DIRECT_DECODE();
	}

	// The client sends 0x4ddc (OP_GroupFollow) or 0x29d3 with the same 176-byte body.
	// Uses the same decoder as OP_GroupFollow.
	DECODE(OP_GroupFollow2) { DECODE_FORWARD(OP_GroupFollow); }

	ENCODE(OP_AdventureMerchantSell)
	{
		ENCODE_LENGTH_EXACT(Adventure_Sell_Struct);
		SETUP_DIRECT_ENCODE(Adventure_Sell_Struct, structs::Adventure_Sell_Struct);

		eq->merchant_type  = 1;   // 1 = LDoN
		OUT(npcid);
		eq->inventory_slot = ServerToTOBTypelessSlot(emu->slot, EQ::invtype::typePossessions);
		OUT(charges);
		OUT(sell_price);

		FINISH_ENCODE();
	}

	DECODE(OP_AdventureMerchantSell)
	{
		DECODE_LENGTH_EXACT(structs::Adventure_Sell_Struct);
		SETUP_DIRECT_DECODE(Adventure_Sell_Struct, structs::Adventure_Sell_Struct);

		IN(npcid);
		emu->slot = TOBToServerTypelessSlot(eq->inventory_slot, invtype::typePossessions);
		IN(charges);
		IN(sell_price);

		FINISH_DIRECT_DECODE();
	}

	// OP_GuildBank: the encoder only reads what the emu packet holds, and the decoder checks the size before reading the action.
	ENCODE(OP_GuildBank)
	{
		auto in = *p;
		*p = nullptr;

		if (in->size < sizeof(uint32)) {
			delete in;
			return;
		}

		auto outapp = new EQApplicationPacket(OP_GuildBank, in->size + 4); // all of them are 4 bytes bigger

		switch (in->ReadUInt32()) {
		case EmuGuildBankAcknowledge:
			if (in->size >= 8) {
				outapp->WriteUInt32(TOBGuildBankAcknowledge);
				outapp->WriteUInt32(in->ReadUInt32());
				outapp->WriteUInt32(0);
			}
			break;
		case EmuGuildBankDeposit: // ack
			if (in->size >= 12) {
				outapp->WriteUInt32(TOBGuildBankDeposit);
				outapp->WriteUInt32(in->ReadUInt32());
				outapp->WriteUInt32(0);
				outapp->WriteUInt32(in->ReadUInt32());
			}
			break;
		case EmuGuildBankItemUpdate: {
			if (in->size < sizeof(GuildBankItemUpdate_Struct) ||
			    outapp->size < sizeof(structs::GuildBankItemUpdate_Struct)) {
				break;
			}
			auto emu = (GuildBankItemUpdate_Struct *)in->pBuffer;
			auto eq  = (structs::GuildBankItemUpdate_Struct *)outapp->pBuffer;
			eq->action = TOBGuildBankItemUpdate;
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

	DECODE(OP_GuildBank)
	{
		if (__packet->size < sizeof(uint32)) {
			__packet->SetOpcode(OP_Unknown);
			return;
		}
		// all actions are 1 off due to the removal of one of the enums
		switch (__packet->ReadUInt32()) {
		case TOBGuildBankPromote: {
			DECODE_LENGTH_EXACT(structs::GuildBankPromote_Struct);
			SETUP_DIRECT_DECODE(GuildBankPromote_Struct, structs::GuildBankPromote_Struct);
			emu->Action = EmuGuildBankPromote;
			IN(Unknown04);
			IN(Slot);
			IN(Slot2);
			FINISH_DIRECT_DECODE();
			return;
		}
		case TOBGuildBankViewItem: {
			DECODE_LENGTH_EXACT(structs::GuildBankViewItem_Struct);
			SETUP_DIRECT_DECODE(GuildBankViewItem_Struct, structs::GuildBankViewItem_Struct);
			emu->Action = EmuGuildBankViewItem;
			IN(Unknown04);
			IN(SlotID);
			IN(Area);
			IN(Unknown12);
			IN(Unknown16);
			FINISH_DIRECT_DECODE();
			return;
		}
		case TOBGuildBankDeposit: {
			__packet->WriteUInt32(EmuGuildBankDeposit);
			return;
		}
		case TOBGuildBankPermissions: {
			DECODE_LENGTH_EXACT(structs::GuildBankPermissions_Struct);
			SETUP_DIRECT_DECODE(GuildBankPermissions_Struct, structs::GuildBankPermissions_Struct);
			emu->Action = EmuGuildBankPermissions;
			IN(Unknown04);
			IN(SlotID);
			IN(Unknown10);
			IN(ItemID);
			IN(Permissions);
			strn0cpy(emu->MemberName, eq->MemberName, sizeof(emu->MemberName));
			FINISH_DIRECT_DECODE();
			return;
		}
		case TOBGuildBankWithdraw:
		case TOBGuildBankSplitStacks:
		case TOBGuildBankMergeStacks: {
			const uint32 wire_action = *reinterpret_cast<uint32 *>(__packet->pBuffer);
			DECODE_LENGTH_EXACT(structs::GuildBankWithdrawItem_Struct);
			SETUP_DIRECT_DECODE(GuildBankWithdrawItem_Struct, structs::GuildBankWithdrawItem_Struct);
			emu->Action = wire_action + 1;   // server: 7 withdraw, 8 split, 9 merge
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

	// ---- LFGuild, projectile, tribute info ----

	ENCODE(OP_LFGuild)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		if (in->size < sizeof(uint32)) {
			delete in;
			return;
		}

		const uchar *src  = in->pBuffer;
		const uint32 size = in->size;
		uint32 command = 0;
		memcpy(&command, src, sizeof(command));

		SerializeBuffer buf;

		switch (command) {
		case structs::LFGuildPlayerStatus: {   // player status. Body is byte-identical to the server struct; only header + u64 time differ.
			if (size < sizeof(::LFGuild_PlayerToggle_Struct)) {
				LogNetcode("TOB::ENCODE(OP_LFGuild) player toggle too short [{}]", size);
				delete in;
				return;
			}
			auto emu = reinterpret_cast<const ::LFGuild_PlayerToggle_Struct *>(src);
			structs::LFGuild_PlayerToggle_Struct eq{};
			eq.Command = structs::LFGuildPlayerStatus;
			// ToB comment field is 524 (server Comment[256] + Unknown328[268]); the server only fills Comment
			strn0cpy(eq.Comment, emu->Comment, sizeof(emu->Comment));
			eq.TimeZone   = static_cast<int32>(emu->TimeZone);
			eq.Toggle     = emu->Toggle;
			eq.TimePosted = emu->TimePosted;    // u32 unix time -> u64
			auto outapp = new EQApplicationPacket(OP_LFGuild, sizeof(eq));
			memcpy(outapp->pBuffer, &eq, sizeof(eq));
			dest->FastQueuePacket(&outapp, ack_req);
			delete in;
			return;
		}
		case structs::LFGuildGuildStatus: {   // guild status
			if (size < sizeof(::LFGuild_GuildToggle_Struct)) {
				LogNetcode("TOB::ENCODE(OP_LFGuild) guild toggle too short [{}]", size);
				delete in;
				return;
			}
			auto emu = reinterpret_cast<const ::LFGuild_GuildToggle_Struct *>(src);
			structs::LFGuild_GuildToggle_Struct eq{};
			eq.Command = structs::LFGuildGuildStatus;
			// ToB comment field is 512; the server only fills Comment[256]
			strn0cpy(eq.Comment, emu->Comment, sizeof(emu->Comment));
			eq.FromLevel  = emu->FromLevel;
			eq.ToLevel    = emu->ToLevel;
			eq.Classes    = emu->Classes;
			eq.AACount    = emu->AACount;
			eq.TimeZone   = static_cast<int32>(emu->TimeZone);
			eq.Toggle     = emu->Toggle;
			eq.TimePosted = emu->TimePosted;
			strn0cpy(eq.Name, emu->Name, sizeof(eq.Name));
			auto outapp = new EQApplicationPacket(OP_LFGuild, sizeof(eq));
			memcpy(outapp->pBuffer, &eq, sizeof(eq));
			dest->FastQueuePacket(&outapp, ack_req);
			delete in;
			return;
		}
		case structs::LFGuildPlayerSearch: {   // player search results. Emu wire: u32 3, u32 ?, u32 count, entries.
			struct Entry { std::string name, comment; uint32 level, cls, aa, tz; };
			std::vector<Entry> entries;
			in->SetReadPosition(0);
			in->ReadSkip(EMU_LFGUILD_HEADER_SIZE);
			uint32 count = in->ReadUInt32();
			if (!in->ReadError()) {
				count = std::min<uint32>(count, TOB_LFGUILD_MAX_RESULTS);
				for (uint32 i = 0; i < count; ++i) {
					Entry e{};
					in->ReadString(e.name, TOB_NAME_MAX_CHARS);
					e.level = in->ReadUInt32();
					e.cls   = in->ReadUInt32();
					e.aa    = in->ReadUInt32();
					e.tz    = in->ReadUInt32();
					in->ReadString(e.comment, TOB_NOTE_MAX_CHARS);
					if (in->ReadError()) {
						break;
					}
					if (e.cls > TOB_LFGUILD_MAX_CLASS) { e.cls = 0; }
					if (static_cast<int32>(e.tz) < TOB_TIMEZONE_NONE || static_cast<int32>(e.tz) > TOB_TIMEZONE_MAX) { e.tz = static_cast<uint32>(TOB_TIMEZONE_NONE); }
					entries.push_back(std::move(e));
				}
			}
			buf.WriteUInt32(structs::LFGuildPlayerSearch);
			TOBWriteZeros(buf, structs::LFGuild_HeaderSize - sizeof(uint32)); // command, then the unknown header bytes
			buf.WriteUInt32(static_cast<uint32>(entries.size()));
			for (const auto &e : entries) {
				buf.WriteString(e.name);
				buf.WriteUInt32(e.level);
				buf.WriteUInt32(e.cls);
				buf.WriteUInt32(e.aa);
				buf.WriteUInt32(e.tz);
				buf.WriteString(e.comment);
			}
			break;
		}
		case structs::LFGuildGuildSearch: {   // guild search results: entries {cstr name, u32 timezone, cstr comment}
			struct Entry { std::string name, comment; uint32 tz; };
			std::vector<Entry> entries;
			in->SetReadPosition(0);
			in->ReadSkip(EMU_LFGUILD_HEADER_SIZE);
			uint32 count = in->ReadUInt32();
			if (!in->ReadError()) {
				count = std::min<uint32>(count, TOB_LFGUILD_MAX_RESULTS);
				for (uint32 i = 0; i < count; ++i) {
					Entry e{};
					in->ReadString(e.name, TOB_NAME_MAX_CHARS);
					e.tz = in->ReadUInt32();
					in->ReadString(e.comment, TOB_NOTE_MAX_CHARS);
					if (in->ReadError()) {
						break;
					}
					// ToB indexes the timezone-name table with this value WITHOUT a range check
					if (static_cast<int32>(e.tz) < TOB_TIMEZONE_NONE || static_cast<int32>(e.tz) > TOB_TIMEZONE_MAX) { e.tz = static_cast<uint32>(TOB_TIMEZONE_NONE); }
					entries.push_back(std::move(e));
				}
			}
			buf.WriteUInt32(structs::LFGuildGuildSearch);
			TOBWriteZeros(buf, structs::LFGuild_HeaderSize - sizeof(uint32)); // command, then the unknown header bytes
			buf.WriteUInt32(static_cast<uint32>(entries.size()));
			for (const auto &e : entries) {
				buf.WriteString(e.name);
				buf.WriteUInt32(e.tz);
				buf.WriteString(e.comment);
			}
			break;
		}
		default: {  // 2, 5 (ignored by the client), 6 (show window), anything else: widen the header 8 -> 0x18
			const uint32 tail = size > EMU_LFGUILD_HEADER_SIZE ? size - EMU_LFGUILD_HEADER_SIZE : 0;
			buf.WriteUInt32(command);
			TOBWriteZeros(buf, structs::LFGuild_HeaderSize - sizeof(command));
			TOBWriteBytes(buf, src + EMU_LFGUILD_HEADER_SIZE, tail);
			break;
		}
		}

		// all commands that are rebuilt through the buffer
		auto outapp = new EQApplicationPacket(OP_LFGuild, std::move(buf));
		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	DECODE(OP_LFGuild)
	{
		const uint32 size = __packet->size;
		if (size < sizeof(uint32)) {
			__packet->SetOpcode(OP_Unknown);
			return;
		}
		unsigned char *eq_buffer = __packet->pBuffer;
		uint32 command = 0;
		memcpy(&command, eq_buffer, sizeof(command));

		auto replace = [&](uint32 new_size) -> unsigned char * {
			__packet->size    = new_size;
			__packet->pBuffer = new unsigned char[new_size]{};
			return __packet->pBuffer;
		};

		switch (command) {
		case structs::LFGuildPlayerStatus: {
			if (size != sizeof(structs::LFGuild_PlayerToggle_Struct)) { break; }
			auto eq  = reinterpret_cast<const structs::LFGuild_PlayerToggle_Struct *>(eq_buffer);
			auto emu = reinterpret_cast<::LFGuild_PlayerToggle_Struct *>(replace(sizeof(::LFGuild_PlayerToggle_Struct)));
			emu->Command = structs::LFGuildPlayerStatus;
			strn0cpy(emu->Comment, eq->Comment, sizeof(emu->Comment));
			emu->TimeZone   = eq->TimeZone;
			emu->Toggle     = eq->Toggle;
			emu->TimePosted = static_cast<uint32>(eq->TimePosted);
			delete[] eq_buffer;
			return;
		}
		case structs::LFGuildGuildStatus: {
			if (size != sizeof(structs::LFGuild_GuildToggle_Struct)) { break; }
			auto eq  = reinterpret_cast<const structs::LFGuild_GuildToggle_Struct *>(eq_buffer);
			auto emu = reinterpret_cast<::LFGuild_GuildToggle_Struct *>(replace(sizeof(::LFGuild_GuildToggle_Struct)));
			emu->Command = structs::LFGuildGuildStatus;
			strn0cpy(emu->Comment, eq->Comment, sizeof(emu->Comment));   // clamp to the server's 256
			emu->FromLevel  = eq->FromLevel;
			emu->ToLevel    = eq->ToLevel;
			emu->Classes    = eq->Classes;
			emu->AACount    = eq->AACount;
			emu->TimeZone   = eq->TimeZone;
			emu->Toggle     = eq->Toggle;
			emu->TimePosted = static_cast<uint32>(eq->TimePosted);
			strn0cpy(emu->Name, eq->Name, sizeof(emu->Name));
			delete[] eq_buffer;
			return;
		}
		case structs::LFGuildPlayerSearch: {
			if (size != sizeof(structs::LFGuild_SearchPlayer_Struct)) { break; }
			auto eq  = reinterpret_cast<const structs::LFGuild_SearchPlayer_Struct *>(eq_buffer);
			auto emu = reinterpret_cast<::LFGuild_SearchPlayer_Struct *>(replace(sizeof(::LFGuild_SearchPlayer_Struct)));
			emu->Command   = structs::LFGuildPlayerSearch;
			emu->FromLevel = eq->FromLevel;
			emu->ToLevel   = eq->ToLevel;
			emu->MinAA     = eq->MinAA;
			emu->TimeZone  = static_cast<uint32>(eq->TimeZone);
			emu->Classes   = eq->Classes;
			delete[] eq_buffer;
			return;
		}
		case structs::LFGuildGuildSearch: {
			if (size != sizeof(structs::LFGuild_SearchGuild_Struct)) { break; }
			auto eq  = reinterpret_cast<const structs::LFGuild_SearchGuild_Struct *>(eq_buffer);
			auto emu = reinterpret_cast<::LFGuild_SearchGuild_Struct *>(replace(sizeof(::LFGuild_SearchGuild_Struct)));
			emu->Command  = structs::LFGuildGuildSearch;
			emu->Level    = eq->Level;
			emu->AAPoints = eq->AAPoints;
			emu->TimeZone = static_cast<uint32>(eq->TimeZone);
			emu->Class    = eq->Class;
			delete[] eq_buffer;
			return;
		}
		default:
			break;
		}

		LogNetcode("TOB::DECODE(OP_LFGuild) unexpected command [{}] size [{}]", command, size);
		__packet->SetOpcode(OP_Unknown);
	}




	ENCODE(OP_SomeItemPacketMaybe)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		if (in->size != sizeof(Arrow_Struct)) {
			LogNetcode("TOB::ENCODE(OP_SomeItemPacketMaybe) wrong size [{}] expected [{}]", in->size, sizeof(Arrow_Struct));
			delete in;
			return;
		}
		auto emu = reinterpret_cast<const Arrow_Struct *>(in->pBuffer);

		// ToB has no model-name string any more: it formats "IT%u" from a number.
		char model[sizeof(emu->model_name) + 1] = {};
		memcpy(model, emu->model_name, sizeof(emu->model_name));
		uint32 it_number = 0;
		if ((model[0] == 'I' || model[0] == 'i') && (model[1] == 'T' || model[1] == 't')) {
			it_number = static_cast<uint32>(strtoul(model + 2, nullptr, 10));
		} else if (emu->item_id <= 0xffff) {
			it_number = emu->item_id;           // EQEmu comment: item_id is "1 to about 150ish" (the IT number)
		}

		SerializeBuffer buf;
		buf.WriteFloat(emu->src_y);             // S[0]
		buf.WriteFloat(emu->src_x);             // S[1]
		buf.WriteFloat(emu->src_z);             // S[2]
		buf.WriteUInt32(0);                     // S[3..5] vector, zeroed by the ctor when tiny
		buf.WriteUInt32(0);
		buf.WriteUInt32(0);
		buf.WriteFloat(emu->velocity);          // S[6]
		buf.WriteFloat(emu->launch_angle);      // S[7]
		buf.WriteFloat(emu->tilt);              // S[8]
		buf.WriteUInt32(0);                     // S[9]
		buf.WriteUInt32(0);                     // S[10]
		buf.WriteFloat(emu->arc);               // S[11]
		buf.WriteUInt32(0);                     // S[12] unknown
		buf.WriteUInt32(0);                     // S[13] unknown
		buf.WriteUInt32(0);                     // S[14] unknown
		buf.WriteUInt32(emu->source_id);        // S[15] source spawn
		buf.WriteUInt32(emu->target_id);        // S[16] target spawn
		buf.WriteUInt32(it_number);             // S[17] "IT%u" model
		buf.WriteUInt32(0);                     // S[18] (unknown)
		buf.WriteUInt32(0);                     // S[19] (unknown)
		buf.WriteUInt8(0);                      // 0x50 (unknown)
		buf.WriteUInt8(emu->item_type); // 0x51 item type
		buf.WriteUInt8(emu->skill); // 0x52 skill
		buf.WriteUInt8(0);                      // 0x53 (unknown flag)

		auto outapp = new EQApplicationPacket(OP_SomeItemPacketMaybe, buf.size());   // 84
		outapp->WriteData(buf.buffer(), buf.size());
		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}
	// Optional trailer the zone appends after the tribute name's NUL for ToB clients (zone/tribute.cpp):
	//   uint32 magic = EQ::TRIBUTE_ITEMS_TRAILER_MAGIC; uint32 count; EQ::InternalSerializedItem_Struct items[count];
	// The instance pointers are only valid because the zone's QueuePacket encodes synchronously.
	static std::vector<EQ::InternalSerializedItem_Struct> TOBReadTributeItemTrailer(EQApplicationPacket *in, size_t name_off)
	{
		std::vector<EQ::InternalSerializedItem_Struct> items;
		if (in->size <= name_off) {
			return items;
		}
		auto nul = static_cast<const uint8 *>(memchr(in->pBuffer + name_off, 0, in->size - name_off));
		if (!nul) {
			return items;
		}
		in->SetReadPosition(static_cast<uint32>(nul + 1 - in->pBuffer));
		const uint32 magic = in->ReadUInt32();
		const uint32 n     = in->ReadUInt32();
		if (!in->ReadError() && magic == EQ::TRIBUTE_ITEMS_TRAILER_MAGIC &&
		    n <= MAX_TRIBUTE_TIERS &&
		    in->ReadRemaining() >= n * sizeof(EQ::InternalSerializedItem_Struct)) {
			items.resize(n);
			memcpy(items.data(), in->pBuffer + in->GetReadPosition(), n * sizeof(EQ::InternalSerializedItem_Struct)); // unaligned-safe
		}
		return items;
	}

	static inline uint32 TOBFromNet32(uint32 v)   // server fills TributeAbility_Struct with htonl()
	{
		return (v >> 24) | ((v >> 8) & 0x0000ff00u) | ((v << 8) & 0x00ff0000u) | (v << 24);
	}

	ENCODE(OP_TributeInfo)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		if (in->size < sizeof(TributeAbility_Struct)) {
			LogNetcode("TOB::ENCODE(OP_TributeInfo) too short [{}]", in->size);
			delete in;
			return;
		}
		auto emu = reinterpret_cast<const TributeAbility_Struct *>(in->pBuffer);

		uint32 tier_count = std::min<uint32>(TOBFromNet32(emu->tier_count), MAX_TRIBUTE_TIERS);

		// optional item trailer after the NUL-terminated name
		auto items = TOBReadTributeItemTrailer(in, sizeof(TributeAbility_Struct));

		SerializeBuffer buf;
		buf.WriteUInt32(TOBFromNet32(emu->tribute_id));
		buf.WriteUInt32(tier_count);
		for (uint32 i = 0; i < tier_count; ++i) {
			const uint32 item_id = TOBFromNet32(emu->tiers[i].tribute_item_id);
			// tier+0 item id, +4 level, +8 cost (level first showed the item id in the Level column)
			buf.WriteUInt32(item_id);
			buf.WriteUInt32(TOBFromNet32(emu->tiers[i].level));
			buf.WriteUInt32(TOBFromNet32(emu->tiers[i].cost));

			const EQ::ItemInstance *inst = (i < items.size()) ? static_cast<const EQ::ItemInstance *>(items[i].inst) : nullptr;
			if (inst && inst->GetItem()) {
				SerializeItem(buf, inst, items[i].slot_id, 0, ItemPacketInvalid);   // same stream as OP_ItemPreviewRequest
			} else {
				// No instance: emit a complete stream for a blank definition so the reader stays in sync.
				// The client then lists level/cost with an empty name. (ItemInstance copies the ItemData,
				// so the stack object is safe.)
				EQ::ItemData blank;
				memset(&blank, 0, sizeof(blank));
				blank.ID = item_id;
				EQ::ItemInstance tmp(&blank, 0);
				SerializeItem(buf, &tmp, 0, 0, ItemPacketInvalid);
			}
		}
		buf.WriteUInt8(0);                                          // flag

		auto outapp = new EQApplicationPacket(OP_TributeInfo, buf.size());
		outapp->WriteData(buf.buffer(), buf.size());
		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	// alt currency sell-back and group roles
	ENCODE(OP_AltCurrencySell)
	{
		ENCODE_LENGTH_EXACT(AltCurrencySellItem_Struct);
		SETUP_DIRECT_ENCODE(AltCurrencySellItem_Struct, structs::AltCurrencySellItem_Struct);

		OUT(merchant_entity_id);
		eq->inventory_slot = ServerToTOBTypelessSlot(emu->slot_id, EQ::invtype::typePossessions);
		eq->unknown010 = 0;
		OUT(charges);
		OUT(cost);

		FINISH_ENCODE();
	}

	ENCODE(OP_GroupRoles)
	{
		ENCODE_LENGTH_EXACT(GroupRole_Struct);
		SETUP_DIRECT_ENCODE(GroupRole_Struct, structs::GroupRole_Struct);

		memcpy(eq->Name1, emu->Name1, sizeof(eq->Name1));
		memcpy(eq->Name2, emu->Name2, sizeof(eq->Name2));
		OUT(RoleNumber);
		OUT(Toggle);
		eq->unknown161 = 0;

		FINISH_ENCODE();
	}

	// The client renames its own spawn by the name at +0 and only when byte +0xc1 is set
	ENCODE(OP_MobRename)
	{
		ENCODE_LENGTH_EXACT(MobRename_Struct);
		SETUP_DIRECT_ENCODE(MobRename_Struct, structs::MobRename_Struct);

		strn0cpy(eq->old_name, emu->old_name, sizeof(eq->old_name));
		strn0cpy(eq->old_name_again, emu->old_name_again, sizeof(eq->old_name_again));
		strn0cpy(eq->new_name, emu->new_name, sizeof(eq->new_name));
		eq->unknown192 = 0;
		eq->rename     = 1;
		OUT(unknown196);

		FINISH_ENCODE();
	}

	ENCODE(OP_PetitionUpdate)
	{
		ENCODE_LENGTH_EXACT(PetitionUpdate_Struct);
		SETUP_DIRECT_ENCODE(PetitionUpdate_Struct, structs::PetitionUpdate_Struct);

		OUT(petnumber);
		OUT(color);
		OUT(status);
		eq->senttime = static_cast<uint32>(emu->senttime);
		strn0cpy(eq->accountid, emu->accountid, sizeof(eq->accountid));
		strn0cpy(eq->gmsenttoo, emu->gmsenttoo, sizeof(eq->gmsenttoo));
		OUT(quetotal);
		strn0cpy(eq->charname, emu->charname, sizeof(eq->charname));
		eq->unknown180 = 0;

		FINISH_ENCODE();
	}

	// GM petition queue: the client copies the body into the
	// window record without a size check, so it must be exactly the 2220-byte TOB record
	ENCODE(OP_PetitionCheckout)
	{
		ENCODE_LENGTH_EXACT(Petition_Struct);
		SETUP_DIRECT_ENCODE(Petition_Struct, structs::Petition_Struct);

		OUT(petnumber);
		OUT(urgency);
		strn0cpy(eq->accountid, emu->accountid, sizeof(eq->accountid));
		strn0cpy(eq->lastgm, emu->lastgm, sizeof(eq->lastgm));
		OUT(zone);
		strn0cpy(eq->charname, emu->charname, sizeof(eq->charname));
		OUT(charlevel);
		OUT(charclass);
		OUT(charrace);
		OUT(checkouts);
		OUT(unavail);
		eq->senttime = static_cast<uint32>(emu->senttime);
		strn0cpy(eq->petitiontext, emu->petitiontext, sizeof(eq->petitiontext));
		strn0cpy(eq->gmtext, emu->gmtext, sizeof(eq->gmtext));

		FINISH_ENCODE();
	}

	// the client prints the body as a C string; the server sends strlen(message) bytes, so add the NUL
	ENCODE(OP_MultiLineMsg)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		const size_t len = strnlen(reinterpret_cast<const char *>(in->pBuffer), in->size);
		SerializeBuffer buf;
		TOBWriteBytes(buf, in->pBuffer, len);
		buf.WriteUInt8(0);

		auto outapp = new EQApplicationPacket(OP_MultiLineMsg, std::move(buf));
		dest->FastQueuePacket(&outapp, ack_req);
		safe_delete(in);
	}

	// guild tribute: S2C layouts grew a u64 guild id / 4 bytes of padding
	ENCODE(OP_GuildTributeFavorAndTimer)
	{
		ENCODE_LENGTH_EXACT(GuildTributeFavorTimer_Struct);
		SETUP_DIRECT_ENCODE(GuildTributeFavorTimer_Struct, structs::GuildTributeFavorTimer_Struct);

		eq->guild_id = ServerToTOBGuildID(emu->guild_id);
		OUT(guild_favor);
		OUT(tribute_timer);
		OUT(trophy_timer);

		FINISH_ENCODE();
	}

	ENCODE(OP_GuildSendActiveTributes)
	{
		ENCODE_LENGTH_EXACT(GuildTributeSendActive_Struct);
		SETUP_DIRECT_ENCODE(GuildTributeSendActive_Struct, structs::GuildTributeSendActive_Struct);

		OUT(not_used);
		OUT(guild_favor);
		OUT(tribute_timer);
		OUT(tribute_enabled);
		OUT(tribute_id_1);
		OUT(tribute_id_2);
		OUT(tribute_id_1_tier);
		OUT(tribute_id_2_tier);

		FINISH_ENCODE();
	}

	ENCODE(OP_GuildTributeToggleReply)
	{
		ENCODE_LENGTH_EXACT(GuildTributeSendActive_Struct);
		SETUP_DIRECT_ENCODE(GuildTributeSendActive_Struct, structs::GuildTributeSendActive_Struct);

		OUT(not_used);
		OUT(guild_favor);
		OUT(tribute_timer);
		OUT(tribute_enabled);
		OUT(tribute_id_1);
		OUT(tribute_id_2);
		OUT(tribute_id_1_tier);
		OUT(tribute_id_2_tier);

		FINISH_ENCODE();
	}

	ENCODE(OP_GuildOptInOut)
	{
		ENCODE_LENGTH_EXACT(GuildTributeOptInOutReply_Struct);
		SETUP_DIRECT_ENCODE(GuildTributeOptInOutReply_Struct, structs::GuildTributeOptInOutReply_Struct);

		eq->guild_id = ServerToTOBGuildID(emu->guild_id);
		TOBCopyStr(eq->player_name, emu->player_name);
		OUT(tribute_toggle);
		OUT(tribute_trophy_toggle);
		OUT(no_donations);
		OUT(time);
		OUT(command);

		FINISH_ENCODE();
	}

	// same stream as OP_TributeInfo with a leading guild id the client skips; tier names come from the
	// optional item trailer after the name (zone/tribute.cpp)
	ENCODE(OP_SendGuildTributes)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		if (in->size < sizeof(GuildTributeAbility_Struct)) {
			LogNetcode("TOB::ENCODE(OP_SendGuildTributes) too short [{}]", in->size);
			delete in;
			return;
		}
		auto emu = reinterpret_cast<const GuildTributeAbility_Struct *>(in->pBuffer);
		const auto &ab = emu->ability;

		const uint32 tribute_id = TOBFromNet32(ab.tribute_id);
		if (tribute_id == TRIBUTE_NONE) {
			delete in;
			return;
		}
		uint32 tier_count = std::min<uint32>(TOBFromNet32(ab.tier_count), MAX_TRIBUTE_TIERS);

		auto items = TOBReadTributeItemTrailer(in, sizeof(GuildTributeAbility_Struct));

		SerializeBuffer buf;
		buf.WriteUInt32(emu->guild_id);                 // skipped by the client
		buf.WriteUInt32(tribute_id);
		buf.WriteUInt32(tier_count);
		for (uint32 i = 0; i < tier_count; ++i) {
			const uint32 item_id = TOBFromNet32(ab.tiers[i].tribute_item_id);
			buf.WriteUInt32(item_id);                           // same tier order as OP_TributeInfo
			buf.WriteUInt32(TOBFromNet32(ab.tiers[i].level));
			buf.WriteUInt32(TOBFromNet32(ab.tiers[i].cost));

			const EQ::ItemInstance *inst = (i < items.size()) ? static_cast<const EQ::ItemInstance *>(items[i].inst) : nullptr;
			if (inst && inst->GetItem()) {
				SerializeItem(buf, inst, items[i].slot_id, 0, ItemPacketInvalid);
			} else {
				EQ::ItemData blank;
				memset(&blank, 0, sizeof(blank));
				blank.ID = item_id;
				EQ::ItemInstance tmp(&blank, 0);
				SerializeItem(buf, &tmp, 0, 0, ItemPacketInvalid);
			}
		}
		buf.WriteUInt8(0);

		auto outapp = new EQApplicationPacket(OP_SendGuildTributes, buf.size());
		outapp->WriteData(buf.buffer(), buf.size());
		dest->FastQueuePacket(&outapp, ack_req);
		delete in;
	}

	// byte 36 is the generic '---' theme the points-merchant buy check reads
	ENCODE(OP_AdventurePointsUpdate)
	{
		ENCODE_LENGTH_EXACT(AdventurePoints_Update_Struct);
		SETUP_DIRECT_ENCODE(AdventurePoints_Update_Struct, AdventurePoints_Update_Struct);

		memcpy(eq, emu, sizeof(AdventurePoints_Update_Struct));
		eq->ldon_generic_points = emu->ldon_available_points;

		FINISH_ENCODE();
	}

	// LFG/LFP search results: rows carry a 16-bit guild id in the emu buffer; ToB reads a u64.
	static uint64 TOBLfgGuildId(uint16 emu_guild)
	{
		// zone/worldserver.cpp writes CLE->GuildID() truncated to u16, so GUILD_NONE arrives as 0xFFFF.
		// ToB spawn packets write "no guild" as u32 -1, u32 0; mirror that.
		return emu_guild == EMU_GUILD_NONE_U16 ? 0x00000000FFFFFFFFull : static_cast<uint64>(emu_guild);
	}

	// Emu wire (built by WorldServer::HandleLFGMatches):
	//   u32 0x74af; repeat { cstr comments; cstr name; u16 class; u16 level; u16 zone; u16 guild; u16 anon }
	// ToB wire:
	//   u32 hdr;    repeat { cstr comments; cstr name; u16 class; u16 level; u16 zone; u64 guild; u16 anon }
	ENCODE(OP_LFGGetMatchesResponse)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		const uint32 size = in->size;
		in->SetReadPosition(0);
		const uint32 hdr = in->ReadUInt32();

		if (in->ReadError()) {
			LogNetcode("TOB::ENCODE(OP_LFGGetMatchesResponse) too short [{}]", size);
			delete in;
			return;
		}

		SerializeBuffer buf;
		buf.WriteUInt32(hdr);

		while (in->ReadRemaining() > 0) {
			const uint32 row_start = in->GetReadPosition();
			std::string comments, name;
			// the client copies both strings into 64-byte locals with no bound: clamp to 63
			in->ReadString(comments, TOB_NAME_MAX_CHARS);
			in->ReadString(name, TOB_NAME_MAX_CHARS);
			const uint16 class_ = in->ReadUInt16();
			const uint16 level  = in->ReadUInt16();
			const uint16 zone   = in->ReadUInt16();
			const uint16 guild  = in->ReadUInt16();
			const uint16 anon   = in->ReadUInt16();
			if (in->ReadError()) {
				LogNetcode("TOB::ENCODE(OP_LFGGetMatchesResponse) truncated row at [{}] of [{}]", row_start, size);
				break;
			}
			buf.WriteString(comments);
			buf.WriteString(name);
			buf.WriteUInt16(class_);
			buf.WriteUInt16(level);
			buf.WriteUInt16(zone);
			buf.WriteUInt64(TOBLfgGuildId(guild));
			buf.WriteUInt16(anon);
		}

		auto outapp = new EQApplicationPacket(OP_LFGGetMatchesResponse, std::move(buf));
		delete in;
		dest->FastQueuePacket(&outapp, ack_req);
	}

	// Emu wire (WorldServer::HandleLFPMatches):
	//   u32 0x74af; repeat { cstr comments; u16 from; u16 to; u32 classes; u16 count;
	//                        count x { cstr name; u16 class; u16 level; u16 zone; u16 guild } }
	// ToB wire: identical except member guild is u64.
	ENCODE(OP_LFPGetMatchesResponse)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		const uint32 size = in->size;
		in->SetReadPosition(0);
		const uint32 hdr = in->ReadUInt32();

		if (in->ReadError()) {
			LogNetcode("TOB::ENCODE(OP_LFPGetMatchesResponse) too short [{}]", size);
			delete in;
			return;
		}

		SerializeBuffer buf;
		buf.WriteUInt32(hdr);

		bool ok = true;
		while (ok && in->ReadRemaining() > 0) {
			const uint32 group_start = in->GetReadPosition();
			std::string comments;
			in->ReadString(comments, TOB_NAME_MAX_CHARS);
			const uint16 from    = in->ReadUInt16();
			const uint16 to      = in->ReadUInt16();
			const uint32 classes = in->ReadUInt32();
			uint16 count         = in->ReadUInt16();
			if (in->ReadError()) {
				LogNetcode("TOB::ENCODE(OP_LFPGetMatchesResponse) truncated group at [{}] of [{}]", group_start, size);
				break;
			}
			// the client's local member array holds MAX_GROUP_MEMBERS entries and does not check count
			if (count > MAX_GROUP_MEMBERS) {
				LogNetcode("TOB::ENCODE(OP_LFPGetMatchesResponse) clamping member count [{}]", count);
				count = MAX_GROUP_MEMBERS;
			}

			SerializeBuffer members;
			uint16 written = 0;
			for (uint16 i = 0; i < count; ++i) {
				const uint32 member_start = in->GetReadPosition();
				std::string name;
				in->ReadString(name, TOB_NAME_MAX_CHARS);
				const uint16 class_ = in->ReadUInt16();
				const uint16 level  = in->ReadUInt16();
				const uint16 zone   = in->ReadUInt16();
				const uint16 guild  = in->ReadUInt16();
				if (in->ReadError()) {
					LogNetcode("TOB::ENCODE(OP_LFPGetMatchesResponse) truncated member at [{}] of [{}]", member_start, size);
					ok = false;
					break;
				}
				members.WriteString(name);
				members.WriteUInt16(class_);
				members.WriteUInt16(level);
				members.WriteUInt16(zone);
				members.WriteUInt64(TOBLfgGuildId(guild));
				++written;
			}

			buf.WriteString(comments);
			buf.WriteUInt16(from);
			buf.WriteUInt16(to);
			buf.WriteUInt32(classes);
			buf.WriteUInt16(written);
			TOBWriteBytes(buf, members.buffer(), members.size());
		}

		auto outapp = new EQApplicationPacket(OP_LFPGetMatchesResponse, std::move(buf));
		delete in;
		dest->FastQueuePacket(&outapp, ack_req);
	}

	// ToB: group/raid assist target slots; client index = server Number - 1 (group -1, raid MA1..3 0..2)
	ENCODE(OP_SetGroupTarget)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		if (in->pBuffer == nullptr || in->size < 2 * sizeof(uint32)) {
			delete in;
			return;
		}
		uint32 target_id = 0, number = 0;
		memcpy(&target_id, in->pBuffer, sizeof(uint32));
		memcpy(&number, in->pBuffer + sizeof(uint32), sizeof(uint32));
		if (number > TOB_SET_GROUP_TARGET_MAX_NUMBER) {
			delete in;
			return;
		}

		auto outapp = new EQApplicationPacket(OP_SetGroupTarget, 2 * sizeof(uint32));
		int32 tob_index = static_cast<int32>(number) - 1;
		memcpy(outapp->pBuffer, &target_id, sizeof(uint32));
		memcpy(outapp->pBuffer + sizeof(uint32), &tob_index, sizeof(int32));
		delete in;
		dest->FastQueuePacket(&outapp, ack_req);
	}

	ENCODE(OP_SetTitleReply)
	{
		ENCODE_LENGTH_EXACT(SetTitleReply_Struct);
		SETUP_DIRECT_ENCODE(SetTitleReply_Struct, structs::SetTitleReply_Struct);
		OUT(is_suffix);
		strn0cpy(eq->title, emu->title, std::min(sizeof(eq->title), sizeof(emu->title)));
		OUT(entity_id);
		FINISH_ENCODE();
	}

	ENCODE(OP_SendFindableNPCs)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		if (in->size != sizeof(FindableNPC_Struct)) {
			LogNetcode("TOB::ENCODE(OP_SendFindableNPCs): wrong size [{}], expected [{}]", in->size, sizeof(FindableNPC_Struct));
			delete in;
			return;
		}

		FindableNPC_Struct *emu = (FindableNPC_Struct *)in->pBuffer;

		// ToB: u32 count, then {u32 action, u32 id, name, last_name, u32 race, u32 class}.
		// The client keeps a limited number of bytes of the names; the emu arrays may be unterminated.
		size_t name_len = strnlen(emu->Name, sizeof(emu->Name));
		if (name_len > TOB_FINDABLE_NAME_MAX_CHARS) { name_len = TOB_FINDABLE_NAME_MAX_CHARS; }
		size_t last_len = strnlen(emu->LastName, sizeof(emu->LastName));
		if (last_len > TOB_FINDABLE_LAST_NAME_MAX_CHARS) { last_len = TOB_FINDABLE_LAST_NAME_MAX_CHARS; }

		SerializeBuffer buffer;
		buffer.WriteUInt32(1);
		buffer.WriteUInt32(emu->Action);
		buffer.WriteUInt32(emu->EntityID);
		buffer.WriteString(std::string(emu->Name, name_len));
		buffer.WriteString(std::string(emu->LastName, last_len));
		buffer.WriteUInt32(emu->Race);
		buffer.WriteUInt32(emu->Class);

		auto outapp = new EQApplicationPacket(OP_SendFindableNPCs, std::move(buffer));
		dest->FastQueuePacket(&outapp, ack_req);

		delete in;
	}

	ENCODE(OP_SendMaxCharacters) {
		ENCODE_LENGTH_EXACT(MaxCharacters_Struct);
		SETUP_DIRECT_ENCODE(MaxCharacters_Struct, structs::MaxCharacters_Struct);

		*eq = {0};
		eq->total_character_slots = emu->max_chars; // server limit (CharacterCreationLimit)
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
		eq->entries[24] = TOBKeyRingSlots(0); // MountKeyRingSlots
		eq->entries[25] = TOBKeyRingSlots(1); // IllusionKeyRingSlots
		eq->entries[26] = TOBKeyRingSlots(2); // FamiliarKeyRingSlots
		eq->entries[27] = 0;  // FamiliarAutoLeave
		eq->entries[28] = TOBKeyRingSlots(3); // HeroForgeKeyRingSlots
		eq->entries[29] = 0;  // DragonHoardSlots
		eq->entries[30] = TOBKeyRingSlots(4); // TeleportKeyRingSlots
		eq->entries[31] = 0;  // PersonalDepotSlots
		eq->entries[32] = TOBKeyRingSlots(5); // ActivatedKeyRingSlots

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

		eq->inventory_slot = ServerToTOBTypelessSlot(emu->itemslot, EQ::invtype::typePossessions);
		OUT(quantity);
		OUT(price);

		FINISH_ENCODE();
	}

	ENCODE(OP_ShopRequest)
	{
		ENCODE_LENGTH_EXACT(MerchantClick_Struct);
		SETUP_DIRECT_ENCODE(MerchantClick_Struct, structs::MerchantClickResponse_Struct);

		OUT(npc_id);
		if (emu->command != 0) {
			OUT(player_id);
			OUT(rate);
			OUT(tab_display);
			eq->unknown028 = 256;
			// points merchants (adventure, alt currency, loyalty) use the regular merchant window
			eq->ldon_category = 0;
			eq->alt_currency1 = emu->alt_currency;
			eq->alt_currency2 = emu->alt_currency;
		}
		// close (command==0): player_id stays 0 (SETUP zeroed it) -- byte at offset 4 = 0 -> close path

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
			// msg_stat_change reads the primary value from lock_id (offset +16) for half the
			// TOBAppearance types (MaxHealth, Health, PVP, Sneak, Linkdead, Invisibility visibility).
			// The other half read from parameter (offset +8). Both are set to the same server value
			// so either read path works; each case ignores the field it doesn't use.
			eq->lock_id = sas->parameter;

			if (eq->type != structs::TOBAppearance::None) {
				dest->FastQueuePacket(&outapp, ack_req);
			}
			else {
				delete outapp; // no ToB equivalent (GuildID goes out as OP_GuildMemberGuildID below)
			}

			// ToB takes guild membership changes from OP_GuildMemberGuildID {u32 spawn_id; u64 guild_id}
			// ("You have joined %1"), not from SpawnAppearance.
			if (sas->type == AppearanceType::GuildID) {
				auto gp    = new EQApplicationPacket(OP_GuildMemberGuildID, sizeof(structs::GuildMemberGuildID_Struct));
				auto gid_s = reinterpret_cast<structs::GuildMemberGuildID_Struct *>(gp->pBuffer);
				gid_s->spawn_id = sas->spawn_id;
				gid_s->guild_id = ServerToTOBGuildID(sas->parameter);
				dest->FastQueuePacket(&gp, ack_req);
			}

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

	ENCODE(OP_TaskActivity)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		// Short (25-byte) and reset (activity_type==0) packets are format-compatible
		if (in->size <= 25) {
			dest->FastQueuePacket(&in, ack_req);
			return;
		}

		structs::TaskActivity_Struct task;
		task.task_slot_index = in->ReadUInt32();
		task.task_type       = in->ReadUInt32();
		task.task_id         = in->ReadUInt32();
		task.activity_id     = in->ReadUInt32();
		task.list_group      = in->ReadUInt32();
		task.activity_type   = in->ReadSInt32();
		task.optional_byte   = in->ReadUInt8();
		in->SetReadPosition(in->GetReadPosition() + sizeof(int32)); // skip request_type
		in->ReadString(task.target_name);
		in->ReadLengthString(task.item_list);
		task.goal_count      = in->ReadUInt32();
		in->ReadLengthString(task.skill_list);
		in->ReadLengthString(task.spell_list);
		in->ReadString(task.zones); // null-terminated in RoF+; TOB expects length-prefixed
		task.dz_switch_id    = in->ReadUInt32();
		in->ReadString(task.description_override);
		task.done_count      = in->ReadUInt32();
		task.flag            = in->ReadUInt8();
		in->ReadString(task.zones2);

		SerializeBuffer buf;
		buf.WriteUInt32(task.task_slot_index);
		buf.WriteUInt32(task.task_type);
		buf.WriteUInt32(task.task_id);
		buf.WriteUInt32(task.activity_id);
		buf.WriteUInt32(task.list_group);
		buf.WriteInt32(task.activity_type);
		buf.WriteUInt8(task.optional_byte);
		buf.WriteString(task.target_name);
		buf.WriteLengthString(task.item_list);
		buf.WriteUInt32(task.goal_count);
		buf.WriteLengthString(task.skill_list);
		buf.WriteLengthString(task.spell_list);
		// the length-prefixed string is shown as extra objective text (it put " 54 54" after the task
		// description) and the Zone column reads the null-terminated one (was empty -> "ALL")
		buf.WriteLengthString("");
		buf.WriteString(task.zones);
		buf.WriteUInt32(task.dz_switch_id);
		buf.WriteString(task.description_override);
		buf.WriteUInt32(task.done_count);
		buf.WriteUInt8(task.flag);
		buf.WriteString(task.zones2);

		uchar *emu_buffer = in->pBuffer;
		in->size = buf.size();
		in->pBuffer = new uint8[buf.size()];
		memcpy(in->pBuffer, buf.buffer(), buf.size());
		delete[] emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_TaskDescription)
	{
		EQApplicationPacket *in = *p;
		*p = nullptr;

		structs::TaskDescription_Struct task;
		task.sequence_number      = in->ReadUInt32();
		task.task_id              = in->ReadUInt32();
		task.open_window          = in->ReadUInt8();
		task.task_type            = in->ReadUInt32();
		task.reward_type          = in->ReadUInt32();
		in->ReadString(task.title);
		task.duration             = in->ReadUInt32();
		in->SetReadPosition(in->GetReadPosition() + sizeof(uint32)); // skip dur_code (not in TOB wire format)
		task.start_time           = in->ReadUInt32();
		in->ReadString(task.description);
		task.has_rewards          = in->ReadUInt8();
		task.coin_reward          = in->ReadUInt32();
		task.xp_reward            = in->ReadUInt32();
		task.faction_reward       = in->ReadUInt32();
		in->ReadString(task.reward_text);
		in->ReadString(task.item_link);
		task.points               = in->ReadUInt32();
		task.has_reward_selection = in->ReadUInt8();

		std::string new_item_link;
		ServerToTOBConvertLinks(new_item_link, task.item_link);

		uint32 out_size = sizeof(uint32) + sizeof(uint32) + sizeof(uint8) + sizeof(uint32) + sizeof(uint32) // header
		                + sizeof(uint32) + task.title.size()        // length-prefixed title
		                + sizeof(uint32) + sizeof(uint32)           // duration, start_time
		                + sizeof(uint32) + task.description.size()  // length-prefixed description
		                + sizeof(uint8)                             // has_rewards
		                + sizeof(uint8)                             // has_reward_selection
		                + 4 * sizeof(uint32)                        // player_levels
		                + sizeof(uint32) + task.reward_text.size(); // length-prefixed reward_text

		if (task.has_rewards) {
			out_size += sizeof(uint32)                      // coin_reward
			          + sizeof(uint8)                       // xp_reward as flag
			          + sizeof(uint32)                      // faction_reward
			          + sizeof(uint32)                      // new unknown field
			          + sizeof(uint32) + new_item_link.size() // length-prefixed item_link
			          + sizeof(uint32);                     // points
		}

		SerializeBuffer buf(out_size);
		buf.WriteUInt32(task.sequence_number);
		buf.WriteUInt32(task.task_id);
		buf.WriteUInt8(task.open_window);
		buf.WriteUInt32(task.task_type);
		buf.WriteUInt32(task.reward_type);
		buf.WriteLengthString(task.title);
		buf.WriteUInt32(task.duration);
		buf.WriteUInt32(task.start_time);
		buf.WriteLengthString(task.description);
		buf.WriteUInt8(task.has_rewards);
		if (task.has_rewards) {
			buf.WriteUInt32(task.coin_reward);
			buf.WriteUInt8(task.xp_reward > 0 ? 1 : 0);
			buf.WriteUInt32(task.faction_reward);
			buf.WriteUInt32(0); // new field, no emu equivalent
			buf.WriteLengthString(new_item_link);
			buf.WriteUInt32(task.points);
		}
		buf.WriteUInt8(task.has_reward_selection);
		buf.WriteUInt32(0); // player_level1
		buf.WriteUInt32(0); // player_level2
		buf.WriteUInt32(0); // player_level3
		buf.WriteUInt32(0); // player_level4
		buf.WriteLengthString(task.reward_text);

		uchar *emu_buffer = in->pBuffer;
		in->size = buf.size();
		in->pBuffer = new uint8[buf.size()];
		memcpy(in->pBuffer, buf.buffer(), buf.size());
		delete[] emu_buffer;
		dest->FastQueuePacket(&in, ack_req);
	}

	ENCODE(OP_TaskSelectWindow)
	{
		EQApplicationPacket* in = *p;
		*p = nullptr;

		// need to calculate the size of the buffer, which can only be done after reading all the strings stored
		uint32 task_count = in->ReadUInt32();
		uint32 type = in->ReadUInt32();
		uint32 task_giver_id = in->ReadUInt32();

		uint32 out_size = 3 * sizeof(uint32); // initial size is the header here

		std::vector<structs::TaskSelectWindow_Struct> tasks;
		tasks.reserve(task_count);
		for (int i = 0; i < task_count ; i++) {
			structs::TaskSelectWindow_Struct task;
			task.task_id = in->ReadUInt32();
			out_size += sizeof(task.task_id);

			task.reward_multiplier = in->ReadFloat();
			out_size += sizeof(task.reward_multiplier);

			task.duration = in->ReadUInt32();
			out_size += sizeof(task.duration);

			in->SetReadPosition(in->GetReadPosition() + sizeof(uint32)); // skip duration_code

			in->ReadString(task.title);
			out_size += task.title.size() + 1;

			in->ReadString(task.description);
			out_size += task.description.size() + 1;

			task.preview_enable = static_cast<bool>(in->ReadUInt8());
			out_size += sizeof(task.preview_enable);

			uint32 selector_count = in->ReadUInt32();
			out_size += sizeof(selector_count);

			std::vector<structs::TaskSelectWindowSelector_Struct> selectors;
			selectors.reserve(selector_count);
			for (int j = 0; j < selector_count; j++) {
				structs::TaskSelectWindowSelector_Struct selector;
				selector.selector_id = in->ReadSInt32();
				out_size += sizeof(selector.selector_id);

				selector.activity_type = in->ReadSInt32();
				out_size += sizeof(selector.activity_type);

				in->SetReadPosition(in->GetReadPosition() + sizeof(int32)); // skip request_type

				in->ReadString(selector.target_name);
				out_size += selector.target_name.size() + 1;

				in->ReadLengthString(selector.item_list);
				out_size += sizeof(uint32) + selector.item_list.size();

				selector.goal_count = in->ReadSInt32();
				out_size += sizeof(selector.goal_count);

				in->ReadLengthString(selector.skill_list);
				out_size += sizeof(uint32) + selector.skill_list.size();

				in->ReadLengthString(selector.spell_list);
				out_size += sizeof(uint32) + selector.spell_list.size();

				in->ReadString(selector.zones);
				out_size += selector.zones.size() + 1;

				in->ReadString(selector.description_override);
				out_size += selector.description_override.size() + 1;

				in->ReadString(selector.zones_internal);
				out_size += selector.zones_internal.size() + 1;

				selectors.push_back(std::move(selector));
			}

			task.selectors = std::move(selectors);
			tasks.push_back(std::move(task));
		}

		SerializeBuffer buf(out_size);
		buf.WriteUInt32(task_count);
		buf.WriteUInt32(type);
		buf.WriteUInt32(task_giver_id);

		for (const auto& task : tasks) {
			buf.WriteUInt32(task.task_id);
			buf.WriteFloat(task.reward_multiplier);
			buf.WriteUInt32(task.duration);
			buf.WriteString(task.title);
			buf.WriteString(task.description);
			buf.WriteUInt8(task.preview_enable);
			buf.WriteUInt32(task.selectors.size());
			for (const auto& selector : task.selectors) {
				buf.WriteUInt32(selector.selector_id);
				buf.WriteUInt32(selector.activity_type);
				buf.WriteString(selector.target_name);
				buf.WriteLengthString(selector.item_list);
				buf.WriteUInt32(selector.goal_count);
				buf.WriteLengthString(selector.skill_list);
				buf.WriteLengthString(selector.spell_list);
				buf.WriteString(selector.zones);
				buf.WriteString(selector.description_override);
				buf.WriteString(selector.zones_internal);
			}
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

	ENCODE(OP_SharedTaskSelectWindow)
	{
		ENCODE_FORWARD(OP_TaskSelectWindow);
	}

	ENCODE(OP_Trader)
	{
		if ((*p)->size < sizeof(uint32)) {
			delete *p;
			*p = nullptr;
			return;
		}
		uint32 action = *(uint32 *) (*p)->pBuffer;

		switch (action) {
			case TraderOn: {
				ENCODE_LENGTH_EXACT(Trader_ShowItems_Struct);
				SETUP_DIRECT_ENCODE(Trader_ShowItems_Struct, structs::Trader_ShowItems_Struct);

				eq->action = structs::TOBBazaarTraderBuyerActions::BeginTraderMode;
				OUT(entity_id);

				LogTrading("(TOB) TraderOn action <green>[{}] entity_id <green>[{}]", action, eq->entity_id);
				FINISH_ENCODE();
				break;
			}
			case TraderOff: {
				ENCODE_LENGTH_EXACT(Trader_ShowItems_Struct);
				SETUP_DIRECT_ENCODE(Trader_ShowItems_Struct, structs::Trader_ShowItems_Struct);

				eq->action = structs::TOBBazaarTraderBuyerActions::EndTraderMode;
				OUT(entity_id);

				LogTrading("(TOB) TraderOff action <green>[{}] entity_id <green>[{}]", action, eq->entity_id);
				FINISH_ENCODE();
				break;
			}
			case ListTraderItems: {
				ENCODE_LENGTH_EXACT(Trader_Struct);
				SETUP_DIRECT_ENCODE(Trader_Struct, structs::ClickTrader_Struct);
				LogTrading("(TOB)  action <green>[{}]", action);

				eq->action = structs::TOBBazaarTraderBuyerActions::ListTraderItems;
				std::transform(
					std::begin(emu->items),
					std::end(emu->items),
					std::begin(eq->items),
					[&](uint64 x) {
						return static_cast<uint32>(x);
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
				LogTrading("(TOB) TraderAck2 action");
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
							"(TOB) PriceUpdate action <green>[{}] AddItem subaction <yellow>[{}]",
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
							"(TOB) PriceUpdate action <green>[{}] RemoveItem subaction <yellow>[{}]",
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
							"(TOB) PriceUpdate action <green>[{}] UpdatePrice subaction <yellow>[{}]",
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
					"(TOB) BuyTraderItem action <green>[{}] item_id <green>[{}] item_sn <green>[{}] buyer <green>[{}]",
					action,
					eq->item_id,
					eq->serial_number,
					eq->buyer_name
				);
				dest->FastQueuePacket(&in);
				break;
			}
			default: {
				LogTrading("(TOB) action <red>[{}]", action);
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
			"(TOB) item_id <green>[{}] price <green>[{}] quantity <green>[{}] trader_id <green>[{}]",
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
			"(TOB) trader_id <green>[{}] item_id <green>[{}]",
			emu->trader_id,
			emu->item_id
		);

		eq->TraderID = emu->trader_id;
		auto serial  = fmt::format("{:016}\n", emu->item_id);
		strn0cpy(eq->SerialNumber, serial.c_str(), sizeof(eq->SerialNumber));
		LogTrading("(TOB) TraderID <green>[{}], SerialNumber: <green>[{}]", emu->trader_id, emu->item_id);

		FINISH_ENCODE();
	}

	ENCODE(OP_TraderShop)
	{
		if ((*p)->size < sizeof(uint32)) {
			delete *p;
			*p = nullptr;
			return;
		}
		auto action = *(uint32 *) (*p)->pBuffer;

		switch (action) {
			case ClickTrader: {
				ENCODE_LENGTH_EXACT(TraderClick_Struct);
				SETUP_DIRECT_ENCODE(TraderClick_Struct, structs::TraderClick_Struct);
				LogTrading(
					"(TOB) ClickTrader action <green>[{}] trader_id <green>[{}]",
					action,
					emu->TraderID
				);

				eq->action      = structs::TOBBazaarTraderBuyerActions::ClickTrader; // Seen on Live
				eq->trader_id   = emu->TraderID;
				eq->unknown_008 = emu->Approval;

				FINISH_ENCODE();
				break;
			}
			case structs::TOBBazaarTraderBuyerActions::BuyTraderItem: {
				ENCODE_LENGTH_EXACT(structs::TraderBuy_Struct);
				SETUP_DIRECT_ENCODE(TraderBuy_Struct, structs::TraderBuy_Struct);
				LogTrading(
					"(TOB) item_id <green>[{}] price <green>[{}] quantity <green>[{}] trader_id <green>[{}]",
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
				LogTrading("(TOB) Unhandled action <red>[{}]", action);
				EQApplicationPacket *in = *p;
				*p = nullptr;

				dest->FastQueuePacket(&in);
			}
		}
	}

	ENCODE(OP_Track)
	{
		SETUP_VAR_ENCODE(Track_Struct);
		int EntryCount = __packet->size / sizeof(Track_Struct);

		if (EntryCount == 0 || (__packet->size % sizeof(Track_Struct)) != 0) {
			LogNetcode("[STRUCTS] Wrong size on outbound [{}]: Got [{}], expected multiple of [{}]", opcodes->EmuToName(__packet->GetOpcode()), __packet->size, sizeof(Track_Struct));
			delete __packet;
			return;
		}

		int PacketSize = 2;
		for (int i = 0; i < EntryCount; ++i)
			PacketSize += 13 + strlen(emu[i].name);

		ALLOC_LEN_ENCODE(PacketSize);

		char *Buffer = (char *)__packet->pBuffer;
		VARSTRUCT_ENCODE_TYPE(uint16, Buffer, EntryCount);
		for (int i = 0; i < EntryCount; ++i) {
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, emu[i].entityid);
			VARSTRUCT_ENCODE_TYPE(float,  Buffer, emu[i].distance);
			VARSTRUCT_ENCODE_TYPE(uint8,  Buffer, emu[i].level);
			VARSTRUCT_ENCODE_TYPE(uint8,  Buffer, emu[i].is_npc);
			VARSTRUCT_ENCODE_STRING(Buffer, emu[i].name);
			VARSTRUCT_ENCODE_TYPE(uint8,  Buffer, emu[i].is_pet);
			VARSTRUCT_ENCODE_TYPE(uint8,  Buffer, emu[i].is_merc);
		}

		FINISH_ENCODE();
	}

	ENCODE(OP_Weather)
	{
		SETUP_DIRECT_ENCODE(Weather_Struct, structs::Weather_Struct);

		OUT(val1);
		OUT(type);
		eq->unknown = 0;
		OUT(mode);

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

	ENCODE(OP_WhoAllResponse)
	{
		SETUP_VAR_ENCODE(WhoAllReturnStruct);

		int Count = emu->playercount;

		// TOB client expects playercount in unknown44[0] and unknown52 = 0
		emu->unknown44[0] = Count;
		emu->unknown52 = 0;

		SerializeBuffer buffer;
		TOBWriteBytes(buffer, __emu_buffer, sizeof(WhoAllReturnStruct));

		char *InBuffer = (char *)__emu_buffer + sizeof(WhoAllReturnStruct);

		for (int i = 0; i < Count; ++i) {
			uint32 x;

			x = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);
			buffer.WriteUInt32(x);                         // FormatMSGID

			InBuffer += 4;                                 // skip server PIDMSGID
			buffer.WriteUInt32(0);                         // PIDMSGID = 0 (no surname)
			buffer.WriteUInt32(0xffffffff);                // extra uint32 before Name

			char Name[64];

			VARSTRUCT_DECODE_STRING(Name, InBuffer);       // Char Name
			buffer.WriteString(Name);

			x = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);
			buffer.WriteUInt32(x);                         // RankMSGID

			VARSTRUCT_DECODE_STRING(Name, InBuffer);       // Guild Name
			buffer.WriteString(Name);

			for (int j = 0; j < 7; ++j) {
				x = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);
				buffer.WriteUInt32(x);
			}

			VARSTRUCT_DECODE_STRING(Name, InBuffer);       // Account
			buffer.WriteString(Name);

			x = VARSTRUCT_DECODE_TYPE(uint32, InBuffer);
			buffer.WriteUInt32(x);                         // Unknown100
		}

		auto outapp = new EQApplicationPacket(OP_WhoAllResponse, std::move(buffer));
		dest->FastQueuePacket(&outapp, ack_req);
		delete __packet;
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
		// 20 fixed bytes + zone name with terminator + 4 trailing uint32s actually written below
		SerializeBuffer buffer;
		buffer.WriteUInt16(emu->bind_zone_id);
		buffer.WriteUInt16(emu->bind_instance_id);
		buffer.WriteFloat(emu->x);
		buffer.WriteFloat(emu->y);
		buffer.WriteFloat(emu->z);
		buffer.WriteFloat(emu->heading);
		buffer.WriteString(emu->zone_name);
		buffer.WriteUInt32(60);
		buffer.WriteUInt32(0);
		buffer.WriteUInt32(51);
		buffer.WriteUInt32(41);

		auto outapp = new EQApplicationPacket(OP_ZonePlayerToBind, std::move(buffer));
		dest->FastQueuePacket(&outapp, ack_req);
		delete __packet;
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
			// ToB guild ids are u64 and 0 means "no guild"; the server's 0xFFFFFFFF made the client
			// think everyone (NPCs too) was in guild 4294967295
			buffer.WriteUInt64(emu->NPC ? 0 : ServerToTOBGuildID(emu->guildID));
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
			buffer.WriteUInt32(emu->npc_tint_id); // NpcTintIndex (npc_types.armortint_id)
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

		emu->inventorySlot = TOBToServerSlot(eq->inventorySlot);
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

		// TODO: expand approval logic — TOB wire format (84 bytes total):
		//   +0x00  char[64]  name
		//   +0x40  uint32    race_id
		//   +0x44  uint32    class_id
		//   +0x48  uint32    deity_id      (not in server NameApproval_Struct)
		//   +0x4c  uint32    heroic_type   (0–4; not in server struct)
		//   +0x50  uint32    unknown       (always 0)

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
		// The ToB client sends up to 60 blocked spell ids, but the server struct only holds
		// BLOCKED_BUFF_COUNT (20). Writing them all overran the heap on every zone-in.
		if (__packet->size < 4) {
			__packet->SetOpcode(OP_Unknown);
			return;
		}
		uint32 count = __packet->ReadUInt32();
		if (__packet->size < 6 + 4ull * count) {
			LogNetcode("Wrong size on incoming OP_BlockedBuffs: [{}] bytes for [{}] ids", __packet->size, count);
			__packet->SetOpcode(OP_Unknown);
			return;
		}
		std::vector<int32> blocked_spell_ids;
		blocked_spell_ids.reserve(count);
		for (uint32 i = 0; i < count; ++i)
			blocked_spell_ids.push_back(static_cast<int32>(__packet->ReadUInt32()));

		bool pet = __packet->ReadUInt8() == 1;
		bool init = __packet->ReadUInt8() == 1;

		__packet->SetReadPosition(0); // reset the packet read to pass it along

		unsigned char* __eq_buffer = __packet->pBuffer;
		__packet->size = sizeof(BlockedBuffs_Struct);
		__packet->pBuffer = new unsigned char[__packet->size]{};
		BlockedBuffs_Struct* emu = (BlockedBuffs_Struct*)__packet->pBuffer;

		const uint32 kept = std::min<uint32>(count, BLOCKED_BUFF_COUNT);
		memset(emu->SpellID, -1, sizeof(emu->SpellID));
		for (uint32 i = 0; i < kept; ++i)
			emu->SpellID[i] = blocked_spell_ids[i];

		emu->Count = kept;
		emu->Pet = pet;
		emu->Initialise = init;
		delete[] __eq_buffer;
	}

	DECODE(OP_BookButton)
	{
		DECODE_LENGTH_EXACT(structs::BookButton_Struct);
		SETUP_DIRECT_DECODE(BookButton_Struct, structs::BookButton_Struct);

		emu->invslot = static_cast<int16_t>(TOBToServerTypelessSlot(eq->slot, invtype::typePossessions));
		IN(target_id);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_BuffDefinition)
	{
		DECODE_LENGTH_EXACT(structs::EQAffectPacket_Struct);
		SETUP_DIRECT_DECODE(SpellBuffPacket_Struct, structs::EQAffectPacket_Struct);

		emu->entityid           = eq->entity_id;
		emu->buff.level         = eq->affect.level;
		emu->buff.bard_modifier = 0;
		emu->buff.spellid       = eq->affect.spell_id;
		emu->buff.duration      = eq->affect.duration;
		emu->buff.counters      = eq->affect.hit_count;
		emu->buff.player_id     = eq->affect.caster_id.Id;
		emu->buff.num_hits      = eq->affect.hit_count;
		emu->buff.y             = eq->affect.y;
		emu->buff.x             = eq->affect.x;
		emu->buff.z             = eq->affect.z;
		emu->slotid             = TOBToServerBuffSlot(static_cast<int>(eq->slot_id));
		emu->bufffade           = eq->buff_fade;

		FINISH_DIRECT_DECODE();
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

	DECODE(OP_Barter)
	{
		if (__packet->size < sizeof(uint32)) {
			__packet->SetOpcode(OP_Unknown);
			return;
		}
		auto action = *(uint32 *) __packet->pBuffer;

		switch (action) {
			case structs::TOBBuyerActions::BuyerRemoveItem: {
				auto emu = (BuyerGeneric_Struct *) __packet->pBuffer;
				emu->action = Barter_BuyerItemRemove;
				LogTradingDetail("(TOB) Buyer Remove Item");

				break;
			}
			case structs::TOBBuyerActions::BuyerInspectBegin: {
				LogTradingDetail("(TOB) Buyer Inspect Begin Item");

				auto emu = (BuyerGeneric_Struct *) __packet->pBuffer;
				emu->action = Barter_BuyerInspectBegin;

				break;
			}
			case structs::TOBBuyerActions::BuyerInspectEnd: {
				LogTradingDetail("(TOB) Buyer Inspect End Item ");

				auto emu = (BuyerGeneric_Struct *) __packet->pBuffer;
				emu->action = Barter_BuyerInspectEnd;

				break;
			}
			case structs::TOBBuyerActions::BuyerWelcomeMessage: {
				LogTradingDetail("(TOB) Buyer Welcome Message Update");
				SETUP_DIRECT_DECODE(BuyerWelcomeMessageUpdate_Struct, structs::BuyerWelcomeMessageUpdate_Struct);

				emu->action = Barter_WelcomeMessageUpdate;
				strn0cpy(emu->welcome_message, eq->welcome_message, sizeof(emu->welcome_message));

				FINISH_DIRECT_DECODE();
				break;
			}
			case structs::TOBBuyerActions::BuyerItemInspect: {
				SETUP_DIRECT_DECODE(BarterItemSearchLinkRequest_Struct, structs::BarterItemSearchLinkRequest_Struct);
				LogTradingDetail("(TOB) Seller ID <green>[{}] Inspecting Item <green>[{}] from Buyer ID <green>[{}] ",
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
				LogTradingDetail("(TOB) Pass thru OP_Barter packet action <red>[{}]", emu->action);
			}
		}
	}

	DECODE(OP_BazaarSearch)
	{
		if (__packet->size < sizeof(uint8)) {
			__packet->SetOpcode(OP_Unknown);
			return;
		}
		char *Buffer = (char *)__packet->pBuffer;

		uint8 SubAction = VARSTRUCT_DECODE_TYPE(uint8, Buffer);
		LogTrading("(TOB) action <green>[{}]", SubAction);

		if ((SubAction != BazaarInspectItem) || (__packet->size != sizeof(structs::NewBazaarInspect_Struct)))
			return;

		SETUP_DIRECT_DECODE(NewBazaarInspect_Struct, structs::NewBazaarInspect_Struct);

		IN(Beginning.Action);
		memcpy(emu->Name, eq->Name, sizeof(emu->Name));
		IN(SerialNumber);
		LogTrading("(TOB) action <green>[{}] serial_number <green>[{}]", eq->Beginning.Action, eq->SerialNumber);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_BuyerItems)
	{
		if (__packet->size < sizeof(uint32)) {
			__packet->SetOpcode(OP_Unknown);
			return;
		}
		auto action = *(uint32 *) __packet->pBuffer;

		switch (action) {
			case structs::TOBBuyerActions::BuyerModifyBuyLine:
			case structs::TOBBuyerActions::BuyerBuyLine: {
				// client-controlled counts and strings: bounds-checked packet reads
				BuyerBuyLines_Struct buyer_buy_lines{};
				__packet->SetReadPosition(0);

				buyer_buy_lines.action = __packet->ReadUInt32();

				buyer_buy_lines.no_items = 1;
				if (action == structs::TOBBuyerActions::BuyerBuyLine) {
					buyer_buy_lines.no_items = std::min<uint16>(__packet->ReadUInt16(), MAX_BUYER_LINES);
				}

				buyer_buy_lines.buy_lines.reserve(buyer_buy_lines.no_items);
				for (int i = 0; i < buyer_buy_lines.no_items && !__packet->ReadError(); i++) {
					BuyerLineItems_Struct b{};
					b.slot           = __packet->ReadUInt32();
					b.enabled        = __packet->ReadUInt8();
					b.item_id        = __packet->ReadUInt32();
					__packet->ReadString(b.item_name, 64);
					b.item_icon      = __packet->ReadUInt32();
					b.item_quantity  = __packet->ReadUInt32();
					b.item_toggle    = __packet->ReadUInt8();
					b.item_cost      = __packet->ReadUInt32();
					auto trade_items = std::min<uint32>(__packet->ReadUInt32(), MAX_BUYER_COMPENSATION_ITEMS);
					buyer_buy_lines.buy_lines.push_back(b);

					if (trade_items > 0) {
						buyer_buy_lines.buy_lines[i].trade_items.reserve(trade_items);
						for (uint32 x = 0; x < trade_items && !__packet->ReadError(); x++) {
							BuyerLineTradeItems_Struct blti{};
							blti.item_id       = __packet->ReadUInt32();
							blti.item_quantity = __packet->ReadUInt32();
							blti.item_icon     = __packet->ReadUInt32();
							__packet->ReadString(blti.item_name, 64);
							buyer_buy_lines.buy_lines[i].trade_items.push_back(blti);
						}
					}
					__packet->ReadSkip(std::min<uint32>(TOB_BUYER_LINE_TRAILER_SIZE, __packet->ReadRemaining())); // trailing padding
				}

				if (__packet->ReadError()) {
					LogNetcode("TOB::DECODE(OP_BuyerItems) malformed buy line packet, size [{}]", __packet->size);
					__packet->SetOpcode(OP_Unknown);
					return;
				}

				std::stringstream           ss{};
				cereal::BinaryOutputArchive ar(ss);
				{
					ar(buyer_buy_lines);
				}

				auto new_size   = sizeof(BuyerGeneric_Struct) + ss.str().length();
				auto new_packet = new unsigned char[new_size];
				delete[] __packet->pBuffer;
				__packet->size    = new_size;
				__packet->pBuffer = new_packet;
				auto emu = (BuyerGeneric_Struct *) __packet->pBuffer;
				emu->action = Barter_BuyerItemUpdate;

				if (action == structs::TOBBuyerActions::BuyerBuyLine) {
					emu->action = Barter_BuyerItemStart;
				}

				memcpy(emu->payload, ss.str().data(), ss.str().length());
				__packet->SetOpcode(OP_Barter);

				break;
			}
			case structs::TOBBuyerActions::BuyerSellItem: {
				BuyerLineSellItem_Struct sell_item{};
				__packet->SetReadPosition(0);

				sell_item.action          = __packet->ReadUInt32();
				sell_item.purchase_method = __packet->ReadUInt32();
				__packet->ReadSkip(4);
				sell_item.buyer_entity_id = __packet->ReadUInt32();
				sell_item.buyer_id        = __packet->ReadUInt32();
				__packet->ReadSkip(11);
				sell_item.slot    = __packet->ReadUInt32();
				sell_item.enabled = __packet->ReadUInt8();
				sell_item.item_id = __packet->ReadUInt32();
				std::string item_name;
				__packet->ReadString(item_name, sizeof(sell_item.item_name) - 1);
				strn0cpy(sell_item.item_name, item_name.c_str(), sizeof(sell_item.item_name));
				sell_item.item_icon      = __packet->ReadUInt32();
				sell_item.item_quantity  = __packet->ReadUInt32();
				sell_item.item_toggle    = __packet->ReadUInt8();
				sell_item.item_cost      = __packet->ReadUInt32();
				sell_item.no_trade_items = std::min<uint32>(__packet->ReadUInt32(), MAX_BUYER_COMPENSATION_ITEMS);

				if (sell_item.no_trade_items > 0) {
					sell_item.trade_items.reserve(sell_item.no_trade_items);
					for (uint32 x = 0; x < sell_item.no_trade_items && !__packet->ReadError(); x++) {
						BuyerLineTradeItems_Struct blti{};
						blti.item_id       = __packet->ReadUInt32();
						blti.item_quantity = __packet->ReadUInt32();
						blti.item_icon     = __packet->ReadUInt32();
						__packet->ReadString(blti.item_name, 64);
						sell_item.trade_items.push_back(blti);
					}
				}

				if (sell_item.purchase_method) {
					sell_item.buyer_entity_id = __packet->ReadUInt32();
					sell_item.buyer_id        = __packet->ReadUInt32();
					sell_item.zone_id         = __packet->ReadUInt32();
					__packet->ReadString(sell_item.buyer_name, 64);
				}
				else {
					__packet->ReadSkip(TOB_BUYER_LINE_TRAILER_SIZE);
				}

				sell_item.seller_quantity = __packet->ReadUInt32();

				if (__packet->ReadError()) {
					LogNetcode("TOB::DECODE(OP_BuyerItems) malformed sell item packet, size [{}]", __packet->size);
					__packet->SetOpcode(OP_Unknown);
					return;
				}

				std::stringstream           ss{};
				cereal::BinaryOutputArchive ar(ss);
				{
					ar(sell_item);
				}

				auto new_size   = sizeof(BuyerGeneric_Struct) + ss.str().length();
				auto new_packet = new unsigned char[new_size];
				delete[] __packet->pBuffer;
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

		emu->slot = static_cast<uint32>(TOBToServerCastingSlot(static_cast<spells::CastingSlot>(eq->slot)));

		IN(spell_id);
		emu->inventoryslot = TOBToServerSlot(TOBCastingInventorySlotToInventorySlot(eq->inventory_slot));
		IN(target_id);
		IN(y_pos);
		IN(x_pos);
		IN(z_pos);
		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ChannelMessage)
	{
		// ToB: cstr sender, cstr target, 8 unknown bytes, u32 language, u32 channel, 5 unknown bytes, u32 skill,
		// cstr message (client limit about 11k; may be unterminated at the end of the packet), then 15 trailing bytes
		// (see ENCODE).
		constexpr uint32 ChannelMessageGapBeforeLanguage = 8;
		constexpr uint32 ChannelMessageGapBeforeSkill    = 5;

		unsigned char *__eq_buffer = __packet->pBuffer;
		const uint32  in_size      = __packet->size;
		__packet->SetReadPosition(0);

		std::string sender, target;
		__packet->ReadString(sender, sizeof(ChannelMessage_Struct::sender) - 1);
		__packet->ReadString(target, sizeof(ChannelMessage_Struct::targetname) - 1);
		__packet->ReadSkip(ChannelMessageGapBeforeLanguage);
		const uint32 language = __packet->ReadUInt32();
		const uint32 channel  = __packet->ReadUInt32();
		__packet->ReadSkip(ChannelMessageGapBeforeSkill);
		const uint32 skill = __packet->ReadUInt32();
		if (__packet->ReadError()) {
			LogNetcode("TOB::DECODE(OP_ChannelMessage) malformed packet size [{}]", in_size);
			__packet->SetOpcode(OP_Unknown);
			return;
		}

		const char *message_start = reinterpret_cast<const char *>(__eq_buffer + __packet->GetReadPosition());
		std::string old_message(message_start, strnlen(message_start, __packet->ReadRemaining()));
		std::string new_message;
		TOBToServerConvertLinks(new_message, old_message);

		__packet->size    = sizeof(ChannelMessage_Struct) + new_message.length() + 1;
		__packet->pBuffer = new unsigned char[__packet->size];
		ChannelMessage_Struct *emu = (ChannelMessage_Struct *) __packet->pBuffer;

		strn0cpy(emu->targetname, target.c_str(), sizeof(emu->targetname));
		strn0cpy(emu->sender, sender.c_str(), sizeof(emu->sender));
		emu->language          = language;
		emu->chan_num          = channel;
		emu->skill_in_language = skill;
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

		// TODO: expand heroic character handling — TOB wire format (168 bytes total):
		//   +0x00  uint8[72]  padding (zeroed)
		//   +0x48  uint32     gender
		//   +0x4c  uint32     race
		//   +0x50  uint32     class_
		//   +0x54  uint32     deity
		//   +0x58  uint32     start_zone
		//   +0x5c  uint32     haircolor
		//   +0x60  uint32     beard
		//   +0x64  uint32     beardcolor
		//   +0x68  uint32     hairstyle
		//   +0x6c  uint32     face
		//   +0x70  uint32     eyecolor1
		//   +0x74  uint32     eyecolor2
		//   +0x78  uint32     drakkin_heritage
		//   +0x7c  uint32     drakkin_tattoo
		//   +0x80  uint32     drakkin_details
		//   +0x84  uint32     STR
		//   +0x88  uint32     STA
		//   +0x8c  uint32     AGI
		//   +0x90  uint32     DEX
		//   +0x94  uint32     WIS
		//   +0x98  uint32     INT
		//   +0x9c  uint32     CHA
		//   +0xa0  uint32     tutorial
		//   +0xa4  uint32     heroic_type  (0=none, 1=lvl 85, 2=lvl 50, 3=lvl 100, 4=lvl 115; not in server CharCreate_Struct)

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ClickDoor)
	{
		DECODE_LENGTH_EXACT(structs::ClickDoor_Struct);
		SETUP_DIRECT_DECODE(ClickDoor_Struct, structs::ClickDoor_Struct);

		IN(doorid);
		IN(player_id);
		IN(item_id);
		emu->picklockskill = static_cast<uint8>(eq->picklockskill);

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

	DECODE(OP_Consume)
	{
		DECODE_LENGTH_EXACT(structs::Consume_Struct);
		SETUP_DIRECT_DECODE(Consume_Struct, structs::Consume_Struct);

		emu->slot          = TOBToServerSlot(eq->slot);
		emu->auto_consumed = (eq->mode == 1) ? 0x000003E7u : 0xFFFFFFFFu;
		emu->type          = static_cast<uint8>(eq->type + 1); // TOB 0/1 → server 1/2

		FINISH_DIRECT_DECODE();
	}

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

	DECODE(OP_Damage)
	{
		DECODE_LENGTH_EXACT(structs::CombatDamage_Struct);
		SETUP_DIRECT_DECODE(CombatDamage_Struct, structs::CombatDamage_Struct);

		IN(target);
		IN(source);
		IN(type);
		IN(spellid);
		IN(damage);
		IN(force);
		IN(hit_heading);
		IN(hit_pitch);
		IN(special);

		FINISH_DIRECT_DECODE();
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

		// TODO: map TOB wire fields to server flags.
		// TOB wire format (72 bytes total):
		//   +0x00  char[64]  name
		//   +0x40  int32     unknown1  — 0 normally; 0x00010000 when re-entering after ZoneNotReady
		//   +0x44  int32     zoneID    — EverQuest_EnterZoneReason: -1 = enter last zone (normal);
		//                               other values = specific zone ID (tutorial zone? home city?)
		// Server struct uses separate `tutorial` and `return_home` uint32 flags.
		// Correct mapping requires knowing which zoneID values correspond to tutorial vs return_home.
		emu->return_home = 0;
		emu->tutorial = 0;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_EnvDamage)
	{
		DECODE_LENGTH_EXACT(structs::EnvDamage2_Struct);
		SETUP_DIRECT_DECODE(EnvDamage2_Struct, structs::EnvDamage2_Struct);

		emu->id = eq->entity_id;
		emu->damage = static_cast<uint32>(eq->damage);
		IN(dmgtype);
		emu->constant = 0xFFFF;

		FINISH_DIRECT_DECODE();
	}

// -----------------------------------------------------------------------------------------------------
	DECODE(OP_GuildInvite)
	{
		DECODE_LENGTH_EXACT(structs::GuildInvite_Struct);
		SETUP_DIRECT_DECODE(GuildCommand_Struct, structs::GuildInvite_Struct);

		TOBCopyStr(emu->othername, eq->othername);
		TOBCopyStr(emu->myname, eq->myname);
		emu->guildeqid = static_cast<uint16>(TOBToServerGuildID(eq->guild_id));
		emu->officer   = eq->rank;                 // 8 when sent from /guildinvite

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GuildInviteAccept)
	{
		DECODE_LENGTH_EXACT(structs::GuildInviteAccept_Struct);
		SETUP_DIRECT_DECODE(GuildInviteAccept_Struct, structs::GuildInviteAccept_Struct);

		TOBCopyStr(emu->inviter, eq->inviter);
		TOBCopyStr(emu->new_member, eq->new_member);
		emu->response = eq->response;              // rank on accept, 9 on decline (popup case 100)
		emu->guild_id = TOBToServerGuildID(eq->guild_id);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GuildRemove)
	{
		DECODE_LENGTH_EXACT(structs::GuildRemove_Struct);
		SETUP_DIRECT_DECODE(GuildCommand_Struct, structs::GuildRemove_Struct);

		TOBCopyStr(emu->othername, eq->othername);
		TOBCopyStr(emu->myname, eq->myname);
		emu->guildeqid = static_cast<uint16>(TOBToServerGuildID(eq->guild_id));
		emu->officer   = eq->officer;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GuildDemote)
	{
		DECODE_LENGTH_EXACT(structs::GuildDemote_Struct);
		SETUP_DIRECT_DECODE(GuildDemoteStruct, structs::GuildDemote_Struct);

		TOBCopyStr(emu->name, eq->name); // NOTE: ToB sends this empty
		TOBCopyStr(emu->target, eq->target);
		IN(rank);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GuildPromote)
	{
		DECODE_LENGTH_EXACT(structs::GuildPromote_Struct);
		SETUP_DIRECT_DECODE(GuildPromoteStruct, structs::GuildPromote_Struct);

		TOBCopyStr(emu->target, eq->target);
		TOBCopyStr(emu->name, eq->name);
		// ToB sends the new rank once (@136). Handle_OP_GuildPromote takes the new rank from myrank
		// (the emu layout), so fill both; the client's own myrank field is uninitialised.
		emu->rank   = eq->rank;
		emu->myrank = eq->rank;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GuildManageBanker)
	{
		DECODE_LENGTH_EXACT(structs::GuildManageBanker_Struct);
		SETUP_DIRECT_DECODE(GuildManageBanker_Struct, structs::GuildManageBanker_Struct);

		emu->unknown0 = 0;
		TOBCopyStr(emu->myname, eq->myname);
		TOBCopyStr(emu->member, eq->member);
		emu->enabled = eq->enabled;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GuildPublicNote)
	{
		DECODE_LENGTH_EXACT(structs::GuildPublicNote_Struct);
		SETUP_DIRECT_DECODE(GuildUpdate_PublicNote, structs::GuildPublicNote_Struct);

		emu->unknown0 = TOBToServerGuildID(eq->guild_id);
		TOBCopyStr(emu->name, eq->name);
		TOBCopyStr(emu->target, eq->target);
		TOBCopyStr(emu->note, eq->note);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GuildStatus)
	{
		DECODE_LENGTH_EXACT(structs::GuildStatus_Struct);
		SETUP_DIRECT_DECODE(GuildStatus_Struct, structs::GuildStatus_Struct);

		TOBCopyStr(emu->Name, eq->Name);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_SetGuildMOTD)
	{
		DECODE_LENGTH_EXACT(structs::GuildMOTD_Struct);
		SETUP_DIRECT_DECODE(GuildMOTD_Struct, structs::GuildMOTD_Struct);

		emu->unknown0 = TOBToServerGuildID(eq->guild_id);
		TOBCopyStr(emu->name, eq->name);
		TOBCopyStr(emu->setby_name, eq->setby_name);
		emu->unknown132 = eq->unknown136;
		TOBCopyStr(emu->motd, eq->motd);

		FINISH_DIRECT_DECODE();
	}
	DECODE(OP_GetGuildMOTD) { DECODE_FORWARD(OP_SetGuildMOTD); }

	DECODE(OP_GuildUpdate)
	{
		DECODE_LENGTH_ATLEAST(structs::GuildUpdateHeader_Struct);

		constexpr uint32 EMU_HDR = EMU_GUILD_UPDATE_HEADER_SIZE;
		constexpr uint32 TOB_HDR = sizeof(structs::GuildUpdateHeader_Struct);

		unsigned char *__eq_buffer = __packet->pBuffer;
		auto hdr = reinterpret_cast<structs::GuildUpdateHeader_Struct *>(__eq_buffer);
		const uint32 payload = __packet->size - TOB_HDR;

		uint32 action = hdr->action;
		if (action == TOBGuildUpdateRankNames)        { action = GuildUpdateRanks; }
		else if (action == TOBGuildUpdatePermissions) { action = GuildUpdatePermissions; }

		// keep the server's largest struct size so fixed-size casts on the server stay in bounds
		const uint32 emu_size = std::max<uint32>(EMU_HDR + payload, sizeof(GuildUpdateURLAndChannel_Struct));
		auto buf = new unsigned char[emu_size] {};
		uint32 gid = TOBToServerGuildID(hdr->guild_id);
		memcpy(buf + EMU_GUILD_UPDATE_ACTION_OFFSET,   &action,         sizeof(action));
		memcpy(buf + EMU_GUILD_UPDATE_UNKNOWN4_OFFSET, &hdr->unknown4, sizeof(hdr->unknown4));
		memcpy(buf + EMU_GUILD_UPDATE_UNKNOWN8_OFFSET, &hdr->unknown8, sizeof(hdr->unknown8));
		memcpy(buf + EMU_GUILD_UPDATE_NAME_OFFSET,     hdr->name,      sizeof(hdr->name));
		buf[EMU_GUILD_UPDATE_NAME_OFFSET + sizeof(hdr->name) - 1] = '\0';
		memcpy(buf + EMU_GUILD_UPDATE_GUILD_ID_OFFSET, &gid,           sizeof(gid));
		if (payload) {
			memcpy(buf + EMU_HDR, __eq_buffer + TOB_HDR, payload);
		}

		__packet->pBuffer = buf;
		__packet->size    = emu_size;
		delete[] __eq_buffer;
	}

	// OP_GuildLeader (0x1710) is {name[64], target[64]}, which matches the emu GuildMakeLeader: opcode only.


	// OP_Emote, C2S: u32 type + char[1024]
	DECODE(OP_Emote)
	{
		if (__packet->size < sizeof(uint32) + 1) {
			__packet->SetOpcode(OP_Unknown);
			return;
		}
		unsigned char *__eq_buffer = __packet->pBuffer;
		size_t eq_size = __packet->size;

		size_t msg_cap = eq_size - sizeof(uint32);
		std::string old_message((const char *)&__eq_buffer[4], strnlen((const char *)&__eq_buffer[4], msg_cap));
		std::string new_message;
		TOBToServerConvertLinks(new_message, old_message);

		__packet->size = sizeof(Emote_Struct);
		__packet->pBuffer = new unsigned char[__packet->size] {};
		Emote_Struct *emu = (Emote_Struct *)__packet->pBuffer;
		memcpy(&emu->type, __eq_buffer, sizeof(uint32));
		strn0cpy(emu->message, new_message.c_str(), sizeof(emu->message));

		delete[] __eq_buffer;
	}

	// OP_InspectRequest 0x6ea6, C2S:
	DECODE(OP_InspectRequest)
	{
		DECODE_LENGTH_EXACT(structs::Inspect_Struct);
		SETUP_DIRECT_DECODE(Inspect_Struct, structs::Inspect_Struct);
		IN(TargetID);
		IN(PlayerID);
		FINISH_DIRECT_DECODE();
	}

	// OP_GMLastName 0x00a0, C2S: (195 bytes)
	DECODE(OP_GMLastName)
	{
		DECODE_LENGTH_ATLEAST(structs::GMLastName_Struct);
		SETUP_DIRECT_DECODE(GMLastName_Struct, structs::GMLastName_Struct);
		strn0cpy(emu->name, eq->name, std::min(sizeof(emu->name), sizeof(eq->name)));
		strn0cpy(emu->gmname, eq->gmname, std::min(sizeof(emu->gmname), sizeof(eq->gmname)));
		strn0cpy(emu->lastname, eq->lastname, std::min(sizeof(emu->lastname), sizeof(eq->lastname)));
		// emu->unknown[] stays zero (the "update clients" flag is left to the server)
		FINISH_DIRECT_DECODE();
	}

	// OP_RaidInvite 0x482c, C2S: serialized raid object
	//   u32 action; cstr player_name; u32 unknown1; [cstr leader_name; u32 parameter; [cstr text]]
	DECODE(OP_RaidInvite)
	{
		const size_t size = __packet->size;
		__packet->SetReadPosition(0);

		char player_name[64] = {}, leader_name[64] = {};
		uint32 action = __packet->ReadUInt32();
		std::string player_name_str;
		__packet->ReadString(player_name_str, sizeof(player_name) - 1);
		if (__packet->ReadError()) {
			LogNetcode("TOB OP_RaidInvite too short ([{}] bytes)", size);
			__packet->SetOpcode(OP_Unknown);
			return;
		}
		const uint32 unknown1 = __packet->ReadUInt32();   // missing fields stay 0
		std::string leader_name_str;
		__packet->ReadString(leader_name_str, sizeof(leader_name) - 1);
		const uint32 parameter = __packet->ReadUInt32();
		strn0cpy(player_name, player_name_str.c_str(), sizeof(player_name));
		strn0cpy(leader_name, leader_name_str.c_str(), sizeof(leader_name));

		// ToB raid-options loot commands use the "2" variants; the server handles them as the plain loot commands
		switch (action) {
			case RaidCommandLootType2: action = RaidCommandLootType; break;
			case RaidCommandAddLooter2: action = RaidCommandAddLooter; break;
			case RaidCommandRemoveLooter2: action = RaidCommandRemoveLooter; break;
			default: break;
		}

		unsigned char *__eq_buffer = __packet->pBuffer;   // keep the original buffer until we are done reading it

		if (action == raidSetMotd || action == raidSetNote) {
			std::string text;
			if (!__packet->ReadError() && __packet->ReadRemaining() > 0) {
				const char *s = reinterpret_cast<const char *>(__packet->pBuffer + __packet->GetReadPosition());
				text.assign(s, strnlen(s, __packet->ReadRemaining()));
			}
			if (action == raidSetMotd) {
				__packet->size = sizeof(RaidMOTD_Struct);
				__packet->pBuffer = new unsigned char[__packet->size] {};
				auto emu = (RaidMOTD_Struct *)__packet->pBuffer;
				emu->general.action = action;
				emu->general.unknown1 = unknown1;
				emu->general.parameter = parameter;
				strn0cpy(emu->general.player_name, player_name, sizeof(emu->general.player_name));
				strn0cpy(emu->general.leader_name, leader_name, sizeof(emu->general.leader_name));
				strn0cpy(emu->motd, text.c_str(), sizeof(emu->motd));
			}
			else {
				__packet->size = sizeof(RaidNote_Struct);
				__packet->pBuffer = new unsigned char[__packet->size] {};
				auto emu = (RaidNote_Struct *)__packet->pBuffer;
				emu->general.action = action;
				emu->general.unknown1 = unknown1;
				emu->general.parameter = parameter;
				strn0cpy(emu->general.player_name, player_name, sizeof(emu->general.player_name));
				strn0cpy(emu->general.leader_name, leader_name, sizeof(emu->general.leader_name));
				strn0cpy(emu->note, text.c_str(), sizeof(emu->note));
			}
		}
		else {
			__packet->size = sizeof(RaidGeneral_Struct);
			__packet->pBuffer = new unsigned char[__packet->size] {};
			auto emu = (RaidGeneral_Struct *)__packet->pBuffer;
			emu->action = action;
			emu->unknown1 = unknown1;
			emu->parameter = parameter;
			strn0cpy(emu->player_name, player_name, sizeof(emu->player_name));
			strn0cpy(emu->leader_name, leader_name, sizeof(emu->leader_name));
		}

		delete[] __eq_buffer;
	}

	DECODE(OP_TributeItem)
	{
		DECODE_LENGTH_EXACT(structs::TributeItem_Struct);
		SETUP_DIRECT_DECODE(TributeItem_Struct, structs::TributeItem_Struct);

		emu->slot = TOBToServerSlot(eq->inventory_slot);
		IN(quantity);
		IN(tribute_master_id);
		IN(tribute_points);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GuildTributeDonatePlat)
	{
		DECODE_LENGTH_EXACT(structs::GuildTributeDonatePlatRequest_Struct);
		SETUP_DIRECT_DECODE(GuildTributeDonatePlatRequest_Struct, structs::GuildTributeDonatePlatRequest_Struct);

		IN(quantity);
		IN(tribute_master_id);
		emu->unknown08 = eq->unknown08;
		emu->guild_id = TOBToServerGuildID(eq->guild_id);
		emu->unknown16 = 0;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GuildTributeDonateItem)
	{
		DECODE_LENGTH_EXACT(structs::GuildTributeDonateItemRequest_Struct);
		SETUP_DIRECT_DECODE(GuildTributeDonateItemRequest_Struct, structs::GuildTributeDonateItemRequest_Struct);

		// server struct splits the slot into type/slot/sub_index/aug_index; convert through the server slot id
		// after the IN()s.
		uint32 server_slot = TOBToServerSlot(eq->slot);
		emu->type = eq->slot.Type;
		emu->slot = static_cast<uint16>(server_slot);
		emu->sub_index = eq->slot.SubIndex;
		emu->aug_index = eq->slot.AugIndex;
		emu->unknown10 = 0;
		IN(quantity);
		IN(tribute_master_id);
		IN(unknown20);
		emu->guild_id = TOBToServerGuildID(eq->guild_id);
		emu->unknown28 = 0;
		emu->unknown32 = 0;

		FINISH_DIRECT_DECODE();
	}
	DECODE(OP_GuildTributeToggleReq)
	{
		DECODE_LENGTH_EXACT(structs::GuildTributeToggleReq_Struct);
		SETUP_DIRECT_DECODE(GuildTributeToggleReq_Struct, structs::GuildTributeToggleReq_Struct);

		emu->command = eq->command & 0xFF; // the client writes a u8 (1 on, 0 off); the rest is stack garbage
		emu->unknown4 = 0;
		emu->unknown8 = 0;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_AltCurrencyReclaim)
	{
		DECODE_LENGTH_EXACT(structs::AltCurrencyReclaim_Struct);
		SETUP_DIRECT_DECODE(AltCurrencyReclaim_Struct, structs::AltCurrencyReclaim_Struct);

		IN(currency_id);
		IN(unknown004);
		IN(count);
		IN(reclaim_flag);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_AltCurrencySell)
	{
		DECODE_LENGTH_EXACT(structs::AltCurrencySellItem_Struct);
		SETUP_DIRECT_DECODE(AltCurrencySellItem_Struct, structs::AltCurrencySellItem_Struct);

		IN(merchant_entity_id);
		emu->slot_id = TOBToServerTypelessSlot(eq->inventory_slot, invtype::typePossessions);
		IN(charges);
		IN(cost);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ShopSendParcel)
	{
		DECODE_LENGTH_EXACT(structs::Parcel_Struct);
		SETUP_DIRECT_DECODE(Parcel_Struct, structs::Parcel_Struct);

		IN(npc_id);
		IN(quantity);
		IN(money_flag);
		emu->item_slot = TOBToServerTypelessSlot(eq->inventory_slot, invtype::typePossessions);
		strn0cpy(emu->send_to, eq->send_to, sizeof(emu->send_to));
		strn0cpy(emu->note, eq->note, sizeof(emu->note));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_AltCurrencySellSelection)
	{
		DECODE_LENGTH_EXACT(structs::AltCurrencySelectItem_Struct);
		SETUP_DIRECT_DECODE(AltCurrencySelectItem_Struct, structs::AltCurrencySelectItem_Struct);

		memset(emu, 0, sizeof(AltCurrencySelectItem_Struct));
		IN(merchant_entity_id);
		emu->slot_id = TOBToServerTypelessSlot(eq->inventory_slot, invtype::typePossessions);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GroupRoles)
	{
		DECODE_LENGTH_EXACT(structs::GroupRole_Struct);
		SETUP_DIRECT_DECODE(GroupRole_Struct, structs::GroupRole_Struct);

		memset(emu, 0, sizeof(GroupRole_Struct));
		memcpy(emu->Name1, eq->Name1, sizeof(emu->Name1));
		memcpy(emu->Name2, eq->Name2, sizeof(emu->Name2));
		IN(RoleNumber);
		IN(Toggle);

		FINISH_DIRECT_DECODE();
	}

	// Modify Benefits button / window close: one command byte, then stack garbage
	DECODE(OP_GuildModifyBenefits)
	{
		DECODE_LENGTH_EXACT(structs::GuildTributeModifyBenefits_Struct);
		SETUP_DIRECT_DECODE(GuildTributeModifyBenefits_Struct, structs::GuildTributeModifyBenefits_Struct);

		memset(emu, 0, sizeof(GuildTributeModifyBenefits_Struct));
		emu->command           = eq->command ? GUILD_TRIBUTES_MODIFY : GUILD_TRIBUTES_SAVE;
		emu->tribute_master_id = eq->tribute_master_id;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GuildOptInOut)
	{
		DECODE_LENGTH_EXACT(structs::GuildTributeOptInOutReq_Struct);
		SETUP_DIRECT_DECODE(GuildTributeOptInOutReq_Struct, structs::GuildTributeOptInOutReq_Struct);

		emu->guild_id = TOBToServerGuildID(eq->guild_id);
		IN(tribute_toggle);
		TOBCopyStr(emu->player, eq->player);
		IN(command);
		IN(tribute_master_id);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GuildSaveActiveTributes)
	{
		DECODE_LENGTH_EXACT(structs::GuildTributeSaveActive_Struct);
		SETUP_DIRECT_DECODE(GuildTributeSaveActive_Struct, structs::GuildTributeSaveActive_Struct);

		IN(command);
		IN(master_tribute_id);
		IN(tribute_id_1);
		IN(tribute_id_2);
		IN(tribute_1_tier);
		IN(tribute_2_tier);

		FINISH_DIRECT_DECODE();
	}



	DECODE(OP_LoadSpellSet)
	{
		DECODE_LENGTH_EXACT(structs::LoadSpellSet_Struct);
		SETUP_DIRECT_DECODE(LoadSpellSet_Struct, structs::LoadSpellSet_Struct);

		constexpr uint32 wire_slots = sizeof(eq->spell) / sizeof(eq->spell[0]); // 16
		uint32 used = std::min({ eq->count, wire_slots, static_cast<uint32>(EQ::spells::SPELL_GEM_COUNT) });

		for (uint32 i = 0; i < EQ::spells::SPELL_GEM_COUNT; ++i) {
			emu->spell[i] = 0xFFFFFFFF;  // no action
		}
		for (uint32 i = 0; i < used; ++i) { // bytes past count are uninitialised stack in the client
			if (eq->spell[i] != 0) {
				emu->spell[i] = eq->spell[i];
			}
		}
		emu->unknown = eq->count;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_VetClaimRequest)
	{
		// ToB: lenstr name, u32 unknown, u8 claim, u32 n, n x {u32 claim_id, u32 count}, u32 action  (same object as
		// VetClaimReply). unknown is not the claim id (Live sends 0): the claim is pairs[0]; n == 0 is a list refresh.
		constexpr size_t VeteranClaimPairSize = 2 * sizeof(uint32); // {u32 claim_id, u32 count}
		constexpr uint32 NoVeteranClaim       = 0xFFFFFFFF;         // claim id when the request names no claim

		const uint32 size = __packet->size;
		__packet->SetReadPosition(0);

		uint32 claim_id = NoVeteranClaim, action = 0;
		std::string name;
		__packet->ReadLengthString(name);
		if (name.size() > TOB_NAME_MAX_CHARS) {
			name.resize(TOB_NAME_MAX_CHARS);
		}
		__packet->ReadUInt32(); // unknown
		bool ok = !__packet->ReadError();
		if (ok && __packet->ReadRemaining() > 0) {
			__packet->ReadSkip(1); // u8
			const uint32 n = __packet->ReadUInt32();
			if (!__packet->ReadError() && n <= __packet->ReadRemaining() / VeteranClaimPairSize) {
				if (n >= 1) {
					claim_id = __packet->ReadUInt32(); // pairs[0].claim_id
					__packet->ReadSkip(sizeof(uint32)); // pairs[0].count
					__packet->ReadSkip((n - 1) * VeteranClaimPairSize);
				}
				action = __packet->ReadUInt32();
			}
		}
		if (!ok) {
			LogNetcode("TOB::DECODE(OP_VetClaimRequest) malformed, size [{}]", size);
			__packet->SetOpcode(OP_Unknown);
			return;
		}

		delete[] __packet->pBuffer;
		__packet->size = sizeof(VeteranClaim);
		__packet->pBuffer = new unsigned char[__packet->size] {};
		auto emu = reinterpret_cast<VeteranClaim*>(__packet->pBuffer);
		strn0cpy(emu->name, name.c_str(), sizeof(emu->name));
		emu->claim_id = claim_id;
		emu->action = action;
	}

	// {u64 guild_id}: the client asks for a guild name it doesn't know (it shows "Unknown Guild"
	// meanwhile). The server answers with OP_GuildRenameGuild.
	DECODE(OP_GuildNameRequest)
	{
		DECODE_LENGTH_EXACT(uint64);
		unsigned char *__eq_buffer = __packet->pBuffer;
		uint64 gid = 0;
		memcpy(&gid, __eq_buffer, sizeof(gid));
		uint32 server_gid = TOBToServerGuildID(gid);
		__packet->size = sizeof(uint32);
		__packet->pBuffer = new unsigned char[sizeof(uint32)];
		memcpy(__packet->pBuffer, &server_gid, sizeof(uint32));
		delete[] __eq_buffer;
	}

	// ToB sends /mark and /rmark as MarkNPC_Struct {target id, number, name} on the mark opcodes (0x1793 group,
	// 0x4187 raid). The server only handles marking through OP_DoGroupLeadershipAbility, so convert.
	static void TOBMarkToLeadershipAbility(EQApplicationPacket *__packet, uint32 ability)
	{
		if (__packet->size < 2 * sizeof(uint32)) {
			__packet->SetOpcode(OP_Unknown);
			return;
		}
		uint32 number = 0;
		memcpy(&number, __packet->pBuffer + sizeof(uint32), sizeof(uint32));
		unsigned char *__eq_buffer = __packet->pBuffer;
		__packet->size    = sizeof(DoGroupLeadershipAbility_Struct);
		__packet->pBuffer = new unsigned char[__packet->size] {};
		auto emu = reinterpret_cast<DoGroupLeadershipAbility_Struct *>(__packet->pBuffer);
		emu->Ability   = ability;
		emu->Parameter = number;
		__packet->SetOpcode(OP_DoGroupLeadershipAbility);
		delete[] __eq_buffer;
	}

	DECODE(OP_MarkRaidNPC) { TOBMarkToLeadershipAbility(__packet, RaidLeadershipAbility_MarkNPC); }
	DECODE(OP_MarkNPC) { TOBMarkToLeadershipAbility(__packet, GroupLeadershipAbility_MarkNPC); }

	// /inspectbuffs [mine] sends one byte (1 = only buffs I cast) on its own opcode
	DECODE(OP_InspectBuffs)
	{
		DECODE_LENGTH_EXACT(structs::InspectBuffsRequest_Struct);
		SETUP_DIRECT_DECODE(InspectBuffsRequest_Struct, structs::InspectBuffsRequest_Struct);

		emu->only_mine = eq->only_mine == 1 ? 1 : 0;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_FindPersonRequest)
	{
		DECODE_LENGTH_EXACT(structs::FindPersonRequest_Struct);
		SETUP_DIRECT_DECODE(FindPersonRequestEx_Struct, structs::FindPersonRequest_Struct);

		// Only FindPersonRequestSpawn refers to a spawn from OP_SendFindableNPCs (client sends id & 0xffff).
		// ZonePoint and Location are zone connection rows (zone point number / destination); other types give
		// an empty reply.
		emu->request.npc_id = (eq->request_type == FindPersonRequestSpawn) ? static_cast<uint32>(eq->id) : 0;
		emu->request.client_pos.x = eq->client_pos.x;
		emu->request.client_pos.y = eq->client_pos.y;
		emu->request.client_pos.z = eq->client_pos.z;
		IN(request_type);
		emu->zone_point_number = (eq->request_type == FindPersonRequestZonePoint) ? static_cast<uint32>(eq->id) : 0;
		IN(dest.x);
		IN(dest.y);
		IN(dest.z);

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

	DECODE(OP_GMSummon)
	{
		DECODE_LENGTH_EXACT(structs::GMSummon_Struct);
		SETUP_DIRECT_DECODE(GMSummon_Struct, structs::GMSummon_Struct);

		strn0cpy(emu->charname, eq->charname, sizeof(emu->charname));
		strn0cpy(emu->gmname, eq->gmname, sizeof(emu->gmname));
		emu->success = eq->success;
		IN(zoneID);
		IN(y);
		IN(x);
		IN(z);
		IN(unknown2);

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
		DECODE_LENGTH_EXACT(structs::GroupGeneric_Struct);
		SETUP_DIRECT_DECODE(GroupGeneric_Struct, structs::GroupGeneric_Struct);

		memcpy(emu->name1, eq->name1, sizeof(emu->name1));
		memcpy(emu->name2, eq->name2, sizeof(emu->name2));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_GroupFollow)
	{
		DECODE_LENGTH_ATLEAST(structs::GroupFollow_Struct);
		SETUP_DIRECT_DECODE(GroupGeneric_Struct, structs::GroupFollow_Struct);
		memcpy(emu->name1, eq->name1, sizeof(emu->name1));
		memcpy(emu->name2, eq->name2, sizeof(emu->name2));
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

	DECODE(OP_Surname)
	{
		DECODE_LENGTH_ATLEAST(structs::Surname_Struct);
		auto eq = reinterpret_cast<const structs::Surname_Struct *>(__packet->pBuffer);
		__packet->SetReadPosition(sizeof(structs::Surname_Struct));
		if (eq->length > __packet->ReadRemaining()) {
			__packet->SetOpcode(OP_Unknown);
			return;
		}

		unsigned char *__eq_buffer = __packet->pBuffer;
		const int32    status      = eq->status;
		const std::string surname(
			reinterpret_cast<const char *>(__eq_buffer + sizeof(structs::Surname_Struct)),
			eq->length
		);

		__packet->size    = sizeof(Surname_Struct);
		__packet->pBuffer = new unsigned char[__packet->size]{};
		auto emu = reinterpret_cast<Surname_Struct *>(__packet->pBuffer);
		emu->unknown0064 = static_cast<uint32>(status);
		strn0cpy(emu->lastname, surname.c_str(), sizeof(emu->lastname));
		delete[] __eq_buffer;
	}

	DECODE(OP_ClickObject)
	{
		DECODE_LENGTH_EXACT(structs::ClickObject_Struct);
		SETUP_DIRECT_DECODE(ClickObject_Struct, structs::ClickObject_Struct);
		IN(drop_id);
		IN(player_id);
		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_RezzAnswer)
	{
		DECODE_LENGTH_ATLEAST(structs::Resurrect_Struct);
		SETUP_DIRECT_DECODE(Resurrect_Struct, structs::Resurrect_Struct);
		IN(unknown000);
		IN(zone_id);
		IN(instance_id);
		IN(y);
		IN(x);
		IN(z);
		IN(unknown020);
		strn0cpy(emu->your_name, eq->your_name, sizeof(emu->your_name));
		emu->unknown088 = static_cast<uint32>(eq->unknown088);
		strn0cpy(emu->rezzer_name, eq->rezzer_name, sizeof(emu->rezzer_name));
		IN(spellid);
		strn0cpy(emu->corpse_name, eq->corpse_name, sizeof(emu->corpse_name));
		IN(action);
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
			delete[] __eq_buffer; // was leaked on every memorize start
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

		// ToB has slots the server doesn't: general 11/12, shared bank past the second slot, and the bags in them.
		// TOBToServerSlot maps them to SLOT_INVALID, which Client::SwapItem treats as "destroy the cursor item".
		// Send SLOT_REJECT instead so the move is refused (the item stays on the cursor).
		auto tob_extra_slot = [](const structs::InventorySlot_Struct &s) {
			const bool extra_general = s.Type == invtype::typePossessions &&
				(s.Slot == invslot::slotGeneral11 || s.Slot == invslot::slotGeneral12);
			const bool unmapped_storage = (s.Type == invtype::typeBank || s.Type == invtype::typeSharedBank) &&
				s.Slot != invslot::SLOT_INVALID && TOBToServerSlot(s) == static_cast<uint32>(EQ::invslot::SLOT_INVALID);
			return extra_general || unmapped_storage;
		};

		// key ring and stat slots that do not map (stat selection, bad index) must not read as "destroy" either
		auto tob_unmapped_ring = [](const structs::InventorySlot_Struct &s) {
			return s.Type >= invtype::typeMountKeyRingItems && s.Type <= invtype::typeViewModActivatedKeyRingItems &&
				TOBToServerSlot(s) == static_cast<uint32>(EQ::invslot::SLOT_INVALID);
		};

		emu->from_slot = (tob_extra_slot(eq->from_slot) || tob_unmapped_ring(eq->from_slot)) ? invslot::SLOT_REJECT : TOBToServerSlot(eq->from_slot);
		emu->to_slot   = (tob_extra_slot(eq->to_slot) || tob_unmapped_ring(eq->to_slot)) ? invslot::SLOT_REJECT : TOBToServerSlot(eq->to_slot);
		IN(number_in_stack);

		FINISH_DIRECT_DECODE();
	}

	// GM petition queue: check-in sends the record header + gmtext as a C string (variable length)
	DECODE(OP_PetitionCheckIn)
	{
		const size_t wire_size = __packet->size;
		if (wire_size <= structs::PetitionCheckInHeaderSize) {
			LogNetcode("Wrong size on incoming [{}] (TOB petition check-in): Got [{}], expected more than [{}]",
				opcodes->EmuToName(__packet->GetOpcode()), wire_size, structs::PetitionCheckInHeaderSize);
			__packet->SetOpcode(OP_Unknown);
			return;
		}
		SETUP_DIRECT_DECODE(Petition_Struct, structs::Petition_Struct);

		IN(petnumber);
		IN(urgency);
		strn0cpy(emu->accountid, eq->accountid, sizeof(emu->accountid));
		strn0cpy(emu->lastgm, eq->lastgm, sizeof(emu->lastgm));
		IN(zone);
		strn0cpy(emu->charname, eq->charname, sizeof(emu->charname));
		IN(charlevel);
		IN(charclass);
		IN(charrace);
		IN(checkouts);
		IN(unavail);
		emu->senttime = eq->senttime;
		strn0cpy(emu->petitiontext, eq->petitiontext, sizeof(emu->petitiontext));

		const char *gm = reinterpret_cast<const char *>(__eq_buffer) + structs::PetitionCheckInHeaderSize;
		size_t gm_len = strnlen(gm, std::min(wire_size - structs::PetitionCheckInHeaderSize, sizeof(emu->gmtext) - 1));
		memcpy(emu->gmtext, gm, gm_len);
		emu->gmtext[gm_len] = '\0';

		FINISH_DIRECT_DECODE();
	}

	// delete / resolve send only the u32 petition id; the handler wants a PetitionUpdate_Struct
	DECODE(OP_PetitionDelete)
	{
		DECODE_LENGTH_EXACT(uint32);
		SETUP_DIRECT_DECODE(PetitionUpdate_Struct, uint32);

		emu->petnumber = *eq;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_PetitionResolve)
	{
		DECODE_LENGTH_EXACT(uint32);
		SETUP_DIRECT_DECODE(PetitionUpdate_Struct, uint32);

		emu->petnumber = *eq;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_PickZone)
	{
		DECODE_LENGTH_EXACT(structs::PickZone_Struct);
		SETUP_DIRECT_DECODE(PickZone_Struct, structs::PickZone_Struct);

		emu->session_id   = static_cast<int64>(eq->session_id);
		emu->selection_id = static_cast<int32>(eq->selection_id);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_TradeSkillRecipeInspect)
	{
		DECODE_LENGTH_EXACT(structs::TradeSkillRecipeInspect_Struct);
		SETUP_DIRECT_DECODE(TradeSkillRecipeInspect_Struct, structs::TradeSkillRecipeInspect_Struct);

		IN(recipe_id);

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_PickPocket)
	{
		DECODE_LENGTH_ATLEAST(structs::PickPocket_Struct);
		SETUP_DIRECT_DECODE(PickPocket_Struct, structs::PickPocket_Struct);

		emu->to      = eq->to;
		emu->from    = eq->from;
		emu->myskill = static_cast<uint16>(eq->myskill);
		emu->type    = eq->type;
		emu->coin    = eq->coin;

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_ReadBook)
	{
		// Client always sends 8216 bytes (struct is 8215); ATLEAST accepts the extra trailing byte.
		DECODE_LENGTH_ATLEAST(structs::BookRequest_Struct);
		SETUP_DIRECT_DECODE(BookRequest_Struct, structs::BookRequest_Struct);

		IN(type);
		emu->invslot = static_cast<int16_t>(TOBToServerTypelessSlot(eq->invslot, invtype::typePossessions));
		IN(target_id);
		emu->window = (uint8)eq->window;
		strn0cpy(emu->txtfile, eq->txtfile, sizeof(emu->txtfile));

		FINISH_DIRECT_DECODE();
	}

	DECODE(OP_RecipeAutoCombine)
	{
		DECODE_LENGTH_EXACT(structs::RecipeAutoCombine_CS_Struct);
		SETUP_DIRECT_DECODE(RecipeAutoCombine_Struct, structs::RecipeAutoCombine_CS_Struct);

		// The recipe id is always at eq[+32] (struct field object_type); eq[+4] (recipe_id) is the container
		// object's id, 0 for a world container. Container type comes from con_type (eq[+0]).
		emu->object_type = eq->con_type;
		IN(some_id);
		emu->unknown1   = TOBToServerSlot(eq->container_slot);
		emu->recipe_id  = eq->object_type;
		emu->reply_code = 0;  // junk in client request; server overwrites in reply

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

	DECODE(OP_Trader)
	{
		if (__packet->size < sizeof(uint32)) {
			__packet->SetOpcode(OP_Unknown);
			return;
		}
		auto action = *(uint32 *)__packet->pBuffer;

		switch (action) {
			case structs::TOBBazaarTraderBuyerActions::BeginTraderMode: {
				DECODE_LENGTH_EXACT(structs::BeginTrader_Struct);
				SETUP_DIRECT_DECODE(ClickTrader_Struct, structs::BeginTrader_Struct);
				LogTrading("(TOB) BeginTraderMode action <green>[{}]", action);

				emu->action = TraderOn;
				std::copy_n(eq->item_cost, invtype::BAZAAR_SIZE, emu->item_cost);
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
			case structs::TOBBazaarTraderBuyerActions::EndTraderMode: {
				DECODE_LENGTH_EXACT(structs::Trader_ShowItems_Struct);
				SETUP_DIRECT_DECODE(Trader_ShowItems_Struct, structs::Trader_ShowItems_Struct);
				LogTrading("(TOB) EndTraderMode action <green>[{}]", action);

				emu->action    = TraderOff;
				emu->entity_id = eq->entity_id;

				FINISH_DIRECT_DECODE();
				break;
			}
			case structs::TOBBazaarTraderBuyerActions::ListTraderItems: {
				LogTrading("(TOB) ListTraderItems action <green>[{}]", action);
				break;
			}
			case structs::TOBBazaarTraderBuyerActions::PriceUpdate: {
				DECODE_LENGTH_EXACT(structs::TraderPriceUpdate_Struct);
				SETUP_DIRECT_DECODE(TraderPriceUpdate_Struct, structs::TraderPriceUpdate_Struct);
				LogTrading("(TOB) PriceUpdate action <green>[{}]", action);

				emu->Action       = PriceUpdate;
				emu->SerialNumber = Strings::ToUnsignedBigInt(eq->serial_number, 0);
				if (emu->SerialNumber == 0) {
					LogTrading("(TOB) Price change with invalid serial number <red>[{}]", eq->serial_number);
				}
				emu->NewPrice = eq->new_price;

				FINISH_DIRECT_DECODE();
				break;
			}
			default: {
				LogTrading("(TOB) Unhandled action <red>[{}]", action);
			}
		}
	}

	DECODE(OP_TraderBuy)
	{
		DECODE_LENGTH_EXACT(structs::TraderBuy_Struct);
		SETUP_DIRECT_DECODE(TraderBuy_Struct, structs::TraderBuy_Struct);
		LogTrading(
			"(TOB) item_id <green>[{}] price <green>[{}] quantity <green>[{}] trader_id <green>[{}]",
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
		if (__packet->size < sizeof(uint32)) {
			__packet->SetOpcode(OP_Unknown);
			return;
		}
		uint32 action = *(uint32 *)__packet->pBuffer;

		switch (action) {
			case structs::TOBBazaarTraderBuyerActions::BazaarSearch: {
				DECODE_LENGTH_EXACT(structs::BazaarSearch_Struct);
				SETUP_DIRECT_DECODE(BazaarSearchCriteria_Struct, structs::BazaarSearch_Struct);
				LogTrading(
					"(TOB) BazaarSearch action <green>[{}]",
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
			case structs::TOBBazaarTraderBuyerActions::ClickTrader: {
				DECODE_LENGTH_EXACT(structs::TraderClick_Struct);
				SETUP_DIRECT_DECODE(TraderClick_Struct, structs::TraderClick_Struct);

				emu->Code     = ClickTrader;
				emu->TraderID = eq->trader_id;
				emu->Approval = eq->unknown_008;
				LogTrading("(TOB) ClickTrader action <green>[{}], trader_id <green>[{}], approval <green>[{}]",
						   eq->action,
						   eq->trader_id,
						   eq->unknown_008
				);
				FINISH_DIRECT_DECODE();
				break;
			}
			case structs::TOBBazaarTraderBuyerActions::BazaarInspect: {
				DECODE_LENGTH_EXACT(structs::BazaarInspect_Struct);
				SETUP_DIRECT_DECODE(BazaarInspect_Struct, structs::BazaarInspect_Struct);

				__packet->SetOpcode(OP_BazaarSearch);
				IN(item_id);
				IN(trader_id);
				emu->action        = BazaarInspect;
				emu->serial_number = Strings::ToUnsignedInt(eq->serial_number, 0);
				if (emu->serial_number == 0) {
					LogTrading(
						"(TOB) trader_id = <green>[{}] requested a BazaarInspect with an invalid serial number of <red>[{}]",
						eq->trader_id,
						eq->serial_number
					);
					FINISH_DIRECT_DECODE();
					return;
				}

				LogTrading("(TOB) BazaarInspect action <green>[{}] item_id <green>[{}] serial_number <green>[{}]",
						   action,
						   eq->item_id,
						   eq->serial_number
				);
				FINISH_DIRECT_DECODE();
				break;
			}
			case structs::TOBBazaarTraderBuyerActions::FirstOpenSearch: {
				__packet->SetOpcode(OP_BazaarSearch);
				LogTrading("(TOB) First time opening Bazaar Search since zoning. Action <green>[{}]", action);
				break;
			}
			case structs::TOBBazaarTraderBuyerActions::WelcomeMessage: {
				__packet->SetOpcode(OP_BazaarSearch);
				LogTrading("(TOB) WelcomeMessage action <green>[{}]", action);
				break;
			}
			case structs::TOBBazaarTraderBuyerActions::BuyTraderItem: {
				DECODE_LENGTH_EXACT(structs::TraderBuy_Struct);
				SETUP_DIRECT_DECODE(TraderBuy_Struct, structs::TraderBuy_Struct);
				LogTrading(
					"(TOB) item_id <green>[{}] price <green>[{}] quantity <green>[{}] trader_id <green>[{}]",
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
				LogTrading("(TOB) Unhandled action <red>[{}]", action);
			}
			return;
		}
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
		// TOB splits RoF2's combined guildid field into a flag (0x94) and actual ID (0x98).
		// When guildid is non-zero it is a real guild ID; otherwise fall back to guildid_flag
		// which carries 0xFFFFFFFF (no filter) or trader/buyer sentinel values.
		emu->guildid = eq->guildid ? eq->guildid : eq->guildid_flag;
		IN(type);

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
		buffer.WriteUInt8(item->Tradeskills ? 1 : 0);

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
		case ItemPacketOverflow:
			// the window does not read the slot, it lists everything sent with this packet type
			slot_id.Type     = invtype::typeOverflow;
			slot_id.Slot     = (slot_id_in >= EQ::invslot::OVERFLOW_BEGIN && slot_id_in <= EQ::invslot::OVERFLOW_END)
				? slot_id_in - EQ::invslot::OVERFLOW_BEGIN : invslot::SLOT_INVALID;
			slot_id.SubIndex = invbag::SLOT_INVALID;
			slot_id.AugIndex = invaug::SOCKET_INVALID;
			break;
		default:
			slot_id = ServerToTOBSlot(slot_id_in);
			break;
		}

		// window items are found by the index below, so they never carry a merchant slot
		const bool window_item   = packet_type == ItemPacketOverflow;
		const bool merchant_item = inst->GetMerchantSlot() && !window_item;

		//u32 slot_type;
		buffer.WriteUInt32(merchant_item ? invtype::typeMerchant : slot_id.Type);
		//s16 main_slot;
		//s16 sub_slot;
		//s16 aug_slot;
		buffer.WriteInt16(merchant_item ? inst->GetMerchantSlot() : slot_id.Slot);
		buffer.WriteInt16(merchant_item ? 0xffff : slot_id.SubIndex);
		buffer.WriteInt16(merchant_item ? 0xffff : slot_id.AugIndex);

		//u64 price;
		buffer.WriteUInt64(inst->GetPrice());

		//u32 MerchantQuantity;
		buffer.WriteUInt32(merchant_item ? inst->GetMerchantCount() : 1);

		//u32 ScriptIndex;
		buffer.WriteUInt32(inst->IsScaling() ? (inst->GetExp() / 100) : 0);

		//u64 MerchantSlot; = u32 instance_id + u32 parcel_item_id. The client echoes the high dword as
		// ParcelRetrieve_Struct::parcel_item_id, so parcels need the item id there (was 0: retrieve failed)
		uint64 merchant_slot = static_cast<uint32>(merchant_item ? inst->GetMerchantSlot() : inst->GetSerialNumber());
		if (packet_type == ItemPacketParcel) {
			merchant_slot |= static_cast<uint64>(inst->GetID()) << 32;
		}
		// evolving items are found by their evolve unique id here (OP_EvolveItem updates, xp transfer window)
		if (item->EvolvingItem && inst->GetEvolveUniqueID() && packet_type != ItemPacketParcel && packet_type != ItemPacketMerchant && !window_item) {
			merchant_slot = inst->GetEvolveUniqueID();
		}
		buffer.WriteUInt64(merchant_slot);

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
			inst->CalculateEvolveProgression();

			//s32 GroupId;
			buffer.WriteInt32(0);

			//s32 EvolvingCurrentLevel;
			buffer.WriteInt32(item->EvolvingLevel);

			//double EvolvingExpPct;
			buffer.WriteDouble(inst->GetEvolveProgression());
			
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

	// client_tags_brackets: the client turns [text] into dialog links itself for plain chat/special messages,
	// but not inside OP_FormattedMessage arguments (NPC say), which need complete links.
	static void ServerToTOBConvertLinks(std::string& message_out, const std::string& message_in, bool client_tags_brackets)
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

					// Say links are encoded as item links with SAYLINK_ITEM_ID (0xFFFFF = "FFFFF").
					// TOB supports dialog links natively, so convert to a proper dialog link.
					if (item_id == "FFFFF") {
						// keyword = proxy text (display name), phrase = saylink text from cache
						std::string keyword = segments[segment_iter].substr(EQ::constants::SAY_LINK_BODY_SIZE);
						uint32 aug1_id = std::stoul(segments[segment_iter].substr(6, 5), nullptr, 16);
						uint32 aug2_id = std::stoul(segments[segment_iter].substr(11, 5), nullptr, 16);
						uint32 saylink_id = (aug1_id != 0) ? aug1_id : aug2_id;
						std::string phrase = EQ::SayLinkEngine::GetSaylinkPhrase(saylink_id);

						// The client's TagBracketedTextAsDialogueResponseLinks runs before ConvertItemTags
						// and converts [text] -> [\x124text\x12]. Only send the content of the link in this case
						char dialog_link[Links::MAX_LINK_SIZE];
						size_t next_seg = segment_iter + 1;
						if (client_tags_brackets && !message_out.empty() && message_out.back() == '[' &&
							next_seg < segments.size() && !segments[next_seg].empty() && segments[next_seg].front() == ']') {
							// only do this if it is enclosed by square brackets
							Links::FormatDialogLinkContent(dialog_link, sizeof(dialog_link), keyword, phrase);
						} else if (client_tags_brackets && keyword.size() > 2 && keyword.front() == '[' && keyword.back() == ']') {
							// the brackets are part of the link text (quest::saylink(..., "[Text]")): the client would
							// re-tag them and mangle the link, so emit "[content]" for it to turn into a link instead
							message_out.push_back('[');
							Links::FormatDialogLinkContent(dialog_link, sizeof(dialog_link),
								keyword.substr(1, keyword.size() - 2), phrase);
							message_out.append(dialog_link);
							dialog_link[0] = ']';
							dialog_link[1] = '\0';
						} else {
							Links::FormatDialogLink(dialog_link, sizeof(dialog_link), keyword, phrase);
						}

						message_out.append(dialog_link);
					} else {
						EQ::SayLinkBody_Struct body{};
						if (EQ::saylink::DeserializeLinkBody(body, segments[segment_iter].substr(0, EQ::constants::SAY_LINK_BODY_SIZE))) {
							std::string text = segments[segment_iter].substr(EQ::constants::SAY_LINK_BODY_SIZE);
							char item_link[Links::MAX_LINK_SIZE];
							Links::FormatItemLink(item_link, sizeof(item_link), text, body);

							// the TOB client will replace anything in between [] with a dialog link, which explicitly
							// breaks item links, so replace [] with ()
							if (!message_out.empty() && message_out.back() == '[')
								message_out.back() = '(';

							message_out.append(item_link);

							size_t next_seg = segment_iter + 1;
							if (next_seg < segments.size() && !segments[next_seg].empty() && segments[next_seg].front() == ']')
								segments[next_seg].front() = ')';
						}
					}

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
		if (message_in.find('\x12') == std::string::npos) {
			message_out = message_in;
			return;
		}

		// TOB item link body layout (91 chars, positions within segment after \x12):
		//   [0]     ETAG (1)
		//   [1-5]   item_id (5)
		//   [6-65]  [aug(5) + luck(5)] * 6 = 60
		//   [66]    is_evolving (1)
		//   [67-70] evolve_group (4)
		//   [71-72] evolve_level (2)
		//   [73-77] ornament_icon (5)
		//   [78-82] luck (5)
		//   [83-90] hash (8)
		//   [91+]   text
		constexpr size_t TOB_ITEM_BODY_SIZE = 91;

		std::vector<std::string> segments = Strings::Split(message_in, '\x12');
		for (size_t i = 0; i < segments.size(); ++i) {
			if (!(i & 1)) {
				message_out.append(segments[i]);
				continue;
			}

			if (segments[i].empty() || segments[i][0] - '0' != Links::ETAG_ITEM
				|| segments[i].length() < TOB_ITEM_BODY_SIZE) {
				message_out.push_back('\x12');
				message_out.append(segments[i]);
				message_out.push_back('\x12');
				continue;
			}

			// Strip luck fields to produce RoF2 body:
			//   action_id(1) item_id(5) aug1-6(30) is_evolving(1) evolve_group(4)
			//   evolve_level(2) ornament_icon(5) hash(8) text
			message_out.push_back('\x12');
			message_out += segments[i][0];                         // action_id
			message_out += segments[i].substr(1, 5);               // item_id
			for (int aug = 0; aug < 6; ++aug) {
				message_out += segments[i].substr(6 + aug * 10, 5); // aug, skip luck
			}
			message_out += segments[i].substr(66, 1);              // is_evolving
			message_out += segments[i].substr(67, 4);              // evolve_group
			message_out += segments[i].substr(71, 2);              // evolve_level
			message_out += segments[i].substr(73, 5);              // ornament_icon
			// skip luck [78-82]
			message_out += segments[i].substr(83, 8);              // hash
			message_out += segments[i].substr(91);                 // text
			message_out.push_back('\x12');
		}
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
		// ToB appearance 14 is the guild RANK, not the
		// guild id; the id goes out as OP_GuildMemberGuildID (see ENCODE(OP_SpawnAppearance)).
		case AppearanceType::GuildID:
			return structs::TOBAppearance::None;
		case AppearanceType::GuildRank:
			return structs::TOBAppearance::GuildID;
		case AppearanceType::AFK:
			return structs::TOBAppearance::AFK;
		case AppearanceType::Pet:
			return structs::TOBAppearance::Pet;
		case AppearanceType::Summoned:
			return structs::TOBAppearance::Summoned;
		case AppearanceType::Split:
			return structs::TOBAppearance::Split;
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
		case AppearanceType::AdvancedLooting:
			return structs::TOBAppearance::AdvancedLooting;
		case AppearanceType::MasterLooterCandidate:
			return structs::TOBAppearance::MasterLooterCandidate;
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
		case structs::TOBAppearance::GuildID: // ToB type 14 is the guild rank
			return AppearanceType::GuildRank;
		case structs::TOBAppearance::AFK:
			return AppearanceType::AFK;
		case structs::TOBAppearance::Pet:
			return AppearanceType::Pet;
		case structs::TOBAppearance::Summoned:
			return AppearanceType::Summoned;
		case structs::TOBAppearance::Split:
			return AppearanceType::Split;
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
		case structs::TOBAppearance::AdvancedLooting:
			return AppearanceType::AdvancedLooting;
		case structs::TOBAppearance::MasterLooterCandidate:
			return AppearanceType::MasterLooterCandidate;
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

		else if (EQ::invslot::IsKeyRingSlot(server_slot)) {
			TOBSlot.Type = invtype::KeyRingTypeFromIndex(EQ::invslot::KeyRingFromSlot(server_slot));
			TOBSlot.Slot = EQ::invslot::KeyRingIndexFromSlot(server_slot);
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
		case invtype::typeMountKeyRingItems:
		case invtype::typeIllusionKeyRingItems:
		case invtype::typeFamiliarKeyRingItems:
		case invtype::typeHeroForgeKeyRingItems:
		case invtype::typeTeleportationKeyRingItems:
		case invtype::typeActivatedKeyRingItems: {
			// adding to a ring sends index -1: the server picks the first free index
			const int ring = invtype::KeyRingIndexFromType(tob_slot.Type);
			if (tob_slot.SubIndex == invbag::SLOT_INVALID) {
				if (tob_slot.Slot == invslot::SLOT_INVALID) {
					server_slot = EQ::invslot::KeyRingAppendSlot(ring);
				}
				else if (tob_slot.Slot >= 0 && tob_slot.Slot < EQ::invslot::KEYRING_RING_SIZE) {
					server_slot = EQ::invslot::KeyRingSlot(ring, tob_slot.Slot);
				}
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
		case ItemPacketType::ItemPacketParcel:
			return item::ItemPacketType::ItemPacketParcel;
		case ItemPacketType::ItemPacketOverflow:
			return item::ItemPacketType::ItemPacketOverflow;
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
		// anything the server has no slot for maps past the end so server range checks reject it
		const int invalid = EQ::spells::LONG_BUFFS + EQ::spells::SHORT_BUFFS + EQ::spells::DISC_BUFFS;
		if (index < 0)
			return invalid;
		// we're a disc
		if (index >= spells::CLIENT_LONG_BUFFS + spells::CLIENT_SHORT_BUFFS) {
			const int disc = index - spells::CLIENT_LONG_BUFFS - spells::CLIENT_SHORT_BUFFS;
			return disc < EQ::spells::DISC_BUFFS ? disc + EQ::spells::LONG_BUFFS + EQ::spells::SHORT_BUFFS : invalid;
		}
		// we're a song
		if (index >= spells::CLIENT_LONG_BUFFS) {
			const int song = index - spells::CLIENT_LONG_BUFFS;
			return song < EQ::spells::SHORT_BUFFS ? song + EQ::spells::LONG_BUFFS : invalid;
		}
		// a long slot the server doesn't have (42..61): map past the end so server range checks reject it
		if (index >= spells::LONG_BUFFS)
			return EQ::spells::LONG_BUFFS + EQ::spells::SHORT_BUFFS + EQ::spells::DISC_BUFFS;
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

	// ToB added a seconds argument to the d:h:m lockout strings (e.g. 3519 "%1d:%2h:%3m:%4s ... %5");
	// callers still pass the arguments without seconds, so insert seconds at that index and shift the rest right.
	static const std::unordered_map<uint32_t, size_t> seconds_slot = {
		{3501, 3}, {3504, 4}, {3519, 3}, {3540, 3}, {3561, 5}, {3591, 4}, {3592, 5},
		{8017, 3}, {8946, 3}, {8947, 4}, {8979, 4}, {8987, 4},
	};
	auto it = seconds_slot.find(ResolveID(id));
	if (it != seconds_slot.end()) {
		for (size_t i = args.size() - 1; i > it->second; --i) {
			args[i] = args[i - 1];
		}
		args[it->second] = "0";
	}

	// ToB appended "%1" (experience value) to the plain XP strings; the server never passes it
	switch (id) {
		case GAIN_XP:
		case GAIN_GROUPXP:
		case GAIN_RAIDEXP:
			if (!args[0]) {
				args[0] = "";
			}
			break;
		default:
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
				::TOB::ServerToTOBConvertLinks(new_message, a, false);
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
	// 1 is remove (by spell id), 2 only updates a slot that already has this spell, 3 writes the affect at
	// slot_id (add or update). Use 3 so the client's local affect array matches our slots; the buff window
	// won't send a remove request for a slot it has nothing in.
	affect->buff_fade = fade ? 1 : 3;

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
	// the client multiplies most effect values by this; 0 would zero the buff
	affect->affect.modifier = buff.instrument_mod > 0 ? static_cast<float>(buff.instrument_mod) / 10.f : 1.0f;
	affect->affect.y = static_cast<float>(buff.caston_y);
	affect->affect.x = static_cast<float>(buff.caston_x);
	affect->affect.z = static_cast<float>(buff.caston_z);
	affect->affect.type = 0; // ToB: any non-zero type reverses the buff (negative regen, red max HP/mana)
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

bool BuffComponent::NeedsBuffDefinitionsOnRefresh() const { return true; }

// The client only rewrites a buff button's tooltip while the button is listed in the window, and it unlists
// the button before clearing it, so a removed buff leaves its last "<spell> (<time>)" tooltip on the empty
// button. Sending the slot once more with the spell and 0 time first makes the client blank the tooltip.
std::unique_ptr<EQApplicationPacket> BuffComponent::BuffRemovePrelude(Mob* mob, const Buffs_Struct& buff,
	uint32_t slot) const
{
	if (buff.spellid <= 1 || slot >= mob->GetMaxTotalSlots())
		return nullptr;

	SerializeBuffer buffer;

	buffer.WriteUInt32(mob->GetID());
	buffer.WriteInt32(0);								// tic timer must be 0, the client adds it to the time left
	buffer.WriteUInt8(0);								// single buff
	buffer.WriteUInt16(1);
	buffer.WriteUInt32(ServerToPatchBuffSlot(slot));
	buffer.WriteInt32(buff.spellid);					// still valid, so the button stays listed
	buffer.WriteUInt32(0);								// no time left, so the tooltip is cleared
	buffer.WriteUInt32(buff.hit_number);
	buffer.WriteString(buff.caster_name);
	buffer.WriteUInt8(0);								// self buff window
	buffer.WriteUInt8(0);								// bBuffTimersOnHold

	return std::make_unique<EQApplicationPacket>(OP_RefreshBuffs, std::move(buffer));
}

// 0 = self buff window, 1 = self target window, 2 = pet buff or target window, 4 = group, 5 = PC, 7 = NPC
void BuffComponent::SetRefreshType(std::unique_ptr<EQApplicationPacket>& packet, uint8_t refresh_type) const
{
	if (packet)
		packet->pBuffer[packet->size - 2] = refresh_type;
}

} /*TOB*/