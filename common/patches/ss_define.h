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
#define ENCODE(x) void Strategy::Encode_##x(EQApplicationPacket **p, std::shared_ptr<EQStreamInterface> dest, bool ack_req)
#define DECODE(x) void Strategy::Decode_##x(EQApplicationPacket *__packet)

#define StructDist(in, f1, f2) (uint32(&in->f2)-uint32(&in->f1))
#define CopyBlock(to, field, from, field1, field2) \
	memcpy((void *) &to->field, (const void *) from->field1, StructDist(from, field1, field2));
#define CopyLen(to, field, from, field1, len) \
	memcpy((void *) &to->field, (const void *) from->field1, len);


/*
 *
 * for encoders
 *
 */
//more complex operations and variable length packets
#define FASTQUEUE(packet) dest->FastQueuePacket(&packet, ack_req);
#define TAKE(packet_name) \
	EQApplicationPacket *packet_name = *p; \
	*p = nullptr;

//simple buffer-to-buffer movement for fixed length packets
//the eq packet is mapped into `eq`, the emu packet into `emu`
#define SETUP_DIRECT_ENCODE(emu_struct, eq_struct) \
	SETUP_VAR_ENCODE(emu_struct); \
	ALLOC_VAR_ENCODE(eq_struct, sizeof(eq_struct));

//like a direct encode, but for variable length packets (two stage)
#define SETUP_VAR_ENCODE(emu_struct) \
	EQApplicationPacket *__packet = *p; \
	*p = nullptr; \
	unsigned char *__emu_buffer = __packet->pBuffer; \
	emu_struct *emu = (emu_struct *) __emu_buffer; \
	uint32 __i = 0; \
	__i++; /* to shut up compiler */

#define ALLOC_VAR_ENCODE(eq_struct, len) \
	__packet->pBuffer = new unsigned char[len]; \
	__packet->size = len; \
	memset(__packet->pBuffer, 0, len); \
	eq_struct *eq = (eq_struct *) __packet->pBuffer; \

#define ALLOC_LEN_ENCODE(len) \
	__packet->pBuffer = new unsigned char[len] {}; \
	__packet->size = len; \
	memset(__packet->pBuffer, 0, len); \

//a shorter assignment for direct mode
#undef OUT
#define OUT(x) eq->x = emu->x;
#define OUT_str(x) \
	strncpy(eq->x, emu->x, sizeof(eq->x)); \
        eq->x[sizeof(eq->x)-1] = '\0';
#define OUT_array(x, n) \
	for(__i = 0; __i < n; __i++) \
		eq->x[__i] = emu->x[__i];

//call before any premature returns in an encoder using SETUP_DIRECT_ENCODE
#define FAIL_ENCODE() \
	delete[] __emu_buffer; \
	delete __packet;

//call to finish an encoder using SETUP_DIRECT_ENCODE
#define FINISH_ENCODE() \
	delete[] __emu_buffer; \
	dest->FastQueuePacket(&__packet, ack_req);

//check length of packet before decoding. Call before setup.
#define ENCODE_LENGTH_EXACT(struct_) \
	if((*p)->size != sizeof(struct_)) { \
		LogNetcode("Wrong size on outbound [{}] (" #struct_ "): Got [{}], expected [{}]", opcodes->EmuToName((*p)->GetOpcode()), (*p)->size, sizeof(struct_)); \
		delete *p; \
		*p = nullptr; \
		return; \
	}
#define ENCODE_LENGTH_ATLEAST(struct_) \
	if((*p)->size < sizeof(struct_)) { \
		LogNetcode("Wrong size on outbound [{}] (" #struct_ "): Got [{}], expected at least [{}]", opcodes->EmuToName((*p)->GetOpcode()), (*p)->size, sizeof(struct_)); \
		delete *p; \
		*p = nullptr; \
		return; \
	}

//forward this opcode to another encoder
#define ENCODE_FORWARD(other_op) \
	Encode_##other_op(p, dest, ack_req);

//destroy the packet, it is not sent to this client version
#define EAT_ENCODE(op) \
	ENCODE(op) { \
	delete *p; \
	*p = nullptr; \
	}



/*
 *
 * for decoders:
 *
 */

//simple buffer-to-buffer movement for fixed length packets
//the eq packet is mapped into `eq`, the emu packet into `emu`
#define SETUP_DIRECT_DECODE(emu_struct, eq_struct) \
	unsigned char *__eq_buffer = __packet->pBuffer; \
	__packet->size = sizeof(emu_struct); \
	__packet->pBuffer = new unsigned char[__packet->size] {}; \
	emu_struct *emu = (emu_struct *) __packet->pBuffer; \
	eq_struct *eq = (eq_struct *) __eq_buffer;

#define SETUP_VAR_DECODE(emu_struct, eq_struct, var_field) \
	unsigned char *__eq_buffer = __packet->pBuffer; \
	eq_struct* in = (eq_struct*)__packet->pBuffer; \
	auto size = strlen(in->var_field); \
	__packet->size = sizeof(emu_struct) + size + 1; \
	__packet->pBuffer = new unsigned char[__packet->size] {}; \
	emu_struct *emu = (emu_struct *) __packet->pBuffer; \
	eq_struct *eq = (eq_struct *) __eq_buffer;

#define MEMSET_IN(emu_struct) \
	memset(__packet->pBuffer, 0, sizeof(emu_struct));

//a shorter assignment for direct mode
#undef IN
#define IN(x) emu->x = eq->x;
#define IN_str(x) \
	strncpy(emu->x, eq->x, sizeof(emu->x)); \
        emu->x[sizeof(emu->x)-1] = '\0';
#define IN_array(x, n) \
	for(__i = 0; __i < n; __i++) \
		emu->x[__i] = eq->x[__i];

//call before any premature returns in an encoder using SETUP_DIRECT_DECODE
#define FAIL_DIRECT_DECODE() \
	delete[] __eq_buffer; \
	p->SetOpcode(OP_Unknown);

#define FINISH_VAR_DECODE() \
	delete[] __eq_buffer; 

//call to finish an encoder using SETUP_DIRECT_DECODE
#define FINISH_DIRECT_DECODE() \
	delete[] __eq_buffer;

//check length of packet before decoding. Call before setup.
#define DECODE_LENGTH_EXACT(struct_) \
	if(__packet->size != sizeof(struct_)) { \
		LogNetcode("Wrong size on incoming [{}] (" #struct_ "): Got [{}], expected [{}]", opcodes->EmuToName(__packet->GetOpcode()), __packet->size, sizeof(struct_)); \
		__packet->SetOpcode(OP_Unknown); /* invalidate the packet */ \
		return; \
	}
#define DECODE_LENGTH_ATLEAST(struct_) \
	if(__packet->size < sizeof(struct_)) { \
		LogNetcode("Wrong size on incoming [{}] (" #struct_ "): Got [{}], expected at least [{}]", opcodes->EmuToName(__packet->GetOpcode()), __packet->size, sizeof(struct_)); \
		__packet->SetOpcode(OP_Unknown); /* invalidate the packet */ \
		return; \
	}

//forward this opcode to another decoder
#define DECODE_FORWARD(other_op) \
	Decode_##other_op(__packet);
