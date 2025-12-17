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

#include "common/database.h"
#include "common/types.h"

#include "mysql.h"
#include <string>
#include <vector>
#include <map>

//this is the main object exported to perl.
class EQDBRes {
public:
	EQDBRes(MYSQL_RES *r) { res=r; }
	~EQDBRes()	{ finish(); }

//BEGIN PERL EXPORT
	unsigned long	num_rows() { return (res) ? mysql_num_rows(res) : 0; }
	unsigned long	num_fields() { return (res) ? mysql_num_fields(res) : 0; }
	void		DESTROY() { }
	void		finish() { if (res) mysql_free_result(res); res=nullptr; };
	std::vector<std::string>	fetch_row_array();
	std::map<std::string,std::string> fetch_row_hash();
	unsigned long *	fetch_lengths() { return (res) ? mysql_fetch_lengths(res) : 0; }
//END PERL EXPORT

private:
	MYSQL_RES *res;
};
