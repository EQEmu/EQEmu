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
