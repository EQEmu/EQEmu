#pragma once

#include "zone/client.h"

class ObjectManipulation {

public:
	static void CommandHandler(Client *c, const Seperator *sep);
	static void CommandHeader(Client *c);
	static void SendSubcommands(Client *c);
};
