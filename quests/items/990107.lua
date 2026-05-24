local epic_rewards = {
    [1] = 60332, -- Warrior: Kreljnok's Sword of Eternal Power
    [2] = 20076, -- Cleric: Aegis of Superior Divinity
    [3] = 48147, -- Paladin: Nightbane, Sword of the Valiant
    [4] = 62649, -- Ranger: Aurora, the Heartwood Blade
    [5] = 48136, -- Shadowknight: Innoruuk's Dark Blessing
    [6] = 62880, -- Druid: Staff of Everliving Brambles
    [7] = 67742, -- Monk: Transcended Fistwraps of Immortality
    [8] = 77640, -- Bard: Blade of Vesagran
    [9] = 52348, -- Rogue: Nightshade, Blade of Entropy
    [10] = 57405, -- Shaman: Blessed Spiritstaff of the Heyokah
    [11] = 64067, -- Necromancer: Deathwhisper
    [12] = 16576, -- Wizard: Staff of Phenomenal Power
    [13] = 19839, -- Magician: Focus of Primal Elements
    [14] = 52962, -- Enchanter: Staff of Eternal Eloquence
    [15] = 52911, -- Beastlord: Savage Lord's Totem
    [16] = 18609  -- Berserker: Vengeful Taelosian Blood Axe
}

function event_click(e)
    local class_id = e.other:GetClass()
    local reward_item = epic_rewards[class_id]

    if reward_item then
        e.other:Message(15, "The coffer recognizes your path and reveals your Epic 2.0 weapon.")
        e.other:SummonItem(reward_item)
        e.other:DeleteItem(990107, 1)
    else
        e.other:Message(13, "The coffer does not respond to your class.")
    end
end
