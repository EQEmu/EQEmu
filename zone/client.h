/* EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2003 EQEMu Development Team (http://eqemulator.org)

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

class Client;
class EQApplicationPacket;
class DynamicZone;
class DzLockout;
class ExpeditionRequest;
class Group;
class NPC;
class Object;
class Raid;
class Seperator;
class ServerPacket;
struct DynamicZoneLocation;
enum WaterRegionType : int;
enum class DynamicZoneMemberStatus;

namespace EQ
{
	struct ItemData;
}

#include "zone/aggromanager.h"
#include "zone/bot_structs.h"
#include "zone/cheat_manager.h"
#include "zone/common.h"
#include "zone/merc.h"
#include "zone/mob.h"
#include "zone/qglobals.h"
#include "zone/questmgr.h"
#include "zone/task_client_state.h"
#include "zone/task_manager.h"
#include "zone/xtargetautohaters.h"
#include "zone/zone.h"
#include "zone/zonedb.h"

#include "common/data_verification.h"
#include "common/emu_constants.h"
#include "common/emu_opcodes.h"
#include "common/eq_packet_structs.h"
#include "common/eq_packet.h"
#include "common/eq_stream_intf.h"
#include "common/events/player_events.h"
#include "common/extprofile.h"
#include "common/guild_base.h"
#include "common/guilds.h"
#include "common/inventory_profile.h"
#include "common/linked_list.h"
#include "common/ptimer.h"
#include "common/races.h"
#include "common/repositories/buyer_buy_lines_repository.h"
#include "common/repositories/character_evolving_items_repository.h"
#include "common/repositories/character_parcels_repository.h"
#include "common/repositories/player_titlesets_repository.h"
#include "common/repositories/trader_repository.h"
#include "common/seperator.h"
#include "common/timer.h"
#include "common/zone_store.h"

#include <cfloat>
#include <set>
#include <algorithm>
#include <memory>
#include <deque>

#define CLIENT_LD_TIMEOUT 30000 // length of time client stays in zone after LDing
#define TARGETING_RANGE 200 // range for /assist and /target
#define XTARGET_HARDCAP 20
#define MAX_SPECIALIZED_SKILL 50

extern Zone* zone;

class CLIENTPACKET
{
public:
	CLIENTPACKET();
	~CLIENTPACKET();
	EQApplicationPacket *app;
	bool ack_req;
};

#define SPELLBAR_UNLOCK 0x2bc
enum { //scribing argument to MemorizeSpell
	memSpellUnknown = -1, // this modifies some state data
	memSpellScribing = 0,
	memSpellMemorize = 1,
	memSpellForget = 2,
	memSpellSpellbar = 3
};

//Modes for the zoning state of the client.
typedef enum {
	ZoneToSafeCoords,	// Always send ZonePlayerToBind_Struct to client: Succor/Evac
	GMSummon,			// Always send ZonePlayerToBind_Struct to client: Only a GM Summon
	GMHiddenSummon,		// Always send ZonePlayerToBind_Struct to client silently: Only a GM Summon
	ZoneToBindPoint,	// Always send ZonePlayerToBind_Struct to client: Death Only
	ZoneSolicited,		// Always send ZonePlayerToBind_Struct to client: Portal, Translocate, Evac spells that have a x y z coord in the spell data
	ZoneUnsolicited,
	GateToBindPoint,	// Always send RequestClientZoneChange_Struct to client: Gate spell or Translocate To Bind Point spell
	SummonPC,			// In-zone GMMove() always: Call of the Hero spell or some other type of in zone only summons
	Rewind,				// Summon to /rewind location.
	EvacToSafeCoords
} ZoneMode;

// translate above enum to a string
std::string GetZoneModeString(ZoneMode mode);

enum {
	HideCorpseNone = 0,
	HideCorpseAll = 1,
	HideCorpseAllButGroup = 2,
	HideCorpseLooted = 3,
	HideCorpseNPC = 5
};

typedef enum
{
	Disciplines,
	Spells
} ShowSpellType;

typedef enum
{
	Empty = 0,
	Auto = 1,
	CurrentTargetPC = 2,
	CurrentTargetNPC = 3,
	TargetsTarget = 4,
	GroupTank = 5,
	GroupTankTarget = 6,
	GroupAssist = 7,
	GroupAssistTarget = 8,
	Puller = 9,
	PullerTarget = 10,
	GroupMarkTarget1 = 11,
	GroupMarkTarget2 = 12,
	GroupMarkTarget3 = 13,
	RaidAssist1 = 14,
	RaidAssist2 = 15,
	RaidAssist3 = 16,
	RaidAssist1Target = 17,
	RaidAssist2Target = 18,
	RaidAssist3Target = 19,
	RaidMarkTarget1 = 20,
	RaidMarkTarget2 = 21,
	RaidMarkTarget3 = 22,
	MyPet = 23,
	MyPetTarget = 24,
	MyMercenary = 25,
	MyMercenaryTarget = 26

} XTargetType;

struct XTarget_Struct
{
	XTargetType Type;
	bool dirty;
	uint16 ID;
	char Name[65];
};

struct RespawnOption
{
	std::string name;
	uint32 zone_id;
	uint16 instance_id;
	float x;
	float y;
	float z;
	float heading;
};

struct BotCommandHelpParams {
    std::vector<std::string> description       = {};
    std::vector<std::string> notes             = {};
    std::vector<std::string> example_format    = {};
    std::vector<std::string> examples_one      = {};
    std::vector<std::string> examples_two      = {};
    std::vector<std::string> examples_three    = {};
    std::vector<std::string> actionables       = {};
    std::vector<std::string> options           = {};
    std::vector<std::string> options_one       = {};
    std::vector<std::string> options_two       = {};
    std::vector<std::string> options_three     = {};
};

// do not ask what all these mean because I have no idea!
// named from the client's CEverQuest::GetInnateDesc, they're missing some
enum eInnateSkill {
	InnateEnabled = 0,
	InnateAwareness = 1,
	InnateBashDoor = 2,
	InnateBreathFire = 3,
	InnateHarmony = 4,
	InnateInfravision = 6,
	InnateLore = 8,
	InnateNoBash = 9,
	InnateRegen = 10,
	InnateSlam = 11,
	InnateSurprise = 12,
	InnateUltraVision = 13,
	InnateInspect = 14,
	InnateOpen = 15,
	InnateReveal = 16,
	InnateSkillMax = 25, // size of array in client
	InnateDisabled = 255
};

inline const std::string DIAWIND_RESPONSE_ONE_KEY       = "diawind_npc_response_one";
inline const std::string DIAWIND_RESPONSE_TWO_KEY       = "diawind_npc_response_two";
const uint32      POPUPID_DIAWIND_ONE            = 99999;
const uint32      POPUPID_DIAWIND_TWO            = 100000;
const uint32      POPUPID_UPDATE_SHOWSTATSWINDOW = 1000000;
const uint32      POPUPID_REPLACE_SPELLWINDOW    = 1000001;

struct ClientReward
{
	uint32 id;
	uint32 amount;
};

struct ExpeditionInvite
{
	uint32_t    dz_id;
	std::string inviter_name;
	std::string swap_name;
};

class Client : public Mob
{
public:
	//pull in opcode mappings:
	#include "client_packet.h"

	Client(EQStreamInterface * ieqs);
	Client(); // mocking / testing
	~Client();

	void ReconnectUCS();
	void RecordStats();

	void SetDisplayMobInfoWindow(bool display_mob_info_window);
	bool GetDisplayMobInfoWindow() const;

	bool IsDevToolsEnabled() const;
	void SetDevToolsEnabled(bool in_dev_tools_enabled);

	bool IsEXPEnabled() const;
	void SetEXPEnabled(bool is_exp_enabled);

	std::vector<EXPModifier> GetEXPModifiers();
	void SetEXPModifiers(std::vector<EXPModifier> exp_modifiers);

	void SetPrimaryWeaponOrnamentation(uint32 model_id);
	void SetSecondaryWeaponOrnamentation(uint32 model_id);

	void SendChatLineBreak(uint16 color = Chat::White);

	bool GotoPlayer(const std::string& player_name);
	bool GotoPlayerGroup(const std::string& player_name);
	bool GotoPlayerRaid(const std::string& player_name);

	//abstract virtual function implementations required by base abstract class
	virtual bool Death(Mob* killer_mob, int64 damage, uint16 spell_id, EQ::skills::SkillType attack_skill, KilledByTypes killed_by = KilledByTypes::Killed_NPC, bool is_buff_tic = false);
	virtual void Damage(Mob* from, int64 damage, uint16 spell_id, EQ::skills::SkillType attack_skill, bool avoidable = true, int8 buffslot = -1, bool iBuffTic = false, eSpecialAttacks special = eSpecialAttacks::None);
	virtual bool HasRaid() { return (GetRaid() ? true : false); }
	virtual bool HasGroup() { return (GetGroup() ? true : false); }
	virtual Raid* GetRaid() { return entity_list.GetRaidByClient(this); }
	virtual Group* GetGroup() { return entity_list.GetGroupByClient(this); }
	virtual inline bool IsBerserk() { return berserk; }
	virtual void SetAttackTimer();
	int GetQuiverHaste(int delay);
	void DoAttackRounds(Mob *target, int hand, bool IsFromSpell = false);

	std::vector<Mob*> GetRaidOrGroupOrSelf(bool clients_only = false);

	bool CheckIfAlreadyDead();

	void AI_Init();
	void AI_Start(uint32 iMoveDelay = 0);
	void AI_Stop();
	void AI_Process();
	void AI_SpellCast();
	void TraderShowItems();
	void Trader_CustomerBrowsing(Client *Customer);

	void TraderEndTrader();
	void TraderPriceUpdate(const EQApplicationPacket *app);
	void SendBazaarDone(uint32 trader_id);
	void SendBulkBazaarTraders();
	void SendBulkBazaarBuyers();
	void DoBazaarInspect(BazaarInspect_Struct &in);
	void SendBazaarDeliveryCosts();
	static std::string DetermineMoneyString(uint64 copper);

	void SendTraderMode(BazaarTraderBarterActions status);
	void TraderStartTrader(const EQApplicationPacket *app);
//	void TraderPriceUpdate(const EQApplicationPacket *app);
	uint8 WithCustomer(uint16 NewCustomer);
	std::vector<uint32> GetKeyRing() { return keyring; }
	void KeyRingLoad();
	bool KeyRingAdd(uint32 item_id);
	bool KeyRingCheck(uint32 item_id);
	bool KeyRingClear();
	bool KeyRingRemove(uint32 item_id);
	void KeyRingList(Client* c = nullptr);
	bool IsNameChangeAllowed();
	void InvokeChangeNameWindow(bool immediate = true);
	bool ClearNameChange();
	void GrantNameChange();
	bool IsPetNameChangeAllowed();
	void GrantPetNameChange();
	void ClearPetNameChange();
	void InvokeChangePetName(bool immediate = true);
	bool ChangePetName(std::string new_name);
	bool IsClient() const override { return true; }
	bool IsOfClientBot() const override { return true; }
	bool IsOfClientBotMerc() const override { return true; }
	void CompleteConnect();
	bool TryStacking(EQ::ItemInstance* item, uint8 type = ItemPacketTrade, bool try_worn = true, bool try_cursor = true);
	void SendTraderPacket(Client* trader, uint32 Unknown72 = 51);
	void SendBuyerPacket(Client* Buyer);
	void SendBuyerToBarterWindow(Client* buyer, uint32 action);
	GetItems_Struct* GetTraderItems();
	void SendBazaarWelcome();
	void SendBarterWelcome();
	void DyeArmor(EQ::TintProfile* dye);
	void DyeArmorBySlot(uint8 slot, uint8 red, uint8 green, uint8 blue, uint8 use_tint = 0x00);
	uint8 SlotConvert(uint8 slot,bool bracer=false);
	void MessageString(uint32 type, uint32 string_id, uint32 distance = 0);
	void MessageString(uint32 type, uint32 string_id, const char* message,const char* message2=0,const char* message3=0,const char* message4=0,const char* message5=0,const char* message6=0,const char* message7=0,const char* message8=0,const char* message9=0, uint32 distance = 0);
	void MessageString(const CZClientMessageString_Struct* msg);
	bool FilteredMessageCheck(Mob *sender, eqFilterType filter);
	void FilteredMessageString(Mob *sender, uint32 type, eqFilterType filter, uint32 string_id);
	void FilteredMessageString(Mob *sender, uint32 type, eqFilterType filter,
					uint32 string_id, const char *message1, const char *message2 = nullptr,
					const char *message3 = nullptr, const char *message4 = nullptr,
					const char *message5 = nullptr, const char *message6 = nullptr,
					const char *message7 = nullptr, const char *message8 = nullptr,
					const char *message9 = nullptr);
	void Tell_StringID(uint32 string_id, const char *who, const char *message);
	void SendColoredText(uint32 color, std::string message);
	void SendTraderItem(uint32 item_id,uint16 quantity, TraderRepository::Trader &trader);
	void DoBazaarSearch(BazaarSearchCriteria_Struct search_criteria);
	uint16 FindTraderItem(int32 SerialNumber,uint16 Quantity);
	uint32 FindTraderItemSerialNumber(int32 ItemID);
	EQ::ItemInstance* FindTraderItemBySerialNumber(int32 SerialNumber);
	void FindAndNukeTraderItem(int32 serial_number, int16 quantity, Client* customer, uint16 trader_slot);
	void NukeTraderItem(uint16 slot, int16 charges, int16 quantity, Client* customer, uint16 trader_slot, int32 serial_number, int32 item_id = 0);
	void ReturnTraderReq(const EQApplicationPacket* app,int16 traderitemcharges, uint32 itemid = 0);
	void TradeRequestFailed(const EQApplicationPacket* app);
	void BuyTraderItem(TraderBuy_Struct* tbs, Client* trader, const EQApplicationPacket* app);
	void BuyTraderItemOutsideBazaar(TraderBuy_Struct* tbs, const EQApplicationPacket* app);
	void FinishTrade(
		Mob *with,
		bool finalizer = false,
		void *event_entry = nullptr,
		std::list<void *> *event_details = nullptr
	);
	void SendZonePoints();
	void SendBulkParcels();
	void DoParcelCancel();
	void DoParcelSend(const Parcel_Struct *parcel_in);
	void DoParcelRetrieve(const ParcelRetrieve_Struct &parcel_in);
	void SendParcel(Parcel_Struct &parcel);
	void SendParcelStatus();
	void SendParcelAck();
	void SendParcelRetrieveAck();
	void SendParcelDelete(const ParcelRetrieve_Struct &parcel_in);
	void SendParcelDeliveryToWorld(const Parcel_Struct &parcel);
	void SetParcelEnabled(bool status) { m_parcel_enabled = status; }
	bool GetParcelEnabled() { return m_parcel_enabled; }
	void SetParcelCount(uint32 count) { m_parcel_count = count; }
	int32 GetParcelCount() { return m_parcel_count; }
	bool GetEngagedWithParcelMerchant() { return m_parcel_merchant_engaged; }
	void SetEngagedWithParcelMerchant(bool status) { m_parcel_merchant_engaged = status; }
	Timer *GetParcelTimer() { return &parcel_timer; }
	bool DeleteParcel(uint32 parcel_id);
	void AddParcel(CharacterParcelsRepository::CharacterParcels &parcel);
	void LoadParcels();
	std::map<uint32, CharacterParcelsRepository::CharacterParcels> GetParcels() { return m_parcels; }
	int32 FindNextFreeParcelSlot(uint32 char_id);
	int32 FindNextFreeParcelSlotUsingMemory();
	void SendParcelIconStatus();

	void SendBecomeTraderToWorld(Client *trader, BazaarTraderBarterActions action);
	void SendBecomeTrader(BazaarTraderBarterActions action, uint32 trader_id);

	bool IsThereACustomer() const { return customer_id ? true : false; }
	uint32 GetCustomerID() { return customer_id; }
	void SetCustomerID(uint32 id) { customer_id = id; }

	void   SetBuyerID(uint32 id) { m_buyer_id = id; }
	uint32 GetBuyerID() { return m_buyer_id; }
	bool   IsBuyer() { return m_buyer_id != 0 ? true : false; }
	void   SetBarterTime() { m_barter_time = time(nullptr); }
	uint32 GetBarterTime() { return m_barter_time; }
	void   SetBuyerWelcomeMessage(const char* welcome_message);
	void   SendBuyerGreeting(uint32 char_id);
	void   SendSellerBrowsing(const std::string &browser);
	void   SendBuyerMode(bool status);
	bool   IsInBuyerSpace();
	void   SendBuyLineUpdate(const BuyerLineItems_Struct &buy_line);
	void   CheckIfMovedItemIsPartOfBuyLines(uint32 item_id);

	void SendBuyerResults(BarterSearchRequest_Struct& bsr);
	void ShowBuyLines(const EQApplicationPacket *app);
	void SellToBuyer(const EQApplicationPacket *app);
	void ToggleBuyerMode(bool TurnOn);
	void ModifyBuyLine(const EQApplicationPacket *app);
	void CreateStartingBuyLines(const EQApplicationPacket *app);
	void BuyerItemSearch(const EQApplicationPacket *app);
	void SendWindowUpdatesToSellerAndBuyer(BuyerLineSellItem_Struct& blsi);
	void SendBarterBuyerClientMessage(BuyerLineSellItem_Struct& blsi, BarterBuyerActions action, BarterBuyerSubActions sub_action, BarterBuyerSubActions error_code);
	bool BuildBuyLineMap(std::map<uint32, BuylineItemDetails_Struct>& item_map, BuyerBuyLines_Struct& bl);
	bool BuildBuyLineMapFromVector(std::map<uint32, BuylineItemDetails_Struct>& item_map, std::vector<BuyerLineItems_Struct>& bl);
	void RemoveItemFromBuyLineMap(std::map<uint32, BuylineItemDetails_Struct>& item_map, const BuyerLineItems_Struct& bl);
	bool ValidateBuyLineItems(std::map<uint32, BuylineItemDetails_Struct>& item_map);
	int64 ValidateBuyLineCost(std::map<uint32, BuylineItemDetails_Struct>& item_map);
	bool DoBarterBuyerChecks(BuyerLineSellItem_Struct& sell_line);
	bool DoBarterSellerChecks(BuyerLineSellItem_Struct& sell_line);
	void CancelBuyerTradeWindow();
	void CancelTraderTradeWindow();

	void FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho);
	bool ShouldISpawnFor(Client *c) { return !GMHideMe(c) && !IsHoveringForRespawn(); }
	virtual bool Process();
	void QueuePacket(const EQApplicationPacket* app, bool ack_req = true, CLIENT_CONN_STATUS = CLIENT_CONNECTINGALL, eqFilterType filter=FilterNone);
	void FastQueuePacket(EQApplicationPacket** app, bool ack_req = true, CLIENT_CONN_STATUS = CLIENT_CONNECTINGALL);
	void ChannelMessageReceived(uint8 chan_num, uint8 language, uint8 lang_skill, const char* orig_message, const char* targetname = nullptr, bool is_silent = false);
	void ChannelMessageSend(const char* from, const char* to, uint8 channel_id, uint8 language_id, uint8 language_skill, const char* message, ...);
	void Message(uint32 type, const char* message, ...);
	void FilteredMessage(Mob *sender, uint32 type, eqFilterType filter, const char* message, ...);
	void VoiceMacroReceived(uint32 Type, char *Target, uint32 MacroNumber);
	void SendSound();
	void LearnRecipe(uint32 recipe_id);
	int GetRecipeMadeCount(uint32 recipe_id);
	bool HasRecipeLearned(uint32 recipe_id);
	bool CanIncreaseTradeskill(EQ::skills::SkillType tradeskill);
	void ScribeRecipes(uint32_t item_id) const;

	bool GetRevoked() const { return revoked; }
	void SetRevoked(bool rev) { revoked = rev; }
	inline uint32 GetIP() const { return ip; }
	std::string GetIPString();
	int GetIPExemption();
	void SetIPExemption(int exemption_amount);
	inline bool GetHideMe() const { return gm_hide_me; }
	void SetHideMe(bool flag);
	inline uint16 GetPort() const { return port; }
	bool IsDead() const { return(dead); }
	bool IsUnconscious() const { return ((current_hp <= 0) ? true : false); }
	inline bool IsLFP() { return LFP; }
	void UpdateLFP();

	virtual bool Save() { return Save(0); }
	bool Save(uint8 iCommitNow); // 0 = delayed, 1=async now, 2=sync now
	inline void SaveCharacterData() {
		database.SaveCharacterData(this, &m_pp, &m_epp);
	};

	/* New PP Save Functions */
	bool SaveCurrency(){ return database.SaveCharacterCurrency(this->CharacterID(), &m_pp); }
	bool SaveAA();
	void RemoveExpendedAA(int aa_id);

	inline bool ClientDataLoaded() const { return client_data_loaded; }
	inline bool Connected() const { return (client_state == CLIENT_CONNECTED); }
	inline bool InZone() const { return (client_state == CLIENT_CONNECTED || client_state == CLIENT_LINKDEAD); }
	inline void Disconnect() { eqs->Close(); client_state = DISCONNECTED; }
	inline bool IsLD() const { return (bool) (client_state == CLIENT_LINKDEAD); }
	void Kick(const std::string &reason);
	void WorldKick();
	inline uint8 GetAnon() const { return m_pp.anon; }
	inline uint8 GetAFK() const { return m_is_afk; }
	void SetAnon(uint8 anon_flag);
	inline Client* ResetAFKTimer() {
		if (!RuleB(Character, EnableAutoAFK)) {
			return this;
		}

		m_afk_reset = true;
		m_last_moved = std::chrono::steady_clock::now();
		return this;
	};
	void SetAFK(uint8 afk_flag);
	inline bool IsIdle() { return m_is_idle; }
	inline PlayerProfile_Struct& GetPP() { return m_pp; }
	inline ExtendedProfile_Struct& GetEPP() { return m_epp; }
	inline EQ::InventoryProfile& GetInv() { return m_inv; }
	inline const EQ::InventoryProfile& GetInv() const { return m_inv; }
	const std::vector<int16>& GetInventorySlots();
	inline PetInfo* GetPetInfo(int pet_info_type) { return pet_info_type == PetInfoType::Suspended ? &m_suspendedminion : &m_petinfo; }
	inline InspectMessage_Struct& GetInspectMessage() { return m_inspect_message; }
	inline const InspectMessage_Struct& GetInspectMessage() const { return m_inspect_message; }
	void ReloadExpansionProfileSetting();

	void SetPetCommandState(uint8 button, uint8 state);

	bool AutoAttackEnabled() const { return auto_attack; }
	bool AutoFireEnabled() const { return auto_fire; }

	bool ChangeFirstName(const std::string in_firstname,const std::string gmname);
	bool ChangeFirstName(const std::string in_firstname);

	void Duck();
	void Stand();
	void Sit();

	virtual void SetMaxHP();
	int32 LevelRegen();
	void SetGM(bool toggle);
	void SetPVP(bool toggle, bool message = true);

	inline bool GetPVP(bool inc_temp = true) const { return m_pp.pvp != 0 || (inc_temp && temp_pvp); }
	inline bool GetGM() const { return m_pp.gm != 0; }

	inline void SetBaseClass(uint32 i) { m_pp.class_=i; }
	inline void SetBaseRace(uint32 i) { m_pp.race=i; }
	inline void SetBaseGender(uint32 i) { m_pp.gender=i; }
	inline void SetDeity(uint32 i) {m_pp.deity=i;deity=i;}

	void SetTrackingID(uint32 entity_id);

	inline uint8 GetLevel2() const { return m_pp.level2; }
	inline uint16 GetBaseRace() const { return m_pp.race; }
	inline uint16 GetBaseClass() const { return m_pp.class_; }
	inline uint8 GetBaseGender() const { return m_pp.gender; }
	inline uint8 GetBaseFace() const { return m_pp.face; }
	inline uint8 GetBaseHairColor() const { return m_pp.haircolor; }
	inline uint8 GetBaseBeardColor() const { return m_pp.beardcolor; }
	inline uint8 GetBaseEyeColor() const { return m_pp.eyecolor1; }
	inline uint8 GetBaseHairStyle() const { return m_pp.hairstyle; }
	inline uint8 GetBaseBeard() const { return m_pp.beard; }
	inline uint8 GetBaseHeritage() const { return m_pp.drakkin_heritage; }
	inline uint8 GetBaseTattoo() const { return m_pp.drakkin_tattoo; }
	inline uint8 GetBaseDetails() const { return m_pp.drakkin_details; }
	inline const float GetBindX(uint32 index = 0) const { return m_pp.binds[index].x; }
	inline const float GetBindY(uint32 index = 0) const { return m_pp.binds[index].y; }
	inline const float GetBindZ(uint32 index = 0) const { return m_pp.binds[index].z; }
	inline const float GetBindHeading(uint32 index = 0) const { return m_pp.binds[index].heading; }
	inline uint32 GetBindZoneID(uint32 index = 0) const { return m_pp.binds[index].zone_id; }
	inline uint32 GetBindInstanceID(uint32 index = 0) const { return m_pp.binds[index].instance_id; }
	int64 CalcMaxMana();
	int64 CalcBaseMana();
	const int64& SetMana(int64 amount);
	int64 CalcManaRegenCap() final;

	// guild pool regen shit. Sends a SpawnAppearance with a value that regens to value * 0.001
	void EnableAreaHPRegen(int value);
	void DisableAreaHPRegen();
	void EnableAreaManaRegen(int value);
	void DisableAreaManaRegen();
	void EnableAreaEndRegen(int value);
	void DisableAreaEndRegen();
	void EnableAreaRegens(int value);
	void DisableAreaRegens();

	void ServerFilter(SetServerFilter_Struct* filter);
	void BulkSendTraderInventory(uint32 char_id);
	void SendSingleTraderItem(uint32 char_id, int serial_number);
	void BulkSendMerchantInventory(int merchant_id, int npcid);

	inline uint8 GetLanguageSkill(uint8 language_id) const { return m_pp.languages[language_id]; }

	void SendPickPocketResponse(Mob *from, uint32 amt, int type, const EQ::ItemData* item = nullptr);

	inline const char* GetLastName() const { return lastname; }

	inline float ProximityX() const { return m_Proximity.x; }
	inline float ProximityY() const { return m_Proximity.y; }
	inline float ProximityZ() const { return m_Proximity.z; }
	inline void ClearAllProximities() { entity_list.ProcessMove(this, glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX)); m_Proximity = glm::vec3(FLT_MAX,FLT_MAX,FLT_MAX); }

	void CheckVirtualZoneLines();

	/*
			Begin client modifiers
	*/

	virtual void CalcBonuses();
	//these are all precalculated now
	inline virtual int32 GetATKBonus() const { return itembonuses.ATK + spellbonuses.ATK; }
	inline virtual int GetHaste() const { return Haste; }
	int GetRawACNoShield(int &shield_ac) const;

	inline virtual int32 GetSTR() const { return STR; }
	inline virtual int32 GetSTA() const { return STA; }
	inline virtual int32 GetDEX() const { return DEX; }
	inline virtual int32 GetAGI() const { return AGI; }
	inline virtual int32 GetINT() const { return INT; }
	inline virtual int32 GetWIS() const { return WIS; }
	inline virtual int32 GetCHA() const { return CHA; }
	inline virtual int32 GetMR() const { return MR; }
	inline virtual int32 GetFR() const { return FR; }
	inline virtual int32 GetDR() const { return DR; }
	inline virtual int32 GetPR() const { return PR; }
	inline virtual int32 GetCR() const { return CR; }
	inline virtual int32 GetCorrup() const { return Corrup; }
	inline virtual int32 GetPhR() const { return PhR; }

	int32 GetMaxStat() const;
	int32 GetMaxResist() const;
	int32 GetMaxSTR() const;
	int32 GetMaxSTA() const;
	int32 GetMaxDEX() const;
	int32 GetMaxAGI() const;
	int32 GetMaxINT() const;
	int32 GetMaxWIS() const;
	int32 GetMaxCHA() const;
	int32 GetMaxMR() const;
	int32 GetMaxPR() const;
	int32 GetMaxDR() const;
	int32 GetMaxCR() const;
	int32 GetMaxFR() const;
	int32 GetMaxCorrup() const;
	inline uint8 GetBaseSTR() const { return m_pp.STR; }
	inline uint8 GetBaseSTA() const { return m_pp.STA; }
	inline uint8 GetBaseCHA() const { return m_pp.CHA; }
	inline uint8 GetBaseDEX() const { return m_pp.DEX; }
	inline uint8 GetBaseINT() const { return m_pp.INT; }
	inline uint8 GetBaseAGI() const { return m_pp.AGI; }
	inline uint8 GetBaseWIS() const { return m_pp.WIS; }
	inline uint8 GetBaseCorrup() const { return 15; } // Same for all
	inline uint8 GetBasePhR() const { return 0; } // Guessing at 0 as base

	inline virtual int32 GetHeroicSTR() const { return itembonuses.HeroicSTR; }
	inline virtual int32 GetHeroicSTA() const { return itembonuses.HeroicSTA; }
	inline virtual int32 GetHeroicDEX() const { return itembonuses.HeroicDEX; }
	inline virtual int32 GetHeroicAGI() const { return itembonuses.HeroicAGI; }
	inline virtual int32 GetHeroicINT() const { return itembonuses.HeroicINT; }
	inline virtual int32 GetHeroicWIS() const { return itembonuses.HeroicWIS; }
	inline virtual int32 GetHeroicCHA() const { return itembonuses.HeroicCHA; }
	inline virtual int32 GetHeroicMR() const { return itembonuses.HeroicMR; }
	inline virtual int32 GetHeroicFR() const { return itembonuses.HeroicFR; }
	inline virtual int32 GetHeroicDR() const { return itembonuses.HeroicDR; }
	inline virtual int32 GetHeroicPR() const { return itembonuses.HeroicPR; }
	inline virtual int32 GetHeroicCR() const { return itembonuses.HeroicCR; }
	inline virtual int32 GetHeroicCorrup() const { return itembonuses.HeroicCorrup; }
	inline virtual int32 GetHeroicPhR() const { return 0; } // Heroic PhR not implemented yet
	// Mod2
	inline virtual int32 GetShielding() const { return itembonuses.MeleeMitigation; }
	inline virtual int32 GetSpellShield() const { return itembonuses.SpellShield; }
	inline virtual int32 GetDoTShield() const { return itembonuses.DoTShielding; }
	inline virtual int32 GetStunResist() const { return itembonuses.StunResist; }
	inline virtual int32 GetStrikeThrough() const { return itembonuses.StrikeThrough; }
	inline virtual int32 GetAvoidance() const { return itembonuses.AvoidMeleeChance; }
	inline virtual int32 GetAccuracy() const { return itembonuses.HitChance; }
	inline virtual int32 GetCombatEffects() const { return itembonuses.ProcChance; }
	inline virtual int32 GetDS() const { return itembonuses.DamageShield; }
	// Mod3
	inline int32 GetHealAmt() const override { return itembonuses.HealAmt; }
	inline int32 GetSpellDmg() const final { return itembonuses.SpellDmg; }
	inline virtual int32 GetClair() const { return itembonuses.Clairvoyance; }
	inline virtual int32 GetDSMit() const { return itembonuses.DSMitigation; }

	inline virtual int32 GetSingMod() const { return itembonuses.singingMod; }
	inline virtual int32 GetBrassMod() const { return itembonuses.brassMod; }
	inline virtual int32 GetPercMod() const { return itembonuses.percussionMod; }
	inline virtual int32 GetStringMod() const { return itembonuses.stringedMod; }
	inline virtual int32 GetWindMod() const { return itembonuses.windMod; }

	inline virtual int32 GetDelayDeath() const { return aabonuses.DelayDeath + spellbonuses.DelayDeath + itembonuses.DelayDeath + 11; }

	virtual bool CheckFizzle(uint16 spell_id);
	virtual int GetCurrentBuffSlots() const;
	virtual int GetCurrentSongSlots() const;
	virtual int GetCurrentDiscSlots() const { return 1; }
	virtual int GetMaxBuffSlots() const { return EQ::spells::LONG_BUFFS; }
	virtual int GetMaxSongSlots() const { return EQ::spells::SHORT_BUFFS; }
	virtual int GetMaxDiscSlots() const { return EQ::spells::DISC_BUFFS; }
	virtual int GetMaxTotalSlots() const { return EQ::spells::TOTAL_BUFFS; }
	virtual uint32 GetFirstBuffSlot(bool disc, bool song);
	virtual uint32 GetLastBuffSlot(bool disc, bool song);
	virtual void InitializeBuffSlots();
	virtual void UninitializeBuffSlots();

	inline const int32 GetBaseHP() const { return base_hp; }

	uint32 GetWeight() const { return(weight); }
	inline void RecalcWeight() { weight = CalcCurrentWeight(); }
	uint32 CalcCurrentWeight();
	inline uint32 GetCopper() const { return m_pp.copper; }
	inline uint32 GetSilver() const { return m_pp.silver; }
	inline uint32 GetGold() const { return m_pp.gold; }
	inline uint32 GetPlatinum() const { return m_pp.platinum; }


	/*Endurance and such*/
	void CalcMaxEndurance(); //This calculates the maximum endurance we can have
	int64 CalcBaseEndurance(); //Calculates Base End
	int64 CalcEnduranceRegen(bool bCombat = false); //Calculates endurance regen used in DoEnduranceRegen()
	int64 GetEndurance() const {return current_endurance;} //This gets our current endurance
	int64 GetMaxEndurance() const {return max_end;} //This gets our endurance from the last CalcMaxEndurance() call
	int64 CalcEnduranceRegenCap() final;
	int64 CalcHPRegenCap() final;
	inline uint8 GetEndurancePercent() { return (uint8)((float)current_endurance / (float)max_end * 100.0f); }
	void SetEndurance(int32 newEnd); //This sets the current endurance to the new value
	void DoEnduranceRegen(); //This Regenerates endurance
	void DoEnduranceUpkeep(); //does the endurance upkeep

	//This calculates total Attack Rating to match very close to what the client should show
	uint32 GetTotalATK();
	uint32 GetATKRating();
	//This gets the skill value of the item type equipped in the Primary Slot
	uint16 GetPrimarySkillValue();

	bool Flurry();
	bool Rampage();

	inline uint32 GetEXP() const { return m_pp.exp; }

	float GetAAEXPModifier(uint32 zone_id, int16 instance_version = -1);
	float GetEXPModifier(uint32 zone_id, int16 instance_version = -1);
	void SetAAEXPModifier(uint32 zone_id, float aa_modifier, int16 instance_version = -1);
	void SetEXPModifier(uint32 zone_id, float exp_modifier, int16 instance_version = -1);

	void SetAAEXPPercentage(uint8 percentage);

	bool UpdateLDoNPoints(uint32 theme_id, int points);
	void SetLDoNPoints(uint32 theme_id, uint32 points);
	void SetPVPPoints(uint32 Points) { m_pp.PVPCurrentPoints = Points; }
	uint32 GetPVPPoints() { return m_pp.PVPCurrentPoints; }
	void AddPVPPoints(uint32 Points);
	void AddEbonCrystals(uint32 amount, bool is_reclaim = false);
	void AddRadiantCrystals(uint32 amount, bool is_reclaim = false);
	void RemoveEbonCrystals(uint32 amount, bool is_reclaim = false);
	void RemoveRadiantCrystals(uint32 amount, bool is_reclaim = false);
	uint32 GetRadiantCrystals() { return m_pp.currentRadCrystals; }
	void SetRadiantCrystals(uint32 value);
	uint32 GetEbonCrystals() { return m_pp.currentEbonCrystals; }
	void SetEbonCrystals(uint32 value);
	void SendCrystalCounts();

	uint64 GetExperienceForKill(Mob *against);
	void AddEXP(ExpSource exp_source, uint64 in_add_exp, uint8 conlevel = 0xFF, bool resexp = false, NPC* npc = nullptr);
	uint64 CalcEXP(uint8 conlevel = 0xFF, bool ignore_mods = false);
	void CalculateNormalizedAAExp(uint64 &add_aaxp, uint8 conlevel, bool resexp);
	void CalculateStandardAAExp(uint64 &add_aaxp, uint8 conlevel, bool resexp);
	void CalculateLeadershipExp(uint64 &add_exp, uint8 conlevel);
	void CalculateExp(uint64 in_add_exp, uint64 &add_exp, uint64 &add_aaxp, uint8 conlevel, bool resexp);
	void SetEXP(ExpSource exp_source, uint64 set_exp, uint64 set_aaxp, bool resexp = false, NPC* npc = nullptr);
	void AddLevelBasedExp(ExpSource exp_source, uint8 exp_percentage, uint8 max_level = 0, bool ignore_mods = false);
	void SetLeadershipEXP(uint64 group_exp, uint64 raid_exp);
	void AddLeadershipEXP(uint64 group_exp, uint64 raid_exp);
	void SendLeadershipEXPUpdate();
	bool IsLeadershipEXPOn();
	inline int GetLeadershipAA(int AAID) { return m_pp.leader_abilities.ranks[AAID]; }
	inline LeadershipAA_Struct &GetLeadershipAA() { return m_pp.leader_abilities; }
	inline GroupLeadershipAA_Struct &GetGroupLeadershipAA() { return m_pp.leader_abilities.group; }
	inline RaidLeadershipAA_Struct &GetRaidLeadershipAA() { return m_pp.leader_abilities.raid; }
	int GroupLeadershipAAHealthEnhancement();
	int GroupLeadershipAAManaEnhancement();
	int GroupLeadershipAAHealthRegeneration();
	int GroupLeadershipAAOffenseEnhancement();
	void InspectBuffs(Client* Inspector, int Rank);
	uint32 GetRaidPoints() { return(m_pp.raid_leadership_points); }
	uint32 GetGroupPoints() { return(m_pp.group_leadership_points); }
	uint32 GetRaidEXP() { return(m_pp.raid_leadership_exp); }
	uint32 GetGroupEXP() { return(m_pp.group_leadership_exp); }
	uint32 GetTotalSecondsPlayed() { return(TotalSecondsPlayed); }
	virtual void SetLevel(uint8 set_level, bool command = false);

	void GoToBind(uint8 bindnum = 0);
	void GoToSafeCoords(uint16 zone_id, uint16 instance_id);
	void Gate(uint8 bindnum = 0);
	void SetBindPoint(int bind_num = 0, int to_zone = -1, int to_instance = 0, const glm::vec3& location = glm::vec3());
	void SetBindPoint2(int bind_num = 0, int to_zone = -1, int to_instance = 0, const glm::vec4& location = glm::vec4());
	void SetStartZone(uint32 zoneid, float x = 0.0f, float y =0.0f, float z = 0.0f, float heading = 0.0f);
	uint32 GetStartZone(void);
	void MovePC(const char* zonename, float x, float y, float z, float heading, uint8 ignorerestrictions = 0, ZoneMode zm = ZoneSolicited);
	void MovePC(uint32 zoneID, float x, float y, float z, float heading, uint8 ignorerestrictions = 0, ZoneMode zm = ZoneSolicited);
	void MovePC(float x, float y, float z, float heading, uint8 ignorerestrictions = 0, ZoneMode zm = ZoneSolicited);
	void MovePC(uint32 zoneID, uint32 instanceID, float x, float y, float z, float heading, uint8 ignorerestrictions = 0, ZoneMode zm = ZoneSolicited);
	void MoveZone(const char *zone_short_name, const glm::vec4& location = glm::vec4(0.f));
	void MoveZoneGroup(const char *zone_short_name, const glm::vec4& location = glm::vec4(0.f));
	void MoveZoneRaid(const char *zone_short_name, const glm::vec4& location = glm::vec4(0.f));
	void MoveZoneInstance(uint16 instance_id, const glm::vec4& location = glm::vec4(0.f));
	void MoveZoneInstanceGroup(uint16 instance_id, const glm::vec4& location = glm::vec4(0.f));
	void MoveZoneInstanceRaid(uint16 instance_id, const glm::vec4& location = glm::vec4(0.f));
	void SendToGuildHall();
	void SendToInstance(std::string instance_type, std::string zone_short_name, uint32 instance_version, float x, float y, float z, float heading, std::string instance_identifier, uint32 duration);
	void AssignToInstance(uint16 instance_id);
	void RemoveFromInstance(uint16 instance_id);
	void WhoAll();
	bool CheckLoreConflict(const EQ::ItemData* item);
	void ChangeLastName(std::string last_name);
	void GetGroupAAs(GroupLeadershipAA_Struct *into) const;
	void GetRaidAAs(RaidLeadershipAA_Struct *into) const;
	void ClearGroupAAs();
	void UpdateGroupAAs(int32 points, uint32 type);
	void SacrificeConfirm(Mob* caster);
	void Sacrifice(Mob* caster);
	void GoToDeath();
	inline const int32 GetInstanceID() const { return zone->GetInstanceID(); }
	void SetZoning(bool in) { bZoning = in; }
	bool IsZoning() { return bZoning; }

	void ShowSpells(Client* c, ShowSpellType show_spell_type);

	FACTION_VALUE GetReverseFactionCon(Mob* iOther);
	FACTION_VALUE GetFactionLevel(uint32 char_id, uint32 npc_id, uint32 p_race, uint32 p_class, uint32 p_deity, int32 pFaction, Mob* tnpc);
	bool ReloadCharacterFaction(Client *c, uint32 facid, uint32 charid);
	int32 GetCharacterFactionLevel(int32 faction_id);
	int32 GetModCharacterFactionLevel(int32 faction_id);
	void MerchantRejectMessage(Mob *merchant, int primaryfaction);
	void SendFactionMessage(int32 tmpvalue, int32 faction_id, int32 faction_before_hit, int32 totalvalue, uint8 temp,  int32 this_faction_min, int32 this_faction_max);

	void UpdatePersonalFaction(int32 char_id, int32 npc_value, int32 faction_id, int32 *current_value, int32 temp, int32 this_faction_min, int32 this_faction_max);
	void SetFactionLevel(uint32 char_id, uint32 npc_faction_id, uint8 char_class, uint8 char_race, uint8 char_deity, bool quest = false);
	void SetFactionLevel2(uint32 char_id, int32 faction_id, uint8 char_class, uint8 char_race, uint8 char_deity, int32 value, uint8 temp);
	int32 GetRawItemAC();

	inline uint32 LSAccountID() const { return lsaccountid; }
	inline uint32 GetWID() const { return WID; }
	inline void SetWID(uint32 iWID) { WID = iWID; }
	inline uint32 AccountID() const { return account_id; }

	inline const char* AccountName()const { return account_name; }
	inline int GetAccountCreation() const { return account_creation; }
	inline int16 Admin() const { return admin; }
	inline uint32 CharacterID() const { return character_id; }
	void UpdateAdmin(bool from_database = true);
	void UpdateWho(uint8 remove = 0);
	bool GMHideMe(Client* client = 0);

	inline bool IsInAGuild() const { return(guild_id != GUILD_NONE && guild_id != 0); }
	inline bool IsInGuild(uint32 in_gid) const { return(in_gid == guild_id && IsInAGuild()); }
	inline bool GetGuildListDirty() { return guild_dirty; }
	inline void SetGuildListDirty(bool state) { guild_dirty = state; }
	inline uint32 GuildID() const { return guild_id; }
	inline uint8 GuildRank() const { return guildrank; }
	inline bool GuildTributeOptIn() const { return guild_tribute_opt_in; }
	void SetGuildTributeOptIn(bool state);
	void SendGuildTributeDonateItemReply(GuildTributeDonateItemRequest_Struct* in, uint32 favor);
	void SendGuildTributeDonatePlatReply(GuildTributeDonatePlatRequest_Struct* in, uint32 favor);
	void SetGuildRank(uint32 rank);
	void SetGuildID(uint32 guild_id);
	void SendGuildMOTD(bool GetGuildMOTDReply = false);
	void SendGuildURL();
	void SendGuildChannel();
	void SendGuildSpawnAppearance();
	void SendGuildRanks();
	void SendGuildMembers();
	void SendGuildList();
	void SendGuildJoin(GuildJoin_Struct* gj);
	void RefreshGuildInfo();
	void SendGuildRankNames();
	void SendGuildTributeDetails(uint32 tribute_id, uint32 tier);
	void DoGuildTributeUpdate();
	void SendGuildActiveTributes(uint32 guild_id);
	void SendGuildFavorAndTimer(uint32 guild_id);
	void SendGuildTributeOptInToggle(const GuildTributeMemberToggle* in);
	void RequestGuildActiveTributes(uint32 guild_id);
	void RequestGuildFavorAndTimer(uint32 guild_id);
	void SendGuildMembersList();
	void SendGuildMemberAdd(uint32 guild_id, uint32 level, uint32 _class, uint32 rank, uint32 guild_show, uint32 zone_id, std::string player_name);
	void SendGuildMemberRename(uint32 guild_id, std::string player_name, std::string new_player_name);
	void SendGuildMemberDelete(uint32 guild_id, std::string player_name);
	void SendGuildMemberLevel(uint32 guild_id, uint32 level, std::string player_name);
	void SendGuildMemberRankAltBanker(uint32 guild_id, uint32 rank, std::string player_name, bool alt, bool banker);
	void SendGuildMemberPublicNote(uint32 guild_id, std::string player_name, std::string public_note);
	void SendGuildMemberDetails(uint32 guild_id, uint32 zone_id, uint32 offline_mode, std::string player_name);
	void SendGuildRenameGuild(uint32 guild_id, std::string new_guild_name);
	void SendGuildDeletePacket(uint32 guild_id);

	uint8 GetClientMaxLevel() const { return client_max_level; }
	void SetClientMaxLevel(uint8 max_level) { client_max_level = max_level; }

	void CheckManaEndUpdate();
	void SendManaUpdate();
	void SendEnduranceUpdate();
	uint8 GetFace() const { return m_pp.face; }
	void WhoAll(Who_All_Struct* whom);
	void FriendsWho(char *FriendsString);

	void Stun(int duration);
	void UnStun();
	void ReadBook(BookRequest_Struct *book);
	void ReadBookByName(std::string book_name, uint8 book_type);
	void QuestReadBook(const char* text, uint8 type);
	void SendMoneyUpdate();
	bool TakeMoneyFromPP(uint64 copper, bool update_client = false);
	bool TakePlatinum(uint32 platinum, bool update_client = false);
	void AddMoneyToPP(uint64 copper, bool update_client = false);
	void AddMoneyToPP(uint32 copper, uint32 silver, uint32 gold, uint32 platinum, bool update_client = false);
	void AddPlatinum(uint32 platinu, bool update_client = false);
	bool HasMoney(uint64 copper);
	uint64 GetCarriedMoney();
	uint32 GetCarriedPlatinum();
	uint64 GetAllMoney();
	uint32 GetMoney(uint8 type, uint8 subtype);
	int GetAccountAge();

	void SendPath(Mob* target);

	bool IsDiscovered(uint32 itemid);
	void DiscoverItem(uint32 itemid);

	bool TGB() const { return tgb; }

	void OnDisconnect(bool hard_disconnect);

	uint16 GetSkillPoints() { return m_pp.points;}
	void SetSkillPoints(int inp) { m_pp.points = inp;}

	void IncreaseSkill(int skill_id, int value = 1) { if (skill_id <= EQ::skills::HIGHEST_SKILL) { m_pp.skills[skill_id] += value; } }
	void IncreaseLanguageSkill(uint8 language_id, uint8 increase = 1);
	virtual uint16 GetSkill(EQ::skills::SkillType skill_id) const;
	uint32 GetRawSkill(EQ::skills::SkillType skill_id) const { if (skill_id <= EQ::skills::HIGHEST_SKILL) { return(m_pp.skills[skill_id]); } return 0; }
	bool HasSkill(EQ::skills::SkillType skill_id) const;
	bool CanHaveSkill(EQ::skills::SkillType skill_id) const;
	void SetSkill(EQ::skills::SkillType skill_num, uint16 value);
	void AddSkill(EQ::skills::SkillType skillid, uint16 value);
	void CheckSpecializeIncrease(uint16 spell_id);
	void CheckSongSkillIncrease(uint16 spell_id);
	bool CheckIncreaseSkill(EQ::skills::SkillType skillid, Mob *against_who, int chancemodi = 0);
	void CheckLanguageSkillIncrease(uint8 language_id, uint8 teacher_skill);
	void SetLanguageSkill(uint8 language_id, uint8 language_skill);
	void SetHoTT(uint32 mobid);
	void ShowSkillsWindow();

	uint16 MaxSkill(EQ::skills::SkillType skill_id, uint8 class_id, uint8 level) const;
	inline uint16 MaxSkill(EQ::skills::SkillType skill_id) const { return MaxSkill(skill_id, GetClass(), GetLevel()); }
	uint8 GetSkillTrainLevel(EQ::skills::SkillType skill_id, uint8 class_id);
	void MaxSkills();

	void SendTradeskillSearchResults(const std::string &query, unsigned long objtype, unsigned long someid);
	void SendTradeskillDetails(uint32 recipe_id);
	bool TradeskillExecute(DBTradeskillRecipe_Struct *spec);
	void CheckIncreaseTradeskill(int16 bonusstat, int16 stat_modifier, float skillup_modifier, uint16 success_modifier, EQ::skills::SkillType tradeskill);
	bool CheckTradeskillLoreConflict(int32 recipe_id);
	void InitInnates();

	void GMKill();
	inline bool IsMedding() const {return medding;}
	inline uint32 GetDuelTarget() const { return duel_target; }
	inline bool IsDueling() const { return duelaccepted; }
	inline void SetDuelTarget(uint32 set_id) { duel_target = set_id; }
	inline void SetDueling(bool duel) { duelaccepted = duel; }
	// use this one instead
	void MemSpell(uint16 spell_id, int slot, bool update_client = true);
	void UnmemSpell(int slot, bool update_client = true);
	void UnmemSpellBySpellID(int32 spell_id);
	void UnmemSpellAll(bool update_client = true);
	int FindEmptyMemSlot();
	uint16 FindMemmedSpellBySlot(int slot);
	int FindMemmedSpellBySpellID(uint16 spell_id);
	int MemmedCount();
	std::vector<int> GetLearnableDisciplines(uint8 min_level = 1, uint8 max_level = 0);
	std::vector<int> GetLearnedDisciplines();
	std::vector<int> GetMemmedSpells();
	std::vector<int> GetScribeableSpells(uint8 min_level = 1, uint8 max_level = 0);
	std::vector<int> GetScribedSpells();
	// defer save used when bulk saving
	void ScribeSpell(uint16 spell_id, int slot, bool update_client = true, bool defer_save = false);
	void SaveSpells();
	void SaveDisciplines();

	// Bulk Scribe/Learn
	uint16 ScribeSpells(uint8 min_level, uint8 max_level);
	uint16 LearnDisciplines(uint8 min_level, uint8 max_level);

	// Configurable Tracking Skill
	uint16 GetClassTrackingDistanceMultiplier(uint16 class_);

	bool CanThisClassTrack();

	// defer save used when bulk saving
	void UnscribeSpell(int slot, bool update_client = true, bool defer_save = false);
	void UnscribeSpellAll(bool update_client = true);
	void UnscribeSpellBySpellID(uint16 spell_id, bool update_client = true);
	void UntrainDisc(int slot, bool update_client = true, bool defer_save = false);
	void UntrainDiscAll(bool update_client = true);
	void UntrainDiscBySpellID(uint16 spell_id, bool update_client = true);
	bool SpellGlobalCheck(uint16 spell_id, uint32 char_id);
	bool SpellBucketCheck(uint16 spell_id, uint32 char_id);
	uint8 GetCharMaxLevelFromQGlobal();
	uint8 GetCharMaxLevelFromBucket();

	void Fling(float value, float target_x, float target_y, float target_z, bool ignore_los = false, bool clip_through_walls = false, bool calculate_speed = false);

	inline bool IsStanding() const { return (playeraction == 0); }
	inline bool IsSitting() const override { return (playeraction == 1); }
	inline bool IsCrouching() const { return (playeraction == 2); }
	inline bool IsBecomeNPC() const { return npcflag; }
	inline uint8 GetBecomeNPCLevel() const { return npclevel; }
	inline void SetBecomeNPC(bool flag) { npcflag = flag; }
	inline void SetBecomeNPCLevel(uint8 level) { npclevel = level; }
	EQStreamInterface* Connection() { return eqs; }
	uint32 GetEquippedItemFromTextureSlot(uint8 material_slot) const; // returns item id
	uint32 GetEquipmentColor(uint8 material_slot) const;
	virtual void UpdateEquipmentLight() { m_Light.Type[EQ::lightsource::LightEquipment] = m_inv.FindBrightestLightType(); m_Light.Level[EQ::lightsource::LightEquipment] = EQ::lightsource::TypeToLevel(m_Light.Type[EQ::lightsource::LightEquipment]); }

	inline bool AutoSplitEnabled() { return m_pp.autosplit != 0; }
	inline bool AutoConsentGroupEnabled() const { return m_pp.groupAutoconsent != 0; }
	inline bool AutoConsentRaidEnabled() const { return m_pp.raidAutoconsent != 0; }
	inline bool AutoConsentGuildEnabled() const { return m_pp.guildAutoconsent != 0; }

	void SummonHorse(uint16 spell_id);
	void SetHorseId(uint16 horseid_in);
	inline void SetControlledMobId(uint16 mob_id_in) { controlled_mob_id = mob_id_in; }
	uint16 GetControlledMobId() const{ return controlled_mob_id; }
	uint16 GetHorseId() const { return horseId; }
	bool CanMedOnHorse();

	bool CanFastRegen() const { return ooc_regen; }

	void NPCSpawn(NPC *target_npc, const char *identifier, uint32 extra = 0);

	void Disarm(Client* disarmer, int chance);
	bool BindWound(Mob* bindmob, bool start, bool fail = false);
	void SetTradeskillObject(Object* object) { m_tradeskill_object = object; }
	Object* GetTradeskillObject() { return m_tradeskill_object; }
	void SendTributes();
	void SendGuildTributes();
	void DoTributeUpdate();
	void SendTributeDetails(uint32 client_id, uint32 tribute_id);
	int32 TributeItem(uint32 slot, uint32 quantity);
	int32 TributeMoney(uint32 platinum);
	void AddTributePoints(int32 ammount);
	void ChangeTributeSettings(TributeInfo_Struct *t);
	void SendTributeTimer();
	void ToggleTribute(bool enabled);
	std::map<uint32, TributeData> GetTributeList();
	uint32 LookupTributeItemID(uint32 tribute_id, uint32 tier);
	void SendPathPacket(const std::vector<FindPerson_Point> &path);

	inline PTimerList &GetPTimers() { return(p_timers); }

	//New AA Methods
	void SendAlternateAdvancementRank(int aa_id, int level);
	void SendAlternateAdvancementTable();
	void SendAlternateAdvancementStats();
	void PurchaseAlternateAdvancementRank(int rank_id);
	bool GrantAlternateAdvancementAbility(int aa_id, int points, bool ignore_cost = false);
	void IncrementAlternateAdvancementRank(int rank_id);
	void ActivateAlternateAdvancementAbility(int rank_id, int target_id);
	void SendAlternateAdvancementPoints();
	void SendAlternateAdvancementTimer(int ability, int begin, int end);
	void SendAlternateAdvancementTimers();
	void ResetAlternateAdvancementTimer(int ability);
	void ResetAlternateAdvancementTimers();
	void ResetOnDeathAlternateAdvancement();

	void SetAAPoints(uint32 points);
	void AddAAPoints(uint32 points);
	bool RemoveAAPoints(uint32 points);
	int GetAAPoints() { return m_pp.aapoints; }
	int GetSpentAA() { return m_pp.aapoints_spent; }
	uint32 GetRequiredAAExperience();
	void AutoGrantAAPoints();
	void GrantAllAAPoints(uint8 unlock_level = 0, bool skip_grant_only = false);
	bool HasAlreadyPurchasedRank(AA::Rank* rank);
	void ListPurchasedAAs(Client *to, std::string search_criteria = std::string());

	bool SendGMCommand(std::string message, bool ignore_status = false);

	void RegisterBug(BugReport_Struct* r);

	std::vector<Mob*> GetApplySpellList(
		ApplySpellType apply_type,
		bool allow_pets,
		bool is_raid_group_only,
		bool allow_bots
	);

	void ApplySpell(
		int spell_id,
		int duration = 0,
		int level = -1,
		ApplySpellType apply_type = ApplySpellType::Solo,
		bool allow_pets = false,
		bool is_raid_group_only = true,
		bool allow_bots = false
	);

	void SetSpellDuration(
		int spell_id,
		int duration = 0,
		int level = -1,
		ApplySpellType apply_type = ApplySpellType::Solo,
		bool allow_pets = false,
		bool is_raid_group_only = true,
		bool allow_bots = false
	);

	//old AA methods that we still use
	void ResetAA();
	void ResetLeadershipAA();
	void RefundAA();
	void SendClearLeadershipAA();
	void SendClearPlayerAA();
	inline uint32 GetAAXP() const { return m_pp.expAA; }
	inline uint32 GetAAPercent() const { return m_epp.perAA; }
	void SetAATitle(std::string title);
	void SetTitleSuffix(std::string suffix);
	void MemorizeSpell(uint32 slot, uint32 spell_id, uint32 scribing, uint32 reduction = 0);

	int GetAAEXPPercentage();
	int GetEXPPercentage();

	// Item methods
	void UseAugmentContainer(int container_slot);
	uint32 NukeItem(uint32 itemnum, uint8 where_to_check =
			(invWhereWorn | invWherePersonal | invWhereBank | invWhereSharedBank | invWhereTrading | invWhereCursor));
	void SetTint(int16 slot_id, uint32 color);
	void SetTint(int16 slot_id, EQ::textures::Tint_Struct& color);
	void SetMaterial(int16 slot_id, uint32 item_id);
	void Undye();
	int32 GetItemIDAt(int16 slot_id);
	int32 GetAugmentIDAt(int16 slot_id, uint8 augslot);
	bool PutItemInInventory(int16 slot_id, const EQ::ItemInstance& inst, bool client_update = false);
	bool PutItemInInventoryWithStacking(EQ::ItemInstance* inst);
	bool FindNumberOfFreeInventorySlotsWithSizeCheck(std::vector<BuyerLineTradeItems_Struct> items);
	bool PushItemOnCursor(const EQ::ItemInstance& inst, bool client_update = false);
	void SendCursorBuffer();
	void DeleteItemInInventory(int16 slot_id, int16 quantity = 0, bool client_update = false, bool update_db = true);
	uint32 CountItem(uint32 item_id);
	void ResetItemCooldown(uint32 item_id);
	void SetItemCooldown(uint32 item_id, bool use_saved_timer = false, uint32 in_seconds = 1);
	uint32 GetItemCooldown(uint32 item_id);
	void RemoveItem(uint32 item_id, uint32 quantity = 1);
	void RemoveItemBySerialNumber(uint32 serial_number, uint32 quantity = 1);
	bool SwapItem(MoveItem_Struct* move_in);
	void SwapItemResync(MoveItem_Struct* move_slots);
	void PutLootInInventory(int16 slot_id, const EQ::ItemInstance &inst, LootItem** bag_item_data = 0);
	bool AutoPutLootInInventory(EQ::ItemInstance& inst, bool try_worn = false, bool try_cursor = true, LootItem** bag_item_data = 0);
	bool SummonItem(uint32 item_id, int16 charges = -1, uint32 aug1 = 0, uint32 aug2 = 0, uint32 aug3 = 0, uint32 aug4 = 0, uint32 aug5 = 0, uint32 aug6 = 0, bool attuned = false, uint16 to_slot = EQ::invslot::slotCursor, uint32 ornament_icon = 0, uint32 ornament_idfile = 0, uint32 ornament_hero_model = 0);
	void SummonItemIntoInventory(uint32 item_id, int16 charges = -1, uint32 aug1 = 0, uint32 aug2 = 0, uint32 aug3 = 0, uint32 aug4 = 0, uint32 aug5 = 0, uint32 aug6 = 0, bool is_attuned = false);
	void SummonBaggedItems(uint32 bag_item_id, const std::vector<LootItem>& bag_items);
	void SetStats(uint8 type,int16 set_val);
	void IncStats(uint8 type,int16 increase_val);
	void DropItem(int16 slot_id, bool recurse = true);
	bool HasItemOnCorpse(uint32 item_id);

	bool IsAugmentRestricted(uint8 item_type, uint32 augment_restriction);

	int GetItemLinkHash(const EQ::ItemInstance* inst); // move to ItemData..or make use of the pre-calculated database field

	void SendItemLink(const EQ::ItemInstance* inst, bool sendtoall=false);
	void SendLootItemInPacket(const EQ::ItemInstance* inst, int16 slot_id);
	void SendItemPacket(int16 slot_id, const EQ::ItemInstance* inst, ItemPacketType packet_type);
	bool IsValidSlot(uint32 slot);
	bool IsBankSlot(uint32 slot);

	bool IsTrader() const { return trader; }
	void SetTrader(bool status) { trader = status; }
	uint16 GetTraderID() { return trader_id; }
	void SetTraderID(uint16 id) { trader_id = id; }
	void SetTraderCount(uint32 no) { m_trader_count = no; }
	uint32 GetTraderCount() { return m_trader_count; }
	void IncrementTraderCount() { m_trader_count += 1; }
	void DecrementTraderCount() { m_trader_count > 0 ? m_trader_count -= 1 : m_trader_count = 0; }
	void SetTraderTransactionDate() { m_trader_transaction_date = time(nullptr); }
	time_t GetTraderTransactionDate() { return m_trader_transaction_date; }

	eqFilterMode GetFilter(eqFilterType filter_id) const { return ClientFilters[filter_id]; }
	void SetFilter(eqFilterType filter_id, eqFilterMode filter_mode) { ClientFilters[filter_id] = filter_mode; }

	void CancelSneakHide();
	void BreakInvis();
	void BreakSneakWhenCastOn(Mob* caster, bool IsResisted);
	void BreakFeignDeathWhenCastOn(bool IsResisted);
	void LeaveGroup();

	bool Hungry() const {if (GetGM() || !RuleB(Character, EnableFoodRequirement)) return false; return m_pp.hunger_level <= 3000;}
	bool Thirsty() const {if (GetGM() || !RuleB(Character, EnableFoodRequirement)) return false; return m_pp.thirst_level <= 3000;}
	int32 GetHunger() const { return m_pp.hunger_level; }
	int32 GetThirst() const { return m_pp.thirst_level; }
	void SetHunger(int32 in_hunger);
	void SetThirst(int32 in_thirst);
	void SetConsumption(int32 in_hunger, int32 in_thirst);
	bool IsStarved() const { if (GetGM() || !RuleB(Character, EnableFoodRequirement) || !RuleB(Character, EnableHungerPenalties)) return false; return m_pp.hunger_level == 0 || m_pp.thirst_level == 0; }
	int32 GetIntoxication() const { return m_pp.intoxication; }

	bool CheckTradeLoreConflict(Client* other);
	bool CheckTradeNonDroppable();
	void LinkDead();
	bool CheckDoubleAttack();
	bool CheckTripleAttack();
	bool CheckDoubleRangedAttack();
	bool CheckDualWield();

	//remove charges/multiple objects from inventory:
	//bool DecreaseByType(uint32 type, uint8 amt);
	bool DecreaseByID(uint32 type, int16 quantity);
	uint8 SlotConvert2(uint8 slot); //Maybe not needed.
	void Escape(); //keep or quest function
	void DisenchantSummonedBags(bool client_update = true);
	void RemoveNoRent(bool client_update = true);
	void RemoveDuplicateLore();
	void MoveSlotNotAllowed(bool client_update = true);
	virtual bool RangedAttack(Mob* other, bool CanDoubleAttack = false);
	virtual void ThrowingAttack(Mob* other, bool CanDoubleAttack = false);
	void DoClassAttacks(Mob *ca_target, uint16 skill = -1, bool IsRiposte=false);

	void ClearZoneFlag(uint32 zone_id);
	inline std::set<uint32> GetZoneFlags() { return zone_flags; } ;
	bool HasZoneFlag(uint32 zone_id) const;
	void LoadZoneFlags();
	void SendZoneFlagInfo(Client *to) const;
	void SetZoneFlag(uint32 zone_id);

	void ClearPEQZoneFlag(uint32 zone_id);
	inline std::set<uint32> GetPEQZoneFlags() { return peqzone_flags; };
	bool HasPEQZoneFlag(uint32 zone_id) const;
	void LoadPEQZoneFlags();
	void SendPEQZoneFlagInfo(Client *to) const;
	void SetPEQZoneFlag(uint32 zone_id);

	bool CanFish();
	void GoFish(bool guarantee = false, bool use_bait = true);
	void ForageItem(bool guarantee = false);
	//Calculate vendor price modifier based on CHA: (reverse==selling)
	float CalcClassicPriceMod(Mob* other = 0, bool reverse = false);
	float CalcNewPriceMod(Mob* other = 0, bool reverse = false);
	float CalcPriceMod(Mob* other = 0, bool reverse = false);
	void ResetTrade();
	void DropInst(const EQ::ItemInstance* inst);
	bool TrainDiscipline(uint32 itemid);
	bool MemorizeSpellFromItem(uint32 item_id);
	void TrainDiscBySpellID(int32 spell_id);
	uint32 GetDisciplineTimer(uint32 timer_id);
	void ResetAllDisciplineTimers();
	int GetDiscSlotBySpellID(int32 spellid);
	void ResetDisciplineTimer(uint32 timer_id);
	void SendDisciplineUpdate();
	void SendDisciplineTimer(uint32 timer_id, uint32 duration);
	bool UseDiscipline(uint32 spell_id, uint32 target);
	bool HasDisciplineLearned(uint16 spell_id);

	void SetLinkedSpellReuseTimer(uint32 timer_id, uint32 duration);
	bool IsLinkedSpellReuseTimerReady(uint32 timer_id);

	void ResetCastbarCooldownBySlot(int slot);
	void ResetAllCastbarCooldowns();
	void ResetCastbarCooldownBySpellID(uint32 spell_id);

	bool CheckTitle(int title_set);
	void EnableTitle(int title_set, bool insert = true);
	const std::vector<PlayerTitlesetsRepository::PlayerTitlesets>& GetTitles() { return m_player_title_sets; };
	void RemoveTitle(int title_set);

	void EnteringMessages(Client* client);
	void SendRules();

	const bool GetGMSpeed() const { return (gmspeed > 0); }
	const bool GetGMInvul() const { return gminvul; }
	bool CanUseReport;

	const std::string GetAutoLoginCharacterName();
	bool SetAutoLoginCharacterName(const std::string& character_name);

	//This is used to later set the buff duration of the spell, in slot to duration.
	//Doesn't appear to work directly after the client recieves an action packet.
	void SendBuffDurationPacket(Buffs_Struct &buff, int slot);
	void SendBuffNumHitPacket(Buffs_Struct &buff, int slot);

	void ProcessInspectRequest(Client* requestee, Client* requester);
	bool ClientFinishedLoading() { return (conn_state == ClientConnectFinished); }
	int FindSpellBookSlotBySpellID(uint16 spellid);
	uint32 GetSpellIDByBookSlot(int book_slot);
	int GetNextAvailableSpellBookSlot(int starting_slot = 0);
	int GetNextAvailableDisciplineSlot(int starting_slot = 0);
	inline uint32 GetSpellByBookSlot(int book_slot) { return m_pp.spell_book[book_slot]; }
	inline bool HasSpellScribed(int spellid) { return FindSpellBookSlotBySpellID(spellid) != -1; }
	uint32 GetHighestScribedSpellinSpellGroup(uint32 spell_group);
	std::unordered_map<uint32, std::vector<uint16>> LoadSpellGroupCache(uint8 min_level, uint8 max_level);
	uint16 GetMaxSkillAfterSpecializationRules(EQ::skills::SkillType skillid, uint16 maxSkill);
	void SendPopupToClient(const char *Title, const char *Text, uint32 PopupID = 0, uint32 Buttons = 0, uint32 Duration = 0);
	void SendFullPopup(const char *Title, const char *Text, uint32 PopupID = 0, uint32 NegativeID = 0, uint32 Buttons = 0, uint32 Duration = 0, const char *ButtonName0 = 0, const char *ButtonName1 = 0, uint32 SoundControls = 0);
	void SendWindow(uint32 button_one_id, uint32 button_two_id, uint32 button_type, const char* button_one_text, const char* button_two_text, uint32 duration, int title_type, Mob* target, const char* title, const char* text, ...);
	bool PendingTranslocate;
	time_t TranslocateTime;
	bool PendingSacrifice;
	uint16 sacrifice_caster_id;
	PendingTranslocate_Struct PendingTranslocateData;
	void SendOPTranslocateConfirm(Mob *Caster, uint16 SpellID);

	// Help Window
	std::string SendBotCommandHelpWindow(const BotCommandHelpParams& params);
	std::string GetCommandHelpHeader(std::string msg, std::string color);
	std::string SplitCommandHelpText(std::vector<std::string> msg, std::string color, uint16 max_length, std::string secondary_color = "");
	void SendSpellTypePrompts(bool commanded_types = false, bool client_only_types = false);

	// Task System Methods
	inline void LoadClientSharedCompletedTasks()
	{
		std::string query = fmt::format(R"(
			SELECT
			cst.task_id
			FROM completed_shared_task_members cstm
			JOIN completed_shared_tasks cst ON cstm.shared_task_id = cst.id
			WHERE cstm.character_id = {}
			GROUP BY cst.task_id;
		)", CharacterID());

		auto results = database.QueryDatabase(query);
		if (!results.Success()) {
			return;
		}

		m_completed_shared_tasks.clear();

		for (auto row = results.begin(); row != results.end(); ++row) {
			m_completed_shared_tasks.push_back(std::stoi(row[0]));
		}
	};
	inline std::vector<uint32_t> GetCompletedSharedTasks() const { return m_completed_shared_tasks; };
	void LoadClientTaskState();
	void RemoveClientTaskState();
	void SendTaskActivityComplete(int task_id, int activity_id, int task_index, TaskType task_type, int task_incomplete=1);
	void SendTaskFailed(int task_id, int task_index, TaskType task_type);
	void SendTaskComplete(int task_index);
	bool HasTaskRequestCooldownTimer();
	void SendTaskRequestCooldownTimerMessage();
	void StartTaskRequestCooldownTimer();
	inline ClientTaskState *GetTaskState() const { return task_state; }
	inline bool HasTaskState() { if (task_state) { return true; } return false; }
	inline void CancelTask(int task_index, TaskType task_type)
	{
		if (task_state) {
			task_state->CancelTask(
				this,
				task_index,
				task_type
			);
		}
	}
	inline bool SaveTaskState()
	{
		return TaskManager::Instance()->SaveClientState(this, task_state);
	}
	inline bool IsTaskStateLoaded() { return task_state != nullptr; }
	inline bool IsTaskActive(int task_id) { return task_state != nullptr && task_state->IsTaskActive(task_id); }
	inline bool IsTaskActivityActive(int task_id, int activity_id)
	{
		return task_state != nullptr &&
			   task_state->IsTaskActivityActive(
				   task_id,
				   activity_id
			   );
	}
	inline ActivityState GetTaskActivityState(TaskType task_type, int index, int activity_id)
	{
		return (task_state ? task_state->GetTaskActivityState(task_type, index, activity_id) : ActivityHidden);
	}
	inline void UpdateTaskActivity(
		int task_id,
		int activity_id,
		int count,
		bool ignore_quest_update = false
	)
	{
		if (task_state) {
			task_state->UpdateTaskActivity(this, task_id, activity_id, count, ignore_quest_update);
		}
	}
	inline void RemoveTaskByTaskID(uint32 task_id) {
		if (task_state) {
			task_state->RemoveTaskByTaskID(this, task_id);
		}
	}
	inline void ResetTaskActivity(int task_id, int activity_id)
	{
		if (task_state) {
			task_state->ResetTaskActivity(
				this,
				task_id,
				activity_id
			);
		}
	}
	inline void UpdateTasksForItem(TaskActivityType type, int item_id, int count = 1)
	{
		if (task_state) {
			task_state->UpdateTasksForItem(this, type, item_id, count);
		}
	}
	inline void UpdateTasksOnLoot(Corpse* corpse, int item_id, int count = 1)
	{
		if (task_state) {
			task_state->UpdateTasksOnLoot(this, corpse, item_id, count);
		}
	}
	inline void UpdateTasksOnExplore(const glm::vec4& pos)
	{
		if (task_state) {
			task_state->UpdateTasksOnExplore(this, pos);
		}
	}
	inline bool UpdateTasksOnSpeakWith(NPC* npc)
	{
		return task_state && task_state->UpdateTasksOnSpeakWith(this, npc);
	}
	inline bool UpdateTasksOnDeliver(std::vector<EQ::ItemInstance*>& items, Trade& trade, NPC* npc)
	{
		return task_state && task_state->UpdateTasksOnDeliver(this, items, trade, npc);
	}
	void UpdateTasksOnTouchSwitch(int dz_switch_id)
	{
		if (task_state) { task_state->UpdateTasksOnTouch(this, dz_switch_id); }
	}
	inline void TaskSetSelector(Mob* mob, int task_set_id, bool ignore_cooldown)
	{
		if (task_state) {
			TaskManager::Instance()->TaskSetSelector(this, mob, task_set_id, ignore_cooldown);
		}
	}
	inline void TaskQuestSetSelector(Mob* mob, const std::vector<int>& tasks, bool ignore_cooldown)
	{
		if (task_state) {
			TaskManager::Instance()->TaskQuestSetSelector(this, mob, tasks, ignore_cooldown);
		}
	}
	inline void EnableTask(int task_count, int *task_list)
	{
		if (task_state) {
			task_state->EnableTask(
				CharacterID(),
				task_count,
				task_list
			);
		}
	}
	inline void DisableTask(int task_count, int *task_list)
	{
		if (task_state) {
			task_state->DisableTask(
				CharacterID(),
				task_count,
				task_list
			);
		}
	}
	inline bool IsTaskEnabled(int task_id) {
		return task_state != nullptr && task_state->IsTaskEnabled(task_id);
	}
	inline void ProcessTaskProximities(float x, float y, float z)
	{
		if (task_state) {
			task_state->ProcessTaskProximities(this, x, y, z);
		}
	}
	inline void AssignTask(
		int task_id,
		int npc_id = 0,
		bool enforce_level_requirement = false
	) {
		if (task_state) {
			task_state->AcceptNewTask(this, task_id, npc_id, std::time(nullptr), enforce_level_requirement);
		}
	}
	inline int ActiveSpeakTask(NPC* npc)
	{
		if (task_state) {
			return task_state->ActiveSpeakTask(this, npc);
		}
		else {
			return 0;
		}
	}
	inline int ActiveSpeakActivity(NPC* npc, int task_id)
	{
		if (task_state) {
			return task_state->ActiveSpeakActivity(this, npc, task_id);
		}
		else { return 0; }
	}
	inline bool CompleteTask(uint32 task_id)
	{
		return task_state ? task_state->CompleteTask(this, task_id) : false;
	}
	bool UncompleteTask(int task_id);
	inline void FailTask(int task_id) { if (task_state) { task_state->FailTask(this, task_id); }}
	inline int TaskTimeLeft(int task_id) { return (task_state ? task_state->TaskTimeLeft(task_id) : 0); }
	inline int EnabledTaskCount(int task_set_id)
	{
		return (task_state ? task_state->EnabledTaskCount(task_set_id) : -1);
	}
	inline bool IsTaskCompleted(int task_id)
	{
		return (task_state ? task_state->IsTaskCompleted(task_id, this) : false);
	}
	inline bool AreTasksCompleted(std::vector<int> task_ids)
	{
		return (task_state ? task_state->AreTasksCompleted(task_ids) : false);
	}
	inline void ShowClientTasks(Client *client) { if (task_state) { task_state->ShowClientTasks(this, client); }}
	inline void CancelAllTasks() { if (task_state) { task_state->CancelAllTasks(this); }}
	inline int GetActiveTaskCount() { return (task_state ? task_state->GetActiveTaskCount() : 0); }
	inline int GetActiveTaskID(int index) { return (task_state ? task_state->GetActiveTaskID(index) : -1); }
	inline int GetTaskStartTime(TaskType task_type, int index)
	{
		return (task_state ? task_state->GetTaskStartTime(
			task_type,
			index
		) : -1);
	}
	inline bool IsTaskActivityCompleted(TaskType task_type, int index, int activity_id)
	{
		return task_state != nullptr && task_state->IsTaskActivityCompleted(task_type, index, activity_id);
	}
	inline int GetTaskActivityDoneCount(TaskType task_type, int client_task_index, int activity_id)
	{
		return (task_state ? task_state->GetTaskActivityDoneCount(task_type, client_task_index, activity_id) : 0);
	}
	inline int GetTaskActivityDoneCountFromTaskID(int task_id, int activity_id)
	{
		return (task_state ? task_state->GetTaskActivityDoneCountFromTaskID(task_id, activity_id) : 0);
	}
	inline int ActiveTasksInSet(int task_set_id)
	{
		return (task_state ? task_state->ActiveTasksInSet(task_set_id) : 0);
	}
	inline int CompletedTasksInSet(int task_set_id)
	{
		return (task_state ? task_state->CompletedTasksInSet(task_set_id) : 0);
	}
	void PurgeTaskTimers();
	void LockSharedTask(bool lock) { if (task_state) { task_state->LockSharedTask(this, lock); } }
	void EndSharedTask(bool fail = false) { if (task_state) { task_state->EndSharedTask(this, fail); } }

	// shared task shims / middleware
	// these variables are used as a shim to intercept normal localized task functionality
	// and pipe it into zone -> world and back to world -> zone
	// world is authoritative
	bool m_requesting_shared_task        = false;
	bool m_shared_task_update            = false;
	bool m_requested_shared_task_removal = false;

	std::vector<Client*> GetPartyMembers();

	inline const EQ::versions::ClientVersion ClientVersion() const { return m_ClientVersion; }
	inline const uint32 ClientVersionBit() const { return m_ClientVersionBit; }
	inline void SetClientVersion(EQ::versions::ClientVersion client_version) { m_ClientVersion = client_version; }

	/** Adventure Stuff **/
	void SendAdventureError(const char *error);
	void SendAdventureDetails();
	void SendAdventureCount(uint32 count, uint32 total);
	void NewAdventure(int id, int theme, const char *text, int member_count, const char *members);
	bool IsOnAdventure();
	void LeaveAdventure();
	void AdventureFinish(bool win, int theme, int points);
	void SetAdventureData(char *data) { adv_data = data; }
	void ClearAdventureData() { safe_delete(adv_data); }
	bool HasAdventureData() { return adv_data != nullptr; }
	void ClearCurrentAdventure();
	void PendingAdventureRequest() { adventure_request_timer = new Timer(8000); }
	bool GetPendingAdventureRequest() const { return (adventure_request_timer != nullptr); }
	void ClearPendingAdventureRequest() { safe_delete(adventure_request_timer); }
	void PendingAdventureCreate() { adventure_create_timer = new Timer(8000); }
	bool GetPendingAdventureCreate() const { return (adventure_create_timer != nullptr); }
	void ClearPendingAdventureCreate() { safe_delete(adventure_create_timer); }
	void PendingAdventureLeave() { adventure_leave_timer = new Timer(8000); }
	bool GetPendingAdventureLeave() const { return (adventure_leave_timer != nullptr); }
	void ClearPendingAdventureLeave() { safe_delete(adventure_leave_timer); }
	void PendingAdventureDoorClick() { adventure_door_timer = new Timer(8000); }
	bool GetPendingAdventureDoorClick() const { return (adventure_door_timer != nullptr); }
	void ClearPendingAdventureDoorClick() { safe_delete(adventure_door_timer); }
	void ClearPendingAdventureData();

	bool CanEnterZone(const std::string& zone_short_name = "", int16 instance_version = -1);

	uint32 GetAggroCount();
	void IncrementAggroCount(bool raid_target = false);
	void DecrementAggroCount();
	void SendPVPStats();
	void SendDisciplineTimers();
	void SendRespawnBinds();

	uint32 GetLDoNWins() { return (m_pp.ldon_wins_guk + m_pp.ldon_wins_mir + m_pp.ldon_wins_mmc + m_pp.ldon_wins_ruj + m_pp.ldon_wins_tak); }
	uint32 GetLDoNLosses() { return (m_pp.ldon_losses_guk + m_pp.ldon_losses_mir + m_pp.ldon_losses_mmc + m_pp.ldon_losses_ruj + m_pp.ldon_losses_tak); }
	uint32 GetLDoNWinsTheme(uint32 t);
	uint32 GetLDoNLossesTheme(uint32 t);
	uint32 GetLDoNPointsTheme(uint32 t);
	void UpdateLDoNWinLoss(uint32 theme_id, bool win = false, bool remove = false);
	void CheckLDoNHail(NPC* n);
	void CheckEmoteHail(NPC* n, const char* message);

	void HandleLDoNOpen(NPC *target);
	void HandleLDoNSenseTraps(NPC *target, uint16 skill, uint8 type);
	void HandleLDoNDisarm(NPC *target, uint16 skill, uint8 type);
	void HandleLDoNPickLock(NPC *target, uint16 skill, uint8 type);
	int LDoNChest_SkillCheck(NPC *target, int skill);

	void MarkSingleCompassLoc(float in_x, float in_y, float in_z, uint8 count=1);

	// cross zone client messaging helpers (null client argument will fallback to messaging by name)
	static void SendCrossZoneMessage(
		Client* client, const std::string& client_name, uint16_t chat_type, const std::string& message);
	static void SendCrossZoneMessageString(
		Client* client, const std::string& client_name, uint16_t chat_type,
		uint32_t string_id, const std::initializer_list<std::string>& arguments = {});

	void AddDzLockout(const DzLockout& lockout, bool update_db = false);
	void AddDzLockout(const std::string& expedition, const std::string& event, uint32_t duration, std::string uuid = {});
	void AddDzLockoutDuration(const DzLockout& lockout, int seconds, const std::string& uuid = {}, bool update_db = false);
	DynamicZone* CreateExpedition(DynamicZone& dz, bool silent = false);
	DynamicZone* CreateExpedition(uint32 zone_id, uint32 version, uint32 duration, const std::string& name, uint32 min_players, uint32 max_players, bool silent = false);
	DynamicZone* CreateExpeditionFromTemplate(uint32_t dz_template_id);
	DynamicZone* GetExpedition() const;
	uint32 GetExpeditionID() const;
	const DzLockout* GetDzLockout(const std::string& expedition, const std::string& event) const;
	const std::vector<DzLockout>& GetDzLockouts() const { return m_dz_lockouts; };
	std::vector<DzLockout> GetDzLockouts(const std::string& expedition);
	uint32 GetPendingDzInviteID() const { return m_dz_invite.dz_id; }
	void SetPendingDzInvite(const ExpeditionInvite& invite) { m_dz_invite = invite; }
	void RequestPendingDzInvite() const;
	bool HasDzLockout(const std::string& expedition, const std::string& event) const;
	void RemoveDzLockouts(const std::string& expedition, bool update_db = false);
	void RemoveDzLockout(const std::string& expedition, const std::string& event, bool update_db = false);
	void SendDzLockoutTimers();
	void DzListTimers();
	void SetDzRemovalTimer(bool enable_timer);
	void SendDzCompassUpdate();
	void GoToDzSafeReturnOrBind(const DynamicZone* dynamic_zone);
	void MovePCDynamicZone(uint32 zone_id, int zone_version = -1, bool msg_if_invalid = false);
	void MovePCDynamicZone(const std::string& zone_name, int zone_version = -1, bool msg_if_invalid = false);
	bool TryMovePCDynamicZoneSwitch(int dz_switch_id);
	std::vector<DynamicZone*> GetDynamicZones(uint32_t zone_id = 0, int zone_version = -1);
	std::unique_ptr<EQApplicationPacket> CreateDzSwitchListPacket(const std::vector<DynamicZone*>& dzs);
	std::unique_ptr<EQApplicationPacket> CreateCompassPacket(const std::vector<DynamicZoneCompassEntry_Struct>& entries);
	void AddDynamicZoneID(uint32_t dz_id);
	void RemoveDynamicZoneID(uint32_t dz_id);
	void SendDynamicZoneUpdates();
	void SetDynamicZoneMemberStatus(DynamicZoneMemberStatus status);
	void CreateTaskDynamicZone(int task_id, DynamicZone& dz_request);

	void CalcItemScale();
	bool CalcItemScale(uint32 slot_x, uint32 slot_y); // behavior change: 'slot_y' is now [RANGE]_END and not [RANGE]_END + 1
	void DoItemEnterZone();
	bool DoItemEnterZone(uint32 slot_x, uint32 slot_y); // behavior change: 'slot_y' is now [RANGE]_END and not [RANGE]_END + 1
	void SummonAndRezzAllCorpses();
	void SummonAllCorpses(const glm::vec4& position);
	void DepopAllCorpses();
	void DepopPlayerCorpse(uint32 dbid);
	void BuryPlayerCorpses();
	int64 GetCorpseCount() { return database.GetCharacterCorpseCount(CharacterID()); }
	uint32 GetCorpseID(int corpse) { return database.GetCharacterCorpseID(CharacterID(), corpse); }
	uint32 GetCorpseItemAt(int corpse_id, int slot_id) {
		if (!corpse_id) {
			return 0;
		}
		return database.GetCharacterCorpseItemAt(corpse_id, slot_id);
	}

	void SuspendMinion(int value);
	void Doppelganger(uint16 spell_id, Mob *target, const char *name_override, int pet_count, int pet_duration);
	void NotifyNewTitlesAvailable();
	void Signal(int signal_id);
	void SendPayload(int payload_id, std::string payload_value = std::string());
	Mob *GetBindSightTarget() { return bind_sight_target; }
	void SetBindSightTarget(Mob *n) { bind_sight_target = n; }
	const uint16 GetBoatID() const { return controlling_boat_id; }
	void SendRewards();
	bool TryReward(uint32 claim_id);
	QGlobalCache *GetQGlobals() { return qGlobals; }
	QGlobalCache *CreateQGlobals() { qGlobals = new QGlobalCache(); return qGlobals; }
	void GuildBankAck();
	void GuildBankDepositAck(bool Fail, int8 action);
	inline bool IsGuildBanker() { return GuildBanker; }
	inline void SetGuildBanker(bool banker) { GuildBanker = banker; }
	void ClearGuildBank();
	void SendGroupCreatePacket();
	void SendGroupLeaderChangePacket(const char *LeaderName);
	void SendGroupJoinAcknowledge();
	void DoTracking();
	inline bool IsTracking() { return (TrackingID > 0); }
	inline void SetPendingGuildInvitation(bool inPendingGuildInvitation) { PendingGuildInvitation = inPendingGuildInvitation; }
	inline bool GetPendingGuildInvitation() { return PendingGuildInvitation; }
	void LocateCorpse();
	void SendTargetCommand(uint32 EntityID);
	bool MoveItemToInventory(EQ::ItemInstance *BInst, bool UpdateClient = false);
	void HandleRespawnFromHover(uint32 Option);
	bool IsHoveringForRespawn() { return RespawnFromHoverTimer.Enabled(); }
	std::list<RespawnOption> respawn_options;
	void AddRespawnOption(std::string option_name, uint32 zoneid, uint16 instance_id, float x, float y, float z, float h = 0, bool initial_selection = false, int8 position = -1);
	bool RemoveRespawnOption(std::string option_name);
	bool RemoveRespawnOption(uint8 position);
	void ClearRespawnOptions() { respawn_options.clear(); }
	void SetPendingRezzData(int XP, uint32 DBID, uint16 SpellID, const char *CorpseName) { PendingRezzXP = XP; PendingRezzDBID = DBID; PendingRezzSpellID = SpellID; PendingRezzCorpseName = CorpseName; }
	bool IsRezzPending() { return PendingRezzSpellID > 0; }
	void ClearHover();
	inline bool IsBlockedBuff(int32 SpellID) { return PlayerBlockedBuffs.find(SpellID) != PlayerBlockedBuffs.end(); }
	inline bool IsBlockedPetBuff(int32 SpellID) { return PetBlockedBuffs.find(SpellID) != PetBlockedBuffs.end(); }
	bool IsDraggingCorpse(uint16 CorpseID);
	inline bool IsDraggingCorpse() { return (DraggedCorpses.size() > 0); }
	void DragCorpses();
	inline void ClearDraggedCorpses() { DraggedCorpses.clear(); }
	void ConsentCorpses(std::string consent_name, bool deny = false);
	void SendAltCurrencies();
	void SetAlternateCurrencyValue(uint32 currency_id, uint32 new_amount);
	int AddAlternateCurrencyValue(uint32 currency_id, int amount, bool is_scripted = false);
	bool RemoveAlternateCurrencyValue(uint32 currency_id, uint32 amount);
	void SendAlternateCurrencyValues();
	void SendAlternateCurrencyValue(uint32 currency_id, bool send_if_null = true);
	uint32 GetAlternateCurrencyValue(uint32 currency_id) const;
	void ProcessAlternateCurrencyQueue();
	void OpenLFGuildWindow();
	void HandleLFGuildResponse(ServerPacket *pack);
	void SendLFGuildStatus();
	void SendGuildLFGuildStatus();
	inline bool XTargettingAvailable() const { return ((m_ClientVersionBit & EQ::versions::maskUFAndLater) && RuleB(Character, EnableXTargetting)); }
	inline uint8 GetMaxXTargets() const { return MaxXTargets; }
	void SetMaxXTargets(uint8 NewMax);
	bool IsXTarget(const Mob *m) const;
	bool IsClientXTarget(const Client *c) const;
	void UpdateClientXTarget(Client *c);
	void UpdateXTargetType(XTargetType Type, Mob *m, const char *Name = nullptr);
	void AddAutoXTarget(Mob *m, bool send = true);
	void RemoveXTarget(Mob *m, bool OnlyAutoSlots);
	void SendXTargetPacket(uint32 Slot, Mob *m);
	void SendXTargetUpdates();
	void RemoveGroupXTargets();
	void RemoveAutoXTargets();
	void ShowXTargets(Client *c);
	inline XTargetAutoHaters *GetXTargetAutoMgr() { return m_activeautohatermgr; } // will be either raid or group or self
	inline void SetXTargetAutoMgr(XTargetAutoHaters *in) { if (in) m_activeautohatermgr = in; else m_activeautohatermgr = &m_autohatermgr; }
	inline void SetDirtyAutoHaters() { m_dirtyautohaters = true; }
	void ProcessXTargetAutoHaters(); // fixes up our auto haters
	void JoinGroupXTargets(Group *g);
	void LeaveGroupXTargets(Group *g);
	void LeaveRaidXTargets(Raid *r);
	void ClearXTargets();
	bool GroupFollow(Client* inviter);
	inline bool  GetRunMode() const { return runmode; }

	virtual bool CheckWaterAutoFireLoS(Mob* m);

	void SendReloadCommandMessages();

	void SendItemRecastTimer(int32 recast_type, uint32 recast_delay = 0, bool in_ignore_casting_requirement = false);
	void SetItemRecastTimer(int32 spell_id, uint32 inventory_slot);
	void DeleteItemRecastTimer(uint32 item_id);
	bool HasItemRecastTimer(int32 spell_id, uint32 inventory_slot);

	inline bool AggroMeterAvailable() const { return ((m_ClientVersionBit & EQ::versions::maskRoF2AndLater)) && RuleB(Character, EnableAggroMeter); } // RoF untested
	inline void SetAggroMeterLock(int in) { m_aggrometer.set_lock_id(in); }

	void ProcessAggroMeter(); // builds packet and sends

	void InitializeMercInfo();
	bool CheckCanSpawnMerc(uint32 template_id);
	bool CheckCanHireMerc(Mob* merchant, uint32 template_id);
	bool CheckCanRetainMerc(uint32 upkeep);
	bool CheckCanUnsuspendMerc();
	bool DismissMerc(uint32 MercID);
	bool MercOnlyOrNoGroup();
	inline uint32 GetMercenaryID() const { return mercid; }
	inline uint8 GetMercSlot() const { return mercSlot; }
	void SetMercID( uint32 newmercid) { mercid = newmercid; }
	void SetMercSlot( uint8 newmercslot) { mercSlot = newmercslot; }
	Merc* GetMerc();
	MercInfo& GetMercInfo(uint8 slot) { return m_mercinfo[slot]; }
	MercInfo& GetMercInfo() { return m_mercinfo[mercSlot]; }
	uint8 GetNumberOfMercenaries();
	void SetMerc(Merc* newmerc);
	void SendMercResponsePackets(uint32 ResponseType);
	void SendMercMerchantResponsePacket(int32 response_type);
	void SendMercenaryUnknownPacket(uint8 type);
	void SendMercenaryUnsuspendPacket(uint8 type);
	void SendMercTimer(Merc* merc = nullptr);
	void SendMercTimerPacket(int32 entity_id, int32 merc_state, int32 suspended_time, int32 update_interval = 900000, int32 unk01 = 180000);
	void SendMercSuspendResponsePacket(uint32 suspended_time);
	void SendMercAssignPacket(uint32 entityID, uint32 unk01, uint32 unk02);
	void SendMercPersonalInfo();
	void SendClearMercInfo();
	void SuspendMercCommand();
	void SpawnMercOnZone();
	void SpawnMerc(Merc* merc, bool setMaxStats);
	void UpdateMercTimer();
	void UpdateMercLevel();
	void CheckMercSuspendTimer();
	Timer* GetMercTimer() { return &merc_timer; };
	Timer* GetPickLockTimer() { return &pick_lock_timer; };

	void SendWebLink(const char* website);
	void SendMarqueeMessage(uint32 type, std::string message, uint32 duration = 3000);
	void SendMarqueeMessage(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, std::string message);
	void SendSpellAnim(uint16 targetid, uint16 spell_id);

	void DuplicateLoreMessage(uint32 ItemID);
	void GarbleMessage(char *, uint8);

	void ItemTimerCheck();
	void TryItemTimer(int slot);
	void SendItemScale(EQ::ItemInstance *inst);

	int32 GetActSTR() { return( std::min(GetMaxSTR(), GetSTR()) ); }
	int32 GetActSTA() { return( std::min(GetMaxSTA(), GetSTA()) ); }
	int32 GetActDEX() { return( std::min(GetMaxDEX(), GetDEX()) ); }
	int32 GetActAGI() { return( std::min(GetMaxAGI(), GetAGI()) ); }
	int32 GetActINT() { return( std::min(GetMaxINT(), GetINT()) ); }
	int32 GetActWIS() { return( std::min(GetMaxWIS(), GetWIS()) ); }
	int32 GetActCHA() { return( std::min(GetMaxCHA(), GetCHA()) ); }
	void LoadAccountFlags();
	void ClearAccountFlag(const std::string& flag);
	void SetAccountFlag(const std::string& flag, const std::string& value);
	std::string GetAccountFlag(const std::string& flag);
	std::vector<std::string> GetAccountFlags();
	void SetGMStatus(int new_status);
	void Consume(const EQ::ItemData *item, uint8 type, int16 slot, bool auto_consume);
	void PlayMP3(const char* fname);
	void ExpeditionSay(const char *str, int ExpID);

	inline int32 GetEnvironmentDamageModifier() const { return environment_damage_modifier; }
	void SetEnvironmentDamageModifier(int32 val) { environment_damage_modifier = val; }
	inline bool GetInvulnerableEnvironmentDamage() const { return invulnerable_environment_damage; }
	void SetInvulnerableEnvironmentDamage(bool val) { invulnerable_environment_damage = val; }
	void SetIntoxication(int32 in_intoxication);

	void ApplyWeaponsStance();
	void TogglePassiveAlternativeAdvancement(const AA::Rank &rank, uint32 ability_id);
	bool UseTogglePassiveHotkey(const AA::Rank &rank);
	void TogglePurchaseAlternativeAdvancementRank(int rank_id);
	void ResetAlternateAdvancementRank(uint32 aa_id);
	bool IsEffectinAlternateAdvancementRankEffects(const AA::Rank &rank, int effect_id);

	void TripInterrogateInvState() { interrogateinv_flag = true; }
	bool GetInterrogateInvState() { return interrogateinv_flag; }

	bool InterrogateInventory(Client* requester, bool log, bool silent, bool allowtrip, bool& error, bool autolog = true);

	void SetNextInvSnapshot(uint32 interval_in_min) {
		m_epp.last_invsnapshot_time = time(nullptr);
		m_epp.next_invsnapshot_time = m_epp.last_invsnapshot_time + (interval_in_min * 60);
	}
	uint32 GetLastInvSnapshotTime() { return m_epp.last_invsnapshot_time; }
	uint32 GetNextInvSnapshotTime() { return m_epp.next_invsnapshot_time; }

	void QuestReward(Mob* target, uint32 copper = 0, uint32 silver = 0, uint32 gold = 0, uint32 platinum = 0, uint32 itemid = 0, uint32 exp = 0, bool faction = false);
	void QuestReward(Mob* target, const QuestReward_Struct &reward, bool faction = false);
	void CashReward(uint32 copper, uint32 silver, uint32 gold, uint32 platinum);
	void RewardFaction(int id, int amount);

	void ResetHPUpdateTimer() { hpupdate_timer.Start(); }

	void SendHPUpdateMarquee();

	void CheckRegionTypeChanges();

	 WaterRegionType GetLastRegion() { return last_region_type; }

	int32 CalcATK();

	uint32 trapid; //ID of trap player has triggered. This is cleared when the player leaves the trap's radius, or it despawns.

	void SendMerchantEnd();

	void CheckItemDiscoverability(uint32 item_id);

	Raid *p_raid_instance;

	inline uint32 GetPotionBeltItemIcon(uint8 slot_id)
	{
		return EQ::ValueWithin(
			slot_id,
			0,
			EQ::profile::POTION_BELT_SIZE - 1
		) ? m_pp.potionbelt.Items[slot_id].Icon : 0;
	};

	inline uint32 GetPotionBeltItemID(uint8 slot_id)
	{
		return EQ::ValueWithin(slot_id, 0, EQ::profile::POTION_BELT_SIZE - 1) ? m_pp.potionbelt.Items[slot_id].ID : 0;
	};

	inline std::string GetPotionBeltItemName(uint8 slot_id)
	{
		return EQ::ValueWithin(
			slot_id,
			0,
			EQ::profile::POTION_BELT_SIZE - 1
		) ? m_pp.potionbelt.Items[slot_id].Name : 0;
	};

	void ShowDevToolsMenu();
	CheatManager cheat_manager;

	// rate limit
	Timer m_list_task_timers_rate_limit = {};

	std::string GetGuildPublicNote();

	PlayerEvent::PlayerEvent GetPlayerEvent();
	void RecordKilledNPCEvent(NPC *n);

	uint32 GetEXPForLevel(uint16 check_level);

	// Evolving Item Info
	void ProcessEvolvingItem(const uint64 exp, const Mob* mob);
	void SendEvolvingPacket(int8 action, const CharacterEvolvingItemsRepository::CharacterEvolvingItems &item);
	void DoEvolveItemToggle(const EQApplicationPacket* app);
	void DoEvolveItemDisplayFinalResult(const EQApplicationPacket* app);
	bool DoEvolveCheckProgression(EQ::ItemInstance &inst);
	void SendEvolveXPWindowDetails(const EQApplicationPacket* app);
	void DoEvolveTransferXP(const EQApplicationPacket* app);
	void SendEvolveXPTransferWindow();
	void SendEvolveTransferResults(const EQ::ItemInstance &inst_from, const EQ::ItemInstance &inst_to, const EQ::ItemInstance &inst_from_new, const EQ::ItemInstance &inst_to_new, const uint32 compatibility, const uint32 max_transfer_level);

	// Account buckets
	std::string GetAccountBucket(std::string bucket_name);
	void SetAccountBucket(std::string bucket_name, std::string bucket_value, std::string expiration = "");
	void DeleteAccountBucket(std::string bucket_name);
	std::string GetAccountBucketExpires(std::string bucket_name);
	std::string GetAccountBucketRemaining(std::string bucket_name);

	std::string GetBandolierName(uint8 bandolier_slot);
	uint32 GetBandolierItemIcon(uint8 bandolier_slot, uint8 slot_id);
	uint32 GetBandolierItemID(uint8 bandolier_slot, uint8 slot_id);
	std::string GetBandolierItemName(uint8 bandolier_slot, uint8 slot_id);

	// External handin tracking
	// this is used to prevent things like quest::givecash and AddMoneyToPP
	// from double giving money back to players in scripts when return_items
	// also gives money back to players
	struct ExternalHandinMoneyReturned {
		uint64 copper;
		uint64 silver;
		uint64 gold;
		uint64 platinum;
		std::string return_source;
	};
private:
	ExternalHandinMoneyReturned m_external_handin_money_returned = {};
	std::vector<uint32_t>       m_external_handin_items_returned = {};
public:
	ExternalHandinMoneyReturned GetExternalHandinMoneyReturned() { return m_external_handin_money_returned; }
	std::vector<uint32_t> GetExternalHandinItemsReturned() { return m_external_handin_items_returned; }

	// used only for testing
	inline void SetCharacterId(uint32_t id) { character_id = id; }

protected:
	friend class Mob;
	void CalcEdibleBonuses(StatBonuses* newbon);
	void MakeBuffFadePacket(uint16 spell_id, int slot_id, bool send_message = true);
	bool client_data_loaded;


	void FinishAlternateAdvancementPurchase(AA::Rank *rank, bool ignore_cost, bool send_message_and_save);

	Mob* bind_sight_target;

	glm::vec4 m_AutoAttackPosition;
	glm::vec3 m_AutoAttackTargetLocation;
	Mob *aa_los_them_mob;
	bool los_status;
	bool los_status_facing;
	QGlobalCache *qGlobals;

	/** Adventure Variables **/
	Timer *adventure_request_timer;
	Timer *adventure_create_timer;
	Timer *adventure_leave_timer;
	Timer *adventure_door_timer;
	Timer *adventure_stats_timer;
	Timer *adventure_leaderboard_timer;
	int adv_requested_theme;
	int adv_requested_id;
	char *adv_requested_data;
	int adv_requested_member_count;
	char *adv_data;

private:

	eqFilterMode ClientFilters[_FilterCount];
	int32 HandlePacket(const EQApplicationPacket *app);
	void OPTGB(const EQApplicationPacket *app);
	void OPRezzAnswer(uint32 Action, uint32 SpellID, uint16 ZoneID, uint16 InstanceID, float x, float y, float z);
	void OPMemorizeSpell(const EQApplicationPacket *app);
	void OPMoveCoin(const EQApplicationPacket* app);
	void MoveItemCharges(EQ::ItemInstance &from, int16 to_slot, uint8 type);
	void OPGMTraining(const EQApplicationPacket *app);
	void OPGMEndTraining(const EQApplicationPacket *app);
	void OPGMTrainSkill(const EQApplicationPacket *app);
	void OPGMSummon(const EQApplicationPacket *app);
	void OPCombatAbility(const CombatAbility_Struct *ca_atk);

	// Bandolier Methods
	void CreateBandolier(const EQApplicationPacket *app);
	void RemoveBandolier(const EQApplicationPacket *app);
	void SetBandolier(const EQApplicationPacket *app);

	int32 CalcItemATKCap() final;
	int32 CalcHaste();

	int32 CalcAlcoholPhysicalEffect();
	int32 CalcSTR();
	int32 CalcSTA();
	int32 CalcDEX();
	int32 CalcAGI();
	int32 CalcINT();
	int32 CalcWIS();
	int32 CalcCHA();

	int32 CalcMR();
	int32 CalcFR();
	int32 CalcDR();
	int32 CalcPR();
	int32 CalcCR();
	int32 CalcCorrup();
	int64 CalcMaxHP();
	int64 CalcBaseHP();
	int64 CalcHPRegen(bool bCombat = false);
	int64 CalcManaRegen(bool bCombat = false);
	int64 CalcBaseManaRegen();
	void DoHPRegen();
	void DoManaRegen();
	void DoStaminaHungerUpdate();
	void CalcRestState();
	// if they have aggro (AggroCount != 0) their timer is saved in m_pp.RestTimer, else we need to get current timer
	inline uint32 GetRestTimer() const { return AggroCount ? m_pp.RestTimer : rest_timer.GetRemainingTime() / 1000; }
	void UpdateRestTimer(uint32 new_timer);

	uint8 playeraction;

	EQStreamInterface* eqs;

	uint32 ip;
	uint16 port;
	CLIENT_CONN_STATUS client_state;
	uint32 character_id;
	uint32 WID;
	uint32 account_id;
	char account_name[30];
	char loginserver[64];
	uint32 lsaccountid;
	char lskey[30];
	int16 admin;
	uint32 guild_id;
	uint8 guildrank; // player's rank in the guild, 1- Leader 8 Recruit
	bool guild_tribute_opt_in;
	bool guild_dirty{ true };	//used to control add/delete opcodes due to client bug in Ti thru RoF2
	bool GuildBanker;
	uint16 duel_target;
	bool duelaccepted;
	std::vector<uint32> keyring;
	bool tellsoff; // GM /toggle
	bool gm_hide_me;
	bool LFG;
	bool LFP;
	uint8 LFGFromLevel;
	uint8 LFGToLevel;
	bool LFGMatchFilter;
	char LFGComments[64];
	bool m_is_afk = false;
	bool m_is_manual_afk = false;
	bool auto_attack;
	bool auto_fire;
	bool runmode;
	uint8 gmspeed;
	bool gminvul;
	bool medding;
	uint16 horseId;
	bool revoked;
	uint16 pClientSideTarget;
	uint32 weight;
	bool berserk;
	bool dead;
	uint16 controlling_boat_id;
	uint16 controlled_mob_id;
	uint16 TrackingID;
	bool   trader;
	uint16 trader_id;
	uint16 customer_id;
	uint32 account_creation;
	bool first_login;
	bool ingame;
	uint32 mercid; // current merc
	uint8 mercSlot; // selected merc slot
	time_t                                                         m_trader_transaction_date;
	uint32                                                         m_trader_count{};
	uint32                                                         m_buyer_id;
	uint32                                                         m_barter_time;
	int32                                                          m_parcel_platinum;
	int32                                                          m_parcel_gold;
	int32                                                          m_parcel_silver;
	int32                                                          m_parcel_copper;
	int32                                                          m_parcel_count;
	bool                                                           m_parcel_enabled;
	bool                                                           m_parcel_merchant_engaged;
	std::map<uint32, CharacterParcelsRepository::CharacterParcels> m_parcels{};
	int Haste; //precalced value
	uint32 tmSitting; // time stamp started sitting, used for HP regen bonus added on MAY 5, 2004

	int32 environment_damage_modifier;
	bool invulnerable_environment_damage;

	// dev tools
	bool display_mob_info_window;
	bool dev_tools_enabled;

	uint16 m_door_tool_entity_id;
	uint16 m_object_tool_entity_id;


public:
	uint16 GetDoorToolEntityId() const;
	void SetDoorToolEntityId(uint16 door_tool_entity_id);
	uint16 GetObjectToolEntityId() const;
	void SetObjectToolEntityId(uint16 object_tool_entity_id);
private:

	int32 max_end;
	int32 current_endurance;

	// https://github.com/EQEmu/Server/pull/2479
	bool m_lock_save_position = false;
public:
	bool IsLockSavePosition() const;
	void SetLockSavePosition(bool lock_save_position);
private:

	PlayerProfile_Struct m_pp;
	ExtendedProfile_Struct m_epp;
	EQ::InventoryProfile m_inv;
	Object* m_tradeskill_object;
	PetInfo m_petinfo; // current pet data, used while loading from and saving to DB
	PetInfo m_suspendedminion; // pet data for our suspended minion.
	MercInfo m_mercinfo[MAXMERCS]; // current mercenary
	InspectMessage_Struct m_inspect_message;
	bool temp_pvp;

	void NPCSpawn(const Seperator* sep);

	void SendLogoutPackets();
	void SendZoneInPackets();
	bool AddPacket(const EQApplicationPacket *, bool);
	bool AddPacket(EQApplicationPacket**, bool);
	bool SendAllPackets();
	std::deque<std::unique_ptr<CLIENTPACKET>> clientpackets;

	//Zoning related stuff
	void SendZoneCancel(ZoneChange_Struct *zc);
	void SendZoneError(ZoneChange_Struct *zc, int8 err);
	void DoZoneSuccess(ZoneChange_Struct *zc, uint16 zone_id, uint32 instance_id, float dest_x, float dest_y, float dest_z, float dest_h, int8 ignore_r);
	void ZonePC(uint32 zoneID, uint32 instance_id, float x, float y, float z, float heading, uint8 ignorerestrictions, ZoneMode zm);
	void ProcessMovePC(uint32 zoneID, uint32 instance_id, float x, float y, float z, float heading, uint8 ignorerestrictions = 0, ZoneMode zm = ZoneSolicited);

	void SendTopLevelInventory();

	glm::vec4 m_ZoneSummonLocation;
	uint16 zonesummon_id;
	uint8 zonesummon_ignorerestrictions;
	ZoneMode zone_mode;

	WaterRegionType last_region_type;

	PTimerList p_timers; //persistent timers
	Timer hpupdate_timer;
	Timer camp_timer;
	Timer bot_camp_timer;
	Timer process_timer;
	Timer consume_food_timer;
	Timer zoneinpacket_timer;
	Timer linkdead_timer;
	Timer dead_timer;
	Timer global_channel_timer;
	Timer fishing_timer;
	Timer endupkeep_timer;
	Timer autosave_timer;
	Timer tribute_timer;

	Timer proximity_timer;
	Timer TaskPeriodic_Timer;
	Timer charm_update_timer;
	Timer rest_timer;
	Timer charm_class_attacks_timer;
	Timer charm_cast_timer;
	Timer qglobal_purge_timer;
	Timer TrackingTimer;
	Timer RespawnFromHoverTimer;
	Timer merc_timer;
	Timer anon_toggle_timer;
	Timer afk_toggle_timer;
	Timer helm_toggle_timer;
	Timer aggro_meter_timer;
	Timer consent_throttle_timer;
	Timer dynamiczone_removal_timer;
	Timer task_request_timer;
	Timer pick_lock_timer;
	Timer parcel_timer;	//Used to limit the number of parcels to one every 30 seconds (default).  Changable via rule.
	Timer lazy_load_bank_check_timer;
	Timer bandolier_throttle_timer;

	bool m_lazy_load_bank            = false;
	int  m_lazy_load_sent_bank_slots = 0;

	glm::vec3 m_Proximity;

	// client aggro
	Timer m_client_npc_aggro_scan_timer;
	void CheckClientToNpcAggroTimer();
	void ClientToNpcAggroProcess();
	void BroadcastPositionUpdate();

	// bulk position updates
	glm::vec4 m_last_position_before_bulk_update;
	Timer     m_client_bulk_npc_pos_update_timer;
	Timer     m_position_update_timer;
	void      CheckSendBulkNpcPositions(bool force = false);

	// afk
	bool                                  m_is_idle    = false;
	bool                                  m_afk_reset  = false; // used to trigger next-tic afk reset
	std::chrono::steady_clock::time_point m_last_moved = std::chrono::steady_clock::now();

	void BulkSendInventoryItems();

	faction_map factionvalues;

	uint32 tribute_master_id;

	bool npcflag;
	uint8 npclevel;
	bool bZoning;
	bool tgb;
	bool instalog;
	int32 last_reported_mana;
	int32 last_reported_endurance;

	int8 last_reported_mana_percent;
	int8 last_reported_endurance_percent;

	uint32 AggroCount; // How many mobs are aggro on us.

	bool ooc_regen;
	float AreaHPRegen;
	float AreaManaRegen;
	float AreaEndRegen;

	std::set<uint32> zone_flags;
	std::set<uint32> peqzone_flags;

	ClientTaskState *task_state;
	int TotalSecondsPlayed;

	// we use this very sparingly at the zone level
	// used for keeping clients in donecount sync before world sends absolute confirmations of state
	int64 m_shared_task_id = 0;
public:
	void SetSharedTaskId(int64 shared_task_id);
	int64 GetSharedTaskId() const;
	struct XTarget_Struct XTargets[XTARGET_HARDCAP];
private:

	bool m_exp_enabled;

	std::vector<EXPModifier> m_exp_modifiers;
	std::vector<PlayerTitlesetsRepository::PlayerTitlesets> m_player_title_sets;

	//Anti Spam Stuff
	Timer *KarmaUpdateTimer;
	uint32 TotalKarma;

	Timer *GlobalChatLimiterTimer; //60 seconds
	uint32 AttemptedMessages;

	EQ::versions::ClientVersion m_ClientVersion;
	uint32 m_ClientVersionBit;

	int XPRate;

	bool alternate_currency_loaded;
	std::map<uint32, uint32> alternate_currency;
	std::queue<std::pair<uint32, int32>> alternate_currency_queued_operations;

	//Connecting debug code.
	enum { //connecting states, used for debugging only
			NoPacketsReceived, //havent gotten anything
			//this is the point where the client changes to the loading screen
			ReceivedZoneEntry, //got the first packet, loading up PP
			PlayerProfileLoaded, //our DB work is done, sending it
			ZoneInfoSent, //includes PP, tributes, tasks, spawns, time and weather
			//this is the point where the client shows a status bar zoning in
			NewZoneRequested, //received and sent new zone request
			ClientSpawnRequested, //client sent ReqClientSpawn
			ZoneContentsSent, //objects, doors, zone points
			ClientReadyReceived, //client told us its ready, send them a bunch of crap like guild MOTD, etc
			//this is the point where the client releases the mouse
			ClientConnectFinished //client finally moved to finished state, were done here
	} conn_state;
	void ReportConnectingState();

	uint8 HideCorpseMode;
	bool PendingGuildInvitation;
	int PendingRezzXP;
	uint32 PendingRezzDBID;
	uint16 PendingRezzSpellID; // Only used for resurrect while hovering.
	std::string PendingRezzCorpseName; // Only used for resurrect while hovering.

	std::set<uint32> PlayerBlockedBuffs;
	std::set<uint32> PetBlockedBuffs;
	std::list<std::pair<std::string, uint16> > DraggedCorpses;

	uint8 MaxXTargets;
	bool XTargetAutoAddHaters;
	bool m_dirtyautohaters;

	XTargetAutoHaters m_autohatermgr;
	XTargetAutoHaters *m_activeautohatermgr;

	AggroMeter m_aggrometer;

	Timer ItemQuestTimer;
	std::map<std::string,std::string> accountflags;

	uint8 initial_respawn_selection;

	bool interrogateinv_flag; // used to minimize log spamming by players

	void InterrogateInventory_(bool errorcheck, Client* requester, int16 head, int16 index, const EQ::ItemInstance* inst, const EQ::ItemInstance* parent, bool log, bool silent, bool &error, int depth);
	bool InterrogateInventory_error(int16 head, int16 index, const EQ::ItemInstance* inst, const EQ::ItemInstance* parent, int depth);

	uint8 client_max_level;

	ExpeditionInvite m_dz_invite = {};
	std::vector<DzLockout> m_dz_lockouts;
	glm::vec3 m_quest_compass;
	bool m_has_quest_compass = false;
	std::vector<uint32_t> m_dynamic_zone_ids;

	std::vector<uint32_t> m_completed_shared_tasks;

public:
	enum BotOwnerOption : size_t {
		booDeathMarquee,
		booStatsUpdate,
		booSpawnMessageSay,
		booSpawnMessageTell,
		booSpawnMessageClassSpecific,
		booUnused,
		booAutoDefend,
		booBuffCounter,
		booMonkWuMessage,
		_booCount
	};

	bool GetBotOption(BotOwnerOption boo) const;
	void SetBotOption(BotOwnerOption boo, bool flag = true);

	bool GetBotPulling() { return m_bot_pulling; }
	void SetBotPulling(bool flag = true) { m_bot_pulling = flag; }
	uint32 GetAssistee() { return bot_assistee; }
	void SetAssistee(uint32 id = 0) { bot_assistee = id; }

	bool GetBotPrecombat() { return m_bot_precombat; }
	void SetBotPrecombat(bool flag = true) { m_bot_precombat = flag; }

	int GetBotRequiredLevel(uint8 class_id = Class::None);
	uint32 GetBotCreationLimit(uint8 class_id = Class::None);
	int GetBotSpawnLimit(uint8 class_id = Class::None);
	void SetBotCreationLimit(uint32 new_creation_limit, uint8 class_id = Class::None);
	void SetBotRequiredLevel(int new_required_level, uint8 class_id = Class::None);
	void SetBotSpawnLimit(int new_spawn_limit, uint8 class_id = Class::None);

	void CampAllBots(uint8 class_id = Class::None);
	void SpawnRaidBotsOnConnect(Raid* raid);

	void LoadDefaultBotSettings();
	int GetDefaultBotSettings(uint8 setting_type, uint16 bot_setting);
	int GetBotSetting(uint8 setting_type, uint16 bot_setting);
	void SetBotSetting(uint8 setting_type, uint16 bot_setting, uint32 setting_value);

	uint16 GetDefaultSpellTypeDelay(uint16 spell_type);
	uint8 GetDefaultSpellTypeMinThreshold(uint16 spell_type);
	uint8 GetDefaultSpellTypeMaxThreshold(uint16 spell_type);
	inline uint16 GetSpellTypeDelay(uint16 spell_type) const { return m_bot_spell_settings[spell_type].delay; }
	inline void SetSpellTypeDelay(uint16 spell_type, uint16 delay_value) { m_bot_spell_settings[spell_type].delay = delay_value; }
	inline uint8 GetSpellTypeMinThreshold(uint16 spell_type) const { return m_bot_spell_settings[spell_type].min_threshold; }
	inline void SetSpellTypeMinThreshold(uint16 spell_type, uint8 threshold_value) { m_bot_spell_settings[spell_type].min_threshold = threshold_value; }
	inline uint8 GetSpellTypeMaxThreshold(uint16 spell_type) const { return m_bot_spell_settings[spell_type].max_threshold; }
	inline void SetSpellTypeMaxThreshold(uint16 spell_type, uint8 threshold_value) { m_bot_spell_settings[spell_type].max_threshold = threshold_value; }
	inline bool SpellTypeRecastCheck(uint16 spellType) { return !m_bot_spell_settings[spellType].recast_timer.GetRemainingTime(); }
	void SetSpellTypeRecastTimer(uint16 spell_type, uint32 recast_time) { m_bot_spell_settings[spell_type].recast_timer.Start(recast_time); }

	void SetIllusionBlock(bool value) { _illusion_block = value; }
	bool GetIllusionBlock() const override { return _illusion_block; }

private:
	bool bot_owner_options[_booCount];
	bool m_bot_pulling;
	bool m_bot_precombat;
	uint32 bot_assistee;
	std::vector<BotSpellSettings> m_bot_spell_settings;
	bool _illusion_block;

	bool CanTradeFVNoDropItem();
	void SendMobPositions();
	void PlayerTradeEventLog(Trade *t, Trade *t2);

	// full and partial mail key cache
	std::string m_mail_key_full;
	std::string m_mail_key;
public:
	const std::string &GetMailKeyFull() const;
	const std::string &GetMailKey() const;
	void ShowZoneShardMenu();
	void Handle_OP_ChangePetName(const EQApplicationPacket *app);
	bool IsFilteredAFKPacket(const EQApplicationPacket *p);
	void CheckAutoIdleAFK(PlayerPositionUpdateClient_Struct *p);
	void SyncWorldPositionsToClient(bool ignore_idle = false);
};
