#include "common/repositories/rule_sets_repository.h"
#include "common/repositories/rule_values_repository.h"
#include "zone/client.h"
#include "zone/command.h"

void command_rules(Client *c, const Seperator *sep)
{
	auto arguments = sep->argnum;
	bool is_help = !strcasecmp(sep->arg[1], "help");
	if (!arguments || is_help) {
		SendRuleSubCommands(c);
		return;
	}

	bool is_current = !strcasecmp(sep->arg[1], "current");
	bool is_get = !strcasecmp(sep->arg[1], "get");
	bool is_list = !strcasecmp(sep->arg[1], "list");
	bool is_list_sets = !strcasecmp(sep->arg[1], "listsets");
	bool is_load = !strcasecmp(sep->arg[1], "load");
	bool is_reload = !strcasecmp(sep->arg[1], "reload");
	bool is_reset = !strcasecmp(sep->arg[1], "reset");
	bool is_set = !strcasecmp(sep->arg[1], "set");
	bool is_set_db = !strcasecmp(sep->arg[1], "setdb");
	bool is_store = !strcasecmp(sep->arg[1], "store");
	bool is_switch = !strcasecmp(sep->arg[1], "switch");
	bool is_values = !strcasecmp(sep->arg[1], "values");
	if (
		!is_current &&
		!is_get &&
		!is_list &&
		!is_list_sets &&
		!is_load &&
		!is_reload &&
		!is_reset &&
		!is_set &&
		!is_set_db &&
		!is_store &&
		!is_switch &&
		!is_values
	) {
		SendRuleSubCommands(c);
		return;
	}

	if (is_current) {
		c->Message(
			Chat::White,
			fmt::format(
				"Currently running Rule Set {} ({}).",
				RuleManager::Instance()->GetActiveRuleset(),
				RuleManager::Instance()->GetActiveRulesetID()
			).c_str()
		);
	} else if (is_list_sets) {
		std::map<int, std::string> m;
		if (!RuleManager::Instance()->ListRulesets(&database, m)) {
			c->Message(Chat::White, "Failed to list Rule Sets!");
			return;
		}

		if (m.empty()) {
			c->Message(Chat::White, "There are no available Rule Sets!");
			return;
		}

		c->Message(Chat::White, "Available Rule Sets:");

		auto rule_set_count = 0;
		auto rule_set_number = 1;

		for (const auto& e : m) {
			c->Message(
				Chat::White,
				fmt::format(
					"Rule Set {} ({})",
					e.second,
					e.first
				).c_str()
			);
		}

		c->Message(
			Chat::White,
			fmt::format(
				"There are {} available Rule Set{}.",
				rule_set_count,
				rule_set_count != 1 ? "s" : ""
			).c_str()
		);
	} else if (is_reload) {
		RuleManager::Instance()->LoadRules(&database, RuleManager::Instance()->GetActiveRuleset(), true);
		c->Message(
			Chat::White,
			fmt::format(
				"Active Rule Set {} ({}) has been reloaded.",
				RuleManager::Instance()->GetActiveRuleset(),
				RuleManager::Instance()->GetActiveRulesetID()
			).c_str()
		);
	} else if (is_switch) {
		//make sure this is a valid rule set..
		const auto rsid = RuleSetsRepository::GetRuleSetID(database, sep->arg[2]);
		if (rsid < 0) {
			c->Message(
				Chat::White,
				fmt::format(
					"Rule Set '{}' does not exist or is invalid.",
					sep->arg[2]
				).c_str()
			);
			return;
		}

		if (!database.SetVariable("RuleSet", sep->arg[2])) {
			c->Message(Chat::White, "Failed to update variables table to change selected Rule Set.");
			return;
		}

		RuleManager::Instance()->LoadRules(&database, sep->arg[2], true);

		c->Message(
			Chat::White,
			"The selected ruleset has been changed to {} ({}) and reloaded locally.",
			sep->arg[2],
			rsid
		);
	} else if (is_load) {
		const auto rsid = RuleSetsRepository::GetRuleSetID(database, sep->arg[2]);
		if (rsid < 0) {
			c->Message(
				Chat::White,
				fmt::format(
					"Rule Set '{}' does not exist or is invalid.",
					sep->arg[2]
				).c_str()
			);
			return;
		}

		RuleManager::Instance()->LoadRules(&database, sep->arg[2], true);
		c->Message(
			Chat::White,
			fmt::format(
				"Loaded Rule Set {} ({}) locally.",
				sep->arg[2],
				rsid
			).c_str()
		);
	} else if (is_store) {
		if (arguments == 1) {
			RuleManager::Instance()->SaveRules(&database, "");
			c->Message(Chat::White, "Rules saved.");
		} else if (arguments == 2) {
			RuleManager::Instance()->SaveRules(&database, sep->arg[2]);
			const auto prersid = RuleManager::Instance()->GetActiveRulesetID();
			const auto rsid    = RuleSetsRepository::GetRuleSetID(database, sep->arg[2]);
			if (rsid < 0) {
				c->Message(Chat::White, "Unable to query Rule Set ID after store.");
			} else {
				c->Message(
					Chat::White,
					fmt::format(
						"Stored rules as Rule Set {} ({}).",
						sep->arg[2],
						rsid
					).c_str()
				);

				if (prersid != rsid) {
					c->Message(
						Chat::White,
						fmt::format(
							"Rule Set {} ({}) is now active locally.",
							sep->arg[2],
							rsid
						).c_str()
					);
				}
			}
		} else {
			SendRuleSubCommands(c);
			return;
		}
	} else if (is_reset) {
		RuleManager::Instance()->ResetRules(true);
		c->Message(
			Chat::White,
			fmt::format(
				"Rule Set {} ({}) has been set to defaults.",
				RuleManager::Instance()->GetActiveRuleset(),
				RuleManager::Instance()->GetActiveRulesetID()
			).c_str()
		);
	} else if (is_get) {
		if (arguments == 2) {
			std::string value;
			if (!RuleManager::Instance()->GetRule(sep->arg[2], value)) {
				c->Message(
					Chat::White,
					fmt::format(
						"Unable to find rule '{}'.",
						sep->arg[2]
					).c_str()
				);
			} else {
				c->Message(
					Chat::White,
					fmt::format(
						"{} has a value of {}.",
						sep->arg[2],
						value
					).c_str()
				);
			}
		} else {
			SendRuleSubCommands(c);
			return;
		}
	} else if (is_set) {
		if (arguments == 3) {
			if (!RuleManager::Instance()->SetRule(sep->arg[2], sep->arg[3], nullptr, false, true)) {
				c->Message(
					Chat::White,
					fmt::format(
						"Failed to modify Rule {} to a value of {}.",
						sep->arg[2],
						sep->arg[3]
					).c_str()
				);
			} else {
				c->Message(
					Chat::White,
					fmt::format(
						"Rule {} modified locally to a value of {}.",
						sep->arg[2],
						sep->arg[3]
					).c_str()
				);
			}
		} else {
			SendRuleSubCommands(c);
			return;
		}
	} else if (is_set_db) {
		if (arguments == 3) {
			if (!RuleManager::Instance()->SetRule(sep->arg[2], sep->arg[3], &database, true, true)) {
				c->Message(
					Chat::White,
					fmt::format(
						"Failed to modify Rule {} to a value of {}.",
						sep->arg[2],
						sep->arg[3]
					).c_str()
				);
			} else {
				c->Message(
					Chat::White,
					fmt::format(
						"Rule {} modified locally and in database to a value of {}.",
						sep->arg[2],
						sep->arg[3]
					).c_str()
				);
			}
		} else {
			SendRuleSubCommands(c);
			return;
		}
	} else if (is_list) {
		if (arguments == 1) {
			std::vector<std::string> l;
			if (!RuleManager::Instance()->ListCategories(l)) {
				c->Message(Chat::White, "Failed to list Rule Categories!");
				return;
			}

			if (l.empty()) {
				c->Message(Chat::White, "There are no Rule Categories to list!");
				return;
			}

			c->Message(Chat::White, "Rule Categories:");

			auto rule_category_count = 0;
			auto rule_category_number = 1;

			for (const auto& e : l) {
				c->Message(
					Chat::White,
					fmt::format(
						"Rule Category {} | {}",
						rule_category_number,
						e
					).c_str()
				);

				rule_category_count++;
				rule_category_number++;
			}

			c->Message(
				Chat::White,
				fmt::format(
					"There {} {} available Rule Categor{}.",
					rule_category_count != 1 ? "are" : "is",
					rule_category_count,
					rule_category_count != 1 ? "ies" : "y"
				).c_str()
			);
		} else if (arguments == 2) {
			std::string category_name;
			if (std::string("all") != sep->arg[2]) {
				category_name = sep->arg[2];
			}

			std::vector<std::string> l;
			if (!RuleManager::Instance()->ListRules(category_name, l)) {
				c->Message(Chat::White, "Failed to list rules!");
				return;
			}

			c->Message(
				Chat::White,
				fmt::format(
					"Rules in {} Category:",
					category_name
				).c_str()
			);

			auto rule_count = 0;
			auto rule_number = 1;

			for (const auto& e : l) {
				c->Message(
					Chat::White,
					fmt::format(
						"Rule {} | {}",
						rule_number,
						e
					).c_str()
				);

				rule_count++;
				rule_number++;
			}

			c->Message(
				Chat::White,
				fmt::format(
					"There {} {} available Rule{} in the {} Category.",
					rule_count != 1 ? "are" : "is",
					rule_count,
					rule_count != 1 ? "s" : "",
					category_name
				).c_str()
			);
		} else {
			SendRuleSubCommands(c);
			return;
		}
	} else if (is_values) {
		if (arguments == 2) {
			std::string category_name;
			if (std::string("all") != sep->arg[2]) {
				category_name = sep->arg[2];
			}

			std::vector<std::string> l;
			if (!RuleManager::Instance()->ListRules(category_name, l)) {
				c->Message(Chat::White, "Failed to list rules!");
				return;
			}

			c->Message(
				Chat::White,
				fmt::format(
					"Rule Values in {} Category:",
					category_name
				).c_str()
			);

			auto rule_count = 0;
			auto rule_number = 1;
			std::string rule_value;

			for (const auto& e : l) {
				if (RuleManager::Instance()->GetRule(e, rule_value)) {
					c->Message(
						Chat::White,
						fmt::format(
							"Rule {} | Name: {} Value: {}",
							rule_number,
							e,
							rule_value
						).c_str()
					);

					rule_count++;
					rule_number++;
				}
			}

			c->Message(
				Chat::White,
				fmt::format(
					"There {} {} available Rule{} in the {} Category.",
					rule_count != 1 ? "are" : "is",
					rule_count,
					rule_count != 1 ? "s" : "",
					category_name
				).c_str()
			);
		} else {
			SendRuleSubCommands(c);
			return;
		}
	}
}

void SendRuleSubCommands(Client *c)
{
	c->Message(Chat::White, "Usage: #rules listsets - List available rule sets");
	c->Message(Chat::White, "Usage: #rules current - gives the name of the ruleset currently running in this zone");
	c->Message(Chat::White, "Usage: #rules reload - Reload the selected ruleset in this zone");
	c->Message(Chat::White, "Usage: #rules switch [Ruleset Name] - Change the selected ruleset and load it");
	c->Message(
		Chat::White,
		"Usage: #rules load [Ruleset Name] - Load a ruleset in just this zone without changing the selected set"
	);
	c->Message(Chat::White, "Usage: #rules store [Ruleset Name] - Store the running ruleset as the specified name");
	c->Message(Chat::White, "Usage: #rules reset - Reset all rules to their default values");
	c->Message(Chat::White, "Usage: #rules get [Rule] - Get the specified rule's local value");
	c->Message(
		Chat::White,
		"Usage: #rules set [Rule) [Value] - Set the specified rule to the specified value locally only"
	);
	c->Message(
		Chat::White,
		"Usage: #rules setdb [Rule] [Value] - Set the specified rule to the specified value locally and in the DB"
	);
	c->Message(
		Chat::White,
		"Usage: #rules list [Category Name] - List all rules in the specified category (or all categiries if omitted)"
	);
	c->Message(
		Chat::White,
		"Usage: #rules values [Category Name] - List the value of all rules in the specified category"
	);
	return;
}
