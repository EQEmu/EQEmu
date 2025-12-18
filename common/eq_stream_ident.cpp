
#include "common/eq_stream_ident.h"
#include "common/eq_stream_proxy.h"
#include "common/eqemu_logsys.h"
#include "common/misc.h"

#include <utility>

EQStreamIdentifier::~EQStreamIdentifier() {
	while(!m_identified.empty()) {
		m_identified.front()->ReleaseFromUse();
		m_identified.pop();
	}
	std::vector<Record>::iterator cur, end;
	cur = m_streams.begin();
	end = m_streams.end();
	for(; cur != end; ++cur) {
		Record &r = *cur;
		r.stream->ReleaseFromUse();
	}
	std::vector<Patch *>::iterator curp, endp;
	curp = m_patches.begin();
	endp = m_patches.end();
	for(; curp != endp; ++curp) {
		delete *curp;
	}
}

void EQStreamIdentifier::RegisterPatch(EQStreamInterface::Signature sig, const char *name, OpcodeManager ** opcodes, const StructStrategy *structs) {
	auto p = new Patch;
	p->signature = sig;
	p->name = name;
	p->opcodes = opcodes;
	p->structs = structs;
	m_patches.push_back(p);
}

void EQStreamIdentifier::Process() {
	std::vector<Record>::iterator cur;
	std::vector<Patch *>::iterator curp, endp;

	//foreach pending stream.
	cur = m_streams.begin();
	while(cur != m_streams.end()) {
		Record &r = *cur;

		//first see if this stream has expired
		if(r.expire.Check(false)) {
			LogNetcode("[StreamIdentify] Unable to identify stream from [{}:{}] before timeout", r.stream->GetRemoteAddr().c_str(), ntohs(r.stream->GetRemotePort()));
			r.stream->Close();

			cur = m_streams.erase(cur);
			continue;
		}

		//then make sure the stream is still active
		//if stream hasn't finished initializing then continue;
		if(r.stream->GetState() == UNESTABLISHED)
		{
			++cur;
			continue;
		}
		if(r.stream->GetState() != ESTABLISHED) {
			//the stream closed before it was identified.
			LogNetcode("[StreamIdentify] Unable to identify stream from [{}:{}] before it closed", long2ip(r.stream->GetRemoteIP()).c_str(), ntohs(r.stream->GetRemotePort()));
			switch(r.stream->GetState())
			{
			case ESTABLISHED:
				LogNetcode("[StreamIdentify] Stream state was Established");
				break;
			case CLOSING:
				LogNetcode("[StreamIdentify] Stream state was Closing");
				break;
			case DISCONNECTING:
				LogNetcode("[StreamIdentify] Stream state was Disconnecting");
				break;
			case CLOSED:
				LogNetcode("[StreamIdentify] Stream state was Closed");
				break;
			default:
				LogNetcode("[StreamIdentify] Stream state was Unestablished or unknown");
				break;
			}
			r.stream->ReleaseFromUse();
			cur = m_streams.erase(cur);
			continue;
		}

		//not expired, check against all patch signatures

		bool found_one = false;		//"we found a matching patch for this stream"
		bool all_ready = true;		//"all signatures were ready to check the stream"

		//foreach possbile patch...
		curp = m_patches.begin();
		endp = m_patches.end();
		for(; !found_one && curp != endp; ++curp) {
			Patch *p = *curp;

			//ask the stream to see if it matches the supplied signature
			EQStreamInterface::MatchState res = r.stream->CheckSignature(&p->signature);
			switch(res) {
			case EQStreamInterface::MatchNotReady:
				//the stream has not received enough packets to compare with this signature
//				Log.LogDebugType(Logs::General, Logs::Netcode, "[StreamIdentify] %s:%d: Tried patch %s, but stream is not ready for it.", long2ip(r.stream->GetRemoteIP()).c_str(), ntohs(r.stream->GetRemotePort()), p->name.c_str());
				all_ready = false;
				break;
			case EQStreamInterface::MatchSuccessful: {
				//yay, a match.

				LogNetcode("[StreamIdentify] Identified stream [{}:{}] with signature [{}]", long2ip(r.stream->GetRemoteIP()).c_str(), ntohs(r.stream->GetRemotePort()), p->name.c_str());

				// before we assign the eqstream to an interface, let the stream recognize it is in use and the session should not be reset any further
				r.stream->SetActive(true);

				//might want to do something less-specific here... some day..
				EQStreamInterface *s = new EQStreamProxy(r.stream, p->structs, p->opcodes);
				m_identified.push(s);

				found_one = true;
				break;
			}
			case EQStreamInterface::MatchFailed:
				//do nothing...
				LogNetcode("[StreamIdentify] [{}:{}] Tried patch [{}] and it did not match", long2ip(r.stream->GetRemoteIP()).c_str(), ntohs(r.stream->GetRemotePort()), p->name.c_str());
				break;
			}
		}

		//if we checked all patches and did not find a match.
		if(all_ready && !found_one) {
			//the stream cannot be identified.
			LogNetcode("[StreamIdentify] Unable to identify stream from [{}:{}], no match found", long2ip(r.stream->GetRemoteIP()).c_str(), ntohs(r.stream->GetRemotePort()));
			r.stream->ReleaseFromUse();
		}

		//if we found a match, or were not able to identify it
		if(found_one || all_ready) {
			//cannot print ip/port here. r.stream is invalid.
			cur = m_streams.erase(cur);
		} else {
			++cur;
		}
	}	//end foreach stream
}

void EQStreamIdentifier::AddStream(std::shared_ptr<EQStreamInterface> eqs) {
	m_streams.emplace_back(Record(eqs));
	eqs = nullptr;
}

EQStreamInterface *EQStreamIdentifier::PopIdentified() {
	if(m_identified.empty())
		return(nullptr);
	EQStreamInterface *res = m_identified.front();
	m_identified.pop();
	return(res);
}

EQStreamIdentifier::Record::Record(std::shared_ptr<EQStreamInterface> s)
:	stream(std::move(s)),
	expire(STREAM_IDENT_WAIT_MS)
{
}
