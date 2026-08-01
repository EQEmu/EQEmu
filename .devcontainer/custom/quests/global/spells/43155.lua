-- Duel lock (helper, 43155)
-- Not castable by players -- applied to both participants by Duel (43030).
-- Carries the Silence and HealRate -100 halves as stock effects; when it fades,
-- the duel is over and the pairing must go with it.
local rx = require("aotv4_reactions")

function event_spell_fade(e)
	if e.target and e.target.valid then
		rx.end_duel(e.target:GetID())
	end
end
