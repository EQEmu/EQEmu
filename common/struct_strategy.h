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
#include "common/emu_versions.h"

#include <memory>
#include <string>


class EQApplicationPacket;
class EQStreamInterface;

class StructStrategy {
public:
	//the encoder takes ownership of the supplied packet, and may enqueue multiple resulting packets into the stream
	typedef void(*Encoder)(EQApplicationPacket **p, std::shared_ptr<EQStreamInterface> dest, bool ack_req);
	//the decoder may only edit the supplied packet, producing a single packet for eqemu to consume.
	typedef void (*Decoder)(EQApplicationPacket *p);

	StructStrategy();
	virtual ~StructStrategy() {}

	//this method takes an eqemu struct, and enqueues the produced structs into the stream.
	void Encode(EQApplicationPacket **p, std::shared_ptr<EQStreamInterface> dest, bool ack_req) const;
	//this method takes an EQ wire struct, and converts it into an eqemu struct
	void Decode(EQApplicationPacket *p) const;

	virtual std::string Describe() const = 0;
	virtual const EQ::versions::ClientVersion ClientVersion() const = 0;

protected:
	//some common coders:
	//Print an error saying unknown struct/opcode and drop it
	static void ErrorEncoder(EQApplicationPacket **p, std::shared_ptr<EQStreamInterface> dest, bool ack_req);
	static void ErrorDecoder(EQApplicationPacket *p);
	//pass the packet through without modification (emu == EQ) (default)
	static void PassEncoder(EQApplicationPacket **p, std::shared_ptr<EQStreamInterface> dest, bool ack_req);
	static void PassDecoder(EQApplicationPacket *p);

	Encoder encoders[_maxEmuOpcode];
	Decoder decoders[_maxEmuOpcode];
};

//effectively a singleton, but I decided to do it this way for no apparent reason.
namespace StructStrategyFactory {
	void RegisterPatch(EmuOpcode first_opcode, const StructStrategy *structs);
};
