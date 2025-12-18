#include "adventure.h"

#include "common/extprofile.h"
#include "common/misc_functions.h"
#include "common/random.h"
#include "common/repositories/character_corpses_repository.h"
#include "common/rulesys.h"
#include "common/servertalk.h"
#include "common/strings.h"
#include "common/zone_store.h"
#include "world/adventure_manager.h"
#include "world/cliententry.h"
#include "world/clientlist.h"
#include "world/worlddb.h"
#include "world/zonelist.h"

#include "glm/vec4.hpp"

extern EQ::Random emu_random;

Adventure::Adventure(AdventureTemplate *t)
{
	adventure_template = t;
	status = AS_WaitingForZoneIn;
	current_timer = new Timer(1000 * t->zone_in_time);
	count = 0;
	assassination_count = 0;
	instance_id = 0;
}

Adventure::Adventure(AdventureTemplate *t, int in_count, int in_assassination_count, AdventureStatus in_status, uint16 in_instance_id, uint32 time_left)
{
	adventure_template = t;
	count = in_count;
	assassination_count = in_assassination_count;
	status = in_status;
	instance_id = in_instance_id;

	if (in_status == AS_Finished) {
		database.SetInstanceDuration(instance_id, time_left);
	} else {
		database.SetInstanceDuration(instance_id, time_left + 60);
	}

	current_timer = new Timer(1000 * time_left);
}

Adventure::~Adventure()
{
	safe_delete(current_timer);
}

void Adventure::AddPlayer(std::string character_name, bool add_client_to_instance)
{
	if(!PlayerExists(character_name))
	{
		int32 character_id = database.GetCharacterID(character_name);
		if(character_id && add_client_to_instance)
		{
			database.AddClientToInstance(instance_id, character_id);
		}
		players.push_back(character_name);
	}
}

void Adventure::RemovePlayer(std::string character_name)
{
	auto iter = players.begin();
	while(iter != players.end())
	{
		if((*iter).compare(character_name) == 0)
		{
			int32 character_id = database.GetCharacterID(character_name);
			if (character_id)
			{
				database.RemoveClientFromInstance(instance_id, character_id);
				players.erase(iter);
			}
			return;
		}
		++iter;
	}
}

bool Adventure::PlayerExists(const std::string& character_name)
{
	auto iter = players.begin();
	while(iter != players.end())
	{
		if(character_name.compare((*iter)) == 0)
		{
			return true;
		}
		++iter;
	}
	return false;
}

bool Adventure::IsActive()
{
	return (status != AS_Finished);
}

bool Adventure::Process()
{
	if(players.size() == 0)
	{
		return false;
	}

	if(current_timer->Check())
	{
		//Timer wore out while waiting for zone in.
		if(status == AS_WaitingForZoneIn)
		{
			MoveCorpsesToGraveyard();
			database.DeleteInstance(instance_id);
			return false;
		}
		else if(status == AS_WaitingForPrimaryEndTime)
		{
			//Do partial failure: send a message to the clients that they can only get a certain amount of points.
			SendAdventureMessage(Chat::Red, "You failed to complete your adventure in time. Complete your adventure goal within 30 minutes to "
				"receive a lesser reward. This adventure will end in 30 minutes and your party will be ejected from the dungeon.");
			SetStatus(AS_WaitingForSecondaryEndTime);
		}
		else
		{
			if(count < GetTemplate()->type_count)
			{
				Finished(AWS_Lose);
			}

			MoveCorpsesToGraveyard();
			database.DeleteInstance(instance_id);
			return false;
		}
	}
	return true;
}

bool Adventure::CreateInstance()
{
	uint32 zone_id = ZoneID(adventure_template->zone);
	if(!zone_id)
	{
		return false;
	}

	uint16 id = 0;
	if(!database.GetUnusedInstanceID(id))
	{
		return false;
	}

	if(!database.CreateInstance(id, zone_id, adventure_template->zone_version, adventure_template->zone_in_time + 60))
	{
		return false;
	}

	instance_id = id;
	return true;
}

void Adventure::SetStatus(AdventureStatus new_status)
{
	if(new_status == AS_WaitingForPrimaryEndTime)
	{
		status = new_status;
		safe_delete(current_timer);
		current_timer = new Timer(adventure_template->duration * 1000);
		database.SetInstanceDuration(instance_id, adventure_template->duration + 60);
		auto pack = new ServerPacket(ServerOP_InstanceUpdateTime, sizeof(ServerInstanceUpdateTime_Struct));
		ServerInstanceUpdateTime_Struct *ut = (ServerInstanceUpdateTime_Struct*)pack->pBuffer;
		ut->instance_id = instance_id;
		ut->new_duration = adventure_template->duration + 60;

		ZSList::Instance()->SendPacket(0, instance_id, pack);
		safe_delete(pack);
	}
	else if(new_status == AS_WaitingForSecondaryEndTime)
	{
		status = new_status;
		safe_delete(current_timer);
		current_timer = new Timer(1800000);
		database.SetInstanceDuration(instance_id, 1860);
		auto pack = new ServerPacket(ServerOP_InstanceUpdateTime, sizeof(ServerInstanceUpdateTime_Struct));
		ServerInstanceUpdateTime_Struct *ut = (ServerInstanceUpdateTime_Struct*)pack->pBuffer;
		ut->instance_id = instance_id;
		ut->new_duration = 1860;

		ZSList::Instance()->SendPacket(0, instance_id, pack);
		safe_delete(pack);
	}
	else if(new_status == AS_Finished)
	{
		status = new_status;
		safe_delete(current_timer);
		current_timer = new Timer(1800000);
		database.SetInstanceDuration(instance_id, 1800);
		auto pack = new ServerPacket(ServerOP_InstanceUpdateTime, sizeof(ServerInstanceUpdateTime_Struct));
		ServerInstanceUpdateTime_Struct *ut = (ServerInstanceUpdateTime_Struct*)pack->pBuffer;
		ut->instance_id = instance_id;
		ut->new_duration = 1860;

		ZSList::Instance()->SendPacket(0, instance_id, pack);
		safe_delete(pack);
	}
	else
	{
		return;
	}

	auto iter = players.begin();
	while(iter != players.end())
	{
		AdventureManager::Instance()->GetAdventureData((*iter).c_str());
		++iter;
	}
}

void Adventure::SendAdventureMessage(uint32 type, const char *msg)
{
	auto pack = new ServerPacket(ServerOP_EmoteMessage, sizeof(ServerEmoteMessage_Struct) + strlen(msg) + 1);
	ServerEmoteMessage_Struct *sms = (ServerEmoteMessage_Struct*)pack->pBuffer;
	sms->type = type;
	strcpy(sms->message, msg);
	auto iter = players.begin();
	while(iter != players.end())
	{
		ClientListEntry *current = ClientList::Instance()->FindCharacter((*iter).c_str());
		if(current)
		{
			strcpy(sms->to, (*iter).c_str());
			ZSList::Instance()->SendPacket(current->zone(), current->instance(), pack);
		}
		++iter;
	}
	delete pack;
}

void Adventure::IncrementCount()
{
	const AdventureTemplate *at = GetTemplate();
	if(count >= at->type_count)
	{
		return;
	}

	if(status == AS_WaitingForPrimaryEndTime)
	{
		count++;
		if(count == at->type_count)
		{
			SetStatus(AS_Finished);
			Finished(AWS_Win);
		}
	}
	else if(status == AS_WaitingForSecondaryEndTime)
	{
		count++;
		if(count == at->type_count)
		{
			SetStatus(AS_Finished);
			Finished(AWS_SecondPlace);
		}
	}
}

void Adventure::IncrementAssassinationCount()
{
	if(count >= RuleI(Adventure, NumberKillsForBossSpawn))
	{
		return;
	}

	assassination_count++;
}

void Adventure::Finished(AdventureWinStatus ws)
{
	auto iter = players.begin();
	while (iter != players.end()) {
		ClientListEntry *current = ClientList::Instance()->FindCharacter((*iter).c_str());
		auto character_id = database.GetCharacterID(*iter);

		if (character_id == 0) {
			++iter;
			continue;
		}

		if (current) {
			if (current->Online() == CLE_Status::InZone) {
				//We can send our packets only.
				auto pack = new ServerPacket(ServerOP_AdventureFinish, sizeof(ServerAdventureFinish_Struct));
				ServerAdventureFinish_Struct *af = (ServerAdventureFinish_Struct*)pack->pBuffer;
				strcpy(af->player, (*iter).c_str());
				af->theme = GetTemplate()->theme;
				if (ws == AWS_Win) {
					af->win = true;
					af->points = GetTemplate()->win_points;
				}
				else if (ws == AWS_SecondPlace) {
					af->win = true;
					af->points = GetTemplate()->lose_points;
				}
				else {
					af->win = false;
					af->points = 0;
				}
				ZSList::Instance()->SendPacket(current->zone(), current->instance(), pack);
				database.UpdateAdventureStatsEntry(character_id, GetTemplate()->theme, (ws != AWS_Lose) ? true : false);
				delete pack;
			}
			else {
				AdventureFinishEvent afe;
				afe.name = (*iter);
				if (ws == AWS_Win) {
					afe.theme = GetTemplate()->theme;
					afe.points = GetTemplate()->win_points;
					afe.win = true;
				}
				else if (ws == AWS_SecondPlace) {
					afe.theme = GetTemplate()->theme;
					afe.points = GetTemplate()->lose_points;
					afe.win = true;
				}
				else {
					afe.win = false;
					afe.points = 0;
				}
				AdventureManager::Instance()->AddFinishedEvent(afe);
				database.UpdateAdventureStatsEntry(character_id, GetTemplate()->theme, (ws != AWS_Lose) ? true : false);
			}
		}
		else {
			AdventureFinishEvent afe;
			afe.name = (*iter);
			if (ws == AWS_Win) {
				afe.theme = GetTemplate()->theme;
				afe.points = GetTemplate()->win_points;
				afe.win = true;
			}
			else if (ws == AWS_SecondPlace) {
				afe.theme = GetTemplate()->theme;
				afe.points = GetTemplate()->lose_points;
				afe.win = true;
			}
			else {
				afe.win = false;
				afe.points = 0;
			}
			AdventureManager::Instance()->AddFinishedEvent(afe);
			database.UpdateAdventureStatsEntry(character_id, GetTemplate()->theme, (ws != AWS_Lose) ? true : false);
		}
		++iter;
	}
	AdventureManager::Instance()->GetAdventureData(this);
}

void Adventure::MoveCorpsesToGraveyard()
{
	if (GetTemplate()->graveyard_zone_id == 0) {
		return;
	}

	glm::vec4 position;

	float x = GetTemplate()->graveyard_x + EQ::Random::Instance()->Real(-GetTemplate()->graveyard_radius, GetTemplate()->graveyard_radius);
	float y = GetTemplate()->graveyard_y + EQ::Random::Instance()->Real(-GetTemplate()->graveyard_radius, GetTemplate()->graveyard_radius);
	float z = GetTemplate()->graveyard_z;

	position.x = x;
	position.y = y;
	position.z = z;
	position.w = 0.0f;

	CharacterCorpsesRepository::SendAdventureCorpsesToGraveyard(database, GetTemplate()->graveyard_zone_id, GetInstanceID(), position);

	const auto& l = CharacterCorpsesRepository::GetWhere(
		database,
		fmt::format(
			"`instance_id` = {}",
			GetInstanceID()
		)
	);

	for (const auto& e : l) {
		auto pack = new ServerPacket(ServerOP_DepopAllPlayersCorpses, sizeof(ServerDepopAllPlayersCorpses_Struct));

		auto d = (ServerDepopAllPlayersCorpses_Struct*) pack->pBuffer;

		d->CharacterID = e.charid;
		d->InstanceID  = 0;
		d->ZoneID      = GetTemplate()->graveyard_zone_id;

		ZSList::Instance()->SendPacket(0, GetInstanceID(), pack);

		delete pack;

		pack = new ServerPacket(ServerOP_SpawnPlayerCorpse, sizeof(SpawnPlayerCorpse_Struct));

		auto spc = (SpawnPlayerCorpse_Struct*) pack->pBuffer;

		spc->player_corpse_id = e.id;
		spc->zone_id          = GetTemplate()->graveyard_zone_id;

		ZSList::Instance()->SendPacket(spc->zone_id, 0, pack);

		delete pack;
	}
}

