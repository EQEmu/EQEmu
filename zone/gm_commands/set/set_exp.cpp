#include "../../client.h"

void SetEXP(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 3 || !sep->IsNumber(3)) {
		c->Message(Chat::White, "Usage: #set exp [aa|exp] [Amount]");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const std::string& type = Strings::ToLower(sep->arg[2]);

	const bool is_aa  = Strings::EqualFold(type, "aa");
	const bool is_exp = Strings::EqualFold(type, "exp");
	if (!is_aa && !is_exp) {
		c->Message(Chat::White, "Usage: #set exp [aa|exp] [Amount]");
		return;
	}

	const uint32 amount = Strings::ToUnsignedInt(sep->arg[3]);

	if (is_aa) {
		t->SetEXP(
			ExpSource::GM,
			t->GetEXP(),
			amount
		);
	} else if (is_exp) {
		t->SetEXP(
			ExpSource::GM,
			amount,
			t->GetAAXP()
		);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} now {} {} {}experience.",
			c->GetTargetDescription(t, TargetDescriptionType::UCYou),
			c == t ? "have" : "has",
			Strings::Commify(amount),
			is_aa ? "AA " : ""
		).c_str()
	);
}
