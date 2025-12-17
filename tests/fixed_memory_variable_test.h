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

#include "common/fixed_memory_variable_hash_set.h"
#include "cppunit/cpptest.h"

struct test_struct {
	char name[512];
};

class FixedMemoryVariableHashTest : public Test::Suite {
	typedef void(FixedMemoryVariableHashTest::*TestFunction)(void);
public:
	FixedMemoryVariableHashTest() {
		size_ = 1024 + 12 + 2008;
		data_ = new uint8[size_];
		memset(data_, 0, size_);
		TEST_ADD(FixedMemoryVariableHashTest::InitTest);
		TEST_ADD(FixedMemoryVariableHashTest::LoadTest);
		TEST_ADD(FixedMemoryVariableHashTest::InsertTest);
		TEST_ADD(FixedMemoryVariableHashTest::RetrieveTest);
		TEST_ADD(FixedMemoryVariableHashTest::InsertAgainTest);
		TEST_ADD(FixedMemoryVariableHashTest::RetrieveAgainTest);
		TEST_ADD(FixedMemoryVariableHashTest::InsertAgainFailTest);
		TEST_ADD(FixedMemoryVariableHashTest::RetrieveAgainFailTest);
	}
	~FixedMemoryVariableHashTest() {
		delete[] data_;
	}

	private:
	void InitTest() {
		EQ::FixedMemoryVariableHashSet<test_struct> hash(data_, size_, 501);
		TEST_ASSERT(!hash.exists(0));
		TEST_ASSERT(!hash.exists(501));
	}

	void LoadTest() {
		EQ::FixedMemoryVariableHashSet<test_struct> hash(data_, size_);
		TEST_ASSERT(!hash.exists(0));
		TEST_ASSERT(!hash.exists(501));
	}

	void InsertTest() {
		EQ::FixedMemoryVariableHashSet<test_struct> hash(data_, size_);
		test_struct test;
		memset(&test, 0, sizeof(test));
		strcpy(test.name, "Bill D.");

		hash.insert(0, reinterpret_cast<byte*>(&test), sizeof(test));
		TEST_ASSERT(hash.exists(0));
		TEST_ASSERT(!hash.exists(501));
	}

	void RetrieveTest() {
		EQ::FixedMemoryVariableHashSet<test_struct> hash(data_, size_);
		TEST_ASSERT(hash.exists(0));
		TEST_ASSERT(!hash.exists(501));

		test_struct test = hash[0];
		TEST_ASSERT(strcmp(test.name, "Bill D.") == 0);
	}

	void InsertAgainTest() {
		EQ::FixedMemoryVariableHashSet<test_struct> hash(data_, size_);
		test_struct test;
		memset(&test, 0, sizeof(test));
		strcpy(test.name, "Jimmy P.");

		hash.insert(501, reinterpret_cast<byte*>(&test), sizeof(test));
		TEST_ASSERT(hash.exists(0));
		TEST_ASSERT(hash.exists(501));
	}

	void RetrieveAgainTest() {
		EQ::FixedMemoryVariableHashSet<test_struct> hash(data_, size_);
		TEST_ASSERT(hash.exists(0));
		TEST_ASSERT(hash.exists(501));

		test_struct test = hash[501];
		TEST_ASSERT(strcmp(test.name, "Jimmy P.") == 0);
	}

	void InsertAgainFailTest() {
		EQ::FixedMemoryVariableHashSet<test_struct> hash(data_, size_);
		test_struct test;
		memset(&test, 0, sizeof(test));
		strcpy(test.name, "Tommy M.");

		try {
			hash.insert(500, reinterpret_cast<byte*>(&test), sizeof(test));
		} catch(std::exception&) { }

		TEST_ASSERT(hash.exists(0));
		TEST_ASSERT(hash.exists(501));
		TEST_ASSERT(!hash.exists(500));
	}

	void RetrieveAgainFailTest() {
		EQ::FixedMemoryVariableHashSet<test_struct> hash(data_, size_);
		TEST_ASSERT(hash.exists(0));
		TEST_ASSERT(hash.exists(501));
		TEST_ASSERT(!hash.exists(500));

		try {
			test_struct test = hash[500];
			TEST_ASSERT(false);
		} catch(std::exception&) { }
	}
private:
	uint8 *data_;
	uint32 size_;
};
