-- aotv4_dungeon_scale.lua -- scale delve creatures to the PLAYER, not just to the layer level.
--
-- Two scaling terms, and both are needed:
--   BASELINE   the layer's level. Picking the level 60 delve should mean level 60 content.
--   DYNAMIC    how far the player is above the NAKED expectation for their level. This is the gear
--              bloat correction, and it is what stops a fully geared character walking the level 50
--              layer as though it were empty.
--
-- WHY MEASURED THIS WAY: in EQ, stats / AC / resists do NOT grow with level on their own -- a naked
-- level 70 has the same STR as a naked level 1, because all of it comes from gear. Only HP, mana and
-- endurance scale with level. So gear bloat is directly visible as "actual over naked expectation",
-- and only the HP/mana axis needs a per-level curve at all.
--
-- THE NAKED BASELINE is the level 1 character in the reference screenshot (Warrior, Karana):
--   HP 38   MP 40   EN 21   AC 31   ATK 33
--   STR 75  STA 100  AGI 100  DEX 80  WIS 80  INT 75  CHA 75      (sum 585)
--   MAGIC 25  FIRE 25  COLD 25  DISEASE 25  POISON 15  CORRUPT 15  (sum 130)
-- ⚠️ Those stat and resist numbers are RACE and CLASS dependent, so using them as one universal
-- baseline is an approximation -- a Troll starts with more STA and a Gnome with more INT. It is close
-- enough for a difficulty knob, and it errs in the right direction: the axis that matters most (HP)
-- is taken from base_data, not from the screenshot.
--
-- ⚠️⚠️ REWARDS MUST NOT BE COMPUTED FROM THE PLAYER'S GEAR AT THE END OF THE RUN. Somebody who
-- clears the whole dungeon naked (weak mobs) and then puts their gear on before turning it in has
-- earned a weak reward, and somebody who fights the whole thing geared has earned a strong one. So
-- the ledger records THE EFFECTIVE LEVEL OF EVERY MOB ACTUALLY KILLED, at the moment it died. That
-- number cannot be retro-edited by changing clothes, which makes the whole class of gear-swap
-- exploits structurally impossible rather than merely detected. See M.record_kill / M.ledger.

local M = {}

-- ---------------------------------------------------------------- the level curve
-- base_data.hp for any class is exactly CLASS_HP[class] * SHAPE(level), and base_data.hp_fac is
-- CLASS_HP_FAC[class] * SHAPE(level) -- the shape is identical for all 16 classes and only the
-- per-class unit differs (warrior 25, magician 20, and warrior/magician hp is a constant 1.25 ratio
-- at every level). Read straight out of base_data, not guessed.
--
--   level  5 10 15 20 25 30 35 40 45 50 55 60 65 70
--   shape  5 10 15 20 25 30 35 40 50 60 70 80 90 100
--
-- ⚠️ It is LINEAR to 40 and then DOUBLE RATE. Treating it as linear all the way would understate the
-- expectation at level 70 by 30 percent (70 against 100) and make every high level character look
-- geared even when naked.
local function shape(level)
    if level < 1 then level = 1 end
    if level <= 40 then return level end
    return 40 + 2 * (level - 40)
end

-- base_data level-1 values, indexed by class id 1..16.
local CLASS_HP     = { 25, 22, 24, 23, 24, 20, 21.3, 22, 21.3, 21.3, 20, 20, 20, 20, 21.3, 21.3 }
local CLASS_HP_FAC = { 0.083, 0.073, 0.08, 0.077, 0.08, 0.067, 0.071, 0.073,
                       0.071, 0.071, 0.067, 0.067, 0.067, 0.067, 0.071, 0.071 }
local CLASS_MANA   = { 0, 15, 15, 15, 15, 15, 0, 15, 0, 15, 15, 15, 15, 15, 15, 0 }
local CLASS_MN_FAC = { 0, 0.075, 0.075, 0.075, 0.075, 0.075, 0, 0.075,
                       0, 0.075, 0.075, 0.075, 0.075, 0.075, 0.075, 0 }

-- The level 1 naked baseline for everything that does NOT scale with level.
local BASE_AC      = 31
local BASE_STATS   = 585    -- STR 75 + STA 100 + AGI 100 + DEX 80 + WIS 80 + INT 75 + CHA 75
local BASE_RESISTS = 130    -- MAGIC 25 + FIRE 25 + COLD 25 + DISEASE 25 + POISON 15 + CORRUPT 15

-- ---------------------------------------------------------------- tunables
M.WEIGHTS = { hp = 0.30, mana = 0.15, ac = 0.20, stats = 0.20, resists = 0.15 }

-- How many effective levels a doubling of power is worth, and how far it may move the layer level.
M.BUMP_PER_POWER = 12
M.MAX_BUMP       = 20   -- a fully decked character can push the level 50 layer to 70
M.MAX_DROP       = 5    -- ...and stripping down cannot drop it more than this below the layer

-- Power has to move by this much before living mobs are re-scaled. Without a deadband a single buff
-- landing or fading would re-scale the whole zone.
M.RESCALE_DELTA = 0.08

-- ---------------------------------------------------------------- power measurement
-- Returns power (1.0 == naked expectation for this level and class) plus the per-axis breakdown, so
-- `#delve power` can explain itself instead of printing one inscrutable number.
function M.power(c)
    if not c or not c.valid then return 1.0, nil end

    local level = c:GetLevel()
    local class = c:GetClass()
    if not class or class < 1 or class > 16 then class = 1 end

    local sh  = shape(level)
    local sta = c:GetSTA()
    local wis = c:GetWIS()
    local int = c:GetINT()

    -- ⚠️ The expectation uses the player's CURRENT stamina on purpose. HP granted BY stamina then
    -- lands in the stats axis rather than being counted twice -- once as raw HP and again as STA.
    local exp_hp = sh * (CLASS_HP[class] + CLASS_HP_FAC[class] * sta)

    -- Casters key mana off wis or int by class; use whichever is higher rather than a class table,
    -- which is close enough here and cannot be wrong for a hybrid.
    local casting = (wis > int) and wis or int
    local exp_mana = sh * (CLASS_MANA[class] + CLASS_MN_FAC[class] * casting)

    local stats = c:GetSTR() + c:GetSTA() + c:GetAGI() + c:GetDEX()
                + c:GetWIS() + c:GetINT() + c:GetCHA()
    local resists = c:GetMR() + c:GetCR() + c:GetDR() + c:GetFR() + c:GetPR() + c:GetCorruption()

    local function ratio(actual, expected)
        if not expected or expected <= 0 then return nil end   -- axis does not apply (mana on a pure melee)
        local r = actual / expected
        if r < 0.25 then r = 0.25 end                          -- floor: never let a 0 collapse the mean
        return r
    end

    local axes = {
        hp      = ratio(c:GetMaxHP(),   exp_hp),
        mana    = ratio(c:GetMaxMana(), exp_mana),
        ac      = ratio(c:GetAC(),      BASE_AC),
        stats   = ratio(stats,          BASE_STATS),
        resists = ratio(resists,        BASE_RESISTS),
    }

    -- Weighted mean over the axes that APPLY. ⚠️ Renormalise by the weight actually used: a Warrior
    -- has no mana, and dividing by the full weight total would score every melee class as 15 percent
    -- weaker than it is and quietly make their delves easier.
    local sum, used = 0.0, 0.0
    for axis, w in pairs(M.WEIGHTS) do
        local r = axes[axis]
        if r then sum = sum + w * r; used = used + w end
    end
    if used <= 0 then return 1.0, axes end

    return sum / used, axes
end

-- ---------------------------------------------------------------- effective level
-- The layer level is the floor of the experience; power moves it.
function M.effective_level(layer_level, c)
    local power = M.power(c)
    local delta = M.BUMP_PER_POWER * (power - 1.0)

    if delta >  M.MAX_BUMP then delta =  M.MAX_BUMP end
    if delta < -M.MAX_DROP then delta = -M.MAX_DROP end

    local eff = math.floor(layer_level + delta + 0.5)
    if eff < 1  then eff = 1  end
    if eff > 90 then eff = 90 end   -- npc_scale_global_base only covers 1..90
    return eff, power
end

-- ---------------------------------------------------------------- applying it to a mob
-- ScaleNPC does the coherent work (hp, damage, ac, resists, regen off npc_scale_global_base); the
-- fine pass afterwards expresses the FRACTION that an integer level cannot.
--
-- ⚠️ ScaleNPC must come FIRST. It rewrites the stats wholesale from the scale table, so any
-- ModifyNPCStat applied before it is simply discarded.
function M.apply(npc, eff_level, power)
    if not npc or not npc.valid then return end

    npc:ScaleNPC(eff_level)

    -- Fine trim: the part of the power ratio the rounded level did not capture. Kept small on purpose
    -- -- the level is doing the heavy lifting and this only smooths the steps between levels.
    if power and power > 1.0 then
        local fine = 1.0 + 0.10 * (power - 1.0)
        if fine > 1.35 then fine = 1.35 end
        local hp = npc:GetMaxHP()
        if hp and hp > 0 then
            npc:ModifyNPCStat("max_hp", tostring(math.floor(hp * fine)))
        end
    end

    -- Remember what this mob was worth, so the ledger can be honest about it when it dies.
    npc:SetEntityVariable("delve_eff", tostring(eff_level))
end

-- ---------------------------------------------------------------- re-scaling mid run
-- Catches the player who enters stripped (weak mobs) and then gears up. Called from a repeating
-- per-client timer, so it only ever walks the instance that player is standing in.
--
-- ⚠️⚠️ OUT OF COMBAT ONLY. ScaleNPC rewrites max_hp and the engine refills to the new maximum, so
-- re-scaling something you are fighting would heal it mid swing -- and re-scaling DOWN would be an
-- exploit in the other direction (strip, rescale the boss soft, gear back up). A mob already pulled
-- keeps the difficulty it was pulled at.
function M.rescale_zone(c, layer_level)
    if not c or not c.valid then return end

    local eff, power = M.effective_level(layer_level, c)

    -- Deadband, per character: only churn the zone when the player has MEANINGFULLY changed.
    -- ⚠️⚠️ IT IS A PER MOB TEST, NOT A WHOLE SWEEP EARLY RETURN, AND THAT DISTINCTION IS THE WHOLE
    -- POINT. It used to `return` here when the power had not moved, which meant the sweep could
    -- never repair a mob that had NEVER been scaled -- and on the second run of a session the stored
    -- power is unchanged by definition, so the sweep did nothing at all. Combined with mobs that
    -- spawn before the player arrives (below), a level 3 delve ran with 32 creatures at level 1 and
    -- NINE still at their native Dragons of Norrath level of 65-70, in the same room.
    -- ⚠️ An unstamped mob is ALWAYS scaled, whatever the deadband says. The deadband exists to stop
    -- churn, not to let a mob escape scaling entirely.
    local key    = "delve_pow_" .. c:CharacterID()
    local last   = tonumber(eq.get_data(key))
    local moved  = (not last) or math.abs(power - last) >= M.RESCALE_DELTA
    eq.set_data(key, tostring(power))

    local n = 0
    local list = eq.get_entity_list():GetNPCList()
    for npc in list.entries do
        if npc and npc.valid and not npc:IsEngaged() and not npc:IsPet() then
            -- never stamped => never scaled => still a native DoN mob, repair it regardless
            local stamped = tonumber(npc:GetEntityVariable("delve_eff"))
            if moved or not stamped then
                M.apply(npc, eff, power)
                n = n + 1
            end
        end
    end

    if n > 0 then
        c:Message(MT.Yellow, string.format(
            "The delve shifts to match you. (power %.2f, creatures now level %d, %d rescaled)",
            power, eff, n))
    end
end

-- ---------------------------------------------------------------- the difficulty ledger
-- What the run is actually WORTH. Recorded per kill from the mob's own scaled level, which is why no
-- amount of changing gear afterwards can alter it.
--   delve_kills_<charid>  kills counted
--   delve_effsum_<charid> sum of effective levels killed
--   delve_effmin_<charid> weakest thing killed
--   delve_effmax_<charid> strongest thing killed
local function lkey(c, what) return "delve_" .. what .. "_" .. c:CharacterID() end

function M.reset_ledger(c)
    if not c or not c.valid then return end
    for _, k in ipairs({ "kills", "effsum", "effmin", "effmax", "score", "named", "bands" }) do
        eq.delete_data(lkey(c, k))
    end
    eq.delete_data("delve_pow_" .. c:CharacterID())
end

-- ---------------------------------------------------------------- what one kill is WORTH
-- ⚠️ QUADRATIC, not linear. A level 70 creature is not 1.4x a level 50 one, it is several times the
-- fight, so a linear score would make grinding a soft layer for volume beat clearing a hard one --
-- exactly the behaviour the whole scaling system exists to discourage.
--   eff 1 -> 1    eff 10 -> 100    eff 25 -> 625    eff 50 -> 2500    eff 70 -> 4900
--
-- ⚠️⚠️ THE VALUE IS `eff * eff`, WITH NO DIVISOR AND NO MINIMUM CLAMP -- and restoring either one
-- breaks the ladder in a way that is invisible from a single run (fixed 2026-07-30). It used to be
--     local v = math.floor((eff * eff) / 25); if v < 1 then v = 1 end
-- and integer truncation meant every eff from 1 to 7 produced floor(1/25 .. 49/25) = 0, which the
-- clamp then lifted to 1. So THE FIRST SEVEN RUNGS ALL PAID EXACTLY THE SAME, the quadratic did not
-- engage until eff 8, and a rung 1 clear was worth as much as a rung 7 one. Farming the bottom was
-- the efficient play over precisely the stretch where a new character actually is.
-- Dividing by 25 bought nothing but smaller numbers, and it cost the whole low end to get them.
--
-- ⚠️ eff is already floored at 1 by M.effective_level, so no clamp is needed -- but the guard below
-- stays because record_kill can fall back to npc:GetLevel() for a mob that was never scaled (a
-- summoned add, a pet), and a squared NEGATIVE would come out POSITIVE and score well.
--
-- ⚠️⚠️ THIS FUNCTION AND THE SIGIL CURVE ARE ONE CALIBRATION, NOT TWO. The Delver's Sigil's
-- required_amount ladder (custom/sql/aotv4_delve_charm.sql) is derived FROM this formula --
-- 24000*n^2, which is ten runs per charm level given a ~41 kill run at rung 7n. Change the scoring
-- here and that SQL is wrong until it is recomputed; the derivation is written out in that file.
M.NAMED_MULT = 3   -- a named is a real fight; worth several trash mobs

-- "Named" by the same heuristic §17c and quest_difficulty.pl use: EQ trash is lowercase ("a bat",
-- "an orc pawn") and named mobs are proper nouns. A leading # is a spawn-name marker.
local function is_named(npc)
    local n = npc:GetCleanName()
    if not n or n == "" then return false end
    local first = n:sub(1, 1)
    return first == "#" or first:match("%u") ~= nil
end

function M.kill_value(eff, named)
    if not eff or eff < 1 then eff = 1 end
    local v = eff * eff
    if named then v = v * M.NAMED_MULT end
    return v
end

-- ⚠️ `layer_level` is the rung the run is on, passed in rather than looked up, and it is only used
-- as the fallback below. Callers must supply it; without it an unscaled kill falls back to 1.
function M.record_kill(c, npc, layer_level)
    if not c or not c.valid or not npc or not npc.valid then return end

    -- The mob's own stamp, set when it was scaled.
    -- ⚠️⚠️ THE FALLBACK IS THE LAYER LEVEL, NEVER `npc:GetLevel()`. These are Dragons of Norrath
    -- zones: an unscaled mob is sitting at its NATIVE level of 60-70, so falling back to its own
    -- level scores it as end-game content. On 2026-07-30 nine such kills in a LEVEL 3 delve were
    -- worth 4,900 each (14,700 for a named) and produced a run score of 59,622 -- enough to evolve
    -- the sigil outright, from a dungeon whose other 32 creatures were level 1.
    -- ⚠️ The stamp being missing means the scaling failed, and the score must not be the place that
    -- failure gets paid out. Scoring it as the rung is the conservative answer: a real mob was
    -- killed, so it is worth something, but never more than the dungeon is nominally worth.
    local eff = tonumber(npc:GetEntityVariable("delve_eff")) or layer_level or 1

    local kills = (tonumber(eq.get_data(lkey(c, "kills"))) or 0) + 1
    local sum   = (tonumber(eq.get_data(lkey(c, "effsum"))) or 0) + eff
    eq.set_data(lkey(c, "kills"),  tostring(kills))
    eq.set_data(lkey(c, "effsum"), tostring(sum))

    local lo = tonumber(eq.get_data(lkey(c, "effmin")))
    local hi = tonumber(eq.get_data(lkey(c, "effmax")))
    if not lo or eff < lo then eq.set_data(lkey(c, "effmin"), tostring(eff)) end
    if not hi or eff > hi then eq.set_data(lkey(c, "effmax"), tostring(eff)) end

    -- Running score.
    local named = is_named(npc)
    local score = (tonumber(eq.get_data(lkey(c, "score"))) or 0) + M.kill_value(eff, named)
    eq.set_data(lkey(c, "score"), tostring(score))
    if named then
        eq.set_data(lkey(c, "named"), tostring((tonumber(eq.get_data(lkey(c, "named"))) or 0) + 1))
    end

    -- Band histogram, in fives, so the run can be shown as a shape rather than one average.
    -- Stored as "45-12,50-20,55-15". ⚠️ Bands use '-' internally because '|' and '^' are the wire
    -- separators and ',' separates the bands themselves.
    local band = eff - (eff % 5)
    local raw  = eq.get_data(lkey(c, "bands")) or ""
    local out, found = {}, false
    for b, n in raw:gmatch("(%d+)%-(%d+)") do
        b, n = tonumber(b), tonumber(n)
        if b == band then n = n + 1; found = true end
        out[#out + 1] = b .. "-" .. n
    end
    if not found then out[#out + 1] = band .. "-1" end
    table.sort(out, function(x, y)
        return tonumber(x:match("^(%d+)")) < tonumber(y:match("^(%d+)"))
    end)
    eq.set_data(lkey(c, "bands"), table.concat(out, ","))
end

-- The run's difficulty, for rewards later. `avg` is the number that matters; `lo`/`hi` are the SWING,
-- which is the trend a gear-swapper leaves behind: clearing naked and gearing up at the end shows as
-- a low average with a high maximum, and it is the average that pays.
function M.ledger(c)
    if not c or not c.valid then return nil end
    local kills = tonumber(eq.get_data(lkey(c, "kills"))) or 0
    if kills <= 0 then
        return { kills = 0, avg = 0, lo = 0, hi = 0, score = 0, named = 0, bands = "" }
    end
    local sum = tonumber(eq.get_data(lkey(c, "effsum"))) or 0
    return {
        kills = kills,
        avg   = sum / kills,
        lo    = tonumber(eq.get_data(lkey(c, "effmin"))) or 0,
        hi    = tonumber(eq.get_data(lkey(c, "effmax"))) or 0,
        score = tonumber(eq.get_data(lkey(c, "score"))) or 0,
        named = tonumber(eq.get_data(lkey(c, "named"))) or 0,
        bands = eq.get_data(lkey(c, "bands")) or "",
    }
end

return M
