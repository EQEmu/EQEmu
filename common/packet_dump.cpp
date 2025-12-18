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

#include "packet_dump.h"

#include "common/servertalk.h"

#include <cstdio>
#include <iostream>
#include <sstream>
#include <iomanip>

void DumpPacketAscii(const uchar* buf, uint32 size, uint32 cols, uint32 skip) {
	// Output as ASCII
	for(uint32 i=skip; i<size; i++)
	{
		if ((i-skip)%cols==0)
		{
			std::cout << std::endl << std::setw(3) << std::setfill(' ') << i-skip << ":";
		}
		else if ((i-skip)%(cols/2)==0)
		{
			std::cout << " - ";
		}
		if (buf[i] > 32 && buf[i] < 127)
		{
			std::cout << buf[i];
		}
		else
		{
			std::cout << '.';
		}
	}
	std::cout << std::endl << std::endl;
}

void DumpPacketHex(const uchar* buf, uint32 size, uint32 cols, uint32 skip) {
	if (size == 0 || size > 39565)
		return;
	// Output as HEX
	char output[4];
	int j = 0;
	auto ascii = new char[cols + 1];
	memset(ascii, 0, cols+1);
	uint32 i;
	for(i=skip; i<size; i++)
	{
		if ((i-skip)%cols==0) {
			if (i != skip)
				std::cout << " | " << ascii << std::endl;
			std::cout << std::setw(4) << std::setfill(' ') << i-skip << ": ";
			memset(ascii, 0, cols+1);
			j = 0;
		}
		else if ((i-skip)%(cols/2) == 0) {
			std::cout << "- ";
		}
		sprintf(output, "%02X ", (unsigned char)buf[i]);
		std::cout << output;

		if (buf[i] >= 32 && buf[i] < 127) {
			ascii[j++] = buf[i];
		}
		else {
			ascii[j++] = '.';
		}
//		std::cout << std::setfill(0) << std::setw(2) << std::hex << (int)buf[i] << " "; // unknown intent [CODEBUG]
	}
	uint32 k = ((i-skip)-1)%cols;
	if (k < 8)
		std::cout << "  ";
	for (uint32 h = k+1; h < cols; h++) {
		std::cout << "   ";
	}
	std::cout << " | " << ascii << std::endl;
	safe_delete_array(ascii);
}

std::string DumpPacketHexToString(const uchar* buf, uint32 size, uint32 cols, uint32 skip) {
	std::ostringstream out;
	if (size == 0 || size > 39565)
		return "";

	out << "\n";
	
	// Output as HEX
	char output[4];
	int j = 0;
	auto ascii = new char[cols + 1];
	memset(ascii, 0, cols + 1);
	uint32 i;
	for (i = skip; i < size; i++)
	{
		if ((i - skip) % cols == 0) {
			if (i != skip)
				out << " | " << ascii << std::endl;
				out << std::setw(4) << std::setfill(' ') << i - skip << ": ";
			memset(ascii, 0, cols + 1);
			j = 0;
		}
		else if ((i - skip) % (cols / 2) == 0) {
			out << "- ";
		}
		sprintf(output, "%02X ", (unsigned char)buf[i]);
		out << output;

		if (buf[i] >= 32 && buf[i] < 127) {
			ascii[j++] = buf[i];
		}
		else {
			ascii[j++] = '.';
		}
		//		std::cout << std::setfill(0) << std::setw(2) << std::hex << (int)buf[i] << " "; // unknown intent [CODEBUG]
	}
	uint32 k = ((i - skip) - 1) % cols;
	if (k < 8)
		out << "  ";
	for (uint32 h = k + 1; h < cols; h++) {
		out << "   ";
	}
	out << " | " << ascii << std::endl;
	safe_delete_array(ascii);

	return out.str();
}

void DumpPacket(const uchar* buf, uint32 size)
{
	DumpPacketHex(buf, size);
//	DumpPacketAscii(buf,size);
}

void DumpPacket(const ServerPacket* pack, bool iShowInfo) {
	if (iShowInfo) {
		std::cout << "Dumping ServerPacket: 0x" << std::hex << std::setfill('0') << std::setw(4) << pack->opcode << std::dec;
		std::cout << " size:" << pack->size << std::endl;
	}
	DumpPacketHex(pack->pBuffer, pack->size);
}

void DumpPacket(uint16 opcode, const EQ::Net::Packet &p, bool iShowInfo) {
	if (iShowInfo) {
		std::cout << "Dumping ServerPacket: 0x" << std::hex << std::setfill('0') << std::setw(4) << opcode << std::dec;
		std::cout << " size:" << p.Length() << std::endl;
	}
	
	std::cout << p.ToString() << std::endl;
}

void DumpPacketBin(const ServerPacket* pack) {
	DumpPacketBin(pack->pBuffer, pack->size);
}

void DumpPacketBin(uint32 data) {
	DumpPacketBin((uchar*)&data, sizeof(uint32));
}

void DumpPacketBin(uint16 data) {
	DumpPacketBin((uchar*)&data, sizeof(uint16));
}

void DumpPacketBin(uint8 data) {
	DumpPacketBin((uchar*)&data, sizeof(uint8));
}


void DumpPacketBin(const void* iData, uint32 len) {
	if (!len)
		return;
	const uint8* data = (const uint8*) iData;
	uint32 k=0;
	for (k=0; k<len; k++) {
		if (k % 4 == 0) {
			if (k != 0) {
				std::cout << " | " << std::hex << std::setw(2) << std::setfill('0') << (int) data[k-4] << std::dec;
				std::cout << " " << std::hex << std::setw(2) << std::setfill('0') << (int) data[k-3] << std::dec;
				std::cout << " " << std::hex << std::setw(2) << std::setfill('0') << (int) data[k-2] << std::dec;
				std::cout << " " << std::hex << std::setw(2) << std::setfill('0') << (int) data[k-1] << std::dec;
				std::cout << std::endl;
			}
			std::cout << std::setw(4) << std::setfill('0') << k << ":";
		}
		else if (k % 2 == 0)
			std::cout << " ";
		std::cout << " ";
		if (data[k] & 1)
			std::cout << "1";
		else
			std::cout << "0";
		if (data[k] & 2)
			std::cout << "1";
		else
			std::cout << "0";
		if (data[k] & 4)
			std::cout << "1";
		else
			std::cout << "0";
		if (data[k] & 8)
			std::cout << "1";
		else
			std::cout << "0";
		if (data[k] & 16)
			std::cout << "1";
		else
			std::cout << "0";
		if (data[k] & 32)
			std::cout << "1";
		else
			std::cout << "0";
		if (data[k] & 64)
			std::cout << "1";
		else
			std::cout << "0";
		if (data[k] & 128)
			std::cout << "1";
		else
			std::cout << "0";
	}
	uint8 tmp = (k % 4);
	if (!tmp)
		tmp = 4;
	if (tmp <= 3)
		std::cout << "         ";
	if (tmp <= 2)
		std::cout << "          ";
	if (tmp <= 1)
		std::cout << "         ";
	std::cout << " | " << std::hex << std::setw(2) << std::setfill('0') << (int) data[k-4] << std::dec;
	if (tmp > 1)
		std::cout << " " << std::hex << std::setw(2) << std::setfill('0') << (int) data[k-3] << std::dec;
	if (tmp > 2)
		std::cout << " " << std::hex << std::setw(2) << std::setfill('0') << (int) data[k-2] << std::dec;
	if (tmp > 3)
		std::cout << " " << std::hex << std::setw(2) << std::setfill('0') << (int) data[k-1] << std::dec;
	std::cout << std::endl;
}
