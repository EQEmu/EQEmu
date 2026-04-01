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

class EQApplicationPacket;

void FileDumpPacketAscii(const char* filename, const uchar* buf, uint32 size, uint32 cols=16, uint32 skip=0);
void FileDumpPacketHex(const char* filename, const uchar* buf, uint32 size, uint32 cols=16, uint32 skip=0);
void FileDumpPacketHex(const char* filename, const EQApplicationPacket* app);
void FileDumpPacketAscii(const char* filename, const EQApplicationPacket* app);
void FileDumpPacket(const char* filename, const uchar* buf, uint32 size);
void FileDumpPacket(const char* filename, const EQApplicationPacket* app);
void FilePrintLine(const char* filename, bool prefix_timestamp = false, const char* text = 0, ...);
void FilePrint(const char* filename, bool newline = true, bool prefix_timestamp = false, const char* text = 0, ...);
