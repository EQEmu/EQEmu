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

#include <list>

class NPC;
class Client;
class Zone;

struct QGlobal
{
	QGlobal() { }
	QGlobal(std::string g_name, uint32 c_id, uint32 n_id, uint32 z_id, std::string n_value, uint32 expire_date)
		: name(g_name), char_id(c_id), npc_id(n_id), zone_id(z_id), value(n_value), expdate(expire_date) { id = 0; }
	std::string name;
	uint32 char_id;
	uint32 npc_id;
	uint32 zone_id;
	std::string value;
	uint32 expdate;
	uint32 id;
};

class QGlobalCache
{
public:
	void AddGlobal(uint32 id, QGlobal global);
	void RemoveGlobal(std::string name, uint32 npcID, uint32 charID, uint32 zoneID);
	std::list<QGlobal> GetBucket() { return qGlobalBucket; }

	//assumes cacheA is already a valid or empty list and doesn't check for valid items.
	static void Combine(std::list<QGlobal> &cacheA, std::list<QGlobal> cacheB, uint32 npcID, uint32 charID, uint32 zoneID);
	static void GetQGlobals(std::list<QGlobal> &globals, NPC *n, Client *c, Zone *z);

	void PurgeExpiredGlobals();
	void LoadByNPCID(uint32 npcID); //npc
	void LoadByCharID(uint32 charID); //client
	void LoadByZoneID(uint32 zoneID); //zone
	void LoadByGlobalContext(); //zone
protected:
	void LoadBy(const std::string &query);
	std::list<QGlobal> qGlobalBucket;
};
