#ifndef EQEMU_STRING_H
#define EQEMU_STRING_H

#include <string>
#include <cctype>

namespace EQEmuString
{

std::string ToUpper(std::string in_string)
{
	std::string out_string = in_string;
	for(unsigned int i = 0; i < in_string.size(); ++i)
	{
		out_string[i] = std::toupper(in_string[i]);
	}
	return out_string;
}

};
#endif