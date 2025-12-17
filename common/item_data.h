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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  04111-1307  USA
*/

#pragma once

#include "common/emu_constants.h"

/*
 * Note: (Doodman)
 *	This structure has field names that match the DB name exactly.
 *	Please take care as to not mess this up as it should make
 *	everyones life (i.e. mine) much easier. And the DB names
 *	match the field name from the 13th floor (SEQ) item collectors,
 *	so please maintain that as well.
 *
 * Note #2: (Doodman)
 *	UnkXXX fields are left in here for completeness but commented
 *	out since they are really unknown and since the items are now
 *	preserialized they should not be needed. Conversly if they
 *	-are- needed, then they shouldn't be unkown.
 *
 * Note #3: (Doodman)
 *	Please take care when adding new found data fields to add them
 *	to the appropriate structure. Item_Struct has elements that are
 *	global to all types of items only.
 *
 * Note #4: (Doodman)
 *	Made ya look! Ha!
 */

namespace EQ
{
	namespace item {
		enum ItemClass {
			ItemClassCommon = 0,
			ItemClassBag,
			ItemClassBook,
			ItemClassCount
		};

		enum ItemType : uint8 {
			/*9138*/ ItemType1HSlash = 0,
			/*9141*/ ItemType2HSlash,
			/*9140*/ ItemType1HPiercing,
			/*9139*/ ItemType1HBlunt,
			/*9142*/ ItemType2HBlunt,
			/*5504*/ ItemTypeBow,                 // 5
			/*----*/ ItemTypeUnknown1,
			/*----*/ ItemTypeLargeThrowing,
			/*5505*/ ItemTypeShield,
			/*5506*/ ItemTypeScroll,
			/*5507*/ ItemTypeArmor,               // 10
			/*5508*/ ItemTypeMisc,                // a lot of random crap has this item use.
			/*7564*/ ItemTypeLockPick,
			/*----*/ ItemTypeUnknown2,
			/*5509*/ ItemTypeFood,
			/*5510*/ ItemTypeDrink,               // 15
			/*5511*/ ItemTypeLight,
			/*5512*/ ItemTypeCombinable,          // not all stackable items are this use...
			/*5513*/ ItemTypeBandage,
			/*----*/ ItemTypeSmallThrowing,
			/*----*/ ItemTypeSpell,               // 20	// spells and tomes
			/*5514*/ ItemTypePotion,
			/*----*/ ItemTypeUnknown3,
			/*0406*/ ItemTypeWindInstrument,
			/*0407*/ ItemTypeStringedInstrument,
			/*0408*/ ItemTypeBrassInstrument,     // 25
			/*0405*/ ItemTypePercussionInstrument,
			/*5515*/ ItemTypeArrow,
			/*----*/ ItemTypeUnknown4,
			/*5521*/ ItemTypeJewelry,
			/*----*/ ItemTypeSkull,               // 30
			/*5516*/ ItemTypeBook,                // skill-up tomes/books? (would probably need a pp flag if true...)
			/*5517*/ ItemTypeNote,
			/*5518*/ ItemTypeKey,
			/*----*/ ItemTypeCoin,
			/*5520*/ ItemType2HPiercing,          // 35
			/*----*/ ItemTypeFishingPole,
			/*----*/ ItemTypeFishingBait,
			/*5519*/ ItemTypeAlcohol,
			/*----*/ ItemTypeKey2,                // keys and satchels?? (questable keys?)
			/*----*/ ItemTypeCompass,             // 40
			/*----*/ ItemTypeUnknown5,
			/*----*/ ItemTypePoison,              // might be wrong, but includes poisons
			/*----*/ ItemTypeUnknown6,
			/*----*/ ItemTypeUnknown7,
			/*5522*/ ItemTypeMartial,             // 45
			/*----*/ ItemTypeAllEffects,
			/*----*/ ItemTypeUnknown9,
			/*----*/ ItemTypeUnknown10,
			/*----*/ ItemTypeFocusEffect,
			/*----*/ ItemTypeSinging,             // 50
			/*5750*/ ItemTypeAllInstrumentTypes,
			/*5776*/ ItemTypeCharm,
			/*----*/ ItemTypeDye,
			/*----*/ ItemTypeAugmentation,
			/*----*/ ItemTypeAugmentationSolvent, // 55
			/*----*/ ItemTypeAugmentationDistiller,
			/*----*/ ItemTypeAlternateAbility,
			/*----*/ ItemTypeFellowshipKit,
			/*----*/ ItemTypeUnknown13,
			/*----*/ ItemTypeRecipe,              // 60
			/*----*/ ItemTypeAdvancedRecipe,
			/*----*/ ItemTypeJournal,             // only one(1) database entry
			/*----*/ ItemTypeAltCurrency,         // alt-currency (as opposed to coinage)
			/*5881*/ ItemTypePerfectedAugmentationDistiller,
			/*----*/ ItemTypeCount,
			/*----*/ ItemTypeCollectible,
			/*----*/ ItemTypeContainer,
			/*----*/ ItemTypeAll     = 0xFF

/*
			Unknowns:

			Mounts?
			Ornamentations?
			GuildBanners?
			Collectible?
			Placeable?
			(others?)
*/
		};

		enum AugTypeBit : uint32 {
			bit_AugTypeNone = 0x00000000,
			bit_AugTypeGeneralSingleStat = 0x00000001,		/*1^16^1 (General: Single Stat)^0*/
			bit_AugTypeGeneralMultipleStat = 0x00000002,	/*2^16^2 (General: Multiple Stat)^0*/
			bit_AugTypeGeneralSpellEffect = 0x00000004,		/*3^16^3 (General: Spell Effect)^0*/
			bit_AugTypeWeaponGeneral = 0x00000008,			/*4^16^4 (Weapon: General)^0*/
			bit_AugTypeWeaponElemDamage = 0x00000010,		/*5^16^5 (Weapon: Elem Damage)^0*/
			bit_AugTypeWeaponBaseDamage = 0x00000020,		/*6^16^6 (Weapon: Base Damage)^0*/
			bit_AugTypeGeneralGroup = 0x00000040,			/*7^16^7 (General: Group)^0*/
			bit_AugTypeGeneralRaid = 0x00000080,			/*8^16^8 (General: Raid)^0*/
			bit_AugTypeGeneralDragonsPoints = 0x00000100,	/*9^16^9 (General: Dragons Points)^0*/
			bit_AugTypeCraftedCommon = 0x00000200,			/*10^16^10 (Crafted: Common)^0*/
			bit_AugTypeCraftedGroup1 = 0x00000400,			/*11^16^11 (Crafted: Group)^0*/
			bit_AugTypeCraftedRaid1 = 0x00000800,			/*12^16^12 (Crafted: Raid)^0*/
			bit_AugTypeEnergeiacGroup = 0x00001000,			/*13^16^13 (Energeiac: Group)^0*/
			bit_AugTypeEnergeiacRaid = 0x00002000,			/*14^16^14 (Energeiac: Raid)^0*/
			bit_AugTypeEmblem = 0x00004000,					/*15^16^15 (Emblem)^0*/
			bit_AugTypeCraftedGroup2 = 0x00008000,			/*16^16^16 (Crafted: Group)^0*/
			bit_AugTypeCraftedRaid2 = 0x00010000,			/*17^16^17 (Crafted: Raid)^0*/
			bit_AugTypeUnknown1 = 0x00020000,				/*18^16^18^0*/
			bit_AugTypeUnknown2 = 0x00040000,				/*19^16^19^0*/
			bit_AugTypeOrnamentation = 0x00080000,			/*20^16^20 (Ornamentation)^0*/
			bit_AugTypeSpecialOrnamentation = 0x00100000,	/*21^16^21 (Special Ornamentation)^0*/
			bit_AugTypeUnknown3 = 0x00200000,				/*22^16^22^0*/
			bit_AugTypeUnknown4 = 0x00400000,				/*23^16^23^0*/
			bit_AugTypeUnknown5 = 0x00800000,				/*24^16^24^0*/
			bit_AugTypeUnknown6 = 0x01000000,				/*25^16^25^0*/
			bit_AugTypeUnknown7 = 0x02000000,				/*26^16^26^0*/
			bit_AugTypeUnknown8 = 0x04000000,				/*27^16^27^0*/
			bit_AugTypeUnknown9 = 0x08000000,				/*28^16^28^0*/
			bit_AugTypeUnknown10 = 0x10000000,				/*29^16^29^0*/
			bit_AugTypeEpic2_5 = 0x20000000,				/*30^16^30^0*/
			bit_AugTypeTest = 0x40000000,					/*31^16^Test^0*/ // listed as 31^16^31^0 in 5-10 client
			bit_AugTypeAll = 0xFFFFFFFF
		};

		enum AugType : uint8 {
			AugTypeNone = 0,
			AugTypeGeneralSingleStat,
			AugTypeGeneralMultipleStat,
			AugTypeGeneralSpellEffect,
			AugTypeWeaponGeneral,
			AugTypeWeaponElemDamage, // 5
			AugTypeWeaponBaseDamage,
			AugTypeGeneralGroup,
			AugTypeGeneralRaid,
			AugTypeGeneralDragonsPoints,
			AugTypeCraftedCommon, // 10
			AugTypeCraftedGroup1,
			AugTypeCraftedRaid1,
			AugTypeEnergeiacGroup,
			AugTypeEnergeiacRaid,
			AugTypeEmblem, // 15
			AugTypeCraftedGroup2,
			AugTypeCraftedRaid2,
			AugTypeUnknown1,
			AugTypeUnknown2,
			AugTypeOrnamentation, // 20
			AugTypeSpecialOrnamentation,
			AugTypeUnknown3,
			AugTypeUnknown4,
			AugTypeUnknown5,
			AugTypeUnknown6, // 25
			AugTypeUnknown7,
			AugTypeUnknown8,
			AugTypeUnknown9,
			AugTypeUnknown10,
			AugTypeEpic2_5, // 30
			AugTypeTest,
			AugTypeCount,
			AugTypeAll = 255
		};

		enum AugRestriction : uint8 {
/*4690*/	AugRestrictionAny = 0,
/*9134*/	AugRestrictionArmor,
/*9135*/	AugRestrictionWeapons,
/*9136*/	AugRestriction1HWeapons,
/*9137*/	AugRestriction2HWeapons,
/*9138*/	AugRestriction1HSlash, // 5
/*9139*/	AugRestriction1HBlunt,
/*9140*/	AugRestrictionPiercing,
/*9148*/	AugRestrictionHandToHand,
/*9141*/	AugRestriction2HSlash,
/*9142*/	AugRestriction2HBlunt, // 10
/*9143*/	AugRestriction2HPierce,
/*9144*/	AugRestrictionBows,
/*9145*/	AugRestrictionShields,
/*8052*/	AugRestriction1HSlash1HBluntOrHandToHand,
/*9200*/	AugRestriction1HBluntOrHandToHand, // 15	// no listed peq entries

		// these three appear to be post-RoF (12-10-2012) and can not be verified until RoF (05-10-2013) is supported
/*????*/	AugRestrictionUnknown1,
/*????*/	AugRestrictionUnknown2,
/*????*/	AugRestrictionUnknown3,					// last value in peq entries
			AugRestrictionCount

/*4687*/	//AugTypeAllItems, // ?? unknown atm
/*4688*/	//AugTypePrestige, // ?? unknown atm
/*4689*/	//AugTypeNonPrestige, // ?? unknown atm
		};

		enum BagType : uint8 {
/*3400*/	BagTypeSmallBag = 0,
/*3401*/	BagTypeLargeBag,
/*3402*/	BagTypeQuiver,
/*3403*/	BagTypeBeltPouch,
/*3404*/	BagTypeWristPouch,
/*3405*/	BagTypeBackPack, // 5
/*3406*/	BagTypeSmallChest,
/*3407*/	BagTypeLargeChest,
/*----*/	BagTypeBandolier,				// <*Database Reference Only>
/*3408*/	BagTypeMedicineBag,
/*3409*/	BagTypeToolBox, // 10
/*3410*/	BagTypeLexicon,
/*3411*/	BagTypeMortar,
/*3412*/	BagTypeSelfDusting,				// Quest container (Auto-clear contents?)
/*3413*/	BagTypeMixingBowl,
/*3414*/	BagTypeOven, // 15
/*3415*/	BagTypeSewingKit,
/*3416*/	BagTypeForge,
/*3417*/	BagTypeFletchingKit,
/*3418*/	BagTypeBrewBarrel,
/*3419*/	BagTypeJewelersKit, // 20
/*3420*/	BagTypePotteryWheel,
/*3421*/	BagTypeKiln,
/*3422*/	BagTypeKeymaker,				// (no database entries as of peq rev 69)
/*3423*/	BagTypeWizardsLexicon,
/*3424*/	BagTypeMagesLexicon, // 25
/*3425*/	BagTypeNecromancersLexicon,
/*3426*/	BagTypeEnchantersLexicon,
/*----*/	BagTypeUnknown1,				// (a coin pouch/purse?) (no database entries as of peq rev 69)
/*----*/	BagTypeConcordanceofResearch,	// <*Database Reference Only>
/*3427*/	BagTypeAlwaysWorks, // 30		// Quest container (Never-fail combines?)
/*3428*/	BagTypeKoadaDalForge,			// High Elf
/*3429*/	BagTypeTeirDalForge,			// Dark Elf
/*3430*/	BagTypeOggokForge,				// Ogre
/*3431*/	BagTypeStormguardForge,			// Dwarf
/*3432*/	BagTypeAkanonForge, // 35		// Gnome
/*3433*/	BagTypeNorthmanForge,			// Barbarian
/*----*/	BagTypeUnknown2,				// (no database entries as of peq rev 69)
/*3434*/	BagTypeCabilisForge,			// Iksar
/*3435*/	BagTypeFreeportForge,			// Human 1
/*3436*/	BagTypeRoyalQeynosForge, // 40	// Human 2
/*3439*/	BagTypeHalflingTailoringKit,
/*3438*/	BagTypeErudTailoringKit,
/*3440*/	BagTypeFierDalTailoringKit,		// Wood Elf
/*3441*/	BagTypeFierDalFletchingKit,		// Wood Elf
/*3437*/	BagTypeIksarPotteryWheel, // 45
/*3442*/	BagTypeTackleBox,
/*3443*/	BagTypeTrollForge,
/*3445*/	BagTypeFierDalForge,			// Wood Elf
/*3444*/	BagTypeValeForge,				// Halfling
/*3446*/	BagTypeErudForge, // 50
/*----*/	BagTypeTradersSatchel,			// <*Database Reference Only> (db: Yellow Trader's Satchel Token?)
/*5785*/	BagTypeGuktaForge,				// Froglok (no database entries as of peq rev 69)
/*3359*/	BagTypeAugmentationSealer,
/*----*/	BagTypeIceCreamChurn,			// <*Database Reference Only>
/*6325*/	BagTypeTransformationmold, // 55	// Ornamentation
/*6340*/	BagTypeDetransformationmold,	// Ornamentation Stripper
/*5400*/	BagTypeUnattuner,
/*7684*/	BagTypeTradeskillBag,
/*7692*/	BagTypeCollectibleBag,
/*----*/	BagTypeCount
		};

		enum ItemEffect {
			ItemEffectCombatProc = 0,
			ItemEffectClick,
			ItemEffectWorn,
			ItemEffectExpendable,
			ItemEffectEquipClick,
			ItemEffectClick2, //5		//name unknown
			ItemEffectFocus,
			ItemEffectScroll,
			ItemEffectCount
		};

		enum ItemSize : uint8 {
			ItemSizeTiny = 0,
			ItemSizeSmall,
			ItemSizeMedium,
			ItemSizeLarge,
			ItemSizeGiant,
			ItemSizeCount
		};

		enum ItemDataType : uint8 {
			ItemDataBase = 0,
			ItemDataScaling,
			ItemDataEvolving,
			ItemDataCount
		};

		struct ItemEffect_Struct {
			int32	Effect;
			uint8	Type;
			uint8	Level;
			uint8	Level2;
			//MaxCharges
			//CastTime
			//RecastDelay
			//RecastType
			//ProcRate
		};

		uint32 ConvertAugTypeToAugTypeBit(uint8 aug_type);
		uint8 ConvertAugTypeBitToAugType(uint32 aug_type_bit);

	} /*item*/

	struct InternalSerializedItem_Struct {
		int16 slot_id;
		const void * inst;
	};

	struct ItemData {
		// Non packet based fields
		uint8	MinStatus {};
		char	Comment[255] {};

		// Packet based fields
		uint8	ItemClass {};		// Item Type: 0=common, 1=container, 2=book
		char	Name[64] {};		// Name
		char	Lore[80] {};		// Lore Name: *=lore, &=summoned, #=artifact, ~=pending lore
		char	IDFile[30] {};		// Visible model
		uint32	ID {};				// Unique ID (also PK for DB)
		int32	Weight {};			// Item weight * 10
		uint8	NoRent{} ;			// No Rent: 0=norent, 255=not norent
		uint8	NoDrop {};			// No Drop: 0=nodrop, 255=not nodrop
		uint8	Size {};			// Size: 0=tiny, 1=small, 2=medium, 3=large, 4=giant
		uint32	Slots {};			// Bitfield for which slots this item can be used in
		uint32	Price {};			// Item cost (?)
		uint32	Icon {};			// Icon Number
		int32	LoreGroup {};		// Later items use LoreGroup instead of LoreFlag. we might want to see about changing this to int32 since it is commonly -1 and is constantly being cast from signed (-1) to unsigned (4294967295)
		bool	LoreFlag {};		// This will be true if LoreGroup is non-zero
		bool	PendingLoreFlag {};
		bool	ArtifactFlag {};
		bool	SummonedFlag {};
		uint8	FVNoDrop {};		// Firiona Vie nodrop flag
		uint32	Favor {};			// Individual favor
		uint32	GuildFavor {};		// Guild favor
		uint32	PointType {};

		//uint32	Unk117;
		//uint32	Unk118;
		//uint32	Unk121;
		//uint32	Unk124;

		uint8	BagType {};		// 0:Small Bag, 1:Large Bag, 2:Quiver, 3:Belt Pouch ... there are 50 types
		uint8	BagSlots {};		// Number of slots: can only be 2, 4, 6, 8, or 10
		uint8	BagSize {};		// 0:TINY, 1:SMALL, 2:MEDIUM, 3:LARGE, 4:GIANT
		uint8	BagWR {};			// 0->100

		bool	BenefitFlag {};
		bool	Tradeskills {};	// Is this a tradeskill item?
		int8	CR {};				// Save vs Cold
		int8	DR {};				// Save vs Disease
		int8	PR {};				// Save vs Poison
		int8	MR {};				// Save vs Magic
		int8	FR {};				// Save vs Fire
		int8	AStr {};			// Strength
		int8	ASta {};			// Stamina
		int8	AAgi {};			// Agility
		int8	ADex {};			// Dexterity
		int8	ACha {};			// Charisma
		int8	AInt {};			// Intelligence
		int8	AWis {};			// Wisdom
		int32	HP {};				// HP
		int32	Mana {};			// Mana
		int32	AC {};				// AC
		uint32	Deity {};			// Bitmask of Deities that can equip this item
		//uint32	Unk033
		int32	SkillModValue {};	// % Mod to skill specified in SkillModType
		int32	SkillModMax {};	// Max skill point modification
		uint32	SkillModType {};	// Type of skill for SkillModValue to apply to
		uint32	BaneDmgRace {};	// Bane Damage Race
		int32	BaneDmgAmt {};		// Bane Damage Body Amount
		uint32	BaneDmgBody {};	// Bane Damage Body
		bool	Magic {};			// True=Magic Item, False=not
		int32	CastTime_ {};
		uint8	ReqLevel {};		// Required Level to use item
		uint32	BardType {};		// Bard Skill Type
		int32	BardValue {};		// Bard Skill Amount
		int8	Light {};			// Light
		uint8	Delay {};			// Delay * 10
		uint8	RecLevel {};		// Recommended level to use item
		uint8	RecSkill {};		// Recommended skill to use item (refers to primary skill of item)
		uint8	ElemDmgType {};	// Elemental Damage Type (1=magic, 2=fire)
		uint8	ElemDmgAmt {};		// Elemental Damage
		uint8	Range {};			// Range of item
		uint32	Damage {};			// Delay between item usage (in 0.1 sec increments)
		uint32	Color {};			// RR GG BB 00 <-- as it appears in pc
		uint32	Classes {};		// Bitfield of classes that can equip item (1 << class#)
		uint32	Races {};			// Bitfield of races that can equip item (1 << race#)
		//uint32	Unk054 {};
		int16	MaxCharges {};		// Maximum charges items can hold: -1 if not a chargeable item
		uint8	ItemType {};		// Item Type/Skill (itemClass* from above)
		int32	SubType {};		// Some items have sub types that can be used for other things (unbreakable fishing poles, SpellEffect::FFItemClass)
		uint8	Material {};		// Item material type
		uint32	HerosForgeModel {};// Hero's Forge Armor Model Type (2-13?)
		float	SellRate {};		// Sell rate
		//uint32	Unk059 {};
		union {
			uint32 Fulfilment;	// Food fulfilment (How long it lasts)
			uint32 CastTime;		// Cast Time for clicky effects, in milliseconds
		};
		uint32 EliteMaterial {};
		int32	ProcRate {};
		int8	CombatEffects {};	// PoP: Combat Effects +
		int8	Shielding {};		// PoP: Shielding %
		int8	StunResist {};		// PoP: Stun Resist %
		int8	StrikeThrough {};	// PoP: Strike Through %
		int32	ExtraDmgSkill {};
		int32	ExtraDmgAmt {};
		int8	SpellShield {};	// PoP: Spell Shield %
		int8	Avoidance {};		// PoP: Avoidance +
		int8	Accuracy {};		// PoP: Accuracy +
		uint32	CharmFileID {};
		int32	FactionMod1 {};	// Faction Mod 1
		int32	FactionMod2 {};	// Faction Mod 2
		int32	FactionMod3 {};	// Faction Mod 3
		int32	FactionMod4 {};	// Faction Mod 4
		int32	FactionAmt1 {};	// Faction Amt 1
		int32	FactionAmt2 {};	// Faction Amt 2
		int32	FactionAmt3 {};	// Faction Amt 3
		int32	FactionAmt4 {};	// Faction Amt 4
		char	CharmFile[32] {};	// ?
		uint32	AugType {};
		uint8	AugSlotType[invaug::SOCKET_COUNT] {};	// RoF: Augment Slot 1-6 Type
		uint8	AugSlotVisible[invaug::SOCKET_COUNT] {};	// RoF: Augment Slot 1-6 Visible
		uint8	AugSlotUnk2[invaug::SOCKET_COUNT] {};	// RoF: Augment Slot 1-6 Unknown Most likely Powersource related
		uint32	LDoNTheme {};
		uint32	LDoNPrice {};
		uint32	LDoNSold {};
		uint32	BaneDmgRaceAmt {};
		uint32	AugRestrict {};
		int32	Endur {};
		int32	DotShielding {};
		int32	Attack {};
		int32	Regen {};
		int32	ManaRegen {};
		int32	EnduranceRegen {};
		int32	Haste {};
		int32	DamageShield {};
		uint32	RecastDelay {};
		int	RecastType {};
		uint32	AugDistiller {};
		bool	Attuneable {};
		bool	NoPet {};
		bool	PotionBelt {};
		bool	Stackable {};
		bool	NoTransfer {};
		bool	QuestItemFlag {};
		int16	StackSize {};
		uint8	PotionBeltSlots {};
		item::ItemEffect_Struct Click {};
		item::ItemEffect_Struct Proc {};
		item::ItemEffect_Struct Worn {};
		item::ItemEffect_Struct Focus {};
		item::ItemEffect_Struct Scroll {};
		item::ItemEffect_Struct Bard {};

		uint8	Book {};			// 0=Not book, 1=Book
		uint32	BookType {};
		char	Filename[33] {};	// Filename for book data
		// Begin SoF Fields
		int32 SVCorruption {};
		uint32 Purity {};
		uint8 EvolvingItem {};
		uint32 EvolvingID {};
		uint8 EvolvingLevel {};
		uint8 EvolvingMax {};
		uint32 BackstabDmg {};
		uint32 DSMitigation {};
		int32 HeroicStr {};
		int32 HeroicInt {};
		int32 HeroicWis {};
		int32 HeroicAgi {};
		int32 HeroicDex {};
		int32 HeroicSta {};
		int32 HeroicCha {};
		int32 HeroicMR {};
		int32 HeroicFR {};
		int32 HeroicCR {};
		int32 HeroicDR {};
		int32 HeroicPR {};
		int32 HeroicSVCorrup {};
		int32 HealAmt {};
		int32 SpellDmg {};
		uint32 LDoNSellBackRate {};
		uint32 ScriptFileID {};
		uint16 ExpendableArrow {};
		uint32 Clairvoyance {};
		char	ClickName[65] {};
		char	ProcName[65] {};
		char	WornName[65] {};
		char	FocusName[65] {};
		char	ScrollName[65] {};
		//BardName

		bool IsEquipable(uint16 Race, uint16 Class) const;
		bool IsClassEquipable(uint16 Class) const;
		bool IsRaceEquipable(uint16 Race) const;
		bool IsClassCommon() const;
		bool IsClassBag() const;
		bool IsClassBook() const;
		bool IsType1HWeapon() const;
		bool IsType2HWeapon() const;
		bool IsTypeShield() const;
		bool IsPetUsable() const;
		bool IsQuestItem() const;

		static bool CheckLoreConflict(const ItemData* l_item, const ItemData* r_item);
		bool CheckLoreConflict(const ItemData* item) const { return CheckLoreConflict(this, item); }
	};

} /*EQEmu*/
