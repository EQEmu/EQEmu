#include "object_manipulation.h"

#include "zone/client.h"

void command_object(Client *c, const Seperator *sep)
{
	ObjectManipulation::CommandHandler(c, sep);
}
