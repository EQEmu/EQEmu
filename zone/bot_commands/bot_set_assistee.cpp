#include "../bot_command.h"

void bot_command_set_assistee(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_set_assistee", sep->arg[0], "setassistee")) {
		c->Message(Chat::White, "note: Sets your bots to assist your target in addition to yourself.");

		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		BotCommandHelpParams p;

		p.description = { "Sets your bots to assist your target in addition to yourself." };
		p.notes =
		{
			"- Your target must be another player in your group or raid.",
			"- This needs to be set on every zone/camp you do.",
			"- If a Raid or Group assist is set and you do not want your bots to auto assist that person, set yourself as the assistee."
		};

		std::string popup_text = c->SendBotCommandHelpWindow(p);
		popup_text = DialogueWindow::Table(popup_text);

		c->SendPopupToClient(sep->arg[0], popup_text.c_str());

		return;
	}

	Mob* assistee = c->GetTarget();

	if (assistee && assistee->IsClient() && c->IsInGroupOrRaid(assistee)) {
		c->SetAssistee(assistee->CastToClient()->CharacterID());
		c->Message(Chat::Green, "Your bots will now assist %s.", assistee->GetCleanName());
		
		return;
	}

	c->Message(Chat::Yellow, "You can only set your bots to assist clients that are in your group or raid.");

	return;
}
