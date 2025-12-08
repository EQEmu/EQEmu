/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2003 EQEMu Development Team (http://eqemulator.net)

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

	client_process.cpp:
	Handles client login sequence and packets sent from client to zone
*/

#include "../common/eqemu_logsys.h"
#include "../common/global_define.h"
#include <iostream>

#ifdef _WINDOWS
	#define snprintf	_snprintf
	#define strncasecmp	_strnicmp
	#define strcasecmp	_stricmp
#else
	#include <pthread.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <unistd.h>
#endif

#include "../common/data_verification.h"
#include "../common/rulesys.h"
#include "../common/skills.h"
#include "../common/spdat.h"
#include "../common/strings.h"
#include "dynamic_zone.h"
#include "event_codes.h"
#include "guild_mgr.h"
#include "map.h"
#include "petitions.h"
#include "queryserv.h"
#include "quest_parser_collection.h"
#include "string_ids.h"
#include "worldserver.h"
#include "zone.h"
#include "zonedb.h"
#include "../common/events/player_event_logs.h"
#include "water_map.h"

extern QueryServ* QServ;
extern Zone* zone;
extern volatile bool is_zone_loaded;
extern WorldServer worldserver;
extern EntityList entity_list;

bool Client::Process() {
	bool ret = true;

	if (Connected() || IsLD()) {
		// try to send all packets that weren't sent before
		if (!IsLD() && zoneinpacket_timer.Check()) {
			SendAllPackets();
		}

		if (adventure_request_timer) {
			if (adventure_request_timer->Check()) {
				safe_delete(adventure_request_timer);
			}
		}

		if (adventure_create_timer) {
			if (adventure_create_timer->Check()) {
				safe_delete(adventure_create_timer);
			}
		}

		if (adventure_leave_timer) {
			if (adventure_leave_timer->Check()) {
				safe_delete(adventure_leave_timer);
			}
		}

		if (adventure_door_timer) {
			if (adventure_door_timer->Check()) {
				safe_delete(adventure_door_timer);
			}
		}

		if (adventure_stats_timer) {
			if (adventure_stats_timer->Check()) {
				safe_delete(adventure_stats_timer);
			}
		}

		if (adventure_leaderboard_timer) {
			if (adventure_leaderboard_timer->Check()) {
				safe_delete(adventure_leaderboard_timer);
			}
		}

		if (dead) {
			SetHP(-100);
			if (RespawnFromHoverTimer.Check())
				HandleRespawnFromHover(0);
		}

		if (IsTracking() && (ClientVersion() >= EQ::versions::ClientVersion::SoD) && TrackingTimer.Check())
			DoTracking();

		// SendHPUpdate calls hpupdate_timer.Start so it can delay this timer, so lets not reset with the check
		// since the function will anyways
		if (hpupdate_timer.Check(false)) {
			SendHPUpdate();
		}

		/* I haven't naturally updated my position in 10 seconds, updating manually */
		if (!IsMoving() && m_position_update_timer.Check()) {
			BroadcastPositionUpdate();
		}

		if (mana_timer.Check())
			CheckManaEndUpdate();

		if (dead && dead_timer.Check()) {
			database.MoveCharacterToZone(GetName(), m_pp.binds[0].zone_id);

			m_pp.zone_id = m_pp.binds[0].zone_id;
			m_pp.zoneInstance = m_pp.binds[0].instance_id;
			m_pp.x = m_pp.binds[0].x;
			m_pp.y = m_pp.binds[0].y;
			m_pp.z = m_pp.binds[0].z;
			Save();

			Group *mygroup = GetGroup();
			if (mygroup)
			{
				entity_list.MessageGroup(this, true, 15, "%s died.", GetName());
				mygroup->MemberZoned(this);
			}
			Raid *myraid = entity_list.GetRaidByClient(this);
			if (myraid)
			{
				myraid->MemberZoned(this);
			}
			return(false);
		}

		if (charm_update_timer.Check()) {
			CalcItemScale();
		}

		if (TaskPeriodic_Timer.Check() && task_state)
			task_state->TaskPeriodicChecks(this);

		if (dynamiczone_removal_timer.Check() && zone && zone->GetInstanceID() != 0)
		{
			dynamiczone_removal_timer.Disable();
			GoToDzSafeReturnOrBind(zone->GetDynamicZone());
		}

		if (linkdead_timer.Check()) {
			LeaveGroup();
			Save();
			if (GetMerc()) {
				GetMerc()->Save();
				GetMerc()->Depop();
			}

			Raid *myraid = entity_list.GetRaidByClient(this);
			if (myraid) {
				myraid->MemberZoned(this);
			}
			if (IsInAGuild()) {
				guild_mgr.UpdateDbMemberOnline(CharacterID(), false);
				guild_mgr.SendGuildMemberUpdateToWorld(GetName(), GuildID(), 0, time(nullptr));
			}

			SetDynamicZoneMemberStatus(DynamicZoneMemberStatus::Offline);

			RecordPlayerEventLog(PlayerEvent::WENT_OFFLINE, PlayerEvent::EmptyEvent{});

			if (parse->PlayerHasQuestSub(EVENT_DISCONNECT)) {
				parse->EventPlayer(EVENT_DISCONNECT, this, "", 0);
			}

			return false; //delete client
		}

		if (RuleB(Bots, Enabled)) {
			if (bot_camp_timer.Check()) {
				CampAllBots();
			}
		}

		if (camp_timer.Check()) {
			Raid *myraid = entity_list.GetRaidByClient(this);
			if (myraid) {
				myraid->MemberZoned(this);
			}
			LeaveGroup();
			Save();
			if (IsInAGuild()) {
				guild_mgr.UpdateDbMemberOnline(CharacterID(), false);
				guild_mgr.SendGuildMemberUpdateToWorld(GetName(), GuildID(), 0, time(nullptr));
			}

			if (GetMerc())
			{
				GetMerc()->Save();
				GetMerc()->Depop();
			}
			instalog = true;

			camp_timer.Disable();
		}

		if (IsStunned() && stunned_timer.Check())
			Mob::UnStun();

		cheat_manager.ClientProcess();

		if (bardsong_timer.Check() && bardsong != 0) {
			//NOTE: this is kinda a heavy-handed check to make sure the mob still exists before
			//doing the next pulse on them...
			Mob *song_target = nullptr;
			if (bardsong_target_id == GetID()) {
				song_target = this;
			}
			else {
				song_target = entity_list.GetMob(bardsong_target_id);
			}

			if (song_target == nullptr) {
				InterruptSpell(SONG_ENDS_ABRUPTLY, 0x121, bardsong);
			}
			else {
				if (!ApplyBardPulse(bardsong, song_target, bardsong_slot)) {
					InterruptSpell(SONG_ENDS_ABRUPTLY, 0x121, bardsong);
				}
			}
		}

		if (GetMerc()) {
			UpdateMercTimer();
		}

		if (GetMercInfo().MercTemplateID != 0 && GetMercInfo().IsSuspended) {
			CheckMercSuspendTimer();
		}

		if (IsAIControlled())
			AI_Process();

		// Don't reset the bindwound timer so we can check it in BindWound as well.
		if (bindwound_timer.Check(false) && bindwound_target != 0) {
			BindWound(bindwound_target, false);
		}

		if (KarmaUpdateTimer) {
			if (KarmaUpdateTimer->Check(false)) {
				KarmaUpdateTimer->Start(RuleI(Chat, KarmaUpdateIntervalMS));
				database.UpdateKarma(AccountID(), ++TotalKarma);
			}
		}

		if (qGlobals) {
			if (qglobal_purge_timer.Check()) {
				qGlobals->PurgeExpiredGlobals();
			}
		}

		if (RuleB(Character, ActiveInvSnapshots) && time(nullptr) >= GetNextInvSnapshotTime()) {
			if (database.SaveCharacterInvSnapshot(CharacterID())) {
				SetNextInvSnapshot(RuleI(Character, InvSnapshotMinIntervalM));
				LogInventory("Successful inventory snapshot taken of [{}] - setting next interval for [{}] minute[{}]",
					GetName(), RuleI(Character, InvSnapshotMinIntervalM), (RuleI(Character, InvSnapshotMinIntervalM) == 1 ? "" : "s"));
			}
			else {
				SetNextInvSnapshot(RuleI(Character, InvSnapshotMinRetryM));
				LogInventory("Failed to take inventory snapshot of [{}] - retrying in [{}] minute[{}]",
					GetName(), RuleI(Character, InvSnapshotMinRetryM), (RuleI(Character, InvSnapshotMinRetryM) == 1 ? "" : "s"));
			}
		}

		if (m_scan_close_mobs_timer.Check()) {
			entity_list.ScanCloseMobs(this);
		}

		if (RuleB(Inventory, LazyLoadBank)) {
			// poll once a second to see if we are close to a banker and we haven't loaded the bank yet
			if (!m_lazy_load_bank && lazy_load_bank_check_timer.Check()) {
				if (m_lazy_load_sent_bank_slots <= EQ::invslot::SHARED_BANK_END && IsCloseToBanker()) {
					m_lazy_load_bank = true;
					lazy_load_bank_check_timer.Disable();
				}
			}

			int lazy_load_bank_slots = 0;
			for (int i = 0; i < 5000; i++) {
				if (m_lazy_load_bank && m_lazy_load_sent_bank_slots <= EQ::invslot::SHARED_BANK_END) {
					const EQ::ItemInstance *inst = nullptr;

					// Jump the gaps
					if (m_lazy_load_sent_bank_slots < EQ::invslot::BANK_BEGIN) {
						m_lazy_load_sent_bank_slots = EQ::invslot::BANK_BEGIN;
					}
					else if (m_lazy_load_sent_bank_slots > EQ::invslot::BANK_END &&
							 m_lazy_load_sent_bank_slots < EQ::invslot::SHARED_BANK_BEGIN) {
						m_lazy_load_sent_bank_slots = EQ::invslot::SHARED_BANK_BEGIN;
					}
					else {
						m_lazy_load_sent_bank_slots++;
					}

					inst = m_inv[m_lazy_load_sent_bank_slots];
					if (inst) {
						SendItemPacket(m_lazy_load_sent_bank_slots, inst, ItemPacketType::ItemPacketTrade);
						lazy_load_bank_slots++;
					}
				} else {
					break;
				}
			}
		}

		bool may_use_attacks = false;
		/*
			Things which prevent us from attacking:
				- being under AI control, the AI does attacks
				- being dead
				- casting a spell and bard check
				- not having a target
				- being stunned or mezzed
				- having used a ranged weapon recently
		*/
		if (auto_attack) {
			if (!IsAIControlled() && !dead
				&& !(spellend_timer.Enabled() && casting_spell_id && !IsBardSong(casting_spell_id))
				&& !IsStunned() && !IsFeared() && !IsMezzed() && GetAppearance() != eaDead && !IsMeleeDisabled()
				)
				may_use_attacks = true;

			if (may_use_attacks && ranged_timer.Enabled()) {
				//if the range timer is enabled, we need to consider it
				if (!ranged_timer.Check(false)) {
					//the ranged timer has not elapsed, cannot attack.
					may_use_attacks = false;
				}
			}
		}

		if (AutoFireEnabled()) {
			if (GetTarget() == this) {
				MessageString(Chat::TooFarAway, TRY_ATTACKING_SOMEONE);
				auto_fire = false;
			}
			EQ::ItemInstance *ranged = GetInv().GetItem(EQ::invslot::slotRange);
			if (ranged) {
				if (ranged->GetItem() && ranged->GetItem()->ItemType == EQ::item::ItemTypeBow) {
					if (ranged_timer.Check(false)) {
						if (GetTarget() && (GetTarget()->IsNPC() || GetTarget()->IsClient()) && IsAttackAllowed(GetTarget())) {
							if (GetTarget()->InFrontMob(this, GetTarget()->GetX(), GetTarget()->GetY())) {
								if (CheckLosFN(GetTarget()) && CheckWaterAutoFireLoS(GetTarget())) {
									//client has built in los check, but auto fire does not.. done last.
									if (RangedAttack(GetTarget()) && CheckDoubleRangedAttack()) {
										RangedAttack(GetTarget(), true);
									}
								} else {
									ranged_timer.Start();
								}
							} else {
								ranged_timer.Start();
							}
						} else {
							ranged_timer.Start();
						}
					}
				} else if (ranged->GetItem() && (ranged->GetItem()->ItemType == EQ::item::ItemTypeLargeThrowing || ranged->GetItem()->ItemType == EQ::item::ItemTypeSmallThrowing)) {
					if (ranged_timer.Check(false)) {
						if (GetTarget() && (GetTarget()->IsNPC() || GetTarget()->IsClient()) && IsAttackAllowed(GetTarget())) {
							if (GetTarget()->InFrontMob(this, GetTarget()->GetX(), GetTarget()->GetY())) {
								if (CheckLosFN(GetTarget()) && CheckWaterAutoFireLoS(GetTarget())) {
									//client has built in los check, but auto fire does not.. done last.
									ThrowingAttack(GetTarget());
								} else {
									ranged_timer.Start();
								}
							} else {
								ranged_timer.Start();
							}
						} else {
							ranged_timer.Start();
						}
					}
				}
			}
		}

		Mob *auto_attack_target = GetTarget();
		if (auto_attack && auto_attack_target != nullptr && may_use_attacks && attack_timer.Check()) {
			//check if change
			//only check on primary attack.. sorry offhand you gotta wait!
			if (aa_los_them_mob) {
				if (auto_attack_target != aa_los_them_mob ||
					m_AutoAttackPosition.x != GetX() ||
					m_AutoAttackPosition.y != GetY() ||
					m_AutoAttackPosition.z != GetZ() ||
					m_AutoAttackTargetLocation.x != aa_los_them_mob->GetX() ||
					m_AutoAttackTargetLocation.y != aa_los_them_mob->GetY() ||
					m_AutoAttackTargetLocation.z != aa_los_them_mob->GetZ()) {
					aa_los_them_mob            = auto_attack_target;
					m_AutoAttackPosition       = GetPosition();
					m_AutoAttackTargetLocation = glm::vec3(aa_los_them_mob->GetPosition());
					los_status                 = CheckLosFN(auto_attack_target);
					los_status_facing          = IsFacingMob(aa_los_them_mob);
				}
				// If only our heading changes, we can skip the CheckLosFN call
				// but above we still need to update los_status_facing
				if (m_AutoAttackPosition.w != GetHeading()) {
					m_AutoAttackPosition.w = GetHeading();
					los_status_facing = IsFacingMob(aa_los_them_mob);
				}
			}
			else {
				aa_los_them_mob            = auto_attack_target;
				m_AutoAttackPosition       = GetPosition();
				m_AutoAttackTargetLocation = glm::vec3(aa_los_them_mob->GetPosition());
				los_status                 = CheckLosFN(auto_attack_target);
				los_status_facing          = IsFacingMob(aa_los_them_mob);
			}

			if (!CombatRange(auto_attack_target)) {
				MessageString(Chat::TooFarAway, TARGET_TOO_FAR);
			}
			else if (auto_attack_target == this) {
				MessageString(Chat::TooFarAway, TRY_ATTACKING_SOMEONE);
			}
			else if (!los_status || !los_status_facing) {
				//you can't see your target
			}
			else if (auto_attack_target->GetHP() > -10 && IsAttackAllowed(auto_attack_target)) // -10 so we can watch people bleed in PvP
			{
				EQ::ItemInstance *wpn = GetInv().GetItem(EQ::invslot::slotPrimary);
				TryCombatProcs(wpn, auto_attack_target, EQ::invslot::slotPrimary);
				TriggerDefensiveProcs(auto_attack_target, EQ::invslot::slotPrimary, false);

				DoAttackRounds(auto_attack_target, EQ::invslot::slotPrimary);

				if (TryDoubleMeleeRoundEffect()) {
					DoAttackRounds(auto_attack_target, EQ::invslot::slotPrimary);
				}

				if (CheckAATimer(aaTimerRampage)) {
					entity_list.AEAttack(this, 40);
				}
			}
		}

		if (GetClass() == Class::Warrior || GetClass() == Class::Berserker) {
			if (!dead && !IsBerserk() && GetHPRatio() < RuleI(Combat, BerserkerFrenzyStart)) {
				entity_list.MessageCloseString(this, false, 200, 0, BERSERK_START, GetName());
				berserk = true;
			}
			if (IsBerserk() && GetHPRatio() > RuleI(Combat, BerserkerFrenzyEnd)) {
				entity_list.MessageCloseString(this, false, 200, 0, BERSERK_END, GetName());
				berserk = false;
			}
		}

		if (auto_attack && may_use_attacks && auto_attack_target != nullptr
			&& CanThisClassDualWield() && attack_dw_timer.Check())
		{
			// Range check
			if (!CombatRange(auto_attack_target)) {
				// this is a duplicate message don't use it.
				//MessageString(Chat::TooFarAway,TARGET_TOO_FAR);
			}
			// Don't attack yourself
			else if (auto_attack_target == this) {
				//MessageString(Chat::TooFarAway,TRY_ATTACKING_SOMEONE);
			}
			else if (!los_status || !los_status_facing)
			{
				//you can't see your target
			}
			else if (auto_attack_target->GetHP() > -10 && IsAttackAllowed(auto_attack_target)) {
				CheckIncreaseSkill(EQ::skills::SkillDualWield, auto_attack_target, -10);
				if (CheckDualWield()) {
					EQ::ItemInstance *wpn = GetInv().GetItem(EQ::invslot::slotSecondary);
					TryCombatProcs(wpn, auto_attack_target, EQ::invslot::slotSecondary);

					DoAttackRounds(auto_attack_target, EQ::invslot::slotSecondary);
				}
			}
		}

		if (viral_timer.Check() && !dead) {
			VirusEffectProcess();
		}

		ProjectileAttack();

		if (spellbonuses.GravityEffect == 1) {
			if (gravity_timer.Check())
				DoGravityEffect();
		}

		if (shield_timer.Check()) {
			ShieldAbilityFinish();
		}

		SpellProcess();
		if (endupkeep_timer.Check() && !dead) {
			DoEnduranceUpkeep();
		}

		// this is independent of the tick timer
		if (consume_food_timer.Check())
			DoStaminaHungerUpdate();

		if (tic_timer.Check() && !dead) {
			CalcMaxHP();
			CalcMaxMana();
			CalcATK();
			CalcMaxEndurance();
			CalcRestState();
			DoHPRegen();
			DoManaRegen();
			DoEnduranceRegen();
			BuffProcess();

			if (auto_attack) {
				ResetAFKTimer();
			}

			if (tribute_timer.Check()) {
				ToggleTribute(true);	//re-activate the tribute.
			}

			if (fishing_timer.Check()) {
				GoFish();
			}

			if (autosave_timer.Check()) {
				Save(0);
			}

			if (GetIntoxication() > 0)
			{
				SetIntoxication(GetIntoxication()-1);
				CalcBonuses();
			}

			if (ItemQuestTimer.Check())
			{
				ItemTimerCheck();
			}

			if (m_clear_wearchange_cache_timer.Check()) {
				m_last_seen_wearchange.clear();
			}
		}
	}

	if (client_state == CLIENT_KICKED) {
		Save();
		OnDisconnect(true);
		std::cout << "Client disconnected (cs=k): " << GetName() << std::endl;
		return false;
	}

	if (client_state == DISCONNECTED) {
		OnDisconnect(true);
		std::cout << "Client disconnected (cs=d): " << GetName() << std::endl;
		RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = "/MQInstantCamp: Possible instant camp disconnect"});
		return false;
	}

	if (client_state == CLIENT_ERROR) {
		OnDisconnect(true);
		std::cout << "Client disconnected (cs=e): " << GetName() << std::endl;
		return false;
	}

	if (client_state != CLIENT_LINKDEAD && !eqs->CheckState(ESTABLISHED)) {
		OnDisconnect(true);
		LogInfo("Client linkdead: {}", name);

		if (Admin() > AccountStatus::GMAdmin) {
			if (GetMerc()) {
				GetMerc()->Save();
				GetMerc()->Depop();
			}
			if (IsInAGuild()) {
				guild_mgr.UpdateDbMemberOnline(CharacterID(), false);
				guild_mgr.SendGuildMemberUpdateToWorld(GetName(), GuildID(), 0, time(nullptr));
			}

			return false;
		}
		else if (!linkdead_timer.Enabled()) {
			linkdead_timer.Start(RuleI(Zone, ClientLinkdeadMS));
			client_state = CLIENT_LINKDEAD;
			AI_Start(CLIENT_LD_TIMEOUT);
			SendAppearancePacket(AppearanceType::Linkdead, 1);

			SetDynamicZoneMemberStatus(DynamicZoneMemberStatus::LinkDead);
		}
	}


	/************ Get all packets from packet manager out queue and process them ************/
	EQApplicationPacket *app = nullptr;
	if (!eqs->CheckState(CLOSING))
	{
		while (app = eqs->PopPacket()) {
			HandlePacket(app);
			safe_delete(app);
		}
	}

	ClientToNpcAggroProcess();

	if (client_state != CLIENT_LINKDEAD && (client_state == CLIENT_ERROR || client_state == DISCONNECTED || client_state == CLIENT_KICKED || !eqs->CheckState(ESTABLISHED)))
	{
		//client logged out or errored out
		//ResetTrade();
		if (client_state != CLIENT_KICKED && !bZoning && !instalog) {
			Save();
		}

		client_state = CLIENT_LINKDEAD;
		if (bZoning || instalog || GetGM())
		{
			Group *mygroup = GetGroup();
			if (mygroup)
			{
				if (!bZoning)
				{
					entity_list.MessageGroup(this, true, 15, "%s logged out.", GetName());
					LeaveGroup();
				}
				else
				{
					entity_list.MessageGroup(this, true, 15, "%s left the zone.", GetName());
					mygroup->MemberZoned(this);
					if (GetMerc() && GetMerc()->HasGroup())
					{
						GetMerc()->RemoveMercFromGroup(GetMerc(), GetMerc()->GetGroup());
					}
				}

			}
			Raid *myraid = entity_list.GetRaidByClient(this);
			if (myraid)
			{
				if (!bZoning)
				{
					//entity_list.MessageGroup(this,true,15,"%s logged out.",GetName());
					myraid->MemberZoned(this);
				}
				else
				{
					//entity_list.MessageGroup(this,true,15,"%s left the zone.",GetName());
					myraid->MemberZoned(this);
				}
			}
			OnDisconnect(false);
			return false;
		}
		else
		{
			LinkDead();
		}
		OnDisconnect(true);
	}
	// Feign Death 2 minutes and zone forgets you
	if (forget_timer.Check()) {
		forget_timer.Disable();
		entity_list.ClearZoneFeignAggro(this);
		Message(0, "Your enemies have forgotten you!");
	}

	if (client_state == CLIENT_CONNECTED) {
		if (m_dirtyautohaters)
			ProcessXTargetAutoHaters();
		if (aggro_meter_timer.Check())
			ProcessAggroMeter();
	}

	return ret;
}

/* Just a set of actions preformed all over in Client::Process */
void Client::OnDisconnect(bool hard_disconnect) {
	if (hard_disconnect) {
		LeaveGroup();

		if (GetMerc()) {
			GetMerc()->Save();
			GetMerc()->Depop();
		}

		auto* r = entity_list.GetRaidByClient(this);

		if (r) {
			r->MemberZoned(this);
		}
	}

	if (!bZoning) {
		SetDynamicZoneMemberStatus(DynamicZoneMemberStatus::Offline);
	}

	RemoveAllAuras();

	auto* o = trade->With();
	if (o) {
		LogTrading("Client disconnected during a trade. Returning their items");
		FinishTrade(this);

		if (o->IsClient()) {
			o->CastToClient()->FinishTrade(o);
		}

		/* Reset both sides of the trade */
		trade->Reset();
		o->trade->Reset();
	}

	database.SetIngame(CharacterID(), 0); //We change ingame status regardless of if a player logs out to zone or not, because we only want to trigger it on their first login from world.

	/* Remove from all proximities */
	ClearAllProximities();

	auto outapp = new EQApplicationPacket(OP_LogoutReply);
	FastQueuePacket(&outapp);

	RecordPlayerEventLog(PlayerEvent::WENT_OFFLINE, PlayerEvent::EmptyEvent{});

	if (parse->PlayerHasQuestSub(EVENT_DISCONNECT)) {
		parse->EventPlayer(EVENT_DISCONNECT, this, "", 0);
	}

	RecordStats();

	Disconnect();
}

// Sends the client complete inventory used in character login
void Client::BulkSendInventoryItems()
{
	// LINKDEAD TRADE ITEMS
	// Move trade slot items back into normal inventory..need them there now for the proceeding validity checks
	for (int16 slot_id = EQ::invslot::TRADE_BEGIN; slot_id <= EQ::invslot::TRADE_END; slot_id++) {
		EQ::ItemInstance* inst = m_inv.PopItem(slot_id);
		if(inst) {
			bool is_arrow = (inst->GetItem()->ItemType == EQ::item::ItemTypeArrow) ? true : false;
			int16 free_slot_id = m_inv.FindFreeSlot(inst->IsClassBag(), true, inst->GetItem()->Size, is_arrow);
			LogInventory("Incomplete Trade Transaction: Moving [{}] from slot [{}] to [{}]", inst->GetItem()->Name, slot_id, free_slot_id);
			PutItemInInventory(free_slot_id, *inst, false);
			database.SaveInventory(character_id, nullptr, slot_id);
			safe_delete(inst);
		}
	}

	const bool delete_no_rent = database.NoRentExpired(GetName());
	if (delete_no_rent) { //client was offline for more than 30 minutes, delete no rent items
		if (RuleB(Inventory, TransformSummonedBags)) {
			DisenchantSummonedBags(false);
		}

		RemoveNoRent(false);
	}

	RemoveDuplicateLore();
	MoveSlotNotAllowed(false);

	EQ::OutBuffer ob;
	EQ::OutBuffer::pos_type last_pos = ob.tellp();

	// Possessions items
	for (int16 slot_id = EQ::invslot::POSSESSIONS_BEGIN; slot_id <= EQ::invslot::POSSESSIONS_END; slot_id++) {
		const EQ::ItemInstance* inst = m_inv[slot_id];
		if (!inst) {
			continue;
		}

		inst->Serialize(ob, slot_id);

		if (ob.tellp() == last_pos) {
			LogInventory("Serialization failed on item slot [{}] during BulkSendInventoryItems. Item skipped", slot_id);
		}

		last_pos = ob.tellp();
	}

	if (!RuleB(Inventory, LazyLoadBank)) {
		// Bank items
		for (int16 slot_id = EQ::invslot::BANK_BEGIN; slot_id <= EQ::invslot::BANK_END; slot_id++) {
			const EQ::ItemInstance* inst = m_inv[slot_id];
			if (!inst) {
				continue;
			}

			inst->Serialize(ob, slot_id);

			if (ob.tellp() == last_pos) {
				LogInventory("Serialization failed on item slot [{}] during BulkSendInventoryItems. Item skipped", slot_id);
			}

			last_pos = ob.tellp();
		}

		// SharedBank items
		for (int16 slot_id = EQ::invslot::SHARED_BANK_BEGIN; slot_id <= EQ::invslot::SHARED_BANK_END; slot_id++) {
			const EQ::ItemInstance* inst = m_inv[slot_id];
			if (!inst) {
				continue;
			}

			inst->Serialize(ob, slot_id);

			if (ob.tellp() == last_pos) {
				LogInventory("Serialization failed on item slot [{}] during BulkSendInventoryItems. Item skipped", slot_id);
			}

			last_pos = ob.tellp();
		}
	}

	auto outapp = new EQApplicationPacket(OP_CharInventory);

	outapp->size    = ob.size();
	outapp->pBuffer = ob.detach();

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::BulkSendMerchantInventory(int merchant_id, int npcid) {
	const EQ::ItemData* handy_item = nullptr;
	const EQ::ItemData *item = nullptr;
	auto merchant_list = zone->merchanttable[merchant_id];
	auto npc = entity_list.GetMobByNpcTypeID(npcid);
	if (merchant_list.empty()) {
		zone->LoadNewMerchantData(merchant_id);
		merchant_list = zone->merchanttable[merchant_id];
		if (merchant_list.empty()) {
			return;
		}
	}

	const int16 merchant_slots = (m_ClientVersionBit & EQ::versions::maskRoFAndLater) ? EQ::invtype::MERCHANT_SIZE : 80;

	auto temporary_merchant_list = zone->tmpmerchanttable[npcid];
	uint32 slot_id = 1;
	uint8 handy_chance = 0;
	for (const auto& ml : merchant_list) {
		if (slot_id > merchant_slots) {
			break;
		}

		auto bucket_name = ml.bucket_name;
		auto const& bucket_value = ml.bucket_value;
		if (!bucket_name.empty() && !bucket_value.empty()) {

			DataBucketKey k = GetScopedBucketKeys();
			k.key = bucket_name;

			auto b = DataBucket::GetData(&database, k);
			if (b.value.empty()) {
				continue;
			}

			if (!zone->CompareDataBucket(ml.bucket_comparison, bucket_value, b.value)) {
				continue;
			}
		}

		if (ml.probability != 100 && zone->random.Int(1, 100) > ml.probability) {
			continue;
		}

		if (GetLevel() < ml.level_required) {
			continue;
		}

		if (!(ml.classes_required & (1 << (GetClass() - 1)))) {
			continue;
		}

		if (!EQ::ValueWithin(Admin(), static_cast<int16>(ml.min_status), static_cast<int16>(ml.max_status))) {
			continue;
		}

		int32 faction_id = npc ? npc->GetPrimaryFaction() : 0;
		int32 faction_level = (
			(!faction_id || sneaking) ?
			0 :
			GetModCharacterFactionLevel(faction_id)
		);

		if (faction_level < ml.faction_required) {
			continue;
		}

		handy_chance = zone->random.Int(0, merchant_list.size() + temporary_merchant_list.size() - 1);

		item = database.GetItem(ml.item);
		if (item) {
			if (!handy_chance) {
				handy_item = item;
			} else {
				handy_chance--;
			}

			int16 charges = item->IsClassCommon() ? item->MaxCharges : 1;

			auto inst = database.CreateItem(item, charges);
			if (inst) {
				auto item_price = static_cast<uint32>(item->Price * item->SellRate);
				auto item_charges = charges ? charges : 1;

				// Don't use SellCostMod if using UseClassicPriceMod
				if (!RuleB(Merchant, UseClassicPriceMod)) {
					item_price *= RuleR(Merchant, SellCostMod);
				}

				if (RuleB(Merchant, UsePriceMod)) {
					item_price *= Client::CalcPriceMod(npc);
				}

				inst->SetCharges(item_charges);
				inst->SetMerchantCount(-1);
				inst->SetMerchantSlot(ml.slot);
				inst->SetPrice(item_price);

				SendItemPacket(ml.slot - 1, inst, ItemPacketMerchant);
				safe_delete(inst);
			}
		}

		// Account for merchant lists with gaps.
		if (ml.slot >= slot_id) {
			if (ml.slot > slot_id) {
				LogDebug("(WARNING) Merchantlist Contains gap at slot [{}]. Merchant: [{}], NPC: [{}]", slot_id, merchant_id, npcid);
			}

			slot_id = ml.slot + 1;
		}
	}

	auto temporary_merchant_list_two = zone->tmpmerchanttable[npcid];
	temporary_merchant_list.clear();
	for (auto ml : temporary_merchant_list_two) {
		if (slot_id > merchant_slots) {
			break;
		}

		item = database.GetItem(ml.item);
		ml.slot = slot_id;
		if (item) {
			if (!handy_chance) {
				handy_item = item;
			} else {
				handy_chance--;
			}

			auto charges = item->MaxCharges;
			auto inst = database.CreateItem(item, charges);
			if (inst) {
				auto item_price = static_cast<uint32>(item->Price * item->SellRate);
				auto item_charges = charges ? charges : 1;

				// Don't use SellCostMod if using UseClassicPriceMod
				if (!RuleB(Merchant, UseClassicPriceMod)) {
					item_price *= RuleR(Merchant, SellCostMod);
				}

				if (RuleB(Merchant, UsePriceMod)) {
					item_price *= Client::CalcPriceMod(npc);
				}

				inst->SetCharges(item_charges);
				inst->SetMerchantCount(ml.charges);
				inst->SetMerchantSlot(ml.slot);
				inst->SetPrice(item_price);

				SendItemPacket(ml.slot - 1, inst, ItemPacketMerchant);
				safe_delete(inst);
			}
		}
		temporary_merchant_list.push_back(ml);
		slot_id++;
	}

	//this resets the slot
	zone->tmpmerchanttable[npcid] = temporary_merchant_list;
	if (npc && handy_item) {
		int greet_id = zone->random.Int(MERCHANT_GREETING, MERCHANT_HANDY_ITEM4);
		auto handy_id = std::to_string(greet_id);
		if (greet_id != MERCHANT_GREETING) {
			MessageString(Chat::NPCQuestSay, GENERIC_STRINGID_SAY, npc->GetCleanName(), handy_id.c_str(), GetName(), handy_item->Name);
		} else {
			MessageString(Chat::NPCQuestSay, GENERIC_STRINGID_SAY, npc->GetCleanName(), handy_id.c_str(), GetName());
		}
	}
}

uint8 Client::WithCustomer(uint16 NewCustomer){

	if(NewCustomer == 0) {
		SetCustomerID(0);
		return 0;
	}

	if(GetCustomerID() == 0) {
		SetCustomerID(NewCustomer);
		return 1;
	}

	// Check that the player browsing our wares hasn't gone away.

	Client* c = entity_list.GetClientByID(GetCustomerID());

	if(!c) {
		LogTrading("Previous customer has gone away");
		SetCustomerID(NewCustomer);
		return 1;
	}

	return 0;
}

void Client::OPRezzAnswer(uint32 Action, uint32 SpellID, uint16 ZoneID, uint16 InstanceID, float x, float y, float z)
{
	if(PendingRezzXP < 0) {
		// pendingrezexp is set to -1 if we are not expecting an OP_RezzAnswer
		LogSpells("Unexpected OP_RezzAnswer. Ignoring it");
		Message(Chat::Red, "You have already been resurrected.\n");
		return;
	}

	if (Action == 1)
	{
		// Mark the corpse as rezzed in the database, just in case the corpse has buried, or the zone the
		// corpse is in has shutdown since the rez spell was cast.
		database.MarkCorpseAsResurrected(PendingRezzDBID);
		LogSpells("Player [{}] got a [{}] Rezz spellid [{}] in zone[{}] instance id [{}]",
				name, (uint16)spells[SpellID].base_value[0],
				SpellID, ZoneID, InstanceID);

		const bool use_old_resurrection = (
			RuleB(Character, UseOldRaceRezEffects) &&
			(
				GetRace() == Race::Barbarian ||
				GetRace() == Race::Dwarf ||
				GetRace() == Race::Troll ||
				GetRace() == Race::Ogre
			)
		);

		const uint16 resurrection_sickness_spell_id = (
			use_old_resurrection ?
			RuleI(Character, OldResurrectionSicknessSpellID) :
			RuleI(Character, ResurrectionSicknessSpellID)
		);

		int SpellEffectDescNum = GetSpellEffectDescriptionNumber(SpellID);
		// Rez spells with Rez effects have this DescNum (first is Titanium, second is 6.2 Client)
		if(RuleB(Character, UseResurrectionSickness) && SpellEffectDescNum == 82 || SpellEffectDescNum == 39067) {
			SetHP(GetMaxHP() / 5);
			SetMana(0);

			if (RuleB(Spells, BuffsFadeOnDeath)) {
				BuffFadeNonPersistDeath();
			}

			SpellOnTarget(resurrection_sickness_spell_id, this);
		} else if (SpellID == SPELL_DIVINE_REZ) {
			if (RuleB(Spells, BuffsFadeOnDeath)) {
				BuffFadeNonPersistDeath();
			}

			RestoreHealth();
			RestoreMana();
			RestoreEndurance();
		} else {
			if (RuleB(Character, UseResurrectionSickness)) {
				bool has_resurrection_sickness = false;

				for (int slot = 0; slot < GetMaxTotalSlots(); slot++) {
					if (IsValidSpell(buffs[slot].spellid) && IsResurrectionSicknessSpell(buffs[slot].spellid)){
						has_resurrection_sickness = true;
						break;
					}
				}

				// Need to wipe buffs after checking if client had rez effects.
				if (RuleB(Spells, BuffsFadeOnDeath)) {
					BuffFadeNonPersistDeath();
				}

				if (has_resurrection_sickness) {
					SpellOnTarget(resurrection_sickness_spell_id, this);
				}
			}

			SetHP(GetMaxHP() / 20);
			SetMana(GetMaxMana() / 20);
			SetEndurance(GetMaxEndurance() / 20);
		}

		if(spells[SpellID].base_value[0] < 100 && spells[SpellID].base_value[0] > 0 && PendingRezzXP > 0) {
			SetEXP(ExpSource::Resurrection, ((int)(GetEXP()+((float)((PendingRezzXP / 100) * spells[SpellID].base_value[0])))), GetAAXP(), true);
		} else if (spells[SpellID].base_value[0] == 100 && PendingRezzXP > 0) {
			SetEXP(ExpSource::Resurrection, (GetEXP() + PendingRezzXP), GetAAXP(), true);
		}

		//Was sending the packet back to initiate client zone...
		//but that could be abusable, so lets go through proper channels
		MovePC(ZoneID, InstanceID, x, y, z, GetHeading(), 0, ZoneSolicited);
		entity_list.RefreshClientXTargets(this);
	}
	PendingRezzXP = -1;
	PendingRezzSpellID = 0;
}

void Client::OPTGB(const EQApplicationPacket *app)
{
	if(!app) return;
	if(!app->pBuffer) return;

	if(!RuleB(Character, EnableTGB))
	{
		return;
	}

	uint32 tgb_flag = *(uint32 *)app->pBuffer;
	if(tgb_flag == 2)
		MessageString(Chat::White, TGB() ? TGB_ON : TGB_OFF);
	else
		tgb = tgb_flag;
}

void Client::OPMemorizeSpell(const EQApplicationPacket* app)
{
	if (app->size != sizeof(MemorizeSpell_Struct)) {
		LogError(
			"Wrong size on OP_MemorizeSpell. Got: [{}] Expected: [{}]",
			app->size,
			sizeof(MemorizeSpell_Struct)
		);
		DumpPacket(app);
		return;
	}

	const auto* m = (MemorizeSpell_Struct*) app->pBuffer;

	if (!IsValidSpell(m->spell_id)) {
		Message(
			Chat::Red,
			fmt::format(
				"Spell ID {} does not exist or is invalid.",
				m->spell_id
			).c_str()
		);
		return;
	}

	if (
		m->scribing != memSpellForget &&
		(
			!IsPlayerClass(GetClass()) ||
			GetLevel() < spells[m->spell_id].classes[GetClass() - 1]
		)
	) {
		MessageString(
			Chat::Red,
			SPELL_LEVEL_TO_LOW,
			std::to_string(spells[m->spell_id].classes[GetClass() - 1]).c_str(),
			spells[m->spell_id].name
		);
		return;
	}

	switch (m->scribing) {
		case memSpellScribing: {
			const auto* inst = m_inv[EQ::invslot::slotCursor];

			if (inst && inst->IsClassCommon()) {
				const auto* item = inst->GetItem();

				if (
					item &&
					RuleB(Character, RestrictSpellScribing) &&
					!item->IsEquipable(GetRace(), GetClass())
				) {
					MessageString(Chat::Red, CANNOT_USE_ITEM);
					break;
				}

				if (item && item->Scroll.Effect == static_cast<int32>(m->spell_id)) {
					ScribeSpell(m->spell_id, m->slot);
					DeleteItemInInventory(EQ::invslot::slotCursor, 1, true);
				} else {
					Message(Chat::Red, "Scribing spell: Item Instance exists but item does not or spell ids do not match.");
				}
			} else {
				Message(Chat::Red, "Scribing a spell without an Item Instance on your cursor?");
			}
			break;
		}
		case memSpellMemorize: {
			if (HasSpellScribed(m->spell_id)) {
				MemSpell(m->spell_id, m->slot);
			} else {
				std::string message = fmt::format("OP_MemorizeSpell [{}] but we don't have this spell scribed", m->spell_id);
				RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});
			}
			break;
		}
		case memSpellForget: {
			UnmemSpell(m->slot);
			break;
		}
	}

	Save();
}

void Client::CancelSneakHide()
{
	if (hidden || improved_hidden) {
		auto app = new EQApplicationPacket(OP_CancelSneakHide, 0);
		FastQueuePacket(&app);
		// SoF and Tit send back a OP_SpawnAppearance turning off AppearanceType::Invisibility
		// so we need to handle our sneaking flag only
		// The later clients send back a OP_Hide (this has a size but data is 0)
		// as well as OP_SpawnAppearance with AppearanceType::Invisibility and one with AppearanceType::Sneak
		// So we don't have to handle any of those flags
		if (ClientVersionBit() & EQ::versions::maskSoFAndEarlier)
			sneaking = false;
	}
}

void Client::BreakInvis()
{
	if (invisible)
	{
		auto outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
		SpawnAppearance_Struct* sa_out = (SpawnAppearance_Struct*)outapp->pBuffer;
		sa_out->spawn_id = GetID();
		sa_out->type = 0x03;
		sa_out->parameter = 0;
		entity_list.QueueClients(this, outapp, true);
		safe_delete(outapp);
		ZeroInvisibleVars(InvisType::T_INVISIBLE);
		ZeroInvisibleVars(InvisType::T_INVISIBLE_VERSE_UNDEAD);
		ZeroInvisibleVars(InvisType::T_INVISIBLE_VERSE_ANIMAL);
		hidden = false;
		improved_hidden = false;
	}
}

static uint64 CoinTypeCoppers(uint32 type) {
	switch(type) {
	case COINTYPE_PP:
		return(1000);
	case COINTYPE_GP:
		return(100);
	case COINTYPE_SP:
		return(10);
	case COINTYPE_CP:
	default:
		break;
	}
	return(1);
}

void Client::OPMoveCoin(const EQApplicationPacket* app)
{
	MoveCoin_Struct* mc = (MoveCoin_Struct*)app->pBuffer;
	uint64 value = 0, amount_to_take = 0, amount_to_add = 0;
	int32 *from_bucket = 0, *to_bucket = 0;
	Mob* trader = trade->With();

	// if amount < 0, client is sending a malicious packet
	if (mc->amount < 0)
	{
		return;
	}

	// could just do a range, but this is clearer and explicit
	if
	(
		(
			mc->cointype1 != COINTYPE_PP &&
			mc->cointype1 != COINTYPE_GP &&
			mc->cointype1 != COINTYPE_SP &&
			mc->cointype1 != COINTYPE_CP
		) ||
		(
			mc->cointype2 != COINTYPE_PP &&
			mc->cointype2 != COINTYPE_GP &&
			mc->cointype2 != COINTYPE_SP &&
			mc->cointype2 != COINTYPE_CP
		)
	)
	{
		return;
	}

	switch(mc->from_slot)
	{
		case -1:	// destroy
		{
			// I don't think you can move coin from the void,
			// but need to check this
			break;
		}
		case 0:	// cursor
		{
			switch(mc->cointype1)
			{
				case COINTYPE_PP:
					from_bucket = (int32 *) &m_pp.platinum_cursor; break;
				case COINTYPE_GP:
					from_bucket = (int32 *) &m_pp.gold_cursor; break;
				case COINTYPE_SP:
					from_bucket = (int32 *) &m_pp.silver_cursor; break;
				case COINTYPE_CP:
					from_bucket = (int32 *) &m_pp.copper_cursor; break;
			}
			break;
		}
		case 1:	// inventory
		{
			switch(mc->cointype1)
			{
				case COINTYPE_PP:
					from_bucket = (int32 *) &m_pp.platinum; break;
				case COINTYPE_GP:
					from_bucket = (int32 *) &m_pp.gold; break;
				case COINTYPE_SP:
					from_bucket = (int32 *) &m_pp.silver; break;
				case COINTYPE_CP:
					from_bucket = (int32 *) &m_pp.copper; break;
			}
			break;
		}
		case 2:	// bank
		{
			uint32 distance = 0;
			NPC *banker = entity_list.GetClosestBanker(this, distance);
			if(!banker || distance > USE_NPC_RANGE2)
			{
				auto message = fmt::format(
					"Player tried to make use of a banker (coin move) but "
					"banker [{}] is non-existent or too far away [{}] units",
					banker ? banker->GetName() : "UNKNOWN NPC", distance
				);
				RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});

				return;
			}

			switch(mc->cointype1)
			{
				case COINTYPE_PP:
					from_bucket = (int32 *) &m_pp.platinum_bank; break;
				case COINTYPE_GP:
					from_bucket = (int32 *) &m_pp.gold_bank; break;
				case COINTYPE_SP:
					from_bucket = (int32 *) &m_pp.silver_bank; break;
				case COINTYPE_CP:
					from_bucket = (int32 *) &m_pp.copper_bank; break;
			}
			break;
		}
		case 3:	// trade
		{
			// can't move coin from trade
			break;
		}
		case 4:	// shared bank
		{
			uint32 distance = 0;
			NPC *banker = entity_list.GetClosestBanker(this, distance);
			if(!banker || distance > USE_NPC_RANGE2)
			{
				auto message = fmt::format(
					"Player tried to make use of a banker (shared coin move) but banker [{}] is "
					"non-existent or too far away [{}] units",
					banker ? banker->GetName() : "UNKNOWN NPC", distance
				);
				RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});

				return;
			}
			if(mc->cointype1 == COINTYPE_PP)	// there's only platinum here
				from_bucket = (int32 *) &m_pp.platinum_shared;
			break;
		}
	}

	switch(mc->to_slot)
	{
		case -1:	// destroy
		{
			// no action required
			break;
		}
		case 0:	// cursor
		{
			switch(mc->cointype2)
			{
				case COINTYPE_PP:
					to_bucket = (int32 *) &m_pp.platinum_cursor; break;
				case COINTYPE_GP:
					to_bucket = (int32 *) &m_pp.gold_cursor; break;
				case COINTYPE_SP:
					to_bucket = (int32 *) &m_pp.silver_cursor; break;
				case COINTYPE_CP:
					to_bucket = (int32 *) &m_pp.copper_cursor; break;
			}
			break;
		}
		case 1:	// inventory
		{
			switch(mc->cointype2)
			{
				case COINTYPE_PP:
					to_bucket = (int32 *) &m_pp.platinum; break;
				case COINTYPE_GP:
					to_bucket = (int32 *) &m_pp.gold; break;
				case COINTYPE_SP:
					to_bucket = (int32 *) &m_pp.silver; break;
				case COINTYPE_CP:
					to_bucket = (int32 *) &m_pp.copper; break;
			}
			break;
		}
		case 2:	// bank
		{
			uint32 distance = 0;
			NPC *banker = entity_list.GetClosestBanker(this, distance);
			if(!banker || distance > USE_NPC_RANGE2)
			{
				auto message = fmt::format(
					"Player tried to make use of a banker(coin move) but "
					"banker [{}] is non-existent or too far away [{}] units",
					banker ? banker->GetName() : "UNKNOWN NPC", distance
				);
				RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});

				return;
			}
			switch(mc->cointype2)
			{
				case COINTYPE_PP:
					to_bucket = (int32 *) &m_pp.platinum_bank; break;
				case COINTYPE_GP:
					to_bucket = (int32 *) &m_pp.gold_bank; break;
				case COINTYPE_SP:
					to_bucket = (int32 *) &m_pp.silver_bank; break;
				case COINTYPE_CP:
					to_bucket = (int32 *) &m_pp.copper_bank; break;
			}
			break;
		}
		case 3:	// trade
		{
			if(trader)
			{
				switch(mc->cointype2)
				{
					case COINTYPE_PP:
						to_bucket = (int32 *) &trade->pp; break;
					case COINTYPE_GP:
						to_bucket = (int32 *) &trade->gp; break;
					case COINTYPE_SP:
						to_bucket = (int32 *) &trade->sp; break;
					case COINTYPE_CP:
						to_bucket = (int32 *) &trade->cp; break;
				}
			}
			else {
				switch (mc->cointype2) {
					case COINTYPE_PP:
						m_parcel_platinum += mc->amount;
						break;
					case COINTYPE_GP:
						m_parcel_gold += mc->amount;
						break;
					case COINTYPE_SP:
						m_parcel_silver += mc->amount;
						break;
					case COINTYPE_CP:
						m_parcel_copper += mc->amount;
						break;
				}
			}
			break;
		}
		case 4:	// shared bank
		{
			uint32 distance = 0;
			NPC *banker = entity_list.GetClosestBanker(this, distance);
			if(!banker || distance > USE_NPC_RANGE2)
			{
				auto message = fmt::format(
					"Player tried to make use of a banker (shared coin move) but banker [{}] is "
					"non-existent or too far away [{}] units",
					banker ? banker->GetName() : "UNKNOWN NPC", distance
				);
				RecordPlayerEventLog(PlayerEvent::POSSIBLE_HACK, PlayerEvent::PossibleHackEvent{.message = message});

				return;
			}
			if(mc->cointype2 == COINTYPE_PP)	// there's only platinum here
				to_bucket = (int32 *) &m_pp.platinum_shared;
			break;
		}
	}

	if(!from_bucket)
	{
		return;
	}

	// don't allow them to go into negatives (from our point of view)
	amount_to_take = *from_bucket < mc->amount ? *from_bucket : mc->amount;

	// if you move 11 gold into a bank platinum location, the packet
	// will say 11, but the client will have 1 left on their cursor, so we have
	// to figure out the conversion ourselves

	amount_to_add = amount_to_take * ((float)CoinTypeCoppers(mc->cointype1) / (float)CoinTypeCoppers(mc->cointype2));

	// the amount we're adding could be different than what was requested, so
	// we have to adjust the amount we take as well
	amount_to_take = amount_to_add * ((float)CoinTypeCoppers(mc->cointype2) / (float)CoinTypeCoppers(mc->cointype1));

	// now we should have a from_bucket, a to_bucket, an amount_to_take
	// and an amount_to_add

	// now we actually take it from the from bucket. if there's an error
	// with the destination slot, they lose their money
	*from_bucket -= amount_to_take;
	// why are intentionally inducing a crash here rather than letting the code attempt to stumble on?
	// assert(*from_bucket >= 0);

	if(to_bucket)
	{
		if(*to_bucket + amount_to_add > *to_bucket)	// overflow check
			*to_bucket += amount_to_add;

		//shared bank plat
		if (RuleB(Character, SharedBankPlat))
		{
			if (to_bucket == &m_pp.platinum_shared || from_bucket == &m_pp.platinum_shared)
			{
				if (from_bucket == &m_pp.platinum_shared)
					amount_to_add = 0 - amount_to_take;

				database.AddSharedPlatinum(AccountID(),amount_to_add);
			}
		}
		else{
			if (to_bucket == &m_pp.platinum_shared || from_bucket == &m_pp.platinum_shared){
				SendPopupToClient(
					"Shared Bank Warning",
					"<c \"#F62217\">::: WARNING! :::<br>"
					"SHARED BANK IS DISABLED AND YOUR PLATINUM WILL BE DESTROYED IF YOU PUT IT HERE!</c>"
				);
				Message(Chat::Red, "::: WARNING! ::: SHARED BANK IS DISABLED AND YOUR PLATINUM WILL BE DESTROYED IF YOU PUT IT HERE!");
			}
		}
	}

	// if this is a trade move, inform the person being traded with
	if(mc->to_slot == 3 && trader && trader->IsClient())
	{

		// If one party accepted the trade then some coin was added, their state needs to be reset
		trade->state = Trading;
		Mob* with = trade->With();
		if (with)
			with->trade->state = Trading;

		Client* recipient = trader->CastToClient();
		recipient->Message(Chat::Yellow, "%s adds some coins to the trade.", GetName());
		recipient->Message(Chat::Yellow, "The total trade is: %i PP, %i GP, %i SP, %i CP",
			trade->pp, trade->gp,
			trade->sp, trade->cp
		);

		auto outapp = new EQApplicationPacket(OP_TradeCoins, sizeof(TradeCoin_Struct));
		TradeCoin_Struct* tcs = (TradeCoin_Struct*)outapp->pBuffer;
		tcs->trader = trader->GetID();
		tcs->slot = mc->cointype2;
		tcs->unknown5 = 0x4fD2;
		tcs->unknown7 = 0;
		tcs->amount = amount_to_add;
		recipient->QueuePacket(outapp);
		safe_delete(outapp);
	}

	SaveCurrency();
}

void Client::OPGMTraining(const EQApplicationPacket *app)
{

	EQApplicationPacket* outapp = app->Copy();
	GMTrainee_Struct* gmtrain = (GMTrainee_Struct*) outapp->pBuffer;

	Mob* pTrainer = entity_list.GetMob(gmtrain->npcid);

	if (!pTrainer || !pTrainer->IsNPC() || pTrainer->GetClass() < Class::WarriorGM || pTrainer->GetClass() > Class::BerserkerGM) {
		return;
	}

	//you can only use your own trainer, client enforces this, but why trust it
	if (!RuleB(Character, AllowCrossClassTrainers)) {
		int trains_class = pTrainer->GetClass() - (Class::WarriorGM - Class::Warrior);
		if (GetClass() != trains_class) {
			safe_delete(outapp);
			return;
		}
	}

	//you have to be somewhat close to a trainer to be properly using them
	if (DistanceSquared(m_Position,pTrainer->GetPosition()) > USE_NPC_RANGE2) {
		safe_delete(outapp);
		return;
	}

	// if this for-loop acts up again (crashes linux), try enabling the before and after #pragmas
//#pragma GCC push_options
//#pragma GCC optimize ("O0")
	for (int sk = EQ::skills::Skill1HBlunt; sk <= EQ::skills::HIGHEST_SKILL; ++sk) {
		if (sk == EQ::skills::SkillTinkering && GetRace() != Race::Gnome) {
			gmtrain->skills[sk] = 0; //Non gnomes can't tinker!
		} else {
			gmtrain->skills[sk] = GetMaxSkillAfterSpecializationRules((EQ::skills::SkillType)sk, MaxSkill((EQ::skills::SkillType)sk, GetClass(), RuleI(Character, MaxLevel)));
			//this is the highest level that the trainer can train you to, this is enforced clientside so we can't just
			//Set it to 1 with CanHaveSkill or you wont be able to train past 1.
		}
	}

	if (ClientVersion() < EQ::versions::ClientVersion::RoF2 && GetClass() == Class::Berserker) {
		gmtrain->skills[EQ::skills::Skill1HPiercing] = gmtrain->skills[EQ::skills::Skill2HPiercing];
		gmtrain->skills[EQ::skills::Skill2HPiercing] = 0;
	}
//#pragma GCC pop_options

	uchar ending[]={0x34,0x87,0x8a,0x3F,0x01
		,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9
		,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9
		,0x76,0x75,0x3f};
	memcpy(&outapp->pBuffer[outapp->size-40],ending,sizeof(ending));
	FastQueuePacket(&outapp);

	// welcome message
	if (pTrainer && pTrainer->IsNPC())
	{
		pTrainer->SayString(zone->random.Int(1204, 1207), GetCleanName());
	}
}

void Client::OPGMEndTraining(const EQApplicationPacket *app)
{
	auto outapp = new EQApplicationPacket(OP_GMEndTrainingResponse, 0);
	GMTrainEnd_Struct *p = (GMTrainEnd_Struct *)app->pBuffer;

	FastQueuePacket(&outapp);

	Mob* pTrainer = entity_list.GetMob(p->npcid);
	if(!pTrainer || !pTrainer->IsNPC() || pTrainer->GetClass() < Class::WarriorGM || pTrainer->GetClass() > Class::BerserkerGM)
		return;

	//you can only use your own trainer, client enforces this, but why trust it
	if (!RuleB(Character, AllowCrossClassTrainers)) {
		int trains_class = pTrainer->GetClass() - (Class::WarriorGM - Class::Warrior);
		if (GetClass() != trains_class)
			return;
	}

	//you have to be somewhat close to a trainer to be properly using them
	if(DistanceSquared(m_Position, pTrainer->GetPosition()) > USE_NPC_RANGE2)
		return;

	// goodbye message
	if (pTrainer->IsNPC())
	{
		pTrainer->SayString(zone->random.Int(1208, 1211), GetCleanName());
	}
}

void Client::OPGMTrainSkill(const EQApplicationPacket *app)
{
	if(!m_pp.points)
		return;

	int Cost = 0;

	GMSkillChange_Struct* gmskill = (GMSkillChange_Struct*) app->pBuffer;

	Mob* pTrainer = entity_list.GetMob(gmskill->npcid);
	if(!pTrainer || !pTrainer->IsNPC() || pTrainer->GetClass() < Class::WarriorGM || pTrainer->GetClass() > Class::BerserkerGM)
		return;

	//you can only use your own trainer, client enforces this, but why trust it
	if (!RuleB(Character, AllowCrossClassTrainers)) {
		int trains_class = pTrainer->GetClass() - (Class::WarriorGM - Class::Warrior);
		if (GetClass() != trains_class)
			return;
	}

	//you have to be somewhat close to a trainer to be properly using them
	if(DistanceSquared(m_Position, pTrainer->GetPosition()) > USE_NPC_RANGE2)
		return;

	if (gmskill->skillbank == 0x01)
	{
		// languages go here
		if (gmskill->skill_id > 25)
		{
			LogSkills("Wrong Training Skill (languages)");
			DumpPacket(app);
			return;
		}
		int AdjustedSkillLevel = GetLanguageSkill(gmskill->skill_id) - 10;
		if(AdjustedSkillLevel > 0)
			Cost = AdjustedSkillLevel * AdjustedSkillLevel * AdjustedSkillLevel / 100;

		IncreaseLanguageSkill(gmskill->skill_id);
	}
	else if (gmskill->skillbank == 0x00)
	{
		// normal skills go here
		if (gmskill->skill_id > EQ::skills::HIGHEST_SKILL)
		{
			LogSkills("Wrong Training Skill (abilities)");
			DumpPacket(app);
			return;
		}

		EQ::skills::SkillType skill = (EQ::skills::SkillType)gmskill->skill_id;

		if(!CanHaveSkill(skill)) {
			LogSkills("Tried to train skill [{}], which is not allowed", skill);
			return;
		}

		if(MaxSkill(skill) == 0) {
			LogSkills("Tried to train skill [{}], but training is not allowed at this level", skill);
			return;
		}

		uint16 skilllevel = GetRawSkill(skill);

		if (skilllevel == 0) {
			//this is a new skill..
			uint16 t_level = GetSkillTrainLevel(skill, GetClass());

			if (t_level == 0) {
				LogSkills("Tried to train a new skill [{}] which is invalid for this race/class.", skill);
				return;
			}

			SetSkill(skill, t_level);
		} else {
			switch(skill) {
			case EQ::skills::SkillBrewing:
			case EQ::skills::SkillMakePoison:
			case EQ::skills::SkillTinkering:
			case EQ::skills::SkillAlchemy:
			case EQ::skills::SkillBaking:
			case EQ::skills::SkillTailoring:
			case EQ::skills::SkillBlacksmithing:
			case EQ::skills::SkillFletching:
			case EQ::skills::SkillJewelryMaking:
			case EQ::skills::SkillPottery:
				if(skilllevel >= RuleI(Skills, MaxTrainTradeskills)) {
					MessageString(Chat::Red, MORE_SKILLED_THAN_I, pTrainer->GetCleanName());
					SetSkill(skill, skilllevel);
					return;
				}
				break;
			case EQ::skills::SkillResearch:
				if(skilllevel >= RuleI(Skills, MaxTrainResearch)) {
					MessageString(Chat::Red, MORE_SKILLED_THAN_I, pTrainer->GetCleanName());
					SetSkill(skill, skilllevel);
					return;
				}
				break;
			case EQ::skills::SkillSpecializeAbjure:
			case EQ::skills::SkillSpecializeAlteration:
			case EQ::skills::SkillSpecializeConjuration:
			case EQ::skills::SkillSpecializeDivination:
			case EQ::skills::SkillSpecializeEvocation:
				if(skilllevel >= RuleI(Skills, MaxTrainSpecializations)) {
					MessageString(Chat::Red, MORE_SKILLED_THAN_I, pTrainer->GetCleanName());
					SetSkill(skill, skilllevel);
					return;
				}
			default:
				break;
			}

			int MaxSkillValue = MaxSkill(skill);
			if (skilllevel >= MaxSkillValue)
			{
				// Don't allow training over max skill level
				MessageString(Chat::Red, MORE_SKILLED_THAN_I, pTrainer->GetCleanName());
				SetSkill(skill, skilllevel);
				return;
			}

			if (gmskill->skill_id >= EQ::skills::SkillSpecializeAbjure && gmskill->skill_id <= EQ::skills::SkillSpecializeEvocation)
			{
				int MaxSpecSkill = GetMaxSkillAfterSpecializationRules(skill, MaxSkillValue);
				if (skilllevel >= MaxSpecSkill)
				{
					// Restrict specialization training to follow the rules
					MessageString(Chat::Red, MORE_SKILLED_THAN_I, pTrainer->GetCleanName());
					SetSkill(skill, skilllevel);
					return;
				}
			}

			// Client train a valid skill
			//
			int AdjustedSkillLevel = skilllevel - 10;

			if(AdjustedSkillLevel > 0)
				Cost = AdjustedSkillLevel * AdjustedSkillLevel * AdjustedSkillLevel / 100;

			SetSkill(skill, skilllevel + 1);


		}
	}

	if (ClientVersion() >= EQ::versions::ClientVersion::SoF) {
		// The following packet decreases the skill points left in the Training Window and
		// produces the 'You have increased your skill / learned the basics of' message.
		//
		auto outapp = new EQApplicationPacket(OP_GMTrainSkillConfirm, sizeof(GMTrainSkillConfirm_Struct));

		GMTrainSkillConfirm_Struct *gmtsc = (GMTrainSkillConfirm_Struct *)outapp->pBuffer;
		gmtsc->SkillID = gmskill->skill_id;

		if(gmskill->skillbank == 1) {
			gmtsc->NewSkill = (GetLanguageSkill(gmtsc->SkillID) == 1);
			gmtsc->SkillID += 100;
		}
		else
			gmtsc->NewSkill = (GetRawSkill((EQ::skills::SkillType)gmtsc->SkillID) == 1);

		gmtsc->Cost = Cost;

		strcpy(gmtsc->TrainerName, pTrainer->GetCleanName());
		QueuePacket(outapp);
		safe_delete(outapp);
	}

	if(Cost)
		TakeMoneyFromPP(Cost);

	m_pp.points--;
}

// this is used for /summon and /corpse
void Client::OPGMSummon(const EQApplicationPacket *app)
{
	GMSummon_Struct* gms = (GMSummon_Struct*) app->pBuffer;
	Mob* st = entity_list.GetMob(gms->charname);

	if(st && st->IsCorpse())
	{
		st->CastToCorpse()->Summon(this, false, true);
	}
	else
	{
		if(admin < AccountStatus::QuestTroupe)
		{
			return;
		}
		if(st)
		{
			Message(0, "Local: Summoning %s to %f, %f, %f", gms->charname, gms->x, gms->y, gms->z);
			if (st->IsClient() && (st->CastToClient()->GetAnon() != 1 || Admin() >= st->CastToClient()->Admin()))
				st->CastToClient()->MovePC(zone->GetZoneID(), zone->GetInstanceID(), (float)gms->x, (float)gms->y, (float)gms->z, GetHeading(), true);
			else
				st->GMMove(GetX(), GetY(), GetZ(),GetHeading());
		}
		else
		{
			uint8 tmp = gms->charname[strlen(gms->charname)-1];
			if (!worldserver.Connected())
			{
				Message(0, "Error: World server disconnected");
			}
			else if (tmp < '0' || tmp > '9') // dont send to world if it's not a player's name
			{
				auto pack = new ServerPacket(ServerOP_ZonePlayer, sizeof(ServerZonePlayer_Struct));
				ServerZonePlayer_Struct* szp = (ServerZonePlayer_Struct*) pack->pBuffer;
				strcpy(szp->adminname, GetName());
				szp->adminrank = Admin();
				strcpy(szp->name, gms->charname);
				strcpy(szp->zone, zone->GetShortName());
				szp->x_pos = (float)gms->x;
				szp->y_pos = (float)gms->y;
				szp->z_pos = (float)gms->z;
				szp->ignorerestrictions = 2;
				worldserver.SendPacket(pack);
				safe_delete(pack);
			}
			else {
				//all options have been exhausted
				//summon our target...
				if(GetTarget() && GetTarget()->IsCorpse()){
					GetTarget()->CastToCorpse()->Summon(this, false, true);
				}
			}
		}
	}
}

void Client::DoHPRegen() {
	SetHP(GetHP() + CalcHPRegen());
	SendHPUpdate();
}

void Client::DoManaRegen() {
	if (GetMana() >= max_mana && spellbonuses.ManaRegen >= 0)
		return;

	if (GetMana() < max_mana && (IsSitting() || CanMedOnHorse()) && HasSkill(EQ::skills::SkillMeditate))
		CheckIncreaseSkill(EQ::skills::SkillMeditate, nullptr, -5);

	SetMana(GetMana() + CalcManaRegen());
	CheckManaEndUpdate();
}

void Client::DoStaminaHungerUpdate()
{
	auto outapp = new EQApplicationPacket(OP_Stamina, sizeof(Stamina_Struct));
	auto sta    = (Stamina_Struct*) outapp->pBuffer;

	LogFood("hunger_level: [{}] thirst_level: [{}] before loss", m_pp.hunger_level, m_pp.thirst_level);

	if (zone->GetZoneID() != Zones::BAZAAR) {
		if (!GetGM()) {
			int loss = RuleI(Character, FoodLossPerUpdate);
			if (GetHorseId() != 0) {
				loss *= 3;
			}

			m_pp.hunger_level = EQ::Clamp(m_pp.hunger_level - loss, 0, 6000);
			m_pp.thirst_level = EQ::Clamp(m_pp.thirst_level - loss, 0, 6000);

			if (spellbonuses.hunger) {
				m_pp.hunger_level = EQ::ClampLower(m_pp.hunger_level, 3500);
				m_pp.thirst_level = EQ::ClampLower(m_pp.thirst_level, 3500);
			}

			sta->food  = m_pp.hunger_level;
			sta->water = m_pp.thirst_level;
		} else {
			sta->food  = 6000;
			sta->water = 6000;
		}
	} else { // No auto food/drink consumption in the Bazaar
		sta->food  = 6000;
		sta->water = 6000;
	}

	LogFood(
		"Current hunger_level: [{}] = ([{}] minutes left) thirst_level: [{}] = ([{}] minutes left) - after loss",
		m_pp.hunger_level,
		m_pp.hunger_level,
		m_pp.thirst_level,
		m_pp.thirst_level
	);

	FastQueuePacket(&outapp);
}

void Client::DoEnduranceRegen()
{
	// endurance has some negative mods that could result in a negative regen when starved
	int64 regen = CalcEnduranceRegen();

	if (regen < 0 || (regen > 0 && GetEndurance() < GetMaxEndurance()))
		SetEndurance(GetEndurance() + regen);
}

void Client::DoEnduranceUpkeep() {

	if (!HasEndurUpkeep())
		return;

	int upkeep_sum = 0;
	int cost_redux = spellbonuses.EnduranceReduction + itembonuses.EnduranceReduction + aabonuses.EnduranceReduction;

	bool has_effect = false;
	uint32 buffs_i;
	uint32 buff_count = GetMaxTotalSlots();
	for (buffs_i = 0; buffs_i < buff_count; buffs_i++) {
		if (IsValidSpell(buffs[buffs_i].spellid)) {
			int upkeep = spells[buffs[buffs_i].spellid].endurance_upkeep;
			if(upkeep > 0) {
				has_effect = true;
				if(cost_redux > 0) {
					if(upkeep <= cost_redux)
						continue;	//reduced to 0
					upkeep -= cost_redux;
				}
				if((upkeep+upkeep_sum) > GetEndurance()) {
					//they do not have enough to keep this one going.
					BuffFadeBySlot(buffs_i);
				} else {
					upkeep_sum += upkeep;
				}
			}
		}
	}

	if(upkeep_sum != 0){
		SetEndurance(GetEndurance() - upkeep_sum);
		TryTriggerOnCastRequirement();
	}

	if (!has_effect)
		SetEndurUpkeep(false);
}

void Client::CalcRestState()
{
	// This method calculates rest state HP and mana regeneration.
	// The client must have been out of combat for RuleI(Character, RestRegenTimeToActivate) seconds,
	// must be sitting down, and must not have any detrimental spells affecting them.
	if(!RuleB(Character, RestRegenEnabled))
		return;

	ooc_regen = false;

	if(AggroCount || !(IsSitting() || CanMedOnHorse()))
		return;

	if(!rest_timer.Check(false))
		return;

	// so we don't have aggro, our timer has expired, we do not want this to cause issues
	m_pp.RestTimer = 0;

	uint32 buff_count = GetMaxTotalSlots();
	for (unsigned int j = 0; j < buff_count; j++) {
		if(IsValidSpell(buffs[j].spellid)) {
			if(IsDetrimentalSpell(buffs[j].spellid) && (buffs[j].ticsremaining > 0))
				if(!IsRestAllowedSpell(buffs[j].spellid))
					return;
		}
	}

	ooc_regen = true;
}

void Client::DoTracking()
{
	if (!TrackingID) {
		return;
	}

	auto *m = entity_list.GetMob(TrackingID);

	if (!m || m->IsCorpse()) {
		MessageString(Chat::Skills, TRACK_LOST_TARGET);
		TrackingID = 0;
		return;
	}

	if (DistanceNoZ(m->GetPosition(), GetPosition()) < 10) {
		Message(
			Chat::Skills,
			fmt::format(
				"You have found {}.",
				m->GetCleanName()
			).c_str()
		);
		TrackingID = 0;
		return;
	}

	float relative_heading = GetHeading() - CalculateHeadingToTarget(m->GetX(), m->GetY());

	if (relative_heading < 0) {
		relative_heading += 512;
	}

	if (relative_heading > 480) {
		MessageString(Chat::Skills, TRACK_STRAIGHT_AHEAD, m->GetCleanName());
	} else if (relative_heading > 416) {
		MessageString(Chat::Skills, TRACK_AHEAD_AND_TO, m->GetCleanName(), "left");
	} else if (relative_heading > 352) {
		MessageString(Chat::Skills, TRACK_TO_THE, m->GetCleanName(), "left");
	} else if (relative_heading > 288) {
		MessageString(Chat::Skills, TRACK_BEHIND_AND_TO, m->GetCleanName(), "left");
	} else if (relative_heading > 224) {
		MessageString(Chat::Skills, TRACK_BEHIND_YOU, m->GetCleanName());
	} else if (relative_heading > 160) {
		MessageString(Chat::Skills, TRACK_BEHIND_AND_TO, m->GetCleanName(), "right");
	} else if (relative_heading > 96) {
		MessageString(Chat::Skills, TRACK_TO_THE, m->GetCleanName(), "right");
	} else if (relative_heading > 32) {
		MessageString(Chat::Skills, TRACK_AHEAD_AND_TO, m->GetCleanName(), "right");
	} else if (relative_heading >= 0) {
		MessageString(Chat::Skills, TRACK_STRAIGHT_AHEAD, m->GetCleanName());
	}
}

void Client::HandleRespawnFromHover(uint32 Option)
{
	RespawnFromHoverTimer.Disable();

	RespawnOption* chosen = nullptr;
	bool is_rez = false;

	//Find the selected option
	if (Option == 0)
	{
		chosen = &respawn_options.front();
	}
	else if (Option == (respawn_options.size() - 1))
	{
		chosen = &respawn_options.back();
		is_rez = true; //Rez must always be the last option
	}
	else
	{
		std::list<RespawnOption>::iterator itr;
		uint32 pos = 0;
		for (itr = respawn_options.begin(); itr != respawn_options.end(); ++itr)
		{
			if (pos++ == Option)
			{
				chosen = &(*itr);
				break;
			}
		}
	}

	//If they somehow chose an option they don't have, just send them to bind
	RespawnOption* default_to_bind = nullptr;
	if (!chosen)
	{
		/* put error logging here */
		BindStruct* b = &m_pp.binds[0];
		default_to_bind = new RespawnOption;
		default_to_bind->name = "Bind Location";
		default_to_bind->zone_id = b->zone_id;
		default_to_bind->instance_id = b->instance_id;
		default_to_bind->x = b->x;
		default_to_bind->y = b->y;
		default_to_bind->z = b->z;
		default_to_bind->heading = b->heading;
		chosen = default_to_bind;
		is_rez = false;
	}

	if (chosen->zone_id == zone->GetZoneID() && chosen->instance_id == zone->GetInstanceID()) //If they should respawn in the current zone...
	{
		if (is_rez)
		{
			if (PendingRezzXP < 0 || PendingRezzSpellID == 0)
			{
				LogSpells("Unexpected Rezz from hover request");
				safe_delete(default_to_bind);
				return;
			}
			SetHP(GetMaxHP() / 5);

			Corpse* corpse = entity_list.GetCorpseByName(PendingRezzCorpseName.c_str());

			if (corpse)
			{
				m_Position.x = corpse->GetX();
				m_Position.y = corpse->GetY();
				m_Position.z = corpse->GetZ();
			}

			auto outapp =
			    new EQApplicationPacket(OP_ZonePlayerToBind, sizeof(ZonePlayerToBind_Struct) + 10);
			ZonePlayerToBind_Struct* gmg = (ZonePlayerToBind_Struct*) outapp->pBuffer;

			gmg->bind_zone_id = zone->GetZoneID();
			gmg->bind_instance_id = zone->GetInstanceID();
			gmg->x = GetX();
			gmg->y = GetY();
			gmg->z = GetZ();
			gmg->heading = GetHeading();
			strcpy(gmg->zone_name, "Resurrect");

			FastQueuePacket(&outapp);

			ClearHover();
			SendHPUpdate();
			OPRezzAnswer(1, PendingRezzSpellID, zone->GetZoneID(), zone->GetInstanceID(), GetX(), GetY(), GetZ());

			if (corpse && corpse->IsCorpse())
			{
				LogSpells("Hover Rez in zone [{}] for corpse [{}]",
						zone->GetShortName(), PendingRezzCorpseName.c_str());

				LogSpells("Found corpse. Marking corpse as rezzed");

				corpse->IsRezzed(true);
				corpse->CompleteResurrection();
			}
		}
		else //Not rez
		{
			PendingRezzSpellID = 0;

			auto outapp = new EQApplicationPacket(OP_ZonePlayerToBind, sizeof(ZonePlayerToBind_Struct) +
										       chosen->name.length() + 1);
			ZonePlayerToBind_Struct* gmg = (ZonePlayerToBind_Struct*) outapp->pBuffer;

			gmg->bind_zone_id = zone->GetZoneID();
			gmg->bind_instance_id = chosen->instance_id;
			gmg->x = chosen->x;
			gmg->y = chosen->y;
			gmg->z = chosen->z;
			gmg->heading = chosen->heading;
			strcpy(gmg->zone_name, chosen->name.c_str());

			FastQueuePacket(&outapp);

			CalcBonuses();
			RestoreHealth();
			RestoreMana();
			RestoreEndurance();

			m_Position.x = chosen->x;
			m_Position.y = chosen->y;
			m_Position.z = chosen->z;
			m_Position.w = chosen->heading;

			ClearHover();
			entity_list.RefreshClientXTargets(this);
			SendHPUpdate();
		}

		//After they've respawned into the same zone, trigger EVENT_RESPAWN
		if (parse->PlayerHasQuestSub(EVENT_RESPAWN)) {
			parse->EventPlayer(EVENT_RESPAWN, this, std::to_string(Option), is_rez ? 1 : 0);
		}

		//Pop Rez option from the respawn options list;
		//easiest way to make sure it stays at the end and
		//doesn't disrupt adding/removing scripted options
		respawn_options.pop_back();
	}
	else
	{
		//Heading to a different zone
		if(isgrouped)
		{
			Group *g = GetGroup();
			if(g)
				g->MemberZoned(this);
		}

		Raid* r = entity_list.GetRaidByClient(this);
		if(r)
			r->MemberZoned(this);

		m_pp.zone_id = chosen->zone_id;
		m_pp.zoneInstance = chosen->instance_id;
		database.MoveCharacterToZone(CharacterID(), chosen->zone_id);

		Save();

		MovePC(chosen->zone_id, chosen->instance_id, chosen->x, chosen->y, chosen->z, chosen->heading, 1);
	}

	safe_delete(default_to_bind);
}

void Client::ClearHover()
{
	// Our Entity ID is currently zero, set in Client::Death
	SetID(entity_list.GetFreeID());

	auto outapp = new EQApplicationPacket(OP_ZoneEntry, sizeof(ServerZoneEntry_Struct));
	ServerZoneEntry_Struct* sze = (ServerZoneEntry_Struct*)outapp->pBuffer;

	FillSpawnStruct(&sze->player,CastToMob());

	sze->player.spawn.NPC = 0;
	sze->player.spawn.z += 6;	//arbitrary lift, seems to help spawning under zone.

	entity_list.QueueClients(this, outapp, false);
	safe_delete(outapp);

	if (IsClient() && CastToClient()->ClientVersionBit() & EQ::versions::maskUFAndLater)
	{
		EQApplicationPacket *outapp = MakeBuffsPacket(false);
		CastToClient()->FastQueuePacket(&outapp);
	}

	dead = false;
}

void Client::HandleLFGuildResponse(ServerPacket *pack)
{
	pack->SetReadPosition(8);

	char Tmp[257];

	pack->ReadString(Tmp);

	pack->ReadSkipBytes(4);
	uint32 SubType, NumberOfMatches;

	SubType = pack->ReadUInt32();

	switch(SubType)
	{
		case QSG_LFGuild_PlayerMatches:
		{
			NumberOfMatches = pack->ReadUInt32();
			uint32 StartOfMatches = pack->GetReadPosition();
			uint32 i = NumberOfMatches;
			uint32 PacketSize = 12;
			while(i > 0)
			{
				pack->ReadString(Tmp);
				PacketSize += strlen(Tmp) + 1;
				pack->ReadString(Tmp);
				PacketSize += strlen(Tmp) + 1;
				PacketSize += 16;
				pack->ReadSkipBytes(16);
				--i;
			}

			auto outapp = new EQApplicationPacket(OP_LFGuild, PacketSize);
			outapp->WriteUInt32(3);
			outapp->WriteUInt32(0xeb63);	// Don't know the significance of this value.
			outapp->WriteUInt32(NumberOfMatches);
			pack->SetReadPosition(StartOfMatches);

			while(NumberOfMatches > 0)
			{
				pack->ReadString(Tmp);
				outapp->WriteString(Tmp);
				pack->ReadString(Tmp);
				uint32 Level = pack->ReadUInt32();
				uint32 Class = pack->ReadUInt32();
				uint32 AACount = pack->ReadUInt32();
				uint32 TimeZone = pack->ReadUInt32();
				outapp->WriteUInt32(Level);
				outapp->WriteUInt32(Class);
				outapp->WriteUInt32(AACount);
				outapp->WriteUInt32(TimeZone);
				outapp->WriteString(Tmp);
				--NumberOfMatches;
			}

			FastQueuePacket(&outapp);
			break;
		}
		case QSG_LFGuild_RequestPlayerInfo:
		{
			auto outapp = new EQApplicationPacket(OP_LFGuild, sizeof(LFGuild_PlayerToggle_Struct));
			LFGuild_PlayerToggle_Struct *pts = (LFGuild_PlayerToggle_Struct *)outapp->pBuffer;

			pts->Command = 0;
			pack->ReadString(pts->Comment);
			pts->TimeZone = pack->ReadUInt32();
			pts->TimePosted = pack->ReadUInt32();
			pts->Toggle = pack->ReadUInt32();

			FastQueuePacket(&outapp);

			break;
		}
		case QSG_LFGuild_GuildMatches:
		{
			NumberOfMatches = pack->ReadUInt32();
			uint32 StartOfMatches = pack->GetReadPosition();
			uint32 i = NumberOfMatches;
			uint32 PacketSize = 12;
			while(i > 0)
			{
				pack->ReadString(Tmp);
				PacketSize += strlen(Tmp) + 1;
				pack->ReadSkipBytes(4);
				pack->ReadString(Tmp);
				PacketSize += strlen(Tmp) + 1;
				PacketSize += 4;
				--i;
			}

			auto outapp = new EQApplicationPacket(OP_LFGuild, PacketSize);
			outapp->WriteUInt32(4);
			outapp->WriteUInt32(0xeb63);
			outapp->WriteUInt32(NumberOfMatches);
			pack->SetReadPosition(StartOfMatches);

			while(NumberOfMatches > 0)
			{
				pack->ReadString(Tmp);
				uint32 TimeZone = pack->ReadUInt32();
				outapp->WriteString(Tmp);
				outapp->WriteUInt32(TimeZone);
				pack->ReadString(Tmp);
				outapp->WriteString(Tmp);
				--NumberOfMatches;
			}
			FastQueuePacket(&outapp);

			break;
		}
		case QSG_LFGuild_RequestGuildInfo:
		{

			char Comments[257];
			uint32 FromLevel, ToLevel, Classes, AACount, TimeZone, TimePosted;

			pack->ReadString(Comments);
			FromLevel = pack->ReadUInt32();
			ToLevel = pack->ReadUInt32();
			Classes = pack->ReadUInt32();
			AACount = pack->ReadUInt32();
			TimeZone = pack->ReadUInt32();
			TimePosted = pack->ReadUInt32();

			auto outapp = new EQApplicationPacket(OP_LFGuild, sizeof(LFGuild_GuildToggle_Struct));

			LFGuild_GuildToggle_Struct *gts = (LFGuild_GuildToggle_Struct *)outapp->pBuffer;
			gts->Command = 1;
			strcpy(gts->Comment, Comments);
			gts->FromLevel = FromLevel;
			gts->ToLevel = ToLevel;
			gts->Classes = Classes;
			gts->AACount = AACount;
			gts->TimeZone = TimeZone;
			gts->Toggle = 1;
			gts->TimePosted = TimePosted;
			gts->Name[0] = 0;

			FastQueuePacket(&outapp);

			break;
		}

		default:
			break;
	}

}

void Client::SendLFGuildStatus()
{
	auto pack = new ServerPacket(ServerOP_QueryServGeneric, strlen(GetName()) + 17);

	pack->WriteUInt32(zone->GetZoneID());
	pack->WriteUInt32(zone->GetInstanceID());
	pack->WriteString(GetName());
	pack->WriteUInt32(QSG_LFGuild);
	pack->WriteUInt32(QSG_LFGuild_RequestPlayerInfo);

	worldserver.SendPacket(pack);
	safe_delete(pack);

}

void Client::SendGuildLFGuildStatus()
{
	auto pack = new ServerPacket(ServerOP_QueryServGeneric,
				     strlen(GetName()) + +strlen(guild_mgr.GetGuildName(GuildID())) + 18);

	pack->WriteUInt32(zone->GetZoneID());
	pack->WriteUInt32(zone->GetInstanceID());
	pack->WriteString(GetName());
	pack->WriteUInt32(QSG_LFGuild);
	pack->WriteUInt32(QSG_LFGuild_RequestGuildInfo);
	pack->WriteString(guild_mgr.GetGuildName(GuildID()));

	worldserver.SendPacket(pack);
	safe_delete(pack);
}

bool Client::CheckWaterAutoFireLoS(Mob* m)
{
	if (
		!RuleB(Combat, WaterMatchRequiredForAutoFireLoS) ||
		!zone->watermap
	) {
		return true;
	}

	return (
		zone->watermap->InLiquid(GetPosition()) ==
		zone->watermap->InLiquid(m->GetPosition())
	);
}
