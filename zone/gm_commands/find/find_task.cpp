#include "../../client.h"

void FindTask(Client *c, const Seperator *sep)
{
	if (!RuleB(TaskSystem, EnableTaskSystem)) {
		c->Message(Chat::White, "This command cannot be used while the Task system is disabled.");
		return;
	}

	const auto can_assign_tasks = c->Admin() >= GetCommandStatus("task");

	if (sep->IsNumber(2)) {
		const auto  task_id   = Strings::ToUnsignedInt(sep->arg[2]);
		const auto& task_name = TaskManager::Instance()->GetTaskName(task_id);

		if (task_name.empty()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Task ID {} was not found.",
					task_id
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Task {} | {}",
				task_id,
				task_name
			).c_str()
		);

		return;
	}

	const auto& search_criteria = Strings::ToLower(sep->argplus[2]);

	auto found_count = 0;

	for (const auto& t : TaskManager::Instance()->GetTaskData()) {
		const auto& task_name       = t.second.title;
		const auto& task_name_lower = Strings::ToLower(task_name);
		if (!Strings::Contains(task_name_lower, search_criteria)) {
			continue;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Task {} | {}{}",
				t.first,
				task_name,
				(
					can_assign_tasks ?
					fmt::format(
						" | {} | {} | {}",
						Saylink::Silent(
							fmt::format(
								"#task assign {}",
								t.first
							),
							"Assign"
						),
						Saylink::Silent(
							fmt::format(
								"#task complete {}",
								t.first
							),
							"Complete"
						),
						Saylink::Silent(
							fmt::format(
								"#task uncomplete {}",
								t.first
							),
							"Uncomplete"
						)
					) :
					""
				)
			).c_str()
		);

		found_count++;

		if (found_count == 50) {
			break;
		}
	}

	if (found_count == 50) {
		c->Message(
			Chat::White,
			fmt::format(
				"50 Tasks were found matching '{}', max reached.",
				sep->argplus[2]
			).c_str()
		);

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"{} Task{} found matching '{}'.",
			found_count,
			found_count != 1 ? "s" : "",
			sep->argplus[2]
		).c_str()
	);
}
