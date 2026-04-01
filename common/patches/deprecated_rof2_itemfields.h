/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
/*


These fields must be in the order of how they are serialized!



*/
#define NEW_TRY
#ifdef NEW_TRY
//* 000 */	I(ItemClass) // Leave this one off on purpose
/* 001 */	S(Name)
/* 002 */	S(Lore)
//* 003 */	C("")//lorefile - Newly Added - Field is Null
/* 004 */	S(IDFile)
/* 005 */	I(ID)
/* 006 */	I(Weight)
/* 007 */	I(NoRent)
/* 008 */	I(NoDrop)
/* 009 */	I(Size)
/* 010 */	I(Slots)
/* 011 */	I(Price)
/* 012 */	I(Icon)
/* 013 */	C("0")//UNK013
/* 014 */	C("0")//UNK014
/* 015 */	I(BenefitFlag)
/* 016 */	I(Tradeskills)
/* 017 */	I(CR)
/* 018 */	I(DR)
/* 019 */	I(PR)
/* 020 */	I(MR)
/* 021 */	I(FR)
/* 022 */	C("0")//svcorruption - Newly Added
/* 023 */	I(AStr)
/* 024 */	I(ASta)
/* 025 */	I(AAgi)
/* 026 */	I(ADex)
/* 027 */	I(ACha)
/* 028 */	I(AInt)
/* 029 */	I(AWis)
/* 030 */	I(HP)
/* 031 */	I(Mana)
/* 032 */	I(Endur) //endur - Relocated
/* 033 */	I(AC)
/* 034 */	I(Classes)//classes - Relocated
/* 035 */	I(Races)//races - Relocated
/* 036 */	I(Deity)
/* 037 */	I(SkillModValue)
/* 038 */	C("0")//UNK038 - Default is 0
/* 039 */	I(SkillModType)
/* 040 */	I(BaneDmgRace)
/* 041 */	I(BaneDmgBody)//banedmgbody - Relocated
/* 042 */	I(BaneDmgRaceAmt)//banedmgraceamt - Relocated
/* 043 */	I(BaneDmgAmt)//banedmgamt - Relocated
/* 044 */	I(Magic)
/* 045 */	I(CastTime_)
/* 046 */	I(ReqLevel)
/* 047 */	I(RecLevel)//reclevel - Relocated
/* 048 */	I(RecSkill)//recskill - Relocated
/* 049 */	I(BardType)
/* 050 */	I(BardValue)
/* 051 */	I(Light)
/* 052 */	I(Delay)
/* 053 */	I(ElemDmgType)
/* 054 */	I(ElemDmgAmt)
/* 055 */	I(Range)
/* 056 */	I(Damage)
/* 057 */	I(Color)
/* 058 */	I(ItemType)
/* 059 */	I(Material)
/* 060 */	C("0")//UNK060 - Default is 0
/* 061 */	C("0")//UNK061 - Default is 0
/* 062 */	F(SellRate)
/* 063 */	I(CombatEffects)
/* 064 */	I(Shielding)
/* 065 */	I(StunResist)
/* 066 */	I(StrikeThrough)
/* 067 */	I(ExtraDmgSkill)
/* 068 */	I(ExtraDmgAmt)
/* 069 */	I(SpellShield)
/* 070 */	I(Avoidance)
/* 071 */	I(Accuracy)
/* 072 */	I(CharmFileID)
/* 073 */	I(FactionMod1)//Swapped these so Faction Amt comes after each Faction Mod
/* 074 */	I(FactionAmt1)//Swapped these so Faction Amt comes after each Faction Mod
/* 075 */	I(FactionMod2)//Swapped these so Faction Amt comes after each Faction Mod
/* 076 */	I(FactionAmt2)//Swapped these so Faction Amt comes after each Faction Mod
/* 077 */	I(FactionMod3)//Swapped these so Faction Amt comes after each Faction Mod
/* 078 */	I(FactionAmt3)//Swapped these so Faction Amt comes after each Faction Mod
/* 079 */	I(FactionMod4)//Swapped these so Faction Amt comes after each Faction Mod
/* 080 */	I(FactionAmt4)//Swapped these so Faction Amt comes after each Faction Mod
/* 081 */	S(CharmFile)
/* 082 */	I(AugType)
/* 083 */	I(AugRestrict)//augrestrict - Relocated
/* 084 */	I(AugDistiller)//augdistiller - Relocated
/* 085 */	I(AugSlotType[0])
/* 086 */	I(AugSlotVisible[0])//augslot1visible - Default 1
/* 087 */	C("0")//augslot1unk2 - Newly Added  - Default 0
/* 088 */	I(AugSlotType[1])
/* 089 */	I(AugSlotVisible[1])
/* 090 */	C("0")//augslot2unk2 - Newly Added
/* 091 */	I(AugSlotType[2])
/* 092 */	I(AugSlotVisible[2])
/* 093 */	C("0")//augslot3unk2 - Newly Added
/* 094 */	I(AugSlotType[3])
/* 095 */	I(AugSlotVisible[3])
/* 096 */	C("0")//augslot4unk2 - Newly Added
/* 097 */	I(AugSlotType[4])
/* 098 */	I(AugSlotVisible[4])
/* 099 */	C("0")//augslot5unk2 - Newly Added
/* 100 */	I(PointType)//pointtype - Relocated
/* 101 */	I(LDoNTheme)
/* 102 */	I(LDoNPrice)
/* 103 */	C("70")//UNK098 - Newly Added - Default 70, but some are set to 0
/* 104 */	I(LDoNSold)
/* 105 */	I(BagType)
/* 106 */	I(BagSlots)
/* 107 */	I(BagSize)
/* 108 */	I(BagWR)
/* 109 */	I(Book)
/* 110 */	I(BookType)
/* 111 */	S(Filename)
/* 112 */	I(LoreGroup)
/* 113 */	I(ArtifactFlag)
/* 114 */	C("0")//I(PendingLoreFlag)?//UNK109 - Default 0, but a few are 1
/* 115 */	I(Favor)
/* 116 */	I(GuildFavor)//guildfavor - Relocated
/* 117 */	I(FVNoDrop)
/* 118 */	I(DotShielding)
/* 119 */	I(Attack)
/* 120 */	I(Regen)
/* 121 */	I(ManaRegen)
/* 122 */	I(EnduranceRegen)
/* 123 */	I(Haste)
/* 124 */	I(DamageShield)
/* 125 */	C("-1") //UNK120 - Default is -1
/* 126 */	C("0") //UNK121 - Default is 0
/* 127 */	I(Attuneable)
/* 128 */	I(NoPet)
/* 129 */	C("0") //UNK124 - Default 0, but a few are 1
/* 130 */	I(PotionBelt)
/* 131 */	C("0") //potionbeltslots - Default 0, but a few are 1
/* 132 */	I(StackSize)
/* 133 */	I(NoTransfer)
/* 134 */	I(Stackable)//UNK129 - Default is 0, but some are much higher
/* 135 */	I(QuestItemFlag)//questitemflag - Default is 0 (off), flag on = 1
/* 136 */	C("0")//UNK131 - Default is 0, but there is an item set to 1
/* 137 */	C("0")//UNK132 - Default is 0? 0000000000000000000?
/* 138 */	I(Click.Effect)
/* 139 */	I(Click.Type)
/* 140 */	I(Click.Level2)
/* 141 */	I(Click.Level)
/* 142 */	I(MaxCharges)//maxcharges - Relocated
/* 143 */	I(CastTime_)//casttime - Relocated - Note Duplicate Entries for CastTime_ and none for CastTime
/* 144 */	I(RecastDelay)//recastdelay - Relocated
/* 145 */	I(RecastType)//recasttype - Relocated
/* 146 */	C("0")//clickunk5 - Newly Added - Default is 0
/* 147 */	C("")//clickname - Newly Added - Default is Null
/* 148 */	C("-1")//clickunk7 - Newly Added - Default is -1, but some set to 0 and some much higher
/* 149 */	I(Proc.Effect)
/* 150 */	I(Proc.Type)
/* 151 */	I(Proc.Level2)
/* 152 */	I(Proc.Level)
/* 153 */	C("0")//procunk1 - Newly Added - Default is 0, but some set to -1 and 1
/* 154 */	C("0")//procunk2 - Newly Added - Default is 0
/* 155 */	C("0")//procunk3 - Newly Added - Default is 0
/* 156 */	C("0")//procunk4 - Newly Added - Default is 0
/* 157 */	I(ProcRate)//procrate - Relocated
/* 158 */	C("")//procname - Newly Added - Default is Null
/* 159 */	C("-1")//procunk7 - Newly Added - Default is -1, but some set to 0
/* 160 */	I(Worn.Effect)
/* 161 */	I(Worn.Type)
/* 162 */	I(Worn.Level2)
/* 163 */	I(Worn.Level)
/* 164 */	C("0")//wornunk1 - Newly Added - Default is 0
/* 165 */	C("0")//wornunk2 - Newly Added - Default is 0
/* 166 */	C("0")//wornunk3 - Newly Added - Default is 0
/* 167 */	C("0")//wornunk4 - Newly Added - Default is 0
/* 168 */	C("0")//wornunk5 - Newly Added - Default is 0
/* 169 */	C("")//wornname - Newly Added - Default is Null
/* 170 */	C("-1")//wornunk7 - Newly Added - Default is -1, but some set to 0
/* 171 */	I(Focus.Effect)
/* 172 */	I(Focus.Type)
/* 173 */	I(Focus.Level2)
/* 174 */	I(Focus.Level)
/* 175 */	C("0")//focusunk1 - Newly Added - Default is 0
/* 176 */	C("0")//focusunk2 - Newly Added - Default is 0
/* 177 */	C("0")//focusunk3 - Newly Added - Default is 0
/* 178 */	C("0")//focusunk4 - Newly Added - Default is 0
/* 179 */	C("0")//focusunk5 - Newly Added - Default is 0
/* 180 */	C("")//focusname - Newly Added - Default is Null
/* 181 */	C("-1")//focusunk7 - Newly Added - Default is -1, but some set to 0
/* 182 */	I(Scroll.Effect)
/* 183 */	I(Scroll.Type)
/* 184 */	I(Scroll.Level2)
/* 185 */	I(Scroll.Level)
/* 186 */	C("0")//scrollunk1 - Renumber this***
/* 187 */	C("0")//scrollunk2 - Newly Added - Default is 0
/* 188 */	C("0")//scrollunk3 - Newly Added - Default is 0
/* 189 */	C("0")//scrollunk4 - Newly Added - Default is 0
/* 190 */	C("0")//scrollunk5 - Newly Added - Default is 0
/* 191 */	C("")//scrollname - Newly Added - Default is Null
/* 192 */	C("-1")//scrollunk7 - Newly Added - Default is -1, but some set to 0
/* 193 */	C("0")//UNK193 - Default is 0
/* 194 */	C("0")//purity - Newly Added - Default is 0, but some go up to 75
/* 195 */	C("0")//dsmitigation - Newly Added - Default is 0, but some are up to 2
/* 196 */	C("0")//heroic_str - Newly Added - Default is 0
/* 197 */	C("0")//heroic_int - Newly Added - Default is 0
/* 198 */	C("0")//heroic_wis - Newly Added - Default is 0
/* 199 */	C("0")//heroic_agi - Newly Added - Default is 0
/* 200 */	C("0")//heroic_dex - Newly Added - Default is 0
/* 201 */	C("0")//heroic_sta - Newly Added - Default is 0
/* 202 */	C("0")//heroic_cha - Newly Added - Default is 0
/* 203 */	C("0")//HeroicSvPoison - Newly Added - Default is 0
/* 204 */	C("0")//HeroicSvMagic - Newly Added - Default is 0
/* 205 */	C("0")//HeroicSvFire - Newly Added - Default is 0
/* 206 */	C("0")//HeroicSvDisease - Newly Added - Default is 0
/* 207 */	C("0")//HeroicSvCold - Newly Added - Default is 0
/* 208 */	C("0")//HeroicSvCorruption - Newly Added - Default is 0
/* 209 */	C("0")//healamt - Newly Added - Default is 0, but some are up to 9
/* 210 */	C("0")//spelldmg - Newly Added - Default is 0, but some are up to 9
/* 211 */	C("0")//clairvoyance - Newly Added - Default is 0, but some are up to 10
/* 212 */	C("0")//backstabdmg - Newly Added - Default is 0, but some are up to 65
//* 213 */	C("0")//evolvinglevel - Newly Added - Default is 0, but some are up to 7
//* 214 */	C("0")//MaxPower - Newly Added
//* 215 */	C("0")//Power - Newly Added

//This doesn't appear to be used /* 102 */	S(verified)//verified
//This doesn't appear to be used /* 102 */	S(serialized)//created
//Unsure where this goes right now (or if it is even used) /* 108 */	I(SummonedFlag)

#else
/* 000 */	//I(ItemClass) Leave this one off on purpose
/* 001 */	S(Name)
/* 002 */	S(Lore)
/* 003 */	C("")	//LoreFile?
/* 003 */	S(IDFile)
/* 004 */	I(ID)
/* 005 */	I(Weight)
/* 006 */	I(NoRent)
/* 007 */	I(NoDrop)
/* 008 */	I(Size)
/* 009 */	I(Slots)
/* 010 */	I(Price)
/* 011 */	I(Icon)
/* 013 */	C("0")
/* 014 */	C("0")
/* 014 */	I(BenefitFlag)
/* 015 */	I(Tradeskills)
/* 016 */	I(CR)
/* 017 */	I(DR)
/* 018 */	I(PR)
/* 019 */	I(MR)
/* 020 */	I(FR)
			C("0")	//svcorruption
/* 021 */	I(AStr)
/* 022 */	I(ASta)
/* 023 */	I(AAgi)
/* 024 */	I(ADex)
/* 025 */	I(ACha)
/* 026 */	I(AInt)
/* 027 */	I(AWis)
/* 028 */	I(HP)
/* 029 */	I(Mana)
			I(Endur)
/* 030 */	I(AC)
/* 052 */	I(Classes)
/* 053 */	I(Races)
/* 031 */	I(Deity)
/* 032 */	I(SkillModValue)
/* 033 */	C("0")
/* 034 */	I(SkillModType)
/* 035 */	I(BaneDmgRace)
/* 037 */	I(BaneDmgBody)
/* 036 */	I(BaneDmgRaceAmt)
/* 036 */	I(BaneDmgAmt)
/* 038 */	I(Magic)
/* 039 */	I(CastTime_)
/* 040 */	I(ReqLevel)
/* 045 */	I(RecLevel)
/* 046 */	I(RecSkill)
/* 041 */	I(BardType)
/* 042 */	I(BardValue)
/* 043 */	I(Light)
/* 044 */	I(Delay)
/* 047 */	I(ElemDmgType)
/* 048 */	I(ElemDmgAmt)
/* 049 */	I(Range)
/* 050 */	I(Damage)
/* 051 */	I(Color)
/* 056 */	I(ItemType)
/* 057 */	I(Material)
/* 060 */	C("0")
/* 061 */	C("0")
/* 058 */	F(SellRate)
/* 063 */	I(CombatEffects)
/* 064 */	I(Shielding)
/* 065 */	I(StunResist)
/* 059 */	//C("0")
/* 061 */	//C("0")
/* 066 */	I(StrikeThrough)
/* 067 */	I(ExtraDmgSkill)
/* 068 */	I(ExtraDmgAmt)
/* 069 */	I(SpellShield)
/* 070 */	I(Avoidance)
/* 071 */	I(Accuracy)
/* 072 */	I(CharmFileID)
/* 073 */	I(FactionMod1)
/* 077 */	I(FactionAmt1)
/* 074 */	I(FactionMod2)
/* 078 */	I(FactionAmt2)
/* 075 */	I(FactionMod3)
/* 079 */	I(FactionAmt3)
/* 076 */	I(FactionMod4)
/* 080 */	I(FactionAmt4)
/* 081 */	S(CharmFile)
/* 082 */	I(AugType)
/* 082 */	I(AugRestrict)
/* 082 */	I(AugDistiller)
/* 083 */	I(AugSlotType[0])
/* 084 */	I(AugSlotVisible[0])
/* 084 */	I(AugSlotUnk2[0])
/* 085 */	I(AugSlotType[1])
/* 086 */	I(AugSlotVisible[1])
/* 086 */	I(AugSlotUnk2[1])
/* 087 */	I(AugSlotType[2])
/* 088 */	I(AugSlotVisible[2])
/* 088 */	I(AugSlotUnk2[2])
/* 089 */	I(AugSlotType[3])
/* 090 */	I(AugSlotVisible[3])
/* 090 */	I(AugSlotUnk2[3])
/* 091 */	I(AugSlotType[4])
/* 092 */	I(AugSlotVisible[4])
/* 092 */	I(AugSlotUnk2[4])
/* 091 */	I(AugSlotType[5])
/* 092 */	I(AugSlotVisible[5])
/* 092 */	I(AugSlotUnk2[5])
/* 093 */	I(PointType)
/* 093 */	I(LDoNTheme)
/* 094 */	I(LDoNPrice)
/* 094 */	C("0")
/* 095 */	I(LDoNSold)
/* 096 */	I(BagType)
/* 097 */	I(BagSlots)
/* 098 */	I(BagSize)
/* 099 */	I(BagWR)
/* 100 */	I(Book)
/* 101 */	I(BookType)
/* 102 */	S(Filename)
/* 105 */	I(LoreGroup)
/* 106 */	//I(PendingLoreFlag)
/* 107 */	I(ArtifactFlag)
/* 094 */	C("0")
/* 108 */	//I(SummonedFlag)
/* 109 */	I(Favor)
/* 121 */	I(GuildFavor)
/* 110 */	I(FVNoDrop)
/* 112 */	I(DotShielding)
/* 113 */	I(Attack)
/* 114 */	I(Regen)
/* 115 */	I(ManaRegen)
/* 116 */	I(EnduranceRegen)
/* 117 */	I(Haste)
/* 118 */	I(DamageShield)
/* 120 */	C("0")
/* 121 */	C("0")
/* 125 */	I(Attuneable)
/* 126 */	I(NoPet)
/* 124 */	C("0")
/* 129 */	I(PotionBelt)
/* 130 */	I(PotionBeltSlots)
/* 131 */	I(StackSize)
/* 132 */	I(NoTransfer)
/* 129 */	C("0")
/* 132 */	I(QuestItemFlag)
/* 131 */	C("0")
/* 132 */	C("00000000000000000000000000000000000000")
/* 134 */	I(Click.Effect)
/* 135 */	I(Click.Type)
/* 136 */	I(Click.Level2)
/* 137 */	I(Click.Level)
/* 055 */	I(MaxCharges)
/* 060 */	I(CastTime)
/* 119 */	I(RecastDelay)
/* 120 */	I(RecastType)
/* 138 */	C("0")	//clickunk5	(prolly ProcRate)
/* 138 */	C("")	//clickunk6
/* 138 */	C("-1")	//clickunk7
/* 139 */	I(Proc.Effect)
/* 140 */	I(Proc.Type)
/* 141 */	I(Proc.Level2)
/* 142 */	I(Proc.Level)
/* 143 */	C("0")	//procunk1	(prolly MaxCharges)
/* 143 */	C("0")	//procunk2	(prolly CastTime)
/* 143 */	C("0")	//procunk3	(prolly RecastDelay)
/* 143 */	C("0")	//procunk4	(prolly RecastType)
/* 062 */	I(ProcRate)
/* 143 */	C("")	//procunk6
/* 143 */	C("-1")	//procunk7
/* 144 */	I(Worn.Effect)
/* 145 */	I(Worn.Type)
/* 146 */	I(Worn.Level2)
/* 147 */	I(Worn.Level)
/* 143 */	C("0")	//wornunk1	(prolly MaxCharges)
/* 143 */	C("0")	//wornunk2	(prolly CastTime)
/* 143 */	C("0")	//wornunk3	(prolly RecastDelay)
/* 143 */	C("0")	//wornunk4	(prolly RecastType)
/* 143 */	C("0")	//wornunk5	(prolly ProcRate)
/* 143 */	C("")	//wornunk6
/* 143 */	C("-1")	//wornunk7
/* 149 */	I(Focus.Effect)
/* 150 */	I(Focus.Type)
/* 151 */	I(Focus.Level2)
/* 152 */	I(Focus.Level)
/* 143 */	C("0")	//focusunk1	(prolly MaxCharges)
/* 143 */	C("0")	//focusunk2	(prolly CastTime)
/* 143 */	C("0")	//focusunk3	(prolly RecastDelay)
/* 143 */	C("0")	//focusunk4	(prolly RecastType)
/* 143 */	C("0")	//focusunk5	(prolly ProcRate)
/* 143 */	C("")	//focusunk6
/* 143 */	C("-1")	//focusunk7
/* 154 */	I(Scroll.Effect)
/* 155 */	I(Scroll.Type)
/* 156 */	I(Scroll.Level2)
/* 157 */	I(Scroll.Level)
/* 143 */	C("0")	//scrollunk1	(prolly MaxCharges)
/* 143 */	C("0")	//scrollunk2	(prolly CastTime)
/* 143 */	C("0")	//scrollunk3	(prolly RecastDelay)
/* 143 */	C("0")	//scrollunk4	(prolly RecastType)
/* 143 */	C("0")	//scrollunk5	(prolly ProcRate)
/* 143 */	C("")	//scrollunk6
/* 143 */	C("-1")	//scrollunk7
/* 193 */	C("0")	//Power Source Capacity
/* 194 */	C("0")	//purity

#endif

#undef I
#undef C
#undef S
#undef F
