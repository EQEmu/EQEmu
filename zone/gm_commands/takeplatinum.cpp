#include "zone/client.h"

void command_takeplatinum(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) { //must be a number
		c->Message(Chat::Red, "Usage: #takeplatinum [Platinum]");
		return;
	}

	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	uint32 platinum = Strings::ToUnsignedInt(sep->arg[1]);
	if (!platinum) {
		c->Message(Chat::Red, "Usage: #takeplatinum [Platinum]");
		return;
	}

	bool success = target->TakePlatinum(
		platinum,
		true
	);

	if (success) {
		c->Message(
			Chat::White,
			fmt::format(
				"Subtracted {} from {}.",
				Strings::Money(
					platinum,
					0,
					0,
					0
				),
				c->GetTargetDescription(target)
			).c_str()
		);
	}
	else {
		c->Message(
			Chat::Red,
			fmt::format(
				"Unable to subtract {} from {}.",
				Strings::Money(
					platinum,
					0,
					0,
					0
				),
				c->GetTargetDescription(target)
			).c_str()
		);
	}
}
