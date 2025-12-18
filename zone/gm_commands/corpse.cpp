#include "zone/client.h"
#include "zone/corpse.h"

void command_corpse(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #corpse delete - Delete targeted corpse");
		c->Message(Chat::White, "Usage: #corpse deletenpccorpses - Deletes all NPC corpses");
		c->Message(Chat::White, "Usage: #corpse inspectloot - Inspects the loot on a corpse");
		c->Message(Chat::White, "Usage: #corpse listnpc - Lists all NPC corpses");
		c->Message(
			Chat::White,
			"Usage: #corpse lock - Locks the corpse, only GMs can loot the corpse when it is locked"
		);
		c->Message(Chat::White, "Usage: #corpse removecash - Removes the cash from a corpse");
		c->Message(Chat::White, "Usage: #corpse unlock - Unlocks the corpses, allowing non-GMs to loot the corpse");
		c->Message(Chat::White, "Usage: #corpse charid [Character ID] - Change player corpse's owner");
		c->Message(Chat::White, "Usage: #corpse deleteplayercorpses - Deletes all player corpses");
		c->Message(Chat::White, "Usage: #corpse depop [Bury] - Depops single target corpse.");
		c->Message(Chat::White, "Usage: #corpse depopall [Bury] - Depops all target player's corpses.");
		c->Message(Chat::White, "Usage: #corpse listplayer - Lists all player corpses");
		c->Message(
			Chat::White,
			"Usage: #corpse moveallgraveyard - Moves all player corpses to the current zone's graveyard or non-instance"
		);
		c->Message(Chat::White, "Note: Set bury to 0 to skip burying the corpses.");
		return;
	}

	Mob *target = c->GetTarget();
	bool is_character_id = !strcasecmp(sep->arg[1], "charid");
	bool is_delete = !strcasecmp(sep->arg[1], "delete");
	bool is_delete_npc_corpses = !strcasecmp(sep->arg[1], "deletenpccorpses");
	bool is_delete_player_corpses = !strcasecmp(sep->arg[1], "deleteplayercorpses");
	bool is_depop = !strcasecmp(sep->arg[1], "depop");
	bool is_depop_all = !strcasecmp(sep->arg[1], "depopall");
	bool is_inspect_loot = !strcasecmp(sep->arg[1], "inspectloot");
	bool is_list_npc = !strcasecmp(sep->arg[1], "listnpc");
	bool is_list_player = !strcasecmp(sep->arg[1], "listplayer");
	bool is_lock = !strcasecmp(sep->arg[1], "lock");
	bool is_move_all_to_graveyard = !strcasecmp(sep->arg[1], "moveallgraveyard");
	bool is_remove_cash = !strcasecmp(sep->arg[1], "removecash");
	bool is_reset_looter = !strcasecmp(sep->arg[1], "resetlooter");
	bool is_unlock = !strcasecmp(sep->arg[1], "unlock");
	if (
		!is_character_id &&
		!is_delete &&
		!is_delete_npc_corpses &&
		!is_delete_player_corpses &&
		!is_depop &&
		!is_depop_all &&
		!is_inspect_loot &&
		!is_list_npc &&
		!is_list_player &&
		!is_lock &&
		!is_move_all_to_graveyard &&
		!is_remove_cash &&
		!is_reset_looter &&
		!is_unlock
	) {
		c->Message(Chat::White, "Usage: #corpse delete - Delete targeted corpse");
		c->Message(Chat::White, "Usage: #corpse deletenpccorpses - Deletes all NPC corpses");
		c->Message(Chat::White, "Usage: #corpse inspectloot - Inspects the loot on a corpse");
		c->Message(Chat::White, "Usage: #corpse listnpc - Lists all NPC corpses");
		c->Message(
			Chat::White,
			"Usage: #corpse lock - Locks the corpse, only GMs can loot the corpse when it is locked"
		);
		c->Message(Chat::White, "Usage: #corpse removecash - Removes the cash from a corpse");
		c->Message(Chat::White, "Usage: #corpse unlock - Unlocks the corpses, allowing non-GMs to loot the corpse");
		c->Message(Chat::White, "Usage: #corpse charid [Character ID] - Change player corpse's owner");
		c->Message(Chat::White, "Usage: #corpse deleteplayercorpses - Deletes all player corpses");
		c->Message(Chat::White, "Usage: #corpse depop [Bury] - Depops single target corpse.");
		c->Message(Chat::White, "Usage: #corpse depopall [Bury] - Depops all target player's corpses.");
		c->Message(Chat::White, "Usage: #corpse listplayer - Lists all player corpses");
		c->Message(
			Chat::White,
			"Usage: #corpse moveallgraveyard - Moves all player corpses to the current zone's graveyard or non-instance"
		);
		c->Message(Chat::White, "Note: Set bury to 0 to skip burying the corpses.");
		return;
	}


	if (is_delete_player_corpses) {
		auto corpses_deleted = entity_list.DeletePlayerCorpses();
		auto deleted_string  = (
			corpses_deleted ?
			fmt::format(
				"{} Player corpse{} deleted.",
				corpses_deleted,
				corpses_deleted != 1 ? "s" : ""
			) :
			"There are no player corpses to delete."
		);
		c->Message(Chat::White, deleted_string.c_str());
	} else if (is_delete) {
		if (!target || !target->IsCorpse()) {
			c->Message(Chat::White, "You must target a corpse to use this command.");
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Deleting {} corpse {}.",
				target->IsNPCCorpse() ? "NPC" : "player",
				c->GetTargetDescription(target)
			).c_str()
		);

		target->CastToCorpse()->Delete();
	} else if (is_list_npc) {
		entity_list.ListNPCCorpses(c);
	} else if (is_list_player) {
		entity_list.ListPlayerCorpses(c);
	} else if (is_delete_npc_corpses) {
		auto corpses_deleted = entity_list.DeleteNPCCorpses();
		auto deleted_string = (
			corpses_deleted ?
			fmt::format(
				"{} NPC corpse{} deleted.",
				corpses_deleted,
				corpses_deleted != 1 ? "s" : ""
			) :
			"There are no NPC corpses to delete."
		);
		c->Message(Chat::White, deleted_string.c_str());
	} else if (is_character_id) {
		if (!target || !target->IsPlayerCorpse()) {
			c->Message(Chat::White, "You must target a player corpse to use this command.");
			return;
		}

		if (!sep->IsNumber(2)) {
			c->Message(Chat::White, "Usage: #corpse charid [Character ID] - Change player corpse's owner");
			return;
		}

		const uint32 character_id = Strings::ToUnsignedInt(sep->arg[2]);
		c->Message(
			Chat::White,
			fmt::format(
				"Setting the owner to {} ({}) for the player corpse {}.",
				database.GetCharNameByID(character_id),
				target->CastToCorpse()->SetCharID(character_id),
				c->GetTargetDescription(target)
			).c_str()
		);
	} else if (is_reset_looter) {
		if (!target || !target->IsCorpse()) {
			c->Message(Chat::White, "You must target a corpse to use this command.");
			return;
		}

		target->CastToCorpse()->ResetLooter();
		c->Message(
			Chat::White,
			fmt::format(
				"Reset looter for {} corpse {}.",
				target->IsNPCCorpse() ? "NPC" : "player",
				c->GetTargetDescription(target)
			).c_str()
		);
	} else if (is_remove_cash) {
		if (!target || !target->IsCorpse()) {
			c->Message(Chat::White, "You must target a corpse to use this command.");
			return;
		}

		target->CastToCorpse()->RemoveCash();

		c->Message(
			Chat::White,
			fmt::format(
				"Removed cash from {} corpse {}.",
				target->IsNPCCorpse() ? "NPC" : "player",
				c->GetTargetDescription(target)
			).c_str()
		);
	} else if (is_inspect_loot) {
		if (!target || !target->IsCorpse()) {
			c->Message(Chat::White, "You must target a corpse to use this command.");
			return;
		}

		target->CastToCorpse()->QueryLoot(c);
	} else if (is_lock) {
		if (!target || !target->IsCorpse()) {
			c->Message(Chat::White, "You must target a corpse to use this command.");
			return;
		}

		target->CastToCorpse()->Lock();
		c->Message(
			Chat::White,
			fmt::format(
				"Locking {} corpse {}.",
				target->IsNPCCorpse() ? "NPC" : "player",
				c->GetTargetDescription(target)
			).c_str()
		);
	} else if (is_unlock) {
		if (!target || !target->IsCorpse()) {
			c->Message(Chat::White, "You must target a corpse to use this command.");
			return;
		}

		target->CastToCorpse()->UnLock();
		c->Message(
			Chat::White,
			fmt::format(
				"Unlocking {} corpse {}.",
				target->IsNPCCorpse() ? "NPC" : "player",
				c->GetTargetDescription(target)
			).c_str()
		);
	} else if (is_depop) {
		if (!target || !target->IsPlayerCorpse()) {
			c->Message(Chat::White, "You must target a player corpse to use this command.");
			return;
		}

		const bool bury_corpse = sep->IsNumber(2) ? Strings::ToBool(sep->arg[2]) : false;

		c->Message(
			Chat::White,
			fmt::format(
				"Depopping player corpse {}.",
				c->GetTargetDescription(target)
			).c_str()
		);

		target->CastToCorpse()->DepopPlayerCorpse();

		if (bury_corpse) {
			target->CastToCorpse()->Bury();
		}
	} else if (is_depop_all) {
		if (!target || !target->IsClient()) {
			c->Message(Chat::White, "You must target a player to use this command.");
			return;
		}

		const bool bury_corpse = sep->IsNumber(2) ? Strings::ToBool(sep->arg[2]) : false;

		c->Message(
			Chat::White,
			fmt::format(
				"Depopping all player corpses for {}.",
				c->GetTargetDescription(target)
			).c_str()
		);

		target->CastToClient()->DepopAllCorpses();

		if (bury_corpse) {
			target->CastToClient()->BuryPlayerCorpses();
		}
	} else if (is_move_all_to_graveyard) {
		const int moved_count = entity_list.MovePlayerCorpsesToGraveyard(true);
		if (moved_count) {
			c->Message(
				Chat::White,
				fmt::format(
					"Moved {} player corpse{} to graveyard in {} ({}).",
					moved_count,
					moved_count != 1 ? "s" : "",
					ZoneLongName(zone->GetZoneID()),
					ZoneName(zone->GetZoneID())
				).c_str()
			);
		} else {
			c->Message(Chat::White, "There are no player corpses to move to the graveyard.");
		}
	}
}

