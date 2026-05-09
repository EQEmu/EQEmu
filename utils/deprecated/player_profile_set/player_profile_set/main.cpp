#include <iostream>
#include <sstream>
#include <vector>

#include "eq_player_structs.h"
#include "database.h"
#include "ini.h"
#include "main.h"
#include "eqemu_string.h"

EQEmuDatabase *emu_db;
std::vector<player_entry> player_list;

//player_profile_set [SET/VIEW] [Field] [Name/*]
//ex: player_profile_set VIEW LDON_MMC Jess
int main(int argc, char* argv[])
{
	INIParser *ini = new INIParser("database.ini");
	std::string host = ini->GetOption("host");
	std::string user = ini->GetOption("user");
	std::string pass = ini->GetOption("password");
	std::string datab = ini->GetOption("db");

	if(argc != 5)
	{
		std::cout << "Usage: player_profile_set [SET/VIEW] [Field] [Value] [Name/*]" << std::endl;
		std::cout << "Ex: player_profile_set VIEW LDON_MMC 0 Jess" << std::endl;
		std::cout << "Would view the number of LDoN MMC points earned by Jess" << std::endl << std::endl;
		std::cout << "Ex: player_profile_set SET LDON_TOTAL 0 *" << std::endl;
		std::cout << "Would view the number of LDoN MMC points earned by All players to 0" << std::endl;
		delete ini;
		ini = 0;
		exit(1);
	}

	std::string program_action = argv[1];
	std::string program_field = argv[2];
	std::string program_value = argv[3];
	std::string player_name = argv[4];
	program_action = EQEmuString::ToUpper(program_action);
	program_field = EQEmuString::ToUpper(program_field);

	emu_db = new EQEmuDatabase(host.c_str(), datab.c_str(), user.c_str(), pass.c_str());
	if(!emu_db->Connected())
	{
		delete ini;
		ini = 0;
		delete emu_db;
		emu_db = 0;
		exit(1);
	}

	if(player_name == "*")
		emu_db->GetPlayers();
	else
		emu_db->GetPlayer(player_name);

	std::vector<player_entry>::iterator iter = player_list.begin();

	while(iter != player_list.end())
	{
		player_entry pe = (*iter);
		PlayerProfile_Struct *m_pp = (PlayerProfile_Struct*)pe.data;

		if(program_action == "SET")
		{
			m_pp->ldon_points_available = 0;
			m_pp->ldon_points_guk = 0;
			m_pp->ldon_points_mir = 0;
			m_pp->ldon_points_mmc = 0;
			m_pp->ldon_points_ruj = 0;
			m_pp->ldon_points_tak = 0;
			emu_db->StorePlayer(pe.id, pe.data);
		}
		else if(program_action == "VIEW")
		{
			std::cout << m_pp->name << std::endl;
			std::cout << m_pp->ldon_points_available << std::endl;
			std::cout << m_pp->ldon_points_guk << std::endl;
			std::cout << m_pp->ldon_points_mir << std::endl;
			std::cout << m_pp->ldon_points_mmc << std::endl;
			std::cout << m_pp->ldon_points_ruj << std::endl;
			std::cout << m_pp->ldon_points_tak << std::endl;
		}
		else
		{
			std::cout << "Unknown action specified" << std::endl;
		}
		delete[] pe.data;
		pe.data = 0;
		iter++;
	}
	player_list.clear();

	delete ini;
	ini = 0;
	delete emu_db;
	emu_db = 0;

	std::cout << "Press enter to exit...";
	std::cin.get();
	exit(0);
}

std::string ConvertFieldToValue(PlayerProfile_Struct *m_pp, std::string field)
{
	if(!m_pp)
	{
		return std::string("Unable to convert field to value");
	}

	std::stringstream ss(std::stringstream::out | std::stringstream::in);

	if(field == "LASTNAME")
	{
		ss << m_pp->last_name;
		return ss.str();
	}

	if(field == "GENDER")
	{
		ss << m_pp->gender;
		return ss.str();
	}

	if(field == "RACE")
	{
		ss << m_pp->race;
		return ss.str();
	}

	if(field == "CLASS")
	{
		ss << m_pp->class_;
		return ss.str();
	}

	return std::string("Unable to convert field to value");
}

void ConvertValueToField(PlayerProfile_Struct *m_pp, std::string field, std::string value)
{
}