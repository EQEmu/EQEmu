#include "../client.h"

void FindAA(Client* c, const Seperator* sep);
void FindAccount(Client* c, const Seperator* sep);
void FindBodyType(Client* c, const Seperator* sep);
void FindBot(Client* c, const Seperator* sep);
void FindBugCategory(Client* c, const Seperator* sep);
void FindCharacter(Client* c, const Seperator* sep);
void FindClass(Client* c, const Seperator* sep);
void FindComparisonType(Client* c, const Seperator* sep);
void FindCurrency(Client* c, const Seperator* sep);
void FindDeity(Client* c, const Seperator* sep);
void FindEmote(Client* c, const Seperator* sep);
void FindFaction(Client* c, const Seperator* sep);
void FindItem(Client* c, const Seperator* sep);
void FindLanguage(Client* c, const Seperator* sep);
void FindLDoNTheme(Client* c, const Seperator* sep);
void FindNPCType(Client* c, const Seperator* sep);
void FindObjectType(Client* c, const Seperator* sep);
void FindRace(Client* c, const Seperator* sep);
void FindRecipe(Client* c, const Seperator* sep);
void FindSkill(Client* c, const Seperator* sep);
void FindSpecialAbility(Client* c, const Seperator* sep);
void FindStance(Client* c, const Seperator* sep);
void FindSpell(Client* c, const Seperator* sep);
void FindTask(Client* c, const Seperator* sep);
void FindZone(Client* c, const Seperator* sep);

void command_find(Client *c, const Seperator *sep)
{
	// Cmd represents a command
	// variables are short to save space horizontally
	// when adding a new sub-command, add it to the vector below
	struct Cmd {
		std::string cmd{}; // command
		std::string u{}; // usage
		void (*fn)(Client *c, const Seperator *sep) = nullptr; // function
		std::vector<std::string> a{}; // aliases
	};

	std::vector<Cmd> commands = {
		Cmd{.cmd = "aa", .u = "aa [Search Criteria]", .fn = FindAA, .a = {"#findaa"}},
		Cmd{.cmd = "account", .u = "account [Search Criteria]", .fn = FindAccount, .a = {"#findaccount"}},
		Cmd{.cmd = "body_type", .u = "body_type [Search Criteria]", .fn = FindBodyType, .a = {"#findbodytype"}},
		Cmd{.cmd = "bug_category", .u = "bug_category [Search Criteria]", .fn = FindBugCategory, .a = {"#findbugcategory"}},
		Cmd{.cmd = "character", .u = "character [Search Criteria]", .fn = FindCharacter, .a = {"#findcharacter"}},
		Cmd{.cmd = "class", .u = "class [Search Criteria]", .fn = FindClass, .a = {"#findclass"}},
		Cmd{.cmd = "comparison_type", .u = "comparison_type [Search Criteria]", .fn = FindComparisonType, .a = {"#findcomparisontype"}},
		Cmd{.cmd = "currency", .u = "currency [Search Criteria]", .fn = FindCurrency, .a = {"#findcurrency"}},
		Cmd{.cmd = "deity", .u = "deity [Search Criteria]", .fn = FindDeity, .a = {"#finddeity"}},
		Cmd{.cmd = "emote", .u = "emote [Search Criteria]", .fn = FindEmote, .a = {"#findemote"}},
		Cmd{.cmd = "faction", .u = "faction [Search Criteria]", .fn = FindFaction, .a = {"#findfaction"}},
		Cmd{.cmd = "item", .u = "item [Search Criteria]", .fn = FindItem, .a = {"#fi", "#finditem"}},
		Cmd{.cmd = "language", .u = "language [Search Criteria]", .fn = FindLanguage, .a = {"#findlanguage"}},
		Cmd{.cmd = "ldon_theme", .u = "ldon_theme [Search Criteria]", .fn = FindLDoNTheme, .a = {"#findldontheme"}},
		Cmd{
			.cmd = "npctype", .u = "npctype [Search Criteria]", .fn = FindNPCType, .a = {
				"#fn",
				"#findnpc",
				"#findnpctype"
			}
		},
		Cmd{.cmd = "object_type", .u = "object_type [Search Criteria]", .fn = FindObjectType, .a = {"#findobjecttype"}},
		Cmd{.cmd = "race", .u = "race [Search Criteria]", .fn = FindRace, .a = {"#findrace"}},
		Cmd{.cmd = "recipe", .u = "recipe [Search Criteria]", .fn = FindRecipe, .a = {"#findrecipe"}},
		Cmd{.cmd = "skill", .u = "skill [Search Criteria]", .fn = FindSkill, .a = {"#findskill"}},
		Cmd{.cmd = "special_ability", .u = "special_ability [Search Criteria]", .fn = FindSpecialAbility, .a = {"#fsa", "#findspecialability"}},
		Cmd{.cmd = "stance", .u = "stance [Search Criteria]", .fn = FindStance, .a = {"#findstance"}},
		Cmd{.cmd = "spell", .u = "spell [Search Criteria]", .fn = FindSpell, .a = {"#fs", "#findspell"}},
		Cmd{.cmd = "task", .u = "task [Search Criteria]", .fn = FindTask, .a = {"#findtask"}},
		Cmd{.cmd = "zone", .u = "zone [Search Criteria]", .fn = FindZone, .a = {"#fz", "#findzone"}},
	};

	if (RuleB(Bots, Enabled)) {
		commands.emplace_back(
			Cmd{.cmd = "bot", .u = "bot [Search Criteria]", .fn = FindBot, .a = {"#findbot"}}
		);

		std::sort(commands.begin(), commands.end(), [](const Cmd& a, const Cmd& b) {
			return a.cmd < b.cmd;
		});
	}

	// Check for arguments
	const auto arguments = sep->argnum;
	if (!arguments) {
		for (const auto &cmd: commands) {
			c->Message(Chat::White, fmt::format("Usage: #find {}", cmd.u).c_str());
		}
		return;
	}

	// look for alias or command
	for (const auto &cmd: commands) {
		// Check for alias first
		for (const auto &alias: cmd.a) {
			if (!alias.empty() && alias == Strings::ToLower(sep->arg[0])) {
				// build string from sep args
				std::vector<std::string> args = {};

				// skip the first arg
				for (auto i = 1; i <= arguments; i++) {
					if (sep->arg[i]) {
						args.emplace_back(sep->arg[i]);
					}
				}

				// build the rewrite string
				std::string rewrite = fmt::format("#find {} {}", cmd.cmd, Strings::Join(args, " "));

				// rewrite to #find <sub-command <args>>
				c->SendGMCommand(rewrite);

				c->Message(
					Chat::Gray,
					fmt::format(
						"{} is now located under {}, using {}.",
						sep->arg[0],
						Saylink::Silent("#find"),
						Saylink::Silent(rewrite)
					).c_str()
				);

				return;
			}
		}

		// Check for command
		if (cmd.cmd == Strings::ToLower(sep->arg[1])) {
			cmd.fn(c, sep);
			return;
		}
	}

	// Command not found
	c->Message(Chat::White, "Command not found. Usage: #find [command]");
	for (const auto &cmd: commands) {
		c->Message(Chat::White, fmt::format("Usage: #find {}", cmd.u).c_str());
	}
}
