/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "zone/client.h"

void command_delpetition(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0 || strcasecmp(sep->arg[1], "*") == 0) {
		c->Message(Chat::White, "Usage: #delpetition (petition number) Type #listpetition for a list");
		return;
	}

	c->Message(Chat::Red, "Attempting to delete petition number: %i", Strings::ToInt(sep->argplus[1]));
	std::string query   = StringFormat("DELETE FROM petitions WHERE petid = %i", Strings::ToInt(sep->argplus[1]));
	auto        results = database.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	LogInfo("Delete petition request from [{}], petition number:", c->GetName(), Strings::ToInt(sep->argplus[1]));

}

#include "common/say_link.h"
#include "zone/petitions.h"
#include "zone/worldserver.h"

#include <algorithm>
#include <ctime>

extern WorldServer worldserver;

namespace {

// Client::Message is printf-style, so text that may hold % goes through "%s".
void Show(Client *c, uint32 type, const std::string &text)
{
	c->Message(type, "%s", text.c_str());
}

struct AccountPetition {
	uint32      id = 0;
	uint32      dib = 0;
	int64       sent = 0;
	bool        checked_out = false;
	std::string character;
	std::string last_gm;
	std::string text;
};

// Petitions are one per account today, but list every open one so the menu
// stays correct if a server allows more.
std::vector<AccountPetition> LoadAccountPetitions(Client *c, bool *loaded = nullptr)
{
	std::vector<AccountPetition> petitions;
	if (loaded) {
		*loaded = false;
	}
	auto results = database.QueryDatabase(fmt::format(
		"SELECT `petid`, `dib`, `senttime`, `ischeckedout`, `charname`, `lastgm`, `petitiontext` "
		"FROM `petitions` WHERE `accountname` = '{}' ORDER BY `dib`",
		Strings::Escape(c->AccountName())
	));
	if (!results.Success()) {
		return petitions;
	}
	if (loaded) {
		*loaded = true;
	}
	auto row = results.begin();
	for (uint32 i = 0; i < results.RowCount(); ++i, ++row) {
		AccountPetition petition;
		petition.id          = Strings::ToUnsignedInt(row[0] ? row[0] : "0");
		petition.dib         = Strings::ToUnsignedInt(row[1] ? row[1] : "0");
		petition.sent        = Strings::ToBigInt(row[2] ? row[2] : "0");
		petition.checked_out = row[3] && Strings::ToInt(row[3]) != 0;
		petition.character   = row[4] ? row[4] : "";
		petition.last_gm     = row[5] ? row[5] : "";
		petition.text        = row[6] ? row[6] : "";
		petitions.push_back(petition);
	}
	return petitions;
}

std::string PetitionAge(int64 sent)
{
	const int64 minutes = std::max<int64>(0, (static_cast<int64>(std::time(nullptr)) - sent) / 60);
	const auto plural = [](int64 n, const char *unit) {
		return fmt::format("{} {}{} ago", n, unit, n == 1 ? "" : "s");
	};
	if (minutes < 1) {
		return "just now";
	}
	if (minutes < 60) {
		return plural(minutes, "minute");
	}
	if (minutes < 1440) {
		return plural(minutes / 60, "hour");
	}
	return plural(minutes / 1440, "day");
}

// Spire stores "status|messages|owner" for each case it tracks. Without it
// the menu falls back to the in-game queue state.
void DescribePetition(const AccountPetition &petition, std::string &status, std::string &messages)
{
	std::string spire;
	auto results = database.QueryDatabase(fmt::format(
		"SELECT `value` FROM `data_buckets` WHERE `key` = 'spire:petition:{}-{}' "
		"AND (`expires` = 0 OR `expires` > UNIX_TIMESTAMP()) LIMIT 1",
		petition.dib,
		petition.sent
	));
	if (results.Success() && results.RowCount() > 0) {
		auto row = results.begin();
		spire = row[0] ? row[0] : "";
	}

	status = "In the queue";
	messages.clear();
	const auto first = spire.find('|');
	const auto second = first == std::string::npos ? std::string::npos : spire.find('|', first + 1);
	if (second != std::string::npos) {
		const auto state = spire.substr(0, first);
		const auto owner = spire.substr(second + 1);
		if (state == "waiting") {
			status = "Waiting for your reply";
		} else if (state == "resolved") {
			status = "Resolved by staff";
		} else if (!owner.empty()) {
			status = fmt::format("Assigned to {}", owner);
		} else {
			status = "Waiting for a Game Master";
		}
		messages = spire.substr(first + 1, second - first - 1) == "1"
			? "On - reply with #petition [message]"
			: "Off - a Game Master will turn them on to talk with you";
	}
	if (petition.checked_out) {
		status = petition.last_gm.empty()
			? "Being reviewed by a Game Master"
			: fmt::format("Being reviewed by {}", petition.last_gm);
	}
}

void ShowPetitionMenu(Client *c)
{
	const auto divider = "------------------------------------------------";
	const auto petitions = LoadAccountPetitions(c);
	c->Message(Chat::White, divider);
	Show(c, Chat::White, fmt::format("Petitions | {}", Saylink::Silent("#petition", "Refresh")));
	c->Message(Chat::White, divider);
	if (petitions.empty()) {
		c->Message(Chat::White, "You have no open petitions.");
		c->Message(Chat::White, "File one with #petition [message] or /guidehelp.");
		c->Message(Chat::White, divider);
		return;
	}
	for (const auto &petition : petitions) {
		std::string status;
		std::string messages;
		DescribePetition(petition, status, messages);
		auto preview = petition.text;
		std::replace(preview.begin(), preview.end(), '\n', ' ');
		if (preview.size() > 120) {
			preview = preview.substr(0, 117) + "...";
		}
		Show(
			c,
			Chat::White,
			fmt::format("Petition #{} | Filed by {} | {}", petition.id, petition.character, PetitionAge(petition.sent))
		);
		Show(c, Chat::White, fmt::format("Status | {}", status));
		if (!messages.empty()) {
			Show(c, Chat::White, fmt::format("Messages | {}", messages));
		}
		Show(c, Chat::White, fmt::format("Request | {}", preview));
		Show(c, Chat::White, fmt::format(
			"Actions | {} | {}",
			Saylink::Silent("#petition reply", "Add a message"),
			Saylink::Silent(fmt::format("#petition withdraw {}", petition.id), "Withdraw")
		));
		c->Message(Chat::White, divider);
	}
}

// Withdrawing reloads the queue the same way /deletepetition does.
void WithdrawPetition(Client *c, uint32 id, bool confirmed)
{
	const auto petitions = LoadAccountPetitions(c);
	const auto petition = std::find_if(petitions.begin(), petitions.end(), [id](const AccountPetition &p) { return p.id == id; });
	if (petition == petitions.end()) {
		c->Message(Chat::White, "That petition is no longer open on your account.");
		return;
	}
	if (!confirmed) {
		Show(c, Chat::White, fmt::format(
			"Withdraw petition #{}? It is removed from the queue. {} | {}",
			id,
			Saylink::Silent(fmt::format("#petition withdraw {} confirm", id), "Yes, withdraw it"),
			Saylink::Silent("#petition", "Cancel")
		));
		return;
	}
	// Delete the row first and check it, then rebuild this zone's queue from
	// the database, so the player is only told it worked when it did.
	const auto deleted = database.QueryDatabase(fmt::format(
		"DELETE FROM `petitions` WHERE `petid` = {} AND `accountname` = '{}'",
		id,
		Strings::Escape(c->AccountName())
	));
	if (!deleted.Success() || deleted.RowsAffected() != 1) {
		c->Message(Chat::Red, "The petition could not be withdrawn. Please try again.");
		return;
	}
	PetitionList::Instance()->ClearPetitions();
	PetitionList::Instance()->UpdateGMQueue();
	PetitionList::Instance()->ReadDatabase();
	PetitionList::Instance()->UpdateZoneListQueue();
	Show(c, Chat::Yellow, fmt::format("Petition #{} has been withdrawn.", id));
}

}

// #petition with no message shows the petition menu. #petition [message]
// files a petition, or, while the account has one open, sends the message to
// Game Master as a tell so Player Support can add it to the case. Any of the
// account's characters may use it. The menu's links use "reply" and
// "withdraw [id] [confirm]", which are not sent as messages.
void command_petition(Client *c, const Seperator *sep)
{
	std::string text = sep->argplus[1] ? sep->argplus[1] : "";
	Strings::Trim(text);
	if (text.empty()) {
		ShowPetitionMenu(c);
		return;
	}
	const bool withdraw = strcasecmp(sep->arg[1], "withdraw") == 0 && Strings::IsNumber(sep->arg[2]) &&
		(sep->arg[3][0] == '\0' || strcasecmp(sep->arg[3], "confirm") == 0) && sep->arg[4][0] == '\0';
	if (withdraw) {
		WithdrawPetition(c, Strings::ToUnsignedInt(sep->arg[2]), strcasecmp(sep->arg[3], "confirm") == 0);
		return;
	}
	if (strcasecmp(sep->arg[1], "reply") == 0 && sep->arg[2][0] == '\0') {
		c->Message(Chat::White, "Type #petition followed by your message to add it to your petition, for example: #petition I am still stuck.");
		return;
	}
	// Decide from the database, like the menu, since this zone's queue can lag it.
	bool loaded = false;
	const bool has_petition = !LoadAccountPetitions(c, &loaded).empty();
	if (!loaded) {
		c->Message(Chat::Red, "Your petitions could not be loaded. Please try again.");
		return;
	}
	if (!has_petition) {
		if (PetitionList::Instance()->FindPetitionByAccountName(c->AccountName())) {
			PetitionList::Instance()->ClearPetitions();
			PetitionList::Instance()->ReadDatabase();
		}
		if (c->FilePetition(text)) {
			c->Message(Chat::Yellow, "Your petition has been submitted. A Game Master will respond as soon as possible.");
		}
		return;
	}
	// Common Tongue at full skill, matching an ordinary tell.
	if (!worldserver.SendChannelMessageRaw(c, "GameMaster", ChatChannel_Tell, 0, 0, 100, text.c_str())) {
		c->Message(Chat::Red, "Error: World server disconnected");
	}
}
