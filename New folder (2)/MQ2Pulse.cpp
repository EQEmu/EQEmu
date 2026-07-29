/*****************************************************************************
MQ2Main.dll: MacroQuest2's extension DLL for EverQuest
Copyright (C) 2002-2003 Plazmic, 2003-2005 Lax

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
******************************************************************************/
#if !defined(CINTERFACE)
//#error /DCINTERFACE
#endif

#define DBG_SPEW


//#define DEBUG_TRY 1
#include "MQ2Main.h"
#include "AoT_AutoLoot.h"
#include "AoT_Exp_Display.h"
#include "AoT_Stat_Display.h"
BOOL TurnNotDone=FALSE;

#ifndef ISXEQ
BOOL DoNextCommand()
{
    if (!ppCharSpawn || !pCharSpawn) return FALSE;
    PSPAWNINFO pCharOrMount = NULL;
    PCHARINFO pCharInfo = GetCharInfo();
    PSPAWNINFO pChar = pCharOrMount = (PSPAWNINFO)pCharSpawn;
    if (pCharInfo && pCharInfo->pSpawn) pChar=pCharInfo->pSpawn;
    if ((!pChar) || (gZoning)/* || (gDelayZoning)*/) return FALSE;
    if (((gFaceAngle != 10000.0f) || (gLookAngle != 10000.0f)) && (TurnNotDone)) return FALSE;
    if (IsMouseWaiting()) return FALSE;
    if (gDelay && gDelayCondition[0])
    {
        CHAR szCond[MAX_STRING];
        strcpy(szCond,gDelayCondition);
        ParseMacroParameter(GetCharInfo()->pSpawn,szCond);
        DOUBLE Result;
        if (!Calculate(szCond,Result))
        {
            FatalError("Failed to parse /delay condition '%s', non-numeric encountered",szCond);
            return false;
        }
        if (Result!=0)
        {
            DebugSpewNoFile("/delay ending early, conditions met");
            gDelay=0;
        }
    }
    if (!gDelay && !gMacroPause && (!gMQPauseOnChat || *EQADDR_NOTINCHATMODE) &&
        gMacroBlock && gMacroStack) {
            PMACROBLOCK tmpBlock = gMacroBlock;
            gMacroStack->Location=gMacroBlock;
#ifdef MQ2_PROFILING
            LARGE_INTEGER BeforeCommand;
            QueryPerformanceCounter(&BeforeCommand);
            PMACROBLOCK ThisMacroBlock = gMacroBlock; 
#endif
            gMacroBlock->MacroCmd = 0;
            DoCommand(pChar,gMacroBlock->Line);
            if (gMacroBlock) {
#ifdef MQ2_PROFILING
                LARGE_INTEGER AfterCommand;
                QueryPerformanceCounter(&AfterCommand);
                ThisMacroBlock->ExecutionCount++;
                ThisMacroBlock->ExecutionTime += AfterCommand.QuadPart - BeforeCommand.QuadPart; 
#endif
                if (!gMacroBlock->pNext) {
                    FatalError("Reached end of macro.");
                } else {
                    // if the macro block changed and there was a /macro 
                    // command don't bump the line 
                    //if (gMacroBlock == tmpBlock || !gMacroBlock->MacroCmd) {
                    gMacroBlock = gMacroBlock->pNext;
                    //}
                }
            }
            return TRUE;
    }
    return FALSE;
}
#endif

void Pulse()
{
    if (!ppCharSpawn || !pCharSpawn) return;
    PSPAWNINFO pCharOrMount = NULL;
    PCHARINFO pCharInfo = GetCharInfo();
    PSPAWNINFO pChar = pCharOrMount = (PSPAWNINFO)pCharSpawn;
    if (pCharInfo && pCharInfo->pSpawn) pChar=pCharInfo->pSpawn;

    static WORD LastZone=-1;

    static PSPAWNINFO pCharOld = NULL;
    static FLOAT LastX = 0.0f;
    static FLOAT LastY = 0.0f;
    static DWORD LastMoveTick = 0;
    static DWORD MapDelay = 0;

    static DWORD LastHealth = 0;
    static DWORD LastMana = 0;
    static DWORD LastEndurance = 0;



    // Drop out here if we're waiting for something.
    if (!pChar || gZoning /* || gDelayZoning*/) return;
    if (!pCharInfo) {
        //DebugSpew("Pulse: no charinfo returning\n");
        return;
    }

    if ((unsigned int)GetCharInfo()->charinfo_info & 0x80000000) return;

    if (pChar!=pCharOld && WereWeZoning)
    {
        WereWeZoning = FALSE;
        pCharOld=pChar;
        gFaceAngle = 10000.0f;
        gLookAngle = 10000.0f;
        gbMoving = FALSE;
        LastX = pChar->X;
        LastY = pChar->Y;
        LastMoveTick=GetTickCount();
        EnviroTarget.Name[0]=0;
        pGroundTarget=0;
        DoorEnviroTarget.Name[0]=0;
        pDoorTarget=0;
        LastHealth=GetCurHPS();
        LastMana=GetCharInfo2()->Mana;
        LastEndurance=GetCharInfo2()->Endurance; 
        ManaGained=0;
        HealthGained=0;
        EnduranceGained=0;
        // see if we're on a pvp server
        if (!strncmp(EQADDR_SERVERNAME,"tallon",6) || !strncmp(EQADDR_SERVERNAME,"vallon",6))
        {
            PVPServer=PVP_TEAM;
        }
        else if (!strncmp(EQADDR_SERVERNAME,"sullon",6))
        {
            PVPServer=PVP_SULLON;
        }
        else if (!strncmp(EQADDR_SERVERNAME,"rallos",6))
        {
            PVPServer=PVP_RALLOS;
        }
        else
            PVPServer=PVP_NONE;

    } else if ((LastX!=pChar->X) || (LastY!=pChar->Y) || LastMoveTick>GetTickCount()-100) {
        if ((LastX!=pChar->X) || (LastY!=pChar->Y)) LastMoveTick=GetTickCount();
        gbMoving = TRUE;
        LastX = pChar->X;
        LastY = pChar->Y;
    } else {
        gbMoving = FALSE;
    }

    DWORD CurrentHealth=GetCurHPS();
    if (LastHealth && CurrentHealth>LastHealth)
    {
        if ((int)pChar->HPCurrent!=GetMaxHPS())
        { // gained health, and not max
            HealthGained=CurrentHealth-LastHealth;
        }
    }
    LastHealth=CurrentHealth;

    if (LastMana && GetCharInfo2()->Mana > LastMana)
    {
        if ((int)GetCharInfo2()->Mana-LastMana > 0 )
        {
            ManaGained=GetCharInfo2()->Mana-LastMana;
        }
    }
    LastMana=GetCharInfo2()->Mana;

    if (LastEndurance && GetCharInfo2()->Endurance > LastEndurance) 
    { 
        if (GetCharInfo2()->Endurance != GetMaxEndurance()) 
        { 
            EnduranceGained = GetCharInfo2()->Endurance - LastEndurance; 
        } 
    } 
    LastEndurance = GetCharInfo2()->Endurance;

    if (gbDoAutoRun && pChar && pCharInfo) {
        gbDoAutoRun = FALSE;
        CHAR szServerAndName[MAX_STRING] = {0};
        CHAR szAutoRun[MAX_STRING] = {0};
        PCHAR pAutoRun = szAutoRun;
        sprintf(szServerAndName,"%s.%s",EQADDR_SERVERNAME,pCharInfo->Name);
        GetPrivateProfileString("AutoRun",szServerAndName,"",szAutoRun,MAX_STRING,gszINIFilename);
        while (pAutoRun[0]==' ' || pAutoRun[0]=='\t') pAutoRun++;
        if (szAutoRun[0]!=0) DoCommand(pChar,pAutoRun);
    }

    if ((gFaceAngle != 10000.0f) || (gLookAngle != 10000.0f)) {
        TurnNotDone = FALSE;
        if (gFaceAngle != 10000.0f) {
            if (abs((INT)(pCharOrMount->Heading - gFaceAngle)) < 10.0f) {
                pCharOrMount->Heading = (FLOAT)gFaceAngle;
                pCharOrMount->SpeedHeading = 0.0f;
                gFaceAngle = 10000.0f;
            } else {
                TurnNotDone = TRUE;
                DOUBLE c1 = pCharOrMount->Heading + 256.0f;
                DOUBLE c2 = gFaceAngle;
                if (c2<pChar->Heading) c2 += 512.0f;
                DOUBLE turn = (DOUBLE)(rand()%200)/10;
                if (c2<c1) {
                    pCharOrMount->Heading += (FLOAT)turn;
                    pCharOrMount->SpeedHeading = 12.0f;
                    if (pCharOrMount->Heading>=512.0f) pCharOrMount->Heading-=512.0f;
                } else {
                    pCharOrMount->Heading -= (FLOAT)turn;
                    pCharOrMount->SpeedHeading = -12.0f;
                    if (pCharOrMount->Heading<0.0f) pCharOrMount->Heading+=512.0f;
                }
            }
        }

        if (gLookAngle != 10000.0f) {
            if (abs((INT)(pChar->CameraAngle - gLookAngle)) < 5.0f) {
                pChar->CameraAngle = (FLOAT)gLookAngle;
                gLookAngle = 10000.0f;
                TurnNotDone = FALSE;
            } else {
                TurnNotDone = TRUE;
                FLOAT c1 = pChar->CameraAngle;
                FLOAT c2 = (FLOAT)gLookAngle;

                DOUBLE turn = (DOUBLE)(rand()%200)/20;
                if (c1<c2) {
                    pChar->CameraAngle += (FLOAT)turn;
                    if (pChar->CameraAngle>=128.0f) pChar->CameraAngle -= 128.0f;
                } else {
                    pChar->CameraAngle -= (FLOAT)turn;
                    if (pChar->CameraAngle<=-128.0f) pChar->CameraAngle += 128.0f;
                }
            }
        }

        if (TurnNotDone) {
            bRunNextCommand=FALSE;
            IsMouseWaiting();
            return;
        }
    }
}


void Heartbeat()
{
    int GameState=GetGameState();
    if (GameState!=-1)
    {
        if ((DWORD)GameState!=gGameState)
        {
            SetMapGameState(GameState);
            gGameState=GameState;
        }
    }
    UpdateMQ2SpawnSort();
    AutoLootPulse();
    AoT_MemorizeGaugePulse();
    AoT_ExpDisplay_Pulse();
    AoT_StatDisplay_Pulse();
}

#ifndef ISXEQ_LEGACY
// *************************************************************************** 
// Function:    ProcessGameEvents 
// Description: Our ProcessGameEvents Hook
// *************************************************************************** 
BOOL Trampoline_ProcessGameEvents(VOID); 
BOOL Detour_ProcessGameEvents(VOID) 
{ 
    

    Heartbeat();
#ifdef ISXEQ
    if (!pISInterface->ScriptEngineActive()) 
        pISInterface->LavishScriptPulse();
#endif
    return Trampoline_ProcessGameEvents();
}

DETOUR_TRAMPOLINE_EMPTY(BOOL Trampoline_ProcessGameEvents(VOID)); 
class CEverQuestHook {
public:
    VOID EnterZone_Trampoline(PVOID pVoid);
    VOID EnterZone_Detour(PVOID pVoid)
    {
        EnterZone_Trampoline(pVoid);
        gZoning = TRUE;
        WereWeZoning = TRUE;
    }

    VOID SetGameState_Trampoline(DWORD GameState);
    VOID SetGameState_Detour(DWORD GameState)
    {
        SetMapGameState(GameState);
        SetGameState_Trampoline(GameState);
    }
};

DETOUR_TRAMPOLINE_EMPTY(VOID CEverQuestHook::EnterZone_Trampoline(PVOID));
DETOUR_TRAMPOLINE_EMPTY(VOID CEverQuestHook::SetGameState_Trampoline(DWORD));

void InitializeMQ2Pulse()
{

    EzDetour(ProcessGameEvents,Detour_ProcessGameEvents,Trampoline_ProcessGameEvents);
    EzDetour(CEverQuest__EnterZone,&CEverQuestHook::EnterZone_Detour,&CEverQuestHook::EnterZone_Trampoline);
    EzDetour(CEverQuest__SetGameState,&CEverQuestHook::SetGameState_Detour,&CEverQuestHook::SetGameState_Trampoline);
}

void ShutdownMQ2Pulse()
{
    RemoveDetour((DWORD)ProcessGameEvents);
    RemoveDetour(CEverQuest__EnterZone);
    RemoveDetour(CEverQuest__SetGameState);
}
#endif
