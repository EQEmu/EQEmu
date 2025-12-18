#include "cheat_manager.h"

#include "common/events/player_event_logs.h"
#include "zone/client.h"
#include "zone/quest_parser_collection.h"
#include "zone/worldserver.h"
#include "zone/queryserv.h"

extern WorldServer worldserver;
extern QueryServ *QServ;

void CheatManager::SetClient(Client *cli)
{
	m_target = cli;
}

void CheatManager::SetExemptStatus(ExemptionType type, bool v)
{
	if (v) {
		MovementCheck();
	}
	m_exemption[type] = v;
}

bool CheatManager::GetExemptStatus(ExemptionType type)
{
	return m_exemption[type];
}

void CheatManager::CheatDetected(CheatTypes type, glm::vec3 position1, glm::vec3 position2)
{
	switch (type) {
		case MQWarp:
			if (m_time_since_last_warp_detection.GetRemainingTime() == 0 && RuleB(Cheat, EnableMQWarpDetector) &&
				((m_target->Admin() < RuleI(Cheat, MQWarpExemptStatus) || (RuleI(Cheat, MQWarpExemptStatus)) == -1))) {
				std::string message = fmt::format(
					"/MQWarp (large warp detection) with location from x [{:.2f}] y [{:.2f}] z [{:.2f}] to x [{:.2f}] y [{:.2f}] z [{:.2f}] Distance [{:.2f}]",
					position1.x,
					position1.y,
					position1.z,
					position2.x,
					position2.y,
					position2.z,
					Distance(position1, position2)
				);

				RecordPlayerEventLogWithClient(m_target, PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});

				LogCheat(fmt::runtime(message));

				if (parse->PlayerHasQuestSub(EVENT_WARP)) {
					const auto& export_string = fmt::format(
						"{} {} {}",
						position1.x,
						position1.y,
						position1.z
					);

					parse->EventPlayer(EVENT_WARP, m_target, export_string, 0);
				}
			}
			break;
		case MQWarpAbsolute:
			if (RuleB(Cheat, EnableMQWarpDetector) &&
				((m_target->Admin() < RuleI(Cheat, MQWarpExemptStatus) || (RuleI(Cheat, MQWarpExemptStatus)) == -1))) {
				std::string message = fmt::format(
					"/MQWarp (Absolute) with location from x [{:.2f}] y [{:.2f}] z [{:.2f}] to x [{:.2f}] y [{:.2f}] z [{:.2f}] Distance [{:.2f}]",
					position1.x,
					position1.y,
					position1.z,
					position2.x,
					position2.y,
					position2.z,
					Distance(position1, position2)
				);
				RecordPlayerEventLogWithClient(m_target, PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});
				LogCheat(fmt::runtime(message));

				if (parse->PlayerHasQuestSub(EVENT_WARP)) {
					const auto& export_string = fmt::format(
						"{} {} {}",
						position1.x,
						position1.y,
						position1.z
					);

					parse->EventPlayer(EVENT_WARP, m_target, export_string, 0);
				}

				m_time_since_last_warp_detection.Start(2500);
			}
			break;
		case MQWarpShadowStep:
			if (RuleB(Cheat, EnableMQWarpDetector) &&
				((m_target->Admin() < RuleI(Cheat, MQWarpExemptStatus) || (RuleI(Cheat, MQWarpExemptStatus)) == -1))) {
				std::string message = fmt::format(
					"/MQWarp (ShadowStep) with location from x [{:.2f}] y [{:.2f}] z [{:.2f}] the target was shadow step exempt but we still found this suspicious.",
					position1.x,
					position1.y,
					position1.z
				);
				RecordPlayerEventLogWithClient(m_target, PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});
				LogCheat(fmt::runtime(message));
			}
			break;
		case MQWarpKnockBack:
			if (RuleB(Cheat, EnableMQWarpDetector) &&
				((m_target->Admin() < RuleI(Cheat, MQWarpExemptStatus) || (RuleI(Cheat, MQWarpExemptStatus)) == -1))) {
				std::string message = fmt::format(
					"/MQWarp (Knockback) with location from x [{:.2f}] y [{:.2f}] z [{:.2f}] the target was Knock Back exempt but we still found this suspicious.",
					position1.x,
					position1.y,
					position1.z
				);
				RecordPlayerEventLogWithClient(m_target, PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});
				LogCheat(fmt::runtime(message));
			}
			break;

		case MQWarpLight:
			if (RuleB(Cheat, EnableMQWarpDetector) &&
				((m_target->Admin() < RuleI(Cheat, MQWarpExemptStatus) || (RuleI(Cheat, MQWarpExemptStatus)) == -1))) {
				if (RuleB(Cheat, MarkMQWarpLT)) {
					std::string message = fmt::format(
						"/MQWarp(Knockback) with location from x [{:.2f}] y [{:.2f}] z [{:.2f}] running fast but not fast enough to get killed, possibly: small warp, speed hack, excessive lag, marked as suspicious.",
						position1.x,
						position1.y,
						position1.z
					);
					RecordPlayerEventLogWithClient(m_target, PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});
					LogCheat(fmt::runtime(message));
				}
			}
			break;

		case MQZone:
			if (RuleB(Cheat, EnableMQZoneDetector) &&
				((m_target->Admin() < RuleI(Cheat, MQZoneExemptStatus) || (RuleI(Cheat, MQZoneExemptStatus)) == -1))) {
				std::string message = fmt::format(
					"/MQZone used at x [{:.2f}] y [{:.2f}] z [{:.2f}]",
					position1.x,
					position1.y,
					position1.z
				);
				RecordPlayerEventLogWithClient(m_target, PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});
				LogCheat(fmt::runtime(message));
			}
			break;
		case MQZoneUnknownDest:
			if (RuleB(Cheat, EnableMQZoneDetector) &&
				((m_target->Admin() < RuleI(Cheat, MQZoneExemptStatus) || (RuleI(Cheat, MQZoneExemptStatus)) == -1))) {
				std::string message = fmt::format(
					"/MQZone used at x [{:.2f}] y [{:.2f}] z [{:.2f}] with Unknown Destination",
					position1.x,
					position1.y,
					position1.z
				);
				RecordPlayerEventLogWithClient(m_target, PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});
				LogCheat(fmt::runtime(message));
			}
			break;
		case MQGate:
			if (RuleB(Cheat, EnableMQGateDetector) &&
				((m_target->Admin() < RuleI(Cheat, MQGateExemptStatus) || (RuleI(Cheat, MQGateExemptStatus)) == -1))) {
				std::string message = fmt::format(
					"/MQGate used at x [{:.2f}] y [{:.2f}] z [{:.2f}]",
					position1.x,
					position1.y,
					position1.z
				);
				RecordPlayerEventLogWithClient(m_target, PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});
				LogCheat(fmt::runtime(message));
			}
			break;
		case MQGhost:
			// this isn't just for ghost, its also for if a person isn't sending their MovementHistory packet also.
			if (RuleB(Cheat, EnableMQGhostDetector) &&
				((m_target->Admin() < RuleI(Cheat, MQGhostExemptStatus) ||
				  (RuleI(Cheat, MQGhostExemptStatus)) == -1))) {
				std::string message = fmt::format(
					"[MQGhost] [{}] [{}] was caught not sending the proper packets as regularly as they were suppose to.",
					m_target->AccountName(),
					m_target->GetName()
				);
				RecordPlayerEventLogWithClient(m_target, PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});
				LogCheat("{}", message);
			}
			break;
		case MQFastMem:
			if (RuleB(Cheat, EnableMQFastMemDetector) &&
				((m_target->Admin() < RuleI(Cheat, MQFastMemExemptStatus) ||
				  (RuleI(Cheat, MQFastMemExemptStatus)) == -1))) {
				std::string message = fmt::format(
					"/MQFastMem used at x [{:.2f}] y [{:.2f}] z [{:.2f}]",
					position1.x,
					position1.y,
					position1.z
				);
				RecordPlayerEventLogWithClient(m_target, PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});
				LogCheat(fmt::runtime(message));
			}
			break;
		default:
			std::string message = fmt::format(
				"Unhandled HackerDetection flag with location from x [{:.2f}] y [{:.2f}] z [{:.2f}]",
				position1.x,
				position1.y,
				position1.z
			);
			RecordPlayerEventLogWithClient(m_target, PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});
			LogCheat(fmt::runtime(message));
			break;
	}
}

void CheatManager::MovementCheck(glm::vec3 updated_position)
{
	if (m_time_since_last_movement_history.GetRemainingTime() == 0) {
		CheatDetected(MQGhost, updated_position);
	}

	float  dist     = DistanceNoZ(m_target->GetPosition(), updated_position);
	uint32 cur_time = Timer::GetCurrentTime();
	if (dist == 0) {
		if (m_distance_since_last_position_check > 0.0f) {
			MovementCheck(0);
		}
		else {
			m_time_since_last_position_check = cur_time;
			m_cheat_detect_moved             = false;
		}
	}
	else {
		m_distance_since_last_position_check += dist;
		m_cheat_detect_moved = true;
		if (m_time_since_last_position_check == 0) {
			m_time_since_last_position_check = cur_time;
		}
		else {
			MovementCheck(2500);
		}
	}
}

void CheatManager::MovementCheck(uint32 time_between_checks)
{
	uint32 cur_time = Timer::GetCurrentTime();
	if ((cur_time - m_time_since_last_position_check) > time_between_checks) {
		float estimated_speed =
				  (m_distance_since_last_position_check * 100) / (float) (cur_time - m_time_since_last_position_check);

		// MQWarpDetection shouldn't go below 1.0f so we can't end up dividing by 0.
		float run_speed = m_target->GetRunspeed() /
						  std::min(
							  RuleR(Cheat, MQWarpDetectionDistanceFactor),
							  1.0f
						  );
		if (estimated_speed > run_speed) {
			bool using_gm_speed = m_target->GetGMSpeed();
			bool is_immobile    = m_target->GetRunspeed() == 0; // this covers stuns, roots, mez, and pseudorooted.
			if (!using_gm_speed && !is_immobile) {
				if (GetExemptStatus(ShadowStep)) {
					if (m_distance_since_last_position_check > 800) {
						CheatDetected(
							MQWarpShadowStep,
							glm::vec3(
								m_target->GetX(),
								m_target->GetY(),
								m_target->GetZ()
							)
						);
					}
				}
				else if (GetExemptStatus(KnockBack)) {
					if (estimated_speed > 30.0f) {
						CheatDetected(MQWarpKnockBack, glm::vec3(m_target->GetX(), m_target->GetY(), m_target->GetZ()));
					}
				}
				else if (!GetExemptStatus(Port)) {
					if (estimated_speed > (run_speed * 1.5)) {
						CheatDetected(MQWarp, glm::vec3(m_target->GetX(), m_target->GetY(), m_target->GetZ()));
						m_time_since_last_position_check     = cur_time;
						m_distance_since_last_position_check = 0.0f;
					}
					else {
						CheatDetected(MQWarpLight, glm::vec3(m_target->GetX(), m_target->GetY(), m_target->GetZ()));
					}
				}
			}
		}
		if (time_between_checks != 1000) {
			SetExemptStatus(ShadowStep, false);
			SetExemptStatus(KnockBack, false);
			SetExemptStatus(Port, false);
		}
		m_time_since_last_position_check     = cur_time;
		m_distance_since_last_position_check = 0.0f;
	}
}

void CheatManager::CheckMemTimer()
{
	if (m_target == nullptr) {
		return;
	}
	if (m_time_since_last_memorization - Timer::GetCurrentTime() <= 1) {
		glm::vec3 pos = m_target->GetPosition();
		CheatDetected(MQFastMem, pos);
	}
	m_time_since_last_memorization = Timer::GetCurrentTime();
}

void CheatManager::ProcessMovementHistory(const EQApplicationPacket *app)
{
	// if they haven't sent sent the packet within this time... they are probably spoofing...
	// linux users reported that they don't send this packet at all but i can't prove they don't so i'm not sure if thats a fake or not.
	m_time_since_last_movement_history.Start(70000);
	if (GetExemptStatus(Port)) {
		return;
	}
	auto *m_MovementHistory = (UpdateMovementEntry *) app->pBuffer;
	if (app->size < sizeof(UpdateMovementEntry)) {
		LogDebug(
			"Size mismatch in OP_MovementHistoryList, expected {}, got [{}]",
			sizeof(UpdateMovementEntry),
			app->size
		);
		DumpPacket(app);
		return;
	}

	for (int index = 0; index < (app->size) / sizeof(UpdateMovementEntry); index++) {
		glm::vec3 to = glm::vec3(m_MovementHistory[index].X, m_MovementHistory[index].Y, m_MovementHistory[index].Z);
		switch (m_MovementHistory[index].type) {
			case UpdateMovementType::ZoneLine:
				SetExemptStatus(Port, true);
				break;
			case UpdateMovementType::TeleportA:
				if (index != 0) {
					glm::vec3 from = glm::vec3(
						m_MovementHistory[index - 1].X,
						m_MovementHistory[index - 1].Y,
						m_MovementHistory[index - 1].Z
					);
					CheatDetected(MQWarpAbsolute, from, to);
				}
				SetExemptStatus(Port, false);
				break;
		}
	}
}

void CheatManager::ProcessSpawnApperance(uint16 spawn_id, uint16 type, uint32 parameter)
{
	if (type == AppearanceType::Animation && parameter == Animation::Sitting) {
		m_time_since_last_memorization = Timer::GetCurrentTime();
	}
	else if (spawn_id == 0 && type == AppearanceType::AntiCheat) {
		m_time_since_last_action = parameter;
	}
}

void CheatManager::ProcessItemVerifyRequest(int32 slot_id, uint32 target_id)
{
	if (slot_id == -1 && m_warp_counter != target_id) {
		m_warp_counter = target_id;
	}
}

void CheatManager::ClientProcess()
{
	if (!m_cheat_detect_moved) {
		m_time_since_last_position_check = Timer::GetCurrentTime();
	}
}
