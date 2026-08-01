-- aotv4_reactions.lua
-- The "when you are hit" half of the custom ability set: Divine Aura, Blade
-- Turn, Counterattack and Vengeful Aura. All four hang off a single
-- EVENT_DAMAGE_TAKEN hook wired in global_player.lua.
--
-- WHY THIS EVENT CAN ABSORB DAMAGE
-- zone/attack.cpp:4404 assigns our return value to damage_override and then:
--     if (damage_override > 0)      damage = damage_override;
--     else if (damage_override < 0) damage = 0;
--     SetHP(GetHP() - damage);
-- ...so returning a positive number REPLACES the damage, a negative number
-- NEGATES it entirely, and 0/nil leaves it alone. It runs before SetHP, so this
-- genuinely prevents damage rather than healing it back afterwards.
--
-- WHY CHARGES ARE TRACKED HERE INSTEAD OF numhits
-- "for the next N attacks" looks like a job for spells_new.numhits with
-- numhitstype 6 (IncomingHitSuccess), but CheckNumHitsRemaining fires at
-- attack.cpp:4336 -- BEFORE our event at 4404. On the final charge the buff has
-- already faded by the time this code runs, so the last hit would silently do
-- nothing. Counting here keeps every charge.
--
-- CAVEAT: charge state lives in this zone process, so zoning with an active
-- Counterattack / Vengeful Aura refills its charges. Buffs are short (4 tics),
-- so this is rare and errs in the player's favour. Persisting it would mean a
-- DB write on every melee swing, which is far worse.

local ab = require("aotv4_abilities")

local M = {}

------------------------------------------------------------------ ability ids
-- ⚠️⚠️ FOUR REACTIONS WERE REMOVED HERE (2026-08-01) WITH THE 43000-43112 SET: Divine Aura (43022),
-- Blade Turn (43035), Counterattack (43056) and Vengeful Aura (43059), along with their charge
-- tables and the arm_/clear_ functions the 43035/43056/43059 spell scripts used to call. Those
-- spells no longer exist, so every one of those branches was unreachable.
-- 📌 What is left is the part that was never tied to that set: the DUEL lock and the Open Wounds
-- bleed. Do not "restore" the reaction branches without restoring the spell rows first -- a
-- FindBuff on a deleted spell id simply never matches, so it would fail silently.

-- helper carriers (see gen_spells.py HELPERS)
-- ⚠️ These two rows still EXIST in spells_new, but nothing applies them any more: the mark was cast
-- by 43052 (Open Wounds) and the lock by 43030, both of which went with the 113. The code below is
-- therefore correct but dormant. See CLAUDE.md section 20.
M.OPEN_WOUNDS_MARK = 43150
M.DUEL_LOCK        = 43155

------------------------------------------------------------------ tuning
local BLEED_PCT          = 25   -- Open Wounds banks this % of every hit as bleed
local BLEED_TICS         = 5    -- ...and pays it out over this many tics
local DUEL_MULT          = 2    -- melee damage between duel partners is doubled

-- Open Wounds: [victim entity_id] = unpaid bleed damage, banked by hits and
-- paid out over BLEED_TICS by the marker's buff tic.
local bleed_pool = {}

function M.clear_bleed(entity_id) bleed_pool[entity_id] = nil end

-- Duel: [entity_id] = partner entity_id. Symmetric -- both directions are set
-- when the duel starts and both are cleared when it ends, so a half-open duel
-- (one side immune, the other not) can never happen.
local duel_partner = {}

function M.start_duel(a_id, b_id)
	duel_partner[a_id] = b_id
	duel_partner[b_id] = a_id
end

function M.end_duel(entity_id)
	local partner = duel_partner[entity_id]
	duel_partner[entity_id] = nil
	if partner then
		duel_partner[partner] = nil
	end
end

function M.duel_partner_of(entity_id) return duel_partner[entity_id] end

------------------------------------------------------------------ helpers

-- Melee damage carries no spell id. Mob::Damage defaults spell_id to
-- SPELL_UNKNOWN (0xFFFF), and some paths pass 0, so treat both as physical.
-- Damage shields and buff tics are never "a physical attack" for our purposes.
local function is_physical_hit(e)
	if e.is_damage_shield or e.is_buff_tic then
		return false
	end
	return e.spell_id == 0 or e.spell_id == 65535
end

-- 📌 A local `in_front_of` helper lived here (mirroring the stock idiom at zone/attack.cpp:419). Its
-- only caller was Counterattack, which went with the 43000-43112 set, so it was removed too. The
-- idiom if it is ever wanted again: `not attacker:BehindMob(me, attacker:GetX(), attacker:GetY())`.

------------------------------------------------------------------ main hook

-- Returns a damage override for EVENT_DAMAGE_TAKEN: negative negates the hit,
-- positive replaces it, nil/0 leaves it untouched.
function M.on_damage_taken(e)
	local me = e.self
	if not me or not me.valid or not me:IsClient() then
		return 0
	end

	local damage = e.damage or 0
	if damage <= 0 then
		return 0
	end

	-- ⚠️⚠️ THIS HOOK RUNS ON EVERY DAMAGE EVENT FOR EVERY PLAYER, so it does the least possible work
	-- before the duel early-out. It used to resolve CastToClient + CharacterID and an
	-- eq.get_entity_list():GetMobID(e.entity_id) lookup up front -- an entity-list walk PER HIT --
	-- purely to feed the four ability reactions that were removed with the 43000-43112 set. Nothing
	-- left here needs the attacker object or the character id. Do not reintroduce either "so it is
	-- available": resolve them inside whichever branch actually wants them.
	local physical = is_physical_hit(e)

	-- 0. Duel outranks everything: while locked in, the pair exist in their own
	--    little world. Anything that isn't your opponent simply cannot touch you,
	--    and what your opponent lands hits twice as hard.
	local partner_id = duel_partner[me:GetID()]
	if partner_id then
		if e.entity_id ~= partner_id then
			return -1                       -- immune to the outside world
		end
		if physical then
			return damage * DUEL_MULT
		end
		return 0
	end

	-- ⚠️ The four ability reactions that used to run here (Divine Aura, Blade Turn, Counterattack,
	-- Vengeful Aura) were removed with the 43000-43112 set on 2026-08-01. The duel block above is
	-- the whole of this hook now.
	return 0
end

------------------------------------------------------- Open Wounds bleed

-- Every hit on a marked victim banks BLEED_PCT of its damage. The marker lasts
-- a minute, so sustained pressure keeps topping the pool up -- exactly the
-- "attacks on this target ... will cause them to take 25% additional bleed
-- damage" wording.
function M.on_damage_given(e)
	if not e.entity_id or e.entity_id == 0 then
		return 0
	end
	local damage = e.damage or 0
	if damage <= 0 or e.is_damage_shield or e.is_buff_tic then
		return 0
	end

	local victim = eq.get_entity_list():GetMobID(e.entity_id)
	if not victim or not victim.valid or not victim:FindBuff(M.OPEN_WOUNDS_MARK) then
		return 0
	end

	local banked = math.floor(damage * BLEED_PCT / 100)
	if banked > 0 then
		bleed_pool[e.entity_id] = (bleed_pool[e.entity_id] or 0) + banked
	end
	return 0
end

-- Paid out a fifth at a time by the marker's buff tic, so damage banked from a
-- flurry bleeds out over the following BLEED_TICS rather than all at once.
function M.bleed_tick(victim, caster)
	if not victim or not victim.valid then
		return
	end
	local id     = victim:GetID()
	local pool   = bleed_pool[id]
	if not pool or pool <= 0 then
		return
	end

	local payout = math.ceil(pool / BLEED_TICS)
	if payout > pool then payout = pool end
	bleed_pool[id] = pool - payout
	if bleed_pool[id] <= 0 then
		bleed_pool[id] = nil
	end

	victim:Damage(caster or victim, payout, M.OPEN_WOUNDS_MARK, ab.SKILL_OFFENSE, false)
end

return M
