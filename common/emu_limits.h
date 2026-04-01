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
#pragma once

#include "common/types.h"

#include <cstdlib>


namespace EntityLimits
{
	namespace NPC {
		const int16 IINVALID = -1;
		const int16 INULL = 0;

		namespace invtype {
			const int16 TRADE_SIZE = 4;

		} // namespace invtype

	} /*NPC*/

	namespace NPCMerchant {
		const int16 IINVALID = -1;
		const int16 INULL = 0;

		namespace invtype {
			const int16 TRADE_SIZE = 4;

		} // namespace invtype

	} /*NPCMerchant*/

	namespace Merc {
		const int16 IINVALID = -1;
		const int16 INULL = 0;

		namespace invtype {
			const int16 TRADE_SIZE = 4;

		} // namespace invtype

	} /*Merc*/

	namespace Bot {
		const int16 IINVALID = -1;
		const int16 INULL = 0;

		namespace invtype {
			const int16 TRADE_SIZE = 8;

		} // namespace invtype

		namespace invslot {
			const uint64 EQUIPMENT_BITMASK = 0x00000000007FFFFF;
			const uint64 GENERAL_BITMASK = 0x0000000000000000;
			const uint64 CURSOR_BITMASK = 0x0000000000000000;
			const uint64 POSSESSIONS_BITMASK = (EQUIPMENT_BITMASK | GENERAL_BITMASK | CURSOR_BITMASK); // based on 34-slot count (RoF+)

		} // namespace invslot

	} /*Bot*/

	namespace ClientPet {
		const int16 IINVALID = -1;
		const int16 INULL = 0;

		namespace invtype {
			const int16 TRADE_SIZE = 4;

		} // namespace invtype

	} /*Pet*/

	namespace NPCPet {
		const int16 IINVALID = -1;
		const int16 INULL = 0;

		namespace invtype {
			const int16 TRADE_SIZE = 4;

		} // namespace invtype

	} /*Pet*/

	namespace MercPet {
		const int16 IINVALID = -1;
		const int16 INULL = 0;

		namespace invtype {
			const int16 TRADE_SIZE = 4;

		} // namespace invtype

	} /*Pet*/

	namespace BotPet {
		const int16 IINVALID = -1;
		const int16 INULL = 0;

		namespace invtype {
			const int16 TRADE_SIZE = 4;

		} // namespace invtype

	} /*Pet*/

}; /*EntityLimits*/
