/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.org)

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

#include "../common/global_define.h"
#include "../common/eq_packet.h"
#include "../common/eq_stream_intf.h"
#include "../common/misc.h"
#include "../common/rulesys.h"
#include "../common/emu_opcodes.h"
#include "../common/eq_packet_structs.h"
#include "../common/packet_dump.h"
#include "../common/eq_stream_intf.h"
#include "../common/inventory_profile.h"
#include "../common/races.h"
#include "../common/classes.h"
#include "../common/skills.h"
#include "../common/extprofile.h"
#include "../common/strings.h"
#include "../common/emu_versions.h"
#include "../common/random.h"
#include "../common/shareddb.h"
#include "../common/opcodemgr.h"
#include "../common/data_verification.h"
#include "../common/data_bucket.h"

#include "client.h"
#include "worlddb.h"
#include "world_config.h"
#include "login_server.h"
#include "login_server_list.h"
#include "zoneserver.h"
#include "zonelist.h"
#include "clientlist.h"
#include "wguild_mgr.h"
#include "sof_char_create_data.h"
#include "../common/zone_store.h"
#include "../common/repositories/account_repository.h"
#include "../common/repositories/player_event_logs_repository.h"
#include "../common/repositories/inventory_repository.h"
#include "../common/events/player_event_logs.h"
#include "../common/content/world_content_service.h"
#include "../common/repositories/group_id_repository.h"
#include "../common/repositories/character_data_repository.h"
#include "../common/skill_caps.h"

#include <iostream>
#include <iomanip>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <limits.h>

//FatherNitwit: uncomment to enable my IP based authentication hack
//#define IPBASED_AUTH_HACK

// Disgrace: for windows compile
#ifdef _WINDOWS
	#include <winsock2.h>
	#include <windows.h>
#else

	#ifdef FREEBSD //Timothy Whitman - January 7, 2003
		#include <sys/types.h>
	#endif

	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
#endif

std::vector<RaceClassAllocation> character_create_allocations;
std::vector<RaceClassCombos> character_create_race_class_combos;

extern uint32 numclients;
extern volatile bool RunLoops;
extern volatile bool UCSServerAvailable_;

// unused ATM, but here for reference, should match RoF2
enum class NameApprovalResponse : int {
	NotValid = -1, // string ID 1576
	Rejected = 0, // string ID 1581
	Approved = 1,
	CharacterLimit = 2, // string ID 1591 older clients mention 1 char on server
	ThreeDeity = 3, // string ID 5502. 3 toons same deity team limit
	HeadStartPreOoW = 4, // string ID 6862, head start failed due to OoW not being unlocked
	HeadStartNoOoW = 5, // string ID 6863, head start failed due to not owning OoW
};

Client::Client(EQStreamInterface* ieqs)
:	autobootup_timeout(RuleI(World, ZoneAutobootTimeoutMS)),
	connect(1000),
	eqs(ieqs)
{
	// Live does not send datarate as of 3/11/2005
	//eqs->SetDataRate(7);
	ip = eqs->GetRemoteIP();
	port = ntohs(eqs->GetRemotePort());

	autobootup_timeout.Disable();
	connect.Disable();
	seen_character_select = false;
	cle = 0;
	zone_id = 0;
	char_name[0] = 0;
	charid = 0;
	zone_waiting_for_bootup = 0;
	enter_world_triggered = false;
	StartInTutorial = false;

	m_ClientVersion = eqs->ClientVersion();
	m_ClientVersionBit = EQ::versions::ConvertClientVersionToClientVersionBit(m_ClientVersion);

	numclients++;
}

Client::~Client() {
	ClearDataBucketsCache();

	if (RunLoops && cle && zone_id == 0)
		cle->SetOnline(CLE_Status::Offline);

	numclients--;

	//let the stream factory know were done with this stream
	eqs->Close();
	eqs->ReleaseFromUse();
	safe_delete(eqs);
}

void Client::SendLogServer()
{
	auto outapp = new EQApplicationPacket(OP_LogServer, sizeof(LogServer_Struct));
	LogServer_Struct *l=(LogServer_Struct *)outapp->pBuffer;
	const char *wsn=WorldConfig::get()->ShortName.c_str();
	memcpy(l->worldshortname,wsn,strlen(wsn));

	if(RuleB(Mail, EnableMailSystem))
		l->enablemail = 1;

	if(RuleB(Chat, EnableVoiceMacros))
		l->enablevoicemacros = 1;

	l->enable_pvp = (RuleI(World, PVPSettings));

	if(RuleB(World, IsGMPetitionWindowEnabled))
		l->enable_petition_wnd = 1;

	if (CanTradeFVNoDropItem()) {
		l->enable_FV = 1;
	}

	QueuePacket(outapp);
	safe_delete(outapp);
}

bool Client::CanTradeFVNoDropItem()
{
	const int16 admin_status             = GetAdmin();
	const int   no_drop_flag             = RuleI(World, FVNoDropFlag);
	const int   no_drop_min_admin_status = RuleI(Character, MinStatusForNoDropExemptions);
	switch (no_drop_flag) {
		case FVNoDropFlagRule::Disabled:
			return false;
		case FVNoDropFlagRule::Enabled:
			return true;
		case FVNoDropFlagRule::AdminOnly:
			return admin_status >= no_drop_min_admin_status;
		default:
			LogWarning("Invalid value {0} set for FVNoDropFlag", no_drop_flag);
			return false;
	}
	return false;
}

void Client::SendEnterWorld(std::string name)
{
	std::string live_name {};

	if (is_player_zoning) {
		live_name = database.GetLiveChar(GetAccountID());
		if (database.GetAccountIDByChar(live_name) != GetAccountID()) {
			eqs->Close();
			return;
		}

		LogInfo("Zoning with live_name [{}] account_id [{}]", live_name, GetAccountID());
	}

	if (!is_player_zoning && RuleB(World, EnableAutoLogin)) {
		live_name = AccountRepository::GetAutoLoginCharacterNameByAccountID(database, GetAccountID());
		LogInfo("Attempting to auto login with live_name [{}] account_id [{}]", live_name, GetAccountID());
	}

	auto outapp = new EQApplicationPacket(OP_EnterWorld, live_name.length() + 1);
	memcpy(outapp->pBuffer, live_name.c_str(), live_name.length() + 1);
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendExpansionInfo() {
	auto outapp = new EQApplicationPacket(OP_ExpansionInfo, sizeof(ExpansionInfo_Struct));
	ExpansionInfo_Struct *eis = (ExpansionInfo_Struct*)outapp->pBuffer;

	if (RuleI(World, CharacterSelectExpansionSettings) != -1) {
		eis->Expansions = RuleI(World, CharacterSelectExpansionSettings);
	}
	else if (RuleB(World, UseClientBasedExpansionSettings)) {
		eis->Expansions = EQ::expansions::ConvertClientVersionToExpansionsMask(eqs->ClientVersion());
	}
	else {
		eis->Expansions = RuleI(World, ExpansionSettings);
	}

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendCharInfo() {
	if (cle) {
		cle->SetOnline(CLE_Status::CharSelect);
	}

	if (m_ClientVersionBit & EQ::versions::maskRoFAndLater) {
		SendMaxCharCreate();
		SendMembership();
		SendMembershipSettings();
	}

	seen_character_select = true;

	// Send OP_SendCharInfo
	EQApplicationPacket *outapp = nullptr;
	database.GetCharSelectInfo(GetAccountID(), &outapp, m_ClientVersionBit);

	if (outapp) {
		QueuePacket(outapp);
	}
	else {
		LogError("Database did not return an OP_SendCharInfo packet for account [{}]", GetAccountID());
	}
	safe_delete(outapp);
}

void Client::SendMaxCharCreate() {
	auto outapp = new EQApplicationPacket(OP_SendMaxCharacters, sizeof(MaxCharacters_Struct));
	MaxCharacters_Struct* mc = (MaxCharacters_Struct*)outapp->pBuffer;

	mc->max_chars = EQ::constants::StaticLookup(m_ClientVersion)->CharacterCreationLimit;
	if (mc->max_chars > EQ::constants::CHARACTER_CREATION_LIMIT)
		mc->max_chars = EQ::constants::CHARACTER_CREATION_LIMIT;

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendMembership() {
	auto outapp = new EQApplicationPacket(OP_SendMembership, sizeof(Membership_Struct));
	Membership_Struct* mc = (Membership_Struct*)outapp->pBuffer;

	/*
		The remaining entry fields probably hold more membership restriction data that needs to be identified.
		Here is a possible list based on the EQ Website membership comparison list:
		1. Spell Ranks Allowed
		2. Prestige Items Usable
		3. In-Game Mail Service (send/recieve)
		4. Parcel Delivery (send/recieve)
		5. Loyalty Rewards Per-Week (30, 60, Max)
		6. Mercenary Tiers (Apprentice 1-2, Apprentice All Tiers, All Tiers)
		7. Neighborhood House
		8. Active Journal Quests (Tasks?) (10, 15, Max)
		9. Guild Function (join, join and create)
		10. Broker System (restricted, unlimited)
		11. Voice Chat
		12. Chat Ability
		13. Progression Server
		14. Customer Support
		15. In-Game Popup Advertising
		That is 15 possible fields, and there are 15 unknowns in the struct...Coincidence?
	*/

	mc->membership = 2;				//Hardcode to gold for now. We don't use anything else.
	mc->races = 0x1ffff;			// Available Races (4110 for silver)
	mc->classes = 0x1ffff;			// Available Classes (4614 for silver) - Was 0x101ffff
	mc->entrysize = 21;				// Number of membership setting entries below
	mc->entries[0] = 0xffffffff;	// Max AA Restriction
	mc->entries[1] = 0xffffffff;	// Max Level Restriction
	mc->entries[2] = 0xffffffff;	// Max Char Slots per Account (not used by client?)
	mc->entries[3] = 0xffffffff;	// 1 for Silver
	mc->entries[4] = 0xffffffff;	// Main Inventory Size (0xffffffff on Live for Gold, but limiting to 8 until 10 is supported)
	mc->entries[5] = 0xffffffff;	// Max Platinum per level
	mc->entries[6] = 1;				// 0 for Silver
	mc->entries[7] = 1;				// 0 for Silver
	mc->entries[8] = 1;				// 1 for Silver
	mc->entries[9] = 0xffffffff;	// Unknown - Maybe Loyalty Points every 12 hours? 60 per week for Silver
	mc->entries[10] = 1;			// 1 for Silver
	mc->entries[11] = 0xffffffff;	// Shared Bank Slots
	mc->entries[12] = 0xffffffff;	// Unknown - Maybe Max Active Tasks?
	mc->entries[13] = 1;			// 1 for Silver
	mc->entries[14] = 1;			// 0 for Silver
	mc->entries[15] = 1;			// 0 for Silver
	mc->entries[16] = 1;			// 1 for Silver
	mc->entries[17] = 1;			// 0 for Silver
	mc->entries[18] = 1;			// 0 for Silver
	mc->entries[19] = 0xffffffff;	// 0 for Silver
	mc->entries[20] = 0xffffffff;	// 0 for Silver
	mc->exit_url_length = 0;
	//mc->exit_url = 0; // Used on Live: "http://www.everquest.com/free-to-play/exit-silver"

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendMembershipSettings() {
	auto outapp = new EQApplicationPacket(OP_SendMembershipDetails, sizeof(Membership_Details_Struct));
	Membership_Details_Struct* mds = (Membership_Details_Struct*)outapp->pBuffer;

	mds->membership_setting_count = 66;
	int32 gold_settings[22] = {-1,-1,-1,-1,-1,-1,1,1,1,-1,1,-1,-1,1,1,1,1,1,1,-1,-1,0};
	uint32 entry_count = 0;
	for (int setting_id=0; setting_id < 22; setting_id++)
	{
		for (int setting_index=0; setting_index < 3; setting_index++)
		{

			mds->settings[entry_count].setting_index = setting_index;
			mds->settings[entry_count].setting_id = setting_id;
			mds->settings[entry_count].setting_value = gold_settings[setting_id];
			entry_count++;
		}
	}

	mds->race_entry_count = 15;
	mds->class_entry_count = 15;

	uint32 cur_purchase_id = 90287;
	uint32 cur_purchase_id2 = 90301;
	uint32 cur_bitwise_value = 1;
	for (int entry_id=0; entry_id < 15; entry_id++)
	{
		if (entry_id == 0)
		{
			mds->membership_races[entry_id].purchase_id = 1;
			mds->membership_races[entry_id].bitwise_entry = 0x1ffff;
			mds->membership_classes[entry_id].purchase_id = 1;
			mds->membership_classes[entry_id].bitwise_entry = 0x1ffff;
		}
		else
		{
			mds->membership_races[entry_id].purchase_id = cur_purchase_id;

			if (entry_id < 3)
			{
				mds->membership_classes[entry_id].purchase_id = cur_purchase_id;
			}
			else
			{
				mds->membership_classes[entry_id].purchase_id = cur_purchase_id2;
				cur_purchase_id2++;
			}

			if (entry_id == 1)
			{
				mds->membership_races[entry_id].bitwise_entry = 4110;
				mds->membership_classes[entry_id].bitwise_entry = 4614;
			}
			else if (entry_id == 2)
			{
				mds->membership_races[entry_id].bitwise_entry = 4110;
				mds->membership_classes[entry_id].bitwise_entry = 4614;
			}
			else
			{
				if (entry_id == 12)
				{
					// Live Skips 4096
					cur_bitwise_value *= 2;
				}
				mds->membership_races[entry_id].bitwise_entry = cur_bitwise_value;
				mds->membership_classes[entry_id].bitwise_entry = cur_bitwise_value;
			}
			cur_purchase_id++;
		}
		cur_bitwise_value *= 2;
	}
	mds->exit_url_length = 0;	// Live uses 42
	//strcpy(eq->exit_url, "http://www.everquest.com/free-to-play/exit");
	mds->exit_url_length2 = 0;	// Live uses 49
	//strcpy(eq->exit_url2, "http://www.everquest.com/free-to-play/exit-silver");

	/*
	Account Access Level Settings

	ID	-	Free	Silver	Gold	-	Possible Setting
	00	-	250		1000	-1		-	Max AA Restriction
	01	-	-1		-1		-1		-	Max Level Restriction
	02	-	2		4		-1		-	Max Char Slots per Account
	03	-	1		1		-1		-	Max Spell Rank
	04	-	4		6		-1		-	Main Inventory Size
	05	-	100		500		-1		-	Max Platinum per level
	06	-	0		0		1		-	Send Mail?
	07	-	0		0		1		-	Send Parcels?
	08	-	1		1		1		-	Voice Chat Unlimited?
	09	-	2		5		-1		-	Mercenary Tiers
	10	-	0		1		1		-	Create Guilds?
	11	-	0		0		-1		-	Shared Bank Slots
	12	-	9		14		-1		-	Max Journal Quests - 1
	13	-	0		1		1		-	Neighborhood-House Allowed?
	14	-	0		0		1		-	Prestige Enabled?
	15	-	0		0		1		-	Broker System Unlimited?
	16	-	0		1		1		-	Chat UnRestricted?
	17	-	0		0		1		-	Progression Server Access?
	18	-	0		0		1		-	Full Customer Support?
	19	-	0		0		-1		-	0 for Silver
	20	-	0		0		-1		-	0 for Silver
	21	-	0		0		0		-	Unknown 0
	*/

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendPostEnterWorld() {
	auto outapp = new EQApplicationPacket(OP_PostEnterWorld, 1);
	outapp->size=0;
	QueuePacket(outapp);
	safe_delete(outapp);
}

bool Client::HandleSendLoginInfoPacket(const EQApplicationPacket *app)
{
	if (app->size != sizeof(LoginInfo)) {
		return false;
	}

	auto *r = (LoginInfo *) app->pBuffer;

	// Quagmire - max len for name is 18, pass 15
	char name[19]     = {0};
	char password[16] = {0};
	strn0cpy(name, (char *) r->login_info, 18);
	strn0cpy(password, (char *) &(r->login_info[strlen(name) + 1]), 15);

	LogDebug("Receiving login info packet from client | name [{}] password [{}]", name, password);

	if (strlen(password) <= 1) {
		LogInfo("Login without a password");
		return false;
	}

	is_player_zoning = (r->zoning == 1);

	uint32 id = Strings::ToInt(name);
	if (id == 0) {
		LogWarning("Receiving login info packet from client | account_id is 0 - disconnecting");
		return false;
	}

	LogClientLogin("Checking authentication id [{}]", id);

	if ((cle = ClientList::Instance()->CheckAuth(id, password))) {
		LoadDataBucketsCache();

		LogClientLogin("Checking authentication id [{}] passed", id);
		if (!is_player_zoning) {
			// Track who is in and who is out of the game
			std::string in_out;

			if (cle->GetOnline() == CLE_Status::Never) {
				// Desktop -> Char Select
				in_out = "in";
			}
			else {
				// Game -> Char Select
				in_out = "out";
			}

			// Always at Char select at this point.
			// Either from a fresh client launch or coming back from the game.
			// Exiting the game entirely does not come through here.
			// Could use a Logging Out Completely message somewhere.
			cle->SetOnline(CLE_Status::CharSelect);

			LogInfo("Account ({}) Logging ({}) to character select :: LSID [{}] ", cle->AccountName(), in_out, cle->LSID());
		}
		else {
			cle->SetOnline();
		}

		const WorldConfig *Config=WorldConfig::get();

		if(Config->UpdateStats) {
			auto pack = new ServerPacket;
			pack->opcode = ServerOP_LSPlayerJoinWorld;
			pack->size = sizeof(ServerLSPlayerJoinWorld_Struct);
			pack->pBuffer = new uchar[pack->size];
			memset(pack->pBuffer,0,pack->size);
			ServerLSPlayerJoinWorld_Struct* join =(ServerLSPlayerJoinWorld_Struct*)pack->pBuffer;
			strcpy(join->key,GetLSKey());
			join->lsaccount_id = GetLSID();
			LoginServerList::Instance()->SendPacket(pack);
			safe_delete(pack);
		}

		if (!is_player_zoning)
			SendGuildList();

		SendLogServer();
		SendApproveWorld();
		SendEnterWorld(cle->name());
		SendPostEnterWorld();
		if (!is_player_zoning) {
			const auto supported_clients = RuleS(World, SupportedClients);
			bool skip_char_info = false;
			if (!supported_clients.empty()) {
				const std::string& name = EQ::versions::ClientVersionName(m_ClientVersion);
				const auto& clients = Strings::Split(supported_clients, ",");
				if (std::find(clients.begin(), clients.end(), name) == clients.end()) {
					SendUnsupportedClientPacket(
						fmt::format(
							"Client Not In Supported List [{}]",
							supported_clients
						)
					);
					skip_char_info = true;
				}
			}
			const auto& custom_files_key = RuleS(World, CustomFilesKey);
			if (!skip_char_info && !custom_files_key.empty() && cle->Admin() < RuleI(World, CustomFilesAdminLevel)) {
				// Modified clients can utilize this unused block in login_info to send custom payloads on login
				// which indicates they are using custom client files with the correct version, based on key payload.
				const auto client_key = std::string(reinterpret_cast<char*>(r->unknown064));
				if (custom_files_key != client_key) {
					std::string message = fmt::format("Missing Files [{}]", RuleS(World, CustomFilesUrl) );
					SendUnsupportedClientPacket(message);
					skip_char_info = true;
				}
			}

			if (!skip_char_info) {
				SendExpansionInfo();
				SendCharInfo();
				database.LoginIP(cle->AccountID(), long2ip(GetIP()));
			}
		}

		cle->SetIP(GetIP());
		return true;
	}
	else {
		LogInfo("Bad/Expired session key [{}]", name);
		return false;
	}
}

bool Client::HandleNameApprovalPacket(const EQApplicationPacket *app)
{
	if (GetAccountID() == 0) {
		LogInfo("Name approval request with no logged in account");
		return false;
	}

	auto n = (NameApproval_Struct*) app->pBuffer;

	strn0cpy(char_name, n->name, sizeof(char_name));

	const uint32 length   = strlen(n->name);
	const uint32 race_id  = n->race_id;
	const uint32 class_id = n->class_id;

	if (!IsPlayerRace(race_id)) {
		LogInfo("Invalid Race ID.");
		return false;
	}

	if (!EQ::ValueWithin(class_id, Class::Warrior, Class::Berserker)) {
		LogInfo("Invalid Class ID.");
		return false;
	}

	LogInfo(
		"char_name [{}] race_id [{}] class_id [{}]",
		char_name,
		GetRaceIDName(race_id),
		GetClassIDName(class_id)
	);

	bool is_valid = true;

	if (!EQ::ValueWithin(length, 4, 15)) { /* Name must be between 4 and 15 characters long, packet forged if this is true */
		is_valid = false;
	} else if (islower(char_name[0])) { /* Name must begin with an upper-case letter, can be sent with some tricking of the client */
		is_valid = false;
	} else if (strstr(char_name, " ")) { /* Name must not have any spaces, packet forged if this is true */
		is_valid = false;
	} else if (!database.CheckNameFilter(char_name)) { /* I would like to do this later, since it's likely more expensive, but oh well */
		is_valid = false;
	} else { /* Name must not contain any uppercase letters, can be sent with some tricking of the client */
		for (int i = 1; i < length; ++i) {
			if (isupper(char_name[i])) {
				is_valid = false;
				break;
			}
		}
	}

	if (is_valid) { /* Still not invalid, let's see if it's taken */
		is_valid = database.ReserveName(GetAccountID(), char_name);
	}

	auto outapp = new EQApplicationPacket(OP_ApproveName, 1);

	outapp->pBuffer[0] = is_valid ? 1 : 0;

	QueuePacket(outapp);
	safe_delete(outapp);

	if (!is_valid) {
		memset(char_name, 0, sizeof(char_name));
	}

	return true;
}

bool Client::HandleGenerateRandomNamePacket(const EQApplicationPacket *app) {
	char newName[17] = {0};
	bool unique = false;

	while (!unique) {
		std::string cons = "bcdfghjklmnpqrstvwxyz";
		std::string vows = "aeou";
		std::string allVows = "aeiou";
		std::vector<std::string> endPhon = {"a", "e", "i", "o", "u", "os", "as", "us", "is", "y", "an", "en", "in", "on", "un"};

		std::random_device rd;
		std::mt19937 gen(rd());

		std::uniform_int_distribution<int> lenDist(5, 10);
		std::uniform_int_distribution<int> firstCharDist(0, 1);
		std::uniform_int_distribution<int> consDist(0, cons.size() - 1);
		std::uniform_int_distribution<int> vowDist(0, vows.size() - 1);
		std::uniform_int_distribution<int> allVowDist(0, allVows.size() - 1);
		std::uniform_int_distribution<int> endPhonDist(0, endPhon.size() - 1);

		int len = 0;
		memset(newName, 0, sizeof(newName));

		if (firstCharDist(gen) == 0) {
			newName[len++] = vows[vowDist(gen)];
			newName[len++] = cons[consDist(gen)];
		} else {
			newName[len++] = cons[consDist(gen)];
			newName[len++] = allVows[allVowDist(gen)];
		}

		newName[0] = toupper(newName[0]);

		while (len < lenDist(gen) - 1) {
			if (len % 2 == 0) {
				newName[len++] = cons[consDist(gen)];
			} else {
				newName[len++] = allVows[allVowDist(gen)];
			}
		}

		std::string end = endPhon[endPhonDist(gen)];
		for (char c : end) {
			if (len < 10) newName[len++] = c;
		}

		if (database.CheckNameFilter(newName)) {
			std::string query = StringFormat("SELECT `name` FROM `character_data` WHERE `name` = '%s'", newName);
			auto res = database.QueryDatabase(query);
			if (res.Success() && res.RowCount() == 0) {
				unique = true;
			}
		}
	}

	NameGeneration_Struct* ngs = (NameGeneration_Struct*)app->pBuffer;
	memset(ngs->name, 0, 64);
	strcpy(ngs->name, newName);

	QueuePacket(app);
	return true;
}

bool Client::HandleCharacterCreateRequestPacket(const EQApplicationPacket *app) {
	// New OpCode in SoF
	uint32 allocs = character_create_allocations.size();
	uint32 combos = character_create_race_class_combos.size();
	uint32 len = sizeof(RaceClassAllocation) * allocs;
	len += sizeof(RaceClassCombos) * combos;
	len += sizeof(uint8);
	len += sizeof(uint32);
	len += sizeof(uint32);

	auto outapp = new EQApplicationPacket(OP_CharacterCreateRequest, len);
	unsigned char *ptr = outapp->pBuffer;
	*((uint8*)ptr) = 0;
	ptr += sizeof(uint8);

	*((uint32*)ptr) = allocs;
	ptr += sizeof(uint32);

	for(int i = 0; i < allocs; ++i) {
		RaceClassAllocation *alc = (RaceClassAllocation*)ptr;

		alc->Index = character_create_allocations[i].Index;
		for(int j = 0; j < 7; ++j) {
			alc->BaseStats[j] = character_create_allocations[i].BaseStats[j];
			alc->DefaultPointAllocation[j] = character_create_allocations[i].DefaultPointAllocation[j];
		}
		ptr += sizeof(RaceClassAllocation);
	}

	*((uint32*)ptr) = combos;
	ptr += sizeof(uint32);
	for(int i = 0; i < combos; ++i) {
		RaceClassCombos *cmb = (RaceClassCombos*)ptr;
		cmb->ExpansionRequired = character_create_race_class_combos[i].ExpansionRequired;
		cmb->Race = character_create_race_class_combos[i].Race;
		cmb->Class = character_create_race_class_combos[i].Class;
		cmb->Deity = character_create_race_class_combos[i].Deity;
		cmb->AllocationIndex = character_create_race_class_combos[i].AllocationIndex;
		cmb->Zone = character_create_race_class_combos[i].Zone;
		ptr += sizeof(RaceClassCombos);
	}

	QueuePacket(outapp);
	safe_delete(outapp);
	return true;
}

bool Client::HandleCharacterCreatePacket(const EQApplicationPacket *app) {
	if (GetAccountID() == 0) {
		LogInfo("Account ID not set; unable to create character");
		return false;
	}
	else if (app->size != sizeof(CharCreate_Struct)) {
		LogInfo("Wrong size on OP_CharacterCreate. Got: [{}], Expected: [{}]", app->size, sizeof(CharCreate_Struct));
		DumpPacket(app);
		// the previous behavior was essentially returning true here
		// but that seems a bit odd to me.
		return true;
	}

	CharCreate_Struct *cc = (CharCreate_Struct*)app->pBuffer;
	if(OPCharCreate(char_name, cc) == false) {
		database.DeleteCharacter(char_name);
		auto outapp = new EQApplicationPacket(OP_ApproveName, 1);
		outapp->pBuffer[0] = 0;
		QueuePacket(outapp);
		safe_delete(outapp);
	}
	else
	{
		if (m_ClientVersionBit & EQ::versions::maskTitaniumAndEarlier)
			StartInTutorial = true;
		SendCharInfo();
	}

	return true;
}

bool Client::HandleEnterWorldPacket(const EQApplicationPacket *app) {
	auto account_id = GetAccountID();
	if (!account_id) {
		LogInfo("Enter world with no logged in account.");
		eqs->Close();
		return true;
	}

	if (GetAdmin() < 0) {
		LogInfo("Account [{}] is banned or suspended.", account_id);
		eqs->Close();
		return true;
	}

	if (
		RuleB(World, EnableIPExemptions) ||
		RuleI(World, MaxClientsPerIP) > 0
	) {
		ClientList::Instance()->GetCLEIP(GetIP()); //Check current CLE Entry IPs against incoming connection
	}

	auto ew = (EnterWorld_Struct *) app->pBuffer;
	strn0cpy(char_name, ew->name, sizeof(char_name));

	const auto& l = CharacterDataRepository::GetWhere(
		database,
		fmt::format(
			"`name` = '{}' LIMIT 1",
			Strings::Escape(char_name)
		)
	);

	if (l.empty()) {
		LogInfo("Could not get CharInfo for [{}]", char_name);
		eqs->Close();
		return true;
	}

	auto r = WorldContentService::Instance()->FindZone(zone_id, instance_id);
	if (r.zone_id && r.instance.id != instance_id) {
		LogInfo(
			"Zone [{}] has been remapped to instance_id [{}] from instance_id [{}] for client [{}]",
			r.zone.short_name,
			r.instance.id,
			instance_id,
			char_name
		);
		instance_id = r.instance.id;
	}

	const auto& e = l.front();

	// Make sure this account owns this character
	if (e.account_id != account_id) {
		LogInfo(
			"Account [{}] does not own the character named [{}] from account [{}]",
			account_id,
			char_name,
			e.account_id
		);
		eqs->Close();
		return true;
	}

	charid      = e.id;
	zone_id     = e.zone_id;
	instance_id = e.zone_instance;

	// This can probably be moved outside and have another method return requested info (don't forget to remove the #include "../common/shareddb.h" above)
	// (This is a literal translation of the original process..I don't see why it can't be changed to a single-target query over account iteration)
	if (!is_player_zoning) {
		size_t character_limit = EQ::constants::StaticLookup(eqs->ClientVersion())->CharacterCreationLimit;
		if (character_limit > EQ::constants::CHARACTER_CREATION_LIMIT) {
			character_limit = EQ::constants::CHARACTER_CREATION_LIMIT;
		}

		if (eqs->ClientVersion() == EQ::versions::ClientVersion::Titanium) {
			character_limit = Titanium::constants::CHARACTER_CREATION_LIMIT;
		}

		auto query = fmt::format(
			"SELECT `id`, `name`, `level`, `last_login` FROM character_data WHERE `account_id` = {} ORDER BY `name` LIMIT {}",
			account_id,
			character_limit
		);
		auto results = database.QueryDatabase(query);

		/* Check GoHome */
		if (ew->return_home && !ew->tutorial) {
			bool home_enabled = false;
			for (auto row : results) {
				if (!strcasecmp(row[1], char_name)) {
					if (RuleB(World, EnableReturnHomeButton)) {
						int now = time(nullptr);
						if ((now - Strings::ToInt(row[3])) >= RuleI(World, MinOfflineTimeToReturnHome)) {
							home_enabled = true;
							break;
						}
					}
				}
			}

			if (home_enabled) {
				zone_id = database.MoveCharacterToBind(charid, 4);
			} else {
				LogInfo("[{}] is trying to go home before they're able.", char_name);
				RecordPossibleHack("[MQGoHome] player tried to go home before they were able");

				eqs->Close();
				return true;
			}
		}

		/* Check Tutorial*/
		if (RuleB(World, EnableTutorialButton) && (ew->tutorial || StartInTutorial)) {
			bool tutorial_enabled = false;
			for (auto row : results) {
				if (!strcasecmp(row[1], char_name)) {
					if (
						RuleB(World, EnableTutorialButton) &&
						Strings::ToInt(row[2]) <= RuleI(World, MaxLevelForTutorial)
					) {
						tutorial_enabled = true;
						break;
					}
				}
			}

			if (tutorial_enabled) {
				zone_id = RuleI(World, TutorialZoneID);
				database.MoveCharacterToZone(charid, zone_id);
			} else {
				LogInfo("[{}] is trying to go to the Tutorial but they are not allowed.", char_name);
				RecordPossibleHack("[MQTutorial] player tried to enter the tutorial without having tutorial enabled for this character");

				eqs->Close();
				return true;
			}
		}
	}

	if (!zone_id || !ZoneName(zone_id)) {
		// This is to save people in an invalid zone, once it's removed from the DB
		database.MoveCharacterToZone(charid, ZoneID("arena"));
		LogInfo("Zone [{}] not found, moving [{}] to Arena.", zone_id, char_name);
	}

	if (instance_id) {
		if (
			!database.VerifyInstanceAlive(instance_id, GetCharID()) ||
			!database.VerifyZoneInstance(zone_id, instance_id)
		) {
			zone_id = database.MoveCharacterToInstanceSafeReturn(charid, zone_id, instance_id);
			instance_id = 0;
		}
	}

	if(!is_player_zoning) {
		GroupIdRepository::DeleteWhere(
			database,
			fmt::format(
				"`character_id` = {} AND `name` = '{}'",
				charid,
				Strings::Escape(char_name)
			)
		);
		database.SetLoginFlags(charid, false, false, 1);
	} else {
		auto group_id = database.GetGroupID(char_name);
		if (group_id) {
			auto leader_name = database.GetGroupLeaderForLogin(char_name);
			if (!leader_name.empty()) {
				auto pack = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
				auto gj = (GroupJoin_Struct*) pack->pBuffer;
				gj->action=8;
				strn0cpy(gj->yourname, char_name, sizeof(gj->yourname));
				strn0cpy(gj->membername, leader_name.c_str(), sizeof(gj->membername));
				QueuePacket(pack);
				safe_delete(pack);
			}
		}
	}

	auto outapp = new EQApplicationPacket(OP_MOTD);
	std::string motd = RuleS(World, MOTD);
	if (!motd.empty()) {
		outapp->size    = motd.length() + 1;
		outapp->pBuffer = new uchar[outapp->size];
		memset(outapp->pBuffer, 0, outapp->size);
		strcpy((char*) outapp->pBuffer, motd.c_str());
	} else if (database.GetVariable("MOTD", motd)) {
		outapp->size    = motd.length() + 1;
		outapp->pBuffer = new uchar[outapp->size];
		memset(outapp->pBuffer, 0, outapp->size);
		strcpy((char*) outapp->pBuffer, motd.c_str());
	} else { // Null Message of the Day. :)
		outapp->size    = 1;
		outapp->pBuffer = new uchar[outapp->size];
		outapp->pBuffer[0] = 0;
	}

	QueuePacket(outapp);
	safe_delete(outapp);

	// set mailkey - used for duration of character session
	uint32 mail_key = EQ::Random::Instance()->Int(1, INT_MAX);

	database.SetMailKey(charid, GetIP(), mail_key);
	if (UCSServerAvailable_) {
		auto config = WorldConfig::get();
		std::string buffer;

		auto connection_type = EQ::versions::ucsUnknown;

		// chat server packet
		switch (GetClientVersion()) {
			case EQ::versions::ClientVersion::Titanium:
				connection_type = EQ::versions::ucsTitaniumChat;
				break;
			case EQ::versions::ClientVersion::SoF:
				connection_type = EQ::versions::ucsSoFCombined;
				break;
			case EQ::versions::ClientVersion::SoD:
				connection_type = EQ::versions::ucsSoDCombined;
				break;
			case EQ::versions::ClientVersion::UF:
				connection_type = EQ::versions::ucsUFCombined;
				break;
			case EQ::versions::ClientVersion::RoF:
				connection_type = EQ::versions::ucsRoFCombined;
				break;
			case EQ::versions::ClientVersion::RoF2:
				connection_type = EQ::versions::ucsRoF2Combined;
				break;
			default:
				connection_type = EQ::versions::ucsUnknown;
				break;
		}

		std::string ucs_addr = config->GetUCSHost();
		if (cle && cle->IsLocalClient()) {
			const char* local_addr = config->LocalAddress.c_str();
			if (local_addr[0]) {
				ucs_addr = local_addr;
			}
		}

		buffer = fmt::format("{},{},{}.{},{}{:08X}",
			ucs_addr,
			config->GetUCSPort(),
			config->ShortName,
			GetCharName(),
			static_cast<char>(connection_type),
			mail_key
		);

		outapp = new EQApplicationPacket(OP_SetChatServer, (buffer.length() + 1));
		memcpy(outapp->pBuffer, buffer.c_str(), buffer.length());
		outapp->pBuffer[buffer.length()] = '\0';

		QueuePacket(outapp);
		safe_delete(outapp);

		// mail server packet
		switch (GetClientVersion()) {
			case EQ::versions::ClientVersion::Titanium:
				connection_type = EQ::versions::ucsTitaniumMail;
				break;
			default:
				// retain value from previous switch
				break;
		}

		buffer = fmt::format("{},{},{}.{},{}{:08X}",
			ucs_addr,
			config->GetUCSPort(),
			config->ShortName,
			GetCharName(),
			static_cast<char>(connection_type),
			mail_key
		);

		outapp = new EQApplicationPacket(OP_SetChatServer2, (buffer.length() + 1));
		memcpy(outapp->pBuffer, buffer.c_str(), buffer.length());
		outapp->pBuffer[buffer.length()] = '\0';

		QueuePacket(outapp);
		safe_delete(outapp);
	}

	EnterWorld();

	return true;
}

bool Client::HandleDeleteCharacterPacket(const EQApplicationPacket *app) {

	uint32 char_acct_id = database.GetAccountIDByChar((char*)app->pBuffer);
	if(char_acct_id == GetAccountID()) {
		LogInfo("Delete character: [{}]", (const char*)app->pBuffer);
		database.DeleteCharacter((char *)app->pBuffer);
		SendCharInfo();
	}

	return true;
}

bool Client::HandleZoneChangePacket(const EQApplicationPacket *app) {
	// HoT sends this to world while zoning and wants it echoed back.
	if (m_ClientVersionBit & EQ::versions::maskRoFAndLater)
	{
		QueuePacket(app);
	}
	return true;
}

bool Client::HandlePacket(const EQApplicationPacket *app) {

	EmuOpcode opcode = app->GetOpcode();

	auto o = eqs->GetOpcodeManager();
	LogPacketClientServer(
		"[{}] [{:#06x}] Size [{}] {}",
		OpcodeManager::EmuToName(app->GetOpcode()),
		o->EmuToEQ(app->GetOpcode()) == 0 ? app->GetProtocolOpcode() : o->EmuToEQ(app->GetOpcode()),
		app->Size(),
		(EQEmuLogSys::Instance()->IsLogEnabled(Logs::Detail, Logs::PacketClientServer) ? DumpPacketToString(app) : "")
	);

	if (!eqs->CheckState(ESTABLISHED)) {
		LogInfo("Client disconnected (net inactive on send)");
		return false;
	}

	// Voidd: Anti-GM Account hack, Checks source ip against valid GM Account IP Addresses
	if (RuleB(World, GMAccountIPList) && GetAdmin() >= (RuleI(World, MinGMAntiHackStatus))) {
		if(!database.CheckGMIPs(long2ip(GetIP()), GetAccountID())) {
			LogInfo("GM Account not permited from source address [{}] and accountid [{}]", long2ip(GetIP()).c_str(), GetAccountID());
			eqs->Close();
		}
	}

	if (GetAccountID() == 0 && opcode != OP_SendLoginInfo) {
		// Got a packet other than OP_SendLoginInfo when not logged in
		LogInfo("Expecting OP_SendLoginInfo, got [{}]", OpcodeNames[opcode]);
		return false;
	}
	else if (opcode == OP_AckPacket) {
		return true;
	}

	switch(opcode)
	{
		case OP_World_Client_CRC1: // eqgame.exe
		case OP_World_Client_CRC2: // SkillCaps.txt
		case OP_World_Client_CRC3: // BaseData.txt
		{
			// There is no obvious entry in the CC struct to indicate that the 'Start Tutorial button
			// is selected when a character is created. I have observed that in this case, OP_EnterWorld is sent
			// before OP_World_Client_CRC1. Therefore, if we receive OP_World_Client_CRC1 before OP_EnterWorld,
			// then 'Start Tutorial' was not chosen.
			StartInTutorial = false;

			return HandleChecksumPacket(app);

		}
		case OP_SendLoginInfo:
		{
			return HandleSendLoginInfoPacket(app);
		}
		case OP_ApproveName: //Name approval
		{
			return HandleNameApprovalPacket(app);
		}
		case OP_RandomNameGenerator:
		{
			return HandleGenerateRandomNamePacket(app);
		}
		case OP_CharacterCreateRequest:
		{
			// New OpCode in SoF
			return HandleCharacterCreateRequestPacket(app);
		}
		case OP_CharacterCreate: //Char create
		{
			return HandleCharacterCreatePacket(app);
		}
		case OP_EnterWorld: // Enter world
		{
			return HandleEnterWorldPacket(app);
		}
		case OP_DeleteCharacter:
		{
			return HandleDeleteCharacterPacket(app);
		}
		case OP_WorldComplete:
		{
			eqs->Close();
			return true;
		}
		case OP_WorldLogout:
		{
			// I don't see this getting executed on logout
			eqs->Close();
			cle->SetOnline(CLE_Status::Offline); //allows this player to log in again without an ip restriction.
			return false;
		}
		case OP_ZoneChange:
		{
			// HoT sends this to world while zoning and wants it echoed back.
			return HandleZoneChangePacket(app);
		}
		case OP_LoginUnknown1:
		case OP_LoginUnknown2:
		case OP_CrashDump:
		case OP_WearChange:
		case OP_LoginComplete:
		case OP_ApproveWorld:
		case OP_WorldClientReady:
		{
			// Essentially we are just 'eating' these packets, indicating
			// they are handled.
			return true;
		}
		default:
		{
			LogNetcode("Received unknown EQApplicationPacket");
			return true;
		}
	}
	return true;
}

bool Client::Process() {
	bool ret = true;
	//bool sendguilds = true;
	sockaddr_in to = {};

	memset((char *) &to, 0, sizeof(to));
	to.sin_family = AF_INET;
	to.sin_port = port;
	to.sin_addr.s_addr = ip;

	if (autobootup_timeout.Check()) {
		LogInfo("Zone bootup timer expired, bootup failed or too slow");
		TellClientZoneUnavailable();
	}

	if(connect.Check()){
		SendGuildList();// Send OPCode: OP_GuildsList
		SendApproveWorld();
		connect.Disable();
	}

	if (cle)
		cle->KeepAlive();

	/************ Get all packets from packet manager out queue and process them ************/
	EQApplicationPacket *app = 0;
	while(ret && (app = (EQApplicationPacket *)eqs->PopPacket())) {
		ret = HandlePacket(app);

		delete app;
	}

	if (!eqs->CheckState(ESTABLISHED)) {
		if(WorldConfig::get()->UpdateStats){
			auto pack = new ServerPacket;
			pack->opcode = ServerOP_LSPlayerLeftWorld;
			pack->size = sizeof(ServerLSPlayerLeftWorld_Struct);
			pack->pBuffer = new uchar[pack->size];
			memset(pack->pBuffer,0,pack->size);
			ServerLSPlayerLeftWorld_Struct* logout =(ServerLSPlayerLeftWorld_Struct*)pack->pBuffer;
			strcpy(logout->key,GetLSKey());
			logout->lsaccount_id = GetLSID();
			LoginServerList::Instance()->SendPacket(pack);
			safe_delete(pack);
		}
		LogInfo("Client disconnected (not active in process)");
		return false;
	}

	return ret;
}

bool Client::HandleChecksumPacket(const EQApplicationPacket *app)
{
	// Is checksum verification turned on
	if (!RuleB(World, EnableChecksumVerification)) {
		return true;
	}

	// Get packet structure
	auto *cs = (Checksum_Struct *)app->pBuffer;

	// Determine which checksum to process
	switch (app->GetOpcode()) {
		case OP_World_Client_CRC1: // eqgame.exe
		{
			bool passes_checksum_validation = (
				ChecksumVerificationCRCEQGame(cs->checksum) ||
				(GetAdmin() >= RuleI(GM, MinStatusToBypassCheckSumVerification))
			);

			LogChecksumVerification(
				"eqgame.exe validation [{}] client [{}] ({}) has [{}] status [{}]",
				passes_checksum_validation ? "Passed" : "Failed",
				GetAccountName(),
				GetAccountID(),
				cs->checksum,
				GetAdmin()
			);

			return passes_checksum_validation;
		}
		case OP_World_Client_CRC2: // SkillCaps.txt
		{
			bool passes_checksum_validation = (
				ChecksumVerificationCRCSkillCaps(cs->checksum) ||
				(GetAdmin() >= RuleI(GM, MinStatusToBypassCheckSumVerification))
			);

			LogChecksumVerification(
				"SkillCaps.txt validation [{}] client [{}] ({}) has [{}] status [{}]",
				passes_checksum_validation ? "Passed" : "Failed",
				GetAccountName(),
				GetAccountID(),
				cs->checksum,
				GetAdmin()
			);

			return passes_checksum_validation;
		}
		case OP_World_Client_CRC3: // BaseData.txt
		{
			bool passes_checksum_validation = (
				ChecksumVerificationCRCBaseData(cs->checksum) ||
				(GetAdmin() >= RuleI(GM, MinStatusToBypassCheckSumVerification))
			);

			LogChecksumVerification(
				"BaseData.txt validation [{}] client [{}] ({}) has [{}] status [{}]",
				passes_checksum_validation ? "Passed" : "Failed",
				GetAccountName(),
				GetAccountID(),
				cs->checksum,
				GetAdmin()
			);

			return passes_checksum_validation;
		}
	}

	return false;
}

bool Client::ChecksumVerificationCRCEQGame(uint64 checksum)
{
	database.SetAccountCRCField(GetAccountID(), "crc_eqgame", checksum);

	// Get checksum variable for eqgame.exe
	std::string checksumvar;
	uint64_t    checksumint;
	if (database.GetVariable("crc_eqgame", checksumvar)) {
		checksumint = Strings::ToBigInt(checksumvar);
	}
	else {
		LogChecksumVerification("variable not set in variables table.");
		return true;
	}

	// Verify checksums match
	if (checksumint == checksum) {
		return true;
	}

	return false;
}

bool Client::ChecksumVerificationCRCSkillCaps(uint64 checksum)
{
	database.SetAccountCRCField(GetAccountID(), "crc_skillcaps", checksum);

	// Get checksum variable for eqgame.exe
	std::string checksumvar;
	uint64_t    checksumint;
	if (database.GetVariable("crc_skillcaps", checksumvar)) {
		checksumint = Strings::ToBigInt(checksumvar);
	}
	else {
		LogChecksumVerification("[checksum_crc2_skillcaps] variable not set in variables table.");
		return true;
	}

	// Verify checksums match
	if (checksumint == checksum) {
		return true;
	}

	return false;
}

bool Client::ChecksumVerificationCRCBaseData(uint64 checksum)
{
	database.SetAccountCRCField(GetAccountID(), "crc_basedata", checksum);

	// Get checksum variable for skill_caps.txt
	std::string checksumvar;
	uint64_t    checksumint;
	if (database.GetVariable("crc_basedata", checksumvar)) {
		checksumint = Strings::ToBigInt(checksumvar);
	}
	else {
		LogChecksumVerification("variable not set in variables table.");
		return true;
	}

	// Verify checksums match
	if (checksumint == checksum) {
		return true;
	}

	return false;
}

void Client::EnterWorld(bool TryBootup) {
	if (zone_id == 0)
		return;

	ZoneServer* zone_server = nullptr;
	if (instance_id > 0)
	{
		if (!database.VerifyInstanceAlive(instance_id, GetCharID()) ||
		    !database.VerifyZoneInstance(zone_id, instance_id))
		{
			instance_id = 0;
			database.MoveCharacterToInstanceSafeReturn(GetCharID(), zone_id, instance_id);
			TellClientZoneUnavailable();
			return;
		}

		zone_server = ZSList::Instance()->FindByInstanceID(instance_id);
	}
	else
	{
		zone_server = ZSList::Instance()->FindByZoneID(zone_id);
	}

	const char *zone_name = ZoneName(zone_id, true);
	if (zone_server) {
		if (false == enter_world_triggered) {
			//Drop any clients we own in other zones.
			ZSList::Instance()->DropClient(GetLSID(), zone_server);

			// warn the zone we're coming
			zone_server->IncomingClient(this);

			//tell the server not to trigger this multiple times before we get a zone unavailable
			enter_world_triggered = true;
		}
	}
	else {
		if (TryBootup) {
			LogInfo("Attempting autobootup of [{}] [{}] [{}]", zone_name, zone_id, instance_id);
			autobootup_timeout.Start();
			zone_waiting_for_bootup = ZSList::Instance()->TriggerBootup(zone_id, instance_id);
			if (zone_waiting_for_bootup == 0) {
				LogInfo("No zoneserver available to boot up");
				TellClientZoneUnavailable();
			}
			return;
		}
		else {
			LogInfo("Requested zone [{}] is not running", zone_name);
			TellClientZoneUnavailable();
			return;
		}
	}

	zone_waiting_for_bootup = 0;

	if (GetAdmin() < 80 && ZSList::Instance()->IsZoneLocked(zone_id)) {
		LogInfo("Enter world failed. Zone is locked");
		TellClientZoneUnavailable();
		return;
	}

	if (!cle) {
		TellClientZoneUnavailable();
		return;
	}

	cle->SetChar(charid, char_name);
	database.UpdateLiveChar(char_name, GetAccountID());

	LogInfo(
		"({}) [{}] [{}] (Zone ID [{}]: Instance ID: [{}]) ",
		char_name,
		(seen_character_select ? "Zoning from character select" : "Zoning to"),
		zone_name,
		zone_id,
		instance_id
	);

	if (seen_character_select) {
		auto pack = new ServerPacket;
		pack->opcode = ServerOP_AcceptWorldEntrance;
		pack->size = sizeof(WorldToZone_Struct);
		pack->pBuffer = new uchar[pack->size];
		memset(pack->pBuffer, 0, pack->size);
		WorldToZone_Struct* wtz = (WorldToZone_Struct*) pack->pBuffer;
		wtz->account_id = GetAccountID();
		wtz->response = 0;
		zone_server->SendPacket(pack);
		delete pack;
	}
	else {	// if they havent seen character select screen, we can assume this is a zone
			// to zone movement, which should be preauthorized before they leave the previous zone
		Clearance(1);
	}
}

void Client::Clearance(int8 response)
{
	ZoneServer* zs = nullptr;
	if(instance_id > 0)
	{
		zs = ZSList::Instance()->FindByInstanceID(instance_id);
	}
	else
	{
		zs = ZSList::Instance()->FindByZoneID(zone_id);
	}

	if(zs == 0 || response == -1 || response == 0)
	{
		if (zs == 0)
		{
			LogInfo("Unable to find zoneserver in Client::Clearance!!");
		} else {
			LogInfo("Invalid response [{}] in Client::Clearance", response);
		}

		TellClientZoneUnavailable();
		return;
	}

	EQApplicationPacket* outapp;

	if (zs->GetCAddress() == nullptr) {
		LogInfo("Unable to do zs->GetCAddress() in Client::Clearance!!");
		TellClientZoneUnavailable();
		return;
	}

	if (zone_id == 0) {
		LogInfo("zoneID is nullptr in Client::Clearance!!");
		TellClientZoneUnavailable();
		return;
	}

	const char* zonename = ZoneName(zone_id);
	if (zonename == 0) {
		LogInfo("zonename is nullptr in Client::Clearance!!");
		TellClientZoneUnavailable();
		return;
	}

	// Send zone server IP data
	outapp = new EQApplicationPacket(OP_ZoneServerInfo, sizeof(ZoneServerInfo_Struct));
	ZoneServerInfo_Struct* zsi = (ZoneServerInfo_Struct*)outapp->pBuffer;

	std::string zs_addr;
	if(cle && cle->IsLocalClient()) {
		const char *local_addr = zs->GetCLocalAddress();

		if(local_addr[0]) {
			zs_addr = local_addr;
		} else {
			zs_addr = zs->GetIP();

			if (zs_addr.empty()) {
				zs_addr = WorldConfig::get()->LocalAddress;
			}

			if(zs_addr == "127.0.0.1")
			{
				LogInfo("Local zone address was [{}], setting local address to: [{}]", zs_addr, WorldConfig::get()->LocalAddress.c_str());
				zs_addr = WorldConfig::get()->LocalAddress;
			} else {
				LogInfo("Local zone address [{}]", zs_addr);
			}
		}

	} else {
		const char *addr = zs->GetCAddress();
		if(addr[0]) {
			zs_addr = addr;
		} else {
			zs_addr = WorldConfig::get()->WorldAddress;
		}
	}

	strcpy(zsi->ip, zs_addr.c_str());
	zsi->port =zs->GetCPort();
	LogInfo("Sending client to zone [{}] ([{}]:[{}]) at [{}]:[{}]", zonename, zone_id, instance_id, zsi->ip, zsi->port);
	QueuePacket(outapp);
	safe_delete(outapp);

	if (cle)
		cle->SetOnline(CLE_Status::Zoning);
}

void Client::TellClientZoneUnavailable() {
	auto outapp = new EQApplicationPacket(OP_ZoneUnavail, sizeof(ZoneUnavail_Struct));
	ZoneUnavail_Struct* ua = (ZoneUnavail_Struct*)outapp->pBuffer;
	const char* zonename = ZoneName(zone_id);
	if (zonename)
		strcpy(ua->zonename, zonename);
	QueuePacket(outapp);
	delete outapp;

	zone_id = 0;
	zone_waiting_for_bootup = 0;
	enter_world_triggered = false;
	autobootup_timeout.Disable();
}

void Client::QueuePacket(const EQApplicationPacket* app, bool ack_req) {
	LogNetcode("Sending EQApplicationPacket OpCode {:#04x}", app->GetOpcode());

	ack_req = true;	// It's broke right now, dont delete this line till fix it. =P
	eqs->QueuePacket(app, ack_req);
}

void Client::SendGuildList()
{
	auto guilds_list = guild_mgr.MakeGuildList();

	std::stringstream           ss;
	cereal::BinaryOutputArchive ar(ss);
	ar(guilds_list);

	uint32 packet_size = ss.str().length();

	std::unique_ptr<EQApplicationPacket> out(new EQApplicationPacket(OP_GuildsList, packet_size));
	memcpy(out->pBuffer, ss.str().data(), out->size);

	QueuePacket(out.get());
}

// @merth: I have no idea what this struct is for, so it's hardcoded for now
void Client::SendApproveWorld()
{
	EQApplicationPacket* outapp;

	// Send OPCode: OP_ApproveWorld, size: 544
	outapp = new EQApplicationPacket(OP_ApproveWorld, sizeof(ApproveWorld_Struct));
	ApproveWorld_Struct* aw = (ApproveWorld_Struct*)outapp->pBuffer;
	uchar foo[] = {
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x37,0x87,0x13,0xbe,0xc8,0xa7,0x77,0xcb,
		0x27,0xed,0xe1,0xe6,0x5d,0x1c,0xaa,0xd3,0x3c,0x26,0x3b,0x6d,0x8c,0xdb,0x36,0x8d,
		0x91,0x72,0xf5,0xbb,0xe0,0x5c,0x50,0x6f,0x09,0x6d,0xc9,0x1e,0xe7,0x2e,0xf4,0x38,
		0x1b,0x5e,0xa8,0xc2,0xfe,0xb4,0x18,0x4a,0xf7,0x72,0x85,0x13,0xf5,0x63,0x6c,0x16,
		0x69,0xf4,0xe0,0x17,0xff,0x87,0x11,0xf3,0x2b,0xb7,0x73,0x04,0x37,0xca,0xd5,0x77,
		0xf8,0x03,0x20,0x0a,0x56,0x8b,0xfb,0x35,0xff,0x59,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x15,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x53,0xC3,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00
	};
	memcpy(aw->unknown544, foo, sizeof(foo));
	QueuePacket(outapp);
	safe_delete(outapp);
}

bool Client::OPCharCreate(char *name, CharCreate_Struct *cc)
{
	PlayerProfile_Struct pp;
	EQ::InventoryProfile inv;

	pp.SetPlayerProfileVersion(EQ::versions::ConvertClientVersionToMobVersion(EQ::versions::ConvertClientVersionBitToClientVersion(m_ClientVersionBit)));
	inv.SetInventoryVersion(EQ::versions::ConvertClientVersionBitToClientVersion(m_ClientVersionBit));
	inv.SetGMInventory(false); // character cannot have gm flag at this point

	time_t  bday = time(nullptr);
	in_addr in;

	const uint32 stats_sum = (
		cc->AGI +
		cc->CHA +
		cc->DEX +
		cc->INT +
		cc->STA +
		cc->STR +
		cc->WIS
	);

	in.s_addr = GetIP();

	LogInfo(
		"Character creation request from [{}] LS [{}] [{}] [{}]",
		GetCLE()->LSName(),
		GetCLE()->LSID(),
		inet_ntoa(in),
		GetPort()
	);
	LogInfo("Name [{}]", name);
	LogInfo(
		"race [{}] class [{}] gender [{}] deity [{}] start_zone [{}] tutorial [{}]",
		cc->race,
		cc->class_,
		cc->gender,
		cc->deity,
		cc->start_zone,
		cc->tutorial ? "true" : "false"
	);
	LogInfo(
		"AGI [{}] CHA [{}] DEX [{}] INT [{}] STA [{}] STR [{}] WIS [{}] Total [{}]",
		cc->AGI,
		cc->CHA,
		cc->DEX,
		cc->INT,
		cc->STA,
		cc->STR,
		cc->WIS,
		stats_sum
	);
	LogInfo("Face [{}] Eye Colors [{}] [{}]", cc->face, cc->eyecolor1, cc->eyecolor2);
	LogInfo("Hair [{}] Hair Color [{}]", cc->hairstyle, cc->haircolor);
	LogInfo("Beard [{}] Beard Color [{}]", cc->beard, cc->beardcolor);

	/* Validate the char creation struct */
	if (m_ClientVersionBit & EQ::versions::maskSoFAndLater) {
		if (!CheckCharCreateInfoSoF(cc)) {
			LogInfo("CheckCharCreateInfo did not validate the request (bad race/class/stats)");
			return false;
		}
	} else {
		if (!CheckCharCreateInfoTitanium(cc)) {
			LogInfo("CheckCharCreateInfo did not validate the request (bad race/class/stats)");
			return false;
		}
	}

	/* Convert incoming cc_s to the new PlayerProfile_Struct */
	memset(&pp, 0, sizeof(PlayerProfile_Struct));	// start building the profile

	strn0cpy(pp.name, name, sizeof(pp.name));

	pp.race             = cc->race;
	pp.class_           = cc->class_;
	pp.gender           = cc->gender;
	pp.deity            = cc->deity;
	pp.STR              = cc->STR;
	pp.STA              = cc->STA;
	pp.AGI              = cc->AGI;
	pp.DEX              = cc->DEX;
	pp.WIS              = cc->WIS;
	pp.INT              = cc->INT;
	pp.CHA              = cc->CHA;
	pp.face             = cc->face;
	pp.eyecolor1        = cc->eyecolor1;
	pp.eyecolor2        = cc->eyecolor2;
	pp.hairstyle        = cc->hairstyle;
	pp.haircolor        = cc->haircolor;
	pp.beard            = cc->beard;
	pp.beardcolor       = cc->beardcolor;
	pp.drakkin_heritage = cc->drakkin_heritage;
	pp.drakkin_tattoo   = cc->drakkin_tattoo;
	pp.drakkin_details  = cc->drakkin_details;
	pp.birthday         = bday;
	pp.lastlogin        = bday;
	pp.level            = 1;
	pp.points           = 5;
	pp.cur_hp           = 1000;
	pp.hunger_level     = 6000;
	pp.thirst_level     = 6000;

	/* Set default skills for everybody */
	pp.skills[EQ::skills::SkillSwimming]     = RuleI(Skills, SwimmingStartValue);
	pp.skills[EQ::skills::SkillSenseHeading] = RuleI(Skills, SenseHeadingStartValue);

	/* Set Racial and Class specific language and skills */
	SetRacialLanguages(&pp);
	SetRaceStartingSkills(&pp);
	SetClassStartingSkills(&pp);
	SetClassLanguages(&pp);

	memset(pp.spell_book, std::numeric_limits<uint8>::max(), (sizeof(uint32) * EQ::spells::SPELLBOOK_SIZE));
	memset(pp.mem_spells, std::numeric_limits<uint8>::max(), (sizeof(uint32) * EQ::spells::SPELL_GEM_COUNT));

	for (auto& b : pp.buffs) {
		b.spellid = std::numeric_limits<uint16>::max();
	}

	/* If server is PVP by default, make all character set to it. */
	pp.pvp = database.GetServerType() == 1 ? 1 : 0;

	/* If it is an SoF Client and the SoF Start Zone rule is set, send new chars there */
	if (m_ClientVersionBit & EQ::versions::maskSoFAndLater) {
		LogInfo("Found [SoFStartZoneID] rule setting [{}]", RuleI(World, SoFStartZoneID));
		if (RuleI(World, SoFStartZoneID) > 0) {
			pp.zone_id = RuleI(World, SoFStartZoneID);
			cc->start_zone = pp.zone_id;
		}
	} else {
		LogInfo("Found [TitaniumStartZoneID] rule setting [{}]", RuleI(World, TitaniumStartZoneID));
		if (RuleI(World, TitaniumStartZoneID) > 0) { 	/* if there's a startzone variable put them in there */
			pp.zone_id     = RuleI(World, TitaniumStartZoneID);
			cc->start_zone = pp.zone_id;
		}
	}

	/* use normal starting zone logic to either get defaults, or if startzone was set, load that from the db table.*/
	const bool is_valid_start_zone = content_db.GetStartZone(&pp, cc, m_ClientVersionBit & EQ::versions::maskTitaniumAndEarlier);
	if (!is_valid_start_zone){
		return false;
	}

	if (!pp.zone_id) {
		pp.zone_id = Zones::QEYNOS;

		pp.x = pp.y = pp.z = -1;
	}

	for (uint8 slot_id = 1; slot_id < 5; slot_id++) {
		pp.binds[slot_id].zone_id = pp.zone_id;
		pp.binds[slot_id].x       = pp.x;
		pp.binds[slot_id].y       = pp.y;
		pp.binds[slot_id].z       = pp.z;
		pp.binds[slot_id].heading = pp.heading;
	}

	/* Overrides if we have the tutorial flag set! */
	if (cc->tutorial && RuleB(World, EnableTutorialButton)) {
		pp.zone_id = RuleI(World, TutorialZoneID);

		auto z = GetZone(pp.zone_id);
		if (z) {
			pp.x = z->safe_x;
			pp.y = z->safe_y;
			pp.z = z->safe_z;
		}
	}

	/*  Will either be the same as home or tutorial if enabled. */
	if (RuleB(World, StartZoneSameAsBindOnCreation)) {
		pp.binds[0].zone_id = pp.zone_id;
		pp.binds[0].x       = pp.x;
		pp.binds[0].y       = pp.y;
		pp.binds[0].z       = pp.z;
		pp.binds[0].heading = pp.heading;
	}

	if (GetZone(pp.zone_id)) {
		LogInfo(
			"Current location zone_short_name [{}] zone_id [{}] x [{:.2f}] y [{:.2f}] z [{:.2f}] heading [{:.2f}]",
			ZoneName(pp.zone_id),
			pp.zone_id,
			pp.x,
			pp.y,
			pp.z,
			pp.heading
		);
	}

	if (GetZone(pp.binds[0].zone_id)) {
		LogInfo(
			"Bind location zone_short_name [{}] zone_id [{}] x [{:.2f}] y [{:.2f}] z [{:.2f}] heading [{:.2f}]",
			ZoneName(pp.binds[0].zone_id),
			pp.binds[0].zone_id,
			pp.binds[0].x,
			pp.binds[0].y,
			pp.binds[0].z,
			pp.binds[4].heading
		);
	}

	if (GetZone(pp.binds[4].zone_id)) {
		LogInfo(
			"Home location zone_short_name [{}] zone_id [{}] x [{:.2f}] y [{:.2f}] z [{:.2f}] heading [{:.2f}]",
			ZoneName(pp.binds[4].zone_id),
			pp.binds[4].zone_id,
			pp.binds[4].x,
			pp.binds[4].y,
			pp.binds[4].z,
			pp.binds[4].heading
		);
	}

	content_db.SetStartingItems(&pp, &inv, pp.race, pp.class_, pp.deity, pp.zone_id, pp.name, GetAdmin());

	const bool success = StoreCharacter(GetAccountID(), &pp, &inv);

	LogInfo("Character creation {} for [{}]", success ? "succeeded" : "failed", pp.name);
	return success;
}

// returns true if the request is ok, false if there's an error
bool CheckCharCreateInfoSoF(CharCreate_Struct *cc)
{
	if (!cc)
		return false;

	LogInfo("Validating char creation info");

	RaceClassCombos class_combo;
	bool found = false;
	int combos = character_create_race_class_combos.size();
	for (int i = 0; i < combos; ++i) {
		if (character_create_race_class_combos[i].Class == cc->class_ &&
				character_create_race_class_combos[i].Race == cc->race &&
				character_create_race_class_combos[i].Deity == cc->deity &&
				character_create_race_class_combos[i].Zone == cc->start_zone) {
			class_combo = character_create_race_class_combos[i];
			found = true;
			break;
		}
	}

	if (!found) {
		LogInfo("Could not find class/race/deity/start_zone combination");
		return false;
	}

	uint32 allocs = character_create_allocations.size();
	RaceClassAllocation allocation = {0};
	found = false;
	for (int i = 0; i < allocs; ++i) {
		if (character_create_allocations[i].Index == class_combo.AllocationIndex) {
			allocation = character_create_allocations[i];
			found = true;
			break;
		}
	}

	if (!found) {
		LogInfo("Could not find starting stats for selected character combo, cannot verify stats");
		return false;
	}

	uint32 max_stats = allocation.DefaultPointAllocation[0] +
		allocation.DefaultPointAllocation[1] +
		allocation.DefaultPointAllocation[2] +
		allocation.DefaultPointAllocation[3] +
		allocation.DefaultPointAllocation[4] +
		allocation.DefaultPointAllocation[5] +
		allocation.DefaultPointAllocation[6];

	if (cc->STR > allocation.BaseStats[0] + max_stats || cc->STR < allocation.BaseStats[0]) {
		LogInfo("Strength out of range");
		return false;
	}

	if (cc->DEX > allocation.BaseStats[1] + max_stats || cc->DEX < allocation.BaseStats[1]) {
		LogInfo("Dexterity out of range");
		return false;
	}

	if (cc->AGI > allocation.BaseStats[2] + max_stats || cc->AGI < allocation.BaseStats[2]) {
		LogInfo("Agility out of range");
		return false;
	}

	if (cc->STA > allocation.BaseStats[3] + max_stats || cc->STA < allocation.BaseStats[3]) {
		LogInfo("Stamina out of range");
		return false;
	}

	if (cc->INT > allocation.BaseStats[4] + max_stats || cc->INT < allocation.BaseStats[4]) {
		LogInfo("Intelligence out of range");
		return false;
	}

	if (cc->WIS > allocation.BaseStats[5] + max_stats || cc->WIS < allocation.BaseStats[5]) {
		LogInfo("Wisdom out of range");
		return false;
	}

	if (cc->CHA > allocation.BaseStats[6] + max_stats || cc->CHA < allocation.BaseStats[6]) {
		LogInfo("Charisma out of range");
		return false;
	}

	uint32 current_stats = 0;
	current_stats += cc->STR - allocation.BaseStats[0];
	current_stats += cc->DEX - allocation.BaseStats[1];
	current_stats += cc->AGI - allocation.BaseStats[2];
	current_stats += cc->STA - allocation.BaseStats[3];
	current_stats += cc->INT - allocation.BaseStats[4];
	current_stats += cc->WIS - allocation.BaseStats[5];
	current_stats += cc->CHA - allocation.BaseStats[6];
	if (current_stats > max_stats) {
		LogInfo("Current Stats > Maximum Stats");
		return false;
	}

	return true;
}

bool CheckCharCreateInfoTitanium(CharCreate_Struct *cc)
{
	uint32 bSTR, bSTA, bAGI, bDEX, bWIS, bINT, bCHA, bTOTAL, cTOTAL, stat_points; //these are all uint32 in CharCreate_Struct, so we'll make them uint32 here to make the compiler shut up
	int classtemp, racetemp;
	int Charerrors = 0;


// if this is increased you'll have to add a column to the classrace
// table below
#define _TABLE_RACES 16

	static const int BaseRace[_TABLE_RACES][7] =
	{            /* STR  STA  AGI  DEX  WIS  INT  CHR */
	{ /*Human*/      75,  75,  75,  75,  75,  75,  75},
	{ /*Barbarian*/ 103,  95,  82,  70,  70,  60,  55},
	{ /*Erudite*/    60,  70,  70,  70,  83, 107,  70},
	{ /*Wood Elf*/   65,  65,  95,  80,  80,  75,  75},
	{ /*High Elf*/   55,  65,  85,  70,  95,  92,  80},
	{ /*Dark Elf*/   60,  65,  90,  75,  83,  99,  60},
	{ /*Half Elf*/   70,  70,  90,  85,  60,  75,  75},
	{ /*Dwarf*/      90,  90,  70,  90,  83,  60,  45},
	{ /*Troll*/     108, 109,  83,  75,  60,  52,  40},
	{ /*Ogre*/      130, 122,  70,  70,  67,  60,  37},
	{ /*Halfling*/   70,  75,  95,  90,  80,  67,  50},
	{ /*Gnome*/      60,  70,  85,  85,  67,  98,  60},
	{ /*Iksar*/      70,  70,  90,  85,  80,  75,  55},
	{ /*Vah Shir*/   90,  75,  90,  70,  70,  65,  65},
	{ /*Froglok*/    70,  80, 100, 100,  75,  75,  50},
	{ /*Drakkin*/    70,  80,  85,  75,  80,  85,  75}
	};

	static const int BaseClass[Class::PLAYER_CLASS_COUNT][8] =
	{              /* STR  STA  AGI  DEX  WIS  INT  CHR  ADD*/
	{ /*Warrior*/      10,  10,   5,   0,   0,   0,   0,  25},
	{ /*Cleric*/        5,   5,   0,   0,  10,   0,   0,  30},
	{ /*Paladin*/      10,   5,   0,   0,   5,   0,  10,  20},
	{ /*Ranger*/        5,  10,  10,   0,   5,   0,   0,  20},
	{ /*ShadowKnight*/ 10,   5,   0,   0,   0,   10,  5,  20},
	{ /*Druid*/         0,  10,   0,   0,  10,   0,   0,  30},
	{ /*Monk*/          5,   5,  10,  10,   0,   0,   0,  20},
	{ /*Bard*/          5,   0,   0,  10,   0,   0,  10,  25},
	{ /*Rouge*/         0,   0,  10,  10,   0,   0,   0,  30},
	{ /*Shaman*/        0,   5,   0,   0,  10,   0,   5,  30},
	{ /*Necromancer*/   0,   0,   0,  10,   0,  10,   0,  30},
	{ /*Wizard*/        0,  10,   0,   0,   0,  10,   0,  30},
	{ /*Magician*/      0,  10,   0,   0,   0,  10,   0,  30},
	{ /*Enchanter*/     0,   0,   0,   0,   0,  10,  10,  30},
	{ /*Beastlord*/     0,  10,   5,   0,  10,   0,   5,  20},
	{ /*Berserker*/    10,   5,   0,  10,   0,   0,   0,  25}
	};

	static const bool ClassRaceLookupTable[Class::PLAYER_CLASS_COUNT][_TABLE_RACES]=
	{                   /*Human  Barbarian Erudite Woodelf Highelf Darkelf Halfelf Dwarf  Troll  Ogre   Halfling Gnome  Iksar  Vahshir Froglok Drakkin*/
	{ /*Warrior*/         true,  true,     false,  true,   false,  true,   true,   true,  true,  true,  true,    true,  true,  true,   true,   true},
	{ /*Cleric*/          true,  false,    true,   false,  true,   true,   true,   true,  false, false, true,    true,  false, false,  true,   true},
	{ /*Paladin*/         true,  false,    true,   false,  true,   false,  true,   true,  false, false, true,    true,  false, false,  true,   true},
	{ /*Ranger*/          true,  false,    false,  true,   false,  false,  true,   false, false, false, true,    false, false, false,  false,  true},
	{ /*ShadowKnight*/    true,  false,    true,   false,  false,  true,   false,  false, true,  true,  false,   true,  true,  false,  true,   true},
	{ /*Druid*/           true,  false,    false,  true,   false,  false,  true,   false, false, false, true,    false, false, false,  false,  true},
	{ /*Monk*/            true,  false,    false,  false,  false,  false,  false,  false, false, false, false,   false, true,  false,  false,  true},
	{ /*Bard*/            true,  false,    false,  true,   false,  false,  true,   false, false, false, false,   false, false, true,   false,  true},
	{ /*Rogue*/           true,  true,     false,  true,   false,  true,   true,   true,  false, false, true,    true,  false, true,   true,   true},
	{ /*Shaman*/          false, true,     false,  false,  false,  false,  false,  false, true,  true,  false,   false, true,  true,   true,   false},
	{ /*Necromancer*/     true,  false,    true,   false,  false,  true,   false,  false, false, false, false,   true,  true,  false,  true,   true},
	{ /*Wizard*/          true,  false,    true,   false,  true,   true,   false,  false, false, false, false,   true,  false, false,  true,   true},
	{ /*Magician*/        true,  false,    true,   false,  true,   true,   false,  false, false, false, false,   true,  false, false,  false,  true},
	{ /*Enchanter*/       true,  false,    true,   false,  true,   true,   false,  false, false, false, false,   true,  false, false,  false,  true},
	{ /*Beastlord*/       false, true,     false,  false,  false,  false,  false,  false, true,  true,  false,   false, true,  true,   false,  false},
	{ /*Berserker*/       false, true,     false,  false,  false,  false,  false,  true,  true,  true,  false,   false, false, true,   false,  false}
	};

	if (!cc)
		return false;

	LogInfo("Validating char creation info");

	classtemp = cc->class_ - 1;
	racetemp = cc->race - 1;
	// these have non sequential race numbers so they need to be mapped
	if (cc->race == Race::Froglok2) racetemp = 14;
	if (cc->race == Race::VahShir) racetemp = 13;
	if (cc->race == Race::Iksar) racetemp = 12;
	if (cc->race == Race::Drakkin) racetemp = 15;

	// if out of range looking it up in the table would crash stuff
	// so we return from these
	if (classtemp >= Class::PLAYER_CLASS_COUNT) {
		LogInfo(" class is out of range");
		return false;
	}
	if (racetemp >= _TABLE_RACES) {
		LogInfo(" race is out of range");
		return false;
	}

	if (!ClassRaceLookupTable[classtemp][racetemp]) { //Lookup table better than a bunch of ifs?
		LogInfo(" invalid race/class combination");
		// we return from this one, since if it's an invalid combination our table
		// doesn't have meaningful values for the stats
		return false;
	}

	// add up the base values for this class/race
	// this is what they start with, and they have stat_points more
	// that can distributed
	bSTR = BaseClass[classtemp][0] + BaseRace[racetemp][0];
	bSTA = BaseClass[classtemp][1] + BaseRace[racetemp][1];
	bAGI = BaseClass[classtemp][2] + BaseRace[racetemp][2];
	bDEX = BaseClass[classtemp][3] + BaseRace[racetemp][3];
	bWIS = BaseClass[classtemp][4] + BaseRace[racetemp][4];
	bINT = BaseClass[classtemp][5] + BaseRace[racetemp][5];
	bCHA = BaseClass[classtemp][6] + BaseRace[racetemp][6];
	stat_points = BaseClass[classtemp][7];
	bTOTAL = bSTR + bSTA + bAGI + bDEX + bWIS + bINT + bCHA;
	cTOTAL = cc->STR + cc->STA + cc->AGI + cc->DEX + cc->WIS + cc->INT + cc->CHA;

	// the first check makes sure the total is exactly what was expected.
	// this will catch all the stat cheating, but there's still the issue
	// of reducing CHA or INT or something, to use for STR, so we check
	// that none are lower than the base or higher than base + stat_points
	// NOTE: these could just be else if, but i want to see all the stats
	// that are messed up not just the first hit

	if (bTOTAL + stat_points != cTOTAL) {
		LogInfo(" stat points total doesn't match expected value: expecting [{}] got [{}]", bTOTAL + stat_points, cTOTAL);
		Charerrors++;
	}

	if (cc->STR > bSTR + stat_points || cc->STR < bSTR) {
		LogInfo(" stat STR is out of range");
		Charerrors++;
	}
	if (cc->STA > bSTA + stat_points || cc->STA < bSTA) {
		LogInfo(" stat STA is out of range");
		Charerrors++;
	}
	if (cc->AGI > bAGI + stat_points || cc->AGI < bAGI) {
		LogInfo(" stat AGI is out of range");
		Charerrors++;
	}
	if (cc->DEX > bDEX + stat_points || cc->DEX < bDEX) {
		LogInfo(" stat DEX is out of range");
		Charerrors++;
	}
	if (cc->WIS > bWIS + stat_points || cc->WIS < bWIS) {
		LogInfo(" stat WIS is out of range");
		Charerrors++;
	}
	if (cc->INT > bINT + stat_points || cc->INT < bINT) {
		LogInfo(" stat INT is out of range");
		Charerrors++;
	}
	if (cc->CHA > bCHA + stat_points || cc->CHA < bCHA) {
		LogInfo(" stat CHA is out of range");
		Charerrors++;
	}

	/*TODO: Check for deity/class/race.. it'd be nice, but probably of any real use to hack(faction, deity based items are all I can think of)
	I am NOT writing those tables - kathgar*/

	LogInfo("Found [{}] errors in character creation request", Charerrors);

	return Charerrors == 0;
}

void Client::SetClassStartingSkills(PlayerProfile_Struct *pp)
{
	for (uint32 i = 0; i <= EQ::skills::HIGHEST_SKILL; ++i) {
		if (pp->skills[i] == 0) {
			// Skip specialized, tradeskills (fishing excluded), Alcohol Tolerance, and Bind Wound
			if (EQ::skills::IsSpecializedSkill((EQ::skills::SkillType)i) ||
				(EQ::skills::IsTradeskill((EQ::skills::SkillType)i) && i != EQ::skills::SkillFishing) ||
				i == EQ::skills::SkillAlcoholTolerance || i == EQ::skills::SkillBindWound)
				continue;

			pp->skills[i] = SkillCaps::Instance()->GetSkillCap(pp->class_, (EQ::skills::SkillType)i, 1).cap;
		}
	}

	if (cle->GetClientVersion() < static_cast<uint8>(EQ::versions::ClientVersion::RoF2) && pp->class_ == Class::Berserker) {
		pp->skills[EQ::skills::Skill1HPiercing] = pp->skills[EQ::skills::Skill2HPiercing];
		pp->skills[EQ::skills::Skill2HPiercing] = 0;
	}
}

void Client::SetRaceStartingSkills( PlayerProfile_Struct *pp )
{
	switch( pp->race )
	{
	case Race::Barbarian:
	case Race::Dwarf:
	case Race::Erudite:
	case Race::HalfElf:
	case Race::HighElf:
	case Race::Human:
	case Race::Ogre:
	case Race::Troll:
	case Race::Drakkin:	//Drakkin are supposed to get a starting AA Skill
		{
			// No Race Specific Skills
			break;
		}
	case Race::DarkElf:
		{
			pp->skills[EQ::skills::SkillHide] = 50;
			break;
		}
	case Race::Froglok2:
		{
			if (RuleI(Skills, SwimmingStartValue) < 125) {
				pp->skills[EQ::skills::SkillSwimming] = 125;
			}
			break;
		}
	case Race::Gnome:
		{
			pp->skills[EQ::skills::SkillTinkering] = 50;
			break;
		}
	case Race::Halfling:
		{
			pp->skills[EQ::skills::SkillHide] = 50;
			pp->skills[EQ::skills::SkillSneak] = 50;
			break;
		}
	case Race::Iksar:
		{
			pp->skills[EQ::skills::SkillForage] = 50;
			if (RuleI(Skills, SwimmingStartValue) < 100) {
				pp->skills[EQ::skills::SkillSwimming] = 100;
			}
			break;
		}
	case Race::WoodElf:
		{
			pp->skills[EQ::skills::SkillForage] = 50;
			pp->skills[EQ::skills::SkillHide] = 50;
			break;
		}
	case Race::VahShir:
		{
			pp->skills[EQ::skills::SkillSafeFall] = 50;
			pp->skills[EQ::skills::SkillSneak] = 50;
			break;
		}
	}
}

void Client::SetRacialLanguages( PlayerProfile_Struct *pp )
{
	switch (pp->race) {
		case Race::Human: {
			pp->languages[Language::CommonTongue] = Language::MaxValue;
			break;
		}
		case Race::Barbarian: {
			pp->languages[Language::CommonTongue] = Language::MaxValue;
			pp->languages[Language::Barbarian]    = Language::MaxValue;
			break;
		}
		case Race::Erudite: {
			pp->languages[Language::CommonTongue] = Language::MaxValue;
			pp->languages[Language::Erudian]      = Language::MaxValue;
			break;
		}
		case Race::WoodElf: {
			pp->languages[Language::CommonTongue] = Language::MaxValue;
			pp->languages[Language::Elvish]       = Language::MaxValue;
			break;
		}
		case Race::HighElf: {
			pp->languages[Language::CommonTongue] = Language::MaxValue;
			pp->languages[Language::DarkElvish]   = 25;
			pp->languages[Language::ElderElvish]  = 25;
			pp->languages[Language::Elvish]       = Language::MaxValue;
			break;
		}
		case Race::DarkElf: {
			pp->languages[Language::CommonTongue] = Language::MaxValue;
			pp->languages[Language::DarkElvish]   = Language::MaxValue;
			pp->languages[Language::DarkSpeech]   = Language::MaxValue;
			pp->languages[Language::ElderElvish]  = Language::MaxValue;
			pp->languages[Language::Elvish]       = 25;
			break;
		}
		case Race::HalfElf: {
			pp->languages[Language::CommonTongue] = Language::MaxValue;
			pp->languages[Language::Elvish]       = Language::MaxValue;
			break;
		}
		case Race::Dwarf: {
			pp->languages[Language::CommonTongue] = Language::MaxValue;
			pp->languages[Language::Dwarvish]     = Language::MaxValue;
			pp->languages[Language::Gnomish]      = 25;
			break;
		}
		case Race::Troll: {
			pp->languages[Language::CommonTongue] = RuleI(Character, TrollCommonTongue);
			pp->languages[Language::DarkSpeech]   = Language::MaxValue;
			pp->languages[Language::Troll]        = Language::MaxValue;
			break;
		}
		case Race::Ogre: {
			pp->languages[Language::CommonTongue] = RuleI(Character, OgreCommonTongue);
			pp->languages[Language::DarkSpeech]   = Language::MaxValue;
			pp->languages[Language::Ogre]         = Language::MaxValue;
			break;
		}
		case Race::Halfling: {
			pp->languages[Language::CommonTongue] = Language::MaxValue;
			pp->languages[Language::Halfling]     = Language::MaxValue;
			break;
		}
		case Race::Gnome: {
			pp->languages[Language::CommonTongue] = Language::MaxValue;
			pp->languages[Language::Dwarvish]     = 25;
			pp->languages[Language::Gnomish]      = Language::MaxValue;
			break;
		}
		case Race::Iksar: {
			pp->languages[Language::CommonTongue] = RuleI(Character, IksarCommonTongue);
			pp->languages[Language::DarkSpeech]   = Language::MaxValue;
			pp->languages[Language::Lizardman]    = Language::MaxValue;
			break;
		}
		case Race::VahShir: {
			pp->languages[Language::CommonTongue]  = Language::MaxValue;
			pp->languages[Language::CombineTongue] = Language::MaxValue;
			pp->languages[Language::Erudian]       = 25;
			pp->languages[Language::VahShir]       = Language::MaxValue;
			break;
		}
		case Race::Froglok2: {
			pp->languages[Language::CommonTongue] = Language::MaxValue;
			pp->languages[Language::Froglok]      = Language::MaxValue;
			pp->languages[Language::Troll]        = 25;
			break;
		}
		case Race::Drakkin: {
			pp->languages[Language::CommonTongue] = Language::MaxValue;
			pp->languages[Language::ElderDragon]  = Language::MaxValue;
			pp->languages[Language::Dragon]       = Language::MaxValue;
			break;
		}
		default: {
			break;
		}
	}
}

void Client::SetClassLanguages(PlayerProfile_Struct *pp)
{
	// we only need to handle one class, but custom server might want to do more
	switch (pp->class_) {
		case Class::Rogue:
			pp->languages[Language::ThievesCant] = Language::MaxValue;
			break;
		default:
			break;
	}
}

bool Client::StoreCharacter(
	uint32 account_id,
	PlayerProfile_Struct *p_player_profile_struct,
	EQ::InventoryProfile *p_inventory_profile
)
{
	const uint32 character_id = database.GetCharacterID(p_player_profile_struct->name);
	if (!character_id) {
		return false;
	}

	const std::string& zone_name = ZoneStore::Instance()->GetZoneName(p_player_profile_struct->zone_id, true);
	if (Strings::EqualFold(zone_name, "UNKNOWN")) {
		p_player_profile_struct->zone_id = Zones::QEYNOS;
	}

	database.SaveCharacterCreate(character_id, account_id, p_player_profile_struct);

	std::vector<InventoryRepository::Inventory> v;

	auto e = InventoryRepository::NewEntity();

	e.character_id = character_id;

	for (int16 slot_id = EQ::invslot::EQUIPMENT_BEGIN; slot_id <= EQ::invbag::BANK_BAGS_END;) {
		const auto inst = p_inventory_profile->GetItem(slot_id);
		if (inst) {
			e.slot_id             = slot_id;
			e.item_id             = inst->GetItem()->ID;
			e.charges             = inst->GetCharges();
			e.color               = inst->GetColor();
			e.augment_one         = inst->GetAugmentItemID(EQ::invaug::SOCKET_BEGIN);
			e.augment_two         = inst->GetAugmentItemID(EQ::invaug::SOCKET_BEGIN + 1);
			e.augment_three       = inst->GetAugmentItemID(EQ::invaug::SOCKET_BEGIN + 2);
			e.augment_four        = inst->GetAugmentItemID(EQ::invaug::SOCKET_BEGIN + 3);
			e.augment_five        = inst->GetAugmentItemID(EQ::invaug::SOCKET_BEGIN + 4);
			e.augment_six         = inst->GetAugmentItemID(EQ::invaug::SOCKET_END);
			e.instnodrop          = inst->IsAttuned() ? 1 : 0;
			e.ornament_icon       = inst->GetOrnamentationIcon();
			e.ornament_idfile     = inst->GetOrnamentationIDFile();
			e.ornament_hero_model = inst->GetOrnamentHeroModel();
			e.guid                = inst->GetSerialNumber();

			v.emplace_back(e);
		}

		if (slot_id == EQ::invslot::slotCursor) {
			slot_id = EQ::invbag::GENERAL_BAGS_BEGIN;
			continue;
		} else if (slot_id == EQ::invbag::CURSOR_BAG_END) {
			slot_id = EQ::invslot::BANK_BEGIN;
			continue;
		} else if (slot_id == EQ::invslot::BANK_END) {
			slot_id = EQ::invbag::BANK_BAGS_BEGIN;
			continue;
		}

		slot_id++;
	}

	if (!v.empty()) {
		InventoryRepository::InsertMany(database, v);
	}

	return true;
}

void Client::RecordPossibleHack(const std::string& message)
{
	if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::POSSIBLE_HACK)) {
		auto event = PlayerEvent::PossibleHackEvent{.message = message};
		std::stringstream ss;
		{
			cereal::JSONOutputArchiveSingleLine ar(ss);
			event.serialize(ar);
		}

		auto e = PlayerEventLogsRepository::NewEntity();
		e.character_id    = charid;
		e.account_id      = GetCLE() ? GetAccountID() : 0;
		e.event_type_id   = PlayerEvent::POSSIBLE_HACK;
		e.event_type_name = PlayerEvent::EventName[PlayerEvent::POSSIBLE_HACK];
		e.event_data      = ss.str();
		e.created_at      = std::time(nullptr);
		PlayerEventLogsRepository::InsertOne(database, e);
	}
}

void Client::SendGuildTributeFavorAndTimer(uint32 favor, uint32 time_remaining)
{
	auto cle = GetCLE();
	if (!cle) {
		return;
	}

	auto guild = guild_mgr.GetGuildByGuildID(GetCLE()->GuildID());
	if (guild) {
		guild->tribute.favor = favor;
		guild->tribute.time_remaining = time_remaining;

		auto outapp = new EQApplicationPacket(OP_GuildTributeFavorAndTimer, sizeof(GuildTributeFavorTimer_Struct));
		auto gtsa   = (GuildTributeFavorTimer_Struct *)outapp->pBuffer;

		gtsa->guild_id      = GetCLE()->GuildID();
		gtsa->guild_favor   = guild->tribute.favor;
		gtsa->tribute_timer = guild->tribute.time_remaining;
		gtsa->trophy_timer  = 0; //not yet implemented

		QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Client::SendGuildTributeOptInToggle(const GuildTributeMemberToggle *in)
{
	auto outapp = new EQApplicationPacket(OP_GuildOptInOut, sizeof(GuildTributeOptInOutReply_Struct));
	auto data   = (GuildTributeOptInOutReply_Struct *)outapp->pBuffer;

	data->guild_id              = in->guild_id;
	data->no_donations          = in->no_donations;
	data->tribute_toggle        = in->tribute_toggle;
	data->tribute_trophy_toggle = 0; //not yet implemented
	data->time                  = time(nullptr);
	data->command               = in->command;
	strn0cpy(data->player_name, in->player_name, sizeof(data->player_name));

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendUnsupportedClientPacket(const std::string& message)
{
	EQApplicationPacket packet(OP_SendCharInfo, sizeof(CharacterSelect_Struct) + sizeof(CharacterSelectEntry_Struct));

	unsigned char* buff_ptr = packet.pBuffer;
	auto cs = (CharacterSelect_Struct*) buff_ptr;

	cs->CharCount  = 1;
	cs->TotalChars = 1;

	buff_ptr += sizeof(CharacterSelect_Struct);

	auto e = (CharacterSelectEntry_Struct*) buff_ptr;

	strcpy(e->Name, message.c_str());

	e->Race        = Race::Human;
	e->Class       = Class::Warrior;
	e->Level       = 1;
	e->ShroudClass = e->Class;
	e->ShroudRace  = e->Race;
	e->Zone        = std::numeric_limits<uint16>::max();
	e->Instance    = 0;
	e->Gender      = Gender::Male;
	e->GoHome      = 0;
	e->Tutorial    = 0;
	e->Enabled     = 0;

	QueuePacket(&packet);
}

void Client::LoadDataBucketsCache()
{
	DataBucket::BulkLoadEntitiesToCache(&database, DataBucketLoadType::Account, {GetAccountID()});
	const auto ids = CharacterDataRepository::GetCharacterIDsByAccountID(database, GetAccountID());
	DataBucket::BulkLoadEntitiesToCache(&database, DataBucketLoadType::Client, ids);
}

void Client::ClearDataBucketsCache()
{
	DataBucket::DeleteFromCache(GetAccountID(), DataBucketLoadType::Account);
	auto ids = CharacterDataRepository::GetCharacterIDsByAccountID(database, GetAccountID());
	for (const auto& id : ids) {
		DataBucket::DeleteFromCache(id, DataBucketLoadType::Client);
	}
}
