/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <memory>

class Database;

// Base class for quest apis to manage connection to a MySQL database
class QuestDB
{
public:
	enum class Connection { Default = 0, Content };

	// Throws std::runtime_error on connection failure
	QuestDB() : QuestDB(Connection::Default) {}
	QuestDB(Connection type) : QuestDB(type, false) {}
	QuestDB(Connection type, bool connect);
	QuestDB(const char* host, const char* user, const char* pass, const char* db, uint32_t port);

protected:
	// allow optional ownership of pointer to support using zone db connections
	struct Deleter
	{
		Deleter() : owner(true) {}
		Deleter(bool owner_) : owner(owner_) {}
		bool owner = true;
		void operator()(Database* ptr) noexcept;
	};

	std::unique_ptr<Database, Deleter> m_db;
};
