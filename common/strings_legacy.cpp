#include "common/strings.h"

#include "fmt/format.h"
#include <algorithm>
#include <cctype>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#ifdef _WINDOWS
#include <windows.h>
#define snprintf        _snprintf
#define strncasecmp     _strnicmp
#define strcasecmp      _stricmp
#endif

#ifndef va_copy
#define va_copy(d,s) ((d) = (s))
#endif

// normal strncpy doesnt put a null term on copied strings, this one does
// ref: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wcecrt/htm/_wcecrt_strncpy_wcsncpy.asp
char *strn0cpy(char *dest, const char *source, uint32 size)
{
	if (!dest) {
		return 0;
	}
	if (size == 0 || source == 0) {
		dest[0] = 0;
		return dest;
	}
	strncpy(dest, source, size);
	dest[size - 1] = 0;
	return dest;
}

// String N w/null Copy Truncated?
// return value =true if entire string(source) fit, false if it was truncated
bool strn0cpyt(char *dest, const char *source, uint32 size)
{
	if (!dest) {
		return 0;
	}
	if (size == 0 || source == 0) {
		dest[0] = 0;
		return false;
	}
	strncpy(dest, source, size);
	dest[size - 1] = 0;
	return (bool) (source[strlen(dest)] == 0);
}

const char *MakeLowerString(const char *source)
{
	static char str[128];
	if (!source) {
		return nullptr;
	}
	MakeLowerString(source, str);
	return str;
}

void MakeLowerString(const char *source, char *target)
{
	if (!source || !target) {
		*target = 0;
		return;
	}
	while (*source) {
		*target = tolower(*source);
		target++;
		source++;
	}
	*target = 0;
}

uint32 hextoi(const char *num)
{
	if (num == nullptr) {
		return 0;
	}

	int len = strlen(num);
	if (len < 3) {
		return 0;
	}

	if (num[0] != '0' || (num[1] != 'x' && num[1] != 'X')) {
		return 0;
	}

	uint32   ret = 0;
	int      mul = 1;
	for (int i   = len - 1; i >= 2; i--) {
		if (num[i] >= 'A' && num[i] <= 'F') {
			ret += ((num[i] - 'A') + 10) * mul;
		}
		else if (num[i] >= 'a' && num[i] <= 'f') {
			ret += ((num[i] - 'a') + 10) * mul;
		}
		else if (num[i] >= '0' && num[i] <= '9') {
			ret += (num[i] - '0') * mul;
		}
		else {
			return 0;
		}
		mul *= 16;
	}
	return ret;
}

uint64 hextoi64(const char *num)
{
	if (num == nullptr) {
		return 0;
	}

	int len = strlen(num);
	if (len < 3) {
		return 0;
	}

	if (num[0] != '0' || (num[1] != 'x' && num[1] != 'X')) {
		return 0;
	}

	uint64   ret = 0;
	int      mul = 1;
	for (int i   = len - 1; i >= 2; i--) {
		if (num[i] >= 'A' && num[i] <= 'F') {
			ret += ((num[i] - 'A') + 10) * mul;
		}
		else if (num[i] >= 'a' && num[i] <= 'f') {
			ret += ((num[i] - 'a') + 10) * mul;
		}
		else if (num[i] >= '0' && num[i] <= '9') {
			ret += (num[i] - '0') * mul;
		}
		else {
			return 0;
		}
		mul *= 16;
	}
	return ret;
}

bool atobool(const char *iBool)
{

	if (iBool == nullptr) {
		return false;
	}
	if (!strcasecmp(iBool, "true")) {
		return true;
	}
	if (!strcasecmp(iBool, "false")) {
		return false;
	}
	if (!strcasecmp(iBool, "yes")) {
		return true;
	}
	if (!strcasecmp(iBool, "no")) {
		return false;
	}
	if (!strcasecmp(iBool, "on")) {
		return true;
	}
	if (!strcasecmp(iBool, "off")) {
		return false;
	}
	if (!strcasecmp(iBool, "enable")) {
		return true;
	}
	if (!strcasecmp(iBool, "disable")) {
		return false;
	}
	if (!strcasecmp(iBool, "enabled")) {
		return true;
	}
	if (!strcasecmp(iBool, "disabled")) {
		return false;
	}
	if (!strcasecmp(iBool, "y")) {
		return true;
	}
	if (!strcasecmp(iBool, "n")) {
		return false;
	}
	if (Strings::ToInt(iBool)) {
		return true;
	}
	return false;
}

// removes the crap and turns the underscores into spaces.
char *CleanMobName(const char *in, char *out)
{
	unsigned i, j;

	for (i = j = 0; i < strlen(in); i++) {
		// convert _ to space.. any other conversions like this?  I *think* this
		// is the only non alpha char that's not stripped but converted.
		if (in[i] == '_') {
			out[j++] = ' ';
		}
		else {
			if (isalpha(in[i]) || (in[i] == '`')) {    // numbers, #, or any other crap just gets skipped
				out[j++] = in[i];
			}
		}
	}
	out[j] = 0;    // terimnate the string before returning it
	return out;
}


void RemoveApostrophes(std::string &s)
{
	for (unsigned int i = 0; i < s.length(); ++i)
		if (s[i] == '\'') {
			s[i] = '_';
		}
}

char *RemoveApostrophes(const char *s)
{
	auto NewString = new char[strlen(s) + 1];

	strcpy(NewString, s);

	for (unsigned int i = 0; i < strlen(NewString); ++i)
		if (NewString[i] == '\'') {
			NewString[i] = '_';
		}

	return NewString;
}

const char *ConvertArray(int64 input, char *returnchar)
{
	sprintf(returnchar, "%" PRId64, input);
	return returnchar;
}

const char *ConvertArrayF(float input, char *returnchar)
{
	sprintf(returnchar, "%0.2f", input);
	return returnchar;
}

bool isAlphaNumeric(const char *text)
{
	for (unsigned int charIndex = 0; charIndex < strlen(text); charIndex++) {
		if ((text[charIndex] < 'a' || text[charIndex] > 'z') &&
			(text[charIndex] < 'A' || text[charIndex] > 'Z') &&
			(text[charIndex] < '0' || text[charIndex] > '9')) {
			return false;
		}
	}

	return true;
}

// first letter capitalized and rest made lower case
std::string FormatName(const std::string &char_name)
{
	std::string formatted(char_name);
	if (!formatted.empty()) {
		std::transform(formatted.begin(), formatted.end(), formatted.begin(), ::tolower);
		formatted[0] = ::toupper(formatted[0]);
	}
	return formatted;
}

bool IsAllowedWorldServerCharacterList(char c)
{
	const char *valid_characters = ":[](){}.!@#$%^&*-=+<>/\\|'\"";
	if (strchr(valid_characters, c)) {
		return true;
	}

	return false;
}

void SanitizeWorldServerName(char *name)
{
	std::string server_long_name = name;

	strcpy(name, SanitizeWorldServerName(server_long_name).c_str());
}

// original source:
// https://github.com/facebook/folly/blob/master/folly/String.cpp
//
const std::string vStringFormat(const char *format, va_list args)
{
	std::string output;
	va_list     tmpargs;

	va_copy(tmpargs, args);
	int characters_used = vsnprintf(nullptr, 0, format, tmpargs);
	va_end(tmpargs);

	// Looks like we have a valid format string.
	if (characters_used > 0) {
		output.resize(characters_used + 1);

		va_copy(tmpargs, args);
		characters_used = vsnprintf(&output[0], output.capacity(), format, tmpargs);
		va_end(tmpargs);

		output.resize(characters_used);

		// We shouldn't have a format error by this point, but I can't imagine what error we
		// could have by this point. Still, return empty string;
		if (characters_used < 0) {
			output.clear();
		}
	}
	return output;
}

const std::string StringFormat(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	std::string output = vStringFormat(format, args);
	va_end(args);
	return output;
}
