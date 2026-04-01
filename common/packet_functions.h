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

uint32 roll(uint32 in, uint8 bits);
uint64 roll(uint64 in, uint8 bits);
uint32 rorl(uint32 in, uint8 bits);
uint64 rorl(uint64 in, uint8 bits);

void EncryptProfilePacket(EQApplicationPacket* app);
void EncryptProfilePacket(uchar* pBuffer, uint32 size);

#define EncryptSpawnPacket EncryptZoneSpawnPacket
//void EncryptSpawnPacket(EQApplicationPacket* app);
//void EncryptSpawnPacket(uchar* pBuffer, uint32 size);

void EncryptZoneSpawnPacket(EQApplicationPacket* app);
void EncryptZoneSpawnPacket(uchar* pBuffer, uint32 size);

int DeflatePacket(const unsigned char* in_data, int in_length, unsigned char* out_data, int max_out_length);
uint32 InflatePacket(const uchar* indata, uint32 indatalen, uchar* outdata, uint32 outdatalen, bool iQuiet = false);
uint32 GenerateCRC(uint32 b, uint32 bufsize, uchar *buf);
