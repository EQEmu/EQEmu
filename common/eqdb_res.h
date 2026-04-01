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
