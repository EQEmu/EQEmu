/*
	Copyright (C) 2005 Michael S. Finger

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

#include "eq_stream.h"

#include "common/crc16.h"
#include "common/eq_packet.h"
#include "common/eqemu_logsys.h"
#include "common/global_define.h"
#include "common/op_codes.h"
#include "common/platform.h"
#include "common/strings.h"

#include <algorithm>
#include <iomanip>
#include <string>
#include <vector>

#ifdef _WINDOWS
#include <time.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <arpa/inet.h>
#endif

//for logsys
#define _L "%s:%d: "
#define __L , long2ip(remote_ip).c_str(), ntohs(remote_port)

uint16 EQStream::MaxWindowSize=2048;

void EQStream::init(bool resetSession) {
	// we only reset these statistics if it is a 'new' connection
	if ( resetSession )
	{
		streamactive = false;
		sessionAttempts = 0;
	}
	active_users = 0;
	Session=0;
	Key=0;
	MaxLen=0;
	NextInSeq=0;
	NextOutSeq=0;
	NextAckToSend=-1;
	LastAckSent=-1;
	MaxSends=5;
	LastPacket=0;
	oversize_buffer=nullptr;
	oversize_length=0;
	oversize_offset=0;
	RateThreshold=RATEBASE/250;
	DecayRate=DECAYBASE/250;
	BytesWritten=0;
	sent_packet_count = 0;
	received_packet_count = 0;
	SequencedBase = 0;
	AverageDelta = 500;

	if(GetExecutablePlatform() == ExePlatformWorld || GetExecutablePlatform() == ExePlatformZone) {
		retransmittimer = Timer::GetCurrentTime();
		retransmittimeout = 500 * RETRANSMIT_TIMEOUT_MULT;
	}

	OpMgr = nullptr;
	if(uint16(SequencedBase + SequencedQueue.size()) != NextOutSeq) {
		LogNetcode(_L "init Invalid Sequenced queue: BS [{}] + SQ [{}] != NOS [{}]" __L, SequencedBase, SequencedQueue.size(), NextOutSeq);
	}
}

EQRawApplicationPacket *EQStream::MakeApplicationPacket(EQProtocolPacket *p)
{
	EQRawApplicationPacket *ap=nullptr;
	LogNetcode(_L "Creating new application packet, length [{}]" __L, p->size);
	// _raw(NET__APP_CREATE_HEX, 0xFFFF, p);
	ap = p->MakeAppPacket();
	return ap;
}

EQRawApplicationPacket *EQStream::MakeApplicationPacket(const unsigned char *buf, uint32 len)
{
	EQRawApplicationPacket *ap=nullptr;
	LogNetcode(_L "Creating new application packet, length [{}]" __L, len);
	ap = new EQRawApplicationPacket(buf, len);
	return ap;
}

EQProtocolPacket *EQStream::MakeProtocolPacket(const unsigned char *buf, uint32 len) {
	uint16 proto_opcode = ntohs(*(const uint16 *)buf);

	//advance over opcode.
	buf += 2;
	len -= 2;

	return(new EQProtocolPacket(proto_opcode, buf, len));
}

void EQStream::ProcessPacket(EQProtocolPacket *p)
{
	uint32 processed=0, subpacket_length=0;
	if (p == nullptr)
		return;
	// Raw Application packet
	if (p->opcode > 0xff) {
		p->opcode = htons(p->opcode); //byte order is backwards in the protocol packet
		EQRawApplicationPacket *ap=MakeApplicationPacket(p);
		if (ap)
			InboundQueuePush(ap);
		return;
	}

	if (!Session && p->opcode!=OP_SessionRequest && p->opcode!=OP_SessionResponse) {
		LogNetcode(_L "Session not initialized, packet ignored" __L);
		// _raw(NET__DEBUG, 0xFFFF, p);
		return;
	}

	switch (p->opcode) {
		case OP_Combined: {
			processed=0;
			while(processed < p->size) {
				subpacket_length=*(p->pBuffer+processed);
				EQProtocolPacket *subp=MakeProtocolPacket(p->pBuffer+processed+1,subpacket_length);
				LogNetcode(_L "Extracting combined packet of length [{}]" __L, subpacket_length);
				// _raw(NET__NET_CREATE_HEX, 0xFFFF, subp);
				subp->copyInfo(p);
				ProcessPacket(subp);
				delete subp;
				processed+=subpacket_length+1;
			}
		}
		break;

		case OP_AppCombined: {
			processed=0;
			while(processed<p->size) {
				EQRawApplicationPacket *ap=nullptr;
				if ((subpacket_length=(unsigned char)*(p->pBuffer+processed))!=0xff) {
					LogNetcode(_L "Extracting combined app packet of length [{}], short len" __L, subpacket_length);
					ap=MakeApplicationPacket(p->pBuffer+processed+1,subpacket_length);
					processed+=subpacket_length+1;
				} else {
					subpacket_length=ntohs(*(uint16 *)(p->pBuffer+processed+1));
					LogNetcode(_L "Extracting combined app packet of length [{}], short len" __L, subpacket_length);
					ap=MakeApplicationPacket(p->pBuffer+processed+3,subpacket_length);
					processed+=subpacket_length+3;
				}
				if (ap) {
					ap->copyInfo(p);
					InboundQueuePush(ap);
				}
			}
		}
		break;

		case OP_Packet: {
			if(!p->pBuffer || (p->Size() < 4))
			{
				LogNetcode(_L "Received OP_Packet that was of malformed size" __L);
				break;
			}
			uint16 seq=ntohs(*(uint16 *)(p->pBuffer));
			SeqOrder check=CompareSequence(NextInSeq,seq);
			if (check == SeqFuture) {
					LogNetcode(_L "Future OP_Packet: Expecting Seq=[{}], but got Seq=[{}]" __L, NextInSeq, seq);
					// _raw(NET__DEBUG, seq, p);

					PacketQueue[seq]=p->Copy();
					LogNetcode(_L "OP_Packet Queue size=[{}]" __L, PacketQueue.size());

				//SendOutOfOrderAck(seq);

			} else if (check == SeqPast) {
				LogNetcode(_L "Duplicate OP_Packet: Expecting Seq=[{}], but got Seq=[{}]" __L, NextInSeq, seq);
				// _raw(NET__DEBUG, seq, p);
				SendOutOfOrderAck(seq); //we already got this packet but it was out of order
			} else {
				// In case we did queue one before as well.
				EQProtocolPacket *qp=RemoveQueue(seq);
				if (qp) {
					LogNetcode("[NET_TRACE] OP_Packet: Removing older queued packet with sequence [{}]", seq);
					delete qp;
				}

				SetNextAckToSend(seq);
				NextInSeq++;
				// Check for an embedded OP_AppCombinded (protocol level 0x19)
				if (*(p->pBuffer+2)==0x00 && *(p->pBuffer+3)==0x19) {
					EQProtocolPacket *subp=MakeProtocolPacket(p->pBuffer+2,p->size-2);
					LogNetcode(_L "seq [{}], Extracting combined packet of length [{}]" __L, seq, subp->size);
					// _raw(NET__NET_CREATE_HEX, seq, subp);
					subp->copyInfo(p);
					ProcessPacket(subp);
					delete subp;
				} else {
					EQRawApplicationPacket *ap=MakeApplicationPacket(p->pBuffer+2,p->size-2);
					if (ap) {
						ap->copyInfo(p);
						InboundQueuePush(ap);
					}
				}
			}
		}
		break;

		case OP_Fragment: {
			if(!p->pBuffer || (p->Size() < 4))
			{
				LogNetcode(_L "Received OP_Fragment that was of malformed size" __L);
				break;
			}
			uint16 seq=ntohs(*(uint16 *)(p->pBuffer));
			SeqOrder check=CompareSequence(NextInSeq,seq);
			if (check == SeqFuture) {
				LogNetcode(_L "Future OP_Fragment: Expecting Seq=[{}], but got Seq=[{}]" __L, NextInSeq, seq);
				// _raw(NET__DEBUG, seq, p);

				PacketQueue[seq]=p->Copy();
				LogNetcode(_L "OP_Fragment Queue size=[{}]" __L, PacketQueue.size());

				//SendOutOfOrderAck(seq);

			} else if (check == SeqPast) {
				LogNetcode(_L "Duplicate OP_Fragment: Expecting Seq=[{}], but got Seq=[{}]" __L, NextInSeq, seq);
				// _raw(NET__DEBUG, seq, p);
				SendOutOfOrderAck(seq);
			} else {
				// In case we did queue one before as well.
				EQProtocolPacket *qp=RemoveQueue(seq);
				if (qp) {
					LogNetcode("[NET_TRACE] OP_Fragment: Removing older queued packet with sequence [{}]", seq);
					delete qp;
				}
				SetNextAckToSend(seq);
				NextInSeq++;
				if (oversize_buffer) {
					memcpy(oversize_buffer+oversize_offset,p->pBuffer+2,p->size-2);
					oversize_offset+=p->size-2;
					LogNetcode(_L "Fragment of oversized of length [{}], seq [{}]: now at [{}]/[{}]" __L, p->size-2, seq, oversize_offset, oversize_length);
					if (oversize_offset==oversize_length) {
						if (*(p->pBuffer+2)==0x00 && *(p->pBuffer+3)==0x19) {
							EQProtocolPacket *subp=MakeProtocolPacket(oversize_buffer,oversize_offset);
							LogNetcode(_L "seq [{}], Extracting combined oversize packet of length [{}]" __L, seq, subp->size);
							//// _raw(NET__NET_CREATE_HEX, subp);
							subp->copyInfo(p);
							ProcessPacket(subp);
							delete subp;
						} else {
							EQRawApplicationPacket *ap=MakeApplicationPacket(oversize_buffer,oversize_offset);
							LogNetcode(_L "seq [{}], completed combined oversize packet of length [{}]" __L, seq, ap->size);
							if (ap) {
								ap->copyInfo(p);
								InboundQueuePush(ap);
							}
						}
						delete[] oversize_buffer;
						oversize_buffer=nullptr;
						oversize_offset=0;
					}
				} else {
					oversize_length=ntohl(*(uint32 *)(p->pBuffer+2));
					oversize_buffer=new unsigned char[oversize_length];
					memcpy(oversize_buffer,p->pBuffer+6,p->size-6);
					oversize_offset=p->size-6;
					LogNetcode(_L "First fragment of oversized of seq [{}]: now at [{}]/[{}]" __L, seq, oversize_offset, oversize_length);
				}
			}
		}
		break;
		case OP_KeepAlive: {
			NonSequencedPush(new EQProtocolPacket(p->opcode,p->pBuffer,p->size));
			LogNetcode(_L "Received and queued reply to keep alive" __L);
		}
		break;
		case OP_Ack: {
			if(!p->pBuffer || (p->Size() < 4))
			{
				LogNetcode(_L "Received OP_Ack that was of malformed size" __L);
				break;
			}
			uint16 seq=ntohs(*(uint16 *)(p->pBuffer));
			AckPackets(seq);

			if(GetExecutablePlatform() == ExePlatformWorld || GetExecutablePlatform() == ExePlatformZone) {
				retransmittimer = Timer::GetCurrentTime();
			}
		}
		break;
		case OP_SessionRequest: {
			if(p->Size() < sizeof(SessionRequest))
			{
				LogNetcode(_L "Received OP_SessionRequest that was of malformed size" __L);
				break;
			}
			if (GetState()==ESTABLISHED) {
				LogNetcode(_L "Received OP_SessionRequest in ESTABLISHED state ([{}]) streamactive ([{}]) attempt ([{}])" __L, GetState(),streamactive,sessionAttempts);

				// client seems to try a max of 30 times (initial+3 retries) then gives up, giving it a few more attempts just in case
				// streamactive means we identified the opcode for the stream, we cannot re-establish this connection
				if ( streamactive || ( sessionAttempts > MAX_SESSION_RETRIES ) )
				{
					_SendDisconnect();
					SetState(CLOSED);
					break;
				}
			}
			sessionAttempts++;
			// we set established below, so statistics will not be reset for session attempts/stream active.
			init(GetState()!=ESTABLISHED);
			OutboundQueueClear();
			SessionRequest *Request=(SessionRequest *)p->pBuffer;
			Session=ntohl(Request->Session);
			SetMaxLen(ntohl(Request->MaxLength));
			LogNetcode(_L "Received OP_SessionRequest: session [{}], maxlen [{}]" __L, (unsigned long)Session, MaxLen);
			SetState(ESTABLISHED);
			Key=0x11223344;
			SendSessionResponse();
		}
		break;
		case OP_SessionResponse: {
			if(p->Size() < sizeof(SessionResponse))
			{
				LogNetcode(_L "Received OP_SessionResponse that was of malformed size" __L);
				break;
			}

			init();
			OutboundQueueClear();
			SessionResponse *Response=(SessionResponse *)p->pBuffer;
			SetMaxLen(ntohl(Response->MaxLength));
			Key=ntohl(Response->Key);
			NextInSeq=0;
			SetState(ESTABLISHED);
			if (!Session)
				Session=ntohl(Response->Session);
			compressed=(Response->Format&FLAG_COMPRESSED);
			encoded=(Response->Format&FLAG_ENCODED);

			LogNetcode(_L "Received OP_SessionResponse: session [{}], maxlen [{}], key [{}], compressed? [{}], encoded? [{}]" __L, (unsigned long)Session, MaxLen, (unsigned long)Key, compressed?"yes":"no", encoded?"yes":"no");

			// Kinda kludgy, but trie for now
			if (StreamType==UnknownStream) {
				if (compressed) {
					if (remote_port==9000 || (remote_port==0 && p->src_port==9000)) {
						SetStreamType(WorldStream);
					} else {
						SetStreamType(ZoneStream);
					}
				} else if (encoded) {
					SetStreamType(ChatOrMailStream);
				} else {
					SetStreamType(LoginStream);
				}
			}
		}
		break;
		case OP_SessionDisconnect: {
			//NextInSeq=0;
			EQStreamState state = GetState();
			if(state == ESTABLISHED) {
				//client initiated disconnect?
				LogNetcode(_L "Received unsolicited OP_SessionDisconnect. Treating like a client-initiated disconnect" __L);
				_SendDisconnect();
				SetState(CLOSED);
			} else if(state == CLOSING) {
				//we were waiting for this anyways, ignore pending messages, send the reply and be closed.
				LogNetcode(_L "Received OP_SessionDisconnect when we have a pending close, they beat us to it. Were happy though" __L);
				_SendDisconnect();
				SetState(CLOSED);
			} else {
				//we are expecting this (or have already gotten it, but dont care either way)
				LogNetcode(_L "Received expected OP_SessionDisconnect. Moving to closed state" __L);
				SetState(CLOSED);
			}
		}
		break;
		case OP_OutOfOrderAck: {
			if(!p->pBuffer || (p->Size() < 4))
			{
				LogNetcode(_L "Received OP_OutOfOrderAck that was of malformed size" __L);
				break;
			}
			uint16 seq=ntohs(*(uint16 *)(p->pBuffer));
			MOutboundQueue.lock();

			if(uint16(SequencedBase + SequencedQueue.size()) != NextOutSeq) {
				LogNetcode(_L "Pre-OOA Invalid Sequenced queue: BS [{}] + SQ [{}] != NOS [{}]" __L, SequencedBase, SequencedQueue.size(), NextOutSeq);
			}

			//if the packet they got out of order is between our last acked packet and the last sent packet, then its valid.
			if (CompareSequence(SequencedBase,seq) != SeqPast && CompareSequence(NextOutSeq,seq) == SeqPast) {
				Log(Logs::Detail, Logs::Netcode, _L "Received OP_OutOfOrderAck for sequence %d, starting retransmit at the start of our unacked buffer (seq %d, was %d)." __L,
					seq, SequencedBase, SequencedBase+SequencedQueue.size());

				uint16 sqsize = SequencedQueue.size();
				uint16 index = seq - SequencedBase;
				LogNetcode(_L "OP_OutOfOrderAck marking packet acked in queue (queue index = [{}], queue size = [{}])" __L, index, sqsize);
				if (index < sqsize) {
					SequencedQueue[index]->acked = true;
					// flag packets for a resend
					uint16 count = 0;
					uint32 timeout = AverageDelta * 2 + 100;
					for (auto sitr = SequencedQueue.begin(); sitr != SequencedQueue.end() && count < index; ++sitr, ++count) {
						if (!(*sitr)->acked && (*sitr)->sent_time > 0 && (((*sitr)->sent_time + timeout) < Timer::GetCurrentTime())) {
							(*sitr)->sent_time = 0;
							LogNetcode(_L "OP_OutOfOrderAck Flagging packet [{}] for retransmission" __L, SequencedBase + count);
						}
					}
				}

				if(RETRANSMIT_TIMEOUT_MULT) {
					retransmittimer = Timer::GetCurrentTime();
				}
			} else {
				LogNetcode(_L "Received OP_OutOfOrderAck for out-of-window [{}]. Window ([{}]->[{}])" __L, seq, SequencedBase, NextOutSeq);
			}

			if(uint16(SequencedBase + SequencedQueue.size()) != NextOutSeq) {
				LogNetcode(_L "Post-OOA Invalid Sequenced queue: BS [{}] + SQ [{}] != NOS [{}]" __L, SequencedBase, SequencedQueue.size(), NextOutSeq);
			}

			MOutboundQueue.unlock();
		}
		break;
		case OP_SessionStatRequest: {
			if(p->Size() < sizeof(ClientSessionStats))
			{
				LogNetcode(_L "Received OP_SessionStatRequest that was of malformed size" __L);
				break;
			}
			ClientSessionStats *ClientStats=(ClientSessionStats *)p->pBuffer;
			Log(Logs::Detail, Logs::Netcode, _L "Received Stats: %lu packets received, %lu packets sent, Deltas: local %lu, (%lu <- %lu -> %lu) remote %lu" __L,
				(unsigned long)ntohl(ClientStats->packets_received), (unsigned long)ntohl(ClientStats->packets_sent), (unsigned long)ntohl(ClientStats->last_local_delta),
				(unsigned long)ntohl(ClientStats->low_delta), (unsigned long)ntohl(ClientStats->average_delta),
				(unsigned long)ntohl(ClientStats->high_delta), (unsigned long)ntohl(ClientStats->last_remote_delta));

			AdjustRates(ntohl(ClientStats->average_delta));

			if(GetExecutablePlatform() == ExePlatformWorld || GetExecutablePlatform() == ExePlatformZone) {
				if (RETRANSMIT_TIMEOUT_MULT && ntohl(ClientStats->average_delta)) {
					//recalculate retransmittimeout using the larger of the last rtt or average rtt, which is multiplied by the rule value
					if ((ntohl(ClientStats->last_local_delta) + ntohl(ClientStats->last_remote_delta)) > (ntohl(ClientStats->average_delta) * 2)) {
						retransmittimeout = (ntohl(ClientStats->last_local_delta) + ntohl(ClientStats->last_remote_delta))
							* RETRANSMIT_TIMEOUT_MULT;
					} else {
						retransmittimeout = ntohl(ClientStats->average_delta) * 2 * RETRANSMIT_TIMEOUT_MULT;
					}
					retransmittimeout += 300;
					if(retransmittimeout > RETRANSMIT_TIMEOUT_MAX)
						retransmittimeout = RETRANSMIT_TIMEOUT_MAX;
					LogNetcode(_L "Retransmit timeout recalculated to [{}]ms" __L, retransmittimeout);
				}
			}

			ServerSessionStats *ServerStats = (ServerSessionStats *)p->pBuffer;

			//ServerStats->RequestID = ClientStats->RequestID; // no change
			ServerStats->ServerTime = htonl(Timer::GetCurrentTime());
			ServerStats->packets_sent_echo = ClientStats->packets_sent; // still in htonll format
			ServerStats->packets_received_echo = ClientStats->packets_received; // still in htonll format
			ServerStats->packets_sent = htonll(GetPacketsSent());
			ServerStats->packets_received = htonll(GetPacketsReceived());

			NonSequencedPush(new EQProtocolPacket(OP_SessionStatResponse, p->pBuffer, p->size));
		}
		break;
		case OP_SessionStatResponse: {
			LogNetcode(_L "Received OP_SessionStatResponse. Ignoring" __L);
		}
		break;
		case OP_OutOfSession: {
			LogNetcode(_L "Received OP_OutOfSession. Ignoring" __L);
		}
		break;
		default:
			EQRawApplicationPacket *ap = MakeApplicationPacket(p);
			if (ap)
				InboundQueuePush(ap);
			break;
	}
}

void EQStream::QueuePacket(const EQApplicationPacket *p, bool ack_req)
{
	if(p == nullptr)
		return;

	EQApplicationPacket *newp = p->Copy();

	if (newp != nullptr)
		FastQueuePacket(&newp, ack_req);
}

void EQStream::FastQueuePacket(EQApplicationPacket **p, bool ack_req)
{
	EQApplicationPacket *pack=*p;
	*p = nullptr;		//clear caller's pointer.. effectively takes ownership

	if(pack == nullptr)
		return;

	if(OpMgr == nullptr || *OpMgr == nullptr) {
		LogNetcode(_L "Packet enqueued into a stream with no opcode manager, dropping" __L);
		delete pack;
		return;
	}

	uint16 opcode = 0;
	if(pack->GetOpcodeBypass() != 0) {
		opcode = pack->GetOpcodeBypass();
	} else {
		opcode = (*OpMgr)->EmuToEQ(pack->GetOpcode());
	}

	if (!ack_req) {
		NonSequencedPush(new EQProtocolPacket(opcode, pack->pBuffer, pack->size));
		delete pack;
	} else {
		SendPacket(opcode, pack);
	}
}

void EQStream::SendPacket(uint16 opcode, EQApplicationPacket *p)
{
	uint32 chunksize, used;
	uint32 length;

	if (EQEmuLogSys::Instance()->log_settings[Logs::Server_Client_Packet].is_category_enabled == 1){
		if (p->GetOpcode() != OP_SpecialMesg){
			Log(Logs::General, Logs::Server_Client_Packet, "[%s - 0x%04x] [Size: %u]", OpcodeManager::EmuToName(p->GetOpcode()), p->GetOpcode(), p->Size());
		}
	}

	if (EQEmuLogSys::Instance()->log_settings[Logs::Server_Client_Packet_With_Dump].is_category_enabled == 1){
		if (p->GetOpcode() != OP_SpecialMesg){
			Log(Logs::General, Logs::Server_Client_Packet_With_Dump, "[%s - 0x%04x] [Size: %u] %s", OpcodeManager::EmuToName(p->GetOpcode()), p->GetOpcode(), p->Size(), DumpPacketToString(p).c_str());
		}
	}

	// Convert the EQApplicationPacket to 1 or more EQProtocolPackets
	if (p->size>(MaxLen-8)) { // proto-op(2), seq(2), app-op(2) ... data ... crc(2)
		LogNetcode(_L "Making oversized packet, len [{}]" __L, p->Size());

		auto tmpbuff = new unsigned char[p->size + 3];
		length=p->serialize(opcode, tmpbuff);
		if (length != p->Size())
			LogNetcode(_L "Packet adjustment, len [{}] to [{}]" __L, p->Size(), length);

		auto out = new EQProtocolPacket(OP_Fragment, nullptr, MaxLen - 4);
		*(uint32 *)(out->pBuffer+2)=htonl(length);
		used=MaxLen-10;
		memcpy(out->pBuffer+6,tmpbuff,used);
		LogNetcode(_L "First fragment: used [{}]/[{}]. Payload size [{}] in the packet" __L, used, length, p->size);
		SequencedPush(out);


		while (used<length) {
			out=new EQProtocolPacket(OP_Fragment,nullptr,MaxLen-4);
			chunksize=std::min(length-used,MaxLen-6);
			memcpy(out->pBuffer+2,tmpbuff+used,chunksize);
			out->size=chunksize+2;
			SequencedPush(out);
			used+=chunksize;
			LogNetcode(_L "Subsequent fragment: len [{}], used [{}]/[{}]" __L, chunksize, used, length);
		}
		delete p;
		delete[] tmpbuff;
	} else {

		auto tmpbuff = new unsigned char[p->Size() + 3];
		length=p->serialize(opcode, tmpbuff+2) + 2;

		auto out = new EQProtocolPacket(OP_Packet, tmpbuff, length);

		delete[] tmpbuff;
		SequencedPush(out);
		delete p;
	}
}

void EQStream::SequencedPush(EQProtocolPacket *p)
{
	MOutboundQueue.lock();
	if (uint16(SequencedBase + SequencedQueue.size()) != NextOutSeq) {
		Log(Logs::Detail, Logs::Netcode, _L "Pre-Push Invalid Sequenced queue: BS %d + SQ %d != NOS %d" __L,
			SequencedBase, SequencedQueue.size(), NextOutSeq);
	}

	Log(Logs::Detail, Logs::Netcode, _L "Pushing sequenced packet %d of length %d. Base Seq is %d." __L,
		NextOutSeq, p->size, SequencedBase);
	*(uint16 *)(p->pBuffer) = htons(NextOutSeq);
	SequencedQueue.push_back(p);
	NextOutSeq++;

	if (uint16(SequencedBase + SequencedQueue.size()) != NextOutSeq) {
		Log(Logs::Detail, Logs::Netcode, _L "Push Invalid Sequenced queue: BS %d + SQ %d != NOS %d" __L,
			SequencedBase, SequencedQueue.size(), NextOutSeq);
	}

	MOutboundQueue.unlock();
}

void EQStream::NonSequencedPush(EQProtocolPacket *p)
{
	MOutboundQueue.lock();
	LogNetcode(_L "Pushing non-sequenced packet of length [{}]" __L, p->size);
	NonSequencedQueue.push(p);
	MOutboundQueue.unlock();
}

void EQStream::SendAck(uint16 seq)
{
uint16 Seq=htons(seq);
	LogNetcode(_L "Sending ack with sequence [{}]" __L, seq);
	SetLastAckSent(seq);
	NonSequencedPush(new EQProtocolPacket(OP_Ack,(unsigned char *)&Seq,sizeof(uint16)));
}

void EQStream::SendOutOfOrderAck(uint16 seq)
{
	LogNetcode(_L "Sending out of order ack with sequence [{}]" __L, seq);
uint16 Seq=htons(seq);
	NonSequencedPush(new EQProtocolPacket(OP_OutOfOrderAck,(unsigned char *)&Seq,sizeof(uint16)));
}

void EQStream::Write(int eq_fd)
{
	std::queue<EQProtocolPacket *> ReadyToSend;
	bool SeqEmpty=false, NonSeqEmpty=false;
	std::deque<EQProtocolPacket *>::iterator sitr;

	// Check our rate to make sure we can send more
	MRate.lock();
	int32 threshold=RateThreshold;
	MRate.unlock();
	if (BytesWritten > threshold) {
		return;
	}

	// If we got more packets to we need to ack, send an ack on the highest one
	MAcks.lock();
	if (CompareSequence(LastAckSent, NextAckToSend) == SeqFuture)
		SendAck(NextAckToSend);
	MAcks.unlock();

	// Lock the outbound queues while we process
	MOutboundQueue.lock();

	// Place to hold the base packet t combine into
	EQProtocolPacket *p=nullptr;

	// Find the next sequenced packet to send from the "queue"
	sitr = SequencedQueue.begin();

	uint16 count = 0;
	// get to start of packets
	while (sitr != SequencedQueue.end() && (*sitr)->sent_time > 0) {
		++sitr;
		++count;
	}

	// Loop until both are empty or MaxSends is reached
	while(!SeqEmpty || !NonSeqEmpty) {

		// See if there are more non-sequenced packets left
		if (!NonSequencedQueue.empty()) {
			if (!p) {
				// If we don't have a packet to try to combine into, use this one as the base
				// And remove it form the queue
				p = NonSequencedQueue.front();
				LogNetcode(_L "Starting combined packet with non-seq packet of len [{}]" __L, p->size);
				NonSequencedQueue.pop();
			} else if (!p->combine(NonSequencedQueue.front())) {
				// Trying to combine this packet with the base didn't work (too big maybe)
				// So just send the base packet (we'll try this packet again later)
				LogNetcode(_L "Combined packet full at len [{}], next non-seq packet is len [{}]" __L, p->size, (NonSequencedQueue.front())->size);
				ReadyToSend.push(p);
				BytesWritten+=p->size;
				p=nullptr;

				if (BytesWritten > threshold) {
					// Sent enough this round, lets stop to be fair
					LogNetcode(_L "Exceeded write threshold in nonseq ([{}] > [{}])" __L, BytesWritten, threshold);
					break;
				}
			} else {
				// Combine worked, so just remove this packet and it's spot in the queue
				LogNetcode(_L "Combined non-seq packet of len [{}], yeilding [{}] combined" __L, (NonSequencedQueue.front())->size, p->size);
				delete NonSequencedQueue.front();
				NonSequencedQueue.pop();
			}
		} else {
			// No more non-sequenced packets
			NonSeqEmpty=true;
		}

		if (sitr != SequencedQueue.end()) {
			uint16 seq_send = SequencedBase + count;	//just for logging...

			if(SequencedQueue.empty()) {
				LogNetcode(_L "Tried to write a packet with an empty queue ([{}] is past next out [{}])" __L, seq_send, NextOutSeq);
				SeqEmpty=true;
				continue;
			}

			if(GetExecutablePlatform() == ExePlatformWorld || GetExecutablePlatform() == ExePlatformZone) {
				if ((*sitr)->acked || (*sitr)->sent_time != 0) {
					++sitr;
					++count;
					if (p) {
						LogNetcode(_L "Final combined packet not full, len [{}]" __L, p->size);
						ReadyToSend.push(p);
						BytesWritten += p->size;
						p = nullptr;
					}
					LogNetcode(_L "Not retransmitting seq packet [{}] because already marked as acked" __L, seq_send);
				} else if (!p) {
					// If we don't have a packet to try to combine into, use this one as the base
					// Copy it first as it will still live until it is acked
					p=(*sitr)->Copy();
					LogNetcode(_L "Starting combined packet with seq packet [{}] of len [{}]" __L, seq_send, p->size);
					(*sitr)->sent_time = Timer::GetCurrentTime();
					++sitr;
					++count;
				} else if (!p->combine(*sitr)) {
					// Trying to combine this packet with the base didn't work (too big maybe)
					// So just send the base packet (we'll try this packet again later)
					LogNetcode(_L "Combined packet full at len [{}], next seq packet [{}] is len [{}]" __L, p->size, seq_send + 1, (*sitr)->size);
					ReadyToSend.push(p);
					BytesWritten+=p->size;
					p=nullptr;
					if ((*sitr)->opcode != OP_Fragment && BytesWritten > threshold) {
						// Sent enough this round, lets stop to be fair
						LogNetcode(_L "Exceeded write threshold in seq ([{}] > [{}])" __L, BytesWritten, threshold);
						break;
					}
				} else {
					// Combine worked
					LogNetcode(_L "Combined seq packet [{}] of len [{}], yeilding [{}] combined" __L, seq_send, (*sitr)->size, p->size);
					(*sitr)->sent_time = Timer::GetCurrentTime();
					++sitr;
					++count;
				}
			} else {
				if ((*sitr)->sent_time != 0) {
					++sitr;
					++count;
					if (p) {
						LogNetcode(_L "Final combined packet not full, len [{}]" __L, p->size);
						ReadyToSend.push(p);
						BytesWritten += p->size;
						p = nullptr;
					}
				} else if (!p) {
					// If we don't have a packet to try to combine into, use this one as the base
					// Copy it first as it will still live until it is acked
					p=(*sitr)->Copy();
					(*sitr)->sent_time = Timer::GetCurrentTime();
					LogNetcode(_L "Starting combined packet with seq packet [{}] of len [{}]" __L, seq_send, p->size);
					++sitr;
					++count;
				} else if (!p->combine(*sitr)) {
					// Trying to combine this packet with the base didn't work (too big maybe)
					// So just send the base packet (we'll try this packet again later)
					LogNetcode(_L "Combined packet full at len [{}], next seq packet [{}] is len [{}]" __L, p->size, seq_send, (*sitr)->size);
					ReadyToSend.push(p);
					BytesWritten+=p->size;
					p=nullptr;

					if (BytesWritten > threshold) {
						// Sent enough this round, lets stop to be fair
						LogNetcode(_L "Exceeded write threshold in seq ([{}] > [{}])" __L, BytesWritten, threshold);
						break;
					}
				} else {
					// Combine worked
					LogNetcode(_L "Combined seq packet [{}] of len [{}], yielding [{}] combined" __L, seq_send, (*sitr)->size, p->size);
					(*sitr)->sent_time = Timer::GetCurrentTime();
					++sitr;
					++count;
				}
			}

			if(uint16(SequencedBase + SequencedQueue.size()) != NextOutSeq) {
				LogNetcode(_L "Post send Invalid Sequenced queue: BS [{}] + SQ [{}] != NOS [{}]" __L, SequencedBase, SequencedQueue.size(), NextOutSeq);
			}
		} else {
			// No more sequenced packets
			SeqEmpty=true;
		}
	}
	// Unlock the queue
	MOutboundQueue.unlock();

	// We have a packet still, must have run out of both seq and non-seq, so send it
	if (p) {
		LogNetcode(_L "Final combined packet not full, len [{}]" __L, p->size);
		ReadyToSend.push(p);
		BytesWritten+=p->size;
	}

	// Send all the packets we "made"
	while(!ReadyToSend.empty()) {
		p = ReadyToSend.front();
		WritePacket(eq_fd,p);
		delete p;
		ReadyToSend.pop();
	}

	//see if we need to send our disconnect and finish our close
	if(SeqEmpty && NonSeqEmpty) {
		//no more data to send
		if(CheckState(CLOSING)) {
			LogNetcode(_L "All outgoing data flushed, closing stream" __L );
			//we are waiting for the queues to empty, now we can do our disconnect.
			//this packet will not actually go out until the next call to Write().
			_SendDisconnect();
			SetState(DISCONNECTING);
		}
	}
}

void EQStream::WritePacket(int eq_fd, EQProtocolPacket *p)
{
uint32 length;
sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr=remote_ip;
	address.sin_port=remote_port;
#ifdef NOWAY
	uint32 ip=address.sin_addr.s_addr;
	std::cout << "Sending to: "
		<< (int)*(unsigned char *)&ip
		<< "." << (int)*((unsigned char *)&ip+1)
		<< "." << (int)*((unsigned char *)&ip+2)
		<< "." << (int)*((unsigned char *)&ip+3)
		<< "," << (int)ntohs(address.sin_port) << "(" << p->size << ")" << std::endl;

	p->DumpRaw();
	std::cout << "-------------" << std::endl;
#endif
	length=p->serialize(buffer);
	if (p->opcode!=OP_SessionRequest && p->opcode!=OP_SessionResponse) {
		if (compressed) {
			BytesWritten -= p->size;
			uint32 newlen=EQProtocolPacket::Compress(buffer,length, _tempBuffer, 2048);
			memcpy(buffer,_tempBuffer,newlen);
			length=newlen;
			BytesWritten += newlen;
		}
		if (encoded) {
			EQProtocolPacket::ChatEncode(buffer,length,Key);
		}

		*(uint16 *)(buffer+length)=htons(CRC16(buffer,length,Key));
		length+=2;
	}
	//dump_message_column(buffer,length,"Writer: ");
	sendto(eq_fd,(char *)buffer,length,0,(sockaddr *)&address,sizeof(address));
	AddBytesSent(length);
}

void EQStream::SendSessionResponse()
{
	auto out = new EQProtocolPacket(OP_SessionResponse, nullptr, sizeof(SessionResponse));
	SessionResponse *Response=(SessionResponse *)out->pBuffer;
	Response->Session=htonl(Session);
	Response->MaxLength=htonl(MaxLen);
	Response->UnknownA=2;
	Response->Format=0;
	if (compressed)
		Response->Format|=FLAG_COMPRESSED;
	if (encoded)
		Response->Format|=FLAG_ENCODED;
	Response->Key=htonl(Key);

	out->size=sizeof(SessionResponse);

	Log(Logs::Detail, Logs::Netcode, _L "Sending OP_SessionResponse: session %lu, maxlen=%d, key=0x%x, compressed? %s, encoded? %s" __L,
		(unsigned long)Session, MaxLen, Key, compressed?"yes":"no", encoded?"yes":"no");

	NonSequencedPush(out);
}

void EQStream::SendSessionRequest()
{
	auto out = new EQProtocolPacket(OP_SessionRequest, nullptr, sizeof(SessionRequest));
	SessionRequest *Request=(SessionRequest *)out->pBuffer;
	memset(Request,0,sizeof(SessionRequest));
	Request->Session=htonl(time(nullptr));
	Request->MaxLength=htonl(512);

	LogNetcode(_L "Sending OP_SessionRequest: session [{}], maxlen=[{}]" __L, (unsigned long)ntohl(Request->Session), ntohl(Request->MaxLength));

	NonSequencedPush(out);
}

void EQStream::_SendDisconnect()
{
	if(GetState() == CLOSED)
		return;

	auto out = new EQProtocolPacket(OP_SessionDisconnect, nullptr, sizeof(uint32));
	*(uint32 *)out->pBuffer=htonl(Session);
	NonSequencedPush(out);

	LogNetcode(_L "Sending OP_SessionDisconnect: session [{}]" __L, (unsigned long)Session);
}

void EQStream::InboundQueuePush(EQRawApplicationPacket *p)
{
	MInboundQueue.lock();
	InboundQueue.push_back(p);
	MInboundQueue.unlock();
}

EQApplicationPacket *EQStream::PopPacket()
{
EQRawApplicationPacket *p=nullptr;

	MInboundQueue.lock();
	if (!InboundQueue.empty()) {
		auto itr = InboundQueue.begin();
		p=*itr;
		InboundQueue.erase(itr);
	}
	MInboundQueue.unlock();

	if (p) {
		if (OpMgr != nullptr && *OpMgr != nullptr) {
			EmuOpcode emu_op = (*OpMgr)->EQToEmu(p->opcode);
			if (emu_op == OP_Unknown) {
				// Log(Logs::General, Logs::Client_Server_Packet_Unhandled, "Unknown :: [%s - 0x%04x] [Size: %u] %s", OpcodeManager::EmuToName(p->GetOpcode()), p->opcode, p->Size(), DumpPacketToString(p).c_str());
			}
			p->SetOpcode(emu_op);
		}
	}

	return p;
}

EQRawApplicationPacket *EQStream::PopRawPacket()
{
EQRawApplicationPacket *p=nullptr;

	MInboundQueue.lock();
	if (!InboundQueue.empty()) {
		auto itr = InboundQueue.begin();
		p=*itr;
		InboundQueue.erase(itr);
	}
	MInboundQueue.unlock();

	//resolve the opcode if we can.
	if(p) {
		if(OpMgr != nullptr && *OpMgr != nullptr) {
			EmuOpcode emu_op = (*OpMgr)->EQToEmu(p->opcode);
			if(emu_op == OP_Unknown) {
				LogNetcode("Unable to convert EQ opcode {:#04x} to an Application opcode", p->opcode);
			}

			p->SetOpcode(emu_op);
		}
	}

	return p;
}

EQRawApplicationPacket *EQStream::PeekPacket()
{
EQRawApplicationPacket *p=nullptr;

	MInboundQueue.lock();
	if (!InboundQueue.empty()) {
		auto itr = InboundQueue.begin();
		p=*itr;
	}
	MInboundQueue.unlock();

	return p;
}

void EQStream::InboundQueueClear()
{
EQApplicationPacket *p=nullptr;

	LogNetcode(_L "Clearing inbound queue" __L);

	MInboundQueue.lock();
	if (!InboundQueue.empty()) {
		std::vector<EQRawApplicationPacket *>::iterator itr;
		for(itr=InboundQueue.begin();itr!=InboundQueue.end();++itr) {
			p=*itr;
			delete p;
		}
		InboundQueue.clear();
	}
	MInboundQueue.unlock();
}

bool EQStream::HasOutgoingData()
{
bool flag;

	//once closed, we have nothing more to say
	if(CheckClosed())
		return(false);

	MOutboundQueue.lock();
	flag=(!NonSequencedQueue.empty());
	if (!flag) {
		//not only wait until we send it all, but wait until they ack everything.
		flag = !SequencedQueue.empty();
	}
	MOutboundQueue.unlock();

	if (!flag) {
		MAcks.lock();
		flag= (NextAckToSend>LastAckSent);
		MAcks.unlock();
	}

	return flag;
}

void EQStream::OutboundQueueClear()
{
EQProtocolPacket *p=nullptr;

	LogNetcode(_L "Clearing outbound queue" __L);

	MOutboundQueue.lock();
	while(!NonSequencedQueue.empty()) {
		delete NonSequencedQueue.front();
		NonSequencedQueue.pop();
	}
	if(!SequencedQueue.empty()) {
		std::deque<EQProtocolPacket *>::iterator itr;
		for(itr=SequencedQueue.begin();itr!=SequencedQueue.end();++itr) {
			p=*itr;
			delete p;
		}
		SequencedQueue.clear();
	}
	MOutboundQueue.unlock();
}

void EQStream::PacketQueueClear()
{
EQProtocolPacket *p=nullptr;

	LogNetcode(_L "Clearing future packet queue" __L);

	if(!PacketQueue.empty()) {
		std::map<unsigned short,EQProtocolPacket *>::iterator itr;
		for(itr=PacketQueue.begin();itr!=PacketQueue.end();++itr) {
			p=itr->second;
			delete p;
		}
		PacketQueue.clear();
	}
}

void EQStream::Process(const unsigned char *buffer, const uint32 length)
{
	static unsigned char newbuffer[2048];
	uint32 newlength=0;
	if (EQProtocolPacket::ValidateCRC(buffer,length,Key)) {
		if (compressed) {
			newlength=EQProtocolPacket::Decompress(buffer,length,newbuffer,2048);
		} else {
			memcpy(newbuffer,buffer,length);
			newlength=length;
			if (encoded)
				EQProtocolPacket::ChatDecode(newbuffer,newlength-2,Key);
		}
		if (buffer[1]!=0x01 && buffer[1]!=0x02 && buffer[1]!=0x1d)
			newlength-=2;
		EQProtocolPacket *p = MakeProtocolPacket(newbuffer,newlength);
		ProcessPacket(p);
		delete p;
		ProcessQueue();
	} else {
		LogNetcode(_L "Incoming packet failed checksum" __L);
	}
}

long EQStream::GetNextAckToSend()
{
	MAcks.lock();
	long l=NextAckToSend;
	MAcks.unlock();

	return l;
}

long EQStream::GetLastAckSent()
{
	MAcks.lock();
	long l=LastAckSent;
	MAcks.unlock();

	return l;
}

void EQStream::AckPackets(uint16 seq)
{
std::deque<EQProtocolPacket *>::iterator itr, tmp;

	MOutboundQueue.lock();

	SeqOrder ord = CompareSequence(SequencedBase, seq);
	if(ord == SeqInOrder) {
		//they are not acking anything new...
		LogNetcode(_L "Received an ack with no window advancement (seq [{}])" __L, seq);
	} else if(ord == SeqPast) {
		//they are nacking blocks going back before our buffer, wtf?
		LogNetcode(_L "Received an ack with backward window advancement (they gave [{}], our window starts at [{}]). This is bad" __L, seq, SequencedBase);
	} else {
		LogNetcode(_L "Received an ack up through sequence [{}]. Our base is [{}]" __L, seq, SequencedBase);


		//this is a good ack, we get to ack some blocks.
		seq++;	//we stop at the block right after their ack, counting on the wrap of both numbers.
		while(SequencedBase != seq) {
			if(SequencedQueue.empty()) {
				LogNetcode(_L "OUT OF PACKETS acked packet with sequence [{}]. Next send is [{}] before this" __L, (unsigned long)SequencedBase, SequencedQueue.size());
				SequencedBase = NextOutSeq;
				break;
			}
			LogNetcode(_L "Removing acked packet with sequence [{}]" __L, (unsigned long)SequencedBase);
			//clean out the acked packet
			delete SequencedQueue.front();
			SequencedQueue.pop_front();
			//advance the base sequence number to the seq of the block after the one we just got rid of.
			SequencedBase++;
		}
		if(uint16(SequencedBase + SequencedQueue.size()) != NextOutSeq) {
			LogNetcode(_L "Post-Ack on [{}] Invalid Sequenced queue: BS [{}] + SQ [{}] != NOS [{}]" __L, seq, SequencedBase, SequencedQueue.size(), NextOutSeq);
		}
	}

	MOutboundQueue.unlock();
}

void EQStream::SetNextAckToSend(uint32 seq)
{
	MAcks.lock();
	LogNetcode(_L "Set Next Ack To Send to [{}]" __L, (unsigned long)seq);
	NextAckToSend=seq;
	MAcks.unlock();
}

void EQStream::SetLastAckSent(uint32 seq)
{
	MAcks.lock();
	LogNetcode(_L "Set Last Ack Sent to [{}]" __L, (unsigned long)seq);
	LastAckSent=seq;
	MAcks.unlock();
}

void EQStream::ProcessQueue()
{
	if(PacketQueue.empty()) {
		return;
	}

	EQProtocolPacket *qp=nullptr;
	while((qp=RemoveQueue(NextInSeq))!=nullptr) {
		LogNetcode(_L "Processing Queued Packet: Seq=[{}]" __L, NextInSeq);
		ProcessPacket(qp);
		delete qp;
		LogNetcode(_L "OP_Packet Queue size=[{}]" __L, PacketQueue.size());
	}
}

EQProtocolPacket *EQStream::RemoveQueue(uint16 seq)
{
std::map<unsigned short,EQProtocolPacket *>::iterator itr;
EQProtocolPacket *qp=nullptr;
	if ((itr=PacketQueue.find(seq))!=PacketQueue.end()) {
		qp=itr->second;
		PacketQueue.erase(itr);
		LogNetcode(_L "OP_Packet Queue size=[{}]" __L, PacketQueue.size());
	}
	return qp;
}

void EQStream::SetStreamType(EQStreamType type)
{
	LogNetcode(_L "Changing stream type from [{}] to [{}]" __L, StreamTypeString(StreamType), StreamTypeString(type));
	StreamType=type;
	switch (StreamType) {
		case LoginStream:
			app_opcode_size=1;
			compressed=false;
			encoded=false;
			LogNetcode(_L "Login stream has app opcode size [{}], is not compressed or encoded" __L, app_opcode_size);
			break;
		case ChatOrMailStream:
		case ChatStream:
		case MailStream:
			app_opcode_size=1;
			compressed=false;
			encoded=true;
			LogNetcode(_L "Chat/Mail stream has app opcode size [{}], is not compressed, and is encoded" __L, app_opcode_size);
			break;
		case ZoneStream:
		case WorldStream:
		default:
			app_opcode_size=2;
			compressed=true;
			encoded=false;
			LogNetcode(_L "World/Zone stream has app opcode size [{}], is compressed, and is not encoded" __L, app_opcode_size);
			break;
	}
}

const char *EQStream::StreamTypeString(EQStreamType t)
{
	switch (t) {
		case LoginStream:
			return "Login";
			break;
		case WorldStream:
			return "World";
			break;
		case ZoneStream:
			return "Zone";
			break;
		case ChatOrMailStream:
			return "Chat/Mail";
			break;
		case ChatStream:
			return "Chat";
			break;
		case MailStream:
			return "Mail";
			break;
		case UnknownStream:
			return "Unknown";
			break;
	}
	return "UnknownType";
}

//returns SeqFuture if `seq` is later than `expected_seq`
EQStream::SeqOrder EQStream::CompareSequence(uint16 expected_seq , uint16 seq)
{
	if (expected_seq==seq) {
		// Curent
		return SeqInOrder;
	} else if ((seq > expected_seq && (uint32)seq < ((uint32)expected_seq + EQStream::MaxWindowSize)) || seq < (expected_seq - EQStream::MaxWindowSize)) {
		// Future
		return SeqFuture;
	} else {
		// Past
		return SeqPast;
	}
}

void EQStream::SetState(EQStreamState state) {
	MState.lock();
	LogNetcode(_L "Changing state from [{}] to [{}]" __L, State, state);
	State=state;
	MState.unlock();
}


void EQStream::CheckTimeout(uint32 now, uint32 timeout) {

	bool outgoing_data = HasOutgoingData();	//up here to avoid recursive locking

	EQStreamState orig_state = GetState();
	if (orig_state == CLOSING && !outgoing_data) {
		LogNetcode(_L "Out of data in closing state, disconnecting" __L);
		_SendDisconnect();
		SetState(DISCONNECTING);
	} else if (LastPacket && (now-LastPacket) > timeout) {
		switch(orig_state) {
		case CLOSING:
			//if we time out in the closing state, they are not acking us, just give up
			LogNetcode(_L "Timeout expired in closing state. Moving to closed state" __L);
			_SendDisconnect();
			SetState(CLOSED);
			break;
		case DISCONNECTING:
			//we timed out waiting for them to send us the disconnect reply, just give up.
			LogNetcode(_L "Timeout expired in disconnecting state. Moving to closed state" __L);
			SetState(CLOSED);
			break;
		case CLOSED:
			LogNetcode(_L "Timeout expired in closed state??" __L);
			break;
		case ESTABLISHED:
			//we timed out during normal operation. Try to be nice about it.
			//we will almost certainly time out again waiting for the disconnect reply, but oh well.
			LogNetcode(_L "Timeout expired in established state. Closing connection" __L);
			_SendDisconnect();
			SetState(DISCONNECTING);
			break;
		default:
			break;
		}
	}
}

void EQStream::Decay()
{
	MRate.lock();
	uint32 rate=DecayRate;
	MRate.unlock();
	if (BytesWritten>0) {
		BytesWritten-=rate;
		if (BytesWritten<0)
			BytesWritten=0;
	}
	// check for any timed out acks
	if ((GetExecutablePlatform() == ExePlatformWorld || GetExecutablePlatform() == ExePlatformZone) && RETRANSMIT_TIMEOUT_MULT && retransmittimeout) {
		int count = 0;
		MOutboundQueue.lock();
		for (auto sitr = SequencedQueue.begin(); sitr != SequencedQueue.end(); ++sitr, count++) {
			if (!(*sitr)->acked && (*sitr)->sent_time > 0 && ((*sitr)->sent_time + retransmittimeout) < Timer::GetCurrentTime()) {
				(*sitr)->sent_time = 0;
				LogNetcode(_L "Timeout exceeded for seq [{}].  Flagging packet for retransmission" __L, SequencedBase + count);
			}
		}
		MOutboundQueue.unlock();
	}
}

void EQStream::AdjustRates(uint32 average_delta)
{
	if(GetExecutablePlatform() == ExePlatformWorld || GetExecutablePlatform() == ExePlatformZone) {
		if (average_delta && (average_delta <= AVERAGE_DELTA_MAX)) {
			MRate.lock();
			AverageDelta = average_delta;
			RateThreshold=RATEBASE/average_delta;
			DecayRate=DECAYBASE/average_delta;
			if (BytesWritten > RateThreshold)
				BytesWritten = RateThreshold + DecayRate;
			Log(Logs::Detail, Logs::Netcode, _L "Adjusting data rate to thresh %d, decay %d based on avg delta %d" __L,
				RateThreshold, DecayRate, average_delta);
			MRate.unlock();
		} else {
			Log(Logs::Detail, Logs::Netcode, _L "Not adjusting data rate because avg delta over max (%d > %d)" __L,
				average_delta, AVERAGE_DELTA_MAX);
			AverageDelta = AVERAGE_DELTA_MAX;
		}
	} else {
		if (average_delta) {
			MRate.lock();
			AverageDelta = average_delta;
			BytesWritten = 0;
			RateThreshold=RATEBASE/average_delta;
			DecayRate=DECAYBASE/average_delta;
			Log(Logs::Detail, Logs::Netcode, _L "Adjusting data rate to thresh %d, decay %d based on avg delta %d" __L,
				RateThreshold, DecayRate, average_delta);
			MRate.unlock();
		}
	}
}

void EQStream::Close() {
	if(HasOutgoingData()) {
		//there is pending data, wait for it to go out.
		LogNetcode(_L "Stream requested to Close(), but there is pending data, waiting for it" __L);
		SetState(CLOSING);
	} else {
		//otherwise, we are done, we can drop immediately.
		_SendDisconnect();
		LogNetcode(_L "Stream closing immediate due to Close()" __L);
		SetState(DISCONNECTING);
	}
}


//this could be expanded to check more than the fitst opcode if
//we needed more complex matching
EQStream::MatchState EQStream::CheckSignature(const Signature *sig) {
	EQRawApplicationPacket *p = nullptr;
	MatchState res = MatchNotReady;

	MInboundQueue.lock();
	if (!InboundQueue.empty()) {
		//this is already getting hackish...
		p = InboundQueue.front();
		if(sig->ignore_eq_opcode != 0 && p->opcode == sig->ignore_eq_opcode) {
			if(InboundQueue.size() > 1) {
				p = InboundQueue[1];
			} else {
				p = nullptr;
			}
		}
		if(p == nullptr) {
			//first opcode is ignored, and nothing else remains... keep waiting
		} else if(p->opcode == sig->first_eq_opcode) {
			//opcode matches, check length..
			if(p->size == sig->first_length) {
				LogNetcode("[StreamIdentify] [{}]:[{}]: First opcode matched {:#04x} and length matched [{}]", long2ip(GetRemoteIP()).c_str(), ntohs(GetRemotePort()), sig->first_eq_opcode, p->size);
				res = MatchSuccessful;
			} else if(sig->first_length == 0) {
				LogNetcode("[StreamIdentify] [{}]:[{}]: First opcode matched {:#04x} and length ([{}]) is ignored", long2ip(GetRemoteIP()).c_str(), ntohs(GetRemotePort()), sig->first_eq_opcode, p->size);
				res = MatchSuccessful;
			} else {
				//opcode matched but length did not.
				LogNetcode("[StreamIdentify] [{}]:[{}]: First opcode matched {:#04x}, but length [{}] did not match expected [{}]", long2ip(GetRemoteIP()).c_str(), ntohs(GetRemotePort()), sig->first_eq_opcode, p->size, sig->first_length);
				res = MatchFailed;
			}
		} else {
			//first opcode did not match..
			LogNetcode("[StreamIdentify] [{}]:[{}]: First opcode {:#04x} did not match expected {:#04x}", long2ip(GetRemoteIP()).c_str(), ntohs(GetRemotePort()), p->opcode, sig->first_eq_opcode);
			res = MatchFailed;
		}
	}
	MInboundQueue.unlock();

	return(res);
}
