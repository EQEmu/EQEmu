#include "../../client.h"
#include "../../../common/repositories/account_repository.h"

void ShowAutoLogin(Client* c, const Seperator* sep)
{
	if (!RuleB(World, EnableAutoLogin)) {
		c->Message(Chat::White, "Auto login is disabled.");
		return;
	}

	Client* t = c;
	if (c->GetGM() && c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const auto& e = AccountRepository::FindOne(database, t->AccountID());

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

	c->Message(
		Chat::White,
		fmt::format(
			"Auto login character for {} is set to '{}'.",
			c->GetTargetDescription(t),
			e.auto_login_charname
		).c_str()
	);
}
