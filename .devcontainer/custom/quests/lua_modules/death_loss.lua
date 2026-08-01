-- death_loss.lua
-- Roguelite death: destroy the player's CARRIED gear/inventory/money and wipe all spells; the
-- bank and epic weapons are kept. Runs from global_player.lua event_death, where the inventory is
-- still intact (EVENT_DEATH fires before the corpse is made, and LeaveNakedCorpses keeps items on
-- the player). Bank (slots 2000+) and shared bank (2500+) are never touched.

local epics  = require("epic_items")   -- { [item_id]=true } for items.epicitem<>0
local skills = require("skill_pool")   -- reward-gated combat skill ids (re-earned via level-up picks)

local M = {}

-- RoF2 slot ranges to wipe. Bag CONTENTS first (251-360) so a bag's items are recorded before the
-- bag itself is destroyed; then possessions (0-33 = worn + 10 general + cursor).
local WIPE = { { 251, 360 }, { 0, 33 } }

local function fmt_money(c)
	local p = math.floor(c / 1000); c = c % 1000
	local g = math.floor(c / 100);  c = c % 100
	local s = math.floor(c / 10);   local cp = c % 10
	local parts = {}
	if p  > 0 then parts[#parts + 1] = p  .. "p" end
	if g  > 0 then parts[#parts + 1] = g  .. "g" end
	if s  > 0 then parts[#parts + 1] = s  .. "s" end
	if cp > 0 then parts[#parts + 1] = cp .. "c" end
	return table.concat(parts, " ")
end

-- Destroy carried items + money + spells; return a list of lost-item names (for the report).
-- ⚠️⚠️ EVOLVING ITEMS SURVIVE THE ROGUELITE DEATH. Everything else in the wipe range goes, but an
-- evolving item is META progression -- the Delver's Sigil takes tens of delve clears to grow, exactly
-- like AA used to, and destroying it on a death would make the whole evolving system pointless on a
-- server where death is routine and expected.
--
-- ⚠️ Tested by PROPERTY, not by an id list. `IsEvolving()` covers the sigil and anything evolving
-- added later without this file needing to know it exists -- an id list here would silently stop
-- protecting the next one. It also keeps death_loss from having to require the dungeon module, which
-- would drag the whole delve system into the death path.
-- ⚠️ Epics are already exempt separately, above; this is an additional rule, not a replacement.
function M.is_kept(inv, slot)
	if not inv then return false end
	local inst = inv:GetItem(slot)
	if not inst or not inst.valid then return false end
	return inst:IsEvolving() and true or false
end

function M.process(client)
	local lost = {}
	local inv  = client:GetInventory()
	for _, rng in ipairs(WIPE) do
		for slot = rng[1], rng[2] do
			local id = client:GetItemIDAt(slot)
			if id and id > 0 and not epics[id] and not M.is_kept(inv, slot) then
				lost[#lost + 1] = eq.get_item_name(id)
				client:DeleteItemInInventory(slot, 0, true)   -- 0 = delete the whole item
			end
		end
	end

	local money = client:GetCarriedMoney() or 0               -- total carried copper (bank is separate)
	if money > 0 then
		client:TakeMoneyFromPP(money, true)
		lost[#lost + 1] = fmt_money(money) .. " in coin"
	end

	-- ⚠️⚠️ CLASS AURAS SURVIVE DEATH, AND THEY HAVE TO -- THEY ARE UNRECOVERABLE.
	-- The 43500-43515 auras are not level-up rewards; each is granted ONCE by its class's
	-- "First Blood" achievement (custom_achievement_rewards, reward_type `scribe_spell`). An
	-- achievement only fires on COMPLETION, and completion is permanent, so once the roguelite wipe
	-- unscribed the aura there was no path back to it at all -- not by levelling, not by dying again,
	-- not by re-earning the achievement. Every other thing this function destroys can be re-earned;
	-- this one could not, which made a single death silently and permanently remove a character's
	-- class ability.
	-- ⚠️ Captured BEFORE the wipe and re-scribed after: there is no "unscribe all except" binding, so
	-- the only way to exempt a spell is to put it back.
	-- ⚠️ Kept in step with the achievement rewards. If the aura band ever moves, this moves with it,
	-- or deaths start eating auras again with no error anywhere.
	local AURA_FIRST, AURA_LAST = 43500, 43515
	local keep_auras = {}
	for id = AURA_FIRST, AURA_LAST do
		if client:HasSpellScribed(id) then keep_auras[#keep_auras + 1] = id end
	end

	-- Spells the player chose to KEEP, and how many are about to be destroyed (one Parchment Fragment
	-- each). ⚠️ Counted BEFORE the wipe -- afterwards there is nothing left to count.
	local ok_rs, ranksys = pcall(require, "aotv4_spell_ranks_sys")
	local destroyed = 0
	if ok_rs then destroyed = ranksys.on_death_before_wipe(client) end

	client:UnmemSpellAll(false)                              -- clear gems (false = no per-spell spam;
	client:UnscribeSpellAll(false)                           -- the client refreshes on the death-zone)
	client:UntrainDiscAll(false)                             -- clear TRAINED disciplines too -- they live in
	                                                         -- character_disciplines (separate from the spellbook),
	                                                         -- so UnscribeSpellAll misses them and the Combat
	                                                         -- Abilities window keeps them across a roguelite death.

	-- Put the auras back. The book is empty at this point, so slots 0..n-1 are free.
	-- ⚠️ A character is ONE class and each aura is class-gated, so this is normally a single spell;
	-- the loop is defensive rather than expected to iterate.
	for i, id in ipairs(keep_auras) do
		client:ScribeSpell(id, i - 1)
	end

	-- ⚠️⚠️ KEPT SPELLS START AFTER THE AURAS. ScribeSpell overwrites whatever occupies the slot
	-- without complaint, so two writers both starting at slot 0 silently destroy one another.
	if ok_rs then ranksys.on_death_after_wipe(client, #keep_auras) end

	-- Pay the fragments: one per spell the wipe destroyed, which is the rank system's currency.
	-- ⚠️ Awarded after the wipe but COUNTED before it -- counting here would always yield 0.
	if destroyed > 0 then
		client:SummonItem(147920, destroyed)   -- Parchment Fragment
		lost[#lost + 1] = string.format("(%d spells left Parchment Fragments behind)", destroyed)
	end

	lost[#lost + 1] = "All memorized spells, disciplines, and your spellbook"
	if #keep_auras > 0 then
		-- Say it plainly in the loss report. The report is the player's record of what a death cost,
		-- so the one thing that was NOT taken belongs there too -- otherwise they reasonably assume
		-- the aura is gone and stop looking for it.
		lost[#lost + 1] = "(your class aura was spared)"
	end

	-- reward-gated COMBAT abilities (Backstab/Kick/etc.) -> reset to 0 so they're re-earned via the
	-- level-up picker. (Caller re-sends SKILLUNLOCKDATA so the client re-hides them.)
	local reset_combat = false
	for id in pairs(skills.SKILLS) do
		if (client:GetSkill(id) or 0) > 0 then client:SetSkill(id, 0); reset_combat = true end
	end
	if reset_combat then lost[#lost + 1] = "Your combat abilities" end

	return lost
end

-- Report the loss. The dll "You Lost" window renders the list (swallows LOSTDATA); we send just one
-- flavor line to chat so there's no duplicate list. (SHOW_CHAT_LIST = fallback when no client mod.)
local SHOW_CHAT_LIST = false
function M.announce(client, lost, killer)
	client:Message(MT.Red, "Death strips you bare -- everything you carried is gone.")
	if SHOW_CHAT_LIST then
		for _, name in ipairs(lost) do client:Message(MT.Red, "   - " .. name) end
	end
	local clean = {}
	for _, name in ipairs(lost) do clean[#clean + 1] = (name:gsub("[%^|]", " ")) end  -- ^ | are delimiters
	client:Message(MT.NPCQuestSay, "LOSTDATA " .. table.concat(clean, "^"))

	M.record(client, clean, killer)
end

-- ---------------------------------------------------------------- permanent loss log
-- Everything this character has ever lost, newest death first, as "epoch|name~epoch|name~...".
--
-- ⚠️ THE OLD BEHAVIOUR KEPT NOTHING. LOSTDATA was pushed once at the moment of death and the window
-- rendered it from memory, so the list was gone the moment it was dismissed or the UI reloaded --
-- which is exactly when a player wants to go back and look at what a death cost them.
--
-- ⚠️ CAPPED, because a data bucket is one string and this grows without bound otherwise. The cap is
-- generous (LOG_MAX entries, oldest dropped first) and the same shape as the shop's price log.
local LOG_MAX = 400

local function log_key(client) return "lostlog_" .. client:CharacterID() end

function M.record(client, names, killer)
	if not names or #names == 0 then return end
	local now = os.time()
	-- ⚠️ strip the delimiters from the killer name too. "a_ghoul|x" would split the entry.
	local who = ((killer or "") :gsub("[%^|~]", " "))

	-- newest first, so the window reads like a mailbox without the client having to sort
	local fresh = {}
	for _, n in ipairs(names) do fresh[#fresh + 1] = now .. "|" .. who .. "|" .. n end

	local prev = eq.get_data(log_key(client)) or ""
	for e in prev:gmatch("[^~]+") do
		if #fresh >= LOG_MAX then break end
		fresh[#fresh + 1] = e
	end

	eq.set_data(log_key(client), table.concat(fresh, "~"))
end

-- Send the log to the "You Lost" window. Chunked: this is the one list on the server that grows for
-- the life of the character, so it WILL outgrow a single chat line.
local LOG_CHUNK = 40

function M.send_log(client)
	local all  = eq.get_data(log_key(client)) or ""
	local rows = {}
	for e in all:gmatch("[^~]+") do rows[#rows + 1] = e end

	local chunks = math.max(1, math.ceil(#rows / LOG_CHUNK))
	for c = 1, chunks do
		local part = {}
		for i = (c - 1) * LOG_CHUNK + 1, math.min(c * LOG_CHUNK, #rows) do
			part[#part + 1] = rows[i]
		end
		client:Message(MT.NPCQuestSay,
			string.format("LOSTLOG %d %d %d^%s", c, chunks, #rows, table.concat(part, "^")))
	end
end

return M
