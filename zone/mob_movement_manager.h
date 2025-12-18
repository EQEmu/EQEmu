#pragma once

#include <memory>

class Mob;
class Client;

struct RotateCommand;
struct MovementCommand;
struct MobMovementEntry;
struct PlayerPositionUpdateServer_Struct;

enum ClientRange : int
{
	ClientRangeNone = 0,
	ClientRangeClose = 1,
	ClientRangeMedium = 2,
	ClientRangeCloseMedium = 3,
	ClientRangeLong = 4,
	ClientRangeCloseLong = 5,
	ClientRangeMediumLong = 6,
	ClientRangeAny = 7
};

enum MobMovementMode : int
{
	MovementWalking = 0,
	MovementRunning = 1
};

enum MobStuckBehavior : int
{
	RunToTarget,
	WarpToTarget,
	TakeNoAction,
	EvadeCombat,
	MaxStuckBehavior
};

class MobMovementManager
{
public:
	~MobMovementManager();
	void Process();
	void AddMob(Mob *mob);
	void RemoveMob(Mob *mob);
	void AddClient(Client *client);
	void RemoveClient(Client *client);

	void RotateTo(Mob *who, float to, MobMovementMode mob_movement_mode = MovementRunning);
	void Teleport(Mob *who, float x, float y, float z, float heading);
	void NavigateTo(Mob *who, float x, float y, float z, MobMovementMode mode = MovementRunning);
	void StopNavigation(Mob *who);

	void SendCommandToClients(
		Mob *mob,
		float delta_x,
		float delta_y,
		float delta_z,
		float delta_heading,
		int anim,
		ClientRange range,
		Client* single_client = nullptr,
		Client* ignore_client = nullptr
	);

	float FixHeading(float in);
	void DumpStats(Client *client);
	void ClearStats();

	static MobMovementManager &Get() {
		static MobMovementManager inst;
		return inst;
	}

private:
	MobMovementManager();
	MobMovementManager(const MobMovementManager&);
	MobMovementManager& operator=(const MobMovementManager&);

	void FillCommandStruct(PlayerPositionUpdateServer_Struct *position_update, Mob *mob, float delta_x, float delta_y, float delta_z, float delta_heading, int anim);
	void UpdatePath(Mob *who, float x, float y, float z, MobMovementMode mob_movement_mode);
	void UpdatePathGround(Mob *who, float x, float y, float z, MobMovementMode mode);
	void UpdatePathUnderwater(Mob *who, float x, float y, float z, MobMovementMode movement_mode);
	void UpdatePathBoat(Mob *who, float x, float y, float z, MobMovementMode mode);
	void PushTeleportTo(MobMovementEntry &ent, float x, float y, float z, float heading);
	void PushMoveTo(MobMovementEntry &ent, float x, float y, float z, MobMovementMode mob_movement_mode);
	void PushSwimTo(MobMovementEntry &ent, float x, float y, float z, MobMovementMode mob_movement_mode);
	void PushFlyTo(MobMovementEntry &ent, float x, float y, float z, MobMovementMode mob_movement_mode);
	void PushRotateTo(MobMovementEntry &ent, Mob *who, float to, MobMovementMode mob_movement_mode);
	void PushStopMoving(MobMovementEntry &mob_movement_entry);
	void PushEvadeCombat(MobMovementEntry &mob_movement_entry);
	void HandleStuckBehavior(Mob *who, float x, float y, float z, MobMovementMode mob_movement_mode);

	struct Implementation;
	std::unique_ptr<Implementation> _impl;
};
