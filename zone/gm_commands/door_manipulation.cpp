#include "door_manipulation.h"

#include "common/misc_functions.h"
#include "common/strings.h"
#include "zone/doors.h"

#define MAX_CLIENT_MESSAGE_LENGTH 2000

void DoorManipulation::CommandHandler(Client *c, const Seperator *sep)
{
	// this should never happen
	if (!c) {
		return;
	}

	// args
	std::string arg1(sep->arg[1]);
	std::string arg2(sep->arg[2]);
	std::string arg3(sep->arg[3]);

	// table check
	std::string table_name = "tool_game_objects";
	std::string url        = "https://raw.githubusercontent.com/EQEmu/database-tool-sqls/main/tool_game_objects.sql";
	if (!database.DoesTableExist(table_name)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Table [{}] does not exist. Downloading from [{}] and installing locally",
				table_name,
				url
			).c_str()
		);
		database.SourceDatabaseTableFromUrl(
			table_name,
			url
		);
	}

	if (arg1 == "drawbox") {
		Doors *door = entity_list.GetDoorsByID(c->GetDoorToolEntityId());

		if (door) {
			uint16 door_size = 15;
			float door_depth = 5.0f;

			if (sep->IsNumber(2) && atof(sep->arg[2]) > 0) {
				door_size = atof(sep->arg[2]);
			}

			if (sep->IsNumber(3) && atof(sep->arg[3]) > 0) {
				door_depth = atof(sep->arg[3]);
			}


			door->IsDoorBetween(c->GetPosition(), (c->GetTarget() ? c->GetTarget()->GetPosition() : c->GetPosition()), door_size, door_depth, true);
		}
	}

	// edit menu
	if (arg1 == "edit") {
		Doors *door = entity_list.GetDoorsByID(c->GetDoorToolEntityId());
		if (door) {

			const std::string move_x_action   = "move_x";
			const std::string move_y_action   = "move_y";
			const std::string move_z_action   = "move_z";
			const std::string move_h_action   = "move_h";
			const std::string set_size_action = "set_size";

			// we're passing a move action here
			if (!arg3.empty() && (Strings::IsFloat(arg3) || Strings::IsNumber(arg3))) {
				float x_move   = 0.0f;
				float y_move   = 0.0f;
				float z_move   = 0.0f;
				float h_move   = 0.0f;
				float set_size = 0.0f;

				if (arg2 == move_x_action) {
					x_move = Strings::ToFloat(arg3);
				}

				if (arg2 == move_y_action) {
					y_move = Strings::ToFloat(arg3);
				}

				if (arg2 == move_z_action) {
					z_move = Strings::ToFloat(arg3);
				}

				if (arg2 == move_h_action) {
					h_move = Strings::ToFloat(arg3);
				}

				if (arg2 == set_size_action) {
					set_size = Strings::ToFloat(arg3);
				}

				door->SetLocation(
					door->GetX() + x_move,
					door->GetY() + y_move,
					door->GetZ() + z_move
				);

				glm::vec4 door_position = door->GetPosition();
				door_position.w = door_position.w + h_move;
				door->SetPosition(door_position);
				door->SetSize(door->GetSize() + set_size);

				// spawn and move helpers
				uint16 helper_mob_x_negative = 0;
				uint16 helper_mob_x_positive = 0;
				uint16 helper_mob_y_positive = 0;
				uint16 helper_mob_y_negative = 0;

				for (auto &n: entity_list.GetNPCList()) {
					NPC         *npc     = n.second;
					std::string npc_name = npc->GetName();
					if (npc_name.find("-X") != std::string::npos) {
						helper_mob_x_negative = npc->GetID();
					}
					if (npc_name.find("-Y") != std::string::npos) {
						helper_mob_y_negative = npc->GetID();
					}
					if (npc_name.find("+X") != std::string::npos) {
						helper_mob_x_positive = npc->GetID();
					}
					if (npc_name.find("+Y") != std::string::npos) {
						helper_mob_y_positive = npc->GetID();
					}
				}

				// -X
				door_position = door->GetPosition();
				if (helper_mob_x_negative == 0) {
					door_position.x = door_position.x - 15;
					helper_mob_x_negative = NPC::SpawnNodeNPC("-X", "", door_position)->GetID();
				}
				else {
					auto n = entity_list.GetNPCByID(helper_mob_x_negative);
					n->GMMove(door->GetX() - 15, door->GetY(), door->GetZ(), n->GetHeading());
				}

				// +X
				door_position = door->GetPosition();
				if (helper_mob_x_positive == 0) {
					door_position.x = door_position.x + 15;
					helper_mob_x_positive = NPC::SpawnNodeNPC("+X", "", door_position)->GetID();
				}
				else {
					auto n = entity_list.GetNPCByID(helper_mob_x_positive);
					n->GMMove(door->GetX() + 15, door->GetY(), door->GetZ(), n->GetHeading());
				}

				// -Y
				door_position = door->GetPosition();
				if (helper_mob_y_negative == 0) {
					door_position.y = door_position.y - 15;
					helper_mob_y_negative = NPC::SpawnNodeNPC("-Y", "", door_position)->GetID();
				}
				else {
					auto n = entity_list.GetNPCByID(helper_mob_y_negative);
					n->GMMove(door->GetX(), door->GetY() - 15, door->GetZ(), n->GetHeading());
				}

				// +Y
				door_position = door->GetPosition();
				if (helper_mob_y_positive == 0) {
					door_position.y = door_position.y + 15;
					helper_mob_y_positive = NPC::SpawnNodeNPC("+Y", "", door_position)->GetID();
				}
				else {
					auto n = entity_list.GetNPCByID(helper_mob_y_positive);
					n->GMMove(door->GetX(), door->GetY() + 15, door->GetZ(), n->GetHeading());
				}

				return;
			}

			c->Message(
				Chat::White,
				fmt::format(
					"Door Selected ID [{}] Name [{}] OpenType [{}] Invertstate [{} | {}/{}] ",
					c->GetDoorToolEntityId(),
					door->GetDoorName(),
					door->GetOpenType(),
					door->GetInvertState(),
					Saylink::Silent("#door setinvertstate 0", "0"),
					Saylink::Silent("#door setinvertstate 1", "1")
				).c_str()
			);


			std::vector<std::string> move_options = {
				move_x_action,
				move_y_action,
				move_z_action,
				move_h_action,
				set_size_action
			};
			std::vector<std::string> move_x_options_positive;
			std::vector<std::string> move_x_options_negative;
			std::vector<std::string> move_y_options_positive;
			std::vector<std::string> move_y_options_negative;
			std::vector<std::string> move_z_options_positive;
			std::vector<std::string> move_z_options_negative;
			std::vector<std::string> move_h_options_positive;
			std::vector<std::string> move_h_options_negative;
			std::vector<std::string> set_size_options_positive;
			std::vector<std::string> set_size_options_negative;

			std::vector<std::string> xyz_values = {
				"0.1", "1", "5", "10", "25", "50", "100"
			};

			// build positive options x/y/z
			for (const auto &v: xyz_values) {
				for (const auto &o: move_options) {
					if (o == move_x_action) {
						move_x_options_positive.emplace_back(
							Saylink::Silent(fmt::format("#door edit {} {}", o, v), v)
						);
					}
					else if (o == move_y_action) {
						move_y_options_positive.emplace_back(
							Saylink::Silent(fmt::format("#door edit {} {}", o, v), v)
						);
					}
					else if (o == move_z_action) {
						move_z_options_positive.emplace_back(
							Saylink::Silent(fmt::format("#door edit {} {}", o, v), v)
						);
					}
				}
			}

			// loop through vector in reverse order
			// build negative options x/y/z
			for (auto v = xyz_values.rbegin(); v != xyz_values.rend(); ++v) {
				for (const auto &o: move_options) {
					if (o == move_x_action) {
						move_x_options_negative.emplace_back(
							Saylink::Silent(fmt::format("#door edit {} -{}", o, *v), *v)
						);
					}
					else if (o == move_y_action) {
						move_y_options_negative.emplace_back(
							Saylink::Silent(fmt::format("#door edit {} -{}", o, *v), *v)
						);
					}
					else if (o == move_z_action) {
						move_z_options_negative.emplace_back(
							Saylink::Silent(fmt::format("#door edit {} -{}", o, *v), *v)
						);
					}
				}
			}

			std::vector<std::string> heading_values = {
				"1", "5", "32.5", "63.75", "130",
			};

			// build positive options h
			for (const auto &v: heading_values) {
				move_h_options_positive.emplace_back(
					Saylink::Silent(fmt::format("#door edit {} {}", move_h_action, v), v)
				);
			}

			// loop through vector in reverse order
			// build negative options h
			for (auto v = heading_values.rbegin(); v != heading_values.rend(); ++v) {
				move_h_options_negative.emplace_back(
					Saylink::Silent(fmt::format("#door edit {} -{}", move_h_action, *v), *v)
				);
			}

			std::vector<std::string> size_values = {
				"1", "5", "10", "25", "50", "100", "1000"
			};

			// build positive options size
			for (const auto &v: size_values) {
				set_size_options_positive.emplace_back(
					Saylink::Silent(fmt::format("#door edit {} {}", set_size_action, v), v)
				);
			}

			// loop through vector in reverse order
			// build negative options size
			for (auto v = size_values.rbegin(); v != size_values.rend(); ++v) {
				set_size_options_negative.emplace_back(
					Saylink::Silent(fmt::format("#door edit {} -{}", set_size_action, *v), *v)
				);
			}

			c->Message(
				Chat::White,
				fmt::format(
					"Name [{}] [{}] [{}] [{}]",
					door->GetDoorName(),
					Saylink::Silent("#door save", "Save"),
					Saylink::Silent("#door changemodelqueue", "Change Model"),
					Saylink::Silent("#door setinclineinc", "Incline")
				).c_str()
			);
			c->Message(
				Chat::White,
				fmt::format(
					"[{}] - [X] + [{}]",
					Strings::Implode(" | ", move_x_options_negative),
					Strings::Implode(" | ", move_x_options_positive)
				).c_str()
			);
			c->Message(
				Chat::White,
				fmt::format(
					"[{}] - [Y] + [{}]",
					Strings::Implode(" | ", move_y_options_negative),
					Strings::Implode(" | ", move_y_options_positive)
				).c_str()
			);
			c->Message(
				Chat::White,
				fmt::format(
					"[{}] - [Z] + [{}]",
					Strings::Implode(" | ", move_z_options_negative),
					Strings::Implode(" | ", move_z_options_positive)
				).c_str()
			);
			c->Message(
				Chat::White,
				fmt::format(
					"[{}] - [H] + [{}]",
					Strings::Implode(" | ", move_h_options_negative),
					Strings::Implode(" | ", move_h_options_positive)
				).c_str()
			);
			c->Message(
				Chat::White,
				fmt::format(
					"[{}] - [Size] + [{}]",
					Strings::Implode(" | ", set_size_options_negative),
					Strings::Implode(" | ", set_size_options_positive)
				).c_str()
			);

			return;
		}

		c->Message(Chat::Red, "Door selection invalid...");
	}

	// create
	if (arg1 == "create") {
		std::string model     = Strings::ToUpper(arg2);
		uint16      entity_id = entity_list.CreateDoor(
			model.c_str(),
			c->GetPosition(),
			58,
			100
		);

		c->Message(
			Chat::White,
			fmt::format("Creating door entity_id [{}] with model [{}]", entity_id, model).c_str());
		c->SetDoorToolEntityId(entity_id);
	}

	// set model
	if (arg1 == "model") {
		Doors       *door = entity_list.GetDoorsByID(c->GetDoorToolEntityId());
		std::string model = Strings::ToUpper(arg2);
		if (door) {
			door->SetDoorName(model.c_str());
		}
	}

	// change model queue
	if (arg1 == "changemodelqueue") {
		c->Message(
			Chat::White,
			fmt::format(
				"#door model <modelname> | Changes door model for selected door or select from [{}] or [{}]",
				Saylink::Silent("#door showmodelszone", "local zone"),
				Saylink::Silent("#door showmodelsglobal", "global")
			).c_str()
		);
	}

	// open type
	if (arg1 == "opentype" && !arg2.empty() && Strings::IsNumber(arg2)) {
		Doors *door = entity_list.GetDoorsByID(c->GetDoorToolEntityId());
		if (door) {
			door->SetOpenType(Strings::ToInt(arg2));
		}
	}

	// incline
	if (arg1 == "setincline" && !arg2.empty() && Strings::IsNumber(arg2)) {
		Doors *door = entity_list.GetDoorsByID(c->GetDoorToolEntityId());
		if (door) {
			door->SetIncline(Strings::ToInt(arg2));
		}
	}

	// invertstate
	if (arg1 == "setinvertstate" && !arg2.empty() && Strings::IsNumber(arg2)) {
		Doors *door = entity_list.GetDoorsByID(c->GetDoorToolEntityId());
		if (door) {
			door->SetInvertState(Strings::ToInt(arg2));
		}
	}

	// save
	if (arg1 == "save") {
		Doors *door = entity_list.GetDoorsByID(c->GetDoorToolEntityId());
		if (door) {
			door->CreateDatabaseEntry();
			c->Message(Chat::White, "Door saved");
		}
	}

	// incline incremental
	if (arg1 == "setinclineinc" && !arg2.empty() && Strings::IsNumber(arg2)) {
		Doors *door = entity_list.GetDoorsByID(c->GetDoorToolEntityId());
		if (door) {
			door->SetIncline(door->GetIncline() + Strings::ToInt(arg2));
		}
	}

	if (arg1 == "setinclineinc") {
		std::map<float, std::string> incline_values = {
			{.01,    "Upright"},
			{63.75,  "45 Degrees",},
			{130,    "90 Degrees"},
			{192.5,  "135 Degrees"},
			{255,    "180 Degrees"},
			{321.25, "225 Degrees"},
			{385,    "270 Degrees"},
			{448.75, "315 Degrees"},
			{512.5,  "360 Degrees"}
		};

		std::vector<std::string> incline_normal_options;
		std::vector<std::string> incline_positive_options;
		std::vector<std::string> incline_negative_options;
		for (auto                incline_value: incline_values) {
			incline_normal_options.emplace_back(
				Saylink::Silent(
					fmt::format(
						"#door setincline {}",
						incline_value.first
					),
					incline_value.second
				)
			);
		}

		for (int incline_index = 0; incline_index <= 100; incline_index += 10) {
			int incline_value = (incline_index == 0 ? 1 : incline_index);
			incline_positive_options.emplace_back(
				Saylink::Silent(
					fmt::format(
						"#door setinclineinc {}",
						incline_value
					),
					itoa(std::abs(incline_value))
				)
			);
		}

		for (int incline_index = -100; incline_index <= 1; incline_index += 10) {
			int incline_value = (incline_index == 0 ? -1 : incline_index);
			incline_negative_options.emplace_back(
				Saylink::Silent(
					fmt::format(
						"#door setinclineinc {}",
						incline_value
					),
					itoa(std::abs(incline_value))
				)
			);
		}

		c->Message(
			Chat::White,
			fmt::format(
				"[Incline] [{}]",
				Strings::Implode(" | ", incline_normal_options)
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"[Incline Increments] [{}] - | + [{}]",
				Strings::Implode(" | ", incline_negative_options),
				Strings::Implode(" | ", incline_positive_options)
			).c_str()
		);
	}

	// show models in zone
	if (arg1 == "showmodelsglobal") {
		auto game_objects = ToolGameObjectsRepository::GetWhere(
			database,
			"object_name LIKE '%IT%' AND zoneid = 0 AND object_name NOT LIKE '%OBJ%' GROUP by file_from"
		);

		if (game_objects.empty()) {
			c->Message(Chat::White, "There are no models to display...");
		}

		c->Message(Chat::White, "------------------------------------------------");
		c->Message(Chat::White, "# Models (Global)");
		c->Message(Chat::White, "------------------------------------------------");

		DisplayModelsFromFileResults(c, game_objects);
	}

	// show models in zone
	if (arg1 == "showmodelszone") {
		auto game_objects = ToolGameObjectsRepository::GetWhere(
			database,
			fmt::format("zoneid = {}", zone->GetZoneID())
		);

		if (game_objects.empty()) {
			c->Message(Chat::White, "There are no models for this zone...");
		}

		c->Message(Chat::White, "------------------------------------------------");
		c->Message(Chat::White, "# Models from zone");
		c->Message(Chat::White, "------------------------------------------------");

		DisplayObjectResultToClient(c, game_objects);
	}

	// show models from file name
	if (arg1 == "showmodelsfromfile" && !arg2.empty()) {
		const std::string &file_name   = arg2;
		auto              game_objects = ToolGameObjectsRepository::GetWhere(
			database,
			fmt::format("file_from = '{}'", file_name)
		);

		if (game_objects.empty()) {
			c->Message(Chat::White, "There are no models for this zone...");
		}

		c->Message(Chat::White, "------------------------------------------------");
		c->Message(Chat::White, fmt::format("# Models from file name [{}]", file_name).c_str());
		c->Message(Chat::White, "------------------------------------------------");

		DisplayObjectResultToClient(c, game_objects);
	}

	// help menu
	if (arg1.empty()) {
		DoorManipulation::CommandHeader(c);
		c->Message(
			Chat::White,
			"#door create <modelname> | Creates a door from a model. (Example IT78 creates a campfire)"
		);
		c->Message(Chat::White, "#door setinvertstate [0|1] | Sets selected door invert state");
		c->Message(Chat::White, "#door setincline <incline> | Sets selected door incline");
		c->Message(Chat::White, "#door opentype <opentype> | Sets selected door opentype");
		c->Message(
			Chat::White,
			fmt::format(
				"{} <door_size> <door_depth> | Draws a box for the door, default size = 15, depth = 5 if none defined",
				Saylink::Silent("#door drawbox")
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"#door model <modelname> | Changes door model for selected door or select from [{}] or [{}]",
				Saylink::Silent("#door showmodelszone", "local zone"),
				Saylink::Silent("#door showmodelsglobal", "global")
			).c_str()
		);
		c->Message(
			Chat::White,
			"#door showmodelsfromfile <file.eqg|file.s3d> | Shows models from s3d or eqg file. Example tssequip.eqg or wallet01.eqg"
		);

		c->Message(
			Chat::White,
			fmt::format(
				"{} | Shows available models in the current zone that you are in",
				Saylink::Silent("#door showmodelszone")
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"{} | Shows available models globally by first listing all global model files",
				Saylink::Silent("#door showmodelsglobal")
			).c_str()
		);

		c->Message(Chat::White, "#door save | Creates database entry for selected door");
		c->Message(
			Chat::White,
			fmt::format(
				"{} - Brings up editing interface for selected door",
				Saylink::Silent("#door edit")
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"{} - lists doors in zone",
				Saylink::Silent("#list doors")
			).c_str()
		);

		return;
	}
}

void DoorManipulation::CommandHeader(Client *c)
{
	c->Message(Chat::White, "------------------------------------------------");
	c->Message(Chat::White, "# Door Commands");
	c->Message(Chat::White, "------------------------------------------------");
}

void DoorManipulation::DisplayObjectResultToClient(
	Client *c,
	std::vector<ToolGameObjectsRepository::ToolGameObjects> game_objects
)
{
	std::vector<std::string> say_links;

	for (auto &g: game_objects) {
		say_links.emplace_back(
			fmt::format(
				"[{}] ",
				Saylink::Silent(
					fmt::format("#door model {}", g.object_name),
					g.object_name
				)
			)
		);
	}

	int                      character_length = 0;
	std::vector<std::string> buffered_links;

	for (auto &links: say_links) {
		buffered_links.emplace_back(links);
		character_length += links.length();

		// empty buffer
		if (character_length > MAX_CLIENT_MESSAGE_LENGTH) {
			std::string message_buffer;

			for (auto &buffered_link: buffered_links) {
				message_buffer += buffered_link;
			}

			c->Message(Chat::White, message_buffer.c_str());

			// reset
			character_length = 0;
			buffered_links   = {};
		}
	}

	if (!buffered_links.empty()) {
		c->Message(Chat::White, Strings::Implode(" ", buffered_links).c_str());
	}
}

void DoorManipulation::DisplayModelsFromFileResults(
	Client *c,
	std::vector<ToolGameObjectsRepository::ToolGameObjects> game_objects
)
{
	std::vector<std::string> say_links;

	for (auto &g: game_objects) {
		say_links.emplace_back(
			fmt::format(
				"[{}] ",
				Saylink::Silent(
					fmt::format("#door showmodelsfromfile {}", g.file_from),
					g.file_from
				)
			)
		);
	}

	int                      character_length = 0;
	std::vector<std::string> buffered_links;

	for (auto &links: say_links) {
		buffered_links.emplace_back(links);
		character_length += links.length();

		// empty buffer
		if (character_length > MAX_CLIENT_MESSAGE_LENGTH) {
			std::string message_buffer;

			for (auto &buffered_link: buffered_links) {
				message_buffer += buffered_link;
			}

			c->Message(Chat::White, message_buffer.c_str());

			// reset
			character_length = 0;
			buffered_links   = {};
		}
	}

	if (!buffered_links.empty()) {
		c->Message(Chat::White, Strings::Implode(" ", buffered_links).c_str());
	}
}
