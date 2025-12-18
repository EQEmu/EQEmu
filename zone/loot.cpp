#include "npc.h"

#include "common/data_verification.h"
#include "common/loot.h"
#include "common/repositories/criteria/content_filter_criteria.h"
#include "common/repositories/global_loot_repository.h"
#include "zone/client.h"
#include "zone/entity.h"
#include "zone/global_loot_manager.h"
#include "zone/mob.h"
#include "zone/quest_parser_collection.h"
#include "zone/zonedb.h"

void NPC::AddLootTable(uint32 loottable_id, bool is_global)
{
	// check if it's a GM spawn
	if (!npctype_id) {
		return;
	}

	if (m_resumed_from_zone_suspend) {
		LogZoneState("NPC [{}] is resuming from zone suspend, skipping", GetCleanName());
		return;
	}

	if (!is_global) {
		m_loot_copper   = 0;
		m_loot_silver   = 0;
		m_loot_gold     = 0;
		m_loot_platinum = 0;
	}

	zone->LoadLootTable(loottable_id);

	const auto *l = zone->GetLootTable(loottable_id);
	if (!l) {
		return;
	}

	LogLootDetail(
		"Attempting to load loot [{}] loottable [{}] ({}) is_global [{}]",
		GetCleanName(),
		loottable_id,
		l->name,
		is_global
	);

	auto content_flags = ContentFlags{
		.min_expansion = l->min_expansion,
		.max_expansion = l->max_expansion,
		.content_flags = l->content_flags,
		.content_flags_disabled = l->content_flags_disabled
	};

	if (!WorldContentService::Instance()->DoesPassContentFiltering(content_flags)) {
		return;
	}

	uint32 min_cash = l->mincash;
	uint32 max_cash = l->maxcash;
	if (min_cash > max_cash) {
		const uint32 t = min_cash;
		min_cash = max_cash;
		max_cash = t;
	}

	uint32 cash = 0;
	if (!is_global) {
		if (max_cash > 0 && l->avgcoin > 0 && EQ::ValueWithin(l->avgcoin, min_cash, max_cash)) {
			const float upper_chance  = static_cast<float>(l->avgcoin - min_cash) /
										static_cast<float>(max_cash - min_cash);
			const float avg_cash_roll = static_cast<float>(zone->random.Real(0.0, 1.0));

			if (avg_cash_roll < upper_chance) {
				cash = zone->random.Int(l->avgcoin, max_cash);
			}
			else {
				cash = zone->random.Int(min_cash, l->avgcoin);
			}
		}
		else {
			cash = zone->random.Int(min_cash, max_cash);
		}
	}

	if (cash != 0) {
		m_loot_platinum = cash / 1000;
		cash -= m_loot_platinum * 1000;

		m_loot_gold = cash / 100;
		cash -= m_loot_gold * 100;

		m_loot_silver = cash / 10;
		cash -= m_loot_silver * 10;

		m_loot_copper = cash;
	}

	const uint32 global_loot_multiplier = RuleI(Zone, GlobalLootMultiplier);
	for (auto    &lte: zone->GetLootTableEntries(loottable_id)) {
		for (uint32 k = 1; k <= (lte.multiplier * global_loot_multiplier); k++) {
			const uint8 drop_limit   = lte.droplimit;
			const uint8 minimum_drop = lte.mindrop;
			const float probability  = lte.probability;

			float drop_chance = 0.0f;
			if (EQ::ValueWithin(probability, 0.0f, 100.0f)) {
				drop_chance = static_cast<float>(zone->random.Real(0.0, 100.0));
			}

			if (probability != 0.0 && (probability == 100.0 || drop_chance <= probability)) {
				AddLootDropTable(lte.lootdrop_id, drop_limit, minimum_drop);
			}
		}
	}

	LogLootDetail(
		"Loaded [{}] Loot Table [{}] is_global [{}]",
		GetCleanName(),
		loottable_id,
		is_global
	);
}

void NPC::AddLootDropTable(uint32 lootdrop_id, uint8 drop_limit, uint8 min_drop)
{
	const auto l  = zone->GetLootdrop(lootdrop_id);
	const auto le = zone->GetLootdropEntries(lootdrop_id);
	if (l.id == 0 || le.empty()) {
		return;
	}

	// if this lootdrop is droplimit=0 and mindrop 0, scan list once and return
	if (drop_limit == 0 && min_drop == 0) {
		for (const auto &e: le) {
			for (int j = 0; j < e.multiplier; ++j) {
				if (zone->random.Real(0.0, 100.0) <= e.chance && MeetsLootDropLevelRequirements(e, true)) {
					const EQ::ItemData *database_item = database.GetItem(e.item_id);
					AddLootDrop(database_item, e);
					LogLootDetail(
						"---- NPC (Rolled) [{}] Lootdrop [{}] Item [{}] ({}) Chance [{}] Multiplier [{}]",
						GetCleanName(),
						lootdrop_id,
						database_item->Name,
						e.item_id,
						e.chance,
						e.multiplier
					);
				}
			}
		}

		return;
	}

	if (le.size() > 100 && drop_limit == 0) {
		drop_limit = 10;
	}

	if (drop_limit < min_drop) {
		drop_limit = min_drop;
	}

	float roll_t                   = 0.0f;
	float no_loot_prob             = 1.0f;
	bool  roll_table_chance_bypass = false;
	bool  active_item_list         = false;

	for (const auto &e: le) {
		const EQ::ItemData *db_item = database.GetItem(e.item_id);
		if (db_item && MeetsLootDropLevelRequirements(e)) {
			roll_t += e.chance;

			if (e.chance >= 100) {
				roll_table_chance_bypass = true;
			}
			else {
				no_loot_prob *= (100 - e.chance) / 100.0f;
			}

			active_item_list = true;
		}
	}

	if (!active_item_list) {
		return;
	}

	// This will pick one item per iteration until mindrop.
	// Don't let the compare against chance fool you.
	// The roll isn't 0-100, its 0-total and it picks the item, we're just
	// looping to find the lucky item, descremening otherwise. This is ok,
	// items with chance 60 are 6 times more likely than items chance 10.
	int drops = 0;

	// translate above for loop using l and le
	for (int i = 0; i < drop_limit; ++i) {
		if (drops < min_drop || roll_table_chance_bypass || (float) zone->random.Real(0.0, 1.0) >= no_loot_prob) {
			float           roll = (float) zone->random.Real(0.0, roll_t);
			for (const auto &e: le) {
				const auto *db_item = database.GetItem(e.item_id);
				if (db_item) {
					// if it doesn't meet the requirements do nothing
					if (!MeetsLootDropLevelRequirements(e)) {
						continue;
					}

					if (roll < e.chance) {
						AddLootDrop(db_item, e);
						drops++;

						uint8 charges = e.multiplier;
						charges = EQ::ClampLower(charges, static_cast<uint8>(1));

						for (int k = 1; k < charges; ++k) {
							float c_roll = static_cast<float>(zone->random.Real(0.0, 100.0));
							if (c_roll <= e.chance) {
								AddLootDrop(db_item, e);
							}
						}

						break;
					}
					else {
						roll -= e.chance;
					}
				}
			}
		}
	}

	UpdateEquipmentLight();
}

bool NPC::MeetsLootDropLevelRequirements(LootdropEntriesRepository::LootdropEntries loot_drop, bool verbose)
{
	if (loot_drop.npc_min_level > 0 && GetLevel() < loot_drop.npc_min_level) {
		if (verbose) {
			LogLootDetail(
				"NPC [{}] does not meet loot_drop level requirements (min_level) level [{}] current [{}] for item [{}]",
				GetCleanName(),
				loot_drop.npc_min_level,
				GetLevel(),
				database.CreateItemLink(loot_drop.item_id)
			);
		}
		return false;
	}

	if (loot_drop.npc_max_level > 0 && GetLevel() > loot_drop.npc_max_level) {
		if (verbose) {
			LogLootDetail(
				"NPC [{}] does not meet loot_drop level requirements (max_level) level [{}] current [{}] for item [{}]",
				GetCleanName(),
				loot_drop.npc_max_level,
				GetLevel(),
				database.CreateItemLink(loot_drop.item_id)
			);
		}
		return false;
	}

	return true;
}

//if itemlist is null, just send wear changes
void NPC::AddLootDrop(
	const EQ::ItemData *item2,
	LootdropEntriesRepository::LootdropEntries loot_drop,
	bool wear_change,
	uint32 augment_one,
	uint32 augment_two,
	uint32 augment_three,
	uint32 augment_four,
	uint32 augment_five,
	uint32 augment_six
)
{
	if (m_resumed_from_zone_suspend) {
		LogZoneState("NPC [{}] is resuming from zone suspend, skipping", GetCleanName());
		return;
	}

	if (!item2) {
		return;
	}

	auto item = new LootItem;

	if (EQEmuLogSys::Instance()->log_settings[Logs::Loot].is_category_enabled == 1) {
		EQ::SayLinkEngine linker;
		linker.SetLinkType(EQ::saylink::SayLinkItemData);
		linker.SetItemData(item2);

		LogLoot(
			"NPC [{}] Item ({}) [{}] charges [{}] chance [{}] trivial min/max [{}/{}] npc min/max [{}/{}]",
			GetName(),
			item2->ID,
			linker.GenerateLink(),
			loot_drop.item_charges,
			loot_drop.chance,
			loot_drop.trivial_min_level,
			loot_drop.trivial_max_level,
			loot_drop.npc_min_level,
			loot_drop.npc_max_level
		);
	}

	EQApplicationPacket *outapp               = nullptr;
	WearChange_Struct   *p_wear_change_struct = nullptr;
	if (wear_change) {
		outapp               = new EQApplicationPacket(OP_WearChange, sizeof(WearChange_Struct));
		p_wear_change_struct = (WearChange_Struct *) outapp->pBuffer;
		p_wear_change_struct->spawn_id = GetID();
		p_wear_change_struct->material = 0;
	}

	item->item_id           = item2->ID;
	item->charges           = loot_drop.item_charges;
	item->aug_1             = augment_one;
	item->aug_2             = augment_two;
	item->aug_3             = augment_three;
	item->aug_4             = augment_four;
	item->aug_5             = augment_five;
	item->aug_6             = augment_six;
	item->attuned           = false;
	item->trivial_min_level = loot_drop.trivial_min_level;
	item->trivial_max_level = loot_drop.trivial_max_level;
	item->equip_slot        = EQ::invslot::SLOT_INVALID;

	// unsure if required to equip, YOLO for now
	if (item2->ItemType == EQ::item::ItemTypeBow) {
		SetBowEquipped(true);
	}

	if (item2->ItemType == EQ::item::ItemTypeArrow) {
		SetArrowEquipped(true);
	}

	bool found = false; // track if we found an empty slot we fit into

	int found_slot = INVALID_INDEX; // for multi-slot items

	auto *inst = database.CreateItem(
		item2->ID,
		loot_drop.item_charges,
		augment_one,
		augment_two,
		augment_three,
		augment_four,
		augment_five,
		augment_six
	);

	if (!inst) {
		return;
	}

	if (loot_drop.equip_item > 0) {
		uint8              equipment_slot = UINT8_MAX;
		const EQ::ItemData *compitem      = nullptr;

		// Equip rules are as follows:
		// If the item has the NoPet flag set it will not be equipped.
		// An empty slot takes priority. The first empty one that an item can
		// fit into will be the one picked for the item.
		// AC is the primary choice for which item gets picked for a slot.
		// If AC is identical HP is considered next.
		// If an item can fit into multiple slots we'll pick the last one where
		// it is an improvement.

		if (!item2->NoPet) {
			for (int i = EQ::invslot::EQUIPMENT_BEGIN; !found && i <= EQ::invslot::EQUIPMENT_END; i++) {
				const uint32 slots = (1 << i);
				if (item2->Slots & slots) {
					if (equipment[i]) {
						compitem = database.GetItem(equipment[i]);
						if (!compitem) {
							continue;
						}

						if (item2->AC > compitem->AC || (item2->AC == compitem->AC && item2->HP > compitem->HP)) {
							// item would be an upgrade
							// check if we're multi-slot, if yes then we have to keep
							// looking in case any of the other slots we can fit into are empty.
							if (item2->Slots != slots) {
								found_slot = i;
							}
							else {
								// Unequip old item
								auto *old_item = GetItem(i);
								if (!old_item) {
									continue;
								}

								old_item->equip_slot = EQ::invslot::SLOT_INVALID;

								equipment[i] = item2->ID;

								found_slot = i;
								found      = true;
							}
						}
					}
					else {
						equipment[i] = item2->ID;

						found_slot = i;
						found      = true;
					}
				}
			}
		}

		// Possible slot was found but not selected. Pick it now.
		if (!found && found_slot >= 0) {
			equipment[found_slot] = item2->ID;

			found = true;
		}

		uint32 equipment_material;
		if (
			item2->Material <= 0 ||
			(
				item2->Slots & (
					(1 << EQ::invslot::slotPrimary) |
					(1 << EQ::invslot::slotSecondary)
				)
			)
			) {
			equipment_material = Strings::ToUnsignedInt(&item2->IDFile[2]);
		}
		else {
			equipment_material = item2->Material;
		}

		if (found_slot == EQ::invslot::slotPrimary) {
			equipment_slot = EQ::textures::weaponPrimary;

			if (item2->Damage > 0) {
				SendAddPlayerState(PlayerState::PrimaryWeaponEquipped);

				if (!RuleB(Combat, ClassicNPCBackstab)) {
					SetFacestab(true);
				}
			}

			if (item2->IsType2HWeapon()) {
				SetTwoHanderEquipped(true);
			}
		}
		else if (
			found_slot == EQ::invslot::slotSecondary &&
			(
				GetOwner() ||
				(CanThisClassDualWield() && zone->random.Roll(NPC_DW_CHANCE)) ||
				item2->Damage == 0
			) &&
			(
				item2->IsType1HWeapon() ||
				item2->ItemType == EQ::item::ItemTypeShield ||
				item2->ItemType == EQ::item::ItemTypeLight
			)
			) {
			equipment_slot = EQ::textures::weaponSecondary;

			if (item2->Damage > 0) {
				SendAddPlayerState(PlayerState::SecondaryWeaponEquipped);
			}
		}
		else if (found_slot == EQ::invslot::slotHead) {
			equipment_slot = EQ::textures::armorHead;
		}
		else if (found_slot == EQ::invslot::slotChest) {
			equipment_slot = EQ::textures::armorChest;
		}
		else if (found_slot == EQ::invslot::slotArms) {
			equipment_slot = EQ::textures::armorArms;
		}
		else if (EQ::ValueWithin(found_slot, EQ::invslot::slotWrist1, EQ::invslot::slotWrist2)) {
			equipment_slot = EQ::textures::armorWrist;
		}
		else if (found_slot == EQ::invslot::slotHands) {
			equipment_slot = EQ::textures::armorHands;
		}
		else if (found_slot == EQ::invslot::slotLegs) {
			equipment_slot = EQ::textures::armorLegs;
		}
		else if (found_slot == EQ::invslot::slotFeet) {
			equipment_slot = EQ::textures::armorFeet;
		}

		if (equipment_slot != UINT8_MAX) {
			if (wear_change) {
				p_wear_change_struct->wear_slot_id = equipment_slot;
				p_wear_change_struct->material     = equipment_material;
			}
		}

		if (found) {
			item->equip_slot = found_slot;
		}
	}

	if (found_slot != INVALID_INDEX) {
		GetInv().PutItem(found_slot, *inst);
	}

	if (parse->HasQuestSub(GetNPCTypeID(), EVENT_LOOT_ADDED)) {
		std::vector<std::any> args = {inst};
		parse->EventNPC(EVENT_LOOT_ADDED, this, nullptr, "", 0, &args);
	}

	item->lootdrop_id = loot_drop.lootdrop_id;
	m_loot_items.push_back(item);

	if (found) {
		CalcBonuses();
	}

	if (IsRecordLootStats()) {
		m_rolled_items.emplace_back(item->item_id);
	}

	if (wear_change && outapp) {
		entity_list.QueueClients(this, outapp);
		safe_delete(outapp);
	}

	UpdateEquipmentLight();

	if (UpdateActiveLight()) {
		SendAppearancePacket(AppearanceType::Light, GetActiveLightType());
	}

	safe_delete(inst);
}

void NPC::AddItem(const EQ::ItemData *item, uint16 charges, bool equip_item)
{
	auto l = LootdropEntriesRepository::NewNpcEntity();

	l.equip_item   = static_cast<uint8>(equip_item ? 1 : 0);
	l.item_charges = charges;

	AddLootDrop(item, l, true);
}

void NPC::AddItem(
	uint32 item_id,
	uint16 charges,
	bool equip_item,
	uint32 augment_one,
	uint32 augment_two,
	uint32 augment_three,
	uint32 augment_four,
	uint32 augment_five,
	uint32 augment_six
)
{
	const auto *item = database.GetItem(item_id);
	if (!item) {
		return;
	}

	auto l = LootdropEntriesRepository::NewNpcEntity();

	l.equip_item   = static_cast<uint8>(equip_item ? 1 : 0);
	l.item_charges = charges;

	AddLootDrop(
		item,
		l,
		true,
		augment_one,
		augment_two,
		augment_three,
		augment_four,
		augment_five,
		augment_six
	);
}

void NPC::AddLootTable()
{
	AddLootTable(m_loottable_id);
}

void NPC::CheckGlobalLootTables()
{
	const auto &l = zone->GetGlobalLootTables(this);
	for (const auto &e: l) {
		AddLootTable(e, true);
	}
}

void ZoneDatabase::LoadGlobalLoot()
{
	const auto &l = GlobalLootRepository::GetWhere(
		*this,
		fmt::format(
			"`enabled` = 1 {}",
			ContentFilterCriteria::apply()
		)
	);

	if (l.empty()) {
		return;
	}

	LogInfo(
		"Loaded [{}] Global Loot Entr{}.",
		Strings::Commify(l.size()),
		l.size() != 1 ? "ies" : "y"
	);

	const std::string &zone_id = std::to_string(zone->GetZoneID());

	for (const auto &e: l) {
		if (!e.zone.empty()) {
			const auto &zones = Strings::Split(e.zone, "|");

			if (!Strings::Contains(zones, zone_id)) {
				continue;
			}
		}

		GlobalLootEntry gle(e.id, e.loottable_id, e.description);

		if (e.min_level) {
			gle.AddRule(GlobalLoot::RuleTypes::LevelMin, e.min_level);
		}

		if (e.max_level) {
			gle.AddRule(GlobalLoot::RuleTypes::LevelMax, e.max_level);
		}

		if (e.rare) {
			gle.AddRule(GlobalLoot::RuleTypes::Rare, e.rare);
		}

		if (e.raid) {
			gle.AddRule(GlobalLoot::RuleTypes::Raid, e.raid);
		}

		if (!e.race.empty()) {
			const auto &races = Strings::Split(e.race, "|");

			for (const auto &r: races) {
				gle.AddRule(GlobalLoot::RuleTypes::Race, Strings::ToInt(r));
			}
		}

		if (!e.class_.empty()) {
			const auto &classes = Strings::Split(e.class_, "|");

			for (const auto &c: classes) {
				gle.AddRule(GlobalLoot::RuleTypes::Class, Strings::ToInt(c));
			}
		}

		if (!e.bodytype.empty()) {
			const auto &bodytypes = Strings::Split(e.bodytype, "|");

			for (const auto &b: bodytypes) {
				gle.AddRule(GlobalLoot::RuleTypes::BodyType, Strings::ToInt(b));
			}
		}

		if (e.hot_zone) {
			gle.AddRule(GlobalLoot::RuleTypes::HotZone, e.hot_zone);
		}

		zone->AddGlobalLootEntry(gle);
	}
}


LootItem *NPC::GetItem(int slot_id)
{
	LootItems::iterator cur, end;
	cur = m_loot_items.begin();
	end = m_loot_items.end();
	for (; cur != end; ++cur) {
		LootItem *item = *cur;
		if (item && item->equip_slot == slot_id) {
			return item;
		}
	}
	return (nullptr);
}

void NPC::RemoveItem(uint32 item_id, uint16 quantity, uint16 slot)
{
	LootItems::iterator cur, end;
	cur = m_loot_items.begin();
	end = m_loot_items.end();
	for (; cur != end; ++cur) {
		LootItem *item = *cur;
		if (item->item_id == item_id && slot <= 0 && quantity <= 0) {
			m_loot_items.erase(cur);
			safe_delete(item);
			UpdateEquipmentLight();
			if (UpdateActiveLight()) { SendAppearancePacket(AppearanceType::Light, GetActiveLightType()); }
			return;
		}
		else if (item->item_id == item_id && item->equip_slot == slot && quantity >= 1) {
			if (item->charges <= quantity) {
				m_loot_items.erase(cur);
				UpdateEquipmentLight();
				if (UpdateActiveLight()) {
					SendAppearancePacket(AppearanceType::Light, GetActiveLightType());
				}
				safe_delete(item);
			}
			else {
				item->charges -= quantity;
			}
			return;
		}
	}
}

void NPC::CheckTrivialMinMaxLevelDrop(Mob *killer)
{
	if (killer == nullptr || !killer->IsClient()) {
		return;
	}

	uint16 killer_level = killer->GetLevel();
	uint8  material;

	auto cur = m_loot_items.begin();
	while (cur != m_loot_items.end()) {
		if (!(*cur)) {
			return;
		}

		uint16 trivial_min_level     = (*cur)->trivial_min_level;
		uint16 trivial_max_level     = (*cur)->trivial_max_level;
		bool   fits_trivial_criteria = (
			(trivial_min_level > 0 && killer_level < trivial_min_level) ||
			(trivial_max_level > 0 && killer_level > trivial_max_level)
		);

		if (fits_trivial_criteria) {
			material = EQ::InventoryProfile::CalcMaterialFromSlot((*cur)->equip_slot);
			if (material != EQ::textures::materialInvalid) {
				SendWearChange(material);
			}

			cur = m_loot_items.erase(cur);
			continue;
		}
		++cur;
	}

	UpdateEquipmentLight();
	if (UpdateActiveLight()) {
		SendAppearancePacket(AppearanceType::Light, GetActiveLightType());
	}
}

void NPC::ClearLootItems()
{
	LootItems::iterator cur, end;
	cur = m_loot_items.begin();
	end = m_loot_items.end();
	for (; cur != end; ++cur) {
		LootItem *item = *cur;
		safe_delete(item);
	}
	m_loot_items.clear();

	UpdateEquipmentLight();
	if (UpdateActiveLight()) {
		SendAppearancePacket(AppearanceType::Light, GetActiveLightType());
	}
}

void NPC::QueryLoot(Client *to, bool is_pet_query)
{
	if (!m_loot_items.empty()) {
		if (!is_pet_query) {
			to->Message(
				Chat::White,
				fmt::format(
					"Loot | {} ({}) ID: {} Loottable ID: {}",
					GetName(),
					GetID(),
					GetNPCTypeID(),
					GetLoottableID()
				).c_str()
			);
		}

		int item_count = 0;

		for (auto current_item: m_loot_items) {
			int item_number = (item_count + 1);
			if (!current_item) {
				LogError("ItemList error, null item.");
				continue;
			}

			if (!current_item->item_id || !database.GetItem(current_item->item_id)) {
				LogError("Database error, invalid item.");
				continue;
			}

			EQ::SayLinkEngine linker;
			linker.SetLinkType(EQ::saylink::SayLinkLootItem);
			linker.SetLootData(current_item);

			to->Message(
				Chat::White,
				fmt::format(
					"Item {} | {} ({}){}",
					item_number,
					linker.GenerateLink().c_str(),
					current_item->item_id,
					(
						current_item->charges > 1 ?
							fmt::format(
								" Amount: {}",
								current_item->charges
							) :
							""
					)
				).c_str()
			);
			item_count++;
		}
	}

	if (!is_pet_query) {
		if (m_loot_platinum || m_loot_gold || m_loot_silver || m_loot_copper) {
			to->Message(
				Chat::White,
				fmt::format(
					"Money | {}",
					Strings::Money(
						m_loot_platinum,
						m_loot_gold,
						m_loot_silver,
						m_loot_copper
					)
				).c_str()
			);
		}
	}
}

bool NPC::HasItem(uint32 item_id)
{
	if (!database.GetItem(item_id)) {
		return false;
	}

	for (auto loot_item: m_loot_items) {
		if (!loot_item) {
			LogError("NPC::HasItem() - ItemList error, null item");
			continue;
		}

		if (!loot_item->item_id || !database.GetItem(loot_item->item_id)) {
			LogError("NPC::HasItem() - Database error, invalid item");
			continue;
		}

		if (loot_item->item_id == item_id) {
			return true;
		}
	}
	return false;
}

uint32 NPC::CountItem(uint32 item_id)
{
	uint32 item_count = 0;
	if (!database.GetItem(item_id)) {
		return item_count;
	}

	for (auto loot_item: m_loot_items) {
		if (!loot_item) {
			LogError("NPC::CountItem() - ItemList error, null item");
			continue;
		}

		if (!loot_item->item_id || !database.GetItem(loot_item->item_id)) {
			LogError("NPC::CountItem() - Database error, invalid item");
			continue;
		}

		if (loot_item->item_id == item_id) {
			item_count += loot_item->charges > 0 ? loot_item->charges : 1;
		}
	}
	return item_count;
}

uint32 NPC::GetLootItemIDBySlot(uint16 loot_slot)
{
	for (auto loot_item: m_loot_items) {
		if (loot_item->lootslot == loot_slot) {
			return loot_item->item_id;
		}
	}
	return 0;
}

uint16 NPC::GetFirstLootSlotByItemID(uint32 item_id)
{
	for (auto loot_item: m_loot_items) {
		if (loot_item->item_id == item_id) {
			return loot_item->lootslot;
		}
	}
	return 0;
}

void NPC::AddLootCash(
	uint32 in_copper,
	uint32 in_silver,
	uint32 in_gold,
	uint32 in_platinum
)
{
	m_loot_copper   = in_copper >= 0 ? in_copper : 0;
	m_loot_silver   = in_silver >= 0 ? in_silver : 0;
	m_loot_gold     = in_gold >= 0 ? in_gold : 0;
	m_loot_platinum = in_platinum >= 0 ? in_platinum : 0;
}

void NPC::RemoveLootCash()
{
	m_loot_copper   = 0;
	m_loot_silver   = 0;
	m_loot_gold     = 0;
	m_loot_platinum = 0;
}
