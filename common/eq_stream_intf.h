#pragma once

#include "common/emu_versions.h"
#include "common/eq_packet.h"
#include "common/net/reliable_stream_connection.h"

#include <string>

//this is the only part of an EQStream that is seen by the application.


typedef enum {
	ESTABLISHED,
	CLOSING,		//waiting for pending data to flush.
	DISCONNECTING,	//have sent disconnect, waiting for their disconnect reply.
	CLOSED,			//received a disconnect from remote side.
	UNESTABLISHED
} EQStreamState;

class EQApplicationPacket;
class OpcodeManager;

struct EQStreamManagerInterfaceOptions
{
	EQStreamManagerInterfaceOptions() {
		opcode_size = 2;
	}

	EQStreamManagerInterfaceOptions(int port, bool encoded, bool compressed) {
		opcode_size = 2;

		//World seems to support both compression and xor zone supports one or the others.
		//Enforce one or the other in the convienence construct
		//Login I had trouble getting to recognize compression at all
		//but that might be because it was still a bit buggy when i was testing that.
		if (compressed) {
			reliable_stream_options.encode_passes[0] = EQ::Net::EncodeCompression;
		}
		else if (encoded) {
			reliable_stream_options.encode_passes[0] = EQ::Net::EncodeXOR;
		}

		reliable_stream_options.port = port;
	}

	int opcode_size;
	bool track_opcode_stats;
	EQ::Net::ReliableStreamConnectionManagerOptions reliable_stream_options;
};

class EQStreamManagerInterface
{
public:
	EQStreamManagerInterface(const EQStreamManagerInterfaceOptions &options) { m_options = options; }
	virtual ~EQStreamManagerInterface() { };

	EQStreamManagerInterfaceOptions GetOptions() { return m_options; }
	const EQStreamManagerInterfaceOptions& GetOptions() const { return m_options; }
	virtual void SetOptions(const EQStreamManagerInterfaceOptions& options) = 0;
protected:
	EQStreamManagerInterfaceOptions m_options;
};

class EQStreamInterface {
public:
	virtual ~EQStreamInterface() {}

	class Signature {
	public:
		//this object could get more complicated if needed...
		uint16 ignore_eq_opcode;		//0=dont ignore
		uint16 first_eq_opcode;
		uint32 first_length;			//0=dont check length
	};

	typedef enum {
		MatchNotReady,
		MatchSuccessful,
		MatchFailed
	} MatchState;

	struct Stats
	{
		EQ::Net::ReliableStreamConnectionStats ReliableStreamStats;
		int RecvCount[_maxEmuOpcode];
		int SentCount[_maxEmuOpcode];
	};

	virtual void QueuePacket(const EQApplicationPacket *p, bool ack_req=true) = 0;
	virtual void FastQueuePacket(EQApplicationPacket **p, bool ack_req=true) = 0;
	virtual EQApplicationPacket *PopPacket() = 0;
	virtual void Close() = 0;
	virtual void ReleaseFromUse() = 0;
	virtual void RemoveData() = 0;
	virtual std::string GetRemoteAddr() const = 0;
	virtual uint32 GetRemoteIP() const = 0;
	virtual uint16 GetRemotePort() const = 0;
	virtual bool CheckState(EQStreamState state) = 0;
	virtual std::string Describe() const = 0;
	virtual void SetActive(bool val) { }
	virtual MatchState CheckSignature(const Signature *sig) { return MatchFailed; }
	virtual EQStreamState GetState() = 0;
	virtual void SetOpcodeManager(OpcodeManager **opm) = 0;
	virtual OpcodeManager* GetOpcodeManager() const = 0;
	virtual const EQ::versions::ClientVersion ClientVersion() const { return EQ::versions::ClientVersion::Unknown; }
	virtual Stats GetStats() const = 0;
	virtual void ResetStats() = 0;
	virtual EQStreamManagerInterface* GetManager() const = 0;
};
