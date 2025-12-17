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

#pragma once

#include "common/eqdb_res.h"
#include "common/types.h"

#include "mysql.h"
#include <map>
#include <string>
#include <vector>

//this is the main object exported to perl.
class EQDB {
	EQDB() = default;
public:
	static EQDB *Singleton() { return(&s_EQDB); }

	static void SetMySQL(MYSQL *m) { s_EQDB.mysql_ref=m; }

//BEGIN PERL EXPORT
	//NOTE: you must have a space after the * of a return value

	unsigned int field_count();
	unsigned long affected_rows();
	unsigned long insert_id();
	unsigned int get_errno();
	Const_char * error();
	EQDBRes * query(Const_char *q);
	Const_char * escape_string(Const_char *from);		//NOT THREAD SAFE! (m_escapeBuffer)
//END PERL EXPORT

private:
	std::string m_escapeBuffer;
	static EQDB s_EQDB;
	MYSQL *mysql_ref;
};
