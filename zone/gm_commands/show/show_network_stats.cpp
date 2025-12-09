#include "../../client.h"
#include "../../dialogue_window.h"

void ShowNetworkStats(Client *c, const Seperator *sep)
{
	const auto connection = c->Connection();
	const auto opts       = connection->GetManager()->GetOptions();
	const auto eqs_stats  = connection->GetStats();

	const auto& stats = eqs_stats.ReliableStreamStats;

	const auto sec_since_stats_reset = std::chrono::duration_cast<std::chrono::duration<double>>(
		EQ::Net::Clock::now() - stats.created
	).count();

	std::string popup_table;

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Sent Bytes") +
		DialogueWindow::TableCell(
			fmt::format(
				"{} ({:.2f} Per Second)",
				Strings::Commify(stats.sent_bytes),
				stats.sent_bytes / sec_since_stats_reset
			)
		)
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Received Bytes") +
		DialogueWindow::TableCell(
			fmt::format(
				"{} ({:.2f} Per Second)",
				Strings::Commify(stats.recv_bytes),
				stats.recv_bytes / sec_since_stats_reset
			)
		)
	);

	popup_table += DialogueWindow::Break(2);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Sent Bytes Before Encode") +
		DialogueWindow::TableCell(Strings::Commify(stats.bytes_before_encode)) +
		DialogueWindow::TableCell("Compression Rate") +
		DialogueWindow::TableCell(
			fmt::format(
				"{:.2f}%%",
				static_cast<double>(stats.bytes_before_encode - stats.sent_bytes) /
				static_cast<double>(stats.bytes_before_encode) * 100.0
			)
		)
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Sent Bytes After Encode") +
		DialogueWindow::TableCell(Strings::Commify(stats.bytes_after_decode)) +
		DialogueWindow::TableCell("Compression Rate") +
		DialogueWindow::TableCell(
			fmt::format(
				"{:.2f}%%",
				static_cast<double>(stats.bytes_after_decode - stats.recv_bytes) /
				static_cast<double>(stats.bytes_after_decode) * 100.0
			)
		)
	);

	popup_table += DialogueWindow::Break(2);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Minimum Ping") +
		DialogueWindow::TableCell(Strings::Commify(stats.min_ping))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Maximum Ping") +
		DialogueWindow::TableCell(Strings::Commify(stats.max_ping))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Last Ping") +
		DialogueWindow::TableCell(Strings::Commify(stats.last_ping))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Average Ping") +
		DialogueWindow::TableCell(Strings::Commify(stats.avg_ping))
	);

	popup_table += DialogueWindow::Break(2);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Real Time Received Packets") +
		DialogueWindow::TableCell(
			fmt::format(
				"{} ({:.2f} Per Second)",
				Strings::Commify(stats.recv_packets),
				stats.recv_packets / sec_since_stats_reset
			)
		)
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Real Time Sent Packets") +
		DialogueWindow::TableCell(
			fmt::format(
				"{} ({:.2f} Per Second)",
				Strings::Commify(stats.sent_packets),
				stats.sent_packets / sec_since_stats_reset
			)
		)
	);

	popup_table += DialogueWindow::Break(2);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Sync Received Packets") +
		DialogueWindow::TableCell(Strings::Commify(stats.sync_recv_packets))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Sync Sent Packets") +
		DialogueWindow::TableCell(Strings::Commify(stats.sync_sent_packets))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Sync Remote Received Packets") +
		DialogueWindow::TableCell(Strings::Commify(stats.sync_remote_recv_packets))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Sync Remote Sent Packets") +
		DialogueWindow::TableCell(Strings::Commify(stats.sync_remote_sent_packets))
	);

	popup_table += DialogueWindow::Break(2);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Packet Loss In") +
		DialogueWindow::TableCell(
			fmt::format(
				"{:.2f}%%",
				(
					100.0 *
					(
						1.0 -
						static_cast<double>(stats.sync_recv_packets) /
						static_cast<double>(stats.sync_remote_sent_packets)
					)
				)
			)
		)
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Packet Loss Out") +
		DialogueWindow::TableCell(
			fmt::format(
				"{:.2f}%%",
				(
					100.0 *
					(
						1.0 -
						static_cast<double>(stats.sync_remote_recv_packets) /
						static_cast<double>(stats.sync_sent_packets)
					)
				)
			)
		)
	);

	popup_table += DialogueWindow::Break(2);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Resent Packets") +
		DialogueWindow::TableCell(
			fmt::format(
				"{} ({:.2f} Per Second)",
				Strings::Commify(stats.resent_packets),
				stats.resent_packets / sec_since_stats_reset
			)
		)
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Resent Fragments") +
		DialogueWindow::TableCell(
			fmt::format(
				"{} ({:.2f} Per Second)",
				Strings::Commify(stats.resent_fragments),
				stats.resent_fragments / sec_since_stats_reset
			)
		)
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Resent Non-Fragments") +
		DialogueWindow::TableCell(
			fmt::format(
				"{} ({:.2f} Per Second)",
				Strings::Commify(stats.resent_full),
				stats.resent_full / sec_since_stats_reset
			)
		)
	);

	popup_table += DialogueWindow::Break(2);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Dropped Datarate Packets") +
		DialogueWindow::TableCell(
			fmt::format(
				"{} ({:.2f} Per Second)",
				Strings::Commify(stats.dropped_datarate_packets),
				stats.dropped_datarate_packets / sec_since_stats_reset
			)
		)
	);

	if (opts.reliable_stream_options.outgoing_data_rate > 0.0) {
		popup_table += DialogueWindow::TableRow(
			DialogueWindow::TableCell("Outgoing Link Saturation") +
			DialogueWindow::TableCell(
				fmt::format(
					"{:.2f}%% ({:.2f}kb Per Second)",
					(
						100.0 *
						(
							1.0 -
							(
								(
									opts.reliable_stream_options.outgoing_data_rate -
									stats.datarate_remaining
								) /
								opts.reliable_stream_options.outgoing_data_rate
							)
						)
					),
					opts.reliable_stream_options.outgoing_data_rate
				)
			)
		);
	}

	popup_table += DialogueWindow::Break(2);

	std::string sent_rows;

	for (int i = 0; i < _maxEmuOpcode; ++i) {
		const int count = eqs_stats.SentCount[i];
		if (count) {
			sent_rows += DialogueWindow::TableRow(
				DialogueWindow::TableCell(OpcodeNames[i]) +
				DialogueWindow::TableCell(
					fmt::format(
						"{} ({:.2f} Per Second)",
						Strings::Commify(count),
						count / sec_since_stats_reset
					)
				)
			);
		}
	}

	std::string recv_rows;

	for (int i = 0; i < _maxEmuOpcode; ++i) {
		const int count = eqs_stats.RecvCount[i];
		if (count) {
			recv_rows += DialogueWindow::TableRow(
				DialogueWindow::TableCell(OpcodeNames[i]) +
				DialogueWindow::TableCell(
					fmt::format(
						"{} ({:.2f} Per Second)",
						Strings::Commify(count),
						count / sec_since_stats_reset
					)
				)
			);
		}
	}

	popup_table += DialogueWindow::TableRow(DialogueWindow::TableCell("Sent Packet Types"));

	popup_table += sent_rows;

	popup_table += DialogueWindow::TableRow(DialogueWindow::TableCell("Received Packet Types"));

	popup_table += recv_rows;

	popup_table = DialogueWindow::Table(popup_table);

	c->SendPopupToClient(
		"Network Statistics",
		popup_table.c_str()
	);
}
