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
#ifndef EQWX_ERRORLOG__H
#define EQWX_ERRORLOG__H

#include <stdio.h>
#include <stdarg.h>

//Log options
enum{
	eqEmuLogConsole = 1,
	eqEmuLogFile = 2,
	eqEmuLogSQL = 4,
	eqEmuLogBoth = (eqEmuLogConsole | eqEmuLogFile),
};

//log file output
#define LOG_FILE_NAME "debug.txt"
#define SQL_LOG_FILE_NAME "sql_log.sql"

//max single log length
//same as a client:message() max len
#define MAX_LOG_LEN 4096

//much simplified logging function, similar to the one used on the server (Tho much simpler)
class EQEmuErrorLog {
public:

	EQEmuErrorLog();
	~EQEmuErrorLog();
	void Log(unsigned int mOutputType, const char *msg, ...);
private:
	FILE* mErrorLog;
	FILE* mErrorLogSQL;
};

#endif