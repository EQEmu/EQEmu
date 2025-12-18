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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "perlpacket.h"

#include "common/misc_functions.h"
#include "common/opcodemgr.h"
#include "zone/client.h"

PerlPacket::PerlPacket(const char *opcode, uint32 length) {
	SetOpcode(opcode);
	packet = nullptr;
	len = 0;
	Resize(length);
}

PerlPacket::~PerlPacket() {
	if(packet != nullptr)
		safe_delete_array(packet);
}

bool PerlPacket::SetOpcode(const char *opcode) {
	op = OP_Unknown;
//	op = ZoneOpcodeManager->NameSearch(opcode);
	return(op != OP_Unknown);
}

void PerlPacket::Resize(uint32 length) {
	Zero();
	if(len == length)
		return;
	if(packet != nullptr)
		safe_delete_array(packet);
	len = length;
	if(len == 0)
		packet = nullptr;
	else {
		packet = new unsigned char[len];
		Zero();
	}
}

//sending functions
void PerlPacket::SendTo(Client *who) {
	if(!who || op == OP_Unknown || (len > 0 && packet == nullptr))
		return;

	auto outapp = new EQApplicationPacket(op, len);
	if(len > 0)
		memcpy(outapp->pBuffer, packet, len);

	//	printf("Created this packet with PerlPacket: OP: %s\n", ZoneOpcodeManager->EmuToName(op));
	DumpPacket(outapp);

	who->FastQueuePacket(&outapp);
}

void PerlPacket::SendToAll() {
	if(op == OP_Unknown || (len > 0 && packet == nullptr))
		return;

	auto outapp = new EQApplicationPacket(op, len);
	if(len > 0)
		memcpy(outapp->pBuffer, packet, len);
	entity_list.QueueClients(nullptr, outapp, false);
	safe_delete(outapp);
}

//editing
void PerlPacket::Zero() {
	if(len == 0 || packet == nullptr)
		return;
	memset(packet, 0, len);
}

void PerlPacket::FromArray(int numbers[], uint32 length) {
	if(length == 0)
		return;
	Resize(length);
	uint32 r;
	for(r = 0; r < length; r++) {
		packet[r] = numbers[r] & 0xFF;
	}
}

void PerlPacket::SetByte(uint32 pos, uint8 val) {
	if(pos + sizeof(val) > len || packet == nullptr)
		return;
	uint8 *p = (uint8 *) (packet + pos);
	*p = val;
}

void PerlPacket::SetShort(uint32 pos, uint16 val) {
	if(pos + sizeof(val) > len || packet == nullptr)
		return;
	uint16 *p = (uint16 *) (packet + pos);
	*p = val;
}

void PerlPacket::SetLong(uint32 pos, uint32 val) {
	if(pos + sizeof(val) > len || packet == nullptr)
		return;
	uint32 *p = (uint32 *) (packet + pos);
	*p = val;
}

void PerlPacket::SetFloat(uint32 pos, float val) {
	if(pos + sizeof(val) > len || packet == nullptr)
		return;
	float *p = (float *) (packet + pos);
	*p = val;
}

void PerlPacket::SetString(uint32 pos, char *str) {
	int slen = strlen(str);
	if(pos + slen > len || packet == nullptr)
		return;
	strcpy((char *)(packet+pos), str);
}

#pragma pack(1)
struct EQ1319 {
	int32 part13:13, part19:19;
};
struct EQ1913 {
	int32 part19:19, part13:13;
};
#pragma pack()

void PerlPacket::SetEQ1319(uint32 pos, float part13, float part19) {
	if(pos + sizeof(EQ1319) > len || packet == nullptr)
		return;
	EQ1319 *p = (EQ1319 *) (packet + pos);
	p->part19 = FloatToEQ19(part19);
	p->part13 = FloatToEQ13(part13);
}

void PerlPacket::SetEQ1913(uint32 pos, float part19, float part13) {
	if(pos + sizeof(EQ1913) > len || packet == nullptr)
		return;
	EQ1913 *p = (EQ1913 *) (packet + pos);
	p->part19 = FloatToEQ19(part19);
	p->part13 = FloatToEQ13(part13);
}

//reading
uint8 PerlPacket::GetByte(uint32 pos) {
	if(pos + sizeof(uint8) > len || packet == nullptr)
		return(0);
	uint8 *p = (uint8 *) (packet + pos);
	return(*p);
}

uint16 PerlPacket::GetShort(uint32 pos) {
	if(pos + sizeof(uint16) > len || packet == nullptr)
		return(0);
	uint16 *p = (uint16 *) (packet + pos);
	return(*p);
}

uint32 PerlPacket::GetLong(uint32 pos) {
	if(pos + sizeof(uint32) > len || packet == nullptr)
		return(0);
	uint32 *p = (uint32 *) (packet + pos);
	return(*p);
}

float PerlPacket::GetFloat(uint32 pos) {
	if(pos + sizeof(float) > len || packet == nullptr)
		return(0);
	float *p = (float *) (packet + pos);
	return(*p);
}

