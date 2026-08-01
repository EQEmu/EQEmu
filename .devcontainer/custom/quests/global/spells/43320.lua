-- Sinewdrain (43320) -- Sinew tap line
-- Deals 120 damage and returns 40 endurance to the caster: a 75 percent damage / 25 percent
-- endurance split. The spell row is a plain nuke, not a lifetap, so the WHOLE return is paid here.
-- See lua_modules/aotv4_sinewtap.lua for why this must not become an ST_Tap.
local sinew = require("aotv4_sinewtap")

function event_spell_effect(e)
	sinew.tap_bonus(e, 40)
end
