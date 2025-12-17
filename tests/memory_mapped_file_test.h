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

#include "common/memory_mapped_file.h"
#include "cppunit/cpptest.h"

class MemoryMappedFileTest : public Test::Suite {
	typedef void(MemoryMappedFileTest::*TestFunction)(void);
public:
	MemoryMappedFileTest() {
		TEST_ADD(MemoryMappedFileTest::LoadAndZeroMMF)
		TEST_ADD(MemoryMappedFileTest::LoadExistingMMF)
	}

	~MemoryMappedFileTest() {
	}

	private:
	void LoadAndZeroMMF() {
		EQ::MemoryMappedFile mmf("testfile.txt", 512);
		mmf.ZeroFile();
		TEST_ASSERT(mmf.Size() == 512);

		unsigned char *data = reinterpret_cast<unsigned char*>(mmf.Get());
		TEST_ASSERT(data != nullptr);

		*reinterpret_cast<uint32*>(data) = 562;
	}

	void LoadExistingMMF() {
		EQ::MemoryMappedFile mmf("testfile.txt");
		TEST_ASSERT(mmf.Size() == 512);

		unsigned char *data = reinterpret_cast<unsigned char*>(mmf.Get());
		TEST_ASSERT(data != nullptr);

		uint32 val = *reinterpret_cast<uint32*>(data);
		TEST_ASSERT(val == 562);
	}
};
