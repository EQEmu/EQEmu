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
#include "AoT_Spell_Book.h"
#include "AoT_AutoLoot.h"
#include "AoT_Exp_Display.h"
#include "AoT_Stat_Display.h"

#define OP_WIRE_AoTMemorizeSpell 0x1969
#define OP_WIRE_AoTLoadSpellSet  0x196E

char *OurCaption = "Core is loading...";

// ── SpellBook vtable hook ─────────────────────────────────────────────────────
// Replaces CSpellBookWnd::Activate (slot 0x090) and Show (slot 0x0d8) so that:
//  - pressing B opens the AoT custom window instead of the native spellbook
//  - StartSpellMemorization's internal Show(1,1) call is suppressed
//  - spell cursor pickup flow doesn't accidentally toggle the AoT window closed

static PCSIDLWNDVFTABLE pSBWOldVftable = nullptr;
static PCSIDLWNDVFTABLE pSBWNewVftable = nullptr;

static void __fastcall SpellBook_Activate_Hook(void* thisptr, void* edx) {
    if (GetGameState() != GAMESTATE_INGAME) {
        typedef void (__fastcall* ActivateFn)(void*, void*);
        ((ActivateFn)pSBWOldVftable->ShowWindow)(thisptr, edx);
        return;
    }
    if (!pAoTSpellBook) {
        pAoTSpellBook = new AoT_Spell_Book();  // CCustomWnd ctor calls Show(1,1)
        return;
    }
    if (pAoTSpellBook->pXWnd()->IsReallyVisible()) {
        pAoTSpellBook->pXWnd()->Show(0, 1);    // already open — toggle closed
    } else {
        pAoTSpellBook->PopulateSpellList();
        pAoTSpellBook->pXWnd()->Show(1, 1);
    }
}

// Intercepts the Show(int, int) virtual (slot 0x0d8). StartSpellMemorization
// calls this to make the native spellbook visible during memorization; we
// suppress it so our AoT window stays in front and the native book never shows.
static void __fastcall SpellBook_Show_Hook(void* thisptr, void* edx,
                                           int bShow, int bModal) {
    if (GetGameState() != GAMESTATE_INGAME) {
        typedef void (__fastcall* ShowFn)(void*, void*, int, int);
        ((ShowFn)pSBWOldVftable->Show)(thisptr, edx, bShow, bModal);
        return;
    }
    // Suppress native spellbook visibility during gameplay so our AoT window
    // stays in front and the native book never shows.
}

static void InstallSpellBookHook() {
    if (!pSpellBookWnd || pSBWNewVftable) return;
    PCSIDLWND pWnd = (PCSIDLWND)pSpellBookWnd;
    pSBWOldVftable = pWnd->pvfTable;
    pSBWNewVftable = new CSIDLWNDVFTABLE;
    *pSBWNewVftable = *pSBWOldVftable;
    pSBWNewVftable->ShowWindow = (LPVOID)SpellBook_Activate_Hook;
    pSBWNewVftable->Show      = (LPVOID)SpellBook_Show_Hook;
    pWnd->pvfTable = pSBWNewVftable;
}

static void RemoveSpellBookHook() {
    if (!pSBWNewVftable) return;
    if (pSpellBookWnd)
        ((PCSIDLWND)pSpellBookWnd)->pvfTable = pSBWOldVftable;
    delete pSBWNewVftable;
    pSBWNewVftable = nullptr;
    pSBWOldVftable = nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────

// ── CCastSpellWnd gem-click intercept ─────────────────────────────────────────
// When the player is in "waiting for gem" state (initiated from the AoT spell
// book Memorize button), we temporarily hook CCastSpellWnd's WndNotification to
// catch left-clicks on spell gem children (CSPW_Spell0..7).  On a gem click,
// we call CSpellBookWnd::MemorizeSet directly and restore the original vtable.

static PCSIDLWNDVFTABLE pCSWOldVftable = nullptr;
static PCSIDLWNDVFTABLE pCSWNewVftable = nullptr;
static CXWnd* g_aotGemWindows[8] = {};

int  g_aotWaitingGemBookSlot = -1;
DWORD g_aotWaitingSpellId    = 0xFFFFFFFF;

// ── Memorization progress gauge state ────────────────────────────────────────
// Duration must match RuleI(AoT, MemorizeDelayMs) on the server.
static DWORD  g_aotMemorizeStartTick = 0;
static DWORD  g_aotMemorizeDuration  = 0;
static DWORD  g_aotGaugeLastUpdate   = 0;
static char   g_aotMemorizeSpellName[64] = {};

static void AoT_CancelGemWait();   // forward decl

static int __fastcall CastSpellWnd_WndNotif_Hook(
    void* thisptr, void* /*edx*/,
    CXWnd* pWnd, unsigned int msg, void* data)
{
    if (g_aotWaitingGemBookSlot >= 0 && msg == XWM_LCLICK) {
        for (int i = 0; i < 8; i++) {
            if (g_aotGemWindows[i] && g_aotGemWindows[i] == pWnd) {
                DWORD spellId = g_aotWaitingSpellId;
                AoT_CancelGemWait();
                if (pAoTSpellBook) pAoTSpellBook->RevertToNormal();

                if (spellId != 0xFFFFFFFF) {
                    // Sit if not already sitting. /sit is a toggle in Titanium — calling it
                    // while seated would immediately stand the player, breaking a second
                    // back-to-back memorization.
                    if (pEverQuest && pLocalPlayer &&
                        ((PSPAWNINFO)pLocalPlayer)->StandState != STANDSTATE_SIT)
                        pEverQuest->InterpretCmd((EQPlayer*)pLocalPlayer, const_cast<char*>("/sit"));

                    // Start client-side progress gauge.
                    PSPELL pSp = GetSpellByID((int)spellId);
                    strncpy_s(g_aotMemorizeSpellName, sizeof(g_aotMemorizeSpellName),
                              pSp ? pSp->Name : "Unknown Spell", _TRUNCATE);
                    g_aotMemorizeStartTick = GetTickCount();
                    g_aotMemorizeDuration  = 3000; // must match RuleI(AoT, MemorizeDelayMs)
                    g_aotGaugeLastUpdate   = 0;

                    struct { uint32_t spell_id; uint8_t gem_slot; } pkt = { (uint32_t)spellId, (uint8_t)i };
                    AoT_SendRawPacket(OP_WIRE_AoTMemorizeSpell, &pkt, sizeof(pkt));
                }

                return 1;
            }
        }
    }
    typedef int (__fastcall* FnWN)(void*, void*, CXWnd*, unsigned int, void*);
    return ((FnWN)pCSWOldVftable->WndNotification)(thisptr, nullptr, pWnd, msg, data);
}

bool AoT_BeginGemWait(int bookSlot) {
    if (!pCastSpellWnd || pCSWNewVftable) return false;
    PCSIDLWND pWnd = (PCSIDLWND)pCastSpellWnd;

    memset(g_aotGemWindows, 0, sizeof(g_aotGemWindows));
    bool hasAny = false;
    for (int i = 0; i < 8; i++) {
        char name[16];
        sprintf_s(name, sizeof(name), "CSPW_Spell%d", i);
        g_aotGemWindows[i] = ((CSidlScreenWnd*)pWnd)->GetChildItem(name);
        if (g_aotGemWindows[i]) hasAny = true;
    }
    if (!hasAny) return false;

    g_aotWaitingGemBookSlot = bookSlot;
    {
        PCHARINFO2 ci2 = GetCharInfo2();
        g_aotWaitingSpellId = (ci2 && bookSlot >= 0 && bookSlot < NUM_BOOK_SLOTS)
                              ? ci2->SpellBook[bookSlot] : 0xFFFFFFFF;
    }
    pCSWOldVftable = pWnd->pvfTable;
    pCSWNewVftable = new CSIDLWNDVFTABLE;
    *pCSWNewVftable = *pCSWOldVftable;
    pCSWNewVftable->WndNotification = (LPVOID)CastSpellWnd_WndNotif_Hook;
    pWnd->pvfTable = pCSWNewVftable;
    return true;
}

static void AoT_CancelGemWait() {
    g_aotWaitingGemBookSlot = -1;
    g_aotMemorizeStartTick  = 0;  // stop gauge pulse
    if (!pCSWNewVftable) return;
    if (pCastSpellWnd)
        ((PCSIDLWND)pCastSpellWnd)->pvfTable = pCSWOldVftable;
    delete pCSWNewVftable;
    pCSWNewVftable = nullptr;
    pCSWOldVftable = nullptr;
    memset(g_aotGemWindows, 0, sizeof(g_aotGemWindows));
}

// Public wrapper for AoT_Spell_Book.cpp to call
void AoT_CancelGemWaitPublic() { AoT_CancelGemWait(); }

// Called from Heartbeat() every frame; throttled internally to ~100 ms updates.
void AoT_MemorizeGaugePulse() {
    if (!pAoTSpellBook || g_aotMemorizeStartTick == 0 || g_aotMemorizeDuration == 0)
        return;
    DWORD now = GetTickCount();
    if (now - g_aotGaugeLastUpdate < 100) return;
    g_aotGaugeLastUpdate = now;

    DWORD elapsed = now - g_aotMemorizeStartTick;
    if (elapsed >= g_aotMemorizeDuration) {
        g_aotMemorizeStartTick = 0;
        pAoTSpellBook->ClearDetailBox();
        return;
    }
    float progress = (float)elapsed / (float)g_aotMemorizeDuration;
    pAoTSpellBook->ShowMemorizeProgress(g_aotMemorizeSpellName, progress);
}

// ─────────────────────────────────────────────────────────────────────────────

class CDisplayHook
{
public:
    VOID CleanUI_Trampoline(VOID);
    VOID CleanUI_Detour(VOID)
    {
        AoT_CancelGemWait();
        RemoveSpellBookHook();
        if (pAoTSpellBook) {
            delete pAoTSpellBook;
            pAoTSpellBook = nullptr;
        }
        AoT_ExpDisplay_Cleanup();
        AoT_StatDisplay_Cleanup();
        DebugTry(CleanUI_Trampoline());
    }

    VOID ReloadUI_Trampoline(BOOL);
    VOID ReloadUI_Detour(BOOL UseINI)
    {
        DebugTry(ReloadUI_Trampoline(UseINI));
        InstallSpellBookHook();
    }

    /* This function is still in the client; however, it was phased out as of 
    the Omens of War Expansion

    bool GetWorldFilePath_Trampoline(char *, char *);
    bool GetWorldFilePath_Detour(char *Filename, char *FullPath)
    {
        if (!stricmp(FullPath,"bmpwad8.s3d"))
        {
            sprintf(Filename,"%s\\bmpwad8.s3d",gszINIPath);
            if (_access(Filename,0)!=-1)
            {
                return 1;
            }
        }

        bool Ret=GetWorldFilePath_Trampoline(Filename,FullPath);
        return Ret;
    }
    */
};

// ── CSpellBookWnd::MemorizeSet intercept ──────────────────────────────────────
// Native MemorizeSet fails because our Show hook hides the spellbook, making
// CanStartMemming() always return false.  We intercept here and forward the
// desired spell IDs to the server as OP_LoadSpellSet so the AoT queue system
// handles the sequential memorization instead.
class CSpellBookWndHook {
public:
    void MemorizeSet_Trampoline(int* spells, int count);
    void MemorizeSet_Detour(int* spells, int count) {
        if (GetGameState() != GAMESTATE_INGAME) {
            MemorizeSet_Trampoline(spells, count);
            return;
        }
        // Auto-sit if standing — same pattern as gem-click hook.
        // InterpretCmd is synchronous so the sit packet reaches the server
        // before OP_LoadSpellSet, satisfying the server's IsSitting() check.
        if (pEverQuest && pLocalPlayer &&
            ((PSPAWNINFO)pLocalPlayer)->StandState != STANDSTATE_SIT)
            pEverQuest->InterpretCmd((EQPlayer*)pLocalPlayer, const_cast<char*>("/sit"));
        // AoTLoadSpellSet_Struct: uint32 spell[9], 0xFFFFFFFF = leave slot alone
        struct { uint32_t spell[9]; } buf;
        int numValid = 0;
        for (int i = 0; i < 9; i++) {
            buf.spell[i] = (i < count && spells[i] > 0)
                           ? (uint32_t)spells[i] : 0xFFFFFFFFu;
            if (buf.spell[i] != 0xFFFFFFFFu) numValid++;
        }

        // Count gems that actually differ from current client state for gauge duration
        int actualChanges = numValid;
        PCHARINFO2 ci2 = GetCharInfo2();
        if (ci2) {
            actualChanges = 0;
            for (int i = 0; i < 9; i++) {
                if (buf.spell[i] == 0xFFFFFFFFu) continue;
                if (ci2->MemorizedSpells[i] == buf.spell[i]) continue;
                actualChanges++;
            }
        }

        AoT_SendRawPacket(OP_WIRE_AoTLoadSpellSet, &buf, sizeof(buf));

        if (numValid > 0) {
            // Ensure window is open — ctor shows and populates automatically
            if (!pAoTSpellBook)
                pAoTSpellBook = new AoT_Spell_Book();
            else if (!pAoTSpellBook->pXWnd()->IsReallyVisible()) {
                pAoTSpellBook->PopulateSpellList();
                pAoTSpellBook->pXWnd()->Show(1, 1);
            }
            if (actualChanges > 0) {
                strncpy_s(g_aotMemorizeSpellName, sizeof(g_aotMemorizeSpellName),
                          "Spell Set", _TRUNCATE);
                g_aotMemorizeStartTick = GetTickCount();
                g_aotMemorizeDuration  = (DWORD)(actualChanges * 3000);
                g_aotGaugeLastUpdate   = 0;
            }
        }
    }
};
// ─────────────────────────────────────────────────────────────────────────────

#ifndef ISXEQ

DWORD __cdecl DrawHUD_Trampoline(DWORD,DWORD,DWORD,DWORD); 
DWORD __cdecl DrawHUD_Detour(DWORD a,DWORD b,DWORD c,DWORD d) 
{ 
    DrawHUDParams[0]=a;
    DrawHUDParams[1]=b;
    DrawHUDParams[2]=c;
    DrawHUDParams[3]=d;
    if (gbHUDUnderUI || gbAlwaysDrawMQHUD)
        return 0;
    int Ret= DrawHUD_Trampoline(a,b,c,d);
    //PluginsDrawHUD();
    if (HMODULE hmEQPlayNice=GetModuleHandle("EQPlayNice.dll"))
    {
        if (fMQPulse pEQPlayNicePulse=(fMQPulse)GetProcAddress(hmEQPlayNice,"Compat_DrawIndicator"))
            pEQPlayNicePulse();
    }
    return Ret;
} 

void DrawHUD()
{
    if (gbAlwaysDrawMQHUD || (gGameState==GAMESTATE_INGAME && gbHUDUnderUI && gbShowNetStatus))
    {
        if (DrawHUDParams[0] && gGameState==GAMESTATE_INGAME && gbShowNetStatus)
        {
            DrawHUD_Trampoline(DrawHUDParams[0],DrawHUDParams[1],DrawHUDParams[2],DrawHUDParams[3]);
            DrawHUDParams[0]=0;
        }
        if (HMODULE hmEQPlayNice=GetModuleHandle("EQPlayNice.dll"))
        {
            if (fMQPulse pEQPlayNicePulse=(fMQPulse)GetProcAddress(hmEQPlayNice,"Compat_DrawIndicator"))
                pEQPlayNicePulse();
        }

    }
    else
        DrawHUDParams[0]=0;
}

VOID DrawHUDText(PCHAR Text, DWORD X, DWORD Y, DWORD Argb, DWORD Size)
{

    DWORD sX=((PCXWNDMGR)pWndMgr)->ScreenExtentX;
    DWORD sY=((PCXWNDMGR)pWndMgr)->ScreenExtentY;

    CTextureFont* pFont=0;
    DWORD* ppDWord=(DWORD*)((PCXWNDMGR)pWndMgr)->font_list_ptr;
    if (ppDWord[1]<=2)
    {
        pFont=(CTextureFont*)ppDWord[0];
    }
    else
    {
        pFont=(CTextureFont*)ppDWord[2];
    }
    if(Size!=2 && Size<12)
        pFont->Size=Size;
    pFont->DrawWrappedText(&CXStr((char*)Text),X,Y,sX-X,&CXRect(X,Y,sX,sY),Argb,1,0);
    pFont->Size=2; // reset back to 2 or it screws up other HUD sizes
}
#endif

class EQ_LoadingSHook
{
public:

    VOID SetProgressBar_Trampoline(int,char const *);
    VOID SetProgressBar_Detour(int A,char const *B)
    {
            SetProgressBar_Trampoline(A,B);
    }
};

//DETOUR_TRAMPOLINE_EMPTY(bool CDisplayHook::GetWorldFilePath_Trampoline(char *, char *)); 
DETOUR_TRAMPOLINE_EMPTY(VOID EQ_LoadingSHook::SetProgressBar_Trampoline(int, char const *));
DETOUR_TRAMPOLINE_EMPTY(DWORD DrawHUD_Trampoline(DWORD,DWORD,DWORD,DWORD));
DETOUR_TRAMPOLINE_EMPTY(VOID CDisplayHook::CleanUI_Trampoline(VOID));
DETOUR_TRAMPOLINE_EMPTY(VOID CDisplayHook::ReloadUI_Trampoline(BOOL));
DETOUR_TRAMPOLINE_EMPTY(void CSpellBookWndHook::MemorizeSet_Trampoline(int*, int));

VOID InitializeDisplayHook()
{
    DebugSpew("Initializing Display Hooks");

    EzDetour(CDisplay__CleanGameUI,&CDisplayHook::CleanUI_Detour,&CDisplayHook::CleanUI_Trampoline);
    EzDetour(CDisplay__ReloadUI,&CDisplayHook::ReloadUI_Detour,&CDisplayHook::ReloadUI_Trampoline);
    //EzDetour(CDisplay__GetWorldFilePath,&CDisplayHook::GetWorldFilePath_Detour,&CDisplayHook::GetWorldFilePath_Trampoline);
#ifndef ISXEQ
   // EzDetour(DrawNetStatus,DrawHUD_Detour,DrawHUD_Trampoline);
#endif
    EzDetour(EQ_LoadingS__SetProgressBar,&EQ_LoadingSHook::SetProgressBar_Detour,&EQ_LoadingSHook::SetProgressBar_Trampoline);
    EzDetour(CSpellBookWnd__MemorizeSet,&CSpellBookWndHook::MemorizeSet_Detour,&CSpellBookWndHook::MemorizeSet_Trampoline);
}

VOID ShutdownDisplayHook()
{
    DebugSpew("Shutting down Display Hooks");

    RemoveDetour(CDisplay__CleanGameUI);
    RemoveDetour(CDisplay__ReloadUI);
#ifndef ISXEQ
    RemoveDetour(DrawNetStatus);
#endif
    RemoveDetour(EQ_LoadingS__SetProgressBar);
    RemoveDetour(CSpellBookWnd__MemorizeSet);
    //RemoveDetour(CDisplay__GetWorldFilePath);
}
