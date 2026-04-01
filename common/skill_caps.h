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

#include "common/classes.h"
#include "common/repositories/skill_caps_repository.h"
#include "common/skills.h"
#include "common/types.h"

class SkillCaps {
public:
	inline void ClearSkillCaps() { m_skill_caps.clear(); }
	SkillCapsRepository::SkillCaps GetSkillCap(uint8 class_id, EQ::skills::SkillType skill_id, uint8 level);
	uint8 GetSkillTrainLevel(uint8 class_id, EQ::skills::SkillType skill_id, uint8 level);
	void LoadSkillCaps();
	void ReloadSkillCaps();
	static int32_t GetSkillCapMaxLevel(uint8 class_id, EQ::skills::SkillType skill_id);

	SkillCaps *SetContentDatabase(Database *db);

	static SkillCaps* Instance()
	{
		static SkillCaps instance;
		return &instance;
	}
private:
	Database                                    *m_content_database{};
	std::map<uint64, SkillCapsRepository::SkillCaps> m_skill_caps = {};
};
