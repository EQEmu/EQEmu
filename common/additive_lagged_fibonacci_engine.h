/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2021 EQEMu Development Team (http://eqemulator.net)

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

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <type_traits>

/*
 * This is an additive lagged fibonacci generator as seen in The Art of Computer Programming, Vol. 2
 * This should roughly match the implementation that EQ's client uses and be compatible with our Random class
 *
 * EQ's rand looks like it was from an example implementation that as posted on pscode.com
 *
 * You might also want to consider defining BIASED_INT_DIST as well to more closely match EQ
 */

namespace EQ {
	template<typename UIntType, size_t w, size_t j, size_t k>
	class additive_lagged_fibonacci_engine {
		static_assert(std::is_unsigned<UIntType>::value, "result_type must be an unsigned integral type");
		static_assert(0u < j && j < k, "0 < j < k");
		static_assert(0u < w && w <= std::numeric_limits<UIntType>::digits,
			      "template argument substituting w out of bounds");

	public:
		using result_type = UIntType;
		static constexpr size_t word_size = w;
		static constexpr size_t short_lag = j;
		static constexpr size_t long_lag = k;
		static constexpr result_type default_seed = 19780503u; // default for subtract_with_carry_engine

		additive_lagged_fibonacci_engine() : additive_lagged_fibonacci_engine(default_seed) {}

		explicit additive_lagged_fibonacci_engine(result_type sd) { seed(sd); }

		void seed(result_type seed = default_seed)
		{
			state1 = long_lag - long_lag;
			state2 = long_lag - short_lag;
			state[0] = static_cast<int>(seed) & ((1u << word_size) - 1);
			state[1] = 1;
			for (int i = 2; i < long_lag; ++i)
				state[i] = (state[i - 1] + state[i - 2]) & ((1u << word_size) - 1);
			return;
			}
			// TODO: seed via seed_seq

			static constexpr result_type min() { return 0; }
			static constexpr result_type max() { return ((1u << word_size) - 1) >> 6; }

			void discard(unsigned long long z) {
				for (; z != 0ULL; --z)
					(*this)();
			}

			result_type operator()() {
				result_type rand = (state[state1] + state[state2]) & ((1u << word_size) - 1);
				state[state1] = rand;
				if (++state1 == long_lag)
					state1 = 0;
				if (++state2 == long_lag)
					state2 = 0;

				return rand >> 6;
			}

		private:
			result_type state1;
			result_type state2;
			result_type state[long_lag];

		public:
			template<typename UInt, size_t W, size_t J, size_t K>
			friend bool operator==(const additive_lagged_fibonacci_engine<UInt, W, J, K> &x,
					const additive_lagged_fibonacci_engine<UInt, W, J, K> &y)
			{
				return std::equal(x.state, x.state + long_lag, y.state) && x.state1 == y.state1 &&
				       x.state2 == y.state2;
			}

			template<typename UInt, size_t W, size_t J, size_t K>
			friend bool operator!=(const additive_lagged_fibonacci_engine<UInt, W, J, K> &x,
					const additive_lagged_fibonacci_engine<UInt, W, J, K> &y)
			{ return !(x == y); }

			template<typename UInt, size_t W, size_t J, size_t K, typename CharT, typename Traits>
			friend std::basic_ostream<CharT, Traits> &
			operator<<(std::basic_istream<CharT, Traits> &os, additive_lagged_fibonacci_engine<UInt, W, J, K> &x)
			{
				using ios_base = typename std::basic_istream<CharT, Traits>::ios_base;

				const typename ios_base::fmtflags flags = os.flags();
				const CharT fill = os.fill();
				const CharT space = os.widen(' ');
				os.flags(ios_base::dec | ios_base::fixed | ios_base::left);
				os.fill(space);

				for (size_t i = 0; i < long_lag; ++i)
					os << x.state[i] << space;
				os << x.state1 << space << x.state2;

				os.flags(flags);
				os.fill(fill);
				return os;
			}

			template<typename UInt, size_t W, size_t J, size_t K, typename CharT, typename Traits>
			friend std::basic_istream<CharT, Traits> &
			operator>>(std::basic_istream<CharT, Traits> &is, additive_lagged_fibonacci_engine<UInt, W, J, K> &x)
			{
				using ios_base = typename std::basic_istream<CharT, Traits>::ios_base;

				const typename ios_base::fmtflags flags = is.flags();
				is.flags(ios_base::dec | ios_base::skipws);

				for (size_t i = 0; i < long_lag; ++i)
					is >> x.state[i];
				is >> x.state1;
				is >> x.state2;

				is.flags(flags);
				return is;
			}
	};

	using EQRand = additive_lagged_fibonacci_engine<uint32_t, 30, 24, 55>;
};

