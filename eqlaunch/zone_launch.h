/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include "common/proc_launcher.h"
#include "common/timer.h"

#include <string>

class WorldServer;
class EQEmuConfig;

class ZoneLaunch : protected ProcLauncher::EventHandler {
public:
	ZoneLaunch(WorldServer *world, const char *launcher_name,
		const char *zone_name, uint16 port, const EQEmuConfig *config);
	virtual ~ZoneLaunch();

	void Stop(bool graceful = true);
	void Restart();

	bool Process();

	void SendStatus() const;

	const char *GetZone() const { return(m_zone.c_str()); }

protected:
	bool IsRunning() const { return(m_state == StateStarted || m_state == StateStopPending || m_state == StateRestartPending); }

	void Start();

	void OnTerminate(const ProcLauncher::ProcRef &ref, const ProcLauncher::Spec *spec);

	enum {
		StateStartPending,
		StateStarted,
		StateRestartPending,
		StateStopPending,
		StateStopped
	} m_state;

	WorldServer *const m_world;
	const std::string m_zone;
	const char *const m_launcherName;
	const EQEmuConfig *const m_config;
	const uint16 m_port;

	Timer m_timer;
	ProcLauncher::ProcRef m_ref;
	uint32 m_startCount;

	uint32 m_killFails;

private:
	static int s_running;
	static Timer s_startTimer;
};
