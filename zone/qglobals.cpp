#include "qglobals.h"

#include "common/strings.h"
#include "zone/client.h"
#include "zone/zone.h"

void QGlobalCache::AddGlobal(uint32 id, QGlobal global)
{
	global.id = id;
	qGlobalBucket.push_back(global);
}

void QGlobalCache::RemoveGlobal(std::string name, uint32 npcID, uint32 charID, uint32 zoneID)
{
	auto iter = qGlobalBucket.begin();
	while(iter != qGlobalBucket.end())
	{
		if(name.compare((*iter).name) == 0)
		{
			if((npcID == (*iter).npc_id || (*iter).npc_id == 0) &&
				(charID == (*iter).char_id || (*iter).char_id == 0) &&
				(zoneID == (*iter).zone_id || (*iter).zone_id == 0))
			{
				qGlobalBucket.erase(iter);
				return;
			}
		}
		++iter;
	}
}

void QGlobalCache::Combine(std::list<QGlobal> &cacheA, std::list<QGlobal> cacheB, uint32 npcID, uint32 charID, uint32 zoneID)
{
	auto iter = cacheB.begin();
	while(iter != cacheB.end())
	{
		QGlobal cur = (*iter);

		if((cur.npc_id == npcID || cur.npc_id == 0) && (cur.char_id == charID || cur.char_id == 0) &&
			(cur.zone_id == zoneID || cur.zone_id == 0))
		{
			if(Timer::GetTimeSeconds() < cur.expdate)
			{
				cacheA.push_back(cur);
			}
		}
		++iter;
	}
}

void QGlobalCache::GetQGlobals(std::list<QGlobal> &globals, NPC *n, Client *c, Zone *z) {
	globals.clear();

	QGlobalCache *npc_c = nullptr;
	QGlobalCache *char_c = nullptr;
	QGlobalCache *zone_c = nullptr;
	uint32 npc_id = 0;
	uint32 char_id = 0;
	uint32 zone_id = 0;

	if(n) {
		npc_id = n->GetNPCTypeID();
		npc_c = n->GetQGlobals();
	}

	if(c) {
		char_id = c->CharacterID();
		char_c = c->GetQGlobals();
	}

	if(z) {
		zone_id = z->GetZoneID();
		zone_c = z->GetQGlobals();
	}

	if(!npc_c && n) {
		npc_c = n->CreateQGlobals();
		npc_c->LoadByNPCID(npc_id);
	}

	if(!char_c && c) {
		char_c = c->CreateQGlobals();
		char_c->LoadByCharID(char_id);
	}

	if(!zone_c && z) {
		zone_c = z->CreateQGlobals();
		zone_c->LoadByZoneID(zone_id);
		zone_c->LoadByGlobalContext();
	}

	if(npc_c) {
		QGlobalCache::Combine(globals, npc_c->GetBucket(), npc_id, char_id, zone_id);
	}

	if(char_c) {
		QGlobalCache::Combine(globals, char_c->GetBucket(), npc_id, char_id, zone_id);
	}

	if(zone_c) {
		QGlobalCache::Combine(globals, zone_c->GetBucket(), npc_id, char_id, zone_id);
	}
}

void QGlobalCache::PurgeExpiredGlobals()
{
	if(!qGlobalBucket.size())
		return;

	auto iter = qGlobalBucket.begin();
	while(iter != qGlobalBucket.end())
	{
		QGlobal cur = (*iter);
		if(Timer::GetTimeSeconds() > cur.expdate)
		{
			iter = qGlobalBucket.erase(iter);
			continue;
		}
		++iter;
	}
}

void QGlobalCache::LoadByNPCID(uint32 npcID)
{
	std::string query = StringFormat("SELECT name, charid, npcid, zoneid, value, expdate "
                                    "FROM quest_globals WHERE npcid = %d", npcID);
    LoadBy(query);
}

void QGlobalCache::LoadByCharID(uint32 charID)
{
	std::string query = StringFormat("SELECT name, charid, npcid, zoneid, value, expdate "
                                    "FROM quest_globals WHERE charid = %d && npcid = 0", charID);
    LoadBy(query);
}

void QGlobalCache::LoadByZoneID(uint32 zoneID)
{
	std::string query = StringFormat("SELECT name, charid, npcid, zoneid, value, expdate "
                                    "FROM quest_globals WHERE zoneid = %d && npcid = 0 && charid = 0", zoneID);
    LoadBy(query);
}

void QGlobalCache::LoadByGlobalContext()
{
	std::string query = "SELECT name, charid, npcid, zoneid, value, expdate "
                        "FROM quest_globals WHERE zoneid = 0 && npcid = 0 && charid = 0";
    LoadBy(query);
 }

void QGlobalCache::LoadBy(const std::string &query)
{
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return;

	for (auto row = results.begin(); row != results.end(); ++row)
		AddGlobal(0, QGlobal(row[0], Strings::ToInt(row[1]), Strings::ToInt(row[2]), Strings::ToInt(row[3]), row[4], row[5] ? Strings::ToInt(row[5]) : 0xFFFFFFFF));
}
