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
