#include "../../client.h"
#include "../../dialogue_window.h"

void ShowAAPoints(Client *c, const Seperator *sep)
{
	Client *t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const int aa_points       = t->GetAAPoints();
	const int spent_aa_points = t->GetSpentAA();
	const int total_aa_points = (aa_points + spent_aa_points);

	if (!total_aa_points) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} no AA Points.",
				c->GetTargetDescription(t, TargetDescriptionType::UCYou),
				c == t ? "have" : "has"
			).c_str()
		);
		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"AA Points for {} | Current: {} Spent: {} Total: {}",
			c->GetTargetDescription(t, TargetDescriptionType::UCSelf),
			Strings::Commify(aa_points),
			Strings::Commify(spent_aa_points),
			Strings::Commify(total_aa_points)
		).c_str()
	);
}
