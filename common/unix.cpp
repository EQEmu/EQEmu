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

#include "unix.h"

#include <cctype>
#include <cstring>

void Sleep(unsigned int x) {
	if (x > 0)
		usleep(x*1000);
}

char* strupr(char* tmp) {
	int l = strlen(tmp);
	for (int x = 0; x < l; x++) {
		tmp[x] = toupper(tmp[x]);
	}
	return tmp;
}

char* strlwr(char* tmp) {
	int l = strlen(tmp);
	for (int x = 0; x < l; x++) {
		tmp[x] = tolower(tmp[x]);
	}
	return tmp;
}

#endif /* !WIN32 */

