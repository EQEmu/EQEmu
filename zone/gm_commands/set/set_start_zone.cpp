#include "../../client.h"

void SetStartZone(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #set start_zone [Zone ID|Zone Short Name]");
		c->Message(
			Chat::White,
			"Optional Usage: Use '#set start_zone reset' or '#set start_zone 0' to clear a starting zone. Player can select a starting zone using /setstartcity"
		);
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const uint32 zone_id = (
		sep->IsNumber(2) ?
		Strings::ToUnsignedInt(sep->arg[2]) :
		ZoneID(sep->arg[2])
	);

	t->SetStartZone(zone_id);

	const bool is_reset = (
		Strings::EqualFold(sep->arg[2], "reset") ||
		zone_id == 0
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Start Zone {} for {} |{}",
			is_reset ? "Reset" : "Changed",
			c->GetTargetDescription(t, TargetDescriptionType::UCSelf),
			(
				zone_id ?
				fmt::format(
					" {} ({}) ID: {}",
					ZoneLongName(zone_id),
					ZoneName(zone_id),
					zone_id
				) :
				""
			)
		).c_str()
	);
}
