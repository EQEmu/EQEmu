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

#include "common/platform/inet.h"
#include "common/serialize_buffer.h"
#include "common/types.h"

#include <cstdio>

class BasePacket
{
protected:
	BasePacket() = default;
	BasePacket(const unsigned char* buf, size_t len);
	BasePacket(SerializeBuffer&& buf);

	virtual ~BasePacket();

public:
	unsigned char* pBuffer = nullptr;
	uint32 size = 0;
	uint32 _wpos = 0;
	uint32 _rpos = 0;
	uint32 src_ip = 0;
	uint32 dst_ip = 0;
	uint16 src_port = 0;
	uint16 dst_port = 0;
	uint32 priority = 0;
	timeval timestamp{};

	virtual void build_raw_header_dump(char *buffer, uint16 seq=0xffff) const;
	virtual void build_header_dump(char *buffer) const;
	virtual void DumpRawHeader(uint16 seq=0xffff, FILE *to = stdout) const;
	virtual void DumpRawHeaderNoTime(uint16 seq=0xffff, FILE *to = stdout) const;
	void DumpRaw(FILE *to = stdout) const;

	void setSrcInfo(uint32 sip, uint16 sport) { src_ip=sip; src_port=sport; }
	void setDstInfo(uint32 dip, uint16 dport) { dst_ip=dip; dst_port=dport; }
	void setTimeInfo(uint32 ts_sec, uint32 ts_usec) { timestamp.tv_sec = ts_sec; timestamp.tv_usec = ts_usec; }
	void copyInfo(const BasePacket *p) { src_ip=p->src_ip; src_port=p->src_port; dst_ip=p->dst_ip; dst_port=p->dst_port; timestamp.tv_sec=p->timestamp.tv_sec; timestamp.tv_usec=p->timestamp.tv_usec; }

	inline bool operator<(const BasePacket &rhs) {
		return (timestamp.tv_sec < rhs.timestamp.tv_sec || (timestamp.tv_sec == rhs.timestamp.tv_sec && timestamp.tv_usec < rhs.timestamp.tv_usec));
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
};

extern void DumpPacketHex(const BasePacket* app);
extern void DumpPacketAscii(const BasePacket* app);
extern void DumpPacketBin(const BasePacket* app);
