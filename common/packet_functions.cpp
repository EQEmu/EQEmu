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

#include "packet_functions.h"

#include "common/packet_dump.h"
#include "common/platform/inet.h"

#include "zlib.h"
#include <iomanip>
#include <iostream>
#include <string.h>

void EncryptProfilePacket(EQApplicationPacket* app) {
	//EncryptProfilePacket(app->pBuffer, app->size);
}

void EncryptProfilePacket(uchar* pBuffer, uint32 size) {
	uint64* data=(uint64*)pBuffer;
	uint64 crypt = 0x659365E7;
	uint64 next_crypt;
	uint32 len = size >> 3;

	uint64 swap = data[0];
	data[0] = data[len/2];
	data[len/2] = swap;

	for(uint32 i=0; i<len;i++) {
		next_crypt = crypt+data[i]-0x422437A9;
		data[i] = ((data[i]>>0x19)|(data[i]<<0x27))+0x422437A9;
		data[i] = (data[i]<<0x07)|(data[i]>>0x39);
		data[i] = data[i] - crypt;
		crypt = next_crypt;
	}
}

void EncryptZoneSpawnPacket(EQApplicationPacket* app) {
	//EncryptZoneSpawnPacket(app->pBuffer, app->size);
}

void EncryptZoneSpawnPacket(uchar* pBuffer, uint32 size) {
	uint64* data=(uint64*)pBuffer;
	uint64 crypt = 0x0000;
	uint64 next_crypt;
	uint32 len = size >> 3;

	uint64 swap = data[0];
	data[0] = data[len/2];
	data[len/2] = swap;

	for(uint32 i=0; i<len;i++) {
		next_crypt = crypt+data[i]-0x659365E7;
		data[i] = ((data[i]<<0x1d)|(data[i]>>0x23))+0x659365E7;
		data[i] = (data[i]<<0x0e)|(data[i]>>0x32);
		data[i] = data[i] - crypt;
		crypt = next_crypt;
	}
}

#define MEMORY_DEBUG

#ifndef MEMORY_DEBUG
#define eqemu_alloc_func Z_NULL
#define eqemu_free_func Z_NULL
#else
//These functions only exist to make my memory profiler
voidpf eqemu_alloc_func(voidpf opaque, uInt items, uInt size);
void eqemu_free_func(voidpf opaque, voidpf address);

voidpf eqemu_alloc_func(voidpf opaque, uInt items, uInt size) {
	voidpf tmp = new char[items*size];
	return(tmp);
}

void eqemu_free_func(voidpf opaque, voidpf address) {
	delete[] (char *)address;
}
#endif


int DeflatePacket(const unsigned char* in_data, int in_length, unsigned char* out_data, int max_out_length) {
#ifdef REUSE_ZLIB
	static bool inited = false;
	static z_stream zstream;
	int zerror;

	if(in_data == nullptr && out_data == nullptr && in_length == 0 && max_out_length == 0) {
		//special delete state
		deflateEnd(&zstream);
		return(0);
	}
	if(!inited) {
		memset(&zstream, 0, sizeof(zstream));
		zstream.zalloc	= eqemu_alloc_func;
		zstream.zfree	= eqemu_free_func;
		zstream.opaque	= Z_NULL;
		deflateInit(&zstream, Z_FINISH);
	}

	zstream.next_in		= const_cast<unsigned char *>(in_data);
	zstream.avail_in	= in_length;
/*	zstream.zalloc	= Z_NULL;
	zstream.zfree	= Z_NULL;
	zstream.opaque	= Z_NULL;
	deflateInit(&zstream, Z_FINISH);*/
	zstream.next_out = out_data;
	zstream.avail_out = max_out_length;
	zerror = deflate(&zstream, Z_FINISH);

	deflateReset(&zstream);

	if (zerror == Z_STREAM_END)
	{
//		deflateEnd(&zstream);
		return zstream.total_out;
	}
	else
	{
//		zerror = deflateEnd(&zstream);
		return 0;
	}
#else
	if(in_data == nullptr) {
		return(0);
	}

	z_stream zstream;
	memset(&zstream, 0, sizeof(zstream));
	int zerror;

	zstream.next_in		= const_cast<unsigned char *>(in_data);
	zstream.avail_in	= in_length;
	zstream.zalloc	= eqemu_alloc_func;
	zstream.zfree	= eqemu_free_func;
	zstream.opaque	= Z_NULL;
	deflateInit(&zstream, Z_FINISH);
	zstream.next_out = out_data;
	zstream.avail_out = max_out_length;
	zerror = deflate(&zstream, Z_FINISH);

	if (zerror == Z_STREAM_END)
	{
		deflateEnd(&zstream);
		return zstream.total_out;
	}
	else
	{
		zerror = deflateEnd(&zstream);
		return 0;
	}
#endif
}

uint32 InflatePacket(const uchar* indata, uint32 indatalen, uchar* outdata, uint32 outdatalen, bool iQuiet) {
#ifdef REUSE_ZLIB
	static bool inited = false;
	static z_stream zstream;
	int zerror;

	if(indata == nullptr && outdata == nullptr && indatalen == 0 && outdatalen == 0) {
		//special delete state
		inflateEnd(&zstream);
		return(0);
	}
	if(!inited) {
		zstream.zalloc	= eqemu_alloc_func;
		zstream.zfree	= eqemu_free_func;
		zstream.opaque	= Z_NULL;
		inflateInit2(&zstream, 15);
	}

	zstream.next_in		= const_cast<unsigned char *>(indata);
	zstream.avail_in	= indatalen;
	zstream.next_out	= outdata;
	zstream.avail_out	= outdatalen;
	zstream.zalloc		= eqemu_alloc_func;
	zstream.zfree		= eqemu_free_func;
	zstream.opaque		= Z_NULL;

	i = inflateInit2( &zstream, 15 );
	if (i != Z_OK) {
		return 0;
	}

	zerror = inflate( &zstream, Z_FINISH );

	inflateReset(&zstream);

	if(zerror == Z_STREAM_END) {
		return zstream.total_out;
	}
	else {
		if (!iQuiet) {
			std::cout << "Error: InflatePacket: inflate() returned " << zerror << " '";
			if (zstream.msg)
				std::cout << zstream.msg;
			std::cout << "'" << std::endl;
#ifdef EQDEBUG
			DumpPacket(indata-16, indatalen+16);
#endif
		}

		if (zerror == -4 && zstream.msg == 0)
		{
			return 0;
		}

		return 0;
	}
#else
	if(indata == nullptr)
		return(0);

	z_stream zstream;
	int zerror = 0;
	int i;

	zstream.next_in		= const_cast<unsigned char *>(indata);
	zstream.avail_in	= indatalen;
	zstream.next_out	= outdata;
	zstream.avail_out	= outdatalen;
	zstream.zalloc		= eqemu_alloc_func;
	zstream.zfree		= eqemu_free_func;
	zstream.opaque		= Z_NULL;

	i = inflateInit2( &zstream, 15 );
	if (i != Z_OK) {
		return 0;
	}

	zerror = inflate( &zstream, Z_FINISH );

	if(zerror == Z_STREAM_END) {
		inflateEnd( &zstream );
		return zstream.total_out;
	}
	else {
		if (!iQuiet) {
			std::cout << "Error: InflatePacket: inflate() returned " << zerror << " '";
			if (zstream.msg)
				std::cout << zstream.msg;
			std::cout << "'" << std::endl;
#ifdef EQDEBUG
			DumpPacket(indata-16, indatalen+16);
#endif
		}

		if (zerror == -4 && zstream.msg == 0)
		{
			return 0;
		}

		zerror = inflateEnd( &zstream );
		return 0;
	}
#endif
}

uint32 roll(uint32 in, uint8 bits) {
	return ((in << bits) | (in >> (32-bits)));
}

uint64 roll(uint64 in, uint8 bits) {
	return ((in << bits) | (in >> (64-bits)));
}

uint32 rorl(uint32 in, uint8 bits) {
	return ((in >> bits) | (in << (32-bits)));
}

uint64 rorl(uint64 in, uint8 bits) {
	return ((in >> bits) | (in << (64-bits)));
}

uint32 CRCLookup(uchar idx) {
	if (idx == 0)
		return 0x00000000;

	if (idx == 1)
		return 0x77073096;

	if (idx == 2)
		return roll(CRCLookup(1), 1);

	if (idx == 4)
		return 0x076DC419;

	for (uchar b=7; b>0; b--) {
		uchar bv = 1 << b;

		if (!(idx ^ bv)) {
			// bit is only one set
			return ( roll(CRCLookup (4), b - 2) );
		}

		if (idx&bv) {
			// bit is set
			return( CRCLookup(bv) ^ CRCLookup(idx&(bv - 1)) );
		}
	}

	//Failure
	return false;
}

uint32 GenerateCRC(uint32 b, uint32 bufsize, uchar *buf) {
	uint32 CRC = (b ^ 0xFFFFFFFF);
	uint32 bufremain = bufsize;
	uchar* bufptr = buf;

	while (bufremain--) {
		CRC = CRCLookup((uchar)(*(bufptr++)^ (CRC&0xFF))) ^ (CRC >> 8);
	}

	return (htonl (CRC ^ 0xFFFFFFFF));
}
