/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include "common/types.h"

class MD5
{
public:
	struct MD5Context {
		uint32 hash[4];
		uint32 bytes[2];
		uint32 input[16];
	};
	static void Generate(const uint8* buf, uint32 len, uint8 digest[16]);

	static void Init(struct MD5Context *context);
	static void Update(struct MD5Context *context, const uint8 *buf, uint32 len);
	static void Final(uint8 digest[16], struct MD5Context *context);

	MD5();
	MD5(const uchar* buf, uint32 len);
	MD5(const char* buf, uint32 len);
	MD5(const uint8 buf[16]);
	MD5(const char* iMD5String);

	void Generate(const char* iString);
	void Generate(const uint8* buf, uint32 len);
	bool Set(const uint8 buf[16]);
	bool Set(const char* iMD5String);

	bool	operator== (const MD5& iMD5);
	bool	operator== (const uint8 iMD5[16]);
	bool	operator== (const char* iMD5String);

	MD5&	operator= (const MD5& iMD5);
	MD5*	operator= (const MD5* iMD5);
	MD5*	operator= (const uint8* iMD5);
	operator const char* ();
protected:
	uint8	pMD5[16];
private:
	static void byteSwap(uint32 *buf, uint32 words);
	static void Transform(uint32 hash[4], const uint32 input[16]);
	char	pMD5String[33];
};
