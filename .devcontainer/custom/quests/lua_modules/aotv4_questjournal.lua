-- AoTv4 quest journal -- backs the Allaclone "Quests" and "Tracked" modes.
-- =================================================================================================
-- The client's NATIVE Quest Journal cannot hold these: MAXACTIVEQUESTS is a hard cap of 19 active
-- quests (29 on SoD+ clients) baked into both the client and the server's task arrays, and there are
-- 2,269 hand-in quests in the scripts. So this is our own journal, rendered in the Allaclone window
-- and fed from `quest_catalogue.lua`.
--
-- ⚠️⚠️ IT RIDES THE EXISTING SEARCH PROTOCOL AND ADDS NO NEW WIRE FORMAT. global_player's say
-- handler already routes "srch <kind> <term>" and "srchdet <kind> <id>"; the kinds are just strings,
-- so "quest" and "tracked" slot in beside item/npc/spell/recipe and the dll needs two more mode
-- buttons rather than a new window. Only tracking adds commands (qtrack / quntrack).
--
-- ⚠️⚠️ WHAT IT CAN AND CANNOT SEE. The catalogue is built by parsing `check_turn_in` (Lua) and
-- `plugin::check_handin` (Perl) out of the scripts, so the ONLY machine readable state a quest has
-- is WHICH ITEMS IT WANTS. A quest gated on saying a keyword, or on killing something first, shows
-- its hand-in requirements and nothing else, and simply completes when handed in. Multi NPC chains
-- appear as separate entries, one per NPC. The window says as much -- do not let it imply otherwise.
--
-- ⚠️ Quantities are NOT recoverable either: `check_turn_in{item1=X, item2=Y}` means one each of two
-- DIFFERENT items, so every requirement is counted as 1. A script wanting four of one item is rare
-- and would read as 1/1 here.
--
-- ⚠️⚠️ PROGRESS IS POLLED, NOT PUSHED, AND THAT IS DELIBERATE. There is no item-loss event in this
-- codebase (only scattered DeleteItemInInventory calls), so a push-only design would show stale
-- progress the moment a player sold or destroyed a required item. The dll re-runs "srch tracked"
-- every few seconds WHILE THE TRACKED MODE IS OPEN and not at all otherwise -- the same shape as the
-- autoskill window's resync. Closed window costs zero traffic. Section 15 records RoF2 dropping and
-- reordering chat bursts, so nothing here streams.
--
-- ⚠️ `client:CountItem` is the TIER AWARE quest binding (section 26), so a Hallowed or Mythic copy
-- of a required item counts -- exactly as it does at the real hand-in. Using a raw inventory walk
-- here would disagree with NPC::CheckHandin and tell the player they are short when they are not.

local cat = require("quest_catalogue")

local M = {}

-- ⚠️⚠️ EVERYTHING KEYS OFF q.id, THE STABLE QUEST NUMBER -- NEVER off the array position. The number
-- is shown to players so they can look a quest up by it, and the generator deliberately preserves it
-- across regenerations (see gen_quest_catalogue.pl). Indexing by position would reintroduce exactly
-- the drift that number exists to avoid: adding one script shifts every quest after it, and a number
-- somebody wrote down would silently point somewhere else.
-- ⚠️ Tracked quests are persisted by this id too, so a regen does not scramble a player's list.
local by_id = {}
for _, q in ipairs(cat.quests) do by_id[q.id] = q end

M.MAX_TRACKED  = 5     -- bounds the poll cost; the list is the thing that gets re-sent
M.MAX_RESULTS  = 40    -- ⚠️ chat lines are silently TRUNCATED when oversized (section 20)

-- ---------------------------------------------------------------- buckets
local function tkey(c) return "qtrack_" .. c:CharacterID() end

local function tracked_ids(c)
    local raw, out = eq.get_data(tkey(c)), {}
    if raw and raw ~= "" then
        for s in raw:gmatch("([^,]+)") do
            local n = tonumber(s)
            -- ⚠️ Drop ids the catalogue no longer has. A regen can renumber or remove entries and a
            -- stale id would otherwise index nil and break the whole list, not just its own row.
            if n and by_id[n] then out[#out + 1] = n end
        end
    end
    return out
end

local function save_tracked(c, ids)
    eq.set_data(tkey(c), table.concat(ids, ","))
end

-- ---------------------------------------------------------------- progress
-- Returns have, need -- counted in REQUIREMENTS MET, not in items. A requirement is met when you
-- carry at least the quantity it asks for, so "2/3" means two of three requirements satisfied.
-- ⚠️ Script quests always ask for 1 of each (the parser cannot recover a quantity); TASK entries
-- carry real goalcounts, which is why `qty` is consulted rather than assumed.
local function progress(c, q)
    local have, need = 0, 0
    for i, id in ipairs(q.items) do
        local want = (q.qty and q.qty[i]) or 1
        need = need + 1
        if c:CountItem(id) >= want then have = have + 1 end
    end
    return have, need
end

local function is_task(q) return q.src == "t" end

-- ⚠️⚠️ SQUARE BRACKETS ARE STRIPPED, NOT JUST THE WIRE SEPARATORS. The RoF2 client AUTO CONVERTS
-- any "[text]" in a chat message into an ITEM LINK, and when the text is not an item name the link
-- resolves to the masked id 0xFFFFF and renders as a wall of hex:
--     Linadian - freeporteast [|0FFFFF005EC0000...040|]
-- That is the same trap already recorded in zone/trading.cpp:1278 for this very window ("never put
-- square brackets in this text"), which is why the native item detail writes its flags comma
-- separated. A level marker written "[29]" and a checklist written "[x]" both trip it.
-- ⚠️ Section 26 also notes a broken link can DESYNC the client's parser for following lines, so one
-- stray bracket can corrupt the rest of the pane rather than just its own line.
-- Use parentheses, which are not special.
--
-- Also strips the three wire separators, so a stray one in an item or npc name cannot split a field.
local function clean(s)
    return (tostring(s or ""):gsub("[%^|~%[%]]", " "))
end

-- ---------------------------------------------------------------- item -> quests reverse index
-- Answers "what is this thing FOR?" from the item lookup, which is the question people actually
-- have when they find an unfamiliar drop. Same data as the catalogue, walked the other way.
--
-- ⚠️ Built LAZILY and cached for the life of the zone process. It is ~7,000 entries across 2,269
-- quests; doing it at require() time would pay that cost in every zone at boot whether or not
-- anybody ever searches for an item.
local item_index = nil
local function build_item_index()
    if item_index then return item_index end
    item_index = {}
    -- ⚠️ Stores q.id, NOT the loop position. Everything downstream resolves through by_id, so a
    -- position here would look up an unrelated quest (or nil) for every single item.
    for _, q in ipairs(cat.quests) do
        for _, id in ipairs(q.items) do
            local t = item_index[id]
            if not t then t = {}; item_index[id] = t end
            t[#t + 1] = q.id
        end
    end
    return item_index
end

M.MAX_USES = 8   -- ⚠️ common junk (bone chips, spider silk) is wanted by HUNDREDS of scripts

-- Returns extra STML-safe detail lines for an item, or "" when it is not a turn-in anywhere.
-- ⚠️ Returns lines WITHOUT a leading separator; the caller joins.
function M.item_usage(item_id)
    local idx = build_item_index()[item_id]
    if not idx then return "" end

    local L = { "", "Turn this in to:" }
    local shown = 0
    for _, i in ipairs(idx) do
        if shown >= M.MAX_USES then break end
        local q = by_id[i]
        shown = shown + 1
        -- ⚠️ The number is the POINT of this list: an item tells you which quests want it, and the
        -- number is how you then find one on the Quests tab. Without it the player has a name and a
        -- zone and no way to get to the entry.
        L[#L + 1] = "  #" .. q.id .. "  " .. clean(q.disp) .. " - " .. clean(q.zone) ..
                    ((q.lvl > 0) and ("  lvl " .. q.lvl) or "")
    end
    if #idx > shown then
        -- ⚠️ Say what was withheld. A silently truncated list reads as "these are the only ones",
        -- which for a common component is badly wrong.
        L[#L + 1] = string.format("  ... and %d more.", #idx - shown)
    end
    return table.concat(L, "~")
end

-- ---------------------------------------------------------------- search  ("srch quest <term>")
-- Term conventions:
--   "here"        quests whose npc lives in the zone you are standing in
--   anything else substring match on npc name or zone short name
-- `_c` is unused: the zone comes from eq.get_zone_id() rather than the client. The parameter stays
-- so all three entry points (search / tracked_list / detail) take the client in the same position,
-- and so a future per-character filter needs no signature change at the call sites.
function M.search(_c, term)
    term = (term or ""):lower()
    local here = (term == "here" or term == "*")
    -- ⚠️ `eq.get_zone_id()` TAKES NO ARGUMENTS and returns the CURRENT zone -- there is no
    -- Lua_Client:GetZoneID on this build, and passing a name is a hard error with a stack traceback
    -- (see the delve module, which was bitten by exactly this). Use eq.get_zone_id_by_name() when a
    -- named lookup is genuinely wanted.
    local zid  = eq.get_zone_id()

    -- Look up by NUMBER. The quest number is printed everywhere a quest is named -- including in an
    -- item's "Turn this in to" list -- so typing one has to find that quest. Checked before the text
    -- match, since a bare number can never be a meaningful name substring.
    local as_num = tonumber(term)
    if as_num and by_id[as_num] then
        local q = by_id[as_num]
        local lvl = (q.lvl > 0) and ("  lvl " .. q.lvl) or ""
        return q.id .. "|#" .. q.id .. "  " .. clean(q.disp) .. lvl .. " - " .. clean(q.zone)
    end

    local out, n = {}, 0
    for _, q in ipairs(cat.quests) do
        local hit
        if here then
            hit = (q.zid == zid)
        else
            hit = q.disp:lower():find(term, 1, true) or q.zone:lower():find(term, 1, true)
        end
        if hit then
            n = n + 1
            if n > M.MAX_RESULTS then break end
            local lvl = (q.lvl > 0) and ("  lvl " .. q.lvl) or ""
            -- ⚠️ The FIRST field is the id the dll sends back on Track and on opening the detail; the
            -- "#id" inside the label is what the player reads. They are the same number on purpose.
            out[#out + 1] = q.id .. "|#" .. q.id .. "  " .. clean(q.disp) .. lvl .. " - " .. clean(q.zone)
        end
    end
    if #out == 0 then return "" end
    return table.concat(out, "^")
end

-- ---------------------------------------------------------------- tracked list ("srch tracked ...")
-- The progress is rendered INTO THE ROW LABEL rather than sent as a separate field, so the existing
-- two field "id|name" result format carries it and the dll needs no parser change.
function M.tracked_list(c)
    local ids = tracked_ids(c)
    if #ids == 0 then return "" end
    local out = {}
    for _, i in ipairs(ids) do
        local q = by_id[i]
        local have, need = progress(c, q)
        local mark = (need > 0 and have >= need) and "READY" or (have .. "/" .. need)
        out[#out + 1] = i .. "|(" .. mark .. ") #" .. q.id .. "  " .. clean(q.disp) .. " - " .. clean(q.zone)
    end
    return table.concat(out, "^")
end

-- ---------------------------------------------------------------- detail  ("srchdet quest <id>")
-- Lines are separated by "~", which is the search window's existing detail format.
function M.detail(c, i)
    local q = by_id[i]
    if not q then return "That quest is no longer in the catalogue." end

    local have, need = progress(c, q)
    local task = is_task(q)
    local L = {}
    L[#L + 1] = "#" .. q.id .. "  " .. clean(q.disp) .. (task and "   (task)" or "")
    L[#L + 1] = "Zone: " .. clean(q.zone) ..
                ((q.lvl > 0) and ((task and "   Min level: " or "   NPC level: ") .. q.lvl) or "")
    L[#L + 1] = ""
    L[#L + 1] = task and "Requires:" or "Hand in to this NPC:"
    -- ⚠️ `slot`, not `i`: `i` is this function's parameter, the QUEST index, and it is read again
    -- below to test whether the quest is tracked. A loop variable named `i` shadows it and is one
    -- rename away from silently testing an item position against the tracked list.
    for slot, id in ipairs(q.items) do
        local want = (q.qty and q.qty[slot]) or 1
        local cnt  = c:CountItem(id)
        -- ⚠️ Resolved at DISPLAY time out of shared memory, not from a baked table. A generated copy
        -- of all 5,774 names cost 192 KB of the catalogue and could drift from the database; this
        -- runs a handful of times per detail view and cannot go stale.
        local nm   = eq.get_item_name(id)
        if not nm or nm == "" then nm = "item " .. id end
        -- Show the quantity only when more than one is wanted, so script quests stay uncluttered.
        -- ⚠️ (+) and ( ), never [x] and [ ] -- see the bracket warning on clean() above.
        local line = (cnt >= want and "  (+) " or "  ( ) ") ..
                     ((want > 1) and (want .. "x ") or "") .. clean(nm)
        if cnt > 0 then line = line .. "  (carrying " .. cnt .. ")" end
        L[#L + 1] = line
    end
    L[#L + 1] = ""
    L[#L + 1] = "Progress: " .. have .. " of " .. need .. " requirements met."
    if task then
        -- ⚠️ Be explicit that this one is not ours to complete. A task is assigned, advanced and
        -- finished by the ENGINE's task system and shown in the client's own Quest Journal; all we
        -- can honestly report is what it asks for and what you are carrying. Implying otherwise
        -- would have players waiting on a completion this window can never deliver.
        L[#L + 1] = "This is a task. Pick it up from its NPC; the game's own Quest Journal tracks it."
    elseif q.steps > 1 then
        L[#L + 1] = "This NPC handles " .. q.steps .. " separate hand ins; they may belong to different quests."
    end
    -- ⚠️ Say plainly what is NOT known. The catalogue only sees hand ins, so a quest with a spoken
    -- trigger or a kill step looks complete here when it is not, and a player who is not told that
    -- reads it as the window being wrong rather than as the window being limited.
    L[#L + 1] = ""
    L[#L + 1] = "Only hand in items are tracked. Steps that need a keyword or a kill are not visible."

    -- ⚠️⚠️ NEVER PRINT THE CATALOGUE INDEX AT A PLAYER. It is an array position in a generated file,
    -- it changes whenever the catalogue is regenerated, and nobody can be expected to know or type
    -- one. The window already holds the id for the selected row and sends it on the button press, so
    -- the instruction is the BUTTON, not a command. (The raw "qtrack <n>" say still exists for the
    -- no-dll fallback, where the unswallowed SRCHDATA line shows the ids anyway.)
    local ids, on = tracked_ids(c), false
    for _, t in ipairs(ids) do if t == i then on = true end end
    L[#L + 1] = on and "Tracked. Press Untrack, on the Tracked tab, to stop following it."
                   or  "Press Track, below, to follow this one."
    return table.concat(L, "~")
end

-- ---------------------------------------------------------------- track / untrack
-- ⚠️⚠️ FEEDBACK GOES TO THE WINDOW, NOT TO CHAT. Tracking is a UI action taken inside a window that
-- is already on screen, so answering it in the chat log is both noise and the wrong place to look --
-- and it is the player's OWN chat, which they may be using. `SRCHMSG` is swallowed by the dll and
-- written into the window's hint line, beside the result count.
-- ⚠️ The dll must swallow the "/say qtrack" ECHO too (AllacloneIsOurEcho) or the command itself is
-- still visible even with the reply silenced. Both halves are needed; either alone leaves spam.
local function notify(c, text)
    c:Message(MT.NPCQuestSay, "SRCHMSG " .. clean(text))
end

function M.track(c, i)
    local q = by_id[i]
    if not q then notify(c, "No such quest.") return end
    local ids = tracked_ids(c)
    for _, t in ipairs(ids) do
        if t == i then notify(c, "Already tracking that one.") return end
    end
    -- ⚠️ The cap is what bounds the poll: every tracked quest is re-counted on every resync while
    -- the window is open. Refuse rather than silently dropping the oldest, so nothing a player
    -- deliberately tracked disappears without being told.
    if #ids >= M.MAX_TRACKED then
        notify(c, string.format("Already tracking %d quests. Untrack one first.", M.MAX_TRACKED))
        return
    end
    ids[#ids + 1] = i
    save_tracked(c, ids)
    notify(c, "Tracking: " .. q.disp)
end

function M.untrack(c, i)
    local ids, out, gone = tracked_ids(c), {}, false
    for _, t in ipairs(ids) do
        if t == i then gone = true else out[#out + 1] = t end
    end
    save_tracked(c, out)
    notify(c, gone and "No longer tracking that quest." or "You were not tracking that one.")
end

-- ---------------------------------------------------------------- say routing
-- Returns true when it consumed the message, so global_player can stop.
function M.handle_say(e)
    local c = e.self
    local id = e.message:match("^qtrack (%d+)$")
    if id then M.track(c, tonumber(id)) return true end
    id = e.message:match("^quntrack (%d+)$")
    if id then M.untrack(c, tonumber(id)) return true end
    return false
end

-- Called from global_player's existing srch/srchdet routing. Returns a string for our kinds, or
-- nil to let the native C++ SearchList/SearchDetail handle item/npc/spell/recipe unchanged.
function M.search_kind(c, kind, term)
    if kind == "quest"   then return M.search(c, term)   end
    if kind == "tracked" then return M.tracked_list(c)   end
    return nil
end

function M.detail_kind(c, kind, id)
    if kind == "quest" or kind == "tracked" then return M.detail(c, id) end
    return nil
end

return M
