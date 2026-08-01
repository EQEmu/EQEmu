-- Herald Coren (npc 2000402), Resplendent -- explains the server in a popup on hail.
-- =================================================================================================
-- ⚠️ The popup is the point: this is the one place a new player is told how AoTv4 differs from EQ,
-- and chat scrolls away while a popup does not. Keywords below repeat each section for anyone who
-- dismissed it.
--
-- ⚠️⚠️ eq.popup TEXT IS STML, NOT PLAIN TEXT. Line breaks are <br>, colour is <c "#RRGGBB">...</c>,
-- and a literal '<' in the body would be read as markup and swallow everything after it. There is no
-- error for malformed STML -- the panel simply renders wrong or empty.
-- ⚠️ A '%' in the body is eaten as a printf token by Client::Message further down the chain
-- (CLAUDE.md section 5), so percentages are spelled out.

local G = "#F0F000"   -- gold, for the headings the stock level-up popups use
local B = "#66CCFF"   -- blue, for names of things

local function line(s) return s .. "<br>" end

local function overview(e)
    local body =
        line('<c "' .. G .. '">This is not ordinary Norrath.</c>') ..
        "<br>" ..
        line('<c "' .. G .. '">Death is the engine.</c>') ..
        line("When you die you return to level 1. Your gear, your coin and your spellbook are lost.") ..
        line("What you keep is what death cannot take: your experience becomes <c \"" .. B .. "\">Alternate Advancement</c>,") ..
        line("and everything you have permanently earned stays with you.") ..
        "<br>" ..
        line('<c "' .. G .. '">The cap is level 30.</c>') ..
        line("A run is short by design. Reaching the cap and falling there is the goal, not a failure.") ..
        "<br>" ..
        line('<c "' .. G .. '">You do not choose your spells. You choose from three.</c>') ..
        line("Every level offers three random rewards -- spells, or a combat ability. Pick one.") ..
        line("Two spells may be marked to <c \"" .. B .. "\">Keep</c>, and they return to you at the start of each run,") ..
        line("at the cost of that level's reward next time.") ..
        "<br>" ..
        line('<c "' .. G .. '">Spells can be ranked up, and ranks are forever.</c>') ..
        line("Fallen spellbooks leave <c \"" .. B .. "\">Parchment Fragments</c>; the world rarely yields <c \"" .. B .. "\">Ink of the Lost</c>.") ..
        line("Together they raise a spell one rank -- stronger, and cheaper to cast. Every future copy") ..
        line("of that spell comes to you at the rank you earned.") ..
        "<br>" ..
        line('<c "' .. G .. '">The world opens a piece at a time.</c>') ..
        line("Six regions. <c \"" .. B .. "\">Wayfinder Alessa</c> opens your first, free. Each time you fall at the") ..
        line("height of your power, another opens -- of your choosing.") ..
        "<br>" ..
        line('<c "' .. G .. '">Nothing is fixed until you live in it.</c>') ..
        line("<c \"" .. B .. "\">Reforger Vael</c> will remake your form or your calling, while you are still level 1.") ..
        "<br>" ..
        line("Ask me about the [<c \"" .. B .. "\">death</c>], the [<c \"" .. B .. "\">rewards</c>], the [<c \"" .. B .. "\">ranks</c>], the [<c \"" .. B .. "\">regions</c>] or the [<c \"" .. B .. "\">delve</c>].")

    eq.popup("The Resplendent Temple", body)
end

function event_say(e)
    if e.message:findi("hail") then
        e.self:Say("Well met. Read what I have set before you -- this place does not work as you expect.")
        overview(e)
        return
    end

    if e.message:findi("death") then
        eq.popup("Death",
            line('<c "' .. G .. '">Death</c>') ..
            line("You return to level 1. Carried gear, coin and your spellbook are destroyed.") ..
            line("Your bank is safe. Your class aura is safe. Your spell ranks are safe.") ..
            "<br>" ..
            line("The run's experience becomes Alternate Advancement -- roughly two and a half points") ..
            line("for a full climb to the cap. Nothing earned is wasted: what does not reach a whole") ..
            line("point is carried to your next death.") ..
            "<br>" ..
            line("Each spell destroyed leaves one <c \"" .. B .. "\">Parchment Fragment</c>."))
        return
    end

    if e.message:findi("reward") or e.message:findi("spell") then
        eq.popup("Rewards",
            line('<c "' .. G .. '">Every level, three choices</c>') ..
            line("Spells, or occasionally a combat ability. You take one and the others are gone.") ..
            line("If you dislike all three you may pay coin to roll again -- the price rises each time.") ..
            "<br>" ..
            line("Open the spell window to see what you have, what the pool still holds, and to mark") ..
            line("up to two spells to <c \"" .. B .. "\">Keep</c> through death. A kept spell costs you the reward at the") ..
            line("level you first took it."))
        return
    end

    if e.message:findi("rank") then
        eq.popup("Spell Ranks",
            line('<c "' .. G .. '">Ranks are permanent</c>') ..
            line("A spell can be raised through five ranks. Each rank is stronger and costs less mana.") ..
            "<br>" ..
            line("It costs <c \"" .. B .. "\">Parchment Fragments</c>, left behind by your own destroyed spellbooks,") ..
            line("and <c \"" .. B .. "\">Ink of the Lost</c>, which anything in the world may rarely drop.") ..
            "<br>" ..
            line("Once earned, a rank belongs to you -- not to the copy in your book. Every future") ..
            line("copy of that spell arrives already raised."))
        return
    end

    if e.message:findi("region") then
        eq.popup("The Regions",
            line('<c "' .. G .. '">Six regions</c>') ..
            line("Kelethin. Freeport. Thurgadin. Firiona Vie. Qeynos. Cabilis.") ..
            "<br>" ..
            line("Most of Norrath is closed. You may walk only the regions you have opened, and this") ..
            line("temple and its trials, which are always open to you.") ..
            "<br>" ..
            line("<c \"" .. B .. "\">Wayfinder Alessa</c> opens your first at no cost. Every time you fall at the level") ..
            line("cap, another may be opened -- and you choose which."))
        return
    end

    if e.message:findi("delve") then
        eq.popup("The Delve",
            line('<c "' .. G .. '">The Delve</c>') ..
            line("A trial of your own, scaled to you. Choose a depth and step in alone.") ..
            "<br>" ..
            line("Everything within is measured against what you carry, so arriving poorly equipped") ..
            line("makes it no easier. Clear it and a chest is left where you finished."))
        return
    end
end
