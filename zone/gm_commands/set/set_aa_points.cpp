#include "../../client.h"
#include "../../groups.h"
#include "../../raids.h"
#include "../../raids.h"

void SetAAPoints(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 3 || !sep->IsNumber(3)) {
		c->Message(Chat::White, "Usage: #set aa_points [aa|group|raid] [Amount]");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const std::string& aa_type   = Strings::ToLower(sep->arg[2]);
	const uint32       aa_points = Strings::ToUnsignedInt(sep->arg[3]);

	std::string group_raid_string;

	const bool is_aa    = Strings::EqualFold(aa_type, "aa");
	const bool is_group = Strings::EqualFold(aa_type, "group");
	const bool is_raid  = Strings::EqualFold(aa_type, "raid");

	if (
		!is_aa &&
		!is_group &&
		!is_raid
	) {
		c->Message(Chat::White, "Usage: #set aa_points [aa|group|raid] [Amount]");
		return;
	}

	if (is_aa) {
		t->GetPP().aapoints = aa_points;
		t->GetPP().expAA    = 0;
		t->SendAlternateAdvancementStats();
	} else if (is_group || is_raid) {
		if (is_group) {
			group_raid_string = "Group ";
			t->GetPP().group_leadership_points = aa_points;
			t->GetPP().group_leadership_exp    = 0;
		} else if (is_raid) {
			group_raid_string = "Raid ";
			t->GetPP().raid_leadership_points = aa_points;
			t->GetPP().raid_leadership_exp    = 0;
		}

		t->SendLeadershipEXPUpdate();
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} now {} {} {}AA Point{}.",
			c->GetTargetDescription(t, TargetDescriptionType::UCYou),
			c == t ? "have" : "has",
			Strings::Commify(aa_points),
			group_raid_string,
			aa_points != 1 ? "s" : ""
		).c_str()
	);
}
