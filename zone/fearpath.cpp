/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

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

#include "common/rulesys.h"
#include "zone/map.h"
#include "zone/water_map.h"
#include "zone/zone.h"

extern Zone* zone;

#define FEAR_PATHING_DEBUG

int Mob::GetFleeRatio(Mob* other)
{
	int flee_ratio = GetSpecialAbility(SpecialAbility::FleePercent); // if a special SpecialAbility::FleePercent exists
	Mob *hate_top = GetHateTop();

	if (other != nullptr) {
		hate_top = other;
	}

	if (!hate_top) {
		return 0;
	}

	// If no special flee_percent check for Gray or Other con rates
	if (flee_ratio == 0) {
		flee_ratio = RuleI(Combat, FleeHPRatio);
		if (GetLevelCon(hate_top->GetLevel(), GetLevel()) == ConsiderColor::Gray && RuleB(Combat, FleeGray) &&
			GetLevel() <= RuleI(Combat, FleeGrayMaxLevel)) {
			flee_ratio = RuleI(Combat, FleeGrayHPRatio);
			LogFlee("Mob [{}] using combat flee gray flee_ratio [{}]", GetCleanName(), flee_ratio);
		}
	}

	return flee_ratio;
}

//this is called whenever we are damaged to process possible fleeing
void Mob::CheckFlee()
{
	if (IsPet() || IsCasting() || GetHP() == 0 || GetBodyType() == BodyType::Undead || (IsNPC() && CastToNPC()->IsUnderwaterOnly())) {
		return;
	}

	//if were already fleeing, we only need to check speed.  Speed changes will trigger pathing updates.
	if (flee_mode && currently_fleeing) {
		int flee_speed = GetFearSpeed();
		if (flee_speed < 1) {
			flee_speed = 0;
		}

		SetRunAnimSpeed(flee_speed);

		if (IsMoving() && flee_speed < 1) {
			StopNavigation();
		}

		return;
	}

	//dont bother if we are immune to fleeing
	if (GetSpecialAbility(SpecialAbility::FleeingImmunity) || spellbonuses.ImmuneToFlee) {
		LogFlee("Mob [{}] is immune to fleeing via special ability or spell bonus", GetCleanName());
		return;
	}

	int hp_ratio   = GetIntHPRatio();
	int flee_ratio = GetFleeRatio();
	Mob *hate_top  = GetHateTop();

	// Sanity Check for race conditions
	if(!hate_top) {
		//this should never happen...
		StartFleeing();
		return;
	}

	bool mob_has_low_enough_health_to_flee = hp_ratio >= flee_ratio;
	if (mob_has_low_enough_health_to_flee) {
		LogFlee(
			"Mob [{}] does not have low enough health to flee | hp_ratio [{}] flee_ratio [{}]",
			GetCleanName(),
			hp_ratio,
			flee_ratio
		);
		return;
	}

	// Sanity Check this should never happen...
	if (!hate_top) {
		currently_fleeing = true;
		StartFleeing();
		return;
	}

	int other_ratio = hate_top->GetIntHPRatio();
	// If the Client is nearing death the NPC will not flee and instead try to kill the client.
	if (other_ratio < 20) {
		return;
	}

	// Flee Chance checking based on con.
	uint32 con = GetLevelCon(hate_top->GetLevel(), GetLevel());
	int    flee_chance;
	switch (con) {
		//these values are not 100% researched
		case ConsiderColor::Gray:
			flee_chance = 100;
			break;
		case ConsiderColor::Green:
			flee_chance = 90;
			break;
		case ConsiderColor::LightBlue:
			flee_chance = 90;
			break;
		case ConsiderColor::DarkBlue:
			flee_chance = 80;
			break;
		default:
			flee_chance = 70;
			break;
	}

	LogFlee(
		"Post con-switch | Mob [{}] con [{}] hp_ratio [{}] flee_ratio [{}] flee_chance [{}]",
		GetCleanName(),
		con,
		hp_ratio,
		flee_ratio,
		flee_chance
	);

	// If we got here we are allowed to roll on flee chance if there is not other hated NPC's in the area.
	// SpecialAbility::AlwaysFlee, skip roll
	// if FleeIfNotAlone is true, we skip alone check
	// roll chance
	if (GetSpecialAbility(SpecialAbility::AlwaysFlee) ||
		((RuleB(Combat, FleeIfNotAlone) || entity_list.FleeAllyCount(hate_top, this) == 0) &&
		 zone->random.Roll(flee_chance))) {

		LogFlee(
			"Passed all checks to flee | Mob [{}] con [{}] hp_ratio [{}] flee_ratio [{}] flee_chance [{}]",
			GetCleanName(),
			con,
			hp_ratio,
			flee_ratio,
			flee_chance
		);

		currently_fleeing = true;
		StartFleeing();
	}
}

void Mob::StopFleeing()
{
	if (!flee_mode) {
		return;
	}

	flee_mode = false;

	//see if we are legitimately feared or blind now
	if (!spellbonuses.IsFeared && !IsBlind()) {
		currently_fleeing = false;
		StopNavigation();
	}
}

void Mob::FleeInfo(Mob* client)
{
	float other_ratio = client->GetHPRatio();
	bool wontflee = false;
	std::string reason;
	std::string flee;

	int allycount = entity_list.FleeAllyCount(client, this);

	if (flee_mode && currently_fleeing) {
		wontflee = true;
		reason = "NPC is already fleeing!";
	} else if (GetSpecialAbility(SpecialAbility::FleeingImmunity)) {
		wontflee = true;
		reason = "NPC is immune to fleeing.";
	} else if (other_ratio < 20) {
		wontflee = true;
		reason = "Player has low health.";
	} else if (GetSpecialAbility(SpecialAbility::AlwaysFlee)) {
		flee = "NPC has ALWAYS_FLEE set.";
	} else if (RuleB(Combat, FleeIfNotAlone) || (!RuleB(Combat, FleeIfNotAlone) && allycount == 0)) {
		flee = "NPC has no allies nearby or the rule to flee when not alone is enabled.";
	} else {
		wontflee = true;
		reason = "NPC likely has allies nearby.";
	}


	if (!wontflee) {
		client->Message(Chat::Green, "%s will flee at %d percent because %s", GetName(), GetFleeRatio(client), flee.c_str());
	} else {
		client->Message(Chat::Red, "%s will not flee because %s", GetName(), reason.c_str());
	}

	client->Message(Chat::Default, "NPC ally count %d", allycount);
}

void Mob::ProcessFlee()
{
	if (!flee_mode) {
		return;
	}

	//Stop fleeing if effect is applied after they start to run.
	//When ImmuneToFlee effect fades it will turn fear back on and check if it can still flee.
	if (flee_mode && (GetSpecialAbility(SpecialAbility::FleeingImmunity) || spellbonuses.ImmuneToFlee) &&
		!spellbonuses.IsFeared && !spellbonuses.IsBlind) {
		currently_fleeing = false;
		return;
	}

	Mob *hate_top = GetHateTop();
	bool dying = GetIntHPRatio() < GetFleeRatio();

	// We have stopped fleeing for an unknown reason (couldn't find a node is possible) restart.
	if (flee_mode && !currently_fleeing) {
		if(dying) {
			StartFleeing();
		}
	}

	//see if we are still dying, if so, do nothing
	if (dying) {
		return;
	}

	//we are not dying anymore, check to make sure we're not blind or feared and cancel flee.
	StopFleeing();
}

void Mob::CalculateNewFearpoint()
{
	// blind waypoint logic isn't the same as fear's.  Has a chance to run toward the player
	// chance is very high if the player is moving, otherwise it's low
	if (IsBlind() && !IsFeared() && GetTarget()) {
		int roll = 20;
		if (GetTarget()->GetCurrentSpeed() > 0.1f || (GetTarget()->IsClient() && GetTarget()->animation != 0)) {
			roll = 80;
		}

		if (zone->random.Roll(roll)) {
			m_FearWalkTarget = glm::vec3(GetTarget()->GetPosition());
			currently_fleeing = true;
			return;
		}
	}

	if (RuleB(Pathing, Fear) && zone->pathing) {
		glm::vec3 Node;
		int flags = PathingNotDisabled ^ PathingZoneLine;

		if (IsNPC() && CastToNPC()->IsUnderwaterOnly() && !zone->IsWaterZone(GetZOffset())) {
			Node = glm::vec3(0.0f);
		} else {
			Node = zone->pathing->GetRandomLocation(glm::vec3(GetX(), GetY(), GetZOffset()), flags);
		}

		if (Node.x != 0.0f || Node.y != 0.0f || Node.z != 0.0f) {
			Node.z = GetFixedZ(Node);
			PathfinderOptions opts;
			opts.smooth_path = true;
			opts.step_size = RuleR(Pathing, NavmeshStepSize);
			opts.offset = GetZOffset();
			opts.flags = flags;
			auto partial = false;
			auto stuck = false;
			auto route = zone->pathing->FindPath(
				glm::vec3(GetX(), GetY(), GetZOffset()),
				glm::vec3(Node.x, Node.y, Node.z),
				partial,
				stuck,
				opts
			);
			glm::vec3 last_good_loc = Node;
			int route_size = route.size();
			int route_count = 0;
			bool have_los = true;

			if (route_size == 2) {
				// FindPath() often fails to compute a route in some places, so to prevent running through walls we need to check LOS on all 2 node routes
				// size 2 route usually means FindPath() bugged out.  sometimes it returns locs outside the geometry
				if (CheckLosFN(Node.x, Node.y, Node.z, 6.0)) {
					LogPathingDetail("Direct route to fearpoint [{}], [{}], [{}] calculated for [{}]", last_good_loc.x, last_good_loc.y, last_good_loc.z, GetName());
					m_FearWalkTarget = last_good_loc;
					currently_fleeing = true;
					return;
				} else {
					LogPathingDetail("FindRoute() returned single hop route to destination without LOS: [{}], [{}], [{}] for [{}]", last_good_loc.x, last_good_loc.y, last_good_loc.z, GetName());
				}
				// use fallback logic if LOS fails
			} else if (!stuck) {
				// check route for LOS failures to prevent mobs ending up outside of playable area
				// only checking the last few hops because LOS will often fail in a valid route which can result in mobs getting undesirably trapped
				auto iter = route.begin();
				glm::vec3 previous_pos(GetX(), GetY(), GetZOffset());
				while (iter != route.end() && have_los == true) {
					auto &current_node = (*iter);
					iter++;
					route_count++;

					if (iter == route.end()) {
						continue;
					}

					previous_pos = current_node.pos;
					auto &next_node = (*iter);

					if (next_node.teleport) {
						continue;
					}

					if ((route_size - route_count) < 5 && !zone->zonemap->CheckLoS(previous_pos, next_node.pos)) {
						have_los = false;
						break;
					} else {
						last_good_loc = next_node.pos;
					}
				}

				if (have_los || route_count > 2) {
					if (have_los) {
						LogPathingDetail("Route to fearpoint [{}], [{}], [{}] calculated for [{}]; route size: [{}]", last_good_loc.x, last_good_loc.y, last_good_loc.z, GetName(), route_size);
					} else {
						LogPathingDetail("Using truncated route to fearpoint [{}], [{}], [{}] for [{}]; node count: [{}]; route size [{}]", last_good_loc.x, last_good_loc.y, last_good_loc.z, GetName(), route_count, route_size);
					}

					m_FearWalkTarget = last_good_loc;
					currently_fleeing = true;
					return;
				}
			}
		}
	}

	// fallback logic if pathing system can't be used
	bool inliquid = zone->HasWaterMap() && zone->watermap->InLiquid(glm::vec3(GetPosition())) || zone->IsWaterZone(GetZ());
	bool stay_inliquid = (inliquid && IsNPC() && CastToNPC()->IsUnderwaterOnly());
	bool levitating = IsClient() && (FindType(SpellEffect::Levitate) || flymode != GravityBehavior::Ground);
	bool open_outdoor_zone = !zone->CanCastOutdoor() && !zone->IsCity();

	int loop = 0;
	float ranx, rany, ranz;
	currently_fleeing = false;
	glm::vec3 myloc(GetX(), GetY(), GetZ());
	glm::vec3 myceil = myloc;
	float ceil = zone->zonemap->FindCeiling(myloc, &myceil);

	if (ceil != BEST_Z_INVALID) {
		ceil -= 1.0f;
	}

	while (loop < 100) { //Max 100 tries
		int ran = 250 - (loop * 2);
		loop++;

		if (open_outdoor_zone && loop < 20) { // try a distant loc first; other way will likely pick a close loc
			ranx = zone->random.Int(0, ran);
			rany = zone->random.Int(0, ran);
			if (ranx + rany < 200) {
				continue;
			}

			ranx = GetX() + (zone->random.Int(0, 1) == 1 ? ranx : -ranx);
			rany = GetY() + (zone->random.Int(0, 1) == 1 ? rany : -rany);
		} else {
			ranx = GetX() + zone->random.Int(0, ran - 1) - zone->random.Int(0, ran - 1);
			rany = GetY() + zone->random.Int(0, ran - 1) - zone->random.Int(0, ran - 1);
		}

		ranz = BEST_Z_INVALID;
		glm::vec3 newloc(ranx, rany, ceil != BEST_Z_INVALID ? ceil : GetZ());

		if (stay_inliquid || levitating || (loop > 50 && inliquid)) {
			if (zone->zonemap->CheckLoS(myloc, newloc)) {
				ranz = GetZ();
				currently_fleeing = true;
				break;
			}
		} else {
			if (ceil != BEST_Z_INVALID) {
				ranz = zone->zonemap->FindGround(newloc, &myceil);
			} else {
				ranz = zone->zonemap->FindBestZ(newloc, &myceil);
			}

			if (ranz != BEST_Z_INVALID) {
				ranz = SetBestZ(ranz);
			}
		}

		if (ranz == BEST_Z_INVALID) {
			continue;
		}

		float fdist = ranz - GetZ();
		if (fdist >= -50 && fdist <= 50 && CheckCoordLosNoZLeaps(GetX(), GetY(), GetZ(), ranx, rany, ranz)) {
			currently_fleeing = true;
			break;
		}
	}

	if (currently_fleeing) {
		m_FearWalkTarget = glm::vec3(ranx, rany, ranz);
		LogPathingDetail("Non-pathed fearpoint [{}], [{}], [{}] selected for [{}]", ranx, rany, ranz, GetName());
	}

	return;
}

