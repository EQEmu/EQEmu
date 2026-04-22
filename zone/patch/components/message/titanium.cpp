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
void Titanium::Simple(Client* c, uint32_t color, uint32_t id, uint32_t distance) const {
	uint32_t string_id = ResolveID(id);
	if (string_id > 0)
		SendSimple(c, color, string_id, distance);
}

void Titanium::Formatted(Client* c, uint32_t color, uint32_t id,
                         const char* a1, const char* a2, const char* a3,
                         const char* a4, const char* a5, const char* a6,
                         const char* a7, const char* a8, const char* a9,
                         uint32_t distance) const {
	uint32_t string_id = ResolveID(id);
	if (string_id > 0)
		SendFormatted(c, color, string_id, distance, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}

EQApplicationPacket* Titanium::InterruptSpell(Client* c, uint32_t message, uint32_t spawn_id, uint32_t spell_id,
                                              const char* spell_name_override) const {
	auto outapp = new EQApplicationPacket(OP_InterruptCast, sizeof(InterruptCast_Struct));
	auto ic = reinterpret_cast<InterruptCast_Struct*>(outapp->pBuffer);
	ic->messageid = ResolveID(message);
	ic->spawnid = spawn_id;
	outapp->priority = 5;

	return outapp;
}

EQApplicationPacket* Titanium::InterruptSpellOther(Mob* m, uint32_t message, uint32_t spawn_id, uint32_t spell_id,
                                                   const char* spell_name_override) const {
	auto name = m->GetCleanName();
	auto outapp = new EQApplicationPacket(OP_InterruptCast, sizeof(InterruptCast_Struct) + strlen(name) + 1);
	auto ic = reinterpret_cast<InterruptCast_Struct*>(outapp->pBuffer);
	ic->messageid = ResolveID(message);
	ic->spawnid = spawn_id;
	fmt::format_to_n(ic->message, strlen(name) + 1, "{}\0", name);
	return outapp;
}

EQApplicationPacket* Titanium::Fizzle(Mob* m, uint32_t type, uint32_t message, uint32_t spell_id) const {
	return nullptr;
}

// A value of 0 means that the string isn't mapped in this client, valid string ids start at 1
uint32_t Titanium::ResolveID(uint32_t id) const {
	// passthrough — string IDs are defined at the base client level;
	// override in patches where IDs need remapping
	return id;
}

// Could override these in patches if the format of the packets differ, but they are all compatible
void Titanium::SendSimple(Client* c, uint32_t color, uint32_t string_id, uint32_t distance) const {
	auto outapp = new EQApplicationPacket(OP_SimpleMessage, sizeof(SimpleMessage_Struct));
	auto* sms = reinterpret_cast<SimpleMessage_Struct*>(outapp->pBuffer);
	sms->string_id = string_id;
	sms->color = color;
	sms->unknown8 = 0;

	if (distance > 0)
		entity_list.QueueCloseClients(c, outapp, false, distance);
	else
		c->QueuePacket(outapp);

	safe_delete(outapp);
}

void Titanium::SendFormatted(
	Client* c, uint32_t color, uint32_t string_id, uint32_t distance,
	const char* a1, const char* a2, const char* a3,
	const char* a4, const char* a5, const char* a6,
	const char* a7, const char* a8, const char* a9) const {
	if (!a1) {
		SendSimple(c, color, string_id, distance);
	} else {
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

		auto outapp = std::make_unique<EQApplicationPacket>(OP_FormattedMessage, std::move(buf));

		if (distance > 0)
			entity_list.QueueCloseClients(c, outapp.get(), false, distance);
		else
			c->QueuePacket(outapp.get());
	}
}
} // namespace ZoneClient::Message
