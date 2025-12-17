/*	EQEMu: Everquest Server Emulator

	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#pragma once

#include "common/types.h"

#include <map>
#include <string>
#include <vector>

namespace EQ
{
	namespace skills {
		enum SkillType : int {
/*13855*/	Skill1HBlunt = 0,
/*13856*/	Skill1HSlashing,
/*13857*/	Skill2HBlunt,
/*13858*/	Skill2HSlashing,
/*13859*/	SkillAbjuration,
/*13861*/	SkillAlteration, // 5
/*13862*/	SkillApplyPoison,
/*13863*/	SkillArchery,
/*13864*/	SkillBackstab,
/*13866*/	SkillBindWound,
/*13867*/	SkillBash, // 10
/*13871*/	SkillBlock,
/*13872*/	SkillBrassInstruments,
/*13874*/	SkillChanneling,
/*13875*/	SkillConjuration,
/*13876*/	SkillDefense, // 15
/*13877*/	SkillDisarm,
/*13878*/	SkillDisarmTraps,
/*13879*/	SkillDivination,
/*13880*/	SkillDodge,
/*13881*/	SkillDoubleAttack, // 20
/*13882*/	SkillDragonPunch,
/*13924*/	SkillTailRake = SkillDragonPunch, // Iksar Monk equivilent
/*13883*/	SkillDualWield,
/*13884*/	SkillEagleStrike,
/*13885*/	SkillEvocation,
/*13886*/	SkillFeignDeath, // 25
/*13888*/	SkillFlyingKick,
/*13889*/	SkillForage,
/*13890*/	SkillHandtoHand,
/*13891*/	SkillHide,
/*13893*/	SkillKick, // 30
/*13894*/	SkillMeditate,
/*13895*/	SkillMend,
/*13896*/	SkillOffense,
/*13897*/	SkillParry,
/*13899*/	SkillPickLock, // 35
/*13900*/	Skill1HPiercing,				// Changed in RoF2(05-10-2013)
/*13903*/	SkillRiposte,
/*13904*/	SkillRoundKick,
/*13905*/	SkillSafeFall,
/*13906*/	SkillSenseHeading, // 40
/*13908*/	SkillSinging,
/*13909*/	SkillSneak,
/*13910*/	SkillSpecializeAbjure,			// No idea why they truncated this one..especially when there are longer ones...
/*13911*/	SkillSpecializeAlteration,
/*13912*/	SkillSpecializeConjuration, // 45
/*13913*/	SkillSpecializeDivination,
/*13914*/	SkillSpecializeEvocation,
/*13915*/	SkillPickPockets,
/*13916*/	SkillStringedInstruments,
/*13917*/	SkillSwimming, // 50
/*13919*/	SkillThrowing,
/*13920*/	SkillTigerClaw,
/*13921*/	SkillTracking,
/*13923*/	SkillWindInstruments,
/*13854*/	SkillFishing, // 55
/*13853*/	SkillMakePoison,
/*13852*/	SkillTinkering,
/*13851*/	SkillResearch,
/*13850*/	SkillAlchemy,
/*13865*/	SkillBaking, // 60
/*13918*/	SkillTailoring,
/*13907*/	SkillSenseTraps,
/*13870*/	SkillBlacksmithing,
/*13887*/	SkillFletching,
/*13873*/	SkillBrewing, // 65
/*13860*/	SkillAlcoholTolerance,
/*13868*/	SkillBegging,
/*13892*/	SkillJewelryMaking,
/*13901*/	SkillPottery,
/*13898*/	SkillPercussionInstruments, // 70
/*13922*/	SkillIntimidation,
/*13869*/	SkillBerserking,
/*13902*/	SkillTaunt,
/*05837*/	SkillFrenzy, // 74				// This appears to be the only listed one not grouped with the others

// SoF+ specific skills
/*03670*/	SkillRemoveTraps, // 75
/*13049*/	SkillTripleAttack,

// RoF2+ specific skills
/*00789*/	Skill2HPiercing, // 77
// /*01216*/	SkillNone,					// This needs to move down as new skills are added

/*00000*/	SkillCount						// move to last position of active enumeration labels

// Skill Counts
// /*-----*/	SkillCount_62 = 75,			// use for Ti and earlier max skill checks
// /*-----*/	SkillCount_SoF = 77,		// use for SoF thru RoF1 max skill checks
// /*-----*/	SkillCount_RoF2 = 78,		// use for RoF2 max skill checks

// Support values
// /*-----*/	SkillServerArraySize = _SkillCount_RoF2,	// Should reflect last client '_SkillCount'

// Superfluous additions to SkillUseTypes..server-use only
// /*-----*/	ExtSkillGenericTradeskill = 100

		/*					([EQClientVersion]	[0] - Unknown, [3] - SoF, [7] - RoF2[05-10-2013])
			[Skill]			[index]	|	[0]		[1]		[2]		[3]		[4]		[5]		[6]		[7]
			Frenzy			(05837)	|	---		074		074		074		074		074		074		074
			Remove Traps	(03670)	|	---		---		---		075		075		075		075		075
			Triple Attack	(13049)	|	---		---		---		076		076		076		076		076
			2H Piercing		(00789)	|	---		---		---		---		---		---		---		077
		*/

		/*
			[SkillCaps.txt] (SoF+)
			a^b^c^d(^e)	(^e is RoF+, but cursory glance appears to be all zeros)

			a - Class
			b - Skill
			c - Level
			d - Max Value
			(e - Unknown)
		*/

		/*
			Changed (tradeskill==75) to ExtSkillGenericTradeskill in tradeskills.cpp for both instances. If it's a pseudo-enumeration of
			an AA ability, then use the 'ExtSkill' ('ExtendedSkill') prefix with a value >= 100. (current implementation)
		*/
	};

	// temporary until it can be sorted out...
#define HIGHEST_SKILL	Skill2HPiercing
	// Spell Effects use this value to determine if an effect applies to all skills.
#define ALL_SKILLS	-1

	// server profile does not reflect this yet..so, prefixed with 'PACKET_'
#define PACKET_SKILL_ARRAY_SIZE 100

		constexpr int format_as(SkillType skill) { return static_cast<int>(skill); }

		bool IsTradeskill(SkillType skill);
		bool IsSpecializedSkill(SkillType skill);
		float GetSkillMeleePushForce(SkillType skill);
		bool IsBardInstrumentSkill(SkillType skill);
		bool IsCastingSkill(SkillType skill);
		int32 GetBaseDamage(SkillType skill);

		extern const std::map<SkillType, std::string>& GetSkillTypeMap();
		extern const std::vector<SkillType>& GetExtraDamageSkills();

		std::string GetSkillName(SkillType skill);
	} /*skills*/

	struct SkillProfile { // prototype - not implemented
		union {
			struct {
				uint32 _1HBlunt;
				uint32 _1HSlashing;
				uint32 _2HBlunt;
				uint32 _2HSlashing;
				uint32 Abjuration;
				uint32 Alteration;
				uint32 ApplyPoison;
				uint32 Archery;
				uint32 Backstab;
				uint32 BindWound;
				uint32 Bash;
				uint32 Block;
				uint32 BrassInstruments;
				uint32 Channeling;
				uint32 Conjuration;
				uint32 Defense;
				uint32 Disarm;
				uint32 DisarmTraps;
				uint32 Divination;
				uint32 Dodge;
				uint32 DoubleAttack;
				union {
					uint32 DragonPunch;
					uint32 TailRake;
				};
				uint32 DualWield;
				uint32 EagleStrike;
				uint32 Evocation;
				uint32 FeignDeath;
				uint32 FlyingKick;
				uint32 Forage;
				uint32 HandtoHand;
				uint32 Hide;
				uint32 Kick;
				uint32 Meditate;
				uint32 Mend;
				uint32 Offense;
				uint32 Parry;
				uint32 PickLock;
				uint32 _1HPiercing;
				uint32 Riposte;
				uint32 RoundKick;
				uint32 SafeFall;
				uint32 SenseHeading;
				uint32 Singing;
				uint32 Sneak;
				uint32 SpecializeAbjure;
				uint32 SpecializeAlteration;
				uint32 SpecializeConjuration;
				uint32 SpecializeDivination;
				uint32 SpecializeEvocation;
				uint32 PickPockets;
				uint32 StringedInstruments;
				uint32 Swimming;
				uint32 Throwing;
				uint32 TigerClaw;
				uint32 Tracking;
				uint32 WindInstruments;
				uint32 Fishing;
				uint32 MakePoison;
				uint32 Tinkering;
				uint32 Research;
				uint32 Alchemy;
				uint32 Baking;
				uint32 Tailoring;
				uint32 SenseTraps;
				uint32 Blacksmithing;
				uint32 Fletching;
				uint32 Brewing;
				uint32 AlcoholTolerance;
				uint32 Begging;
				uint32 JewelryMaking;
				uint32 Pottery;
				uint32 PercussionInstruments;
				uint32 Intimidation;
				uint32 Berserking;
				uint32 Taunt;
				uint32 Frenzy;
				uint32 RemoveTraps;
				uint32 TripleAttack;
				uint32 _2HPiercing;
				uint32 unused1;
				uint32 unused2;
				uint32 unused3;
				uint32 unused4;
				uint32 unused5;
				uint32 unused6;
				uint32 unused7;
				uint32 unused8;
				uint32 unused9;
				uint32 unused10;
				uint32 unused11;
				uint32 unused12;
				uint32 unused13;
				uint32 unused14;
				uint32 unused15;
				uint32 unused16;
				uint32 unused17;
				uint32 unused18;
				uint32 unused19;
				uint32 unused20;
				uint32 unused21;
				uint32 unused22;
			};
			uint32 Skill[PACKET_SKILL_ARRAY_SIZE];
		};

		SkillProfile();

		uint32* GetSkills() { return reinterpret_cast<uint32*>(&Skill); }

		skills::SkillType GetLastUseableSkill() { return EQ::skills::Skill2HPiercing; }

		size_t GetSkillsArraySize() { return PACKET_SKILL_ARRAY_SIZE; }
		uint32 GetSkill(int skill_id);

		uint32 operator[](int skill_id) { return GetSkill(skill_id); }

		// const
		uint32* GetSkills() const { return const_cast<SkillProfile*>(this)->GetSkills(); }

		skills::SkillType GetLastUseableSkill() const { return const_cast<SkillProfile*>(this)->GetLastUseableSkill(); }

		size_t GetSkillsArraySize() const { return const_cast<SkillProfile*>(this)->GetSkillsArraySize(); }
		uint32 GetSkill(int skill_id) const { return const_cast<SkillProfile*>(this)->GetSkill(skill_id); }

		uint32 operator[](int skill_id) const { return const_cast<SkillProfile*>(this)->GetSkill(skill_id); }
	};

} /*EQEmu*/
