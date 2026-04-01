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
#include "mysql_request_row.h"

MySQLRequestRow::MySQLRequestRow(const MySQLRequestRow& row)
	: m_Result(row.m_Result), m_MySQLRow(row.m_MySQLRow)
{
}

MySQLRequestRow::MySQLRequestRow()
	: m_Result(nullptr), m_MySQLRow(nullptr)
{
}

MySQLRequestRow::MySQLRequestRow(MySQLRequestRow&& moveItem)
{
	m_Result = moveItem.m_Result;
	m_MySQLRow = moveItem.m_MySQLRow;

	moveItem.m_Result = nullptr;
	moveItem.m_MySQLRow = nullptr;
}

MySQLRequestRow& MySQLRequestRow::operator=(MySQLRequestRow& moveItem)
{
	m_Result = moveItem.m_Result;
	m_MySQLRow = moveItem.m_MySQLRow;

	moveItem.m_Result = nullptr;
	moveItem.m_MySQLRow = nullptr;

	return *this;
}


MySQLRequestRow MySQLRequestRow::operator*()
{
	return *this;
}

MySQLRequestRow::MySQLRequestRow(MYSQL_RES *result)
	: m_Result(result)
{
    if (result != nullptr)
        m_MySQLRow = mysql_fetch_row(result);
    else
        m_MySQLRow = nullptr;
}

MySQLRequestRow& MySQLRequestRow::operator++()
{
	m_MySQLRow = mysql_fetch_row(m_Result);
	return *this;
}

MySQLRequestRow MySQLRequestRow::operator++(int)
{
	MySQLRequestRow tmp(*this);
	operator++();
	return tmp;
}

bool MySQLRequestRow::operator==(const MySQLRequestRow& rhs)
{
	return m_MySQLRow == rhs.m_MySQLRow;
}

bool MySQLRequestRow::operator!=(const MySQLRequestRow& rhs)
{
	return m_MySQLRow != rhs.m_MySQLRow;
}

char* MySQLRequestRow::operator[](int index)
{
	return m_MySQLRow[index];
}
