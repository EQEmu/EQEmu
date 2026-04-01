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
#include <cstdint>
#include <type_traits>

namespace EQ
{
	namespace Net
	{
		inline bool IsLittleEndian() {
			const int32_t v = 1;
			return 1 == *(int8_t*)&v;
		}

		template<typename T>
		T ByteSwap(T in) {
			static_assert(std::is_integral<T>::value, "Byte swap only works on integer types.");
			T ret;

			char *first = (char*)&in;
			char *last = (char*)&in + sizeof(in);
			char *d_first = (char*)&ret;
			while (first != last) {
				*(d_first++) = *(--last);
			}

			return ret;
		}

		template<typename T>
		T HostToNetwork(T in) {
			if (IsLittleEndian()) {
				return ByteSwap(in);
			}
			else {
				return in;
			}
		}

		template<typename T>
		T NetworkToHost(T in) {
			if (IsLittleEndian()) {
				return ByteSwap(in);
			}
			else {
				return in;
			}
		}
	}
}
