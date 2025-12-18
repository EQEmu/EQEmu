#include "zone/client.h"
#include "zone/npc_scale_manager.h"

void command_scale(Client *c, const Seperator *sep)
{
	auto arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #scale [Scale Type] - Must target an NPC");
		c->Message(Chat::White, "Usage: #scale [Search Criteria] [Scale Type] - Zone-wide Changes");
		c->Message(Chat::White, "Usage: #scale all [Scale Type] - Scale every NPC in the zone");
		c->Message(Chat::White, "Note: Scale Type can be \"dynamic\" or \"static\".");
		return;
	}

	if (c->GetTarget() && c->GetTarget()->IsNPC() && arguments < 2) {
		auto n = c->GetTarget()->CastToNPC();

		bool apply_status = false;

		bool is_dynamic = !strcasecmp(sep->arg[1], "dynamic");
		bool is_static  = !strcasecmp(sep->arg[1], "static");

		if (is_dynamic || is_static) {
			c->Message(
				Chat::White,
				fmt::format(
					"Applying global base scaling to {} {}ally, all stats set to zeroes.",
					c->GetTargetDescription(n),
					sep->arg[1]
				).c_str()
			);

			if (is_dynamic) {
				apply_status = npc_scale_manager->ApplyGlobalBaseScalingToNPCDynamically(n);
			} else {
				apply_status = npc_scale_manager->ApplyGlobalBaseScalingToNPCStatically(n);
			}
		} else {
			c->Message(Chat::White, "Usage: #scale [Scale Type] - Must target an NPC");
			c->Message(Chat::White, "Usage: #scale [Search Criteria] [Scale Type] - Zone-wide Changes");
			c->Message(Chat::White, "Usage: #scale all [Scale Type] - Scale every NPC in the zone");
			c->Message(Chat::White, "Note: Scale Type can be \"dynamic\" or \"static\".");
			return;
		}

		if (apply_status) {
			c->Message(
				Chat::White,
				fmt::format(
					"Applied global base scaling to {} {}ally successfully.",
					c->GetTargetDescription(n),
					sep->arg[1]
				).c_str()
			);
		} else {
			c->Message(
				Chat::White,
				fmt::format(
					"Failed to load scaling data from the database for {}, see 'NPCScaling' log for more info.",
					c->GetTargetDescription(n)
				).c_str()
			);
		}
	} else if (c->GetTarget() && arguments < 2) {
		c->Message(Chat::White, "You must target an NPC to use targeted scaling!");
	}

	if (arguments > 1) {
		std::string scale_type;

		bool is_all     = !strcasecmp(sep->arg[1], "all");
		bool is_dynamic = !strcasecmp(sep->arg[2], "dynamic");
		bool is_static  = !strcasecmp(sep->arg[2], "static");

		bool is_apply = arguments == 3 && !strcasecmp(sep->arg[3], "apply");

		if (is_dynamic) {
			scale_type = "dynamic";
		} else if (is_static) {
			scale_type = "static";
		} else {
			c->Message(Chat::White, "Usage: #scale [Scale Type] - Must target an NPC");
			c->Message(Chat::White, "Usage: #scale [Search Criteria] [Scale Type] - Zone-wide Changes");
			c->Message(Chat::White, "Usage: #scale all [Scale Type] - Scale every NPC in the zone");
			c->Message(Chat::White, "Note: Scale Type can be \"dynamic\" or \"static\".");
			return;
		}

		const std::string search_string = sep->arg[1];

		const auto& l = entity_list.GetNPCList();

		auto found_count  = 0;
		auto found_number = 1;

		for (const auto &e : l) {
			auto n = e.second;

			std::string entity_name = n->GetName();

			if (
				!search_string.empty() &&
				!Strings::Contains(entity_name, search_string) &&
				!is_all
			) {
				continue;
			}

			std::string status = "(Searching)";

			if (is_apply) {
				status = "(Applying)";

				if (is_dynamic) {
					npc_scale_manager->ApplyGlobalBaseScalingToNPCDynamically(n);
				} else if (is_static) {
					npc_scale_manager->ApplyGlobalBaseScalingToNPCStatically(n);
				}
			}

			c->Message(
				Chat::White,
				fmt::format(
					"Entity {} | Name: {} | NPC ID: {} | Position: {:.2f}, {:.2f}, {:.2f}, {:.2f} {}",
					found_number,
					c->GetTargetDescription(n),
					n->GetNPCTypeID(),
					n->GetX(),
					n->GetY(),
					n->GetZ(),
					n->GetHeading(),
					status
				).c_str()
			);

			found_count++;
			found_number++;
		}

		if (is_apply) {
			c->Message(
				Chat::White,
				fmt::format(
					"{} scaling applied against {} NPC{}.",
					Strings::UcFirst(sep->arg[2]),
					found_count,
					found_count != 1 ? "s" : ""
				).c_str()
			);
		} else {
			c->Message(
				Chat::White,
				fmt::format(
					"Found {} NPC{}{}.",
					found_count,
					found_count != 1 ? "s" : "",
					(
						is_all ?
						"" :
						fmt::format(
							" matching '{}'",
							search_string
						)
					)
				).c_str()
			);

			c->Message(
				Chat::White,
				fmt::format(
					"Would you like to {} these changes?",
					Saylink::Silent(
						fmt::format(
							"#scale {} {} apply",
							sep->arg[1],
							sep->arg[2]
						),
						"apply"
					)
				).c_str()
			);
		}
	}
}
