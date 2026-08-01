-- AoTv4 spell ranks -- permanent spell knowledge, ranks that persist, and keeping 2 through death.
-- =================================================================================================
-- ⚠️⚠️ RANK IS A PERMANENT PROPERTY OF THE CHARACTER, NOT OF A SCRIBED COPY.
-- Earn rank III on Superior Healing once and EVERY future award of that spell arrives at rank III --
-- from the level-up picker, from a kept slot, from anywhere. Ranks are meta-progression, like AAs:
-- they survive the roguelite wipe because they live in a data bucket, not in the spellbook.
--
-- That decoupling is the whole design:
--     KNOWN   every spell this character has ever been awarded. Permanent. Never wiped.
--     RANK    per base spell, 1-5. Permanent. Applies to every future copy.
--     KEPT    at most 2 spells you START a run with. This is the only thing death touches.
--
-- ⚠️ Keeping is therefore NOT how you protect a rank -- the rank was never at risk. Keeping only
-- buys you the spell in hand at level 1, paid for by forfeiting the level-up pick at the level that
-- spell was originally taken.
--
-- ⚠️⚠️ A RANK IS A DIFFERENT SPELL ROW, NOT A MODIFIER. spell_ranks.lua maps base -> {r2,r3,r4,r5}
-- (real rows at 43576-44327 from gen_spell_ranks.pl). "Applying a rank" means scribing a different
-- id. That is why the native spellbook shows the real damage and the real mana cost.
--
-- ⚠️ Rank rows are named "... Rk. II".."Rk. V", which gen_stock_pool.pl filters out of the offerable
-- pool (`name NOT LIKE '% Rk. %'`), so the picker can only ever award a BASE spell -- which this
-- module then upgrades on the way in. Renaming them would put 752 upgraded spells in the reward pool.

local ranks = require("spell_ranks")

local M = {}

M.MAX_KEPT      = 2
M.FRAGMENT_ITEM = 147920
M.INK_ITEM      = 147921

-- ⚠️ Fragments DOUBLE per rank (against ~31 from a deep death); ink is a flat 10 per rank. The two
-- scale differently on purpose: fragments come from dying, so they track how hard you have been
-- pushing, while ink comes from killing, so it tracks time played. A rank costs both.
-- ⚠️ Tune HERE only. This table is sent to the client as SPELLRANKCOST, so the window renders
-- whatever it says -- there is no second copy in the dll to keep in step (the kIcons drift trap).
M.COST = {
    [2] = { frag =  30, ink = 10 },
    [3] = { frag =  60, ink = 20 },
    [4] = { frag = 120, ink = 30 },
    [5] = { frag = 240, ink = 40 },
}

-- ⚠️⚠️ FEEDBACK GOES TO THE WINDOW, NOT TO CHAT. Keeping and ranking are actions taken inside a
-- window that is already on screen, so answering them in the chat log is both noise and the wrong
-- place to look -- and it is the player's OWN chat, which they may be using for something. SPELLRANKMSG
-- is swallowed by the dll and written into the requirement panel under the description.
-- ⚠️ The dll must ALSO swallow the "/say spellkeep" style echoes (core_spellwindow's chat detour) or
-- the commands themselves stay visible even with every reply silenced. Both halves are needed.
local function notify(c, text)
    c:Message(MT.NPCQuestSay, "SPELLRANKMSG " .. (tostring(text or ""):gsub("[%^|~]", " ")))
end

local function rkey(c) return "spellrank_"  .. c:CharacterID() end  -- base:rank  PERMANENT
local function nkey(c) return "spellknown_" .. c:CharacterID() end  -- base list   PERMANENT
local function kkey(c) return "spellkeep_"  .. c:CharacterID() end  -- base list   which 2 survive
local function pkey(c) return "pickspent_"  .. c:CharacterID() end  -- forfeited levels this run
-- ⚠️ The origin level is SPELL_CHOICE'S bucket, written at the moment of the pick. Keeping a second
-- copy here would be two sources of truth for one fact and would charge the wrong level once they
-- drifted. Never derive it from the spell's own classes8 -- that is the level it becomes AVAILABLE.
local function okey(c) return "spelllvl_"   .. c:CharacterID() end

local function split(s, sep)
    local out = {}
    if s and s ~= "" then for v in s:gmatch("([^" .. (sep or ",") .. "]+)") do out[#out + 1] = v end end
    return out
end

-- ---------------------------------------------------------------- permanent knowledge
function M.known(c)
    local out = {}
    for _, v in ipairs(split(eq.get_data(nkey(c)))) do out[#out + 1] = tonumber(v) end
    return out
end

function M.is_known(c, base)
    for _, id in ipairs(M.known(c)) do if id == base then return true end end
    return false
end

-- Record that this character has been awarded a spell. Called from spell_choice at award time and
-- from the keep path, so the Known tab is a permanent library rather than a view of the spellbook.
function M.note_known(c, base)
    base = ranks.base[base] or base
    if M.is_known(c, base) then return end
    local raw = eq.get_data(nkey(c)) or ""
    eq.set_data(nkey(c), (raw ~= "" and raw .. "," or "") .. base)
end

-- ---------------------------------------------------------------- permanent rank
function M.rank_of(c, base)
    base = ranks.base[base] or base
    for _, e in ipairs(split(eq.get_data(rkey(c)))) do
        local id, rk = e:match("^(%d+):(%d+)$")
        if id and tonumber(id) == base then return tonumber(rk) end
    end
    return 1
end

local function set_rank(c, base, rank)
    local out, done = {}, false
    for _, e in ipairs(split(eq.get_data(rkey(c)))) do
        local id = tonumber(e:match("^(%d+):"))
        if id == base then out[#out + 1] = base .. ":" .. rank; done = true
        else out[#out + 1] = e end
    end
    if not done then out[#out + 1] = base .. ":" .. rank end
    eq.set_data(rkey(c), table.concat(out, ","))
end

-- ⚠️⚠️ THE ONE FUNCTION EVERYTHING ELSE GOES THROUGH. Given a base spell, return the id that should
-- actually be scribed for this character. Every path that hands a player a spell must call this, or
-- that path silently awards rank 1 and the player's permanent progress appears to have vanished.
function M.ranked_id(c, base)
    base = ranks.base[base] or base
    local rk = M.rank_of(c, base)
    if rk <= 1 then return base end
    local chain = ranks.chain[base]
    -- ⚠️ Fall back to the base if the chain is missing: a regen that narrowed the ranked set must
    -- degrade to an unranked spell, never to nil (which would scribe nothing at all).
    return chain and chain[rk - 1] or base
end

-- ---------------------------------------------------------------- kept spells
function M.kept(c)
    local out = {}
    for _, v in ipairs(split(eq.get_data(kkey(c)))) do out[#out + 1] = tonumber(v) end
    return out
end

-- Is this spell in the character's book RIGHT NOW, at any rank?
-- ⚠️ Check the base AND every rank id: after an upgrade the scribed copy is a different spell id, so
-- testing only the base would report a ranked spell as not carried.
function M.currently_scribed(c, base)
    base = ranks.base[base] or base
    if c:HasSpellScribed(base) then return true end
    for _, rid in ipairs(ranks.chain[base] or {}) do
        if c:HasSpellScribed(rid) then return true end
    end
    return false
end

-- ⚠️⚠️ KEEP AND UPGRADE HAVE DELIBERATELY DIFFERENT GATES.
--   KEEP    only from spells you are CARRYING RIGHT NOW -- it is a choice about this run's loadout,
--           made from what this run actually gave you, and capped at 2.
--   UPGRADE anything you have ever DISCOVERED -- rank is permanent character progress and is not
--           hostage to whether the spell happens to be in your book today.
-- Using the same gate for both would either forbid ranking a spell you are between copies of, or
-- let a player pre-load keeps from their entire history at the start of a run.
function M.keep(c, spell_id)
    local base = ranks.base[spell_id] or spell_id
    if not M.currently_scribed(c, base) then
        notify(c, "You can only keep a spell you are currently carrying.")
        return false
    end
    M.note_known(c, base)
    local list = M.kept(c)
    for _, id in ipairs(list) do
        if id == base then notify(c, "That spell is already kept.") return false end
    end
    if #list >= M.MAX_KEPT then
        notify(c, string.format("You can only keep %d spells. Release one first.", M.MAX_KEPT))
        return false
    end
    list[#list + 1] = base
    eq.set_data(kkey(c), table.concat(list, ","))
    local lv = M.origin_of(c, base)
    notify(c, string.format("%s will be in hand at the start of each run.%s",
        eq.get_spell_name(base) or ("spell " .. base),
        lv and string.format(" You forfeit your level %d pick.", lv) or ""))
    return true
end

function M.release(c, spell_id)
    local base = ranks.base[spell_id] or spell_id
    local out, gone = {}, false
    for _, id in ipairs(M.kept(c)) do
        if id == base then gone = true else out[#out + 1] = id end
    end
    eq.set_data(kkey(c), table.concat(out, ","))
    -- ⚠️ Releasing costs NO rank. The rank is permanent and independent of keeping, so this is always
    -- a safe, reversible decision -- which is what makes it worth offering at all.
    notify(c, gone and "That spell will no longer start in hand." or "You were not keeping that.")
end

function M.origin_of(c, spell_id)
    local base = ranks.base[spell_id] or spell_id
    for _, e in ipairs(split(eq.get_data(okey(c)))) do
        local id, lv = e:match("^(%d+):(%d+)$")
        if id and tonumber(id) == base then return tonumber(lv) end
    end
    return nil
end

-- ---------------------------------------------------------------- death
-- Called BEFORE the wipe. Returns how many spells are about to be destroyed, one Parchment Fragment
-- each. ⚠️ Kept spells still count: they are re-scribed rather than spared, and excluding them would
-- make keeping the best way to farm currency as well as the best way to keep a spell.
function M.on_death_before_wipe(c)
    local n = 0
    for base, chain in pairs(ranks.chain) do
        if c:HasSpellScribed(base) then n = n + 1 end
        for _, rid in ipairs(chain) do if c:HasSpellScribed(rid) then n = n + 1 end end
    end
    return n
end

-- Called AFTER the wipe. Re-scribe the kept spells AT THEIR EARNED RANK and charge the forfeit.
-- ⚠️ `first_slot` because the class auras are re-scribed by the same caller and take the low slots;
-- ScribeSpell overwrites without complaint, so two writers starting at 0 destroy one another.
function M.on_death_after_wipe(c, first_slot)
    local spent, slot = {}, first_slot or 0
    for _, base in ipairs(M.kept(c)) do
        c:ScribeSpell(M.ranked_id(c, base), slot)
        slot = slot + 1
        local lv = M.origin_of(c, base)
        if lv then spent[#spent + 1] = lv end
    end
    eq.set_data(pkey(c), table.concat(spent, ","))
    if #spent > 0 then
        c:Message(15, string.format("Your kept spells return. You forfeit your level %s pick%s.",
            table.concat(spent, " and "), #spent == 1 and "" or "s"))
    end
end

function M.pick_is_forfeit(c, level)
    for _, lv in ipairs(split(eq.get_data(pkey(c)))) do
        if tonumber(lv) == level then return true end
    end
    return false
end

-- ---------------------------------------------------------------- upgrading
-- What the next rank costs, and whether it can be afforded. Returned as data so the UI can render
-- the requirement list without duplicating the cost table client side.
function M.upgrade_info(c, spell_id)
    local base  = ranks.base[spell_id] or spell_id
    local cur   = M.rank_of(c, base)
    local nxt   = cur + 1
    local cost  = M.COST[nxt]
    return {
        base    = base,
        rank    = cur,
        next    = cost and nxt or nil,
        frag    = cost and cost.frag or 0,
        ink     = cost and cost.ink  or 0,
        have_f  = c:CountItem(M.FRAGMENT_ITEM),
        have_i  = c:CountItem(M.INK_ITEM),
        rankable = (ranks.chain[base] ~= nil),
    }
end

function M.upgrade(c, spell_id)
    local i = M.upgrade_info(c, spell_id)
    if not i.rankable  then notify(c, "That spell cannot be ranked up.") return end
    -- ⚠️ Knowledge, not possession: you may rank a spell you are not currently carrying, because the
    -- rank attaches to the character. Requiring it to be scribed would make ranks hostage to the
    -- death wipe, which is exactly what this design removes.
    if not M.is_known(c, i.base) then notify(c, "You do not know that spell.") return end
    if not i.next then notify(c, "That spell is already at its highest rank.") return end
    if i.have_f < i.frag or i.have_i < i.ink then
        notify(c, string.format("Rank %d needs %d Parchment Fragments and %d Ink of the Lost. You have %d and %d.",
            i.next, i.frag, i.ink, i.have_f, i.have_i))
        return
    end

    -- ⚠️⚠️ ORDER IS LOAD-BEARING: take the materials, THEN record the rank. There is no refund path,
    -- but recording first and failing to charge would hand out free ranks. Same reasoning as the
    -- reroll gate order in spell_choice.
    c:RemoveItem(M.FRAGMENT_ITEM, i.frag)
    c:RemoveItem(M.INK_ITEM, i.ink)
    set_rank(c, i.base, i.next)

    -- If it happens to be scribed right now, swap the copy in hand to the new rank too.
    -- ⚠️ UnscribeSpell takes a BOOK SLOT, not a spell id (the binding is FindSpellBookSlotBySpellID;
    -- there is no GetSpellBookSlotBySpellID). Passing an id where a slot is expected would unscribe
    -- whatever occupies that numbered slot -- silently destroying an unrelated spell.
    local old = (i.rank > 1) and (ranks.chain[i.base] or {})[i.rank - 1] or i.base
    local slot = c:FindSpellBookSlotBySpellID(old)
    if slot >= 0 then
        c:UnscribeSpell(slot)
        c:ScribeSpell(M.ranked_id(c, i.base), slot)
    end

    notify(c, string.format("%s is now rank %d. Every future copy will be.",
        eq.get_spell_name(i.base) or "That spell", i.next))
end

-- ---------------------------------------------------------------- transport to the spell window
-- Wire format, matching the Pool tab's shape:
--   SPELLRANKCOST <rank>:<frag>:<ink>,...          the cost table, sent once
--   SPELLRANKDATA <chunk> <chunks> <frag> <ink>^<base>:<rank>:<kept>:<origin>,...
--
-- ⚠️⚠️ IDS ONLY -- the client resolves names from its own spell record. Sending names would multiply
-- the payload several times over for nothing (section 20).
-- ⚠️⚠️ CHUNKED, because a character accumulates discovered spells forever and an oversized chat line
-- is silently TRUNCATED -- which reads as a short library rather than as an error. Each line carries
-- its own chunk index so the client can tell a partial refresh from a complete one.
-- ⚠️ The cost table is SENT rather than hardcoded client side, so M.COST stays the single source of
-- truth. A duplicated table in the dll is the kIcons drift trap (section 3).
M.CHUNK = 50

function M.send_state(c)
    local costs = {}
    for rk = 2, 5 do
        local x = M.COST[rk]
        if x then costs[#costs + 1] = rk .. ":" .. x.frag .. ":" .. x.ink end
    end
    c:Message(MT.NPCQuestSay, "SPELLRANKCOST " .. table.concat(costs, ","))

    local kept = {}
    for _, id in ipairs(M.kept(c)) do kept[id] = true end

    local rows = {}
    for _, base in ipairs(M.known(c)) do
        rows[#rows + 1] = string.format("%d:%d:%d:%d",
            base, M.rank_of(c, base), kept[base] and 1 or 0, M.origin_of(c, base) or 0)
    end

    local frag, ink = c:CountItem(M.FRAGMENT_ITEM), c:CountItem(M.INK_ITEM)
    local chunks = math.max(1, math.ceil(#rows / M.CHUNK))
    for ci = 1, chunks do
        local part = {}
        for i = (ci - 1) * M.CHUNK + 1, math.min(ci * M.CHUNK, #rows) do part[#part + 1] = rows[i] end
        c:Message(MT.NPCQuestSay, string.format("SPELLRANKDATA %d %d %d %d^%s",
            ci, chunks, frag, ink, table.concat(part, ",")))
    end
end

-- ---------------------------------------------------------------- say routing
function M.handle_say(e)
    local c = e.self
    -- ⚠️ Every mutation re-sends the state. The window has no way to know a keep succeeded, was
    -- refused for the 2-cap, or that materials were just spent -- and a stale panel offering an
    -- upgrade the player can no longer afford reads as the button being broken.
    local id = e.message:match("^spellkeep (%d+)$")
    if id then M.keep(c, tonumber(id)); M.send_state(c) return true end
    id = e.message:match("^spellrelease (%d+)$")
    if id then M.release(c, tonumber(id)); M.send_state(c) return true end
    id = e.message:match("^spellrank (%d+)$")
    if id then M.upgrade(c, tonumber(id)); M.send_state(c) return true end
    if e.message == "spellrankreq" then M.send_state(c) return true end
    if e.message == "spellkept" then
        local kept = M.kept(c)
        c:Message(15, string.format("Fragments %d, Ink %d.", c:CountItem(M.FRAGMENT_ITEM), c:CountItem(M.INK_ITEM)))
        if #kept == 0 then c:Message(15, "You are keeping no spells.") end
        for _, base in ipairs(kept) do
            local lv = M.origin_of(c, base)
            c:Message(15, string.format("  %s (rank %d)%s", eq.get_spell_name(base) or base,
                M.rank_of(c, base), lv and (" -- costs your level " .. lv .. " pick") or ""))
        end
        return true
    end
    return false
end

return M
