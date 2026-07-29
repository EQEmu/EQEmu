#pragma once
#include "MQ2Main.h"
#include <vector>
#include <map>
#include <set>
#include <string>
#include <cstdint>

static constexpr uint32_t ROW_TIER_HDR = 0xFFFFFFFF;

class AoT_Spell_Book : public CCustomWnd {
public:
    AoT_Spell_Book();

    int  WndNotification(CXWnd* pWnd, unsigned int msg, void* data);
    void PopulateSpellList();
    void ShowRollOptions(int level, const std::vector<uint16_t>& options);
    void RestorePendingRoll(int level, const std::vector<uint16_t>& options);
    void RevertToNormal();
    void ShowMemorizeProgress(const char* spellName, float progress);
    void ClearDetailBox();
    void RestoreDiscovered(const std::string& payload);

private:
    enum BookState { BOOK_NORMAL, BOOK_CHOOSING };

    CListWnd*   spellList           = nullptr;
    CListWnd*   detailBox           = nullptr;
    CListWnd*   discoveredList      = nullptr;
    CListWnd*   discoveredDetailBox = nullptr;
    CButtonWnd* rollBtn             = nullptr;
    CButtonWnd* memorizeBtn         = nullptr;

    BookState             m_state          = BOOK_NORMAL;
    int                   m_choosing_level = 0;
    std::vector<uint16_t> m_options;
    int                   m_chosen_idx     = -1;
    std::map<int, std::vector<uint16_t>> m_pending_rolls;

    std::vector<uint32_t> m_pool_spells;
    std::set<uint32_t>    m_discovered_set;

    void SendServerCmd(const char* cmd);
    void PopulateDetailInto(CListWnd* box, uint32_t spellId);
    void PopulateDetailBox(uint16_t spellId);
    void PopulateDiscoveredList();
};

extern AoT_Spell_Book* pAoTSpellBook;
