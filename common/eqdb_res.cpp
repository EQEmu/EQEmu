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
#include "eqdb_res.h"

#include "mysql.h"

std::vector<std::string> EQDBRes::fetch_row_array() {
	std::vector<std::string> array;
	if(res == nullptr)
		return(array);

	int count=mysql_num_fields(res);
	MYSQL_ROW row=mysql_fetch_row(res);
	for (int i=0;i<count;i++)
		array.push_back(row[i]);

	return array;
}

std::map<std::string,std::string> EQDBRes::fetch_row_hash() {
	std::map<std::string,std::string> rowhash;
	if(res == nullptr)
		return(rowhash);

	MYSQL_FIELD *fields;
	MYSQL_ROW row;
	unsigned long num_fields,i;

	if (res && (num_fields=mysql_num_fields(res)) && (row = mysql_fetch_row(res))!=nullptr && (fields = mysql_fetch_fields(res))!=nullptr) {
		for(i=0;i<num_fields;i++) {
			rowhash[fields[i].name]=(row[i] ? row[i] : "");
		}
	}

	return rowhash;
}
