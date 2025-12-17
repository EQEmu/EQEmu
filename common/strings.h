// for folly stuff
/*
 * Copyright 2013 Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
// for our stuff
/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2022 EQEMu Development Team (http://eqemulator.net)

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

#pragma once

#include "common/types.h"

#include <cctype>
#include <charconv>
#include <cstdarg>
#include <cstring>
#include <string_view>
#include <string>
#include <type_traits>
#include <vector>

class Strings {
public:
	static bool Contains(std::vector<std::string> container, const std::string& element);
	static bool Contains(const std::string& subject, const std::string& search);
	static bool ContainsLower(const std::string& subject, const std::string& search);
	static int ToInt(const std::string &s, int fallback = 0);
	static int64 ToBigInt(const std::string &s, int64 fallback = 0);
	static uint32 ToUnsignedInt(const std::string &s, uint32 fallback = 0);
	static uint64 ToUnsignedBigInt(const std::string &s, uint64 fallback = 0);
	static float ToFloat(const std::string &s, float fallback = 0.0f);
	static bool IsNumber(const std::string &s);
	static std::string RemoveNumbers(std::string s);
	static bool IsFloat(const std::string &s);
	static const std::string ToLower(std::string s);
	static const std::string ToUpper(std::string s);
	static const std::string UcFirst(const std::string& s);
	static std::string &LTrim(std::string &str, std::string_view chars = "\t\n\v\f\r ");
	static std::string &RTrim(std::string &str, std::string_view chars = "\t\n\v\f\r ");
	static std::string &Trim(std::string &str, const std::string &chars = "\t\n\v\f\r ");
	static std::string Commify(const std::string &number);
	static std::string Commify(uint16 number) { return Strings::Commify(std::to_string(number)); };
	static std::string Commify(uint32 number) { return Strings::Commify(std::to_string(number)); };
	static std::string Commify(uint64 number) { return Strings::Commify(std::to_string(number)); };
	static std::string Commify(int16 number) { return Strings::Commify(std::to_string(number)); };
	static std::string Commify(int32 number) { return Strings::Commify(std::to_string(number)); };
	static std::string Commify(int64 number) { return Strings::Commify(std::to_string(number)); };
	static std::string ConvertToDigit(int n, const std::string& suffix);
	static std::string Escape(const std::string &s);
	static std::string GetBetween(const std::string &s, std::string start_delim, std::string stop_delim);
	static std::string Implode(const std::string& glue, std::vector<std::string> src);
	static std::string Join(const std::vector<std::string> &ar, const std::string &delim);
	static std::string Join(const std::vector<uint32_t> &ar, const std::string &delim);
	static std::string MillisecondsToTime(int duration);
	static std::string Money(uint64 platinum, uint64 gold = 0, uint64 silver = 0, uint64 copper = 0);
	static std::string NumberToWords(unsigned long long int n);
	static std::string Repeat(std::string s, int n);
	static std::string Replace(std::string subject, const std::string &search, const std::string &replace);
	static std::string SecondsToTime(int duration, bool is_milliseconds = false);
	static std::string::size_type SearchDelim(const std::string &haystack, const std::string &needle, const char deliminator = ',');
	static std::vector<std::string> Split(const std::string &s, const char delim = ',');
	static std::vector<std::string> Split(const std::string& s, const std::string& delimiter);
	static std::vector<std::string> Wrap(std::vector<std::string> &src, const std::string& character);
	static void FindReplace(std::string &string_subject, const std::string &search_string, const std::string &replace_string);
	static uint32 TimeToSeconds(std::string time_string);
	static bool ToBool(const std::string& bool_string);
	static inline bool EqualFold(const std::string &string_one, const std::string &string_two) { return strcasecmp(string_one.c_str(), string_two.c_str()) == 0; }
	static std::string Random(size_t length);
	static bool BeginsWith(const std::string& subject, const std::string& search);
	static bool EndsWith(const std::string& subject, const std::string& search);
	static std::string ZoneTime(const uint8 hours, const uint8 minutes);
	static std::string Slugify(const std::string &input, const std::string &separator = "-");
	static bool IsValidJson(const std::string& json);
};

const std::string StringFormat(const char *format, ...);
const std::string vStringFormat(const char *format, va_list args);

// For converstion of numerics into English
// Used for grid nodes, as NPC names remove numerals.
// But general purpose

// misc functions
std::string SanitizeWorldServerName(std::string server_long_name);
std::vector<std::string> GetBadWords();
void ParseAccountString(const std::string &s, std::string &account, std::string &loginserver);

// old c string functions

bool atobool(const char *iBool);
bool isAlphaNumeric(const char *text);
bool strn0cpyt(char *dest, const char *source, uint32 size);
char *CleanMobName(const char *in, char *out);
char *RemoveApostrophes(const char *s);
char *strn0cpy(char *dest, const char *source, uint32 size);
const char *ConvertArray(int64 input, char *returnchar);
const char *ConvertArrayF(float input, char *returnchar);
const char *MakeLowerString(const char *source);
uint32 hextoi(const char *num);
uint64 hextoi64(const char *num);
void MakeLowerString(const char *source, char *target);
void RemoveApostrophes(std::string &s);
std::string FormatName(const std::string &char_name);
bool IsAllowedWorldServerCharacterList(char c);
void SanitizeWorldServerName(char *name);
