#include "../../client.h"
#include "../../dialogue_window.h"

void ShowSkills(Client *c, const Seperator *sep)
{
	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	std::string popup_table;

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("ID") +
		DialogueWindow::TableCell("Name") +
		DialogueWindow::TableCell("Current") +
		DialogueWindow::TableCell("Max") +
		DialogueWindow::TableCell("Raw")
	);

	for (const auto& s : EQ::skills::GetSkillTypeMap()) {
		if (t->CanHaveSkill(s.first) && t->MaxSkill(s.first)) {
			popup_table += DialogueWindow::TableRow(
				DialogueWindow::TableCell(std::to_string(s.first)) +
				DialogueWindow::TableCell(s.second) +
				DialogueWindow::TableCell(std::to_string(t->GetSkill(s.first))) +
				DialogueWindow::TableCell(std::to_string(t->MaxSkill(s.first))) +
				DialogueWindow::TableCell(std::to_string(t->GetRawSkill(s.first)))
			);
		}
	}

	popup_table = DialogueWindow::Table(popup_table);

	c->SendPopupToClient(
		fmt::format(
			"Skills for {}",
			c->GetTargetDescription(t, TargetDescriptionType::UCSelf)
		).c_str(),
		popup_table.c_str()
	);
}
