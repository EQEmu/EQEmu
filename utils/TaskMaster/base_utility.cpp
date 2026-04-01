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
#include <wx/wx.h>
#include <mysql.h>
#include <vector>
#include "tasks.h"
#include "utility.h"
#include "items.h"
#include "base.h"
#include "ErrorLog.h"

using namespace std;

bool MainFrame::GetDatabaseSettings(){
	FILE *mFile = NULL;
	mFile = fopen("db.txt", "r");

	//very little error checking
	//we don't make sure the values in the file are valid
	//or all there just that the file exists so:
	//don't fuck this up.
	if(mFile){
		//this isn't all that safe
		char chunk[256];
		memset(chunk, 0, 256);

		fgets(chunk, 256, mFile);
		strcpy(server, chunk);
		for(int i = 0; i<256; i++){
			if(server[i] == '\n')
				server[i] = '\0';
		}
		memset(chunk, 0, 256);

		fgets(chunk, 256, mFile);
		strcpy(database, chunk);
		for(int i = 0; i<256; i++){
			if(database[i] == '\n')
				database[i] = '\0';
		}
		memset(chunk, 0, 256);

		fgets(chunk, 256, mFile);
		strcpy(user, chunk);
		for(int i = 0; i<256; i++){
			if(user[i] == '\n')
				user[i] = '\0';
		}
		memset(chunk, 0, 256);

		fgets(chunk, 256, mFile);
		strcpy(password, chunk);
		for(int i = 0; i<256; i++){
			if(password[i] == '\n')
				password[i] = '\0';
		}
		memset(chunk, 0, 256);
	}
	else{
		mErrorLog->Log(eqEmuLogBoth, "Error opening file db.txt, could not read db settings.");
		return false;
	}
	return true;
}

bool MainFrame::LoadItems(){
	if(mMysql){
		mErrorLog->Log(eqEmuLogBoth, "Loading Items...");
		unsigned int itemsLoaded = 0;
		MYSQL_RES *res;
		MYSQL_ROW row;

		if (mysql_query(mMysql, "SELECT name,id FROM items")) {
			mErrorLog->Log(eqEmuLogBoth, "MySQL Connection Error: %s", mysql_error(mMysql));
			return false;
		}
		res = mysql_use_result(mMysql);
		while ((row = mysql_fetch_row(res)) != NULL){
			eqitem newIT;
			strcpy(newIT.name, row[0]);
			newIT.id = Strings::ToInt(row[1]);
			itemList.push_back(newIT);
			itemsLoaded++;
		}
		mErrorLog->Log(eqEmuLogBoth, "%u Successfully Loaded Items", itemsLoaded);
		mysql_free_result(res);
	}
	else{
		mErrorLog->Log(eqEmuLogBoth, "Mysql connection did not exist for item load.");
		return false;
	}
	return true;
}

bool MainFrame::LoadZones()
{
	if(mMysql){
		mErrorLog->Log(eqEmuLogBoth, "Loading Zones...");
		unsigned int zonesLoaded = 0;
		MYSQL_RES *res;
		MYSQL_ROW row;

		if (mysql_query(mMysql, "SELECT short_name,zoneidnumber FROM zone")) {
			mErrorLog->Log(eqEmuLogBoth, "MySQL Connection Error: %s", mysql_error(mMysql));
			return false;
		}

		wxString zoneStr;
		zoneStr.Printf("**Unknown Zone**");
		int * id = new int;
		*id = -1;
		mStartZone->Append(zoneStr, (void*)id);

		zoneStr.Clear();
		zoneStr.Printf("*Any Zone*");
		id = new int;
		*id = 0;
		mStartZone->Append(zoneStr, (void*)id);

		mActivityZone->Append(zoneStr, (void*)id);
		mProxZone->Append(zoneStr, (void*)id);
		zoneStr.clear();
		mStartZone->Select(0);
		mActivityZone->Select(0);
		mProxZone->Select(0);

		res = mysql_use_result(mMysql);
		while ((row = mysql_fetch_row(res)) != NULL){
			eqtask_zones newZ;
			strcpy(newZ.name, row[0]);

			newZ.id = Strings::ToInt(row[1]);
			taskZoneList.push_back(newZ);

			int * zoneId = new int;
			*zoneId = newZ.id;
			wxString zoneNameStr;

			zoneNameStr.Printf("%s", newZ.name);
			mStartZone->Append(zoneNameStr, (void*)zoneId);
			mActivityZone->Append(zoneNameStr, (void*)zoneId);
			mProxZone->Append(zoneNameStr, (void*)zoneId);
			zoneNameStr.clear();

			zonesLoaded++;
		}
		mErrorLog->Log(eqEmuLogBoth, "%u Successfully Loaded Zones", zonesLoaded);
		mysql_free_result(res);
	}
	else{
		mErrorLog->Log(eqEmuLogBoth, "Mysql connection did not exist for zone load.");
		return false;
	}
	return true;
}

bool MainFrame::LoadTasks()
{
	if(mMysql){
		mErrorLog->Log(eqEmuLogBoth, "Loading Tasks...");
		unsigned int tasksLoaded = 0;
		MYSQL_RES *res;
		MYSQL_ROW row;

		if (mysql_query(mMysql, "SELECT id, title, description, reward, rewardid, cashreward, xpreward, rewardmethod, startzone, duration, minlevel, maxlevel, repeatable FROM tasks")) {
			mErrorLog->Log(eqEmuLogBoth, "MySQL Connection Error: %s", mysql_error(mMysql));
			return false;
		}

		res = mysql_use_result(mMysql);
		while ((row = mysql_fetch_row(res)) != NULL){
			eqtask newT;
			newT.id = Strings::ToInt(row[0]);

			//This isn't all that safe
			//Working under the assumption that:
			//Any database you connect to is not
			//Going to want to hurt you
			strcpy(newT.title, row[1]);
			strcpy(newT.desc, row[2]);
			strcpy(newT.reward, row[3]);

			wxString str;
			str.Printf("%d:%s", newT.id, newT.title);
			ItemSelectionList->Append(str);

			if(newT.id > highestIndex)
				highestIndex = newT.id;

			newT.rewardid = Strings::ToInt(row[4]);
			newT.cashreward = Strings::ToInt(row[5]);
			newT.xpreward = Strings::ToInt(row[6]);
			newT.rewardmethod = Strings::ToInt(row[7]);
			newT.startzone = Strings::ToInt(row[8]);
			newT.duration = Strings::ToInt(row[9]);
			newT.level_min = Strings::ToInt(row[10]);
			newT.level_max = Strings::ToInt(row[11]);
			newT.repeatable = Strings::ToInt(row[12]) ? true : false;

			taskList.push_back(newT);

			tasksLoaded++;
		}
		mErrorLog->Log(eqEmuLogBoth, "%u Successfully Loaded Tasks", tasksLoaded);
		mysql_free_result(res);
	}
	else{
		mErrorLog->Log(eqEmuLogBoth, "Mysql connection did not exist for task load.");
		return false;
	}
	return true;
}

bool MainFrame::LoadGoals()
{
	if(mMysql){
		mErrorLog->Log(eqEmuLogBoth, "Loading Goals...");
		unsigned int goalsLoaded = 0;
		MYSQL_RES *res;
		MYSQL_ROW row;

		if (mysql_query(mMysql, "SELECT listid, entry FROM goallists")) {
			mErrorLog->Log(eqEmuLogBoth, "MySQL Connection Error: %s", mysql_error(mMysql));
			return false;
		}

		res = mysql_use_result(mMysql);
		while ((row = mysql_fetch_row(res)) != NULL){
			eqtask_goallist newGL;
			newGL.id = Strings::ToInt(row[0]);
			newGL.value = Strings::ToInt(row[1]);
			goalTaskList.push_back(newGL);

			goalsLoaded++;
		}
		mErrorLog->Log(eqEmuLogBoth, "%u Successfully Loaded Goals", goalsLoaded);
		mysql_free_result(res);
	}
	else{
		mErrorLog->Log(eqEmuLogBoth, "Mysql connection did not exist for goal load.");
		return false;
	}
	return true;
}

bool MainFrame::LoadActivities() {
	if (mMysql) {
		mErrorLog->Log(eqEmuLogBoth, "Loading Activities...");
		unsigned int activitiesLoaded = 0;
		MYSQL_RES    *res;
		MYSQL_ROW    row;

		if (mysql_query(mMysql,
		                "SELECT taskid, activityid, step, activitytype, text1, text2, text3, goalid, goalmethod, goalcount, delivertonpc, zoneid, optional FROM `task_activities`")) {
			mErrorLog->Log(eqEmuLogBoth, "MySQL Connection Error: %s", mysql_error(mMysql));
			return false;
		}

		res         = mysql_use_result(mMysql);
		while ((row = mysql_fetch_row(res)) != NULL) {
			eqtask_activities newAL;

			newAL.id           = Strings::ToInt(row[0]);
			newAL.activityId   = Strings::ToInt(row[1]);
			newAL.step         = Strings::ToInt(row[2]);
			newAL.activityType = Strings::ToInt(row[3]);
			strcpy(newAL.text1, row[4]);
			strcpy(newAL.text2, row[5]);
			strcpy(newAL.text3, row[6]);
			newAL.goalid       = Strings::ToInt(row[7]);
			newAL.goalmethod   = Strings::ToInt(row[8]);
			newAL.goalcount    = Strings::ToInt(row[9]);
			newAL.deliverToNpc = Strings::ToInt(row[10]);
			newAL.zoneid       = Strings::ToInt(row[11]);
			newAL.optional     = Strings::ToInt(row[12]) ? true : false;

			taskActivitiesList.push_back(newAL);
			activitiesLoaded++;
		}
		mErrorLog->Log(eqEmuLogBoth, "%u Successfully Loaded Activities", activitiesLoaded);
		mysql_free_result(res);
	} else {
		mErrorLog->Log(eqEmuLogBoth, "Mysql connection did not exist for activity load.");
		return false;
	}
	return true;
}

bool MainFrame::LoadProximity()
{
	if(mMysql){
		mErrorLog->Log(eqEmuLogBoth, "Loading Proximities...");
		unsigned int proximitiesLoaded = 0;
		MYSQL_RES *res;
		MYSQL_ROW row;

		if (mysql_query(mMysql, "SELECT zoneid, exploreid, minx, maxx, miny, maxy, minz, maxz FROM proximities")) {
			mErrorLog->Log(eqEmuLogBoth, "MySQL Connection Error: %s", mysql_error(mMysql));
			return false;
		}

		res = mysql_use_result(mMysql);
		while ((row = mysql_fetch_row(res)) != NULL){
			eqtask_proximity newPR;

			newPR.zoneid = Strings::ToInt(row[0]);
			newPR.exploreid = Strings::ToInt(row[1]);
			newPR.minx = atof(row[2]);
			newPR.maxx = atof(row[3]);
			newPR.miny = atof(row[4]);
			newPR.maxy = atof(row[5]);
			newPR.minz = atof(row[6]);
			newPR.maxz = atof(row[7]);

			taskProximityList.push_back(newPR);
			proximitiesLoaded++;
		}
		mErrorLog->Log(eqEmuLogBoth, "%u Successfully Loaded Proximities", proximitiesLoaded);
		mysql_free_result(res);
	}
	else{
		mErrorLog->Log(eqEmuLogBoth, "Mysql connection did not exist for proximity load.");
		return false;
	}
	return true;
}

void MainFrame::ShowRewardChange(int rewardType, int rewardId)
{
	if(rewardType == 2){
		ShowRewardItems->Clear();
	}
	else if(rewardType == 0 && rewardId != 0){
		ShowRewardItems->Clear();
		vector<eqitem>::iterator Iter;

		for(Iter = itemList.begin(); Iter!=itemList.end(); Iter++)
		{
			eqitem eqi = *Iter;
			if(eqi.id == rewardId){
				wxString itStr;
				itStr.Printf("%u: %s", eqi.id, eqi.name);
				ShowRewardItems->Append(itStr);
				itStr.clear();
			}
		}
	}
	else if(rewardType == 1 && rewardId != 0)
	{
		ShowRewardItems->Clear();
		vector<eqtask_goallist>::iterator Iter;
		for(Iter = goalTaskList.begin(); Iter!=goalTaskList.end(); Iter++)
		{
			eqtask_goallist gli = *Iter;
			if(gli.id == rewardId)
			{
				vector<eqitem>::iterator itemIter;
				for(itemIter = itemList.begin(); itemIter!=itemList.end(); itemIter++)
				{
					eqitem eqi = *itemIter;
					if(eqi.id == gli.value)
					{
						wxString itStr;
						itStr.Printf("%u: %s", eqi.id, eqi.name);
						ShowRewardItems->Append(itStr);
						itStr.clear();
					}
				}
			}
		}
	}
	else{
		ShowRewardItems->Clear();
	}
}

void MainFrame::SetZoneSelectionById(int zid)
{
	for(unsigned int x = 0; x < mStartZone->GetCount(); x++)
	{
		int *i;
		i = (int*) mStartZone->GetClientData(x);
		if(i){
			if(*i == zid){
				mStartZone->Select(x);
				return;
			}
		}
		else
		{
			mErrorLog->Log(eqEmuLogBoth, "MainFrame::SetZoneSelectionById: i was NULL");
		}
	}
	mStartZone->Select(0); //we have no valid zone
}

void MainFrame::SetZoneSelectionByIdActivity(int zid)
{
	for(unsigned int x = 0; x < mActivityZone->GetCount(); x++)
	{
		int *i;
		i = (int*) mActivityZone->GetClientData(x);
		if(i){
			if(*i == zid){
				mActivityZone->Select(x);
				return;
			}
		}
		else
		{
			mErrorLog->Log(eqEmuLogBoth, "MainFrame::SetZoneSelectionByIdActivity: i was NULL");
		}
	}
	mActivityZone->Select(0); //we have no valid zone
}

void MainFrame::SetZoneSelectionByIdProximity(int zid)
{
	for(unsigned int x = 0; x < mProxZone->GetCount(); x++)
	{
		int *i;
		i = (int*) mProxZone->GetClientData(x);
		if(i){
			if(*i == zid){
				mProxZone->Select(x);
				return;
			}
		}
		else
		{
			mErrorLog->Log(eqEmuLogBoth, "MainFrame::SetZoneSelectionByIdProximity: i was NULL");
		}
	}
	mProxZone->Select(0); //we have no valid zone
}

wxString MainFrame::MakeStringSQLSafe(const char * c)
{
	wxString ret;
	ret.Printf("%s", c);
	ret.Replace("\'", "\\\'");
	return ret;
}
