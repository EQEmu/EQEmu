#include "door_manipulation.h"

#include "zone/client.h"

void command_door(Client *c, const Seperator *sep)
{
	DoorManipulation::CommandHandler(c, sep);
}

