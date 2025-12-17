/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

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

#include "common/global_define.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdio.h>

//#ifdef _CRTDBG_MAP_ALLOC
//	#undef new
//	#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
//#endif
#include <string.h>

#ifdef _WINDOWS
	#define snprintf	_snprintf
	#define strncasecmp	_strnicmp
	#define strcasecmp	_stricmp
#else
	#include <stdarg.h>
#endif

#include "eq_stream_intf.h"
#include "packet_dump_file.h"

void FileDumpPacketAscii(const char* filename, const uchar* buf, uint32 size, uint32 cols, uint32 skip) {
	std::ofstream logfile(filename, std::ios::app);
	// Output as ASCII
	for(uint32 i=skip; i<size; i++)
	{
		if ((i-skip)%cols==0)
		{
			logfile << std::endl << std::setw(3) << std::setfill(' ') << i-skip << ":";
		}
		else if ((i-skip)%(cols/2)==0)
		{
			logfile << " - ";
		}
		if (buf[i] > 32 && buf[i] < 127)
		{
			logfile << buf[i];
		}
		else
		{
			logfile << '.';
		}
	}
	logfile << std::endl << std::endl;
}

void oldFileDumpPacketHex(const char* filename, const uchar* buf, uint32 size, uint32 cols, uint32 skip)
{
	std::ofstream logfile(filename, std::ios::app);
	// Output as HEX
	char output[4];
	for(uint32 i=skip; i<size; i++)
	{
		if ((i-skip)%cols==0)
		{
			logfile << std::endl << std::setw(3) << std::setfill(' ') << i-skip << ": ";
		}
		else if ((i-skip)%(cols/2) == 0)
		{
			logfile << "- ";
		}
		sprintf(output, "%02X ",(unsigned char)buf[i]);
		logfile << output;
		//logfile << std::setfill(0) << std::setw(2) << std::hex << (int)buf[i] << " "; // unknown intent [CODEBUG]
	}
	logfile << std::endl << std::endl;
}

void FileDumpPacketHex(const char* filename, const uchar* buf, uint32 size, uint32 cols, uint32 skip)
{
	if (size == 0)
		return;
	std::ofstream logfile(filename, std::ios::app);
	// Output as HEX
	char output[4];
	int j = 0;
	auto ascii = new char[cols + 1];
	memset(ascii, 0, cols + 1);
	uint32 i;
	for(i=skip; i<size; i++)
	{
		if ((i-skip)%cols==0) {
			if (i != skip)
				logfile << " | " << ascii << std::endl;
			logfile << std::setw(4) << std::setfill(' ') << i-skip << ": ";
			memset(ascii, 0, cols+1);
			j = 0;
		}
		else if ((i-skip)%(cols/2) == 0) {
			logfile << "- ";
		}
		sprintf(output, "%02X ", (unsigned char)buf[i]);
		logfile << output;

		if (buf[i] >= 32 && buf[i] < 127) {
			ascii[j++] = buf[i];
		}
		else {
			ascii[j++] = '.';
		}
		//logfile << std::setfill(0) << std::setw(2) << std::hex << (int)buf[i] << " "; // unknown intent [CODEBUG]
	}
	uint32 k = ((i-skip)-1)%cols;
	if (k < 8)
		logfile << "  ";
	for (uint32 h = k+1; h < cols; h++) {
		logfile << "   ";
	}
	logfile << " | " << ascii << std::endl;
	delete[] ascii;
}

void FileDumpPacketHex(const char* filename, const EQApplicationPacket* app)
{
	FileDumpPacketHex(filename, app->pBuffer, app->size);
}

void FileDumpPacketAscii(const char* filename, const EQApplicationPacket* app)
{
	FileDumpPacketAscii(filename, app->pBuffer, app->size);
}

void FileDumpPacket(const char* filename, const uchar* buf, uint32 size)
{
	FilePrintLine(filename, true, "Size: %5i", size);
	FileDumpPacketHex(filename, buf, size);
//	FileDumpPacketAscii(filename, buf,size);
}

void FileDumpPacket(const char* filename, const EQApplicationPacket* app)
{
	FilePrintLine(filename, true, "Size: %5i, OPCode: 0x%04x", app->size, app->GetOpcode());
	FileDumpPacketHex(filename, app->pBuffer, app->size);
//	FileDumpPacketAscii(filename, app->pBuffer, app->size);
}

/*
	prints a line to the file. if text = 0, prints a blank line
	if prefix_timestamp specified, prints the current date/time to the file + ": " + text
*/
void FilePrintLine(const char* filename, bool prefix_timestamp, const char* text, ...) {
	std::ofstream logfile(filename, std::ios::app);
	if (prefix_timestamp) {
		time_t rawtime;
		struct tm* gmt_t;
		time(&rawtime);
		gmt_t = gmtime(&rawtime);
		logfile << (gmt_t->tm_year + 1900) << "/" << std::setw(2) << std::setfill('0') << (gmt_t->tm_mon + 1) << "/" << std::setw(2) << std::setfill('0') << gmt_t->tm_mday << " " << std::setw(2) << std::setfill('0') << gmt_t->tm_hour << ":" << std::setw(2) << std::setfill('0') << gmt_t->tm_min << ":" << std::setw(2) << std::setfill('0') << gmt_t->tm_sec << " GMT";
	}

	if (text != 0) {
		va_list argptr;
		char buffer[256];
		va_start(argptr, text);
		vsnprintf(buffer, 256, text, argptr);
		va_end(argptr);

		if (prefix_timestamp)
			logfile << ": ";
		logfile << buffer;
	}
	logfile << std::endl;
}

void FilePrint(const char* filename, bool newline, bool prefix_timestamp, const char* text, ...) {
	std::ofstream logfile(filename, std::ios::app);
	if (prefix_timestamp) {
		time_t rawtime;
		struct tm* gmt_t;
		time(&rawtime);
		gmt_t = gmtime(&rawtime);
		logfile << (gmt_t->tm_year + 1900) << "/" << std::setw(2) << std::setfill('0') << (gmt_t->tm_mon + 1) << "/" << std::setw(2) << std::setfill('0') << gmt_t->tm_mday << " " << std::setw(2) << std::setfill('0') << gmt_t->tm_hour << ":" << std::setw(2) << std::setfill('0') << gmt_t->tm_min << ":" << std::setw(2) << std::setfill('0') << gmt_t->tm_sec << " GMT";
	}

	if (text != 0) {
		va_list argptr;
		char buffer[1000];
		va_start(argptr, text);
		vsnprintf(buffer,1000, text, argptr);
		va_end(argptr);

		if (prefix_timestamp)
			logfile << ": ";
		logfile << buffer;
	}

	if (newline)
		logfile << std::endl;
}

