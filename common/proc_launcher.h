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
