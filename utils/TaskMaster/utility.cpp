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
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "utility.h"

#if (_MSC_VER < 1500)
	#define vsnprintf	_vsnprintf
#endif

#define safe_delete(d) if(d) { delete d; d=0; }
#define safe_delete_array(d) if(d) { delete[] d; d=0; }

int MakeAnyLenString(char** ret, const char* format, ...) {
	int buf_len = 128;
    int chars = -1;
	va_list argptr;
	va_start(argptr, format);
	while (chars == -1 || chars >= buf_len) {
		safe_delete_array(*ret);
		if (chars == -1)
			buf_len *= 2;
		else
			buf_len = chars + 1;
		*ret = new char[buf_len];
		chars = vsnprintf(*ret, buf_len, format, argptr);
	}
	va_end(argptr);
	return chars;
}