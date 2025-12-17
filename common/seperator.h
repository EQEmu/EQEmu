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
// This class will split up a string smartly at the div character (default is space and tab)
// Seperator.arg[i] is a copy of the string chopped at the divs
// Seperator.argplus[i] is a pointer to the original string so it doesnt end at the div

// Written by Quagmire

#pragma once

#include "common/types.h"

#include <stdlib.h>
#include <string.h>

class Seperator
{
public:
	Seperator(const char* message_in, char div = ' ', uint16 in_maxargnum = 10, uint16 arglen = 100, bool iObeyQuotes = false, char div2 = '\t', char div3 = 0, bool iSkipEmpty = true) {
		int i;
		argnum = 0;
		int len = static_cast<int>(strlen(message_in));

		if(arglen > len)
			arglen = len+1;

		//msg = strdup(message);
		msg = new char[len+1];
		strcpy(msg, message_in);
		const char *message = msg;
		this->maxargnum = in_maxargnum;
		argplus = new const char *[maxargnum+1];
		arg = new char *[maxargnum+1];
		for (i=0; i<=maxargnum; i++) {
			argplus[i]=arg[i] = new char[arglen+1];
			memset(arg[i], 0, arglen+1);
		}

		int s = 0, l = 0;
		bool inarg = (!iSkipEmpty || !(message[0] == div || message[0] == div2 || message[0] == div3));
		bool inquote = (iObeyQuotes && (message[0] == '\"' || message[0] == '\''));
		argplus[0] = message;
		if (len == 0)
			return;

		for (i=0; i<len; i++) {
//			std::cout << i << ": 0x" << std::hex << (int) message[i] << std::dec << " " << message[i] << std::endl; // undefined cout [CODEBUG]
			if (inarg) {
				if ((inquote == false && (message[i] == div || message[i] == div2 || message[i] == div3)) || (inquote && (message[i] == '\'' || message[i] == '\"') && (message[i+1] == div || message[i+1] == div2 || message[i+1] == div3 || message[i+1] == 0))) {
					inquote = false;
					l = i-s;
					if (l >= arglen)
						l = arglen;
					if (l)
						memcpy(arg[argnum], argplus[argnum], l);
					arg[argnum][l] = 0;
					argnum++;
					if (iSkipEmpty)
						inarg = false;
					else {
						s=i+1;
						argplus[argnum] = &message[s];
					}
				}
			}
			else if (iObeyQuotes && (message[i] == '\"' || message[i] == '\'')) {
				inquote = true;
			}
			else {
				s = i;
				argplus[argnum] = &message[s];
				if (!(message[i] == div || message[i] == div2 || message[i] == div3)) {
					inarg = true;
				}
			}
			if (argnum > maxargnum)
				break;
		}
		if (inarg && argnum <= maxargnum) {
			l = i-s;
			if (l >= arglen)
				l = arglen;
			if (l)
				memcpy(arg[argnum], argplus[argnum], l);
		}
	}
	~Seperator() {
		for (int i=0; i<=maxargnum; i++) {
			safe_delete_array(arg[i]);
		}
		safe_delete_array(arg);
		safe_delete_array(argplus);
		safe_delete_array(msg);
	}
	uint16 argnum;
	char** arg;
	const char** argplus;
	char * msg;
	bool IsNumber(int num) const {
		return IsNumber(arg[num]);
	}
	bool IsHexNumber(int num) const {
		return IsHexNumber(arg[num]);
	}
	static bool IsNumber(const char* check) {
		bool SeenDec = false;
		int len = static_cast<int>(strlen(check));
		if (len == 0) {
			return false;
		}
		int i;
		for (i = 0; i < len; i++) {
			if (check[i] < '0' || check[i] > '9') {
				if (check[i] == '.' && !SeenDec) {
					SeenDec = true;
				}
				else if (i == 0 && (check[i] == '-' || check[i] == '+') && check[i + 1] != '\0') {
					// this is ok, do nothin
				}
				else {
					return false;
				}
			}
		}
		return true;
	}
	static bool IsHexNumber(char* check) {
		int len = static_cast<int>(strlen(check));
		if (len < 3)
			return false;
		if (check[0] != '0' || (check[1] != 'x' && check[1] != 'X'))
			return false;
		for (int i=2; i<len; i++) {
			if ((check[i] < '0' || check[i] > '9') && (check[i] < 'A' || check[i] > 'F') && (check[i] < 'a' || check[i] > 'f'))
				return false;
		}
		return true;
	}
	inline uint16 GetMaxArgNum() const { return maxargnum; }
private:
	uint16 maxargnum;
};
