/*	EQEMu: Everquest Server Emulator
	
	Copyright (C) 2001-2019 EQEMu Development Team (http://eqemulator.net)

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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#pragma once

#include "fmt/format.h"
#include <list>
#include <string>

class DBcore;

namespace EQ
{
	class ProfanityManager {
	public:
		static bool LoadProfanityList(DBcore *db);
		static bool UpdateProfanityList(DBcore *db);
		static bool DeleteProfanityList(DBcore *db);

		static bool AddProfanity(DBcore *db, std::string profanity);
		static bool RemoveProfanity(DBcore *db, std::string profanity);
		
		static void RedactMessage(char *message);
		static void RedactMessage(std::string &message);

		static bool ContainsCensoredLanguage(const std::string &message);

		static const std::list<std::string> &GetProfanityList();

		static bool IsCensorshipActive();

		static const char REDACTION_CHARACTER = '*';
		static const int REDACTION_LENGTH_MIN = 3;

	private:
		static bool load_database_entries(DBcore *db);
		static bool clear_database_entries(DBcore *db);
		static bool check_for_existing_entry(const std::string& profanity);
		
	};

} /*EQEmu*/
