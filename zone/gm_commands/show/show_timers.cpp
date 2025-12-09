#include "../../client.h"
#include "../../dialogue_window.h"

void ShowTimers(Client *c, const Seperator *sep)
{
	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	std::vector<std::pair<pTimerType, PersistentTimer *>> l;
	t->GetPTimers().ToVector(l);

	if (l.empty()) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} no recast timers.",
				c->GetTargetDescription(t, TargetDescriptionType::UCYou),
				c == t ? "have" : "has"
			).c_str()
		);
		return;
	}

	std::string popup_table;

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Timer ID") +
		DialogueWindow::TableCell("Remaining Time")
	);

	for (const auto& e : l) {
		const uint32 remaining_time = e.second->GetRemainingTime();
		if (remaining_time) {
			popup_table += DialogueWindow::TableRow(
				DialogueWindow::TableCell(std::to_string(e.first)) +
				DialogueWindow::TableCell(Strings::SecondsToTime(remaining_time))
			);
		}
	}

	popup_table = DialogueWindow::Table(popup_table);

	c->SendPopupToClient(
		fmt::format(
			"Recast Timers for {}",
			c->GetTargetDescription(t, TargetDescriptionType::UCSelf)
		).c_str(),
		popup_table.c_str()
	);
}
