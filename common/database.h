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

#pragma once

#include "common/dbcore.h"
#include "common/eq_packet_structs.h"
#include "common/eqemu_logsys.h"
#include "common/linked_list.h"
#include "common/types.h"

#include <cmath>
#include <string>
#include <vector>
#include <map>

#define AUTHENTICATION_TIMEOUT    60
#define INVALID_ID                0xFFFFFFFF

class MySQLRequestResult;
class Client;

namespace EQ {
	class InventoryProfile;
}

struct npcDecayTimes_Struct {
	uint16 minlvl;
	uint16 maxlvl;
	uint32 seconds;
};


struct VarCache_Struct {
	std::map<std::string, std::string> m_cache;
	uint32                             last_update;
	VarCache_Struct() : last_update(0) { }
	void Add(const std::string& key, const std::string& value) { m_cache[key] = value; }
	const std::string* Get(const std::string& key)
	{
		auto it = m_cache.find(key);
		return (it != m_cache.end() ? &it->second : nullptr);
	}
};

class PTimerList;

#ifdef _WINDOWS
#if _MSC_VER > 1700 // greater than 2012 (2013+)
#	define _ISNAN_(a) std::isnan(a)
#else
#	include <float.h>
#	define _ISNAN_(a) _isnan(a)
#endif
#else
#	define _ISNAN_(a) std::isnan(a)
#endif

#define SQL(...) #__VA_ARGS__

class Database : public DBcore {
public:
	Database();
	Database(
		const std::string& host,
		const std::string& user,
		const std::string& password,
		const std::string& database,
		uint32 port
	);
	bool Connect(
		const std::string& host,
		const std::string& user,
		const std::string& password,
		const std::string& database,
		uint32 port,
		std::string connection_label = "default"
	);
	~Database();

	/* Character Creation */
	bool DeleteCharacter(const std::string& name);
	bool MoveCharacterToZone(const std::string& name, uint32 zone_id);
	bool MoveCharacterToZone(uint32 character_id, uint32 zone_id);
	bool ReserveName(uint32 account_id, const std::string& name);
	bool SaveCharacterCreate(uint32 character_id, uint32 account_id, PlayerProfile_Struct* pp);
	bool UpdateName(const std::string& old_name, const std::string& new_name);
	bool UpdateNameByID(const int character_id, const std::string& new_name);
	bool CopyCharacter(
		const std::string& source_character_name,
		const std::string& destination_character_name,
		const std::string& destination_account_name
	);

	/* General Information Queries */
	bool AddBannedIP(const std::string& banned_ip, const std::string& notes); //Add IP address to the banned_ips table.
	bool AddToNameFilter(const std::string& name);
	bool CheckBannedIPs(const std::string& login_ip); //Check incoming connection against banned IP table.
	bool CheckGMIPs(const std::string& login_ip, uint32 account_id);
	bool CheckNameFilter(const std::string& name, bool surname = false);
	bool IsNameUsed(const std::string& name);
	bool IsPetNameUsed(const std::string& name);

	uint32 GetAccountIDByChar(const std::string& name, uint32* character_id = 0);
	uint32 GetAccountIDByChar(uint32 character_id);
	uint32 GetAccountIDByName(const std::string& account_name, const std::string& loginserver, int16* status = 0, uint32* lsid = 0);
	uint32 GetCharacterID(const std::string& name);
	uint32 GetGuildIDByCharID(uint32 character_id);
	uint32 GetGroupIDByCharID(uint32 character_id);
	uint32 GetRaidIDByCharID(uint32 character_id);

	const std::string GetAccountName(uint32 account_id, uint32* lsaccount_id = 0);
	const std::string GetCharName(uint32 character_id);
	const std::string GetCharNameByID(uint32 character_id);
	const std::string GetNPCNameByID(uint32 npc_id);
	const std::string GetCleanNPCNameByID(uint32 npc_id);
	void LoginIP(uint32 account_id, const std::string& login_ip);

	/* Instancing */
	bool AddClientToInstance(uint16 instance_id, uint32 character_id);
	bool CheckInstanceByCharID(uint16 instance_id, uint32 character_id);
	bool CheckInstanceExists(uint16 instance_id);
	bool CheckInstanceExpired(uint16 instance_id);
	bool CreateInstance(uint16 instance_id, uint32 zone_id, uint32 version, uint32 duration);
	bool GetUnusedInstanceID(uint16& instance_id);
	bool TryGetUnusedInstanceID(uint16& instance_id);
	bool IsGlobalInstance(uint16 instance_id);
	bool RemoveClientFromInstance(uint16 instance_id, uint32 char_id);
	bool RemoveClientsFromInstance(uint16 instance_id);
	bool VerifyInstanceAlive(uint16 instance_id, uint32 character_id);
	bool VerifyZoneInstance(uint32 zone_id, uint16 instance_id);

	uint16 GetInstanceID(uint32 zone, uint32 character_id, int16 version);
	std::vector<uint16> GetInstanceIDs(uint32 zone_id, uint32 character_id);
	uint8_t GetInstanceVersion(uint16 instance_id);
	uint32 GetTimeRemainingInstance(uint16 instance_id, bool& is_perma);
	uint32 GetInstanceZoneID(uint16 instance_id);

	void AssignGroupToInstance(uint32 group_id, uint32 instance_id);
	void AssignRaidToInstance(uint32 raid_id, uint32 instance_id);
	void DeleteInstance(uint16 instance_id);
	void FlagInstanceByGroupLeader(uint32 zone_id, int16 version, uint32 character_id, uint32 group_id);
	void FlagInstanceByRaidLeader(uint32 zone_id, int16 version, uint32 character_id, uint32 raid_id);
	void GetCharactersInInstance(uint16 instance_id, std::list<uint32>& character_ids);
	void PurgeExpiredInstances();
	void SetInstanceDuration(uint16 instance_id, uint32 new_duration);
	void CleanupInstanceCorpses();

	/* Adventure related. */
	void UpdateAdventureStatsEntry(uint32 character_id, uint8 theme_id, bool is_win = false, bool is_remove = false);
	bool GetAdventureStats(uint32 character_id, AdventureStats_Struct* as);

	/* Account Related */
	const std::string GetLiveChar(uint32 account_id);
	bool SetAccountStatus(const std::string& account_name, int16 status);
	bool SetLocalPassword(uint32 account_id, const std::string& password);
	bool UpdateLiveChar(const std::string& name, uint32 account_id);
	int16 GetAccountStatus(uint32 account_id);
	void SetAccountCRCField(uint32 account_id, const std::string& field_name, uint64 checksum);
	uint32 CheckLogin(const std::string& name, const std::string& password, const std::string& loginserver, int16* status = 0);
	uint32 CreateAccount(
		const std::string& name,
		const std::string& password,
		int16 status,
		const std::string& loginserver,
		uint32 lsaccount_id
	);
	uint32 GetAccountIDFromLSID(
		const std::string& in_loginserver_id,
		uint32 in_loginserver_account_id,
		char* in_account_name = 0,
		int16* in_status = 0
	);

	uint8 GetAgreementFlag(uint32 account_id);
	void SetAgreementFlag(uint32 account_id);

	int GetIPExemption(const std::string& account_ip);
	void SetIPExemption(const std::string& account_ip, int exemption_amount);

	int GetInstanceID(uint32 character_id, uint32 zone_id);


	/* Groups */
	std::string GetGroupLeaderForLogin(const std::string& character_name);
	char* GetGroupLeadershipInfo(
		uint32 group_id,
		char* leaderbuf,
		char* maintank = nullptr,
		char* assist = nullptr,
		char* puller = nullptr,
		char* marknpc = nullptr,
		char* mentoree = nullptr,
		int* mentor_percent = nullptr,
		GroupLeadershipAA_Struct* GLAA = nullptr
	);
	std::string GetGroupLeaderName(uint32 group_id);
	uint32 GetGroupID(const std::string& name);
	void ClearGroup(uint32 group_id = 0);
	void ClearGroupLeader(uint32 group_id = 0);
	void SetGroupLeaderName(uint32 group_id, const std::string& name);

	/* Raids */
	const std::string GetRaidLeaderName(uint32 raid_id);
	uint32 GetRaidID(const std::string& name);
	void ClearRaid(uint32 raid_id = 0);
	void ClearRaidDetails(uint32 raid_id = 0);
	void ClearRaidLeader(uint32 group_id = std::numeric_limits<uint32>::max(), uint32 raid_id = 0);
	void GetGroupLeadershipInfo(
		uint32 group_id,
		uint32 raid_id,
		char* maintank = nullptr,
		char* assist = nullptr,
		char* puller = nullptr,
		char* marknpc = nullptr,
		char* mentoree = nullptr,
		int* mentor_percent = nullptr,
		GroupLeadershipAA_Struct* GLAA = nullptr
	);
	void GetRaidLeadershipInfo(
		uint32 raid_id,
		char* maintank = nullptr,
		char* assist = nullptr,
		char* puller = nullptr,
		char* marknpc = nullptr,
		RaidLeadershipAA_Struct* RLAA = nullptr
	);
	void SetRaidGroupLeaderInfo(uint32 group_id, uint32 raid_id);

	void PurgeAllDeletedDataBuckets();
	void ClearGuildOnlineStatus();
	void ClearTraderDetails();
	void ClearBuyerDetails();


	/* Database Variables */
	bool GetVariable(const std::string& name, std::string& value);
	bool SetVariable(const std::string& name, const std::string& value);
	bool LoadVariables();

	uint8 GetPEQZone(uint32 zone_id, uint32 version);
	uint32 GetServerType();
	void AddReport(const std::string& who, const std::string& against, const std::string& lines);
	struct TimeOfDay_Struct LoadTime(time_t& realtime);
	bool SaveTime(int8 minute, int8 hour, int8 day, int8 month, int16 year);
	void ClearMerchantTemp();
	void ClearPTimers(uint32 character_id);
	void SetIngame(uint32 character_id, uint8 ingame);
	void SetLFG(uint32 character_id, bool is_lfg);
	void SetLFP(uint32 character_id, bool is_lfp);
	void SetLoginFlags(uint32 character_id, bool is_lfp, bool is_lfg, uint8 first_logon);

	int64 CountInvSnapshots();
	void ClearInvSnapshots(bool from_now = false);

	void SourceDatabaseTableFromUrl(const std::string& table_name, const std::string& url);
	void SourceSqlFromUrl(const std::string& url);
	void PurgeCharacterParcels();
	void Encode(std::string &in);
	void Decode(std::string &in);

	uint64_t GetNextTableId(const std::string& table_name);

private:
	Mutex           Mvarcache;
	VarCache_Struct varcache;

	/* Groups, utility methods. */
	void ClearAllGroupLeaders();
	void ClearAllGroups();

	/* Raid, utility methods. */
	void ClearAllRaids();
	void ClearAllRaidDetails();
	void ClearAllRaidLeaders();
};
