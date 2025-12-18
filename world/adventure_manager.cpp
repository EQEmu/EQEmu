#include "adventure_manager.h"

#include "common/misc_functions.h"
#include "common/random.h"
#include "common/rulesys.h"
#include "common/servertalk.h"
#include "common/strings.h"
#include "common/zone_store.h"
#include "world/adventure.h"
#include "world/cliententry.h"
#include "world/clientlist.h"
#include "world/worlddb.h"
#include "world/zonelist.h"

#include <cstdio>
#include <sstream>

AdventureManager::AdventureManager()
{
	process_timer = new Timer(500);
	save_timer = new Timer(90000);
	leaderboard_info_timer = new Timer(180000);
}

AdventureManager::~AdventureManager()
{
	safe_delete(process_timer);
	safe_delete(save_timer);
	safe_delete(leaderboard_info_timer);

	for (auto &elem : adventure_templates)
		delete elem.second;
}

void AdventureManager::Process()
{
	if(process_timer->Check())
	{
		auto iter = adventure_list.begin();
		while(iter != adventure_list.end())
		{
			if(!(*iter)->Process())
			{
				Adventure *adv = (*iter);
				iter = adventure_list.erase(iter);
				GetAdventureData(adv);
				delete adv;
				continue;
			}
			++iter;
		}
	}

	if(leaderboard_info_timer->Check())
	{
		LoadLeaderboardInfo();
	}

	if(save_timer->Check())
	{
		Save();
	}
}

void AdventureManager::CalculateAdventureRequestReply(const char *data)
{
	ServerAdventureRequest_Struct *sar = (ServerAdventureRequest_Struct*)data;
	ClientListEntry *leader = ClientList::Instance()->FindCharacter(sar->leader);
	if(!leader)
	{
		return;
	}

	/**
	* This block checks to see if we actually have any adventures for the requested theme.
	*/
	auto adv_list_iter = adventure_entries.find(sar->template_id);
	if(adv_list_iter == adventure_entries.end())
	{
		auto pack = new ServerPacket(ServerOP_AdventureRequestDeny, sizeof(ServerAdventureRequestDeny_Struct));
		ServerAdventureRequestDeny_Struct *deny = (ServerAdventureRequestDeny_Struct*)pack->pBuffer;
		strcpy(deny->leader, sar->leader);
		strcpy(deny->reason, "There are currently no adventures set for this theme.");
		ZSList::Instance()->SendPacket(leader->zone(), leader->instance(), pack);
		delete pack;
		return;
	}

	/**
	* This block checks to see if our requested group has anyone with an "Active" adventure
	* Active being in progress, finished adventures that are still waiting to expire do not count
	* Though they will count against you for which new adventure you can get.
	*/
	auto iter = adventure_list.begin();
	while(iter != adventure_list.end())
	{
		Adventure* current = (*iter);
		if(current->IsActive())
		{
			for(int i = 0; i < sar->member_count; ++i)
			{
				if(current->PlayerExists((data + sizeof(ServerAdventureRequest_Struct) + (64 * i))))
				{
					auto pack = new ServerPacket(ServerOP_AdventureRequestDeny,
								     sizeof(ServerAdventureRequestDeny_Struct));
					ServerAdventureRequestDeny_Struct *deny = (ServerAdventureRequestDeny_Struct*)pack->pBuffer;
					strcpy(deny->leader, sar->leader);

					std::stringstream ss(std::stringstream::out | std::stringstream::in);
					ss << (data + sizeof(ServerAdventureRequest_Struct) + (64 * i)) << " is already apart of an active adventure.";

					strcpy(deny->reason, ss.str().c_str());
					ZSList::Instance()->SendPacket(leader->zone(), leader->instance(), pack);
					delete pack;
					return;
				}
			}
		}
		++iter;
	}

	/**
	* Now we need to get every available adventure for our selected theme and exclude ones we can't use.
	* ie. the ones that would cause overlap issues for new adventures with the old unexpired adventures.
	*/
	std::list<AdventureZones> excluded_zones;
	std::list<AdventureZoneIn> excluded_zone_ins;
	for(int i = 0; i < sar->member_count; ++i)
	{
		int finished_adventures_count;
		Adventure **finished_adventures = GetFinishedAdventures((data + sizeof(ServerAdventureRequest_Struct) + (64 * i)), finished_adventures_count);
		for(int i = 0; i < finished_adventures_count; ++i)
		{
			if(!IsInExcludedZoneList(excluded_zones, finished_adventures[i]->GetTemplate()->zone, finished_adventures[i]->GetTemplate()->zone_version))
			{
				AdventureZones adv;
				adv.zone = finished_adventures[i]->GetTemplate()->zone;
				adv.version = finished_adventures[i]->GetTemplate()->zone_version;
				excluded_zones.push_back(adv);
			}
			if(!IsInExcludedZoneInList(excluded_zone_ins, finished_adventures[i]->GetTemplate()->zone_in_zone_id,
				finished_adventures[i]->GetTemplate()->zone_in_object_id))
			{
				AdventureZoneIn adv;
				adv.door_id = finished_adventures[i]->GetTemplate()->zone_in_object_id;
				adv.zone_id = finished_adventures[i]->GetTemplate()->zone_in_zone_id;
				excluded_zone_ins.push_back(adv);
			}
		}
		safe_delete_array(finished_adventures);
	}

	std::list<AdventureTemplate*> eligible_adventures = adventure_entries[sar->template_id];
	/**
	* Remove zones from eligible zones based on their difficulty and type.
	* ie only use difficult zones for difficult, collect for collect, etc.
	*/
	auto ea_iter = eligible_adventures.begin();
	while(ea_iter != eligible_adventures.end())
	{
		if((*ea_iter)->is_hard != ((sar->risk == 2) ? true : false))
		{
			ea_iter = eligible_adventures.erase(ea_iter);
			continue;
		}

		if(sar->type != 0 && sar->type != (*ea_iter)->type)
		{
			ea_iter = eligible_adventures.erase(ea_iter);
			continue;
		}
		++ea_iter;
	}

	/**
	* Get levels for this group.
	*/
	int valid_count = 0;
	int avg_level = 0;
	int min_level = 40000;
	int max_level = 0;

	for(int i = 0; i < sar->member_count; ++i)
	{
		ClientListEntry *current = ClientList::Instance()->FindCharacter((data + sizeof(ServerAdventureRequest_Struct) + (64 * i)));
		if(current)
		{
			int lvl = current->level();
			if(lvl != 0)
			{
				avg_level += lvl;
				valid_count++;
				if(lvl < min_level)
				{
					min_level = lvl;
				}
				if(lvl > max_level)
				{
					max_level = lvl;
				}
			}
			else
			{
				if(database.GetCharacterLevel((data + sizeof(ServerAdventureRequest_Struct) + (64 * i)), lvl))
				{
					avg_level += lvl;
					valid_count++;
					if(lvl < min_level)
					{
						min_level = lvl;
					}
					if(lvl > max_level)
					{
						max_level = lvl;
					}
				}
			}
		}
		else
		{
			int lvl = 0;
			if(database.GetCharacterLevel((data + sizeof(ServerAdventureRequest_Struct) + (64 * i)), lvl))
			{
				avg_level += lvl;
				valid_count++;
				if(lvl < min_level)
				{
					min_level = lvl;
				}
				if(lvl > max_level)
				{
					max_level = lvl;
				}
			}
		}
	}

	if(valid_count == 0)
	{
		auto pack = new ServerPacket(ServerOP_AdventureRequestDeny, sizeof(ServerAdventureRequestDeny_Struct));
		ServerAdventureRequestDeny_Struct *deny = (ServerAdventureRequestDeny_Struct*)pack->pBuffer;
		strcpy(deny->leader, sar->leader);
		strcpy(deny->reason, "The number of found players for this adventure was zero.");
		ZSList::Instance()->SendPacket(leader->zone(), leader->instance(), pack);
		delete pack;
		return;
	}

	avg_level = avg_level / valid_count;

	if(max_level - min_level > RuleI(Adventure, MaxLevelRange))
	{
		auto pack = new ServerPacket(ServerOP_AdventureRequestDeny, sizeof(ServerAdventureRequestDeny_Struct));
		ServerAdventureRequestDeny_Struct *deny = (ServerAdventureRequestDeny_Struct*)pack->pBuffer;
		strcpy(deny->leader, sar->leader);

		std::stringstream ss(std::stringstream::out | std::stringstream::in);
		ss << "The maximum level range for this adventure is " << RuleI(Adventure, MaxLevelRange);
		ss << " but the level range calculated was " << (max_level - min_level) << ".";
		strcpy(deny->reason, ss.str().c_str());
		ZSList::Instance()->SendPacket(leader->zone(), leader->instance(), pack);
		delete pack;
		return;
	}

	/**
	* Remove the zones from our eligible zones based on the exclusion above
	*/
	auto ez_iter = excluded_zones.begin();
	while(ez_iter != excluded_zones.end())
	{
		auto ea_iter = eligible_adventures.begin();
		while(ea_iter != eligible_adventures.end())
		{
			if((*ez_iter).zone.compare((*ea_iter)->zone) == 0 && (*ez_iter).version == (*ea_iter)->zone_version)
			{
				ea_iter = eligible_adventures.erase(ea_iter);
				continue;
			}
			++ea_iter;
		}
		++ez_iter;
	}

	auto ezi_iter = excluded_zone_ins.begin();
	while(ezi_iter != excluded_zone_ins.end())
	{
		auto ea_iter = eligible_adventures.begin();
		while(ea_iter != eligible_adventures.end())
		{
			if((*ezi_iter).zone_id == (*ea_iter)->zone_in_zone_id && (*ezi_iter).door_id == (*ea_iter)->zone_in_object_id)
			{
				ea_iter = eligible_adventures.erase(ea_iter);
				continue;
			}
			++ea_iter;
		}
		++ezi_iter;
	}

	/**
	* Remove Zones based on level
	*/
	ea_iter = eligible_adventures.begin();
	while(ea_iter != eligible_adventures.end())
	{
		if((*ea_iter)->min_level > avg_level)
		{
			ea_iter = eligible_adventures.erase(ea_iter);
			continue;
		}

		if((*ea_iter)->max_level < avg_level)
		{
			ea_iter = eligible_adventures.erase(ea_iter);
			continue;
		}
		++ea_iter;
	}

	if(eligible_adventures.size() > 0)
	{
		ea_iter = eligible_adventures.begin();
		int c_index = EQ::Random::Instance()->Int(0, (eligible_adventures.size()-1));
		for(int i = 0; i < c_index; ++i)
		{
			++ea_iter;
		}
		auto pack = new ServerPacket(ServerOP_AdventureRequestAccept,
					     sizeof(ServerAdventureRequestAccept_Struct) + (sar->member_count * 64));
		ServerAdventureRequestAccept_Struct *sra = (ServerAdventureRequestAccept_Struct*)pack->pBuffer;
		strcpy(sra->leader, sar->leader);
		strcpy(sra->text, (*ea_iter)->text);
		sra->theme = sar->template_id;
		sra->id = (*ea_iter)->id;
		sra->member_count = sar->member_count;
		memcpy((pack->pBuffer + sizeof(ServerAdventureRequestAccept_Struct)), (data + sizeof(ServerAdventureRequest_Struct)), (sar->member_count * 64));
		ZSList::Instance()->SendPacket(leader->zone(), leader->instance(), pack);
		delete pack;
		return;
	}
	else
	{
		auto pack = new ServerPacket(ServerOP_AdventureRequestDeny, sizeof(ServerAdventureRequestDeny_Struct));
		ServerAdventureRequestDeny_Struct *deny = (ServerAdventureRequestDeny_Struct*)pack->pBuffer;
		strcpy(deny->leader, sar->leader);
		strcpy(deny->reason, "The number of adventures returned was zero.");
		ZSList::Instance()->SendPacket(leader->zone(), leader->instance(), pack);
		delete pack;
		return;
	}
}

void AdventureManager::TryAdventureCreate(const char *data)
{
	ServerAdventureRequestCreate_Struct *src = (ServerAdventureRequestCreate_Struct*)data;
	ClientListEntry *leader = ClientList::Instance()->FindCharacter(src->leader);
	if(!leader)
	{
		return;
	}

	AdventureTemplate *adv_template = GetAdventureTemplate(src->theme, src->id);
	if(!adv_template)
	{
		auto pack = new ServerPacket(ServerOP_AdventureCreateDeny, 64);
		strcpy((char*)pack->pBuffer, src->leader);
		ZSList::Instance()->SendPacket(leader->zone(), leader->instance(), pack);
		delete pack;
		return;
	}

	auto adv = new Adventure(adv_template);
	if(!adv->CreateInstance())
	{
		auto pack = new ServerPacket(ServerOP_AdventureCreateDeny, 64);
		strcpy((char*)pack->pBuffer, src->leader);
		ZSList::Instance()->SendPacket(leader->zone(), leader->instance(), pack);
		delete pack;
		delete adv;
		return;
	}

	for(int i = 0; i < src->member_count; ++i)
	{
		Adventure *a = GetActiveAdventure((data + sizeof(ServerAdventureRequestCreate_Struct) + (64 * i)));
		if(a)
		{
			auto pack = new ServerPacket(ServerOP_AdventureCreateDeny, 64);
			strcpy((char*)pack->pBuffer, src->leader);
			ZSList::Instance()->SendPacket(leader->zone(), leader->instance(), pack);
			delete pack;
			delete adv;
			return;
		}

		adv->AddPlayer((data + sizeof(ServerAdventureRequestCreate_Struct) + (64 * i)));
	}

	//Need to send adventure data to zone server for each client.
	for(int i = 0; i < src->member_count; ++i)
	{

		ClientListEntry *player = ClientList::Instance()->FindCharacter((data + sizeof(ServerAdventureRequestCreate_Struct) + (64 * i)));
		if(player)
		{
			int f_count = 0;
			Adventure** finished_adventures = GetFinishedAdventures((data + sizeof(ServerAdventureRequestCreate_Struct) + (64 * i)), f_count);
			auto pack = new ServerPacket(ServerOP_AdventureData,
						     sizeof(ServerSendAdventureData_Struct) +
							 (sizeof(ServerFinishedAdventures_Struct) * f_count));
			ServerSendAdventureData_Struct *sca = (ServerSendAdventureData_Struct*)pack->pBuffer;

			strcpy(sca->player, (data + sizeof(ServerAdventureRequestCreate_Struct) + (64 * i)));
			strcpy(sca->text, adv_template->text);
			sca->time_left = adv_template->zone_in_time;
			sca->time_to_enter = adv_template->zone_in_time;
			sca->risk = adv_template->is_hard ? 1 : 0;
			sca->x = adv_template->zone_in_x;
			sca->y = adv_template->zone_in_y;
			sca->zone_in_id = adv_template->zone_in_zone_id;
			sca->zone_in_object = adv_template->zone_in_object_id;
			sca->instance_id = adv->GetInstanceID();
			sca->count = 0;
			sca->total = adv_template->type_count;
			sca->finished_adventures = f_count;
			for(int f = 0; f < f_count; ++f)
			{
				ServerFinishedAdventures_Struct *sfa = (ServerFinishedAdventures_Struct*)(pack->pBuffer
					+ sizeof(ServerSendAdventureData_Struct)
					+ (sizeof(ServerFinishedAdventures_Struct) * f));
				sfa->zone_in_id = finished_adventures[f]->GetTemplate()->zone_in_zone_id;
				sfa->zone_in_object = finished_adventures[f]->GetTemplate()->zone_in_object_id;
			}

			ZSList::Instance()->SendPacket(player->zone(), player->instance(), pack);
			safe_delete_array(finished_adventures);
			delete pack;
		}
	}

	adventure_list.push_back(adv);
}

void AdventureManager::GetAdventureData(Adventure *adv)
{
	std::list<std::string> player_list = adv->GetPlayers();
	auto iter = player_list.begin();
	while(iter != player_list.end())
	{
		GetAdventureData((*iter).c_str());
		++iter;
	}
}

void AdventureManager::GetAdventureData(const char *name)
{
	ClientListEntry *player = ClientList::Instance()->FindCharacter(name);
	if(player)
	{
		int f_count = 0;
		Adventure** finished_adventures = GetFinishedAdventures(name, f_count);
		Adventure *current = GetActiveAdventure(name);
		auto pack =
		    new ServerPacket(ServerOP_AdventureData, sizeof(ServerSendAdventureData_Struct) +
								 (sizeof(ServerFinishedAdventures_Struct) * f_count));
		ServerSendAdventureData_Struct *sca = (ServerSendAdventureData_Struct*)pack->pBuffer;

		if(current)
		{
			const AdventureTemplate *adv_template = current->GetTemplate();
			strcpy(sca->player, name);
			strcpy(sca->text, adv_template->text);
			sca->risk = adv_template->is_hard ? 1 : 0;
			sca->x = adv_template->zone_in_x;
			sca->y = adv_template->zone_in_y;
			sca->zone_in_id = adv_template->zone_in_zone_id;
			sca->zone_in_object = adv_template->zone_in_object_id;
			sca->count = current->GetCount();
			sca->total = adv_template->type_count;

			sca->time_left = current->GetRemainingTime();
			if(current->GetStatus() == AS_WaitingForZoneIn)
			{
				sca->time_to_enter = sca->time_left;
			}
		}
		else
		{
			//We have no mission and no finished missions
			//Delete our stuff and return instead of sending a blank packet.
			if(f_count == 0)
			{
				delete pack;
				auto pack = new ServerPacket(ServerOP_AdventureDataClear, 64);
				strcpy((char*)pack->pBuffer, name);
				ZSList::Instance()->SendPacket(player->zone(), player->instance(), pack);

				delete pack;
				delete[] finished_adventures;
				return;
			}
		}

		sca->finished_adventures = f_count;
		for(int i = 0; i < f_count; ++i)
		{
			ServerFinishedAdventures_Struct *sfa = (ServerFinishedAdventures_Struct*)(pack->pBuffer
				+ sizeof(ServerSendAdventureData_Struct)
				+ (sizeof(ServerFinishedAdventures_Struct) * i));
			sfa->zone_in_id = finished_adventures[i]->GetTemplate()->zone_in_zone_id;
			sfa->zone_in_object = finished_adventures[i]->GetTemplate()->zone_in_object_id;
		}

		ZSList::Instance()->SendPacket(player->zone(), player->instance(), pack);
		safe_delete_array(finished_adventures);
		delete pack;
		delete[] finished_adventures;
	}
}

bool AdventureManager::IsInExcludedZoneList(std::list<AdventureZones> excluded_zones, std::string zone_name, int version)
{
	auto iter = excluded_zones.begin();
	while(iter != excluded_zones.end())
	{
		if(((*iter).zone.compare(zone_name) == 0) && ((*iter).version == version))
		{
			return true;
		}
		++iter;
	}
	return false;
}

bool AdventureManager::IsInExcludedZoneInList(std::list<AdventureZoneIn> excluded_zone_ins, int zone_id, int door_object)
{
	auto iter = excluded_zone_ins.begin();
	while(iter != excluded_zone_ins.end())
	{
		if(((*iter).zone_id == zone_id) && ((*iter).door_id == door_object))
		{
			return true;
		}
		++iter;
	}
	return false;
}

Adventure **AdventureManager::GetFinishedAdventures(const char *player, int &count)
{
	Adventure **ret = nullptr;
	count = 0;

	auto iter = adventure_list.begin();
	while(iter != adventure_list.end())
	{
		if((*iter)->PlayerExists(player))
		{
			if(!(*iter)->IsActive())
			{
				if(ret)
				{
					auto t = new Adventure *[count + 1];
					for(int i = 0; i < count; i++)
					{
						t[i] = ret[i];
					}
					t[count] = (*iter);
					delete[] ret;
					ret = t;
				}
				else
				{
					ret = new Adventure*[1];
					ret[0] = (*iter);
				}
				count++;
			}
		}
		++iter;
	}
	return ret;
}

Adventure *AdventureManager::GetActiveAdventure(const char *player)
{
	auto iter = adventure_list.begin();
	while(iter != adventure_list.end())
	{
		if((*iter)->PlayerExists(player) && (*iter)->IsActive())
		{
			return (*iter);
		}
		++iter;
	}
	return nullptr;
}

AdventureTemplate *AdventureManager::GetAdventureTemplate(int theme, int id)
{
	auto iter = adventure_entries.find(theme);
	if(iter == adventure_entries.end())
	{
		return nullptr;
	}

	auto l_iter = (*iter).second.begin();
	while(l_iter != (*iter).second.end())
	{
		if((*l_iter)->id == id)
		{
			return (*l_iter);
		}
		++l_iter;
	}
	return nullptr;
}

AdventureTemplate *AdventureManager::GetAdventureTemplate(int id)
{
	auto iter = adventure_templates.find(id);
	if(iter == adventure_templates.end())
	{
		return nullptr;
	}

	return iter->second;
}

bool AdventureManager::LoadAdventureTemplates()
{
	std::string query =
		SQL (
			SELECT
			id,
			zone,
			zone_version,
			is_hard,
			min_level,
			max_level,
			type,
			type_data,
			type_count,
			assa_x,
			assa_y,
			assa_z,
			assa_h,
			text,
			duration,
			zone_in_time,
			win_points,
			lose_points,
			theme,
			zone_in_zone_id,
			zone_in_x,
			zone_in_y,
			zone_in_object_id,
			dest_x,
			dest_y,
			dest_z,
			dest_h,
			graveyard_zone_id,
			graveyard_x,
			graveyard_y,
			graveyard_z,
			graveyard_radius
			FROM
			adventure_template
		)
	;
    auto results = content_db.QueryDatabase(query);
    if (!results.Success()) {
		return false;
    }

	for (auto row = results.begin(); row != results.end(); ++row) {
		auto adventure_template = new AdventureTemplate;
		adventure_template->id = Strings::ToInt(row[0]);
		strcpy(adventure_template->zone, row[1]);

		adventure_template->zone_version = Strings::ToInt(row[2]);
		adventure_template->is_hard      = Strings::ToInt(row[3]);
		adventure_template->min_level    = Strings::ToInt(row[4]);
		adventure_template->max_level    = Strings::ToInt(row[5]);
		adventure_template->type         = Strings::ToInt(row[6]);
		adventure_template->type_data    = Strings::ToInt(row[7]);
		adventure_template->type_count   = Strings::ToInt(row[8]);
		adventure_template->assa_x       = Strings::ToFloat(row[9]);
		adventure_template->assa_y       = Strings::ToFloat(row[10]);
		adventure_template->assa_z       = Strings::ToFloat(row[11]);
		adventure_template->assa_h       = Strings::ToFloat(row[12]);

		strn0cpy(adventure_template->text, row[13], sizeof(adventure_template->text));

		adventure_template->duration          = Strings::ToInt(row[14]);
		adventure_template->zone_in_time      = Strings::ToInt(row[15]);
		adventure_template->win_points        = Strings::ToInt(row[16]);
		adventure_template->lose_points       = Strings::ToInt(row[17]);
		adventure_template->theme             = Strings::ToInt(row[18]);
		adventure_template->zone_in_zone_id   = Strings::ToInt(row[19]);
		adventure_template->zone_in_x         = Strings::ToFloat(row[20]);
		adventure_template->zone_in_y         = Strings::ToFloat(row[21]);
		adventure_template->zone_in_object_id = Strings::ToInt(row[22]);
		adventure_template->dest_x            = Strings::ToFloat(row[23]);
		adventure_template->dest_y            = Strings::ToFloat(row[24]);
		adventure_template->dest_z            = Strings::ToFloat(row[25]);
		adventure_template->dest_h            = Strings::ToFloat(row[26]);
		adventure_template->graveyard_zone_id = Strings::ToInt(row[27]);
		adventure_template->graveyard_x       = Strings::ToFloat(row[28]);
		adventure_template->graveyard_y       = Strings::ToFloat(row[29]);
		adventure_template->graveyard_z       = Strings::ToFloat(row[30]);
		adventure_template->graveyard_radius  = Strings::ToFloat(row[31]);

		adventure_templates[adventure_template->id] = adventure_template;
	}

    return true;
}

bool AdventureManager::LoadAdventureEntries()
{
	std::string query = "SELECT id, template_id FROM adventure_template_entry";
    auto results = content_db.QueryDatabase(query);
    if (!results.Success())
	{
		return false;
	}

    for (auto row = results.begin(); row != results.end(); ++row)
    {
        int id = Strings::ToInt(row[0]);
        int template_id = Strings::ToInt(row[1]);
        AdventureTemplate* tid = nullptr;

        auto t_iter = adventure_templates.find(template_id);
        if(t_iter == adventure_templates.end())
            continue;

        tid = adventure_templates[template_id];

        std::list<AdventureTemplate*> temp;
        auto iter = adventure_entries.find(id);
        if(iter != adventure_entries.end())
            temp = adventure_entries[id];

        temp.push_back(tid);
        adventure_entries[id] = temp;
    }

    return true;
}

void AdventureManager::PlayerClickedDoor(const char *player, int zone_id, int door_id)
{
	auto iter = adventure_list.begin();
	while(iter != adventure_list.end())
	{
		const AdventureTemplate *t = (*iter)->GetTemplate();
		if(t->zone_in_zone_id == zone_id && t->zone_in_object_id == door_id)
		{
			if((*iter)->PlayerExists(player))
			{
				ClientListEntry *pc = ClientList::Instance()->FindCharacter(player);
				if(pc)
				{
					auto pack =
					    new ServerPacket(ServerOP_AdventureClickDoorReply,
							     sizeof(ServerPlayerClickedAdventureDoorReply_Struct));
					ServerPlayerClickedAdventureDoorReply_Struct *sr = (ServerPlayerClickedAdventureDoorReply_Struct*)pack->pBuffer;
					strcpy(sr->player, player);
					sr->zone_id = ZoneID(t->zone);
					sr->instance_id = (*iter)->GetInstanceID();
					sr->x = t->dest_x;
					sr->y = t->dest_y;
					sr->z = t->dest_z;
					sr->h = t->dest_h;
					if((*iter)->GetStatus() == AS_WaitingForZoneIn)
					{
						(*iter)->SetStatus(AS_WaitingForPrimaryEndTime);
					}

					ZSList::Instance()->SendPacket(pc->zone(), pc->instance(), pack);
					safe_delete(pack);
				}
				return;
			}
		}
		++iter;
	}

	ClientListEntry *pc = ClientList::Instance()->FindCharacter(player);
	if(pc)
	{
		auto pack = new ServerPacket(ServerOP_AdventureClickDoorError, 64);
		strcpy((char*)pack->pBuffer, player);
		ZSList::Instance()->SendPacket(pc->zone(), pc->instance(), pack);
		safe_delete(pack);
	}
}

void AdventureManager::LeaveAdventure(const char *name)
{
	ClientListEntry *pc = ClientList::Instance()->FindCharacter(name);
	if(pc)
	{
		Adventure *current = GetActiveAdventure(name);
		if(current)
		{
			if(pc->instance() != 0 && pc->instance() == current->GetInstanceID())
			{
				auto pack = new ServerPacket(ServerOP_AdventureLeaveDeny, 64);
				strcpy((char*)pack->pBuffer, name);
				ZSList::Instance()->SendPacket(pc->zone(), pc->instance(), pack);
				safe_delete(pack);
			}
			else
			{
				if(current->GetStatus() != AS_WaitingForZoneIn)
				{
					database.UpdateAdventureStatsEntry(database.GetCharacterID(name), current->GetTemplate()->theme, false);
				}

				current->RemovePlayer(name);
				auto pack = new ServerPacket(ServerOP_AdventureLeaveReply, 64);
				strcpy((char*)pack->pBuffer, name);
				ZSList::Instance()->SendPacket(pc->zone(), pc->instance(), pack);
				safe_delete(pack);
			}
		}
		else
		{
			auto pack = new ServerPacket(ServerOP_AdventureLeaveReply, 64);
			strcpy((char*)pack->pBuffer, name);
			ZSList::Instance()->SendPacket(pc->zone(), pc->instance(), pack);
			safe_delete(pack);
		}
	}
}

void AdventureManager::IncrementCount(uint16 instance_id)
{
	auto iter = adventure_list.begin();
	Adventure *current = nullptr;
	while(iter != adventure_list.end())
	{
		if((*iter)->GetInstanceID() == instance_id)
		{
			current = (*iter);
			break;
		}
		++iter;
	}

	if(current)
	{
		current->IncrementCount();
		std::list<std::string> slist = current->GetPlayers();
		auto siter = slist.begin();
		auto pack = new ServerPacket(ServerOP_AdventureCountUpdate, sizeof(ServerAdventureCountUpdate_Struct));
		ServerAdventureCountUpdate_Struct *ac = (ServerAdventureCountUpdate_Struct*)pack->pBuffer;
		ac->count = current->GetCount();
		ac->total = current->GetTemplate()->type_count;

		while(siter != slist.end())
		{
			ClientListEntry *pc = ClientList::Instance()->FindCharacter((*siter).c_str());
			if(pc)
			{
				memset(ac->player, 0, 64);
				strcpy(ac->player, (*siter).c_str());
				ZSList::Instance()->SendPacket(pc->zone(), pc->instance(), pack);
			}
			++siter;
		}

		delete pack;
	}
}

void AdventureManager::IncrementAssassinationCount(uint16 instance_id)
{
	auto iter = adventure_list.begin();
	Adventure *current = nullptr;
	while(iter != adventure_list.end())
	{
		if((*iter)->GetInstanceID() == instance_id)
		{
			current = (*iter);
			break;
		}
		++iter;
	}

	if(current)
	{
		current->IncrementAssassinationCount();
	}
}


void AdventureManager::GetZoneData(uint16 instance_id)
{
	auto iter = adventure_list.begin();
	Adventure *current = nullptr;
	while(iter != adventure_list.end())
	{
		if((*iter)->GetInstanceID() == instance_id)
		{
			current = (*iter);
			break;
		}
		++iter;
	}

	if(current)
	{
		auto pack = new ServerPacket(ServerOP_AdventureZoneData, sizeof(ServerZoneAdventureDataReply_Struct));
		ServerZoneAdventureDataReply_Struct *zd = (ServerZoneAdventureDataReply_Struct*)pack->pBuffer;

		const AdventureTemplate* temp = current->GetTemplate();
		zd->instance_id = instance_id;
		zd->count = current->GetCount();
		zd->total = temp->type_count;
		zd->type = temp->type;
		zd->data_id = temp->type_data;
		zd->assa_count = current->GetAssassinationCount();
		zd->assa_x = temp->assa_x;
		zd->assa_y = temp->assa_y;
		zd->assa_z = temp->assa_z;
		zd->assa_h = temp->assa_h;
		zd->dest_x = temp->dest_x;
		zd->dest_y = temp->dest_y;
		zd->dest_z = temp->dest_z;
		zd->dest_h = temp->dest_h;
		ZSList::Instance()->SendPacket(0, instance_id, pack);
		delete pack;
	}
}

// Sort the leaderboard by wins.
bool pred_sort_by_win_count(const LeaderboardInfo &lbi1, const LeaderboardInfo &lbi2)
{
	return lbi1.wins > lbi2.wins;
}

// Sort the leaderboard by win percentage.
bool pred_sort_by_win_percentage(const LeaderboardInfo &lbi1, const LeaderboardInfo &lbi2)
{
	int per1 = 10000;
	int per2 = 10000;
	if(lbi1.wins + lbi1.losses != 0)
	{
		per1 = (lbi1.wins * 10000 / (lbi1.wins + lbi1.losses));
	}

	if(lbi2.wins + lbi2.losses != 0)
	{
		per1 = (lbi2.wins * 10000 / (lbi2.wins + lbi2.losses));
	}
	return per1 > per2;
}

// Sort the leaderboard by wins(guk).
bool pred_sort_by_win_count_guk(const LeaderboardInfo &lbi1, const LeaderboardInfo &lbi2)
{
	return lbi1.guk_wins > lbi2.guk_wins;
}

// Sort the leaderboard by win percentage(guk).
bool pred_sort_by_win_percentage_guk(const LeaderboardInfo &lbi1, const LeaderboardInfo &lbi2)
{
	int per1 = 10000;
	int per2 = 10000;
	if(lbi1.guk_wins + lbi1.guk_losses != 0)
	{
		per1 = (lbi1.guk_wins * 10000 / (lbi1.guk_wins + lbi1.guk_losses));
	}

	if(lbi2.guk_wins + lbi2.guk_losses != 0)
	{
		per1 = (lbi2.guk_wins * 10000 / (lbi2.guk_wins + lbi2.guk_losses));
	}
	return per1 > per2;
}

// Sort the leaderboard by wins(mir).
bool pred_sort_by_win_count_mir(const LeaderboardInfo &lbi1, const LeaderboardInfo &lbi2)
{
	return lbi1.mir_wins > lbi2.mir_wins;
}

// Sort the leaderboard by win percentage(mir).
bool pred_sort_by_win_percentage_mir(const LeaderboardInfo &lbi1, const LeaderboardInfo &lbi2)
{
	int per1 = 10000;
	int per2 = 10000;
	if(lbi1.mir_wins + lbi1.mir_losses != 0)
	{
		per1 = (lbi1.mir_wins * 10000 / (lbi1.mir_wins + lbi1.mir_losses));
	}

	if(lbi2.mir_wins + lbi2.mir_losses != 0)
	{
		per1 = (lbi2.mir_wins * 10000 / (lbi2.mir_wins + lbi2.mir_losses));
	}
	return per1 > per2;
}

// Sort the leaderboard by wins(mmc).
bool pred_sort_by_win_count_mmc(const LeaderboardInfo &lbi1, const LeaderboardInfo &lbi2)
{
	return lbi1.mmc_wins > lbi2.mmc_wins;
}

// Sort the leaderboard by win percentage(mmc).
bool pred_sort_by_win_percentage_mmc(const LeaderboardInfo &lbi1, const LeaderboardInfo &lbi2)
{
	int per1 = 10000;
	int per2 = 10000;
	if(lbi1.mmc_wins + lbi1.mmc_losses != 0)
	{
		per1 = (lbi1.mmc_wins * 10000 / (lbi1.mmc_wins + lbi1.mmc_losses));
	}

	if(lbi2.mmc_wins + lbi2.mmc_losses != 0)
	{
		per1 = (lbi2.mmc_wins * 10000 / (lbi2.mmc_wins + lbi2.mmc_losses));
	}
	return per1 > per2;
}

// Sort the leaderboard by wins(ruj).
bool pred_sort_by_win_count_ruj(const LeaderboardInfo &lbi1, const LeaderboardInfo &lbi2)
{
	return lbi1.ruj_wins > lbi2.ruj_wins;
}

// Sort the leaderboard by win percentage(ruj).
bool pred_sort_by_win_percentage_ruj(const LeaderboardInfo &lbi1, const LeaderboardInfo &lbi2)
{
	int per1 = 10000;
	int per2 = 10000;
	if(lbi1.ruj_wins + lbi1.ruj_losses != 0)
	{
		per1 = (lbi1.ruj_wins * 10000 / (lbi1.ruj_wins + lbi1.ruj_losses));
	}

	if(lbi2.ruj_wins + lbi2.ruj_losses != 0)
	{
		per1 = (lbi2.ruj_wins * 10000 / (lbi2.ruj_wins + lbi2.ruj_losses));
	}
	return per1 > per2;
}

// Sort the leaderboard by wins(tak).
bool pred_sort_by_win_count_tak(const LeaderboardInfo &lbi1, const LeaderboardInfo &lbi2)
{
	return lbi1.tak_wins > lbi2.tak_wins;
}

// Sort the leaderboard by win percentage(tak).
bool pred_sort_by_win_percentage_tak(const LeaderboardInfo &lbi1, const LeaderboardInfo &lbi2)
{
	int per1 = 10000;
	int per2 = 10000;
	if(lbi1.tak_wins + lbi1.tak_losses != 0)
	{
		per1 = (lbi1.tak_wins * 10000 / (lbi1.tak_wins + lbi1.tak_losses));
	}

	if(lbi2.tak_wins + lbi2.tak_losses != 0)
	{
		per1 = (lbi2.tak_wins * 10000 / (lbi2.tak_wins + lbi2.tak_losses));
	}
	return per1 > per2;
}

void AdventureManager::LoadLeaderboardInfo()
{
	leaderboard_info_wins.clear();
	leaderboard_info_percentage.clear();
	leaderboard_info_wins_guk.clear();
	leaderboard_info_percentage_guk.clear();
	leaderboard_info_wins_mir.clear();
	leaderboard_info_percentage_mir.clear();
	leaderboard_info_wins_mmc.clear();
	leaderboard_info_percentage_mmc.clear();
	leaderboard_info_wins_ruj.clear();
	leaderboard_info_percentage_ruj.clear();
	leaderboard_info_wins_tak.clear();
	leaderboard_info_percentage_tak.clear();

	std::string query = "SELECT ch.name, ch.id, adv_stats.* FROM adventure_stats "
		"AS adv_stats LEFT JOIN `character_data` AS ch ON adv_stats.player_id = ch.id;";
    auto results = database.QueryDatabase(query);
	if(!results.Success()) {
		return;
	}

    for (auto row = results.begin(); row != results.end(); ++row)
    {
        if(!row[0])
            continue;

        LeaderboardInfo lbi;
        lbi.name = row[0];
        lbi.wins = Strings::ToInt(row[3]);
        lbi.guk_wins = Strings::ToInt(row[3]);
        lbi.wins += Strings::ToInt(row[4]);
        lbi.mir_wins = Strings::ToInt(row[4]);
        lbi.wins += Strings::ToInt(row[5]);
        lbi.mmc_wins = Strings::ToInt(row[5]);
        lbi.wins += Strings::ToInt(row[6]);
        lbi.ruj_wins = Strings::ToInt(row[6]);
        lbi.wins += Strings::ToInt(row[7]);
        lbi.tak_wins = Strings::ToInt(row[7]);
        lbi.losses = Strings::ToInt(row[8]);
        lbi.guk_losses = Strings::ToInt(row[8]);
        lbi.losses += Strings::ToInt(row[9]);
        lbi.mir_losses = Strings::ToInt(row[9]);
        lbi.losses += Strings::ToInt(row[10]);
        lbi.mmc_losses = Strings::ToInt(row[10]);
        lbi.losses += Strings::ToInt(row[11]);
        lbi.ruj_losses = Strings::ToInt(row[11]);
        lbi.losses += Strings::ToInt(row[12]);
        lbi.tak_losses = Strings::ToInt(row[12]);

        leaderboard_info_wins.push_back(lbi);
        leaderboard_info_percentage.push_back(lbi);
        leaderboard_info_wins_guk.push_back(lbi);
        leaderboard_info_percentage_guk.push_back(lbi);
        leaderboard_info_wins_mir.push_back(lbi);
        leaderboard_info_percentage_mir.push_back(lbi);
        leaderboard_info_wins_mmc.push_back(lbi);
        leaderboard_info_percentage_mmc.push_back(lbi);
        leaderboard_info_wins_ruj.push_back(lbi);
        leaderboard_info_percentage_ruj.push_back(lbi);
        leaderboard_info_wins_tak.push_back(lbi);
        leaderboard_info_percentage_tak.push_back(lbi);

        leaderboard_sorted_wins = false;
        leaderboard_sorted_percentage = false;
        leaderboard_sorted_wins_guk = false;
        leaderboard_sorted_percentage_guk = false;
        leaderboard_sorted_wins_mir = false;
        leaderboard_sorted_percentage_mir = false;
        leaderboard_sorted_wins_mmc = false;
        leaderboard_sorted_percentage_mmc = false;
        leaderboard_sorted_wins_ruj = false;
        leaderboard_sorted_percentage_ruj = false;
        leaderboard_sorted_wins_tak = false;
        leaderboard_sorted_percentage_tak = false;
	}
};

void AdventureManager::DoLeaderboardRequest(const char* player, uint8 type)
{
	switch(type)
	{
	case 1:
		if(!leaderboard_sorted_wins)
		{
			leaderboard_info_wins.sort(pred_sort_by_win_count);
			leaderboard_sorted_wins = true;
		}
		DoLeaderboardRequestWins(player);
		break;
	case 2:
		if(!leaderboard_sorted_percentage)
		{
			leaderboard_info_percentage.sort(pred_sort_by_win_percentage);
			leaderboard_sorted_percentage = true;
		}
		DoLeaderboardRequestPercentage(player);
		break;
	case 3:
		if(!leaderboard_sorted_wins_guk)
		{
			leaderboard_info_wins_guk.sort(pred_sort_by_win_count_guk);
			leaderboard_sorted_wins_guk = true;
		}
		DoLeaderboardRequestWinsGuk(player);
		break;
	case 4:
		if(!leaderboard_sorted_percentage_guk)
		{
			leaderboard_info_percentage_guk.sort(pred_sort_by_win_percentage_guk);
			leaderboard_sorted_percentage_guk = true;
		}
		DoLeaderboardRequestPercentageGuk(player);
		break;
	case 5:
		if(!leaderboard_sorted_wins_mir)
		{
			leaderboard_info_wins_mir.sort(pred_sort_by_win_count_mir);
			leaderboard_sorted_wins_mir = true;
		}
		DoLeaderboardRequestWinsMir(player);
		break;
	case 6:
		if(!leaderboard_sorted_percentage_mir)
		{
			leaderboard_info_percentage_mir.sort(pred_sort_by_win_percentage_mir);
			leaderboard_sorted_percentage_mir = true;
		}
		DoLeaderboardRequestPercentageMir(player);
		break;
	case 7:
		if(!leaderboard_sorted_wins_mmc)
		{
			leaderboard_info_wins_mmc.sort(pred_sort_by_win_count_mmc);
			leaderboard_sorted_wins_mmc = true;
		}
		DoLeaderboardRequestWinsMmc(player);
		break;
	case 8:
		if(!leaderboard_sorted_percentage_mmc)
		{
			leaderboard_info_percentage_mmc.sort(pred_sort_by_win_percentage_mmc);
			leaderboard_sorted_percentage_mmc = true;
		}
		DoLeaderboardRequestPercentageMmc(player);
		break;
	case 9:
		if(!leaderboard_sorted_wins_ruj)
		{
			leaderboard_info_wins_ruj.sort(pred_sort_by_win_count_ruj);
			leaderboard_sorted_wins_ruj = true;
		}
		DoLeaderboardRequestWinsRuj(player);
		break;
	case 10:
		if(!leaderboard_sorted_percentage_ruj)
		{
			leaderboard_info_percentage_ruj.sort(pred_sort_by_win_percentage_ruj);
			leaderboard_sorted_percentage_ruj = true;
		}
		DoLeaderboardRequestPercentageRuj(player);
		break;
	case 11:
		if(!leaderboard_sorted_wins_tak)
		{
			leaderboard_info_wins_tak.sort(pred_sort_by_win_count_tak);
			leaderboard_sorted_wins_tak = true;
		}
		DoLeaderboardRequestWinsTak(player);
		break;
	case 12:
		if(!leaderboard_sorted_percentage_tak)
		{
			leaderboard_info_percentage_tak.sort(pred_sort_by_win_percentage_tak);
			leaderboard_sorted_percentage_tak = true;
		}
		DoLeaderboardRequestPercentageTak(player);
		break;
	}
}

void AdventureManager::DoLeaderboardRequestWins(const char* player)
{
	ClientListEntry *pc = ClientList::Instance()->FindCharacter(player);
	if(pc)
	{
		auto pack = new ServerPacket(ServerOP_AdventureLeaderboard, 64 + sizeof(AdventureLeaderboard_Struct));
		AdventureLeaderboard_Struct *al = (AdventureLeaderboard_Struct*)(pack->pBuffer + 64);
		strcpy((char*)pack->pBuffer, player);

		int place = -1;
		int our_successes = 0;
		int our_failures = 0;
		int i = 0;
		auto iter = leaderboard_info_wins.begin();
		while(i < 100 && iter != leaderboard_info_wins.end())
		{
			LeaderboardInfo li = (*iter);
			if(li.name.compare(player) == 0)
			{
				place = i;
				our_successes = li.wins;
				our_failures = li.losses;
			}

			al->entries[i].success = li.wins;
			al->entries[i].failure = li.losses;
			strcpy(al->entries[i].name, li.name.c_str());
			i++;
			++iter;
		}

		if(place == -1 && iter != leaderboard_info_wins.end())
		{
			while(iter != leaderboard_info_wins.end())
			{
				LeaderboardInfo li = (*iter);
				if(li.name.compare(player) == 0)
				{
					place = i;
					our_successes = li.wins;
					our_failures = li.losses;
					break;
				}
				i++;
				++iter;
			}
		}

		if(place == -1)
		{
			al->our_rank = leaderboard_info_wins.size() + 1;
			al->success = 0;
			al->failure = 0;
		}
		else
		{
			al->our_rank = place;
			al->success = our_successes;
			al->failure = our_failures;
		}

		ZSList::Instance()->SendPacket(pc->zone(), pc->instance(), pack);
		delete pack;
	}
}

void AdventureManager::DoLeaderboardRequestPercentage(const char* player)
{
	ClientListEntry *pc = ClientList::Instance()->FindCharacter(player);
	if(pc)
	{
		auto pack = new ServerPacket(ServerOP_AdventureLeaderboard, 64 + sizeof(AdventureLeaderboard_Struct));
		AdventureLeaderboard_Struct *al = (AdventureLeaderboard_Struct*)(pack->pBuffer + 64);
		strcpy((char*)pack->pBuffer, player);

		int place = -1;
		int our_successes = 0;
		int our_failures = 0;
		int i = 0;
		auto iter = leaderboard_info_percentage.begin();
		while(i < 100 && iter != leaderboard_info_percentage.end())
		{
			LeaderboardInfo li = (*iter);
			if(li.name.compare(player) == 0)
			{
				place = i;
				our_successes = li.wins;
				our_failures = li.losses;
			}

			al->entries[i].success = li.wins;
			al->entries[i].failure = li.losses;
			strcpy(al->entries[i].name, li.name.c_str());
			i++;
			++iter;
		}

		if(place == -1 && iter != leaderboard_info_percentage.end())
		{
			while(iter != leaderboard_info_percentage.end())
			{
				LeaderboardInfo li = (*iter);
				if(li.name.compare(player) == 0)
				{
					place = i;
					our_successes = li.wins;
					our_failures = li.losses;
					break;
				}
				i++;
				++iter;
			}
		}

		if(place == -1)
		{
			al->our_rank = leaderboard_info_percentage.size() + 1;
			al->success = 0;
			al->failure = 0;
		}
		else
		{
			al->our_rank = place;
			al->success = our_successes;
			al->failure = our_failures;
		}

		ZSList::Instance()->SendPacket(pc->zone(), pc->instance(), pack);
		delete pack;
	}
}

void AdventureManager::DoLeaderboardRequestWinsGuk(const char* player)
{
	ClientListEntry *pc = ClientList::Instance()->FindCharacter(player);
	if(pc)
	{
		auto pack = new ServerPacket(ServerOP_AdventureLeaderboard, 64 + sizeof(AdventureLeaderboard_Struct));
		AdventureLeaderboard_Struct *al = (AdventureLeaderboard_Struct*)(pack->pBuffer + 64);
		strcpy((char*)pack->pBuffer, player);

		int place = -1;
		int our_successes = 0;
		int our_failures = 0;
		int i = 0;
		auto iter = leaderboard_info_wins_guk.begin();
		while(i < 100 && iter != leaderboard_info_wins_guk.end())
		{
			LeaderboardInfo li = (*iter);
			if(li.name.compare(player) == 0)
			{
				place = i;
				our_successes = li.guk_wins;
				our_failures = li.guk_losses;
			}

			al->entries[i].success = li.guk_wins;
			al->entries[i].failure = li.guk_losses;
			strcpy(al->entries[i].name, li.name.c_str());
			i++;
			++iter;
		}

		if(place == -1 && iter != leaderboard_info_wins_guk.end())
		{
			while(iter != leaderboard_info_wins_guk.end())
			{
				LeaderboardInfo li = (*iter);
				if(li.name.compare(player) == 0)
				{
					place = i;
					our_successes = li.guk_wins;
					our_failures = li.guk_losses;
					break;
				}
				i++;
				++iter;
			}
		}

		if(place == -1)
		{
			al->our_rank = leaderboard_info_wins_guk.size() + 1;
			al->success = 0;
			al->failure = 0;
		}
		else
		{
			al->our_rank = place;
			al->success = our_successes;
			al->failure = our_failures;
		}

		ZSList::Instance()->SendPacket(pc->zone(), pc->instance(), pack);
		delete pack;
	}
}

void AdventureManager::DoLeaderboardRequestPercentageGuk(const char* player)
{
	ClientListEntry *pc = ClientList::Instance()->FindCharacter(player);
	if(pc)
	{
		auto pack = new ServerPacket(ServerOP_AdventureLeaderboard, 64 + sizeof(AdventureLeaderboard_Struct));
		AdventureLeaderboard_Struct *al = (AdventureLeaderboard_Struct*)(pack->pBuffer + 64);
		strcpy((char*)pack->pBuffer, player);

		int place = -1;
		int our_successes = 0;
		int our_failures = 0;
		int i = 0;
		auto iter = leaderboard_info_percentage_guk.begin();
		while(i < 100 && iter != leaderboard_info_percentage_guk.end())
		{
			LeaderboardInfo li = (*iter);
			if(li.name.compare(player) == 0)
			{
				place = i;
				our_successes = li.guk_wins;
				our_failures = li.guk_losses;
			}

			al->entries[i].success = li.guk_wins;
			al->entries[i].failure = li.guk_losses;
			strcpy(al->entries[i].name, li.name.c_str());
			i++;
			++iter;
		}

		if(place == -1 && iter != leaderboard_info_percentage_guk.end())
		{
			while(iter != leaderboard_info_percentage_guk.end())
			{
				LeaderboardInfo li = (*iter);
				if(li.name.compare(player) == 0)
				{
					place = i;
					our_successes = li.guk_wins;
					our_failures = li.guk_losses;
					break;
				}
				i++;
				++iter;
			}
		}

		if(place == -1)
		{
			al->our_rank = leaderboard_info_percentage_guk.size() + 1;
			al->success = 0;
			al->failure = 0;
		}
		else
		{
			al->our_rank = place;
			al->success = our_successes;
			al->failure = our_failures;
		}

		ZSList::Instance()->SendPacket(pc->zone(), pc->instance(), pack);
		delete pack;
	}
}

void AdventureManager::DoLeaderboardRequestWinsMir(const char* player)
{
	ClientListEntry *pc = ClientList::Instance()->FindCharacter(player);
	if(pc)
	{
		auto pack = new ServerPacket(ServerOP_AdventureLeaderboard, 64 + sizeof(AdventureLeaderboard_Struct));
		AdventureLeaderboard_Struct *al = (AdventureLeaderboard_Struct*)(pack->pBuffer + 64);
		strcpy((char*)pack->pBuffer, player);

		int place = -1;
		int our_successes = 0;
		int our_failures = 0;
		int i = 0;
		auto iter = leaderboard_info_wins_mir.begin();
		while(i < 100 && iter != leaderboard_info_wins_mir.end())
		{
			LeaderboardInfo li = (*iter);
			if(li.name.compare(player) == 0)
			{
				place = i;
				our_successes = li.mir_wins;
				our_failures = li.mir_losses;
			}

			al->entries[i].success = li.mir_wins;
			al->entries[i].failure = li.mir_losses;
			strcpy(al->entries[i].name, li.name.c_str());
			i++;
			++iter;
		}

		if(place == -1 && iter != leaderboard_info_wins_mir.end())
		{
			while(iter != leaderboard_info_wins_mir.end())
			{
				LeaderboardInfo li = (*iter);
				if(li.name.compare(player) == 0)
				{
					place = i;
					our_successes = li.mir_wins;
					our_failures = li.mir_losses;
					break;
				}
				i++;
				++iter;
			}
		}

		if(place == -1)
		{
			al->our_rank = leaderboard_info_wins_mir.size() + 1;
			al->success = 0;
			al->failure = 0;
		}
		else
		{
			al->our_rank = place;
			al->success = our_successes;
			al->failure = our_failures;
		}

		ZSList::Instance()->SendPacket(pc->zone(), pc->instance(), pack);
		delete pack;
	}
}

void AdventureManager::DoLeaderboardRequestPercentageMir(const char* player)
{
	ClientListEntry *pc = ClientList::Instance()->FindCharacter(player);
	if(pc)
	{
		auto pack = new ServerPacket(ServerOP_AdventureLeaderboard, 64 + sizeof(AdventureLeaderboard_Struct));
		AdventureLeaderboard_Struct *al = (AdventureLeaderboard_Struct*)(pack->pBuffer + 64);
		strcpy((char*)pack->pBuffer, player);

		int place = -1;
		int our_successes = 0;
		int our_failures = 0;
		int i = 0;
		auto iter = leaderboard_info_percentage_mir.begin();
		while(i < 100 && iter != leaderboard_info_percentage_mir.end())
		{
			LeaderboardInfo li = (*iter);
			if(li.name.compare(player) == 0)
			{
				place = i;
				our_successes = li.mir_wins;
				our_failures = li.mir_losses;
			}

			al->entries[i].success = li.mir_wins;
			al->entries[i].failure = li.mir_losses;
			strcpy(al->entries[i].name, li.name.c_str());
			i++;
			++iter;
		}

		if(place == -1 && iter != leaderboard_info_percentage_mir.end())
		{
			while(iter != leaderboard_info_percentage_mir.end())
			{
				LeaderboardInfo li = (*iter);
				if(li.name.compare(player) == 0)
				{
					place = i;
					our_successes = li.mir_wins;
					our_failures = li.mir_losses;
					break;
				}
				i++;
				++iter;
			}
		}

		if(place == -1)
		{
			al->our_rank = leaderboard_info_percentage_mir.size() + 1;
			al->success = 0;
			al->failure = 0;
		}
		else
		{
			al->our_rank = place;
			al->success = our_successes;
			al->failure = our_failures;
		}

		ZSList::Instance()->SendPacket(pc->zone(), pc->instance(), pack);
		delete pack;
	}
}

void AdventureManager::DoLeaderboardRequestWinsMmc(const char* player)
{
	ClientListEntry *pc = ClientList::Instance()->FindCharacter(player);
	if(pc)
	{
		auto pack = new ServerPacket(ServerOP_AdventureLeaderboard, 64 + sizeof(AdventureLeaderboard_Struct));
		AdventureLeaderboard_Struct *al = (AdventureLeaderboard_Struct*)(pack->pBuffer + 64);
		strcpy((char*)pack->pBuffer, player);

		int place = -1;
		int our_successes = 0;
		int our_failures = 0;
		int i = 0;
		auto iter = leaderboard_info_wins_mmc.begin();
		while(i < 100 && iter != leaderboard_info_wins_mmc.end())
		{
			LeaderboardInfo li = (*iter);
			if(li.name.compare(player) == 0)
			{
				place = i;
				our_successes = li.mmc_wins;
				our_failures = li.mmc_losses;
			}

			al->entries[i].success = li.mmc_wins;
			al->entries[i].failure = li.mmc_losses;
			strcpy(al->entries[i].name, li.name.c_str());
			i++;
			++iter;
		}

		if(place == -1 && iter != leaderboard_info_wins_mmc.end())
		{
			while(iter != leaderboard_info_wins_mmc.end())
			{
				LeaderboardInfo li = (*iter);
				if(li.name.compare(player) == 0)
				{
					place = i;
					our_successes = li.mmc_wins;
					our_failures = li.mmc_losses;
					break;
				}
				i++;
				++iter;
			}
		}

		if(place == -1)
		{
			al->our_rank = leaderboard_info_wins_mmc.size() + 1;
			al->success = 0;
			al->failure = 0;
		}
		else
		{
			al->our_rank = place;
			al->success = our_successes;
			al->failure = our_failures;
		}

		ZSList::Instance()->SendPacket(pc->zone(), pc->instance(), pack);
		delete pack;
	}
}

void AdventureManager::DoLeaderboardRequestPercentageMmc(const char* player)
{
	ClientListEntry *pc = ClientList::Instance()->FindCharacter(player);
	if(pc)
	{
		auto pack = new ServerPacket(ServerOP_AdventureLeaderboard, 64 + sizeof(AdventureLeaderboard_Struct));
		AdventureLeaderboard_Struct *al = (AdventureLeaderboard_Struct*)(pack->pBuffer + 64);
		strcpy((char*)pack->pBuffer, player);

		int place = -1;
		int our_successes = 0;
		int our_failures = 0;
		int i = 0;
		auto iter = leaderboard_info_percentage_mmc.begin();
		while(i < 100 && iter != leaderboard_info_percentage_mmc.end())
		{
			LeaderboardInfo li = (*iter);
			if(li.name.compare(player) == 0)
			{
				place = i;
				our_successes = li.mmc_wins;
				our_failures = li.mmc_losses;
			}

			al->entries[i].success = li.mmc_wins;
			al->entries[i].failure = li.mmc_losses;
			strcpy(al->entries[i].name, li.name.c_str());
			i++;
			++iter;
		}

		if(place == -1 && iter != leaderboard_info_percentage_mmc.end())
		{
			while(iter != leaderboard_info_percentage_mmc.end())
			{
				LeaderboardInfo li = (*iter);
				if(li.name.compare(player) == 0)
				{
					place = i;
					our_successes = li.mmc_wins;
					our_failures = li.mmc_losses;
					break;
				}
				i++;
				++iter;
			}
		}

		if(place == -1)
		{
			al->our_rank = leaderboard_info_percentage_mmc.size() + 1;
			al->success = 0;
			al->failure = 0;
		}
		else
		{
			al->our_rank = place;
			al->success = our_successes;
			al->failure = our_failures;
		}

		ZSList::Instance()->SendPacket(pc->zone(), pc->instance(), pack);
		delete pack;
	}
}

void AdventureManager::DoLeaderboardRequestWinsRuj(const char* player)
{
	ClientListEntry *pc = ClientList::Instance()->FindCharacter(player);
	if(pc)
	{
		auto pack = new ServerPacket(ServerOP_AdventureLeaderboard, 64 + sizeof(AdventureLeaderboard_Struct));
		AdventureLeaderboard_Struct *al = (AdventureLeaderboard_Struct*)(pack->pBuffer + 64);
		strcpy((char*)pack->pBuffer, player);

		int place = -1;
		int our_successes = 0;
		int our_failures = 0;
		int i = 0;
		auto iter = leaderboard_info_wins_ruj.begin();
		while(i < 100 && iter != leaderboard_info_wins_ruj.end())
		{
			LeaderboardInfo li = (*iter);
			if(li.name.compare(player) == 0)
			{
				place = i;
				our_successes = li.ruj_wins;
				our_failures = li.ruj_losses;
			}

			al->entries[i].success = li.ruj_wins;
			al->entries[i].failure = li.ruj_losses;
			strcpy(al->entries[i].name, li.name.c_str());
			i++;
			++iter;
		}

		if(place == -1 && iter != leaderboard_info_wins_ruj.end())
		{
			while(iter != leaderboard_info_wins_ruj.end())
			{
				LeaderboardInfo li = (*iter);
				if(li.name.compare(player) == 0)
				{
					place = i;
					our_successes = li.ruj_wins;
					our_failures = li.ruj_losses;
					break;
				}
				i++;
				++iter;
			}
		}

		if(place == -1)
		{
			al->our_rank = leaderboard_info_wins_ruj.size() + 1;
			al->success = 0;
			al->failure = 0;
		}
		else
		{
			al->our_rank = place;
			al->success = our_successes;
			al->failure = our_failures;
		}

		ZSList::Instance()->SendPacket(pc->zone(), pc->instance(), pack);
		delete pack;
	}
}

void AdventureManager::DoLeaderboardRequestPercentageRuj(const char* player)
{
	ClientListEntry *pc = ClientList::Instance()->FindCharacter(player);
	if(pc)
	{
		auto pack = new ServerPacket(ServerOP_AdventureLeaderboard, 64 + sizeof(AdventureLeaderboard_Struct));
		AdventureLeaderboard_Struct *al = (AdventureLeaderboard_Struct*)(pack->pBuffer + 64);
		strcpy((char*)pack->pBuffer, player);

		int place = -1;
		int our_successes = 0;
		int our_failures = 0;
		int i = 0;
		auto iter = leaderboard_info_percentage_ruj.begin();
		while(i < 100 && iter != leaderboard_info_percentage_ruj.end())
		{
			LeaderboardInfo li = (*iter);
			if(li.name.compare(player) == 0)
			{
				place = i;
				our_successes = li.ruj_wins;
				our_failures = li.ruj_losses;
			}

			al->entries[i].success = li.ruj_wins;
			al->entries[i].failure = li.ruj_losses;
			strcpy(al->entries[i].name, li.name.c_str());
			i++;
			++iter;
		}

		if(place == -1 && iter != leaderboard_info_percentage_ruj.end())
		{
			while(iter != leaderboard_info_percentage_ruj.end())
			{
				LeaderboardInfo li = (*iter);
				if(li.name.compare(player) == 0)
				{
					place = i;
					our_successes = li.ruj_wins;
					our_failures = li.ruj_losses;
					break;
				}
				i++;
				++iter;
			}
		}

		if(place == -1)
		{
			al->our_rank = leaderboard_info_percentage_ruj.size() + 1;
			al->success = 0;
			al->failure = 0;
		}
		else
		{
			al->our_rank = place;
			al->success = our_successes;
			al->failure = our_failures;
		}

		ZSList::Instance()->SendPacket(pc->zone(), pc->instance(), pack);
		delete pack;
	}
}

void AdventureManager::DoLeaderboardRequestWinsTak(const char* player)
{
	ClientListEntry *pc = ClientList::Instance()->FindCharacter(player);
	if(pc)
	{
		auto pack = new ServerPacket(ServerOP_AdventureLeaderboard, 64 + sizeof(AdventureLeaderboard_Struct));
		AdventureLeaderboard_Struct *al = (AdventureLeaderboard_Struct*)(pack->pBuffer + 64);
		strcpy((char*)pack->pBuffer, player);

		int place = -1;
		int our_successes = 0;
		int our_failures = 0;
		int i = 0;
		auto iter = leaderboard_info_wins_ruj.begin();
		while(i < 100 && iter != leaderboard_info_wins_ruj.end())
		{
			LeaderboardInfo li = (*iter);
			if(li.name.compare(player) == 0)
			{
				place = i;
				our_successes = li.tak_wins;
				our_failures = li.tak_losses;
			}

			al->entries[i].success = li.tak_wins;
			al->entries[i].failure = li.tak_losses;
			strcpy(al->entries[i].name, li.name.c_str());
			i++;
			++iter;
		}

		if(place == -1 && iter != leaderboard_info_wins_ruj.end())
		{
			while(iter != leaderboard_info_wins_ruj.end())
			{
				LeaderboardInfo li = (*iter);
				if(li.name.compare(player) == 0)
				{
					place = i;
					our_successes = li.tak_wins;
					our_failures = li.tak_losses;
					break;
				}
				i++;
				++iter;
			}
		}

		if(place == -1)
		{
			al->our_rank = leaderboard_info_wins_ruj.size() + 1;
			al->success = 0;
			al->failure = 0;
		}
		else
		{
			al->our_rank = place;
			al->success = our_successes;
			al->failure = our_failures;
		}

		ZSList::Instance()->SendPacket(pc->zone(), pc->instance(), pack);
		delete pack;
	}
}

void AdventureManager::DoLeaderboardRequestPercentageTak(const char* player)
{
	ClientListEntry *pc = ClientList::Instance()->FindCharacter(player);
	if(pc)
	{
		auto pack = new ServerPacket(ServerOP_AdventureLeaderboard, 64 + sizeof(AdventureLeaderboard_Struct));
		AdventureLeaderboard_Struct *al = (AdventureLeaderboard_Struct*)(pack->pBuffer + 64);
		strcpy((char*)pack->pBuffer, player);

		int place = -1;
		int our_successes = 0;
		int our_failures = 0;
		int i = 0;
		auto iter = leaderboard_info_percentage_tak.begin();
		while(i < 100 && iter != leaderboard_info_percentage_tak.end())
		{
			LeaderboardInfo li = (*iter);
			if(li.name.compare(player) == 0)
			{
				place = i;
				our_successes = li.tak_wins;
				our_failures = li.tak_losses;
			}

			al->entries[i].success = li.tak_wins;
			al->entries[i].failure = li.tak_losses;
			strcpy(al->entries[i].name, li.name.c_str());
			i++;
			++iter;
		}

		if(place == -1 && iter != leaderboard_info_percentage_tak.end())
		{
			while(iter != leaderboard_info_percentage_tak.end())
			{
				LeaderboardInfo li = (*iter);
				if(li.name.compare(player) == 0)
				{
					place = i;
					our_successes = li.tak_wins;
					our_failures = li.tak_losses;
					break;
				}
				i++;
				++iter;
			}
		}

		if(place == -1)
		{
			al->our_rank = leaderboard_info_percentage_tak.size() + 1;
			al->success = 0;
			al->failure = 0;
		}
		else
		{
			al->our_rank = place;
			al->success = our_successes;
			al->failure = our_failures;
		}

		ZSList::Instance()->SendPacket(pc->zone(), pc->instance(), pack);
		delete pack;
	}
}

bool AdventureManager::PopFinishedEvent(const char *name, AdventureFinishEvent &fe)
{
	auto iter = finished_list.begin();
	while(iter != finished_list.end())
	{
		if((*iter).name.compare(name) == 0)
		{
			fe.name = (*iter).name;
			fe.points = (*iter).points;
			fe.theme = (*iter).theme;
			fe.win = (*iter).win;
			finished_list.erase(iter);
			Save();
			return true;
		}
		++iter;
	}
	return false;
}

void AdventureManager::SendAdventureFinish(AdventureFinishEvent fe)
{
	ClientListEntry *pc = ClientList::Instance()->FindCharacter(fe.name.c_str());
	if(pc)
	{
		auto pack = new ServerPacket(ServerOP_AdventureFinish, sizeof(ServerAdventureFinish_Struct));
		ServerAdventureFinish_Struct *af = (ServerAdventureFinish_Struct*)pack->pBuffer;
		strcpy(af->player, fe.name.c_str());
		af->theme = fe.theme;
		af->win = fe.win;
		af->points = fe.points;

		ZSList::Instance()->SendPacket(pc->zone(), pc->instance(), pack);
		delete pack;
	}
}

void AdventureManager::Save()
{
	//disabled for now
	return;

	std::stringstream ss(std::stringstream::in | std::stringstream::out);

	int number_of_elements = adventure_list.size();
	ss.write((const char*)&number_of_elements, sizeof(int));

	char null_term = 0;
	auto a_iter = adventure_list.begin();
	while(a_iter != adventure_list.end())
	{
		int cur = (*a_iter)->GetCount();
		ss.write((const char*)&cur, sizeof(int));

		cur = (*a_iter)->GetAssassinationCount();
		ss.write((const char*)&cur, sizeof(int));

		cur = (*a_iter)->GetTemplate()->id;
		ss.write((const char*)&cur, sizeof(int));

		cur = (int)(*a_iter)->GetStatus();
		ss.write((const char*)&cur, sizeof(int));

		cur = (*a_iter)->GetInstanceID();
		ss.write((const char*)&cur, sizeof(int));

		cur = (*a_iter)->GetRemainingTime();
		ss.write((const char*)&cur, sizeof(int));

		std::list<std::string> players = (*a_iter)->GetPlayers();
		cur = players.size();
		ss.write((const char*)&cur, sizeof(int));

		auto s_iter = players.begin();
		while(s_iter != players.end())
		{
			ss.write((const char*)(*s_iter).c_str(), (*s_iter).size());
			ss.write((const char*)&null_term, sizeof(char));
			s_iter++;
		}

		a_iter++;
	}

	number_of_elements = finished_list.size();
	ss.write((const char*)&number_of_elements, sizeof(int));
	auto f_iter = finished_list.begin();
	while(f_iter != finished_list.end())
	{
		ss.write((const char*)&(*f_iter).win, sizeof(bool));
		ss.write((const char*)&(*f_iter).points, sizeof(int));
		ss.write((const char*)&(*f_iter).theme, sizeof(int));
		ss.write((const char*)(*f_iter).name.c_str(), (*f_iter).name.size());
		ss.write((const char*)&null_term, sizeof(char));
		f_iter++;
	}

	FILE *f = fopen("adventure_state.dat", "w");
	if(f)
	{
		fwrite(ss.str().c_str(), ss.str().size(), 1, f);
		fclose(f);
	}
}

