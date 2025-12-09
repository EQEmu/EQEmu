#include "../../client.h"
#include "../../corpse.h"

void ShowBuriedCorpseCount(Client *c, const Seperator *sep)
{
	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM()) {
		t = c->GetTarget()->CastToClient();
	}

	const int64 corpse_count = database.GetCharacterBuriedCorpseCount(t->CharacterID());

	c->Message(
		Chat::White,
		fmt::format(
			"{} {} {} buried corpse{}.",
			c->GetTargetDescription(t, TargetDescriptionType::UCYou),
			c == t ? "have" : "has",
			corpse_count,
			corpse_count != 1 ? "s" : ""
		).c_str()
	);
}
