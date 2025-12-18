#include "common/repositories/account_repository.h"
#include "zone/client.h"
#include "zone/worldserver.h"

extern WorldServer worldserver;

void command_suspendmulti(Client *c, const Seperator *sep)
{
	auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #suspendmulti [Character Name|Character Name Two|etc] [Days] [Reason]");
		c->Message(Chat::White, "Note: Specify 0 days to lift a suspension");
		return;
	}

	const auto& n = Strings::Split(sep->arg[1], "|");
	std::vector<std::string> v;
	for (const auto& c : n) {
		v.emplace_back(fmt::format("'{}'", Strings::ToLower(c)));
	}

	auto days = Strings::ToUnsignedInt(sep->arg[2]);

	const std::string reason = sep->arg[3] ? sep->argplus[3] : "";

	auto l = AccountRepository::GetWhere(
		database,
		fmt::format(
			"LOWER(charname) IN ({})",
			Strings::Implode(", ", v)
		)
	);

	if (l.empty()) {
		c->Message(Chat::White, "No characters found.");
		return;
	}

	for (auto a : l) {
		a.status         = -1;
		a.suspendeduntil = std::time(nullptr) + (days * 86400);
		a.suspend_reason = reason;

		if (!AccountRepository::UpdateOne(database, a)) {
			c->Message(
				Chat::White,
				fmt::format(
					"Failed to suspend Account {} ({}).",
					a.name,
					a.id
				).c_str()
			);
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Account {} ({}) {}.",
				a.name,
				a.id,
				(
					days ?
						fmt::format(
							"has been temporarily suspended for {} day{}.",
							days,
							days != 1 ? "s" : ""
						) :
						"is no longer suspended"
				)
			).c_str()
		);

		auto *b = entity_list.GetClientByAccID(a.id);

		if (b) {
			b->WorldKick();
			return;
		}

		auto pack = new ServerPacket(ServerOP_KickPlayer, sizeof(ServerKickPlayer_Struct));
		auto *k = (ServerKickPlayer_Struct *) pack->pBuffer;

		strn0cpy(k->adminname, c->GetName(), sizeof(k->adminname));
		k->account_id = a.id;
		k->adminrank = c->Admin();

		worldserver.SendPacket(pack);

		safe_delete(pack);
	}
}

