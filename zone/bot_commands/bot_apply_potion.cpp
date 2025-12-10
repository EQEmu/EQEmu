#include "../bot_command.h"

void bot_command_apply_potion(Client* c, const Seperator* sep)
{
	if (helper_command_disabled(c, RuleB(Bots, AllowApplyPotionCommand), "applypotion")) {
		return;
	}
	if (helper_command_alias_fail(c, "bot_command_apply_potion", sep->arg[0], "applypotion")) {
		return;
	}
	if (helper_is_help_or_usage(sep->arg[1])) {

		c->Message(Chat::White, "usage: <bot_target> %s", sep->arg[0]);
		return;
	}

	Bot* my_bot = nullptr;
	if (c->GetTarget() && c->GetTarget()->IsBot() && c->GetTarget()->CastToBot()->GetBotOwnerCharacterID() == c->CharacterID()) {
		my_bot = c->GetTarget()->CastToBot();
	}
	if (!my_bot) {

		c->Message(Chat::White, "You must target a bot that you own to use this command!");
		return;
	}

	const auto potion_instance = c->GetInv().GetItem(EQ::invslot::slotCursor);
	if (!potion_instance) {

		c->Message(Chat::White, "No item found on cursor!");
		return;
	}

	auto potion_data = potion_instance->GetItem();
	if (!potion_data) {

		c->Message(Chat::White, "No data found for cursor item!");
		return;
	}

	if (potion_data->ItemType == EQ::item::ItemTypePotion && potion_data->Click.Effect > 0) {

		if (RuleB(Bots, RestrictApplyPotionToRogue) && potion_data->Classes != player_class_bitmasks[Class::Rogue]) {

			c->Message(Chat::White, "This command is restricted to rogue poison potions only!");
			return;
		}
		if ((~potion_data->Races) & GetPlayerRaceBit(my_bot->GetRace())) {

			c->Message(Chat::White, "Invalid race for potion!");
			return;
		}
		if ((~potion_data->Classes) & GetPlayerClassBit(my_bot->GetClass())) {

			c->Message(Chat::White, "Invalid class for potion!");
			return;
		}

		if (potion_data->Click.Level2 > my_bot->GetLevel()) {

			c->Message(Chat::White, "This potion is too powerful for your intended target!");
			return;
		}

		// TODO: figure out best way to handle casting time/animation
		if (my_bot->SpellFinished(potion_data->Click.Effect, my_bot, EQ::spells::CastingSlot::Item, 0)) {
			c->Message(Chat::White, "Successfully applied %s to %s's buff effects.", potion_data->Name, my_bot->GetCleanName());
		}
		else {
			c->Message(Chat::White, "Failed to apply %s to %s's buff effects.", potion_data->Name, my_bot->GetCleanName());
		}

		c->DeleteItemInInventory(EQ::invslot::slotCursor, 1, true);
	}
	else {

		c->Message(Chat::White, "Item on cursor is not a potion!");
		return;
	}
}
