-- Open Wounds bleed marker (helper, 43150)
-- Not castable by players -- applied by Open Wounds (43052). Each tic pays out a
-- share of the damage banked by hits on this target, then the marker expires
-- after a minute.
local ab = require("aotv4_abilities")
local rx = require("aotv4_reactions")

function event_spell_buff_tic(e)
	rx.bleed_tick(e.target, ab.caster(e))
end

function event_spell_fade(e)
	if e.target and e.target.valid then
		rx.clear_bleed(e.target:GetID())
	end
end
