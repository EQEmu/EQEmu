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

#include "client_version.h"

// Migration path: replace string_ids.h usage with ID enum values one call site at a time.

class Client;
class Mob;
class EQApplicationPacket;

namespace Message {

template<typename... Args>
concept AllConstChar = (std::is_convertible_v<Args, const char*> && ...);

class IMessage
{
public:
	IMessage() = default;
	virtual ~IMessage() = default;

	// these two are the basic string message packets
	virtual std::unique_ptr<EQApplicationPacket> Simple(uint32_t color, uint32_t id) const = 0;
	virtual std::unique_ptr<EQApplicationPacket> Formatted(uint32_t color, uint32_t id,
		const std::array<const char*, 9>& args) const = 0;

	// These aren't technically messages, but they use the same format and are similar enough to include here
	virtual std::unique_ptr<EQApplicationPacket> InterruptSpell(uint32_t message, uint32_t spawn_id,
		const char* spell_link) const = 0;
	virtual std::unique_ptr<EQApplicationPacket> InterruptSpellOther(Mob* sender, uint32_t message, uint32_t spawn_id,
		const char* name, const char* spell_link) const = 0;
};

} // namespace Message
