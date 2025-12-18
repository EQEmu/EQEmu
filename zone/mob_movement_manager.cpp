#include "mob_movement_manager.h"

#include "common/data_verification.h"
#include "common/eq_packet_structs.h"
#include "common/misc_functions.h"
#include "zone/client.h"
#include "zone/mob.h"
#include "zone/position.h"
#include "zone/water_map.h"
#include "zone/zone.h"

#include <cstdlib>
#include <deque>
#include <map>
#include <vector>

extern double frame_time;
extern Zone   *zone;

class IMovementCommand {
public:
	IMovementCommand() = default;
	virtual ~IMovementCommand() = default;
	virtual bool Process(MobMovementManager *mob_movement_manager, Mob *mob) = 0;
	virtual bool Started() const = 0;
};

class RotateToCommand : public IMovementCommand {
public:
	RotateToCommand(double rotate_to, double dir, MobMovementMode mob_movement_mode)
	{
		m_rotate_to      = rotate_to;
		m_rotate_to_dir  = dir;
		m_rotate_to_mode = mob_movement_mode;
		m_started        = false;
	}

	virtual ~RotateToCommand()
	{

	}

	virtual bool Process(MobMovementManager *mob_movement_manager, Mob *mob)
	{
		if (!mob->IsAIControlled()) {
			return true;
		}

		auto rotate_to_speed = m_rotate_to_mode == MovementRunning ? 200.0 : 16.0; //todo: get this from mob

		auto from = FixHeading(mob->GetHeading());
		auto to   = FixHeading(m_rotate_to);
		auto diff = to - from;

		while (diff < -256.0) {
			diff += 512.0;
		}

		while (diff > 256) {
			diff -= 512.0;
		}

		auto dist = std::abs(diff);

		if (!m_started) {
			m_started = true;
			mob->turning = true;
			mob->SetMoving(true);

			if (dist > 15.0f && rotate_to_speed > 0.0 && rotate_to_speed <= 25.0) { //send basic rotation
				mob_movement_manager->SendCommandToClients(
					mob,
					0.0,
					0.0,
					0.0,
					m_rotate_to_dir * rotate_to_speed,
					0,
					ClientRangeClose
				);
			}
		}

		auto td = rotate_to_speed * 19.0 * frame_time;

		if (td >= dist) {
			mob->SetHeading(to);
			mob->SetMoving(false);
			mob_movement_manager->SendCommandToClients(mob, 0.0, 0.0, 0.0, 0.0, 0, ClientRangeCloseMedium);
			mob->turning = false;
			return true;
		}

		from += td * m_rotate_to_dir;
		mob->SetHeading(FixHeading(from));
		return false;
	}

	virtual bool Started() const
	{
		return m_started;
	}

private:
	double          m_rotate_to;
	double          m_rotate_to_dir;
	MobMovementMode m_rotate_to_mode;
	bool            m_started;
};

class MoveToCommand : public IMovementCommand {
public:
	MoveToCommand(float x, float y, float z, MobMovementMode mob_movement_mode)
	{
		m_distance_moved_since_correction = 0.0;
		m_move_to_x                       = x;
		m_move_to_y                       = y;
		m_move_to_z                       = z;
		m_move_to_mode                    = mob_movement_mode;
		m_last_sent_time                  = 0.0;
		m_last_sent_speed                 = 0;
		m_started                         = false;
		m_total_h_dist                    = 0.0;
		m_total_v_dist                    = 0.0;
	}

	virtual ~MoveToCommand()
	{

	}

	virtual bool Process(MobMovementManager *mob_movement_manager, Mob *mob)
	{
		if (!mob->IsAIControlled()) {
			return true;
		}

		//Send a movement packet when you start moving
		double current_time  = static_cast<double>(Timer::GetCurrentTime()) / 1000.0;
		int    current_speed = 0;

		if (m_move_to_mode == MovementRunning) {
			if (mob->IsFeared()) {
				current_speed = mob->GetFearSpeed();
			}
			else {
				current_speed = mob->GetRunspeed();
			}
		}
		else {
			current_speed = mob->GetWalkspeed();
		}

		if (!m_started) {
			m_started = true;
			//rotate to the point
			mob->SetMoving(true);
			mob->SetHeading(mob->CalculateHeadingToTarget(m_move_to_x, m_move_to_y));

			m_last_sent_speed = current_speed;
			m_last_sent_time  = current_time;
			m_total_h_dist    = DistanceNoZ(mob->GetPosition(), glm::vec4(m_move_to_x, m_move_to_y, 0.0f, 0.0f));
			m_total_v_dist    = m_move_to_z - mob->GetZ();
			mob_movement_manager->SendCommandToClients(mob, 0.0, 0.0, 0.0, 0.0, current_speed, ClientRangeCloseMedium);
		}

		//When speed changes
		if (current_speed != m_last_sent_speed) {
			if (RuleB(Map, FixZWhenPathing)) {
				mob->FixZ();
			}

			m_distance_moved_since_correction = 0.0;

			m_last_sent_speed = current_speed;
			m_last_sent_time  = current_time;
			mob_movement_manager->SendCommandToClients(mob, 0.0, 0.0, 0.0, 0.0, current_speed, ClientRangeCloseMedium);
		}

		//If x seconds have passed without sending an update.
		if (current_time - m_last_sent_time >= 5.0) {
			if (RuleB(Map, FixZWhenPathing)) {
				mob->FixZ();
			}

			m_distance_moved_since_correction = 0.0;

			m_last_sent_speed = current_speed;
			m_last_sent_time  = current_time;
			mob_movement_manager->SendCommandToClients(mob, 0.0, 0.0, 0.0, 0.0, current_speed, ClientRangeCloseMedium);
		}

		auto      &p  = mob->GetPosition();
		glm::vec2 tar(m_move_to_x, m_move_to_y);
		glm::vec2 pos(p.x, p.y);
		double    len = glm::distance(pos, tar);
		if (len == 0) {
			return true;
		}

		mob->SetMoved(true);

		glm::vec2 dir            = tar - pos;
		glm::vec2 ndir           = glm::normalize(dir);
		double    distance_moved = frame_time * current_speed * 0.4f * 1.45f;

		if (distance_moved > len) {
			if (mob->IsNPC()) {
				entity_list.ProcessMove(mob->CastToNPC(), m_move_to_x, m_move_to_y, m_move_to_z);
			}

			mob->SetPosition(m_move_to_x, m_move_to_y, m_move_to_z);

			if (RuleB(Map, FixZWhenPathing)) {
				mob->FixZ();
			}
			return true;
		}
		else {
			glm::vec2 npos = pos + (ndir * static_cast<float>(distance_moved));

			len -= distance_moved;
			double total_distance_traveled = m_total_h_dist - len;
			double start_z                 = m_move_to_z - m_total_v_dist;
			double z_at_pos                = start_z + (m_total_v_dist * (total_distance_traveled / m_total_h_dist));

			if (mob->IsNPC()) {
				entity_list.ProcessMove(mob->CastToNPC(), npos.x, npos.y, z_at_pos);
			}

			mob->SetPosition(npos.x, npos.y, z_at_pos);


			if (RuleB(Map, FixZWhenPathing)) {
				m_distance_moved_since_correction += distance_moved;
				if (m_distance_moved_since_correction > (mob->IsEngaged() ? 1 : 10)) {
					m_distance_moved_since_correction = 0.0;
					mob->FixZ();
				}
			}
		}

		return false;
	}

	virtual bool Started() const
	{
		return m_started;
	}

protected:
	double          m_distance_moved_since_correction;
	double          m_move_to_x;
	double          m_move_to_y;
	double          m_move_to_z;
	MobMovementMode m_move_to_mode;
	bool            m_started;

	double m_last_sent_time;
	int    m_last_sent_speed;
	double m_total_h_dist;
	double m_total_v_dist;
};

class FlyToCommand : public IMovementCommand {
public:
	FlyToCommand(float x, float y, float z, MobMovementMode mob_movement_mode)
	{
		m_distance_moved_since_correction = 0.0;
		m_move_to_x                       = x;
		m_move_to_y                       = y;
		m_move_to_z                       = z;
		m_move_to_mode                    = mob_movement_mode;
		m_last_sent_time                  = 0.0;
		m_last_sent_speed                 = 0;
		m_started                         = false;
		m_total_h_dist                    = 0.0;
		m_total_v_dist                    = 0.0;
	}

	virtual ~FlyToCommand()
	{

	}

	virtual bool Process(MobMovementManager *mob_movement_manager, Mob *mob)
	{
		if (!mob->IsAIControlled()) {
			return true;
		}

		//Send a movement packet when you start moving
		double current_time  = static_cast<double>(Timer::GetCurrentTime()) / 1000.0;
		int    current_speed = 0;

		if (m_move_to_mode == MovementRunning) {
			if (mob->IsFeared()) {
				current_speed = mob->GetFearSpeed();
			}
			else {
				current_speed = mob->GetRunspeed();
			}
		}
		else {
			current_speed = mob->GetWalkspeed();
		}

		if (!m_started) {
			m_started = true;
			//rotate to the point
			mob->SetMoving(true);
			mob->SetHeading(mob->CalculateHeadingToTarget(m_move_to_x, m_move_to_y));

			m_last_sent_speed = current_speed;
			m_last_sent_time  = current_time;
			m_total_h_dist    = DistanceNoZ(mob->GetPosition(), glm::vec4(m_move_to_x, m_move_to_y, 0.0f, 0.0f));
			m_total_v_dist    = m_move_to_z - mob->GetZ();
			mob_movement_manager->SendCommandToClients(mob, 0.0, 0.0, 0.0, 0.0, current_speed, ClientRangeCloseMedium);
		}

		//When speed changes
		if (current_speed != m_last_sent_speed) {
			m_distance_moved_since_correction = 0.0;
			m_last_sent_speed = current_speed;
			m_last_sent_time  = current_time;
			mob_movement_manager->SendCommandToClients(mob, 0.0, 0.0, 0.0, 0.0, current_speed, ClientRangeCloseMedium);
		}

		//If x seconds have passed without sending an update.
		if (current_time - m_last_sent_time >= 0.5) {
			m_distance_moved_since_correction = 0.0;
			m_last_sent_speed = current_speed;
			m_last_sent_time  = current_time;
			mob_movement_manager->SendCommandToClients(mob, 0.0, 0.0, 0.0, 0.0, current_speed, ClientRangeCloseMedium);
		}

		auto      &p  = mob->GetPosition();
		glm::vec2 tar(m_move_to_x, m_move_to_y);
		glm::vec2 pos(p.x, p.y);
		double    len = glm::distance(pos, tar);
		if (len == 0) {
			return true;
		}

		mob->SetMoved(true);

		glm::vec2 dir            = tar - pos;
		glm::vec2 ndir           = glm::normalize(dir);
		double    distance_moved = frame_time * current_speed * 0.4f * 1.45f;

		if (distance_moved > len) {
			if (mob->IsNPC()) {
				entity_list.ProcessMove(mob->CastToNPC(), m_move_to_x, m_move_to_y, m_move_to_z);
			}

			mob->SetPosition(m_move_to_x, m_move_to_y, m_move_to_z);

			if (RuleB(Map, FixZWhenPathing)) {
				mob->FixZ();
			}
			return true;
		}
		else {
			glm::vec2 npos = pos + (ndir * static_cast<float>(distance_moved));

			len -= distance_moved;
			double total_distance_traveled = m_total_h_dist - len;
			double start_z                 = m_move_to_z - m_total_v_dist;
			double z_at_pos                = start_z + (m_total_v_dist * (total_distance_traveled / m_total_h_dist));

			if (mob->IsNPC()) {
				entity_list.ProcessMove(mob->CastToNPC(), npos.x, npos.y, z_at_pos);
			}

			mob->SetPosition(npos.x, npos.y, z_at_pos);
		}

		return false;
	}

	virtual bool Started() const
	{
		return m_started;
	}

protected:
	double          m_distance_moved_since_correction;
	double          m_move_to_x;
	double          m_move_to_y;
	double          m_move_to_z;
	MobMovementMode m_move_to_mode;
	bool            m_started;

	double m_last_sent_time;
	int    m_last_sent_speed;
	double m_total_h_dist;
	double m_total_v_dist;
};

class SwimToCommand : public MoveToCommand {
public:
	SwimToCommand(float x, float y, float z, MobMovementMode mob_movement_mode) : MoveToCommand(x, y, z, mob_movement_mode)
	{

	}

	virtual bool Process(MobMovementManager *mob_movement_manager, Mob *mob)
	{
		if (!mob->IsAIControlled()) {
			return true;
		}

		//Send a movement packet when you start moving
		double current_time  = static_cast<double>(Timer::GetCurrentTime()) / 1000.0;
		int    current_speed = 0;

		if (m_move_to_mode == MovementRunning) {
			if (mob->IsFeared()) {
				current_speed = mob->GetFearSpeed();
			}
			else {
				current_speed = mob->GetRunspeed();
			}
		}
		else {
			current_speed = mob->GetWalkspeed();
		}

		if (!m_started) {
			m_started = true;
			//rotate to the point
			mob->SetMoving(true);
			mob->SetHeading(mob->CalculateHeadingToTarget(m_move_to_x, m_move_to_y));

			m_last_sent_speed = current_speed;
			m_last_sent_time  = current_time;
			m_total_h_dist    = DistanceNoZ(mob->GetPosition(), glm::vec4(m_move_to_x, m_move_to_y, 0.0f, 0.0f));
			m_total_v_dist    = m_move_to_z - mob->GetZ();
			mob_movement_manager->SendCommandToClients(mob, 0.0, 0.0, 0.0, 0.0, current_speed, ClientRangeCloseMedium);
		}

		//When speed changes
		if (current_speed != m_last_sent_speed) {
			m_last_sent_speed = current_speed;
			m_last_sent_time  = current_time;
			mob_movement_manager->SendCommandToClients(mob, 0.0, 0.0, 0.0, 0.0, current_speed, ClientRangeCloseMedium);
		}

		//If x seconds have passed without sending an update.
		if (current_time - m_last_sent_time >= 1.5) {
			m_last_sent_speed = current_speed;
			m_last_sent_time  = current_time;
			mob_movement_manager->SendCommandToClients(mob, 0.0, 0.0, 0.0, 0.0, current_speed, ClientRangeCloseMedium);
		}

		auto      &p  = mob->GetPosition();
		glm::vec2 tar(m_move_to_x, m_move_to_y);
		glm::vec2 pos(p.x, p.y);
		double    len = glm::distance(pos, tar);
		if (len == 0) {
			return true;
		}

		mob->SetMoved(true);

		glm::vec2 dir            = tar - pos;
		glm::vec2 ndir           = glm::normalize(dir);
		double    distance_moved = frame_time * current_speed * 0.4f * 1.45f;

		if (distance_moved > len) {
			if (mob->IsNPC()) {
				entity_list.ProcessMove(mob->CastToNPC(), m_move_to_x, m_move_to_y, m_move_to_z);
			}

			mob->SetPosition(m_move_to_x, m_move_to_y, m_move_to_z);
			return true;
		}
		else {
			glm::vec2 npos = pos + (ndir * static_cast<float>(distance_moved));

			len -= distance_moved;
			double total_distance_traveled = m_total_h_dist - len;
			double start_z                 = m_move_to_z - m_total_v_dist;
			double z_at_pos                = start_z + (m_total_v_dist * (total_distance_traveled / m_total_h_dist));

			if (mob->IsNPC()) {
				entity_list.ProcessMove(mob->CastToNPC(), npos.x, npos.y, z_at_pos);
			}

			mob->SetPosition(npos.x, npos.y, z_at_pos);
		}

		return false;
	}
};

class TeleportToCommand : public IMovementCommand {
public:
	TeleportToCommand(float x, float y, float z, float heading)
	{
		m_teleport_to_x       = x;
		m_teleport_to_y       = y;
		m_teleport_to_z       = z;
		m_teleport_to_heading = heading;
	}

	virtual ~TeleportToCommand()
	{

	}

	virtual bool Process(MobMovementManager *mob_movement_manager, Mob *mob)
	{
		if (!mob->IsAIControlled()) {
			return true;
		}

		if (mob->IsNPC()) {
			entity_list.ProcessMove(mob->CastToNPC(), m_teleport_to_x, m_teleport_to_y, m_teleport_to_z);
		}

		mob->SetPosition(m_teleport_to_x, m_teleport_to_y, m_teleport_to_z);
		mob->SetHeading(mob_movement_manager->FixHeading(m_teleport_to_heading));
		mob_movement_manager->SendCommandToClients(mob, 0.0, 0.0, 0.0, 0.0, 0, ClientRangeAny);

		return true;
	}

	virtual bool Started() const
	{
		return false;
	}

private:

	double m_teleport_to_x;
	double m_teleport_to_y;
	double m_teleport_to_z;
	double m_teleport_to_heading;
};

class StopMovingCommand : public IMovementCommand {
public:
	StopMovingCommand()
	{
	}

	virtual ~StopMovingCommand()
	{

	}

	virtual bool Process(MobMovementManager *mob_movement_manager, Mob *mob)
	{
		if (!mob->IsAIControlled()) {
			return true;
		}

		if (mob->IsMoving()) {
			mob->SetMoving(false);
			if (RuleB(Map, FixZWhenPathing)) {
				mob->FixZ();
			}
			mob_movement_manager->SendCommandToClients(mob, 0.0, 0.0, 0.0, 0.0, 0, ClientRangeCloseMedium);
		}

		return true;
	}

	virtual bool Started() const
	{
		return false;
	}
};

class EvadeCombatCommand : public IMovementCommand {
public:
	EvadeCombatCommand()
	{
	}

	virtual ~EvadeCombatCommand()
	{

	}

	virtual bool Process(MobMovementManager *mob_movement_manager, Mob *mob)
	{
		if (!mob->IsAIControlled()) {
			return true;
		}

		if (mob->IsMoving()) {
			mob->SetMoving(false);
			mob_movement_manager->SendCommandToClients(mob, 0.0, 0.0, 0.0, 0.0, 0, ClientRangeCloseMedium);
		}

		mob->BuffFadeAll();
		mob->WipeHateList();
		mob->RestoreHealth();

		return false;
	}

	virtual bool Started() const
	{
		return false;
	}
};

struct MovementStats {
	MovementStats()
	{
		LastResetTime     = static_cast<double>(Timer::GetCurrentTime()) / 1000.0;
		TotalSent         = 0ULL;
		TotalSentMovement = 0ULL;
		TotalSentPosition = 0ULL;
		TotalSentHeading  = 0ULL;
	}

	double   LastResetTime;
	uint64_t TotalSent;
	uint64_t TotalSentMovement;
	uint64_t TotalSentPosition;
	uint64_t TotalSentHeading;
};

struct NavigateTo {
	NavigateTo()
	{
		navigate_to_x       = 0.0;
		navigate_to_y       = 0.0;
		navigate_to_z       = 0.0;
		navigate_to_heading = 0.0;
		last_set_time       = 0.0;
	}

	double navigate_to_x;
	double navigate_to_y;
	double navigate_to_z;
	double navigate_to_heading;
	double last_set_time;
};

struct MobMovementEntry {
	std::deque<std::unique_ptr<IMovementCommand>> Commands;
	NavigateTo                                    NavTo;
};

void AdjustRoute(std::list<IPathfinder::IPathNode> &nodes, Mob *who)
{
	if (!zone->HasMap() || !zone->HasWaterMap()) {
		return;
	}

	auto offset = who->GetZOffset();

	for (auto &node : nodes) {
		if (!zone->watermap->InLiquid(node.pos)) {
			auto best_z = zone->zonemap->FindBestZ(node.pos, nullptr);
			if (best_z != BEST_Z_INVALID) {
				node.pos.z = best_z + offset;
			}
		} // todo: floating logic?
	}
}

struct MobMovementManager::Implementation {
	std::map<Mob *, MobMovementEntry> Entries;
	std::vector<Client *>             Clients;
	MovementStats                     Stats;
};

MobMovementManager::MobMovementManager()
{
	_impl = std::make_unique<Implementation>();
}

MobMovementManager::~MobMovementManager()
{
}

void MobMovementManager::Process()
{
	for (auto &iter : _impl->Entries) {
		auto &ent      = iter.second;
		auto &commands = ent.Commands;

		while (true != commands.empty()) {
			auto &cmd = commands.front();
			auto r    = cmd->Process(this, iter.first);

			if (true != r) {
				break;
			}

			commands.pop_front();
		}
	}
}

void MobMovementManager::AddMob(Mob *mob)
{
	_impl->Entries.insert(std::make_pair(mob, MobMovementEntry()));
}

void MobMovementManager::RemoveMob(Mob *mob)
{
	_impl->Entries.erase(mob);
}

void MobMovementManager::AddClient(Client *client)
{
	_impl->Clients.push_back(client);
}

void MobMovementManager::RemoveClient(Client *client)
{
	auto iter = _impl->Clients.begin();
	while (iter != _impl->Clients.end()) {
		if (client == *iter) {
			_impl->Clients.erase(iter);
			return;
		}

		++iter;
	}
}

void MobMovementManager::RotateTo(Mob *who, float to, MobMovementMode mob_movement_mode)
{
	auto iter = _impl->Entries.find(who);
	auto &ent = (*iter);

	if (true != ent.second.Commands.empty()) {
		return;
	}

	PushRotateTo(ent.second, who, to, mob_movement_mode);
}

void MobMovementManager::Teleport(Mob *who, float x, float y, float z, float heading)
{
	auto iter = _impl->Entries.find(who);
	auto &ent = (*iter);

	ent.second.Commands.clear();

	PushTeleportTo(ent.second, x, y, z, heading);
}

void MobMovementManager::NavigateTo(Mob *who, float x, float y, float z, MobMovementMode mode)
{
	if (IsPositionEqualWithinCertainZ(glm::vec3(x, y, z), glm::vec3(who->GetX(), who->GetY(), who->GetZ()), 6.0f)) {
		return;
	}

	auto iter = _impl->Entries.find(who);
	auto &ent = (*iter);
	auto &nav = ent.second.NavTo;

	double current_time = static_cast<double>(Timer::GetCurrentTime()) / 1000.0;
	if ((current_time - nav.last_set_time) > 0.5) {
		//Can potentially recalc

		auto within        = IsPositionWithinSimpleCylinder(
			glm::vec3(x, y, z),
			glm::vec3(nav.navigate_to_x, nav.navigate_to_y, nav.navigate_to_z),
			1.5f,
			6.0f
		);
		auto heading_match = IsHeadingEqual(0.0, nav.navigate_to_heading);

		if (false == within || false == heading_match || ent.second.Commands.size() == 0) {
			ent.second.Commands.clear();

			//Path is no longer valid, calculate a new path
			UpdatePath(who, x, y, z, mode);
			nav.navigate_to_x       = x;
			nav.navigate_to_y       = y;
			nav.navigate_to_z       = z;
			nav.navigate_to_heading = 0.0;
			nav.last_set_time       = current_time;
		}
	}
}

void MobMovementManager::StopNavigation(Mob *who)
{
	auto iter = _impl->Entries.find(who);
	auto &ent = (*iter);
	auto &nav = ent.second.NavTo;

	nav.navigate_to_x       = 0.0;
	nav.navigate_to_y       = 0.0;
	nav.navigate_to_z       = 0.0;
	nav.navigate_to_heading = 0.0;

	if (true == ent.second.Commands.empty()) {
		PushStopMoving(ent.second);
		return;
	}

	if (!who->IsMoving()) {
		ent.second.Commands.clear();
		return;
	}

	ent.second.Commands.clear();
	PushStopMoving(ent.second);
}

void MobMovementManager::SendCommandToClients(
	Mob *mob,
	float delta_x,
	float delta_y,
	float delta_z,
	float delta_heading,
	int anim,
	ClientRange range,
	Client* single_client,
	Client* ignore_client
)
{
	if (range == ClientRangeNone) {
		return;
	}

	static EQApplicationPacket p(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
	auto                       *spu = (PlayerPositionUpdateServer_Struct *) p.pBuffer;

	FillCommandStruct(spu, mob, delta_x, delta_y, delta_z, delta_heading, anim);

	if (range == ClientRangeAny) {
		for (auto &c : _impl->Clients) {
			if (single_client && c != single_client) {
				continue;
			}

			if (ignore_client && c == ignore_client) {
				continue;
			}

			if (c->IsIdle()) {
				continue;
			}

			_impl->Stats.TotalSent++;

			if (anim != 0) {
				_impl->Stats.TotalSentMovement++;
			}
			else if (delta_heading != 0) {
				_impl->Stats.TotalSentHeading++;
			}
			else {
				_impl->Stats.TotalSentPosition++;
			}

			if (!mob->IsClient() && c->m_last_seen_mob_position.contains(mob->GetID())) {
				if (c->m_last_seen_mob_position[mob->GetID()] == mob->GetPosition() && anim == 0) {
					LogPositionUpdate(
						"Mob [{}] has already been sent to client [{}] at this position, skipping",
						mob->GetCleanName(),
						c->GetCleanName()
					);
					continue;
				}
			}

			c->QueuePacket(&p, false);
			c->m_last_seen_mob_position[mob->GetID()] = mob->GetPosition();
		}
	}
	else {
		float short_range = RuleR(Pathing, ShortMovementUpdateRange);
		float long_range  = RuleI(Range, MobCloseScanDistance);

		for (auto &c : _impl->Clients) {
			if (single_client && c != single_client) {
				continue;
			}

			if (ignore_client && c == ignore_client) {
				continue;
			}

			if (c->IsIdle()) {
				continue;
			}

			float distance = c->CalculateDistance(mob->GetX(), mob->GetY(), mob->GetZ());

			bool match = false;
			if (range & ClientRangeClose) {
				if (distance < short_range) {
					match = true;
				}
			}

			if (!match && range & ClientRangeMedium) {
				if (distance >= short_range && distance < long_range) {
					match = true;
				}
			}

			if (!match && range & ClientRangeLong) {
				if (distance >= long_range) {
					match = true;
				}
			}

			if (match) {
				_impl->Stats.TotalSent++;

				if (anim != 0) {
					_impl->Stats.TotalSentMovement++;
				}
				else if (delta_heading != 0) {
					_impl->Stats.TotalSentHeading++;
				}
				else {
					_impl->Stats.TotalSentPosition++;
				}

				if (!mob->IsClient() && c->m_last_seen_mob_position.contains(mob->GetID())) {
					if (c->m_last_seen_mob_position[mob->GetID()] == mob->GetPosition() && anim == 0) {
						LogPositionUpdate(
							"Mob [{}] has already been sent to client [{}] at this position, skipping",
							mob->GetCleanName(),
							c->GetCleanName()
						);
						continue;
					}
				}

				c->QueuePacket(&p, false);
				c->m_last_seen_mob_position[mob->GetID()] = mob->GetPosition();
			}
		}
	}
}

float MobMovementManager::FixHeading(float in)
{
	int h = static_cast<int>(in) % 512;
	if (h < 0) {
		h += 512;
	}
	return static_cast<float>(h);
}

void MobMovementManager::DumpStats(Client *client)
{
	auto current_time = static_cast<double>(Timer::GetCurrentTime()) / 1000.0;
	auto total_time   = current_time - _impl->Stats.LastResetTime;

	client->Message(Chat::System, "Dumping Movement Stats:");
	client->Message(
		Chat::System,
		"Total Sent: %u (%.2f / sec)",
		_impl->Stats.TotalSent,
		static_cast<double>(_impl->Stats.TotalSent) / total_time
	);
	client->Message(
		Chat::System,
		"Total Heading: %u (%.2f / sec)",
		_impl->Stats.TotalSentHeading,
		static_cast<double>(_impl->Stats.TotalSentHeading) / total_time
	);
	client->Message(
		Chat::System,
		"Total Movement: %u (%.2f / sec)",
		_impl->Stats.TotalSentMovement,
		static_cast<double>(_impl->Stats.TotalSentMovement) / total_time
	);
	client->Message(
		Chat::System,
		"Total Position: %u (%.2f / sec)",
		_impl->Stats.TotalSentPosition,
		static_cast<double>(_impl->Stats.TotalSentPosition) / total_time
	);
}

void MobMovementManager::ClearStats()
{
	_impl->Stats.LastResetTime     = static_cast<double>(Timer::GetCurrentTime()) / 1000.0;
	_impl->Stats.TotalSent         = 0;
	_impl->Stats.TotalSentHeading  = 0;
	_impl->Stats.TotalSentMovement = 0;
	_impl->Stats.TotalSentPosition = 0;
}

/**
 * @param position_update
 * @param mob
 * @param delta_x
 * @param delta_y
 * @param delta_z
 * @param delta_heading
 * @param anim
 */
void MobMovementManager::FillCommandStruct(
	PlayerPositionUpdateServer_Struct *position_update,
	Mob *mob,
	float delta_x,
	float delta_y,
	float delta_z,
	float delta_heading,
	int anim
)
{
	memset(position_update, 0x00, sizeof(PlayerPositionUpdateServer_Struct));
	position_update->spawn_id      = mob->GetID();
	position_update->x_pos         = FloatToEQ19(mob->GetX());
	position_update->y_pos         = FloatToEQ19(mob->GetY());
	position_update->z_pos         = FloatToEQ19(mob->GetZ());
	position_update->heading       = FloatToEQ12(mob->GetHeading());
	position_update->delta_x       = FloatToEQ13(delta_x);
	position_update->delta_y       = FloatToEQ13(delta_y);
	position_update->delta_z       = FloatToEQ13(delta_z);
	position_update->delta_heading = FloatToEQ10(delta_heading);
	position_update->animation     = (mob->IsBot() ? (int) ((float) anim / 1.785714f) : anim);

	if (RuleB(Map, MobPathingVisualDebug)) {
		mob->DrawDebugCoordinateNode(
			fmt::format("{} position update", mob->GetCleanName()),
			mob->GetPosition()
		);
	}
}

void MobMovementManager::UpdatePath(Mob *who, float x, float y, float z, MobMovementMode mob_movement_mode)
{
	Mob *target=who->GetTarget();

	if (!zone->HasMap() || !zone->HasWaterMap()) {
		auto iter = _impl->Entries.find(who);
		auto &ent = (*iter);

		PushMoveTo(ent.second, x, y, z, mob_movement_mode);
		PushStopMoving(ent.second);
		return;
	}

	if (who->GetIsBoat()) {
		UpdatePathBoat(who, x, y, z, mob_movement_mode);
	}
	else if (who->IsUnderwaterOnly()) {
		UpdatePathUnderwater(who, x, y, z, mob_movement_mode);
	}
	// If we can fly, and we have a target and we have LoS, simply fly to them.
	// if we ever lose LoS we go back to mesh run mode.
	else if (target && who->GetFlyMode() == GravityBehavior::Flying &&
				who->CheckLosFN(x,y,z,target->GetSize())) {
		auto iter = _impl->Entries.find(who);
		auto &ent = (*iter);

		PushFlyTo(ent.second, x, y, z, mob_movement_mode);
		PushStopMoving(ent.second);
		}
	// Below for npcs that can traverse land or water so they don't sink
	else if (who->GetFlyMode() == GravityBehavior::Water &&
			 zone->watermap->InLiquid(who->GetPosition()) &&
			 zone->watermap->InLiquid(glm::vec3(x, y, z)) &&
			 zone->zonemap->CheckLoS(who->GetPosition(), glm::vec3(x, y, z))) {
		auto iter = _impl->Entries.find(who);
		auto &ent = (*iter);

		PushSwimTo(ent.second, x, y, z, mob_movement_mode);
		PushStopMoving(ent.second);
	}
	else {
		UpdatePathGround(who, x, y, z, mob_movement_mode);
	}
}

void MobMovementManager::UpdatePathGround(Mob *who, float x, float y, float z, MobMovementMode mode)
{
	PathfinderOptions opts;
	opts.smooth_path = true;
	opts.step_size   = RuleR(Pathing, NavmeshStepSize);
	opts.offset      = who->GetZOffset();
	opts.flags       = PathingNotDisabled ^ PathingZoneLine;

	//This is probably pointless since the nav mesh tool currently sets zonelines to disabled anyway
	auto partial = false;
	auto stuck   = false;
	auto route   = zone->pathing->FindPath(
		glm::vec3(who->GetX(), who->GetY(), who->GetZ()),
		glm::vec3(x, y, z),
		partial,
		stuck,
		opts
	);

	auto eiter = _impl->Entries.find(who);
	auto &ent  = (*eiter);

	if (route.size() == 0) {
		HandleStuckBehavior(who, x, y, z, mode);
		return;
	}

	AdjustRoute(route, who);

	//avoid doing any processing if the mob is stuck to allow normal stuck code to work.
	if (!stuck) {

		//there are times when the routes returned are no differen than where the mob is currently standing. What basically happens
		//is a mob will get 'stuck' in such a way that it should be moving but the 'moving' place is the exact same spot it is at.
		//this is a problem and creates an area of ground that if a mob gets to, will stay there forever. If socal this creates a
		//"Ball of Death" (tm). This code tries to prevent this by simply warping the mob to the requested x/y. Better to have a warp than
		//have stuck mobs.

		auto routeNode   = route.begin();
		bool noValidPath = true;
		while (routeNode != route.end() && noValidPath == true) {
			auto &currentNode = (*routeNode);

			if (routeNode == route.end()) {
				continue;
			}

			if (!(currentNode.pos.x == who->GetX() && currentNode.pos.y == who->GetY())) {
				//if one of the nodes to move to, is not our current node, pass it.
				noValidPath = false;
				break;
			}
			//move to the next node
			routeNode++;

		}

		if (noValidPath) {
			//we are 'stuck' in a path, lets just get out of this by 'teleporting' to the next position.
			PushTeleportTo(
				ent.second,
				x,
				y,
				z,
				CalculateHeadingAngleBetweenPositions(who->GetX(), who->GetY(), x, y)
			);

			return;
		}

	}

	auto iter = route.begin();

	glm::vec3 previous_pos(who->GetX(), who->GetY(), who->GetZ());

	bool first_node = true;
	while (iter != route.end()) {
		auto &current_node = (*iter);

		iter++;

		if (iter == route.end()) {
			continue;
		}

		previous_pos = current_node.pos;
		auto &next_node = (*iter);

		if (first_node) {

			if (mode == MovementWalking) {
				auto h = who->CalculateHeadingToTarget(next_node.pos.x, next_node.pos.y);
				PushRotateTo(ent.second, who, h, mode);
			}

			first_node = false;
		}

		//move to / teleport to node + 1
		if (next_node.teleport && next_node.pos.x != 0.0f && next_node.pos.y != 0.0f) {
			PushTeleportTo(
				ent.second,
				next_node.pos.x,
				next_node.pos.y,
				next_node.pos.z,
				CalculateHeadingAngleBetweenPositions(
					current_node.pos.x,
					current_node.pos.y,
					next_node.pos.x,
					next_node.pos.y
				)
			);
		}
		else if(!next_node.teleport) {
			if (zone->watermap->InLiquid(previous_pos)) {
				PushSwimTo(ent.second, next_node.pos.x, next_node.pos.y, next_node.pos.z, mode);
			}
			else {
				PushMoveTo(ent.second, next_node.pos.x, next_node.pos.y, next_node.pos.z, mode);
			}
		}
	}

	if (stuck) {
		HandleStuckBehavior(who, x, y, z, mode);
	}
	else {
		PushStopMoving(ent.second);
	}
}

void MobMovementManager::UpdatePathUnderwater(Mob *who, float x, float y, float z, MobMovementMode movement_mode)
{
	auto eiter = _impl->Entries.find(who);
	auto &ent  = (*eiter);
	if (zone->watermap->InLiquid(who->GetPosition()) && zone->watermap->InLiquid(glm::vec3(x, y, z)) &&
		zone->zonemap->CheckLoS(who->GetPosition(), glm::vec3(x, y, z))) {
		PushSwimTo(ent.second, x, y, z, movement_mode);
		PushStopMoving(ent.second);
		return;
	}

	PathfinderOptions opts;
	opts.smooth_path = true;
	opts.step_size   = RuleR(Pathing, NavmeshStepSize);
	opts.offset      = who->GetZOffset();
	opts.flags       = PathingNotDisabled ^ PathingZoneLine;

	auto partial = false;
	auto stuck   = false;
	auto route   = zone->pathing->FindPath(
		glm::vec3(who->GetX(), who->GetY(), who->GetZ()),
		glm::vec3(x, y, z),
		partial,
		stuck,
		opts
	);

	if (route.size() == 0) {
		HandleStuckBehavior(who, x, y, z, movement_mode);
		return;
	}

	AdjustRoute(route, who);

	auto      iter       = route.begin();
	glm::vec3 previous_pos(who->GetX(), who->GetY(), who->GetZ());
	bool      first_node = true;

	while (iter != route.end()) {
		auto &current_node = (*iter);

		if (!zone->watermap->InLiquid(current_node.pos)) {
			stuck = true;

			while (iter != route.end()) {
				iter = route.erase(iter);
			}

			break;
		}
		else {
			iter++;
		}
	}

	if (route.size() == 0) {
		HandleStuckBehavior(who, x, y, z, movement_mode);
		return;
	}

	iter = route.begin();

	while (iter != route.end()) {
		auto &current_node = (*iter);

		iter++;

		if (iter == route.end()) {
			continue;
		}

		previous_pos = current_node.pos;
		auto &next_node = (*iter);

		if (first_node) {

			if (movement_mode == MovementWalking) {
				auto h = who->CalculateHeadingToTarget(next_node.pos.x, next_node.pos.y);
				PushRotateTo(ent.second, who, h, movement_mode);
			}

			first_node = false;
		}

		//move to / teleport to node + 1
		if (next_node.teleport && next_node.pos.x != 0.0f && next_node.pos.y != 0.0f) {
			PushTeleportTo(
				ent.second, next_node.pos.x, next_node.pos.y, next_node.pos.z,
				CalculateHeadingAngleBetweenPositions(
					current_node.pos.x,
					current_node.pos.y,
					next_node.pos.x,
					next_node.pos.y
				));
		}
		else if(!next_node.teleport) {
			PushSwimTo(ent.second, next_node.pos.x, next_node.pos.y, next_node.pos.z, movement_mode);
		}
	}

	if (stuck) {
		HandleStuckBehavior(who, x, y, z, movement_mode);
	}
	else {
		PushStopMoving(ent.second);
	}
}

void MobMovementManager::UpdatePathBoat(Mob *who, float x, float y, float z, MobMovementMode mode)
{
	auto eiter = _impl->Entries.find(who);
	auto &ent  = (*eiter);
	float to = who->CalculateHeadingToTarget(x, y);

	PushRotateTo(ent.second, who, to, mode);
	PushSwimTo(ent.second, x, y, z, mode);
	PushStopMoving(ent.second);
}

void MobMovementManager::PushTeleportTo(MobMovementEntry &ent, float x, float y, float z, float heading)
{
	ent.Commands.emplace_back(std::unique_ptr<IMovementCommand>(new TeleportToCommand(x, y, z, heading)));
}

void MobMovementManager::PushMoveTo(MobMovementEntry &ent, float x, float y, float z, MobMovementMode mob_movement_mode)
{
	ent.Commands.emplace_back(std::unique_ptr<IMovementCommand>(new MoveToCommand(x, y, z, mob_movement_mode)));
}

void MobMovementManager::PushSwimTo(MobMovementEntry &ent, float x, float y, float z, MobMovementMode mob_movement_mode)
{
	ent.Commands.emplace_back(std::unique_ptr<IMovementCommand>(new SwimToCommand(x, y, z, mob_movement_mode)));
}

void MobMovementManager::PushRotateTo(MobMovementEntry &ent, Mob *who, float to, MobMovementMode mob_movement_mode)
{
	auto from = FixHeading(who->GetHeading());
	to = FixHeading(to);

	float diff = to - from;

	if (std::abs(diff) < 0.001f) {
		return;
	}

	while (diff < -256.0) {
		diff += 512.0;
	}

	while (diff > 256) {
		diff -= 512.0;
	}

	ent.Commands.emplace_back(std::unique_ptr<IMovementCommand>(new RotateToCommand(to, diff > 0 ? 1.0 : -1.0, mob_movement_mode)));
}

void MobMovementManager::PushFlyTo(MobMovementEntry &ent, float x, float y, float z, MobMovementMode mob_movement_mode)
{
	ent.Commands.emplace_back(std::unique_ptr<IMovementCommand>(new FlyToCommand(x, y, z, mob_movement_mode)));
}

void MobMovementManager::PushStopMoving(MobMovementEntry &mob_movement_entry)
{
	mob_movement_entry.Commands.emplace_back(std::unique_ptr<IMovementCommand>(new StopMovingCommand()));
}

void MobMovementManager::PushEvadeCombat(MobMovementEntry &mob_movement_entry)
{
	mob_movement_entry.Commands.emplace_back(std::unique_ptr<IMovementCommand>(new EvadeCombatCommand()));
}

void MobMovementManager::HandleStuckBehavior(Mob *who, float x, float y, float z, MobMovementMode mob_movement_mode)
{
	LogDebug("Handle stuck behavior for {0} at ({1}, {2}, {3}) with movement_mode {4}", who->GetName(), x, y, z, static_cast<int>(mob_movement_mode));

	auto sb = who->GetStuckBehavior();
	MobStuckBehavior behavior = RunToTarget;

	if (sb >= 0 && sb < MaxStuckBehavior) {
		behavior = (MobStuckBehavior) sb;
	}

	auto eiter = _impl->Entries.find(who);
	auto &ent = (*eiter);

	switch (sb) {
		case RunToTarget:
			PushMoveTo(ent.second, x, y, z, mob_movement_mode);
			PushStopMoving(ent.second);
			break;
		case WarpToTarget:
			PushTeleportTo(ent.second, x, y, z, 0.0f);
			PushStopMoving(ent.second);
			break;
		case TakeNoAction:
			PushStopMoving(ent.second);
			break;
		case EvadeCombat:
			PushEvadeCombat(ent.second);
			break;
	}
}
