#include "zone/client.h"

void command_unmemspells(Client *c, const Seperator *sep)
{
	auto target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient() && c->GetGM()) {
		target = c->GetTarget()->CastToClient();
	}

	auto memmed_count = target->MemmedCount();
	if (!memmed_count) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} no spells to unmemorize.",
				c->GetTargetDescription(target, TargetDescriptionType::UCYou),
				c == target ? "have" : "has"
			).c_str()
		);
		return;
	}

	target->UnmemSpellAll();

	if (c != target) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} has had {} spells unmemorized.",
				c->GetTargetDescription(target),
				memmed_count
			).c_str()
		);
	}
}
