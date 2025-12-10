#include "../bot_command.h"

void bot_command_owner_option(Client *c, const Seperator *sep)
{
	if (helper_is_help_or_usage(sep->arg[1])) {

		c->Message(Chat::White, "usage: %s [option] [argument]", sep->arg[0]);

		std::string window_title = "Bot Owner Options";
		std::string window_text =
						"<table>"
						"<tr>"
						"<td><c \"#FFFFFF\">Option<br>------</td>"
						"<td><c \"#00FF00\">Argument<br>-------</td>"
						"<td><c \"#AAAAAA\">Notes<br>-----</td>"
						"</tr>"
						"<tr>"
						"<td><c \"#CCCCCC\">deathmarquee</td>"
						"<td><c \"#00CC00\">enable <c \"#CCCCCC\">| <c \"#00CC00\">disable</td>"
						"<td><c \"#888888\">marquee message on death</td>"
						"</tr>"
						"<tr>"
						"<td></td>"
						"<td><c \"#00CCCC\">null</td>"
						"<td><c \"#888888\">(toggles)</td>"
						"</tr>"
						"<tr>"
						"<td><c \"#CCCCCC\">statsupdate</td>"
						"<td><c \"#00CC00\">enable <c \"#CCCCCC\">| <c \"#00CC00\">disable</td>"
						"<td><c \"#888888\">report stats on update</td>"
						"</tr>"
						"<tr>"
						"<td></td>"
						"<td><c \"#00CCCC\">null</td>"
						"<td><c \"#888888\">(toggles)</td>"
						"</tr>"
						"<tr>"
						"<td><c \"#CCCCCC\">spawnmessage</td>"
						"<td><c \"#00CC00\">say <c \"#CCCCCC\">| <c \"#00CC00\">tell <c \"#CCCCCC\">| <c \"#00CC00\">silent</td>"
						"<td><c \"#888888\">spawn message into channel</td>"
						"</tr>"
						"<tr>"
						"<td></td>"
						"<td><c \"#00CC00\">class <c \"#CCCCCC\">| <c \"#00CC00\">default</td>"
						"<td><c \"#888888\">spawn with class-based message</td>"
						"</tr>"
						"<tr>"
						"<td></td>"
						"<td><c \"#00CCCC\">null</td>"
						"<td><c \"#888888\">(toggles)</td>"
						"</tr>"
						"<tr>"
						"<td><c \"#CCCCCC\">autodefend</td>"
						"<td><c \"#00CC00\">enable <c \"#CCCCCC\">| <c \"#00CC00\">disable</td>"
						"<td><c \"#888888\">bots defend owner when aggroed</td>"
						"</tr>"
						"<tr>"
						"<td></td>"
						"<td><c \"#00CCCC\">null</td>"
						"<td><c \"#888888\">(toggles)</td>"
						"</tr>"
						"<tr>"
						"<td><c \"#CCCCCC\">buffcounter</td>"
						"<td><c \"#00CC00\">enable <c \"#CCCCCC\">| <c \"#00CC00\">disable</td>"
						"<td><c \"#888888\">marquee message on buff counter change</td>"
						"</tr>"
						"<tr>"
						"<td></td>"
						"<td><c \"#00CCCC\">null</td>"
						"<td><c \"#888888\">(toggles)</td>"
						"</tr>"
						"<tr>"
						"<td><c \"#CCCCCC\">monkwumessage</td>"
						"<td><c \"#00CC00\">enable <c \"#CCCCCC\">| <c \"#00CC00\">disable</td>"
						"<td><c \"#888888\">displays monk wu trigger messages</td>"
						"</tr>"
						"<tr>"
						"<td></td>"
						"<td><c \"#00CCCC\">null</td>"
						"<td><c \"#888888\">(toggles)</td>"
						"</tr>"
						"<tr>"
						"<td><c \"#CCCCCC\">current</td>"
						"<td></td>"
						"<td><c \"#888888\">show current settings</td>"
						"</tr>"
						"</table>";

		c->SendPopupToClient(window_title.c_str(), window_text.c_str());

		return;
	}

	std::string owner_option(sep->arg[1]);
	std::string argument(sep->arg[2]);

	if (!owner_option.compare("deathmarquee")) {

		if (!argument.compare("enable")) {
			c->SetBotOption(Client::booDeathMarquee, true);
		}
		else if (!argument.compare("disable")) {
			c->SetBotOption(Client::booDeathMarquee, false);
		}
		else {
			c->SetBotOption(Client::booDeathMarquee, !c->GetBotOption(Client::booDeathMarquee));
		}

		database.botdb.SaveOwnerOption(c->CharacterID(), Client::booDeathMarquee, c->GetBotOption(Client::booDeathMarquee));

		c->Message(Chat::White, "Bot 'death marquee' is now %s.", (c->GetBotOption(Client::booDeathMarquee) ? "enabled" : "disabled"));
	}
	else if (!owner_option.compare("statsupdate")) {

		if (!argument.compare("enable")) {
			c->SetBotOption(Client::booStatsUpdate, true);
		}
		else if (!argument.compare("disable")) {
			c->SetBotOption(Client::booStatsUpdate, false);
		}
		else {
			c->SetBotOption(Client::booStatsUpdate, !c->GetBotOption(Client::booStatsUpdate));
		}

		database.botdb.SaveOwnerOption(c->CharacterID(), Client::booStatsUpdate, c->GetBotOption(Client::booStatsUpdate));

		c->Message(Chat::White, "Bot 'stats update' is now %s.", (c->GetBotOption(Client::booStatsUpdate) ? "enabled" : "disabled"));
	}
	else if (!owner_option.compare("spawnmessage")) {

		Client::BotOwnerOption boo = Client::_booCount;

		if (!argument.compare("say")) {

			boo = Client::booSpawnMessageSay;
			c->SetBotOption(Client::booSpawnMessageSay, true);
			c->SetBotOption(Client::booSpawnMessageTell, false);
		}
		else if (!argument.compare("tell")) {

			boo = Client::booSpawnMessageSay;
			c->SetBotOption(Client::booSpawnMessageSay, false);
			c->SetBotOption(Client::booSpawnMessageTell, true);
		}
		else if (!argument.compare("silent")) {

			boo = Client::booSpawnMessageSay;
			c->SetBotOption(Client::booSpawnMessageSay, false);
			c->SetBotOption(Client::booSpawnMessageTell, false);
		}
		else if (!argument.compare("class")) {

			boo = Client::booSpawnMessageClassSpecific;
			c->SetBotOption(Client::booSpawnMessageClassSpecific, true);
		}
		else if (!argument.compare("default")) {

			boo = Client::booSpawnMessageClassSpecific;
			c->SetBotOption(Client::booSpawnMessageClassSpecific, false);
		}
		else {

			c->Message(Chat::White, "Owner option '%s' argument '%s' is not recognized.", owner_option.c_str(), argument.c_str());
			return;
		}

		if (boo == Client::booSpawnMessageSay) {

			database.botdb.SaveOwnerOption(
				c->CharacterID(),
				std::pair<size_t, size_t>(
					Client::booSpawnMessageSay,
					Client::booSpawnMessageTell
				),
				std::pair<bool, bool>(
					c->GetBotOption(Client::booSpawnMessageSay),
					c->GetBotOption(Client::booSpawnMessageTell)
				)
			);
		}
		else if (boo == Client::booSpawnMessageClassSpecific) {

			database.botdb.SaveOwnerOption(
				c->CharacterID(),
				Client::booSpawnMessageClassSpecific,
				c->GetBotOption(Client::booSpawnMessageClassSpecific)
			);
		}
		else {

			c->Message(Chat::White, "Bot 'spawn message' is now ERROR.");
			return;
		}

		c->Message(Chat::White, "Bot 'spawn message' is now %s.", argument.c_str());
	}
	else if (!owner_option.compare("autodefend")) {

		if (RuleB(Bots, AllowOwnerOptionAutoDefend)) {

			if (!argument.compare("enable")) {
				c->SetBotOption(Client::booAutoDefend, true);
			}
			else if (!argument.compare("disable")) {
				c->SetBotOption(Client::booAutoDefend, false);
			}
			else {
				c->SetBotOption(Client::booAutoDefend, !c->GetBotOption(Client::booAutoDefend));
			}

			database.botdb.SaveOwnerOption(c->CharacterID(), Client::booAutoDefend, c->GetBotOption(Client::booAutoDefend));

			c->Message(Chat::White, "Bot 'auto defend' is now %s.", (c->GetBotOption(Client::booAutoDefend) ? "enabled" : "disabled"));
		}
		else {
			c->Message(Chat::White, "Bot owner option 'autodefend' is not allowed on this server.");
		}
	}
	else if (!owner_option.compare("buffcounter")) {

		if (!argument.compare("enable")) {
			c->SetBotOption(Client::booBuffCounter, true);
		}
		else if (!argument.compare("disable")) {
			c->SetBotOption(Client::booBuffCounter, false);
		}
		else {
			c->SetBotOption(Client::booBuffCounter, !c->GetBotOption(Client::booBuffCounter));
		}

		database.botdb.SaveOwnerOption(c->CharacterID(), Client::booBuffCounter, c->GetBotOption(Client::booBuffCounter));

		c->Message(Chat::White, "Bot 'buff counter' is now %s.", (c->GetBotOption(Client::booBuffCounter) ? "enabled" : "disabled"));
	}
	else if (!owner_option.compare("monkwumessage")) {

		if (!argument.compare("enable")) {
			c->SetBotOption(Client::booMonkWuMessage, true);
		}
		else if (!argument.compare("disable")) {
			c->SetBotOption(Client::booMonkWuMessage, false);
		}
		else {
			c->SetBotOption(Client::booMonkWuMessage, !c->GetBotOption(Client::booMonkWuMessage));
		}

		database.botdb.SaveOwnerOption(c->CharacterID(), Client::booMonkWuMessage, c->GetBotOption(Client::booMonkWuMessage));

		c->Message(
			Chat::White,
			"Bot 'monk wu message' is now %s.",
			(c->GetBotOption(Client::booMonkWuMessage) ? "enabled" : "disabled")
		);
	}
	else if (!owner_option.compare("current")) {

		std::string window_title = "Current Bot Owner Options Settings";
		std::string window_text = fmt::format(
			"<table>"
			"<tr>"
			"<td><c \"#FFFFFF\">Option<br>------</td>"
			"<td><c \"#00FF00\">Argument<br>-------</td>"
			"</tr>"
			"<tr>" "<td><c \"#CCCCCC\">deathmarquee</td>"   "<td><c \"#00CC00\">{}</td>" "</tr>"
			"<tr>" "<td><c \"#CCCCCC\">statsupdate</td>"    "<td><c \"#00CC00\">{}</td>" "</tr>"
			"<tr>" "<td><c \"#CCCCCC\">spawnmessage</td>"   "<td><c \"#00CC00\">{}</td>" "</tr>"
			"<tr>" "<td><c \"#CCCCCC\">spawnmessage</td>"   "<td><c \"#00CC00\">{}</td>" "</tr>"
			"<tr>" "<td><c \"#CCCCCC\">autodefend</td>"     "<td><c \"#00CC00\">{}</td>" "</tr>"
			"<tr>" "<td><c \"#CCCCCC\">buffcounter</td>"    "<td><c \"#00CC00\">{}</td>" "</tr>"
			"<tr>" "<td><c \"#CCCCCC\">monkwumessage</td>"  "<td><c \"#00CC00\">{}</td>" "</tr>"
			"</table>",
			(c->GetBotOption(Client::booDeathMarquee) ? "enabled" : "disabled"),
			(c->GetBotOption(Client::booStatsUpdate) ? "enabled" : "disabled"),
			(c->GetBotOption(Client::booSpawnMessageSay) ? "say" : (c->GetBotOption(Client::booSpawnMessageTell) ? "tell" : "silent")),
			(c->GetBotOption(Client::booSpawnMessageClassSpecific) ? "class" : "default"),
			(RuleB(Bots, AllowOwnerOptionAutoDefend) ? (c->GetBotOption(Client::booAutoDefend) ? "enabled" : "disabled") : "restricted"),
			(c->GetBotOption(Client::booBuffCounter) ? "enabled" : "disabled"),
			(c->GetBotOption(Client::booMonkWuMessage) ? "enabled" : "disabled")
		);

		c->SendPopupToClient(window_title.c_str(), window_text.c_str());
	}
	else {
		c->Message(Chat::White, "Owner option '%s' is not recognized.", owner_option.c_str());
	}
}
