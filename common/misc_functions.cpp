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

#include "misc_functions.h"

#include "common/platform/inet.h"
#include "common/platform/platform.h"
#include "common/seperator.h"
#include "common/strings.h"
#include "common/timer.h"
#include "common/types.h"

#include <ctime>

#ifndef _WINDOWS
#include <sys/stat.h>
#include <netdb.h>
#endif

uint32 ResolveIP(const char* hostname, char* errbuf) {
#ifdef _WINDOWS
	static InitWinsock ws;
#endif
	if (errbuf)
		errbuf[0] = 0;
	if (hostname == 0) {
		if (errbuf)
			snprintf(errbuf, ERRBUF_SIZE, "ResolveIP(): hostname == 0");
		return 0;
	}
	struct sockaddr_in	server_sin;
#ifdef _WINDOWS
	PHOSTENT phostent = nullptr;
#else
	struct hostent *phostent = nullptr;
#endif
	server_sin.sin_family = AF_INET;
	if ((phostent = gethostbyname(hostname)) == nullptr) {
#ifdef _WINDOWS
		if (errbuf)
			snprintf(errbuf, ERRBUF_SIZE, "Unable to get the host name. Error: %i", WSAGetLastError());
#else
		if (errbuf)
			snprintf(errbuf, ERRBUF_SIZE, "Unable to get the host name. Error: %s", strerror(errno));
#endif
		return 0;
	}
#ifdef _WINDOWS
	memcpy ((char FAR *)&(server_sin.sin_addr), phostent->h_addr, phostent->h_length);
#else
	memcpy ((char*)&(server_sin.sin_addr), phostent->h_addr, phostent->h_length);
#endif
	return server_sin.sin_addr.s_addr;
}

bool ParseAddress(const char* iAddress, uint32* oIP, uint16* oPort, char* errbuf) {
	Seperator sep(iAddress, ':', 2, 250, false, 0, 0);
	if (sep.argnum == 1 && sep.IsNumber(1)) {
		*oIP = ResolveIP(sep.arg[0], errbuf);
		if (*oIP == 0)
			return false;
		if (oPort)
			*oPort = Strings::ToUnsignedInt(sep.arg[1]);
		return true;
	}
	return false;
}

#ifdef _WINDOWS
InitWinsock::InitWinsock() {
	WORD version = MAKEWORD (1,1);
	WSADATA wsadata;
	WSAStartup (version, &wsadata);
}

InitWinsock::~InitWinsock() {
	WSACleanup();
}
#endif


const char * itoa(int num) {
		static char temp[_ITOA_BUFLEN];
		memset(temp,0,_ITOA_BUFLEN);
		snprintf(temp,_ITOA_BUFLEN,"%d",num);
		return temp;
}

#ifndef WIN32
const char * itoa(int num, char* a,int b) {
		static char temp[_ITOA_BUFLEN];
		memset(temp,0,_ITOA_BUFLEN);
		snprintf(temp,_ITOA_BUFLEN,"%d",num);
		return temp;
		return temp;
}
#endif

uint32 rnd_hash( time_t t, clock_t c )
{
	// Get a uint32 from t and c
	// Better than uint32(x) in case x is floating point in [0,1]
	// Based on code by Lawrence Kirby (fred@genesis.demon.co.uk)

	static uint32 differ = 0; // guarantee time-based seeds will change

	uint32 h1 = 0;
	unsigned char *p = (unsigned char *) &t;
	for( size_t i = 0; i < sizeof(t); ++i )
	{
		h1 *= 255 + 2U;
		h1 += p[i];
	}
	uint32 h2 = 0;
	p = (unsigned char *) &c;
	for( size_t j = 0; j < sizeof(c); ++j )
	{
		h2 *= 255 + 2U;
		h2 += p[j];
	}
	return ( h1 + differ++ ) ^ h2;
}

float EQ13toFloat(int d)
{
	return static_cast<float>(d) / 64.0f;
}

float EQ19toFloat(int d)
{
	return static_cast<float>(d) / 8.0f;
}

int FloatToEQ13(float d)
{
	return static_cast<int>(d * 64.0f);
}

int FloatToEQ19(float d)
{
	return static_cast<int>(d * 8.0f);
}

float EQ12toFloat(int d)
{
	return static_cast<float>(d) / 4.0f;
}

int FloatToEQ12(float d)
{
	return static_cast<int>((d + 2048.0f) * 4.0f) % 2048;
}

float EQ10toFloat(int d)
{
	return static_cast<float>(d) / 20.0f;
}

int FloatToEQ10(float d)
{
	return static_cast<int>(d * 20.0f);
}

float EQSpeedRunToFloat(int d)
{
	return static_cast<float>(d) / 40.0f;
}

int FloatToEQSpeedRun(float d)
{
	return static_cast<int>(d * 40.0f);
}

float FixHeading(float in)
{
	int i = 0;
	if (in >= 512.0f) {
		do {
			in -= 512.0f;
		} while (in >= 512.0f && i++ <= 5);
	}
	i = 0;
	if (in < 0.0f) {
		do {
			in += 512.0f;
		} while (in < 0.0f && i++ <= 5);
	}

	return in;
}

/*
	Heading of 0 points in the pure positive Y direction

*/
int FloatToEQH(float d)
{
	return(int((360.0f - d) * float(1<<11)) / 360);
}

float EQHtoFloat(int d)
{
	return(360.0f - float((d * 360) >> 11));
}

// returns a swapped-bit value for use in client translator and inventory functions
uint32 SwapBits21And22(uint32 mask)
{
	static const uint32 BIT21 = 1 << 21;
	static const uint32 BIT22 = 1 << 22;
	static const uint32 SWAPBITS = (BIT21 | BIT22);

	if ((bool)(mask & BIT21) != (bool)(mask & BIT22))
		mask ^= SWAPBITS;

	return mask;
}

// returns an unset bit 22 value for use in client translators
uint32 Catch22(uint32 mask)
{
	static const uint32 KEEPBITS = ~(1 << 22);

	mask &= KEEPBITS;

	return mask;
}
