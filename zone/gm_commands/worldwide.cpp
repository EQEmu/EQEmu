#include "zone/client.h"

void command_worldwide(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #worldwide cast [Spell ID] [Disable Message] - Cast a spell worldwide, use 1 for Disable Message to not send a global message");
		c->Message(Chat::White, "Usage: #worldwide message [Message] - Send a message worldwide");
		c->Message(Chat::White, "Usage: #worldwide move [Zone ID|Zone Short Name] - Move all players worldwide to specified zone");
		c->Message(Chat::White, "Usage: #worldwide moveinstance [Instance ID] - Move all players worldwide to specified instance");
		c->Message(Chat::White, "Usage: #worldwide remove [Spell ID] - Remove a spell worldwide");
		return;
	}

	bool is_cast = !strcasecmp(sep->arg[1], "cast");
	bool is_remove = !strcasecmp(sep->arg[1], "remove");
	bool is_message = !strcasecmp(sep->arg[1], "message");
	bool is_move = !strcasecmp(sep->arg[1], "move");
	bool is_moveinstance = !strcasecmp(sep->arg[1], "moveinstance");

	if (
		!is_cast &&
		!is_message &&
		!is_move &&
		!is_moveinstance &&
		!is_remove
	) {
		c->Message(Chat::White, "Usage: #worldwide cast [Spell ID] [Disable Message] - Cast a spell worldwide, use 1 for Disable Message to not send a global message");
		c->Message(Chat::White, "Usage: #worldwide message [Message] - Send a message worldwide");
		c->Message(Chat::White, "Usage: #worldwide move [Zone ID|Zone Short Name] - Move all players worldwide to specified zone");
		c->Message(Chat::White, "Usage: #worldwide moveinstance [Instance ID] - Move all players worldwide to specified instance");
		c->Message(Chat::White, "Usage: #worldwide remove [Spell ID] - Remove a spell worldwide");
		return;
	}

	if (is_cast) {
		if (sep->IsNumber(2)) {
			uint8 update_type = WWSpellUpdateType_Cast;
			auto spell_id = Strings::ToUnsignedInt(sep->arg[2]);
			bool disable_message = false;
			if (sep->IsNumber(3)) {
				disable_message = Strings::ToInt(sep->arg[3]) ? true : false;
			}

			if (!IsValidSpell(spell_id)) {
				c->Message(
					Chat::White,
					fmt::format(
						"Spell ID {} could not be found.",
						spell_id
					).c_str()
				);
				return;
			}

			c->Message(
				Chat::White,
				fmt::format(
					"World Wide Cast Spell | Spell: {} ({})",
					GetSpellName(spell_id),
					spell_id
				).c_str()
			);

			quest_manager.WorldWideSpell(update_type, spell_id);

			if (!disable_message) {
				quest_manager.WorldWideMessage(
					Chat::Yellow,
					fmt::format(
						"[SYSTEM] A GM has cast [{}] world-wide!",
						GetSpellName(spell_id)
					).c_str()
				);
			}
		} else {
			c->Message(Chat::White, "Usage: #worldwide cast [Spell ID] [Disable Message] - Cast a spell worldwide, use 1 for Disable Message to not send a global message");
		}
	} else if (is_message) {
		std::string message = sep->argplus[2];
		if (!message.empty()) {
			quest_manager.WorldWideMessage(
				Chat::White,
				fmt::format(
					"{}",
					message
				).c_str()
			);
		} else {
			c->Message(Chat::White, "Usage: #worldwide message [Message] - Send a message worldwide");
		}
	} else if (is_move) {
		uint8 update_type = WWMoveUpdateType_MoveZone;
		auto zone_id = (
			sep->IsNumber(2) ?
			static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[2])) :
			static_cast<uint16>(ZoneID(sep->arg[2]))
		);
		auto zone_short_name = ZoneName(zone_id);

		if (!zone_id || !zone_short_name) {
			c->Message(
				Chat::White,
				fmt::format(
					"Zone ID {} could not be found.",
					zone_id
				).c_str()
			);
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"World Wide Zone | Zone: {} ({}) ID: {}",
				ZoneLongName(
					zone_id
				),
				zone_short_name,
				zone_id
			).c_str()
		);

		quest_manager.WorldWideMove(update_type, zone_short_name);
	}
	else if (is_moveinstance) {
		if (sep->IsNumber(2)) {
			uint8 update_type = WWMoveUpdateType_MoveZoneInstance;
			const char *zone_short_name = "";
			auto instance_id = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[2]));

			c->Message(
				Chat::White,
				fmt::format(
					"World Wide Zone Instance | Instance ID: {}",
					instance_id
				).c_str()
			);

			quest_manager.WorldWideMove(update_type, zone_short_name, instance_id);
		} else {
			c->Message(Chat::White, "Usage: #worldwide moveinstance [Instance ID] - Move all players worldwide to specified instance");
		}
	} else if (is_remove) {
		if (sep->IsNumber(2)) {
			uint8 update_type = WWSpellUpdateType_Remove;
			auto spell_id = Strings::ToUnsignedInt(sep->arg[2]);

			if (!IsValidSpell(spell_id)) {
				c->Message(
					Chat::White,
					fmt::format(
						"Spell ID {} could not be found.",
						spell_id
					).c_str()
				);
				return;
			}

			c->Message(
				Chat::White,
				fmt::format(
					"World Wide Remove Spell | Spell: {} ({})",
					GetSpellName(spell_id),
					spell_id
				).c_str()
			);

			quest_manager.WorldWideSpell(update_type, spell_id);
		} else {
			c->Message(Chat::White, "Usage: #worldwide remove [Spell ID] - Remove a spell worldwide");
		}
	}
}

