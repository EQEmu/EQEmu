#include "../../client.h"

void ShowNPCType(Client *c, const Seperator *sep)
{
	if (!sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #show npc_type [NPC ID]");
		return;
	}

	const uint32 npc_id = Strings::ToUnsignedInt(sep->arg[2]);
	const auto   d      = content_db.LoadNPCTypesData(npc_id);

	if (!d) {
		c->Message(
			Chat::White,
			fmt::format(
				"NPC ID {} was not found.",
				npc_id
			).c_str()
		);

		return;
	}

	auto npc = new NPC(
		d,
		nullptr,
		c->GetPosition(),
		GravityBehavior::Water
	);

	npc->ShowStats(c);

	safe_delete(npc);
}
