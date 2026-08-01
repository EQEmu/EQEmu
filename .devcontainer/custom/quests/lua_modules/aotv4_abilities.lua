-- aotv4_abilities.lua
-- Shared helpers for the custom ability set (spell ids 43000+, see
-- .devcontainer/custom/spells/spell_design.csv).
--
-- These are the "Bucket B" abilities: the ones with no stock SPA equivalent, so
-- their mechanics live in per-spell Lua at quests/global/spells/<id>.lua. The
-- spell row still carries name / cost / cooldown / duration / icon; only the
-- EFFECT is scripted here.
--
-- Spell script events (zone/lua_parser.cpp):
--   EVENT_SPELL_EFFECT_CLIENT / _NPC        -- landed on a client / an NPC
--   EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT/_NPC -- every tick while the buff holds
--   EVENT_SPELL_FADE                        -- buff expired or was removed
-- Every one of them receives: e.target, e.spell_id, e.caster_id,
-- e.tics_remaining, e.caster_level, e.buff_slot, e.spell.

local M = {}

------------------------------------------------------------------ constants
-- Equipment slots (common/patches/rof2_limits.h; INULL = 0)
M.SLOT_RANGE     = 11
M.SLOT_PRIMARY   = 13
M.SLOT_SECONDARY = 14
M.SLOT_FEET      = 19

-- Skill ids (common/skills.h)
M.SKILL_ARCHERY  = 7
M.SKILL_KICK     = 30
M.SKILL_OFFENSE  = 33
M.SKILL_THROWING = 51

-- A bare hand / empty slot still needs a damage basis, so unarmed characters and
-- players with no offhand aren't locked out of their own abilities.
local FIST_DAMAGE, FIST_DELAY = 4, 24

--------------------------------------------------------------------- helpers

-- Resolve the caster of a spell event. Returns nil if they left the zone.
function M.caster(e)
	if not e.caster_id or e.caster_id == 0 then
		return nil
	end
	local mob = eq.get_entity_list():GetMobID(e.caster_id)
	if not mob or not mob.valid then
		return nil
	end
	return mob
end

-- The Lua_Item in an equipment slot, or nil when the slot is empty.
-- NOTE: Lua_ItemInst exposes `valid` as a PROPERTY, Lua_Item as a METHOD
-- (zone/lua_iteminst.cpp:471 vs zone/lua_item.cpp:931) -- they are not
-- interchangeable and mixing them up throws.
function M.equipped(mob, slot)
	if not mob or not mob.valid or not mob:IsClient() then
		return nil
	end
	local inst = mob:CastToClient():GetInventory():GetItem(slot)
	if not inst or not inst.valid then
		return nil
	end
	local item = inst:GetItem()
	if not item or not item:valid() then
		return nil
	end
	return item
end

-- Damage-per-second of the weapon in `slot`. EQ weapon delay is in tenths of a
-- second, so DPS = damage / (delay / 10).
function M.weapon_dps(mob, slot)
	local item  = M.equipped(mob, slot)
	local dmg   = item and item:Damage() or FIST_DAMAGE
	local delay = item and item:Delay()  or FIST_DELAY
	if dmg   <= 0 then dmg   = FIST_DAMAGE end
	if delay <= 0 then delay = FIST_DELAY  end
	return dmg / (delay / 10.0)
end

-- Core of the "% of weapon DPS" abilities (Strike, The Left, Snipe, ...).
--
-- `pct` is the design sheet's percentage: 750 means "750% of primary DPS". The
-- resulting number is handed to DoMeleeSkillAttackDmg as base_damage, which then
-- runs the normal to-hit and mitigation pass (zone/special_attacks.cpp:2536) --
-- so these attacks can miss, crit and be mitigated like any other swing rather
-- than landing a flat guaranteed number.
--
-- `accuracy` is added to the to-hit roll (chance_mod). Leave nil for a normal
-- chance to hit.
function M.pct_dps_attack(caster, target, pct, slot, skill, accuracy)
	if not caster or not caster.valid or not target or not target.valid then
		return 0
	end
	local base = math.floor(M.weapon_dps(caster, slot) * pct / 100.0)
	if base < 1 then base = 1 end
	caster:DoMeleeSkillAttackDmg(target, base, skill, accuracy or 0)
	return base
end

-- Percent-of-max resource helpers. Always move at least 1 point, so a low-level
-- character with a tiny pool still sees the ability do something.
function M.pct_of(value, pct)
	local amount = math.floor(value * pct / 100.0)
	if amount < 1 then amount = 1 end
	return amount
end

return M
