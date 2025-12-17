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

#include "item_data.h"

#include "common/classes.h"
#include "common/races.h"


uint32 EQ::item::ConvertAugTypeToAugTypeBit(uint8 aug_type)
{
	switch (aug_type) {
	case AugTypeGeneralSingleStat:
		return bit_AugTypeGeneralSingleStat;
	case AugTypeGeneralMultipleStat:
		return bit_AugTypeGeneralMultipleStat;
	case AugTypeGeneralSpellEffect:
		return bit_AugTypeGeneralSpellEffect;
	case AugTypeWeaponGeneral:
		return bit_AugTypeWeaponGeneral;
	case AugTypeWeaponElemDamage:
		return bit_AugTypeWeaponElemDamage;
	case AugTypeWeaponBaseDamage:
		return bit_AugTypeWeaponBaseDamage;
	case AugTypeGeneralGroup:
		return bit_AugTypeGeneralGroup;
	case AugTypeGeneralRaid:
		return bit_AugTypeGeneralRaid;
	case AugTypeGeneralDragonsPoints:
		return bit_AugTypeGeneralDragonsPoints;
	case AugTypeCraftedCommon:
		return bit_AugTypeCraftedCommon;
	case AugTypeCraftedGroup1:
		return bit_AugTypeCraftedGroup1;
	case AugTypeCraftedRaid1:
		return bit_AugTypeCraftedRaid1;
	case AugTypeEnergeiacGroup:
		return bit_AugTypeEnergeiacGroup;
	case AugTypeEnergeiacRaid:
		return bit_AugTypeEnergeiacRaid;
	case AugTypeEmblem:
		return bit_AugTypeEmblem;
	case AugTypeCraftedGroup2:
		return bit_AugTypeCraftedGroup2;
	case AugTypeCraftedRaid2:
		return bit_AugTypeCraftedRaid2;
	case AugTypeUnknown1:
		return bit_AugTypeUnknown1;
	case AugTypeUnknown2:
		return bit_AugTypeUnknown2;
	case AugTypeOrnamentation:
		return bit_AugTypeOrnamentation;
	case AugTypeSpecialOrnamentation:
		return bit_AugTypeSpecialOrnamentation;
	case AugTypeUnknown3:
		return bit_AugTypeUnknown3;
	case AugTypeUnknown4:
		return bit_AugTypeUnknown4;
	case AugTypeUnknown5:
		return bit_AugTypeUnknown5;
	case AugTypeUnknown6:
		return bit_AugTypeUnknown6;
	case AugTypeUnknown7:
		return bit_AugTypeUnknown7;
	case AugTypeUnknown8:
		return bit_AugTypeUnknown8;
	case AugTypeUnknown9:
		return bit_AugTypeUnknown9;
	case AugTypeUnknown10:
		return bit_AugTypeUnknown10;
	case AugTypeEpic2_5:
		return bit_AugTypeEpic2_5;
	case AugTypeTest:
		return bit_AugTypeTest;
	case AugTypeAll:
		return bit_AugTypeAll;
	default:
		return bit_AugTypeNone;
	}
}

uint8 EQ::item::ConvertAugTypeBitToAugType(uint32 aug_type_bit)
{
	switch (aug_type_bit) {
	case bit_AugTypeGeneralSingleStat:
		return AugTypeGeneralSingleStat;
	case bit_AugTypeGeneralMultipleStat:
		return AugTypeGeneralMultipleStat;
	case bit_AugTypeGeneralSpellEffect:
		return AugTypeGeneralSpellEffect;
	case bit_AugTypeWeaponGeneral:
		return AugTypeWeaponGeneral;
	case bit_AugTypeWeaponElemDamage:
		return AugTypeWeaponElemDamage;
	case bit_AugTypeWeaponBaseDamage:
		return AugTypeWeaponBaseDamage;
	case bit_AugTypeGeneralGroup:
		return AugTypeGeneralGroup;
	case bit_AugTypeGeneralRaid:
		return AugTypeGeneralRaid;
	case bit_AugTypeGeneralDragonsPoints:
		return AugTypeGeneralDragonsPoints;
	case bit_AugTypeCraftedCommon:
		return AugTypeCraftedCommon;
	case bit_AugTypeCraftedGroup1:
		return AugTypeCraftedGroup1;
	case bit_AugTypeCraftedRaid1:
		return AugTypeCraftedRaid1;
	case bit_AugTypeEnergeiacGroup:
		return AugTypeEnergeiacGroup;
	case bit_AugTypeEnergeiacRaid:
		return AugTypeEnergeiacRaid;
	case bit_AugTypeEmblem:
		return AugTypeEmblem;
	case bit_AugTypeCraftedGroup2:
		return AugTypeCraftedGroup2;
	case bit_AugTypeCraftedRaid2:
		return AugTypeCraftedRaid2;
	case bit_AugTypeUnknown1:
		return AugTypeUnknown1;
	case bit_AugTypeUnknown2:
		return AugTypeUnknown2;
	case bit_AugTypeOrnamentation:
		return AugTypeOrnamentation;
	case bit_AugTypeSpecialOrnamentation:
		return AugTypeSpecialOrnamentation;
	case bit_AugTypeUnknown3:
		return AugTypeUnknown3;
	case bit_AugTypeUnknown4:
		return AugTypeUnknown4;
	case bit_AugTypeUnknown5:
		return AugTypeUnknown5;
	case bit_AugTypeUnknown6:
		return AugTypeUnknown6;
	case bit_AugTypeUnknown7:
		return AugTypeUnknown7;
	case bit_AugTypeUnknown8:
		return AugTypeUnknown8;
	case bit_AugTypeUnknown9:
		return AugTypeUnknown9;
	case bit_AugTypeUnknown10:
		return AugTypeUnknown10;
	case bit_AugTypeEpic2_5:
		return AugTypeEpic2_5;
	case bit_AugTypeTest:
		return AugTypeTest;
	case bit_AugTypeAll:
		return AugTypeAll;
	default:
		return AugTypeNone;
	}
}

bool EQ::ItemData::IsEquipable(uint16 race_id, uint16 class_id) const
{
	if (!(Races & GetPlayerRaceBit(race_id))) {
		return false;
	}

	if (!(Classes & GetPlayerClassBit(GetPlayerClassValue(class_id)))) {
		return false;
	}

	return true;
}

bool EQ::ItemData::IsClassEquipable(uint16 class_id) const
{

	if (!(Classes & GetPlayerClassBit(GetPlayerClassValue(class_id)))) {
		return false;
	}

	return true;
}

bool EQ::ItemData::IsRaceEquipable(uint16 race_id) const
{

	if (!(Races & GetPlayerRaceBit(race_id))) {
		return false;
	}

	return true;
}

bool EQ::ItemData::IsClassCommon() const
{
	return (ItemClass == item::ItemClassCommon);
}

bool EQ::ItemData::IsClassBag() const
{
	return (ItemClass == item::ItemClassBag);
}

bool EQ::ItemData::IsClassBook() const
{
	return (ItemClass == item::ItemClassBook);
}

bool EQ::ItemData::IsType1HWeapon() const
{
	return ((ItemType == item::ItemType1HBlunt) || (ItemType == item::ItemType1HSlash) || (ItemType == item::ItemType1HPiercing) || (ItemType == item::ItemTypeMartial));
}

bool EQ::ItemData::IsPetUsable() const
{
	if (ItemClass == item::ItemClassBag) {
		return true;
	}

	// if it's a misc item and has slots, it's wearable
	// this item type is conflated with many other item types
	if (ItemClass == item::ItemTypeMisc && Slots != 0) {
		return true;
	}

	switch (ItemType) {
		case item::ItemType1HBlunt:
		case item::ItemType1HSlash:
		case item::ItemType1HPiercing:
		case item::ItemType2HBlunt:
		case item::ItemType2HSlash:
		case item::ItemTypeMartial:
		case item::ItemTypeShield:
		case item::ItemTypeArmor:
		case item::ItemTypeJewelry:
			return true;
		default:
			return false;
	}
}

bool EQ::ItemData::IsType2HWeapon() const
{
	return ((ItemType == item::ItemType2HBlunt) || (ItemType == item::ItemType2HSlash) || (ItemType == item::ItemType2HPiercing));
}

bool EQ::ItemData::IsTypeShield() const
{
	return (ItemType == item::ItemTypeShield);
}

bool EQ::ItemData::IsQuestItem() const {
	return QuestItemFlag;
}

bool EQ::ItemData::CheckLoreConflict(const ItemData* l_item, const ItemData* r_item)
{
	if (!l_item || !r_item)
		return false;

	if (!l_item->LoreGroup || !r_item->LoreGroup)
		return false;

	if (l_item->LoreGroup == r_item->LoreGroup) {
		if ((l_item->LoreGroup == -1) && (l_item->ID != r_item->ID))
			return false;

		return true;
	}

	return false;
}
