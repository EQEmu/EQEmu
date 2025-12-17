/*	EQEMu: Everquest Server Emulator

	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#pragma once

#include "common/emu_versions.h"
#include "common/skills.h"
#include "common/types.h"


namespace RoF2
{
	const int16 IINVALID = -1;
	const int16 INULL = 0;

	namespace inventory {
		inline EQ::versions::ClientVersion GetInventoryRef() { return EQ::versions::ClientVersion::RoF2; }

		const bool ConcatenateInvTypeLimbo = false;

		const bool AllowOverLevelEquipment = true;

		const bool AllowEmptyBagInBag    = true;
		const bool AllowClickCastFromBag = true;

	} /*inventory*/

	namespace invtype {
		inline EQ::versions::ClientVersion GetInvTypeRef() { return EQ::versions::ClientVersion::RoF2; }

		namespace enum_ {
			enum InventoryTypes : int16 {
				typePossessions = INULL,
				typeBank,
				typeSharedBank,
				typeTrade,
				typeWorld,
				typeLimbo,
				typeTribute,
				typeTrophyTribute,
				typeGuildTribute,
				typeMerchant,
				typeDeleted,
				typeCorpse,
				typeBazaar,
				typeInspect,
				typeRealEstate,
				typeViewMODPC,
				typeViewMODBank,
				typeViewMODSharedBank,
				typeViewMODLimbo,
				typeAltStorage,
				typeArchived,
				typeMail,
				typeGuildTrophyTribute,
				typeKrono,
				typeOther
			};

		} // namespace enum_
		using namespace enum_;

		const int16 POSSESSIONS_SIZE          = 34;
		const int16 BANK_SIZE                 = 24;
		const int16 SHARED_BANK_SIZE          = 2;
		const int16 TRADE_SIZE                = 8;
		const int16 WORLD_SIZE                = 10;
		const int16 LIMBO_SIZE                = 36;
		const int16 TRIBUTE_SIZE              = 5;
		const int16 TROPHY_TRIBUTE_SIZE       = 0;//unknown
		const int16 GUILD_TRIBUTE_SIZE        = 2;//unverified
		const int16 MERCHANT_SIZE             = 500;
		const int16 DELETED_SIZE              = 0;//unknown - "Recovery Tab"
		const int16 CORPSE_SIZE               = POSSESSIONS_SIZE;
		const int16 BAZAAR_SIZE               = 200;
		const int16 INSPECT_SIZE              = 23;
		const int16 REAL_ESTATE_SIZE          = 0;//unknown
		const int16 VIEW_MOD_PC_SIZE          = POSSESSIONS_SIZE;
		const int16 VIEW_MOD_BANK_SIZE        = BANK_SIZE;
		const int16 VIEW_MOD_SHARED_BANK_SIZE = SHARED_BANK_SIZE;
		const int16 VIEW_MOD_LIMBO_SIZE       = LIMBO_SIZE;
		const int16 ALT_STORAGE_SIZE          = 0;//unknown - "Shroud Bank"
		const int16 ARCHIVED_SIZE             = 0;//unknown
		const int16 MAIL_SIZE                 = 0;//unknown
		const int16 GUILD_TROPHY_TRIBUTE_SIZE = 0;//unknown
		const int16 KRONO_SIZE                = 0;//unknown
		const int16 GUILD_BANK_MAIN_SIZE      = 200;
		const int16 GUILD_BANK_DEPOSIT_SIZE   = 40;
		const int16 OTHER_SIZE                = 0;//unknown

		const int16 TRADE_NPC_SIZE = 4; // defined by implication

		const int16 TYPE_INVALID = IINVALID;
		const int16 TYPE_BEGIN   = typePossessions;
		const int16 TYPE_END     = typeOther;
		const int16 TYPE_COUNT   = (TYPE_END - TYPE_BEGIN) + 1;

		int16 GetInvTypeSize(int16 inv_type);
		const char* GetInvTypeName(int16 inv_type);

		bool IsInvTypePersistent(int16 inv_type);

	} /*invtype*/

	namespace invslot {
		inline EQ::versions::ClientVersion GetInvSlotRef() { return EQ::versions::ClientVersion::RoF2; }

		namespace enum_ {
			enum InventorySlots : int16 {
				slotCharm = INULL,
				slotEar1,
				slotHead,
				slotFace,
				slotEar2,
				slotNeck,
				slotShoulders,
				slotArms,
				slotBack,
				slotWrist1,
				slotWrist2,
				slotRange,
				slotHands,
				slotPrimary,
				slotSecondary,
				slotFinger1,
				slotFinger2,
				slotChest,
				slotLegs,
				slotFeet,
				slotWaist,
				slotPowerSource,
				slotAmmo,
				slotGeneral1,
				slotGeneral2,
				slotGeneral3,
				slotGeneral4,
				slotGeneral5,
				slotGeneral6,
				slotGeneral7,
				slotGeneral8,
				slotGeneral9,
				slotGeneral10,
				slotCursor
			};

			constexpr int16 format_as(InventorySlots slot) { return static_cast<int16>(slot); }
		} // namespace enum_
		using namespace enum_;

		const int16 SLOT_TRADESKILL_EXPERIMENT_COMBINE = 1000;
		const int16 SLOT_INVALID                       = IINVALID;
		const int16 SLOT_BEGIN                         = INULL;

		const int16 BANK_BEGIN = 2000;
		const int16 BANK_END   = (BANK_BEGIN + invtype::BANK_SIZE) - 1;

		const int16 SHARED_BANK_BEGIN = 2500;
		const int16 SHARED_BANK_END   = (SHARED_BANK_BEGIN + invtype::SHARED_BANK_SIZE) - 1;

		const int16 TRADE_BEGIN = 3000;
		const int16 TRADE_END   = (TRADE_BEGIN + invtype::TRADE_SIZE) - 1;

		const int16 TRADE_NPC_END = (TRADE_BEGIN + invtype::TRADE_NPC_SIZE) - 1; // defined by implication

		const int16 WORLD_BEGIN = 4000;
		const int16 WORLD_END   = (WORLD_BEGIN + invtype::WORLD_SIZE) - 1;

		const int16 TRIBUTE_BEGIN = 400;
		const int16 TRIBUTE_END   = (TRIBUTE_BEGIN + invtype::TRIBUTE_SIZE) - 1;

		const int16 GUILD_TRIBUTE_BEGIN = 450;
		const int16 GUILD_TRIBUTE_END   = (GUILD_TRIBUTE_BEGIN + invtype::GUILD_TRIBUTE_SIZE) - 1;

		const int16 POSSESSIONS_BEGIN = slotCharm;
		const int16 POSSESSIONS_END   = slotCursor;
		const int16 POSSESSIONS_COUNT = (POSSESSIONS_END - POSSESSIONS_BEGIN) + 1;

		const int16 EQUIPMENT_BEGIN = slotCharm;
		const int16 EQUIPMENT_END   = slotAmmo;
		const int16 EQUIPMENT_COUNT = (EQUIPMENT_END - EQUIPMENT_BEGIN) + 1;

		const int16 GENERAL_BEGIN = slotGeneral1;
		const int16 GENERAL_END   = slotGeneral10;
		const int16 GENERAL_COUNT = (GENERAL_END - GENERAL_BEGIN) + 1;

		const int16 BONUS_BEGIN     = invslot::slotCharm;
		const int16 BONUS_STAT_END  = invslot::slotPowerSource;
		const int16 BONUS_SKILL_END = invslot::slotAmmo;

		const int16 CORPSE_BEGIN = invslot::slotGeneral1;
		const int16 CORPSE_END   = invslot::slotGeneral1 + invslot::slotCursor;

		const uint64 EQUIPMENT_BITMASK   = 0x00000000007FFFFF;
		const uint64 GENERAL_BITMASK     = 0x00000001FF800000;
		const uint64 CURSOR_BITMASK      = 0x0000000200000000;
		const uint64 POSSESSIONS_BITMASK = (EQUIPMENT_BITMASK | GENERAL_BITMASK | CURSOR_BITMASK); // based on 34-slot count (RoF+)
		const uint64 CORPSE_BITMASK      = (GENERAL_BITMASK | CURSOR_BITMASK | (EQUIPMENT_BITMASK << 34)); // based on 34-slot count (RoF+)


		const char* GetInvPossessionsSlotName(int16 inv_slot);
		const char* GetInvSlotName(int16 inv_type, int16 inv_slot);

	} /*invslot*/

	namespace invbag {
		inline EQ::versions::ClientVersion GetInvBagRef() { return EQ::versions::ClientVersion::RoF2; }

		const int16 SLOT_TRADESKILL_EXPERIMENT_COMBINE = 1000;
		const int16 SLOT_INVALID                       = IINVALID;
		const int16 SLOT_BEGIN                         = INULL;
		const int16 SLOT_COUNT                         = 200;
		const int16 SLOT_END                           = SLOT_COUNT - 1;

		const int16 GENERAL_BAGS_BEGIN = 251;

		const int16 CURSOR_BAG_BEGIN = 351;

		const int16 BANK_BAGS_BEGIN = 2031;

		const int16 SHARED_BANK_BAGS_BEGIN = 2531;

		const int16 TRADE_BAGS_BEGIN = 3031;

		const char* GetInvBagIndexName(int16 bag_index);

	} /*invbag*/

	namespace invaug {
		inline EQ::versions::ClientVersion GetInvAugRef() { return EQ::versions::ClientVersion::RoF2; }

		const int16 SOCKET_INVALID = IINVALID;
		const int16 SOCKET_BEGIN   = INULL;
		const int16 SOCKET_END     = 5;
		const int16 SOCKET_COUNT   = 6;

		const char* GetInvAugIndexName(int16 aug_index);

	} /*invaug*/

	namespace item {
		inline EQ::versions::ClientVersion GetItemRef() { return EQ::versions::ClientVersion::RoF2; }

		//enum Unknown : int { // looks like item class..but, RoF has it too - nothing in UF-
		//	Unknown1 = 0,
		//	Unknown2 = 1,
		//	Unknown3 = 2,
		//	Unknown4 = 5 // krono?
		//};

		enum ItemPacketType : int {
			ItemPacketMerchant = 100,
			ItemPacketTradeView = 101,
			ItemPacketLoot = 102,
			ItemPacketTrade = 103,
			ItemPacketCharInventory = 105,
			ItemPacketLimbo = 106,
			ItemPacketWorldContainer = 107,
			ItemPacketTributeItem = 108,
			ItemPacketGuildTribute = 109,
			ItemPacket10 = 110,
			ItemPacket11 = 111,
			ItemPacket12 = 112,
			ItemPacketRecovery = 113,
			ItemPacket14 = 115 // Parcel? adds to merchant window too
		};

	} /*item*/

	namespace profile {
		inline EQ::versions::ClientVersion GetProfileRef() { return EQ::versions::ClientVersion::RoF2; }

		const int16 BANDOLIERS_SIZE = 20;		// number of bandolier instances
		const int16 BANDOLIER_ITEM_COUNT = 4;	// number of equipment slots in bandolier instance

		const int16 POTION_BELT_SIZE = 5;

		const int16 SKILL_ARRAY_SIZE = 100;

	} /*profile*/

	namespace constants {
		inline EQ::versions::ClientVersion GetConstantsRef() { return EQ::versions::ClientVersion::RoF2; }

		const EQ::expansions::Expansion EXPANSION = EQ::expansions::Expansion::RoF;
		const uint32 EXPANSION_BIT = EQ::expansions::bitRoF;
		const uint32 EXPANSIONS_MASK = EQ::expansions::maskRoF;

		const size_t CHARACTER_CREATION_LIMIT = 12;

		const size_t SAY_LINK_BODY_SIZE = 56;
		const uint32 MAX_GUILD_ID       = 50000;
		const uint32 MAX_BAZAAR_TRADERS = 600;

	} /*constants*/

	namespace behavior {
		inline EQ::versions::ClientVersion GetBehaviorRef() { return EQ::versions::ClientVersion::RoF2; }

		const bool CoinHasWeight = false;

	} /*behavior*/

	namespace skills {
		inline EQ::versions::ClientVersion GetSkillsRef() { return EQ::versions::ClientVersion::RoF2; }

		const size_t LastUsableSkill = EQ::skills::Skill2HPiercing;

	} /*skills*/

	namespace spells {
		inline EQ::versions::ClientVersion GetSkillsRef() { return EQ::versions::ClientVersion::RoF2; }

		enum class CastingSlot : uint32 {
			Gem1 = 0,
			Gem2 = 1,
			Gem3 = 2,
			Gem4 = 3,
			Gem5 = 4,
			Gem6 = 5,
			Gem7 = 6,
			Gem8 = 7,
			Gem9 = 8,
			Gem10 = 9,
			Gem11 = 10,
			Gem12 = 11,
			MaxGems = 16, // fallacy..only 12 slot are useable...
			Item = 12,
			Discipline = 13,
			AltAbility = 0xFF
		};

		const int SPELL_ID_MAX = 45000;
		const int SPELLBOOK_SIZE = 720;
		const int SPELL_GEM_COUNT = static_cast<uint32>(CastingSlot::MaxGems);

		const int LONG_BUFFS = 42;
		const int SHORT_BUFFS = 20;
		const int DISC_BUFFS = 1;
		const int TOTAL_BUFFS = LONG_BUFFS + SHORT_BUFFS + DISC_BUFFS;
		const int NPC_BUFFS = 97;
		const int PET_BUFFS = NPC_BUFFS;
		const int MERC_BUFFS = LONG_BUFFS;

	} /*spells*/

} /*RoF2*/
