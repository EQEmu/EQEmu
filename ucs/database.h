/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2008 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

*/

#pragma once

#include "common/database.h"
#include "common/database.h"
#include "common/linked_list.h"
#include "common/shareddb.h"
#include "common/types.h"
#include "ucs/chatchannel.h"
#include "ucs/clientlist.h"

#include <map>
#include <string>
#include <vector>

#define AUTHENTICATION_TIMEOUT	60
#define INVALID_ID				0xFFFFFFFF

class UCSDatabase : public Database {
public:
	int FindAccount(const char *CharacterName, Client *c);
	int FindCharacter(const char *CharacterName);
	bool VerifyMailKey(const std::string& characterName, int IPAddress, const std::string& MailKey);
	bool GetVariable(const char* varname, char* varvalue, uint16 varvalue_len);
	bool LoadChatChannels();
	void LoadReservedNamesFromDB();
	void LoadFilteredNamesFromDB();
	bool CheckChannelNameFilter(const std::string& channel_name);
	void SaveChatChannel(const std::string& channel_name, const std::string& channel_owner, const std::string& channel_password, const uint16& min_status);
	void DeleteChatChannel(const std::string& channel_name);
	int CurrentPlayerChannelCount(const std::string& player_name);
	std::vector<std::string> CurrentPlayerChannels(const std::string& player_name);
	void GetAccountStatus(Client *c);
	void SetChannelPassword(const std::string& channel_name, const std::string& password);
	void SetChannelOwner(const std::string& channel_name, const std::string& owner);
	void SendHeaders(Client *c);
	void SendBody(Client *c, const int& message_number);
	bool SendMail(const std::string& recipient, const std::string& from, const std::string& subject, const std::string& body, const std::string& recipients_string);
	void SetMessageStatus(const int& message_number, const int& Status);
	void ExpireMail();
	void AddFriendOrIgnore(const int& char_id, const int& type, const std::string& name);
	void RemoveFriendOrIgnore(const int& char_id, const int& type, const std::string& name);
	void GetFriendsAndIgnore(const int& char_id, std::vector<std::string> &Friends, std::vector<std::string> &Ignorees);
};
