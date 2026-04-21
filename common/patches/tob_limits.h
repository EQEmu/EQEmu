#ifndef COMMON_LAURION_LIMITS_H
#define COMMON_LAURION_LIMITS_H

#include "../types.h"
#include "../emu_versions.h"
#include "../skills.h"

namespace TOB
{
	const int16 IINVALID = -1;
	const int16 INULL = 0;

	namespace inventory {
		inline EQ::versions::ClientVersion GetInventoryRef() { return EQ::versions::ClientVersion::TOB; }

		const bool ConcatenateInvTypeLimbo = false;

		const bool AllowOverLevelEquipment = true;

		const bool AllowEmptyBagInBag = true;
		const bool AllowClickCastFromBag = true;

	} /*inventory*/

	namespace invtype {
		inline EQ::versions::ClientVersion GetInvTypeRef() { return EQ::versions::ClientVersion::TOB; }

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
				typeOther,
				typeMercenaryItems,
				typeViewModMercenaryItems,
				typeMountKeyRingItems,
				typeViewModMountKeyRingItems,
				typeIllusionKeyRingItems,
				typeViewModIllusionKeyRingItems,
				typeFamiliarKeyRingItems,
				typeViewModFamiliarKeyRingItems,
				typeHeroForgeKeyRingItems,
				typeViewModHeroForgeKeyRingItems,
				typeTeleportationKeyRingItems,
				typeViewModTeleportationKeyRingItems,
				typeOverflow,
				typeDragonHoard,
				typeTradeskillDepot,
				typeGuildTradeskillDepot
			};

		} // namespace enum_
		using namespace enum_;

		const int16 POSSESSIONS_SIZE = 34;
		const int16 BANK_SIZE = 24;
		const int16 SHARED_BANK_SIZE = 2;
		const int16 TRADE_SIZE = 8;
		const int16 WORLD_SIZE = 10;
		const int16 LIMBO_SIZE = 36;
		const int16 TRIBUTE_SIZE = 5;
		const int16 TROPHY_TRIBUTE_SIZE = 0;//unknown
		const int16 GUILD_TRIBUTE_SIZE = 2;//unverified
		const int16 MERCHANT_SIZE = 200;
		const int16 DELETED_SIZE = 0;//unknown - "Recovery Tab"
		const int16 CORPSE_SIZE = POSSESSIONS_SIZE;
		const int16 BAZAAR_SIZE = 200;
		const int16 INSPECT_SIZE = 23;
		const int16 REAL_ESTATE_SIZE = 0;//unknown
		const int16 VIEW_MOD_PC_SIZE = POSSESSIONS_SIZE;
		const int16 VIEW_MOD_BANK_SIZE = BANK_SIZE;
		const int16 VIEW_MOD_SHARED_BANK_SIZE = SHARED_BANK_SIZE;
		const int16 VIEW_MOD_LIMBO_SIZE = LIMBO_SIZE;
		const int16 ALT_STORAGE_SIZE = 0;//unknown - "Shroud Bank"
		const int16 ARCHIVED_SIZE = 0;//unknown
		const int16 MAIL_SIZE = 0;//unknown
		const int16 GUILD_TROPHY_TRIBUTE_SIZE = 0;//unknown
		const int16 KRONO_SIZE = 0;//unknown
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
		inline EQ::versions::ClientVersion GetInvSlotRef() { return EQ::versions::ClientVersion::TOB; }

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
				slotGeneral11,
				slotGeneral12,
				slotCursor
			};

			constexpr int16 format_as(InventorySlots slot) { return static_cast<int16>(slot); }
		} // namespace enum_
		using namespace enum_;

		const int16 SLOT_INVALID = IINVALID;
		const int16 SLOT_BEGIN = INULL;

		const int16 POSSESSIONS_BEGIN = slotCharm;
		const int16 POSSESSIONS_END = slotCursor;
		const int16 POSSESSIONS_COUNT = (POSSESSIONS_END - POSSESSIONS_BEGIN) + 1;

		const int16 EQUIPMENT_BEGIN = slotCharm;
		const int16 EQUIPMENT_END = slotAmmo;
		const int16 EQUIPMENT_COUNT = (EQUIPMENT_END - EQUIPMENT_BEGIN) + 1;

		//We support more if enabled but for now lets leave it at the 10 slots
		const int16 GENERAL_BEGIN = slotGeneral1;
		const int16 GENERAL_END = slotGeneral10;
		const int16 GENERAL_COUNT = (GENERAL_END - GENERAL_BEGIN) + 1;

		const int16 BONUS_BEGIN = invslot::slotCharm;
		const int16 BONUS_STAT_END = invslot::slotPowerSource;
		const int16 BONUS_SKILL_END = invslot::slotAmmo;

		const int16 CORPSE_BEGIN = invslot::slotGeneral1;
		const int16 CORPSE_END = invslot::slotGeneral1 + invslot::slotCursor;

		const uint64 EQUIPMENT_BITMASK = 0x00000000007FFFFF;
		const uint64 GENERAL_BITMASK = 0x00000007FF800000;
		const uint64 CURSOR_BITMASK = 0x0000000800000000;
		const uint64 POSSESSIONS_BITMASK = (EQUIPMENT_BITMASK | GENERAL_BITMASK | CURSOR_BITMASK); // based on 36-slot count (TOB+)
		const uint64 CORPSE_BITMASK = (GENERAL_BITMASK | CURSOR_BITMASK | (EQUIPMENT_BITMASK << 36)); // based on 36-slot count (TOB+)


		const char* GetInvPossessionsSlotName(int16 inv_slot);
		const char* GetInvSlotName(int16 inv_type, int16 inv_slot);

	} /*invslot*/

	namespace invbag {
		inline EQ::versions::ClientVersion GetInvBagRef() { return EQ::versions::ClientVersion::TOB; }

		const int16 SLOT_INVALID = IINVALID;
		const int16 SLOT_BEGIN = INULL;
		const int16 SLOT_END = 199;
		const int16 SLOT_COUNT = 200; // server Size will be 200..unsure what actual client is (test)

		const char* GetInvBagIndexName(int16 bag_index);

	} /*invbag*/

	namespace invaug {
		inline EQ::versions::ClientVersion GetInvAugRef() { return EQ::versions::ClientVersion::TOB; }

		const int16 SOCKET_INVALID = IINVALID;
		const int16 SOCKET_BEGIN = INULL;
		const int16 SOCKET_END = 5;
		const int16 SOCKET_COUNT = 6;

		const char* GetInvAugIndexName(int16 aug_index);

	} /*invaug*/

	namespace item {
		inline EQ::versions::ClientVersion GetItemRef() { return EQ::versions::ClientVersion::TOB; }

		//enum Unknown : int { // looks like item class..but, RoF has it too - nothing in UF-
		//	Unknown1 = 0,
		//	Unknown2 = 1,
		//	Unknown3 = 2,
		//	Unknown4 = 5 // krono?
		//};

		enum ItemPacketType : int {
			ItemPacketMerchant = 0x64,
			ItemPacketTradeView = 0x65,
			ItemPacketLoot = 0x66,
			ItemPacketTrade = 0x67,
			//looks like they added something at 0x68 that didn't exist before and shifted everything after it up by 1
			ItemPacketUnknown068 = 0x68, //Not sure but it seems to deal with the cursor somehow.
			ItemPacketCharInventory = 0x6A, //Rof 0x69 -> Larion 0x6a (requires translation)
			ItemPacketLimbo = 0x6B, //0x6A -> 0x6B
			ItemPacketWorldContainer = 0x6C,
			ItemPacketTributeItem = 0x6D,
			ItemPacketGuildTribute = 0x6E,
			ItemPacketCharmUpdate = 0x6f,
			ItemPacketRecovery = 0x72,
			ItemPacketParcel = 0x74,
			ItemPacketUnknown075 = 0x75, //Not sure but uses a lot of the same logic as the trade and char inventory types
			ItemPacketOverflow = 0x76,
			ItemPacketDragonHoard = 0x77,
			ItemPacketTradeskill = 0x78,
			ItemPacketTradeskillDepot = 0x79,
			ItemPacketInvalid = 0xFF
		};

	} /*item*/

	namespace profile {
		inline EQ::versions::ClientVersion GetProfileRef() { return EQ::versions::ClientVersion::TOB; }

		const int16 BANDOLIERS_SIZE = 20;		// number of bandolier instances
		const int16 BANDOLIER_ITEM_COUNT = 4;	// number of equipment slots in bandolier instance

		const int16 POTION_BELT_SIZE = 5;

		const int16 SKILL_ARRAY_SIZE = 100;

	} /*profile*/

	namespace constants {
		inline EQ::versions::ClientVersion GetConstantsRef() { return EQ::versions::ClientVersion::TOB; }

		const EQ::expansions::Expansion EXPANSION = EQ::expansions::Expansion::LS;
		const uint32 EXPANSION_BIT = EQ::expansions::bitLS;
		const uint32 EXPANSIONS_MASK = EQ::expansions::maskLS;

		const size_t CHARACTER_CREATION_LIMIT = 12;

		const size_t SAY_LINK_BODY_SIZE = 56;
		const uint32 MAX_GUILD_ID = 50000;
		const uint32 MAX_BAZAAR_TRADERS = 600;

	} /*constants*/

	namespace behavior {
		inline EQ::versions::ClientVersion GetBehaviorRef() { return EQ::versions::ClientVersion::TOB; }

		const bool CoinHasWeight = false;

	} /*behavior*/

	namespace skills {
		inline EQ::versions::ClientVersion GetSkillsRef() { return EQ::versions::ClientVersion::TOB; }

		const size_t LastUsableSkill = EQ::skills::Skill2HPiercing;

	} /*skills*/

	namespace spells {
		inline EQ::versions::ClientVersion GetSkillsRef() { return EQ::versions::ClientVersion::TOB; }

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
			MaxGems = 18, // fallacy..only 12 slot are useable...
			Item = 12,
			Discipline = 13,
			AltAbility = 0xFF
		};

		const int SPELL_ID_MAX = 71999;
		const int SPELLBOOK_SIZE = 1120;
		const int SPELL_GEM_COUNT = static_cast<uint32>(CastingSlot::MaxGems);
		const int SPELL_GEM_RECAST_TIMER = 15;

		const int LONG_BUFFS = 42;
		const int SHORT_BUFFS = 30;
		const int DISC_BUFFS = 1;
		const int TOTAL_BUFFS = LONG_BUFFS + SHORT_BUFFS + DISC_BUFFS;
		const int NPC_BUFFS = 400;
		const int PET_BUFFS = NPC_BUFFS;
		const int MERC_BUFFS = LONG_BUFFS;

	} /*spells*/
	
}; /* TOB */

#endif /*COMMON_LAURION_LIMITS_H*/
