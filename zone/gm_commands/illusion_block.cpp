#include "zone/client.h"
#include "zone/dialogue_window.h"

void command_illusion_block(Client* c, const Seperator* sep)
{
	int arguments = sep->argnum;
	if (!arguments || !strcasecmp(sep->arg[1], "help")) {
		BotCommandHelpParams p;

		p.description = { "Toggles whether or not you will block the illusion effects of spells cast by players or bots." };
		p.notes = {
			"- This will still allow other portions of a spell to land on you.",
			"- Spells cast by NPCs will ignore this setting and land as normal."
		};
		p.example_format = { fmt::format("{} [value]", sep->arg[0]) };
		p.examples_one =
		{
			"To enable illusion block:",
			fmt::format(
				"{} 1",
				sep->arg[0]
			)
		};
		p.examples_two =
		{
			"To disable illusion block:",
			fmt::format(
				"{} 0",
				sep->arg[0]
			)
		};

		std::string popup_text = c->SendBotCommandHelpWindow(p);
		popup_text = DialogueWindow::Table(popup_text);

		c->SendPopupToClient(sep->arg[0], popup_text.c_str());

		return;
	}

	if (sep->IsNumber(1)) {
		int set_status = atoi(sep->arg[1]);
		if (EQ::ValueWithin(set_status, 0, 1)) {
			c->SetIllusionBlock(set_status);
			c->Message(Chat::Green, "Your Illusion Block has been %s.", (set_status ? "enabled" : "disabled"));
		}
		else {
			c->Message(Chat::Yellow, "You must enter 0 for disabled or 1 for enabled.");
			return;
		}
	}
	else if (!strcasecmp(sep->arg[1], "current")) {
		c->Message(Chat::Green, "You're currently %s illusions.", (c->GetIllusionBlock() ? "blocking" : "allowing"));
	}
	else {
		c->Message(Chat::Yellow , "Incorrect argument, use %s help for a list of options.", sep->arg[0]);
	}
}
