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
#include "common/base_packet.h"
#include "common/misc.h"
#include "common/packet_dump.h"

#include <algorithm>

BasePacket::BasePacket(const unsigned char* buf, size_t len)
{
	if (len > 0) {
		size = static_cast<uint32>(len);
		pBuffer = new unsigned char[len];
		if (buf) {
			memcpy(pBuffer, buf, len);
		}
		else {
			memset(pBuffer, 0, len);
		}
	}
}

BasePacket::BasePacket(SerializeBuffer&& buf)
	: pBuffer(std::exchange(buf.m_buffer, nullptr))
{
	// We are essentially taking ownership of this serialize buffer.
	size = static_cast<uint32>(std::exchange(buf.m_pos, 0));
	buf.m_capacity = 0;
}

BasePacket::~BasePacket()
{
	delete[] pBuffer;
	pBuffer = nullptr;
}

void BasePacket::build_raw_header_dump(char *buffer, uint16 seq) const
{
	if (timestamp.tv_sec) {
		char temp[20];
		strftime(temp,20,"%F %T",localtime((const time_t *)&timestamp.tv_sec));
		buffer += sprintf(buffer, "%s.%06lu ",temp,timestamp.tv_usec);
	}
	if (src_ip) {
		std::string sIP,dIP;;
		sIP=long2ip(src_ip);
		dIP=long2ip(dst_ip);
		buffer += sprintf(buffer, "[%s:%d->%s:%d]\n",sIP.c_str(),src_port,dIP.c_str(),dst_port);
	}
	if (seq != 0xffff)
		buffer += sprintf(buffer, "[Seq=%u] ",seq);
}

void BasePacket::DumpRawHeader(uint16 seq, FILE *to) const
{
	char buff[128];
	build_raw_header_dump(buff, seq);
	fprintf(to, "%s", buff);
}

void BasePacket::build_header_dump(char *buffer) const
{
	sprintf(buffer, "[packet]\n");
}

void BasePacket::DumpRawHeaderNoTime(uint16 seq, FILE *to) const
{
	if (src_ip) {
		std::string sIP,dIP;;
		sIP=long2ip(src_ip);
		dIP=long2ip(dst_ip);
		fprintf(to, "[%s:%d->%s:%d] ",sIP.c_str(),src_port,dIP.c_str(),dst_port);
	}
	if (seq != 0xffff)
		fprintf(to, "[Seq=%u] ",seq);
}

void BasePacket::DumpRaw(FILE *to) const
{
	DumpRawHeader();
	if (pBuffer && size)
		dump_message_column(pBuffer, size, " ", to);
	fprintf(to, "\n");
}

bool BasePacket::ReadCStringLength(size_t &len)
{
	if (_read_error || _rpos >= size) {
		_read_error = true;
		_rpos       = size;
		return false;
	}

	const size_t remaining = size - _rpos;
	len = strnlen(reinterpret_cast<const char *>(pBuffer + _rpos), remaining);
	if (len == remaining) {
		_read_error = true;
		_rpos       = size;
		return false;
	}

	return true;
}

void BasePacket::ReadString(char *str)
{
	size_t len;
	if (!ReadCStringLength(len)) {
		str[0] = '\0';
		return;
	}

	memcpy(str, pBuffer + _rpos, len + 1);
	_rpos += len + 1;
}

void BasePacket::ReadString(std::string &str)
{
	size_t len;
	if (!ReadCStringLength(len)) {
		str.clear();
		return;
	}

	str.assign(reinterpret_cast<const char *>(pBuffer + _rpos), len);
	_rpos += len + 1;
}

void BasePacket::ReadString(std::string &str, size_t max_len)
{
	size_t len;
	if (!ReadCStringLength(len)) {
		str.clear();
		return;
	}

	str.assign(reinterpret_cast<const char *>(pBuffer + _rpos), std::min(len, max_len));
	_rpos += len + 1;
}

void BasePacket::ReadLengthString(std::string &str)
{
	const uint32 len = ReadUInt32();
	if (_read_error || ReadRemaining() < len) {
		_read_error = true;
		_rpos       = size;
		str.clear();
		return;
	}

	str.assign(reinterpret_cast<const char *>(pBuffer + _rpos), len);
	_rpos += len;
}

void BasePacket::ReadSkip(uint32 count)
{
	if (_read_error || ReadRemaining() < count) {
		_read_error = true;
		_rpos       = size;
		return;
	}

	_rpos += count;
}

void BasePacket::ReadString(char *str, uint32 Offset, uint32 MaxLength) const
{
	uint32 i = 0, j = Offset;

	do
	{
		str[i++] = pBuffer[j++];
	}
	while((j < size) && (i < MaxLength) && (str[i - 1] != 0));

	str[i - 1] = '\0';
}

void DumpPacketHex(const BasePacket* app)
{
	DumpPacketHex(app->pBuffer, app->size);
}

void DumpPacketAscii(const BasePacket* app)
{
	DumpPacketAscii(app->pBuffer, app->size);
}

void DumpPacketBin(const BasePacket* app) {
	DumpPacketBin(app->pBuffer, app->size);
}

