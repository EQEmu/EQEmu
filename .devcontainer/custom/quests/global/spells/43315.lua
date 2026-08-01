-- Moonstorm (43315) -- Moonfire tap line
-- Deals 100 damage and heals the caster 300: a 25 percent damage / 75 percent heal tap.
-- The spell row is a real tap (targettype 13), so the engine heals the first 100 itself;
-- this adds the remaining 200. See lua_modules/aotv4_moonfire.lua for why.
local moonfire = require("aotv4_moonfire")

function event_spell_effect(e)
	moonfire.tap_bonus(e, 200)
end
