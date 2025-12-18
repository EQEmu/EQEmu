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

#include "zone_launch.h"

#include "common/eqemu_config.h"
#include "common/eqemu_logsys.h"
#include "eqlaunch/worldserver.h"

//static const uint32 ZONE_RESTART_DELAY = 10000;
//static const uint32 ZONE_TERMINATE_WAIT = 10000;

int ZoneLaunch::s_running = 0;	//the number of zones running under this launcher
Timer ZoneLaunch::s_startTimer(1);	//I do not trust this things state after static initialization

ZoneLaunch::ZoneLaunch(WorldServer *world, const char *launcher_name,
const char *zone_name, uint16 port, const EQEmuConfig *config)
: m_state(StateStartPending),
	m_world(world),
	m_zone(zone_name),
	m_port(port),
	m_launcherName(launcher_name),
	m_config(config),
	m_timer(config->RestartWait),
	m_ref(ProcLauncher::ProcError),
	m_startCount(0),
	m_killFails(0)
{
	//trigger the startup timer initially so it boots the first time.
	m_timer.Trigger();
}

ZoneLaunch::~ZoneLaunch() {
	if(IsRunning())
		s_running--;
}

void ZoneLaunch::SendStatus() const {
	m_world->SendStatus(m_zone.c_str(), m_startCount, IsRunning());
}

void ZoneLaunch::Start() {
	auto spec = new ProcLauncher::Spec();
	spec->program = m_config->ZoneExe;

	if(m_port) {
		std::string arg = m_zone + std::string(":") + std::to_string(m_port);
		spec->args.push_back(arg);
	} else {
		spec->args.push_back(m_zone);
	}

	spec->args.push_back(m_launcherName);
	spec->handler = this;
	spec->logFile = m_config->LogPrefix + m_zone + m_config->LogSuffix;

	//spec is consumed, even on failure
	m_ref = ProcLauncher::get()->Launch(spec);
	if(m_ref == ProcLauncher::ProcError) {
		Log(Logs::Detail, Logs::Launcher, "Failure to launch '%s %s %s'. ", m_config->ZoneExe.c_str(), m_zone.c_str(), m_launcherName);
		m_timer.Start(m_config->RestartWait);
		return;
	}

	m_startCount++;
	m_state = StateStarted;
	s_running++;
	m_killFails = 0;

	SendStatus();

	Log(Logs::Detail, Logs::Launcher, "Zone %s has been started.", m_zone.c_str());
}

void ZoneLaunch::Restart() {
	switch(m_state) {
	case StateRestartPending:
		Log(Logs::Detail, Logs::Launcher, "Restart of zone %s requested when a restart is already pending.", m_zone.c_str());
		break;
	case StateStartPending:
		//we havent started yet, do nothing
		Log(Logs::Detail, Logs::Launcher, "Restart of %s before it has started. Ignoring.", m_zone.c_str());
		break;
	case StateStarted:
		//process is running along, kill it off..
		if(m_ref == ProcLauncher::ProcError)
			break;	//we have no proc ref... cannot stop..
		if(!ProcLauncher::get()->Terminate(m_ref, true)) {
			//failed to terminate the process, its not likely that it will work if we try again, so give up.
			Log(Logs::Detail, Logs::Launcher, "Failed to terminate zone %s. Giving up and moving to stopped.", m_zone.c_str());
			m_state = StateStopped;
			break;
		}
		Log(Logs::Detail, Logs::Launcher, "Termination signal sent to zone %s.", m_zone.c_str());
		m_timer.Start(m_config->TerminateWait);
		m_state = StateRestartPending;
		break;
	case StateStopPending:
		Log(Logs::Detail, Logs::Launcher, "Restart of zone %s requested when a stop is pending. Ignoring.", m_zone.c_str());
		break;
	case StateStopped:
		//process is already stopped... nothing to do..
		Log(Logs::Detail, Logs::Launcher, "Restart requested when zone %s is already stopped.", m_zone.c_str());
		break;
	}
}

void ZoneLaunch::Stop(bool graceful) {
	switch(m_state) {
	case StateStartPending:
		//we havent started yet, transition directly to stopped.
		Log(Logs::Detail, Logs::Launcher, "Stopping zone %s before it has started.", m_zone.c_str());
		m_state = StateStopped;
		break;
	case StateStarted:
	case StateRestartPending:
	case StateStopPending:
		if(m_ref == ProcLauncher::ProcError)
			break;	//we have no proc ref... cannot stop..
		if(!ProcLauncher::get()->Terminate(m_ref, graceful)) {
			//failed to terminate the process, its not likely that it will work if we try again, so give up.
			Log(Logs::Detail, Logs::Launcher, "Failed to terminate zone %s. Giving up and moving to stopped.", m_zone.c_str());
			m_state = StateStopped;
			break;
		}
		Log(Logs::Detail, Logs::Launcher, "Termination signal sent to zone %s.", m_zone.c_str());
		m_timer.Start(m_config->TerminateWait);
		m_state = StateStopPending;
		break;
	case StateStopped:
		//process is already stopped... nothing to do..
		Log(Logs::Detail, Logs::Launcher, "Stop requested when zone %s is already stopped.", m_zone.c_str());
		break;
	}
}

bool ZoneLaunch::Process() {
	switch(m_state) {
	case StateStartPending:
		if(m_timer.Check(false)) {
			//our internal timer says its time to start. Check with the shared timer.
			if(!s_startTimer.Check(false)) {
				//we have to wait on the shared timer now..
				break;
			}

			//ok, both timers say we can start.
			//disable our internal timer, will get started again if it is needed.
			m_timer.Disable();

			//actually start up the program
			Log(Logs::Detail, Logs::Launcher, "Starting zone %s", m_zone.c_str());
			Start();

			//now update the shared timer to reflect the proper start interval.
			if(s_running == 1) {
				//we are the first zone started. wait that interval.
				Log(Logs::Detail, Logs::Launcher, "Waiting %d milliseconds before booting the second zone.", m_config->InitialBootWait);
				s_startTimer.Start(m_config->InitialBootWait);
			} else {
				//just some follow on zone, use that interval.
				Log(Logs::Detail, Logs::Launcher, "Waiting %d milliseconds before booting the next zone.", m_config->ZoneBootInterval);
				s_startTimer.Start(m_config->ZoneBootInterval);
			}

		}	//else, timer still ticking, keep waiting
		break;
	case StateStarted:
		//happy state, do nothing..
		break;
	case StateRestartPending:
		//waiting for notification that our child has died..
		if(m_timer.Check()) {
			//we have timed out, try to kill the child again
			Log(Logs::Detail, Logs::Launcher, "Zone %s refused to die, killing again.", m_zone.c_str());
			Restart();
		}
		break;
	case StateStopPending:
		//waiting for notification that our child has died..
		if(m_timer.Check()) {
			//we have timed out, try to kill the child again
			m_killFails++;
			if(m_killFails > 5) {	//should get this number from somewhere..
				Log(Logs::Detail, Logs::Launcher, "Zone %s refused to die, giving up and acting like its dead.", m_zone.c_str());
				m_state = StateStopped;
				s_running--;
				SendStatus();
			} else {
				Log(Logs::Detail, Logs::Launcher, "Zone %s refused to die, killing again.", m_zone.c_str());
				Stop(false);
			}
		}
		break;
	case StateStopped:
		//signal our caller to remove us
		return(false);
		break;
	}
	return(true);
}

//called when the process actually dies off...
void ZoneLaunch::OnTerminate(const ProcLauncher::ProcRef &ref, const ProcLauncher::Spec *spec) {
	s_running--;

	switch(m_state) {
	case StateStartPending:
		Log(Logs::Detail, Logs::Launcher, "Zone %s has gone down before we started it..?? Restart timer started.", m_zone.c_str());
		m_state = StateStartPending;
		m_timer.Start(m_config->RestartWait);
		break;
	case StateStarted:
		//something happened to our happy process...
		Log(Logs::Detail, Logs::Launcher, "Zone %s has gone down. Restart timer started.", m_zone.c_str());
		m_state = StateStartPending;
		m_timer.Start(m_config->RestartWait);
		break;
	case StateRestartPending:
		//it finally died, start it on up again
		Log(Logs::Detail, Logs::Launcher, "Zone %s has terminated. Transitioning to starting state.", m_zone.c_str());
		m_state = StateStartPending;
		break;
	case StateStopPending:
		//it finally died, transition to close.
		Log(Logs::Detail, Logs::Launcher, "Zone %s has terminated. Transitioning to stopped state.", m_zone.c_str());
		m_state = StateStopped;
		break;
	case StateStopped:
		//we already thought it was stopped... dont care...
		Log(Logs::Detail, Logs::Launcher, "Notified of zone %s terminating when we thought it was stopped.", m_zone.c_str());
		break;
	}

	SendStatus();
}

