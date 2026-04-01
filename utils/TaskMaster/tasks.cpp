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

void MainFrame::NewTask(wxCommandEvent& event)
{
	if(mMysql){
		eqtask newT;
		newT.id = (highestIndex+1);
		newT.duration = 0;
		strcpy(newT.title, "Default Task Name");
		strcpy(newT.desc, "Default Task Description");
		strcpy(newT.reward, "");
		newT.cashreward = 0;
		newT.xpreward = 0;
		newT.rewardmethod = 2;
		newT.rewardid = 0;
		newT.startzone = -1;
		newT.level_min = 0;
		newT.level_max = 0;
		newT.repeatable = true;

		unsigned int newID = wxGetNumberFromUser("", "ID:", "Input ID", (highestIndex+1), 0, 2147483600);

		newT.id = newID;

		char * mQuery = 0;
		MakeAnyLenString(&mQuery, "INSERT INTO `tasks` (`id`,`duration`,`title`,`description`,`reward`,`rewardid`,`cashreward`,`xpreward`,`rewardmethod`,`startzone`, `minlevel`, `maxlevel`, `repeatable`) VALUES (%u,%u,'%s','%s','%s',%u,%u,%u,%u,%u,%u,%u,%u)",
			newT.id, newT.duration, newT.title, newT.desc, newT.reward, newT.rewardid, newT.cashreward, newT.xpreward, newT.rewardmethod, newT.startzone, newT.level_min, newT.level_max, newT.repeatable);

		mErrorLog->Log(eqEmuLogSQL, "%s", mQuery);
		if (mysql_query(mMysql, mQuery)) {
			mErrorLog->Log(eqEmuLogBoth, "MySQL Error: %s", mysql_error(mMysql));
			return;
		}
		taskList.push_back(newT);

		wxString str;
		str.Printf("%d:%s", newT.id, "Default Task Name");
		ItemSelectionList->Append(str);
		if(highestIndex < (newID + 1))
			highestIndex = newID + 1;
	}
	else{
		mErrorLog->Log(eqEmuLogBoth, "Error with new task create, mysql not initialized");
	}
}

void MainFrame::DeleteTask(wxCommandEvent& event)
{
	if(selectedIndex < 0)
	{
		mErrorLog->Log(eqEmuLogBoth, "No item selected for delete, delete failing.");
		return;
	}

	if(mMysql){
		int reply = wxMessageBox("Are you sure?", "Confirm Delete", wxYES_NO, this);
		if(reply != wxYES)
		{
			mErrorLog->Log(eqEmuLogBoth, "User aborted delete of task.");
			return;
		}

		vector<eqtask>::iterator Iter;
		Iter = taskList.begin();
		Iter += selectedIndex;

		eqtask mTask = *Iter;

		char * mQuery = 0;
		MakeAnyLenString(&mQuery, "DELETE FROM `tasks` WHERE id=%u", mTask.id);
		mErrorLog->Log(eqEmuLogSQL, "%s", mQuery);

		if (mysql_query(mMysql, mQuery)) {
			mErrorLog->Log(eqEmuLogBoth, "MySQL Error: %s", mysql_error(mMysql));
			return;
		}

		if(openedIndex == selectedIndex){
			wxString mStr;
			mStr.Printf("%s", "Task Name");
			mTaskName->Clear();
			mTaskName->AppendText(mStr);
			mTaskName->Disable();
			mStr.clear();

			mStr.Printf("%s", "Task Description");
			mTaskDesc->Clear();
			mTaskDesc->AppendText(mStr);
			mTaskDesc->Disable();
			mStr.clear();

			mStr.Printf("%u", 0);
			mTaskMinLvl->Clear();
			mTaskMinLvl->AppendText(mStr);
			mTaskMinLvl->Disable();
			mStr.clear();

			mStr.Printf("%u", 0);
			mTaskMaxLvl->Clear();
			mTaskMaxLvl->AppendText(mStr);
			mTaskMaxLvl->Disable();
			mStr.clear();

			mStr.Printf("%u", 0);
			mTaskDuration->Clear();
			mTaskDuration->AppendText(mStr);
			mTaskDuration->Disable();
			mStr.clear();

			mRewardName->Clear();
			mRewardName->Disable();

			mStr.Printf("%u", 0);
			mRewardID->Clear();
			mRewardID->AppendText(mStr);
			mRewardID->Disable();
			mStr.clear();

			mStr.Printf("%u", 0);
			mRewardCash->Clear();
			mRewardCash->AppendText(mStr);
			mRewardCash->Disable();
			mStr.clear();

			mStr.Printf("%u", 0);
			mRewardXP->Clear();
			mRewardXP->AppendText(mStr);
			mRewardXP->Disable();
			mStr.clear();
			openedIndex = -1;

			mRewardMethod->Select(0);
			mRewardMethod->Disable();

			mStartZone->Select(0);
			mStartZone->Disable();

			mTaskRepeatable->SetValue(false);
			mTaskRepeatable->Disable();

			ShowRewardItems->Disable();
			ShowRewardItems->Clear();
			RefreshItems->Disable();

			ClearActivities();
			ActivitiesSelectionList->Disable();
			openedActivity.activityid = -1;
			openedActivity.id = -1;
			openedActivity.step = -1;
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

		taskList.erase(Iter);
		ItemSelectionList->Delete(selectedIndex);
	}
	else{
		mErrorLog->Log(eqEmuLogBoth, "Error with task delete, mysql not initialized");
	}
	selectedIndex = -1;
}

void MainFrame::OnRewardButton(wxCommandEvent& event)
{
	wxString ridStr = mRewardID->GetValue();
	int rtype = mRewardMethod->GetCurrentSelection();
	int rid = Strings::ToInt(ridStr.mb_str());

	ShowRewardChange(rtype,rid);
}

void MainFrame::SaveTask(wxCommandEvent& event)
{
	if(!mMysql){
		mErrorLog->Log(eqEmuLogBoth, "Mysql not connected for save of task");
		return;
	}

    if(openedIndex>=0){
		mErrorLog->Log(eqEmuLogBoth, "Saving task...");
		vector<eqtask>::iterator Iter;
		Iter = taskList.begin();
		Iter += openedIndex;
		eqtask ourTask = *Iter;
		wxString getStr;

		getStr = mTaskName->GetValue();
		strcpy(ourTask.title, getStr.mb_str());
		getStr.Clear();

		getStr = mTaskDesc->GetValue();
		strcpy(ourTask.desc, getStr.mb_str());
		getStr.Clear();

		getStr = mTaskMinLvl->GetValue();
		ourTask.level_min = Strings::ToInt(getStr.mb_str());
		getStr.Clear();

		getStr = mTaskMaxLvl->GetValue();
		ourTask.level_max = Strings::ToInt(getStr.mb_str());
		getStr.Clear();

		getStr = mTaskDuration->GetValue();
		ourTask.duration = Strings::ToInt(getStr.mb_str());
		getStr.Clear();

		getStr = mRewardName->GetValue();
		strcpy(ourTask.reward, getStr.mb_str());
		getStr.Clear();

		getStr = mRewardID->GetValue();
		ourTask.rewardid = Strings::ToInt(getStr.mb_str());
		getStr.Clear();

		getStr = mRewardCash->GetValue();
		ourTask.cashreward = Strings::ToInt(getStr.mb_str());
		getStr.Clear();

		getStr = mRewardXP->GetValue();
		ourTask.xpreward = Strings::ToInt(getStr.mb_str());
		getStr.Clear();

		int * i = (int*)mStartZone->GetClientData(mStartZone->GetSelection());
		ourTask.startzone = *i;

		ourTask.rewardmethod = mRewardMethod->GetSelection();
		ourTask.repeatable = mTaskRepeatable->GetValue();


		char * mQuery = 0;
		MakeAnyLenString(&mQuery, "UPDATE tasks SET duration=%u, title='%s', description='%s', reward='%s', rewardid=%u, cashreward=%u, xpreward=%i, rewardmethod=%u, startzone=%u, minlevel=%u, maxlevel=%u, repeatable=%u WHERE id=%u",
			ourTask.duration, MakeStringSQLSafe(ourTask.title).mb_str(), MakeStringSQLSafe(ourTask.desc).mb_str(), MakeStringSQLSafe(ourTask.reward).mb_str(), ourTask.rewardid, ourTask.cashreward, ourTask.xpreward, ourTask.rewardmethod, ourTask.startzone, ourTask.level_min, ourTask.level_max, ourTask.repeatable, ourTask.id);

		mErrorLog->Log(eqEmuLogSQL, "%s", mQuery);
		if (mysql_query(mMysql, mQuery)) {
			mErrorLog->Log(eqEmuLogBoth, "MySQL Error: %s", mysql_error(mMysql));
			return;
		}

		/*char * mQuery = 0;
		MakeAnyLenString(&mQuery, "DELETE FROM tasks WHERE id=%u", (*Iter).id);
		mErrorLog->Log(eqEmuLogSQL, "%s", mQuery);
		if (mysql_query(mMysql, mQuery)) {
			mErrorLog->Log(eqEmuLogBoth, "MySQL Error: %s", mysql_error(mMysql));
			return;
		}

		MakeAnyLenString(&mQuery, "INSERT INTO `tasks` (`id`,`duration`,`title`,`description`,`reward`,`rewardid`,`cashreward`,`xpreward`,`rewardmethod`,`startzone`, `minlevel`, `maxlevel`) VALUES (%u,%u,'%s','%s','%s',%u,%u,%u,%u,%u,%u,%u)",
			ourTask.id, ourTask.duration, MakeStringSQLSafe(ourTask.title).mb_str(), MakeStringSQLSafe(ourTask.desc).mb_str(), MakeStringSQLSafe(ourTask.reward).mb_str(), ourTask.rewardid, ourTask.cashreward, ourTask.xpreward, ourTask.rewardmethod, ourTask.startzone, ourTask.level_min, ourTask.level_max);

		mErrorLog->Log(eqEmuLogSQL, "%s", mQuery);
		if (mysql_query(mMysql, mQuery)) {
			mErrorLog->Log(eqEmuLogBoth, "MySQL Error: %s", mysql_error(mMysql));
			return;
		}*/

		(*Iter).cashreward = ourTask.cashreward;
		strcpy((*Iter).desc, ourTask.desc);
		(*Iter).duration = ourTask.duration;
		(*Iter).level_max = ourTask.level_max;
		(*Iter).level_min = ourTask.level_min;
		strcpy((*Iter).reward, ourTask.reward);
		(*Iter).rewardid = ourTask.rewardid;
		(*Iter).rewardmethod = ourTask.rewardmethod;
		(*Iter).startzone = ourTask.startzone;
		strcpy((*Iter).title, ourTask.title);
		(*Iter).xpreward = ourTask.xpreward;
		(*Iter).repeatable = ourTask.repeatable;

		getStr.Printf("%u:%s", (*Iter).id, (*Iter).title);
        ItemSelectionList->SetString(openedIndex, getStr);
		mErrorLog->Log(eqEmuLogBoth, "Save finished.");
	}
	else{
		mErrorLog->Log(eqEmuLogBoth, "Opened index for Task Save not valid.");
	}

}

void MainFrame::ContextMenuTaskList()
{
	if(!mMysql){
		mErrorLog->Log(eqEmuLogBoth, "Context menu cannot open, not connected to db");
		return;
	}

	wxMenu *mMenu;
	mMenu = new wxMenu();

	mMenu->Append(MENU_NewTask, wxT("New Task"), wxT("Creates a new task"));
	mMenu->Append(MENU_DeleteTask, wxT("Delete Task"), wxT("Deletes the selected task"));
	mMenu->AppendSeparator();
	mMenu->Append(MENU_SaveTask, wxT("Save Task"), wxT("Saves the opened task"));

	PopupMenu(mMenu);
	delete mMenu;
}
