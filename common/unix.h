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
#ifndef _WINDOWS

#ifndef __UNIX_H__
#define __UNIX_H__
#include <unistd.h>

typedef int SOCKET;

void Sleep(unsigned int x);
#ifndef __CYGWIN__
char* strupr(char* tmp);
char* strlwr(char* tmp);
#endif
#endif

#endif
