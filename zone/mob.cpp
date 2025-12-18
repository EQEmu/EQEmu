/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemu.org)

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

#include "mob.h"

#include "common/data_bucket.h"
#include "common/data_verification.h"
#include "common/misc_functions.h"
#include "common/repositories/bot_data_repository.h"
#include "common/repositories/character_data_repository.h"
#include "common/spdat.h"
#include "common/strings.h"
#include "zone/bot.h"
#include "zone/dialogue_window.h"
#include "zone/mob_movement_manager.h"
#include "zone/quest_parser_collection.h"
#include "zone/string_ids.h"
#include "zone/water_map.h"
#include "zone/worldserver.h"

#include <algorithm>
#include <climits>
#include <cmath>
#include <sstream>

extern EntityList entity_list;

extern Zone* zone;
extern WorldServer worldserver;

Mob::Mob(
	const char *in_name,
	const char *in_lastname,
	int64 in_cur_hp,
	int64 in_max_hp,
	uint8 in_gender,
	uint16 in_race,
	uint8 in_class,
	uint8 in_bodytype,
	uint8 in_deity,
	uint8 in_level,
	uint32 in_npctype_id,
	float in_size,
	float in_runspeed,
	const glm::vec4 &position,
	uint8 in_light,
	uint8 in_texture,
	uint8 in_helmtexture,
	uint16 in_ac,
	uint16 in_atk,
	uint16 in_str,
	uint16 in_sta,
	uint16 in_dex,
	uint16 in_agi,
	uint16 in_int,
	uint16 in_wis,
	uint16 in_cha,
	uint8 in_haircolor,
	uint8 in_beardcolor,
	uint8 in_eyecolor1, // the eyecolors always seem to be the same, maybe left and right eye?
	uint8 in_eyecolor2,
	uint8 in_hairstyle,
	uint8 in_luclinface,
	uint8 in_beard,
	uint32 in_drakkin_heritage,
	uint32 in_drakkin_tattoo,
	uint32 in_drakkin_details,
	EQ::TintProfile in_armor_tint,
	uint8 in_aa_title,
	uint16 in_see_invis, // see through invis/ivu
	uint16 in_see_invis_undead,
	uint8 in_see_hide,
	uint8 in_see_improved_hide,
	int64 in_hp_regen,
	int64 in_mana_regen,
	uint8 in_qglobal,
	uint8 in_maxlevel,
	uint32 in_scalerate,
	uint8 in_armtexture,
	uint8 in_bracertexture,
	uint8 in_handtexture,
	uint8 in_legtexture,
	uint8 in_feettexture,
	uint16 in_usemodel,
	bool in_always_aggro,
	int32 in_heroic_strikethrough,
	bool in_keeps_sold_items,
	int64 in_hp_regen_per_second,
	uint32 npc_tint_id
) :
	attack_timer(2000),
	attack_dw_timer(2000),
	ranged_timer(2000),
	hp_regen_per_second_timer(1000),
	m_z_clip_check_timer(1000),
	tic_timer(6000),
	mana_timer(2000),
	spellend_timer(0),
	rewind_timer(30000),
	bindwound_timer(10000),
	stunned_timer(0),
	spun_timer(0),
	bardsong_timer(6000),
	forget_timer(0),
	gravity_timer(1000),
	viral_timer(0),
	m_FearWalkTarget(-999999.0f, -999999.0f, -999999.0f),
	flee_timer(FLEE_CHECK_TIMER),
	m_Position(position),
	tmHidden(-1),
	mitigation_ac(0),
	m_specialattacks(eSpecialAttacks::None),
	attack_anim_timer(100),
	position_update_melee_push_timer(500),
	hate_list_cleanup_timer(6000),
	m_scan_close_mobs_timer(6000),
	m_see_close_mobs_timer(1000),
	m_mob_check_moving_timer(1000),
	bot_attack_flag_timer(10000),
	m_clear_wearchange_cache_timer(60 * 10 * 1000), // 10 minutes
	m_destroying(false)
{
	mMovementManager = &MobMovementManager::Get();
	mMovementManager->AddMob(this);

	targeted          = 0;
	currently_fleeing = false;

	AI_Init();
	SetMoving(false);

	moved              = false;
	turning            = false;
	m_RewindLocation   = glm::vec3();
	m_RelativePosition = glm::vec4();

	name[0]      = 0;
	orig_name[0] = 0;

	clean_name[0] = 0;
	lastname[0]   = 0;

	if (in_name) {
		strn0cpy(name, in_name, 64);
		strn0cpy(orig_name, in_name, 64);
	}

	if (in_lastname) {
		strn0cpy(lastname, in_lastname, 64);
	}

	current_hp    = in_cur_hp;
	max_hp        = in_max_hp;
	base_hp       = in_max_hp;
	gender        = in_gender;
	race          = in_race;
	base_gender   = in_gender;
	base_race     = in_race;
	use_model     = in_usemodel;
	class_        = in_class;
	bodytype      = in_bodytype;
	orig_bodytype = in_bodytype;
	deity         = in_deity;
	level         = in_level;
	orig_level    = in_level;
	npctype_id    = in_npctype_id;
	size          = in_size;
	base_size     = size;
	runspeed      = in_runspeed;
	// neotokyo: sanity check
	if (runspeed < 0 || runspeed > 20) {
		runspeed = 1.25f;
	}

	// clients -- todo movement this doesn't take into account gm speed we need to fix that.
	base_runspeed = (int)((float)runspeed * 40.0f);
	if (runspeed == 0.7f) {
		base_runspeed  = 28;
		walkspeed      = 0.3f;
		base_walkspeed = 12;
		fearspeed      = 0.625f;
		base_fearspeed = 25;
		// npcs
	} else {
		base_walkspeed = base_runspeed * 100 / 265;
		walkspeed      = ((float) base_walkspeed) * 0.025f;
		base_fearspeed = base_runspeed * 100 / 127;
		fearspeed      = ((float) base_fearspeed) * 0.025f;
	}

	last_hp_percent = 0;
	last_hp         = 0;

	current_speed = base_runspeed;

	m_PlayerState = 0;

	// sanity check
	if (runspeed < 0 || runspeed > 20) {
		runspeed = 1.25f;
	}

	m_Light.Type[EQ::lightsource::LightInnate]  = in_light;
	m_Light.Level[EQ::lightsource::LightInnate] = EQ::lightsource::TypeToLevel(m_Light.Type[EQ::lightsource::LightInnate]);
	m_Light.Type[EQ::lightsource::LightActive]  = m_Light.Type[EQ::lightsource::LightInnate];
	m_Light.Level[EQ::lightsource::LightActive] = m_Light.Level[EQ::lightsource::LightInnate];

	texture       = in_texture;
	helmtexture   = in_helmtexture;
	armtexture    = in_armtexture;
	bracertexture = in_bracertexture;
	handtexture   = in_handtexture;
	legtexture    = in_legtexture;
	feettexture   = in_feettexture;
	multitexture  = (armtexture || bracertexture || handtexture || legtexture || feettexture);

	haircolor                   = in_haircolor;
	beardcolor                  = in_beardcolor;
	eyecolor1                   = in_eyecolor1;
	eyecolor2                   = in_eyecolor2;
	hairstyle                   = in_hairstyle;
	luclinface                  = in_luclinface;
	beard                       = in_beard;
	drakkin_heritage            = in_drakkin_heritage;
	drakkin_tattoo              = in_drakkin_tattoo;
	drakkin_details             = in_drakkin_details;
	attack_speed                = 0;
	attack_delay                = 0;
	slow_mitigation             = 0;
	findable                    = false;
	trackable                   = true;
	has_shield_equipped         = false;
	has_two_hand_blunt_equipped = false;
	has_two_hander_equipped     = false;
	has_dual_weapons_equipped   = false;
	can_facestab                = false;
	has_numhits                 = false;
	has_MGB                     = false;
	has_ProjectIllusion         = false;
	SpellPowerDistanceMod       = 0;
	last_los_check              = false;

	aa_title = in_aa_title > 0 ? in_aa_title : 0xFF;

	AC                   = in_ac;
	ATK                  = in_atk;
	STR                  = in_str;
	STA                  = in_sta;
	DEX                  = in_dex;
	AGI                  = in_agi;
	INT                  = in_int;
	WIS                  = in_wis;
	CHA                  = in_cha;
	MR                   = CR = FR = DR = PR = Corrup = PhR = 0;
	extra_haste          = 0;
	bEnraged             = false;
	current_mana         = 0;
	max_mana             = 0;
	hp_regen             = in_hp_regen;
	hp_regen_per_second  = in_hp_regen_per_second;
	mana_regen           = in_mana_regen;
	ooc_regen            = RuleI(NPC, OOCRegen); //default Out of Combat Regen
	maxlevel             = in_maxlevel;
	scalerate            = in_scalerate;
	invisible            = 0;
	invisible_undead     = 0;
	invisible_animals    = 0;
	sneaking             = false;
	hidden               = false;
	improved_hidden      = false;
	invulnerable         = false;
	qglobal              = 0;
	spawned              = false;
	rare_spawn           = false;
	always_aggro         = in_always_aggro;
	heroic_strikethrough = in_heroic_strikethrough;
	keeps_sold_items     = in_keeps_sold_items;
	m_npc_tint_id        = npc_tint_id;

	InitializeBuffSlots();

	feigned = false;

	int max_procs = MAX_PROCS;
	m_max_procs = std::min(RuleI(Combat, MaxProcs), max_procs);

	// clear the proc arrays
	for (int j = 0; j < m_max_procs; j++) {
		PermaProcs[j].spellID             = SPELL_UNKNOWN;
		PermaProcs[j].chance              = 0;
		PermaProcs[j].base_spellID        = SPELL_UNKNOWN;
		PermaProcs[j].level_override      = -1;
		PermaProcs[j].proc_reuse_time     = 0;
		SpellProcs[j].spellID             = SPELL_UNKNOWN;
		SpellProcs[j].chance              = 0;
		SpellProcs[j].base_spellID        = SPELL_UNKNOWN;
		SpellProcs[j].proc_reuse_time     = 0;
		SpellProcs[j].level_override      = -1;
		DefensiveProcs[j].spellID         = SPELL_UNKNOWN;
		DefensiveProcs[j].chance          = 0;
		DefensiveProcs[j].base_spellID    = SPELL_UNKNOWN;
		DefensiveProcs[j].level_override  = -1;
		DefensiveProcs[j].proc_reuse_time = 0;
		RangedProcs[j].spellID            = SPELL_UNKNOWN;
		RangedProcs[j].chance             = 0;
		RangedProcs[j].base_spellID       = SPELL_UNKNOWN;
		RangedProcs[j].level_override     = -1;
		RangedProcs[j].proc_reuse_time    = 0;
	}

	for (int i = EQ::textures::textureBegin; i < EQ::textures::materialCount; i++) {
		armor_tint.Slot[i].Color = in_armor_tint.Slot[i].Color;
	}

	m_Delta   = glm::vec4();
	animation = 0;

	isgrouped     = false;
	israidgrouped = false;

	is_horse = false;

	entity_id_being_looted = 0;
	_appearance            = eaStanding;
	pRunAnimSpeed          = 0;

	spellend_timer.Disable();
	bardsong_timer.Disable();
	bardsong                     = 0;
	bardsong_target_id           = 0;
	casting_spell_id             = 0;
	casting_spell_timer          = 0;
	casting_spell_timer_duration = 0;
	casting_spell_inventory_slot = 0;
	casting_spell_aa_id          = 0;
	casting_spell_recast_adjust  = 0;
	target                       = 0;

	ActiveProjectileATK = false;
	for (int i = 0; i < MAX_SPELL_PROJECTILE; i++) {
		ProjectileAtk[i].increment     = 0;
		ProjectileAtk[i].hit_increment = 0;
		ProjectileAtk[i].target_id     = 0;
		ProjectileAtk[i].wpn_dmg       = 0;
		ProjectileAtk[i].origin_x      = 0.0f;
		ProjectileAtk[i].origin_y      = 0.0f;
		ProjectileAtk[i].origin_z      = 0.0f;
		ProjectileAtk[i].tlast_x       = 0.0f;
		ProjectileAtk[i].tlast_y       = 0.0f;
		ProjectileAtk[i].ranged_id     = 0;
		ProjectileAtk[i].ammo_id       = 0;
		ProjectileAtk[i].ammo_slot     = 0;
		ProjectileAtk[i].skill         = 0;
		ProjectileAtk[i].speed_mod     = 0.0f;
		ProjectileAtk[i].disable_procs = false;
	}

	for (int i = 0; i < MAX_FOCUS_PROC_LIMIT_TIMERS; i++) {
		focusproclimit_spellid[i] = 0;
		focusproclimit_timer[i].Disable();
	}

	for (int i = 0; i < MAX_PROC_LIMIT_TIMERS; i++) {
		spell_proclimit_spellid[i] = 0;
		spell_proclimit_timer[i].Disable();
		ranged_proclimit_spellid[i] = 0;
		ranged_proclimit_timer[i].Disable();
		def_proclimit_spellid[i] = 0;
		def_proclimit_timer[i].Disable();
	}

	memset(&itembonuses, 0, sizeof(StatBonuses));
	memset(&spellbonuses, 0, sizeof(StatBonuses));
	memset(&aabonuses, 0, sizeof(StatBonuses));
	spellbonuses.AggroRange  = -1;
	spellbonuses.AssistRange = -1;
	SetPetID(0);
	SetOwnerID(0);
	SetPetType(PetType::None); // default to not a pet
	SetPetPower(0);
	held              = false;
	gheld             = false;
	nocast            = false;
	focused           = false;
	pet_stop          = false;
	pet_regroup       = false;
	_IsTempPet        = false;
	pet_owner_bot     = false;
	pet_owner_client  = false;
	pet_owner_npc     = false;
	pet_targetlock_id = 0;

	//bot attack flag
	bot_attack_flags.clear();
	bot_attack_flag_timer.Disable();

	attacked_count = 0;
	mezzed         = false;
	stunned        = false;
	silenced       = false;
	amnesiad       = false;

	shield_timer.Disable();
	m_shield_target_id = 0;
	m_shielder_id = 0;
	m_shield_target_mitigation = 0;
	m_shielder_mitigation = 0;
	m_shielder_max_distance = 0;

	destructibleobject = false;
	wandertype         = 0;
	pausetype          = 0;
	cur_wp             = 0;
	m_CurrentWayPoint  = glm::vec4();
	cur_wp_pause       = 0;
	patrol             = 0;
	follow_id          = 0;
	follow_dist        = 100;    // Default Distance for Follow
	follow_run         = true;    // We can run if distance great enough
	no_target_hotkey   = false;
	flee_mode          = false;
	currently_fleeing  = false;
	flee_timer.Start();

	permarooted = (runspeed > 0) ? false : true;

	pause_timer_complete = false;
	ForcedMovement       = 0;
	roamer               = false;
	rooted               = false;
	charmed              = false;

	weaponstance.enabled                  = false;
	weaponstance.spellbonus_enabled       = false; //Set when bonus is applied
	weaponstance.itembonus_enabled        = false; //Set when bonus is applied
	weaponstance.aabonus_enabled          = false; //Controlled by function TogglePassiveAA
	weaponstance.spellbonus_buff_spell_id = 0;
	weaponstance.itembonus_buff_spell_id  = 0;
	weaponstance.aabonus_buff_spell_id    = 0;

	m_pet_order          = PetOrder::Follow;
	m_previous_pet_order = PetOrder::Follow;
	pseudo_rooted     = false;

	nobuff_invisible = 0;
	see_invis        = 0;

	innate_see_invis  = GetSeeInvisibleLevelFromNPCStat(in_see_invis);
	see_invis_undead  = GetSeeInvisibleLevelFromNPCStat(in_see_invis_undead);
	see_hide          = GetSeeInvisibleLevelFromNPCStat(in_see_hide);
	see_improved_hide = GetSeeInvisibleLevelFromNPCStat(in_see_improved_hide);

	qglobal = in_qglobal != 0;

	// Bind wound
	bindwound_timer.Disable();
	bindwound_target = 0;

	trade          = new Trade(this);
	// hp event
	nexthpevent    = -1;
	nextinchpevent = -1;

	hasTempPet    = false;
	count_TempPet = 0;

	m_is_running = false;

	nimbus_effect1 = 0;
	nimbus_effect2 = 0;
	nimbus_effect3 = 0;
	m_targetable   = true;

	m_TargetRing = glm::vec3();

	flymode = GravityBehavior::Water;

	DistractedFromGrid = false;
	hate_list.SetHateOwner(this);

	m_AllowBeneficial = false;
	m_DisableMelee    = false;

	SetMerchantSessionEntityID(0);

	for (int i = 0; i < EQ::skills::HIGHEST_SKILL + 2; i++) {
		SkillDmgTaken_Mod[i] = 0;
	}

	for (int i = 0; i < HIGHEST_RESIST + 2; i++) {
		Vulnerability_Mod[i] = 0;
	}

	for (int i = 0; i < MAX_APPEARANCE_EFFECTS; i++) {
		appearance_effects_id[i]   = 0;
		appearance_effects_slot[i] = 0;
	}

	emoteid              = 0;
	endur_upkeep         = false;
	degenerating_effects = false;
	PrimaryAggro         = false;
	AssistAggro          = false;
	npc_assist_cap       = 0;

	use_double_melee_round_dmg_bonus = false;
	dw_same_delay                    = 0;

	m_manual_follow = false;

	m_scan_close_mobs_timer.Trigger();

	SetCanOpenDoors(true);

	is_boat = IsBoat();

	current_alliance_faction = -1;
}

Mob::~Mob()
{
	m_destroying = true;

	entity_list.RemoveMobFromCloseLists(this);
	m_close_mobs.clear();

	quest_manager.stopalltimers(this);

	mMovementManager->RemoveMob(this);

	AI_Stop();
	if (GetPet()) {
		if (GetPet()->Charmed()) {
			GetPet()->BuffFadeByEffect(SpellEffect::Charm);
		}
		else {
			SetPet(0);
		}
	}

	EQApplicationPacket app;
	CreateDespawnPacket(&app, !IsCorpse());
	Corpse *corpse = entity_list.GetCorpseByID(GetID());
	if (!corpse || (corpse && !corpse->IsPlayerCorpse())) {
		entity_list.QueueClients(this, &app, true);
	}

	entity_list.RemoveFromTargets(this, true);

	if (trade) {
		Mob *with = trade->With();
		if (with && with->IsClient()) {
			with->CastToClient()->FinishTrade(with);
			with->trade->Reset();
		}
		delete trade;
	}

	if (HasTempPetsActive()) {
		entity_list.DestroyTempPets(this);
	}

	entity_list.UnMarkNPC(GetID());
	UninitializeBuffSlots();

	entity_list.RemoveAuraFromMobs(this);

	ClearDataBucketCache();

	LeaveHealRotationTargetPool();
}

uint32 Mob::GetAppearanceValue(EmuAppearance in_appearance) {
	switch (in_appearance) {
		case eaStanding: {
			return Animation::Standing;
		}
		case eaSitting: {
			return Animation::Sitting;
		}
		case eaCrouching: {
			return Animation::Crouching;
		}
		case eaDead: {
			return Animation::Lying;
		}
		case eaLooting: {
			return Animation::Looting;
		}
		case _eaMaxAppearance: {
			break;
		}
	}

	return Animation::Standing;
}


void Mob::CalcSeeInvisibleLevel()
{
	see_invis = std::max({ spellbonuses.SeeInvis, itembonuses.SeeInvis, aabonuses.SeeInvis, innate_see_invis });
}

bool Mob::HasAnInvisibilityEffect() {
	return invisible || hidden || improved_hidden || invisible_animals || invisible_undead;
}

void Mob::BreakCharmPetIfConditionsMet() {
	auto pet = GetPet();
	if (pet && pet->GetPetType() == PetType::Charmed && HasAnInvisibilityEffect()) {
		if (RuleB(Pets, LivelikeBreakCharmOnInvis) || IsInvisible(pet)) {
			pet->BuffFadeByEffect(SpellEffect::Charm);
		}
		LogRules(
			"Pets:LivelikeBreakCharmOnInvis for [{}] invisible [{}] hidden [{}] improved_hidden (shroud of stealth) [{}] invisible_animals [{}] invisible_undead [{}]",
			GetCleanName(),
			invisible,
			hidden,
			improved_hidden,
			invisible_animals,
			invisible_undead
		);
	}
}

void Mob::CalcInvisibleLevel()
{
	uint8 was_invisible = invisible;

	invisible         = std::max({spellbonuses.invisibility, nobuff_invisible});
	invisible_undead  = spellbonuses.invisibility_verse_undead;
	invisible_animals = spellbonuses.invisibility_verse_animal;

	if (was_invisible != invisible) {
		SetInvisible(invisible, true);
		return;
	}

	BreakCharmPetIfConditionsMet();
}

void Mob::SetPetOrder(uint8 pet_order) {
	if (pet_order == PetOrder::Sit || pet_order == PetOrder::Feign) {
		if (m_pet_order == PetOrder::Follow || m_pet_order == PetOrder::Guard) {
			m_previous_pet_order = m_pet_order;
		}
	}

	m_pet_order = pet_order;
}

void Mob::SetInvisible(uint8 state, bool set_on_bonus_calc) {
	if (state == Invisibility::Visible) {
		SendAppearancePacket(AppearanceType::Invisibility, Invisibility::Visible);
		ZeroInvisibleVars(InvisType::T_INVISIBLE);
	} else {
		if (!set_on_bonus_calc) {
			nobuff_invisible = state;
			CalcInvisibleLevel();
		}
		SendAppearancePacket(AppearanceType::Invisibility, invisible);
	}

	BreakCharmPetIfConditionsMet();
}

void Mob::ZeroInvisibleVars(uint8 invisible_type)
{
	switch (invisible_type) {

		case T_INVISIBLE:
			invisible = 0;
			nobuff_invisible = 0;
			break;

		case T_INVISIBLE_VERSE_UNDEAD:
			invisible_undead = 0;
			break;

		case T_INVISIBLE_VERSE_ANIMAL:
			invisible_animals = 0;
			break;
	}
}

//check to see if `this` is invisible to `other`
bool Mob::IsInvisible(Mob* other) const
{
	if (!other) {
		return(false);
	}

	//check regular invisibility
	if (invisible && (invisible > other->SeeInvisible())) {
		return true;
	}

	//check invis vs. undead
	if (other->GetBodyType() == BodyType::Undead || other->GetBodyType() == BodyType::SummonedUndead) {
		if (invisible_undead && (invisible_undead > other->SeeInvisibleUndead())) {
			return true;
		}
	}

	//check invis vs. animals. //TODO: should we have a specific see invisible animal stat or this how live does it?
	if (other->GetBodyType() == BodyType::Animal){
		if (invisible_animals && (invisible_animals > other->SeeInvisible())) {
			return true;
		}
	}

	if(hidden){
		if(!other->see_hide && !other->see_improved_hide){
			return true;
		}
	}

	if(improved_hidden){
		if(!other->see_improved_hide){
			return true;
		}
	}

	//handle sneaking
	if(sneaking) {
		if (BehindMob(other, GetX(), GetY())) {
			return true;
		}
	}

	return(false);
}

int Mob::_GetWalkSpeed() const {

	if (IsRooted() || IsStunned() || IsMezzed())
		return 0;

	else if (IsPseudoRooted())
		return 0;

	int aa_mod = 0;
	int speed_mod = base_walkspeed;
	int base_run = base_runspeed;
	bool has_horse = false;
	int runspeedcap = RuleI(Character,BaseRunSpeedCap);
	runspeedcap += itembonuses.IncreaseRunSpeedCap + spellbonuses.IncreaseRunSpeedCap + aabonuses.IncreaseRunSpeedCap;
	aa_mod += aabonuses.BaseMovementSpeed;

	if (IsClient() && CastToClient()->GetHorseId()) {
		Mob *horse = entity_list.GetMob(CastToClient()->GetHorseId());
		if (horse) {
			speed_mod = horse->GetBaseRunspeed();
			return speed_mod;
		}
	}

	int spell_mod = spellbonuses.movementspeed + itembonuses.movementspeed;
	int movemod = 0;

	if (spell_mod < 0)
		movemod += spell_mod;
	else if (spell_mod > aa_mod)
		movemod = spell_mod;
	else
		movemod = aa_mod;

	// hard cap
	if (runspeedcap > 225)
		runspeedcap = 225;

	if(movemod < -85) //cap it at moving very very slow
		movemod = -85;

	if (!has_horse && movemod != 0)
		speed_mod += (base_run * movemod / 100);

	if(speed_mod < 1)
		return(0);

	//runspeed cap.
	if (IsClient() || IsBot())
	{
		if(speed_mod > runspeedcap)
			speed_mod = runspeedcap;
	}
	return speed_mod;
}

int Mob::_GetRunSpeed() const {
	if (IsRooted() || IsStunned() || IsMezzed() || IsPseudoRooted())
		return 0;

	int aa_mod = 0;
	int speed_mod = base_runspeed;
	int base_walk = base_walkspeed;
	bool has_horse = false;
	if (IsClient())
	{
		if(CastToClient()->GetGMSpeed())
		{
			speed_mod = 325;
		}
		else if (CastToClient()->GetHorseId())
		{
			Mob* horse = entity_list.GetMob(CastToClient()->GetHorseId());
			if(horse)
			{
				speed_mod = horse->GetBaseRunspeed();
				base_walk = horse->GetBaseWalkspeed();
				has_horse = true;
			}
		}
	}

	int runspeedcap = RuleI(Character,BaseRunSpeedCap);
	runspeedcap += itembonuses.IncreaseRunSpeedCap + spellbonuses.IncreaseRunSpeedCap + aabonuses.IncreaseRunSpeedCap;

	aa_mod += aabonuses.BaseMovementSpeed + aabonuses.movementspeed;
	int spell_mod = spellbonuses.movementspeed + itembonuses.movementspeed;
	int movemod = 0;

	if(spell_mod < 0)
	{
		movemod += spell_mod;
	}
	else if(spell_mod > aa_mod)
	{
		movemod = spell_mod;
	}
	else
	{
		movemod = aa_mod;
	}

	if(movemod < -85) //cap it at moving very very slow
		movemod = -85;

	if (!has_horse && movemod != 0)
	{
		if (IsClient() || IsBot())
		{
			speed_mod += (speed_mod * movemod / 100);
		} else {
			if (movemod < 0) {
				speed_mod += (50 * movemod / 100);
				// basically stoped
				if(speed_mod < 1)
				{
					return(0);
				}
				// moving slowly
				if (speed_mod < 8)
					return(8);
			} else {
				speed_mod += GetBaseWalkspeed();
				if (movemod > 50)
					speed_mod += 4;
				if (movemod > 40)
					speed_mod += 3;
			}
		}
	}

	if(speed_mod < 1)
	{
		return(0);
	}
	//runspeed cap.
	if (IsClient() || IsBot())
	{
		if(speed_mod > runspeedcap)
			speed_mod = runspeedcap;
	}
	return speed_mod;
}

int Mob::_GetFearSpeed() const {

	if (IsRooted() || IsStunned() || IsMezzed()) {
		return 0;
	}

	//float speed_mod = fearspeed;
	int speed_mod = GetBaseFearSpeed();

	// use a max of 1.75f in calcs.
	int base_run = std::min(GetBaseRunspeed(), 70);

	int spell_mod = spellbonuses.movementspeed + itembonuses.movementspeed;
	int movemod = 0;

	if(spell_mod < 0)
	{
		movemod += spell_mod;
	}

	if(movemod < -85) //cap it at moving very very slow
		movemod = -85;

	if (IsClient()) {
		if (CastToClient()->GetRunMode())
			speed_mod = GetBaseRunspeed();
		else
			speed_mod = GetBaseWalkspeed();
		if (movemod < 0)
			return GetBaseWalkspeed();
		speed_mod += (base_run * movemod / 100);
		return speed_mod;
	} else {
		int hp_ratio = GetIntHPRatio();
		// very large snares 50% or higher
		if (movemod < -49)
		{
			if (hp_ratio < 25)
			{
				return (0);
			}
			if (hp_ratio < 50)
				return (8);
			else
				return (12);
		}
		if (hp_ratio < 5) {
			speed_mod = base_walkspeed / 3;
		} else if (hp_ratio < 15) {
			speed_mod = base_walkspeed / 2;
		} else if (hp_ratio < 25) {
			speed_mod = base_walkspeed + 1; // add the +1 so they do the run animation
		} else if (hp_ratio < 50) {
			speed_mod *= 82;
			speed_mod /= 100;
		}
		if (movemod > 0) {
			speed_mod += GetBaseWalkspeed();
			if (movemod > 50)
				speed_mod += 4;
			if (movemod > 40)
				speed_mod += 3;
			return speed_mod;
		}
		else if (movemod < 0) {
			speed_mod += (base_run * movemod / 100);
		}
	}
	if (speed_mod < 1)
		return (0);
	if (speed_mod < 9)
		return (8);
	if (speed_mod < 13)
		return (12);

	return speed_mod;
}

int64 Mob::CalcMaxMana()
{
	if (IsIntelligenceCasterClass()) {
		max_mana = (((GetINT() / 2) + 1) * GetLevel()) + spellbonuses.Mana + itembonuses.Mana;
	} else if (IsWisdomCasterClass()) {
		max_mana = (((GetWIS() / 2) + 1) * GetLevel()) + spellbonuses.Mana + itembonuses.Mana;
	} else {
		max_mana = 0;
	}

	if (max_mana < 0) {
		max_mana = 0;
	}

	return max_mana;
}

int64 Mob::CalcMaxHP() {
	max_hp = (base_hp + itembonuses.HP);
	max_hp += max_hp * ((aabonuses.PercentMaxHPChange + spellbonuses.PercentMaxHPChange + itembonuses.PercentMaxHPChange) / 10000.0f);
	max_hp += spellbonuses.FlatMaxHPChange + itembonuses.FlatMaxHPChange + aabonuses.FlatMaxHPChange;

	return max_hp;
}

int64 Mob::GetItemHPBonuses() {
	int64 item_hp = 0;
	item_hp = itembonuses.HP;
	item_hp += item_hp * ((itembonuses.PercentMaxHPChange + spellbonuses.FlatMaxHPChange + aabonuses.FlatMaxHPChange) / 10000.0f);
	return item_hp;
}

int64 Mob::GetSpellHPBonuses() {
	int64 spell_hp = 0;
	spell_hp += spellbonuses.FlatMaxHPChange;
	return spell_hp;
}

bool Mob::IsIntelligenceCasterClass() const
{
	switch (GetClass()) {
		case Class::ShadowKnight:
		case Class::Bard:
		case Class::Necromancer:
		case Class::Wizard:
		case Class::Magician:
		case Class::Enchanter:
		case Class::ShadowKnightGM:
		case Class::BardGM:
		case Class::NecromancerGM:
		case Class::WizardGM:
		case Class::MagicianGM:
		case Class::EnchanterGM:
			return true;
	}

	return false;
}

bool Mob::IsPureMeleeClass() const
{
	switch (GetClass()) {
		case Class::Warrior:
		case Class::Monk:
		case Class::Rogue:
		case Class::Berserker:
		case Class::WarriorGM:
		case Class::MonkGM:
		case Class::RogueGM:
		case Class::BerserkerGM:
			return true;
		default:
			break;
	}

	return false;
}

bool Mob::IsWarriorClass() const
{
	switch (GetClass()) {
		case Class::Warrior:
		case Class::Paladin:
		case Class::Ranger:
		case Class::ShadowKnight:
		case Class::Monk:
		case Class::Bard:
		case Class::Rogue:
		case Class::Beastlord:
		case Class::Berserker:
		case Class::WarriorGM:
		case Class::PaladinGM:
		case Class::RangerGM:
		case Class::ShadowKnightGM:
		case Class::MonkGM:
		case Class::BardGM:
		case Class::RogueGM:
		case Class::BeastlordGM:
		case Class::BerserkerGM:
			return true;
		default:
			break;
	}

	return false;
}

bool Mob::IsWisdomCasterClass() const
{
	switch (GetClass()) {
		case Class::Cleric:
		case Class::Paladin:
		case Class::Ranger:
		case Class::Druid:
		case Class::Shaman:
		case Class::Beastlord:
		case Class::ClericGM:
		case Class::PaladinGM:
		case Class::RangerGM:
		case Class::DruidGM:
		case Class::ShamanGM:
		case Class::BeastlordGM:
			return true;
	}

	return false;
}

uint8 Mob::GetArchetype() const
{
	switch (GetClass()) {
		case Class::Paladin:
		case Class::Ranger:
		case Class::ShadowKnight:
		case Class::Bard:
		case Class::Beastlord:
		case Class::PaladinGM:
		case Class::RangerGM:
		case Class::ShadowKnightGM:
		case Class::BardGM:
		case Class::BeastlordGM:
			return Archetype::Hybrid;
		case Class::Cleric:
		case Class::Druid:
		case Class::Shaman:
		case Class::Necromancer:
		case Class::Wizard:
		case Class::Magician:
		case Class::Enchanter:
		case Class::ClericGM:
		case Class::DruidGM:
		case Class::ShamanGM:
		case Class::NecromancerGM:
		case Class::WizardGM:
		case Class::MagicianGM:
		case Class::EnchanterGM:
			return Archetype::Caster;
		case Class::Warrior:
		case Class::Monk:
		case Class::Rogue:
		case Class::Berserker:
		case Class::WarriorGM:
		case Class::MonkGM:
		case Class::RogueGM:
		case Class::BerserkerGM:
			return Archetype::Melee;
		default:
			break;
	}

	return Archetype::Hybrid;
}

const std::string Mob::GetArchetypeName()
{
	switch (GetArchetype()) {
		case Archetype::Hybrid:
			return "Hybrid";
		case Archetype::Caster:
			return "Caster";
		case Archetype::Melee:
			return "Melee";
		default:
			break;
	}

	return "Hybrid";
}

void Mob::SetSpawnLastNameByClass(NewSpawn_Struct* ns)
{
	switch (ns->spawn.class_) {
		case Class::TributeMaster:
			strcpy(ns->spawn.lastName, "Tribute Master");
			break;
		case Class::GuildTributeMaster:
			strcpy(ns->spawn.lastName, "Guild Tribute Master");
			break;
		case Class::GuildBanker:
			strcpy(ns->spawn.lastName, "Guild Banker");
			break;
		case Class::AdventureRecruiter:
			strcpy(ns->spawn.lastName, "Adventure Recruiter");
			break;
		case Class::AdventureMerchant:
			strcpy(ns->spawn.lastName, "Adventure Merchant");
			break;
		case Class::Banker:
			strcpy(ns->spawn.lastName, "Banker");
			break;
		case Class::WarriorGM:
			strcpy(ns->spawn.lastName, "Warrior Guildmaster");
			break;
		case Class::ClericGM:
			strcpy(ns->spawn.lastName, "Cleric Guildmaster");
			break;
		case Class::PaladinGM:
			strcpy(ns->spawn.lastName, "Paladin Guildmaster");
			break;
		case Class::RangerGM:
			strcpy(ns->spawn.lastName, "Ranger Guildmaster");
			break;
		case Class::ShadowKnightGM:
			strcpy(ns->spawn.lastName, "Shadow Knight Guildmaster");
			break;
		case Class::DruidGM:
			strcpy(ns->spawn.lastName, "Druid Guildmaster");
			break;
		case Class::MonkGM:
			strcpy(ns->spawn.lastName, "Monk Guildmaster");
			break;
		case Class::BardGM:
			strcpy(ns->spawn.lastName, "Bard Guildmaster");
			break;
		case Class::RogueGM:
			strcpy(ns->spawn.lastName, "Rogue Guildmaster");
			break;
		case Class::ShamanGM:
			strcpy(ns->spawn.lastName, "Shaman Guildmaster");
			break;
		case Class::NecromancerGM:
			strcpy(ns->spawn.lastName, "Necromancer Guildmaster");
			break;
		case Class::WizardGM:
			strcpy(ns->spawn.lastName, "Wizard Guildmaster");
			break;
		case Class::MagicianGM:
			strcpy(ns->spawn.lastName, "Magician Guildmaster");
			break;
		case Class::EnchanterGM:
			strcpy(ns->spawn.lastName, "Enchanter Guildmaster");
			break;
		case Class::BeastlordGM:
			strcpy(ns->spawn.lastName, "Beastlord Guildmaster");
			break;
		case Class::BerserkerGM:
			strcpy(ns->spawn.lastName, "Berserker Guildmaster");
			break;
		case Class::MercenaryLiaison:
			strcpy(ns->spawn.lastName, "Mercenary Liaison");
			break;
		default:
			break;
	}
}

void Mob::CreateSpawnPacket(EQApplicationPacket *app, Mob *ForWho)
{
	app->SetOpcode(OP_NewSpawn);
	app->size = sizeof(NewSpawn_Struct);
	safe_delete_array(app->pBuffer);
	app->pBuffer = new uchar[app->size];
	memset(app->pBuffer, 0, app->size);
	auto ns = (NewSpawn_Struct *) app->pBuffer;
	FillSpawnStruct(ns, ForWho);

	if (
		!RuleB(NPC, DisableLastNames) &&
		RuleB(NPC, UseClassAsLastName) &&
		!strlen(ns->spawn.lastName)
	) {
		SetSpawnLastNameByClass(ns);
	}
}

void Mob::CreateSpawnPacket(EQApplicationPacket* app, NewSpawn_Struct* ns) {
	app->SetOpcode(OP_NewSpawn);
	app->size = sizeof(NewSpawn_Struct);
	safe_delete_array(app->pBuffer);
	app->pBuffer = new uchar[sizeof(NewSpawn_Struct)];

	// Copy ns directly into packet
	memcpy(app->pBuffer, ns, sizeof(NewSpawn_Struct));

	// Custom packet data
	auto ns2 = (NewSpawn_Struct*) app->pBuffer;
	strcpy(ns2->spawn.name, ns->spawn.name);

	// Set default Last Names for certain Classes if not defined
	if (
		!RuleB(NPC, DisableLastNames) &&
		RuleB(NPC, UseClassAsLastName) &&
		!strlen(ns->spawn.lastName)
	) {
		SetSpawnLastNameByClass(ns2);
	} else {
		strcpy(ns2->spawn.lastName, ns->spawn.lastName);
	}
}

void Mob::FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho)
{
	int i;

	strcpy(ns->spawn.name, name);
	if(IsClient()) {
		strn0cpy(ns->spawn.lastName, lastname, sizeof(ns->spawn.lastName));
	}

	ns->spawn.heading     = FloatToEQ12(m_Position.w);
	ns->spawn.x           = FloatToEQ19(m_Position.x); //((int32)x_pos)<<3;
	ns->spawn.y           = FloatToEQ19(m_Position.y); //((int32)y_pos)<<3;
	ns->spawn.z           = FloatToEQ19(m_Position.z); //((int32)z_pos)<<3;
	ns->spawn.spawnId     = GetID();
	ns->spawn.curHp       = static_cast<uint8>(GetHPRatio());
	ns->spawn.max_hp      = 100; // this field needs a better name
	ns->spawn.race        = (use_model) ? use_model : race;
	ns->spawn.runspeed    = runspeed;
	ns->spawn.walkspeed   = walkspeed;
	ns->spawn.class_      = class_;
	ns->spawn.gender      = gender;
	ns->spawn.level       = level;
	ns->spawn.PlayerState = GetPlayerState();
	ns->spawn.deity       = deity;
	ns->spawn.animation   = 0;
	ns->spawn.findable    = findable ? 1 : 0;
	ns->spawn.npc_tint_id = GetNpcTintId();

	UpdateActiveLight();
	ns->spawn.light		= m_Light.Type[EQ::lightsource::LightActive];

	ns->spawn.showhelm = helmtexture != std::numeric_limits<uint8>::max() ? 1 : 0;

	ns->spawn.invis		= (invisible || hidden) ? 1 : 0;	// TODO: load this before spawning players
	ns->spawn.NPC		= IsClient() ? 0 : 1;
	ns->spawn.IsMercenary = IsMerc() ? 1 : 0;
	ns->spawn.targetable_with_hotkey = no_target_hotkey ? 0 : 1; // opposite logic!
	ns->spawn.untargetable = IsTargetable();

	ns->spawn.petOwnerId	= ownerid;

	ns->spawn.haircolor = haircolor;
	ns->spawn.beardcolor = beardcolor;
	ns->spawn.eyecolor1 = eyecolor1;
	ns->spawn.eyecolor2 = eyecolor2;
	ns->spawn.hairstyle = hairstyle;
	ns->spawn.face = luclinface;
	ns->spawn.beard = beard;
	ns->spawn.StandState = GetAppearanceValue(_appearance);
	ns->spawn.drakkin_heritage = drakkin_heritage;
	ns->spawn.drakkin_tattoo = drakkin_tattoo;
	ns->spawn.drakkin_details = drakkin_details;
	ns->spawn.equip_chest2 = GetHerosForgeModel(1) != 0 || multitexture? 0xff : texture;

//	ns->spawn.invis2 = 0xff;//this used to be labeled beard.. if its not FF it will turn mob invis

	if (helmtexture && helmtexture != 0xFF && GetHerosForgeModel(0) == 0)
	{
		ns->spawn.helm=helmtexture;
	} else {
		ns->spawn.helm = 0;
	}

	ns->spawn.guildrank	= 0xFF;
	ns->spawn.size = size;
	ns->spawn.bodytype = bodytype;
	// The 'flymode' settings have the following effect:
	// 0 - Mobs in water sink like a stone to the bottom
	// 1 - Same as #flymode 1
	// 2 - Same as #flymode 2
	// 3 - Mobs in water do not sink. A value of 3 in this field appears to be the default setting for all mobs
	// (in water or not) according to 6.2 era packet collects.
	if(IsClient())
		ns->spawn.flymode = FindType(SpellEffect::Levitate) ? 2 : 0;
	else
		ns->spawn.flymode = flymode;

	ns->spawn.lastName[0] = '\0';

	strn0cpy(ns->spawn.lastName, lastname, sizeof(ns->spawn.lastName));

	for (i = 0; i < EQ::textures::materialCount; i++) {
		if (IsPlayerRace(race) || i > EQ::textures::armorFeet) {
			ns->spawn.equipment.Slot[i].Material        = GetEquipmentMaterial(i);
			ns->spawn.equipment.Slot[i].EliteModel      = IsEliteMaterialItem(i);
			ns->spawn.equipment.Slot[i].HerosForgeModel = GetHerosForgeModel(i);
			ns->spawn.equipment_tint.Slot[i].Color      = GetEquipmentColor(i);
		}
	}

	for (i = 0; i < EQ::textures::weaponPrimary; i++) {
		if (texture == std::numeric_limits<uint8>::max()) {
			continue;
		}

		if (i == EQ::textures::armorHead && helmtexture != texture) {
			ns->spawn.equipment.Slot[i].Material = helmtexture;
			continue;
		}

		if (i == EQ::textures::armorArms && armtexture != 0) {
			ns->spawn.equipment.Slot[i].Material = armtexture;
			continue;
		}

		if (i == EQ::textures::armorWrist && bracertexture != 0) {
			ns->spawn.equipment.Slot[i].Material = bracertexture;
			continue;
		}

		if (i == EQ::textures::armorHands && handtexture != 0) {
			ns->spawn.equipment.Slot[i].Material = handtexture;
			continue;
		}

		if (i == EQ::textures::armorLegs && legtexture != 0) {
			ns->spawn.equipment.Slot[i].Material = legtexture;
			continue;
		}

		if (i == EQ::textures::armorFeet && feettexture != 0) {
			ns->spawn.equipment.Slot[i].Material = feettexture;
			continue;
		}

		ns->spawn.equipment.Slot[i].Material = texture;
	}

	memset(ns->spawn.set_to_0xFF, 0xFF, sizeof(ns->spawn.set_to_0xFF));
	if(IsNPC() && IsDestructibleObject())
	{
		ns->spawn.DestructibleObject = true;

		// Changing the first string made it vanish, so it has some significance.
		if(lastname)
			sprintf(ns->spawn.DestructibleModel, "%s", lastname);
		// Changing the second string made no visible difference
		sprintf(ns->spawn.DestructibleName2, "%s", ns->spawn.name);
		// Putting a string in the final one that was previously empty had no visible effect.
		ns->spawn.DestructibleString[0] = '\0';

		// Sets damage appearance level of the object.
		ns->spawn.DestructibleAppearance = luclinface; // Was 0x00000000
		//ns->spawn.DestructibleAppearance = static_cast<EmuAppearance>(_appearance);
		// #appearance 44 1 makes it jump but no visible damage
		// #appearance 44 2 makes it look completely broken but still visible
		// #appearance 44 3 makes it jump but not visible difference to 3
		// #appearance 44 4 makes it disappear altogether
		// #appearance 44 5 makes the client crash.

		ns->spawn.DestructibleUnk1 = 0x00000224;	// Was 0x000001f5;
		// These next 4 are mostly always sequential
		// Originally they were 633, 634, 635, 636
		// Changing them all to 633 - no visible effect.
		// Changing them all to 636 - no visible effect.
		// Reversing the order of these four numbers and then using #appearance gain had no visible change.
		// Setting these four ids to zero had no visible effect when the catapult spawned, nor when #appearance was used.
		ns->spawn.DestructibleID1 = 1968;
		ns->spawn.DestructibleID2 = 1969;
		ns->spawn.DestructibleID3 = 1970;
		ns->spawn.DestructibleID4 = 1971;
		// Next one was originally 0x1ce45008, changing it to 0x00000000 made no visible difference
		ns->spawn.DestructibleUnk2 = 0x13f79d00;
		// Next one was originally 0x1a68fe30, changing it to 0x00000000 made no visible difference
		ns->spawn.DestructibleUnk3 = 0x00000000;
		// Next one was already 0x00000000
		ns->spawn.DestructibleUnk4 = 0x13f79d58;
		// Next one was originally 0x005a69ec, changing it to 0x00000000 made no visible difference.
		ns->spawn.DestructibleUnk5 = 0x13c55b00;
		// Next one was originally 0x1a68fe30, changing it to 0x00000000 made no visible difference.
		ns->spawn.DestructibleUnk6 = 0x00128860;
		// Next one was originally 0x0059de6d, changing it to 0x00000000 made no visible difference.
		ns->spawn.DestructibleUnk7 = 0x005a8f66;
		// Next one was originally 0x00000201, changing it to 0x00000000 made no visible difference.
		// For the Minohten tents, 0x00000000 had them up in the air, while 0x201 put them on the ground.
		// Changing it it 0x00000001 makes the tent sink into the ground.
		ns->spawn.DestructibleUnk8 = 0x01;			// Needs to be 1 for tents?
		ns->spawn.DestructibleUnk9 = 0x00000002;	// Needs to be 2 for tents?

		ns->spawn.flymode = 0;
	}

	if (IsZoneController()) {
		ns->spawn.invis = 255; // gm invis
	}

	if (RuleB(Character, AllowCrossClassTrainers) && ForWho) {
		if (ns->spawn.class_ >= Class::WarriorGM && ns->spawn.class_ <= Class::BerserkerGM) {
			int trainer_class = Class::WarriorGM + (ForWho->GetClass() - 1);
			ns->spawn.class_ = trainer_class;
		}
	}
}

void Mob::CreateDespawnPacket(EQApplicationPacket* app, bool Decay)
{
	app->SetOpcode(OP_DeleteSpawn);
	app->size = sizeof(DeleteSpawn_Struct);
	safe_delete_array(app->pBuffer);
	app->pBuffer = new uchar[app->size];
	memset(app->pBuffer, 0, app->size);
	DeleteSpawn_Struct* ds = (DeleteSpawn_Struct*)app->pBuffer;
	ds->spawn_id = GetID();
	// The next field only applies to corpses. If 0, they vanish instantly, otherwise they 'decay'
	ds->Decay = Decay ? 1 : 0;
}

void Mob::CreateHPPacket(EQApplicationPacket* app)
{
	app->SetOpcode(OP_MobHealth);
	app->size = sizeof(SpawnHPUpdate_Struct2);
	safe_delete_array(app->pBuffer);
	app->pBuffer = new uchar[app->size];
	memset(app->pBuffer, 0, sizeof(SpawnHPUpdate_Struct2));
	SpawnHPUpdate_Struct2* ds = (SpawnHPUpdate_Struct2*)app->pBuffer;

	ds->spawn_id = GetID();
	// they don't need to know the real hp
	ds->hp = (int)GetHPRatio();

	// hp event
	if (IsNPC() && (GetNextHPEvent() > 0)) {
		if (ds->hp < GetNextHPEvent()) {
			std::string export_string = fmt::format("{}", GetNextHPEvent());
			SetNextHPEvent(-1);
			if (parse->HasQuestSub(CastToNPC()->GetNPCTypeID(), EVENT_HP)) {
				parse->EventNPC(EVENT_HP, CastToNPC(), nullptr, export_string, 0);
			}
		}
	}

	if (IsNPC() && (GetNextIncHPEvent() > 0)) {
		if (ds->hp > GetNextIncHPEvent()) {
			std::string export_string = fmt::format("{}", GetNextIncHPEvent());
			SetNextIncHPEvent(-1);
			if (parse->HasQuestSub(CastToNPC()->GetNPCTypeID(), EVENT_HP)) {
				parse->EventNPC(EVENT_HP, CastToNPC(), nullptr, export_string, 1);
			}
		}
	}
}

void Mob::SendHPUpdate(bool force_update_all)
{

	// If our HP is different from last HP update call - let's update selves
	if (IsClient()) {
		if (current_hp != last_hp || force_update_all) {

			LogHPUpdate(
				"Update HP of self [{}] current_hp [{}] max_hp [{}] last_hp [{}]",
				GetCleanName(),
				current_hp,
				max_hp,
				last_hp
			);

			static EQApplicationPacket p(OP_HPUpdate, sizeof(SpawnHPUpdate_Struct));
			auto b = (SpawnHPUpdate_Struct*) p.pBuffer;
			b->cur_hp   = static_cast<uint32>(CastToClient()->GetHP() - itembonuses.HP);
			b->spawn_id = GetID();
			b->max_hp   = CastToClient()->GetMaxHP() - itembonuses.HP;
			CastToClient()->QueuePacket(&p);

			ResetHPUpdateTimer();

			// Used to check if HP has changed to update self next round
			last_hp = current_hp;
		}
	}

	auto current_hp_percent = GetIntHPRatio();

	LogHPUpdateDetail(
		"Client [{}] HP is [{}] last [{}]",
		GetCleanName(),
		current_hp_percent,
		last_hp_percent
	);

	if (current_hp_percent == last_hp_percent && !force_update_all) {
		LogHPUpdateDetail("Same HP for mob [{}] skipping update", GetCleanName());
		ResetHPUpdateTimer();
		return;
	}
	else {

		if (IsClient() && RuleB(Character, MarqueeHPUpdates)) {
			CastToClient()->SendHPUpdateMarquee();
		}

		LogHPUpdate("HP Changed for mob [{}] send update", GetCleanName());

		last_hp_percent = current_hp_percent;
	}

	EQApplicationPacket hp_packet;
	Group               *group = nullptr;

	CreateHPPacket(&hp_packet);

	// update those who have us targeted
	entity_list.QueueClientsByTarget(this, &hp_packet, false, 0, false, true, EQ::versions::maskAllClients);

	// Update those who have us on x-target
	entity_list.QueueClientsByXTarget(this, &hp_packet, false);

	// Update groups using Group LAA health name tag counter
	entity_list.QueueToGroupsForNPCHealthAA(this, &hp_packet);

	// Group
	if (IsGrouped()) {
		group = entity_list.GetGroupByMob(this);
		if (group) {
			group->SendHPPacketsFrom(this);
		}
	}

	// Raid
	if (IsClient()) {
		Raid *raid = entity_list.GetRaidByClient(CastToClient());
		if (raid) {
			raid->SendHPManaEndPacketsFrom(this);
		}
	}

	// Pet
	if (GetOwner() && GetOwner()->IsClient()) {
		GetOwner()->CastToClient()->QueuePacket(&hp_packet, false);
		group = entity_list.GetGroupByClient(GetOwner()->CastToClient());

		if (group) {
			group->SendHPPacketsFrom(this);
		}

		Raid *raid = entity_list.GetRaidByClient(GetOwner()->CastToClient());
		if (raid) {
			raid->SendHPManaEndPacketsFrom(this);
		}
	}

	if (RuleB(Bots, Enabled) && GetOwner() && GetOwner()->IsBot() && GetOwner()->CastToBot()->GetBotOwner() && GetOwner()->CastToBot()->GetBotOwner()->IsClient()) {
		auto bot_owner = GetOwner()->CastToBot()->GetBotOwner()->CastToClient();
		if (bot_owner) {
			bot_owner->QueuePacket(&hp_packet, false);
			group = entity_list.GetGroupByClient(bot_owner);

			if (group) {
				group->SendHPPacketsFrom(this);
			}

			Raid *raid = entity_list.GetRaidByClient(bot_owner);
			if (raid) {
				raid->SendHPManaEndPacketsFrom(this);
			}
		}
	}

	if (GetPet() && GetPet()->IsClient()) {
		GetPet()->CastToClient()->QueuePacket(&hp_packet, false);
	}

	/**
	 * Destructible objects
	 */
	if (IsNPC() && IsDestructibleObject()) {
		if (GetHPRatio() > 74) {
			if (GetAppearance() != eaStanding) {
				SendAppearancePacket(AppearanceType::DamageState, eaStanding);
				_appearance = eaStanding;
			}
		}
		else if (GetHPRatio() > 49) {
			if (GetAppearance() != eaSitting) {
				SendAppearancePacket(AppearanceType::DamageState, eaSitting);
				_appearance = eaSitting;
			}
		}
		else if (GetHPRatio() > 24) {
			if (GetAppearance() != eaCrouching) {
				SendAppearancePacket(AppearanceType::DamageState, eaCrouching);
				_appearance = eaCrouching;
			}
		}
		else if (GetHPRatio() > 0) {
			if (GetAppearance() != eaDead) {
				SendAppearancePacket(AppearanceType::DamageState, eaDead);
				_appearance = eaDead;
			}
		}
		else if (GetAppearance() != eaLooting) {
			SendAppearancePacket(AppearanceType::DamageState, eaLooting);
			_appearance = eaLooting;
		}
	}
}

void Mob::SendRename(Mob *sender, const char* old_name, const char* new_name)
{
	auto out2 = new EQApplicationPacket(OP_MobRename, sizeof(MobRename_Struct));
	auto data = (MobRename_Struct *)out2->pBuffer;
	out2->priority = 6;

	strn0cpy(data->old_name, old_name, sizeof(data->old_name));
	strn0cpy(data->old_name_again, old_name, sizeof(data->old_name_again));
	strn0cpy(data->new_name, new_name, sizeof(data->new_name));
	data->unknown192 = 0;
	data->unknown196 = 1;

	entity_list.QueueClients(sender, out2);
	safe_delete(out2);
}

void Mob::StopMoving()
{
	StopNavigation();

	if (moved) {
		moved = false;
	}
}

void Mob::StopMoving(float new_heading)
{
	StopNavigation();
	RotateTo(new_heading);

	if (moved) {
		moved = false;
	}
}

void Mob::SentPositionPacket(float dx, float dy, float dz, float dh, int anim, bool send_to_self)
{
	EQApplicationPacket outapp(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
	PlayerPositionUpdateServer_Struct *spu = (PlayerPositionUpdateServer_Struct*)outapp.pBuffer;

	memset(spu, 0x00, sizeof(PlayerPositionUpdateServer_Struct));
	spu->spawn_id = GetID();
	spu->x_pos = FloatToEQ19(GetX());
	spu->y_pos = FloatToEQ19(GetY());
	spu->z_pos = FloatToEQ19(GetZ());
	spu->heading = FloatToEQ12(GetHeading());
	spu->delta_x = FloatToEQ13(dx);
	spu->delta_y = FloatToEQ13(dy);
	spu->delta_z = FloatToEQ13(dz);
	spu->delta_heading = FloatToEQ10(dh);
	spu->animation = anim;

	entity_list.QueueClients(this, &outapp, send_to_self == false, false);
}

// this is for SendPosUpdate()
void Mob::MakeSpawnUpdate(PlayerPositionUpdateServer_Struct* spu) {
	spu->spawn_id = GetID();
	spu->x_pos = FloatToEQ19(m_Position.x);
	spu->y_pos = FloatToEQ19(m_Position.y);
	spu->z_pos = FloatToEQ19(m_Position.z);
	spu->delta_x = FloatToEQ13(m_Delta.x);
	spu->delta_y = FloatToEQ13(m_Delta.y);
	spu->delta_z = FloatToEQ13(m_Delta.z);
	spu->heading = FloatToEQ12(m_Position.w);
	if (IsClient() || IsBot())
		spu->animation = animation;
	else
		spu->animation = pRunAnimSpeed;//animation;

	spu->delta_heading = FloatToEQ10(m_Delta.w);
}

void Mob::SendStatsWindow(Client* c, bool use_window)
{
	if (!IsOfClientBot()) {
		return;
	}

	const std::string& color_red    = "red_1";
	const std::string& color_blue   = "royal_blue";
	const std::string& color_green  = "forest_green";
	const std::string& bright_green = "green";
	const std::string& bright_red   = "red";
	const std::string& heroic_color = "gold";

	// Health, Mana, and Endurance
	std::string HME_row;
	std::string hme_color;
	std::string cur_field;
	std::string total_field;
	std::string cur_name;
	std::string cur_color;

	auto hme_rows = 3; // Rows in display

	for (auto hme_row_counter = 0; hme_row_counter < hme_rows; hme_row_counter++) {
		switch (hme_row_counter) {
			case 0: {
				cur_name    = "Health ";
				cur_field   = Strings::Commify(GetHP());
				total_field = Strings::Commify(GetMaxHP());
				hme_color   = color_red;
				break;
			}
			case 1: {
				if (CalcMaxMana()) {
					cur_name    = "Mana ";
					cur_field   = Strings::Commify(GetMana());
					total_field = Strings::Commify(GetMaxMana());
					hme_color   = color_blue;
				} else {
					continue;
				}

				break;
			}
			case 2: {
				cur_name    = "Endurance ";
				cur_field   = Strings::Commify(GetEndurance());
				total_field = Strings::Commify(GetMaxEndurance());
				hme_color   = color_green;
				break;
			}
			default: {
				break;
			}
		}

		if (!cur_field.compare(total_field)) {
			cur_color = bright_green;
		} else {
			cur_color = bright_red;
		}

		HME_row += DialogueWindow::TableRow(
			fmt::format(
				"{}{}",
				DialogueWindow::TableCell(DialogueWindow::ColorMessage(hme_color, cur_name)),
				DialogueWindow::TableCell(
					fmt::format(
						"{} / {}",
						DialogueWindow::ColorMessage(cur_color, cur_field),
						DialogueWindow::ColorMessage(bright_green, total_field)
					)
				)
			)
		);
	}

	// Regen
	std::string regen_string;
	std::string regen_row_header;
	std::string regen_row_color;
	std::string base_regen_field;
	std::string item_regen_field;
	std::string cap_regen_field;
	std::string spell_regen_field;
	std::string aa_regen_field;
	std::string total_regen_field;

	auto regen_rows = 3;

	for (auto regen_row_counter = 0; regen_row_counter < regen_rows; regen_row_counter++) {
		switch (regen_row_counter) {
			case 0: {
				regen_row_header = "Health";
				regen_row_color = color_red;

				if (IsBot()) {
					base_regen_field = Strings::Commify(CastToBot()->LevelRegen());
				} else if (IsClient()) {
					base_regen_field = Strings::Commify(CastToClient()->LevelRegen());
				}

				item_regen_field  = Strings::Commify(itembonuses.HPRegen + itembonuses.heroic_hp_regen);
				cap_regen_field   = Strings::Commify(CalcHPRegenCap());
				spell_regen_field = Strings::Commify(spellbonuses.HPRegen);
				aa_regen_field    = Strings::Commify(aabonuses.HPRegen);

				if (IsBot()) {
					total_regen_field = Strings::Commify(CastToBot()->CalcHPRegen());
				} else if (IsClient()) {
					total_regen_field = Strings::Commify(CastToClient()->CalcHPRegen(true));
				}

				break;
			}
			case 1: {
				if (GetMaxMana() > 0) {
					regen_row_header = "Mana";
					regen_row_color = color_blue;

					if (IsBot()) {
						base_regen_field = std::to_string(0);
					} else if (IsClient()) {
						base_regen_field = Strings::Commify(CastToClient()->CalcBaseManaRegen());
					}

					item_regen_field  = Strings::Commify(itembonuses.ManaRegen + itembonuses.heroic_mana_regen);
					cap_regen_field   = Strings::Commify(CalcManaRegenCap());
					spell_regen_field = Strings::Commify(spellbonuses.ManaRegen);
					aa_regen_field    = Strings::Commify(aabonuses.ManaRegen);

					if (IsBot()) {
						total_regen_field = Strings::Commify(CastToBot()->CalcManaRegen());
					} else if (IsClient()) {
						total_regen_field = Strings::Commify(CastToClient()->CalcManaRegen(true));
					}
				} else {
					continue;
				}

				break;
			}
			case 2: {
				regen_row_header = "Endurance";
				regen_row_color  = color_green;

				base_regen_field  = Strings::Commify(((GetLevel() * 4 / 10) + 2));
				item_regen_field  = Strings::Commify(itembonuses.EnduranceRegen + itembonuses.heroic_end_regen);
				cap_regen_field   = Strings::Commify(CalcEnduranceRegenCap());
				spell_regen_field = Strings::Commify(spellbonuses.EnduranceRegen);
				aa_regen_field    = Strings::Commify(aabonuses.EnduranceRegen);

				if (IsBot()) {
					total_regen_field = Strings::Commify(CastToBot()->CalcEnduranceRegen());
				} else if (IsClient()) {
					total_regen_field = Strings::Commify(CastToClient()->CalcEnduranceRegen(true));
				}

				break;
			}
			default: {
				break;
			}
		}

		regen_string += DialogueWindow::TableRow(
			DialogueWindow::TableCell(DialogueWindow::ColorMessage(regen_row_color, regen_row_header)) +
			DialogueWindow::TableCell(DialogueWindow::ColorMessage(regen_row_color, base_regen_field)) +
			DialogueWindow::TableCell(DialogueWindow::ColorMessage(regen_row_color, fmt::format("{} ({})", item_regen_field, cap_regen_field))) +
			DialogueWindow::TableCell(DialogueWindow::ColorMessage(regen_row_color, spell_regen_field)) +
			DialogueWindow::TableCell(DialogueWindow::ColorMessage(regen_row_color, aa_regen_field)) +
			DialogueWindow::TableCell(DialogueWindow::ColorMessage(regen_row_color, total_regen_field))
		);
	}

	// Stats
	std::string stat_table;
	std::string a_stat;
	std::string a_stat_name;
	std::string h_stat;
	std::string a_resist;
	std::string a_resist_name;
	std::string h_resist_field;

	auto stat_rows = 7;

	for (auto stat_row_counter = 0; stat_row_counter < stat_rows; stat_row_counter++) {
		switch (stat_row_counter) {
			case 0: {
				a_stat_name   = "Agility";
				a_resist_name = "Cold";

				a_stat         = Strings::Commify(GetAGI());
				h_stat         = Strings::Commify(GetHeroicAGI());
				a_resist       = Strings::Commify(GetCR());
				h_resist_field = Strings::Commify(GetHeroicCR());
				break;
			}
			case 1: {
				a_stat_name   = "Charisma";
				a_resist_name = "Corruption";

				a_stat   = Strings::Commify(GetCHA());
				h_stat   = Strings::Commify(GetHeroicCHA());
				a_resist = Strings::Commify(GetCorrup());

				if (IsBot()) {
					h_resist_field = Strings::Commify(CastToBot()->GetHeroicCorrup());
				} else if (IsClient()) {
					h_resist_field = Strings::Commify(CastToClient()->GetHeroicCorrup());
				}

				break;
			}
			case 2: {
				a_stat_name   = "Dexterity";
				a_resist_name = "Disease";

				a_stat         = Strings::Commify(GetDEX());
				h_stat         = Strings::Commify(GetHeroicDEX());
				a_resist       = Strings::Commify(GetDR());
				h_resist_field = Strings::Commify(GetHeroicDR());
				break;
			}
			case 3: {
				a_stat_name   = "Intelligence";
				a_resist_name = "Fire";

				a_stat         = Strings::Commify(GetINT());
				h_stat         = Strings::Commify(GetHeroicINT());
				a_resist       = Strings::Commify(GetFR());
				h_resist_field = Strings::Commify(GetHeroicFR());
				break;
			}
			case 4: {
				a_stat_name   = "Stamina";
				a_resist_name = "Magic";

				a_stat         = Strings::Commify(GetSTA());
				h_stat         = Strings::Commify(GetHeroicSTA());
				a_resist       = Strings::Commify(GetMR());
				h_resist_field = Strings::Commify(GetHeroicMR());
				break;
			}
			case 5: {
				a_stat_name   = "Strength";
				a_resist_name = "Physical";

				a_stat   = Strings::Commify(GetSTR());
				h_stat   = Strings::Commify(GetHeroicSTR());
				a_resist = Strings::Commify(GetPhR());

				if (IsBot()) {
					h_resist_field = std::to_string(0);
				} else if (IsClient()) {
					h_resist_field = Strings::Commify(CastToClient()->GetHeroicPhR());
				}

				break;
			}
			case 6: {
				a_stat_name   = "Wisdom";
				a_resist_name = "Poison";

				a_stat         = Strings::Commify(GetWIS());
				h_stat         = Strings::Commify(GetHeroicWIS());
				a_resist       = Strings::Commify(GetPR());
				h_resist_field = Strings::Commify(GetHeroicPR());

				break;
			}
			default: {
				break;
			}
		}

		stat_table += DialogueWindow::TableRow(
			DialogueWindow::TableCell(a_stat_name) +
			DialogueWindow::TableCell(
				fmt::format(
					"{} {}",
					a_stat,
					DialogueWindow::ColorMessage(heroic_color, fmt::format("+{}", h_stat))
				)
			) +
			DialogueWindow::TableCell(a_resist_name) +
			DialogueWindow::TableCell(
				fmt::format(
					"{} {}",
					a_resist,
					DialogueWindow::ColorMessage(heroic_color, fmt::format("+{}", h_resist_field))
				)
			)
		);
	}

	// Mod2
	std::string mod2_table;
	std::string mod2a;
	std::string mod2a_name;
	std::string mod2a_cap;
	std::string mod2b;
	std::string mod2b_name;
	std::string mod2b_cap;

	auto mod2_rows = 4;

	for (auto mod2_row_counter = 0; mod2_row_counter < mod2_rows; mod2_row_counter++) {
		switch (mod2_row_counter) {
			case 0: {
				mod2a_name = "Avoidance";
				mod2b_name = "Combat Effects";
				mod2a_cap  = Strings::Commify(RuleI(Character, ItemAvoidanceCap));
				mod2b_cap  = Strings::Commify(RuleI(Character, ItemCombatEffectsCap));

				if (IsBot()) {
					mod2a = Strings::Commify(CastToBot()->GetAvoidance());
				} else if (IsClient()) {
					mod2a = Strings::Commify(CastToClient()->GetAvoidance());
				}

				if (IsBot()) {
					mod2b = Strings::Commify(CastToBot()->GetCombatEffects());
				} else if (IsClient()) {
					mod2b = Strings::Commify(CastToClient()->GetCombatEffects());
				}

				break;
			}
			case 1: {
				mod2a_name = "Accuracy";
				mod2b_name = "Strikethrough";
				mod2a_cap  = Strings::Commify(RuleI(Character, ItemAccuracyCap));
				mod2b_cap  = Strings::Commify(RuleI(Character, ItemStrikethroughCap));

				if (IsBot()) {
					mod2a = Strings::Commify(CastToBot()->GetAccuracy());
				} else if (IsClient()) {
					mod2a = Strings::Commify(CastToClient()->GetAccuracy());
				}

				if (IsBot()) {
					mod2b = Strings::Commify(CastToBot()->GetStrikeThrough());
				} else if (IsClient()) {
					mod2b = Strings::Commify(CastToClient()->GetStrikeThrough());
				}

				break;
			}
			case 2: {
				mod2a_name = "Shielding";
				mod2b_name = "Spell Shielding";
				mod2a_cap  = Strings::Commify(RuleI(Character, ItemShieldingCap));
				mod2b_cap  = Strings::Commify(RuleI(Character, ItemSpellShieldingCap));

				if (IsBot()) {
					mod2a = Strings::Commify(CastToBot()->GetShielding());
				} else if (IsClient()) {
					mod2a = Strings::Commify(CastToClient()->GetShielding());
				}


				if (IsBot()) {
					mod2b = Strings::Commify(CastToBot()->GetSpellShield());
				} else if (IsClient()) {
					mod2b = Strings::Commify(CastToClient()->GetSpellShield());
				}

				break;
			}
			case 3: {
				mod2a_name = "Stun Resist";
				mod2b_name = "DOT Shielding";
				mod2a_cap  = Strings::Commify(RuleI(Character, ItemStunResistCap));
				mod2b_cap  = Strings::Commify(RuleI(Character, ItemDoTShieldingCap));

				if (IsBot()) {
					mod2a = Strings::Commify(CastToBot()->GetStunResist());
				} else if (IsClient()) {
					mod2a = Strings::Commify(CastToClient()->GetStunResist());
				}

				if (IsBot()) {
					mod2b = Strings::Commify(CastToBot()->GetDoTShield());
				} else if (IsClient()) {
					mod2b = Strings::Commify(CastToClient()->GetDoTShield());
				}

				break;
			}
		}

		mod2_table += DialogueWindow::TableRow(
			DialogueWindow::TableCell(
				fmt::format(
					"{}: {} / {}",
					mod2a_name,
					mod2a,
					mod2a_cap
				)
			) +
			DialogueWindow::TableCell(
				fmt::format(
					"{}: {} / {}",
					mod2b_name,
					mod2b,
					mod2b_cap
				)
			)
		);
	}

	uint32 rune_number       = 0;
	uint32 magic_rune_number = 0;

	for (auto i = 0; i < GetMaxTotalSlots(); i++) {
		if (IsValidSpell(buffs[i].spellid)) {
			if (buffs[i].melee_rune > 0) {
				rune_number += buffs[i].melee_rune;
			}

			if (buffs[i].magic_rune > 0) {
				magic_rune_number += buffs[i].magic_rune;
			}
		}
	}

	auto shield_ac = 0;

	if (IsBot()) {
		CastToBot()->GetRawACNoShield(shield_ac);
	} else if (IsClient()) {
		CastToClient()->GetRawACNoShield(shield_ac);
	}

	std::string skill_mods;

	for (auto j = 0; j <= EQ::skills::HIGHEST_SKILL; j++) {
		if (itembonuses.skillmod[j] != 0) {
			const std::string& sign = itembonuses.skillmod[j] >= 0 ? "+" : "-";

			skill_mods += fmt::format(
				"{}: {}{}%%{}",
				EQ::skills::GetSkillName(static_cast<EQ::skills::SkillType>(j)),
				sign,
				Strings::Commify(itembonuses.skillmod[j]),
				DialogueWindow::Break(1)
			);
		}
	}

	std::string skill_dmgs;

	for (auto j = 0; j <= EQ::skills::HIGHEST_SKILL; j++) {
		if ((itembonuses.SkillDamageAmount[j] + spellbonuses.SkillDamageAmount[j]) != 0) {
			const std::string& sign = (itembonuses.SkillDamageAmount[j] + spellbonuses.SkillDamageAmount[j]) >= 0 ? "+" : "-";

			skill_dmgs += fmt::format(
				"{}: {}{}{}",
				EQ::skills::GetSkillName(static_cast<EQ::skills::SkillType>(j)),
				sign,
				Strings::Commify(itembonuses.SkillDamageAmount[j] + spellbonuses.SkillDamageAmount[j]),
				DialogueWindow::Break(1)
			);
		}
	}

	std::string faction_item_string;

	for (const auto& f : item_faction_bonuses) {
		if (f.second != 0) {
			const auto &faction_name = content_db.GetFactionName(f.first);
			const std::string& sign  = f.second >= 0 ? "+" : "-";

			faction_item_string += fmt::format(
				"{}: {}{}{}",
				!faction_name.empty() ? faction_name : "Unknown Faction",
				sign,
				f.second,
				DialogueWindow::Break(1)
			);
		}
	}

	std::string bard_info;

	if (GetClass() == Class::Bard) {
		const auto brass_mod  = IsBot() ? CastToBot()->GetBrassMod() : CastToClient()->GetBrassMod();
		const auto perc_mod   = IsBot() ? CastToBot()->GetPercMod() : CastToClient()->GetPercMod();
		const auto sing_mod   = IsBot() ? CastToBot()->GetSingMod() : CastToClient()->GetSingMod();
		const auto string_mod = IsBot() ? CastToBot()->GetStringMod() : CastToClient()->GetStringMod();
		const auto wind_mod   = IsBot() ? CastToBot()->GetWindMod() : CastToClient()->GetWindMod();

		if (brass_mod) {
			bard_info += fmt::format(
				"Brass: {}{}",
				Strings::Commify(brass_mod),
				DialogueWindow::Break(1)
			);
		}

		if (perc_mod) {
			bard_info += fmt::format(
				"Percussion: {}{}",
				Strings::Commify(perc_mod),
				DialogueWindow::Break(1)
			);
		}

		if (sing_mod) {
			bard_info += fmt::format(
				"Singing: {}{}",
				Strings::Commify(sing_mod),
				DialogueWindow::Break(1)
			);
		}

		if (string_mod) {
			bard_info += fmt::format(
				"String: {}{}",
				Strings::Commify(string_mod),
				DialogueWindow::Break(1)
			);
		}

		if (wind_mod) {
			bard_info += fmt::format(
				"Wind: {}{}",
				Strings::Commify(wind_mod),
				DialogueWindow::Break(1)
			);
		}
	}

	auto skill = EQ::skills::SkillHandtoHand;
	auto *inst = GetInv().GetItem(EQ::invslot::slotPrimary);

	if (inst && inst->IsClassCommon()) {
		switch (inst->GetItem()->ItemType) {
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
				if (IsClient() && CastToClient()->ClientVersion() < EQ::versions::ClientVersion::RoF2) {
					skill = EQ::skills::Skill1HPiercing;
				} else {
					skill = EQ::skills::Skill2HPiercing;
				}

				break;
			default:
				break;
		}
	}

	std::string final_string;

	// Class, Level, and Race
	final_string += DialogueWindow::Table(
		DialogueWindow::TableRow(
			DialogueWindow::TableCell(fmt::format("Race: {}", GetPlayerRaceAbbreviation(GetBaseRace()))) +
			DialogueWindow::TableCell(fmt::format("Class: {}", GetPlayerClassAbbreviation(GetClass()))) +
			DialogueWindow::TableCell(fmt::format("Level: {}", std::to_string(GetLevel())))
		)
	);

	// Runes
	if (rune_number || magic_rune_number) {
		final_string += DialogueWindow::Table(
			DialogueWindow::TableRow(
				DialogueWindow::TableCell(
					fmt::format("Rune: {}", rune_number)
				) +
				DialogueWindow::TableCell("") +
				DialogueWindow::TableCell(
					fmt::format("Spell Rune: {}", magic_rune_number)
				)
			)
		);

		final_string += DialogueWindow::Break(1);
	}

	// Health, Mana, and Endurance
	final_string += DialogueWindow::Table(HME_row);

	// Attack
	final_string += DialogueWindow::ColorMessage(
		"green_yellow",
		fmt::format(
			"To Hit: {} / {}",
			Strings::Commify(compute_tohit(skill)),
			Strings::Commify(GetTotalToHit(skill, 0))
		)
	);

	final_string += DialogueWindow::Break(1);

	// Attack 2
	final_string += fmt::format(
		"Offense: {}{}{}{}",
		Strings::Commify(offense(skill)),
		(
			itembonuses.ATK ?
			fmt::format(
				" | Item: {} / {} | Used: {}",
				Strings::Commify(itembonuses.ATK),
				Strings::Commify(RuleI(Character, ItemATKCap)),
				Strings::Commify(static_cast<int>(itembonuses.ATK * 1.342))
			) :
			""
		),
		spellbonuses.ATK ? fmt::format(" | Spell: {}", Strings::Commify(spellbonuses.ATK)) : "",
		DialogueWindow::Break(1)
	);

	// Armor Class
	final_string += fmt::format(
		"{}{}",
		DialogueWindow::ColorMessage(
			"green_yellow",
			fmt::format(
				"Mitigation AC: {}",
				Strings::Commify(GetMitigationAC())
			)
		),
		DialogueWindow::Break(1)
	);

	// Armor Class 2
	final_string += fmt::format(
		"Defense: {} / {}{}{}{}",
		Strings::Commify(compute_defense()),
		Strings::Commify(GetTotalDefense()),
		spellbonuses.AC ? fmt::format(" | Spell: {}", Strings::Commify(spellbonuses.AC)) : "",
		shield_ac ? fmt::format(" | Shield: {}", Strings::Commify(shield_ac)) : "",
		DialogueWindow::Break(1)
	);

	// Run Speed
	final_string += fmt::format(
		"{}{}",
		DialogueWindow::ColorMessage(
			"green_yellow",
			fmt::format(
				"Runspeed: {}",
				IsBot() ? CastToBot()->GetRunspeed() : CastToClient()->GetRunspeed()
			)
		),
		DialogueWindow::Break(1)
	);

	final_string += DialogueWindow::CenterMessage("Haste");

	// Haste Table
	const auto& haste_table = DialogueWindow::Table(
		fmt::format(
			"{}{}",
			DialogueWindow::TableRow(
				DialogueWindow::TableCell("Item") +
				DialogueWindow::TableCell("Spell") +
				DialogueWindow::TableCell("Over") +
				DialogueWindow::TableCell("Total (Cap)")
			),
			DialogueWindow::TableRow(
				DialogueWindow::TableCell(Strings::Commify(itembonuses.haste)) +
				DialogueWindow::TableCell(Strings::Commify(spellbonuses.haste + spellbonuses.hastetype2)) +
				DialogueWindow::TableCell(Strings::Commify(spellbonuses.hastetype3 + extra_haste)) +
				DialogueWindow::TableCell(
					fmt::format(
						"{} ({})",
						IsClient() ? Strings::Commify(CastToClient()->GetHaste()) : Strings::Commify(GetHaste()),
						Strings::Commify(RuleI(Character, HasteCap))
					)
				)
			)
		)
	);

	final_string += haste_table;

	// Regen Table
	final_string += DialogueWindow::CenterMessage("Regen");

	const auto& regen_table = DialogueWindow::Table(
		DialogueWindow::TableRow(
			DialogueWindow::TableCell("Type") +
			DialogueWindow::TableCell("Base") +
			DialogueWindow::TableCell("Items (Cap)") +
			DialogueWindow::TableCell("Spell") +
			DialogueWindow::TableCell("AAs") +
			DialogueWindow::TableCell("Total")
		) +
		regen_string
	);

	// Regen
	final_string += regen_table + DialogueWindow::Break(1);

	// Stats
	final_string += DialogueWindow::Table(stat_table) + DialogueWindow::Break(1);

	// Mod 2
	final_string += DialogueWindow::Table(mod2_table) + DialogueWindow::Break(1);

	// Heal Amount
	if (GetHealAmt()) {
		final_string += fmt::format(
			"Heal Amount: {} / {}{}",
			Strings::Commify(GetHealAmt()),
			Strings::Commify(RuleI(Character, ItemHealAmtCap)),
			DialogueWindow::Break(1)
		);
	}

	// Heal Amount
	if (GetSpellDmg()) {
		final_string += fmt::format(
			"Spell Damage: {} / {}{}",
			Strings::Commify(GetSpellDmg()),
			Strings::Commify(RuleI(Character, ItemSpellDmgCap)),
			DialogueWindow::Break(1)
		);
	}

	// Damage Shield
	if (itembonuses.DamageShield || spellbonuses.DamageShield) {
		final_string += fmt::format(
			"Damage Shield: {}{}{}{}",
			Strings::Commify(itembonuses.DamageShield + spellbonuses.DamageShield),
			(
				spellbonuses.DamageShield ?
				fmt::format(" | Spell: {}", Strings::Commify(spellbonuses.DamageShield)) :
				""
			),
			(
				itembonuses.DamageShield ?
				fmt::format(
					" | Item: {} / {}",
					Strings::Commify(itembonuses.DamageShield),
					Strings::Commify(RuleI(Character, ItemDamageShieldCap))
				) :
				""
			),
			DialogueWindow::Break(2)
		);
	}

	// Clairvoyance
	const auto clairvoyance  = IsBot() ? CastToBot()->GetClair() : CastToClient()->GetClair();
	if (clairvoyance) {
		final_string += fmt::format(
			"Clairvoyance: {} / {}{}",
			Strings::Commify(clairvoyance),
			Strings::Commify(RuleI(Character, ItemClairvoyanceCap)),
			DialogueWindow::Break(1)
		);
	}

	// Damage Shield Mitigation
	const auto ds_mitigation = IsBot() ? CastToBot()->GetDSMit() : CastToClient()->GetDSMit();
	if (ds_mitigation) {
		final_string += fmt::format(
			"DS Mitigation: {} / {}{}",
			Strings::Commify(ds_mitigation),
			Strings::Commify(RuleI(Character, ItemDSMitigationCap)),
			DialogueWindow::Break(1)
		);
	}

	if (clairvoyance || ds_mitigation) {
		final_string += DialogueWindow::Break(1);
	}

	// Bard Modifiers
	if (GetClass() == Class::Bard) {
		final_string += bard_info + DialogueWindow::Break(1);
	}

	// Skill Modifiers
	if (!skill_mods.empty()) {
		final_string += skill_mods + DialogueWindow::Break(1);
	}

	// Skill Damage Modifiers
	if (!skill_dmgs.empty()) {
		final_string += skill_dmgs + DialogueWindow::Break(1);
	}

	// Faction Modifiers
	if (!faction_item_string.empty()) {
		final_string += faction_item_string;
	}

	if (use_window) {
		if (final_string.size() < 4096) {
			const uint32 popup_buttons = (c->ClientVersion() < EQ::versions::ClientVersion::SoD) ? 0 : 1;
			c->SendWindow(
				0,
				POPUPID_UPDATE_SHOWSTATSWINDOW,
				popup_buttons,
				"Close",
				"Update",
				0,
				1,
				this,
				"",
				final_string.c_str()
			);

			goto extra_info;
		} else {
			c->Message(Chat::Yellow, "The window has exceeded its character limit, displaying stats to chat window:");
		}
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Statistics Information for {} {}",
			GetCleanName(),
			GetLastName()
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Level: {} Class: {} ({}) Race: {} ({}) Damage Shield: {}/{} Size: {:.1f} Run Speed: {} Weight: {:.1f}/{}",
			GetLevel(),
			GetClassIDName(GetClass()),
			GetClass(),
			GetRaceIDName(GetRace()),
			GetRace(),
			IsBot() ? Strings::Commify(CastToBot()->GetDS()) : Strings::Commify(CastToClient()->GetDS()),
			Strings::Commify(RuleI(Character, ItemDamageShieldCap)),
			GetSize(),
			GetRunspeed(),
			IsBot() ? static_cast<float>(CastToBot()->CalcCurrentWeight()) / 10.0f : static_cast<float>(CastToClient()->CalcCurrentWeight()) / 10.0f,
			Strings::Commify(GetSTR())
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Health: {}/{} Regen: {}/{}",
			GetHP(),
			GetMaxHP(),
			IsBot() ? Strings::Commify(CastToBot()->CalcHPRegen()) : Strings::Commify(CastToClient()->CalcHPRegen()),
			CalcHPRegenCap()
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"To Hit: {} Total: {}",
			Strings::Commify(compute_tohit(skill)),
			Strings::Commify(GetTotalToHit(skill, 0))
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Defense: {} Total: {}",
			Strings::Commify(compute_defense()),
			Strings::Commify(GetTotalDefense())
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Offense: {} Mitigation AC: {}",
			Strings::Commify(offense(skill)),
			Strings::Commify(GetMitigationAC())
		).c_str()
	);

	if (IsClient()) {
		c->Message(
			Chat::White,
			fmt::format(
				" AFK: {} LFG: {} Anon: {} PVP: {} GM: {} Fly Mode: {} ({}) GM Speed: {} Hide Me: {} Invulnerability: {} LD: {} Client Version: {} Tells Off: {}",
				CastToClient()->m_is_afk ? "Yes" : "No",
				CastToClient()->LFG ? "Yes" : "No",
				CastToClient()->GetAnon() ? "Yes" : "No",
				CastToClient()->GetPVP() ? "Yes" : "No",
				CastToClient()->GetGM() ? "On" : "Off",
				EQ::constants::GetFlyModeName(static_cast<uint8>(flymode)),
				static_cast<int>(flymode),
				CastToClient()->GetGMSpeed() ? "On" : "Off",
				CastToClient()->GetHideMe() ? "On" : "Off",
				CastToClient()->GetGMInvul() ? "On" : "Off",
				CastToClient()->IsLD() ? "Yes" : "No",
				CastToClient()->ClientVersionBit(),
				CastToClient()->tellsoff ? "Yes" : "No"
			).c_str()
		);
	}

	if (CalcMaxMana()) {
		c->Message(
			Chat::White,
			fmt::format(
				"Mana: {}/{} Regen: {}/{}",
				Strings::Commify(GetMana()),
				Strings::Commify(GetMaxMana()),
				IsBot() ? Strings::Commify(CastToBot()->CalcManaRegen()) : Strings::Commify(CastToClient()->CalcManaRegen()),
				Strings::Commify(CalcManaRegenCap())
			).c_str()
		);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Endurance: {}/{} Regen: {}/{}",
			Strings::Commify(GetEndurance()),
			Strings::Commify(GetMaxEndurance()),
			IsBot() ? Strings::Commify(CastToBot()->CalcEnduranceRegen()) : Strings::Commify(CastToClient()->CalcEnduranceRegen(true)),
			Strings::Commify(CalcEnduranceRegenCap())
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Attack: {} Item and Spell Attack: {}/{} Server Side Attack: {}",
			IsBot() ? Strings::Commify(CastToBot()->GetTotalATK()) : Strings::Commify(CastToClient()->GetTotalATK()),
			Strings::Commify(GetATKBonus()),
			Strings::Commify(RuleI(Character, ItemATKCap)),
			Strings::Commify(GetATK())
		).c_str()
	);

	if ((IsClient() && CastToClient()->GetHaste()) || (!IsClient() && GetHaste())) {
		c->Message(
			Chat::White,
			fmt::format(
				"Haste: {}/{} (Item: {} + Spell: {} + Over: {})",
				IsClient() ? Strings::Commify(CastToClient()->GetHaste()) : Strings::Commify(GetHaste()),
				Strings::Commify(RuleI(Character, HasteCap)),
				Strings::Commify(itembonuses.haste),
				Strings::Commify(spellbonuses.haste + spellbonuses.hastetype2),
				Strings::Commify(spellbonuses.hastetype3 + extra_haste)
			).c_str()
		);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Statistics | Agility: {} Charisma: {} Dexterity: {} Intelligence: {} Stamina: {} Strength: {} Wisdom: {}",
			Strings::Commify(GetAGI()),
			Strings::Commify(GetCHA()),
			Strings::Commify(GetDEX()),
			Strings::Commify(GetINT()),
			Strings::Commify(GetSTA()),
			Strings::Commify(GetSTR()),
			Strings::Commify(GetWIS())
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Heroic Statistics | Agility: {} Charisma: {} Dexterity: {} Intelligence: {} Stamina: {} Strength: {} Wisdom: {}",
			Strings::Commify(GetHeroicAGI()),
			Strings::Commify(GetHeroicCHA()),
			Strings::Commify(GetHeroicDEX()),
			Strings::Commify(GetHeroicINT()),
			Strings::Commify(GetHeroicSTA()),
			Strings::Commify(GetHeroicSTR()),
			Strings::Commify(GetHeroicWIS())
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Resistances | Cold: {} Corruption: {} Disease: {} Fire: {} Magic: {} Poison: {} Physical: {}",
			Strings::Commify(GetCR()),
			Strings::Commify(GetCorrup()),
			Strings::Commify(GetDR()),
			Strings::Commify(GetFR()),
			Strings::Commify(GetMR()),
			Strings::Commify(GetPR()),
			Strings::Commify(GetPhR())
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Heroic Resistances | Cold: {} Corruption: {} Disease: {} Fire: {} Magic: {} Poison: {}",
			GetHeroicCR(),
			IsBot() ? Strings::Commify(CastToBot()->GetHeroicCorrup()) : Strings::Commify(CastToClient()->GetHeroicCorrup()),
			GetHeroicDR(),
			GetHeroicFR(),
			GetHeroicMR(),
			GetHeroicPR()
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Accuracy: {} Avoidance: {} Combat Effects: {} DOT Shielding: {} Shielding: {} Spell Shield: {} Strikethrough: {} Stun Resist: {}",
			IsBot() ? Strings::Commify(CastToBot()->GetAccuracy()) : Strings::Commify(CastToClient()->GetAccuracy()),
			IsBot() ? Strings::Commify(CastToBot()->GetAvoidance()) : Strings::Commify(CastToClient()->GetAvoidance()),
			IsBot() ? Strings::Commify(CastToBot()->GetCombatEffects()) : Strings::Commify(CastToClient()->GetCombatEffects()),
			IsBot() ? Strings::Commify(CastToBot()->GetDoTShield()) : Strings::Commify(CastToClient()->GetDoTShield()),
			IsBot() ? Strings::Commify(CastToBot()->GetShielding()) : Strings::Commify(CastToClient()->GetShielding()),
			IsBot() ? Strings::Commify(CastToBot()->GetSpellShield()) : Strings::Commify(CastToClient()->GetSpellShield()),
			IsBot() ? Strings::Commify(CastToBot()->GetStrikeThrough()) : Strings::Commify(CastToClient()->GetStrikeThrough()),
			IsBot() ? Strings::Commify(CastToBot()->GetStunResist()) : Strings::Commify(CastToClient()->GetStunResist())
		).c_str()
	);

	if (GetHealAmt() || GetSpellDmg()) {
		c->Message(
			Chat::White,
			fmt::format(
				"Heal Amount: {} Spell Damage: {}",
				Strings::Commify(GetHealAmt()),
				Strings::Commify(GetSpellDmg())
			).c_str()
		);
	}

	if (clairvoyance || ds_mitigation) {
		c->Message(
			Chat::White,
			fmt::format(
				"Clairvoyance: {} Damage Shield Mitigation: {}",
				Strings::Commify(clairvoyance),
				Strings::Commify(ds_mitigation)
			).c_str()
		);
	}

	if (GetClass() == Class::Bard) {
		const auto brass_mod  = IsBot() ? CastToBot()->GetBrassMod() : CastToClient()->GetBrassMod();
		const auto perc_mod   = IsBot() ? CastToBot()->GetPercMod() : CastToClient()->GetPercMod();
		const auto sing_mod   = IsBot() ? CastToBot()->GetSingMod() : CastToClient()->GetSingMod();
		const auto string_mod = IsBot() ? CastToBot()->GetStringMod() : CastToClient()->GetStringMod();
		const auto wind_mod   = IsBot() ? CastToBot()->GetWindMod() : CastToClient()->GetWindMod();

		if (
			brass_mod ||
			perc_mod ||
			sing_mod ||
			string_mod ||
			wind_mod
		) {
			c->Message(
				Chat::White,
				fmt::format(
					"Brass: {} Percussion: {} Singing: {} String: {} Wind: {}",
					Strings::Commify(brass_mod),
					Strings::Commify(perc_mod),
					Strings::Commify(sing_mod),
					Strings::Commify(string_mod),
					Strings::Commify(wind_mod)
				).c_str()
			);
		}
	}

	extra_info:

	c->Message(
		Chat::White,
		fmt::format(
			"Base Race: {} ({}) Gender: {} ({}) Base Gender: {} ({}) Texture: {} Helmet Texture: {}",
			GetRaceIDName(GetBaseRace()),
			GetBaseRace(),
			GetGenderName(GetGender()),
			GetGender(),
			GetGenderName(GetBaseGender()),
			GetBaseGender(),
			GetTexture(),
			GetHelmTexture()
		).c_str()
	);

	if (c->Admin() >= AccountStatus::GMAdmin) {
		c->Message(
			Chat::White,
			fmt::format(
				"ID: {} Entity ID: {} Pet ID: {} Owner ID: {} AI Controlled: {} Targeted: {}",
				IsBot() ? CastToBot()->GetBotID() : CastToClient()->CharacterID(),
				GetID(),
				GetPetID(),
				GetOwnerID(),
				IsAIControlled() ? "Yes" : "No",
				targeted
			).c_str()
		);
	}
}

void Mob::ShowStats(Client* c)
{
	if (IsOfClientBot()) {
		SendStatsWindow(c, RuleB(Character, UseNewStatsWindow));
	} else if (IsCorpse()) {
		if (IsPlayerCorpse()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Player Corpse | Character ID: {}  ID: {}",
					CastToCorpse()->GetCharID(),
					CastToCorpse()->GetCorpseDBID()
				).c_str()
			);
		} else {
			c->Message(
				Chat::White,
				fmt::format(
					"NPC Corpse | ID: {}",
					GetID()
				).c_str()
			);
		}
	} else {
		const auto& t = CastToNPC();

		const std::string target_name      = t->GetCleanName();
		const std::string target_last_name = t->GetLastName();

		const auto has_charmed_stats = (
			t->GetCharmedAccuracy() != 0 ||
			t->GetCharmedArmorClass() != 0 ||
			t->GetCharmedAttack() != 0 ||
			t->GetCharmedAttackDelay() != 0 ||
			t->GetCharmedAvoidance() != 0 ||
			t->GetCharmedMaxDamage() != 0 ||
			t->GetCharmedMinDamage() != 0
		);

		// Faction
		if (t->GetNPCFactionID()) {
			const std::string& faction_name = content_db.GetFactionName(t->GetPrimaryFaction());
			c->Message(
				Chat::White,
				fmt::format(
					"Faction: {} ({})",
					faction_name,
					t->GetPrimaryFaction()
				).c_str()
			);
		}

		// Adventure Template
		if (t->GetAdventureTemplate()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Adventure Template: {}",
					t->GetAdventureTemplate()
				).c_str()
			);
		}

		// Body
		auto bodytype_name = BodyType::GetName(t->GetBodyType());
		c->Message(
			Chat::White,
			fmt::format(
				"Body | Size: {:.2f} Type: {}",
				t->GetSize(),
				(
					bodytype_name.empty() ?
					fmt::format(
						"{}",
						t->GetBodyType()
					) :
					fmt::format(
						"{} ({})",
						bodytype_name,
						t->GetBodyType()
					)
				)
			).c_str()
		);

		// Face
		c->Message(
			Chat::White,
			fmt::format(
				"Features | Face: {} Eye One: {} Eye Two: {}",
				t->GetLuclinFace(),
				t->GetEyeColor1(),
				t->GetEyeColor2()
			).c_str()
		);

		// Hair
		c->Message(
			Chat::White,
			fmt::format(
				"Features | Hair: {} Hair Color: {}",
				t->GetHairStyle(),
				t->GetHairColor()
			).c_str()
		);

		// Beard
		c->Message(
			Chat::White,
			fmt::format(
				"Features | Beard: {} Beard Color: {}",
				t->GetBeard(),
				t->GetBeardColor()
			).c_str()
		);

		// Drakkin Features
		if (t->GetRace() == Race::Drakkin) {
			c->Message(
				Chat::White,
				fmt::format(
					"Drakkin Features | Heritage: {} Tattoo: {} Details: {}",
					t->GetDrakkinHeritage(),
					t->GetDrakkinTattoo(),
					t->GetDrakkinDetails()
				).c_str()
			);
		}

		// Textures
		c->Message(
			Chat::White,
			fmt::format(
				"Textures | Armor: {} Helmet: {}",
				t->GetTexture(),
				t->GetHelmTexture()
			).c_str()
		);

		if (
			t->GetArmTexture() ||
			t->GetBracerTexture() ||
			t->GetHandTexture()
		) {
			c->Message(
				Chat::White,
				fmt::format(
					"Textures | Arms: {} Bracers: {} Hands: {}",
					t->GetArmTexture(),
					t->GetBracerTexture(),
					t->GetHandTexture()
				).c_str()
			);
		}

		if (
			t->GetFeetTexture() ||
			t->GetLegTexture()
		) {
			c->Message(
				Chat::White,
				fmt::format(
					"Textures | Legs: {} Feet: {}",
					t->GetLegTexture(),
					t->GetFeetTexture()
				).c_str()
			);
		}

		// Hero's Forge
		if (t->GetHeroForgeModel()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Hero's Forge: {}",
					t->GetHeroForgeModel()
				).c_str()
			);
		}

		// Owner Data
		if (t->GetOwner()) {
			const auto& o = t->GetOwner();
			auto owner_name = o->GetCleanName();
			auto owner_type = (
				o->IsNPC() ?
				"NPC" :
				(
					o->IsClient() ?
					"Client" :
					"Other"
				)
			);
			auto owner_id = t->GetOwnerID();
			c->Message(
				Chat::White,
				fmt::format(
					"Owner | Name: {} ({}) Type: {}",
					owner_name,
					owner_id,
					owner_type
				).c_str()
			);
		}

		// Pet Data
		if (t->GetPet()) {
			auto pet_name = t->GetPet()->GetCleanName();
			auto pet_id = t->GetPetID();
			c->Message(
				Chat::White,
				fmt::format(
					"Pet | Name: {} ({})",
					pet_name,
					pet_id
				).c_str()
			);
		}

		// Merchant Data
		if (t->MerchantType) {
			c->Message(
				Chat::White,
				fmt::format(
					"Merchant | ID: {} Currency Type: {}",
					t->MerchantType,
					t->GetAltCurrencyType()
				).c_str()
			);
		}

		// Spell Data
		if (t->AI_HasSpells() || t->AI_HasSpellsEffects()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Spells | ID: {} Effects ID: {}",
					t->GetNPCSpellsID(),
					t->GetNPCSpellsEffectsID()
				).c_str()
			);
		}

		// Health
		c->Message(
			Chat::White,
			fmt::format(
				"Health: {}/{} ({:.2f}%) Regen: {}",
				t->GetHP(),
				t->GetMaxHP(),
				t->GetHPRatio(),
				t->GetHPRegen()
			).c_str()
		);

		// Mana
		if (t->GetMaxMana() > 0) {
			c->Message(
				Chat::White,
				fmt::format(
					"Mana: {}/{} ({:.2f}%) Regen: {}",
					t->GetMana(),
					t->GetMaxMana(),
					t->GetManaRatio(),
					t->GetManaRegen()
				).c_str()
			);
		}

		// Damage
		c->Message(
			Chat::White,
			fmt::format(
				"Damage | Min: {} Max: {}",
				t->GetMinDMG(),
				t->GetMaxDMG()
			).c_str()
		);

		// Attack Count / Delay
		c->Message(
			Chat::White,
			fmt::format(
				"Attack | Count: {} Delay: {}",
				t->GetNumberOfAttacks(),
				t->GetAttackDelay()
			).c_str()
		);

		// Weapon Textures
		c->Message(
			Chat::White,
			fmt::format(
				"Weapon Textures | Primary: {} Secondary: {} Ammo: {}",
				t->GetEquipmentMaterial(EQ::textures::weaponPrimary),
				t->GetEquipmentMaterial(EQ::textures::weaponSecondary),
				t->GetAmmoIDfile()
			).c_str()
		);

		// Weapon Types
		c->Message(
			Chat::White,
			fmt::format(
				"Weapon Types | Primary: {} ({}) Secondary: {} ({})",
				EQ::skills::GetSkillName(static_cast<EQ::skills::SkillType>(t->GetPrimSkill())),
				t->GetPrimSkill(),
				EQ::skills::GetSkillName(static_cast<EQ::skills::SkillType>(t->GetSecSkill())),
				t->GetSecSkill()
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Weapon Types | Ranged: {} ({})",
				EQ::skills::GetSkillName(static_cast<EQ::skills::SkillType>(t->GetRangedSkill())),
				t->GetRangedSkill()
			).c_str()
		);

		// Combat Stats
		c->Message(
			Chat::White,
			fmt::format(
				"Combat Stats | Accuracy: {} Armor Class: {} Attack: {}",
				t->GetAccuracyRating(),
				t->GetAC(),
				t->GetATK()
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Combat Stats | Avoidance: {} Slow Mitigation: {}",
				t->GetAvoidanceRating(),
				t->GetSlowMitigation()
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Combat Stats | To Hit: {} Total To Hit: {}",
				compute_tohit(EQ::skills::SkillHandtoHand),
				GetTotalToHit(EQ::skills::SkillHandtoHand, 0)
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Combat Stats | Defense: {} Total Defense: {}",
				compute_defense(),
				GetTotalDefense()
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Combat Stats | Offense: {} Mitigation Armor Class: {}",
				offense(EQ::skills::SkillHandtoHand),
				GetMitigationAC()
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Combat Stats | Haste: {}",
				GetHaste()
			).c_str()
		);

		// Stats
		c->Message(
			Chat::White,
			fmt::format(
				"Stats | Agility: {} Charisma: {} Dexterity: {} Intelligence: {}",
				t->GetAGI(),
				t->GetCHA(),
				t->GetDEX(),
				t->GetINT()
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Stats | Stamina: {} Strength: {} Wisdom: {}",
				t->GetSTA(),
				t->GetSTR(),
				t->GetWIS()
			).c_str()
		);

		// Charmed Stats
		if (has_charmed_stats) {
			c->Message(
				Chat::White,
				fmt::format(
					"Charmed Stats | Attack: {} Attack Delay: {}",
					t->GetCharmedAttack(),
					t->GetCharmedAttackDelay()
				).c_str()
			);

			c->Message(
				Chat::White,
				fmt::format(
					"Charmed Stats | Accuracy: {} Avoidance: {}",
					t->GetCharmedAccuracy(),
					t->GetCharmedAvoidance()
				).c_str()
			);

			c->Message(
				Chat::White,
				fmt::format(
					"Charmed Stats | Min Damage: {} Max Damage: {}",
					t->GetCharmedMinDamage(),
					t->GetCharmedMaxDamage()
				).c_str()
			);
		}

		// Resists
		c->Message(
			Chat::White,
			fmt::format(
				"Resists | Cold: {} Disease: {} Fire: {} Magic: {}",
				t->GetCR(),
				t->GetDR(),
				t->GetFR(),
				t->GetMR()
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Resists | Poison: {} Corruption: {} Physical: {}",
				t->GetPR(),
				t->GetCorrup(),
				t->GetPhR()
			).c_str()
		);

		// Scaling
		c->Message(
			Chat::White,
			fmt::format(
				"Scaling | Heal: {} Spell: {}",
				t->GetHealScale(),
				t->GetSpellScale()
			).c_str()
		);

		// See Invisible / Invisible vs. Undead / Hide / Improved Hide
		c->Message(
			Chat::White,
			fmt::format(
				"Can See | Invisible: {} Invisible vs. Undead: {}",
				t->SeeInvisible() ? "Yes" : "No",
				t->SeeInvisibleUndead() ? "Yes" : "No"
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Can See | Hide: {} Improved Hide: {}",
				t->SeeHide() ? "Yes" : "No",
				t->SeeImprovedHide() ? "Yes" : "No"
			).c_str()
		);

		// Aggro / Assist Radius
		c->Message(
			Chat::White,
			fmt::format(
				"Radius | Aggro: {} Assist: {}",
				t->GetAggroRange(),
				t->GetAssistRange()
			).c_str()
		);

		// Emote
		if (t->GetEmoteID()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Emote: {}",
					t->GetEmoteID()
				).c_str()
			);
		}

		// Run/Walk Speed
		c->Message(
			Chat::White,
			fmt::format(
				"Speed | Run: {} Walk: {}",
				t->GetRunspeed(),
				t->GetWalkspeed()
			).c_str()
		);

		// Position
		c->Message(
			Chat::White,
			fmt::format(
				"Position | {}, {}, {}, {}",
				t->GetX(),
				t->GetY(),
				t->GetZ(),
				t->GetHeading()
			).c_str()
		);

		// Experience Modifier
		c->Message(
			Chat::White,
			fmt::format(
				"Experience Modifier: {}",
				t->GetKillExpMod()
			).c_str()
		);

		// Quest Globals
		c->Message(
			Chat::White,
			fmt::format(
				"Quest Globals: {}",
				t->qglobal ? "Enabled" : "Disabled"
			).c_str()
		);

		// Proximity
		if (t->IsProximitySet()) {
			c->Message(
				Chat::White,
				fmt::format(
					"Proximity | Say: {}",
					t->proximity->say ? "Enabled" : "Disabled"
				).c_str()
			);

			c->Message(
				Chat::White,
				fmt::format(
					"Proximity X | Min: {} Max: {} Range: {}",
					t->GetProximityMinX(),
					t->GetProximityMaxX(),
					(t->GetProximityMaxX() - t->GetProximityMinX())
				).c_str()
			);

			c->Message(
				Chat::White,
				fmt::format(
					"Proximity Y | Min: {} Max: {} Range: {}",
					t->GetProximityMinY(),
					t->GetProximityMaxY(),
					(t->GetProximityMaxY() - t->GetProximityMinY())
				).c_str()
			);

			c->Message(
				Chat::White,
				fmt::format(
					"Proximity Z | Min: {} Max: {} Range: {}",
					t->GetProximityMinZ(),
					t->GetProximityMaxZ(),
					(t->GetProximityMaxZ() - t->GetProximityMinZ())
				).c_str()
			);
		}

		// Spawn Data
		if (
			t->GetGrid() ||
			t->GetSpawnGroupId() ||
			t->GetSpawnPointID()
		) {
			c->Message(
				Chat::White,
				fmt::format(
					"Spawn | Group: {} Point: {} Grid: {}",
					t->GetSpawnGroupId(),
					t->GetSpawnPointID(),
					t->GetGrid()
				).c_str()
			);
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Spawn | Raid: {} Rare: {}",
				t->IsRaidTarget() ? "Yes" : "No",
				t->IsRareSpawn() ? "Yes" : "No",
				t->GetSkipGlobalLoot() ? "Yes" : "No"
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Spawn | Skip Global Loot: {} Ignore Despawn: {}",
				t->GetSkipGlobalLoot() ? "Yes" : "No",
				t->GetIgnoreDespawn() ? "Yes" : "No"
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Spawn | Findable: {} Trackable: {} Underwater: {}",
				t->IsFindable() ? "Yes" : "No",
				t->IsTrackable() ? "Yes" : "No",
				t->IsUnderwaterOnly() ? "Yes" : "No"
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Spawn | Stuck Behavior: {} Fly Mode: {}",
				t->GetStuckBehavior(),
				static_cast<int>(t->GetFlyMode())
			).c_str()
		);

		c->Message(
			Chat::White,
			fmt::format(
				"Spawn | Aggro NPCs: {} Always Aggro: {}",
				t->GetNPCAggro() ? "Yes" : "No",
				t->GetAlwaysAggro() ? "Yes" : "No"
			).c_str()
		);

		// Race / Class / Gender
		c->Message(
			Chat::White,
			fmt::format(
				"Race: {} ({}) Class: {} ({}) Gender: {} ({})",
				GetRaceIDName(t->GetRace()),
				t->GetRace(),
				GetClassIDName(t->GetClass()),
				t->GetClass(),
				GetGenderName(t->GetGender()),
				t->GetGender()
			).c_str()
		);

		// NPC
		c->Message(
			Chat::White,
			fmt::format(
				"NPC | ID: {} Entity ID: {} Name: {}{} Level: {}",
				t->GetNPCTypeID(),
				t->GetID(),
				target_name,
				(
					!target_last_name.empty() ?
					fmt::format(" ({})", target_last_name) :
					""
				),
				t->GetLevel()
			).c_str()
		);
	}
}

void Mob::DoAnim(const int animation_id, int animation_speed, bool ackreq, eqFilterType filter)
{
	if (!attack_anim_timer.Check()) {
		return;
	}

	static EQApplicationPacket p(OP_Animation, sizeof(Animation_Struct));
	auto a = (Animation_Struct*) p.pBuffer;
	a->spawnid = GetID();
	a->action  = animation_id;
	a->speed   = animation_speed ? animation_speed : 10;

	entity_list.QueueCloseClients(
		this, /* Sender */
		&p, /* Packet */
		false, /* Ignore Sender */
		RuleI(Range, Anims),
		0, /* Skip this mob */
		ackreq, /* Packet ACK */
		filter /* eqFilterType filter */
	);
}

void Mob::ShowBuffs(Client* c) {
	if (SPDAT_RECORDS <= 0) {
		return;
	}

	std::string buffs_table;

	buffs_table += DialogueWindow::TableRow(
		fmt::format(
			"{}{}{}{}{}",
			DialogueWindow::TableCell("Slot"),
			DialogueWindow::TableCell("Spell"),
			DialogueWindow::TableCell("Spell ID"),
			DialogueWindow::TableCell("Duration"),
			DialogueWindow::TableCell("Hits")
		)
	);

	for (auto i = 0; i < GetMaxTotalSlots(); i++) {
		const auto spell_id = buffs[i].spellid;
		if (IsValidSpell(spell_id)) {
			const auto buff_duration_formula = spells[spell_id].buff_duration_formula;
			const auto is_permanent          = (
				buff_duration_formula == DF_Aura ||
				buff_duration_formula == DF_Permanent
			);

			const auto time = Strings::SecondsToTime(buffs[i].ticsremaining * 6);

			buffs_table += DialogueWindow::TableRow(
				fmt::format(
					"{}{}{}{}{}",
					DialogueWindow::TableCell(std::to_string(i)),
					DialogueWindow::TableCell(GetSpellName(spell_id)),
					DialogueWindow::TableCell(std::to_string(spell_id)),
					DialogueWindow::TableCell(is_permanent ? "Permanent" : time),
					DialogueWindow::TableCell(std::to_string(buffs[i].hit_number))
				)
			);
		}
	}

	buffs_table = DialogueWindow::Table(buffs_table);

	c->SendPopupToClient(
		fmt::format(
			"Buffs on {}",
			c->GetTargetDescription(this, TargetDescriptionType::UCSelf)
		).c_str(),
		buffs_table.c_str()
	);
}

void Mob::GMMove(float x, float y, float z, float heading, bool save_guard_spot) {
	m_Position.x = x;
	m_Position.y = y;
	m_Position.z = z;
	SetHeading(heading);
	mMovementManager->SendCommandToClients(this, 0.0, 0.0, 0.0, 0.0, 0, ClientRangeAny);

	if (IsNPC() && save_guard_spot) {
		CastToNPC()->SaveGuardSpot(glm::vec4(x, y, z, heading));
	}
}

void Mob::GMMove(const glm::vec4 &position, bool save_guard_spot) {
	m_Position.x = position.x;
	m_Position.y = position.y;
	m_Position.z = position.z;
	SetHeading(position.w);
	mMovementManager->SendCommandToClients(this, 0.0, 0.0, 0.0, 0.0, 0, ClientRangeAny);

	if (IsNPC() && save_guard_spot) {
		CastToNPC()->SaveGuardSpot(position);
	}
}

void Mob::SendIllusionPacket(const AppearanceStruct& a)
{
	uint16 new_race = (
		a.race_id != Race::Doug ?
		a.race_id :
		(use_model ? use_model : GetBaseRace())
	);

	uint8 new_gender = (
		a.gender_id != UINT8_MAX ?
		a.gender_id :
		(a.race_id ? GetDefaultGender(a.race_id, a.gender_id) : GetBaseGender())
	);

	float new_size = a.size <= 0.0f ? GetRaceGenderDefaultHeight(race, gender) : a.size;

	uint8 new_texture        = a.texture == UINT8_MAX && !IsPlayerRace(a.race_id) ? GetTexture() : a.texture;
	uint8 new_helmet_texture = a.helmet_texture == UINT8_MAX && !IsPlayerRace(race) ? GetHelmTexture() : a.helmet_texture;

	uint8 new_hair       = a.hair == UINT8_MAX ? GetHairStyle() : a.hair;
	uint8 new_hair_color = a.hair_color == UINT8_MAX ? GetHairColor() : a.hair_color;

	uint8 new_beard       = a.beard == UINT8_MAX ? GetBeard() : a.beard;
	uint8 new_beard_color = a.beard_color == UINT8_MAX ? GetBeardColor() : a.beard_color;

	uint8 new_eye_color_one = a.eye_color_one == UINT8_MAX ? GetEyeColor1() : a.eye_color_one;
	uint8 new_eye_color_two = a.eye_color_two == UINT8_MAX ? GetEyeColor2() : a.eye_color_two;

	uint8 new_face = a.face == UINT8_MAX ? GetLuclinFace() : a.face;

	uint32 new_drakkin_details  = a.drakkin_details == UINT32_MAX ? GetDrakkinDetails() : a.drakkin_details;
	uint32 new_drakkin_heritage = a.drakkin_heritage == UINT32_MAX ? GetDrakkinHeritage() : a.drakkin_heritage;
	uint32 new_drakkin_tattoo   = a.drakkin_tattoo == UINT32_MAX ? GetDrakkinTattoo() : a.drakkin_tattoo;

	// Reset features to Base from the Player Profile
	if (IsClient() && a.race_id == Race::Doug) {
		new_beard            = CastToClient()->GetBaseBeard();
		new_beard_color      = CastToClient()->GetBaseBeardColor();
		new_drakkin_details  = CastToClient()->GetBaseDetails();
		new_drakkin_heritage = CastToClient()->GetBaseHeritage();
		new_drakkin_tattoo   = CastToClient()->GetBaseTattoo();
		new_eye_color_one    = CastToClient()->GetBaseEyeColor();
		new_eye_color_two    = CastToClient()->GetBaseEyeColor();
		new_face             = CastToClient()->GetBaseFace();
		new_gender           = CastToClient()->GetBaseGender();
		new_helmet_texture   = UINT8_MAX;
		new_hair             = CastToClient()->GetBaseHairStyle();
		new_hair_color       = CastToClient()->GetBaseHairColor();
		new_race             = CastToClient()->GetBaseRace();
		new_size             = CastToClient()->GetSize();
		new_texture          = UINT8_MAX;
	}

	beard            = new_beard;
	beardcolor       = new_beard_color;
	drakkin_heritage = new_drakkin_heritage;
	drakkin_tattoo   = new_drakkin_tattoo;
	drakkin_details  = new_drakkin_details;
	eyecolor1        = new_eye_color_one;
	eyecolor2        = new_eye_color_two;
	luclinface       = new_face;
	gender           = new_gender;
	hairstyle        = new_hair;
	haircolor        = new_hair_color;
	race             = new_race;
	size             = new_size;

	// These two should not be modified in base data - it kills db texture
	// when illusion is only for RandomizeFeatures...
	if (new_helmet_texture != UINT8_MAX) {
		helmtexture      = new_helmet_texture;
	}
	if (new_texture != UINT8_MAX) {
		texture          = new_texture;
	}

	auto outapp = new EQApplicationPacket(OP_Illusion, sizeof(Illusion_Struct));
	auto is     = (Illusion_Struct *) outapp->pBuffer;

	is->spawnid = GetID();
	strn0cpy(is->charname, GetCleanName(), sizeof(is->charname));
	is->beardcolor       = new_beard_color;
	is->beard            = new_beard;
	is->drakkin_heritage = new_drakkin_heritage;
	is->drakkin_tattoo   = new_drakkin_tattoo;
	is->drakkin_details  = new_drakkin_details;
	is->eyecolor1        = new_eye_color_one;
	is->eyecolor2        = new_eye_color_two;
	is->face             = new_face;
	is->gender           = new_gender;
	is->hairstyle        = new_hair;
	is->haircolor        = new_hair_color;
	is->helmtexture      = new_helmet_texture;
	is->race             = new_race;
	is->size             = new_size;
	is->texture          = new_texture;

	if (!a.target) {
		entity_list.QueueClients(this, outapp);
	} else {
		a.target->QueuePacket(outapp, false);
	}

	safe_delete(outapp);

	/* Refresh armor and tints after send illusion packet */
	SendArmorAppearance();

	if (a.send_effects) {
		SendSavedAppearanceEffects(nullptr);
	}

	LogSpells(
		"Illusion: Race [{}] Gender [{}] Texture [{}] HelmTexture [{}] HairColor [{}] BeardColor [{}] EyeColor1 [{}] EyeColor2 [{}] HairStyle [{}] Face [{}] DrakkinHeritage [{}] DrakkinTattoo [{}] DrakkinDetails [{}] Size [{}] Target [{}]",
		race,
		gender,
		new_texture,
		new_helmet_texture,
		new_hair_color,
		new_beard_color,
		new_eye_color_one,
		new_eye_color_two,
		new_hair,
		new_face,
		new_drakkin_heritage,
		new_drakkin_tattoo,
		new_drakkin_details,
		new_size,
		target ? target->GetCleanName() : "No Target"
	);
}

void Mob::SetFaceAppearance(const FaceChange_Struct& face, bool skip_sender)
{
	haircolor        = face.haircolor;
	beardcolor       = face.beardcolor;
	eyecolor1        = face.eyecolor1;
	eyecolor2        = face.eyecolor2;
	hairstyle        = face.hairstyle;
	luclinface       = face.face;
	beard            = face.beard;
	drakkin_heritage = face.drakkin_heritage;
	drakkin_tattoo   = face.drakkin_tattoo;
	drakkin_details  = face.drakkin_details;

	EQApplicationPacket outapp(OP_SetFace, sizeof(FaceChange_Struct));
	memcpy(outapp.pBuffer, &face, sizeof(FaceChange_Struct));
	auto buf = reinterpret_cast<FaceChange_Struct*>(outapp.pBuffer);
	buf->entity_id = GetID();

	entity_list.QueueClients(this, &outapp, skip_sender);
}

bool Mob::RandomizeFeatures(bool send_illusion, bool set_variables)
{
	if (!IsPlayerRace(GetRace())) {
		return false;
	}

	uint8  current_gender       = GetGender();
	uint8  new_texture          = UINT8_MAX;
	uint8  new_helm_texture     = UINT8_MAX;
	uint8  new_hair_color       = UINT8_MAX;
	uint8  new_beard_color      = UINT8_MAX;
	uint8  new_eye_color_one    = zone->random.Int(0, 9);
	uint8  new_eye_color_two    = zone->random.Int(0, 9);
	uint8  new_hair_style       = UINT8_MAX;
	uint8  new_luclin_face      = zone->random.Int(0, 7);
	uint8  new_beard            = UINT8_MAX;
	uint32 new_drakkin_heritage = UINT32_MAX;
	uint32 new_drakkin_tattoo   = UINT32_MAX;
	uint32 new_drakkin_details  = UINT32_MAX;

	// Adjust all settings based on the min and max for each feature of each race and gender
	switch (GetRace()) {
		case Race::Human:
			new_hair_color = zone->random.Int(0, 19);

			if (current_gender == Gender::Male) {
				new_beard_color = new_hair_color;
				new_hair_style  = zone->random.Int(0, 3);
				new_beard       = zone->random.Int(0, 5);
			} else if (current_gender == Gender::Female) {
				new_hair_style = zone->random.Int(0, 2);
			}

			break;
		case Race::Barbarian:
			new_hair_color  = zone->random.Int(0, 19);
			new_luclin_face = zone->random.Int(0, 87);

			if (current_gender == Gender::Male) {
				new_beard_color = new_hair_color;
				new_hair_style  = zone->random.Int(0, 3);
				new_beard       = zone->random.Int(0, 5);
			} else if (current_gender == Gender::Female) {
				new_hair_style = zone->random.Int(0, 2);
			}

			break;
		case Race::Erudite:
			if (current_gender == Gender::Male) {
				new_beard_color = zone->random.Int(0, 19);
				new_beard       = zone->random.Int(0, 5);
				new_luclin_face = zone->random.Int(0, 57);
			} else if (current_gender == Gender::Female) {
				new_luclin_face = zone->random.Int(0, 87);
			}

			break;
		case Race::WoodElf:
			new_hair_color = zone->random.Int(0, 19);

			if (current_gender == Gender::Male) {
				new_hair_style = zone->random.Int(0, 3);
			} else if (current_gender == Gender::Female) {
				new_hair_style = zone->random.Int(0, 2);
			}

			break;
		case Race::HighElf:
			new_hair_color = zone->random.Int(0, 14);

			if (current_gender == Gender::Male) {
				new_hair_style  = zone->random.Int(0, 3);
				new_luclin_face = zone->random.Int(0, 37);
				new_beard_color = new_hair_color;
			} else if (current_gender == Gender::Female) {
				new_hair_style = zone->random.Int(0, 2);
			}

			break;
		case Race::DarkElf:
			new_hair_color = zone->random.Int(13, 18);

			if (current_gender == Gender::Male) {
				new_hair_style  = zone->random.Int(0, 3);
				new_luclin_face = zone->random.Int(0, 37);
				new_beard_color = new_hair_color;
			} else if (current_gender == Gender::Female) {
				new_hair_style = zone->random.Int(0, 2);
			}

			break;
		case Race::HalfElf:
			new_hair_color = zone->random.Int(0, 19);

			if (current_gender == Gender::Male) {
				new_hair_style  = zone->random.Int(0, 3);
				new_luclin_face = zone->random.Int(0, 37);
				new_beard_color = new_hair_color;
			} else if (current_gender == Gender::Female) {
				new_hair_style = zone->random.Int(0, 2);
			}

			break;
		case Race::Dwarf:
			new_hair_color  = zone->random.Int(0, 19);
			new_beard_color = new_hair_color;

			if (current_gender == Gender::Male) {
				new_hair_style = zone->random.Int(0, 3);
				new_beard      = zone->random.Int(0, 5);
			} else if (current_gender == Gender::Female) {
				new_hair_style  = zone->random.Int(0, 2);
				new_luclin_face = zone->random.Int(0, 17);
			}

			break;
		case Race::Troll:
			new_eye_color_one = zone->random.Int(0, 10);
			new_eye_color_two = zone->random.Int(0, 10);

			if (current_gender == Gender::Female) {
				new_hair_style = zone->random.Int(0, 3);
				new_hair_color = zone->random.Int(0, 23);
			}

			break;
		case Race::Ogre:
			if (current_gender == Gender::Female) {
				new_hair_style = zone->random.Int(0, 3);
				new_hair_color = zone->random.Int(0, 23);
			}

			break;
		case Race::Halfling:
			new_hair_color = zone->random.Int(0, 19);

			if (current_gender == Gender::Male) {
				new_beard_color = new_hair_color;
				new_hair_style  = zone->random.Int(0, 3);
				new_beard       = zone->random.Int(0, 5);
			} else if (current_gender == Gender::Female) {
				new_hair_style = zone->random.Int(0, 2);
			}

			break;
		case Race::Gnome:
			new_hair_color = zone->random.Int(0, 24);

			if (current_gender == Gender::Male) {
				new_beard_color = new_hair_color;
				new_hair_style  = zone->random.Int(0, 3);
				new_beard       = zone->random.Int(0, 5);
			} else if (current_gender == Gender::Female) {
				new_hair_style = zone->random.Int(0, 2);
			}

			break;
		case Race::Iksar:
		case Race::VahShir:
			new_luclin_face = zone->random.Int(0, 7);
			break;
		case Race::Froglok2:
			new_luclin_face = zone->random.Int(0, 9);
			break;
		case Race::Drakkin:
			new_hair_color       = zone->random.Int(0, 3);
			new_beard_color      = new_hair_color;
			new_eye_color_one    = zone->random.Int(0, 11);
			new_eye_color_two    = zone->random.Int(0, 11);
			new_luclin_face      = zone->random.Int(0, 6);
			new_drakkin_heritage = zone->random.Int(0, 6);
			new_drakkin_tattoo   = zone->random.Int(0, 7);
			new_drakkin_details  = zone->random.Int(0, 7);

			if (current_gender == Gender::Male) {
				new_beard      = zone->random.Int(0, 12);
				new_hair_style = zone->random.Int(0, 8);
			} else if (current_gender == Gender::Female) {
				new_beard      = zone->random.Int(0, 3);
				new_hair_style = zone->random.Int(0, 7);
			}

			break;
		default:
			break;
	}

	if (set_variables) {
		haircolor        = new_hair_color;
		beardcolor       = new_beard_color;
		eyecolor1        = new_eye_color_one;
		eyecolor2        = new_eye_color_two;
		hairstyle        = new_hair_style;
		luclinface       = new_luclin_face;
		beard            = new_beard;
		drakkin_heritage = new_drakkin_heritage;
		drakkin_tattoo   = new_drakkin_tattoo;
		drakkin_details  = new_drakkin_details;
	}

	if (send_illusion) {
		SendIllusionPacket(
			AppearanceStruct{
				.beard = new_beard,
				.beard_color = new_beard_color,
				.drakkin_details = new_drakkin_details,
				.drakkin_heritage = new_drakkin_heritage,
				.drakkin_tattoo = new_drakkin_tattoo,
				.eye_color_one = new_eye_color_one,
				.eye_color_two = new_eye_color_two,
				.face = new_luclin_face,
				.gender_id = current_gender,
				.hair = new_hair_style,
				.hair_color = new_hair_color,
				.helmet_texture = new_helm_texture,
				.race_id = GetRace(),
				.texture = new_texture,
			}
		);
	}

	return true;
}

uint16 Mob::GetFactionRace() {
	uint16 current_race = GetRace();
	if (IsPlayerRace(current_race) || current_race == Race::Tree ||
		current_race == Race::MinorIllusion) {
		return current_race;
	}
	else {
		return (GetBaseRace());
	}
}

uint8 Mob::GetDefaultGender(uint16 in_race, uint8 in_gender) {
	if (
		IsPlayerRace(in_race) ||
		in_race == Race::Brownie ||
		in_race == Race::Kerran ||
		in_race == Race::Lion ||
		in_race == Race::Drachnid ||
		in_race == Race::Zombie ||
		in_race == Race::QeynosCitizen ||
		in_race == Race::RivervaleCitizen ||
		in_race == Race::HalasCitizen ||
		in_race == Race::GrobbCitizen ||
		in_race == Race::OggokCitizen ||
		in_race == Race::KaladimCitizen ||
		in_race == Race::ElfVampire ||
		in_race == Race::Felguard ||
		in_race == Race::Fayguard ||
		in_race == Race::EruditeGhost ||
		in_race == Race::IksarCitizen ||
		in_race == Race::Shade ||
		in_race == Race::TrollCrewMember ||
		in_race == Race::PirateDeckhand ||
		in_race == Race::GnomePirate ||
		in_race == Race::DarkElfPirate ||
		in_race == Race::OgrePirate ||
		in_race == Race::HumanPirate ||
		in_race == Race::EruditePirate ||
		in_race == Race::TrollZombie ||
		in_race == Race::KnightOfHate ||
		in_race == Race::ArcanistOfHate ||
		in_race == Race::UndeadVampire ||
		in_race == Race::Vampire3 ||
		in_race == Race::SandElf ||
		in_race == Race::Nihil ||
		in_race == Race::Trusik ||
		in_race == Race::Drachnid2 ||
		in_race == Race::Zombie2 ||
		in_race == Race::Elddar ||
		in_race == Race::Vampire4 ||
		in_race == Race::Kerran2 ||
		in_race == Race::Brownie2 ||
		in_race == Race::Human2 ||
		in_race == Race::ElvenGhost ||
		in_race == Race::HumanGhost ||
		in_race == Race::Coldain2 ||
		in_race == Race::Akheva
	) {
		if (in_gender >= Gender::Neuter) { // Male default for PC Races
			return Gender::Male;
		} else {
			return in_gender;
		}
	} else if (
		in_race == Race::FreeportGuard ||
		in_race == Race::Mimic ||
		in_race == Race::HumanBeggar ||
		in_race == Race::Vampire ||
		in_race == Race::HighpassCitizen ||
		in_race == Race::NeriakCitizen ||
		in_race == Race::EruditeCitizen ||
		in_race == Race::ClockworkGnome ||
		in_race == Race::DwarfGhost ||
		in_race == Race::IksarSpirit ||
		in_race == Race::InvisibleMan ||
		in_race == Race::Vampire2 ||
		in_race == Race::Recuso ||
		in_race == Race::BrokenSkullPirate ||
		in_race == Race::InvisibleManOfZomm ||
		in_race == Race::Ogre2 ||
		in_race == Race::RoyalGuard ||
		in_race == Race::Erudite2
	) { // Male only races
		return Gender::Male;
	} else if (
		in_race == Race::Fairy ||
		in_race == Race::Pixie ||
		in_race == Race::Banshee2 ||
		in_race == Race::Banshee3 ||
		in_race == Race::AyonaeRo ||
		in_race == Race::SullonZek
	) { // Female only races
		return Gender::Female;
	} else { // Neutral default for NPC Races
		return Gender::Neuter;
	}
}

void Mob::SendAppearancePacket(
	uint32 type,
	uint32 value,
	bool whole_zone,
	bool ignore_self,
	Client* target
)
{
	if (!GetID()) {
		return;
	}

	auto outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
	auto* a = (SpawnAppearance_Struct*)outapp->pBuffer;

	a->spawn_id  = GetID();
	a->type      = type;
	a->parameter = value;

	if (whole_zone) {
		entity_list.QueueClients(this, outapp, ignore_self);
	} else if (target) {
		target->QueuePacket(outapp, false, Client::CLIENT_CONNECTED);
	} else if (IsClient()) {
		CastToClient()->QueuePacket(outapp, false, Client::CLIENT_CONNECTED);
	}

	safe_delete(outapp);
}

void Mob::SendLevelAppearance(){
	auto outapp = new EQApplicationPacket(OP_LevelAppearance, sizeof(LevelAppearance_Struct));
	LevelAppearance_Struct* la = (LevelAppearance_Struct*)outapp->pBuffer;
	la->parm1 = 0x4D;
	la->parm2 = la->parm1 + 1;
	la->parm3 = la->parm2 + 1;
	la->parm4 = la->parm3 + 1;
	la->parm5 = la->parm4 + 1;
	la->spawn_id = GetID();
	la->value1a = 1;
	la->value2a = 2;
	la->value3a = 1;
	la->value3b = 1;
	la->value4a = 1;
	la->value4b = 1;
	la->value5a = 2;
	entity_list.QueueCloseClients(this,outapp);
	safe_delete(outapp);
}

void Mob::SendAppearanceEffect(uint32 parm1, uint32 parm2, uint32 parm3, uint32 parm4, uint32 parm5, Client *specific_target,
	uint32 value1slot, uint32 value1ground, uint32 value2slot, uint32 value2ground, uint32 value3slot, uint32 value3ground,
	uint32 value4slot, uint32 value4ground, uint32 value5slot, uint32 value5ground){
	auto outapp = new EQApplicationPacket(OP_LevelAppearance, sizeof(LevelAppearance_Struct));

	/* Location of the effect from value#slot, this is removed upon mob death/despawn.
		0 = pelvis1
		1 = pelvis2
		2 = helm
		3 = Offhand
		4 = Mainhand
		5 = left foot
		6 = right foot
		9 = Face

		value#ground = 1, will place the effect on ground, this is permanenant
	*/

	//higher values can crash client
	if (value1slot > 9) {
		value1slot = 1;
	}
	if (value2slot > 9) {
		value2slot = 1;
	}
	if (value2slot > 9) {
		value2slot = 1;
	}
	if (value3slot > 9) {
		value3slot = 1;
	}
	if (value4slot > 9) {
		value4slot = 1;
	}
	if (value5slot > 9) {
		value5slot = 1;
	}

	if (!value1ground && parm1) {
		SetAppearanceEffects(value1slot, parm1);
	}
	if (!value2ground && parm2) {
		SetAppearanceEffects(value2slot, parm2);
	}
	if (!value3ground && parm3) {
		SetAppearanceEffects(value3slot, parm3);
	}
	if (!value4ground && parm4) {
		SetAppearanceEffects(value4slot, parm4);
	}
	if (!value5ground && parm5) {
		SetAppearanceEffects(value5slot, parm5);
	}

	LevelAppearance_Struct* la = (LevelAppearance_Struct*)outapp->pBuffer;
	la->spawn_id = GetID();
	la->parm1 = parm1;
	la->parm2 = parm2;
	la->parm3 = parm3;
	la->parm4 = parm4;
	la->parm5 = parm5;
	// Note that setting the b values to 0 will disable the related effect from the corresponding parameter.
	// Setting the a value appears to have no affect at all.s
	la->value1a = value1slot;
	la->value1b = value1ground;
	la->value2a = value2slot;
	la->value2b = value2ground;
	la->value3a = value3slot;
	la->value3b = value3ground;
	la->value4a = value4slot;
	la->value4b = value4ground;
	la->value5a = value5slot;
	la->value5b = value5ground;
	if(specific_target == nullptr) {
		entity_list.QueueClients(this,outapp);
	}
	else if (specific_target->IsClient()) {
		specific_target->CastToClient()->QueuePacket(outapp, false);
	}
	safe_delete(outapp);
}

void Mob::SetAppearanceEffects(int32 slot, int32 value)
{
	for (int i = 0; i < MAX_APPEARANCE_EFFECTS; i++) {
		if (!appearance_effects_id[i]) {
			appearance_effects_id[i] = value;
			appearance_effects_slot[i] = slot;
			return;
		}
	}
}

void Mob::ListAppearanceEffects(Client* c)
{
	if (!appearance_effects_id[0]) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} no appearance effects.",
				c->GetTargetDescription(this, TargetDescriptionType::UCYou),
				c == this ? "have" : "has"
			).c_str()
		);
		return;
	}

	for (int i = 0; i < MAX_APPEARANCE_EFFECTS; i++) {
		c->Message(
			Chat::Red,
			fmt::format(
				"Effect {} | ID: {} Slot: {}",
				i,
				appearance_effects_id[i],
				appearance_effects_slot[i]
			).c_str()
		);
	}
}

void Mob::ClearAppearanceEffects()
{
	for (int i = 0; i < MAX_APPEARANCE_EFFECTS; i++) {
		appearance_effects_id[i] = 0;
		appearance_effects_slot[i] = 0;
	}
}

void Mob::SendSavedAppearanceEffects(Client *receiver = nullptr)
{
	if (!appearance_effects_id[0]) {
		return;
	}

	if (appearance_effects_id[0]) {
		SendAppearanceEffect(appearance_effects_id[0], appearance_effects_id[1], appearance_effects_id[2], appearance_effects_id[3], appearance_effects_id[4], receiver,
			appearance_effects_slot[0], 0, appearance_effects_slot[1], 0, appearance_effects_slot[2], 0, appearance_effects_slot[3], 0, appearance_effects_slot[4], 0);
	}
	if (appearance_effects_id[5]) {
		SendAppearanceEffect(appearance_effects_id[5], appearance_effects_id[6], appearance_effects_id[7], appearance_effects_id[8], appearance_effects_id[9], receiver,
			appearance_effects_slot[5], 0, appearance_effects_slot[6], 0, appearance_effects_slot[7], 0, appearance_effects_slot[8], 0, appearance_effects_slot[9], 0);
	}
	if (appearance_effects_id[10]) {
		SendAppearanceEffect(appearance_effects_id[10], appearance_effects_id[11], appearance_effects_id[12], appearance_effects_id[13], appearance_effects_id[14], receiver,
			appearance_effects_slot[10], 0, appearance_effects_slot[11], 0, appearance_effects_slot[12], 0, appearance_effects_slot[13], 0, appearance_effects_slot[14], 0);
	}
	if (appearance_effects_id[15]) {
		SendAppearanceEffect(appearance_effects_id[15], appearance_effects_id[16], appearance_effects_id[17], appearance_effects_id[18], appearance_effects_id[19], receiver,
			appearance_effects_slot[15], 0, appearance_effects_slot[16], 0, appearance_effects_slot[17], 0, appearance_effects_slot[18], 0, appearance_effects_slot[19], 0);
	}
}

void Mob::SendTargetable(bool on, Client *specific_target) {
	auto outapp = new EQApplicationPacket(OP_Untargetable, sizeof(Untargetable_Struct));
	Untargetable_Struct *ut = (Untargetable_Struct*)outapp->pBuffer;
	ut->id = GetID();
	ut->targetable_flag = on == true ? 1 : 0;

	if(specific_target == nullptr) {
		entity_list.QueueClients(this, outapp);
	}
	else if (specific_target->IsClient()) {
		specific_target->CastToClient()->QueuePacket(outapp);
	}
	safe_delete(outapp);
}

void Mob::CameraEffect(uint32 duration, float intensity, Client *c, bool global) {


	if(global == true)
	{
		auto pack = new ServerPacket(ServerOP_CameraShake, sizeof(ServerCameraShake_Struct));
		ServerCameraShake_Struct* scss = (ServerCameraShake_Struct*) pack->pBuffer;
		scss->duration = duration;
		scss->intensity = intensity;
		worldserver.SendPacket(pack);
		safe_delete(pack);
		return;
	}

	auto outapp = new EQApplicationPacket(OP_CameraEffect, sizeof(Camera_Struct));
	Camera_Struct* cs = (Camera_Struct*) outapp->pBuffer;
	cs->duration = duration;	// Duration in milliseconds
	cs->intensity = intensity;

	if(c)
		c->QueuePacket(outapp, false, Client::CLIENT_CONNECTED);
	else
		entity_list.QueueClients(this, outapp);

	safe_delete(outapp);
}

void Mob::SendSpellEffect(uint32 effect_id, uint32 duration, uint32 finish_delay, bool zone_wide, uint32 unk020, bool perm_effect, Client *c, uint32 caster_id, uint32 target_id) {

	if (!caster_id) {
		caster_id = GetID();
	}

	if (!target_id) {
		target_id = GetID();
	}

	auto outapp = new EQApplicationPacket(OP_SpellEffect, sizeof(SpellEffect_Struct));
	SpellEffect_Struct* se = (SpellEffect_Struct*) outapp->pBuffer;
	se->EffectID = effect_id;	// ID of the Particle Effect
	se->EntityID = caster_id; //casting graphic animation
	se->EntityID2 = target_id;	// //target graphic animation
	se->Duration = duration;	// In Milliseconds
	se->FinishDelay = finish_delay;	// Seen 0
	se->Unknown020 = unk020;	// Seen 3000
	se->Unknown024 = 1;		// Seen 1 for SoD
	se->Unknown025 = 1;		// Seen 1 for Live
	se->Unknown026 = 0;		// Seen 1157

	if(c)
		c->QueuePacket(outapp, false, Client::CLIENT_CONNECTED);
	else if(zone_wide)
		entity_list.QueueClients(this, outapp);
	else
		entity_list.QueueCloseClients(this, outapp);

	safe_delete(outapp);

	if (perm_effect) {
		if(!IsNimbusEffectActive(effect_id)) {
			SetNimbusEffect(effect_id);
		}
	}

}

void Mob::TempName(const char *newname)
{
	char temp_name[64];
	char old_name[64];
	strn0cpy(old_name, GetName(), 64);
	clean_name[0] = 0;

	if(newname)
		strn0cpy(temp_name, newname, 64);

	// Reset the name to the original if left null.
	if(!newname) {
		strn0cpy(temp_name, GetOrigName(), 64);
		SetName(temp_name);
		strn0cpy(temp_name, GetCleanName(), 64);
	}

	// Remove Numbers before making name unique
	EntityList::RemoveNumbers(temp_name);
	// Make the new name unique and set it
	entity_list.MakeNameUnique(temp_name);

	// Send the new name to all clients
	SendRename(this, old_name, temp_name);

	SetName(temp_name);
}

void Mob::SetTargetable(bool on) {
	if(m_targetable != on) {
		m_targetable = on;
		SendTargetable(on);
	}
}

const int64& Mob::SetMana(int64 amount)
{
	CalcMaxMana();
	int64 mmana = GetMaxMana();
	current_mana = amount < 0 ? 0 : (amount > mmana ? mmana : amount);

	return current_mana;
}


void Mob::SetAppearance(EmuAppearance app, bool ignore_self) {
	if (_appearance == app) {
		return;
	}

	_appearance = app;

	SendAppearancePacket(AppearanceType::Animation, GetAppearanceValue(app), true, ignore_self);

	if (IsClient() && IsAIControlled()) {
		SendAppearancePacket(AppearanceType::Animation, Animation::Freeze, false);
	}
}

bool Mob::UpdateActiveLight()
{
	uint8 old_light_level = m_Light.Level[EQ::lightsource::LightActive];

	m_Light.Type[EQ::lightsource::LightActive] = 0;
	m_Light.Level[EQ::lightsource::LightActive] = 0;

	if (EQ::lightsource::IsLevelGreater((m_Light.Type[EQ::lightsource::LightInnate] & 0x0F), m_Light.Type[EQ::lightsource::LightActive])) { m_Light.Type[EQ::lightsource::LightActive] = m_Light.Type[EQ::lightsource::LightInnate]; }
	if (m_Light.Level[EQ::lightsource::LightEquipment] > m_Light.Level[EQ::lightsource::LightActive]) { m_Light.Type[EQ::lightsource::LightActive] = m_Light.Type[EQ::lightsource::LightEquipment]; } // limiter in property handler
	if (m_Light.Level[EQ::lightsource::LightSpell] > m_Light.Level[EQ::lightsource::LightActive]) { m_Light.Type[EQ::lightsource::LightActive] = m_Light.Type[EQ::lightsource::LightSpell]; } // limiter in property handler

	m_Light.Level[EQ::lightsource::LightActive] = EQ::lightsource::TypeToLevel(m_Light.Type[EQ::lightsource::LightActive]);

	return (m_Light.Level[EQ::lightsource::LightActive] != old_light_level);
}

void Mob::SendWearChangeAndLighting(int8 last_texture) {

	for (int i = EQ::textures::textureBegin; i <= last_texture; i++) {
		SendWearChange(i);
	}
	UpdateActiveLight();
	SendAppearancePacket(AppearanceType::Light, GetActiveLightType());

}

void Mob::ChangeSize(float in_size = 0, bool unrestricted)
{
	size = std::clamp(in_size, 1.0f, 255.0f);

	if (!unrestricted) {
		if (IsClient() || petid != 0) {
			size = std::clamp(in_size, 3.0f, 15.0f);
		}
	}

	SendAppearancePacket(AppearanceType::Size, static_cast<uint32>(size));
}

Mob* Mob::GetOwnerOrSelf()
{
	if (!GetOwnerID()) {
		return this;
	}

	Mob* m = entity_list.GetMob(GetOwnerID());

	if (!m) {
		SetOwnerID(0);
		return this;
	}

	if (m->GetPetID() == GetID()) {
		return m;
	}

	if (IsNPC() && CastToNPC()->GetSwarmInfo()){
		return CastToNPC()->GetSwarmInfo()->GetOwner();
	}

	SetOwnerID(0);
	return this;
}

Mob* Mob::GetOwner() {
	Mob* m = entity_list.GetMob(GetOwnerID());

	if (m && m->GetPetID() == GetID()) {
		return m;
	}

	if(IsNPC() && CastToNPC()->GetSwarmInfo()){
		return CastToNPC()->GetSwarmInfo()->GetOwner();
	}

	SetOwnerID(0);
	return 0;
}

Mob* Mob::GetUltimateOwner()
{
	Mob* m = GetOwner();

	if (!m) {
		return this;
	}

	while (m && m->HasOwner()) {
		m = m->GetOwner();
	}

	return m ? m : this;
}

void Mob::SetOwnerID(uint16 new_owner_id) {
	if (new_owner_id && new_owner_id == GetID()) {
		return;
	}

	ownerid = new_owner_id;

	// if we're setting the owner ID to 0 and they're not either charmed or not-a-pet then
	// they're a normal pet and should be despawned
	if (
		!ownerid &&
		IsNPC() &&
		GetPetType() != PetType::Charmed &&
		GetPetType() != PetType::None
	) {
		Depop();
	}
}

// used in checking for behind (backstab) and checking in front (melee LoS)
float Mob::MobAngle(Mob *other, float ourx, float oury) const {
	if (!other || other == this)
		return 0.0f;

	float angle, lengthb, vectorx, vectory, dotp;
	float mobx = -(other->GetX());	// mob xloc (inverse because eq)
	float moby = other->GetY();		// mob yloc
	float heading = other->GetHeading();	// mob heading
	heading = (heading * 360.0f) / 512.0f;	// convert to degrees
	if (heading < 270)
		heading += 90;
	else
		heading -= 270;

	heading = heading * 3.1415f / 180.0f;	// convert to radians
	vectorx = mobx + (10.0f * std::cos(heading));	// create a vector based on heading
	vectory = moby + (10.0f * std::sin(heading));	// of mob length 10

	// length of mob to player vector
	lengthb = (float) std::sqrt(((-ourx - mobx) * (-ourx - mobx)) + ((oury - moby) * (oury - moby)));

	// calculate dot product to get angle
	// Handle acos domain errors due to floating point rounding errors
	dotp = ((vectorx - mobx) * (-ourx - mobx) +
			(vectory - moby) * (oury - moby)) / (10 * lengthb);
	// I haven't seen any errors that  cause problems that weren't slightly
	// larger/smaller than 1/-1, so only handle these cases for now
	if (dotp > 1)
		return 0.0f;
	else if (dotp < -1)
		return 180.0f;

	angle = std::acos(dotp);
	angle = angle * 180.0f / 3.1415f;

	return angle;
}

void Mob::SetZone(uint32 zone_id, uint32 instance_id)
{
	if(IsClient())
	{
		CastToClient()->GetPP().zone_id = zone_id;
		CastToClient()->GetPP().zoneInstance = instance_id;
		CastToClient()->SaveCharacterData();
	}

	if (!IsClient()) {
		Save(); // bots or other things might be covered here for some reason
	}
}

void Mob::Kill() {
	Death(this, 0, SPELL_UNKNOWN, EQ::skills::SkillHandtoHand);
}

bool Mob::CanThisClassDualWield(void) const {
	if(!IsClient()) {
		return(GetSkill(EQ::skills::SkillDualWield) > 0);
	}
	else if (CastToClient()->HasSkill(EQ::skills::SkillDualWield)) {
		const EQ::ItemInstance* pinst = CastToClient()->GetInv().GetItem(EQ::invslot::slotPrimary);
		const EQ::ItemInstance* sinst = CastToClient()->GetInv().GetItem(EQ::invslot::slotSecondary);

		// 2HS, 2HB, or 2HP
		if(pinst && pinst->IsWeapon()) {
			const EQ::ItemData* item = pinst->GetItem();

			if (item->IsType2HWeapon())
				return false;
		}

		// OffHand Weapon
		if(sinst && !sinst->IsWeapon())
			return false;

		// Dual-Wielding Empty Fists
		if(!pinst && !sinst)
			if(class_ != Class::Monk && class_ != Class::MonkGM && class_ != Class::Beastlord && class_ != Class::BeastlordGM)
				return false;

		return true;
	}

	return false;
}

bool Mob::CanThisClassDoubleAttack(void) const
{
	if(!IsClient()) {
		return(GetSkill(EQ::skills::SkillDoubleAttack) > 0);
	} else {
		if(aabonuses.GiveDoubleAttack || itembonuses.GiveDoubleAttack || spellbonuses.GiveDoubleAttack) {
			return true;
		}
		return(CastToClient()->HasSkill(EQ::skills::SkillDoubleAttack));
	}
}

bool Mob::CanThisClassTripleAttack() const {
	if (!IsOfClientBot()) {
		return false; // Mobs lost the ability to triple attack when the real skill was added
	}

	if (RuleB(Combat, ClassicTripleAttack)) {
		return GetLevel() >= 60 && (
			GetClass() == Class::Warrior ||
			GetClass() == Class::Ranger ||
			GetClass() == Class::Monk ||
			GetClass() == Class::Berserker
		);
	}

	return IsClient() ? CastToClient()->HasSkill(EQ::skills::SkillTripleAttack)
		: GetSkill(EQ::skills::SkillTripleAttack) > 0;
}

bool Mob::CanThisClassParry(void) const
{
	if(!IsClient()) {
		return(GetSkill(EQ::skills::SkillParry) > 0);
	} else {
		return(CastToClient()->HasSkill(EQ::skills::SkillParry));
	}
}

bool Mob::CanThisClassDodge(void) const
{
	if(!IsClient()) {
		return(GetSkill(EQ::skills::SkillDodge) > 0);
	} else {
		return(CastToClient()->HasSkill(EQ::skills::SkillDodge));
	}
}

bool Mob::CanThisClassRiposte(void) const
{
	if(!IsClient()) {
		return(GetSkill(EQ::skills::SkillRiposte) > 0);
	} else {
		return(CastToClient()->HasSkill(EQ::skills::SkillRiposte));
	}
}

bool Mob::CanThisClassBlock(void) const
{
	if(!IsClient()) {
		return(GetSkill(EQ::skills::SkillBlock) > 0);
	} else {
		return(CastToClient()->HasSkill(EQ::skills::SkillBlock));
	}
}
/*
float Mob::GetReciprocalHeading(Mob* target) {
	float Result = 0;

	if(target) {
		// Convert to radians
		float h = (target->GetHeading() / 256.0f) * 6.283184f;

		// Calculate the reciprocal heading in radians
		Result = h + 3.141592f;

		// Convert back to eq heading from radians
		Result = (Result / 6.283184f) * 256.0f;
	}

	return Result;
}
*/
bool Mob::PlotPositionAroundTarget(Mob* target, float &x_dest, float &y_dest, float &z_dest, bool lookForAftArc) {
	bool Result = false;

	if(target) {
		float look_heading = 0;

		if(lookForAftArc)
			look_heading = GetReciprocalHeading(target->GetPosition());
		else
			look_heading = target->GetHeading();

		// Convert to sony heading to radians
		look_heading = (look_heading / 512.0f) * 6.283184f;

		float tempX = 0;
		float tempY = 0;
		float tempZ = 0;
		float tempSize = 0;
		const float rangeCreepMod = 0.25;
		const uint8 maxIterationsAllowed = 4;
		uint8 counter = 0;
		float rangeReduction= 0;

		tempSize = target->GetSize();
		rangeReduction = (tempSize * rangeCreepMod);

		while(tempSize > 0 && counter != maxIterationsAllowed) {
			tempX = GetX() + (tempSize * static_cast<float>(sin(double(look_heading))));
			tempY = GetY() + (tempSize * static_cast<float>(cos(double(look_heading))));
			tempZ = target->GetZ();

			if(!CheckLosFN(tempX, tempY, tempZ, tempSize)) {
				tempSize -= rangeReduction;
			}
			else {
				Result = true;
				break;
			}

			counter++;
		}

		if(!Result) {
			// Try to find an attack arc to position at from the opposite direction.
			look_heading += (3.141592 / 2);

			tempSize = target->GetSize();
			counter = 0;

			while(tempSize > 0 && counter != maxIterationsAllowed) {
				tempX = GetX() + (tempSize * static_cast<float>(sin(double(look_heading))));
				tempY = GetY() + (tempSize * static_cast<float>(cos(double(look_heading))));
				tempZ = target->GetZ();

				if(!CheckLosFN(tempX, tempY, tempZ, tempSize)) {
					tempSize -= rangeReduction;
				}
				else {
					Result = true;
					break;
				}

				counter++;
			}
		}

		if(Result) {
			x_dest = tempX;
			y_dest = tempY;
			z_dest = tempZ;
		}
	}

	return Result;
}

bool Mob::HateSummon() {
	// check if mob has ability to summon
	// 97% is the offical % that summoning starts on live, not 94
	if (IsCharmed())
		return false;

	int summon_level = GetSpecialAbility(SpecialAbility::Summon);
	if(summon_level == 1 || summon_level == 2) {
		if(!GetTarget()) {
			return false;
		}
	} else {
		//unsupported summon level or OFF
		return false;
	}

	// validate hp
	int hp_ratio = GetSpecialAbilityParam(SpecialAbility::Summon, 1);
	hp_ratio = hp_ratio > 0 ? hp_ratio : 97;
	if(GetHPRatio() > static_cast<float>(hp_ratio)) {
		return false;
	}

	// now validate the timer
	int summon_timer_duration = GetSpecialAbilityParam(SpecialAbility::Summon, 0);
	summon_timer_duration = summon_timer_duration > 0 ? summon_timer_duration : 6000;
	Timer *timer = GetSpecialAbilityTimer(SpecialAbility::Summon);
	if (!timer)
	{
		StartSpecialAbilityTimer(SpecialAbility::Summon, summon_timer_duration);
	} else {
		if(!timer->Check())
			return false;

		timer->Start(summon_timer_duration);
	}

	// get summon target
	SetTarget(GetHateTop());
	if(target)
	{
		if(summon_level == 1) {
			entity_list.MessageClose(this, true, 500, Chat::Say, "%s says 'You will not evade me, %s!' ", GetCleanName(), target->GetCleanName() );

			float summoner_zoff = GetZOffset();
			float summoned_zoff = target->GetZOffset();
			auto new_pos = m_Position;
			new_pos.z -= (summoner_zoff - summoned_zoff);
			float angle = new_pos.w - target->GetHeading();
			new_pos.w = target->GetHeading();

			// probably should be like half melee range, but we can't get melee range nicely because reasons :)
			new_pos = target->TryMoveAlong(new_pos, 5.0f, angle);

			if (target->IsClient()) {
				target->CastToClient()->MovePC(
					zone->GetZoneID(),
					zone->GetInstanceID(),
					new_pos.x,
					new_pos.y,
					new_pos.z,
					new_pos.w,
					0,
					SummonPC
				);
			} else {
				bool target_is_client_pet = (
					target->IsPet() &&
					target->IsPetOwnerOfClientBot()
				);
				bool set_new_guard_spot = !(IsNPC() && target_is_client_pet);

				target->GMMove(
					new_pos.x,
					new_pos.y,
					new_pos.z,
					new_pos.w,
					set_new_guard_spot
				);
			}

			return true;
		} else if(summon_level == 2) {
			entity_list.MessageClose(this, true, 500, Chat::Say, "%s says 'You will not evade me, %s!'", GetCleanName(), target->GetCleanName());
			GMMove(target->GetX(), target->GetY(), target->GetZ());
		}
	}
	return false;
}

void Mob::FaceTarget(Mob* mob_to_face /*= 0*/) {

	if (GetIsBoat()) {
		return;
	}

	Mob* faced_mob = mob_to_face;
	if(!faced_mob) {
		if(!GetTarget()) {
			return;
		}
		else {
			faced_mob = GetTarget();
		}
	}

	float current_heading = GetHeading();
	float new_heading = CalculateHeadingToTarget(faced_mob->GetX(), faced_mob->GetY());
	if(current_heading != new_heading) {
		if (IsEngaged() || IsRunning()) {
			RotateToRunning(new_heading);
		}
		else {
			RotateToWalking(new_heading);
		}
	}

	if(IsNPC() && !IsEngaged()) {
		CastToNPC()->GetRefaceTimer()->Start(15000);
		CastToNPC()->GetRefaceTimer()->Enable();
	}
}

bool Mob::RemoveFromHateList(Mob* mob)
{
	SetRunAnimSpeed(0);
	bool bFound = false;
	if(IsEngaged())
	{
		bFound = hate_list.RemoveEntFromHateList(mob);
		if(hate_list.IsHateListEmpty())
		{
			AI_Event_NoLongerEngaged();
			zone->DelAggroMob();
			if (IsNPC() && !RuleB(Aggro, AllowTickPulling))
				ResetAssistCap();
		}
	}
	if(GetTarget() == mob)
	{
		SetTarget(hate_list.GetMobWithMostHateOnList(this));
	}

	return bFound;
}

void Mob::WipeHateList(bool npc_only) {
	if (IsEngaged()) {
		hate_list.WipeHateList(npc_only);
		if (hate_list.IsHateListEmpty()) {
			AI_Event_NoLongerEngaged();
		}
	} else {
		hate_list.WipeHateList(npc_only);
	}
}

uint32 Mob::RandomTimer(int min, int max)
{
	int r = 14000;
	if (min != 0 && max != 0 && min < max) {
		r = zone->random.Int(min, max);
	}
	return r;
}

uint32 Mob::IsEliteMaterialItem(uint8 material_slot) const
{
	const EQ::ItemData *item = nullptr;

	item = database.GetItem(GetEquippedItemFromTextureSlot(material_slot));
	if(item != 0)
	{
		return item->EliteMaterial;
	}

	return 0;
}

// works just like a printf
void Mob::Say(const char *format, ...)
{
	char    buf[1000];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buf, 1000, format, ap);
	va_end(ap);

	Mob *talker = this;
	if (spellbonuses.VoiceGraft != 0) {
		if (spellbonuses.VoiceGraft == GetPetID()) {
			talker = entity_list.GetMob(spellbonuses.VoiceGraft);
		}
		else {
			spellbonuses.VoiceGraft = 0;
		}
	}

	if (!talker) {
		talker = this;
	}

	int16 distance = 200;

	if (RuleB(Chat, QuestDialogueUsesDialogueWindow)) {
		for (auto &e : talker->GetCloseMobList(distance)) {
			Mob *mob = e.second;
			if (!mob) {
				continue;
			}

			if (!mob->IsClient()) {
				continue;
			}

			Client *client = mob->CastToClient();
			if (client->GetTarget() && client->GetTarget()->IsMob() && client->GetTarget()->CastToMob() == talker) {
				std::string window_markdown = buf;
				DialogueWindow::Render(client, window_markdown);
			}
		}

		return;
	}
	else if (RuleB(Chat, AutoInjectSaylinksToSay)) {
		std::string new_message = EQ::SayLinkEngine::InjectSaylinksIfNotExist(buf);
		entity_list.MessageCloseString(
			talker, false, distance, Chat::NPCQuestSay,
			GENERIC_SAY, GetCleanName(), new_message.c_str()
		);
	}
	else {
		entity_list.MessageCloseString(
			talker, false, distance, Chat::NPCQuestSay,
			GENERIC_SAY, GetCleanName(), buf
		);
	}
}

//
// this is like the above, but the first parameter is a string id
//
void Mob::SayString(uint32 string_id, const char *message3, const char *message4, const char *message5, const char *message6, const char *message7, const char *message8, const char *message9)
{
	char string_id_str[10];

	snprintf(string_id_str, 10, "%d", string_id);

	entity_list.MessageCloseString(
		this, false, 200, 10,
		GENERIC_STRINGID_SAY, GetCleanName(), string_id_str, message3, message4, message5,
		message6, message7, message8, message9
	);
}

void Mob::SayString(uint32 type, uint32 string_id, const char *message3, const char *message4, const char *message5, const char *message6, const char *message7, const char *message8, const char *message9)
{
	char string_id_str[10];

	snprintf(string_id_str, 10, "%d", string_id);

	entity_list.MessageCloseString(
		this, false, 200, type,
		GENERIC_STRINGID_SAY, GetCleanName(), string_id_str, message3, message4, message5,
		message6, message7, message8, message9
	);
}

void Mob::SayString(Client *to, uint32 string_id, const char *message3, const char *message4, const char *message5, const char *message6, const char *message7, const char *message8, const char *message9)
{
	if (!to)
		return;

	auto string_id_str = std::to_string(string_id);

	to->MessageString(Chat::NPCQuestSay, GENERIC_STRINGID_SAY, GetCleanName(), string_id_str.c_str(), message3, message4, message5, message6, message7, message8, message9);
}

void Mob::SayString(Client *to, uint32 type, uint32 string_id, const char *message3, const char *message4, const char *message5, const char *message6, const char *message7, const char *message8, const char *message9)
{
	if (!to)
		return;

	auto string_id_str = std::to_string(string_id);

	to->MessageString(type, GENERIC_STRINGID_SAY, GetCleanName(), string_id_str.c_str(), message3, message4, message5, message6, message7, message8, message9);
}

void Mob::Shout(const char *format, ...)
{
	char buf[1000];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buf, 1000, format, ap);
	va_end(ap);

	entity_list.MessageString(this, false, Chat::Shout,
		GENERIC_SHOUT, GetCleanName(), buf);
}

void Mob::Emote(const char *format, ...)
{
	char buf[1000];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buf, 1000, format, ap);
	va_end(ap);

	entity_list.MessageCloseString(
		this, false, 200, 10,
		GENERIC_EMOTE, GetCleanName(), buf
	);
}

void Mob::QuestJournalledSay(Client *QuestInitiator, const char *str, Journal::Options &opts)
{
	// just in case
	if (opts.target_spawn_id == 0 && QuestInitiator)
		opts.target_spawn_id = QuestInitiator->GetID();

	entity_list.QuestJournalledSayClose(this, 200, GetCleanName(), str, opts);
}

const char *Mob::GetCleanName()
{
	if (!strlen(clean_name)) {
		CleanMobName(GetName(), clean_name);
	}

	return clean_name;
}

std::string Mob::GetTargetDescription(Mob* target, uint8 description_type, uint16 entity_id_override)
{
	std::string self_return = "yourself";

	switch (description_type)
	{
		case TargetDescriptionType::LCSelf:
		{
			self_return = "yourself";
			break;
		}
		case TargetDescriptionType::UCSelf:
		{
			self_return = "Yourself";
			break;
		}
		case TargetDescriptionType::LCYou:
		{
			self_return = "you";
			break;
		}
		case TargetDescriptionType::UCYou:
		{
			self_return = "You";
			break;
		}
		case TargetDescriptionType::LCYour:
		{
			self_return = "your";
			break;
		}
		case TargetDescriptionType::UCYour:
		{
			self_return = "Your";
			break;
		}
		default:
		{
			break;
		}
	}


	auto d = fmt::format(
		"{}",
		(
			target && this == target ?
			self_return :
			fmt::format(
				"{} ({})",
				target->GetCleanName(),
				entity_id_override ? entity_id_override : target->GetID()
			)
		)
	);

	return d;
}

// hp event
void Mob::SetNextHPEvent( int hpevent )
{
	nexthpevent = hpevent;
}

void Mob::SetNextIncHPEvent( int inchpevent )
{
	nextinchpevent = inchpevent;
}

uint32 Mob::GetLevelHP(uint8 tlevel)
{
	int multiplier = 0;
	if (tlevel < 10)
	{
		multiplier = tlevel*20;
	}
	else if (tlevel < 20)
	{
		multiplier = tlevel*25;
	}
	else if (tlevel < 40)
	{
		multiplier = tlevel*tlevel*12*((tlevel*2+60)/100)/10;
	}
	else if (tlevel < 45)
	{
		multiplier = tlevel*tlevel*15*((tlevel*2+60)/100)/10;
	}
	else if (tlevel < 50)
	{
		multiplier = tlevel*tlevel*175*((tlevel*2+60)/100)/100;
	}
	else
	{
		multiplier = tlevel*tlevel*2*((tlevel*2+60)/100)*(1+((tlevel-50)*20/10));
	}
	return multiplier;
}

int32 Mob::GetActSpellCasttime(uint16 spell_id, int32 casttime)
{
	int32 cast_reducer = GetFocusEffect(focusSpellHaste, spell_id);
	int32 cast_reducer_amt = GetFocusEffect(focusFcCastTimeAmt, spell_id);
	int32 cast_reducer_no_limit = GetFocusEffect(focusFcCastTimeMod2, spell_id);

	if (level > 50 && casttime >= 3000 && !spells[spell_id].good_effect &&
	    (GetClass() == Class::Ranger || GetClass() == Class::ShadowKnight || GetClass() == Class::Paladin || GetClass() == Class::Beastlord)) {
		int level_mod = std::min(15, GetLevel() - 50);
		cast_reducer += level_mod * 3;
	}

	cast_reducer = std::min(cast_reducer, 50);  //Max cast time with focusSpellHaste and level reducer is 50% of cast time.
	cast_reducer += cast_reducer_no_limit;
	casttime = casttime * (100 - cast_reducer) / 100;
	casttime -= cast_reducer_amt;

	return std::max(casttime, 0);

}

void Mob::ExecWeaponProc(const EQ::ItemInstance* inst, uint16 spell_id, Mob* on, int level_override)
{
	// Changed proc targets to look up based on the spells goodEffect flag.
	// This should work for the majority of weapons.
	if (!on) {
		return;
	}

	if (!IsValidSpell(spell_id) || on->GetSpecialAbility(SpecialAbility::HarmFromClientImmunity)) {
		//This is so 65535 doesn't get passed to the client message and to logs because it is not relavant information for debugging.
		return;
	}

	if (IsBot() && on->GetSpecialAbility(SpecialAbility::BotDamageImmunity)) {
		return;
	}

	if (IsClient() && on->GetSpecialAbility(SpecialAbility::ClientDamageImmunity)) {
		return;
	}

	if (IsNPC() && on->GetSpecialAbility(SpecialAbility::NPCDamageImmunity)) {
		return;
	}

	if (IsNoCast()) {
		return;
	}

	if (!IsValidSpell(spell_id)) { // Check for a valid spell otherwise it will crash through the function
		if (IsClient()) {
			Message(
				Chat::White,
				fmt::format(
					"Invalid spell ID for proc {}.",
					spell_id
				).c_str()
			);
			LogSpells("Player [{}] Weapon Procced invalid spell [{}]", GetName(), spell_id);
		}

		return;
	}

	if (IsSilenced() && !IsDiscipline(spell_id)) {
		MessageString(Chat::Red, SILENCED_STRING);
		return;
	}

	if (IsAmnesiad() && IsDiscipline(spell_id)) {
		MessageString(Chat::Red, MELEE_SILENCE);
		return;
	}

	if (inst && IsClient()) {
		//const cast is dirty but it would require redoing a ton of interfaces at this point
		//It should be safe as we don't have any truly const EQ::ItemInstance floating around anywhere.
		//So we'll live with it for now
		if (parse->ItemHasQuestSub(const_cast<EQ::ItemInstance*>(inst), EVENT_WEAPON_PROC)) {
			int i = parse->EventItem(EVENT_WEAPON_PROC, CastToClient(), const_cast<EQ::ItemInstance*>(inst), on, "", spell_id);

			if (i != 0) {
				return;
			}
		}
	}

	bool  twin_proc        = false;
	int32 twin_proc_chance = 0;

	if (IsClient() || IsBot()) {
		twin_proc_chance = GetFocusEffect(focusTwincast, spell_id);
	}

	if (twin_proc_chance && zone->random.Roll(twin_proc_chance)) {
		twin_proc = true;
	}

	if (
		IsBeneficialSpell(spell_id) &&
		(
			!IsNPC() ||
			(
				IsNPC() &&
				CastToNPC()->GetInnateProcSpellID() != spell_id
			)
		) &&
		spells[spell_id].target_type != ST_TargetsTarget
	) { // NPC innate procs don't take this path ever
		SpellFinished(
			spell_id,
			this,
			EQ::spells::CastingSlot::Item,
			0,
			-1,
			spells[spell_id].resist_difficulty,
			true,
			level_override
		);

		if (twin_proc) {
			SpellFinished(
				spell_id,
				this,
				EQ::spells::CastingSlot::Item,
				0,
				-1,
				spells[spell_id].resist_difficulty,
				true,
				level_override
			);
		}
	} else if (!(on->IsClient() && on->CastToClient()->dead)) { //dont proc on dead clients
		SpellFinished(
			spell_id,
			on,
			EQ::spells::CastingSlot::Item,
			0,
			-1,
			spells[spell_id].resist_difficulty,
			true,
			level_override
		);

		if (twin_proc && (!(on->IsClient() && on->CastToClient()->dead))) {
			SpellFinished(
				spell_id,
				on,
				EQ::spells::CastingSlot::Item,
				0,
				-1,
				spells[spell_id].resist_difficulty,
				true,
				level_override
			);
		}
	}
}

uint32 Mob::GetZoneID() const {
	return zone->GetZoneID();
}

uint16 Mob::GetInstanceVersion() const {
	return zone->GetInstanceVersion();
}

int Mob::GetHaste()
{
	// See notes in Client::CalcHaste
	// Need to check if the effect of inhibit melee differs for NPCs
	if (spellbonuses.haste < 0) {
		if (-spellbonuses.haste <= spellbonuses.inhibitmelee)
			return 100 - spellbonuses.inhibitmelee;
		else
			return 100 + spellbonuses.haste;
	}

	if (spellbonuses.haste == 0 && spellbonuses.inhibitmelee)
		return 100 - spellbonuses.inhibitmelee;

	int h = 0;
	int cap = 0;
	int level = GetLevel();

	if (spellbonuses.haste)
		h += spellbonuses.haste - spellbonuses.inhibitmelee;
	if (spellbonuses.hastetype2 && level > 49)
		h += spellbonuses.hastetype2 > 10 ? 10 : spellbonuses.hastetype2;

	// 26+ no cap, 1-25 10
	if (
		level > 25 ||
		(
			(IsNPC() && RuleB(NPC, NPCIgnoreLevelBasedHasteCaps)) ||
			(IsBot() && RuleB(Bots, BotsIgnoreLevelBasedHasteCaps)) ||
			(IsMerc() && RuleB(Mercs, MercsIgnoreLevelBasedHasteCaps))
		)
	) {
		h += itembonuses.haste;
	}
	else { // 1-25
		h += itembonuses.haste > 10 ? 10 : itembonuses.haste;
	}

	// mobs are different!
	Mob *owner = nullptr;
	if (IsPet())
		owner = GetOwner();
	else if (IsNPC() && CastToNPC()->GetSwarmOwner())
		owner = entity_list.GetMobID(CastToNPC()->GetSwarmOwner());
	if (owner) {
		cap = 10 + level;
		cap += std::max(0, owner->GetLevel() - 39) + std::max(0, owner->GetLevel() - 60);
	} else {
		cap = (IsNPC() ? RuleI(NPC, NPCHasteCap) : IsBot() ? RuleI(Bots, BotsHasteCap) : IsMerc() ? RuleI(Mercs, MercsHasteCap) : 150);
	}

	if(h > cap)
		h = cap;

	// 51+ 25 (despite there being higher spells...), 1-50 10
	if (
		(IsNPC() && !RuleB(NPC, NPCIgnoreLevelBasedHasteCaps)) ||
		(IsBot() && !RuleB(Bots, BotsIgnoreLevelBasedHasteCaps)) ||
		(IsMerc() && !RuleB(Mercs, MercsIgnoreLevelBasedHasteCaps))
	) {
		if (level > 50) { // 51+
			cap = (IsNPC() ? RuleI(NPC, NPCHastev3Cap) : IsBot() ? RuleI(Bots, BotsHastev3Cap) : IsMerc() ? RuleI(Mercs, MercsHastev3Cap) : RuleI(Character, Hastev3Cap));

			if (spellbonuses.hastetype3 > cap) {
				h += cap;
			}
			else {
				h += spellbonuses.hastetype3;
			}
		}
		else { // 1-50
			h += spellbonuses.hastetype3 > 10 ? 10 : spellbonuses.hastetype3;
		}
	}
	else {
		h += spellbonuses.hastetype3;
	}

	h += extra_haste;	//GM granted haste.

	return 100 + h;
}

void Mob::SetTarget(Mob *mob)
{
	if (target == mob) {
		return;
	}

	target = mob;
	entity_list.UpdateHoTT(this);

	if (IsClient() && CastToClient()->admin > AccountStatus::GMMgmt) {
		DisplayInfo(mob);
	}

	std::vector<std::any> args = { mob };

	parse->EventMob(EVENT_TARGET_CHANGE, this, mob, [&]() { return ""; }, 0, &args);

	if (IsPet() && GetOwner() && GetOwner()->IsClient()) {
		GetOwner()->CastToClient()->UpdateXTargetType(MyPetTarget, mob);
	}

	if (IsClient() && GetTarget()) {
		GetTarget()->SendHPUpdate(true);
	}

	if (IsOfClientBot()) {
		Raid* r = GetRaid();
		if (r) {
			r->UpdateRaidXTargets();
			r->SendRaidAssistTarget();
		}
	}
}

// For when we want a Ground Z at a location we are not at yet
// Like MoveTo.
float Mob::FindDestGroundZ(glm::vec3 dest, float z_offset)
{
	float best_z = BEST_Z_INVALID;
	if (zone->zonemap != nullptr)
	{
		dest.z += z_offset;
		best_z = zone->zonemap->FindBestZ(dest, nullptr);
	}
	return best_z;
}

float Mob::FindGroundZ(float new_x, float new_y, float z_offset)
{
	float ret = BEST_Z_INVALID;
	if (zone->zonemap != nullptr)
	{
		glm::vec3 me;
		me.x = new_x;
		me.y = new_y;
		me.z = m_Position.z + z_offset;
		glm::vec3 hit;
		float best_z = zone->zonemap->FindBestZ(me, &hit);
		if (best_z != BEST_Z_INVALID)
		{
			ret = best_z;
		}
	}
	return ret;
}

// Copy of above function that isn't protected to be exported to Perl::Mob
float Mob::GetGroundZ(float new_x, float new_y, float z_offset)
{
	float ret = BEST_Z_INVALID;
	if (zone->zonemap != 0)
	{
		glm::vec3 me;
		me.x = new_x;
		me.y = new_y;
		me.z = m_Position.z+z_offset;
		glm::vec3 hit;
		float best_z = zone->zonemap->FindBestZ(me, &hit);
		if (best_z != BEST_Z_INVALID)
		{
			ret = best_z;
		}
	}
	return ret;
}

//helper function for npc AI; needs to be mob:: cause we need to be able to count buffs on other clients and npcs
int Mob::CountDispellableBuffs()
{
	int val = 0;
	int buff_count = GetMaxTotalSlots();
	for(int x = 0; x < buff_count; x++)
	{
		if(!IsValidSpell(buffs[x].spellid))
			continue;

		if(buffs[x].counters)
			continue;

		if(spells[buffs[x].spellid].good_effect == DETRIMENTAL_EFFECT)
			continue;

		if(IsValidSpell(buffs[x].spellid) && spells[buffs[x].spellid].buff_duration_formula != DF_Permanent) {
			val++;
		}
	}
	return val;
}

// Returns the % that a mob is snared (as a positive value). -1 means not snared
int Mob::GetSnaredAmount()
{
	int worst_snare = -1;

	int buff_count = GetMaxTotalSlots();
	for (int i = 0; i < buff_count; i++)
	{
		if (!IsValidSpell(buffs[i].spellid))
			continue;

		for(int j = 0; j < EFFECT_COUNT; j++)
		{
			if (spells[buffs[i].spellid].effect_id[j] == SpellEffect::MovementSpeed)
			{
				int64 val = CalcSpellEffectValue_formula(spells[buffs[i].spellid].formula[j], spells[buffs[i].spellid].base_value[j], spells[buffs[i].spellid].max_value[j], buffs[i].casterlevel, buffs[i].spellid);
				//int effect = CalcSpellEffectValue(buffs[i].spellid, spells[buffs[i].spellid].effectid[j], buffs[i].casterlevel);
				if (val < 0 && std::abs(val) > worst_snare)
					worst_snare = std::abs(val);
			}
		}
	}

	return worst_snare;
}

void Mob::TriggerDefensiveProcs(Mob *on, uint16 hand, bool FromSkillProc, int64 damage)
{
	if (!on) {
		return;
	}

	if (!FromSkillProc) {
		on->TryDefensiveProc(this, hand);
	}

	//Defensive Skill Procs
	if (damage < 0 && damage >= -4) {
		EQ::skills::SkillType skillinuse = EQ::skills::SkillBlock;
		switch (damage) {
			case (-1):
				skillinuse = EQ::skills::SkillBlock;
			break;

			case (-2):
				skillinuse = EQ::skills::SkillParry;
			break;

			case (-3):
				skillinuse = EQ::skills::SkillRiposte;
			break;

			case (-4):
				skillinuse = EQ::skills::SkillDodge;
			break;
		}

		TryCastOnSkillUse(on, skillinuse);

		if (on && on->HasSkillProcs()) {
			on->TrySkillProc(this, skillinuse, 0, false, hand, true);
		}

		if (on && on->HasSkillProcSuccess()) {
			on->TrySkillProc(this, skillinuse, 0, true, hand, true);
		}
	}
}

void Mob::SetDelta(const glm::vec4& delta) {
	m_Delta = delta;
}

bool Mob::ClearEntityVariables()
{
	if (m_EntityVariables.empty()) {
		return false;
	}

	for (const auto& e : m_EntityVariables) {
		std::vector<std::any> args = { e.first, e.second };

		parse->EventMob(EVENT_ENTITY_VARIABLE_DELETE, this, nullptr, [&]() { return ""; }, 0, &args);
	}

	m_EntityVariables.clear();
	return true;
}

bool Mob::DeleteEntityVariable(std::string variable_name)
{
	if (m_EntityVariables.empty() || variable_name.empty()) {
		return false;
	}

	auto v = m_EntityVariables.find(variable_name);
	if (v == m_EntityVariables.end()) {
		return false;
	}

	std::vector<std::any> args = { v->first, v->second };
	parse->EventMob(EVENT_ENTITY_VARIABLE_DELETE, this, nullptr, [&]() { return ""; }, 0, &args);

	m_EntityVariables.erase(v);

	return true;
}

std::string Mob::GetEntityVariable(std::string variable_name)
{
	if (m_EntityVariables.empty() || variable_name.empty()) {
		return std::string();
	}

	const auto& v = m_EntityVariables.find(variable_name);

	return v != m_EntityVariables.end() ? v->second : std::string();
}

std::vector<std::string> Mob::GetEntityVariables()
{
	std::vector<std::string> l;

	if (m_EntityVariables.empty()) {
		return l;
	}

	l.reserve(m_EntityVariables.size());

	for (const auto& v : m_EntityVariables) {
		l.emplace_back(v.first);
	}

	return l;
}

bool Mob::EntityVariableExists(std::string variable_name)
{
	if (m_EntityVariables.empty() || variable_name.empty()) {
		return false;
	}

	return m_EntityVariables.find(variable_name) != m_EntityVariables.end();
}

void Mob::SetEntityVariable(std::string variable_name, std::string variable_value)
{
	if (variable_name.empty()) {
		return;
	}

	std::vector<std::any> args;

	if (!EntityVariableExists(variable_name)) {
		args = { variable_name, variable_value };

		parse->EventMob(EVENT_ENTITY_VARIABLE_SET, this, nullptr, [&]() { return ""; }, 0, &args);
	} else {
		args = { variable_name, GetEntityVariable(variable_name), variable_value };

		parse->EventMob(EVENT_ENTITY_VARIABLE_UPDATE, this, nullptr, [&]() { return ""; }, 0, &args);
	}

	m_EntityVariables[variable_name] = variable_value;
}

void Mob::SetFlyMode(GravityBehavior in_flymode)
{
	flymode = in_flymode;
}

void Mob::Teleport(const glm::vec3 &pos)
{
	mMovementManager->Teleport(this, pos.x, pos.y, pos.z, m_Position.w);
}

void Mob::Teleport(const glm::vec4 &pos)
{
	mMovementManager->Teleport(this, pos.x, pos.y, pos.z, pos.w);
}

bool Mob::IsNimbusEffectActive(uint32 nimbus_effect)
{
	if(nimbus_effect1 == nimbus_effect || nimbus_effect2 == nimbus_effect || nimbus_effect3 == nimbus_effect)
	{
		return true;
	}
	return false;
}

void Mob::SetNimbusEffect(uint32 nimbus_effect)
{
	if(nimbus_effect1 == 0)
	{
		nimbus_effect1 = nimbus_effect;
	}
	else if(nimbus_effect2 == 0)
	{
		nimbus_effect2 = nimbus_effect;
	}
	else
	{
		nimbus_effect3 = nimbus_effect;
	}
}

bool Mob::TrySpellTrigger(Mob *target, uint32 spell_id, int effect)
{
	if (!target || !IsValidSpell(spell_id))
		return false;

	/*The effects SpellEffect::SpellTrigger (SPA 340) and SpellEffect::Chance_Best_in_Spell_Grp (SPA 469) work as follows, you typically will have 2-3 different spells each with their own
	chance to be triggered with all chances equaling up to 100 pct, with only 1 spell out of the group being ultimately cast.
	(ie Effect1 trigger spellA with 30% chance, Effect2 triggers spellB with 20% chance, Effect3 triggers spellC with 50% chance).
	The following function ensures a statistically accurate chance for each spell to be cast based on their chance values. These effects are also  used in spells where there
	is only 1 effect using the trigger effect. In those situations we simply roll a chance for that spell to be cast once.
	Note: Both SPA 340 and 469 can be in same spell and both cumulative add up to 100 pct chances. SPA469 only difference being the spell cast will
	be "best in spell group", instead of a defined spell_id.*/

	int chance_array[EFFECT_COUNT] = {};
	int total_chance = 0;
	int effect_slot = effect;
	bool CastSpell = false;

	for (int i = 0; i < EFFECT_COUNT; i++)
	{
		if (spells[spell_id].effect_id[i] == SpellEffect::SpellTrigger || spells[spell_id].effect_id[i] == SpellEffect::Chance_Best_in_Spell_Grp)
			total_chance += spells[spell_id].base_value[i];
	}

	if (total_chance == 100)
	{
		int current_chance = 0;

		for (int i = 0; i < EFFECT_COUNT; i++){
			//Find spells with SPA 340 and add the cumulative percent chances to the roll array
			if ((spells[spell_id].effect_id[i] == SpellEffect::SpellTrigger) || (spells[spell_id].effect_id[i] == SpellEffect::Chance_Best_in_Spell_Grp)){
				const int cumulative_chance = current_chance + spells[spell_id].base_value[i];
				chance_array[i] = cumulative_chance;
				current_chance = cumulative_chance;
			}
		}
		int random_roll = zone->random.Int(1, 100);
		//Determine which spell out of the group of the spells (each with own percent chance out of 100) will be cast based on a single roll.
		for (int i = 0; i < EFFECT_COUNT; i++){
			if (chance_array[i] != 0 && random_roll <= chance_array[i]) {
				effect_slot = i;
				CastSpell = true;
				break;
			}
		}
	}

	//If the chances don't add to 100, then each effect gets a chance to fire, chance for no trigger as well.
	else if (zone->random.Roll(spells[spell_id].base_value[effect])) {
			CastSpell = true; //In this case effect_slot is what was passed into function.
	}

	if (CastSpell) {
		if (spells[spell_id].effect_id[effect_slot] == SpellEffect::SpellTrigger && IsValidSpell(spells[spell_id].limit_value[effect_slot])) {
			SpellFinished(spells[spell_id].limit_value[effect_slot], target, EQ::spells::CastingSlot::Item, 0, -1, spells[spells[spell_id].limit_value[effect_slot]].resist_difficulty);
			return true;
		}
		else if (IsClient() && spells[spell_id].effect_id[effect_slot] == SpellEffect::Chance_Best_in_Spell_Grp) {
			uint32 best_spell_id = CastToClient()->GetHighestScribedSpellinSpellGroup(spells[spell_id].limit_value[effect_slot]);
			if (IsValidSpell(best_spell_id)) {
				SpellFinished(best_spell_id, target, EQ::spells::CastingSlot::Item, 0, -1, spells[best_spell_id].resist_difficulty);
			}
			return true;//Do nothing if you don't have the any spell in spell group scribed.
		}
	}

	return false;
}

void Mob::TryTriggerOnCastRequirement()
{
	if (spellbonuses.TriggerOnCastRequirement) {
		int buff_count = GetMaxTotalSlots();
		for (int e = 0; e < buff_count; e++) {
			int spell_id = buffs[e].spellid;
			if (IsValidSpell(spell_id)) {
				for (int i = 0; i < EFFECT_COUNT; i++) {
					if ((spells[spell_id].effect_id[i] == SpellEffect::TriggerOnReqTarget) || (spells[spell_id].effect_id[i] == SpellEffect::TriggerOnReqCaster)) {
						if (PassCastRestriction(spells[spell_id].limit_value[i])) {
							SpellFinished(spells[spell_id].base_value[i], this, EQ::spells::CastingSlot::Item, 0, -1, spells[spell_id].resist_difficulty);
							if (!TryFadeEffect(e)) {
								BuffFadeBySlot(e);
							}
						}
					}
				}
			}
		}
	}
}

//Twincast Focus effects should stack across different types (Spell, AA - when implemented ect)
void Mob::TryTwincast(Mob *caster, Mob *target, uint32 spell_id)
{
	if (!IsValidSpell(spell_id)) {
		return;
	}

	if (IsOfClientBot())
	{
		int focus = GetFocusEffect(focusTwincast, spell_id);

		if (focus > 0)
		{
			if (zone->random.Roll(focus))
			{
				if (IsClient()) {
					Message(Chat::Spells,"You twincast %s!", spells[spell_id].name);
				}
				SpellFinished(spell_id, target, EQ::spells::CastingSlot::Item, 0, -1, spells[spell_id].resist_difficulty);
			}
		}
	}

	//Retains function for non clients
	else if (spellbonuses.FocusEffects[focusTwincast] || itembonuses.FocusEffects[focusTwincast])
	{
		int buff_count = GetMaxTotalSlots();
		for(int i = 0; i < buff_count; i++)
		{
			if(IsEffectInSpell(buffs[i].spellid, SpellEffect::FcTwincast))
			{
				int32 focus = CalcFocusEffect(focusTwincast, buffs[i].spellid, spell_id);
				if(focus > 0)
				{
					if(zone->random.Roll(focus))
					{
						SpellFinished(spell_id, target, EQ::spells::CastingSlot::Item, 0, -1, spells[spell_id].resist_difficulty);
					}
				}
			}
		}
	}
}

//Used for effects that should occur after the completion of the spell
void Mob::ApplyHealthTransferDamage(Mob *caster, Mob *target, uint16 spell_id)
{
	if (!IsValidSpell(spell_id))
		return;

	/*
		Apply damage from Lifeburn type effects on caster at end of spell cast.
		This allows for the AE spells to function without repeatedly killing caster
		Damage or heal portion can be found as regular single use spell effect
	*/
	if (IsEffectInSpell(spell_id, SpellEffect::Health_Transfer)){
		for (int i = 0; i < EFFECT_COUNT; i++) {

			if (spells[spell_id].effect_id[i] == SpellEffect::Health_Transfer) {
				int64 new_hp = GetMaxHP();
				new_hp -= GetMaxHP()  * spells[spell_id].base_value[i] / 1000;

				if (new_hp > 0) {
					SetHP(new_hp);
				}
				else {
					Kill();
				}
			}
		}
	}
}

int32 Mob::GetVulnerability(Mob *caster, uint32 spell_id, uint32 ticsremaining, bool from_buff_tic)
{
	/*
	Modifies incoming spell damage by percent, to increase or decrease damage, can be limited to specific resists.
	Can be applied through quest function, spell focus or npc_spells_effects table. This function is run on the target of the spell.
	*/

	if (!IsValidSpell(spell_id))
		return 0;

	if (!caster)
		return 0;

	int32 total_mod = 0;
	int32 innate_mod = 0;
	int32 fc_spell_vulnerability_mod = 0;
	int32 fc_spell_damage_pct_incomingPC_mod = 0;

	//Apply innate vulnerabilities from quest functions and tables
	if (Vulnerability_Mod[GetSpellResistType(spell_id)] != 0) {
		innate_mod = Vulnerability_Mod[GetSpellResistType(spell_id)];
	}
	else if (Vulnerability_Mod[HIGHEST_RESIST + 1] != 0) {
		innate_mod = Vulnerability_Mod[HIGHEST_RESIST + 1];
	}

	fc_spell_vulnerability_mod = GetFocusEffect(focusSpellVulnerability, spell_id, caster, from_buff_tic);
	fc_spell_damage_pct_incomingPC_mod = GetFocusEffect(focusFcSpellDamagePctIncomingPC, spell_id, caster, from_buff_tic);

	total_mod = fc_spell_vulnerability_mod + fc_spell_damage_pct_incomingPC_mod;

	//Don't let focus derived mods reduce past 99% mitigation. Quest related can, and for custom functionality if negative will give a healing affect instead of damage.
	if (total_mod < -99) {
		total_mod = -99;
	}

	total_mod += innate_mod;
	return total_mod;
}

bool Mob::IsTargetedFocusEffect(int focus_type) {

	switch (focus_type) {
		case focusSpellVulnerability:
		case focusFcSpellDamagePctIncomingPC:
		case focusFcDamageAmtIncoming:
		case focusFcSpellDamageAmtIncomingPC:
		case focusFcCastSpellOnLand:
		case focusFcHealAmtIncoming:
		case focusFcHealPctCritIncoming:
		case focusFcHealPctIncoming:
			return true;
		default:
			return false;
	}
}

int32 Mob::GetSkillDmgTaken(const EQ::skills::SkillType skill_used, ExtraAttackOptions *opts)
{
	int skilldmg_mod = 0;

	// All skill dmg mod + Skill specific
	skilldmg_mod += itembonuses.SkillDmgTaken[EQ::skills::HIGHEST_SKILL + 1] + spellbonuses.SkillDmgTaken[EQ::skills::HIGHEST_SKILL + 1] +
					itembonuses.SkillDmgTaken[skill_used] + spellbonuses.SkillDmgTaken[skill_used];

	skilldmg_mod += SkillDmgTaken_Mod[skill_used] + SkillDmgTaken_Mod[EQ::skills::HIGHEST_SKILL + 1];

	if (opts)
		skilldmg_mod += opts->skilldmgtaken_bonus_flat;

	if(skilldmg_mod < -100)
		skilldmg_mod = -100;

	return skilldmg_mod;
}

int32 Mob::GetPositionalDmgTaken(Mob *attacker)
{
	if (!attacker)
		return 0;

	int front_arc = 0;
	int back_arc = 0;
	int total_mod = 0;

	back_arc += itembonuses.Damage_Taken_Position_Mod[SBIndex::POSITION_BACK] + aabonuses.Damage_Taken_Position_Mod[SBIndex::POSITION_BACK] + spellbonuses.Damage_Taken_Position_Mod[SBIndex::POSITION_BACK];
	front_arc += itembonuses.Damage_Taken_Position_Mod[SBIndex::POSITION_FRONT] + aabonuses.Damage_Taken_Position_Mod[SBIndex::POSITION_FRONT] + spellbonuses.Damage_Taken_Position_Mod[SBIndex::POSITION_FRONT];

	if (back_arc || front_arc) { //Do they have this bonus?
		if (attacker->BehindMob(this, attacker->GetX(), attacker->GetY()))//Check if attacker is striking from behind
			total_mod = back_arc; //If so, apply the back arc modifier only
		else
			total_mod = front_arc;//If not, apply the front arc modifer only
	}

	total_mod = round(static_cast<double>(total_mod) * 0.1);

	if (total_mod < -100)
		total_mod = -100;

	return total_mod;
}

int32 Mob::GetPositionalDmgTakenAmt(Mob *attacker)
{
	if (!attacker)
		return 0;

	int front_arc = 0;
	int back_arc = 0;
	int total_amt = 0;

	back_arc += itembonuses.Damage_Taken_Position_Amt[SBIndex::POSITION_BACK] + aabonuses.Damage_Taken_Position_Amt[SBIndex::POSITION_BACK] + spellbonuses.Damage_Taken_Position_Amt[SBIndex::POSITION_BACK];
	front_arc += itembonuses.Damage_Taken_Position_Amt[SBIndex::POSITION_FRONT] + aabonuses.Damage_Taken_Position_Amt[SBIndex::POSITION_FRONT] + spellbonuses.Damage_Taken_Position_Amt[SBIndex::POSITION_FRONT];

	if (back_arc || front_arc) {
		if (attacker->BehindMob(this, attacker->GetX(), attacker->GetY()))
			total_amt = back_arc;
		else
			total_amt = front_arc;
	}

	return total_amt;
}

void Mob::SetBottomRampageList()
{
	for (auto &e : GetCloseMobList()) {
		auto mob = e.second;
		if (!mob) {
			continue;
		}

		if (!mob->GetSpecialAbility(SpecialAbility::Rampage)) {
			continue;
		}

		if (mob->IsNPC() && mob->CheckAggro(this)) {
			for (int i = 0; i < mob->RampageArray.size(); i++) {
				// Find this mob in the rampage list
				if (GetID() == mob->RampageArray[i]) {
					//Move to bottom of Rampage List
					auto it = mob->RampageArray.begin() + i;
					std::rotate(it, it + 1, mob->RampageArray.end());
				}
			}
		}
	}
}

void Mob::SetTopRampageList()
{
	for (auto &e : GetCloseMobList()) {
		auto mob = e.second;
		if (!mob) {
			continue;
		}

		if (!mob->GetSpecialAbility(SpecialAbility::Rampage)) {
			continue;
		}

		if (mob->IsNPC() && mob->CheckAggro(this)) {
			for (int i = 0; i < mob->RampageArray.size(); i++) {
				// Find this mob in the rampage list
				if (GetID() == mob->RampageArray[i]) {
					//Move to Top of Rampage List
					auto it = mob->RampageArray.begin() + i;
					std::rotate(it, it + 1, mob->RampageArray.end());
					std::rotate(mob->RampageArray.rbegin(), mob->RampageArray.rbegin() + 1, mob->RampageArray.rend());
				}
			}
		}
	}
}

bool Mob::TryFadeEffect(int slot)
{
	if (!buffs[slot].spellid)
		return false;

	if(IsValidSpell(buffs[slot].spellid))
	{
		for(int i = 0; i < EFFECT_COUNT; i++)
		{

			if (!spells[buffs[slot].spellid].effect_id[i])
				continue;

			if (spells[buffs[slot].spellid].effect_id[i] == SpellEffect::CastOnFadeEffectAlways ||
				spells[buffs[slot].spellid].effect_id[i] == SpellEffect::CastOnRuneFadeEffect)
			{
				uint16 spell_id = spells[buffs[slot].spellid].base_value[i];
				BuffFadeBySlot(slot);

				if(spell_id)
				{

					if (!IsValidSpell(spell_id)) {
						return false;
					}
					if(IsValidSpell(spell_id))
					{
						if (IsBeneficialSpell(spell_id)) {
							SpellFinished(spell_id, this, EQ::spells::CastingSlot::Item, 0, -1, spells[spell_id].resist_difficulty);
						}
						else if(!(IsClient() && CastToClient()->dead)) {
							SpellFinished(spell_id, this, EQ::spells::CastingSlot::Item, 0, -1, spells[spell_id].resist_difficulty);
						}
						return true;
					}
				}
			}
		}
	}
	return false;
}

void Mob::TrySympatheticProc(Mob* target, uint32 spell_id)
{
	if (!target || !IsValidSpell(spell_id) || !IsOfClientBotMerc()) {
		return;
	}

	const uint16 focus_spell = GetSympatheticFocusEffect(focusSympatheticProc, spell_id);

	if (!IsValidSpell(focus_spell)) {
		return;
	}

	const uint16 focus_trigger = GetSympatheticSpellProcID(focus_spell);

	if (!IsValidSpell(focus_trigger)) {
		return;
	}

	if (IsBeneficialSpell(spell_id)) {
		// For beneficial spells, if the triggered spell is also beneficial then proc it on the target
		// if the triggered spell is detrimental, then it will trigger on the caster(ie cursed items)
		if (IsBeneficialSpell(focus_trigger)) {
			SpellFinished(focus_trigger, target);
		} else {
			SpellFinished(
				focus_trigger,
				this,
				EQ::spells::CastingSlot::Item,
				0,
				-1,
				spells[focus_trigger].resist_difficulty
			);
		}
	} else {
		// For detrimental spells, if the triggered spell is beneficial, then it will land on the caster
		// if the triggered spell is also detrimental, then it will land on the target
		if (IsBeneficialSpell(focus_trigger)) {
			SpellFinished(focus_trigger, this);
		} else {
			SpellFinished(
				focus_trigger,
				target,
				EQ::spells::CastingSlot::Item,
				0,
				-1,
				spells[focus_trigger].resist_difficulty
			);
		}
	}

	CheckNumHitsRemaining(NumHit::MatchingSpells, -1, focus_spell);
}

const int Mob::GetItemStat(uint32 item_id, std::string identifier)
{
	return EQ::InventoryProfile::GetItemStatValue(item_id, identifier);
}

std::string Mob::GetGlobal(const char *varname) {
	int qgCharid = 0;
	int qgNpcid = 0;

	if (IsNPC())
		qgNpcid = GetNPCTypeID();

	if (IsClient())
		qgCharid = CastToClient()->CharacterID();

	QGlobalCache *qglobals = nullptr;
	std::list<QGlobal> globalMap;

	if (IsClient())
		qglobals = CastToClient()->GetQGlobals();

	if (IsNPC())
		qglobals = CastToNPC()->GetQGlobals();

	if(qglobals)
		QGlobalCache::Combine(globalMap, qglobals->GetBucket(), qgNpcid, qgCharid, zone->GetZoneID());

	auto iter = globalMap.begin();
	while(iter != globalMap.end()) {
		if ((*iter).name.compare(varname) == 0)
			return (*iter).value;

		++iter;
	}

	return "Undefined";
}

void Mob::SetGlobal(const char *varname, const char *newvalue, int options, const char *duration, Mob *other) {

	int qgZoneid = zone->GetZoneID();
	int qgCharid = 0;
	int qgNpcid = 0;

	if (IsNPC())
	{
		qgNpcid = GetNPCTypeID();
	}
	else if (other && other->IsNPC())
	{
		qgNpcid = other->GetNPCTypeID();
	}

	if (IsClient())
	{
		qgCharid = CastToClient()->CharacterID();
	}
	else if (other && other->IsClient())
	{
		qgCharid = other->CastToClient()->CharacterID();
	}
	else
	{
		qgCharid = -qgNpcid;		// make char id negative npc id as a fudge
	}

	if (options < 0 || options > 7)
	{
		//cerr << "Invalid options for global var " << varname << " using defaults" << endl;
		options = 0;	// default = 0 (only this npcid,player and zone)
	}
	else
	{
		if (options & 1)
			qgNpcid=0;
		if (options & 2)
			qgCharid=0;
		if (options & 4)
			qgZoneid=0;
	}

	InsertQuestGlobal(qgCharid, qgNpcid, qgZoneid, varname, newvalue, QGVarDuration(duration));
}

void Mob::TarGlobal(const char *varname, const char *value, const char *duration, int qgNpcid, int qgCharid, int qgZoneid)
{
	InsertQuestGlobal(qgCharid, qgNpcid, qgZoneid, varname, value, QGVarDuration(duration));
}

void Mob::DelGlobal(const char *varname) {

	if (!zone) {
		return;
	}

	int qgZoneid=zone->GetZoneID();
	int qgCharid=0;
	int qgNpcid=0;

	if (IsNPC())
		qgNpcid = GetNPCTypeID();

	if (IsClient())
		qgCharid = CastToClient()->CharacterID();
	else
		qgCharid = -qgNpcid;		// make char id negative npc id as a fudge

    std::string query = StringFormat("DELETE FROM quest_globals "
                                    "WHERE name='%s' && (npcid=0 || npcid=%i) "
                                    "&& (charid=0 || charid=%i) "
                                    "&& (zoneid=%i || zoneid=0)",
                                    varname, qgNpcid, qgCharid, qgZoneid);

	database.QueryDatabase(query);

	auto pack = new ServerPacket(ServerOP_QGlobalDelete, sizeof(ServerQGlobalDelete_Struct));
	ServerQGlobalDelete_Struct *qgu = (ServerQGlobalDelete_Struct*)pack->pBuffer;

	qgu->npc_id = qgNpcid;
	qgu->char_id = qgCharid;
	qgu->zone_id = qgZoneid;
	strcpy(qgu->name, varname);

	entity_list.DeleteQGlobal(std::string((char*)qgu->name), qgu->npc_id, qgu->char_id, qgu->zone_id);
	zone->DeleteQGlobal(std::string((char*)qgu->name), qgu->npc_id, qgu->char_id, qgu->zone_id);

	worldserver.SendPacket(pack);
	safe_delete(pack);
}

// Inserts global variable into quest_globals table
void Mob::InsertQuestGlobal(int charid, int npcid, int zoneid, const char *varname, const char *varvalue, int duration) {

	// Make duration string either "unix_timestamp(now()) + xxx" or "NULL"
	std::stringstream duration_ss;

	if (duration == INT_MAX)
		duration_ss << "NULL";
	else
		duration_ss << "unix_timestamp(now()) + " << duration;

	//NOTE: this should be escaping the contents of arglist
	//npcwise a malicious script can arbitrarily alter the DB
	uint32 last_id = 0;
	std::string query = StringFormat("REPLACE INTO quest_globals "
                                    "(charid, npcid, zoneid, name, value, expdate)"
                                    "VALUES (%i, %i, %i, '%s', '%s', %s)",
                                    charid, npcid, zoneid, varname, varvalue, duration_ss.str().c_str());
	database.QueryDatabase(query);

	if(zone)
	{
		//first delete our global
		auto pack = new ServerPacket(ServerOP_QGlobalDelete, sizeof(ServerQGlobalDelete_Struct));
		ServerQGlobalDelete_Struct *qgd = (ServerQGlobalDelete_Struct*)pack->pBuffer;
		qgd->npc_id = npcid;
		qgd->char_id = charid;
		qgd->zone_id = zoneid;
		qgd->from_zone_id = zone->GetZoneID();
		qgd->from_instance_id = zone->GetInstanceID();
		strcpy(qgd->name, varname);

		entity_list.DeleteQGlobal(std::string((char*)qgd->name), qgd->npc_id, qgd->char_id, qgd->zone_id);
		zone->DeleteQGlobal(std::string((char*)qgd->name), qgd->npc_id, qgd->char_id, qgd->zone_id);

		worldserver.SendPacket(pack);
		safe_delete(pack);

		//then create a new one with the new id
		pack = new ServerPacket(ServerOP_QGlobalUpdate, sizeof(ServerQGlobalUpdate_Struct));
		ServerQGlobalUpdate_Struct *qgu = (ServerQGlobalUpdate_Struct*)pack->pBuffer;
		qgu->npc_id = npcid;
		qgu->char_id = charid;
		qgu->zone_id = zoneid;

		if(duration == INT_MAX)
			qgu->expdate = 0xFFFFFFFF;
		else
			qgu->expdate = Timer::GetTimeSeconds() + duration;

		strcpy((char*)qgu->name, varname);
		strcpy((char*)qgu->value, varvalue);
		qgu->id = last_id;
		qgu->from_zone_id = zone->GetZoneID();
		qgu->from_instance_id = zone->GetInstanceID();

		QGlobal temp;
		temp.npc_id = npcid;
		temp.char_id = charid;
		temp.zone_id = zoneid;
		temp.expdate = qgu->expdate;
		temp.name.assign(qgu->name);
		temp.value.assign(qgu->value);
		entity_list.UpdateQGlobal(qgu->id, temp);
		zone->UpdateQGlobal(qgu->id, temp);

		worldserver.SendPacket(pack);
		safe_delete(pack);
	}

}

// Converts duration string to duration value (in seconds)
// Return of INT_MAX indicates infinite duration
int Mob::QGVarDuration(const char *fmt)
{
	int duration = 0;

	// format:	Y#### or D## or H## or M## or S## or T###### or C#######

	int len = static_cast<int>(strlen(fmt));

	// Default to no duration
	if (len < 1)
		return 0;

	// Set val to value after type character
	// e.g., for "M3924", set to 3924
	int val = Strings::ToInt(&fmt[0] + 1);

	switch (fmt[0])
	{
		// Forever
		case 'F':
		case 'f':
			duration = INT_MAX;
			break;
		// Years
		case 'Y':
		case 'y':
			duration = val * 31556926;
			break;
		case 'D':
		case 'd':
			duration = val * 86400;
			break;
		// Hours
		case 'H':
		case 'h':
			duration = val * 3600;
			break;
		// Minutes
		case 'M':
		case 'm':
			duration = val * 60;
			break;
		// Seconds
		case 'S':
		case 's':
			duration = val;
			break;
		// Invalid
		default:
			duration = 0;
			break;
	}

	return duration;
}

void Mob::DoKnockback(Mob *caster, uint32 push_back, uint32 push_up)
{
	if(IsClient())
	{
		CastToClient()->cheat_manager.SetExemptStatus(KnockBack, true);
		auto outapp_push = new EQApplicationPacket(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
		PlayerPositionUpdateServer_Struct* spu = (PlayerPositionUpdateServer_Struct*)outapp_push->pBuffer;

		double look_heading = caster->CalculateHeadingToTarget(GetX(), GetY());
		look_heading /= 256;
		look_heading *= 360;
		if(look_heading > 360)
			look_heading -= 360;

		//x and y are crossed mkay
		double new_x = push_back * sin(double(look_heading * 3.141592 / 180.0));
		double new_y = push_back * cos(double(look_heading * 3.141592 / 180.0));

		spu->spawn_id	= GetID();
		spu->x_pos		= FloatToEQ19(GetX());
		spu->y_pos		= FloatToEQ19(GetY());
		spu->z_pos		= FloatToEQ19(GetZ());
		spu->delta_x	= FloatToEQ13(static_cast<float>(new_x));
		spu->delta_y	= FloatToEQ13(static_cast<float>(new_y));
		spu->delta_z	= FloatToEQ13(static_cast<float>(push_up));
		spu->heading	= FloatToEQ12(GetHeading());
		// for ref: these were not passed on to other 5 clients while on Titanium standard (change to RoF2 standard: 11/16/2019)
		//eq->padding0002 = 0;
		//eq->padding0006 = 0x7;
		//eq->padding0014 = 0x7F;
		//eq->padding0018 = 0x5dF27;
		spu->animation = 0;
		spu->delta_heading = FloatToEQ10(0);
		outapp_push->priority = 6;
		entity_list.QueueClients(this, outapp_push, true);
		CastToClient()->FastQueuePacket(&outapp_push);
	}
}

void Mob::TrySpellOnKill(uint8 level, uint16 spell_id)
{
	if (IsValidSpell(spell_id))
	{
		if(IsEffectInSpell(spell_id, SpellEffect::ProcOnSpellKillShot)) {
			for (int i = 0; i < EFFECT_COUNT; i++) {
				if (spells[spell_id].effect_id[i] == SpellEffect::ProcOnSpellKillShot)
				{
					if (IsValidSpell(spells[spell_id].limit_value[i]) && spells[spell_id].max_value[i] <= level)
					{
						if(zone->random.Roll(spells[spell_id].base_value[i]))
							SpellFinished(spells[spell_id].limit_value[i], this, EQ::spells::CastingSlot::Item, 0, -1, spells[spells[spell_id].limit_value[i]].resist_difficulty);
					}
				}
			}
		}
	}

	if (!aabonuses.SpellOnKill[0] && !itembonuses.SpellOnKill[0] && !spellbonuses.SpellOnKill[0])
		return;

	// Allow to check AA, items and buffs in all cases. Base2 = Spell to fire | Base1 = % chance | Base3 = min level
	for(int i = 0; i < MAX_SPELL_TRIGGER*3; i+=3) {

		if(aabonuses.SpellOnKill[i] && IsValidSpell(aabonuses.SpellOnKill[i]) && (level >= aabonuses.SpellOnKill[i + 2])) {
			if(zone->random.Roll(static_cast<int>(aabonuses.SpellOnKill[i + 1])))
				SpellFinished(aabonuses.SpellOnKill[i], this, EQ::spells::CastingSlot::Item, 0, -1, spells[aabonuses.SpellOnKill[i]].resist_difficulty);
		}

		if(itembonuses.SpellOnKill[i] && IsValidSpell(itembonuses.SpellOnKill[i]) && (level >= itembonuses.SpellOnKill[i + 2])){
			if(zone->random.Roll(static_cast<int>(itembonuses.SpellOnKill[i + 1])))
				SpellFinished(itembonuses.SpellOnKill[i], this, EQ::spells::CastingSlot::Item, 0, -1, spells[aabonuses.SpellOnKill[i]].resist_difficulty);
		}

		if(spellbonuses.SpellOnKill[i] && IsValidSpell(spellbonuses.SpellOnKill[i]) && (level >= spellbonuses.SpellOnKill[i + 2])) {
			if(zone->random.Roll(static_cast<int>(spellbonuses.SpellOnKill[i + 1])))
				SpellFinished(spellbonuses.SpellOnKill[i], this, EQ::spells::CastingSlot::Item, 0, -1, spells[aabonuses.SpellOnKill[i]].resist_difficulty);
		}

	}
}

bool Mob::TrySpellOnDeath()
{
	if (IsNPC() && !spellbonuses.SpellOnDeath[0] && !itembonuses.SpellOnDeath[0])
		return false;

	if (IsClient() && !aabonuses.SpellOnDeath[0] && !spellbonuses.SpellOnDeath[0] && !itembonuses.SpellOnDeath[0])
		return false;

	for(int i = 0; i < MAX_SPELL_TRIGGER*2; i+=2) {
		if(IsClient() && aabonuses.SpellOnDeath[i] && IsValidSpell(aabonuses.SpellOnDeath[i])) {
			if(zone->random.Roll(static_cast<int>(aabonuses.SpellOnDeath[i + 1]))) {
				SpellFinished(aabonuses.SpellOnDeath[i], this, EQ::spells::CastingSlot::Item, 0, -1, spells[aabonuses.SpellOnDeath[i]].resist_difficulty);
			}
		}

		if(itembonuses.SpellOnDeath[i] && IsValidSpell(itembonuses.SpellOnDeath[i])) {
			if(zone->random.Roll(static_cast<int>(itembonuses.SpellOnDeath[i + 1]))) {
				SpellFinished(itembonuses.SpellOnDeath[i], this, EQ::spells::CastingSlot::Item, 0, -1, spells[itembonuses.SpellOnDeath[i]].resist_difficulty);
			}
		}

		if(spellbonuses.SpellOnDeath[i] && IsValidSpell(spellbonuses.SpellOnDeath[i])) {
			if(zone->random.Roll(static_cast<int>(spellbonuses.SpellOnDeath[i + 1]))) {
				SpellFinished(spellbonuses.SpellOnDeath[i], this, EQ::spells::CastingSlot::Item, 0, -1, spells[spellbonuses.SpellOnDeath[i]].resist_difficulty);
			}
		}
	}

	if (RuleB(Spells, BuffsFadeOnDeath)) {
		BuffFadeNonPersistDeath();
	}

	return false;
	//You should not be able to use this effect and survive (ALWAYS return false),
	//attempting to place a heal in these effects will still result
	//in death because the heal will not register before the script kills you.
}

int16 Mob::GetCritDmgMod(uint16 skill, Mob* owner)
{
	int critDmg_mod = 0;

	// All skill dmg mod + Skill specific [SPA 330 and 496]
	critDmg_mod += itembonuses.CritDmgMod[EQ::skills::HIGHEST_SKILL + 1] + spellbonuses.CritDmgMod[EQ::skills::HIGHEST_SKILL + 1] + aabonuses.CritDmgMod[EQ::skills::HIGHEST_SKILL + 1] +
					itembonuses.CritDmgMod[skill] + spellbonuses.CritDmgMod[skill] + aabonuses.CritDmgMod[skill];

	critDmg_mod += itembonuses.CritDmgModNoStack[EQ::skills::HIGHEST_SKILL + 1] + spellbonuses.CritDmgModNoStack[EQ::skills::HIGHEST_SKILL + 1] + aabonuses.CritDmgModNoStack[EQ::skills::HIGHEST_SKILL + 1] +
				   itembonuses.CritDmgModNoStack[skill] + spellbonuses.CritDmgModNoStack[skill] + aabonuses.CritDmgModNoStack[skill];

	if (owner) //Checked in TryPetCriticalHit
		critDmg_mod += owner->aabonuses.Pet_Crit_Melee_Damage_Pct_Owner + owner->itembonuses.Pet_Crit_Melee_Damage_Pct_Owner + owner->spellbonuses.Pet_Crit_Melee_Damage_Pct_Owner;

	return critDmg_mod;
}

void Mob::SetGrouped(bool v)
{
	if (v) {
		israidgrouped = false;
	}

	isgrouped = v;

	if (IsClient()) {
		if (parse->PlayerHasQuestSub(EVENT_GROUP_CHANGE)) {
			parse->EventPlayer(EVENT_GROUP_CHANGE, CastToClient(), "", 0);
		}

		if (!v) {
			CastToClient()->RemoveGroupXTargets();
		}
	}
}

void Mob::SetRaidGrouped(bool v)
{
	if (v) {
		isgrouped = false;
	}

	israidgrouped = v;

	if (IsClient()) {
		if (parse->PlayerHasQuestSub(EVENT_GROUP_CHANGE)) {
			parse->EventPlayer(EVENT_GROUP_CHANGE, CastToClient(), "", 0);
		}
	}
}

int Mob::GetCriticalChanceBonus(uint16 skill)
{
	int critical_chance = 0;

	// All skills + Skill specific
	critical_chance += itembonuses.CriticalHitChance[EQ::skills::HIGHEST_SKILL + 1] + spellbonuses.CriticalHitChance[EQ::skills::HIGHEST_SKILL + 1] + aabonuses.CriticalHitChance[EQ::skills::HIGHEST_SKILL + 1] +
						itembonuses.CriticalHitChance[skill] + spellbonuses.CriticalHitChance[skill] + aabonuses.CriticalHitChance[skill];

	if(critical_chance < -100)
		critical_chance = -100;

	return critical_chance;
}

int16 Mob::GetMeleeDamageMod_SE(uint16 skill)
{
	int64 dmg_mod = 0;

	// All skill dmg mod + Skill specific
	dmg_mod += itembonuses.DamageModifier[EQ::skills::HIGHEST_SKILL + 1] + spellbonuses.DamageModifier[EQ::skills::HIGHEST_SKILL + 1] + aabonuses.DamageModifier[EQ::skills::HIGHEST_SKILL + 1] +
				itembonuses.DamageModifier[skill] + spellbonuses.DamageModifier[skill] + aabonuses.DamageModifier[skill];

	dmg_mod += itembonuses.DamageModifier2[EQ::skills::HIGHEST_SKILL + 1] + spellbonuses.DamageModifier2[EQ::skills::HIGHEST_SKILL + 1] + aabonuses.DamageModifier2[EQ::skills::HIGHEST_SKILL + 1] +
				itembonuses.DamageModifier2[skill] + spellbonuses.DamageModifier2[skill] + aabonuses.DamageModifier2[skill];

	dmg_mod += itembonuses.DamageModifier3[EQ::skills::HIGHEST_SKILL + 1] + spellbonuses.DamageModifier3[EQ::skills::HIGHEST_SKILL + 1] + aabonuses.DamageModifier3[EQ::skills::HIGHEST_SKILL + 1] +
		itembonuses.DamageModifier3[skill] + spellbonuses.DamageModifier3[skill] + aabonuses.DamageModifier3[skill];

	if (GetUseDoubleMeleeRoundDmgBonus()) {
		dmg_mod += itembonuses.DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_DMG_BONUS] + spellbonuses.DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_DMG_BONUS] + aabonuses.DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_DMG_BONUS];
	}

	if(dmg_mod < -100)
		dmg_mod = -100;

	return dmg_mod;
}

int16 Mob::GetMeleeMinDamageMod_SE(uint16 skill)
{
	int64 dmg_mod = 0;

	dmg_mod = itembonuses.MinDamageModifier[skill] + spellbonuses.MinDamageModifier[skill] +
		itembonuses.MinDamageModifier[EQ::skills::HIGHEST_SKILL + 1] + spellbonuses.MinDamageModifier[EQ::skills::HIGHEST_SKILL + 1];

	if(dmg_mod < -100)
		dmg_mod = -100;

	return dmg_mod;
}

int16 Mob::GetCrippBlowChance()
{
	int16 crip_chance = 0;

	crip_chance += itembonuses.CrippBlowChance + spellbonuses.CrippBlowChance + aabonuses.CrippBlowChance;

	if(crip_chance < 0)
		crip_chance = 0;

	return crip_chance;
}


int16 Mob::GetMeleeDmgPositionMod(Mob* defender)
{
	if (!defender)
		return 0;

	int front_arc = 0;
	int back_arc = 0;
	int total_mod = 0;

	back_arc += itembonuses.Melee_Damage_Position_Mod[SBIndex::POSITION_BACK] + aabonuses.Melee_Damage_Position_Mod[SBIndex::POSITION_BACK] + spellbonuses.Melee_Damage_Position_Mod[SBIndex::POSITION_BACK];
	front_arc += itembonuses.Melee_Damage_Position_Mod[SBIndex::POSITION_FRONT] + aabonuses.Melee_Damage_Position_Mod[SBIndex::POSITION_FRONT] + spellbonuses.Melee_Damage_Position_Mod[SBIndex::POSITION_FRONT];

	if (back_arc || front_arc) { //Do they have this bonus?
		if (BehindMob(defender, GetX(), GetY()))//Check if attacker is striking from behind
			total_mod = back_arc; //If so, apply the back arc modifier only
		else
			total_mod = front_arc;//If not, apply the front arc modifer only
	}

	total_mod = round(static_cast<double>(total_mod) * 0.1);

	if (total_mod < -100)
		total_mod = -100;

	return total_mod;

}

int16 Mob::GetSkillReuseTime(uint16 skill)
{
	int skill_reduction = itembonuses.SkillReuseTime[skill] + spellbonuses.SkillReuseTime[skill] + aabonuses.SkillReuseTime[skill];

	return skill_reduction;
}

int Mob::GetSkillDmgAmt(int skill_id)
{
	int skill_dmg = 0;

	if (!EQ::ValueWithin(skill_id, ALL_SKILLS, EQ::skills::HIGHEST_SKILL)) {
		return skill_dmg;
	}

	skill_dmg += (
		spellbonuses.SkillDamageAmount[EQ::skills::HIGHEST_SKILL + 1] +
		itembonuses.SkillDamageAmount[EQ::skills::HIGHEST_SKILL + 1] +
		aabonuses.SkillDamageAmount[EQ::skills::HIGHEST_SKILL + 1]
	);

	if (skill_id != ALL_SKILLS) {
		skill_dmg += (
			itembonuses.SkillDamageAmount[skill_id] +
			spellbonuses.SkillDamageAmount[skill_id] +
			aabonuses.SkillDamageAmount[skill_id]
		);
	}

	skill_dmg += (
		spellbonuses.SkillDamageAmount2[EQ::skills::HIGHEST_SKILL + 1] +
		itembonuses.SkillDamageAmount2[EQ::skills::HIGHEST_SKILL + 1] +
		aabonuses.SkillDamageAmount2[EQ::skills::HIGHEST_SKILL + 1]
	);

	if (skill_id != ALL_SKILLS) {
		skill_dmg += (
			itembonuses.SkillDamageAmount2[skill_id] +
			spellbonuses.SkillDamageAmount2[skill_id] +
			aabonuses.SkillDamageAmount2[skill_id]
		);
	}

	return skill_dmg;
}

int16 Mob::GetPositionalDmgAmt(Mob* defender)
{
	if (!defender)
		return 0;

	//SPA 504
	int front_arc_dmg_amt = 0;
	int back_arc_dmg_amt = 0;

	int total_amt = 0;

	back_arc_dmg_amt += itembonuses.Melee_Damage_Position_Amt[SBIndex::POSITION_BACK] + aabonuses.Melee_Damage_Position_Amt[SBIndex::POSITION_BACK] + spellbonuses.Melee_Damage_Position_Amt[SBIndex::POSITION_BACK];
	front_arc_dmg_amt += itembonuses.Melee_Damage_Position_Amt[SBIndex::POSITION_FRONT] + aabonuses.Melee_Damage_Position_Amt[SBIndex::POSITION_FRONT] + spellbonuses.Melee_Damage_Position_Amt[SBIndex::POSITION_FRONT];

	if (back_arc_dmg_amt || front_arc_dmg_amt) {
		if (BehindMob(defender, GetX(), GetY()))
			total_amt = back_arc_dmg_amt;
		else
			total_amt = front_arc_dmg_amt;
	}

	return total_amt;
}

void Mob::MeleeLifeTap(int64 damage) {

	int32 lifetap_amt = 0;
	int32 melee_lifetap_mod = spellbonuses.MeleeLifetap + itembonuses.MeleeLifetap + aabonuses.MeleeLifetap
					+ spellbonuses.Vampirism + itembonuses.Vampirism + aabonuses.Vampirism;

	if(melee_lifetap_mod && damage > 0){

		lifetap_amt = damage * (static_cast<float>(melee_lifetap_mod) / 100.0f);
		LogCombat("Melee lifetap healing [{}] points of damage with modifier of [{}] ", lifetap_amt, melee_lifetap_mod);

		if (lifetap_amt >= 0) {
			HealDamage(lifetap_amt); //Heal self for modified damage amount.
		}
		else {
			Damage(this, -lifetap_amt, 0, EQ::skills::SkillEvocation, false); //Dmg self for modified damage amount.
		}
	}
}

bool Mob::TryDoubleMeleeRoundEffect() {

	auto chance = aabonuses.DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_CHANCE] + itembonuses.DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_CHANCE] +
							spellbonuses.DoubleMeleeRound[SBIndex::DOUBLE_MELEE_ROUND_CHANCE];

	if (chance && zone->random.Roll(chance)) {
		SetUseDoubleMeleeRoundDmgBonus(true);
		return true;
	}

	SetUseDoubleMeleeRoundDmgBonus(false);
	return false;
}

void Mob::DoGravityEffect()
{
	Mob *caster = nullptr;
	int away = -1;
	float caster_x, caster_y, amount, value, cur_x, my_x, cur_y, my_y, x_vector, y_vector, hypot;

	// Set values so we can run through all gravity effects and then apply the culmative move at the end
	// instead of many small moves if the mob/client had more than 1 gravity effect on them
	cur_x = my_x = GetX();
	cur_y = my_y = GetY();

	int buff_count = GetMaxTotalSlots();
	for (int slot = 0; slot < buff_count; slot++)
	{
		if (IsValidSpell(buffs[slot].spellid) && IsEffectInSpell(buffs[slot].spellid, SpellEffect::GravityEffect))
		{
			for (int i = 0; i < EFFECT_COUNT; i++)
			{
				if(spells[buffs[slot].spellid].effect_id[i] == SpellEffect::GravityEffect) {

					int casterId = buffs[slot].casterid;
					if(casterId)
						caster = entity_list.GetMob(casterId);

					if(!caster || casterId == GetID())
						continue;

					caster_x = caster->GetX();
					caster_y = caster->GetY();

					value = static_cast<float>(spells[buffs[slot].spellid].base_value[i]);
					if(value == 0)
						continue;

					if(value > 0)
						away = 1;

					amount = std::abs(value) /
						 (100.0f); // to bring the values in line, arbitarily picked

					x_vector = cur_x - caster_x;
					y_vector = cur_y - caster_y;
					hypot = sqrt(x_vector*x_vector + y_vector*y_vector);

					if(hypot <= 5) // dont want to be inside the mob, even though we can, it looks bad
						continue;

					x_vector /= hypot;
					y_vector /= hypot;

					cur_x = cur_x + (x_vector * amount * away);
					cur_y = cur_y + (y_vector * amount * away);
				}
			}
		}
	}

	if ((std::abs(my_x - cur_x) > 0.01) || (std::abs(my_y - cur_y) > 0.01)) {
		float new_ground = GetGroundZ(cur_x, cur_y);
		// If we cant get LoS on our new spot then keep checking up to 5 units up.
		if(!CheckLosFN(cur_x, cur_y, new_ground, GetSize())) {
			for(float z_adjust = 0.1f; z_adjust < 5; z_adjust += 0.1f) {
				if(CheckLosFN(cur_x, cur_y, new_ground+z_adjust, GetSize())) {
					new_ground += z_adjust;
					break;
				}
			}
			// If we still fail, then lets only use the x portion(ie sliding around a wall)
			if(!CheckLosFN(cur_x, my_y, new_ground, GetSize())) {
				// If that doesnt work, try the y
				if(!CheckLosFN(my_x, cur_y, new_ground, GetSize())) {
					// If everything fails, then lets do nothing
					return;
				}
				else {
					cur_x = my_x;
				}
			}
			else {
				cur_y = my_y;
			}
		}

		if(IsClient())
			CastToClient()->MovePC(zone->GetZoneID(), zone->GetInstanceID(), cur_x, cur_y, new_ground, GetHeading());
		else
			GMMove(cur_x, cur_y, new_ground, GetHeading());
	}
}

void Mob::AddNimbusEffect(int effect_id)
{
	SetNimbusEffect(effect_id);

	auto outapp = new EQApplicationPacket(OP_AddNimbusEffect, sizeof(RemoveNimbusEffect_Struct));
	auto ane = (RemoveNimbusEffect_Struct *)outapp->pBuffer;
	ane->spawnid = GetID();
	ane->nimbus_effect = effect_id;
	entity_list.QueueClients(this, outapp);
	safe_delete(outapp);
}

void Mob::RemoveNimbusEffect(int effect_id)
{
	if (effect_id == nimbus_effect1)
		nimbus_effect1 = 0;

	else if (effect_id == nimbus_effect2)
		nimbus_effect2 = 0;

	else if (effect_id == nimbus_effect3)
		nimbus_effect3 = 0;

	auto outapp = new EQApplicationPacket(OP_RemoveNimbusEffect, sizeof(RemoveNimbusEffect_Struct));
	RemoveNimbusEffect_Struct* rne = (RemoveNimbusEffect_Struct*)outapp->pBuffer;
	rne->spawnid = GetID();
	rne->nimbus_effect = effect_id;
	entity_list.QueueClients(this, outapp);
	safe_delete(outapp);
}

void Mob::RemoveAllNimbusEffects()
{
	uint32 nimbus_effects[3] = { nimbus_effect1, nimbus_effect2, nimbus_effect3 };
	for (auto &current_nimbus : nimbus_effects) {
		auto remove_packet = new EQApplicationPacket(OP_RemoveNimbusEffect, sizeof(RemoveNimbusEffect_Struct));
		auto *remove_effect = (RemoveNimbusEffect_Struct*)remove_packet->pBuffer;
		remove_effect->spawnid = GetID();
		remove_effect->nimbus_effect = current_nimbus;
		entity_list.QueueClients(this, remove_packet);
		safe_delete(remove_packet);
	}
	nimbus_effect1 = 0;
	nimbus_effect2 = 0;
	nimbus_effect3 = 0;
}

bool Mob::IsBoat() const {

	return (
		race == Race::Ship ||
		race == Race::Launch ||
		race == Race::GhostShip ||
		race == Race::DiscordShip ||
		race == Race::MerchantShip ||
		race == Race::PirateShip ||
		race == Race::GhostShip2 ||
		race == Race::Boat2
	);
}

bool Mob::IsControllableBoat() const {

	return (
		race == Race::Boat ||
		race == Race::Rowboat
	);
}

void Mob::SetBodyType(uint8 new_body, bool overwrite_orig) {
	bool needs_spawn_packet = false;
	if(bodytype == 11 || bodytype >= 65 || new_body == 11 || new_body >= 65) {
		needs_spawn_packet = true;
	}

	if(overwrite_orig) {
		orig_bodytype = new_body;
	}
	bodytype = new_body;

	if(needs_spawn_packet) {
		auto app = new EQApplicationPacket;
		CreateDespawnPacket(app, true);
		entity_list.QueueClients(this, app);
		CreateSpawnPacket(app, this);
		entity_list.QueueClients(this, app);
		safe_delete(app);
	}
}


void Mob::ModSkillDmgTaken(EQ::skills::SkillType skill_num, int value)
{
	if (skill_num == ALL_SKILLS)
		SkillDmgTaken_Mod[EQ::skills::HIGHEST_SKILL + 1] = value;

	else if (skill_num >= 0 && skill_num <= EQ::skills::HIGHEST_SKILL)
		SkillDmgTaken_Mod[skill_num] = value;
}

int16 Mob::GetModSkillDmgTaken(const EQ::skills::SkillType skill_num)
{
	if (skill_num == ALL_SKILLS)
		return SkillDmgTaken_Mod[EQ::skills::HIGHEST_SKILL + 1];

	else if (skill_num >= 0 && skill_num <= EQ::skills::HIGHEST_SKILL)
		return SkillDmgTaken_Mod[skill_num];

	return 0;
}

void Mob::ModVulnerability(uint8 resist, int16 value)
{
	if (resist < HIGHEST_RESIST+1)
		Vulnerability_Mod[resist] = value;

	else if (resist == 255)
		Vulnerability_Mod[HIGHEST_RESIST+1] = value;
}

int16 Mob::GetModVulnerability(const uint8 resist)
{
	if (resist < HIGHEST_RESIST+1)
		return Vulnerability_Mod[resist];

	else if (resist == 255)
		return Vulnerability_Mod[HIGHEST_RESIST+1];

	return 0;
}

void Mob::CastOnCurer(uint32 spell_id)
{
	for(int i = 0; i < EFFECT_COUNT; i++)
	{
		if (spells[spell_id].effect_id[i] == SpellEffect::CastOnCurer)
		{
			if(IsValidSpell(spells[spell_id].base_value[i]))
			{
				SpellFinished(spells[spell_id].base_value[i], this);
			}
		}
	}
}

void Mob::CastOnCure(uint32 spell_id)
{
	for(int i = 0; i < EFFECT_COUNT; i++)
	{
		if (spells[spell_id].effect_id[i] == SpellEffect::CastOnCure)
		{
			if(IsValidSpell(spells[spell_id].base_value[i]))
			{
				SpellFinished(spells[spell_id].base_value[i], this);
			}
		}
	}
}

void Mob::CastOnNumHitFade(uint32 spell_id)
{
	if(!IsValidSpell(spell_id))
		return;

	for(int i = 0; i < EFFECT_COUNT; i++)
	{
		if (spells[spell_id].effect_id[i] == SpellEffect::CastonNumHitFade)
		{
			if(IsValidSpell(spells[spell_id].base_value[i]))
			{
				SpellFinished(spells[spell_id].base_value[i], this);
			}
		}
	}
}

void Mob::SlowMitigation(Mob* caster)
{
	if (GetSlowMitigation() && caster && caster->IsClient())
	{
		if ((GetSlowMitigation() > 0) && (GetSlowMitigation() < 26))
			caster->MessageString(Chat::SpellFailure, SLOW_MOSTLY_SUCCESSFUL);

		else if ((GetSlowMitigation() >= 26) && (GetSlowMitigation() < 74))
			caster->MessageString(Chat::SpellFailure, SLOW_PARTIALLY_SUCCESSFUL);

		else if ((GetSlowMitigation() >= 74) && (GetSlowMitigation() < 101))
			caster->MessageString(Chat::SpellFailure, SLOW_SLIGHTLY_SUCCESSFUL);

		else if (GetSlowMitigation() > 100)
			caster->MessageString(Chat::SpellFailure, SPELL_OPPOSITE_EFFECT);
	}
}

EQ::skills::SkillType Mob::GetSkillByItemType(int ItemType)
{
	switch (ItemType) {
		case EQ::item::ItemType1HSlash:
			return EQ::skills::Skill1HSlashing;
		case EQ::item::ItemType2HSlash:
			return EQ::skills::Skill2HSlashing;
		case EQ::item::ItemType1HPiercing:
			return EQ::skills::Skill1HPiercing;
		case EQ::item::ItemType1HBlunt:
			return EQ::skills::Skill1HBlunt;
		case EQ::item::ItemType2HBlunt:
			return EQ::skills::Skill2HBlunt;
		case EQ::item::ItemType2HPiercing:
			if (IsClient() && CastToClient()->ClientVersion() < EQ::versions::ClientVersion::RoF2)
				return EQ::skills::Skill1HPiercing;
			else
				return EQ::skills::Skill2HPiercing;
		case EQ::item::ItemTypeBow:
			return EQ::skills::SkillArchery;
		case EQ::item::ItemTypeLargeThrowing:
		case EQ::item::ItemTypeSmallThrowing:
			return EQ::skills::SkillThrowing;
		case EQ::item::ItemTypeMartial:
			return EQ::skills::SkillHandtoHand;
		default:
			return EQ::skills::SkillHandtoHand;
	}
 }

uint8 Mob::GetItemTypeBySkill(EQ::skills::SkillType skill)
{
	switch (skill) {
		case EQ::skills::SkillThrowing:
			return EQ::item::ItemTypeSmallThrowing;
		case EQ::skills::SkillArchery:
			return EQ::item::ItemTypeArrow;
		case EQ::skills::Skill1HSlashing:
			return EQ::item::ItemType1HSlash;
		case EQ::skills::Skill2HSlashing:
			return EQ::item::ItemType2HSlash;
		case EQ::skills::Skill1HPiercing:
			return EQ::item::ItemType1HPiercing;
		case EQ::skills::Skill2HPiercing: // watch for undesired client behavior
			return EQ::item::ItemType2HPiercing;
		case EQ::skills::Skill1HBlunt:
			return EQ::item::ItemType1HBlunt;
		case EQ::skills::Skill2HBlunt:
			return EQ::item::ItemType2HBlunt;
		case EQ::skills::SkillHandtoHand:
			return EQ::item::ItemTypeMartial;
		default:
			return EQ::item::ItemTypeMartial;
	}
 }

uint16 Mob::GetWeaponSpeedbyHand(uint16 hand) {

	uint16 weapon_speed = 0;
	switch (hand) {
		case 13:
			weapon_speed = attack_timer.GetDuration();
			break;
		case 14:
			weapon_speed = attack_dw_timer.GetDuration();
			break;
		case 11:
			weapon_speed = ranged_timer.GetDuration();
			break;
	}

	if (weapon_speed < RuleI(Combat, MinHastedDelay))
		weapon_speed = RuleI(Combat, MinHastedDelay);

	return weapon_speed;
}

int8 Mob::GetDecayEffectValue(uint16 spell_id, uint16 spelleffect) {

	if (!IsValidSpell(spell_id))
		return false;

	int spell_level = spells[spell_id].classes[(GetClass()%17) - 1];
	int effect_value = 0;
	int lvlModifier = 100;

	int buff_count = GetMaxTotalSlots();
	for (int slot = 0; slot < buff_count; slot++){
		if (IsValidSpell(buffs[slot].spellid)){
			for (int i = 0; i < EFFECT_COUNT; i++){
				if(spells[buffs[slot].spellid].effect_id[i] == spelleffect) {

					int critchance = spells[buffs[slot].spellid].base_value[i];
					int decay = spells[buffs[slot].spellid].limit_value[i];
					int lvldiff = spell_level - spells[buffs[slot].spellid].max_value[i];

					if(lvldiff > 0 && decay > 0)
					{
						lvlModifier -= decay*lvldiff;
						if (lvlModifier > 0){
							critchance = (critchance*lvlModifier)/100;
							effect_value += critchance;
						}
					}

					else
						effect_value += critchance;
				}
			}
		}
	}

	return effect_value;
}

// Faction Mods for Alliance type spells (only 1 ever active)
void Mob::AddFactionBonus(uint32 pFactionID,int32 bonus) {
	current_alliance_faction = pFactionID;
	current_alliance_mod = bonus;
}

// Faction Mods from items
void Mob::AddItemFactionBonus(uint32 pFactionID,int32 bonus) {
	std::map <uint32, int32> :: const_iterator faction_bonus;
	typedef std::pair <uint32, int32> NewFactionBonus;

	faction_bonus = item_faction_bonuses.find(pFactionID);
	if(faction_bonus == item_faction_bonuses.end())
	{
		item_faction_bonuses.emplace(NewFactionBonus(pFactionID,bonus));
	}
	else
	{
		if((bonus > 0 && faction_bonus->second < bonus) || (bonus < 0 && faction_bonus->second > bonus))
		{
			item_faction_bonuses.erase(pFactionID);
			item_faction_bonuses.emplace(NewFactionBonus(pFactionID,bonus));
		}
	}
}

int32 Mob::GetFactionBonus(uint32 pFactionID) {
	if(current_alliance_faction == pFactionID)
	{
		return current_alliance_mod;
	}
	return 0;
}

int32 Mob::GetItemFactionBonus(uint32 pFactionID) {
	std::map <uint32, int32> :: const_iterator faction_bonus;
	faction_bonus = item_faction_bonuses.find(pFactionID);
	if(faction_bonus != item_faction_bonuses.end())
	{
		return (*faction_bonus).second;
	}
	return 0;
}

void Mob::ClearItemFactionBonuses() {
	item_faction_bonuses.clear();
}

FACTION_VALUE Mob::GetSpecialFactionCon(Mob* iOther) {
	if (!iOther)
		return FACTION_INDIFFERENTLY;

	iOther = iOther->GetOwnerOrSelf();
	Mob* self = GetOwnerOrSelf();

	bool selfAIcontrolled = self->IsAIControlled();
	bool iOtherAIControlled = iOther->IsAIControlled();
	int selfPrimaryFaction = self->GetPrimaryFaction();
	int iOtherPrimaryFaction = iOther->GetPrimaryFaction();

	if (selfPrimaryFaction >= 0 && selfAIcontrolled)
		return FACTION_INDIFFERENTLY;
	if (iOther->GetPrimaryFaction() >= 0)
		return FACTION_INDIFFERENTLY;
/* special values:
	-2 = indiff to player, ally to AI on special values, indiff to AI
	-3 = dub to player, ally to AI on special values, indiff to AI
	-4 = atk to player, ally to AI on special values, indiff to AI
	-5 = indiff to player, indiff to AI
	-6 = dub to player, indiff to AI
	-7 = atk to player, indiff to AI
	-8 = indiff to players, ally to AI on same value, indiff to AI
	-9 = dub to players, ally to AI on same value, indiff to AI
	-10 = atk to players, ally to AI on same value, indiff to AI
	-11 = indiff to players, ally to AI on same value, atk to AI
	-12 = dub to players, ally to AI on same value, atk to AI
	-13 = atk to players, ally to AI on same value, atk to AI
*/
	switch (iOtherPrimaryFaction) {
		case -2: // -2 = indiff to player, ally to AI on special values, indiff to AI
			if (selfAIcontrolled && iOtherAIControlled)
				return FACTION_ALLY;
			else
				return FACTION_INDIFFERENTLY;
		case -3: // -3 = dub to player, ally to AI on special values, indiff to AI
			if (selfAIcontrolled && iOtherAIControlled)
				return FACTION_ALLY;
			else
				return FACTION_DUBIOUSLY;
		case -4: // -4 = atk to player, ally to AI on special values, indiff to AI
			if (selfAIcontrolled && iOtherAIControlled)
				return FACTION_ALLY;
			else
				return FACTION_SCOWLS;
		case -5: // -5 = indiff to player, indiff to AI
			return FACTION_INDIFFERENTLY;
		case -6: // -6 = dub to player, indiff to AI
			if (selfAIcontrolled && iOtherAIControlled)
				return FACTION_INDIFFERENTLY;
			else
				return FACTION_DUBIOUSLY;
		case -7: // -7 = atk to player, indiff to AI
			if (selfAIcontrolled && iOtherAIControlled)
				return FACTION_INDIFFERENTLY;
			else
				return FACTION_SCOWLS;
		case -8: // -8 = indiff to players, ally to AI on same value, indiff to AI
			if (selfAIcontrolled && iOtherAIControlled) {
				if (selfPrimaryFaction == iOtherPrimaryFaction)
					return FACTION_ALLY;
				else
					return FACTION_INDIFFERENTLY;
			}
			else
				return FACTION_INDIFFERENTLY;
		case -9: // -9 = dub to players, ally to AI on same value, indiff to AI
			if (selfAIcontrolled && iOtherAIControlled) {
				if (selfPrimaryFaction == iOtherPrimaryFaction)
					return FACTION_ALLY;
				else
					return FACTION_INDIFFERENTLY;
			}
			else
				return FACTION_DUBIOUSLY;
		case -10: // -10 = atk to players, ally to AI on same value, indiff to AI
			if (selfAIcontrolled && iOtherAIControlled) {
				if (selfPrimaryFaction == iOtherPrimaryFaction)
					return FACTION_ALLY;
				else
					return FACTION_INDIFFERENTLY;
			}
			else
				return FACTION_SCOWLS;
		case -11: // -11 = indiff to players, ally to AI on same value, atk to AI
			if (selfAIcontrolled && iOtherAIControlled) {
				if (selfPrimaryFaction == iOtherPrimaryFaction)
					return FACTION_ALLY;
				else
					return FACTION_SCOWLS;
			}
			else
				return FACTION_INDIFFERENTLY;
		case -12: // -12 = dub to players, ally to AI on same value, atk to AI
			if (selfAIcontrolled && iOtherAIControlled) {
				if (selfPrimaryFaction == iOtherPrimaryFaction)
					return FACTION_ALLY;
				else
					return FACTION_SCOWLS;


			}
			else
				return FACTION_DUBIOUSLY;
		case -13: // -13 = atk to players, ally to AI on same value, atk to AI
			if (selfAIcontrolled && iOtherAIControlled) {
				if (selfPrimaryFaction == iOtherPrimaryFaction)
					return FACTION_ALLY;
				else
					return FACTION_SCOWLS;
			}
			else
				return FACTION_SCOWLS;
		default:
			return FACTION_INDIFFERENTLY;
	}
}

bool Mob::HasSpellEffect(int effect_id)
{
	const auto buff_count = GetMaxTotalSlots();
	for (int i = 0; i < buff_count; i++) {
		const auto spell_id = buffs[i].spellid;

		if (!IsValidSpell(spell_id)) {
			continue;
		}

		if (IsEffectInSpell(spell_id, effect_id)) {
			return true;
		}
	}

	return false;
}

int Mob::GetSpecialAbility(int ability)
{
	if (ability >= SpecialAbility::Max || ability < 0) {
		return 0;
	}

	return SpecialAbilities[ability].level;
}

bool Mob::HasSpecialAbilities()
{
	for (int i = 0; i < SpecialAbility::Max; ++i) {
		if (GetSpecialAbility(i)) {
			return true;
		}
	}

	return false;
}

int Mob::GetSpecialAbilityParam(int ability, int param) {
	if(param >= SpecialAbility::MaxParameters || param < 0 || ability >= SpecialAbility::Max || ability < 0) {
		return 0;
	}

	return SpecialAbilities[ability].params[param];
}

void Mob::SetSpecialAbility(int ability, int level) {
	if(ability >= SpecialAbility::Max || ability < 0) {
		return;
	}

	SpecialAbilities[ability].level = level;
}

void Mob::SetSpecialAbilityParam(int ability, int param, int value) {
	if(param >= SpecialAbility::MaxParameters || param < 0 || ability >= SpecialAbility::Max || ability < 0) {
		return;
	}

	SpecialAbilities[ability].params[param] = value;
}

void Mob::StartSpecialAbilityTimer(int ability, uint32 time) {
	if (ability >= SpecialAbility::Max || ability < 0) {
		return;
	}

	if(SpecialAbilities[ability].timer) {
		SpecialAbilities[ability].timer->Start(time);
	} else {
		SpecialAbilities[ability].timer = new Timer(time);
		SpecialAbilities[ability].timer->Start();
	}
}

void Mob::StopSpecialAbilityTimer(int ability) {
	if (ability >= SpecialAbility::Max || ability < 0) {
		return;
	}

	safe_delete(SpecialAbilities[ability].timer);
}

Timer *Mob::GetSpecialAbilityTimer(int ability) {
	if (ability >= SpecialAbility::Max || ability < 0) {
		return nullptr;
	}

	return SpecialAbilities[ability].timer;
}

void Mob::ClearSpecialAbilities() {
	for(int a = 0; a < SpecialAbility::Max; ++a) {
		SpecialAbilities[a].level = 0;
		safe_delete(SpecialAbilities[a].timer);
		for(int p = 0; p < SpecialAbility::MaxParameters; ++p) {
			SpecialAbilities[a].params[p] = 0;
		}
	}
}

void Mob::ProcessSpecialAbilities(const std::string &str) {
	ClearSpecialAbilities();

	const auto& sp = Strings::Split(str, '^');
	for (const auto& s : sp) {
		const auto& sub_sp = Strings::Split(s, ',');
		if (
			sub_sp.size() >= 2 &&
			Strings::IsNumber(sub_sp[0]) &&
			Strings::IsNumber(sub_sp[1])
		) {
			int ability_id = Strings::ToInt(sub_sp[0]);
			int value = Strings::ToInt(sub_sp[1]);

			SetSpecialAbility(ability_id, value);

			switch (ability_id) {
				case SpecialAbility::QuadrupleAttack:
					if (value > 0) {
						SetSpecialAbility(SpecialAbility::TripleAttack, 1);
					}
					break;
				case SpecialAbility::DestructibleObject:
					if (value == 0) {
						SetDestructibleObject(false);
					} else {
						SetDestructibleObject(true);
					}
					break;
				default:
					break;
			}

			for (size_t i = 2, param_id = 0; i < sub_sp.size(); ++i, ++param_id) {
				if (param_id >= SpecialAbility::MaxParameters) {
					break;
				}

				if (Strings::IsNumber(sub_sp[i])) {
					SetSpecialAbilityParam(ability_id, param_id, Strings::ToInt(sub_sp[i]));
				}
			}
		}
	}
}

// derived from client to keep these functions more consistent
// if anything seems weird, blame SoE
bool Mob::IsFacingMob(Mob *other)
{
	if (!other)
		return false;
	float angle = HeadingAngleToMob(other);
	float heading = GetHeading();

	if (angle > 472.0 && heading < 40.0)
		angle = heading;
	if (angle < 40.0 && heading > 472.0)
		angle = heading;

	if (std::abs(angle - heading) <= 80.0)
		return true;

	return false;
}

// All numbers derived from the client
float Mob::HeadingAngleToMob(float other_x, float other_y)
{
	float this_x = GetX();
	float this_y = GetY();

	return CalculateHeadingAngleBetweenPositions(this_x, this_y, other_x, other_y);
}

uint8 Mob::GetSeeInvisibleLevelFromNPCStat(uint16 in_see_invis)
{
	/*
		Returns the NPC's see invisible level based on 'see_invs' value in npc_types.
		1 = See Invs Level 1, 2-99 will gives a random roll to apply see invs level 1
		100 = See Invs Level 2, where 101-199 gives a random roll to apply see invs 2, if fails get see invs 1
		ect... for higher levels, 200,300 ect.
		MAX 25499, which can give you level 254.
	*/

	//npc does not have see invis
	if (!in_see_invis) {
		return 0;
	}
	//npc has basic see invis
	if (in_see_invis == 1) {
		return 1;
	}

	//random chance to apply standard level 1 see invs
	if (in_see_invis > 1 && in_see_invis < 100) {
		if (zone->random.Int(0, 99) < in_see_invis) {
			return 1;
		}
	}
	//covers npcs with see invis levels beyond level 1, max calculated level allowed is 254
	int see_invis_level = 1;
	see_invis_level += (in_see_invis / 100);

	int see_invis_chance = in_see_invis % 100;

	//has enhanced see invis level
	if (see_invis_chance == 0) {
		return std::min(see_invis_level, MAX_INVISIBILTY_LEVEL);
	}
	//has chance for enhanced see invis level
	if (zone->random.Int(0, 99) < see_invis_chance) {
		return std::min(see_invis_level, MAX_INVISIBILTY_LEVEL);
	}
	//failed chance at attempted enhanced see invs level, use previous level.
	return std::min((see_invis_level - 1), MAX_INVISIBILTY_LEVEL);
}

int32 Mob::GetSpellStat(uint32 spell_id, const char *identifier, uint8 slot)
{
	return GetSpellStatValue(spell_id, identifier, slot);
}

bool Mob::CanClassEquipItem(uint32 item_id)
{
	const auto *item = database.GetItem(item_id);
	if (!item) {
		return false;
	}

	const uint16 item_classes = item->Classes;
	if (item_classes == Class::ALL_CLASSES_BITMASK) {
		return true;
	}

	const uint8 class_id = GetClass();
	if (!IsPlayerClass(class_id)) {
		return false;
	}

	const uint16 class_bitmask = GetPlayerClassBit(class_id);
	return (item_classes & class_bitmask);
}

bool Mob::CanRaceEquipItem(uint32 item_id)
{
	const EQ::ItemData* itm = nullptr;
	itm = database.GetItem(item_id);

	if (!itm) {
		return false;
	}

	auto item_races = itm->Races;
	if(item_races == RaceBitmask::All) {
		return true;
	}

	auto race_id = GetBaseRace();
	if (!IsPlayerRace(race_id)) {
		return false;
	}

	int race_bitmask = GetPlayerRaceBit(race_id);
	if(!(item_races & race_bitmask)) {
		return false;
	} else {
		return true;
	}
}

void Mob::SendAddPlayerState(PlayerState new_state)
{
	static EQApplicationPacket p(OP_PlayerStateAdd, sizeof(PlayerState_Struct));
	auto                       b = (PlayerState_Struct *) p.pBuffer;

	b->spawn_id = GetID();
	b->state    = static_cast<uint32>(new_state);

	AddPlayerState(b->state);
	entity_list.QueueClients(nullptr, &p);
}

void Mob::SendRemovePlayerState(PlayerState old_state)
{
	static EQApplicationPacket p(OP_PlayerStateRemove, sizeof(PlayerState_Struct));
	auto                       b = (PlayerState_Struct *) p.pBuffer;

	b->spawn_id = GetID();
	b->state    = static_cast<uint32>(old_state);

	RemovePlayerState(b->state);
	entity_list.QueueClients(nullptr, &p);
}

int32 Mob::GetMeleeMitigation() {
	int32 mitigation = 0;
	mitigation += spellbonuses.MeleeMitigationEffect;
	mitigation += itembonuses.MeleeMitigationEffect;
	mitigation += aabonuses.MeleeMitigationEffect;
	return mitigation;
}

/* this is the mob being attacked.
 * Pass in the weapon's EQ::ItemInstance
 */
int Mob::ResistElementalWeaponDmg(const EQ::ItemInstance *item)
{
	if (!item)
		return 0;
	int magic = 0, fire = 0, cold = 0, poison = 0, disease = 0, chromatic = 0, prismatic = 0, physical = 0,
	    corruption = 0;
	int resist = 0;
	int roll = 0;
	/*  this is how the client does the resist rolls for these.
	 *  Given the difficulty of parsing out these resists, I'll trust the client
	 */
	if (item->GetItemElementalDamage(magic, fire, cold, poison, disease, chromatic, prismatic, physical, corruption, true)) {
		if (magic) {
			resist = GetMR();
			if (resist >= 201) {
				magic = 0;
			} else {
				roll = zone->random.Int(0, 200) - resist;
				if (roll < 1)
					magic = 0;
				else if (roll < 100)
					magic = magic * roll / 100;
			}
		}

		if (fire) {
			resist = GetFR();
			if (resist >= 201) {
				fire = 0;
			} else {
				roll = zone->random.Int(0, 200) - resist;
				if (roll < 1)
					fire = 0;
				else if (roll < 100)
					fire = fire * roll / 100;
			}
		}

		if (cold) {
			resist = GetCR();
			if (resist >= 201) {
				cold = 0;
			} else {
				roll = zone->random.Int(0, 200) - resist;
				if (roll < 1)
					cold = 0;
				else if (roll < 100)
					cold = cold * roll / 100;
			}
		}

		if (poison) {
			resist = GetPR();
			if (resist >= 201) {
				poison = 0;
			} else {
				roll = zone->random.Int(0, 200) - resist;
				if (roll < 1)
					poison = 0;
				else if (roll < 100)
					poison = poison * roll / 100;
			}
		}

		if (disease) {
			resist = GetDR();
			if (resist >= 201) {
				disease = 0;
			} else {
				roll = zone->random.Int(0, 200) - resist;
				if (roll < 1)
					disease = 0;
				else if (roll < 100)
					disease = disease * roll / 100;
			}
		}

		if (corruption) {
			resist = GetCorrup();
			if (resist >= 201) {
				corruption = 0;
			} else {
				roll = zone->random.Int(0, 200) - resist;
				if (roll < 1)
					corruption = 0;
				else if (roll < 100)
					corruption = corruption * roll / 100;
			}
		}

		if (chromatic) {
			resist = GetFR();
			int temp = GetCR();
			if (temp < resist)
				resist = temp;

			temp = GetMR();
			if (temp < resist)
				resist = temp;

			temp = GetDR();
			if (temp < resist)
				resist = temp;

			temp = GetPR();
			if (temp < resist)
				resist = temp;

			if (resist >= 201) {
				chromatic = 0;
			} else {
				roll = zone->random.Int(0, 200) - resist;
				if (roll < 1)
					chromatic = 0;
				else if (roll < 100)
					chromatic = chromatic * roll / 100;
			}
		}

		if (prismatic) {
			resist = (GetFR() + GetCR() + GetMR() + GetDR() + GetPR()) / 5;
			if (resist >= 201) {
				prismatic = 0;
			} else {
				roll = zone->random.Int(0, 200) - resist;
				if (roll < 1)
					prismatic = 0;
				else if (roll < 100)
					prismatic = prismatic * roll / 100;
			}
		}

		if (physical) {
			resist = GetPhR();
			if (resist >= 201) {
				physical = 0;
			} else {
				roll = zone->random.Int(0, 200) - resist;
				if (roll < 1)
					physical = 0;
				else if (roll < 100)
					physical = physical * roll / 100;
			}
		}
	}

	return magic + fire + cold + poison + disease + chromatic + prismatic + physical + corruption;
}

/* this is the mob being attacked.
 * Pass in the weapon's EQ::ItemInstance
 */
int Mob::CheckBaneDamage(const EQ::ItemInstance *item)
{
	if (!item)
		return 0;

	int64 damage = item->GetItemBaneDamageBody(GetBodyType(), true);
	damage += item->GetItemBaneDamageRace(GetRace(), true);

	return damage;
}

void Mob::CancelSneakHide()
{
	if (hidden || improved_hidden) {
		hidden = false;
		improved_hidden = false;
		auto outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
		SpawnAppearance_Struct* sa_out = (SpawnAppearance_Struct*)outapp->pBuffer;
		sa_out->spawn_id = GetID();
		sa_out->type = 0x03;
		sa_out->parameter = 0;
		entity_list.QueueClients(this, outapp, true);
		safe_delete(outapp);
	}
}

void Mob::CommonBreakInvisible()
{
	BreakInvisibleSpells();
	CancelSneakHide();
}

float Mob::GetDefaultRaceSize(int race_id, int gender_id) const {
	return GetRaceGenderDefaultHeight(
		race_id > 0 ? race_id : race,
		gender_id >= 0 ? gender_id : gender
	);
}

bool Mob::ShieldAbility(uint32 target_id, int shielder_max_distance, int shield_duration, int shield_target_mitigation, int shielder_mitigation, bool use_aa, bool can_shield_npc)
{
	Mob* shield_target = entity_list.GetMob(target_id);
	if (!shield_target) {
		return false;
	}

	if (!can_shield_npc && shield_target->IsNPC()) {
		if (IsClient()) {
			MessageString(Chat::White, SHIELD_TARGET_NPC);
		}
		return false;
	}

	if (shield_target->GetID() == GetID()) { //Client will give message "You can not shield yourself"
		return false;
	}

	//Edge case situations. If 'Shield Target' still has Shielder set but Shielder is not in zone. Catch and fix here.
	if (shield_target->GetShielderID() && !entity_list.GetMob(shield_target->GetShielderID())) {
		shield_target->SetShielderID(0);
	}

	if (GetShielderID() && !entity_list.GetMob(GetShielderID())) {
		SetShielderID(0);
	}

	//You have a shielder, or your 'Shield Target' already has a 'Shielder'
	if (GetShielderID() || shield_target->GetShielderID()) {
		if (IsClient()) {
			MessageString(Chat::White, ALREADY_SHIELDED);
		}
		return false;
	}

	//You are being shielded or already have a 'Shield Target'
	if (GetShieldTargetID() || shield_target->GetShieldTargetID()) {
		if (IsClient()) {
			MessageString(Chat::White, ALREADY_SHIELDING);
		}
		return false;
	}

	//AA to increase SPA 230 extended shielding (default live is 15 distance units)
	if (use_aa) {
		shielder_max_distance += aabonuses.ExtendedShielding + itembonuses.ExtendedShielding + spellbonuses.ExtendedShielding;
		shielder_max_distance = std::max(shielder_max_distance, 0);
	}

	if (shield_target->CalculateDistance(GetX(), GetY(), GetZ()) > static_cast<float>(shielder_max_distance)) {
		MessageString(Chat::Blue, TARGET_TOO_FAR);
		return false;
	}

	entity_list.MessageCloseString(this, false, 100, 0, START_SHIELDING, GetCleanName(), shield_target->GetCleanName());

	SetShieldTargetID(shield_target->GetID());
	SetShielderMitigation(shielder_mitigation);
	SetShielderMaxDistance(shielder_max_distance);

	shield_target->SetShielderID(GetID());
	shield_target->SetShieldTargetMitigation(shield_target_mitigation);

	//Calculate AA for adding time SPA 255 extend shield duration (Baseline ability is 12 seconds)
	if (use_aa) {
		shield_duration += (aabonuses.ShieldDuration + itembonuses.ShieldDuration + spellbonuses.ShieldDuration) * 1000;
		shield_duration = std::max(shield_duration, 1); //Incase of negative modifiers lets just make min duration 1 ms.
	}

	shield_timer.Start(static_cast<uint32>(shield_duration));
	return true;
}

void Mob::ShieldAbilityFinish()
{
	Mob* shield_target = entity_list.GetMob(GetShieldTargetID());

	if (shield_target) {
		entity_list.MessageCloseString(this, false, 100, 0, END_SHIELDING, GetCleanName(), shield_target->GetCleanName());
		shield_target->SetShielderID(0);
		shield_target->SetShieldTargetMitigation(0);
	}
	SetShieldTargetID(0);
	SetShielderMitigation(0);
	SetShielderMaxDistance(0);
	shield_timer.Disable();
}

void Mob::ShieldAbilityClearVariables()
{
	//If 'shield target' dies
	if (GetShielderID()){
		Mob* shielder = entity_list.GetMob(GetShielderID());
		if (shielder) {
			shielder->SetShieldTargetID(0);
			shielder->SetShielderMitigation(0);
			shielder->SetShielderMaxDistance(0);
			shielder->shield_timer.Disable();
		}
		SetShielderID(0);
		SetShieldTargetMitigation(0);
	}

	//If 'shielder' dies
	if (GetShieldTargetID()) {
		Mob* shield_target = entity_list.GetMob(GetShieldTargetID());
		if (shield_target) {
			shield_target->SetShielderID(0);
			shield_target->SetShieldTargetMitigation(0);
		}
		SetShieldTargetID(0);
		SetShielderMitigation(0);
		SetShielderMaxDistance(0);
		shield_timer.Disable();
	}
}

void Mob::SetFeigned(bool in_feigned) {

	if (in_feigned)	{
		if (IsClient()) {
			if (RuleB(Character, FeignKillsPet)){
				SetPet(0);
			}
			CastToClient()->SetHorseId(0);
		}
		entity_list.ClearFeignAggro(this);
		forget_timer.Start(FeignMemoryDuration);
	}
	else {
		forget_timer.Disable();
	}
	feigned = in_feigned;
}

bool Mob::JoinHealRotationTargetPool(std::shared_ptr<HealRotation>* heal_rotation)
{
	if (IsHealRotationTarget())
		return false;
	if (!heal_rotation->use_count())
		return false;
	if (!(*heal_rotation))
		return false;
	if (!IsHealRotationTargetMobType(this))
		return false;

	if (!(*heal_rotation)->AddTargetToPool(this))
		return false;

	m_target_of_heal_rotation = *heal_rotation;

	return IsHealRotationTarget();
}

bool Mob::LeaveHealRotationTargetPool()
{
	if (!IsHealRotationTarget()) {
		m_target_of_heal_rotation.reset();
		return true;
	}

	m_target_of_heal_rotation->RemoveTargetFromPool(this);
	m_target_of_heal_rotation.reset();

	return !IsHealRotationTarget();
}

uint32 Mob::HealRotationHealCount()
{
	if (!IsHealRotationTarget())
		return 0;

	return m_target_of_heal_rotation->HealCount(this);
}

uint32 Mob::HealRotationExtendedHealCount()
{
	if (!IsHealRotationTarget())
		return 0;

	return m_target_of_heal_rotation->ExtendedHealCount(this);
}

float Mob::HealRotationHealFrequency()
{
	if (!IsHealRotationTarget())
		return 0.0f;

	return m_target_of_heal_rotation->HealFrequency(this);
}

float Mob::HealRotationExtendedHealFrequency()
{
	if (!IsHealRotationTarget())
		return 0.0f;

	return m_target_of_heal_rotation->ExtendedHealFrequency(this);
}

bool Mob::CanOpenDoors() const
{
	return m_can_open_doors;
}

void Mob::SetCanOpenDoors(bool can_open)
{
	m_can_open_doors = can_open;
}

void Mob::DeleteBucket(std::string bucket_name)
{
	DataBucketKey k = GetScopedBucketKeys();
	k.key = bucket_name;

	DataBucket::DeleteData(&database, k);
}

std::string Mob::GetBucket(std::string bucket_name)
{
	DataBucketKey k = GetScopedBucketKeys();
	k.key = bucket_name;

	auto b = DataBucket::GetData(&database, k);
	if (!b.value.empty()) {
		return b.value;
	}
	return {};
}

std::string Mob::GetBucketExpires(std::string bucket_name)
{
	DataBucketKey k = GetScopedBucketKeys();
	k.key = bucket_name;

	std::string bucket_expiration = DataBucket::GetDataExpires(&database, k);
	if (!bucket_expiration.empty()) {
		return bucket_expiration;
	}

	return {};
}

std::string Mob::GetBucketRemaining(std::string bucket_name)
{
	DataBucketKey k = GetScopedBucketKeys();
	k.key = bucket_name;

	std::string bucket_remaining = DataBucket::GetDataRemaining(&database, k);
	if (!bucket_remaining.empty() && Strings::ToInt(bucket_remaining) > 0) {
		return bucket_remaining;
	}
	else if (Strings::ToInt(bucket_remaining) == 0) {
		return "0";
	}

	return {};
}

void Mob::SetBucket(std::string bucket_name, std::string bucket_value, std::string expiration)
{
	DataBucketKey k = GetScopedBucketKeys();
	k.key     = bucket_name;
	k.expires = expiration;
	k.value   = bucket_value;

	DataBucket::SetData(&database, k);
}

std::string Mob::GetMobDescription()
{
	return fmt::format(
		"[{}] ({})",
		GetCleanName(),
		GetID()
	);
}

uint8 Mob::ConvertItemTypeToSkillID(uint8 item_type)
{
	if (item_type >= EQ::item::ItemTypeCount) {
		return EQ::skills::SkillHandtoHand;
	}

	std::map<uint8, uint8> convert_item_types_map = {
		{ EQ::item::ItemType1HSlash, EQ::skills::Skill1HSlashing },
		{ EQ::item::ItemType2HSlash, EQ::skills::Skill2HSlashing },
		{ EQ::item::ItemType1HPiercing, EQ::skills::Skill1HPiercing },
		{ EQ::item::ItemType2HPiercing, EQ::skills::Skill2HPiercing },
		{ EQ::item::ItemType1HBlunt, EQ::skills::Skill1HBlunt },
		{ EQ::item::ItemType2HBlunt, EQ::skills::Skill2HBlunt },
		{ EQ::item::ItemTypeBow, EQ::skills::SkillArchery },
		{ EQ::item::ItemTypeSmallThrowing, EQ::skills::SkillThrowing },
		{ EQ::item::ItemTypeLargeThrowing, EQ::skills::SkillThrowing },
		{ EQ::item::ItemTypeShield, EQ::skills::SkillBash },
		{ EQ::item::ItemTypeArmor, EQ::skills::SkillHandtoHand },
		{ EQ::item::ItemTypeMartial, EQ::skills::SkillHandtoHand }
	};

	const auto& s = convert_item_types_map.find(item_type);
	if (s != convert_item_types_map.end()) {
		return s->second;
	}

	return EQ::skills::SkillHandtoHand;
}

void Mob::CloneAppearance(Mob* other, bool clone_name)
{
	if (!other) {
		return;
	}

	SendIllusionPacket(
		AppearanceStruct{
			.beard = other->GetBeard(),
			.beard_color = other->GetBeardColor(),
			.drakkin_details = other->GetDrakkinDetails(),
			.drakkin_heritage = other->GetDrakkinHeritage(),
			.drakkin_tattoo = other->GetDrakkinTattoo(),
			.eye_color_one = other->GetEyeColor1(),
			.eye_color_two = other->GetEyeColor2(),
			.face = other->GetLuclinFace(),
			.gender_id = other->GetGender(),
			.hair = other->GetHairStyle(),
			.hair_color = other->GetHairColor(),
			.helmet_texture = other->GetHelmTexture(),
			.race_id = other->GetRace(),
			.size = other->GetSize(),
			.texture = other->GetTexture(),
		}
	);

	for (
		uint8 slot = EQ::textures::armorHead;
		slot <= EQ::textures::armorFeet;
		slot++
	) {
		auto color = 0;
		auto material = 0;
		if (other->IsClient()) {
			color = other->CastToClient()->GetEquipmentColor(slot);
			material = other->CastToClient()->GetEquipmentMaterial(slot);
		} else {
			color = other->GetArmorTint(slot);
			material = !slot ? other->GetHelmTexture() : other->GetTexture();
		}

		WearChange(slot, material, color);
	}

	WearChange(
		EQ::textures::weaponPrimary,
		other->GetEquipmentMaterial(EQ::textures::weaponPrimary),
		other->GetEquipmentColor(EQ::textures::weaponPrimary)
	);

	WearChange(
		EQ::textures::weaponSecondary,
		other->GetEquipmentMaterial(EQ::textures::weaponSecondary),
		other->GetEquipmentColor(EQ::textures::weaponSecondary)
	);

	if (IsNPC()) {
		auto primary_skill = (
			other->IsNPC() ?
			other->CastToNPC()->GetPrimSkill() :
			ConvertItemTypeToSkillID(other->GetEquipmentType(EQ::textures::weaponSecondary))
		);

		auto secondary_skill = (
			other->IsNPC() ?
			other->CastToNPC()->GetSecSkill()  :
			ConvertItemTypeToSkillID(other->GetEquipmentType(EQ::textures::weaponSecondary))
		);

		CastToNPC()->SetPrimSkill(primary_skill);
		CastToNPC()->SetSecSkill(secondary_skill);
	}

	if (clone_name) {
		TempName(other->GetCleanName());
	}
}

void Mob::CopyHateList(Mob* to) {
	if (hate_list.IsHateListEmpty() || !to || to->IsClient()) {
		return;
	}

	for (const auto& h : hate_list.GetHateList()) {
		if (h->entity_on_hatelist) {
			to->AddToHateList(h->entity_on_hatelist, h->stored_hate_amount, h->hatelist_damage);
		}
	}
}

int Mob::DispatchZoneControllerEvent(
	QuestEventID evt,
	Mob* init,
	const std::string& data,
	uint32 extra,
	std::vector<std::any>* pointers
) {
	auto ret = 0;

	if (
		RuleB(Zone, UseZoneController) &&
		(
			!IsNPC() ||
			(IsNPC() && !IsZoneController())
		)
	) {
		auto controller = entity_list.GetNPCByNPCTypeID(ZONE_CONTROLLER_NPC_ID);
		if (controller) {
			ret = parse->EventNPC(evt, controller, init, data, extra, pointers);
		}
	}

	return ret;
}

std::string Mob::GetRacePlural()
{
	switch (GetBaseRace()) {
		case Race::Human:
			return "Humans";
		case Race::Barbarian:
			return "Barbarians";
		case Race::Erudite:
			return "Erudites";
		case Race::WoodElf:
			return "Wood Elves";
		case Race::HighElf:
			return "High Elves";
		case Race::DarkElf:
			return "Dark Elves";
		case Race::HalfElf:
			return "Half Elves";
		case Race::Dwarf:
			return "Dwarves";
		case Race::Troll:
			return "Trolls";
		case Race::Ogre:
			return "Ogres";
		case Race::Halfling:
			return "Halflings";
		case Race::Gnome:
			return "Gnomes";
		case Race::Iksar:
			return "Iksar";
		case Race::VahShir:
			return "Vah Shir";
		case Race::Froglok2:
			return "Frogloks";
		case Race::Drakkin:
			return "Drakkin";
		default:
			return "Races";
	}
}

std::string Mob::GetClassPlural()
{
	switch (GetClass()) {
		case Class::Warrior:
			return "Warriors";
		case Class::Cleric:
			return "Clerics";
		case Class::Paladin:
			return "Paladins";
		case Class::Ranger:
			return "Rangers";
		case Class::ShadowKnight:
			return fmt::format("{}s", shadow_knight_class_name);
		case Class::Druid:
			return "Druids";
		case Class::Monk:
			return "Monks";
		case Class::Bard:
			return "Bards";
		case Class::Rogue:
			return "Rogues";
		case Class::Shaman:
			return "Shamans";
		case Class::Necromancer:
			return "Necromancers";
		case Class::Wizard:
			return "Wizards";
		case Class::Magician:
			return "Magicians";
		case Class::Enchanter:
			return "Enchanters";
		case Class::Beastlord:
			return "Beastlords";
		case Class::Berserker:
			return "Berserkers";
		default:
			return "Classes";
	}
}

DataBucketKey Mob::GetScopedBucketKeys()
{
	DataBucketKey k = {};
	if (IsClient()) {
		k.character_id = CastToClient()->CharacterID();
	} else if (IsNPC()) {
		k.npc_id = GetNPCTypeID();
	} else if (IsBot()) {
		k.bot_id = CastToBot()->GetBotID();
	}

	return k;
}

uint32 Mob::GetMobTypeIdentifier()
{
	if (IsClient()) {
		return CastToClient()->CharacterID();
	} else if (IsNPC()) {
		return GetNPCTypeID();
	} else if (IsBot()) {
		return CastToBot()->GetBotID();
	}

	return 0;
}

void Mob::HandleDoorOpen()
{
	for (auto e : entity_list.GetDoorsList()) {
		Doors *d = e.second;
		if (d->GetKeyItem()) {
			continue;
		}
		if (d->GetLockpick()) {
			continue;
		}
		if (d->IsDoorOpen()) {
			continue;
		}
		if (d->IsDoorBlacklisted()) {
			continue;
		}

		// If the door is a trigger door, check if the trigger door is open
		if (d->GetTriggerDoorID() > 0) {
			auto td = entity_list.GetDoorsByDoorID(d->GetTriggerDoorID());
			if (td) {
				if (Strings::RemoveNumbers(d->GetDoorName()) != Strings::RemoveNumbers(td->GetDoorName())) {
					continue;
				}
			}
		}

		if (d->GetDoorParam() > 0) {
			continue;
		}

		float distance                = DistanceSquared(m_Position, d->GetPosition());
		float distance_scan_door_open = 20;

		if (distance <= (distance_scan_door_open * distance_scan_door_open)) {
			// Make sure we're opening a door within height relevance and not platforms above or below us
			if (std::abs(m_Position.z - d->GetPosition().z) > 10) {
				continue;
			}

			d->ForceOpen(this);
		}
	}
}

void Mob::SetExtraHaste(int haste, bool need_to_save)
{
	extra_haste = haste;

	if (need_to_save) {
		if (IsBot()) {
			auto e = BotDataRepository::FindOne(database, CastToBot()->GetBotID());
			if (!e.bot_id) {
				return;
			}

			e.extra_haste = haste;

			BotDataRepository::UpdateOne(database, e);
		} else if (IsClient()) {
			auto e = CharacterDataRepository::FindOne(database, CastToClient()->CharacterID());
			if (!e.id) {
				return;
			}

			e.extra_haste = haste;

			CharacterDataRepository::UpdateOne(database, e);
		}
	}
}

bool Mob::IsCloseToBanker()
{
	for (auto &e: GetCloseMobList()) {
		auto mob = e.second;
		if (mob && mob->IsNPC() && mob->GetClass() == Class::Banker) {
			return true;
		}
	}

	return false;
}

bool Mob::HasBotAttackFlag(Mob* tar) {
	if (!tar) {
		return false;
	}

	std::vector<uint32> l = tar->GetBotAttackFlags();

	for (uint32 e : l) {
		if (IsBot() && e == CastToBot()->GetBotOwnerCharacterID()) {
			return true;
		}

		if (IsClient() && e == CastToClient()->CharacterID()) {
			return true;
		}
	}

	return false;
}


const uint16 scan_close_mobs_timer_moving = 6000; // 6 seconds
const uint16 scan_close_mobs_timer_idle = 60000; // 60 seconds

// If the moving timer triggers, lets see if we are moving or idle to restart the appropriate dynamic timer
void Mob::CheckScanCloseMobsMovingTimer()
{
	LogAIScanCloseDetail(
		"Mob [{}] {}moving, scan timer [{}]",
		GetCleanName(),
		IsMoving() ? "" : "NOT ",
		m_scan_close_mobs_timer.GetRemainingTime()
	);

	// If the mob is still moving, restart the moving timer
	if (moving) {
		if (m_scan_close_mobs_timer.GetRemainingTime() > scan_close_mobs_timer_moving) {
			LogAIScanCloseDetail("Mob [{}] Restarting with moving timer", GetCleanName());
			m_scan_close_mobs_timer.Disable();
			m_scan_close_mobs_timer.Start(scan_close_mobs_timer_moving);
			m_scan_close_mobs_timer.Trigger();
		}
	}
		// If the mob is not moving, restart the idle timer
	else if (m_scan_close_mobs_timer.GetDuration() == scan_close_mobs_timer_moving) {
		LogAIScanCloseDetail("Mob [{}] Restarting with idle timer", GetCleanName());
		m_scan_close_mobs_timer.Disable();
		m_scan_close_mobs_timer.Start(scan_close_mobs_timer_idle);
	}
}

std::unordered_map<uint16, Mob*>& Mob::GetCloseMobList(float distance)
{
	return entity_list.GetCloseMobList(this, distance);
}

void Mob::ClearDataBucketCache()
{
	if (IsOfClientBot()) {
		uint64                   id = 0;
		DataBucketLoadType::Type t{};
		if (IsBot()) {
			id = CastToBot()->GetBotID();
			t  = DataBucketLoadType::Bot;
		}
		else if (IsClient()) {
			id = CastToClient()->CharacterID();
			t  = DataBucketLoadType::Client;
		}

		DataBucket::DeleteFromCache(id, t);
	}
}

bool Mob::IsInGroupOrRaid(Mob* other, bool same_raid_group) {
	if (!other || !IsOfClientBotMerc() || !other->IsOfClientBotMerc()) {
		return false;
	}

	if (this == other) {
		return true;
	}

	Raid* raid = IsBot() ? CastToBot()->GetStoredRaid() : (IsRaidGrouped() ? GetRaid() : nullptr);

	if (raid) {
		if (!other->IsRaidGrouped()) {
			return false;
		}

		Raid* other_raid = other->IsBot() ? other->CastToBot()->GetStoredRaid() : other->GetRaid();

		if (!other_raid) {
			return false;
		}

		auto raid_group = raid->GetGroup(GetCleanName());
		auto other_raid_group = other_raid->GetGroup(other->GetCleanName());

		if (
			raid_group == RAID_GROUPLESS ||
			other_raid_group == RAID_GROUPLESS ||
			(same_raid_group && raid_group != other_raid_group)
		) {
			return false;
		}

		return true;
	}

	Group* group = GetGroup();
	Group* other_group = other->GetGroup();

	return group && group == other_group;
}

bool Mob::DoLosChecks(Mob* other) {
	if (!CheckLosFN(other) || !CheckWaterLoS(other)) {
		if (RuleB(Map, EnableLoSCheatExemptions) && CheckLosCheatExempt(other)) {
			return true;
		}

		return false;
	}

	if (RuleB(Map, CheckForDoorLoSCheat) && !CheckDoorLoSCheat(other)) {
		return false;
	}

	return true;
}

bool Mob::CheckDoorLoSCheat(Mob* other) {
	if (!other->IsOfClientBotMerc() && other->CastToNPC()->IsOnHatelist(this)) {
		return true;
	}

	const std::string& zones_to_check = RuleS(Map, ZonesToCheckDoorCheat);

	if (zones_to_check.empty()) {
		return true;
	}

	const auto& v = Strings::Split(zones_to_check, ",");

	if (zones_to_check == "all" || std::find(v.begin(), v.end(), std::to_string(zone->GetZoneID())) != v.end()) {
		for (auto itr: entity_list.GetDoorsList()) {
			Doors *d = itr.second;

			if (
				!d->IsDoorOpen() &&
				(
					d->GetKeyItem() ||
					d->GetLockpick() ||
					d->IsDoorBlacklisted() ||
					d->GetNoKeyring() != 0
				)
				) {
				float distance = Distance(m_Position, d->GetPosition());

				if (distance > RuleR(Map, RangeCheckForDoorLoSCheat) || !CheckLosFN(d->GetX(), d->GetY(), d->GetZ(), GetSize())) {
					continue;
				}

				if (d->IsDoorBetween(GetPosition(), other->GetPosition(), d->GetSize())) {
					return false;
				}
			}
		}
	}

	return true;
}

bool Mob::CheckLosCheatExempt(Mob* other) {
	glm::vec4 exempt_check_who;

	switch (zone->GetZoneID()) {
		case Zones::POEARTHB:
			exempt_check_who.x = 2053; exempt_check_who.y = 408; exempt_check_who.z = -219; //Middle of councilman spawns
			//if the player is inside the cove they cannot be higher than the ceiling (no exploiting from uptop) --- 800 from center of council to furthest corner in cove
			if (GetZ() <= -171 && other->GetZ() <= -171 && DistanceNoZ(other->GetPosition(), exempt_check_who) <= 800 && DistanceNoZ(GetPosition(), exempt_check_who) <= 800) {
				return true;
			}
		default:
			return false;
	}

	return false;
}

bool Mob::IsGuildmaster() const {
	switch (GetClass()) {
		case Class::WarriorGM:
		case Class::ClericGM:
		case Class::PaladinGM:
		case Class::RangerGM:
		case Class::ShadowKnightGM:
		case Class::DruidGM:
		case Class::MonkGM:
		case Class::BardGM:
		case Class::RogueGM:
		case Class::ShamanGM:
		case Class::NecromancerGM:
		case Class::WizardGM:
		case Class::MagicianGM:
		case Class::EnchanterGM:
		case Class::BeastlordGM:
		case Class::BerserkerGM:
			return true;
		default:
			return false;
	}
}

bool Mob::LoadDataBucketsCache()
{
	const uint32 id = GetMobTypeIdentifier();

	if (!id) {
		return false;
	}

	if (IsBot()) {
		DataBucket::BulkLoadEntitiesToCache(&database, DataBucketLoadType::Bot, {id});
	}
	else if (IsClient()) {
		uint32 account_id = CastToClient()->AccountID();
		DataBucket::BulkLoadEntitiesToCache(&database, DataBucketLoadType::Account, {account_id});
		DataBucket::BulkLoadEntitiesToCache(&database, DataBucketLoadType::Client, {id});
	}

	return true;
}
