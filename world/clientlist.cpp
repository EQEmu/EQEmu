/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

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

#include "common/classes.h"
#include "common/event_sub.h"
#include "common/guilds.h"
#include "common/json/json.h"
#include "common/misc_functions.h"
#include "common/misc.h"
#include "common/packet_dump.h"
#include "common/races.h"
#include "common/strings.h"
#include "common/zone_store.h"
#include "world/client.h"
#include "world/clientlist.h"
#include "world/web_interface.h"
#include "world/wguild_mgr.h"
#include "world/worlddb.h"
#include "world/zonelist.h"
#include "world/zoneserver.h"

#include <set>

uint32 numplayers = 0;	//this really wants to be a member variable of ClientList...

ClientList::ClientList()
	: CLStale_timer(10000),
	  m_poll_cache_timer(6000)
{
	NextCLEID = 1;

	m_tick = std::make_unique<EQ::Timer>(5000, true, std::bind(&ClientList::OnTick, this, std::placeholders::_1));

	// pre-allocate / pin memory for the zone server caches
	m_gm_zone_server_ids.reserve(512);
	m_guild_zone_server_ids.reserve(1024);
}

ClientList::~ClientList() {
}

void ClientList::Process() {

	if (CLStale_timer.Check())
		CLCheckStale();

	if (m_poll_cache_timer.Check()) {
		RebuildZoneServerCaches();
	}

	LinkedListIterator<Client*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (!iterator.GetData()->Process()) {
			struct in_addr in;
			in.s_addr = iterator.GetData()->GetIP();
			LogInfo("Removing client from [{}]:[{}]", inet_ntoa(in), iterator.GetData()->GetPort());
			iterator.RemoveCurrent();
		}
		else
			iterator.Advance();
	}
}

void ClientList::CLERemoveZSRef(ZoneServer* iZS) {
	LinkedListIterator<ClientListEntry*> iterator(clientlist);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->Server() == iZS) {
			iterator.GetData()->ClearServer(); // calling this before LeavingZone() makes CLE not update the number of players in a zone
			iterator.GetData()->LeavingZone();
		}
		iterator.Advance();
	}
}

//Check current CLE Entry IPs against incoming connection

void ClientList::GetCLEIP(uint32 in_ip) {
	ClientListEntry* cle = nullptr;
	LinkedListIterator<ClientListEntry*> iterator(clientlist);

	int count = 0;
	iterator.Reset();

	const auto& zones = Strings::Split(RuleS(World, IPExemptionZones), ",");

	while (iterator.MoreElements()) {
		cle = iterator.GetData();

		if (!zones.empty() && cle->zone()) {
			auto it = std::ranges::find_if(
				zones,
				[cle](const auto& z) {
					return Strings::ToUnsignedInt(z) == cle->zone();
				}
			);

			if (it != zones.end()) {
				iterator.Advance();
				continue;
			}
		}

		if (
			cle->GetIP() == in_ip &&
			(
				cle->Admin() < RuleI(World, ExemptMaxClientsStatus) ||
				RuleI(World, ExemptMaxClientsStatus) < 0
			)
		) { // If the IP matches, and the connection admin status is below the exempt status, or exempt status is less than 0 (no-one is exempt)
			auto ip_string = long2ip(cle->GetIP());
			count++; // Increment the occurences of this IP address
			LogClientLogin("Account ID: [{}] Account Name: [{}] IP: [{}]", cle->LSID(), cle->LSName(), ip_string);

			if (RuleB(World, EnableIPExemptions)) {
				LogClientLogin(
					"Account ID: [{}] Account Name: [{}] IP: [{}] IP Instances: [{}] Max IP Instances: [{}]",
					cle->LSID(),
					cle->LSName(),
					ip_string,
					count,
					database.GetIPExemption(ip_string)
				);

				auto exemption_amount = database.GetIPExemption(ip_string);
				if (exemption_amount > 0 && count > exemption_amount) {
					if (RuleB(World, IPLimitDisconnectAll)) {
						LogClientLogin("Disconnect: All Accounts on IP [{}]", ip_string);
						DisconnectByIP(in_ip);
						return;
					} else {
						LogClientLogin("Disconnect: Account [{}] on IP [{}]", cle->LSName(), ip_string);
						cle->SetOnline(CLE_Status::Offline);
						iterator.RemoveCurrent();
						continue;
					}
				}
			} else {
				if (
					RuleI(World, MaxClientsPerIP) > 0 &&
					count > RuleI(World, MaxClientsPerIP)
				) { // If the number of connections exceeds the lower limit
					if (RuleB(World, MaxClientsSetByStatus)) { // If MaxClientsSetByStatus is set to True, override other IP Limit Rules
						LogClientLogin(
							"Account ID: [{}] Account Name: [{}] IP: [{}] IP Instances: [{}] Max IP Instances: [{}]",
							cle->LSID(),
							cle->LSName(),
							ip_string,
							count,
							cle->Admin()
						);

						if (count > cle->Admin()) { // The IP Limit is set by the status of the account if status > MaxClientsPerIP
							if (RuleB(World, IPLimitDisconnectAll)) {
								LogClientLogin("Disconnect: All Accounts on IP [{}]", ip_string);
								DisconnectByIP(in_ip);
								return;
							} else {
								LogClientLogin("Disconnect: Account [{}] on IP [{}]", cle->LSName(), ip_string);
								cle->SetOnline(CLE_Status::Offline); // Remove the connection
								iterator.RemoveCurrent();
								continue;
							}
						}
					} else if (
						cle->Admin() < RuleI(World, AddMaxClientsStatus) ||
						RuleI(World, AddMaxClientsStatus) < 0
					) { // Else if the Admin status of the connection is not eligible for the higher limit, or there is no higher limit (AddMaxClientStatus < 0)
						if (RuleB(World, IPLimitDisconnectAll)) {
							LogClientLogin("Disconnect: All Accounts on IP [{}]", ip_string);
							DisconnectByIP(in_ip);
							return;
						} else {
							LogClientLogin("Disconnect: Account [{}] on IP [{}]", cle->LSName(), ip_string);
							cle->SetOnline(CLE_Status::Offline); // Remove the connection
							iterator.RemoveCurrent();
							continue;
						}
					} else if (
						RuleI(World, AddMaxClientsPerIP) > 0 &&
						count > RuleI(World, AddMaxClientsPerIP)
					) { // else they are eligible for the higher limit, but if they exceed that
						if (RuleB(World, IPLimitDisconnectAll)) {
							LogClientLogin("Disconnect: All Accounts on IP [{}]", ip_string);
							DisconnectByIP(in_ip);
							return;
						} else {
							LogClientLogin("Disconnect: Account [{}] on IP [{}]", cle->LSName(), ip_string);
							cle->SetOnline(CLE_Status::Offline); // Remove the connection
							iterator.RemoveCurrent();
							continue;
						}
					}
				}
			}
		}

		iterator.Advance();
	}
}

void ClientList::DisconnectByIP(uint32 in_ip) {
	ClientListEntry* cle = nullptr;
	LinkedListIterator<ClientListEntry*> iterator(clientlist);
	iterator.Reset();

	while (iterator.MoreElements()) {
		cle = iterator.GetData();
		if (cle->GetIP() == in_ip) {
			if (strlen(cle->name())) {
				auto pack = new ServerPacket(ServerOP_KickPlayer, sizeof(ServerKickPlayer_Struct));
				auto skp = (ServerKickPlayer_Struct*) pack->pBuffer;
				strn0cpy(skp->adminname, "SessionLimit", sizeof(skp->adminname));
				strn0cpy(skp->name, cle->name(), sizeof(skp->name));
				skp->adminrank = 255;
				ZSList::Instance()->SendPacket(pack);
				safe_delete(pack);
			}
			cle->SetOnline(CLE_Status::Offline);
			iterator.RemoveCurrent();
		}
		iterator.Advance();
	}
}

ClientListEntry* ClientList::FindCharacter(const char* name) {
	LinkedListIterator<ClientListEntry*> iterator(clientlist);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (strcasecmp(iterator.GetData()->name(), name) == 0) {
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return nullptr;
}

ClientListEntry* ClientList::FindCLEByAccountID(uint32 iAccID) {
	LinkedListIterator<ClientListEntry*> iterator(clientlist);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->AccountID() == iAccID) {
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return nullptr;
}

ClientListEntry* ClientList::FindCLEByCharacterID(uint32 iCharID) {
	LinkedListIterator<ClientListEntry*> iterator(clientlist);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->CharID() == iCharID) {
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return nullptr;
}

void ClientList::SendCLEList(const int16& admin, const char* to, WorldTCPConnection* connection, const char* search_criteria)
{
	LinkedListIterator<ClientListEntry*> iterator(clientlist);

	int found_count = 0;
	int total_count = 0;
	int name_length = search_criteria ? strlen(search_criteria) : 0;

	const char* new_line = connection->IsConsole() ? "\r\n" : "^";
	bool add_new_line = false;

	std::string message;

	iterator.Reset();

	while (iterator.MoreElements()) {
		ClientListEntry* cle = iterator.GetData();

		struct in_addr in;
		in.s_addr = cle->GetIP();

		if (
			admin >= cle->Admin() &&
			(
				!search_criteria ||
				Strings::Contains(std::string(inet_ntoa(in)), search_criteria) ||
				Strings::Contains(cle->name(), search_criteria) == 0 ||
				Strings::Contains(cle->AccountName(), search_criteria) == 0 ||
				Strings::Contains(cle->LSName(), search_criteria) == 0
			)
		) {
			if (add_new_line) {
				message += new_line;
			}

			message += fmt::format(
				"Account: {} ({}) | IP: {} | Admin: {}",
				cle->AccountName(),
				cle->AccountID(),
				inet_ntoa(in),
				cle->Admin()
			);

			if (cle->CharID()) {
				message += fmt::format(
					"{}Character: {} ({}) | Zone: {} ({})",
					new_line,
					cle->name(),
					cle->CharID(),
					ZoneLongName(cle->zone()),
					cle->zone()
				);
			}

			if (message.size() >= 3072) {
				connection->SendEmoteMessageRaw(
					to,
					0,
					AccountStatus::Player,
					Chat::NPCQuestSay,
					message.c_str()
				);
				message.clear();
				add_new_line = false;
			} else {
				add_new_line = true;
			}

			found_count++;
		}

		iterator.Advance();
		total_count++;
	}

	message += fmt::format(
		"{}{}Player Count: {}",
		new_line,
		(
			found_count != total_count ?
			fmt::format(
				"Total: {} CLE{} | Found: {} CLE{} | ",
				total_count,
				(total_count != 1 ? "s" : ""),
				found_count,
				(found_count != 1 ? "s" : "")
			) :
			""
		),
		numplayers
	);

	connection->SendEmoteMessageRaw(
		to,
		0,
		AccountStatus::Player,
		Chat::NPCQuestSay,
		message.c_str()
	);
}

void ClientList::CLEAdd(
	uint32 login_server_id,
	const char *login_server_name,
	const char *login_name,
	const char *login_key,
	int16 world_admin,
	uint32 ip_address,
	uint8 is_local
)
{
	auto tmp = new ClientListEntry(
		GetNextCLEID(),
		login_server_id,
		login_server_name,
		login_name,
		login_key,
		world_admin,
		ip_address,
		is_local
	);

	clientlist.Append(tmp);
}

void ClientList::CLCheckStale() {
	LinkedListIterator<ClientListEntry*> iterator(clientlist);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->CheckStale()) {
			iterator.RemoveCurrent();
		}
		else
			iterator.Advance();
	}
}

void ClientList::ClientUpdate(ZoneServer *zoneserver, ServerClientList_Struct *scl)
{
	LinkedListIterator<ClientListEntry *> iterator(clientlist);
	ClientListEntry                       *cle;
	iterator.Reset();
	while (iterator.MoreElements()) {
		if (iterator.GetData()->GetID() == scl->wid) {
			cle = iterator.GetData();
			if (scl->remove == 2) {
				cle->LeavingZone(zoneserver, CLE_Status::Offline);
			}
			else if (scl->remove == 1) {
				cle->LeavingZone(zoneserver, CLE_Status::Zoning);
			}
			else {
				cle->Update(zoneserver, scl);
				AddToZoneServerCaches(cle);
			}
			return;
		}
		iterator.Advance();
	}
	if (scl->remove == 2) {
		cle = new ClientListEntry(GetNextCLEID(), zoneserver, scl, CLE_Status::Online);
	}
	else if (scl->remove == 1) {
		cle = new ClientListEntry(GetNextCLEID(), zoneserver, scl, CLE_Status::Zoning);
	}
	else {
		cle = new ClientListEntry(GetNextCLEID(), zoneserver, scl, CLE_Status::InZone);
	}

	LogClientListDetail(
		"[ClientUpdate] "
		" remove [{}]"
		" wid [{}]"
		" IP [{}]"
		" zone [{}]"
		" instance_id [{}]"
		" Admin [{}]"
		" charid [{}]"
		" name [{}]"
		" AccountID [{}]"
		" AccountName [{}]"
		" LSAccountID [{}]"
		" lskey [{}]"
		" race [{}]"
		" class_ [{}]"
		" level [{}]"
		" anon [{}]"
		" tellsoff [{}]"
		" guild_id [{}]"
		" guild_rank [{}]"
		" guild_tribute_opt_in [{}]"
		" LFG [{}]"
		" gm [{}]"
		" ClientVersion [{}]"
		" LFGFromLevel [{}]"
		" LFGToLevel [{}]"
		" LFGMatchFilter [{}]"
		" LFGComments [{}]",
		scl->remove,
		scl->wid,
		scl->IP,
		scl->zone,
		scl->instance_id,
		scl->Admin,
		scl->charid,
		scl->name,
		scl->AccountID,
		scl->AccountName,
		scl->LSAccountID,
		scl->lskey,
		scl->race,
		scl->class_,
		scl->level,
		scl->anon,
		scl->tellsoff,
		scl->guild_id,
		scl->guild_rank,
		scl->guild_tribute_opt_in,
		scl->LFG,
		scl->gm,
		scl->ClientVersion,
		scl->LFGFromLevel,
		scl->LFGToLevel,
		scl->LFGMatchFilter,
		scl->LFGComments
	);

	clientlist.Insert(cle);
	AddToZoneServerCaches(cle);
	zoneserver->ChangeWID(scl->charid, cle->GetID());
}

void ClientList::CLEKeepAlive(uint32 numupdates, uint32* wid) {
	LinkedListIterator<ClientListEntry*> iterator(clientlist);
	uint32 i;

	iterator.Reset();
	while(iterator.MoreElements()) {
		for (i=0; i<numupdates; i++) {
			if (wid[i] == iterator.GetData()->GetID())
				iterator.GetData()->KeepAlive();
		}
		iterator.Advance();
	}
}

ClientListEntry *ClientList::CheckAuth(uint32 loginserver_account_id, const char *key)
{
	LinkedListIterator<ClientListEntry *> iterator(clientlist);

	iterator.Reset();
	while (iterator.MoreElements()) {
		if (iterator.GetData()->CheckAuth(loginserver_account_id, key)) {
			return iterator.GetData();
		}
		iterator.Advance();
	}

	return nullptr;
}

void ClientList::SendOnlineGuildMembers(uint32 FromID, uint32 GuildID)
{
	int PacketLength = 8;

	uint32 Count = 0;
	ClientListEntry* from = FindCLEByCharacterID(FromID);

	if(!from)
	{
		LogInfo("Invalid client. FromID=[{}] GuildID=[{}]", FromID, GuildID);
		return;
	}

	LinkedListIterator<ClientListEntry*> Iterator(clientlist);

	Iterator.Reset();

	while(Iterator.MoreElements())
	{
		ClientListEntry* CLE = Iterator.GetData();

		if(CLE && (CLE->GuildID() == GuildID))
		{
			PacketLength += (strlen(CLE->name()) + 5);
			++Count;
		}

		Iterator.Advance();

	}

	Iterator.Reset();

	auto pack = new ServerPacket(ServerOP_OnlineGuildMembersResponse, PacketLength);

	char *Buffer = (char *)pack->pBuffer;

	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, FromID);
	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, Count);

	while(Iterator.MoreElements())
	{
		ClientListEntry* CLE = Iterator.GetData();

		if(CLE && (CLE->GuildID() == GuildID))
		{
			VARSTRUCT_ENCODE_STRING(Buffer, CLE->name());
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, CLE->zone());
		}

		Iterator.Advance();
	}
	ZSList::Instance()->SendPacket(from->zone(), from->instance(), pack);
	safe_delete(pack);
}

void ClientList::SendWhoAll(uint32 fromid,const char* to, int16 admin, Who_All_Struct* whom, WorldTCPConnection* connection) {
	try {
		LinkedListIterator<ClientListEntry*> iterator(clientlist);
		LinkedListIterator<ClientListEntry*> countclients(clientlist);
		ClientListEntry* cle = 0;
		ClientListEntry* countcle = 0;
		//char tmpgm[25] = "";
		//char accinfo[150] = "";
		char line[300] = "";
		//char tmpguild[50] = "";
		//char LFG[10] = "";
		//uint32 x = 0;
		int whomlen = 0;

		if (whom) {
			// fixes for client converting some queries into a race query instead of zone
			if (whom->wrace == 221) {
				whom->wrace = 0xFFFF;
				strcpy(whom->whom, "scarlet");
			}

			if (whom->wrace == 327) {
				whom->wrace = 0xFFFF;
				strcpy(whom->whom, "crystal");
			}

			if (whom->wrace == 103) {
				whom->wrace = 0xFFFF;
				strcpy(whom->whom, "kedge");
			}

			if (whom->wrace == 230) {
				whom->wrace = 0xFFFF;
				strcpy(whom->whom, "akheva");
			}

			if (whom->wrace == 229) {
				whom->wrace = 0xFFFF;
				strcpy(whom->whom, "netherbian");
			}

			whomlen = strlen(whom->whom);

			if (whom->wrace == 0x001A) { // 0x001A is the old Froglok race number and is sent by the client for /who all froglok
				whom->wrace = Race::Froglok2; // This is what EQEmu uses for the Froglok Race number.
			}
		}

		uint32 totalusers=0;
		uint32 totallength=0;
		countclients.Reset();
		while (countclients.MoreElements()) {
			countcle = countclients.GetData();
			const char* tmpZone = ZoneName(countcle->zone());
			if (
				(countcle->Online() >= CLE_Status::Zoning) &&
				(!countcle->GetGM() || countcle->Anon() != 1 || admin >= countcle->Admin()) &&
				(whom == 0 || (
					((countcle->Admin() >= AccountStatus::QuestTroupe && countcle->GetGM()) || whom->gmlookup == 0xFFFF) &&
					(whom->lvllow == 0xFFFF ||
									(countcle->level() >= whom->lvllow && countcle->level() <= whom->lvlhigh &&
									(countcle->Anon() == 0 || admin > countcle->Admin()))) &&
									(whom->wclass == 0xFFFF || (countcle->class_() == whom->wclass &&
																(countcle->Anon() == 0 || admin > countcle->Admin()))) &&
									(whom->wrace == 0xFFFF ||
									(countcle->race() == whom->wrace && (countcle->Anon() == 0 || admin > countcle->Admin()))) &&
					(whomlen == 0 || (
						(tmpZone != 0 && strncasecmp(tmpZone, whom->whom, whomlen) == 0) ||
						strncasecmp(countcle->name(),whom->whom, whomlen) == 0 ||
						(strncasecmp(guild_mgr.GetGuildName(countcle->GuildID()), whom->whom, whomlen) == 0) ||
						(admin >= AccountStatus::GMAdmin && strncasecmp(countcle->AccountName(), whom->whom, whomlen) == 0)
					))
				))
			) {
				// these blocks can all be condensed but it's simpler to conceptualize this way
				if ((countcle->Anon()>0 && admin >= countcle->Admin() && admin > AccountStatus::Player) || countcle->Anon()==0 ) {
					totalusers++;
					if (totalusers<=20 || admin >= AccountStatus::GMAdmin) {
						totallength = totallength + strlen(countcle->name()) + strlen(countcle->AccountName()) +
									strlen(guild_mgr.GetGuildName(countcle->GuildID())) + 5;
					}
				} else if (((countcle->Anon() == 1 && admin <= countcle->Admin()) && whomlen != 0 &&
							strncasecmp(countcle->name(), whom->whom, whomlen) == 0)) {
					totalusers++;
					if (totalusers <= 20 || admin >= AccountStatus::GMAdmin) {
						totallength = totallength + strlen(countcle->name()) + strlen(countcle->AccountName()) +
									strlen(guild_mgr.GetGuildName(countcle->GuildID())) + 5;
					}
				} else if (((countcle->Anon() == 2 && admin <= countcle->Admin()) && whomlen != 0 &&
							(strncasecmp(countcle->name(), whom->whom, whomlen) == 0 ||
							strncasecmp(guild_mgr.GetGuildName(countcle->GuildID()), whom->whom, whomlen) == 0))) {
					totalusers++;
					if (totalusers <= 20 || admin >= AccountStatus::GMAdmin) {
						totallength = totallength + strlen(countcle->name()) + strlen(countcle->AccountName()) +
									strlen(guild_mgr.GetGuildName(countcle->GuildID())) + 5;
					}
				}
			}
			countclients.Advance();
		}

		uint32 plid=fromid;
		uint32 playerineqstring=5001;
		const char line2[]="---------------------------";
		uint8 unknown35=0x0A;
		uint32 unknown36=0;
		uint32 playersinzonestring=5028;

		if (totalusers>20 && admin<AccountStatus::GMAdmin) {
			totalusers=20;
			playersinzonestring=5033;
		} else if(totalusers>1) {
			playersinzonestring=5036;
		}

		uint32 unknown44[2];
		unknown44[0]=0;
		unknown44[1]=0;
		uint32 unknown52=totalusers;
		uint32 unknown56=1;
		auto pack2 = new ServerPacket(ServerOP_WhoAllReply, 64 + totallength + (49 * totalusers));
		memset(pack2->pBuffer,0,pack2->size);
		uchar *buffer=pack2->pBuffer;
		uchar *bufptr=buffer;
		//memset(buffer,0,pack2->size);
		memcpy(bufptr,&plid, sizeof(uint32));
		bufptr+=sizeof(uint32);
		memcpy(bufptr,&playerineqstring, sizeof(uint32));
		bufptr+=sizeof(uint32);
		memcpy(bufptr,&line2, strlen(line2));
		bufptr+=strlen(line2);
		memcpy(bufptr,&unknown35, sizeof(uint8));
		bufptr+=sizeof(uint8);
		memcpy(bufptr,&unknown36, sizeof(uint32));
		bufptr+=sizeof(uint32);
		memcpy(bufptr,&playersinzonestring, sizeof(uint32));
		bufptr+=sizeof(uint32);
		memcpy(bufptr,&unknown44[0], sizeof(uint32));
		bufptr+=sizeof(uint32);
		memcpy(bufptr,&unknown44[1], sizeof(uint32));
		bufptr+=sizeof(uint32);
		memcpy(bufptr,&unknown52, sizeof(uint32));
		bufptr+=sizeof(uint32);
		memcpy(bufptr,&unknown56, sizeof(uint32));
		bufptr+=sizeof(uint32);
		memcpy(bufptr,&totalusers, sizeof(uint32));
		bufptr+=sizeof(uint32);

		iterator.Reset();
		int idx=-1;
		while(iterator.MoreElements()) {
			cle = iterator.GetData();
			const char* tmpZone = ZoneName(cle->zone());

			if (
				(cle->Online() >= CLE_Status::Zoning) &&
				(!cle->GetGM() || cle->Anon() != 1 || admin >= cle->Admin()) &&
				(whom == 0 || (
					((cle->Admin() >= AccountStatus::QuestTroupe && cle->GetGM()) || whom->gmlookup == 0xFFFF) &&
					(whom->lvllow == 0xFFFF || (cle->level() >= whom->lvllow && cle->level() <= whom->lvlhigh && (cle->Anon()==0 || admin>cle->Admin()))) &&
					(whom->wclass == 0xFFFF || (cle->class_() == whom->wclass && (cle->Anon()==0 || admin>cle->Admin()))) &&
					(whom->wrace == 0xFFFF || (cle->race() == whom->wrace && (cle->Anon()==0 || admin>cle->Admin()))) &&
					(whomlen == 0 || (
						(tmpZone != 0 && strncasecmp(tmpZone, whom->whom, whomlen) == 0) ||
						strncasecmp(cle->name(),whom->whom, whomlen) == 0 ||
						(strncasecmp(guild_mgr.GetGuildName(cle->GuildID()), whom->whom, whomlen) == 0) ||
						(admin >= AccountStatus::GMAdmin && strncasecmp(cle->AccountName(), whom->whom, whomlen) == 0)
					))
				))
			) {
				line[0] = 0;
				uint32 rankstring = 0xFFFFFFFF;
				// These lines can be simplified but easier to conceptualize this way
				if ((cle->Anon()==1 && cle->GetGM() && cle->Admin()>admin) || (idx>=20 && admin < AccountStatus::GMAdmin)) { //hide gms that are anon from lesser gms and normal players, cut off at 20
					rankstring = 0;
					iterator.Advance();
					continue;
				} else if (cle->Anon() == 1 && cle->Admin()>=admin && (whomlen == 0 || (whomlen !=0 && strncasecmp(cle->name(), whom->whom, whomlen) != 0))) {
					rankstring = 0;
					iterator.Advance();
					continue;
				} else if (cle->Anon() == 2 && cle->Admin()>=admin && (whomlen == 0 || (whomlen !=0 && strncasecmp(cle->name(), whom->whom, whomlen) != 0 && strncasecmp(guild_mgr.GetGuildName(cle->GuildID()), whom->whom, whomlen) != 0))) {
					rankstring = 0;
					iterator.Advance();
					continue;
				} else if (cle->GetGM()) {
					if (cle->Admin() >= AccountStatus::GMImpossible) {
						rankstring = 5021;
					} else if (cle->Admin() >= AccountStatus::GMMgmt) {
						rankstring = 5020;
					} else if (cle->Admin() >= AccountStatus::GMCoder) {
						rankstring = 5019;
					} else if (cle->Admin() >= AccountStatus::GMAreas) {
						rankstring = 5018;
					} else if (cle->Admin() >= AccountStatus::QuestMaster) {
						rankstring = 5017;
					} else if (cle->Admin() >= AccountStatus::GMLeadAdmin) {
						rankstring = 5016;
					} else if (cle->Admin() >= AccountStatus::GMAdmin) {
						rankstring = 5015;
					} else if (cle->Admin() >= AccountStatus::GMStaff) {
						rankstring = 5014;
					} else if (cle->Admin() >= AccountStatus::EQSupport) {
						rankstring = 5013;
					} else if (cle->Admin() >= AccountStatus::GMTester) {
						rankstring = 5012;
					} else if (cle->Admin() >= AccountStatus::SeniorGuide) {
						rankstring = 5011;
					} else if (cle->Admin() >= AccountStatus::QuestTroupe) {
						rankstring = 5010;
					} else if (cle->Admin() >= AccountStatus::Guide) {
						rankstring = 5009;
					} else if (cle->Admin() >= AccountStatus::ApprenticeGuide) {
						rankstring = 5008;
					} else if (cle->Admin() >= AccountStatus::Steward) {
						rankstring = 5007;
					}
				}

				idx++;
				char guildbuffer[67]={0};

				if (cle->GuildID() != GUILD_NONE && cle->GuildID()>0) {
					sprintf(guildbuffer,"<%s>", guild_mgr.GetGuildName(cle->GuildID()));
				}

				uint32 formatstring=5025;

				if (cle->Anon()==1 && (admin<cle->Admin() || admin == AccountStatus::Player)) {
					formatstring=5024;
				} else if(cle->Anon()==1 && admin>=cle->Admin() && admin > AccountStatus::Player) {
					formatstring=5022;
				} else if(cle->Anon()==2 && (admin<cle->Admin() || admin == AccountStatus::Player)) {
					formatstring=5023;//display guild
				} else if(cle->Anon()==2 && admin>=cle->Admin() && admin > AccountStatus::Player) {
					formatstring=5022;//display everything
				}

				//war* wars2 = (war*)pack2->pBuffer;

				uint32 plclass_=0;
				uint32 pllevel=0;
				uint32 pidstring=0xFFFFFFFF;//5003;
				uint32 plrace=0;
				uint32 zonestring=0xFFFFFFFF;
				uint32 plzone=0;
				uint32 unknown80[2];

				if (cle->Anon()==0 || (admin>=cle->Admin() && admin> AccountStatus::Player)) {
					plclass_=cle->class_();
					pllevel=cle->level();

					if(admin>=AccountStatus::GMAdmin) {
						pidstring=5003;
					}
					plrace=cle->race();
					zonestring=5006;
					plzone=cle->zone();
				}

				if (admin>=cle->Admin() && admin > AccountStatus::Player) {
					unknown80[0]=cle->Admin();
				} else {
					unknown80[0]=0xFFFFFFFF;
				}

				unknown80[1]=0xFFFFFFFF;//1035

				//char plstatus[20]={0};
				//sprintf(plstatus, "Status %i",cle->Admin());
				char plname[64]={0};
				strcpy(plname,cle->name());

				char placcount[30]={0};
				if (admin>=cle->Admin() && admin > AccountStatus::Player) {
					strcpy(placcount,cle->AccountName());
				}

				memcpy(bufptr,&formatstring, sizeof(uint32));
				bufptr+=sizeof(uint32);
				memcpy(bufptr,&pidstring, sizeof(uint32));
				bufptr+=sizeof(uint32);
				memcpy(bufptr,&plname, strlen(plname)+1);
				bufptr+=strlen(plname)+1;
				memcpy(bufptr,&rankstring, sizeof(uint32));
				bufptr+=sizeof(uint32);
				memcpy(bufptr,&guildbuffer, strlen(guildbuffer)+1);
				bufptr+=strlen(guildbuffer)+1;
				memcpy(bufptr,&unknown80[0], sizeof(uint32));
				bufptr+=sizeof(uint32);
				memcpy(bufptr,&unknown80[1], sizeof(uint32));
				bufptr+=sizeof(uint32);
				memcpy(bufptr,&zonestring, sizeof(uint32));
				bufptr+=sizeof(uint32);
				memcpy(bufptr,&plzone, sizeof(uint32));
				bufptr+=sizeof(uint32);
				memcpy(bufptr,&plclass_, sizeof(uint32));
				bufptr+=sizeof(uint32);
				memcpy(bufptr,&pllevel, sizeof(uint32));
				bufptr+=sizeof(uint32);
				memcpy(bufptr,&plrace, sizeof(uint32));
				bufptr+=sizeof(uint32);
				uint32 ending=0;
				memcpy(bufptr,&placcount, strlen(placcount)+1);
				bufptr+=strlen(placcount)+1;
				ending=207;
				memcpy(bufptr,&ending, sizeof(uint32));
				bufptr+=sizeof(uint32);
			}
			iterator.Advance();
		}

		SendPacket(to,pack2);
		safe_delete(pack2);
	} catch(...) {
		LogInfo("Unknown error in world's SendWhoAll (probably mem error), ignoring");
		return;
	}
}

void ClientList::SendFriendsWho(ServerFriendsWho_Struct *FriendsWho, WorldTCPConnection* connection) {

	std::vector<ClientListEntry*> FriendsCLEs;
	FriendsCLEs.reserve(100);

	char Friend_[65];

	char *FriendsPointer = FriendsWho->FriendsString;

	// FriendsString is a comma delimited list of names.

	char *Seperator = nullptr;

	Seperator = strchr(FriendsPointer, ',');
	if(!Seperator) Seperator = strchr(FriendsPointer, '\0');

	uint32 TotalLength=0;

	while(Seperator != nullptr) {

		if((Seperator - FriendsPointer) > 64) return;

		strncpy(Friend_, FriendsPointer, Seperator - FriendsPointer);
		Friend_[Seperator - FriendsPointer] = 0;

		ClientListEntry* CLE = FindCharacter(Friend_);
		if(CLE && CLE->name() && (CLE->Online() >= CLE_Status::Zoning) && !(CLE->GetGM() && CLE->Anon())) {
			FriendsCLEs.push_back(CLE);
			TotalLength += strlen(CLE->name());
			int GuildNameLength = strlen(guild_mgr.GetGuildName(CLE->GuildID()));
			if(GuildNameLength>0)
				TotalLength += (GuildNameLength + 2);
		}

		if(Seperator[0] == '\0') break;

		FriendsPointer = Seperator + 1;
		Seperator = strchr(FriendsPointer, ',');
		if(!Seperator) Seperator = strchr(FriendsPointer, '\0');
	}


	try{
		ClientListEntry* cle;
		int FriendsOnline = FriendsCLEs.size();
		int PacketLength = sizeof(WhoAllReturnStruct) + (47 * FriendsOnline) + TotalLength;
		auto pack2 = new ServerPacket(ServerOP_WhoAllReply, PacketLength);
		memset(pack2->pBuffer,0,pack2->size);
		uchar *buffer=pack2->pBuffer;
		uchar *bufptr=buffer;

		WhoAllReturnStruct *WARS = (WhoAllReturnStruct *)bufptr;

		WARS->id = FriendsWho->FromID;
		WARS->playerineqstring = 0xffffffff;
		strcpy(WARS->line, "");
		WARS->unknown35 = 0x0a;
		WARS->unknown36 = 0x00;

		if(FriendsCLEs.size() == 1)
			WARS->playersinzonestring = 5028; // 5028 There is %1 player in EverQuest.
		else
			WARS->playersinzonestring = 5036; // 5036 There are %1 players in EverQuest.

		WARS->unknown44[0] = 0;
		WARS->unknown44[1] = 0;
		WARS->unknown52 = FriendsOnline;
		WARS->unknown56 = 1;
		WARS->playercount = FriendsOnline;

		bufptr+=sizeof(WhoAllReturnStruct);

		for(int CLEEntry = 0; CLEEntry < FriendsOnline; CLEEntry++) {

			cle = FriendsCLEs[CLEEntry];

			char GuildName[67]={0};
			if (cle->GuildID() != GUILD_NONE && cle->GuildID()>0)
				sprintf(GuildName,"<%s>", guild_mgr.GetGuildName(cle->GuildID()));
			uint32 FormatMSGID=5025; // 5025 %T1[%2 %3] %4 (%5) %6 %7 %8 %9
			if(cle->Anon()==1)
				FormatMSGID=5024; // 5024 %T1[ANONYMOUS] %2 %3
			else if(cle->Anon()==2)
				FormatMSGID=5023; // 5023 %T1[ANONYMOUS] %2 %3 %4

			uint32 PlayerClass=0;
			uint32 PlayerLevel=0;
			uint32 PlayerRace=0;
			uint32 ZoneMSGID=0xffffffff;
			uint32 PlayerZone=0;

			if(cle->Anon()==0) {
				PlayerClass=cle->class_();
				PlayerLevel=cle->level();
				PlayerRace=cle->race();
				ZoneMSGID=5006; // 5006 ZONE: %1
				PlayerZone=cle->zone();
			}

			char PlayerName[64]={0};
			strcpy(PlayerName,cle->name());

			WhoAllPlayerPart1* WAPP1 = (WhoAllPlayerPart1*)bufptr;

			WAPP1->FormatMSGID = FormatMSGID;
			WAPP1->PIDMSGID = 0xffffffff;
			strcpy(WAPP1->Name, PlayerName);

			bufptr += sizeof(WhoAllPlayerPart1) + strlen(PlayerName);
			WhoAllPlayerPart2* WAPP2 = (WhoAllPlayerPart2*)bufptr;

			WAPP2->RankMSGID = 0xffffffff;
			strcpy(WAPP2->Guild, GuildName);

			bufptr += sizeof(WhoAllPlayerPart2) + strlen(GuildName);
			WhoAllPlayerPart3* WAPP3 = (WhoAllPlayerPart3*)bufptr;

			WAPP3->Unknown80[0] = 0xffffffff;
			WAPP3->Unknown80[1] = 0xffffffff;
			WAPP3->ZoneMSGID = ZoneMSGID;
			WAPP3->Zone = PlayerZone;
			WAPP3->Class_ = PlayerClass;
			WAPP3->Level = PlayerLevel;
			WAPP3->Race = PlayerRace;
			WAPP3->Account[0] = 0;

			bufptr += sizeof(WhoAllPlayerPart3);

			WhoAllPlayerPart4* WAPP4 = (WhoAllPlayerPart4*)bufptr;
			WAPP4->Unknown100 = 207;

			bufptr += sizeof(WhoAllPlayerPart4);

		}
		SendPacket(FriendsWho->FromName,pack2);
		safe_delete(pack2);
	}
	catch(...){
		LogInfo("Unknown error in world's SendFriendsWho (probably mem error), ignoring");
		return;
	}
}

void ClientList::SendLFGMatches(ServerLFGMatchesRequest_Struct *smrs) {

	// Send back matches when someone searches player's Looking For A Group.

	LinkedListIterator<ClientListEntry*> Iterator(clientlist);
	ClientListEntry* CLE = 0;
	int Matches = 0;

	Iterator.Reset();

	// We run the ClientList twice. The first time is to determine how big the outgoing packet needs to be.
	while(Iterator.MoreElements()) {
		CLE = Iterator.GetData();
		if(CLE->LFG()) {
			unsigned int BitMask = 1 << CLE->class_();
			// First we check that the player meets the level and class criteria of the person
			// doing the search.
			if((CLE->level() >= smrs->FromLevel) && (CLE->level() <= smrs->ToLevel) &&
				(BitMask & smrs->Classes))
				// Then we check if if the player doing the search meets the level criteria specified
				// by the player who is LFG.
				//
				// GetLFGMatchFilter returns the setting of the 'Only players who match my posted filters
				//						can query me' checkbox.
				//
				// FromLevel and ToLevel are the settings of the 'Want group levels:' boxes.
				if(!CLE->GetLFGMatchFilter() || ((smrs->QuerierLevel >= CLE->GetLFGFromLevel()) &&
								(smrs->QuerierLevel <= CLE->GetLFGToLevel())))
					Matches++;
		}
		Iterator.Advance();
	}
	auto Pack = new ServerPacket(ServerOP_LFGMatches, (sizeof(ServerLFGMatchesResponse_Struct) * Matches) + 4);

	char *Buf = (char *)Pack->pBuffer;
	// FromID is the Entity ID of the player doing the search.
	VARSTRUCT_ENCODE_TYPE(uint32, Buf, smrs->FromID);

	ServerLFGMatchesResponse_Struct* Buffer = (ServerLFGMatchesResponse_Struct*)Buf;

	Iterator.Reset();

	if(Matches) {
		while(Iterator.MoreElements() && (Matches > 0)) {
			CLE = Iterator.GetData();
			if(CLE->LFG()) {
				unsigned int BitMask = 1 << CLE->class_();
				if((CLE->level() >= smrs->FromLevel) && (CLE->level() <= smrs->ToLevel) &&
					(BitMask & smrs->Classes)) {
					Matches--;
					strcpy(Buffer->Name, CLE->name());
					Buffer->Class_ = CLE->class_();
					Buffer->Level = CLE->level();
					Buffer->Zone = CLE->zone();
					// If the LFG player is anon, level and class are still displayed, but
					// zone shows as UNAVAILABLE.
					Buffer->Anon = (CLE->Anon() != 0);
					// The client can filter on Guildname
					Buffer->GuildID = CLE->GuildID();
					strcpy(Buffer->Comments, CLE->GetLFGComments());
					Buffer++;
				}
			}
			Iterator.Advance();
		}
	}
	SendPacket(smrs->FromName,Pack);
	safe_delete(Pack);
}

void ClientList::ConsoleSendWhoAll(const char* to, int16 admin, Who_All_Struct* whom, WorldTCPConnection* connection) {
	LinkedListIterator<ClientListEntry*> iterator(clientlist);
	ClientListEntry* cle = 0;
	char tmpgm[25] = "";
	char accinfo[150] = "";
	char line[300] = "";
	char tmpguild[50] = "";
	char LFG[10] = "";
	uint32 x = 0;
	int whomlen = 0;
	if (whom)
		whomlen = strlen(whom->whom);

	auto out = fmt::memory_buffer();
	fmt::format_to(std::back_inserter(out), "Players on server:");
	if (connection->IsConsole())
		fmt::format_to(std::back_inserter(out), "\r\n");
	else
		fmt::format_to(std::back_inserter(out), "\n");
	iterator.Reset();
	while (iterator.MoreElements()) {
		cle = iterator.GetData();
		const char* tmpZone = ZoneName(cle->zone());
		if (
			(cle->Online() >= CLE_Status::Zoning)
			&& (whom == 0 || (
				((cle->Admin() >= AccountStatus::QuestTroupe && cle->GetGM()) || whom->gmlookup == 0xFFFF) &&
				(whom->lvllow == 0xFFFF || (cle->level() >= whom->lvllow && cle->level() <= whom->lvlhigh)) &&
				(whom->wclass == 0xFFFF || cle->class_() == whom->wclass) &&
				(whom->wrace == 0xFFFF || cle->race() == whom->wrace) &&
				(whomlen == 0 || (
					(tmpZone != 0 && strncasecmp(tmpZone, whom->whom, whomlen) == 0) ||
					strncasecmp(cle->name(), whom->whom, whomlen) == 0 ||
					(strncasecmp(guild_mgr.GetGuildName(cle->GuildID()), whom->whom, whomlen) == 0) ||
					(admin >= AccountStatus::GMAdmin && strncasecmp(cle->AccountName(), whom->whom, whomlen) == 0)
					))
				))
			) {
			line[0] = 0;
			// MYRA - use new (5.x) Status labels in who for telnet connection
			if (cle->Admin() >= AccountStatus::GMImpossible)
				strcpy(tmpgm, "* GM-Impossible * ");
			else if (cle->Admin() >= AccountStatus::GMMgmt)
				strcpy(tmpgm, "* GM-Mgmt * ");
			else if (cle->Admin() >= AccountStatus::GMCoder)
				strcpy(tmpgm, "* GM-Coder * ");
			else if (cle->Admin() >= AccountStatus::GMAreas)
				strcpy(tmpgm, "* GM-Areas * ");
			else if (cle->Admin() >= AccountStatus::QuestMaster)
				strcpy(tmpgm, "* QuestMaster * ");
			else if (cle->Admin() >= AccountStatus::GMLeadAdmin)
				strcpy(tmpgm, "* GM-Lead Admin * ");
			else if (cle->Admin() >= AccountStatus::GMAdmin)
				strcpy(tmpgm, "* GM-Admin * ");
			else if (cle->Admin() >= AccountStatus::GMStaff)
				strcpy(tmpgm, "* GM-Staff * ");
			else if (cle->Admin() >= AccountStatus::EQSupport)
				strcpy(tmpgm, "* EQ Support * ");
			else if (cle->Admin() >= AccountStatus::GMTester)
				strcpy(tmpgm, "* GM-Tester * ");
			else if (cle->Admin() >= AccountStatus::SeniorGuide)
				strcpy(tmpgm, "* Senior Guide * ");
			else if (cle->Admin() >= AccountStatus::QuestTroupe)
				strcpy(tmpgm, "* QuestTroupe * ");
			else if (cle->Admin() >= AccountStatus::Guide)
				strcpy(tmpgm, "* Guide * ");
			else if (cle->Admin() >= AccountStatus::ApprenticeGuide)
				strcpy(tmpgm, "* Apprentice Guide * ");
			else if (cle->Admin() >= AccountStatus::Steward)
				strcpy(tmpgm, "* Steward * ");
			else
				tmpgm[0] = 0;
			// end Myra

			if (guild_mgr.GuildExists(cle->GuildID())) {
				snprintf(tmpguild, 36, " <%s>", guild_mgr.GetGuildName(cle->GuildID()));
			}
			else
				tmpguild[0] = 0;

			if (cle->LFG())
				strcpy(LFG, " LFG");
			else
				LFG[0] = 0;

			if (admin >= AccountStatus::GMLeadAdmin && admin >= cle->Admin()) {
				sprintf(accinfo, " AccID: %i AccName: %s LSID: %i Status: %i", cle->AccountID(), cle->AccountName(), cle->LSAccountID(), cle->Admin());
			}
			else
				accinfo[0] = 0;

			if (cle->Anon() == 2) { // Roleplay
				if (admin >= AccountStatus::GMAdmin && admin >= cle->Admin())
					sprintf(line, "  %s[RolePlay %i %s] %s (%s)%s zone: %s%s%s", tmpgm, cle->level(), GetClassIDName(cle->class_(), cle->level()), cle->name(), GetRaceIDName(cle->race()), tmpguild, tmpZone, LFG, accinfo);
				else if (cle->Admin() >= AccountStatus::QuestTroupe && admin < AccountStatus::QuestTroupe && cle->GetGM()) {
					iterator.Advance();
					continue;
				}
				else
					sprintf(line, "  %s[ANONYMOUS] %s%s%s%s", tmpgm, cle->name(), tmpguild, LFG, accinfo);
			}
			else if (cle->Anon() == 1) { // Anon
				if (admin >= AccountStatus::GMAdmin && admin >= cle->Admin())
					sprintf(line, "  %s[ANON %i %s] %s (%s)%s zone: %s%s%s", tmpgm, cle->level(), GetClassIDName(cle->class_(), cle->level()), cle->name(), GetRaceIDName(cle->race()), tmpguild, tmpZone, LFG, accinfo);
				else if (cle->Admin() >= AccountStatus::QuestTroupe && cle->GetGM()) {
					iterator.Advance();
					continue;
				}
				else
					sprintf(line, "  %s[ANONYMOUS] %s%s%s", tmpgm, cle->name(), LFG, accinfo);
			}
			else
				sprintf(line, "  %s[%i %s] %s (%s)%s zone: %s%s%s", tmpgm, cle->level(), GetClassIDName(cle->class_(), cle->level()), cle->name(), GetRaceIDName(cle->race()), tmpguild, tmpZone, LFG, accinfo);

			fmt::format_to(std::back_inserter(out), fmt::runtime(line));
			if (out.size() >= 3584) {
				connection->SendEmoteMessageRaw(
					to,
					0,
					AccountStatus::Player,
					Chat::NPCQuestSay,
					out.data()
				);
				out.clear();
			}
			else {
				if (connection->IsConsole())
					fmt::format_to(std::back_inserter(out), "\r\n");
				else
					fmt::format_to(std::back_inserter(out), "\n");
			}
			x++;
			if (x >= 20 && admin < AccountStatus::QuestTroupe)
				break;
		}
		iterator.Advance();
	}

	if (x >= 20 && admin < AccountStatus::QuestTroupe)
		fmt::format_to(std::back_inserter(out), "too many results...20 players shown");
	else
		fmt::format_to(std::back_inserter(out), "{} players online", x);
	if (admin >= AccountStatus::GMAdmin && (whom == 0 || whom->gmlookup != 0xFFFF)) {
		if (connection->IsConsole())
			fmt::format_to(std::back_inserter(out), "\r\n");
		else
			fmt::format_to(std::back_inserter(out), "\n");

		//console_list.SendConsoleWho(connection, to, admin, &output, &outsize, &outlen);
	}

	connection->SendEmoteMessageRaw(
		to,
		0,
		AccountStatus::Player,
		Chat::NPCQuestSay,
		out.data()
	);
}

void ClientList::Add(Client* client) {
	list.Insert(client);
}

Client* ClientList::FindByAccountID(uint32 account_id) {
	LinkedListIterator<Client*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->GetAccountID() == account_id) {
			Client* tmp = iterator.GetData();
			return tmp;
		}
		iterator.Advance();
	}
	return 0;
}

Client* ClientList::Get(uint32 ip, uint16 port) {
	LinkedListIterator<Client*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->GetIP() == ip && iterator.GetData()->GetPort() == port)
		{
			Client* tmp = iterator.GetData();
			return tmp;
		}
		iterator.Advance();
	}
	return 0;
}

void ClientList::ZoneBootup(ZoneServer* zs) {
	LinkedListIterator<Client*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->WaitingForBootup()) {
			if (iterator.GetData()->GetZoneID() == zs->GetZoneID()
				&& iterator.GetData()->GetInstanceID() == zs->GetInstanceID()) {
				iterator.GetData()->EnterWorld(false);
			}
			else if (iterator.GetData()->WaitingForBootup() == zs->GetID()) {
				iterator.GetData()->TellClientZoneUnavailable();
			}
		}
		iterator.Advance();
	}
}

void ClientList::RemoveCLEReferances(ClientListEntry* cle) {
	LinkedListIterator<Client*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->GetCLE() == cle) {
			iterator.GetData()->SetCLE(0);
		}
		iterator.Advance();
	}
}


bool ClientList::SendPacket(const char* to, ServerPacket* pack) {
	if (to == 0 || to[0] == 0) {
		ZSList::Instance()->SendPacket(pack);
		return true;
	}
	else if (to[0] == '*') {
		// Cant send a packet to a console....
		return false;
	}
	else {
		ClientListEntry* cle = FindCharacter(to);
		if (cle != nullptr) {
			if (cle->Server() != nullptr) {
				cle->Server()->SendPacket(pack);
				return true;
			}
			return false;
		} else {
			ZoneServer* zs = ZSList::Instance()->FindByName(to);
			if (zs != nullptr) {
				zs->SendPacket(pack);
				return true;
			}
			return false;
		}
	}
	return false;
}

void ClientList::UpdateClientGuild(uint32 char_id, uint32 guild_id) {
	LinkedListIterator<ClientListEntry*> iterator(clientlist);

	iterator.Reset();
	while(iterator.MoreElements()) {
		ClientListEntry *cle = iterator.GetData();
		if (cle->CharID() == char_id) {
			cle->SetGuild(guild_id);
		}
		iterator.Advance();
	}
}

bool ClientList::IsAccountInGame(uint32 iLSID) {
	LinkedListIterator<ClientListEntry*> iterator(clientlist);
	iterator.Reset();
	while (iterator.MoreElements()) {
		if (iterator.GetData()->LSID() == iLSID && iterator.GetData()->Online() == CLE_Status::InZone) {
			return true;
		}
		iterator.Advance();
	}

	return false;
}

int ClientList::GetClientCount() {
	return(numplayers);
}

void ClientList::GetClients(const char *zone_name, std::vector<ClientListEntry *> &res) {
	LinkedListIterator<ClientListEntry *> iterator(clientlist);
	iterator.Reset();

	if(zone_name[0] == '\0') {
		while(iterator.MoreElements()) {
			ClientListEntry* tmp = iterator.GetData();
			res.push_back(tmp);
			iterator.Advance();
		}
	} else {
		uint32 zoneid = ZoneID(zone_name);
		while(iterator.MoreElements()) {
			ClientListEntry* tmp = iterator.GetData();
			if(tmp->zone() == zoneid)
				res.push_back(tmp);
			iterator.Advance();
		}
	}
}

void ClientList::SendClientVersionSummary(const char *Name)
{
	std::vector<uint32> unique_ips;
	std::map<EQ::versions::ClientVersion,int> client_count = {
		{ EQ::versions::ClientVersion::Titanium, 0 },
		{ EQ::versions::ClientVersion::SoF, 0 },
		{ EQ::versions::ClientVersion::SoD, 0 },
		{ EQ::versions::ClientVersion::UF, 0 },
		{ EQ::versions::ClientVersion::RoF, 0 },
		{ EQ::versions::ClientVersion::RoF2, 0 }
	};

	LinkedListIterator<ClientListEntry*> Iterator(clientlist);
	Iterator.Reset();
	while (Iterator.MoreElements()) {
		auto CLE = Iterator.GetData();
		if (CLE && CLE->zone()) {
			auto client_version = CLE->GetClientVersion();
			if (
				client_version >= (uint8) EQ::versions::ClientVersion::Titanium &&
				client_version <= (uint8) EQ::versions::ClientVersion::RoF2
			) {
				client_count[(EQ::versions::ClientVersion)client_version]++;
			}

			if (std::find(unique_ips.begin(), unique_ips.end(), CLE->GetIP()) == unique_ips.end()) {
				unique_ips.push_back(CLE->GetIP());
			}
		}

		Iterator.Advance();
	}

	uint32 total_clients = (
		client_count[EQ::versions::ClientVersion::Titanium] +
		client_count[EQ::versions::ClientVersion::SoF] +
		client_count[EQ::versions::ClientVersion::SoD] +
		client_count[EQ::versions::ClientVersion::UF] +
		client_count[EQ::versions::ClientVersion::RoF] +
		client_count[EQ::versions::ClientVersion::RoF2]
	);

	if (client_count[EQ::versions::ClientVersion::Titanium]) {
		ZSList::Instance()->SendEmoteMessage(
			Name,
			0,
			AccountStatus::Player,
			Chat::White,
			fmt::format(
				"Client Counts | Titanium: {}",
				client_count[EQ::versions::ClientVersion::Titanium]
			).c_str()
		);
	}

	if (client_count[EQ::versions::ClientVersion::SoF]) {
		ZSList::Instance()->SendEmoteMessage(
			Name,
			0,
			AccountStatus::Player,
			Chat::White,
			fmt::format(
				"Client Counts | SOF: {}",
				client_count[EQ::versions::ClientVersion::SoF]
			).c_str()
		);
	}

	if (client_count[EQ::versions::ClientVersion::SoD]) {
		ZSList::Instance()->SendEmoteMessage(
			Name,
			0,
			AccountStatus::Player,
			Chat::White,
			fmt::format(
				"Client Counts | SOD: {}",
				client_count[EQ::versions::ClientVersion::SoD]
			).c_str()
		);
	}

	if (client_count[EQ::versions::ClientVersion::UF]) {
		ZSList::Instance()->SendEmoteMessage(
			Name,
			0,
			AccountStatus::Player,
			Chat::White,
			fmt::format(
				"Client Counts | UF: {}",
				client_count[EQ::versions::ClientVersion::UF]
			).c_str()
		);
	}

	if (client_count[EQ::versions::ClientVersion::RoF]) {
		ZSList::Instance()->SendEmoteMessage(
			Name,
			0,
			AccountStatus::Player,
			Chat::White,
			fmt::format(
				"Client Counts | ROF: {}",
				client_count[EQ::versions::ClientVersion::RoF]
			).c_str()
		);
	}

	if (client_count[EQ::versions::ClientVersion::RoF2]) {
		ZSList::Instance()->SendEmoteMessage(
			Name,
			0,
			AccountStatus::Player,
			Chat::White,
			fmt::format(
				"Client Counts | ROF2: {}",
				client_count[EQ::versions::ClientVersion::RoF2]
			).c_str()
		);
	}


	ZSList::Instance()->SendEmoteMessage(
		Name,
		0,
		AccountStatus::Player,
		Chat::White,
		fmt::format(
			"Client Counts | Total: {} Unique IPs: {}",
			total_clients,
			unique_ips.size()
		).c_str()
	);

}

void ClientList::OnTick(EQ::Timer *t)
{
	if (!EventSubscriptionWatcher::Get()->IsSubscribed("EQW::ClientUpdate")) {
		return;
	}

	Json::Value out;
	out["event"] = "EQW::ClientUpdate";
	out["data"] = Json::Value();

	LinkedListIterator<ClientListEntry*> Iterator(clientlist);

	Iterator.Reset();

	while (Iterator.MoreElements())
	{
		ClientListEntry* cle = Iterator.GetData();

		Json::Value outclient;

		outclient["Online"] = cle->Online();
		outclient["ID"] = cle->GetID();
		outclient["IP"] = cle->GetIP();
		outclient["LSID"] = cle->LSID();
		outclient["LSAccountID"] = cle->LSAccountID();
		outclient["LSName"] = cle->LSName();
		outclient["WorldAdmin"] = cle->WorldAdmin();

		outclient["AccountID"] = cle->AccountID();
		outclient["AccountName"] = cle->AccountName();
		outclient["Admin"] = cle->Admin();

		auto server = cle->Server();
		if (server) {
			outclient["Server"]["CAddress"] = server->GetCAddress();
			outclient["Server"]["CLocalAddress"] = server->GetCLocalAddress();
			outclient["Server"]["CompileTime"] = server->GetCompileTime();
			outclient["Server"]["CPort"] = server->GetCPort();
			outclient["Server"]["ID"] = server->GetID();
			outclient["Server"]["InstanceID"] = server->GetInstanceID();
			outclient["Server"]["IP"] = server->GetIP();
			outclient["Server"]["LaunchedName"] = server->GetLaunchedName();
			outclient["Server"]["LaunchName"] = server->GetLaunchName();
			outclient["Server"]["Port"] = server->GetPort();
			outclient["Server"]["PrevZoneID"] = server->GetPrevZoneID();
			outclient["Server"]["UUID"] = server->GetUUID();
			outclient["Server"]["ZoneID"] = server->GetZoneID();
			outclient["Server"]["ZoneLongName"] = server->GetZoneLongName();
			outclient["Server"]["ZoneName"] = server->GetZoneName();
			outclient["Server"]["ZoneOSProcessID"] = server->GetZoneOSProcessID();
			outclient["Server"]["NumPlayers"] = server->NumPlayers();
			outclient["Server"]["BootingUp"] = server->IsBootingUp();
			outclient["Server"]["StaticZone"] = server->IsStaticZone();
		}
		else {
			outclient["Server"] = Json::Value();
		}

		outclient["CharID"] = cle->CharID();
		outclient["name"] = cle->name();
		outclient["zone"] = cle->zone();
		outclient["instance"] = cle->instance();
		outclient["level"] = cle->level();
		outclient["class_"] = cle->class_();
		outclient["race"] = cle->race();
		outclient["Anon"] = cle->Anon();

		outclient["TellsOff"] = cle->TellsOff();
		outclient["GuildID"] = cle->GuildID();
		outclient["LFG"] = cle->LFG();
		outclient["GM"] = cle->GetGM();
		outclient["LocalClient"] = cle->IsLocalClient();
		outclient["LFGFromLevel"] = cle->GetLFGFromLevel();
		outclient["LFGToLevel"] = cle->GetLFGToLevel();
		outclient["LFGMatchFilter"] = cle->GetLFGMatchFilter();
		outclient["LFGComments"] = cle->GetLFGComments();
		outclient["ClientVersion"] = cle->GetClientVersion();
		out["data"].append(outclient);

		Iterator.Advance();
	}

	WebInterfaceList::Instance()->SendEvent(out);
}

/**
 * @param response
 */
void ClientList::GetClientList(Json::Value &response, bool full_list)
{
	LinkedListIterator<ClientListEntry *> Iterator(clientlist);

	Iterator.Reset();

	while (Iterator.MoreElements()) {
		ClientListEntry *cle = Iterator.GetData();

		Json::Value row;

		row["id"]             = cle->GetID();
		row["name"]           = cle->name();
		row["level"]          = cle->level();
		row["ip"]             = cle->GetIP();
		row["gm"]             = cle->GetGM();
		row["race"]           = cle->race();
		row["class"]          = cle->class_();
		row["client_version"] = cle->GetClientVersion();
		row["admin"]          = cle->Admin();
		row["account_id"]     = cle->AccountID();
		row["account_name"]   = cle->AccountName();
		row["character_id"]   = cle->CharID();
		row["anon"]           = cle->Anon();
		row["guild_id"]       = cle->GuildID();

		if (full_list) {
			row["loginserver_account_id"] = cle->LSAccountID();
			row["loginserver_id"]         = cle->LSID();
			row["loginserver_name"]       = cle->LSName();
			row["online"]                 = cle->Online();
			row["world_admin"]            = cle->WorldAdmin();
			row["guild_rank"]             = cle->GuildRank();
			row["guild_tribute_opt_in"]   = cle->GuildTributeOptIn();
			row["instance"]               = cle->instance();
			row["is_local_client"]        = cle->IsLocalClient();
			row["lfg"]                    = cle->LFG();
			row["lfg_comments"]           = cle->GetLFGComments();
			row["lfg_from_level"]         = cle->GetLFGFromLevel();
			row["lfg_match_filter"]       = cle->GetLFGMatchFilter();
			row["lfg_to_level"]           = cle->GetLFGToLevel();
			row["tells_off"]              = cle->TellsOff();
			row["zone"]                   = cle->zone();
		}

		auto server = cle->Server();
		if (server) {
			row["server"]["zone_id"]        = server->GetZoneID();
			row["server"]["zone_long_name"] = server->GetZoneLongName();
			row["server"]["zone_name"]      = server->GetZoneName();
			row["server"]["zone_os_pid"]    = server->GetZoneOSProcessID();
			row["server"]["id"]             = server->GetID();

			if (full_list) {
				row["server"]["client_address"]       = server->GetCAddress();
				row["server"]["client_local_address"] = server->GetCLocalAddress();
				row["server"]["client_port"]          = server->GetCPort();
				row["server"]["compile_time"]         = server->GetCompileTime();
				row["server"]["instance_id"]          = server->GetInstanceID();
				row["server"]["ip"]                   = server->GetIP();
				row["server"]["is_booting"]           = server->IsBootingUp();
				row["server"]["launch_name"]          = server->GetLaunchName();
				row["server"]["launched_name"]        = server->GetLaunchedName();
				row["server"]["number_players"]       = server->NumPlayers();
				row["server"]["port"]                 = server->GetPort();
				row["server"]["previous_zone_id"]     = server->GetPrevZoneID();
				row["server"]["static_zone"]          = server->IsStaticZone();
				row["server"]["uui"]                  = server->GetUUID();
			}
		}
		else {
			row["server"] = Json::Value();
		}

		response.append(row);

		Iterator.Advance();
	}
}

void ClientList::SendCharacterMessage(uint32_t character_id, int chat_type, const std::string& message)
{
	auto character = FindCLEByCharacterID(character_id);
	SendCharacterMessage(character, chat_type, message);
}

void ClientList::SendCharacterMessage(const std::string& character_name, int chat_type, const std::string& message)
{
	auto character = FindCharacter(character_name.c_str());
	SendCharacterMessage(character, chat_type, message);
}

void ClientList::SendCharacterMessage(ClientListEntry* character, int chat_type, const std::string& message)
{
	if (!character || !character->Server())
	{
		return;
	}

	uint32_t pack_size = sizeof(CZMessage_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_CZMessage, pack_size);
	auto buf = reinterpret_cast<CZMessage_Struct*>(pack->pBuffer);
	buf->update_type = CZUpdateType_ClientName;
	buf->update_identifier = 0;
	buf->type = chat_type;
	strn0cpy(buf->message, message.c_str(), sizeof(buf->message));
	strn0cpy(buf->client_name, character->name(), sizeof(buf->client_name));

	character->Server()->SendPacket(pack.get());
}

void ClientList::SendCharacterMessageID(uint32_t character_id,
	int chat_type, int eqstr_id, std::initializer_list<std::string> args)
{
	auto character = FindCLEByCharacterID(character_id);
	SendCharacterMessageID(character, chat_type, eqstr_id, args);
}

void ClientList::SendCharacterMessageID(const std::string& character_name,
	int chat_type, int eqstr_id, std::initializer_list<std::string> args)
{
	auto character = FindCharacter(character_name.c_str());
	SendCharacterMessageID(character, chat_type, eqstr_id, args);
}

void ClientList::SendCharacterMessageID(ClientListEntry* character,
	int chat_type, int eqstr_id, std::initializer_list<std::string> args)
{
	if (!character || !character->Server())
	{
		return;
	}

	SerializeBuffer argbuf;
	for (const auto& arg : args)
	{
		argbuf.WriteString(arg);
	}

	uint32_t args_size = static_cast<uint32_t>(argbuf.size());
	uint32_t pack_size = sizeof(CZClientMessageString_Struct) + args_size;
	auto pack = std::make_unique<ServerPacket>(ServerOP_CZClientMessageString, pack_size);
	auto buf = reinterpret_cast<CZClientMessageString_Struct*>(pack->pBuffer);
	buf->string_id = eqstr_id;
	buf->chat_type = chat_type;
	strn0cpy(buf->client_name, character->name(), sizeof(buf->client_name));
	buf->args_size = args_size;
	if (argbuf.size() > 0)
	{
		memcpy(buf->args, argbuf.buffer(), argbuf.size());
	}

	character->Server()->SendPacket(pack.get());
}

void ClientList::GetGuildClientList(Json::Value& response, uint32 guild_id)
{
	LinkedListIterator<ClientListEntry*> Iterator(clientlist);

	Iterator.Reset();

	while (Iterator.MoreElements()) {
		ClientListEntry* cle = Iterator.GetData();
		if (cle->GuildID() != guild_id) {
			Iterator.Advance();
			continue;
		}
		Json::Value row;

		row["account_id"]             = cle->AccountID();
		row["account_name"]           = cle->AccountName();
		row["admin"]                  = cle->Admin();
		row["id"]                     = cle->GetID();
		row["ip"]                     = cle->GetIP();
		row["loginserver_account_id"] = cle->LSAccountID();
		row["loginserver_id"]         = cle->LSID();
		row["loginserver_name"]       = cle->LSName();
		row["online"]                 = cle->Online();
		row["world_admin"]            = cle->WorldAdmin();

		auto server = cle->Server();
		if (server) {
			row["server"]["client_address"]       = server->GetCAddress();
			row["server"]["client_local_address"] = server->GetCLocalAddress();
			row["server"]["client_port"]          = server->GetCPort();
			row["server"]["compile_time"]         = server->GetCompileTime();
			row["server"]["id"]                   = server->GetID();
			row["server"]["instance_id"]          = server->GetInstanceID();
			row["server"]["ip"]                   = server->GetIP();
			row["server"]["is_booting"]           = server->IsBootingUp();
			row["server"]["launch_name"]          = server->GetLaunchName();
			row["server"]["launched_name"]        = server->GetLaunchedName();
			row["server"]["number_players"]       = server->NumPlayers();
			row["server"]["port"]                 = server->GetPort();
			row["server"]["previous_zone_id"]     = server->GetPrevZoneID();
			row["server"]["static_zone"]          = server->IsStaticZone();
			row["server"]["uui"]                  = server->GetUUID();
			row["server"]["zone_id"]              = server->GetZoneID();
			row["server"]["zone_long_name"]       = server->GetZoneLongName();
			row["server"]["zone_name"]            = server->GetZoneName();
			row["server"]["zone_os_pid"]          = server->GetZoneOSProcessID();
		}
		else {
			row["server"] = Json::Value();
		}
		row["anon"]                 = cle->Anon();
		row["character_id"]         = cle->CharID();
		row["class"]                = cle->class_();
		row["client_version"]       = cle->GetClientVersion();
		row["gm"]                   = cle->GetGM();
		row["guild_id"]             = cle->GuildID();
		row["guild_rank"]           = cle->GuildRank();
		row["guild_tribute_opt_in"] = cle->GuildTributeOptIn();
		row["instance"]             = cle->instance();
		row["is_local_client"]      = cle->IsLocalClient();
		row["level"]                = cle->level();
		row["lfg"]                  = cle->LFG();
		row["lfg_comments"]         = cle->GetLFGComments();
		row["lfg_from_level"]       = cle->GetLFGFromLevel();
		row["lfg_match_filter"]     = cle->GetLFGMatchFilter();
		row["lfg_to_level"]         = cle->GetLFGToLevel();
		row["name"]                 = cle->name();
		row["race"]                 = cle->race();
		row["tells_off"]            = cle->TellsOff();
		row["zone"]                 = cle->zone();

		response.append(row);

		Iterator.Advance();
	}
}

std::map<uint32, ClientListEntry *> ClientList::GetGuildClientsWithTributeOptIn(uint32 guild_id)
{
	std::map<uint32, ClientListEntry *> guild_members;

	LinkedListIterator<ClientListEntry *> Iterator(clientlist);
	Iterator.Reset();

	while (Iterator.MoreElements()) {
		auto c = Iterator.GetData();
		if (c->GuildID() == guild_id && c->GuildTributeOptIn()) {
			guild_members.emplace(c->CharID(), c);
		}
		Iterator.Advance();
	}
	return guild_members;
}

void ClientList::RebuildZoneServerCaches()
{
	// Clear without freeing memory (buckets stay allocated)
	m_gm_zone_server_ids.clear();
	m_guild_zone_server_ids.clear();

	LinkedListIterator<ClientListEntry*> iterator(clientlist);
	iterator.Reset();

	while (iterator.MoreElements()) {
		ClientListEntry* cle = iterator.GetData();

		if (cle->Online() != CLE_Status::InZone || !cle->Server()) {
			iterator.Advance();
			continue;
		}

		uint32_t server_id = cle->Server()->GetID();

		// Track GM zone server
		if (cle->GetGM()) {
			m_gm_zone_server_ids.insert(server_id);
		}

		// Track guild zone servers
		if (cle->GuildID() > 0) {
			auto& guild_set = m_guild_zone_server_ids[cle->GuildID()];
			guild_set.insert(server_id);
		}

		iterator.Advance();
	}
}

std::vector<uint32_t> ClientList::GetGuildZoneServers(uint32 guild_id)
{
	if (RuleB(World, RealTimeCalculateGuilds)) {
		std::vector<uint32_t>        zone_server_ids;
		std::unordered_set<uint32_t> seen_ids;

		LinkedListIterator<ClientListEntry *> iterator(clientlist);

		iterator.Reset();
		while (iterator.MoreElements()) {
			ClientListEntry *cle = iterator.GetData();

			if (cle->Online() != CLE_Status::InZone) {
				iterator.Advance();
				continue;
			}

			if (!cle->Server()) {
				iterator.Advance();
				continue;
			}

			if (cle->GuildID() == guild_id) {
				uint32_t id = cle->Server()->GetID();
				if (seen_ids.insert(id).second) {
					zone_server_ids.emplace_back(id);
				}
			}

			iterator.Advance();
		}

		return zone_server_ids;
	}

	auto it = m_guild_zone_server_ids.find(guild_id);
	if (it == m_guild_zone_server_ids.end()) {
		return {};
	}
	return {it->second.begin(), it->second.end()};
}

void ClientList::AddToZoneServerCaches(ClientListEntry* cle)
{
	if (!cle || cle->Online() != CLE_Status::InZone || !cle->Server()) {
		return;
	}

	uint32_t server_id = cle->Server()->GetID();

	// Add GM zone server if applicable
	if (cle->GetGM()) {
		m_gm_zone_server_ids.insert(server_id);
	}

	// Add guild zone server if applicable
	if (cle->GuildID() > 0) {
		m_guild_zone_server_ids[cle->GuildID()].insert(server_id);
	}
}
