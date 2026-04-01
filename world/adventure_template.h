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
#pragma once

#include "common/types.h"

#pragma pack(push)
#pragma pack(1)

struct AdventureTemplate
{
	uint32 id;
	char zone[64];
	uint32 zone_version;
	bool is_hard;
	int32 min_level;
	int32 max_level;
	uint8 type;
	uint32 type_data;
	uint16 type_count;
	float assa_x;
	float assa_y;
	float assa_z;
	float assa_h;
	char text[1024];
	uint32 duration;
	uint32 zone_in_time;
	int32 win_points;
	int32 lose_points;
	uint8 theme;
	uint16 zone_in_zone_id;
	float zone_in_x;
	float zone_in_y;
	uint16 zone_in_object_id;
	float dest_x;
	float dest_y;
	float dest_z;
	float dest_h;
	int graveyard_zone_id;
	float graveyard_x;
	float graveyard_y;
	float graveyard_z;
	float graveyard_radius;
};

#pragma pack(pop)
