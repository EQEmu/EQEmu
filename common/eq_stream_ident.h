#pragma once

#include "common/eq_stream_intf.h"
#include "common/timer.h"

#include <memory>
#include <queue>
#include <vector>

#define STREAM_IDENT_WAIT_MS 30000

class OpcodeManager;
class StructStrategy;
class EQStreamInterface;

class EQStreamIdentifier {
public:
	~EQStreamIdentifier();

	//registration interface.
	void RegisterPatch(EQStreamInterface::Signature sig, const char *name, OpcodeManager ** opcodes, const StructStrategy *structs);

	//main processing interface
	void Process();
	void AddStream(std::shared_ptr<EQStreamInterface> eqs);
	EQStreamInterface *PopIdentified();

protected:

	//registered patches..
	class Patch {
	public:
		std::string				name;
		EQStreamInterface::Signature		signature;
		OpcodeManager **		opcodes;
		const StructStrategy *structs;
	};
	std::vector<Patch *> m_patches;	//we own these objects.

	//pending streams..
	class Record {
	public:
		Record(std::shared_ptr<EQStreamInterface> s);
		std::shared_ptr<EQStreamInterface> stream;		//we own this
		Timer expire;
	};
	std::vector<Record> m_streams;	//we own these objects, and the streams contained in them.
	std::queue<EQStreamInterface *> m_identified;	//we own these objects
};
