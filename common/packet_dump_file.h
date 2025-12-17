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

#include <iostream>

class EQApplicationPacket;

void FileDumpPacketAscii(const char* filename, const uchar* buf, uint32 size, uint32 cols=16, uint32 skip=0);
void FileDumpPacketHex(const char* filename, const uchar* buf, uint32 size, uint32 cols=16, uint32 skip=0);
void FileDumpPacketHex(const char* filename, const EQApplicationPacket* app);
void FileDumpPacketAscii(const char* filename, const EQApplicationPacket* app);
void FileDumpPacket(const char* filename, const uchar* buf, uint32 size);
void FileDumpPacket(const char* filename, const EQApplicationPacket* app);
void FilePrintLine(const char* filename, bool prefix_timestamp = false, const char* text = 0, ...);
void FilePrint(const char* filename, bool newline = true, bool prefix_timestamp = false, const char* text = 0, ...);
