/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#pragma once

#include "common/types.h"
#include "common/net/packet.h"

class ServerPacket;

void DumpPacketAscii(const uchar* buf, uint32 size, uint32 cols=16, uint32 skip=0);
void DumpPacketHex(const uchar* buf, uint32 size, uint32 cols=16, uint32 skip=0);
std::string DumpPacketHexToString(const uchar* buf, uint32 size, uint32 cols = 16, uint32 skip = 0);
void DumpPacketBin(const void* data, uint32 len);
void DumpPacket(const uchar* buf, uint32 size);
void DumpPacket(const ServerPacket* pack, bool iShowInfo = false);
void DumpPacket(uint16 opcode, const EQ::Net::Packet &p, bool iShowInfo = false);
void DumpPacketBin(const ServerPacket* pack);
void DumpPacketBin(uint32 data);
void DumpPacketBin(uint16 data);
void DumpPacketBin(uint8 data);
