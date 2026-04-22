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
class TOB : public RoF2 {
public:
	constexpr TOB() {}
	constexpr ~TOB() override {}

	EQApplicationPacket* InterruptSpell(Client* c, uint32_t message, uint32_t spawn_id, uint32_t spell_id,
	                                    const char* spell_name_override) const override;
	EQApplicationPacket* InterruptSpellOther(Mob* m, uint32_t message, uint32_t spawn_id, uint32_t spell_id,
	                                         const char* spell_name_override) const override;

protected:
	uint32_t ResolveID(uint32_t id) const override;
};

} // namespace Zone::Message
