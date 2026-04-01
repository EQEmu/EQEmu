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
#include "struct_strategy.h"

#include "common/eq_stream_intf.h"
#include "common/eqemu_logsys.h"
#include "common/opcodemgr.h"

#include <map>
#include <memory>


//note: all encoders and decoders must be valid functions.
//so if you specify set_defaults=false
StructStrategy::StructStrategy() {
	int r;
	for(r = 0; r < _maxEmuOpcode; r++) {
		encoders[r] = PassEncoder;
		decoders[r] = PassDecoder;
	}
}

void StructStrategy::Encode(EQApplicationPacket **p, std::shared_ptr<EQStreamInterface> dest, bool ack_req) const {
	if((*p)->GetOpcodeBypass() != 0) {
		PassEncoder(p, dest, ack_req);
		return;
	}

	EmuOpcode op = (*p)->GetOpcode();
	Encoder proc = encoders[op];
	proc(p, dest, ack_req);
}

void StructStrategy::Decode(EQApplicationPacket *p) const {
	EmuOpcode op = p->GetOpcode();
	Decoder proc = decoders[op];
	proc(p);
}


void StructStrategy::ErrorEncoder(EQApplicationPacket **in_p, std::shared_ptr<EQStreamInterface> dest, bool ack_req) {
	EQApplicationPacket *p = *in_p;
	*in_p = nullptr;

	LogNetcode("[STRUCTS] Error encoding opcode [{}]: no encoder provided. Dropping", OpcodeManager::EmuToName(p->GetOpcode()));

	delete p;
}

void StructStrategy::ErrorDecoder(EQApplicationPacket *p) {
	LogNetcode("[STRUCTS] Error decoding opcode [{}]: no decoder provided. Invalidating", OpcodeManager::EmuToName(p->GetOpcode()));
	p->SetOpcode(OP_Unknown);
}

void StructStrategy::PassEncoder(EQApplicationPacket **p, std::shared_ptr<EQStreamInterface> dest, bool ack_req) {
	dest->FastQueuePacket(p, ack_req);
}

void StructStrategy::PassDecoder(EQApplicationPacket *p) {
	//do nothing since we decode in place
}




//effectively a singleton, but I decided to do it this way for no apparent reason.
namespace StructStrategyFactory {

	static std::map<EmuOpcode, const StructStrategy *> strategies;

	void RegisterPatch(EmuOpcode first_opcode, const StructStrategy *structs) {
		strategies[first_opcode] = structs;
	}

};
