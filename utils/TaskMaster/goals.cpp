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

void MainFrame::GoalsListBoxDoubleClick(wxCommandEvent& event)
{
	int * i = (int*) GoalsSelectionList->GetClientData(event.GetInt());
	if(i){
		int mI = *i;
		PopulateGoalValues(mI);
	}
}

void MainFrame::PopulateGoals()
{
	mErrorLog->Log(eqEmuLogBoth, "Adding goals to the goal list box");
	vector<eqtask_goallist>::iterator Iter;

	if(goalTaskList.size() == 0)
	{
		mErrorLog->Log(eqEmuLogBoth, "No goals in DB, cannot add to list box");
		return;
	}

	for(Iter = goalTaskList.begin(); Iter != goalTaskList.end(); Iter++)
	{
		eqtask_goallist val = (*Iter);
		bool exists = false;
		if(GoalsSelectionList->GetCount() > 0){
			for(unsigned int i = 0; i < GoalsSelectionList->GetCount(); i++)
			{
				int *cd = (int*)GoalsSelectionList->GetClientData(i);
				if(*cd == val.id){
					exists = true;
				}
			}
		}
		if(!exists){
			int *newCD = new int;
			*newCD = val.id;
			wxString newStr;
			newStr.Printf("%u", *newCD);
			GoalsSelectionList->Append(newStr, (void*)newCD);
		}
	}
}

void MainFrame::PopulateGoalValues(unsigned int goalid)
{
	GoalsValuesList->Enable();
	mGoalsNewValueButton->Enable();
	mGoalsDeleteValueButton->Enable();
	mGoalsChangeValueButton->Enable();
	ClearGoalValues();
	openedGoal = goalid;
	vector<eqtask_goallist>::iterator Iter;
	if(goalTaskList.size() != 0){
		for(Iter = goalTaskList.begin(); Iter != goalTaskList.end(); Iter++)
		{
			if((*Iter).id == goalid){
				int *i = new int;
				*i = (*Iter).value;

				wxString curStr;
				curStr.Clear();
				curStr.Printf("%u", (*Iter).value);

				GoalsValuesList->Append(curStr, (void*)i);
			}
		}
	}
}

void MainFrame::ClearGoalValues()
{
	if(GoalsValuesList->GetCount() > 0){
		for(unsigned int x = 0; x < GoalsValuesList->GetCount(); x++)
		{
			int *i = (int*)GoalsValuesList->GetClientData(x);
			if(i){
				delete i;
				i = 0;
			}
		}
	}
	GoalsValuesList->Clear();
}

void MainFrame::NewGoal(wxCommandEvent& event)
{
	if(!mMysql){
		mErrorLog->Log(eqEmuLogBoth, "New goal failed, not connected to db");
		return;
	}
	int newID = wxGetNumberFromUser("", "New ID:", "Input ID", 0, 0, 9999999);
	int newVal = wxGetNumberFromUser("", "New Value:", "Input Value", 0, 0, 9999999);

	bool exists = false;
	for(unsigned int x = 0; x < GoalsSelectionList->GetCount(); x++)
	{
		int *di = (int*)GoalsSelectionList->GetClientData(x);
		if(*di == newID)
			exists = true;
	}

	if(exists){
		mErrorLog->Log(eqEmuLogBoth, "New goal failed, already exists");
		return;
	}

	if(newID <= 0){
		mErrorLog->Log(eqEmuLogBoth, "New goal failed, ID <= 0");
		return;
	}

	if(newVal < 0){
		mErrorLog->Log(eqEmuLogBoth, "New goal failed, Val < 0");
		return;
	}
	char * mQuery = 0;
	MakeAnyLenString(&mQuery, "INSERT INTO `goallists` (`listid`,`entry`) VALUES (%u,%u)", newID, newVal);
	mErrorLog->Log(eqEmuLogSQL, "%s", mQuery);
	if (mysql_query(mMysql, mQuery)) {
		mErrorLog->Log(eqEmuLogBoth, "MySQL Error: %s", mysql_error(mMysql));
		return;
	}

	eqtask_goallist newGoal;
	newGoal.id = newID;
	newGoal.value = newVal;
	goalTaskList.push_back(newGoal);

	wxString newGoalStr;
	newGoalStr.Printf("%u", newGoal.id);
	int * i = new int;
	*i = newGoal.id;
	GoalsSelectionList->Append(newGoalStr, (void*)i);
}

void MainFrame::DeleteGoal(wxCommandEvent& event)
{
	if(!mMysql){
		mErrorLog->Log(eqEmuLogBoth, "Delete goal failed, not connected to db");
		return;
	}

	if(GoalsSelectionList->GetCount() == 0)
	{
		mErrorLog->Log(eqEmuLogBoth, "Delete goal failed, does not exist");
		return;
	}

	if(GoalsSelectionList->GetSelection() == wxNOT_FOUND)
	{
		mErrorLog->Log(eqEmuLogBoth, "Delete goal failed, nothing selected");
		return;
	}

	int *mId;
	mId = (int*)GoalsSelectionList->GetClientData(GoalsSelectionList->GetSelection());

	if(!mId){
		mErrorLog->Log(eqEmuLogBoth, "Delete goal failed, mId is NULL");
		return;
	}

	int reply = wxMessageBox("Are you sure?", "Confirm Delete", wxYES_NO, this);
	if(reply != wxYES)
	{
		mErrorLog->Log(eqEmuLogBoth, "User aborted delete of goal");
		return;
	}

	int delVal = *mId;
	char * mQuery = 0;
	MakeAnyLenString(&mQuery, "DELETE FROM goallists where listid=%u", delVal);
	mErrorLog->Log(eqEmuLogSQL, "%s", mQuery);
	if (mysql_query(mMysql, mQuery)) {
		mErrorLog->Log(eqEmuLogBoth, "MySQL Error: %s", mysql_error(mMysql));
		return;
	}

	vector<eqtask_goallist>::iterator Iter;
	for(Iter = goalTaskList.begin(); Iter != goalTaskList.end(); Iter++)
	{
		if((*Iter).id == delVal)
		{
			goalTaskList.erase(Iter);
			Iter = goalTaskList.begin();
		}
	}

	if(delVal == openedGoal){
		ClearGoalValues();
		openedGoal = 0;
		mGoalsNewValueButton->Disable();
		mGoalsDeleteValueButton->Disable();
		mGoalsChangeValueButton->Disable();
	}

	for(unsigned int x = 0; x < GoalsSelectionList->GetCount(); x++)
	{
		int * ni = (int*)GoalsSelectionList->GetClientData(x);
		if(ni){
			if(*ni == delVal){
				delete ni;
				ni = NULL;
				GoalsSelectionList->Delete(x);
				return;
			}
		}
	}
}

void MainFrame::NewGoalValue(wxCommandEvent& event)
{
	if(!mMysql){
		mErrorLog->Log(eqEmuLogBoth, "New goal value failed, not connected to db");
		return;
	}

	if(!GoalsValuesList->IsEnabled())
	{
		mErrorLog->Log(eqEmuLogBoth, "New goal value failed, goal values not active");
		return;
	}
	int newVal = wxGetNumberFromUser("", "Value:", "Input Value", 0, 0, 9999999);
	if(newVal < 0){
		mErrorLog->Log(eqEmuLogBoth, "New goal value failed, Val < 0");
	}

	int id = openedGoal;

	char * mQuery = 0;
	MakeAnyLenString(&mQuery, "INSERT INTO `goallists` (`listid`,`entry`) VALUES (%u,%u)", id, newVal);
	mErrorLog->Log(eqEmuLogSQL, "%s", mQuery);
	if (mysql_query(mMysql, mQuery)) {
		mErrorLog->Log(eqEmuLogBoth, "MySQL Error: %s", mysql_error(mMysql));
		return;
	}
	int * iptr = new int;
	*iptr = newVal;
	wxString iStr;
	iStr.Printf("%u", newVal);
	GoalsValuesList->Append(iStr, (void*)iptr);

	eqtask_goallist newGoal;
	newGoal.id = id;
	newGoal.value = newVal;
	goalTaskList.push_back(newGoal);
}

void MainFrame::DeleteGoalValue(wxCommandEvent& event)
{
	if(!mMysql){
		mErrorLog->Log(eqEmuLogBoth, "Delete goal value failed, not connected to db");
		return;
	}

	if(!GoalsValuesList->IsEnabled()){
		mErrorLog->Log(eqEmuLogBoth, "Delete goal value failed, goal values not active");
		return;
	}

	if(GoalsValuesList->GetCount() <= 1)
	{
		mErrorLog->Log(eqEmuLogBoth, "Delete goal value failed, too few values");
		return;
	}

	int ourId = openedGoal;
	int ourVal;
	int *iPtr = (int*)GoalsValuesList->GetClientData(GoalsValuesList->GetSelection());

	if(!iPtr){
		mErrorLog->Log(eqEmuLogBoth, "Delete goal value failed, iPtr is NULL");
		return;
	}

	int reply = wxMessageBox("Are you sure?", "Confirm Delete", wxYES_NO, this);
	if(reply != wxYES)
	{
		mErrorLog->Log(eqEmuLogBoth, "User aborted delete of goal value");
		return;
	}

	ourVal = *iPtr;

	char * mQuery = 0;
	MakeAnyLenString(&mQuery, "DELETE FROM goallists WHERE listid=%u AND entry=%u", ourId, ourVal);
	mErrorLog->Log(eqEmuLogSQL, "%s", mQuery);
	if (mysql_query(mMysql, mQuery)) {
		mErrorLog->Log(eqEmuLogBoth, "MySQL Error: %s", mysql_error(mMysql));
		return;
	}

	for(unsigned int x = 0; x < GoalsValuesList->GetCount(); x++)
	{
		int *ip = (int*)GoalsValuesList->GetClientData(x);
		if(ip){
			if(*ip == ourVal){
				GoalsValuesList->Delete(x);
				break;
			}
		}
	}
	vector<eqtask_goallist>::iterator Iter;
	for(Iter = goalTaskList.begin(); Iter != goalTaskList.end(); Iter++)
	{
		if((*Iter).value == ourVal && (*Iter).id == ourId)
		{
			goalTaskList.erase(Iter);
			return;
		}
	}
}

void MainFrame::ChangeGoalValue(wxCommandEvent& event)
{
	if(!mMysql){
		mErrorLog->Log(eqEmuLogBoth, "Modify goal value failed, not connected to db");
		return;
	}

	if(!GoalsValuesList->IsEnabled()){
		mErrorLog->Log(eqEmuLogBoth, "Modify goal value failed, goal values not active");
		return;
	}

	if(GoalsValuesList->GetSelection() == wxNOT_FOUND)
	{
		mErrorLog->Log(eqEmuLogBoth, "Modify goal value failed, no selection found");
		return;
	}

	int changedId = openedGoal;
	int changedGoal;
	int * changedGoalPtr = (int*)GoalsValuesList->GetClientData(GoalsValuesList->GetSelection());

	if(!changedGoalPtr)
	{
		mErrorLog->Log(eqEmuLogBoth, "Modify goal value failed, changedGoalPtr was NULL");
		return;
	}

	changedGoal = *changedGoalPtr;

	int newVal = wxGetNumberFromUser("", "Value:", "Input Value", changedGoal, 0, 9999999);

	char * mQuery = 0;
	MakeAnyLenString(&mQuery, "DELETE FROM goallists WHERE listid=%u AND entry=%u", changedId, changedGoal);
	mErrorLog->Log(eqEmuLogSQL, "%s", mQuery);
	if (mysql_query(mMysql, mQuery)) {
		mErrorLog->Log(eqEmuLogBoth, "MySQL Error: %s", mysql_error(mMysql));
		return;
	}

	MakeAnyLenString(&mQuery, "INSERT INTO `goallists` (`listid`,`entry`) VALUES (%u,%u)", changedId, newVal);
	mErrorLog->Log(eqEmuLogSQL, "%s", mQuery);
	if (mysql_query(mMysql, mQuery)) {
		mErrorLog->Log(eqEmuLogBoth, "MySQL Error: %s", mysql_error(mMysql));
		return;
	}

	for(unsigned int x = 0; x < GoalsValuesList->GetCount(); x++)
	{
		int *ip = (int*)GoalsValuesList->GetClientData(x);
		if(ip)
		{
			if(*ip == changedGoal)
			{
				*ip = newVal;
				wxString newStr;
				newStr.Printf("%u", newVal);
				GoalsValuesList->SetString(x, newStr);
				break;
			}
		}
	}

	vector<eqtask_goallist>::iterator Iter;
	for(Iter = goalTaskList.begin(); Iter != goalTaskList.end(); Iter++)
	{
		if((*Iter).id == changedId && (*Iter).value == changedGoal)
		{
			(*Iter).value = newVal;
			return;
		}
	}

}

void MainFrame::ContextMenuGoalList()
{
	if(!mMysql){
		mErrorLog->Log(eqEmuLogBoth, "Context menu cannot open, not connected to db");
		return;
	}

	wxMenu *mMenu;
	mMenu = new wxMenu();

	mMenu->Append(MENU_NewGoal, wxT("New Goal"), wxT("Creates a new goal list entry"));
	mMenu->Append(MENU_DeleteGoal, wxT("Delete Goal"), wxT("Deletes the selected goal list entry"));

	PopupMenu(mMenu);
	delete mMenu;
}

void MainFrame::ContextMenuGoalValueList()
{
	if(!mMysql){
		mErrorLog->Log(eqEmuLogBoth, "Context menu cannot open, not connected to db");
		return;
	}

	wxMenu *mMenu;
	mMenu = new wxMenu();

	mMenu->Append(MENU_AddGoalItem, wxT("New Goal Value"), wxT("Creates a new goal value entry for the opened list"));
	mMenu->Append(MENU_DeleteGoalItem, wxT("Delete Goal Value"), wxT("Deletes the selected goal value entry"));
	mMenu->Append(MENU_ModifyGoalItem, wxT("Modify Goal Value"), wxT("Attempts to change the selected goal value entry"));

	PopupMenu(mMenu);
	delete mMenu;
}