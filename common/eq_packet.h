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

#pragma once

#include "common/base_packet.h"
#include "common/platform.h"

#ifdef STATIC_OPCODE
	typedef unsigned short EmuOpcode;
	static const EmuOpcode OP_Unknown = 0;
#else
#include "common/emu_opcodes.h"
#endif

class EQPacket : public BasePacket {
	friend class EQStream;
public:
	virtual ~EQPacket() {}

	uint32 Size() const { return size+2; }

	virtual void build_raw_header_dump(char *buffer, uint16 seq=0xffff) const;
	virtual void build_header_dump(char *buffer) const;
	virtual void DumpRawHeader(uint16 seq=0xffff, FILE *to = stdout) const;
	virtual void DumpRawHeaderNoTime(uint16 seq=0xffff, FILE *to = stdout) const;

	void SetOpcode(EmuOpcode op) { emu_opcode = op; }
	const EmuOpcode GetOpcode() const { return(emu_opcode); }
//	const char *GetOpcodeName() const;

protected:
	//this is just a cache so we dont look it up several times on Get()
	//and it is mutable so we can store the cached copy even on a const object
	EmuOpcode emu_opcode;

	EQPacket(EmuOpcode opcode, const unsigned char *buf, const uint32 len);
	EQPacket(EmuOpcode opcode, SerializeBuffer &buf) : BasePacket(buf), emu_opcode(opcode) { };
//	EQPacket(const EQPacket &p) { }
	EQPacket() { emu_opcode=OP_Unknown; pBuffer=nullptr; size=0; }

};

class EQRawApplicationPacket;

class EQProtocolPacket : public BasePacket {
	friend class EQStream;
	friend class EQStreamPair;
public:
	EQProtocolPacket(uint16 op, const unsigned char *buf, uint32 len) : BasePacket(buf, len), opcode(op) { acked = false; sent_time = 0; }
//	EQProtocolPacket(const unsigned char *buf, uint32 len);
	bool combine(const EQProtocolPacket *rhs);
	uint32 serialize (unsigned char *dest) const;
	EQProtocolPacket *Copy() { return new EQProtocolPacket(opcode,pBuffer,size); }
	EQRawApplicationPacket *MakeAppPacket() const;

	bool acked;
	uint32 sent_time;

	virtual void build_raw_header_dump(char *buffer, uint16 seq=0xffff) const;
	virtual void build_header_dump(char *buffer) const;
	virtual void DumpRawHeader(uint16 seq=0xffff, FILE *to = stdout) const;
	virtual void DumpRawHeaderNoTime(uint16 seq=0xffff, FILE *to = stdout) const;

protected:

	static uint32 Decompress(const unsigned char *buffer, const uint32 length, unsigned char *newbuf, uint32 newbufsize);
	static uint32 Compress(const unsigned char *buffer, const uint32 length, unsigned char *newbuf, uint32 newbufsize);

	uint16 GetRawOpcode() const { return(opcode); }

	uint32 Size() const { return size+2; }

	//the actual raw EQ opcode
	uint16 opcode;
};

class EQApplicationPacket : public EQPacket {
	friend class EQStream;
public:
	EQApplicationPacket() : EQPacket(OP_Unknown, nullptr, 0), opcode_bypass(0)
		{ app_opcode_size = GetExecutablePlatform() == ExePlatformUCS ? 1 : 2; }
	EQApplicationPacket(const EmuOpcode op) : EQPacket(op, nullptr, 0), opcode_bypass(0)
		{ app_opcode_size = GetExecutablePlatform() == ExePlatformUCS ? 1 : 2; }
	EQApplicationPacket(const EmuOpcode op, const uint32 len) : EQPacket(op, nullptr, len), opcode_bypass(0)
		{ app_opcode_size = GetExecutablePlatform() == ExePlatformUCS ? 1 : 2; }
	EQApplicationPacket(const EmuOpcode op, const unsigned char *buf, const uint32 len) : EQPacket(op, buf, len), opcode_bypass(0)
		{ app_opcode_size = GetExecutablePlatform() == ExePlatformUCS ? 1 : 2; }
	EQApplicationPacket(const EmuOpcode op, SerializeBuffer &buf) : EQPacket(op, buf), opcode_bypass(0)
		{ app_opcode_size = GetExecutablePlatform() == ExePlatformUCS ? 1 : 2; }
	bool combine(const EQApplicationPacket *rhs);
	uint32 serialize (uint16 opcode, unsigned char *dest) const;
	uint32 Size() const { return size+app_opcode_size; }

	virtual EQApplicationPacket *Copy() const;

	virtual void build_raw_header_dump(char *buffer, uint16 seq=0xffff) const;
	virtual void build_header_dump(char *buffer) const;
	virtual void DumpRawHeader(uint16 seq=0xffff, FILE *to = stdout) const;
	virtual void DumpRawHeaderNoTime(uint16 seq=0xffff, FILE *to = stdout) const;

	uint16 GetOpcodeBypass() const { return opcode_bypass; }
	void SetOpcodeBypass(uint16 v) { opcode_bypass = v; }

	uint16 GetProtocolOpcode() const { return protocol_opcode; }
	void SetProtocolOpcode(uint16 v) { protocol_opcode = v; }
protected:

	uint16 protocol_opcode;
	uint8 app_opcode_size;
	uint16 opcode_bypass;
private:

	EQApplicationPacket(const EQApplicationPacket &p) : EQPacket(p.emu_opcode, p.pBuffer, p.size), opcode_bypass(p.opcode_bypass) { app_opcode_size = p.app_opcode_size; }

};

class EQRawApplicationPacket : public EQApplicationPacket {
	friend class EQStream;
public:
	EQRawApplicationPacket(uint16 opcode, const unsigned char *buf, const uint32 len);
	uint16 GetRawOpcode() const { return(opcode); }

	virtual void build_raw_header_dump(char *buffer, uint16 seq=0xffff) const;
	virtual void build_header_dump(char *buffer) const;
	virtual void DumpRawHeader(uint16 seq=0xffff, FILE *to = stdout) const;
	virtual void DumpRawHeaderNoTime(uint16 seq=0xffff, FILE *to = stdout) const;

protected:

	//the actual raw EQ opcode
	uint16 opcode;

	EQRawApplicationPacket(const unsigned char *buf, const uint32 len);
};

extern void DumpPacket(const EQApplicationPacket* app, bool iShowInfo = false);
extern std::string DumpPacketToString(const EQApplicationPacket* app);
