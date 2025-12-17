/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2005 EQEMu Development Team (http://eqemulator.net)

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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 04111-1307 USA
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
