#include "task_client_state.h"

#include "common/events/player_event_logs.h"
#include "common/misc_functions.h"
#include "common/repositories/character_activities_repository.h"
#include "common/repositories/character_task_timers_repository.h"
#include "common/repositories/character_tasks_repository.h"
#include "common/repositories/completed_tasks_repository.h"
#include "common/rulesys.h"
#include "common/shared_tasks.h"
#include "zone/client.h"
#include "zone/dynamic_zone.h"
#include "zone/queryserv.h"
#include "zone/quest_parser_collection.h"
#include "zone/string_ids.h"
#include "zone/worldserver.h"
#include "zone/zonedb.h"

#define EBON_CRYSTAL 40902
#define RADIANT_CRYSTAL 40903

extern WorldServer worldserver;
extern QueryServ   *QServ;

ClientTaskState::ClientTaskState()
{
	m_active_task_count          = 0;
	m_last_completed_task_loaded = 0;

	for (int i = 0; i < MAXACTIVEQUESTS; i++) {
		m_active_quests[i].slot    = i;
		m_active_quests[i].task_id = TASKSLOTEMPTY;
	}

	m_active_task = {};
	m_active_task.slot    = 0;
	m_active_task.task_id = TASKSLOTEMPTY;

	m_active_shared_task = {};
	m_active_shared_task.slot    = 0;
	m_active_shared_task.task_id = TASKSLOTEMPTY;
}

ClientTaskState::~ClientTaskState()
{
}

void ClientTaskState::SendTaskHistory(Client *client, int task_index)
{

	LogTasks("Task history requested for completed task index [{}]", task_index);

	// We only sent the most recent 50 completed tasks, so we need to offset the Index the client sent to us.

	int adjusted_task_index = task_index;
	if (m_completed_tasks.size() > 50) {
		adjusted_task_index += (m_completed_tasks.size() - 50);
	}

	if ((adjusted_task_index < 0) || (adjusted_task_index >= (int) m_completed_tasks.size())) {
		return;
	}

	int task_id = m_completed_tasks[adjusted_task_index].task_id;
	if (task_id < 0) {
		return;
	}

	const auto task_data = TaskManager::Instance()->GetTaskData(task_id);
	if (!task_data) {
		return;
	}

	TaskHistoryReplyHeader_Struct *task_history_reply;
	TaskHistoryReplyData1_Struct  *task_history_reply_data_1;
	TaskHistoryReplyData2_Struct  *task_history_reply_data_2;

	char *reply;

	int completed_activity_count = 0;
	int packet_length            = sizeof(TaskHistoryReplyHeader_Struct);

	for (int i = 0; i < task_data->activity_count; i++) {
		if (m_completed_tasks[adjusted_task_index].activity_done[i]) {
			completed_activity_count++;
			packet_length = packet_length + sizeof(TaskHistoryReplyData1_Struct) +
							task_data->activity_information[i].target_name.size() + 1 +
							task_data->activity_information[i].item_list.size() + 1 +
							sizeof(TaskHistoryReplyData2_Struct) +
							task_data->activity_information[i].description_override.size() + 1;
		}
	}

	auto outapp = new EQApplicationPacket(OP_TaskHistoryReply, packet_length);

	task_history_reply = (TaskHistoryReplyHeader_Struct *) outapp->pBuffer;

	// We use the TaskIndex the client sent in the request
	task_history_reply->TaskID        = task_index;
	task_history_reply->ActivityCount = completed_activity_count;

	reply = (char *) task_history_reply + sizeof(TaskHistoryReplyHeader_Struct);

	for (int i = 0; i < task_data->activity_count; i++) {
		if (m_completed_tasks[adjusted_task_index].activity_done[i]) {
			task_history_reply_data_1 = (TaskHistoryReplyData1_Struct *) reply;
			task_history_reply_data_1->ActivityType = static_cast<uint32_t>(task_data->activity_information[i].activity_type);
			reply = (char *) task_history_reply_data_1 + sizeof(TaskHistoryReplyData1_Struct);
			VARSTRUCT_ENCODE_STRING(reply, task_data->activity_information[i].target_name.c_str());
			VARSTRUCT_ENCODE_STRING(reply, task_data->activity_information[i].item_list.c_str());
			task_history_reply_data_2 = (TaskHistoryReplyData2_Struct *) reply;
			task_history_reply_data_2->GoalCount = task_data->activity_information[i].goal_count;
			task_history_reply_data_2->unknown04 = 0xffffffff;
			task_history_reply_data_2->unknown08 = 0xffffffff;
			task_history_reply_data_2->ZoneID    = task_data->activity_information[i].zone_ids.empty() ? 0
				: task_data->activity_information[i].zone_ids.front();
			task_history_reply_data_2->unknown16 = 0x00000000;
			reply = (char *) task_history_reply_data_2 + sizeof(TaskHistoryReplyData2_Struct);
			VARSTRUCT_ENCODE_STRING(reply, task_data->activity_information[i].description_override.c_str());
		}
	}

	client->QueuePacket(outapp);
	safe_delete(outapp);
}

void ClientTaskState::EnableTask(int character_id, int task_count, int *task_list)
{
	// Check if the Task is already enabled for this client
	std::vector<int> tasks_enabled;

	for (int i = 0; i < task_count; i++) {
		auto iterator = m_enabled_tasks.begin();
		bool add_task = true;

		while (iterator != m_enabled_tasks.end()) {
			// If this task is already enabled, stop looking
			if ((*iterator) == task_list[i]) {
				add_task = false;
				break;
			}
			// Our list of enabled tasks is sorted, so we can quit if we find a taskid higher than
			// the one we are looking for.
			if ((*iterator) > task_list[i]) {
				break;
			}
			++iterator;
		}

		if (add_task) {
			m_enabled_tasks.insert(iterator, task_list[i]);
			// Make a note of the task we enabled, for later SQL generation
			tasks_enabled.push_back(task_list[i]);
		}
	}

	LogTasksDetail("New enabled task list");
	for (int enabled_task : m_enabled_tasks) {
		LogTasksDetail("enabled [{}] character_id [{}]", enabled_task, character_id);
	}

	if (tasks_enabled.empty()) {
		return;
	}

	std::stringstream query_stream;
	query_stream << "REPLACE INTO character_enabledtasks (charid, taskid) VALUES ";
	for (unsigned int i = 0; i < tasks_enabled.size(); i++) {
		query_stream << (i ? ", " : "") << StringFormat("(%i, %i)", character_id, tasks_enabled[i]);
	}

	database.QueryDatabase(query_stream.str());
}

void ClientTaskState::DisableTask(int character_id, int task_count, int *task_list)
{

	// Check if the Task is enabled for this client
	std::vector<int> tasks_disabled;

	for (int task_id = 0; task_id < task_count; task_id++) {
		auto iterator   = m_enabled_tasks.begin();
		bool removeTask = false;

		while (iterator != m_enabled_tasks.end()) {
			if ((*iterator) == task_list[task_id]) {
				removeTask = true;
				break;
			}

			if ((*iterator) > task_list[task_id]) {
				break;
			}

			++iterator;
		}

		if (removeTask) {
			m_enabled_tasks.erase(iterator);
			tasks_disabled.push_back(task_list[task_id]);
		}
	}

	LogTasks("New enabled task list ");
	for (int enabled_task : m_enabled_tasks) {
		LogTasks("enabled_tasks [{}]", enabled_task);
	}

	if (tasks_disabled.empty()) {
		return;
	}

	std::stringstream queryStream;
	queryStream << StringFormat("DELETE FROM character_enabledtasks WHERE charid = %i AND (", character_id);

	for (unsigned int i = 0; i < tasks_disabled.size(); i++)
		queryStream
			<< (i ? StringFormat("taskid = %i ", tasks_disabled[i]) : StringFormat(
				"OR taskid = %i ",
				tasks_disabled[i]
			));

	queryStream << ")";

	database.QueryDatabase(queryStream.str());
}

bool ClientTaskState::IsTaskEnabled(int task_id)
{
	std::vector<int>::iterator iterator;

	iterator = m_enabled_tasks.begin();

	while (iterator != m_enabled_tasks.end()) {
		if ((*iterator) == task_id) { return true; }
		if ((*iterator) > task_id) { break; }
		++iterator;
	}

	return false;
}

int ClientTaskState::EnabledTaskCount(int task_set_id)
{

	// Return the number of tasks in TaskSet that this character is enabled for.

	unsigned int enabled_task_index = 0;
	unsigned int task_set_index     = 0;
	int          enabled_task_count = 0;

	if ((task_set_id <= 0) || (task_set_id >= MAXTASKSETS)) { return -1; }

	while ((enabled_task_index < m_enabled_tasks.size()) &&
		   (task_set_index < TaskManager::Instance()->m_task_sets[task_set_id].size())) {

		if (m_enabled_tasks[enabled_task_index] == TaskManager::Instance()->m_task_sets[task_set_id][task_set_index]) {
			enabled_task_count++;
			enabled_task_index++;
			task_set_index++;
			continue;
		}

		if (m_enabled_tasks[enabled_task_index] < TaskManager::Instance()->m_task_sets[task_set_id][task_set_index]) {
			enabled_task_index++;
		}
		else {
			task_set_index++;
		}

	}

	return enabled_task_count;
}

int ClientTaskState::ActiveTasksInSet(int task_set_id)
{
	if ((task_set_id <= 0) || (task_set_id >= MAXTASKSETS)) {
		return -1;
	}

	int active_task_in_set_count = 0;

	for (int task_id : TaskManager::Instance()->m_task_sets[task_set_id]) {
		if (IsTaskActive(task_id)) {
			active_task_in_set_count++;
		}
	}

	return active_task_in_set_count;
}

int ClientTaskState::CompletedTasksInSet(int task_set_id)
{
	if ((task_set_id <= 0) || (task_set_id >= MAXTASKSETS)) {
		return -1;
	}

	int completed_tasks_count = 0;

	for (int i : TaskManager::Instance()->m_task_sets[task_set_id]) {
		if (IsTaskCompleted(i)) {
			completed_tasks_count++;
		}
	}

	return completed_tasks_count;
}

bool ClientTaskState::HasSlotForTask(const TaskInformation* task)
{
	if (task == nullptr) {
		return false;
	}

	switch (task->type) {
		case TaskType::Task:
			return m_active_task.task_id == TASKSLOTEMPTY;
		case TaskType::Shared:
			return m_active_shared_task.task_id == TASKSLOTEMPTY;
		case TaskType::Quest:
			for (auto &active_quest : m_active_quests) {
				if (active_quest.task_id == TASKSLOTEMPTY) {
					return true;
				}
			}
		case TaskType::E:
			return false; // removed on live
	}

	return false;
}

int ClientTaskState::GetActiveTaskID(int index)
{
	// Return the task_id from the client's specified Active Task slot.
	if ((index < 0) || (index >= MAXACTIVEQUESTS)) {
		return 0;
	}

	return m_active_quests[index].task_id;
}

static void DeleteCompletedTaskFromDatabase(int character_id, int task_id)
{
	LogTasks("character_id [{}], task_id [{}]", character_id, task_id);

	CompletedTasksRepository::DeleteWhere(
		database,
		fmt::format("charid = {} and taskid = {}", character_id, task_id)
	);
}

bool ClientTaskState::UnlockActivities(Client* client, ClientTaskInformation& task_info)
{
	LogTasksDetail(
		"Fetching task info for character_id [{}] task [{}] slot [{}] accepted_time [{}] updated [{}]",
		client->CharacterID(),
		task_info.task_id,
		task_info.slot,
		task_info.accepted_time,
		task_info.updated
	);

	const auto task = TaskManager::Instance()->GetTaskData(task_info.task_id);
	if (!task)
	{
		return true;
	}

	for (int i = 0; i < task->activity_count; ++i)
	{
		if (task_info.activity[i].activity_id >= 0) {
			LogTasksDetail(
				"character_id [{}] task [{}] activity_id [{}] done_count [{}] activity_state [{}] updated [{}]",
				client->CharacterID(),
				task_info.task_id,
				task_info.activity[i].activity_id,
				task_info.activity[i].done_count,
				task_info.activity[i].activity_state,
				task_info.activity[i].updated
			);
		}
	}

	auto res = Tasks::GetActiveElements(task->activity_information, task_info.activity, task->activity_count);

	for (int activity_id : res.active)
	{
		ClientActivityInformation& client_activity = task_info.activity[activity_id];
		if (client_activity.activity_state == ActivityHidden)
		{
			LogTasksDetail("task [{}] activity [{}] (ActivityActive)", task_info.task_id, activity_id);
			client_activity.activity_state = ActivityActive;
			client_activity.updated = true;
		}
	}

	if (res.is_task_complete && RuleB(TaskSystem, RecordCompletedTasks))
	{
		RecordCompletedTask(client->CharacterID(), *task, task_info);
	}

	// check if client has an explore task in current zone to enable task explore processing
	m_has_explore_task = HasExploreTask(client);

	return res.is_task_complete;
}

void ClientTaskState::RecordCompletedTask(uint32_t character_id, const TaskInformation& task, const ClientTaskInformation& client_task)
{
	// If we are only keeping one completed record per task, and the player has done
	// the same task again, erase the previous completed entry for this task.
	if (RuleB(TasksSystem, KeepOneRecordPerCompletedTask))
	{
		size_t before = m_completed_tasks.size();

		m_completed_tasks.erase(std::remove_if(m_completed_tasks.begin(), m_completed_tasks.end(),
			[&](const CompletedTaskInformation& completed) { return completed.task_id == client_task.task_id; }
		), m_completed_tasks.end());

		size_t erased = before - m_completed_tasks.size();

		LogTasksDetail("KeepOneRecord erased [{}] elements", erased);

		if (erased > 0)
		{
			m_last_completed_task_loaded -= erased;
			DeleteCompletedTaskFromDatabase(character_id, client_task.task_id);
		}
	}

	if (task.type != TaskType::Shared)
	{
		CompletedTaskInformation completed{};
		completed.task_id = client_task.task_id;
		completed.completed_time = std::time(nullptr);

		for (int i = 0; i < task.activity_count; ++i)
		{
			completed.activity_done[i] = (client_task.activity[i].activity_state == ActivityCompleted);
		}

		LogTasksDetail("[{}] for character [{}]", client_task.task_id, character_id);
		m_completed_tasks.push_back(completed);
	}
}

const TaskInformation* ClientTaskState::GetTaskData(const ClientTaskInformation& client_task) const
{
	if (client_task.task_id == TASKSLOTEMPTY)
	{
		return nullptr;
	}

	return TaskManager::Instance()->GetTaskData(client_task.task_id);
}

bool ClientTaskState::CanUpdate(Client* client, const TaskUpdateFilter& filter, int task_id,
	const ActivityInformation& activity, const ClientActivityInformation& client_activity) const
{
	if (activity.goal_method == METHODQUEST && activity.goal_method != filter.method)
	{
		return false;
	}

	// todo: some tasks do allow hidden/unlocked elements to silently update
	if (client_activity.activity_state != ActivityActive)
	{
		return false;
	}

	if (activity.activity_type != filter.type)
	{
		return false;
	}

	if (activity.dz_switch_id != 0 && activity.dz_switch_id != filter.dz_switch_id)
	{
		return false;
	}

	if (!activity.CheckZone(zone->GetZoneID(), zone->GetInstanceVersion()))
	{
		LogTasks("client [{}] task [{}]-[{}] failed zone filter", client->GetName(), task_id, client_activity.activity_id);
		return false;
	}

	if (activity.has_area && !filter.ignore_area && RuleB(TaskSystem, EnableTaskProximity))
	{
		const glm::vec4& pos = filter.use_pos ? filter.pos : client->GetPosition();
		if (pos.x < activity.min_x || pos.x > activity.max_x ||
		    pos.y < activity.min_y || pos.y > activity.max_y ||
		    pos.z < activity.min_z || pos.z > activity.max_z)
		{
			LogTasksDetail("client [{}] task [{}]-[{}] failed area filter", client->GetName(), task_id, client_activity.activity_id);
			return false;
		}
	}

	// item is only checked for updates that provide an item to check (unlike npc which may be null for non-npcs)
	if (!activity.item_id_list.empty() && filter.item_id != 0 &&
	    !Tasks::IsInMatchList(activity.item_id_list, std::to_string(filter.item_id)))
	{
		LogTasks("client [{}] task [{}]-[{}] failed item match filter", client->GetName(), task_id, client_activity.activity_id);
		return false;
	}

	// npc filter supports both npc names and ids in match lists
	if (!activity.npc_match_list.empty() && (!filter.mob ||
	    (!Tasks::IsInMatchListPartial(activity.npc_match_list, filter.mob->GetName()) &&
	     !Tasks::IsInMatchListPartial(activity.npc_match_list, filter.mob->GetCleanName()) &&
	     !Tasks::IsInMatchList(activity.npc_match_list, std::to_string(filter.mob->GetNPCTypeID())))))
	{
		LogTasks("client [{}] task [{}]-[{}] failed npc match filter", client->GetName(), task_id, client_activity.activity_id);
		return false;
	}

	return true;
}

int ClientTaskState::UpdateTasks(Client* client, const TaskUpdateFilter& filter, int count)
{
	int max_updated = 0;

	for (const auto& client_task : m_active_tasks)
	{
		const auto task = GetTaskData(client_task);
		if (!task)
		{
			continue;
		}

		// legacy eqemu task update logic loops through group on kill of npc to update a single task
		// shared tasks only require one client to receive an update to propagate
		if (filter.type == TaskActivityType::Kill && task->type == TaskType::Shared && client != filter.exp_client)
		{
			continue;
		}

		for (const ClientActivityInformation& client_activity : client_task.activity)
		{
			const ActivityInformation& activity = task->activity_information[client_activity.activity_id];

			if (CanUpdate(client, filter, client_task.task_id, activity, client_activity))
			{
				if (parse->PlayerHasQuestSub(EVENT_TASK_BEFORE_UPDATE)) {
					const auto& export_string = fmt::format(
						"{} {} {}",
						count,
						client_activity.activity_id,
						client_task.task_id
					);

					if (parse->EventPlayer(EVENT_TASK_BEFORE_UPDATE, client, export_string, 0) != 0) {
						LogTasks(
							"client [{}] task [{}]-[{}] update prevented by quest",
							client->GetName(),
							client_task.task_id,
							client_activity.activity_id
						);

						continue;
					}
				}

				LogTasks(
					"client [{}] task [{}] activity [{}] increment [{}]",
					client->GetName(),
					client_task.task_id,
					client_activity.activity_id,
					count
				);

				int updated = IncrementDoneCount(client, task, client_task.slot, client_activity.activity_id, count);
				max_updated = std::max(max_updated, updated);

				if (RuleB(TaskSystem, UpdateOneElementPerTask))
				{
					break; // only one element updated per task, move to next task
				}
			}
		}
	}

	return max_updated;
}

std::pair<int, int> ClientTaskState::FindTask(Client* client, const TaskUpdateFilter& filter) const
{
	for (const auto& client_task : m_active_tasks)
	{
		const auto task = GetTaskData(client_task);
		if (!task || (filter.task_id != 0 && client_task.task_id != filter.task_id))
		{
			continue;
		}

		for (const ClientActivityInformation& client_activity : client_task.activity)
		{
			const ActivityInformation& activity = task->activity_information[client_activity.activity_id];
			if (CanUpdate(client, filter, client_task.task_id, activity, client_activity))
			{
				return std::make_pair(client_task.task_id, client_activity.activity_id);
			}
		}
	}

	return std::make_pair(0, 0);
}

bool ClientTaskState::HasExploreTask(Client* client) const
{
	TaskUpdateFilter filter{};
	filter.type = TaskActivityType::Explore;
	filter.ignore_area = true; // we don't care if client is currently in the explore area

	auto result = FindTask(client, filter);
	bool has_explore = result.first != 0;

	LogTasksDetail("client [{}] has explore task in current zone [{}]", client->GetName(), has_explore);
	return has_explore;
}

bool ClientTaskState::UpdateTasksOnSpeakWith(Client* client, NPC* npc)
{
	return UpdateTasksByNPC(client, TaskActivityType::SpeakWith, npc);
}

bool ClientTaskState::UpdateTasksByNPC(Client* client, TaskActivityType type, NPC* npc)
{
	TaskUpdateFilter filter{};
	filter.type = type;
	filter.mob = npc;

	return UpdateTasks(client, filter) > 0;
}

int ClientTaskState::ActiveSpeakTask(Client* client, NPC* npc)
{
	// This method is to be used from Perl quests only and returns the task_id of the first
	// active task found which has an active SpeakWith activity_information for this NPC.
	TaskUpdateFilter filter{};
	filter.type = TaskActivityType::SpeakWith;
	filter.mob = npc;
	filter.method = METHODQUEST;

	auto result = FindTask(client, filter);
	return result.first; // task id
}

int ClientTaskState::ActiveSpeakActivity(Client* client, NPC* npc, int task_id)
{
	// This method is to be used from Perl quests only and returns the activity_id of the first
	// active activity_information found in the specified task which is to SpeakWith this NPC.

	if (task_id <= 0)
	{
		return -1;
	}

	TaskUpdateFilter filter{};
	filter.type = TaskActivityType::SpeakWith;
	filter.mob = npc;
	filter.method = METHODQUEST;
	filter.task_id = task_id;

	auto result = FindTask(client, filter);
	return result.first != 0 ? result.second : -1; // activity id
}

void ClientTaskState::UpdateTasksForItem(Client* client, TaskActivityType type, int item_id, int count)
{

	// This method updates the client's task activities of the specified type which relate
	// to the specified item.
	//
	// Type should be one of ActivityTradeSkill, ActivityFish or ActivityForage

	LogTasks("activity_type [{}] item_id [{}] count [{}]", static_cast<int>(type), item_id, count);

	TaskUpdateFilter filter{};
	filter.type = type;
	filter.item_id = item_id;

	UpdateTasks(client, filter, count);
}

void ClientTaskState::UpdateTasksOnLoot(Client* client, Corpse* corpse, int item_id, int count)
{
	LogTasks("corpse [{}] item_id [{}] count [{}]", corpse->GetName(), item_id, count);

	TaskUpdateFilter filter{};
	filter.type = TaskActivityType::Loot;
	filter.mob = corpse;
	filter.item_id = item_id;

	UpdateTasks(client, filter, count);
}

void ClientTaskState::UpdateTasksOnExplore(Client* client, const glm::vec4& pos)
{
	LogTasksDetail("client [{}]", client->GetName());

	TaskUpdateFilter filter{};
	filter.type = TaskActivityType::Explore;
	filter.pos = pos;
	filter.use_pos = true;

	UpdateTasks(client, filter);
}

bool ClientTaskState::UpdateTasksOnDeliver(Client* client, std::vector<EQ::ItemInstance*>& items, Trade& trade, NPC* npc)
{
	LogTasks("npc [{}]", npc->GetName());

	bool is_updated = false;

	TaskUpdateFilter filter{};
	filter.mob = npc;

	int cash = trade.cp + (trade.sp * 10) + (trade.gp * 100) + (trade.pp * 1000);
	if (cash != 0)
	{
		filter.type = TaskActivityType::GiveCash;
		int updated_count = UpdateTasks(client, filter, cash);
		if (updated_count > 0)
		{
			// todo: remove used coin and use Deliver with explicit coin fields instead of custom type
			is_updated = true;
		}
	}

	filter.type = TaskActivityType::Deliver;
	for (EQ::ItemInstance*& item : items)
	{
		// items may have gaps for unused trade slots
		if (!item)
		{
			continue;
		}

		filter.item_id = item->GetID();

		int count = item->IsStackable() ? item->GetCharges() : 1;
		int updated_count = UpdateTasks(client, filter, count);
		if (updated_count > 0)
		{
			item->SetTaskDeliveredCount(updated_count);
			is_updated = true;
		}
	}

	return is_updated;
}

void ClientTaskState::UpdateTasksOnTouch(Client *client, int dz_switch_id)
{
	LogTasks("dz switch [{}] ", dz_switch_id);

	TaskUpdateFilter filter{};
	filter.type = TaskActivityType::Touch;
	filter.dz_switch_id = dz_switch_id;

	UpdateTasks(client, filter);
}

void ClientTaskState::UpdateTasksOnKill(Client* client, Client* exp_client, NPC* npc)
{
	TaskUpdateFilter filter{};
	filter.type = TaskActivityType::Kill;
	filter.mob = npc;
	filter.pos = npc->GetPosition(); // or should areas be filtered by client position?
	filter.use_pos = true;
	filter.exp_client = exp_client;

	UpdateTasks(client, filter);
}

int ClientTaskState::IncrementDoneCount(
	Client *client,
	const TaskInformation* task_data,
	int task_index,
	int activity_id,
	int count,
	bool ignore_quest_update
)
{
	auto info = GetClientTaskInfo(task_data->type, task_index);
	if (info == nullptr) {
		return 0;
	}

	LogTasks(
		"client [{}] task_id [{}] activity_id [{}] count [{}]",
		client->GetCleanName(),
		info->task_id,
		activity_id,
		count
	);

	int remaining = task_data->activity_information[activity_id].goal_count - info->activity[activity_id].done_count;
	count = std::min(count, remaining);

	// shared task shim
	// intercept and pass to world first before processing normally
	if (!client->m_shared_task_update && task_data->type == TaskType::Shared) {

		// struct
		auto pack = new ServerPacket(ServerOP_SharedTaskUpdate, sizeof(ServerSharedTaskActivityUpdate_Struct));
		auto *r   = (ServerSharedTaskActivityUpdate_Struct *) pack->pBuffer;

		// fill
		r->source_character_id = client->CharacterID();
		r->task_id             = info->task_id;
		r->activity_id         = activity_id;
		r->done_count          = info->activity[activity_id].done_count + count;
		r->ignore_quest_update = ignore_quest_update;

		LogTasksDetail(
			"shared_task sending client [{}] task_id [{}] activity_id [{}] count [{}] ignore_quest_update [{}]",
			r->source_character_id,
			r->task_id,
			r->activity_id,
			r->done_count,
			(ignore_quest_update ? "true" : "false")
		);

		SyncSharedTaskZoneClientDoneCountState(
			client,
			task_data->type,
			task_index,
			activity_id,
			r->done_count
		);

		// send
		worldserver.SendPacket(pack);
		safe_delete(pack);

		return count;
	}

	info->activity[activity_id].done_count += count;

	if (!ignore_quest_update) {
		if (parse->PlayerHasQuestSub(EVENT_TASK_UPDATE)) {
			const auto& export_string = fmt::format(
				"{} {} {}",
				info->activity[activity_id].done_count,
				info->activity[activity_id].activity_id,
				info->task_id
			);

			parse->EventPlayer(EVENT_TASK_UPDATE, client, export_string, 0);
		}
	}

	if (task_data->type != TaskType::Shared) {
		// live messages for each increment of non-shared tasks
		auto activity_type = task_data->activity_information[activity_id].activity_type;
		int msg_count = 1;

		if (activity_type != TaskActivityType::GiveCash) {
			msg_count = std::min(count, RuleI(TaskSystem, MaxUpdateMessages));
		}

		for (int i = 0; i < msg_count; ++i) {
			client->MessageString(Chat::DefaultText, TASK_UPDATED, task_data->title.c_str());
		}
	}

	info->activity[activity_id].updated = true;
	// Have we reached the goal count for this activity_information ?
	if (info->activity[activity_id].done_count >= task_data->activity_information[activity_id].goal_count) {
		LogTasks("done_count [{}] goal_count [{}] activity_id [{}]",
			info->activity[activity_id].done_count,
			task_data->activity_information[activity_id].goal_count,
			activity_id
		);

		// Flag the activity_information as complete
		info->activity[activity_id].activity_state = ActivityCompleted;
		// Unlock subsequent activities for this task
		bool task_complete = UnlockActivities(client, *info);
		LogTasks("task_complete is [{}]", task_complete);
		// shared tasks only send update messages on activity completion
		if (task_data->type == TaskType::Shared) {
			client->MessageString(Chat::DefaultText, TASK_UPDATED, task_data->title.c_str());
		}
		// and by the 'Task Stage Completed' message
		client->SendTaskActivityComplete(info->task_id, activity_id, task_index, task_data->type);
		// Send the updated task/activity_information list to the client
		TaskManager::Instance()->SendSingleActiveTaskToClient(client, *info, task_complete, false);

		if (!ignore_quest_update) {
			if (parse->PlayerHasQuestSub(EVENT_TASK_STAGE_COMPLETE)) {
				const auto& export_string = fmt::format(
					"{} {}",
					info->task_id,
					info->activity[activity_id].activity_id
				);

				parse->EventPlayer(EVENT_TASK_STAGE_COMPLETE, client, export_string, 0);
			}
		}

		// If this task is now complete, the Completed tasks will have been
		// updated in UnlockActivities. Send the completed task list to the
		// client. This is the same sequence the packets are sent on live.
		if (task_complete) {
			// world adds timers for shared tasks
			if (task_data->type != TaskType::Shared) {
				AddReplayTimer(client, *info, *task_data);
			}

			int event_res = DispatchEventTaskComplete(client, *info, activity_id);

			if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::TASK_COMPLETE)) {
				auto e = PlayerEvent::TaskCompleteEvent{
					.task_id = static_cast<uint32>(info->task_id),
					.task_name = TaskManager::Instance()->GetTaskName(static_cast<uint32>(info->task_id)),
					.activity_id = static_cast<uint32>(info->activity[activity_id].activity_id),
					.done_count = static_cast<uint32>(info->activity[activity_id].done_count)
				};
				RecordPlayerEventLogWithClient(client, PlayerEvent::TASK_COMPLETE, e);
			}

			client->SendTaskActivityComplete(info->task_id, 0, task_index, task_data->type, 0);

			// If Experience and/or cash rewards are set, reward them from the task even if reward_method is METHODQUEST
			// do not reward client if EVENT_TASK_COMPLETE returns non-zero
			if (event_res == 0)
			{
				RewardTask(client, task_data, *info);
			}
			//RemoveTask(c, TaskIndex);

			// shared tasks linger at the completion step and do not get removed from the task window unlike quests/task
			if (task_data->type != TaskType::Shared) {
				TaskManager::Instance()->SendCompletedTasksToClient(client, this);

				client->CancelTask(task_index, task_data->type);
			}

			client->LoadClientSharedCompletedTasks();
		}
	}
	else {
		// Send an updated packet for this single activity_information
		TaskManager::Instance()->SendTaskActivityLong(
			client,
			info->task_id,
			activity_id,
			task_index
		);

		if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::TASK_UPDATE)) {
			auto e = PlayerEvent::TaskUpdateEvent{
				.task_id = static_cast<uint32>(info->task_id),
				.task_name = TaskManager::Instance()->GetTaskName(static_cast<uint32>(info->task_id)),
				.activity_id = static_cast<uint32>(info->activity[activity_id].activity_id),
				.done_count = static_cast<uint32>(info->activity[activity_id].done_count)
			};
			RecordPlayerEventLogWithClient(client, PlayerEvent::TASK_UPDATE, e);
		}
	}

	TaskManager::Instance()->SaveClientState(client, this);

	return count;
}

int ClientTaskState::DispatchEventTaskComplete(Client* client, ClientTaskInformation& info, int activity_id)
{
	if (parse->PlayerHasQuestSub(EVENT_TASK_COMPLETE)) {
		const auto& export_string = fmt::format(
			"{} {} {}",
			info.activity[activity_id].done_count,
			info.activity[activity_id].activity_id,
			info.task_id
		);

		return parse->EventPlayer(EVENT_TASK_COMPLETE, client, export_string, 0);
	}

	return 0;
}

void ClientTaskState::RewardTask(Client *c, const TaskInformation *ti, ClientTaskInformation& client_task)
{
	if (!ti || !c || client_task.was_rewarded) {
		return;
	}

	client_task.was_rewarded = true;
	client_task.updated = true;

	if (!ti->completion_emote.empty()) {
		c->Message(Chat::Yellow, ti->completion_emote.c_str());
	}

	// TODO: this function should sometimes use QuestReward_Struct and CashReward_Struct
	// assumption is they use QuestReward_Struct when there is more than 1 thing getting rewarded
	if (ti->reward_method != METHODQUEST) {
		for (const auto &i: Strings::Split(ti->reward_id_list, "|")) {
			// handle charges
			int16  charges = -1;
			uint32 item_id = Strings::IsNumber(i) ? Strings::ToInt(i) : 0;
			if (Strings::Contains(i, ",")) {
				auto s = Strings::Split(i, ",");
				if (!s.empty() && s.size() == 2) {
					item_id = Strings::IsNumber(s[0]) ? Strings::ToInt(s[0]) : 0;
					charges = Strings::IsNumber(s[1]) ? Strings::ToInt(s[1]) : 0;
				}
			}

			if (item_id > 0) {
				std::unique_ptr<EQ::ItemInstance> inst(database.CreateItem(item_id, charges));
				if (inst && inst->GetItem()) {
					c->CheckItemDiscoverability(item_id);
					bool stacked = c->TryStacking(inst.get());
					if (!stacked) {
						int16_t slot = c->GetInv().FindFreeSlot(inst->IsClassBag(), true, inst->GetItem()->Size);
						c->SummonItem(item_id, charges, 0, 0, 0, 0, 0, 0, false, slot);
					}
					c->MessageString(Chat::Yellow, YOU_HAVE_BEEN_GIVEN, inst->GetItem()->Name);
				}
			}
		}
	}

	// just use normal NPC faction ID stuff
	if (ti->faction_reward && ti->faction_amount == 0) {
		zone->LoadNPCFaction(ti->faction_reward);
		c->SetFactionLevel(
			c->CharacterID(),
			ti->faction_reward,
			c->GetBaseClass(),
			c->GetBaseRace(),
			c->GetDeity()
		);
	} else if (ti->faction_reward != 0 && ti->faction_amount != 0) {
		// faction_reward is a faction ID
		zone->LoadFactionAssociation(ti->faction_reward);
		c->RewardFaction(
			ti->faction_reward,
			ti->faction_amount
		);
	}

	if (ti->cash_reward) {
		int platinum, gold, silver, copper;

		copper = ti->cash_reward;

		platinum = copper / 1000;
		copper   = copper - (platinum * 1000);
		gold     = copper / 100;
		copper   = copper - (gold * 100);
		silver   = copper / 10;
		copper   = copper - (silver * 10);

		c->CashReward(copper, silver, gold, platinum);
	}

	auto experience_reward = ti->experience_reward;
	if (experience_reward > 0) {
		c->AddEXP(ExpSource::Task, experience_reward);
	} else if (experience_reward < 0) {
		uint32 pos_reward = experience_reward * -1;
		// Minimal Level Based Exp reward Setting is 101 (1% exp at level 1)
		if (pos_reward > 100 && pos_reward < 25700) {
			uint8 max_level   = pos_reward / 100;
			uint8 exp_percent = pos_reward - (max_level * 100);
			c->AddLevelBasedExp(ExpSource::Task, exp_percent, max_level, RuleB(TaskSystem, ExpRewardsIgnoreLevelBasedEXPMods));
		}
	}

	if (ti->reward_points > 0) {
		if (ti->reward_point_type == static_cast<int32_t>(zone->GetCurrencyID(RADIANT_CRYSTAL))) {
			c->AddRadiantCrystals(ti->reward_points);
		} else if (ti->reward_point_type == static_cast<int32_t>(zone->GetCurrencyID(EBON_CRYSTAL))) {
			c->AddEbonCrystals(ti->reward_points);
		} else {
			for (const auto& ac : zone->AlternateCurrencies) {
				if (ti->reward_point_type == ac.id) {
					const EQ::ItemData *item = database.GetItem(ac.item_id);
					if (item) {
						c->AddAlternateCurrencyValue(ti->reward_point_type, ti->reward_points);
						c->Message(Chat::Yellow, fmt::format("You have received ({}) {}!", ti->reward_points, item->Name).c_str());
					}
				}
			}
		}
	}
}

bool ClientTaskState::IsTaskActive(int task_id)
{
	if (m_active_task.task_id == task_id) {
		return true;
	}

	if (m_active_shared_task.task_id == task_id) {
		return true;
	}

	if (m_active_task_count == 0 || task_id == 0) {
		return false;
	}

	for (auto &active_quest : m_active_quests) {
		if (active_quest.task_id == task_id) {
			return true;
		}
	}

	return false;
}

void ClientTaskState::FailTask(Client *client, int task_id)
{
	LogTasks(
		"Failing task for character [{}] task_id [{}] task_count [{}]",
		client->GetCleanName(),
		task_id,
		m_active_task_count
	);


	// type: Task
	if (m_active_task.task_id == task_id) {
		client->SendTaskFailed(task_id, TASKSLOTTASK, TaskType::Task);
		// Remove the task from the client
		client->CancelTask(TASKSLOTTASK, TaskType::Task);
		return;
	}

	// type: Shared Task (failed via world for all members)
	if (m_active_shared_task.task_id == task_id) {
		TaskManager::Instance()->EndSharedTask(*client, task_id, true);
		return;
	}

	if (m_active_task_count == 0) {
		return;
	}

	for (int i = 0; i < MAXACTIVEQUESTS; i++) {
		if (m_active_quests[i].task_id == task_id) {
			client->SendTaskFailed(m_active_quests[i].task_id, i, TaskType::Quest);
			// Remove the task from the client
			client->CancelTask(i, TaskType::Quest);
			return;
		}
	}
}

bool ClientTaskState::IsTaskActivityActive(int task_id, int activity_id)
{
	LogTasks("task_id [{}] activity_id [{}]", task_id, activity_id);

	// Quick sanity check
	if (activity_id < 0) {
		return false;
	}
	if (m_active_task_count == 0 && m_active_task.task_id == TASKSLOTEMPTY &&
		m_active_shared_task.task_id == TASKSLOTEMPTY) {
		return false;
	}

	int  active_task_index = -1;
	auto task_type         = TaskType::Task;
	if (m_active_task.task_id == task_id) {
		active_task_index = TASKSLOTTASK;
	}
	if (m_active_shared_task.task_id == task_id) {
		task_type         = TaskType::Shared;
		active_task_index = TASKSLOTSHAREDTASK;
	}

	if (active_task_index == -1) {
		for (int i = 0; i < MAXACTIVEQUESTS; i++) {
			if (m_active_quests[i].task_id == task_id) {
				active_task_index = i;
				task_type         = TaskType::Quest;
				break;
			}
		}
	}

	// The client does not have this task
	if (active_task_index == -1) {
		return false;
	}

	auto info = GetClientTaskInfo(task_type, active_task_index);

	if (info == nullptr) {
		return false;
	}

	const auto task_data = TaskManager::Instance()->GetTaskData(info->task_id);
	if (!task_data) {
		return false;
	}

	// The activity_id is out of range
	if (activity_id >= task_data->activity_count) {
		return false;
	}

	LogTasks(
		"(Update) task_id [{}] activity_id [{}] activity_state [{}]",
		task_id,
		activity_id,
		info->activity[activity_id].activity_state
	);

	return (info->activity[activity_id].activity_state == ActivityActive);
}

void ClientTaskState::UpdateTaskActivity(
	Client *client,
	int task_id,
	int activity_id,
	int count,
	bool ignore_quest_update /*= false*/)
{
	LogTasks(
		"[UpdateTaskActivity] Increment done count (pre) on UpdateTaskActivity task_id [{}] activity_id [{}] count [{}]",
		task_id,
		activity_id,
		count
	);

	// Quick sanity check
	if (activity_id < 0 || (m_active_task_count == 0 && m_active_task.task_id == TASKSLOTEMPTY &&
							m_active_shared_task.task_id == TASKSLOTEMPTY)) {
		return;
	}

	int  active_task_index = -1;
	auto type              = TaskType::Task;

	if (m_active_task.task_id == task_id) {
		active_task_index = TASKSLOTTASK;
	}
	if (m_active_shared_task.task_id == task_id) {
		type              = TaskType::Shared;
		active_task_index = TASKSLOTSHAREDTASK;
	}

	if (active_task_index == -1) {
		for (int i = 0; i < MAXACTIVEQUESTS; i++) {
			if (m_active_quests[i].task_id == task_id) {
				active_task_index = i;
				type              = TaskType::Quest;
				break;
			}
		}
	}

	// The client does not have this task
	if (active_task_index == -1) {
		return;
	}

	auto info = GetClientTaskInfo(type, active_task_index);
	if (info == nullptr) {
		return;
	}

	const auto task_data = TaskManager::Instance()->GetTaskData(info->task_id);
	if (!task_data) {
		return;
	}

	// The activity_id is out of range
	if (activity_id >= task_data->activity_count) {
		return;
	}

	// The activity_information is not currently active
	if (info->activity[activity_id].activity_state == ActivityHidden) {
		return;
	}

	LogTasks(
		"[UpdateTaskActivity] Increment done count (done) on UpdateTaskActivity task_id [{}] activity_id [{}] count [{}]",
		task_id,
		activity_id,
		count
	);

	IncrementDoneCount(client, task_data, active_task_index, activity_id, count, ignore_quest_update);
}

void ClientTaskState::ResetTaskActivity(Client *client, int task_id, int activity_id)
{
	LogTasks(
		"[ResetTaskActivity] (pre) client [{}] task_id [{}] activity_id [{}]",
		client->GetCleanName(),
		task_id,
		activity_id
	);

	// Quick sanity check
	if (activity_id < 0 || (m_active_task_count == 0 && m_active_task.task_id == TASKSLOTEMPTY &&
							m_active_shared_task.task_id == TASKSLOTEMPTY)) {
		return;
	}

	int  active_task_index = -1;
	auto type              = TaskType::Task;
	if (m_active_task.task_id == task_id) {
		active_task_index = TASKSLOTTASK;
	}
	if (m_active_shared_task.task_id == task_id) {
		type              = TaskType::Shared;
		active_task_index = TASKSLOTSHAREDTASK;
	}

	if (active_task_index == -1) {
		for (int i = 0; i < MAXACTIVEQUESTS; i++) {
			if (m_active_quests[i].task_id == task_id) {
				active_task_index = i;
				type              = TaskType::Quest;
				break;
			}
		}
	}

	// The client does not have this task
	if (active_task_index == -1) {
		return;
	}

	auto info = GetClientTaskInfo(type, active_task_index);
	if (info == nullptr) {
		return;
	}

	const auto task_data = TaskManager::Instance()->GetTaskData(info->task_id);
	if (!task_data) {
		return;
	}

	// The activity_id is out of range
	if (activity_id >= task_data->activity_count) {
		return;
	}

	// The activity_information is not currently active
	if (info->activity[activity_id].activity_state == ActivityHidden) {
		return;
	}

	LogTasks(
		"[ResetTaskActivity] (IncrementDoneCount) client [{}] task_id [{}] activity_id [{}]",
		client->GetCleanName(),
		task_id,
		activity_id
	);

	IncrementDoneCount(
		client,
		task_data,
		active_task_index,
		activity_id,
		(info->activity[activity_id].done_count * -1),
		false
	);
}

bool ClientTaskState::CompleteTask(Client *c, uint32 task_id)
{
	const auto task_data = TaskManager::Instance()->GetTaskData(task_id);
	if (!task_data) {
		return false;
	}

	for (
		int activity_id = 0;
		activity_id < TaskManager::Instance()->GetActivityCount(task_id);
		activity_id++
	) {
		c->UpdateTaskActivity(
			task_id,
			activity_id,
			task_data->activity_information[activity_id].goal_count
		);
	}

	return true;
}

bool ClientTaskState::UncompleteTask(int task_id)
{
	return std::erase_if(
		m_completed_tasks,
		[&](const CompletedTaskInformation& task) {
			return task.task_id == task_id;
		}
	);
}

void ClientTaskState::ShowClientTaskInfoMessage(ClientTaskInformation *task, Client *c)
{
	const auto task_data = TaskManager::Instance()->GetTaskData(task->task_id);

	c->Message(Chat::White, "------------------------------------------------");
	c->Message(
		Chat::White,
		fmt::format(
			"Task {} | Title: {} ID: {} Type: {} | {}",
			task->slot,
			task_data->title,
			task->task_id,
			Tasks::GetTaskTypeDescription(task_data->type),
			Saylink::Create(
				fmt::format(
					"#task complete {}",
					task->task_id
				),
				false,
				"Complete"
			)
		).c_str()
	);
	c->Message(Chat::White, "------------------------------------------------");
	c->Message(
		Chat::White,
		fmt::format(
			"Description | {}",
			task_data->description
		).c_str()
	);

	for (int activity_id = 0; activity_id < TaskManager::Instance()->GetActivityCount(task->task_id); activity_id++) {
		std::vector<std::string> update_increments = { "1", "5", "10", "20", "50" };
		std::vector<std::string> update_saylinks;

		for (auto &increment: update_increments) {
			auto task_update_saylink = Saylink::Silent(
				fmt::format(
					"#task update {} {} {}",
					task->task_id,
					task->activity[activity_id].activity_id,
					increment
				),
				increment
			);

			update_saylinks.push_back(task_update_saylink);
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Activity {} | Count: {} State: {} ({}) [{}]",
				task->activity[activity_id].activity_id,
				task->activity[activity_id].done_count,
				task->activity[activity_id].activity_state,
				Tasks::GetActivityStateDescription(task->activity[activity_id].activity_state),
				Strings::Implode(" | ", update_saylinks)
			).c_str()
		);
	}
}

void ClientTaskState::ShowClientTasks(Client* who, Client *to)
{
	to->SendChatLineBreak();

	to->Message(
		Chat::White,
		fmt::format(
			"Task Information for {}",
			to->GetTargetDescription(who, TargetDescriptionType::UCSelf)
		).c_str()
	);

	if (m_active_task.task_id != TASKSLOTEMPTY) {
		ShowClientTaskInfoMessage(&m_active_task, to);
	}

	if (m_active_shared_task.task_id != TASKSLOTEMPTY) {
		ShowClientTaskInfoMessage(&m_active_shared_task, to);
	}

	for (auto &active_quest : m_active_quests) {
		if (active_quest.task_id == TASKSLOTEMPTY) {
			continue;
		}

		ShowClientTaskInfoMessage(&active_quest, to);
	}

	to->SendChatLineBreak();
}

// TODO: Shared Task
int ClientTaskState::TaskTimeLeft(int task_id)
{

	// type "task"
	if (m_active_task.task_id == task_id) {
		int time_now = time(nullptr);

		const auto task_data = TaskManager::Instance()->GetTaskData(task_id);
		if (!task_data) {
			return -1;
		}

		if (!task_data->duration) {
			return -1;
		}

		int time_left = (m_active_task.accepted_time + task_data->duration - time_now);

		return (time_left > 0 ? time_left : 0);
	}

	// type "shared task"
	if (m_active_shared_task.task_id == task_id) {
		int time_now = time(nullptr);

		const auto task_data = TaskManager::Instance()->GetTaskData(task_id);
		if (!task_data) {
			return -1;
		}

		if (!task_data->duration) {
			return -1;
		}

		int time_left = (m_active_shared_task.accepted_time + task_data->duration - time_now);

		return (time_left > 0 ? time_left : 0);
	}

	if (m_active_task_count == 0) {
		return -1;
	}

	for (auto &active_quest : m_active_quests) {
		if (active_quest.task_id != task_id) {
			continue;
		}

		int time_now = time(nullptr);

		const auto task_data = TaskManager::Instance()->GetTaskData(active_quest.task_id);
		if (!task_data) {
			return -1;
		}

		if (!task_data->duration) {
			return -1;
		}

		int time_left = (active_quest.accepted_time + task_data->duration - time_now);

		// If Timeleft is negative, return 0, else return the number of seconds left

		return (time_left > 0 ? time_left : 0);
	}

	return -1;
}

bool ClientTaskState::IsTaskCompleted(int task_id, Client *c)
{
	if (!RuleB(TaskSystem, RecordCompletedTasks)) {
		return false;
	}

	for (const auto& e : m_completed_tasks) {
		LogTasks("Comparing completed task [{}] with [{}]", e.task_id, task_id);
		if (e.task_id == task_id) {
			return true;
		}
	}

	if (c) {
		for (auto &e: c->GetCompletedSharedTasks()) {
			if (e == task_id) {
				return true;
			}
		}
	}

	return false;
}

bool ClientTaskState::AreTasksCompleted(const std::vector<int>& task_ids)
{
	if (!RuleB(TaskSystem, RecordCompletedTasks)) {
		return false;
	}

	for (const auto& e : task_ids) {
		if (!IsTaskCompleted(e)) {
			return false;
		}
	}

	return true;
}

bool ClientTaskState::TaskOutOfTime(TaskType task_type, int index)
{
	// Returns true if the Task in the specified slot has a time limit that has been exceeded.
	auto info = GetClientTaskInfo(task_type, index);
	if (info == nullptr) {
		return false;
	}

	// make sure the task_id is at least maybe in our array
	if (info->task_id <= 0) {
		return false;
	}

	int time_now = time(nullptr);
	const auto task_data = TaskManager::Instance()->GetTaskData(info->task_id);
	if (!task_data) {
		return false;
	}

	return (task_data->duration && (info->accepted_time + task_data->duration <= time_now));
}

void ClientTaskState::TaskPeriodicChecks(Client *client)
{
	// shared task expiration is handled by world

	// type "task"
	if (m_active_task.task_id != TASKSLOTEMPTY) {
		if (TaskOutOfTime(TaskType::Task, TASKSLOTTASK)) {
			// Send Red Task Failed Message
			client->SendTaskFailed(m_active_task.task_id, TASKSLOTTASK, TaskType::Task);
			// Remove the task from the client
			client->CancelTask(TASKSLOTTASK, TaskType::Task);
			// It is a conscious decision to only fail one task per call to this method,
			// otherwise the player will not see all the failed messages where multiple
			// tasks fail at the same time.
			return;
		}
	}

	if (m_active_task_count == 0) {
		return;
	}

	// Check for tasks that have failed because they have not been completed in the specified time
	//
	for (int task_index = 0; task_index < MAXACTIVEQUESTS; task_index++) {
		if (m_active_quests[task_index].task_id == TASKSLOTEMPTY) {
			continue;
		}

		if (TaskOutOfTime(TaskType::Quest, task_index)) {
			// Send Red Task Failed Message
			client->SendTaskFailed(m_active_quests[task_index].task_id, task_index, TaskType::Quest);
			// Remove the task from the client
			client->CancelTask(task_index, TaskType::Quest);
			// It is a conscious decision to only fail one task per call to this method,
			// otherwise the player will not see all the failed messages where multiple
			// tasks fail at the same time.
			break;
		}
	}
}

bool ClientTaskState::IsTaskActivityCompleted(TaskType task_type, int index, int activity_id)
{
	switch (task_type) {
		case TaskType::Task:
			if (index != TASKSLOTTASK) {
				return false;
			}
			return m_active_task.activity[activity_id].activity_state == ActivityCompleted;
		case TaskType::Shared:
			if (index != TASKSLOTSHAREDTASK) {
				return false;
			}
			return m_active_shared_task.activity[activity_id].activity_state == ActivityCompleted;
		case TaskType::Quest:
			if (index < MAXACTIVEQUESTS) {
				return m_active_quests[index].activity[activity_id].activity_state == ActivityCompleted;
			}
		default:
			return false;
	}

}

// should we be defaulting to hidden?
ActivityState ClientTaskState::GetTaskActivityState(TaskType task_type, int index, int activity_id)
{
	ActivityState return_state = ActivityHidden;
	switch (task_type) {
		case TaskType::Task:
			if (index != TASKSLOTTASK) {
				return_state = ActivityHidden;
				break;
			}
			return_state = m_active_task.activity[activity_id].activity_state;
			break;
		case TaskType::Shared:
			if (index != TASKSLOTSHAREDTASK) {
				return_state = ActivityHidden;
				break;
			}
			return_state = m_active_shared_task.activity[activity_id].activity_state;
			break;
		case TaskType::Quest:
			if (index < MAXACTIVEQUESTS) {
				return_state = m_active_quests[index].activity[activity_id].activity_state;
				break;
			}
			break;
		default:
			return_state = ActivityHidden;
	}

	LogTasksDetail(
		"-- task_type [{}] ({}) index [{}] activity_id [{}] activity_state [{}] ({})",
		Tasks::GetTaskTypeIdentifier(task_type),
		Tasks::GetTaskTypeDescription(task_type),
		index,
		activity_id,
		Tasks::GetActivityStateIdentifier(return_state),
		Tasks::GetActivityStateDescription(return_state)
	);

	return return_state;
}

int ClientTaskState::GetTaskActivityDoneCount(TaskType task_type, int index, int activity_id)
{
	switch (task_type) {
		case TaskType::Task:
			if (index != TASKSLOTTASK) {
				return 0;
			}
			return m_active_task.activity[activity_id].done_count;
		case TaskType::Shared:
			if (index != TASKSLOTSHAREDTASK) {
				return 0;
			}
			return m_active_shared_task.activity[activity_id].done_count;
		case TaskType::Quest:
			if (index < MAXACTIVEQUESTS) {
				return m_active_quests[index].activity[activity_id].done_count;
			}
		default:
			return 0;
	}
}

int ClientTaskState::GetTaskActivityDoneCountFromTaskID(int task_id, int activity_id)
{

	// type "task"
	if (m_active_task.task_id == task_id) {
		return m_active_task.activity[activity_id].done_count;
	}

	// type "shared"
	if (m_active_shared_task.task_id == task_id) {
		return m_active_shared_task.activity[activity_id].done_count;
	}

	int active_task_index = -1;

	for (int task_index = 0; task_index < MAXACTIVEQUESTS; task_index++) {
		if (m_active_quests[task_index].task_id == task_id) {
			active_task_index = task_index;
			break;
		}
	}

	if (active_task_index == -1) {
		return 0;
	}

	if (m_active_quests[active_task_index].activity[activity_id].done_count) {
		return m_active_quests[active_task_index].activity[activity_id].done_count;
	}
	else {
		return 0;
	}
}

int ClientTaskState::GetTaskStartTime(TaskType task_type, int index)
{
	switch (task_type) {
		case TaskType::Task:
			return m_active_task.accepted_time;
		case TaskType::Quest:
			return m_active_quests[index].accepted_time;
		case TaskType::Shared:
			return m_active_shared_task.accepted_time;
		default:
			return -1;
	}
}

void ClientTaskState::CancelAllTasks(Client *client)
{

	// This method exists solely to be called during #task reloadall
	// It removes tasks from the in-game client state ready for them to be
	// resent to the client, in case an updated task fails to load

	// task
	// these cancels lock up the client for some reason
	CancelTask(client, TASKSLOTTASK, TaskType::Task, false);
	m_active_task.task_id = TASKSLOTEMPTY;

	// shared task
	client->m_requested_shared_task_removal = true;
	CancelTask(client, TASKSLOTSHAREDTASK, TaskType::Shared, false);
	client->m_requested_shared_task_removal = false;
	m_active_shared_task.task_id = TASKSLOTEMPTY;

	// "quests"
	for (int task_index = 0; task_index < MAXACTIVEQUESTS; task_index++)
		if (m_active_quests[task_index].task_id != TASKSLOTEMPTY) {
			CancelTask(client, task_index, TaskType::Quest, false);
			m_active_quests[task_index].task_id = TASKSLOTEMPTY;
		}
}

void ClientTaskState::CancelTask(Client *c, int sequence_number, TaskType task_type, bool remove_from_db)
{
	LogTasks("CancelTask");

	// shared task middleware
	// intercept and pass to world first before processing normally
	if (!c->m_requested_shared_task_removal && task_type == TaskType::Shared && m_active_shared_task.task_id != 0) {

		// struct
		auto pack = new ServerPacket(ServerOP_SharedTaskQuit, sizeof(ServerSharedTaskQuit_Struct));
		auto *r   = (ServerSharedTaskQuit_Struct *) pack->pBuffer;

		// fill
		r->requested_character_id = c->CharacterID();
		r->task_id                = m_active_shared_task.task_id;
		r->remove_from_db         = remove_from_db;

		// send
		worldserver.SendPacket(pack);
		safe_delete(pack);

		return;
	}

	// packet
	auto outapp = new EQApplicationPacket(OP_CancelTask, sizeof(CancelTask_Struct));

	// fill
	auto *cts = (CancelTask_Struct *) outapp->pBuffer;
	cts->SequenceNumber = sequence_number;
	cts->type           = static_cast<uint32>(task_type);

	// send
	c->QueuePacket(outapp);
	safe_delete(outapp);

	// persistence
	if (remove_from_db) {
		RemoveTask(c, sequence_number, task_type);
		m_has_explore_task = HasExploreTask(c);
	}
}

void ClientTaskState::KickPlayersSharedTask(Client* client)
{
	uint32_t pack_size = sizeof(ServerSharedTaskKickPlayers_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_SharedTaskKickPlayers, pack_size);
	auto buf = reinterpret_cast<ServerSharedTaskKickPlayers_Struct*>(pack->pBuffer);

	buf->source_character_id = client->CharacterID();
	buf->task_id             = m_active_shared_task.task_id;

	worldserver.SendPacket(pack.get());
}

void ClientTaskState::RemoveTask(Client *client, int sequence_number, TaskType task_type)
{
	int character_id = client->CharacterID();
	Log(Logs::General, Logs::Tasks, "ClientTaskState Cancel Task %i ", sequence_number);

	int task_id = -1;
	switch (task_type) {
		case TaskType::Task:
			if (sequence_number == TASKSLOTTASK) {
				task_id = m_active_task.task_id;
			}
			break;
		case TaskType::Quest:
			if (sequence_number < MAXACTIVEQUESTS) {
				task_id = m_active_quests[sequence_number].task_id;
			}
			break;
		case TaskType::Shared:
			if (sequence_number == TASKSLOTSHAREDTASK) {
				task_id = m_active_shared_task.task_id;
			}
			break;
		default:
			break;
	}

	CharacterActivitiesRepository::DeleteWhere(
		database,
		fmt::format("charid = {} AND taskid = {}", character_id, task_id)
	);

	CharacterTasksRepository::DeleteWhere(
		database,
		fmt::format("charid = {} AND taskid = {} AND type = {}", character_id, task_id, static_cast<int>(task_type))
	);

	switch (task_type) {
		case TaskType::Task:
			m_active_task.task_id = TASKSLOTEMPTY;
			break;
		case TaskType::Shared:
			m_active_shared_task.task_id = TASKSLOTEMPTY;
			break;
		case TaskType::Quest:
			m_active_quests[sequence_number].task_id = TASKSLOTEMPTY;
			m_active_task_count--;
			break;
		default:
			break;
	}
}

void ClientTaskState::RemoveTaskByTaskID(Client *client, uint32 task_id)
{
	switch (TaskManager::Instance()->GetTaskType(task_id)) {
		case TaskType::Task: {
			if (m_active_task.task_id == task_id) {
				LogTasks("RemoveTaskByTaskID found Task [{}]", task_id);
				CancelTask(client, TASKSLOTTASK, TaskType::Task, true);
			}
			break;
		}
		case TaskType::Shared: {
			if (m_active_shared_task.task_id == task_id) {
				LogTasks("RemoveTaskByTaskID found Shared Task [{}]", task_id);
				CancelTask(client, TASKSLOTSHAREDTASK, TaskType::Shared, true);
			}
			break;
		}
		case TaskType::Quest: {
			for (int active_quest = 0; active_quest < MAXACTIVEQUESTS; active_quest++) {
				if (m_active_quests[active_quest].task_id == task_id) {
					LogTasks("RemoveTaskByTaskID found Quest [{}] at index [{}]", task_id, active_quest);
					CancelTask(client, active_quest, TaskType::Quest, true);
				}
			}
		}
		default: {
			break;
		}
	}
}

void ClientTaskState::AcceptNewTask(
	Client *client,
	int task_id,
	int npc_entity_id,
	time_t accept_time,
	bool enforce_level_requirement
)
{
	if (task_id < 0) {
		client->Message(Chat::Red, "Task system not functioning, or task_id %i out of range.", task_id);
		return;
	}

	const auto task = TaskManager::Instance()->GetTaskData(task_id);
	if (!task) {
		client->Message(Chat::Red, "Invalid task_id %i", task_id);
		return;
	}

	// shared task
	// intercept and pass to world first before processing normally
	if (!client->m_requesting_shared_task && task->type == TaskType::Shared) {
		LogTasksDetail(
			"Initiating shared_task request | task_id [{}] character_id [{}] name [{}]",
			task_id,
			client->CharacterID(),
			client->GetCleanName()
		);

		// struct
		auto pack = new ServerPacket(ServerOP_SharedTaskRequest, sizeof(ServerSharedTaskRequest_Struct));
		auto *r   = (ServerSharedTaskRequest_Struct *) pack->pBuffer;

		// fill
		r->requested_character_id = client->CharacterID();
		r->requested_task_id      = task_id;
		r->requested_npc_entity_id  = npc_entity_id;

		// send
		worldserver.SendPacket(pack);
		safe_delete(pack);

		return;
	}

	bool max_tasks = false;

	switch (task->type) {
		case TaskType::Task:
			if (m_active_task.task_id != TASKSLOTEMPTY) {
				max_tasks = true;
			}
			break;
		case TaskType::Shared: // TODO: shared tasks
			if (m_active_shared_task.task_id != TASKSLOTEMPTY) {
				max_tasks = true;
			}
			break;
		case TaskType::Quest:
			if (m_active_task_count == MAXACTIVEQUESTS) {
				max_tasks = true;
			}
			break;
		default:
			break;
	}

	if (max_tasks) {
		client->MessageString(Chat::Yellow, MAX_ACTIVE_TASKS, ".", ".", client->GetName());
		return;
	}

	// only Quests can have more than one, so don't need to check others
	if (task->type == TaskType::Quest) {
		for (auto &active_quest : m_active_quests) {
			if (active_quest.task_id == task_id) {
				// live doesn't have an eqstr for it but this seems to be the string used for this scenario
				client->Message(Chat::Yellow, "You are already working on a task for this person, you must finish it before asking for another.");
				return;
			}
		}
	}

	if (enforce_level_requirement && !TaskManager::Instance()->ValidateLevel(task_id, client->GetLevel())) {
		client->MessageString(Chat::Yellow, TASK_NOT_RIGHT_LEVEL);
		return;
	}

	if (!TaskManager::Instance()->IsTaskRepeatable(task_id) && IsTaskCompleted(task_id)) {
		return;
	}

	// solo task timer lockout validation
	if (task->type != TaskType::Shared)
	{
		auto task_timers = CharacterTaskTimersRepository::GetWhere(database, fmt::format(
			SQL(
				character_id = {}
				AND (task_id = {}
					OR (timer_group > 0 AND timer_type = {} AND timer_group = {})
					OR (timer_group > 0 AND timer_type = {} AND timer_group = {}))
				AND expire_time > NOW() ORDER BY timer_type ASC LIMIT 1
			),
			client->CharacterID(),
			task_id,
			static_cast<int>(TaskTimerType::Replay),
			task->replay_timer_group,
			static_cast<int>(TaskTimerType::Request),
			task->request_timer_group
		));

		if (!task_timers.empty())
		{
			auto timer_type = static_cast<TaskTimerType>(task_timers.front().timer_type);
			auto seconds = task_timers.front().expire_time - std::time(nullptr);
			auto days  = fmt::format_int(seconds / 86400).str();
			auto hours = fmt::format_int((seconds / 3600) % 24).str();
			auto mins  = fmt::format_int((seconds / 60) % 60).str();

			// these solo task messages are in SharedTaskMessage for convenience
			if (timer_type == TaskTimerType::Replay)
			{
				client->MessageString(Chat::Red, TaskStr::ASSIGN_REPLAY_TIMER, days.c_str(), hours.c_str(), mins.c_str());
			}
			else if (timer_type == TaskTimerType::Request)
			{
				auto eqstr = TaskStr::Get(TaskStr::ASSIGN_REQUEST_TIMER);
				client->Message(Chat::Red, fmt::format(fmt::runtime(eqstr), days, hours, mins).c_str());
			}

			return;
		}
	}

	// We do it this way, because when the Client cancels a task, it retains the sequence number of the remaining
	// tasks in it's window, until something causes the TaskDescription packets to be sent again. We could just
	// resend all the active task data to the client when it cancels a task, but that could be construed as a
	// waste of bandwidth.
	//
	ClientTaskInformation *active_slot = nullptr;
	switch (task->type) {
		case TaskType::Task:
			active_slot = &m_active_task;
			break;

		case TaskType::Shared:
			active_slot = &m_active_shared_task;
			break;

		case TaskType::Quest:
			for (int task_index = 0; task_index < MAXACTIVEQUESTS; task_index++) {
				Log(Logs::General, Logs::Tasks,
					"ClientTaskState Looking for free slot in slot %i, found task_id of %i", task_index,
					m_active_quests[task_index].task_id);
				if (m_active_quests[task_index].task_id == 0) {
					active_slot = &m_active_quests[task_index];
					break;
				}
			}
			break;

		default:
			break;
	}

	// This shouldn't happen unless there is a bug in the handling of ActiveTaskCount somewhere
	if (active_slot == nullptr) {
		client->MessageString(Chat::Yellow, MAX_ACTIVE_TASKS, ".", ".", client->GetName());
		return;
	}

	active_slot->task_id       = task_id;
	active_slot->accepted_time = static_cast<int>(accept_time);
	active_slot->updated       = true;
	active_slot->was_rewarded  = false;

	for (int activity_id = 0; activity_id < task->activity_count; activity_id++) {
		active_slot->activity[activity_id].activity_id    = activity_id;
		active_slot->activity[activity_id].done_count     = 0;
		active_slot->activity[activity_id].activity_state = ActivityHidden;
		active_slot->activity[activity_id].updated        = true;
	}

	UnlockActivities(client, *active_slot);

	if (task->type == TaskType::Quest) {
		m_active_task_count++;
	}

	// add request timer (shared task timers are added to members by world)
	if (task->request_timer_seconds > 0)
	{
		auto expire_time = active_slot->accepted_time + task->request_timer_seconds;

		auto seconds = expire_time - std::time(nullptr);
		if (seconds > 0) // not already expired
		{
			if (task->type != TaskType::Shared)
			{
				auto timer = CharacterTaskTimersRepository::NewEntity();
				timer.character_id = client->CharacterID();
				timer.task_id      = task_id;
				timer.timer_type   = static_cast<int>(TaskTimerType::Request);
				timer.timer_group  = task->request_timer_group;
				timer.expire_time  = expire_time;

				CharacterTaskTimersRepository::InsertOne(database, timer);
			}

			client->Message(Chat::Yellow, fmt::format(
				TaskStr::Get(TaskStr::RECEIVED_REQUEST_TIMER),
				task->title,
				fmt::format_int(seconds / 86400).c_str(),       // days
				fmt::format_int((seconds / 3600) % 24).c_str(), // hours
				fmt::format_int((seconds / 60) % 60).c_str()    // minutes
			).c_str());
		}
	}

	TaskManager::Instance()->SendSingleActiveTaskToClient(client, *active_slot, false, true);
	client->StartTaskRequestCooldownTimer();
	client->MessageString(Chat::DefaultText, YOU_ASSIGNED_TASK, task->title.c_str());

	TaskManager::Instance()->SaveClientState(client, this);

	NPC *npc = entity_list.GetNPCByID(npc_entity_id);
	if (npc) {
		if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::TASK_ACCEPT)) {
			auto e = PlayerEvent::TaskAcceptEvent{
				.npc_id = npc->GetNPCTypeID(),
				.npc_name = npc->GetCleanName(),
				.task_id = static_cast<uint32>(task_id),
				.task_name = TaskManager::Instance()->GetTaskName(static_cast<uint32>(task_id)),
			};
			RecordPlayerEventLogWithClient(client, PlayerEvent::TASK_ACCEPT, e);
		}

		if (parse->HasQuestSub(npc->GetNPCTypeID(), EVENT_TASK_ACCEPTED)) {
			parse->EventNPC(EVENT_TASK_ACCEPTED, npc, client, std::to_string(task_id), 0);
		}
	} else {
		if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::TASK_ACCEPT)) {
			auto e = PlayerEvent::TaskAcceptEvent{
				.npc_id = 0,
				.npc_name = "No NPC",
				.task_id = static_cast<uint32>(task_id),
				.task_name = TaskManager::Instance()->GetTaskName(static_cast<uint32>(task_id)),
			};
			RecordPlayerEventLogWithClient(client, PlayerEvent::TASK_ACCEPT, e);
		}
	}

	if (parse->PlayerHasQuestSub(EVENT_TASK_ACCEPTED)) {
		parse->EventPlayer(EVENT_TASK_ACCEPTED, client, std::to_string(task_id), 0);
	}
}

void ClientTaskState::ProcessTaskProximities(Client *client, float x, float y, float z)
{
	if (!m_has_explore_task) {
		return;
	}

	float last_x = client->ProximityX();
	float last_y = client->ProximityY();
	float last_z = client->ProximityZ();

	if ((last_x == x) && (last_y == y) && (last_z == z)) {
		return;
	}

	UpdateTasksOnExplore(client, glm::vec4(x, y, z, 0.0f));
}

void ClientTaskState::SharedTaskIncrementDoneCount(
	Client *client,
	int task_id,
	int activity_id,
	int done_count,
	bool ignore_quest_update
)
{
	const auto t = TaskManager::Instance()->GetTaskData(task_id);

	auto info = GetClientTaskInfo(t->type, TASKSLOTSHAREDTASK);
	if (info == nullptr) {
		return;
	}

	// absolute value update
	info->activity[activity_id].done_count = done_count;

	LogTasksDetail(
		"Setting task_id [{}] to absolute done_count value of [{}] via increment [{}]",
		task_id,
		info->activity[activity_id].done_count,
		done_count
	);

	IncrementDoneCount(
		client,
		t,
		TASKSLOTSHAREDTASK,
		activity_id,
		0, // no op
		ignore_quest_update
	);
}

const ClientTaskInformation &ClientTaskState::GetActiveSharedTask() const
{
	return m_active_shared_task;
}

bool ClientTaskState::HasActiveSharedTask()
{
	return GetActiveSharedTask().task_id != 0;
}

void ClientTaskState::CreateTaskDynamicZone(Client* client, int task_id, DynamicZone& dz_request)
{
	const auto task = TaskManager::Instance()->GetTaskData(task_id);
	if (!task)
	{
		return;
	}

	// dz should be named the version-based zone name (used in choose zone window and dz window on live)
	auto zone_info = ZoneStore::Instance()->GetZoneWithFallback(dz_request.GetZoneID(), dz_request.GetZoneVersion());
	dz_request.SetName(zone_info && !zone_info->long_name.empty() ? zone_info->long_name : task->title);
	dz_request.SetMinPlayers(task->min_players);
	dz_request.SetMaxPlayers(task->max_players);

	// a task might create a dz from an element so override dz duration to time remaining
	std::chrono::seconds seconds(TaskTimeLeft(task_id));
	if (task->duration == 0 || seconds.count() < 0)
	{
		// cap unlimited duration tasks so instance isn't held indefinitely
		// expected behavior is to re-acquire any unlimited tasks that have an expired dz
		seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::hours(24));
	}

	dz_request.SetDuration(static_cast<uint32_t>(seconds.count()));

	if (task->type == TaskType::Shared)
	{
		dz_request.SetType(DynamicZoneType::Mission);

		// shared task missions are created in world
		std::ostringstream ss = dz_request.GetSerialized();
		std::string_view sv = ss.view();

		auto pack_size = sizeof(ServerSharedTaskCreateDynamicZone_Struct) + sv.size();
		auto pack = std::make_unique<ServerPacket>(ServerOP_SharedTaskCreateDynamicZone, static_cast<uint32_t>(pack_size));
		auto buf = reinterpret_cast<ServerSharedTaskCreateDynamicZone_Struct*>(pack->pBuffer);
		buf->source_character_id = client->CharacterID();
		buf->task_id = task_id;
		buf->cereal_size = static_cast<uint32_t>(sv.size());
		memcpy(buf->cereal_data, sv.data(), sv.size());

		worldserver.SendPacket(pack.get());
	}

}

void ClientTaskState::ListTaskTimers(Client* client)
{
	LogTasksDetail("Client [{}]", client->GetCleanName());

	// this isn't live-like but we need to throttle query (alternative is caching timers)
	if (!client->m_list_task_timers_rate_limit.Check()) {
		client->Message(Chat::White, "Sending messages too fast");
		return;
	}

	auto character_task_timers = CharacterTaskTimersRepository::GetWhere(
		database, fmt::format(
			"character_id = {} AND expire_time > NOW()",
			client->CharacterID()
		)
	);

	for (const auto& task_timer : character_task_timers)
	{
		const auto task = TaskManager::Instance()->GetTaskData(task_timer.task_id);
		if (task)
		{
			auto timer_type = static_cast<TaskTimerType>(task_timer.timer_type);
			auto seconds = task_timer.expire_time - std::time(nullptr);
			auto days  = fmt::format_int(seconds / 86400).str();
			auto hours = fmt::format_int((seconds / 3600) % 24).str();
			auto mins  = fmt::format_int((seconds / 60) % 60).str();

			if (timer_type == TaskTimerType::Replay)
			{
				client->MessageString(Chat::Yellow, TaskStr::REPLAY_TIMER_REMAINING,
					task->title.c_str(), days.c_str(), hours.c_str(), mins.c_str());
			}
			else
			{
				auto eqstr = TaskStr::Get(TaskStr::REQUEST_TIMER_REMAINING);
				client->Message(Chat::Yellow, fmt::format(fmt::runtime(eqstr), task->title, days, hours, mins).c_str());
			}
		}
	}

	if (character_task_timers.empty()) {
		client->MessageString(Chat::Yellow, TaskStr::NO_REPLAY_TIMERS);
	}
}

void ClientTaskState::AddReplayTimer(Client* client, ClientTaskInformation& client_task, const TaskInformation& task)
{
	if (task.replay_timer_seconds > 0)
	{
		// solo task replay timers are based on completion time
		auto expire_time = std::time(nullptr) + task.replay_timer_seconds;

		auto timer = CharacterTaskTimersRepository::NewEntity();
		timer.character_id = client->CharacterID();
		timer.task_id      = client_task.task_id;
		timer.timer_type   = static_cast<int>(TaskTimerType::Replay);
		timer.timer_group  = task.replay_timer_group;
		timer.expire_time  = expire_time;

		// replace any existing replay timer
		CharacterTaskTimersRepository::DeleteWhere(database, fmt::format(
			"(task_id = {} OR (timer_group > 0 AND timer_group = {})) AND timer_type = {} AND character_id = {}",
			client_task.task_id,
			task.replay_timer_group,
			static_cast<int>(TaskTimerType::Replay),
			client->CharacterID()));

		CharacterTaskTimersRepository::InsertOne(database, timer);

		client->Message(Chat::Yellow, fmt::format(
			TaskStr::Get(TaskStr::RECEIVED_REPLAY_TIMER),
			task.title,
			fmt::format_int(task.replay_timer_seconds / 86400).c_str(),       // days
			fmt::format_int((task.replay_timer_seconds / 3600) % 24).c_str(), // hours
			fmt::format_int((task.replay_timer_seconds / 60) % 60).c_str()    // minutes
		).c_str());
	}
}

// Sync zone client donecount state
//
// World is always authoritative, but we still need to keep zone state in sync with reality and in this case we need
// to update zone since world hasn't had a chance to let clients at the zone level know it's ok to progress to the next
// donecount
//
// If we send updates too fast and world has not had a chance to confirm and then inform clients to set their
// absolute state we will miss and discard updates because each update sets the same donecount
//
// Example: say we have a 100 kill task and we kill 10 mobs in an AOE, world gets 10 updates at once but they are
// all from the same donecount so world only confirms 1 was actually killed and the task updates only 1 which is not
// intended behavior.
//
// In this case we need to ensure that clients at the zone level increment internally even if they aren't
// necessarily confirmed by world yet because any of them could inform world of the next donecount so we need to sync
// zone-level before sending updates to world
void ClientTaskState::SyncSharedTaskZoneClientDoneCountState(
	Client *p_client,
	TaskType type,
	int task_index,
	int activity_id,
	uint32 done_count
)
{
	for (auto &e : entity_list.GetClientList()) {
		auto c = e.second;
		if (c->GetSharedTaskId() == p_client->GetSharedTaskId()) {
			auto t = c->GetTaskState()->GetClientTaskInfo(type, task_index);
			if (t == nullptr) {
				continue;
			}

			LogTasksDetail(
				"Setting internally client [{}] to donecount [{}]",
				c->GetCleanName(),
				done_count
			);

			t->activity[activity_id].done_count = (int) done_count;
		}
	}
}

bool ClientTaskState::HasActiveTasks()
{
	if (m_active_task.task_id != TASKSLOTEMPTY) {
		return true;
	}

	if (m_active_shared_task.task_id != TASKSLOTEMPTY) {
		return true;
	}

	bool has_active_quest = false;
	for (auto &active_quest : m_active_quests) {
		if (active_quest.task_id == TASKSLOTEMPTY) {
			continue;
		}

		return true;
	}

	return false;
}

void ClientTaskState::LockSharedTask(Client* client, bool lock)
{
	if (m_active_shared_task.task_id != TASKSLOTEMPTY)
	{
		ServerPacket pack(ServerOP_SharedTaskLock, sizeof(ServerSharedTaskLock_Struct));
		auto buf = reinterpret_cast<ServerSharedTaskLock_Struct*>(pack.pBuffer);
		buf->source_character_id = client->CharacterID();
		buf->task_id = m_active_shared_task.task_id;
		buf->lock = lock;

		worldserver.SendPacket(&pack);
	}
}

void ClientTaskState::EndSharedTask(Client* client, bool send_fail)
{
	if (m_active_shared_task.task_id != TASKSLOTEMPTY)
	{
		TaskManager::Instance()->EndSharedTask(*client, m_active_shared_task.task_id, send_fail);
	}
}
bool ClientTaskState::CanAcceptNewTask(Client* client, int task_id, int npc_entity_id) const
{
	auto it = std::find_if(m_last_offers.begin(), m_last_offers.end(),
		[&](const TaskOffer& offer) { return offer.task_id == task_id; });

	if (it == m_last_offers.end())
	{
		LogTasks("Client [{}] accepted unoffered task [{}]", client->GetName(), task_id);
		return false;
	}

	if (npc_entity_id != it->npc_entity_id)
	{
		LogTasks("Client [{}] accepted task [{}] with wrong npc entity id [{}] vs offered [{}]",
			client->GetName(), task_id, npc_entity_id, it->npc_entity_id);
		return false;
	}

	NPC* npc = entity_list.GetID(it->npc_entity_id)->CastToNPC();
	if (!npc) // client window disappears in this case
	{
		LogTasks("Client [{}] accepted task [{}] from missing npc", client->GetName(), task_id);
		return false;
	}

	auto dist = npc->CalculateDistance(client->GetX(), client->GetY(), client->GetZ());
	if (dist > MAX_TASK_SELECT_DISTANCE)
	{
		LogTasks("Client [{}] accepted task [{}] from npc [{}] out of range [{}]",
			client->GetName(), task_id, npc->GetCleanName(), dist);
		return false;
	}

	return true;
}
