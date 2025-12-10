#include "../bot_command.h"

void bot_command_class_race_list(Client* c, const Seperator* sep)
{
	const std::string class_substrs[17] = {
		"",
		"WAR", "CLR", "PAL", "RNG",
		"SHD", "DRU", "MNK", "BRD",
		"ROG", "SHM", "NEC", "WIZ",
		"MAG", "ENC", "BST", "BER"
	};

	const std::string race_substrs[17] = {
		"",
		"HUM", "BAR", "ERU", "ELF",
		"HIE", "DEF", "HEF", "DWF",
		"TRL", "OGR", "HFL", "GNM",
		"IKS", "VAH", "FRG", "DRK"
	};

	const uint16 race_values[17] = {
		Race::Doug,
		Race::Human, Race::Barbarian, Race::Erudite, Race::WoodElf,
		Race::HighElf, Race::DarkElf, Race::HalfElf, Race::Dwarf,
		Race::Troll, Race::Ogre, Race::Halfling, Race::Gnome,
		Race::Iksar, Race::VahShir, Race::Froglok2, Race::Drakkin
	};

	std::string window_text;
	std::string message_separator;
	int object_count = 0;
	const int object_max = 4;

	window_text.append(
		fmt::format(
			"<c \"#EDDA74\">Classes{}<c \"#357EC7\">",
			DialogueWindow::Break()
		)
	);

	window_text.append(
		fmt::format(
			"<c \"#D4A017\">--------------------------------------------------------------------<c \"#357EC7\">",
			DialogueWindow::Break()
		)
	);

	window_text.append(DialogueWindow::Break());

	message_separator = " ";
	object_count = 0;
	for (int i = 0; i <= 15; ++i) {
		window_text.append(message_separator);

		if (object_count >= object_max) {
			window_text.append(DialogueWindow::Break());
			object_count = 0;
		}

		window_text.append(
			fmt::format("{} ({})",
				class_substrs[i + 1],
				(i + 1)
			)
		);

		++object_count;
		message_separator = ", ";
	}

	window_text.append(DialogueWindow::Break(2));

	window_text.append(
		fmt::format(
			"<c \"#EDDA74\">Races{}<c \"#357EC7\">",
			DialogueWindow::Break()
		)
	);

	window_text.append(
		fmt::format(
			"<c \"#D4A017\">--------------------------------------------------------------------<c \"#357EC7\">",
			DialogueWindow::Break()
		)
	);

	window_text.append(DialogueWindow::Break());

	message_separator = " ";
	object_count = 0;
	for (int i = 0; i <= 15; ++i) {
		window_text.append(message_separator);

		if (object_count >= object_max) {
			window_text.append(DialogueWindow::Break());
			object_count = 0;
		}

		window_text.append(
			fmt::format("{} ({})",
				race_substrs[i + 1],
				race_values[i + 1]
			)
		);

		++object_count;
		message_separator = ", ";
	}

	c->SendPopupToClient("Bot Creation Options", window_text.c_str());

	return;
}
