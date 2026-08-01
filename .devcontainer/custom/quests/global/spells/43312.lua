-- Moonspark (43312) -- Moonfire tap line
-- Deals 25 damage and heals the caster 75: a 25 percent damage / 75 percent heal tap.
-- The spell row is a real tap (targettype 13), so the engine heals the first 25 itself;
-- this adds the remaining 50. See lua_modules/aotv4_moonfire.lua for why.
local moonfire = require("aotv4_moonfire")

function event_spell_effect(e)
	moonfire.tap_bonus(e, 50)
end
