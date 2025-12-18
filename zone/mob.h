
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

#pragma once

#include "common/data_bucket.h"
#include "common/emu_constants.h"
#include "common/light_source.h"
#include "zone/aa_ability.h"
#include "zone/aa.h"
#include "zone/combat_record.h"
#include "zone/common.h"
#include "zone/entity.h"
#include "zone/event_codes.h"
#include "zone/hate_list.h"
#include "zone/heal_rotation.h"
#include "zone/pathfinder_interface.h"
#include "zone/position.h"

#include <any>
#include <memory>
#include <set>
#include <vector>

char* strn0cpy(char* dest, const char* source, uint32 size);

class Client;
class EQApplicationPacket;
class Group;
class NPC;
class Raid;
class Aura;
struct AuraRecord;
struct NewSpawn_Struct;
struct PlayerPositionUpdateServer_Struct;
class MobMovementManager;

const int COLLISION_BOX_SIZE = 8;

namespace EQ
{
	struct ItemData;
	class ItemInstance;
}

namespace DeathSave {
    constexpr uint32 HP300  = 1;
    constexpr uint32 HP8000 = 2;
}

enum class eSpecialAttacks : int {
	None,
	Rampage,
	AERampage,
	ChaoticStab
};

struct AppearanceStruct {
	uint8  aa_title         = UINT8_MAX;
	uint8  beard            = UINT8_MAX;
	uint8  beard_color      = UINT8_MAX;
	uint32 drakkin_details  = UINT32_MAX;
	uint32 drakkin_heritage = UINT32_MAX;
	uint32 drakkin_tattoo   = UINT32_MAX;
	uint8  eye_color_one    = UINT8_MAX;
	uint8  eye_color_two    = UINT8_MAX;
	uint8  face             = UINT8_MAX;
	uint8  gender_id        = UINT8_MAX;
	uint8  hair             = UINT8_MAX;
	uint8  hair_color       = UINT8_MAX;
	uint8  helmet_texture   = UINT8_MAX;
	uint16 race_id          = Race::Doug;
	bool   send_effects     = true;
	float  size             = -1.0f;
	Client *target          = nullptr;
	uint8  texture          = UINT8_MAX;
};

class DataBucketKey;
class Mob : public Entity {
public:
	enum CLIENT_CONN_STATUS { CLIENT_CONNECTING, CLIENT_CONNECTED, CLIENT_LINKDEAD,
						CLIENT_KICKED, DISCONNECTED, CLIENT_ERROR, CLIENT_CONNECTINGALL };

	struct MobSpecialAbility {
		MobSpecialAbility() {
			level = 0;
			timer = nullptr;
			for (int i = 0; i < SpecialAbility::MaxParameters; ++i) {
				params[i] = 0;
			}
		}

		~MobSpecialAbility() {
			safe_delete(timer);
		}

		int level;
		Timer *timer;
		int params[SpecialAbility::MaxParameters];
	};

	struct AuraInfo {
		char name[64];
		int spawn_id;
		int icon;
		Aura *aura;
		AuraInfo() : spawn_id(0), icon(0), aura(nullptr)
		{
			memset(name, 0, 64);
		}
	};

	struct AuraMgr {
		int count; // active auras
		AuraInfo auras[AURA_HARDCAP];
		AuraMgr() : count(0) { }
	};

	Mob(
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
		uint16 in_see_invis, // see through invis
		uint16 in_see_invis_undead, // see through invis vs. undead
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
		bool in_always_aggros_foes,
		int32 in_heroic_strikethrough,
		bool keeps_sold_items,
		int64 in_hp_regen_per_second = 0,
		uint32 npc_tint_id = 0
	);
	virtual ~Mob();

	inline virtual bool IsMob() const { return true; }
	inline virtual bool InZone() const { return true; }

	void DisplayInfo(Mob *mob);

	std::unordered_map<uint16, Mob *>  m_close_mobs;
	std::unordered_map<int, glm::vec4> m_last_seen_mob_position;
	Timer                              m_scan_close_mobs_timer;
	Timer                              m_see_close_mobs_timer;
	Timer                              m_mob_check_moving_timer;

	uint16 m_last_wearchange_race_id = 0;
	// client_id -> slot_id -> key
	std::unordered_map<uint32_t, std::unordered_map<uint8_t, uint64_t>> m_last_seen_wearchange;
	Timer m_clear_wearchange_cache_timer;

	// Bot attack flag
	Timer bot_attack_flag_timer;

	//Somewhat sorted: needs documenting!

	//Attack
	virtual void RogueBackstab(Mob* other, bool min_damage = false, int ReuseTime = 10);
	virtual void RogueAssassinate(Mob* other);
	float MobAngle(Mob *other = 0, float ourx = 0.0f, float oury = 0.0f) const;
	// greater than 90 is behind
	inline bool BehindMob(Mob *other = 0, float ourx = 0.0f, float oury = 0.0f) const
		{ return (!other || other == this) ? true : MobAngle(other, ourx, oury) > 90.0f; }
	// less than 56 is in front, greater than 56 is usually where the client generates the messages
	inline bool InFrontMob(Mob *other = 0, float ourx = 0.0f, float oury = 0.0f) const
		{ return (!other || other == this) ? true : MobAngle(other, ourx, oury) < 56.0f; }
	bool IsFacingMob(Mob *other); // kind of does the same as InFrontMob, but derived from client
	float HeadingAngleToMob(Mob *other) { return HeadingAngleToMob(other->GetX(), other->GetY()); }
	float HeadingAngleToMob(float other_x, float other_y); // to keep consistent with client generated messages
	virtual void RangedAttack(Mob* other) { }
	virtual void ThrowingAttack(Mob* other) { }
	// 13 = Primary (default), 14 = secondary
	virtual bool Attack(Mob* other, int Hand = EQ::invslot::slotPrimary, bool FromRiposte = false, bool IsStrikethrough = false,
	bool IsFromSpell = false, ExtraAttackOptions *opts = nullptr);
	void DoAttack(Mob *other, DamageHitInfo &hit, ExtraAttackOptions *opts = nullptr, bool FromRiposte = false);
	int MonkSpecialAttack(Mob* other, uint8 skill_used);
	virtual void TryBackstab(Mob *other,int ReuseTime = 10);
	bool AvoidDamage(Mob *attacker, DamageHitInfo &hit);
	int compute_tohit(EQ::skills::SkillType skillinuse);
	int GetTotalToHit(EQ::skills::SkillType skill, int chance_mod); // compute_tohit + spell bonuses
	int compute_defense();
	int GetTotalDefense(); // compute_defense + spell bonuses
	bool CheckHitChance(Mob* attacker, DamageHitInfo &hit);
	void TryCriticalHit(Mob *defender, DamageHitInfo &hit, ExtraAttackOptions *opts = nullptr);
	void TryPetCriticalHit(Mob *defender, DamageHitInfo &hit);
	virtual bool TryFinishingBlow(Mob *defender, int64 &damage);
	int TryHeadShot(Mob* defender, EQ::skills::SkillType skillInUse);
	int TryAssassinate(Mob* defender, EQ::skills::SkillType skillInUse);
	virtual void DoRiposte(Mob* defender);
	void ApplyMeleeDamageMods(uint16 skill, int64 &damage, Mob * defender = nullptr, ExtraAttackOptions *opts = nullptr);
	int ACSum(bool skip_caps = false);
	inline int GetDisplayAC() { return 1000 * (ACSum(true) + compute_defense()) / 847; }
	int offense(EQ::skills::SkillType skill);
	int GetBestMeleeSkill();
	void CalcAC() { mitigation_ac = ACSum(); }
	int GetACSoftcap();
	double GetSoftcapReturns();
	int GetClassRaceACBonus();
	inline int GetMitigationAC() { return mitigation_ac; }
	void MeleeMitigation(Mob *attacker, DamageHitInfo &hit, ExtraAttackOptions *opts = nullptr);
	double RollD20(int offense, int mitigation); // CALL THIS FROM THE DEFENDER
	bool CombatRange(Mob* other, float fixed_size_mod = 1.0, bool aeRampage = false, ExtraAttackOptions *opts = nullptr);
	virtual inline bool IsBerserk() { return false; } // only clients
	void RogueEvade(Mob *other);
	void CommonOutgoingHitSuccess(Mob *defender, DamageHitInfo &hit, ExtraAttackOptions *opts = nullptr);
	bool HasDied();
	virtual bool CheckDualWield();
	void DoMainHandAttackRounds(Mob *target, ExtraAttackOptions *opts = nullptr, bool rampage = false);
	void DoOffHandAttackRounds(Mob *target, ExtraAttackOptions *opts = nullptr, bool rampage = false);
	virtual bool CheckDoubleAttack();
	// inline process for places where we need to do them outside of the AI_Process
	void ProcessAttackRounds(Mob *target, ExtraAttackOptions *opts = nullptr, bool rampage = false)
	{
		if (target) {
			DoMainHandAttackRounds(target, opts, rampage);
			if (CanThisClassDualWield()) {
				DoOffHandAttackRounds(target, opts, rampage);
			}
		}
		return;
	}

	bool HasAnInvisibilityEffect();
	void BreakCharmPetIfConditionsMet();
	//Invisible
	bool IsInvisible(Mob* other = 0) const;
	void SetInvisible(uint8 state, bool set_on_bonus_calc = false);

	void CalcSeeInvisibleLevel();
	void CalcInvisibleLevel();
	void ZeroInvisibleVars(uint8 invisible_type);

	inline uint8 GetSeeInvisibleLevelFromNPCStat(uint16 in_see_invis);

	void BreakInvisibleSpells();
	virtual void CancelSneakHide();
	void CommonBreakInvisible();
	void CommonBreakInvisibleFromCombat();

	inline uint8 GetInvisibleLevel() const { return invisible; }
	inline uint8 GetInvisibleUndeadLevel() const { return invisible_undead; }

	inline bool SeeHide() const { return see_hide; }
	inline bool SeeImprovedHide() const { return see_improved_hide; }
	inline uint8 SeeInvisibleUndead() const { return see_invis_undead; }
	inline uint8 SeeInvisible() const { return see_invis; }

	inline void SetInnateSeeInvisible(uint8 val) { innate_see_invis = val; }
	inline void SetSeeInvisibleUndead(uint8 val) { see_invis_undead = val; }

	uint32 tmHidden; // timestamp of hide, only valid while hidden == true
	uint8 invisible, nobuff_invisible, invisible_undead, invisible_animals;
	uint8 see_invis, innate_see_invis, see_invis_undead; //TODO: do we need a see_invis_animal ?

	bool sneaking, hidden, improved_hidden;
	bool see_hide, see_improved_hide;

	/**
	 ************************************************
	 * Appearance
	 ************************************************
	 */

	EQ::InternalTextureProfile mob_texture_profile = {};

	EQ::skills::SkillType AttackAnimation(int Hand, const EQ::ItemInstance* weapon, EQ::skills::SkillType skillinuse = EQ::skills::Skill1HBlunt);

	uint32 GetTextureProfileMaterial(uint8 material_slot) const;
	uint32 GetTextureProfileColor(uint8 material_slot) const;
	uint32 GetTextureProfileHeroForgeModel(uint8 material_slot) const;

	virtual void SendArmorAppearance(Client *one_client = nullptr);
	virtual void SendTextureWC(uint8 slot, uint32 texture, uint32 hero_forge_model = 0, uint32 elite_material = 0, uint32 unknown06 = 0, uint32 unknown18 = 0);
	virtual void SendWearChange(uint8 material_slot, Client *one_client = nullptr);
	virtual void SetSlotTint(uint8 material_slot, uint8 red_tint, uint8 green_tint, uint8 blue_tint);
	virtual void WearChange(uint8 material_slot, uint32 texture, uint32 color = 0, uint32 hero_forge_model = 0);

	void ChangeSize(float in_size, bool unrestricted = false);
	void DoAnim(const int animation_id, int animation_speed = 0, bool ackreq = true, eqFilterType filter = FilterNone);
	void ProjectileAnimation(Mob* to, int item_id, bool IsArrow = false, float speed = 0, float angle = 0, float tilt = 0, float arc = 0, const char *IDFile = nullptr, EQ::skills::SkillType skillInUse = EQ::skills::SkillArchery);
	void SendAppearanceEffect(uint32 parm1, uint32 parm2, uint32 parm3, uint32 parm4, uint32 parm5, Client *specific_target=nullptr, uint32 value1slot = 1, uint32 value1ground = 1, uint32 value2slot = 1, uint32 value2ground = 1,
		uint32 value3slot = 1, uint32 value3ground = 1, uint32 value4slot = 1, uint32 value4ground = 1, uint32 value5slot = 1, uint32 value5ground = 1);
	void SendLevelAppearance();
	void SendTargetable(bool on, Client *specific_target = nullptr);
	void SetMobTextureProfile(uint8 material_slot, uint32 texture, uint32 color = 0, uint32 hero_forge_model = 0);

	//Spell
	void SendSpellEffect(uint32 effect_id, uint32 duration, uint32 finish_delay, bool zone_wide,
		uint32 unk020, bool perm_effect = false, Client *c = nullptr, uint32 caster_id = 0, uint32 target_id = 0);
	bool IsBeneficialAllowed(Mob *target);
	virtual int GetCasterLevel(uint16 spell_id);
	void ApplySpellsBonuses(uint16 spell_id, uint8 casterlevel, StatBonuses* newbon, uint16 casterID = 0,
		uint8 WornType = 0, int32 ticsremaining = 0, int buffslot = -1, int instrument_mod = 10,
		bool IsAISpellEffect = false, uint16 effect_id = 0, int32 se_base = 0, int32 se_limit = 0, int32 se_max = 0);
	void NegateSpellEffectBonuses(uint16 spell_id);
	bool NegateSpellEffect(uint16 spell_id, int effect_id);
	float GetActSpellRange(uint16 spell_id, float range);
	int64 GetActSpellDamage(uint16 spell_id, int64 value, Mob* target = nullptr);
	int64 GetActDoTDamage(uint16 spell_id, int64 value, Mob* target, bool from_buff_tic = true);
	int64 GetActSpellHealing(uint16 spell_id, int64 value, Mob* target = nullptr, bool from_buff_tic = false);
	int32 GetActSpellCost(uint16 spell_id, int32 cost);
	virtual int32 GetActSpellDuration(uint16 spell_id, int32 duration);
	int32 GetActSpellCasttime(uint16 spell_id, int32 casttime);
	virtual int64 GetActReflectedSpellDamage(uint16 spell_id, int64 value, int effectiveness);
	float ResistSpell(uint8 resist_type, uint16 spell_id, Mob *caster, bool use_resist_override = false,
		int resist_override = 0, bool CharismaCheck = false, bool CharmTick = false, bool IsRoot = false,
		int level_override = -1);
	int GetResist(uint8 resist_type);
	int ResistPhysical(int level_diff, uint8 caster_level);
	int ResistElementalWeaponDmg(const EQ::ItemInstance *item);
	int CheckBaneDamage(const EQ::ItemInstance *item);
	uint16 GetSpecializeSkillValue(uint16 spell_id) const;
	void SendSpellBarDisable();
	void SendSpellBarEnable(uint16 spellid);
	void ZeroCastingVars();
	virtual void SpellProcess();
	virtual bool CastSpell(uint16 spell_id, uint16 target_id, EQ::spells::CastingSlot slot = EQ::spells::CastingSlot::Item, int32 casttime = -1,
		int32 mana_cost = -1, uint32* oSpellWillFinish = 0, uint32 item_slot = 0xFFFFFFFF,
		uint32 timer = 0xFFFFFFFF, uint32 timer_duration = 0, int16 *resist_adjust = nullptr,
		uint32 aa_id = 0);
	virtual bool DoCastSpell(uint16 spell_id, uint16 target_id, EQ::spells::CastingSlot slot = EQ::spells::CastingSlot::Item, int32 casttime = -1,
		int32 mana_cost = -1, uint32* oSpellWillFinish = 0, uint32 item_slot = 0xFFFFFFFF,
		uint32 timer = 0xFFFFFFFF, uint32 timer_duration = 0, int16 resist_adjust = 0,
		uint32 aa_id = 0);
	void CastedSpellFinished(uint16 spell_id, uint32 target_id, EQ::spells::CastingSlot slot, int mana_used,
		uint32 inventory_slot = 0xFFFFFFFF, int16 resist_adjust = 0);
	bool SpellFinished(uint16 spell_id, Mob *target, EQ::spells::CastingSlot slot = EQ::spells::CastingSlot::Item, int mana_used = 0,
		uint32 inventory_slot = 0xFFFFFFFF, int16 resist_adjust = 0, bool isproc = false, int level_override = -1, uint32 timer = 0xFFFFFFFF, uint32 timer_duration = 0, bool from_casted_spell = false, uint32 aa_id = 0);
	void SendBeginCast(uint16 spell_id, uint32 casttime);
	virtual bool SpellOnTarget(
		uint16 spell_id,
		Mob* spelltar,
		int reflect_effectiveness = 0,
		bool use_resist_adjust = false,
		int16 resist_adjust = 0,
		bool isproc = false,
		int level_override = -1,
		int duration_override = 0,
		bool disable_buff_overwrite = false
	);
	virtual bool SpellEffect(Mob* caster, uint16 spell_id, float partial = 100, int level_override = -1, int reflect_effectiveness = 0, int32 duration_override = 0, bool disable_buff_overwrite = false);
	virtual bool DetermineSpellTargets(uint16 spell_id, Mob *&spell_target, Mob *&ae_center,
		CastAction_type &CastAction, EQ::spells::CastingSlot slot, bool isproc = false);
	bool DoCastingChecksOnCaster(int32 spell_id, EQ::spells::CastingSlot slot);
	bool DoCastingChecksZoneRestrictions(bool check_on_casting, int32 spell_id);
	bool DoCastingChecksOnTarget(bool check_on_casting, int32 spell_id, Mob* spell_target);
	virtual bool CheckFizzle(uint16 spell_id);
	virtual bool CheckSpellLevelRestriction(Mob *caster, uint16 spell_id);
	virtual bool IsImmuneToSpell(uint16 spell_id, Mob *caster);

	virtual float GetAOERange(uint16 spell_id);
	void InterruptSpell(uint16 spellid = SPELL_UNKNOWN);
	void InterruptSpell(uint16, uint16, uint16 spellid = SPELL_UNKNOWN);
	void StopCasting();
	void StopCastSpell(int32 spell_id, bool send_spellbar_enable);
	inline bool IsCasting() const { return((casting_spell_id != 0)); }
	uint16 CastingSpellID() const { return casting_spell_id; }
	bool TryDispel(uint8 caster_level, uint8 buff_level, int level_modifier);
	bool TrySpellProjectile(Mob* spell_target,  uint16 spell_id, float speed = 1.5f);
	void ResourceTap(int64 damage, uint16 spell_id);
	void TryTriggerThreshHold(int64 damage, int effect_id, Mob* attacker);
	void CalcDestFromHeading(float heading, float distance, float MaxZDiff, float StartX, float StartY, float &dX, float &dY, float &dZ);
	void BeamDirectional(uint16 spell_id, int16 resist_adjust);
	void ConeDirectional(uint16 spell_id, int16 resist_adjust);
	void ApplyHealthTransferDamage(Mob *caster, Mob *target, uint16 spell_id);
	void ApplySpellEffectIllusion(int32 spell_id, Mob* caster, int buffslot, int base, int limit, int max);
	void ApplyIllusionToCorpse(int32 spell_id, Corpse* new_corpse);
	void SendIllusionWearChange(Client* c);
	int16 GetItemSlotToConsumeCharge(int32 spell_id, uint32 inventory_slot);
	bool CheckItemRaceClassDietyRestrictionsOnCast(uint32 inventory_slot);
	bool IsFromTriggeredSpell(EQ::spells::CastingSlot slot, uint32 item_slot = 0xFFFFFFFF);

	//Bard
	bool ApplyBardPulse(int32 spell_id, Mob *spell_target, EQ::spells::CastingSlot slot);
	bool IsActiveBardSong(int32 spell_id);
	bool HasActiveSong() const { return(bardsong != 0); }
	void ZeroBardPulseVars();
	void DoBardCastingFromItemClick(bool is_casting_bard_song, uint32 cast_time, int32 spell_id, uint16 target_id, EQ::spells::CastingSlot slot, uint32 item_slot,
		uint32 recast_type , uint32 recast_delay);
	bool UseBardSpellLogic(uint16 spell_id = 0xffff, int slot = -1);

	//Buff
	void BuffProcess();
	virtual void DoBuffTic(const Buffs_Struct &buff, int slot, Mob* caster = nullptr);
	void BuffFadeBySpellID(uint16 spell_id);
	void BuffFadeBySpellIDAndCaster(uint16 spell_id, uint16 caster_id);
	void BuffFadeByEffect(int effect_id, int slot_to_skip = -1);
	void BuffFadeAll();
	void BuffFadeBeneficial();
	void BuffFadeNonPersistDeath();
	void BuffFadeDetrimental();
	void BuffFadeBySlot(int slot, bool iRecalcBonuses = true);
	void BuffFadeDetrimentalByCaster(Mob *caster);
	void BuffFadeBySitModifier();
	void BuffFadeSongs();
	void BuffDetachCaster(Mob *caster);
	bool IsAffectedByBuffByGlobalGroup(GlobalGroup group);
	void BuffModifyDurationBySpellID(uint16 spell_id, int32 newDuration);
	int AddBuff(Mob *caster, const uint16 spell_id, int duration = 0, int32 level_override = -1, bool disable_buff_overwrite = false);
	int CanBuffStack(uint16 spellid, uint8 caster_level, bool iFailIfOverwrite = false);
	int CalcBuffDuration(Mob *caster, Mob *target, uint16 spell_id, int32 caster_level_override = -1);
	void SendPetBuffsToClient();
	virtual int GetCurrentBuffSlots() const { return 0; }
	virtual int GetCurrentSongSlots() const { return 0; }
	virtual int GetCurrentDiscSlots() const { return 0; }
	virtual int GetMaxBuffSlots() const { return 0; }
	virtual int GetMaxSongSlots() const { return 0; }
	virtual int GetMaxDiscSlots() const { return 0; }
	virtual int GetMaxTotalSlots() const { return 0; }
	bool HasDiscBuff();
	virtual uint32 GetFirstBuffSlot(bool disc, bool song);
	virtual uint32 GetLastBuffSlot(bool disc, bool song);
	virtual void InitializeBuffSlots() { buffs = nullptr; }
	virtual void UninitializeBuffSlots() { }
	EQApplicationPacket *MakeBuffsPacket(bool for_target = true, bool clear_buffs = false);
	void SendBuffsToClient(Client *c);
	inline Buffs_Struct* GetBuffs() { return buffs; }
	void DoGravityEffect();
	void DamageShield(Mob* other, bool spell_ds = false);
	int32 RuneAbsorb(int64 damage, uint16 type);
	std::vector<uint16> GetBuffSpellIDs();
	bool FindBuff(uint16 spell_id, uint16 caster_id = 0);
	uint16 FindBuffBySlot(int slot);
	uint32 BuffCount(bool is_beneficial = true, bool is_detrimental = true);
	bool FindType(uint16 type, bool bOffensive = false, uint16 threshold = 100);
	int16 GetBuffSlotFromType(uint16 type);
	uint16 GetSpellIDFromSlot(uint8 slot);
	int CountDispellableBuffs();
	void CheckNumHitsRemaining(NumHit type, int32 buff_slot = -1, uint16 spell_id = SPELL_UNKNOWN);
	bool HasNumhits() const { return has_numhits; }
	inline void Numhits(bool val) { has_numhits = val; }
	bool HasMGB() const { return has_MGB; }
	inline void SetMGB(bool val) { has_MGB = val; }
	bool HasProjectIllusion() const { return has_ProjectIllusion ; }
	inline void SetProjectIllusion(bool val) { has_ProjectIllusion  = val; }
	bool IsNimbusEffectActive(uint32 nimbus_effect);
	void SetNimbusEffect(uint32 nimbus_effect);
	inline virtual uint32 GetNimbusEffect1() const { return nimbus_effect1; }
	inline virtual uint32 GetNimbusEffect2() const { return nimbus_effect2; }
	inline virtual uint32 GetNimbusEffect3() const { return nimbus_effect3; }
	void AddNimbusEffect(int effect_id);
	void RemoveNimbusEffect(int effect_id);
	void RemoveAllNimbusEffects();
	inline const glm::vec3& GetTargetRingLocation() const { return m_TargetRing; }
	inline float GetTargetRingX() const { return m_TargetRing.x; }
	inline float GetTargetRingY() const { return m_TargetRing.y; }
	inline float GetTargetRingZ() const { return m_TargetRing.z; }
	inline bool HasEndurUpkeep() const { return endur_upkeep; }
	inline void SetEndurUpkeep(bool val) { endur_upkeep = val; }
	bool HasBuffWithSpellGroup(int spell_group);
	void SetAppearanceEffects(int32 slot, int32 value);
	void ListAppearanceEffects(Client* c);
	void ClearAppearanceEffects();
	void SendSavedAppearanceEffects(Client *receiver);
	void SetBuffDuration(int spell_id, int duration = 0, int level_override = -1);
	void ApplySpellBuff(int spell_id, int duration = 0, int level_override = -1);
	int GetBuffStatValueBySpell(int32 spell_id, const char* stat_identifier);
	int GetBuffStatValueBySlot(uint8 slot, const char* stat_identifier);
	virtual bool GetIllusionBlock() const { return false; }

	//Basic Stats/Inventory
	virtual void SetLevel(uint8 in_level, bool command = false) { level = in_level; }
	void TempName(const char *newname = nullptr);
	void SetTargetable(bool on);
	bool IsTargetable() const { return m_targetable; }
	bool HasShieldEquipped() const { return has_shield_equipped; }
	inline void SetShieldEquipped(bool val) { has_shield_equipped = val; }
	bool HasTwoHandBluntEquipped() const { return has_two_hand_blunt_equipped; }
	inline void SetTwoHandBluntEquipped(bool val) { has_two_hand_blunt_equipped = val; }
	bool HasTwoHanderEquipped() { return has_two_hander_equipped; }
	void SetTwoHanderEquipped(bool val) { has_two_hander_equipped = val; }
	bool HasDualWeaponsEquipped() const { return has_dual_weapons_equipped; }
	bool HasBowEquipped() const { return has_bowequipped; }
	void SetBowEquipped(bool val) { has_bowequipped = val; }
	bool HasArrowEquipped() const { return has_arrowequipped; }
	void SetArrowEquipped(bool val) { has_arrowequipped = val; }
	bool HasBowAndArrowEquipped() const { return HasBowEquipped() && HasArrowEquipped(); }
	inline void SetDualWeaponsEquipped(bool val) { has_dual_weapons_equipped = val; }
	bool CanFacestab() { return can_facestab; }
	void SetFacestab(bool val) { can_facestab = val; }
	virtual uint8 ConvertItemTypeToSkillID(uint8 item_type);
	virtual uint16 GetSkill(EQ::skills::SkillType skill_num) const { return 0; }
	virtual uint32 GetEquippedItemFromTextureSlot(uint8 material_slot) const { return(0); }
	virtual uint32 GetEquipmentMaterial(uint8 material_slot) const;
	virtual uint8 GetEquipmentType(uint8 material_slot) const;
	virtual uint32 GetHerosForgeModel(uint8 material_slot) const;
	virtual uint32 GetEquipmentColor(uint8 material_slot) const;
	virtual uint32 IsEliteMaterialItem(uint8 material_slot) const;
	bool CanClassEquipItem(uint32 item_id);
	bool CanRaceEquipItem(uint32 item_id);
	bool AffectedBySpellExcludingSlot(int slot, int effect);
	virtual bool Death(Mob* killer_mob, int64 damage, uint16 spell_id, EQ::skills::SkillType attack_skill, KilledByTypes killed_by = KilledByTypes::Killed_NPC, bool is_buff_tic = false) = 0;
	virtual void Damage(Mob* from, int64 damage, uint16 spell_id, EQ::skills::SkillType attack_skill,
		bool avoidable = true, int8 buffslot = -1, bool iBuffTic = false, eSpecialAttacks special = eSpecialAttacks::None) = 0;
	void SetHP(int64 hp);
	inline void SetOOCRegen(int64 new_ooc_regen) { ooc_regen = new_ooc_regen; }
	virtual void HealDamage(uint64 ammount, Mob* caster = nullptr, uint16 spell_id = SPELL_UNKNOWN);
	virtual void SetMaxHP() { current_hp = max_hp; }
	virtual inline uint16 GetBaseRace() const { return base_race; }
	virtual inline uint8 GetBaseGender() const { return base_gender; }
	virtual uint16 GetFactionRace();
	virtual inline uint16 GetDeity() const { return deity; }
	virtual uint32 GetDeityBit() { return Deity::GetBitmask(deity); }
	inline uint16 GetRace() const { return race; }
	inline uint16 GetModel() const { return (use_model == 0) ? race : use_model; }
	inline uint8 GetGender() const { return gender; }
	inline uint8 GetTexture() const { return texture; }
	inline uint8 GetHelmTexture() const { return helmtexture; }
	inline uint8 GetHairColor() const { return haircolor; }
	inline uint8 GetBeardColor() const { return beardcolor; }
	inline uint8 GetEyeColor1() const { return eyecolor1; }
	inline uint8 GetEyeColor2() const { return eyecolor2; }
	inline uint8 GetHairStyle() const { return hairstyle; }
	inline uint8 GetLuclinFace() const { return luclinface; }
	inline uint8 GetBeard() const { return beard; }
	inline uint8 GetDrakkinHeritage() const { return drakkin_heritage; }
	inline uint8 GetDrakkinTattoo() const { return drakkin_tattoo; }
	inline uint8 GetDrakkinDetails() const { return drakkin_details; }
	inline void ChangeRace(uint16 in) { race = in; }
	inline void ChangeGender(uint8 in) { gender = in;}
	inline void ChangeTexture(uint8 in) { texture = in; }
	inline void ChangeHelmTexture(uint8 in) { helmtexture = in; }
	inline void ChangeHairColor(uint8 in) { haircolor = in; }
	inline void ChangeBeardColor(uint8 in) { beardcolor = in; }
	inline void ChangeEyeColor1(uint8 in) { eyecolor1 = in; }
	inline void ChangeEyeColor2(uint8 in) { eyecolor2 = in; }
	inline void ChangeHairStyle(uint8 in) { hairstyle = in; }
	inline void ChangeLuclinFace(uint8 in) { luclinface = in; }
	inline void ChangeBeard(uint8 in) { beard = in; }
	inline void ChangeDrakkinHeritage(uint8 in) { drakkin_heritage = in; }
	inline void ChangeDrakkinTattoo(uint8 in) { drakkin_tattoo = in; }
	inline void ChangeDrakkinDetails(uint8 in) { drakkin_details = in; }
	inline uint32 GetArmorTint(uint8 i) const { return armor_tint.Slot[(i < EQ::textures::materialCount) ? i : 0].Color; }
	inline uint8 GetClass() const { return class_; }
	inline uint8 GetLevel() const { return level; }
	inline uint8 GetOrigLevel() const { return orig_level; }
	inline const char* GetName() const { return name; }
	inline const char* GetOrigName() const { return orig_name; }
	inline const char* GetLastName() const { return lastname; }
	inline const uint8 GetPreviousPetOrder() const { return m_previous_pet_order; }
	const char *GetCleanName();
	virtual void SetName(const char *new_name = nullptr) { new_name ? strn0cpy(name, new_name, 64) :
		strn0cpy(name, GetName(), 64); return; };
	inline Mob* GetTarget() const { return target; }
	std::string GetTargetDescription(Mob* target, uint8 description_type = TargetDescriptionType::LCSelf, uint16 entity_id_override = 0);
	virtual void SetTarget(Mob* mob);
	inline bool HasTargetReflection() const { return (target && target != this && target->target == this); }
	virtual inline float GetHPRatio() const { return max_hp == 0 ? 0 : ((float) current_hp / max_hp * 100); }
	virtual inline int GetIntHPRatio() const { return max_hp == 0 ? 0 : static_cast<int>(GetHPRatio()); }
	inline int32 GetAC() const { return AC; }
	inline virtual int32 GetATK() const { return ATK + itembonuses.ATK + spellbonuses.ATK; }
	inline virtual int32 GetATKBonus() const { return itembonuses.ATK + spellbonuses.ATK; }
	inline virtual int32 GetSTR() const { return STR + itembonuses.STR + spellbonuses.STR; }
	inline virtual int32 GetSTA() const { return STA + itembonuses.STA + spellbonuses.STA; }
	inline virtual int32 GetDEX() const { return DEX + itembonuses.DEX + spellbonuses.DEX; }
	inline virtual int32 GetAGI() const { return AGI + itembonuses.AGI + spellbonuses.AGI; }
	inline virtual int32 GetINT() const { return INT + itembonuses.INT + spellbonuses.INT; }
	inline virtual int32 GetWIS() const { return WIS + itembonuses.WIS + spellbonuses.WIS; }
	inline virtual int32 GetCHA() const { return CHA + itembonuses.CHA + spellbonuses.CHA; }
	inline virtual int32 GetHeroicMR() const { return 0; }
	inline virtual int32 GetHeroicFR() const { return 0; }
	inline virtual int32 GetHeroicDR() const { return 0; }
	inline virtual int32 GetHeroicPR() const { return 0; }
	inline virtual int32 GetHeroicCR() const { return 0; }
	inline virtual int32 GetMR() const { return MR + itembonuses.MR + spellbonuses.MR; }
	inline virtual int32 GetFR() const { return FR + itembonuses.FR + spellbonuses.FR; }
	inline virtual int32 GetDR() const { return DR + itembonuses.DR + spellbonuses.DR; }
	inline virtual int32 GetPR() const { return PR + itembonuses.PR + spellbonuses.PR; }
	inline virtual int32 GetCR() const { return CR + itembonuses.CR + spellbonuses.CR; }
	inline virtual int32 GetCorrup() const { return Corrup + itembonuses.Corrup + spellbonuses.Corrup; }
	inline virtual int32 GetPhR() const { return PhR; } // PhR bonuses not implemented yet
	inline StatBonuses GetItemBonuses() const { return itembonuses; }
	inline StatBonuses GetSpellBonuses() const { return spellbonuses; }
	inline StatBonuses GetAABonuses() const { return aabonuses; }
	inline StatBonuses* GetItemBonusesPtr() { return &itembonuses; }
	inline StatBonuses* GetSpellBonusesPtr() { return &spellbonuses; }
	inline StatBonuses* GetAABonusesPtr() { return &aabonuses; }
	inline virtual int32 GetHeroicSTR() const { return 0; }
	inline virtual int32 GetHeroicSTA() const { return 0; }
	inline virtual int32 GetHeroicDEX() const { return 0; }
	inline virtual int32 GetHeroicAGI() const { return 0; }
	inline virtual int32 GetHeroicINT() const { return 0; }
	inline virtual int32 GetHeroicWIS() const { return 0; }
	inline virtual int32 GetHeroicCHA() const { return 0; }
	inline virtual int32 GetMaxSTR() const { return GetSTR(); }
	inline virtual int32 GetMaxSTA() const { return GetSTA(); }
	inline virtual int32 GetMaxDEX() const { return GetDEX(); }
	inline virtual int32 GetMaxAGI() const { return GetAGI(); }
	inline virtual int32 GetMaxINT() const { return GetINT(); }
	inline virtual int32 GetMaxWIS() const { return GetWIS(); }
	inline virtual int32 GetMaxCHA() const { return GetCHA(); }
	inline virtual int32 GetMaxMR() const { return 255; }
	inline virtual int32 GetMaxPR() const { return 255; }
	inline virtual int32 GetMaxDR() const { return 255; }
	inline virtual int32 GetMaxCR() const { return 255; }
	inline virtual int32 GetMaxFR() const { return 255; }
	inline virtual int32 GetDelayDeath() const { return 0; }
	inline int64 GetHP() const { return current_hp; }
	inline int64 GetMaxHP() const { return max_hp; }
	virtual int64 CalcMaxHP();
	virtual int64 CalcHPRegenCap() { return 0; }
	inline int64 GetMaxMana() const { return max_mana; }
	virtual int64 CalcManaRegenCap() { return 0; }
	inline int64 GetMana() const { return current_mana; }
	virtual int64 GetEndurance() const { return 0; }
	virtual int64 GetMaxEndurance() const { return 0; }
	virtual int64 CalcEnduranceRegenCap() { return 0; }
	virtual void SetEndurance(int32 newEnd) { return; }
	int64 GetItemHPBonuses();
	int64 GetSpellHPBonuses();
	const int64& SetMana(int64 amount);
	inline float GetManaRatio() const { return max_mana == 0 ? 100 :
		((static_cast<float>(current_mana) / max_mana) * 100); }
	virtual int64 CalcMaxMana();
	uint32 GetNPCTypeID() const { return npctype_id; }
	inline bool IsZoneController() const { return npctype_id == ZONE_CONTROLLER_NPC_ID; }
	void SetNPCTypeID(uint32 npctypeid) { npctype_id = npctypeid; }
	inline const glm::vec4& GetPosition() const { return m_Position; }
	inline void SetPosition(const float x, const float y, const float z) { m_Position.x = x; m_Position.y = y; m_Position.z = z; }
	inline const float GetX() const { return m_Position.x; }
	inline const float GetY() const { return m_Position.y; }
	inline const float GetZ() const { return m_Position.z; }
	inline const float GetHeading() const { return m_Position.w; }
	inline const glm::vec4& GetRelativePosition() const { return m_RelativePosition; }
	inline void SetRelativePosition(const float x, const float y, const float z) { m_RelativePosition.x = x; m_RelativePosition.y = y; m_RelativePosition.z = z; }
	inline const float GetRelativeX() const { return m_RelativePosition.x; }
	inline const float GetRelativeY() const { return m_RelativePosition.y; }
	inline const float GetRelativeZ() const { return m_RelativePosition.z; }
	inline const float GetRelativeHeading() const { return m_RelativePosition.w; }
	inline const float GetSize() const { return size; }
	inline const float GetBaseSize() const { return base_size; }
	inline const float SetBestZ(float z_coord) const { return z_coord + GetZOffset(); }
	inline const GravityBehavior GetFlyMode() const { return flymode; }
	bool IsBoat() const; // Checks races - used on mob instantiation
	bool GetIsBoat() const { return is_boat; } // Set on instantiation for speed
	void SetIsBoat(bool boat) { is_boat = boat; }
	bool IsControllableBoat() const;
	inline const bool AlwaysAggro() const { return always_aggro; }
	inline int32 GetHeroicStrikethrough() const  { return heroic_strikethrough; }
	inline const bool GetKeepsSoldItems() const { return keeps_sold_items; }
	inline void SetKeepsSoldItems(bool in_keeps_sold_items)  { keeps_sold_items = in_keeps_sold_items; }
	virtual int32 GetHealAmt() const { return 0; }
	virtual int32 GetSpellDmg() const { return 0; }
	void ProcessItemCaps();
	virtual int32 CalcItemATKCap() { return 0; }
	virtual bool IsSitting() const { return false; }

	void CopyHateList(Mob* to);

	//Group
	virtual bool HasRaid() = 0;
	virtual bool HasGroup() = 0;
	virtual Raid* GetRaid() = 0;
	virtual Group* GetGroup() = 0;
	bool IsInGroupOrRaid(Mob* other, bool same_raid_group = false);

	//Faction
	virtual inline int32 GetPrimaryFaction() const { return 0; }

	//Movement
	inline bool IsMoving() const { return moving; }
	virtual void SetMoving(bool move) { moving = move; m_Delta = glm::vec4(); }
	virtual void GoToBind(uint8 bindnum = 0) { }
	virtual void Gate(uint8 bindnum = 0);
	virtual int GetWalkspeed() const { return(_GetWalkSpeed()); }
	virtual int GetRunspeed() const { return(_GetRunSpeed()); }
	int GetBaseRunspeed() const { return base_runspeed; }
	int GetBaseWalkspeed() const { return base_walkspeed; }
	int GetBaseFearSpeed() const { return base_fearspeed; }
	float GetMovespeed() const { return IsRunning() ? GetRunspeed() : GetWalkspeed(); }
	bool IsRunning() const { return m_is_running; }
	void SetRunning(bool val) { m_is_running = val; }
	float GetCurrentSpeed() { return current_speed; }
	virtual void GMMove(float x, float y, float z, float heading = 0.01, bool save_guard_spot = true);
	virtual void GMMove(const glm::vec4 &position, bool save_guard_spot = true);
	void SetDelta(const glm::vec4& delta);
	void MakeSpawnUpdate(PlayerPositionUpdateServer_Struct* spu);
	void SentPositionPacket(float dx, float dy, float dz, float dh, int anim, bool send_to_self = false);
	virtual void StopMoving();
	virtual void StopMoving(float new_heading);
	void SetSpawned() { spawned = true; };
	bool Spawned() { return spawned; };
	virtual bool ShouldISpawnFor(Client *c) { return true; }
	void SetFlyMode(GravityBehavior in_flymode);
	void Teleport(const glm::vec3 &pos);
	void Teleport(const glm::vec4 &pos);
	void TryMoveAlong(float distance, float angle, bool send = true);
	glm::vec4 TryMoveAlong(const glm::vec4 &start, float distance, float angle);
	void ProcessForcedMovement();
	inline void IncDeltaX(float in) { m_Delta.x += in; }
	inline void IncDeltaY(float in) { m_Delta.y += in; }
	inline void IncDeltaZ(float in) { m_Delta.z += in; }
	inline void SetForcedMovement(int in) { ForcedMovement = in; }
	void SetHeading(float iHeading) { m_Position.w = iHeading; }

	//AI
	static uint32 GetLevelCon(uint8 mylevel, uint8 iOtherLevel);
	inline uint32 GetLevelCon(uint8 iOtherLevel) const { return GetLevelCon(GetLevel(), iOtherLevel); }
	void AddToHateList(Mob* other, int64 hate = 0, int64 damage = 0, bool iYellForHelp = true,
		bool bFrenzy = false, bool iBuffTic = false, uint16 spell_id = SPELL_UNKNOWN, bool pet_comand = false);
	bool RemoveFromHateList(Mob* mob);
	void SetHateAmountOnEnt(Mob* other, int64 hate = 0, int64 damage = 0) { hate_list.SetHateAmountOnEnt(other,hate,damage);}
	void HalveAggro(Mob *other) { int64 in_hate = GetHateAmount(other); SetHateAmountOnEnt(other, (in_hate > 1 ? in_hate / 2 : 1)); }
	void DoubleAggro(Mob *other) { int64 in_hate = GetHateAmount(other); SetHateAmountOnEnt(other, (in_hate ? in_hate * 2 : 1)); }
	int64 GetHateAmount(Mob* tmob, bool is_dam = false) { return hate_list.GetEntHateAmount(tmob,is_dam);}
	int64 GetDamageAmount(Mob* tmob) { return hate_list.GetEntHateAmount(tmob, true);}
	int GetHateRatio(Mob *first, Mob *with) { return hate_list.GetHateRatio(first, with); }
	Mob* GetHateTop() { return hate_list.GetMobWithMostHateOnList(this);}
	Bot* GetHateTopBot() { return hate_list.GetMobWithMostHateOnList(this, nullptr, false, EntityFilterType::Bots)->CastToBot();}
	Client* GetHateTopClient() { return hate_list.GetMobWithMostHateOnList(this, nullptr, false, EntityFilterType::Clients)->CastToClient();}
	NPC* GetHateTopNPC() { return hate_list.GetMobWithMostHateOnList(this, nullptr, false, EntityFilterType::NPCs)->CastToNPC();}
	Mob* GetSecondaryHate(Mob *skip) { return hate_list.GetMobWithMostHateOnList(this, skip); }
	Mob* GetHateDamageTop(Mob* other) { return hate_list.GetDamageTopOnHateList(other);}
	Mob* GetHateRandom() { return hate_list.GetRandomMobOnHateList(); }
	Bot* GetHateRandomBot() { return hate_list.GetRandomMobOnHateList(EntityFilterType::Bots)->CastToBot(); }
	Client* GetHateRandomClient() { return hate_list.GetRandomMobOnHateList(EntityFilterType::Clients)->CastToClient(); }
	NPC* GetHateRandomNPC() { return hate_list.GetRandomMobOnHateList(EntityFilterType::NPCs)->CastToNPC(); }
	Mob* GetHateMost() { return hate_list.GetMobWithMostHateOnList();}
	Mob* GetHateClosest(bool skip_mezzed = false) { return hate_list.GetClosestEntOnHateList(this, skip_mezzed); }
	Bot* GetHateClosestBot(bool skip_mezzed = false) { return hate_list.GetClosestEntOnHateList(this, skip_mezzed, EntityFilterType::Bots)->CastToBot(); }
	Client* GetHateClosestClient(bool skip_mezzed = false) { return hate_list.GetClosestEntOnHateList(this, skip_mezzed, EntityFilterType::Clients)->CastToClient(); }
	NPC* GetHateClosestNPC(bool skip_mezzed = false) { return hate_list.GetClosestEntOnHateList(this, skip_mezzed, EntityFilterType::NPCs)->CastToNPC(); }
	bool IsEngaged() { return(!hate_list.IsHateListEmpty()); }
	inline uint32 GetHateListCount(HateListCountType count_type = HateListCountType::All) { return hate_list.GetHateListCount(count_type); }
	bool HasPrimaryAggro() { return PrimaryAggro; }
	bool HasAssistAggro() { return AssistAggro; }
	void SetPrimaryAggro(bool value) { PrimaryAggro = value; if (value) AssistAggro = false; }
	void SetAssistAggro(bool value) { AssistAggro = value; if (PrimaryAggro) AssistAggro = false; }
	bool HateSummon();
	void FaceTarget(Mob* mob_to_face = 0);
	void WipeHateList(bool npc_only = false);
	void AddFeignMemory(Mob* attacker);
	void RemoveFromFeignMemory(Mob* attacker);
	void ClearFeignMemory();
	bool IsOnFeignMemory(Mob *attacker) const;
	void PrintHateListToClient(Client *who) { hate_list.PrintHateListToClient(who); }
	std::list<struct_HateList*>& GetHateList() { return hate_list.GetHateList(); }
	bool CheckLosFN(Mob* other);
	bool CheckLosFN(float posX, float posY, float posZ, float mobSize);
	static bool CheckLosFN(glm::vec3 posWatcher, float sizeWatcher, glm::vec3 posTarget, float sizeTarget);
	virtual bool CheckWaterLoS(Mob* m);
	bool CheckPositioningLosFN(Mob* other, float posX, float posY, float posZ);
	bool CheckDoorLoSCheat(Mob* other); //door skipping checks for LoS
	bool CheckLosCheatExempt(Mob* other); //exemptions to bypass los
	bool DoLosChecks(Mob* other);
	inline void SetLastLosState(bool value) { last_los_check = value; }
	inline bool CheckLastLosState() const { return last_los_check; }
	std::string GetMobDescription();

	std::list<struct_HateList*> GetFilteredHateList(
		EntityFilterType filter_type = EntityFilterType::All,
		uint32 distance = 0
	) {
		return hate_list.GetFilteredHateList(filter_type, distance);
	}

	void DamageHateList(
		int64 damage,
		uint32 distance = 0,
		EntityFilterType filter_type = EntityFilterType::All,
		bool is_percentage = false
	) {
		hate_list.DamageHateList(damage, distance, filter_type, is_percentage);
	}

	void DamageArea(
		int64 damage,
		uint32 distance = 0,
		EntityFilterType filter_type = EntityFilterType::All,
		bool is_percentage = false
	) {
		entity_list.DamageArea(this, damage, distance, filter_type, is_percentage);
	}

	//Quest
	void CameraEffect(uint32 duration, float intensity, Client *c = nullptr, bool global = false);
	inline bool GetQglobal() const { return qglobal; }

	//Other Packet
	void CreateDespawnPacket(EQApplicationPacket* app, bool Decay);
	void CreateHorseSpawnPacket(EQApplicationPacket* app, Mob* m = nullptr);
	void CreateSpawnPacket(EQApplicationPacket* app, Mob* ForWho = 0);
	static void CreateSpawnPacket(EQApplicationPacket* app, NewSpawn_Struct* ns);
	virtual void FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho);
	void CreateHPPacket(EQApplicationPacket* app);
	void SendHPUpdate(bool force_update_all = false);
	virtual void ResetHPUpdateTimer() {}; // does nothing
	static void SetSpawnLastNameByClass(NewSpawn_Struct* ns);
	void SendRename(Mob* sender, const char* old_name, const char* new_name);

	//Util
	static uint32 RandomTimer(int min, int max);
	static uint8 GetDefaultGender(uint16 in_race, uint8 in_gender = 0xFF);
	EQ::skills::SkillType GetSkillByItemType(int ItemType);
	uint8 GetItemTypeBySkill(EQ::skills::SkillType skill);
	virtual void MakePet(uint16 spell_id, const char* pettype, const char *petname = nullptr);
	virtual void MakePoweredPet(uint16 spell_id, const char* pettype, int16 petpower, const char *petname = nullptr, float in_size = 0.0f);
	bool IsWarriorClass() const;
	bool IsIntelligenceCasterClass() const;
	bool IsPureMeleeClass() const;
	bool IsWisdomCasterClass() const;
	uint8 GetArchetype() const;
	const std::string GetArchetypeName();
	void SetZone(uint32 zone_id, uint32 instance_id);
	void SendStatsWindow(Client* c, bool use_window);
	void ShowStats(Client* client);
	void ShowBuffs(Client* c);
	bool PlotPositionAroundTarget(Mob* target, float &x_dest, float &y_dest, float &z_dest, bool lookForAftArc = true);

	virtual int GetKillExpMod() const { return 100; }

	// aura functions
	void MakeAura(uint16 spell_id);
	inline int GetAuraSlots() { return 1 + aabonuses.aura_slots + itembonuses.aura_slots + spellbonuses.aura_slots; }
	inline int GetTrapSlots() { return 1 + aabonuses.trap_slots + itembonuses.trap_slots + spellbonuses.trap_slots; }
	inline bool HasFreeAuraSlots() { return aura_mgr.count < GetAuraSlots(); }
	inline bool HasFreeTrapSlots() { return trap_mgr.count < GetTrapSlots(); }
	void AddAura(Aura *aura, AuraRecord &record);
	void AddTrap(Aura *aura, AuraRecord &record);
	bool CanSpawnAura(bool trap);
	void RemoveAura(int spawn_id, bool skip_strip = false, bool expired = false);
	void RemoveAllAuras();
	inline AuraMgr &GetAuraMgr() { return aura_mgr; } // mainly used for zone db loading/saving

	//Procs
	void TriggerDefensiveProcs(Mob *on, uint16 hand = EQ::invslot::slotPrimary, bool FromSkillProc = false, int64 damage = 0);
	bool AddRangedProc(uint16 spell_id, uint16 iChance = 3, uint16 base_spell_id = SPELL_UNKNOWN, uint32 proc_reuse_time = 0);
	bool RemoveRangedProc(uint16 spell_id, bool bAll = false);
	bool HasRangedProcs() const;
	bool AddDefensiveProc(uint16 spell_id, uint16 iChance = 3, uint16 base_spell_id = SPELL_UNKNOWN, uint32 proc_reuse_time = 0);
	bool RemoveDefensiveProc(uint16 spell_id, bool bAll = false);
	bool HasDefensiveProcs() const;
	bool HasSkillProcs() const;
	bool HasSkillProcSuccess() const;
	bool AddProcToWeapon(uint16 spell_id, bool bPerma = false, uint16 iChance = 3, uint16 base_spell_id = SPELL_UNKNOWN, int level_override = -1, uint32 proc_reuse_time = 0);
	bool RemoveProcFromWeapon(uint16 spell_id, bool bAll = false);
	bool HasProcs() const;
	bool IsCombatProc(uint16 spell_id);

	//More stuff to sort:
	virtual bool IsRaidTarget() const { return false; };
	virtual bool IsAttackAllowed(Mob *target, bool isSpellAttack = false);
	bool IsTargeted() const { return (targeted > 0); }
	inline void IsTargeted(int in_tar) { targeted += in_tar; if(targeted < 0) targeted = 0;}
	void SetFollowID(uint32 id) { follow_id = id; }
	void SetFollowDistance(uint32 dist) { follow_dist = dist; }
	void SetFollowCanRun(bool v) { follow_run = v; }
	uint32 GetFollowID() const { return follow_id; }
	uint32 GetFollowDistance() const { return follow_dist; }
	bool GetFollowCanRun() const { return follow_run; }
	inline bool IsRareSpawn() const { return rare_spawn; }
	inline void SetRareSpawn(bool in) { rare_spawn = in; }

	virtual void Message(uint32 type, const char* message, ...) { }
	virtual void MessageString(uint32 type, uint32 string_id, uint32 distance = 0) { }
	virtual void MessageString(uint32 type, uint32 string_id, const char* message, const char* message2 = 0,
		const char* message3 = 0, const char* message4 = 0, const char* message5 = 0, const char* message6 = 0,
		const char* message7 = 0, const char* message8 = 0, const char* message9 = 0, uint32 distance = 0) { }
	virtual void FilteredMessageString(Mob *sender, uint32 type, eqFilterType filter, uint32 string_id) { }
	virtual void FilteredMessageString(Mob *sender, uint32 type, eqFilterType filter,
			uint32 string_id, const char *message1, const char *message2 = nullptr,
			const char *message3 = nullptr, const char *message4 = nullptr,
			const char *message5 = nullptr, const char *message6 = nullptr,
			const char *message7 = nullptr, const char *message8 = nullptr,
			const char *message9 = nullptr) { }
	void Say(const char *format, ...);
	void SayString(uint32 string_id, const char *message3 = 0, const char *message4 = 0, const char *message5 = 0,
		const char *message6 = 0, const char *message7 = 0, const char *message8 = 0, const char *message9 = 0);
	void SayString(uint32 type, uint32 string_id, const char *message3 = 0, const char *message4 = 0, const char *message5 = 0,
		const char *message6 = 0, const char *message7 = 0, const char *message8 = 0, const char *message9 = 0);
	void SayString(Client *to, uint32 string_id, const char *message3 = 0, const char *message4 = 0, const char *message5 = 0,
		const char *message6 = 0, const char *message7 = 0, const char *message8 = 0, const char *message9 = 0);
	void SayString(Client *to, uint32 type, uint32 string_id, const char *message3 = 0, const char *message4 = 0, const char *message5 = 0,
		const char *message6 = 0, const char *message7 = 0, const char *message8 = 0, const char *message9 = 0);
	void Shout(const char *format, ...);
	void Emote(const char *format, ...);
	void QuestJournalledSay(Client *QuestInitiator, const char *str, Journal::Options &opts);
	const int GetItemStat(uint32 item_id, std::string identifier);

	int64 CalcFocusEffect(focusType type, uint16 focus_id, uint16 spell_id, bool best_focus=false, uint16 casterid = 0, Mob *caster = nullptr);
	uint8 IsFocusEffect(uint16 spellid, int effect_index, bool AA=false,uint32 aa_effect=0);
	void SendIllusionPacket(const AppearanceStruct& a);
	void CloneAppearance(Mob* other, bool clone_name = false);
	void SetFaceAppearance(const FaceChange_Struct& face, bool skip_sender = false);
	bool RandomizeFeatures(bool send_illusion = true, bool set_variables = true);
	virtual void Stun(int duration);
	virtual void UnStun();
	inline void Silence(bool newval) { silenced = newval; }
	inline void Amnesia(bool newval) { amnesiad = newval; }
	void TemporaryPets(uint16 spell_id, Mob *target, const char *name_override = nullptr, uint32 duration_override = 0, bool followme=true, bool sticktarg=false, uint16 *controlled_pet_id = nullptr);
	void TypesTemporaryPets(uint32 typesid, Mob *target, const char *name_override = nullptr, uint32 duration_override = 0, bool followme=true, bool sticktarg=false);
	void WakeTheDead(uint16 spell_id, Corpse *corpse_to_use, Mob *target, uint32 duration);
	void Spin();
	void Kill();
	bool PassCharismaCheck(Mob* caster, uint16 spell_id);
	bool TryDeathSave();
	bool TryDivineSave();
	void TryTriggerOnCastFocusEffect(focusType type, uint16 spell_id);
	bool TryTriggerOnCastProc(uint16 focusspellid, uint16 spell_id, uint16 proc_spellid);
	bool TrySpellTrigger(Mob *target, uint32 spell_id, int effect);
	void TryTriggerOnCastRequirement();
	void TryTwincast(Mob *caster, Mob *target, uint32 spell_id);
	void TrySympatheticProc(Mob *target, uint32 spell_id);
	uint16 GetSympatheticFocusEffect(focusType type, uint16 spell_id);
	bool TryFadeEffect(int slot);
	void DispelMagic(Mob* casterm, uint16 spell_id, int effect_value);
	bool TrySpellEffectResist(uint16 spell_id);
	int32 GetVulnerability(Mob *caster, uint32 spell_id, uint32 ticsremaining, bool from_buff_tic = false);
	int64 GetFcDamageAmtIncoming(Mob *caster, int32 spell_id, bool from_buff_tic = false);
	int64 GetFocusIncoming(focusType type, int effect, Mob *caster, uint32 spell_id); //**** This can be removed when bot healing focus code is updated ****
	int32 GetSkillDmgTaken(const EQ::skills::SkillType skill_used, ExtraAttackOptions *opts = nullptr);
	int32 GetPositionalDmgTaken(Mob *attacker);
	int32 GetPositionalDmgTakenAmt(Mob *attacker);
	void DoKnockback(Mob *caster, uint32 push_back, uint32 push_up);
	int16 CalcResistChanceBonus();
	int16 CalcFearResistChance();
	void TrySpellOnKill(uint8 level, uint16 spell_id);
	bool TrySpellOnDeath();
	void CastOnCurer(uint32 spell_id);
	void CastOnCure(uint32 spell_id);
	void CastOnNumHitFade(uint32 spell_id);
	void SlowMitigation(Mob* caster);
	int16 GetCritDmgMod(uint16 skill, Mob* owner = nullptr);
	int16 GetMeleeDamageMod_SE(uint16 skill);
	int16 GetMeleeMinDamageMod_SE(uint16 skill);
	int16 GetCrippBlowChance();
	int16 GetMeleeDmgPositionMod(Mob* defender);
	int16 GetSkillReuseTime(uint16 skill);
	int GetCriticalChanceBonus(uint16 skill);
	int GetSkillDmgAmt(int skill_id);
	int16 GetPositionalDmgAmt(Mob* defender);
	inline bool CanBlockSpell() const { return(spellbonuses.FocusEffects[focusBlockNextSpell]); }
	bool DoHPToManaCovert(int32 mana_cost = 0);
	int8 GetDecayEffectValue(uint16 spell_id, uint16 spelleffect);
	int64 GetExtraSpellAmt(uint16 spell_id, int64 extra_spell_amt, int64 base_spell_dmg);
	void MeleeLifeTap(int64 damage);
	bool PassCastRestriction(int value);
	void SendCastRestrictionMessage(int requirement_id, bool is_target_requirement = true, bool is_discipline = false);
	bool ImprovedTaunt();
	bool TryRootFadeByDamage(int buffslot, Mob* attacker);
	float GetSlowMitigation() const { return slow_mitigation; }
	void CalcSpellPowerDistanceMod(uint16 spell_id, float range, Mob* caster = nullptr);
	inline int16 GetSpellPowerDistanceMod() const { return SpellPowerDistanceMod; };
	inline void SetSpellPowerDistanceMod(int16 value) { SpellPowerDistanceMod = value; };
	int32 GetSpellStat(uint32 spell_id, const char *identifier, uint8 slot = 0);
	bool HarmonySpellLevelCheck(int32 spell_id, Mob* target = nullptr);
	bool PassCharmTargetRestriction(Mob *target);
	bool CanFocusUseRandomEffectivenessByType(focusType type);
	int GetFocusRandomEffectivenessValue(int focus_base, int focus_base2, bool best_focus = 0);
	int GetHealRate() const { return itembonuses.HealRate + spellbonuses.HealRate + aabonuses.HealRate; }
	int GetMemoryBlurChance(int base_chance);
	inline bool HasBaseEffectFocus() const { return (spellbonuses.FocusEffects[focusFcBaseEffects] || aabonuses.FocusEffects[focusFcBaseEffects] || itembonuses.FocusEffects[focusFcBaseEffects]); }
	int32 GetDualWieldingSameDelayWeapons() const { return dw_same_delay; }
	inline void SetDualWieldingSameDelayWeapons(int32 val) { dw_same_delay = val; }
	bool IsTargetedFocusEffect(int focus_type);
	bool HasPersistDeathIllusion(int32 spell_id);
	void DoShieldDamageOnShielderSpellEffect(Mob* shield_target, int64 hit_damage_done, EQ::skills::SkillType skillInUse);


	bool TryDoubleMeleeRoundEffect();
	bool GetUseDoubleMeleeRoundDmgBonus() const { return use_double_melee_round_dmg_bonus; }
	inline void SetUseDoubleMeleeRoundDmgBonus(bool val) { use_double_melee_round_dmg_bonus = val; }

	void CastSpellOnLand(Mob* caster, int32 spell_id);

	bool IsFocusProcLimitTimerActive(int32 focus_spell_id);
	void SetFocusProcLimitTimer(int32 focus_spell_id, uint32 focus_reuse_time);
	bool IsProcLimitTimerActive(int32 base_spell_id, uint32 proc_reuse_time, int proc_type);
	void SetProcLimitTimer(int32 base_spell_id, uint32 proc_reuse_time, int proc_type);

	void VirusEffectProcess();
	void SpreadVirusEffect(int32 spell_id, uint32 caster_id, int32 buff_tics_remaining);

	void ModSkillDmgTaken(EQ::skills::SkillType skill_num, int value);
	int16 GetModSkillDmgTaken(const EQ::skills::SkillType skill_num);
	void ModVulnerability(uint8 resist, int16 value);
	int16 GetModVulnerability(const uint8 resist);

	void SetAllowBeneficial(bool value) { m_AllowBeneficial = value; }
	bool GetAllowBeneficial() { if (m_AllowBeneficial || GetSpecialAbility(SpecialAbility::AllowBeneficial)){return true;} return false; }
	void SetDisableMelee(bool value) { m_DisableMelee = value; }
	bool IsMeleeDisabled() { if (m_DisableMelee || GetSpecialAbility(SpecialAbility::DisableMelee)){return true;} return false; }

	bool IsOffHandAtk() const { return offhand; }
	inline void OffHandAtk(bool val) { offhand = val; }

	void SetFlurryChance(uint8 value) { SetSpecialAbilityParam(SpecialAbility::Flurry, 0, value); }
	uint8 GetFlurryChance() { return GetSpecialAbilityParam(SpecialAbility::Flurry, 0); }

	static uint32 GetAppearanceValue(EmuAppearance in_appearance);
	void SendAppearancePacket(uint32 type, uint32 value, bool whole_zone = true, bool ignore_self = false, Client* target = nullptr);
	void SetAppearance(EmuAppearance app, bool ignore_self = true);
	inline EmuAppearance GetAppearance() const { return _appearance; }
	inline const int GetAnimation() const { return animation; }
	inline void SetAnimation(int a) { animation = a; }
	inline const uint8 GetRunAnimSpeed() const { return pRunAnimSpeed; }
	inline void SetRunAnimSpeed(int8 in) { pRunAnimSpeed = in; }
	bool IsDestructibleObject() { return destructibleobject; }
	void SetDestructibleObject(bool in) { destructibleobject = in; }

	inline uint8 GetInnateLightType() { return m_Light.Type[EQ::lightsource::LightInnate]; }
	inline uint8 GetEquipmentLightType() { return m_Light.Type[EQ::lightsource::LightEquipment]; }
	inline uint8 GetSpellLightType() { return m_Light.Type[EQ::lightsource::LightSpell]; }

	virtual void UpdateEquipmentLight() { m_Light.Type[EQ::lightsource::LightEquipment] = 0; m_Light.Level[EQ::lightsource::LightEquipment] = 0; }
	inline void SetSpellLightType(uint8 light_type) { m_Light.Type[EQ::lightsource::LightSpell] = (light_type & 0x0F); m_Light.Level[EQ::lightsource::LightSpell] = EQ::lightsource::TypeToLevel(m_Light.Type[EQ::lightsource::LightSpell]); }

	void SendWearChangeAndLighting(int8 last_texture);
	inline uint8 GetActiveLightType() { return m_Light.Type[EQ::lightsource::LightActive]; }
	bool UpdateActiveLight(); // returns true if change, false if no change
	uint32 GetNpcTintId() { return m_npc_tint_id; }

	EQ::LightSourceProfile* GetLightProfile() { return &m_Light; }

	Mob* GetPet();
	void SetPet(Mob* newpet);
	virtual Mob* GetOwner();
	virtual Mob* GetOwnerOrSelf();
	Mob* GetUltimateOwner();
	void SetPetID(uint16 NewPetID);
	inline uint16 GetPetID() const { return petid; }
	inline uint8 GetPetType() const { return type_of_pet; }
	void SetPetType(uint8 pet_type) { type_of_pet = pet_type; }
	inline int16 GetPetPower() const { return (petpower < 0) ? 0 : petpower; }
	void SetPetPower(int16 p) { if (p < 0) petpower = 0; else petpower = p; }
	bool IsFamiliar() const { return type_of_pet == PetType::Familiar; }
	bool IsAnimation() const { return type_of_pet == PetType::Animation; }
	bool IsCharmed() const { return type_of_pet == PetType::Charmed; }
	bool IsTargetLockPet() const { return type_of_pet == PetType::TargetLock; }
	inline uint32 GetPetTargetLockID() { return pet_targetlock_id; };
	inline void SetPetTargetLockID(uint32 value) { pet_targetlock_id = value; };
	void SetOwnerID(uint16 new_owner_id);
	inline uint16 GetOwnerID() const { return ownerid; }
	inline virtual bool HasOwner() { if (!GetOwnerID()){ return false; } return entity_list.GetMob(GetOwnerID()) != 0; }
	inline virtual bool IsPet() { return HasOwner() && !IsMerc(); }
	bool HasPet() const;
	virtual bool IsCharmedPet() { return IsPet() && IsCharmed(); }
	inline bool HasTempPetsActive() const { return(hasTempPet); }
	inline void SetTempPetsActive(bool i) { hasTempPet = i; }
	inline int16 GetTempPetCount() const { return count_TempPet; }
	inline void SetTempPetCount(int16 i) { count_TempPet = i; }
	bool HasPetAffinity() { if (aabonuses.GivePetGroupTarget || itembonuses.GivePetGroupTarget || spellbonuses.GivePetGroupTarget) return true; return false; }
	inline bool IsPetOwnerBot() const { return pet_owner_bot; }
	inline void SetPetOwnerBot(bool b) { pet_owner_bot = b; }
	inline bool IsPetOwnerClient() const { return pet_owner_client; }
	inline void SetPetOwnerClient(bool b) { pet_owner_client = b; }
	inline bool IsPetOwnerNPC() const { return pet_owner_npc; }
	inline void SetPetOwnerNPC(bool b) { pet_owner_npc = b; }
	inline bool IsPetOwnerOfClientBot() const { return pet_owner_bot || pet_owner_client; }
	inline bool IsTempPet() const { return _IsTempPet; }
	inline void SetTempPet(bool value) { _IsTempPet = value; }
	inline bool IsHorse() { return is_horse; }
	int GetPetAvoidanceBonusFromOwner();
	int GetPetACBonusFromOwner();
	int GetPetATKBonusFromOwner();

	inline const uint8 GetBodyType() const { return bodytype; }
	inline const uint8 GetOrigBodyType() const { return orig_bodytype; }
	void SetBodyType(uint8 new_body, bool overwrite_orig);

	bool invulnerable;
	bool qglobal;

	inline std::vector<uint32> GetBotAttackFlags() { return bot_attack_flags; }
	inline void SetBotAttackFlag(uint32 value) { bot_attack_flags.push_back(value); }
	inline void ClearBotAttackFlags() { bot_attack_flags.clear(); }
	bool HasBotAttackFlag(Mob* tar);

	virtual void SetAttackTimer();
	inline void SetInvul(bool invul) { invulnerable=invul; }
	inline bool GetInvul() { return invulnerable; }
	void SetExtraHaste(int haste, bool need_to_save = true);
	inline int GetExtraHaste() { return extra_haste; }
	virtual int GetHaste();
	int32 GetMeleeMitigation();

	uint8 GetWeaponDamageBonus(const EQ::ItemData* weapon, bool offhand = false);
	const DamageTable &GetDamageTable() const;
	int GetMobFixedOffenseSkill();
	int GetMobFixedWeaponSkill();
	void ApplyDamageTable(DamageHitInfo &hit);
	virtual int GetHandToHandDamage(void);

	bool CanThisClassDoubleAttack(void) const;
	bool CanThisClassTripleAttack() const;
	bool CanThisClassDualWield(void) const;
	bool CanThisClassRiposte(void) const;
	bool CanThisClassDodge(void) const;
	bool CanThisClassParry(void) const;
	bool CanThisClassBlock(void) const;

	int GetHandToHandDelay(void);
	uint32 GetClassLevelFactor();
	void Mesmerize();
	inline bool IsMezzed() const { return mezzed; }
	inline bool IsStunned() const { return stunned; }
	inline bool IsSilenced() const { return silenced; }
	inline bool IsAmnesiad() const { return amnesiad; }

	int64 ReduceDamage(int64 damage);
	int64 AffectMagicalDamage(int64 damage, uint16 spell_id, const bool iBuffTic, Mob* attacker);
	int64 ReduceAllDamage(int64 damage);

	void DoSpecialAttackDamage(Mob *who, EQ::skills::SkillType skill, int base_damage, int min_damage = 0, int32 hate_override = -1, int ReuseTime = 10);
	void DoThrowingAttackDmg(Mob* other, const EQ::ItemInstance* RangeWeapon = nullptr, const EQ::ItemData* AmmoItem = nullptr, int32 weapon_damage = 0, int16 chance_mod = 0, int16 focus = 0, int ReuseTime = 0, uint32 range_id = 0, int AmmoSlot = 0, float speed = 4.0f, bool DisableProcs = false);
	void DoMeleeSkillAttackDmg(Mob* other, int32 weapon_damage, EQ::skills::SkillType skillinuse, int16 chance_mod = 0, int16 focus = 0, bool can_riposte = false, int ReuseTime = 0);
	void DoArcheryAttackDmg(Mob* other, const EQ::ItemInstance* RangeWeapon = nullptr, const EQ::ItemInstance* Ammo = nullptr, int32 weapon_damage = 0, int16 chance_mod = 0, int16 focus = 0, int ReuseTime = 0, uint32 range_id = 0, uint32 ammo_id = 0, const EQ::ItemData *AmmoItem = nullptr, int AmmoSlot = 0, float speed = 4.0f, bool DisableProcs = false);
	bool TryProjectileAttack(Mob* other, const EQ::ItemData *item, EQ::skills::SkillType skillInUse, uint64 weapon_dmg, const EQ::ItemInstance* RangeWeapon, const EQ::ItemInstance* Ammo, int AmmoSlot, float speed, bool DisableProcs = false);
	void ProjectileAttack();
	inline bool HasProjectileAttack() const { return ActiveProjectileATK; }
	inline void SetProjectileAttack(bool value) { ActiveProjectileATK = value; }
	float GetRangeDistTargetSizeMod(Mob* other);
	bool CanDoSpecialAttack(Mob *other);
	bool Flurry(ExtraAttackOptions *opts);
	bool Rampage(ExtraAttackOptions *opts);
	bool AddRampage(Mob*);
	void ClearRampage();
	void RemoveFromRampageList(Mob* mob, bool remove_feigned = false);
	void SetBottomRampageList();
	void SetTopRampageList();
	void AreaRampage(ExtraAttackOptions *opts);
	inline bool IsSpecialAttack(eSpecialAttacks in) { return m_specialattacks == in; }

	void StartEnrage();
	void ProcessEnrage();
	bool IsEnraged();
	void Taunt(NPC *who, bool always_succeed, int chance_bonus = 0, bool from_spell = false, int32 bonus_hate = 0);

	virtual void AI_Init();
	virtual void AI_Start(uint32 iMoveDelay = 0);
	virtual void AI_Stop();
	virtual void AI_ShutDown();
	virtual void AI_Process();

	bool ClearEntityVariables();
	bool DeleteEntityVariable(std::string variable_name);
	std::string GetEntityVariable(std::string variable_name);
	std::vector<std::string> GetEntityVariables();
	void SetEntityVariable(std::string variable_name, std::string variable_value);
	bool EntityVariableExists(std::string variable_name);

	void AI_Event_Engaged(Mob* attacker, bool yell_for_help = true);
	void AI_Event_NoLongerEngaged();

	FACTION_VALUE GetSpecialFactionCon(Mob* iOther);
	inline const bool IsAIControlled() const { return pAIControlled; }
	inline const float GetAggroRange() const { return (spellbonuses.AggroRange == -1) ? pAggroRange : spellbonuses.AggroRange; }
	inline const float GetAssistRange() const { return (spellbonuses.AssistRange == -1) ? pAssistRange : spellbonuses.AssistRange; }


	void SetPetOrder(uint8 pet_order);
	inline const uint8 GetPetOrder() const { return m_pet_order; }
	inline void SetHeld(bool nState) { held = nState; }
	inline const bool IsHeld() const { return held; }
	inline void SetGHeld(bool nState) { gheld = nState; }
	inline const bool IsGHeld() const { return gheld; }
	inline void SetNoCast(bool nState) { nocast = nState; }
	inline const bool IsNoCast() const { return nocast; }
	inline void SetFocused(bool nState) { focused = nState; }
	inline const bool IsFocused() const { return focused; }
	inline void SetPetStop(bool nState) { pet_stop = nState; }
	inline const bool IsPetStop() const { return pet_stop; }
	inline void SetPetRegroup(bool nState) { pet_regroup = nState; }
	inline const bool IsPetRegroup() const { return pet_regroup; }
	inline const bool IsRoamer() const { return roamer; }
	inline const int GetWanderType() const { return wandertype; }
	inline const bool IsRooted() const { return rooted || permarooted; }
	inline const bool IsPermaRooted() const { return permarooted; }
	int GetSnaredAmount();
	inline const bool IsPseudoRooted() const { return pseudo_rooted; }
	inline void SetPseudoRoot(bool prState) { pseudo_rooted = prState; }

	int GetCurWp() { return cur_wp; }

	//old fear function
	//void SetFeared(Mob *caster, uint32 duration, bool flee = false);
	int GetFearSpeed() { return _GetFearSpeed(); }
	bool IsFeared() { return (spellbonuses.IsFeared || flee_mode); } // This returns true if the mob is feared or fleeing due to low HP
	inline void StartFleeing() { flee_mode = true; CalculateNewFearpoint(); }
	void StopFleeing();
	inline bool IsFleeing() { return flee_mode; }
	void ProcessFlee();
	void CheckFlee();
	void FleeInfo(Mob* client);
	int GetFleeRatio(Mob* other = nullptr);
	inline bool IsBlind() { return spellbonuses.IsBlind; }

	inline bool			CheckAggro(Mob* other) {return hate_list.IsEntOnHateList(other);}
	float				CalculateHeadingToTarget(float in_x, float in_y) { return HeadingAngleToMob(in_x, in_y); }
	virtual void		WalkTo(float x, float y, float z);
	virtual void		RunTo(float x, float y, float z);
	void				NavigateTo(float x, float y, float z);
	void				RotateTo(float new_heading);
	void				RotateToWalking(float new_heading);
	void				RotateToRunning(float new_heading);
	void				StopNavigation();
	float				CalculateDistance(float x, float y, float z);
	float				CalculateDistance(Mob* mob);
	float				GetGroundZ(float new_x, float new_y, float z_offset=0.0);
	void				SendTo(float new_x, float new_y, float new_z);
	void				SendToFixZ(float new_x, float new_y, float new_z);
	float				GetZOffset() const;
	float               GetDefaultRaceSize(int race_id = -1, int gender_id = -1) const;
	void 				FixZ(int32 z_find_offset = 5, bool fix_client_z = false);
	float				GetFixedZ(const glm::vec3 &destination, int32 z_find_offset = 5);
	virtual int			GetStuckBehavior() const { return 0; }

	void NPCSpecialAttacks(const char *parse, int permtag, bool reset = true, bool remove = false);
	inline uint32 DontHealMeBefore() const { return m_dont_heal_me_before; }
	inline uint32 DontBuffMeBefore() const { return m_dont_buff_me_before; }
	inline uint32 DontDotMeBefore() const { return m_dont_dot_me_before; }
	inline uint32 DontRootMeBefore() const { return m_dont_root_me_before; }
	inline uint32 DontSnareMeBefore() const { return m_dont_snare_me_before; }
	inline uint32 DontCureMeBefore() const { return m_dont_cure_me_before; }

	void SetDontRootMeBefore(uint32 time) { m_dont_root_me_before = time; }
	void SetDontHealMeBefore(uint32 time) { m_dont_heal_me_before = time; }
	void SetDontBuffMeBefore(uint32 time) { m_dont_buff_me_before = time; }
	void SetDontDotMeBefore(uint32 time) { m_dont_dot_me_before = time; }
	void SetDontSnareMeBefore(uint32 time) { m_dont_snare_me_before = time; }
	void SetDontCureMeBefore(uint32 time) { m_dont_cure_me_before = time; }

	// calculate interruption of spell via movement of mob
	void SaveSpellLoc() { m_SpellLocation = glm::vec3(m_Position); }
	inline float GetSpellX() const {return m_SpellLocation.x;}
	inline float GetSpellY() const {return m_SpellLocation.y;}
	inline float GetSpellZ() const {return m_SpellLocation.z;}
	inline bool IsGrouped() const { return isgrouped; }
	void SetGrouped(bool v);
	inline bool IsRaidGrouped() const { return israidgrouped; }
	void SetRaidGrouped(bool v);
	inline uint16 IsLooting() const { return entity_id_being_looted; }
	void SetLooting(uint16 val) { entity_id_being_looted = val; }

	bool CheckWillAggro(Mob *mob);
	bool IsPetAggroExempt(Mob *pet_owner);

	void InstillDoubt(Mob *who);
	bool Charmed() const { return type_of_pet == PetType::Charmed; }
	static uint32 GetLevelHP(uint8 tlevel);
	uint32 GetZoneID() const; //for perl
	uint16 GetInstanceVersion() const; //for perl
	virtual int32 CheckAggroAmount(uint16 spell_id, Mob *target, bool isproc = false);
	virtual int32 CheckHealAggroAmount(uint16 spell_id, Mob *target, uint32 heal_possible = 0);

	//uint32 GetInstrumentMod(uint16 spell_id) const;
	uint32 GetInstrumentMod(uint16 spell_id);
	int64 CalcSpellEffectValue(uint16 spell_id, int effect_id, int caster_level = 1, uint32 instrument_mod = 10, Mob *caster = nullptr, int ticsremaining = 0,uint16 casterid=0);
	int64 CalcSpellEffectValue_formula(uint32 formula, int64 base_value, int64 max_value, int caster_level, uint16 spell_id, int ticsremaining = 0);
	virtual int CheckStackConflict(uint16 spellid1, int caster_level1, uint16 spellid2, int caster_level2, Mob* caster1 = nullptr, Mob* caster2 = nullptr, int buffslot = -1);
	uint32 GetCastedSpellInvSlot() const { return casting_spell_inventory_slot; }

	// HP Event
	inline int GetNextHPEvent() const { return nexthpevent; }
	void SetNextHPEvent( int hpevent );
	void SendItemAnimation(Mob *to, const EQ::ItemData *item, EQ::skills::SkillType skillInUse, float velocity = 4.0);
	inline int& GetNextIncHPEvent() { return nextinchpevent; }
	void SetNextIncHPEvent( int inchpevent );

	inline bool DivineAura() const { return spellbonuses.DivineAura; }
 	inline bool Sanctuary() const { return spellbonuses.Sanctuary; }

	bool HasNPCSpecialAtk(const char* parse);
	bool HasSpecialAbilities();
	int GetSpecialAbility(int ability);
	int GetSpecialAbilityParam(int ability, int param);
	void SetSpecialAbility(int ability, int level);
	void SetSpecialAbilityParam(int ability, int param, int value);
	void StartSpecialAbilityTimer(int ability, uint32 time);
	void StopSpecialAbilityTimer(int ability);
	Timer *GetSpecialAbilityTimer(int ability);
	void ClearSpecialAbilities();
	void ProcessSpecialAbilities(const std::string &str);
	bool IsMoved() { return moved; }
	void SetMoved(bool moveflag) { moved = moveflag; }

	Trade* trade;

	bool ShieldAbility(uint32 target_id, int shielder_max_distance = 15, int shield_duration = 12000, int shield_target_mitigation = 50, int shielder_mitigation = 75, bool use_aa = false, bool can_shield_npc = true);
	void DoShieldDamageOnShielder(Mob *shield_target, int64 hit_damage_done, EQ::skills::SkillType skillInUse);
	void ShieldAbilityFinish();
	void ShieldAbilityClearVariables();
	inline uint32 GetShielderID() const { return m_shielder_id; }
	inline void SetShielderID(uint32 val) { m_shielder_id = val; }
	inline uint32 GetShieldTargetID() const { return m_shield_target_id; }
	inline void SetShieldTargetID(uint32 val) { m_shield_target_id = val; }
	inline int GetShieldTargetMitigation() const { return m_shield_target_mitigation; }
	inline void SetShieldTargetMitigation(int val) { m_shield_target_mitigation = val; }
	inline int GetShielderMitigation() const { return m_shielder_mitigation; }
	inline void SetShielderMitigation(int val) { m_shielder_mitigation = val; }
	inline int GetMaxShielderDistance() const { return m_shielder_max_distance; }
	inline void SetShielderMaxDistance(int val) { m_shielder_max_distance = val; }

	WeaponStance_Struct weaponstance;
	bool IsWeaponStanceEnabled() const { return weaponstance.enabled; }
	inline void SetWeaponStanceEnabled(bool val) { weaponstance.enabled = val; }


	inline glm::vec4 GetCurrentWayPoint() const { return m_CurrentWayPoint; }
	inline float GetCWPP() const { return(static_cast<float>(cur_wp_pause)); }
	inline int GetCWP() const { return(cur_wp); }
	void SetCurrentWP(int waypoint) { cur_wp = waypoint; }
	virtual FACTION_VALUE GetReverseFactionCon(Mob* iOther) { return FACTION_INDIFFERENTLY; }

	virtual const bool IsUnderwaterOnly() const { return false; }
	inline bool IsTrackable() const { return(trackable); }
	Timer* GetAIThinkTimer() { return AI_think_timer.get(); }
	Timer* GetAIMovementTimer() { return AI_movement_timer.get(); }
	Timer GetAttackTimer() { return attack_timer; }
	Timer GetAttackDWTimer() { return attack_dw_timer; }
	inline bool IsFindable() { return findable; }
	inline uint8 GetManaPercent() { return (uint8)((float)current_mana / (float)max_mana * 100.0f); }
	virtual uint8 GetEndurancePercent() { return 0; }

	inline virtual bool IsBlockedBuff(int32 SpellID) { return false; }
	inline virtual bool IsBlockedPetBuff(int32 SpellID) { return false; }

	inline void RestoreEndurance() { SetEndurance(GetMaxEndurance()); }
	inline void RestoreHealth() { SetMaxHP(); SendHPUpdate(); }
	inline void RestoreMana() { SetMana(GetMaxMana()); }

	std::string GetGlobal(const char *varname);
	void SetGlobal(const char *varname, const char *newvalue, int options, const char *duration, Mob *other = nullptr);
	void TarGlobal(const char *varname, const char *value, const char *duration, int npcid, int charid, int zoneid);
	void DelGlobal(const char *varname);

	inline void SetEmoteID(uint32 emote) { emoteid = emote; }
	inline uint32 GetEmoteID() { return emoteid; }

	bool HasSpellEffect(int effect_id);

	std::string GetRacePlural();
	std::string GetClassPlural();

	inline void SetMerchantSessionEntityID(uint16 value) { m_merchant_session_entity_id = value; }
	inline uint16 GetMerchantSessionEntityID() { return m_merchant_session_entity_id; }

	//Command #Tune functions
	void TuneGetStats(Mob* defender, Mob *attacker);
	void TuneGetACByPctMitigation(Mob* defender, Mob *attacker, float pct_mitigation, int interval = 10, int max_loop = 1000, int atk_override = 0, int Msg = 0);
	void TuneGetATKByPctMitigation(Mob* defender, Mob *attacker, float pct_mitigation, int interval = 10, int max_loop = 1000, int ac_override = 0, int Msg = 0);
	void TuneGetAvoidanceByHitChance(Mob* defender, Mob *attacker, float hit_chance, int interval, int max_loop, int accuracy_override, int Msg);
	void TuneGetAccuracyByHitChance(Mob* defender, Mob *attacker, float hit_chance, int interval, int max_loop, int avoidance_override, int Msg);
	/*support functions*/
	int64 TuneClientGetMeanDamage(Mob* other, int ac_override = 0, int atk_override = 0, int add_ac = 0, int add_atk = 0);
	int64 TuneClientGetMaxDamage(Mob* other);
	int64 TuneClientGetMinDamage(Mob* other, int max_hit);
	float TuneGetACMitigationPct(Mob* defender, Mob *attacker);
	int64 TuneGetOffense(Mob* defender, Mob *attacker, int atk_override = 0);
	int64 TuneGetAccuracy(Mob* defender, Mob *attacker, int accuracy_override = 0, int add_accuracy = 0);
	int64 TuneGetAvoidance(Mob* defender, Mob *attacker, int avoidance_override = 0, int add_avoidance = 0);
	float TuneGetHitChance(Mob* defender, Mob *attacker, int avoidance_override = 0, int accuracy_override = 0, int add_avoidance = 0, int add_accuracy = 0);
	float TuneGetAvoidMeleeChance(Mob* defender, Mob *attacker, int type);
	int64 TuneCalcEvasionBonus(int final_avoidance, int base_avoidance);
	/*modified combat code - These SYNC to attack.cpp, relevant changes to these functions in attack.cpp should be changed to the below as well*/
	int64 TuneNPCAttack(Mob* other, bool no_avoid = true, bool no_hit_chance = true, int hit_chance_bonus = 10000, int ac_override = 0, int atk_override = 0, int add_ac = 0, int add_atk = 0,
		bool get_offense = false, bool get_accuracy = false, int avoidance_override = 0, int accuracy_override = 0, int add_avoidance = 0, int add_accuracy = 0);
	int64 TuneClientAttack(Mob* other, bool no_avoid = true, bool no_hit_chance = true, int hit_chance_bonus = 10000, int ac_override = 0, int atk_override = 0, int add_ac = 0, int add_atk = 0,
		bool get_offense = false, bool get_accuracy = false, int avoidance_override = 0, int accuracy_override = 0, int add_avoidance = 0, int add_accuracy = 0);
	void TuneDoAttack(Mob *other, DamageHitInfo &hit, ExtraAttackOptions *opts = nullptr, bool no_avoid = true, bool no_hit_chance = true, int ac_override = 0, int add_ac = 0,
		int avoidance_override = 0, int accuracy_override = 0, int add_avoidance = 0, int add_accuracy = 0);
	void TuneMeleeMitigation(Mob *attacker, DamageHitInfo &hit, int ac_override, int add_ac);
	int64 Tuneoffense(EQ::skills::SkillType skill, int atk_override = 0, int add_atk = 0);
	int64 TuneACSum(bool skip_caps=false, int ac_override = 0, int add_ac = 0);
	int64 TuneGetTotalToHit(EQ::skills::SkillType skill, int chance_mod, int accuracy_override = 0, int add_accurracy = 0); // compute_tohit + spell bonuses
	int64 Tunecompute_tohit(EQ::skills::SkillType skillinuse, int accuracy_override = 0, int add_accuracy = 0);
	int64 TuneGetTotalDefense(int avoidance_override = 0, int add_avoidance = 0);
	int64 Tunecompute_defense(int avoidance_override = 0, int add_avoidance = 0);
	bool TuneCheckHitChance(Mob* other, DamageHitInfo &hit, int avoidance_override = 0, int add_avoidance = 0);
	EQ::skills::SkillType TuneAttackAnimation(int Hand, const EQ::ItemInstance* weapon, EQ::skills::SkillType skillinuse = EQ::skills::Skill1HBlunt);
	void TuneCommonOutgoingHitSuccess(Mob *defender, DamageHitInfo &hit, ExtraAttackOptions *opts = nullptr);

	//aa new
	uint32 GetAA(uint32 rank_id, uint32 *charges = nullptr) const;
	uint32 GetAAByAAID(uint32 aa_id, uint32 *charges = nullptr) const;
	bool SetAA(uint32 rank_id, uint32 new_value, uint32 charges = 0);
	void ClearAAs() { aa_ranks.clear(); }
	bool CanUseAlternateAdvancementRank(AA::Rank *rank);
	bool CanPurchaseAlternateAdvancementRank(AA::Rank *rank, bool check_price, bool check_grant);
	int GetAlternateAdvancementCooldownReduction(AA::Rank *rank_in);
	void ExpendAlternateAdvancementCharge(uint32 aa_id);
	void CalcAABonuses(StatBonuses* newbon);
	int64 CalcAAFocus(focusType type, const AA::Rank &rank, uint16 spell_id);
	void ApplyAABonuses(const AA::Rank &rank, StatBonuses* newbon);
	bool CheckAATimer(int timer);

	void CalcItemBonuses(StatBonuses* b);
	void AddItemBonuses(const EQ::ItemInstance* inst, StatBonuses* b, bool is_augment = false, bool is_tribute = false, int recommended_level_override = 0, bool is_ammo_item = false);
	void AdditiveWornBonuses(const EQ::ItemInstance* inst, StatBonuses* b, bool is_augment = false);
	int CalcRecommendedLevelBonus(uint8 current_level, uint8 recommended_level, int base_stat);

	int NPCAssistCap() { return npc_assist_cap; }
	void AddAssistCap() { ++npc_assist_cap; }
	void DelAssistCap() { --npc_assist_cap; }
	void ResetAssistCap() { npc_assist_cap = 0; }
	int64 GetWeaponDamage(Mob *against, const EQ::ItemData *weapon_item);
	int64 GetWeaponDamage(Mob *against, const EQ::ItemInstance *weapon_item, int64 *hate = nullptr);
	int64 DoDamageCaps(int64 base_damage);

	int64 GetHPRegen() const;
	int64 GetHPRegenPerSecond() const;
	int64 GetManaRegen() const;
	int64 GetEnduranceRegen() const;

	bool CanOpenDoors() const;
	void SetCanOpenDoors(bool can_open);

	void SetFeigned(bool in_feigned);
	/// this cures timing issues cuz dead animation isn't done but server side feigning is?
	inline bool GetFeigned() const { return(feigned); }

	// Data Bucket Methods
	void DeleteBucket(std::string bucket_name);
	std::string GetBucket(std::string bucket_name);
	std::string GetBucketExpires(std::string bucket_name);
	std::string GetBucketRemaining(std::string bucket_name);
	void SetBucket(std::string bucket_name, std::string bucket_value, std::string expiration = "");

	uint32 GetMobTypeIdentifier();

	// Heroic Stat Benefits
	float CheckHeroicBonusesDataBuckets(std::string bucket_name);

	int DispatchZoneControllerEvent(QuestEventID evt, Mob* init, const std::string& data, uint32 extra, std::vector<std::any>* pointers);

	// Bots HealRotation methods
	bool IsHealRotationTarget() { return (m_target_of_heal_rotation.use_count() && m_target_of_heal_rotation.get()); }
	bool JoinHealRotationTargetPool(std::shared_ptr<HealRotation>* heal_rotation);
	bool LeaveHealRotationTargetPool();

	uint32 HealRotationHealCount();
	uint32 HealRotationExtendedHealCount();
	float HealRotationHealFrequency();
	float HealRotationExtendedHealFrequency();

	const std::shared_ptr<HealRotation>* TargetOfHealRotation() const { return &m_target_of_heal_rotation; }


	// not Bots HealRotation methods
	void SetManualFollow(bool flag) { m_manual_follow = flag; }
	bool GetManualFollow() const { return m_manual_follow; }

	void DrawDebugCoordinateNode(std::string node_name, const glm::vec4 vec);

	void CalcHeroicBonuses(StatBonuses* newbon);

	DataBucketKey GetScopedBucketKeys();
	bool LoadDataBucketsCache();

	bool IsCloseToBanker();

	std::unordered_map<uint16, Mob *> &GetCloseMobList(float distance = 0.0f);
	void CheckScanCloseMobsMovingTimer();

	void ClearDataBucketCache();
	bool IsGuildmaster() const;
	bool IsDestroying() const { return m_destroying; }

protected:
	void CommonDamage(Mob* other, int64 &damage, const uint16 spell_id, const EQ::skills::SkillType attack_skill, bool &avoidable, const int8 buffslot, const bool iBuffTic, eSpecialAttacks specal = eSpecialAttacks::None);
	static uint16 GetProcID(uint16 spell_id, uint8 effect_index);
	int _GetWalkSpeed() const;
	int _GetRunSpeed() const;
	int _GetFearSpeed() const;

	Timer m_z_clip_check_timer;

	// dynamically set via memory on constructor
	int8 m_max_procs = 0;

	virtual bool AI_EngagedCastCheck() { return(false); }
	virtual bool AI_PursueCastCheck() { return(false); }
	virtual bool AI_IdleCastCheck() { return(false); }

	bool moved;

	std::vector<uint16> RampageArray;
	std::map<std::string, std::string> m_EntityVariables;

	int16 SkillDmgTaken_Mod[EQ::skills::HIGHEST_SKILL + 2];
	int16 Vulnerability_Mod[HIGHEST_RESIST+2];
	bool m_AllowBeneficial;
	bool m_DisableMelee;

	bool isgrouped;
	bool israidgrouped;
	uint16 entity_id_being_looted; //the id of the entity being looted, 0 if not looting.
	uint8 texture;
	uint8 helmtexture;
	uint8 armtexture;
	uint8 bracertexture;
	uint8 handtexture;
	uint8 legtexture;
	uint8 feettexture;
	bool multitexture;

	int AC;
	int mitigation_ac; // cached Mob::ACSum
	int32 ATK;
	int32 STR;
	int32 STA;
	int32 DEX;
	int32 AGI;
	int32 INT;
	int32 WIS;
	int32 CHA;
	int32 MR;
	int32 CR;
	int32 FR;
	int32 DR;
	int32 PR;
	int32 Corrup;
	int32 PhR;
	bool moving;
	int targeted;
	bool findable;
	bool trackable;
	int64 current_hp;
	int64 max_hp;
	int64 base_hp;
	int64 current_mana;
	int64 max_mana;
	int64 hp_regen;
	int64 hp_regen_per_second;
	int64 mana_regen;
	int64 ooc_regen;
	uint8 maxlevel;
	uint32 scalerate;
	Buffs_Struct *buffs;
	StatBonuses itembonuses;
	StatBonuses spellbonuses;
	StatBonuses aabonuses;
	uint16 petid;
	uint16 ownerid;
	uint8 type_of_pet;
	int16 petpower;
	uint32 follow_id;
	uint32 follow_dist;
	bool follow_run;
	bool no_target_hotkey;
	bool rare_spawn;
	int32 heroic_strikethrough;
	bool keeps_sold_items;
	uint32 m_npc_tint_id;

	uint32 m_PlayerState;
	uint32 GetPlayerState() { return m_PlayerState; }
	void AddPlayerState(uint32 new_state) { m_PlayerState |= new_state; }
	void RemovePlayerState(uint32 old_state) { m_PlayerState &= ~old_state; }
	void SendAddPlayerState(PlayerState new_state);
	void SendRemovePlayerState(PlayerState old_state);

	uint8 gender;
	uint16 race;
	uint16 use_model;
	uint8 base_gender;
	uint16 base_race;
	uint8 class_;
	uint8 bodytype;
	uint8 orig_bodytype;
	uint16 deity;
	uint8 level;
	uint8 orig_level;
	uint32 npctype_id;

	glm::vec4 m_Position;
	glm::vec4 m_RelativePosition;
	int animation; // this is really what MQ2 calls SpeedRun just packed like (int)(SpeedRun * 40.0f)
	float base_size;
	float size;
	float runspeed;
	float walkspeed;
	float fearspeed;
	int base_runspeed;
	int base_walkspeed;
	int base_fearspeed;
	int current_speed;
	eSpecialAttacks m_specialattacks;

	bool held;
	bool gheld;
	bool nocast;
	bool focused;
	bool pet_stop;
	bool pet_regroup;
	bool spawned;
	void CalcSpellBonuses(StatBonuses* newbon);
	virtual void CalcBonuses();
	void TrySkillProc(Mob *on, EQ::skills::SkillType skill, uint16 ReuseTime, bool Success = false, uint16 hand = 0, bool IsDefensive = false); // hand if 0 means its a skill ability for proc rate checks, otherwise hand is passed.
	bool PassLimitToSkill(EQ::skills::SkillType skill, int32 spell_id, int proc_type, int aa_id=0);
	bool PassLimitClass(uint32 Classes_, uint16 Class_);
	void TryCastOnSkillUse(Mob *on, EQ::skills::SkillType skill);
	void TryDefensiveProc(Mob *on, uint16 hand = EQ::invslot::slotPrimary);
	void TryWeaponProc(const EQ::ItemInstance* inst, const EQ::ItemData* weapon, Mob *on, uint16 hand = EQ::invslot::slotPrimary);
	void TrySpellProc(const EQ::ItemInstance* inst, const EQ::ItemData* weapon, Mob *on, uint16 hand = EQ::invslot::slotPrimary);
	void TryCombatProcs(const EQ::ItemInstance* weapon, Mob *on, uint16 hand = EQ::invslot::slotPrimary, const EQ::ItemData* weapon_data = nullptr);
	void ExecWeaponProc(const EQ::ItemInstance* weapon, uint16 spell_id, Mob *on, int level_override = -1);
	virtual float GetProcChances(float ProcBonus, uint16 hand = EQ::invslot::slotPrimary);
	virtual float GetDefensiveProcChances(float &ProcBonus, float &ProcChance, uint16 hand = EQ::invslot::slotPrimary, Mob *on = nullptr);
	virtual float GetSkillProcChances(uint16 ReuseTime, uint16 hand = 0); // hand = MainCharm?
	uint16 GetWeaponSpeedbyHand(uint16 hand);
	virtual int GetBaseSkillDamage(EQ::skills::SkillType skill, Mob *target = nullptr);
	virtual int64 GetFocusEffect(focusType type, uint16 spell_id, Mob *caster = nullptr, bool from_buff_tic = false);
	virtual EQ::InventoryProfile& GetInv() { return m_inv; }
	void CalculateNewFearpoint();
	float FindGroundZ(float new_x, float new_y, float z_offset=0.0);
	float FindDestGroundZ(glm::vec3 dest, float z_offset=0.0);

	virtual float GetSympatheticProcChances(uint16 spell_id, int16 ProcRateMod, int32 ItemProcRate = 0);
	int16 GetSympatheticSpellProcRate(uint16 spell_id);
	uint16 GetSympatheticSpellProcID(uint16 spell_id);

	enum {MAX_PROCS = 10};
	tProc PermaProcs[MAX_PROCS];
	tProc SpellProcs[MAX_PROCS];
	tProc DefensiveProcs[MAX_PROCS];
	tProc RangedProcs[MAX_PROCS];

	char name[64];
	char orig_name[64];
	char clean_name[64];
	char lastname[64];

	glm::vec4 m_Delta;
	// just locs around them to double check, if we do expand collision this should be cached on movement
	// ideally we should use real models, but this should be quick and work mostly
	glm::vec4 m_CollisionBox[COLLISION_BOX_SIZE];

	EQ::LightSourceProfile m_Light;

	EmuAppearance _appearance;
	uint8 pRunAnimSpeed;
	bool m_is_running;

	Timer attack_timer;
	Timer attack_dw_timer;
	Timer ranged_timer;
	float attack_speed; //% increase/decrease in attack speed (not haste)
	int attack_delay; //delay between attacks in 10ths of seconds
	bool always_aggro;
	int16 slow_mitigation; // Allows for a slow mitigation (100 = 100%, 50% = 50%)
	Timer hp_regen_per_second_timer;
	Timer tic_timer;
	Timer mana_timer;
	int32 dw_same_delay;

	Timer focusproclimit_timer[MAX_FOCUS_PROC_LIMIT_TIMERS];	//SPA 511
	int32 focusproclimit_spellid[MAX_FOCUS_PROC_LIMIT_TIMERS];	//SPA 511

	Timer spell_proclimit_timer[MAX_PROC_LIMIT_TIMERS];			//SPA 512
	int32 spell_proclimit_spellid[MAX_PROC_LIMIT_TIMERS];		//SPA 512
	Timer ranged_proclimit_timer[MAX_PROC_LIMIT_TIMERS];		//SPA 512
	int32 ranged_proclimit_spellid[MAX_PROC_LIMIT_TIMERS];		//SPA 512
	Timer def_proclimit_timer[MAX_PROC_LIMIT_TIMERS];			//SPA 512
	int32 def_proclimit_spellid[MAX_PROC_LIMIT_TIMERS];			//SPA 512

	int32 appearance_effects_id[MAX_APPEARANCE_EFFECTS];
	int32 appearance_effects_slot[MAX_APPEARANCE_EFFECTS];

	Timer shield_timer;
	uint32 m_shield_target_id;
	uint32 m_shielder_id;
	int m_shield_target_mitigation;
	int m_shielder_mitigation;
	int m_shielder_max_distance;

	//spell casting vars
	Timer spellend_timer;
	uint16 casting_spell_id;
	glm::vec3 m_SpellLocation;
	int attacked_count;
	bool delaytimer;
	uint16 casting_spell_targetid;
	EQ::spells::CastingSlot casting_spell_slot;
	int32 casting_spell_mana;
	uint32 casting_spell_inventory_slot;
	uint32 casting_spell_timer;
	uint32 casting_spell_timer_duration;
	int16 casting_spell_resist_adjust;
	uint32 casting_spell_aa_id;
	uint32 casting_spell_recast_adjust;
	bool casting_spell_checks;
	uint16 bardsong;
	EQ::spells::CastingSlot bardsong_slot;
	uint32 bardsong_target_id;

	bool ActiveProjectileATK;
	tProjatk ProjectileAtk[MAX_SPELL_PROJECTILE];

	glm::vec3 m_RewindLocation;

	Timer rewind_timer;

	// Currently 3 max nimbus particle effects at a time
	uint32 nimbus_effect1;
	uint32 nimbus_effect2;
	uint32 nimbus_effect3;

	uint8 haircolor;
	uint8 beardcolor;
	uint8 eyecolor1; // the eyecolors always seem to be the same, maybe left and right eye?
	uint8 eyecolor2;
	uint8 hairstyle;
	uint8 luclinface; //
	uint8 beard;
	uint32 drakkin_heritage;
	uint32 drakkin_tattoo;
	uint32 drakkin_details;
	EQ::TintProfile armor_tint;

	uint8 aa_title;

	int extra_haste; // for the #haste command
	bool mezzed;
	bool stunned;
	bool charmed; //this isnt fully implemented yet
	bool rooted;
	bool silenced;
	bool amnesiad;
	bool offhand;
	bool has_shield_equipped;
	bool has_two_hand_blunt_equipped;
	bool has_two_hander_equipped;
	bool has_dual_weapons_equipped;
	bool has_bowequipped = false;
	bool has_arrowequipped = false;
	bool use_double_melee_round_dmg_bonus;
	bool can_facestab;
	bool has_numhits;
	bool has_MGB;
	bool has_ProjectIllusion;
	int16 SpellPowerDistanceMod;
	bool last_los_check;
	bool pseudo_rooted;
	bool endur_upkeep;
	bool degenerating_effects; // true if we have a buff that needs to be recalced every tick
	bool spawned_in_water;
	bool is_boat;

	CombatRecord m_combat_record{};

public:
	const CombatRecord &GetCombatRecord() const;

public:
	bool GetWasSpawnedInWater() const;

	void SetSpawnedInWater(bool spawned_in_water);
	bool turning;

protected:

	// Bind wound
	Timer bindwound_timer;
	Mob* bindwound_target;

	Timer stunned_timer;
	Timer spun_timer;
	Timer bardsong_timer;
	Timer gravity_timer;
	Timer viral_timer;

	// MobAI stuff
	uint8 m_pet_order;
	uint8 m_previous_pet_order;
	uint32 minLastFightingDelayMoving;
	uint32 maxLastFightingDelayMoving;
	float pAggroRange = 0;
	float pAssistRange = 0;
	std::unique_ptr<Timer> AI_think_timer;
	std::unique_ptr<Timer> AI_movement_timer;
	std::unique_ptr<Timer> AI_target_check_timer;
	int8 ForcedMovement; // push
	bool permarooted;
	std::unique_ptr<Timer> AI_scan_area_timer;
	std::unique_ptr<Timer> AI_walking_timer;
	std::unique_ptr<Timer> AI_feign_remember_timer;
	std::unique_ptr<Timer> AI_check_signal_timer;
	std::unique_ptr<Timer> AI_scan_door_open_timer;
	uint32 time_until_can_move;
	HateList hate_list;
	std::set<uint32> feign_memory_list;
	// This is to keep track of the current (one only) faction mod (alliance)
	uint32 current_alliance_faction;
	int32 current_alliance_mod;
	void AddFactionBonus(uint32 pFactionID,int32 bonus);
	int32 GetFactionBonus(uint32 pFactionID);
	// This is to keep track of item faction modifiers
	std::map<uint32,int32> item_faction_bonuses; // Primary FactionID, Bonus
	void AddItemFactionBonus(uint32 pFactionID,int32 bonus);
	int32 GetItemFactionBonus(uint32 pFactionID);
	void ClearItemFactionBonuses();
	Timer hate_list_cleanup_timer;

	bool flee_mode;
	Timer flee_timer;
	Timer attack_anim_timer;
	Timer position_update_melee_push_timer;

	bool pAIControlled;
	bool roamer;

	int wandertype;
	int pausetype;

	int8 last_hp_percent;
	int32 last_hp;

	int cur_wp;
	glm::vec4 m_CurrentWayPoint;
	int cur_wp_pause;

	bool PrimaryAggro;
	bool AssistAggro;
	int npc_assist_cap;
	Timer assist_cap_timer; // clear assist cap so more nearby mobs can be called for help

	int patrol;
	glm::vec3 m_FearWalkTarget;
	bool currently_fleeing;

	bool pause_timer_complete;
	bool DistractedFromGrid;
	uint32 m_dont_heal_me_before;
	uint32 m_dont_buff_me_before;
	uint32 m_dont_dot_me_before;
	uint32 m_dont_root_me_before;
	uint32 m_dont_snare_me_before;
	uint32 m_dont_cure_me_before;

	// hp event
	int nexthpevent;
	int nextinchpevent;

	//temppet
	bool hasTempPet;
	bool _IsTempPet;
	int16 count_TempPet;
	bool pet_owner_bot;    // Flags pets as belonging to a Bot
	bool pet_owner_client; // Flags pets as belonging to a Client
	bool pet_owner_npc;    // Flags pets as belonging to an NPC
	uint32 pet_targetlock_id;

	//bot attack flags
	std::vector<uint32> bot_attack_flags;

	glm::vec3 m_TargetRing;

	GravityBehavior flymode;
	bool m_targetable;
	int QGVarDuration(const char *fmt);
	void InsertQuestGlobal(int charid, int npcid, int zoneid, const char *name, const char *value, int expdate);
	uint32 emoteid;

	MobSpecialAbility SpecialAbilities[SpecialAbility::Max];
	bool bEnraged;
	bool destructibleobject;

	std::unordered_map<uint32, std::pair<uint32, uint32>> aa_ranks;
	Timer aa_timers[aaTimerMax];

	bool is_horse;

	AuraMgr aura_mgr;
	AuraMgr trap_mgr;

	bool feigned;
	Timer forget_timer; // our 2 min everybody forgets you timer

	bool m_can_open_doors;

	MobMovementManager *mMovementManager;

	uint16 m_merchant_session_entity_id;

private:
	Mob* target;
	EQ::InventoryProfile m_inv;
	std::shared_ptr<HealRotation> m_target_of_heal_rotation;
	bool m_manual_follow;
	bool m_destroying;

	void SetHeroicStrBonuses(StatBonuses* n);
	void SetHeroicStaBonuses(StatBonuses* n);
	void SetHeroicAgiBonuses(StatBonuses* n);
	void SetHeroicDexBonuses(StatBonuses* n);
	void SetHeroicIntBonuses(StatBonuses* n);
	void SetHeroicWisBonuses(StatBonuses* n);

	void DoSpellInterrupt(uint16 spell_id, int32 mana_cost, int my_curmana);
	void HandleDoorOpen();
};
