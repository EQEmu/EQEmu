-- Wayfinder Alessa (npc 2000400), Resplendent -- opens regions.
-- =================================================================================================
-- Every character gets ONE region to choose at the door. After that a region costs an "Ending"
-- achievement (a death at the level cap), so the second is earned rather than picked.
--
-- ⚠️⚠️ WITHOUT HER A NEW CHARACTER IS STUCK IN THE UNMAPPED WORLD. RegionManager::CanEnterZone
-- allows any zone NOT in `zone_regions` and blocks a mapped zone whose region you lack, so a
-- character holding no region cannot reach Crushbone, Blackburrow, the Commonlands or the Karanas --
-- the entire classic levelling spine. She is the way in, which is why the first credit is free.
--
-- ⚠️ The credits and the unlocking itself live in lua_modules/aotv4_regions.lua; this file is only
-- the conversation. Keeping the rules out of the NPC means the same credit can be spent from
-- anywhere later (a window, a command) without duplicating the gate order.

local regions = require("aotv4_regions")

-- ⚠️ Granted HERE, on first hail, rather than at character creation. Creation has no convenient
-- Lua hook, and doing it on hail means the credit cannot be spent before she has explained what it
-- is for. The bucket makes it once-ever, not once-per-run: the roguelite wipes levels and gear, not
-- data buckets, so a death must not hand out a fresh free region.
local function grant_starting_credit(c)
    local key = "region_start_" .. c:CharacterID()
    if (eq.get_data(key) or "") ~= "" then return false end
    eq.set_data(key, "1")
    eq.set_data("region_credits_" .. c:CharacterID(),
        tostring((tonumber(eq.get_data("region_credits_" .. c:CharacterID())) or 0) + 1))
    return true
end

local function offer(e)
    local c = e.self
    local n = regions.credits(c)

    if n < 1 then
        e.self:Say("You have walked all the ways open to you. Fall at the height of your power, and I will show you another.")
        return
    end

    e.self:Say(string.format("You may open %d more %s. Choose:", n, n == 1 and "way" or "ways"))
    for id, name in ipairs(regions.REGIONS) do
        if not c:HasRegion(id) then
            -- ⚠️ A SAYLINK carries the exact command, so the player never types a region number. The
            -- link text is the region NAME; the payload is what aotv4_regions.handle_say parses.
            e.self:Say(string.format("  %s", eq.say_link("openregion " .. id, true, name)))
        end
    end
end

function event_say(e)
    local c = e.self

    if e.message:findi("hail") then
        e.self:Say("Well met. I am Alessa, and I keep the ways out of this place.")
        e.self:Say("Norrath is wide, but it does not open all at once. Six ways lead from here -- " ..
                   "to [Kelethin] among the trees, to [Freeport] and its roads, to [Thurgadin] under the ice, " ..
                   "to [Firiona Vie] across the sea, to [Qeynos] and its hills, and to [Cabilis] in the sand.")
        if grant_starting_credit(c) then
            e.self:Say("You may walk ONE of them now. The rest are earned -- each time you fall at the " ..
                       "height of your power, another way opens to you.")
        end
        offer(e)
        return
    end

    -- Talking about a region by name, for players who would rather read than click.
    for id, name in ipairs(regions.REGIONS) do
        if e.message:findi(name) then
            if c:HasRegion(id) then
                e.self:Say(name .. " is already open to you. Walk it well.")
            elseif regions.credits(c) < 1 then
                e.self:Say("I cannot open " .. name .. " for you yet. Fall at the height of your power first.")
            else
                e.self:Say("Then " .. name .. " it is. " ..
                    eq.say_link("openregion " .. id, true, "Open the way to " .. name))
            end
            return
        end
    end

    if e.message:findi("region") or e.message:findi("way") then
        offer(e)
    end
end
