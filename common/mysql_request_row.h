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

#include "common/types.h"

#include "mysql.h"
#include <iterator>

class MySQLRequestRow
{
public:
	using iterator_category = std::input_iterator_tag;
	using value_type = MYSQL_ROW;
	using difference_type = std::ptrdiff_t;
	using pointer = MYSQL_ROW*;
	using reference = MYSQL_ROW&;

private:
	MYSQL_RES* m_Result;
	MYSQL_ROW m_MySQLRow;

public:

	MySQLRequestRow();
	MySQLRequestRow(MYSQL_RES *result);
	MySQLRequestRow(const MySQLRequestRow& row);
	MySQLRequestRow(MySQLRequestRow&& moveItem);
	MySQLRequestRow& operator=(MySQLRequestRow& moveItem);
	MySQLRequestRow& operator++();
	MySQLRequestRow operator++(int);
	bool operator==(const MySQLRequestRow& rhs);
	bool operator!=(const MySQLRequestRow& rhs);
	MySQLRequestRow operator*();
	char* operator[](int index);
};
