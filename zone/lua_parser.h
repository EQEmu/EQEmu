#pragma once

#ifdef LUA_EQEMU

#include "common/repositories/bug_reports_repository.h"
#include "zone/lua_mod.h"
#include "zone/quest_interface.h"
#include "zone/quest_parser_collection.h"
#include "zone/zone_config.h"

#include <exception>
#include <list>
#include <map>
#include <string>

extern const ZoneConfig *Config;

struct lua_State;
class Client;
class NPC;

namespace EQ
{
	class ItemInstance;
}

#include "lua_parser_events.h"

struct lua_registered_event;
namespace luabind {
	namespace adl {
		class object;
	}
}

class LuaParser : public QuestInterface {
public:
	~LuaParser();

	virtual int EventNPC(
		QuestEventID evt,
		NPC* npc,
		Mob *init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any> *extra_pointers
	);
	virtual int EventGlobalNPC(
		QuestEventID evt,
		NPC* npc,
		Mob *init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any> *extra_pointers
	);
	virtual int EventPlayer(
		QuestEventID evt,
		Client *client,
		std::string data,
		uint32 extra_data,
		std::vector<std::any> *extra_pointers
	);
	virtual int EventGlobalPlayer(
		QuestEventID evt,
		Client *client,
		std::string data,
		uint32 extra_data,
		std::vector<std::any> *extra_pointers
	);
	virtual int EventItem(
		QuestEventID evt,
		Client *client,
		EQ::ItemInstance *item,
		Mob *mob,
		std::string data,
		uint32 extra_data,
		std::vector<std::any> *extra_pointers
	);
	virtual int EventSpell(
		QuestEventID evt,
		Mob* mob,
		Client *client,
		uint32 spell_id,
		std::string data,
		uint32 extra_data,
		std::vector<std::any> *extra_pointers
	);
	virtual int EventEncounter(
		QuestEventID evt,
		std::string encounter_name,
		std::string data,
		uint32 extra_data,
		std::vector<std::any> *extra_pointers
	);
	virtual int EventBot(
		QuestEventID evt,
		Bot *bot,
		Mob *init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any> *extra_pointers
	);
	virtual int EventGlobalBot(
		QuestEventID evt,
		Bot *bot,
		Mob *init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any> *extra_pointers
	);
	virtual int EventMerc(
		QuestEventID evt,
		Merc* merc,
		Mob* init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	);
	virtual int EventGlobalMerc(
		QuestEventID evt,
		Merc* merc,
		Mob* init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	);
	virtual int EventZone(
		QuestEventID evt,
		Zone* zone,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	);
	virtual int EventGlobalZone(
		QuestEventID evt,
		Zone* zone,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	);

	virtual bool HasQuestSub(uint32 npc_id, QuestEventID evt);
	virtual bool HasGlobalQuestSub(QuestEventID evt);
	virtual bool PlayerHasQuestSub(QuestEventID evt);
	virtual bool GlobalPlayerHasQuestSub(QuestEventID evt);
	virtual bool SpellHasQuestSub(uint32 spell_id, QuestEventID evt);
	virtual bool ItemHasQuestSub(EQ::ItemInstance *itm, QuestEventID evt);
	virtual bool EncounterHasQuestSub(std::string encounter_name, QuestEventID evt);
	virtual bool HasEncounterSub(const std::string& package_name, QuestEventID evt);
	virtual bool BotHasQuestSub(QuestEventID evt);
	virtual bool GlobalBotHasQuestSub(QuestEventID evt);
	virtual bool MercHasQuestSub(QuestEventID evt);
	virtual bool GlobalMercHasQuestSub(QuestEventID evt);
	virtual bool ZoneHasQuestSub(QuestEventID evt);
	virtual bool GlobalZoneHasQuestSub(QuestEventID evt);

	virtual void LoadNPCScript(std::string filename, int npc_id);
	virtual void LoadGlobalNPCScript(std::string filename);
	virtual void LoadPlayerScript(std::string filename);
	virtual void LoadGlobalPlayerScript(std::string filename);
	virtual void LoadItemScript(std::string filename, EQ::ItemInstance *item);
	virtual void LoadSpellScript(std::string filename, uint32 spell_id);
	virtual void LoadEncounterScript(std::string filename, std::string encounter_name);
	virtual void LoadBotScript(std::string filename);
	virtual void LoadGlobalBotScript(std::string filename);
	virtual void LoadMercScript(std::string filename);
	virtual void LoadGlobalMercScript(std::string filename);
	virtual void LoadZoneScript(std::string filename);
	virtual void LoadGlobalZoneScript(std::string filename);

	virtual void AddVar(std::string name, std::string val);
	virtual std::string GetVar(std::string name);
	virtual void Init();
	virtual void ReloadQuests();
	virtual void RemoveEncounter(const std::string &name);
    virtual uint32 GetIdentifier() { return 0xb0712acc; }

	virtual int DispatchEventNPC(
		QuestEventID evt,
		NPC* npc,
		Mob *init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any> *extra_pointers
	);
	virtual int DispatchEventPlayer(
		QuestEventID evt,
		Client *client,
		std::string data,
		uint32 extra_data,
		std::vector<std::any> *extra_pointers
	);
	virtual int DispatchEventItem(
		QuestEventID evt,
		Client *client,
		EQ::ItemInstance *item,
		Mob *mob,
		std::string data,
		uint32 extra_data,
		std::vector<std::any> *extra_pointers
	);
	virtual int DispatchEventSpell(
		QuestEventID evt,
		Mob* mob,
		Client *client,
		uint32 spell_id,
		std::string data,
		uint32 extra_data,
		std::vector<std::any> *extra_pointers
	);
	virtual int DispatchEventBot(
		QuestEventID evt,
		Bot *bot,
		Mob *init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any> *extra_pointers
	);
	virtual int DispatchEventMerc(
		QuestEventID evt,
		Merc* merc,
		Mob* init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	);
	virtual int DispatchEventZone(
		QuestEventID evt,
		Zone* zone,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers
	);

	static LuaParser* Instance() {
		static LuaParser inst;
		return &inst;
	}

	bool HasFunction(std::string function, std::string package_name);

	//Mod Extensions
	void MeleeMitigation(Mob *self, Mob *attacker, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault);
	void ApplyDamageTable(Mob *self, DamageHitInfo &hit, bool &ignoreDefault);
	bool AvoidDamage(Mob *self, Mob *other, DamageHitInfo &hit, bool &ignoreDefault);
	bool CheckHitChance(Mob *self, Mob* other, DamageHitInfo &hit, bool &ignoreDefault);
	void TryCriticalHit(Mob *self, Mob *defender, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault);
	void CommonOutgoingHitSuccess(Mob *self, Mob* other, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault);
	uint32 GetRequiredAAExperience(Client *self, bool &ignoreDefault);
	uint32 GetEXPForLevel(Client *self, uint16 level, bool &ignoreDefault);
	uint64 GetExperienceForKill(Client *self, Mob *against, bool &ignoreDefault);
	int64 CalcSpellEffectValue_formula(Mob *self, uint32 formula, int64 base_value, int64 max_value, int caster_level, uint16 spell_id, int ticsremaining, bool &ignoreDefault);
	int32 UpdatePersonalFaction(Mob *self, int32 npc_value, int32 faction_id, int32 current_value, int32 temp, int32 this_faction_min, int32 this_faction_max, bool &ignore_default);
	void RegisterBug(Client *self, BaseBugReportsRepository::BugReports bug, bool &ignore_default);
	int64 CommonDamage(Mob *self, Mob* attacker, int64 value, uint16 spell_id, int skill_used, bool avoidable, int8 buff_slot, bool buff_tic, int special, bool &ignore_default);
	uint64 HealDamage(Mob *self, Mob* caster, uint64 value, uint16 spell_id, bool &ignore_default);
	uint64 SetEXP(Mob *self, ExpSource exp_source, uint64 current_exp, uint64 set_exp, bool is_rezz_exp, bool &ignore_default);
	uint64 SetAAEXP(Mob *self, ExpSource exp_source, uint64 current_aa_exp, uint64 set_aa_exp, bool is_rezz_exp, bool &ignore_default);
	bool IsImmuneToSpell(Mob *self, Mob* caster, uint16 spell_id, bool &ignore_default);
private:
	LuaParser();
	LuaParser(const LuaParser&);
	LuaParser& operator=(const LuaParser&);

	int _EventNPC(
		std::string package_name,
		QuestEventID evt,
		NPC* npc,
		Mob *init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any> *extra_pointers,
		luabind::adl::object *l_func = nullptr
	);
	int _EventPlayer(
		std::string package_name,
		QuestEventID evt,
		Client *client,
		std::string data,
		uint32 extra_data,
		std::vector<std::any> *extra_pointers,
		luabind::adl::object *l_func = nullptr
	);
	int _EventItem(
		std::string package_name,
		QuestEventID evt,
		Client *client,
		EQ::ItemInstance *item,
		Mob *mob,
		std::string data,
		uint32 extra_data,
		std::vector<std::any> *extra_pointers,
		luabind::adl::object *l_func = nullptr
	);
	int _EventSpell(
		std::string package_name,
		QuestEventID evt,
		Mob* mob,
		Client *client,
		uint32 spell_id,
		std::string data,
		uint32 extra_data,
		std::vector<std::any> *extra_pointers,
		luabind::adl::object *l_func = nullptr
	);
	int _EventEncounter(
		std::string package_name,
		QuestEventID evt,
		std::string encounter_name,
		std::string data,
		uint32 extra_data,
		std::vector<std::any> *extra_pointers
	);
	int _EventBot(
		std::string package_name,
		QuestEventID evt,
		Bot *bot,
		Mob *init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any> *extra_pointers,
		luabind::adl::object *l_func = nullptr
	);
	int _EventMerc(
		std::string package_name,
		QuestEventID evt,
		Merc* merc,
		Mob* init,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers,
		luabind::adl::object* l_func = nullptr
	);
	int _EventZone(
		std::string package_name,
		QuestEventID evt,
		Zone* zone,
		std::string data,
		uint32 extra_data,
		std::vector<std::any>* extra_pointers,
		luabind::adl::object* l_func = nullptr
	);

	void LoadScript(std::string filename, std::string package_name);
	void MapFunctions(lua_State *L);
	QuestEventID ConvertLuaEvent(QuestEventID evt);

	std::map<std::string, std::string> vars_;
	std::map<std::string, bool> loaded_;
	std::vector<LuaMod> mods_;
	lua_State *L;

	NPCArgumentHandler       NPCArgumentDispatch[_LargestEventID];
	PlayerArgumentHandler    PlayerArgumentDispatch[_LargestEventID];
	ItemArgumentHandler      ItemArgumentDispatch[_LargestEventID];
	SpellArgumentHandler     SpellArgumentDispatch[_LargestEventID];
	EncounterArgumentHandler EncounterArgumentDispatch[_LargestEventID];
	BotArgumentHandler       BotArgumentDispatch[_LargestEventID];
	ZoneArgumentHandler      ZoneArgumentDispatch[_LargestEventID];
};

#endif // LUA_EQEMU
