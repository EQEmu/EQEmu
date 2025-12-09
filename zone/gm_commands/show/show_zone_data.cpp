#include "../../client.h"
#include "../../dialogue_window.h"

void ShowZoneData(Client *c, const Seperator *sep)
{
	std::string popup_table;

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Type") +
		DialogueWindow::TableCell(std::to_string(zone->newzone_data.ztype))
	);

	for (uint8 fog_index = 0; fog_index < 4; fog_index++) {
		const uint8 fog_number = (fog_index + 1);

		popup_table += DialogueWindow::TableRow(
			DialogueWindow::TableCell(
				fmt::format(
					"Fog {} Colors",
					fog_number
				)
			) +
			DialogueWindow::TableCell(
				fmt::format(
					"{} {} {}",
					DialogueWindow::ColorMessage(
						"red_1",
						std::to_string(zone->newzone_data.fog_red[fog_index])
					),
					DialogueWindow::ColorMessage(
						"forest_green",
						std::to_string(zone->newzone_data.fog_green[fog_index])
					),
					DialogueWindow::ColorMessage(
						"royal_blue",
						std::to_string(zone->newzone_data.fog_blue[fog_index])
					)
				)
			)
		);

		popup_table += DialogueWindow::TableRow(
			DialogueWindow::TableCell(
				fmt::format(
					"Fog {} Clipping",
					fog_number
				)
			) +
			DialogueWindow::TableCell(
				fmt::format(
					"{} to {}",
					zone->newzone_data.fog_minclip[fog_index],
					zone->newzone_data.fog_maxclip[fog_index]
				)
			)
		);
	}

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Fog Density") +
		DialogueWindow::TableCell(
			fmt::format(
				"{:.2f}",
				zone->newzone_data.fog_density
			)
		)
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Gravity") +
		DialogueWindow::TableCell(
			fmt::format(
				"{:.2f}",
				zone->newzone_data.gravity
			)
		)
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Time Type") +
		DialogueWindow::TableCell(std::to_string(zone->newzone_data.time_type))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Experience Multiplier") +
		DialogueWindow::TableCell(
			fmt::format(
				"{:.2f}%%",
				(zone->newzone_data.zone_exp_multiplier * 100)
			)
		)
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Safe Coordinates") +
		DialogueWindow::TableCell(
			fmt::format(
				"{:.2f}, {:.2f}, {:.2f} Heading: {:.2f}",
				zone->newzone_data.safe_x,
				zone->newzone_data.safe_y,
				zone->newzone_data.safe_z,
				zone->newzone_data.safe_heading
			)
		)
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Max Z") +
		DialogueWindow::TableCell(
			fmt::format(
				"{:.2f}",
				zone->newzone_data.max_z
			)
		)
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Underworld Z") +
		DialogueWindow::TableCell(
			fmt::format(
				"{:.2f}",
				zone->newzone_data.underworld
			)
		)
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Clipping Distance") +
		DialogueWindow::TableCell(
			fmt::format(
				"{} to {}",
				zone->newzone_data.minclip,
				zone->newzone_data.maxclip
			)
		)
	);

	// Weather Data
	for (uint8 weather_index = 0; weather_index < 4; weather_index++) {
		const uint8 weather_number = (weather_index + 1);

		if (
			zone->newzone_data.rain_chance[weather_index] ||
			zone->newzone_data.rain_duration[weather_index]
		) {
			popup_table += DialogueWindow::TableRow(
				DialogueWindow::TableCell(
					fmt::format(
						"Rain {}",
						weather_number
					)
				) +
				DialogueWindow::TableCell(
					fmt::format(
						"Chance: {}",
						zone->newzone_data.rain_chance[weather_index]
					)
				) +
				DialogueWindow::TableCell(
					fmt::format(
						"Duration: {}",
						zone->newzone_data.rain_duration[weather_index]
					)
				)
			);
		}

		if (
			zone->newzone_data.snow_chance[weather_index] ||
			zone->newzone_data.snow_duration[weather_index]
		) {
			popup_table += DialogueWindow::TableRow(
				DialogueWindow::TableCell(
					fmt::format(
						"Snow {}",
						weather_number
					)
				) +
				DialogueWindow::TableCell(
					fmt::format(
						"Chance: {}",
						zone->newzone_data.snow_chance[weather_index]
					)
				) +
				DialogueWindow::TableCell(
					fmt::format(
						"Duration: {}",
						zone->newzone_data.snow_duration[weather_index]
					)
				)
			);
		}
	}

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Sky") +
		DialogueWindow::TableCell(std::to_string(zone->newzone_data.sky))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Suspend Buffs") +
		DialogueWindow::TableCell(
			zone->newzone_data.suspend_buffs ?
			DialogueWindow::ColorMessage("forest_green", "Y") :
			DialogueWindow::ColorMessage("red_1", "N")
		)
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Health Regen") +
		DialogueWindow::TableCell(
			fmt::format(
				"{} ({})",
				Strings::Commify(zone->newzone_data.fast_regen_hp),
				Strings::SecondsToTime(zone->newzone_data.fast_regen_hp)
			)
		)
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Mana Regen") +
		DialogueWindow::TableCell(
			fmt::format(
				"{} ({})",
				Strings::Commify(zone->newzone_data.fast_regen_mana),
				Strings::SecondsToTime(zone->newzone_data.fast_regen_mana)
			)
		)
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Endurance Regen") +
		DialogueWindow::TableCell(
			fmt::format(
				"{} ({})",
				Strings::Commify(zone->newzone_data.fast_regen_endurance),
				Strings::SecondsToTime(zone->newzone_data.fast_regen_endurance)
			)
		)
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Maximum Aggro Distance") +
		DialogueWindow::TableCell(Strings::Commify(zone->newzone_data.npc_aggro_max_dist))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Underworld Teleport Index") +
		DialogueWindow::TableCell(Strings::Commify(zone->newzone_data.underworld_teleport_index))
	);

	popup_table += DialogueWindow::TableRow(
		DialogueWindow::TableCell("Lava Damage") +
		DialogueWindow::TableCell(
			fmt::format(
				"{} to {}",
				Strings::Commify(zone->newzone_data.min_lava_damage),
				Strings::Commify(zone->newzone_data.lava_damage)
			)
		)
	);

	popup_table = DialogueWindow::Table(popup_table);

	c->SendPopupToClient(
		fmt::format(
			"Zone Data for {}",
			zone->GetZoneDescription()
		).c_str(),
		popup_table.c_str()
	);
}
