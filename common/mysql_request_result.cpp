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
#include "mysql_request_result.h"


MySQLRequestResult::MySQLRequestResult()
	: m_CurrentRow(), m_OneBeyondRow()
{
	ZeroOut();
}

MySQLRequestResult::MySQLRequestResult(MYSQL_RES* result, uint32 rowsAffected, uint32 rowCount, uint32 columnCount, uint32 lastInsertedID, uint32 errorNumber, char *errorBuffer)
	: m_CurrentRow(result), m_OneBeyondRow()
{
	m_Result = result;
	m_RowsAffected = rowsAffected;
	m_RowCount = rowCount;
	m_ColumnCount = columnCount;
	m_LastInsertedID = lastInsertedID;

	// If we actually need the column length / fields it will be
	// requested at that time, no need to pull it in just to cache it.
	// Normal usage would have it as nullptr most likely anyways.
	m_ColumnLengths = nullptr;
	m_Fields = nullptr;

	m_Success = true;
	if (errorBuffer != nullptr)
		m_Success = false;

	m_ErrorNumber = errorNumber;
	m_ErrorBuffer = errorBuffer;
}

void MySQLRequestResult::FreeInternals()
{

	safe_delete_array(m_ErrorBuffer);

	if (m_Result != nullptr)
		mysql_free_result(m_Result);

	ZeroOut();
}

void MySQLRequestResult::ZeroOut()
{
	m_Success = false;
	m_Result = nullptr;
	m_ErrorBuffer = nullptr;
	m_ColumnLengths = nullptr;
	m_Fields = nullptr;
	m_RowCount = 0;
	m_RowsAffected = 0;
	m_LastInsertedID = 0;
	m_error_message = "";
}

MySQLRequestResult::~MySQLRequestResult()
{
	FreeInternals();
}

uint32 MySQLRequestResult::LengthOfColumn(int columnIndex)
{
	if (m_ColumnLengths == nullptr && m_Result != nullptr)
		m_ColumnLengths = mysql_fetch_lengths(m_Result);

	// If someone screws up and tries to get the length of a
	// column when no result occured (check Success! argh!)
	// then we always return 0. Also applies if mysql screws
	// up and can't get the column lengths for whatever reason.
	if (m_ColumnLengths == nullptr)
		return 0;

	// Want to index check to be sure we don't read passed
	// the end of the array. Just default to 0 in that case.
	// We *shouldn't* need this or the previous checks if all
	// interface code is correctly written.
	if (columnIndex >= m_ColumnCount)
		return 0;

	return m_ColumnLengths[columnIndex];
}

const std::string MySQLRequestResult::FieldName(int columnIndex)
{
	if (columnIndex >= m_ColumnCount || m_Result == nullptr)
		return std::string();

	if (m_Fields == nullptr)
		m_Fields = mysql_fetch_fields(m_Result);

	return std::string(m_Fields[columnIndex].name);
}

MySQLRequestResult::MySQLRequestResult(MySQLRequestResult&& moveItem)
	: m_CurrentRow(moveItem.m_CurrentRow), m_OneBeyondRow()
{
	m_Result = moveItem.m_Result;
	m_ErrorBuffer = moveItem.m_ErrorBuffer;
	m_Success = moveItem.m_Success;
	m_RowCount = moveItem.m_RowCount;
	m_RowsAffected = moveItem.m_RowsAffected;
	m_LastInsertedID = moveItem.m_LastInsertedID;
	m_ColumnLengths = moveItem.m_ColumnLengths;
	m_ColumnCount = moveItem.m_ColumnCount;
	m_Fields = moveItem.m_Fields;

	// Keeps deconstructor from double freeing
	// pre move instance.
	moveItem.ZeroOut();
}

MySQLRequestResult& MySQLRequestResult::operator=(MySQLRequestResult&& other)
{
	// Assigning something to itself?
	// Silly! (but happens)
	if (this == &other)
		return *this;

	FreeInternals();

	m_Success = other.m_Success;

	m_Result = other.m_Result;
	m_ErrorBuffer = other.m_ErrorBuffer;

	m_RowCount = other.m_RowCount;
	m_RowsAffected = other.m_RowsAffected;
	m_LastInsertedID = other.m_LastInsertedID;
	m_CurrentRow = other.m_CurrentRow;
	m_OneBeyondRow = other.m_OneBeyondRow;
	m_ColumnLengths = other.m_ColumnLengths;
	m_ColumnCount = other.m_ColumnCount;
	m_Fields = other.m_Fields;

	// Keeps deconstructor from double freeing
	// pre move instance.
	other.ZeroOut();
	return *this;
}

uint32 MySQLRequestResult::GetErrorNumber() const
{
	return m_ErrorNumber;
}

void MySQLRequestResult::SetErrorNumber(uint32 m_error_number)
{
	m_ErrorNumber = m_error_number;
}

const std::string &MySQLRequestResult::GetErrorMessage() const
{
	return m_error_message;
}

void MySQLRequestResult::SetErrorMessage(const std::string &m_error_message)
{
	MySQLRequestResult::m_error_message = m_error_message;
}
