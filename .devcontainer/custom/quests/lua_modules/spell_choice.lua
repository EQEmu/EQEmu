-- spell_choice.lua
-- Level-up "choose 1 of 3 spells" system.
--   * Class-agnostic: the pool is every scribable spell (any class's spells).
--   * Level-appropriate: only spells whose lowest class-learn-level is <= the
--     player's level are offered, biased to within LEVEL_BAND of their level,
--     so a level 1 never sees a level 50 spell.
--   * Server-authoritative: the 3 offered spell ids are stored in a data bucket
--     and the player picks by INDEX (1-3), so a modified client cannot scribe an
--     arbitrary spell -- only one of the three it was actually offered.
--
-- Transport (no opcode patching):
--   server -> client : a window is opened (TEST: chat saylinks; DLL: magic chat line)
--   client -> server : player "says" "spellpick <N>" (saylink today, eq-core-dll
--                       window injects the same say later) -> handled in event_say.

local pool      = require("spell_pool")
local icons     = require("spell_icons")     -- id -> spellbook icon index (for the DLL window)
local spelldesc = require("spell_desc")      -- id -> description text (for the DLL window hover)
local skills    = require("skill_pool")      -- combat skill rewards: id -> {name, icon}
local blacklist = require("spell_blacklist") -- spell ids never offered (rez, enchant, ...)

local M = {}

------------------------------------------------------------------- tuning
local CHOICE_COUNT      = 3          -- choices offered per level-up
local LEVEL_BAND        = 6          -- prefer spells learned within [L-band, L]
local SAY_TRIGGER       = "spellpick" -- player says "spellpick <N>" to choose
local USE_DLL_TRIGGER   = true       -- emit the SPELLCHOICEDATA line for the eq-core-dll window
local SHOW_SAYLINKS     = true        -- also show chat saylinks (no-client-mod fallback)
-- Chance a level-up spends ONE of its three slots on a cross-class combat ability.
-- 0.125 = roughly one level in eight. Deliberately rare: only FOUR abilities can be on autoskill at
-- once (AOTV4_AUTOSKILL_MAX, zone/special_attacks.cpp), so handing them out often would fill that
-- budget long before the choice mattered.
-- ⚠️ It was 0.0 -- combat abilities were never offered at all, while CLAUDE.md still described it as
-- "~0.7". If this is changed again, change the doc with it.
local SKILL_OFFER_CHANCE = 0.125
local SKILL_GRANT_PCT    = 0.25      -- a picked skill is set to this fraction of its level cap
local DLL_MARK        = string.char(18) -- DC2 control byte the DLL brackets the payload with
-- Reroll pricing. The Reroll button charges COIN directly -- there is no token item and no vendor.
-- Cost is 5p for your first reroll, 10p for the second, 15p for the third, and so on forever.
--
-- ⚠️ The count is per CHARACTER and PERMANENT (bucket "rerollbuy_<charid>"). It is deliberately NOT
-- cleared by the roguelite death that resets level, gear and spells: rerolls are meant to get more
-- expensive over a character's whole life, not to reset to 5p every run.
local REROLL_BASE_PLAT = 5
local function reroll_key(client) return "rerollbuy_" .. client:CharacterID() end
local function reroll_count(client) return tonumber(eq.get_data(reroll_key(client))) or 0 end

-- Price in COPPER of this character's next reroll. Copper because that is the unit every money call
-- on Lua_Client speaks (GetCarriedMoney / TakeMoneyFromPP).
function M.reroll_cost(client)
	return REROLL_BASE_PLAT * (reroll_count(client) + 1) * 1000
end

-- Tell the window what the next reroll costs, so the box beside the button is never stale. Sent with
-- every offer AND after every reroll -- the price changes the moment one is bought.
function M.send_reroll_cost(client)
	client:Message(MT.NPCQuestSay, "SPELLREROLLCOST " .. M.reroll_cost(client))
end

-- Offset that lifts a COMBAT ABILITY's skill id (0-76) clear of the spell id space, so the Pool tab
-- can list both and ask about either with one "sjinfo <id>".
-- ⚠️ MUST match SJ_SKILL_BASE in eq-core-dll/src/core_spelljournal.cpp.
local SKILL_ID_BASE   = 1000000

------------------------------------------------------------------- helpers
local function bucket_key(client)
	return "spell_choice_" .. client:CharacterID()
end

-- Disciplines are trained (Combat Abilities window); normal spells are scribed.
local function already_known(client, id)
	if eq.is_discipline(id) then
		return client:HasDisciplineLearned(id)
	end
	return client:HasSpellScribed(id)
end

-- Level-appropriate, not-yet-known candidates. Walk from the player's level
-- downward; if too few inside the band, widen all the way to level 1.
local function gather_candidates(client, level)
	local seen, cand = {}, {}
	local function collect(from, to)
		for lv = from, to, -1 do
			local list = pool[lv]
			if list then
				for _, sp in ipairs(list) do
					-- AoTv4: reject enchant-MATERIAL spells by name. The id blacklist only caught
					-- "Enchant <metal>"; the "Mass Enchant <metal>" line slipped through. Both are junk.
					local is_enchant = sp.name and (sp.name:match("^Enchant ") or sp.name:match("^Mass Enchant "))
					if not seen[sp.id] and not blacklist[sp.id] and not already_known(client, sp.id) and not is_enchant then
						seen[sp.id] = true
						cand[#cand + 1] = sp
					end
				end
			end
		end
	end

	local lo = math.max(1, level - LEVEL_BAND)
	collect(level, lo)
	if #cand < CHOICE_COUNT and lo > 1 then
		collect(lo - 1, 1)
	end
	return cand
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

-- The skill value a reward grants at the player's current level.
local function skill_target(client, id)
	return math.floor((client:MaxSkill(id) or 0) * SKILL_GRANT_PCT)
end

-- Combat skills worth offering: NOT native to this class, and not already earned.
--
-- ⚠️ CanHaveSkill is no longer a useful filter. aotv4_open_spells_and_skills.sql gives every class a
-- skill_caps entry for all twelve -- it has to, or a picked reward cannot be granted at all -- so
-- CanHaveSkill now answers true for everything and would offer a Rogue its own Backstab. The real
-- filter is skill_pool.NATIVE: a class already gets its native specials automatically on connect,
-- so offering one back would be a wasted pick.
--
-- Each ability is still offered at most once: once earned it never re-appears. (It keeps improving
-- through use after that.)
local function gather_skill_candidates(client)
	local out = {}
	for id, info in pairs(skills.rotatable_for(client:GetClass())) do
		if client:MaxSkill(id) > 0 and client:GetRawSkill(id) <= 0 then
			out[#out + 1] = { kind = "skill", id = id, name = info.name, icon = info.icon }
		end
	end
	return out
end

-- Send the window data (SPELLCHOICEDATA + SPELLDESCDATA) + saylink fallback for ONE offer, given its
-- comma-separated token string ("S:id,S:id,K:id"). Names/icons/descs are reconstructed from the tokens
-- so the queue only needs to store the tokens.
local function send_offer(client, offer_str)
	if not offer_str or offer_str == "" then return end
	local names, descs, labels = {}, {}, {}
	for tok in offer_str:gmatch("[^,]+") do
		local kind, id = tok:match("^(%a):(%d+)$"); id = tonumber(id)
		local nm, ic, ds
		if kind == "K" then
			local info = skills[id]
			nm = (info and info.name) or ("skill " .. id); ic = (info and info.icon) or 0; ds = "A combat skill."
		else
			nm = eq.get_spell_name(id) or ("spell " .. id); ic = icons[id] or 0
			-- ⚠️ SAME SOURCE AS THE SEARCH WINDOW. Client::SearchDetail("spell", id) gives mana, cast
			-- time, recast, range, learn level, resist, duration AND the db_str text with its #N/@N
			-- placeholders already filled from the effect values -- none of which spell_desc.lua has;
			-- that is the bare description only. One builder means a spell reads the same wherever it
			-- is shown, and the cast time is right there when comparing three choices.
			--
			-- ⚠️ SearchDetail separates lines with "~" because the search overlay draws itself. The
			-- picker's pane is an STML widget, so convert to <br> here. Same conversion the loot
			-- window's detail panel does, for the same reason.
			ds = client:SearchDetail("spell", id) or ""
			if ds == "" then ds = spelldesc[id] or "" end   -- fall back if the row vanished
			ds = ds:gsub("~", "<br>")
		end
		names[#names + 1]  = nm .. "|" .. tostring(ic)
		descs[#descs + 1]  = (ds:gsub("[%^|]", " "))
		labels[#labels + 1] = nm
	end
	if #names == 0 then return end
	if USE_DLL_TRIGGER then
		client:Message(MT.NPCQuestSay, "SPELLCHOICEDATA " .. table.concat(names, "^"))
		-- ⚠️ ONE LINE PER CHOICE, not all three joined with "^". Each description is now a full
		-- SearchDetail block rather than a single sentence, and three of them on one chat line runs
		-- past what the client will carry -- and an oversized line is truncated SILENTLY, so the third
		-- choice would just lose its text with nothing to show for it.
		-- The dll still understands the old joined form, so an older dll keeps working.
		for i, d in ipairs(descs) do
			client:Message(MT.NPCQuestSay, string.format("SPELLDESCDATA %d %s", i, d))
		end
		-- The reroll price rides along with every offer. It is per character and escalates, so the
		-- window has no way to work it out for itself and a hardcoded client-side figure would be
		-- wrong for everyone after their first reroll.
		M.send_reroll_cost(client)
	end
	if SHOW_SAYLINKS then
		client:Message(MT.Yellow, "Choose a new reward:")
		for i, nm in ipairs(labels) do
			client:Message(MT.LightBlue, string.format("  %d) %s   %s", i, nm,
				eq.say_link(SAY_TRIGGER .. " " .. i, true, "[ Select " .. nm .. " ]")))
		end
	end
end

-- Prune already-earned rewards from the FRONT offer, drop offers that become empty, save the queue, then
-- show the (pruned) front. Queued offers can go stale -- a spell/ability offered at level N may be learned
-- from a different offer before this one is resolved -- so we re-check right before showing, guaranteeing
-- you're never shown (or able to pick) a spell you already have OR a combat ability you already earned.
local function already_have(client, kind, id)
	if kind == "K" then return client:GetRawSkill(id) > 0 end   -- combat ability already trained
	return already_known(client, id)                            -- spell scribed / discipline learned
end

local function refresh_and_show(client, key)
	local q = eq.get_data(key) or ""
	while q ~= "" do
		local front, rest = q:match("^([^;]*);?(.*)$")
		local kept = {}
		for tok in front:gmatch("[^,]+") do
			local kind, id = tok:match("^(%a):(%d+)$"); id = tonumber(id)
			if id and not already_have(client, kind, id) then
				kept[#kept + 1] = tok
			end
		end
		if #kept > 0 then
			-- AoTv4: pruning already-known/stale tokens can leave fewer than CHOICE_COUNT. Top the offer
			-- back up with FRESH candidates so the player ALWAYS gets a full set of choices -- never a
			-- short 1-2 option window because one option was learned from an earlier offer.
			if #kept < CHOICE_COUNT then
				local in_offer = {}
				for _, t in ipairs(kept) do
					local k, i = t:match("^(%a):(%d+)$")
					if i then in_offer[k .. i] = true end
				end
				for _, sp in ipairs(pick_random(gather_candidates(client, client:GetLevel()), CHOICE_COUNT)) do
					if #kept >= CHOICE_COUNT then break end
					if not in_offer["S" .. sp.id] then
						kept[#kept + 1] = "S:" .. sp.id
						in_offer["S" .. sp.id] = true
					end
				end
			end
			q = (rest == "") and table.concat(kept, ",") or (table.concat(kept, ",") .. ";" .. rest)
			eq.set_data(key, q)
			send_offer(client, table.concat(kept, ","))
			return
		end
		q = rest   -- entire front offer is already-known -> discard it and try the next queued offer
	end
	eq.set_data(key, "")   -- nothing left to offer
end

------------------------------------------------------------------- public API
-- Offer a fresh set of choices. Call from event_level_up.
-- Build ONE offer's token string ("S:id,S:id,K:id") for this client at this level. Returns "" if
-- there is nothing left to offer.
--
-- ⚠️ Factored out of M.offer so that REROLL uses the exact same generator. If reroll had its own copy
-- the two would drift -- the skill chance, the level band and the shuffle all have to match, or a
-- rerolled offer would obey different rules from the one it replaced.
local function build_offer(client, level)
	-- Vary the offer EACH level-up. A per-(char,level) seed repeats the same 3 spells every run
	-- (roguelite re-levels constantly), so mix in a per-offer counter (bucket) -- like aa_choice.
	-- ⚠️ Reroll bumps the SAME counter, which is what makes a reroll produce a different set instead
	-- of re-deriving the one you just paid to get rid of.
	local sk = "spell_seed_" .. client:CharacterID()
	local n  = (tonumber(eq.get_data(sk)) or 0) + 1
	eq.set_data(sk, tostring(n))
	math.randomseed(client:CharacterID() * 2654435761 + level * 7919 + n * 104729)

	-- Mostly spells with ~1 combat skill: reserve one slot for a skill (when eligible),
	-- fill the rest with level-appropriate spells, then shuffle so the skill isn't last.
	local n_skill = 0
	local skill_cands = gather_skill_candidates(client)
	if #skill_cands > 0 and math.random() < SKILL_OFFER_CHANCE then n_skill = 1 end

	local choices = {}
	for _, sp in ipairs(pick_random(gather_candidates(client, level), CHOICE_COUNT - n_skill)) do
		choices[#choices + 1] = { kind = "spell", id = sp.id, name = sp.name, icon = icons[sp.id] or 0 }
	end
	for _, sc in ipairs(pick_random(skill_cands, n_skill)) do
		choices[#choices + 1] = sc
	end
	if #choices == 0 then return "" end
	choices = pick_random(choices, #choices)  -- shuffle order

	-- typed, ordered tokens so the pick is validated server-side:
	--   "S:<spellid>" = spell/discipline,  "K:<skillid>" = combat skill
	local toks = {}
	for _, c in ipairs(choices) do
		toks[#toks + 1] = (c.kind == "skill" and "K:" or "S:") .. c.id
	end
	return table.concat(toks, ",")
end

function M.offer(e)
	local client = e.self

	-- ⚠️⚠️ A FORFEITED LEVEL GETS NO OFFER. Keeping a spell through death costs the pick at the level
	-- that spell was originally taken (aotv4_spell_ranks_sys) -- that forfeit is the entire price of
	-- keeping, and it is charged HERE by simply not offering. Returning early rather than building
	-- and discarding an offer matters: build_offer bumps the RNG seed, so building one we throw away
	-- would silently change every later roll.
	local ok, ranksys = pcall(require, "aotv4_spell_ranks_sys")
	if ok and ranksys.pick_is_forfeit(client, client:GetLevel()) then
		client:Message(15, "You forfeit this level's reward -- the price of the spell you kept.")
		return
	end

	local this_offer = build_offer(client, client:GetLevel())
	if this_offer == "" then return end

	-- APPEND this offer to the pending QUEUE (offers separated by ";") rather than overwriting, so an
	-- un-picked offer from an earlier level-up is never lost. The window always shows the FRONT (oldest);
	-- picking one pops it and reveals the next.
	local q = eq.get_data(bucket_key(client)) or ""
	q = (q == "") and this_offer or (q .. ";" .. this_offer)
	eq.set_data(bucket_key(client), q)

	refresh_and_show(client, bucket_key(client))   -- prune known spells, then show the oldest pending offer
end

-- ---------------------------------------------------------------- reroll
-- Pay coin to replace the offer currently on screen with a fresh one.
--
-- ⚠️ IT REPLACES THE FRONT OFFER, NEVER APPENDS. The queue's front is what the window is showing, so
-- rerolling has to overwrite that entry -- appending would leave the disliked set still sitting there
-- to be picked, and the player would have paid for nothing.
--
-- ⚠️⚠️ THE ORDER OF THE THREE GATES IS LOAD BEARING: pending offer, then funds, then BUILD, and only
-- then charge. Taking the money before building would bill a player for a reroll that turns out to
-- have nothing left to offer, and there is no refund path once TakeMoneyFromPP has run.
function M.reroll(client)
	local key = bucket_key(client)
	local q   = eq.get_data(key) or ""
	if q == "" then
		client:Message(MT.Red, "You have no reward choice to reroll.")
		return false
	end

	local cost = M.reroll_cost(client)
	if (client:GetCarriedMoney() or 0) < cost then
		client:Message(MT.Red, string.format(
			"You cannot afford that. A reroll costs %dp, and it must be coin you are CARRYING, not banked.",
			math.floor(cost / 1000)))
		return false
	end

	-- ⚠️ Rerolled at the player's CURRENT level, not the level the offer was made at -- the queue does
	-- not record that. It only differs for an offer left un-picked across several levels, and taking
	-- the current level is both the friendlier reading and the one that matches what the Pool tab
	-- shows for "what can I be offered now".
	local fresh = build_offer(client, client:GetLevel())
	if fresh == "" then
		client:Message(MT.Red, "There is nothing else to offer you at this level. You were not charged.")
		return false
	end

	if not client:TakeMoneyFromPP(cost, true) then
		client:Message(MT.Red, "The coin could not be taken from you. Nothing was rerolled.")
		return false
	end

	local _, rest = q:match("^([^;]*);?(.*)$")
	eq.set_data(key, (rest ~= "" and (fresh .. ";" .. rest) or fresh))
	eq.set_data(reroll_key(client), tostring(reroll_count(client) + 1))

	client:Message(MT.Yellow, string.format(
		"You spend %dp. Three new paths open before you -- the next reroll will cost %dp.",
		math.floor(cost / 1000), math.floor(M.reroll_cost(client) / 1000)))
	refresh_and_show(client, key)
	M.send_reroll_cost(client)
	return true
end

-- Discard any un-picked reward offers still queued. Call on death (roguelite reset) so a new run doesn't
-- inherit a stale high-level offer from the previous life.
function M.clear_pending(client)
	eq.set_data(bucket_key(client), "")
end

-- Tell the client (eq-core-dll skill-unlock hook) which combat skills the player has EARNED
-- (value > 0) so it reveals only those. Call on connect and after each skill pick. The dll
-- swallows this line; with no client mod it's harmless noise the player won't normally see.
function M.send_unlocks(client)
	local earned = {}
	for id, _ in pairs(skills.SKILLS) do
		if client:GetRawSkill(id) > 0 then earned[#earned + 1] = tostring(id) end
	end
	client:Message(MT.NPCQuestSay, "SKILLUNLOCKDATA " .. table.concat(earned, ","))

	-- Nudge the client to REBUILD its Combat Abilities list. The client builds that list at zone-in,
	-- BEFORE the SKILLUNLOCKDATA chat line above reaches the dll, so earned combat abilities stayed
	-- invisible/unusable until the player forced a rebuild (jumping did it). Re-sending each earned
	-- skill's own value (OP_SkillUpdate, no skill-up spam) triggers that rebuild now that the dll has
	-- the earned set -- so abilities appear on login without the jump.
	for id, _ in pairs(skills.SKILLS) do
		local v = client:GetRawSkill(id)
		if v > 0 then client:SetSkill(id, v) end
	end
end

-- =================================================================================================
-- Spell Journal (core_spelljournal.cpp) -- "what can still be offered to me at level N".
--
-- The Journal's KNOWN tab needs no server help at all: the dll reads the character's spellbook
-- straight out of CHARINFO2. Only the POOL tab needs us, because the pool is a Lua construct -- the
-- client cannot know spell_pool.lua, spell_blacklist.lua or the enchant-name reject.
--
-- ⚠️ IDS ONLY, NO NAMES. The dll resolves names through GetSpellByID against the client's own spell
-- data. Names would roughly quadruple the payload for nothing: level 70 holds 125 pool spells, so
-- "id:known" comes to ~1.1 KB while "id|name|known" would be ~4 KB.
--
-- ⚠️ AND IT IS CHUNKED ANYWAY. A single oversized chat line is silently truncated, which would look
-- exactly like a short pool rather than an error -- and CLAUDE.md section 15 already records the RoF2
-- chat pipe dropping and reordering bursts. Each line carries its own chunk index so the dll can
-- tell a complete answer from a partial one, and the count is small (3 lines at the worst level).
local JOURNAL_TRIGGER = "sjpool"
local JOURNAL_CHUNK   = 60

-- ---------------------------------------------------------------- pick levels
-- "what level was I when I took this", per character, as "id:lvl,id:lvl,...".
--
-- ⚠️ Kept in a data bucket rather than derived, because it CANNOT be derived: the character record
-- stores which spells are scribed, never when. Written at the moment of the pick and never rewritten,
-- so it is the level at acquisition and not the level of some later re-scribe.
local function level_key(client) return "spelllvl_" .. client:CharacterID() end

function M.record_pick_level(client, spell_id)
	local key  = level_key(client)
	local cur  = eq.get_data(key) or ""
	local lvl  = client:GetLevel()

	-- ⚠️⚠️ THE MOST RECENT ACQUISITION WINS -- this used to keep the FIRST record and never rewrite it.
	-- That predates the roguelite: a character now re-takes spells from scratch every run, so "the
	-- first time I ever saw this" is a fact about a character that no longer exists. Ashrem's bucket
	-- held 21 entries at levels 44 and 50, recorded before the cap came down to 35.
	--
	-- ⚠️ It is not merely a confusing display. The kept-spell FORFEIT charges the pick at this level
	-- (aotv4_spell_ranks_sys), so a level recorded above the current cap can NEVER come round again
	-- and keeping that spell is silently free -- the one thing the forfeit exists to prevent.
	local out = {}
	for e in cur:gmatch("([^,]+)") do
		local id = tonumber(e:match("^(%d+):"))
		if id and id ~= spell_id then out[#out + 1] = e end
	end
	out[#out + 1] = spell_id .. ":" .. lvl
	eq.set_data(key, table.concat(out, ","))
end

-- Send the map to the spell window. Chunked for the same reason the pool is: a character can hold
-- one pick per level, so this grows with the level cap and an oversized chat line truncates silently.
function M.send_pick_levels(client)
	local all = eq.get_data(level_key(client)) or ""
	local rows = {}
	for e in all:gmatch("[^,]+") do rows[#rows + 1] = e end

	local chunks = math.max(1, math.ceil(#rows / JOURNAL_CHUNK))
	for c = 1, chunks do
		local part = {}
		for i = (c - 1) * JOURNAL_CHUNK + 1, math.min(c * JOURNAL_CHUNK, #rows) do
			part[#part + 1] = rows[i]
		end
		client:Message(MT.NPCQuestSay, string.format("SJLEVELS %d %d^%s", c, chunks, table.concat(part, ",")))
	end
end

function M.send_pool(client, level)
	if level < 1 then level = 1 end

	-- ⚠️⚠️ THE BAND, NOT THE EXACT LEVEL. gather_candidates offers everything learnable in
	-- [level - LEVEL_BAND, level], so a tab showing only pool[level] answers a different question than
	-- the picker does -- and misses most of what you can actually be offered.
	--
	-- That is not a subtle discrepancy. The custom lines (Reptile, Sloth, Moonfire, Promised, Kindred,
	-- Mark, Thirst) exist ONLY at levels 8/18/28/38/48/58, so at any level in between the tab showed
	-- none of them at all while the picker was happily offering them. They looked missing from the
	-- pool entirely.
	--
	-- Same filters as gather_candidates EXCEPT the already-known reject: the tab shows what you own
	-- too, flagged, so the window can grey it out. Keep the rest in step or the tab will advertise
	-- something the picker will never hand you.
	local rows, seen = {}, {}

	-- ⚠️ COMBAT ABILITIES ARE NOT IN spell_pool -- they are a SEPARATE pool (skill_pool.lua) and
	-- without this the tab silently answered "these are not obtainable", which is wrong: the picker
	-- spends one of its three slots on one roughly every eighth level (SKILL_OFFER_CHANCE).
	-- They are NOT level-banded either -- gather_skill_candidates offers rotatable_for(class) at any
	-- level -- so they are listed once, ahead of the spells, at every level.
	-- Wire form is "k<skillid>:<known>:<name>": a skill id (0-76) would otherwise be read as a spell
	-- id, and the client has no skill-name table of its own to resolve it with (a hardcoded one would
	-- drift from this file the moment the list changed -- the kIcons lesson, CLAUDE.md section 3).
	local rot = skills.rotatable_for(client:GetClass())
	for id, info in pairs(rot) do
		rows[#rows + 1] = "k" .. id .. ":" ..
			(((client:GetSkill(id) or 0) > 0) and "1" or "0") .. ":" .. info.name
	end

	local lo = math.max(1, level - LEVEL_BAND)
	for lv = level, lo, -1 do
		local list = pool[lv]
		if list then
			for _, sp in ipairs(list) do
				local is_enchant = sp.name and (sp.name:match("^Enchant ") or sp.name:match("^Mass Enchant "))
				if not seen[sp.id] and not blacklist[sp.id] and not is_enchant then
					seen[sp.id] = true
					rows[#rows + 1] = sp.id .. ":" .. (already_known(client, sp.id) and "1" or "0")
				end
			end
		end
	end

	local chunks = math.max(1, math.ceil(#rows / JOURNAL_CHUNK))
	for c = 1, chunks do
		local part = {}
		for i = (c - 1) * JOURNAL_CHUNK + 1, math.min(c * JOURNAL_CHUNK, #rows) do
			part[#part + 1] = rows[i]
		end
		client:Message(MT.NPCQuestSay, string.format("SJPOOLDATA %d %d %d^%s",
			level, c, chunks, table.concat(part, ",")))
	end
end

-- Call from event_say alongside handle_say; returns true if it consumed the message.
function M.handle_journal_say(e)
	local lv = (e.message or ""):lower():match("^" .. JOURNAL_TRIGGER .. "%s+(%d+)%s*$")
	if lv then
		M.send_pool(e.self, tonumber(lv))
		return true
	end

	-- "sjinfo <spellid>" -- readable detail for the Known and Pool tabs.
	--
	-- ⚠️ THE CLIENT CANNOT WRITE THIS TEXT ITSELF, which is why this exists. The dll rendered the
	-- detail from the client's own spell record using a hand-kept SPA-to-label table, and every SPA
	-- missing from that table fell through to a raw line like "SPA 137: 0" -- meaningless to a player,
	-- and there are hundreds of SPAs so the table can never be complete.
	-- SearchDetail carries the db_str description with its #N/@N placeholders already filled, which is
	-- the text a human can actually read, and it is the SAME source the picker, the search window and
	-- the loot window now use.
	if (e.message or ""):lower():match("^sjlevels%s*$") then
		M.send_pick_levels(e.self)
		return true
	end

	-- The picker window's Reroll button. Routed here rather than through handle_say so it shares the
	-- journal's "consumed, never reaches chat" path -- the dll swallows the echo either way.
	if (e.message or ""):lower():match("^spellreroll%s*$") then
		M.reroll(e.self)
		return true
	end

	local sid = (e.message or ""):lower():match("^sjinfo%s+(%d+)%s*$")
	if sid then
		sid = tonumber(sid)

		-- ⚠️ IDS AT OR ABOVE SKILL_ID_BASE ARE COMBAT ABILITIES, NOT SPELLS. The Pool tab lists both
		-- and the client asks about a row by the one id it was given, so the two id spaces are
		-- separated by an offset rather than by a second command. SearchDetail("spell", 8) would
		-- happily describe spell 8 -- a real spell, and nothing to do with Backstab.
		if sid >= SKILL_ID_BASE then
			local skid = sid - SKILL_ID_BASE
			local info = skills.SKILLS[skid]
			local c    = e.self
			local cur, cap = c:GetSkill(skid) or 0, c:MaxSkill(skid) or 0
			local lines = { (info and info.name or ("Skill #" .. skid)) .. "  (combat ability)" }
			if cur > 0 then
				lines[#lines + 1] = string.format("Trained: %d of %d", cur, cap)
				lines[#lines + 1] = "Fires from the Combat Abilities window, or automatically if you"
				lines[#lines + 1] = "enable it in the Autoskill window."
			else
				lines[#lines + 1] = string.format("Not trained.  Cap at your level: %d", cap)
				lines[#lines + 1] = "Offered as a level-up reward; it is granted at a quarter of cap."
			end
			c:Message(MT.NPCQuestSay, "SJINFO " .. sid .. " " .. table.concat(lines, "~"))
			return true
		end

		local txt = e.self:SearchDetail("spell", sid) or ""
		e.self:Message(MT.NPCQuestSay, "SJINFO " .. sid .. " " .. txt)
		return true
	end

	return false
end

-- Resolve a pick. Call from event_say; returns true if it consumed the message.
function M.handle_say(e)
	local idx = (e.message or ""):lower():match("^" .. SAY_TRIGGER .. "%s+(%d+)%s*$")
	if not idx then return false end
	idx = tonumber(idx)

	local client = e.self
	local key  = bucket_key(client)
	local q    = eq.get_data(key)
	if not q or q == "" then
		client:Message(MT.Red, "You have no spell choice pending.")
		return true
	end

	-- the window shows the FRONT offer (oldest); the rest stay queued
	local front, rest = q:match("^([^;]*);?(.*)$")
	local toks = {}
	for s in front:gmatch("([^,]+)") do toks[#toks + 1] = s end
	local tok = toks[idx]
	if not tok then
		client:Message(MT.Red, "That is not a valid choice.")
		return true
	end

	local kind, id = tok:match("^(%a):(%d+)$")
	id = tonumber(id)
	if not kind or not id then
		client:Message(MT.Red, "That is not a valid choice.")
		return true
	end

	if kind == "K" then
		-- combat skill reward: granted ONCE (offered only while unearned). Reject if already trained.
		local info   = skills[id]
		local sname  = info and info.name or ("skill " .. id)
		if client:GetRawSkill(id) > 0 then
			client:Message(MT.Red, "You already have " .. sname .. ".")
			return true  -- leave the choice pending so they can pick another
		end
		client:SetSkill(id, skill_target(client, id))
		client:Message(MT.Yellow, "You have trained " .. sname .. "!")
		M.send_unlocks(client)  -- reveal the newly-earned skill on the client
	else
		-- spell / discipline reward
		local spell_id = id
		local name = eq.get_spell_name(spell_id)
		if already_known(client, spell_id) then
			client:Message(MT.Red, "You already know " .. name .. ".")
			return true  -- leave the choice pending so they can pick another
		end
		if eq.is_discipline(spell_id) then
			client:TrainDiscBySpellID(spell_id)
			client:Message(MT.Yellow, "You have learned the discipline " .. name .. "!")
		else
			local slot = client:GetNextAvailableSpellBookSlot()
			if slot < 0 then
				client:Message(MT.Red, "Your spellbook is full -- make room and pick again.")
				return true
			end
			-- ⚠️⚠️ AWARD AT THE CHARACTER'S EARNED RANK, NOT AT RANK 1. Ranks are permanent
			-- meta-progression: once Superior Healing has been taken to rank III, EVERY future copy
			-- arrives at rank III, from here or from a kept slot. Scribing the base id here instead
			-- would silently hand back a rank-1 spell and the player's permanent progress would look
			-- like it had vanished -- with no error anywhere.
			-- ⚠️ ranked_id() falls back to the base id when the spell has no rank chain, so unranked
			-- spells are unaffected.
			local ok_rs, ranksys = pcall(require, "aotv4_spell_ranks_sys")
			local give = ok_rs and ranksys.ranked_id(client, spell_id) or spell_id
			client:ScribeSpell(give, slot)
			-- Permanent library: the Known tab is what this character has EVER been awarded, not what
			-- happens to be scribed right now, so it is recorded here and never wiped.
			if ok_rs then ranksys.note_known(client, spell_id) end
			client:Message(MT.Yellow, "You have learned " .. (eq.get_spell_name(give) or name) .. "!")
		end

		-- Record the level the reward was TAKEN at, for the spell window's Known tab.
		--
		-- ⚠️ THIS IS THE ONLY PLACE THE PICK LEVEL EXISTS. Nothing in the character record remembers
		-- when a spell was scribed -- CHARINFO2 knows only WHAT you have -- so if it is not written
		-- here it cannot be reconstructed afterwards. A spell obtained any other way (a GM grant, a
		-- pre-existing character) will therefore have no entry and shows no level.
		M.record_pick_level(client, spell_id)
	end

	-- resolved: drop the front offer, keep the queued rest, then prune + reveal the next pending offer
	eq.set_data(key, rest)
	refresh_and_show(client, key)
	return true
end

return M
