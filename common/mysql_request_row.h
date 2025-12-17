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
