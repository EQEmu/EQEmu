#include "titles.h"

#include "common/eq_packet_structs.h"
#include "common/misc_functions.h"
#include "common/repositories/player_titlesets_repository.h"
#include "common/strings.h"
#include "zone/client.h"
#include "zone/mob.h"
#include "zone/worldserver.h"

extern WorldServer worldserver;

TitleManager::TitleManager() {
}

bool TitleManager::LoadTitles()
{
	titles.clear();

	const auto& l = TitlesRepository::All(database);

	if (l.empty()) {
		return false;
	}

	for (const auto& e : l) {
		titles.push_back(e);
	}

	LogInfo("Loaded [{}] Title{}", Strings::Commify(l.size()), l.size() != 1 ? "s" : "");

	return true;
}

EQApplicationPacket* TitleManager::MakeTitlesPacket(Client* c)
{
	const auto& eligible_titles = GetEligibleTitles(c);

	uint32 total_length = 4;

	for (const auto& e : eligible_titles) {
		total_length += e.prefix.length() + e.suffix.length() + 6;
	}

	auto outapp = new EQApplicationPacket(OP_SendTitleList, total_length);
	char* buffer = (char*) outapp->pBuffer;

	VARSTRUCT_ENCODE_TYPE(uint32, buffer, eligible_titles.size());

	for (const auto& t : eligible_titles) {
		VARSTRUCT_ENCODE_TYPE(uint32, buffer, t.id);
		VARSTRUCT_ENCODE_STRING(buffer, t.prefix.c_str());
		VARSTRUCT_ENCODE_STRING(buffer, t.suffix.c_str());
	}

	return outapp;
}

std::string TitleManager::GetPrefix(int title_id)
{
	if (!title_id) {
		return "";
	}

	auto e = std::find_if(
		titles.begin(),
		titles.end(),
		[title_id](const auto& t) {
			return t.id == title_id;
		}
	);

	return e != titles.end() ? e->prefix : "";
}

std::string TitleManager::GetSuffix(int title_id)
{
	if (!title_id) {
		return "";
	}

	auto e = std::find_if(
		titles.begin(),
		titles.end(),
		[title_id](const auto& t) {
			return t.id == title_id;
		}
	);

	return e != titles.end() ? e->suffix : "";
}

bool TitleManager::HasTitle(Client* c, uint32 title_id)
{
	if (!c || !title_id) {
		return false;
	}

	const auto& eligible_titles = GetEligibleTitles(c);

	return std::any_of(
		eligible_titles.begin(),
		eligible_titles.end(),
		[title_id](const auto& t) {
			return t.id == title_id;
		}
	);
}

std::vector<TitlesRepository::Titles> TitleManager::GetEligibleTitles(Client* c)
{
	std::vector<TitlesRepository::Titles> eligible_titles = {};
	if (!c) {
		return eligible_titles;
	}

	const auto& player_title_sets = PlayerTitlesetsRepository::GetWhere(
		database,
		fmt::format(
			"`char_id` = {}",
			c->CharacterID()
		)
	);

	for (auto t : titles) {
		if (t.char_id >= 0 && c->CharacterID() != static_cast<uint32>(t.char_id)) {
			continue;
		}

		if (t.status >= 0 && c->Admin() < t.status) {
			continue;
		}

		if (t.gender >= Gender::Male && c->GetBaseGender() != t.gender) {
			continue;
		}

		if (t.class_ >= Class::None && c->GetBaseClass() != t.class_) {
			continue;
		}

		if (t.min_aa_points >= 0 && c->GetSpentAA() < t.min_aa_points) {
			continue;
		}

		if (t.max_aa_points >= 0 && c->GetSpentAA() > t.max_aa_points) {
			continue;
		}

		if (t.skill_id >= 0) {
			auto skill_id = static_cast<EQ::skills::SkillType>(t.skill_id);
			if (
				t.min_skill_value >= 0 &&
				c->GetRawSkill(skill_id) < static_cast<uint32>(t.min_skill_value)
			) {
				continue;
			}

			if (
				t.max_skill_value >= 0 &&
				c->GetRawSkill(skill_id) > static_cast<uint32>(t.max_skill_value)
			) {
				continue;
			}
		}

		if (t.item_id >= 1 && c->GetInv().HasItem(t.item_id) == INVALID_INDEX) {
			continue;
		}

		if (
			t.title_set > 0 &&
			!std::any_of(
				player_title_sets.begin(),
				player_title_sets.end(),
				[t](const auto& e) {
					return e.title_set == t.title_set;
				}
			)
		) {
			continue;
		}

		eligible_titles.emplace_back(t);
	}

	return eligible_titles;
}

bool TitleManager::IsNewAATitleAvailable(int aa_points, int class_id)
{
	return std::any_of(
		titles.begin(),
		titles.end(),
		[class_id, aa_points](const auto& t) {
			return (
				(t.class_ == -1 || t.class_ == class_id) &&
				t.min_aa_points == aa_points
			);
		}
	);
}

bool TitleManager::IsNewTradeSkillTitleAvailable(int skill_id, int skill_value)
{
	return std::any_of(
		titles.begin(),
		titles.end(),
		[skill_id, skill_value](const auto& t) {
			return t.skill_id == skill_id && t.min_skill_value == skill_value;
		}
	);
}

void TitleManager::CreateNewPlayerTitle(Client* c, std::string title)
{
	if (!c || title.empty()) {
		return;
	}

	c->SetAATitle(title);

	const auto& l = TitlesRepository::GetWhere(
		database,
		fmt::format(
			"`char_id` = {} AND `prefix` = '{}'",
			c->CharacterID(),
			Strings::Escape(title)
		)
	);

	if (!l.empty()) {
		return;
	}

	auto e = TitlesRepository::NewEntity();

	e.char_id = c->CharacterID();
	e.prefix  = title;

	if (!TitlesRepository::InsertOne(database, e).id) {
		return;
	}

	worldserver.SendReload(ServerReload::Type::Titles);
}

void TitleManager::CreateNewPlayerSuffix(Client* c, std::string suffix)
{
	if (!c || suffix.empty()) {
		return;
	}

	c->SetTitleSuffix(suffix);

	const auto& l = TitlesRepository::GetWhere(
		database,
		fmt::format(
			"`char_id` = {} AND `suffix` = '{}'",
			c->CharacterID(),
			Strings::Escape(suffix)
		)
	);

	if (!l.empty()) {
		return;
	}

	auto e = TitlesRepository::NewEntity();

	e.char_id = c->CharacterID();
	e.suffix  = suffix;

	if (!TitlesRepository::InsertOne(database, e).id) {
		return;
	}

	worldserver.SendReload(ServerReload::Type::Titles);
}

void Client::SetAATitle(std::string title)
{
	strn0cpy(m_pp.title, title.c_str(), sizeof(m_pp.title));

	auto outapp = new EQApplicationPacket(OP_SetTitleReply, sizeof(SetTitleReply_Struct));
	auto strs   = (SetTitleReply_Struct*) outapp->pBuffer;

	strn0cpy(strs->title, title.c_str(), sizeof(strs->title));
	strs->entity_id = GetID();

	entity_list.QueueClients(this, outapp, false);
	safe_delete(outapp);
}

void Client::SetTitleSuffix(std::string suffix)
{
	strn0cpy(m_pp.suffix, suffix.c_str(), sizeof(m_pp.suffix));

	auto outapp = new EQApplicationPacket(OP_SetTitleReply, sizeof(SetTitleReply_Struct));
	auto strs   = (SetTitleReply_Struct*) outapp->pBuffer;

	strs->is_suffix = 1;
	strn0cpy(strs->title, suffix.c_str(), sizeof(strs->title));
	strs->entity_id = GetID();

	entity_list.QueueClients(this, outapp, false);
	safe_delete(outapp);
}

void Client::EnableTitle(int title_set, bool insert)
{
	if (CheckTitle(title_set)) {
		return;
	}

	auto e = PlayerTitlesetsRepository::NewEntity();

	e.char_id   = CharacterID();
	e.title_set = title_set;

	if (insert) {
		e = PlayerTitlesetsRepository::InsertOne(database, e);
		if (!e.id) {
			LogError("Error in EnableTitle query for titleset [{}] and charid [{}]", title_set, CharacterID());
			return;
		}
	}

	m_player_title_sets.emplace_back(e);
}

bool Client::CheckTitle(int title_set)
{
	for (const auto& e : m_player_title_sets) {
		if (e.title_set == title_set) {
			return true;
		}
	}

	return false;
}

void Client::RemoveTitle(int title_set)
{
	if (!CheckTitle(title_set)) {
		return;
	}

	for (const auto& t : title_manager.GetTitles()) {
		if (t.title_set == title_set) {
			if (std::string(m_pp.title) == t.prefix) {
				SetAATitle("");
			}

			if (std::string(m_pp.suffix) == t.suffix) {
				SetTitleSuffix("");
			}

			break;
		}
	}

	auto& titles = m_player_title_sets;
	for (auto e = titles.begin(); e != titles.end(); e++) {
		if (e->title_set == title_set) {
			titles.erase(e);
			break;
		}
	}

	PlayerTitlesetsRepository::DeleteWhere(
		database,
		fmt::format(
			"`title_set` = {} AND `char_id` = {}",
			title_set,
			CharacterID()
		)
	);
}
