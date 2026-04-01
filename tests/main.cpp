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
