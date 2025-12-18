#include "common/repositories/account_repository.h"
#include "zone/client.h"
#include "zone/groups.h"
#include "zone/raids.h"

void SetAutoLogin(Client* c, const Seperator* sep)
{
	if (!RuleB(World, EnableAutoLogin)) {
		c->Message(Chat::White, "Auto login is disabled.");
		return;
	}

	const uint16 arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set auto_login [0|1]");
		c->Message(Chat::White, "0 = Disable auto login for your account");
		c->Message(Chat::White, "1 = Set auto login character to your current character");
		return;
	}

	Client* t = c;
	if (c->GetGM() && c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	auto e = AccountRepository::FindOne(database, t->AccountID());

	if (!e.id) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to find an account entry for {}.",
				c->GetTargetDescription(t)
			).c_str()
		);
		return;
	}

	const bool set_login = Strings::ToBool(sep->arg[2]);

	e.auto_login_charname = set_login ? t->GetCleanName() : std::string();

	AccountRepository::UpdateOne(database, e);

	c->Message(
		Chat::White,
		fmt::format(
			"Auto login character has been set to '{}' for {}.",
			e.auto_login_charname,
			c->GetTargetDescription(t)
		).c_str()
	);
}
