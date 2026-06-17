//
// Created by dannu on 4/18/2026.
//

#pragma once
#include "item_instance.h"

namespace EQ { class ItemInstance; }

namespace Links
{
// Max Link Size in bytes
constexpr size_t MAX_LINK_SIZE = 512;

// Universal link tag character
constexpr char ITEM_TAG_CHAR = '\x12';

// Enumeration of different types of item tags
enum ETagCodes
{
    ETAG_ITEM = 0,
    ETAG_PLAYER,
    ETAG_SPAM,
    ETAG_ACHIEVEMENT,
    ETAG_DIALOG_RESPONSE,
    ETAG_COMMAND,
    ETAG_SPELL,
    ETAG_FACTION,
    ETAG_COMMAND2,
    ETAG_UNKNOWN9,

    ETAG_COUNT,
    ETAG_FIRST = ETAG_ITEM,
    ETAG_LAST = ETAG_UNKNOWN9,

    ETAG_INVALID = -1,
};

//----------------------------------------------------------------------------
// Link Formatting -- Pulled from MQ code

// Create an achievement link for the given achievement.
// TODO: implement this when achievements are added, leave the signature here for reference. Code in eqlib's ItemLinks.cpp
// void FormatAchievementLink(char* Buffer, size_t BufferSize, const Achievement* achievement,
//     std::string_view playerName);

// Create an item link from the given item.
void FormatItemLink(char* Buffer, size_t BufferSize, const EQ::ItemInstance* item);

// Create a spell link for the given spell, with optional spell name override. Spells on items often have
// spell name overrides that changes the display name of the spell.
void FormatSpellLink(char* Buffer, size_t BufferSize, int32_t SpellID,
    const char* spellNameOverride = nullptr);

// Format text into a clickable dialog link. The keyword is the text that will be displayed in the chat window,
// and the text is the text that will be sent to the server when the link is clicked. If no text is provided,
// then the keyword will be used as the text.
void FormatDialogLink(char* Buffer, size_t BufferSize, std::string_view keyword,
    std::string_view text = {});

}
