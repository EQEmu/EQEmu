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

//this is the highest opcode possibly used in the regular EQ protocol
#define MAX_EQ_OPCODE 0xFFFF


/*


the list of opcodes is in emu_oplist.h

we somewhat rely on the fact that we have more than 255 opcodes,
so we know the enum type for the opcode defines must be at least
16 bits, so we can use the protocol flags on them.

*/

typedef enum { //EQEmu internal opcodes list
	OP_Unknown=0,

//a preprocessor hack so we dont have to maintain two lists
#define N(x) x
	#include "emu_oplist.h"
	#include "mail_oplist.h"
#undef N

	_maxEmuOpcode
} EmuOpcode;

constexpr int format_as(EmuOpcode opcode) { return static_cast<int>(opcode); }
extern const char *OpcodeNames[_maxEmuOpcode+1];
