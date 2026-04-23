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
#include "zone/patch/components/message/tob.h"

#include "common/links.h"

namespace ZoneClient::Message {
struct TOBStringIDs
{
	static constexpr uint32_t DisarmedTrap = 1458; // You successfully disarmed the trap
};

uint32_t TOB::ResolveID(uint32_t id) const
{
	switch (id) {
	case YOU_FLURRY:
	case BOW_DOUBLE_DAMAGE:
	case NO_INSTRUMENT_SKILL:
	case DISCIPLINE_CONLOST:
	case TGB_ON:
	case TGB_OFF:
	case DISCIPLINE_RDY:
	case SONG_NEEDS_DRUM:
	case SONG_NEEDS_WIND:
	case SONG_NEEDS_STRINGS:
	case SONG_NEEDS_BRASS:
	case YOU_CRIT_HEAL:
	case YOU_CRIT_BLAST:
	case SPELL_WORN_OFF:
	case PET_TAUNTING:
	case DISC_LEVEL_ERROR:
	case MALE_SLAYUNDEAD:
	case FEMALE_SLAYUNDEAD:
	case FINISHING_BLOW:
	case ASSASSINATES:
	case CRIPPLING_BLOW:
	case CRITICAL_HIT:
	case DEADLY_STRIKE:
	case OTHER_CRIT_HEAL:
	case OTHER_CRIT_BLAST:
	case NPC_RAMPAGE:
	case NPC_FLURRY:
	case DISCIPLINE_FEARLESS:
	case CORPSE_ITEM_LOST:
	case FATAL_BOW_SHOT:
	case CURRENT_SPELL_EFFECTS:
	case NOT_DELEGATED_MARKER:
	case STRIKETHROUGH_STRING:
	case AE_RAMPAGE:
	case DISC_LEVEL_USE_ERROR:
	case SPLIT_FAIL:
		// removed from the client
		return 0;
	case DISARMED_TRAP:
		return TOBStringIDs::DisarmedTrap;
	default:
		return RoF2::ResolveID(id);
	}
}

void TOB::ResolveArguments(uint32_t id, std::array<const char*, 9>& args) const
{
	switch (id) {
	case SPELL_FIZZLE:
	case MISS_NOTE:
	case SPELL_FIZZLE_OTHER:
	case MISSED_NOTE_OTHER:
		// take all arguments (spell link)
		break;
	default:
		RoF2::ResolveArguments(id, args);
		break;
	}
}

// TOB is the first patch to fully support links in the client. This helper function is therefore internal to TOB
// because any future patches would default to the TOB message strings
static void ServerToTOBConvertLinks(std::string& message_out, const std::string& message_in)
{
	if (message_in.find('\x12') == std::string::npos) {
		message_out = message_in;
		return;
	}

	std::vector<std::string> segments = Strings::Split(message_in, '\x12');
	for (size_t segment_iter = 0; segment_iter < segments.size(); ++segment_iter) {
		if (segment_iter & 1) {
			auto etag = std::stoi(segments[segment_iter].substr(0, 1));

			switch (etag) {
			case 0: {
				size_t index = 1;
				std::string item_id = segments[segment_iter].substr(index, 5);
				index += 5;

				std::string aug1 = segments[segment_iter].substr(index, 5);
				index += 5;

				std::string aug2 = segments[segment_iter].substr(index, 5);
				index += 5;

				std::string aug3 = segments[segment_iter].substr(index, 5);
				index += 5;

				std::string aug4 = segments[segment_iter].substr(index, 5);
				index += 5;

				std::string aug5 = segments[segment_iter].substr(index, 5);
				index += 5;

				std::string aug6 = segments[segment_iter].substr(index, 5);
				index += 5;

				std::string is_evolving = segments[segment_iter].substr(index, 1);
				index += 1;

				std::string evolutionGroup = segments[segment_iter].substr(index, 4);
				index += 4;

				std::string evolutionLevel = segments[segment_iter].substr(index, 2);
				index += 2;

				std::string ornamentationIconID = segments[segment_iter].substr(index, 5);
				index += 5;

				std::string itemHash = segments[segment_iter].substr(index, 8);
				index += 8;

				std::string text = segments[segment_iter].substr(index);

				message_out.push_back('\x12');
				message_out.push_back('0'); //etag item
				message_out.append(item_id);
				message_out.append(aug1);
				message_out.append("00000");
				message_out.append(aug2);
				message_out.append("00000");
				message_out.append(aug3);
				message_out.append("00000");
				message_out.append(aug4);
				message_out.append("00000");
				message_out.append(aug5);
				message_out.append("00000");
				message_out.append(aug6);
				message_out.append("00000");
				message_out.append(is_evolving);
				message_out.append(evolutionGroup);
				message_out.append(evolutionLevel);
				message_out.append(ornamentationIconID);
				message_out.append("00000");
				message_out.append(itemHash);
				message_out.append(text);
				message_out.push_back('\x12');

				break;
			}
			default:
				//unsupported etag right now; just pass it as is
				message_out.push_back('\x12');
				message_out.append(segments[segment_iter]);
				message_out.push_back('\x12');
				break;
			}
		} else {
			message_out.append(segments[segment_iter]);
		}
	}
}

EQApplicationPacket* TOB::Formatted(uint32_t color, uint32_t id, const std::array<const char*, 9>& args) const
	// const char* a1, const char* a2, const char* a3,
	// const char* a4, const char* a5, const char* a6,
	// const char* a7, const char* a8, const char* a9) const
{
	uint32_t string_id = ResolveID(id);
	if (string_id > 0) {
		std::array<const char*, 9> resolved_args = args;
		ResolveArguments(id, resolved_args);
		if (!resolved_args[0])
			return Simple(color, id);

		SerializeBuffer buffer(49);
		// 49 is the minimum size needed for this packet since each arg writes at least 4 bytes
		buffer.WriteUInt32(0);
		// This is a string written like the message arrays, but it seems to be discarded by the client
		buffer.WriteUInt8(0); // 0 is a zone packet, 1 is a world packet -- these are always sent from zone from here
		buffer.WriteUInt32(string_id);
		buffer.WriteUInt32(color);

		for (auto a : resolved_args) {
			if (a != nullptr) {
				std::string new_message;
				ServerToTOBConvertLinks(new_message, a);
				buffer.WriteLengthString(new_message);
			} else
				buffer.WriteUInt32(0);
		}

		return new EQApplicationPacket(OP_FormattedMessage, std::move(buffer));
	}

	return nullptr;
}

EQApplicationPacket* TOB::InterruptSpell(uint32_t message, uint32_t spawn_id, const char* spell_link) const
{
	auto outapp = new EQApplicationPacket(OP_InterruptCast, sizeof(InterruptCast_Struct) + strlen(spell_link) + 1);
	auto ic = reinterpret_cast<InterruptCast_Struct*>(outapp->pBuffer);
	ic->messageid = ResolveID(message);
	ic->spawnid = spawn_id;
	fmt::format_to_n(ic->message, strlen(spell_link) + 1, "{}\0", spell_link);
	outapp->priority = 5;

	return outapp;
}

EQApplicationPacket* TOB::InterruptSpellOther(Mob* sender, uint32_t message, uint32_t spawn_id, const char* name,
	const char* spell_link) const
{
	auto outapp = new EQApplicationPacket(OP_InterruptCast,
		sizeof(InterruptCast_Struct) + strlen(name) + strlen(spell_link) + 2);
	auto ic = reinterpret_cast<InterruptCast_Struct*>(outapp->pBuffer);
	ic->messageid = ResolveID(message);
	ic->spawnid = spawn_id;
	fmt::format_to_n(ic->message, strlen(name) + strlen(spell_link) + 2, "{}\0{}\0", name, spell_link);

	return outapp;
}

} // namespace ZoneClient::Message
