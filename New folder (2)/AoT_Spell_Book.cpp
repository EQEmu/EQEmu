#include "AoT_Spell_Book.h"
#include "AoT_AutoLoot.h"
#include <cmath>
#include <sstream>

#define OP_WIRE_AoTSyncPendingRolls 0x196A
#define OP_WIRE_AoTSyncDiscovered   0x196B

AoT_Spell_Book* pAoTSpellBook = nullptr;

extern int  g_aotWaitingGemBookSlot;
extern bool AoT_BeginGemWait(int bookSlot);
extern void AoT_CancelGemWaitPublic();

static int LevelToTier(int level)       { return (level - 1) / 5 + 1; }
static int TierFirstLevel(int tier)     { return (tier - 1) * 5 + 1; }
// Spell ID bit layout (16-bit, big-endian field order):
//   bits 15-13 : mastery (0-7)
//   bits 12-8  : tier 0-indexed (0-31), display as +1
//   bits 7-0   : base spell index (0-255)
static int SpellIdTier(uint32_t id)     { return ((id >> 8) & 0x1F) + 1; }
static int SpellIdMastery(uint32_t id)  { return (id >> 13) & 0x7; }

// ── Duration formula ─────────────────────────────────────────────────────────
// Returns ticks, or -1 for permanent, or 0 for instant.
static float ComputeDurationTicks(PSPELL s, int refLevel) {
    DWORD type = s->DurationType;
    DWORD val  = s->DurationValue1;
    int   L    = refLevel > 0 ? refLevel : 1;
    switch (type) {
    case  0: return 0.0f;
    case  1: return (float)(L / 2);
    case  2: return (float)(L / 5 + 1);
    case  3: return (float)(L);
    case  4: return (float)(val);
    case  5: return 3.0f;
    case  6: return 2.0f;
    case  7: return (float)(L / 4);
    case  8: return (float)((val / 10) * L);
    case  9: return 4.0f;
    case 10: return (float)(val * 5);
    case 11: return (float)(val);
    case 12: return (float)(val * L);
    case 50: return -1.0f; // permanent
    default: return (float)(val);
    }
}

// ── SPA → human-readable description ─────────────────────────────────────────

struct SimpleSPA {
    DWORD       spa;
    const char* label;
    const char* units;
    bool        per_tick;
    bool        invert; // negative base = positive effect (e.g. snare %)
};

static const SimpleSPA kSimpleSPAs[] = {
    {  0, "HP",              " HP",      false, false },
    {  1, "AC",              "",         false, false },
    {  2, "ATK",             "",         false, false },
    {  3, "Movement Speed",  "%",        false, false },
    { 11, "STR",             "",         false, false },
    { 12, "STA",             "",         false, false },
    { 13, "AGI",             "",         false, false },
    { 14, "DEX",             "",         false, false },
    { 15, "WIS",             "",         false, false },
    { 16, "INT",             "",         false, false },
    { 17, "CHA",             "",         false, false },
    { 35, "Spell Haste",     "%",        false, false },
    { 36, "Melee Haste",     "%",        false, false },
    { 46, "Mana",            "",         false, false },
    { 64, "Magic Resist",    "",         false, false },
    { 65, "Fire Resist",     "",         false, false },
    { 66, "Cold Resist",     "",         false, false },
    { 67, "Poison Resist",   "",         false, false },
    { 68, "Disease Resist",  "",         false, false },
    { 76, "Rune",            " HP",      false, false },
    { 79, "Endurance",       "",         false, false },
    { 85, "Mana Regen",      "",         true,  false },
    { 86, "Max HP",          " HP",      false, false },
    { 88, "Snare",           "%",        false, true  },
    { 96, "Disease Counter", "",         false, true  },
    { 97, "Poison Counter",  "",         false, true  },
    {100, "Hate",            "%",        false, false },
    {111, "Endurance Regen", "",         true,  false },
    {114, "HP Regen",        "",         true,  false },
    {119, "Magic Resist",    "",         false, false },
    {120, "Fire Resist",     "",         false, false },
    {121, "Cold Resist",     "",         false, false },
    {122, "Poison Resist",   "",         false, false },
    {123, "Disease Resist",  "",         false, false },
    {127, "HP Regen",        "",         true,  false },
    {162, "Endurance",       "",         false, false },
    {174, "Max HP",          "%",        false, false },
    {190, "Slow",            "%",        false, true  },
    {339, "Spell Damage",    "",         false, false },
    {529, "Dmg Reduction",   " per-hit", false, false },
    {530, "Dmg Bonus",       " per-hit", false, false },
    {533, "Fortify Attack",  " dmg/hit", false, false },
};

static void DescribeSPA(DWORD spa, LONG base, LONG /*maxval*/, DWORD /*calc*/,
                        char* out, size_t outLen, COLORREF* color)
{
    static const COLORREF BUFF   = 0xFF88FF88;
    static const COLORREF DEBUFF = 0xFFFF8888;
    static const COLORREF NEUTR  = 0xFFFFFFFF;

    for (auto& e : kSimpleSPAs) {
        if (e.spa != spa) continue;
        LONG val = e.invert ? -base : base;
        *color = (val > 0) ? BUFF : (val < 0) ? DEBUFF : NEUTR;
        if (val >= 0)
            snprintf(out, outLen, "  %s: +%ld%s%s",  e.label, val, e.units, e.per_tick ? "/tick" : "");
        else
            snprintf(out, outLen, "  %s: %ld%s%s",   e.label, val, e.units, e.per_tick ? "/tick" : "");
        return;
    }

    // Special cases
    switch (spa) {
    case 87: case 238:
        *color = DEBUFF;
        if (base > 0) snprintf(out, outLen, "  Stun: %.1fs", base / 1000.0f);
        else           snprintf(out, outLen, "  Stun");
        break;
    case 89:
        *color = DEBUFF;
        snprintf(out, outLen, "  Fear");
        break;
    case 90:
        *color = DEBUFF;
        snprintf(out, outLen, "  Blind");
        break;
    case 91: case 239:
        *color = DEBUFF;
        snprintf(out, outLen, "  Mesmerize");
        break;
    case 92: case 240:
        *color = DEBUFF;
        snprintf(out, outLen, "  Charm");
        break;
    case 242:
        *color = DEBUFF;
        snprintf(out, outLen, "  Root");
        break;
    case 248:
        *color = BUFF;
        snprintf(out, outLen, "  See Invisible");
        break;
    case 249:
        *color = BUFF;
        snprintf(out, outLen, "  Invisible to Undead");
        break;
    default:
        *color = NEUTR;
        snprintf(out, outLen, "  Effect (SPA %u): %ld", (unsigned)spa, base);
        break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────

AoT_Spell_Book::AoT_Spell_Book() : CCustomWnd("AoTSpellBookWnd") {
    CloseOnESC = 1;
    SetWndNotification(AoT_Spell_Book);
    spellList           = (CListWnd*)GetChildItem("ASBW_SpellList");
    detailBox           = (CListWnd*)GetChildItem("ASBW_SpellDetail");
    discoveredList      = (CListWnd*)GetChildItem("ASBW_DiscoveredList");
    discoveredDetailBox = (CListWnd*)GetChildItem("ASBW_DiscoveredDetail");
    rollBtn             = (CButtonWnd*)GetChildItem("ASBW_RollBtn");
    memorizeBtn         = (CButtonWnd*)GetChildItem("ASBW_MemorizeBtn");
    if (!spellList)
        WriteChatf("[AoT] SpellBook: spellList null -- SIDL parse error?");
    else
        RevertToNormal(); // populates list and sets button labels to their default state
    AoT_SendRawPacket(OP_WIRE_AoTSyncPendingRolls, nullptr, 0);
    AoT_SendRawPacket(OP_WIRE_AoTSyncDiscovered, nullptr, 0);
}

void AoT_Spell_Book::SendServerCmd(const char* cmd) {
    if (pEverQuest && pLocalPlayer)
        pEverQuest->InterpretCmd((EQPlayer*)pLocalPlayer, const_cast<char*>(cmd));
}

// ── Spell list (normal + choosing modes) ─────────────────────────────────────

void AoT_Spell_Book::PopulateSpellList() {
    if (!spellList) return;
    spellList->DeleteAll();

    PCHARINFO2 ci2 = GetCharInfo2();
    int playerLevel = (ci2 ? (int)ci2->Level : 1);
    int maxTier     = LevelToTier(playerLevel);

    static const COLORREF COLOR_TIER_HDR = 0xFF00AAFF;
    static const COLORREF COLOR_UNROLLED = 0xFFFFD700;
    static const COLORREF COLOR_SCRIBED  = 0xFFFFFFFF;
    static const COLORREF COLOR_PENDING  = 0xFFFF8C00;

    for (int tier = 1; tier <= maxTier; ++tier) {
        char tierLabel[64];
        sprintf_s(tierLabel, sizeof(tierLabel), "Tier %d (Levels %d-%d)",
                  tier, TierFirstLevel(tier), TierFirstLevel(tier) + 4);
        CXStr emptyStr("");
        int hdrRow = spellList->AddString(emptyStr, COLOR_TIER_HDR,
                                          ROW_TIER_HDR, nullptr, nullptr);
        CXStr tierLabelStr(tierLabel);
        spellList->SetItemText(hdrRow, 1, &tierLabelStr);
        spellList->SetItemColor(hdrRow, 1, COLOR_TIER_HDR);
        spellList->SetItemText(hdrRow, 2, &emptyStr);

        int tierEnd = TierFirstLevel(tier) + 4;
        for (int lvl = TierFirstLevel(tier); lvl <= tierEnd; ++lvl) {
            if (lvl > playerLevel) break;

            bool hasPending = m_pending_rolls.count(lvl) > 0;

            char lvlStr[8];
            sprintf_s(lvlStr, sizeof(lvlStr), "%d", lvl);
            CXStr lvlCXStr(lvlStr);

            int bookIdx = lvl - 1;
            DWORD spellId = (ci2 && bookIdx < NUM_BOOK_SLOTS) ? ci2->SpellBook[bookIdx] : 0xFFFFFFFF;
            PSPELL pSpell = (spellId != 0xFFFFFFFF && spellId != 0) ? GetSpellByID(spellId) : nullptr;

            COLORREF rowColor;
            CXStr slotStr;
            if (pSpell) {
                rowColor = COLOR_SCRIBED;
                slotStr  = CXStr(pSpell->Name);
            } else if (hasPending) {
                rowColor = COLOR_PENDING;
                slotStr  = CXStr("Pending - press Roll to choose");
            } else {
                rowColor = COLOR_UNROLLED;
                slotStr  = CXStr("Ready to be rolled");
            }

            int row = spellList->AddString(lvlCXStr, rowColor,
                                           (uint32_t)lvl, nullptr, nullptr);
            CXStr emptyStr2("");
            spellList->SetItemText(row, 1, &slotStr);
            spellList->SetItemText(row, 2, &emptyStr2);
            spellList->SetItemColor(row, 0, rowColor);
            spellList->SetItemColor(row, 1, rowColor);
        }
    }
}

void AoT_Spell_Book::ShowRollOptions(int level, const std::vector<uint16_t>& options) {
    m_state          = BOOK_CHOOSING;
    m_choosing_level = level;
    m_options        = options;
    m_chosen_idx     = -1;
    m_pending_rolls[level] = options;

    if (spellList) {
        spellList->DeleteAll();
        static const COLORREF COLOR_OPTION = 0xFFFFFFFF;
        for (int i = 0; i < (int)m_options.size(); ++i) {
            char label[64];
            sprintf_s(label, sizeof(label), "Option %d", i + 1);
            CXStr labelStr(label);
            PSPELL pSpell = GetSpellByID(m_options[i]);
            CXStr spellCXStr(pSpell ? pSpell->Name : "Unknown Spell");
            int row = spellList->AddString(labelStr, COLOR_OPTION,
                                           (uint32_t)i, nullptr, nullptr);
            spellList->SetItemText(row, 1, &spellCXStr);
        }
    }

    if (rollBtn) {
        CXStr s("Choose");
        reinterpret_cast<CXWnd*>(rollBtn)->SetWindowTextA(s);
    }
    if (memorizeBtn) {
        CXStr s("Back");
        reinterpret_cast<CXWnd*>(memorizeBtn)->SetWindowTextA(s);
    }
}

void AoT_Spell_Book::RestorePendingRoll(int level, const std::vector<uint16_t>& options) {
    if (options.empty()) return;
    m_pending_rolls[level] = options;
    if (m_state == BOOK_NORMAL)
        PopulateSpellList();
}

// ── Revert to normal mode ─────────────────────────────────────────────────────

void AoT_Spell_Book::RevertToNormal() {
    m_state      = BOOK_NORMAL;
    m_chosen_idx = -1;
    AoT_CancelGemWaitPublic();

    if (rollBtn) {
        CXStr s("Roll Spell");
        reinterpret_cast<CXWnd*>(rollBtn)->SetWindowTextA(s);
    }
    if (memorizeBtn) {
        CXStr s("Memorize Spell");
        reinterpret_cast<CXWnd*>(memorizeBtn)->SetWindowTextA(s);
    }

    if (detailBox) detailBox->DeleteAll();
    PopulateSpellList();
}

// ── Discovered spells list ────────────────────────────────────────────────────

static void ParseCommaIds(const std::string& s, std::vector<uint32_t>& out) {
    std::stringstream ss(s);
    std::string token;
    while (std::getline(ss, token, ',')) {
        if (token.empty()) continue;
        try { out.push_back(static_cast<uint32_t>(std::stoul(token))); }
        catch (...) {}
    }
}

void AoT_Spell_Book::RestoreDiscovered(const std::string& payload) {
    m_pool_spells.clear();
    m_discovered_set.clear();

    if (payload.empty()) {
        PopulateDiscoveredList();
        return;
    }

    if (payload.size() > 2 && payload[0] == 'P' && payload[1] == ':') {
        // New format: "P:sp1,sp2,...|D:sp3,sp4,..."
        auto pipe = payload.find('|');
        size_t pool_end = (pipe != std::string::npos) ? pipe : payload.size();
        std::string pool_part = payload.substr(2, pool_end - 2);
        ParseCommaIds(pool_part, m_pool_spells);

        if (pipe != std::string::npos && payload.size() > pipe + 3
            && payload[pipe + 1] == 'D' && payload[pipe + 2] == ':') {
            std::vector<uint32_t> disc_vec;
            ParseCommaIds(payload.substr(pipe + 3), disc_vec);
            for (uint32_t id : disc_vec)
                m_discovered_set.insert(id);
        }
    } else {
        // Legacy format: comma-separated discovered IDs only.
        // Treat each as both a pool entry and discovered so they still show up.
        ParseCommaIds(payload, m_pool_spells);
        for (uint32_t id : m_pool_spells)
            m_discovered_set.insert(id);
    }

    PopulateDiscoveredList();
}

void AoT_Spell_Book::PopulateDiscoveredList() {
    if (!discoveredList) return;
    discoveredList->DeleteAll();

    static const COLORREF COLOR_TIER_HDR  = 0xFF00AAFF;
    static const COLORREF C_UNDISCOVERED  = 0xFF888888;
    static const COLORREF C_DETRIM        = 0xFFFF5555;
    static const COLORREF C_BENEF         = 0xFF55AAFF;
    static const COLORREF C_GROUP         = 0xFF55FF99;

    int currentTier = -1;

    for (size_t i = 0; i < m_pool_spells.size(); ++i) {
        uint32_t sid  = m_pool_spells[i];
        int      tier = SpellIdTier(sid);

        if (tier != currentTier) {
            currentTier = tier;
            int firstLevel = TierFirstLevel(tier);
            char label[64];
            sprintf_s(label, sizeof(label), "--- Tier %d (Levels %d-%d) ---",
                      tier, firstLevel, firstLevel + 4);
            CXStr labelStr(label);
            int hdrRow = discoveredList->AddString(labelStr, COLOR_TIER_HDR,
                                                   ROW_TIER_HDR, nullptr, nullptr);
            discoveredList->SetItemColor(hdrRow, 0, COLOR_TIER_HDR);
        }

        bool discovered = m_discovered_set.count(sid) > 0;

        if (discovered) {
            PSPELL   pSpell = GetSpellByID(sid);
            COLORREF color  = C_BENEF;
            char     name[128];
            if (pSpell) {
                color = (pSpell->SpellType == 0) ? C_DETRIM :
                        (pSpell->SpellType == 2) ? C_GROUP  : C_BENEF;
                int mastery = SpellIdMastery(sid);
                if (mastery > 0)
                    sprintf_s(name, sizeof(name), "  %s  (Mastery %d)", pSpell->Name, mastery);
                else
                    sprintf_s(name, sizeof(name), "  %s", pSpell->Name);
            } else {
                sprintf_s(name, sizeof(name), "  Unknown (%u)", (unsigned)sid);
            }
            CXStr nameStr(name);
            int row = discoveredList->AddString(nameStr, color, (uint32_t)i, nullptr, nullptr);
            discoveredList->SetItemColor(row, 0, color);
        } else {
            CXStr unknownStr("  ???");
            int row = discoveredList->AddString(unknownStr, C_UNDISCOVERED,
                                                (uint32_t)i, nullptr, nullptr);
            discoveredList->SetItemColor(row, 0, C_UNDISCOVERED);
        }
    }
}

// ── Memorization progress gauge ───────────────────────────────────────────────

void AoT_Spell_Book::ClearDetailBox() {
    if (detailBox) detailBox->DeleteAll();
}

void AoT_Spell_Book::ShowMemorizeProgress(const char* spellName, float progress) {
    if (!detailBox) return;
    detailBox->DeleteAll();

    char line1[128];
    sprintf_s(line1, sizeof(line1), "Memorizing %s, please wait...", spellName);
    CXStr cs1(line1);
    int r1 = detailBox->AddString(cs1, 0xFFFFFF00, 0, nullptr, nullptr);
    detailBox->SetItemColor(r1, 0, 0xFFFFFF00);

    // EQ's UI font is proportional — '=' is ~2x the width of ' '.
    // Two spaces per empty slot approximates the visual width of one '='.
    const int W = 20;
    int filled = (int)(progress * W);
    if (filled > W) filled = W;
    char bar[64]; // '[' + 20 '=' + 40 spaces + ']' + '\0'
    int pos = 0;
    bar[pos++] = '[';
    for (int i = 0; i < filled; i++) bar[pos++] = '=';
    for (int i = filled; i < W; i++) { bar[pos++] = ' '; bar[pos++] = ' '; }
    bar[pos++] = ']';
    bar[pos]   = '\0';
    CXStr cs2(bar);
    int r2 = detailBox->AddString(cs2, 0xFF00FF00, 0, nullptr, nullptr);
    detailBox->SetItemColor(r2, 0, 0xFF00FF00);
}

// ── Spell detail panel ────────────────────────────────────────────────────────

void AoT_Spell_Book::PopulateDetailBox(uint16_t spellId) {
    PopulateDetailInto(detailBox, (uint32_t)spellId);
}

void AoT_Spell_Book::PopulateDetailInto(CListWnd* box, uint32_t spellId) {
    if (!box) return;
    box->DeleteAll();

    PSPELL s = GetSpellByID(spellId);
    if (!s) return;

    static const COLORREF C_WHITE   = 0xFFFFFFFF;
    static const COLORREF C_LABEL   = 0xFFCCCCCC;
    static const COLORREF C_SECTION = 0xFFFFAA00;
    static const COLORREF C_DETRIM  = 0xFFFF5555;
    static const COLORREF C_BENEF   = 0xFF55AAFF;
    static const COLORREF C_GROUP   = 0xFF55FF99;

    auto addLine = [&](const char* text, COLORREF color) {
        CXStr cs(text);
        int row = box->AddString(cs, color, 0, nullptr, nullptr);
        box->SetItemColor(row, 0, color);
    };

    auto addSection = [&](const char* title) { addLine(title, C_SECTION); };

    auto addField = [&](const char* label, const char* value) {
        char line[256];
        sprintf_s(line, sizeof(line), "  %-14s %s", label, value);
        addLine(line, C_LABEL);
    };

    // ── Name + type ──────────────────────────────────────────────────────────
    COLORREF nameColor = (s->SpellType == 0) ? C_DETRIM :
                         (s->SpellType == 2) ? C_GROUP  : C_BENEF;
    addLine(s->Name, nameColor);

    const char* typeStr = (s->SpellType == 0) ? "Detrimental" :
                          (s->SpellType == 2) ? "Group Beneficial" : "Beneficial";
    {
        char typeLine[64];
        sprintf_s(typeLine, sizeof(typeLine), "  %s", typeStr);
        addLine(typeLine, nameColor);
    }

    // ── Casting ──────────────────────────────────────────────────────────────
    addSection("--- Casting ---");

    char tmp[128];

    if (s->Mana > 0) {
        sprintf_s(tmp, sizeof(tmp), "%u", (unsigned)s->Mana);
        addField("Mana:", tmp);
    }
    if (s->EnduranceCost > 0) {
        sprintf_s(tmp, sizeof(tmp), "%u", (unsigned)s->EnduranceCost);
        addField("Endurance:", tmp);
    }

    if (s->CastTime == 0) {
        addField("Cast Time:", "Instant");
    } else {
        sprintf_s(tmp, sizeof(tmp), "%.2fs", s->CastTime / 1000.0f);
        addField("Cast Time:", tmp);
    }

    if (s->RecastTime > 0) {
        sprintf_s(tmp, sizeof(tmp), "%.2fs", s->RecastTime / 1000.0f);
        addField("Recast:", tmp);
    }

    // Duration
    PCHARINFO2 ci2 = GetCharInfo2();
    int refLevel = ci2 ? (int)ci2->Level : 50;
    float ticks = ComputeDurationTicks(s, refLevel);
    if (s->DurationType == 0 || ticks == 0.0f) {
        addField("Duration:", "Instant");
    } else if (ticks < 0.0f) {
        addField("Duration:", "Permanent");
    } else {
        float secs = ticks * 6.0f;
        if (secs >= 60.0f) {
            int mins = (int)(secs / 60.0f);
            int rem  = (int)fmodf(secs, 60.0f);
            if (rem > 0)
                sprintf_s(tmp, sizeof(tmp), "%.0f ticks  (%dm %ds)", ticks, mins, rem);
            else
                sprintf_s(tmp, sizeof(tmp), "%.0f ticks  (%dm)", ticks, mins);
        } else {
            sprintf_s(tmp, sizeof(tmp), "%.0f ticks  (%.0fs)", ticks, secs);
        }
        addField("Duration:", tmp);
    }

    // ── Targeting ────────────────────────────────────────────────────────────
    addSection("--- Targeting ---");

    static const struct { BYTE id; const char* name; } kTargets[] = {
        { 2,"Single (no MGB)"}, { 3,"Group v1"},    { 4,"PB AE"},
        { 5,"Single"},          { 6,"Self"},         { 7,"Targeted Group"},
        { 8,"Targeted AE"},     { 9,"Animal"},       {10,"Undead"},
        {11,"Summoned"},        {13,"Lifetap"},      {14,"Pet"},
        {15,"Corpse"},          {40,"AE PC"},        {41,"Group v2"},
        {50,"Beam"},
    };
    const char* targetName = "Unknown";
    for (auto& t : kTargets)
        if (t.id == s->TargetType) { targetName = t.name; break; }
    addField("Target:", targetName);

    if (s->Range > 0.0f) {
        sprintf_s(tmp, sizeof(tmp), "%.0f", s->Range);
        addField("Range:", tmp);
    }
    if (s->AERange > 0.0f) {
        sprintf_s(tmp, sizeof(tmp), "%.0f", s->AERange);
        addField("AE Radius:", tmp);
    }

    static const char* kResists[] = {
        "Unresistable","Magic","Fire","Cold","Poison",
        "Disease","Chromatic","Prismatic","Physical","Corruption"
    };
    if (s->Resist < 10)
        addField("Resist:", kResists[s->Resist]);

    // ── Effects ──────────────────────────────────────────────────────────────
    addSection("--- Effects ---");

    bool anyEffect = false;
    for (int i = 0; i < 12; ++i) {
        DWORD spa = s->Attrib[i];
        if (spa == 254 || spa == 0xFFFFFFFF) continue;
        anyEffect = true;

        char effectStr[256];
        COLORREF effectColor = C_WHITE;
        DescribeSPA(spa, s->Base[i], s->Max[i], s->Calc[i],
                    effectStr, sizeof(effectStr), &effectColor);
        addLine(effectStr, effectColor);
    }
    if (!anyEffect)
        addLine("  (no effects)", C_LABEL);

    if (s->CastOnYou[0]) {
        char msg[96];
        sprintf_s(msg, sizeof(msg), "  \"%s\"", s->CastOnYou);
        addLine(msg, 0xFF888888);
    }
}

// ── Input / notifications ─────────────────────────────────────────────────────

int AoT_Spell_Book::WndNotification(CXWnd* pWnd, unsigned int msg, void* data) {
    if (msg == XWM_LCLICK) {

        // ── Roll / Choose button ──────────────────────────────────────────────
        if (pWnd == (CXWnd*)rollBtn) {
            if (m_state == BOOK_NORMAL) {
                int sel = spellList ? spellList->GetCurSel() : -1;
                if (sel < 0) {
                    WriteChatf("[AoT] Select a spell slot first.");
                    return 1;
                }
                uint32_t rowData = spellList->GetItemData(sel);
                if (rowData == ROW_TIER_HDR) {
                    WriteChatf("[AoT] Select a level row, not a tier header.");
                    return 1;
                }
                int level = (int)rowData;
                if (m_pending_rolls.count(level) > 0) {
                    ShowRollOptions(level, m_pending_rolls[level]);
                } else {
                    char cmd[64];
                    sprintf_s(cmd, sizeof(cmd), "#rollspell %d", level);
                    SendServerCmd(cmd);
                }
            } else if (m_state == BOOK_CHOOSING) {
                if (m_chosen_idx < 0 || m_chosen_idx >= (int)m_options.size()) {
                    WriteChatf("[AoT] Select one of the options first.");
                    return 1;
                }
                char cmd[64];
                sprintf_s(cmd, sizeof(cmd), "#choosespell %d %u",
                          m_choosing_level, (unsigned)m_options[m_chosen_idx]);
                m_pending_rolls.erase(m_choosing_level);

                // Optimistic write so list refreshes immediately
                PCHARINFO2 ci2 = GetCharInfo2();
                if (ci2 && m_choosing_level > 0 && m_choosing_level <= NUM_BOOK_SLOTS)
                    ci2->SpellBook[m_choosing_level - 1] = m_options[m_chosen_idx];

                SendServerCmd(cmd);
                AoT_SendRawPacket(OP_WIRE_AoTSyncDiscovered, nullptr, 0);
                RevertToNormal();
            }
            return 1;
        }

        // ── Cancel / Back / Memorize Spell button ────────────────────────────
        if (pWnd == (CXWnd*)memorizeBtn) {
            if (m_state == BOOK_CHOOSING || g_aotWaitingGemBookSlot >= 0) {
                RevertToNormal();
                return 1;
            }
            // BOOK_NORMAL: enter "waiting for gem click" state
            int sel = spellList ? spellList->GetCurSel() : -1;
            if (sel < 0) {
                WriteChatf("[AoT] Select a spell to memorize first.");
                return 1;
            }
            uint32_t rowData = spellList->GetItemData(sel);
            if (rowData == ROW_TIER_HDR) {
                WriteChatf("[AoT] Select a spell level row, not a tier header.");
                return 1;
            }
            int level   = (int)rowData;
            int bookIdx = level - 1;
            PCHARINFO2 ci2 = GetCharInfo2();
            DWORD sid = (ci2 && bookIdx >= 0 && bookIdx < NUM_BOOK_SLOTS)
                        ? ci2->SpellBook[bookIdx] : 0xFFFFFFFF;
            if (sid == 0xFFFFFFFF || sid == 0) {
                WriteChatf("[AoT] No spell scribed at that level.");
                return 1;
            }
            if (!AoT_BeginGemWait(bookIdx)) {
                WriteChatf("[AoT] Spell gem windows not found — reopen the book and try again.");
                return 1;
            }
            if (memorizeBtn) {
                CXStr s("Cancel");
                reinterpret_cast<CXWnd*>(memorizeBtn)->SetWindowTextA(s);
            }
            if (detailBox) {
                detailBox->DeleteAll();
                PSPELL pSpell = GetSpellByID(sid);
                char prompt[256];
                sprintf_s(prompt, sizeof(prompt),
                    "Click a spell gem slot on your spell bar to memorize %s.",
                    pSpell ? pSpell->Name : "this spell");
                CXStr cs(prompt);
                int row = detailBox->AddString(cs, 0xFFFFFF00, 0, nullptr, nullptr);
                detailBox->SetItemColor(row, 0, 0xFFFFFF00);
            }
            return 1;
        }

        // ── Spell list clicks ─────────────────────────────────────────────────
        if (pWnd == (CXWnd*)spellList) {
            int sel = spellList->GetCurSel();
            if (sel >= 0) {
                uint32_t rowData = spellList->GetItemData(sel);
                if (m_state == BOOK_CHOOSING) {
                    m_chosen_idx = (int)rowData;
                    if (m_chosen_idx >= 0 && m_chosen_idx < (int)m_options.size())
                        PopulateDetailBox(m_options[m_chosen_idx]);
                } else {
                    if (rowData != ROW_TIER_HDR) {
                        PCHARINFO2 ci2 = GetCharInfo2();
                        int bookIdx = (int)rowData - 1;
                        if (ci2 && bookIdx >= 0 && bookIdx < NUM_BOOK_SLOTS) {
                            DWORD sid = ci2->SpellBook[bookIdx];
                            if (sid != 0xFFFFFFFF && sid != 0)
                                PopulateDetailBox(static_cast<uint16_t>(sid));
                            else if (detailBox)
                                detailBox->DeleteAll();
                        }
                    }
                }
            }
            return 1;
        }

        // ── Discovered list clicks ────────────────────────────────────────────
        if (pWnd == (CXWnd*)discoveredList) {
            int sel = discoveredList->GetCurSel();
            if (sel >= 0) {
                uint32_t idx = discoveredList->GetItemData(sel);
                if (idx == ROW_TIER_HDR) return 1;
                if (idx < (uint32_t)m_pool_spells.size()) {
                    uint32_t sid = m_pool_spells[idx];
                    if (m_discovered_set.count(sid) > 0) {
                        PopulateDetailInto(discoveredDetailBox, sid);
                    } else if (discoveredDetailBox) {
                        discoveredDetailBox->DeleteAll();
                        CXStr msg("Not yet discovered.");
                        int row = discoveredDetailBox->AddString(msg, 0xFF888888, 0, nullptr, nullptr);
                        discoveredDetailBox->SetItemColor(row, 0, 0xFF888888);
                    }
                }
            }
            return 1;
        }
    }

    if (msg == XWM_CLOSE) {
        pXWnd()->Show(0, 1);
        return 1;
    }
    return CSidlScreenWnd::WndNotification(pWnd, msg, data);
}
