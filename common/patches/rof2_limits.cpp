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

#include "rof2_limits.h"

#include "common/strings.h"


int16 RoF2::invtype::GetInvTypeSize(int16 inv_type)
{
	switch (inv_type) {
	case invtype::typePossessions:
		return invtype::POSSESSIONS_SIZE;
	case invtype::typeBank:
		return invtype::BANK_SIZE;
	case invtype::typeSharedBank:
		return invtype::SHARED_BANK_SIZE;
	case invtype::typeTrade:
		return invtype::TRADE_SIZE;
	case invtype::typeWorld:
		return invtype::WORLD_SIZE;
	case invtype::typeLimbo:
		return invtype::LIMBO_SIZE;
	case invtype::typeTribute:
		return invtype::TRIBUTE_SIZE;
	case invtype::typeTrophyTribute:
		return invtype::TROPHY_TRIBUTE_SIZE;
	case invtype::typeGuildTribute:
		return invtype::GUILD_TRIBUTE_SIZE;
	case invtype::typeMerchant:
		return invtype::MERCHANT_SIZE;
	case invtype::typeDeleted:
		return invtype::DELETED_SIZE;
	case invtype::typeCorpse:
		return invtype::CORPSE_SIZE;
	case invtype::typeBazaar:
		return invtype::BAZAAR_SIZE;
	case invtype::typeInspect:
		return invtype::INSPECT_SIZE;
	case invtype::typeRealEstate:
		return invtype::REAL_ESTATE_SIZE;
	case invtype::typeViewMODPC:
		return invtype::VIEW_MOD_PC_SIZE;
	case invtype::typeViewMODBank:
		return invtype::VIEW_MOD_BANK_SIZE;
	case invtype::typeViewMODSharedBank:
		return invtype::VIEW_MOD_SHARED_BANK_SIZE;
	case invtype::typeViewMODLimbo:
		return invtype::VIEW_MOD_LIMBO_SIZE;
	case invtype::typeAltStorage:
		return invtype::ALT_STORAGE_SIZE;
	case invtype::typeArchived:
		return invtype::ARCHIVED_SIZE;
	case invtype::typeMail:
		return invtype::MAIL_SIZE;
	case invtype::typeGuildTrophyTribute:
		return invtype::GUILD_TROPHY_TRIBUTE_SIZE;
	case invtype::typeKrono:
		return invtype::KRONO_SIZE;
	case invtype::typeOther:
		return invtype::OTHER_SIZE;
	default:
		return INULL;
	}
}

const char* RoF2::invtype::GetInvTypeName(int16 inv_type)
{
	switch (inv_type) {
	case invtype::TYPE_INVALID:
		return "Invalid Type";
	case invtype::typePossessions:
		return "Possessions";
	case invtype::typeBank:
		return "Bank";
	case invtype::typeSharedBank:
		return "Shared Bank";
	case invtype::typeTrade:
		return "Trade";
	case invtype::typeWorld:
		return "World";
	case invtype::typeLimbo:
		return "Limbo";
	case invtype::typeTribute:
		return "Tribute";
	case invtype::typeTrophyTribute:
		return "Trophy Tribute";
	case invtype::typeGuildTribute:
		return "Guild Tribute";
	case invtype::typeMerchant:
		return "Merchant";
	case invtype::typeDeleted:
		return "Deleted";
	case invtype::typeCorpse:
		return "Corpse";
	case invtype::typeBazaar:
		return "Bazaar";
	case invtype::typeInspect:
		return "Inspect";
	case invtype::typeRealEstate:
		return "Real Estate";
	case invtype::typeViewMODPC:
		return "View MOD PC";
	case invtype::typeViewMODBank:
		return "View MOD Bank";
	case invtype::typeViewMODSharedBank:
		return "View MOD Shared Bank";
	case invtype::typeViewMODLimbo:
		return "View MOD Limbo";
	case invtype::typeAltStorage:
		return "Alt Storage";
	case invtype::typeArchived:
		return "Archived";
	case invtype::typeMail:
		return "Mail";
	case invtype::typeGuildTrophyTribute:
		return "Guild Trophy Tribute";
	case invtype::typeKrono:
		return "Krono";
	case invtype::typeOther:
		return "Other";
	default:
		return "Unknown Type";
	}
}

bool RoF2::invtype::IsInvTypePersistent(int16 inv_type)
{
	switch (inv_type) {
	case invtype::typePossessions:
	case invtype::typeBank:
	case invtype::typeSharedBank:
	case invtype::typeTrade:
	case invtype::typeWorld:
	case invtype::typeLimbo:
	case invtype::typeTribute:
	case invtype::typeTrophyTribute:
	case invtype::typeGuildTribute:
		return true;
	default:
		return false;
	}
}

const char* RoF2::invslot::GetInvPossessionsSlotName(int16 inv_slot)
{
	switch (inv_slot) {
	case invslot::SLOT_INVALID:
		return "Invalid Slot";
	case invslot::slotCharm:
		return "Charm";
	case invslot::slotEar1:
		return "Ear 1";
	case invslot::slotHead:
		return "Head";
	case invslot::slotFace:
		return "Face";
	case invslot::slotEar2:
		return "Ear 2";
	case invslot::slotNeck:
		return "Neck";
	case invslot::slotShoulders:
		return "Shoulders";
	case invslot::slotArms:
		return "Arms";
	case invslot::slotBack:
		return "Back";
	case invslot::slotWrist1:
		return "Wrist 1";
	case invslot::slotWrist2:
		return "Wrist 2";
	case invslot::slotRange:
		return "Range";
	case invslot::slotHands:
		return "Hands";
	case invslot::slotPrimary:
		return "Primary";
	case invslot::slotSecondary:
		return "Secondary";
	case invslot::slotFinger1:
		return "Finger 1";
	case invslot::slotFinger2:
		return "Finger 2";
	case invslot::slotChest:
		return "Chest";
	case invslot::slotLegs:
		return "Legs";
	case invslot::slotFeet:
		return "Feet";
	case invslot::slotWaist:
		return "Waist";
	case invslot::slotPowerSource:
		return "Power Source";
	case invslot::slotAmmo:
		return "Ammo";
	case invslot::slotGeneral1:
		return "General 1";
	case invslot::slotGeneral2:
		return "General 2";
	case invslot::slotGeneral3:
		return "General 3";
	case invslot::slotGeneral4:
		return "General 4";
	case invslot::slotGeneral5:
		return "General 5";
	case invslot::slotGeneral6:
		return "General 6";
	case invslot::slotGeneral7:
		return "General 7";
	case invslot::slotGeneral8:
		return "General 8";
	case invslot::slotGeneral9:
		return "General 9";
	case invslot::slotGeneral10:
		return "General 10";
	case invslot::slotCursor:
		return "Cursor";
	default:
		return "Unknown Slot";
	}
}

const char* RoF2::invslot::GetInvSlotName(int16 inv_type, int16 inv_slot)
{
	if (inv_type == invtype::typePossessions)
		return invslot::GetInvPossessionsSlotName(inv_slot);

	int16 type_size = invtype::GetInvTypeSize(inv_type);

	if (!type_size || inv_slot == invslot::SLOT_INVALID)
		return "Invalid Slot";

	if ((inv_slot + 1) >= type_size)
		return "Unknown Slot";

	static std::string ret_str;
	ret_str = StringFormat("Slot %i", (inv_slot + 1));

	return ret_str.c_str();
}

const char* RoF2::invbag::GetInvBagIndexName(int16 bag_index)
{
	if (bag_index == invbag::SLOT_INVALID)
		return "Invalid Bag";

	if (bag_index >= invbag::SLOT_COUNT)
		return "Unknown Bag";

	static std::string ret_str;
	ret_str = StringFormat("Bag %i", (bag_index + 1));

	return ret_str.c_str();
}

const char* RoF2::invaug::GetInvAugIndexName(int16 aug_index)
{
	if (aug_index == invaug::SOCKET_INVALID)
		return "Invalid Augment";

	if (aug_index >= invaug::SOCKET_COUNT)
		return "Unknown Augment";

	static std::string ret_str;
	ret_str = StringFormat("Augment %i", (aug_index + 1));

	return ret_str.c_str();
}
