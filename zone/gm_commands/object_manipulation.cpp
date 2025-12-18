#include "common/misc_functions.h"
#include "common/repositories/object_repository.h"
#include "common/strings.h"
#include "object_manipulation.h"
#include "zone/doors.h"
#include "zone/object.h"

#define MAX_CLIENT_MESSAGE_LENGTH 2000

void ObjectManipulation::CommandHandler(Client *c, const Seperator *sep)
{
	const int arguments = sep->argnum;
	if (!arguments) {
		ObjectManipulation::SendSubcommands(c);
		return;
	}

	const bool is_add        = !strcasecmp(sep->arg[1], "add");
	const bool is_delete     = !strcasecmp(sep->arg[1], "delete");
	const bool is_edit       = !strcasecmp(sep->arg[1], "edit");
	const bool is_icon       = !strcasecmp(sep->arg[1], "icon");
	const bool is_incline    = !strcasecmp(sep->arg[1], "incline");
	const bool is_model      = !strcasecmp(sep->arg[1], "model");
	const bool is_move       = !strcasecmp(sep->arg[1], "move");
	const bool is_rotate     = !strcasecmp(sep->arg[1], "rotate");
	const bool is_save       = !strcasecmp(sep->arg[1], "save");
	const bool is_size       = !strcasecmp(sep->arg[1], "size");
	const bool is_solid_type = !strcasecmp(sep->arg[1], "solid_type");
	const bool is_type       = !strcasecmp(sep->arg[1], "type");
	const bool is_undo       = !strcasecmp(sep->arg[1], "undo");

	if (
		!is_add &&
		!is_delete &&
		!is_edit &&
		!is_icon &&
		!is_incline &&
		!is_model &&
		!is_move &&
		!is_rotate &&
		!is_save &&
		!is_size &&
		!is_solid_type &&
		!is_type &&
		!is_undo
	) {
		ObjectManipulation::SendSubcommands(c);
		return;
	}

	if (is_add) {
		if (arguments < 2) {
			c->Message(
				Chat::White,
				"Usage: #object add [Type] [Model] [Icon] [Size] [Solid Type] [Incline] | Add an object"
			);
			c->Message(
				Chat::White,
				"Note: Model must start with a letter, max length 16. Solid Types | 0 (Solid), 1 (Sometimes Non-Solid)"
			);
			return;
		}

		Object_Struct od;

		memset(&od, 0, sizeof(od));

		const uint32 type = Strings::ToUnsignedInt(sep->arg[2]) == ObjectTypes::StaticLocked ? ObjectTypes::StaticUnlocked : Strings::ToUnsignedInt(sep->arg[2]);
		if (type == ObjectTypes::StaticLocked) {
			c->Message(
				Chat::White,
				"Note: Object Type 0 will act like a tradeskill container and will not reflect "
				"size, solidtype, or incline values until you commit with '#object save', after "
				"which it will be unchangeable until you use '#object edit' and zone back in."
			);
		} else if (type == ObjectTypes::Temporary) {
			c->Message(
				Chat::White,
				"Note: Object Type 1 is used for temporarily spawned ground spawns and dropped "
				"items, which are not supported with #object. See the 'ground_spawns' table in "
				"the database."
			);
			return;
		}

		std::string name = sep->arg[3];
		name = Strings::ToUpper(Strings::Replace(name, "'", ""));

		if (name[0] < 'A' || name[0] > 'Z') {
			c->Message(Chat::White, "Model name must start with a letter.");
			return;
		}

		const uint32 icon       = Strings::ToUnsignedInt(sep->arg[4]);
		const float  size       = Strings::ToFloat(sep->arg[5]);
		const uint16 solid_type = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[6]));
		const uint32 incline    = Strings::ToUnsignedInt(sep->arg[7]);

		const uint32 zone_id          = zone->GetZoneID();
		const uint16 instance_version = zone->GetInstanceVersion();

		od.incline       = incline;
		od.object_type   = type;
		od.size          = size;
		od.solid_type    = solid_type;
		od.x             = c->GetX();
		od.y             = c->GetY();
		od.z             = c->GetZ();
		od.heading       = c->GetHeading();
		od.zone_id       = zone_id;
		od.zone_instance = instance_version;

		strn0cpy(od.object_name, name.c_str(), sizeof(od.object_name));

		const auto &l = ObjectRepository::GetWhere(
			content_db,
			fmt::format(
				SQL(
					zoneid = {} AND (version = {} OR version = -1) AND
					xpos BETWEEN {:.2f} AND {:.2f} AND
					ypos BETWEEN {:.2f} AND {:.2f} AND
					zpos BETWEEN {:.2f} AND {:.2f}
				),
				zone_id,
				instance_version,
				od.x - 0.2f,
				od.x + 0.2f,
				od.y - 0.2f,
				od.y + 0.2f,
				od.z - 0.2f,
				od.z + 0.2f
			)
		);

		for (const auto& e : l) {
			c->Message(
				Chat::White,
				fmt::format(
					"ID: {} Name: {} XYZ: {:.2f}, {:.2f}, {:.2f} Heading: {:.2f}",
					e.id,
					e.objectname,
					e.xpos,
					e.ypos,
					e.zpos,
					e.heading
				).c_str()
			);
		}

		if (!l.empty()) {
			c->Message(Chat::White, "An object already exists at this location.");
			return;
		}

		const uint32 object_id = (ObjectRepository::GetMaxId(content_db) + 1);

		Object* o = new Object(object_id, od.object_type, icon, od);

		entity_list.AddObject(o, true);

		const float position_offset = 10.0f * sin(c->GetHeading() / 256.0f * 3.14159265f);
		c->MovePC(
			c->GetX() - position_offset,
			c->GetY() - position_offset,
			c->GetZ(),
			c->GetHeading()
		);

		const auto &save_saylink = Saylink::Silent(
			fmt::format(
				"#object save {}",
				object_id
			),
			"save"
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Spawned object ID {} at {:.2f}, {:.2f}, {:.2f}, {:.2f}. You can {} it if you'd like.",
				object_id,
				od.x,
				od.y,
				od.z,
				od.heading,
				save_saylink
			).c_str()
		);

		c->SetObjectToolEntityId(o->GetID());
	} else if (is_delete) {
		if (!c->GetObjectToolEntityId()) {
			c->Message(Chat::White, "You do not have a selected object.");
			return;
		}

		Object *o = entity_list.GetObjectByID(c->GetObjectToolEntityId());

		if (!o) {
			c->Message(Chat::White, "You do not have a valid selected object.");
			return;
		}

		const uint32 object_id = o->GetDBID();

		auto app = new EQApplicationPacket();
		o->CreateDeSpawnPacket(app);
		entity_list.QueueClients(nullptr, app);
		entity_list.RemoveObject(o->GetID());
		safe_delete(app);

		const int deleted_object = ObjectRepository::DeleteWhere(
			content_db,
			fmt::format(
				"id = {} AND zoneid = {} AND version = {}",
				object_id,
				zone->GetZoneID(),
				zone->GetInstanceVersion()
			)
		);

		if (deleted_object) {
			c->Message(
				Chat::White,
				fmt::format(
					"Successfully deleted Object ID {}.",
					object_id
				).c_str()
			);
		} else {
			c->Message(
				Chat::White,
				fmt::format(
					"Failed to delete Object ID {}.",
					object_id
				).c_str()
			);
		}
	} else if (is_edit) {
		Object *o = entity_list.GetObjectByID(c->GetObjectToolEntityId());

		const uint32 object_id = o->GetDBID();

		c->Message(
			Chat::White,
			fmt::format(
				"Object Selected ID [{}] Database ID [{}]",
				c->GetObjectToolEntityId(),
				object_id
			).c_str()
		);

		std::vector<std::string> move_x_options_positive;
		std::vector<std::string> move_x_options_negative;
		std::vector<std::string> move_y_options_positive;
		std::vector<std::string> move_y_options_negative;
		std::vector<std::string> move_z_options_positive;
		std::vector<std::string> move_z_options_negative;
		std::vector<std::string> rotate_options_positive;
		std::vector<std::string> rotate_options_negative;
		std::vector<std::string> size_options_positive;
		std::vector<std::string> size_options_negative;

		std::vector<std::string> xyz_values = {
			"0.1", "1", "5", "10", "25", "50", "100"
		};

		for (const auto &v: xyz_values) {
			const float current_x       = o->GetX();
			const float current_y       = o->GetY();
			const float current_z       = o->GetZ();
			const float current_heading = o->GetHeadingData();

			const float new_x       = current_x + Strings::ToFloat(v);
			const float new_y       = current_y + Strings::ToFloat(v);
			const float new_z       = current_z + Strings::ToFloat(v);
			const float new_heading = current_heading + Strings::ToFloat(v);

			move_x_options_positive.emplace_back(
				Saylink::Silent(
					fmt::format(
						"#object move {:.2f} {:.2f} {:.2f}",
						new_x,
						current_y,
						current_z
					),
					v
				)
			);

			move_y_options_positive.emplace_back(
				Saylink::Silent(
					fmt::format(
						"#object move {:.2f} {:.2f} {:.2f}",
						current_x,
						new_y,
						current_z
					),
					v
				)
			);

			move_z_options_positive.emplace_back(
				Saylink::Silent(
					fmt::format(
						"#object move {:.2f} {:.2f} {:.2f}",
						current_x,
						current_y,
						new_z
					),
					v
				)
			);

			rotate_options_positive.emplace_back(
				Saylink::Silent(
					fmt::format(
						"#object rotate {:.2f}",
						new_heading
					),
					v
				)
			);
		}

		for (auto v = xyz_values.rbegin(); v != xyz_values.rend(); ++v) {
			const float current_x       = o->GetX();
			const float current_y       = o->GetY();
			const float current_z       = o->GetZ();
			const float current_heading = o->GetHeadingData();

			const float new_x       = current_x - Strings::ToFloat(*v);
			const float new_y       = current_y - Strings::ToFloat(*v);
			const float new_z       = current_z - Strings::ToFloat(*v);
			const float new_heading = current_heading - Strings::ToFloat(*v);

			move_x_options_negative.emplace_back(
				Saylink::Silent(
					fmt::format(
						"#object move {:.2f} {:.2f} {:.2f}",
						new_x,
						current_y,
						current_z
					),
					*v
				)
			);

			move_y_options_negative.emplace_back(
				Saylink::Silent(
					fmt::format(
						"#object move {:.2f} {:.2f} {:.2f}",
						current_x,
						new_y,
						current_z
					),
					*v
				)
			);

			move_z_options_negative.emplace_back(
				Saylink::Silent(
					fmt::format(
						"#object move {:.2f} {:.2f} {:.2f}",
						current_x,
						current_y,
						new_z
					),
					*v
				)
			);

			rotate_options_negative.emplace_back(
				Saylink::Silent(
					fmt::format(
						"#object rotate {:.2f}",
						new_heading
					),
					*v
				)
			);
		}

		std::vector<std::string> size_values = {
			"1", "5", "10", "25", "50", "100", "1000"
		};

		for (const auto &v: size_values) {
			const float current_size = o->GetSize();

			const float new_size = current_size + Strings::ToFloat(v);

			size_options_positive.emplace_back(
				Saylink::Silent(fmt::format("#door size {:.2f}", new_size), v)
			);
		}

		for (auto v = size_values.rbegin(); v != size_values.rend(); ++v) {
			const float current_size = o->GetSize();

			const float new_size = current_size - Strings::ToFloat(*v);

			size_options_negative.emplace_back(
				Saylink::Silent(fmt::format("#door edit {:.2f}", new_size, *v), *v)
			);
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Name [{}] [{}] [{}] [{}]",
				o->GetModelName(),
				Saylink::Silent("#object save", "Save"),
				Saylink::Silent("#object delete", "Delete"),
				Saylink::Silent("#object undo", "Undo")
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
				Strings::Implode(" | ", rotate_options_negative),
				Strings::Implode(" | ", rotate_options_positive)
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"[{}] - [Size] + [{}]",
				Strings::Implode(" | ", size_options_negative),
				Strings::Implode(" | ", size_options_positive)
			).c_str()
		);

		return;
	} else if (
		is_icon ||
		is_incline ||
		is_model ||
		is_size ||
		is_solid_type ||
		is_type
	) {
		if (!c->GetObjectToolEntityId()) {
			c->Message(Chat::White, "You do not have a selected object.");
			return;
		}

		Object *o = entity_list.GetObjectByID(c->GetObjectToolEntityId());

		const uint32 object_id = o->GetDBID();

		Object_Struct od;

		if (!o) {
			auto e = ObjectRepository::FindOne(content_db, object_id);
			if (!e.id) {
				c->Message(
					Chat::White,
					fmt::format(
						"Object ID {} does not exist.",
						object_id
					).c_str()
				);
				return;
			}

			if (e.zoneid != zone->GetZoneID()) {
				c->Message(
					Chat::White,
					fmt::format(
						"Object ID {} is not a part of this zone.",
						object_id
					).c_str()
				);
				return;
			}

			if (e.version != zone->GetInstanceVersion()) {
				c->Message(
					Chat::White,
					fmt::format(
						"Object ID {} is not a part of this instance version.",
						object_id
					).c_str()
				);
				return;
			}

			memset(&od, 0, sizeof(od));

			od.zone_id       = e.zoneid;
			od.zone_instance = e.version;
			od.object_type   = e.type;

			if (od.object_type == ObjectTypes::StaticLocked) {
				e.type = ObjectTypes::StaticUnlocked;
				const int updated = ObjectRepository::UpdateOne(content_db, e);
				if (updated) {
					const std::string &reload_saylink = Saylink::Silent(
						"#reload objects",
						"reload"
					);

					const auto &save_saylink = Saylink::Silent(
						fmt::format(
							"#object save {}",
							object_id
						),
						"save"
					);

					c->Message(
						Chat::White,
						fmt::format(
							"Object ID {} unlocked, you must {} to make changes then you can {}.",
							object_id,
							reload_saylink,
							save_saylink
						).c_str()
					);
					return;
				}
			} else if (od.object_type == ObjectTypes::Temporary) {
				c->Message(
					Chat::White,
					"Note: Object Type 1 is used for temporarily spawned ground spawns and dropped "
					"items, which are not supported with #object. See the 'ground_spawns' table in "
					"the database."
				);
				return;
			}
		}

		uint32 icon = o->GetIcon();
		o->GetObjectData(&od);

		if (is_icon) {
			if (
				od.object_type <= ObjectTypes::Temporary ||
				od.object_type == ObjectTypes::StaticUnlocked
			) {
				c->Message(
					Chat::White,
					fmt::format(
						"Object ID {} is not a Tradeskill Object and does not support the 'icon' property.",
						object_id
					).c_str()
				);
				return;
			}

			if (!Strings::IsNumber(sep->arg[2])) {
				c->Message(Chat::White, "Invalid icon specified. Please enter a valid icon.");
				return;
			}

			icon = Strings::ToUnsignedInt(sep->arg[2]);

			o->SetIcon(icon);

			c->Message(
				Chat::White,
				fmt::format(
					"Object ID {} icon set to {}.",
					object_id,
					icon
				).c_str()
			);
		} else if (is_incline) {
			if (od.object_type != ObjectTypes::StaticUnlocked) {
				c->Message(
					Chat::White,
					fmt::format(
						"Object ID {} is not a static object and does not support the 'incline' property.",
						object_id
					).c_str()
				);
				return;
			}

			if (!Strings::IsNumber(sep->arg[2])) {
				c->Message(Chat::White, "Invalid incline specified. Please enter a valid incline.");
				return;
			}

			od.incline = Strings::ToUnsignedInt(sep->arg[2]);
			o->SetObjectData(&od);

			const auto &save_saylink = Saylink::Silent(
				fmt::format(
					"#object save {}",
					object_id
				),
				"save"
			);

			c->Message(
				Chat::White,
				fmt::format(
					"Object ID {} set to {} incline. You will need to {} to retain this change.",
					object_id,
					od.incline,
					save_saylink
				).c_str()
			);
		} else if (is_model) {
			const std::string &object_name = sep->argplus[2];

			if (object_name[0] < 'A' || object_name[0] > 'Z') {
				c->Message(Chat::White, "Model names must begin with a letter.");
				return;
			}

			strn0cpy(od.object_name, object_name.c_str(), sizeof(od.object_name));

			c->Message(
				Chat::White,
				fmt::format(
					"Object ID {} is now using model '{}'",
					object_id,
					object_name
				).c_str()
			);
		} else if (is_size) {
			if (od.object_type != ObjectTypes::StaticUnlocked) {
				c->Message(
					Chat::White,
					fmt::format(
						"Object ID {} is not a static object and does not support the 'size' property.",
						object_id
					).c_str()
				);
				return;
			}

			if (!Strings::IsNumber(sep->arg[2])) {
				c->Message(Chat::White, "Invalid size specified. Please provide a valid size.");
				return;
			}

			od.size = Strings::ToFloat(sep->arg[2]);
			o->SetObjectData(&od);

			if (od.size == 0.0f) {
				od.size = 100.0f;
			}

			const auto &save_saylink = Saylink::Silent(
				fmt::format(
					"#object save {}",
					object_id
				),
				"save"
			);

			c->Message(
				Chat::White,
				fmt::format(
					"Object ID {} set to {:.2f} size. You will need to {} to retain this change.",
					object_id,
					od.size,
					save_saylink
				).c_str()
			);
		} else if (is_solid_type) {
			if (od.object_type != ObjectTypes::StaticUnlocked) {
				c->Message(
					Chat::White,
					fmt::format(
						"Object ID {} is not a static object and does not support the 'solid_type' property.",
						object_id
					).c_str()
				);
				return;
			}

			if (!Strings::IsNumber(sep->arg[2])) {
				c->Message(Chat::White, "Invalid solid type specified. Please provide a valid solid type.");
				return;
			}

			od.solid_type = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[2]));
			o->SetObjectData(&od);

			const auto &save_saylink = Saylink::Silent(
				fmt::format(
					"#object save {}",
					object_id
				),
				"save"
			);

			c->Message(
				Chat::White,
				fmt::format(
					"Object ID {} set to solid type {}. You will need to {} to retain this change.",
					object_id,
					od.solid_type,
					save_saylink
				).c_str()
			);
		} else if (is_type) {
			if (!Strings::IsNumber(sep->arg[2])) {
				c->Message(Chat::White, "Invalid type specified. Please enter a valid type.");
				return;
			}

			od.object_type = Strings::ToUnsignedInt(sep->arg[2]);

			if (od.object_type == ObjectTypes::StaticLocked) {
				const auto &save_saylink = Saylink::Silent(
					fmt::format(
						"#object save {}",
						object_id
					),
					"save"
				);

				c->Message(
					Chat::White,
					fmt::format(
						"Note: Static Objects will not reflect property changes until you {}.",
						save_saylink
					).c_str()
				);
				return;
			} else if (od.object_type == ObjectTypes::Temporary) {
				c->Message(
					Chat::White,
					"Note: Object Type 1 is used for temporarily spawned ground spawns and dropped "
					"items, which are not supported with #object. See the 'ground_spawns' table in "
					"the database."
				);
				return;
			}

			o->SetType(od.object_type);

			c->Message(
				Chat::White,
				fmt::format(
					"Object ID {} set to type {}.",
					object_id,
					od.object_type
				).c_str()
			);
		}

		auto app = new EQApplicationPacket();
		o->CreateDeSpawnPacket(app);
		entity_list.QueueClients(0, app);
		safe_delete(app);

		app = new EQApplicationPacket();
		o->CreateSpawnPacket(app);
		entity_list.QueueClients(0, app);
		safe_delete(app);
		return;
	} else if (is_move) {
		if (!c->GetObjectToolEntityId()) {
			c->Message(Chat::White, "You do not have a selected object.");
			return;
		}

		if (arguments < 2) {
			c->Message(Chat::White, "Usage: #object move [X] [Y] [Z] | Move the selected object");
			return;
		}

		Object_Struct od;

		Object *o = entity_list.GetObjectByID(c->GetObjectToolEntityId());

		const uint32 object_id = o->GetDBID();

		if (!o) {
			const auto &e = ObjectRepository::FindOne(content_db, object_id);

			if (!e.id) {
				c->Message(
					Chat::White,
					fmt::format(
						"Object ID {} does not exist.",
						object_id
					).c_str()
				);
				return;
			}

			od.zone_id       = e.zoneid;
			od.zone_instance = e.version;
			od.object_type   = e.type;

			if (e.zoneid != zone->GetZoneID()) {
				c->Message(
					Chat::White,
					fmt::format(
						"Object ID {} is not a part of this zone.",
						object_id
					).c_str()
				);
				return;
			}

			if (e.version != zone->GetInstanceVersion()) {
				c->Message(
					Chat::White,
					fmt::format(
						"Object ID {} is not a part of this instance version.",
						object_id
					).c_str()
				);
				return;
			}

			if (od.object_type == ObjectTypes::StaticLocked) {
				c->Message(
					Chat::White,
					fmt::format(
						"Object ID {} is not unlocked for editing. Use '#object edit' to unlock it.",
						object_id
					).c_str()
				);
				return;
			} else if (od.object_type == ObjectTypes::Temporary) {
				c->Message(
					Chat::White,
					fmt::format(
						"Object ID {} is a temporarily spawned ground spawn or dropped item, which is not supported with #object. See the 'ground_spawns' table in the database.",
						object_id
					).c_str()
				);
				return;
			} else if (od.object_type == ObjectTypes::StaticUnlocked) {
				const std::string &reload_saylink = Saylink::Silent(
					"#reload objects",
					"reload"
				);

				const std::string &save_saylink = Saylink::Silent(
					fmt::format(
						"#object save {}",
						object_id
					),
					"save"
				);

				c->Message(
					Chat::White,
					fmt::format(
						"Object ID {} has been unlocked for editing, you must {} to make changes then you can {}.",
						object_id,
						reload_saylink,
						save_saylink
					).c_str()
				);
			} else {
				c->Message(
					Chat::White,
					fmt::format(
						"Object ID {} could not be located.",
						object_id
					).c_str()
				);
				return;
			}
		}

		const float x = Strings::IsFloat(sep->arg[2]) ? Strings::ToFloat(sep->arg[2]) : 0.0f;
		const float y = arguments >= 3 ? Strings::ToFloat(sep->arg[3]) : 0.0f;
		const float z = arguments >= 4 ? Strings::ToFloat(sep->arg[4]) : 0.0f;

		od.x = x;

		if (arguments >= 3) {
			od.y = y;
		} else {
			o->GetLocation(nullptr, &od.y, nullptr);
		}

		if (arguments >= 4) {
			od.z = z;
		} else {
			o->GetLocation(nullptr, nullptr, &od.z);
		}

		o->SetLocation(od.x, od.y, od.z);

		auto app = new EQApplicationPacket();
		o->CreateDeSpawnPacket(app);
		entity_list.QueueClients(0, app);
		safe_delete(app);

		app = new EQApplicationPacket();
		o->CreateSpawnPacket(app);
		entity_list.QueueClients(0, app);
		safe_delete(app);
		return;
	} else if (is_rotate) {
		if (!c->GetObjectToolEntityId()) {
			c->Message(Chat::White, "You do not have a selected object.");
			return;
		}

		if (!sep->IsNumber(2)) {
			c->Message(Chat::White, "Usage: #object rotate [Heading] | Rotate the selected object");
			return;
		}

		Object *o = entity_list.GetObjectByID(c->GetObjectToolEntityId());

		const uint32 object_id = o->GetDBID();

		const float heading = Strings::ToFloat(sep->arg[2]);

		o->SetHeading(heading);

		auto app = new EQApplicationPacket();
		o->CreateDeSpawnPacket(app);
		entity_list.QueueClients(0, app);
		safe_delete(app);

		app = new EQApplicationPacket();
		o->CreateSpawnPacket(app);
		entity_list.QueueClients(0, app);
		safe_delete(app);
		return;
	} else if (is_save) {
		if (!c->GetObjectToolEntityId()) {
			c->Message(Chat::White, "You do not have a selected object.");
			return;
		}

		Object_Struct od;

		Object *o = entity_list.GetObjectByID(c->GetObjectToolEntityId());

		uint32 object_id = o->GetDBID();

		od.zone_id       = 0;
		od.zone_instance = 0;
		od.object_type   = 0;

		bool is_new = true;

		const auto &e = ObjectRepository::FindOne(content_db, object_id);
		if (e.id) {
			od.zone_id       = e.zoneid;
			od.zone_instance = e.version;
			od.object_type   = e.type;

			is_new = false;
		}

		if (od.zone_id && od.zone_id != zone->GetZoneID()) {
			object_id = 0;
		}

		if (object_id > 0 && od.zone_instance != zone->GetInstanceVersion()) {
			object_id = 0;
		}

		is_new |= (object_id == 0);

		o->GetObjectData(&od);

		od.object_type = o->GetType();

		const uint32 icon = o->GetIcon();

		if (od.object_type == ObjectTypes::StaticUnlocked) {
			od.object_type = ObjectTypes::StaticLocked;
		}

		int updated = 0;

		if (!is_new) {
			auto e = ObjectRepository::FindOne(content_db, object_id);

			e.xpos            = od.x;
			e.ypos            = od.y;
			e.zpos            = od.z;
			e.heading         = od.heading;
			e.objectname      = od.object_name;
			e.type            = od.object_type;
			e.icon            = icon;
			e.size_percentage = od.size;
			e.solid_type      = od.solid_type;
			e.incline         = od.incline;

			updated = ObjectRepository::UpdateOne(content_db, e);
		} else if (!object_id) {
			auto e = ObjectRepository::NewEntity();

			e.xpos            = od.x;
			e.ypos            = od.y;
			e.zpos            = od.z;
			e.heading         = od.heading;
			e.objectname      = od.object_name;
			e.type            = od.object_type;
			e.icon            = icon;
			e.size_percentage = od.size;
			e.solid_type      = od.solid_type;
			e.incline         = od.incline;
			e.zoneid          = zone->GetZoneID();
			e.version         = zone->GetInstanceVersion();

			e = ObjectRepository::InsertOne(content_db, e);
			updated = e.id ? 2 : 0;
			object_id = e.id;
		} else {
			auto e = ObjectRepository::NewEntity();

			e.id              = object_id;
			e.xpos            = od.x;
			e.ypos            = od.y;
			e.zpos            = od.z;
			e.heading         = od.heading;
			e.objectname      = od.object_name;
			e.type            = od.object_type;
			e.icon            = icon;
			e.size_percentage = od.size;
			e.solid_type      = od.solid_type;
			e.incline         = od.incline;
			e.zoneid          = zone->GetZoneID();
			e.version         = zone->GetInstanceVersion();

			updated = ObjectRepository::InsertOne(content_db, e).id ? 1 : 0;
		}

		if (!updated) {
			c->Message(
				Chat::White,
				fmt::format(
					"Object ID {} could not be saved.",
					object_id
				).c_str()
			);
			return;
		}

		if (is_new) {
			if (updated == 1) {
				c->Message(
					Chat::White,
					fmt::format(
						"Object ID {} has been saved.",
						object_id
					).c_str()
				);
			} else if (updated == 2) {
				c->Message(
					Chat::White,
					fmt::format(
						"Object ID {} has been created.",
						object_id
					).c_str()
				);
			}
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Object ID {} has been saved.",
				object_id
			).c_str()
		);

		if (od.object_type == ObjectTypes::StaticLocked) {
			auto app = new EQApplicationPacket();
			o->CreateDeSpawnPacket(app);
			entity_list.QueueClients(0, app);
			safe_delete(app);

			entity_list.RemoveObject(o->GetID());

			auto door = DoorsRepository::NewEntity();

			door.zone = zone->GetShortName();

			door.id      = 1000000000 + object_id; // Out of range of normal use for doors.id
			door.doorid  = -1; // Client doesn't care if these are all the same door_id
			door.pos_x   = od.x;
			door.pos_y   = od.y;
			door.pos_z   = od.z;
			door.heading = od.heading;

			door.name = Strings::Replace(od.object_name, "_ACTORDEF", "");

			door.dest_zone = "NONE";

			if ((door.size = od.size) == 0) {
				door.size = 100;
			}

			door.opentype = od.solid_type;

			switch (door.opentype) {
				case 0:
					door.opentype = 31;
					break;
				case 1:
					door.opentype = 9;
					break;
			}

			door.incline             = od.incline; // unknown20 = optional incline value
			door.client_version_mask = 0xFFFFFFFF;

			Doors *doors = new Doors(door);

			entity_list.AddDoor(doors);

			app = new EQApplicationPacket(OP_SpawnDoor, sizeof(Door_Struct));
			auto ds = (Door_Struct *) app->pBuffer;

			memset(ds, 0, sizeof(Door_Struct));
			memcpy(ds->name, door.name.c_str(), 32);

			ds->xPos     = door.pos_x;
			ds->yPos     = door.pos_y;
			ds->zPos     = door.pos_z;
			ds->heading  = door.heading;
			ds->incline  = door.incline;
			ds->size     = door.size;
			ds->doorId   = door.doorid;
			ds->opentype = door.opentype;
			ds->unknown0052[9]  = 1; // *ptr-1 and *ptr-3 from EntityList::MakeDoorSpawnPacket()
			ds->unknown0052[11] = 1;

			entity_list.QueueClients(0, app);
			safe_delete(app);

			c->Message(
				Chat::White,
				fmt::format(
					"Object ID {} is now a static object and cannot be edited. Use '#object edit' to edit it again.",
					object_id
				).c_str()
			);
		}
	} else if (is_undo) {
		if (!c->GetObjectToolEntityId()) {
			c->Message(Chat::White, "You do not have a selected object.");
			return;
		}

		Object *o = entity_list.GetObjectByID(c->GetObjectToolEntityId());

		const uint32 object_id = o->GetDBID();

		if (o->GetType() == ObjectTypes::Temporary) {
			c->Message(
				Chat::White,
				"Note: Object Type 1 is used for temporarily spawned ground spawns and dropped "
				"items, which are not supported with #object. See the 'ground_spawns' table in "
				"the database."
			);
			return;
		}

		auto app = new EQApplicationPacket();
		o->CreateDeSpawnPacket(app);
		entity_list.QueueClients(0, app);
		entity_list.RemoveObject(o->GetID());
		safe_delete(app);

		const auto &e = ObjectRepository::FindOne(content_db, object_id);

		if (!e.id) {
			c->Message(
				Chat::White,
				fmt::format(
					"Object ID {} does not exist.",
					object_id
				).c_str()
			);
			return;
		}

		Object_Struct od;

		memset(&od, 0, sizeof(od));

		const uint32 icon = e.icon;

		od.x           = e.xpos;
		od.y           = e.ypos;
		od.z           = e.zpos;
		od.heading     = e.heading;
		od.object_type = e.type;
		od.size        = e.size_percentage;
		od.solid_type  = e.solid_type;
		od.incline     = e.incline;

		strn0cpy(od.object_name, e.objectname.c_str(), sizeof(od.object_name));

		if (od.object_type == ObjectTypes::StaticUnlocked) {
			od.object_type = ObjectTypes::StaticLocked;
		}

		o = new Object(object_id, od.object_type, icon, od);

		entity_list.AddObject(o, true);

		c->Message(
			Chat::White,
			fmt::format(
				"Object ID {} reloaded from database.",
				object_id
			).c_str()
		);
	}
}

void ObjectManipulation::CommandHeader(Client *c)
{
	c->Message(Chat::White, "------------------------------------------------");
	c->Message(Chat::White, "# Object Commands");
	c->Message(Chat::White, "------------------------------------------------");
}

void ObjectManipulation::SendSubcommands(Client *c)
{
	ObjectManipulation::CommandHeader(c);
	c->Message(
		Chat::White,
		"Usage: #object add [Type] [Model] [Icon] [Size] [Solid Type] [Incline] | Add an object"
	);
	c->Message(
		Chat::White,
		"Note: Model must start with a letter, max length 16. Solid Types | 0 (Solid), 1 (Sometimes Non-Solid)"
	);
	c->Message(Chat::White, "Note: Only objects saved in the database can be copied to another instance");
	c->Message(Chat::White, "Usage: #object delete | Delete the selected object");
	c->Message(Chat::White, "Usage: #object edit | Edit the selected object");
	c->Message(Chat::White, "Note: Static Object (Type 0) Properties: model, type, size, solid_type, incline");
	c->Message(Chat::White, "Note: Tradeskill Object (Type 2+) Properties: model, type, icon");
	c->Message(Chat::White, "Usage: #object icon [Icon] | Edit the selected object's icon");
	c->Message(Chat::White, "Usage: #object incline [Incline] | Edit the selected object's incline");
	c->Message(Chat::White, "Usage: #object model [Model] | Edit the selected object's model");
	c->Message(Chat::White, "Usage: #object move [0|X] [Y] [Z] [H] | Move the selected object");
	c->Message(Chat::White, "Note: Using 0 for X moves the object to your position, heading is optional");
	c->Message(Chat::White, "Usage: #object rotate [Heading] | Rotate the selected object");
	c->Message(Chat::White, "Usage: #object save | Save the selected object");
	c->Message(Chat::White, "Usage: #object size [Size] | Edit the selected object's size");
	c->Message(Chat::White, "Usage: #object solid_type [Solid Type] | Edit the selected object's solid type");
	c->Message(Chat::White, "Usage: #object type [Type] | Edit the selected object's type");
	c->Message(Chat::White, "Usage: #object undo | Reload the selected object from the database");
}
