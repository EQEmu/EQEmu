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

#include "patch/client_version.h"

#include <cstdint>

// Migration path: replace string_ids.h usage with ID enum values one call site at a time.

class Client;
class Mob;
class EQApplicationPacket;

namespace ZoneClient::Message {

template<typename... Args>
concept AllConstChar = (std::is_convertible_v<Args, const char*> && ...);

class IMessage
{
public:
	IMessage() = default;
	virtual ~IMessage() = default;

	// these two are the basic string message packets
	[[nodiscard]] virtual EQApplicationPacket* Simple(uint32_t color, uint32_t id) const = 0;
	[[nodiscard]] virtual EQApplicationPacket* Formatted(uint32_t color, uint32_t id, const std::array<const char*, 9>& args) const = 0;

	// These aren't technically messages, but they use the same format and are similar enough to include here
	virtual EQApplicationPacket* InterruptSpell(uint32_t message, uint32_t spawn_id, const char* spell_link) const = 0;
	virtual EQApplicationPacket* InterruptSpellOther(Mob* sender, uint32_t message, uint32_t spawn_id,
		const char* name, const char* spell_link) const = 0;
};

static std::function GetComponent = [](const Client* c) -> IMessage* {
	return ClientPatch::GetMessageComponent(c->GetClientVersion()).get();
};

// Helper functions to wrap the packet construction in sends
template <AllConstChar... Args>
	requires (sizeof...(Args) <= 9)
void MessageString(Client* c, uint32_t type, uint32_t id, Args&&... args)
{
	if constexpr (sizeof...(Args) == 0) {
		ClientPatch::QueuePacket(c, &IMessage::Simple, c->GetMessageComponent().get(), type, id);
	} else {
		std::array<const char*, 9> a = {args...};
		ClientPatch::QueuePacket(c, &IMessage::Formatted, c->GetMessageComponent().get(), type, id, a);
	}
}

static auto CloseMessageString(
	Mob* sender, bool ignore_sender = false, float distance = 200.f,
	Mob* skipped_mob = nullptr, bool is_ack_required = true,
	eqFilterType filter = FilterNone)
{
	return [=]<AllConstChar... Args>(uint32_t type, uint32_t id, Args&&... args) {
		static_assert(sizeof...(Args) <= 9, "Too many arguments");

		auto queue_close_clients = ClientPatch::QueueCloseClients(sender, ignore_sender, distance, skipped_mob,
			is_ack_required, filter);

		if constexpr (sizeof...(Args) == 0) {
			return queue_close_clients(&IMessage::Simple, GetComponent, type, id);
		} else {
			std::array<const char*, 9> a = {args...};
			return queue_close_clients(&IMessage::Formatted, GetComponent, type, id, a);
		}
	};
}

inline void InterruptSpell(Client* c, uint32_t message, uint32_t spawn_id, const char* spell_link)
{
	ClientPatch::QueuePacket(c, &IMessage::InterruptSpell, c->GetMessageComponent().get(), message, spawn_id, spell_link);
}

inline void InterruptSpellOther(Mob* sender, uint32_t message, uint32_t spawn_id, const char* name, const char* spell_link)
{
	ClientPatch::QueueCloseClients(sender, true, RuleI(Range, SongMessages), nullptr, true,
		sender->IsClient() ? FilterPCSpells : FilterNPCSpells)(
			&IMessage::InterruptSpellOther, GetComponent, sender, message, spawn_id, name, spell_link);
}

} // namespace ZoneClient::Message
