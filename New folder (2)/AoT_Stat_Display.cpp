#include "MQ2Main.h"
#include "AoT_Stat_Display.h"

static AoT_StatBlock_Struct_Client g_sb;
static bool  s_dirty      = false;
static int   s_init_phase = 0;  // 0=not started, 1-3=batches done, 4=all done
static DWORD s_lastUpdate = 0;

static CXWnd* s_AC           = nullptr;
static CXWnd* s_MinMit        = nullptr;
static CXWnd* s_Deflect       = nullptr;
static CXWnd* s_Avoidance     = nullptr;
static CXWnd* s_AvoidPct      = nullptr;
static CXWnd* s_Critable      = nullptr;
static CXWnd* s_Haste         = nullptr;
static CXWnd* s_CastSpeed     = nullptr;
static CXWnd* s_PriMaxHit     = nullptr;
static CXWnd* s_PriOffense    = nullptr;
static CXWnd* s_PriAccuracy   = nullptr;
static CXWnd* s_PriHitPct     = nullptr;
static CXWnd* s_PriCritPct    = nullptr;
static CXWnd* s_PriCritDmgPct = nullptr;
static CXWnd* s_PriDelay      = nullptr;
static CXWnd* s_PriDPS        = nullptr;
static CXWnd* s_SecMaxHit     = nullptr;
static CXWnd* s_SecOffense    = nullptr;
static CXWnd* s_SecAccuracy   = nullptr;
static CXWnd* s_SecHitPct     = nullptr;
static CXWnd* s_SecCritPct    = nullptr;
static CXWnd* s_SecCritDmgPct = nullptr;
static CXWnd* s_SecDelay      = nullptr;
static CXWnd* s_SecDPS        = nullptr;
static CXWnd* s_RngMaxHit     = nullptr;
static CXWnd* s_RngOffense    = nullptr;
static CXWnd* s_RngAccuracy   = nullptr;
static CXWnd* s_RngHitPct     = nullptr;
static CXWnd* s_RngCritPct    = nullptr;
static CXWnd* s_RngCritDmgPct = nullptr;
static CXWnd* s_RngDelay      = nullptr;
static CXWnd* s_RngDPS        = nullptr;
static CXWnd* s_SpellCritPct  = nullptr;
static CXWnd* s_SpellDC[6]    = {};
static CXWnd* s_SpellPot[6]   = {};
static CXWnd* s_HealCritPct   = nullptr;
static CXWnd* s_HealDirectPot = nullptr;
static CXWnd* s_HealHoTPot    = nullptr;
static CXWnd* s_HealRunePot   = nullptr;
static CXWnd* s_ThreatMod     = nullptr;

static void SetLabel(CXWnd* w, const char* text)
{
    if (w) SetCXStr(&w->WindowText, (PCHAR)text);
}

static void WriteWeapon(
    CXWnd* maxhit, CXWnd* offense, CXWnd* accuracy,
    CXWnd* hitpct, CXWnd* critpct, CXWnd* critdmg,
    CXWnd* delay,  CXWnd* dps,
    bool    valid,
    int32_t max_hit,    int32_t off,        int32_t acc,
    int32_t hit_x100,   int32_t crit_x1000, int32_t critdmg_x100,
    int32_t delay_x100, int32_t dps_x100)
{
    char buf[32];
    if (!valid) {
        SetLabel(maxhit,   "N/A");
        SetLabel(offense,  "N/A");
        SetLabel(accuracy, "N/A");
        SetLabel(hitpct,   "N/A");
        SetLabel(critpct,  "N/A");
        SetLabel(critdmg,  "N/A");
        SetLabel(delay,    "N/A");
        SetLabel(dps,      "N/A");
        return;
    }
    sprintf_s(buf, sizeof(buf), "%d",     max_hit);                  SetLabel(maxhit,   buf);
    sprintf_s(buf, sizeof(buf), "%d",     off);                      SetLabel(offense,  buf);
    sprintf_s(buf, sizeof(buf), "%d",     acc);                      SetLabel(accuracy, buf);
    sprintf_s(buf, sizeof(buf), "%d%%",   hit_x100 / 100);           SetLabel(hitpct,   buf);
    sprintf_s(buf, sizeof(buf), "%.1f%%", crit_x1000 / 1000.0);     SetLabel(critpct,  buf);
    sprintf_s(buf, sizeof(buf), "%d%%",   critdmg_x100 / 100);       SetLabel(critdmg,  buf);
    sprintf_s(buf, sizeof(buf), "%.2fs",  delay_x100 / 100.0);       SetLabel(delay,    buf);
    sprintf_s(buf, sizeof(buf), "%.2f",   dps_x100 / 100.0);         SetLabel(dps,      buf);
}

void AoT_StatDisplay_Receive(const char* buf, size_t len)
{
    if (!buf || len < sizeof(AoT_StatBlock_Struct_Client)) return;
    memcpy(&g_sb, buf, sizeof(AoT_StatBlock_Struct_Client));
    s_dirty = true;
}

void AoT_StatDisplay_Cleanup()
{
    s_AC = s_MinMit = s_Deflect = s_Avoidance = s_AvoidPct = s_Critable = nullptr;
    s_Haste = s_CastSpeed = nullptr;
    s_PriMaxHit = s_PriOffense = s_PriAccuracy = nullptr;
    s_PriHitPct = s_PriCritPct = s_PriCritDmgPct = nullptr;
    s_PriDelay = s_PriDPS = nullptr;
    s_SecMaxHit = s_SecOffense = s_SecAccuracy = nullptr;
    s_SecHitPct = s_SecCritPct = s_SecCritDmgPct = nullptr;
    s_SecDelay = s_SecDPS = nullptr;
    s_RngMaxHit = s_RngOffense = s_RngAccuracy = nullptr;
    s_RngHitPct = s_RngCritPct = s_RngCritDmgPct = nullptr;
    s_RngDelay = s_RngDPS = nullptr;
    s_SpellCritPct = nullptr;
    for (int i = 0; i < 6; ++i) { s_SpellDC[i] = nullptr; s_SpellPot[i] = nullptr; }
    s_HealCritPct = s_HealDirectPot = s_HealHoTPot = s_HealRunePot = nullptr;
    s_ThreatMod = nullptr;
    s_dirty = false;
    s_init_phase = 0;
    s_lastUpdate = 0;
}

void AoT_StatDisplay_Pulse()
{
    if (GetGameState() != GAMESTATE_INGAME) return;

    DWORD now = GetTickCount();
    if (now - s_lastUpdate < 200) return;
    s_lastUpdate = now;

    if (!s_dirty) return;

    if (s_init_phase < 4) {
        if (!ppInventoryWnd || !pInventoryWnd) return;
        auto* win = (CSidlScreenWnd*)pInventoryWnd;

        if (s_init_phase == 0) {
            // Batch 1 (13): defensive(6) + speed(2) + SpellCritPct(1) + SpellDC[0-3](4)
            s_AC           = win->GetChildItem("AISD_AC");
            s_MinMit        = win->GetChildItem("AISD_MinMit");
            s_Deflect       = win->GetChildItem("AISD_Deflect");
            s_Avoidance     = win->GetChildItem("AISD_Avoidance");
            s_AvoidPct      = win->GetChildItem("AISD_AvoidPct");
            s_Critable      = win->GetChildItem("AISD_Critable");
            s_Haste         = win->GetChildItem("AISD_Haste");
            s_CastSpeed     = win->GetChildItem("AISD_CastSpeed");
            s_SpellCritPct  = win->GetChildItem("AISD_SpellCritPct");
            s_SpellDC[0]    = win->GetChildItem("AISD_PSN_DC");
            s_SpellDC[1]    = win->GetChildItem("AISD_MAG_DC");
            s_SpellDC[2]    = win->GetChildItem("AISD_DIS_DC");
            s_SpellDC[3]    = win->GetChildItem("AISD_FIR_DC");
            s_init_phase = 1;
            return;
        }

        if (s_init_phase == 1) {
            // Batch 2 (13): SpellDC[4-5](2) + SpellPot[0-5](6) + heal(4) + threat(1)
            s_SpellDC[4]    = win->GetChildItem("AISD_CLD_DC");
            s_SpellDC[5]    = win->GetChildItem("AISD_CRP_DC");
            s_SpellPot[0]   = win->GetChildItem("AISD_PSN_Pot");
            s_SpellPot[1]   = win->GetChildItem("AISD_MAG_Pot");
            s_SpellPot[2]   = win->GetChildItem("AISD_DIS_Pot");
            s_SpellPot[3]   = win->GetChildItem("AISD_FIR_Pot");
            s_SpellPot[4]   = win->GetChildItem("AISD_CLD_Pot");
            s_SpellPot[5]   = win->GetChildItem("AISD_CRP_Pot");
            s_HealCritPct   = win->GetChildItem("AISD_HealCritPct");
            s_HealDirectPot = win->GetChildItem("AISD_HealDirect_Pot");
            s_HealHoTPot    = win->GetChildItem("AISD_HealHoT_Pot");
            s_HealRunePot   = win->GetChildItem("AISD_HealRune_Pot");
            s_ThreatMod     = win->GetChildItem("AISD_ThreatMod");
            s_init_phase = 2;
            return;
        }

        if (s_init_phase == 2) {
            // Batch 3 (13): primary(8) + secondary first 5
            s_PriMaxHit     = win->GetChildItem("AISD_PriMaxHit");
            s_PriOffense    = win->GetChildItem("AISD_PriOffense");
            s_PriAccuracy   = win->GetChildItem("AISD_PriAccuracy");
            s_PriHitPct     = win->GetChildItem("AISD_PriHitPct");
            s_PriCritPct    = win->GetChildItem("AISD_PriCritPct");
            s_PriCritDmgPct = win->GetChildItem("AISD_PriCritDmgPct");
            s_PriDelay      = win->GetChildItem("AISD_PriDelay");
            s_PriDPS        = win->GetChildItem("AISD_PriDPS");
            s_SecMaxHit     = win->GetChildItem("AISD_SecMaxHit");
            s_SecOffense    = win->GetChildItem("AISD_SecOffense");
            s_SecAccuracy   = win->GetChildItem("AISD_SecAccuracy");
            s_SecHitPct     = win->GetChildItem("AISD_SecHitPct");
            s_SecCritPct    = win->GetChildItem("AISD_SecCritPct");
            s_init_phase = 3;
            return;
        }

        // Batch 4 (11): secondary last 3 + ranged(8)
        s_SecCritDmgPct = win->GetChildItem("AISD_SecCritDmgPct");
        s_SecDelay      = win->GetChildItem("AISD_SecDelay");
        s_SecDPS        = win->GetChildItem("AISD_SecDPS");
        s_RngMaxHit     = win->GetChildItem("AISD_RngMaxHit");
        s_RngOffense    = win->GetChildItem("AISD_RngOffense");
        s_RngAccuracy   = win->GetChildItem("AISD_RngAccuracy");
        s_RngHitPct     = win->GetChildItem("AISD_RngHitPct");
        s_RngCritPct    = win->GetChildItem("AISD_RngCritPct");
        s_RngCritDmgPct = win->GetChildItem("AISD_RngCritDmgPct");
        s_RngDelay      = win->GetChildItem("AISD_RngDelay");
        s_RngDPS        = win->GetChildItem("AISD_RngDPS");
        s_init_phase = 4;
        return;
    }

    char buf[48];
    const AoT_StatBlock_Struct_Client& sb = g_sb;

    // Defensive
    sprintf_s(buf, sizeof(buf), "%d",   sb.ac_mitigation);          SetLabel(s_AC,       buf);
    sprintf_s(buf, sizeof(buf), "%d%%", sb.min_mit_pct_x100 / 100); SetLabel(s_MinMit,   buf);
    sprintf_s(buf, sizeof(buf), "%d%%", sb.deflect_pct_x100  / 100); SetLabel(s_Deflect,  buf);
    sprintf_s(buf, sizeof(buf), "%d",   sb.avoidance_score);         SetLabel(s_Avoidance,buf);
    sprintf_s(buf, sizeof(buf), "%d%%", sb.avoid_pct_x100   / 100); SetLabel(s_AvoidPct, buf);
    SetLabel(s_Critable, sb.critable ? "TRUE" : "FALSE");

    // Speed
    sprintf_s(buf, sizeof(buf), "%d%%", sb.melee_haste_x100 / 100); SetLabel(s_Haste,    buf);
    sprintf_s(buf, sizeof(buf), "%d%%", sb.cast_speed_x100  / 100); SetLabel(s_CastSpeed,buf);

    // Weapons
    WriteWeapon(s_PriMaxHit, s_PriOffense, s_PriAccuracy,
                s_PriHitPct, s_PriCritPct, s_PriCritDmgPct, s_PriDelay, s_PriDPS,
                sb.wpn_valid != 0,
                sb.wpn_max_hit, sb.wpn_offense, sb.wpn_accuracy,
                sb.wpn_hit_pct_x100, sb.wpn_crit_pct_x1000, sb.wpn_crit_dmg_pct_x100,
                sb.wpn_delay_x100, sb.wpn_dps_x100);

    WriteWeapon(s_SecMaxHit, s_SecOffense, s_SecAccuracy,
                s_SecHitPct, s_SecCritPct, s_SecCritDmgPct, s_SecDelay, s_SecDPS,
                sb.sec_valid != 0,
                sb.sec_max_hit, sb.sec_offense, sb.sec_accuracy,
                sb.sec_hit_pct_x100, sb.sec_crit_pct_x1000, sb.sec_crit_dmg_pct_x100,
                sb.sec_delay_x100, sb.sec_dps_x100);

    WriteWeapon(s_RngMaxHit, s_RngOffense, s_RngAccuracy,
                s_RngHitPct, s_RngCritPct, s_RngCritDmgPct, s_RngDelay, s_RngDPS,
                sb.rng_valid != 0,
                sb.rng_max_hit, sb.rng_offense, sb.rng_accuracy,
                sb.rng_hit_pct_x100, sb.rng_crit_pct_x1000, sb.rng_crit_dmg_pct_x100,
                sb.rng_delay_x100, sb.rng_dps_x100);

    // Spell stats
    sprintf_s(buf, sizeof(buf), "%.1f%%", sb.spell_crit_pct_x1000 / 1000.0);
    SetLabel(s_SpellCritPct, buf);
    for (int i = 0; i < 6; ++i) {
        sprintf_s(buf, sizeof(buf), "%d",   sb.spell_dc[i]);                SetLabel(s_SpellDC[i],  buf);
        sprintf_s(buf, sizeof(buf), "%d%%", sb.spell_potency_x100[i] / 100); SetLabel(s_SpellPot[i], buf);
    }

    // Healing stats
    sprintf_s(buf, sizeof(buf), "%.1f%%", sb.heal_crit_pct_x1000 / 1000.0);
    SetLabel(s_HealCritPct, buf);
    sprintf_s(buf, sizeof(buf), "%d%%", sb.heal_potency_direct_x100 / 100); SetLabel(s_HealDirectPot, buf);
    sprintf_s(buf, sizeof(buf), "%d%%", sb.heal_potency_hot_x100    / 100); SetLabel(s_HealHoTPot,    buf);
    sprintf_s(buf, sizeof(buf), "%d%%", sb.heal_potency_rune_x100   / 100); SetLabel(s_HealRunePot,   buf);

    // Threat
    sprintf_s(buf, sizeof(buf), "%d%%", sb.threat_mod_x100 / 100);
    SetLabel(s_ThreatMod, buf);

    s_dirty = false;
}
