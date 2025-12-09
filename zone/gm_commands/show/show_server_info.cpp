#include "../../client.h"
#include "../../dialogue_window.h"
#include "../../../common/serverinfo.h"

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
