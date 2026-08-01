-- Reforger Vael (npc 2000401), Resplendent -- race and class change at level 1.
-- =================================================================================================
-- ⚠️ The rules live in lua_modules/aotv4_reforge.lua; this file is only the conversation. Keeping
-- the legality check and the relog out of the NPC means the same reforge can be driven from anywhere
-- later without duplicating either.
--
-- ⚠️⚠️ ONLY 112 OF 256 CLASS/RACE PAIRS ARE LEGAL, so the menus below only ever offer what the
-- CURRENT half allows: picking a race lists the classes that race may be, and vice versa. Offering
-- all sixteen and refusing afterwards would be the same information delivered as a failure.

local reforge = require("aotv4_reforge")

local function gate(e)
    local ok, why = reforge.can_reforge(e.self)
    if not ok then e.self:Say(why) end
    return ok
end

local function offer_races(e)
    local c = e.self
    local class = c:GetClass()
    e.self:Say("These forms can hold a " .. (reforge.CLASS_NAME[class] or "?") .. ":")
    -- ⚠️ RACE_ORDER, not ipairs -- the race ids are sparse (128, 130, 330, 522) and ipairs would stop
    -- at 13, silently hiding Iksar, Vah Shir, Froglok and Drakkin.
    for _, r in ipairs(reforge.RACE_ORDER) do
        if reforge.legal(r, class) and c:GetRace() ~= r then
            -- ⚠️ SILENT saylink. Non-silent makes the player SAY the command, which shows in chat and
            -- is heard by everyone in the starting hub.
            e.self:Say("  " .. eq.say_link("reforgerace " .. r, true, reforge.RACE_NAME[r]))
        end
    end
end

local function offer_classes(e)
    local c = e.self
    local race = c:GetRace()
    e.self:Say("A " .. (reforge.RACE_NAME[race] or "?") .. " may follow these callings:")
    for cl = 1, 16 do
        if reforge.legal(race, cl) and c:GetClass() ~= cl then
            e.self:Say("  " .. eq.say_link("reforgeclass " .. cl, true, reforge.CLASS_NAME[cl]))
        end
    end
end

function event_say(e)
    local c = e.self

    if e.message:findi("hail") then
        e.self:Say("I am Vael. I unmake and remake -- but only what has not yet been lived in.")
        e.self:Say(string.format("You are a %s %s.",
            reforge.RACE_NAME[c:GetRace()] or "?", reforge.CLASS_NAME[c:GetClass()] or "?"))
        if (c:GetLevel() or 0) > 1 then
            e.self:Say("You have already walked too far this run. Fall, and come to me at the start of the next.")
            return
        end
        e.self:Say("I can change your [form] or your [calling]. Choose while you are still unwritten.")
        return
    end

    if e.message:findi("form") or e.message:findi("race") then
        if gate(e) then offer_races(e) end
        return
    end

    if e.message:findi("calling") or e.message:findi("class") then
        if gate(e) then offer_classes(e) end
        return
    end
end
