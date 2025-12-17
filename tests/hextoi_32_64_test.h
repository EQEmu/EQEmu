/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2013 EQEMu Development Team (http://eqemulator.net)

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

#include "common/strings.h"
#include "cppunit/cpptest.h"

class hextoi_32_64_Test : public Test::Suite {
	typedef void(hextoi_32_64_Test::*TestFunction)(void);
public:
	hextoi_32_64_Test() {
		TEST_ADD(hextoi_32_64_Test::nullptr32Test);
		TEST_ADD(hextoi_32_64_Test::ShortString32Test);
		TEST_ADD(hextoi_32_64_Test::SingleDigitUpper32Test);
		TEST_ADD(hextoi_32_64_Test::SingleDigitLower32Test);
		TEST_ADD(hextoi_32_64_Test::DoubleDigitUpper32Test);
		TEST_ADD(hextoi_32_64_Test::DoubleDigitLower32Test);

		TEST_ADD(hextoi_32_64_Test::nullptr64Test);
		TEST_ADD(hextoi_32_64_Test::ShortString64Test);
		TEST_ADD(hextoi_32_64_Test::SingleDigitUpper64Test);
		TEST_ADD(hextoi_32_64_Test::SingleDigitLower64Test);
		TEST_ADD(hextoi_32_64_Test::DoubleDigitUpper64Test);
		TEST_ADD(hextoi_32_64_Test::DoubleDigitLower64Test);
	}

	~hextoi_32_64_Test() {
	}

	private:

	void nullptr32Test() {
		TEST_ASSERT(hextoi(nullptr) == 0);
	}

	void ShortString32Test() {
		// if the string is too short then it should
		// spit out a zero.
		// strings should be formatted: 0x** or 0X**
		TEST_ASSERT(hextoi("") == 0);
		TEST_ASSERT(hextoi("0") == 0);
		TEST_ASSERT(hextoi("01") == 0);
	}

	void SingleDigitUpper32Test() {
		TEST_ASSERT(hextoi("0x0") == 0);
		TEST_ASSERT(hextoi("0x1") == 1);
		TEST_ASSERT(hextoi("0x2") == 2);
		TEST_ASSERT(hextoi("0x3") == 3);
		TEST_ASSERT(hextoi("0x4") == 4);
		TEST_ASSERT(hextoi("0x5") == 5);
		TEST_ASSERT(hextoi("0x6") == 6);
		TEST_ASSERT(hextoi("0x7") == 7);
		TEST_ASSERT(hextoi("0x8") == 8);
		TEST_ASSERT(hextoi("0x9") == 9);
		TEST_ASSERT(hextoi("0xA") == 10);
		TEST_ASSERT(hextoi("0xB") == 11);
		TEST_ASSERT(hextoi("0xC") == 12);
		TEST_ASSERT(hextoi("0xD") == 13);
		TEST_ASSERT(hextoi("0xE") == 14);
		TEST_ASSERT(hextoi("0xF") == 15);
	}

	void SingleDigitLower32Test() {
		TEST_ASSERT(hextoi("0x0") == 0);
		TEST_ASSERT(hextoi("0x1") == 1);
		TEST_ASSERT(hextoi("0x2") == 2);
		TEST_ASSERT(hextoi("0x3") == 3);
		TEST_ASSERT(hextoi("0x4") == 4);
		TEST_ASSERT(hextoi("0x5") == 5);
		TEST_ASSERT(hextoi("0x6") == 6);
		TEST_ASSERT(hextoi("0x7") == 7);
		TEST_ASSERT(hextoi("0x8") == 8);
		TEST_ASSERT(hextoi("0x9") == 9);
		TEST_ASSERT(hextoi("0xa") == 10);
		TEST_ASSERT(hextoi("0xb") == 11);
		TEST_ASSERT(hextoi("0xc") == 12);
		TEST_ASSERT(hextoi("0xd") == 13);
		TEST_ASSERT(hextoi("0xe") == 14);
		TEST_ASSERT(hextoi("0xf") == 15);
	}

	// A bit excessive to do an exhaustive test like this
	// but it usefully tests multi digit hex.
	void DoubleDigitUpper32Test() {

		std::string prepend = "0x";
		std::string hexToTest;

		std::string hexElements = "0123456789ABCDEF";
		uint32 value = 0;
		for (std::string::iterator firstDigitIter = hexElements.begin(); firstDigitIter != hexElements.end(); ++firstDigitIter) {
			for (std::string::iterator secondDigitIter = hexElements.begin(); secondDigitIter != hexElements.end(); ++secondDigitIter) {
				std::string hexToTest = prepend + *firstDigitIter + *secondDigitIter;
				TEST_ASSERT(hextoi(hexToTest.c_str()) == value);
				value++;
			}
		}
	}

	// A bit excessive to do an exhaustive test like this
	// but it usefully tests multi digit hex.
	void DoubleDigitLower32Test() {
		std::string prepend = "0x";
		std::string hexToTest;

		std::string hexElements = "0123456789abcdef";
		uint32 value = 0;
		for (std::string::iterator firstDigitIter = hexElements.begin(); firstDigitIter != hexElements.end(); ++firstDigitIter) {
			for (std::string::iterator secondDigitIter = hexElements.begin(); secondDigitIter != hexElements.end(); ++secondDigitIter) {
				std::string hexToTest = prepend + *firstDigitIter + *secondDigitIter;
				TEST_ASSERT(hextoi(hexToTest.c_str()) == value);
				value++;
			}
		}
	}


	void nullptr64Test() {
		TEST_ASSERT(hextoi64(nullptr) == 0);
	}

	void ShortString64Test() {
		// if the string is too short then it should
		// spit out a zero.
		// strings should be formatted: 0x** or 0X**
		TEST_ASSERT(hextoi64("") == 0);
		TEST_ASSERT(hextoi64("0") == 0);
		TEST_ASSERT(hextoi64("01") == 0);
	}

	void SingleDigitUpper64Test() {
		std::string prepend = "0x";
		std::string hexToTest;

		std::string hexElements = "0123456789ABCDEF";
		uint64 value = 0;
		for (std::string::iterator firstDigitIter = hexElements.begin(); firstDigitIter != hexElements.end(); ++firstDigitIter) {
			std::string hexToTest = prepend + *firstDigitIter;
			TEST_ASSERT(hextoi64(hexToTest.c_str()) == value);
			value++;
		}
	}

	void SingleDigitLower64Test() {

		std::string prepend = "0x";
		std::string hexToTest;

		std::string hexElements = "0123456789abcdef";
		uint64 value = 0;
		for (std::string::iterator firstDigitIter = hexElements.begin(); firstDigitIter != hexElements.end(); ++firstDigitIter) {
			std::string hexToTest = prepend + *firstDigitIter;
			TEST_ASSERT(hextoi64(hexToTest.c_str()) == value);
			value++;
		}
	}

	// A bit excessive to do an exhaustive test like this
	// but it usefully tests multi digit hex.
	void DoubleDigitUpper64Test() {

		std::string prepend = "0x";
		std::string hexToTest;

		std::string hexElements = "0123456789ABCDEF";
		uint64 value = 0;
		for (std::string::iterator firstDigitIter = hexElements.begin(); firstDigitIter != hexElements.end(); ++firstDigitIter) {
			for (std::string::iterator secondDigitIter = hexElements.begin(); secondDigitIter != hexElements.end(); ++secondDigitIter) {
				std::string hexToTest = prepend + *firstDigitIter + *secondDigitIter;
				TEST_ASSERT(hextoi64(hexToTest.c_str()) == value);
				value++;
			}
		}
	}

	// A bit excessive to do an exhaustive test like this
	// but it usefully tests multi digit hex.
	void DoubleDigitLower64Test() {
		std::string prepend = "0x";
		std::string hexToTest;

		std::string hexElements = "0123456789abcdef";
		uint64 value = 0;
		for (std::string::iterator firstDigitIter = hexElements.begin(); firstDigitIter != hexElements.end(); ++firstDigitIter) {
			for (std::string::iterator secondDigitIter = hexElements.begin(); secondDigitIter != hexElements.end(); ++secondDigitIter) {
				std::string hexToTest = prepend + *firstDigitIter + *secondDigitIter;
				TEST_ASSERT(hextoi64(hexToTest.c_str()) == value);
				value++;
			}
		}
	}

};
