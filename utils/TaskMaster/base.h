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
#ifndef EQWX_BASE__H
#define EQWX_BASE__H
#include <wx/frame.h>
#include <wx/textctrl.h>
#include <wx/notebook.h>
#include <mysql.h>
#include "ErrorLog.h"
#include "tasks.h"
#include "items.h"
#include <vector>

#define TASK_MASTER_VERSION "1.0.8"

class MainApp: public wxApp
{
public:
	virtual bool OnInit();
};

class MainFrame: public wxFrame
{
public:
	MainFrame( const wxString& title, const wxPoint& pos, const wxSize& size );
	wxTextCtrl *MainEditBox;
	wxMenuBar *MainMenu;

	//various elements
	wxBoxSizer* BoxSizer1;
	wxListBox *ItemSelectionList;
	wxNotebook *MainNotebookBack;

	wxPanel *MainPanel1;

	/*General Panel Controls*/
	wxStaticText *mTaskNameLabel;
	wxTextCtrl *mTaskName;
	wxStaticText *mTaskDescLabel;
	wxTextCtrl *mTaskDesc;
	wxStaticText *mTaskMinLvlLabel;
	wxTextCtrl *mTaskMinLvl;
	wxStaticText *mTaskMaxLvlLabel;
	wxTextCtrl *mTaskMaxLvl;
	wxStaticText *mTaskDurationLabel;
	wxTextCtrl *mTaskDuration;
	wxStaticBox *mRewardsBox;
	wxStaticText *mRewardNameLabel;
	wxTextCtrl *mRewardName;
	wxStaticText *mRewardIDLabel;
	wxTextCtrl *mRewardID;
	wxStaticText *mRewardCashLabel;
	wxTextCtrl *mRewardCash;
	wxStaticText *mRewardXPLabel;
	wxTextCtrl *mRewardXP;
	wxStaticText *mRewardMethodLabel;
	wxChoice *mRewardMethod;
	wxListBox *ShowRewardItems;
	wxButton *RefreshItems;
	wxStaticText *mStartZoneLabel;
	wxChoice *mStartZone;

	wxCheckBox *mTaskRepeatable;
	/*General Panel Controls End*/

	wxPanel *MainPanel2;

	/*Activities Panel Controls*/
	wxListBox *ActivitiesSelectionList;
	wxStaticText *mActText1Label;
	wxTextCtrl *mActText1;
	wxStaticText *mActText2Label;
	wxTextCtrl *mActText2;
	wxStaticText *mActText3Label;
	wxTextCtrl *mActText3;
	wxStaticText *mActivityZoneLabel;
	wxChoice *mActivityZone;
	wxCheckBox *mActivityOptional;
	wxStaticText *mActIDLabel;
	wxTextCtrl *mActID;
	wxStaticText *mActStepLabel;
	wxTextCtrl *mActStep;
	wxStaticText *mActTypeLabel;
	wxChoice *mActType;
	wxStaticBox *mActInfo;
	wxStaticText *mActInfoText;
	wxStaticText *mActDeliverLabel;
	wxTextCtrl *mActDeliver;
	wxStaticText *mActMethodLabel;
	wxChoice *mActMethod;
	wxStaticText *mActGoalIDLabel;
	wxTextCtrl *mActGoalID;
	wxStaticText *mActGoalCountLabel;
	wxTextCtrl *mActGoalCount;
	/*Activities Panel Controls End*/

	wxPanel *MainPanel3;

	/*Goals Panel Control Start*/
	wxListBox *GoalsSelectionList;
	wxListBox *GoalsValuesList;
	wxButton *mGoalsNewValueButton;
	wxButton *mGoalsDeleteValueButton;
	wxButton *mGoalsChangeValueButton;
	/*Goals Panel Control End*/

	wxPanel *MainPanel4;

	/*Proximity Panel Control Start*/
	wxListBox *ProximitySelectionList;
	wxStaticText *mProxIdLabel;
	wxTextCtrl *mProxId;
	wxStaticText *mProxMinxLabel;
	wxTextCtrl *mProxMinx;
	wxStaticText *mProxMaxxLabel;
	wxTextCtrl *mProxMaxx;
	wxStaticText *mProxMinyLabel;
	wxTextCtrl *mProxMiny;
	wxStaticText *mProxMaxyLabel;
	wxTextCtrl *mProxMaxy;
	wxStaticText *mProxMinzLabel;
	wxTextCtrl *mProxMinz;
	wxStaticText *mProxMaxzLabel;
	wxTextCtrl *mProxMaxz;
	wxStaticText *mProxZoneLabel;
	wxChoice *mProxZone;
	/*Proximity Panel Control End*/

	wxPanel *MainPanel5;
	/*Task Set Panel Control Start*/
	/*Task Set Panel Control End*/

	//error
	EQEmuErrorLog *mErrorLog;

	/*database stuff*/
	MYSQL *mMysql;
	char server[256];
	char user[256];
	char password[256];
	char database[256];
	bool GetDatabaseSettings();
	bool LoadTasks();
	bool LoadGoals();
	bool LoadActivities();
	bool LoadItems();
	bool LoadZones();
	bool LoadProximity();

	//I use vectors for everything speed of adding/removing isn't an issue for me.
	//Could use list for task, goals and activities if wanted.
	std::vector<eqtask> taskList;
	std::vector<eqtask_goallist> goalTaskList;
	std::vector<eqitem> itemList;
	std::vector<eqtask_activities> taskActivitiesList;
	std::vector<eqtask_zones> taskZoneList;
	std::vector<eqtask_proximity> taskProximityList;

	int selectedIndex;
	int openedIndex;
	unsigned int highestIndex;
	eqtask_activity_id openedActivity;
	int openedGoal;
	eqtask_prox openedProximity;

	void Connect(wxCommandEvent& event);
	void Save(wxCommandEvent& event);
	void Quit(wxCommandEvent& event);
	void NewTask(wxCommandEvent& event);
	void DeleteTask(wxCommandEvent& event);
	void SaveTask(wxCommandEvent& event);
	void About(wxCommandEvent& event);

	void NewActivity(wxCommandEvent& event);
	void DeleteActivity(wxCommandEvent& event);
	void SaveActivity(wxCommandEvent& event);

	void ListBoxDoubleClick(wxCommandEvent& event);
	void ListBoxSimpleSelect(wxCommandEvent& event);
	void OnRewardButton(wxCommandEvent& event);
	void ActivitiesListBoxSimpleSelect(wxCommandEvent& event);
	void ActivitiesListBoxDoubleClick(wxCommandEvent& event);
	void ActivityChoiceChange(wxCommandEvent& event);
	void PopulateActivities();
	void FillActivity(int id, int activityid, int step);

	void GoalsListBoxDoubleClick(wxCommandEvent& event);
	void PopulateGoals();
	void PopulateGoalValues(unsigned int goalid);
	void ClearGoalValues();
	void NewGoal(wxCommandEvent& event);
	void DeleteGoal(wxCommandEvent& event);
	void NewGoalValue(wxCommandEvent& event);
	void DeleteGoalValue(wxCommandEvent& event);
	void ChangeGoalValue(wxCommandEvent& event);

	void ProximityListBoxDoubleClick(wxCommandEvent& event);
	void NewProximity(wxCommandEvent& event);
	void DeleteProximity(wxCommandEvent& event);
	void SaveProximity(wxCommandEvent& event);
	void PopulateProximity();
	void FillProximityValues(unsigned int zone, unsigned int explore);

	void ShowRewardChange(int rewardType, int rewardId);
	void SetZoneSelectionById(int zid);
	void SetZoneSelectionByIdActivity(int zid);
	void SetZoneSelectionByIdProximity(int zid);
	wxString MakeStringSQLSafe(const char * c);

	void ContextClick(wxContextMenuEvent& event);
	void ContextMenuTaskList();
	void ContextMenuActivityList();
	void ContextMenuGoalList();
	void ContextMenuGoalValueList();
	void ContextMenuProximity();


	//we need to free the memory used when we change activites becase we attach it to
	//the items in our activities table and don't have any other way to track them so
	//Free it up before we call ActivitiesSelectionList->Clear();
	void ClearActivities();

	DECLARE_EVENT_TABLE()
};

enum
{
	TEXT_Main = wxID_HIGHEST + 1,
	MENU_Connect,
	MENU_Quit,
	MENU_NewTask,
	MENU_DeleteTask,
	MENU_SaveTask,
	LIST_Click,
	BUTTON_Reward,
	LIST_Click_Activities,
	MENU_NewActivity,
	MENU_DeleteActivity,
	MENU_SaveActivity,
	CHOICE_ActivityChoiceChange,
	MENU_About,
	LIST_Click_Goals,
	MENU_NewGoal,
	MENU_DeleteGoal,
	MENU_AddGoalItem,
	MENU_DeleteGoalItem,
	MENU_ModifyGoalItem,
	BUTTON_AddGoalItem,
	BUTTON_DeleteGoalItem,
	BUTTON_ModifyGoalItem,
	MENU_NewProximity,
	MENU_DeleteProximity,
	MENU_SaveProximity,
	LIST_Click_Proximity,
};

#endif