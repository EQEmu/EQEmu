#include "shared_task_zone_messaging.h"

#include "common/servertalk.h"
#include "common/shared_tasks.h"
#include "zone/client.h"

#include <memory>
#include <vector>

void SharedTaskZoneMessaging::HandleWorldMessage(ServerPacket *pack)
{
	switch (pack->opcode) {
		case ServerOP_SharedTaskAcceptNewTask: {
			auto p = reinterpret_cast<ServerSharedTaskRequest_Struct *>(pack->pBuffer);
			auto c = entity_list.GetClientByCharID(p->requested_character_id);
			if (c) {
				LogTasks("[ServerOP_SharedTaskAcceptNewTask] We're back in zone and I found [{}]", c->GetCleanName());

				c->m_requesting_shared_task = true;
				c->GetTaskState()
					->AcceptNewTask(
						c,
						(int) p->requested_task_id,
						(int) p->requested_npc_entity_id,
						p->accept_time
					);
				c->LoadClientTaskState();
				c->m_requesting_shared_task = false;
			}

			break;
		}
		case ServerOP_SharedTaskUpdate: {
			auto p = reinterpret_cast<ServerSharedTaskActivityUpdate_Struct *>(pack->pBuffer);
			auto c = entity_list.GetClientByCharID(p->source_character_id);
			if (c) {
				LogTasks("[ServerOP_SharedTaskUpdate] We're back in zone and I found [{}]", c->GetCleanName());

				c->m_shared_task_update = true;
				c->GetTaskState()->SharedTaskIncrementDoneCount(
					c,
					(int) p->task_id,
					(int) p->activity_id,
					(int) p->done_count,
					p->ignore_quest_update
				);
				c->m_shared_task_update = false;
			}

			break;
		}
		case ServerOP_SharedTaskQuit: {
			auto p = reinterpret_cast<ServerSharedTaskQuit_Struct *>(pack->pBuffer);
			auto c = entity_list.GetClientByCharID(p->requested_character_id);
			if (c) {
				LogTasks("[ServerOP_SharedTaskQuit] We're back in zone and I found [{}]", c->GetCleanName());

				c->m_requested_shared_task_removal = true;
				c->GetTaskState()->CancelTask(
					c,
					TASKSLOTSHAREDTASK,
					static_cast<TaskType>((int) TASK_TYPE_SHARED),
					p->remove_from_db
				);
				c->m_requested_shared_task_removal = false;

				c->SetSharedTaskId(0);
			}

			break;
		}
		case ServerOP_SharedTaskMemberlist: {
			auto p = reinterpret_cast<ServerSharedTaskMemberListPacket_Struct *>(pack->pBuffer);

			LogTasks(
				"[ServerOP_SharedTaskMemberlist] We're back in zone and I'm searching for [{}]",
				p->destination_character_id
			);

			// find character and route packet
			auto c = entity_list.GetClientByCharID(p->destination_character_id);
			if (c) {
				LogTasks("[ServerOP_SharedTaskMemberlist] We're back in zone and I found [{}]", c->GetCleanName());

				std::vector<SharedTaskMember> members;

				// deserialize members from world
				EQ::Util::MemoryStreamReader ss(p->cereal_serialized_members, p->cereal_size);
				cereal::BinaryInputArchive   archive(ss);
				archive(members);

				SerializeBuffer buf(sizeof(SharedTaskMemberList_Struct) + 15 * members.size());
				buf.WriteInt32(0); // unknown ids
				buf.WriteInt32(0);
				buf.WriteInt32((int32) members.size());

				for (auto &m : members) {
					buf.WriteString(m.character_name);
					buf.WriteInt32(0); // monster mission
					buf.WriteInt8(m.is_leader ? 1 : 0);
				}

				auto outapp = std::make_unique<EQApplicationPacket>(OP_SharedTaskMemberList, buf);
				c->QueuePacket(outapp.get());
			}

			break;
		}
		case ServerOP_SharedTaskMemberChange: {
			auto p = reinterpret_cast<ServerSharedTaskMemberChangePacket_Struct *>(pack->pBuffer);

			LogTasksDetail("[ServerOP_SharedTaskMemberChange] Searching for [{}]", p->destination_character_id);

			auto c = entity_list.GetClientByCharID(p->destination_character_id);
			if (c) {
				LogTasksDetail("[ServerOP_SharedTaskMemberChange] Found [{}]", c->GetCleanName());

				SerializeBuffer buf;
				buf.WriteInt32(0);                 // unique character id of receiver, leave 0 for emu
				buf.WriteInt32(0);                 // unknown, seen 50, 4, 0
				buf.WriteInt8(p->removed ? 0 : 1); // 0: removed 1: added
				buf.WriteString(p->player_name);

				// live sends more after the name but it might just be garbage from
				// a re-used buffer (possibly a former name[64] buffer?)

				auto outapp = std::make_unique<EQApplicationPacket>(OP_SharedTaskMemberChange, buf);
				c->QueuePacket(outapp.get());
			}

			break;
		}
		case ServerOP_SharedTaskInvitePlayer: {
			auto p = reinterpret_cast<ServerSharedTaskInvitePlayer_Struct *>(pack->pBuffer);
			auto c = entity_list.GetClientByCharID(p->requested_character_id);
			if (c) {
				LogTasks("[ServerOP_SharedTaskInvitePlayer] We're back in zone and I found [{}]", c->GetCleanName());

				// init packet
				auto outapp = new EQApplicationPacket(OP_SharedTaskInvite, sizeof(SharedTaskInvite_Struct));
				auto *i     = (SharedTaskInvite_Struct *) outapp->pBuffer;

				// fill
				i->unknown00 = 0;
				i->invite_id = (int) p->invite_shared_task_id;
				strn0cpy(i->inviter_name, p->inviter_name, 64);
				strn0cpy(i->task_name, p->task_name, 64);

				// sends
				c->QueuePacket(outapp);
				safe_delete(outapp);
			}

			break;
		}
		case ServerOP_SharedTaskPurgeAllCommand: {
			LogTasksDetail("[ServerOP_SharedTaskPurgeAllCommand] Syncing clients");

			for (auto &client: entity_list.GetClientList()) {
				Client *c = client.second;
				TaskManager::Instance()->SyncClientSharedTaskState(c, c->GetTaskState());
				c->RemoveClientTaskState();
				c->LoadClientTaskState();
			}

			break;
		}
		case ServerOP_SharedTaskFailed: {
			auto buf = reinterpret_cast<ServerSharedTaskCharacterTask_Struct*>(pack->pBuffer);
			Client* client = entity_list.GetClientByCharID(buf->character_id);
			if (client)
			{
				client->SendTaskFailed(buf->task_id, TASKSLOTSHAREDTASK, TaskType::Shared);
			}
			break;
		}
		default:
			break;
	}

}
