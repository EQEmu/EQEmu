-- aotv4_moonfire.lua -- shared logic for the Moonfire tap line (43312-43317).
--
-- The line is a LIFETAP weighted 25 percent damage / 75 percent healing: a tier that deals 25
-- heals the caster 75. One cast, no optional second spell, no trigger spell.
--
-- Why any Lua is needed at all: EQEmu hardcodes the tap ratio at 1:1. Mob::Damage does
--   int64 healed = damage;                                  (zone/attack.cpp:4287)
-- and there is no ratio field anywhere in spells_new, so a native tap can never heal more than it
-- deals. The spell rows are therefore real taps (targettype 13 ST_Tap, which is exactly what
-- IsLifetapSpell keys on -- common/spdat.cpp:108) and the engine pays the first 1x of healing
-- itself; this module adds the remaining 2x on top so the total lands at 3x damage.
--
-- Keeping the spell a genuine tap rather than a plain nuke plus a Lua heal matters: IsLifetapSpell
-- gates a pile of behaviour elsewhere (it is excluded from IsDamageSpell / IsAnyDamageSpell /
-- IsDamageOverTimeSpell, it drives the "beams a smile at" emote, and bots treat taps specially),
-- so the spell behaves like every other tap in the game.
--
-- ⚠️ The bonus is a FLAT amount, while the engine's own 1x follows the damage actually dealt. On a
-- partial resist the native half shrinks and the bonus does not, so a resisted cast heals slightly
-- more than 3x its reduced damage. Accepted: the alternative is re-deriving post-resist damage in
-- Lua, which the event does not hand us.

local M = {}

-- Called from each tier's spell script. `bonus` is 2x that tier's base damage: the engine has
-- already healed 1x by the time this runs.
function M.tap_bonus(e, bonus)
	if not e or not bonus or bonus <= 0 then
		return
	end
	if not e.caster_id or e.caster_id == 0 then
		return
	end
	local caster = eq.get_entity_list():GetMobID(e.caster_id)
	if not caster or not caster.valid then
		return          -- caster zoned or died mid-cast
	end
	caster:HealDamage(bonus)
end

return M
