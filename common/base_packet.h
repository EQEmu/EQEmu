/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

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

#include "common/serialize_buffer.h"
#include "common/types.h"

#include <cstdio>
#include <cstring>

#ifdef _WINDOWS
	#include <time.h>
	#include <winsock2.h>
	#include <windows.h>
#else
	#include <sys/time.h>
	#include <netinet/in.h>
#endif

class BasePacket {
public:
	unsigned char *pBuffer;
	uint32 size, _wpos, _rpos;
	uint32 src_ip,dst_ip;
	uint16 src_port,dst_port;
	uint32 priority;
	timeval timestamp;

	virtual void build_raw_header_dump(char *buffer, uint16 seq=0xffff) const;
	virtual void build_header_dump(char *buffer) const;
	virtual void DumpRawHeader(uint16 seq=0xffff, FILE *to = stdout) const;
	virtual void DumpRawHeaderNoTime(uint16 seq=0xffff, FILE *to = stdout) const;
	void DumpRaw(FILE *to = stdout) const;

	void setSrcInfo(uint32 sip, uint16 sport) { src_ip=sip; src_port=sport; }
	void setDstInfo(uint32 dip, uint16 dport) { dst_ip=dip; dst_port=dport; }
	void setTimeInfo(uint32 ts_sec, uint32 ts_usec) { timestamp.tv_sec=ts_sec; timestamp.tv_usec=ts_usec; }
	void copyInfo(const BasePacket *p) { src_ip=p->src_ip; src_port=p->src_port; dst_ip=p->dst_ip; dst_port=p->dst_port; timestamp.tv_sec=p->timestamp.tv_sec; timestamp.tv_usec=p->timestamp.tv_usec; }

	inline bool operator<(const BasePacket &rhs) {
		return (timestamp.tv_sec < rhs.timestamp.tv_sec || (timestamp.tv_sec==rhs.timestamp.tv_sec && timestamp.tv_usec < rhs.timestamp.tv_usec));
	}

	void WriteUInt8(uint8 value) { *(uint8 *)(pBuffer + _wpos) = value; _wpos += sizeof(uint8); }
	void WriteUInt32(uint32 value) { *(uint32 *)(pBuffer + _wpos) = value; _wpos += sizeof(uint32); }
	void WriteUInt64(uint64 value) { *(uint64 *)(pBuffer + _wpos) = value; _wpos += sizeof(uint64); }
	void WriteUInt16(uint32 value) { *(uint16 *)(pBuffer + _wpos) = value; _wpos += sizeof(uint16); }
	void WriteSInt32(int32 value) { *(int32 *)(pBuffer + _wpos) = value; _wpos += sizeof(int32); }
	void WriteFloat(float value) { *(float *)(pBuffer + _wpos) = value; _wpos += sizeof(float); }
	void WriteDouble(double value) { *(double *)(pBuffer + _wpos) = value; _wpos += sizeof(double); }
	void WriteString(const char * str) { uint32 len = static_cast<uint32>(strlen(str)) + 1; memcpy(pBuffer + _wpos, str, len); _wpos += len; }
	// this is used in task system a lot, it is NOT null-termed
	void WriteLengthString(uint32 len, const char *str) { *(uint32 *)(pBuffer + _wpos) = len; _wpos += sizeof(uint32); memcpy(pBuffer + _wpos, str, len); _wpos += len; }
	void WriteData(const void *ptr, size_t n) { memcpy(pBuffer + _wpos, ptr, n); _wpos += n; }

	uint8 ReadUInt8() { uint8 value = *(uint8 *)(pBuffer + _rpos); _rpos += sizeof(uint8); return value; }
	uint8 ReadUInt8(uint32 Offset) const { uint8 value = *(uint8 *)(pBuffer + Offset); return value; }
	uint16 ReadUInt16() { uint16 value = *(uint16 *)(pBuffer + _rpos); _rpos += sizeof(uint16); return value; }
	uint16 ReadUInt16(uint32 Offset) const { uint16 value = *(uint16 *)(pBuffer + Offset); return value; }
	uint32 ReadUInt32() { uint32 value = *(uint32 *)(pBuffer + _rpos); _rpos += sizeof(uint32); return value; }
	uint32 ReadUInt32(uint32 Offset) const { uint32 value = *(uint32 *)(pBuffer + Offset); return value; }
	void ReadString(char *str) { uint32 len = static_cast<uint32>(strlen((char *)(pBuffer + _rpos))) + 1; memcpy(str, pBuffer + _rpos, len); _rpos += len; }
	void ReadString(std::string &str) { str = reinterpret_cast<char *>(pBuffer + _rpos); _rpos += str.length() + 1; }
	void ReadString(char *str, uint32 Offset, uint32 MaxLength) const;

	uint32 GetWritePosition() { return _wpos; }
	uint32 GetReadPosition() { return _rpos; }
	void SetWritePosition(uint32 Newwpos) { _wpos = Newwpos; }
	void SetReadPosition(uint32 Newrpos) { _rpos = Newrpos; }

protected:
	virtual ~BasePacket();
	BasePacket() { pBuffer=nullptr; size=0; _wpos = 0; _rpos = 0; }
	BasePacket(const unsigned char *buf, const uint32 len);
	BasePacket(SerializeBuffer &buf);
};

extern void DumpPacketHex(const BasePacket* app);
extern void DumpPacketAscii(const BasePacket* app);
extern void DumpPacketBin(const BasePacket* app);
