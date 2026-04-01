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
#include <wx/aboutdlg.h>
#include <wx/cmdline.h>
#include <mysql.h>
#include <vector>
#include "tasks.h"
#include "utility.h"
#include "base.h"
#include "ErrorLog.h"

using namespace std;

//Defined Events
BEGIN_EVENT_TABLE ( MainFrame, wxFrame )
EVT_MENU(MENU_Connect, MainFrame::Connect)
EVT_MENU(MENU_Quit, MainFrame::Quit)
EVT_MENU(MENU_NewTask, MainFrame::NewTask)
EVT_MENU(MENU_DeleteTask, MainFrame::DeleteTask)
EVT_MENU(MENU_SaveTask, MainFrame::SaveTask)
EVT_MENU(MENU_NewActivity, MainFrame::NewActivity)
EVT_MENU(MENU_DeleteActivity, MainFrame::DeleteActivity)
EVT_MENU(MENU_SaveActivity, MainFrame::SaveActivity)
EVT_MENU(MENU_About, MainFrame::About)
EVT_LISTBOX_DCLICK(LIST_Click, MainFrame::ListBoxDoubleClick)
EVT_LISTBOX(LIST_Click, MainFrame::ListBoxSimpleSelect)
EVT_BUTTON(BUTTON_Reward, MainFrame::OnRewardButton )
EVT_LISTBOX_DCLICK(LIST_Click_Activities, MainFrame::ActivitiesListBoxDoubleClick)
EVT_LISTBOX(LIST_Click_Activities, MainFrame::ActivitiesListBoxSimpleSelect)
EVT_CHOICE(CHOICE_ActivityChoiceChange, MainFrame::ActivityChoiceChange)
EVT_CONTEXT_MENU(MainFrame::ContextClick)
EVT_LISTBOX_DCLICK(LIST_Click_Goals, MainFrame::GoalsListBoxDoubleClick)
EVT_MENU(MENU_NewGoal, MainFrame::NewGoal)
EVT_MENU(MENU_DeleteGoal, MainFrame::DeleteGoal)
EVT_MENU(MENU_AddGoalItem, MainFrame::NewGoalValue)
EVT_MENU(MENU_DeleteGoalItem, MainFrame::DeleteGoalValue)
EVT_MENU(MENU_ModifyGoalItem, MainFrame::ChangeGoalValue)
EVT_BUTTON(BUTTON_AddGoalItem, MainFrame::NewGoalValue)
EVT_BUTTON(BUTTON_DeleteGoalItem, MainFrame::DeleteGoalValue)
EVT_BUTTON(BUTTON_ModifyGoalItem, MainFrame::ChangeGoalValue)
EVT_MENU(MENU_NewProximity, MainFrame::NewProximity)
EVT_MENU(MENU_DeleteProximity, MainFrame::DeleteProximity)
EVT_MENU(MENU_SaveProximity, MainFrame::SaveProximity)
EVT_LISTBOX_DCLICK(LIST_Click_Proximity, MainFrame::ProximityListBoxDoubleClick)
END_EVENT_TABLE()

//Our main() thread
IMPLEMENT_APP_CONSOLE(MainApp)

bool MainApp::OnInit()
{
	MainFrame *MainWin = new MainFrame(wxT("Task Master"), wxDefaultPosition, //wxPoint(1,1),
		wxSize(800, 600)); // Create an instance of our frame, or window
	MainWin->Show(TRUE); // show the window
	SetTopWindow(MainWin);// and finally, set it as the main window

	return TRUE;
}

MainFrame::MainFrame(const wxString& title,
					 const wxPoint& pos, const wxSize& size)
					 : wxFrame((wxFrame *) NULL, -1, title, pos, size, (wxCAPTION|wxCLOSE_BOX|wxMINIMIZE_BOX|wxSYSTEM_MENU|wxCLIP_CHILDREN|wxTAB_TRAVERSAL))
{
	mErrorLog = new EQEmuErrorLog;
	CreateStatusBar(1, 0);

	mErrorLog->Log(eqEmuLogBoth, "Creating Menus.");
	MainMenu = new wxMenuBar();
	wxMenu *FileMenu = new wxMenu();

	FileMenu->Append(MENU_Connect, wxT("&Connect"),	wxT("Connect to the Database"));
	FileMenu->AppendSeparator();
	FileMenu->Append(MENU_Quit, wxT("&Quit"), wxT("Quit"));
	MainMenu->Append(FileMenu, wxT("&Main"));

	wxMenu *EditMenu = new wxMenu();

	EditMenu->Append(MENU_NewTask, wxT("&New Task"), wxT("Creates a new task"));
	EditMenu->Append(MENU_DeleteTask, wxT("&Delete Task"), wxT("Deletes the selected task"));
	EditMenu->AppendSeparator();
	EditMenu->Append(MENU_SaveTask, wxT("&Save Task"), wxT("Saves the opened task"));
	MainMenu->Append(EditMenu, wxT("&Tasks"));

	wxMenu *ActMenu = new wxMenu();
	ActMenu->Append(MENU_NewActivity, wxT("&New Activity"), wxT("Create a new activity for this task"));
	ActMenu->Append(MENU_DeleteActivity, wxT("&Delete Activity"), wxT("Deletes the selected activity for this task"));
	ActMenu->AppendSeparator();
	ActMenu->Append(MENU_SaveActivity, wxT("&Save Activity"), wxT("Saves the opened task activity"));
	MainMenu->Append(ActMenu, wxT("&Activities"));

	wxMenu *GoalsMenu = new wxMenu();
	GoalsMenu->Append(MENU_NewGoal, wxT("&New Goal"), wxT("Creates a new goal list entry"));
	GoalsMenu->Append(MENU_DeleteGoal, wxT("&Delete Goal"), wxT("Deletes the selected goal list entry"));
	GoalsMenu->AppendSeparator();
	GoalsMenu->Append(MENU_AddGoalItem, wxT("N&ew Goal Value"), wxT("Saves the opened task activity"));
	GoalsMenu->Append(MENU_DeleteGoalItem, wxT("De&lete Goal Value"), wxT("Deletes the selected goal value entry"));
	GoalsMenu->Append(MENU_ModifyGoalItem, wxT("M&odify Goal Value"), wxT("Attempts to change the selected goal value entry"));
	MainMenu->Append(GoalsMenu, wxT("&Goals"));

	wxMenu *ProximityMenu = new wxMenu();
	ProximityMenu->Append(MENU_NewProximity, wxT("&New Proximity"), wxT("Creates a new proximity entry"));
	ProximityMenu->Append(MENU_DeleteProximity, wxT("&Delete Proximity"), wxT("Deletes the selected proximity entry"));
	ProximityMenu->AppendSeparator();
	ProximityMenu->Append(MENU_SaveProximity, wxT("&Save Proximity"), wxT("Saves the opened proximity entry"));
	MainMenu->Append(ProximityMenu, wxT("&Proximity"));

	wxMenu *AboutMenu = new wxMenu();
	AboutMenu->Append(MENU_About, wxT("A&bout"), wxT("Program Information"));
	MainMenu->Append(AboutMenu, wxT("A&bout"));

	SetMenuBar(MainMenu);

	mErrorLog->Log(eqEmuLogBoth, "Creating Sizer.");
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);

	mErrorLog->Log(eqEmuLogBoth, "Creating Item List Box.");
	ItemSelectionList = new wxListBox(this, LIST_Click, wxDefaultPosition, wxSize( -1,-1 ), 0, NULL, 0);

	mErrorLog->Log(eqEmuLogBoth, "Adding Item List Box to Sizer.");
	BoxSizer1->Add(ItemSelectionList, 0, wxALIGN_TOP|wxALL|wxEXPAND, 5);

	mErrorLog->Log(eqEmuLogBoth, "Creating Notebook Backing.");
	MainNotebookBack = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), 0);

	mErrorLog->Log(eqEmuLogBoth, "Creating Panels.");
	MainPanel1 = new wxPanel(MainNotebookBack, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	MainPanel2 = new wxPanel(MainNotebookBack, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	MainPanel3 = new wxPanel(MainNotebookBack, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	MainPanel4 = new wxPanel(MainNotebookBack, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	MainPanel5 = new wxPanel(MainNotebookBack, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	mErrorLog->Log(eqEmuLogBoth, "Adding Panels to Notebook Backing.");
	MainNotebookBack->AddPage(MainPanel1, wxT("General"), true);
	MainNotebookBack->AddPage(MainPanel2, wxT("Activities"), false);
	MainNotebookBack->AddPage(MainPanel3, wxT("Goals"), false);
	MainNotebookBack->AddPage(MainPanel4, wxT("Proximities"), false);
	MainNotebookBack->AddPage(MainPanel5, wxT("Task Sets"), false);

	/*General Page Elements*/
	mErrorLog->Log(eqEmuLogBoth, "Adding elements to General Page");
	//name label + text
	mTaskNameLabel = new wxStaticText(MainPanel1, -1, "Name:",  wxPoint(0,2), wxDefaultSize);
	mTaskName = new wxTextCtrl(MainPanel1, -1, wxT("Task Name"), wxPoint(65,0), wxSize(300,20));
	mTaskName->SetMaxLength(100);
	mTaskName->Disable();

	//desc label + text
	mTaskDescLabel = new wxStaticText(MainPanel1, -1, "Desc:",  wxPoint(0,32), wxDefaultSize);
	mTaskDesc = new wxTextCtrl(MainPanel1, -1, wxT("Task Description"), wxPoint(65,30), wxSize(300,200), wxTE_MULTILINE);
	mTaskDesc->SetMaxLength(2047);
	mTaskDesc->Disable();

	//Min Level label + text
	mTaskMinLvlLabel = new wxStaticText(MainPanel1, -1, "Min Lvl:",  wxPoint(375,2), wxDefaultSize);
	mTaskMinLvl = new wxTextCtrl(MainPanel1, -1, wxT("0"), wxPoint(450,0), wxSize(25,20), wxTE_DONTWRAP, wxTextValidator(wxFILTER_NUMERIC));
	mTaskMinLvl->SetMaxLength(2);
	mTaskMinLvl->Disable();

	//Max Level label + text
	mTaskMaxLvlLabel = new wxStaticText(MainPanel1, -1, "Max Lvl:",  wxPoint(500,2), wxDefaultSize);
	mTaskMaxLvl = new wxTextCtrl(MainPanel1, -1, wxT("0"), wxPoint(580,0), wxSize(25,20), wxTE_DONTWRAP, wxTextValidator(wxFILTER_NUMERIC));
	mTaskMaxLvl->SetMaxLength(2);
	mTaskMaxLvl->Disable();

	//duration
	mTaskDurationLabel = new wxStaticText(MainPanel1, -1, "Duration:",  wxPoint(375,32), wxDefaultSize);
	mTaskDuration = new wxTextCtrl(MainPanel1, -1, wxT("0"), wxPoint(450,30), wxSize(70,20), wxTE_DONTWRAP, wxTextValidator(wxFILTER_NUMERIC));
	mTaskDuration->SetMaxLength(9);
	mTaskDuration->Disable();

	//box to enclose rewards
	mRewardsBox = new wxStaticBox(MainPanel1, -1, "Rewards", wxPoint(0,300), wxSize(644,210));
	mRewardNameLabel = new wxStaticText(mRewardsBox, -1, "Reward:",  wxPoint(10,17), wxDefaultSize);
	mRewardName = new wxTextCtrl(mRewardsBox, -1, wxT(""), wxPoint(80,15), wxSize(280,20));
	mRewardName->SetMaxLength(64);
	mRewardName->Disable();

	mRewardIDLabel = new wxStaticText(mRewardsBox, -1, "Reward ID:",  wxPoint(10,47), wxDefaultSize);
	mRewardID = new wxTextCtrl(mRewardsBox, -1, wxT("0"), wxPoint(160,45), wxSize(70,20), wxTE_DONTWRAP, wxTextValidator(wxFILTER_NUMERIC));
	mRewardID->SetMaxLength(9);
	mRewardID->Disable();

	mRewardCashLabel = new wxStaticText(mRewardsBox, -1, "Money Rewarded:",  wxPoint(10,77), wxDefaultSize);
	mRewardCash = new wxTextCtrl(mRewardsBox, -1, wxT("0"), wxPoint(160,75), wxSize(70,20), wxTE_DONTWRAP, wxTextValidator(wxFILTER_NUMERIC));
	mRewardCash->SetMaxLength(9);
	mRewardCash->Disable();

	mRewardXPLabel = new wxStaticText(mRewardsBox, -1, "Experience Rewarded:",  wxPoint(10,107), wxDefaultSize);
	mRewardXP = new wxTextCtrl(mRewardsBox, -1, wxT("0"), wxPoint(160,105), wxSize(70,20), wxTE_DONTWRAP, wxTextValidator(wxFILTER_NUMERIC));
	mRewardXP->SetMaxLength(9);
	mRewardXP->Disable();

	mRewardMethodLabel = new wxStaticText(mRewardsBox, -1, "Reward Method:",  wxPoint(10,137), wxDefaultSize);
	mRewardMethod = new wxChoice(mRewardsBox, -1, wxPoint(160, 135), wxDefaultSize);
	wxString mRewardMethodStr;
	mRewardMethodStr.Printf("%s", "(0) Single Item Id");
	mRewardMethod->Append(mRewardMethodStr);
	mRewardMethodStr.clear();
	mRewardMethodStr.Printf("%s", "(1) List from Goal List");
	mRewardMethod->Append(mRewardMethodStr);
	mRewardMethodStr.clear();
	mRewardMethodStr.Printf("%s", "(2) Perl Quest Reward");
	mRewardMethod->Append(mRewardMethodStr);
	mRewardMethod->Select(0);
	mRewardMethod->Disable();

	ShowRewardItems = new wxListBox(mRewardsBox, -1, wxPoint(375, 40), wxSize(250,160), 0, NULL, 0);
	ShowRewardItems->Disable();

	RefreshItems = new wxButton(mRewardsBox,BUTTON_Reward,"Refresh Item List", wxPoint(375,10), wxSize(250,25));
	RefreshItems->Disable();

	mStartZoneLabel =  new wxStaticText(MainPanel1, -1, "Start Zone:",  wxPoint(375,62), wxDefaultSize);
	mStartZone = new wxChoice(MainPanel1, -1, wxPoint(460, 60), wxDefaultSize, 0, NULL, wxCB_SORT);
	mStartZone->Disable();

	mTaskRepeatable = new wxCheckBox(MainPanel1, -1, "Repeatable:", wxPoint(375, 87), wxDefaultSize, wxCHK_2STATE | wxALIGN_RIGHT);
	mTaskRepeatable->Disable();

	/*General Page Elements End*/

	/*Activities Page Elements*/
	mErrorLog->Log(eqEmuLogBoth, "Adding elements to Activities Page");
	ActivitiesSelectionList = new wxListBox(MainPanel2, LIST_Click_Activities, wxPoint(0,0), wxSize(100,510), 0, NULL, 0);
	ActivitiesSelectionList->Disable();


	mActText1Label = new wxStaticText(MainPanel2, -1, "Text 1:",  wxPoint(105,2), wxDefaultSize);
	mActText1 = new wxTextCtrl(MainPanel2, -1, wxT(""), wxPoint(185,0), wxSize(180,20), wxTE_DONTWRAP);
	mActText1->SetMaxLength(64);
	mActText1->Disable();

	mActText2Label = new wxStaticText(MainPanel2, -1, "Text 2:",  wxPoint(105,22), wxDefaultSize);
	mActText2 = new wxTextCtrl(MainPanel2, -1, wxT(""), wxPoint(185,20), wxSize(180,20), wxTE_DONTWRAP);
	mActText2->SetMaxLength(64);
	mActText2->Disable();

	mActText3Label = new wxStaticText(MainPanel2, -1, "Text 3:",  wxPoint(105,42), wxDefaultSize);
	mActText3 = new wxTextCtrl(MainPanel2, -1, wxT(""), wxPoint(185,40), wxSize(180,60), wxTE_MULTILINE);
	mActText3->SetMaxLength(128);
	mActText3->Disable();

	mActivityZoneLabel =  new wxStaticText(MainPanel2, -1, "Zone:",  wxPoint(380,2), wxDefaultSize);
	mActivityZone = new wxChoice(MainPanel2, -1, wxPoint(440, 0), wxDefaultSize, 0, NULL, wxCB_SORT);
	mActivityZone->Disable();

	mActivityOptional = new wxCheckBox(MainPanel2, -1, "Optional:", wxPoint(580, 2), wxDefaultSize, wxCHK_2STATE | wxALIGN_RIGHT);
	mActivityOptional->Disable();

	mActIDLabel = new wxStaticText(MainPanel2, -1, "Activity ID:",  wxPoint(105,107), wxDefaultSize);
	mActID = new wxTextCtrl(MainPanel2, -1, wxT(""), wxPoint(185,105), wxSize(25,20), wxTE_DONTWRAP, wxTextValidator(wxFILTER_NUMERIC));
	mActID->SetMaxLength(2);
	mActID->Disable();

	mActStepLabel = new wxStaticText(MainPanel2, -1, "Step:",  wxPoint(105,129), wxDefaultSize);
	mActStep = new wxTextCtrl(MainPanel2, -1, wxT(""), wxPoint(185,127), wxSize(25,20), wxTE_DONTWRAP, wxTextValidator(wxFILTER_NUMERIC));
	mActStep->SetMaxLength(2);
	mActStep->Disable();

	mActTypeLabel = new wxStaticText(MainPanel2, -1, "Type:",  wxPoint(105,155), wxDefaultSize);
	mActType = new wxChoice(MainPanel2, CHOICE_ActivityChoiceChange, wxPoint(185, 153), wxDefaultSize);
	mRewardMethodStr.Clear();
	mRewardMethodStr.Printf("%s", "(0) Unknown");
	mActType->Append(mRewardMethodStr);
	mRewardMethodStr.Clear();
	mRewardMethodStr.Printf("%s", "(1) Deliver");
	mActType->Append(mRewardMethodStr);
	mRewardMethodStr.Clear();
	mRewardMethodStr.Printf("%s", "(2) Kill");
	mActType->Append(mRewardMethodStr);
	mRewardMethodStr.Clear();
	mRewardMethodStr.Printf("%s", "(3) Loot");
	mActType->Append(mRewardMethodStr);
	mRewardMethodStr.Clear();
	mRewardMethodStr.Printf("%s", "(4) Speak To");
	mActType->Append(mRewardMethodStr);
	mRewardMethodStr.Clear();
	mRewardMethodStr.Printf("%s", "(5) Explore");
	mActType->Append(mRewardMethodStr);
	mRewardMethodStr.Clear();
	mRewardMethodStr.Printf("%s", "(6) Trade Skill");
	mActType->Append(mRewardMethodStr);
	mRewardMethodStr.Clear();
	mRewardMethodStr.Printf("%s", "(7) Fish");
	mActType->Append(mRewardMethodStr);
	mRewardMethodStr.Clear();
	mRewardMethodStr.Printf("%s", "(8) Forage");
	mActType->Append(mRewardMethodStr);
	mRewardMethodStr.Clear();
	mRewardMethodStr.Printf("%s", "(9) Use");
	mActType->Append(mRewardMethodStr);
	mRewardMethodStr.Clear();
	mRewardMethodStr.Printf("%s", "(10) Use");
	mActType->Append(mRewardMethodStr);
	mRewardMethodStr.Clear();
	mRewardMethodStr.Printf("%s", "(11) Touch");
	mActType->Append(mRewardMethodStr);
	mRewardMethodStr.Clear();
	mRewardMethodStr.Printf("%s", "(100) GiveCash");
	mActType->Append(mRewardMethodStr);
	mRewardMethodStr.Clear();
	mRewardMethodStr.Printf("%s", "(999) Custom");
	mActType->Append(mRewardMethodStr);
	mActType->Disable();
	mActType->Select(0);

	mActInfo = new wxStaticBox(MainPanel2, -1, "Info", wxPoint(425,385), wxSize(219,125));
	mActInfoText = new wxStaticText(mActInfo, -1, "Unknown Activity Type:\nThis is not a valid activity type",  wxPoint(25,25), wxDefaultSize);

	mActDeliverLabel = new wxStaticText(MainPanel2, -1, "Deliver to NPCID:",  wxPoint(105,182), wxDefaultSize);
	mActDeliver = new wxTextCtrl(MainPanel2, -1, wxT(""), wxPoint(215,180), wxSize(60,20), wxTE_DONTWRAP, wxTextValidator(wxFILTER_NUMERIC));
	mActDeliver->SetMaxLength(7);
	mActDeliver->Disable();

	mActMethodLabel = new wxStaticText(MainPanel2, -1, "Goal Method:",  wxPoint(105,402), wxDefaultSize);
	mActMethod = new wxChoice(MainPanel2, -1, wxPoint(200, 400), wxDefaultSize);

	mRewardMethodStr.Clear();
	mRewardMethodStr.Printf("%s", "(0) Single Goal ID");
	mActMethod->Append(mRewardMethodStr);
	mRewardMethodStr.Clear();
	mRewardMethodStr.Printf("%s", "(1) Goal ID List");
	mActMethod->Append(mRewardMethodStr);
	mRewardMethodStr.Clear();
	mRewardMethodStr.Printf("%s", "(2) Perl Quest");
	mActMethod->Append(mRewardMethodStr);
	mActMethod->Select(2);
	mActMethod->Disable();

	mActGoalIDLabel = new wxStaticText(MainPanel2, -1, "Goal ID:",  wxPoint(105,432), wxDefaultSize);
	mActGoalID = new wxTextCtrl(MainPanel2, -1, wxT(""), wxPoint(200,430), wxSize(60,20), wxTE_DONTWRAP, wxTextValidator(wxFILTER_NUMERIC));
	mActGoalID->SetMaxLength(7);
	mActGoalID->Disable();

	mActGoalCountLabel = new wxStaticText(MainPanel2, -1, "Goal Count:",  wxPoint(105,457), wxDefaultSize);
	mActGoalCount = new wxTextCtrl(MainPanel2, -1, wxT(""), wxPoint(200,455), wxSize(60,20), wxTE_DONTWRAP, wxTextValidator(wxFILTER_NUMERIC));
	mActGoalCount->SetMaxLength(8);
	mActGoalCount->Disable();

	/*Activities Page Elements End*/

	/*Goals Page Elements*/
	mErrorLog->Log(eqEmuLogBoth, "Adding elements to Goals Page");
	GoalsSelectionList = new wxListBox(MainPanel3, LIST_Click_Goals, wxPoint(0,0), wxSize(100,510), 0, NULL, 0);

	GoalsValuesList = new wxListBox(MainPanel3, -1, wxPoint(150,50), wxSize(150,300), 0, NULL, 0);
	GoalsValuesList->Disable();

	mGoalsNewValueButton = new wxButton(MainPanel3, BUTTON_AddGoalItem, "Add Value", wxPoint(302,275), wxSize(100,-1));
	mGoalsNewValueButton->Disable();
	mGoalsDeleteValueButton = new wxButton(MainPanel3, BUTTON_DeleteGoalItem, "Delete Value", wxPoint(302,300), wxSize(100,-1));
	mGoalsDeleteValueButton->Disable();
	mGoalsChangeValueButton = new wxButton(MainPanel3, BUTTON_ModifyGoalItem, "Change Value", wxPoint(302,325), wxSize(100,-1));
	mGoalsChangeValueButton->Disable();
	/*Goals Page Elements End*/

	/*Proximities Page Elements*/
	mErrorLog->Log(eqEmuLogBoth, "Adding elements to Proximities Page");
	ProximitySelectionList = new wxListBox(MainPanel4, LIST_Click_Proximity, wxPoint(0,0), wxSize(100,510), 0, NULL, 0);

	mProxIdLabel = new wxStaticText(MainPanel4, -1, "Id:",  wxPoint(105,12), wxDefaultSize);
	mProxId = new wxTextCtrl(MainPanel4, -1, wxT(""), wxPoint(150,10), wxSize(50,20), wxTE_DONTWRAP, wxTextValidator(wxFILTER_NUMERIC));
	mProxId->SetMaxLength(6);
	mProxId->Disable();
	mProxMinxLabel = new wxStaticText(MainPanel4, -1, "MinX:",  wxPoint(105,42), wxDefaultSize);
	mProxMinx = new wxTextCtrl(MainPanel4, -1, wxT(""), wxPoint(150,40), wxSize(80,20), wxTE_DONTWRAP, wxTextValidator(wxFILTER_NUMERIC));
	mProxMinx->SetMaxLength(12);
	mProxMinx->Disable();
	mProxMaxxLabel = new wxStaticText(MainPanel4, -1, "MaxX:",  wxPoint(255,42), wxDefaultSize);
	mProxMaxx = new wxTextCtrl(MainPanel4, -1, wxT(""), wxPoint(300,40), wxSize(80,20), wxTE_DONTWRAP, wxTextValidator(wxFILTER_NUMERIC));
	mProxMaxx->SetMaxLength(12);
	mProxMaxx->Disable();
	mProxMinyLabel = new wxStaticText(MainPanel4, -1, "MinY:",  wxPoint(105,72), wxDefaultSize);
	mProxMiny = new wxTextCtrl(MainPanel4, -1, wxT(""), wxPoint(150,70), wxSize(80,20), wxTE_DONTWRAP, wxTextValidator(wxFILTER_NUMERIC));
	mProxMiny->SetMaxLength(12);
	mProxMiny->Disable();
	mProxMaxyLabel = new wxStaticText(MainPanel4, -1, "MaxY:",  wxPoint(255,72), wxDefaultSize);
	mProxMaxy = new wxTextCtrl(MainPanel4, -1, wxT(""), wxPoint(300,70), wxSize(80,20), wxTE_DONTWRAP, wxTextValidator(wxFILTER_NUMERIC));
	mProxMaxy->SetMaxLength(12);
	mProxMaxy->Disable();
	mProxMinzLabel = new wxStaticText(MainPanel4, -1, "MinZ:",  wxPoint(105,102), wxDefaultSize);
	mProxMinz = new wxTextCtrl(MainPanel4, -1, wxT(""), wxPoint(150,100), wxSize(80,20), wxTE_DONTWRAP, wxTextValidator(wxFILTER_NUMERIC));
	mProxMinz->SetMaxLength(12);
	mProxMinz->Disable();
	mProxMaxzLabel = new wxStaticText(MainPanel4, -1, "MaxZ:",  wxPoint(255,102), wxDefaultSize);
	mProxMaxz = new wxTextCtrl(MainPanel4, -1, wxT(""), wxPoint(300,100), wxSize(80,20), wxTE_DONTWRAP, wxTextValidator(wxFILTER_NUMERIC));
	mProxMaxz->SetMaxLength(12);
	mProxMaxz->Disable();
	mProxZoneLabel = new wxStaticText(MainPanel4, -1, "Zone:",  wxPoint(105,202), wxDefaultSize);
	mProxZone = new wxChoice(MainPanel4, -1, wxPoint(150, 200), wxDefaultSize);
	mProxZone->Disable();
	/*Proximities Page Elements End*/

	mErrorLog->Log(eqEmuLogBoth, "Adding Notebook to Sizer.");
	BoxSizer1->Add(MainNotebookBack, 1, wxALIGN_TOP|wxEXPAND, 5);

	mErrorLog->Log(eqEmuLogBoth, "Setting Frame Sizer.");
	this->SetSizer(BoxSizer1);
	mErrorLog->Log(eqEmuLogBoth, "Calculating Frame Layout.");
	this->Layout();

	selectedIndex = -1; //nothing selected
	openedIndex = -1; //nothing opened
	highestIndex = 0;
	openedActivity.activityid = -1;
	openedActivity.id = -1;
	openedActivity.step = -1;
	openedGoal = 0;
	openedProximity.exploreid = 0xFFFFFFFF;
	openedProximity.zoneid = 0xFFFFFFFF;

	//database setting init.
	mMysql = NULL;
	memset(server,0,256);
	memset(user,0,256);
	memset(password,0,256);
	memset(database,0,256);
	if(!GetDatabaseSettings())
		Close(TRUE);
}

void MainFrame::Connect(wxCommandEvent& WXUNUSED(event))
{
	if(mMysql){
		mErrorLog->Log(eqEmuLogBoth, "Connect to database requested but database connection exists.");
		return;
	}
	ItemSelectionList->Clear();
	mErrorLog->Log(eqEmuLogBoth, "Connect to database requested.");

	if(!mMysql){
		mMysql = mysql_init(NULL);
	}

	if(mMysql){
		if (!mysql_real_connect(mMysql, server,
			user, password, database, 0, NULL, 0)) {
				mErrorLog->Log(eqEmuLogBoth, "MySQL Connection Error: %s", mysql_error(mMysql));
				Close(TRUE);
				return;
			}

		if(!LoadTasks()){
			mysql_close(mMysql);
			mErrorLog->Log(eqEmuLogBoth, "Failed to load tasks, exiting.");
			Close(TRUE);
			return;
		}

		if(!LoadGoals()){
			mysql_close(mMysql);
			mErrorLog->Log(eqEmuLogBoth, "Failed to load goals, exiting.");
			Close(TRUE);
			return;
		}

		if(!LoadActivities()){
			mysql_close(mMysql);
			mErrorLog->Log(eqEmuLogBoth, "Failed to load activities, exiting.");
			Close(TRUE);
			return;
		}

		if(!LoadItems()){
			mysql_close(mMysql);
			mErrorLog->Log(eqEmuLogBoth, "Failed to load items, exiting.");
			Close(TRUE);
			return;
		}

		if(!LoadZones()){
			mysql_close(mMysql);
			mErrorLog->Log(eqEmuLogBoth, "Failed to load zones, exiting.");
			Close(TRUE);
			return;
		}

		if(!LoadProximity()){
			mysql_close(mMysql);
			mErrorLog->Log(eqEmuLogBoth, "Failed to load proximities, exiting.");
			Close(TRUE);
			return;
		}

		PopulateGoals();
		PopulateProximity();
	}
}

void MainFrame::Quit(wxCommandEvent& WXUNUSED(event))
{
	mErrorLog->Log(eqEmuLogBoth, "Exit requested, exiting.");
	delete mErrorLog;
	mErrorLog = NULL;
	mysql_close(mMysql);

	Close(TRUE);
}

void MainFrame::ListBoxSimpleSelect(wxCommandEvent& event)
{
	selectedIndex = event.GetInt();
}

void MainFrame::ListBoxDoubleClick(wxCommandEvent& event)
{
	mErrorLog->Log(eqEmuLogBoth, "Double clicked on item %d.", event.GetInt());
	openedIndex = event.GetInt();

	vector<eqtask>::iterator Iter;
	Iter = taskList.begin();
	Iter += openedIndex;
	eqtask mTask = *Iter;

	wxString mStr;
	mStr.Printf("%s", mTask.title);
	mTaskName->Clear();
    mTaskName->AppendText(mStr);
	mTaskName->Enable();
	mStr.clear();

	mStr.Printf("%s", mTask.desc);
	mTaskDesc->Clear();
    mTaskDesc->AppendText(mStr);
	mTaskDesc->Enable();
	mStr.clear();

	mStr.Printf("%u", mTask.level_min);
	mTaskMinLvl->Clear();
    mTaskMinLvl->AppendText(mStr);
	mTaskMinLvl->Enable();
	mStr.clear();

	mStr.Printf("%u", mTask.level_max);
	mTaskMaxLvl->Clear();
    mTaskMaxLvl->AppendText(mStr);
	mTaskMaxLvl->Enable();
	mStr.clear();

	mStr.Printf("%u", mTask.duration);
	mTaskDuration->Clear();
    mTaskDuration->AppendText(mStr);
	mTaskDuration->Enable();
	mStr.clear();

	mStr.Printf("%s", mTask.reward);
	mRewardName->Clear();
    mRewardName->AppendText(mStr);
	mRewardName->Enable();
	mStr.clear();

	mStr.Printf("%u", mTask.rewardid);
	mRewardID->Clear();
    mRewardID->AppendText(mStr);
	mRewardID->Enable();
	mStr.clear();

	mStr.Printf("%u", mTask.cashreward);
	mRewardCash->Clear();
    mRewardCash->AppendText(mStr);
	mRewardCash->Enable();
	mStr.clear();

	mStr.Printf("%i", mTask.xpreward);
	mRewardXP->Clear();
    mRewardXP->AppendText(mStr);
	mRewardXP->Enable();
	mStr.clear();

	mRewardMethod->Select(mTask.rewardmethod);
	mRewardMethod->Enable();
	ShowRewardChange(mTask.rewardmethod, mTask.rewardid);

	mStartZone->Enable();
	SetZoneSelectionById(mTask.startzone);

	mTaskRepeatable->Enable();
	mTaskRepeatable->SetValue(mTask.repeatable);

	ClearActivities();
	ActivitiesSelectionList->Enable();
	PopulateActivities();
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
	mActGoalID->Clear();
	mActGoalID->Disable();
	mActGoalCount->Clear();
	mActGoalCount->Disable();

	mActMethod->Select(2);
	mActMethod->Disable();

	ShowRewardItems->Enable();
	RefreshItems->Enable();
}

void MainFrame::ContextClick(wxContextMenuEvent& event)
{
	mErrorLog->Log(eqEmuLogBoth, "Context Menu Requested");
	if(event.GetPosition() == wxDefaultPosition)
	{
		mErrorLog->Log(eqEmuLogBoth, "Context Menu Triggered From Keyboard.");
	}
	else{
		wxPoint p = event.GetPosition();
		mErrorLog->Log(eqEmuLogBoth, "Context Menu Triggered At (%d,%d)", p.x, p.y);
		wxPoint stcp = ScreenToClient(p);
		mErrorLog->Log(eqEmuLogBoth, "Context Menu Triggered At (%d,%d)(Adjusted for Client)", stcp.x, stcp.y);

		//big box on the side: holds tasks
		if(stcp.x >= 7 && stcp.x <= 133)
		{
			if(stcp.y >=7 && stcp.y <= 258)
			{
				ContextMenuTaskList();
			}
		}

		//small box on the 2nd tab: must be on top to work
		if(stcp.x >= 146 && stcp.x <= 243)
		{
			if(stcp.y >=23 && stcp.y <= 529)
			{
				if(MainPanel2->IsShownOnScreen())
					ContextMenuActivityList();
				else if(MainPanel3->IsShownOnScreen())
					ContextMenuGoalList();
				else if(MainPanel4->IsShownOnScreen())
					ContextMenuProximity();
			}
		}

		if(stcp.x >= 297 && stcp.x <= 442)
		{
			if(stcp.y >= 74 && stcp.y <= 368)
			{
				if(MainPanel3->IsShownOnScreen() && GoalsValuesList->IsEnabled())
					ContextMenuGoalValueList();
			}
		}
	}
}

void MainFrame::About(wxCommandEvent& event)
{
    wxAboutDialogInfo info;
    info.SetName(_("Task Master"));
    info.SetVersion(_(TASK_MASTER_VERSION));
    info.SetDescription(_("Task Creation Tool"));
	info.SetWebSite("www.eqemulator.net");

    wxAboutBox(info);
}