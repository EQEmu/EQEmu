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

#include <vector>

class Mob;
class Client;
class Group;
class Raid;

class XTargetAutoHaters {
struct HatersCount {
	int spawn_id;
	int count;
};
public:
	XTargetAutoHaters() : m_client(nullptr), m_group(nullptr), m_raid(nullptr) {}
	~XTargetAutoHaters() = default;

	void merge(XTargetAutoHaters &other);
	void demerge(XTargetAutoHaters &other);
	void increment_count(Mob *in);
	void decrement_count(Mob *in);

	bool contains_mob(int spawn_id);

	inline const std::vector<HatersCount> &get_list() { return m_haters; }
	inline void SetOwner(Client *c, Group *g, Raid *r) {m_client = c; m_group = g; m_raid = r; }
	inline void clear() { m_haters.clear(); }
	inline bool empty() { return m_haters.empty(); }

private:
	/* This will contain all of the mobs that are possible to fill in an autohater
	 * slot. This keeps track of ALL MOBS for a client or group or raid
	 * This list needs to be merged when you join group/raid/etc
	 */
	std::vector<HatersCount> m_haters;

	// So this is the object that owns us ... only 1 shouldn't be null
	Client	*m_client;
	Group	*m_group;
	Raid	*m_raid;
};
