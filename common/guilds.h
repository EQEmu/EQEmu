/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#pragma once

#include "common/types.h"

#define GUILD_NONE		0xFFFFFFFF // user has no guild

#define GUILD_MAX_RANK                     8 // 0-2 - some places in the code assume a single digit, dont go above 9
#define GUILD_MAX_FUNCTIONS                30
#define GUILD_TRIBUTES_MODIFY              1
#define GUILD_TRIBUTES_SAVE                0
#define GUILD_TRIBUTES_OFF                 0
#define GUILD_TRIBUTES_ON                  1
#define GUILD_INVITE_DECLINE               9

//defines for standard ranks
#define GUILD_MEMBER_TI	    0
#define GUILD_OFFICER_TI    1
#define GUILD_LEADER_TI     2
#define GUILD_RANK_NONE_TI (GUILD_MAX_RANK + 1)

//defines for standard ranks base on RoF2 definitions
#define GUILD_RANK_NONE      0
#define GUILD_LEADER         1
#define GUILD_SENIOR_OFFICER 2
#define GUILD_OFFICER        3
#define GUILD_SENIOR_MEMBER  4
#define GUILD_MEMBER         5
#define GUILD_JUNIOR_MEMBER  6
#define GUILD_INITIATE       7
#define GUILD_RECRUIT        8

typedef enum {
	GUILD_ACTION_BANNER_CHANGE                        = 1,
	GUILD_ACTION_BANNER_PLANT                         = 2,
	GUILD_ACTION_BANNER_REMOVE                        = 3,
	GUILD_ACTION_DISPLAY_GUILD_NAME                   = 4,
	GUILD_ACTION_RANKS_CHANGE_PERMISSIONS             = 5,
	GUILD_ACTION_RANKS_CHANGE_RANK_NAMES              = 6,
	GUILD_ACTION_MEMBERS_INVITE                       = 7,
	GUILD_ACTION_MEMBERS_PROMOTE                      = 8,
	GUILD_ACTION_MEMBERS_DEMOTE                       = 9,
	GUILD_ACTION_MEMBERS_REMOVE                       = 10,
	GUILD_ACTION_EDIT_RECRUITING_SETTINGS             = 11,
	GUILD_ACTION_EDIT_PUBLIC_NOTES                    = 12,
	GUILD_ACTION_BANK_DEPOSIT_ITEMS                   = 13,
	GUILD_ACTION_BANK_WITHDRAW_ITEMS                  = 14,
	GUILD_ACTION_BANK_VIEW_ITEMS                      = 15,
	GUILD_ACTION_BANK_PROMOTE_ITEMS                   = 16,
	GUILD_ACTION_BANK_CHANGE_ITEM_PERMISSIONS         = 17,
	GUILD_ACTION_CHANGE_THE_MOTD                      = 18,
	GUILD_ACTION_GUILD_CHAT_SEE                       = 19,
	GUILD_ACTION_GUILD_CHAT_SPEAK_IN                  = 20,
	GUILD_ACTION_SEND_THE_WHOLE_GUILD_E_MAIL          = 21,
	GUILD_ACTION_TRIBUTE_CHANGE_FOR_OTHERS            = 22,
	GUILD_ACTION_TRIBUTE_CHANGE_ACTIVE_BENEFIT        = 23,
	GUILD_ACTION_TROPHY_TRIBUTE_CHANGE_FOR_OTHERS     = 24,
	GUILD_ACTION_TROPHY_TRIBUTE_CHANGE_ACTIVE_BENEFIT = 25,
	GUILD_ACTION_MEMBERS_CHANGE_ALT_FLAG_FOR_OTHER    = 26,
	GUILD_ACTION_REAL_ESTATE_GUILD_PLOT_BUY           = 27,
	GUILD_ACTION_REAL_ESTATE_GUILD_PLOT_SELL          = 28,
	GUILD_ACTION_REAL_ESTATE_MODIFY_TROPHIES          = 29,
	GUILD_ACTION_MEMBERS_DEMOTE_SELF                  = 30,
} GuildAction;

constexpr int format_as(GuildAction action) { return static_cast<int>(action); }
