/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "zone/client.h"
#include "zone/dialogue_window.h"

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
