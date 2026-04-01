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

#include "fastmath.h"

#include <numbers>

FastMath g_Math;

// This should match EQ's sin/cos LUTs
// Some values didn't match on linux, but they were the "same" float :P
FastMath::FastMath()
{
	int ci = 0;
	int si = 128;
	float res;
	do {
		res = std::cos(static_cast<float>(ci) * std::numbers::pi * 2 / 512);
		lut_cos[ci] = res;
		if (si == 512)
			si = 0;
		lut_sin[si] = res;
		++ci;
		++si;
	} while (ci < 512);

	lut_sin[0] = 0.0f;
	lut_sin[128] = 1.0f;
	lut_sin[256] = 0.0f;
	lut_sin[384] = -1.0f;

	lut_cos[0] = 1.0f;
	lut_cos[128] = 0.0f;
	lut_cos[384] = 0.0f;
}

