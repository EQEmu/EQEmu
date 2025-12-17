#include "packetfile.h"

#include "common/eq_opcodes.h"
#include "common/eq_packet_structs.h"
#include "common/misc.h"

#include <errno.h>
#include <string.h>
#include <time.h>
#include <map>

PacketFileWriter::PacketFileWriter(bool _force_flush) {
	out = NULL;
	force_flush = _force_flush;
}

PacketFileWriter::~PacketFileWriter() {
	CloseFile();
}

bool PacketFileWriter::SetPacketStamp(const char *name, uint32 stamp) {
	FILE *in;
	in = fopen(name, "r+b");
	if(in == NULL) {
		fprintf(stderr, "Error opening packet file '%s': %s\n", name, strerror(errno));
		return(false);
	}

	unsigned long magic = 0;

	if(fread(&magic, sizeof(magic), 1, in) != 1) {
		fprintf(stderr, "Error reading header from packet file: %s\n", strerror(errno));
		fclose(in);
		return(false);
	}

	PacketFileReader *ret = NULL;
	if(magic == OLD_PACKET_FILE_MAGIC) {
		OldPacketFileHeader *pos = 0;
		uint32 stamp_pos = (uint32) &pos->packet_file_stamp;
		fseek(in, stamp_pos, SEEK_SET);
		OldPacketFileHeader hdr;
		hdr.packet_file_stamp = stamp;
		if(fwrite(&hdr.packet_file_stamp, sizeof(hdr.packet_file_stamp), 1, in) != 1) {
			fprintf(stderr, "Error writting to packet file: %s\n", strerror(errno));
			fclose(in);
			return(false);
		}
	} else if(magic == PACKET_FILE_MAGIC) {
		PacketFileHeader *pos = 0;
		uint32 stamp_pos = (uint32) &pos->packet_file_stamp;
		fseek(in, stamp_pos, SEEK_SET);
		PacketFileHeader hdr;
		hdr.packet_file_stamp = stamp;
		if(fwrite(&hdr.packet_file_stamp, sizeof(hdr.packet_file_stamp), 1, in) != 1) {
			fprintf(stderr, "Error writting to packet file: %s\n", strerror(errno));
			fclose(in);
			return(false);
		}
	} else {
		fprintf(stderr, "Unknown packet file type 0x%.8x\n", magic);
		fclose(in);
		return(false);
	}

	fclose(in);
	return(true);
}

bool PacketFileWriter::OpenFile(const char *name) {
	CloseFile();

	printf("Opening packet file: %s\n", name);

	out = fopen(name, "wb");
	if(out == NULL) {
		fprintf(stderr, "Error opening packet file '%s': %s\n", name, strerror(errno));
		return(false);
	}

	PacketFileHeader head;
	head.packet_file_magic = PACKET_FILE_MAGIC;
	head.packet_file_version = PACKET_FILE_CURRENT_VERSION;
	head.packet_file_stamp = time(NULL);

	if(fwrite(&head, sizeof(head), 1, out) != 1) {
		fprintf(stderr, "Error writting header to packet file: %s\n", strerror(errno));
		fclose(out);
		return(false);
	}

	return(true);
}

void PacketFileWriter::CloseFile() {
	if(out != NULL) {
		fclose(out);
		out = NULL;
		printf("Closed packet file.\n");
	}
}

void PacketFileWriter::WritePacket(uint16 eq_op, uint32 packlen, const unsigned char *packet, bool to_server, const struct timeval &tv) {
	if(out == NULL)
		return;

	_WriteBlock(eq_op, packet, packlen, to_server, tv);

/*
	Could log only the packets we care about, but this is most of the stream,
	so just log them all...

	switch(eq_op) {
	case OP_NewZone:
	case OP_ZoneSpawns:
	case OP_NewSpawn:
	case OP_MobUpdate:
	case OP_ClientUpdate:
	case OP_Death:
	case OP_DeleteSpawn:
	case OP_CastSpell:
	case OP_ShopRequest:
	case OP_ShopEndConfirm:
	case OP_ItemPacket:
		_WriteBlock(eq_op, packet, packlen);
	default:
		return;
	}
	*/
}

bool PacketFileWriter::_WriteBlock(uint16 eq_op, const void *d, uint16 len, bool to_server, const struct timeval &tv) {
	if(out == NULL)
		return(false);

	PacketFileSection s;
	s.opcode = eq_op;
	s.len = len;
	s.tv_sec = tv.tv_sec;
	s.tv_msec = tv.tv_usec/1000;

	if(to_server)
		SetToServer(s);
	else
		SetToClient(s);

	if(fwrite(&s, sizeof(s), 1, out) != 1) {
		fprintf(stderr, "Error writting block header: %s\n", strerror(errno));
		return(false);
	}

	if(fwrite(d, 1, len, out) != len) {
		fprintf(stderr, "Error writting block body: %s\n", strerror(errno));
		return(false);
	}

	if(force_flush)
		fflush(out);

	return(true);
}






PacketFileReader *PacketFileReader::OpenPacketFile(const char *name) {
	FILE *in;
	in = fopen(name, "rb");
	if(in == NULL) {
		fprintf(stderr, "Error opening packet file '%s': %s\n", name, strerror(errno));
		return(NULL);
	}

	unsigned long magic = 0;

	if(fread(&magic, sizeof(magic), 1, in) != 1) {
		fprintf(stderr, "Error reading header to packet file: %s\n", strerror(errno));
		fclose(in);
		return(NULL);
	}

	PacketFileReader *ret = NULL;
	if(magic == OLD_PACKET_FILE_MAGIC) {
		ret = new OldPacketFileReader();
	} else if(magic == PACKET_FILE_MAGIC) {
		ret = new NewPacketFileReader();
	} else {
		fprintf(stderr, "Unknown packet file type 0x%.8x\n", magic);
		fclose(in);
		return(NULL);
	}

	if(!ret->OpenFile(name)) {
		safe_delete(ret);
		return(NULL);
	}

	return(ret);
}

PacketFileReader::PacketFileReader() {
	packet_file_stamp = 0;
}

OldPacketFileReader::OldPacketFileReader()
: PacketFileReader()
{
	in = NULL;
}

OldPacketFileReader::~OldPacketFileReader() {
	CloseFile();
}

bool OldPacketFileReader::OpenFile(const char *name) {
	CloseFile();


	in = fopen(name, "rb");
	if(in == NULL) {
		fprintf(stderr, "Error opening packet file '%s': %s\n", name, strerror(errno));
		return(false);
	}

	OldPacketFileHeader head;

	if(fread(&head, sizeof(head), 1, in) != 1) {
		fprintf(stderr, "Error reading header to packet file: %s\n", strerror(errno));
		fclose(in);
		return(false);
	}

	if(head.packet_file_magic != OLD_PACKET_FILE_MAGIC) {
		fclose(in);
		if(head.packet_file_magic > (OLD_PACKET_FILE_MAGIC)) {
			fprintf(stderr, "Error: this is a build file, not a packet file, its allready processed!\n");
		} else {
			fprintf(stderr, "Error: this is not a packet file!\n");
		}
		return(false);
	}

	uint32 now = time(NULL);
	if(head.packet_file_stamp > now) {
		fprintf(stderr, "Error: invalid timestamp in file. Your clock or the collector's is wrong (%d sec ahead).\n", head.packet_file_stamp-now);
		fclose(in);
		return(false);
	}

	packet_file_stamp = head.packet_file_stamp;

	return(true);
}

void OldPacketFileReader::CloseFile() {
	if(in != NULL) {
		fclose(in);
		in = NULL;
	}
}

bool OldPacketFileReader::ResetFile() {
	if(in == NULL)
		return(false);
	rewind(in);

	//gotta read past the header again
	OldPacketFileHeader head;

	if(fread(&head, sizeof(head), 1, in) != 1) {
		return(false);
	}

	return(true);
}

bool OldPacketFileReader::ReadPacket(uint16 &eq_op, uint32 &packlen, unsigned char *packet, bool &to_server, struct timeval &tv) {
	if(in == NULL)
		return(false);
	if(feof(in))
		return(false);

	OldPacketFileSection s;

	if(fread(&s, sizeof(s), 1, in) != 1) {
		if(!feof(in))
			fprintf(stderr, "Error reading section header: %s\n", strerror(errno));
		return(false);
	}

	eq_op = s.opcode;

	if(packlen < s.len) {
		fprintf(stderr, "Packet buffer is too small! %d < %d, skipping\n", packlen, s.len);
		fseek(in, s.len, SEEK_CUR);
		return(false);
	}

	if(fread(packet, 1, s.len, in) != s.len) {
		if(feof(in))
			fprintf(stderr, "Error: EOF encountered when expecting packet data.\n");
		else
			fprintf(stderr, "Error reading packet body: %s\n", strerror(errno));
		return(false);
	}

	packlen = s.len;
	to_server = false;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	return(true);
}


NewPacketFileReader::NewPacketFileReader()
: PacketFileReader()
{
	in = NULL;
}

NewPacketFileReader::~NewPacketFileReader() {
	CloseFile();
}

bool NewPacketFileReader::OpenFile(const char *name) {
	CloseFile();


	in = fopen(name, "rb");
	if(in == NULL) {
		fprintf(stderr, "Error opening packet file '%s': %s\n", name, strerror(errno));
		return(false);
	}

	PacketFileHeader head;

	if(fread(&head, sizeof(head), 1, in) != 1) {
		fprintf(stderr, "Error reading header to packet file: %s\n", strerror(errno));
		fclose(in);
		return(false);
	}

	if(head.packet_file_magic != PACKET_FILE_MAGIC) {
		fclose(in);
		if(head.packet_file_magic == (PACKET_FILE_MAGIC+1)) {
			fprintf(stderr, "Error: this is a build file, not a packet file, its allready processed!\n");
		} else {
			fprintf(stderr, "Error: this is not a packet file!\n");
		}
		return(false);
	}

	uint32 now = time(NULL);
	if(head.packet_file_stamp > now) {
		fprintf(stderr, "Error: invalid timestamp in file. Your clock or the collector's is wrong (%d sec ahead).\n", head.packet_file_stamp-now);
		fclose(in);
		return(false);
	}

	packet_file_stamp = head.packet_file_stamp;

	return(true);
}

void NewPacketFileReader::CloseFile() {
	if(in != NULL) {
		fclose(in);
		in = NULL;
	}
}

bool NewPacketFileReader::ResetFile() {
	if(in == NULL)
		return(false);
	rewind(in);

	//gotta read past the header again
	PacketFileHeader head;

	if(fread(&head, sizeof(head), 1, in) != 1) {
		return(false);
	}

	return(true);
}

bool NewPacketFileReader::ReadPacket(uint16 &eq_op, uint32 &packlen, unsigned char *packet, bool &to_server, struct timeval &tv) {
	if(in == NULL)
		return(false);
	if(feof(in))
		return(false);

	PacketFileSection s;

	if(fread(&s, sizeof(s), 1, in) != 1) {
		if(!feof(in))
			fprintf(stderr, "Error reading section header: %s\n", strerror(errno));
		return(false);
	}

	eq_op = s.opcode;

	if(packlen < s.len) {
		fprintf(stderr, "Packet buffer is too small! %d < %d, skipping\n", packlen, s.len);
		fseek(in, s.len, SEEK_CUR);
		return(false);
	}

	if(fread(packet, 1, s.len, in) != s.len) {
		if(feof(in))
			fprintf(stderr, "Error: EOF encountered when expecting packet data.\n");
		else
			fprintf(stderr, "Error reading packet body: %s\n", strerror(errno));
		return(false);
	}

	packlen = s.len;
	to_server = IsToServer(s);
	tv.tv_sec = s.tv_sec;
	tv.tv_usec = 1000*s.tv_msec;

	return(true);
}








