-- aotv4_thirst.lua -- flat heal-per-melee-hit for the Thirst line (43342-43347).
--
-- The line heals a SET AMOUNT per hit, not a percentage. That cannot be done with the native
-- mechanic: SPA 178 MeleeLifetap is percentage-only by construction --
--     lifetap_amt = damage * (melee_lifetap_mod / 100.0f);      (zone/mob.cpp:6860)
-- there is no flat variant, and nothing else fires on every successful melee hit. So the spell rows
-- are inert markers (a buff with an icon and a duration and no mechanical effect) and the healing
-- happens here, off global_player's event_damage_given.
--
-- CHARGES are handled NATIVELY, not here: the spell rows carry numhits + numhitstype 5 ("Outgoing
-- Hit Successes"), so the engine decrements the buff on each successful melee hit and fades it at
-- zero (Mob::CheckNumHitsRemaining, zone/spell_effects.cpp:7075; fired from CommonOutgoingHitSuccess,
-- zone/attack.cpp:6690). The client shows the counter for free. Native precedent: the Blade
-- Attunement line (4644/4646/4648) is likewise an inert buff with numhits 100 and type 5.
--
-- ⚠️ The charge and the heal are counted in DIFFERENT places -- the engine decrements in
-- CommonOutgoingHitSuccess, we heal from event_damage_given in Mob::Damage. They line up on a normal
-- swing, but a hit fully absorbed to 0 damage still spends a charge while paying no heal.
--
-- ⚠️ THIS RUNS ON EVERY DAMAGE EVENT FOR EVERY PLAYER. Keep it cheap and keep the early-outs first.
-- The ordering below matters: the arithmetic filters (damage, spell_id, flags) reject the vast
-- majority of events before we ever touch the buff array.

local M = {}

-- spell id -> hit points healed per melee hit. Must match the tier values in
-- custom/sql/aotv4_thirst_line.sql; the SQL is documentation only, this table is what pays out.
local HEAL = {
	[43342] = 2,    -- Faint Thirst          L8
	[43343] = 4,    -- Rising Thirst         L18
	[43344] = 6,    -- Keen Thirst           L28
	[43345] = 8,    -- Ravening Thirst       L38
	[43346] = 10,   -- Savage Thirst         L48
	[43347] = 12,   -- Unquenchable Thirst   L58
}

-- Ordered high tier first: only one tier can be up at a time (they share a spellgroup), so the
-- first hit is the answer and we stop looking.
local ORDER = { 43347, 43346, 43345, 43344, 43343, 43342 }

function M.on_damage_given(e, client)
	-- Cheapest rejections first, before any buff lookup.
	if not e or not e.damage or e.damage <= 0 then return end
	if e.spell_id and e.spell_id > 0 and e.spell_id < 65535 then return end  -- spell damage, not a swing
	if e.is_damage_shield then return end                                    -- our own DS reflecting
	if e.is_buff_tic then return end                                         -- a DoT ticking
	if not client or not client.valid then return end

	for i = 1, #ORDER do
		local id = ORDER[i]
		if client:FindBuff(id) then
			client:HealDamage(HEAL[id])
			return
		end
	end
end

return M
