#include "ini.h"

INIParser::INIParser(const char *filename)
{
	memset(file, 0, 128);
	if(strlen(filename) > 128)
	{
		strncpy(file, filename, 128);
	}
	else
	{
		strncpy(file, filename, strlen(filename));
	}
	Parse();
}

INIParser::~INIParser()
{

}

void INIParser::Parse()
{
	FILE *fp;
	if(fp = fopen(file, "r"))
	{
		char Option[255], Param[255];
		while(!feof(fp))
		{
			ReadLine(fp, Option, Param);
			AddOption(std::string(Option), std::string(Param));
		}
		fclose(fp);
	}
}

void INIParser::ReadLine(FILE *fp, char *Option, char *Param)
{
	typedef enum ReadingState	{ ReadingOption, ReadingParameter };

	ReadingState State = ReadingOption;

	int StrIndex = 0;
	char ch;

	strcpy(Option, "");
	strcpy(Param, "");

	while(true) {
		ch = fgetc(fp);
		if((ch=='#')&&(StrIndex==0)) { // Discard comment lines beginning with a hash
			while((ch!=EOF)&&(ch!='\n'))
				ch = fgetc(fp);

			continue;
		}
		if(ch=='\r') continue;
		if((ch==EOF)||(ch=='\n')) {
			switch(State) {
				case ReadingOption: {
					Option[StrIndex]='\0';
					break;
				}
				case ReadingParameter: {
					Param[StrIndex] = '\0';
					break;
				}
			}

			break;
		}
		if(ch=='=') {
			if(State==ReadingOption) {
				Option[StrIndex] = '\0';
				State = ReadingParameter;
				StrIndex = 0;
				continue;
			}
		}
		switch(State) {
			case ReadingOption: {
				Option[StrIndex++]=tolower(ch);
				break;
			}
			case ReadingParameter: {
				Param[StrIndex++]=ch;
				break;
			}
		}
	}

	if(!strcmp(Param,"true")) strcpy(Param,"1");
	if(!strcmp(Param,"false")) strcpy(Param,"0");
}

void INIParser::AddOption(std::string option, std::string param)
{
	for(unsigned int x = 0; x < options.size(); ++x)
	{
		if(options[x].option == option)
			return;
	}

	iniData d;
	d.option = option;
	d.param = param;
	options.push_back(d);
}

std::string INIParser::GetOption(std::string option)
{
	for(unsigned int x = 0; x < options.size(); ++x)
	{
		if(options[x].option == option)
			return options[x].param;
	}
	return std::string("Not Found");
}