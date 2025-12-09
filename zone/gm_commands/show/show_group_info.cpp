#include "../../client.h"
#include "../../dialogue_window.h"
#include "../../groups.h"

void ShowGroupInfo(Client *c, const Seperator *sep)
{
	auto t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	auto g = t->GetGroup();
	if (!g) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} not in a group.",
				c->GetTargetDescription(t, TargetDescriptionType::UCYou),
				c == t ? "are" : "is"
			).c_str()
		);
		return;
	}

	std::string popup_table;

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Group ID") +
		DialogueWindow::TableCell(std::to_string(g->GetID()))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Members") +
		DialogueWindow::TableCell(std::to_string(g->GroupCount()))
	);

	popup_table += DialogueWindow::Break(2);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Index") +
		DialogueWindow::TableCell("Name") +
		DialogueWindow::TableCell("In Zone") +
		DialogueWindow::TableCell("Assist") +
		DialogueWindow::TableCell("Puller") +
		DialogueWindow::TableCell("Tank")
	);

	const std::string yes = DialogueWindow::ColorMessage("forest_green", "Y");
	const std::string no  = DialogueWindow::ColorMessage("red_1", "N");

	for (int group_member = 0; group_member < MAX_GROUP_MEMBERS; group_member++) {
		if (g->membername[group_member][0] == '\0') {
			continue;
		}

		const bool is_assist = g->MemberRoles[group_member] & RoleAssist;
		const bool is_puller = g->MemberRoles[group_member] & RolePuller;
		const bool is_tank   = g->MemberRoles[group_member] & RoleTank;

		popup_table += DialogueWindow::TableRow(
			fmt::format(
				"{}{}{}{}{}{}",
				DialogueWindow::TableCell(std::to_string(group_member)),
				DialogueWindow::TableCell(
					strcmp(g->membername[group_member], c->GetCleanName()) ?
					g->membername[group_member] :
					fmt::format(
						"{} (You)",
						g->membername[group_member]
					)
				),
				DialogueWindow::TableCell(g->members[group_member] ? yes : no),
				DialogueWindow::TableCell(is_assist ? yes : no),
				DialogueWindow::TableCell(is_puller ? yes : no),
				DialogueWindow::TableCell(is_tank ? yes : no)
			)
		);
	}

	popup_table = DialogueWindow::Table(popup_table);

	c->SendPopupToClient(
		fmt::format(
			"Group Info for {}",
			c->GetTargetDescription(t, TargetDescriptionType::UCSelf)
		).c_str(),
		popup_table.c_str()
	);
}
