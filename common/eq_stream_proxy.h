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

#include "common/eq_stream_intf.h"
#include "common/types.h"

#include <memory>

class StructStrategy;
class OpcodeManager;
class EQApplicationPacket;

class EQStreamProxy : public EQStreamInterface {
public:
	//takes ownership of the stream.
	EQStreamProxy(std::shared_ptr<EQStreamInterface> &stream, const StructStrategy *structs, OpcodeManager **opcodes);
	virtual ~EQStreamProxy();

	//EQStreamInterface:
	virtual void QueuePacket(const EQApplicationPacket *p, bool ack_req=true);
	virtual void FastQueuePacket(EQApplicationPacket **p, bool ack_req=true);
	virtual EQApplicationPacket *PopPacket();
	virtual void Close();
	virtual std::string GetRemoteAddr() const;
	virtual uint32 GetRemoteIP() const;
	virtual uint16 GetRemotePort() const;
	virtual void ReleaseFromUse();
	virtual void RemoveData();
	virtual bool CheckState(EQStreamState state);
	virtual std::string Describe() const;
	virtual const EQ::versions::ClientVersion ClientVersion() const;
	virtual EQStreamState GetState();
	virtual void SetOpcodeManager(OpcodeManager **opm);
	virtual Stats GetStats() const;
	virtual void ResetStats();
	virtual EQStreamManagerInterface* GetManager() const;
	virtual OpcodeManager* GetOpcodeManager() const;

protected:
	std::shared_ptr<EQStreamInterface> const m_stream;	//we own this stream object.
	const StructStrategy *const	m_structs;	//we do not own this object.
	//this is a pointer to a pointer to make it less likely that a packet will
	//reference an invalid opcode manager when they are being reloaded.
	OpcodeManager **const			m_opcodes;
	//we do not own this object.
};
