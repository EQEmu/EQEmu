#pragma once

#include "common/eq_packet_structs.h"
#include "common/eq_packet.h"
#include "common/rulesys.h"
#include "common/timer.h"

#include "glm/vec3.hpp"

class CheatManager;
class Client;

typedef enum {
	Collision = 1,
	TeleportB,
	TeleportA,
	ZoneLine,
	Unknown0x5,
	Unknown0x6,
	SpellA, // Titanium - UF
	Unknown0x8,
	SpellB // Used in RoF+
} UpdateMovementType;

typedef enum {
	ShadowStep,
	KnockBack,
	Port,
	Assist,
	Sense,
	MAX_EXEMPTIONS
} ExemptionType;

typedef enum {
	MQWarp,
	MQWarpShadowStep,
	MQWarpKnockBack,
	MQWarpLight,
	MQZone,
	MQZoneUnknownDest,
	MQGate,
	MQGhost,
	MQFastMem,
	MQWarpAbsolute
} CheatTypes;

class CheatManager {
public:
	CheatManager()
	{
		SetExemptStatus(ShadowStep, false);
		SetExemptStatus(KnockBack, false);
		SetExemptStatus(Port, false);
		SetExemptStatus(Assist, false);
		SetExemptStatus(Sense, false);
		m_distance_since_last_position_check = 0.0f;
		m_cheat_detect_moved                 = false;
		m_target                             = nullptr;
		m_time_since_last_memorization       = 0;
		m_time_since_last_position_check     = 0;
		m_time_since_last_warp_detection.Start();
		m_time_since_last_movement_history.Start(70000);
		m_warp_counter = 0;
	}
	void SetClient(Client *cli);
	void SetExemptStatus(ExemptionType type, bool v);
	bool GetExemptStatus(ExemptionType type);
	void CheatDetected(CheatTypes type, glm::vec3 position1, glm::vec3 position2 = glm::vec3(0, 0, 0));
	void MovementCheck(glm::vec3 updated_position);
	void MovementCheck(uint32 time_between_checks = 1000);
	void CheckMemTimer();
	void ProcessMovementHistory(const EQApplicationPacket *app);
	void ProcessSpawnApperance(uint16 spawn_id, uint16 type, uint32 parameter);
	void ProcessItemVerifyRequest(int32 slot_id, uint32 target_id);
	void ClientProcess();
private:
	bool  m_exemption[ExemptionType::MAX_EXEMPTIONS]{};
	float m_distance_since_last_position_check;
	bool  m_cheat_detect_moved;

	Client *m_target;
	uint32 m_time_since_last_position_check;
	uint32 m_time_since_last_memorization;
	uint32 m_time_since_last_action{};
	Timer  m_time_since_last_warp_detection;
	Timer  m_time_since_last_movement_history;
	uint32 m_warp_counter;
};
