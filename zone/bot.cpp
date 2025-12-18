/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "bot.h"

#include "common/data_verification.h"
#include "common/repositories/bot_inventories_repository.h"
#include "common/repositories/bot_spell_settings_repository.h"
#include "common/repositories/bot_starting_items_repository.h"
#include "common/repositories/criteria/content_filter_criteria.h"
#include "common/skill_caps.h"
#include "zone/doors.h"
#include "zone/lua_parser.h"
#include "zone/object.h"
#include "zone/quest_parser_collection.h"
#include "zone/raids.h"

/*
TODO bot rewrite:
--command cleanup remaining commands (move to new help window, make more descriptive)
--Add quest methods for functions
*/

// This constructor is used during the bot create command
Bot::Bot(NPCType *npcTypeData, Client* botOwner) : NPC(npcTypeData, nullptr, glm::vec4(), Ground, false), rest_timer(1), m_ping_timer(1) {
	GiveNPCTypeData(npcTypeData);

	if (botOwner) {
		SetBotOwner(botOwner);
		_botOwnerCharacterID = botOwner->CharacterID();
	} else {
		SetBotOwner(nullptr);
		_botOwnerCharacterID = 0;
	}

	m_inv.SetInventoryVersion(EQ::versions::MobVersion::Bot);
	m_inv.SetGMInventory(false); // bot expansions are not currently implemented (defaults to static)

	_guildRank = 0;
	_guildId = 0;
	_lastTotalPlayTime = 0;
	_startTotalPlayTime = time(&_startTotalPlayTime);
	_lastZoneId = 0;
	_baseMR = npcTypeData->MR;
	_baseCR = npcTypeData->CR;
	_baseDR = npcTypeData->DR;
	_baseFR = npcTypeData->FR;
	_basePR = npcTypeData->PR;
	_baseCorrup = npcTypeData->Corrup;
	_baseAC = npcTypeData->AC;
	_baseSTR = npcTypeData->STR;
	_baseSTA = npcTypeData->STA;
	_baseDEX = npcTypeData->DEX;
	_baseAGI = npcTypeData->AGI;
	_baseINT = npcTypeData->INT;
	_baseWIS = npcTypeData->WIS;
	_baseCHA = npcTypeData->CHA;
	_baseATK = npcTypeData->ATK;
	_baseRace = npcTypeData->race;
	_baseGender = npcTypeData->gender;
	RestRegenHP = 0;
	RestRegenMana = 0;
	RestRegenEndurance = 0;

	SetBotID(0);
	SetBotSpellID(0);
	SetSpawnStatus(false);
	SetBotCharmer(false);
	SetDefaultBotStance();
	SetTaunting((GetClass() == Class::Warrior || GetClass() == Class::Paladin || GetClass() == Class::ShadowKnight) && (GetBotStance() == Stance::Aggressive));

	SetPauseAI(false);

	m_combat_jitter_timer.Disable();
	m_auto_save_timer.Disable();
	m_rogue_evade_timer.Disable();
	m_monk_evade_timer.Disable();
	m_auto_defend_timer.Disable();
	SetGuardFlag(false);
	SetHoldFlag(false);
	SetAttackFlag(false);
	SetCombatRoundForAlerts(false);
	SetAttackingFlag(false);
	SetPullFlag(false);
	SetPullingFlag(false);
	SetReturningFlag(false);
	SetIsUsingItemClick(false);
	m_previous_pet_order = PetOrder::Guard;

	rest_timer.Disable();
	m_ping_timer.Disable();

	LoadDefaultBotSettings();
	SetCastedSpellType(UINT16_MAX);
	SetTempSpellType(UINT16_MAX);
	SetCommandedSpell(false);
	SetPullingSpell(false);

	// Do this once and only in this constructor
	GenerateAppearance();
	GenerateBaseStats();
	bot_timers.clear();
	bot_blocked_buffs.clear();
	_spell_target_list.clear();
	_group_spell_target_list.clear();
	SetStoredRaid(nullptr);
	SetVerifiedRaid(false);
	p_raid_instance = nullptr;

	// Calculate HitPoints Last As It Uses Base Stats
	current_hp = GenerateBaseHitPoints();
	current_mana = GenerateBaseManaPoints();
	cur_end = CalcBaseEndurance();
	hp_regen = CalcHPRegen();
	mana_regen = CalcManaRegen();
	end_regen = CalcEnduranceRegen();

	SetExtraHaste(database.botdb.GetBotExtraHasteByID(GetBotID()), false);

	strcpy(name, GetCleanName());
	memset(&_botInspectMessage, 0, sizeof(InspectMessage_Struct));
}

// This constructor is used when the bot is loaded out of the database
Bot::Bot(
	uint32 botID,
	uint32 botOwnerCharacterID,
	uint32 botSpellsID,
	double totalPlayTime,
	uint32 lastZoneId,
	NPCType *npcTypeData
)
	: NPC(npcTypeData, nullptr, glm::vec4(), Ground, false), rest_timer(1), m_ping_timer(1)
{
	GiveNPCTypeData(npcTypeData);

	_botOwnerCharacterID = botOwnerCharacterID;
	if (_botOwnerCharacterID > 0)
		SetBotOwner(entity_list.GetClientByCharID(_botOwnerCharacterID));

	auto bot_owner = GetBotOwner();

	m_inv.SetInventoryVersion(EQ::versions::MobVersion::Bot);
	m_inv.SetGMInventory(false); // bot expansions are not currently implemented (defaults to static)

	_guildRank = 0;
	_guildId = 0;
	_lastTotalPlayTime = totalPlayTime;
	_startTotalPlayTime = time(&_startTotalPlayTime);
	_lastZoneId = lastZoneId;
	berserk = false;
	_baseMR = npcTypeData->MR;
	_baseCR = npcTypeData->CR;
	_baseDR = npcTypeData->DR;
	_baseFR = npcTypeData->FR;
	_basePR = npcTypeData->PR;
	_baseCorrup = npcTypeData->Corrup;
	_baseAC = npcTypeData->AC;
	_baseSTR = npcTypeData->STR;
	_baseSTA = npcTypeData->STA;
	_baseDEX = npcTypeData->DEX;
	_baseAGI = npcTypeData->AGI;
	_baseINT = npcTypeData->INT;
	_baseWIS = npcTypeData->WIS;
	_baseCHA = npcTypeData->CHA;
	_baseATK = npcTypeData->ATK;
	_baseRace = npcTypeData->race;
	_baseGender = npcTypeData->gender;
	current_hp = npcTypeData->current_hp;
	current_mana = npcTypeData->Mana;
	RestRegenHP = 0;
	RestRegenMana = 0;
	RestRegenEndurance = 0;
	SetBotID(botID);
	SetBotSpellID(botSpellsID);
	SetSpawnStatus(false);
	SetBotCharmer(false);
	SetCastedSpellType(UINT16_MAX);
	SetTempSpellType(UINT16_MAX);
	SetCommandedSpell(false);
	SetPullingSpell(false);

	bool stance_flag = false;
	if (!database.botdb.LoadStance(this, stance_flag) && bot_owner) {
		bot_owner->Message(
			Chat::White,
			fmt::format(
				"Failed to load stance for '{}'.",
				GetCleanName()
			).c_str()
		);
	}

	if (!stance_flag && bot_owner) {
		bot_owner->Message(
			Chat::White,
			fmt::format(
				"Could not locate stance for '{}'.",
				GetCleanName()
			).c_str()
		);
	}

	SetTaunting((GetClass() == Class::Warrior || GetClass() == Class::Paladin || GetClass() == Class::ShadowKnight) && (GetBotStance() == Stance::Aggressive));
	SetPauseAI(false);

	m_combat_jitter_timer.Disable();
	m_auto_save_timer.Disable();
	m_rogue_evade_timer.Disable();
	m_monk_evade_timer.Disable();
	m_auto_defend_timer.Disable();
	SetGuardFlag(false);
	SetHoldFlag(false);
	SetAttackFlag(false);
	SetCombatRoundForAlerts(false);
	SetAttackingFlag(false);
	SetPullFlag(false);
	SetPullingFlag(false);
	SetReturningFlag(false);
	SetIsUsingItemClick(false);
	m_previous_pet_order = PetOrder::Guard;

	rest_timer.Disable();
	m_ping_timer.Disable();

	strcpy(name, GetCleanName());

	memset(&_botInspectMessage, 0, sizeof(InspectMessage_Struct));

	database.botdb.LoadInspectMessage(GetBotID(), _botInspectMessage);

	EquipBot();

	m_combat_jitter_timer.Start();

	if (GetClass() == Class::Rogue) {
		m_rogue_evade_timer.Start();
	}

	if (GetClass() == Class::Monk) {
		m_monk_evade_timer.Start();
	}

	m_CastingRoles.GroupHealer = false;
	m_CastingRoles.GroupSlower = false;
	m_CastingRoles.GroupNuker = false;
	m_CastingRoles.GroupDoter = false;

	GenerateBaseStats();

	bot_timers.clear();
	database.botdb.LoadTimers(this);

	LoadDefaultBotSettings();
	database.botdb.LoadBotSettings(this);

	if (RuleB(Bots, AllowBotBlockedBuffs)) {
		bot_blocked_buffs.clear();
		database.botdb.LoadBotBlockedBuffs(this);
	}

	_spell_target_list.clear();
	_group_spell_target_list.clear();
	SetStoredRaid(nullptr);
	SetVerifiedRaid(false);
	p_raid_instance = nullptr;
	LoadAAs();

	if (database.botdb.LoadBuffs(this)) {
		//reapply some buffs
		uint32 buff_count = GetMaxBuffSlots();
		for (uint32 j1 = 0; j1 < buff_count; j1++) {
			if (!IsValidSpell(buffs[j1].spellid)) {
				continue;
			}

			const SPDat_Spell_Struct& spell = spells[buffs[j1].spellid];

			if (int NimbusEffect = GetSpellNimbusEffect(buffs[j1].spellid); NimbusEffect && !IsNimbusEffectActive(NimbusEffect)) {
				SendSpellEffect(NimbusEffect, 500, 0, 1, 3000, true);
			}

			for (int x1 = 0; x1 < EFFECT_COUNT; x1++) {
				switch (spell.effect_id[x1]) {
					case SpellEffect::IllusionCopy:
					case SpellEffect::Illusion: {
						if (GetIllusionBlock()) {
							break;
						}

						if (spell.base_value[x1] == -1) {
							if (gender == Gender::Female) {
								gender = Gender::Male;
							} else if (gender == Gender::Male) {
								gender = Gender::Female;
							}

							SendIllusionPacket(
								AppearanceStruct{
									.gender_id = gender,
									.race_id = GetRace(),
								}
							);
						} else if (spell.base_value[x1] == -2) // WTF IS THIS
						{
							if (GetRace() == Race::Iksar || GetRace() == Race::VahShir || GetRace() <= Race::Gnome) {
								SendIllusionPacket(
									AppearanceStruct{
										.gender_id = GetGender(),
										.helmet_texture = static_cast<uint8>(spell.max_value[x1]),
										.race_id = GetRace(),
										.texture = static_cast<uint8>(spell.limit_value[x1]),
									}
								);
							}
						} else if (spell.max_value[x1] > 0) {
							SendIllusionPacket(
								AppearanceStruct{
									.helmet_texture = static_cast<uint8>(spell.max_value[x1]),
									.race_id = static_cast<uint16>(spell.base_value[x1]),
									.texture = static_cast<uint8>(spell.limit_value[x1]),
								}
							);
						} else {
							SendIllusionPacket(
								AppearanceStruct{
									.helmet_texture = static_cast<uint8>(spell.max_value[x1]),
									.race_id = static_cast<uint16>(spell.base_value[x1]),
									.texture = static_cast<uint8>(spell.limit_value[x1]),
								}
							);
						}

						switch (spell.base_value[x1]) {
							case Race::Ogre:
								SendAppearancePacket(AppearanceType::Size, 9);
								break;
							case Race::Troll:
								SendAppearancePacket(AppearanceType::Size, 8);
								break;
							case Race::VahShir:
							case Race::Barbarian:
								SendAppearancePacket(AppearanceType::Size, 7);
								break;
							case Race::HalfElf:
							case Race::WoodElf:
							case Race::DarkElf:
							case Race::Froglok2:
								SendAppearancePacket(AppearanceType::Size, 5);
								break;
							case Race::Dwarf:
								SendAppearancePacket(AppearanceType::Size, 4);
								break;
							case Race::Halfling:
							case Race::Gnome:
								SendAppearancePacket(AppearanceType::Size, 3);
								break;
							default:
								SendAppearancePacket(AppearanceType::Size, 6);
								break;
						}
						break;
					}
					case SpellEffect::Silence:
					{
						Silence(true);
						break;
					}
					case SpellEffect::Amnesia:
					{
						Amnesia(true);
						break;
					}
					case SpellEffect::DivineAura:
					{
						invulnerable = true;
						break;
					}
					case SpellEffect::Invisibility2:
					case SpellEffect::Invisibility:
					{
						invisible = true;
						SendAppearancePacket(AppearanceType::Invisibility, 1);
						break;
					}
					case SpellEffect::Levitate:
					{
						if (!zone->CanLevitate())
						{
								SendAppearancePacket(AppearanceType::FlyMode, 0);
								BuffFadeByEffect(SpellEffect::Levitate);
						}
						else {
							SendAppearancePacket(AppearanceType::FlyMode, 2);
						}
						break;
					}
					case SpellEffect::InvisVsUndead2:
					case SpellEffect::InvisVsUndead:
					{
						invisible_undead = true;
						break;
					}
					case SpellEffect::InvisVsAnimals:
					{
						invisible_animals = true;
						break;
					}
					case SpellEffect::AddMeleeProc:
					case SpellEffect::WeaponProc:
					{
						AddProcToWeapon(GetProcID(buffs[j1].spellid, x1), false, 100 + spells[buffs[j1].spellid].limit_value[x1], buffs[j1].spellid, buffs[j1].casterlevel);
						break;
					}
					case SpellEffect::DefensiveProc:
					{
						AddDefensiveProc(GetProcID(buffs[j1].spellid, x1), 100 + spells[buffs[j1].spellid].limit_value[x1], buffs[j1].spellid);
						break;
					}
					case SpellEffect::RangedProc:
					{
						AddRangedProc(GetProcID(buffs[j1].spellid, x1), 100 + spells[buffs[j1].spellid].limit_value[x1], buffs[j1].spellid);
						break;
					}
				}
			}
		}
	}

	CalcBotStats(false);
	hp_regen = CalcHPRegen();
	mana_regen = CalcManaRegen();
	end_regen = CalcEnduranceRegen();
	if (current_hp > max_hp)
		current_hp = max_hp;

	if (current_hp <= 0) {
		if (RuleB(Spells, BuffsFadeOnDeath)) {
			BuffFadeNonPersistDeath();
		}

		if (RuleB(Bots, ResurrectionSickness)) {
			int resurrection_sickness_spell_id = (
				RuleB(Bots, OldRaceRezEffects) &&
				(
					GetRace() == Race::Barbarian ||
					GetRace() == Race::Dwarf ||
					GetRace() == Race::Troll ||
					GetRace() == Race::Ogre
				) ?
				RuleI(Bots, OldResurrectionSicknessSpell) :
				RuleI(Bots, ResurrectionSicknessSpell)
			);
			SetHP(max_hp / 5);
			SetMana(0);
			SpellOnTarget(resurrection_sickness_spell_id, this); // Rezz effects
		} else {
			SetHP(GetMaxHP() / 20);
			SetMana(GetMaxMana() / 20);
			SetEndurance(GetMaxEndurance() / 20);
		}
	}

	if (current_mana > max_mana) {
		current_mana = max_mana;
	}

	cur_end = max_end;

	SetExtraHaste(database.botdb.GetBotExtraHasteByID(GetBotID()), false);
}

Bot::~Bot() {
	AI_Stop();
	LeaveHealRotationMemberPool();
	DataBucket::DeleteCachedBuckets(DataBucketLoadType::Bot, GetBotID());

	if (HasPet()) {
		GetPet()->Depop();
	}

	entity_list.RemoveBot(GetID());

	if (GetGroup()) {
		GetGroup()->MemberZoned(this);
	}

	if (GetRaid()) {
		GetRaid()->MemberZoned(CastToClient());
	}
}

void Bot::SetBotID(uint32 botID) {
	_botID = botID;
	npctype_id = botID;
}

void Bot::SetBotSpellID(uint32 newSpellID) {
	npc_spells_id = newSpellID;
}

void Bot::SetSurname(std::string_view bot_surname) {

	_surname = bot_surname.substr(0, 31);

	if (spawned) {

		auto outapp = new EQApplicationPacket(OP_GMLastName, sizeof(GMLastName_Struct));
		auto gmn = (GMLastName_Struct*)outapp->pBuffer;

		strcpy(gmn->name, GetCleanName());
		strcpy(gmn->gmname, GetCleanName());
		strcpy(gmn->lastname, GetSurname().c_str());
		gmn->unknown[0] = 1;
		gmn->unknown[1] = 1;
		gmn->unknown[2] = 1;
		gmn->unknown[3] = 1;

		entity_list.QueueClients(this, outapp);
		safe_delete(outapp);
	}
}

void Bot::SetTitle(std::string_view bot_title) {

	_title = bot_title.substr(0, 31);

	if (spawned) {

		auto outapp = new EQApplicationPacket(OP_SetTitleReply, sizeof(SetTitleReply_Struct));
		auto strs = (SetTitleReply_Struct*)outapp->pBuffer;

		strs->is_suffix = 0;
		strn0cpy(strs->title, _title.c_str(), sizeof(strs->title));
		strs->entity_id = GetID();

		entity_list.QueueClients(this, outapp, false);
		safe_delete(outapp);
	}
}

void  Bot::SetSuffix(std::string_view bot_suffix) {

	_suffix = bot_suffix.substr(0, 31);

	if (spawned) {

		auto outapp = new EQApplicationPacket(OP_SetTitleReply, sizeof(SetTitleReply_Struct));
		auto strs = (SetTitleReply_Struct*)outapp->pBuffer;

		strs->is_suffix = 1;
		strn0cpy(strs->title, _suffix.c_str(), sizeof(strs->title));
		strs->entity_id = GetID();

		entity_list.QueueClients(this, outapp, false);
		safe_delete(outapp);
	}
}

uint32 Bot::GetBotRangedValue() {
	const EQ::ItemInstance *range_inst = GetBotItem(EQ::invslot::slotRange);
	const EQ::ItemInstance *ammo_inst = GetBotItem(EQ::invslot::slotAmmo);

	if (!range_inst) {
		return 0;
	}

	const EQ::ItemData *range_item = range_inst->GetItem();
	const EQ::ItemData *ammo_item = nullptr;

	if (ammo_inst) {
		ammo_item = ammo_inst->GetItem();
	}

	bool has_ammo = ammo_item;

	if (!range_item || !has_ammo) {
		return 0;
	}

	// Bow requires arrows
	if (
		range_item->ItemType == EQ::item::ItemTypeBow &&
		ammo_item->ItemType != EQ::item::ItemTypeArrow
	) {
		return 0;
	}

	// Throwing items
	if (
		range_item->ItemType == EQ::item::ItemTypeSmallThrowing ||
		range_item->ItemType == EQ::item::ItemTypeLargeThrowing
	) {
		if (range_item->ID == ammo_item->ID) {
			return range_item->Range;
		}

		return 0; // mismatched throwing
	}

	// Bows and arrows
	if (
		range_item->ItemType == EQ::item::ItemTypeBow &&
		ammo_item->ItemType == EQ::item::ItemTypeArrow
	) {
		return (range_item->Range + ammo_item->Range);
	}

	return 0;
}

void Bot::ChangeBotRangedWeapons(bool isRanged) {
	if (!isRanged) {
		BotAddEquipItem(EQ::invslot::slotPrimary, GetBotItemBySlot(EQ::invslot::slotPrimary));
		BotAddEquipItem(EQ::invslot::slotSecondary, GetBotItemBySlot(EQ::invslot::slotSecondary));
		SetAttackTimer();
		RaidGroupSay("My blade is ready");
	} else {
		BotRemoveEquipItem(EQ::invslot::slotPrimary);
		BotRemoveEquipItem(EQ::invslot::slotSecondary);
		BotAddEquipItem(EQ::invslot::slotAmmo, GetBotItemBySlot(EQ::invslot::slotAmmo));
		BotAddEquipItem(EQ::invslot::slotSecondary, GetBotItemBySlot(EQ::invslot::slotRange));
		SetAttackTimer();
		RaidGroupSay("My blades are sheathed");
	}
}

void Bot::Sit() {
	if (IsMoving()) {
		moved = false;
		StopNavigation();
	}

	SetAppearance(eaSitting);
}

void Bot::Stand() {
	SetAppearance(eaStanding);
}

bool Bot::IsSitting() const {
	bool result = false;
	if (GetAppearance() == eaSitting && !IsMoving())
		result = true;

	return result;
}

bool Bot::IsStanding() {
	bool result = false;
	if (GetAppearance() == eaStanding)
		result = true;

	return result;
}

NPCType *Bot::FillNPCTypeStruct(
	uint32 botSpellsID,
	const std::string& botName,
	const std::string& botLastName,
	uint8 botLevel,
	uint16 botRace,
	uint8 botClass,
	uint8 gender,
	float size,
	uint32 face,
	uint32 hairStyle,
	uint32 hairColor,
	uint32 eyeColor,
	uint32 eyeColor2,
	uint32 beard,
	uint32 beardColor,
	uint32 drakkinHeritage,
	uint32 drakkinTattoo,
	uint32 drakkinDetails,
	int32 hp,
	int32 mana,
	int32 mr,
	int32 cr,
	int32 dr,
	int32 fr,
	int32 pr,
	int32 corrup,
	int32 ac,
	uint32 str,
	uint32 sta,
	uint32 dex,
	uint32 agi,
	uint32 _int,
	uint32 wis,
	uint32 cha,
	uint32 attack
) {
	auto n = new NPCType{ 0 };

	strn0cpy(n->name, botName.c_str(), sizeof(n->name));
	strn0cpy(n->lastname, botLastName.c_str(), sizeof(n->lastname));

	n->current_hp = hp;
	n->max_hp = hp;
	n->size = size;
	n->runspeed = 1.25f;
	n->gender = gender;
	n->race = botRace;
	n->class_ = botClass;
	n->bodytype = 1;
	n->deity = Deity::Agnostic1;
	n->level = botLevel;
	n->npc_spells_id = botSpellsID;
	n->AC = ac;
	n->Mana = mana;
	n->ATK = attack;
	n->STR = str;
	n->STA = sta;
	n->DEX = dex;
	n->AGI = agi;
	n->INT = _int;
	n->WIS = wis;
	n->CHA = cha;
	n->MR = mr;
	n->FR = fr;
	n->CR = cr;
	n->PR = pr;
	n->DR = dr;
	n->Corrup = corrup;
	n->haircolor = hairColor;
	n->beardcolor = beardColor;
	n->eyecolor1 = eyeColor;
	n->eyecolor2 = eyeColor2;
	n->hairstyle = hairStyle;
	n->luclinface = face;
	n->beard = beard;
	n->drakkin_heritage = drakkinHeritage;
	n->drakkin_tattoo = drakkinTattoo;
	n->drakkin_details = drakkinDetails;
	n->hp_regen = 1;
	n->mana_regen = 1;
	n->trackable = true;
	n->maxlevel = botLevel;
	n->show_name = true;
	n->skip_global_loot = true;
	n->stuck_behavior = Ground;
	n->skip_auto_scale = true;

	return n;
}

NPCType *Bot::CreateDefaultNPCTypeStructForBot(
	const std::string& botName,
	const std::string& botLastName,
	uint8 botLevel,
	uint16 botRace,
	uint8 botClass,
	uint8 gender
) {
	auto n = new NPCType{ 0 };

	strn0cpy(n->name, botName.c_str(), sizeof(n->name));
	strn0cpy(n->lastname, botLastName.c_str(), sizeof(n->lastname));

	n->size = 6.0f;
	n->runspeed = 0.7f;
	n->gender = gender;
	n->race = botRace;
	n->class_ = botClass;
	n->bodytype = 1;
	n->deity = Deity::Agnostic1;
	n->level = botLevel;
	n->AC = 12;
	n->ATK = 75;
	n->STR = 75;
	n->STA = 75;
	n->DEX = 75;
	n->AGI = 75;
	n->INT = 75;
	n->WIS = 75;
	n->CHA = 75;
	n->MR = 25;
	n->FR = 25;
	n->CR = 25;
	n->PR = 15;
	n->DR = 15;
	n->Corrup = 15;
	n->hp_regen = 1;
	n->mana_regen = 1;
	n->trackable = true;
	n->maxlevel = botLevel;
	n->show_name = true;
	n->skip_global_loot = true;
	n->stuck_behavior = Ground;

	return n;
}

void Bot::GenerateBaseStats()
{
	int BotSpellID = 0;

	// base stats
	uint32 Strength = _baseSTR;
	uint32 Stamina = _baseSTA;
	uint32 Dexterity = _baseDEX;
	uint32 Agility = _baseAGI;
	uint32 Wisdom = _baseWIS;
	uint32 Intelligence = _baseINT;
	uint32 Charisma = _baseCHA;
	uint32 Attack = _baseATK;
	int32 MagicResist = _baseMR;
	int32 FireResist = _baseFR;
	int32 DiseaseResist = _baseDR;
	int32 PoisonResist = _basePR;
	int32 ColdResist = _baseCR;
	int32 CorruptionResist = _baseCorrup;

	// pulling fixed values from an auto-increment field is dangerous...
	switch (GetClass()) {
		case Class::Warrior:
			BotSpellID = 3001;
			Strength += 10;
			Stamina += 20;
			Agility += 10;
			Dexterity += 10;
			Attack += 12;
			break;
		case Class::Cleric:
			BotSpellID = 3002;
			Strength += 5;
			Stamina += 5;
			Agility += 10;
			Wisdom += 30;
			Attack += 8;
			break;
		case Class::Paladin:
			BotSpellID = 3003;
			Strength += 15;
			Stamina += 5;
			Wisdom += 15;
			Charisma += 10;
			Dexterity += 5;
			Attack += 17;
			break;
		case Class::Ranger:
			BotSpellID = 3004;
			Strength += 15;
			Stamina += 10;
			Agility += 10;
			Wisdom += 15;
			Attack += 17;
			break;
		case Class::ShadowKnight:
			BotSpellID = 3005;
			Strength += 10;
			Stamina += 15;
			Intelligence += 20;
			Charisma += 5;
			Attack += 17;
			break;
		case Class::Druid:
			BotSpellID = 3006;
			Stamina += 15;
			Wisdom += 35;
			Attack += 5;
			break;
		case Class::Monk:
			BotSpellID = 3007;
			Strength += 5;
			Stamina += 15;
			Agility += 15;
			Dexterity += 15;
			Attack += 17;
			break;
		case Class::Bard:
			BotSpellID = 3008;
			Strength += 15;
			Dexterity += 10;
			Charisma += 15;
			Intelligence += 10;
			Attack += 17;
			break;
		case Class::Rogue:
			BotSpellID = 3009;
			Strength += 10;
			Stamina += 20;
			Agility += 10;
			Dexterity += 10;
			Attack += 12;
			break;
		case Class::Shaman:
			BotSpellID = 3010;
			Stamina += 10;
			Wisdom += 30;
			Charisma += 10;
			Attack += 28;
			break;
		case Class::Necromancer:
			BotSpellID = 3011;
			Dexterity += 10;
			Agility += 10;
			Intelligence += 30;
			Attack += 5;
			break;
		case Class::Wizard:
			BotSpellID = 3012;
			Stamina += 20;
			Intelligence += 30;
			Attack += 5;
			break;
		case Class::Magician:
			BotSpellID = 3013;
			Stamina += 20;
			Intelligence += 30;
			Attack += 5;
			break;
		case Class::Enchanter:
			BotSpellID = 3014;
			Intelligence += 25;
			Charisma += 25;
			Attack += 5;
			break;
		case Class::Beastlord:
			BotSpellID = 3015;
			Stamina += 10;
			Agility += 10;
			Dexterity += 5;
			Wisdom += 20;
			Charisma += 5;
			Attack += 31;
			break;
		case Class::Berserker:
			BotSpellID = 3016;
			Strength += 10;
			Stamina += 15;
			Dexterity += 15;
			Agility += 10;
			Attack += 25;
			break;
		default:
			break;
	}

	float BotSize = GetSize();

	switch(GetRace()) {
		case Race::Human: // Humans have no race bonus
			break;
		case Race::Barbarian:
			Strength += 28;
			Stamina += 20;
			Agility += 7;
			Dexterity -= 5;
			Wisdom -= 5;
			Intelligence -= 10;
			Charisma -= 20;
			BotSize = 7.0;
			ColdResist += 10;
			break;
		case Race::Erudite:
			Strength -= 15;
			Stamina -= 5;
			Agility -= 5;
			Dexterity -= 5;
			Wisdom += 8;
			Intelligence += 32;
			Charisma -= 5;
			MagicResist += 5;
			DiseaseResist -= 5;
			break;
		case Race::WoodElf:
			Strength -= 10;
			Stamina -= 10;
			Agility += 20;
			Dexterity += 5;
			Wisdom += 5;
			BotSize = 5.0;
			break;
		case Race::HighElf:
			Strength -= 20;
			Stamina -= 10;
			Agility += 10;
			Dexterity -= 5;
			Wisdom += 20;
			Intelligence += 12;
			Charisma += 5;
			break;
		case Race::DarkElf:
			Strength -= 15;
			Stamina -= 10;
			Agility += 15;
			Wisdom += 8;
			Intelligence += 24;
			Charisma -= 15;
			BotSize = 5.0;
			break;
		case Race::HalfElf:
			Strength -= 5;
			Stamina -= 5;
			Agility += 15;
			Dexterity += 10;
			Wisdom -= 15;
			BotSize = 5.5;
			break;
		case Race::Dwarf:
			Strength += 15;
			Stamina += 15;
			Agility -= 5;
			Dexterity += 15;
			Wisdom += 8;
			Intelligence -= 15;
			Charisma -= 30;
			BotSize = 4.0;
			MagicResist -= 5;
			PoisonResist += 5;
			break;
		case Race::Troll:
			Strength += 33;
			Stamina += 34;
			Agility += 8;
			Wisdom -= 15;
			Intelligence -= 23;
			Charisma -= 35;
			BotSize = 8.0;
			FireResist -= 20;
			break;
		case Race::Ogre:
			Strength += 55;
			Stamina += 77;
			Agility -= 5;
			Dexterity -= 5;
			Wisdom -= 8;
			Intelligence -= 15;
			Charisma -= 38;
			BotSize = 9.0;
			break;
		case Race::Halfling:
			Strength -= 5;
			Agility += 20;
			Dexterity += 15;
			Wisdom += 5;
			Intelligence -= 8;
			Charisma -= 25;
			BotSize = 3.5;
			PoisonResist += 5;
			DiseaseResist += 5;
			break;
		case Race::Gnome:
			Strength -= 15;
			Stamina -= 5;
			Agility += 10;
			Dexterity += 10;
			Wisdom -= 8;
			Intelligence += 23;
			Charisma -= 15;
			BotSize = 3.0;
			break;
		case Race::Iksar:
			Strength -= 5;
			Stamina -= 5;
			Agility += 15;
			Dexterity += 10;
			Wisdom += 5;
			Charisma -= 20;
			MagicResist -= 5;
			FireResist -= 5;
			break;
		case Race::VahShir:
			Strength += 15;
			Agility += 15;
			Dexterity -= 5;
			Wisdom -= 5;
			Intelligence -= 10;
			Charisma -= 10;
			BotSize = 7.0;
			MagicResist -= 5;
			FireResist -= 5;
			break;
		case Race::Froglok2:
			Strength -= 5;
			Stamina += 5;
			Agility += 25;
			Dexterity += 25;
			Charisma -= 25;
			BotSize = 5.0;
			MagicResist -= 5;
			FireResist -= 5;
			break;
		case Race::Drakkin:
			Strength -= 5;
			Stamina += 5;
			Agility += 10;
			Intelligence += 10;
			Wisdom += 5;
			BotSize = 5.0;
			PoisonResist += 2;
			DiseaseResist += 2;
			MagicResist += 2;
			FireResist += 2;
			ColdResist += 2;
			break;
		default:
			break;
	}

	STR = Strength;
	STA = Stamina;
	DEX = Dexterity;
	AGI = Agility;
	WIS = Wisdom;
	INT = Intelligence;
	CHA = Charisma;
	ATK = Attack;
	MR = MagicResist;
	FR = FireResist;
	DR = DiseaseResist;
	PR = PoisonResist;
	CR = ColdResist;
	PhR = 0;
	Corrup = CorruptionResist;
	SetBotSpellID(BotSpellID);
	size = BotSize;
	pAggroRange = 0;
	pAssistRange = 0;
	raid_target = false;
	deity = 396;
}

void Bot::GenerateAppearance() {
	// Randomize facial appearance
	int iFace = 0;
	if (GetRace() == Race::Barbarian) // Barbarian w/Tatoo
	{
		iFace = zone->random.Int(0, 79);
	}
	else
	{
		iFace = zone->random.Int(0, 7);
	}

	int iHair = 0;
	int iBeard = 0;
	int iBeardColor = 1;
	if (GetRace() == Race::Drakkin) {
		iHair = zone->random.Int(0, 8);
		iBeard = zone->random.Int(0, 11);
		iBeardColor = zone->random.Int(0, 3);
	} else if (GetGender()) {
		iHair = zone->random.Int(0, 2);
		if (GetRace() == Race::Dwarf && zone->random.Int(1, 100) < 50) {
			iFace += 10;
		}
	} else {
		iHair = zone->random.Int(0, 3);
		iBeard = zone->random.Int(0, 5);
		iBeardColor = zone->random.Int(0, 19);
	}

	int iHairColor = 0;
	if (GetRace() == Race::Drakkin) {
		iHairColor = zone->random.Int(0, 3);
	} else {
		iHairColor = zone->random.Int(0, 19);
	}

	auto iEyeColor1 = (uint8)zone->random.Int(0, 9);
	uint8 iEyeColor2 = 0;
	if (GetRace() == Race::Drakkin) {
		iEyeColor1 = iEyeColor2 = (uint8)zone->random.Int(0, 11);
	} else if (zone->random.Int(1, 100) > 96) {
		iEyeColor2 = zone->random.Int(0, 9);
	} else {
		iEyeColor2 = iEyeColor1;
	}

	int iHeritage = 0;
	int iTattoo = 0;
	int iDetails = 0;
	if (GetRace() == Race::Drakkin) {
		iHeritage = zone->random.Int(0, 6);
		iTattoo = zone->random.Int(0, 7);
		iDetails = zone->random.Int(0, 7);
	}
	luclinface = iFace;
	hairstyle = iHair;
	beard = iBeard;
	beardcolor = iBeardColor;
	haircolor = iHairColor;
	eyecolor1 = iEyeColor1;
	eyecolor2 = iEyeColor2;
	drakkin_heritage = iHeritage;
	drakkin_tattoo = iTattoo;
	drakkin_details = iDetails;
}

uint16 Bot::GetPrimarySkillValue() {
	EQ::skills::SkillType skill = EQ::skills::HIGHEST_SKILL; //because nullptr == 0, which is 1H Slashing, & we want it to return 0 from GetSkill
	if (bool equipped = m_inv.GetItem(EQ::invslot::slotPrimary); !equipped) {
		skill = EQ::skills::SkillHandtoHand;
	} else {
		uint8 type = m_inv.GetItem(EQ::invslot::slotPrimary)->GetItem()->ItemType; //is this the best way to do this?
		switch (type) {
			case EQ::item::ItemType1HSlash:
				skill = EQ::skills::Skill1HSlashing;
				break;
			case EQ::item::ItemType2HSlash:
				skill = EQ::skills::Skill2HSlashing;
				break;
			case EQ::item::ItemType1HPiercing:
				skill = EQ::skills::Skill1HPiercing;
				break;
			case EQ::item::ItemType1HBlunt:
				skill = EQ::skills::Skill1HBlunt;
				break;
			case EQ::item::ItemType2HBlunt:
				skill = EQ::skills::Skill2HBlunt;
				break;
			case EQ::item::ItemType2HPiercing:
				skill = EQ::skills::Skill2HPiercing;
				break;
			case EQ::item::ItemTypeMartial:
				skill = EQ::skills::SkillHandtoHand;
				break;
			default:
				skill = EQ::skills::SkillHandtoHand;
				break;
		}
	}

	return GetSkill(skill);
}

uint16 Bot::MaxSkill(EQ::skills::SkillType skillid, uint16 class_, uint16 level) const {
	return SkillCaps::Instance()->GetSkillCap(class_, skillid, level).cap;
}

uint32 Bot::GetTotalATK() {
	uint32 AttackRating = 0;
	uint32 WornCap = itembonuses.ATK;
	if (IsBot()) {
		AttackRating = ((WornCap * 1.342) + (GetSkill(EQ::skills::SkillOffense) * 1.345) + ((GetSTR() - 66) * 0.9) + (GetPrimarySkillValue() * 2.69));
		AttackRating += aabonuses.ATK + GroupLeadershipAAOffenseEnhancement();
		if (AttackRating < 10)
			AttackRating = 10;
	}
	else
		AttackRating = GetATK();

	AttackRating += spellbonuses.ATK;
	return AttackRating;
}

uint32 Bot::GetATKRating() {
	uint32 AttackRating = 0;
	if (IsBot()) {
		AttackRating = (GetSkill(EQ::skills::SkillOffense) * 1.345) + ((GetSTR() - 66) * 0.9) + (GetPrimarySkillValue() * 2.69);
		if (AttackRating < 10)
			AttackRating = 10;
	}
	return AttackRating;
}

int32 Bot::GenerateBaseHitPoints() {
	// Calc Base Hit Points
	int new_base_hp = 0;
	uint32 lm = GetClassLevelFactor();
	int32 Post255;
	int32 NormalSTA = GetSTA();
	if (GetOwner() && GetOwner()->CastToClient() && GetOwner()->CastToClient()->ClientVersion() >= EQ::versions::ClientVersion::SoD && RuleB(Character, SoDClientUseSoDHPManaEnd)) {
		float SoDPost255;
		if (((NormalSTA - 255) / 2) > 0)
			SoDPost255 = ((static_cast<float>(NormalSTA) - 255.0f) / 2.0f);
		else
			SoDPost255 = 0.0f;

		int hp_factor = GetClassHPFactor();

		if (level < 41)
			new_base_hp = (5 + (GetLevel() * hp_factor / 12) + ((NormalSTA - SoDPost255) * GetLevel() * hp_factor / 3600));
		else if (level < 81)
			new_base_hp = (5 + (40 * hp_factor / 12) + ((GetLevel() - 40) * hp_factor / 6) + ((NormalSTA - SoDPost255) * hp_factor / 90) + ((NormalSTA - SoDPost255) * (GetLevel() - 40) * hp_factor / 1800));
		else
			new_base_hp = (5 + (80 * hp_factor / 8) + ((GetLevel() - 80) * hp_factor / 10) + ((NormalSTA - SoDPost255) * hp_factor / 90) + ((NormalSTA - SoDPost255) * hp_factor / 45));
	} else {
		if (((NormalSTA - 255) / 2) > 0)
			Post255 = ((NormalSTA - 255) / 2);
		else
			Post255 = 0;

		new_base_hp = 5 + (GetLevel() * lm / 10) + ((NormalSTA - Post255) * GetLevel() * lm / 3000) + ((Post255 * 1) * lm / 6000);
	}
	base_hp = new_base_hp;
	return new_base_hp;
}

void Bot::LoadAAs() {
	aa_ranks.clear();

	int id = 0;
	int points = 0;
	auto iter = zone->aa_abilities.begin();
	while(iter != zone->aa_abilities.end()) {
		AA::Ability *ability = (*iter).second.get();

		//skip expendables
		if (!ability->first || ability->charges > 0) {
			++iter;
			continue;
		}

		id = ability->first->id;
		points = 0;

		AA::Rank *current = ability->first;

		if (current->level_req > GetLevel()) {
			++iter;
			continue;
		}

		while(current) {
			if (current->level_req > GetLevel() || !CanUseAlternateAdvancementRank(current)) {
				current = nullptr;
			} else {
				current = current->next;
				points++;
			}
		}

		if (points > 0) {
			SetAA(id, points);
		}

		++iter;
	}
}

bool Bot::IsValidRaceClassCombo()
{
	return Bot::IsValidRaceClassCombo(GetRace(), GetClass());
}

bool Bot::IsValidRaceClassCombo(uint16 bot_race, uint8 bot_class)
{
	bool is_valid = false;
	auto classes = database.botdb.GetRaceClassBitmask(bot_race);
	if (auto bot_class_bitmask = GetPlayerClassBit(bot_class); classes & bot_class_bitmask) {
		is_valid = true;
	}
	return is_valid;
}

bool Bot::IsValidName()
{
	std::string name = GetCleanName();
	return Bot::IsValidName(name);
}

bool Bot::IsValidName(std::string& name)
{
	if (name.empty()) { // can't be empty
		return false;
	}

	if (islower(name[0])) { // capitalize first letter if not
		name[0] = toupper(name[0]);
	}

	if (!EQ::ValueWithin(name.size(), 4, 15)) { // must be between 4 and 15 characters
		return false;
	}

	if (std::any_of(name.begin(), name.end(), [](char c) { return c == ' ' || c == '_'; })) { // cannot contain spaces or _
		return false;
	}

	if (!RuleB(Bots, AllowCamelCaseNames)) {
		for (int i = 1; i < name.size(); ++i) {
			if (isupper(name[i])) {
				return false;
			}
		}
	}

	return true;
}

bool Bot::Save()
{
	auto bot_owner = GetBotOwner();
	if (!bot_owner)
		return false;

	if (!GetBotID()) { // New bot record
		uint32 bot_id = 0;
		if (!database.botdb.SaveNewBot(this, bot_id) || !bot_id) {
			return false;
		}
		SetBotID(bot_id);
	}
	else { // Update existing bot record
		if (!database.botdb.SaveBot(this)) {
			return false;
		}
	}

	// All of these continue to process if any fail
	database.botdb.SaveBuffs(this);
	database.botdb.SaveTimers(this);
	database.botdb.SaveStance(this);
	database.botdb.SaveBotSettings(this);

	if (RuleB(Bots, AllowBotBlockedBuffs)) {
		database.botdb.SaveBotBlockedBuffs(this);
	}

	if (!SavePet())
		bot_owner->Message(Chat::White, "Failed to save pet for '%s'", GetCleanName());

	return true;
}

bool Bot::DeleteBot()
{
	auto bot_owner = GetBotOwner();
	if (!bot_owner) {
		return false;
	}

	if (!database.botdb.DeleteHealRotation(GetBotID())) {
		return false;
	}

	std::string query = StringFormat("DELETE FROM `bot_heal_rotation_members` WHERE `bot_id` = '%u'", GetBotID());
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		bot_owner->Message(Chat::White, "Failed to delete heal rotation member '%s'", GetCleanName());
		return false;
	}

	query = StringFormat("DELETE FROM `bot_heal_rotation_targets` WHERE `target_name` LIKE '%s'", GetCleanName());
	results = database.QueryDatabase(query);
	if (!results.Success()) {
		bot_owner->Message(Chat::White, "Failed to delete heal rotation target '%s'", GetCleanName());
		return false;
	}

	if (!DeletePet()) {
		bot_owner->Message(Chat::White, "Failed to delete pet for '%s'", GetCleanName());
		return false;
	}

	if (GetGroup()) {
		RemoveBotFromGroup(this, GetGroup());
	}

	if (GetRaid()) {
		RemoveBotFromRaid(this);
	}

	if (!RuleB(Bots, BotSoftDeletes)) {
		if (!database.botdb.DeleteItems(GetBotID())) {
			return false;
		}

		if (!database.botdb.DeleteTimers(GetBotID())) {
			return false;
		}

		if (!database.botdb.DeleteBuffs(GetBotID())) {
			return false;
		}

		if (!database.botdb.DeleteStance(GetBotID())) {
			return false;
		}

		if (!database.botdb.DeleteBotSettings(GetBotID())) {
			return false;
		}

		if (!database.botdb.DeleteBotBlockedBuffs(GetBotID())) {
			return false;
		}
	}

	if (!database.botdb.DeleteBot(GetBotID())) {
		return false;
	}

	return true;
}

// Returns the current total play time for the bot
uint32 Bot::GetTotalPlayTime() {
	uint32 Result = 0;
	double TempTotalPlayTime = 0;
	time_t currentTime = time(&currentTime);
	TempTotalPlayTime = difftime(currentTime, _startTotalPlayTime);
	TempTotalPlayTime += _lastTotalPlayTime;
	Result = (uint32)TempTotalPlayTime;
	return Result;
}

bool Bot::LoadPet()
{
	if (GetPet())
		return true;

	auto bot_owner = GetBotOwner();
	if (!bot_owner)
		return false;

	if (GetClass() == Class::Wizard) {
		auto buffs_max = GetMaxBuffSlots();
		auto my_buffs = GetBuffs();
		if (buffs_max && my_buffs) {
			for (int index = 0; index < buffs_max; ++index) {
				if (IsEffectInSpell(my_buffs[index].spellid, SpellEffect::Familiar)) {
					MakePet(my_buffs[index].spellid, spells[my_buffs[index].spellid].teleport_zone);
					return true;
				}
			}
		}
	}

	uint32 pet_index = 0;
	if (!database.botdb.LoadPetIndex(GetBotID(), pet_index)) {
		return false;
	}
	if (!pet_index)
		return true;

	uint32 saved_pet_spell_id = 0;
	database.botdb.LoadPetSpellID(GetBotID(), saved_pet_spell_id);

	if (!IsValidSpell(saved_pet_spell_id)) {
		bot_owner->Message(Chat::White, "Invalid spell id for %s's pet", GetCleanName());
		DeletePet();
		return false;
	}

	std::string pet_name;
	uint32 pet_mana = 0;
	uint32 pet_hp = 0;
	uint32 pet_spell_id = 0;

	if (!database.botdb.LoadPetStats(GetBotID(), pet_name, pet_mana, pet_hp, pet_spell_id)) {
		return false;
	}

	MakePet(pet_spell_id, spells[pet_spell_id].teleport_zone, pet_name.c_str());
	if (!GetPet()->IsNPC()) {
		DeletePet();
		return false;
	}

	NPC *pet_inst = GetPet()->CastToNPC();

	SpellBuff_Struct pet_buffs[PET_BUFF_COUNT];
	memset(pet_buffs, 0, (sizeof(SpellBuff_Struct) * PET_BUFF_COUNT));
	database.botdb.LoadPetBuffs(GetBotID(), pet_buffs);

	uint32 pet_items[EQ::invslot::EQUIPMENT_COUNT];
	memset(pet_items, 0, (sizeof(uint32) * EQ::invslot::EQUIPMENT_COUNT));
	database.botdb.LoadPetItems(GetBotID(), pet_items);

	pet_inst->SetPetState(pet_buffs, pet_items);
	pet_inst->CalcBonuses();
	pet_inst->SetHP(pet_hp);
	pet_inst->SetMana(pet_mana);

	return true;
}

bool Bot::SavePet()
{
	if (!GetPet() || GetPet()->IsFamiliar()) { // dead?
		return true;
	}

	NPC *pet_inst = GetPet()->CastToNPC();
	if (!pet_inst->GetPetSpellID() || !IsValidSpell(pet_inst->GetPetSpellID())) {
		return false;
	}

	auto bot_owner = GetBotOwner();
	if (!bot_owner) {
		return false;
	}

	auto pet_name = new char[64];
	SpellBuff_Struct pet_buffs[PET_BUFF_COUNT];
	uint32 pet_items[EQ::invslot::EQUIPMENT_COUNT];

	memset(pet_name, 0, 64);
	memset(pet_buffs, 0, (sizeof(SpellBuff_Struct) * PET_BUFF_COUNT));
	memset(pet_items, 0, (sizeof(uint32) * EQ::invslot::EQUIPMENT_COUNT));

	pet_inst->GetPetState(pet_buffs, pet_items, pet_name);

	std::string pet_name_str = pet_name;
	safe_delete_array(pet_name)

	if (!database.botdb.SavePetStats(GetBotID(), pet_name_str, pet_inst->GetMana(), pet_inst->GetHP(), pet_inst->GetPetSpellID())) {
		return false;
	}

	database.botdb.SavePetBuffs(GetBotID(), pet_buffs);
	database.botdb.SavePetItems(GetBotID(), pet_items);

	return true;
}

bool Bot::DeletePet()
{
	auto bot_owner = GetBotOwner();
	if (!bot_owner)
		return false;

	if (!database.botdb.DeletePetItems(GetBotID())) {
		return false;
	}
	if (!database.botdb.DeletePetBuffs(GetBotID())) {
		return false;
	}
	if (!database.botdb.DeletePetStats(GetBotID())) {
		return false;
	}

	if (!GetPet() || !GetPet()->IsNPC())
		return true;

	NPC* pet_inst = GetPet()->CastToNPC();
	pet_inst->SetOwnerID(0);

	SetPet(nullptr);

	return true;
}

bool Bot::Process()
{
	if (IsStunned() && stunned_timer.Check()) {
		Mob::UnStun();
		spun_timer.Disable();
	}

	if (!GetBotOwner()) {
		return false;
	}

	if (GetDepop()) {
		_botOwner = nullptr;
		_botOwnerCharacterID = 0;

		return false;
	}

	if (m_scan_close_mobs_timer.Check()) {
		entity_list.ScanCloseMobs(this);
	}

	SpellProcess();

	if (tic_timer.Check()) {

		// 6 seconds, or whatever the rule is set to has passed, send this position to everyone to avoid ghosting
		if (!IsEngaged() && !rest_timer.Enabled()) {
			rest_timer.Start(RuleI(Character, RestRegenTimeToActivate) * 1000);
		}

		BuffProcess();
		CalcRestState();

		if (currently_fleeing || IsFeared()) {
			ProcessFlee();
		}

		if (GetHP() < GetMaxHP()) {
			SetHP(GetHP() + CalcHPRegen() + RestRegenHP);
		}

		if (GetMana() < GetMaxMana()) {
			SetMana(GetMana() + CalcManaRegen() + RestRegenMana);
		}

		CalcATK();

		if (GetEndurance() < GetMaxEndurance()) {
			SetEndurance(GetEndurance() + CalcEnduranceRegen() + RestRegenEndurance);
		}
	}

	if (send_hp_update_timer.Check(false)) {

		SendHPUpdate();
		if (HasPet()) {
			GetPet()->SendHPUpdate();
		}

		// hack fix until percentage changes can be implemented
		auto g = GetGroup();
		if (g) {

			g->SendManaPacketFrom(this);
			g->SendEndurancePacketFrom(this);
		}
	}

	if (viral_timer.Check()) {
		VirusEffectProcess();
	}

	if (spellbonuses.GravityEffect == 1) {
		if (gravity_timer.Check()) {
			DoGravityEffect();
		}
	}

	if (GetAppearance() == eaDead && GetHP() > 0) {
		SetAppearance(eaStanding);
	}

	if (IsMoving()) {
		m_ping_timer.Disable();
	}
	else {
		if (!m_ping_timer.Enabled()) {
			m_ping_timer.Start(BOT_KEEP_ALIVE_INTERVAL);
		}

		if (m_ping_timer.Check()) {
			SentPositionPacket(0.0f, 0.0f, 0.0f, 0.0f, 0);
		}
	}

	if (m_auto_save_timer.Check()) {
		clock_t t = std::clock(); /* Function timer start */
		Save();
		LogDebug(
			"ZoneDatabase::SaveBotData [{}], done Took [{}] seconds",
			GetBotID(),
			((float)(std::clock() - t)) / CLOCKS_PER_SEC
		);
		m_auto_save_timer.Start(RuleI(Bots, AutosaveIntervalSeconds) * 1000);
	}

	if (ForcedMovement) {
		ProcessForcedMovement();
	}

	if (IsMezzed()) {
		return true;
	}

	if (IsStunned()) {
		if (spun_timer.Check()) {
			Spin();
		}

		return true;
	}

	AI_Process();

	return true;
}

void Bot::AI_Bot_Start(uint32 iMoveDelay) {
	Mob::AI_Start(iMoveDelay);
	if (!pAIControlled) {
		return;
	}

	if (AIBot_spells.empty()) {
		AIautocastspell_timer = std::make_unique<Timer>(1000);
		AIautocastspell_timer->Disable();
	} else {
		AIautocastspell_timer = std::make_unique<Timer>(500);
		AIautocastspell_timer->Start(RandomTimer(0, 300), false);
	}

	if (NPCTypedata) {
		ProcessSpecialAbilities(NPCTypedata->special_abilities);
		AI_AddNPCSpellsEffects(NPCTypedata->npc_spells_effects_id);
	}

	SendTo(GetX(), GetY(), GetZ());
	SaveGuardSpot(GetPosition());
}

void Bot::AI_Bot_Init()
{
	AIautocastspell_timer.reset(nullptr);
	casting_spell_AIindex = static_cast<uint8>(AIBot_spells.size());

	m_roambox.max_x		 = 0;
	m_roambox.max_y		 = 0;
	m_roambox.min_x		 = 0;
	m_roambox.min_y		 = 0;
	m_roambox.distance  = 0;
	m_roambox.dest_x		= 0;
	m_roambox.dest_y		= 0;
	m_roambox.dest_z		= 0;
	m_roambox.delay		 = 2500;
	m_roambox.min_delay = 2500;
}

void Bot::SpellProcess() {
	if (spellend_timer.Check(false)) {
		NPC::SpellProcess();
		if (GetClass() == Class::Bard && casting_spell_id != 0) casting_spell_id = 0;
	}
}

void Bot::BotMeditate(bool is_sitting) {
	bool needs_to_med =
		(!IsEngaged() && (GetManaRatio() < 100 || GetHPRatio() < 100)) ||
		(GetMedInCombat() && !HasTargetReflection() &&
			(
				GetManaRatio() < GetSitManaPct() ||
				(GetHPRatio() < GetSitHPPct() && GetLevel() >= GetStopMeleeLevel())
			)
		);

	if (needs_to_med && !is_sitting) {
		Sit();
	}
	else if (!needs_to_med && is_sitting) {
		Stand();
	}
}

bool Bot::BotRangedAttack(Mob* other, bool can_double_attack) {
	if (
		!other ||
		!IsAttackAllowed(other) ||
		IsCasting() ||
		DivineAura() ||
		IsStunned() ||
		IsMezzed() ||
		(GetAppearance() == eaDead)
	) {
		return false;
	}

	if (
		!GetPullingFlag() &&
		!GetReturningFlag() &&
		(
			(
				GetBotStance() != Stance::Aggressive &&
				GetBotStance() != Stance::Burn &&
				GetBotStance() != Stance::AEBurn
			) &&
			other->GetHPRatio() > 99.0f
		)
	) {
		return false;
	}

	if (
		!can_double_attack &&
		(
			(
				attack_timer.Enabled() &&
				!attack_timer.Check(false)
			) ||
			(
				ranged_timer.Enabled() &&
				!ranged_timer.Check()
			)
		)
	) {
		LogCombatDetail("Bot ranged attack canceled. Timer not up. Attack [{}] ranged [{}]", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		return false;
	}

	const auto ranged_item = GetBotItem(EQ::invslot::slotRange);
	const EQ::ItemData* ranged_weapon = nullptr;

	if (ranged_item) {
		ranged_weapon = ranged_item->GetItem();
	}

	if (!ranged_weapon) {
		return false;
	}

	const auto ammo_item = GetBotItem(EQ::invslot::slotAmmo);
	const EQ::ItemData* ammo = nullptr;

	if (ammo_item) {
		ammo = ammo_item->GetItem();
	}

	// Bow requires arrows
	// Check if ammo is invalid
	bool is_invalid_ammo = !ammo;

	// Check if ranged weapon type is invalid
	bool is_invalid_ranged_weapon_type = ranged_weapon &&
		(ranged_weapon->ItemType != EQ::item::ItemTypeBow &&
			ranged_weapon->ItemType != EQ::item::ItemTypeSmallThrowing &&
			ranged_weapon->ItemType != EQ::item::ItemTypeLargeThrowing);

	  // Check if bow has the wrong ammo
	bool is_bow_with_invalid_ammo = ranged_weapon &&
		(ranged_weapon->ItemType == EQ::item::ItemTypeBow &&
			(!ammo || ammo->ItemType != EQ::item::ItemTypeArrow));

	// Check if throwing weapon has insufficient charges
	bool is_throwing_weapon_with_insufficient_ammo = ranged_weapon &&
		(ranged_weapon->ItemType == EQ::item::ItemTypeSmallThrowing || ranged_weapon->ItemType == EQ::item::ItemTypeLargeThrowing) &&
		(
			!ammo_item || ammo_item->GetCharges() < 1 || // Not enough ammo
			(
				(RuleI(Bots, StackSizeMin) != -1 && ranged_item->GetCharges() != ranged_weapon->StackSize) || // Invalid stack size
				ranged_item->GetCharges() < RuleI(Bots, StackSizeMin) // Charges below minimum
			)
		);

	// Final ranged weapon validity check
	bool is_invalid_ranged_weapon = is_invalid_ranged_weapon_type || is_bow_with_invalid_ammo || is_throwing_weapon_with_insufficient_ammo;

	// Final condition
	if (is_invalid_ammo || is_invalid_ranged_weapon) {
		if (!ammo || ammo_item->GetCharges() < 1) {
			if (!GetCombatRoundForAlerts()) {
				SetCombatRoundForAlerts();
				RaidGroupSay("I do not have any ammo!");
			}
		}
		return false;
	}

	LogCombatDetail("Ranged attacking [{}] with {} [{}] ([{}]){}{}{}",
		other->GetCleanName(),
		(ranged_weapon->ItemType == EQ::item::ItemTypeBow ? "bow" : "throwing"),
		ranged_weapon->Name,
		ranged_weapon->ID,
		(ammo && ammo->ItemType == EQ::item::ItemTypeArrow ? " and arrow " : ""),
		(ammo && ammo->ItemType == EQ::item::ItemTypeArrow ? ammo->Name : ""),
		(ammo && ammo->ItemType == EQ::item::ItemTypeArrow ? std::to_string(ammo->ID) : "")
	);

	SendItemAnimation(other, ammo, (ranged_weapon->ItemType == EQ::item::ItemTypeBow ? EQ::skills::SkillArchery : EQ::skills::SkillThrowing));
	if (ranged_weapon->ItemType == EQ::item::ItemTypeBow) {
		DoArcheryAttackDmg(other, ranged_item, ammo_item);

		int chance_avoid_consume = aabonuses.ConsumeProjectile + itembonuses.ConsumeProjectile + spellbonuses.ConsumeProjectile;
		bool consumes_ammo = RuleB(Bots, BotArcheryConsumesAmmo);
		bool is_expendable_arrow = ranged_weapon->ExpendableArrow;
		bool no_chance_to_avoid = chance_avoid_consume == 0;
		bool failed_avoid_check = chance_avoid_consume < 100 && zone->random.Int(0, 99) > chance_avoid_consume;
		bool should_consume_ammo = consumes_ammo && (is_expendable_arrow || no_chance_to_avoid || failed_avoid_check);

		if (should_consume_ammo) {
			ammo_item->SetCharges(ammo_item->GetCharges() - 1);
			LogCombatDetail("Consumed Archery Ammo from slot {}.", EQ::invslot::slotAmmo);

			if (ammo_item->GetCharges() < 1) {
				RemoveBotItemBySlot(EQ::invslot::slotAmmo);
				BotRemoveEquipItem(EQ::invslot::slotAmmo);
			}
		}
		else if (!consumes_ammo) {
			LogCombatDetail("Archery Ammo Consumption is disabled.");
		}
		else {
			LogCombatDetail("Endless Quiver prevented Ammo Consumption.");
		}
	}
	else {
		DoThrowingAttackDmg(other, ranged_item); // watch
		// Consume Ammo, unless Ammo Consumption is disabled
		if (RuleB(Bots, BotThrowingConsumesAmmo)) {
			ammo_item->SetCharges((ammo_item->GetCharges() - 1));
			LogCombatDetail("Consumed Throwing Ammo from slot {}.", EQ::invslot::slotAmmo);

			if (ammo_item->GetCharges() < 1) {
				RemoveBotItemBySlot(EQ::invslot::slotAmmo);
				BotRemoveEquipItem(EQ::invslot::slotAmmo);
			}
		}
		else {
			LogCombatDetail("Throwing Ammo Consumption is disabled.");
		}
	}

	CommonBreakInvisibleFromCombat();

	return true;
}

bool Bot::CheckBotDoubleAttack(bool triple_attack) {
	//Check for bonuses that give you a double attack chance regardless of skill (ie Bestial Frenzy/Harmonious Attack AA)
	uint32 bonus_give_double_attack = aabonuses.GiveDoubleAttack + spellbonuses.GiveDoubleAttack + itembonuses.GiveDoubleAttack;

	if (!GetSkill(EQ::skills::SkillDoubleAttack) && !bonus_give_double_attack) {
		return false;
	}

	float chance = 0.0f;
	uint16 skill = GetSkill(EQ::skills::SkillDoubleAttack);

	int32 bonus_double_attack = 0;
	if ((GetClass() == Class::Paladin || GetClass() == Class::ShadowKnight) && (!HasTwoHanderEquipped())) {
		LogCombatDetail("Knight class without a 2 hand weapon equipped = No DA Bonus!");
	}
	else {
		bonus_double_attack = aabonuses.DoubleAttackChance + spellbonuses.DoubleAttackChance + itembonuses.DoubleAttackChance;
	}

	//Use skill calculations otherwise, if you only have AA applied GiveDoubleAttack chance then use that value as the base.
	if (skill) {
		chance = (float(skill + GetLevel()) * (float(100.0f + bonus_double_attack + bonus_give_double_attack) / 100.0f)) / 500.0f;
	}
	else {
		chance = (float(bonus_give_double_attack + bonus_double_attack) / 100.0f);
	}

	LogCombatDetail(
		"skill [{}] bonus_give_double_attack [{}] bonus_double_attack [{}] chance [{}]",
		skill,
		bonus_give_double_attack,
		bonus_double_attack,
		chance
	);

	return zone->random.Roll(chance);
}

bool Bot::CheckTripleAttack()
{
	int chance;

	if (RuleB(Combat, ClassicTripleAttack)) {
		if (
			GetLevel() >= 60 &&
			(
				GetClass() == Class::Warrior ||
				GetClass() == Class::Ranger ||
				GetClass() == Class::Monk ||
				GetClass() == Class::Berserker
			)
		) {
			switch (GetClass()) {
				case Class::Warrior:
					chance = RuleI(Combat, ClassicTripleAttackChanceWarrior);
					break;
				case Class::Ranger:
					chance = RuleI(Combat, ClassicTripleAttackChanceRanger);
					break;
				case Class::Monk:
					chance = RuleI(Combat, ClassicTripleAttackChanceMonk);
					break;
				case Class::Berserker:
					chance = RuleI(Combat, ClassicTripleAttackChanceBerserker);
					break;
				default:
					break;
			}
		}
	}
	else {
		chance = GetSkill(EQ::skills::SkillTripleAttack);
	}

	if (chance < 1) {
		return false;
	}

	int inc = aabonuses.TripleAttackChance + spellbonuses.TripleAttackChance + itembonuses.TripleAttackChance;
	chance = static_cast<int>(chance * (1 + inc / 100.0f));
	chance = (chance * 100) / (chance + 800);

	return zone->random.Int(1, 100) <= chance;
}

void Bot::SetTarget(Mob *mob)
{
	if (mob != this) {
		NPC::SetTarget(mob);
	}
}

void Bot::SetGuardMode() {

	StopMoving();
	m_GuardPoint = GetPosition();
	SetGuardFlag();
}

void Bot::SetHoldMode() {

	SetHoldFlag();
}

// AI Processing for the Bot object

void Bot::AI_Process()
{

#define PULLING_BOT (GetPullingFlag())
#define NOT_PULLING_BOT (!GetPullingFlag())
#define RETURNING_BOT (GetReturningFlag())
#define NOT_RETURNING_BOT (!GetReturningFlag())
#define GUARDING (GetGuardFlag())
#define NOT_GUARDING (!GetGuardFlag())
#define HOLDING (GetHoldFlag())
#define NOT_HOLDING (!GetHoldFlag())
#define PASSIVE (GetBotStance() == Stance::Passive)
#define NOT_PASSIVE (GetBotStance() != Stance::Passive)

	Client* bot_owner = (GetBotOwner() && GetBotOwner()->IsClient() ? GetBotOwner()->CastToClient() : nullptr);

	if (!bot_owner) {
		return;
	}

	Raid* raid = entity_list.GetRaidByBot(this);
	SetStoredRaid(raid);
	uint32 r_group = RAID_GROUPLESS;

	if (raid) {
		if (!GetVerifiedRaid()) {
			raid->VerifyRaid();
			SetVerifiedRaid(true);
		}

		if (mana_timer.Check() || send_hp_update_timer.Check()) {
			raid->SendHPManaEndPacketsFrom(this);
		}

		r_group = raid->GetGroup(GetName());
	}

	auto bot_group = GetGroup();

	// Primary reasons for not processing AI
	if (!IsAIProcessValid(bot_owner, bot_group, raid)) {
		return;
	}

	Client* leash_owner = bot_owner;

	if (!leash_owner) {
		return;
	}

	Mob* follow_mob = nullptr;

	if (!GetFollowID()) {
		follow_mob = leash_owner;
	}
	else {
		follow_mob = entity_list.GetMob(GetFollowID());

		if (!follow_mob || !IsInGroupOrRaid(follow_mob)) {
			follow_mob = leash_owner;
		}
	}

	SetFollowID(follow_mob->GetID());

	SetBerserkState();

	// Secondary reasons for not processing AI
	if (CheckIfIncapacitated()) {
		return;
	}

	if (HOLDING || (raid && r_group == RAID_GROUPLESS)) {
		TryNonCombatMovementChecks(bot_owner, follow_mob);

		return;
	}

	float fm_distance = DistanceSquared(m_Position, follow_mob->GetPosition());
	float lo_distance = DistanceSquared(m_Position, leash_owner->GetPosition());
	float leash_distance = RuleR(Bots, LeashDistance);

// CURRENTLY CASTING CHECKS

	if (CheckIfCasting(fm_distance)) {
		return;
	}

	_spell_target_list.clear();
	_group_spell_target_list.clear();
	SetTempSpellType(UINT16_MAX);

// HEAL ROTATION CASTING CHECKS
	HealRotationChecks();

	if (GetAttackFlag()) { // Push owner's target onto our hate list
		SetOwnerTarget(bot_owner);
	}
	else if (GetPullFlag()) { // Push owner's target onto our hate list and set flags so other bots do not aggro
		BotPullerProcess(bot_owner, raid);
	}

//ALT COMBAT (ACQUIRE HATE)
	// We have aggro to choose from
	if (IsEngaged()) {
		if (rest_timer.Enabled()) {
			rest_timer.Disable();
		}

// PULLING FLAG (TARGET VALIDATION)

		if (PULLING_BOT) {
			if (!PullingFlagChecks(bot_owner)) {
				return;
			}
		}

// RETURNING FLAG

		if (RETURNING_BOT) {
			if (ReturningFlagChecks(bot_owner, leash_owner, fm_distance)) {
				return;
			}
		}

// DEFAULT (ACQUIRE TARGET)

// VERIFY TARGET AND STANCE
		auto tar = GetBotTarget(bot_owner);

		if (!tar) {
			return;
		}

// ATTACKING FLAG (HATE VALIDATION)

		if (GetAttackingFlag() && tar->CheckAggro(this)) {
			SetAttackingFlag(false);
		}

		float tar_distance = DistanceSquaredNoZ(m_Position, tar->GetPosition());

// TARGET VALIDATION
		if (!IsValidTarget(bot_owner, leash_owner, lo_distance, leash_distance, tar, tar_distance)) {
			if (HasPet()) {
				if (tar && GetPet()->GetTarget() && GetPet()->GetTarget() == tar) {
					GetPet()->WipeHateList();
					GetPet()->SetTarget(nullptr);
				}
			}

			return;
		}

		// This causes conflicts with default pet handler (bounces between targets)
		if (NOT_PULLING_BOT && NOT_RETURNING_BOT && HasControllablePet(BotAnimEmpathy::Attack)) {
			// We don't add to hate list here because it's assumed to already be on the list
			GetPet()->SetTarget(tar);
		}

		if (DivineAura()) {
			return;
		}

		if (!(m_PlayerState & static_cast<uint32>(PlayerState::Aggressive))) {
			SendAddPlayerState(PlayerState::Aggressive);
		}

// COMBAT RANGE CALCS
		bool front_mob = InFrontMob(tar, GetX(), GetY());
		bool behind_mob = BehindMob(tar, GetX(), GetY());
		bool stop_melee_level = GetLevel() >= GetStopMeleeLevel();

		tar_distance = sqrt(tar_distance); // sqrt this for future calculations
		const EQ::ItemInstance* p_item = GetBotItem(EQ::invslot::slotPrimary);
		const EQ::ItemInstance* s_item = GetBotItem(EQ::invslot::slotSecondary);

		CombatRangeInput i = {
			.target                 = tar,
			.target_distance        = tar_distance,
			.stop_melee_level       = stop_melee_level,
			.p_item                 = p_item,
			.s_item                 = s_item
		};

		CombatRangeOutput o = EvaluateCombatRange(i);

		// Combat range variables
		bool  at_combat_range    = o.at_combat_range;
		float melee_distance_min = o.melee_distance_min;
		float melee_distance     = o.melee_distance;
		float melee_distance_max = o.melee_distance_max;

// PULLING FLAG (ACTIONABLE RANGE)

		if (PULLING_BOT) {
			if (!TargetValidation(tar)) {
				SetPullFlag(false);
				SetPullingFlag(false);
				bot_owner->SetBotPulling(false);

				if (GetPet()) {
					GetPet()->SetPetOrder(PetOrder::Follow);
					GetPet()->CastToNPC()->SaveGuardSpot(true);
				}

				return;
			}

			if (!at_combat_range && RuleB(Bots, UseSpellPulling)) {
				uint16 pull_spell_id = RuleI(Bots, PullSpellID);

				if (IsValidSpell(pull_spell_id) && tar_distance <= spells[pull_spell_id].range) {
					at_combat_range = true;
				}
			}

			if (at_combat_range && DoLosChecks(tar)) {
				bool ai_cast_successful = false;
				bool can_range_attack = !tar->GetSpecialAbility(SpecialAbility::RangedAttackImmunity) &&
					RuleB(Bots, AllowRangedPulling) &&
					IsBotRanged() &&
					ranged_timer.Check(false);

				if (can_range_attack) {
					StopMoving(CalculateHeadingToTarget(tar->GetX(), tar->GetY()));

					if (BotRangedAttack(tar) && CheckDoubleRangedAttack()) {
						BotRangedAttack(tar, true);
					}

					ranged_timer.Start();

					return;
				}

				bool can_ai_spell_pull = RuleB(Bots, AllowAISpellPulling) &&
					!IsBotNonSpellFighter() &&
					AI_HasSpells();

				if (can_ai_spell_pull) {
					StopMoving(CalculateHeadingToTarget(tar->GetX(), tar->GetY()));
					SetPullingSpell(true);
					ai_cast_successful = AI_EngagedCastCheck();
					SetPullingSpell(false);

					if (ai_cast_successful) {
						return;
					}
				}

				if (RuleB(Bots, UseSpellPulling)) {
					uint16 pull_spell_id = RuleI(Bots, PullSpellID);

					if (IsValidSpell(pull_spell_id) && tar_distance <= spells[pull_spell_id].range) {
						StopMoving(CalculateHeadingToTarget(tar->GetX(), tar->GetY()));
						SetPullingSpell(true);
						CastSpell(pull_spell_id, tar->GetID());
						SetPullingSpell(false);
					}
				}

				return;
			}

			TryPursueTarget(leash_distance);

			return;
		}

		if (
			HasTargetReflection() &&
			!IsTaunting() &&
			!tar->IsFleeing() &&
			!tar->IsFeared() &&
			TryEvade(tar)
		) {
			return;
		}

// ENGAGED AT COMBAT RANGE

		// We can fight
		bool other_bot_pulling =
			(bot_owner->GetBotPulling() && NOT_PULLING_BOT) &&
			(bot_owner->GetBotPulling() && NOT_RETURNING_BOT);

		if (!other_bot_pulling && at_combat_range) {
			CombatPositioningInput cpi {
				.tar                    = tar,
				.stop_melee_level       = stop_melee_level,
				.tar_distance           = tar_distance,
				.melee_distance_min     = melee_distance_min,
				.melee_distance         = melee_distance,
				.melee_distance_max     = melee_distance_max,
				.behind_mob             = behind_mob,
				.front_mob              = front_mob
			};

			if (DoCombatPositioning(cpi) && IsMoving()) {
				return;
			}

			if (!IsSitting() && !IsFacingMob(tar)) {
				FaceTarget(tar);
				return;
			}

			if (!IsBotNonSpellFighter() && AI_HasSpells() && AI_EngagedCastCheck()) {
				return;
			}

			if (
				!tar->GetSpecialAbility(SpecialAbility::RangedAttackImmunity) &&
				IsBotRanged() &&
				ranged_timer.Check(false)
			) {
				if (BotRangedAttack(tar) && CheckDoubleRangedAttack()) {
					BotRangedAttack(tar, true);
				}

				ranged_timer.Start();
			}
			else if (!IsBotRanged() && !stop_melee_level) {
				if (
					IsTaunting() ||
					!GetMaxMeleeRange() ||
					!RuleB(Bots, DisableSpecialAbilitiesAtMaxMelee)
				) {
					DoClassAttacks(tar);
				}

				if (!TargetValidation(tar)) { return; }

				if (attack_timer.Check()) {
					TryCombatProcs(p_item, tar, EQ::invslot::slotPrimary);
					TriggerDefensiveProcs(tar, EQ::invslot::slotPrimary, false);
					DoAttackRounds(tar, EQ::invslot::slotPrimary);

					if (TryDoubleMeleeRoundEffect()) {
						DoAttackRounds(tar, EQ::invslot::slotPrimary);
					}
				}

				if (!TargetValidation(tar)) { return; }

				if (attack_dw_timer.Check()) {
					if (CanThisClassDualWield() && CastToClient()->CheckDualWield()) {
						TryCombatProcs(s_item, tar, EQ::invslot::slotSecondary);
						DoAttackRounds(tar, EQ::invslot::slotSecondary);
					}
				}

				if (!TargetValidation(tar)) { return; }

			}
		}

// ENGAGED NOT AT COMBAT RANGE

		else if (!other_bot_pulling && !TryPursueTarget(leash_distance)) {
			return;
		}

// End not in combat range
		if (bot_owner->GetBotPulling() && HasPet()) {
			GetPet()->WipeHateList();
			GetPet()->SetTarget(nullptr);
		}

		TryMeditate();
	}
	else { // Out-of-combat behavior
		DoOutOfCombatChecks(bot_owner, follow_mob, leash_distance, fm_distance);
	}
}

bool Bot::TryBardMovementCasts() {// Basically, bard bots get a chance to cast idle spells while moving
	if (HOLDING) {
		return false;
	}

	if (GetClass() == Class::Bard && IsMoving() && NOT_PASSIVE && !spellend_timer.Enabled() && AI_think_timer->Check()) {

		AI_IdleCastCheck();
		return true;
	}
	return false;
}

bool Bot::TryNonCombatMovementChecks(Client* bot_owner, const Mob* follow_mob) {// Non-engaged movement checks
	if (AI_movement_timer->Check() && (!IsCasting() || GetClass() == Class::Bard)) {
		glm::vec3 Goal(0, 0, 0);

		if (GUARDING) {
			Goal = GetGuardPoint();
		}
		else {
			Goal = follow_mob->GetPosition();
		}

		float destination_distance = DistanceSquared(GetPosition(), Goal);

		if (destination_distance > GetFollowDistance()) {
			if (!IsRooted()) {
				if (rest_timer.Enabled()) {
					rest_timer.Disable();
				}

				RunTo(Goal.x, Goal.y, Goal.z);

				return true;
			}
		}
		else {

			if (IsMoving()) {
				StopMoving();
				return true;
			}
		}
	}

	return false;
}

bool Bot::TryIdleChecks(float fm_distance) {

	if (
		(
			(NOT_GUARDING && fm_distance <= GetFollowDistance()) ||
			(GUARDING && DistanceSquared(GetPosition(), GetGuardPoint()) <= GetFollowDistance())) &&
			!IsMoving() &&
			AI_think_timer->Check() &&
			!spellend_timer.Enabled()
	) {

		if (!AI_IdleCastCheck() && !IsCasting()) {
			BotMeditate(IsSitting());
		}

		return true;
	}

	return false;
}

void Bot::DoOutOfCombatChecks(Client* bot_owner, Mob* follow_mob, float leash_distance, float fm_distance) {
	SetAttackFlag(false);
	SetCombatRoundForAlerts(false);
	SetAttackingFlag(false);

	if (PULLING_BOT || RETURNING_BOT || !bot_owner->GetBotPulling()) {
		SetPullingFlag(false);
		SetReturningFlag(false);
		bot_owner->SetBotPulling(false);

		if (GetPet()) {
			GetPet()->SetPetOrder(PetOrder::Follow);
			GetPet()->CastToNPC()->SaveGuardSpot(true);
		}
	}

	if (TryAutoDefend(bot_owner, leash_distance) ) {
		return;
	}

	SetTarget(nullptr);

	if (HasControllablePet(BotAnimEmpathy::BackOff)) {
		GetPet()->WipeHateList();
		GetPet()->SetTarget(nullptr);
	}

	if (m_PlayerState & static_cast<uint32>(PlayerState::Aggressive)) {
		SendRemovePlayerState(PlayerState::Aggressive);
	}

	// Ok to idle
	if (TryNonCombatMovementChecks(bot_owner, follow_mob)) {
		return;
	}

	if (!IsBotNonSpellFighter() && AI_HasSpells() && TryIdleChecks(fm_distance)) {
		return;
	}

	if (GetClass() == Class::Bard && AI_HasSpells() && TryBardMovementCasts()) {
		return;
	}

	TryMeditate();
}

// This is as close as I could get without modifying the aggro mechanics and making it an expensive process...
// 'class Client' doesn't make use of hate_list
bool Bot::TryAutoDefend(Client* bot_owner, float leash_distance) {

	if (RuleB(Bots, AllowOwnerOptionAutoDefend) && bot_owner->GetBotOption(Client::booAutoDefend)) {

		if (!m_auto_defend_timer.Enabled()) {

			m_auto_defend_timer.Start(zone->random.Int(250, 1250)); // random timer to simulate 'awareness' (cuts down on scanning overhead)
			return true;
		}

		if (
			m_auto_defend_timer.Check() &&
			NOT_HOLDING &&
			NOT_PASSIVE
		) {
			XTargetAutoHaters* temp_haters;
			std::vector<XTargetAutoHaters*> assistee_haters;
			std::vector<Client*> assistee_members;
			bool found = false;

			if (bot_owner->GetAggroCount()) {
				temp_haters = bot_owner->GetXTargetAutoMgr();

				if (temp_haters && !temp_haters->empty()) {
					assistee_haters.emplace_back(temp_haters);
					assistee_members.emplace_back(bot_owner);
				}
			}

			if (
				(!bot_owner->GetAssistee() || !entity_list.GetClientByCharID(bot_owner->GetAssistee())) &&
				RuleB(Bots, AllowCrossGroupRaidAssist)
			) {
				XTargetAutoHaters* temp_xhaters = bot_owner->GetXTargetAutoMgr();
				bool assistee_found = false;

				if (IsRaidGrouped()) {
					Raid* raid = GetStoredRaid();
					if (raid) {
						for (const auto& m : raid->members) {
							if (
								m.member &&
								m.member->IsClient() &&
								m.member->GetAggroCount() &&
								raid->IsAssister(m.member_name)
							) {
								temp_xhaters = m.member->GetXTargetAutoMgr();

								if (!temp_xhaters || temp_xhaters->empty()) {
									continue;
								}

								assistee_haters.emplace_back(temp_xhaters);
								assistee_members.emplace_back(m.member);
							}
						}
					}
				}
				else if (HasGroup()) {
					Group* g = GetGroup();
					if (g) {
						for (auto& m : g->members) {
							if (
								m &&
								m->IsClient() &&
								m->CastToClient()->GetAggroCount() &&
								g->AmIMainAssist(m->GetName())
							) {
								temp_xhaters = m->CastToClient()->GetXTargetAutoMgr();

								if (!temp_xhaters || temp_xhaters->empty()) {
									continue;
								}

								assistee_haters.emplace_back(temp_xhaters);
								assistee_members.emplace_back(m->CastToClient());
							}
						}
					}
				}
				else {
					return false;
				}
			}
			else {
				if (bot_owner->GetAssistee()) {
					Client* c = entity_list.GetClientByCharID(bot_owner->GetAssistee());

					if (bot_owner->IsInGroupOrRaid(c) && c->GetAggroCount()) {
						temp_haters = bot_owner->GetXTargetAutoMgr();

						if (temp_haters && !temp_haters->empty()) {
							assistee_haters.emplace_back(temp_haters);
							assistee_members.emplace_back(c);
						}
					}
				}
			}

			if (!assistee_haters.empty()) {
				for (XTargetAutoHaters* x_haters : assistee_haters) {
					if (!x_haters->empty()) {
						for (auto hater_iter : x_haters->get_list()) {
							if (!hater_iter.spawn_id) {
								continue;
							}

							Mob* hater = nullptr;

							for (Client* x_member : assistee_members) {
								if (
									x_member &&
									x_member->GetBotPulling() &&
									x_member->GetTarget() &&
									(hater_iter.spawn_id == x_member->GetTarget()->GetID())
								) {
									continue;
								}

								hater = entity_list.GetMob(hater_iter.spawn_id);

								if (
									hater &&
									!hater->IsMezzed() &&
									(DistanceSquared(hater->GetPosition(), bot_owner->GetPosition()) <= leash_distance) &&
									hater->CastToNPC()->IsOnHatelist(x_member)
								) {
									break;
								}

								hater = nullptr;
							}

							if (hater) {
								AddToHateList(hater, 1);
								SetTarget(hater);
								SetAttackingFlag();

								if (HasControllablePet(BotAnimEmpathy::Attack)) {
									GetPet()->AddToHateList(hater, 1);
									GetPet()->SetTarget(hater);
								}

								m_auto_defend_timer.Disable();

								return true;
							}
						}
					}
				}
			}

			return false;
		}
	}

	return false;
}

bool Bot::TryMeditate() {
	if (!IsMoving() && !spellend_timer.Enabled()) {
		if (IsEngaged()) {
			if (HasOrMayGetAggro(IsSitting())) {
				if (IsSitting()) {
					Stand();
				}

				return false;
			}

			for (auto mob : hate_list.GetHateList()) {
				auto tar = mob->entity_on_hatelist;

				if (tar) {
					Mob* tar_target = tar->GetTarget();

					if (tar_target && tar_target == this) {
						return false;
					}
				}
			}
		}

		BotMeditate(IsSitting());

		if (!(GetPlayerState() & static_cast<uint32>(PlayerState::Aggressive))) {
			SendAddPlayerState(PlayerState::Aggressive);
		}

		return true;
	}

	return false;
}

// This code actually gets processed when we are too far away from target and have not engaged yet
bool Bot::TryPursueTarget(float leash_distance) {
	if (AI_movement_timer->Check() && (!spellend_timer.Enabled() || GetClass() == Class::Bard)) {
		if (GetTarget() && !IsRooted()) {
			LogAIDetail("Pursuing [{}] while engaged", GetTarget()->GetCleanName());
			glm::vec3 Goal = GetTarget()->GetPosition();

			if (DistanceSquared(m_Position, Goal) <= leash_distance) {
				RunTo(Goal.x, Goal.y, Goal.z);
			} else {
				WipeHateList();
				SetTarget(nullptr);

				if (HasControllablePet(BotAnimEmpathy::BackOff)) {
					GetPet()->WipeHateList();
					GetPet()->SetTarget(nullptr);
				}
			}

			return true;
		} else {
			if (IsMoving()) {
				StopMoving();
			}

			return false;
		}
	}

	if (GetTarget() && GetTarget()->IsFeared() && !spellend_timer.Enabled() && AI_think_timer->Check()) {
		if (!IsFacingMob(GetTarget())) {
			FaceTarget(GetTarget());
		}

		// This is a mob that is fleeing either because it has been feared or is low on hitpoints
		if (!HOLDING && !IsBotNonSpellFighter() && AI_HasSpells()) {
			AI_PursueCastCheck();
		}

		return true;
	}

	return false;
}

void Bot::DoAttackRounds(Mob* target, int hand) {
	if (!target || (target && target->IsCorpse())) {
		return;
	}

	Attack(target, hand, false, false);

	bool candouble = CanThisClassDoubleAttack();
	// extra off hand non-sense, can only double with skill of 150 or above
	// or you have any amount of GiveDoubleAttack
	if (candouble && hand == EQ::invslot::slotSecondary)
		candouble =
		GetSkill(EQ::skills::SkillDoubleAttack) > 149 ||
		(aabonuses.GiveDoubleAttack + spellbonuses.GiveDoubleAttack + itembonuses.GiveDoubleAttack) > 0;

	if (candouble) {
		if (CheckBotDoubleAttack()) {
			Attack(target, hand, false, false);

			if (hand == EQ::invslot::slotPrimary) {
				bool is_two_hander = HasTwoHanderEquipped();

				auto extra_attack_chance = is_two_hander
					? aabonuses.ExtraAttackChance[SBIndex::EXTRA_ATTACK_CHANCE] +
					spellbonuses.ExtraAttackChance[SBIndex::EXTRA_ATTACK_CHANCE] +
					itembonuses.ExtraAttackChance[SBIndex::EXTRA_ATTACK_CHANCE]
					: aabonuses.ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_CHANCE] +
					spellbonuses.ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_CHANCE] +
					itembonuses.ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_CHANCE];

				int extra_attack_amt_aas = is_two_hander
					? aabonuses.ExtraAttackChance[SBIndex::EXTRA_ATTACK_NUM_ATKS]
					: aabonuses.ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_NUM_ATKS];

				int extra_attack_amt_spells = is_two_hander
					? spellbonuses.ExtraAttackChance[SBIndex::EXTRA_ATTACK_NUM_ATKS]
					: spellbonuses.ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_NUM_ATKS];

				int extra_attack_amt_items = is_two_hander
					? itembonuses.ExtraAttackChance[SBIndex::EXTRA_ATTACK_NUM_ATKS]
					: itembonuses.ExtraAttackChancePrimary[SBIndex::EXTRA_ATTACK_NUM_ATKS];

				int extra_attack_amt = std::max({ extra_attack_amt_aas, extra_attack_amt_spells, extra_attack_amt_items });

				if (extra_attack_chance && zone->random.Roll(extra_attack_chance)) {
					for (int i = 0; i < extra_attack_amt; i++) {
						Attack(target, hand, false, false);
					}
				}
			}

			if (hand == EQ::invslot::slotSecondary) {
				auto extra_attack_chance_secondary =
					aabonuses.ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_CHANCE] +
					spellbonuses.ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_CHANCE] +
					itembonuses.ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_CHANCE];

				if (extra_attack_chance_secondary && zone->random.Roll(extra_attack_chance_secondary)) {
					auto extra_attack_amt_secondary = std::max({
						aabonuses.ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_NUM_ATKS],
						spellbonuses.ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_NUM_ATKS],
						itembonuses.ExtraAttackChanceSecondary[SBIndex::EXTRA_ATTACK_NUM_ATKS]
					});

					for (int i = 0; i < extra_attack_amt_secondary; i++) {
						Attack(target, hand, false, false);
					}
				}
			}

			// you can only triple from the main hand
			if (hand == EQ::invslot::slotPrimary && CanThisClassTripleAttack()) {
				if (CheckTripleAttack()) {
					Attack(target, hand, false, false);

					int flurry_chance = aabonuses.FlurryChance + spellbonuses.FlurryChance + itembonuses.FlurryChance;

					if (flurry_chance && zone->random.Roll(flurry_chance)) {
						Attack(target, hand, false, false);

						if (zone->random.Roll(flurry_chance)) {
							Attack(target, hand, false, false);
						}

						if (GetOwner()) {
							GetOwner()->MessageString(
								Chat::NPCFlurry,
								NPC_FLURRY,
								GetCleanName(),
								target->GetCleanName()
							);
						}
					}
				}
			}
		}
	}
}

bool Bot::TryFacingTarget(Mob* tar) {
	if (!IsSitting() && !IsFacingMob(tar)) {
		FaceTarget(tar);
		return true;
	}
	return false;
}


bool Bot::TryEvade(Mob* tar) {
	if (GetSpellTypeHold(BotSpellTypes::Escape)) {
		return false;
	}

	switch (GetClass()) {
		case Class::Rogue: {
			if (GetSkill(EQ::skills::SkillHide) == 0) {
				return false;
			}

			if (m_rogue_evade_timer.Check(false)) {
				int timer_duration = (HideReuseTime - GetSkillReuseTime(EQ::skills::SkillHide)) * 1000;

				if (timer_duration < 0) {
					timer_duration = 0;
				}

				m_rogue_evade_timer.Start(timer_duration);

				if (zone->random.Int(0, 260) < (int)GetSkill(EQ::skills::SkillHide)) {

				}

				float hidechance = ((GetSkill(EQ::skills::SkillHide) / 250.0f) + .25) * 100;
				float random = zone->random.Real(0, 100);

				if (random < hidechance) {
					//SendAppearancePacket(AT_Invis, Invisibility::Invisible);

					if (spellbonuses.ShroudofStealth || aabonuses.ShroudofStealth || itembonuses.ShroudofStealth) {
						improved_hidden = true;
						hidden = true;
					}
					else {
						hidden = true;
					}
				}

				if (zone->random.Int(0, 260) < (int)GetSkill(EQ::skills::SkillHide)) {
					RaidGroupSay("I have momentarily ducked away from the main combat.");
					RogueEvade(tar);
				}
				else {
					RaidGroupSay("My attempts at ducking clear of combat fail.");
				}

				//SendAppearancePacket(AT_Invis, Invisibility::Visible);
				hidden = false;

				return true;
			}

			break;
		}
		case Class::Monk: {
			if (GetSkill(EQ::skills::SkillFeignDeath) == 0) {
				return false;
			}

			if (m_monk_evade_timer.Check(false)) {
				int timer_duration = (FeignDeathReuseTime - GetSkillReuseTime(EQ::skills::SkillFeignDeath)) * 1000;

				if (timer_duration < 0) {
					timer_duration = 0;
				}

				m_monk_evade_timer.Start(timer_duration);

				uint16 primfeign = GetSkill(EQ::skills::SkillFeignDeath);
				uint16 secfeign = GetSkill(EQ::skills::SkillFeignDeath);
				if (primfeign > 100) {
					primfeign = 100;
					secfeign = secfeign - 100;
					secfeign = secfeign / 2;
				}
				else
					secfeign = 0;

				uint16 totalfeign = primfeign + secfeign;

				if (zone->random.Real(0, 160) > totalfeign) {
					//SendAppearancePacket(AT_Anim, ANIM_DEATH);
					RaidGroupSay("I have fallen to the ground.");
					SetFeigned(false);
				}
				else {
					RaidGroupSay("I have successfully feigned my death.");
					SetFeigned(true);
					//SendAppearancePacket(AT_Anim, ANIM_DEATH);
				}

				//SendAppearancePacket(AT_Anim, ANIM_STAND);
				SetFeigned(false);
				return true;
			}
		}
	}

	return false;
}

CombatRangeOutput Bot::EvaluateCombatRange(const CombatRangeInput& input) {
	CombatRangeOutput o;

	float size_mod = GetSize();
	float other_size_mod = input.target->GetSize();

	// For races with a fixed size
	if (GetRace() == Race::LavaDragon || GetRace() == Race::Wurm || GetRace() == Race::GhostDragon) {
		size_mod = 60.0f;
	}
	else if (size_mod < 6.0f) {
		size_mod = 8.0f;
	}

	// For races with a fixed size
	if (input.target->GetRace() == Race::LavaDragon || input.target->GetRace() == Race::Wurm || input.target->GetRace() == Race::GhostDragon) {
		other_size_mod = 60.0f;
	}
	else if (other_size_mod < 6.0f) {
		other_size_mod = 8.0f;
	}

	if (other_size_mod > size_mod) {
		size_mod = other_size_mod;
	}

	if (size_mod > 29.0f) {
		size_mod *= size_mod;
	}
	else if (size_mod > 19.0f) {
		size_mod *= (size_mod * 2.0f);
	}
	else {
		size_mod *= (size_mod * 4.0f);
	}

	if (input.target->GetRace() == Race::VeliousDragon)		// Lord Vyemm and other velious dragons
	{
		size_mod *= 1.75;
	}

	if (input.target->GetRace() == Race::DragonSkeleton)		// Dracoliche in Fear.  Skeletal Dragon
	{
		size_mod *= 2.25;
	}

	size_mod *= RuleR(Combat, HitBoxMod);		// used for testing sizemods on different races.

	// Prevention of ridiculously sized hit boxes
	if (size_mod > 10000.0f) {
		size_mod = (size_mod / 7.0f);
	}

	o.melee_distance_max        = sqrt(size_mod);

	bool is_two_hander          = input.p_item && input.p_item->GetItem()->IsType2HWeapon();
	bool is_shield              = input.s_item && input.s_item->GetItem()->IsTypeShield();
	bool is_backstab_weapon     = input.p_item && input.p_item->GetItemBackstabDamage();

	if (IsTaunting()) { // Taunting bots
		o.melee_distance_min    = o.melee_distance_max * 0.25f;
		o.melee_distance        = o.melee_distance_max * 0.45f;
	}
	else if (IsBotRanged()) { // Archers/Throwers
		float min_distance      = RuleI(Combat, MinRangedAttackDist);
		float max_distance      = GetBotRangedValue();
		float desired_range     = GetBotDistanceRanged();

		max_distance            = (max_distance == 0 ? desired_range : max_distance); // stay ranged even if items/ammo aren't correct
		o.melee_distance_min    = std::max(min_distance, (desired_range * 0.75f));
		o.melee_distance        = std::min(max_distance, desired_range);
	}
	else if (input.stop_melee_level) { // Casters
		float desired_range     = GetBotDistanceRanged();

		o.melee_distance_min    = std::max(o.melee_distance_max, (desired_range * 0.75f));
		o.melee_distance        = std::max((o.melee_distance_max * 1.25f), desired_range);
	}
	else if (GetMaxMeleeRange()) { // Melee bots set to max melee range
		o.melee_distance_min    = o.melee_distance_max * 0.80f;
		o.melee_distance        = o.melee_distance_max * 0.95f;
	}
	else { // Regular melee
		o.melee_distance_min    = o.melee_distance_max * 0.30f;
		o.melee_distance        = o.melee_distance_max * 0.65f;
	}

	o.at_combat_range = (input.target_distance <= o.melee_distance);

	return o;
}

bool Bot::IsValidTarget(
	Client* bot_owner,
	Client* leash_owner,
	float lo_distance,
	float leash_distance,
	Mob* tar,
	float tar_distance
)
{
	if (!tar || !bot_owner || !leash_owner) {
		return false;
	}

	SetHasLoS(DoLosChecks(tar));

	bool invalid_target_state = false;

	if (HOLDING ||
		!tar->IsNPC() ||
		(tar->IsMezzed() && !HasBotAttackFlag(tar)) ||
		(!Charmed() && tar->GetUltimateOwner()->IsOfClientBotMerc()) ||
		lo_distance > leash_distance ||
		tar_distance > leash_distance ||
		(!GetAttackingFlag() && !HasLoS()) ||
		!IsAttackAllowed(tar)
	) {
		invalid_target_state = true;
	}

	if (invalid_target_state) {
		// Normally, we wouldn't want to do this without class checks..but, too many issues can arise if we let enchanter animation pets run rampant
		if (HasPet()) {
			GetPet()->RemoveFromHateList(tar);
			GetPet()->RemoveFromRampageList(tar);
			GetPet()->SetTarget(nullptr);
		}

		RemoveFromHateList(tar);
		RemoveFromRampageList(tar);
		SetTarget(nullptr);

		SetAttackFlag(false);
		SetCombatRoundForAlerts(false);
		SetAttackingFlag(false);

		if (PULLING_BOT || RETURNING_BOT) {
			SetPullingFlag(false);
			SetReturningFlag(false);
			bot_owner->SetBotPulling(false);

			if (GetPet()) {
				GetPet()->SetPetOrder(PetOrder::Follow);
				GetPet()->CastToNPC()->SaveGuardSpot(true);
			}
		}

		if (IsMoving()) {
			StopMoving();
		}

		return false;
	}

	return true;
}

Mob* Bot::GetBotTarget(Client* bot_owner)
{
	Mob* t = GetTarget();

	if (!t || PASSIVE) {
		if (GetTarget()) {
			SetTarget(nullptr);
		}

		WipeHateList();
		SetAttackFlag(false);
		SetAttackingFlag(false);

		if (PULLING_BOT || RETURNING_BOT) {
			// 'Flags' should only be set on the bot that is pulling
			SetPullingFlag(false);
			SetReturningFlag(false);
			bot_owner->SetBotPulling(false);

			if (GetPet()) {
				GetPet()->SetPetOrder(PetOrder::Follow);
				GetPet()->CastToNPC()->SaveGuardSpot(true);
			}
		}

		BotMeditate(IsSitting());
	}

	return t;
}

bool Bot::TargetValidation(Mob* other) {
	if (GetAppearance() == eaDead || GetHP() < 0) {
		return false;
	}

	if (!other || other->GetAppearance() == eaDead || other->GetHP() < 0) {
		return false;
	}

	return true;
}

bool Bot::ReturningFlagChecks(Client* bot_owner, Mob* leash_owner, float fm_distance) {
	bool target_check = !GetTarget() || Distance(GetPosition(), GetTarget()->GetPosition()) <= 75.0f;
	bool returned_check = (NOT_GUARDING && fm_distance <= GetFollowDistance()) ||
		(GUARDING && DistanceSquared(GetPosition(), GetGuardPoint()) <= GetFollowDistance());

	if (target_check && returned_check) { // Once we're back, clear blocking flags so everyone else can join in
		WipeHateList();
		SetTarget(nullptr);
		SetPullingFlag(false);
		SetReturningFlag(false);
		bot_owner->SetBotPulling(false);

		if (GetPet()) {
			GetPet()->SetPetOrder(PetOrder::Follow);
			GetPet()->CastToNPC()->SaveGuardSpot(true);

			if (HasControllablePet(BotAnimEmpathy::BackOff)) {
				GetPet()->WipeHateList();
				GetPet()->SetTarget(nullptr);
			}
		}

		return false;
	}

	if (!IsMoving()) {
		glm::vec3 Goal(0, 0, 0);

		if (GUARDING) {
			Goal = GetGuardPoint();
		}
		else {
			Mob* follow_mob = entity_list.GetMob(GetFollowID());

			if (!follow_mob) {
				follow_mob = leash_owner;
				SetFollowID(leash_owner->GetID());
			}

			Goal = follow_mob->GetPosition();
		}

		RunTo(Goal.x, Goal.y, Goal.z);
	}

	return true;
}

bool Bot::PullingFlagChecks(Client* bot_owner) {
	if (!GetTarget()) {
		WipeHateList();
		SetTarget(nullptr);
		SetPullingFlag(false);
		SetReturningFlag(false);
		bot_owner->SetBotPulling(false);

		if (GetPet()) {
			GetPet()->SetPetOrder(PetOrder::Follow);
			GetPet()->CastToNPC()->SaveGuardSpot(true);
		}

		return false;
	}
	else if (GetTarget()->GetHateList().size()) {
		SetPullingFlag(false);
		SetReturningFlag();

		Mob* my_pet = GetPet();

		if (my_pet) {
			if (HasControllablePet(BotAnimEmpathy::BackOff)) {
				my_pet->WipeHateList();
				my_pet->SetTarget(nullptr);
			} else {
				my_pet->AddToHateList(GetTarget(), 1);
				my_pet->SetTarget(GetTarget());
			}
		}

		if (GetPlayerState() & static_cast<uint32>(PlayerState::Aggressive)) {
			SendRemovePlayerState(PlayerState::Aggressive);
		}

		return false;
	}

	return true;
}

void Bot::HealRotationChecks() {
	if (IsMyHealRotationSet()) {
		if (AIHealRotation(HealRotationTarget(), UseHealRotationFastHeals())) {
			m_member_of_heal_rotation->SetMemberIsCasting(this);
			m_member_of_heal_rotation->UpdateTargetHealingStats(HealRotationTarget());
			AdvanceHealRotation();
		}
		else {
			m_member_of_heal_rotation->SetMemberIsCasting(this, false);
			AdvanceHealRotation(false);
		}
	}
}

bool Bot::IsAIProcessValid(const Client* bot_owner, const Group* bot_group, const Raid* raid) {
	if (!bot_owner || (!bot_group && !raid) || !IsAIControlled()) {
		return false;
	}

	if (bot_owner->IsDead()) {
		SetTarget(nullptr);
		SetBotOwner(nullptr);
		return false;
	}

	return true;
}

bool Bot::CheckIfCasting(float fm_distance) {
	if (IsCasting()) {
		if (IsHealRotationMember() &&
			m_member_of_heal_rotation->CastingOverride() &&
			m_member_of_heal_rotation->CastingTarget() != nullptr &&
			m_member_of_heal_rotation->CastingReady() &&
			m_member_of_heal_rotation->CastingMember() == this &&
			!m_member_of_heal_rotation->MemberIsCasting(this))
		{
			InterruptSpell();
		}
		else if (AmICastingForHealRotation() && m_member_of_heal_rotation->CastingMember() == this) {
			AdvanceHealRotation(false);
			return true;
		}
		else if (GetClass() != Class::Bard) {
			if (IsEngaged()) {
				return true;
			}

			if (
				(NOT_GUARDING && fm_distance > GetFollowDistance()) || // Cancel out-of-combat casting if movement to follow mob is required
				(GUARDING && DistanceSquared(GetPosition(), GetGuardPoint()) > GetFollowDistance()) // Cancel out-of-combat casting if movement to guard point is required
			) {
				InterruptSpell();
			}

			return true;
		}
	}
	else if (IsHealRotationMember()) {
		m_member_of_heal_rotation->SetMemberIsCasting(this, false);
	}

	return false;
}

bool Bot::CheckIfIncapacitated() {
	if (GetPauseAI() || IsStunned() || IsMezzed() || (GetAppearance() == eaDead)) {
		if (IsCasting()) {
			InterruptSpell();
		}

		if (IsMyHealRotationSet() || (AmICastingForHealRotation() && m_member_of_heal_rotation->CastingMember() == this)) {
			AdvanceHealRotation(false);
			m_member_of_heal_rotation->SetMemberIsCasting(this, false);
		}

		return true;
	}

	if (IsRooted() && IsMoving()) {
		StopMoving();
		return true;
	}

	if (currently_fleeing) {
		if (RuleB(Combat, EnableFearPathing) && AI_movement_timer->Check()) {
			// Check if we have reached the last fear point
			if (DistanceNoZ(glm::vec3(GetX(), GetY(), GetZ()), m_FearWalkTarget) <= 5.0f) {
				// Calculate a new point to run to
				StopNavigation();
				CalculateNewFearpoint();
			}

			RunTo(
				m_FearWalkTarget.x,
				m_FearWalkTarget.y,
				m_FearWalkTarget.z
			);
		}

		return true;
	}

	return false;
}

void Bot::SetBerserkState() {// Berserk updates should occur if primary AI criteria are met
	if (GetClass() == Class::Warrior || GetClass() == Class::Berserker) {
		if (!berserk && GetHPRatio() < RuleI(Combat, BerserkerFrenzyStart)) {
			entity_list.MessageCloseString(this, false, 200, 0, BERSERK_START, GetName());
			berserk = true;
		}
		if (berserk && GetHPRatio() > RuleI(Combat, BerserkerFrenzyEnd)) {
			entity_list.MessageCloseString(this, false, 200, 0, BERSERK_END, GetName());
			berserk = false;
		}
	}
}

Mob* Bot::SetFollowMob(Client* leash_owner) {
	Mob* follow_mob = entity_list.GetMob(GetFollowID());

	if (!follow_mob) {
		follow_mob = leash_owner;
		SetFollowID(leash_owner->GetID());
	}

	return follow_mob;
}

Client* Bot::SetLeashOwner(Client* bot_owner, Group* bot_group, Raid* raid, uint32 r_group) const {
	Client* leash_owner = nullptr;

	if (raid && r_group < MAX_RAID_GROUPS && raid->GetGroupLeader(r_group)) {
		leash_owner =
			raid->GetGroupLeader(r_group) &&
			raid->GetGroupLeader(r_group)->IsClient() ?
				raid->GetGroupLeader(r_group)->CastToClient() : bot_owner;
	} else if (bot_group) {
		leash_owner = (bot_group->GetLeader() && bot_group->GetLeader()->IsClient() ? bot_group->GetLeader()->CastToClient() : bot_owner);
	} else {
		leash_owner = bot_owner;
	}

	return leash_owner;
}

void Bot::SetOwnerTarget(Client* bot_owner) {
	if (GetPet() && (PULLING_BOT || RETURNING_BOT)) {
		GetPet()->SetPetOrder(PetOrder::Follow);
	}

	SetAttackFlag(false);
	SetAttackingFlag(false);
	SetPullFlag(false);
	SetPullingFlag(false);
	SetReturningFlag(false);
	bot_owner->SetBotPulling(false);

	if (NOT_HOLDING && NOT_PASSIVE) {
		auto attack_target = bot_owner->GetTarget();

		if (attack_target && HasBotAttackFlag(attack_target)) {
			InterruptSpell();
			WipeHateList();
			AddToHateList(attack_target, 1);
			SetTarget(attack_target);
			SetAttackingFlag();

			if (HasControllablePet(BotAnimEmpathy::Attack)) {
				GetPet()->WipeHateList();
				GetPet()->AddToHateList(attack_target, 1);
				GetPet()->SetTarget(attack_target);
			}
		}
	}
}

void Bot::BotPullerProcess(Client* bot_owner, Raid* raid) {
	SetAttackFlag(false);
	SetCombatRoundForAlerts(false);
	SetAttackingFlag(false);
	SetPullFlag(false);
	SetPullingFlag(false);
	SetReturningFlag(false);
	bot_owner->SetBotPulling(false);

	if (GetPet()) {
		GetPet()->SetPetOrder(PetOrder::Follow);
		GetPet()->CastToNPC()->SaveGuardSpot(true);
	}

	if (NOT_HOLDING && NOT_PASSIVE) {
		auto pull_target = bot_owner->GetTarget();

		if (pull_target) {
			RaidGroupSay(
				fmt::format(
					"Pulling {}.",
					pull_target->GetCleanName()
				).c_str()
			);

			InterruptSpell();
			WipeHateList();
			AddToHateList(pull_target, 1);
			SetTarget(pull_target);
			SetPullingFlag();
			bot_owner->SetBotPulling();

			if (GetPet()) {
				GetPet()->WipeHateList();
				GetPet()->SetTarget(nullptr);

				if (HasControllablePet(BotAnimEmpathy::Guard)) {
					m_previous_pet_order = GetPet()->GetPetOrder();
					GetPet()->CastToNPC()->SaveGuardSpot(GetPosition());
					GetPet()->SetPetOrder(PetOrder::Guard);
				}
			}
		}
	}
}

void Bot::Depop() {
	WipeHateList();
	entity_list.RemoveFromHateLists(this);
	RemoveAllAuras();

	Mob* bot_pet = GetPet();

	if (bot_pet) {
		if (bot_pet->Charmed()) {
			bot_pet->BuffFadeByEffect(SpellEffect::Charm);
		}
		else {
			bot_pet->Depop();
		}
	}

	_botOwner = nullptr;
	_botOwnerCharacterID = 0;

	NPC::Depop(false);
}

bool Bot::Spawn(Client* botCharacterOwner) {
	if (
		GetBotID() &&
		_botOwnerCharacterID &&
		botCharacterOwner &&
		botCharacterOwner->CharacterID() == _botOwnerCharacterID
	) {
		// Rename the bot name to make sure that Mob::GetName() matches Mob::GetCleanName() so we dont have a bot named "Jesuschrist001"
		strcpy(name, GetCleanName());

		// Get the zone id this bot spawned in
		_lastZoneId = GetZoneID();

		// this change propagates to Bot::FillSpawnStruct()
		helmtexture = 0; //0xFF;
		texture = 0; //0xFF;

		if (Save()) {
			GetBotOwner()->CastToClient()->Message(
				Chat::White,
				fmt::format(
					"{} saved.",
					GetCleanName()
				).c_str()
			);
		} else {
			GetBotOwner()->CastToClient()->Message(
				Chat::White,
				fmt::format(
					"{} save failed!",
					GetCleanName()
				).c_str()
			);
		}

		// Spawn the bot at the bot owner's loc
		m_Position.x = botCharacterOwner->GetX();
		m_Position.y = botCharacterOwner->GetY();
		m_Position.z = botCharacterOwner->GetZ();

		// Make the bot look at the bot owner
		FaceTarget(botCharacterOwner);
		UpdateEquipmentLight();
		UpdateActiveLight();

		m_targetable = true;
		entity_list.AddBot(this, true, true);

		ClearDataBucketCache();
		LoadDataBucketsCache();
		LoadBotSpellSettings();
		if (!AI_AddBotSpells(GetBotSpellID())) {
			GetBotOwner()->CastToClient()->Message(
				Chat::White,
				fmt::format(
					"Failed to load spells for '{}' (ID {}).",
					GetCleanName(),
					GetBotID()
				).c_str()
			);
		}

		// Load pet
		LoadPet();
		SentPositionPacket(0.0f, 0.0f, 0.0f, 0.0f, 0);
		m_ping_timer.Start(8000);
		m_auto_save_timer.Start(RuleI(Bots, AutosaveIntervalSeconds) * 1000);

		m_dont_heal_me_before                = 0;
		m_dont_buff_me_before                = Timer::GetCurrentTime() + 400;
		m_dont_dot_me_before                 = 0;
		m_dont_root_me_before                = 0;
		m_dont_snare_me_before               = 0;
		m_dont_cure_me_before                = 0;


		// there is something askew with spawn struct appearance fields...
		// I re-enabled this until I can sort it out
		const auto& m = GetBotItemSlots();
		uint8 material_from_slot = 0xFF;
		for (int slot_id = EQ::invslot::EQUIPMENT_BEGIN; slot_id <= EQ::invslot::EQUIPMENT_END; ++slot_id) {
			if (m.find(slot_id) != m.end()) {
				material_from_slot = EQ::InventoryProfile::CalcMaterialFromSlot(slot_id);
				if (material_from_slot != 0xFF) {
					SendWearChange(material_from_slot);
				}
			}
		}

		if (auto raid = entity_list.GetRaidByBotName(GetName())) {
			// Safety Check to confirm we have a valid raid
			auto owner = GetBotOwner();
			if (owner && !raid->IsRaidMember(owner->GetCleanName())) {
				Bot::RemoveBotFromRaid(this);
			} else {
				SetRaidGrouped(true);
				raid->LearnMembers();
				raid->VerifyRaid();
				SetStoredRaid(raid);
				p_raid_instance = raid;
				SetVerifiedRaid(true);
			}
		}
		else if (auto group = entity_list.GetGroupByMobName(GetName())) {
			// Safety Check to confirm we have a valid group
			auto owner = GetBotOwner();
			if (owner && !group->IsGroupMember(owner->GetCleanName())) {
				Bot::RemoveBotFromGroup(this, group);
			} else {
				SetGrouped(true);
				group->LearnMembers();
				group->VerifyGroup();
			}
		}

		if (IsBotRanged()) {
			ChangeBotRangedWeapons(true);
		}

		return true;
	}

	return false;
}

// Deletes the inventory record for the specified item from the database for this bot.
void Bot::RemoveBotItemBySlot(uint16 slot_id)
{
	if (!GetBotID()) {
		return;
	}

	database.botdb.DeleteItemBySlot(GetBotID(), slot_id);

	m_inv.DeleteItem(slot_id);
	UpdateEquipmentLight();
}

// Retrieves all the inventory records from the database for this bot.
void Bot::GetBotItems(EQ::InventoryProfile &inv)
{
	if (!GetBotID()) {
		return;
	}

	if (!database.botdb.LoadItems(GetBotID(), inv)) {
		return;
	}

	UpdateEquipmentLight();
}

std::map<uint16, uint32> Bot::GetBotItemSlots()
{
	std::map<uint16, uint32> m;
	if (!GetBotID()) {
		return m;
	}

	if (!database.botdb.LoadItemSlots(GetBotID(), m)) {
		GetBotOwner()->CastToClient()->Message(
			Chat::White,
			fmt::format(
				"Failed to load inventory slots for {}.",
				GetCleanName()
			).c_str()
		);
	}

	return m;
}

// Returns the inventory record for this bot from the database for the specified equipment slot.
uint32 Bot::GetBotItemBySlot(uint16 slot_id)
{
	uint32 item_id = 0;
	if (!GetBotID()) {
		return item_id;
	}

	if (!database.botdb.LoadItemBySlot(GetBotID(), slot_id, item_id) && GetBotOwner() && GetBotOwner()->IsClient()) {
		GetBotOwner()->CastToClient()->Message(
			Chat::White,
			fmt::format(
				"Failed to load slot ID {} for {}.",
				slot_id,
				GetCleanName()
			).c_str()
		);
	}

	return item_id;
}

void Bot::SetLevel(uint8 in_level, bool command) {
	if (in_level > 0)
		Mob::SetLevel(in_level, command);
}

void Bot::FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho) {
	if (ns) {
		Mob::FillSpawnStruct(ns, ForWho);
		ns->spawn.afk = 0;
		ns->spawn.lfg = 0;
		ns->spawn.anon = 0;
		ns->spawn.gm = 0;
		if (IsInAGuild())
			ns->spawn.guildID = GuildID();
		else
			ns->spawn.guildID = 0xFFFFFFFF;		// 0xFFFFFFFF = NO GUILD, 0 = Unknown Guild
		ns->spawn.is_npc = 0;				// 0=no, 1=yes
		ns->spawn.is_pet = 0;
		ns->spawn.guildrank = 0;
		ns->spawn.showhelm = GetShowHelm() ? 1 : 0;
		ns->spawn.flymode = 0;
		ns->spawn.size = 0;
		ns->spawn.NPC = 0;					// 0=player,1=npc,2=pc corpse,3=npc corpse
		UpdateActiveLight();
		ns->spawn.light = m_Light.Type[EQ::lightsource::LightActive];
		ns->spawn.helm = helmtexture; //(GetShowHelm() ? helmtexture : 0); //0xFF;
		ns->spawn.equip_chest2 = texture; //0xFF;
		ns->spawn.show_name = true;
		strcpy(ns->spawn.lastName, GetSurname().c_str());
		strcpy(ns->spawn.title, GetTitle().c_str());
		strcpy(ns->spawn.suffix, GetSuffix().c_str());
		const EQ::ItemData* item = nullptr;
		const EQ::ItemInstance* inst = nullptr;
		for (int i = EQ::textures::textureBegin; i < EQ::textures::weaponPrimary; i++) {
			inst = GetBotItem(i);
			if (inst) {
				item = inst->GetItem();
				if (item != nullptr) {
					ns->spawn.equipment.Slot[i].Material = item->Material;
					ns->spawn.equipment.Slot[i].EliteModel = item->EliteMaterial;
					ns->spawn.equipment.Slot[i].HerosForgeModel = item->HerosForgeModel;
					if (armor_tint.Slot[i].Color)
						ns->spawn.equipment_tint.Slot[i].Color = armor_tint.Slot[i].Color;
					else
						ns->spawn.equipment_tint.Slot[i].Color = item->Color;
				} else {
					if (armor_tint.Slot[i].Color)
						ns->spawn.equipment_tint.Slot[i].Color = armor_tint.Slot[i].Color;
				}
			}
		}

		inst = GetBotItem(EQ::invslot::slotPrimary);
		if (inst) {
			item = inst->GetItem();

			if(item) {
				if(strlen(item->IDFile) > 2)
					ns->spawn.equipment.Primary.Material = Strings::ToUnsignedInt(&item->IDFile[2]);


				ns->spawn.equipment_tint.Primary.Color = GetEquipmentColor(EQ::textures::weaponPrimary);
			}
		}

		inst = GetBotItem(EQ::invslot::slotSecondary);
		if (inst) {
			item = inst->GetItem();

			if(item) {
				if(strlen(item->IDFile) > 2)
					ns->spawn.equipment.Secondary.Material = Strings::ToUnsignedInt(&item->IDFile[2]);

				ns->spawn.equipment_tint.Secondary.Color = GetEquipmentColor(EQ::textures::weaponSecondary);
			}
		}
	}
}

Bot* Bot::LoadBot(uint32 botID)
{
	Bot* loaded_bot = nullptr;
	if (!botID)
		return loaded_bot;

	if (!database.botdb.LoadBot(botID, loaded_bot)) // TODO: Consider update to message handler
		return loaded_bot;

	return loaded_bot;
}

// Load and spawn all zoned bots by bot owner character
void Bot::LoadAndSpawnAllZonedBots(Client* bot_owner) {
	if (bot_owner && bot_owner->HasGroup()) {
		std::vector<int> bot_class_spawn_limits;
		std::vector<int> bot_class_spawned_count = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

		for (uint8 class_id = Class::Warrior; class_id <= Class::Berserker; class_id++) {
			auto bot_class_limit = bot_owner->GetBotSpawnLimit(class_id);
			bot_class_spawn_limits.push_back(bot_class_limit);
		}

		auto* g = bot_owner->GetGroup();
		if (g) {
			uint32 group_id = g->GetID();
			std::list<uint32> active_bots;

			auto spawned_bots_count = 0;
			auto bot_spawn_limit = bot_owner->GetBotSpawnLimit();

			if (!database.botdb.LoadGroupedBotsByGroupID(bot_owner->CharacterID(), group_id, active_bots)) {
				bot_owner->Message(Chat::White, "Failed to load grouped bots by group ID.");
				return;
			}

			if (!active_bots.empty()) {
				for (const auto& bot_id : active_bots) {
					auto* b = Bot::LoadBot(bot_id);
					if (!b) {
						continue;
					}

					if (bot_spawn_limit >= 0 && spawned_bots_count >= bot_spawn_limit) {
						Group::RemoveFromGroup(b);
						g->UpdatePlayer(bot_owner);
						continue;
					}

					auto spawned_bot_count_class = bot_class_spawned_count[b->GetClass() - 1];

					if (
						auto bot_spawn_limit_class = bot_class_spawn_limits[b->GetClass() - 1];
						bot_spawn_limit_class >= 0 &&
						spawned_bot_count_class >= bot_spawn_limit_class
					) {
						Group::RemoveFromGroup(b);
						g->UpdatePlayer(bot_owner);
						continue;
					}

					if (!b->Spawn(bot_owner)) {
						safe_delete(b);
						continue;
					}

					spawned_bots_count++;
					bot_class_spawned_count[b->GetClass() - 1]++;

					g->UpdatePlayer(b);

					if (g->IsGroupMember(bot_owner) && g->IsGroupMember(b)) {
						b->SetFollowID(bot_owner->GetID());
					}

					if (!bot_owner->HasGroup()) {
						Group::RemoveFromGroup(b);
					}
				}
			}
		}
	}
}

// Returns TRUE if there is atleast 1 bot in the specified group
bool Bot::GroupHasBot(Group* group) {
	bool Result = false;
	if (group) {
		for (int Counter = 0; Counter < MAX_GROUP_MEMBERS; Counter++) {
			if (group->members[Counter] == nullptr)
				continue;

			if (group->members[Counter]->IsBot()) {
				Result = true;
				break;
			}
		}
	}
	return Result;
}

uint32 Bot::SpawnedBotCount(const uint32 owner_id, uint8 class_id) {
	uint32 spawned_bot_count = 0;

	if (owner_id) {
		const auto& sbl = entity_list.GetBotListByCharacterID(owner_id, class_id);
		spawned_bot_count = sbl.size();
	}

	return spawned_bot_count;
}

void Bot::LevelBotWithClient(Client* c, uint8 new_level, bool send_appearance) {
	// This essentially performs a '#bot update,' with appearance packets, based on the current methods.
	// This should not be called outside of Client::SetEXP() due to its lack of rule checks.

	if (c) {
		const auto &l = entity_list.GetBotsByBotOwnerCharacterID(c->CharacterID());

		for (const auto &e : l) {
			if (e && (e->GetLevel() != c->GetLevel())) {
				int levels_change = (new_level - e->GetLevel());

				if (levels_change < 0) {
					parse->EventBot(EVENT_LEVEL_DOWN, e, nullptr, std::to_string(std::abs(levels_change)), 0);
				} else {
					parse->EventBot(EVENT_LEVEL_UP, e, nullptr, std::to_string(levels_change), 0);
				}

				e->CalcBotStats(c->GetBotOption(Client::booStatsUpdate));

				if (send_appearance) {
					e->SendLevelAppearance();
				}

				if (!RuleB(Bots, BotHealOnLevel)) {
					const int64 max_hp = e->CalcMaxHP();
					if (e->GetHP() > max_hp) {
						e->SetHP(max_hp);
					}
				} else {
					e->SetHP(e->CalcMaxHP());
					e->SetMana(e->CalcMaxMana());
				}

				e->SendHPUpdate();
				e->SendAppearancePacket(AppearanceType::WhoLevel, new_level, true, true); // who level change
				e->AI_AddBotSpells(e->GetBotSpellID());
			}
		}
	}
}

// Returns the item id that is in the bot inventory collection for the specified slot.
EQ::ItemInstance* Bot::GetBotItem(uint16 slot_id) {
	EQ::ItemInstance* item = m_inv.GetItem(slot_id);
	if (item) {
		return item;
	}

	return nullptr;
}

// Adds the specified item it bot to the NPC equipment array and to the bot inventory collection.
void Bot::BotAddEquipItem(uint16 slot_id, uint32 item_id) {
	// this is being called before bot is assigned an entity id..
	// ..causing packets to be sent out to zone with an id of '0'
	if (item_id) {

		if (uint8 material_from_slot = EQ::InventoryProfile::CalcMaterialFromSlot(slot_id); material_from_slot != EQ::textures::materialInvalid) {
			equipment[slot_id] = item_id; // npc has more than just material slots. Valid material should mean valid inventory index
			if (GetID()) { // temp hack fix
				SendWearChange(material_from_slot);
			}
		}

		UpdateEquipmentLight();
		if (UpdateActiveLight() && GetID()) { // temp hack fix
			SendAppearancePacket(AppearanceType::Light, GetActiveLightType());
		}
	}
}

// Erases the specified item from bot the NPC equipment array and from the bot inventory collection.
void Bot::BotRemoveEquipItem(uint16 slot_id)
{
	uint8 material_slot = EQ::InventoryProfile::CalcMaterialFromSlot(slot_id);

	if (material_slot != EQ::textures::materialInvalid) {
		equipment[slot_id] = 0; // npc has more than just material slots. Valid material should mean valid inventory index
		SendWearChange(material_slot);
		if (material_slot == EQ::textures::armorChest) {
			SendWearChange(EQ::textures::armorArms);
		}
	}

	UpdateEquipmentLight();
	if (UpdateActiveLight()) {
		SendAppearancePacket(AppearanceType::Light, GetActiveLightType());
	}
}

void Bot::BotTradeAddItem(const EQ::ItemInstance* inst, uint16 slot_id, bool save_to_database)
{
	if (!inst) {
		return;
	}

	if (save_to_database) {
		if (!database.botdb.SaveItemBySlot(this, slot_id, inst)) {
			return;
		}

		m_inv.PutItem(slot_id, *inst);
	}

	auto item_id = inst ? inst->GetID() : 0;

	BotAddEquipItem(slot_id, item_id);
}

void Bot::AddBotItem(
	uint16 slot_id,
	uint32 item_id,
	int16 charges,
	bool attuned,
	uint32 augment_one,
	uint32 augment_two,
	uint32 augment_three,
	uint32 augment_four,
	uint32 augment_five,
	uint32 augment_six
) {
	auto inst = database.CreateItem(
		item_id,
		charges,
		augment_one,
		augment_two,
		augment_three,
		augment_four,
		augment_five,
		augment_six,
		attuned
	);

	if (!inst) {
		LogError(
			"Bot:AddItem Invalid Item data: ID [{}] Charges [{}] Aug1 [{}] Aug2 [{}] Aug3 [{}] Aug4 [{}] Aug5 [{}] Aug6 [{}] Attuned [{}]",
			item_id,
			charges,
			augment_one,
			augment_two,
			augment_three,
			augment_four,
			augment_five,
			augment_six,
			attuned
		);
		return;
	}

	if (!database.botdb.SaveItemBySlot(this, slot_id, inst)) {
		LogError("Failed to save item by slot to slot [{}] for [{}].", slot_id, GetCleanName());
		safe_delete(inst);
		return;
	}

	m_inv.PutItem(slot_id, *inst);
	safe_delete(inst);

	BotAddEquipItem(slot_id, item_id);
}

uint32 Bot::CountBotItem(uint32 item_id) {
	uint32 item_count = 0;
	EQ::ItemInstance *inst = nullptr;

	for (uint16 slot_id = EQ::invslot::EQUIPMENT_BEGIN; slot_id <= EQ::invslot::EQUIPMENT_END; ++slot_id) {
		inst = GetBotItem(slot_id);
		if (!inst || !inst->GetItem()) {
			continue;
		}

		if (inst->GetID() == item_id) {
			item_count++;
		}
	}

	return item_count;
}

int16 Bot::HasBotItem(uint32 item_id) {
	EQ::ItemInstance const *inst = nullptr;

	for (uint16 slot_id = EQ::invslot::EQUIPMENT_BEGIN; slot_id <= EQ::invslot::EQUIPMENT_END; ++slot_id) {
		inst = GetBotItem(slot_id);
		if (!inst || !inst->GetItem()) {
			continue;
		}
		if (inst->GetID() == item_id) {
			return slot_id;
		}
	}
	return INVALID_INDEX;
}

void Bot::RemoveBotItem(uint32 item_id) {
	EQ::ItemInstance *inst = nullptr;

	for (uint16 slot_id = EQ::invslot::EQUIPMENT_BEGIN; slot_id <= EQ::invslot::EQUIPMENT_END; ++slot_id) {
		inst = GetBotItem(slot_id);
		if (!inst || !inst->GetItem()) {
			continue;
		}


		if (inst->GetID() == item_id) {
			RemoveBotItemBySlot(slot_id);
			BotRemoveEquipItem(slot_id);
			CalcBotStats(GetOwner()->CastToClient()->GetBotOption(Client::booStatsUpdate));
			return;
		}
	}
}

bool Bot::RemoveBotFromGroup(Bot* bot, Group* group) {
	bool Result = false;
	if (bot && group && bot->HasGroup())  {
		if (!group->IsLeader(bot)) {
			bot->SetFollowID(0);
			if (group->DelMember(bot)) {
				group->DelMemberOOZ(bot->GetName());
				Group::RemoveFromGroup(bot);
				if (group->GroupCount() < 2) {
					group->DisbandGroup();
				}
			}
		} else {
			for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
				if (!group->members[i]) {
					continue;
				}

				group->members[i]->SetFollowID(0);
			}
			group->DisbandGroup();
			Group::RemoveFromGroup(bot);
		}
		Result = true;
	}
	return Result;
}

bool Bot::AddBotToGroup(Bot* bot, Group* group) {
	bool result = false;
	if (!group || group->GroupCount() >= MAX_GROUP_MEMBERS) {
		return result;
	}

	if (bot && group->AddMember(bot)) {
		if (group->GetLeader()) {
			// Need to send this only once when a group is formed with a bot so the client knows it is also the group leader
			if (group->GroupCount() == 2 && group->GetLeader()->IsClient()) {
				group->UpdateGroupAAs();
				Mob *TempLeader = group->GetLeader();
				group->SendUpdate(groupActUpdate, TempLeader);
			}
		}
		group->VerifyGroup();
		group->SendGroupJoinOOZ(bot);
		result = true;
	}
	return result;
}

// Completes a trade with a client bot owner
void Bot::FinishTrade(Client* client, BotTradeType trade_type, int16 chosen_slot)
{
	if (
		!client ||
		GetOwner() != client ||
		client->GetTradeskillObject() ||
		client->trade->state == Trading
	) {
		if (client) {
			client->ResetTrade();
		}

		return;
	}

	// these notes are not correct or obselete
	if (trade_type == BotTradeClientNormal) {
		// Items being traded are found in the normal trade window used to trade between a Client and a Client or NPC
		// Items in this mode are found in slot ids 3000 thru 3003 - thought bots used the full 8-slot window..?
		PerformTradeWithClient(EQ::invslot::TRADE_BEGIN, EQ::invslot::TRADE_END, client); // {3000..3007}
	}
	else if (trade_type == BotTradeClientNoDropNoTrade) {
		// Items being traded are found on the Client's cursor slot, slot id 30. This item can be either a single item or it can be a bag.
		// If it is a bag, then we have to search for items in slots 331 thru 340
		PerformTradeWithClient(EQ::invslot::slotCursor, EQ::invslot::slotCursor, client, chosen_slot);

		// TODO: Add logic here to test if the item in SLOT_CURSOR is a container type, if it is then we need to call the following:
		// PerformTradeWithClient(331, 340, client);
	}
}

// Perfoms the actual trade action with a client bot owner
void Bot::PerformTradeWithClient(int16 begin_slot_id, int16 end_slot_id, Client* client, int16 chosen_slot)
{
	using namespace EQ;

	struct ClientTrade {
		ItemInstance* trade_item_instance;
		int16 from_client_slot;
		int16 to_bot_slot = invslot::SLOT_INVALID;

		ClientTrade(ItemInstance* item, int16 from) : trade_item_instance(item), from_client_slot(from) { }
	};

	struct ClientReturn {
		ItemInstance* return_item_instance;
		int16 from_bot_slot;
		int16 to_client_slot = invslot::SLOT_INVALID;

		ClientReturn(ItemInstance* item, int16 from) : return_item_instance(item), from_bot_slot(from) { }
	};

	std::vector<int16> bot_equip_order;

	if (chosen_slot != INVALID_INDEX) {
		bot_equip_order.emplace_back(chosen_slot);
	}
	else {
		for (int16 i = 0; i < invslot::EQUIPMENT_COUNT; ++i) {
			bot_equip_order.push_back(i);
		}
	}

	enum { stageStackable = 0, stageEmpty, stageReplaceable };

	if (!client) {
		Emote("NO CLIENT");
		return;
	}

	if (client != GetOwner()) {
		client->Message(Chat::White, "You are not the owner of this bot, the trade has been cancelled.");
		client->ResetTrade();
		return;
	}

	if (begin_slot_id != invslot::TRADE_BEGIN && begin_slot_id != invslot::slotCursor) {
		client->Message(Chat::White, "Trade request processing from illegal 'begin' slot, the trade has been cancelled.");
		client->ResetTrade();
		return;
	}

	if (end_slot_id != invslot::TRADE_END && end_slot_id != invslot::slotCursor) {
		client->Message(Chat::White, "Trade request processing from illegal 'end' slot, the trade has been cancelled.");
		client->ResetTrade();
		return;
	}

	if ((begin_slot_id == invslot::slotCursor && end_slot_id != invslot::slotCursor) || (begin_slot_id != invslot::slotCursor && end_slot_id == invslot::slotCursor)) {
		client->Message(Chat::White, "Trade request processing illegal slot range, the trade has been cancelled.");
		client->ResetTrade();
		return;
	}

	if (end_slot_id < begin_slot_id) {
		client->Message(Chat::White, "Trade request processing in reverse slot order, the trade has been cancelled.");
		client->ResetTrade();
		return;
	}

	if (client->IsEngaged() || IsEngaged()) {
		client->Message(Chat::Yellow, "You may not perform a trade while engaged, the trade has been cancelled!");
		client->ResetTrade();
		return;
	}

	std::list<ClientTrade> client_trade;
	std::list<ClientTrade> event_trade;
	std::list<ClientReturn> client_return;

	bool trade_event_exists = false;
	if (parse->BotHasQuestSub(EVENT_TRADE)) {
		// There is a EVENT_TRADE, we will let the Event handle returning of items.
		trade_event_exists = true;
	}

	// pre-checks for incoming illegal transfers
	EQ::InventoryProfile& user_inv = client->GetInv();
	for (int16 trade_index = begin_slot_id; trade_index <= end_slot_id; ++trade_index) {
		auto trade_instance = user_inv.GetItem(trade_index);
		if (!trade_instance) {
			continue;
		}

		if (!trade_instance->GetItem()) {
			LogError("could not find item from instance in trade for [{}] with [{}] in slot [{}].", client->GetCleanName(), GetCleanName(), trade_index);
			client->Message(
				Chat::White,
				fmt::format(
					"A server error was encountered while processing client slot {}, the trade has been cancelled.",
					trade_index
				).c_str()
			);
			client->ResetTrade();
			return;
		}

		EQ::SayLinkEngine linker;
		linker.SetLinkType(EQ::saylink::SayLinkItemInst);
		linker.SetItemInst(trade_instance);

		auto item_link = linker.GenerateLink();

		if (trade_index != invslot::slotCursor && !trade_instance->IsDroppable()) {
			LogError("trade hack detected by [{}] with [{}].", client->GetCleanName(), GetCleanName());
			client->Message(Chat::White, "Trade hack detected, the trade has been cancelled.");
			client->ResetTrade();
			return;
		}

		if (RuleI(Bots, StackSizeMin) != -1) {
			if (
				trade_instance->IsStackable() &&
				trade_instance->GetCharges() < RuleI(Bots, StackSizeMin)
			) { // temp until partial stacks are implemented
				if (trade_event_exists) {
					event_trade.push_back(ClientTrade(trade_instance, trade_index));
					continue;
				}

				client->Message(
					Chat::Yellow,
					fmt::format(
						"{} is too small of a stack, you need atleast {}, the trade has been cancelled!",
						item_link,
						RuleI(Bots, StackSizeMin)
					).c_str()
				);
				client->ResetTrade();

				return;

			}
		}
		else if (
			trade_instance->IsStackable() &&
			trade_instance->GetCharges() < trade_instance->GetItem()->StackSize
		) {
			client->Message(
				Chat::Yellow,
				fmt::format(
					"{} is only a partially stacked item, the trade has been cancelled!",
					item_link
				).c_str()
			);
			client->ResetTrade();
			return;
		}

		for (int m = EQ::invaug::SOCKET_BEGIN; m <= EQ::invaug::SOCKET_END; ++m) {
			const auto augment = trade_instance->GetAugment(m);
			if (!augment) {
				continue;
			}

			if (!CheckLoreConflict(augment->GetItem())) {
				continue;
			}

			linker.SetLinkType(EQ::saylink::SayLinkItemInst);
			linker.SetItemInst(augment);

			item_link = linker.GenerateLink();

			client->Message(
					Chat::Yellow,
					fmt::format(
							"{} already has {}, the trade has been cancelled!",
							GetCleanName(),
							item_link
					).c_str()
			);
			client->ResetTrade();
			return;
		}

		if (CheckLoreConflict(trade_instance->GetItem())) {
			if (trade_event_exists) {
				event_trade.push_back(ClientTrade(trade_instance, trade_index));
				continue;
			}
			else {
				client->Message(
					Chat::Yellow,
					fmt::format(
						"This bot already has {}, the trade has been cancelled!",
						item_link
					).c_str()
				);
				client->ResetTrade();
				return;
			}
		}

		if (!trade_instance->IsType(item::ItemClassCommon)) {
			if (trade_event_exists) {
				event_trade.push_back(ClientTrade(trade_instance, trade_index));
				continue;
			}
			else {
				client->ResetTrade();
				return;
			}
		}

		if (
			(
				!trade_instance->IsClassEquipable(GetClass()) &&
				!RuleB(Bots, AllowBotEquipAnyClassGear)
			) ||
			GetLevel() < trade_instance->GetItem()->ReqLevel ||
			(
				!trade_instance->IsRaceEquipable(GetBaseRace()) &&
				!RuleB(Bots, AllowBotEquipAnyRaceGear)
			)
		) {
			if (trade_event_exists) {
				event_trade.push_back(ClientTrade(trade_instance, trade_index));
				continue;
			}
			else {
				client->ResetTrade();
				return;
			}
		}

		client_trade.push_back(ClientTrade(trade_instance, trade_index));
	}

	// check for incoming lore hacks
	for (auto& trade_iterator : client_trade) {
		auto trade_instance = trade_iterator.trade_item_instance;
		auto trade_index = trade_iterator.from_client_slot;
		if (!trade_instance->GetItem()->LoreFlag) {
			continue;
		}

		for (const auto& check_iterator : client_trade) {
			if (check_iterator.from_client_slot == trade_iterator.from_client_slot) {
				continue;
			}

			if (!check_iterator.trade_item_instance->GetItem()->LoreFlag) {
				continue;
			}

			if (trade_instance->GetItem()->LoreGroup == -1 && check_iterator.trade_item_instance->GetItem()->ID == trade_instance->GetItem()->ID) {
				LogError("trade hack detected by [{}] with [{}].", client->GetCleanName(), GetCleanName());
				client->Message(Chat::White, "Trade hack detected, the trade has been cancelled.");
				client->ResetTrade();
				return;
			}

			if ((trade_instance->GetItem()->LoreGroup > 0) && (check_iterator.trade_item_instance->GetItem()->LoreGroup == trade_instance->GetItem()->LoreGroup)) {
				LogError("trade hack detected by [{}] with [{}].", client->GetCleanName(), GetCleanName());
				client->Message(Chat::White, "Trade hack detected, the trade has been cancelled.");
				client->ResetTrade();
				return;
			}
		}
	}

	// find equipment slots
	const bool can_dual_wield = (GetSkill(EQ::skills::SkillDualWield) > 0);
	bool melee_2h_weapon = false;
	bool melee_secondary = false;

	//for (unsigned stage_loop = stageStackable; stage_loop <= stageReplaceable; ++stage_loop) { // awaiting implementation
	for (unsigned stage_loop = stageEmpty; stage_loop <= stageReplaceable; ++stage_loop) {
		for (auto& trade_iterator : client_trade) {
			if (trade_iterator.to_bot_slot != invslot::SLOT_INVALID) {
				continue;
			}

			auto trade_instance = trade_iterator.trade_item_instance;
			//if ((stage_loop == stageStackable) && !trade_instance->IsStackable())
			//	continue;

			for (auto index : bot_equip_order) {
				if (!(trade_instance->GetItem()->Slots & (1 << index))) {
					continue;
				}

				if (trade_instance->GetItem()->ItemType == EQ::item::ItemTypeAugmentation) {
					continue;
				}

				//if (stage_loop == stageStackable) {
				//	// TODO: implement
				//	continue;
				//}

				if (stage_loop != stageReplaceable) {
					if (m_inv[index]) {
						continue;
					}
				}

				bool slot_taken = false;
				for (const auto& check_iterator : client_trade) {
					if (check_iterator.from_client_slot == trade_iterator.from_client_slot) {
						continue;
					}

					if (check_iterator.to_bot_slot == index) {
						slot_taken = true;
						break;
					}
				}

				if (slot_taken) {
					continue;
				}

				if (index == invslot::slotPrimary) {
					if (trade_instance->GetItem()->IsType2HWeapon()) {
						if (!melee_secondary) {
							melee_2h_weapon = true;
							auto equipped_secondary_weapon = GetBotItem(invslot::slotSecondary);
							if (equipped_secondary_weapon) {
								client_return.push_back(ClientReturn(equipped_secondary_weapon, invslot::slotSecondary));
							}
						} else {
							continue;
						}
					}
				} else if (index == invslot::slotSecondary) {
					if (!melee_2h_weapon) {
						if (
							(can_dual_wield && trade_instance->GetItem()->IsType1HWeapon()) ||
							trade_instance->GetItem()->IsTypeShield() ||
							!trade_instance->IsWeapon()
						) {
							melee_secondary = true;
							auto equipped_primary_weapon = GetBotItem(invslot::slotPrimary);
							if (equipped_primary_weapon && equipped_primary_weapon->GetItem()->IsType2HWeapon()) {
								client_return.push_back(ClientReturn(equipped_primary_weapon, invslot::slotPrimary));
							}
						} else {
							continue;
						}
					} else {
						continue;
					}
				}

				trade_iterator.to_bot_slot = index;

				if (m_inv[index]) {
					client_return.push_back(ClientReturn(GetBotItem(index), index));
				}

				break;
			}
		}
	}

	// move unassignable items from trade list to event list
	for (std::list<ClientTrade>::iterator trade_iterator = client_trade.begin(); trade_iterator != client_trade.end();) {
		if (trade_iterator->to_bot_slot == invslot::SLOT_INVALID) {
			if (trade_event_exists) {
				event_trade.push_back(ClientTrade(trade_iterator->trade_item_instance, trade_iterator->from_client_slot));
				trade_iterator = client_trade.erase(trade_iterator);
				continue;
			}
			else {
				client_return.push_back(ClientReturn(trade_iterator->trade_item_instance, trade_iterator->from_client_slot));
				trade_iterator = client_trade.erase(trade_iterator);
				continue;
			}
		}
		++trade_iterator;
	}

	// out-going return checks for client
	for (auto& return_iterator : client_return) {
		auto return_instance = return_iterator.return_item_instance;
		if (!return_instance) {
			continue;
		}

		if (!return_instance->GetItem()) {
			LogError("error processing bot slot [{}] for [{}] in trade with [{}].", return_iterator.from_bot_slot, GetCleanName(), client->GetCleanName());
			client->Message(
				Chat::White,
				fmt::format(
					"A server error was encountered while processing bot slot {}, the trade has been cancelled.",
					return_iterator.from_bot_slot
				).c_str()
			);
			client->ResetTrade();
			return;
		}

		EQ::SayLinkEngine linker;
		linker.SetLinkType(EQ::saylink::SayLinkItemInst);
		linker.SetItemInst(return_instance);

		auto item_link = linker.GenerateLink();

		// non-failing checks above are causing this to trigger (i.e., !ItemClassCommon and !IsEquipable{race, class, min_level})
		// this process is hindered by not having bots use the inventory trade method (TODO: implement bot inventory use)
		if (client->CheckLoreConflict(return_instance->GetItem())) {
			client->Message(
				Chat::Yellow,
				fmt::format(
					"You already have {}, the trade has been cancelled!",
					item_link
				).c_str()
			);
			client->ResetTrade();
			return;
		}

		if (return_iterator.from_bot_slot == invslot::slotCursor) {
			return_iterator.to_client_slot = invslot::slotCursor;
		} else {
			int16 client_search_general = invslot::GENERAL_BEGIN;
			uint8 client_search_bag = invbag::SLOT_BEGIN;
			bool run_search = true;
			while (run_search) {
				int16 client_test_slot = client->GetInv().FindFreeSlotForTradeItem(return_instance, client_search_general, client_search_bag);
				if (client_test_slot == invslot::SLOT_INVALID) {
					run_search = false;
					continue;
				}

				bool slot_taken = false;
				for (const auto& check_iterator : client_return) {
					if (check_iterator.from_bot_slot == return_iterator.from_bot_slot) {
						continue;
					}

					if (check_iterator.to_client_slot == client_test_slot && client_test_slot != invslot::slotCursor) {
						slot_taken = true;
						break;
					}
				}

				if (slot_taken) {
					if (client_test_slot >= invslot::GENERAL_BEGIN && client_test_slot <= invslot::GENERAL_END) {
						++client_search_general;
						client_search_bag = invbag::SLOT_BEGIN;
					} else {
						client_search_general = InventoryProfile::CalcSlotId(client_test_slot);
						client_search_bag = InventoryProfile::CalcBagIdx(client_test_slot);
						++client_search_bag;
						if (client_search_bag >= invbag::SLOT_COUNT) {
							// incrementing this past legacy::GENERAL_END triggers the (client_test_slot == legacy::SLOT_INVALID) at the beginning of the search loop
							// ideally, this will never occur because we always start fresh with each loop iteration and should receive SLOT_CURSOR as a return value
							++client_search_general;
							client_search_bag = invbag::SLOT_BEGIN;
						}
					}

					continue;
				}

				return_iterator.to_client_slot = client_test_slot;
				run_search = false;
			}
		}

		if (return_iterator.to_client_slot == invslot::SLOT_INVALID) {
			client->Message(Chat::Yellow, "You do not have room to complete this trade, the trade has been cancelled!");
			client->ResetTrade();
			return;
		}
	}

	// perform actual trades
	// returns first since clients have trade slots and bots do not
	for (auto& return_iterator : client_return) {
		// TODO: code for stackables

		if (return_iterator.from_bot_slot == invslot::slotCursor) { // failed trade return
			// no movement action required
		} else if (return_iterator.from_bot_slot >= invslot::TRADE_BEGIN && return_iterator.from_bot_slot <= invslot::TRADE_END) { // failed trade returns
			client->PutItemInInventory(return_iterator.to_client_slot, *return_iterator.return_item_instance);
			client->SendItemPacket(return_iterator.to_client_slot, return_iterator.return_item_instance, ItemPacketTrade);
			client->DeleteItemInInventory(return_iterator.from_bot_slot);
		} else { // successful trade returns
			auto return_instance = m_inv.PopItem(return_iterator.from_bot_slot);

			//	// TODO: add logging

			if (!database.botdb.DeleteItemBySlot(GetBotID(), return_iterator.from_bot_slot)) {
				OwnerMessage(
					fmt::format(
						"Failed to delete item by slot from slot {}.",
						return_iterator.from_bot_slot
					).c_str()
				);
			}

			BotRemoveEquipItem(return_iterator.from_bot_slot);

			if (parse->BotHasQuestSub(EVENT_UNEQUIP_ITEM_BOT)) {
				const auto& export_string = fmt::format(
					"{} {}",
					return_iterator.return_item_instance->IsStackable() ? return_iterator.return_item_instance->GetCharges() : 1,
					return_iterator.from_bot_slot
				);

				std::vector<std::any> args = { return_iterator.return_item_instance };

				parse->EventBot(EVENT_UNEQUIP_ITEM_BOT, this, nullptr, export_string, return_iterator.return_item_instance->GetID(), &args);
			}

			if (return_instance) {
				EQ::SayLinkEngine linker;
				linker.SetLinkType(EQ::saylink::SayLinkItemInst);
				linker.SetItemInst(return_instance);
				auto item_link = linker.GenerateLink();

				OwnerMessage(
					fmt::format(
						"I have returned {}.",
						item_link
					)
				);

				client->PutItemInInventory(return_iterator.to_client_slot, *return_instance, true);
			}

			InventoryProfile::MarkDirty(return_instance);
		}
		return_iterator.return_item_instance = nullptr;
	}

	// trades can now go in as empty slot inserts
	for (auto& trade_iterator : client_trade) {
		// TODO: code for stackables

		if (!trade_iterator.trade_item_instance) {
			continue;
		}

		if (!database.botdb.SaveItemBySlot(this, trade_iterator.to_bot_slot, trade_iterator.trade_item_instance)) {
			OwnerMessage(
				fmt::format(
					"Failed to save item by slot to slot {}.",
					trade_iterator.to_bot_slot
				).c_str()
			);
		}

		EQ::SayLinkEngine linker;
		linker.SetLinkType(EQ::saylink::SayLinkItemInst);
		linker.SetItemInst(trade_iterator.trade_item_instance);
		auto item_link = linker.GenerateLink();

		OwnerMessage(
			fmt::format(
				"I have accepted {}.",
				item_link
			)
		);

		m_inv.PutItem(trade_iterator.to_bot_slot, *trade_iterator.trade_item_instance);
		BotAddEquipItem(trade_iterator.to_bot_slot, (trade_iterator.trade_item_instance ? trade_iterator.trade_item_instance->GetID() : 0));

		if (parse->BotHasQuestSub(EVENT_EQUIP_ITEM_BOT)) {
			const auto& export_string = fmt::format(
				"{} {}",
				trade_iterator.trade_item_instance->IsStackable() ? trade_iterator.trade_item_instance->GetCharges() : 1,
				trade_iterator.to_bot_slot
			);

			std::vector<std::any> args = { trade_iterator.trade_item_instance };

			parse->EventBot(EVENT_EQUIP_ITEM_BOT, this, nullptr, export_string, trade_iterator.trade_item_instance->GetID(), &args);
		}

		trade_iterator.trade_item_instance = nullptr; // actual deletion occurs in client delete below

		client->DeleteItemInInventory(trade_iterator.from_client_slot, 0, (trade_iterator.from_client_slot == EQ::invslot::slotCursor));

		// database currently has unattuned item saved in inventory..it will be attuned on next bot load
		// this prevents unattuned item returns in the mean time (TODO: re-work process)
		if (trade_iterator.to_bot_slot >= invslot::EQUIPMENT_BEGIN && trade_iterator.to_bot_slot <= invslot::EQUIPMENT_END) {
			auto attune_item = m_inv.GetItem(trade_iterator.to_bot_slot);
			if (attune_item && attune_item->GetItem()->Attuneable) {
				attune_item->SetAttuned(true);
			}
		}
	}

	size_t accepted_count = client_trade.size();
	size_t returned_count = client_return.size();

	if (accepted_count) {
		CalcBotStats(client->GetBotOption(Client::booStatsUpdate));
	}

	if (event_trade.size()) {
		// Get Traded Items

		// Accept Items from Cursor to support bot command ^inventorygive
		if (begin_slot_id == invslot::slotCursor && end_slot_id == invslot::slotCursor) {
			EQ::ItemInstance* insts[1] = { 0 };
			user_inv = client->GetInv();
			insts[0] = user_inv.GetItem(invslot::slotCursor);
			client->DeleteItemInInventory(invslot::slotCursor);

			// copy to be filtered by task updates, null trade slots preserved for quest event arg
			std::vector<EQ::ItemInstance*> items(insts, insts + std::size(insts));

			// Check if EVENT_TRADE accepts any items
			if (parse->BotHasQuestSub(EVENT_TRADE)) {
				std::vector<std::any> item_list(items.begin(), items.end());
				parse->EventBot(EVENT_TRADE, this, client, "", 0, &item_list);
			}

			CalcBotStats(false);

		} else {
			EQ::ItemInstance* insts[8] = { 0 };
			user_inv = client->GetInv();
			for (int i = EQ::invslot::TRADE_BEGIN; i <= EQ::invslot::TRADE_END; ++i) {
				insts[i - EQ::invslot::TRADE_BEGIN] = user_inv.GetItem(i);
				client->DeleteItemInInventory(i);
			}

			// copy to be filtered by task updates, null trade slots preserved for quest event arg
			std::vector<EQ::ItemInstance*> items(insts, insts + std::size(insts));

			// Check if EVENT_TRADE accepts any items
			if (parse->BotHasQuestSub(EVENT_TRADE)) {
				std::vector<std::any> item_list(items.begin(), items.end());
				parse->EventBot(EVENT_TRADE, this, client, "", 0, &item_list);
			}

			CalcBotStats(false);
		}
	}
}

bool Bot::Death(Mob *killer_mob, int64 damage, uint16 spell_id, EQ::skills::SkillType attack_skill, KilledByTypes killed_by, bool is_buff_tic)
{
	if (!NPC::Death(killer_mob, damage, spell_id, attack_skill)) {
		return false;
	}

	Mob *my_owner = GetBotOwner();

	if (my_owner && my_owner->IsClient() && my_owner->CastToClient()->GetBotOption(Client::booDeathMarquee)) {
		if (killer_mob) {
			my_owner->CastToClient()->SendMarqueeMessage(Chat::White, 510, 0, 1000, 3000, StringFormat("%s has been slain by %s", GetCleanName(), killer_mob->GetCleanName()));
		} else {
			my_owner->CastToClient()->SendMarqueeMessage(Chat::White, 510, 0, 1000, 3000, StringFormat("%s has been slain", GetCleanName()));
		}
	}

	const auto c = entity_list.GetCorpseByID(GetID());

	if (c) {
		c->Depop();
	}

	LeaveHealRotationMemberPool();

	if ((PULLING_BOT || RETURNING_BOT) && my_owner && my_owner->IsClient()) {
		my_owner->CastToClient()->SetBotPulling(false);
	}

	if (parse->BotHasQuestSub(EVENT_DEATH_COMPLETE)) {
		const auto& export_string = fmt::format(
			"{} {} {} {}",
			killer_mob ? killer_mob->GetID() : 0,
			damage,
			spell_id,
			static_cast<int>(attack_skill)
		);

		parse->EventBot(EVENT_DEATH_COMPLETE, this, killer_mob, export_string, 0);
	}

	Zone();
	entity_list.RemoveBot(GetID());

	return true;
}

void Bot::Damage(Mob *from, int64 damage, uint16 spell_id, EQ::skills::SkillType attack_skill, bool avoidable, int8 buffslot, bool iBuffTic, eSpecialAttacks special) {
	if (!from) {
		return;
	}

	if (spell_id == 0) {
		spell_id = SPELL_UNKNOWN;
	}

	//handle EVENT_ATTACK. Resets after we have not been attacked for 12 seconds
	if (attacked_timer.Check()) {
		if (parse->BotHasQuestSub(EVENT_ATTACK)) {
			LogCombat("Triggering EVENT_ATTACK due to attack by [{}]", from->GetName());

			parse->EventBot(EVENT_ATTACK, this, from, "", 0);
		}
	}

	attacked_timer.Start(CombatEventTimer_expire);
	// if spell is lifetap add hp to the caster
	if (IsValidSpell(spell_id) && IsLifetapSpell(spell_id)) {
		int64 healed = GetActSpellHealing(spell_id, damage);
		LogCombatDetail("Applying lifetap heal of [{}] to [{}]", healed, GetCleanName());
		HealDamage(healed);
		if (from) {
			entity_list.FilteredMessageClose(this, true, RuleI(Range, SpellMessages), Chat::Emote, FilterSocials, "%s beams a smile at %s", GetCleanName(), from->GetCleanName());
		}
	}

	CommonDamage(from, damage, spell_id, attack_skill, avoidable, buffslot, iBuffTic, special);
	if (GetHP() < 0) {
		if (IsCasting())
			InterruptSpell();
		SetAppearance(eaDead);
	}

	SendHPUpdate();
	if (this == from)
		return;

	// Aggro the bot's group members
	if (IsGrouped()) {
		Group *g = GetGroup();
		if (g) {
			for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
				if (g->members[i] && g->members[i]->IsBot() && from && !g->members[i]->CheckAggro(from) && g->members[i]->IsAttackAllowed(from))
					g->members[i]->AddToHateList(from, 1);
			}
		}
	}
}

//proc chance includes proc bonus
float Bot::GetProcChances(float ProcBonus, uint16 hand) {
	int mydex = GetDEX();
	float ProcChance = 0.0f;
	uint32 weapon_speed = 0;
	switch (hand) {
		case EQ::invslot::slotPrimary:
			weapon_speed = attack_timer.GetDuration();
			break;
		case EQ::invslot::slotSecondary:
			weapon_speed = attack_dw_timer.GetDuration();
			break;
		case EQ::invslot::slotRange:
			weapon_speed = ranged_timer.GetDuration();
			break;
	}

	if (weapon_speed < RuleI(Combat, MinHastedDelay))
		weapon_speed = RuleI(Combat, MinHastedDelay);

	if (RuleB(Combat, AdjustProcPerMinute)) {
		ProcChance = (static_cast<float>(weapon_speed) * RuleR(Combat, AvgProcsPerMinute) / 60000.0f);
		ProcBonus += static_cast<float>(mydex) * RuleR(Combat, ProcPerMinDexContrib);
		ProcChance += (ProcChance * ProcBonus / 100.0f);
	} else {
		ProcChance = (RuleR(Combat, BaseProcChance) + static_cast<float>(mydex) / RuleR(Combat, ProcDexDivideBy));
		ProcChance += (ProcChance * ProcBonus / 100.0f);
	}

	LogCombat("Proc chance [{}] ([{}] from bonuses)", ProcChance, ProcBonus);
	return ProcChance;
}

int Bot::GetHandToHandDamage(void) {
	if (RuleB(Combat, UseRevampHandToHand)) {
		// everyone uses this in the revamp!
		int skill = GetSkill(EQ::skills::SkillHandtoHand);
		int epic = 0;
		if (CastToNPC()->GetEquippedItemFromTextureSlot(EQ::textures::armorHands) == 10652 && GetLevel() > 46)
			epic = 280;
		if (epic > skill)
			skill = epic;
		return skill / 15 + 3;
	}

	static uint8 mnk_dmg[] = {99,
				4, 4, 4, 4, 5, 5, 5, 5, 5, 6,		   // 1-10
				6, 6, 6, 6, 7, 7, 7, 7, 7, 8,		   // 11-20
				8, 8, 8, 8, 9, 9, 9, 9, 9, 10,		  // 21-30
				10, 10, 10, 10, 11, 11, 11, 11, 11, 12, // 31-40
				12, 12, 12, 12, 13, 13, 13, 13, 13, 14, // 41-50
				14, 14, 14, 14, 14, 14, 14, 14, 14, 14, // 51-60
				14, 14};								// 61-62
	static uint8 bst_dmg[] = {99,
				4, 4, 4, 4, 4, 5, 5, 5, 5, 5,		// 1-10
				5, 6, 6, 6, 6, 6, 6, 7, 7, 7,		// 11-20
				7, 7, 7, 8, 8, 8, 8, 8, 8, 9,		// 21-30
				9, 9, 9, 9, 9, 10, 10, 10, 10, 10,   // 31-40
				10, 11, 11, 11, 11, 11, 11, 12, 12}; // 41-49
	if (GetClass() == Class::Monk) {
		if (CastToNPC()->GetEquippedItemFromTextureSlot(EQ::textures::armorHands) == 10652 && GetLevel() > 50)
			return 9;
		if (level > 62)
			return 15;
		return mnk_dmg[level];
	} else if (GetClass() == Class::Beastlord) {
		if (level > 49)
			return 13;
		return bst_dmg[level];
	}
	return 2;
}

bool Bot::TryFinishingBlow(Mob *defender, int64 &damage)
{
	if (!defender)
		return false;

	if (aabonuses.FinishingBlow[1] && !defender->IsClient() && defender->GetHPRatio() < 10) {
		int chance = aabonuses.FinishingBlow[0];
		int fb_damage = aabonuses.FinishingBlow[1];
		int levelreq = aabonuses.FinishingBlowLvl[0];
		if (defender->GetLevel() <= levelreq && (chance >= zone->random.Int(1, 1000))) {
			LogCombat("Landed a finishing blow: levelreq at [{}] other level [{}]",
				levelreq, defender->GetLevel());
			entity_list.MessageCloseString(this, false, 200, Chat::MeleeCrit, FINISHING_BLOW, GetName());
			damage = fb_damage;
			return true;
		} else {
			LogCombat("failed a finishing blow: levelreq at [{}] other level [{}]",
				levelreq, defender->GetLevel());
			return false;
		}
	}
	return false;
}

void Bot::DoRiposte(Mob* defender) {
	LogCombatDetail("Preforming a riposte");
	if (!defender)
		return;

	defender->Attack(this, EQ::invslot::slotPrimary, true);
	int32 DoubleRipChance = (defender->GetAABonuses().GiveDoubleRiposte[0] + defender->GetSpellBonuses().GiveDoubleRiposte[0] + defender->GetItemBonuses().GiveDoubleRiposte[0]);
	if (DoubleRipChance && (DoubleRipChance >= zone->random.Int(0, 100))) {
		LogCombatDetail("Preforming a double riposte ([{}] percent chance)", DoubleRipChance);
		defender->Attack(this, EQ::invslot::slotPrimary, true);
	}

	DoubleRipChance = defender->GetAABonuses().GiveDoubleRiposte[1];
	if (DoubleRipChance && (DoubleRipChance >= zone->random.Int(0, 100))) {
		if (defender->GetClass() == Class::Monk)
			defender->MonkSpecialAttack(this, defender->GetAABonuses().GiveDoubleRiposte[2]);
		else if (defender->IsBot())
			defender->CastToClient()->DoClassAttacks(this,defender->GetAABonuses().GiveDoubleRiposte[2], true);
	}
}

int Bot::GetBaseSkillDamage(EQ::skills::SkillType skill, Mob *target)
{
	int base = EQ::skills::GetBaseDamage(skill);
	auto skill_level = GetSkill(skill);
	switch (skill) {
		case EQ::skills::SkillDragonPunch:
		case EQ::skills::SkillEagleStrike:
		case EQ::skills::SkillTigerClaw:
			if (skill_level >= 25)
				base++;
			if (skill_level >= 75)
				base++;
			if (skill_level >= 125)
				base++;
			if (skill_level >= 175)
				base++;
			return base;
		case EQ::skills::SkillFrenzy:
			if (GetBotItem(EQ::invslot::slotPrimary)) {
				if (GetLevel() > 15)
					base += GetLevel() - 15;
				if (base > 23)
					base = 23;
				if (GetLevel() > 50)
					base += 2;
				if (GetLevel() > 54)
					base++;
				if (GetLevel() > 59)
					base++;
			}
			return base;
		case EQ::skills::SkillFlyingKick: {
			float skill_bonus = skill_level / 9.0f;
			float ac_bonus = 0.0f;
			auto inst = GetBotItem(EQ::invslot::slotFeet);
			if (inst)
				ac_bonus = inst->GetItemArmorClass(true) / 25.0f;
			if (ac_bonus > skill_bonus)
				ac_bonus = skill_bonus;
			return static_cast<int>(ac_bonus + skill_bonus);
		}
		case EQ::skills::SkillKick: {
			float skill_bonus = skill_level / 10.0f;
			float ac_bonus = 0.0f;
			auto inst = GetBotItem(EQ::invslot::slotFeet);
			if (inst)
				ac_bonus = inst->GetItemArmorClass(true) / 25.0f;
			if (ac_bonus > skill_bonus)
				ac_bonus = skill_bonus;
			return static_cast<int>(ac_bonus + skill_bonus);
		}
		case EQ::skills::SkillBash: {
			float skill_bonus = skill_level / 10.0f;
			float ac_bonus = 0.0f;
			const EQ::ItemInstance *inst = nullptr;
			if (HasShieldEquipped())
				inst = GetBotItem(EQ::invslot::slotSecondary);
			else if (HasTwoHanderEquipped())
				inst = GetBotItem(EQ::invslot::slotPrimary);
			if (inst)
				ac_bonus = inst->GetItemArmorClass(true) / 25.0f;
			if (ac_bonus > skill_bonus)
				ac_bonus = skill_bonus;
			return static_cast<int>(ac_bonus + skill_bonus);
		}
		case EQ::skills::SkillBackstab: {
			float skill_bonus = static_cast<float>(skill_level) * 0.02f;
			auto inst = GetBotItem(EQ::invslot::slotPrimary);
			if (inst && inst->GetItem() && inst->GetItem()->ItemType == EQ::item::ItemType1HPiercing) {
				base = inst->GetItemBackstabDamage(true);
				if (!inst->GetItemBackstabDamage())
					base += inst->GetItemWeaponDamage(true);
				if (target) {
					if (
						inst->GetItemElementalFlag(true) &&
						inst->GetItemElementalDamage(true)
					) {
						base += target->ResistElementalWeaponDmg(inst);
					}

					if (
						inst->GetItemBaneDamageBody(true) ||
						inst->GetItemBaneDamageRace(true)
					) {
						base += target->CheckBaneDamage(inst);
					}
				}
			}
			return static_cast<int>(static_cast<float>(base) * (skill_bonus + 2.0f));
		}
		default:
			return 0;
	}
}

void Bot::DoSpecialAttackDamage(Mob *who, EQ::skills::SkillType skill, int32 max_damage, int32 min_damage, int32 hate_override, int ReuseTime, bool HitChance) {
	int32 hate = max_damage;
	if (hate_override > -1)
		hate = hate_override;

	if (skill == EQ::skills::SkillBash) {
		const EQ::ItemInstance* inst = GetBotItem(EQ::invslot::slotSecondary);
		const EQ::ItemData* botweapon = nullptr;
		if (inst)
			botweapon = inst->GetItem();

		if (botweapon) {
			if (botweapon->ItemType == EQ::item::ItemTypeShield)
				hate += botweapon->AC;

			hate = (hate * (100 + GetSpellFuriousBash(botweapon->Focus.Effect)) / 100);
		}
	}

	DamageHitInfo my_hit;
	my_hit.base_damage = max_damage;
	my_hit.min_damage = min_damage;
	my_hit.damage_done = 1;

	my_hit.skill = skill;
	my_hit.offense = offense(my_hit.skill);
	my_hit.tohit = GetTotalToHit(my_hit.skill, 0);
	my_hit.hand = EQ::invslot::slotPrimary;

	if (skill == EQ::skills::SkillThrowing || skill == EQ::skills::SkillArchery)
		my_hit.hand = EQ::invslot::slotRange;

	DoAttack(who, my_hit);

	who->AddToHateList(this, hate);

	who->Damage(this, my_hit.damage_done, SPELL_UNKNOWN, skill, false);

	if (!GetTarget() || HasDied())
		return;

	if (my_hit.damage_done > 0)
		CheckNumHitsRemaining(NumHit::OutgoingHitSuccess);

	if (HasSkillProcs())
		TrySkillProc(who, skill, (ReuseTime * 1000));

	if (my_hit.damage_done > 0 && HasSkillProcSuccess())
		TrySkillProc(who, skill, (ReuseTime * 1000), true);
}

void Bot::DoClassAttacks(Mob *target, bool IsRiposte) {
	if (
		!target ||
		GetAppearance() == eaDead ||
		spellend_timer.Enabled() ||
		IsFeared() ||
		IsStunned() ||
		IsMezzed() ||
		DivineAura() ||
		GetHP() < 0 ||
		!IsAttackAllowed(target)
	) {
		return;
	}

	bool taunt_time = taunt_timer.Check();
	bool ca_time = classattack_timer.Check(false);
	bool ma_time = monkattack_timer.Check(false);

	if (taunt_time) {

		// Bots without this skill shouldn't be 'checking' on this timer..let's just disable it and avoid the extra IsAttackAllowed() checks
		// Note: this is done here instead of NPC::ctor() because taunt skill can be acquired during level ups (the timer is re-enabled in CalcBotStats())
		if (!GetSkill(EQ::skills::SkillTaunt)) {
			taunt_timer.Disable();
			return;
		}

		if (!IsAttackAllowed(target)) {
			return;
		}
	}

	if ((ca_time || ma_time) && !IsAttackAllowed(target)) {
		return;
	}

	if (IsTaunting() && target->IsNPC() && taunt_time) {
		if (GetTarget() && GetTarget()->GetHateTop() && GetTarget()->GetHateTop() != this) {
			RaidGroupSay(
				fmt::format(
					"Taunting {}.",
					target->GetCleanName()
				).c_str()
			);
			Taunt(target->CastToNPC(), false);
			taunt_timer.Start(TauntReuseTime * 1000);
		}
	}

	if (ma_time) {
		switch (GetClass()) {
			case Class::Monk: {
				int reuse = (MonkSpecialAttack(target, EQ::skills::SkillTigerClaw) - 1);

				// Live AA - Technique of Master Wu
				int wuchance = itembonuses.DoubleSpecialAttack + spellbonuses.DoubleSpecialAttack + aabonuses.DoubleSpecialAttack;

				if (wuchance) {
					const int MonkSPA[5] = {
						EQ::skills::SkillFlyingKick,
						EQ::skills::SkillDragonPunch,
						EQ::skills::SkillEagleStrike,
						EQ::skills::SkillTigerClaw,
						EQ::skills::SkillRoundKick
					};
					int extra = 0;
					// always 1/4 of the double attack chance, 25% at rank 5 (100/4)
					while (wuchance > 0) {
						if (zone->random.Roll(wuchance)) {
							++extra;
						}
						else {
							break;
						}
						wuchance /= 4;
					}

					Mob* bo = GetBotOwner();
					if (bo && bo->IsClient() && bo->CastToClient()->GetBotOption(Client::booMonkWuMessage)) {

						bo->Message(
							GENERIC_EMOTE,
							"The spirit of Master Wu fills %s!  %s gains %d additional attack(s).",
							GetCleanName(),
							GetCleanName(),
							extra
						);
					}

					auto classic = RuleB(Combat, ClassicMasterWu);
					while (extra && TargetValidation(GetTarget())) {
						MonkSpecialAttack(GetTarget(), (classic ? MonkSPA[zone->random.Int(0, 4)] : EQ::skills::SkillTigerClaw));
						--extra;
					}
				}

				float HasteModifier = (GetHaste() * 0.01f);
				monkattack_timer.Start((reuse * 1000) / HasteModifier);

				break;
			}
			default:
				break;
		}
	}

	if (!ca_time) {
		return;
	}

	float HasteModifier = (GetHaste() * 0.01f);
	uint16 skill_to_use = -1;
	int bot_level = GetLevel();
	int reuse = (TauntReuseTime * 1000); // Same as Bash and Kick
	bool did_attack = false;

	switch (GetClass()) {
		case Class::Warrior:
		case Class::Cleric:
		case Class::ShadowKnight:
		case Class::Paladin:
			{
				bool is_large_race = (
					GetBaseRace() == Race::Ogre ||
					GetBaseRace() == Race::Troll ||
					GetBaseRace() == Race::Barbarian
					);
				bool has_bash_skill = GetSkill(EQ::skills::SkillBash) > 0;
				bool has_shield_in_secondary =
					m_inv.GetItem(EQ::invslot::slotSecondary) &&
					m_inv.GetItem(EQ::invslot::slotSecondary)->GetItem()->ItemType == EQ::item::ItemTypeShield;
				bool has_two_hander_with_aa =
					m_inv.GetItem(EQ::invslot::slotPrimary) &&
					m_inv.GetItem(EQ::invslot::slotPrimary)->GetItem()->IsType2HWeapon() &&
					GetAA(aa2HandBash) >= 1;
				bool can_bash =
					is_large_race ||
					(
						has_bash_skill &&
						(
							has_shield_in_secondary || has_two_hander_with_aa
							)
						);

				if (can_bash) {
					skill_to_use = EQ::skills::SkillBash;
				}

				break;
			}
		case Class::Ranger:
		case Class::Beastlord:
			if (GetSkill(EQ::skills::SkillKick)) {
				skill_to_use = EQ::skills::SkillKick;
			}

			break;
		case Class::Berserker:
			if (GetSkill(EQ::skills::SkillFrenzy)) {
				skill_to_use = EQ::skills::SkillFrenzy;
			}

			break;
		case Class::Monk:
			if (GetSkill(EQ::skills::SkillFlyingKick)) {
				skill_to_use = EQ::skills::SkillFlyingKick;
			}
			else if (GetSkill(EQ::skills::SkillDragonPunch)) {
				skill_to_use = EQ::skills::SkillDragonPunch;
			}
			else if (GetSkill(EQ::skills::SkillEagleStrike)) {
				skill_to_use = EQ::skills::SkillEagleStrike;
			}
			else if (GetSkill(EQ::skills::SkillRoundKick)) {
				skill_to_use = EQ::skills::SkillRoundKick;
			}
			else if (GetSkill(EQ::skills::SkillKick)) {
				skill_to_use = EQ::skills::SkillKick;
			}

			break;
		case Class::Rogue:
			if (GetSkill(EQ::skills::SkillBackstab)) {
				skill_to_use = EQ::skills::SkillBackstab;
			}

			break;
	}

	int64 dmg = GetBaseSkillDamage(static_cast<EQ::skills::SkillType>(skill_to_use), GetTarget());

	if (skill_to_use == EQ::skills::SkillBash) {
		if (target != this) {
			DoAnim(animTailRake);
			if (GetWeaponDamage(target, GetBotItem(EQ::invslot::slotSecondary)) <= 0 && GetWeaponDamage(target, GetBotItem(EQ::invslot::slotShoulders)) <= 0)
				dmg = DMG_INVULNERABLE;

			DoSpecialAttackDamage(target, EQ::skills::SkillBash, dmg, 0, -1, reuse);
			did_attack = true;
		}
	}

	if (skill_to_use == EQ::skills::SkillFrenzy) {
		int AtkRounds = 1;
		float HasteMod = (FrenzyReuseTime - 1) / (GetHaste() * 0.01f);
		reuse = (FrenzyReuseTime * 1000);
		DoAnim(anim2HSlashing);

		// bards can do riposte frenzy for some reason
		if (!IsRiposte && GetClass() == Class::Berserker) {
			int chance = GetLevel() * 2 + GetSkill(EQ::skills::SkillFrenzy);

			if (zone->random.Roll0(450) < chance) {
				AtkRounds++;
			}

			if (zone->random.Roll0(450) < chance) {
				AtkRounds++;
			}
		}

		while (AtkRounds > 0) {
			if (GetTarget() != this && TargetValidation(GetTarget())) {
				DoSpecialAttackDamage(GetTarget(), EQ::skills::SkillFrenzy, dmg, 0, dmg, HasteMod);
			}

			AtkRounds--;
		}

		if (reuse > 0 && IsRiposte) {
			reuse = 0;
		}

		did_attack = true;
	}

	if (skill_to_use == EQ::skills::SkillKick) {
		if (target != this) {
			DoAnim(animKick);
			if (GetWeaponDamage(target, GetBotItem(EQ::invslot::slotFeet)) <= 0)
				dmg = DMG_INVULNERABLE;

			DoSpecialAttackDamage(target, EQ::skills::SkillKick, dmg, 0, -1, reuse);
			did_attack = true;
		}
	}

	if (
		skill_to_use == EQ::skills::SkillFlyingKick ||
		skill_to_use == EQ::skills::SkillDragonPunch ||
		skill_to_use == EQ::skills::SkillEagleStrike ||
		skill_to_use == EQ::skills::SkillRoundKick
	) {
		reuse = (MonkSpecialAttack(target, skill_to_use) - 1);

		// Live AA - Technique of Master Wu
		int wuchance = itembonuses.DoubleSpecialAttack + spellbonuses.DoubleSpecialAttack + aabonuses.DoubleSpecialAttack;

		if (wuchance) {
			const int MonkSPA[5] = {
				EQ::skills::SkillFlyingKick,
				EQ::skills::SkillDragonPunch,
				EQ::skills::SkillEagleStrike,
				EQ::skills::SkillTigerClaw,
				EQ::skills::SkillRoundKick
			};
			int extra = 0;
			// always 1/4 of the double attack chance, 25% at rank 5 (100/4)
			while (wuchance > 0) {
				if (zone->random.Roll(wuchance)) {
					++extra;
				}
				else {
					break;
				}
				wuchance /= 4;
			}

			Mob* bo = GetBotOwner();
			if (bo && bo->IsClient() && bo->CastToClient()->GetBotOption(Client::booMonkWuMessage)) {

				bo->Message(
					GENERIC_EMOTE,
					"The spirit of Master Wu fills %s!  %s gains %d additional attack(s).",
					GetCleanName(),
					GetCleanName(),
					extra
				);
			}

			auto classic = RuleB(Combat, ClassicMasterWu);
			while (extra && TargetValidation(GetTarget())) {
				MonkSpecialAttack(GetTarget(), (classic ? MonkSPA[zone->random.Int(0, 4)] : skill_to_use));
				--extra;
			}
		}

		reuse *= 1000;
		did_attack = true;
	}

	if (skill_to_use == EQ::skills::SkillBackstab) {
		reuse = (BackstabReuseTime * 1000);
		did_attack = true;

		if (IsRiposte) {
			reuse = 0;
		}

		Mob::TryBackstab(target, reuse);
	}

	classattack_timer.Start(reuse / HasteModifier);
}

void Bot::MakePet(uint16 spell_id, const char* pettype, const char *petname) {
	Mob::MakePet(spell_id, pettype, petname);
}

void Bot::AI_Stop() {
	NPC::AI_Stop();
	Mob::AI_Stop();
}

FACTION_VALUE Bot::GetReverseFactionCon(Mob* iOther) {
	if (iOther->IsBot())
		return FACTION_ALLY;

	return NPC::GetReverseFactionCon(iOther);
}

Mob* Bot::GetOwnerOrSelf() {
	Mob* Result = nullptr;
	if (GetBotOwner())
		Result = GetBotOwner();
	else
		Result = this;

	return Result;
}

Mob* Bot::GetOwner() {
	Mob* Result = nullptr;
	Result = GetBotOwner();
	if (!Result)
		SetBotOwner(0);

	return Result;
}

bool Bot::IsBotAttackAllowed(Mob* attacker, Mob* target, bool& hasRuleDefined) {
	bool Result = false;
	if (attacker && target) {
		if (attacker == target) {
			hasRuleDefined = true;
			Result = false;
		} else if (attacker->IsClient() && target->IsBot() && attacker->CastToClient()->GetPVP() && target->CastToBot()->GetBotOwner()->CastToClient()->GetPVP()) {
			hasRuleDefined = true;
			Result = true;
		} else if (attacker->IsClient() && target->IsBot()) {
			hasRuleDefined = true;
			Result = false;
		} else if (attacker->IsBot() && target->IsNPC()) {
			hasRuleDefined = true;
			Result = true;
		} else if (attacker->IsBot() && !target->IsNPC()) {
			hasRuleDefined = true;
			Result = false;
		} else if (attacker->IsPet() && attacker->IsFamiliar()) {
			hasRuleDefined = true;
			Result = false;
		} else if (attacker->IsBot() && attacker->CastToBot()->GetBotOwner() && attacker->CastToBot()->GetBotOwner()->CastToClient()->GetPVP()) {
			if (target->IsBot() && target->GetOwner() && target->GetOwner()->CastToClient()->GetPVP()) {
				hasRuleDefined = true;
				if (target->GetOwner() == attacker->GetOwner())
					Result = false;
				else
					Result = true;
			} else if (target->IsClient() && target->CastToClient()->GetPVP()) {
				hasRuleDefined = true;
				if (target == attacker->GetOwner())
					Result = false;
				else
					Result = true;
			} else if (target->IsNPC()) {
				hasRuleDefined = true;
				Result = true;
			} else if (!target->IsNPC()) {
				hasRuleDefined = true;
				Result = false;
			}
		}
	}
	return Result;
}

void Bot::EquipBot() {
	GetBotItems(m_inv);
	const EQ::ItemInstance* inst = nullptr;
	const EQ::ItemData* item = nullptr;
	for (int slot_id = EQ::invslot::EQUIPMENT_BEGIN; slot_id <= EQ::invslot::EQUIPMENT_END; ++slot_id) {
		inst = GetBotItem(slot_id);
		if (inst) {
			item = inst->GetItem();
			BotTradeAddItem(inst, slot_id, false);
		}
	}
	UpdateEquipmentLight();
}

void Bot::BotOrderCampAll(Client* c, uint8 class_id) {
	if (c) {
		const auto& l = entity_list.GetBotsByBotOwnerCharacterID(c->CharacterID());
		for (const auto& b : l) {
			if (!class_id || b->GetClass() == class_id) {
				b->Camp(true);
			}
		}
	}
}

void Bot::ProcessBotOwnerRefDelete(Mob* botOwner) {
	if (botOwner && botOwner->IsClient()) {
		std::vector<Bot*> BotList = entity_list.GetBotsByBotOwnerCharacterID(botOwner->CastToClient()->CharacterID());
		if (!BotList.empty()) {
			for (std::vector<Bot*>::iterator botListItr = BotList.begin(); botListItr != BotList.end(); ++botListItr) {
				Bot* tempBot = *botListItr;
				if (tempBot) {
					tempBot->SetTarget(nullptr);
					tempBot->SetBotOwner(nullptr);
					tempBot->Zone();
				}
			}
		}
	}
}

int64 Bot::CalcMaxMana()
{
	if (IsIntelligenceCasterClass() || IsWisdomCasterClass()) {
		max_mana = (
			GenerateBaseManaPoints() +
			itembonuses.Mana +
			spellbonuses.Mana +
			aabonuses.Mana +
			GroupLeadershipAAManaEnhancement()
		);
		max_mana += itembonuses.heroic_max_mana;
	} else {
		max_mana = 0;
	}

	if (current_mana > max_mana) {
		current_mana = max_mana;
	} else if (max_mana < 0) {
		max_mana = 0;
	}

	return max_mana;
}

void Bot::SetAttackTimer() {
	float haste_mod = (GetHaste() * 0.01f);
	attack_timer.SetAtTrigger(4000, true);
	Timer* TimerToUse = nullptr;
	const EQ::ItemData* PrimaryWeapon = nullptr;
	for (int i = EQ::invslot::slotRange; i <= EQ::invslot::slotSecondary; i++) {
		if (i == EQ::invslot::slotPrimary)
			TimerToUse = &attack_timer;
		else if (i == EQ::invslot::slotRange)
			TimerToUse = &ranged_timer;
		else if (i == EQ::invslot::slotSecondary)
			TimerToUse = &attack_dw_timer;
		else
			continue;

		const EQ::ItemData* ItemToUse = nullptr;
		EQ::ItemInstance* ci = GetBotItem(i);
		if (ci)
			ItemToUse = ci->GetItem();

		if (i == EQ::invslot::slotSecondary) {
			if (PrimaryWeapon != nullptr) {
				if (PrimaryWeapon->IsClassCommon() && PrimaryWeapon->IsType2HWeapon()) {
					attack_dw_timer.Disable();
					continue;
				}
			}

			if (!GetSkill(EQ::skills::SkillDualWield)) {
				attack_dw_timer.Disable();
				continue;
			}
		}

		if (ItemToUse != nullptr) {
			if (!ItemToUse->IsClassCommon() || ItemToUse->Damage == 0 || ItemToUse->Delay == 0 || ((ItemToUse->ItemType > EQ::item::ItemTypeLargeThrowing) && (ItemToUse->ItemType != EQ::item::ItemTypeMartial) && (ItemToUse->ItemType != EQ::item::ItemType2HPiercing)))
				ItemToUse = nullptr;
		}

		int hhe = (itembonuses.HundredHands + spellbonuses.HundredHands);
		int speed = 0;
		int delay = 36;
		if (ItemToUse == nullptr) {
			delay = GetHandToHandDelay();
		} else {
			delay = ItemToUse->Delay;
		}

		speed = (RuleB(Spells, Jun182014HundredHandsRevamp) ? static_cast<int>(((delay / haste_mod) + ((hhe / 1000.0f) * (delay / haste_mod))) * 100) : static_cast<int>(((delay / haste_mod) + ((hhe / 100.0f) * delay)) * 100));
		TimerToUse->SetAtTrigger(std::max(RuleI(Combat, MinHastedDelay), speed), true, true);

		if (i == EQ::invslot::slotPrimary)
			PrimaryWeapon = ItemToUse;
	}
}

int32 Bot::GetActSpellDuration(uint16 spell_id, int32 duration) {
	int increase = 100;
	increase += GetFocusEffect(focusSpellDuration, spell_id);
	int64 tic_inc = 0;
	tic_inc = GetFocusEffect(focusSpellDurByTic, spell_id);

	if (IsBeneficialSpell(spell_id)) {
		switch (GetAA(aaSpellCastingReinforcement)) {
			case 1:
				increase += 5;
				break;
			case 2:
				increase += 15;
				break;
			case 3:
				increase += 30;
				if (GetAA(aaSpellCastingReinforcementMastery) == 1)
					increase += 20;

				break;
		}

		if (GetAA(aaSpellCastingReinforcementMastery))
			increase += 20;
	}

	if (IsMesmerizeSpell(spell_id))
		tic_inc += GetAA(aaMesmerizationMastery);

	return (((duration * increase) / 100) + tic_inc);
}

float Bot::GetAOERange(uint16 spell_id) {
	float range;
	range = spells[spell_id].aoe_range;
	if (range == 0)
		range = spells[spell_id].range;

	if (range == 0)
		range = 10;

	if (IsBardSong(spell_id) && IsBeneficialSpell(spell_id)) {
		float song_bonus = (aabonuses.SongRange + spellbonuses.SongRange + itembonuses.SongRange);
		range += (range * song_bonus / 100.0f);
	}
	range = GetActSpellRange(spell_id, range);
	return range;
}

bool Bot::SpellEffect(Mob* caster, uint16 spell_id, float partial) {
	bool Result = false;
	Result = Mob::SpellEffect(caster, spell_id, partial);
	if (IsGrouped()) {
		Group *g = GetGroup();
		if (g) {
			EQApplicationPacket hp_app;
			CreateHPPacket(&hp_app);
			for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
				if (g->members[i] && g->members[i]->IsClient())
					g->members[i]->CastToClient()->QueuePacket(&hp_app);
			}
		}
	}
	return Result;
}

void Bot::DoBuffTic(const Buffs_Struct &buff, int slot, Mob* caster) {
	Mob::DoBuffTic(buff, slot, caster);
}

bool Bot::CastSpell(
	uint16 spell_id,
	uint16 target_id,
	EQ::spells::CastingSlot slot,
	int32 cast_time,
	int32 mana_cost,
	uint32* oSpellWillFinish,
	uint32 item_slot,
	int16 *resist_adjust,
	uint32 aa_id
) {
	bool Result = false;
	if (zone && !zone->IsSpellBlocked(spell_id, glm::vec3(GetPosition()))) {
		// LogSpells("CastSpell called for spell [{}] ([{}]) on entity [{}], slot [{}], time [{}], mana [{}], from item slot [{}]", GetSpellName(spell_id), spell_id, target_id, slot, cast_time, mana_cost, (item_slot==0xFFFFFFFF)?999:item_slot);

		if (casting_spell_id == spell_id) {
			ZeroCastingVars();
		}

		if (GetClass() != Class::Bard) {
			if (
				!IsValidSpell(spell_id) ||
				casting_spell_id ||
				delaytimer ||
				spellend_timer.Enabled() ||
				((IsStunned() || IsMezzed() || DivineAura()) && !IsCastNotStandingSpell(spell_id)) ||
				(IsSilenced() && !IsDiscipline(spell_id)) ||
				(IsAmnesiad() && IsDiscipline(spell_id))
			) {
				LogSpellsDetail("Spell casting canceled: not able to cast now. Valid? [{}] casting [{}] waiting? [{}] spellend? [{}] stunned? [{}] feared? [{}] mezed? [{}] silenced? [{}]",
					IsValidSpell(spell_id), casting_spell_id, delaytimer, spellend_timer.Enabled(), IsStunned(), IsFeared(), IsMezzed(), IsSilenced()
				);

				if (casting_spell_id) {
					AI_Bot_Event_SpellCastFinished(false, static_cast<uint16>(casting_spell_slot));
				}

				return false;
			}
		}

		if (IsDetrimentalSpell(spell_id) && !zone->CanDoCombat()) {
			//MessageString(Chat::White, SPELL_WOULDNT_HOLD);
			if (casting_spell_id) {
				AI_Bot_Event_SpellCastFinished(false, static_cast<uint16>(casting_spell_slot));
			}

			return false;
		}

		if (DivineAura()) {
			LogSpellsDetail("Spell casting canceled: cannot cast while Divine Aura is in effect");
			InterruptSpell(SPELL_FIZZLE, 0x121, false);
			return false;
		}

		if (slot < EQ::spells::CastingSlot::MaxGems && !CheckFizzle(spell_id)) {
			int fizzle_msg = IsBardSong(spell_id) ? MISS_NOTE : SPELL_FIZZLE;
			InterruptSpell(fizzle_msg, 0x121, spell_id);

			uint32 use_mana = ((spells[spell_id].mana) / 4);
			LogSpellsDetail("Spell casting canceled: fizzled. [{}] mana has been consumed", use_mana);
			SetMana(GetMana() - use_mana);
			return false;
		}

		if (HasActiveSong()) {
			LogSpellsDetail("Casting a new spell/song while singing a song. Killing old song [{}]", bardsong);
			bardsong = 0;
			bardsong_target_id = 0;
			bardsong_slot = EQ::spells::CastingSlot::Gem1;
			bardsong_timer.Disable();
		}

		Result = Mob::CastSpell(spell_id, target_id, slot, cast_time, mana_cost, oSpellWillFinish, item_slot, 0xFFFFFFFF, 0, resist_adjust, aa_id);

	}
	return Result;
}

bool Bot::SpellOnTarget(
		uint16 spell_id,
		Mob *spelltar,
		int reflect_effectiveness,
		bool use_resist_adjust,
		int16 resist_adjust,
		bool isproc,
		int level_override,
		int duration_override,
		bool disable_buff_overwrite
) {
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	if (spelltar) {
		if (
			IsDetrimentalSpell(spell_id) &&
			(spelltar->IsBot() || spelltar->IsClient()) &&
			!IsResurrectionEffects(spell_id)
		) {
			return false;
		}

		if (spelltar->IsPet()) {
			for (int i = 0; i < EFFECT_COUNT; ++i) {
				if (spells[spell_id].effect_id[i] == SpellEffect::Illusion) {
					return false;
				}
			}
		}

		return Mob::SpellOnTarget(spell_id, spelltar);
	}

	return false;
}

bool Bot::IsImmuneToSpell(uint16 spell_id, Mob *caster) {
	bool Result = false;
	if (!caster)
		return false;

	if (!IsSacrificeSpell(spell_id) && zone->GetZoneID() != Zones::POKNOWLEDGE && this != caster) {
		Result = Mob::IsImmuneToSpell(spell_id, caster);
		if (!Result) {
			if (caster->IsBot()) {
				if (spells[spell_id].target_type == ST_Undead) {
					if ((GetBodyType() != BodyType::SummonedUndead) && (GetBodyType() != BodyType::Undead) && (GetBodyType() != BodyType::Vampire)) {
						LogSpellsDetail("Bot's target is not an undead");
						return true;
					}
				}
				if (spells[spell_id].target_type == ST_Summoned) {
					if ((GetBodyType() != BodyType::SummonedUndead) && (GetBodyType() != BodyType::Summoned) && (GetBodyType() != BodyType::Summoned2) && (GetBodyType() != BodyType::Summoned3)) {
						LogSpellsDetail("Bot's target is not a summoned creature");
						return true;
					}
				}
			}

			LogSpellsDetail("No bot immunities to spell [{}] found", spell_id);
		}
	}

	return Result;
}

bool Bot::DetermineSpellTargets(uint16 spell_id, Mob *&spell_target, Mob *&ae_center, CastAction_type &CastAction, EQ::spells::CastingSlot slot) {
	bool Result = false;
	SpellTargetType target_type = spells[spell_id].target_type;

	if (target_type == ST_GroupClientAndPet) {
		if ((spell_id == 1768 && zone->GetZoneID() == 202) || (!IsDetrimentalSpell(spell_id))) {
			CastAction = SingleTarget;
			return true;
		}
	}

	Result = Mob::DetermineSpellTargets(spell_id, spell_target, ae_center, CastAction, slot);

	return Result;
}

bool Bot::DoCastSpell(uint16 spell_id, uint16 target_id, EQ::spells::CastingSlot slot, int32 cast_time, int32 mana_cost, uint32* oSpellWillFinish, uint32 item_slot, uint32 aa_id) {
	bool Result = false;
	if (GetClass() == Class::Bard)
		cast_time = 0;

	Result = Mob::DoCastSpell(spell_id, target_id, slot, cast_time, mana_cost, oSpellWillFinish, item_slot, aa_id);

	if (oSpellWillFinish) {
		const SPDat_Spell_Struct &spell = spells[spell_id];
		*oSpellWillFinish = Timer::GetCurrentTime() + ((spell.recast_time > 20000) ? 10000 : spell.recast_time);
	}
	return Result;
}

int32 Bot::GenerateBaseManaPoints()
{
	int32 bot_mana        = 0;
	int32 WisInt          = 0;
	int32 MindLesserFactor, MindFactor;
	int   wisint_mana     = 0;
	int   base_mana       = 0;
	int   ConvertedWisInt = 0;

	if (IsIntelligenceCasterClass()) {
		WisInt = INT;

		if (
			GetOwner() &&
			GetOwner()->CastToClient() &&
			GetOwner()->CastToClient()->ClientVersion() >= EQ::versions::ClientVersion::SoD &&
			RuleB(Character, SoDClientUseSoDHPManaEnd)
		) {
			if (WisInt > 100) {
				ConvertedWisInt = (((WisInt - 100) * 5 / 2) + 100);
				if (WisInt > 201) {
					ConvertedWisInt -= ((WisInt - 201) * 5 / 4);
				}
			} else {
				ConvertedWisInt = WisInt;
			}

			if (GetLevel() < 41) {
				wisint_mana = (GetLevel() * 75 * ConvertedWisInt / 1000);
				base_mana   = (GetLevel() * 15);
			} else if (GetLevel() < 81) {
				wisint_mana = ((3 * ConvertedWisInt) + ((GetLevel() - 40) * 15 * ConvertedWisInt / 100));
				base_mana   = (600 + ((GetLevel() - 40) * 30));
			} else {
				wisint_mana = (9 * ConvertedWisInt);
				base_mana   = (1800 + ((GetLevel() - 80) * 18));
			}

			bot_mana = (base_mana + wisint_mana);
		} else {
			if (((WisInt - 199) / 2) > 0) {
				MindLesserFactor = ((WisInt - 199) / 2);
			} else {
				MindLesserFactor = 0;
			}

			MindFactor = WisInt - MindLesserFactor;
			if (WisInt > 100) {
				bot_mana = (((5 * (MindFactor + 20)) / 2) * 3 * GetLevel() / 40);
			} else {
				bot_mana = (((5 * (MindFactor + 200)) / 2) * 3 * GetLevel() / 100);
			}
		}
	} else if (IsWisdomCasterClass()) {
		WisInt = WIS;

		if (
			GetOwner() &&
			GetOwner()->CastToClient() &&
			GetOwner()->CastToClient()->ClientVersion() >= EQ::versions::ClientVersion::SoD &&
			RuleB(Character, SoDClientUseSoDHPManaEnd)
		) {
			if (WisInt > 100) {
				ConvertedWisInt = (((WisInt - 100) * 5 / 2) + 100);
				if (WisInt > 201) {
					ConvertedWisInt -= ((WisInt - 201) * 5 / 4);
				}
			} else {
				ConvertedWisInt = WisInt;
			}

			if (GetLevel() < 41) {
				wisint_mana = (GetLevel() * 75 * ConvertedWisInt / 1000);
				base_mana   = (GetLevel() * 15);
			} else if (GetLevel() < 81) {
				wisint_mana = ((3 * ConvertedWisInt) + ((GetLevel() - 40) * 15 * ConvertedWisInt / 100));
				base_mana   = (600 + ((GetLevel() - 40) * 30));
			} else {
				wisint_mana = (9 * ConvertedWisInt);
				base_mana   = (1800 + ((GetLevel() - 80) * 18));
			}

			bot_mana = (base_mana + wisint_mana);
		} else {
			if (((WisInt - 199) / 2) > 0) {
				MindLesserFactor = ((WisInt - 199) / 2);
			} else {
				MindLesserFactor = 0;
			}

			MindFactor = (WisInt - MindLesserFactor);
			if (WisInt > 100) {
				bot_mana = (((5 * (MindFactor + 20)) / 2) * 3 * GetLevel() / 40);
			} else {
				bot_mana = (((5 * (MindFactor + 200)) / 2) * 3 * GetLevel() / 100);
			}
		}
	} else {
		bot_mana = 0;
	}

	max_mana = bot_mana;
	return bot_mana;
}

void Bot::GenerateSpecialAttacks() {
	if (((GetClass() == Class::Monk) || (GetClass() == Class::Warrior) || (GetClass() == Class::Ranger) || (GetClass() == Class::Berserker))	&& (GetLevel() >= 60))
		SetSpecialAbility(SpecialAbility::TripleAttack, 1);
}

bool Bot::DoFinishedSpellSingleTarget(uint16 spell_id, Mob* spellTarget, EQ::spells::CastingSlot slot, bool& stopLogic) {

	if (
		spellTarget &&
		GetClass() != Class::Bard &&
		(IsGrouped() || (IsRaidGrouped() && GetStoredRaid()->GetGroup(GetCleanName()) != RAID_GROUPLESS)) &&
		(spellTarget->IsBot() || spellTarget->IsClient()) &&
		(RuleB(Bots, GroupBuffing) || RuleB(Bots, RaidBuffing))
	) {
		bool noGroupSpell = false;
		uint16 thespell = spell_id;
		for (int i = 0; i < AIBot_spells.size(); i++) { // TODO bot rewrite - fix this to reduce loop with AIBot_spells_by_type?
			int j = BotGetSpells(i);
			int spelltype = BotGetSpellType(i);
			bool spellequal = (j == thespell);
			bool spelltypeequal = (
				(spelltype == BotSpellTypes::RegularHeal) ||
				(spelltype == BotSpellTypes::Escape) ||
				(spelltype == BotSpellTypes::Pet)
			);
			bool spelltypetargetequal = (
				(spelltype == BotSpellTypes::Buff) &&
				(spells[thespell].target_type == ST_Self)
			);
			bool spelltypeclassequal = (
				(spelltype == BotSpellTypes::InCombatBuff) &&
				(GetClass() == Class::Shaman)
			);
			bool slotequal = (slot == EQ::spells::CastingSlot::Item);

			if (spellequal || slotequal) {
				if (
					(
						spelltypeequal || spelltypetargetequal
					) ||
					spelltypeclassequal ||
					slotequal
				) {
					if (
						(
							(spells[thespell].effect_id[0] == 0) &&
							(spells[thespell].base_value[0] < 0)
						) &&
						(spellTarget->GetHP() < ((spells[thespell].base_value[0] * (-1)) + 100))
					) {
						LogSpells("GroupBuffing failure");
						return false;
					}

					SpellOnTarget(thespell, spellTarget);
					noGroupSpell = true;
					stopLogic = true;
				}
			}
		}

		if (!noGroupSpell) {
			for (Mob* m : GetBuffTargets(spellTarget)) {
				if (IsEffectInSpell(thespell, SpellEffect::AbsorbMagicAtt) || IsEffectInSpell(thespell, SpellEffect::Rune)) {
					for (int i = 0; i < m->GetMaxTotalSlots(); i++) {
						uint32 buff_count = m->GetMaxTotalSlots();

						for (unsigned int j = 0; j < buff_count; j++) {
							if (IsValidSpell(m->GetBuffs()[j].spellid)) {
								if (IsLichSpell(m->GetBuffs()[j].spellid)) {
									continue;
								}
							}
						}
					}
				}

				SpellOnTarget(thespell, m);

				if (
					m->GetPetID() &&
					(
						!RuleB(Bots, RequirePetAffinity) ||
						m->HasPetAffinity()
					)
				) {
					SpellOnTarget(thespell, m->GetPet());
				}

				SetMana(GetMana() - (GetActSpellCost(thespell, spells[thespell].mana) * (GetBuffTargets(spellTarget).size() - 1)));
			}
		}

		stopLogic = true;
	}

	return true;
}

bool Bot::DoFinishedSpellGroupTarget(uint16 spell_id, Mob* spellTarget, EQ::spells::CastingSlot slot, bool& stopLogic) {
	bool isMainGroupMGB = false;

	if (isMainGroupMGB && (GetClass() != Class::Bard)) {
		RaidGroupSay(
			fmt::format(
				"Casting {} as a Mass Group Buff.",
				spells[spell_id].name
			).c_str()
		);
		SpellOnTarget(spell_id, this);
		entity_list.AESpell(this, this, spell_id, true);
	}
	else {
		if (spellTarget != this) {
			SpellOnTarget(spell_id, this);
		}

		if (spellTarget->IsOfClientBotMerc()) {
			for (Mob* m : GetBuffTargets(spellTarget)) {
				if (m == this && spellTarget != this) {
					continue;
				}

				SpellOnTarget(spell_id, m);

				if (m->GetPetID() && (!RuleB(Bots, RequirePetAffinity) || m->HasPetAffinity())) {
					SpellOnTarget(spell_id, m->GetPet());
				}
			}
		}
		else if (spellTarget->IsPet() && !spellTarget->IsFamiliar() && spellTarget->GetOwner() && (!RuleB(Bots, RequirePetAffinity) || spellTarget->GetOwner()->HasPetAffinity())) {
			SpellOnTarget(spell_id, spellTarget);
		}
	}

	stopLogic = true;
	return true;
}

void Bot::CalcBonuses() {
	memset(&itembonuses, 0, sizeof(StatBonuses));
	GenerateBaseStats();
	CalcItemBonuses(&itembonuses);
	CalcHeroicBonuses(&itembonuses);
	CalcSpellBonuses(&spellbonuses);
	CalcAABonuses(&aabonuses);
	SetAttackTimer();
	CalcSeeInvisibleLevel();
	CalcInvisibleLevel();
	ProcessItemCaps();
	CalcATK();
	CalcSTR();
	CalcSTA();
	CalcDEX();
	CalcAGI();
	CalcINT();
	CalcWIS();
	CalcCHA();
	CalcMR();
	CalcFR();
	CalcDR();
	CalcPR();
	CalcCR();
	CalcCorrup();
	CalcAC();
	CalcMaxHP();
	CalcMaxMana();
	CalcMaxEndurance();
	hp_regen = CalcHPRegen();
	mana_regen = CalcManaRegen();
	end_regen = CalcEnduranceRegen();
}

int64 Bot::CalcHPRegenCap() {
	int64 hpregen_cap = 0;
	hpregen_cap = (RuleI(Character, ItemHealthRegenCap) + itembonuses.HeroicSTA / 25);
	hpregen_cap += (aabonuses.ItemHPRegenCap + spellbonuses.ItemHPRegenCap + itembonuses.ItemHPRegenCap);
	return (hpregen_cap * RuleI(Character, HPRegenMultiplier) / 100);
}

int64 Bot::CalcManaRegenCap() {
	int64 cap = RuleI(Character, ItemManaRegenCap) + aabonuses.ItemManaRegenCap + itembonuses.heroic_mana_regen;
	return (cap * RuleI(Character, ManaRegenMultiplier) / 100);
}

int32 Bot::GetMaxStat() {
	int level = GetLevel();
	int32 base = 0;
	if (level < 61)
		base = 255;
	else if (GetOwner() && GetOwner()->CastToClient() && GetOwner()->CastToClient()->ClientVersion() >= EQ::versions::ClientVersion::SoF)
		base = (255 + 5 * (level - 60));
	else if (level < 71)
		base = (255 + 5 * (level - 60));
	else
		base = 330;

	return base;
}

int32 Bot::GetMaxResist() {
	int level = GetLevel();
	int32 base = 500;
	if (level > 65) {
		base += ((level - 65) * 5);
	}
	return base;
}

int32 Bot::GetMaxSTR() {
	return (GetMaxStat() + itembonuses.STRCapMod + spellbonuses.STRCapMod + aabonuses.STRCapMod);
}

int32 Bot::GetMaxSTA() {
	return (GetMaxStat() + itembonuses.STACapMod + spellbonuses.STACapMod + aabonuses.STACapMod);
}

int32 Bot::GetMaxDEX() {
	return (GetMaxStat() + itembonuses.DEXCapMod + spellbonuses.DEXCapMod + aabonuses.DEXCapMod);
}

int32 Bot::GetMaxAGI() {
	return (GetMaxStat() + itembonuses.AGICapMod + spellbonuses.AGICapMod + aabonuses.AGICapMod);
}

int32 Bot::GetMaxINT() {
	return (GetMaxStat() + itembonuses.INTCapMod + spellbonuses.INTCapMod + aabonuses.INTCapMod);
}

int32 Bot::GetMaxWIS() {
	return (GetMaxStat() + itembonuses.WISCapMod + spellbonuses.WISCapMod + aabonuses.WISCapMod);
}
int32 Bot::GetMaxCHA() {
	return (GetMaxStat() + itembonuses.CHACapMod + spellbonuses.CHACapMod + aabonuses.CHACapMod);
}

int32 Bot::GetMaxMR() {
	return (GetMaxResist() + itembonuses.MRCapMod + spellbonuses.MRCapMod + aabonuses.MRCapMod);
}

int32 Bot::GetMaxPR() {
	return (GetMaxResist() + itembonuses.PRCapMod + spellbonuses.PRCapMod + aabonuses.PRCapMod);
}

int32 Bot::GetMaxDR() {
	return (GetMaxResist() + itembonuses.DRCapMod + spellbonuses.DRCapMod + aabonuses.DRCapMod);
}

int32 Bot::GetMaxCR() {
	return (GetMaxResist() + itembonuses.CRCapMod + spellbonuses.CRCapMod + aabonuses.CRCapMod);
}

int32 Bot::GetMaxFR() {
	return (GetMaxResist() + itembonuses.FRCapMod + spellbonuses.FRCapMod + aabonuses.FRCapMod);
}

int32 Bot::GetMaxCorrup() {
	return (GetMaxResist() + itembonuses.CorrupCapMod + spellbonuses.CorrupCapMod + aabonuses.CorrupCapMod);
}

int32 Bot::CalcSTR() {
	int32 val = (STR + itembonuses.STR + spellbonuses.STR);
	int32 mod = aabonuses.STR;
	if (val > 255 && GetLevel() <= 60)
		val = 255;

	STR = (val + mod);
	if (STR < 1)
		STR = 1;

	int m = GetMaxSTR();
	if (STR > m)
		STR = m;

	return STR;
}

int32 Bot::CalcSTA() {
	int32 val = (STA + itembonuses.STA + spellbonuses.STA);
	int32 mod = aabonuses.STA;
	if (val > 255 && GetLevel() <= 60)
		val = 255;

	STA = (val + mod);
	if (STA < 1)
		STA = 1;

	int m = GetMaxSTA();
	if (STA > m)
		STA = m;

	return STA;
}

int32 Bot::CalcAGI() {
	int32 val = (AGI + itembonuses.AGI + spellbonuses.AGI);
	int32 mod = aabonuses.AGI;
	if (val > 255 && GetLevel() <= 60)
		val = 255;

	AGI = (val + mod);
	if (AGI < 1)
		AGI = 1;

	int m = GetMaxAGI();
	if (AGI > m)
		AGI = m;

	return AGI;
}

int32 Bot::CalcDEX() {
	int32 val = (DEX + itembonuses.DEX + spellbonuses.DEX);
	int32 mod = aabonuses.DEX;
	if (val > 255 && GetLevel() <= 60)
		val = 255;

	DEX = (val + mod);
	if (DEX < 1)
		DEX = 1;

	int m = GetMaxDEX();
	if (DEX > m)
		DEX = m;

	return DEX;
}

int32 Bot::CalcINT() {
	int32 val = (INT + itembonuses.INT + spellbonuses.INT);
	int32 mod = aabonuses.INT;
	if (val > 255 && GetLevel() <= 60)
		val = 255;

	INT = (val + mod);

	if (INT < 1)
		INT = 1;

	int m = GetMaxINT();
	if (INT > m)
		INT = m;

	return INT;
}

int32 Bot::CalcWIS() {
	int32 val = (WIS + itembonuses.WIS + spellbonuses.WIS);
	int32 mod = aabonuses.WIS;
	if (val > 255 && GetLevel() <= 60)
		val = 255;

	WIS = (val + mod);

	if (WIS < 1)
		WIS = 1;

	int m = GetMaxWIS();
	if (WIS > m)
		WIS = m;

	return WIS;
}

int32 Bot::CalcCHA() {
	int32 val = (CHA + itembonuses.CHA + spellbonuses.CHA);
	int32 mod = aabonuses.CHA;
	if (val > 255 && GetLevel() <= 60)
		val = 255;

	CHA = (val + mod);

	if (CHA < 1)
		CHA = 1;

	int m = GetMaxCHA();
	if (CHA > m)
		CHA = m;

	return CHA;
}

int32 Bot::CalcMR() {
	MR += (itembonuses.MR + spellbonuses.MR + aabonuses.MR);
	if (GetClass() == Class::Warrior)
		MR += (GetLevel() / 2);

	if (MR < 1)
		MR = 1;

	if (MR > GetMaxMR())
		MR = GetMaxMR();

	return MR;
}

int32 Bot::CalcFR() {
	int c = GetClass();
	if (c == Class::Ranger) {
		FR += 4;
		int l = GetLevel();
		if (l > 49)
			FR += (l - 49);
	}

	FR += (itembonuses.FR + spellbonuses.FR + aabonuses.FR);

	if (FR < 1)
		FR = 1;

	if (FR > GetMaxFR())
		FR = GetMaxFR();

	return FR;
}

int32 Bot::CalcDR() {
	int c = GetClass();
	if (c == Class::Paladin) {
		DR += 8;
		int l = GetLevel();
		if (l > 49)
			DR += (l - 49);
	} else if (c == Class::ShadowKnight) {
		DR += 4;
		int l = GetLevel();
		if (l > 49)
			DR += (l - 49);
	}

	DR += (itembonuses.DR + spellbonuses.DR + aabonuses.DR);
	if (DR < 1)
		DR = 1;

	if (DR > GetMaxDR())
		DR = GetMaxDR();

	return DR;
}

int32 Bot::CalcPR() {
	int c = GetClass();
	if (c == Class::Rogue) {
		PR += 8;
		int l = GetLevel();
		if (l > 49)
			PR += (l - 49);
	} else if (c == Class::ShadowKnight) {
		PR += 4;
		int l = GetLevel();
		if (l > 49)
			PR += (l - 49);
	}

	PR += (itembonuses.PR + spellbonuses.PR + aabonuses.PR);

	if (PR < 1)
		PR = 1;

	if (PR > GetMaxPR())
		PR = GetMaxPR();

	return PR;
}

int32 Bot::CalcCR() {
	int c = GetClass();
	if (c == Class::Ranger) {
		CR += 4;
		int l = GetLevel();
		if (l > 49)
			CR += (l - 49);
	}

	CR += (itembonuses.CR + spellbonuses.CR + aabonuses.CR);

	if (CR < 1)
		CR = 1;

	if (CR > GetMaxCR())
		CR = GetMaxCR();

	return CR;
}

int32 Bot::CalcCorrup() {
	Corrup = (Corrup + itembonuses.Corrup + spellbonuses.Corrup + aabonuses.Corrup);
	if (Corrup > GetMaxCorrup())
		Corrup = GetMaxCorrup();

	return Corrup;
}

int32 Bot::CalcATK() {
	ATK = (itembonuses.ATK + spellbonuses.ATK + aabonuses.ATK + GroupLeadershipAAOffenseEnhancement());
	return ATK;
}

void Bot::CalcRestState() {
	if (!RuleB(Character, RestRegenEnabled))
		return;

	RestRegenHP = RestRegenMana = RestRegenEndurance = 0;
	if (IsEngaged() || !IsSitting() || !rest_timer.Check(false))
		return;

	uint32 buff_count = GetMaxTotalSlots();
	for (unsigned int j = 0; j < buff_count; j++) {
		if (IsValidSpell(buffs[j].spellid)) {
			if (IsDetrimentalSpell(buffs[j].spellid) && (buffs[j].ticsremaining > 0))
				if (!IsRestAllowedSpell(buffs[j].spellid))
					return;
		}
	}

	RestRegenHP = 6 * (GetMaxHP() / zone->newzone_data.fast_regen_hp);
	RestRegenMana = 6 * (GetMaxMana() / zone->newzone_data.fast_regen_mana);
	RestRegenEndurance = 6 * (GetMaxEndurance() / zone->newzone_data.fast_regen_endurance);
}

int32 Bot::LevelRegen() {
	int level = GetLevel();
	bool bonus = GetPlayerRaceBit(_baseRace) & RuleI(Character, BaseHPRegenBonusRaces);
	uint8 multiplier1 = bonus ? 2 : 1;
	int64 hp = 0;
	if (level < 51) {
		if (IsSitting()) {
			if (level < 20)
				hp += (2 * multiplier1);
			else if (level < 50)
				hp += (3 * multiplier1);
			else
				hp += (4 * multiplier1);
		} else
			hp += (1 * multiplier1);
	} else {
		int32 tmp = 0;
		float multiplier2 = 1;
		if (level < 56) {
			tmp = 2;
			if (bonus)
				multiplier2 = 3;
		} else if (level < 60) {
			tmp = 3;
			if (bonus)
				multiplier2 = 3.34;
		}
		else if (level < 61) {
			tmp = 4;
			if (bonus)
				multiplier2 = 3;
		} else if (level < 63) {
			tmp = 5;
			if (bonus)
				multiplier2 = 2.8;
		} else if (level < 65) {
			tmp = 6;
			if (bonus)
				multiplier2 = 2.67;
		} else {
			tmp = 7;
			if (bonus)
				multiplier2 = 2.58;
		}
		hp += (int32(float(tmp) * multiplier2));
	}
	return hp;
}

int64 Bot::CalcHPRegen() {
	int32 regen = (LevelRegen() + itembonuses.HPRegen + spellbonuses.HPRegen);
	regen += itembonuses.heroic_hp_regen;
	regen += (aabonuses.HPRegen + GroupLeadershipAAHealthRegeneration());

	regen = ((regen * RuleI(Character, HPRegenMultiplier)) / 100);
	return regen;
}

int64 Bot::CalcManaRegen() {
    uint8 level = GetLevel();
    uint8 bot_class = GetClass();

    // Default values
    int32 regen = 2; // Default regen for non-sitting state
    float mana_regen_rate = std::max(0.0f, RuleR(Bots, ManaRegen));

    if (bot_class == Class::Bard) {
        regen = IsSitting() ? 2 : 1;

        if (IsSitting()) {
            BuffFadeBySitModifier();
        }

        regen += itembonuses.ManaRegen + aabonuses.ManaRegen;

        return regen;
    }

    if (IsSitting()) {
        BuffFadeBySitModifier();

        if (GetArchetype() != Archetype::Melee) {
            regen = (((GetSkill(EQ::skills::SkillMeditate) / 10) + (level - (level / 4))) / 4) + 4;
        }
    }

    regen += spellbonuses.ManaRegen + itembonuses.ManaRegen;

    if (IsHeroicINTCasterClass(bot_class)) {
        regen += itembonuses.HeroicINT * RuleR(Character, HeroicIntelligenceMultiplier) / 25;
    } else if (IsHeroicWISCasterClass(bot_class)) {
        regen += itembonuses.HeroicWIS * RuleR(Character, HeroicWisdomMultiplier) / 25;
    }

    regen += aabonuses.ManaRegen;
    regen = (regen * RuleI(Character, ManaRegenMultiplier)) / 100;

    return static_cast<int32>(regen * mana_regen_rate);
}

uint64 Bot::GetClassHPFactor() {
	uint32 factor;
	switch (GetClass()) {
		case Class::Beastlord:
		case Class::Berserker:
		case Class::Monk:
		case Class::Rogue:
		case Class::Shaman:
			factor = 255;
			break;
		case Class::Bard:
		case Class::Cleric:
			factor = 264;
			break;
		case Class::ShadowKnight:
		case Class::Paladin:
			factor = 288;
			break;
		case Class::Ranger:
			factor = 276;
			break;
		case Class::Warrior:
			factor = 300;
			break;
		default:
			factor = 240;
			break;
	}
	return factor;
}

int64 Bot::CalcMaxHP() {
	int32 bot_hp = 0;
	uint32 nd = 10000;
	bot_hp += (GenerateBaseHitPoints() + itembonuses.HP);
	bot_hp += itembonuses.heroic_max_hp;
	nd += aabonuses.PercentMaxHPChange + spellbonuses.PercentMaxHPChange + itembonuses.PercentMaxHPChange;
	bot_hp = ((float)bot_hp * (float)nd / (float)10000);
	bot_hp += (spellbonuses.FlatMaxHPChange + aabonuses.FlatMaxHPChange + itembonuses.FlatMaxHPChange);
	bot_hp += GroupLeadershipAAHealthEnhancement();
	max_hp = bot_hp;
	if (current_hp > max_hp)
		current_hp = max_hp;

	int hp_perc_cap = spellbonuses.HPPercCap[0];
	if (hp_perc_cap) {
		int curHP_cap = ((max_hp * hp_perc_cap) / 100);
		if (current_hp > curHP_cap || (spellbonuses.HPPercCap[1] && current_hp > spellbonuses.HPPercCap[1]))
			current_hp = curHP_cap;
	}
	return max_hp;
}

int64 Bot::CalcMaxEndurance() {
	max_end = (CalcBaseEndurance() + spellbonuses.Endurance + itembonuses.Endurance);
	if (max_end < 0)
		max_end = 0;

	if (cur_end > max_end)
		cur_end = max_end;

	int end_perc_cap = spellbonuses.EndPercCap[0];
	if (end_perc_cap) {
		int curEnd_cap = ((max_end * end_perc_cap) / 100);
		if (cur_end > curEnd_cap || (spellbonuses.EndPercCap[1] && cur_end > spellbonuses.EndPercCap[1]))
			cur_end = curEnd_cap;
	}

	return max_end;
}

int64 Bot::CalcBaseEndurance() {
	int32 base_end = 0;
	int32 base_endurance = 0;
	int32 converted_stats = 0;
	int32 sta_end = 0;
	int stats = 0;
	if (GetOwner() && GetOwner()->CastToClient() && GetOwner()->CastToClient()->ClientVersion() >= EQ::versions::ClientVersion::SoD && RuleB(Character, SoDClientUseSoDHPManaEnd)) {
		stats = ((GetSTR() + GetSTA() + GetDEX() + GetAGI()) / 4);

		if (stats > 100) {
			converted_stats = (((stats - 100) * 5 / 2) + 100);
			if (stats > 201) {
				converted_stats -= ((stats - 201) * 5 / 4);
			}
		} else {
			converted_stats = stats;
		}

		if (GetLevel() < 41) {
			sta_end = (GetLevel() * 75 * converted_stats / 1000);
			base_endurance = (GetLevel() * 15);
		} else if (GetLevel() < 81) {
			sta_end = ((3 * converted_stats) + ((GetLevel() - 40) * 15 * converted_stats / 100));
			base_endurance = (600 + ((GetLevel() - 40) * 30));
		} else {
			sta_end = (9 * converted_stats);
			base_endurance = (1800 + ((GetLevel() - 80) * 18));
		}
		base_end = base_endurance + sta_end + itembonuses.heroic_max_end;
	} else {

		stats = (GetSTR() + GetSTA() + GetDEX() + GetAGI());
		int level_base = (GetLevel() * 15);
		int at_most_800 = stats;
		if(at_most_800 > 800)
			at_most_800 = 800;

		int Bonus400to800 = 0;
		int HalfBonus400to800 = 0;
		int Bonus800plus = 0;
		int HalfBonus800plus = 0;

		auto BonusUpto800 = int(at_most_800 / 4) ;

		if(stats > 400) {
			Bonus400to800 = int((at_most_800 - 400) / 4);
			HalfBonus400to800 = int(std::max((at_most_800 - 400), 0) / 8);
			if(stats > 800) {
				Bonus800plus = (int((stats - 800) / 8) * 2);
				HalfBonus800plus = int((stats - 800) / 16);
			}
		}
		int bonus_sum = (BonusUpto800 + Bonus400to800 + HalfBonus400to800 + Bonus800plus + HalfBonus800plus);
		base_end = level_base;
		base_end += ((bonus_sum * 3 * GetLevel()) / 40);
	}
	return base_end;
}

int64 Bot::CalcEnduranceRegen() {
	int32 regen = (int32(GetLevel() * 4 / 10) + 2);
	regen += (spellbonuses.EnduranceRegen + itembonuses.EnduranceRegen);
	return (regen * RuleI(Character, EnduranceRegenMultiplier) / 100);
}

int64 Bot::CalcEnduranceRegenCap() {
	int cap = (RuleI(Character, ItemEnduranceRegenCap) + itembonuses.HeroicSTR / 25 + itembonuses.HeroicDEX / 25 + itembonuses.HeroicAGI / 25 + itembonuses.HeroicSTA / 25);
	return (cap * RuleI(Character, EnduranceRegenMultiplier) / 100);
}

void Bot::SetEndurance(int32 newEnd) {
	if (newEnd < 0)
		newEnd = 0;
	else if (newEnd > GetMaxEndurance())
		newEnd = GetMaxEndurance();

	cur_end = newEnd;
}

void Bot::DoEnduranceUpkeep() {
	int upkeep_sum = 0;
	int cost_redux = (spellbonuses.EnduranceReduction + itembonuses.EnduranceReduction);
	uint32 buffs_i;
	uint32 buff_count = GetMaxTotalSlots();
	for (buffs_i = 0; buffs_i < buff_count; buffs_i++) {
		if (IsValidSpell(buffs[buffs_i].spellid)) {
			int upkeep = spells[buffs[buffs_i].spellid].endurance_upkeep;
			if (upkeep > 0) {
				if (cost_redux > 0) {
					if (upkeep <= cost_redux)
						continue;

					upkeep -= cost_redux;
				}

				if ((upkeep+upkeep_sum) > GetEndurance())
					BuffFadeBySlot(buffs_i);
				else
					upkeep_sum += upkeep;
			}
		}
	}

	if (upkeep_sum != 0)
		SetEndurance(GetEndurance() - upkeep_sum);
}

void Bot::Camp(bool save_to_database) {
	Sit();

	LeaveHealRotationMemberPool();

	if (save_to_database) {
		Save();
	}

	if (HasGroup() || HasRaid()) {
		Zone();
	} else {
		Depop();
	}
}

void Bot::Zone() {
	if (auto raid = entity_list.GetRaidByBot(this)) {
		raid->MemberZoned(CastToClient());
	}
	else if (HasGroup()) {
		GetGroup()->MemberZoned(this);
	}

	Save();
	Depop();
}

bool Bot::IsAtRange(Mob *target) {
	bool result = false;

	if (target) {
		float range = (GetBotRangedValue() + 5.0);
		range *= range;
		float targetDistance = DistanceSquaredNoZ(m_Position, target->GetPosition());
		float minRuleDistance = (RuleI(Combat, MinRangedAttackDist) * RuleI(Combat, MinRangedAttackDist));

		if ((targetDistance > range) || (targetDistance < minRuleDistance)) {
			result = false;
		}
		else {
			result = true;
		}
	}
	return result;
}

void Bot::UpdateGroupCastingRoles(const Group* group, bool disband)
{
	if (!group)
		return;

	for (auto iter : group->members) {
		if (!iter)
			continue;

		if (iter->IsBot()) {
			iter->CastToBot()->SetGroupHealer(false);
			iter->CastToBot()->SetGroupSlower(false);
			iter->CastToBot()->SetGroupNuker(false);
			iter->CastToBot()->SetGroupDoter(false);
		}
	}

	if (disband)
		return;

	Mob* healer = nullptr;
	Mob* slower = nullptr;

	for (auto iter : group->members) {
		if (!iter)
			continue;

		// GroupHealer
		switch (iter->GetClass()) {
			case Class::Cleric:
				if (!healer)
					healer = iter;
				else
					switch (healer->GetClass()) {
						case Class::Cleric:
							break;
						default:
							healer = iter;
					}

				break;
			case Class::Druid:
				if (!healer)
					healer = iter;
				else
					switch (healer->GetClass()) {
						case Class::Cleric:
						case Class::Druid:
							break;
						default:
							healer = iter;
					}
				break;
			case Class::Shaman:
				if (!healer)
					healer = iter;
				else
					switch (healer->GetClass()) {
						case Class::Cleric:
						case Class::Druid:
						case Class::Shaman:
							break;
						default:
							healer = iter;
					}
				break;
			case Class::Paladin:
			case Class::Ranger:
			case Class::Beastlord:
				if (!healer)
					healer = iter;
				break;
			default:
				break;
			}

			// GroupSlower
			switch (iter->GetClass()) {
				case Class::Shaman:
					if (!slower)
						slower = iter;
					else
						switch (slower->GetClass()) {
							case Class::Shaman:
								break;
							default:
								slower = iter;
						}
					break;
				case Class::Enchanter:
					if (!slower)
						slower = iter;
					else
						switch (slower->GetClass()) {
							case Class::Shaman:
							case Class::Enchanter:
								break;
							default:
								slower = iter;
						}
					break;
				case Class::Beastlord:
					if (!slower)
						slower = iter;
					break;
				default:
					break;
			}

			// GroupNuker
			switch (iter->GetClass()) {
				// wizard
				// magician
				// necromancer
				// enchanter
				// druid
				// cleric
				// shaman
				// shadowknight
				// paladin
				// ranger
				// beastlord
				default:
					break;
			}

			// GroupDoter
			switch (iter->GetClass()) {
				default:
					break;
		}
	}

	if (healer && healer->IsBot())
		healer->CastToBot()->SetGroupHealer();
	if (slower && slower->IsBot())
		slower->CastToBot()->SetGroupSlower();
}

Bot* Bot::GetBotByBotClientOwnerAndBotName(Client* c, const std::string& botName) {
	Bot* Result = nullptr;
	if (c) {
		std::vector<Bot*> BotList = entity_list.GetBotsByBotOwnerCharacterID(c->CharacterID());
		if (!BotList.empty()) {
			for (auto botListItr = BotList.begin(); botListItr != BotList.end(); ++botListItr) {
				if (std::string((*botListItr)->GetCleanName()) == botName) {
					Result = (*botListItr);
					break;
				}
			}
		}
	}
	return Result;
}

void Bot::ProcessBotGroupInvite(Client* c, std::string const& botName) {
	if (c && !c->HasRaid()) {
		Bot* invitedBot = entity_list.GetBotByBotName(botName);

		if (!invitedBot) {
			return;
		}

		if (
			invitedBot->GetBotOwnerCharacterID() != c->CharacterID() &&
			!(
				c->GetGroup() &&
				!invitedBot->HasGroup() &&
				invitedBot->GetOwner()->HasGroup() &&
				c->GetGroup() == invitedBot->GetOwner()->GetGroup()
			)
		) {
			c->Message(Chat::Red, "%s's owner needs to be in your group to be able to invite them.", invitedBot->GetCleanName());

			return;
		}

		if (!invitedBot->HasGroup() && !invitedBot->HasRaid()) {
			if (!c->IsGrouped()) {
				auto g = new Group(c);
				if (AddBotToGroup(invitedBot, g)) {
					entity_list.AddGroup(g);
					database.SetGroupLeaderName(g->GetID(), c->GetName());
					g->SaveGroupLeaderAA();
					g->AddToGroup(c);
					g->AddToGroup(invitedBot);
				} else {
					delete g;
				}
			} else {
				if (AddBotToGroup(invitedBot, c->GetGroup())) {
					c->GetGroup()->AddToGroup(invitedBot);
				}
			}
		} else if (invitedBot->HasGroup()) {
			c->MessageString(Chat::LightGray, TARGET_ALREADY_IN_GROUP, invitedBot->GetCleanName());
		}
	}
}

// Processes a group disband request from a Client for a Bot.
void Bot::ProcessBotGroupDisband(Client* c, const std::string& botName) {
	if (c) {
		Bot* tempBot = nullptr;

		if (botName.empty())
			tempBot = GetFirstBotInGroup(c->GetGroup());
		else
			tempBot = GetBotByBotClientOwnerAndBotName(c, botName);

		RemoveBotFromGroup(tempBot, c->GetGroup());
	}
}

// Processes a raid disband request from a Client for a Bot.
void Bot::RemoveBotFromRaid(Bot* bot) {

	Raid* bot_raid = entity_list.GetRaidByBot(bot);
	if (bot_raid) {
		uint32 gid = bot_raid->GetGroup(bot->GetName());
		bot_raid->SendRaidGroupRemove(bot->GetName(), gid);
		bot_raid->RemoveMember(bot->GetName());
		bot_raid->GroupUpdate(gid);
		if (!bot_raid->RaidCount()) {
			bot_raid->DisbandRaid();
		}
	}

	bot->SetStoredRaid(nullptr);
	bot->p_raid_instance = nullptr;
	bot->SetVerifiedRaid(false);
}

// Handles all client zone change event
void Bot::ProcessClientZoneChange(Client* botOwner) {
	if (botOwner) {
		std::vector<Bot*> BotList = entity_list.GetBotsByBotOwnerCharacterID(botOwner->CharacterID());

		for (std::vector<Bot*>::iterator itr = BotList.begin(); itr != BotList.end(); ++itr) {
			Bot* tempBot = *itr;

			if (tempBot) {
				Raid* raid = entity_list.GetRaidByBot(tempBot);
				if (raid) {
					tempBot->Zone();
				}
				else if (tempBot->HasGroup()) {
					Group* g = tempBot->GetGroup();
					if (g && g->IsGroupMember(botOwner)) {
						if (botOwner->IsClient()) {
							// Modified to not only zone bots if you're the leader.
							// Also zone bots of the non-leader when they change zone.
							if (tempBot->GetBotOwnerCharacterID() == botOwner->CharacterID() && g->IsGroupMember(botOwner))
								tempBot->Zone();
							else
								tempBot->Camp();
						}
					}
					else
						tempBot->Camp();
				}
				else
					tempBot->Camp();
			}
		}
	}
}

// Finds and returns the first Bot object found in specified group
Bot* Bot::GetFirstBotInGroup(Group* group) {
	Bot* Result = nullptr;

	if (group) {
		for (int Counter = 0; Counter < MAX_GROUP_MEMBERS; Counter++) {
			if (group->members[Counter] == nullptr) {
				continue;
			}

			if (group->members[Counter]->IsBot()) {
				Result = group->members[Counter]->CastToBot();
				break;
			}
		}
	}

	return Result;
}

// Processes a client request to inspect a bot's equipment.
void Bot::ProcessBotInspectionRequest(Bot* inspectedBot, Client* client) {
	if (inspectedBot && client) {
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_InspectAnswer, sizeof(InspectResponse_Struct));
		InspectResponse_Struct* insr = (InspectResponse_Struct*) outapp->pBuffer;
		insr->TargetID = inspectedBot->GetNPCTypeID();
		insr->playerid = inspectedBot->GetID();

		const EQ::ItemData* item = nullptr;
		const EQ::ItemInstance* inst = nullptr;

		for (int16 L = EQ::invslot::EQUIPMENT_BEGIN; L <= EQ::invslot::EQUIPMENT_END; L++) {
			inst = inspectedBot->GetBotItem(L);

			if (inst) {
				item = inst->GetItem();
				if (item) {
					strcpy(insr->itemnames[L], item->Name);
					insr->itemicons[L] = item->Icon;
				}
				else {
					insr->itemnames[L][0] = '\0';
					insr->itemicons[L] = 0xFFFFFFFF;
				}
			}
			else {
				insr->itemnames[L][0] = '\0';
				insr->itemicons[L] = 0xFFFFFFFF;
			}
		}

		strcpy(insr->text, inspectedBot->GetInspectMessage().text);

		client->QueuePacket(outapp); // Send answer to requester
		safe_delete(outapp);
	}
}

// This method is intended to call all necessary methods to do all bot stat calculations, including spell buffs, equipment, AA bonsues, etc.
void Bot::CalcBotStats(bool showtext) {
	if (!GetBotOwner())
		return;

	if (showtext) {
		GetBotOwner()->Message(Chat::Yellow, "Updating %s...", GetCleanName());
	}

	// this code is annoying since many classes change their name and illusions change the race id
	/*if (!IsValidRaceClassCombo()) {
		GetBotOwner()->Message(Chat::Yellow, "A %s - %s bot was detected. Is this Race/Class combination allowed?.", GetRaceIDName(GetRace()), GetClassIDName(GetClass(), GetLevel()));
		GetBotOwner()->Message(Chat::Yellow, "Previous Bots Code releases did not check Race/Class combinations during create.");
		GetBotOwner()->Message(Chat::Yellow, "Unless you are experiencing heavy lag, you should delete and remake this bot.");
	}*/

	if (GetBotOwner()->GetLevel() != GetLevel())
		SetLevel(GetBotOwner()->GetLevel());

	for (int sindex = 0; sindex <= EQ::skills::HIGHEST_SKILL; ++sindex) {
		skills[sindex] = SkillCaps::Instance()->GetSkillCap(GetClass(), (EQ::skills::SkillType)sindex, GetLevel()).cap;
	}

	taunt_timer.Start(1000);

	if (GetClass() == Class::Monk && GetLevel() >= 10) {
		monkattack_timer.Start(1000);
	}

	LoadAAs();
	GenerateSpecialAttacks();

	if (showtext) {
		GetBotOwner()->Message(Chat::Yellow, "Base stats:");
		GetBotOwner()->Message(Chat::Yellow, "Level: %i HP: %i AC: %i Mana: %i STR: %i STA: %i DEX: %i AGI: %i INT: %i WIS: %i CHA: %i", GetLevel(), base_hp, AC, max_mana, STR, STA, DEX, AGI, INT, WIS, CHA);
		GetBotOwner()->Message(Chat::Yellow, "Resists-- Magic: %i, Poison: %i, Fire: %i, Cold: %i, Disease: %i, Corruption: %i.",MR,PR,FR,CR,DR,Corrup);
		// Test Code
		if (GetClass() == Class::Bard)
			GetBotOwner()->Message(Chat::Yellow, "Bard Skills-- Brass: %i, Percussion: %i, Singing: %i, Stringed: %i, Wind: %i",
			GetSkill(EQ::skills::SkillBrassInstruments), GetSkill(EQ::skills::SkillPercussionInstruments), GetSkill(EQ::skills::SkillSinging), GetSkill(EQ::skills::SkillStringedInstruments), GetSkill(EQ::skills::SkillWindInstruments));
	}

	//if (Save())
	//	GetBotOwner()->CastToClient()->Message(Chat::White, "%s saved.", GetCleanName());
	//else
	//	GetBotOwner()->CastToClient()->Message(Chat::White, "%s save failed!", GetCleanName());

	CalcBonuses();

	if (showtext) {
		GetBotOwner()->Message(Chat::Yellow, "%s has been updated.", GetCleanName());
		GetBotOwner()->Message(Chat::Yellow, "Level: %i HP: %i AC: %i Mana: %i STR: %i STA: %i DEX: %i AGI: %i INT: %i WIS: %i CHA: %i", GetLevel(), max_hp, GetAC(), max_mana, GetSTR(), GetSTA(), GetDEX(), GetAGI(), GetINT(), GetWIS(), GetCHA());
		GetBotOwner()->Message(Chat::Yellow, "Resists-- Magic: %i, Poison: %i, Fire: %i, Cold: %i, Disease: %i, Corruption: %i.",GetMR(),GetPR(),GetFR(),GetCR(),GetDR(),GetCorrup());
		// Test Code
		if (GetClass() == Class::Bard) {
			GetBotOwner()->Message(Chat::Yellow, "Bard Skills-- Brass: %i, Percussion: %i, Singing: %i, Stringed: %i, Wind: %i",
				GetSkill(EQ::skills::SkillBrassInstruments) + GetBrassMod(),
				GetSkill(EQ::skills::SkillPercussionInstruments) + GetPercMod(),
				GetSkill(EQ::skills::SkillSinging) + GetSingMod(),
				GetSkill(EQ::skills::SkillStringedInstruments) + GetStringMod(),
				GetSkill(EQ::skills::SkillWindInstruments) + GetWindMod());
			GetBotOwner()->Message(Chat::Yellow, "Bard Skill Mods-- Brass: %i, Percussion: %i, Singing: %i, Stringed: %i, Wind: %i", GetBrassMod(), GetPercMod(), GetSingMod(), GetStringMod(), GetWindMod());
		}
	}
}

bool Bot::CheckLoreConflict(const EQ::ItemData* item) {
	if (!item || !(item->LoreFlag) || (item->LoreGroup == 0)) {
		return false;
	}

	if (item->LoreGroup == -1)	// Standard lore items; look everywhere except the shared bank, return the result
		return (m_inv.HasItem(item->ID, 0, invWhereWorn) != INVALID_INDEX);

	//If the item has a lore group, we check for other items with the same group and return the result
	return (m_inv.HasItemByLoreGroup(item->LoreGroup, invWhereWorn) != INVALID_INDEX);
}

bool Bot::AttemptCloseBeneficialSpells(uint16 spell_type) {
	bool result = false;
	Mob* tar = nullptr;

	for (Mob* m : GetSpellTargetList(RuleB(Bots, CrossRaidBuffingAndHealing))) {
		tar = m;

		if (!tar) {
			continue;
		}

		if (RuleB(Bots, CrossRaidBuffingAndHealing) && IsGroupTargetOnlyBotSpellType(spell_type)) {
			Raid* raid = GetStoredRaid();

			if (raid &&
				(raid->GetGroup(GetName()) == raid->GetGroup(tar->GetName()))
			) {
				continue;
			}
		}

		result = AttemptAICastSpell(spell_type, tar);

		if (!result) {
			if (tar->HasPet()) {
				Mob* pet = m->GetPet();

				if (!pet->IsFamiliar() || RuleB(Bots, AllowBuffingHealingFamiliars)) {
					tar = pet;

					if (!tar) {
						continue;
					}

					if (tar->IsOfClientBot() ||
						(
							tar->IsPet() &&
							tar->GetOwner() &&
							tar->GetOwner()->IsOfClientBot()
						)
					) {
						result = AttemptAICastSpell(spell_type, tar);
					}
				}
			}
		}

		if (result) {
			break;
		}
	}

	return result;
}

Mob* EntityList::GetMobByBotID(uint32 botID) {
	Mob* Result = nullptr;
	if (botID > 0) {
		for (const auto& m: mob_list) {
			if (!m.second)
				continue;

			if (m.second->IsBot() && m.second->CastToBot()->GetBotID() == botID) {
				Result = m.second;
				break;
			}
		}
	}
	return Result;
}

Bot* EntityList::GetBotByBotID(uint32 botID) {
	auto it = bot_list.begin();
	while (it != bot_list.end()) {
		if (it->second->GetBotID() == botID)
			return it->second;
		++it;
	}
	return nullptr;
}

Bot* EntityList::GetBotByBotName(std::string botName) {
	for (const auto& e : bot_list) {
		if (e.second && Strings::EqualFold(e.second->GetName(), botName)) {
			return e.second;
		}
	}

	return nullptr;
}

Client* EntityList::GetBotOwnerByBotEntityID(uint32 entity_id) {
	if (entity_id) {
		auto it = bot_list.begin();
		while (it != bot_list.end()) {
			if (it->second->GetID() == entity_id)
				return it->second->GetBotOwner()->CastToClient();
			++it;
		}
	}
	return nullptr;
}

Client* EntityList::GetBotOwnerByBotID(const uint32 bot_id)  {
	if (bot_id) {
		auto it = bot_list.begin();
		while (it != bot_list.end()) {
			if (it->second->GetBotID() == bot_id)
				return it->second->GetBotOwner()->CastToClient();
			++it;
		}
	}
	return nullptr;
}

void EntityList::AddBot(Bot *new_bot, bool send_spawn_packet, bool dont_queue) {
	if (new_bot) {
		new_bot->SetID(GetFreeID());
		bot_list.emplace(std::pair<uint16, Bot*>(new_bot->GetID(), new_bot));
		mob_list.emplace(std::pair<uint16, Mob*>(new_bot->GetID(), new_bot));

		if (parse->BotHasQuestSub(EVENT_SPAWN)) {
			parse->EventBot(EVENT_SPAWN, new_bot, nullptr, "", 0);
		}

		new_bot->SetSpawned();
		if (send_spawn_packet) {
			if (dont_queue) {
				EQApplicationPacket* outapp = new EQApplicationPacket();
				new_bot->CreateSpawnPacket(outapp);
				outapp->priority = 6;
				QueueClients(new_bot, outapp, true);
				safe_delete(outapp);
			} else {
				NewSpawn_Struct* ns = new NewSpawn_Struct;
				memset(ns, 0, sizeof(NewSpawn_Struct));
				new_bot->FillSpawnStruct(ns, new_bot);
				AddToSpawnQueue(new_bot->GetID(), &ns);
				safe_delete(ns);
			}
		}

		if (parse->HasQuestSub(ZONE_CONTROLLER_NPC_ID, EVENT_SPAWN_ZONE)) {
			new_bot->DispatchZoneControllerEvent(EVENT_SPAWN_ZONE, new_bot, "", 0, nullptr);
		}
	}
}

std::vector<Bot*> EntityList::GetBotsByBotOwnerCharacterID(uint32 bot_owner_character_id) {
	std::vector<Bot*> client_bot_list;

	if (bot_owner_character_id <= 0) {
		return client_bot_list;
	}

	auto it = bot_list.begin();

	while (it != bot_list.end()) {
		if (it->second->GetOwner() && it->second->GetBotOwnerCharacterID() == bot_owner_character_id) {
			client_bot_list.push_back(it->second);
		}
		++it;
	}

	return client_bot_list;
}

bool EntityList::RemoveBot(uint16 entityID) {
	auto it = bot_list.find(entityID);
	if (it != bot_list.end()) {
		bot_list.erase(it); // Already deleted
		return true;
	}
	return false;
}

void EntityList::ShowSpawnWindow(Client* client, int Distance, bool NamedOnly) {
	const char *WindowTitle = "Bot Tracking Window";
	std::string WindowText;
	int LastCon = -1;
	int CurrentCon = 0;
	Mob* curMob = nullptr;
	uint32 array_counter = 0;

	for (const auto& m : mob_list) {
	curMob = m.second;
		if (curMob && DistanceNoZ(curMob->GetPosition(), client->GetPosition()) <= Distance) {
			if (curMob->IsTrackable()) {
				Mob* cur_entity = curMob;
				int Extras = (cur_entity->IsBot() || cur_entity->IsPet() || cur_entity->IsFamiliar() || cur_entity->IsClient());
				const char *const MyArray[] = {
					"a_","an_","Innkeep_","Barkeep_",
					"Guard_","Merchant_","Lieutenant_",
					"Banker_","Centaur_","Aviak_","Baker_",
					"Sir_","Armorer_","Deathfist_","Deputy_",
					"Sentry_","Sentinel_","Leatherfoot_",
					"Corporal_","goblin_","Bouncer_","Captain_",
					"orc_","fire_","inferno_","young_","cinder_",
					"flame_","gnomish_","CWG_","sonic_","greater_",
					"ice_","dry_","Priest_","dark-boned_",
					"Tentacle_","Basher_","Dar_","Greenblood_",
					"clockwork_","guide_","rogue_","minotaur_",
					"brownie_","Teir'","dark_","tormented_",
					"mortuary_","lesser_","giant_","infected_",
					"wharf_","Apprentice_","Scout_","Recruit_",
					"Spiritist_","Pit_","Royal_","scalebone_",
					"carrion_","Crusader_","Trooper_","hunter_",
					"decaying_","iksar_","klok_","templar_","lord_",
					"froglok_","war_","large_","charbone_","icebone_",
					"Vicar_","Cavalier_","Heretic_","Reaver_","venomous_",
					"Sheildbearer_","pond_","mountain_","plaguebone_","Brother_",
					"great_","strathbone_","briarweb_","strathbone_","skeletal_",
					"minion_","spectral_","myconid_","spurbone_","sabretooth_",
					"Tin_","Iron_","Erollisi_","Petrifier_","Burynai_",
					"undead_","decayed_","You_","smoldering_","gyrating_",
					"lumpy_","Marshal_","Sheriff_","Chief_","Risen_",
					"lascar_","tribal_","fungi_","Xi_","Legionnaire_",
					"Centurion_","Zun_","Diabo_","Scribe_","Defender_","Capt_",
					"blazing_","Solusek_","imp_","hexbone_","elementalbone_",
					"stone_","lava_","_",""
				};
				unsigned int MyArraySize;
				for ( MyArraySize = 0; true; MyArraySize++) {
					if (!(*(MyArray[MyArraySize])))
						break;
				}
				if (NamedOnly) {
					bool ContinueFlag = false;
					const char *CurEntityName = cur_entity->GetName();
					for (int Index = 0; Index < MyArraySize; Index++) {
						if (!strncasecmp(CurEntityName, MyArray[Index], strlen(MyArray[Index])) || (Extras)) {
							ContinueFlag = true;
							break;
						}
					}
					if (ContinueFlag)
						continue;
				}

				CurrentCon = client->GetLevelCon(cur_entity->GetLevel());
				if (CurrentCon != LastCon) {
					if (LastCon != -1)
						WindowText += "</c>";

					LastCon = CurrentCon;
					switch(CurrentCon) {
						case ConsiderColor::Green: {
							WindowText += "<c \"#00FF00\">";
							break;
						}
						case ConsiderColor::LightBlue: {
							WindowText += "<c \"#8080FF\">";
							break;
						}
						case ConsiderColor::DarkBlue: {
							WindowText += "<c \"#2020FF\">";
							break;
						}
						case ConsiderColor::Yellow: {
							WindowText += "<c \"#FFFF00\">";
							break;
						}
						case ConsiderColor::Red: {
							WindowText += "<c \"#FF0000\">";
							break;
						}
						default: {
							WindowText += "<c \"#FFFFFF\">";
							break;
						}
					}
				}
				WindowText += cur_entity->GetCleanName();
				WindowText += "<br>";
				if (strlen(WindowText.c_str()) > 4000) {
					WindowText += "</c><br><br>List truncated... too many mobs to display";
					break;
				}
			}
		}
	}
	WindowText += "</c>";
	client->SendPopupToClient(WindowTitle, WindowText.c_str());
	return;
}

uint8 Bot::GetNumberNeedingHealedInGroup(Mob* tar, uint16 spell_type, uint16 spell_id, float range) {
	if (!TargetValidation(tar)) {
		return 0;
	}

	uint8 count = 0;
	std::vector<Mob*> target_list = tar->IsClient() ? GatherSpellTargets(false, tar) : tar->CastToBot()->GetSpellTargetList();

	for (auto* m : target_list) {
		if (
			m &&
			entity_list.IsMobInZone(m) &&
			TargetValidation(m) &&
			tar->CalculateDistance(m) < range &&
			CastChecks(spell_id, m, spell_type, true, IsGroupBotSpellType(spell_type))
		) {
			++count;
		}
	}

	return count;
}

int Bot::GetRawACNoShield(int &shield_ac) {
	int ac = itembonuses.AC + spellbonuses.AC;
	shield_ac = 0;
	EQ::ItemInstance* inst = GetBotItem(EQ::invslot::slotSecondary);
	if (inst) {
		if (inst->GetItem()->ItemType == EQ::item::ItemTypeShield) {
			ac -= inst->GetItem()->AC;
			shield_ac = inst->GetItem()->AC;
			for (uint8 i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; i++) {
				if (inst->GetAugment(i)) {
					ac -= inst->GetAugment(i)->GetItem()->AC;
					shield_ac += inst->GetAugment(i)->GetItem()->AC;
				}
			}
		}
	}
	return ac;
}

uint32 Bot::CalcCurrentWeight() {
	const EQ::ItemData* TempItem = nullptr;
	EQ::ItemInstance* inst = nullptr;
	uint32 Total = 0;
	for (int i = EQ::invslot::EQUIPMENT_BEGIN; i <= EQ::invslot::EQUIPMENT_END; ++i) {
		inst = GetBotItem(i);
		if (inst) {
			TempItem = inst->GetItem();
			if (TempItem)
				Total += TempItem->Weight;
		}
	}

	float Packrat = ((float)spellbonuses.Packrat + (float)aabonuses.Packrat);
	if (Packrat > 0)
		Total = (uint32)((float)Total * (1.0f - ((Packrat * 1.0f) / 100.0f)));

	return Total;
}

int Bot::GroupLeadershipAAHealthEnhancement() {
	Group *g = GetGroup();

	if (!g || (g->GroupCount() < 3))
		return 0;

	switch(g->GetLeadershipAA(groupAAHealthEnhancement)) {
		case 0:
			return 0;
		case 1:
			return 30;
		case 2:
			return 60;
		case 3:
			return 100;
	}
	return 0;
}

int Bot::GroupLeadershipAAManaEnhancement() {
	Group *g = GetGroup();
	if (!g || (g->GroupCount() < 3))
		return 0;

	switch(g->GetLeadershipAA(groupAAManaEnhancement)) {
		case 0:
			return 0;
		case 1:
			return 30;
		case 2:
			return 60;
		case 3:
			return 100;
	}
	return 0;
}

int Bot::GroupLeadershipAAHealthRegeneration() {
	Group *g = GetGroup();
	if (!g || (g->GroupCount() < 3))
		return 0;

	switch(g->GetLeadershipAA(groupAAHealthRegeneration)) {
		case 0:
			return 0;
		case 1:
			return 4;
		case 2:
			return 6;
		case 3:
			return 8;
	}

	return 0;
}

int Bot::GroupLeadershipAAOffenseEnhancement() {
	Group *g = GetGroup();

	if (!g || (g->GroupCount() < 3))
		return 0;

	switch(g->GetLeadershipAA(groupAAOffenseEnhancement)) {
		case 0:
			return 0;
		case 1:
			return 10;
		case 2:
			return 19;
		case 3:
			return 28;
		case 4:
			return 34;
		case 5:
			return 40;
	}
	return 0;
}

bool Bot::GetNeedsCured(Mob *tar) {
	bool need_cured = false;

	if (tar) {
		if (tar->FindType(SpellEffect::PoisonCounter) || tar->FindType(SpellEffect::DiseaseCounter) || tar->FindType(SpellEffect::CurseCounter) || tar->FindType(SpellEffect::CorruptionCounter)) {
			uint32 buff_count = tar->GetMaxTotalSlots();

			for (unsigned int j = 0; j < buff_count; j++) {
				if (IsValidSpell(tar->GetBuffs()[j].spellid)) {
					if (CalculateCounters(tar->GetBuffs()[j].spellid) > 0) {
						if (tar->GetBuffs()[j].ticsremaining < 1) {
							continue;
						}

						need_cured = true;
					}
				}
			}
		}
	}

	return need_cured;
}

bool Bot::GetNeedsHateRedux(Mob *tar) {
	if (!tar || !tar->IsEngaged() || !tar->HasTargetReflection() || !tar->GetTarget()->IsNPC() || (tar->IsBot() && tar->CastToBot()->IsTaunting())) {
		return false;
	}

	if (tar->IsBot()) {
		if (tar->GetHPRatio() > GetUltimateSpellTypeMinThreshold(BotSpellTypes::HateRedux, tar) && tar->GetHPRatio() <
																									   GetUltimateSpellTypeMaxThreshold(
																										   BotSpellTypes::HateRedux,
																										   tar
																									   )) {
			return true;
		}
	}

	return false;
}

bool Bot::HasOrMayGetAggro(bool sit_aggro, uint32 spell_id) {
    if (
        !GetTarget() ||
        !GetTarget()->GetHateTop()
    ) {
        return false;
    }

    Mob* top_hate = GetTarget()->GetHateTop();

    if (top_hate == this) {
        return true;
    }

    uint32 my_hate = GetTarget()->GetHateAmount(this);
    uint32 top_hate_amt = GetTarget()->GetHateAmount(top_hate);

    if (sit_aggro && !spell_id) {
        my_hate *= (1 + (RuleI(Aggro, SittingAggroMod) / 100));
    }

    if (spell_id && IsValidSpell(spell_id)) {
        my_hate += CheckAggroAmount(spell_id, GetTarget());
    }

    if (
        top_hate_amt < 1 ||
        (
            my_hate > 0 &&
            (my_hate * 100 / top_hate_amt) > RuleI(Bots, HasOrMayGetAggroThreshold)
        )
    ) {
        return true;
    }

    return false;
}

void Bot::SetDefaultBotStance() {
	_botStance = GetClass() == Class::Warrior ? Stance::Aggressive : Stance::Balanced;
}

void Bot::RaidGroupSay(const char* msg, ...) {
	if (
		GetTempSpellType() != UINT16_MAX &&
		!GetSpellTypeAnnounceCast(GetTempSpellType())
	) {
		return;
	}

	char buf[1000];
	va_list ap;
	va_start(ap, msg);
	vsnprintf(buf, 1000, msg, ap);
	va_end(ap);

	if (IsRaidGrouped()) {
		Raid* r = GetStoredRaid() ? GetStoredRaid() : GetRaid();

		if (r) {
			for (const auto& m : r->members) {
				if (m.member && m.member->IsClient()) {
					m.member->FilteredMessageString(this,Chat::PetResponse,FilterSocials,GENERIC_SAY,GetCleanName(),buf);
				}
			}
		}
	}
	else if (HasGroup()) {
			for (auto& m : GetGroup()->members) {
				if (m && m->IsClient()) {
					m->FilteredMessageString(this,Chat::PetResponse,FilterSocials,GENERIC_SAY,GetCleanName(),buf);
				}
			}
	}
	else if (GetOwner()) {
		GetOwner()->FilteredMessageString(this,Chat::PetResponse,FilterSocials,GENERIC_SAY,GetCleanName(),buf);
	}
}

bool Bot::UseDiscipline(uint32 spell_id, uint32 target) {
	if (!IsValidSpell(spell_id)) {
		RaidGroupSay("Not a valid spell.");
		return false;
	}

	const SPDat_Spell_Struct &spell = spells[spell_id];
	uint8 level_to_use = spell.classes[GetClass() - 1];
	if (level_to_use == 255 || level_to_use > GetLevel()) {
		return false;
	}

	if (GetEndurance() > spell.endurance_cost)
		SetEndurance(GetEndurance() - spell.endurance_cost);
	else
		return false;

	if (spell.recast_time > 0) {
		if (CheckDisciplineReuseTimer(spell_id)) {
			if (spells[spell_id].timer_id > 0) {
				SetDisciplineReuseTimer(spell_id);
			}
		} else {
			uint32 remaining_time = (GetDisciplineReuseRemainingTime(spell_id) / 1000);
			OwnerMessage(
				fmt::format(
					"I can use this discipline in {}.",
					Strings::SecondsToTime(remaining_time)
				)
			);
			return false;
		}
	}

	if (IsCasting())
		InterruptSpell();

	CastSpell(spell_id, target, EQ::spells::CastingSlot::Discipline);
	return true;
}

// new healrotation code
bool Bot::CreateHealRotation(uint32 interval_ms, bool fast_heals, bool adaptive_targeting, bool casting_override)
{
	if (IsHealRotationMember())
		return false;
	if (!IsHealRotationMemberClass(GetClass()))
		return false;

	m_member_of_heal_rotation = std::make_shared<HealRotation>(this, interval_ms, fast_heals, adaptive_targeting, casting_override);

	return IsHealRotationMember();
}

bool Bot::DestroyHealRotation()
{
	if (!IsHealRotationMember())
		return true;

	m_member_of_heal_rotation->ClearTargetPool();
	m_member_of_heal_rotation->ClearMemberPool();

	return !IsHealRotationMember();
}

bool Bot::JoinHealRotationMemberPool(std::shared_ptr<HealRotation>* heal_rotation)
{
	if (IsHealRotationMember())
		return false;
	if (!heal_rotation->use_count())
		return false;
	if (!(*heal_rotation))
		return false;
	if (!IsHealRotationMemberClass(GetClass()))
		return false;

	if (!(*heal_rotation)->AddMemberToPool(this))
		return false;

	m_member_of_heal_rotation = *heal_rotation;

	return true;
}

bool Bot::LeaveHealRotationMemberPool()
{
	if (!IsHealRotationMember()) {
		m_member_of_heal_rotation.reset();
		return true;
	}

	m_member_of_heal_rotation->RemoveMemberFromPool(this);
	m_member_of_heal_rotation.reset();

	return !IsHealRotationMember();
}

bool Bot::UseHealRotationFastHeals()
{
	if (!IsHealRotationMember())
		return false;

	return m_member_of_heal_rotation->FastHeals();
}

bool Bot::UseHealRotationAdaptiveTargeting()
{
	if (!IsHealRotationMember())
		return false;

	return m_member_of_heal_rotation->AdaptiveTargeting();
}

bool Bot::IsHealRotationActive()
{
	if (!IsHealRotationMember())
		return false;

	return m_member_of_heal_rotation->IsActive();
}

bool Bot::IsHealRotationReady()
{
	if (!IsHealRotationMember())
		return false;

	return m_member_of_heal_rotation->CastingReady();
}

bool Bot::IsHealRotationCaster()
{
	if (!IsHealRotationMember())
		return false;

	return (m_member_of_heal_rotation->CastingMember() == this);
}

bool Bot::HealRotationPokeTarget()
{
	if (!IsHealRotationMember())
		return false;

	return m_member_of_heal_rotation->PokeCastingTarget();
}

Mob* Bot::HealRotationTarget()
{
	if (!IsHealRotationMember())
		return nullptr;

	return m_member_of_heal_rotation->CastingTarget();
}

bool Bot::AdvanceHealRotation(bool use_interval)
{
	if (!IsHealRotationMember())
		return false;

	return m_member_of_heal_rotation->AdvanceRotation(use_interval);
}

bool Bot::IsMyHealRotationSet()
{
	if (!IsHealRotationMember())
		return false;
	if (!m_member_of_heal_rotation->IsActive() && !m_member_of_heal_rotation->IsHOTActive())
		return false;
	if (!m_member_of_heal_rotation->CastingReady())
		return false;
	if (m_member_of_heal_rotation->CastingMember() != this)
		return false;
	if (m_member_of_heal_rotation->MemberIsCasting(this))
		return false;
	if (!m_member_of_heal_rotation->PokeCastingTarget())
		return false;

	return true;
}

bool Bot::AmICastingForHealRotation()
{
	if (!IsHealRotationMember())
		return false;

	return m_member_of_heal_rotation->MemberIsCasting(this);
}

void Bot::SetMyCastingForHealRotation(bool flag)
{
	if (!IsHealRotationMember())
		return;

	m_member_of_heal_rotation->SetMemberIsCasting(this, flag);
}

bool Bot::DyeArmor(int16 slot_id, uint32 rgb, bool all_flag, bool save_flag)
{
	if (all_flag) {
		if (slot_id != INVALID_INDEX)
			return false;

		for (uint8 i = EQ::textures::textureBegin; i < EQ::textures::weaponPrimary; ++i) {
			uint8 inv_slot = EQ::InventoryProfile::CalcSlotFromMaterial(i);
			EQ::ItemInstance* inst = m_inv.GetItem(inv_slot);
			if (!inst)
				continue;

			inst->SetColor(rgb);
			SendWearChange(i);
		}
	}
	else {
		uint8 mat_slot = EQ::InventoryProfile::CalcMaterialFromSlot(slot_id);
		if (mat_slot == EQ::textures::materialInvalid || mat_slot >= EQ::textures::weaponPrimary)
			return false;

		EQ::ItemInstance* inst = m_inv.GetItem(slot_id);
		if (!inst)
			return false;

		inst->SetColor(rgb);
		SendWearChange(mat_slot);
	}

	if (save_flag) {
		int16 save_slot = slot_id;
		if (all_flag)
			save_slot = -2;

		if (!database.botdb.SaveEquipmentColor(GetBotID(), save_slot, rgb)) {
			return false;
		}
	}

	return true;
}

void Bot::Signal(int signal_id)
{
	if (parse->BotHasQuestSub(EVENT_SIGNAL)) {
		parse->EventBot(EVENT_SIGNAL, this, nullptr, std::to_string(signal_id), 0);
	}
}

void Bot::SendPayload(int payload_id, std::string payload_value)
{
	if (parse->BotHasQuestSub(EVENT_PAYLOAD)) {
		const auto& export_string = fmt::format("{} {}", payload_id, payload_value);

		parse->EventBot(EVENT_PAYLOAD, this, nullptr, export_string, 0);
	}
}

void Bot::OwnerMessage(const std::string& message)
{
	if (!GetBotOwner() || !GetBotOwner()->IsClient()) {
		return;
	}

	GetBotOwner()->Message(
		Chat::Tell,
		fmt::format(
			"{} tells you, '{}'",
			GetCleanName(),
			message
		).c_str()
	);
}

bool Bot::CheckDataBucket(std::string bucket_name, const std::string& bucket_value, uint8 bucket_comparison)
{
	if (!bucket_name.empty() && !bucket_value.empty()) {
		// try to fetch from bot first
		DataBucketKey k = GetScopedBucketKeys();
		k.key = bucket_name;

		auto b = DataBucket::GetData(&database, k);
		if (b.value.empty() && GetBotOwner()) {
			// fetch from owner
			k = GetBotOwner()->GetScopedBucketKeys();
			k.key = bucket_name;

			b = DataBucket::GetData(&database, k);
			if (b.value.empty()) {
				return false;
			}
		}

		if (zone->CompareDataBucket(bucket_comparison, bucket_value, b.value)) {
			return true;
		}
	}

	return false;
}

int Bot::GetExpansionBitmask()
{
	if (_expansionBitmask >= 0) {
		return _expansionBitmask;
	}

	return RuleI(Bots, BotExpansionSettings);
}

void Bot::SetExpansionBitmask(int expansionBitmask)
{
	_expansionBitmask = expansionBitmask;

	LoadAAs();
}

void Bot::SetBotEnforceSpellSetting(bool enforceSpellSettings)
{
	_enforceSpellSettings = enforceSpellSettings;

	LoadBotSpellSettings();
	AI_AddBotSpells(GetBotSpellID());
}

bool Bot::AddBotSpellSetting(uint16 spell_id, BotSpellSetting* bs)
{
	if (!IsValidSpell(spell_id) || !bs) {
		return false;
	}

	auto obs = GetBotSpellSetting(spell_id);
	if (obs) {
		return false;
	}

	auto s = BotSpellSettingsRepository::NewEntity();

	s.spell_id = spell_id;
	s.bot_id = GetBotID();

	s.priority = bs->priority;
	s.min_hp = bs->min_hp;
	s.max_hp = bs->max_hp;
	s.is_enabled = bs->is_enabled;

	const auto& nbs = BotSpellSettingsRepository::InsertOne(content_db, s);
	if (!nbs.id) {
		return false;
	}

	LoadBotSpellSettings();
	return true;
}

bool Bot::DeleteBotSpellSetting(uint16 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	auto bs = GetBotSpellSetting(spell_id);
	if (!bs) {
		return false;
	}

	BotSpellSettingsRepository::DeleteWhere(
		content_db,
		fmt::format(
			"bot_id = {} AND spell_id = {}",
			GetBotID(),
			spell_id
		)
	);
	LoadBotSpellSettings();
	return true;
}

BotSpellSetting* Bot::GetBotSpellSetting(uint16 spell_id)
{
	if (!IsValidSpell(spell_id) || !bot_spell_settings.count(spell_id)) {
		return nullptr;
	}

	auto b = bot_spell_settings.find(spell_id);
	if (b != bot_spell_settings.end()) {
		return &b->second;
	}

	return nullptr;
}

void Bot::ListBotSpells(uint8 min_level)
{
	auto bot_owner = GetBotOwner();
	if (!bot_owner) {
		return;
	}

	if (AIBot_spells.empty() && AIBot_spells_enforced.empty()) {
		bot_owner->Message(
			Chat::White,
			fmt::format(
				"{} has no AI Spells.",
				GetCleanName()
			).c_str()
		);
		return;
	}

	auto spell_count = 0;
	auto spell_number = 1;

	for (const auto& s : (GetBotEnforceSpellSetting()) ? AIBot_spells_enforced : AIBot_spells) {
		auto b = bot_spell_settings.find(s.spellid);
		if (b == bot_spell_settings.end() && s.minlevel >= min_level) {
			bot_owner->Message(
				Chat::White,
				fmt::format(
					"Spell {} | Spell: {} (ID: {}) | Add Spell: {}",
					spell_number,
					Saylink::Silent(
						fmt::format("^spellinfo {}", s.spellid),
						spells[s.spellid].name
					),
					s.spellid,
					Saylink::Silent(
						fmt::format("^spellsettingsadd {} {} {} {}", s.spellid, s.priority, s.min_hp, s.max_hp), "Add")
				).c_str()
			);

			spell_count++;
			spell_number++;
		}
	}

	bot_owner->Message(
		Chat::White,
		fmt::format(
			"{} has {} AI Spell{}.",
			GetCleanName(),
			spell_count,
			spell_count != 1 ? "s" :""
		).c_str()
	);
}

void Bot::ListBotSpellSettings()
{
	auto bot_owner = GetBotOwner();
	if (!bot_owner) {
		return;
	}

	if (!bot_spell_settings.size()) {
		bot_owner->Message(
			Chat::White,
			fmt::format(
				"{} does not have any spell settings.",
				GetCleanName()
			).c_str()
		);
		return;
	}

	auto setting_count = 0;
	auto setting_number = 1;

	for (const auto& bs : bot_spell_settings) {
		bot_owner->Message(
			Chat::White,
			fmt::format(
				"Setting {} | Spell: {} | State: {} | {}",
				setting_number,
				Saylink::Silent(fmt::format("^spellinfo {}", bs.first), spells[bs.first].name),
				Saylink::Silent(
					fmt::format("^spellsettingstoggle {} {}",
					bs.first, bs.second.is_enabled ? "False" : "True"),
					bs.second.is_enabled ? "Enabled" : "Disabled"
				),
				Saylink::Silent(fmt::format("^spellsettingsdelete {}", bs.first), "Remove")
			).c_str()
		);

		setting_count++;
		setting_number++;
	}

	bot_owner->Message(
		Chat::White,
		fmt::format(
			"{} has {} spell setting{}.",
			GetCleanName(),
			setting_count,
			setting_count != 1 ? "s" : ""
		).c_str()
	);
}

void Bot::LoadBotSpellSettings()
{
	bot_spell_settings.clear();

	auto s = BotSpellSettingsRepository::GetWhere(content_db, fmt::format("bot_id = {}", GetBotID()));
	if (s.empty()) {
		return;
	}

	for (const auto& e : s) {
		BotSpellSetting b;

		b.priority = e.priority;
		b.min_hp = e.min_hp;
		b.max_hp = e.max_hp;
		b.is_enabled = e.is_enabled;
		bot_spell_settings[e.spell_id] = b;
	}
}

bool Bot::UpdateBotSpellSetting(uint16 spell_id, BotSpellSetting* bs)
{
	if (!IsValidSpell(spell_id) || !bs) {
		return false;
	}

	auto s = BotSpellSettingsRepository::NewEntity();

	s.spell_id = spell_id;
	s.bot_id = GetBotID();
	s.priority = bs->priority;
	s.min_hp = bs->min_hp;
	s.max_hp = bs->max_hp;
	s.is_enabled = bs->is_enabled;

	auto obs = GetBotSpellSetting(spell_id);
	if (!obs) {
		return false;
	}

	if (!BotSpellSettingsRepository::UpdateSpellSetting(content_db, s)) {
		return false;
	}

	LoadBotSpellSettings();
	return true;
}

std::string Bot::GetHPString(int8 min_hp, int8 max_hp)
{
	std::string hp_string = "Any";
	if (min_hp && max_hp) {
		hp_string = fmt::format(
			"{}%% to {}%%",
			min_hp,
			max_hp
		);
	} else if (min_hp && !max_hp) {
		hp_string = fmt::format(
			"{}%% to 100%%",
			min_hp
		);
	} else if (!min_hp && max_hp) {
		hp_string = fmt::format(
			"1%% to {}%%",
			max_hp
		);
	}

	return hp_string;
}

std::vector<Mob*> Bot::GetApplySpellList(
	ApplySpellType apply_type,
	bool allow_pets,
	bool is_raid_group_only
) {
	std::vector<Mob*> l;

	if (apply_type == ApplySpellType::Raid && IsRaidGrouped()) {
		auto* r = GetRaid();
		if (r) {
			auto group_id = r->GetGroup(GetCleanName());
			if (EQ::ValueWithin(group_id, 0, (MAX_RAID_GROUPS - 1))) {
				for (const auto& m: r->members) {
					if (m.is_bot) {
						continue;
					}
					if (m.member && m.member->IsClient() &&
						(!is_raid_group_only || r->GetGroup(m.member) == group_id)) {
						l.push_back(m.member);

						if (allow_pets && m.member->HasPet()) {
							l.push_back(m.member->GetPet());
						}

						const auto& sbl = entity_list.GetBotListByCharacterID(m.member->CharacterID());
						for (const auto& b: sbl) {
							l.push_back(b);
						}
					}
				}
			}
		}
	} else if (apply_type == ApplySpellType::Group && IsGrouped()) {
		auto* g = GetGroup();
		if (g) {
			for (auto i = 0; i < MAX_GROUP_MEMBERS; i++) {
				auto* m = g->members[i];
				if (m && m->IsClient()) {
					l.push_back(m->CastToClient());

					if (allow_pets && m->HasPet()) {
						l.push_back(m->GetPet());
					}
					const auto& sbl = entity_list.GetBotListByCharacterID(m->CastToClient()->CharacterID());
					for (const auto& b : sbl) {
						l.push_back(b);
					}
				}
			}
		}
	} else {
		l.push_back(this);

		if (allow_pets && HasPet()) {
			l.push_back(GetPet());
		}
		const auto& sbl = entity_list.GetBotListByCharacterID(CharacterID());
		for (const auto& b : sbl) {
			l.push_back(b);
		}
	}

	return l;
}

void Bot::ApplySpell(
	int spell_id,
	int duration,
	int level,
	ApplySpellType apply_type,
	bool allow_pets,
	bool is_raid_group_only
) {
	const auto& l = GetApplySpellList(apply_type, allow_pets, is_raid_group_only);

	for (const auto& m : l) {
		m->ApplySpellBuff(spell_id, duration, level);
	}
}

void Bot::SetSpellDuration(
	int spell_id,
	int duration,
	int level,
	ApplySpellType apply_type,
	bool allow_pets,
	bool is_raid_group_only
) {
	const auto& l = GetApplySpellList(apply_type, allow_pets, is_raid_group_only);

	for (const auto& m : l) {
		m->SetBuffDuration(spell_id, duration, level);
	}
}

void Bot::Escape()
{
	entity_list.RemoveFromTargets(this, true);
	SetInvisible(Invisibility::Invisible);
}

void Bot::Fling(float value, float target_x, float target_y, float target_z, bool ignore_los, bool clip_through_walls, bool calculate_speed) {
	BuffFadeByEffect(SpellEffect::Levitate);
	if (CheckLosFN(target_x, target_y, target_z, 6.0f) || ignore_los) {
		auto p = new EQApplicationPacket(OP_Fling, sizeof(fling_struct));
		auto* f = (fling_struct*) p->pBuffer;

		if (!calculate_speed) {
			f->speed_z = value;
		} else {
			auto speed = 1.0f;
			const auto distance = CalculateDistance(target_x, target_y, target_z);

			auto z_diff = target_z - GetZ();
			if (z_diff != 0.0f) {
				speed += std::abs(z_diff) / 12.0f;
			}

			speed += distance / 200.0f;

			speed++;

			speed = std::abs(speed);

			f->speed_z = speed;
		}

		f->collision = clip_through_walls ? 0 : -1;
		f->travel_time = -1;
		f->unk3 = 1;
		f->disable_fall_damage = 1;
		f->new_y = target_y;
		f->new_x = target_x;
		f->new_z = target_z;
		p->priority = 6;
		GetBotOwner()->CastToClient()->FastQueuePacket(&p);
	}
}

// This should return the combined AC of all the items the Bot is wearing.
int32 Bot::GetRawItemAC()
{
	int32 Total = 0;
	// this skips MainAmmo..add an '=' conditional if that slot is required (original behavior)
	for (int16 slot_id = EQ::invslot::BONUS_BEGIN; slot_id <= EQ::invslot::BONUS_STAT_END; slot_id++) {
		const EQ::ItemInstance* inst = m_inv[slot_id];
		if (inst && inst->IsClassCommon()) {
			Total += inst->GetItem()->AC;
		}
	}
	return Total;
}

void Bot::SendSpellAnim(uint16 target_id, uint16 spell_id)
{
	if (!target_id || !IsValidSpell(spell_id)) {
		return;
	}

	EQApplicationPacket app(OP_Action, sizeof(Action_Struct));
	auto* a = (Action_Struct*) app.pBuffer;

	a->target      = target_id;
	a->source      = GetID();
	a->type        = 231;
	a->spell       = spell_id;
	a->hit_heading = GetHeading();

	app.priority = 1;
	entity_list.QueueCloseClients(this, &app, false, RuleI(Range, SpellParticles));
}

int32 Bot::CalcItemATKCap()
{
	return RuleI(Character, ItemATKCap) + itembonuses.ItemATKCap + spellbonuses.ItemATKCap + aabonuses.ItemATKCap;
}

bool Bot::CheckCampSpawnConditions(Client* c) {

	if (RuleB(Bots, PreventBotSpawnOnFD) && c->GetFeigned()) {
		c->Message(Chat::White, "You cannot camp or spawn bots while feigned.");

		return false;
	}

	if (RuleB(Bots, PreventBotSpawnOnEngaged)) {
		Raid* raid = entity_list.GetRaidByClient(c);
		if (raid && raid->IsEngaged()) {
			c->Message(Chat::White, "You cannot camp or spawn bots while your raid is engaged.");

			return false;
		}

		auto* owner_group = c->GetGroup();
		if (owner_group) {
			std::list<Client*> member_list;
			owner_group->GetClientList(member_list);
			member_list.remove(nullptr);

			for (auto member_iter : member_list) {
				if (member_iter->IsEngaged() || member_iter->GetAggroCount() > 0) {
					c->Message(Chat::White, "You cannot camp or spawn bots while your group is engaged,");
					return false;
				}
			}
		}
		else {
			if (c->GetAggroCount() > 0) {
				c->Message(Chat::White, "You cannot camp or spawn bots while you are engaged,");
				return false;
			}
		}
	}

	return true;
}

bool Bot::CheckHighEnoughLevelForBots(Client* c, uint8 bot_class) {
	auto bot_character_level = c->GetBotRequiredLevel(bot_class);
	bool not_high_enough_level = bot_character_level >= 0 && c->GetLevel() < bot_character_level;

	if (not_high_enough_level) {
		c->Message(
			Chat::White,
			fmt::format(
				"You must be level {} to spawn {}bots.",
				bot_character_level,
				bot_class ? GetClassIDName(bot_class) : ""
			).c_str()
		);

		return false;
	}

	return true;
}

bool Bot::CheckCreateLimit(Client* c, uint32 bot_count, uint8 bot_class) {
	auto bot_creation_limit = c->GetBotCreationLimit(bot_class);
	bool is_beyond_spawn_limit = bot_creation_limit >= 0 && bot_count >= bot_creation_limit;

	if (is_beyond_spawn_limit) {
		std::string message;

		if (bot_creation_limit) {
			message = fmt::format(
				"You cannot create anymore than {} {}bot{}.",
				bot_creation_limit,
				bot_class ? GetClassIDName(bot_class) : "",
				bot_creation_limit != 1 ? "s" : ""
			);
		} else {
			message = fmt::format(
				"You cannot create any {}bots.",
				bot_class ? GetClassIDName(bot_class) : ""
			);
		}

		c->Message(Chat::Yellow, message.c_str());

		return false;
	}

	return true;
}

bool Bot::CheckSpawnLimit(Client* c, uint8 bot_class) {
	auto bot_spawn_limit = c->GetBotSpawnLimit(bot_class);
	auto spawned_bot_count = Bot::SpawnedBotCount(c->CharacterID(), bot_class);
	bool is_beyond_spawn_limit = bot_spawn_limit >= 0 && spawned_bot_count >= bot_spawn_limit;

	if (is_beyond_spawn_limit) {
		std::string message;

		if (bot_spawn_limit) {
			message = fmt::format(
				"You cannot have more than {} spawned {}bot{}.",
				bot_spawn_limit,
				bot_class ? GetClassIDName(bot_class) : "",
				bot_spawn_limit != 1 ? "s" : ""
			);
		}
		else {
			message = fmt::format(
				"You are not currently allowed to spawn any {}bots.",
				bot_class ? GetClassIDName(bot_class) : ""
			);
		}

		c->Message(Chat::White, message.c_str());

		return false;
	}

	return true;
}

void Bot::AddBotStartingItems(uint16 race_id, uint8 class_id)
{
	if (!IsPlayerRace(race_id) || !IsPlayerClass(class_id)) {
		return;
	}

	const uint16 race_bitmask  = GetPlayerRaceBit(race_id);
	const uint16 class_bitmask = GetPlayerClassBit(class_id);

	const auto& l = BotStartingItemsRepository::GetWhere(
		content_db,
		fmt::format(
			"(races & {} OR races = 0) AND "
			"(classes & {} OR classes = 0) {}",
			race_bitmask,
			class_bitmask,
			ContentFilterCriteria::apply()
		)
	);

	if (l.empty()) {
		return;
	}

	std::vector<BotInventoriesRepository::BotInventories> v;

	for (const auto& e : l) {
		if (
			CanClassEquipItem(e.item_id) &&
			(CanRaceEquipItem(e.item_id) || RuleB(Bots, AllowBotEquipAnyRaceGear))
		) {
			auto i = BotInventoriesRepository::NewEntity();

			i.bot_id       = GetBotID();
			i.slot_id      = e.slot_id;
			i.item_id      = e.item_id;
			i.inst_charges = e.item_charges;
			i.augment_1    = e.augment_one;
			i.augment_2    = e.augment_two;
			i.augment_3    = e.augment_three;
			i.augment_4    = e.augment_four;
			i.augment_5    = e.augment_five;
			i.augment_6    = e.augment_six;

			v.emplace_back(i);
		}
	}

	if (!v.empty()) {
		BotInventoriesRepository::InsertMany(content_db, v);
	}
}

void Bot::SetSpellRecastTimer(uint16 spell_id, int32 recast_delay) {
	if (!IsValidSpell(spell_id)) {
		OwnerMessage("Failed to set spell recast timer.");
		return;
	}

	if (!recast_delay) {
		recast_delay = CalcSpellRecastTimer(spell_id);
	}

	if (CheckSpellRecastTimer(spell_id)) {
		BotTimer t;

		t.timer_id    = spells[spell_id].timer_id;
		t.timer_value = (Timer::GetCurrentTime() + recast_delay);
		t.recast_time = recast_delay;
		t.is_spell    = true;
		t.is_disc     = false;
		t.spell_id    = spells[spell_id].id;
		t.is_item     = false;
		t.item_id     = 0;

		bot_timers.push_back(t);
	} else {
		if (!bot_timers.empty()) {
			for (int i = 0; i < bot_timers.size(); i++) {
				if (
					bot_timers[i].is_spell &&
					(
						(
							spells[spell_id].timer_id != 0 &&
							spells[spell_id].timer_id == bot_timers[i].timer_id
						) ||
						bot_timers[i].spell_id == spell_id
					)
				) {
					bot_timers[i].timer_value = (Timer::GetCurrentTime() + recast_delay);
					bot_timers[i].recast_time = recast_delay;
					break;
				}
			}
		}
	}
}

uint32 Bot::GetSpellRecastTimer(uint16 spell_id)
{
	uint32 result = 0;

	if (spell_id && !IsValidSpell(spell_id)) {
		OwnerMessage("Failed to get spell recast timer.");
		return result;
	}

	if (!bot_timers.empty()) {
		for (int i = 0; i < bot_timers.size(); i++) {
			if (
				bot_timers[i].is_spell &&
				(
					!spell_id ||
					(
						(
							spells[spell_id].timer_id != 0 &&
							spells[spell_id].timer_id == bot_timers[i].timer_id
						) ||
						bot_timers[i].spell_id == spell_id
					)
				)
			) {
				result = bot_timers[i].timer_value;
				break;
			}
		}
	}

	return result;
}

uint32 Bot::GetSpellRecastRemainingTime(uint16 spell_id)
{
	uint32 result = 0;

	if (GetSpellRecastTimer(spell_id) > Timer::GetCurrentTime()) {
		result = (GetSpellRecastTimer(spell_id) - Timer::GetCurrentTime());
	}

	return result;
}

bool Bot::CheckSpellRecastTimer(uint16 spell_id)
{
	ClearExpiredTimers();

	if (spell_id && !IsValidSpell(spell_id)) {
		OwnerMessage("Failed to check spell recast timer.");
		return false;
	}

	if (GetSpellRecastTimer(spell_id) < Timer::GetCurrentTime()) {
		return true;
	}

	return false;
}

void Bot::SetDisciplineReuseTimer(uint16 spell_id, int32 reuse_timer)
{
	if (!IsValidSpell(spell_id)) {
		OwnerMessage("Failed to set discipline reuse timer.");
		return;
	}

	if (!reuse_timer) {
		reuse_timer = CalcSpellRecastTimer(spell_id);
	}

	if (CheckDisciplineReuseTimer(spell_id)) {
		BotTimer t;

		t.timer_id    = spells[spell_id].timer_id;
		t.timer_value = (Timer::GetCurrentTime() + reuse_timer);
		t.recast_time = reuse_timer;
		t.is_spell    = false;
		t.is_disc     = true;
		t.spell_id    = spells[spell_id].id;
		t.is_item     = false;
		t.item_id     = 0;

		bot_timers.push_back(t);
	} else {
		if (!bot_timers.empty()) {
			for (int i = 0; i < bot_timers.size(); i++) {
				if (
					bot_timers[i].is_disc &&
					(
						(
							spells[spell_id].timer_id != 0 &&
							spells[spell_id].timer_id == bot_timers[i].timer_id
						) ||
						bot_timers[i].spell_id == spell_id
					)
				) {
					bot_timers[i].timer_value = (Timer::GetCurrentTime() + reuse_timer);
					bot_timers[i].recast_time = reuse_timer;
					break;
				}
			}
		}
	}
}

uint32 Bot::GetDisciplineReuseTimer(uint16 spell_id)
{
	uint32 result = 0;

	if (!bot_timers.empty()) {
		for (int i = 0; i < bot_timers.size(); i++) {
			if (
				bot_timers[i].is_disc &&
				(
					!spell_id ||
					(
						(
							spells[spell_id].timer_id != 0 &&
							spells[spell_id].timer_id == bot_timers[i].timer_id
						) ||
						bot_timers[i].spell_id == spell_id
					)
				)
			) {
				result = bot_timers[i].timer_value;
				break;
			}
		}
	}

	return result;
}

uint32 Bot::GetDisciplineReuseRemainingTime(uint16 spell_id) {
	uint32 result = 0;

	if (GetDisciplineReuseTimer(spell_id) > Timer::GetCurrentTime()) {
		result = (GetDisciplineReuseTimer(spell_id) - Timer::GetCurrentTime());
	}

	return result;
}

bool Bot::CheckDisciplineReuseTimer(uint16 spell_id)
{
	ClearExpiredTimers();

	if (GetDisciplineReuseTimer(spell_id) < Timer::GetCurrentTime()) { //checks for spells on the same timer
		return true; //can cast spell
	}

	return false;
}

void Bot::SetItemReuseTimer(uint32 item_id, uint32 reuse_timer)
{
	const auto *item = database.GetItem(item_id);

	if (!item) {
		OwnerMessage("Failed to set item reuse timer.");
		return;
	}

	if (item->RecastDelay <= 0) {
		return;
	}

	if (CheckItemReuseTimer(item_id)) {
		BotTimer t;

		t.timer_id    = (item->RecastType == NegativeItemReuse ? item->ID : item->RecastType);
		t.timer_value = (
			reuse_timer != 0 ?
			(Timer::GetCurrentTime() + reuse_timer) :
			(Timer::GetCurrentTime() + (item->RecastDelay * 1000))
		);
		t.recast_time = (reuse_timer != 0 ? reuse_timer : (item->RecastDelay * 1000));
		t.is_spell    = false;
		t.is_disc     = false;
		t.spell_id    = 0;
		t.is_item     = true;
		t.item_id     = item->ID;

		bot_timers.push_back(t);
	}
	else {
		if (!bot_timers.empty()) {
			for (int i = 0; i < bot_timers.size(); i++) {
				if (
					bot_timers[i].is_item &&
					(
						(
							item->RecastType != 0 &&
							item->RecastType == bot_timers[i].timer_id
						) ||
						bot_timers[i].item_id == item_id
					)
				) {
					bot_timers[i].timer_value = (
						reuse_timer != 0 ?
						(Timer::GetCurrentTime() + reuse_timer) :
						(Timer::GetCurrentTime() + (item->RecastDelay * 1000))
					);
					bot_timers[i].recast_time = (
						reuse_timer != 0 ?
						reuse_timer :
						(item->RecastDelay * 1000)
					);
					break;
				}
			}
		}
	}
}

uint32 Bot::GetItemReuseTimer(uint32 item_id)
{
	uint32 result = 0;
	const EQ::ItemData* item;

	if (item_id) {
		item = database.GetItem(item_id);

		if (!item) {
			OwnerMessage("Failed to get item reuse timer.");
			return result;
		}
	}

	if (!bot_timers.empty()) {
		for (int i = 0; i < bot_timers.size(); i++) {
			if (
				bot_timers[i].is_item &&
				(
					!item_id ||
					(
						(
							item->RecastType != 0 &&
							item->RecastType == bot_timers[i].timer_id
						) ||
						bot_timers[i].item_id == item_id
					)
				)
			) {
				result = bot_timers[i].timer_value;
				break;
			}
		}
	}

	ClearExpiredTimers();

	return result;
}

bool Bot::CheckItemReuseTimer(uint32 item_id)
{
	ClearExpiredTimers();

	if (GetItemReuseTimer(item_id) < Timer::GetCurrentTime()) {
		return true;
	}

	return false;
}

uint32 Bot::GetItemReuseRemainingTime(uint32 item_id)
{
	uint32 result = 0;

	if (GetItemReuseTimer(item_id) > Timer::GetCurrentTime()) {
		result = (GetItemReuseTimer(item_id) - Timer::GetCurrentTime());
	}

	return result;
}

uint32 Bot::CalcSpellRecastTimer(uint16 spell_id)
{
	uint32 result = 0;

	if (spells[spell_id].recast_time == 0 && spells[spell_id].recovery_time == 0) {
		return result;
	} else {
		if (spells[spell_id].recovery_time > spells[spell_id].recast_time) {
			result = spells[spell_id].recovery_time;
		} else {
			result = spells[spell_id].recast_time;
		}
	}

	return result;
}

void Bot::ClearDisciplineReuseTimer(uint16 spell_id)
{
	if (spell_id && !IsValidSpell(spell_id)) {
		OwnerMessage(
			fmt::format(
				"{} is not a valid spell ID.'",
				spell_id
			)
		);
		return;
	}

	if (!bot_timers.empty()) {
		for (int i = 0; i < bot_timers.size(); i++) {
			if (
				bot_timers[i].is_disc &&
				bot_timers[i].timer_value >= Timer::GetCurrentTime()
			) {
				if (
					!spell_id ||
					(
						(
							spells[spell_id].timer_id != 0 &&
							spells[spell_id].timer_id == bot_timers[i].timer_id
						) ||
						bot_timers[i].spell_id == spell_id
					)
				) {
					bot_timers[i].timer_value = 0;
				}
			}
		}
	}

	ClearExpiredTimers();
}

void Bot::ClearItemReuseTimer(uint32 item_id)
{
	const EQ::ItemData* item;

	if (item_id) {
		item = database.GetItem(item_id);

		if (!item) {
			OwnerMessage(
				fmt::format(
					"{} is not a valid item ID.",
					item_id
				)
			);
			return;
		}
	}

	if (!bot_timers.empty()) {
		for (int i = 0; i < bot_timers.size(); i++) {
			if (bot_timers[i].is_item && bot_timers[i].timer_value >= Timer::GetCurrentTime()) {
				if (
					!item_id ||
					(
						(
							item->RecastType != 0 &&
							item->RecastType == bot_timers[i].timer_id
						) ||
						bot_timers[i].item_id == item_id
					)
				) {
					bot_timers[i].timer_value = 0;
				}
			}
		}
	}

	ClearExpiredTimers();
}

void Bot::ClearSpellRecastTimer(uint16 spell_id)
{
	if (spell_id && !IsValidSpell(spell_id)) {
		OwnerMessage(
			fmt::format(
				"{} is not a valid spell ID.",
				spell_id
			)
		);
		return;
	}

	if (!bot_timers.empty()) {
		for (int i = 0; i < bot_timers.size(); i++) {
			if (bot_timers[i].is_spell && bot_timers[i].timer_value >= Timer::GetCurrentTime()) {
				if (
					!spell_id ||
					(
						(
							spells[spell_id].timer_id != 0 &&
							spells[spell_id].timer_id == bot_timers[i].timer_id
						) ||
						bot_timers[i].spell_id == spell_id
					)
				) {
					bot_timers[i].timer_value = 0;
				}
			}
		}
	}

	ClearExpiredTimers();
}


void Bot::ClearExpiredTimers()
{
	if (!bot_timers.empty()) {
		int current = 0;
		int end = bot_timers.size();

		while (current < end) {
			if (bot_timers[current].timer_value < Timer::GetCurrentTime()) {
				bot_timers.erase(bot_timers.begin() + current);
			} else {
				current++;
			}

			end = bot_timers.size();
		}
	}
}

void Bot::TryItemClick(uint16 slot_id)
{
	if (!GetOwner()) {
		return;
	}

	const auto *inst = GetClickItem(slot_id);

	if (!inst) {
		return;
	}

	const auto *item = inst->GetItem();

	if (!item) {
		return;
	}

	if (!CheckItemReuseTimer(item->ID)) {
		uint32 remaining_time = (GetItemReuseRemainingTime(item->ID) / 1000);
		OwnerMessage(
			fmt::format(
				"I can use this item in {}.",
				Strings::SecondsToTime(remaining_time)
			)
		);
		return;
	}

	DoItemClick(item, slot_id);
}

EQ::ItemInstance *Bot::GetClickItem(uint16 slot_id)
{
	EQ::ItemInstance* inst = nullptr;
	const EQ::ItemData* item = nullptr;

	inst = GetBotItem(slot_id);

	if (!inst || !inst->GetItem()) {
		return nullptr;
	}

	item = inst->GetItem();

	if (item->ID == MAG_EPIC_1_0 && !RuleB(Bots, CanClickMageEpicV1)) {
		OwnerMessage(
			fmt::format(
				"{} is currently disabled for bots to click.",
				item->Name
			)
		);
		return nullptr;
	}

	if (item->Click.Effect <= 0) {
		OwnerMessage(
			fmt::format(
				"{} does not have a clickable effect.",
				item->Name
			)
		);
		return nullptr;
	}

	if (!IsValidSpell(item->Click.Effect)) {
		OwnerMessage(
			fmt::format(
				"{} does not have a valid clickable effect.",
				item->Name
			)
		);
		return nullptr;
	}

	if (item->ReqLevel > GetLevel()) {
		OwnerMessage(
			fmt::format(
				"I am below the level requirement of {} for {}.",
				item->ReqLevel,
				item->Name
			)
		);
		return nullptr;
	}

	if (item->Click.Level2 > GetLevel()) {
		OwnerMessage(
			fmt::format(
				"I must be level {} to use {}.",
				item->Click.Level2,
				item->Name
			)
		);
		return nullptr;
	}

	if (inst->GetCharges() == 0) {
		OwnerMessage(
			fmt::format(
				"{} is out of charges.",
				item->Name
			)
		);
		return nullptr;
	}

	return inst;
}

void Bot::DoItemClick(const EQ::ItemData *item, uint16 slot_id)
{
	bool is_casting_bard_song = false;
	Mob* tar = (GetOwner()->GetTarget() ? GetOwner()->GetTarget() : this);

	if (!DoLosChecks(tar)) {
		return;
	}

	if (IsCasting()) {
		InterruptSpell();
	}

	SetIsUsingItemClick(true);

	RaidGroupSay(
		fmt::format(
			"Attempting to cast [{}] on {}.",
			spells[item->Click.Effect].name,
			tar->GetCleanName()
		).c_str()
	);

	if (!IsCastWhileInvisibleSpell(item->Click.Effect)) {
		CommonBreakInvisible();
	}

	if (GetClass() == Class::Bard && IsCasting() && casting_spell_slot < EQ::spells::CastingSlot::MaxGems) {
		is_casting_bard_song = true;
	}

	if (GetClass() == Class::Bard) {
		DoBardCastingFromItemClick(is_casting_bard_song, item->CastTime, item->Click.Effect, tar->GetID(), EQ::spells::CastingSlot::Item, slot_id, item->RecastType, item->RecastDelay);
	} else {
		if (!CastSpell(item->Click.Effect, tar->GetID(), EQ::spells::CastingSlot::Item, item->CastTime, 0, 0, slot_id)) {
			OwnerMessage(
				fmt::format(
					"Casting failed for {}. This could be due to zone restrictions, target restrictions or other limiting factors.",
					item->Name
				)
			);
		}
	}

}

uint8 Bot::spell_casting_chances[SPELL_TYPE_COUNT][Class::PLAYER_CLASS_COUNT][Stance::AEBurn][cntHSND] = { 0 };

bool Bot::PrecastChecks(Mob* tar, uint16 spell_type) {
	if (!TargetValidation(tar)) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast due to PrecastChecks !tar.'", GetCleanName());
		return false;
	}

	LogBotSpellChecksDetail("{} says, 'Running [{}] PreChecks on [{}].'", GetCleanName(), GetSpellTypeNameByID(spell_type), tar->GetCleanName());

	if (GetUltimateSpellTypeHold(spell_type, tar)) {
		if (!IsCommandedSpell()) {
			SetSpellTypeAITimer(spell_type, RuleI(Bots, AICastSpellTypeHeldDelay));
		}
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of [{}] on [{}] due to GetUltimateSpellTypeHold.'", GetCleanName(), GetSpellTypeNameByID(spell_type), tar->GetCleanName());
		return false;
	}

	if (
		IsPullingSpell() &&
		IsPullingBotSpellType(spell_type)
	) { //Skip remaining checks for commanded
		return true;
	}

	if (!EQ::ValueWithin(GetManaRatio(), GetSpellTypeMinManaLimit(spell_type), GetSpellTypeMaxManaLimit(spell_type))) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of [{}] on [{}] due to GetSpellTypeMinManaLimit or GetSpellTypeMaxManaLimit.'", GetCleanName(), GetSpellTypeNameByID(spell_type), tar->GetCleanName());
		return false;
	}

	if (!EQ::ValueWithin(GetHPRatio(), GetSpellTypeMinHPLimit(spell_type), GetSpellTypeMaxHPLimit(spell_type))) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of [{}] on [{}] due to GetSpellTypeMinHPLimit or GetSpellTypeMaxHPLimit.'", GetCleanName(), GetSpellTypeNameByID(spell_type), tar->GetCleanName());
		return false;
	}

	if (!GetUltimateSpellTypeRecastCheck(spell_type, tar)) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of [{}] on [{}] due to GetUltimateSpellTypeRecastCheck.'", GetCleanName(), GetSpellTypeNameByID(spell_type), tar->GetCleanName());
		return false;
	}

	switch (spell_type) { //This will skip Threshold Checks during Precast for specific SpellTypes that are checked when acquiring new targets
		case BotSpellTypes::Mez:
		case BotSpellTypes::AEMez:
			return true;
		default:
			if (!EQ::ValueWithin(GetHPRatioForSpellType(spell_type, tar), GetUltimateSpellTypeMinThreshold(spell_type, tar), GetUltimateSpellTypeMaxThreshold(spell_type, tar))) {
				LogBotSpellChecksDetail("{} says, 'Cancelling cast of [{}] on [{}] due to GetUltimateSpellTypeMinThreshold or GetUltimateSpellTypeMaxThreshold.'", GetCleanName(), GetSpellTypeNameByID(spell_type), tar->GetCleanName());
				return false;
			}
	}

	return true;
}

bool Bot::CastChecks(uint16 spell_id, Mob* tar, uint16 spell_type, bool prechecks, bool ae_check) {
	if (prechecks) {
		if (!tar || tar->GetAppearance() == eaDead  || tar->GetHP() < 0) {
			LogBotSpellChecksDetail("{} says, 'Cancelling cast due to CastChecks !tar.'", GetCleanName());
			return false;
		}

		if (spells[spell_id].target_type == ST_Self && tar != this) {
			if (
				!IsEffectInSpell(spell_id, SpellEffect::SummonCorpse) ||
				(
					IsEffectInSpell(spell_id, SpellEffect::SummonCorpse) &&
					!RuleB(Bots, AllowCommandedSummonCorpse)
				)
			) {
				tar = this;
			}
		}

		if (!PrecastChecks(tar, spell_type)) {
			LogBotSpellChecksDetail("{} says, 'Cancelling cast due to !PrecastChecks.'", GetCleanName());
			return false;
		}
	}

	LogBotSpellChecksDetail("{} says, 'Running [{}] CastChecks on [{}].'", GetCleanName(), GetSpellTypeNameByID(spell_type), (tar ? tar->GetCleanName() : "nobody"));

	if (!IsValidSpell(spell_id)) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast due to !IsValidSpell.'", GetCleanName());
		return false;
	}

	if (
		IsFeared() ||
		IsSilenced() ||
		IsAmnesiad()
	) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to Incapacitated.'", GetCleanName(), GetSpellName(spell_id), (tar ? tar->GetCleanName() : "nobody"));
		return false;
	}

	if (
		(
			IsStunned() ||
			IsMezzed() ||
			DivineAura()
		) &&
		!IsCastNotStandingSpell(spell_id)
	) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to !IsCastNotStandingSpell.'", GetCleanName(), GetSpellName(spell_id), (tar ? tar->GetCleanName() : "nobody"));
		return false;
	}

	if (
		IsDetrimentalSpell(spell_id) &&
		!zone->CanDoCombat()
	) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to !CanDoCombat.'", GetCleanName(), GetSpellName(spell_id), (tar ? tar->GetCleanName() : "nobody"));
		return false;
	}

	if (!CheckSpellRecastTimer(spell_id)) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} due to !CheckSpellRecastTimer.'", GetCleanName(), GetSpellName(spell_id));
		return false;
	}

	bool is_mana_exempt = RuleB(Bots, FinishBuffing) && !IsEngaged() && IsBotBuffSpellType(spell_type);

	if (!BotHasEnoughMana(spell_id) && !is_mana_exempt) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} due to !BotHasEnoughMana.'", GetCleanName(), GetSpellName(spell_id));
		return false;
	}

	if (zone->IsSpellBlocked(spell_id, glm::vec3(GetPosition()))) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} due to IsSpellBlocked.'", GetCleanName(), GetSpellName(spell_id));
		return false;
	}

	if (
		spells[spell_id].caster_requirement_id &&
		!PassCastRestriction(spells[spell_id].caster_requirement_id)
	) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} due to !PassCastRestriction.'", GetCleanName(), GetSpellName(spell_id));
		return false;
	}

	if (
		!spells[spell_id].can_cast_in_combat &&
		spells[spell_id].can_cast_out_of_combat
	) {
		if (IsBeneficialSpell(spell_id)) {
			if (IsEngaged()) {
				LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} due to !can_cast_in_combat.'", GetCleanName(), GetSpellName(spell_id));
				return false;
			}
		}
	}
	else if (
		spells[spell_id].can_cast_in_combat &&
		!spells[spell_id].can_cast_out_of_combat
	) {
		if (IsBeneficialSpell(spell_id)) {
			if (!IsEngaged()) {
				LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} due to !can_cast_out_of_combat.'", GetCleanName(), GetSpellName(spell_id));
				return false;
			}
		}
	}

	if (!IsDiscipline(spell_id)) {
		int chance = GetFocusEffect(focusFcMute, spell_id);

		if (
			chance &&
			zone->random.Roll(chance)
		) {
			LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} due to focusFcMute.'", GetCleanName(), GetSpellName(spell_id));
			return false;
		}
	}

	if (
		!zone->CanLevitate() &&
		IsEffectInSpell(spell_id, SpellEffect::Levitate)
	) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} due to !CanLevitate.'", GetCleanName(), GetSpellName(spell_id));
		return false;
	}

	if (
		spells[spell_id].time_of_day == SpellTimeRestrictions::Day &&
		!zone->zone_time.IsDayTime()
	) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} due to !IsDayTime.'", GetCleanName(), GetSpellName(spell_id));
		return false;
	}

	if (
		spells[spell_id].time_of_day == SpellTimeRestrictions::Night &&
		!zone->zone_time.IsNightTime()
	) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} due to !IsNightTime.'", GetCleanName(), GetSpellName(spell_id));
		return false;
	}

	if (
		spells[spell_id].zone_type == 1 &&
		!zone->CanCastOutdoor()
	) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} due to !CanCastOutdoor.'", GetCleanName(), GetSpellName(spell_id));
		return false;
	}

	if (
		BotSpellTypeRequiresTarget(spell_type) &&
		!TargetValidation(tar)
	) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast due to CastChecks !tar.'", GetCleanName());
		return false;
	}

	if (spells[spell_id].target_type == ST_Self && tar != this) {
		if (spell_type == BotSpellTypes::SummonCorpse && RuleB(Bots, AllowCommandedSummonCorpse)) {
			// Don't cancel (Summon Corpse is allowed)
		}
		else {
			LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to ST_Self.'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName());

			return false;
		}
	}

	if (
		this == tar &&
		IsSacrificeSpell(spell_id)
	) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} due to IsSacrificeSpell.'", GetCleanName(), GetSpellName(spell_id));
		return false;
	}

	if (tar->GetSpecialAbility(SpecialAbility::MagicImmunity)) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to MagicImmunity.'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName());
		return false;
	}

	if (
		tar->GetSpecialAbility(SpecialAbility::CastingFromRangeImmunity) &&
		!CombatRange(tar)
	) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to CastingFromRangeImmunity.'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName());
		return false;
	}

	if (tar->CastToBot()->IsImmuneToBotSpell(spell_id, this)) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to IsImmuneToBotSpell.'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName());
		return false;
	}

	if (!tar->CheckSpellLevelRestriction(this, spell_id)) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to CheckSpellLevelRestriction.'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName());
		return false;
	}

	if (
		IsBeneficialSpell(spell_id) &&
		(
			(
				RuleB(Spells, EnableBlockedBuffs) &&
				tar->IsClient()
			) ||
			(
				RuleB(Bots, AllowBotBlockedBuffs) &&
				tar->IsBot()
			)
		)
	) {
		if (tar->IsBlockedBuff(spell_id)) {
			LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to IsBlockedPetBuff.'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName());
			return false;
		}
	}

	if (
		IsBeneficialSpell(spell_id) &&
		tar->IsPet() &&
		(
			(
				RuleB(Spells, EnableBlockedBuffs) &&
				tar->GetOwner() &&
				tar->GetOwner()->IsClient()
			) ||
			(
				RuleB(Bots, AllowBotBlockedBuffs) &&
				tar->GetOwner() &&
				tar->GetOwner()->IsBot()
			)
		)
	) {
		if (tar->GetOwner()->IsBlockedPetBuff(spell_id)) {
			LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to IsBlockedPetBuff.'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName());
			return false;
		}
	}
	//LogBotSpellChecksDetail("{} says, 'Doing CanCastSpellType checks of {} on {}.'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName());
	if (!CanCastSpellType(spell_type, spell_id, tar)) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to CanCastSpellType.'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName());
		return false;
	}

	if (!IsValidTargetType(spell_id, GetSpellTargetType(spell_id), tar->GetBodyType())) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to IsValidTargetType.'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName());
		return false;
	}

	if (
		(
			RequiresStackCheck(spell_type) ||
			(
				!RequiresStackCheck(spell_type) &&
				CalcBuffDuration(this, tar, spell_id) != 0
			)
		)
		&&
		tar->CanBuffStack(spell_id, GetLevel(), true) < 0
	) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to !CanBuffStack.'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName());
		return false;
	}

	if (
		IsBeneficialSpell(spell_id) &&
		tar->BuffCount() >= tar->GetCurrentBuffSlots() &&
		CalcBuffDuration(this, tar, spell_id) != 0
	) {
		return false;
	}

	if (
		!ae_check &&
		!IsValidSpellRange(spell_id, tar)
	) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to IsValidSpellRange.'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName());
		return false;
	}

	if (IsCommandedSpell() || spell_type == UINT16_MAX) { //AA/Forced cast checks, return here
		return true;
	}

	if (
		!IsTaunting() &&
		GetSpellTypeAggroCheck(spell_type) &&
		HasOrMayGetAggro(IsSitting(), spell_id) &&
		!tar->IsFleeing()
	) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to HasOrMayGetAggro.'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName());
		return false;
	}

	if (!DoResistCheckBySpellType(tar, spell_id, spell_type)) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to DoResistCheckBySpellType.'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName());
		return false;
	}

	if (
		spells[spell_id].target_type != ST_Self &&
		IsBeneficialSpell(spell_id) &&
		!IsAnyHealSpell(spell_id) &&
		!IsCureSpell(spell_id) &&
		!IsHealOverTimeSpell(spell_id) &&
		!IsGroupHealOverTimeSpell(spell_id) &&
		IsTargetAlreadyReceivingSpell(tar, spell_id)
	) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to IsTargetAlreadyReceivingSpell.'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName());
		return false;
	}

	return true;
}

bool Bot::CanCastSpellType(uint16 spell_type, uint16 spell_id, Mob* tar) {
	if (!spell_id || !tar) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to failsafe checks.'", GetCleanName(), (spell_id ? GetSpellName(spell_id) : (spell_type ? GetSpellTypeNameByID(spell_type) : "Unknown")), (tar ? tar->GetCleanName() : "Unknown"));
		return false;
	}

	uint8 bot_class = GetClass();
	auto spell = spells[spell_id];

	switch (spell_type) {
		case BotSpellTypes::Buff:
		case BotSpellTypes::PetBuffs:
		case BotSpellTypes::PreCombatBuff:
		case BotSpellTypes::InCombatBuff:
		case BotSpellTypes::DamageShields:
		case BotSpellTypes::PetDamageShields:
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
			if (
				tar == this &&
				spell.target_type == ST_TargetsTarget
			) {
				LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to target_type checks. Using {}'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName(), GetSpellTargetType(spell_id));
				return false;
			}

			if (
				(
					spell_type != BotSpellTypes::Teleport &&
					spell_type != BotSpellTypes::Succor
				) &&
				(
					IsEffectInSpell(spell_id, SpellEffect::Teleport) ||
					IsEffectInSpell(spell_id, SpellEffect::Succor)
				)
			) {
				LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to Teleport.'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName());
				return false;
			}

			if (
				tar->IsPet() &&
				!RuleB(Bots, CanCastIllusionsOnPets) &&
				IsEffectInSpell(spell_id, SpellEffect::Illusion)
			) {
				LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to PetSE_Illusion.'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName());
				return false;
			}

			if (
				(spell.target_type == ST_Pet || spell.target_type == ST_SummonedPet) &&
				(
					!tar->IsPet() ||
					(
						tar->GetOwner() != this &&
						!RuleB(Bots, CanCastPetOnlyOnOthersPets)
					)
				)
			) {
				LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to PetOnly.'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName());
				return false;
			}

			if (
				(
					IsGroupSpell(spell_id) &&
					tar->IsPet()
				) &&
				(
					!tar->GetOwner() ||
					(
						RuleB(Bots, RequirePetAffinity) &&
						!tar->GetOwner()->HasPetAffinity()
					)
				)
			) {
				LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to PetGroupSpellTarget.'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName());
				return false;
			}

			if (!IsCommandedSpell()) {
				switch (tar->GetArchetype()) {
					case Archetype::Caster:
						if (
							tar->IsBot() &&
							(tar->GetLevel() >= tar->CastToBot()->GetStopMeleeLevel()) &&
							(
								IsEffectInSpell(spell_id, SpellEffect::AttackSpeed) ||
								IsEffectInSpell(spell_id, SpellEffect::ReverseDS)
							) ||
							(
								SpellEffectsCount(spell_id) == 1 &&
								(
									IsEffectInSpell(spell_id, SpellEffect::ATK) || IsEffectInSpell(spell_id, SpellEffect::STR)
								)
							)
						) {
							LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to Archetype::Caster.'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName());
							return false;
						}
						break;
					case Archetype::Melee:
						if (
							IsEffectInSpell(spell_id, SpellEffect::IncreaseSpellHaste) ||
							IsEffectInSpell(spell_id, SpellEffect::ManaPool) ||
							IsEffectInSpell(spell_id, SpellEffect::CastingLevel) ||
							IsEffectInSpell(spell_id, SpellEffect::ManaRegen_v2) ||
							IsEffectInSpell(spell_id, SpellEffect::CurrentMana)
						) {
							LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to Archetype::Melee.'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName());
							return false;
						}
						break;
					case Archetype::Hybrid: //Hybrids get all buffs
					default:
						break;
				}
			}

			// Differences for each type
			if (spell_type != BotSpellTypes::InCombatBuff) {
				if (
					IsEffectInSpell(spell_id, SpellEffect::AbsorbMagicAtt) ||
					IsEffectInSpell(spell_id, SpellEffect::Rune)
				) {
					for (int i = 0; i < tar->GetMaxTotalSlots(); i++) {
						uint32 buff_count = tar->GetMaxTotalSlots();

						for (unsigned int j = 0; j < buff_count; j++) {
							if (IsValidSpell(tar->GetBuffs()[j].spellid)) {
								if (IsLichSpell(tar->GetBuffs()[j].spellid)) {
									LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to IsLichSpell.'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName());
									return false;
								}
							}
						}
					}
				}
			}

			break;
		case BotSpellTypes::PreCombatBuffSong:
		case BotSpellTypes::InCombatBuffSong:
		case BotSpellTypes::OutOfCombatBuffSong:
			if (!IsCommandedSpell()) {
				switch (tar->GetArchetype()) {
					case Archetype::Caster:
						if (
							tar->IsBot() &&
							(tar->GetLevel() >= tar->CastToBot()->GetStopMeleeLevel()) &&
							(
								IsEffectInSpell(spell_id, SpellEffect::AttackSpeed) ||
								IsEffectInSpell(spell_id, SpellEffect::ReverseDS)
							) ||
							(
								SpellEffectsCount(spell_id) == 1 &&
								(
									IsEffectInSpell(spell_id, SpellEffect::ATK) ||
									IsEffectInSpell(spell_id, SpellEffect::STR)
								)
							)
						) {
							LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to Archetype::Caster.'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName());
							return false;
						}
						break;
					case Archetype::Melee:
						if (
							IsEffectInSpell(spell_id, SpellEffect::IncreaseSpellHaste) ||
							IsEffectInSpell(spell_id, SpellEffect::ManaPool) ||
							IsEffectInSpell(spell_id, SpellEffect::CastingLevel) ||
							IsEffectInSpell(spell_id, SpellEffect::ManaRegen_v2) ||
							IsEffectInSpell(spell_id, SpellEffect::CurrentMana)
						) {
							LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to Archetype::Melee.'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName());
							return false;
						}
						break;
					case Archetype::Hybrid: //Hybrids get all buffs
					default:
						break;
				}
			}

			break;
		case BotSpellTypes::AELull:
		case BotSpellTypes::Lull:
			if (
				IsHarmonySpell(spell_id) &&
				!HarmonySpellLevelCheck(spell_id, tar)
			) {
				LogBotSpellChecksDetail("{} says, 'Cancelling cast of {} on {} due to HarmonySpellLevelCheck.'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName());
				return false;
			}

			break;
		default:
			break;
	}

	//LogBotSpellChecksDetail("{} says, {} on {} passed CanCastSpellType.'", GetCleanName(), GetSpellName(spell_id), tar->GetCleanName());
	return true;
}

bool Bot::BotHasEnoughMana(uint16 spell_id) {
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	int32 mana_cost = spells[spell_id].mana;

	if (GetMana() < mana_cost) {
		return false;
	}

	return true;
}

bool Bot::IsTargetAlreadyReceivingSpell(Mob* tar, uint16 spell_id) {
	if (!tar || !spell_id) {
		return true;
	}

	uint16 target_id = tar->GetID();

	for (Mob* m : GetSpellTargetList(RuleB(Bots, CrossRaidBuffingAndHealing))) {
		if (
			m->IsBot() &&
			m->IsCasting() &&
			m->CastToBot()->casting_spell_targetid &&
			m->CastingSpellID() == spell_id
		) {
			if (m->CastToBot()->casting_spell_targetid == target_id) {
				return true;
			}

			if (!RuleB(Bots, CrossRaidBuffingAndHealing) && IsGroupSpell(spell_id)) {
				for (Mob* x : GatherSpellTargets(false, tar)) {
					if (x->GetID() == m->CastToBot()->casting_spell_targetid) {
						return true;
					}
				}
			}
		}
	}

	return false;
}

bool Bot::DoResistCheck(Mob* tar, uint16 spell_id, int32 resist_limit) {

	if (!tar || spell_id == 0) {
		return false;
	}

	int32 resist_difficulty = -spells[spell_id].resist_difficulty;
	int32 level_mod = (tar->GetLevel() - GetLevel()) * (tar->GetLevel() - GetLevel()) / 2;

	if (tar->GetLevel() - GetLevel() < 0) {
		level_mod = -level_mod;
	}

	int32 target_resist = 0;

	switch (GetSpellResistType(spell_id)) {
		case RESIST_NONE:
			return true;
		case RESIST_MAGIC:
			target_resist = tar->GetMR();
			break;
		case RESIST_COLD:
			target_resist = tar->GetCR();
			break;
		case RESIST_FIRE:
			target_resist = tar->GetFR();
			break;
		case RESIST_POISON:
			target_resist = tar->GetPR();
			break;
		case RESIST_DISEASE:
			target_resist = tar->GetDR();
			break;
		case RESIST_CORRUPTION:
			target_resist = tar->GetCorrup();
			break;
		default:
			return true;
	}

	if ((target_resist + level_mod - resist_difficulty) > resist_limit) {
		return false;
	}

	return true;
}

bool Bot::DoResistCheckBySpellType(Mob* tar, uint16 spell_id, uint16 spell_type) {
	if (!tar || !IsValidSpell(spell_id)) {
		return false;
	}

	if (GetSpellTypeResistLimit(spell_type) == 0) {
		return true;
	}

	return DoResistCheck(tar, spell_id, GetSpellTypeResistLimit(spell_type));
}

bool Bot::IsValidTargetType(uint16 spell_id, int target_type, uint8 body_type) {
	if (!spell_id) {
		return false;
	}

	switch (target_type) {
		case ST_Undead:
			if (
				body_type == BodyType::Undead ||
				body_type == BodyType::SummonedUndead ||
				body_type == BodyType::Vampire
			) {
				return true;
			}

			break;
		case ST_Summoned:
			if (
				body_type == BodyType::Summoned ||
				body_type == BodyType::Summoned2 ||
				body_type == BodyType::Summoned3
			) {
				return true;
			}

			break;
		case ST_Animal:
			if (body_type == BodyType::Animal) {
				return true;
			}

			break;
		case ST_Plant:
			if (body_type == BodyType::Plant) {
				return true;
			}

			break;
		case ST_Giant:
			if (
				body_type == BodyType::Giant ||
				body_type == BodyType::RaidGiant
			) {
				return true;
			}

			break;
		case ST_Dragon:
			if (
				body_type == BodyType::Dragon ||
				body_type == BodyType::VeliousDragon
			) {
				return true;
			}

			break;
		default:
			return true;
	}

	return false;
}

bool Bot::IsMobEngagedByAnyone(Mob* tar) {
    if (!tar) {
        return false;
    }

    for (Mob* m : GetSpellTargetList(true)) {
        if (m->GetTarget() != tar) {
            continue;
        }

        bool bot_is_engaged = m->IsBot() && m->IsEngaged();
        bool bot_melee_or_casting =
			bot_is_engaged &&
            (
				!m->CastToBot()->IsBotNonSpellFighter() ||
				(
					m->GetLevel() >= m->CastToBot()->GetStopMeleeLevel() &&
					!m->IsCasting()
				)
			);

        if (bot_melee_or_casting) {
            return true;
        }

        if (
			m->IsCasting() &&
			SpellBreaksMez(m->CastingSpellID())
		) {
            return true;
        }

        if (
			m->IsClient() &&
			(
				m->CastToClient()->AutoAttackEnabled() ||
				m->CastToClient()->AutoFireEnabled()
			)
		) {
            return true;
        }
    }

    return false;
}

bool Bot::IsValidMezTarget(Mob* owner, Mob* npc, uint16 spell_id) {
	if (npc->GetSpecialAbility(SpecialAbility::MesmerizeImmunity)) {
		return false;
	}

	if (!npc->CastToNPC()->IsOnHatelist(owner)) {
		return false;
	}

	if (npc->IsMezzed() || HasBotAttackFlag(npc)) {
		return false;
	}

	if (npc->HasOwner() && npc->GetOwner() && npc->GetOwner()->IsOfClientBotMerc()) {
		return false;
	}

	if (!IsValidTargetType(spell_id, GetSpellTargetType(spell_id), npc->GetBodyType())) {
		return false;
	}

	if (!IsAttackAllowed(GetTarget())) {
		return false;
	}

	if (!DoLosChecks(npc)) {
		return false;
	}

	if (IsMobEngagedByAnyone(npc)) {
		return false;
	}

	int buff_count = npc->GetMaxTotalSlots();
	auto npc_buffs = npc->GetBuffs();

	for (int i = 0; i < buff_count; i++) {
		if (IsDetrimentalSpell(npc_buffs[i].spellid) && IsEffectInSpell(npc_buffs[i].spellid, SpellEffect::CurrentHP)) {
			return false;
		}
	}

	return true;
}

void Bot::SetBotSetting(uint8 setting_type, uint16 bot_setting, int setting_value) {
	switch (setting_type) {
		case BotSettingCategories::BaseSetting:
			SetBotBaseSetting(bot_setting, setting_value);
			break;
		case BotSettingCategories::SpellHold:
			SetSpellTypeHold(bot_setting, setting_value);
			break;
		case BotSettingCategories::SpellDelay:
			SetSpellTypeDelay(bot_setting, setting_value);
			break;
		case BotSettingCategories::SpellMinThreshold:
			SetSpellTypeMinThreshold(bot_setting, setting_value);
			break;
		case BotSettingCategories::SpellMaxThreshold:
			SetSpellTypeMaxThreshold(bot_setting, setting_value);
			break;
		case BotSettingCategories::SpellTypeResistLimit:
			SetSpellTypeResistLimit(bot_setting, setting_value);
			break;
		case BotSettingCategories::SpellTypeAggroCheck:
			SetSpellTypeAggroCheck(bot_setting, setting_value);
			break;
		case BotSettingCategories::SpellTypeMinManaPct:
			SetSpellTypeMinManaLimit(bot_setting, setting_value);
			break;
		case BotSettingCategories::SpellTypeMaxManaPct:
			SetSpellTypeMaxManaLimit(bot_setting, setting_value);
			break;
		case BotSettingCategories::SpellTypeMinHPPct:
			SetSpellTypeMinHPLimit(bot_setting, setting_value);
			break;
		case BotSettingCategories::SpellTypeMaxHPPct:
			SetSpellTypeMaxHPLimit(bot_setting, setting_value);
			break;
		case BotSettingCategories::SpellTypeIdlePriority:
			SetSpellTypePriority(bot_setting, BotPriorityCategories::Idle, setting_value);
			break;
		case BotSettingCategories::SpellTypeEngagedPriority:
			SetSpellTypePriority(bot_setting, BotPriorityCategories::Engaged, setting_value);
			break;
		case BotSettingCategories::SpellTypePursuePriority:
			SetSpellTypePriority(bot_setting, BotPriorityCategories::Pursue, setting_value);
			break;
		case BotSettingCategories::SpellTypeAEOrGroupTargetCount:
			SetSpellTypeAEOrGroupTargetCount(bot_setting, setting_value);
			break;
		case BotSettingCategories::SpellTypeAnnounceCast:
			SetSpellTypeAnnounceCast(bot_setting, setting_value);
			break;
	}
}

void Bot::SetBotBaseSetting(uint16 bot_setting, int setting_value) {
	switch (bot_setting) {
		case BotBaseSettings::ExpansionBitmask:
			SetExpansionBitmask(setting_value);
			break;
		case BotBaseSettings::ShowHelm:
			SetShowHelm(setting_value);
			break;
		case BotBaseSettings::FollowDistance:
			SetFollowDistance(EQ::Clamp(static_cast<uint32>(setting_value * setting_value), static_cast<uint32>(1), static_cast<uint32>((RuleI(Bots, MaxFollowDistance) * RuleI(Bots, MaxFollowDistance)))));
			break;
		case BotBaseSettings::StopMeleeLevel:
			SetStopMeleeLevel(setting_value);
			break;
		case BotBaseSettings::EnforceSpellSettings:
			SetBotEnforceSpellSetting(setting_value);
			break;
		case BotBaseSettings::RangedSetting:
			SetBotRangedSetting(setting_value);
			break;
		case BotBaseSettings::PetSetTypeSetting:
			SetPetChooserID(setting_value);
			break;
		case BotBaseSettings::BehindMob:
			SetBehindMob(setting_value);
			break;
		case BotBaseSettings::DistanceRanged:
			SetBotDistanceRanged(setting_value);
			break;
		case BotBaseSettings::IllusionBlock:
			SetIllusionBlock(setting_value);
			break;
		case BotBaseSettings::MaxMeleeRange:
			SetMaxMeleeRange(setting_value);
			break;
		case BotBaseSettings::MedInCombat:
			SetMedInCombat(setting_value);
			break;
		case BotBaseSettings::SitHPPct:
			SetSitHPPct(setting_value);
			break;
		case BotBaseSettings::SitManaPct:
			SetSitManaPct(setting_value);
			break;
		default:
			break;
	}
}

int Bot::GetBotBaseSetting(uint16 bot_setting) {
	switch (bot_setting) {
		case BotBaseSettings::ExpansionBitmask:
			return GetExpansionBitmask();
		case BotBaseSettings::ShowHelm:
			return GetShowHelm();
		case BotBaseSettings::FollowDistance:
			return sqrt(GetFollowDistance());
		case BotBaseSettings::StopMeleeLevel:
			return GetStopMeleeLevel();
		case BotBaseSettings::EnforceSpellSettings:
			return GetBotEnforceSpellSetting();
		case BotBaseSettings::RangedSetting:
			return IsBotRanged();
		case BotBaseSettings::PetSetTypeSetting:
			return GetPetChooserID();
		case BotBaseSettings::BehindMob:
			return GetBehindMob();
		case BotBaseSettings::DistanceRanged:
			return GetBotDistanceRanged();
		case BotBaseSettings::IllusionBlock:
			return GetIllusionBlock();
		case BotBaseSettings::MaxMeleeRange:
			return GetMaxMeleeRange();
		case BotBaseSettings::MedInCombat:
			return GetMedInCombat();
		case BotBaseSettings::SitHPPct:
			return GetSitHPPct();
		case BotBaseSettings::SitManaPct:
			return GetSitManaPct();
		default:
			return true;
	}

	return true;
}

int Bot::GetDefaultBotBaseSetting(uint16 bot_setting, uint8 stance) {
	switch (bot_setting) {
		case BotBaseSettings::ExpansionBitmask:
			return RuleI(Bots, BotExpansionSettings);
		case BotBaseSettings::ShowHelm:
			return true;
		case BotBaseSettings::FollowDistance:
			return RuleI(Bots, DefaultFollowDistance);
		case BotBaseSettings::StopMeleeLevel:
			if (IsCasterClass(GetClass())) {
				return RuleI(Bots, CasterStopMeleeLevel);
			}
			else {
				return 255;
			}
		case BotBaseSettings::PetSetTypeSetting:
			return 0;
		case BotBaseSettings::BehindMob:
			if (GetClass() == Class::Rogue || (IsPureMeleeClass() && GetClass() != Class::Warrior)) {
				return true;
			}
			else {
				return false;
			}
		case BotBaseSettings::DistanceRanged:
			switch (GetClass()) {
				case Class::Warrior:
				case Class::Monk:
				case Class::Rogue:
				case Class::Berserker:
					return 0;
				case Class::Bard:
					return 30;
				default:
					return 90;
			}
		case BotBaseSettings::MedInCombat:
			if (IsCasterClass(GetClass())) {
				return true;
			}

			return false;
		case BotBaseSettings::SitHPPct:
		case BotBaseSettings::SitManaPct:
			return 80;
		case BotBaseSettings::EnforceSpellSettings:
		case BotBaseSettings::RangedSetting:
		case BotBaseSettings::IllusionBlock:
		case BotBaseSettings::MaxMeleeRange:
		default:
			return false;
	}

	return true;
}


void Bot::LoadDefaultBotSettings() {
	m_bot_spell_settings.clear();

	uint8 bot_stance = GetBotStance();

	for (uint16 i = BotBaseSettings::START_ALL; i <= BotBaseSettings::END; ++i) {
		SetBotBaseSetting(i, GetDefaultSetting(BotSettingCategories::BaseSetting, i, bot_stance));
		LogBotSettingsDetail("{} says, 'Setting default {} [{}] to [{}]'", GetCleanName(), GetBotSettingCategoryName(i), i, GetDefaultBotBaseSetting(i, bot_stance));
	}

	for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
		BotSpellSettings t;

		t.spell_type               = i;
		t.short_name               = GetSpellTypeShortNameByID(i);
		t.name                     = GetSpellTypeNameByID(i);
		t.hold                     = GetDefaultSpellTypeHold(i, bot_stance);
		t.delay                    = GetDefaultSpellTypeDelay(i, bot_stance);
		t.min_threshold            = GetDefaultSpellTypeMinThreshold(i, bot_stance);
		t.max_threshold            = GetDefaultSpellTypeMaxThreshold(i, bot_stance);
		t.resist_limit             = GetDefaultSpellTypeResistLimit(i, bot_stance);
		t.aggro_check              = GetDefaultSpellTypeAggroCheck(i, bot_stance);
		t.min_mana_pct             = GetDefaultSpellTypeMinManaLimit(i, bot_stance);
		t.max_mana_pct             = GetDefaultSpellTypeMaxManaLimit(i, bot_stance);
		t.min_hp_pct               = GetDefaultSpellTypeMinHPLimit(i, bot_stance);
		t.max_hp_pct               = GetDefaultSpellTypeMaxHPLimit(i, bot_stance);
		t.idle_priority            = GetDefaultSpellTypePriority(
			i,
			BotPriorityCategories::Idle,
			GetClass(),
			bot_stance
		);
		t.engaged_priority         = GetDefaultSpellTypePriority(
			i,
			BotPriorityCategories::Engaged,
			GetClass(),
			bot_stance
		);
		t.pursue_priority          = GetDefaultSpellTypePriority(
			i,
			BotPriorityCategories::Pursue,
			GetClass(),
			bot_stance
		);
		t.ae_or_group_target_count = GetDefaultSpellTypeAEOrGroupTargetCount(i, bot_stance);
		t.announce_cast            = GetDefaultSpellTypeAnnounceCast(i, bot_stance);
		t.recast_timer.Start();
		t.ai_delay.Start();

		m_bot_spell_settings.push_back(t);

		LogBotSettingsDetail("{} says, 'Setting defaults for {} ({}) [#{}] - [{} [#{}] stance]'", GetCleanName(), t.name, t.short_name, t.spell_type, Stance::GetName(bot_stance), bot_stance);
		LogBotSettingsDetail("{} says, 'Hold = [{}] | Delay = [{}ms] | MinThreshold = [{}\%] | MaxThreshold = [{}\%]'", GetCleanName(),
							 GetDefaultSpellTypeHold(i, bot_stance),
							 GetDefaultSpellTypeDelay(i, bot_stance),
							 GetDefaultSpellTypeMinThreshold(i, bot_stance),
							 GetDefaultSpellTypeMaxThreshold(i, bot_stance));
		LogBotSettingsDetail("{} says, 'AggroCheck = [{}] | MinManaPCT = [{}\%] | MaxManaPCT = [{}\%] | MinHPPCT = [{}\% | MaxHPPCT = [{}\%]'", GetCleanName(), GetDefaultSpellTypeAggroCheck(i, bot_stance), GetDefaultSpellTypeMinManaLimit(i, bot_stance), GetDefaultSpellTypeMaxManaLimit(i, bot_stance), GetDefaultSpellTypeMinHPLimit(i, bot_stance), GetDefaultSpellTypeMaxHPLimit(i, bot_stance));
		LogBotSettingsDetail("{} says, 'IdlePriority = [{}] | EngagedPriority = [{}] | PursuePriority = [{}]'", GetCleanName(), GetDefaultSpellTypeIdlePriority(i, GetClass(), bot_stance), GetDefaultSpellTypeEngagedPriority(i, GetClass(), bot_stance), GetDefaultSpellTypePursuePriority(i, GetClass(), bot_stance));
		LogBotSettingsDetail("{} says, 'TargetCount = [{}] | AnnounceCast = [{}]'", GetCleanName(), GetDefaultSpellTypeAEOrGroupTargetCount(i, bot_stance), GetDefaultSpellTypeAnnounceCast(i, bot_stance));
	}
}

void Bot::SetBotSpellRecastTimer(uint16 spell_type, Mob* tar, bool precast) {
	if (!tar) {
		return;
	}

	if (!precast && BotSpellTypeUsesTargetSettings(spell_type)) {
		return;
	}

	uint32 added_delay = 0;

	switch (spell_type) { //Additional delays
		case BotSpellTypes::Mez:
			added_delay = RuleI(Bots, MezSuccessDelay);
			break;
		case BotSpellTypes::AEMez:
			added_delay = RuleI(Bots, AEMezSuccessDelay);
			break;
	}

	if (tar->IsPet() && tar->GetOwner() && tar->GetOwner()->IsOfClientBot()) {
		Mob* owner = tar->GetOwner();

		if (owner->IsClient()) {
			owner->CastToClient()->SetSpellTypeRecastTimer(spell_type, (GetUltimateSpellTypeDelay(spell_type, tar) + added_delay));
		}
		else {
			owner->CastToBot()->SetSpellTypeRecastTimer(spell_type, (GetUltimateSpellTypeDelay(spell_type, tar) + added_delay));
		}
	}
	else if (BotSpellTypeUsesTargetSettings(spell_type)) {
		if (tar->IsClient()) {
			tar->CastToClient()->SetSpellTypeRecastTimer(spell_type, (GetUltimateSpellTypeDelay(spell_type, tar) + added_delay));
		}
		else {
			tar->CastToBot()->SetSpellTypeRecastTimer(spell_type, (GetUltimateSpellTypeDelay(spell_type, tar) + added_delay));
		}
	}
	else {
		SetSpellTypeRecastTimer(spell_type, (GetUltimateSpellTypeDelay(spell_type, tar) + added_delay));
	}
}

BotSpell Bot::GetSpellByHealType(uint16 spell_type, Mob* tar) {
	if (!TargetValidation(tar)) {
		BotSpell result;

		return result;
	}

	switch (spell_type) {
		case BotSpellTypes::VeryFastHeals:
		case BotSpellTypes::PetVeryFastHeals:
			return GetBestBotSpellForVeryFastHeal(this, tar, spell_type);
		case BotSpellTypes::FastHeals:
		case BotSpellTypes::PetFastHeals:
			return GetBestBotSpellForFastHeal(this, tar, spell_type);
		case BotSpellTypes::RegularHeal:
		case BotSpellTypes::PetRegularHeals:
			return GetBestBotSpellForRegularSingleTargetHeal(this, tar, spell_type);
		case BotSpellTypes::GroupHeals:
			return GetBestBotSpellForGroupHeal(this, tar, spell_type);
		case BotSpellTypes::CompleteHeal:
		case BotSpellTypes::PetCompleteHeals:
			return GetBestBotSpellForPercentageHeal(this, tar, spell_type);
		case BotSpellTypes::GroupCompleteHeals:
			return GetBestBotSpellForGroupCompleteHeal(this, tar, spell_type);
		case BotSpellTypes::HoTHeals:
		case BotSpellTypes::PetHoTHeals:
			return GetBestBotSpellForHealOverTime(this, tar, spell_type);
		case BotSpellTypes::GroupHoTHeals:
			return GetBestBotSpellForGroupHealOverTime(this, tar, spell_type);
		default:
			return BotSpell(); // Return an empty BotSpell if no valid spell type is found
	}
}

uint16 Bot::GetSpellTypePriority(uint16 spell_type, uint8 priority_type) {
	switch (priority_type) {
		case BotPriorityCategories::Idle:
			return m_bot_spell_settings[spell_type].idle_priority;
		case BotPriorityCategories::Engaged:
			return m_bot_spell_settings[spell_type].engaged_priority;
		case BotPriorityCategories::Pursue:
			return m_bot_spell_settings[spell_type].pursue_priority;
		default:
			return 0;
	}
}

int Bot::GetDefaultSetting(uint16 setting_category, uint16 setting_type, uint8 stance) {
	switch (setting_category) {
		case BotSettingCategories::BaseSetting:
			return GetDefaultBotBaseSetting(setting_type, stance);
		case BotSettingCategories::SpellHold:
			return GetDefaultSpellTypeHold(setting_type, stance);
		case BotSettingCategories::SpellDelay:
			return GetDefaultSpellTypeDelay(setting_type, stance);
		case BotSettingCategories::SpellMinThreshold:
			return GetDefaultSpellTypeMinThreshold(setting_type, stance);
		case BotSettingCategories::SpellMaxThreshold:
			return GetDefaultSpellTypeMaxThreshold(setting_type, stance);
		case BotSettingCategories::SpellTypeResistLimit:
			return GetDefaultSpellTypeResistLimit(setting_type, stance);
		case BotSettingCategories::SpellTypeAggroCheck:
			return GetDefaultSpellTypeAggroCheck(setting_type, stance);
		case BotSettingCategories::SpellTypeMinManaPct:
			return GetDefaultSpellTypeMinManaLimit(setting_type, stance);
		case BotSettingCategories::SpellTypeMaxManaPct:
			return GetDefaultSpellTypeMaxManaLimit(setting_type, stance);
		case BotSettingCategories::SpellTypeMinHPPct:
			return GetDefaultSpellTypeMinHPLimit(setting_type, stance);
		case BotSettingCategories::SpellTypeMaxHPPct:
			return GetDefaultSpellTypeMaxHPLimit(setting_type, stance);
		case BotSettingCategories::SpellTypeIdlePriority:
			return GetDefaultSpellTypePriority(setting_type, BotPriorityCategories::Idle, GetClass(), stance);
		case BotSettingCategories::SpellTypeEngagedPriority:
			return GetDefaultSpellTypePriority(setting_type, BotPriorityCategories::Engaged, GetClass(), stance);
		case BotSettingCategories::SpellTypePursuePriority:
			return GetDefaultSpellTypePriority(setting_type, BotPriorityCategories::Pursue, GetClass(), stance);
		case BotSettingCategories::SpellTypeAEOrGroupTargetCount:
			return GetDefaultSpellTypeAEOrGroupTargetCount(setting_type, stance);
		case BotSettingCategories::SpellTypeAnnounceCast:
			return GetDefaultSpellTypeAnnounceCast(setting_type, stance);
		default:
			return 0; // Default return value for unrecognized categories
	}
}

int Bot::GetSetting(uint16 setting_category, uint16 setting_type) {
	switch (setting_category) {
		case BotSettingCategories::BaseSetting:
			return GetBotBaseSetting(setting_type);
		case BotSettingCategories::SpellHold:
			return GetSpellTypeHold(setting_type);
		case BotSettingCategories::SpellDelay:
			return GetSpellTypeDelay(setting_type);
		case BotSettingCategories::SpellMinThreshold:
			return GetSpellTypeMinThreshold(setting_type);
		case BotSettingCategories::SpellMaxThreshold:
			return GetSpellTypeMaxThreshold(setting_type);
		case BotSettingCategories::SpellTypeResistLimit:
			return GetSpellTypeResistLimit(setting_type);
		case BotSettingCategories::SpellTypeAggroCheck:
			return GetSpellTypeAggroCheck(setting_type);
		case BotSettingCategories::SpellTypeMinManaPct:
			return GetSpellTypeMinManaLimit(setting_type);
		case BotSettingCategories::SpellTypeMaxManaPct:
			return GetSpellTypeMaxManaLimit(setting_type);
		case BotSettingCategories::SpellTypeMinHPPct:
			return GetSpellTypeMinHPLimit(setting_type);
		case BotSettingCategories::SpellTypeMaxHPPct:
			return GetSpellTypeMaxHPLimit(setting_type);
		case BotSettingCategories::SpellTypeIdlePriority:
			return GetSpellTypePriority(setting_type, BotPriorityCategories::Idle);
		case BotSettingCategories::SpellTypeEngagedPriority:
			return GetSpellTypePriority(setting_type, BotPriorityCategories::Engaged);
		case BotSettingCategories::SpellTypePursuePriority:
			return GetSpellTypePriority(setting_type, BotPriorityCategories::Pursue);
		case BotSettingCategories::SpellTypeAEOrGroupTargetCount:
			return GetSpellTypeAEOrGroupTargetCount(setting_type);
		case BotSettingCategories::SpellTypeAnnounceCast:
			return GetSpellTypeAnnounceCast(setting_type);
		default:
			return 0; // Default return value for unrecognized categories
	}
}

bool Bot::GetDefaultSpellTypeHold(uint16 spell_type, uint8 stance) {
	uint8 bot_class = GetClass();

	switch (spell_type) {
		case BotSpellTypes::FastHeals:
		case BotSpellTypes::VeryFastHeals:
		case BotSpellTypes::Pet:
		case BotSpellTypes::Escape:
		case BotSpellTypes::Lifetap:
		case BotSpellTypes::Buff:
		case BotSpellTypes::PetBuffs:
		case BotSpellTypes::InCombatBuff:
		case BotSpellTypes::PreCombatBuff:
		case BotSpellTypes::DamageShields:
		case BotSpellTypes::ResistBuffs:
			return false;
		case BotSpellTypes::GroupCompleteHeals:
		case BotSpellTypes::GroupHeals:
		case BotSpellTypes::GroupHoTHeals:
		case BotSpellTypes::HoTHeals:
		case BotSpellTypes::CompleteHeal:
		case BotSpellTypes::PetFastHeals:
		case BotSpellTypes::PetRegularHeals:
		case BotSpellTypes::PetVeryFastHeals:
		case BotSpellTypes::RegularHeal:
			switch (stance) {
				case Stance::Aggressive:
				case Stance::AEBurn:
				case Stance::Burn:
					return true;
				default:
					return false;
			}
		case BotSpellTypes::Cure:
		case BotSpellTypes::GroupCures:
			switch (stance) {
				case Stance::Aggressive:
				case Stance::AEBurn:
				case Stance::Burn:
					return true;
				default:
					return false;
			}
		case BotSpellTypes::InCombatBuffSong:
		case BotSpellTypes::OutOfCombatBuffSong:
		case BotSpellTypes::PreCombatBuffSong:
			if (bot_class == Class::Bard) {
				return false;
			}
			else {
				return true;
			}
		case BotSpellTypes::Nuke:
		case BotSpellTypes::DOT:
		case BotSpellTypes::Stun:
			switch (stance) {
				case Stance::Assist:
					return true;
				default:
					return false;
			}
		case BotSpellTypes::AENukes:
		case BotSpellTypes::AERains:
		case BotSpellTypes::AEStun:
		case BotSpellTypes::AEDoT:
		case BotSpellTypes::AELifetap:
		case BotSpellTypes::PBAENuke:
			switch (stance) {
				case Stance::AEBurn:
					return false;
				default:
					return true;
			}
		case BotSpellTypes::Mez:
		case BotSpellTypes::AEMez:
		case BotSpellTypes::Debuff:
		case BotSpellTypes::AEDebuff:
		case BotSpellTypes::Slow:
		case BotSpellTypes::AESlow:
		case BotSpellTypes::HateRedux:
			switch (stance) {
				case Stance::AEBurn:
				case Stance::Burn:
				case Stance::Assist:
					return true;
				default:
					return false;
			}
		case BotSpellTypes::Snare:
			switch (stance) {
				case Stance::AEBurn:
				case Stance::Burn:
				case Stance::Assist:
					return true;
				default:
					return false;
			}
		case BotSpellTypes::HateLine:
			if (bot_class == Class::ShadowKnight || bot_class == Class::Paladin) {
				switch (stance) {
					case Stance::Aggressive:
						return false;
					default:
						return true;
				}
			}
			else {
				return true;
			}
		case BotSpellTypes::Charm:
		case BotSpellTypes::Resurrect:
		case BotSpellTypes::AESnare:
		case BotSpellTypes::AERoot:
		case BotSpellTypes::Root:
		case BotSpellTypes::AEDispel:
		case BotSpellTypes::Dispel:
		case BotSpellTypes::AEFear:
		case BotSpellTypes::Fear:
		case BotSpellTypes::AEHateLine:
		case BotSpellTypes::PetCures:
		case BotSpellTypes::PetHoTHeals:
		case BotSpellTypes::PetCompleteHeals:
		case BotSpellTypes::PetDamageShields:
		case BotSpellTypes::PetResistBuffs:
		default:
			return true;
	}
}

uint16 Bot::GetDefaultSpellTypePriority(uint16 spell_type, uint8 priority_type, uint8 bot_class, uint8 stance) {
	switch (priority_type) {
		case BotPriorityCategories::Idle:
			return GetDefaultSpellTypeIdlePriority(spell_type, bot_class, stance);
		case BotPriorityCategories::Engaged:
			return GetDefaultSpellTypeEngagedPriority(spell_type, bot_class, stance);
		case BotPriorityCategories::Pursue:
			return GetDefaultSpellTypePursuePriority(spell_type, bot_class, stance);
		default:
			return 0;
	}
}

uint16 Bot::GetDefaultSpellTypeIdlePriority(uint16 spell_type, uint8 bot_class, uint8 stance) {
	if (!IsBotSpellTypeBeneficial(spell_type)) {
		return 0;
	}

	switch (spell_type) {
		case BotSpellTypes::VeryFastHeals:
			return 1;
		case BotSpellTypes::FastHeals:
			return 2;
		case BotSpellTypes::GroupHeals:
			return 3;
		case BotSpellTypes::RegularHeal:
			return 4;
		case BotSpellTypes::GroupCompleteHeals:
			return 5;
		case BotSpellTypes::CompleteHeal:
			return 6;
		case BotSpellTypes::GroupHoTHeals:
			return 7;
		case BotSpellTypes::HoTHeals:
			return 8;
		case BotSpellTypes::GroupCures:
			return 9;
		case BotSpellTypes::Cure:
			return 10;
		case BotSpellTypes::PetVeryFastHeals:
			return 11;
		case BotSpellTypes::PetFastHeals:
			return 12;
		case BotSpellTypes::PetRegularHeals:
			return 13;
		case BotSpellTypes::PetCompleteHeals:
			return 14;
		case BotSpellTypes::PetHoTHeals:
			return 15;
		case BotSpellTypes::PetCures:
			return 16;
		case BotSpellTypes::Pet:
			return 17;
		case BotSpellTypes::Buff:
			return 18;
		case BotSpellTypes::OutOfCombatBuffSong:
			return 19;
		case BotSpellTypes::ResistBuffs:
			return 20;
		case BotSpellTypes::DamageShields:
			return 21;
		case BotSpellTypes::PetBuffs:
			return 22;
		case BotSpellTypes::PreCombatBuff:
			return 23;
		case BotSpellTypes::PreCombatBuffSong:
			return 24;
		case BotSpellTypes::PetResistBuffs:
			return 25;
		case BotSpellTypes::PetDamageShields:
			return 26;
		default:
			return 0; //unused
	}

	return 0;
}

uint16 Bot::GetDefaultSpellTypeEngagedPriority(uint16 spell_type, uint8 bot_class, uint8 stance) {
	switch (spell_type) {
		case BotSpellTypes::Escape:
			return 1;
		case BotSpellTypes::VeryFastHeals:
			return 2;
		case BotSpellTypes::FastHeals:
			return 3;
		case BotSpellTypes::GroupHeals:
			return 4;
		case BotSpellTypes::RegularHeal:
			return 5;
		case BotSpellTypes::GroupCompleteHeals:
			return 6;
		case BotSpellTypes::CompleteHeal:
			return 7;
		case BotSpellTypes::GroupHoTHeals:
			return 8;
		case BotSpellTypes::HoTHeals:
			return 9;
		case BotSpellTypes::GroupCures:
			return 10;
		case BotSpellTypes::Cure:
			return 11;
		case BotSpellTypes::PetVeryFastHeals:
			return 12;
		case BotSpellTypes::PetFastHeals:
			return 13;
		case BotSpellTypes::PetRegularHeals:
			return 14;
		case BotSpellTypes::PetCompleteHeals:
			return 15;
		case BotSpellTypes::PetHoTHeals:
			return 16;
		case BotSpellTypes::PetCures:
			return 17;
		case BotSpellTypes::AELifetap:
			return 18;
		case BotSpellTypes::Lifetap:
			return 19;
		case BotSpellTypes::HateRedux:
			return 20;
		case BotSpellTypes::AEMez:
			return 21;
		case BotSpellTypes::Mez:
			return 22;
		case BotSpellTypes::AEHateLine:
			return 23;
		case BotSpellTypes::HateLine:
			return 24;
		case BotSpellTypes::AEDispel:
			return 25;
		case BotSpellTypes::Dispel:
			return 26;
		case BotSpellTypes::AEDebuff:
			return 27;
		case BotSpellTypes::Debuff:
			return 28;
		case BotSpellTypes::AESnare:
			return 29;
		case BotSpellTypes::Snare:
			return 30;
		case BotSpellTypes::AESlow:
			return 31;
		case BotSpellTypes::Slow:
			return 32;
		case BotSpellTypes::AERoot:
			return 33;
		case BotSpellTypes::Root:
			return 34;
		case BotSpellTypes::AEDoT:
			return 35;
		case BotSpellTypes::DOT:
			return 36;
		case BotSpellTypes::AEStun:
			return 37;
		case BotSpellTypes::PBAENuke:
			return 38;
		case BotSpellTypes::AENukes:
			return 39;
		case BotSpellTypes::AERains:
			return 40;
		case BotSpellTypes::Stun:
			return 41;
		case BotSpellTypes::Nuke:
			return 42;
		case BotSpellTypes::InCombatBuff:
			return 43;
		case BotSpellTypes::InCombatBuffSong:
			return 44;
		default:
			return 0; //unused
	}
}

uint16 Bot::GetDefaultSpellTypePursuePriority(uint16 spell_type, uint8 bot_class, uint8 stance) {
	switch (spell_type) {
		case BotSpellTypes::Escape:
			return 1;
		case BotSpellTypes::VeryFastHeals:
			return 2;
		case BotSpellTypes::FastHeals:
			return 3;
		case BotSpellTypes::GroupHeals:
			return 4;
		case BotSpellTypes::RegularHeal:
			return 5;
		case BotSpellTypes::GroupCompleteHeals:
			return 6;
		case BotSpellTypes::CompleteHeal:
			return 7;
		case BotSpellTypes::GroupHoTHeals:
			return 8;
		case BotSpellTypes::HoTHeals:
			return 9;
		case BotSpellTypes::GroupCures:
			return 10;
		case BotSpellTypes::Cure:
			return 11;
		case BotSpellTypes::Snare:
			return 12;
		case BotSpellTypes::Lifetap:
			return 13;
		case BotSpellTypes::Dispel:
			return 14;
		case BotSpellTypes::Stun:
			return 15;
		case BotSpellTypes::Nuke:
			return 16;
		case BotSpellTypes::DOT:
			return 17;
		case BotSpellTypes::PetVeryFastHeals:
			return 18;
		case BotSpellTypes::PetFastHeals:
			return 19;
		case BotSpellTypes::PetRegularHeals:
			return 20;
		case BotSpellTypes::PetCompleteHeals:
			return 21;
		case BotSpellTypes::PetHoTHeals:
			return 22;
		case BotSpellTypes::PetCures:
			return 23;
		default:
			return 0; //unused
	}
}

uint16 Bot::GetDefaultSpellTypeResistLimit(uint16 spell_type, uint8 stance) {
	return IsBotSpellTypeBeneficial(spell_type) ? 0 : RuleI(Bots, SpellResistLimit);
}

bool Bot::GetDefaultSpellTypeAggroCheck(uint16 spell_type, uint8 stance) {
	switch (stance) {
		case Stance::AEBurn:
		case Stance::Burn:
		case Stance::Aggressive:
			return false;
		default:
			break;
	}

	switch (spell_type) {
		case BotSpellTypes::AEDebuff:
		case BotSpellTypes::AEDispel:
		case BotSpellTypes::AEDoT:
		case BotSpellTypes::AEFear:
		case BotSpellTypes::AEHateLine:
		case BotSpellTypes::AELifetap:
		case BotSpellTypes::AENukes:
		case BotSpellTypes::AERains:
		case BotSpellTypes::AERoot:
		case BotSpellTypes::AESlow:
		case BotSpellTypes::AESnare:
		case BotSpellTypes::AEStun:
		case BotSpellTypes::DOT:
		case BotSpellTypes::Debuff:
		case BotSpellTypes::Dispel:
		case BotSpellTypes::Fear:
		case BotSpellTypes::HateLine:
		case BotSpellTypes::Nuke:
		case BotSpellTypes::PBAENuke:
		case BotSpellTypes::Root:
		case BotSpellTypes::Slow:
		case BotSpellTypes::Snare:
		case BotSpellTypes::Stun:
			return true;
		default:
			return false;
	}
}

uint8 Bot::GetDefaultSpellTypeMinManaLimit(uint16 spell_type, uint8 stance) {
	return 0;
}

uint8 Bot::GetDefaultSpellTypeMaxManaLimit(uint16 spell_type, uint8 stance) {
	switch (spell_type) {
		case BotSpellTypes::InCombatBuff:
			if (GetClass() == Class::Shaman) {
				return 75;
			}

			break;
		default:
			break;
	}

	return 100;
}

uint8 Bot::GetDefaultSpellTypeMinHPLimit(uint16 spell_type, uint8 stance) {
	switch (spell_type) {
		case BotSpellTypes::InCombatBuff:
			if (GetClass() == Class::Shaman) {
				return 40;
			}

			break;
		default:
			break;
	}

	return 0;
}

uint8 Bot::GetDefaultSpellTypeMaxHPLimit(uint16 spell_type, uint8 stance) {
	return 100;
}

uint16 Bot::GetDefaultSpellTypeAEOrGroupTargetCount(uint16 spell_type, uint8 stance) {
	if (IsAEBotSpellType(spell_type)) {
		return RuleI(Bots, MinTargetsForAESpell);
	}

	if (IsGroupBotSpellType(spell_type)) {
		return RuleI(Bots, MinTargetsForGroupSpell);
	}

	return 1;
}

uint16 Bot::GetDefaultSpellTypeAnnounceCast(uint16 spell_type, uint8 stance) {
	switch (GetClass()) {
		case Class::Bard:
			switch (spell_type) {
				case BotSpellTypes::Mez:
				case BotSpellTypes::AEMez:
				case BotSpellTypes::Cure:
				case BotSpellTypes::GroupCures:
				case BotSpellTypes::PetCures:
				case BotSpellTypes::Charm:
					return 1;
				default:
					return 0;
			}

			return 1;
		default:
			return 1;
	}

	return 1;
}

bool Bot::GetUltimateSpellTypeHold(uint16 spell_type, Mob* tar) {
	if (!TargetValidation(tar)) {
		return GetSpellTypeHold(spell_type);
	}

	Mob* owner = tar->IsPet() ? tar->GetOwner() : nullptr;
	Bot* bot_owner = (owner && owner->IsBot())
		? owner->CastToBot()
		: nullptr;

	return bot_owner
		? bot_owner->GetSpellTypeHold(GetPetBotSpellType(spell_type))
		: GetSpellTypeHold(spell_type);
}

void Bot::SetSpellTypePriority(uint16 spell_type, uint8 priority_type, uint16 priority) {
	switch (priority_type) {
		case BotPriorityCategories::Idle:
			m_bot_spell_settings[spell_type].idle_priority = priority;
			break;
		case BotPriorityCategories::Engaged:
			m_bot_spell_settings[spell_type].engaged_priority = priority;
			break;
		case BotPriorityCategories::Pursue:
			m_bot_spell_settings[spell_type].pursue_priority = priority;
			break;
		default:
			return;
	}
}

std::list<BotSpellTypeOrder> Bot::GetSpellTypesPrioritized(uint8 priority_type) {
	std::list<BotSpellTypeOrder> cast_order;

	for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; i++) {
		BotSpellTypeOrder typeSettings = {
			.spellType = i,
			.priority = GetSpellTypePriority(i, priority_type)
		};

		if (typeSettings.priority != 0) {
			cast_order.emplace_back(typeSettings);
		}
	}

	if (cast_order.size() > 1) {
		cast_order.sort(
			[](BotSpellTypeOrder const& l, BotSpellTypeOrder const& r) {
				return l.priority < r.priority;
			}
		);
	}

	return cast_order;
}

bool Bot::AttemptAICastSpell(uint16 spell_type, Mob* tar) {
	bool result = false;

	if (!tar) {
		if (GetTarget()) {
			tar = GetTarget();
		}
		else {
			if (!IsBotSpellTypeBeneficial(spell_type)) {
				return result;
			}
		}
	}

	if (!IsTaunting() && !IsCommandedSpell() && GetSpellTypeAggroCheck(spell_type) && HasOrMayGetAggro(IsSitting())) {
		LogBotSpellChecksDetail("{} says, 'Cancelling cast of [{}] due to GetSpellTypeAggroCheck and HasOrMayGetAggro.'", GetCleanName(), GetSpellTypeNameByID(spell_type));
		return result;
	}

	if (IsBotSpellTypeBeneficial(spell_type)) {
		if (GetClass() == Class::Bard) {
			tar = this;
		}

		if (!PrecastChecks(tar, spell_type) || !AICastSpell(tar, GetChanceToCastBySpellType(spell_type), spell_type)) {
			return result;
		}
	}
	else {
		if (!PrecastChecks(tar, spell_type) || !AICastSpell(tar, GetChanceToCastBySpellType(spell_type), spell_type)) {
			return result;
		}
	}

	result = true;

	return result;
}

bool Bot::AttemptAACastSpell(Mob* tar, uint16 spell_id, AA::Rank* rank) {
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	SPDat_Spell_Struct spell = spells[spell_id];

	if (!tar || spell.target_type == ST_Self) {
		tar = this;
	}

	if (!DoLosChecks(tar)) {
		return false;
	}

	if (CheckSpellRecastTimer(spell_id)) {
		if (IsBeneficialSpell(spell_id)) {
			if (
				(tar->IsNPC() && !tar->GetOwner()) ||
				(tar->GetOwner() && tar->GetOwner()->IsOfClientBot() && !GetOwner()->IsInGroupOrRaid(tar->GetOwner())) ||
				(tar->IsOfClientBot() && !GetOwner()->IsInGroupOrRaid(tar))
			) {
				GetBotOwner()->Message(Chat::Yellow, "[%s] is an invalid target. Only players or their pet in your group or raid are eligible targets.", tar->GetCleanName());

				return false;
			}
		}

		if (IsDetrimentalSpell(spell_id) && !IsAttackAllowed(tar)) {
			GetBotOwner()->Message(Chat::Yellow, "%s says, 'I cannot attack [%s]'.", GetCleanName(), tar->GetCleanName());

			return false;
		}

		if (!CastChecks(spell_id, tar, UINT16_MAX)) {
			GetBotOwner()->Message(
				Chat::Red,
				fmt::format(
					"{} says, 'Ability failed to cast. This could be due to any number of things: range, mana, line of sight, immune, target type, etc.'",
					GetCleanName()
				).c_str()
			);

			return false;
		}


		if (IsCasting()) {
			RaidGroupSay(
				fmt::format(
					"Interrupting {}. I have been commanded to try to cast an AA - {} on {}.",
					CastingSpellID() ? spells[CastingSpellID()].name : "my spell",
					spell.name,
					tar->GetCleanName()
				).c_str()
			);

			InterruptSpell();
		}

		if (CastSpell(spell_id, tar->GetID())) {
			RaidGroupSay(
				fmt::format(
					"Casting an AA - {} on {}.",
					GetSpellName(spell_id),
					(tar == this ? "myself" : tar->GetCleanName())
				).c_str()
			);

			int timer_duration = (rank->recast_time - GetAlternateAdvancementCooldownReduction(rank)) * 1000;

			if (timer_duration < 0) {
				timer_duration = 0;
			}

			SetSpellRecastTimer(spell_id, timer_duration);
		}
		else {
			GetBotOwner()->Message(
				Chat::Red,
				fmt::format(
					"{} says, 'Ability failed to cast. This could be due to any number of things: range, mana, line of sight, immune, target type, etc.'",
					GetCleanName()
				).c_str()
			);

			return false;
		}
	}
	else {
		GetBotOwner()->Message(
		Chat::Yellow,
			fmt::format(
				"{} says, 'Ability recovery time not yet met. {} remaining.'",
				GetCleanName(),
				Strings::SecondsToTime(GetSpellRecastRemainingTime(spell_id), true)
			).c_str()
		);

		return false;
	}

	return true;
}

bool Bot::AttemptForcedCastSpell(Mob* tar, uint16 spell_id, bool is_disc) {
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	SPDat_Spell_Struct spell = spells[spell_id];

	if (!tar || (spell.target_type == ST_Self && tar != this)) {
		tar = this;
	}

	if ((IsCharmSpell(spell_id) || (IsPetSpell(spell_id) && HasPet()))) {
		return false;
	}

	if (IsResurrectSpell(spell_id) && (!tar->IsCorpse() || !tar->CastToCorpse()->IsPlayerCorpse())) {
		return false;
	}

	if (IsBeneficialSpell(spell_id)) {
		bool invalid_beneficial_target =
			(tar->IsNPC() && !tar->GetOwner()) ||
			(tar->GetOwner() && tar->GetOwner()->IsOfClientBot() && !GetBotOwner()->IsInGroupOrRaid(tar->GetOwner())) ||
			(tar->IsOfClientBot() && !GetBotOwner()->IsInGroupOrRaid(tar));

		if (invalid_beneficial_target) {
			GetBotOwner()->Message(
				Chat::Yellow,
				fmt::format("[{}] is an invalid target. Only players or their pet in your group or raid are eligible targets.", tar->GetCleanName()).c_str()
			);
			return false;
		}
	}

	if (IsDetrimentalSpell(spell_id) && (!GetBotOwner()->IsAttackAllowed(tar) || !IsAttackAllowed(tar))) {
		GetBotOwner()->Message(
			Chat::Yellow,
			fmt::format("{} says, 'I cannot attack [{}]'.", GetCleanName(), tar->GetCleanName()).c_str()
		);
		return false;
	}

	if (!is_disc && !CheckSpellRecastTimer(spell_id)) {
		return false;
	}

	if (!IsInGroupOrRaid(tar, true) &&
		(!RuleB(Bots, EnableBotTGB) || (IsGroupSpell(spell_id) && !IsTGBCompatibleSpell(spell_id)))) {
		return false;
	}

	if (!HasLoS() && !DoLosChecks(tar)) {
		return false;
	}

	if (!CastChecks(spell_id, tar, UINT16_MAX)) {
		GetBotOwner()->Message(
			Chat::Red,
			fmt::format(
				"{} says, 'Ability failed to cast. This could be due to any number of things: range, mana, line of sight, immune, target type, etc.'",
				GetCleanName()
			).c_str()
		);

		return false;
	}

	if (IsCasting()) {
		RaidGroupSay(
			fmt::format(
				"Interrupting {}. I have been commanded to try to cast {} on {}.",
				CastingSpellID() ? spells[CastingSpellID()].name : "my spell",
				spell.name,
				tar->GetCleanName()
			).c_str()
		);
		InterruptSpell();
	}

	if (!CastSpell(spell_id, tar->GetID())) {
		return false;
	}

	RaidGroupSay(
		fmt::format(
			"Casting {} on {}.",
			GetSpellName(spell_id),
			(tar == this ? "myself" : tar->GetCleanName())
		).c_str()
	);

	int timer_duration = 0;

	if (!is_disc) {
		timer_duration = CalcBuffDuration(tar, this, spell_id);
		timer_duration = std::max(0, GetActSpellDuration(spell_id, timer_duration));
		SetSpellRecastTimer(spell_id, timer_duration);
	}
	else if (spell.recast_time > 0) {
		timer_duration = std::max(0, int(spell.recast_time / 1000 - GetFocusEffect(focusReduceRecastTime, spell_id)));
		SetDisciplineReuseTimer(spell_id, timer_duration * 1000);
	}

	return true;
}

uint16 Bot::GetParentSpellType(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::AENukes:
		case BotSpellTypes::AERains:
		case BotSpellTypes::AEStun:
		case BotSpellTypes::PBAENuke:
		case BotSpellTypes::Nuke:
		case BotSpellTypes::Stun:
			return BotSpellTypes::Nuke;
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
			return BotSpellTypes::RegularHeal;
		case BotSpellTypes::Buff:
		case BotSpellTypes::PetBuffs:
		case BotSpellTypes::DamageShields:
		case BotSpellTypes::PetDamageShields:
		case BotSpellTypes::ResistBuffs:
		case BotSpellTypes::PetResistBuffs:
		case BotSpellTypes::BindAffinity:
		case BotSpellTypes::Identify:
		case BotSpellTypes::Levitate:
		case BotSpellTypes::Rune:
		case BotSpellTypes::WaterBreathing:
		case BotSpellTypes::Size:
		case BotSpellTypes::Invisibility:
		case BotSpellTypes::MovementSpeed:
			return BotSpellTypes::Buff;
		case BotSpellTypes::AEMez:
		case BotSpellTypes::Mez:
			return BotSpellTypes::Mez;
		case BotSpellTypes::AEDebuff:
		case BotSpellTypes::Debuff:
			return BotSpellTypes::Debuff;
		case BotSpellTypes::AESlow:
		case BotSpellTypes::Slow:
			return BotSpellTypes::Slow;
		case BotSpellTypes::AESnare:
		case BotSpellTypes::Snare:
			return BotSpellTypes::Snare;
		case BotSpellTypes::AEFear:
		case BotSpellTypes::Fear:
			return BotSpellTypes::Fear;
		case BotSpellTypes::GroupCures:
		case BotSpellTypes::Cure:
		case BotSpellTypes::PetCures:
			return BotSpellTypes::Cure;
		case BotSpellTypes::AERoot:
		case BotSpellTypes::Root:
			return BotSpellTypes::Root;
		case BotSpellTypes::AEDispel:
		case BotSpellTypes::Dispel:
			return BotSpellTypes::Dispel;
		case BotSpellTypes::AEDoT:
		case BotSpellTypes::DOT:
			return BotSpellTypes::DOT;
		case BotSpellTypes::AELifetap:
		case BotSpellTypes::Lifetap:
			return BotSpellTypes::Lifetap;
		case BotSpellTypes::AELull:
		case BotSpellTypes::Lull:
			return BotSpellTypes::Lull;
		case BotSpellTypes::HateLine:
		case BotSpellTypes::AEHateLine:
			return BotSpellTypes::HateLine;
		case BotSpellTypes::Charm:
		case BotSpellTypes::Escape:
		case BotSpellTypes::HateRedux:
		case BotSpellTypes::InCombatBuff:
		case BotSpellTypes::InCombatBuffSong:
		case BotSpellTypes::OutOfCombatBuffSong:
		case BotSpellTypes::Pet:
		case BotSpellTypes::PreCombatBuff:
		case BotSpellTypes::PreCombatBuffSong:
		case BotSpellTypes::Resurrect:
		default:
			return spell_type;
	}

	return spell_type;
}

bool Bot::IsValidBotSpellType(uint16 spell_type) {
	return (
		EQ::ValueWithin(spell_type, BotSpellTypes::START, BotSpellTypes::END) ||
		EQ::ValueWithin(spell_type, BotSpellTypes::COMMANDED_START, BotSpellTypes::COMMANDED_END)
	);
}

bool Bot::IsValidSpellTypeBySpellID(uint16 spell_type, uint16 spell_id) {
	if (IsAEBotSpellType(spell_type) && !IsAnyAESpell(spell_id)) {
		return false;
	}

	if (IsGroupBotSpellType(spell_type) && !IsGroupSpell(spell_id)) {
		return false;
	}

	if (IsBotSpellTypeDetrimental(spell_type) && !IsDetrimentalSpell(spell_id)) {
		return false;
	}

	if (IsBotSpellTypeBeneficial(spell_type) && !IsBeneficialSpell(spell_id)) {
		return false;
	}

	auto spell = spells[spell_id];
	std::string teleport_zone = spell.teleport_zone;

	switch (spell_type) {
		case BotSpellTypes::Nuke:
			if (IsAnyNukeOrStunSpell(spell_id) && !IsStunSpell(spell_id)) {
				return true;
			}

			return false;
		case BotSpellTypes::RegularHeal:
		case BotSpellTypes::PetRegularHeals:
			if (
				IsAnyHealSpell(spell_id) &&
				!IsVeryFastHealSpell(spell_id) &&
				!IsFastHealSpell(spell_id) &&
				!IsCompleteHealSpell(spell_id) &&
				!IsHealOverTimeSpell(spell_id) &&
				!IsBuffSpell(spell_id)
			) {
				return true;
			}

			return false;
		case BotSpellTypes::Root:
		case BotSpellTypes::AERoot:
			if (IsDetrimentalSpell(spell_id) && IsEffectInSpell(spell_id, SpellEffect::Root)) {
				return true;
			}

			return false;
		case BotSpellTypes::Buff:
		case BotSpellTypes::PreCombatBuff:
		case BotSpellTypes::PetBuffs:
			if (
				IsBuffSpell(spell_id) &&
				IsBeneficialSpell(spell_id) &&
				!IsBardSong(spell_id) &&
				!IsResistanceOnlySpell(spell_id) &&
				!IsDamageShieldOnlySpell(spell_id) &&
				!IsDamageShieldAndResistSpell(spell_id)
			) {
				if (
					spell_type != BotSpellTypes::PetBuffs &&
					(spell.target_type == ST_Pet || spell.target_type == ST_SummonedPet)
				) {
					return false;
				}

				return true;
			}

			return false;
		case BotSpellTypes::Escape:
			if (IsEscapeSpell(spell_id)) {
				return true;
			}

			return false;
		case BotSpellTypes::Pet:
			if (IsSummonPetSpell(spell_id) || IsEffectInSpell(spell_id, SpellEffect::TemporaryPets)) {
				return true;
			}

			return false;
		case BotSpellTypes::Lifetap:
		case BotSpellTypes::AELifetap:
			if (IsDetrimentalSpell(spell_id) && IsLifetapSpell(spell_id)) {
				return true;
			}

			return false;
		case BotSpellTypes::Snare:
		case BotSpellTypes::AESnare:
			if (IsDetrimentalSpell(spell_id) && IsEffectInSpell(spell_id, SpellEffect::MovementSpeed)) {
				return true;
			}

			return false;
		case BotSpellTypes::DOT:
		case BotSpellTypes::AEDoT:
			if (
				IsDetrimentalSpell(spell_id) &&
				(IsStackableDOT(spell_id) || IsDamageOverTimeSpell(spell_id))
			) {
				return true;
			}

			return false;
		case BotSpellTypes::Dispel:
		case BotSpellTypes::AEDispel:
			if (IsDispelSpell(spell_id)) {
				return true;
			}

			return false;
		case BotSpellTypes::InCombatBuff:
			if (
				!IsBardSong(spell_id) &&
				(
					(IsSelfConversionSpell(spell_id) && spell.buff_duration < 1) ||
					(spell_type == BotSpellTypes::InCombatBuff && IsAnyBuffSpell(spell_id))
				)
			) {
				return true;
			}

			return false;
		case BotSpellTypes::Mez:
		case BotSpellTypes::AEMez:
			if (IsMesmerizeSpell(spell_id)) {
				return true;
			}

			return false;
		case BotSpellTypes::Charm:
			if (IsCharmSpell(spell_id)) {
				return true;
			}

			return false;
		case BotSpellTypes::Slow:
		case BotSpellTypes::AESlow:
			if (IsDetrimentalSpell(spell_id) && IsSlowSpell(spell_id)) {
				return true;
			}

			return false;
		case BotSpellTypes::Debuff:
		case BotSpellTypes::AEDebuff:
			if (
				IsDebuffSpell(spell_id) &&
				!IsHateReduxSpell(spell_id) &&
				!IsHateSpell(spell_id)
			) {
				return true;
			}

			return false;
		case BotSpellTypes::Cure:
		case BotSpellTypes::GroupCures:
		case BotSpellTypes::PetCures:
			if (IsCureSpell(spell_id)) {
				return true;
			}

			return false;
		case BotSpellTypes::Resurrect:
			if (IsEffectInSpell(spell_id, SpellEffect::Revive)) {
				return true;
			}

			return false;
		case BotSpellTypes::HateRedux:
			if (IsHateReduxSpell(spell_id)) {
				return true;
			}

			return false;
		case BotSpellTypes::InCombatBuffSong:
		case BotSpellTypes::OutOfCombatBuffSong:
		case BotSpellTypes::PreCombatBuffSong:
			if (
				IsBuffSpell(spell_id) &&
				IsBeneficialSpell(spell_id) &&
				IsBardSong(spell_id)
			) {
				return true;
			}

			return false;
		case BotSpellTypes::Fear:
		case BotSpellTypes::AEFear:
			if (IsFearSpell(spell_id)) {
				return true;
			}

			return false;
		case BotSpellTypes::Stun:
		case BotSpellTypes::AEStun:
			if (IsDetrimentalSpell(spell_id) && IsStunSpell(spell_id)) {
				return true;
			}

			return false;
		case BotSpellTypes::HateLine:
		case BotSpellTypes::AEHateLine:
			if (IsDetrimentalSpell(spell_id) && IsHateSpell(spell_id)) {
				return true;
			}

			return false;
		case BotSpellTypes::CompleteHeal:
		case BotSpellTypes::GroupCompleteHeals:
		case BotSpellTypes::PetCompleteHeals:
			if (IsCompleteHealSpell(spell_id) || IsGroupCompleteHealSpell(spell_id)) {
				return true;
			}

			return false;
		case BotSpellTypes::FastHeals:
		case BotSpellTypes::PetFastHeals:
			if (IsFastHealSpell(spell_id)) {
				return true;
			}

			return false;
		case BotSpellTypes::VeryFastHeals:
		case BotSpellTypes::PetVeryFastHeals:
			if (IsVeryFastHealSpell(spell_id)) {
				return true;
			}

			return false;
		case BotSpellTypes::GroupHeals:
			if (IsRegularGroupHealSpell(spell_id)) {
				return true;
			}

			return false;
		case BotSpellTypes::HoTHeals:
		case BotSpellTypes::GroupHoTHeals:
		case BotSpellTypes::PetHoTHeals:
			if (IsHealOverTimeSpell(spell_id) || IsGroupHealOverTimeSpell(spell_id)) {
				return true;
			}

			return false;
		case BotSpellTypes::AENukes:
			if (
				IsDetrimentalSpell(spell_id) &&
				!IsAERainSpell(spell_id) &&
				!IsPBAENukeSpell(spell_id) &&
				!IsStunSpell(spell_id)
			) {
				return true;
			}

			return false;
		case BotSpellTypes::AERains:
			if (IsAERainNukeSpell(spell_id) && !IsStunSpell(spell_id)) {
				return true;
			}

			return false;
		case BotSpellTypes::PBAENuke:
			if (IsPBAENukeSpell(spell_id) && !IsStunSpell(spell_id)) {
				return true;
			}

			return false;
		case BotSpellTypes::ResistBuffs:
		case BotSpellTypes::PetResistBuffs:
			if (IsResistanceBuffSpell(spell_id) && !IsEffectInSpell(spell_id, SpellEffect::DamageShield)) {
				return true;
			}

			return false;
		case BotSpellTypes::DamageShields:
		case BotSpellTypes::PetDamageShields:
			if (IsBeneficialSpell(spell_id) && IsEffectInSpell(spell_id, SpellEffect::DamageShield)) {
				return true;
			}

			return false;
		case BotSpellTypes::Teleport:
			if (
				IsBeneficialSpell(spell_id) &&
				(IsEffectInSpell(spell_id, SpellEffect::Teleport) || IsEffectInSpell(spell_id, SpellEffect::Translocate))
			) {
				return true;
			}

			return false;
		case BotSpellTypes::Lull:
		case BotSpellTypes::AELull:
			if (IsHarmonySpell(spell_id)) {
				return true;
			}

			return false;
		case BotSpellTypes::Succor:
			if (
				IsBeneficialSpell(spell_id) &&
				IsEffectInSpell(spell_id, SpellEffect::Succor)
			) {
				return true;
			}

			return false;
		case BotSpellTypes::BindAffinity:
			if (IsEffectInSpell(spell_id, SpellEffect::BindAffinity)) {
				return true;
			}

			return false;
		case BotSpellTypes::Identify:
			if (IsEffectInSpell(spell_id, SpellEffect::Identify)) {
				return true;
			}

			return false;
		case BotSpellTypes::Levitate:
			if (IsBeneficialSpell(spell_id) && IsEffectInSpell(spell_id, SpellEffect::Levitate)) {
				return true;
			}

			return false;
		case BotSpellTypes::Rune:
			if (IsBeneficialSpell(spell_id) &&
				(IsEffectInSpell(spell_id, SpellEffect::AbsorbMagicAtt) || IsEffectInSpell(spell_id, SpellEffect::Rune))
			) {
				return true;
			}

			return false;
		case BotSpellTypes::WaterBreathing:
			if (IsBeneficialSpell(spell_id) && IsEffectInSpell(spell_id, SpellEffect::WaterBreathing)) {
				return true;
			}

			return false;
		case BotSpellTypes::Size:
			if (
				IsBeneficialSpell(spell_id) &&
				(IsEffectInSpell(spell_id, SpellEffect::ModelSize) || IsEffectInSpell(spell_id, SpellEffect::ChangeHeight))
			) {
				return true;
			}

			return false;
		case BotSpellTypes::Invisibility:
			if (IsBeneficialSpell(spell_id) &&
				(IsEffectInSpell(spell_id, SpellEffect::SeeInvis) ||IsInvisibleSpell(spell_id))
			) {
				return true;
			}

			return false;
		case BotSpellTypes::MovementSpeed:
			if (IsBeneficialSpell(spell_id) && IsEffectInSpell(spell_id, SpellEffect::MovementSpeed)) {
				return true;
			}

			return false;
		case BotSpellTypes::SendHome:
			if (
				IsBeneficialSpell(spell_id) &&
					(
						IsEffectInSpell(spell_id, SpellEffect::GateToHomeCity) ||
							(
								teleport_zone.compare("") &&
								(IsEffectInSpell(spell_id, SpellEffect::Teleport) || IsEffectInSpell(spell_id, SpellEffect::Translocate))
							)
					)
			) {
				return true;
			}

			return false;
		case BotSpellTypes::SummonCorpse:
			if (IsEffectInSpell(spell_id, SpellEffect::SummonCorpse)) {
				return true;
			}

			return false;
		default:
			return true;
	}

	return true;
}

void Bot::SetCastedSpellType(uint16 spell_type) {
	_castedSpellType = spell_type;
}

void Bot::RunToGoalWithJitter(glm::vec3 Goal) {
	RunTo(Goal.x, Goal.y, Goal.z);
	SetCombatJitter();
}

void Bot::SetCombatJitter() {
	if (RuleI(Bots, MaxJitterTimer) > 0) {
		m_combat_jitter_timer.Start(zone->random.Int(RuleI(Bots, MinJitterTimer), RuleI(Bots, MaxJitterTimer)), true);
	}
}

bool Bot::DoCombatPositioning(const CombatPositioningInput& input)
{
	bool adjustment_needed          = false;
	bool is_too_close               = input.tar_distance < input.melee_distance_min;
	bool los_adjust                 = !HasRequiredLoSForPositioning(input.tar);
	bool behind_mob_set             = !input.stop_melee_level &&
		!IsBotRanged() &&
		GetBehindMob();	// Don't want casters or ranged to find positions behind the target.
	bool adjustment_allowed         = !IsMoving() &&
		m_combat_jitter_timer.Check() &&
		(!spellend_timer.Enabled() || GetClass() == Class::Bard);


	if (!IsMoving() && !IsSitting() && !IsFacingMob(input.tar)) {
		FaceTarget(input.tar);
	}

	FindPositionInput find_position_input = {
		.tar                 = input.tar,
		.distance_min        = input.melee_distance_min,
		.distance_max        = input.melee_distance_max,
		.behind_only         = behind_mob_set,
		.front_only          = IsTaunting(),
		.bypass_los          = false,
	};

	bool is_melee = (!input.stop_melee_level && !IsBotRanged());

	if (input.tar->IsRooted() && !IsTaunting()) { // Move non-taunting melee out of range
		adjustment_needed =
			(input.tar_distance <= input.melee_distance_max) &&
			HasTargetReflection();

		if (adjustment_needed && adjustment_allowed) {
			find_position_input.distance_min = input.melee_distance_max + 1;
			find_position_input.distance_max = input.melee_distance_max * 1.25f;

			PlotBotPositionAroundTarget(find_position_input);
		}
	} else {
		if (input.tar->IsFeared()) {
			adjustment_needed = los_adjust;

			if (adjustment_needed && adjustment_allowed) {
				find_position_input.distance_min = input.melee_distance_min;
				find_position_input.distance_max = input.melee_distance;
				find_position_input.behind_only  = false;
				find_position_input.front_only   = false;

				PlotBotPositionAroundTarget(find_position_input);
			}
		}
		else if (IsTaunting() || HasTargetReflection()) { // Taunting/Aggro adjustments
			adjustment_needed =
				(IsTaunting() && is_too_close) ||
				los_adjust ||
				(is_melee && !input.front_mob);

			if (adjustment_needed && adjustment_allowed) {
				find_position_input.distance_min = input.melee_distance_min;
				find_position_input.distance_max = input.melee_distance;
				find_position_input.behind_only  = false;
				find_position_input.front_only   = true;

				PlotBotPositionAroundTarget(find_position_input);
			}
		} else {
			if (input.tar->IsEnraged() && is_melee) { // Move non-taunting melee bots behind target during enrage
				adjustment_needed =
					!behind_mob_set ||
					is_too_close ||
					los_adjust;

				if (adjustment_needed && adjustment_allowed) {
					find_position_input.distance_min = input.melee_distance_min;
					find_position_input.distance_max = input.melee_distance;
					find_position_input.behind_only  = true;
					find_position_input.front_only   = false;

					PlotBotPositionAroundTarget(find_position_input);
				}
			} else { // Regular adjustments
				adjustment_needed =
					is_too_close ||
					los_adjust ||
					(behind_mob_set && !input.behind_mob);

				if (adjustment_needed && adjustment_allowed) {
					find_position_input.distance_min = input.melee_distance_min;
					find_position_input.distance_max = input.melee_distance;

					PlotBotPositionAroundTarget(find_position_input);
				}
			}
		}
	}

	if (!adjustment_needed && IsMoving()) {
		StopMoving();
	}

	return adjustment_needed;
}

bool Bot::PlotBotPositionAroundTarget(const FindPositionInput& input) {
	bool Result = false;

	if (input.tar) {
		glm::vec3 temp_goal(0, 0, input.tar->GetZ());
		glm::vec3 tar_position(input.tar->GetX(), input.tar->GetY(), input.tar->GetZ());
		float look_heading                      = 0;
		float best_z                            = 0;
		float tar_distance                      = 0;
		float desired_angle                     = 0;
		const float offset                      = GetZOffset();
		const uint16 max_iterations_allowed     = 50;
		uint16 counter                          = 0;

		while (counter < max_iterations_allowed) {
			temp_goal.x     = tar_position.x + zone->random.Real(-input.distance_max, input.distance_max);
			temp_goal.y     = tar_position.y + zone->random.Real(-input.distance_max, input.distance_max);
			best_z          = GetFixedZ(temp_goal);

			if (best_z != BEST_Z_INVALID) {
				temp_goal.z = best_z;
			}
			else {
				counter++;

				continue;
			}

			tar_distance = Distance(input.tar->GetPosition(), temp_goal);

			if (tar_distance > input.distance_max || tar_distance < std::max(input.distance_min, (input.distance_max * 0.75f))) {
				counter++;

				continue;
			}

			if (input.front_only && !InFrontMob(input.tar, temp_goal.x, temp_goal.y)) {
				counter++;

				continue;
			}
			else if (input.behind_only && !BehindMob(input.tar, temp_goal.x, temp_goal.y)) {
				counter++;

				continue;
			}

			if (!input.bypass_los && CastToBot()->RequiresLoSForPositioning() && !CheckPositioningLosFN(input.tar, temp_goal.x, temp_goal.y, temp_goal.z)) {
				counter++;

				continue;
			}

			Result = true;
			break;
		}

		if (Result) {
			RunToGoalWithJitter(temp_goal);
		}
	}

	return Result;
}

bool Bot::CheckDoubleRangedAttack() {
	int32 chance = spellbonuses.DoubleRangedAttack + itembonuses.DoubleRangedAttack + aabonuses.DoubleRangedAttack;

	return (
		chance &&
		zone->random.Roll(chance)
	);
}

bool Bot::RequiresLoSForPositioning() {
	if (GetLevel() < GetStopMeleeLevel()) {
		return true;
	}

	for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
		if (IsHealBotSpellType(i) || i == BotSpellTypes::PBAENuke) {
			continue;
		}

		if (!GetSpellTypeHold(i)) {
			return true;
		}
	}

	return false;
}

bool Bot::HasRequiredLoSForPositioning(Mob* tar) {
	if (!tar) {
		return true;
	}

	if (RequiresLoSForPositioning() && !HasLoS()) {
		return false;
	}

	return true;
}

bool Bot::HasValidAETarget(Bot* caster, uint16 spell_id, uint16 spell_type, Mob* tar) {
	int spell_range = caster->GetActSpellRange(spell_id, spells[spell_id].range);
	int spell_ae_range = caster->GetAOERange(spell_id);
	int target_count = 0;

	for (auto& close_mob : caster->m_close_mobs) {
		Mob* m = close_mob.second;

		switch (spell_type) {
			case BotSpellTypes::AELull:
				if (m->GetSpecialAbility(SpecialAbility::PacifyImmunity)) {
					continue;
				}

				break;
			case BotSpellTypes::AEMez:
				if (m->GetSpecialAbility(SpecialAbility::MesmerizeImmunity)) {
					continue;
				}

				break;
			case BotSpellTypes::AEDispel:
				if (m->GetSpecialAbility(SpecialAbility::DispellImmunity)) {
					continue;
				}

				break;
			case BotSpellTypes::AEFear:
				if (m->GetSpecialAbility(SpecialAbility::FearImmunity)) {
					continue;
				}

				break;
			case BotSpellTypes::AESnare:
				if (m->GetSpecialAbility(SpecialAbility::SnareImmunity)) {
					continue;
				}

				break;
			case BotSpellTypes::AESlow:
				if (m->GetSpecialAbility(SpecialAbility::SlowImmunity)) {
					continue;
				}

				break;
			case BotSpellTypes::AEHateLine:
				if (!m->IsNPC()) {
					continue;
				}

				break;
			default:
				break;
		}

		if (
			!m->IsNPC() ||
			(
				!IsCommandedSpell() &&
				!m->CastToNPC()->IsOnHatelist(caster->GetOwner())
			)
		) {
			continue;
		}

		if (SpellBreaksMez(spell_id) && m->IsMezzed()) {
			continue;
		}

		if (IsPBAESpell(spell_id)) {
			if (
				spell_ae_range >= Distance(caster->GetPosition(), m->GetPosition()) &&
				caster->CastChecks(spell_id, m, spell_type, true, true)
			) {
				++target_count;
			}
		}
		else {
			if (
				!tar ||
				spell_range < Distance(caster->GetPosition(), tar->GetPosition()) ||
				!DoLosChecks(m)
			) {
				continue;
			}

			if (
				spell_ae_range >= Distance(tar->GetPosition(), m->GetPosition()) &&
				caster->CastChecks(spell_id, m, spell_type, true, true)
			) {
				++target_count;
			}
		}
	}

	if (target_count < caster->GetSpellTypeAEOrGroupTargetCount(spell_type)) {
		return false;
	}

	return true;
}

void Bot::CopyBotSpellSettings(Bot* to)
{
	if (!to) {
		return;
	}

	to->ResetBotSpellSettings();
	to->bot_spell_settings.clear();

	auto s = BotSpellSettingsRepository::GetWhere(content_db, fmt::format("bot_id = {}", GetBotID()));
	if (s.empty()) {
		return;
	}

	auto* spell_list = content_db.GetBotSpells(to->GetBotSpellID());

	for (const auto& e : s) {
		BotSpellSetting b;

		b.priority = e.priority;
		b.min_hp = e.min_hp;
		b.max_hp = e.max_hp;
		b.is_enabled = e.is_enabled;

		if (IsSpellInBotList(spell_list, e.spell_id)) {
			for (auto& se : spell_list->entries) {
				if (se.spellid == e.spell_id) {
					if (EQ::ValueWithin(to->GetLevel(), se.minlevel, se.maxlevel) && se.spellid) {
						to->AddBotSpellSetting(e.spell_id, &b);
					}
				}
			}
		}
	}

	to->LoadBotSpellSettings();
	to->AI_AddBotSpells(to->GetBotSpellID());
	to->SetBotEnforceSpellSetting(GetBotEnforceSpellSetting());
}

void Bot::ResetBotSpellSettings()
{
	auto s = BotSpellSettingsRepository::GetWhere(content_db, fmt::format("bot_id = {}", GetBotID()));
	if (s.empty()) {
		return;
	}

	for (const auto& e : s) {
		DeleteBotSpellSetting(e.spell_id);
	}

	LoadBotSpellSettings();
	AI_AddBotSpells(GetBotSpellID());
	SetBotEnforceSpellSetting(false);
}

void Bot::CopyBotBlockedBuffs(Bot* to) {
	if (!to) {
		return;
	}

	to->ClearBotBlockedBuffs();

	std::vector<BotBlockedBuffs> blocked_buffs = GetBotBlockedBuffs();

	if (!blocked_buffs.empty()) {
		for (auto& blocked_buff : blocked_buffs) {
			to->SetBotBlockedBuff(blocked_buff.spell_id, blocked_buff.blocked);
		}
	}
}

void Bot::CopyBotBlockedPetBuffs(Bot* to) {
	if (!to) {
		return;
	}

	to->ClearBotBlockedBuffs();

	std::vector<BotBlockedBuffs> blocked_buffs = GetBotBlockedBuffs();

	if (!blocked_buffs.empty()) {
		for (auto& blocked_buff : blocked_buffs) {
			to->SetBotBlockedPetBuff(blocked_buff.spell_id, blocked_buff.blocked_pet);
		}
	}
}

bool Bot::BotPassiveCheck() {
	if (GetBotStance() == Stance::Passive) {
		GetOwner()->Message(
			Chat::Yellow,
			fmt::format(
				"{} says, 'I am currently set to stance {} [#{}]. I cannot do commands and my settings cannot be modified.'",
				GetCleanName(),
				Stance::GetName(Stance::Passive),
				Stance::Passive
			).c_str()
		);

		return true;
	}

	return false;
}

bool Bot::ValidStateCheck(Mob* other, bool same_raid_group) {
	if (
		GetBotStance() == Stance::Passive ||
		GetHoldFlag() ||
		GetAppearance() == eaDead ||
		IsFeared() ||
		IsSilenced() ||
		IsAmnesiad() ||
		GetHP() < 0 ||
		!IsInGroupOrRaid(other, same_raid_group)
	) {
		return false;
	}

	return true;
}

bool Bot::IsValidSpellTypeSubType(uint16 spell_type, uint16 sub_type, uint16 spell_id) {
	if (sub_type == UINT16_MAX) {
		return true;
	}

	switch (sub_type) {
		case CommandedSubTypes::SingleTarget:
			if (
				!IsAnyAESpell(spell_id) &&
				!IsGroupSpell(spell_id)
			) {
				return true;
			}

			break;
		case CommandedSubTypes::GroupTarget:
			if (IsGroupSpell(spell_id)) {
				return true;
			}

			break;
		case CommandedSubTypes::AETarget:
			if (
				IsAnyAESpell(spell_id) &&
				!IsGroupSpell(spell_id)
			) {
				return true;
			}

			break;
		case CommandedSubTypes::SeeInvis:
			if (IsEffectInSpell(spell_id, SpellEffect::SeeInvis)) {
				return true;
			}

			break;
		case CommandedSubTypes::Invis:
			if (
				IsEffectInSpell(spell_id, SpellEffect::Invisibility) ||
				IsEffectInSpell(spell_id, SpellEffect::Invisibility2)
			) {
				return true;
			}

			break;
		case CommandedSubTypes::InvisUndead:
			if (
				IsEffectInSpell(spell_id, SpellEffect::InvisVsUndead) ||
				IsEffectInSpell(spell_id, SpellEffect::InvisVsUndead2)
			) {
				return true;
			}

			break;
		case CommandedSubTypes::InvisAnimals:
			if (
				IsEffectInSpell(spell_id, SpellEffect::InvisVsAnimals) ||
				IsEffectInSpell(spell_id, SpellEffect::ImprovedInvisAnimals)
			) {
				return true;
			}

			break;
		case CommandedSubTypes::Shrink:
			if (
				(
					IsEffectInSpell(spell_id, SpellEffect::ModelSize) &&
					CalcSpellEffectValue(spell_id, GetSpellEffectIndex(spell_id, SpellEffect::ModelSize), GetLevel()) < 100
				)
				||
				(
					IsEffectInSpell(spell_id, SpellEffect::ChangeHeight) &&
					CalcSpellEffectValue(spell_id, GetSpellEffectIndex(spell_id, SpellEffect::ChangeHeight), GetLevel()) < 100
				)
			) {
				return true;
			}

			break;
		case CommandedSubTypes::Grow:
			if (
				(
					IsEffectInSpell(spell_id, SpellEffect::ModelSize) &&
					CalcSpellEffectValue(spell_id, GetSpellEffectIndex(spell_id, SpellEffect::ModelSize), GetLevel()) > 100
				)
				||
				(
					IsEffectInSpell(spell_id, SpellEffect::ChangeHeight) &&
					CalcSpellEffectValue(spell_id, GetSpellEffectIndex(spell_id, SpellEffect::ChangeHeight), GetLevel()) > 100
				)
			) {
				return true;
			}

			break;
		case CommandedSubTypes::Selo:
			if (
				IsBeneficialSpell(spell_id) &&
				IsEffectInSpell(spell_id, SpellEffect::MovementSpeed) &&
				IsBardSong(spell_id)
			) {
				return true;
			}

			break;
		default:
			break;
	}

	return false;
}

uint16 Bot::GetSpellByAA(int id, AA::Rank*& rank) {
	uint16 spell_id = 0;
	std::pair<AA::Ability*, AA::Rank*> aa_ability = std::make_pair(nullptr, nullptr);
	AA::Ability* ability = zone->GetAlternateAdvancementAbility(id);

	if (!ability || !ability->first_rank_id) {
		return spell_id;
	}

	uint32 points = GetAA(ability->first_rank_id);

	if (points > 0) {
		aa_ability = zone->GetAlternateAdvancementAbilityAndRank(ability->id, points);
	}

	rank = aa_ability.second;

	if (!points || !rank) {
		return spell_id;
	}

	spell_id = rank->spell;

	return spell_id;
}

void Bot::SetBotBlockedBuff(uint16 spell_id, bool block)
{
	if (!IsValidSpell(spell_id)) {
		OwnerMessage("Failed to set blocked buff.");
		return;
	}

	auto it = std::find_if(
		bot_blocked_buffs.begin(), bot_blocked_buffs.end(),
		[spell_id](const BotBlockedBuffs& buff) { return buff.spell_id == spell_id; }
	);

	if (it != bot_blocked_buffs.end()) {
		it->blocked = block;
	}
	else {
		bot_blocked_buffs.push_back({ GetBotID(), spell_id, block });
	}

	CleanBotBlockedBuffs();
}

bool Bot::IsBlockedBuff(int32 spell_id)
{
	bool result = false;

	if (!IsValidSpell(spell_id)) {
		OwnerMessage("Failed to get blocked buff.");

		return result;
	}

	CleanBotBlockedBuffs();

	if (!bot_blocked_buffs.empty()) {
		for (int i = 0; i < bot_blocked_buffs.size(); i++) {
			if (bot_blocked_buffs[i].spell_id != spell_id) {
				continue;
			}

			return bot_blocked_buffs[i].blocked;
		}
	}

	return result;
}

void Bot::SetBotBlockedPetBuff(uint16 spell_id, bool block)
{
	if (!IsValidSpell(spell_id)) {
		OwnerMessage("Failed to set blocked pet buff.");
		return;
	}

	auto it = std::find_if(
		bot_blocked_buffs.begin(), bot_blocked_buffs.end(),
		[spell_id](const BotBlockedBuffs& buff) { return buff.spell_id == spell_id; }
	);

	if (it != bot_blocked_buffs.end()) {
		it->blocked_pet = block;
	}
	else {
		bot_blocked_buffs.push_back({ GetBotID(), spell_id, false, block });
	}

	CleanBotBlockedBuffs();
}

bool Bot::IsBlockedPetBuff(int32 spell_id)
{
	bool result = false;

	if (!IsValidSpell(spell_id)) {
		OwnerMessage("Failed to get blocked pet buff.");

		return result;
	}

	CleanBotBlockedBuffs();

	if (!bot_blocked_buffs.empty()) {
		for (int i = 0; i < bot_blocked_buffs.size(); i++) {
			if (bot_blocked_buffs[i].spell_id != spell_id) {
				continue;
			}

			return bot_blocked_buffs[i].blocked_pet;
		}
	}

	return result;
}

void Bot::CleanBotBlockedBuffs()
{
	if (!bot_blocked_buffs.empty()) {
		int current = 0;
		int end = bot_blocked_buffs.size();

		while (current < end) {
			if (!IsValidSpell(bot_blocked_buffs[current].spell_id) || (bot_blocked_buffs[current].blocked == 0 && bot_blocked_buffs[current].blocked_pet == 0)) {
				bot_blocked_buffs.erase(bot_blocked_buffs.begin() + current);
			}
			else {
				current++;
			}

			end = bot_blocked_buffs.size();
		}
	}
}

const std::vector<BotSpells_wIndex>& Bot::BotGetSpellsByType(uint16 spell_type) const {
	auto it = AIBot_spells_by_type.find(spell_type);

	if (it == AIBot_spells_by_type.end() || it->second.empty()) {
		it = AIBot_spells_by_type.find(GetParentSpellType(spell_type));

		if (it == AIBot_spells_by_type.end()) {
			static const std::vector<BotSpells_wIndex> empty;

			return empty;
		}
	}

	return it->second;
}

void Bot::AssignBotSpellsToTypes(std::vector<BotSpells>& AIBot_spells, std::unordered_map<uint16, std::vector<BotSpells_wIndex>>& AIBot_spells_by_type) {
	AIBot_spells_by_type.clear();

	for (size_t i = 0; i < AIBot_spells.size(); ++i) {
		const auto& spell = AIBot_spells[i];

		if (spell.spellid <= 0) {
			continue;
		}

		BotSpells_wIndex spell_with_index{
			static_cast<uint32>(i),
			spell.type,
			spell.spellid,
			spell.manacost,
			spell.time_cancast,
			spell.recast_delay,
			spell.priority,
			spell.resist_adjust,
			spell.minlevel,
			spell.maxlevel,
			spell.min_hp,
			spell.max_hp,
			spell.bucket_name,
			spell.bucket_value,
			spell.bucket_comparison
		};

		AIBot_spells_by_type[spell.type].emplace_back(spell_with_index);
	}
}

std::vector<Mob*> Bot::GatherSpellTargets(bool entire_raid, Mob* target, bool no_clients, bool no_bots) {
	std::vector<Mob*> valid_spell_targets;

	auto is_valid_target = [no_clients, no_bots](Mob* member) {
		return member &&
			   ((member->IsClient() && !no_clients) || (member->IsBot() && !no_bots));
	};

	if (IsRaidGrouped()) {
		Raid* raid = IsBot() ? CastToBot()->GetStoredRaid() : GetRaid();

		if (raid) {
			if (entire_raid) {
				for (const auto& m : raid->members) {
					if (is_valid_target(m.member) && m.group_number != RAID_GROUPLESS) {
						valid_spell_targets.emplace_back(m.member);
					}
				}
			}
			else {
				auto group_name = target ? raid->GetGroup(target->GetName()) : raid->GetGroup(GetName());
				auto raid_group = raid->GetRaidGroupMembers(group_name);

				for (const auto& m : raid_group) {
					if (is_valid_target(m.member) && m.group_number != RAID_GROUPLESS) {
						valid_spell_targets.emplace_back(m.member);
					}
				}
			}
		}
	}
	else if (IsGrouped()) {
		Group* group = GetGroup();

		if (group) {
			for (const auto& m : group->members) {
				if (is_valid_target(m)) {
					valid_spell_targets.emplace_back(m);
				}
			}
		}
	}
	else {
		valid_spell_targets.emplace_back(this);
	}

	return valid_spell_targets;
}

std::vector<Mob*> Bot::GetBuffTargets(Mob* spellTarget) {
	if (RuleB(Bots, RaidBuffing)) {
		return GetSpellTargetList(true);
	}

	return GatherSpellTargets(false, spellTarget);
}

uint8 Bot::GetHPRatioForSpellType(uint16 spell_type, Mob* tar) {
	switch (spell_type) {
		case BotSpellTypes::Escape:
		case BotSpellTypes::HateRedux:
		case BotSpellTypes::InCombatBuff:
		case BotSpellTypes::InCombatBuffSong:
		case BotSpellTypes::AELifetap:
		case BotSpellTypes::Lifetap:
		case BotSpellTypes::OutOfCombatBuffSong:
		case BotSpellTypes::Pet:
		case BotSpellTypes::PreCombatBuff:
		case BotSpellTypes::PreCombatBuffSong:
			return GetHPRatio();
		default:
			return tar->GetHPRatio();
	}

	return tar->GetHPRatio();
}

uint16 Bot::GetPetBotSpellType(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::VeryFastHeals:
			return BotSpellTypes::PetVeryFastHeals;
		case BotSpellTypes::FastHeals:
			return BotSpellTypes::PetFastHeals;
		case BotSpellTypes::RegularHeal:
			return BotSpellTypes::PetRegularHeals;
		case BotSpellTypes::CompleteHeal:
			return BotSpellTypes::PetCompleteHeals;
		case BotSpellTypes::HoTHeals:
			return BotSpellTypes::PetHoTHeals;
		case BotSpellTypes::Buff:
			return BotSpellTypes::PetBuffs;
		case BotSpellTypes::Cure:
			return BotSpellTypes::PetCures;
		case BotSpellTypes::DamageShields:
			return BotSpellTypes::PetDamageShields;
		case BotSpellTypes::ResistBuffs:
			return BotSpellTypes::PetResistBuffs;
		default:
			break;
	}

	return spell_type;
}

uint16 Bot::GetSpellTypeIDByShortName(std::string spell_type_string) {

	for (int i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
		if (!Strings::ToLower(spell_type_string).compare(GetSpellTypeShortNameByID(i))) {
			return i;
		}
	}

	for (int i = BotSpellTypes::COMMANDED_START; i <= BotSpellTypes::COMMANDED_END; ++i) {
		if (!Strings::ToLower(spell_type_string).compare(GetSpellTypeShortNameByID(i))) {
			return i;
		}
	}

	return UINT16_MAX;
}

bool Bot::IsValidBotSpellCategory(uint8 setting_type) {
	return EQ::ValueWithin(setting_type, BotSettingCategories::START, BotSettingCategories::END);
}

std::string Bot::GetBotSpellCategoryName(uint8 setting_type) {
	return Bot::IsValidBotSpellCategory(setting_type) ? bot_setting_category_names[setting_type] : "UNKNOWN CATEGORY";
}

std::string Bot::GetBotSpellCategoryShortName(uint8 setting_type) {
	return Bot::IsValidBotSpellCategory(setting_type) ? bot_setting_category_short_names[setting_type] : "UNKNOWN CATEGORY";
}

uint16 Bot::GetBotSpellCategoryIDByShortName(std::string setting_string) {
	for (int i = BotSettingCategories::START; i <= BotSettingCategories::END; ++i) {
		if (!Strings::ToLower(setting_string).compare(Strings::ToLower(GetBotSpellCategoryShortName(i)))) {
			return i;
		}
	}

	return UINT16_MAX;
}

bool Bot::IsValidBotBaseSetting(uint16 setting_type) {
	return EQ::ValueWithin(setting_type, BotBaseSettings::START_ALL, BotBaseSettings::END);
}

std::string Bot::GetBotSettingCategoryName(uint16 setting_type) {
	return Bot::IsValidBotBaseSetting(setting_type) ? botBaseSettings_names[setting_type] : "UNKNOWN SETTING";
}

uint16 Bot::GetBaseSettingIDByShortName(std::string setting_string) {
	for (int i = BotSettingCategories::START; i <= BotSettingCategories::END; ++i) {
		if (!Strings::ToLower(setting_string).compare(Strings::ToLower(GetBotSettingCategoryName(i)))) {
			return i;
		}
	}

	return UINT16_MAX;
}

std::string Bot::GetSpellTypeShortNameByID(uint16 spell_type) {
	return IsValidBotSpellType(spell_type) ? spell_type_short_names[spell_type] : "UNKNOWN SPELLTYPE";
}

std::string Bot::GetSpellTypeNameByID(uint16 spell_type) {
	return IsValidBotSpellType(spell_type) ? spell_type_names[spell_type] : "UNKNOWN SPELLTYPE";
}

bool Bot::IsValidSubType(uint16 sub_type) {
	return EQ::ValueWithin(sub_type, CommandedSubTypes::START, CommandedSubTypes::END);
}

std::string Bot::GetSubTypeNameByID(uint16 sub_type) {
	return IsValidBotSpellType(sub_type) ? botSubType_names[sub_type] : "UNKNOWN SUBTYPE";
}

uint16 Bot::GetDefaultSpellTypeDelay(uint16 spell_type, uint8 stance) {
	switch (spell_type) {
		case BotSpellTypes::VeryFastHeals:
		case BotSpellTypes::PetVeryFastHeals:
			return 1500;
		case BotSpellTypes::FastHeals:
		case BotSpellTypes::PetFastHeals:
			return 2500;
		case BotSpellTypes::GroupHeals:
		case BotSpellTypes::RegularHeal:
		case BotSpellTypes::PetRegularHeals:
			return 4000;
		case BotSpellTypes::CompleteHeal:
		case BotSpellTypes::GroupCompleteHeals:
		case BotSpellTypes::PetCompleteHeals:
			return 8000;
		case BotSpellTypes::GroupHoTHeals:
		case BotSpellTypes::HoTHeals:
		case BotSpellTypes::PetHoTHeals:
			return 22000;
		case BotSpellTypes::Cure:
			return 2000;
		case BotSpellTypes::GroupCures:
			return 3000;
		case BotSpellTypes::PetCures:
			return 5000;
		case BotSpellTypes::AEDoT:
		case BotSpellTypes::DOT:
			switch (stance) {
				case Stance::AEBurn:
				case Stance::Burn:
					return 100;
				case Stance::Aggressive:
					return 2000;
				case Stance::Efficient:
					return 8000;
				default:
					return 4000;
			}
		case BotSpellTypes::AENukes:
		case BotSpellTypes::AERains:
		case BotSpellTypes::PBAENuke:
		case BotSpellTypes::Nuke:
		case BotSpellTypes::AESnare:
		case BotSpellTypes::Snare:
		case BotSpellTypes::AEDebuff:
		case BotSpellTypes::Debuff:
		case BotSpellTypes::AESlow:
		case BotSpellTypes::Slow:
		case BotSpellTypes::AEStun:
		case BotSpellTypes::Stun:
			switch (stance) {
				case Stance::AEBurn:
				case Stance::Burn:
					return 100;
				case Stance::Aggressive:
					return 3000;
				case Stance::Efficient:
					return 10000;
				default:
					return 6000;
			}
		case BotSpellTypes::AERoot:
		case BotSpellTypes::Root:
			return 8000;
		case BotSpellTypes::Fear:
		case BotSpellTypes::AEFear:
			return 15000;
		default:
			return 100;
	}
}

uint8 Bot::GetDefaultSpellTypeMinThreshold(uint16 spell_type, uint8 stance) {
	switch (spell_type) {
		case BotSpellTypes::AEDebuff:
		case BotSpellTypes::Debuff:
		case BotSpellTypes::AEDispel:
		case BotSpellTypes::Dispel:
		case BotSpellTypes::AESlow:
		case BotSpellTypes::Slow:
			switch (stance) {
				case Stance::AEBurn:
				case Stance::Burn:
				case Stance::Aggressive:
					return 0;
				default:
					return 20;
			}
		case BotSpellTypes::AENukes:
		case BotSpellTypes::AERains:
		case BotSpellTypes::PBAENuke:
		case BotSpellTypes::Nuke:
			switch (stance) {
				case Stance::AEBurn:
				case Stance::Burn:
				case Stance::Aggressive:
					return 0;
				default:
					return 5;
			}
		case BotSpellTypes::AEDoT:
		case BotSpellTypes::DOT:
			switch (stance) {
				case Stance::AEBurn:
				case Stance::Burn:
				case Stance::Aggressive:
					return 0;
				case Stance::Efficient:
					return 40;
				default:
					return 25;
			}
		case BotSpellTypes::Mez:
		case BotSpellTypes::AEMez:
			return 85;
		default:
			return 0;
	}
}

uint8 Bot::GetDefaultSpellTypeMaxThreshold(uint16 spell_type, uint8 stance) {
	uint8 bot_class = GetClass();

	switch (spell_type) {
		case BotSpellTypes::Escape:
		case BotSpellTypes::VeryFastHeals:
		case BotSpellTypes::PetVeryFastHeals:
			switch (stance) {
				case Stance::AEBurn:
				case Stance::Burn:
				case Stance::Aggressive:
					return 40;
				case Stance::Efficient:
				default:
					return 25;
			}
		case BotSpellTypes::AELifetap:
		case BotSpellTypes::Lifetap:
		case BotSpellTypes::FastHeals:
		case BotSpellTypes::PetFastHeals:
			switch (stance) {
				case Stance::AEBurn:
				case Stance::Burn:
				case Stance::Aggressive:
					return 55;
				case Stance::Efficient:
					return 35;
				default:
					return 40;
			}
		case BotSpellTypes::GroupHeals:
		case BotSpellTypes::RegularHeal:
			if (bot_class == Class::Necromancer || bot_class == Class::Shaman) {
				return 60;
			}

			switch (stance) {
				case Stance::AEBurn:
				case Stance::Burn:
				case Stance::Aggressive:
					return 70;
				case Stance::Efficient:
					return 50;
				default:
					return 60;
			}
		case BotSpellTypes::PetRegularHeals:
			switch (stance) {
				case Stance::AEBurn:
				case Stance::Burn:
				case Stance::Aggressive:
					return 70;
				case Stance::Efficient:
					return 50;
				default:
					return 60;
			}
		case BotSpellTypes::CompleteHeal:
		case BotSpellTypes::GroupCompleteHeals:
			if (bot_class == Class::Necromancer || (bot_class == Class::Shaman && !GetSpellTypeHold(BotSpellTypes::InCombatBuff))) {
				return 55;
			}

			switch (stance) {
				case Stance::AEBurn:
				case Stance::Burn:
				case Stance::Aggressive:
					return 90;
				case Stance::Efficient:
					return 65;
				default:
					return 80;
			}
		case BotSpellTypes::PetCompleteHeals:
			switch (stance) {
				case Stance::AEBurn:
				case Stance::Burn:
				case Stance::Aggressive:
					return 90;
				case Stance::Efficient:
					return 65;
				default:
					return 80;
			}
		case BotSpellTypes::AENukes:
		case BotSpellTypes::AERains:
		case BotSpellTypes::PBAENuke:
		case BotSpellTypes::AEStun:
		case BotSpellTypes::Nuke:
		case BotSpellTypes::AEDoT:
		case BotSpellTypes::DOT:
		case BotSpellTypes::AERoot:
		case BotSpellTypes::Root:
		case BotSpellTypes::AESlow:
		case BotSpellTypes::Slow:
		case BotSpellTypes::AESnare:
		case BotSpellTypes::Snare:
		case BotSpellTypes::AEFear:
		case BotSpellTypes::Fear:
		case BotSpellTypes::AEDispel:
		case BotSpellTypes::Dispel:
		case BotSpellTypes::AEDebuff:
		case BotSpellTypes::Debuff:
		case BotSpellTypes::Stun:
			switch (stance) {
				case Stance::AEBurn:
				case Stance::Burn:
					return 100;
				case Stance::Aggressive:
					return 100;
				case Stance::Efficient:
					return 90;
				default:
					return 99;
			}
		case BotSpellTypes::GroupHoTHeals:
		case BotSpellTypes::HoTHeals:
		case BotSpellTypes::PetHoTHeals:
			if (bot_class == Class::Necromancer || (bot_class == Class::Shaman && !GetSpellTypeHold(BotSpellTypes::InCombatBuff))) {
				return 70;
			}
			else {
				switch (stance) {
					case Stance::AEBurn:
					case Stance::Burn:
					case Stance::Aggressive:
						return 95;
					case Stance::Efficient:
						return 80;
					default:
						return 90;
				}
			}
		case BotSpellTypes::Buff:
		case BotSpellTypes::Charm:
		case BotSpellTypes::Cure:
		case BotSpellTypes::GroupCures:
		case BotSpellTypes::PetCures:
		case BotSpellTypes::DamageShields:
		case BotSpellTypes::HateRedux:
		case BotSpellTypes::InCombatBuff:
		case BotSpellTypes::InCombatBuffSong:
		case BotSpellTypes::Mez:
		case BotSpellTypes::AEMez:
		case BotSpellTypes::OutOfCombatBuffSong:
		case BotSpellTypes::Pet:
		case BotSpellTypes::PetBuffs:
		case BotSpellTypes::PreCombatBuff:
		case BotSpellTypes::PreCombatBuffSong:
		case BotSpellTypes::PetDamageShields:
		case BotSpellTypes::PetResistBuffs:
		case BotSpellTypes::ResistBuffs:
		case BotSpellTypes::Resurrect:
		case BotSpellTypes::HateLine:
		case BotSpellTypes::AEHateLine:
		default:
			return 100;
	}
}

uint16 Bot::GetUltimateSpellTypeDelay(uint16 spell_type, Mob* tar) {
	if (!TargetValidation(tar)) {
		return GetSpellTypeDelay(spell_type);
	}

	Mob* owner = tar->IsPet() ? tar->GetOwner() : nullptr;

	if (owner && owner->IsOfClientBot()) {
		return owner->IsClient()
			? owner->CastToClient()->GetSpellTypeDelay(GetPetBotSpellType(spell_type))
			: owner->CastToBot()->GetSpellTypeDelay(GetPetBotSpellType(spell_type));
	}

	if (BotSpellTypeUsesTargetSettings(spell_type) && tar->IsOfClientBot()) {
		return tar->IsClient()
			? tar->CastToClient()->GetSpellTypeDelay(spell_type)
			: tar->CastToBot()->GetSpellTypeDelay(spell_type);
	}

	return GetSpellTypeDelay(spell_type);
}

bool Bot::GetUltimateSpellTypeRecastCheck(uint16 spell_type, Mob* tar) {
	if (!TargetValidation(tar)) {
		return SpellTypeRecastCheck(spell_type);
	}

	Mob* owner = tar->IsPet() ? tar->GetOwner() : nullptr;

	if (owner && owner->IsOfClientBot()) {
		return owner->IsClient()
			? owner->CastToClient()->SpellTypeRecastCheck(GetPetBotSpellType(spell_type))
			: owner->CastToBot()->SpellTypeRecastCheck(GetPetBotSpellType(spell_type));
	}

	if (BotSpellTypeUsesTargetSettings(spell_type) && tar->IsOfClientBot()) {
		return tar->IsClient()
			? tar->CastToClient()->SpellTypeRecastCheck(spell_type)
			: tar->CastToBot()->SpellTypeRecastCheck(spell_type);
	}

	return SpellTypeRecastCheck(spell_type);
}

uint8 Bot::GetUltimateSpellTypeMinThreshold(uint16 spell_type, Mob* tar) {
	if (!TargetValidation(tar)) {
		return GetSpellTypeMinThreshold(spell_type);
	}

	Mob* owner = tar->IsPet() ? tar->GetOwner() : nullptr;

	if (owner && owner->IsOfClientBot()) {
		return owner->IsClient()
			? owner->CastToClient()->GetSpellTypeMinThreshold(GetPetBotSpellType(spell_type))
			: owner->CastToBot()->GetSpellTypeMinThreshold(GetPetBotSpellType(spell_type));
	}

	if (BotSpellTypeUsesTargetSettings(spell_type) && tar->IsOfClientBot()) {
		return tar->IsClient()
			? tar->CastToClient()->GetSpellTypeMinThreshold(spell_type)
			: tar->CastToBot()->GetSpellTypeMinThreshold(spell_type);
	}

	return GetSpellTypeMinThreshold(spell_type);
}

uint8 Bot::GetUltimateSpellTypeMaxThreshold(uint16 spell_type, Mob* tar) {
	if (!TargetValidation(tar)) {
		return GetSpellTypeMaxThreshold(spell_type);
	}

	Mob* owner = tar->IsPet() ? tar->GetOwner() : nullptr;

	if (owner && owner->IsOfClientBot()) {
		return owner->IsClient()
			? owner->CastToClient()->GetSpellTypeMaxThreshold(GetPetBotSpellType(spell_type))
			: owner->CastToBot()->GetSpellTypeMaxThreshold(GetPetBotSpellType(spell_type));
	}

	if (BotSpellTypeUsesTargetSettings(spell_type) && tar->IsOfClientBot()) {
		return tar->IsClient()
			? tar->CastToClient()->GetSpellTypeMaxThreshold(spell_type)
			: tar->CastToBot()->GetSpellTypeMaxThreshold(spell_type);
	}

	return GetSpellTypeMaxThreshold(spell_type);
}

bool Bot::IsImmuneToBotSpell(uint16 spell_id, Mob* caster) {
	int effect_index;

	if (!caster) {
		return false;
	}

	LogSpells("Checking to see if we are immune to spell [{}] cast by [{}]", spell_id, caster->GetName());

	if (!IsValidSpell(spell_id)) {
		return true;
	}

	if (GetSpecialAbility(SpecialAbility::DispellImmunity) && IsDispelSpell(spell_id)) {
		return true;
	}

	if (GetSpecialAbility(SpecialAbility::PacifyImmunity) && IsHarmonySpell(spell_id)) {
		return true;
	}

	if (!GetSpecialAbility(SpecialAbility::MesmerizeImmunity) && IsMesmerizeSpell(spell_id)) {
		// check max level for spell
		effect_index = GetSpellEffectIndex(spell_id, SpellEffect::Mez);
		assert(effect_index >= 0);
		// NPCs get to ignore the max level
		if (
			(GetLevel() > spells[spell_id].max_value[effect_index]) &&
			(!caster->IsNPC() || (caster->IsNPC() && !RuleB(Spells, NPCIgnoreBaseImmunity)))
			) {
			return true;
		}
	}

	// slow and haste spells
	if (GetSpecialAbility(SpecialAbility::SlowImmunity) && IsEffectInSpell(spell_id, SpellEffect::AttackSpeed)) {
		return true;
	}

	// client vs client fear
	if (!GetSpecialAbility(SpecialAbility::FearImmunity) && IsEffectInSpell(spell_id, SpellEffect::Fear)) {
		effect_index = GetSpellEffectIndex(spell_id, SpellEffect::Fear);

		if (IsClient() && caster->IsClient() && (caster->CastToClient()->GetGM() == false)) {
			LogSpells("Clients cannot fear eachother!");
			caster->MessageString(Chat::Red, IMMUNE_FEAR);	// need to verify message type, not in MQ2Cast for easy look up
			return true;
		}
		else if (GetLevel() > spells[spell_id].max_value[effect_index] && spells[spell_id].max_value[effect_index] != 0) {
			return true;
		}
		else if (CheckAATimer(aaTimerWarcry)) {
			return true;
		}
	}

	if (!GetSpecialAbility(SpecialAbility::CharmImmunity) && IsCharmSpell(spell_id)) {

		if (this == caster) {
			return true;
		}

		//let npcs cast whatever charm on anyone
		if (!caster->IsNPC()) {
			// check level limit of charm spell
			effect_index = GetSpellEffectIndex(spell_id, SpellEffect::Charm);
			assert(effect_index >= 0);
			if (GetLevel() > spells[spell_id].max_value[effect_index] && spells[spell_id].max_value[effect_index] != 0) {
				return true;
			}
		}
	}

	if (
		GetSpecialAbility(SpecialAbility::SnareImmunity) &&
		(
			IsEffectInSpell(spell_id, SpellEffect::Root) ||
			IsEffectInSpell(spell_id, SpellEffect::MovementSpeed)
		)
	) {
		return true;
	}

	if (IsLifetapSpell(spell_id)) {
		if (this == caster) {
			return true;
		}
	}

	if (IsSacrificeSpell(spell_id)) {
		if (this == caster) {
			return true;
		}
	}

	return false;
}

std::vector<Mob*> Bot::GetSpellTargetList(bool entire_raid) {
	if (entire_raid && _spell_target_list.empty()) {
		_spell_target_list = GatherSpellTargets(RuleB(Bots, CrossRaidBuffingAndHealing));
	}
	else if (!entire_raid && _group_spell_target_list.empty()) {
		_group_spell_target_list = GatherSpellTargets();
	}

	return entire_raid ? _spell_target_list : _group_spell_target_list;
}

bool Bot::IsValidBotStance(uint8 stance) {
	switch (stance) {
		case Stance::Passive:
		case Stance::Balanced:
		case Stance::Efficient:
		case Stance::Aggressive:
		case Stance::Assist:
		case Stance::Burn:
		case Stance::AEBurn:
			return true;
		default:
			return false;
	}

	return false;
}

bool Bot::HasControllablePet(uint8 ranks_required) {
	if (!GetPet()) {
		return false;
	}

	return GetClass() != Class::Enchanter ||
		GetPet()->GetPetType() != PetType::Animation ||
		GetAA(aaAnimationEmpathy) >= ranks_required;
}
