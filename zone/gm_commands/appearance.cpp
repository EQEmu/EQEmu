#include "zone/client.h"

void command_appearance(Client *c, const Seperator *sep)
{
	const int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1) || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #appearance [Type] [Value]");
		c->Message(Chat::White, "Note: Types are as follows:");

		for (const auto& a : EQ::constants::GetAppearanceTypeMap()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Appearance Type {} | {}",
					a.first,
					a.second
				).c_str()
			);
		}

		return;
	}

	Mob *t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	const uint32 type  = Strings::ToUnsignedInt(sep->arg[1]);
	const uint32 value = Strings::ToUnsignedInt(sep->arg[2]);

	t->SendAppearancePacket(type, value);

	c->Message(
		Chat::White,
		fmt::format(
			"Appearance Sent to {} | Type: {} ({}) Value: {}",
			c->GetTargetDescription(t, TargetDescriptionType::UCSelf),
			EQ::constants::GetAppearanceTypeName(type),
			type,
			value
		).c_str()
	);
}
