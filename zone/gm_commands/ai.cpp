#include "zone/client.h"

void command_ai(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #ai consider [Mob Name] - Show how an NPC considers to a mob");
		c->Message(Chat::White, "Usage: #ai faction [Faction ID] - Set an NPC's Faction ID");
		c->Message(Chat::White, "Usage: #ai guard - Save an NPC's guard spot to their current location");
		c->Message(Chat::White, "Usage: #ai roambox [Distance] [Min X] [Max X] [Min Y] [Max Y] [Delay] [Minimum Delay] - Set an NPC's roambox using X and Y coordinates");
		c->Message(Chat::White, "Usage: #ai roambox [Distance] [Roam Distance] [Delay] [Minimum Delay] - Set an NPC's roambox using roam distance");
		c->Message(Chat::White, "Usage: #ai spells [Spell List ID] - Set an NPC's Spell List ID");
		return;
	}

	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	auto target = c->GetTarget()->CastToNPC();

	bool is_consider = !strcasecmp(sep->arg[1], "consider");
	bool is_faction = !strcasecmp(sep->arg[1], "faction");
	bool is_guard = !strcasecmp(sep->arg[1], "guard");
	bool is_roambox = !strcasecmp(sep->arg[1], "roambox");
	bool is_spells = !strcasecmp(sep->arg[1], "spells");

	if (
		!is_consider &&
		!is_faction &&
		!is_guard &&
		!is_roambox &&
		!is_spells
	) {
		c->Message(Chat::White, "Usage: #ai consider [Mob Name] - Show how an NPC considers to a mob");
		c->Message(Chat::White, "Usage: #ai faction [Faction ID] - Set an NPC's Faction ID");
		c->Message(Chat::White, "Usage: #ai guard - Save an NPC's guard spot to their current location");
		c->Message(Chat::White, "Usage: #ai roambox [Distance] [Min X] [Max X] [Min Y] [Max Y] [Delay] [Minimum Delay] - Set an NPC's roambox using X and Y coordinates");
		c->Message(Chat::White, "Usage: #ai roambox [Distance] [Roam Distance] [Delay] [Minimum Delay] - Set an NPC's roambox using roam distance");
		c->Message(Chat::White, "Usage: #ai spells [Spell List ID] - Set an NPC's Spell List ID");
		return;
	}

	if (is_consider) {
		if (arguments == 2) {
			auto mob_name = sep->arg[2];
			auto mob_to_consider = entity_list.GetMob(mob_name);
			if (mob_to_consider) {
				auto consider_level = static_cast<uint8>(mob_to_consider->GetReverseFactionCon(target));
				c->Message(
					Chat::White,
					fmt::format(
						"{} considers {} as {} ({}).",
						c->GetTargetDescription(target),
						c->GetTargetDescription(mob_to_consider),
						EQ::constants::GetConsiderLevelName(consider_level),
						consider_level
					).c_str()
				);
			}
		} else {
			c->Message(Chat::White, "Usage: #ai consider [Mob Name] - Show how an NPC considers a mob");
		}
	} else if (is_faction) {
		if (sep->IsNumber(2)) {
			auto faction_id = Strings::ToInt(sep->arg[2]);
			auto faction_name = content_db.GetFactionName(faction_id);
			target->SetNPCFactionID(faction_id);
			c->Message(
				Chat::White,
				fmt::format(
					"{} is now on Faction {}.",
					c->GetTargetDescription(target),
					(
						faction_name.empty() ?
						std::to_string(faction_id) :
						fmt::format(
							"{} ({})",
							faction_name,
							faction_id
						)
					)
				).c_str()
			);
		} else {
			c->Message(Chat::White, "Usage: #ai faction [Faction ID] - Set an NPC's Faction ID");
		}
	} else if (is_guard) {
		auto target_position = target->GetPosition();

		target->SaveGuardSpot(target_position);

		c->Message(
			Chat::White,
			fmt::format(
				"{} now has a guard spot of {:.2f}, {:.2f}, {:.2f} with a heading of {:.2f}.",
				c->GetTargetDescription(target),
				target_position.x,
				target_position.y,
				target_position.z,
				target_position.w
			).c_str()
		);
	} else if (is_roambox) {
		if (target->IsAIControlled()) {
			if (
				arguments >= 6 &&
				arguments <= 8 &&
				sep->IsNumber(2) &&
				sep->IsNumber(3) &&
				sep->IsNumber(4) &&
				sep->IsNumber(5) &&
				sep->IsNumber(6)
			) {
				auto distance = Strings::ToFloat(sep->arg[2]);
				auto min_x = Strings::ToFloat(sep->arg[3]);
				auto max_x = Strings::ToFloat(sep->arg[4]);
				auto min_y = Strings::ToFloat(sep->arg[5]);
				auto max_y = Strings::ToFloat(sep->arg[6]);

				uint32 delay = 2500;
				uint32 minimum_delay = 2500;

				if (sep->IsNumber(7)) {
					delay = Strings::ToUnsignedInt(sep->arg[7]);
				}

				if (sep->IsNumber(8)) {
					minimum_delay = Strings::ToUnsignedInt(sep->arg[8]);
				}

				target->CastToNPC()->AI_SetRoambox(
					distance,
					max_x,
					min_x,
					max_y,
					min_y,
					delay,
					minimum_delay
				);

				c->Message(
					Chat::White,
					fmt::format(
						"{} now has a roambox from {}, {} to {}, {} with {} and {} and a distance of {}.",
						c->GetTargetDescription(target),
						min_x,
						min_y,
						max_x,
						max_y,
						(
							delay ?
							fmt::format(
								"a delay of {} ({})",
								Strings::MillisecondsToTime(delay),
								delay
							):
							"no delay"
						),
						(
							minimum_delay ?
							fmt::format(
								"a minimum delay of {} ({})",
								Strings::MillisecondsToTime(minimum_delay),
								minimum_delay
							):
							"no minimum delay"
						),
						distance
					).c_str()
				);
			} else if (
				arguments >= 3 &&
				arguments <= 4 &&
				sep->IsNumber(2) &&
				sep->IsNumber(3)
			) {
				auto max_distance = Strings::ToFloat(sep->arg[2]);
				auto roam_distance_variance = Strings::ToFloat(sep->arg[3]);

				uint32 delay = 2500;
				uint32 minimum_delay = 2500;

				if (sep->IsNumber(4)) {
					delay = Strings::ToUnsignedInt(sep->arg[4]);
				}

				if (sep->IsNumber(5)) {
					minimum_delay = Strings::ToUnsignedInt(sep->arg[5]);
				}

				target->CastToNPC()->AI_SetRoambox(
					max_distance,
					roam_distance_variance,
					delay,
					minimum_delay
				);

				c->Message(
					Chat::White,
					fmt::format(
						"{} now has a roambox with a max distance of {} and a roam distance variance of {} with {} and {}.",
						c->GetTargetDescription(target),
						max_distance,
						roam_distance_variance,
						(
							delay ?
							fmt::format(
								"a delay of {} ({})",
								delay,
								Strings::MillisecondsToTime(delay)
							):
							"no delay"
						),
						(
							minimum_delay ?
							fmt::format(
								"a minimum delay of {} ({})",
								minimum_delay,
								Strings::MillisecondsToTime(delay)
							):
							"no minimum delay"
						)
					).c_str()
				);
			} else {
				c->Message(Chat::White, "Usage: #ai roambox [Distance] [Min X] [Max X] [Min Y] [Max Y] [Delay] [Minimum Delay] - Set an NPC's roambox using X and Y coordinates");
				c->Message(Chat::White, "Usage: #ai roambox [Distance] [Roam Distance] [Delay] [Minimum Delay] - Set an NPC's roambox using roam distance");
			}
		} else {
			c->Message(Chat::White, "You must target an NPC with AI.");
		}
	} else if (is_spells) {
		if (sep->IsNumber(2)) {
			auto spell_list_id = Strings::ToUnsignedInt(sep->arg[2]);
			if (spell_list_id >= 0) {
				target->CastToNPC()->AI_AddNPCSpells(spell_list_id);

				c->Message(
					Chat::White,
					fmt::format(
						"{} is now using Spell List {}.",
						c->GetTargetDescription(target),
						spell_list_id
					).c_str()
				);
			} else {
				c->Message(Chat::White, "Spell List ID must be greater than or equal to 0.");
			}
		} else {
			c->Message(Chat::White, "Usage: #ai spells [Spell List ID] - Set an NPC's Spell List ID");
		}
	}
}

