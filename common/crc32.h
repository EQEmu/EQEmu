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
