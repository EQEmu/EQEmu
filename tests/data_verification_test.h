/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2014 EQEMu Development Team (http://eqemulator.net)

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

#include "common/data_verification.h"
#include "cppunit/cpptest.h"

class DataVerificationTest : public Test::Suite {
	typedef void(DataVerificationTest::*TestFunction)(void);
public:
	DataVerificationTest() {
		TEST_ADD(DataVerificationTest::Clamp);
		TEST_ADD(DataVerificationTest::ClampUpper);
		TEST_ADD(DataVerificationTest::ClampLower);
		TEST_ADD(DataVerificationTest::ValueWithin);
	}

	~DataVerificationTest() {
	}

	private:
	void Clamp() {
		float value_f = 500.0f;
		int value_i = 500;
		
		float vf1 = EQ::Clamp(value_f, 0.0f, 1000.0f);
		float vf2 = EQ::Clamp(value_f, 0.0f, 250.0f);
		float vf3 = EQ::Clamp(value_f, 750.0f, 1000.0f);
		
		int vi1 = EQ::Clamp(value_i, 0, 1000);
		int vi2 = EQ::Clamp(value_i, 0, 250);
		int vi3 = EQ::Clamp(value_i, 750, 1000);
		
		TEST_ASSERT_EQUALS(vf1, 500.0f);
		TEST_ASSERT_EQUALS(vf2, 250.0f);
		TEST_ASSERT_EQUALS(vf3, 750.0f);
		
		TEST_ASSERT_EQUALS(vi1, 500);
		TEST_ASSERT_EQUALS(vi2, 250);
		TEST_ASSERT_EQUALS(vi3, 750);
	}

	void ClampUpper() {
		float value_f = 500.0f;
		int value_i = 500;

		float vf1 = EQ::ClampUpper(value_f, 1000.0f);
		float vf2 = EQ::ClampUpper(value_f, 250.0f);

		int vi1 = EQ::ClampUpper(value_i, 1000);
		int vi2 = EQ::ClampUpper(value_i, 250);

		TEST_ASSERT_EQUALS(vf1, 500.0f);
		TEST_ASSERT_EQUALS(vf2, 250.0f);

		TEST_ASSERT_EQUALS(vi1, 500);
		TEST_ASSERT_EQUALS(vi2, 250);
	}

	void ClampLower() {
		float value_f = 500.0f;
		int value_i = 500;

		float vf1 = EQ::ClampLower(value_f, 0.0f);
		float vf2 = EQ::ClampLower(value_f, 750.0f);

		int vi1 = EQ::ClampLower(value_i, 0);
		int vi2 = EQ::ClampLower(value_i, 750);

		TEST_ASSERT_EQUALS(vf1, 500.0f);
		TEST_ASSERT_EQUALS(vf2, 750.0f);

		TEST_ASSERT_EQUALS(vi1, 500);
		TEST_ASSERT_EQUALS(vi2, 750);
	}
	
	void ValueWithin() {
		float value_f = 500.0f;
		int value_i = 500;

		TEST_ASSERT(EQ::ValueWithin(value_f, 0.0f, 1000.0f));
		TEST_ASSERT(!EQ::ValueWithin(value_f, 0.0f, 400.0f));
		TEST_ASSERT(!EQ::ValueWithin(value_f, 600.0f, 900.0f));
		
		TEST_ASSERT(EQ::ValueWithin(value_i, 0, 1000));
		TEST_ASSERT(!EQ::ValueWithin(value_i, 0, 400));
		TEST_ASSERT(!EQ::ValueWithin(value_i, 600, 900));
	}
};
