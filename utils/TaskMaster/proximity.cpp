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
#include <wx/numdlg.h>
#include <mysql.h>
#include <vector>
#include "tasks.h"
#include "utility.h"
#include "base.h"
#include "ErrorLog.h"

using namespace std;

void MainFrame::ProximityListBoxDoubleClick(wxCommandEvent& event)
{
	eqtask_prox *eqp = (eqtask_prox*)ProximitySelectionList->GetClientData(event.GetInt());
	if(eqp){
		FillProximityValues(eqp->zoneid, eqp->exploreid);
	}
	else{
		mErrorLog->Log(eqEmuLogBoth, "ProximityListBoxDoubleClick failed, eqp is NULL");
	}
}

void MainFrame::PopulateProximity()
{
	mErrorLog->Log(eqEmuLogBoth, "Populating Proximity List");
	vector<eqtask_proximity>::iterator Iter;
	for(Iter = taskProximityList.begin(); Iter != taskProximityList.end(); Iter++)
	{
		eqtask_proximity eqt;
		eqt = *Iter;
		eqtask_prox * prox = new eqtask_prox;

		prox->exploreid = eqt.exploreid;
		prox->zoneid = eqt.zoneid;

		wxString zoneName;
		zoneName.Printf("Unknown Zone");
		vector<eqtask_zones>::iterator ZoneIter;
		for(ZoneIter = taskZoneList.begin(); ZoneIter != taskZoneList.end(); ZoneIter++)
		{
			eqtask_zones curZone = *ZoneIter;
			if(curZone.id == prox->zoneid)
			{
				zoneName.Clear();
				zoneName.Printf("%s", curZone.name);
			}
		}

		wxString newStr;
		newStr.Printf("%s - %u", zoneName.mb_str(), prox->exploreid);
		ProximitySelectionList->Append(newStr, (void*)prox);
	}
}

void MainFrame::FillProximityValues(unsigned int zone, unsigned int explore)
{
	mErrorLog->Log(eqEmuLogBoth,"zon %u, ex %u", zone, explore);
	eqtask_proximity ourProx;
	vector<eqtask_proximity>::iterator Iter;
	for(Iter = taskProximityList.begin(); Iter != taskProximityList.end(); Iter++)
	{
		eqtask_proximity curProx = *Iter;
		if(curProx.exploreid == explore && curProx.zoneid == zone){
			ourProx = curProx;
		}
	}

	wxString ourStr;

	ourStr.Clear();
	ourStr.Printf("%u", ourProx.exploreid);
	mProxId->Enable();
	mProxId->SetValue(ourStr);

	ourStr.Clear();
	ourStr.Printf("%.4f", ourProx.minx);
	mProxMinx->Enable();
	mProxMinx->SetValue(ourStr);

	ourStr.Clear();
	ourStr.Printf("%.4f", ourProx.maxx);
	mProxMaxx->Enable();
	mProxMaxx->SetValue(ourStr);

	ourStr.Clear();
	ourStr.Printf("%.4f", ourProx.miny);
	mProxMiny->Enable();
	mProxMiny->SetValue(ourStr);

	ourStr.Clear();
	ourStr.Printf("%.4f", ourProx.maxy);
	mProxMaxy->Enable();
	mProxMaxy->SetValue(ourStr);

	ourStr.Clear();
	ourStr.Printf("%.4f", ourProx.minz);
	mProxMinz->Enable();
	mProxMinz->SetValue(ourStr);

	ourStr.Clear();
	ourStr.Printf("%.4f", ourProx.maxz);
	mProxMaxz->Enable();
	mProxMaxz->SetValue(ourStr);

    openedProximity.exploreid = explore;
	openedProximity.zoneid = zone;

	SetZoneSelectionByIdProximity(zone);
	mProxZone->Enable();
}

void MainFrame::NewProximity(wxCommandEvent& event)
{
	if(!mMysql){
		mErrorLog->Log(eqEmuLogBoth, "New proximity failed, not connected to db");
		return;
	}

	int newID = wxGetNumberFromUser("", "Explore ID:", "Explore ID", 0, 0, 9999999);
	int newZoneID = wxGetNumberFromUser("", "Zone ID:", "Zone ID", 0, 0, 9999999);

	if(newID < 0 || newZoneID < 0)
	{
		mErrorLog->Log(eqEmuLogBoth, "New proximity failed, id or zone < 0");
		return;
	}

	if(newID == 0xFFFFFFFF || newZoneID == 0xFFFFFFF)
	{
		mErrorLog->Log(eqEmuLogBoth, "New proximity failed, 0xFFFFFFFF cannot be set with editor");
		return;
	}

	eqtask_proximity newProx;
	newProx.exploreid = newID;
	newProx.zoneid = newZoneID;
	newProx.maxx = 0.0;
	newProx.minx = 0.0;
	newProx.maxy = 0.0;
	newProx.miny = 0.0;
	newProx.maxz = 0.0;
	newProx.minz = 0.0;

	char * mQuery = 0;
	MakeAnyLenString(&mQuery, "INSERT INTO `proximities` (`zoneid`,`exploreid`) VALUES (%u,%u)", newZoneID, newID);
	mErrorLog->Log(eqEmuLogSQL, "%s", mQuery);
	if (mysql_query(mMysql, mQuery)) {
		mErrorLog->Log(eqEmuLogBoth, "MySQL Error: %s", mysql_error(mMysql));
		return;
	}

	eqtask_prox * pPtr = new eqtask_prox;
	pPtr->exploreid = newID;
	pPtr->zoneid = newZoneID;

	wxString zoneName;
	zoneName.Printf("Unknown Zone");
	vector<eqtask_zones>::iterator ZoneIter;
	for(ZoneIter = taskZoneList.begin(); ZoneIter != taskZoneList.end(); ZoneIter++)
	{
		eqtask_zones curZone = *ZoneIter;
		if(curZone.id == newZoneID)
		{
			zoneName.Clear();
			zoneName.Printf("%s", curZone.name);
		}
	}

	wxString newStr;
	newStr.Printf("%s - %u", zoneName.mb_str(), newID);
	ProximitySelectionList->Append(newStr, (void*)pPtr);

	taskProximityList.push_back(newProx);
}

void MainFrame::DeleteProximity(wxCommandEvent& event)
{
	if(!mMysql){
		mErrorLog->Log(eqEmuLogBoth, "Delete proximity failed, not connected to db");
		return;
	}

	eqtask_prox * pPtr = (eqtask_prox*)ProximitySelectionList->GetClientData(ProximitySelectionList->GetSelection());

	if(!pPtr){
		mErrorLog->Log(eqEmuLogBoth, "Delete proximity failed, pPtr is NULL");
		return;
	}

	int explore = pPtr->exploreid;
	int zone = pPtr->zoneid;

	int reply = wxMessageBox("Are you sure?", "Confirm Delete", wxYES_NO, this);
	if(reply != wxYES)
	{
		mErrorLog->Log(eqEmuLogBoth, "User aborted delete of proximity.");
		return;
	}

	char * mQuery = 0;
	MakeAnyLenString(&mQuery, "DELETE FROM proximities WHERE zoneid=%u AND exploreid=%u", zone, explore);
	mErrorLog->Log(eqEmuLogSQL, "%s", mQuery);
	if (mysql_query(mMysql, mQuery)) {
		mErrorLog->Log(eqEmuLogBoth, "MySQL Error: %s", mysql_error(mMysql));
		return;
	}

	for(unsigned int x = 0; x < ProximitySelectionList->GetCount(); x++)
	{
		eqtask_prox * p = (eqtask_prox*)ProximitySelectionList->GetClientData(x);
		if(p){
			if(p->exploreid == explore && p->zoneid == zone)
			{
				delete p;
				p = NULL;
				ProximitySelectionList->Delete(x);
				break;
			}
		}
	}

	vector<eqtask_proximity>::iterator Iter;
	for(Iter = taskProximityList.begin(); Iter != taskProximityList.end(); Iter++)
	{
		eqtask_proximity curProx = *Iter;
		if(curProx.exploreid == explore && curProx.zoneid == zone){
			taskProximityList.erase(Iter);
			break;
		}
	}

	if(openedProximity.exploreid == explore && openedProximity.zoneid == zone)
	{
		mProxId->Clear();
		mProxId->Disable();
		mProxMinx->Clear();
		mProxMinx->Disable();
		mProxMaxx->Clear();
		mProxMaxx->Disable();
		mProxMiny->Clear();
		mProxMiny->Disable();
		mProxMaxy->Clear();
		mProxMaxy->Disable();
		mProxMinz->Clear();
		mProxMinz->Disable();
		mProxMaxz->Clear();
		mProxMaxz->Disable();

		mProxZone->Select(0);
		mProxZone->Disable();
		openedProximity.exploreid = 0xFFFFFFFF;
		openedProximity.zoneid = 0xFFFFFFFF;
	}
}

void MainFrame::SaveProximity(wxCommandEvent& event)
{
	if(!mMysql){
		mErrorLog->Log(eqEmuLogBoth, "Save proximity failed, not connected to db");
		return;
	}

	if(openedProximity.exploreid == 0xFFFFFFFF && openedProximity.zoneid == 0xFFFFFFFF)
	{
		mErrorLog->Log(eqEmuLogBoth, "Save proximity failed, no proximity opened");
		return;
	}

	mErrorLog->Log(eqEmuLogBoth, "Saving proximity...");

	eqtask_proximity toSave;
	wxString inStr;
	int explore = openedProximity.exploreid;
	int zone = openedProximity.zoneid;
	bool canUpdate = false;

	inStr.Clear();
	inStr = mProxId->GetValue();
	toSave.exploreid = Strings::ToInt(inStr.mb_str());

	inStr.Clear();
	inStr = mProxMinx->GetValue();
	toSave.minx = atof(inStr.mb_str());

	inStr.Clear();
	inStr = mProxMaxx->GetValue();
	toSave.maxx = atof(inStr.mb_str());

	inStr.Clear();
	inStr = mProxMiny->GetValue();
	toSave.miny = atof(inStr.mb_str());

	inStr.Clear();
	inStr = mProxMaxy->GetValue();
	toSave.maxy = atof(inStr.mb_str());

	inStr.Clear();
	inStr = mProxMinz->GetValue();
	toSave.minz = atof(inStr.mb_str());

	inStr.Clear();
	inStr = mProxMaxz->GetValue();
	toSave.maxz = atof(inStr.mb_str());

	if(toSave.maxx < toSave.minx){
		mErrorLog->Log(eqEmuLogBoth, "min x and max x mismatched, correcting");
		int temp = toSave.minx;
		toSave.minx = toSave.maxx;
		toSave.maxx = temp;

		wxString tempStr;
		tempStr.Printf("%.4f", toSave.minx);
		mProxMinx->SetValue(tempStr);
		tempStr.Clear();

		tempStr.Printf("%.4f", toSave.maxx);
		mProxMaxx->SetValue(tempStr);
		tempStr.Clear();
	}

	if(toSave.maxy < toSave.miny){
		mErrorLog->Log(eqEmuLogBoth, "min y and max y mismatched, correcting");
		int temp = toSave.miny;
		toSave.miny = toSave.maxy;
		toSave.maxy = temp;

		wxString tempStr;
		tempStr.Printf("%.4f", toSave.miny);
		mProxMiny->SetValue(tempStr);
		tempStr.Clear();

		tempStr.Printf("%.4f", toSave.maxy);
		mProxMaxy->SetValue(tempStr);
		tempStr.Clear();
	}

	if(toSave.maxz < toSave.minz){
		mErrorLog->Log(eqEmuLogBoth, "min z and max z mismatched, correcting");
		int temp = toSave.minz;
		toSave.minz = toSave.maxz;
		toSave.maxz = temp;

		wxString tempStr;
		tempStr.Printf("%.4f", toSave.minz);
		mProxMinz->SetValue(tempStr);
		tempStr.Clear();

		tempStr.Printf("%.4f", toSave.maxz);
		mProxMaxz->SetValue(tempStr);
		tempStr.Clear();
	}

	int * iPtr = (int*)mProxZone->GetClientData(mProxZone->GetCurrentSelection());
	toSave.zoneid = *iPtr;

	if(!iPtr){
		mErrorLog->Log(eqEmuLogBoth, "Save proximity failed, iPtr is NULL");
		return;
	}

	if(openedProximity.exploreid == toSave.exploreid && openedProximity.zoneid == toSave.zoneid)
	{
		mErrorLog->Log(eqEmuLogBoth, "Can use UPDATE");
	}
	else{
		mErrorLog->Log(eqEmuLogBoth, "Cannot use UPDATE must replace instead");
	}

	if(canUpdate){
		char * mQuery = 0;
		MakeAnyLenString(&mQuery, "UPDATE proximities SET minx=%.4f, maxx=%.4f, miny=%.4f, maxy=%.4f, minz=%.4f, maxz=%.4f) WHERE zoneid=%u AND exploreid=%u",
			toSave.minx, toSave.maxx, toSave.miny, toSave.maxy, toSave.minz, toSave.maxz, toSave.zoneid, toSave.exploreid);

		mErrorLog->Log(eqEmuLogSQL, "%s", mQuery);
		if (mysql_query(mMysql, mQuery)) {
			mErrorLog->Log(eqEmuLogBoth, "MySQL Error: %s", mysql_error(mMysql));
			return;
		}
	}
	else
	{
		char * mQuery = 0;
		MakeAnyLenString(&mQuery, "DELETE FROM proximities WHERE zoneid=%u AND exploreid=%u", zone, explore);
		mErrorLog->Log(eqEmuLogSQL, "%s", mQuery);
		if (mysql_query(mMysql, mQuery)) {
			mErrorLog->Log(eqEmuLogBoth, "MySQL Error: %s", mysql_error(mMysql));
			return;
		}

		MakeAnyLenString(&mQuery, "INSERT INTO `proximities` (`zoneid`,`exploreid`,`minx`,`maxx`,`miny`,`maxy`,`minz`,`maxz`) VALUES (%u,%u,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f)"
			, toSave.zoneid, toSave.exploreid, toSave.minx, toSave.maxx, toSave.miny, toSave.maxy, toSave.minz, toSave.maxz);

		mErrorLog->Log(eqEmuLogSQL, "%s", mQuery);
		if (mysql_query(mMysql, mQuery)) {
			mErrorLog->Log(eqEmuLogBoth, "MySQL Error: %s", mysql_error(mMysql));
			return;
		}
	}

	for(unsigned int x = 0; x < ProximitySelectionList->GetCount(); x++)
	{
		eqtask_prox * p = (eqtask_prox*)ProximitySelectionList->GetClientData(x);
		if(p){
			if(p->exploreid == openedProximity.exploreid && p->zoneid == openedProximity.zoneid)
			{
				p->exploreid = toSave.exploreid;
				p->zoneid = toSave.zoneid;
				wxString newStr;
				wxString zoneName;
				zoneName.Printf("Unknown Zone");
				vector<eqtask_zones>::iterator ZoneIter;
				for(ZoneIter = taskZoneList.begin(); ZoneIter != taskZoneList.end(); ZoneIter++)
				{
					eqtask_zones curZone = *ZoneIter;
					if(curZone.id == p->zoneid)
					{
						zoneName.Clear();
						zoneName.Printf("%s", curZone.name);
					}
				}
				newStr.Printf("%s - %u", zoneName.mb_str(), p->exploreid );
				ProximitySelectionList->SetString(x, newStr);
				break;
			}
		}
	}


	vector<eqtask_proximity>::iterator Iter;
	for(Iter = taskProximityList.begin(); Iter != taskProximityList.end(); Iter++)
	{
		eqtask_proximity cur = *Iter;
		if(cur.exploreid == openedProximity.exploreid && cur.zoneid == openedProximity.zoneid)
		{
			(*Iter).exploreid = toSave.exploreid;
			(*Iter).zoneid = toSave.zoneid;
			(*Iter).minx = toSave.minx;
			(*Iter).maxx = toSave.maxx;
			(*Iter).miny = toSave.miny;
			(*Iter).maxy = toSave.maxy;
			(*Iter).minz = toSave.minz;
			(*Iter).maxz = toSave.maxz;
		}
	}

	openedProximity.exploreid = toSave.exploreid;
	openedProximity.zoneid = toSave.zoneid;
	mErrorLog->Log(eqEmuLogBoth, "Save finished.");
}

void MainFrame::ContextMenuProximity()
{
	if(!mMysql){
		mErrorLog->Log(eqEmuLogBoth, "Context menu cannot open, not connected to db");
		return;
	}

	wxMenu *mMenu;
	mMenu = new wxMenu();

	mMenu->Append(MENU_NewProximity, wxT("New Proximity"), wxT("Creates a new goal proximity entry"));
	mMenu->Append(MENU_DeleteProximity, wxT("Delete Proximity"), wxT("Deletes the selected proximity entry"));
	mMenu->AppendSeparator();
	mMenu->Append(MENU_SaveProximity, wxT("Save Proximity"), wxT("Saves the opened proximity entry"));

	PopupMenu(mMenu);
	delete mMenu;
}
