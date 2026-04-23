//
// Created by dannu on 4/18/2026.
//

#include "links.h"

#include "spdat.h"

void Links::FormatItemLink(char* Buffer, size_t BufferSize, const EQ::ItemInstance* item)
{
    // TODO: Reverse 0x14064B220 to get definition of this function
}

void Links::FormatSpellLink(char* Buffer, size_t BufferSize, uint32_t SpellID,
    const char* spellNameOverride)
{
    snprintf(Buffer, BufferSize, "%c%d3^%d^0^'%s%c", ITEM_TAG_CHAR, ETAG_SPELL, SpellID,
        spellNameOverride && spellNameOverride[0] ? spellNameOverride : GetSpellName(SpellID), ITEM_TAG_CHAR);
}

void Links::FormatDialogLink(char* Buffer, size_t BufferSize, std::string_view keyword, std::string_view text)
{
    if (text.empty()) {
        snprintf(Buffer, BufferSize, "%c%d%.*s%c", ITEM_TAG_CHAR, ETAG_DIALOG_RESPONSE,
            static_cast<int>(keyword.length()), keyword.data(), ITEM_TAG_CHAR);
    } else {
        snprintf(Buffer, BufferSize, "%c%d%.*s:%.*s%c", ITEM_TAG_CHAR, ETAG_DIALOG_RESPONSE,
            static_cast<int>(keyword.length()), keyword.data(),
            static_cast<int>(text.length()), text.data(), ITEM_TAG_CHAR);
    }
}
