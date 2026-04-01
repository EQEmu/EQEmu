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

class CRC32 {
public:
	// one buffer CRC32
	static uint32			Generate(const uint8* buf, uint32 bufsize);
	static uint32			GenerateNoFlip(const uint8* buf, uint32 bufsize); // Same as Generate(), but without the ~
	static void				SetEQChecksum(uchar* in_data, uint32 in_length, uint32 start_at=4);

	// Multiple buffer CRC32
	static uint32			Update(const uint8* buf, uint32 bufsize, uint32 crc32 = 0xFFFFFFFF);
	static inline uint32	Finish(uint32 crc32)	{ return ~crc32; }
	static inline void		Finish(uint32* crc32)	{ *crc32 = ~(*crc32); }

private:
	static inline void		Calc(const uint8 byte, uint32& crc32);
};
