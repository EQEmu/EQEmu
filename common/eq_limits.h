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
#include "common/eq_constants.h"
#include "common/patches/rof_limits.h"
#include "common/patches/rof2_limits.h"
#include "common/patches/sod_limits.h"
#include "common/patches/sof_limits.h"
#include "common/patches/titanium_limits.h"
#include "common/patches/uf_limits.h"
#include "common/types.h"


namespace EQ
{
	void InitializeDynamicLookups();

	namespace constants {
		struct LookupEntry {
			EQ::expansions::Expansion Expansion;
			uint32 ExpansionBit;
			uint32 ExpansionsMask;
			int16 CharacterCreationLimit;
			size_t SayLinkBodySize;
			uint32 BazaarTraderLimit;
			
			LookupEntry(const LookupEntry *lookup_entry) { }
			LookupEntry(
				EQ::expansions::Expansion Expansion,
				uint32 ExpansionBit,
				uint32 ExpansionsMask,
				int16 CharacterCreationLimit,
				size_t SayLinkBodySize,
				uint32 BazaarTraderLimit
			) :
				Expansion(Expansion),
				ExpansionBit(ExpansionBit),
				ExpansionsMask(ExpansionsMask),
				CharacterCreationLimit(CharacterCreationLimit),
				SayLinkBodySize(SayLinkBodySize),
				BazaarTraderLimit(BazaarTraderLimit)
			{ }
		};

		void InitializeDynamicLookups();

		const LookupEntry* DynamicLookup(versions::ClientVersion client_version, bool gm_flag);
		const LookupEntry* DynamicNonGMLookup(versions::ClientVersion client_version);
		const LookupEntry* DynamicGMLookup(versions::ClientVersion client_version);
		const LookupEntry* StaticLookup(versions::ClientVersion client_version);

	} /*constants*/
	
	namespace inventory {
		struct LookupEntry {
			// note: 'PossessionsBitmask' needs to be attuned to the client version with the highest number
			// of possessions slots and 'InventoryTypeSize[typePossessions]' should reflect the same count
			// with translators adjusting for valid slot indices. Server-side validations will be performed
			// against 'PossessionsBitmask' (note: the same applies to CorpseBitmask..size is not dependent)

			struct InventoryTypeSize_Struct { // should reflect count and naming conventions referenced in emu_constants.h
				int16 Possessions,	Bank,				SharedBank;
				int16 Trade,		World,				Limbo;
				int16 Tribute,		TrophyTribute,		GuildTribute;
				int16 Merchant,		Deleted,			Corpse;
				int16 Bazaar,		Inspect,			RealEstate;
				int16 ViewMODPC,	ViewMODBank,		ViewMODSharedBank;
				int16 ViewMODLimbo,	AltStorage,			Archived;
				int16 Mail,			GuildTrophyTribute,	Krono;
				int16 GuildBankMain,GuildBankDeposit,   Other;

				InventoryTypeSize_Struct(
					int16 Possessions,	int16 Bank,					int16 SharedBank,
					int16 Trade,		int16 World,				int16 Limbo,
					int16 Tribute,		int16 TrophyTribute,		int16 GuildTribute,
					int16 Merchant,		int16 Deleted,				int16 Corpse,
					int16 Bazaar,		int16 Inspect,				int16 RealEstate,
					int16 ViewMODPC,	int16 ViewMODBank,			int16 ViewMODSharedBank,
					int16 ViewMODLimbo,	int16 AltStorage,			int16 Archived,
					int16 Mail,			int16 GuildTrophyTribute,	int16 Krono,
					int16 GuildBankMain,int16 GuildBankDeposit,     int16 Other
				) :
					Possessions(Possessions),	Bank(Bank),								SharedBank(SharedBank),
					Trade(Trade),				World(World),							Limbo(Limbo),
					Tribute(Tribute),			TrophyTribute(TrophyTribute),			GuildTribute(GuildTribute),
					Merchant(Merchant),			Deleted(Deleted),						Corpse(Corpse),
					Bazaar(Bazaar),				Inspect(Inspect),						RealEstate(RealEstate),
					ViewMODPC(ViewMODPC),		ViewMODBank(ViewMODBank),				ViewMODSharedBank(ViewMODSharedBank),
					ViewMODLimbo(ViewMODLimbo),	AltStorage(AltStorage),					Archived(Archived),
					Mail(Mail),					GuildTrophyTribute(GuildTrophyTribute),	Krono(Krono),
					GuildBankMain(GuildBankMain), GuildBankDeposit(GuildBankDeposit),   Other(Other)
				{ }
			};

			union {
				InventoryTypeSize_Struct InventoryTypeSize;
				int16 InventoryTypeSizeArray[25]; // should reflect EQ::invtype::TYPE_COUNT referenced in emu_constants.h
			};

			uint64 EquipmentBitmask;
			uint64 GeneralBitmask;
			uint64 CursorBitmask;
			uint64 PossessionsBitmask;
			uint64 CorpseBitmask;
			int16 BagSlotCount;
			int16 AugSocketCount;

			bool AllowEmptyBagInBag;
			bool AllowClickCastFromBag;
			bool ConcatenateInvTypeLimbo;
			bool AllowOverLevelEquipment;

			LookupEntry(const LookupEntry *lookup_entry) { }
			LookupEntry(
				const InventoryTypeSize_Struct& InventoryTypeSize,
				uint64 EquipmentBitmask,
				uint64 GeneralBitmask,
				uint64 CursorBitmask,
				uint64 PossessionsBitmask,
				uint64 CorpseBitmask,
				int16 BagSlotCount,
				int16 AugSocketCount,
				bool AllowEmptyBagInBag,
				bool AllowClickCastFromBag,
				bool ConcatenateInvTypeLimbo,
				bool AllowOverLevelEquipment
			) :
				InventoryTypeSize(InventoryTypeSize),
				EquipmentBitmask(EquipmentBitmask),
				GeneralBitmask(GeneralBitmask),
				CursorBitmask(CursorBitmask),
				PossessionsBitmask(PossessionsBitmask),
				CorpseBitmask(CorpseBitmask),
				BagSlotCount(BagSlotCount),
				AugSocketCount(AugSocketCount),
				AllowEmptyBagInBag(AllowEmptyBagInBag),
				AllowClickCastFromBag(AllowClickCastFromBag),
				ConcatenateInvTypeLimbo(ConcatenateInvTypeLimbo),
				AllowOverLevelEquipment(AllowOverLevelEquipment)
			{ }
		};

		void InitializeDynamicLookups();

		const LookupEntry* DynamicLookup(versions::MobVersion mob_version, bool gm_flag);
		const LookupEntry* DynamicNonGMLookup(versions::MobVersion mob_version);
		const LookupEntry* DynamicGMLookup(versions::MobVersion mob_version);
		const LookupEntry* StaticLookup(versions::MobVersion mob_version);

	} /*inventory*/
	
	namespace behavior {
		struct LookupEntry {
			bool CoinHasWeight;

			LookupEntry(const LookupEntry *lookup_entry) { }
			LookupEntry(
				bool CoinHasWeight
			) :
				CoinHasWeight(CoinHasWeight)
			{ }
		};

		void InitializeDynamicLookups();

		const LookupEntry* DynamicLookup(versions::MobVersion mob_version, bool gm_flag);
		const LookupEntry* DynamicNonGMLookup(versions::MobVersion mob_version);
		const LookupEntry* DynamicGMLookup(versions::MobVersion mob_version);
		const LookupEntry* StaticLookup(versions::MobVersion mob_version);

	} /*behavior*/

	namespace spells {
		struct LookupEntry {
			int SpellIdMax;
			int SpellbookSize;
			int SpellGemCount;

			int LongBuffs;
			int ShortBuffs;
			int DiscBuffs;
			int TotalBuffs;
			int NPCBuffs;
			int PetBuffs;
			int MercBuffs;
			
			LookupEntry(const LookupEntry *lookup_entry) { }
			LookupEntry(
				int SpellIdMax,
				int SpellbookSize,
				int SpellGemCount,
				int LongBuffs,
				int ShortBuffs,
				int DiscBuffs,
				int TotalBuffs,
				int NPCBuffs,
				int PetBuffs,
				int MercBuffs
			) :
				SpellIdMax(SpellIdMax),
				SpellbookSize(SpellbookSize),
				SpellGemCount(SpellGemCount),
				LongBuffs(LongBuffs),
				ShortBuffs(ShortBuffs),
				DiscBuffs(DiscBuffs),
				TotalBuffs(TotalBuffs),
				NPCBuffs(NPCBuffs),
				PetBuffs(PetBuffs),
				MercBuffs(MercBuffs)
			{ }
		};
		
		void InitializeDynamicLookups();
		
		const LookupEntry* DynamicLookup(versions::ClientVersion client_version, bool gm_flag);
		const LookupEntry* DynamicNonGMLookup(versions::ClientVersion client_version);
		const LookupEntry* DynamicGMLookup(versions::ClientVersion client_version);
		const LookupEntry* StaticLookup(versions::ClientVersion client_version);

	} /*spells*/

} /*EQEmu*/

namespace ClientUnknown
{
	const int16 IINVALID = -1;
	const int16 INULL = 0;

	namespace constants {
		const EQ::expansions::Expansion EXPANSION = EQ::expansions::Expansion::EverQuest;
		const uint32 EXPANSION_BIT = EQ::expansions::bitEverQuest;
		const uint32 EXPANSIONS_MASK = EQ::expansions::maskEverQuest;

	} // namespace constants

} /*ClientUnknown*/

namespace Client62
{
	const int16 IINVALID = -1;
	const int16 INULL = 0;

	namespace constants {
		const EQ::expansions::Expansion EXPANSION = EQ::expansions::Expansion::EverQuest;
		const uint32 EXPANSION_BIT = EQ::expansions::bitEverQuest;
		const uint32 EXPANSIONS_MASK = EQ::expansions::maskEverQuest;

	} // namespace constants

} /*Client62*/
