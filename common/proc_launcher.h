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

#include "common/platform/platform.h"
#include "common/types.h"

#include <map>
#include <string>
#include <vector>

//I forced this object to become a singleton because it registers its
//signal handler for UNIX
class ProcLauncher {
	ProcLauncher();
public:
	//Singleton method
	static ProcLauncher *get() { return(&s_launcher); }
	static void ProcessInThisThread();

#ifdef WIN32
	typedef DWORD ProcRef;
	static const ProcRef ProcError;
#else
	typedef pid_t ProcRef;
	static const ProcRef ProcError;
#endif
	class EventHandler;
	class Spec {
		friend class ProcLauncher;	//for visual c++
	public:
		Spec();
		Spec(const Spec &other);
		Spec &operator=(const Spec &other);

		std::string program;
		std::vector<std::string> args;
		//std::map<std::string,std::string> environment;
		EventHandler *handler;	//optional, we do not own this pointer
		std::string logFile;	//empty = do not redirect output.
	protected:
		//None of these fields get copied around
#ifdef WIN32
		PROCESS_INFORMATION proc_info;
#endif
	};
	class EventHandler {
	public:
		virtual ~EventHandler() {}
		virtual void OnTerminate(const ProcRef &ref, const Spec *spec) = 0;
	};

	/* The main launch method, call to start a new background process. */
	ProcRef Launch(Spec *&to_launch);	//takes ownership of the pointer

	/* The terminate method */
	bool Terminate(const ProcRef &proc, bool graceful = true);
	void TerminateAll(bool final = true);

	/* The main processing method. Call regularly to check for terminated background processes. */
	void Process();

protected:
	//std::vector<Spec *> m_specs;
	std::map<ProcRef, Spec *> m_running;	//we own the pointers in this map

	void ProcessTerminated(std::map<ProcRef, Spec *>::iterator &it);

private:
	static ProcLauncher s_launcher;
#ifndef WIN32
	uint32 m_signalCount;
	static void HandleSigChild(int signum);
#endif
};
