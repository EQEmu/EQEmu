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

#include "common/eqemu_config.h"
#include "common/ipc_mutex.h"
#include "cppunit/cpptest.h"

extern const EQEmuConfig *Config;

class IPCMutexTest : public Test::Suite {
	typedef void(IPCMutexTest::*TestFunction)(void);
public:
	IPCMutexTest() {
		TEST_ADD(IPCMutexTest::LockMutexTest);
		TEST_ADD(IPCMutexTest::UnlockMutexTest);
		TEST_ADD(IPCMutexTest::DoubleLockMutexTest);
		TEST_ADD(IPCMutexTest::DoubleUnlockMutexTest);
	}

	~IPCMutexTest() {
	}

	private:
	void LockMutexTest() {
		EQ::IPCMutex mutex("TestMutex1");
		TEST_ASSERT(mutex.Lock());
		TEST_ASSERT(mutex.Unlock());
	}

	void UnlockMutexTest() {
		EQ::IPCMutex mutex("TestMutex2");
		TEST_ASSERT(!mutex.Unlock());
	}

	void DoubleLockMutexTest() {
		EQ::IPCMutex mutex("TestMutex3");
		TEST_ASSERT(mutex.Lock());
		TEST_ASSERT(!mutex.Lock());
	}

	void DoubleUnlockMutexTest() {
		EQ::IPCMutex mutex("TestMutex4");
		TEST_ASSERT(mutex.Lock());
		TEST_ASSERT(mutex.Unlock());
		TEST_ASSERT(!mutex.Unlock());
	}
};
