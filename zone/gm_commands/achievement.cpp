#include "common/say_link.h"
#include "common/strings.h"
#include "zone/achievement_manager.h"
#include "zone/client.h"

namespace
{

const char *AchievementStatusName(int status)
{
	using EQ::Achievements::Status;
	switch (static_cast<Status>(status)) {
	case Status::Completed: return "Completed";
	case Status::Open: return "Open";
	case Status::Locked: return "Locked";
	case Status::Hidden: return "Hidden";
	}
	return "Unavailable";
}

void SendAchievementCommandHelp(Client *client)
{
	client->Message(Chat::White, "Usage: #achievement find <name>");
	client->Message(Chat::White, "Usage: #achievement list [all|completed|open|locked|hidden] [limit]");
	client->Message(Chat::White, "Usage: #achievement inspect <achievement_id>");
	client->Message(Chat::White, "Usage: #achievement set <achievement_id> <component_type> <component_id> <value>");
	client->Message(Chat::White, "Usage: #achievement add <achievement_id> <component_type> <component_id> <amount>");
	client->Message(Chat::White, "Usage: #achievement complete <achievement_id>");
	client->Message(Chat::White, "Usage: #achievement reset <achievement_id> [rewards]");
	client->Message(Chat::White, "Set, add, and complete target your selected player, or yourself when no player is selected.");
	client->Message(Chat::White, "Reset preserves reward ledgers unless the explicit rewards argument is supplied.");
}

Client *AchievementCommandTarget(Client *client)
{
	if (client->GetTarget() && client->GetTarget()->IsClient()) {
		return client->GetTarget()->CastToClient();
	}
	return client;
}

} // namespace

void command_achievement(Client *c, const Seperator *sep)
{
	if (!RuleB(Achievements, EnableAchievements)) {
		c->Message(Chat::White, "The achievement system is disabled.");
		return;
	}
	if (!sep->argnum || Strings::EqualFold(sep->arg[1], "help")) {
		SendAchievementCommandHelp(c);
		return;
	}

	auto &manager = AchievementManager::Instance();
	if (!manager.IsLoaded()) {
		c->Message(Chat::White, "Achievement data is not loaded in this zone.");
		return;
	}

	Client *target = AchievementCommandTarget(c);
	const auto action = Strings::ToLower(sep->arg[1]);

	if (action == "find") {
		if (sep->argnum < 2 || !sep->argplus[2][0]) {
			SendAchievementCommandHelp(c);
			return;
		}
		const auto search = Strings::ToLower(sep->argplus[2]);
		uint32_t matches = 0;
		for (const auto &definition : manager.Definitions()) {
			if (!Strings::Contains(Strings::ToLower(definition.name), search)) {
				continue;
			}
			c->Message(
				Chat::White,
				fmt::format(
					"{} (ID {}) [{}]",
					Saylink::Silent(
						fmt::format("#achievement inspect {}", definition.achievement_id),
						definition.name
					),
					definition.achievement_id,
					AchievementStatusName(target->GetAchievementStatus(definition.achievement_id))
				).c_str()
			);
			if (++matches >= 50) {
				break;
			}
		}
		c->Message(
			Chat::White,
			fmt::format("Found {} matching achievement{}.", matches, matches == 1 ? "" : "s").c_str()
		);
		return;
	}

	if (action == "list") {
		const auto filter = sep->argnum >= 2 ? Strings::ToLower(sep->arg[2]) : "all";
		if (
			filter != "all" && filter != "completed" && filter != "open" &&
			filter != "locked" && filter != "hidden"
		) {
			SendAchievementCommandHelp(c);
			return;
		}
		const auto limit = static_cast<uint32_t>(std::clamp(
			sep->IsNumber(3) ? Strings::ToInt(sep->arg[3]) : 50,
			1,
			200
		));
		uint32_t shown = 0;
		uint32_t total = 0;
		for (const auto &definition : manager.Definitions()) {
			const auto status = target->GetAchievementStatus(definition.achievement_id);
			const auto status_name = Strings::ToLower(AchievementStatusName(status));
			if (filter != "all" && filter != status_name) {
				continue;
			}
			++total;
			if (shown >= limit) {
				continue;
			}
			c->Message(
				Chat::White,
				fmt::format(
					"{} (ID {}) [{}]",
					Saylink::Silent(
						fmt::format("#achievement inspect {}", definition.achievement_id),
						definition.name
					),
					definition.achievement_id,
					AchievementStatusName(status)
				).c_str()
			);
			++shown;
		}
		c->Message(
			Chat::White,
			fmt::format(
				"Showing {} of {} {} achievement{} for {}.",
				shown,
				total,
				filter,
				total == 1 ? "" : "s",
				c->GetTargetDescription(target)
			).c_str()
		);
		return;
	}

	if (sep->argnum < 2 || !sep->IsNumber(2)) {
		SendAchievementCommandHelp(c);
		return;
	}
	const auto achievement_id = Strings::ToUnsignedInt(sep->arg[2]);
	const auto definition = manager.FindDefinition(achievement_id);
	if (!definition) {
		c->Message(Chat::White, fmt::format("Achievement ID {} is not loaded.", achievement_id).c_str());
		return;
	}

	if (action == "inspect") {
		const auto status = target->GetAchievementStatus(achievement_id);
		c->Message(
			Chat::White,
			fmt::format(
				"{} (ID {}) for {}: {} | {} points | {} automatic reward row{} | {} selectable reward set",
				definition->name,
				achievement_id,
				c->GetTargetDescription(target),
				AchievementStatusName(status),
				definition->points,
				manager.Rewards(achievement_id).size(),
				manager.Rewards(achievement_id).size() == 1 ? "" : "s",
				manager.RewardSet(achievement_id) ? "has" : "no"
			).c_str()
		);
		for (uint8_t component_type = 0; component_type < 4; ++component_type) {
			for (const auto &component : definition->components[component_type]) {
				const auto progress = component_type < 3
					? target->GetAchievementProgress(achievement_id, component_type, component.component_id)
					: -1;
				c->Message(
					Chat::White,
					fmt::format(
						"Type {} | Component {} | {}/{} | {}{}",
						component_type,
						component.component_id,
						progress >= 0 ? std::to_string(progress) : "display-only",
						component.required_count,
						component.name,
						component.description.empty() ? "" : fmt::format(" ({})", component.description)
					).c_str()
				);
			}
		}
		return;
	}

	if (action == "complete") {
		if (target->HasCompletedAchievement(achievement_id)) {
			c->Message(
				Chat::White,
				fmt::format(
					"{} has already completed {} (ID {}).",
					c->GetTargetDescription(target),
					definition->name,
					achievement_id
				).c_str()
			);
			return;
		}
		const auto succeeded = target->CompleteAchievement(achievement_id);
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} (ID {}) for {}.",
				succeeded ? "Completed" : "Failed to complete",
				definition->name,
				achievement_id,
				c->GetTargetDescription(target)
			).c_str()
		);
		return;
	}

	if (action == "reset") {
		const bool reset_rewards = sep->argnum >= 3 && Strings::EqualFold(sep->arg[3], "rewards");
		if (sep->argnum >= 3 && !reset_rewards) {
			SendAchievementCommandHelp(c);
			return;
		}
		const auto succeeded = target->ResetAchievement(achievement_id, reset_rewards);
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} (ID {}) for {}{}.",
				succeeded ? "Reset" : "Failed to reset",
				definition->name,
				achievement_id,
				c->GetTargetDescription(target),
				reset_rewards ? ", including reward ledgers" : "; reward ledgers were preserved"
			).c_str()
		);
		return;
	}

	if (action == "set" || action == "add") {
		if (
			sep->argnum < 5 || !sep->IsNumber(3) || !sep->IsNumber(4) ||
			!sep->IsNumber(5)
		) {
			SendAchievementCommandHelp(c);
			return;
		}
		if (target->HasCompletedAchievement(achievement_id)) {
			c->Message(Chat::White, "Reset the completed achievement before changing its component progress.");
			return;
		}
		const auto component_type = Strings::ToUnsignedInt(sep->arg[3]);
		const auto component_id = Strings::ToUnsignedInt(sep->arg[4]);
		const auto value = Strings::ToUnsignedInt(sep->arg[5]);
		if (
			component_type > 2 ||
			!manager.FindComponentIndex(achievement_id, component_type, component_id)
		) {
			c->Message(Chat::White, "The component type or component ID is invalid for this achievement.");
			return;
		}
		const bool additive = action == "add";
		const auto succeeded = target->SetAchievementProgress(
			achievement_id,
			static_cast<uint8_t>(component_type),
			component_id,
			value,
			additive
		);
		c->Message(
			Chat::White,
			fmt::format(
				"{} progress for {} (ID {}), type {}, component {}, value {} for {}.",
				succeeded ? (additive ? "Added" : "Set") : "Failed to update",
				definition->name,
				achievement_id,
				component_type,
				component_id,
				value,
				c->GetTargetDescription(target)
			).c_str()
		);
		return;
	}

	SendAchievementCommandHelp(c);
}
