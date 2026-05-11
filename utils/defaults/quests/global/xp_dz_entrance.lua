-- XP Dynamic Zone Entrance NPC
-- Place this as a global player script or attach to a specific NPC type ID
-- via a file named after the NPC type ID (e.g., 12345.lua in the zone directory)
--
-- This script provides SAY and ITEM hand-in triggers for creating XP DZ instances.
-- The NPC should be placed in a common hub zone (e.g. Plane of Knowledge).
--
-- Configuration: Insert rows into monomyth_xp_dz_zones for each zone you want
-- to enable for XP DZ. The script reads from that table at runtime.

local XPDZ_NPC_TYPE_ID = 999001 -- change this to match your NPC's type ID

-- Build the XP DZ zone menu from the database config table
-- eq.get_zone_list() is not available; instead, we hardcode a helper that
-- queries via MySQL through eq.query or we present a curated list.
-- For the MVP we use a simple curated table here.  Operators should edit
-- this list to match the rows they inserted into monomyth_xp_dz_zones.

local XP_DZ_ZONES = {
    { zone = "crushbone",     label = "Crushbone",         min_level = 5,  max_level = 20 },
    { zone = "unrest",        label = "Unrest",            min_level = 15, max_level = 35 },
    { zone = "gukbottom",     label = "Lower Guk",         min_level = 30, max_level = 50 },
    { zone = "soltemple",     label = "Solusek Temple",    min_level = 35, max_level = 55 },
    { zone = "permafrost",    label = "Permafrost",        min_level = 25, max_level = 45 },
}

function event_say(e)
    if not eq.get_rule("Monomyth.DynamicZonesEnabled") then
        e.self:Say("XP Dynamic Zones are not currently available.")
        return
    end

    if e.message:find("hail") then
        local has_dz = e.other:GetExpedition()
        if has_dz then
            e.self:Say(string.format(
                "You already have an active expedition: %s. You must leave it before creating an XP Dynamic Zone.",
                has_dz:GetName()
            ))
            return
        end

        e.self:Say("Greetings, adventurer! I can create an XP Dynamic Zone for you -- a private hunting ground with normal respawn but no raid targets. Which zone would you like?")
        for i, z in ipairs(XP_DZ_ZONES) do
            if e.other:GetLevel() >= z.min_level and e.other:GetLevel() <= z.max_level then
                e.self:Say(string.format("Tell me [%s] to create an XP Dynamic Zone for %s (levels %d-%d).",
                    z.zone, z.label, z.min_level, z.max_level))
            end
        end
        return
    end

    -- check if the spoken phrase matches a zone keyword
    local msg = e.message:lower()
    for _, z in ipairs(XP_DZ_ZONES) do
        if msg:find(z.zone) then
            if e.other:GetLevel() < z.min_level or e.other:GetLevel() > z.max_level then
                e.self:Say(string.format(
                    "You must be between level %d and %d to enter an XP Dynamic Zone for %s.",
                    z.min_level, z.max_level, z.label
                ))
                return
            end

            -- check for existing lockout
            if e.other:HasExpeditionLockout("XP DZ - " .. z.label, "XP DZ Lockout") then
                e.self:Say(string.format(
                    "You have a recent lockout for %s. Please wait before requesting another.",
                    z.label
                ))
                return
            end

            -- check for existing expedition/DZ
            if e.other:GetExpedition() then
                e.self:Say("You already have an active expedition. Leave it first.")
                return
            end

            local dz_name = "XP DZ - " .. z.label
            local exp = e.other:CreateXPDZ(z.zone, 0, dz_name, 1, 6)
            if exp then
                e.self:Say(string.format(
                    "Your XP Dynamic Zone for %s is ready! You have 24 hours. Good hunting!",
                    z.label
                ))
                exp:SetCompass(z.zone, 0, 0, 0)
                exp:SetSafeReturn(e.other:GetZoneID(), e.other:GetX(), e.other:GetY(), e.other:GetZ(), e.other:GetHeading())
                e.other:MovePCDynamicZone(z.zone)
            else
                e.self:Say("I was unable to create your XP Dynamic Zone. You may already have one active, or have a lockout.")
            end
            return
        end
    end
end

function event_trade(e)
    local item_lib = require("items")
    item_lib.return_items(e.self, e.other, e.trade)
end
