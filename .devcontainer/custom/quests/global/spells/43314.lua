-- Moonblaze (43314) -- Moonfire tap line
-- Deals 80 damage and heals the caster 240: a 25 percent damage / 75 percent heal tap.
-- The spell row is a real tap (targettype 13), so the engine heals the first 80 itself;
-- this adds the remaining 160. See lua_modules/aotv4_moonfire.lua for why.
local moonfire = require("aotv4_moonfire")

function event_spell_effect(e)
	moonfire.tap_bonus(e, 160)
end
