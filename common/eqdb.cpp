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

#include "eqdb.h"

#include "common/database.h"
#include "common/global_define.h"

#include "mysql.h"
#include <cstring>

EQDB EQDB::s_EQDB;

unsigned int EQDB::field_count() {
	return mysql_field_count(mysql_ref);
}

unsigned long EQDB::affected_rows() {
	return mysql_affected_rows(mysql_ref);
}

unsigned long EQDB::insert_id() {
	return mysql_insert_id(mysql_ref);
}

unsigned int EQDB::get_errno() {
	return mysql_errno(mysql_ref);
}

Const_char * EQDB::error() {
	return mysql_error(mysql_ref);
}

EQDBRes * EQDB::query(Const_char *q) {
	if (mysql_real_query(mysql_ref,q,strlen(q))==0) {
		if (mysql_field_count(mysql_ref)) {
			MYSQL_RES *r=mysql_store_result(mysql_ref);
			return new EQDBRes(r);
		} else {
			//no result, give them back a 'true but empty' result set
			return(new EQDBRes(nullptr));
		}
	}

	return nullptr;
}

//NOT THREAD SAFE!
Const_char *EQDB::escape_string(Const_char *from) {
	int len = strlen(from);
	auto res = new char[len * 2 + 1];

	mysql_real_escape_string(mysql_ref,res,from,len);

	res[len*2] = '\0';
	m_escapeBuffer = res;
	delete[] res;
	return(m_escapeBuffer.c_str());
}

