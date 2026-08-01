-- AoTv4 region unlocks -- spending the credits the "Ending" achievements award.
-- =================================================================================================
-- Dying AT THE LEVEL CAP completes the next achievement in the chain (9100001-9100005), each of
-- which awards ONE region credit. A credit is spent on whichever region the player chooses, so the
-- order of the world opening up is theirs, not ours.
--
-- ⚠️⚠️ THE CREDIT IS NOT A REGION. The achievement reward type `region_choice` only increments
-- `region_credits_<charid>`; nothing is unlocked until the player picks. That indirection is the
-- feature -- naming a region in the reward row would fix the order and make the five achievements
-- interchangeable with a single counter.
--
-- ⚠️⚠️ REGION LOCKING IS INVERTED FROM WHAT IT LOOKS LIKE. RegionManager::CanEnterZone allows any
-- zone NOT in `zone_regions` and blocks a mapped zone whose region you lack. So the 51 organised
-- zones are the gated ones and the other ~250 are wide open. Until a character holds a region they
-- cannot reach Crushbone or Blackburrow but can walk into most of the rest of the world.
--
-- ⚠️ `Client:UnlockRegion` / `HasRegion` / `GetRegionMaxLevel` are C++ bindings (zone/regions.cpp).
-- The region ceiling also feeds era_system.level_cap, so unlocking a region can raise the level cap
-- if a region with a higher max_level is ever added -- today every region is 35.

local M = {}

-- ⚠️⚠️ THE SIX CHOOSABLE REGIONS -- ids and names must match the `regions` table exactly, because
-- Lua has no way to query it. A region present in the DB but missing here simply cannot be chosen
-- (fails safe), but the reverse is worse: a name here that does not exist in the DB would let a
-- player spend a credit on nothing.
--
-- ⚠️⚠️ 0 AND 99 ARE DELIBERATELY ABSENT AND MUST STAY ABSENT.
--   0  "Always Available" -- the pass-through. RegionManager::CanEnterZone returns true the moment
--      GetZoneRegion yields 0, so those 7 zones (Resplendent and the six DoN zones the Delve
--      instances) are open to everyone. Nobody needs to hold it, and unlocking it would waste a
--      credit on a region that grants nothing.
--   99 "Unused" -- 414 zones fenced off from the whole server. Nobody is meant to hold it, and
--      granting it would open every zone the balance pass deliberately closed.
-- Both are real rows in `regions`, so a player naming one is an ordinary integer away; M.open()
-- rejects anything not in this table, which is what actually enforces it.
M.REGIONS = {
    [1] = "Kelethin",
    [2] = "Freeport",
    [3] = "Thurgadin",
    [4] = "Firiona Vie",
    [5] = "Qeynos",
    [6] = "Cabilis",
}

local function ckey(c) return "region_credits_" .. c:CharacterID() end

function M.credits(c) return tonumber(eq.get_data(ckey(c))) or 0 end

-- ---------------------------------------------------------------- the capped-death trigger
-- ⚠️⚠️ "AT MAX" IS ERA_SYSTEM'S ANSWER, NOT A CONSTANT. The cap is the lowest of the era cap, the
-- hard cap and this character's region ceiling, and the region ceiling moves as regions are
-- unlocked. Hardcoding 35 here would give two definitions of the cap that drift apart the moment
-- either moves -- and this is the one that decides whether a death counts at all.
function M.on_death(e)
    local c = e.self
    local era = require("era_system")
    local cap = era.level_cap(c)
    if (c:GetLevel() or 0) < cap then return end

    -- ⚠️ Called BEFORE the roguelite reset sets the character to level 1. From event_death, which
    -- runs first -- reading the level after the reset would never see the cap.
    c:CreditDeathAtMaxLevel()
end

-- ---------------------------------------------------------------- spending a credit
function M.list(c)
    local n = M.credits(c)
    c:Message(15, string.format("Region unlocks available: %d", n))
    for id, name in ipairs(M.REGIONS) do
        local have = c:HasRegion(id)
        c:Message(have and 15 or 13, string.format("  %d. %s%s", id, name, have and "  (open)" or ""))
    end
    if n > 0 then
        c:Message(15, "Say 'openregion <number>' to spend one.")
    end
end

function M.open(c, region_id)
    local name = M.REGIONS[region_id]
    if not name then c:Message(13, "No such region.") return end
    if c:HasRegion(region_id) then c:Message(15, name .. " is already open to you.") return end

    local n = M.credits(c)
    if n < 1 then
        c:Message(13, "You have no region unlocks. Fall at the height of your power to earn one.")
        return
    end

    -- ⚠️⚠️ GATE ORDER IS LOAD-BEARING: validate, check the credit, UNLOCK, then spend. UnlockRegion
    -- can fail, and there is no refund path -- spending first would bill for a region the player did
    -- not get. Same reasoning as the reroll gate order in spell_choice.
    if not c:UnlockRegion(region_id) then
        c:Message(13, "That region could not be opened.")
        return
    end
    eq.set_data(ckey(c), tostring(n - 1))

    c:Message(15, string.format("%s is open to you. (%d unlock%s remaining)",
        name, n - 1, (n - 1) == 1 and "" or "s"))
    eq.world_emote(15, string.format("%s has opened the way to %s.", c:GetCleanName(), name))
end

-- ---------------------------------------------------------------- say routing
function M.handle_say(e)
    local c = e.self
    if e.message == "regions" then M.list(c) return true end
    local id = e.message:match("^openregion (%d+)$")
    if id then M.open(c, tonumber(id)) return true end
    return false
end

return M
