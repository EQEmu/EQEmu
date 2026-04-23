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

#include "zone/patch/components/message/rof2.h"

namespace ZoneClient::Message {
class TOB : public RoF2
{
public:
	TOB() {}
	~TOB() override {}

	[[nodiscard]] EQApplicationPacket* Formatted(uint32_t color, uint32_t id, const std::array<const char*, 9>& args) const override;

	EQApplicationPacket* InterruptSpell(uint32_t message, uint32_t spawn_id, const char* spell_link) const override;
	EQApplicationPacket* InterruptSpellOther(Mob* sender, uint32_t message, uint32_t spawn_id, const char* name, const char* spell_link) const override;

protected:
	[[nodiscard]] uint32_t ResolveID(uint32_t id) const override;
	void ResolveArguments(uint32_t id, std::array<const char*, 9>& args) const override;
};
} // namespace ZoneClient::Message
