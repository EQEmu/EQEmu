// MQ2Map.cpp : Defines the entry point for the DLL application.
//

// PLUGIN_API is only to be used for callbacks.  All existing callbacks at this time
// are shown below. Remove the ones your plugin does not use.  Always use Initialize
// and Shutdown for setup and cleanup, do NOT do it in DllMain.

#include "MQ2Map.h"


unsigned long bmMapRefresh=0;

DWORD HighlightColor=0xFF700070;

CHAR MapSpecialClickString[16][MAX_STRING]=
{
    "",// unused, will always target
    "",//SHIFT 
    "/maphide id %i",//CTRL
    "",//CTRL|SHIFT
    "/highlight id %i",//LALT 
    "",//LALT|SHIFT
    "",//LALT|CTRL
    "",//LALT|SHIFT|CTRL
    "",//RALT
    "",//RALT|SHIFT
    "",//RALT|CTRL
    "",//RALT|SHIFT|CTRL
    "",//RALT|LALT
    "",//RALT|LALT|SHIFT
    "",//RALT|LALT|CTRL
    "" //RALT|LALT|SHIFT|CTRL
};

CHAR MapNameString[MAX_STRING]={"%N"};
CHAR MapTargetNameString[MAX_STRING]={"%N"};
SEARCHSPAWN MapFilterCustom = {0};
MAPFILTER MapFilterOptions[] = {
    {"All",          FALSE,-1,          TRUE,MAPFILTER_Invalid,TRUE,  "Enables/disables map functions", 1},
    {"PC",           TRUE,0xFF00FF,   TRUE,MAPFILTER_All,TRUE,      "Displays PCs", 1},
    {"PCConColor",   FALSE,-1,         TRUE,MAPFILTER_PC,FALSE,      "Displays PCs in consider colors", 1},
    {"Group",        FALSE,0x0080C0,   TRUE,MAPFILTER_PC,FALSE,      "Displays group members in a specific color"},
   {"Mount", TRUE, 0x707070, TRUE, MAPFILTER_All, TRUE, "Displays mounts"},
   {"NPC", TRUE, 0x404040, TRUE, MAPFILTER_All, TRUE, "Displays NPCs", 1},
   {"NPCConColor", FALSE, -1, TRUE, MAPFILTER_NPC, FALSE, "Displays NPCs in consider colors", 1},
   {"Untargetable", TRUE, 0x404040, TRUE, MAPFILTER_All, TRUE, "Displays Untargetable NPCs"},
   {"Pet", TRUE, 0x707070, TRUE, MAPFILTER_All, TRUE, "Displays pets"},
   {"Corpse", FALSE, 0x00C000, TRUE, MAPFILTER_All, TRUE, "Displays corpses"},
   {"Chest", TRUE, 0xC08000, TRUE, MAPFILTER_All, TRUE, "Displays chestesses"},
   {"Trigger", TRUE, 0xC08000, TRUE, MAPFILTER_All, TRUE, "Displays hidden triggers"},
   {"Trap", TRUE, 0xC08000, TRUE, MAPFILTER_All, TRUE, "Displays hidden traps"},
   {"Timer", TRUE, 0xC08000, TRUE, MAPFILTER_All, TRUE, "Displays hidden timers"},
   {"Ground", TRUE, 0xC0C0C0, TRUE, MAPFILTER_All, TRUE, "Displays ground items"},
   {"Target", TRUE, 0xC00000, TRUE, MAPFILTER_All, FALSE, "Displays your target"},
   {"TargetLine", TRUE, 0x808080, TRUE, MAPFILTER_Target, FALSE,
    "Displays a line to your target"},
   {"TargetRadius", TRUE, 0x808080, FALSE, MAPFILTER_Target,
    FALSE, "Sets radius of a circle around your target to # (omit or set to 0 to disable)"},
   {"TargetMelee", TRUE, 0xFF8080, FALSE, MAPFILTER_Target, FALSE,
    "Draws a melee-range circle around your target"},
   {"Vector", TRUE, -1, TRUE, MAPFILTER_All, TRUE, "Displays heading vectors"},
   {"Custom", TRUE, -1, FALSE, MAPFILTER_All, TRUE, "Sets custom filter (omit to disable)"},
   {"CastRadius", TRUE, 0x808080, FALSE, MAPFILTER_All, FALSE,
    "Sets radius of casting circle to # (omit or set to 0 to disable)"},
    {"NormalLabels", TRUE,-1,             TRUE,MAPFILTER_Invalid,TRUE, "Toggles non-MQ2 label display"},
   {"Menu", TRUE, -1, TRUE, MAPFILTER_Invalid, FALSE,
    "Allows display of right-click context menu"},
   {"SpellRadius", TRUE, 0x00C000, FALSE, MAPFILTER_All, FALSE,
    "Sets radius of 2nd casting circle to # (omit or set to 0 to disable)"},
   {"Aura", TRUE, 0x404040, TRUE, MAPFILTER_All, TRUE, "Displays Auras"},
   {"Object", TRUE, 0x404040, TRUE, MAPFILTER_All, TRUE, "Displays inanimate objects"},
   {"Banner", TRUE, 0x404040, TRUE, MAPFILTER_All, TRUE, "Displays banners"},
   {"Campfire", TRUE, 0x404040, TRUE, MAPFILTER_All, TRUE, "Displays campfires"},
   {"PCCorpse", FALSE, 0x00C000, TRUE, MAPFILTER_All, TRUE,
    "Displays PC corpses, when corpse setting is on", 1},
   {"NPCCorpse", TRUE, 0x00C000, TRUE, MAPFILTER_All, TRUE,
    "Displays NPC corpses, when corpse setting is on"},
   {"Mercenary", TRUE, 0x404040, TRUE, MAPFILTER_All, TRUE, "Displays mercenaries"},
   {"Named", TRUE, 0x404040, TRUE, MAPFILTER_All, TRUE, "Displays named NPCs"},
   {"TargetPath", TRUE, -1, TRUE, MAPFILTER_Target, FALSE,
    "Draws EQ Path to selected target"},
    {NULL,           FALSE,-1,         FALSE,MAPFILTER_Invalid,FALSE,  NULL}
};


PCSIDLWNDVFTABLE CMyMapViewWnd__OldvfTable=0;
PCSIDLWNDVFTABLE MapViewMap_OldvfTable=0;
DWORD CMyMapViewWnd__OldDestructor=0;
DWORD CMyMapViewWnd__OldHandleRButtonDown=0;
DWORD CMyMapViewWnd__OldPostDraw=0;
DWORD MapViewMap__OldHandleRButtonDown=0;

DWORD __declspec(naked) CMyMapViewWnd__Destructor(const BOOL Deallocate)
{
    __asm {   
        push ecx;
		push edx;//maybe a compiler issue, but if we dont push edx as well here, we will crash on /loadskin... -eqmule Oct 23 2015
		push eax;//it doesnt really matter what the reason is, because it wont hurt pushing it in older compilers
	}			 //cause we pop it anyway at the bottom of this func...
				 //the important thing is that esp is the same both on entering this func and on exit...
				 //(and it is now)

    if (CMyMapViewWnd__OldvfTable && MapViewMap_OldvfTable) { 
        // make our own little stack frame here
        // operator delete assumes that it is there
        // it uses (unnecessarily) ebp-4
        __asm {
            push    ebp
                push    eax
                push    eax
                mov            ebp, esp
        }
		delete (PVOID)(*ppMapViewWnd)->pvfTable;
		(*ppMapViewWnd)->pvfTable = CMyMapViewWnd__OldvfTable;
        CMyMapViewWnd__OldvfTable = NULL;
		delete ((PEQMAPWINDOW)(*ppMapViewWnd))->pMapViewMapVfTable;
		((PEQMAPWINDOW)(*ppMapViewWnd))->pMapViewMapVfTable = MapViewMap_OldvfTable;
        MapViewMap_OldvfTable = NULL;
        __asm {
            pop     eax
                pop     eax
                pop     ebp
        }
    }

    __asm {
        pop eax;
		pop edx;
        pop ecx;
        jmp [CMyMapViewWnd__OldDestructor];
    }
}

bool RButtonDown()
{
    if (pCurrentMapLabel)
    {
        return MapSelectTarget();
    }
    if (!IsOptionEnabled(MAPFILTER_ContextMenu))
        return false;
    return true;
}
//int MapViewMap::HandleRButtonDown(const CXPoint& point, UINT Flags) - should probably just change to this
int __declspec(naked) MapViewMap__HandleRButtonDown(CXPoint& point, unsigned __int32 flags)
{
    __asm {   
        push ecx;
        push eax;
    }
    if (RButtonDown())
    {
        __asm {
            pop eax;
            pop ecx;
            jmp [MapViewMap__OldHandleRButtonDown];
        };
    }
    else
    {
        __asm {
            pop eax;
            pop ecx;
            xor eax, eax;
            retn 8;
        }
    }
} 


VOID __declspec(naked) CMyMapViewWnd__PostDraw()
{
    __asm {

        push esi;
        mov esi, ecx;

        call [MapUpdate];
        call [MapAttach];

        mov ecx, esi;
        call [CMyMapViewWnd__OldPostDraw];
        push eax;

        call [MapDetach];
        pop eax;
        mov ecx, esi;
        pop esi;
        ret;
    };
}

class CMyMapViewWnd
{
public:
    DWORD Constructor_Trampoline(class CXWnd *);
    DWORD Constructor_Detour(class CXWnd *wnd)
    {
		DWORD Ret = Constructor_Trampoline(wnd);
        CMapViewWnd *pWnd=(CMapViewWnd*)this;
		PEQMAPWINDOW mwnd = (PEQMAPWINDOW)pWnd;
        PCSIDLWNDVFTABLE pvfTable = new CSIDLWNDVFTABLE;
        PCSIDLWNDVFTABLE pMapViewMapVfTable = new CSIDLWNDVFTABLE;
        *pvfTable=*pWnd->pvfTable;
        *pMapViewMapVfTable=*((PEQMAPWINDOW)pWnd)->pMapViewMapVfTable;

        CMyMapViewWnd__OldvfTable=pWnd->pvfTable;
        pWnd->pvfTable=pvfTable;
		MapViewMap_OldvfTable = mwnd->pMapViewMapVfTable;
		mwnd->pMapViewMapVfTable = pMapViewMapVfTable;
		CMyMapViewWnd__OldPostDraw = (DWORD)mwnd->pMapViewMapVfTable->PostDraw2;
        CMyMapViewWnd__OldDestructor=(DWORD)pWnd->pvfTable->vector_deleting_destructor;
        pWnd->pvfTable->vector_deleting_destructor=CMyMapViewWnd__Destructor;
		mwnd->pMapViewMapVfTable->PostDraw2 = CMyMapViewWnd__PostDraw;
		MapViewMap__OldHandleRButtonDown = (DWORD)mwnd->pMapViewMapVfTable->HandleRButtonDown;
		mwnd->pMapViewMapVfTable->HandleRButtonDown = MapViewMap__HandleRButtonDown;
        return Ret;
    }

    static void StealVFTable()
    {
        if (CMapViewWnd *pWnd=(CMapViewWnd*)pMapViewWnd)
        {
            PCSIDLWNDVFTABLE pvfTable = new CSIDLWNDVFTABLE;
            PCSIDLWNDVFTABLE pMapViewMapVfTable = new CSIDLWNDVFTABLE;
            *pvfTable=*pWnd->pvfTable;
            *pMapViewMapVfTable=*((PEQMAPWINDOW)pWnd)->pMapViewMapVfTable;

            CMyMapViewWnd__OldvfTable=pWnd->pvfTable;
            pWnd->pvfTable=pvfTable;
            MapViewMap_OldvfTable=((PEQMAPWINDOW)pWnd)->pMapViewMapVfTable;
            ((PEQMAPWINDOW)pWnd)->pMapViewMapVfTable=pMapViewMapVfTable;
            CMyMapViewWnd__OldPostDraw=(DWORD)((PEQMAPWINDOW)pWnd)->pMapViewMapVfTable->PostDraw2;
            CMyMapViewWnd__OldDestructor=(DWORD)pWnd->pvfTable->vector_deleting_destructor;
            pWnd->pvfTable->vector_deleting_destructor=CMyMapViewWnd__Destructor;
            ((PEQMAPWINDOW)pWnd)->pMapViewMapVfTable->PostDraw2=CMyMapViewWnd__PostDraw;
            MapViewMap__OldHandleRButtonDown=(DWORD)((PEQMAPWINDOW)pWnd)->pMapViewMapVfTable->HandleRButtonDown;
            ((PEQMAPWINDOW)pWnd)->pMapViewMapVfTable->HandleRButtonDown=MapViewMap__HandleRButtonDown;
        }
    }

    static void RestoreVFTable()
    {
        if (CMapViewWnd *pWnd=(CMapViewWnd*)pMapViewWnd)
        {
            if (CMyMapViewWnd__OldvfTable && MapViewMap_OldvfTable) { 
                delete pWnd->pvfTable;
                pWnd->pvfTable=CMyMapViewWnd__OldvfTable;
                delete ((PEQMAPWINDOW)pWnd)->pMapViewMapVfTable;
                ((PEQMAPWINDOW)pWnd)->pMapViewMapVfTable = MapViewMap_OldvfTable;
            }
        }
    }
};

DETOUR_TRAMPOLINE_EMPTY(DWORD CMyMapViewWnd::Constructor_Trampoline(class CXWnd *)); 

bool Update=true;

#ifndef ISXEQ
// Called once, when the plugin is to initialize
EQLIB_API VOID InitializeMapPlugin(VOID)
{

    unsigned long i;
    CHAR szBuffer[MAX_STRING]={0};
    MapInit();

    // Do not use Custom, since the string isn't stored
    MapFilterOptions[MAPFILTER_Custom].Enabled = 0;
    MapFilterOptions[MAPFILTER_NPCConColor].Enabled = 1;
    MapFilterOptions[MAPFILTER_NPC].Enabled = 1;
    MapFilterOptions[MAPFILTER_PC].Enabled = 1;
    MapFilterOptions[MAPFILTER_PCConColor].Enabled = 1;
	MapFilterOptions[MAPFILTER_NormalLabels].Enabled = 1;
    EzDetour(CMapViewWnd__CMapViewWnd,&CMyMapViewWnd::Constructor_Detour,&CMyMapViewWnd::Constructor_Trampoline);
    CMyMapViewWnd::StealVFTable();
   // AddMQ2Data("MapSpawn",dataMapSpawn);
}

// Called once, when the plugin is to shutdown
PLUGIN_API VOID ShutdownMapPlugin(VOID)
{
    Update=false;
    RemoveDetour(CMapViewWnd__CMapViewWnd);

    MapClear();
    CMyMapViewWnd::RestoreVFTable();
}

// This is called each time a spawn is added to a zone (inserted into EQ's list of spawns),
// or for each existing spawn when a plugin first initializes
// NOTE: When you zone, these will come BEFORE OnZoned
PLUGIN_API VOID OnAddSpawn(PSPAWNINFO pNewSpawn)
{
	    // your toon's spawn id changes and it's no longer zero to start
	    // don't added it all 
		if (pNewSpawn) {
			if (PCHARINFO pMe = GetCharInfo()) {
				if (Update && pNewSpawn->SpawnID != 0 && pMe->pSpawn != pNewSpawn) {
					//DebugSpewAlways("MQ2Map::OnAddSpawn(%s) = %d", pNewSpawn->Name, pNewSpawn->SpawnID);
	        AddSpawn(pNewSpawn);
	    }
	}
	}
}

// This is called each time a spawn is removed from a zone (removed from EQ's list of spawns).
// It is NOT called for each existing spawn when a plugin shuts down.
PLUGIN_API VOID OnRemoveSpawn(PSPAWNINFO pSpawn)
{
	//DebugSpewAlways("MQ2Map::OnRemoveSpawn(%s) = %d", pSpawn->Name, pSpawn->SpawnID);
	if (Update && pSpawn)
        RemoveSpawn(pSpawn);
}

PLUGIN_API VOID SetMapGameState(DWORD GameState)
{
    if (GameState==3)
        MapClear();
}

#endif
