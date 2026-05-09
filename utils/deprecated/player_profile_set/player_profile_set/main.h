#ifndef EQ_MAIN_H
#define EQ_MAIN_H

#include "types.h"
#include "eq_player_structs.h"

struct player_entry
{
	int32 id;
	char *data;
};

std::string ConvertFieldToValue(PlayerProfile_Struct *m_pp, std::string field);
void ConvertValueToField(PlayerProfile_Struct *m_pp, std::string field, std::string value);

#endif