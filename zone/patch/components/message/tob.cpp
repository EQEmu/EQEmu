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

struct TOBStringIDs {
	static constexpr uint32_t DisarmedTrap = 1458; // You successfully disarmed the trap
};

uint32_t TOB::ResolveID(uint32_t id) const {
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

EQApplicationPacket* TOB::InterruptSpell(Client* c, uint32_t message, uint32_t spawn_id, uint32_t spell_id, const char* spell_name_override) const {
	std::string spell_name = spell_name_override == nullptr || *spell_name_override == '\0'
								 ? GetSpellName(spell_id)
								 : spell_name_override;

	std::string spell_link = Links::FormatSpellLink(spell_id, spell_name);

	auto outapp = new EQApplicationPacket(OP_InterruptCast, sizeof(InterruptCast_Struct) + spell_link.size() + 1);
	auto ic = reinterpret_cast<InterruptCast_Struct*>(outapp->pBuffer);
	ic->messageid = ResolveID(message);
	ic->spawnid = spawn_id;
	fmt::format_to_n(ic->message, spell_link.size(), "{}", spell_link);
	outapp->priority = 5;

	return outapp;
}

EQApplicationPacket* TOB::InterruptSpellOther(Mob* m, uint32_t message, uint32_t spawn_id, uint32_t spell_id,
                                              const char* spell_name_override) const {
	std::string spell_name = spell_name_override == nullptr || *spell_name_override == '\0'
								 ? GetSpellName(spell_id)
								 : spell_name_override;

	std::string spell_link = Links::FormatSpellLink(spell_id, spell_name);

	auto name = m->GetCleanName();
	auto outapp = new EQApplicationPacket(OP_InterruptCast, sizeof(InterruptCast_Struct) + strlen(name) + spell_link.size() + 2);
	auto ic = reinterpret_cast<InterruptCast_Struct*>(outapp->pBuffer);
	ic->messageid = ResolveID(message);
	ic->spawnid = spawn_id;
	fmt::format_to_n(ic->message, strlen(name) + spell_link.size() + 2, "{}\0{}\0", name, spell_link);

	return outapp;
}

} // namespace Zone::Message
