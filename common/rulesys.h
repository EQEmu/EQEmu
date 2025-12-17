/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

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

#include "common/types.h"

#include <map>
#include <string>
#include <vector>

/*
* Access to the rules system in normal code is done with three calls:
* - RuleI(category, rule) -> fetch an integer rule's value
* - RuleR(category, rule) -> fetch a real (float) rule's value
* - RuleB(category, rule) -> fetch a boolean/flag rule's value
* - RuleS(category, rule) -> fetch a string rule's value
*
*/

//note, these macros assume there is always a RuleManager *rules in scope,
//which makes it a global for now, but with instancing we will do exactly
//what we do with the zone global and just make it a member of core classes
#define RuleI(category_name, rule_name) \
    RuleManager::Instance()->GetIntRule( RuleManager::Int__##rule_name )
#define RuleR(category_name, rule_name) \
    RuleManager::Instance()->GetRealRule( RuleManager::Real__##rule_name )
#define RuleB(category_name, rule_name) \
    RuleManager::Instance()->GetBoolRule( RuleManager::Bool__##rule_name )
#define RuleS(category_name, rule_name) \
    RuleManager::Instance()->GetStringRule( RuleManager::String__##rule_name )


class Database;

class RuleManager {
public:
	//generate our rule enums:
	typedef enum {
#define RULE_INT(category_name, rule_name, default_value, notes) \
        Int__##rule_name,

#include "ruletypes.h"

		_IntRuleCount
	} IntType;

	static const int IntRuleCount = static_cast<int>(_IntRuleCount);

	typedef enum {
#define RULE_REAL(category_name, rule_name, default_value, notes) \
        Real__##rule_name,

#include "ruletypes.h"

		_RealRuleCount
	} RealType;

	static const int RealRuleCount = static_cast<int>(_RealRuleCount);

	typedef enum {
#define RULE_BOOL(category_name, rule_name, default_value, notes) \
        Bool__##rule_name,

#include "ruletypes.h"

		_BoolRuleCount
	} BoolType;

	static const int BoolRuleCount = static_cast<int>(_BoolRuleCount);

	typedef enum {
#define RULE_STRING(category_name, rule_name, default_value, notes) \
        String__##rule_name,

#include "ruletypes.h"

		_StringRuleCount
	} StringType;

	static const int StringRuleCount = static_cast<int>(_StringRuleCount);

	typedef enum {
#define RULE_CATEGORY(category_name) \
        Category__##category_name,

#include "ruletypes.h"

		_CatCount
	} CategoryType;

	static RuleManager *Instance()
	{
		static RuleManager rules;
		return &rules;
	}

	static const IntType      InvalidInt      = _IntRuleCount;
	static const RealType     InvalidReal     = _RealRuleCount;
	static const BoolType     InvalidBool     = _BoolRuleCount;
	static const StringType   InvalidString   = _StringRuleCount;
	static const CategoryType InvalidCategory = _CatCount;

	static const uint32 RulesCount = IntRuleCount + RealRuleCount + BoolRuleCount + StringRuleCount;

	//fetch routines, you should generally use the Rule* macros instead of this
	int GetIntRule(IntType t) const;
	float GetRealRule(RealType t) const;
	bool GetBoolRule(BoolType t) const;
	std::string GetStringRule(StringType t) const;

	//management routines
	static std::string GetRuleName(IntType t) { return s_RuleInfo[static_cast<int>(t)].name; }
	static std::string GetRuleName(RealType t) { return s_RuleInfo[static_cast<int>(t) + IntRuleCount].name; }
	static std::string GetRuleName(BoolType t) { return s_RuleInfo[static_cast<int>(t) + IntRuleCount + RealRuleCount].name; }
	static std::string GetRuleName(StringType t) { return s_RuleInfo[static_cast<int>(t) + IntRuleCount + RealRuleCount + StringRuleCount].name; }
	static const std::string& GetRuleNotes(IntType t) { return s_RuleInfo[static_cast<int>(t)].notes; }
	static const std::string& GetRuleNotes(RealType t) { return s_RuleInfo[static_cast<int>(t) + IntRuleCount].notes; }
	static const std::string& GetRuleNotes(BoolType t) { return s_RuleInfo[static_cast<int>(t) + IntRuleCount + RealRuleCount].notes; }
	static const std::string& GetRuleNotes(StringType t) { return s_RuleInfo[static_cast<int>(t) + IntRuleCount + RealRuleCount + StringRuleCount].notes; }
	static uint32 CountRules() { return RulesCount; }
	static CategoryType FindCategory(const std::string& category_name);
	bool ListRules(const std::string& category_name, std::vector<std::string>& l);
	bool ListCategories(std::vector<std::string>& l);
	bool GetRule(const std::string& rule_name, std::string& rule_value);
	bool SetRule(
		const std::string& rule_name,
		const std::string& rule_value,
		Database* db = nullptr,
		bool db_save = false,
		bool reload = false
	);

	int GetActiveRulesetID() const { return m_activeRuleset; }
	std::string GetActiveRuleset() const { return m_activeName; }
	static bool ListRulesets(Database* db, std::map<int, std::string>& l);

	void ResetRules(bool reload = false);
	bool LoadRules(Database* db, const std::string& rule_set_name, bool reload = false);
	void SaveRules(Database* db, const std::string& rule_set_name);
	bool UpdateInjectedRules(Database* db, const std::string& rule_set_name, bool quiet_update = false);
	bool UpdateOrphanedRules(Database* db, bool quiet_update = false);
	bool RestoreRuleNotes(Database* db);

private:
	RuleManager();
	RuleManager(const RuleManager &);
	const RuleManager &operator=(const RuleManager &);

	int         m_activeRuleset;
	std::string m_activeName;

	int         m_RuleIntValues[IntRuleCount];
	float       m_RuleRealValues[RealRuleCount];
	uint32      m_RuleBoolValues[BoolRuleCount];
	std::string m_RuleStringValues[StringRuleCount];

	typedef enum {
		IntRule,
		RealRule,
		BoolRule,
		StringRule
	} RuleType;

	static bool _FindRule(const std::string& rule_name, RuleType& type_into, uint16& index_into);
	static std::string _GetRuleName(RuleType type, uint16 index);
	static const std::string& _GetRuleNotes(RuleType type, uint16 index);
	static int _FindOrCreateRuleset(Database* db, const std::string& rule_set_name);
	void _SaveRule(Database* db, RuleType type, uint16 index);

	static const char* s_categoryNames[];
	typedef struct {
		std::string       name;
		CategoryType      category;
		RuleType          type;
		uint16            rule_index;
		const std::string notes;
	} RuleInfo;
	static const RuleInfo s_RuleInfo[];
};
