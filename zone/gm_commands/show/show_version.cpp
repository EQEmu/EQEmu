#include "../../client.h"
#include "../../dialogue_window.h"

void ShowVersion(Client *c, const Seperator *sep)
{
	std::string popup_table;

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Version") +
		DialogueWindow::TableCell(CURRENT_VERSION)
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Compiled") +
		DialogueWindow::TableCell(
			fmt::format(
				"{} {}",
				COMPILE_DATE,
				COMPILE_TIME
			)
		)
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Last Modified") +
		DialogueWindow::TableCell(LAST_MODIFIED)
	);

	popup_table = DialogueWindow::Table(popup_table);

	c->SendPopupToClient(
		"Server Version",
		popup_table.c_str()
	);
}
