#include "../../client.h"
#include "../../worldserver.h"

extern WorldServer worldserver;

void SetGMStatus(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		auto t = c;
		if (c->GetTarget() && c->GetTarget()->IsClient()) {
			t = c->GetTarget()->CastToClient();
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Status level has been refreshed for {}.",
				c->GetTargetDescription(t)
			).c_str()
		);

		if (t != c) {
			t->Message(
				Chat::White,
				fmt::format(
					"Your status level has been refreshed by {}.",
					c->GetCleanName()
				).c_str()
			);
		}

		t->UpdateAdmin();
		return;
	}


	if (
		!sep->IsNumber(2) ||
		!strlen(sep->arg[3])
	) {
		c->Message(Chat::White, "Usage: #set gm_status [Status] [Account Name]");
		return;
	}

	const int gm_status = Strings::ToInt(sep->arg[2]);
	if (!EQ::ValueWithin(gm_status, -2, UINT8_MAX)) {
		c->Message(Chat::White, "The lowest a status level can go is -2 and the highest a status level can go is 255.");
		return;
	}

	const std::string& account_name = sep->argplus[3];

	if (gm_status > c->Admin()) {
		c->Message(
			Chat::White,
			fmt::format(
				"You cannot set someone's status level to {} because your status level is only {}.",
				gm_status,
				c->Admin()
			).c_str()
		);
	} else if (!database.SetAccountStatus(account_name, gm_status)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to set status level to {} for account {}.",
				gm_status,
				account_name
			).c_str()
		);
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"Set GM status to {} on account {}.",
				gm_status,
				account_name
			).c_str()
		);

		std::string user;
		std::string loginserver;
		ParseAccountString(account_name, user, loginserver);

		const uint32 account_id = database.GetAccountIDByName(account_name, loginserver);

		ServerPacket pack(ServerOP_FlagUpdate, sizeof(ServerFlagUpdate_Struct));

		auto s = (ServerFlagUpdate_Struct *) pack.pBuffer;
		s->account_id = account_id;
		s->admin      = gm_status;

		worldserver.SendPacket(&pack);
	}
}

