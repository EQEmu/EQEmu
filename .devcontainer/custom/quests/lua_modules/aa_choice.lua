-- aa_choice.lua
-- Death-driven random AA picker (sibling of spell_choice.lua).
--   * On death, global_player.lua banks the run's XP as AA points (client:AddAAPoints) and
--     restarts at level 1, then calls M.grant_picks(e).
--   * The player's UNSPENT AA pool (GetAAPoints) IS the budget. Each window pop offers 3 random,
--     level-appropriate AAs the player can AFFORD; picking one trains its next rank and SPENDS its
--     cost (GrantAlternateAdvancementAbility ignore_cost=false). We keep popping the window until
--     the budget can't afford anything -> no fixed pick count, no wasted/failed picks.
--   * Class-agnostic: aa_ability.classes carries the Bard bit (128); the server's class gate was
--     fixed to use GetPlayerClassBit (see zone/aa.cpp). Expansion gating is done by the pool, with
--     Expansion:UseCurrentExpansionAAOnly = false so classic theming doesn't block all AAs.
--
-- Transport mirrors spell_choice but on its OWN channel so it can drive a SEPARATE dll window:
--   server -> client : "AACHOICEDATA name|icon|cost^name|icon|cost^name|icon|cost"
--   client -> server : "/say aapick N"
-- A chat saylink fallback also works with no client mod (for testing before the dll window exists).

local aapool = require("aa_pool")
local era    = require("era_system")   -- AAs are gated by the server-wide unlocked expansion era

local M = {}

local CHOICE_COUNT  = 3
local SAY_TRIGGER   = "aapick"
local MAX_PICKS     = 30     -- safety cap on window pops per death (budget normally ends it sooner)
local SHOW_SAYLINKS = true   -- chat fallback until the dll AA window is built

-- ⚠️⚠️ THE PER-AA INVESTMENT CAP IS DISABLED (0 = off), and it must stay that way under the current
-- pricing. It used to be 10, meaning an AA stopped being offered once `next_rank * cost` exceeded
-- that -- a design from when a death banked ~10 points and one AA could otherwise soak the lot.
-- With the 2026-07-31 repricing that cap silently makes most of the set unfinishable:
--   * a custom tree's rank 5 costs 3, so 5 * 3 = 15 > 10 -- every custom tree would stall at rank 3
--   * Slow Knitting has 31 ranks at 1 point, so it would stall at rank 10 of 31
-- and the failure is invisible: the AA simply stops appearing in the offers, with no message.
-- The only gates now are the ones the design actually calls for -- can you afford it, is there a
-- rank left to buy, and are its prerequisites owned.
local RANK_BUDGET   = 0

-- Banked AA points live in a PRIVATE data bucket (aa_bank_<charid>), NOT the native unspent pool
-- (m_pp.aapoints). That way the native AA window shows 0 spendable points -- the player cannot
-- bypass the random picker and buy AAs directly there. Picks are granted FREE (ignore_cost=true)
-- and we deduct the cost from the bucket ourselves.
local function bank_key(client)   return "aa_bank_"   .. client:CharacterID() end
local function choice_key(client) return "aa_choice_" .. client:CharacterID() end
local function seed_key(client)   return "aa_seed_"   .. client:CharacterID() end
local function count_key(client)  return "aa_pcount_" .. client:CharacterID() end

local function get_num(key) local v = eq.get_data(key); return tonumber(v) or 0 end

-- A few EQ AAs (Origin, Harm Touch, Lay on Hands, Hunter's Attack Power) have a real first-rank
-- cost of 0 in aa_ranks -- they're class "freebies". In the roguelite picker every reward should
-- cost at least 1 banked point, so we floor the cost here. Used everywhere the cost is read (offer
-- fields, affordability, the stored "id:cost" the deduction reads) so all three stay consistent.
local function aa_cost(aa) local c = tonumber(aa.cost) or 1; return c < 1 and 1 or c end

-- Reverse id -> name from the pool (built once), so the pick confirmation can name the AA. The
-- engine's own "gained the ability ... at a cost of 0" line is misleading (we grant ignore_cost,
-- so it always says 0); the dll hides it and we print the REAL banked cost instead.
local aa_by_id
local function aa_lookup(id)
	if not aa_by_id then
		aa_by_id = {}
		for _, list in pairs(aapool) do
			for _, aa in ipairs(list) do aa_by_id[aa.id] = aa end
		end
	end
	return aa_by_id[id]
end
local function name_of(id)
	local aa = aa_lookup(id)
	return aa and aa.name or ("ability " .. tostring(id))
end

-- Push the banked total to the dll with NO choices, so its header updates when the picker STOPS
-- (everything spent / nothing affordable). The dll only auto-opens the journal when choices > 0,
-- so this just corrects the "Banked: N" number (e.g. back to 0) without re-popping the window --
-- otherwise the header keeps showing the last offer's banked count after the final pick.
local function send_bank_update(client, budget)
	client:Message(MT.NPCQuestSay, "AACHOICEDATA " .. tostring(budget))
end

-- The player satisfies an AA's first-rank prerequisites (so the server won't reject it). Mirrors
-- CanPurchaseAlternateAdvancementRank's prereq loop: needs `r` ranks of each required ability `a`.
local function prereqs_met(client, aa)
	if not aa.pr then return true end
	for _, req in ipairs(aa.pr) do
		if (client:GetAAByAAID(req.a) or 0) < req.r then return false end
	end
	return true
end

-- Level-appropriate AAs the player can afford AND whose prereqs are met. The 1-60 tier was lowered
-- to level_req 1 (most live at pool[1]; 61+ unlock at their level); prereqs gate dependents until
-- the player has trained what they require -- so dependency chains unlock progressively.
local function gather_affordable(client, level, budget)
	local out = {}
	for lv = 1, level do
		local list = aapool[lv]
		if list then
			for _, aa in ipairs(list) do
				local cost = aa_cost(aa)
				local next_rank = (client:GetAAByAAID(aa.id) or 0) + 1
				-- AoTv4: offered when affordable, the NEXT rank is still within the AA's max picker rank
				-- (aa.mr), within the per-AA investment cap (next_rank*cost <= RANK_BUDGET), and PREREQS
				-- are owned. The mr gate keeps MULTI-rank AAs coming (next rank <= mr) but drops a
				-- SINGLE-rank AA you already own -- e.g. Origin (mr=1): own rank 1 -> next 2 > 1 -> not
				-- offered. mr is floored to 1 so the handful of mr=0 pool rows still offer their first rank.
				local max_rank    = math.max(1, tonumber(aa.mr) or 1)
				local within_rank = next_rank <= max_rank
				-- ⚠️ RANK_BUDGET = 0 disables the per-AA investment cap entirely -- see the note on the
				-- constant for why it must stay off under the current pricing.
				local within_cap  = (RANK_BUDGET <= 0) or (next_rank * cost <= RANK_BUDGET)
				if cost <= budget and within_rank and within_cap and prereqs_met(client, aa) then
					out[#out + 1] = aa
				end
			end
		end
	end
	return out
end

local function pick_random(list, n)
	local bag, out = {}, {}
	for i, v in ipairs(list) do bag[i] = v end
	for _ = 1, n do
		if #bag == 0 then break end
		local i = math.random(#bag)
		out[#out + 1] = bag[i]
		table.remove(bag, i)
	end
	return out
end

-- Send an offer to the client: the dll AA window (AACHOICEDATA + AADESCDATA) and the chat saylink
-- fallback. `choices` is an ordered list of aa objects. Used to send a freshly-rolled offer AND to
-- re-send the STORED one unchanged.
local function send_offer(client, budget, choices)
	local fields, descs = {}, {}
	for _, aa in ipairs(choices) do
		local cost = aa_cost(aa)
		fields[#fields + 1] = aa.name .. "|" .. tostring(aa.icon or 0) .. "|" ..
			tostring(cost) .. "|" .. tostring(aa.cls or 0)
		descs[#descs + 1]   = ((aa.desc or ""):gsub("[%^|]", " "))               -- ^ and | are delimiters
	end
	client:Message(MT.NPCQuestSay,
		"AACHOICEDATA " .. tostring(budget) .. "^" .. table.concat(fields, "^"))
	client:Message(MT.NPCQuestSay, "AADESCDATA " .. table.concat(descs, "^"))
	if SHOW_SAYLINKS then
		client:Message(MT.Yellow, string.format(
			"Death has tempered you (%d AA banked) -- choose an Alternate Ability:", budget))
		for i, aa in ipairs(choices) do
			local link = eq.say_link(SAY_TRIGGER .. " " .. i, true, "[ Train " .. aa.name .. " ]")
			client:Message(MT.LightBlue, string.format("  %d) %s  (cost %d)  %s", i, aa.name, aa_cost(aa), link))
		end
	end
end

-- Re-send the CURRENTLY STORED offer verbatim (no re-roll) by rebuilding the choice list from the
-- "id:cost,..." in choice_key. Returns true if there was a stored offer. This is what makes the offer
-- STICKY: it only changes when the player picks one; leveling/relog just re-show the same options.
local function resend_stored_offer(client)
	local stored = eq.get_data(choice_key(client))
	if not stored or stored == "" then return false end
	local choices, want = {}, 0
	for s in stored:gmatch("([^,]+)") do
		want = want + 1
		local id = s:match("^(%d+):")
		local aa = id and aa_lookup(tonumber(id))
		if aa then choices[#choices + 1] = aa end
	end

	-- ⚠️⚠️ A PARTIALLY RESOLVED OFFER IS THROWN AWAY, NOT SENT. This used to keep whatever it could
	-- look up and silently drop the rest, which combines terribly with the offer being STICKY: a
	-- stored offer naming abilities that are no longer in the pool -- because the pool was
	-- regenerated, or an AA was disabled -- came back as a SHORTER offer, and then re-sent that same
	-- degraded set forever, since only a pick ever re-rolls it. Observed as "23 points banked and
	-- exactly one option", where two of the three stored ids had been disabled.
	-- Discarding it makes M.offer roll a fresh full set instead, which is always safe: nothing has
	-- been spent yet, so nothing is lost by re-rolling.
	if #choices < want then
		eq.set_data(choice_key(client), "")
		return false
	end

	if #choices == 0 then return false end
	send_offer(client, get_num(bank_key(client)), choices)
	return true
end

------------------------------------------------------------------- public API
-- Bank `points` AA points (added to the private bucket) and start presenting choices.
function M.grant_picks(e, points)
	local client = e.self
	points = tonumber(points) or 0
	if points > 0 then
		eq.set_data(bank_key(client), tostring(get_num(bank_key(client)) + points))
	end
	eq.set_data(count_key(client), "0")
	M.offer(e)
end

-- Re-open the picker for any LEFTOVER banked points (called on level-up). After a death the picker
-- spends down to whatever couldn't be afforded YET (level too low, rank cap, unmet prereqs) and stops;
-- those points stay in the bank but had no way to be spent later. On each level-up we re-check: if the
-- leftover can now afford something new, start a fresh pick session (reset the per-death pick counter)
-- so it pops; otherwise just refresh the dll's banked header silently (no "nothing affordable" spam).
function M.reoffer_if_banked(e)
	-- Re-show the CURRENT pending offer (the SAME options) so it stays reachable after leveling/zoning.
	-- NEVER re-rolls or generates -- the offered set changes ONLY when the player picks one. If nothing
	-- is pending, do nothing (a new set is rolled only on death, or right after a pick).
	resend_stored_offer(e.self)
end

-- Present one AA choice if the banked budget can still afford anything.
function M.offer(e)
	local client = e.self
	local budget = get_num(bank_key(client))

	-- STICKY: if an offer is already pending, re-send it UNCHANGED and stop. The offered set only
	-- changes when the player PICKS one (handle_say clears choice_key first, THEN calls offer() to roll
	-- the next set). So leveling, relogging, or just not picking never reshuffles the options.
	if resend_stored_offer(client) then return end

	if get_num(count_key(client)) >= MAX_PICKS then
		eq.set_data(choice_key(client), "")
		send_bank_update(client, budget)
		client:Message(MT.Yellow, string.format(
			"That is enough training for now -- %d AA point(s) remain banked.", budget))
		return
	end

	if budget < 1 then
		eq.set_data(choice_key(client), "")
		send_bank_update(client, 0)
		if get_num(count_key(client)) > 0 then
			client:Message(MT.Yellow, "You have spent all of your banked AA points.")
		end
		return
	end

	local pool = gather_affordable(client, client:GetLevel(), budget)
	if #pool == 0 then
		eq.set_data(choice_key(client), "")
		send_bank_update(client, budget)
		client:Message(MT.Yellow, string.format(
			"You have %d AA point(s) banked -- nothing else is affordable right now.", budget))
		return
	end

	-- vary the offered set per pick: after death GetEXP() is 0, so seed on a per-offer counter
	-- (otherwise every pick would reroll the SAME 3 AAs).
	local n = get_num(seed_key(client)) + 1
	eq.set_data(seed_key(client), tostring(n))
	math.randomseed(client:CharacterID() * 2654435761 + n * 7919)

	local choices = pick_random(pool, CHOICE_COUNT)
	local ids = {}
	for _, aa in ipairs(choices) do
		ids[#ids + 1] = tostring(aa.id) .. ":" .. tostring(aa_cost(aa))   -- "id:cost"
	end
	eq.set_data(choice_key(client), table.concat(ids, ","))   -- store BEFORE sending (so it's stickily re-sendable)
	send_offer(client, budget, choices)
end

-- Resolve an "aapick N". Returns true if it consumed the message.
function M.handle_say(e)
	local idx = (e.message or ""):lower():match("^" .. SAY_TRIGGER .. "%s+(%d+)%s*$")
	if not idx then return false end
	idx = tonumber(idx)

	local client = e.self
	local data = eq.get_data(choice_key(client))
	if not data or data == "" then
		client:Message(MT.Red, "You have no Alternate Ability choice pending.")
		return true
	end

	local ids, costs = {}, {}
	for s in data:gmatch("([^,]+)") do
		local id, cost = s:match("^(%d+):(%d+)$")
		ids[#ids + 1]   = tonumber(id)
		costs[#costs + 1] = tonumber(cost) or 0
	end
	local aa_id = ids[idx]
	if not aa_id then
		client:Message(MT.Red, "That is not a valid choice.")
		return true
	end

	-- Train the next rank FREE (ignore_cost=true) -- still respects class/expansion/level/prereq
	-- gates -- then deduct the cost from the PRIVATE bank (the points never touch the native pool,
	-- so the native AA window can't be used to spend them).
	local cur = client:GetAAByAAID(aa_id) or 0
	local ok  = client:GrantAlternateAdvancementAbility(aa_id, cur + 1, true)

	eq.set_data(choice_key(client), "")                                       -- consume this offer
	eq.set_data(count_key(client), tostring(get_num(count_key(client)) + 1))

	if ok then
		local spent = costs[idx] or 0
		local left  = get_num(bank_key(client)) - spent
		if left < 0 then left = 0 end
		eq.set_data(bank_key(client), tostring(left))
		-- Accurate confirmation (the engine's "at a cost of 0" line is hidden by the dll).
		client:Message(MT.Yellow, string.format(
			"You train %s for %d AA. (%d banked.)", name_of(aa_id), spent, left))
		era.check_unlock(client)   -- maxed the era's tier? -> unlock the next expansion server-wide
	else
		client:Message(MT.Red, "You cannot train that ability any further right now.")
	end

	M.offer(e)                                                                -- next choice if budget remains
	return true
end

return M
