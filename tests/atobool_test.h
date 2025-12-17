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

class atoboolTest : public Test::Suite {
	typedef void(atoboolTest::*TestFunction)(void);
public:
	atoboolTest() {
		TEST_ADD(atoboolTest::TrueTest);
		TEST_ADD(atoboolTest::FalseTest);
		TEST_ADD(atoboolTest::YesTest);
		TEST_ADD(atoboolTest::NoTest);
		TEST_ADD(atoboolTest::OnTest);
		TEST_ADD(atoboolTest::OffTest);
		TEST_ADD(atoboolTest::EnableTest);
		TEST_ADD(atoboolTest::DisableTest);
		TEST_ADD(atoboolTest::EnabledTest);
		TEST_ADD(atoboolTest::DisabledTest);
		TEST_ADD(atoboolTest::YTest);
		TEST_ADD(atoboolTest::NTest);
		TEST_ADD(atoboolTest::nullptrTest);
	}

	~atoboolTest() {
	}

	private:

	void TrueTest() {
		TEST_ASSERT(atobool("true"));
	}

	void FalseTest() {
		TEST_ASSERT(!atobool("false"));
	}

	void YesTest() {
		TEST_ASSERT(atobool("yes"));
	}

	void NoTest() {
		TEST_ASSERT(!atobool("no"));
	}

	void OnTest() {
		TEST_ASSERT(atobool("on"));
	}

	void OffTest() {
		TEST_ASSERT(!atobool("off"));
	}

	void EnableTest() {
		TEST_ASSERT(atobool("enable"));
	}

	void DisableTest() {
		TEST_ASSERT(!atobool("disable"));
	}

	void EnabledTest() {
		TEST_ASSERT(atobool("enabled"));
	}

	void DisabledTest() {
		TEST_ASSERT(!atobool("disabled"));
	}

	void YTest() {
		TEST_ASSERT(atobool("y"));
	}

	void NTest() {
		TEST_ASSERT(!atobool("n"));
	}

	void nullptrTest() {
		TEST_ASSERT(!atobool(nullptr));
	}

};
