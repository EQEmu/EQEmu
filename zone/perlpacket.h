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

#include "common/emu_opcodes.h"
#include "common/types.h"

#include <string>
#include <vector>

class Client;

class PerlPacket {
public:
	PerlPacket(const char *opcode = "OP_Unknown", uint32 len = 0);
	~PerlPacket();

	bool SetOpcode(const char *opcode);
	void Resize(uint32 len);

	//sending functions
	void SendTo(Client *who);
	void SendToAll();

	//editing
	void Zero();
	void FromArray(int numbers[], uint32 length);
	void SetByte(uint32 pos, uint8 val);
	void SetShort(uint32 pos, uint16 val);
	void SetLong(uint32 pos, uint32 val);
	void SetFloat(uint32 pos, float val);
	void SetString(uint32 pos, char *str);

	void SetEQ1319(uint32 pos, float part13, float part19);
	void SetEQ1913(uint32 pos, float part19, float part13);

	//reading
	uint8 GetByte(uint32 pos);
	uint16 GetShort(uint32 pos);
	uint32 GetLong(uint32 pos);
	float GetFloat(uint32 pos);

protected:
	EmuOpcode op;
	uint32 len;
	unsigned char *packet;
};
