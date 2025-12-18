#include "common/repositories/completed_tasks_repository.h"
#include "common/shared_tasks.h"
#include "zone/client.h"
#include "zone/worldserver.h"

extern WorldServer worldserver;

void command_task(Client *c, const Seperator *sep)
{
	if (!RuleB(TaskSystem, EnableTaskSystem)) {
		c->Message(Chat::White, "This command cannot be used while the Task system is disabled.");
		return;
	}

	const int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Syntax: #task [subcommand]");
		c->Message(Chat::White, "------------------------------------------------");
		c->Message(Chat::White, "# Task System Commands");
		c->Message(Chat::White, "------------------------------------------------");
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] List active tasks for a client",
				Saylink::Silent("#task show", "show")
			).c_str()
		);
		c->Message(Chat::White, "--- update <task_id> <activity_id> [count] | Updates task");
		c->Message(Chat::White, "--- assign <task_id> | Assigns task to client");
		c->Message(Chat::White, "--- complete <task_id> | Completes a task if a client has it assigned to them");
		c->Message(Chat::White, "--- uncomplete <task_id> | Uncompletes a task if a client has completed it");
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] Reload all Task information from the database",
				Saylink::Silent("#task reloadall", "reloadall")
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] <task_id> Reload Task and Activity information for a single task",
				Saylink::Silent("#task reload task", "reload task")
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] Reload goal/reward list information",
				Saylink::Silent("#task reload lists", "reload lists")
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] Reload task set information",
				Saylink::Silent("#task reload sets", "reload sets")
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] Purges targeted characters task timers",
				Saylink::Silent("#task purgetimers", "purgetimers")
			).c_str()
		);

		c->Message(Chat::White, "------------------------------------------------");
		c->Message(Chat::White, "# Shared Task System Commands");
		c->Message(Chat::White, "------------------------------------------------");
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] Purges all active Shared Tasks in memory and database ",
				Saylink::Silent("#task sharedpurge", "sharedpurge")
			).c_str()
		);
		return;
	}

	Client *t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	const bool is_assign      = !strcasecmp(sep->arg[1], "assign");
	const bool is_complete    = !strcasecmp(sep->arg[1], "complete");
	const bool is_purgetimers = !strcasecmp(sep->arg[1], "purgetimers");
	const bool is_reload      = !strcasecmp(sep->arg[1], "reload");
	const bool is_reloadall   = !strcasecmp(sep->arg[1], "reloadall");
	const bool is_sharedpurge = !strcasecmp(sep->arg[1], "sharedpurge");
	const bool is_show        = !strcasecmp(sep->arg[1], "show");
	const bool is_uncomplete  = !strcasecmp(sep->arg[1], "uncomplete");
	const bool is_update      = !strcasecmp(sep->arg[1], "update");

	if (
		!is_assign &&
		!is_complete &&
		!is_purgetimers &&
		!is_reload &&
		!is_reloadall &&
		!is_sharedpurge &&
		!is_show &&
		!is_uncomplete &&
		!is_update
	) {
		c->Message(Chat::White, "Syntax: #task [subcommand]");
		c->Message(Chat::White, "------------------------------------------------");
		c->Message(Chat::White, "# Task System Commands");
		c->Message(Chat::White, "------------------------------------------------");
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] List active tasks for a client",
				Saylink::Silent("#task show", "show")
			).c_str()
		);
		c->Message(Chat::White, "--- update <task_id> <activity_id> [count] | Updates task");
		c->Message(Chat::White, "--- assign <task_id> | Assigns task to client");
		c->Message(Chat::White, "--- uncomplete <task_id> | Uncompletes a task if a client has completed it");
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] Reload all Task information from the database",
				Saylink::Silent("#task reloadall", "reloadall")
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] <task_id> Reload Task and Activity information for a single task",
				Saylink::Silent("#task reload task", "reload task")
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] Reload goal/reward list information",
				Saylink::Silent("#task reload lists", "reload lists")
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] Reload task set information",
				Saylink::Silent("#task reload sets", "reload sets")
			).c_str()
		);
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] Purges targeted characters task timers",
				Saylink::Silent("#task purgetimers", "purgetimers")
			).c_str()
		);

		c->Message(Chat::White, "------------------------------------------------");
		c->Message(Chat::White, "# Shared Task System Commands");
		c->Message(Chat::White, "------------------------------------------------");
		c->Message(
			Chat::White,
			fmt::format(
				"--- [{}] Purges all active Shared Tasks in memory and database ",
				Saylink::Silent("#task sharedpurge", "sharedpurge")
			).c_str()
		);
		return;
	}

	if (is_assign) {
		const uint32 task_id = Strings::ToUnsignedInt(sep->arg[2]);
		if (task_id) {
			t->AssignTask(task_id, 0, false);
			c->Message(
				Chat::White,
				fmt::format(
					"Assigned {} (ID {}) to {}.",
					TaskManager::Instance()->GetTaskName(task_id),
					task_id,
					c->GetTargetDescription(t)
				).c_str()
			);
		}
	} else if (is_complete) {
		if (sep->IsNumber(2)) {
			const uint32 task_id = Strings::ToUnsignedInt(sep->arg[2]);
			if (!task_id) {
				c->Message(Chat::White, "Invalid task ID specified.");
				return;
			}

			if (t->IsTaskActive(task_id)) {
				const bool did_complete = t->CompleteTask(task_id);
				if (did_complete) {
					c->Message(
						Chat::White,
						fmt::format(
							"Successfully completed {} (ID {}) for {}.",
							TaskManager::Instance()->GetTaskName(task_id),
							task_id,
							c->GetTargetDescription(t)
						).c_str()
					);
				} else {
					c->Message(
						Chat::White,
						fmt::format(
							"Failed to complete {} (ID {}) for {}.",
							TaskManager::Instance()->GetTaskName(task_id),
							task_id,
							c->GetTargetDescription(t)
						).c_str()
					);
				}
			} else {
				c->Message(
					Chat::White,
					fmt::format(
						"{} {} not have not {} (ID {}) assigned to them.",
						c->GetTargetDescription(t, TargetDescriptionType::UCYou),
						c == t ? "do" : "does",
						TaskManager::Instance()->GetTaskName(task_id),
						task_id
					).c_str()
				);
			}
		}
	} else if (is_purgetimers) {
		c->Message(
			Chat::White,
			fmt::format(
				"Task timers have been purged for {}.",
				c->GetTargetDescription(t)
			).c_str()
		);

		if (c != t) {
			t->Message(Chat::White, "Your task timers have been purged by a GM.");
		}

		t->PurgeTaskTimers();
	} else if (is_reload) {
		if (arguments >= 2) {
			const bool is_sets = !strcasecmp(sep->arg[2], "sets");
			const bool is_task = !strcasecmp(sep->arg[2], "task");

			if (is_sets) {
				c->Message(Chat::White, "Attempting to reload task sets.");
				worldserver.SendReloadTasks(RELOADTASKSETS);
				c->Message(Chat::White, "Successfully reloaded task sets.");
			} else if (is_task && arguments == 3 && sep->IsNumber(3)) {
				const uint32 task_id = Strings::ToUnsignedInt(sep->arg[3]);
				if (task_id) {
					c->Message(
						Chat::White,
						fmt::format(
							"Attempting to reload {} (ID {}).",
							TaskManager::Instance()->GetTaskName(task_id),
							task_id
						).c_str()
					);
					worldserver.SendReloadTasks(RELOADTASKS, task_id);
					c->Message(
						Chat::White,
						fmt::format(
							"Successfully reloaded {} (ID {}).",
							TaskManager::Instance()->GetTaskName(task_id),
							task_id
						).c_str()
					);
				}
			}
		}
	} else if (is_reloadall) {
		c->Message(Chat::White, "Attempting to reload tasks.");
		worldserver.SendReloadTasks(RELOADTASKS);
		c->Message(Chat::White, "Successfully reloaded tasks.");
	} else if (is_sharedpurge) {
		const bool is_confirm = !strcasecmp(sep->arg[2], "confirm");

		if (is_confirm) {
			LogTasksDetail("Sending purge request");
			auto pack = new ServerPacket(ServerOP_SharedTaskPurgeAllCommand, 0);
			worldserver.SendPacket(pack);
			safe_delete(pack);
			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"[WARNING] This will purge all active Shared Tasks [{}]?",
				Saylink::Silent("#task sharedpurge confirm", "confirm")
			).c_str()
		);
	} else if (is_show) {
		t->ShowClientTasks(c);
	} else if (is_uncomplete) {
		if (sep->IsNumber(2)) {
			const uint32 task_id = Strings::ToUnsignedInt(sep->arg[2]);
			if (!task_id) {
				c->Message(Chat::White, "Invalid task ID specified.");
				return;
			}

			if (t->UncompleteTask(task_id)) {
				c->Message(
					Chat::White,
					fmt::format(
						"Successfully uncompleted {} (ID {}) for {}.",
						TaskManager::Instance()->GetTaskName(task_id),
						task_id,
						c->GetTargetDescription(t)
					).c_str()
				);
			} else {
				c->Message(
					Chat::White,
					fmt::format(
						"{} {} not completed {} (ID {}).",
						c->GetTargetDescription(t, TargetDescriptionType::UCYou),
						c == t ? "have" : "has",
						TaskManager::Instance()->GetTaskName(task_id),
						task_id
					).c_str()
				);
			}
		}
	} else if (is_update) {
		if (arguments >= 3 && sep->IsNumber(2) && sep->IsNumber(3)) {
			const uint32 task_id     = Strings::ToUnsignedInt(sep->arg[2]);
			const uint32 activity_id = Strings::ToUnsignedInt(sep->arg[3]);
			int          count       = 1;

			if (arguments == 4 && sep->IsNumber(4)) {
				count = Strings::ToInt(sep->arg[4]);
				if (count <= 0) {
					count = 1;
				}
			}

			c->Message(
				Chat::White,
				fmt::format(
					"Updating {} (ID {}), activity {} with a count of {} for {}.",
					TaskManager::Instance()->GetTaskName(task_id),
					task_id,
					activity_id,
					count,
					c->GetTargetDescription(t)
				).c_str()
			);

			t->UpdateTaskActivity(task_id, activity_id, count);
			t->ShowClientTasks(c);
		}
	}
}
