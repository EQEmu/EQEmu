/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
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
