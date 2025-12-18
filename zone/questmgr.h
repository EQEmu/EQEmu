/*  EQEMu:  Everquest Server Emulator
    Copyright (C) 2001-2004  EQEMu Development Team (http://eqemulator.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#pragma once

#include "common/timer.h"
#include "zone/tasks.h"

#include <list>
#include <stack>

class Client;
class Mob;
class NPC;

namespace EQ
{
	class ItemInstance;
}

struct RunningQuest {
	Mob* owner = nullptr;
	Client* initiator = nullptr;
	EQ::ItemInstance* questitem = nullptr;
	const SPDat_Spell_Struct* questspell = nullptr;
	bool depop_npc = false;
	std::string encounter = "";
	Zone* zone = nullptr;
};

class QuestManager {

	struct PausedTimer {
		Mob*        owner;
		std::string name;
		uint32      time;
	};
public:

	QuestManager();
	virtual ~QuestManager();

	void StartQuest(const RunningQuest& q);
	void EndQuest();
	bool QuestsRunning() { return !m_running_quests.empty(); }

	void Process();

	void ClearTimers(Mob *who);
	void ClearAllTimers();

	//quest functions
	void echo(int colour, const char *str);
	void say(const char *str, Journal::Options &opts);
	void me(const char *str);
	void summonitem(uint32 itemid, int16 charges = -1);
	EQ::ItemInstance* CreateItem(uint32 item_id, int16 charges = 0, uint32 augment_one = 0, uint32 augment_two = 0, uint32 augment_three = 0, uint32 augment_four = 0, uint32 augment_five = 0, uint32 augment_six = 0, bool attuned = false) const;
	void write(const char *file, const char *str);
	Mob* spawn2(int npc_type, int grid, int unused, const glm::vec4& position);
	Mob* unique_spawn(int npc_type, int grid, int unused, const glm::vec4& position);
	Mob* spawn_from_spawn2(uint32 spawn2_id);
	void enable_spawn2(uint32 spawn2_id);
	void disable_spawn2(uint32 spawn2_id);
	void setstat(int stat, int value);
	void incstat(int stat, int value);
	void castspell(uint16 spell_id, uint16 target_id);
	void selfcast(uint16 spell_id);
	void addloot(int item_id, int charges = 0, bool equipitem = true, int aug1 = 0, int aug2 = 0, int aug3 = 0, int aug4 = 0, int aug5 = 0, int aug6 = 0);
	void Zone(const char *zone_name);
	void ZoneGroup(const char *zone_name);
	void ZoneRaid(const char *zone_name);
	void settimer(const std::string& timer_name, uint32 seconds, Mob* m = nullptr);
	void settimerMS(const std::string& timer_name, uint32 milliseconds);
	void settimerMS(const std::string& timer_name, uint32 milliseconds, EQ::ItemInstance* inst);
	void settimerMS(const std::string& timer_name, uint32 milliseconds, Mob* m);
	void stoptimer(const std::string& timer_name);
	void stoptimer(const std::string& timer_name, EQ::ItemInstance* inst);
	void stoptimer(const std::string& timer_name, Mob* m);
	void stopalltimers();
	void stopalltimers(EQ::ItemInstance* inst);
	void stopalltimers(Mob* m);
	void pausetimer(const std::string& timer_name, Mob* m = nullptr);
	void resumetimer(const std::string& timer_name, Mob* m = nullptr);
	bool ispausedtimer(const std::string& timer_name, Mob* m = nullptr);
	bool hastimer(const std::string& timer_name, Mob* m = nullptr);
	uint32 getremainingtimeMS(const std::string& timer_name, Mob* m = nullptr);
	uint32 gettimerdurationMS(const std::string& timer_name, Mob* m = nullptr);
	void emote(const char *str);
	void shout(const char *str);
	void shout2(const char *str);
	void gmsay(const char *str, uint32 color, bool send_to_world, uint32 to_guilddbid, uint32 to_minstatus);
	void depop(int npc_type = 0); // depop NPC and don't start spawn timer
	void depop_withtimer(int npc_type = 0); // depop NPC and start spawn timer
	void depopall(int npc_type = 0);
	void depopzone(bool StartSpawnTimer = true);
	void repopzone(bool is_forced = false);
	void processmobswhilezoneempty(bool quest_override_on);
	void settarget(const char *type, int target_id);
	void follow(int entity_id, int distance);
	void sfollow();
	void changedeity(uint32 deity_id);
	void exp(int amt);
	void level(int newlevel);
	void traindisc(uint32 discipline_tome_item_id);
	bool isdisctome(uint32 item_id);
	std::string getracename(uint16 race_id);
	std::string getspellname(uint32 spell_id);
	std::string getskillname(int skill_id);
	std::string getldonthemename(uint32 theme_id);
	std::string getfactionname(int faction_id);
	std::string getlanguagename(uint8 language_id);
	std::string getbodytypename(uint8 body_type_id);
	std::string getconsiderlevelname(uint8 consider_level);
	void safemove();
	void rain(int weather);
	void snow(int weather);
	void rename(std::string name);
	void surname(std::string last_name);
	void permaclass(int class_id);
	void permarace(int race_id);
	void permagender(int gender_id);
	uint16 scribespells(uint8 max_level, uint8 min_level = 1);
	uint16 traindiscs(uint8 max_level, uint8 min_level = 1);
	void unscribespells();
	void untraindiscs();
	void givecash(uint32 copper, uint32 silver = 0, uint32 gold = 0, uint32 platinum = 0);
	void pvp(const char *mode);
	void movepc(int zone_id, float x, float y, float z, float heading);
	void gmmove(float x, float y, float z);
	void movegrp(int zoneid, float x, float y, float z);
	void doanim(int animation_id, int animation_speed = 0, bool ackreq = true, eqFilterType filter = FilterNone);
	void addskill(int skill_id, int value);
	void setlanguage(uint8 language_id, uint8 language_skill);
	void setskill(int skill_id, int value);
	void setallskill(int value);
	void attack(const char *client_name);
	void attacknpc(int npc_entity_id);
	void attacknpctype(int npc_type_id);
	void save();
	void faction(int faction_id, int faction_value, int temp);
	void rewardfaction(int faction_id, int faction_value);
	void setsky(uint8 new_sky);
	void SetGuild(uint32 new_guild_id, uint8 new_rank);
	void CreateGuild(const char *guild_name, const char *leader);
	void settime(uint8 new_hour, uint8 new_min, bool update_world = true);
	void itemlink(int item_id);
	void signal(int npc_id, int wait_ms = 0);
	void signalwith(int npc_id, int signal_id, int wait_ms = 0);
	void setglobal(const char *varname, const char *newvalue, int options, const char *duration);
	void targlobal(const char *varname, const char *value, const char *duration, int npcid, int charid, int zoneid);
	void delglobal(const char *varname);
	void ding();
	void rebind(int zone_id, const glm::vec3& location);
	void rebind(int zone_id, const glm::vec4& location);
	void start(int wp);
	void stop();
	void pause(int duration);
	void moveto(const glm::vec4& position, bool saveguardspot);
	void resume();
	void addldonpoints(uint32 theme_id, int points);
	void addldonloss(uint32 theme_id);
	void addldonwin(uint32 theme_id);
	void removeldonloss(uint32 theme_id);
	void removeldonwin(uint32 theme_id);
	void setnexthpevent(int at);
	void setnextinchpevent(int at);
	void respawn(int npc_type, int grid);
	void set_proximity(float min_x, float max_x, float min_y, float max_y, float min_z = -999999.0f, float max_z = 999999.0f, bool enable_say = false);
	void set_proximity_range(float x_range, float y_range, float z_range = 999999.0f, bool enable_say = false);
	void clear_proximity();
	void enable_proximity_say();
	void disable_proximity_say();
	void setanim(int npc_type, int animnum);
	void showgrid(int grid_id);
	void spawn_condition(const char *zone_short, uint32 instance_id, uint16 condition_id, short new_value);
	short get_spawn_condition(const char *zone_short, uint32 instance_id, uint16 condition_id);
	void toggle_spawn_event(int event_id, bool enable, bool strict, bool reset_base);
	bool has_zone_flag(int zone_id);
	void set_zone_flag(int zone_id);
	void clear_zone_flag(int zone_id);
	void sethp(int64 hpperc);
	bool summonburiedplayercorpse(uint32 char_id, const glm::vec4& position);
	bool summonallplayercorpses(uint32 char_id, const glm::vec4& position);
	int64 getplayerburiedcorpsecount(uint32 char_id);
	int64 getplayercorpsecount(uint32 character_id);
	int64 getplayercorpsecountbyzoneid(uint32 character_id, uint32 zone_id);
	bool buryplayercorpse(uint32 char_id);
	void forcedooropen(uint32 doorid, bool altmode);
	void forcedoorclose(uint32 doorid, bool altmode);
	void toggledoorstate(uint32 doorid);
	bool isdooropen(uint32 doorid);
	void npcrace(uint16 race_id);
	void npcgender(uint8 gender_id);
	void npcsize(float size);
	void npctexture(uint8 texture);
	void playerrace(uint16 race_id);
	void playergender(uint8 gender_id);
	void playersize(float size);
	void playertexture(uint8 texture);
	void playerfeature(const char* feature, int setting);
	void npcfeature(const char* feature, int setting);
	void popup(const char *title, const char *text, uint32 popupid, uint32 buttons, uint32 Duration);
	void taskselector(const std::vector<int>& tasks, bool ignore_cooldown = false);
	void tasksetselector(int tasksettid, bool ignore_cooldown = false);
	void enabletask(int taskcount, int *tasks);
	void disabletask(int taskcount, int *tasks);
	bool istaskenabled(int taskid);
	bool istaskactive(int task);
	bool istaskactivityactive(int task, int activity);
	int gettaskactivitydonecount(int task, int activity);
	void updatetaskactivity(int task, int activity, int count, bool ignore_quest_update = false);
	void resettaskactivity(int task, int activity);
	void assigntask(int taskid, bool enforce_level_requirement = false);
	void failtask(int taskid);
	bool completetask(int task_id);
	bool uncompletetask(int task_id);
	int tasktimeleft(int taskid);
	bool istaskcompleted(int task_id);
	bool aretaskscompleted(const std::vector<int>& task_ids);
	int enabledtaskcount(int taskset);
	int firsttaskinset(int taskset);
	int lasttaskinset(int taskset);
	int nexttaskinset(int taskset, int taskid);
	int activespeaktask();
	int activespeakactivity(int taskid);
	int activetasksinset(int taskset);
	int completedtasksinset(int taskset);
	bool istaskappropriate(int task);
	std::string gettaskname(uint32 task_id);
	int GetCurrentDzTaskID();
	void EndCurrentDzTask(bool send_fail = false);
	void clearspawntimers();
	void ze(int type, const char *str);
	void we(int type, const char *str);
	void marquee(uint32 type, std::string message, uint32 duration = 3000);
	void marquee(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, std::string message);
	void message(uint32 type, const char *message);
	void whisper(const char *message);
	int getlevel(uint8 type);
	int collectitems(uint32 item_id, bool remove);
	int collectitems_processSlot(int16 slot_id, uint32 item_id, bool remove);
	uint32 countitem(uint32 item_id);
	void removeitem(uint32 item_id, uint32 quantity = 1);
	std::string getitemcomment(uint32 item_id);
	std::string getitemlore(uint32 item_id);
	std::string getitemname(uint32 item_id);
	void enabletitle(int titleset);
	bool checktitle(int titlecheck);
	void removetitle(int titlecheck);
	uint16 CreateGroundObject(uint32 itemid, const glm::vec4& position, uint32 decay_time = 300000);
	uint16 CreateGroundObjectFromModel(const char* model, const glm::vec4& position, uint8 type = 0x00, uint32 decay_time = 0);
	void ModifyNPCStat(std::string stat, std::string value);
	void UpdateSpawnTimer(uint32 id, uint32 newTime);
	void MerchantSetItem(uint32 NPCid, uint32 itemid, uint32 quantity = 0);
	uint32 MerchantCountItem(uint32 NPCid, uint32 itemid);
	uint16 CreateInstance(const std::string& zone_short_name, int16 instance_version, uint32 duration);
	void UpdateInstanceTimer(uint16 instance_id, uint32 new_duration);
	void UpdateZoneHeader(std::string type, std::string value);
	uint32 GetInstanceTimer();
	uint32 GetInstanceTimerByID(uint16 instance_id = 0);
	void DestroyInstance(uint16 instance_id);
	uint16 GetInstanceID(const char *zone, int16 version);
	std::vector<uint16> GetInstanceIDs(std::string zone_name, uint32 character_id = 0);
	uint16 GetInstanceIDByCharID(const std::string& zone_short_name, int16 instance_version, uint32 character_id);
	void AssignToInstance(uint16 instance_id);
	void AssignToInstanceByCharID(uint16 instance_id, uint32 char_id);
	void AssignGroupToInstance(uint16 instance_id);
	void AssignRaidToInstance(uint16 instance_id);
	void RemoveFromInstance(uint16 instance_id);
	void RemoveFromInstanceByCharID(uint16 instance_id, uint32 char_id);
	bool CheckInstanceByCharID(uint16 instance_id, uint32 char_id);
	//void RemoveGroupFromInstance(uint16 instance_id);	//potentially useful but not implmented at this time.
	//void RemoveRaidFromInstance(uint16 instance_id);	//potentially useful but not implmented at this time.
	void RemoveAllFromInstance(uint16 instance_id);
	void MovePCInstance(int zone_id, int instance_id, const glm::vec4& position);
	void FlagInstanceByGroupLeader(uint32 zone, int16 version);
	void FlagInstanceByRaidLeader(uint32 zone, int16 version);
	std::string varlink(EQ::ItemInstance* inst);
	std::string varlink(uint32 item_id, int16 charges = 0, uint32 aug1 = 0, uint32 aug2 = 0, uint32 aug3 = 0, uint32 aug4 = 0, uint32 aug5 = 0, uint32 aug6 = 0, bool attuned = false);
	std::string getcharnamebyid(uint32 char_id);
	uint32 getcharidbyname(const char* name);
	std::string getclassname(uint8 class_id, uint8 level = 0);
	uint32 getcurrencyid(uint32 item_id);
	uint32 getcurrencyitemid(uint32 currency_id);
	const char* getguildnamebyid(int guild_id);
	int getguildidbycharid(uint32 char_id);
	int getgroupidbycharid(uint32 char_id);
	std::string getnpcnamebyid(uint32 npc_id);
	std::string getcleannpcnamebyid(uint32 npc_id);
	int getraididbycharid(uint32 char_id);
	void SetRunning(bool val);
	bool IsRunning();
	void FlyMode(GravityBehavior flymode);
	uint8 FactionValue();
	void wearchange(uint8 slot, uint32 texture, uint32 hero_forge_model = 0, uint32 elite_material = 0);
	void voicetell(const char *str, int macronum, int racenum, int gendernum);
	void LearnRecipe(uint32 recipe_id);
	void SendMail(const char *to, const char *from, const char *subject, const char *message);
	uint16 CreateDoor( const char* model, float x, float y, float z, float heading, uint8 opentype, uint16 size);
	int32 GetZoneID(const char *zone);
	static std::string GetZoneLongName(std::string zone_short_name);
	static std::string GetZoneLongNameByID(uint32 zone_id);
	static std::string GetZoneShortName(uint32 zone_id);
	void CrossZoneDialogueWindow(uint8 update_type, int update_identifier, const char* message, const char* client_name = "");
	void CrossZoneLDoNUpdate(uint8 update_type, uint8 update_subtype, int update_identifier, uint32 theme_id, int points = 1, const char* client_name = "");
	void CrossZoneMarquee(uint8 update_type, int update_identifier, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message, const char* client_name = "");
	void CrossZoneMessage(uint8 update_type, int update_identifier, uint32 type, const char* message, const char* client_name = "");
	void CrossZoneMove(const CZMove_Struct& m);
	void CrossZoneSetEntityVariable(uint8 update_type, int update_identifier, const char* variable_name, const char* variable_value, const char* client_name = "");
	void CrossZoneSignal(uint8 update_type, int update_identifier, int signal_id, const char* client_name = "");
	void CrossZoneSpell(uint8 update_type, uint8 update_subtype, int update_identifier, uint32 spell_id, const char* client_name = "");
	void CrossZoneTaskUpdate(uint8 update_type, uint8 update_subtype, int update_identifier, uint32 task_identifier, int task_subidentifier = -1, int update_count = 1, bool enforce_level_requirement = false, const char* client_name = "");
	void WorldWideDialogueWindow(const char* message, uint8 min_status = AccountStatus::Player, uint8 max_status = AccountStatus::Player);
	void WorldWideLDoNUpdate(uint8 update_type, uint32 theme_id, int points = 1, uint8 min_status = AccountStatus::Player, uint8 max_status = AccountStatus::Player);
	void WorldWideMarquee(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message, uint8 min_status = AccountStatus::Player, uint8 max_status = AccountStatus::Player);
	void WorldWideMessage(uint32 type, const char* message, uint8 min_status = AccountStatus::Player, uint8 max_status = AccountStatus::Player);
	void WorldWideMove(uint8 update_type, const char* zone_short_name, uint16 instance_id = 0, uint8 min_status = AccountStatus::Player, uint8 max_status = AccountStatus::Player);
	void WorldWideSetEntityVariable(uint8 update_type, const char* variable_name, const char* variable_value, uint8 min_status = AccountStatus::Player, uint8 max_status = AccountStatus::Player);
	void WorldWideSignal(uint8 update_type, int signal_id, uint8 min_status = AccountStatus::Player, uint8 max_status = AccountStatus::Player);
	void WorldWideSpell(uint8 update_type, uint32 spell_id, uint8 min_status = AccountStatus::Player, uint8 max_status = AccountStatus::Player);
	void WorldWideTaskUpdate(uint8 update_type, uint32 task_identifier, int task_subidentifier = -1, int update_count = 1, bool enforce_level_requirement = false, uint8 min_status = AccountStatus::Player, uint8 max_status = AccountStatus::Player);
	bool EnableRecipe(uint32 recipe_id);
	bool DisableRecipe(uint32 recipe_id);
	void ClearNPCTypeCache(int npctype_id);
	void ReloadZoneStaticData();
	std::string gethexcolorcode(std::string color_name);
	float GetAAEXPModifierByCharID(uint32 character_id, uint32 zone_id, int16 instance_version = -1) const;
	float GetEXPModifierByCharID(uint32 character_id, uint32 zone_id, int16 instance_version = -1) const;
	void SetAAEXPModifierByCharID(uint32 character_id, uint32 zone_id, float aa_modifier, int16 instance_version = -1);
	void SetEXPModifierByCharID(uint32 character_id, uint32 zone_id, float exp_modifier, int16 instance_version = -1);
	std::string getgendername(uint32 gender_id);
	std::string getdeityname(uint32 deity_id);
	std::string getinventoryslotname(int16 slot_id);
	const int getitemstat(uint32 item_id, std::string stat_identifier);
	int getspellstat(uint32 spell_id, std::string stat_identifier, uint8 slot = 0);
	const SPDat_Spell_Struct *getspell(uint32 spell_id);
	std::string getenvironmentaldamagename(uint8 damage_type);
	void TrackNPC(uint32 entity_id);
	int GetRecipeMadeCount(uint32 recipe_id);
	std::string GetRecipeName(uint32 recipe_id);
	bool HasRecipeLearned(uint32 recipe_id);
	bool DoAugmentSlotsMatch(uint32 item_one, uint32 item_two);
	int8 DoesAugmentFit(EQ::ItemInstance* inst, uint32 augment_id, uint8 augment_slot = 255);
	void SendPlayerHandinEvent();
	void SendChannelMessage(uint8 channel_number, uint32 guild_id, uint8 language_id, uint8 language_skill, const char* message);
	void SendChannelMessage(Client* from, uint8 channel_number, uint32 guild_id, uint8 language_id, uint8 language_skill, const char* message);
	void SendChannelMessage(Client* from, const char* to, uint8 channel_number, uint32 guild_id, uint8 language_id, uint8 language_skill, const char* message);
	std::string GetAutoLoginCharacterNameByAccountID(uint32 account_id);
	bool SetAutoLoginCharacterNameByAccountID(uint32 account_id, const std::string& character_name);
	void SpawnCircle(uint32 npc_id, glm::vec4 position, float radius, uint32 points);
	void SpawnGrid(uint32 npc_id, glm::vec4 position, float spacing, uint32 spawn_count);
	std::vector<std::string> GetPausedTimers(Mob* m);
	std::vector<std::string> GetTimers(Mob* m);

	Bot *GetBot() const;
	Client *GetInitiator() const;
	Merc* GetMerc() const;
	NPC *GetNPC() const;
	Mob *GetOwner() const;
	EQ::InventoryProfile* GetInventory() const;
	EQ::ItemInstance *GetQuestItem() const;
	const SPDat_Spell_Struct *GetQuestSpell();
	std::string GetEncounter() const;
	inline bool ProximitySayInUse() { return HaveProximitySays; }

	int createbotcount(uint8 class_id = Class::None);
	int spawnbotcount(uint8 class_id = Class::None);
	bool botquest();
	bool createBot(const char *name, const char *lastname, uint8 level, uint16 race, uint8 botclass, uint8 gender);

	bool handin(std::map<std::string, uint32> required);

private:
	std::stack<RunningQuest> m_running_quests;

	bool HaveProximitySays;

	int QGVarDuration(const char *fmt);
	int InsertQuestGlobal(int charid, int npcid, int zoneid, const char *name, const char *value, int expdate);

	class QuestTimer {
	public:
		inline QuestTimer(int duration, Mob *_mob, std::string _name)
			: mob(_mob), name(_name), Timer_(duration) { Timer_.Start(duration, false); }
		Mob*   mob;
		std::string name;
		Timer Timer_;
	};
	class SignalTimer {
	public:
		inline SignalTimer(int duration, int _npc_id, int _signal_id) : npc_id(_npc_id), signal_id(_signal_id), Timer_(duration) { Timer_.Start(duration, false); }
		int npc_id;
		int signal_id;
		Timer Timer_;
	};
	std::list<QuestTimer>	QTimerList;
	std::list<SignalTimer>	STimerList;
	std::list<PausedTimer>	PTimerList;
};

extern QuestManager quest_manager;
