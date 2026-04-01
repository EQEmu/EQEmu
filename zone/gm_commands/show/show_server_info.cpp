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
#include "common/serverinfo.h"
#include "zone/client.h"
#include "zone/dialogue_window.h"

void ShowServerInfo(Client *c, const Seperator *sep)
{
	auto         os         = EQ::GetOS();
	auto         cpus       = EQ::GetCPUs();
	const uint32 process_id = EQ::GetPID();
	const double rss        = EQ::GetRSS() / 1048576.0;
	const uint32 uptime     = static_cast<uint32>(EQ::GetUptime());

	std::string popup_table;

	std::string popup_text;

	popup_text += DialogueWindow::CenterMessage(
		DialogueWindow::ColorMessage("green", "Operating System Information")
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Machine") +
		DialogueWindow::TableCell(os.machine)
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("System") +
		DialogueWindow::TableCell(os.sysname)
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Release") +
		DialogueWindow::TableCell(os.release)
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Uptime") +
		DialogueWindow::TableCell(Strings::SecondsToTime(uptime))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Version") +
		DialogueWindow::TableCell(os.version)
	);

	popup_text += DialogueWindow::Table(popup_table);

	popup_table = std::string();

	popup_text += DialogueWindow::Break();

	popup_text += DialogueWindow::CenterMessage(
		DialogueWindow::ColorMessage("green", "CPU Information")
	);

	for (size_t cpu = 0; cpu < cpus.size(); ++cpu) {
		auto &current_cpu = cpus[cpu];
		popup_table += DialogueWindow::TableRow(
			DialogueWindow::TableCell(
				fmt::format(
					"CPU {}",
					cpu
				)
			) +
			DialogueWindow::TableCell(
				fmt::format(
					"{} ({:.2f}GHz)",
					current_cpu.model,
					current_cpu.speed
				)
			)
		);
	}

	popup_text += DialogueWindow::Table(popup_table);

	popup_table = std::string();

	popup_text += DialogueWindow::Break();

	popup_text += DialogueWindow::CenterMessage(
		DialogueWindow::ColorMessage("green", "CPU Information")
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Process ID") +
		DialogueWindow::TableCell(Strings::Commify(process_id))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("RSS") +
		DialogueWindow::TableCell(
			fmt::format(
				"{:.2f} MB",
				rss
			)
		)
	);

	popup_text += DialogueWindow::Table(popup_table);

	c->SendPopupToClient(
		"Server Information",
		popup_text.c_str()
	);
}
