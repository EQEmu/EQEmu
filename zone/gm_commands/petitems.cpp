#include "zone/bot.h"
#include "zone/client.h"

void command_petitems(Client *c, const Seperator *sep)
{
	NPC* t = nullptr;

	if (c->GetPet()) {
		t = c->GetPet()->CastToNPC();
	}

	if (c->GetTarget() && c->GetTarget()->IsNPC()) {
		t = c->GetTarget()->CastToNPC();
	}

	if (!t || !t->IsPet()) {
		c->Message(Chat::White, "You must have a pet or target a bot's pet to use this command.");
		return;
	}	

	Mob* o = t->GetOwner();
	if (!o) {
		c->Message(Chat::White, "Invalid owner for pet.");
		return;
	}

	if (
		o->IsBot() &&
		o->CastToBot()->GetBotOwnerCharacterID() != c->CharacterID()
	) {
		c->Message(Chat::White, "You do not own the targeted pet.");
		return;
	}

	if (
		o->IsClient() &&
		o->GetID() != c->GetID()
	) {
		c->Message(Chat::White, "You do not own the targeted pet.");
		return;
	}

	const std::string& pet_owner = (
		o->IsClient() ?
		"Your" :
		fmt::format(
			"Your bot {}{}",
			o->GetCleanName(),
			Strings::EndsWith(o->GetCleanName(), "s") ? "'" : "'s"
		)
	);

	const auto& l = t->GetLootList();
	if (!l.empty()) {
		t->QueryLoot(c, true);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} pet has {} item{}.",
			pet_owner,
			l.size(),
			l.size() != 1 ? "s" : ""
		).c_str()
	);
}
