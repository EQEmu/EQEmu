//
// Created by dannu on 4/18/2026.
//

#include "links.h"

void Links::FormatItemLink(char* Buffer, size_t BufferSize, const std::string& text, const EQ::SayLinkBody_Struct item)
{
    // interspersed between augs and after ornament_icon are assumed zero
    // This is specifically the TOB style item link (ref: 0x14064B220), this could be in patches
    snprintf(Buffer, BufferSize,
        "%c%d"
        "%05X"
        "%05X%05X"  // aug1, luck
        "%05X%05X"  // aug2, luck
        "%05X%05X"  // aug3, luck
        "%05X%05X"  // aug4, luck
        "%05X%05X"  // aug5, luck
        "%05X%05X"  // aug6, luck
        "%1d"       // is_evolving
        "%04X"      // evolve_group
        "%02X"      // evolve_level
        "%05X"      // ornament_icon
        "%05X"      // luck
        "%08X"      // hash
        "%s%c",
        ITEM_TAG_CHAR, ETAG_ITEM,
        item.item_id,
        item.augment_1, 0u, // Set Luck to 0 for all augs until server supports it
        item.augment_2, 0u, // Set Luck to 0 for all augs until server supports it
        item.augment_3, 0u, // Set Luck to 0 for all augs until server supports it
        item.augment_4, 0u, // Set Luck to 0 for all augs until server supports it
        item.augment_5, 0u, // Set Luck to 0 for all augs until server supports it
        item.augment_6, 0u, // Set Luck to 0 for all augs until server supports it
        item.is_evolving,
        item.evolve_group,
        item.evolve_level,
        item.ornament_icon,
        0u, // Set Luck to 0 until server supports it
        item.hash,
        text.c_str(), ITEM_TAG_CHAR
    );
}

void Links::FormatSpellLink(char* Buffer, size_t BufferSize, int32_t SpellID, const char* spellName)
{
    snprintf(Buffer, BufferSize, "%c%d3^%d^0^'%s%c", ITEM_TAG_CHAR, ETAG_SPELL, SpellID,
        spellName, ITEM_TAG_CHAR);
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

void Links::FormatDialogLinkContent(char* Buffer, size_t BufferSize, std::string_view keyword, std::string_view text)
{
    if (text.empty()) {
        snprintf(Buffer, BufferSize, "%.*s",
            static_cast<int>(keyword.length()), keyword.data());
    } else {
        snprintf(Buffer, BufferSize, "%.*s:%.*s",
            static_cast<int>(keyword.length()), keyword.data(),
            static_cast<int>(text.length()), text.data());
    }
}
