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
EQApplicationPacket* Titanium::Simple(uint32_t color, uint32_t id) const {
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

EQApplicationPacket* Titanium::Formatted(uint32_t color, uint32_t id,
                                         const char* a1, const char* a2, const char* a3,
                                         const char* a4, const char* a5, const char* a6,
                                         const char* a7, const char* a8, const char* a9) const {
	uint32_t string_id = ResolveID(id);
	if (string_id > 0) {
		if (!a1)
			return Simple(color, id);

		const char* args[] = {a1, a2, a3, a4, a5, a6, a7, a8, a9};

		SerializeBuffer buf(20);
		buf.WriteInt32(0);
		buf.WriteInt32(string_id);
		buf.WriteInt32(color);

		for (const auto* arg : args) {
			if (!arg)
				break;
			buf.WriteString(arg);
		}

		buf.WriteInt8(0);

		return new EQApplicationPacket(OP_FormattedMessage, std::move(buf));
	}

	return nullptr;
}

EQApplicationPacket* Titanium::InterruptSpell(uint32_t message, uint32_t spawn_id, uint32_t spell_id,
                                              const char* spell_name_override) const {
	auto outapp = new EQApplicationPacket(OP_InterruptCast, sizeof(InterruptCast_Struct));
	auto ic = reinterpret_cast<InterruptCast_Struct*>(outapp->pBuffer);
	ic->messageid = ResolveID(message);
	ic->spawnid = spawn_id;
	outapp->priority = 5;

	return outapp;
}

EQApplicationPacket* Titanium::InterruptSpellOther(Mob* sender, uint32_t message, uint32_t spawn_id, uint32_t spell_id,
                                                   const char* spell_name_override) const {
	auto name = sender->GetCleanName();
	auto outapp = new EQApplicationPacket(OP_InterruptCast, sizeof(InterruptCast_Struct) + strlen(name) + 1);
	auto ic = reinterpret_cast<InterruptCast_Struct*>(outapp->pBuffer);
	ic->messageid = ResolveID(message);
	ic->spawnid = spawn_id;
	fmt::format_to_n(ic->message, strlen(name) + 1, "{}\0", name);
	return outapp;
}

EQApplicationPacket* Titanium::Fizzle(uint32_t type, uint32_t message, uint32_t spell_id) const {
	return Simple(type, message);
}

EQApplicationPacket* Titanium::FizzleOther(uint32_t type, uint32_t message, uint32_t spell_id, const char* caster) const {
	return Formatted(type, message, caster);
}

// A value of 0 means that the string isn't mapped in this client, valid string ids start at 1
uint32_t Titanium::ResolveID(uint32_t id) const {
	// passthrough — string IDs are defined at the base client level;
	// override in patches where IDs need remapping
	return id;
}

} // namespace ZoneClient::Message
