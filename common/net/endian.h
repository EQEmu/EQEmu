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
