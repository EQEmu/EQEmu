#pragma once

#include "common/repositories/character_tasks_repository.h"
#include "common/types.h"
#include "zone/task_client_state.h"
#include "zone/tasks.h"

#include <algorithm>
#include <list>
#include <string>
#include <vector>

class Client;
class Mob;
class SharedTaskRequest;

class TaskManager {

public:
	int GetActivityCount(int task_id);
	bool LoadTasks(int single_task = 0);
	bool LoadTaskSets();
	bool LoadClientState(Client *client, ClientTaskState *cts);
	bool SaveClientState(Client *client, ClientTaskState *cts);
	void SendTaskSelector(Client* client, Mob* mob, const std::vector<int>& tasks);
	bool ValidateLevel(int task_id, int player_level);
	std::string GetTaskName(uint32 task_id);
	TaskType GetTaskType(uint32 task_id);
	void TaskSetSelector(Client* client, Mob* mob, int task_set_id, bool ignore_cooldown);
	// task list provided by QuestManager (perl/lua)
	void TaskQuestSetSelector(Client* client, Mob* mob, const std::vector<int>& tasks, bool ignore_cooldown);
	void SharedTaskSelector(Client* client, Mob* mob, const std::vector<int>& tasks, bool ignore_cooldown);
	void SendActiveTasksToClient(Client *client, bool task_complete = false);
	void SendSingleActiveTaskToClient(
		Client *client,
		ClientTaskInformation &task_info,
		bool task_complete,
		bool bring_up_task_journal = false
	);
	void SendTaskActivityShort(Client *client, int task_id, int activity_id, int client_task_index);
	void SendTaskActivityLong(
		Client *client,
		int task_id,
		int activity_id,
		int client_task_index,
		bool task_complete = false
	);
	void SendCompletedTasksToClient(Client *c, ClientTaskState *cts);
	int FirstTaskInSet(int task_set);
	int LastTaskInSet(int task_set);
	int NextTaskInSet(int task_set, int task_id);
	bool IsTaskRepeatable(int task_id);
	bool IsActiveTaskComplete(ClientTaskInformation& client_task);
	int GetCurrentDzTaskID();
	void EndCurrentDzTask(bool send_fail);
	void EndSharedTask(Client& client, int task_id, bool send_fail);
	void EndSharedTask(uint32_t dz_id, bool send_fail);

	friend class ClientTaskState;

	// shared tasks
	void SyncClientSharedTaskState(Client *c, ClientTaskState *cts);

	void HandleUpdateTasksOnKill(Client* client, NPC* npc);

	const std::unordered_map<uint32_t, TaskInformation>& GetTaskData() const { return m_task_data; }
	TaskInformation* GetTaskData(int task_id)
	{
		auto it = m_task_data.find(task_id);
		return it != m_task_data.end() ? &it->second : nullptr;
	}

	static TaskManager* Instance()
	{
		static TaskManager instance;
		return &instance;
	}

private:
	std::vector<int>                              m_task_sets[MAXTASKSETS];
	std::unordered_map<uint32_t, TaskInformation> m_task_data;
	void SendActiveTaskDescription(
		Client *client,
		int task_id,
		ClientTaskInformation &task_info,
		int start_time,
		int duration,
		bool bring_up_task_journal = false
	);

	void SendActiveTaskToClient(ClientTaskInformation *task, Client *client, int task_index, bool task_complete);

	// shared tasks
	bool CanOfferSharedTask(int task_id, const SharedTaskRequest& request);
	void SyncClientSharedTaskWithPersistedState(Client *c, ClientTaskState *cts);
	void SyncClientSharedTaskRemoveLocalIfNotExists(Client *c, ClientTaskState *cts);
	void SendSharedTaskSelector(Client* client, Mob* mob, const std::vector<int>& tasks);
	void SyncClientSharedTaskStateToLocal(Client *c);
};
