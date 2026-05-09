#ifndef INI__H
#define INI__H

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

struct iniData
{
	std::string option;
	std::string param;
};

class INIParser
{
public:
	INIParser(const char *filename);
	~INIParser();
	void Parse();
	void ReadLine(FILE *fp, char *Option, char *Param);

	void AddOption(std::string option, std::string param);
	std::string GetOption(std::string option);


protected:
	char file[128];
	std::vector<iniData> options;
};

#endif