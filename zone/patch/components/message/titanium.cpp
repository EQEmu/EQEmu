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
#include "zone/patch/components/message/titanium.h"

#include "client.h"

#include "common/eq_packet.h"
#include "common/eq_packet_structs.h"
#include "common/serialize_buffer.h"

namespace ZoneClient::Message {

EQApplicationPacket* Titanium::Simple(uint32_t color, uint32_t id) const
{
	uint32_t string_id = ResolveID(id);
	if (string_id > 0) {
		auto outapp = new EQApplicationPacket(OP_SimpleMessage, sizeof(SimpleMessage_Struct));
		auto* sms = reinterpret_cast<SimpleMessage_Struct*>(outapp->pBuffer);
		sms->string_id = string_id;
		sms->color = color;
		sms->unknown8 = 0;

		return outapp;
	}

	return nullptr;
}

EQApplicationPacket* Titanium::Formatted(
	uint32_t color, uint32_t id, const std::array<const char*, 9>& args) const
{
	uint32_t string_id = ResolveID(id);
	if (string_id > 0) {
		std::array<const char*, 9> resolved_args = args;
		ResolveArguments(id, resolved_args);
		if (!resolved_args[0])
			return Simple(color, id);

		SerializeBuffer buf(20);
		buf.WriteUInt32(0);
		buf.WriteUInt32(string_id);
		buf.WriteUInt32(color);

		for (const auto* a : resolved_args) {
			if (a != nullptr)
				buf.WriteString(a);
		}

		buf.WriteUInt8(0);

		return new EQApplicationPacket(OP_FormattedMessage, std::move(buf));
	}

	return nullptr;
}

EQApplicationPacket* Titanium::InterruptSpell(uint32_t message, uint32_t spawn_id, const char* spell_link) const
{
	auto outapp = new EQApplicationPacket(OP_InterruptCast, sizeof(InterruptCast_Struct));
	auto ic = reinterpret_cast<InterruptCast_Struct*>(outapp->pBuffer);
	ic->messageid = ResolveID(message);
	ic->spawnid = spawn_id;
	outapp->priority = 5;

	return outapp;
}

EQApplicationPacket* Titanium::InterruptSpellOther(Mob* sender, uint32_t message, uint32_t spawn_id, const char* name,
	const char* spell_link) const
{
	auto outapp = new EQApplicationPacket(OP_InterruptCast, sizeof(InterruptCast_Struct) + strlen(name) + 1);
	auto ic = reinterpret_cast<InterruptCast_Struct*>(outapp->pBuffer);
	ic->messageid = ResolveID(message);
	ic->spawnid = spawn_id;
	fmt::format_to_n(ic->message, strlen(name) + 1, "{}\0", name);
	return outapp;
}

// A value of 0 means that the string isn't mapped in this client, valid string ids start at 1
uint32_t Titanium::ResolveID(uint32_t id) const
{
	// passthrough — string IDs are defined at the base client level;
	// override in patches where IDs need remapping
	return id;
}

void Titanium::ResolveArguments(uint32_t id, std::array<const char*, 9>& args) const
{
	switch (id) {
	case SPELL_FIZZLE:
	case MISS_NOTE:
		args[0] = nullptr; // drop spell link
		break;
	case SPELL_FIZZLE_OTHER:
	case MISSED_NOTE_OTHER:
		args[1] = nullptr; // drop spell link
		break;
	default:
		break;
	}
}
} // namespace ZoneClient::Message
