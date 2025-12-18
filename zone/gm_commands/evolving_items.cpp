#include "common/evolving_items.h"
#include "zone/client.h"
#include "zone/command.h"

void command_evolvingitems(Client *c, const Seperator *sep)
{
	const uint16 arguments = sep->argnum;
	if (!arguments) {
		SendEvolvingItemsSubCommands(c);
		return;
	}

	Client* t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const bool is_item   = !strcasecmp(sep->arg[1], "item");
	const bool is_target = !strcasecmp(sep->arg[1], "target");

	if (!is_item && !is_target) {
		SendEvolvingItemsSubCommands(c);
		return;
	}

	if (is_target) {
		if (arguments > 1) {
			c->Message(Chat::White, "Usage: #evolve target");
		} else {
			c->Message(Chat::Red, "Worn Items");
			for (auto const &[key, value]: t->GetInv().GetWorn()) {
				if (!value->IsEvolving()) {
					continue;
				}

				auto item = EvolvingItemsManager::Instance()->GetEvolvingItemsCache().at(value->GetID());
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Evolving Items | {:0d}",
						value->GetID()
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Unique ID | {:0d}",
						value->GetEvolveUniqueID()
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Final Item ID | {:0d}",
						value->GetEvolveFinalItemID()
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Activated | {}",
						value->GetEvolveActivated() ? "Yes" : "No"
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Equipped | {}",
						value->GetEvolveEquipped() ? "Yes" : "No"
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Current Amount | {:0d}",
						value->GetEvolveCurrentAmount()
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Required Amount | {:0d}",
						item.required_amount
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Progression (%) | {:.2f}",
						value->GetEvolveProgression()
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Type | {}",
						item.type
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Subtype | {}",
						item.sub_type
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Timer | {}",
						value->GetTimers().at("evolve").Enabled()
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Timer Remaining | {}",
						value->GetTimers().at("evolve").GetRemainingTime()
					).c_str()
				);
			}

			c->Message(Chat::Red, "Personal Items");
			for (auto const &[key, value]: t->GetInv().GetPersonal()) {
				if (!value->IsEvolving()) {
					continue;
				}

				auto item = EvolvingItemsManager::Instance()->GetEvolvingItemsCache().at(value->GetID());
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Evolving Items | {:0d}",
						value->GetID()
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Unique ID | {:0d}",
						value->GetEvolveUniqueID()
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Final Item ID | {:0d}",
						value->GetEvolveFinalItemID()
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Activated | {}",
						value->GetEvolveActivated() ? "Yes" : "No"
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Equipped | {}",
						value->GetEvolveEquipped() ? "Yes" : "No"
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Current Amount | {:0d}",
						value->GetEvolveCurrentAmount()
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Required Amount | {:0d}",
						item.required_amount
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Progression (%) | {:.2f}",
						value->GetEvolveProgression()
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Type | {}",
						item.type
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Subtype | {}",
						item.sub_type
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Timer | {}",
						value->GetTimers().at("evolve").Enabled()
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Timer Remaining | {}",
						value->GetTimers().at("evolve").GetRemainingTime()
					).c_str()
				);
			}
		}
	} else if (is_item) {
		if (arguments > 2) {
			c->Message(Chat::White, "Usage: #evolve item item_id");
		} else if (sep->IsNumber(2)) {
			auto item_id = Strings::ToUnsignedInt(sep->arg[2]);
			auto item    = c->GetInv().GetItem(c->GetInv().HasItem(item_id));
			if (item) {
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Evolving Items | {:0d}",
						item->GetID()
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Slot | {:0d}",
						c->GetInv().HasItem(item_id)
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Activated | {}",
						item->GetEvolveActivated() ? "Yes" : "No"
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Equipped | {}",
						item->GetEvolveEquipped() ? "Yes" : "No"
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Unique ID | {:0d}",
						item->GetEvolveUniqueID()
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Progression (%) | {:.2f}",
						item->GetEvolveProgression()
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Timer | {}",
						item->GetTimers().at("evolve").Enabled()
					).c_str()
				);
				c->Message(
					Chat::Yellow,
					fmt::format(
						"Timer Remaining | {}",
						item->GetTimers().at("evolve").GetRemainingTime()
					).c_str()
				);
			} else {
				c->Message(
					Chat::Red,
					fmt::format(
						"Item {} could not be found in your inventory.",
						item_id
					).c_str()
				);
			}
		} else {
			SendEvolvingItemsSubCommands(c);
		}
	}
}

void SendEvolvingItemsSubCommands(Client *c)
{
	c->Message(Chat::White, "#evolve item item_id (Shows evolve values within the iteminstance)");
	c->Message(
		Chat::White,
		"#evolve target (Shows evolve values within the target's cache - Must have a player target selected.)"
	);
}
