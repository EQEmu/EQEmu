#include "shared_task_world_messaging.h"

#include "common/eqemu_logsys.h"
#include "common/repositories/shared_task_members_repository.h"
#include "common/repositories/task_activities_repository.h"
#include "common/repositories/tasks_repository.h"
#include "common/shared_tasks.h"
#include "common/tasks.h"
#include "world/cliententry.h"
#include "world/cliententry.h"
#include "world/clientlist.h"
#include "world/dynamic_zone.h"
#include "world/shared_task_manager.h"
#include "world/worlddb.h"
#include "world/zonelist.h"
#include "world/zoneserver.h"

void SharedTaskWorldMessaging::HandleZoneMessage(ServerPacket *pack)
{
	switch (pack->opcode) {
		case ServerOP_SharedTaskRequest: {
			auto *r = (ServerSharedTaskRequest_Struct *) pack->pBuffer;
			LogTasksDetail(
				"[ServerOP_SharedTaskRequest] Received request from character [{}] task_id [{}] npc_entity_id [{}]",
				r->requested_character_id,
				r->requested_task_id,
				r->requested_npc_entity_id
			);

			SharedTaskManager::Instance()->AttemptSharedTaskCreation(
				r->requested_task_id,
				r->requested_character_id,
				r->requested_npc_entity_id
			);

			break;
		}
		case ServerOP_SharedTaskQuit: {
			auto *r = (ServerSharedTaskQuit_Struct *) pack->pBuffer;
			LogTasksDetail(
				"[ServerOP_SharedTaskQuit] Received request from character [{}] task_id [{}] remove_from_db [{}]",
				r->requested_character_id,
				r->task_id,
				r->remove_from_db
			);

			auto s = SharedTaskManager::Instance()->FindSharedTaskByTaskIdAndCharacterId(r->task_id, r->requested_character_id);
			if (s) {
				auto member = s->FindMemberFromCharacterID(r->requested_character_id);
				if (member.character_id != 0) {
					SharedTaskManager::Instance()->RemoveMember(s, member, r->remove_from_db);
				}
			}
			break;
		}
		case ServerOP_SharedTaskKickPlayers: {
			auto r = reinterpret_cast<ServerSharedTaskKickPlayers_Struct *>(pack->pBuffer);
			LogTasksDetail(
				"[ServerOP_SharedTaskKickPlayers] Received request from character [{}] task_id [{}]",
				r->source_character_id,
				r->task_id
			);

			auto t = SharedTaskManager::Instance()->FindSharedTaskByTaskIdAndCharacterId(r->task_id, r->source_character_id);
			if (t) {
				auto leader = t->GetLeader();
				if (leader.character_id != r->source_character_id) {
					ClientList::Instance()->SendCharacterMessageID(r->source_character_id, Chat::Red, TaskStr::NOT_LEADER, {leader.character_name});
				}
				else {
					SharedTaskManager::Instance()->RemoveEveryoneFromSharedTask(t, r->source_character_id);
				}
			}

			break;
		}
		case ServerOP_SharedTaskUpdate: {
			auto *r = (ServerSharedTaskActivityUpdate_Struct *) pack->pBuffer;

			LogTasksDetail(
				"[ServerOP_SharedTaskUpdate] Received request from character [{}] task_id [{}] activity_id [{}] donecount [{}] ignore_quest_update [{}]",
				r->source_character_id,
				r->task_id,
				r->activity_id,
				r->done_count,
				(r->ignore_quest_update ? "true" : "false")
			);

			SharedTaskManager::Instance()->SharedTaskActivityUpdate(
				r->source_character_id,
				r->task_id,
				r->activity_id,
				r->done_count,
				r->ignore_quest_update
			);

			break;
		}
		case ServerOP_SharedTaskRequestMemberlist: {
			auto *r = (ServerSharedTaskRequestMemberlist_Struct *) pack->pBuffer;

			LogTasksDetail(
				"[ServerOP_SharedTaskRequestMemberlist] Received request from character [{}] task_id [{}]",
				r->source_character_id,
				r->task_id
			);

			auto t = SharedTaskManager::Instance()->FindSharedTaskByTaskIdAndCharacterId(r->task_id, r->source_character_id);
			if (t) {
				LogTasksDetail(
					"[ServerOP_SharedTaskRequestMemberlist] Found shared task character [{}] shared_task_id [{}]",
					r->source_character_id,
					t->GetDbSharedTask().id
				);

				SharedTaskManager::Instance()->SendSharedTaskMemberList(
					r->source_character_id,
					t->GetMembers()
				);
			}

			break;
		}
		case ServerOP_SharedTaskRemovePlayer: {
			auto *r = (ServerSharedTaskRemovePlayer_Struct *) pack->pBuffer;

			LogTasksDetail(
				"[ServerOP_SharedTaskRemovePlayer] Received request from character [{}] task_id [{}] player_name [{}]",
				r->source_character_id,
				r->task_id,
				r->player_name
			);

			auto t = SharedTaskManager::Instance()->FindSharedTaskByTaskIdAndCharacterId(r->task_id, r->source_character_id);
			if (t) {
				LogTasksDetail(
					"[ServerOP_SharedTaskRemovePlayer] Found shared task character [{}] shared_task_id [{}]",
					r->source_character_id,
					t->GetDbSharedTask().id
				);

				auto leader = t->GetLeader();
				if (leader.character_id != r->source_character_id) {
					ClientList::Instance()->SendCharacterMessageID(r->source_character_id, Chat::Red, TaskStr::NOT_LEADER, {leader.character_name});
				}
				else {
					LogTasksDetail(
						"[ServerOP_SharedTaskRemovePlayer] character_id [{}] shared_task_id [{}] is_leader",
						r->source_character_id,
						t->GetDbSharedTask().id
					);

					auto member = t->FindMemberFromCharacterName(r->player_name);
					if (member.character_id == 0) {
						SharedTaskManager::Instance()->SendLeaderMessageID(t, Chat::Red, TaskStr::IS_NOT_MEMBER, {r->player_name});
					}
					else {
						SharedTaskManager::Instance()->RemoveMember(t, member, true);
						SharedTaskManager::Instance()->SendLeaderMessageID(t, Chat::Yellow, TaskStr::PLAYER_REMOVED, {member.character_name, t->GetTaskData().title});
					}
				}
			}

			break;
		}
		case ServerOP_SharedTaskMakeLeader: {
			auto *r = (ServerSharedTaskMakeLeader_Struct *) pack->pBuffer;

			LogTasksDetail(
				"[ServerOP_SharedTaskMakeLeader] Received request from character [{}] task_id [{}] player_name [{}]",
				r->source_character_id,
				r->task_id,
				r->player_name
			);

			auto t = SharedTaskManager::Instance()->FindSharedTaskByTaskIdAndCharacterId(r->task_id, r->source_character_id);
			if (t) {
				LogTasksDetail(
					"[ServerOP_SharedTaskMakeLeader] Found shared task character [{}] shared_task_id [{}]",
					r->source_character_id,
					t->GetDbSharedTask().id
				);

				auto leader = t->GetLeader();
				if (leader.character_id != r->source_character_id) {
					ClientList::Instance()->SendCharacterMessageID(r->source_character_id, Chat::Red, TaskStr::NOT_LEADER, {leader.character_name});
				}
				else if (strcasecmp(leader.character_name.c_str(), r->player_name) == 0) {
					ClientList::Instance()->SendCharacterMessageID(r->source_character_id, Chat::Red, TaskStr::YOU_ALREADY_LEADER);
				}
				else {
					LogTasksDetail(
						"[ServerOP_SharedTaskMakeLeader] character_id [{}] shared_task_id [{}] is_leader",
						r->source_character_id,
						t->GetDbSharedTask().id
					);

					std::string character_name = r->player_name;
					SharedTaskManager::Instance()->MakeLeaderByPlayerName(t, character_name);
				}
			}

			break;
		}
		case ServerOP_SharedTaskAddPlayer: {
			auto *r = (ServerSharedTaskAddPlayer_Struct *) pack->pBuffer;

			LogTasksDetail(
				"[ServerOP_SharedTaskAddPlayer] Received request from character [{}] task_id [{}] player_name [{}]",
				r->source_character_id,
				r->task_id,
				r->player_name
			);

			auto t = SharedTaskManager::Instance()->FindSharedTaskByTaskIdAndCharacterId(r->task_id, r->source_character_id);
			if (t) {
				LogTasksDetail(
					"[ServerOP_SharedTaskAddPlayer] Found shared task character [{}] shared_task_id [{}]",
					r->source_character_id,
					t->GetDbSharedTask().id
				);

				auto leader = t->GetLeader();
				if (leader.character_id != r->source_character_id) {
					// taskadd is client sided with System color in newer clients, server side might still be red
					ClientList::Instance()->SendCharacterMessageID(r->source_character_id, Chat::Red, TaskStr::NOT_LEADER, {leader.character_name});
				}
				else {
					LogTasksDetail(
						"[ServerOP_SharedTaskAddPlayer] character_id [{}] shared_task_id [{}] is_leader",
						r->source_character_id,
						t->GetDbSharedTask().id
					);

					std::string character_name = r->player_name;
					SharedTaskManager::Instance()->InvitePlayerByPlayerName(t, character_name);
				}
			}

			break;
		}
		case ServerOP_SharedTaskInviteAcceptedPlayer: {
			auto *r = (ServerSharedTaskInviteAccepted_Struct *) pack->pBuffer;

			LogTasksDetail(
				"[ServerOP_SharedTaskInviteAcceptedPlayer] Received request from source_character_id [{}] shared_task_id [{}] accepted [{}]",
				r->source_character_id,
				r->shared_task_id,
				r->accepted
			);

			auto t = SharedTaskManager::Instance()->FindSharedTaskById(r->shared_task_id);
			if (t && SharedTaskManager::Instance()->IsInvitationActive(r->shared_task_id, r->source_character_id)) {
				LogTasksDetail(
					"[ServerOP_SharedTaskInviteAcceptedPlayer] Found shared task character [{}] shared_task_id [{}]",
					r->source_character_id,
					t->GetDbSharedTask().id
				);

				SharedTaskManager::Instance()->RemoveActiveInvitation(r->shared_task_id, r->source_character_id);

				if (r->accepted) {
					SharedTaskManager::Instance()->AddPlayerByCharacterIdAndName(t, r->source_character_id, r->player_name);
				}
				else {
					SharedTaskManager::Instance()->SendLeaderMessageID(t, Chat::Red, TaskStr::PLAYER_DECLINED_OFFER, {r->player_name});
				}
			}
			break;
		}
		case ServerOP_SharedTaskCreateDynamicZone: {
			auto buf = reinterpret_cast<ServerSharedTaskCreateDynamicZone_Struct *>(pack->pBuffer);

			LogTasksDetail(
				"[ServerOP_SharedTaskCreateDynamicZone] Received request from source_character_id [{}] task_id [{}]",
				buf->source_character_id,
				buf->task_id
			);

			auto t = SharedTaskManager::Instance()->FindSharedTaskByTaskIdAndCharacterId(buf->task_id, buf->source_character_id);
			if (t) {
				DynamicZone dz;
				dz.Unserialize({ buf->cereal_data, buf->cereal_size });

				SharedTaskManager::Instance()->CreateDynamicZone(t, dz);
			}
			break;
		}
		case ServerOP_SharedTaskPurgeAllCommand: {
			LogTasksDetail("[ServerOP_SharedTaskPurgeAllCommand] Received request to purge all shared tasks");

			SharedTaskManager::Instance()->PurgeAllSharedTasks();
			auto p = std::make_unique<ServerPacket>(
				ServerOP_SharedTaskPurgeAllCommand,
				0
			);

			ZSList::Instance()->SendPacket(p.get());

			break;
		}
		case ServerOP_SharedTaskPlayerList: {
			auto buf = reinterpret_cast<ServerSharedTaskPlayerList_Struct *>(pack->pBuffer);

			LogTasksDetail(
				"[ServerOP_SharedTaskPlayerList] Received request from source_character_id [{}] task_id [{}]",
				buf->source_character_id,
				buf->task_id
			);

			auto s = SharedTaskManager::Instance()->FindSharedTaskByTaskIdAndCharacterId(buf->task_id, buf->source_character_id);
			if (s) {
				std::vector<std::string> player_names;

				for (const auto &member : s->GetMembers()) {
					player_names.emplace_back(member.character_name);

					if (member.is_leader) {
						ClientList::Instance()->SendCharacterMessageID(buf->source_character_id, Chat::Yellow, TaskStr::LEADER_PRINT, {member.character_name});
					}
				}

				std::string player_list = fmt::format("{}", Strings::Join(player_names, ", "));
				ClientList::Instance()->SendCharacterMessageID(buf->source_character_id, Chat::Yellow, TaskStr::MEMBERS_PRINT, {player_list});
			}

			break;
		}
		case ServerOP_SharedTaskLock: {
			auto buf = reinterpret_cast<ServerSharedTaskLock_Struct*>(pack->pBuffer);
			auto shared_task = SharedTaskManager::Instance()->FindSharedTaskByTaskIdAndCharacterId(buf->task_id, buf->source_character_id);
			if (shared_task)
			{
				SharedTaskManager::Instance()->LockTask(shared_task, buf->lock);
			}
			break;
		}
		case ServerOP_SharedTaskEndByDz: {
			auto buf = reinterpret_cast<ServerSharedTaskEnd_Struct*>(pack->pBuffer);
			auto shared_task = SharedTaskManager::Instance()->FindSharedTaskByDzId(buf->dz_id);
			if (shared_task)
			{
				SharedTaskManager::Instance()->Terminate(*shared_task, buf->send_fail, true);
			}
			break;
		}
		case ServerOP_SharedTaskEnd: {
			auto buf = reinterpret_cast<ServerSharedTaskEnd_Struct*>(pack->pBuffer);
			auto shared_task = SharedTaskManager::Instance()->FindSharedTaskByTaskIdAndCharacterId(buf->task_id, buf->character_id);
			if (shared_task)
			{
				SharedTaskManager::Instance()->Terminate(*shared_task, buf->send_fail, true);
			}
			break;
		}
		default:
			break;
	}
}
