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

#include <random>
#include <utility>
#include <algorithm>
#include <iterator>
#include <type_traits>

#ifdef USE_SFMT19937
// only GCC supports, so lets turn it off
#ifndef __GNUC__
#undef USE_SFMT19937
#endif
#ifdef __clang__
#undef USE_SFMT19937
#endif
#endif

#ifdef USE_ADDITIVE_LFIB_PRNG
#include "common/additive_lagged_fibonacci_engine.h"
#elif defined(USE_SFMT19937)
#include <ext/random>
#endif

/* This uses mt19937 (or other compatible engine) seeded with the std::random_device. The idea is to have this be
 * included as a member of another class so mocking out for testing is easier If you need to reseed random.Reseed()
 * Eventually this should be derived from an abstract base class
 */

namespace EQ {
	class Random {
	public:
		// AKA old MakeRandomInt
		int Int(int low, int high)
		{
			if (low > high)
				std::swap(low, high);
// EQ uses biased int distribution, so I guess we can support it :P
#ifdef BIASED_INT_DIST
			return low + m_gen() % (high - low + 1);
#else
			return int_dist(m_gen, int_param_t(low, high)); // [low, high]
#endif
		}

		// AKA old MakeRandomFloat
		double Real(double low, double high)
		{
			if (low > high)
				std::swap(low, high);
			return real_dist(m_gen, real_param_t(low, high)); // [low, high)
		}

		// example Roll(50) would have a 50% success rate
		// Roll(100) 100%, etc
		// valid values 0-100 (well, higher works too but ...)
		bool Roll(const int required)
		{
			return Int(0, 99) < required;
		}

		// valid values 0.0 - 1.0
		bool Roll(const double required)
		{
			return Real(0.0, 1.0) <= required;
		}

		// same range as client's roll0
		// This is their main high level RNG function
		int Roll0(int max)
		{
			if (max - 1 > 0)
				return Int(0, max - 1);
			return 0;
		}

		// std::shuffle requires a RNG engine passed to it, so lets provide a wrapper to use our engine
		template<typename RandomAccessIterator>
		void Shuffle(RandomAccessIterator first, RandomAccessIterator last)
		{
			static_assert(std::is_same<std::random_access_iterator_tag,
					typename std::iterator_traits<RandomAccessIterator>::iterator_category>::value,
					"EQ::Random::Shuffle requires random access iterators");
			std::shuffle(first, last, m_gen);
		}

		void Reseed()
		{
			// We could do the seed_seq thing here too if we need better seeding
			// but that is mostly overkill for us, so just seed once
			std::random_device rd;
			m_gen.seed(rd());
		}

		Random()
		{
			Reseed();
		}

		static Random* Instance()
		{
			static Random instance;
			return &instance;
		}

	private:
#ifndef BIASED_INT_DIST
		typedef std::uniform_int_distribution<int>::param_type int_param_t;
		std::uniform_int_distribution<int> int_dist;
#endif
		typedef std::uniform_real_distribution<double>::param_type real_param_t;
		std::uniform_real_distribution<double> real_dist;
// define USE_CUSTOM_PRNG_ENGINE to any supported random engine like:
// #define USE_CUSTOM_PRNG_ENGINE std::default_random_engine
#ifdef USE_ADDITIVE_LFIB_PRNG
		EQRand
#elif defined(USE_SFMT19937)
		__gnu_cxx::sfmt19937
#elif defined(USE_CUSTOM_PRNG_ENGINE)
		USE_CUSTOM_PRNG_ENGINE
#else
		std::mt19937
#endif
		m_gen;
	};
}
