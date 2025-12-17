/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2019 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
*/

#pragma once

#include "common/eqemu_logsys.h"

inline auto logsys = EQEmuLogSys::Instance();

#define LogAA(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::AA))\
        OutF(logsys, Logs::General, Logs::AA, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAADetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::AA))\
        OutF(logsys, Logs::Detail, Logs::AA, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAI(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::AI))\
        OutF(logsys, Logs::General, Logs::AI, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAIDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::AI))\
        OutF(logsys, Logs::Detail, Logs::AI, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAggro(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Aggro))\
        OutF(logsys, Logs::General, Logs::Aggro, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAggroDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Aggro))\
        OutF(logsys, Logs::Detail, Logs::Aggro, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAttack(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Attack))\
        OutF(logsys, Logs::General, Logs::Attack, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAttackDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Attack))\
        OutF(logsys, Logs::Detail, Logs::Attack, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogPacketClientServer(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::PacketClientServer))\
        OutF(logsys, Logs::General, Logs::PacketClientServer, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogPacketClientServerDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::PacketClientServer))\
        OutF(logsys, Logs::Detail, Logs::PacketClientServer, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogCombat(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Combat))\
        OutF(logsys, Logs::General, Logs::Combat, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogCombatDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Combat))\
        OutF(logsys, Logs::Detail, Logs::Combat, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogCommands(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Commands))\
        OutF(logsys, Logs::General, Logs::Commands, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogCommandsDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Commands))\
        OutF(logsys, Logs::Detail, Logs::Commands, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogCrash(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Crash))\
        OutF(logsys, Logs::General, Logs::Crash, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogCrashDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Crash))\
        OutF(logsys, Logs::Detail, Logs::Crash, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogDebug(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Debug))\
        OutF(logsys, Logs::General, Logs::Debug, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogDebugDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Debug))\
        OutF(logsys, Logs::Detail, Logs::Debug, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogDoors(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Doors))\
        OutF(logsys, Logs::General, Logs::Doors, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogDoorsDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Doors))\
        OutF(logsys, Logs::Detail, Logs::Doors, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogError(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Error))\
        OutF(logsys, Logs::General, Logs::Error, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogErrorDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Error))\
        OutF(logsys, Logs::Detail, Logs::Error, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogEvolveItem(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::EvolveItem))\
        OutF(logsys, Logs::General, Logs::EvolveItem, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogEvolveItemDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::EvolveItem))\
        OutF(logsys, Logs::Detail, Logs::EvolveItem, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogGuilds(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Guilds))\
        OutF(logsys, Logs::General, Logs::Guilds, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogGuildsDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Guilds))\
        OutF(logsys, Logs::Detail, Logs::Guilds, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogInventory(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Inventory))\
        OutF(logsys, Logs::General, Logs::Inventory, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogInventoryDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Inventory))\
        OutF(logsys, Logs::Detail, Logs::Inventory, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogLauncher(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Launcher))\
        OutF(logsys, Logs::General, Logs::Launcher, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogLauncherDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Launcher))\
        OutF(logsys, Logs::Detail, Logs::Launcher, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogNetcode(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Netcode))\
        OutF(logsys, Logs::General, Logs::Netcode, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogNetcodeDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Netcode))\
        OutF(logsys, Logs::Detail, Logs::Netcode, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogNormal(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Normal))\
        OutF(logsys, Logs::General, Logs::Normal, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogNormalDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Normal))\
        OutF(logsys, Logs::Detail, Logs::Normal, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogObject(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Object))\
        OutF(logsys, Logs::General, Logs::Object, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogObjectDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Object))\
        OutF(logsys, Logs::Detail, Logs::Object, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogPathing(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Pathing))\
        OutF(logsys, Logs::General, Logs::Pathing, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogPathingDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Pathing))\
        OutF(logsys, Logs::Detail, Logs::Pathing, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogQuests(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Quests))\
        OutF(logsys, Logs::General, Logs::Quests, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogQuestsDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Quests))\
        OutF(logsys, Logs::Detail, Logs::Quests, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogRules(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Rules))\
        OutF(logsys, Logs::General, Logs::Rules, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogRulesDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Rules))\
        OutF(logsys, Logs::Detail, Logs::Rules, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogSkills(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Skills))\
        OutF(logsys, Logs::General, Logs::Skills, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogSkillsDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Skills))\
        OutF(logsys, Logs::Detail, Logs::Skills, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogSpawns(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Spawns))\
        OutF(logsys, Logs::General, Logs::Spawns, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogSpawnsDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Spawns))\
        OutF(logsys, Logs::Detail, Logs::Spawns, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogSpells(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Spells))\
        OutF(logsys, Logs::General, Logs::Spells, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogSpellsDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Spells))\
        OutF(logsys, Logs::Detail, Logs::Spells, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogTasks(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Tasks))\
        OutF(logsys, Logs::General, Logs::Tasks, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogTasksDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Tasks))\
        OutF(logsys, Logs::Detail, Logs::Tasks, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogTradeskills(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Tradeskills))\
        OutF(logsys, Logs::General, Logs::Tradeskills, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogTradeskillsDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Tradeskills))\
        OutF(logsys, Logs::Detail, Logs::Tradeskills, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogTrading(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Trading))\
        OutF(logsys, Logs::General, Logs::Trading, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogTradingDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Trading))\
        OutF(logsys, Logs::Detail, Logs::Trading, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogTribute(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Tribute))\
        OutF(logsys, Logs::General, Logs::Tribute, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogTributeDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Tribute))\
        OutF(logsys, Logs::Detail, Logs::Tribute, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogMySQLError(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::MySQLError))\
        OutF(logsys, Logs::General, Logs::MySQLError, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogMySQLErrorDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::MySQLError))\
        OutF(logsys, Logs::Detail, Logs::MySQLError, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogMySQLQuery(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::MySQLQuery))\
        OutF(logsys, Logs::General, Logs::MySQLQuery, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogMySQLQueryDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::MySQLQuery))\
        OutF(logsys, Logs::Detail, Logs::MySQLQuery, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogMercenaries(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Mercenaries))\
        OutF(logsys, Logs::General, Logs::Mercenaries, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogMercenariesDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Mercenaries))\
        OutF(logsys, Logs::Detail, Logs::Mercenaries, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogQuestDebug(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::QuestDebug))\
        OutF(logsys, Logs::General, Logs::QuestDebug, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogQuestDebugDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::QuestDebug))\
        OutF(logsys, Logs::Detail, Logs::QuestDebug, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogPacketServerClient(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::PacketServerClient))\
        OutF(logsys, Logs::General, Logs::PacketServerClient, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogPacketServerClientDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::PacketServerClient))\
        OutF(logsys, Logs::Detail, Logs::PacketServerClient, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)


#define LogLoginserver(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Loginserver))\
        OutF(logsys, Logs::General, Logs::Loginserver, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogLoginserverDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Loginserver))\
        OutF(logsys, Logs::Detail, Logs::Loginserver, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogClientLogin(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::ClientLogin))\
        OutF(logsys, Logs::General, Logs::ClientLogin, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogClientLoginDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::ClientLogin))\
        OutF(logsys, Logs::Detail, Logs::ClientLogin, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogHPUpdate(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::HPUpdate))\
        OutF(logsys, Logs::General, Logs::HPUpdate, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogHPUpdateDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::HPUpdate))\
        OutF(logsys, Logs::Detail, Logs::HPUpdate, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogFixZ(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::FixZ))\
        OutF(logsys, Logs::General, Logs::FixZ, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogFixZDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::FixZ))\
        OutF(logsys, Logs::Detail, Logs::FixZ, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogFood(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Food))\
        OutF(logsys, Logs::General, Logs::Food, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogFoodDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Food))\
        OutF(logsys, Logs::Detail, Logs::Food, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogTraps(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Traps))\
        OutF(logsys, Logs::General, Logs::Traps, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogTrapsDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Traps))\
        OutF(logsys, Logs::Detail, Logs::Traps, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogNPCRoamBox(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::NPCRoamBox))\
        OutF(logsys, Logs::General, Logs::NPCRoamBox, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogNPCRoamBoxDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::NPCRoamBox))\
        OutF(logsys, Logs::Detail, Logs::NPCRoamBox, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogNPCScaling(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::NPCScaling))\
        OutF(logsys, Logs::General, Logs::NPCScaling, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogNPCScalingDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::NPCScaling))\
        OutF(logsys, Logs::Detail, Logs::NPCScaling, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogMobAppearance(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::MobAppearance))\
        OutF(logsys, Logs::General, Logs::MobAppearance, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogMobAppearanceDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::MobAppearance))\
        OutF(logsys, Logs::Detail, Logs::MobAppearance, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogInfo(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Info))\
        OutF(logsys, Logs::General, Logs::Info, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogInfoDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Info))\
        OutF(logsys, Logs::Detail, Logs::Info, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogWarning(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Warning))\
        OutF(logsys, Logs::General, Logs::Warning, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogWarningDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Warning))\
        OutF(logsys, Logs::Detail, Logs::Warning, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogCritical(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Critical))\
        OutF(logsys, Logs::General, Logs::Critical, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogCriticalDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Critical))\
        OutF(logsys, Logs::Detail, Logs::Critical, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogEmergency(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Emergency))\
        OutF(logsys, Logs::General, Logs::Emergency, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogEmergencyDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Emergency))\
        OutF(logsys, Logs::Detail, Logs::Emergency, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAlert(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Alert))\
        OutF(logsys, Logs::General, Logs::Alert, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAlertDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Alert))\
        OutF(logsys, Logs::Detail, Logs::Alert, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogNotice(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Notice))\
        OutF(logsys, Logs::General, Logs::Notice, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogNoticeDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Notice))\
        OutF(logsys, Logs::Detail, Logs::Notice, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAIScanClose(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::AIScanClose))\
        OutF(logsys, Logs::General, Logs::AIScanClose, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAIScanCloseDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::AIScanClose))\
        OutF(logsys, Logs::Detail, Logs::AIScanClose, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAIYellForHelp(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::AIYellForHelp))\
        OutF(logsys, Logs::General, Logs::AIYellForHelp, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAIYellForHelpDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::AIYellForHelp))\
        OutF(logsys, Logs::Detail, Logs::AIYellForHelp, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAICastBeneficialClose(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::AICastBeneficialClose))\
        OutF(logsys, Logs::General, Logs::AICastBeneficialClose, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAICastBeneficialCloseDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::AICastBeneficialClose))\
        OutF(logsys, Logs::Detail, Logs::AICastBeneficialClose, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAoeCast(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::AoeCast))\
        OutF(logsys, Logs::General, Logs::AoeCast, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAoeCastDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::AoeCast))\
        OutF(logsys, Logs::Detail, Logs::AoeCast, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogEntityManagement(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::EntityManagement))\
        OutF(logsys, Logs::General, Logs::EntityManagement, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogEntityManagementDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::EntityManagement))\
        OutF(logsys, Logs::Detail, Logs::EntityManagement, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogFlee(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Flee))\
        OutF(logsys, Logs::General, Logs::Flee, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogFleeDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Flee))\
        OutF(logsys, Logs::Detail, Logs::Flee, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAura(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Aura))\
        OutF(logsys, Logs::General, Logs::Aura, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogAuraDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Aura))\
        OutF(logsys, Logs::Detail, Logs::Aura, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogHotReload(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::HotReload))\
        OutF(logsys, Logs::General, Logs::HotReload, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogHotReloadDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::HotReload))\
        OutF(logsys, Logs::Detail, Logs::HotReload, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogMerchants(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Merchants))\
        OutF(logsys, Logs::General, Logs::Merchants, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogMerchantsDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Merchants))\
        OutF(logsys, Logs::Detail, Logs::Merchants, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogZonePoints(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::ZonePoints))\
        OutF(logsys, Logs::General, Logs::ZonePoints, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogZonePointsDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::ZonePoints))\
        OutF(logsys, Logs::Detail, Logs::ZonePoints, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogLoot(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Loot))\
        OutF(logsys, Logs::General, Logs::Loot, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogLootDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Loot))\
        OutF(logsys, Logs::Detail, Logs::Loot, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogExpeditions(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Expeditions))\
        OutF(logsys, Logs::General, Logs::Expeditions, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogExpeditionsDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Expeditions))\
        OutF(logsys, Logs::Detail, Logs::Expeditions, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogDynamicZones(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::DynamicZones))\
        OutF(logsys, Logs::General, Logs::DynamicZones, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogDynamicZonesDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::DynamicZones))\
        OutF(logsys, Logs::Detail, Logs::DynamicZones, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogScheduler(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Scheduler))\
        OutF(logsys, Logs::General, Logs::Scheduler, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogSchedulerDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Scheduler))\
        OutF(logsys, Logs::Detail, Logs::Scheduler, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogCheat(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Cheat))\
        OutF(logsys, Logs::General, Logs::Cheat, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogCheatDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Cheat))\
        OutF(logsys, Logs::Detail, Logs::Cheat, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogClientList(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::ClientList))\
        OutF(logsys, Logs::General, Logs::ClientList, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogClientListDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::ClientList))\
        OutF(logsys, Logs::Detail, Logs::ClientList, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogDiaWind(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::DiaWind))\
        OutF(logsys, Logs::General, Logs::DiaWind, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogDiaWindDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::DiaWind))\
        OutF(logsys, Logs::Detail, Logs::DiaWind, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogHTTP(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::HTTP))\
        OutF(logsys, Logs::General, Logs::HTTP, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogHTTPDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::HTTP))\
        OutF(logsys, Logs::Detail, Logs::HTTP, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogSaylink(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Saylink))\
        OutF(logsys, Logs::General, Logs::Saylink, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogSaylinkDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Saylink))\
        OutF(logsys, Logs::Detail, Logs::Saylink, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogChecksumVerification(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::ChecksumVerification))\
        OutF(logsys, Logs::General, Logs::ChecksumVerification, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogChecksumVerificationDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::ChecksumVerification))\
        OutF(logsys, Logs::Detail, Logs::ChecksumVerification, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogCombatRecord(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::CombatRecord))\
        OutF(logsys, Logs::General, Logs::CombatRecord, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogCombatRecordDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::CombatRecord))\
        OutF(logsys, Logs::Detail, Logs::CombatRecord, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogHate(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Hate))\
        OutF(logsys, Logs::General, Logs::Hate, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogHateDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Hate))\
        OutF(logsys, Logs::Detail, Logs::Hate, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogDiscord(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Discord))\
        OutF(logsys, Logs::General, Logs::Discord, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogDiscordDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Discord))\
        OutF(logsys, Logs::Detail, Logs::Discord, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogFaction(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Faction))\
        OutF(logsys, Logs::General, Logs::Faction, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogFactionDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Faction))\
        OutF(logsys, Logs::Detail, Logs::Faction, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogPacketServerToServer(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::PacketServerToServer))\
        OutF(logsys, Logs::General, Logs::PacketServerToServer, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogPacketServerToServerDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::PacketServerToServer))\
        OutF(logsys, Logs::Detail, Logs::PacketServerToServer, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

// manually created

#define LogBugs(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Bugs))\
        OutF(logsys, Logs::General, Logs::Bugs, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogBugsDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Bugs))\
        OutF(logsys, Logs::Detail, Logs::Bugs, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogQuestErrors(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::QuestErrors))\
        OutF(logsys, Logs::General, Logs::QuestErrors, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogQuestErrorsDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::QuestErrors))\
        OutF(logsys, Logs::Detail, Logs::QuestErrors, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogPlayerEvents(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::PlayerEvents))\
        OutF(logsys, Logs::General, Logs::PlayerEvents, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogPlayerEventsDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::PlayerEvents))\
        OutF(logsys, Logs::Detail, Logs::PlayerEvents, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogDataBuckets(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::DataBuckets))\
        OutF(logsys, Logs::General, Logs::DataBuckets, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogDataBucketsDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::DataBuckets))\
        OutF(logsys, Logs::Detail, Logs::DataBuckets, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogZoning(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Zoning))\
        OutF(logsys, Logs::General, Logs::Zoning, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogZoningDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Zoning))\
        OutF(logsys, Logs::Detail, Logs::Zoning, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogEqTime(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::EqTime))\
        OutF(logsys, Logs::General, Logs::EqTime, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogEqTimeDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::EqTime))\
        OutF(logsys, Logs::Detail, Logs::EqTime, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogCorpses(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::Corpses))\
        OutF(logsys, Logs::General, Logs::Corpses, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogCorpsesDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::Corpses))\
        OutF(logsys, Logs::Detail, Logs::Corpses, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogXTargets(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::XTargets))\
        OutF(logsys, Logs::General, Logs::XTargets, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogXTargetsDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::XTargets))\
        OutF(logsys, Logs::Detail, Logs::XTargets, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogPositionUpdate(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::PositionUpdate))\
        OutF(logsys, Logs::General, Logs::PositionUpdate, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogPositionUpdateDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::PositionUpdate))\
        OutF(logsys, Logs::Detail, Logs::PositionUpdate, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__); \
} while (0)

#define LogKSM(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::KSM))\
        OutF(logsys, Logs::General, Logs::KSM, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogKSMDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::KSM))\
        OutF(logsys, Logs::Detail, Logs::KSM, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogBotSettings(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::BotSettings))\
        OutF(logsys, Logs::General, Logs::BotSettings, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogBotSettingsDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::BotSettings))\
        OutF(logsys, Logs::Detail, Logs::BotSettings, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogBotSpellChecks(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::BotSpellChecks))\
        OutF(logsys, Logs::General, Logs::BotSpellChecks, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogBotSpellChecksDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::BotSpellChecks))\
        OutF(logsys, Logs::Detail, Logs::BotSpellChecks, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogBotSpellTypeChecks(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::BotSpellTypeChecks))\
        OutF(logsys, Logs::General, Logs::BotSpellTypeChecks, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogBotSpellTypeChecksDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::BotSpellTypeChecks))\
        OutF(logsys, Logs::Detail, Logs::BotSpellTypeChecks, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogNpcHandin(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::NpcHandin))\
        OutF(logsys, Logs::General, Logs::NpcHandin, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogNpcHandinDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::NpcHandin))\
        OutF(logsys, Logs::Detail, Logs::NpcHandin, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogZoneState(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::ZoneState))\
        OutF(logsys, Logs::General, Logs::ZoneState, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogZoneStateDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::ZoneState))\
        OutF(logsys, Logs::Detail, Logs::ZoneState, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogNetClient(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::NetClient))\
        OutF(logsys, Logs::General, Logs::NetClient, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogNetClientDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::NetClient))\
        OutF(logsys, Logs::Detail, Logs::NetClient, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogNetTCP(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::General, Logs::NetTCP))\
        OutF(logsys, Logs::General, Logs::NetTCP, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogNetTCPDetail(message, ...) do {\
    if (logsys->IsLogEnabled(Logs::Detail, Logs::NetTCP))\
        OutF(logsys, Logs::Detail, Logs::NetTCP, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define Log(debug_level, log_category, message, ...) do {\
    if (logsys->IsLogEnabled(debug_level, log_category))\
        logsys->Out(debug_level, log_category, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)

#define LogF(debug_level, log_category, message, ...) do {\
    if (logsys->IsLogEnabled(debug_level, log_category))\
        OutF(logsys, debug_level, log_category, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__);\
} while (0)
