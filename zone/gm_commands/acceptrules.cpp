#include "zone/client.h"

void command_acceptrules(Client *c, const Seperator *sep)
{
	if (database.GetAgreementFlag(c->AccountID())) {
		c->Message(Chat::White, "You have already agreed to the rules.");
		return;
	}

	database.SetAgreementFlag(c->AccountID());
	c->SendAppearancePacket(AppearanceType::Animation, Animation::Standing);
	c->Message(Chat::White, "It is recorded you have agreed to the rules.");
}
