#include "../../client.h"
#include "../../dialogue_window.h"

void ShowNetwork(Client *c, const Seperator *sep)
{
	auto eqsi    = c->Connection();
	auto manager = eqsi->GetManager();
	auto opts    = manager->GetOptions();

	std::string popup_table;

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Option") +
		DialogueWindow::TableCell("Value")
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Max Packet Size") +
		DialogueWindow::TableCell(Strings::Commify(opts.reliable_stream_options.max_packet_size))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Max Connection Count") +
		DialogueWindow::TableCell(Strings::Commify(opts.reliable_stream_options.max_connection_count))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Keep Alive Delay") +
		DialogueWindow::TableCell(Strings::MillisecondsToTime(opts.reliable_stream_options.keepalive_delay_ms))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Resend Delay Factor") +
		DialogueWindow::TableCell(
			fmt::format(
				"{:.2f}",
				opts.reliable_stream_options.resend_delay_factor
			)
		)
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Resend Delay") +
		DialogueWindow::TableCell(Strings::MillisecondsToTime(opts.reliable_stream_options.resend_delay_ms))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Resend Delay Minimum") +
		DialogueWindow::TableCell(Strings::MillisecondsToTime(opts.reliable_stream_options.resend_delay_min))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Resend Delay Maximum") +
		DialogueWindow::TableCell(Strings::MillisecondsToTime(opts.reliable_stream_options.resend_delay_max))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Connect Delay") +
		DialogueWindow::TableCell(Strings::MillisecondsToTime(opts.reliable_stream_options.connect_delay_ms))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Connect Stale") +
		DialogueWindow::TableCell(Strings::MillisecondsToTime(opts.reliable_stream_options.connect_stale_ms))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Stale Connection") +
		DialogueWindow::TableCell(Strings::MillisecondsToTime(opts.reliable_stream_options.stale_connection_ms))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("CRC Length") +
		DialogueWindow::TableCell(Strings::Commify(opts.reliable_stream_options.crc_length))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Hold Size") +
		DialogueWindow::TableCell(Strings::Commify(opts.reliable_stream_options.hold_size))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Hold Length") +
		DialogueWindow::TableCell(Strings::MillisecondsToTime(opts.reliable_stream_options.hold_length_ms))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Simulated In Packet Loss") +
		DialogueWindow::TableCell(std::to_string(opts.reliable_stream_options.simulated_in_packet_loss))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Simulated Out Packet Loss") +
		DialogueWindow::TableCell(std::to_string(opts.reliable_stream_options.simulated_out_packet_loss))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Tic Rate (Hz)") +
		DialogueWindow::TableCell(
			fmt::format(
				"{:.2f}",
				opts.reliable_stream_options.tic_rate_hertz
			)
		)
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Resend Timeout") +
		DialogueWindow::TableCell(Strings::MillisecondsToTime(opts.reliable_stream_options.resend_timeout))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Connection Close Time") +
		DialogueWindow::TableCell(Strings::MillisecondsToTime(opts.reliable_stream_options.connection_close_time))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Encode Passes (1)") +
		DialogueWindow::TableCell(Strings::Commify(opts.reliable_stream_options.encode_passes[0]))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Encode Passes (2)") +
		DialogueWindow::TableCell(Strings::Commify(opts.reliable_stream_options.encode_passes[1]))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Port") +
		DialogueWindow::TableCell(Strings::Commify(opts.reliable_stream_options.port))
	);

	popup_table = DialogueWindow::Table(popup_table);

	c->SendPopupToClient(
		"Network Information",
		popup_table.c_str()
	);
}
