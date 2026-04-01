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

#include <map>
#include <cstdio>
#include <string>

#define ITEMFIELDCOUNT 116

void Unprotect(std::string &s, char what);

void Protect(std::string &s, char what);

bool ItemParse(const char *data, int length, std::map<int,std::map<int,std::string> > &items, int id_pos, int name_pos, int max_field, int level=0);

int Tokenize(std::string s, std::map<int,std::string> & tokens, char delim='|');

#ifndef WIN32
int print_stacktrace();
#endif

void dump_message_column(unsigned char *buffer, unsigned long length, std::string leader="", FILE *to = stdout);
std::string string_from_time(std::string pattern, time_t now=0);
std::string timestamp(time_t now=0);
std::string long2ip(unsigned long ip);
std::string pop_arg(std::string &s, std::string seps, bool obey_quotes);
int EQsprintf(char *buffer, const char *pattern, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5, const char *arg6, const char *arg7, const char *arg8, const char *arg9);
void build_hex_line(const char *buffer, unsigned long length, unsigned long offset, char *out_buffer, unsigned char padding=4);
void print_hex(const char *buffer, unsigned long length);
