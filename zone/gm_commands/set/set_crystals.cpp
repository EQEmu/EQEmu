#include "../../client.h"

void SetCrystals(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(3)) {
		c->Message(Chat::White, "Usage: #setcrystals [ebon|radiant] [Amount]");
		return;
	}

	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const std::string& crystal_type = Strings::ToLower(sep->arg[2]);
	const uint32 crystal_amount = Strings::ToUnsignedInt(sep->arg[3]);

	const bool is_ebon = Strings::EqualFold(crystal_type, "ebon");
	const bool is_radiant = Strings::EqualFold(crystal_type, "radiant");
	if (!is_ebon && !is_radiant) {
		c->Message(Chat::White, "Usage: #setcrystals [ebon|radiant] [Amount]");
		return;
	}

	const uint32 crystal_item_id = (
		is_ebon ?
		RuleI(Zone, EbonCrystalItemID) :
		RuleI(Zone, RadiantCrystalItemID)
	);

	if (is_radiant) {
		t->SetRadiantCrystals(crystal_amount);
	} else {
		t->SetEbonCrystals(crystal_amount);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} now {} {} {}.",
			c->GetTargetDescription(t, TargetDescriptionType::UCYou),
			c == t ? "have" : "has",
			Strings::Commify(crystal_amount),
			database.CreateItemLink(crystal_item_id)
		).c_str()
	);
}
