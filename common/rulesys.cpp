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

#include "rulesys.h"
#include "eqemu_logsys.h"
#include "database.h"
#include "strings.h"
#include <cstdlib>
#include <cstring>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include "../common/repositories/rule_sets_repository.h"
#include "../common/repositories/rule_values_repository.h"

const char *RuleManager::s_categoryNames[_CatCount + 1] = {
	#define RULE_CATEGORY(category_name) \
		#category_name ,
	#include "ruletypes.h"
	"InvalidCategory"
};

const RuleManager::RuleInfo RuleManager::s_RuleInfo[IntRuleCount + RealRuleCount + BoolRuleCount + StringRuleCount + 1] = {
	/* this is done in three steps, so we can reliably get to them by index*/
	#define RULE_INT(category_name, rule_name, default_value, notes) \
		{ #category_name ":" #rule_name, Category__##category_name, IntRule, Int__##rule_name, notes },
	#include "ruletypes.h"
	#define RULE_REAL(category_name, rule_name, default_value, notes) \
		{ #category_name ":" #rule_name, Category__##category_name, RealRule, Real__##rule_name, notes },
	#include "ruletypes.h"
	#define RULE_BOOL(category_name, rule_name, default_value, notes) \
		{ #category_name ":" #rule_name, Category__##category_name, BoolRule, Bool__##rule_name, notes },
	#include "ruletypes.h"
	#define RULE_STRING(category_name, rule_name, default_value, notes) \
		{ #category_name ":" #rule_name, Category__##category_name, StringRule, String__##rule_name, notes },
	#include "ruletypes.h"
	{ "Invalid Rule", _CatCount, IntRule }
};

RuleManager::RuleManager()
	: m_activeRuleset(0),
	  m_activeName("default")
{
	ResetRules(false);
}

RuleManager::CategoryType RuleManager::FindCategory(const std::string &category_name)
{
	for (int i = 0; i < _CatCount; i++) {
		if (Strings::Contains(category_name, s_categoryNames[i])) {
			return static_cast<CategoryType>(i);
		}
	}

	return InvalidCategory;
}

bool RuleManager::ListRules(const std::string &category_name, std::vector<std::string> &l)
{
	CategoryType category_type = InvalidCategory;
	if (!category_name.empty()) {
		category_type = FindCategory(category_name);
		if (category_type == InvalidCategory) {
			LogRulesDetail("Unable to find category '{}'.", category_name);
			return false;
		}
	}

	for (int i = 0; i < CountRules(); i++) {
		const auto& r = s_RuleInfo[i];
		if (category_name.empty() || category_type == r.category) {
			l.push_back(r.name);
		}
	}

	return true;
}

bool RuleManager::ListCategories(std::vector <std::string> &l)
{
	for (int i = 0; i < _CatCount; i++) {
		l.push_back(s_categoryNames[i]);
	}

	return true;
}

bool RuleManager::GetRule(const std::string &rule_name, std::string &rule_value)
{
	RuleType type;
	uint16 index;
	if (!_FindRule(rule_name, type, index)) {
		return false;
	}

	switch (type) {
		case IntRule:
			rule_value = fmt::format("{}", m_RuleIntValues[index]);
			break;
		case RealRule:
			rule_value = fmt::format("{}", m_RuleRealValues[index]);
			break;
		case BoolRule:
			rule_value = m_RuleBoolValues[index] ? "true" : "false";
			break;
		case StringRule:
			rule_value = m_RuleStringValues[index];
			break;
	}

	return true;
}

bool RuleManager::SetRule(const std::string &rule_name, const std::string &rule_value, Database *db, bool db_save, bool reload)
{
	if (rule_name.empty() || rule_value.empty()) {
		return false;
	}

	RuleType type;
	uint16 index;
	if (!_FindRule(rule_name, type, index)) {
		return (false);
	}

	if (reload) {
		bool is_client = Strings::EqualFold(rule_name, "World:UseClientBasedExpansionSettings");
		bool is_world = Strings::EqualFold(rule_name, "World:ExpansionSettings");
		if (is_client || is_world) {
			return false;
		}
	}

	switch (type) {
		case IntRule:
			m_RuleIntValues[index] = Strings::ToInt(rule_value);
			LogRules("Set rule [{}] to value [{}]", rule_name, m_RuleIntValues[index]);
			break;
		case RealRule:
			m_RuleRealValues[index] = Strings::ToFloat(rule_value);
			LogRules("Set rule [{}] to value [{:.2f}]", rule_name, m_RuleRealValues[index]);
			break;
		case BoolRule:
			m_RuleBoolValues[index] = static_cast<uint32>(Strings::ToBool(rule_value));
			LogRules("Set rule [{}] to value [{}]", rule_name, m_RuleBoolValues[index] == 1 ? "true" : "false");
			break;
		case StringRule:
			m_RuleStringValues[index] = rule_value;
			LogRules("Set rule [{}] to value [{}]", rule_name, rule_value);
			break;
	}

	if (db_save) {
		_SaveRule(db, type, index);
	}

	return true;
}

void RuleManager::ResetRules(bool reload) {
	std::string client_rule;
	std::string world_rule;

	// these rules must not change during server runtime
	if (reload) {
		GetRule("World:UseClientBasedExpansionSettings", client_rule);
		GetRule("World:ExpansionSettings", world_rule);
	}

	LogRulesDetail("Resetting running rules to default values.");

	#define RULE_INT(category_name, rule_name, default_value, notes) \
		m_RuleIntValues[ Int__##rule_name ] = default_value;
	#define RULE_REAL(category_name, rule_name, default_value, notes) \
		m_RuleRealValues[ Real__##rule_name ] = default_value;
	#define RULE_BOOL(category_name, rule_name, default_value, notes) \
		m_RuleBoolValues[ Bool__##rule_name ] = default_value;
	#define RULE_STRING(category_name, rule_name, default_value, notes) \
		m_RuleStringValues[ String__##rule_name ] = default_value;
	#include "ruletypes.h"

	// restore these rules to their pre-reset values
	if (reload) {
		SetRule("World:UseClientBasedExpansionSettings", client_rule);
		SetRule("World:ExpansionSettings", world_rule);
	}
}

bool RuleManager::_FindRule(const std::string &rule_name, RuleType &type_into, uint16 &index_into) {
	if (rule_name.empty()) {
		return false;
	}

	for (int i = 0; i < CountRules(); i++) {
		const auto& r = s_RuleInfo[i];
		if (rule_name == r.name) {
			type_into = r.type;
			index_into = r.rule_index;
			return true;
		}
	}

	LogRulesDetail("Unable to find rule '{}'.", rule_name);
	return false;
}

//assumes index is valid!
std::string RuleManager::_GetRuleName(RuleType type, uint16 index) {
	switch (type) {
		case IntRule:
			return s_RuleInfo[index].name;
		case RealRule:
			return s_RuleInfo[index + IntRuleCount].name;
		case BoolRule:
			return s_RuleInfo[index + IntRuleCount + RealRuleCount].name;
		case StringRule:
			return s_RuleInfo[index + IntRuleCount + RealRuleCount + StringRuleCount].name;
		default:
			break;
	}

	return s_RuleInfo[IntRuleCount + RealRuleCount + BoolRuleCount + StringRuleCount].name;
}

//assumes index is valid!
const std::string &RuleManager::_GetRuleNotes(RuleType type, uint16 index) {
	switch (type) {
		case IntRule:
			return s_RuleInfo[index].notes;
		case RealRule:
			return s_RuleInfo[index + IntRuleCount].notes;
		case BoolRule:
			return s_RuleInfo[index + IntRuleCount + RealRuleCount].notes;
		case StringRule:
			return s_RuleInfo[index + IntRuleCount + RealRuleCount + StringRuleCount].notes;
		default:
			break;
	}

	return s_RuleInfo[IntRuleCount + RealRuleCount + BoolRuleCount + StringRuleCount].notes;
}

bool RuleManager::LoadRules(Database *db, const std::string &rule_set_name, bool reload) {
	const auto rule_set_id = RuleSetsRepository::GetRuleSetID(*db, rule_set_name);
	if (rule_set_id < 0) {
		LogRulesDetail("Failed to find Rule Set {} for load operation. Canceling.", rule_set_name);
		return false;
	}

	m_activeRuleset = rule_set_id;
	m_activeName = rule_set_name;

	/* Load default ruleset values first if we're loading something other than default */

	const std::string default_ruleset_name = "default";
	bool is_default = rule_set_name == default_ruleset_name;

	if (!is_default) {
		const auto default_rule_set_id = RuleSetsRepository::GetRuleSetID(*db, default_ruleset_name);

		if (default_rule_set_id < 0) {
			LogRulesDetail(
				"Failed to load default Rule Set {} for load operation.",
				default_ruleset_name
			);

			return false;
		}

		LogRulesDetail("Loading Rule Set {} ({}).", default_ruleset_name, default_rule_set_id);

		const auto& l = RuleValuesRepository::GetWhere(
			*db,
			fmt::format(
				"ruleset_id = {}",
				default_rule_set_id
			)
		);
		if (l.empty()) {
			return false;
		}

		for (const auto& e : l) {
			if (!SetRule(e.rule_name, e.rule_value, nullptr, false, reload)) {
				LogRulesDetail("Unable to interpret rule record for '{}'.", e.rule_name);
			}
		}
	}

	LogRulesDetail("Loading Rule Set {} ({}).", rule_set_name, rule_set_id);

	const auto& l = RuleValuesRepository::GetWhere(
		*db,
		fmt::format(
			"ruleset_id = {}",
			rule_set_id
		)
	);
	if (l.empty()) {
		return false;
	}

	for (const auto& e : l) {
		if (!SetRule(e.rule_name, e.rule_value, nullptr, false, reload)) {
			LogRulesDetail("Unable to interpret Rule record for Rule '{}'.", e.rule_name);
		}
	}

	LogInfo(
		"Loaded [{}] rules(s) in rule_set [{}] id [{}]",
		Strings::Commify(std::to_string(l.size())),
		rule_set_name,
		rule_set_id
	);

	return true;
}

void RuleManager::SaveRules(Database *db, const std::string &rule_set_name) {
	if (!rule_set_name.empty()) {
		if (m_activeName != rule_set_name) {
			m_activeRuleset = _FindOrCreateRuleset(db, rule_set_name);
			if (m_activeRuleset == -1) {
				LogRulesDetail("Unable to find or create Rule Set {}.", rule_set_name);
				return;
			}

			m_activeName = rule_set_name;
		}

		LogRulesDetail("Saving running rules into Rule Set {} ({}).", rule_set_name, m_activeRuleset);
	} else {
		LogRulesDetail("Saving running rules into running Rule Set {} ({}).", m_activeName, m_activeRuleset);
	}

	int i;
	for (i = 0; i < IntRuleCount; i++) {
		_SaveRule(db, IntRule, i);
	}

	for (i = 0; i < RealRuleCount; i++) {
		_SaveRule(db, RealRule, i);
	}

	for (i = 0; i < BoolRuleCount; i++) {
		_SaveRule(db, BoolRule, i);
	}

	for (i = 0; i < StringRuleCount; i++) {
		_SaveRule(db, StringRule, i);
	}
}

void RuleManager::_SaveRule(Database *db, RuleType type, uint16 index) {
	const auto rule_name = _GetRuleName(type, index);

	if (
		(type == BoolRule && Strings::EqualFold(rule_name, "World:UseClientBasedExpansionSettings")) ||
		(type == IntRule && Strings::EqualFold(rule_name, "World:ExpansionSettings"))
	) {
		return;
	}

	std::string rule_value;

	switch (type) {
		case IntRule:
			rule_value = fmt::format("{}", m_RuleIntValues[index]);
			break;
		case RealRule:
			rule_value = fmt::format("{:.13f}", m_RuleRealValues[index]);
			break;
		case BoolRule:
			rule_value = m_RuleBoolValues[index] ? "true" : "false";
			break;
		case StringRule:
			rule_value = m_RuleStringValues[index];
			break;
	}

	const auto& rule_notes = _GetRuleNotes(type, index);

	const auto& l = RuleValuesRepository::GetWhere(
		*db,
		fmt::format(
			"ruleset_id = {} AND rule_name = '{}' LIMIT 1",
			m_activeRuleset,
			rule_name
		)
	);

	if (!l.empty()) {
		auto e = l[0];
		e.rule_value = rule_value;
		e.notes      = rule_notes;

		db->QueryDatabase(
			fmt::format(
				"UPDATE rule_values SET rule_value = '{}', notes = '{}' WHERE ruleset_id = {} AND rule_name = '{}'",
				rule_value,
				Strings::Escape(rule_notes),
				e.ruleset_id,
				e.rule_name
			)
		);

		return;
	}

	auto e = RuleValuesRepository::NewEntity();

	e.ruleset_id = m_activeRuleset;
	e.rule_name  = _GetRuleName(type, index);
	e.rule_value = rule_value;
	e.notes      = rule_notes;

	RuleValuesRepository::InsertOne(*db, e);
}

bool RuleManager::UpdateInjectedRules(Database *db, const std::string &rule_set_name, bool quiet_update)
{
	std::map<std::string, std::pair<std::string, const std::string *>>  rule_data;
	std::vector<std::tuple<int, std::string, std::string, std::string>> injected_rule_entries;

	if (rule_set_name.empty()) {
		return false;
	}

	const auto rule_set_id = RuleSetsRepository::GetRuleSetID(*db, rule_set_name);
	if (rule_set_id < 0) {
		return false;
	}

	const auto& v = RuleValuesRepository::GetRuleNames(*db, rule_set_id);
	if (v.empty()) {
		return false;
	}

	// build rule data entries
	for (const auto& r : s_RuleInfo) {
		if (Strings::EqualFold(r.name, "Invalid Rule")) {
			continue;
		}

		switch (r.type) {
			case IntRule:
				rule_data[r.name].first = fmt::format("{}", m_RuleIntValues[r.rule_index]);
				rule_data[r.name].second = &r.notes;
				break;
			case RealRule:
				rule_data[r.name].first = fmt::format("{:.13f}", m_RuleRealValues[r.rule_index]);
				rule_data[r.name].second = &r.notes;
				break;
			case BoolRule:
				rule_data[r.name].first = fmt::format("{}", m_RuleBoolValues[r.rule_index] ? "true" : "false");
				rule_data[r.name].second = &r.notes;
				break;
			case StringRule:
				rule_data[r.name].first = m_RuleStringValues[r.rule_index];
				rule_data[r.name].second = &r.notes;
				break;
			default:
				break;
		}
	}

	// build injected entries
	for (const auto &d : rule_data) {
		if (std::find(v.begin(), v.end(), d.first) == v.end()) {
			injected_rule_entries.push_back(
				std::tuple<int, std::string, std::string, std::string>(
					rule_set_id,
					d.first,
					d.second.first,
					Strings::Escape(*d.second.second)
				)
			);

			if (!quiet_update) {
				LogInfo(
					"Adding new rule [{}] ruleset [{}] ({}) value [{}]",
					d.first,
					rule_set_name,
					rule_set_id,
					d.second.first
				);
			}
		}
	}

	// update rules in the database where the description is different
	for (auto &e : RuleValuesRepository::All(*db)) {
		auto i = rule_data.find(e.rule_name);
		if (i != rule_data.end()) {
			// if notes are different, update them
			if (i->second.second != nullptr && *i->second.second != e.notes) {
				LogInfo("Updating rule [{}] notes to [{}]", i->first, *i->second.second);
				e.notes = *i->second.second;
				RuleValuesRepository::ReplaceOne(*db, e);
			}
		}
	}

	if (injected_rule_entries.size()) {
		if (!RuleValuesRepository::InjectRules(*db, injected_rule_entries)) {
			return false;
		}

		LogInfo(
			"[{}] New rule(s) added to ruleset [{}] [{}]",
			injected_rule_entries.size(),
			rule_set_name,
			rule_set_id
		);
	}

	return true;
}

bool RuleManager::UpdateOrphanedRules(Database *db, bool quiet_update)
{
	std::vector<std::string> rule_data;
	std::vector<std::string> orphaned_rule_entries;

	const auto& l = RuleValuesRepository::GetGroupedRules(*db);
	if (l.empty()) {
		return false;
	}

	// build rule data entries
	for (const auto &r : s_RuleInfo) {
		if (Strings::EqualFold(r.name, "Invalid Rule")) {
			continue;
		}

		rule_data.push_back(r.name);
	}

	for (const auto& e : l) {
		const auto &d = std::find(rule_data.begin(), rule_data.end(), e);
		if (d == rule_data.end()) {
			orphaned_rule_entries.push_back(e);

			if (!quiet_update) {
				LogInfo(
					"Rule [{}] no longer exists... Deleting orphaned entry from `rule_values` table...",
					e
				);
			}
		}
	}

	if (orphaned_rule_entries.size()) {
		if (!RuleValuesRepository::DeleteOrphanedRules(*db, orphaned_rule_entries)) {
			return false;
		}

		LogInfo("[{}] Orphaned Rule(s) Deleted from [All Rulesets] (-1)", orphaned_rule_entries.size());
	}

	return true;
}

bool RuleManager::RestoreRuleNotes(Database *db)
{
	const auto& l = RuleValuesRepository::All(*db);
	if (l.empty()) {
		return false;
	}

	int update_count = 0;
	for (const auto& e : l) {

		auto rule = [](std::string rule_name) {
			for (auto rule_iter : s_RuleInfo) {
				if (Strings::EqualFold(rule_iter.name, rule_name)) {
					return rule_iter;
				}
			}

			return s_RuleInfo[IntRuleCount + RealRuleCount + BoolRuleCount + StringRuleCount];
		}(e.rule_name);

		if (Strings::Contains(rule.name, e.rule_name)) {
			continue;
		}

		if (!e.notes.empty() && !rule.notes.compare(e.notes)) {
			continue;
		}

		if (!RuleValuesRepository::UpdateRuleNote(*db, e.ruleset_id, e.rule_name, rule.notes)) {
			continue;
		}

		++update_count;
	}

	if (update_count) {
		LogInfo("[{}] Rule Note{} Restored", update_count, update_count != 1 ? "s" : "");
	}

	return true;
}

int RuleManager::_FindOrCreateRuleset(Database *db, const std::string &rule_set_name)
{
	const auto rule_set_id = RuleSetsRepository::GetRuleSetID(*db, rule_set_name);
	if (rule_set_id >= 0) {
		return rule_set_id;
	}

	return RuleSetsRepository::CreateNewRuleSet(*db, rule_set_name);
}

bool RuleManager::ListRulesets(Database *db, std::map<int, std::string> &m)
{
	m[0] = "default";

	const auto& l = RuleSetsRepository::All(*db);
	if (l.empty()) {
		return false;
	}

	for (const auto& e : l) {
		m[e.ruleset_id] = e.name;
	}

	return true;
}

int RuleManager::GetIntRule(RuleManager::IntType t) const
{
	return m_RuleIntValues[t];
}

float RuleManager::GetRealRule(RuleManager::RealType t) const
{
	return m_RuleRealValues[t];
}

bool RuleManager::GetBoolRule(RuleManager::BoolType t) const
{
	return m_RuleBoolValues[t] == 1;
}

std::string RuleManager::GetStringRule(RuleManager::StringType t) const
{
	return m_RuleStringValues[t];
}
