#pragma once

#include "common/types.h"
#include "zone/tasks.h"

#include <algorithm>
#include <list>
#include <string>
#include <vector>

constexpr float MAX_TASK_SELECT_DISTANCE = 60.0f; // client closes window at this distance

struct TaskOffer
{
	int task_id;
	uint16_t npc_entity_id;
};

struct TaskUpdateFilter
{
	int task_id           = 0;
	int dz_switch_id      = 0;
	uint32_t item_id      = 0;
	glm::vec4 pos;
	bool use_pos          = false; // if true uses pos instead of client position for area filters
	bool ignore_area      = false; // if true, area check is disabled
	Mob* mob              = nullptr;
	Client* exp_client    = nullptr; // used by Kill tasks to filter shared task updates
	TaskActivityType type = TaskActivityType::None;
	TaskMethodType method = TaskMethodType::METHODSINGLEID;
};

class ClientTaskState {

public:
	ClientTaskState();
	~ClientTaskState();
	void ShowClientTasks(Client* who, Client *to);
	inline int GetActiveTaskCount() { return m_active_task_count; }
	int GetActiveTaskID(int index);
	bool IsTaskActivityCompleted(TaskType task_type, int index, int activity_id);
	int GetTaskActivityDoneCount(TaskType task_type, int index, int activity_id);
	int GetTaskActivityDoneCountFromTaskID(int task_id, int activity_id);
	int GetTaskStartTime(TaskType task_type, int index);
	void AcceptNewTask(Client *client, int task_id, int npc_entity_id, time_t accept_time, bool enforce_level_requirement = false);
	void FailTask(Client *client, int task_id);
	int TaskTimeLeft(int task_id);
	bool IsTaskCompleted(int task_id, Client *c = nullptr);
	bool AreTasksCompleted(const std::vector<int>& task_ids);
	bool IsTaskActive(int task_id);
	bool IsTaskActivityActive(int task_id, int activity_id);
	ActivityState GetTaskActivityState(TaskType task_type, int index, int activity_id);
	void UpdateTaskActivity(Client *client, int task_id, int activity_id, int count, bool ignore_quest_update = false);
	void ResetTaskActivity(Client *client, int task_id, int activity_id);
	void CancelTask(Client *c, int sequence_number, TaskType task_type, bool remove_from_db = true);
	void CancelAllTasks(Client *client);
	void RemoveTask(Client *client, int sequence_number, TaskType task_type);
	void RemoveTaskByTaskID(Client *client, uint32 task_id);
	bool UpdateTasksByNPC(Client* client, TaskActivityType type, NPC* npc);
	void UpdateTasksForItem(Client* client, TaskActivityType type, int item_id, int count = 1);
	void UpdateTasksOnLoot(Client* client, Corpse* corpse, int item_id, int count = 1);
	void UpdateTasksOnExplore(Client* client, const glm::vec4& loc);
	bool UpdateTasksOnSpeakWith(Client* client, NPC* npc);
	bool UpdateTasksOnDeliver(Client* client, std::vector<EQ::ItemInstance*>& items, Trade& trade, NPC* npc);
	void UpdateTasksOnTouch(Client *client, int dz_switch_id);
	void ProcessTaskProximities(Client *client, float x, float y, float z);
	bool TaskOutOfTime(TaskType task_type, int index);
	void TaskPeriodicChecks(Client *client);
	void SendTaskHistory(Client *client, int task_index);
	void RewardTask(Client* c, const TaskInformation* ti, ClientTaskInformation& client_task);
	void EnableTask(int character_id, int task_count, int *task_list);
	void DisableTask(int character_id, int task_count, int *task_list);
	bool IsTaskEnabled(int task_id);
	int EnabledTaskCount(int task_set_id);
	int ActiveSpeakTask(Client* client, NPC* npc);
	int ActiveSpeakActivity(Client* client, NPC* npc, int task_id);
	int ActiveTasksInSet(int task_set_id);
	int CompletedTasksInSet(int task_set_id);
	bool HasSlotForTask(const TaskInformation* task);
	void CreateTaskDynamicZone(Client* client, int task_id, DynamicZone& dz);
	void ListTaskTimers(Client* client);
	void KickPlayersSharedTask(Client* client);
	void LockSharedTask(Client* client, bool lock);
	void ClearLastOffers() { m_last_offers.clear(); }
	bool CanAcceptNewTask(Client* client, int task_id, int npc_entity_id) const;
	bool HasExploreTask(Client* client) const;
	void EndSharedTask(Client* client, bool send_fail);
	bool CompleteTask(Client *c, uint32 task_id);
	bool UncompleteTask(int task_id);

	inline bool HasFreeTaskSlot() { return m_active_task.task_id == TASKSLOTEMPTY; }

	friend class TaskManager;

	// wrapper to call internal IncrementDoneCount
	void SharedTaskIncrementDoneCount(
		Client *client,
		int task_id,
		int activity_id,
		int done_count,
		bool ignore_quest_update = false
	);

	const ClientTaskInformation &GetActiveSharedTask() const;
	bool HasActiveSharedTask();

private:
	const TaskInformation* GetTaskData(const ClientTaskInformation& client_task) const;

	void AddOffer(int task_id, uint16_t npc_entity_id) { m_last_offers.push_back({task_id, npc_entity_id}); };
	void AddReplayTimer(Client *client, ClientTaskInformation& client_task, const TaskInformation& task);
	bool CanUpdate(Client* client, const TaskUpdateFilter& filter, int task_id,
		const ActivityInformation& activity, const ClientActivityInformation& client_activity) const;
	int DispatchEventTaskComplete(Client* client, ClientTaskInformation& client_task, int activity_id);
	std::pair<int, int> FindTask(Client* client, const TaskUpdateFilter& filter) const;
	void RecordCompletedTask(uint32_t character_id, const TaskInformation& task, const ClientTaskInformation& client_task);
	void UpdateTasksOnKill(Client* client, Client* exp_client, NPC* npc);
	int UpdateTasks(Client* client, const TaskUpdateFilter& filter, int count = 1);

	int IncrementDoneCount(
		Client *client,
		const TaskInformation* task_data,
		int task_index,
		int activity_id,
		int count = 1,
		bool ignore_quest_update = false
	);

	bool UnlockActivities(Client* client, ClientTaskInformation& task_info);

	inline ClientTaskInformation *GetClientTaskInfo(TaskType task_type, int index)
	{
		ClientTaskInformation *info = nullptr;
		switch (task_type) {
			case TaskType::Task:
				if (index == TASKSLOTTASK) {
					info = &m_active_task;
				}
				break;
			case TaskType::Shared:
				if (index == TASKSLOTSHAREDTASK) {
					info = &m_active_shared_task;
				}
				break;
			case TaskType::Quest:
				if (index < MAXACTIVEQUESTS) {
					info = &m_active_quests[index];
				}
				break;
			default:
				break;
		}
		return info;
	}

	union { // easier to loop over
		struct {
			ClientTaskInformation m_active_task; // only one

			// acts as a read-only "view" of data that is managed by world and the internal task
			// system largely behaves like other tasks but shims logic to world where necessary
			ClientTaskInformation m_active_shared_task; // only one
			ClientTaskInformation m_active_quests[MAXACTIVEQUESTS];
		};
		ClientTaskInformation m_active_tasks[MAXACTIVEQUESTS + 2] = {};
	};
	// Shared tasks should be limited to 1 as well
	int                                   m_active_task_count;
	std::vector<int>                      m_enabled_tasks;
	std::vector<CompletedTaskInformation> m_completed_tasks;
	int                                   m_last_completed_task_loaded;
	std::vector<TaskOffer>                m_last_offers;
	bool                                  m_has_explore_task = false;

	static void ShowClientTaskInfoMessage(ClientTaskInformation *task, Client *c);

	void SyncSharedTaskZoneClientDoneCountState(
		Client *p_client,
		TaskType type,
		int task_index,
		int activity_id,
		uint32 done_count
	);
	bool HasActiveTasks();
};
