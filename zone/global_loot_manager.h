#pragma once

#include <vector>
#include <string>

class NPC;
class Client;

namespace GlobalLoot {

enum class RuleTypes {
	LevelMin = 0,
	LevelMax = 1,
	Race = 2,
	Class = 3,
	BodyType = 4,
	Rare = 5,
	Raid = 6,
	HotZone = 7,
	Max
};

struct Rule {
	RuleTypes type;
	int value;
	Rule(RuleTypes t, int v) : type(t), value(v) { }
};

};

class GlobalLootEntry {
	int m_id;
	int m_loottable_id;
	std::string m_description;
	std::vector<GlobalLoot::Rule> m_rules;
public:
	GlobalLootEntry(int id, int loottable, std::string des)
		: m_id(id), m_loottable_id(loottable), m_description(std::move(des))
	{ }
	bool PassesRules(NPC *mob) const;
	inline int GetLootTableID() const { return m_loottable_id; }
	inline int GetID() const { return m_id; }
	inline const std::string &GetDescription() const { return m_description; }
	inline void SetLootTableID(int in) { m_loottable_id = in; }
	inline void SetID(int in) { m_id = in; }
	inline void SetDescription(const std::string &in) { m_description = in; }
	inline void AddRule(GlobalLoot::RuleTypes rule, int value) { m_rules.emplace_back(rule, value); }
};

class GlobalLootManager {
	std::vector<GlobalLootEntry> m_entries;

public:
	std::vector<int> GetGlobalLootTables(NPC *mob) const;
	inline void Clear() { m_entries.clear(); }
	inline void AddEntry(GlobalLootEntry &in) { m_entries.push_back(in); }
	void ShowZoneGlobalLoot(Client *to) const;
	void ShowNPCGlobalLoot(Client *to, NPC *who) const;
};
