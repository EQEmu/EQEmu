#pragma once

#include "common/repositories/tool_game_objects_repository.h"
#include "zone/client.h"

class DoorManipulation {

public:
	static void CommandHandler(Client *c, const Seperator *sep);
	static void CommandHeader(Client *c);
	static void DisplayObjectResultToClient(
		Client *c,
		std::vector<ToolGameObjectsRepository::ToolGameObjects> game_objects
	);
	static void DisplayModelsFromFileResults(
		Client *c,
		std::vector<ToolGameObjectsRepository::ToolGameObjects> game_objects
	);
};
