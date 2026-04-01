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

#include "common/skills.h"
#include "cppunit/cpptest.h"

class SkillsUtilsTest: public Test::Suite {
	typedef void(SkillsUtilsTest::*TestFunction)(void);
public:
	SkillsUtilsTest() {
		TEST_ADD(SkillsUtilsTest::IsTradeskill);
		TEST_ADD(SkillsUtilsTest::IsSpecializedSkill);
	}

	~SkillsUtilsTest() {
	}

	private:
	void IsTradeskill() {
		TEST_ASSERT(EQ::skills::IsTradeskill(EQ::skills::SkillPottery));
		TEST_ASSERT(!EQ::skills::IsTradeskill(EQ::skills::SkillParry));
	}

	void IsSpecializedSkill() {
		TEST_ASSERT(EQ::skills::IsSpecializedSkill(EQ::skills::SkillSpecializeConjuration));
		TEST_ASSERT(!EQ::skills::IsSpecializedSkill(EQ::skills::SkillConjuration))
	}
};
