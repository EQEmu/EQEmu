/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2004 EQEMu Development Team (http://eqemu.org)

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

#include "npc.h"

#include "common/eqemu_logsys.h"
#include "common/misc_functions.h"
#include "common/repositories/grid_entries_repository.h"
#include "common/repositories/grid_repository.h"
#include "common/repositories/spawn2_repository.h"
#include "common/rulesys.h"
#include "common/strings.h"
#include "zone/bot.h"
#include "zone/fastmath.h"
#include "zone/map.h"
#include "zone/mob_movement_manager.h"
#include "zone/quest_parser_collection.h"
#include "zone/water_map.h"

#include <cmath>

extern FastMath g_Math;

struct wp_distance
{
	float dist;
	int index;
};

void NPC::AI_SetRoambox(float max_distance, float roam_distance_variance, uint32 delay, uint32 min_delay) {
	AI_SetRoambox(
		max_distance,
		GetX() + roam_distance_variance,
		GetX() - roam_distance_variance,
		GetY() + roam_distance_variance,
		GetY() - roam_distance_variance,
		delay,
		min_delay
	);
}

void NPC::AI_SetRoambox(
	float distance,
	float max_x,
	float min_x,
	float max_y,
	float min_y,
	uint32 delay,
	uint32 min_delay
)
{
	m_roambox.distance  = distance;
	m_roambox.max_x     = max_x;
	m_roambox.min_x     = min_x;
	m_roambox.max_y     = max_y;
	m_roambox.min_y     = min_y;
	m_roambox.dest_x    = max_x + 1; // this will trigger a recalc
	m_roambox.delay     = delay;
	m_roambox.min_delay = min_delay;
}

void NPC::DisplayWaypointInfo(Client *client) {
	client->Message(
		Chat::White,
		fmt::format(
			"Waypoint Info for {} ({}) | Grid: {} Waypoint: {} of {}",
			GetCleanName(),
			GetID(),
			GetGrid(),
			GetCurWp(),
			GetMaxWp()
		).c_str()
	);

	client->Message(
		Chat::White,
		fmt::format(
			"Waypoint Info for {} ({}) | Spawn Group: {} Spawn Point: {}",
			GetCleanName(),
			GetID(),
			GetSpawnGroupId(),
			GetSpawnPointID()
		).c_str()
	);


	for (const auto& current_waypoint : Waypoints) {
		client->Message(
			Chat::White,
			fmt::format(
				"Waypoint {}{} | XYZ: {:.2f}, {:.2f}, {:.2f} Heading: {:.2f}{}",
				current_waypoint.index,
				current_waypoint.centerpoint ? " (Center)" : "",
				current_waypoint.x,
				current_waypoint.y,
				current_waypoint.z,
				current_waypoint.heading,
				(
					current_waypoint.pause ?
					fmt::format(
						" {} ({})",
						Strings::SecondsToTime(current_waypoint.pause),
						current_waypoint.pause
					) :
					""
				)
			).c_str()
		);
	}
}

void NPC::StopWandering()
{	// stops a mob from wandering, takes him off grid and sends him back to spawn point
	roamer = false;
	CastToNPC()->SetGrid(0);
	StopNavigation();
	LogPathing("Stop Wandering requested");
	return;
}

void NPC::ResumeWandering()
{	// causes wandering to continue - overrides waypoint pause timer and PauseWandering()
	if (!IsNPC())
		return;
	if (GetGrid() != 0)
	{
		if (GetGrid() < 0)
		{	// we were paused by a quest
			AI_walking_timer->Disable();
			SetGrid(0 - GetGrid());
			if (cur_wp == EQ::WaypointStatus::QuestControlGrid)
			{	// got here by a MoveTo()
				cur_wp = save_wp;
				UpdateWaypoint(cur_wp);	// have him head to last destination from here
			}
			LogPathing("Resume Wandering requested. Grid [{}], wp [{}]", GetGrid(), cur_wp);
		}
		else if (AI_walking_timer->Enabled())
		{	// we are at a waypoint paused normally
			LogPathing("Resume Wandering on timed pause. Grid [{}], wp [{}]", GetGrid(), cur_wp);
			AI_walking_timer->Trigger();	// disable timer to end pause now
		}
		else
		{
			LogPathing("NPC not paused - can't resume wandering: [{}]", (unsigned long)GetNPCTypeID());
			return;
		}

		if (m_CurrentWayPoint.x == GetX() && m_CurrentWayPoint.y == GetY())
		{	// are we we at a waypoint? if so, trigger event and start to next
			CalculateNewWaypoint();
			SetAppearance(eaStanding, false);

			if (parse->HasQuestSub(GetNPCTypeID(), EVENT_WAYPOINT_DEPART)) {
				parse->EventNPC(EVENT_WAYPOINT_DEPART, this, nullptr, std::to_string(cur_wp), 0);
			}
		}	// if not currently at a waypoint, we continue on to the one we were headed to before the stop
	}
	else
	{
		LogPathing("NPC not on grid - can't resume wandering: [{}]", (unsigned long)GetNPCTypeID());
	}
	return;
}

void NPC::PauseWandering(int pausetime)
{	// causes wandering to stop but is resumable
	// 0 pausetime means pause until resumed
	// otherwise automatically resume when time is up
	if (GetGrid() != 0) {
		moving = false;
		DistractedFromGrid = true;
		LogPathing("Paused Wandering requested. Grid [{}]. Resuming in [{}] seconds (0=not until told)", GetGrid(), pausetime);
		StopNavigation();
		if (pausetime < 1) {	// negative grid number stops him dead in his tracks until ResumeWandering()
			SetGrid(0 - GetGrid());
		}
		else {	// specified waiting time, he'll resume after that
			AI_walking_timer->Start(pausetime * 1000); // set the timer
		}
	}
	else {
		LogPathing("NPC not on grid - can't pause wandering: [{}]", (unsigned long)GetNPCTypeID());
	}
	return;
}

void NPC::MoveTo(const glm::vec4 &position, bool saveguardspot)
{    // makes mob walk to specified location
	if (!AI_walking_timer) {
		return;
	}
	if (IsNPC() && GetGrid() != 0) {    // he is on a grid
		if (GetGrid() < 0) {    // currently stopped by a quest command
			SetGrid(0 - GetGrid());    // get him moving again
			LogAIDetail("MoveTo during quest wandering. Canceling quest wandering and going back to grid [{}] when MoveTo is done", GetGrid());
		}
		AI_walking_timer->Disable();    // disable timer in case he is paused at a wp
		if (cur_wp >= 0) {    // we've not already done a MoveTo()
			save_wp = cur_wp;    // save the current waypoint
			cur_wp  = EQ::WaypointStatus::QuestControlGrid;
		}
		LogAIDetail("MoveTo [{}], pausing regular grid wandering. Grid [{}], save_wp [{}]",
			to_string(static_cast<glm::vec3>(position)).c_str(),
			-GetGrid(),
			save_wp);
	}
	else {    // not on a grid
		roamer  = true;
		save_wp = 0;
		cur_wp  = EQ::WaypointStatus::QuestControlNoGrid;
		LogAIDetail("MoveTo [{}] without a grid", to_string(static_cast<glm::vec3>(position)).c_str());
	}

	glm::vec3 dest(position);

	m_CurrentWayPoint = position;
	m_CurrentWayPoint.z = GetFixedZ(dest);

	if (saveguardspot) {
		m_GuardPoint = m_CurrentWayPoint;

		if (m_GuardPoint.w == 0) {
			m_GuardPoint.w = 0.0001;
		}        //hack to make IsGuarding simpler

		if (m_GuardPoint.w == -1)
			m_GuardPoint.w = CalculateHeadingToTarget(position.x, position.y);

		LogAIDetail("Setting guard position to [{}]", to_string(static_cast<glm::vec3>(m_GuardPoint)).c_str());
	}

	cur_wp_pause        = 0;
	time_until_can_move = 0;
	if (AI_walking_timer->Enabled()) {
		AI_walking_timer->Start(100);
	}
}

void NPC::UpdateWaypoint(int wp_index)
{
	if (wp_index >= static_cast<int>(Waypoints.size())) {
		LogAIDetail("Update to waypoint [{}] failed. Not found", wp_index);
		return;
	}
	std::vector<wplist>::iterator cur;
	cur = Waypoints.begin();
	cur += wp_index;

	m_CurrentWayPoint = glm::vec4(cur->x, cur->y, cur->z, cur->heading);
	cur_wp_pause = cur->pause;
	LogAIDetail("Next waypoint [{}]: ({}, {}, {}, {})", wp_index, m_CurrentWayPoint.x, m_CurrentWayPoint.y, m_CurrentWayPoint.z, m_CurrentWayPoint.w);

}

void NPC::CalculateNewWaypoint()
{
	int old_wp = cur_wp;
	bool reached_end = false;
	bool reached_beginning = false;
	if (cur_wp == max_wp - 1) //cur_wp starts at 0, max_wp starts at 1.
		reached_end = true;
	if (cur_wp == 0)
		reached_beginning = true;

	switch (wandertype)
	{
	case GridCircular:
	{
		if (reached_end)
			cur_wp = 0;
		else
			cur_wp = cur_wp + 1;
		break;
	}
	case GridRandom10:
	{
		std::list<wplist> closest;
		GetClosestWaypoints(closest, 10, glm::vec3(GetPosition()));
		auto iter = closest.begin();
		if (closest.size() != 0)
		{
			iter = closest.begin();
			std::advance(iter, zone->random.Int(0, closest.size() - 1));
			cur_wp = (*iter).index;
		}

		break;
	}
	case GridRandom:
	case GridCenterPoint:
	{
		if (wandertype == GridCenterPoint && !reached_beginning)
		{
			cur_wp = 0;
		}
		else
		{
			cur_wp = zone->random.Int(0, Waypoints.size() - 1);
			if (cur_wp == old_wp || (wandertype == GridCenterPoint && cur_wp == 0))
			{
				if (cur_wp == (Waypoints.size() - 1))
				{
					if (cur_wp > 0)
					{
						cur_wp--;
					}
				}
				else if (cur_wp == 0)
				{
					if ((Waypoints.size() - 1) > 0)
					{
						cur_wp++;
					}
				}
			}
		}

		break;
	}
	case GridRandomCenterPoint:
	{
		bool on_center = Waypoints[cur_wp].centerpoint;
		std::vector<wplist> random_waypoints;
		for (auto &wpl : Waypoints)
		{
			if (wpl.index != cur_wp &&
				((on_center && !wpl.centerpoint) || (!on_center && wpl.centerpoint)))
			{
				random_waypoints.push_back(wpl);
			}
		}

		if (random_waypoints.size() == 0)
		{
			cur_wp = 0;
		}
		else
		{
			int windex = zone->random.Roll0(random_waypoints.size());
			cur_wp = random_waypoints[windex].index;
		}

		break;
	}
	case GridPatrol:
	{
		if (reached_end)
			patrol = 1;
		else if (reached_beginning)
			patrol = 0;
		if (patrol == 1)
			cur_wp = cur_wp - 1;
		else
			cur_wp = cur_wp + 1;

		break;
	}
	case GridOneWayRepop:
	case GridOneWayDepop:
	{
		cur_wp = cur_wp + 1;
		break;
	}
	case GridRand5LoS:
	{
		std::list<wplist> closest;
		GetClosestWaypoints(closest, 5, glm::vec3(GetPosition()));

		auto iter = closest.begin();
		while (iter != closest.end())
		{
			if (CheckLosFN((*iter).x, (*iter).y, (*iter).z, GetSize()))
			{
				++iter;
			}
			else
			{
				iter = closest.erase(iter);
			}
		}

		if (closest.size() != 0)
		{
			iter = closest.begin();
			std::advance(iter, zone->random.Int(0, closest.size() - 1));
			cur_wp = (*iter).index;
		}
		break;
	}
	case GridRandomPath: // randomly select a waypoint but follow path to it instead of walk directly to it ignoring walls
	{
		if (Waypoints.size() == 0)
		{
			cur_wp = 0;
		}
		else
		{
			if (cur_wp == patrol) // reutilizing patrol member instead of making new member for this wander type; here we use it to save a random waypoint
			{
				if (!Waypoints[cur_wp].centerpoint)
				{
					// if we have arrived at a waypoint that is NOT a centerpoint, then check for the existence of any centerpoint waypoint
					// if any exists then randomly go to it otherwise go to one that exist.
					std::vector<wplist> random_centerpoints;
					for (auto& wpl : Waypoints)
					{
						if (wpl.index != cur_wp && wpl.centerpoint)
						{
							random_centerpoints.push_back(wpl);
						}
					}

					if (random_centerpoints.size() == 1)
					{
						patrol = random_centerpoints[0].index;
						break;
					}
					else if (random_centerpoints.size() > 1)
					{
						int windex = zone->random.Roll0(random_centerpoints.size());
						patrol = random_centerpoints[windex].index;
						break;
					}
				}

				while (patrol == cur_wp)
				{
					// Setting a negative number in pause of the select waypoints will NOT be included in the group of waypoints to be random.
					// This will cause the NPC to not stop and pause in any of the waypoints that is not part of random waypoints.
					std::vector<wplist> random_waypoints;
					for (auto& wpl : Waypoints)
					{
						if (wpl.index != cur_wp && wpl.pause >= 0 && !wpl.centerpoint)
						{
							random_waypoints.push_back(wpl);
						}
					}
					int windex = zone->random.Roll0(random_waypoints.size());
					patrol = random_waypoints[windex].index;
				}
			}
			if (patrol > cur_wp)
				cur_wp = cur_wp + 1;
			else
				cur_wp = cur_wp - 1;
		}
	}
	}

	// Preserve waypoint setting for quest controlled NPCs
	if (cur_wp < 0)
		cur_wp = old_wp;

	// Check to see if we need to update the waypoint.
	if (cur_wp != old_wp)
		UpdateWaypoint(cur_wp);
}

bool wp_distance_pred(const wp_distance& left, const wp_distance& right)
{
	return left.dist < right.dist;
}

int NPC::GetClosestWaypoint(const glm::vec3& location)
{
	if (Waypoints.size() <= 1)
		return 0;

	int closest = 0;
	float closestDist = 9999999.0f;
	float dist;

	for (int i = 0; i < Waypoints.size(); ++i)
	{
		dist = DistanceSquared(location, glm::vec3(Waypoints[i].x, Waypoints[i].y, Waypoints[i].z));

		if (dist < closestDist)
		{
			closestDist = dist;
			closest = i;
		}
	}
	return closest;
}

// fills wp_list with the closest count number of waypoints
void NPC::GetClosestWaypoints(std::list<wplist> &wp_list, int count, const glm::vec3& location)
{
	wp_list.clear();
	if (Waypoints.size() <= count)
	{
		for (int i = 0; i < Waypoints.size(); ++i)
		{
			wp_list.push_back(Waypoints[i]);
		}
		return;
	}

	std::list<wp_distance> distances;
	for (int i = 0; i < Waypoints.size(); ++i)
	{
		float cur_x = (Waypoints[i].x - location.x);
		cur_x *= cur_x;
		float cur_y = (Waypoints[i].y - location.y);
		cur_y *= cur_y;
		float cur_z = (Waypoints[i].z - location.z);
		cur_z *= cur_z;
		float cur_dist = cur_x + cur_y + cur_z;
		wp_distance w_dist;
		w_dist.dist = cur_dist;
		w_dist.index = i;
		distances.push_back(w_dist);
	}
	distances.sort([](const wp_distance& a, const wp_distance& b) {
		return a.dist < b.dist;
	});

	auto iter = distances.begin();
	for (int i = 0; i < count; ++i)
	{
		wp_list.push_back(Waypoints[(*iter).index]);
		++iter;
	}
}

void NPC::SetWaypointPause()
{
	//Declare time to wait on current WP

	if(cur_wp_pause > 0)
	{
		switch (pausetype)
		{
		case 0: //Random Half
			AI_walking_timer->Start((cur_wp_pause - zone->random.Int(0, cur_wp_pause - 1) / 2) * 1000);
			break;
		case 1: //Full
			AI_walking_timer->Start(cur_wp_pause * 1000);
			break;
		case 2: //Random Full
			AI_walking_timer->Start(zone->random.Int(0, cur_wp_pause - 1) * 1000);
			break;
		}
	}
}

void NPC::SaveGuardSpot(bool ClearGuardSpot) {
	if (ClearGuardSpot) {
		LogAIDetail("Clearing guard order.");
		m_GuardPoint = glm::vec4();
	} else {
		m_GuardPoint = m_Position;

		if (m_GuardPoint.w == 0) {
			m_GuardPoint.w = 0.0001; //hack to make IsGuarding simpler
		}
		LogAIDetail("Setting guard position to [{}]", to_string(static_cast<glm::vec3>(m_GuardPoint)));
	}
}

void NPC::SaveGuardSpot(const glm::vec4 &pos)
{
	m_GuardPoint = pos;

	if (m_GuardPoint.w == 0)
		m_GuardPoint.w = 0.0001;		//hack to make IsGuarding simpler
	LogAIDetail("Setting guard position to [{}]", to_string(static_cast<glm::vec3>(m_GuardPoint)));
}

void NPC::NextGuardPosition() {
	NavigateTo(m_GuardPoint.x, m_GuardPoint.y, m_GuardPoint.z);
	if (IsPositionEqualWithinCertainZ(m_Position, m_GuardPoint, 5.0f))
	{
		if (moved)
		{
			moved = false;
		}
	}
}

float Mob::CalculateDistance(float x, float y, float z) {
	return sqrtf(
		((m_Position.x - x) * (m_Position.x - x)) +
		((m_Position.y - y) * (m_Position.y - y)) +
		((m_Position.z - z) * (m_Position.z - z))
	);
}

float Mob::CalculateDistance(Mob* mob) {
	return sqrtf(
		((m_Position.x - mob->GetX()) * (m_Position.x - mob->GetX())) +
		((m_Position.y - mob->GetY()) * (m_Position.y - mob->GetY())) +
		((m_Position.z - mob->GetZ()) * (m_Position.z - mob->GetZ()))
	);
}

void Mob::WalkTo(float x, float y, float z)
{
	mMovementManager->NavigateTo(this, x, y, z, MovementWalking);
}

void Mob::RunTo(float x, float y, float z)
{
	mMovementManager->NavigateTo(this, x, y, z, MovementRunning);
}

void Mob::NavigateTo(float x, float y, float z)
{
	if (IsRunning()) {
		RunTo(x, y, z);
	}
	else {
		WalkTo(x, y, z);
	}
}

void Mob::RotateTo(float new_heading)
{
	if (IsRunning()) {
		RotateToRunning(new_heading);
	}
	else {
		RotateToWalking(new_heading);
	}
}

void Mob::RotateToWalking(float new_heading)
{
	mMovementManager->RotateTo(this, new_heading, MovementWalking);
}

void Mob::RotateToRunning(float new_heading)
{
	mMovementManager->RotateTo(this, new_heading, MovementRunning);
}

void Mob::StopNavigation() {
	mMovementManager->StopNavigation(this);
}

void NPC::AssignWaypoints(int32 grid_id, int start_wp)
{
	if (grid_id == 0)
		return; // grid ID 0 not supported

	if (grid_id < 0) {
		// Allow setting negative grid values for pausing pathing
		CastToNPC()->SetGrid(grid_id);
		return;
	}

	Waypoints.clear();
	roamer = false;

	auto grid_entry = GridRepository::GetGrid(zone->zone_grids, grid_id);
	if (grid_entry.id == 0) {
		return;
	}

	wandertype = grid_entry.type;
	pausetype  = grid_entry.type2;

	SetGrid(grid_id);	// Assign grid number

	roamer = true;
	max_wp = 0;	// Initialize it; will increment it for each waypoint successfully added to the list

	for (auto &entry : zone->zone_grid_entries) {
		if (entry.gridid == grid_id) {
			wplist new_waypoint{};
			new_waypoint.index       = max_wp;
			new_waypoint.x           = entry.x;
			new_waypoint.y           = entry.y;
			new_waypoint.z           = entry.z;
			new_waypoint.pause       = entry.pause;
			new_waypoint.heading     = entry.heading;
			new_waypoint.centerpoint = entry.centerpoint;

			LogPathing(
				"Loading Grid [{}] number [{}] name [{}]",
				grid_id,
				entry.number,
				GetCleanName()
			);

			Waypoints.push_back(new_waypoint);
			max_wp++;
		}
	}

	cur_wp = start_wp;
	UpdateWaypoint(start_wp);
	SetWaypointPause();

	if (wandertype == GridRandomPath) {
		cur_wp = GetClosestWaypoint(glm::vec3(GetPosition()));
		patrol = cur_wp;
	}

	if (wandertype == GridRandom10 || wandertype == GridRandom || wandertype == GridRand5LoS) {
		CalculateNewWaypoint();
	}

}

void Mob::SendTo(float new_x, float new_y, float new_z) {
	if (IsNPC()) {
		entity_list.ProcessMove(CastToNPC(), new_x, new_y, new_z);
	}

	m_Position.x = new_x;
	m_Position.y = new_y;
	m_Position.z = new_z;
	LogAIDetail("Sent To ({}, {}, {})", new_x, new_y, new_z);

	if (flymode == GravityBehavior::Flying)
		return;

	//fix up pathing Z, this shouldent be needed IF our waypoints
	//are corrected instead
	if (zone->HasMap() && RuleB(Map, FixPathingZOnSendTo))
	{
		if (!RuleB(Watermap, CheckForWaterOnSendTo) || !zone->HasWaterMap() ||
			(zone->HasWaterMap() && !zone->watermap->InWater(glm::vec3(m_Position))))
		{
			glm::vec3 dest(m_Position.x, m_Position.y, m_Position.z);

			float newz = zone->zonemap->FindBestZ(dest, nullptr);

			LogAIDetail("BestZ returned {} at {}, {}, {}", newz, m_Position.x, m_Position.y, m_Position.z);

			if ((newz > -2000) && std::abs(newz - dest.z) < RuleR(Map, FixPathingZMaxDeltaSendTo)) // Sanity check.
				m_Position.z = newz + 1;
		}
	}
	else
		m_Position.z += 0.1;
}

void Mob::SendToFixZ(float new_x, float new_y, float new_z) {
	if (IsNPC()) {
		entity_list.ProcessMove(CastToNPC(), new_x, new_y, new_z + 0.1);
	}

	m_Position.x = new_x;
	m_Position.y = new_y;
	m_Position.z = new_z + 0.1;

	if (zone->HasMap() && RuleB(Map, FixPathingZOnSendTo))
	{
		if (!RuleB(Watermap, CheckForWaterOnSendTo) || !zone->HasWaterMap() ||
			(zone->HasWaterMap() && !zone->watermap->InWater(glm::vec3(m_Position))))
		{
			glm::vec3 dest(m_Position.x, m_Position.y, m_Position.z);

			float newz = zone->zonemap->FindBestZ(dest, nullptr);

			LogPathing("BestZ returned [{}] at [{}], [{}], [{}]", newz, m_Position.x, m_Position.y, m_Position.z);

			if ((newz > -2000) && std::abs(newz - dest.z) < RuleR(Map, FixPathingZMaxDeltaSendTo)) // Sanity check.
				m_Position.z = newz + 1;
		}
	}
}

float Mob::GetFixedZ(const glm::vec3 &destination, int32 z_find_offset) {
	BenchTimer timer;
	timer.reset();

	float new_z = destination.z;

	if (zone->HasMap()) {
		if (flymode == GravityBehavior::Flying) {
			return new_z;
		}

		if (zone->HasWaterMap() && zone->watermap->InLiquid(glm::vec3(m_Position))) {
			return new_z;
		}

		new_z = FindDestGroundZ(destination, ((-GetZOffset() / 2) + z_find_offset));

		if (RuleB(Map, MobPathingVisualDebug)) {
			DrawDebugCoordinateNode(
				fmt::format("{} search z node", GetCleanName()),
				glm::vec4{
					m_Position.x,
					m_Position.y,
					((-GetZOffset() / 2) + z_find_offset),
					m_Position.w
				}
			);
		}
		if (new_z != BEST_Z_INVALID) {
			new_z += GetZOffset();

			if (new_z < -2000) {
				new_z = m_Position.z;
			}
		}

		auto duration = timer.elapsed();

		LogFixZ("[{}] returned [{}] at [{}] [{}] [{}] - Took [{}]",
			GetCleanName(),
			new_z,
			destination.x,
			destination.y,
			destination.z,
			duration);
	}

	return new_z;
}

void Mob::FixZ(int32 z_find_offset /*= 5*/, bool fix_client_z /*= false*/) {
	if (IsClient() && !fix_client_z) {
		return;
	}

	if (GetIsBoat()) {
		return;
	}

	if (flymode == GravityBehavior::Flying) {
		return;
	}

	if (zone->watermap && zone->watermap->InLiquid(m_Position)) {
		return;
	}

	glm::vec3 current_loc(m_Position);
	float new_z = GetFixedZ(current_loc, z_find_offset);

	// reject z if it is too far from the current z
	if (std::abs(new_z - m_Position.z) > 100) {
		return;
	}

	// reject if it's the same as the current z
	if (new_z == m_Position.z) {
		return;
	}

	if ((new_z > -2000) && new_z != BEST_Z_INVALID) {
		if (RuleB(Map, MobZVisualDebug)) {
			SendAppearanceEffect(78, 0, 0, 0, 0);
		}

		m_Position.z = new_z;

		if (RuleB(Map, MobPathingVisualDebug)) {
			DrawDebugCoordinateNode(fmt::format("{} new fixed z node", GetCleanName()), GetPosition());
		}
	}
	else {
		if (RuleB(Map, MobZVisualDebug)) {
			SendAppearanceEffect(103, 0, 0, 0, 0);
		}

		LogFixZ("[{}] is failing to find Z [{}]", GetCleanName(), std::abs(m_Position.z - new_z));
	}
}

float Mob::GetZOffset() const {
	float offset = 3.125f;

	switch (GetModel()) {
		case Race::Basilisk:
			offset = 0.577f;
			break;
		case Race::Drake2:
			offset = 0.5f;
			break;
		case Race::Drake3:
			offset = 1.9f;
			break;
		case Race::Dragon:
			offset = 0.93f;
			break;
		case Race::LavaSpider:
			offset = 0.938f;
			break;
		case Race::Alligator2:
			offset = 0.8f;
			break;
		case Race::LavaSpiderQueen:
			offset = 0.816f;
			break;
		case Race::Dragon2:
			offset = 0.527f;
			break;
		case Race::Puma2:
			offset = 1.536f;
			break;
		case Race::Rat:
			offset = 1.0f;
			break;
		case Race::Dragon3:
		case Race::Dragon4:
			offset = 0.776f;
			break;
		case Race::SpiderQueen:
			offset = 0.816f;
			break;
		case Race::Spider:
			offset = 0.938f;
			break;
		case Race::Imp:
		case Race::Snake:
		case Race::Corathus:
			offset = 1.0f;
			break;
		case Race::DrachnidCocoon:
			offset = 1.5f;
			break;
		case Race::Dragon5:
			offset = 1.2f;
			break;
		case Race::Goo4:
		case Race::Goo3:
			offset = 0.5f;
			break;
		case Race::Goo2:
			offset = 0.5f;
			break;
		case Race::Dracolich:
			offset = 1.2f;
			break;
		case Race::Telmira:
			offset = 5.9f;
			break;
		case Race::MorellThule:
			offset = 4.0f;
			break;
		case Race::AnimatedArmor:
		case Race::Amygdalan:
			offset = 5.0f;
			break;
		case Race::IksarSpirit:
		case Race::Sandman:
			offset = 4.0f;
			break;
		case Race::LavaDragon:
		case Race::AlaranSentryStone:
			offset = 9.0f;
			break;
		case Race::Rabbit:
			offset = 5.0f;
			break;
		case Race::Wurm:
		case Race::BlindDreamer:
			offset = 7.0f;
			break;
		case Race::Siren:
		case Race::HalasCitizen:
		case Race::Othmir:
			offset = .5f;
			break;
		case Race::Coldain:
			offset = .6f;
			break;
		case Race::Werewolf:
			offset = 1.2f;
			break;
		case Race::Dwarf:
			offset = .7f;
			break;
		case Race::Horse:
			offset = 1.4f;
			break;
		case Race::EnchantedArmor:
		case Race::Tiger:
			offset = 1.75f;
			break;
		case Race::StatueOfRallosZek:
			offset = 1.0f;
			break;
		case Race::Goral:
		case Race::Selyrah:
			offset = 2.0f;
			break;
		default:
			offset = 3.125f;
	}

	float mob_size = (GetSize() > 0 ? GetSize() : GetDefaultRaceSize());

	return static_cast<float>(0.2 * mob_size * offset);
}

// This function will try to move the mob along the relative angle a set distance
// if it can't be moved, it will lower the distance and try again
// If we want to move on like say a spawn, we can pass send as false
void Mob::TryMoveAlong(float distance, float angle, bool send)
{
	angle += GetHeading();
	angle = FixHeading(angle);

	glm::vec3 tmp_pos;
	glm::vec3 new_pos = GetPosition();
	new_pos.x += distance * g_Math.FastSin(angle);
	new_pos.y += distance * g_Math.FastCos(angle);
	new_pos.z += GetZOffset();

	if (zone->HasMap()) {
		auto new_z = zone->zonemap->FindClosestZ(new_pos, nullptr);
		if (new_z != BEST_Z_INVALID)
			new_pos.z = new_z;

		if (zone->zonemap->LineIntersectsZone(GetPosition(), new_pos, 0.0f, &tmp_pos))
			new_pos = tmp_pos;
	}

	new_pos.z = GetFixedZ(new_pos);

	Teleport(new_pos);
}

// like above, but takes a starting position and returns a new location instead of actually moving
glm::vec4 Mob::TryMoveAlong(const glm::vec4 &start, float distance, float angle)
{
	angle += start.w;
	angle = FixHeading(angle);

	glm::vec3 tmp_pos;
	glm::vec3 new_pos = start;
	new_pos.x += distance * g_Math.FastSin(angle);
	new_pos.y += distance * g_Math.FastCos(angle);

	if (zone->HasMap()) {
		if (zone->zonemap->LineIntersectsZone(start, new_pos, 0.0f, &tmp_pos))
			new_pos = tmp_pos;
	}

	new_pos.z = GetFixedZ(new_pos);

	return {new_pos.x, new_pos.y, new_pos.z, start.w};
}

int	ZoneDatabase::GetHighestGrid(uint32 zone_id)
{
	return GridRepository::GetHighestGrid(*this, zone_id);
}

void ZoneDatabase::ModifyGrid(
	Client* c,
	bool remove,
	uint32 grid_id,
	uint8 type,
	uint8 type2,
	uint32 zone_id
)
{
	if (!remove) {
		GridRepository::InsertOne(
			*this,
			GridRepository::Grid{
				.id = static_cast<int32_t>(grid_id),
				.zoneid = static_cast<int32_t>(zone_id),
				.type = type,
				.type2 = type2
			}
		);

		return;
	}

	GridRepository::DeleteWhere(
		*this,
		fmt::format(
			"`id` = {} AND `zoneid` = {}",
			grid_id,
			zone_id
		)
	);

	GridEntriesRepository::DeleteWhere(
		*this,
		fmt::format(
			"`gridid` = {} AND `zoneid` = {}",
			grid_id,
			zone_id
		)
	);
}

bool ZoneDatabase::GridExistsInZone(uint32 zone_id, uint32 grid_id)
{
	const auto& l = GridRepository::GetWhere(
		*this,
		fmt::format(
			"`id` = {} AND `zoneid` = {}",
			grid_id,
			zone_id
		)
	);

	if (l.empty()) {
		return false;
	}

	return true;
}

void ZoneDatabase::AddWaypoint(
	Client* c,
	uint32 grid_id,
	uint32 number,
	const glm::vec4& position,
	uint32 pause,
	uint32 zone_id
)
{
	GridEntriesRepository::InsertOne(
		*this,
		GridEntriesRepository::GridEntries{
			.gridid = static_cast<int32_t>(grid_id),
			.zoneid = static_cast<int32_t>(zone_id),
			.number = static_cast<int32_t>(number),
			.x = position.x,
			.y = position.y,
			.z = position.z,
			.heading = position.w,
			.pause = static_cast<int32_t>(pause)
		}
	);
}

void ZoneDatabase::DeleteWaypoint(Client* c, uint32 grid_id, uint32 number, uint32 zone_id)
{
	GridEntriesRepository::DeleteWhere(
		*this,
		fmt::format(
			"`gridid` = {} AND `zoneid` = {} AND `number` = {}",
			grid_id,
			zone_id,
			number
		)
	);
}

uint32 ZoneDatabase::AddWaypointForSpawn(
	Client* c,
	uint32 spawn2_id,
	const glm::vec4& position,
	uint32 pause,
	int type,
	int type2,
	uint32 zone_id
)
{
	uint32 grid_id = Spawn2Repository::GetPathGridBySpawn2ID(*this, spawn2_id);    // The grid number the spawn is assigned to (if spawn has no grid, will be the grid number we end up creating)
	bool   created;       // Did we create a new grid in this function?

	if (!grid_id) { // Our spawn doesn't have a grid assigned to it -- we need to create a new grid and assign it to the spawn
		created = true;
		grid_id = GetFreeGrid(zone_id);

		if (grid_id == 0) { // There are no grids for the current zone -- create Grid #1
			grid_id = 1;
		}

		GridRepository::InsertOne(
			*this,
			GridRepository::Grid{
				.id = static_cast<int32_t>(grid_id),
				.zoneid = static_cast<int32_t>(zone_id),
				.type = type,
				.type2 = type2
			}
		);

		Spawn2Repository::SetPathGridBySpawn2ID(*this, spawn2_id, grid_id);
	} else { // NPC had a grid assigned to it
		created = false;
	}

	int next_waypoint = GridEntriesRepository::GetNextWaypoint(*this, zone_id, grid_id); // The waypoint number we should be assigning to the new waypoint

	GridEntriesRepository::InsertOne(
		*this,
		GridEntriesRepository::GridEntries{
			.gridid = static_cast<int32_t>(grid_id),
			.zoneid = static_cast<int32_t>(zone_id),
			.number = next_waypoint,
			.x = position.x,
			.y = position.y,
			.z = position.z,
			.heading = position.w,
			.pause = static_cast<int32_t>(pause)
		}
	);

	return created ? grid_id : 0;
}

uint32 ZoneDatabase::GetFreeGrid(uint32 zone_id)
{
	return GridRepository::GetHighestGrid(*this, zone_id) + 1;
}

int ZoneDatabase::GetHighestWaypoint(uint32 zone_id, uint32 grid_id)
{
	return GridEntriesRepository::GetHighestWaypoint(*this, zone_id, grid_id);
}

int ZoneDatabase::GetRandomWaypointFromGrid(glm::vec4 &loc, uint32 zone_id, uint32 grid_id)
{
	loc.x = loc.y = loc.z = loc.w = 0.0f;

	const auto& l = GridEntriesRepository::GetWhere(
		*this,
		fmt::format(
			"`zoneid` = {} AND `gridid` = {} ORDER BY RAND() LIMIT 1",
			zone_id,
			grid_id
		)
	);

	if (l.empty()) {
		return 0;
	}

	auto e = l.front();

	loc.x = e.x;
	loc.y = e.y;
	loc.z = e.z;
	loc.w = e.heading;

	return e.number;
}

void NPC::SaveGuardSpotCharm()
{
	m_GuardPointSaved = m_GuardPoint;
}

void NPC::RestoreGuardSpotCharm()
{
	m_GuardPoint = m_GuardPointSaved;
}

/******************
* Bot-specific overloads to make them play nice with the new movement system
*/

void Bot::WalkTo(float x, float y, float z)
{
	if (IsSitting())
		Stand();

	Mob::WalkTo(x, y, z);
}

void Bot::RunTo(float x, float y, float z)
{
	if (IsSitting())
		Stand();

	Mob::RunTo(x, y, z);
}
