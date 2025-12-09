#include "../../client.h"
#include "../../dialogue_window.h"

void ShowContentFlags(Client *c, const Seperator *sep)
{
	Client *t = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	std::string flags = DialogueWindow::TableRow(
		DialogueWindow::TableCell("id") +
		DialogueWindow::TableCell("flag_name") +
		DialogueWindow::TableCell("enabled")
	);

	for (auto &f: ContentFlagsRepository::All(database)) {
		flags += DialogueWindow::TableRow(
			DialogueWindow::TableCell(std::to_string(f.id)) +
			DialogueWindow::TableCell(f.flag_name) +
			DialogueWindow::TableCell(
				f.enabled ?
					DialogueWindow::ColorMessage("forest_green", "yes") :
					DialogueWindow::ColorMessage("red", "no")
			)
		);
	}

	c->SendPopupToClient("Server Content Flag Settings", DialogueWindow::Table(flags).c_str());
}
