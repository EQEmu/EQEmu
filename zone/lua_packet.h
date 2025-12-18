#pragma once

#ifdef LUA_EQEMU

#include "common/types.h"
#include "zone/lua_ptr.h"

class EQApplicationPacket;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_packet();
luabind::scope lua_register_packet_opcodes();

class Lua_Packet : public Lua_Ptr<EQApplicationPacket>
{
	typedef EQApplicationPacket NativeType;
public:
	Lua_Packet() : Lua_Ptr(nullptr), owned_(false) { }
	Lua_Packet(EQApplicationPacket *d) : Lua_Ptr(d), owned_(false) { }
	Lua_Packet(int opcode, int size);
	Lua_Packet(int opcode, int size, bool raw);
	Lua_Packet& operator=(const Lua_Packet& o);
	Lua_Packet(const Lua_Packet& o);
	virtual ~Lua_Packet() { if(owned_) { EQApplicationPacket *ptr = GetLuaPtrData(); if(ptr) { delete ptr; } } }

	int GetSize();
	int GetOpcode();
	void SetOpcode(int op);
	int GetRawOpcode();
	int GetProtocolOpcode();
	void SetRawOpcode(int op);
	int GetWritePosition();
	void SetWritePosition(int offset);
	void WriteInt8(int value);
	void WriteInt8(int offset, int value);
	void WriteInt16(int value);
	void WriteInt16(int offset, int value);
	void WriteInt32(int value);
	void WriteInt32(int offset, int value);
	void WriteInt64(int64 value);
	void WriteInt64(int offset, int64 value);
	void WriteFloat(float value);
	void WriteFloat(int offset, float value);
	void WriteDouble(double value);
	void WriteDouble(int offset, double value);
	void WriteString(std::string value);
	void WriteString(int offset, std::string value);
	void WriteFixedLengthString(std::string value);
	void WriteFixedLengthString(int offset, std::string value, int string_length);
	int ReadInt8(int offset);
	int ReadInt16(int offset);
	int ReadInt32(int offset);
	int64 ReadInt64(int offset);
	float ReadFloat(int offset);
	double ReadDouble(int offset);
	std::string ReadString(int offset);
	std::string ReadFixedLengthString(int offset, int string_length);

	operator EQApplicationPacket*() {
		return reinterpret_cast<EQApplicationPacket*>(GetLuaPtrData());
	}

private:
	bool owned_;
};

#endif // LUA_EQEMU

