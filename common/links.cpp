//
// Created by dannu on 4/18/2026.
//

#include "links.h"

std::string Links::FormatSpellLink(uint32_t SpellID, const std::string& SpellName)
{
    return fmt::format("{}63^{}^0^'{}{}", ITEM_TAG_CHAR, SpellID, SpellName.c_str(), ITEM_TAG_CHAR);
}
