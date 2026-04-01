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
#include "base.h"
#include "ErrorLog.h"

using namespace std;

void MainFrame::ActivitiesListBoxSimpleSelect(wxCommandEvent& event)
{
	eqtask_activity_id *eqaid;
	eqaid = (eqtask_activity_id*)ActivitiesSelectionList->GetClientData(event.GetInt());
	if(eqaid){
	}
	else{
		mErrorLog->Log(eqEmuLogBoth, "Error selected a NULL activity object!");
	}
}

void MainFrame::ActivitiesListBoxDoubleClick(wxCommandEvent& event)
{
	eqtask_activity_id *eqaid;
	eqaid = (eqtask_activity_id*)ActivitiesSelectionList->GetClientData(event.GetInt());
	if(eqaid){
		openedActivity.activityid = eqaid->activityid;
		openedActivity.id = eqaid->id;
		openedActivity.step = eqaid->step;

		FillActivity(eqaid->id, eqaid->activityid, eqaid->step);
		mActText1->Enable();
		mActText2->Enable();
		mActText3->Enable();
		mActivityZone->Enable();
		mActivityOptional->Enable();
		mActID->Enable();
		mActStep->Enable();
		mActType->Enable();
		mActDeliver->Enable();
		mActMethod->Enable();
		mActGoalID->Enable();
		mActGoalCount->Enable();
	}
	else{
		mErrorLog->Log(eqEmuLogBoth, "Error double clicked a NULL activity object!");
	}
}

void MainFrame::NewActivity(wxCommandEvent& event)
{
	if(!mMysql)
		return;

	if(openedIndex == -1)
		return;

	mErrorLog->Log(eqEmuLogBoth, "Creating new activity...");
	eqtask_activities newAct;
	eqtask curTask;

	mErrorLog->Log(eqEmuLogBoth, "Opened Index %u", openedIndex);
	vector<eqtask>::iterator tIter = taskList.begin();
	tIter += openedIndex;
	curTask = *tIter;

	vector<eqtask_activities>::iterator Iter;
	unsigned int highestId = 0;
	bool zeroExists;
	if(!taskActivitiesList.empty()){
		for(Iter = taskActivitiesList.begin(); Iter != taskActivitiesList.end(); Iter++)
		{
			eqtask_activities curAct = *Iter;
			if(curAct.id == curTask.id){
				if(curAct.activityId == 0)
					zeroExists = true;

				if(curAct.activityId > highestId){
					highestId = curAct.activityId;
				}
			}
		}
	}

	mErrorLog->Log(eqEmuLogBoth, "Highest id: %u", highestId);

	if(highestId == 0)
	{
		if(zeroExists)
		{
			newAct.activityId = highestId+1;
		}
		else
		{
			newAct.activityId = 0;
		}
	}
	else
	{
		newAct.activityId = highestId+1;
	}

	newAct.id = curTask.id;
	newAct.zoneid = curTask.startzone;
	newAct.activityType = 0;
	newAct.deliverToNpc = 0;
	newAct.goalcount = 0;
	newAct.goalid = 0;
	newAct.goalmethod = 2;
	newAct.optional = false;
	newAct.step = 0;
	strcpy(newAct.text1, "");
	strcpy(newAct.text2, "");
	strcpy(newAct.text3, "");

	taskActivitiesList.push_back(newAct);

	eqtask_activity_id * eqaid = new eqtask_activity_id;
	eqaid->id = newAct.id;
	eqaid->step = 0;
	eqaid->activityid = newAct.activityId;

	wxString taskStr;
	taskStr.Printf("Step: %u", 0);
	ActivitiesSelectionList->Append(taskStr, (void*)eqaid);
	taskStr.clear();

	if(mMysql){
		char * mActQuery = 0;
		MakeAnyLenString(&mActQuery, "INSERT INTO `task_activities` (`taskid`,`activityid`,`step`,`activitytype`,`text1`,`text2`,`text3`,`goalid`,`goalmethod`,`goalcount`,`delivertonpc`,`zoneid`,`optional`) VALUES (%u,%u,%u,%u,'%s','%s','%s',%u,%u,%u,%u,%u,%u)",
				newAct.id, newAct.activityId, newAct.step, newAct.activityType, newAct.text1, newAct.text2, newAct.text3, newAct.goalid, newAct.goalmethod, newAct.goalcount, newAct.deliverToNpc, newAct.zoneid, newAct.optional	);

		mErrorLog->Log(eqEmuLogSQL, "%s", mActQuery);
		if (mysql_query(mMysql, mActQuery)) {
			mErrorLog->Log(eqEmuLogBoth, "MySQL Error: %s", mysql_error(mMysql));
		}
	}
	else{
		mErrorLog->Log(eqEmuLogBoth, "Mysql not connected for activity creation.");
	}
}

void MainFrame::DeleteActivity(wxCommandEvent& event)
{
	if(!mMysql){
		mErrorLog->Log(eqEmuLogBoth, "Mysql not connected for delete activity");
		return;
	}

	if(ActivitiesSelectionList->GetCount() == 0){
		mErrorLog->Log(eqEmuLogBoth, "No activities in list to select for delete");
		return;
	}

	int reply = wxMessageBox("Are you sure?", "Confirm Delete", wxYES_NO, this);
	if(reply != wxYES)
	{
		mErrorLog->Log(eqEmuLogBoth, "User aborted delete of activity.");
		return;
	}

	eqtask_activity_id *selAct = (eqtask_activity_id*)ActivitiesSelectionList->GetClientData(ActivitiesSelectionList->GetSelection());
	unsigned int aid = selAct->activityid;
	unsigned int tid = selAct->id;

	if(selAct)
	{
		if(mMysql){
			char * mActQuery = 0;
			MakeAnyLenString(&mActQuery, "DELETE FROM `task_activities` WHERE taskid=%u AND activityid=%u",selAct->id, selAct->activityid);
			mErrorLog->Log(eqEmuLogSQL, "%s", mActQuery);
			if (mysql_query(mMysql, mActQuery)) {
				mErrorLog->Log(eqEmuLogBoth, "MySQL Error: %s", mysql_error(mMysql));
				return;
			}
		}
		else{
			mErrorLog->Log(eqEmuLogBoth, "No mysql connection for delete");
		}

		for(unsigned int i = 0; i < ActivitiesSelectionList->GetCount(); i++)
		{
			eqtask_activity_id * id = (eqtask_activity_id*)ActivitiesSelectionList->GetClientData(i);
			if(id){
				if(id->activityid == selAct->activityid && id->id == selAct->id)
				{
					if(openedActivity.activityid == selAct->activityid && openedActivity.id == selAct->id)
					{
						mActText1->Clear();
						mActText1->Disable();

						mActText2->Clear();
						mActText2->Disable();

						mActText3->Clear();
						mActText3->Disable();

						mActivityZone->Select(0);
						mActivityZone->Disable();

						mActivityOptional->SetValue(false);
						mActivityOptional->Disable();

						mActID->Clear();
						mActID->Disable();

						mActStep->Clear();
						mActStep->Disable();

						mActType->Select(0);
						mActType->Disable();

						mActDeliver->Clear();
						mActDeliver->Disable();

						mActMethod->Select(2);
						mActMethod->Disable();

						mActGoalID->Clear();
						mActGoalID->Disable();

						mActGoalCount->Clear();
						mActGoalCount->Disable();
					}

					mErrorLog->Log(eqEmuLogBoth, "Erasing Activity Selection");
					ActivitiesSelectionList->Delete(i);
					delete id;
					mErrorLog->Log(eqEmuLogBoth, "Erased..");
					break;
				}
			}
		}

		vector<eqtask_activities>::iterator Iter;
		for(Iter = taskActivitiesList.begin(); Iter!=taskActivitiesList.end(); Iter++)
		{
			eqtask_activities curAct = *Iter;
			if(curAct.activityId == aid && curAct.id == tid){
				mErrorLog->Log(eqEmuLogBoth, "Erasing Activity Iterator");
				taskActivitiesList.erase(Iter);
				mErrorLog->Log(eqEmuLogBoth, "Erased..");
				break;
			}
		}
	}
	else
	{
		mErrorLog->Log(eqEmuLogBoth, "No activity selected for delete.");
	}
}

void MainFrame::PopulateActivities()
{
	int taskId = -1;
	mErrorLog->Log(eqEmuLogBoth, "Populating activities...");
	vector<eqtask>::iterator Iter;
	int index = 0;
	for(Iter = taskList.begin(); Iter != taskList.end(); Iter++)
	{
		if(index == openedIndex){
			eqtask eqt = *Iter;
			taskId = eqt.id;
			break;
		}
		index++;
	}

	if(taskId > -1){
		vector<eqtask_activities>::iterator aIter;
		for(aIter = taskActivitiesList.begin(); aIter != taskActivitiesList.end(); aIter++)
		{
			eqtask_activities eqta = *aIter;
			if(eqta.id == taskId){
				eqtask_activity_id * eqaid = new eqtask_activity_id;

				eqaid->id = eqta.id;
				eqaid->step = eqta.step;
				eqaid->activityid = eqta.activityId;

				wxString taskStr;
				taskStr.Printf("Step: %u", eqta.step);
				ActivitiesSelectionList->Append(taskStr, (void*)eqaid);
				taskStr.clear();
			}
		}
	}
}

void MainFrame::ClearActivities()
{
	mErrorLog->Log(eqEmuLogBoth, "Clearing activities...");
	for(unsigned int x = 0; x < ActivitiesSelectionList->GetCount(); x++)
	{
		eqtask_activity_id *eqaid = (eqtask_activity_id*)ActivitiesSelectionList->GetClientData(x);
		if(eqaid)
		{
			mErrorLog->Log(eqEmuLogBoth, "Delete of activity index: %u (%u:%u)", x, eqaid->id, eqaid->step);
			delete eqaid;
			eqaid = NULL;
		}
	}

	ActivitiesSelectionList->Clear();
}

void MainFrame::FillActivity(int id, int activityid, int step)
{
	if(id > -1){
		vector<eqtask_activities>::iterator Iter;
		for(Iter = taskActivitiesList.begin(); Iter != taskActivitiesList.end();Iter++)
		{
			eqtask_activities eqta = *Iter;
			if(eqta.id == id && eqta.activityId == activityid && eqta.step == step)
			{
				wxString actStr;
				actStr.Clear();
				actStr.Printf("%s", eqta.text1);
				mActText1->Clear();
				mActText1->AppendText(actStr);
				actStr.Clear();

				actStr.Printf("%s", eqta.text2);
				mActText2->Clear();
				mActText2->AppendText(actStr);
				actStr.Clear();

				actStr.Printf("%s", eqta.text3);
				mActText3->Clear();
				mActText3->AppendText(actStr);
				actStr.Clear();

				SetZoneSelectionByIdActivity(eqta.zoneid);
				mActivityOptional->SetValue(eqta.optional);

				actStr.Printf("%u", eqta.activityId);
				mActID->Clear();
				mActID->AppendText(actStr);
				actStr.Clear();

				actStr.Printf("%u", eqta.step);
				mActStep->Clear();
				mActStep->AppendText(actStr);
				actStr.Clear();

				actStr.Printf("%u", eqta.deliverToNpc);
				mActDeliver->Clear();
				mActDeliver->AppendText(actStr);
				actStr.Clear();

				actStr.Printf("%u", eqta.goalid);
				mActGoalID->Clear();
				mActGoalID->AppendText(actStr);
				actStr.Clear();

				actStr.Printf("%u", eqta.goalcount);
				mActGoalCount->Clear();
				mActGoalCount->AppendText(actStr);
				actStr.Clear();

				if(eqta.goalmethod >= 0 && eqta.goalmethod <= 2)
					mActMethod->Select(eqta.goalmethod);
				else
					mActMethod->Select(2);

				if(eqta.activityType > 0 && eqta.activityType < 12)
				{
					mActType->Select(eqta.activityType);
				}
				else
				{
					if(eqta.activityType == 100){
						mActType->Select(12);
					}
					else if(eqta.activityType == 999)
					{
						mActType->Select(13);
					}
					else{
						mActType->Select(0);
					}
				}

				wxCommandEvent evnt;
				evnt.SetInt(eqta.activityType);
				ActivityChoiceChange(evnt);
				return;
			}
		}
	}
}

void MainFrame::ActivityChoiceChange(wxCommandEvent& event)
{
	mErrorLog->Log(eqEmuLogBoth, "Activity Choice Change: %u", event.GetInt());
	switch(event.GetInt()){
	case 1:
		mActInfoText->SetLabel("Deliver:\nDeliver a number of items to a NPC\n"
			"text1: NPC Name\n"
			"text2: Item Name"
			"\n\nRemember: text3 can be used to \noverwrite text1 or text2");
		break;
	case 2:
		mActInfoText->SetLabel("Kill:\nKill a number of NPCs\n"
			"text1: NPC Name\n"
			"\n\nRemember: text3 can be used to \noverwrite text1 or text2");
		break;
	case 3:
		mActInfoText->SetLabel("Loot:\nLoot a number of items\n"
			"text1: Item Name\n"
			"\n\nRemember: text3 can be used to \noverwrite text1 or text2");
		break;
	case 4:
		mActInfoText->SetLabel("Speak To:\nSpeak with a specific NPC\n"
			"text1: NPC Name\n"
			"\n\nRemember: text3 can be used to \noverwrite text1 or text2");
		break;
	case 5:
		mActInfoText->SetLabel("Explore:\nExplore a certain area of the game\n"
			"text1: Area Description\n"
			"\n\nRemember: text3 can be used to \noverwrite text1 or text2");
		break;
	case 6:
		mActInfoText->SetLabel("Tradeskill:\nCreate a number of items\n"
			"text1: Item Name\n"
			"\n\nRemember: text3 can be used to \noverwrite text1 or text2");
		break;
	case 7:
		mActInfoText->SetLabel("Fish:\nObtain a number of items with fishing\n"
			"text1: Item Name\n"
			"\n\nRemember: text3 can be used to \noverwrite text1 or text2");
		break;
	case 8:
		mActInfoText->SetLabel("Forage:\nObtain a number of items via foraging\n"
			"text1: Item Name\n"
			"\n\nRemember: text3 can be used to \noverwrite text1 or text2");
		break;
	case 11:
		mActInfoText->SetLabel("Touch:\nEnter a certain zone\n"
			"text1: Zone Name\n"
			"\n\nRemember: text3 can be used to \noverwrite text1 or text2");
		break;
	case 10:
	case 9:
		mActInfoText->SetLabel("Use:\nNot currently in use\n");
		break;
	case 12:
		mActInfoText->SetLabel("Give Cash:\nGive money to a certain npc\n"
			"text3: Custom Description");
		break;
	case 13:
		mActInfoText->SetLabel("Custom:\nCustom task handled by the quest\nsystem\n"
			"text3: Custom Description");
		break;
	default:
		mActInfoText->SetLabel("Unknown Activity Type:\nThis is not a valid activity type");
		break;

	}
}

void MainFrame::SaveActivity(wxCommandEvent& event)
{
	mErrorLog->Log(eqEmuLogBoth, "Save activity requested...");

	if(!mMysql){
		mErrorLog->Log(eqEmuLogBoth, "Mysql not connected for save activity");
		return;
	}

	if(openedActivity.activityid < 0 || openedActivity.id < 0){
		mErrorLog->Log(eqEmuLogBoth, "Activity not valid for save.");
		return;
	}

	eqtask_activities ourAct;

	vector<eqtask_activities>::iterator Iter;
	for(Iter = taskActivitiesList.begin(); Iter != taskActivitiesList.end(); Iter++)
	{
		if((*Iter).activityId == openedActivity.activityid && (*Iter).id == openedActivity.id)
		{
			ourAct = *Iter;
			break;
		}
	}

	int delid = openedActivity.activityid;
	bool canUpdate = false;
	wxString getStr;

	getStr = mActText1->GetValue();
	strcpy(ourAct.text1, getStr.mb_str());
	getStr.Clear();

	getStr = mActText2->GetValue();
	strcpy(ourAct.text2, getStr.mb_str());
	getStr.Clear();

	getStr = mActText3->GetValue();
	strcpy(ourAct.text3, getStr.mb_str());
	getStr.Clear();

	int * i = (int*)mActivityZone->GetClientData(mActivityZone->GetSelection());
	ourAct.zoneid = *i;

	ourAct.optional = mActivityOptional->GetValue();

	getStr = mActID->GetValue();
	ourAct.activityId = Strings::ToInt(getStr.mb_str());
	getStr.Clear();

	getStr = mActStep->GetValue();
	ourAct.step = Strings::ToInt(getStr.mb_str());
	getStr.Clear();

	int type = mActType->GetSelection();
	if(type > 0 && type < 12){
		ourAct.activityType = type;
	}
	else{
		if(type == 12){
			ourAct.activityType = 100;
		}
		else if(type == 13){
			ourAct.activityType = 999;
		}
		else
			ourAct.activityType = 0;
	}

	getStr = mActDeliver->GetValue();
	ourAct.deliverToNpc = Strings::ToInt(getStr.mb_str());
	getStr.Clear();

	ourAct.goalmethod = mActMethod->GetSelection();

	getStr = mActGoalID->GetValue();
	ourAct.goalid = Strings::ToInt(getStr.mb_str());
	getStr.Clear();

	getStr = mActGoalCount->GetValue();
	ourAct.goalcount = Strings::ToInt(getStr.mb_str());
	getStr.Clear();

	if(ourAct.activityId == openedActivity.activityid && ourAct.id == openedActivity.id){
		canUpdate = true;
		mErrorLog->Log(eqEmuLogBoth, "Can use UPDATE.");
	}
	else{
		mErrorLog->Log(eqEmuLogBoth, "Cannot use UPDATE must replace instead");
	}

	if(canUpdate)
	{
		char * mQuery = 0;
		MakeAnyLenString(&mQuery, "UPDATE `task_activities` SET step=%u, activitytype=%u, text1='%s', text2='%s', text3='%s', goalid=%u, goalmethod=%u,goalcount=%u, delivertonpc=%u, zoneid=%u, optional=%u WHERE taskid=%u AND activityid =%u",
			ourAct.step, ourAct.activityType, MakeStringSQLSafe(ourAct.text1).mb_str(), MakeStringSQLSafe(ourAct.text2).mb_str(), MakeStringSQLSafe(ourAct.text3).mb_str(), ourAct.goalid, ourAct.goalmethod, ourAct.goalcount, ourAct.deliverToNpc, ourAct.zoneid, ourAct.optional, ourAct.id, ourAct.activityId );
		mErrorLog->Log(eqEmuLogSQL, "%s", mQuery);
		if (mysql_query(mMysql, mQuery)) {
			mErrorLog->Log(eqEmuLogBoth, "MySQL Error: %s", mysql_error(mMysql));
			return;
		}
	}
	else
	{
		char * mQuery = 0;
		MakeAnyLenString(&mQuery, "DELETE FROM `task_activities` WHERE taskid=%u AND activityid=%u", ourAct.id, delid);
		mErrorLog->Log(eqEmuLogSQL, "%s", mQuery);
		if (mysql_query(mMysql, mQuery)) {
			mErrorLog->Log(eqEmuLogBoth, "MySQL Error: %s", mysql_error(mMysql));
			return;
		}

		MakeAnyLenString(&mQuery, "INSERT INTO `task_activities` (`taskid`,`activityid`,`step`,`activitytype`,`text1`,`text2`,`text3`,`goalid`,`goalmethod`,`goalcount`,`delivertonpc`,`zoneid`,`optional`) VALUES (%u,%u,%u,%u,'%s','%s','%s',%u,%u,%u,%u,%u,%u)",
			ourAct.id, ourAct.activityId, ourAct.step, ourAct.activityType, MakeStringSQLSafe(ourAct.text1).mb_str(), MakeStringSQLSafe(ourAct.text2).mb_str(), MakeStringSQLSafe(ourAct.text3).mb_str(), ourAct.goalid, ourAct.goalmethod, ourAct.goalcount, ourAct.deliverToNpc, ourAct.zoneid, ourAct.optional);
		mErrorLog->Log(eqEmuLogSQL, "%s", mQuery);
		if (mysql_query(mMysql, mQuery)) {
			mErrorLog->Log(eqEmuLogBoth, "MySQL Error: %s", mysql_error(mMysql));
			return;
		}
	}

	(*Iter).activityId = ourAct.activityId;
	(*Iter).activityType = ourAct.activityType;
	(*Iter).deliverToNpc = ourAct.deliverToNpc;
	(*Iter).goalcount = ourAct.goalcount;
	(*Iter).goalid = ourAct.goalid;
	(*Iter).goalmethod = ourAct.goalmethod;
	(*Iter).id = ourAct.id;
	(*Iter).optional = ourAct.optional;
	(*Iter).step = ourAct.step;
	strcpy((*Iter).text1, ourAct.text1);
	strcpy((*Iter).text2, ourAct.text2);
	strcpy((*Iter).text3, ourAct.text3);
	(*Iter).zoneid = ourAct.zoneid;

	int ourIndex = 0;
	for(unsigned int Index = 0 ;Index < ActivitiesSelectionList->GetCount(); Index++)
	{
		eqtask_activity_id *eqta;
		eqta = (eqtask_activity_id*)ActivitiesSelectionList->GetClientData(Index);
		if(eqta){
			if(eqta->activityid == openedActivity.activityid && eqta->id == openedActivity.id){
				ourIndex = Index;
				break;
			}
		}
		else{
			mErrorLog->Log(eqEmuLogBoth, "NULL eqta, index %u", Index);
		}
	}

	getStr.Clear();
	getStr.Printf("Step: %u", (*Iter).step);
	eqtask_activity_id * eqt = (eqtask_activity_id*)ActivitiesSelectionList->GetClientData(ourIndex);
	eqt->activityid = ourAct.activityId;
	eqt->id = ourAct.id;
	eqt->step = ourAct.step;
    ActivitiesSelectionList->SetString(ourIndex, getStr);
	ActivitiesSelectionList->SetClientData(ourIndex, (void*)eqt);


	openedActivity.activityid = ourAct.activityId;
	openedActivity.id = ourAct.id;
	openedActivity.step = ourAct.step;

	mErrorLog->Log(eqEmuLogBoth, "Save finished.");
}

void MainFrame::ContextMenuActivityList()
{
	if(!mMysql){
		mErrorLog->Log(eqEmuLogBoth, "Context menu cannot open, not connected to db");
		return;
	}

	wxMenu *mMenu;
	mMenu = new wxMenu();

	mMenu->Append(MENU_NewActivity, wxT("New Activity"), wxT("Creates a new activity"));
	mMenu->Append(MENU_DeleteActivity, wxT("Delete Activity"), wxT("Deletes the selected activity"));
	mMenu->AppendSeparator();
	mMenu->Append(MENU_SaveActivity, wxT("Save Activity"), wxT("Saves the opened activity"));

	PopupMenu(mMenu);
	delete mMenu;
}
