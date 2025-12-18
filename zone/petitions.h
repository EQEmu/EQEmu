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

#pragma once

#include "common/linked_list.h"
#include "common/misc_functions.h"
#include "common/mutex.h"
#include "common/types.h"
#include "common/zone_store.h"
#include "zone/client.h"
#include "zone/zonedb.h"

class Client;

class Petition
{
public:
	Petition(uint32 id);
	~Petition() {}
	void SendPetitionToPlayer(Client* clientto);


	uint32 GetID()			{ return this->petid; }
	uint32 GetUnavails()		{ return unavailables; }
	uint32 GetCheckouts()	{ return checkouts; }
	uint32 GetCharLevel()	{ return charlevel; }
	uint32 GetCharRace()		{ return charrace; }
	uint32 GetCharClass()	{ return charclass; }
	uint32 GetUrgency()		{ return urgency; }
	//char* GetZone()		{ return this->zone; }
	uint32	GetZone()		{ return this->zone; }
	char* GetCharName()	{ return charname; }
	char* GetAccountName()	{ return accountname; }
	char* GetLastGM()		{ return lastgm; }
	time_t GetSentTime()	{ return senttime; }
	char* GetPetitionText() { return petitiontext; }
	char* GetGMText()		{ return gmtext; }
	bool CheckedOut()				{ return ischeckedout; }


// Set Petition Stuff Functions
	void SetCheckedOut(bool status_in)	{ ischeckedout = status_in; }
	void SetUrgency(uint32 urgency_in)	{ urgency = urgency_in; }
	void SetClass(uint32 class_in)	{ charclass = class_in; }
	void SetRace(uint32 race_in)		{ charrace = race_in; }
	void SetLevel(uint32 level_in)	{ charlevel = level_in; }
	void AddCheckout()				{ checkouts = checkouts + 1; }
	void AddUnavails()				{ unavailables = unavailables + 1; }
	//void SetZone(char* zone_in)		{ strcpy(this->zone, zone_in); }
	void SetZone(uint32 zone_in)			{ this->zone = zone_in; }
	void SetCName(const char* name_in)		{ strcpy(charname, name_in); }
	void SetAName(const char* name_in)		{ strcpy(accountname, name_in); }
	void SetLastGM(const char* gm_in)		{ strcpy(lastgm, gm_in); }
	void SetGMText(const char* gmtext_in)	{ if(gmtext_in) strcpy(gmtext, gmtext_in); }
	void SetPetitionText(char* pet_in) { strn0cpy(petitiontext, pet_in, sizeof(petitiontext)); }
	void SetPetID(uint32 id_in)		{ petid = id_in; }
	void SetCheckouts(uint32 checks_in) { checkouts = checks_in; }
	void SetUnavails(uint32 unavails_in) { unavailables = unavails_in; }
	void SetSentTime(time_t senttime_in) { senttime = senttime_in; }

protected:

	uint32 petid;
	char charname[64];
	char accountname[32];
	char lastgm[64];
	char petitiontext[1024];
	char gmtext[1024];
	//char zone[32];
	uint32 zone;
	uint32 urgency; // 0 = green, 1 = yellow, 2 = red
	uint32 charclass;
	uint32 charrace;
	uint32 charlevel;
	uint32 checkouts;
	uint32 unavailables;
	time_t senttime;
	bool ischeckedout;
};

class PetitionList
{
public:
	int DeletePetition(uint32 petnumber);
	void UpdateGMQueue();
	PetitionList() {}
	~PetitionList() {}
	void AddPetition(Petition* pet);
	Petition* GetPetitionByID(uint32 id_in);
	uint32	GetTotalPetitions();
	void ClearPetitions();
	void ReadDatabase();
	void UpdatePetition(Petition* pet);
	void UpdateZoneListQueue();
	bool FindPetitionByAccountName(const char* acctname);
	bool DeletePetitionByCharName(const char* charname);

	static PetitionList* Instance()
	{
		static PetitionList instance;
		return &instance;
	}

private:
	LinkedList<Petition*> list;
	Mutex PList_Mutex;
};
