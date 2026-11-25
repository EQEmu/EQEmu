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
#include "petitions.h"

#include "common/eq_packet_structs.h"
#include "common/eqemu_logsys.h"
#include "common/servertalk.h"
#include "common/strings.h"
#include "zone/entity.h"
#include "zone/worldserver.h"

#include <cstring>
#include <algorithm>
#include <cstdlib>

extern WorldServer worldserver;

void Petition::SendPetitionToPlayer(Client* clientto) {
	auto outapp = new EQApplicationPacket(OP_PetitionCheckout, sizeof(Petition_Struct));
	Petition_Struct* pet = (Petition_Struct*) outapp->pBuffer;
	strn0cpy(pet->accountid, GetAccountName(), sizeof(pet->accountid));
	strn0cpy(pet->lastgm, GetLastGM(), sizeof(pet->lastgm));
	strn0cpy(pet->charname, GetCharName(), sizeof(pet->charname));
	pet->petnumber = petid;
	pet->charclass = GetCharClass();
	pet->charlevel = GetCharLevel();
	pet->charrace = GetCharRace();
	pet->zone = GetZone();
	//strcpy(pet->zone,GetZone());
	strn0cpy(pet->petitiontext, GetPetitionText(), sizeof(pet->petitiontext));
	pet->checkouts = GetCheckouts();
	pet->unavail = GetUnavails();
	pet->senttime = GetSentTime();
	//memset(pet->unknown5, 0, sizeof(pet->unknown5));
	//pet->unknown5[3] = 0x1f;
	pet->urgency = GetUrgency();
	strn0cpy(pet->gmtext, GetGMText(), sizeof(pet->gmtext));
	clientto->QueuePacket(outapp);
	safe_delete(outapp);
	return;
}

Petition::Petition(uint32 id)
{
	petid = id;
	charclass = Class::None;
	charrace = Race::Doug;
	charlevel = 0;
	checkouts = 0;
	unavailables = 0;
	urgency = 0;
	time(&senttime);
	ischeckedout = false;
	memset(accountname, 0, sizeof(accountname));
	memset(charname, 0, sizeof(charname));
	memset(lastgm, 0, sizeof(lastgm));
	memset(petitiontext, 0, sizeof(petitiontext));
	memset(gmtext, 0, sizeof(gmtext));

	//memset(zone, 0, sizeof(zone));
	zone = 1;
}
Petition* PetitionList::GetPetitionByID(uint32 id_in) {
	LinkedListIterator<Petition*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->GetID() == id_in)
			return iterator.GetData();
		iterator.Advance();
	}
	return 0;
}
std::vector<uint32> PetitionList::GetPetitionIDs()
{
    std::vector<uint32> ids;
    LinkedListIterator<Petition*> iterator(list);

    iterator.Reset();

    while (iterator.MoreElements()) {
        ids.push_back(iterator.GetData()->GetID());
        iterator.Advance();
    }

    return ids;
}

uint32 PetitionList::GetTotalPetitions(){
	LinkedListIterator<Petition*> iterator(list);
	iterator.Reset();
	uint32 total=0;
	while(iterator.MoreElements()) {
		total++;
		iterator.Advance();
	}
	return total;
}
bool PetitionList::FindPetitionByAccountName(const char* acctname) {
	LinkedListIterator<Petition*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (!strcmp(acctname,iterator.GetData()->GetAccountName()))
			return true;
		iterator.Advance();
	}
	return false;
}
bool PetitionList::DeletePetitionByCharName(const char* charname) {
	LinkedListIterator<Petition*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (!strcmp(charname,iterator.GetData()->GetCharName())) {
			if(DeletePetition(iterator.GetData()->GetID())==0)
				return true;
			else
				return false;
		}
		iterator.Advance();
	}
	return false;
}
void PetitionList::UpdateZoneListQueue() {
	auto pack = new ServerPacket(ServerOP_Petition, sizeof(ServerPetitionUpdate_Struct));
	ServerPetitionUpdate_Struct* pupdate = (ServerPetitionUpdate_Struct*) pack->pBuffer;
	pupdate->petid = 0x00;
	pupdate->status = 0x00;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void PetitionList::AddPetition(Petition* pet) {
	list.Insert(pet);
	return;
}

//Return Values: 0 = Ok ; -1 = Error deleting petition.
int PetitionList::DeletePetition(uint32 petnumber, const char* owner)
{
    bool deleted = false;

    {
        std::scoped_lock lock(PList_Mutex);

        LinkedListIterator<Petition*> iterator(list);
        iterator.Reset();

        while (iterator.MoreElements()) {
            if (iterator.GetData()->GetID() == petnumber) {
                if (!database.DeletePetitionFromDB(iterator.GetData(), owner)) {
                    return -1;
                }

                iterator.RemoveCurrent(true);
                deleted = true;
                break;
            }

            iterator.Advance();
        }
    }

    if (!deleted) {
        return -1;
    }

    entity_list.RemovePetitionFromAdmins(petnumber);

    ReadDatabase();
    UpdateZoneListQueue();

    return 0;
}

void PetitionList::UpdateGMQueue() {
	LinkedListIterator<Petition*> iterator(list);
	iterator.Reset();
	uint32 total=0;
	while(iterator.MoreElements()) {
		total++;
		entity_list.SendPetitionToAdmins(iterator.GetData());
		iterator.Advance();
	}
	if(total==0)
		entity_list.SendPetitionToAdmins();
	return;
}

void PetitionList::ClearPetitions()
{
    LinkedListIterator<Petition*> iterator(list);
    iterator.Reset();

    while (iterator.MoreElements()) {
        // RemoveCurrent already advances the iterator.
        iterator.RemoveCurrent(true);
    }
}

void PetitionList::ReadDatabase()
{
    std::scoped_lock lock(PList_Mutex);

    const auto old_ids = GetPetitionIDs();

    // Preserve the existing cache if the query fails.
    if (!database.RefreshPetitionsFromDB()) {
        LogError("Failed to refresh petition cache");
        return;
    }

    const auto new_ids = GetPetitionIDs();

    for (const auto id : old_ids) {
        if (std::find(
                new_ids.begin(), new_ids.end(), id
            ) == new_ids.end()) {
            entity_list.RemovePetitionFromAdmins(id);
        }
    }

    UpdateGMQueue();
}

bool PetitionList::UpdatePetition(
    Petition* pet, const Petition& proposed)
{
    std::scoped_lock lock(PList_Mutex);

    Petition next = proposed;

    if (!database.UpdatePetitionToDB(pet, &next)) {
        return false;
    }

    *pet = next;
    return true;
}

bool ZoneDatabase::DeletePetitionFromDB(
    Petition* wpet, const char* owner)
{
    if (!wpet) {
        return false;
    }

    const auto dib =
        GetUniquePetitionRow(wpet->GetID());

    if (!dib) {
        return false;
    }

    std::string query = fmt::format(
        "DELETE FROM petitions "
        "WHERE dib = {} AND petid = {}",
        dib,
        wpet->GetID()
    );

    if (owner) {
        char escaped_owner[129] = {};

        DoEscapeString(
            escaped_owner, owner, strlen(owner)
        );

        query += fmt::format(
            " AND ischeckedout = 1 "
            "AND BINARY lastgm = BINARY '{}' "
            "AND checkouts = {}",
            escaped_owner,
            wpet->GetCheckouts()
        );
    }

    auto results = QueryDatabase(query);

    return results.Success() &&
           results.RowsAffected() == 1;
}

uint32 ZoneDatabase::GetUniquePetitionRow(
    uint32 petition_id)
{
    auto results = QueryDatabase(
        fmt::format(
            "SELECT dib FROM petitions "
            "WHERE petid = {} LIMIT 2",
            petition_id
        )
    );

    if (!results.Success() ||
        results.RowCount() != 1) {
        return 0;
    }

    auto row = results.begin();

    return static_cast<uint32>(
        std::strtoul(row[0], nullptr, 10)
    );
}

bool ZoneDatabase::OwnsPetition(
    Petition* pet, const char* owner)
{
    if (!pet || !owner) {
        return false;
    }

    const auto dib =
        GetUniquePetitionRow(pet->GetID());

    if (!dib) {
        return false;
    }

    char escaped_owner[129] = {};

    DoEscapeString(
        escaped_owner, owner, strlen(owner)
    );

    auto results = QueryDatabase(
        fmt::format(
            "SELECT dib FROM petitions "
            "WHERE dib = {} AND petid = {} "
            "AND ischeckedout = 1 "
            "AND BINARY lastgm = BINARY '{}' "
            "AND checkouts = {}",
            dib,
            pet->GetID(),
            escaped_owner,
            pet->GetCheckouts()
        )
    );

    return results.Success() &&
           results.RowCount() == 1;
}

bool ZoneDatabase::UpdatePetitionToDB(
    Petition* previous, Petition* proposed)
{
    if (!previous || !proposed ||
        previous->GetID() != proposed->GetID()) {
        return false;
    }

    const auto dib =
        GetUniquePetitionRow(previous->GetID());

    if (!dib) {
        return false;
    }

    char escaped_gmtext[2049] = {};
    char escaped_lastgm[129] = {};
    char escaped_previous_owner[129] = {};

    DoEscapeString(
        escaped_gmtext,
        proposed->GetGMText(),
        strlen(proposed->GetGMText())
    );

    DoEscapeString(
        escaped_lastgm,
        proposed->GetLastGM(),
        strlen(proposed->GetLastGM())
    );

    DoEscapeString(
        escaped_previous_owner,
        previous->GetLastGM(),
        strlen(previous->GetLastGM())
    );

    std::string query;

    if (!previous->CheckedOut() &&
        proposed->CheckedOut()) {

        // Atomic checkout. Never overwrite existing GM
        // notes, urgency or unavailable counters.

        query = fmt::format(
            "UPDATE petitions "
            "SET ischeckedout = 1, "
            "lastgm = '{}', "
            "checkouts = checkouts + 1 "
            "WHERE dib = {} AND petid = {} "
            "AND ischeckedout = 0 "
            "AND checkouts = {}",
            escaped_lastgm,
            dib,
            previous->GetID(),
            previous->GetCheckouts()
        );

    } else if (previous->CheckedOut() &&
               !proposed->CheckedOut()) {

        // CheckIn or UnCheckout. Only the current
        // checkout owner may release the petition.

        query = fmt::format(
            "UPDATE petitions "
            "SET ischeckedout = 0, "
            "gmtext = '{}', "
            "lastgm = '{}', "
            "urgency = {}, "
            "unavailables = {} "
            "WHERE dib = {} AND petid = {} "
            "AND ischeckedout = 1 "
            "AND BINARY lastgm = BINARY '{}' "
            "AND checkouts = {}",
            escaped_gmtext,
            escaped_lastgm,
            proposed->GetUrgency(),
            proposed->GetUnavails(),
            dib,
            previous->GetID(),
            escaped_previous_owner,
            previous->GetCheckouts()
        );

    } else {
        return false;
    }

    auto results = QueryDatabase(query);

    if (!results.Success()) {
        LogError(
            "Petition update failed for [{}]",
            previous->GetID()
        );
        return false;
    }

    return results.RowsAffected() == 1;
}

void ZoneDatabase::InsertPetitionToDB(Petition* wpet)
{

	uint32 len = strlen(wpet->GetPetitionText());
	auto petitiontext = new char[2 * len + 1];
	memset(petitiontext, 0, 2*len+1);
	DoEscapeString(petitiontext, wpet->GetPetitionText(), len);

	std::string query = StringFormat("INSERT INTO petitions "
                                    "(petid, charname, accountname, lastgm, "
                                    "petitiontext, zone, urgency, charclass, "
                                    "charrace, charlevel, checkouts, unavailables, "
                                    "ischeckedout, senttime, gmtext) "
                                    "VALUES (%i, '%s', '%s', '%s', '%s', "
                                    "%i, %i, %i, %i, %i, "
                                    "%i, %i, %i, %i, '%s')",
                                    wpet->GetID(), wpet->GetCharName(), wpet->GetAccountName(), wpet->GetLastGM(),
                                    petitiontext, wpet->GetZone(), wpet->GetUrgency(), wpet->GetCharClass(),
                                    wpet->GetCharRace(), wpet->GetCharLevel(), wpet->GetCheckouts(), wpet->GetUnavails(),
                                    wpet->CheckedOut()? 1: 0, wpet->GetSentTime(), wpet->GetGMText());
    safe_delete_array(petitiontext);
    auto results = QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

#if EQDEBUG >= 5
		LogDebug("New petition created");
#endif

}

bool ZoneDatabase::RefreshPetitionsFromDB()
{
	Petition* newpet;
	std::string query = "SELECT petid, charname, accountname, lastgm, petitiontext, "
                        "zone, urgency, charclass, charrace, charlevel, checkouts, "
                        "unavailables, ischeckedout, senttime, gmtext "
                        "FROM petitions ORDER BY petid";
    auto results = QueryDatabase(query);
	if (!results.Success()) {
        return false;
    }

    PetitionList::Instance()->ClearPetitions();

    for (auto row = results.begin(); row != results.end(); ++row) {
        newpet = new Petition(Strings::ToInt(row[0]));
        newpet->SetCName(row[1]);
        newpet->SetAName(row[2]);
        newpet->SetLastGM(row[3]);
        newpet->SetPetitionText(row[4]);
        newpet->SetZone(Strings::ToInt(row[5]));
        newpet->SetUrgency(Strings::ToInt(row[6]));
        newpet->SetClass(Strings::ToInt(row[7]));
        newpet->SetRace(Strings::ToInt(row[8]));
        newpet->SetLevel(Strings::ToInt(row[9]));
        newpet->SetCheckouts(Strings::ToInt(row[10]));
        newpet->SetUnavails(Strings::ToInt(row[11]));
        newpet->SetSentTime(atol(row[13]));
        newpet->SetGMText(row[14]);

        if (Strings::ToInt(row[12]) == 1)
            newpet->SetCheckedOut(true);
        else
            newpet->SetCheckedOut(false);
        PetitionList::Instance()->AddPetition(newpet);
    }

    return true;
}
