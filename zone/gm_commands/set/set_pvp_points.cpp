#include "../../client.h"

void SetPVPPoints(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set pvp_points [Amount]");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const uint32 pvp_points = Strings::ToUnsignedInt(sep->arg[2]);

	t->SetPVPPoints(pvp_points);
	t->Save();
	t->SendPVPStats();

	c->Message(
		Chat::White,
		fmt::format(
			"{} now {} {} PVP Point{}.",
			c->GetTargetDescription(t, TargetDescriptionType::UCYou),
			c == t ? "have" : "has",
			Strings::Commify(pvp_points),
			pvp_points != 1 ? "s" : ""
		).c_str()
	);
}

