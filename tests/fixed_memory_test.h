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

#include "common/fixed_memory_hash_set.h"
#include "common/item_instance.h"
#include "cppunit/cpptest.h"

class FixedMemoryHashTest : public Test::Suite {
	typedef void(FixedMemoryHashTest::*TestFunction)(void);
public:
	FixedMemoryHashTest() {
		size_ = EQ::FixedMemoryHashSet<EQ::ItemData>::estimated_size(72000, 190000);
		data_ = new uint8[size_];
		memset(data_, 0, size_);
		TEST_ADD(FixedMemoryHashTest::InitTest);
		TEST_ADD(FixedMemoryHashTest::LoadTest);
		TEST_ADD(FixedMemoryHashTest::InsertTest);
		TEST_ADD(FixedMemoryHashTest::RetrieveTest);
		TEST_ADD(FixedMemoryHashTest::OverwriteTest);
		TEST_ADD(FixedMemoryHashTest::OverwriteRetrieveTest);
		TEST_ADD(FixedMemoryHashTest::InsertAgainTest);
		TEST_ADD(FixedMemoryHashTest::RetrieveAgainTest);
		TEST_ADD(FixedMemoryHashTest::InsertBeginTest);
		TEST_ADD(FixedMemoryHashTest::RetrieveBeginTest);
		TEST_ADD(FixedMemoryHashTest::InsertEndTest);
		TEST_ADD(FixedMemoryHashTest::RetrieveEndTest);
	}
	~FixedMemoryHashTest() {
		delete[] data_;
	}

	private:
	void InitTest() {
		EQ::FixedMemoryHashSet<EQ::ItemData> hash(data_, size_, 72000, 190000);
		TEST_ASSERT(!hash.exists(1001));
		TEST_ASSERT(hash.size() == 0);
		TEST_ASSERT(hash.max_size() == 72000);
		TEST_ASSERT(hash.empty());
	}

	void LoadTest() {
		EQ::FixedMemoryHashSet<EQ::ItemData> hash(data_, size_);
		TEST_ASSERT(!hash.exists(1001));
		TEST_ASSERT(hash.size() == 0);
		TEST_ASSERT(hash.max_size() == 72000);
		TEST_ASSERT(hash.empty());
	}

	void InsertTest() {
		EQ::FixedMemoryHashSet<EQ::ItemData> hash(data_, size_);
		EQ::ItemData item;
		memset(&item, 0, sizeof(item));
		strcpy(item.Name, "Iron Sword");
		item.ID = 1001;
		hash.insert(1001, item);

		TEST_ASSERT(hash.exists(1001));
		TEST_ASSERT(hash.size() == 1);
		TEST_ASSERT(hash.max_size() == 72000);
		TEST_ASSERT(!hash.empty());
	}

	void RetrieveTest() {
		EQ::FixedMemoryHashSet<EQ::ItemData> hash(data_, size_);
		TEST_ASSERT(hash.exists(1001));
		TEST_ASSERT(hash.size() == 1);
		TEST_ASSERT(hash.max_size() == 72000);
		TEST_ASSERT(!hash.empty());

		EQ::ItemData item = hash[1001];
		TEST_ASSERT(strcmp(item.Name, "Iron Sword") == 0);
		TEST_ASSERT(item.ID == 1001);
	}

	void OverwriteTest() {
		EQ::FixedMemoryHashSet<EQ::ItemData> hash(data_, size_);
		EQ::ItemData item;
		memset(&item, 0, sizeof(item));
		strcpy(item.Name, "Steel Sword");
		item.ID = 1001;
		hash.insert(1001, item);

		TEST_ASSERT(hash.exists(1001));
		TEST_ASSERT(hash.size() == 1);
		TEST_ASSERT(hash.max_size() == 72000);
		TEST_ASSERT(!hash.empty());
	}

	void OverwriteRetrieveTest() {
		EQ::FixedMemoryHashSet<EQ::ItemData> hash(data_, size_);
		TEST_ASSERT(hash.exists(1001));
		TEST_ASSERT(hash.size() == 1);
		TEST_ASSERT((hash.max_size() == 72000));
		TEST_ASSERT(!hash.empty());

		EQ::ItemData item = hash[1001];
		TEST_ASSERT(strcmp(item.Name, "Steel Sword") == 0);
		TEST_ASSERT(item.ID == 1001);
	}

	void InsertAgainTest() {
		EQ::FixedMemoryHashSet<EQ::ItemData> hash(data_, size_);
		EQ::ItemData item;
		memset(&item, 0, sizeof(item));
		strcpy(item.Name, "Iron Sword");
		item.ID = 1000;
		hash.insert(1000, item);

		TEST_ASSERT(hash.exists(1000));
		TEST_ASSERT(hash.exists(1001));
		TEST_ASSERT(hash.size() == 2);
		TEST_ASSERT(hash.max_size() == 72000);
		TEST_ASSERT(!hash.empty());
	}

	void RetrieveAgainTest() {
		EQ::FixedMemoryHashSet<EQ::ItemData> hash(data_, size_);
		TEST_ASSERT(hash.exists(1000));
		TEST_ASSERT(hash.exists(1001));
		TEST_ASSERT(hash.size() == 2);
		TEST_ASSERT(hash.max_size() == 72000);
		TEST_ASSERT(!hash.empty());

		EQ::ItemData item = hash[1000];
		TEST_ASSERT(strcmp(item.Name, "Iron Sword") == 0);
		TEST_ASSERT(item.ID == 1000);

		item = hash[1001];
		TEST_ASSERT(strcmp(item.Name, "Steel Sword") == 0);
		TEST_ASSERT(item.ID == 1001);
	}

	void InsertBeginTest() {
		EQ::FixedMemoryHashSet<EQ::ItemData> hash(data_, size_);
		EQ::ItemData item;
		memset(&item, 0, sizeof(item));
		strcpy(item.Name, "Bronze Sword");
		item.ID = 0;
		hash.insert(0, item);

		TEST_ASSERT(hash.exists(1000));
		TEST_ASSERT(hash.exists(1001));
		TEST_ASSERT(hash.exists(0));
		TEST_ASSERT(hash.size() == 3);
		TEST_ASSERT(hash.max_size() == 72000);
		TEST_ASSERT(!hash.empty());
	}

	void RetrieveBeginTest() {
		EQ::FixedMemoryHashSet<EQ::ItemData> hash(data_, size_);
		TEST_ASSERT(hash.exists(1000));
		TEST_ASSERT(hash.exists(1001));
		TEST_ASSERT(hash.exists(0));
		TEST_ASSERT(hash.size() == 3);
		TEST_ASSERT(hash.max_size() == 72000);
		TEST_ASSERT(!hash.empty());

		EQ::ItemData item = hash[1000];
		TEST_ASSERT(strcmp(item.Name, "Iron Sword") == 0);
		TEST_ASSERT(item.ID == 1000);

		item = hash[1001];
		TEST_ASSERT(strcmp(item.Name, "Steel Sword") == 0);
		TEST_ASSERT(item.ID == 1001);

		item = hash[0];
		TEST_ASSERT(strcmp(item.Name, "Bronze Sword") == 0);
		TEST_ASSERT(item.ID == 0);
	}

	void InsertEndTest() {
		EQ::FixedMemoryHashSet<EQ::ItemData> hash(data_, size_);
		EQ::ItemData item;
		memset(&item, 0, sizeof(item));
		strcpy(item.Name, "Jade Sword");
		item.ID = 190000;
		hash.insert(190000, item);

		TEST_ASSERT(hash.exists(1000));
		TEST_ASSERT(hash.exists(1001));
		TEST_ASSERT(hash.exists(0));
		TEST_ASSERT(hash.exists(190000));
		TEST_ASSERT(hash.size() == 4);
		TEST_ASSERT(hash.max_size() == 72000);
		TEST_ASSERT(!hash.empty());
	}

	void RetrieveEndTest() {
		EQ::FixedMemoryHashSet<EQ::ItemData> hash(data_, size_);
		TEST_ASSERT(hash.exists(1000));
		TEST_ASSERT(hash.exists(1001));
		TEST_ASSERT(hash.exists(0));
		TEST_ASSERT(hash.exists(190000));
		TEST_ASSERT(hash.size() == 4);
		TEST_ASSERT(hash.max_size() == 72000);
		TEST_ASSERT(!hash.empty());

		EQ::ItemData item = hash[1000];
		TEST_ASSERT(strcmp(item.Name, "Iron Sword") == 0);
		TEST_ASSERT(item.ID == 1000);

		item = hash[1001];
		TEST_ASSERT(strcmp(item.Name, "Steel Sword") == 0);
		TEST_ASSERT(item.ID == 1001);

		item = hash[0];
		TEST_ASSERT(strcmp(item.Name, "Bronze Sword") == 0);
		TEST_ASSERT(item.ID == 0);

		item = hash[190000];
		TEST_ASSERT(strcmp(item.Name, "Jade Sword") == 0);
		TEST_ASSERT(item.ID == 190000);
	}
private:
	uint8 *data_;
	size_t size_;
};
