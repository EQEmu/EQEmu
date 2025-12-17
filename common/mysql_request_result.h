#pragma once

#include "common/mysql_request_row.h"
#include "common/types.h"

#include "mysql.h"
#include <string>

class MySQLRequestResult {
private:
	MYSQL_RES* m_Result;
	MYSQL_FIELD* m_Fields;
	char* m_ErrorBuffer;
	unsigned long* m_ColumnLengths;
	MySQLRequestRow m_CurrentRow;
	MySQLRequestRow m_OneBeyondRow;

	bool m_Success;
	uint32 m_RowsAffected;
	uint32 m_RowCount;
	uint32 m_ColumnCount;
	uint32 m_LastInsertedID;
	uint32 m_ErrorNumber;

	std::string m_error_message;

public:

	MySQLRequestResult(MYSQL_RES* result, uint32 rowsAffected = 0, uint32 rowCount = 0, uint32 columnCount = 0, uint32 lastInsertedID = 0, uint32 errorNumber = 0, char *errorBuffer = nullptr);
	MySQLRequestResult();
	MySQLRequestResult(MySQLRequestResult&& moveItem);
	~MySQLRequestResult();

	MySQLRequestResult& operator=(MySQLRequestResult&& other);

	bool Success() const { return m_Success;}
	std::string ErrorMessage() const {
		if (!m_error_message.empty()) {
			return m_error_message;
		}

		return m_ErrorBuffer ? std::string(m_ErrorBuffer) : std::string("");
	}
	uint32 ErrorNumber() const {return m_ErrorNumber;}
	uint32 RowsAffected() const {return m_RowsAffected;}
	uint32 RowCount() const {return m_RowCount;}
	uint32 ColumnCount() const {return m_ColumnCount;}
	uint32 LastInsertedID() const {return m_LastInsertedID;}
	// default to 0 index since we mostly use it that way anyways.
	uint32 LengthOfColumn(int columnIndex = 0);
	const std::string FieldName(int columnIndex);

	MySQLRequestRow& begin() { return m_CurrentRow; }
	MySQLRequestRow& end() { return m_OneBeyondRow; }

	uint32 GetErrorNumber() const;
	void SetErrorNumber(uint32 m_error_number);
	const std::string &GetErrorMessage() const;
	void SetErrorMessage(const std::string &m_error_message);

private:
	void FreeInternals();
	void ZeroOut();
};
