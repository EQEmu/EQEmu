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

#include <stdint.h>

// Migration path: replace string_ids.h usage with ID enum values one call site at a time.

class Client;
class Mob;
class EQApplicationPacket;

namespace ZoneClient::Message {

class IMessage {
public:
	constexpr IMessage() {}
	constexpr virtual ~IMessage() {}

	virtual void Simple(Client* c, uint32_t color, uint32_t id, uint32_t distance = 0) const = 0;

	virtual void Formatted(Client* c, uint32_t color, uint32_t id,
	               const char* a1 = nullptr, const char* a2 = nullptr, const char* a3 = nullptr,
	               const char* a4 = nullptr, const char* a5 = nullptr, const char* a6 = nullptr,
	               const char* a7 = nullptr, const char* a8 = nullptr, const char* a9 = nullptr,
	               uint32_t distance = 0) const = 0;

	// These aren't technically messages, but they use the same format and are similar enough to include here
	virtual EQApplicationPacket* InterruptSpell(Client* c, uint32_t message, uint32_t spawn_id, uint32_t spell_id,
	                                            const char* spell_name_override = "") const = 0;
	virtual EQApplicationPacket* InterruptSpellOther(Mob* m, uint32_t message, uint32_t spawn_id, uint32_t spell_id,
	                                                 const char* spell_name_override = "") const = 0;

	virtual EQApplicationPacket* Fizzle(Mob* m, uint32_t type, uint32_t message, uint32_t spell_id) const = 0;

protected:
	virtual uint32_t ResolveID(uint32_t id) const = 0;
	virtual void SendSimple(Client* c, uint32_t color, uint32_t string_id, uint32_t distance) const = 0;
	virtual void SendFormatted(Client* c, uint32_t color, uint32_t string_id, uint32_t distance,
		const char* a1 = nullptr, const char* a2 = nullptr, const char* a3 = nullptr,
		const char* a4 = nullptr, const char* a5 = nullptr, const char* a6 = nullptr,
		const char* a7 = nullptr, const char* a8 = nullptr, const char* a9 = nullptr) const = 0;
};

} // namespace Zone::Message
