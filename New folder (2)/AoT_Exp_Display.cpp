#include "MQ2Main.h"
#include "AoT_Exp_Display.h"

static CXWnd* s_xpLabel   = nullptr;
static DWORD  s_lastUpdate = 0;

void AoT_ExpDisplay_Cleanup() {
    s_xpLabel   = nullptr;
    s_lastUpdate = 0;
}

void AoT_ExpDisplay_Pulse() {
    if (GetGameState() != GAMESTATE_INGAME) return;

    DWORD now = GetTickCount();
    if (now - s_lastUpdate < 200) return;
    s_lastUpdate = now;

    if (!s_xpLabel) {
        if (!ppInventoryWnd || !pInventoryWnd) return;
        s_xpLabel = ((CSidlScreenWnd*)pInventoryWnd)->GetChildItem("NextLevelLabel");
        if (!s_xpLabel) return;
    }

    PCHARINFO  pCI  = GetCharInfo();
    PCHARINFO2 pCI2 = GetCharInfo2();
    if (!pCI || !pCI2) return;

    int level = (int)pCI2->Level;
    if (level < 1) level = 1;

    // Per-level XP span at level L = 1000 * (L + 1), derived from server formula
    // GetEXPForLevel: finalxp = 500 * (L-1) * (L+2); span = finalxp(L+1) - finalxp(L)
    uint32_t span   = (uint32_t)(1000 * (level + 1));
    uint32_t xp_now = (uint32_t)(((ULONGLONG)pCI->Exp * span) / 330);

    char buf[48];
    sprintf_s(buf, sizeof(buf), "XP: %u / %u", xp_now, span);
    SetCXStr(&s_xpLabel->WindowText, buf);
}
