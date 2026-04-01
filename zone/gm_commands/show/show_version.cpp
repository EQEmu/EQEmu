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
