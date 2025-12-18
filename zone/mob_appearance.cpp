/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2018 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include "mob.h"

#include "common/data_verification.h"
#include "common/eqemu_logsys.h"
#include "common/item_data.h"
#include "common/spdat.h"
#include "common/strings.h"
#include "zone/bot.h"
#include "zone/quest_parser_collection.h"
#include "zone/zonedb.h"

void Mob::SetMobTextureProfile(
	uint8 material_slot,
	uint32 texture,
	uint32 color,
	uint32 hero_forge_model
)
{
	LogMobAppearanceDetail(
		"[{}] material_slot [{}] texture [{}] color [{}] hero_forge_model [{}]",
		GetCleanName(),
		material_slot,
		texture,
		color,
		hero_forge_model
	);

	switch (material_slot) {
		case EQ::textures::armorHead:
			mob_texture_profile.Head.Material        = texture;
			mob_texture_profile.Head.HerosForgeModel = hero_forge_model;
			mob_texture_profile.Head.Color           = color;
			break;
		case EQ::textures::armorChest:
			mob_texture_profile.Chest.Material        = texture;
			mob_texture_profile.Chest.HerosForgeModel = hero_forge_model;
			mob_texture_profile.Chest.Color           = color;
			break;
		case EQ::textures::armorArms:
			mob_texture_profile.Arms.Material        = texture;
			mob_texture_profile.Arms.HerosForgeModel = hero_forge_model;
			mob_texture_profile.Arms.Color           = color;
			break;
		case EQ::textures::armorWrist:
			mob_texture_profile.Wrist.Material        = texture;
			mob_texture_profile.Wrist.HerosForgeModel = hero_forge_model;
			mob_texture_profile.Wrist.Color           = color;
			break;
		case EQ::textures::armorHands:
			mob_texture_profile.Hands.Material        = texture;
			mob_texture_profile.Hands.HerosForgeModel = hero_forge_model;
			mob_texture_profile.Hands.Color           = color;
			break;
		case EQ::textures::armorLegs:
			mob_texture_profile.Legs.Material        = texture;
			mob_texture_profile.Legs.HerosForgeModel = hero_forge_model;
			mob_texture_profile.Legs.Color           = color;
			break;
		case EQ::textures::armorFeet:
			mob_texture_profile.Feet.Material        = texture;
			mob_texture_profile.Feet.HerosForgeModel = hero_forge_model;
			mob_texture_profile.Feet.Color           = color;
			break;
		case EQ::textures::weaponPrimary:
			mob_texture_profile.Primary.Material        = texture;
			mob_texture_profile.Primary.HerosForgeModel = hero_forge_model;
			mob_texture_profile.Primary.Color           = color;
			break;
		case EQ::textures::weaponSecondary:
			mob_texture_profile.Secondary.Material        = texture;
			mob_texture_profile.Secondary.HerosForgeModel = hero_forge_model;
			mob_texture_profile.Secondary.Color           = color;
			break;
		default:
			return;
	}
}

uint32 Mob::GetTextureProfileMaterial(uint8 material_slot) const
{
	switch (material_slot) {
		case EQ::textures::armorHead:
			return mob_texture_profile.Head.Material;
		case EQ::textures::armorChest:
			return mob_texture_profile.Chest.Material;
		case EQ::textures::armorArms:
			return mob_texture_profile.Arms.Material;
		case EQ::textures::armorWrist:
			return mob_texture_profile.Wrist.Material;
		case EQ::textures::armorHands:
			return mob_texture_profile.Hands.Material;
		case EQ::textures::armorLegs:
			return mob_texture_profile.Legs.Material;
		case EQ::textures::armorFeet:
			return mob_texture_profile.Feet.Material;
		case EQ::textures::weaponPrimary:
			return mob_texture_profile.Primary.Material;
		case EQ::textures::weaponSecondary:
			return mob_texture_profile.Secondary.Material;
		default:
			return 0;
	}
}

uint32 Mob::GetTextureProfileColor(uint8 material_slot) const
{
	switch (material_slot) {
		case EQ::textures::armorHead:
			return mob_texture_profile.Head.Color;
		case EQ::textures::armorChest:
			return mob_texture_profile.Chest.Color;
		case EQ::textures::armorArms:
			return mob_texture_profile.Arms.Color;
		case EQ::textures::armorWrist:
			return mob_texture_profile.Wrist.Color;
		case EQ::textures::armorHands:
			return mob_texture_profile.Hands.Color;
		case EQ::textures::armorLegs:
			return mob_texture_profile.Legs.Color;
		case EQ::textures::armorFeet:
			return mob_texture_profile.Feet.Color;
		case EQ::textures::weaponPrimary:
			return mob_texture_profile.Primary.Color;
		case EQ::textures::weaponSecondary:
			return mob_texture_profile.Secondary.Color;
		default:
			return 0;
	}
}

uint32 Mob::GetTextureProfileHeroForgeModel(uint8 material_slot) const
{
	switch (material_slot) {
		case EQ::textures::armorHead:
			return mob_texture_profile.Head.HerosForgeModel;
		case EQ::textures::armorChest:
			return mob_texture_profile.Chest.HerosForgeModel;
		case EQ::textures::armorArms:
			return mob_texture_profile.Arms.HerosForgeModel;
		case EQ::textures::armorWrist:
			return mob_texture_profile.Wrist.HerosForgeModel;
		case EQ::textures::armorHands:
			return mob_texture_profile.Hands.HerosForgeModel;
		case EQ::textures::armorLegs:
			return mob_texture_profile.Legs.HerosForgeModel;
		case EQ::textures::armorFeet:
			return mob_texture_profile.Feet.HerosForgeModel;
		case EQ::textures::weaponPrimary:
			return mob_texture_profile.Primary.HerosForgeModel;
		case EQ::textures::weaponSecondary:
			return mob_texture_profile.Secondary.HerosForgeModel;
		default:
			return 0;
	}
}

uint32 Mob::GetEquipmentMaterial(uint8 material_slot) const
{
	uint32       equipment_material       = 0;
	const uint32 texture_profile_material = GetTextureProfileMaterial(material_slot);

	LogMobAppearance(
		"[{}] material_slot [{}] texture_profile_material [{}]",
		clean_name,
		material_slot,
		texture_profile_material
	);

	if (texture_profile_material) {
		return texture_profile_material;
	}

	auto item = database.GetItem(GetEquippedItemFromTextureSlot(material_slot));

	if (item) {
		const auto is_equipped_weapon = EQ::ValueWithin(material_slot, EQ::textures::weaponPrimary, EQ::textures::weaponSecondary);

		if (is_equipped_weapon) {
			if (IsClient()) {
				const auto inventory_slot = EQ::InventoryProfile::CalcSlotFromMaterial(material_slot);
				if (inventory_slot == INVALID_INDEX) {
					return 0;
				}

				const auto inst = CastToClient()->m_inv[inventory_slot];

				if (inst) {
					const auto augment = inst->GetOrnamentationAugment();

					if (augment) {
						item = augment->GetItem();
						if (item && strlen(item->IDFile) > 2 && Strings::IsNumber(&item->IDFile[2])) {
							equipment_material = Strings::ToUnsignedInt(&item->IDFile[2]);
						}
					} else if (inst->GetOrnamentationIDFile()) {
						equipment_material = inst->GetOrnamentationIDFile();
					}
				}
			}

			if (!equipment_material && strlen(item->IDFile) > 2 && Strings::IsNumber(&item->IDFile[2])) {
				equipment_material = Strings::ToUnsignedInt(&item->IDFile[2]);
			}
		} else {
			equipment_material = item->Material;
		}
	}

	return equipment_material;
}

uint8 Mob::GetEquipmentType(uint8 material_slot) const
{
	const auto item      = database.GetItem(GetEquippedItemFromTextureSlot(material_slot));
	auto       item_type = static_cast<uint8>(EQ::item::ItemType2HBlunt);

	if (item) {
		const auto is_equipped_weapon = EQ::ValueWithin(material_slot, EQ::textures::weaponPrimary, EQ::textures::weaponSecondary);

		if (is_equipped_weapon) {
			if (IsClient()) {
				const auto inventory_slot = EQ::InventoryProfile::CalcSlotFromMaterial(material_slot);
				if (inventory_slot == INVALID_INDEX) {
					return item_type;
				}

				const auto inst = CastToClient()->m_inv[inventory_slot];
				if (inst) {
					item_type = inst->GetItemType();
				}
			}
		}
	}

	return item_type;
}

uint32 Mob::GetEquipmentColor(uint8 material_slot) const
{
	if (armor_tint.Slot[material_slot].Color) {
		return armor_tint.Slot[material_slot].Color;
	}

	const auto item = database.GetItem(GetEquippedItemFromTextureSlot(material_slot));
	if (item) {
		return item->Color;
	}

	return 0;
}

uint32 Mob::GetHerosForgeModel(uint8 material_slot) const
{
	uint32 heros_forge_model = 0;
	if (EQ::ValueWithin(material_slot, 0, EQ::textures::weaponPrimary)) {
		auto       item = database.GetItem(GetEquippedItemFromTextureSlot(material_slot));
		const auto slot = EQ::InventoryProfile::CalcSlotFromMaterial(material_slot);

		if (item && slot != INVALID_INDEX) {
			if (IsClient()) {
				const auto inst = CastToClient()->m_inv[slot];
				if (inst) {
					const auto augment = inst->GetOrnamentationAugment();

					if (augment) {
						item              = augment->GetItem();
						heros_forge_model = item->HerosForgeModel;
					} else if (inst->GetOrnamentHeroModel(material_slot)) {
						heros_forge_model = inst->GetOrnamentHeroModel(material_slot);
					}
				}
			}

			if (!heros_forge_model) {
				heros_forge_model = item->HerosForgeModel;
			}
		}

		if (IsNPC()) {
			heros_forge_model = CastToNPC()->GetHeroForgeModel();

			/**
			 * Robes require full model number, and should only be sent to chest, arms, wrists, and legs slots
			 */
			if (
				heros_forge_model > 1000 &&
				material_slot != EQ::textures::armorChest &&
				material_slot != EQ::textures::armorArms &&
				material_slot != EQ::textures::armorWrist &&
				material_slot != EQ::textures::armorLegs
			) {
				heros_forge_model = 0;
			}
		}
	}

	/**
	 * Auto-Convert Hero Model to match the slot
	 *
	 * Otherwise, use the exact Model if model is > 999
	 * Robes for example are 11607 to 12107 in RoF
	 */
	if (EQ::ValueWithin(heros_forge_model, 1, 999)) {
		heros_forge_model *= 100;
		heros_forge_model += material_slot;
	}

	return heros_forge_model;
}

uint32 NPC::GetEquippedItemFromTextureSlot(uint8 material_slot) const
{
	if (material_slot >= EQ::textures::materialCount) {
		return 0;
	}

	const int16 inventory_slot = EQ::InventoryProfile::CalcSlotFromMaterial(material_slot);
	if (inventory_slot == INVALID_INDEX) {
		return 0;
	}

	return equipment[inventory_slot];
}

void Mob::SendArmorAppearance(Client *one_client)
{
	/**
	 * one_client of 0 means sent to all clients
	 *
	 * Despite the fact that OP_NewSpawn and OP_ZoneSpawns include the
	 * armor being worn and its mats, the client doesn't update the display
	 * on arrival of these packets reliably.
	 *
	 * Send Wear changes if mob is a PC race and item is an armor slot.
	 * The other packets work for primary/secondary.
	 */

	LogMobAppearance("[{}]", GetCleanName());

	if (IsPlayerRace(race)) {
		if (!IsClient()) {
			for (uint8 slot_id = 0; slot_id <= EQ::textures::materialCount; ++slot_id) {
				const auto item = database.GetItem(GetEquippedItemFromTextureSlot(slot_id));
				if (item) {
					SendWearChange(slot_id, one_client);
				}
			}
		}
	}

	for (uint8 slot_id = 0; slot_id <= EQ::textures::materialCount; ++slot_id) {
		if (GetTextureProfileMaterial(slot_id)) {
			SendWearChange(slot_id, one_client);
		}
	}
}

void Mob::SendWearChange(uint8 material_slot, Client *one_client)
{
	auto packet = new EQApplicationPacket(OP_WearChange, sizeof(WearChange_Struct));
	auto w      = (WearChange_Struct *) packet->pBuffer;

	w->spawn_id         = GetID();
	w->material         = static_cast<uint32>(GetEquipmentMaterial(material_slot));
	w->elite_material   = IsEliteMaterialItem(material_slot);
	w->hero_forge_model = static_cast<uint32>(GetHerosForgeModel(material_slot));

	if (IsBot()) {
		const auto item_inst = CastToBot()->GetBotItem(EQ::InventoryProfile::CalcSlotFromMaterial(material_slot));
		w->color.Color = item_inst ? item_inst->GetColor() : 0;
	} else {
		w->color.Color = GetEquipmentColor(material_slot);
	}

	w->wear_slot_id = material_slot;

	if (GetRace() != m_last_wearchange_race_id) {
		m_last_seen_wearchange.clear();
		m_last_wearchange_race_id = GetRace();
	}

	// this is a hash-like key to deduplicate packets sent to clients
	// it includes spawn_id, material, elite_material, hero_forge_model, wear_slot_id, and color
	// we send an enormous amount of wearchange packets in brute-force fashion and this is a low cost way to deduplicate them
	// we could remove all the extra wearchanges at the expense of tracing down intermittent visual bugs over a long time
	auto build_key = [&](const WearChange_Struct& s) -> uint64_t {
		uint64_t key = 0;

		key |= static_cast<uint64_t>(s.material & 0xFFF)             << 0;   // 12 bits
		key |= static_cast<uint64_t>(s.elite_material & 0x1)         << 12;  // 1 bit
		key |= static_cast<uint64_t>(s.hero_forge_model & 0xFFFFF)   << 13;  // 20 bits
		key |= static_cast<uint64_t>(GetRace() & 0xFFFF)             << 33;  // 16 bits

		// Optional: Fold in color for appearance differences
		uint8_t folded_color = static_cast<uint8_t>(
			(s.color.Color * 17ull) & 0xFF
		);
		key |= static_cast<uint64_t>(folded_color) << 49;

		return key;
	};


	auto dedupe_key = build_key(*w);
	auto send_if_changed = [&](Client* client) {
		auto& last_key = m_last_seen_wearchange[client->GetID()][material_slot];
		if (last_key == dedupe_key) {
			return;
		}
		last_key = dedupe_key;
		client->QueuePacket(packet, true, Client::CLIENT_CONNECTED);
	};

	if (one_client) {
		send_if_changed(one_client);
	}
	else {
		for (auto& [_, client] : entity_list.GetClientList()) {
			send_if_changed(client);
		}
	}

	safe_delete(packet);
}

void Mob::SendTextureWC(
	uint8 slot,
	uint32 texture,
	uint32 hero_forge_model,
	uint32 elite_material,
	uint32 unknown06,
	uint32 unknown18
)
{
	auto outapp = new EQApplicationPacket(OP_WearChange, sizeof(WearChange_Struct));
	auto w      = (WearChange_Struct *) outapp->pBuffer;

	w->color.Color      = IsClient() ? GetEquipmentColor(slot) : GetArmorTint(slot);
	w->spawn_id         = GetID();
	w->material         = texture;
	w->wear_slot_id     = slot;
	w->unknown06        = unknown06;
	w->elite_material   = elite_material;
	w->hero_forge_model = hero_forge_model;
	w->unknown18        = unknown18;

	SetMobTextureProfile(slot, texture, w->color.Color, hero_forge_model);

	entity_list.QueueClients(this, outapp);
	safe_delete(outapp);
}

void Mob::SetSlotTint(uint8 material_slot, uint8 red_tint, uint8 green_tint, uint8 blue_tint)
{
	uint32 color;
	color = (red_tint & 0xFF) << 16;
	color |= (green_tint & 0xFF) << 8;
	color |= (blue_tint & 0xFF);
	color |= (color) ? (0xFF << 24) : 0;
	armor_tint.Slot[material_slot].Color = color;

	auto outapp = new EQApplicationPacket(OP_WearChange, sizeof(WearChange_Struct));
	auto w      = (WearChange_Struct *) outapp->pBuffer;

	w->spawn_id         = GetID();
	w->material         = GetEquipmentMaterial(material_slot);
	w->hero_forge_model = GetHerosForgeModel(material_slot);
	w->color.Color      = color;
	w->wear_slot_id     = material_slot;

	SetMobTextureProfile(material_slot, texture, color);

	entity_list.QueueClients(this, outapp);
	safe_delete(outapp);
}

void Mob::WearChange(
	uint8 material_slot,
	uint32 texture,
	uint32 color,
	uint32 hero_forge_model
)
{
	armor_tint.Slot[material_slot].Color = color;

	SetMobTextureProfile(material_slot, texture, color, hero_forge_model);

	auto outapp = new EQApplicationPacket(OP_WearChange, sizeof(WearChange_Struct));
	auto w      = (WearChange_Struct *) outapp->pBuffer;

	w->spawn_id         = GetID();
	w->material         = texture;
	w->hero_forge_model = hero_forge_model;
	w->color.Color      = color;
	w->wear_slot_id     = material_slot;

	entity_list.QueueClients(this, outapp);
	safe_delete(outapp);
}
