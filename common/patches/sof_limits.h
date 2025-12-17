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


namespace SoF
{
	const int16 IINVALID = -1;
	const int16 INULL = 0;

	namespace inventory {
		inline EQ::versions::ClientVersion GetInventoryRef() { return EQ::versions::ClientVersion::SoF; }

		const bool ConcatenateInvTypeLimbo = true;

		const bool AllowOverLevelEquipment = false;

		const bool AllowEmptyBagInBag = false;
		const bool AllowClickCastFromBag = false;

	} /*inventory*/

	namespace invtype {
		inline EQ::versions::ClientVersion GetInvTypeRef() { return EQ::versions::ClientVersion::SoF; }

		namespace enum_ {
			enum InventoryTypes : int16 {
				typePossessions = INULL,
				typeBank,
				typeSharedBank,
				typeTrade,
				typeWorld,
				typeLimbo,
				typeTribute,
				typeGuildTribute,
				typeMerchant,
				typeCorpse,
				typeBazaar,
				typeInspect,
				typeViewMODPC,
				typeViewMODBank,
				typeViewMODSharedBank,
				typeViewMODLimbo,
				typeAltStorage,
				typeArchived,
				typeOther
			};

		} // namespace enum_
		using namespace enum_;

		const int16 POSSESSIONS_SIZE = 32;
		const int16 BANK_SIZE = 24;
		const int16 SHARED_BANK_SIZE = 2;
		const int16 TRADE_SIZE = 8;
		const int16 WORLD_SIZE = 10;
		const int16 LIMBO_SIZE = 36;
		const int16 TRIBUTE_SIZE = 5;
		const int16 GUILD_TRIBUTE_SIZE = 2;
		const int16 MERCHANT_SIZE = 80;
		const int16 CORPSE_SIZE = POSSESSIONS_SIZE;
		const int16 BAZAAR_SIZE = 80;
		const int16 INSPECT_SIZE = 23;
		const int16 VIEW_MOD_PC_SIZE = POSSESSIONS_SIZE;
		const int16 VIEW_MOD_BANK_SIZE = BANK_SIZE;
		const int16 VIEW_MOD_SHARED_BANK_SIZE = SHARED_BANK_SIZE;
		const int16 VIEW_MOD_LIMBO_SIZE = LIMBO_SIZE;
		const int16 ALT_STORAGE_SIZE = 0;//unknown - "Shroud Bank"
		const int16 ARCHIVED_SIZE = 0;//unknown
		const int16 OTHER_SIZE = 0;//unknown

		const int16 TRADE_NPC_SIZE = 4; // defined by implication

		const int16 TYPE_INVALID = IINVALID;
		const int16 TYPE_BEGIN = typePossessions;
		const int16 TYPE_END = typeOther;
		const int16 TYPE_COUNT = (TYPE_END - TYPE_BEGIN) + 1;

		int16 GetInvTypeSize(int16 inv_type);
		const char* GetInvTypeName(int16 inv_type);

		bool IsInvTypePersistent(int16 inv_type);

	} /*invtype*/

	namespace invslot {
		inline EQ::versions::ClientVersion GetInvSlotRef() { return EQ::versions::ClientVersion::SoF; }

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
				slotCursor
			};

		} // namespace enum_
		using namespace enum_;

		const int16 SLOT_INVALID = IINVALID;
		const int16 SLOT_BEGIN = INULL;

		const int16 SLOT_TRADESKILL_EXPERIMENT_COMBINE = 1000;

		const int16 POSSESSIONS_BEGIN = slotCharm;
		const int16 POSSESSIONS_END = slotCursor;
		const int16 POSSESSIONS_COUNT = (POSSESSIONS_END - POSSESSIONS_BEGIN) + 1;

		const int16 EQUIPMENT_BEGIN = slotCharm;
		const int16 EQUIPMENT_END = slotAmmo;
		const int16 EQUIPMENT_COUNT = (EQUIPMENT_END - EQUIPMENT_BEGIN + 1);

		const int16 GENERAL_BEGIN = slotGeneral1;
		const int16 GENERAL_END = slotGeneral8;
		const int16 GENERAL_COUNT = (GENERAL_END - GENERAL_BEGIN + 1);

		const int16 BONUS_BEGIN = invslot::slotCharm;
		const int16 BONUS_STAT_END = invslot::slotPowerSource;
		const int16 BONUS_SKILL_END = invslot::slotAmmo;

		const int16 BANK_BEGIN = 2000;
		const int16 BANK_END = (BANK_BEGIN + invtype::BANK_SIZE) - 1;

		const int16 SHARED_BANK_BEGIN = 2500;
		const int16 SHARED_BANK_END = (SHARED_BANK_BEGIN + invtype::SHARED_BANK_SIZE) - 1;

		const int16 TRADE_BEGIN = 3000;
		const int16 TRADE_END = (TRADE_BEGIN + invtype::TRADE_SIZE) - 1;

		const int16 TRADE_NPC_END = (TRADE_BEGIN + invtype::TRADE_NPC_SIZE) - 1; // defined by implication

		const int16 WORLD_BEGIN = 4000;
		const int16 WORLD_END = (WORLD_BEGIN + invtype::WORLD_SIZE) - 1;

		const int16 TRIBUTE_BEGIN = 400;
		const int16 TRIBUTE_END = (TRIBUTE_BEGIN + invtype::TRIBUTE_SIZE) - 1;

		const int16 GUILD_TRIBUTE_BEGIN = 450;
		const int16 GUILD_TRIBUTE_END = (GUILD_TRIBUTE_BEGIN + invtype::GUILD_TRIBUTE_SIZE) - 1;

		const int16 CORPSE_BEGIN = invslot::slotGeneral1;
		const int16 CORPSE_END = invslot::slotGeneral1 + invslot::slotCursor;

		const uint64 EQUIPMENT_BITMASK = 0x00000000007FFFFF;
		const uint64 GENERAL_BITMASK = 0x000000007F800000;
		const uint64 CURSOR_BITMASK = 0x0000000200000000;
		const uint64 POSSESSIONS_BITMASK = (EQUIPMENT_BITMASK | GENERAL_BITMASK | CURSOR_BITMASK); // based on 34-slot count (RoF+)
		const uint64 CORPSE_BITMASK = (GENERAL_BITMASK | CURSOR_BITMASK | (EQUIPMENT_BITMASK << 34)); // based on 34-slot count (RoF+)


		const char* GetInvPossessionsSlotName(int16 inv_slot);
		const char* GetInvCorpseSlotName(int16 inv_slot);
		const char* GetInvSlotName(int16 inv_type, int16 inv_slot);

	} /*invslot*/

	namespace invbag {
		inline EQ::versions::ClientVersion GetInvBagRef() { return EQ::versions::ClientVersion::SoF; }

		const int16 SLOT_INVALID = IINVALID;
		const int16 SLOT_BEGIN = INULL;
		const int16 SLOT_END = 9;
		const int16 SLOT_COUNT = 10;

		const int16 GENERAL_BAGS_BEGIN = 262;
		const int16 GENERAL_BAGS_COUNT = invslot::GENERAL_COUNT * SLOT_COUNT;
		const int16 GENERAL_BAGS_END = (GENERAL_BAGS_BEGIN + GENERAL_BAGS_COUNT) - 1;

		const int16 CURSOR_BAG_BEGIN = 342;
		const int16 CURSOR_BAG_COUNT = SLOT_COUNT;
		const int16 CURSOR_BAG_END = (CURSOR_BAG_BEGIN + CURSOR_BAG_COUNT) - 1;

		const int16 BANK_BAGS_BEGIN = 2032;
		const int16 BANK_BAGS_COUNT = (invtype::BANK_SIZE * SLOT_COUNT);
		const int16 BANK_BAGS_END = (BANK_BAGS_BEGIN + BANK_BAGS_COUNT) - 1;

		const int16 SHARED_BANK_BAGS_BEGIN = 2532;
		const int16 SHARED_BANK_BAGS_COUNT = invtype::SHARED_BANK_SIZE * SLOT_COUNT;
		const int16 SHARED_BANK_BAGS_END = (SHARED_BANK_BAGS_BEGIN + SHARED_BANK_BAGS_COUNT) - 1;

		const int16 TRADE_BAGS_BEGIN = 3031;
		const int16 TRADE_BAGS_COUNT = invtype::TRADE_SIZE * SLOT_COUNT;
		const int16 TRADE_BAGS_END = (TRADE_BAGS_BEGIN + TRADE_BAGS_COUNT) - 1;

		const char* GetInvBagIndexName(int16 bag_index);

	} /*invbag*/

	namespace invaug {
		inline EQ::versions::ClientVersion GetInvAugRef() { return EQ::versions::ClientVersion::SoF; }

		const int16 SOCKET_INVALID = IINVALID;
		const int16 SOCKET_BEGIN = INULL;
		const int16 SOCKET_END = 4;
		const int16 SOCKET_COUNT = 5;

		const char* GetInvAugIndexName(int16 aug_index);

	} /*invaug*/

	namespace item {
		inline EQ::versions::ClientVersion GetItemRef() { return EQ::versions::ClientVersion::SoF; }

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
			ItemPacketCharmUpdate = 110
		};

	} /*item*/

	namespace profile {
		inline EQ::versions::ClientVersion GetProfileRef() { return EQ::versions::ClientVersion::SoF; }

		const int16 BANDOLIERS_SIZE = 20;		// number of bandolier instances
		const int16 BANDOLIER_ITEM_COUNT = 4;	// number of equipment slots in bandolier instance

		const int16 POTION_BELT_SIZE = 5;

		const int16 SKILL_ARRAY_SIZE = 100;

	} /*profile*/

	namespace constants {
		inline EQ::versions::ClientVersion GetConstantsRef() { return EQ::versions::ClientVersion::SoF; }

		const EQ::expansions::Expansion EXPANSION = EQ::expansions::Expansion::SoF;
		const uint32 EXPANSION_BIT = EQ::expansions::bitSoF;
		const uint32 EXPANSIONS_MASK = EQ::expansions::maskSoF;

		const size_t CHARACTER_CREATION_LIMIT = 12;

		const size_t SAY_LINK_BODY_SIZE = 50;

	} /*constants*/

	namespace behavior {
		inline EQ::versions::ClientVersion GetBehaviorRef() { return EQ::versions::ClientVersion::SoF; }

		const bool CoinHasWeight = true;

	} /*behavior*/

	namespace skills {
		inline EQ::versions::ClientVersion GetSkillsRef() { return EQ::versions::ClientVersion::SoF; }

		const size_t LastUsableSkill = EQ::skills::SkillTripleAttack;

	} /*skills*/

	namespace spells {
		inline EQ::versions::ClientVersion GetSkillsRef() { return EQ::versions::ClientVersion::SoF; }
		
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
			MaxGems = 9,
			Ability = 9,
			Item = 10,
			Discipline = 10,
			PotionBelt = 11,
			AltAbility = 0xFF
		};

		const int SPELL_ID_MAX = 15999;
		const int SPELLBOOK_SIZE = 480;
		// Be careful not to confuse these two..SoF disc release has a special requirement...
		// - The number of available spell gems HAS NOT increased from 9 at this point
		// - The profile allocation HAS increased to 10 at this point
		const int SPELL_GEM_COUNT = static_cast<uint32>(CastingSlot::MaxGems);
		const int SPELL_GEM_PROFILE_SIZE = 10; // special case declaration
		
		const int LONG_BUFFS = 25;
		const int SHORT_BUFFS = 15;
		const int DISC_BUFFS = 1;
		const int TOTAL_BUFFS = LONG_BUFFS + SHORT_BUFFS + DISC_BUFFS;
		const int NPC_BUFFS = 60;
		const int PET_BUFFS = 30;
		const int MERC_BUFFS = 0;

	} /*spells*/

} /*SoF*/
