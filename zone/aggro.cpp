/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

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

#include "common/eqemu_logsys.h"
#include "common/faction.h"
#include "common/rulesys.h"
#include "common/spdat.h"
#include "zone/bot.h"
#include "zone/client.h"
#include "zone/entity.h"
#include "zone/map.h"
#include "zone/mob.h"
#include "zone/water_map.h"

extern Zone* zone;
//#define LOSDEBUG 6

void EntityList::DescribeAggro(Client *to_who, NPC *from_who, float d, bool verbose) {
	float distance_squared = (d * d);

	to_who->Message(
		Chat::White,
		fmt::format(
			"Describing aggro for {}.",
			to_who->GetTargetDescription(from_who)
		).c_str()
	);

	bool is_engaged = from_who->IsEngaged();
	bool will_aggro_npcs = from_who->GetNPCAggro();
	if (is_engaged) {
		Mob *top = from_who->GetHateTop();
		to_who->Message(
			Chat::White,
			fmt::format(
				"I am currently engaged with {}.",
				(
					!top ?
					"nothing" :
					from_who->GetTargetDescription(top)
				)
			).c_str()
		);
	}

	if (verbose) {
		int faction_id = from_who->GetPrimaryFaction();
		Mob *owner = from_who->GetOwner();
		if(owner) {
			faction_id = owner->GetPrimaryFaction();
		}

		std::string faction_name = (
			faction_id > 0 ?
			content_db.GetFactionName(faction_id) :
			(
				faction_id == 0 ?
				"None" :
				fmt::format(
					"Special Faction {}",
					faction_id
				)
			)
		);

		to_who->Message(
			Chat::White,
			fmt::format(
				"{} is on Faction {} ({}).",
				to_who->GetTargetDescription(from_who),
				faction_name,
				faction_id
			).c_str()
		);
	}

	for (const auto& npc_entity : entity_list.GetNPCList()) {
		auto entity_id = npc_entity.first;
		auto npc = npc_entity.second;
		if (npc == from_who) {
			continue;
		}

		if (DistanceSquared(npc->GetPosition(), from_who->GetPosition()) > distance_squared) {
			continue;
		}

		if (is_engaged) {
			int64 hate_amount = from_who->GetHateAmount(npc);
			to_who->Message(
				Chat::White,
				fmt::format(
					"{} is {}on my hate list{}.",
					to_who->GetTargetDescription(npc),
					!hate_amount ? "not " : "",
					(
						!hate_amount ?
						"" :
						fmt::format(
							" with a hate amount of {}.",
							hate_amount
						)
					)
				).c_str()
			);
		} else if (!will_aggro_npcs) {
			to_who->Message(
				Chat::White,
				fmt::format(
					"{} is an NPC and I cannot aggro NPCs.",
					to_who->GetTargetDescription(npc)
				).c_str()
			);
		} else {
			from_who->DescribeAggro(to_who, npc, verbose);
		}
	}
}

void NPC::DescribeAggro(Client *to_who, Mob *mob, bool verbose) {
	//this logic is duplicated from below, try to keep it up to date.
	float aggro_range = GetAggroRange();
	float x_range = std::abs(mob->GetX() - GetX());
	float y_range = std::abs(mob->GetY() - GetY());
	float z_range = std::abs(mob->GetZ() - GetZ());
	if (
		x_range > aggro_range ||
		y_range > aggro_range ||
		z_range > aggro_range
	) {
		to_who->Message(
			Chat::White,
			fmt::format(
				"{} is out of range. X Range: {} Y Range: {} Z Range: {} Aggro Range: {}",
				to_who->GetTargetDescription(mob),
				x_range,
				y_range,
				z_range,
				aggro_range
			).c_str()
		);
		return;
	}

	if (mob->IsInvisible(this)) {
		to_who->Message(
			Chat::White,
			fmt::format(
				"{} is invisible to me.",
				to_who->GetTargetDescription(mob)
			).c_str()
		);
		return;
	}

	if (
		mob->IsClient() &&
		(
			!mob->CastToClient()->Connected() ||
			mob->CastToClient()->IsLD() ||
			mob->CastToClient()->IsBecomeNPC() ||
			mob->CastToClient()->GetGM()
		)
	) {
		to_who->Message(
			Chat::White,
			fmt::format(
				"{} is a GM or is not connected.",
				to_who->GetTargetDescription(mob)
			).c_str()
		);
		return;
	}


	if (mob == GetOwner()) {
		to_who->Message(
			Chat::White,
			fmt::format(
				"{} is my owner.",
				to_who->GetTargetDescription(mob)
			).c_str()
		);
		return;
	}

	float distance_squared = DistanceSquared(mob->GetPosition(), m_Position);
	float aggro_range_squared = (aggro_range * aggro_range);
	if (distance_squared > aggro_range_squared) {
		to_who->Message(
			Chat::White,
			fmt::format(
				"{} is out of range. Distance: {:.2f} Aggro Range: {:.2f}",
				to_who->GetTargetDescription(mob),
				distance_squared,
				aggro_range_squared
			).c_str()
		);
		return;
	}

	if (RuleB(Aggro, UseLevelAggro)) {
		if (
			GetLevel() < RuleI(Aggro, MinAggroLevel) &&
			mob->GetLevelCon(GetLevel()) == ConsiderColor::Gray &&
			GetBodyType() != BodyType::Undead &&
			!AlwaysAggro()
		) {
			to_who->Message(
				Chat::White,
				fmt::format(
					"{} considers Red to me.",
					to_who->GetTargetDescription(mob)
				).c_str()
			);
			return;
		}
	} else {
		if (
			GetINT() > RuleI(Aggro, IntAggroThreshold) &&
			mob->GetLevelCon(GetLevel()) == ConsiderColor::Gray &&
			!AlwaysAggro()
		) {
			to_who->Message(
				Chat::White,
				fmt::format(
					"{} considers Red to me.",
					to_who->GetTargetDescription(mob)
				).c_str()
			);
			return;
		}
	}

	if (verbose) {
		int faction_id = GetPrimaryFaction();
		int mob_faction_id = mob->GetPrimaryFaction();
		Mob *owner = GetOwner();
		if (owner) {
			faction_id = owner->GetPrimaryFaction();
		}

		owner = mob->GetOwner();
		if (mob_faction_id && owner) {
			mob_faction_id = owner->GetPrimaryFaction();
		}

		if (!mob_faction_id) {
			to_who->Message(
				Chat::White,
				fmt::format(
					"{} has no primary Faction.",
					to_who->GetTargetDescription(mob)
				).c_str()
			);
		} else if (mob_faction_id < 0) {
			to_who->Message(
				Chat::White,
				fmt::format(
					"{} is on special Faction {}.",
					to_who->GetTargetDescription(mob),
					mob_faction_id
				).c_str()
			);
		} else {
			auto faction_name = content_db.GetFactionName(mob_faction_id);
			bool has_entry = false;
			for (auto faction : faction_list) {
				if (static_cast<int>(faction.faction_id) == mob_faction_id) {
					to_who->Message(
						Chat::White,
						fmt::format(
							"{} has {} standing with Faction {} ({}) with their Faction Level of {}",
							to_who->GetTargetDescription(mob),
							(
								faction.npc_value != 0 ?
								(
									faction.npc_value > 0 ?
									"positive" :
									"negative"
								 ) :
								"neutral"
							),
							faction_name,
							faction.faction_id,
							faction.npc_value
						).c_str()
					);
					has_entry = true;
					break;
				}
			}

			if (!has_entry) {
				to_who->Message(
					Chat::White,
					fmt::format(
						"{} is on Faction {} ({}), for which I do not have an entry.",
						to_who->GetTargetDescription(mob),
						faction_name,
						mob_faction_id
					).c_str()
				);
			}
		}
	}

	auto faction_value = mob->GetReverseFactionCon(this);

	if (
		!(
			faction_value == FACTION_THREATENINGLY ||
			faction_value == FACTION_SCOWLS ||
			(
				mob->GetPrimaryFaction() != GetPrimaryFaction() &&
				mob->GetPrimaryFaction() == -4 &&
				!GetOwner()
			)
		)
	) {
		to_who->Message(
			Chat::White,
			fmt::format(
				"{} does not have low enough faction, their Faction Level is {} ({}).",
				to_who->GetTargetDescription(mob),
				FactionValueToString(faction_value),
				static_cast<int>(faction_value)
			).c_str()
		);
		return;
	}

	if (!CheckLosFN(mob)) {
		to_who->Message(
			Chat::White,
			fmt::format(
				"{} is out of sight.",
				to_who->GetTargetDescription(mob)
			).c_str()
		);
	}

	to_who->Message(
		Chat::White,
		fmt::format(
			"{} meets all conditions, I should be attacking them.",
			to_who->GetTargetDescription(mob)
		).c_str()
	);
}

/*
	If you change this function, you should update the above function
	to keep the #aggro command accurate.
*/
bool Mob::CheckWillAggro(Mob *mob) {
	if(!mob) {
		return false;
	}

	//sometimes if a client has some lag while zoning into a dangerous place while either invis or a GM
	//they will aggro mobs even though it's supposed to be impossible, to lets make sure we've finished connecting
	if (mob->IsClient()) {
		if (
			!mob->CastToClient()->ClientFinishedLoading() ||
			mob->CastToClient()->IsHoveringForRespawn() ||
			mob->CastToClient()->bZoning
		) {
			return false;
		}
	}

	// We don't want to aggro clients outside of water if we're water only.
	if (
		mob->IsClient() &&
		mob->CastToClient()->GetLastRegion() != RegionTypeWater &&
		IsUnderwaterOnly()
	) {
		return false;
	}

	/**
	 * Pets shouldn't scan for aggro
	 */
	if (GetOwner()) {
		return false;
	}

	Mob* pet_owner = mob->GetOwner();

	if (pet_owner && pet_owner->IsOfClientBot()) {
		if (mob->IsPetAggroExempt(pet_owner)) {
			return false;
		}
	}

	if (IsNPC() && mob->IsNPC() && mob->GetSpecialAbility(SpecialAbility::NPCAggroImmunity)) {
		return false;
	}

	// Check If it's invisible and if we can see invis
	// Check if it's a client, and that the client is connected and not linkdead,
	// and that the client isn't Playing an NPC, with thier gm flag on
	// Check if it's not a Interactive NPC
	// Trumpcard: The 1st 3 checks are low cost calcs to filter out unnessecary distance checks. Leave them at the beginning, they are the most likely occurence.
	// Image: I moved this up by itself above faction and distance checks because if one of these return true, theres no reason to go through the other information

	float aggro_range = GetAggroRange();
	float x_range = std::abs(mob->GetX() - GetX());
	float y_range = std::abs(mob->GetY() - GetY());
	float z_range = std::abs(mob->GetZ() - GetZ());

	if (
		x_range > aggro_range ||
		y_range > aggro_range ||
		z_range > aggro_range ||
		mob->IsInvisible(this) ||
		(
			mob->IsClient() &&
			(
				!mob->CastToClient()->Connected()
				|| mob->CastToClient()->IsLD()
				|| mob->CastToClient()->IsBecomeNPC()
				|| mob->CastToClient()->GetGM()
			)
		)
	) {
		return false;
	}

	// Don't aggro new clients if we are already engaged unless PROX_AGGRO is set
	// Frustrated mobs (all rooted up with no one to kill)
	// will engage without PROX_AGGRO ability if someone new is close now.

	const bool is_frustrated = IsRooted() && !CombatRange(target);

	if (
		!is_frustrated &&
		IsEngaged() &&
		(
			(
				!GetSpecialAbility(SpecialAbility::ProximityAggro) &&
				GetBodyType() != BodyType::Undead
			) ||
			(
				GetSpecialAbility(SpecialAbility::ProximityAggro) &&
				!CombatRange(mob)
			)
		)
	) {
		LogAggro(
			"[{}] is in combat, and does not have prox_aggro, or does and is out of combat range with [{}]",
			GetName(),
			mob->GetName()
		);
		return false;
	}

	//im not sure I understand this..
	//if I have an owner and it is not this mob, then I cannot
	//aggro this mob...???
	//changed to be 'if I have an owner and this is it'
	if(mob == GetOwner()) {
		return false;
	}

	float distance_squared = DistanceSquared(mob->GetPosition(), m_Position);
	float aggro_range_squared = (aggro_range * aggro_range);

	if (distance_squared > aggro_range_squared ) {
		// Skip it, out of range
		return false;
	}

	//Image: Get their current target and faction value now that its required
	//this function call should seem backwards
	FACTION_VALUE faction_value = mob->GetReverseFactionCon(this);

	// Make sure they're still in the zone
	// Are they in range?
	// Are they kos?
	// Are we stupid or are they green
	// and they don't have their gm flag on
	int heroic_cha_mod = (mob->itembonuses.HeroicCHA / 25); // 800 Heroic CHA cap
	if(heroic_cha_mod > THREATENINGLY_AGGRO_CHANCE) {
		heroic_cha_mod = THREATENINGLY_AGGRO_CHANCE;
	}

	if (
		RuleB(Aggro, UseLevelAggro) &&
		(
			GetLevel() >= RuleI(Aggro, MinAggroLevel) ||
			GetBodyType() == BodyType::Undead ||
			AlwaysAggro() ||
			(
				mob->IsClient() &&
				mob->CastToClient()->IsSitting()
			) ||
			mob->GetLevelCon(GetLevel()) != ConsiderColor::Gray
		) &&
		(
			faction_value == FACTION_SCOWLS ||
			(
				mob->GetPrimaryFaction() != GetPrimaryFaction() &&
				mob->GetPrimaryFaction() == -4 &&
				!GetOwner()
			) ||
			(
				faction_value == FACTION_THREATENINGLY &&
				zone->random.Roll(THREATENINGLY_AGGRO_CHANCE - heroic_cha_mod)
			)
		)
	) {
		if(CheckLosFN(mob)) {
			LogAggro("Check aggro for [{}] target [{}]", GetName(), mob->GetName());
			return true;
		}
	} else {
		if (
			(
				(RuleB(Aggro, UndeadAlwaysAggro) && GetBodyType() == BodyType::Undead) ||
				(GetINT() <= RuleI(Aggro, IntAggroThreshold)) ||
				AlwaysAggro() ||
				(
					mob->IsClient() &&
					mob->CastToClient()->IsSitting()
				) ||
				mob->GetLevelCon(GetLevel()) != ConsiderColor::Gray
			) &&
			(
				faction_value == FACTION_SCOWLS	||
				(
					mob->GetPrimaryFaction() != GetPrimaryFaction() &&
					mob->GetPrimaryFaction() == -4 &&
					!GetOwner()
				) ||
				(
					faction_value == FACTION_THREATENINGLY
					&& zone->random.Roll(THREATENINGLY_AGGRO_CHANCE - heroic_cha_mod)
				)
			)
		) {
			if(CheckLosFN(mob)) {
				LogAggro("Check aggro for [{}] target [{}]", GetName(), mob->GetName());
				return true;
			}
		}
	}

	LogAggro("Is In zone?:[{}]\n", mob->InZone());
	LogAggro("Dist^2: [{}]\n", distance_squared);
	LogAggro("Range^2: [{}]\n", aggro_range_squared);
	LogAggro("Faction: [{}]\n", static_cast<int>(faction_value));
	LogAggro("AlwaysAggroFlag: [{}]\n", AlwaysAggro());
	LogAggro("Int: [{}]\n", GetINT());
	LogAggro("Con: [{}]\n", GetLevelCon(mob->GetLevel()));

	return false;
}

bool Mob::IsPetAggroExempt(Mob* pet_owner) {
	if (!pet_owner) {
		return false;
	}

	bool exempt_client_pet = pet_owner->IsClient() && !RuleB(Aggro, AggroPlayerPets);
	bool exempt_bot_pet = pet_owner->IsBot() && !RuleB(Aggro, AggroBotPets);

	if (exempt_client_pet || exempt_bot_pet) {
		return true;
	}

	Mob* ultimate_owner = GetUltimateOwner();
	Client* client_owner = (ultimate_owner && ultimate_owner->IsClient())
		? ultimate_owner->CastToClient()
		: nullptr;

	if (client_owner && client_owner->GetGM()) {
		return true;
	}

	if (GetSpecialAbility(SpecialAbility::AggroImmunity)) {
		return true;
	}

	return false;
}

int EntityList::FleeAllyCount(Mob* attacker, Mob* skipped)
{
	// Return a list of how many NPCs of the same faction or race are within aggro range of the given exclude Mob.
	if (!attacker) {
		return 0;
	}

	int count = 0;

	for (const auto& e : npc_list) {
		NPC* n = e.second;
		if (!n || n == skipped) {
			continue;
		}

		float       aggro_range  = n->GetAggroRange();
		const float assist_range = n->GetAssistRange();

		if (assist_range > aggro_range) {
			aggro_range = assist_range;
		}

		// Square it because we will be using DistNoRoot
		aggro_range *= aggro_range;

		if (DistanceSquared(n->GetPosition(), skipped->GetPosition()) > aggro_range) {
			continue;
		}

		const auto& excluded = Strings::Split(RuleS(Aggro, ExcludedFleeAllyFactionIDs));

		const auto& f = std::find_if(
			excluded.begin(),
			excluded.end(),
			[&](std::string x) {
				return Strings::ToUnsignedInt(x) == skipped->GetPrimaryFaction();
			}
		);

		const bool is_excluded = f != excluded.end();

		// If exclude doesn't have a faction, check for buddies based on race.
		// Also exclude common factions such as noob monsters, indifferent, kos, kos animal
		if (!is_excluded) {
			if (n->GetPrimaryFaction() != skipped->GetPrimaryFaction()) {
				continue;
			}
		} else {
			if (n->GetBaseRace() != skipped->GetBaseRace() || n->IsCharmedPet()) {
				continue;
			}
		}

		LogFleeDetail(
		"[{}] on faction [{}] with aggro_range [{}] is at [{}], [{}], [{}] and will count as an ally for [{}]",
		n->GetName(),
		n->GetPrimaryFaction(),
		aggro_range,
		n->GetX(),
		n->GetY(),
		n->GetZ(),
		skipped->GetName()
		);

		++count;
	}

	return count;
}

int EntityList::GetHatedCount(Mob *attacker, Mob *exclude, bool inc_gray_con)
{
	// Return a list of how many non-feared, non-mezzed, non-green mobs, within aggro range, hate *attacker
	if (!attacker)
		return 0;

	int Count = 0;

	for (auto it = npc_list.begin(); it != npc_list.end(); ++it) {
		NPC *mob = it->second;
		if (!mob || (mob == exclude)) {
			continue;
		}

		if (!mob->IsEngaged()) {
			continue;
		}

		if (mob->IsFeared() || mob->IsMezzed()) {
			continue;
		}

		if (!inc_gray_con && attacker->GetLevelCon(mob->GetLevel()) == ConsiderColor::Gray) {
			continue;
		}

		if (!mob->CheckAggro(attacker)) {
			continue;
		}

		float AggroRange = mob->GetAggroRange();

		// Square it because we will be using DistNoRoot

		AggroRange *= AggroRange;

		if (DistanceSquared(mob->GetPosition(), attacker->GetPosition()) > AggroRange) {
			continue;
		}
		Count++;
	}
	return Count;
}

/**
 * @param target
 * @param isSpellAttack
 * @return
 */
bool Mob::IsAttackAllowed(Mob *target, bool isSpellAttack)
{

	Mob *mob1 = nullptr, *mob2 = nullptr, *tempmob = nullptr;
	Client *c1 = nullptr, *c2 = nullptr, *becomenpc = nullptr;
//	NPC *npc1, *npc2;
	int reverse;

	if (!zone->CanDoCombat()) {
		return false;
	}

	// some special cases
	if (!target) {
		return false;
	}

	if (this == target) {    // you can attack yourself
		return true;
	}

	if (target->GetSpecialAbility(SpecialAbility::HarmFromClientImmunity)) {
		return false;
	}

	if (IsBot() && target->GetSpecialAbility(SpecialAbility::BotDamageImmunity)) {
		return false;
	}

	if (IsClient() && target->GetSpecialAbility(SpecialAbility::ClientDamageImmunity)) {
		return false;
	}

	if (IsNPC() && target->GetSpecialAbility(SpecialAbility::NPCDamageImmunity)) {
		return false;
	}

	if (target->IsHorse()) {
		return false;
	}

	// can't damage own pet (applies to everthing)
	Mob* target_owner = target->GetOwner();
	Mob* our_owner = GetOwner();

	// Self-owner check
	if (target_owner == this || our_owner == target) {
		return false;
	}

	// Bot-specific logic
	if (IsBot()) {
		Mob* target_ultimate_owner = target->IsBot() ? target->CastToBot()->GetBotOwner() : target->GetUltimateOwner();
		Mob* our_ultimate_owner = CastToBot()->GetBotOwner();

		if (target_ultimate_owner) {
			if (target_ultimate_owner == our_ultimate_owner || target_ultimate_owner->IsOfClientBot()) {
				return false;
			}
		}

		// Bots should not attack their ultimate owner
		if (our_ultimate_owner == target) {
			return false;
		}
	}

	// invalidate for swarm pets for later on if their owner is a corpse
	if (IsNPC() && CastToNPC()->GetSwarmInfo() && our_owner &&
			our_owner->IsCorpse() && !our_owner->IsPlayerCorpse())
		our_owner = nullptr;
	if (target->IsNPC() && target->CastToNPC()->GetSwarmInfo() && target_owner &&
			target_owner->IsCorpse() && !target_owner->IsPlayerCorpse())
		target_owner = nullptr;

	//cannot hurt untargetable mobs
	uint8 bt = target->GetBodyType();

	if(bt == BodyType::NoTarget || bt == BodyType::NoTarget2) {
		if (RuleB(Pets, UnTargetableSwarmPet)) {
			if (target->IsNPC()) {
				if (!target->CastToNPC()->GetSwarmOwner()) {
					return(false);
				}
			} else {
				return(false);
			}
		} else {
			return(false);
		}
	}

	if(!isSpellAttack)
	{
		if(GetClass() == Class::LDoNTreasure)
		{
			return false;
		}
	}

	// the format here is a matrix of mob type vs mob type.
	// redundant ones are omitted and the reverse is tried if it falls through.

	// first figure out if we're pets. we always look at the master's flags.
	// no need to compare pets to anything
	mob1 = our_owner ? our_owner : this;
	mob2 = target_owner ? target_owner : target;

	reverse = 0;
	do
	{
		if(_CLIENT(mob1))
		{
			if(_CLIENT(mob2))					// client vs client
			{
				c1 = mob1->CastToClient();
				c2 = mob2->CastToClient();

				if	// if both are pvp they can fight
				(
					c1->GetPVP() &&
					c2->GetPVP()
				)
					return true;
				else if	// if they're dueling they can go at it
				(
					c1->IsDueling() &&
					c2->IsDueling() &&
					c1->GetDuelTarget() == c2->GetID() &&
					c2->GetDuelTarget() == c1->GetID()
				)
					return true;
				else
					return false;
			}
			else if(_NPC(mob2))				// client vs npc
			{
				return true;
			}
			else if(_BECOMENPC(mob2))	// client vs becomenpc
			{
				c1 = mob1->CastToClient();
				becomenpc = mob2->CastToClient();

				if(c1->GetLevel() > becomenpc->GetBecomeNPCLevel())
					return false;
				else
					return true;
			}
			else if(_CLIENTCORPSE(mob2))	// client vs client corpse
			{
				return false;
			}
			else if(_NPCCORPSE(mob2))	// client vs npc corpse
			{
				return false;
			}
		}
		else if(_NPC(mob1))
		{
			if(_NPC(mob2))						// npc vs npc
			{
/*
this says that an NPC can NEVER attack a faction ally...
this is stupid... somebody else should check this rule if they want to
enforce it, this just says 'can they possibly fight based on their
type', in which case, the answer is yes.
*/
/*				npc1 = mob1->CastToNPC();
				npc2 = mob2->CastToNPC();
				if
				(
					npc1->GetPrimaryFaction() != 0 &&
					npc2->GetPrimaryFaction() != 0 &&
					(
						npc1->GetPrimaryFaction() == npc2->GetPrimaryFaction() ||
						npc1->IsFactionListAlly(npc2->GetPrimaryFaction())
					)
				)
					return false;
				else
*/
					return true;
			}
			else if(_BECOMENPC(mob2))	// npc vs becomenpc
			{
				return true;
			}
			else if(_CLIENTCORPSE(mob2))	// npc vs client corpse
			{
				return false;
			}
			else if(_NPCCORPSE(mob2))	// npc vs npc corpse
			{
				return false;
			}
		}
		else if(_BECOMENPC(mob1))
		{
			if(_BECOMENPC(mob2))			// becomenpc vs becomenpc
			{
				return true;
			}
			else if(_CLIENTCORPSE(mob2))	// becomenpc vs client corpse
			{
				return false;
			}
			else if(_NPCCORPSE(mob2))	// becomenpc vs npc corpse
			{
				return false;
			}
		}
		else if(_CLIENTCORPSE(mob1))
		{
			if(_CLIENTCORPSE(mob2))		// client corpse vs client corpse
			{
				return false;
			}
			else if(_NPCCORPSE(mob2))	// client corpse vs npc corpse
			{
				return false;
			}
		}
		else if(_NPCCORPSE(mob1))
		{
			if(_NPCCORPSE(mob2))			// npc corpse vs npc corpse
			{
				return false;
			}
		}

		if (RuleB(Bots, Enabled)) {
			// this is HIGHLY inefficient
			bool HasRuleDefined     = false;
			bool IsBotAttackAllowed = false;
			IsBotAttackAllowed = Bot::IsBotAttackAllowed(mob1, mob2, HasRuleDefined);
			if (HasRuleDefined) {
				return IsBotAttackAllowed;
			}
		}

		// we fell through, now we swap the 2 mobs and run through again once more
		tempmob = mob1;
		mob1 = mob2;
		mob2 = tempmob;
	}
	while( reverse++ == 0 );

	LogDebug("Mob::IsAttackAllowed: don't have a rule for this - [{}] vs [{}]\n", GetName(), target->GetName());
	return false;
}


// this is to check if non detrimental things are allowed to be done
// to the target. clients cannot affect npcs and vice versa, and clients
// cannot affect other clients that are not of the same pvp flag as them.
// also goes for their pets
bool Mob::IsBeneficialAllowed(Mob *target)
{
	Mob *mob1 = nullptr, *mob2 = nullptr, *tempmob = nullptr;
	Client *c1 = nullptr, *c2 = nullptr;
	int reverse;

	if(!target)
		return false;

	if (target->GetAllowBeneficial())
		return true;

	// see IsAttackAllowed for notes

	// first figure out if we're pets. we always look at the master's flags.
	// no need to compare pets to anything
	mob1 = GetOwnerID() ? GetOwner() : this;
	mob2 = target->GetOwnerID() ? target->GetOwner() : target;

	// if it's self target or our own pet it's ok
	if(mob1 == mob2)
		return true;

	reverse = 0;
	do
	{
		if(_CLIENT(mob1))
		{
			if(_CLIENT(mob2))					// client to client
			{
				c1 = mob1->CastToClient();
				c2 = mob2->CastToClient();

				if(c1->GetPVP() == c2->GetPVP())
					return true;
				else if	// if they're dueling they can heal each other too
				(
					c1->IsDueling() &&
					c2->IsDueling() &&
					c1->GetDuelTarget() == c2->GetID() &&
					c2->GetDuelTarget() == c1->GetID()
				)
					return true;
				else
					return false;
			}
			else if(_NPC(mob2))				// client to npc
			{
				/* fall through and swap positions */
			}
			else if(_BECOMENPC(mob2))	// client to becomenpc
			{
				return false;
			}
			else if(_CLIENTCORPSE(mob2))	// client to client corpse
			{
				return true;
			}
			else if(_NPCCORPSE(mob2))	// client to npc corpse
			{
				return false;
			}
			else if (mob2 && mob2->IsBot()) {
				return true;
			}
		}
		else if(_NPC(mob1))
		{
			if(_CLIENT(mob2))
			{
				return false;
			}
			if(_NPC(mob2))						// npc to npc
			{
				return true;
			}
			else if(_BECOMENPC(mob2))	// npc to becomenpc
			{
				return true;
			}
			else if(_CLIENTCORPSE(mob2))	// npc to client corpse
			{
				return false;
			}
			else if(_NPCCORPSE(mob2))	// npc to npc corpse
			{
				return false;
			}
		}
		else if(_BECOMENPC(mob1))
		{
			if(_BECOMENPC(mob2))			// becomenpc to becomenpc
			{
				return true;
			}
			else if(_CLIENTCORPSE(mob2))	// becomenpc to client corpse
			{
				return false;
			}
			else if(_NPCCORPSE(mob2))	// becomenpc to npc corpse
			{
				return false;
			}
		}
		else if(_CLIENTCORPSE(mob1))
		{
			if(_CLIENTCORPSE(mob2))		// client corpse to client corpse
			{
				return false;
			}
			else if(_NPCCORPSE(mob2))	// client corpse to npc corpse
			{
				return false;
			}
		}
		else if(_NPCCORPSE(mob1))
		{
			if(_NPCCORPSE(mob2))			// npc corpse to npc corpse
			{
				return false;
			}
		}

		// we fell through, now we swap the 2 mobs and run through again once more
		tempmob = mob1;
		mob1 = mob2;
		mob2 = tempmob;
	}
	while( reverse++ == 0 );

	LogDebug("Mob::IsBeneficialAllowed: don't have a rule for this - [{}] to [{}]\n", GetName(), target->GetName());
	return false;
}

bool Mob::CombatRange(Mob* other, float fixed_size_mod, bool aeRampage, ExtraAttackOptions *opts) {
	if (!other) {
		return(false);
	}

	float size_mod = GetSize();
	float other_size_mod = other->GetSize();

	if (GetRace() == Race::LavaDragon || GetRace() == Race::Wurm || GetRace() == Race::GhostDragon) { //For races with a fixed size
		size_mod = 60.0f;
	}
	else if (size_mod < 6.0) {
		size_mod = 8.0f;
	}

	if (other->GetRace() == Race::LavaDragon || other->GetRace() == Race::Wurm || other->GetRace() == Race::GhostDragon) { //For races with a fixed size
		other_size_mod = 60.0f;
	}
	else if (other_size_mod < 6.0) {
		other_size_mod = 8.0f;
	}

	if (other_size_mod > size_mod) {
		size_mod = other_size_mod;
	}

	// this could still use some work, but for now it's an improvement....

	if (size_mod > 29) {
		size_mod *= size_mod;
	} else if (size_mod > 19) {
		size_mod *= size_mod * 2;
	} else {
		size_mod *= size_mod * 4;
	}

	if (other->GetRace() == Race::VeliousDragon)		// Lord Vyemm and other velious dragons
	{
		size_mod *= 1.75;
	}
	if (other->GetRace() == Race::DragonSkeleton)		// Dracoliche in Fear.  Skeletal Dragon
	{
		size_mod *= 2.25;
	}

	size_mod *= RuleR(Combat,HitBoxMod);		// used for testing sizemods on different races.
	size_mod *= fixed_size_mod;					// used to extend the size_mod

	// Melee chasing fleeing mobs is borked.  The client updates don't
	// come to the server quickly enough, especially when mob is running
	// and/or PC has good run speed.  This change is a hack, but it greatly
	// improved playability and "you are too far away" while chasing
	// a fleeing mob.  The Blind check is to make sure that this does not
	// apply to disoriented fleeing mobs who need proximity to turn and fight.
	if  (other->currently_fleeing && !other->IsBlind()) {
		size_mod *= 3;
	}

	// prevention of ridiculously sized hit boxes
	if (size_mod > 10000) {
		size_mod = size_mod / 7;
	}

	float _DistNoRoot = DistanceSquaredNoZ(m_Position, other->GetPosition());
	float _zDist = m_Position.z - other->GetZ();
	_zDist *= _zDist;

	if (GetSpecialAbility(SpecialAbility::NPCChaseDistance)) {

		bool DoLoSCheck = true;
		float max_dist = static_cast<float>(GetSpecialAbilityParam(SpecialAbility::NPCChaseDistance, 0));
		float min_distance = static_cast<float>(GetSpecialAbilityParam(SpecialAbility::NPCChaseDistance, 1));

		if (GetSpecialAbilityParam(SpecialAbility::NPCChaseDistance, 2)) {
			DoLoSCheck = false; //Ignore line of sight check
		}

		if (max_dist == 1) {
			max_dist = 250.0f; //Default it to 250 if you forget to put a value
		}

		max_dist = max_dist * max_dist;

		if (!min_distance) {
			min_distance = size_mod; //Default to melee range
		} else {
			min_distance = min_distance * min_distance;
		}

		if ((DoLoSCheck && CheckLastLosState()) && (_DistNoRoot >= min_distance && _DistNoRoot <= max_dist)) {
			SetPseudoRoot(true);
		} else {
			SetPseudoRoot(false);
		}
	}

	if (aeRampage) {
		float aeramp_size = RuleR(Combat, AERampageMaxDistance);

		LogCombatDetail("AERampage: Default - aeramp_size = [{}] ", aeramp_size);

		if (opts) {
			if (opts->range_percent > 0) {
				aeramp_size = opts->range_percent;
				LogCombatDetail("AE Rampage: range_percent = [{}] -- aeramp_size [{}]", opts->range_percent, aeramp_size);
			}
		}

		if (aeramp_size <= 0) {
			aeramp_size = 0.90;
		} else {
			aeramp_size /= 100;
		}

		float ramp_range = size_mod * aeramp_size;

		LogCombatDetail("AE Rampage: ramp_range = [{}] -- (size_mod [{}] * aeramp_size [{}])", ramp_range, size_mod, aeramp_size);
		LogCombatDetail("AE Rampage: _DistNoRoot [{}] <= ramp_range [{}]", _DistNoRoot, ramp_range);

		if (_DistNoRoot <= ramp_range) {
			LogCombatDetail("AE Rampage: Combat Distance returned [true]");
			return true;
		} else {
			LogCombatDetail("AE Rampage: Combat Distance returned [false]");
			return false;
		}
	}

	if (_DistNoRoot <= size_mod) {
		//A hack to kill an exploit till we get something better.
		if (flymode != GravityBehavior::Flying && _zDist > 500 && !CheckLastLosState()) {
			return false;
		}
		return true;
	}

	return false;
}

bool Mob::CheckLosFN(Mob *other)
{
	bool Result = false;

	if (other) {
		Result = CheckLosFN(other->GetX(), other->GetY(), other->GetZ(), other->GetSize());
	}

	SetLastLosState(Result);

	return Result;
}

bool Mob::CheckLosFN(float posX, float posY, float posZ, float mobSize) {
	if(zone->zonemap == nullptr) {
		//not sure what the best return is on error
		//should make this a database variable, but im lazy today
#ifdef LOS_DEFAULT_CAN_SEE
		return(true);
#else
		return(false);
#endif
	}

	glm::vec3 myloc;
	glm::vec3 oloc;

#define LOS_DEFAULT_HEIGHT 6.0f

	myloc.x = GetX();
	myloc.y = GetY();
	myloc.z = GetZ() + (GetSize()==0.0?LOS_DEFAULT_HEIGHT:GetSize())/2 * HEAD_POSITION;

	oloc.x = posX;
	oloc.y = posY;
	oloc.z = posZ + (mobSize==0.0?LOS_DEFAULT_HEIGHT:mobSize)/2 * SEE_POSITION;

#if LOSDEBUG>=5
	LogDebug("LOS from ([{}], [{}], [{}]) to ([{}], [{}], [{}]) sizes: ([{}], [{}])", myloc.x, myloc.y, myloc.z, oloc.x, oloc.y, oloc.z, GetSize(), mobSize);
#endif
	return zone->zonemap->CheckLoS(myloc, oloc);
}

bool Mob::CheckLosFN(glm::vec3 posWatcher, float sizeWatcher, glm::vec3 posTarget, float sizeTarget) {
	if (zone->zonemap == nullptr) {
		//not sure what the best return is on error
		//should make this a database variable, but im lazy today
#ifdef LOS_DEFAULT_CAN_SEE
		return(true);
#else
		return(false);
#endif
	}

#define LOS_DEFAULT_HEIGHT 6.0f

	posWatcher.z += (sizeWatcher == 0.0f ? LOS_DEFAULT_HEIGHT : sizeWatcher) / 2 * HEAD_POSITION;
	posTarget.z += (sizeTarget == 0.0f ? LOS_DEFAULT_HEIGHT : sizeTarget) / 2 * SEE_POSITION;

#if LOSDEBUG>=5
	LogDebug("LOS from ([{}], [{}], [{}]) to ([{}], [{}], [{}]) sizes: ([{}], [{}]) [static]", posWatcher.x, posWatcher.y, posWatcher.z, posTarget.x, posTarget.y, posTarget.z, sizeWatcher, sizeTarget);
#endif
	return zone->zonemap->CheckLoS(posWatcher, posTarget);
}

bool Mob::CheckPositioningLosFN(Mob* other, float x, float y, float z) {
	if (!zone->zonemap) {
		//not sure what the best return is on error
		//should make this a database variable, but im lazy today
#ifdef LOS_DEFAULT_CAN_SEE
		return(true);
#else
		return(false);
#endif
	}

	if (!other) {
		return(true);
	}
	glm::vec3 myloc;
	glm::vec3 oloc;

#define LOS_DEFAULT_HEIGHT 6.0f

	oloc.x = other->GetX();
	oloc.y = other->GetY();
	oloc.z = other->GetZ() + (other->GetSize() == 0.0 ? LOS_DEFAULT_HEIGHT : other->GetSize()) / 2 * SEE_POSITION;

	myloc.x = x;
	myloc.y = y;
	myloc.z = z + (GetSize() == 0.0 ? LOS_DEFAULT_HEIGHT : GetSize()) / 2 * HEAD_POSITION;

#if LOSDEBUG>=5
	LogDebug("LOS from ([{}], [{}], [{}]) to ([{}], [{}], [{}]) sizes: ([{}], [{}])", myloc.x, myloc.y, myloc.z, oloc.x, oloc.y, oloc.z, GetSize(), mobSize);
#endif
	return zone->zonemap->CheckLoS(myloc, oloc);
}

//offensive spell aggro
int32 Mob::CheckAggroAmount(uint16 spell_id, Mob *target, bool is_proc)
{
	if (IsNoDetrimentalSpellAggroSpell(spell_id)) {
		return 0;
	}

	int32 aggro_amount = 0;
	int32 non_modified_aggro = 0;
	uint16 mob_level = GetLevel();
	bool dispel = false;
	bool on_hatelist = target ? target->CheckAggro(this) : false;
	int proc_cap = RuleI(Aggro, MaxScalingProcAggro);
	int64 hate_cap = is_proc && proc_cap != -1 ? proc_cap : 1200;

	int64 target_hp = target ? target->GetMaxHP() : 18000; // default to max
	int64 default_aggro = 25;
	if (target_hp >= 18000) { // max
		default_aggro = hate_cap;
	} else if (target_hp >= 390) { // min, 390 is the first number with int division that is 26
		default_aggro = target_hp / 15;
	}

	for (int o = 0; o < EFFECT_COUNT; o++) {
		switch (spells[spell_id].effect_id[o]) {
			case SpellEffect::CurrentHPOnce:
			case SpellEffect::CurrentHP: {
				int64 val = CalcSpellEffectValue_formula(spells[spell_id].formula[o], spells[spell_id].base_value[o], spells[spell_id].max_value[o], mob_level, spell_id);
				if(val < 0) {
					aggro_amount -= val;
				}
				break;
			}
			case SpellEffect::MovementSpeed: {
				int64 val = CalcSpellEffectValue_formula(spells[spell_id].formula[o], spells[spell_id].base_value[o], spells[spell_id].max_value[o], mob_level, spell_id);
				if (val < 0) {
					aggro_amount += default_aggro;
				}
				break;
			}
			case SpellEffect::AttackSpeed:
			case SpellEffect::AttackSpeed2:
			case SpellEffect::AttackSpeed3: {
				int64 val = CalcSpellEffectValue_formula(spells[spell_id].formula[o], spells[spell_id].base_value[o], spells[spell_id].max_value[o], mob_level, spell_id);
				if (val < 100) {
					aggro_amount += default_aggro;
				}
				break;
			}
			case SpellEffect::Stun:
			case SpellEffect::Blind:
			case SpellEffect::Mez:
			case SpellEffect::Charm:
			case SpellEffect::Fear:
			case SpellEffect::Fearstun:
				aggro_amount += default_aggro;
				break;
			case SpellEffect::Root:
				aggro_amount += 10;
				break;
			case SpellEffect::ACv2:
			case SpellEffect::ArmorClass: {
				int64 val = CalcSpellEffectValue_formula(spells[spell_id].formula[o], spells[spell_id].base_value[o], spells[spell_id].max_value[o], mob_level, spell_id);
				if (val < 0) {
					aggro_amount += default_aggro;
				}
				break;
			}
			case SpellEffect::ATK:
			case SpellEffect::ResistMagic:
			case SpellEffect::ResistFire:
			case SpellEffect::ResistCold:
			case SpellEffect::ResistPoison:
			case SpellEffect::ResistDisease:
			case SpellEffect::STR:
			case SpellEffect::STA:
			case SpellEffect::DEX:
			case SpellEffect::AGI:
			case SpellEffect::INT:
			case SpellEffect::WIS:
			case SpellEffect::CHA: {
				int64 val = CalcSpellEffectValue_formula(spells[spell_id].formula[o], spells[spell_id].base_value[o], spells[spell_id].max_value[o], mob_level, spell_id);
				if (val < 0) {
					aggro_amount += 10;
				}
				break;
			}
			case SpellEffect::ResistAll: {
				int64 val = CalcSpellEffectValue_formula(spells[spell_id].formula[o], spells[spell_id].base_value[o], spells[spell_id].max_value[o], mob_level, spell_id);
				if (val < 0) {
					aggro_amount += 50;
				}
				break;
			}
			case SpellEffect::AllStats: {
				int64 val = CalcSpellEffectValue_formula(spells[spell_id].formula[o], spells[spell_id].base_value[o], spells[spell_id].max_value[o], mob_level, spell_id);
				if (val < 0) {
					aggro_amount += 70;
				}
				break;
			}
			case SpellEffect::BardAEDot:
				aggro_amount += 10;
				break;
			case SpellEffect::SpinTarget:
			case SpellEffect::Amnesia:
			case SpellEffect::Silence:
			case SpellEffect::Destroy:
				aggro_amount += default_aggro;
				break;
			// unsure -- leave them this for now
			case SpellEffect::Harmony:
			case SpellEffect::CastingLevel:
			case SpellEffect::MeleeMitigation:
			case SpellEffect::CriticalHitChance:
			case SpellEffect::AvoidMeleeChance:
			case SpellEffect::RiposteChance:
			case SpellEffect::DodgeChance:
			case SpellEffect::ParryChance:
			case SpellEffect::DualWieldChance:
			case SpellEffect::DoubleAttackChance:
			case SpellEffect::MeleeSkillCheck:
			case SpellEffect::HitChance:
			case SpellEffect::DamageModifier:
			case SpellEffect::MinDamageModifier:
			case SpellEffect::IncreaseBlockChance:
			case SpellEffect::Accuracy:
			case SpellEffect::DamageShield:
			case SpellEffect::SpellDamageShield:
			case SpellEffect::ReverseDS: {
				aggro_amount += mob_level * 2;
				break;
			}
			// unsure -- leave them this for now
			case SpellEffect::CurrentMana:
			case SpellEffect::ManaRegen_v2:
			case SpellEffect::ManaPool:
			case SpellEffect::CurrentEndurance: {
				int64 val = CalcSpellEffectValue_formula(spells[spell_id].formula[o], spells[spell_id].base_value[o], spells[spell_id].max_value[o], mob_level, spell_id);
				if (val < 0) {
					aggro_amount -= val * 2;
				}
				break;
			}
			case SpellEffect::CancelMagic:
			case SpellEffect::DispelDetrimental:
			case SpellEffect::DispelBeneficial:
				dispel = true;
				break;
			case SpellEffect::ReduceHate:
			case SpellEffect::InstantHate:
				non_modified_aggro = CalcSpellEffectValue_formula(spells[spell_id].formula[o], spells[spell_id].base_value[o], spells[spell_id].max_value[o], mob_level, spell_id);
				break;
		}
	}

	if (IsBardSong(spell_id) && aggro_amount > RuleI(Aggro, BardAggroCap)) {
		aggro_amount = RuleI(Aggro, BardAggroCap);
	}

	if (dispel && target && target->GetHateAmount(this) < 100) {
		aggro_amount += 50;
	}

	if (spells[spell_id].hate_added != 0) { // overrides the hate (ex. tash), can be negative.
		aggro_amount = spells[spell_id].hate_added;
	}

	if (GetOwner() && IsPet() && aggro_amount > 0) {
		aggro_amount = aggro_amount * RuleI(Aggro, PetSpellAggroMod) / 100;
	}

	// hate focus ignored on first action for some reason
	if (!on_hatelist && aggro_amount > 0) {
		int HateMod = RuleI(Aggro, SpellAggroMod);
		HateMod += GetFocusEffect(focusSpellHateMod, spell_id);
		aggro_amount = (aggro_amount * HateMod) / 100;
	}

	// initial aggro gets a bonus 100 besides for dispel or hate override
	// We add this 100 in AddToHateList so we need to account for the oddities here
	if (dispel && spells[spell_id].hate_added > 0 && !on_hatelist) {
		aggro_amount -= 100;
	}

	return aggro_amount + spells[spell_id].bonus_hate + non_modified_aggro;
}

//healing and buffing aggro
int32 Mob::CheckHealAggroAmount(uint16 spell_id, Mob *target, uint32 heal_possible)
{
	int32 AggroAmount = 0;
	auto target_level = target ? target->GetLevel() : 1;
	bool ignore_default_buff = false; // rune/hot don't use the default 9, HP buffs that heal (virtue) do use the default

	for (int o = 0; o < EFFECT_COUNT; o++) {
		switch (spells[spell_id].effect_id[o]) {
			case SpellEffect::CurrentHP:
			case SpellEffect::PercentalHeal:
			{
			if (heal_possible == 0) {
				AggroAmount += 1;
				break;
			}
			// hate based on base healing power of the spell
			int64 val = CalcSpellEffectValue_formula(spells[spell_id].formula[o],
							 spells[spell_id].base_value[o], spells[spell_id].max_value[o], GetLevel(), spell_id);
			if (val > 0) {
				if (heal_possible < val)
					val = heal_possible; // capped to amount healed
				val = 2 * val / 3; // 3:2 ratio

				if (target_level > 50 && val > 1500)
					val = 1500; // target 51+ seems ~1500
				else if (target_level <= 50 && val > 800)
					val = 800; // per live patch notes, capped to 800
			}
			AggroAmount += std::max(val, (int64)1);
			break;
		}
		case SpellEffect::Rune:
			AggroAmount += CalcSpellEffectValue_formula(spells[spell_id].formula[o],
							 spells[spell_id].base_value[o], spells[spell_id].max_value[o], GetLevel(), spell_id) * 2;
			ignore_default_buff = true;
			break;
		case SpellEffect::HealOverTime:
			AggroAmount += 10;
			ignore_default_buff = true;
			break;
		default:
			break;
		}
	}

	if (GetOwner() && IsPet()) {
		AggroAmount = AggroAmount * RuleI(Aggro, PetSpellAggroMod) / 100;
	}

	if (!ignore_default_buff && IsBuffSpell(spell_id) && IsBeneficialSpell(spell_id)) {
		AggroAmount = IsBardSong(spell_id) ? 2 : 9;
	}

	// overrides the hate (ex. Healing Splash), can be negative (but function will return 0).
	if (spells[spell_id].hate_added != 0) {
		AggroAmount = spells[spell_id].hate_added;
	}

	if (AggroAmount > 0) {
		int HateMod = RuleI(Aggro, SpellAggroMod);
		HateMod += GetFocusEffect(focusSpellHateMod, spell_id);

		AggroAmount = (AggroAmount * HateMod) / 100;
	}

	return std::max(0, AggroAmount + spells[spell_id].bonus_hate); //Bonus Hate from spells like Aurora of Morrow
}

void Mob::AddFeignMemory(Mob* attacker) {
	if (feign_memory_list.empty() && AI_feign_remember_timer != nullptr) {
		AI_feign_remember_timer->Start(AIfeignremember_delay);
	}

	if (attacker) {
		feign_memory_list.insert(attacker->GetID());
	}
}

void Mob::RemoveFromFeignMemory(Mob* attacker) {

	if (!attacker) {
		return;
	}

	feign_memory_list.erase(attacker->GetID());
	if (feign_memory_list.empty() && AI_feign_remember_timer != nullptr) {
		AI_feign_remember_timer->Disable();
	}
	if(feign_memory_list.empty())
	{
		minLastFightingDelayMoving = RuleI(NPC, LastFightingDelayMovingMin);
		maxLastFightingDelayMoving = RuleI(NPC, LastFightingDelayMovingMax);
		if (AI_feign_remember_timer != nullptr) {
			AI_feign_remember_timer->Disable();
		}
	}
}

void Mob::ClearFeignMemory() {
	auto remembered_feigned_mobid = feign_memory_list.begin();
	while (remembered_feigned_mobid != feign_memory_list.end())
	{
		Mob* remembered_mob = entity_list.GetMob(*remembered_feigned_mobid);
		if (remembered_mob && remembered_mob->IsClient()) { //Still in zone
			remembered_mob->CastToClient()->RemoveXTarget(this, false);
		}
		++remembered_feigned_mobid;
	}

	feign_memory_list.clear();
	minLastFightingDelayMoving = RuleI(NPC, LastFightingDelayMovingMin);
	maxLastFightingDelayMoving = RuleI(NPC, LastFightingDelayMovingMax);
	if (AI_feign_remember_timer != nullptr) {
		AI_feign_remember_timer->Disable();
	}
}

bool Mob::IsOnFeignMemory(Mob *attacker) const
{
	if (!attacker) {
		return 0;
	}

	return feign_memory_list.find(attacker->GetID()) != feign_memory_list.end();
}

bool Mob::PassCharismaCheck(Mob* caster, uint16 spell_id) {

	/*
	Charm formula is correct based on over 50 hours of personal live parsing - Kayen
	Charisma ONLY effects the initial resist check when charm is cast with 10 CHA = -1 Resist mod up to 255 CHA (min ~ 75 CHA)
	Charisma DOES NOT extend charm durations.
	Base effect value of charm spells in the spell file DOES NOT effect duration OR resist rate (unclear if does anything)
	Charm has a lower limit of 5% chance to break per tick, regardless of resist modifiers / level difference.
	*/

	if(!caster) return false;

	if(spells[spell_id].resist_difficulty <= -600)
		return true;

	float resist_check = 0;

	if(IsCharmSpell(spell_id)) {

		if (spells[spell_id].no_resist) //If charm spell has this set(-1), it can not break till end of duration.
			return true;

		//1: The mob has a default 25% chance of being allowed a resistance check against the charm.
		if (zone->random.Int(0, 99) > RuleI(Spells, CharmBreakCheckChance))
			return true;

		if (RuleB(Spells, CharismaCharmDuration))
			resist_check = ResistSpell(spells[spell_id].resist_type, spell_id, caster,false,0,true,true);
		else
			resist_check = ResistSpell(spells[spell_id].resist_type, spell_id, caster, false,0, false, true);

		//2: The mob makes a resistance check against the charm
		if (resist_check == 100) {
			return true;
		} else {
			if (caster->IsOfClientBot()) {
				//3: At maxed ability, Total Domination has a 50% chance of preventing the charm break that otherwise would have occurred.
				int16 TotalDominationBonus = caster->aabonuses.CharmBreakChance + caster->spellbonuses.CharmBreakChance + caster->itembonuses.CharmBreakChance;

				if (zone->random.Int(0, 99) < TotalDominationBonus) {
					return true;
				}
			}
		}
	}

	else
	{
		// Assume this is a harmony/pacify spell
		// If 'Lull' spell resists, do a second resist check with a charisma modifier AND regular resist checks. If resists agian you gain aggro.
		resist_check = ResistSpell(spells[spell_id].resist_type, spell_id, caster, false,0,true);
		if (resist_check == 100)
			return true;
	}

	return false;
}

void Mob::RogueEvade(Mob *other)
{
	int64 amount = other->GetHateAmount(this) * zone->random.Int(40, 70) / 100;
	other->SetHateAmountOnEnt(this, std::max(static_cast<int64>(100), amount));

	return;
}
