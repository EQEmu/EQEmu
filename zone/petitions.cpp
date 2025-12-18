/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "petitions.h"

#include "common/eq_packet_structs.h"
#include "common/eqemu_logsys.h"
#include "common/servertalk.h"
#include "common/strings.h"
#include "zone/entity.h"
#include "zone/worldserver.h"

#include <cstring>

extern WorldServer worldserver;

void Petition::SendPetitionToPlayer(Client* clientto) {
	auto outapp = new EQApplicationPacket(OP_PetitionCheckout, sizeof(Petition_Struct));
	Petition_Struct* pet = (Petition_Struct*) outapp->pBuffer;
	strcpy(pet->accountid,GetAccountName());
	strcpy(pet->lastgm,GetLastGM());
	strcpy(pet->charname,GetCharName());
	pet->petnumber = petid;
	pet->charclass = GetCharClass();
	pet->charlevel = GetCharLevel();
	pet->charrace = GetCharRace();
	pet->zone = GetZone();
	//strcpy(pet->zone,GetZone());
	strcpy(pet->petitiontext,GetPetitionText());
	pet->checkouts = GetCheckouts();
	pet->unavail = GetUnavails();
	pet->senttime = GetSentTime();
	//memset(pet->unknown5, 0, sizeof(pet->unknown5));
	//pet->unknown5[3] = 0x1f;
	pet->urgency = GetUrgency();
	strcpy(pet->gmtext, GetGMText());
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
int PetitionList::DeletePetition(uint32 petnumber) {
	LinkedListIterator<Petition*> iterator(list);
	iterator.Reset();
	LockMutex lock(&PList_Mutex);
	while(iterator.MoreElements()) {
		if (iterator.GetData()->GetID() == petnumber) {
			database.DeletePetitionFromDB(iterator.GetData());
			iterator.RemoveCurrent();
			return 0;
			break;
		}
		else {
			iterator.Advance();
		}
	}
	return -1;
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

void PetitionList::ClearPetitions() {
	//	entity_list.ClearClientPetitionQueue();
	LinkedListIterator<Petition*> iterator(list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		iterator.RemoveCurrent(true);
		iterator.Advance();
	}
	return;
}

void PetitionList::ReadDatabase() {
	LockMutex lock(&PList_Mutex);
	ClearPetitions();
	database.RefreshPetitionsFromDB();
	UpdateGMQueue();
	return;
}

void PetitionList::UpdatePetition(Petition* pet) {
	LockMutex lock(&PList_Mutex);
	database.UpdatePetitionToDB(pet);
	return;
}

void ZoneDatabase::DeletePetitionFromDB(Petition* wpet) {

    std::string query = StringFormat("DELETE FROM petitions WHERE petid = %i", wpet->GetID());
    auto results = QueryDatabase(query);
}

void ZoneDatabase::UpdatePetitionToDB(Petition* wpet) {

	std::string query = StringFormat("UPDATE petitions SET gmtext = '%s', lastgm = '%s', urgency = %i, "
                                    "checkouts = %i, unavailables = %i, ischeckedout = %i "
                                    "WHERE petid = %i",
                                    wpet->GetGMText(), wpet->GetLastGM(), wpet->GetUrgency(),
                                    wpet->GetCheckouts(), wpet->GetUnavails(),
                                    wpet->CheckedOut() ? 1: 0, wpet->GetID());
    auto results = QueryDatabase(query);
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

void ZoneDatabase::RefreshPetitionsFromDB()
{
	Petition* newpet;
	std::string query = "SELECT petid, charname, accountname, lastgm, petitiontext, "
                        "zone, urgency, charclass, charrace, charlevel, checkouts, "
                        "unavailables, ischeckedout, senttime, gmtext "
                        "FROM petitions ORDER BY petid";
    auto results = QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

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

}
