-- Moonfury (43316) -- Moonfire tap line
-- Deals 125 damage and heals the caster 375: a 25 percent damage / 75 percent heal tap.
-- The spell row is a real tap (targettype 13), so the engine heals the first 125 itself;
-- this adds the remaining 250. See lua_modules/aotv4_moonfire.lua for why.
local moonfire = require("aotv4_moonfire")

function event_spell_effect(e)
	moonfire.tap_bonus(e, 250)
end
