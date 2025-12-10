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

#include "strings.h"
#include <cereal/external/rapidjson/document.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <algorithm>
#include <cctype>

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>

#include <random>
#include <string>

#ifdef _WINDOWS
#include <ctype.h>
#include <functional>
#endif

std::string Strings::Random(size_t length)
{
	static auto &chrs = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

	thread_local static std::mt19937 rg{std::random_device{}()};

	thread_local static std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);

	std::string s;

	s.reserve(length);

	while (length--) {
		s += chrs[pick(rg)];
	}

	return s;
}

std::vector<std::string> Strings::Split(const std::string &str, const char delim)
{
	std::vector<std::string> ret;
	std::string::size_type   start = 0;
	auto                     end   = str.find(delim);
	while (end != std::string::npos) {
		ret.emplace_back(str, start, end - start);
		start = end + 1;
		end   = str.find(delim, start);
	}
	// this will catch the last word since the string is unlikely to end with a delimiter
	if (str.length() > start) {
		ret.emplace_back(str, start, str.length() - start);
	}
	return ret;
}

// this one takes delimiter length into consideration
std::vector<std::string> Strings::Split(const std::string &s, const std::string &delimiter)
{
	size_t                   pos_start = 0, pos_end, delim_len = delimiter.length();
	std::string              token;
	std::vector<std::string> res;

	while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
		token     = s.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		res.push_back(token);
	}

	res.emplace_back(s.substr(pos_start));
	return res;
}

std::string Strings::Strings::GetBetween(const std::string &s, std::string start_delim, std::string stop_delim)
{
	if (s.find(start_delim) == std::string::npos && s.find(stop_delim) == std::string::npos) {
		return "";
	}

	auto first_split = Strings::Split(s, start_delim);
	if (!first_split.empty()) {
		std::string remaining_block = first_split[1];
		auto        second_split    = Strings::Split(remaining_block, stop_delim);
		if (!second_split.empty()) {
			std::string between = second_split[0];
			return between;
		}
	}

	return "";
}

std::string::size_type
Strings::SearchDelim(const std::string &haystack, const std::string &needle, const char deliminator)
{
	// this shouldn't go out of bounds, even without obvious bounds checks
	auto pos = haystack.find(needle);
	while (pos != std::string::npos) {
		auto c = haystack[pos + needle.length()];
		if ((c == '\0' || c == deliminator) && (pos == 0 || haystack[pos - 1] == deliminator)) {
			return pos;
		}
		pos = haystack.find(needle, pos + needle.length());
	}
	return std::string::npos;
}


std::string Strings::Implode(const std::string& glue, std::vector<std::string> src)
{
	if (src.empty()) {
		return {};
	}

	std::ostringstream                 output;
	std::vector<std::string>::iterator src_iter;

	for (src_iter = src.begin(); src_iter != src.end(); src_iter++) {
		output << *src_iter << glue;
	}

	std::string final_output = output.str();
	final_output.resize(output.str().size() - glue.size());

	return final_output;
}

std::vector<std::string> wrap(std::vector<std::string> &src, std::string character)
{
	std::vector<std::string> new_vector;
	new_vector.reserve(src.size());

	for (auto &e: src) {
		if (e == "null") {
			new_vector.emplace_back(e);
			continue;
		}

		new_vector.emplace_back(character + e + character);
	}

	return new_vector;
}

std::string Strings::Escape(const std::string &s)
{
	std::string ret;

	size_t      sz = s.length();
	for (size_t i  = 0; i < sz; ++i) {
		char c = s[i];
		switch (c) {
			case '\x00':
				ret += "\\x00";
				break;
			case '\n':
				ret += "\\n";
				break;
			case '\r':
				ret += "\\r";
				break;
			case '\\':
				ret += "\\\\";
				break;
			case '\'':
				ret += "\\'";
				break;
			case '\"':
				ret += "\\\"";
				break;
			case '\x1a':
				ret += "\\x1a";
				break;
			default:
				ret.push_back(c);
				break;
		}
	}

	return ret;
}

bool Strings::IsNumber(const std::string &s)
{
	for (char const &c: s) {
		if (c == s[0] && s[0] == '-') {
			continue;
		}
		if (std::isdigit(c) == 0) {
			return false;
		}
	}

	return true;
}

bool Strings::IsFloat(const std::string &s)
{
	char* ptr;
	strtof(s.c_str(), &ptr);
	return (*ptr) == '\0';
}

std::string Strings::Join(const std::vector<std::string> &ar, const std::string &delim)
{
	std::string ret;
	for (size_t i = 0; i < ar.size(); ++i) {
		if (i != 0) {
			ret += delim;
		}

		ret += ar[i];
	}

	return ret;
}

std::string Strings::Join(const std::vector<uint32_t> &ar, const std::string &delim)
{
	std::string ret;
	for (size_t i = 0; i < ar.size(); ++i) {
		if (i != 0) {
			ret += delim;
		}

		ret += std::to_string(ar[i]);
	}

	return ret;
}

void
Strings::FindReplace(std::string &string_subject, const std::string &search_string, const std::string &replace_string)
{
	if (string_subject.find(search_string) == std::string::npos) {
		return;
	}

	size_t start_pos = 0;
	while ((start_pos = string_subject.find(search_string, start_pos)) != std::string::npos) {
		string_subject.replace(start_pos, search_string.length(), replace_string);
		start_pos += replace_string.length();
	}
}

std::string Strings::Replace(std::string subject, const std::string &search, const std::string &replace)
{
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
	return subject;
}

std::string Strings::Repeat(std::string s, int n)
{
	std::string s1 = s;
	for (int    i  = 1; i < n; i++) {
		s += s1;
	}

	return s;
}

bool Strings::Contains(std::vector<std::string> container, const std::string& element)
{
	return std::find(container.begin(), container.end(), element) != container.end();
}

std::string Strings::Commify(const std::string &number)
{
	std::string temp_string;

	auto string_length = static_cast<int>(number.length());

	if (string_length == 3) {
		return number;
	} else if (string_length == 4 && number.starts_with("-")) {
		return number;
	}

	int i = 0;
	for (i = string_length - 3; i >= 0; i -= 3) {
		if (i > 0) {
			temp_string = "," + number.substr(static_cast<unsigned long>(i), 3) + temp_string;
		} else {
			temp_string = number.substr(static_cast<unsigned long>(i), 3) + temp_string;
		}
	}

	temp_string = number.substr(0, static_cast<unsigned long>(3 + i)) + temp_string;

	return temp_string;
}

const std::string Strings::ToLower(std::string s)
{
	std::transform(
		s.begin(), s.end(), s.begin(),
		[](unsigned char c) { return ::tolower(c); }
	);
	return s;
}
const std::string Strings::ToUpper(std::string s)
{
	std::transform(
		s.begin(), s.end(), s.begin(),
		[](unsigned char c) { return ::toupper(c); }
	);
	return s;
}
const std::string Strings::UcFirst(const std::string& s)
{
	std::string output = s;
	if (!s.empty()) {
		output[0] = static_cast<char>(::toupper(s[0]));
	}

	return output;
}


std::vector<std::string> Strings::Wrap(std::vector<std::string> &src, const std::string& character)
{
	std::vector<std::string> new_vector;
	new_vector.reserve(src.size());

	for (auto &e: src) {
		if (e == "null") {
			new_vector.emplace_back(e);
			continue;
		}

		new_vector.emplace_back(character + e + character);
	}

	return new_vector;
}

std::string Strings::NumberToWords(unsigned long long int n)
{
	// string to store word representation of given number
	std::string res;

	// this handles digits at ones & tens place
	res = Strings::ConvertToDigit((n % 100), "");

	if (n > 100 && n % 100) {
		res = "and " + res;
	}

	// this handles digit at hundreds place
	res = Strings::ConvertToDigit(((n / 100) % 10), "Hundred ") + res;

	// this handles digits at thousands & tens thousands place
	res = Strings::ConvertToDigit(((n / 1000) % 100), "Thousand ") + res;

	// this handles digits at hundred thousands & one millions place
	res = Strings::ConvertToDigit(((n / 100000) % 100), "Lakh, ") + res;

	// this handles digits at ten millions & hundred millions place
	res = Strings::ConvertToDigit((n / 10000000) % 100, "Crore, ") + res;

	// this handles digits at ten millions & hundred millions place
	res = Strings::ConvertToDigit((n / 1000000000) % 100, "Billion, ") + res;

	return res;
}

std::string Strings::Money(uint64 platinum, uint64 gold, uint64 silver, uint64 copper)
{
	std::string money_string = "Unknown";
	if (copper && silver && gold && platinum) { // CSGP
		money_string = fmt::format(
			"{} platinum, {} gold, {} silver, and {} copper",
			Strings::Commify(std::to_string(platinum)),
			Strings::Commify(std::to_string(gold)),
			Strings::Commify(std::to_string(silver)),
			Strings::Commify(std::to_string(copper))
		);
	}
	else if (copper && silver && !gold && platinum) { // CSP
		money_string = fmt::format(
			"{} platinum, {} silver, and {} copper",
			Strings::Commify(std::to_string(platinum)),
			Strings::Commify(std::to_string(silver)),
			Strings::Commify(std::to_string(copper))
		);
	}
	else if (copper && silver && gold && !platinum) { // CSG
		money_string = fmt::format(
			"{} gold, {} silver, and {} copper",
			Strings::Commify(std::to_string(gold)),
			Strings::Commify(std::to_string(silver)),
			Strings::Commify(std::to_string(copper))
		);
	}
	else if (copper && !silver && !gold && platinum) { // CP
		money_string = fmt::format(
			"{} platinum and {} copper",
			Strings::Commify(std::to_string(platinum)),
			Strings::Commify(std::to_string(copper))
		);
	}
	else if (copper && silver && !gold && !platinum) { // CS
		money_string = fmt::format(
			"{} silver and {} copper",
			Strings::Commify(std::to_string(silver)),
			Strings::Commify(std::to_string(copper))
		);
	}
	else if (!copper && silver && gold && platinum) { // SGP
		money_string = fmt::format(
			"{} platinum, {} gold, and {} silver",
			Strings::Commify(std::to_string(platinum)),
			Strings::Commify(std::to_string(gold)),
			Strings::Commify(std::to_string(silver))
		);
	}
	else if (!copper && silver && !gold && platinum) { // SP
		money_string = fmt::format(
			"{} platinum and {} silver",
			Strings::Commify(std::to_string(platinum)),
			Strings::Commify(std::to_string(silver))
		);
	}
	else if (!copper && silver && gold && !platinum) { // SG
		money_string = fmt::format(
			"{} gold and {} silver",
			Strings::Commify(std::to_string(gold)),
			Strings::Commify(std::to_string(silver))
		);
	}
	else if (copper && !silver && gold && platinum) { // CGP
		money_string = fmt::format(
			"{} platinum, {} gold, and {} copper",
			Strings::Commify(std::to_string(platinum)),
			Strings::Commify(std::to_string(gold)),
			Strings::Commify(std::to_string(copper))
		);
	}
	else if (copper && !silver && gold && !platinum) { // CG
		money_string = fmt::format(
			"{} gold and {} copper",
			Strings::Commify(std::to_string(gold)),
			Strings::Commify(std::to_string(copper))
		);
	}
	else if (!copper && !silver && gold && platinum) { // GP
		money_string = fmt::format(
			"{} platinum and {} gold",
			Strings::Commify(std::to_string(platinum)),
			Strings::Commify(std::to_string(gold))
		);
	}
	else if (!copper && !silver && !gold && platinum) { // P
		money_string = fmt::format(
			"{} platinum",
			Strings::Commify(std::to_string(platinum))
		);
	}
	else if (!copper && !silver && gold && !platinum) { // G
		money_string = fmt::format(
			"{} gold",
			Strings::Commify(std::to_string(gold))
		);
	}
	else if (!copper && silver && !gold && !platinum) { // S
		money_string = fmt::format(
			"{} silver",
			Strings::Commify(std::to_string(silver))
		);
	}
	else if (copper && !silver && !gold && !platinum) { // C
		money_string = fmt::format(
			"{} copper",
			Strings::Commify(std::to_string(copper))
		);
	}
	return money_string;
}
std::string Strings::SecondsToTime(int duration, bool is_milliseconds)
{
	if (duration <= 0) {
		return "Unknown";
	}

	if (is_milliseconds && duration < 1000) {
		return fmt::format(
			"{} Millisecond{}",
			duration,
			duration != 1 ? "s" : ""
		);
	}

	int timer_length = (
		is_milliseconds ?
			static_cast<int>(std::ceil(static_cast<float>(duration) / 1000.0f)) :
			duration
	);

	int days = int(timer_length / 86400);
	timer_length %= 86400;
	int hours = int(timer_length / 3600);
	timer_length %= 3600;
	int minutes = int(timer_length / 60);
	timer_length %= 60;
	int         seconds       = timer_length;
	std::string time_string   = "Unknown";
	std::string day_string    = (days == 1 ? "Day" : "Days");
	std::string hour_string   = (hours == 1 ? "Hour" : "Hours");
	std::string minute_string = (minutes == 1 ? "Minute" : "Minutes");
	std::string second_string = (seconds == 1 ? "Second" : "Seconds");
	if (days && hours && minutes && seconds) { // DHMS
		time_string = fmt::format(
			"{} {}, {} {}, {} {}, and {} {}",
			days,
			day_string,
			hours,
			hour_string,
			minutes,
			minute_string,
			seconds,
			second_string
		);
	}
	else if (days && hours && minutes && !seconds) { // DHM
		time_string = fmt::format(
			"{} {}, {} {}, and {} {}",
			days,
			day_string,
			hours,
			hour_string,
			minutes,
			minute_string
		);
	}
	else if (days && hours && !minutes && seconds) { // DHS
		time_string = fmt::format(
			"{} {}, {} {}, and {} {}",
			days,
			day_string,
			hours,
			hour_string,
			seconds,
			second_string
		);
	}
	else if (days && hours && !minutes && !seconds) { // DH
		time_string = fmt::format(
			"{} {} and {} {}",
			days,
			day_string,
			hours,
			hour_string
		);
	}
	else if (days && !hours && minutes && seconds) { // DMS
		time_string = fmt::format(
			"{} {}, {} {}, and {} {}",
			days,
			day_string,
			minutes,
			minute_string,
			seconds,
			second_string
		);
	}
	else if (days && !hours && minutes && !seconds) { // DM
		time_string = fmt::format(
			"{} {} and {} {}",
			days,
			day_string,
			minutes,
			minute_string
		);
	}
	else if (days && !hours && !minutes && seconds) { // DS
		time_string = fmt::format(
			"{} {} and {} {}",
			days,
			day_string,
			seconds,
			second_string
		);
	}
	else if (days && !hours && !minutes && !seconds) { // D
		time_string = fmt::format(
			"{} {}",
			days,
			day_string
		);
	}
	else if (!days && hours && minutes && seconds) { // HMS
		time_string = fmt::format(
			"{} {}, {} {}, and {} {}",
			hours,
			hour_string,
			minutes,
			minute_string,
			seconds,
			second_string
		);
	}
	else if (!days && hours && minutes && !seconds) { // HM
		time_string = fmt::format(
			"{} {} and {} {}",
			hours,
			hour_string,
			minutes,
			minute_string
		);
	}
	else if (!days && hours && !minutes && seconds) { // HS
		time_string = fmt::format(
			"{} {} and {} {}",
			hours,
			hour_string,
			seconds,
			second_string
		);
	}
	else if (!days && hours && !minutes && !seconds) { // H
		time_string = fmt::format(
			"{} {}",
			hours,
			hour_string
		);
	}
	else if (!days && !hours && minutes && seconds) { // MS
		time_string = fmt::format(
			"{} {} and {} {}",
			minutes,
			minute_string,
			seconds,
			second_string
		);
	}
	else if (!days && !hours && minutes && !seconds) { // M
		time_string = fmt::format(
			"{} {}",
			minutes,
			minute_string
		);
	}
	else if (!days && !hours && !minutes && seconds) { // S
		time_string = fmt::format(
			"{} {}",
			seconds,
			second_string
		);
	}
	return time_string;
}

std::string &Strings::LTrim(std::string &str, std::string_view chars)
{
	str.erase(0, str.find_first_not_of(chars));
	return str;
}

std::string Strings::MillisecondsToTime(int duration)
{
	return SecondsToTime(duration, true);
}

std::string &Strings::RTrim(std::string &str, std::string_view chars)
{
	str.erase(str.find_last_not_of(chars) + 1);
	return str;
}

std::string &Strings::Trim(std::string &str, const std::string &chars)
{
	return LTrim(RTrim(str, chars), chars);
}

const std::string NUM_TO_ENGLISH_X[] = {
	"", "One ", "Two ", "Three ", "Four ",
	"Five ", "Six ", "Seven ", "Eight ", "Nine ", "Ten ", "Eleven ",
	"Twelve ", "Thirteen ", "Fourteen ", "Fifteen ",
	"Sixteen ", "Seventeen ", "Eighteen ", "Nineteen "
};

const std::string NUM_TO_ENGLISH_Y[] = {
	"", "", "Twenty ", "Thirty ", "Forty ",
	"Fifty ", "Sixty ", "Seventy ", "Eighty ", "Ninety "
};

// Function to convert single digit or two digit number into words
std::string Strings::ConvertToDigit(int n, const std::string& suffix)
{
	// if n is zero
	if (n == 0) {
		return "";
	}

	// split n if it is more than 19
	if (n > 19) {
		return NUM_TO_ENGLISH_Y[n / 10] + NUM_TO_ENGLISH_X[n % 10] + suffix;
	}
	else {
		return NUM_TO_ENGLISH_X[n] + suffix;
	}
}

bool Strings::BeginsWith(const std::string& subject, const std::string& search)
{
	if (subject.length() < search.length()) {
		return false;
	}

	return subject.starts_with(search);
}

bool Strings::EndsWith(const std::string& subject, const std::string& search)
{
	if (subject.length() < search.length()) {
		return false;
	}

	return subject.ends_with(search);
}

bool Strings::Contains(const std::string& subject, const std::string& search)
{
	if (subject.length() < search.length()) {
		return false;
	}

	return subject.find(search) != std::string::npos;
}

bool Strings::ContainsLower(const std::string& subject, const std::string& search)
{
	if (subject.length() < search.length()) {
		return false;
	}

	return ToLower(subject).find(ToLower(search)) != std::string::npos;
}

uint32 Strings::TimeToSeconds(std::string time_string)
{
	if (time_string.empty()) {
		return 0;
	}

	time_string = Strings::ToLower(time_string);

	if (time_string == "f") {
		return 0;
	}

	std::string time_unit = time_string;

	time_unit.erase(
		remove_if(
			time_unit.begin(),
			time_unit.end(),
			[](char c) {
				return !isdigit(c);
			}
		),
		time_unit.end()
	);

	auto unit = Strings::ToUnsignedInt(time_unit);
	uint32 duration = 0;

	if (Strings::Contains(time_string, "s")) {
		duration = unit;
	} else if (Strings::Contains(time_string, "m")) {
		duration = unit * 60;
	} else if (Strings::Contains(time_string, "h")) {
		duration = unit * 3600;
	} else if (Strings::Contains(time_string, "d")) {
		duration = unit * 86400;
	} else if (Strings::Contains(time_string, "y")) {
		duration = unit * 31556926;
	}

	return duration;
}

bool Strings::ToBool(const std::string& bool_string)
{
	if (
		Strings::Contains(bool_string, "true") ||
		Strings::Contains(bool_string, "y") ||
		Strings::Contains(bool_string, "yes") ||
		Strings::Contains(bool_string, "on") ||
		Strings::Contains(bool_string, "enable") ||
		Strings::Contains(bool_string, "enabled") ||
		(Strings::IsNumber(bool_string) && Strings::ToInt(bool_string))
	) {
		return true;
	}

	return false;
}

// a wrapper for stoi which will return a fallback if the string
// fails to cast to a number
int Strings::ToInt(const std::string &s, int fallback)
{
	if (!Strings::IsNumber(s)) {
		return fallback;
	}

	try {
		return std::stoi(s);
	}
	catch (std::exception &) {
		return fallback;
	}
}

int64 Strings::ToBigInt(const std::string &s, int64 fallback)
{
	if (!Strings::IsNumber(s)) {
		return fallback;
	}

	try {
		return std::stoll(s);
	}
	catch (std::exception &) {
		return fallback;
	}
}

uint32 Strings::ToUnsignedInt(const std::string &s, uint32 fallback)
{
	if (!Strings::IsNumber(s)) {
		return fallback;
	}

	try {
		return std::stoul(s);
	}
	catch (std::exception &) {
		return fallback;
	}
}

uint64 Strings::ToUnsignedBigInt(const std::string &s, uint64 fallback)
{
	if (!Strings::IsNumber(s)) {
		return fallback;
	}

	try {
		return std::stoull(s);
	}
	catch (std::exception &) {
		return fallback;
	}
}

float Strings::ToFloat(const std::string &s, float fallback)
{
	if (!Strings::IsFloat(s)) {
		return fallback;
	}

	try {
		return std::stof(s);
	}
	catch (std::exception &) {
		return fallback;
	}
}

std::string Strings::RemoveNumbers(std::string s)
{
	int      current = 0;
	for (int i       = 0; i < s.length(); i++) {
		if (!isdigit(s[i])) {
			s[current] = s[i];
			current++;
		}
	}

	return s.substr(0, current);
}

std::string Strings::ZoneTime(const uint8 hours, const uint8 minutes)
{
	return fmt::format(
		"{:02}:{:02} {}",
		(
			(hours % 12) == 0 ?
			12 :
			(hours % 12)
		),
		minutes,
		hours >= 13 ? "PM" : "AM"
	);
}

std::string Strings::Slugify(const std::string& input, const std::string& separator) {
	std::string slug;
	bool last_was_hyphen = false;

	for (char c : input) {
		if (std::isalnum(c)) {
			slug += std::tolower(c);
			last_was_hyphen = false;
		} else if (c == ' ' || c == '_' || c == '-') {
			if (!last_was_hyphen && !slug.empty()) {
				slug += separator;
				last_was_hyphen = true;
			}
		}
	}

	// Remove trailing hyphen if present
	if (!slug.empty() && slug.back() == '-') {
		slug.pop_back();
	}

	return slug;
}

bool Strings::IsValidJson(const std::string &json)
{
	rapidjson::Document    doc;
	rapidjson::ParseResult result = doc.Parse(json.c_str());

	return result;
}
