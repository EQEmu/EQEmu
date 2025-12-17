#pragma once

#include "common/database.h"
#include "common/repositories/character_data_repository.h"
#include "common/repositories/shared_tasks_repository.h"
#include "common/repositories/task_activities_repository.h"
#include "common/repositories/tasks_repository.h"
#include "common/tasks.h"
#include "common/timer.h"
#include "common/types.h"

#include <string>
#include <vector>

// ops
#define ServerOP_SharedTaskRequest                  0x0300 // zone -> world. Player trying to get task. Relayed world -> zone on confirmation
#define ServerOP_SharedTaskAddPlayer                0x0301 // bidirectional. /taskaddplayer request zone -> world. success world -> zone
#define ServerOP_SharedTaskMakeLeader               0x0302 // zone -> world -> zone
#define ServerOP_SharedTaskRemovePlayer             0x0303 // zone -> world -> zone
#define ServerOP_SharedTaskQuit                     0x0304 // zone -> world. Player trying to delete task
#define ServerOP_SharedTaskUpdate                   0x0305 // zone -> world. Client sending task update to world. Relayed world -> zone on confirmation
#define ServerOP_SharedTaskMemberlist               0x0306 // world -> zone. Send shared task memberlist
#define ServerOP_SharedTaskRequestMemberlist        0x0307 // zone -> world. Send shared task memberlist (zone in initial for now, could change)
#define ServerOP_SharedTaskAcceptNewTask            0x0308 // world -> zone. World verified, continue AcceptNewTask
#define ServerOP_SharedTaskInvitePlayer             0x0309 // world -> zone. Sends task invite to player
#define ServerOP_SharedTaskInviteAcceptedPlayer     0x0310 // zone -> world. Confirming task invite
#define ServerOP_SharedTaskCreateDynamicZone        0x0311 // zone -> world
#define ServerOP_SharedTaskPurgeAllCommand          0x0312 // zone -> world
#define ServerOP_SharedTaskPlayerList               0x0313 // zone -> world /taskplayerlist command
#define ServerOP_SharedTaskMemberChange             0x0314 // world -> zone. Send shared task single member added/removed (client also handles message)
#define ServerOP_SharedTaskKickPlayers              0x0315 // zone -> world /kickplayers task
#define ServerOP_SharedTaskLock                     0x0316 // zone -> world
#define ServerOP_SharedTaskEnd                      0x0317 // zone -> world
#define ServerOP_SharedTaskEndByDz                  0x0318 // zone -> world
#define ServerOP_SharedTaskFailed                   0x0319 // world -> zone. Sends red text task failed banner to client

enum class SharedTaskRequestGroupType {
	Solo = 0,
	Group,
	Raid
};

// used in
// ServerOP_SharedTaskRequest

// ServerOP_SharedTaskAcceptNewTask
struct ServerSharedTaskRequest_Struct {
	uint32 requested_character_id;
	uint32 requested_task_id;
	uint32 requested_npc_entity_id; // original task logic passthrough
	uint32 accept_time;
};

// ServerOP_SharedTaskInvitePlayer
struct ServerSharedTaskInvitePlayer_Struct {
	uint32 requested_character_id;
	uint32 invite_shared_task_id;
	char   task_name[64];
	char   inviter_name[64];
};

// ServerOP_SharedTaskQuit
// gets re-used when sent back to clients
struct ServerSharedTaskQuit_Struct {
	uint32 requested_character_id;
	uint32 task_id;
	bool   remove_from_db;
};

// used in the shared task request process (currently)
struct SharedTaskMember {
	uint32      character_id = 0;
	std::string character_name;
	bool        is_leader    = false;

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(character_id, character_name, is_leader);
	}
};

// used in shared task requests to validate group/raid members
struct SharedTaskRequest {
	uint32_t lowest_level;
	uint32_t highest_level;
	uint32_t leader_id;
	SharedTaskRequestGroupType group_type;
	std::vector<uint32_t> character_ids;
	std::vector<SharedTaskMember> members;
};

// ServerOP_SharedTaskMemberlist
// builds the buffer and sends to clients directly
struct ServerSharedTaskMemberListPacket_Struct {
	uint32 destination_character_id;
	uint32 cereal_size;
	char   cereal_serialized_members[0]; // serialized member list using cereal
};

struct ServerSharedTaskMemberChangePacket_Struct {
	uint32 destination_character_id;
	uint32 shared_task_id;
	bool   removed;
	char   player_name[64];
};

struct SharedTaskActivityStateEntry {
	uint32 activity_id;
	uint32 done_count;
	uint32 max_done_count; // goalcount
	uint32 updated_time;
	uint32 completed_time;
	int    req_activity_id;
	int    step;
	bool   optional;
	ActivityState activity_state; // world only uses Hidden and Completed states
};

struct ServerSharedTaskActivityUpdate_Struct {
	uint32 source_character_id;
	uint32 task_id;
	uint32 activity_id;
	uint32 done_count;
	bool   ignore_quest_update;
};

struct ServerSharedTaskRequestMemberlist_Struct {
	uint32 source_character_id;
	uint32 task_id;
};

struct ServerSharedTaskRemovePlayer_Struct {
	uint32 source_character_id;
	uint32 task_id;
	char   player_name[64];
};

struct ServerSharedTaskAddPlayer_Struct {
	uint32 source_character_id;
	uint32 task_id;
	char   player_name[64];
};

struct ServerSharedTaskMakeLeader_Struct {
	uint32 source_character_id;
	uint32 task_id;
	char   player_name[64];
};

struct ServerSharedTaskInviteAccepted_Struct {
	uint32 source_character_id;
	uint32 shared_task_id;
	bool   accepted;
	char   player_name[64];
};

struct ServerSharedTaskCreateDynamicZone_Struct {
	uint32 source_character_id;
	uint32 task_id;
	uint32 cereal_size;
	char   cereal_data[0]; // serialized dz with creation parameters
};

struct ServerSharedTaskPlayerList_Struct {
	uint32 source_character_id;
	uint32 task_id;
};

struct ServerSharedTaskKickPlayers_Struct {
	uint32 source_character_id;
	uint32 task_id;
};

struct ServerSharedTaskLock_Struct {
	uint32 source_character_id;
	uint32 task_id;
	bool   lock;
};

struct ServerSharedTaskCharacterTask_Struct {
	uint32 character_id;
	uint32 task_id;
};

struct ServerSharedTaskEnd_Struct {
	uint32 character_id;
	uint32 task_id;
	uint32 dz_id;
	bool   send_fail; // if true members receive red text failure banner
};

class SharedTask {
public:
	// used in both zone and world validation
	static SharedTaskRequest GetRequestCharacters(Database& db, uint32_t requested_character_id);

	void AddCharacterToMemberHistory(uint32_t character_id);
	SharedTaskMember FindMemberFromCharacterID(uint32_t character_id) const;
	SharedTaskMember FindMemberFromCharacterName(const std::string& character_name) const;
	SharedTaskMember GetLeader() const;
	std::vector<SharedTaskActivityStateEntry> GetActivityState() const;
	const std::vector<SharedTaskMember>& GetMembers() const;
	bool IsExpired() const;

	// getters
	const std::vector<TaskActivitiesRepository::TaskActivities> &GetTaskActivityData() const;
	const TasksRepository::Tasks &GetTaskData() const;

	// setters
	void SetMembers(const std::vector<SharedTaskMember> &members);
	void SetSharedTaskActivityState(const std::vector<SharedTaskActivityStateEntry> &activity_state);
	void SetTaskActivityData(const std::vector<TaskActivitiesRepository::TaskActivities> &task_activity_data);
	void SetTaskData(const TasksRepository::Tasks &task_data);

	// active record of database shared task
	const SharedTasksRepository::SharedTasks &GetDbSharedTask() const;
	void SetDbSharedTask(const SharedTasksRepository::SharedTasks &t);

	std::vector<SharedTaskActivityStateEntry> m_shared_task_activity_state;
	std::vector<SharedTaskMember>             m_members;
	std::vector<uint32_t>                     member_id_history; // past and present members for replay timers
	std::vector<uint32_t>                     dynamic_zone_ids;

	Timer terminate_timer;

protected:
	SharedTasksRepository::SharedTasks m_db_shared_task;

	// reference to task data (only for this shared task)
	TasksRepository::Tasks                                m_task_data;
	std::vector<TaskActivitiesRepository::TaskActivities> m_task_activity_data;
};
