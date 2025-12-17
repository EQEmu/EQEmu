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

class StringUtilTest : public Test::Suite {
	typedef void(StringUtilTest::*TestFunction)(void);
public:
	StringUtilTest() {
		TEST_ADD(StringUtilTest::StringFormatTest);
		TEST_ADD(StringUtilTest::EscapeStringTest);
		TEST_ADD(StringUtilTest::SearchDeliminatedStringTest);
		TEST_ADD(StringUtilTest::SplitStringTest);
		TEST_ADD(StringUtilTest::TestIsFloat);
		TEST_ADD(StringUtilTest::TestIsNumber);
	}

	~StringUtilTest() {
	}

	private:
	void StringFormatTest() {
		const char* fmt = "Test: %c %d %4.2f";
		char c = 'a';
		int i = 2014;
		float f = 3.1416;

		auto s = StringFormat(fmt, c, i, f);
		TEST_ASSERT_EQUALS(s.length(), 17);
		TEST_ASSERT(s.compare("Test: a 2014 3.14") == 0);
	}

	void EscapeStringTest() {
		std::string t;
		t.resize(10);
		t[0] = 'a';
		t[1] = 'b';
		t[2] = 'c';
		t[3] = '\x00';
		t[4] = '\n';
		t[5] = '\r';
		t[6] = '\\';
		t[7] = '\'';
		t[8] = '\"';
		t[9] = '\x1a';

		auto s = Strings::Escape(t);
		TEST_ASSERT(s.compare("abc\\x00\\n\\r\\\\\\'\\\"\\x1a") == 0);
	}

	void SearchDeliminatedStringTest() {
		std::string h =
		    "befallen,charasis,dalnir,frontiermtns,gukbottom,iceclad,lakeofillomen,northkarana,qey2hh1,soldunga,southro,wakening,podisease,velketor,akheva,riwwi,bothunder,poair";
		TEST_ASSERT(Strings::SearchDelim(h, "befallen") == 0);
		TEST_ASSERT(Strings::SearchDelim(h, "charasis") == 9);
		TEST_ASSERT(Strings::SearchDelim(h, "dalnir") == 18);
		TEST_ASSERT(Strings::SearchDelim(h, "frontiermtns") == 25);
		TEST_ASSERT(Strings::SearchDelim(h, "gukbottom") == 38);
		TEST_ASSERT(Strings::SearchDelim(h, "iceclad") == 48);
		TEST_ASSERT(Strings::SearchDelim(h, "lakeofillomen") == 56);
		TEST_ASSERT(Strings::SearchDelim(h, "northkarana") == 70);
		TEST_ASSERT(Strings::SearchDelim(h, "qey2hh1") == 82);
		TEST_ASSERT(Strings::SearchDelim(h, "soldunga") == 90);
		TEST_ASSERT(Strings::SearchDelim(h, "southro") == 99);
		TEST_ASSERT(Strings::SearchDelim(h, "wakening") == 107);
		TEST_ASSERT(Strings::SearchDelim(h, "podisease") == 116);
		TEST_ASSERT(Strings::SearchDelim(h, "velketor") == 126);
		TEST_ASSERT(Strings::SearchDelim(h, "akheva") == 135);
		TEST_ASSERT(Strings::SearchDelim(h, "riwwi") == 142);
		TEST_ASSERT(Strings::SearchDelim(h, "bothunder") == 148);
		TEST_ASSERT(Strings::SearchDelim(h, "poair") == 158);
		TEST_ASSERT(Strings::SearchDelim(h, "pod") == std::string::npos);
		TEST_ASSERT(Strings::SearchDelim(h, "air") == std::string::npos);
		TEST_ASSERT(Strings::SearchDelim(h, "bef") == std::string::npos);
		TEST_ASSERT(Strings::SearchDelim(h, "wwi") == std::string::npos);
		TEST_ASSERT(Strings::SearchDelim(h, "bothunder,") == std::string::npos);
		TEST_ASSERT(Strings::SearchDelim(h, "poair,") == std::string::npos);
		TEST_ASSERT(Strings::SearchDelim(h, "befallen,") == std::string::npos);
		TEST_ASSERT(Strings::SearchDelim(h, "wwi,") == std::string::npos);
	}

	void SplitStringTest() {
		std::string s = "123,456,789,";
		auto v = Strings::Split(s, ',');
		TEST_ASSERT(v.size() == 3);
		TEST_ASSERT(v[0] == "123");
		TEST_ASSERT(v[1] == "456");
		TEST_ASSERT(v[2] == "789");
	}

	void TestIsFloat() {
		TEST_ASSERT_EQUALS(Strings::IsFloat("0.23424523"), true);
		TEST_ASSERT_EQUALS(Strings::IsFloat("12312312313.23424523"), true);
		TEST_ASSERT_EQUALS(Strings::IsFloat("12312312313"), true);
		TEST_ASSERT_EQUALS(Strings::IsFloat(".234234"), true);
		TEST_ASSERT_EQUALS(Strings::IsFloat(".234234f"), false);
		TEST_ASSERT_EQUALS(Strings::IsFloat("Johnson"), false);
		TEST_ASSERT_EQUALS(Strings::IsFloat("12312312313f"), false); // character at end
		TEST_ASSERT_EQUALS(Strings::IsFloat("18446744073709551616"), true);
		TEST_ASSERT_EQUALS(Strings::IsFloat("-18"), true);
		TEST_ASSERT_EQUALS(Strings::IsFloat("-f18"), false);
		TEST_ASSERT_EQUALS(Strings::IsFloat("-18446744073709551616"), true);
	}

	void TestIsNumber() {
		TEST_ASSERT_EQUALS(Strings::IsNumber("0.23424523"), false);
		TEST_ASSERT_EQUALS(Strings::IsNumber("12312312313.23424523"), false);
		TEST_ASSERT_EQUALS(Strings::IsNumber("12312312313"), true);
		TEST_ASSERT_EQUALS(Strings::IsNumber("12312312313f"), false); // character at end
		TEST_ASSERT_EQUALS(Strings::IsNumber("18446744073709551616"), true); // 64
		TEST_ASSERT_EQUALS(Strings::IsNumber("-18"), true);
		TEST_ASSERT_EQUALS(Strings::IsNumber("-f18"), false);
		TEST_ASSERT_EQUALS(Strings::IsNumber("-18446744073709551616"), true); // 64
		TEST_ASSERT_EQUALS(Strings::IsNumber("18446744073709551616f"), false); // 64
		TEST_ASSERT_EQUALS(Strings::IsNumber("18446744073709551616.0"), false); // 64
		TEST_ASSERT_EQUALS(Strings::IsNumber("18446744073709551616.0f"), false); // 64
	}
};
