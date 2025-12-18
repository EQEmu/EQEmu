#include "zone/client.h"

void command_givemoney(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) { //as long as the first one is a number, we'll just let atoi convert the rest to 0 or a number
		c->Message(Chat::Red, "Usage: #givemoney [Platinum] [Gold] [Silver] [Copper]");
		return;
	}


	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	uint32 platinum = Strings::ToUnsignedInt(sep->arg[1]);
	uint32 gold = sep->IsNumber(2) ? Strings::ToUnsignedInt(sep->arg[2]) : 0;
	uint32 silver = sep->IsNumber(3) ? Strings::ToUnsignedInt(sep->arg[3]) : 0;
	uint32 copper = sep->IsNumber(4) ? Strings::ToUnsignedInt(sep->arg[4]) : 0;
	if (!platinum && !gold && !silver && !copper) {
		c->Message(Chat::Red, "Usage: #givemoney [Platinum] [Gold] [Silver] [Copper]");
		return;
	}

	target->AddMoneyToPP(
		copper,
		silver,
		gold,
		platinum,
		true
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Added {} to {}.",
			Strings::Money(
				platinum,
				gold,
				silver,
				copper
			),
			c->GetTargetDescription(target)
		).c_str()
	);
}
