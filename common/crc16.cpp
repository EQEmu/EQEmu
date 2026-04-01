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
#include "crc16.h"

#include "common/crc32.h"

uint16 CRC16(const unsigned char *buf, int size, int key)
{
	// This is computed as the lowest 16 bits of an Ethernet CRC32 checksum
	// where the key is prepended to the data in little endian order.
	uint8 keyBuf[] = {(uint8)((key >> 0) & 0xff),
					(uint8)((key >> 8) & 0xff),
					(uint8)((key >> 16) & 0xff),
					(uint8)((key >> 24) & 0xff)};
	uint32 crc = CRC32::Update(keyBuf, sizeof(uint32));
	crc = CRC32::Update(buf, size, crc);
	return CRC32::Finish(crc) & 0xffff;
}
