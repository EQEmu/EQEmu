#include "spdat.h"

#include "zone/bot.h"

bool IsBotSpellTypeDetrimental(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::Nuke:
		case BotSpellTypes::Root:
		case BotSpellTypes::Lifetap:
		case BotSpellTypes::Snare:
		case BotSpellTypes::DOT:
		case BotSpellTypes::Dispel:
		case BotSpellTypes::Mez:
		case BotSpellTypes::Charm:
		case BotSpellTypes::Slow:
		case BotSpellTypes::Debuff:
		case BotSpellTypes::HateRedux:
		case BotSpellTypes::Fear:
		case BotSpellTypes::Stun:
		case BotSpellTypes::AENukes:
		case BotSpellTypes::AERains:
		case BotSpellTypes::AEMez:
		case BotSpellTypes::AEStun:
		case BotSpellTypes::AEDebuff:
		case BotSpellTypes::AESlow:
		case BotSpellTypes::AESnare:
		case BotSpellTypes::AEFear:
		case BotSpellTypes::AEDispel:
		case BotSpellTypes::AERoot:
		case BotSpellTypes::AEDoT:
		case BotSpellTypes::AELifetap:
		case BotSpellTypes::PBAENuke:
		case BotSpellTypes::Lull:
		case BotSpellTypes::AELull:
		case BotSpellTypes::HateLine:
		case BotSpellTypes::AEHateLine:
			return true;
		default:
			return false;
	}

	return false;
}

bool IsBotSpellTypeBeneficial(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::RegularHeal:
		case BotSpellTypes::CompleteHeal:
		case BotSpellTypes::GroupCompleteHeals:
		case BotSpellTypes::FastHeals:
		case BotSpellTypes::VeryFastHeals:
		case BotSpellTypes::GroupHeals:
		case BotSpellTypes::GroupHoTHeals:
		case BotSpellTypes::HoTHeals:
		case BotSpellTypes::PetRegularHeals:
		case BotSpellTypes::PetCompleteHeals:
		case BotSpellTypes::PetFastHeals:
		case BotSpellTypes::PetVeryFastHeals:
		case BotSpellTypes::PetHoTHeals:
		case BotSpellTypes::Buff:
		case BotSpellTypes::Cure:
		case BotSpellTypes::GroupCures:
		case BotSpellTypes::PetCures:
		case BotSpellTypes::DamageShields:
		case BotSpellTypes::InCombatBuffSong:
		case BotSpellTypes::OutOfCombatBuffSong:
		case BotSpellTypes::Pet:
		case BotSpellTypes::PetBuffs:
		case BotSpellTypes::PreCombatBuff:
		case BotSpellTypes::PreCombatBuffSong:
		case BotSpellTypes::PetDamageShields:
		case BotSpellTypes::PetResistBuffs:
		case BotSpellTypes::ResistBuffs:
		case BotSpellTypes::Resurrect:
		case BotSpellTypes::Teleport:
		case BotSpellTypes::Succor:
		case BotSpellTypes::BindAffinity:
		case BotSpellTypes::Identify:
		case BotSpellTypes::Levitate:
		case BotSpellTypes::Rune:
		case BotSpellTypes::WaterBreathing:
		case BotSpellTypes::Size:
		case BotSpellTypes::Invisibility:
		case BotSpellTypes::MovementSpeed:
		case BotSpellTypes::SendHome:
		case BotSpellTypes::SummonCorpse:
			return true;
		default:
			return false;
	}

	return false;
}

bool BotSpellTypeUsesTargetSettings(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::RegularHeal:
		case BotSpellTypes::CompleteHeal:
		case BotSpellTypes::GroupCompleteHeals:
		case BotSpellTypes::FastHeals:
		case BotSpellTypes::VeryFastHeals:
		case BotSpellTypes::GroupHeals:
		case BotSpellTypes::GroupHoTHeals:
		case BotSpellTypes::HoTHeals:
		case BotSpellTypes::PetRegularHeals:
		case BotSpellTypes::PetCompleteHeals:
		case BotSpellTypes::PetFastHeals:
		case BotSpellTypes::PetVeryFastHeals:
		case BotSpellTypes::PetHoTHeals:
		case BotSpellTypes::Buff:
		case BotSpellTypes::Cure:
		case BotSpellTypes::GroupCures:
		case BotSpellTypes::PetCures:
		case BotSpellTypes::DamageShields:
		case BotSpellTypes::PetDamageShields:
		case BotSpellTypes::PetBuffs:
		case BotSpellTypes::ResistBuffs:
		case BotSpellTypes::PetResistBuffs:
		case BotSpellTypes::Teleport:
		case BotSpellTypes::Succor:
		case BotSpellTypes::BindAffinity:
		case BotSpellTypes::Identify:
		case BotSpellTypes::Levitate:
		case BotSpellTypes::Rune:
		case BotSpellTypes::WaterBreathing:
		case BotSpellTypes::Size:
		case BotSpellTypes::Invisibility:
		case BotSpellTypes::MovementSpeed:
		case BotSpellTypes::SendHome:
		case BotSpellTypes::SummonCorpse:
			return true;
		default:
			return false;
	}

	return false;
}

bool IsBotSpellTypeInnate(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::AENukes:
		case BotSpellTypes::AERains:
		case BotSpellTypes::PBAENuke:
		case BotSpellTypes::Nuke:
		case BotSpellTypes::AEDispel:
		case BotSpellTypes::Dispel:
		case BotSpellTypes::AERoot:
		case BotSpellTypes::Root:
		case BotSpellTypes::AESlow:
		case BotSpellTypes::Slow:
		case BotSpellTypes::Charm:
		case BotSpellTypes::AEDebuff:
		case BotSpellTypes::Debuff:
		case BotSpellTypes::AEDoT:
		case BotSpellTypes::DOT:
		case BotSpellTypes::AELifetap:
		case BotSpellTypes::Lifetap:
		case BotSpellTypes::AEStun:
		case BotSpellTypes::Stun:
		case BotSpellTypes::AEMez:
		case BotSpellTypes::Mez:
		case BotSpellTypes::Lull:
		case BotSpellTypes::AELull:
		case BotSpellTypes::HateLine:
		case BotSpellTypes::AEHateLine:
			return true;
		default:
			return false;
	}

	return false;
}

bool IsAEBotSpellType(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::AEDebuff:
		case BotSpellTypes::AEFear:
		case BotSpellTypes::AEMez:
		case BotSpellTypes::AENukes:
		case BotSpellTypes::AERains:
		case BotSpellTypes::AESlow:
		case BotSpellTypes::AESnare:
		case BotSpellTypes::AEStun:
		case BotSpellTypes::AEDispel:
		case BotSpellTypes::AEDoT:
		case BotSpellTypes::PBAENuke:
		case BotSpellTypes::AELifetap:
		case BotSpellTypes::AERoot:
		case BotSpellTypes::AEHateLine:
		case BotSpellTypes::AELull:
			return true;
		default:
			return false;
	}

	return false;
}

bool IsGroupBotSpellType(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::GroupCures:
		case BotSpellTypes::GroupCompleteHeals:
		case BotSpellTypes::GroupHeals:
		case BotSpellTypes::GroupHoTHeals:
			return true;
		default:
			return false;
	}

	return false;
}

bool IsGroupTargetOnlyBotSpellType(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::GroupCures:
		case BotSpellTypes::GroupCompleteHeals:
		case BotSpellTypes::GroupHeals:
			return true;
		default:
			return false;
	}

	return false;
}

bool IsPetBotSpellType(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::PetBuffs:
		case BotSpellTypes::PetRegularHeals:
		case BotSpellTypes::PetCompleteHeals:
		case BotSpellTypes::PetFastHeals:
		case BotSpellTypes::PetVeryFastHeals:
		case BotSpellTypes::PetHoTHeals:
		case BotSpellTypes::PetDamageShields:
		case BotSpellTypes::PetResistBuffs:
		case BotSpellTypes::PetCures:
			return true;
		default:
			return false;
	}

	return false;
}

bool IsClientBotSpellType(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::RegularHeal:
		case BotSpellTypes::CompleteHeal:
		case BotSpellTypes::GroupCompleteHeals:
		case BotSpellTypes::FastHeals:
		case BotSpellTypes::VeryFastHeals:
		case BotSpellTypes::GroupHeals:
		case BotSpellTypes::GroupHoTHeals:
		case BotSpellTypes::HoTHeals:
		case BotSpellTypes::PetRegularHeals:
		case BotSpellTypes::PetCompleteHeals:
		case BotSpellTypes::PetFastHeals:
		case BotSpellTypes::PetVeryFastHeals:
		case BotSpellTypes::PetHoTHeals:
		case BotSpellTypes::Buff:
		case BotSpellTypes::Cure:
		case BotSpellTypes::GroupCures:
		case BotSpellTypes::PetCures:
		case BotSpellTypes::DamageShields:
		case BotSpellTypes::PetDamageShields:
		case BotSpellTypes::PetBuffs:
		case BotSpellTypes::ResistBuffs:
		case BotSpellTypes::PetResistBuffs:
			return true;
		default:
			return false;
	}

	return false;
}

bool IsHealBotSpellType(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::VeryFastHeals:
		case BotSpellTypes::FastHeals:
		case BotSpellTypes::RegularHeal:
		case BotSpellTypes::GroupHeals:
		case BotSpellTypes::CompleteHeal:
		case BotSpellTypes::GroupCompleteHeals:
		case BotSpellTypes::HoTHeals:
		case BotSpellTypes::GroupHoTHeals:
		case BotSpellTypes::PetRegularHeals:
		case BotSpellTypes::PetCompleteHeals:
		case BotSpellTypes::PetFastHeals:
		case BotSpellTypes::PetVeryFastHeals:
		case BotSpellTypes::PetHoTHeals:
			return true;
		default:
			return false;
	}

	return false;
}

bool BotSpellTypeRequiresLoS(uint16 spell_type) {
	if (IsAEBotSpellType(spell_type)) { // These gather their own targets later
		return false;
	}

	switch (spell_type) {
		case BotSpellTypes::RegularHeal:
		case BotSpellTypes::GroupCompleteHeals:
		case BotSpellTypes::CompleteHeal:
		case BotSpellTypes::FastHeals:
		case BotSpellTypes::VeryFastHeals:
		case BotSpellTypes::GroupHeals:
		case BotSpellTypes::GroupHoTHeals:
		case BotSpellTypes::HoTHeals:
		case BotSpellTypes::PetRegularHeals:
		case BotSpellTypes::PetCompleteHeals:
		case BotSpellTypes::PetFastHeals:
		case BotSpellTypes::PetVeryFastHeals:
		case BotSpellTypes::PetHoTHeals:
		case BotSpellTypes::InCombatBuff:
			return false;
		default:
			return true;
	}

	return true;
}

bool BotSpellTypeRequiresTarget(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::Pet:
		case BotSpellTypes::Succor:
			return false;
		default:
			return true;
	}

	return true;
}

bool BotSpellTypeRequiresAEChecks(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::AEMez:
			return false;
		default:
			return true;
	}

	return true;
}

bool RequiresStackCheck(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::VeryFastHeals:
		case BotSpellTypes::PetVeryFastHeals:
		case BotSpellTypes::FastHeals:
		case BotSpellTypes::PetFastHeals:
		case BotSpellTypes::RegularHeal:
		case BotSpellTypes::PetRegularHeals:
		case BotSpellTypes::CompleteHeal:
		case BotSpellTypes::PetCompleteHeals:
		case BotSpellTypes::GroupCompleteHeals:
		case BotSpellTypes::Resurrect:
			return false;
		default:
			return true;
	}

	return true;
}

bool IsCommandedBotSpellType(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::Charm:
		case BotSpellTypes::AEFear:
		case BotSpellTypes::Fear:
		case BotSpellTypes::Resurrect:
		case BotSpellTypes::AELull:
		case BotSpellTypes::Lull:
		case BotSpellTypes::Teleport:
		case BotSpellTypes::Succor:
		case BotSpellTypes::BindAffinity:
		case BotSpellTypes::Identify:
		case BotSpellTypes::Levitate:
		case BotSpellTypes::Rune:
		case BotSpellTypes::WaterBreathing:
		case BotSpellTypes::Size:
		case BotSpellTypes::Invisibility:
		case BotSpellTypes::MovementSpeed:
		case BotSpellTypes::SendHome:
		case BotSpellTypes::SummonCorpse:
			return true;
		default:
			return false;
	}

	return false;
}

bool IsPullingBotSpellType(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::Nuke:
		case BotSpellTypes::Lifetap:
		case BotSpellTypes::Snare:
		case BotSpellTypes::DOT:
		case BotSpellTypes::Dispel:
		case BotSpellTypes::Slow:
		case BotSpellTypes::Debuff:
		case BotSpellTypes::Stun:
		case BotSpellTypes::HateLine:
			return true;
		default:
			return false;
	}

	return false;
}

uint16 GetCorrectBotSpellType(uint16 spell_type, uint16 spell_id) {
	if (!IsValidSpell(spell_id)) {
		return UINT16_MAX;
	}

	uint16 correct_type = spell_type;

	if (!Bot::IsValidSpellTypeBySpellID(spell_type, spell_id)) {
		correct_type = UINT16_MAX;

		auto start = std::min({ BotSpellTypes::START, BotSpellTypes::COMMANDED_START, BotSpellTypes::DISCIPLINE_START });
		auto end = std::max({ BotSpellTypes::END, BotSpellTypes::COMMANDED_END, BotSpellTypes::DISCIPLINE_END });

		for (int i = end; i >= start; --i) {
			if (!Bot::IsValidBotSpellType(i) || i == BotSpellTypes::InCombatBuff) {
				continue;
			}

			if (Bot::IsValidSpellTypeBySpellID(i, spell_id)) {
				correct_type = i;

				break;
			}
		}
	}

	return correct_type;
}

uint16 GetPetBotSpellType(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::Buff:
			return BotSpellTypes::PetBuffs;
		case BotSpellTypes::RegularHeal:
			return BotSpellTypes::PetRegularHeals;
		case BotSpellTypes::CompleteHeal:
			return BotSpellTypes::PetCompleteHeals;
		case BotSpellTypes::FastHeals:
			return BotSpellTypes::PetFastHeals;
		case BotSpellTypes::VeryFastHeals:
			return BotSpellTypes::PetVeryFastHeals;
		case BotSpellTypes::HoTHeals:
			return BotSpellTypes::PetHoTHeals;
		case BotSpellTypes::Cure:
			return BotSpellTypes::PetCures;
		case BotSpellTypes::DamageShields:
			return BotSpellTypes::PetDamageShields;
		case BotSpellTypes::ResistBuffs:
			return BotSpellTypes::PetResistBuffs;
		default:
			return spell_type;
	}

	return spell_type;
}

bool IsBotBuffSpellType(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::Buff:
		case BotSpellTypes::PetBuffs:
		case BotSpellTypes::ResistBuffs:
		case BotSpellTypes::PetResistBuffs:
		case BotSpellTypes::DamageShields:
		case BotSpellTypes::PetDamageShields:
			return true;
		default:
			return false;
	}

	return false;
}

bool BotRequiresLoSToCast(uint16 spell_type, uint16 spell_id) {
	if (!BotSpellTypeRequiresTarget(spell_type)) {
		return false;
	}

	if (!IsTargetRequiredForSpell(spell_id)) {
		return false;
	}

	return true;
}
