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
#include "zone/water_map.h"

void command_loc(Client *c, const Seperator *sep)
{
	Mob *target = c;
	if (c->GetTarget()) {
		target = c->GetTarget();
	}

	auto target_position = target->GetPosition();

	// check los benchmark
	BenchTimer timer;
	for (int i = 0; i < 1000; i++) {
		zone->zonemap->CheckLoS(c->GetPosition(), target_position);
	}
	c->Message(
		Chat::White,
		fmt::format(
			"CheckLoS benchmark took [{}]",
			timer.elapsed()
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Location for {} | XYZ: {:.2f}, {:.2f}, {:.2f} Heading: {:.2f}",
			c->GetTargetDescription(target, TargetDescriptionType::UCSelf),
			target_position.x,
			target_position.y,
			target_position.z,
			target_position.w
		).c_str()
	);

	if (!zone->zonemap) {
		c->Message(Chat::White, "Map not loaded for this zone.");
	} else {
		auto z = target->GetZ() + (target->GetSize() == 0.0 ? 6 : target->GetSize()) * HEAD_POSITION;
		auto tarloc = glm::vec3(target->GetX(), target->GetY(), z);
		glm::vec3 hit;

		auto best_z = zone->zonemap->FindBestZ(tarloc, &hit);
		auto fixed_z = c->GetFixedZ(c->GetPosition());

		if (best_z != BEST_Z_INVALID) {
			c->Message(
				Chat::White,
				fmt::format(
					"Best Z for {} | {:.2f}",
					c->GetTargetDescription(target, TargetDescriptionType::UCSelf),
					best_z
				).c_str()
			);
			c->Message(
				Chat::White,
				fmt::format(
					"Fixed Z for {} | {:.2f}",
					c->GetTargetDescription(target, TargetDescriptionType::UCSelf),
					fixed_z
				).c_str()
			);
		} else {
			c->Message(Chat::White, "Could not find Best Z.");
		}
	}

	if (!zone->watermap) {
		c->Message(Chat::White, "Water Map not loaded for this zone.");
	} else {
		auto position = glm::vec3(target->GetX(), target->GetY(), target->GetZ());
		auto region_type = zone->watermap->ReturnRegionType(position);
		auto position_string = fmt::format(
			"{} {}",
			c->GetTargetDescription(target, TargetDescriptionType::UCYou),
			c == target ? "are" : "is"
		);

		switch (region_type) {
			case RegionTypeIce: {
				c->Message(
					Chat::White,
					fmt::format(
						"{} in Ice.",
						position_string
					).c_str()
				);
				break;
			}
			case RegionTypeLava: {
				c->Message(
					Chat::White,
					fmt::format(
						"{} in Lava.",
						position_string
					).c_str()
				);
				break;
			}
			case RegionTypeNormal: {
				c->Message(
					Chat::White,
					fmt::format(
						"{} in a Normal Region.",
						position_string
					).c_str()
				);
				break;
			}
			case RegionTypePVP: {
				c->Message(
					Chat::White,
					fmt::format(
						"{} in a PvP Area.",
						position_string
					).c_str()
				);
				break;
			}
			case RegionTypeSlime: {
				c->Message(
					Chat::White,
					fmt::format(
						"{} in Slime.",
						position_string
					).c_str()
				);
				break;
			}
			case RegionTypeVWater: {
				c->Message(
					Chat::White,
					fmt::format(
						"{} in VWater (Icy Water?).",
						position_string
					).c_str()
				);
				break;
			}
			case RegionTypeWater: {
				c->Message(
					Chat::White,
					fmt::format(
						"{} in Water.",
						position_string
					).c_str()
				);
				break;
			}
			default: {
				c->Message(
					Chat::White,
					fmt::format(
						"{} in an Unknown Region.",
						position_string
					).c_str()
				);
				break;
			}
		}
	}
}

