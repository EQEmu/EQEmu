#include "zone/client.h"

void command_interrupt(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;

	uint16 interrupt_message = 0x01b7, interrupt_color = 0x0121;

	if (arguments >= 1 && sep->IsNumber(1)) {
		interrupt_message = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[1]));
	}

	if (arguments == 2 && sep->IsNumber(2)) {
		interrupt_color = static_cast<uint16>(Strings::ToUnsignedInt(sep->arg[2]));
	}

	c->InterruptSpell(interrupt_message, interrupt_color);
}

