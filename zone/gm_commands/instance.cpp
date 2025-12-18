#include "zone/client.h"

void command_instance(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(
			Chat::White,
			"Usage: #instance create [Zone ID|Zone Short Name] [Version] [Duration]"
		);
		c->Message(
			Chat::White,
			"Usage: #instance destroy [Instance ID]"
		);
		c->Message(
			Chat::White,
			"Usage: #instance add [Instance ID] [Name]"
		);
		c->Message(
			Chat::White,
			"Usage: #instance remove [Instance ID] [Name]"
		);
		c->Message(
			Chat::White,
			"Usage: #instance list [Name]"
		);
		return;
	}

	Client* target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	bool is_add = !strcasecmp(sep->arg[1], "add");
	bool is_create = !strcasecmp(sep->arg[1], "create");
	bool is_destroy = !strcasecmp(sep->arg[1], "destroy");
	bool is_help = !strcasecmp(sep->arg[1], "help");
	bool is_list = !strcasecmp(sep->arg[1], "list");
	bool is_remove = !strcasecmp(sep->arg[1], "remove");
	if (
		!is_add &&
		!is_create &&
		!is_destroy &&
		!is_help &&
		!is_list &&
		!is_remove
	) {
		c->Message(
			Chat::White,
			"Usage: #instance create [Zone ID|Zone Short Name] [Version] [Duration]"
		);
		c->Message(
			Chat::White,
			"Usage: #instance destroy [Instance ID]"
		);
		c->Message(
			Chat::White,
			"Usage: #instance add [Instance ID] [Name]"
		);
		c->Message(
			Chat::White,
			"Usage: #instance remove [Instance ID] [Name]"
		);
		c->Message(
			Chat::White,
			"Usage: #instance list [Name]"
		);
		return;
	}

	if (is_add) {
		if (!sep->IsNumber(2)) {
			c->Message(
				Chat::White,
				"#instance add [Instance ID] [Name]"
			);
			return;
		}

		std::string character_name = sep->arg[3];
		uint16 instance_id = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[2]));
		uint32 character_id = database.GetCharacterID(character_name);
		if (instance_id <= 0 || character_id <= 0) {
			c->Message(Chat::White, "You must enter a valid Instance ID and player name.");
			return;
		}

		if (!database.CheckInstanceExists(instance_id)) {
			c->Message(
				Chat::White,
				fmt::format(
					"Instance ID {} does not exist.",
					instance_id
				).c_str()
			);
			return;
		}

		auto zone_id = database.GetInstanceZoneID(instance_id);
		auto version = database.GetInstanceVersion(instance_id);
		auto current_id  = database.GetInstanceID(zone_id, character_id, version);
		if (!current_id) {
			c->Message(
				Chat::White,
				(
					database.AddClientToInstance(instance_id, character_id) ?
					fmt::format(
						"Added {} to Instance ID {}.",
						c->GetTargetDescription(target),
						instance_id
					) :
					fmt::format(
						"Failed to add {} to Instance ID {}.",
						c->GetTargetDescription(target),
						instance_id
					)
				).c_str()
			);
		}
		else {
			c->Message(
				Chat::White,
				fmt::format(
					"Client was already saved to Instance ID {}{}.",
					current_id,
					(
						current_id != instance_id ?
						fmt::format(
							"which has the same zone and version as Instance ID {}",
							instance_id
						) :
						""
					)
				).c_str()
			);
		}
	} else if (is_create) {
		if (!sep->IsNumber(3) || !sep->IsNumber(4)) {
			c->Message(
				Chat::White,
				"Usage: #instance create [Zone ID|Zone Short Name] [Version] [Duration]"
			);
			return;
		}

		uint32 zone_id = (
			sep->IsNumber(2) ?
			Strings::ToUnsignedInt(sep->arg[2]) :
			ZoneID(sep->arg[2])
		);
		uint32 version = Strings::ToUnsignedInt(sep->arg[3]);
		uint32 duration = Strings::ToUnsignedInt(sep->arg[4]);
		std::string zone_short_name = ZoneName(zone_id);
		if (zone_short_name.empty()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Zone ID {} was not found by the server.",
					zone_id
				).c_str()
			);
			return;
		}

		uint16 instance_id = 0;
		if (!database.GetUnusedInstanceID(instance_id)) {
			c->Message(Chat::White, "Server was unable to find a free instance id.");
			return;
		}

		if (!database.CreateInstance(instance_id, zone_id, version, duration)) {
			c->Message(Chat::White, "Server was unable to create a new instance.");
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Instance {} Created | Zone: {} ({}){}",
				instance_id,
				ZoneLongName(zone_id),
				zone_id,
				(
					version ?
					fmt::format(
						" Version: {}",
						version
					) :
					""
				)
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Instance {} Created |  Duration: {} ({})",
				instance_id,
				Strings::SecondsToTime(duration),
				duration
			).c_str()
		);
	} else if (is_destroy) {
		if (!sep->IsNumber(2)) {
			c->Message(
				Chat::White,
				"#instance destroy [Instance ID]"
			);
			return;
		}

		uint16 instance_id = Strings::ToUnsignedInt(sep->arg[2]);
		if (!database.CheckInstanceExists(instance_id)) {
			c->Message(
				Chat::White,
				fmt::format(
					"Instance ID {} does not exist.",
					instance_id
				).c_str()
			);
			return;
		}

		database.DeleteInstance(instance_id);
		c->Message(
			Chat::White,
			fmt::format(
				"Instance ID {} Deleted.",
				instance_id
			).c_str()
		);
	} else if (is_help) {
		c->Message(
			Chat::White,
			"Usage: #instance create [Zone ID|Zone Short Name] [Version] [Duration]"
		);
		c->Message(
			Chat::White,
			"Usage: #instance destroy [Instance ID]"
		);
		c->Message(
			Chat::White,
			"Usage: #instance add [Instance ID] [Name]"
		);
		c->Message(
			Chat::White,
			"Usage: #instance remove [Instance ID] [Name]"
		);
		c->Message(
			Chat::White,
			"Usage: #instance list [Name]"
		);
		return;
	} else if (is_list) {
		uint32 character_id = database.GetCharacterID(sep->arg[2]);
		if (character_id <= 0) {
			character_id = target->CharacterID();
		}

		database.ListAllInstances(c, character_id);
	} else if (is_remove) {
		if (!sep->IsNumber(2)) {
			c->Message(
				Chat::White,
				"#instance remove [Instance ID] [Name]"
			);
			return;
		}

		std::string character_name = sep->arg[3];
		uint16 instance_id = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[2]));
		uint32 character_id = database.GetCharacterID(character_name);
		if (instance_id <= 0 || character_id <= 0) {
			c->Message(Chat::White, "You must enter a valid Instance ID and player name.");
			return;
		}

		if (!database.CheckInstanceExists(instance_id)) {
			c->Message(
				Chat::White,
				fmt::format(
					"Instance ID {} does not exist.",
					instance_id
				).c_str()
			);
			return;
		}

		c->Message(
			Chat::White,
			(
				database.RemoveClientFromInstance(instance_id, character_id) ?
				fmt::format(
					"Removed {} from Instance ID {}.",
					c->GetTargetDescription(target),
					instance_id
				) :
				fmt::format(
					"Failed to remove {} from Instance ID {}.",
					c->GetTargetDescription(target),
					instance_id
				)
			).c_str()
		);
	}
}

