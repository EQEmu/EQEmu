/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

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

#include "eqdb_res.h"

#include "common/global_define.h"
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
