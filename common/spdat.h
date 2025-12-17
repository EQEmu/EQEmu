/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#pragma once

#include "common/classes.h"
#include "common/item_data.h"
#include "common/skills.h"

#define SPELL_UNKNOWN 0xFFFF
#define POISON_PROC 0xFFFE
#define SPELLBOOK_UNKNOWN 0xFFFFFFFF		//player profile spells are 32 bit

//some spell IDs which will prolly change, but are needed
#define SPELL_COMPLETE_HEAL 13
#define SPELL_LIFEBURN 2755
#define SPELL_LEECH_TOUCH 2766
#define SPELL_LAY_ON_HANDS 87
#define SPELL_HARM_TOUCH 88
#define SPELL_HARM_TOUCH2 2821
#define SPELL_IMP_HARM_TOUCH 2774
#define SPELL_NPC_HARM_TOUCH 929
#define SPELL_AVATAR_ST_PROC 2434
#define SPELL_CAZIC_TOUCH 982
#define SPELL_TOUCH_OF_VINITRAS 2859
#define SPELL_DESPERATE_HOPE 841
#define SPELL_CHARM 300
#define SPELL_METAMORPHOSIS65 2314
#define SPELL_JT_BUFF 3716
#define SPELL_CAN_O_WHOOP_ASS 911
#define SPELL_PHOENIX_CHARM 3014
#define SPELL_AVATAR_KNOCKBACK 905
#define SPELL_SHAPECHANGE65 2079
#define SPELL_SUNSET_HOME1218 1218
#define SPELL_SUNSET_HOME819 819
#define SPELL_SHAPECHANGE75 780
#define SPELL_SHAPECHANGE80 781
#define SPELL_SHAPECHANGE85 782
#define SPELL_SHAPECHANGE90 783
#define SPELL_SHAPECHANGE95 784
#define SPELL_SHAPECHANGE100 785
#define SPELL_SHAPECHANGE25 1200
#define SPELL_SHAPECHANGE30 1201
#define SPELL_SHAPECHANGE35 1202
#define SPELL_SHAPECHANGE40 1203
#define SPELL_SHAPECHANGE45 1204
#define SPELL_SHAPECHANGE50 1205
#define SPELL_NPC_AEGOLISM 1343
#define SPELL_SHAPECHANGE55 1923
#define SPELL_SHAPECHANGE60 1924
#define SPELL_COMMAND_OF_DRUZZIL 3355
#define SPELL_SHAPECHANGE70 6503
#define SPELL_MANA_BURN 2751
#define SPELL_LIFE_BURN 2755
#define SPELL_TOUCH_OF_THE_DIVINE 4789
// these have known hardcoded behavior but we don't do anything yet, move them above this comment when fixed
#define SPELL_THE_DAINS_JUSTICE 1476
#define SPELL_MODULATION 1502
#define SPELL_TORPOR 1576
#define SPELL_SPLURT 1620
#define SPELL_SEBILITE_POX 1814
#define SPELL_SOUL_WELL 1816
#define SPELL_MYSTICAL_TRANSVERGENCE 2716
#define SPELL_ACT_OF_VALOR 2775
#define SPELL_STOICISM 3694
#define SPELL_ALTER_PLANE_HATE 666
#define SPELL_ALTER_PLANE_SKY 674
#define SPELL_DENONS_DESPERATE_DIRGE 742
#define SPELL_BOND_OF_SATHIR 833
#define SPELL_DISEASED_CLOUD 836
#define SPELL_ACTING_RESIST 775
#define SPELL_ACTING_SHIELD 776
#define SPELL_ACTING_GUARD 777
#define SPELL_GUIDE_ACTING 778
#define SPELL_BYE_BYE 779
#define SPELL_ACTING_RESIST_II 1206
#define SPELL_ACTING_SHIELD_II 1207
#define SPELL_ACTING_GUARD_II 1208
#define SPELL_GUIDE_ACTING2 1209
#define SPELL_BYE_BYTE2 1210
#define SPELL_GUIDE_CANCEL_MAGIC 1211
#define SPELL_GUIDE_JOURNEY 1212
#define SPELL_GUIDE_VISION 1213
#define SPELL_GUIDE_HEALTH 1214
#define SPELL_GUIDE_INVULNERABILITY 1215
#define SPELL_GUIDE_BOLT 1216
#define SPELL_GUIDE_MEMORY_BLUE 1217
#define SPELL_GUIDE_ALLIANCE 1219
#define SPELL_SPECIAL_SIGHT 1220
#define SPELL_TERROR_OF_DARKNESS 1221
#define SPELL_TERROR_OF_SHADOWS 1222
#define SPELL_TERROR_OF_DEATH 1223
#define SPELL_TERROR_OF_TERRIS 1224
#define SPELL_VOICE_OF_DARKNESS 1225
#define SPELL_VOICE_OF_SHADOWS 1226
#define SPELL_VOICE_OF_DEATH 1227
#define SPELL_VOICE_OF_TERRIS 1228
#define SPELL_VENGEANCE_V 1229
#define SPELL_VENGEANCE_VII 1230
#define SPELL_VENGEANCE_VIII 1231
#define SPELL_VENGEANCE_IX 1232
#define SPELL_CORRUPTED_LACERATION 1233
#define SPELL_VISIONS_OF_CHAOS 1234
#define SPELL_VISIONS_OF_PAIN 1235
#define SPELL_COMMANDING_PRESENCE 1236
#define SPELL_MALICIOUS_INTENT 1237
#define SPELL_CURSE_OF_FLAMES 1238
#define SPELL_DEVOURING_CONFLAGRATION 1239
#define SPELL_AVATAR_SHIELD 1240
#define SPELL_AVATAR_SIGHT 1241
#define SPELL_AVATAR_GUARD 1242
#define SPELL_AVATAR_RESIST 1243
#define SPELL_MAGI_BOLT 1244
#define SPELL_MAGI_STRIKE 1245
#define SPELL_MAGI_CURSE 1246
#define SPELL_MAGI_CIRCLE 1247
#define SPELL_SPIRITUAL_ECHO 1248
#define SPELL_BRISTLING_ARMAMENT 1249
#define SPELL_WATON_DESTRUCTION 1250
#define SPELL_TRANSLOCATE_GROUP 1334
#define SPELL_TRANSLOCATE 1422
#define SPELL_ACTING_MAGIC_RESIST_I 1900
#define SPELL_ACTING_FIRE_RESIST_I 1901
#define SPELL_ACTING_COLD_RESIST_I 1902
#define SPELL_ACTING_POISON_RESIST_I 1903
#define SPELL_ACTING_DISEASE_RESIST_I 1904
#define SPELL_ACTING_MAGIC_RESIST_II 1905
#define SPELL_ACTING_FIRE_RESIST_II 1906
#define SPELL_ACTING_COLD_RESIST_II 1907
#define SPELL_ACTING_POISON_RESIST_II 1908
#define SPELL_ACTING_DISEASE_RESIST_II 1909
#define SPELL_ACTING_FIRE_SHIELD 1910
#define SPELL_ACTING_POISON_SHIELD 1911
#define SPELL_ACTING_COLD_SHIELD 1912
#define SPELL_ACTING_DISEASE_SHIELD 1913
#define SPELL_ACTING_ARMOR_I 1914
#define SPELL_ACTING_ARMOR_II 1915
#define SPELL_ACTING_ARMOR_III 1916
#define SPELL_ACTING_HEALTH_I 1917
#define SPELL_ACTING_HEALTH_II 1918
#define SPELL_ACTING_HEALTH_III 1919
#define SPELL_ACTING_HEALTH_IV 1920
#define SPELL_ACTING_SPIRIT_I 1921
#define SPELL_ACTING_SPIRIT_II 1922
#define SPELL_RESURRECTION_SICKNESS 756
#define SPELL_RESURRECTION_SICKNESS4 757
#define SPELL_TELEPORT 3243
#define SPELL_RESURRECTION_SICKNESS2 5249
#define SPELL_REVIVAL_SICKNESS 13087
#define SPELL_RESURRECTION_SICKNESS3 37624
#define SPELL_PACT_OF_HATE_RECOURSE 40375
#define SPELL_INCENDIARY_OOZE_BUFF 32513
#define SPELL_EYE_OF_ZOMM 323
#define SPELL_MINOR_ILLUSION 287
#define SPELL_ILLUSION_TREE 601
#define SPELL_ILLUSION_FEMALE 1731
#define SPELL_ILLUSION_MALE 1732
#define SPELL_UNSUMMON_SELF 892
#define SPELL_ANCIENT_LIFEBANE 2115
#define SPELL_GMHP25K 6817
#define SPELL_GMHP50K 6818
#define SPELL_GMHP100K 6819
#define SPELL_GMHP225K 6820
#define SPELL_GMHP475K 6821
#define SPELL_GMHP925K 6822
#define SPELL_GMHP2M 6823
#define SPELL_GMHP3M 6824
#define SPELL_GMHP5M 39851
#define SPELL_GUIDE_ACTING_ONE 778
#define SPELL_GUIDE_ALLIANCE_ONE 810
#define SPELL_GUIDE_CANCEL_MAGIC_ONE 811
#define SPELL_GUIDE_JOURNEY_ONE 813
#define SPELL_GUIDE_VISION_ONE 814
#define SPELL_GUIDE_HEALTH_ONE 815
#define SPELL_GUIDE_INVULNERABILITY_ONE 816
#define SPELL_GUIDE_BOLT_ONE 817
#define SPELL_GUIDE_MEMORY_BLUR_ONE 818
#define SPELL_GUIDE_ACTING_TWO 1209
#define SPELL_GUIDE_CANCEL_MAGIC_TWO 1211
#define SPELL_GUIDE_JOURNEY_TWO 1212
#define SPELL_GUIDE_VISION_TWO 1213
#define SPELL_GUIDE_HEALTH_TWO 1214
#define SPELL_GUIDE_INVULNERABILITY_TWO 1215
#define SPELL_GUIDE_BOLT_TWO 1216
#define SPELL_GUIDE_MEMORY_BLUR_TWO 1217
#define SPELL_GUIDE_ALLIANCE_TWO 1219
#define SPELL_GUIDE_EVACUATION 3921
#define SPELL_GUIDE_LEVITATION 39852
#define SPELL_GUIDE_SPELL_HASTE 39853
#define SPELL_GUIDE_HASTE 39854
#define SPELL_VAMPIRIC_EMBRACE 821
#define SPELL_VAMPIRIC_EMBRACE_OF_SHADOW 822
#define SPELL_BATTLE_CRY 5027
#define SPELL_WAR_CRY 5028
#define SPELL_BATTLE_CRY_OF_DRAVEL 5029
#define SPELL_WAR_CRY_OF_DRAVEL 5030
#define SPELL_BATTLE_CRY_OF_THE_MASTRUQ 5031
#define SPELL_ANCIENT_CRY_OF_CHAOS 5032
#define SPELL_BLOODTHIRST 8476
#define SPELL_AMPLIFICATION 2603
#define SPELL_DIVINE_REZ 2738
#define SPELL_NATURES_RECOVERY 2520
#define SPELL_MINOR_HEALING 200
#define SPELL_ADRENALINE_SWELL 14445
#define SPELL_ADRENALINE_SWELL_RK2 14446
#define SPELL_ADRENALINE_SWELL_RK3 14447

// discipline IDs.
#define DISC_UNHOLY_AURA 4520

//spellgroup ids
#define SPELLGROUP_FRENZIED_BURNOUT 2754
#define SPELLGROUP_ILLUSION_OF_GRANDEUR 38603
#define SPELLGROUP_ROGUES_FURY 16861
#define SPELLGROUP_HARMONIOUS_PRECISION 15634
#define SPELLGROUP_HARMONIOUS_EXPANSE 15633
#define SPELLGROUP_FURIOUS_RAMPAGE 38106
#define SPELLGROUP_SHROUD_OF_PRAYER 41050

#define EFFECT_COUNT 12
#define MAX_SPELL_TRIGGER 12	// One for each slot(only 6 for AA since AA use 2)
#define MAX_RESISTABLE_EFFECTS 12	// Number of effects that are typcially checked agianst resists.
#define MaxLimitInclude 18 //Number(x 0.5) of focus Limiters that have inclusive checks used when calcing focus effects
#define MAX_SKILL_PROCS 4 //Number of spells to check skill procs from. (This is arbitrary) [Single spell can have multiple proc checks]
#define MAX_AA_PROCS 16 //(Actual Proc Amount is MAX_AA_PROCS/4) Number of spells to check AA procs from. (This is arbitrary)
#define MAX_SYMPATHETIC_PROCS 10 // Number of sympathetic procs a client can have (This is arbitrary)
#define MAX_FOCUS_PROC_LIMIT_TIMERS 20 //Number of focus recast timers that can be going at same time (This is arbitrary)
#define MAX_PROC_LIMIT_TIMERS 8 //Number of proc delay timers that can be going at same time, different proc types get their own timer array. (This is arbitrary)
#define MAX_APPEARANCE_EFFECTS 20 //Up to 20 Appearance Effects can be saved to a mobs appearance effect array, these will be sent to other clients when they enter a zone (This is arbitrary)
#define MAX_CAST_ON_SKILL_USE 36 //Actual amount is MAX/3

#define MAX_INVISIBILTY_LEVEL 254

//instrument item id's used as song components
#define INSTRUMENT_HAND_DRUM 13000
#define INSTRUMENT_WOODEN_FLUTE 13001
#define INSTRUMENT_LUTE 13011
#define INSTRUMENT_HORN 13012

//option types for the rule Spells:ResurrectionEffectBlock
#define RES_EFFECTS_CANNOT_STACK -1
#define NO_RES_EFFECTS_BLOCK 0
#define RES_EFFECTS_BLOCK 1
#define RES_EFFECTS_BLOCK_WITH_BUFFS 2
#define MOVE_NEW_SLOT 2

#define PARTIAL_DEATH_SAVE 1
#define FULL_DEATH_SAVE 2

#define MAX_FAST_HEAL_CASTING_TIME 2000
#define MAX_VERY_FAST_HEAL_CASTING_TIME 1000

#define DETRIMENTAL_EFFECT 0
#define BENEFICIAL_EFFECT 1
#define BENEFICIAL_EFFECT_GROUP_ONLY 2

const uint32 MobAISpellRange=100; // max range of buffs

enum FocusLimitIncludes {
	IncludeExistsSELimitResist        = 0,
	IncludeFoundSELimitResist         = 1,
	IncludeExistsSELimitSpell         = 2,
	IncludeFoundSELimitSpell          = 3,
	IncludeExistsSELimitEffect        = 4,
	IncludeFoundSELimitEffect         = 5,
	IncludeExistsSELimitTarget        = 6,
	IncludeFoundSELimitTarget         = 7,
	IncludeExistsSELimitSpellGroup    = 8,
	IncludeFoundSELimitSpellGroup     = 9,
	IncludeExistsSELimitCastingSkill  = 10,
	IncludeFoundSELimitCastingSkill   = 11,
	IncludeExistsSELimitSpellClass    = 12,
	IncludeFoundSELimitSpellClass     = 13,
	IncludeExistsSELimitSpellSubclass = 14,
	IncludeFoundSELimitSpellSubclass  = 15,
	IncludeExistsSEFFItemClass        = 16,
	IncludeFoundSEFFItemClass         = 17
};
/*
	The id's correspond to 'type' 39 in live(2021) dbstr_us gives the message for target and caster restricted effects. These are not present in the ROF2 dbstr_us.
	If from CasterRestriction spell field. "Your target does not meet the spell requirements. <insert below corresponding string>." Msg in combat window, color red.
	If set as limit in a direct damage or heal spell (SPA 0) do not give message.
*/
enum SpellRestriction
{
	UNKNOWN_3                                                                 = 3,   // | caster restriction | seen in spell 30183 Mind Spiral
	IS_NOT_ON_HORSE                                                           = 5,   // | caster restriction |
	IS_ANIMAL_OR_HUMANOID                                                     = 100, // This spell will only work on animals or humanoid creatures.
	IS_DRAGON                                                                 = 101, // This spell will only work on dragons.
	IS_ANIMAL_OR_INSECT                                                       = 102, // This spell will only work on animals or insects.
	IS_BODY_TYPE_MISC                                                         = 103, // This spell will only work on humanoids, lycanthropes, giants, Kael Drakkel giants, Coldain, animals, insects, constructs, dragons, Skyshrine dragons, Muramites, or creatures constructed from magic.
	IS_BODY_TYPE_MISC2                                                        = 104, // This spell will only work on humanoids, lycanthropes, giants, Kael Drakkel giants, Coldain, animals, or insects.
	IS_PLANT                                                                  = 105, // This spell will only work on plants.
	IS_GIANT                                                                  = 106, // This spell will only work on animals. | Live used to have this on spells restricted to Giants, but those spells were removed... We still have them
	IS_NOT_ANIMAL_OR_HUMANOID                                                 = 108, // This spell will only work on targets that are neither animals or humanoid.
	IS_BIXIE                                                                  = 109, // This spell will only work on bixies.
	IS_HARPY                                                                  = 110, // This spell will only work on harpies.
	IS_GNOLL                                                                  = 111, // This spell will only work on gnolls.
	IS_SPORALI                                                                = 112, // This spell will only work on fungusoids.
	IS_KOBOLD                                                                 = 113, // This spell will only work on kobolds.
	IS_FROSTCRYPT_SHADE                                                       = 114, // This spell will only work on undead creatures or the Shades of Frostcrypt.
	IS_DRAKKIN                                                                = 115, // This spell will only work on Drakkin.
	IS_UNDEAD_OR_VALDEHOLM_GIANT                                              = 116, // This spell will only work on undead creatures or the inhabitants of Valdeholm.
	IS_ANIMAL_OR_PLANT                                                        = 117, // This spell will only work on plants or animals.
	IS_SUMMONED                                                               = 118, // This spell will only work on constructs, elementals, or summoned elemental minions.
	IS_WIZARD_USED_ON_MAGE_FIRE_PET                                           = 119, // This spell will only work on wizards. | Live uses this on high level mage fire pets, which are wizard class
	IS_UNDEAD                                                                 = 120, //
	IS_NOT_UNDEAD_OR_SUMMONED_OR_VAMPIRE                                      = 121, // This spell will only work on creatures that are not undead, constructs, elementals, or vampires.
	IS_FAE_OR_PIXIE                                                           = 122, // This spell will only work on Fae or pixies.
	IS_HUMANOID                                                               = 123, //
	IS_UNDEAD_AND_HP_LESS_THAN_10_PCT                                         = 124, // The Essence Extractor whirrs but does not light up.
	IS_CLOCKWORK_AND_HP_LESS_THAN_45_PCT                                      = 125, // This spell will only work on clockwork gnomes.
	IS_WISP_AND_HP_LESS_THAN_10_PCT                                           = 126, // This spell will only work on wisps at or below 10% of their maximum HP.
	IS_CLASS_MELEE_THAT_CAN_BASH_OR_KICK_EXCEPT_BARD                          = 127, // This spell will only work on non-bard targets that can bash or kick.
	IS_CLASS_PURE_MELEE                                                       = 128, // This spell will only affect melee classes (warriors, monks, rogues, and berserkers).
	IS_CLASS_PURE_CASTER                                                      = 129, // This spell will only affect pure caster classes (necromancers, wizards, magicians, and enchanters).
	IS_CLASS_HYBRID_CLASS                                                     = 130, // This spell will only affect hybrid classes (paladins, rangers, shadow knights, bards, and beastlords).
	IS_CLASS_WARRIOR                                                          = 131, // This spell will only affect Warriors.
	IS_CLASS_CLERIC                                                           = 132, // This spell will only affect Clerics.
	IS_CLASS_PALADIN                                                          = 133, // This spell will only affect Paladins.
	IS_CLASS_RANGER                                                           = 134, // This spell will only affect Rangers.
	IS_CLASS_SHADOWKNIGHT                                                     = 135, // This spell will only affect Shadow Knights.
	IS_CLASS_DRUID                                                            = 136, // This spell will only affect Druids.
	IS_CLASS_MONK                                                             = 137, // This spell will only affect Monks.
	IS_CLASS_BARD                                                             = 138, // This spell will only affect Bards.
	IS_CLASS_ROGUE                                                            = 139, // This spell will only affect Rogues.
	IS_CLASS_SHAMAN                                                           = 140, // This spell will only affect Shamans.
	IS_CLASS_NECRO                                                            = 141, // This spell will only affect Necromancers.
	IS_CLASS_WIZARD                                                           = 142, // This spell will only affect Wizards.
	IS_CLASS_MAGE                                                             = 143, // This spell will only affect Magicians.
	IS_CLASS_ENCHANTER                                                        = 144, // This spell will only affect Enchanters.
	IS_CLASS_BEASTLORD                                                        = 145, // This spell will only affect Beastlords.
	IS_CLASS_BERSERKER                                                        = 146, // This spell will only affect Berserkers.
	IS_CLASS_CLR_SHM_DRU                                                      = 147, // This spell will only affect priest classes (clerics, druids, and shaman).
	IS_CLASS_NOT_WAR_PAL_SK                                                   = 148, // This spell will not affect Warriors, Paladins, or Shadow Knights.
	IS_LEVEL_UNDER_100                                                        = 150, // This spell will not affect any target over level 100.
	IS_NOT_RAID_BOSS                                                          = 190, // This spell will not affect raid bosses.
	IS_RAID_BOSS                                                              = 191, // This spell will only affect raid bosses.
	FRENZIED_BURNOUT_ACTIVE                                                   = 192, // This spell will only cast if you have Frenzied Burnout active.
	FRENZIED_BURNOUT_NOT_ACTIVE                                               = 193, // This spell will only cast if you do not have Frenzied Burnout active.
	UNKNOWN_199                                                               = 199, //
	IS_HP_ABOVE_75_PCT                                                        = 201, //
	IS_HP_LESS_THAN_20_PCT                                                    = 203, // Your target's HP must be at 20% of its maximum or below. | caster restriction |
	IS_HP_LESS_THAN_50_PCT                                                    = 204, //	Your target's HP must be at 50% of its maximum or below. | caster restriction |
	IS_HP_LESS_THAN_75_PCT                                                    = 205, // Your target's HP must be at 75% of its maximum or below.
	IS_NOT_IN_COMBAT                                                          = 216, // This spell will only affect creatures that are not in combat.
	HAS_AT_LEAST_1_PET_ON_HATELIST                                            = 221, //
	HAS_AT_LEAST_2_PETS_ON_HATELIST                                           = 222, //
	HAS_AT_LEAST_3_PETS_ON_HATELIST                                           = 223, //
	HAS_AT_LEAST_4_PETS_ON_HATELIST                                           = 224, //
	HAS_AT_LEAST_5_PETS_ON_HATELIST                                           = 225, //
	HAS_AT_LEAST_6_PETS_ON_HATELIST                                           = 226, //
	HAS_AT_LEAST_7_PETS_ON_HATELIST                                           = 227, //
	HAS_AT_LEAST_8_PETS_ON_HATELIST                                           = 228, //
	HAS_AT_LEAST_9_PETS_ON_HATELIST                                           = 229, //
	HAS_AT_LEAST_10_PETS_ON_HATELIST                                          = 230, //
	HAS_AT_LEAST_11_PETS_ON_HATELIST                                          = 231, //
	HAS_AT_LEAST_12_PETS_ON_HATELIST                                          = 232, //
	HAS_AT_LEAST_13_PETS_ON_HATELIST                                          = 233, //
	HAS_AT_LEAST_14_PETS_ON_HATELIST                                          = 234, //
	HAS_AT_LEAST_15_PETS_ON_HATELIST                                          = 235, //
	HAS_AT_LEAST_16_PETS_ON_HATELIST                                          = 236, //
	HAS_AT_LEAST_17_PETS_ON_HATELIST                                          = 237, //
	HAS_AT_LEAST_18_PETS_ON_HATELIST                                          = 238, //
	HAS_AT_LEAST_19_PETS_ON_HATELIST                                          = 239, //
	HAS_AT_LEAST_20_PETS_ON_HATELIST                                          = 240, //
	IS_HP_LESS_THAN_35_PCT                                                    = 250, //	Your target's HP must be at 35% of its maximum or below.
	HAS_BETWEEN_1_TO_2_PETS_ON_HATELIST                                       = 260, // between 1 and 2 pets
	HAS_BETWEEN_3_TO_5_PETS_ON_HATELIST                                       = 261, // between 3 and 5 pets
	HAS_BETWEEN_6_TO_9_PETS_ON_HATELIST                                       = 262, // between 6 and 9 pets
	HAS_BETWEEN_10_TO_14_PETS_ON_HATELIST                                     = 263, // between 10 and 14 pets
	HAS_MORE_THAN_14_PETS_ON_HATELIST                                         = 264, // 15 or more pets
	IS_CLASS_CHAIN_OR_PLATE                                                   = 304, // This spell will only affect plate or chain wearing classes.
	IS_HP_BETWEEN_5_AND_9_PCT                                                 = 350, // Your target's HP must be between 5% and 9% of its maximum.
	IS_HP_BETWEEN_10_AND_14_PCT                                               = 351, //	Your target's HP must be between 10% and 14% of its maximum.
	IS_HP_BETWEEN_15_AND_19_PCT                                               = 352, // Your target's HP must be between 15% and 19% of its maximum.
	IS_HP_BETWEEN_20_AND_24_PCT                                               = 353, //	Your target's HP must be between 20% and 24% of its maximum.
	IS_HP_BETWEEN_25_AND_29_PCT                                               = 354, // Your target's HP must be between 25% and 29% of its maximum.
	IS_HP_BETWEEN_30_AND_34_PCT                                               = 355, //	Your target's HP must be between 30% and 34% of its maximum.
	IS_HP_BETWEEN_35_AND_39_PCT                                               = 356, // Your target's HP must be between 35% and 39% of its maximum.
	IS_HP_BETWEEN_40_AND_44_PCT                                               = 357, //	Your target's HP must be between 40% and 44% of its maximum.
	IS_HP_BETWEEN_45_AND_49_PCT                                               = 358, // Your target's HP must be between 45% and 49% of its maximum.
	IS_HP_BETWEEN_50_AND_54_PCT                                               = 359, //	Your target's HP must be between 50% and 54% of its maximum.
	IS_HP_BETWEEN_55_AND_59_PCT                                               = 360, // Your target's HP must be between 55% and 59% of its maximum.
	IS_HP_BETWEEN_5_AND_15_PCT                                                = 398, //	Your target's HP must be between 5% and 15% of its maximum.
	IS_HP_BETWEEN_15_AND_25_PCT                                               = 399, // Your target's HP must be between 15% and 25% of its maximum.
	IS_HP_BETWEEN_1_AND_25_PCT                                                = 400, //	Your target's HP must be at 25% of its maximum or below.
	IS_HP_BETWEEN_25_AND_35_PCT                                               = 401, // Your target's HP must be between 25% and 35% of its maximum.
	IS_HP_BETWEEN_35_AND_45_PCT                                               = 402, //	Your target's HP must be between 35% and 45% of its maximum.
	IS_HP_BETWEEN_45_AND_55_PCT                                               = 403, // Your target's HP must be between 45% and 55% of its maximum.
	IS_HP_BETWEEN_55_AND_65_PCT                                               = 404, //	Your target's HP must be between 55% and 65% of its maximum.
	IS_HP_BETWEEN_65_AND_75_PCT                                               = 405, // Your target's HP must be between 65% and 75% of its maximum.
	IS_HP_BETWEEN_75_AND_85_PCT                                               = 406, //	Your target's HP must be between 75% and 85% of its maximum.
	IS_HP_BETWEEN_85_AND_95_PCT                                               = 407, // Your target's HP must be between 85% and 95% of its maximum.
	IS_HP_ABOVE_45_PCT                                                        = 408, //	Your target's HP must be at least 45% of its maximum.
	IS_HP_ABOVE_55_PCT                                                        = 409, // Your target's HP must be at least 55% of its maximum.
	UNKNOWN_TOO_MUCH_HP_410                                                   = 410, //	Your target has too much HP to be affected by this spell.
	UNKNOWN_TOO_MUCH_HP_411                                                   = 411, //	Your target has too much HP to be affected by this spell.
	IS_HP_ABOVE_99_PCT                                                        = 412, //
	IS_MANA_ABOVE_10_PCT                                                      = 429, //	You must have at least 10% of your maximum mana available to cast this spell. | caster restriction |
	IS_HP_BELOW_5_PCT                                                         = 501, //
	IS_HP_BELOW_10_PCT                                                        = 502, //
	IS_HP_BELOW_15_PCT                                                        = 503, //
	IS_HP_BELOW_20_PCT                                                        = 504, //	Your target's HP must be at 20% of its maximum or below.
	IS_HP_BELOW_25_PCT                                                        = 505, //
	IS_HP_BELOW_30_PCT                                                        = 506, //
	IS_HP_BELOW_35_PCT                                                        = 507, //
	IS_HP_BELOW_40_PCT                                                        = 508, //
	IS_HP_BELOW_45_PCT                                                        = 509, // Your target's HP must be at 45% of its maximum or below.
	IS_HP_BELOW_50_PCT                                                        = 510, //
	IS_HP_BELOW_55_PCT                                                        = 511, //
	IS_HP_BELOW_60_PCT                                                        = 512, //
	IS_HP_BELOW_65_PCT                                                        = 513, //
	IS_HP_BELOW_70_PCT                                                        = 514, //
	IS_HP_BELOW_75_PCT                                                        = 515, //
	IS_HP_BELOW_80_PCT                                                        = 516, //
	IS_HP_BELOW_85_PCT                                                        = 517, //
	IS_HP_BELOW_90_PCT                                                        = 518, //	This ability requires you to be at or below 90% of your maximum HP.	| caster restriction |
	IS_HP_BELOW_95_PCT                                                        = 519, //
	IS_MANA_BELOW_UNKNOWN_PCT                                                 = 521, //
	IS_ENDURANCE_BELOW_40_PCT                                                 = 522, //
	IS_MANA_BELOW_40_PCT                                                      = 523, //
	IS_HP_ABOVE_20_PCT                                                        = 524, //	Your target's HP must be at least 21% of its maximum.
	IS_BODY_TYPE_UNDEFINED                                                    = 600, // This spell will only work on creatures with an undefined body type.
	IS_BODY_TYPE_HUMANOID                                                     = 601, // This spell will only work on humanoid creatures.
	IS_BODY_TYPE_WEREWOLF													  = 602, // This spell will only work on lycanthrope creatures.
	IS_BODY_TYPE_UNDEAD                                                       = 603, // This spell will only work on undead creatures.
	IS_BODY_TYPE_GIANTS                                                       = 604, // This spell will only work on giants.
	IS_BODY_TYPE_CONSTRUCTS                                                   = 605, // This spell will only work on constructs.
	IS_BODY_TYPE_EXTRAPLANAR                                                  = 606, // This spell will only work on extraplanar creatures.
	IS_BODY_TYPE_MAGICAL_CREATURE                                             = 607, // This spell will only work on creatures constructed from magic.
	IS_BODY_TYPE_UNDEADPET                                                    = 608, // This spell will only work on animated undead servants.
	IS_BODY_TYPE_KAELGIANT                                                    = 609, // This spell will only work on the Giants of Kael Drakkal.
	IS_BODY_TYPE_COLDAIN                                                      = 610, // This spell will only work on Coldain Dwarves.
	IS_BODY_TYPE_VAMPIRE                                                      = 612, // This spell will only work on vampires.
	IS_BODY_TYPE_ATEN_HA_RA                                                   = 613, // This spell will only work on Aten Ha Ra.
	IS_BODY_TYPE_GREATER_AHKEVANS                                             = 614, // This spell will only work on Greater Ahkevans.
	IS_BODY_TYPE_KHATI_SHA                                                    = 615, // This spell will only work on Khati Sha.
	IS_BODY_TYPE_LORD_INQUISITOR_SERU                                         = 616, // This spell will only work on Lord Inquisitor Seru.
	IS_BODY_TYPE_GRIEG_VENEFICUS                                              = 617, // This spell will only work on Grieg Veneficus.
	IS_BODY_TYPE_FROM_PLANE_OF_WAR                                            = 619, // This spell will only work on creatures from the Plane of War.
	IS_BODY_TYPE_LUGGALD                                                      = 620, // This spell will only work on Luggalds.
	IS_BODY_TYPE_ANIMAL                                                       = 621, // This spell will only work on animals.
	IS_BODY_TYPE_INSECT                                                       = 622, // This spell will only work on insects.
	IS_BODY_TYPE_MONSTER                                                      = 623, // This spell will only work on monsters.
	IS_BODY_TYPE_ELEMENTAL                                                    = 624, // This spell will only work on elemental creatures.
	IS_BODY_TYPE_PLANT                                                        = 625, // This spell will only work on plants.
	IS_BODY_TYPE_DRAGON2                                                      = 626, // This spell will only work on dragons.
	IS_BODY_TYPE_SUMMONED_ELEMENTAL                                           = 627, // This spell will only work on summoned elementals.
	IS_BODY_TYPE_WARDER                                                       = 628, //
	IS_BODY_TYPE_DRAGON_OF_TOV                                                = 630, // This spell will only work on Dragons of Veeshan's Temple.
	IS_BODY_TYPE_FAMILIAR                                                     = 631, //	This spell will only work on familiars.
	IS_BODY_TYPE_MURAMITE                                                     = 634, //	This spell will only work on Muramites.
	IS_NOT_UNDEAD_OR_SUMMONED                                                 = 635, //
	IS_NOT_PLANT                                                              = 636, //	This spell will not affect plants.
	IS_NOT_CLIENT                                                             = 700, //	This spell will not work on adventurers.
	IS_CLIENT                                                                 = 701, //	This spell will only work on adventurers.
	IS_LEVEL_ABOVE_42_AND_IS_CLIENT                                           = 800, //	This spell will only work on level 43 or higher adventurers.
	UNKNOWN_812                                                               = 812, //	| seen in spell 22616 Thaumatize Pet Mana Regen Base |
	UNKNOWN_814                                                               = 814, //	| seen in spell 22704 Vegetentacles I |
	IS_TREANT                                                                 = 815, //	This spell will only work on treants.
	IS_BIXIE2                                                                 = 816, //	This spell will only work on bixies.
	IS_SCARECROW                                                              = 817, //	This spell will only work on scarecrows.
	IS_VAMPIRE_OR_UNDEAD_OR_UNDEADPET                                         =	818, //	This spell will only work on vampires, undead, or animated undead creatures.
	IS_NOT_VAMPIRE_OR_UNDEAD                                                  = 819, //	This spell will not work on vampires or undead creatures.
	IS_CLASS_KNIGHT_HYBRID_MELEE                                              = 820, //	This spell will only work on knights, hybrids, or melee classes.
	IS_CLASS_WARRIOR_CASTER_PRIEST                                            = 821, //	This spell will only work on warriors, casters, or priests.
	UNKNOWN_822                                                               = 822, // | seen in spell 22870 Morell's Distraction 822 |
	IS_END_BELOW_21_PCT                                                       = 825, //	This ability requires you to be at or below 21% of your maximum endurance.
	IS_END_BELOW_25_PCT                                                       = 826, //	This ability requires you to be at or below 25% of your maximum endurance.
	IS_END_BELOW_29_PCT                                                       = 827, //	This ability requires you to be at or below 29% of your maximum endurance.
	IS_REGULAR_SERVER                                                         = 836, //
	IS_PROGRESSION_SERVER                                                     = 837, //
	IS_GOD_EXPANSION_UNLOCKED                                                 = 839, //
	UNKNOWN_840                                                               = 840, // | caster restriction | seen in spell 6883 Expedient Recovery
	UNKNOWN_841                                                               = 841, // | caster restriction | seen in spell 32192 Merciless Blow
	IS_HUMANOID_LEVEL_84_MAX                                                  = 842, //
	IS_HUMANOID_LEVEL_86_MAX                                                  = 843, //
	IS_HUMANOID_LEVEL_88_MAX                                                  = 844, //
	HAS_CRYSTALLIZED_FLAME_BUFF                                               = 845, //	This spell will only work on targets afflicted by Crystallized Flame. | On live spell does not appear to be a buff
	HAS_INCENDIARY_OOZE_BUFF                                                  = 847, //	This spell will only work on targets afflicted by Incendiary Ooze.
	IS_LEVEL_90_MAX                                                           = 860, //
	IS_LEVEL_92_MAX                                                           = 861, //
	IS_LEVEL_94_MAX                                                           = 862, //
	IS_LEVEL_95_MAX                                                           = 863, //
	IS_LEVEL_97_MAX                                                           = 864, //
	IS_LEVEL_99_MAX                                                           = 865, //
	HAS_WEAPONSTANCE_DEFENSIVE_PROFICIENCY                                    = 866, // | caster restriction |
	HAS_WEAPONSTANCE_TWO_HAND_PROFICIENCY                                     = 867, // | caster restriction |
	HAS_WEAPONSTANCE_DUAL_WEILD_PROFICIENCY                                   = 868, // | caster restriction |
	IS_LEVEL_100_MAX                                                          = 869, //
	IS_LEVEL_102_MAX                                                          = 870, //
	IS_LEVEL_104_MAX                                                          = 871, //
	IS_LEVEL_105_MAX                                                          = 872, //
	IS_LEVEL_107_MAX                                                          = 873, //
	IS_LEVEL_109_MAX                                                          = 874, //
	IS_LEVEL_110_MAX                                                          = 875, //
	IS_LEVEL_112_MAX                                                          = 876, //
	IS_LEVEL_114_MAX                                                          = 877, //
	HAS_TBL_ESIANTI_ACCESS                                                    = 997, //	This spell will only transport adventurers who have gained access to Esianti: Palace of the Winds.	| not implemented
	HAS_ITEM_CLOCKWORK_SCRAPS                                                 = 999, //
	IS_BETWEEN_LEVEL_1_AND_75                                                 = 1000, //
	IS_BETWEEN_LEVEL_76_AND_85                                                = 1001, //
	IS_BETWEEN_LEVEL_86_AND_95                                                = 1002, //
	IS_BETWEEN_LEVEL_96_AND_105                                               = 1003, //
	IS_HP_LESS_THAN_80_PCT                                                    = 1004, //
	IS_LEVEL_ABOVE_34                                                         = 1474, // Your target must be level 35 or higher.
	IN_TWO_HANDED_STANCE                                                      = 2000, // You must be in your two-handed stance to use this ability.
	IN_DUAL_WIELD_HANDED_STANCE                                               = 2001, // You must be in your dual-wielding stance to use this ability.
	IN_SHIELD_STANCE                                                          = 2002, // You must be in your shield stance to use this ability.
	NOT_IN_TWO_HANDED_STANCE                                                  = 2010, // You may not use this ability if you are in your two-handed stance.
	NOT_IN_DUAL_WIELD_HANDED_STANCE                                           = 2011, // You may not use this ability if you are in your dual-wielding stance.
	NOT_IN_SHIELD_STANCE                                                      = 2012, // You may not use this ability if you are in your shield stance.
	LEVEL_46_MAX                                                              = 2761, //
	DISABLED_UNTIL_EXPANSION_ROK                                              = 7000, // This ability is disabled until Ruins of Kunark.
	DISABLED_UNTIL_EXPANSION_SOV                                              = 7001, // This ability is disabled until Scars of Velious.
	DISABLED_UNTIL_EXPANSION_SOL                                              = 7002, // This ability is disabled until Shadows of Luclin.
	DISABLED_UNTIL_EXPANSION_POP                                              = 7003, // This ability is disabled until Planes of Power.
	DISABLED_UNTIL_EXPANSION_LOY                                              = 7004, // This ability is disabled until Legacy of Ykesha.
	DISABLED_UNTIL_EXPANSION_LDON                                             = 7005, // This ability is disabled until Lost Dungeons of Norrath.
	DISABLED_UNTIL_EXPANSION_GOD                                              = 7006, // This ability is disabled until Gates of Discord.
	DISABLED_UNTIL_EXPANSION_OOW                                              = 7007, // This ability is disabled until Omens of War.
	DISABLED_UNTIL_EXPANSION_DON                                              = 7008, // This ability is disabled until Dragons of Norrath.
	DISABLED_UNTIL_EXPANSION_DOD                                              = 7009, // This ability is disabled until Depths of Darkhollow.
	DISABLED_UNTIL_EXPANSION_POR                                              = 7010, // This ability is disabled until Prophecy of Ro.
	DISABLED_UNTIL_EXPANSION_TSS                                              = 7011, // This ability is disabled until Serpent's Spine.
	DISABLED_UNTIL_EXPANSION_TBS                                              = 7012, // This ability is disabled until Buried Sea.
	DISABLED_UNTIL_EXPANSION_SOF                                              = 7013, // This ability is disabled until Secrets of Faydwer.
	DISABLED_UNTIL_EXPANSION_SOD                                              = 7014, // This ability is disabled until Seeds of Destruction.
	DISABLED_UNTIL_EXPANSION_UF                                               = 7015, // This ability is disabled until Underfoot.
	DISABLED_UNTIL_EXPANSION_HOT                                              = 7016, // This ability is disabled until House of Thule.
	DISABLED_UNTIL_EXPANSION_VOA                                              = 7017, // This ability is disabled until Veil of Alaris.
	DISABLED_UNTIL_EXPANSION_ROF                                              = 7018, // This ability is disabled until Rain of Fear.
	DISABLED_UNTIL_EXPANSION_COF                                              = 7019, // This ability is disabled until Call of the Forsaken.
	DISABLED_UNTIL_EXPANSION_TDS                                              = 7020, // This ability is disabled until Darkened Sea.
	DISABLED_UNTIL_EXPANSION_TBM                                              = 7021, // This ability is disabled until Broken Mirror.
	DISABLED_UNTIL_EXPANSION_EOK                                              = 7022, // This ability is disabled until Empires of Kunark.
	DISABLED_UNTIL_EXPANSION_ROS                                              = 7023, // This ability is disabled until Ring of Scale.
	DISABLED_UNTIL_EXPANSION_TBL                                              = 7024, // This ability is disabled until The Burning Lands.
	DISABLED_UNTIL_EXPANSION_TOV                                              = 7025, // This ability is disabled until Torment of Velious.
	DISABLED_UNTIL_EXPANSION_COV                                              = 7026, // This ability is disabled until Claws of Veeshan.
	HAS_NO_MANA_BURN_BUFF                                                     = 8450,  // This spell will not take hold until the effects of the previous Mana Burn have expired.
	IS_RACE_FIRST_CUSTOM                                                      = 10000, // | custom range to restrict targets or casters by race *not on live* |
	IS_RACE_LAST_CUSTOM                                                       = 11000, // | custom range to restrict targets or casters by race *not on live* |
	IS_CLIENT_AND_MALE_PLATE_USER                                             = 11044, // Your target wouldn't look right as that Jann.
	IS_CLEINT_AND_MALE_DRUID_ENCHANTER_MAGICIAN_NECROANCER_SHAMAN_OR_WIZARD   = 11090, // Your target wouldn't look right as that Jann.
	IS_CLIENT_AND_MALE_BEASTLORD_BERSERKER_MONK_RANGER_OR_ROGUE               = 11209, // Your target wouldn't look right as that Jann.
	IS_CLIENT_AND_FEMALE_PLATE_USER                                           = 11210, // Your target wouldn't look right as that Jann.
	IS_CLIENT_AND_FEMALE_DRUID_ENCHANTER_MAGICIAN_NECROANCER_SHAMAN_OR_WIZARD = 11211, // Your target wouldn't look right as that Jann.
	IS_CLIENT_AND_FEMALE_BEASTLORD_BERSERKER_MONK_RANGER_OR_ROGUE             = 11248, // Your target wouldn't look right as that Jann.
	HAS_TRAVELED_TO_STRATOS                                                   = 11260, // You must travel to Stratos at least once before wishing to go there.
	HAS_TRAVELED_TO_AALISHAI                                                  = 11261, // You must travel to Aalishai at least once before wishing to go there.
	HAS_TRAVELED_TO_MEARATS                                                   = 11268, // You must travel to Mearatas at least once before wishing to go there.
	HAS_NO_ILLUSIONS_OF_GRANDEUR_BUFF                                         = 12519, //
	IS_HP_ABOVE_50_PCT                                                        = 16010, //
	IS_HP_UNDER_50_PCT                                                        = 16031, //
	IS_OFF_HAND_EQUIPPED                                                       = 27672, // You must be wielding a weapon or shield in your offhand to use this ability.
	HAS_NO_PACT_OF_FATE_RECOURSE_BUFF                                         = 29556, // This spell will not work while Pact of Fate Recourse is active. | caster restriction |
	HAS_NO_SHROUD_OF_PRAYER_BUFF                                              = 32339, // Your target cannot receive another Quiet Prayer this soon.
	IS_MANA_BELOW_20_PCT                                                      = 38311, // This ability requires you to be at or below 20% of your maximum mana.
	IS_MANA_ABOVE_50_PCT                                                      = 38312, // This ability requires you to be at or above 50% of your maximum mana.
	COMPLETED_ACHIEVEMENT_LEGENDARY_ANSWERER                                  = 39281, // You have completed Legendary Answerer.
	HAS_NO_ROGUES_FURY_BUFF                                                   = 40297, // This spell will not affect anyone that currently has Rogue's Fury active.	| caster restriction |
	NOT_COMPLETED_ACHIEVEMENT_LEGENDARY_ANSWERER                              = 42280, // You must complete Legendary Answerer.
	IS_SUMMONED_OR_UNDEAD                                                     = 49326, //
	IS_CLASS_CASTER_PRIEST                                                    = 49529, //
	IS_END_OR_MANA_ABOVE_20_PCT                                               = 49543, // You must have at least 20% of your maximum mana and endurance to use this ability.	//pure melee class check end, other check mana
	IS_END_OR_MANA_BELOW_10_PCT                                               = 49545, // 																			//pure melee class check end, other check mana, hybrid check both
	IS_END_OR_MANA_BELOW_30_PCT                                               = 49573, // Your target already has 30% or more of their maximum mana or endurance.	//pure melee class check the, other check more
	IS_CLASS_BARD2                                                            = 49574, //
	IS_NOT_CLASS_BARD                                                         = 49575, //
	HAS_NO_FURIOUS_RAMPAGE_BUFF                                               = 49612, // This ability cannot be activated while Furious Rampage is active.
	IS_END_OR_MANA_BELOW_30_PCT2                                              = 49809, // You can only perform this solo if you have less than 30% mana or endurance.
	HAS_NO_HARMONIOUS_PRECISION_BUFF                                          = 50003, // This spell will not work if you have the Harmonious Precision line active.
	HAS_NO_HARMONIOUS_EXPANSE_BUFF                                            = 50009, // This spell will not work if you have the Harmonious Expanse line active.
	UNKNOWN_99999                                                             = 99999, // | caster restriction | works will spell 27672 Strike of Ire
};

enum NegateSpellEffectType
{
	NEGATE_SPA_ALL_BONUSES                = 0,
	NEGATE_SPA_SPELLBONUS                 = 1,
	NEGATE_SPA_ITEMBONUS                  = 2,
	NEGATE_SPA_SPELLBONUS_AND_ITEMBONUS   = 3,
	NEGATE_SPA_AABONUS                    = 4,
	NEGATE_SPA_SPELLBONUS_AND_AABONUS     = 5,
	NEGATE_SPA_ITEMBONUS_AND_AABONUS      = 6,
};
//Used for rule RuleI(Spells, ReflectType))
enum ReflectSpellType
{
	REFLECT_DISABLED                  = 0,
	REFLECT_SINGLE_TARGET_SPELLS_ONLY = 1,
	REFLECT_ALL_PLAYER_SPELLS         = 2,
	RELFECT_ALL_SINGLE_TARGET_SPELLS  = 3,
	REFLECT_ALL_SPELLS                = 4,
};

enum InvisType {
	T_INVISIBLE						= 0,
	T_INVISIBLE_VERSE_UNDEAD		= 1,
	T_INVISIBLE_VERSE_ANIMAL		= 2,
};

//For better organizing in proc effects, not used in spells.
enum ProcType
{
	MELEE_PROC             = 1,
	RANGED_PROC            = 2,
	DEFENSIVE_PROC         = 3,
	SKILL_PROC             = 4,
	SKILL_PROC_SUCCESS     = 5,
};

enum SpellTypes : uint32
{
	SpellType_Nuke                   = (1 << 0),
	SpellType_Heal                   = (1 << 1),
	SpellType_Root                   = (1 << 2),
	SpellType_Buff                   = (1 << 3),
	SpellType_Escape                 = (1 << 4),
	SpellType_Pet                    = (1 << 5),
	SpellType_Lifetap                = (1 << 6),
	SpellType_Snare                  = (1 << 7),
	SpellType_DOT                    = (1 << 8),
	SpellType_Dispel                 = (1 << 9),
	SpellType_InCombatBuff           = (1 << 10),
	SpellType_Mez                    = (1 << 11),
	SpellType_Charm                  = (1 << 12),
	SpellType_Slow                   = (1 << 13),
	SpellType_Debuff                 = (1 << 14),
	SpellType_Cure                   = (1 << 15),
	SpellType_Resurrect              = (1 << 16),
	SpellType_HateRedux              = (1 << 17),
	SpellType_InCombatBuffSong       = (1 << 18), // bard in-combat group/ae buffs
	SpellType_OutOfCombatBuffSong    = (1 << 19), // bard out-of-combat group/ae buffs
	SpellType_PreCombatBuff          = (1 << 20),
	SpellType_PreCombatBuffSong      = (1 << 21)
};

namespace BotSpellTypes
{
	constexpr uint16	Nuke                       = 0;
	constexpr uint16	RegularHeal                = 1;
	constexpr uint16	Root                       = 2;
	constexpr uint16	Buff                       = 3;
	constexpr uint16	Escape                     = 4;
	constexpr uint16	Pet                        = 5;
	constexpr uint16	Lifetap                    = 6;
	constexpr uint16	Snare                      = 7;
	constexpr uint16	DOT                        = 8;
	constexpr uint16	Dispel                     = 9;
	constexpr uint16	InCombatBuff               = 10;
	constexpr uint16	Mez                        = 11;
	constexpr uint16	Charm                      = 12;
	constexpr uint16	Slow                       = 13;
	constexpr uint16	Debuff                     = 14;
	constexpr uint16	Cure                       = 15;
	constexpr uint16	Resurrect                  = 16;
	constexpr uint16	HateRedux                  = 17;
	constexpr uint16	InCombatBuffSong           = 18;
	constexpr uint16	OutOfCombatBuffSong        = 19;
	constexpr uint16	PreCombatBuff              = 20;
	constexpr uint16	PreCombatBuffSong          = 21;
	constexpr uint16	Fear                       = 22;
	constexpr uint16	Stun                       = 23;
	constexpr uint16	HateLine                   = 24;
	constexpr uint16	GroupCures                 = 25;
	constexpr uint16	CompleteHeal               = 26;
	constexpr uint16	FastHeals                  = 27;
	constexpr uint16	VeryFastHeals              = 28;
	constexpr uint16	GroupHeals                 = 29;
	constexpr uint16	GroupCompleteHeals         = 30;
	constexpr uint16	GroupHoTHeals              = 31;
	constexpr uint16	HoTHeals                   = 32;
	constexpr uint16	AENukes                    = 33;
	constexpr uint16	AERains                    = 34;
	constexpr uint16	AEMez                      = 35;
	constexpr uint16	AEStun                     = 36;
	constexpr uint16	AEDebuff                   = 37;
	constexpr uint16	AESlow                     = 38;
	constexpr uint16	AESnare                    = 39;
	constexpr uint16	AEFear                     = 40;
	constexpr uint16	AEDispel                   = 41;
	constexpr uint16	AERoot                     = 42;
	constexpr uint16	AEDoT                      = 43;
	constexpr uint16	AELifetap                  = 44;
	constexpr uint16	AEHateLine                 = 45;
	constexpr uint16	PBAENuke                   = 46;
	constexpr uint16	PetBuffs                   = 47;
	constexpr uint16	PetRegularHeals            = 48;
	constexpr uint16	PetCompleteHeals           = 49;
	constexpr uint16	PetFastHeals               = 50;
	constexpr uint16	PetVeryFastHeals           = 51;
	constexpr uint16	PetHoTHeals                = 52;
	constexpr uint16	PetCures                   = 53;
	constexpr uint16	DamageShields              = 54;
	constexpr uint16	ResistBuffs                = 55;
	constexpr uint16	PetDamageShields           = 56;
	constexpr uint16	PetResistBuffs             = 57;

	// Command Spell Types
	constexpr uint16	Teleport                   = 100; // this is handled by ^depart so uses other logic
	constexpr uint16	Lull                       = 101;
	constexpr uint16	Succor                     = 102;
	constexpr uint16	BindAffinity               = 103;
	constexpr uint16	Identify                   = 104;
	constexpr uint16	Levitate                   = 105;
	constexpr uint16	Rune                       = 106;
	constexpr uint16	WaterBreathing             = 107;
	constexpr uint16	Size                       = 108;
	constexpr uint16	Invisibility               = 109;
	constexpr uint16	MovementSpeed              = 110;
	constexpr uint16	SendHome                   = 111;
	constexpr uint16	SummonCorpse               = 112;
	constexpr uint16	AELull                     = 113;

	// Discipline Types
	constexpr uint16	Discipline                 = 200;
	constexpr uint16	DiscAggressive             = 201;
	constexpr uint16	DiscDefensive              = 202;
	constexpr uint16	DiscUtility                = 203;

	constexpr uint16	START                      = BotSpellTypes::Nuke;              // Do not remove or change this
	constexpr uint16	END                        = BotSpellTypes::PetResistBuffs;    // Do not remove this, increment as needed
	constexpr uint16	COMMANDED_START            = BotSpellTypes::Lull;              // Do not remove or change this
	constexpr uint16	COMMANDED_END              = BotSpellTypes::AELull;            // Do not remove this, increment as needed
	constexpr uint16	DISCIPLINE_START           = BotSpellTypes::Discipline;        // Do not remove or change this
	constexpr uint16	DISCIPLINE_END             = BotSpellTypes::DiscUtility;       // Do not remove this, increment as needed
	constexpr uint16 	PARENT_TYPE_END            = BotSpellTypes::PreCombatBuffSong; // This is the last ID of the original bot spell types, the rest are considered sub types.
}

static std::map<uint16, std::string> spell_type_names = {
	{ BotSpellTypes::Nuke,                     "Nuke" },
	{ BotSpellTypes::RegularHeal,              "Regular Heal" },
	{ BotSpellTypes::Root,                     "Root" },
	{ BotSpellTypes::Buff,                     "Buff" },
	{ BotSpellTypes::Escape,                   "Escape" },
	{ BotSpellTypes::Pet,                      "Pet" },
	{ BotSpellTypes::Lifetap,                  "Lifetap" },
	{ BotSpellTypes::Snare,                    "Snare" },
	{ BotSpellTypes::DOT,                      "DoT" },
	{ BotSpellTypes::Dispel,                   "Dispel" },
	{ BotSpellTypes::InCombatBuff,             "In-Combat Buff" },
	{ BotSpellTypes::Mez,                      "Mez" },
	{ BotSpellTypes::Charm,                    "Charm" },
	{ BotSpellTypes::Slow,                     "Slow" },
	{ BotSpellTypes::Debuff,                   "Debuff" },
	{ BotSpellTypes::Cure,                     "Cure" },
	{ BotSpellTypes::GroupCures,               "Group Cure" },
	{ BotSpellTypes::PetCures,                 "Pet Cure" },
	{ BotSpellTypes::Resurrect,                "Resurrect" },
	{ BotSpellTypes::HateRedux,                "Hate Reduction" },
	{ BotSpellTypes::InCombatBuffSong,         "In-Combat Buff Song" },
	{ BotSpellTypes::OutOfCombatBuffSong,      "Out-of-Combat Buff Song" },
	{ BotSpellTypes::PreCombatBuff,            "Pre-Combat Buff" },
	{ BotSpellTypes::PreCombatBuffSong,        "Pre-Combat Buff Song" },
	{ BotSpellTypes::Fear,                     "Fear" },
	{ BotSpellTypes::Stun,                     "Stun" },
	{ BotSpellTypes::CompleteHeal,             "Complete Heal" },
	{ BotSpellTypes::FastHeals,                "Fast Heal" },
	{ BotSpellTypes::VeryFastHeals,            "Very Fast Heal" },
	{ BotSpellTypes::GroupHeals,               "Group Heal" },
	{ BotSpellTypes::GroupCompleteHeals,       "Group Complete Heal" },
	{ BotSpellTypes::GroupHoTHeals,            "Group HoT Heal" },
	{ BotSpellTypes::HoTHeals,                 "HoT Heal" },
	{ BotSpellTypes::AENukes,                  "AE Nuke" },
	{ BotSpellTypes::AERains,                  "AE Rain" },
	{ BotSpellTypes::AEMez,                    "AE Mez" },
	{ BotSpellTypes::AEStun,                   "AE Stun" },
	{ BotSpellTypes::AEDebuff,                 "AE Debuff" },
	{ BotSpellTypes::AESlow,                   "AE Slow" },
	{ BotSpellTypes::AESnare,                  "AE Snare" },
	{ BotSpellTypes::AEFear,                   "AE Fear" },
	{ BotSpellTypes::AEDispel,                 "AE Dispel" },
	{ BotSpellTypes::AERoot,                   "AE Root" },
	{ BotSpellTypes::AEDoT,                    "AE DoT" },
	{ BotSpellTypes::AELifetap,                "AE Lifetap" },
	{ BotSpellTypes::PBAENuke,                 "PBAE Nuke" },
	{ BotSpellTypes::PetBuffs,                 "Pet Buff" },
	{ BotSpellTypes::PetRegularHeals,          "Pet Regular Heal" },
	{ BotSpellTypes::PetCompleteHeals,         "Pet Complete Heal" },
	{ BotSpellTypes::PetFastHeals,             "Pet Fast Heal" },
	{ BotSpellTypes::PetVeryFastHeals,         "Pet Very Fast Heal" },
	{ BotSpellTypes::PetHoTHeals,              "Pet HoT Heal" },
	{ BotSpellTypes::DamageShields,            "Damage Shield" },
	{ BotSpellTypes::ResistBuffs,              "Resist Buff" },
	{ BotSpellTypes::PetDamageShields,         "Pet Damage Shield" },
	{ BotSpellTypes::PetResistBuffs,           "Pet Resist Buff" },
	{ BotSpellTypes::HateLine,                 "Hate Line" },
	{ BotSpellTypes::AEHateLine,               "AE Hate Line" },
	{ BotSpellTypes::Lull,                     "Lull" },
	{ BotSpellTypes::Teleport,                 "Teleport" },
	{ BotSpellTypes::Succor,                   "Succor" },
	{ BotSpellTypes::BindAffinity,             "Bind Affinity" },
	{ BotSpellTypes::Identify,                 "Identify" },
	{ BotSpellTypes::Levitate,                 "Levitate" },
	{ BotSpellTypes::Rune,                     "Rune" },
	{ BotSpellTypes::WaterBreathing,           "Water Breathing" },
	{ BotSpellTypes::Size,                     "Size" },
	{ BotSpellTypes::Invisibility,             "Invisibility" },
	{ BotSpellTypes::MovementSpeed,            "Movement Speed" },
	{ BotSpellTypes::SendHome,                 "Send Home" },
	{ BotSpellTypes::SummonCorpse,             "Summon Corpse" },
	{ BotSpellTypes::AELull,                   "AE Lull" }
};

static std::map<uint16, std::string> spell_type_short_names = {
	{ BotSpellTypes::Nuke,                     "nukes" },
	{ BotSpellTypes::RegularHeal,              "regularheals" },
	{ BotSpellTypes::Root,                     "roots" },
	{ BotSpellTypes::Buff,                     "buffs" },
	{ BotSpellTypes::Escape,                   "escapes" },
	{ BotSpellTypes::Pet,                      "pets" },
	{ BotSpellTypes::Lifetap,                  "lifetaps" },
	{ BotSpellTypes::Snare,                    "snares" },
	{ BotSpellTypes::DOT,                      "dots" },
	{ BotSpellTypes::Dispel,                   "dispels" },
	{ BotSpellTypes::InCombatBuff,             "incombatbuffs" },
	{ BotSpellTypes::Mez,                      "mez" },
	{ BotSpellTypes::Charm,                    "charms" },
	{ BotSpellTypes::Slow,                     "slows" },
	{ BotSpellTypes::Debuff,                   "debuffs" },
	{ BotSpellTypes::Cure,                     "cures" },
	{ BotSpellTypes::GroupCures,               "groupcures" },
	{ BotSpellTypes::PetCures,                 "petcures" },
	{ BotSpellTypes::Resurrect,                "resurrects" },
	{ BotSpellTypes::HateRedux,                "hateredux" },
	{ BotSpellTypes::InCombatBuffSong,         "incombatbuffsongs" },
	{ BotSpellTypes::OutOfCombatBuffSong,      "outofcombatbuffsongs" },
	{ BotSpellTypes::PreCombatBuff,            "precombatbuffs" },
	{ BotSpellTypes::PreCombatBuffSong,        "precombatbuffsongs" },
	{ BotSpellTypes::Fear,                     "fears" },
	{ BotSpellTypes::Stun,                     "stuns" },
	{ BotSpellTypes::CompleteHeal,             "completeheals" },
	{ BotSpellTypes::FastHeals,                "fastheals" },
	{ BotSpellTypes::VeryFastHeals,            "veryfastheals" },
	{ BotSpellTypes::GroupHeals,               "groupheals" },
	{ BotSpellTypes::GroupCompleteHeals,       "groupcompleteheals" },
	{ BotSpellTypes::GroupHoTHeals,            "grouphotheals" },
	{ BotSpellTypes::HoTHeals,                 "hotheals" },
	{ BotSpellTypes::AENukes,                  "aenukes" },
	{ BotSpellTypes::AERains,                  "aerains" },
	{ BotSpellTypes::AEMez,                    "aemez" },
	{ BotSpellTypes::AEStun,                   "aestuns" },
	{ BotSpellTypes::AEDebuff,                 "aedebuffs" },
	{ BotSpellTypes::AESlow,                   "aeslows" },
	{ BotSpellTypes::AESnare,                  "aesnares" },
	{ BotSpellTypes::AEFear,                   "aefears" },
	{ BotSpellTypes::AEDispel,                 "aedispels" },
	{ BotSpellTypes::AERoot,                   "aeroots" },
	{ BotSpellTypes::AEDoT,                    "aedots" },
	{ BotSpellTypes::AELifetap,                "aelifetaps" },
	{ BotSpellTypes::PBAENuke,                 "pbaenukes" },
	{ BotSpellTypes::PetBuffs,                 "petbuffs" },
	{ BotSpellTypes::PetRegularHeals,          "petregularheals" },
	{ BotSpellTypes::PetCompleteHeals,         "petcompleteheals" },
	{ BotSpellTypes::PetFastHeals,             "petfastheals" },
	{ BotSpellTypes::PetVeryFastHeals,         "petveryfastheals" },
	{ BotSpellTypes::PetHoTHeals,              "pethotheals" },
	{ BotSpellTypes::DamageShields,            "damageshields" },
	{ BotSpellTypes::ResistBuffs,              "resistbuffs" },
	{ BotSpellTypes::PetDamageShields,         "petdamageshields" },
	{ BotSpellTypes::PetResistBuffs,           "petresistbuffs" },
	{ BotSpellTypes::HateLine,                 "hatelines" },
	{ BotSpellTypes::AEHateLine,               "aehatelines" },
	{ BotSpellTypes::Lull,                     "lull" },
	{ BotSpellTypes::Teleport,                 "teleport" },
	{ BotSpellTypes::Succor,                   "succor" },
	{ BotSpellTypes::BindAffinity,             "bindaffinity" },
	{ BotSpellTypes::Identify,                 "identify" },
	{ BotSpellTypes::Levitate,                 "levitate" },
	{ BotSpellTypes::Rune,                     "rune" },
	{ BotSpellTypes::WaterBreathing,           "waterbreathing" },
	{ BotSpellTypes::Size,                     "size" },
	{ BotSpellTypes::Invisibility,             "invisibility" },
	{ BotSpellTypes::MovementSpeed,            "movementspeed" },
	{ BotSpellTypes::SendHome,                 "sendhome" },
	{ BotSpellTypes::SummonCorpse,             "summoncorpse" },
	{ BotSpellTypes::AELull,                   "aelull" }
};

const uint32 SPELL_TYPES_DETRIMENTAL = (SpellType_Nuke | SpellType_Root | SpellType_Lifetap | SpellType_Snare | SpellType_DOT | SpellType_Dispel | SpellType_Mez | SpellType_Charm | SpellType_Debuff | SpellType_Slow);
const uint32 SPELL_TYPES_BENEFICIAL = (SpellType_Heal | SpellType_Buff | SpellType_Escape | SpellType_Pet | SpellType_InCombatBuff | SpellType_Cure | SpellType_HateRedux | SpellType_InCombatBuffSong | SpellType_OutOfCombatBuffSong | SpellType_PreCombatBuff | SpellType_PreCombatBuffSong);
const uint32 SPELL_TYPES_INNATE = (SpellType_Nuke | SpellType_Lifetap | SpellType_DOT | SpellType_Dispel | SpellType_Mez | SpellType_Slow | SpellType_Debuff | SpellType_Charm | SpellType_Root);

// Bot related functions
bool IsBotSpellTypeDetrimental(uint16 spell_type);
bool IsBotSpellTypeBeneficial(uint16 spell_type);
bool BotSpellTypeUsesTargetSettings(uint16 spell_type);
bool IsBotSpellTypeInnate (uint16 spell_type);
bool IsAEBotSpellType(uint16 spell_type);
bool IsGroupBotSpellType(uint16 spell_type);
bool IsGroupTargetOnlyBotSpellType(uint16 spell_type);
bool IsPetBotSpellType(uint16 spell_type);
bool IsClientBotSpellType(uint16 spell_type);
bool IsHealBotSpellType(uint16 spell_type);
bool BotSpellTypeRequiresLoS(uint16 spell_type);
bool BotSpellTypeRequiresTarget(uint16 spell_type);
bool BotSpellTypeRequiresAEChecks(uint16 spell_type);
bool IsCommandedBotSpellType(uint16 spell_type);
bool IsPullingBotSpellType(uint16 spell_type);
uint16 GetCorrectBotSpellType(uint16 spell_type, uint16 spell_id);
uint16 GetPetBotSpellType(uint16 spell_type);
bool IsBotBuffSpellType(uint16 spell_type);
bool BotRequiresLoSToCast(uint16 spell_type, uint16 spell_id);

// These should not be used to determine spell category..
// They are a graphical affects (effects?) index only
// TODO: import sai list
enum spell_affect_index {
	SAI_Summon_Mount_Unclass	= -1,
	SAI_Direct_Damage			= 0,
	SAI_Heal_Cure				= 1,
	SAI_AC_Buff					= 2,
	SAI_AE_Damage				= 3,
	SAI_Summon					= 4,	// Summoned Pets and Items
	SAI_Sight					= 5,
	SAI_Mana_Regen_Resist_Song	= 6,
	SAI_Stat_Buff				= 7,
	SAI_Vanish					= 9,	// Invisibility and Gate/Port
	SAI_Appearance				= 10,	// Illusion and Size
	SAI_Enchanter_Pet			= 11,
	SAI_Calm					= 12,	// Lull and Alliance Spells
	SAI_Fear					= 13,
	SAI_Dispell_Sight			= 14,	// Dispells and Spells like Bind Sight
	SAI_Stun					= 15,
	SAI_Haste_Runspeed			= 16,	// Haste and SoW
	SAI_Combat_Slow				= 17,
	SAI_Damage_Shield			= 18,
	SAI_Cannibalize_Weapon_Proc	= 19,
	SAI_Weaken					= 20,
	SAI_Banish					= 21,
	SAI_Blind_Poison			= 22,
	SAI_Cold_DD					= 23,
	SAI_Poison_Disease_DD		= 24,
	SAI_Fire_DD					= 25,
	SAI_Memory_Blur				= 27,
	SAI_Gravity_Fling			= 28,
	SAI_Suffocate				= 29,
	SAI_Lifetap_Over_Time		= 30,
	SAI_Fire_AE					= 31,
	SAI_Cold_AE					= 33,
	SAI_Poison_Disease_AE		= 34,
	SAI_Teleport				= 40,
	SAI_Direct_Damage_Song		= 41,
	SAI_Combat_Buff_Song		= 42,
	SAI_Calm_Song				= 43,	// Lull and Alliance Songs
	SAI_Firework				= 45,
	SAI_Firework_AE				= 46,
	SAI_Weather_Rocket			= 47,
	SAI_Convert_Vitals			= 50,
	SAI_NPC_Special_60			= 60,
	SAI_NPC_Special_61			= 61,
	SAI_NPC_Special_62			= 62,
	SAI_NPC_Special_63			= 63,
	SAI_NPC_Special_70			= 70,
	SAI_NPC_Special_71			= 71,
	SAI_NPC_Special_80			= 80,
	SAI_Trap_Lock				= 88
};

enum class GlobalGroup {
	Lich = 46,
};

enum RESISTTYPE
{
	RESIST_NONE = 0,
	RESIST_MAGIC = 1,
	RESIST_FIRE = 2,
	RESIST_COLD = 3,
	RESIST_POISON = 4,
	RESIST_DISEASE = 5,
	RESIST_CHROMATIC = 6,
	RESIST_PRISMATIC = 7,
	RESIST_PHYSICAL = 8,	// see Muscle Shock, Back Swing
	RESIST_CORRUPTION = 9
};

//Target Type IDs
typedef enum {
/* 01 */	ST_TargetOptional = 0x01, //only used for targeted projectile spells
/* 02 */	ST_AEClientV1 = 0x02,
/* 03 */	ST_GroupTeleport = 0x03,
/* 04 */	ST_AECaster = 0x04,
/* 05 */	ST_Target = 0x05,
/* 06 */	ST_Self = 0x06,
/* 07 */	// NOT USED
/* 08 */	ST_AETarget = 0x08,
/* 09 */	ST_Animal = 0x09,
/* 10 */	ST_Undead = 0x0a,
/* 11 */	ST_Summoned = 0x0b,
/* 12 */	// NOT USED error is 218 (This spell only works on things that are flying.)
/* 13 */	ST_Tap = 0x0d,
/* 14 */	ST_Pet = 0x0e,
/* 15 */	ST_Corpse = 0x0f,
/* 16 */	ST_Plant = 0x10,
/* 17 */	ST_Giant = 0x11, //special giant
/* 18 */	ST_Dragon = 0x12, //special dragon
/* 19 */	// NOT USED error is 227 (This spell only works on specific coldain.)
/* 20 */	ST_TargetAETap = 0x14,
/* 21 */	// NOT USED same switch case as ST_Undead
/* 22 */	// NOT USED same switch case as ST_Summoned
/* 23 */	// NOT USED same switch case as ST_Animal
/* 24 */	ST_UndeadAE = 0x18,
/* 25 */	ST_SummonedAE = 0x19,
/* 26 */	// NOT USED
/* 27 */	// NOT USED error is 223 (This spell only works on insects.)
/* 28 */	// NOT USED error is 223 (This spell only works on insects.)
/* 29 */	// NOT USED
/* 30 */	// NOT USED
/* 31 */	// NOT USED
/* 32 */	ST_AETargetHateList = 0x20,
/* 33 */	ST_HateList = 0x21,
/* 34 */	ST_LDoNChest_Cursed = 0x22,
/* 35 */	ST_Muramite = 0x23, //only works on special muramites
/* 36 */	ST_AreaClientOnly = 0x24,
/* 37 */	ST_AreaNPCOnly = 0x25,
/* 38 */	ST_SummonedPet = 0x26,
/* 39 */	ST_GroupNoPets = 0x27,
/* 40 */	ST_AEBard = 0x28,
/* 41 */	ST_Group = 0x29,
/* 42 */	ST_Directional = 0x2a, //ae around this target between two angles
/* 43 */	ST_GroupClientAndPet = 0x2b,
/* 44 */	ST_Beam = 0x2c,
/* 45 */	ST_Ring = 0x2d,
/* 46 */	ST_TargetsTarget = 0x2e, // uses the target of your target
/* 47 */	ST_PetMaster = 0x2f, // uses the master as target
/* 48 */	// UNKNOWN
/* 49 */	// NOT USED
/* 50 */	ST_TargetAENoPlayersPets = 0x32,
} SpellTargetType;

typedef enum {
	DS_DECAY = 244,
	DS_CHILLED = 245,
	DS_FREEZING = 246,
	DS_TORMENT = 247,
	DS_BURN = 248,
	DS_THORNS = 249
} DmgShieldType;

//Spell Effect IDs
// https://web.archive.org/web/20250816011656/https://forums.everquest.com/index.php?threads/enumerated-spa-list.206288/
// mirror: http://pastebin.com/MYeQqGwe

namespace SpellEffect {
	constexpr int CurrentHP                       = 0; // implemented - Heals and nukes, repeates every tic if in a buff
	constexpr int ArmorClass                      = 1; // implemented
	constexpr int ATK                             = 2; // implemented
	constexpr int MovementSpeed                   = 3; // implemented - SoW, SoC, etc
	constexpr int STR                             = 4; // implemented
	constexpr int DEX                             = 5; // implemented
	constexpr int AGI                             = 6; // implemented
	constexpr int STA                             = 7; // implemented
	constexpr int INT                             = 8; // implemented
	constexpr int WIS                             = 9; // implemented
	constexpr int CHA                             = 10; // implemented - used as a spacer
	constexpr int AttackSpeed                     = 11; // implemented
	constexpr int Invisibility                    = 12; // implemented - TO DO: Implemented Invisiblity Levels
	constexpr int SeeInvis                        = 13; // implemented - TO DO: Implemented See Invisiblity Levels
	constexpr int WaterBreathing                  = 14; // implemented
	constexpr int CurrentMana                     = 15; // implemented
	constexpr int NPCFrenzy                       = 16; // not used
	constexpr int NPCAwareness                    = 17; // not used
	constexpr int Lull                            = 18; // implemented - Reaction Radius
	constexpr int AddFaction                      = 19; // implemented - Alliance line
	constexpr int Blind                           = 20; // implemented
	constexpr int Stun                            = 21; // implemented
	constexpr int Charm                           = 22; // implemented
	constexpr int Fear                            = 23; // implemented
	constexpr int Stamina                         = 24; // implemented - Invigor and such
	constexpr int BindAffinity                    = 25; // implemented - TO DO: Implement 2nd and 3rd Recall (value 2,3 ect). Sets additional bind points.
	constexpr int Gate                            = 26; // implemented - Gate to bind point
	constexpr int CancelMagic                     = 27; // implemented
	constexpr int InvisVsUndead                   = 28; // implemented
	constexpr int InvisVsAnimals                  = 29; // implemented
	constexpr int ChangeFrenzyRad                 = 30; // implemented - Pacify
	constexpr int Mez                             = 31; // implemented
	constexpr int SummonItem                      = 32; // implemented
	constexpr int SummonPet                       = 33; // implemented
	constexpr int Confuse                         = 34; // not used (Nimbus of Temporal Rifting) ?
	constexpr int DiseaseCounter                  = 35; // implemented
	constexpr int PoisonCounter                   = 36; // implemented
	constexpr int DetectHostile                   = 37; // not used
	constexpr int DetectMagic                     = 38; // not used
	constexpr int TwinCastBlocker                 = 39; // implemented - If present in spell, then the spell can not be twincast.
	constexpr int DivineAura                      = 40; // implemented
	constexpr int Destroy                         = 41; // implemented - Disintegrate, Banishment of Shadows
	constexpr int ShadowStep                      = 42; // implemented
	constexpr int Berserk                         = 43; // implemented (*not used in any known live spell) Makes client 'Berserk' giving crip blow chance.
	constexpr int Lycanthropy                     = 44; // implemented
	constexpr int Vampirism                       = 45; // implemented (*not used in any known live spell) Stackable lifetap from melee.
	constexpr int ResistFire                      = 46; // implemented
	constexpr int ResistCold                      = 47; // implemented
	constexpr int ResistPoison                    = 48; // implemented
	constexpr int ResistDisease                   = 49; // implemented
	constexpr int ResistMagic                     = 50; // implemented
	constexpr int DetectTraps                     = 51; // not used
	constexpr int SenseDead                       = 52; // implemented
	constexpr int SenseSummoned                   = 53; // implemented
	constexpr int SenseAnimals                    = 54; // implemented
	constexpr int Rune                            = 55; // implemented
	constexpr int TrueNorth                       = 56; // implemented
	constexpr int Levitate                        = 57; // implemented
	constexpr int Illusion                        = 58; // implemented
	constexpr int DamageShield                    = 59; // implemented
	constexpr int TransferItem                    = 60; // not used
	constexpr int Identify                        = 61; // implemented
	constexpr int ItemID                          = 62; // not used
	constexpr int WipeHateList                    = 63; // implemented, @Memblur, chance to wipe hate list of target, base: pct chance, limit: none, max: ? (not implemented), Note: caster level and CHA add to pct chance
	constexpr int SpinTarget                      = 64; // implemented - TO DO: Not sure stun portion is working correctly
	constexpr int InfraVision                     = 65; // implemented
	constexpr int UltraVision                     = 66; // implemented
	constexpr int EyeOfZomm                       = 67; // implemented
	constexpr int ReclaimPet                      = 68; // implemented
	constexpr int TotalHP                         = 69; // implemented
	constexpr int CorpseBomb                      = 70; // not used
	constexpr int NecPet                          = 71; // implemented
	constexpr int PreserveCorpse                  = 72; // not used
	constexpr int BindSight                       = 73; // implemented, @Vision, see through the eyes of your target, click off buff to end effect, base: 1, limit: none, max: none
	constexpr int FeignDeath                      = 74; // implemented
	constexpr int VoiceGraft                      = 75; // implemented
	constexpr int Sentinel                        = 76; // *not implemented?(just seems to send a message)
	constexpr int LocateCorpse                    = 77; // implemented
	constexpr int AbsorbMagicAtt                  = 78; // implemented - Rune for spells
	constexpr int CurrentHPOnce                   = 79; // implemented - Heals and nukes, non-repeating if in a buff
	constexpr int EnchantLight                    = 80; // not used
	constexpr int Revive                          = 81; // implemented - Resurrect
	constexpr int SummonPC                        = 82; // implemented
	constexpr int Teleport                        = 83; // implemented
	constexpr int TossUp                          = 84; // implemented - Gravity Flux
	constexpr int WeaponProc                      = 85; // implemented - i.e. Call of Fire
	constexpr int Harmony                         = 86; // implemented
	constexpr int MagnifyVision                   = 87; // implemented - Telescope
	constexpr int Succor                          = 88; // implemented - Evacuate/Succor lines
	constexpr int ModelSize                       = 89; // implemented - Shrink, Growth
	constexpr int Cloak                           = 90; // *not implemented - Used in only 2 spells
	constexpr int SummonCorpse                    = 91; // implemented
	constexpr int InstantHate                     = 92; // implemented - add hate
	constexpr int StopRain                        = 93; // implemented - Wake of Karana
	constexpr int NegateIfCombat                  = 94; // implemented
	constexpr int Sacrifice                       = 95; // implemented
	constexpr int Silence                         = 96; // implemented
	constexpr int ManaPool                        = 97; // implemented
	constexpr int AttackSpeed2                    = 98; // implemented - Melody of Ervaj
	constexpr int Root                            = 99; // implemented
	constexpr int HealOverTime                    = 100; // implemented
	constexpr int CompleteHeal                    = 101; // implemented
	constexpr int Fearless                        = 102; // implemented - Valiant Companion
	constexpr int CallPet                         = 103; // implemented - Summon Companion
	constexpr int Translocate                     = 104; // implemented
	constexpr int AntiGate                        = 105; // implemented - Translocational Anchor
	constexpr int SummonBSTPet                    = 106; // implemented
	constexpr int AlterNPCLevel                   = 107; // implemented - not used on live
	constexpr int Familiar                        = 108; // implemented
	constexpr int SummonItemIntoBag               = 109; // implemented - summons stuff into container
	constexpr int IncreaseArchery                 = 110; // implemented
	constexpr int ResistAll                       = 111; // implemented - Note: Physical Resists are not modified by this effect.
	constexpr int CastingLevel                    = 112; // implemented
	constexpr int SummonHorse                     = 113; // implemented
	constexpr int ChangeAggro                     = 114; // implemented - Hate modifing buffs(ie horrifying visage)
	constexpr int Hunger                          = 115; // implemented - Song of Sustenance
	constexpr int CurseCounter                    = 116; // implemented
	constexpr int MagicWeapon                     = 117; // implemented - makes weapon magical
	constexpr int Amplification                   = 118; // implemented, @Song, stackable singing mod, base: mod%, limit: none, max: none, Note: Can focus itself.
	constexpr int AttackSpeed3                    = 119; // implemented
	constexpr int HealRate                        = 120; // implemented - reduces healing by a %
	constexpr int ReverseDS                       = 121; // implemented
	constexpr int ReduceSkill                     = 122; // implemented - base: skill id, limit: none, max: none, formula: % skill is reduced (positive)
	constexpr int Screech                         = 123; // implemented Spell Blocker(If have buff with value +1 will block any effect with -1)
	constexpr int ImprovedDamage                  = 124; // implemented
	constexpr int ImprovedHeal                    = 125; // implemented
	constexpr int SpellResistReduction            = 126; // implemented
	constexpr int IncreaseSpellHaste              = 127; // implemented, @Fc, On Caster, cast time mod pct, base: pct
	constexpr int IncreaseSpellDuration           = 128; // implemented, @Fc, On Caster, spell duration mod pct, base: pct
	constexpr int IncreaseRange                   = 129; // implemented, @Fc, On Caster, spell range mod pct, base: pct
	constexpr int SpellHateMod                    = 130; // implemented, @Fc, On Caster, spell hate mod pct, base: min pct, limit: max pct
	constexpr int ReduceReagentCost               = 131; // implemented, @Fc, On Caster, do not consume reagent pct chance, base: min pct, limit: max pct
	constexpr int ReduceManaCost                  = 132; // implemented, @Fc, On Caster, reduce mana cost by pct, base: min pct, limt: max pct
	constexpr int FcStunTimeMod                   = 133; // implemented, @Fc, On Caster, spell range mod pct, base: pct
	constexpr int LimitMaxLevel                   = 134; // implemented, @Ff, Max level of spell that can be focused, if base2 then decrease effectiviness by base2 % per level over max, base: lv, base2: effectiveness pct
	constexpr int LimitResist                     = 135; // implemented, @Ff, Resist Type(s) that a spell focus can require or exclude, base1: resist type, Include: Positive Exclude: Negative
	constexpr int LimitTarget                     = 136; // implemented, @Ff, Target Type(s) that a spell focus can require or exclude, base1: target type, Include: Positive Exclude: Negative
	constexpr int LimitEffect                     = 137; // implemented, @Ff, Spell effect(s) that a spell focus can require or exclude, base1: SPA id, Include: Positive Exclude: Negative
	constexpr int LimitSpellType                  = 138; // implemented, @Ff, Only allow focus spells that are Beneficial or Detrimental, base1: 0=det 1=bene
	constexpr int LimitSpell                      = 139; // implemented, @Ff, Specific spell id(s) that a spell focus can require or exclude, base1: SPA id, Include: Positive Exclude: Negative
	constexpr int LimitMinDur                     = 140; // implemented, @Ff, Mininum duration of spell that can be focused, base1: tics
	constexpr int LimitInstant                    = 141; // implemented, @Ff, Include or exclude if an isntant cast spell can be focused, base1: 0=Exclude if Instant 1=Allow only if Instant
	constexpr int LimitMinLevel                   = 142; // implemented, @Ff, Mininum level of spell that can be focused, base1: lv
	constexpr int LimitCastTimeMin                = 143; // implemented, @Ff, Mininum cast time of spell that can be focused, base1: milliseconds
	constexpr int LimitCastTimeMax                = 144; // implemented, @Ff, Max cast time of spell that can be focused, base1: milliseconds
	constexpr int Teleport2                       = 145; // implemented - Banishment of the Pantheon
	constexpr int ElectricityResist               = 146; // *not implemented TODO: Now used on live, xyz for teleport spells? also in temp pets?
	constexpr int PercentalHeal                   = 147; // implemented
	constexpr int StackingCommand_Block           = 148; // implemented?
	constexpr int StackingCommand_Overwrite       = 149; // implemented?
	constexpr int DeathSave                       = 150; // implemented
	constexpr int SuspendPet                      = 151; // implemented, @Pet, allow caster to have an extra suspended pet, base: 0=no buffs/items 1=buffs+items, limit: none, max: none
	constexpr int TemporaryPets                   = 152; // implemented
	constexpr int BalanceHP                       = 153; // implemented
	constexpr int DispelDetrimental               = 154; // implemented, @Dispel, removes only detrimental effects on a target, base: pct chance (950=95%), limit: none, max: none
	constexpr int SpellCritDmgIncrease            = 155; // implemented - no known live spells use this currently
	constexpr int IllusionCopy                    = 156; // implemented - Deception
	constexpr int SpellDamageShield               = 157; // implemented, @DS, causes non-melee damage on caster of a spell, base: Amt DS (negative), limit: none, max: unknown (same as base but +)
	constexpr int Reflect                         = 158; // implemented, @SpellMisc, reflect casted detrimental spell back at caster, base: chance pct, limit: resist modifier (positive value reduces resists), max: pct of base dmg mod (50=50pct of base)
	constexpr int AllStats                        = 159; // implemented
	constexpr int MakeDrunk                       = 160; // *not implemented - Effect works entirely client side (Should check against tolerance)
	constexpr int MitigateSpellDamage             = 161; // implemented, @Runes, mitigate incoming spell damage by percentage until rune fades, base: percent mitigation, limit: max dmg absorbed per hit, max: rune amt, Note: If placed on item or AA, will provide stackable percent mitigation.
	constexpr int MitigateMeleeDamage             = 162; // implemented - rune with max value
	constexpr int NegateAttacks                   = 163; // implemented
	constexpr int AppraiseLDonChest               = 164; // implemented
	constexpr int DisarmLDoNTrap                  = 165; // implemented
	constexpr int UnlockLDoNChest                 = 166; // implemented
	constexpr int PetPowerIncrease                = 167; // implemented, @Fc, On Caster, pet power mod, base: value
	constexpr int MeleeMitigation                 = 168; // implemented
	constexpr int CriticalHitChance               = 169; // implemented
	constexpr int SpellCritChance                 = 170; // implemented
	constexpr int CrippBlowChance                 = 171; // implemented
	constexpr int AvoidMeleeChance                = 172; // implemented
	constexpr int RiposteChance                   = 173; // implemented
	constexpr int DodgeChance                     = 174; // implemented
	constexpr int ParryChance                     = 175; // implemented
	constexpr int DualWieldChance                 = 176; // implemented
	constexpr int DoubleAttackChance              = 177; // implemented
	constexpr int MeleeLifetap                    = 178; // implemented
	constexpr int AllInstrumentMod                = 179; // implemented, @Song, set mod for ALL instrument/singing skills that will be used if higher then item mods, base: mod%, limit: none, max: none
	constexpr int ResistSpellChance               = 180; // implemented
	constexpr int ResistFearChance                = 181; // implemented
	constexpr int HundredHands                    = 182; // implemented
	constexpr int MeleeSkillCheck                 = 183; // implemented
	constexpr int HitChance                       = 184; // implemented
	constexpr int DamageModifier                  = 185; // implemented
	constexpr int MinDamageModifier               = 186; // implemented
	constexpr int BalanceMana                     = 187; // implemented - Balances party mana
	constexpr int IncreaseBlockChance             = 188; // implemented
	constexpr int CurrentEndurance                = 189; // implemented
	constexpr int EndurancePool                   = 190; // implemented
	constexpr int Amnesia                         = 191; // implemented - Silence vs Melee Effect
	constexpr int Hate                            = 192; // implemented - Instant and hate over time.
	constexpr int SkillAttack                     = 193; // implemented,
	constexpr int FadingMemories                  = 194; // implemented, @Aggro, Remove from hate lists and make invisible. Can set max level of NPCs that can be affected. base: success chance, limit: max level (ROF2), max: max level (modern client), Note: Support for max level requires Rule (Spells, UseFadingMemoriesMaxLevel) to be true. If used from limit field, then it set as the level, ie. max level of 75 would use limit value of 75. If set from max field, max level 75 would use max value of 1075, if you want to set it so it checks a level range above the spell target then for it to only work on mobs 5 levels or below you set max value to 5.
	constexpr int StunResist                      = 195; // implemented
	constexpr int StrikeThrough                   = 196; // implemented
	constexpr int SkillDamageTaken                = 197; // implemented
	constexpr int CurrentEnduranceOnce            = 198; // implemented
	constexpr int Taunt                           = 199; // implemented - % chance to taunt the target
	constexpr int ProcChance                      = 200; // implemented
	constexpr int RangedProc                      = 201; // implemented
	constexpr int IllusionOther                   = 202; // implemented - Project Illusion
	constexpr int MassGroupBuff                   = 203; // implemented
	constexpr int GroupFearImmunity               = 204; // implemented - (Does not use bonus)
	constexpr int Rampage                         = 205; // implemented, @Combat Instant, Perform a primary slot combat rounds on all creatures within a 40 foot radius, base: number of attack rounds, limit: max entities hit per round, max: none, Note: AE range is 40 by default. Custom: Set field 'aoe_range' to override default. Adding additional attacks and hit count limit.
	constexpr int AETaunt                         = 206; // implemented
	constexpr int FleshToBone                     = 207; // implemented
	constexpr int PurgePoison                     = 208; // not used
	constexpr int DispelBeneficial                = 209; // implemented, @Dispel, removes only beneficial effects on a target, base: pct chance (950=95%), limit: none, max: none
	constexpr int PetShield                       = 210; // implmented, @ShieldAbility, allows pet to 'shield' owner for 50 pct of damage taken for a duration, base: Time multiplier 1=12 seconds, 2=24 ect, limit: mitigation on pet owner override (not on live), max: mitigation on pet overide (not on live)
	constexpr int AEMelee                         = 211; // implemented TO DO: Implement to allow NPC use (client only atm).
	constexpr int FrenziedDevastation             = 212; // implemented - increase spell criticals + all DD spells cast 2x mana.
	constexpr int PetMaxHP                        = 213; // implemented[AA] - increases the maximum hit points of your pet
	constexpr int MaxHPChange                     = 214; // implemented
	constexpr int PetAvoidance                    = 215; // implemented[AA] - increases pet ability to avoid melee damage
	constexpr int Accuracy                        = 216; // implemented
	constexpr int HeadShot                        = 217; // implemented - ability to head shot (base2 = damage)
	constexpr int PetCriticalHit                  = 218; // implemented[AA] - gives pets a baseline critical hit chance
	constexpr int SlayUndead                      = 219; // implemented - Allow extra damage against undead (base1 = rate, base2 = damage mod).
	constexpr int SkillDamageAmount               = 220; // implemented
	constexpr int Packrat                         = 221; // implemented as bonus
	constexpr int BlockBehind                     = 222; // implemented - Chance to block from behind (with our without Shield)
	constexpr int DoubleRiposte                   = 223; // implemented - Chance to double riposte [not used on live]
	constexpr int GiveDoubleRiposte               = 224; // implemented[AA]
	constexpr int GiveDoubleAttack                = 225; // implemented[AA] - Allow any class to double attack with set chance.
	constexpr int TwoHandBash                     = 226; // *not implemented as bonus
	constexpr int ReduceSkillTimer                = 227; // implemented
	constexpr int ReduceFallDamage                = 228; // implented - reduce the damage that you take from falling
	constexpr int PersistantCasting               = 229; // implemented
	constexpr int ExtendedShielding               = 230; // implemented, @ShieldAbility, extends the range of your /shield ability by an amount of distance, base: distance units, limit: none, max: none
	constexpr int StunBashChance                  = 231; // implemented - increase chance to stun from bash.
	constexpr int DivineSave                      = 232; // implemented (base1 == % chance on death to insta-res) (base2 == spell cast on save)
	constexpr int Metabolism                      = 233; // implemented - Modifies food/drink consumption rates.
	constexpr int ReduceApplyPoisonTime           = 234; // not implemented as bonus - reduces the time to apply poison
	constexpr int ChannelChanceSpells             = 235; // implemented[AA] - chance to channel from SPELLS *No longer used on live.
	constexpr int FreePet                         = 236; // not used
	constexpr int GivePetGroupTarget              = 237; // implemented[AA] - (Pet Affinity)
	constexpr int IllusionPersistence             = 238; // implemented - lends persistence to your illusionary disguises, causing them to last until you die or the illusion is forcibly removed.
	constexpr int FeignedCastOnChance             = 239; // implemented - ability gives you an increasing chance for your feigned deaths to not be revealed by spells cast upon you.
	constexpr int StringUnbreakable               = 240; // not used [Likely related to above - you become immune to feign breaking on a resisted spell and have a good chance of feigning through a spell that successfully lands upon you.]
	constexpr int ImprovedReclaimEnergy           = 241; // implemented - increase the amount of mana returned to you when reclaiming your pet.
	constexpr int IncreaseChanceMemwipe           = 242; // implemented - @Memblur, increases the chance to wipe hate with memory blurr, base: chance pct, limit: none, max: none, Note: Mods final blur chance after other bonuses added.
	constexpr int CharmBreakChance                = 243; // implemented - Total Domination
	constexpr int RootBreakChance                 = 244; // implemented[AA] reduce the chance that your root will break.
	constexpr int TrapCircumvention               = 245; // implemented, @Traps, decreases the chance that you will set off a trap when opening a chest or other similar container by percentage, base: chance modifer, limit: none, max: none
	constexpr int SetBreathLevel                  = 246; // *not implemented as bonus
	constexpr int RaiseSkillCap                   = 247; // implemented[AA] - adds skill over the skill cap.
	constexpr int SecondaryForte                  = 248; // not implemented as bonus(gives you a 2nd specialize skill that can go past 50 to 100)
	constexpr int SecondaryDmgInc                 = 249; // implemented[AA] Allows off hand weapon to recieve a damage bonus (Sinister Strikes)
	constexpr int SpellProcChance                 = 250; // implemented - Increase chance to proc from melee proc spells (ie Spirit of Panther)
	constexpr int ConsumeProjectile               = 251; // implemented[AA] - chance to not consume an arrow (ConsumeProjectile = 100)
	constexpr int FrontalBackstabChance           = 252; // implemented[AA] - chance to perform a full damage backstab from front.
	constexpr int FrontalBackstabMinDmg           = 253; // implemented[AA] - allow a frontal backstab for mininum damage.
	constexpr int Blank                           = 254; // implemented
	constexpr int ShieldDuration                  = 255; // implemented, , @ShieldAbility, extends the duration of your /shield ability, base: seconds, limit: none, max: none
	constexpr int ShroudofStealth                 = 256; // implemented
	constexpr int PetDiscipline                   = 257; // not implemented as bonus - /pet hold - official name is GivePetHold
	constexpr int TripleBackstab                  = 258; // implemented[AA] - chance to perform a triple backstab
	constexpr int CombatStability                 = 259; // implemented[AA] - damage mitigation
	constexpr int AddSingingMod                   = 260; // implemented, @Song, set mod for specific instrument/singing skills that will be used if higher then item mods, base: mod%, limit: ItemType ID, max: none
	constexpr int SongModCap                      = 261; // implemented, @Song, raise max song modifier cap, base: amt, limit: none, max: none, Note: No longer used on live
	constexpr int RaiseStatCap                    = 262; // implemented
	constexpr int TradeSkillMastery               = 263; // implemented - lets you raise more than one tradeskill above master.
	constexpr int HastenedAASkill                 = 264; // implemented
	constexpr int MasteryofPast                   = 265; // implemented[AA] - Spells less than effect values level can not be fizzled
	constexpr int ExtraAttackChance               = 266; // implemented, @OffBonus, gives your double attacks a percent chance to perform an extra attack with 2-handed primary weapon, base: chance, limit: amt attacks, max: none
	constexpr int AddPetCommand                   = 267; // implemented - sets command base2 to base1
	constexpr int ReduceTradeskillFail            = 268; // implemented - reduces chance to fail with given tradeskill by a percent chance
	constexpr int MaxBindWound                    = 269; // implemented[AA] - Increase max HP you can bind wound.
	constexpr int BardSongRange                   = 270; // implemented, @Song, increase range of beneficial bard songs, base: mod%, limit: none, max: none , Note: example Sionachie's Crescendo
	constexpr int BaseMovementSpeed               = 271; // implemented[AA] - mods basemove speed, doesn't stack with other move mods
	constexpr int CastingLevel2                   = 272; // implemented
	constexpr int CriticalDoTChance               = 273; // implemented
	constexpr int CriticalHealChance              = 274; // implemented
	constexpr int CriticalMend                    = 275; // implemented[AA] - chance to critical monk mend
	constexpr int Ambidexterity                   = 276; // implemented[AA] - increase chance to duel weild by adding bonus 'skill'
	constexpr int UnfailingDivinity               = 277; // implemented[AA] - ability grants your Death Pact-type spells a second chance to successfully heal their target, also can cause said spells to do a portion of their healing value even on a complete failure.
	constexpr int FinishingBlow                   = 278; // implemented[AA] - chance to do massive damage under 10% HP (base1 = chance, base2 = damage)
	constexpr int Flurry                          = 279; // implemented
	constexpr int PetFlurry                       = 280; // implemented[AA]
	constexpr int FeignedMinion                   = 281; // implemented, ability allows you to instruct your pet to feign death via the '/pet feign' command, base: succeed chance, limit: none, max: none, Note: Only implemented as an AA.
	constexpr int ImprovedBindWound               = 282; // implemented[AA] - increase bind wound amount by percent.
	constexpr int DoubleSpecialAttack             = 283; // implemented[AA] - Chance to perform second special attack as monk
	constexpr int LoHSetHeal                      = 284; // not used
	constexpr int NimbleEvasion                   = 285; // *not implemented - base1 = 100 for max
	constexpr int FcDamageAmt                     = 286; // implemented, @Fc, On Caster, spell damage mod flat amt, base: amt
	constexpr int SpellDurationIncByTic           = 287; // implemented, @Fc, SPA: 287, SpellEffect::SpellDurationIncByTic, = On Caster, spell buff duration mod, base: tics
	constexpr int SkillAttackProc                 = 288; // implemented, @Procs, chance to cast a spell when using a skill, base: chance, limit: skill, max: spellid, note: if used in AA the spell id is set in aa_ranks spell field, chance is calculated as 100% = value 1000.
	constexpr int CastOnFadeEffect                = 289; // implemented - Triggers only if fades after natural duration.
	constexpr int IncreaseRunSpeedCap             = 290; // implemented[AA] - increases run speed over the hard cap
	constexpr int Purify                          = 291; // implemented, @Dispel, remove up specified amount of detiremental spells, base: amt removed, limit: none, max: none, Note: excluding charm, fear, resurrection, and revival sickness
	constexpr int StrikeThrough2                  = 292; // implemented[AA] - increasing chance of bypassing an opponent's special defenses, such as dodge, block, parry, and riposte.
	constexpr int FrontalStunResist               = 293; // implemented[AA] - Reduce chance to be stunned from front. -- live descriptions sounds like this isn't limited to frontal anymore
	constexpr int CriticalSpellChance             = 294; // implemented - increase chance to critical hit and critical damage modifier.
	constexpr int ReduceTimerSpecial              = 295; // not used
	constexpr int FcSpellVulnerability            = 296; // implemented, @Fc, On Target, spell damage taken mod pct, base: min pct, limit: max pct
	constexpr int FcDamageAmtIncoming             = 297; // implemetned, @Fc, On Target, damage taken flat amt, base: amt
	constexpr int ChangeHeight                    = 298; // implemented
	constexpr int WakeTheDead                     = 299; // implemented, @Pets, summon one temporary pet from nearby corpses that last a set duration, base: none, limit: none, max: duration (seconds). Note: max range of corpse is 250.
	constexpr int Doppelganger                    = 300; // implemented
	constexpr int ArcheryDamageModifier           = 301; // implemented[AA] - increase archery damage by percent
	constexpr int FcDamagePctCrit                 = 302; // implemented, @Fc, On Caster, spell damage mod pct, base: min pct, limit: max pct, Note: applied after critical hits has been calculated.
	constexpr int FcDamageAmtCrit                 = 303; // implemented, @Fc, On Caster, spell damage mod flat amt, base: amt
	constexpr int OffhandRiposteFail              = 304; // implemented as bonus - enemy cannot riposte offhand attacks
	constexpr int MitigateDamageShield            = 305; // implemented - off hand attacks only (Shielding Resistance)
	constexpr int ArmyOfTheDead                   = 306; // implemented, @Pets, summon multiple temporary pets from nearby corpses that last a set duration, base: amount of corpses that a pet can summon from, limit: none, max: duration (seconds). Note: max range of corpse is 250.
	constexpr int Appraisal                       = 307; // *not implemented Rogue AA - This ability allows you to estimate the selling price of an item you are holding on your cursor.
	constexpr int ZoneSuspendMinion               = 308; // implemented, @Pet, allow suspended pets to be resummoned upon zoning, base: 1, limit: none, max: none, Calc: Bool
	constexpr int GateCastersBindpoint            = 309; // implemented - Gate to casters bind point
	constexpr int ReduceReuseTimer                = 310; // implemented, @Fc, On Caster, spell and disc reuse time mod by amount, base: milliseconds
	constexpr int LimitCombatSkills               = 311; // implemented, @Ff, Include or exclude combat skills or procs from being focused, base1: 0=Exclude if proc 1=Allow only if proc.
	constexpr int Sanctuary                       = 312; // implemented - Places caster at bottom hate list, effect fades if cast cast spell on targets other than self.
	constexpr int ForageAdditionalItems           = 313; // implemented[AA] - chance to forage additional items
	constexpr int Invisibility2                   = 314; // implemented - fixed duration invisible
	constexpr int InvisVsUndead2                  = 315; // implemented - fixed duration ITU
	constexpr int ImprovedInvisAnimals            = 316; // implemented
	constexpr int ItemHPRegenCapIncrease          = 317; // implemented[AA] - increases amount of health regen gained via items
	constexpr int ItemManaRegenCapIncrease        = 318; // implemented - increases amount of mana regen you can gain via items
	constexpr int CriticalHealOverTime            = 319; // implemented
	constexpr int ShieldBlock                     = 320; // implemented - Block attacks with shield
	constexpr int ReduceHate                      = 321; // implemented
	constexpr int GateToHomeCity                  = 322; // implemented
	constexpr int DefensiveProc                   = 323; // implemented
	constexpr int HPToMana                        = 324; // implemented
	constexpr int NoBreakAESneak                  = 325; // implemented[AA] - [AA Nerves of Steel] increasing chance to remain hidden when they are an indirect target of an AoE spell.
	constexpr int SpellSlotIncrease               = 326; // *not implemented as bonus - increases your spell slot availability
	constexpr int MysticalAttune                  = 327; // implemented - increases amount of buffs that a player can have
	constexpr int DelayDeath                      = 328; // implemented - increases how far you can fall below 0 hp before you die
	constexpr int ManaAbsorbPercentDamage         = 329; // implemented
	constexpr int CriticalDamageMob               = 330; // implemented
	constexpr int Salvage                         = 331; // implemented - chance to recover items that would be destroyed in failed tradeskill combine
	constexpr int SummonToCorpse                  = 332; // *not implemented AA - Call of the Wild (Druid/Shaman Res spell with no exp) TOOD: implement this.
	constexpr int CastOnRuneFadeEffect            = 333; // implemented
	constexpr int BardAEDot                       = 334; // implemented
	constexpr int BlockNextSpellFocus             = 335; // implemented, @Fc, On Caster, chance to block next spell, base: chance
	constexpr int IllusionaryTarget               = 336; // not used
	constexpr int PercentXPIncrease               = 337; // implemented
	constexpr int SummonAndResAllCorpses          = 338; // implemented
	constexpr int TriggerOnCast                   = 339; // implemented, @Fc, On Caster, cast on spell use, base: chance pct limit: spellid
	constexpr int SpellTrigger                    = 340; // implemented - chance to trigger spell [Share rolls with 469] All base2 spells share roll chance, only 1 cast.
	constexpr int ItemAttackCapIncrease           = 341; // implemented[AA] - increases the maximum amount of attack you can gain from items.
	constexpr int ImmuneFleeing                   = 342; // implemented - stop mob from fleeing
	constexpr int InterruptCasting                = 343; // implemented - % chance to interrupt spells being cast every tic. Cacophony (8272)
	constexpr int ChannelChanceItems              = 344; // implemented[AA] - chance to not have ITEM effects interrupted when you take damage.
	constexpr int AssassinateLevel                = 345; // implemented as bonus - AA Assisination max level to kill
	constexpr int HeadShotLevel                   = 346; // implemented[AA] - HeadShot max level to kill
	constexpr int DoubleRangedAttack              = 347; // implemented - chance at an additional archery attack (consumes arrow)
	constexpr int LimitManaMin                    = 348; // implemented, @Ff, Mininum mana of spell that can be focused, base1: mana amt
	constexpr int ShieldEquipDmgMod               = 349; // implemented[AA] Increase melee base damage (indirectly increasing hate) when wearing a shield.
	constexpr int ManaBurn                        = 350; // implemented - Drains mana for damage/heal at a defined ratio up to a defined maximum amount of mana.
	constexpr int PersistentEffect                = 351; // *not implemented. creates a trap/totem that casts a spell (spell id + base1?) when anything comes near it. can probably make a beacon for this
	constexpr int IncreaseTrapCount               = 352; // *not implemented - looks to be some type of invulnerability? Test ITC (8755)
	constexpr int AdditionalAura                  = 353; // *not implemented - allows use of more than 1 aura, aa effect
	constexpr int DeactivateAllTraps              = 354; // *not implemented - looks to be some type of invulnerability? Test DAT (8757)
	constexpr int LearnTrap                       = 355; // *not implemented - looks to be some type of invulnerability? Test LT (8758)
	constexpr int ChangeTriggerType               = 356; // not used
	constexpr int FcMute                          = 357; // implemented, @Fc, On Caster, prevents spell casting, base: chance pct
	constexpr int CurrentManaOnce                 = 358; // implemented
	constexpr int PassiveSenseTrap                = 359; // *not implemented - Invulnerability (Brell's Blessing)
	constexpr int ProcOnKillShot                  = 360; // implemented - a buff that has a base1 % to cast spell base2 when you kill a "challenging foe" base3 min level
	constexpr int SpellOnDeath                    = 361; // implemented - casts spell on death of buffed
	constexpr int PotionBeltSlots                 = 362; // *not implemented[AA] 'Quick Draw' expands the potion belt by one additional available item slot per rank.
	constexpr int BandolierSlots                  = 363; // *not implemented[AA] 'Battle Ready' expands the bandolier by one additional save slot per rank.
	constexpr int TripleAttackChance              = 364; // implemented
	constexpr int ProcOnSpellKillShot             = 365; // implemented - chance to trigger a spell on kill when the kill is caused by a specific spell with this effect in it (10470 Venin)
	constexpr int GroupShielding                  = 366; // *not implemented[AA] This gives you /shieldgroup
	constexpr int SetBodyType                     = 367; // implemented - set body type of base1 so it can be affected by spells that are limited to that type (Plant, Animal, Undead, etc)
	constexpr int FactionMod                      = 368; // *not implemented - increases faction with base1 (faction id, live won't match up w/ ours) by base2
	constexpr int CorruptionCounter               = 369; // implemented
	constexpr int ResistCorruption                = 370; // implemented
	constexpr int AttackSpeed4                    = 371; // implemented - stackable slow effect 'Inhibit Melee'
	constexpr int ForageSkill                     = 372; // implemented[AA] Will increase the skill cap for those that have the Forage skill and grant the skill and raise the cap to those that do not.
	constexpr int CastOnFadeEffectAlways          = 373; // implemented - Triggers if fades after natural duration OR from rune/numhits fades.
	constexpr int ApplyEffect                     = 374; // implemented
	constexpr int DotCritDmgIncrease              = 375; // implemented - Increase damage of DoT critical amount
	constexpr int Fling                           = 376; // *not implemented - used in 2 test spells (12945 | Movement Test Spell 1)
	constexpr int CastOnFadeEffectNPC             = 377; // implemented - Triggers only if fades after natural duration (On live these are usually players spells that effect an NPC).
	constexpr int SpellEffectResistChance         = 378; // implemented - Increase chance to resist specific spell effect (base1=value, base2=spell effect id)
	constexpr int ShadowStepDirectional           = 379; // implemented - handled by client
	constexpr int Knockdown                       = 380; // implemented - small knock back(handled by client)
	constexpr int KnockTowardCaster               = 381; // *not implemented (Call of Hither) knocks you back to caster (value) distance units infront
	constexpr int NegateSpellEffect               = 382; // implemented, @Debuff, negates specific spell effect benefits for the duration of the debuff and prevent non-duration spell effect from working, base: see NegateSpellEffecttype Enum, limit: SPA id, max: none
	constexpr int SympatheticProc                 = 383; // implemented, @Fc, On Caster, cast on spell use, base: variable proc chance on cast time, limit: spellid
	constexpr int Leap                            = 384; // implemented - Leap effect, ie stomping leap
	constexpr int LimitSpellGroup                 = 385; // implemented, @Ff, Spell group(s) that a spell focus can require or exclude, base1: spellgroup id, Include: Positive Exclude: Negative
	constexpr int CastOnCurer                     = 386; // implemented - Casts a spell on the person curing
	constexpr int CastOnCure                      = 387; // implemented - Casts a spell on the cured person
	constexpr int SummonCorpseZone                = 388; // implemented - summons a corpse from any zone(nec AA)
	constexpr int FcTimerRefresh                  = 389; // implemented, @Fc, On Caster, reset all recast timers, base: 1, Note: Applied from casted spells only
	constexpr int FcTimerLockout                  = 390; // implemented, @Fc, On Caster, set a spell to be on recast timer, base: recast duration milliseconds, Note: Applied from casted spells only
	constexpr int LimitManaMax                    = 391; // implemented, @Ff, Mininum mana of spell that can be focused, base1: mana amt
	constexpr int FcHealAmt                       = 392; // implemented, @Fc, On Caster, spell healing mod flat amt, base: amt
	constexpr int FcHealPctIncoming               = 393; // implemented, @Fc, On Target, heal received mod pct, base: pct, limit: random max pct
	constexpr int FcHealAmtIncoming               = 394; // implemented, @Fc, On Target, heal received mod flat amt, base: amt
	constexpr int FcHealPctCritIncoming           = 395; // implemented, @Fc, On Target, heal received mod pct, base: pct, limit: random max pct
	constexpr int FcHealAmtCrit                   = 396; // implemented, @Fc, On Caster, spell healing mod flat amt, base: amt
	constexpr int PetMeleeMitigation              = 397; // implemented[AA] - additional mitigation to your pets. Adds AC
	constexpr int SwarmPetDuration                = 398; // implemented - Affects the duration of swarm pets
	constexpr int FcTwincast                      = 399; // implemented - cast 2 spells for every 1
	constexpr int HealGroupFromMana               = 400; // implemented - Drains mana and heals for each point of mana drained
	constexpr int ManaDrainWithDmg                = 401; // implemented - Deals damage based on the amount of mana drained
	constexpr int EndDrainWithDmg                 = 402; // implemented - Deals damage for the amount of endurance drained
	constexpr int LimitSpellClass                 = 403; // implemented, @Ff, 'Spell Category' using table field 'spell_class' that a spell focus can require or exclude, base1: category type, Include: Positive Exclude: Negative
	constexpr int LimitSpellSubclass              = 404; // implemented, @Ff, 'Spell Category Subclass' using table field 'spell_subclass' that a spell focus can require or exclude, base1: category type, Include: Positive Exclude: Negative
	constexpr int TwoHandBluntBlock               = 405; // implemented - chance to block attacks when using two hand blunt weapons (similiar to shield block)
	constexpr int CastonNumHitFade                = 406; // implemented - casts a spell when a buff fades due to its numhits being depleted
	constexpr int CastonFocusEffect               = 407; // implemented - casts a spell if focus limits are met (ie triggers when a focus effects is applied)
	constexpr int LimitHPPercent                  = 408; // implemented - limited to a certain percent of your hp(ie heals up to 50%)
	constexpr int LimitManaPercent                = 409; // implemented - limited to a certain percent of your mana
	constexpr int LimitEndPercent                 = 410; // implemented - limited to a certain percent of your end
	constexpr int LimitClass                      = 411; // implemented, @Ff, Class(es) that can use the spell focus, base1: class(es), Note: The class value in dbase is +1 in relation to item class value, set as you would item for multiple classes
	constexpr int LimitRace                       = 412; // implemented, @Ff, Race that can use the spell focus, base1: race, Note: not used in any known live spells. Use only single race at a time.
	constexpr int FcBaseEffects                   = 413; // implemented, @Fc, On Caster, base spell effectiveness mod pct, base: pct
	constexpr int LimitCastingSkill               = 414; // implemented, @Ff, Spell and singing skills(s) that a spell focus can require or exclude, base1: skill id, Include: Positive Exclude: Negative
	constexpr int FFItemClass                     = 415; // implemented, @Ff, Limits focuses to be applied only from item click. base1: item ItemType (-1 to include for all ItemTypes,-1000 to exclude clicks from getting the focus, or exclude specific SubTypes or Slots if set), limit: item SubType (-1 for all SubTypes), max: item Slots (bitmask of valid slots, -1 ALL slots), Note: not used on live. See comments in Mob::CalcFocusEffect for more details.
	constexpr int ACv2                            = 416; // implemented - New AC spell effect
	constexpr int ManaRegen_v2                    = 417; // implemented - New mana regen effect
	constexpr int SkillDamageAmount2              = 418; // implemented - adds skill damage directly to certain attacks
	constexpr int AddMeleeProc                    = 419; // implemented - Adds a proc
	constexpr int FcLimitUse                      = 420; // implemented, @Fc, On Caster, numhits mod pct, base: pct, Note: not used in any known live spells
	constexpr int FcIncreaseNumHits               = 421; // implemented, @Fc, On Caster, numhits mod flat amt, base: amt
	constexpr int LimitUseMin                     = 422; // implemented, @Ff Minium amount of numhits for a spell to be focused, base: numhit amt
	constexpr int LimitUseType                    = 423; // implemented, = @Ff Focus will only affect if has this numhits type, base: numhit type
	constexpr int GravityEffect                   = 424; // implemented - Pulls/pushes you toward/away the mob at a set pace
	constexpr int Display                         = 425; // *not implemented - Illusion: Flying Dragon(21626)
	constexpr int IncreaseExtTargetWindow         = 426; // *not implmented[AA] - increases the capacity of your extended target window
	constexpr int SkillProcAttempt                = 427; // implemented - chance to proc when using a skill(ie taunt)
	constexpr int LimitToSkill                    = 428; // implemented, @Procs, limits what combat skills will effect a skill proc, base: skill value, limit: none, max: none
	constexpr int SkillProcSuccess                = 429; // implemented - chance to proc when tje skill in use successfully fires.
	constexpr int PostEffect                      = 430; // *not implemented - Fear of the Dark(27641) - Alters vision
	constexpr int PostEffectData                  = 431; // *not implemented - Fear of the Dark(27641) - Alters vision
	constexpr int ExpandMaxActiveTrophyBen        = 432; // not used
	constexpr int CriticalDotDecay                = 433; // implemented - increase critical dot chance, effect decays based on level of spell it effects. (12266)
	constexpr int CriticalHealDecay               = 434; // implemented - increase critical heal chance, effect decays based on level of spell it effects.
	constexpr int CriticalRegenDecay              = 435; // implemented - increase critical heal over time chance, effect decays based on level of spell it effects.
	constexpr int BeneficialCountDownHold         = 436; // not used ( 23491 | ABTest Buff Hold)
	constexpr int TeleporttoAnchor                = 437; // *not implemented - Teleport Guild Hall Anchor(33099)
	constexpr int TranslocatetoAnchor             = 438; // *not implemented - Translocate Primary Anchor (27750)
	constexpr int Assassinate                     = 439; // implemented[AA] - Assassinate damage
	constexpr int FinishingBlowLvl                = 440; // implemented[AA] - Sets the level Finishing blow can be triggered on an NPC
	constexpr int DistanceRemoval                 = 441; // implemented - Buff is removed from target when target moves X amount of distance away from where initially hit.
	constexpr int TriggerOnReqTarget              = 442; // implemented, @SpellTrigger, triggers a spell when Target Requirement conditions are met (see enum SpellRestriction for IDs), base: spellid, limit: SpellRestriction ID, max: none, Note: Usually cast on a target
	constexpr int TriggerOnReqCaster              = 443; // implemented, @SpellTrigger, triggers a spell when Caster Requirement conditions are met (see enum SpellRestriction for IDs), base: spellid, limit: SpellRestriction ID, max: none, Note: Usually self only
	constexpr int ImprovedTaunt                   = 444; // implemented - Locks Aggro On Caster and Decrease other Players Aggro by X% on NPC targets below level Y
	constexpr int AddMercSlot                     = 445; // *not implemented[AA] - [Hero's Barracks] Allows you to conscript additional mercs.
	constexpr int AStacker                        = 446; // implementet - bufff stacking blocker (26219 | Qirik's Watch)
	constexpr int BStacker                        = 447; // implemented
	constexpr int CStacker                        = 448; // implemented
	constexpr int DStacker                        = 449; // implemented
	constexpr int MitigateDotDamage               = 450; // implemented, @Runes, mitigate incoming dot damage by percentage until rune fades, base: percent mitigation, limit: max dmg absorbed per hit, max: rune amt, Note: If placed on item or AA, will provide stackable percent mitigation.
	constexpr int MeleeThresholdGuard             = 451; // implemented Partial Melee Rune that only is lowered if melee hits are over X amount of damage
	constexpr int SpellThresholdGuard             = 452; // implemented Partial Spell Rune that only is lowered if spell hits are over X amount of damage
	constexpr int TriggerMeleeThreshold           = 453; // implemented Trigger effect on X amount of melee damage taken in a single hit
	constexpr int TriggerSpellThreshold           = 454; // implemented Trigger effect on X amount of spell damage taken in a single hit
	constexpr int AddHatePct                      = 455; // implemented Modify total hate by %
	constexpr int AddHateOverTimePct              = 456; // implemented Modify total hate by % over time.
	constexpr int ResourceTap                     = 457; // implemented Coverts a percent of dmg from dmg spells(DD/DoT) to hp/mana/end.
	constexpr int FactionModPct                   = 458; // implemented Modifies faction gains and losses by percent.
	constexpr int DamageModifier2                 = 459; // implemented - Modifies melee damage by skill type
	constexpr int Ff_Override_NotFocusable        = 460; // implemented, @Fc, Allow spell to be focused event if flagged with 'not_focusable' in spell table, base: 1
	constexpr int ImprovedDamage2                 = 461; // implemented, @Fc, On Caster, spell damage mod pct, base: min pct, limit: max pct
	constexpr int FcDamageAmt2                    = 462; // implemented, @Fc, On Caster, spell damage mod flat amt, base: amt
	constexpr int Shield_Target                   = 463; // implemented, Base1 % damage shielded on target
	constexpr int PC_Pet_Rampage                  = 464; // implemented - Base1 % chance to do rampage for base2 % of damage each melee round
	constexpr int PC_Pet_AE_Rampage               = 465; // implemented - Base1 % chance to do AE rampage for base2 % of damage each melee round
	constexpr int PC_Pet_Flurry_Chance            = 466; // implemented - Base1 % chance to do flurry from double attack hit.
	constexpr int DS_Mitigation_Amount            = 467; // implemented - Modify incoming damage shield damage by a flat amount
	constexpr int DS_Mitigation_Percentage        = 468; // implemented - Modify incoming damage shield damage by percentage
	constexpr int Chance_Best_in_Spell_Grp        = 469; // implemented - Chance to cast highest scribed spell within a spell group. All base2 spells share roll chance, only 1 cast.
	constexpr int Trigger_Best_in_Spell_Grp       = 470; // implemented - Chance to cast highest scribed spell within a spell group. Each spell has own chance.
	constexpr int Double_Melee_Round              = 471; // implemented, @OffBonus, percent chance to repeat primary weapon round with a percent damage modifier, base: pct chance repeat, limit: pct dmg mod, max: none
	constexpr int Buy_AA_Rank                     = 472; // implemented, @Special, Used in AA abilities that have Enable/Disable toggle. Spell on Disabled Rank has this effect in it, base: 1, limit: none, max: none, Note: This will not just buy an AA
	constexpr int Double_Backstab_Front           = 473; // implemented - Chance to double backstab from front
	constexpr int Pet_Crit_Melee_Damage_Pct_Owner = 474; // implemenetd - Critical damage mod applied to pets from owner
	constexpr int Trigger_Spell_Non_Item          = 475; // implemented - Trigger spell on cast only if not from item click.
	constexpr int Weapon_Stance                   = 476; // implemented, @Misc, Apply a specific spell buffs automatically depending 2Hander, Shield or Dual Wield is equipped, base: spellid, base: 0=2H 1=Shield 2=DW, max: none
	constexpr int Hatelist_To_Top_Index           = 477; // Implemented - Chance to be set to top of rampage list
	constexpr int Hatelist_To_Tail_Index          = 478; // Implemented - Chance to be set to bottom of rampage list
	constexpr int Ff_Value_Min                    = 479; // implemented, @Ff, Minimum base value of a spell that can be focused, base: spells to be focused base1 value
	constexpr int Ff_Value_Max                    = 480; // implemented, @Ff, Max base value of a spell that can be focused, base: spells to be focused base1 value
	constexpr int Fc_Cast_Spell_On_Land           = 481; // implemented, @Fc, On Target, cast spell if hit by spell, base: chance pct, limit: spellid
	constexpr int Skill_Base_Damage_Mod           = 482; // implemented, @OffBonus, modify base melee damage by percent, base: pct, limit: skill(-1=ALL), max: none
	constexpr int Fc_Spell_Damage_Pct_IncomingPC  = 483; // implemented, @Fc, On Target, spell damage taken mod pct, base: min pct, limit: max pct
	constexpr int Fc_Spell_Damage_Amt_IncomingPC  = 484; // implemented, @Fc, On Target, damage taken flat amt, base: amt
	constexpr int Ff_CasterClass                  = 485; // implemented, @Ff, Caster of spell on target with a focus effect that is checked by incoming spells must be specified class(es). base1: class(es), Note: Set multiple classes same as would for items
	constexpr int Ff_Same_Caster                  = 486; // implemented, @Ff, Caster of spell on target with a focus effect that is checked by incoming spells, base1: 0=Must be different caster 1=Must be same caster
	constexpr int Extend_Tradeskill_Cap           = 487; //
	constexpr int Defender_Melee_Force_Pct_PC     = 488; //
	constexpr int Worn_Endurance_Regen_Cap        = 489; // implemented, modify worn regen cap, base: amt, limit: none, max: none
	constexpr int Ff_ReuseTimeMin                 = 490; // implemented, @Ff, Minimum recast time of a spell that can be focused, base: recast time
	constexpr int Ff_ReuseTimeMax                 = 491; // implemented, @Ff, Max recast time of a spell that can be focused, base: recast time
	constexpr int Ff_Endurance_Min                = 492; // implemented, @Ff, Minimum endurance cost of a spell that can be focused, base: endurance cost
	constexpr int Ff_Endurance_Max                = 493; // implemented, @Ff, Max endurance cost of a spell that can be focused, base: endurance cost
	constexpr int Pet_Add_Atk                     = 494; // implemented - Bonus on pet owner which gives their pet increased attack stat
	constexpr int Ff_DurationMax                  = 495; // implemented, @Ff, Max duration of spell that can be focused, base: tics
	constexpr int Critical_Melee_Damage_Mod_Max   = 496; // implemented - increase or decrease by percent critical damage (not stackable)
	constexpr int Ff_FocusCastProcNoBypass        = 497; //
	constexpr int AddExtraAttackPct_1h_Primary    = 498; // implemented, @OffBonus, gives your double attacks a percent chance to perform an extra attack with 1-handed primary weapon, base: chance, limit: amt attacks, max: none
	constexpr int AddExtraAttackPct_1h_Secondary  = 499; //implemented, @OffBonus, gives your double attacks a percent chance to perform an extra attack with 1-handed secondary weapon, base: chance, limit: amt attacks, max: none
	constexpr int Fc_CastTimeMod2                 = 500; // implemented, @Fc, On Caster, cast time mod pct, base: pct, Note: Can reduce to instant cast
	constexpr int Fc_CastTimeAmt                  = 501; // implemented, @Fc, On Caster, cast time mod flat amt, base: milliseconds, Note: Can reduce to instant cast
	constexpr int Fearstun                        = 502; // implemented - Stun with a max level limit. Normal stun restrictions don't apply.
	constexpr int Melee_Damage_Position_Mod       = 503; // implemented, @OffBonus, modify melee damage by percent if done from Front or Behind opponent, base: pct, limit: 0=back 1=front, max: none
	constexpr int Melee_Damage_Position_Amt       = 504; // implemented, @OffBonus, modify melee damage by flat amount if done from Front or Behind opponent, base: amt, limit: 0=back 1=front, max: none
	constexpr int Damage_Taken_Position_Mod       = 505; // implemented, @DefBonus, modify melee damage by percent if dmg taken from Front or Behind, base: pct, limit: 0=back 1=front, max: none
	constexpr int Damage_Taken_Position_Amt       = 506; // implemented -@DefBonus, modify melee damage by flat amount if dmg taken from your Front or Behind, base: amt, limit: 0=back 1=front, max: none
	constexpr int Fc_Amplify_Mod                  = 507; // implemented, @Fc, On Caster, damage-heal-dot mod pct, base: pct
	constexpr int Fc_Amplify_Amt                  = 508; // implemented, @Fc, On Caster, damage-heal-dot mod flat amt, base: amt
	constexpr int Health_Transfer                 = 509; // implemented - exchange health for damage or healing on a target. ie Lifeburn/Act of Valor
	constexpr int Fc_ResistIncoming               = 510; // implemented, @Fc, On Target, resist modifier, base: amt
	constexpr int Ff_FocusTimerMin                = 511; // implemented, @Ff, sets a recast time until focus can be used again, base: 1, limit: time ms, Note: ie. limit to 1 trigger every 1.5 seconds
	constexpr int Proc_Timer_Modifier             = 512; // implemented - limits procs per amount of a time based on timer value, base: 1, limit: time ms, Note:, ie limit to 1 proc every 55 seconds)
	constexpr int Mana_Max_Percent                = 513; //
	constexpr int Endurance_Max_Percent           = 514; //
	constexpr int AC_Avoidance_Max_Percent        = 515; // implemented - stackable avoidance modifier
	constexpr int AC_Mitigation_Max_Percent       = 516; // implemented - stackable defense modifier
	constexpr int Attack_Offense_Max_Percent      = 517; //
	constexpr int Attack_Accuracy_Max_Percent     = 518; // implemented - stackable accurary modifer
	constexpr int Luck_Amount                     = 519; //
	constexpr int Luck_Percent                    = 520; //
	constexpr int Endurance_Absorb_Pct_Damage     = 521; // implemented - Reduces % of Damage using Endurance, drains endurance at a ratio (ie. 0.05 Endurance per Hit Point)
	constexpr int Instant_Mana_Pct                = 522; // implemented - Increase/Decrease mana by percent of max mana
	constexpr int Instant_Endurance_Pct           = 523; // implemented - Increase/Decrease mana by percent of max endurance
	constexpr int Duration_HP_Pct                 = 524; // implemented - Decrease Current Hit Points by % of Total Hit Points per Tick, up to a MAX per tick
	constexpr int Duration_Mana_Pct               = 525; // implemented - Decrease Current Mana by % of Total Mana per Tick, up to a MAX per tick
	constexpr int Duration_Endurance_Pct          = 526; // implemented - Decrease Current Endurance by % of Total Hit Points per Tick, up to a MAX per tick
}

#define DF_Permanent				50
#define DF_Aura						51
#define PERMANENT_BUFF_DURATION 	-1000 //this is arbitrary used when overriding spells regular buff duration to set it as permenant

// note this struct is historical, we don't actually need it to be
// aligned to anything, but for maintaining it it is kept in the order that
// the fields in the text file are. the numbering is not offset, but field
// number. note that the id field is counted as 0, this way the numbers
// here match the numbers given to sep in the loading function net.cpp
//
#define SPELL_LOAD_FIELD_COUNT 236

struct SPDat_Spell_Struct
{
/* 000 */	int id;	// not used -- SPELLINDEX
/* 001 */	char name[64]; // Name of the spell -- SPELLNAME
/* 002 */	char player_1[32]; // "PLAYER_1" -- ACTORTAG
/* 003 */	char teleport_zone[64];	// Teleport zone, pet name summoned, or item summoned -- NPC_FILENAME
/* 004 */	char you_cast[64]; // Message when you cast -- CASTERMETXT
/* 005 */	char other_casts[64]; // Message when other casts -- CASTEROTHERTXT
/* 006 */	char cast_on_you[64]; // Message when spell is cast on you -- CASTEDMETXT
/* 007 */	char cast_on_other[64]; // Message when spell is cast on someone else -- CASTEDOTHERTXT
/* 008 */	char spell_fades[64]; // Spell fades -- SPELLGONE
/* 009 */	float range; // -- RANGE
/* 010 */	float aoe_range; // -- IMPACTRANGE
/* 011 */	float push_back; // -- OUTFORCE
/* 012 */	float push_up; // -- UPFORCE
/* 013 */	uint32 cast_time; // Cast time -- CASTINGTIME
/* 014 */	uint32 recovery_time; // Recovery time -- RECOVERYDELAY
/* 015 */	uint32 recast_time; // Recast same spell time -- SPELLDELAY
/* 016 */	uint32 buff_duration_formula; // -- DURATIONBASE
/* 017 */	uint32 buff_duration; // -- DURATIONCAP
/* 018 */	uint32 aoe_duration;	// sentinel, rain of something -- IMPACTDURATION
/* 019 */	int32 mana; // Mana Used -- MANACOST
/* 020 */	int base_value[EFFECT_COUNT];	//various purposes -- BASEAFFECT1 .. BASEAFFECT12
/* 032 */	int limit_value[EFFECT_COUNT]; //various purposes -- BASE_EFFECT2_1 ... BASE_EFFECT2_12
/* 044 */	int32 max_value[EFFECT_COUNT]; // -- AFFECT1CAP ... AFFECT12CAP
/* 056 */	//uint16 icon; // Spell icon -- IMAGENUMBER
/* 057 */	//uint16 memicon; // Icon on membarthing -- MEMIMAGENUMBER
/* 058 */	int32 component[4]; // reagents -- EXPENDREAGENT1 ... EXPENDREAGENT4
/* 062 */	int component_count[4]; // amount of regents used -- EXPENDQTY1 ... EXPENDQTY4
/* 066 */	int no_expend_reagent[4];	// focus items (Need but not used; Flame Lick has a Fire Beetle Eye focus.)
											// If it is a number between 1-4 it means components[number] is a focus and not to expend it
											// If it is a valid itemid it means this item is a focus as well
											// -- NOEXPENDREAGENT1 ... NOEXPENDREAGENT4
/* 070 */	uint32 formula[EFFECT_COUNT]; // Spell's value formula -- LEVELAFFECT1MOD ... LEVELAFFECT12MOD
/* 082 */	//int LightType; // probaly another effecttype flag -- LIGHTTYPE
/* 083 */	int8 good_effect; //0=detrimental, 1=Beneficial, 2=Beneficial, Group Only -- BENEFICIAL
/* 084 */	int activated; // probably another effecttype flag -- ACTIVATED
/* 085 */	int resist_type; // -- RESISTTYPE
/* 086 */	int effect_id[EFFECT_COUNT];	// Spell's effects -- SPELLAFFECT1 ... SPELLAFFECT12
/* 098 */	SpellTargetType target_type;	// Spell's Target -- TYPENUMBER
/* 099 */	int base_difficulty; // base difficulty fizzle adjustment -- BASEDIFFICULTY
/* 100 */	EQ::skills::SkillType skill; // -- CASTINGSKILL
/* 101 */	int8 zone_type; // 01=Outdoors, 02=dungeons, ff=Any -- ZONETYPE
/* 102 */	int8 environment_type; // -- ENVIRONMENTTYPE
/* 103 */	int8 time_of_day; // -- TIMEOFDAY
/* 104 */	uint8 classes[Class::PLAYER_CLASS_COUNT]; // Classes, and their min levels -- WARRIORMIN ... BERSERKERMIN
/* 120 */	uint8 casting_animation; // -- CASTINGANIM
/* 121 */	//uint8 TargetAnim; // -- TARGETANIM
/* 122 */	//uint32 TravelType; // -- TRAVELTYPE
/* 123 */	uint16 spell_affect_index; // -- SPELLAFFECTINDEX
/* 124 */	int8 disallow_sit; // 124: high-end Yaulp spells (V, VI, VII, VIII [Rk 1, 2, & 3], & Gallenite's Bark of Fury -- CANCELONSIT
/* 125 */	int8 deity_agnostic;// 125: Words of the Skeptic -- DEITY_AGNOSTIC
/* 126 */	int8 deities[16];	// Deity check. 201 - 216 per http://www.eqemulator.net/wiki/wikka.php?wakka=DeityList
										// -1: Restrict to Deity; 1: Restrict to Deity, but only used on non-Live (Test Server "Blessing of ...") spells; 0: Don't restrict
										// the client actually stores deities in a single int32_t
										// -- DEITY_BERTOXXULOUS ... DEITY_VEESHAN
/* 142 */	//int8 npc_no_cast;			// 142: between 0 & 100 -- NPC_NO_CAST
/* 143 */	//int ai_pt_bonus;			// 143: always set to 0, client doesn't save this -- AI_PT_BONUS
/* 144 */	int16 new_icon;	// Spell icon used by the client in uifiles/default/spells??.tga, both for spell gems & buff window. Looks to depreciate icon & memicon -- NEW_ICON
/* 145 */	//int16 spellanim; // Doesn't look like it's the same as #doanim, so not sure what this is, particles I think -- SPELL_EFFECT_INDEX
/* 146 */	bool uninterruptable;	// Looks like anything != 0 is uninterruptable. Values are mostly -1, 0, & 1 (Fetid Breath = 90?) -- NO_INTERRUPT
/* 147 */	int16 resist_difficulty; // -- RESIST_MOD
/* 148 */	bool unstackable_dot; // -- NOT_STACKABLE_DOT
/* 149 */	//int deletable; // -- DELETE_OK
/* 150 */	uint16 recourse_link; // -- REFLECT_SPELLINDEX
/* 151 */	bool no_partial_resist;	// 151: -1, 0, or 1 -- NO_PARTIAL_SAVE
/* 152 */	//bool small_targets_only; // -- SMALL_TARGETS_ONLY
/* 153 */	//bool uses_persistent_particles; // -- USES_PERSISTENT_PARTICLES
/* 154 */	int8 short_buff_box;	// != 0, goes to short buff box. -- BARD_BUFF_BOX
/* 155 */	int description_id; // eqstr of description of spell -- DESCRIPTION_INDEX
/* 156 */	int type_description_id; // eqstr of type description -- PRIMARY_CATEGORY
/* 157 */	int effect_description_id; // eqstr of effect description -- SECONDARY_CATEGORY_1
/* 158 */	//int secondary_category_2;   //Category Desc ID 3 -- SECONDARY_CATEGORY_2
/* 159 */	bool npc_no_los; // -- NO_NPC_LOS
/* 160 */	bool feedbackable; // -- FEEDBACKABLE
/* 161 */	bool reflectable; // -- REFLECTABLE
/* 162 */	int bonus_hate; // -- HATE_MOD
/* 163 */	//int resist_per_level; // -- RESIST_PER_LEVEL
/* 164 */	//int resist_cap; // for most spells this appears to mimic ResistDiff -- RESIST_CAP
/* 165 */	bool ldon_trap; //Flag found on all LDON trap / chest related spells. -- AFFECT_INANIMATE
/* 166 */	int endurance_cost; // -- STAMINA_COST
/* 167 */	int8 timer_id; // bad name, used for all spells -- TIMER_INDEX
/* 168 */	bool is_discipline; //Will goto the combat window when cast -- IS_SKILL
/* 169 - 172*/ //These are zero for ALL spells, also removed from live -- ATTACK_OPENING, DEFENSE_OPENING, SKILL_OPENING, NPC_ERROR_OPENING
/* 173 */	int hate_added; // -- SPELL_HATE_GIVEN
/* 174 */	int endurance_upkeep; // -- ENDUR_UPKEEP
/* 175 */	int hit_number_type; // defines which type of behavior will tick down the numhit counter. -- LIMITED_USE_TYPE
/* 176 */	int hit_number; // -- LIMITED_USE_COUNT
/* 177 */	int pvp_resist_base; // -- PVP_RESIST_MOD
/* 178 */	int pvp_resist_per_level; // -- PVP_RESIST_PER_LEVEL
/* 179 */	int pvp_resist_cap; // -- PVP_RESIST_CAP
/* 180 */	int spell_category; // -- GLOBAL_GROUP
/* 181 */	int pvp_duration; // buffdurationformula for PvP -- PVP_DURATION
/* 182 */	int pvp_duration_cap; // buffduration for PvP -- PVP_DURATION_CAP
/* 183 */	int pcnpc_only_flag; // valid values are 0, 1 = PCs (and mercs), and 2 = NPCs (and not mercs) -- PCNPC_ONLY_FLAG
/* 184 */	bool cast_not_standing; // this is checked in the client's EQ_Spell::IsCastWhileInvisSpell, this also blocks SpellEffect::InterruptCasting from affecting this spell -- CAST_NOT_STANDING (Allows casting if DA, stun, mezed, charm? fear?, damage to invul targets)
/* 185 */	bool can_mgb; // 0=no, -1 or 1 = yes -- CAN_MGB
/* 186 */	int dispel_flag; // -- NO_DISPELL
/* 187 */	//int npc_category; // -- NPC_MEM_CATEGORY
/* 188 */	//int npc_usefulness; // -- NPC_USEFULNESS
/* 189 */	int min_resist; // -- MIN_RESIST
/* 190 */	int max_resist; // -- MAX_RESIST
/* 191 */	uint8 viral_targets; // -- MIN_SPREAD_TIME
/* 192 */	uint8 viral_timer; // -- MAX_SPREAD_TIME
/* 193 */	int nimbus_effect; // -- DURATION_PARTICLE_EFFECT
/* 194 */	float directional_start; //Cone Start Angle: -- CONE_START_ANGLE
/* 195 */	float directional_end; // Cone End Angle: -- CONE_END_ANGLE
/* 196 */   bool sneak; // effect can only be used if sneaking (rogue 'Daggerfall' ect) -- SNEAK_ATTACK
/* 197 */	bool not_focusable; //prevents focus effects from being applied to spell -- NOT_FOCUSABLE
/* 198 */   bool no_detrimental_spell_aggro; // -- NO_DETRIMENTAL_SPELL_AGGRO
/* 199 */	//bool show_wear_off_message; // -- SHOW_WEAR_OFF_MESSAGE
/* 200 */	bool suspendable; // buff is suspended in suspended buff zones -- IS_COUNTDOWN_HELD
/* 201 */	int viral_range; // -- SPREAD_RADIUS
/* 202 */	int song_cap; // individual song cap -- BASE_EFFECTS_FOCUS_CAP
/* 203 */	//bool stacks_with_self; // -- STACKS_WITH_SELF
/* 204 */	//int not_shown_to_player; // client skips this -- NOT_SHOWN_TO_PLAYER
/* 205 */	bool no_block; // -- NO_BUFF_BLOCK
/* 206 */	//int8 anim_variation; // -- ANIM_VARIATION
/* 207 */	int spell_group; // -- SPELL_GROUP
/* 208 */	int rank; //increments AA effects with same name -- SPELL_GROUP_RANK
/* 209 */	int no_resist; //makes spells unresistable, which makes charms unbreakable as well. -- NO_RESIST
/* 210 */	// bool allow_spellscribe; // -- ALLOW_SPELLSCRIBE
/* 211 */	int cast_restriction; //Various restriction categories for spells most seem targetable race related but have also seen others for instance only castable if target hp 20% or lower or only if target out of combat -- SPELL_REQ_ASSOCIATION_ID
/* 212 */	bool allow_rest; // -- BYPASS_REGEN_CHECK
/* 213 */	bool can_cast_in_combat; //Allow spell if target is in combat -- CAN_CAST_IN_COMBAT
/* 214 */   bool can_cast_out_of_combat; //Allow spell if target is out of combat -- CAN_CAST_OUT_OF_COMBAT
/* 215 */	//bool show_dot_message; // -- SHOW_DOT_MESSAGE
/* 216 */	//bool invalid; // -- INVALID
/* 217 */   int override_crit_chance; //Places a cap on the max chance to critical -- OVERRIDE_CRIT_CHANCE
/* 218 */	int aoe_max_targets;  //Is used for various AE effects -- MAX_TARGETS
/* 219 */	int no_heal_damage_item_mod; // -- NO_HEAL_DAMAGE_ITEM_MOD
/* 220 */	int caster_requirement_id; // -- CASTER_REQUIREMENT_ID
/* 221 */	int spell_class; // -- SPELL_CLASS
/* 222 */	int spell_subclass; // -- SPELL_SUBCLASS
/* 223 */	//int ai_valid_targets; // -- AI_VALID_TARGETS
/* 224 */	bool persist_death; // buff doesn't get stripped on death -- NO_STRIP_ON_DEATH
/* 225 */	//float base_effects_focus_slope; // -- BASE_EFFECTS_FOCUS_SLOPE
/* 226 */	//float base_effects_focus_offset; // -- BASE_EFFECTS_FOCUS_OFFSET
/* 227 */	float min_distance; //spell power modified by distance from caster (Min Distance) -- DISTANCE_MOD_CLOSE_DIST
/* 228 */	float min_distance_mod;  //spell power modified by distance from caster (Modifier at Min Distance) -- DISTANCE_MOD_CLOSE_MULT
/* 229 */	float max_distance; //spell power modified by distance from caster (Max Distance) -- DISTANCE_MOD_FAR_DIST
/* 230 */   float max_distance_mod; //spell power modified by distance from caster (Modifier at Max Distance) -- DISTANCE_MOD_FAR_MULT
/* The client also does this
 *  v26 = *(float *)&v4->DistanceModFarDist - *(float *)&v4->DistanceModCloseDist;
 *  if ( v26 > -0.00000011920929 && v26 < 0.00000011920929 )
 *    v26 = 1.0;
 *  v27 = (st7_0 - *(float *)&v4->DistanceModCloseMult) / v26;
 *  *(float *)&v4->DistanceMod = v27;
 */
/* 231 */   float min_range; //Min casting range -- MIN_RANGE
/* 232 */   bool no_remove; //prevents buff from being removed by click -- NO_REMOVE
/* 233 */	//int spell_recourse_type; // -- SPELL_RECOURSE_TYPE
/* 234 */	//bool only_during_fast_regen; // -- ONLY_DURING_FAST_REGEN
/* 235 */	//bool is_beta_only; // -- IS_BETA_ONLY
/* 236 */	//int spell_subgroup; // -- SPELL_SUBGROUP
			uint8 damage_shield_type; // This field does not exist in spells_us.txt
};

extern const SPDat_Spell_Struct* spells;
extern int32 SPDAT_RECORDS;

bool IsTargetableAESpell(uint16 spell_id);
bool IsSacrificeSpell(uint16 spell_id);
bool IsLifetapSpell(uint16 spell_id);
bool IsMesmerizeSpell(uint16 spell_id);
bool SpellBreaksMez(uint16 spell_id);
bool IsStunSpell(uint16 spell_id);
bool IsSlowSpell(uint16 spell_id);
bool IsHasteSpell(uint16 spell_id);
bool IsHarmonySpell(uint16 spell_id);
bool IsPercentalHealSpell(uint16 spell_id);
bool IsGroupOnlySpell(uint16 spell_id);
bool IsBeneficialSpell(uint16 spell_id);
bool IsDetrimentalSpell(uint16 spell_id);
bool IsInvisibleSpell(uint16 spell_id);
bool IsInvulnerabilitySpell(uint16 spell_id);
bool IsCompleteHealDurationSpell(uint16 spell_id);
bool IsPoisonCounterSpell(uint16 spell_id);
bool IsDiseaseCounterSpell(uint16 spell_id);
bool IsSummonItemSpell(uint16 spell_id);
bool IsSummonSkeletonSpell(uint16 spell_id);
bool IsSummonPetSpell(uint16 spell_id);
bool IsSummonPCSpell(uint16 spell_id);
bool IsPetSpell(uint16 spell_id);
bool IsCharmSpell(uint16 spell_id);
bool IsResurrectionSicknessSpell(uint16 spell_id);
bool IsBlindSpell(uint16 spell_id);
bool IsHealthSpell(uint16 spell_id);
bool IsCastTimeReductionSpell(uint16 spell_id);
bool IsIncreaseDurationSpell(uint16 spell_id);
bool IsManaCostReductionSpell(uint16 spell_id);
bool IsIncreaseRangeSpell(uint16 spell_id);
bool IsImprovedHealingSpell(uint16 spell_id);
bool IsImprovedDamageSpell(uint16 spell_id);
bool IsAEDurationSpell(uint16 spell_id);
bool IsPureNukeSpell(uint16 spell_id);
bool IsAENukeSpell(uint16 spell_id);
bool IsPBAENukeSpell(uint16 spell_id);
bool IsAERainNukeSpell(uint16 spell_id);
bool IsAnyNukeOrStunSpell(uint16 spell_id);
bool IsAnyAESpell(uint16 spell_id);
bool IsAESpell(uint16 spell_id);
bool IsPBAESpell(uint16 spell_id);
bool IsAERainSpell(uint16 spell_id);
bool IsPartialResistableSpell(uint16 spell_id);
bool IsResistableSpell(uint16 spell_id);
bool IsGroupSpell(uint16 spell_id);
bool IsTGBCompatibleSpell(uint16 spell_id);
bool IsBardSong(uint16 spell_id);
bool IsEffectInSpell(uint16 spell_id, int effect_id);
uint16 GetSpellTriggerSpellID(uint16 spell_id, int effect_id);
bool IsBlankSpellEffect(uint16 spell_id, int effect_index);
bool IsValidSpell(uint32 spell_id);
bool IsSummonSpell(uint16 spell_id);
bool IsDamageSpell(uint16 spell_id);
bool IsAnyDamageSpell(uint16 spell_id);
bool IsDamageOverTimeSpell(uint16 spell_i);
bool IsFearSpell(uint16 spell_id);
bool IsCureSpell(uint16 spell_id);
bool IsHarmTouchSpell(uint16 spell_id);
int GetSpellEffectIndex(uint16 spell_id, int effect_id);
uint8 GetSpellMinimumLevel(uint16 spell_id);
uint8 GetSpellLevel(uint16 spell_id, uint8 class_id);
int CalcBuffDuration_formula(int level, int formula, int duration);
int32 CalculatePoisonCounters(uint16 spell_id);
int32 CalculateDiseaseCounters(uint16 spell_id);
int32 CalculateCurseCounters(uint16 spell_id);
int32 CalculateCorruptionCounters(uint16 spell_id);
int32 CalculateCounters(uint16 spell_id);
bool IsDisciplineBuff(uint16 spell_id);
bool IsDiscipline(uint16 spell_id);
bool IsCombatSkill(uint16 spell_id);
bool IsResurrectionEffects(uint16 spell_id);
int8 GetSpellResurrectionSicknessCheck(uint16 spell_id_one, uint16 spell_id_two);
bool IsRuneSpell(uint16 spell_id);
bool IsMagicRuneSpell(uint16 spell_id);
bool IsManaTapSpell(uint16 spell_id);
bool IsAllianceSpell(uint16 spell_id);
bool IsDeathSaveSpell(uint16 spell_id);
bool IsFullDeathSaveSpell(uint16 spell_id);
bool IsPartialDeathSaveSpell(uint16 spell_id);
bool IsShadowStepSpell(uint16 spell_id);
bool IsSuccorSpell(uint16 spell_id);
bool IsTeleportSpell(uint16 spell_id);
bool IsTranslocateSpell(uint16 spell_id);
bool IsGateSpell(uint16 spell_id);
bool IsIllusionSpell(uint16 spell_id);
bool IsLDoNObjectSpell(uint16 spell_id);
int GetSpellResistType(uint16 spell_id);
int GetSpellTargetType(uint16 spell_id);
bool IsHealOverTimeSpell(uint16 spell_id);
bool IsCompleteHealSpell(uint16 spell_id);
bool IsFastHealSpell(uint16 spell_id);
bool IsVeryFastHealSpell(uint16 spell_id);
bool IsRegularSingleTargetHealSpell(uint16 spell_id);
bool IsRegularPetHealSpell(uint16 spell_id);
bool IsRegularGroupHealSpell(uint16 spell_id);
bool IsGroupCompleteHealSpell(uint16 spell_id);
bool IsGroupHealOverTimeSpell(uint16 spell_id);
bool IsAnyHealSpell(uint16 spell_id);
bool IsAnyBuffSpell(uint16 spell_id);
bool IsDispelSpell(uint16 spell_id);
bool IsEscapeSpell(uint16 spell_id);
bool IsDebuffSpell(uint16 spell_id);
bool IsHateReduxSpell(uint16 spell_id);
bool IsResistDebuffSpell(uint16 spell_id);
bool IsSelfConversionSpell(uint16 spell_id);
bool IsBuffSpell(uint16 spell_id);
bool IsPersistDeathSpell(uint16 spell_id);
bool IsSuspendableSpell(uint16 spell_id);
bool IsCastOnFadeDurationSpell(uint16 spell_id);
bool IsDistanceModifierSpell(uint16 spell_id);
int GetSpellPartialMeleeRuneReduction(uint16 spell_id);
int GetSpellPartialMagicRuneReduction(uint16 spell_id);
int GetSpellPartialMeleeRuneAmount(uint16 spell_id);
int GetSpellPartialMagicRuneAmount(uint16 spell_id);
bool IsNoDetrimentalSpellAggroSpell(uint16 spell_id);
bool IsStackableDOT(uint16 spell_id);
bool IsBardOnlyStackEffect(int effect_id);
bool IsCastWhileInvisibleSpell(uint16 spell_id);
bool IsEffectIgnoredInStacking(int effect_id);
bool IsFocusLimit(int effect_id);
bool IsTargetRequiredForSpell(uint16 spell_id);
bool IsVirusSpell(uint16 spell_id);
int GetSpellViralMinimumSpreadTime(uint16 spell_id);
int GetSpellViralMaximumSpreadTime(uint16 spell_id);
int GetSpellViralSpreadRange(uint16 spell_id);
bool IsInstrumentModifierAppliedToSpellEffect(uint16 spell_id, int effect_id);
bool IsPulsingBardSong(uint16 spell_id);
int GetSpellProcLimitTimer(uint16 spell_id, int proc_type);
bool IsCastNotStandingSpell(uint16 spell_id);
int GetSpellEffectDescriptionNumber(uint16 spell_id);
DmgShieldType GetDamageShieldType(uint16 spell_id, int damage_shield_type = 0);
bool IsRestAllowedSpell(uint16 spell_id);
int GetSpellNimbusEffect(uint16 spell_id);
int GetSpellFuriousBash(uint16 spell_id);
bool IsShortDurationBuff(uint16 spell_id);
bool IsSpellUsableInThisZoneType(uint16 spell_id, uint8 zone_type);
const char *GetSpellName(uint16 spell_id);
int GetSpellStatValue(uint16 spell_id, const char* stat_identifier, uint8 slot = 0);
bool IsCastRestrictedSpell(uint16 spell_id);
bool IsAegolismSpell(uint16 spell_id);
bool AegolismStackingIsSymbolSpell(uint16 spell_id);
bool AegolismStackingIsArmorClassSpell(uint16 spell_id);
int8 SpellEffectsCount(uint16 spell_id);
bool IsLichSpell(uint16 spell_id);
bool IsInstantHealSpell(uint32 spell_id);
bool IsResurrectSpell(uint16 spell_id);
bool RequiresStackCheck(uint16 spell_type);
bool IsResistanceBuffSpell(uint16 spell_id);
bool IsResistanceOnlySpell(uint16 spell_id);
bool IsDamageShieldOnlySpell(uint16 spell_id);
bool IsDamageShieldAndResistSpell(uint16 spell_id);
bool IsHateSpell(uint16 spell_id);
bool IsDisciplineTome(const EQ::ItemData* item);
