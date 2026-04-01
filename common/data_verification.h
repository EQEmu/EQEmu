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

#include <algorithm>
#include <cmath>

namespace EQ {
	template<typename T>
	T Clamp(const T &value, const T &lower, const T &upper)
	{
		return std::max(lower, std::min(value, upper));
	}

	template<typename T>
	T ClampLower(const T &value, const T &lower)
	{
		return std::max(lower, value);
	}

	template<typename T>
	T ClampUpper(const T &value, const T &upper)
	{
		return std::min(value, upper);
	}

	template<typename T>
	bool ValueWithin(const T &value, const T &lower, const T &upper)
	{
		return value >= lower && value <= upper;
	}

	template<typename T1, typename T2, typename T3>
	bool ValueWithin(const T1 &value, const T2 &lower, const T3 &upper)
	{
		return value >= (T1) lower && value <= (T1) upper;
	}
} /*EQEmu*/

