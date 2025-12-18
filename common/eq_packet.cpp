/*
	Copyright (C) 2005 Michael S. Finger

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

#include "eq_packet.h"

#include "common/misc.h"
#include "common/op_codes.h"
#ifndef STATIC_OPCODE
#include "common/opcodemgr.h"
#endif
#include "common/packet_dump.h"
#include "common/packet_functions.h"
#include "common/platform.h"

#include <cstdio>
#include <iomanip>
#include <iostream>

EQPacket::EQPacket(EmuOpcode op, const unsigned char *buf, uint32 len)
:	BasePacket(buf, len),
	emu_opcode(op)
{
}

void EQPacket::build_raw_header_dump(char *buffer, uint16 seq) const {
	BasePacket::build_raw_header_dump(buffer, seq);
	buffer += strlen(buffer);

	buffer += sprintf(buffer, "[EmuOpCode 0x%04x Size=%u]\n", emu_opcode, size);
}

void EQPacket::DumpRawHeader(uint16 seq, FILE *to) const
{
	char buff[196];
	build_raw_header_dump(buff, seq);
	fprintf(to, "%s", buff);
}

void EQPacket::build_header_dump(char *buffer) const {
	sprintf(buffer, "[EmuOpCode 0x%04x Size=%u]", emu_opcode, size);
}

void EQPacket::DumpRawHeaderNoTime(uint16 seq, FILE *to) const
{
	if (src_ip) {
		std::string sIP,dIP;;
		sIP=long2ip(src_ip);
		dIP=long2ip(dst_ip);
		fprintf(to, "[%s:%d->%s:%d] ",sIP.c_str(),src_port,dIP.c_str(),dst_port);
	}
	if (seq != 0xffff)
		fprintf(to, "[Seq=%u] ",seq);

	fprintf(to, "[EmuOpCode 0x%04x Size=%lu]\n",emu_opcode,(unsigned long)size);
}

void EQProtocolPacket::build_raw_header_dump(char *buffer, uint16 seq) const
{
	BasePacket::build_raw_header_dump(buffer, seq);
	buffer += strlen(buffer);

	buffer += sprintf(buffer, "[ProtoOpCode 0x%04x Size=%u]\n",opcode,size);
}

void EQProtocolPacket::DumpRawHeader(uint16 seq, FILE *to) const
{
	char buff[196];
	build_raw_header_dump(buff, seq);
	fprintf(to, "%s", buff);
}

void EQProtocolPacket::build_header_dump(char *buffer) const
{
	sprintf(buffer, "[ProtoOpCode 0x%04x Size=%u]",opcode,size);
}

void EQProtocolPacket::DumpRawHeaderNoTime(uint16 seq, FILE *to) const
{
	if (src_ip) {
		std::string sIP,dIP;;
		sIP=long2ip(src_ip);
		dIP=long2ip(dst_ip);
		fprintf(to, "[%s:%d->%s:%d] ",sIP.c_str(),src_port,dIP.c_str(),dst_port);
	}
	if (seq != 0xffff)
		fprintf(to, "[Seq=%u] ",seq);

	fprintf(to, "[ProtoOpCode 0x%04x Size=%lu]\n",opcode,(unsigned long)size);
}

void EQApplicationPacket::build_raw_header_dump(char *buffer, uint16 seq) const
{
	BasePacket::build_raw_header_dump(buffer, seq);
	buffer += strlen(buffer);

#ifdef STATIC_OPCODE
	buffer += sprintf(buffer, "[OpCode 0x%04x Size=%u]\n", emu_opcode,size);
#else
	buffer += sprintf(buffer, "[OpCode %s Size=%u]\n",OpcodeManager::EmuToName(emu_opcode),size);
#endif
}

void EQApplicationPacket::DumpRawHeader(uint16 seq, FILE *to) const
{
	char buff[196];
	build_raw_header_dump(buff, seq);
	fprintf(to, "%s", buff);
}

void EQApplicationPacket::build_header_dump(char *buffer) const
{
#ifdef STATIC_OPCODE
	sprintf(buffer, "[OpCode 0x%04x Size=%u]\n", emu_opcode,size);
#else
	sprintf(buffer, "[OpCode %s(0x%04x) Size=%u]",OpcodeManager::EmuToName(emu_opcode), GetProtocolOpcode(), size);
#endif
}

void EQApplicationPacket::DumpRawHeaderNoTime(uint16 seq, FILE *to) const
{
	if (src_ip) {
		std::string sIP,dIP;;
		sIP=long2ip(src_ip);
		dIP=long2ip(dst_ip);
		fprintf(to, "[%s:%d->%s:%d] ",sIP.c_str(),src_port,dIP.c_str(),dst_port);
	}
	if (seq != 0xffff)
		fprintf(to, "[Seq=%u] ",seq);

#ifdef STATIC_OPCODE
	fprintf(to, "[OpCode 0x%04x Size=%u]\n", emu_opcode,size);
#else
	fprintf(to, "[OpCode %s Size=%lu]\n",OpcodeManager::EmuToName(emu_opcode),(unsigned long)size);
#endif
}

void EQRawApplicationPacket::build_raw_header_dump(char *buffer, uint16 seq) const
{
	BasePacket::build_raw_header_dump(buffer, seq);
	buffer += strlen(buffer);

#ifdef STATIC_OPCODE
	buffer += sprintf(buffer, "[OpCode 0x%04x (0x%04x) Size=%u]\n", emu_opcode, opcode,size);
#else
	buffer += sprintf(buffer, "[OpCode %s (0x%04x) Size=%u]\n", OpcodeManager::EmuToName(emu_opcode), opcode,size);
#endif
}

void EQRawApplicationPacket::DumpRawHeader(uint16 seq, FILE *to) const
{
	char buff[196];
	build_raw_header_dump(buff, seq);
	fprintf(to, "%s", buff);
}

void EQRawApplicationPacket::build_header_dump(char *buffer) const
{
#ifdef STATIC_OPCODE
	sprintf(buffer, "[OpCode 0x%04x (0x%04x) Size=%u]\n", emu_opcode, opcode,size);
#else
	sprintf(buffer, "[OpCode %s (0x%04x) Size=%u]", OpcodeManager::EmuToName(emu_opcode), opcode,size);
#endif
}

void EQRawApplicationPacket::DumpRawHeaderNoTime(uint16 seq, FILE *to) const
{
	if (src_ip) {
		std::string sIP,dIP;;
		sIP=long2ip(src_ip);
		dIP=long2ip(dst_ip);
		fprintf(to, "[%s:%d->%s:%d] ",sIP.c_str(),src_port,dIP.c_str(),dst_port);
	}
	if (seq != 0xffff)
		fprintf(to, "[Seq=%u] ",seq);

#ifdef STATIC_OPCODE
	fprintf(to, "[OpCode 0x%04x (0x%04x) Size=%u]\n", emu_opcode, opcode,size);
#else
	fprintf(to, "[OpCode %s (0x%04x) Size=%lu]\n", OpcodeManager::EmuToName(emu_opcode), opcode,(unsigned long)size);
#endif
}

uint32 EQProtocolPacket::serialize(unsigned char *dest) const
{
	if (opcode>0xff) {
		*(uint16 *)dest=opcode;
	} else {
		*(dest)=0;
		*(dest+1)=opcode;
	}
	memcpy(dest+2,pBuffer,size);

	return size+2;
}

uint32 EQApplicationPacket::serialize(uint16 opcode, unsigned char *dest) const
{
	uint8 OpCodeBytes = app_opcode_size;

	if (app_opcode_size==1)
		*(unsigned char *)dest = opcode;
	else
	{
		// Application opcodes with a low order byte of 0x00 require an extra 0x00 byte inserting prior to the opcode.
		if((opcode & 0x00ff) == 0)
		{
			*(uint8 *)dest = 0;
			*(uint16 *)(dest + 1) = opcode;
			++OpCodeBytes;
		}
		else
			*(uint16 *)dest = opcode;
	}
	memcpy(dest+OpCodeBytes,pBuffer,size);

	return size+OpCodeBytes;
}

bool EQProtocolPacket::combine(const EQProtocolPacket *rhs)
{
bool result=false;
	if (opcode==OP_Combined && size+rhs->size+5<256) {
		auto tmpbuffer = new unsigned char[size + rhs->size + 3];
		memcpy(tmpbuffer,pBuffer,size);
		uint32 offset=size;
		tmpbuffer[offset++]=rhs->Size();
		offset+=rhs->serialize(tmpbuffer+offset);
		size=offset;
		delete[] pBuffer;
		pBuffer=tmpbuffer;
		result=true;
	} else if (size+rhs->size+7<256) {
		auto tmpbuffer = new unsigned char[size + rhs->size + 6];
		uint32 offset=0;
		tmpbuffer[offset++]=Size();
		offset+=serialize(tmpbuffer+offset);
		tmpbuffer[offset++]=rhs->Size();
		offset+=rhs->serialize(tmpbuffer+offset);
		size=offset;
		delete[] pBuffer;
		pBuffer=tmpbuffer;
		opcode=OP_Combined;
		result=true;
	}

	return result;

}

uint32 EQProtocolPacket::Decompress(const unsigned char *buffer, const uint32 length, unsigned char *newbuf, uint32 newbufsize)
{
uint32 newlen=0;
uint32 flag_offset=0;
	newbuf[0]=buffer[0];
	if (buffer[0]==0x00) {
		flag_offset=2;
		newbuf[1]=buffer[1];
	} else
		flag_offset=1;

	if (length>2 && buffer[flag_offset]==0x5a) {
		newlen=InflatePacket(buffer+flag_offset+1,length-(flag_offset+1)-2,newbuf+flag_offset,newbufsize-flag_offset)+2;
		newbuf[newlen++]=buffer[length-2];
		newbuf[newlen++]=buffer[length-1];
	} else if (length>2 && buffer[flag_offset]==0xa5) {
		memcpy(newbuf+flag_offset,buffer+flag_offset+1,length-(flag_offset+1));
		newlen=length-1;
	} else {
		memcpy(newbuf,buffer,length);
		newlen=length;
	}

	return newlen;
}

uint32 EQProtocolPacket::Compress(const unsigned char *buffer, const uint32 length, unsigned char *newbuf, uint32 newbufsize) {
uint32 flag_offset=1,newlength;
	//dump_message_column(buffer,length,"Before: ");
	newbuf[0]=buffer[0];
	if (buffer[0]==0) {
		flag_offset=2;
		newbuf[1]=buffer[1];
	}
	if (length>30) {
		newlength=DeflatePacket(buffer+flag_offset,length-flag_offset,newbuf+flag_offset+1,newbufsize);
		*(newbuf+flag_offset)=0x5a;
		newlength+=flag_offset+1;
	} else {
		memmove(newbuf+flag_offset+1,buffer+flag_offset,length-flag_offset);
		*(newbuf+flag_offset)=0xa5;
		newlength=length+1;
	}
	//dump_message_column(newbuf,length,"After: ");

	return newlength;
}

EQApplicationPacket *EQApplicationPacket::Copy() const {
	return(new EQApplicationPacket(*this));
}

EQRawApplicationPacket *EQProtocolPacket::MakeAppPacket() const {
	auto res = new EQRawApplicationPacket(opcode, pBuffer, size);
	res->copyInfo(this);
	return(res);
}

EQRawApplicationPacket::EQRawApplicationPacket(uint16 opcode, const unsigned char *buf, const uint32 len)
:	EQApplicationPacket(OP_Unknown, buf, len),
	opcode(opcode)
{
}
EQRawApplicationPacket::EQRawApplicationPacket(const unsigned char *buf, const uint32 len)
: EQApplicationPacket(OP_Unknown, buf+sizeof(uint16), len-sizeof(uint16))
{
	if(GetExecutablePlatform() != ExePlatformUCS) {
		opcode = *((const uint16 *) buf);
		if(opcode == 0x0000)
		{
			if(len >= 3)
			{
				opcode = *((const uint16 *) (buf + 1));
				const unsigned char *packet_start = (buf + 3);
				const int32 packet_length = len - 3;
				safe_delete_array(pBuffer);
				if(packet_length >= 0)
				{
					size = packet_length;
					pBuffer = new unsigned char[size];
					memcpy(pBuffer, packet_start, size);
				}
				else
				{
					size = 0;
				}
			}
			else
			{
				safe_delete_array(pBuffer);
				size = 0;
			}
		}
	} else {
		opcode = *((const uint8 *) buf);
	}
}

void DumpPacket(const EQApplicationPacket* app, bool iShowInfo) {
	if (iShowInfo) {
		std::cout << "Dumping Applayer: 0x" << std::hex << std::setfill('0') << std::setw(4) << app->GetOpcode() << std::dec;
		std::cout << " size:" << app->size << std::endl;
	}
	DumpPacketHex(app->pBuffer, app->size);
//	DumpPacketAscii(app->pBuffer, app->size);
}

std::string DumpPacketToString(const EQApplicationPacket* app){
	std::ostringstream out;
	out << DumpPacketHexToString(app->pBuffer, app->size);
	return out.str();
}
