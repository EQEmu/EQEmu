#pragma once

#include "common/types.h"

#include <stdio.h>
#include <time.h>

//constants used in the packet file header
#define PACKET_FILE_MAGIC 0x93a7b6f6
#define OLD_PACKET_FILE_MAGIC 0x93a7b6f7

#define PACKET_FILE_CURRENT_VERSION 1

#pragma pack(1)
//old structs from when I forgot to put the version number in
struct OldPacketFileHeader {
	uint32 packet_file_magic;
	uint32 packet_file_stamp;
};
struct OldPacketFileSection {
	uint16 opcode;
	uint32 len;
};

struct PacketFileHeader {
	uint32 packet_file_magic;
	uint16 packet_file_version;
	uint32 packet_file_stamp;
};

struct PacketFileSection {
	uint16 opcode;
	uint8 flags;	//mainly for client->server, but others could be added
	uint32 tv_sec;
	uint16 tv_msec;
	uint32 len;
};
#pragma pack()

#define TO_SERVER_FLAG 0x01
#define SetToClient(pfs) pfs.flags = pfs.flags&~TO_SERVER_FLAG
#define SetToServer(pfs) pfs.flags = pfs.flags|TO_SERVER_FLAG
#define IsToClient(pfs) (pfs.flags&TO_SERVER_FLAG == 0)
#define IsToServer(pfs) (pfs.flags&TO_SERVER_FLAG != 0)


class PacketFileWriter {
public:
	PacketFileWriter(bool force_flush);
	~PacketFileWriter();

	bool OpenFile(const char *name);
	void CloseFile();

	void WritePacket(uint16 eq_op, uint32 packlen, const unsigned char *packet, bool to_server, const struct timeval &tv);

	static bool SetPacketStamp(const char *file, uint32 stamp);

protected:
	bool _WriteBlock(uint16 eq_op, const void *d, uint16 len, bool to_server, const struct timeval &tv);

	//gzFile out;
	FILE *out;
	bool force_flush;
};


class PacketFileReader {
public:
	PacketFileReader();

	virtual bool OpenFile(const char *name) = 0;
	virtual void CloseFile() = 0;
	virtual bool ResetFile() = 0;	//aka rewind

	virtual bool ReadPacket(uint16 &eq_op, uint32 &packlen, unsigned char *packet, bool &to_server, struct timeval &tv) = 0;

	time_t GetStamp() { return(time_t(packet_file_stamp)); }

	//factory method to open the right packet file.
	static PacketFileReader *OpenPacketFile(const char *name);

protected:

	uint32 packet_file_stamp;
};

class OldPacketFileReader : public PacketFileReader {
public:
	OldPacketFileReader();
	virtual ~OldPacketFileReader();

	bool OpenFile(const char *name);
	void CloseFile();
	bool ResetFile();	//aka rewind

	bool ReadPacket(uint16 &eq_op, uint32 &packlen, unsigned char *packet, bool &to_server, struct timeval &tv);

	time_t GetStamp() { return(time_t(packet_file_stamp)); }

protected:

	//gzFile in;
	FILE *in;
};

class NewPacketFileReader: public PacketFileReader {
public:
	NewPacketFileReader();
	virtual ~NewPacketFileReader();

	bool OpenFile(const char *name);
	void CloseFile();
	bool ResetFile();	//aka rewind

	bool ReadPacket(uint16 &eq_op, uint32 &packlen, unsigned char *packet, bool &to_server, struct timeval &tv);

	time_t GetStamp() { return(time_t(packet_file_stamp)); }

protected:

	//gzFile in;
	FILE *in;
};


#endif


