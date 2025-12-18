/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2008 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

*/

#include "database.h"

#include "common/eq_packet_structs.h"
#include "common/eqemu_logsys.h"
#include "common/misc_functions.h"
#include "common/platform/platform.h"
#include "common/repositories/chatchannel_reserved_names_repository.h"
#include "common/repositories/chatchannels_repository.h"
#include "common/repositories/name_filter_repository.h"
#include "common/strings.h"
#include "ucs/chatchannel.h"

#include "mysqld_error.h"
#include <cassert>
#include <cctype>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>

extern Clientlist      *g_Clientlist;
extern std::string GetMailPrefix();
extern ChatChannelList *ChannelList;
extern uint32          MailMessagesSent;

void UCSDatabase::GetAccountStatus(Client *client)
{

	std::string query = StringFormat(
		"SELECT `status`, `hideme`, `karma`, `revoked` FROM `account` WHERE `id` = %i LIMIT 1",
		client->GetAccountID()
	);

	auto results = QueryDatabase(query);
	if (!results.Success()) {
		LogInfo(
			"Unable to get account status for character [{}], error [{}]",
			client->GetName().c_str(),
			results.ErrorMessage().c_str()
		);

		return;
	}

	if (results.RowCount() != 1) {
		LogInfo("Error in GetAccountStatus");
		return;
	}

	auto row = results.begin();

	client->SetAccountStatus(Strings::ToInt(row[0]));
	client->SetHideMe(Strings::ToInt(row[1]) != 0);
	client->SetKarma(Strings::ToInt(row[2]));
	client->SetRevoked((Strings::ToInt(row[3]) == 1 ? true : false));

	LogDebug(
		"Set account status to [{}], hideme to [{}] and karma to [{}] for [{}]",
		client->GetAccountStatus(),
		client->GetHideMe(),
		client->GetKarma(),
		client->GetName().c_str()
	);
}

int UCSDatabase::FindAccount(const char *characterName, Client *client)
{

	LogInfo("FindAccount for character [{}]", characterName);

	client->ClearCharacters();
	std::string query   = StringFormat(
		"SELECT `id`, `account_id`, `level` FROM `character_data` WHERE `name` = '%s' LIMIT 1",
		characterName
	);
	auto        results = QueryDatabase(query);

	if (!results.Success()) {
		return -1;
	}

	if (results.RowCount() != 1) {
		return -1;
	}

	auto row = results.begin();
	client->AddCharacter(Strings::ToInt(row[0]), characterName, Strings::ToInt(row[2]));

	int accountID = Strings::ToInt(row[1]);

	LogInfo("Account ID for [{}] is [{}]", characterName, accountID);

	query = StringFormat(
		"SELECT `id`, `name`, `level` FROM `character_data` "
		"WHERE `account_id` = %i AND `name` != '%s' AND deleted_at is NULL",
		accountID, characterName
	);

	results = QueryDatabase(query);
	if (!results.Success()) {
		return accountID;
	}

	for (auto row = results.begin(); row != results.end(); ++row)
		client->AddCharacter(Strings::ToInt(row[0]), row[1], Strings::ToInt(row[2]));

	return accountID;
}

bool UCSDatabase::VerifyMailKey(const std::string& characterName, int IPAddress, const std::string& MailKey)
{

	std::string query = StringFormat(
		"SELECT `mailkey` FROM `character_data` WHERE `name`='%s' LIMIT 1",
		characterName.c_str()
	);

	auto results = QueryDatabase(query);
	if (!results.Success()) {
		LogInfo("Error retrieving mailkey from database: [{}]", results.ErrorMessage().c_str());
		return false;
	}

	if (results.RowCount() == 0) {
		LogInfo("No mailkeys found for [{}].", characterName.c_str());
		return false;
	}

	auto row = results.begin();

	// The key is the client's IP address (expressed as 8 hex digits) and an 8 hex digit random string generated
	// by world.
	//
	char combinedKey[17];

	if (RuleB(Chat, EnableMailKeyIPVerification) == true) {
		sprintf(combinedKey, "%08X%s", IPAddress, MailKey.c_str());
	}
	else {
		sprintf(combinedKey, "%s", MailKey.c_str());
	}

	LogInfo("DB key is [[{}]], Client key is [[{}]]", (row[0] ? row[0] : ""), combinedKey);

	return !strcmp(row[0], combinedKey);
}

int UCSDatabase::FindCharacter(const char *characterName)
{
	char        *safeCharName = RemoveApostrophes(characterName);
	std::string query         = StringFormat(
		"SELECT `id` FROM `character_data` WHERE `name`='%s' LIMIT 1",
		safeCharName
	);
	auto        results       = QueryDatabase(query);
	if (!results.Success()) {
		safe_delete_array(safeCharName);
		return -1;
	}

	safe_delete_array(safeCharName);

	if (results.RowCount() != 1) {
		LogInfo("Bad result from FindCharacter query for character [{}]", characterName);
		return -1;
	}

	auto row = results.begin();

	int characterID = Strings::ToInt(row[0]);

	return characterID;
}

bool UCSDatabase::GetVariable(const char *varname, char *varvalue, uint16 varvalue_len)
{

	std::string query   = StringFormat("SELECT `value` FROM `variables` WHERE `varname` = '%s'", varname);
	auto        results = QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	if (results.RowCount() != 1) {
		return false;
	}

	auto row = results.begin();

	snprintf(varvalue, varvalue_len, "%s", row[0]);

	return true;
}


bool UCSDatabase::LoadChatChannels()
{
	if (!RuleB(Chat, ChannelsIgnoreNameFilter)) {
		LoadFilteredNamesFromDB();
	}
	LoadReservedNamesFromDB();
	LogInfo("Loading chat channels from the database");

	const std::string query   = "SELECT `name`, `owner`, `password`, `minstatus` FROM `chatchannels`";
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		std::string channel_name = row[0];
		std::string channel_owner = row[1];
		std::string channel_password = row[2];
		auto channel_min_status = row[3];

		if (!ChannelList->FindChannel(channel_name)) {
			ChannelList->CreateChannel(channel_name, channel_owner, channel_password, true, Strings::ToInt(channel_min_status), false);
		}
	}
	return true;
}

void UCSDatabase::LoadReservedNamesFromDB()
{
	ChatChannelList::ClearChannelBlockList();

	auto channels = ChatchannelReservedNamesRepository::All(*this);
	if (channels.empty()) {
		LogDebug("No reserved names exist in the database...");
	}

	for (auto &e: channels) {
		ChatChannelList::AddToChannelBlockList(e.name);
		LogInfo("Adding channel [{}] to blocked list from database...", e.name);
	}

	LogInfo("Loaded [{}] reserved channel name(s)", channels.size());
}

void UCSDatabase::LoadFilteredNamesFromDB()
{
	ChatChannelList::ClearFilteredNameList();

	auto names = NameFilterRepository::All(*this);
	if (names.empty()) {
		LogDebug("No filtered names exist in the database...");
	}

	for (const auto& e : names) {
		ChatChannelList::AddToFilteredNames(e.name);
	}

	LogInfo("Loaded [{}] filtered channel name(s)", names.size());
}

void UCSDatabase::SaveChatChannel(
	const std::string& channel_name,
	const std::string& channel_owner,
	const std::string& channel_password,
	const uint16& min_status
)
{
	auto e = ChatchannelsRepository::GetWhere(
		*this,
		fmt::format(
			"`name` = '{}' LIMIT 1", Strings::Escape(channel_name)
		)
	);

	// If channel name is blocked, do not save it to the database
	if (ChatChannelList::IsOnChannelBlockList(channel_name)) {
		LogInfo("Channel [{}] already found on the block list, ignoring", channel_name);
		return;
	}

	// update if exists, create new if it doesn't
	auto c = !e.empty() ? e[0] : ChatchannelsRepository::NewEntity();
	c.name      = channel_name;
	c.owner     = channel_owner;
	c.password  = channel_password;
	c.minstatus = min_status;

	if (e.empty()) {
		ChatchannelsRepository::InsertOne(*this, c);
		return;
	}

	ChatchannelsRepository::UpdateOne(*this, c);
}

void UCSDatabase::DeleteChatChannel(const std::string& channel_name)
{
	ChatchannelsRepository::DeleteWhere(*this, fmt::format("`name` = '{}'", Strings::Escape(channel_name)));
	LogInfo("Deleting channel [{}] from the database.", channel_name);
}

std::vector<std::string> UCSDatabase::CurrentPlayerChannels(const std::string& player_name) {
	auto rows = ChatchannelsRepository::GetWhere(*this,	fmt::format("`owner` = '{}'", Strings::Escape(player_name)));
	if (rows.empty()) {
		return {};
	}
	std::vector<std::string> channels = {};
	channels.reserve(rows.size());
	for (auto &e: rows) {
		channels.emplace_back(e.name);
	}
	LogDebug("Player [{}] has the following [{}] permanent channels saved to the database: [{}].", player_name, rows.size(), Strings::Join(channels, ", "));
	return channels;
}

int UCSDatabase::CurrentPlayerChannelCount(const std::string& player_name)
{
	return (int) ChatchannelsRepository::Count(*this, fmt::format("`owner` = '{}'", Strings::Escape(player_name)));
}

void UCSDatabase::SetChannelPassword(const std::string& channel_name, const std::string& password)
{
	LogInfo("UCSDatabase::SetChannelPassword([{}], [{}])", channel_name.c_str(), password.c_str());

	std::string query = fmt::format(
		"UPDATE `chatchannels` SET `password` = '{}' WHERE `name` = '{}'",
		Strings::Escape(password), Strings::Escape(channel_name));
	QueryDatabase(query);
}

void UCSDatabase::SetChannelOwner(const std::string& channel_name, const std::string& owner)
{
	LogInfo("Setting channel [{}] owner to [{}]", channel_name, owner);

	std::string query = fmt::format(
		"UPDATE `chatchannels` SET `owner` = '{}' WHERE `name` = '{}'",
		Strings::Escape(owner),
		Strings::Escape(channel_name)
	);

	QueryDatabase(query);
}

bool UCSDatabase::CheckChannelNameFilter(const std::string& channel_name)
{
	LogDebug("Checking if [{}] is on the name filter", channel_name);

	for (const auto &e: ChatChannelList::GetFilteredNames()) {
		if (Strings::Contains(Strings::ToLower(channel_name), Strings::ToLower(e))) {
			LogInfo("Failed to pass name filter check for [{}] against word [{}]", channel_name, e);
			return false;
		}
	}

	LogDebug("Name Filter Check Passed!");

	return true;
}

void UCSDatabase::SendHeaders(Client *client)
{

	int unknownField2 = 25015275;
	int unknownField3 = 1;
	int characterID   = FindCharacter(client->MailBoxName().c_str());

	LogDebug("Sendheaders for [{}], CharID is [{}]", client->MailBoxName().c_str(), characterID);

	if (characterID <= 0) {
		return;
	}

	std::string query   = StringFormat(
		"SELECT `msgid`,`timestamp`, `from`, `subject`, `status` "
		"FROM `mail` WHERE `charid`=%i", characterID
	);
	auto        results = QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	char buffer[100];

	int headerCountPacketLength = 0;

	sprintf(buffer, "%i", client->GetMailBoxNumber());
	headerCountPacketLength += (strlen(buffer) + 1);

	sprintf(buffer, "%i", unknownField2);
	headerCountPacketLength += (strlen(buffer) + 1);

	sprintf(buffer, "%i", unknownField3);
	headerCountPacketLength += (strlen(buffer) + 1);

	sprintf(buffer, "%i", results.RowCount());
	headerCountPacketLength += (strlen(buffer) + 1);

	auto outapp = new EQApplicationPacket(OP_MailHeaderCount, headerCountPacketLength);

	char *packetBuffer = (char *) outapp->pBuffer;

	VARSTRUCT_ENCODE_INTSTRING(packetBuffer, client->GetMailBoxNumber());
	VARSTRUCT_ENCODE_INTSTRING(packetBuffer, unknownField2);
	VARSTRUCT_ENCODE_INTSTRING(packetBuffer, unknownField3);
	VARSTRUCT_ENCODE_INTSTRING(packetBuffer, results.RowCount());


	client->QueuePacket(outapp);

	safe_delete(outapp);

	int       rowIndex = 0;
	for (auto row      = results.begin(); row != results.end(); ++row, ++rowIndex) {
		int headerPacketLength = 0;

		sprintf(buffer, "%i", client->GetMailBoxNumber());
		headerPacketLength += strlen(buffer) + 1;
		sprintf(buffer, "%i", unknownField2);
		headerPacketLength += strlen(buffer) + 1;
		sprintf(buffer, "%i", rowIndex);
		headerPacketLength += strlen(buffer) + 1;

		headerPacketLength += strlen(row[0]) + 1;
		headerPacketLength += strlen(row[1]) + 1;
		headerPacketLength += strlen(row[4]) + 1;
		headerPacketLength += GetMailPrefix().length() + strlen(row[2]) + 1;
		headerPacketLength += strlen(row[3]) + 1;

		outapp = new EQApplicationPacket(OP_MailHeader, headerPacketLength);

		packetBuffer = (char *) outapp->pBuffer;

		VARSTRUCT_ENCODE_INTSTRING(packetBuffer, client->GetMailBoxNumber());
		VARSTRUCT_ENCODE_INTSTRING(packetBuffer, unknownField2);
		VARSTRUCT_ENCODE_INTSTRING(packetBuffer, rowIndex);
		VARSTRUCT_ENCODE_STRING(packetBuffer, row[0]);
		VARSTRUCT_ENCODE_STRING(packetBuffer, row[1]);
		VARSTRUCT_ENCODE_STRING(packetBuffer, row[4]);
		VARSTRUCT_ENCODE_STRING(packetBuffer, GetMailPrefix().c_str());
		packetBuffer--;
		VARSTRUCT_ENCODE_STRING(packetBuffer, row[2]);
		VARSTRUCT_ENCODE_STRING(packetBuffer, row[3]);


		client->QueuePacket(outapp);

		safe_delete(outapp);
	}

}

void UCSDatabase::SendBody(Client *client, const int& messageNumber)
{

	int characterID = FindCharacter(client->MailBoxName().c_str());

	LogInfo("SendBody: MsgID [{}], to [{}], CharID is [{}]", messageNumber, client->MailBoxName().c_str(), characterID);

	if (characterID <= 0) {
		return;
	}

	std::string query = StringFormat(
		"SELECT `msgid`, `body`, `to` FROM `mail` "
		"WHERE `charid`=%i AND `msgid`=%i",
		characterID,
		messageNumber
	);

	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return;
	}


	if (results.RowCount() != 1) {
		return;
	}

	auto row = results.begin();

	LogInfo("Message: [{}]  body ([{}] bytes)", messageNumber, strlen(row[1]));

	int packetLength = 12 + strlen(row[0]) + strlen(row[1]) + strlen(row[2]);

	auto outapp = new EQApplicationPacket(OP_MailSendBody, packetLength);

	char *packetBuffer = (char *) outapp->pBuffer;

	VARSTRUCT_ENCODE_INTSTRING(packetBuffer, client->GetMailBoxNumber());
	VARSTRUCT_ENCODE_STRING(packetBuffer, row[0]);
	VARSTRUCT_ENCODE_STRING(packetBuffer, row[1]);
	VARSTRUCT_ENCODE_STRING(packetBuffer, "1");
	VARSTRUCT_ENCODE_TYPE(uint8, packetBuffer, 0);
	VARSTRUCT_ENCODE_TYPE(uint8, packetBuffer, 0x0a);
	VARSTRUCT_ENCODE_STRING(packetBuffer, "TO:");
	packetBuffer--;
	VARSTRUCT_ENCODE_STRING(packetBuffer, row[2]);
	packetBuffer--; // Overwrite the null terminator
	VARSTRUCT_ENCODE_TYPE(uint8, packetBuffer, 0x0a);

	client->QueuePacket(outapp);

	safe_delete(outapp);
}

bool UCSDatabase::SendMail(
	const std::string& recipient,
	const std::string& from,
	const std::string& subject,
	const std::string& body,
	const std::string& recipientsString
)
{

	int         characterID;
	std::string characterName;

	auto lastPeriod = recipient.find_last_of(".");

	if (lastPeriod == std::string::npos) {
		characterName = recipient;
	}
	else {
		characterName = recipient.substr(lastPeriod + 1);
	}

	characterName[0] = toupper(characterName[0]);

	for (unsigned int i = 1; i < characterName.length(); i++)
		characterName[i] = tolower(characterName[i]);

	characterID = FindCharacter(characterName.c_str());

	LogInfo("SendMail: CharacterID for recipient [{}] is [{}]", characterName.c_str(), characterID);

	if (characterID <= 0) {
		return false;
	}

	auto escSubject = new char[subject.length() * 2 + 1];
	auto escBody    = new char[body.length() * 2 + 1];

	DoEscapeString(escSubject, subject.c_str(), subject.length());
	DoEscapeString(escBody, body.c_str(), body.length());

	int now = time(nullptr); // time returns a 64 bit int on Windows at least, which vsnprintf doesn't like.

	std::string query = StringFormat(
		"INSERT INTO `mail` (`charid`, `timestamp`, `from`, `subject`, `body`, `to`, `status`) VALUES ('%i', %i, '%s', '%s', '%s', '%s', %i)",
		characterID,
		now,
		from.c_str(),
		escSubject,
		escBody,
		recipientsString.c_str(),
		1
	);
	safe_delete_array(escSubject);
	safe_delete_array(escBody);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	LogInfo("MessageID [{}] generated, from [{}], to [{}]", results.LastInsertedID(), from.c_str(), recipient.c_str());

	Client *client = g_Clientlist->IsCharacterOnline(characterName);

	if (client) {
		std::string FQN = GetMailPrefix() + from;
		client->SendNotification(client->GetMailBoxNumber(characterName), subject, FQN, results.LastInsertedID());
	}

	MailMessagesSent++;

	return true;
}

void UCSDatabase::SetMessageStatus(const int& messageNumber, const int& status)
{

	LogInfo("SetMessageStatus [{}] [{}]", messageNumber, status);

	if (status == 0) {
		std::string query   = StringFormat("DELETE FROM `mail` WHERE `msgid` = %i", messageNumber);
		auto        results = QueryDatabase(query);
		return;
	}

	std::string query = StringFormat("UPDATE `mail` SET `status` = %i WHERE `msgid`=%i", status, messageNumber);
	QueryDatabase(query);
}

void UCSDatabase::ExpireMail()
{

	LogInfo("Expiring mail");

	std::string query   = "SELECT COUNT(*) FROM `mail`";
	auto        results = QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	auto row = results.begin();

	LogInfo("There are [{}] messages in the database", row[0]);

	/**
	 * Expire trash
	 */
	if (RuleI(Mail, ExpireTrash) >= 0) {
		query   = StringFormat(
			"DELETE FROM `mail` WHERE `status`=4 AND `timestamp` < %i",
			time(nullptr) - RuleI(Mail, ExpireTrash));
		results = QueryDatabase(query);
		if (results.Success()) {
			LogInfo("Expired [{}] trash messages", results.RowsAffected());
		}
	}

	/**
	 * Expire read
	 */
	if (RuleI(Mail, ExpireRead) >= 0) {
		query = StringFormat(
			"DELETE FROM `mail` WHERE `status` = 3 AND `timestamp` < %i",
			time(nullptr) - RuleI(Mail, ExpireRead)
		);

		results = QueryDatabase(query);
		if (results.Success())
			LogInfo("Expired [{}] read messages", results.RowsAffected());
	}

	/**
	 * Expire unread
	 */
	if (RuleI(Mail, ExpireUnread) >= 0) {
		query = StringFormat(
			"DELETE FROM `mail` WHERE `status`=1 AND `timestamp` < %i",
			time(nullptr) - RuleI(Mail, ExpireUnread)
		);

		results = QueryDatabase(query);
		if (results.Success()) {
			LogInfo("Expired [{}] unread messages", results.RowsAffected());
		}
	}
}

void UCSDatabase::AddFriendOrIgnore(const int& charID, const int& type, const std::string& name)
{
	std::string query = StringFormat(
		"INSERT INTO `friends` (`charid`, `type`, `name`) "
		"VALUES('%i', %i, '%s')",
		charID,
		type,
		CapitaliseName(name).c_str()
	);

	auto results = QueryDatabase(query);

	if (results.Success()) {
		LogInfo(
			"Wrote Friend/Ignore entry for charid [{}], type [{}], name [{}] to database",
			charID,
			type,
			name.c_str()
		);
	}
}

void UCSDatabase::RemoveFriendOrIgnore(const int& charID, const int& type, const std::string& name)
{
	std::string query = StringFormat(
		"DELETE FROM `friends` WHERE `charid` = %i AND `type` = %i AND `name` = '%s'",
		charID,
		type,
		CapitaliseName(name).c_str()
	);

	auto results = QueryDatabase(query);

	if (results.Success()) {
		LogInfo(
			"Removed Friend/Ignore entry for charid [{}], type [{}], name [{}] from database",
			charID,
			type,
			name.c_str()
		);
	}
}

void UCSDatabase::GetFriendsAndIgnore(const int& charID, std::vector<std::string> &friends, std::vector<std::string> &ignorees)
{

	std::string query   = StringFormat("select `type`, `name` FROM `friends` WHERE `charid`=%i", charID);
	auto        results = QueryDatabase(query);
	if (!results.Success()) {
		return;
	}


	for (auto row = results.begin(); row != results.end(); ++row) {
		std::string name = row[1];

		if (Strings::ToInt(row[0]) == 0) {
			ignorees.push_back(name);
			LogInfo("Added Ignoree from DB [{}]", name.c_str());
			continue;
		}

		friends.push_back(name);
		LogInfo("Added Friend from DB [{}]", name.c_str());
	}

}
