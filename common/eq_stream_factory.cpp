#include "eq_stream_factory.h"

#include "common/global_define.h"
#ifdef _WINDOWS
#include <winsock2.h>
#include <process.h>
#include <io.h>
#include <stdio.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#endif

#include <iostream>
#include <fcntl.h>

#include "op_codes.h"

ThreadReturnType EQStreamFactoryReaderLoop(void *eqfs)
{
	EQStreamFactory *fs=(EQStreamFactory *)eqfs;

	fs->ReaderLoop();

	THREAD_RETURN(nullptr);
}

ThreadReturnType EQStreamFactoryWriterLoop(void *eqfs)
{
	EQStreamFactory *fs=(EQStreamFactory *)eqfs;

	fs->WriterLoop();

	THREAD_RETURN(nullptr);
}

EQStreamFactory::EQStreamFactory(EQStreamType type, int port, uint32 timeout)
	: Timeoutable(5000), stream_timeout(timeout)
{
	StreamType=type;
	Port=port;
	sock=-1;
}

void EQStreamFactory::Close()
{
	Stop();

#ifdef _WINDOWS
	closesocket(sock);
#else
	close(sock);
#endif
	sock=-1;
}

bool EQStreamFactory::Open()
{
struct sockaddr_in address;
#ifndef WIN32
	pthread_t t1,t2;
#endif
	/* Setup internet address information.
	This is used with the bind() call */
	memset((char *) &address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(Port);
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	/* Setting up UDP port for new clients */
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		return false;
	}

	if (bind(sock, (struct sockaddr *) &address, sizeof(address)) < 0) {
		close(sock);
		sock=-1;
		return false;
	}
	#ifdef _WINDOWS
		unsigned long nonblock = 1;
		ioctlsocket(sock, FIONBIO, &nonblock);
	#else
		fcntl(sock, F_SETFL, O_NONBLOCK);
	#endif
	//moved these because on windows the output was delayed and causing the console window to look bad
	#ifdef _WINDOWS
		_beginthread(EQStreamFactoryReaderLoop,0, this);
		_beginthread(EQStreamFactoryWriterLoop,0, this);
	#else
		pthread_create(&t1,nullptr,EQStreamFactoryReaderLoop,this);
		pthread_create(&t2,nullptr,EQStreamFactoryWriterLoop,this);
	#endif
	return true;
}

std::shared_ptr<EQStream> EQStreamFactory::Pop()
{
	std::shared_ptr<EQStream> s = nullptr;
	MNewStreams.lock();
	if (!NewStreams.empty()) {
		s = NewStreams.front();
		NewStreams.pop();
		s->PutInUse();
	}
	MNewStreams.unlock();

	return s;
}

void EQStreamFactory::Push(std::shared_ptr<EQStream> s)
{
	MNewStreams.lock();
	NewStreams.push(s);
	MNewStreams.unlock();
}

void EQStreamFactory::ReaderLoop()
{
	fd_set readset;
	std::map<std::pair<uint32, uint16>, std::shared_ptr<EQStream>>::iterator stream_itr;
	int num;
	int length;
	unsigned char buffer[2048];
	sockaddr_in from;
	int socklen = sizeof(sockaddr_in);
	timeval sleep_time;
	ReaderRunning = true;

	while(sock!=-1) {
		MReaderRunning.lock();
		if (!ReaderRunning)
			break;
		MReaderRunning.unlock();

		FD_ZERO(&readset);
		FD_SET(sock,&readset);

		sleep_time.tv_sec=30;
		sleep_time.tv_usec=0;
		if ((num=select(sock+1,&readset,nullptr,nullptr,&sleep_time))<0) {
			// What do we wanna do?
			continue;
		} else if (num==0)
			continue;

		if(sock == -1)
			break;		//somebody closed us while we were sleeping.

		if (FD_ISSET(sock,&readset)) {
#ifdef _WINDOWS
			if ((length=recvfrom(sock,(char*)buffer,sizeof(buffer),0,(struct sockaddr*)&from,(int *)&socklen)) < 2)
#else
			if ((length=recvfrom(sock,buffer,2048,0,(struct sockaddr *)&from,(socklen_t *)&socklen)) < 2)
#endif
			{
				// What do we wanna do?
			} else {
				MStreams.lock();
				stream_itr = Streams.find(std::make_pair(from.sin_addr.s_addr, from.sin_port));
				if (stream_itr == Streams.end()) {
					if (buffer[1]==OP_SessionRequest) {
						std::shared_ptr<EQStream> s = std::make_shared<EQStream>(from);
						s->SetStreamType(StreamType);
						Streams[std::make_pair(from.sin_addr.s_addr, from.sin_port)]=s;
						WriterWork.Signal();
						Push(s);
						s->AddBytesRecv(length);
						s->Process(buffer,length);
						s->SetLastPacketTime(Timer::GetCurrentTime());
					}
					MStreams.unlock();
				} else {
					std::shared_ptr<EQStream> curstream = stream_itr->second;
					//dont bother processing incoming packets for closed connections
					if(curstream->CheckClosed())
						curstream = nullptr;
					else
						curstream->PutInUse();
						//the in use flag prevents the stream from being deleted while we are using it.

					if(curstream) {
						curstream->AddBytesRecv(length);
						curstream->Process(buffer,length);
						curstream->SetLastPacketTime(Timer::GetCurrentTime());
						curstream->ReleaseFromUse();
					}
					MStreams.unlock();
				}
			}
		}
	}
}

void EQStreamFactory::CheckTimeout()
{
	//lock streams the entire time were checking timeouts, it should be fast.
	MStreams.lock();

	unsigned long now=Timer::GetCurrentTime();
	std::map<std::pair<uint32, uint16>, std::shared_ptr<EQStream>>::iterator stream_itr;

	for(stream_itr = Streams.begin(); stream_itr != Streams.end();) {
		std::shared_ptr<EQStream> s = stream_itr->second;

		s->CheckTimeout(now, stream_timeout);

		EQStreamState state = s->GetState();

		//not part of the else so we check it right away on state change
		if (state==CLOSED) {
			if (s->IsInUse()) {
				//give it a little time for everybody to finish with it
			} else {
				//everybody is done, we can delete it now
				auto temp = stream_itr;
				++stream_itr;
				temp->second = nullptr;
				Streams.erase(temp);
				continue;
			}
		}

		++stream_itr;
	}
	MStreams.unlock();
}

void EQStreamFactory::WriterLoop()
{
	bool havework = true;
	std::vector<std::shared_ptr<EQStream>> wants_write;
	std::vector<std::shared_ptr<EQStream>>::iterator cur, end;
	bool decay = false;
	uint32 stream_count;
	Timer DecayTimer(20);
	WriterRunning = true;
	DecayTimer.Enable();

	while (sock != -1) {
		MWriterRunning.lock();
		if (!WriterRunning)
			break;
		MWriterRunning.unlock();

		havework = false;
		wants_write.clear();

		decay = DecayTimer.Check();

		// copy streams into a seperate list so we dont have to keep
		// MStreams locked while we are writting
		MStreams.lock();
		for (auto stream_itr = Streams.begin(); stream_itr != Streams.end(); ++stream_itr) {
			// If it's time to decay the bytes sent, then let's do it before we try to write
			if (decay)
				stream_itr->second->Decay();

			// bullshit checking, to see if this is really happening, GDB seems to think so...
			if (stream_itr->second == nullptr) {
				fprintf(stderr,
					"ERROR: nullptr Stream encountered in EQStreamFactory::WriterLoop for: %i:%i",
					stream_itr->first.first, stream_itr->first.second);
				continue;
			}

			if (stream_itr->second->HasOutgoingData()) {
				havework = true;
				stream_itr->second->PutInUse();
				wants_write.push_back(stream_itr->second);
			}
		}
		MStreams.unlock();

		// do the actual writes
		cur = wants_write.begin();
		end = wants_write.end();
		for (; cur != end; ++cur) {
			(*cur)->Write(sock);
			(*cur)->ReleaseFromUse();
		}

		Sleep(10);

		MStreams.lock();
		stream_count = Streams.size();
		MStreams.unlock();
		if (!stream_count) {
			WriterWork.Wait();
		}
	}
}

