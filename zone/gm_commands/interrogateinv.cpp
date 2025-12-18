#include "zone/client.h"

void command_interrogateinv(Client *c, const Seperator *sep)
{
	// 'command_interrogateinv' is an in-memory inventory interrogation tool only.
	//
	// it does not verify against actual database entries..but, the output can be
	// used to verify that something has been corrupted in a player's inventory.
	// any error condition should be assumed that the item in question will be
	// lost when the player logs out or zones (or incurrs any action that will
	// consume the Client-Inventory object instance in question.)
	//
	// any item instances located at a greater depth than a reported error should
	// be treated as an error themselves regardless of whether they report as the
	// same or not.

	if (strcasecmp(sep->arg[1], "help") == 0) {
		c->Message(Chat::White, "Usage: #interrogateinv [log] [silent]");
		c->Message(
			Chat::White,
			"  Displays your or your Player target inventory's current in-memory nested storage references"
		);
		c->Message(Chat::White, "  [log] - Logs interrogation to file");
		c->Message(Chat::White, "  [silent] - Omits the in-game message portion of the interrogation");
	}

	Client                                    *target   = nullptr;
	std::map<int16, const EQ::ItemInstance *> instmap;
	bool                                      log       = false;
	bool                                      silent    = false;
	bool                                      error     = false;
	bool                                      allowtrip = false;

	if (c->GetTarget() == nullptr) {
		target = c;
	} else if (c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	} else {
		c->Message(Chat::Default, "Use of this command is limited to Client entities");
		return;
	}

	if (strcasecmp(sep->arg[1], "log") == 0) {
		log = true;
	}
	if (strcasecmp(sep->arg[2], "silent") == 0) {
		silent = true;
	}

	bool success = target->InterrogateInventory(c, log, silent, allowtrip, error);

	if (!success) {
		c->Message(Chat::Red, "An unknown error occurred while processing Client::InterrogateInventory()");
	}
}

