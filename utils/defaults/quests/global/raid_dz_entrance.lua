-- Raid Dynamic Zone Entrance NPC
-- Place this as a global player script or attach to a specific NPC type ID
-- via a file named after the NPC type ID (e.g., 999002.lua in the zone directory)
--
-- This script provides SAY triggers for creating Raid DZ instances.
-- The NPC should be placed in a common hub zone (e.g. Plane of Knowledge).
--
-- Configuration: Insert rows into monomyth_raid_dz_zones for each zone you
-- want to enable for Raid DZ. Insert rows into monomyth_raid_dz_targets for
-- NPC type IDs that must be guaranteed present in the Raid DZ instance.

local RAID_DZ_NPC_TYPE_ID = 999002

local RAID_DZ_ZONES = {
    { zone = "crushbone",     label = "Crushbone",         min_level = 5,  max_level = 20 },
    { zone = "unrest",        label = "Unrest",            min_level = 15, max_level = 35 },
    { zone = "gukbottom",     label = "Lower Guk",         min_level = 30, max_level = 50 },
    { zone = "soltemple",     label = "Solusek Temple",    min_level = 35, max_level = 55 },
    { zone = "permafrost",    label = "Permafrost",        min_level = 25, max_level = 45 },
}

function event_say(e)
    if not eq.get_rule("Monomyth.DynamicZonesEnabled") then
        e.self:Say("Raid Dynamic Zones are not currently available.")
        return
    end

    if e.message:find("hail") then
        local has_raid_dz = false
        local exp = e.other:GetExpedition()
        if exp then
            e.self:Say(string.format(
                "You already have an active expedition: %s. You must leave it before creating a Raid Dynamic Zone.",
                exp:GetName()
            ))
            return
        end

        e.self:Say("Greetings, adventurer! I can create a Raid Dynamic Zone for you -- a private instance where all mobs are spawned once with no respawns, and raid targets are guaranteed. Which zone would you like?")
        for i, z in ipairs(RAID_DZ_ZONES) do
            if e.other:GetLevel() >= z.min_level and e.other:GetLevel() <= z.max_level then
                e.self:Say(string.format("Tell me [raid %s] to create a Raid Dynamic Zone for %s (levels %d-%d).",
                    z.zone, z.label, z.min_level, z.max_level))
            end
        end
        return
    end

    local msg = e.message:lower()
    for _, z in ipairs(RAID_DZ_ZONES) do
        if msg:find("raid " .. z.zone) then
            if e.other:GetLevel() < z.min_level or e.other:GetLevel() > z.max_level then
                e.self:Say(string.format(
                    "You must be between level %d and %d to enter a Raid Dynamic Zone for %s.",
                    z.min_level, z.max_level, z.label
                ))
                return
            end

            if e.other:HasExpeditionLockout("Raid DZ - " .. z.label, "Raid DZ Lockout") then
                e.self:Say(string.format(
                    "You have a recent lockout for %s. Please wait before requesting another Raid Dynamic Zone.",
                    z.label
                ))
                return
            end

            if e.other:GetExpedition() then
                e.self:Say("You already have an active expedition. Leave it first.")
                return
            end

            local dz_name = "Raid DZ - " .. z.label
            local exp = e.other:CreateRaidDZ(z.zone, 0, dz_name, 1, 54)
            if exp then
                e.self:Say(string.format(
                    "Your Raid Dynamic Zone for %s is ready! You have 24 hours. Killed mobs will not respawn. Good hunting!",
                    z.label
                ))
                exp:SetCompass(z.zone, 0, 0, 0)
                exp:SetSafeReturn(e.other:GetZoneID(), e.other:GetX(), e.other:GetY(), e.other:GetZ(), e.other:GetHeading())
                e.other:MovePCDynamicZone(z.zone)
            else
                e.self:Say("I was unable to create your Raid Dynamic Zone. You may already have one active, or have a lockout.")
            end
            return
        end
    end
end

function event_trade(e)
    local item_lib = require("items")
    item_lib.return_items(e.self, e.other, e.trade)
end
