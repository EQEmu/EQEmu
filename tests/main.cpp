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

#include "tests/atobool_test.h"
#include "tests/data_verification_test.h"
#include "tests/fixed_memory_test.h"
#include "tests/fixed_memory_variable_test.h"
#include "tests/hextoi_32_64_test.h"
#include "tests/ipc_mutex_test.h"
#include "tests/memory_mapped_file_test.h"
#include "tests/skills_util_test.h"
#include "tests/string_util_test.h"
#include "tests/task_state_test.h"

#include "common/path_manager.h"
#include "common/platform.h"

#include <iostream>
#include <fstream>
#include <memory>

const EQEmuConfig *Config;

int main()
{
	RegisterExecutablePlatform(ExePlatformClientImport);
	EQEmuLogSys::Instance()->LoadLogSettingsDefaults();
	PathManager::Instance()->Init();

	auto ConfigLoadResult = EQEmuConfig::LoadConfig();
	Config = EQEmuConfig::get();
	try {
		std::unique_ptr<Test::Output> output(new Test::TextOutput(Test::TextOutput::Verbose));
		Test::Suite                   tests;
		tests.add(new MemoryMappedFileTest());
		tests.add(new IPCMutexTest());
		tests.add(new FixedMemoryHashTest());
		tests.add(new FixedMemoryVariableHashTest());
		tests.add(new atoboolTest());
		tests.add(new hextoi_32_64_Test());
		tests.add(new StringUtilTest());
		tests.add(new DataVerificationTest());
		tests.add(new SkillsUtilsTest());
		tests.add(new TaskStateTest());
		tests.run(*output, true);
	}
	catch (std::exception &ex) {
		LogError("Test Failure [{}]", ex.what());
		return -1;
	}
	return 0;
}
