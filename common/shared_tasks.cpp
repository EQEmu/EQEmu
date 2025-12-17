#include "shared_tasks.h"

#include "common/repositories/character_data_repository.h"

#include <algorithm>

std::vector<SharedTaskActivityStateEntry> SharedTask::GetActivityState() const
{
	return m_shared_task_activity_state;
}

const std::vector<SharedTaskMember>& SharedTask::GetMembers() const
{
	return m_members;
}

void SharedTask::SetSharedTaskActivityState(const std::vector<SharedTaskActivityStateEntry> &activity_state)
{
	SharedTask::m_shared_task_activity_state = activity_state;
}

void SharedTask::SetTaskData(const TasksRepository::Tasks &task_data)
{
	SharedTask::m_task_data = task_data;
}

void SharedTask::SetTaskActivityData(const std::vector<TaskActivitiesRepository::TaskActivities> &task_activity_data)
{
	SharedTask::m_task_activity_data = task_activity_data;
}

const TasksRepository::Tasks &SharedTask::GetTaskData() const
{
	return m_task_data;
}

const std::vector<TaskActivitiesRepository::TaskActivities> &SharedTask::GetTaskActivityData() const
{
	return m_task_activity_data;
}

void SharedTask::SetMembers(const std::vector<SharedTaskMember> &members)
{
	SharedTask::m_members = members;
}

const SharedTasksRepository::SharedTasks &SharedTask::GetDbSharedTask() const
{
	return m_db_shared_task;
}

void SharedTask::SetDbSharedTask(const SharedTasksRepository::SharedTasks &t)
{
	SharedTask::m_db_shared_task = t;
}

SharedTaskRequest SharedTask::GetRequestCharacters(Database &db, uint32_t requested_character_id)
{
	// todo: group/raid refactors to check online members without querying
	SharedTaskRequest request{};

	request.group_type = SharedTaskRequestGroupType::Group;
	auto characters = CharacterDataRepository::GetWhere(
		db, fmt::format(
			"id IN (select character_id from group_id where group_id = (select group_id from group_id where character_id = {}))",
			requested_character_id
		)
	);

	if (characters.empty()) {
		request.group_type = SharedTaskRequestGroupType::Raid;
		characters = CharacterDataRepository::GetWhere(
			db, fmt::format(
				"id IN (select charid from raid_members where raidid = (select raidid from raid_members where charid = {}))",
				requested_character_id
			)
		);
	}

	if (characters.empty()) // solo request
	{
		request.group_type = SharedTaskRequestGroupType::Solo;
		characters = CharacterDataRepository::GetWhere(
			db, fmt::format(
				"id = {} LIMIT 1",
				requested_character_id
			)
		);
	}

	request.leader_id     = requested_character_id;
	request.lowest_level  = std::numeric_limits<uint8_t>::max();
	request.highest_level = 0;
	request.members.reserve(characters.size());
	for (const auto& character: characters)
	{
		request.lowest_level  = std::min(request.lowest_level, character.level);
		request.highest_level = std::max(request.highest_level, character.level);
		request.character_ids.emplace_back(character.id); // convenience

		SharedTaskMember member = {};
		member.character_id   = character.id;
		member.character_name = character.name;
		if (character.id == requested_character_id)
		{
			member.is_leader = true;
		}
		request.members.emplace_back(member);
	}

	return request;
}

void SharedTask::AddCharacterToMemberHistory(uint32_t character_id)
{
	auto it = std::find(member_id_history.begin(), member_id_history.end(), character_id);
	if (it == member_id_history.end()) {
		member_id_history.emplace_back(character_id);
	}
}

SharedTaskMember SharedTask::FindMemberFromCharacterID(uint32_t character_id) const
{
	auto it = std::find_if(
		m_members.begin(), m_members.end(),
		[&](const SharedTaskMember &member) {
			return member.character_id == character_id;
		}
	);

	return it != m_members.end() ? *it : SharedTaskMember{};
}

SharedTaskMember SharedTask::FindMemberFromCharacterName(const std::string &character_name) const
{
	auto it = std::find_if(
		m_members.begin(), m_members.end(),
		[&](const SharedTaskMember &member) {
			return strcasecmp(member.character_name.c_str(), character_name.c_str()) == 0;
		}
	);

	return it != m_members.end() ? *it : SharedTaskMember{};
}

SharedTaskMember SharedTask::GetLeader() const
{
	for (const auto &member : m_members) {
		if (member.is_leader) {
			return member;
		}
	}
	return {};
}

bool SharedTask::IsExpired() const
{
	return m_db_shared_task.expire_time > 0 && m_db_shared_task.expire_time < std::time(nullptr);
}
