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

#include "proc_launcher.h"

#include "common/types.h"

#include <csignal>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#ifdef _WINDOWS
#include "common/platform/win/include_windows.h"
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#endif

ProcLauncher ProcLauncher::s_launcher;

#ifdef _WINDOWS
const ProcLauncher::ProcRef ProcLauncher::ProcError = 0xFFFFFFFF;
#else
const ProcLauncher::ProcRef ProcLauncher::ProcError = -1;
#endif

ProcLauncher::ProcLauncher()
{
#ifndef WIN32
	if(signal(SIGCHLD, ProcLauncher::HandleSigChild) == SIG_ERR)
		fprintf(stderr, "Unable to register child signal handler. Thats bad.");
	m_signalCount = 0;
#endif
}

void ProcLauncher::Process() {
#ifdef _WINDOWS
	std::map<ProcRef, Spec *>::iterator cur, end, tmp;
	cur = m_running.begin();
	end = m_running.end();
	while(cur != end) {
		DWORD res;
		if(GetExitCodeProcess(cur->second->proc_info.hProcess, &res)) {
			//got exit code, see if its still running...
			if(res == STILL_ACTIVE) {
				++cur;
				continue;
			}
			//else, it died, handle properly
		} else {
			//not sure the right thing to do here... why would this fail?
			//GetLastError();
			TerminateProcess(cur->second->proc_info.hProcess, 1);
		}

		//if we get here, the current process died.
		tmp = cur;
		++tmp;
		ProcessTerminated(cur);
		cur = tmp;
	}
#else	//!WIN32
	while(m_signalCount > 0) {
		m_signalCount--;
		int status;
		ProcRef died = waitpid(-1, &status, WNOHANG);
		if(died == -1) {
			//error waiting... shouldent really happen...

		} else if(died == 0) {
			//nothing pending...
			break;
		} else {
			//one died...
			std::map<ProcRef, Spec *>::iterator ref;
			ref = m_running.find(died);
			if(ref == m_running.end()) {
				//unable to find this process in our list...
			} else {
				//found... hooray
				ProcessTerminated(ref);
			}
		}
	}
#endif	//!WIN32

}

void ProcLauncher::ProcessTerminated(std::map<ProcRef, Spec *>::iterator &it) {

	if(it->second->handler != nullptr)
		it->second->handler->OnTerminate(it->first, it->second);

#ifdef _WINDOWS
	CloseHandle(it->second->proc_info.hProcess);
#else	//!WIN32
#endif	//!WIN32
	delete it->second;
	m_running.erase(it);
}

ProcLauncher::ProcRef ProcLauncher::Launch(Spec *&to_launch) {
	//consume the pointer
	Spec *it = to_launch;
	to_launch = nullptr;

#ifdef _WINDOWS
	STARTUPINFO siStartInfo;
	BOOL bFuncRetn = FALSE;

	// Set up members of the PROCESS_INFORMATION structure.

	ZeroMemory( &it->proc_info, sizeof(PROCESS_INFORMATION) );

	// Set up members of the STARTUPINFO structure.

	ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.dwFlags = 0;

	//handle output redirection.
	HANDLE logOut = nullptr;
	BOOL inherit_handles = FALSE;
	if(it->logFile.length() > 0) {
		inherit_handles = TRUE;
		// Set up our log file to redirect output into.
		SECURITY_ATTRIBUTES saAttr;
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;		//we want this handle to be inherited by the child.
		saAttr.lpSecurityDescriptor = nullptr;
		logOut = CreateFile(
			it->logFile.c_str(),	//lpFileName
			FILE_WRITE_DATA,		//dwDesiredAccess
			FILE_SHARE_READ,		//dwShareMode
			&saAttr,				//lpSecurityAttributes
			CREATE_ALWAYS,			//dwCreationDisposition
			FILE_FLAG_NO_BUFFERING,	//dwFlagsAndAttributes
			nullptr );					//hTemplateFile

		//configure the startup info to redirect output appropriately.
		siStartInfo.hStdError = logOut;
		siStartInfo.hStdOutput = logOut;
		siStartInfo.hStdInput = nullptr;
		siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
	}

	siStartInfo.dwFlags |= CREATE_NEW_CONSOLE;

	// Create the child process.

	//glue together all the nice command line arguments
	std::string args(it->program);
	std::vector<std::string>::iterator cur, end;
	cur = it->args.begin();
	end = it->args.end();
	for(; cur != end; ++cur) {
		args += " ";
		args += *cur;
	}

	bFuncRetn = CreateProcess(it->program.c_str(),
		const_cast<char *>(args.c_str()), // command line
		nullptr, // process security attributes
		nullptr, // primary thread security attributes
		inherit_handles, // handles are not inherited
		0, // creation flags (CREATE_NEW_PROCESS_GROUP maybe)
		nullptr, // use parent's environment
		nullptr, // use parent's current directory
		&siStartInfo, // STARTUPINFO pointer
		&it->proc_info); // receives PROCESS_INFORMATION

	if (bFuncRetn == 0) {
		safe_delete(it);
		//GetLastError()
		return(ProcError);
	}


	//keep process handle open to get exit code
	CloseHandle(it->proc_info.hThread);	//we dont need their thread handle
	if(logOut != nullptr)
		CloseHandle(logOut);	//we dont want their output handle either.

	ProcRef res = it->proc_info.dwProcessId;

	//record this entry..
	m_running[res] = it;

	return(res);

#else	//!WIN32

	//build argv
	auto argv = new char *[it->args.size() + 2];
	unsigned int r;
	argv[0] = const_cast<char *>(it->program.c_str());
	for(r = 1; r <= it->args.size(); r++) {
		argv[r] = const_cast<char *>(it->args[r-1].c_str());
	}
	argv[r] = nullptr;

	ProcRef res = fork();		//cant use vfork since we are opening the log file.
	if(res == -1) {
		//error forking... errno
		safe_delete(it);
		safe_delete_array(argv);
		return(ProcError);
	}

	if(res == 0) {
		//child... exec this bitch

		//handle output redirection if requested.
		if(it->logFile.length() > 0) {
			//we will put their output directly into a file.
			int outfd = creat(it->logFile.c_str(), S_IRUSR | S_IWUSR | S_IRGRP); // S_I + R/W/X + USR/GRP/OTH
			if(outfd == -1) {
				fprintf(stderr, "Unable to open log file %s: %s.\n", it->logFile.c_str(), strerror(errno));
				close(STDOUT_FILENO);
				close(STDERR_FILENO);
				close(STDIN_FILENO);
			} else {
				close(STDOUT_FILENO);
				if(dup2(outfd, STDOUT_FILENO) == -1) {
					fprintf(stderr, "Unable to duplicate FD %d to %d. Log file will be empty: %s\n", outfd, STDOUT_FILENO, strerror(errno));
					const char *err = "Unable to redirect stdout into this file. That sucks.";
					write(outfd, err, strlen(err));
				}
				close(STDERR_FILENO);
				if(dup2(outfd, STDERR_FILENO) == -1) {
					//can no longer print to screen..
					const char *err = "Unable to redirect stderr into this file. You might miss some error info in this log.";
					write(outfd, err, strlen(err));
				}
				close(STDIN_FILENO);

				close(outfd);	//dont need this one, we have two more copies...
			}
		}

		//call it...
		execv(argv[0], argv);
		_exit(1);
	}
	safe_delete_array(argv);

	//record this entry..
	m_running[res] = it;

	return(res);
#endif	//!WIN32
}


//if graceful is true, we try to be nice about it if possible
bool ProcLauncher::Terminate(const ProcRef &proc, bool graceful) {
	//we are only willing to kill things we started...
	auto res = m_running.find(proc);
	if(res == m_running.end())
		return(false);

	//we do not remove it from the list until we have been notified
	//that they have been terminated.

#ifdef _WINDOWS
	if(!TerminateProcess(res->second->proc_info.hProcess, 0)) {
		return(false);
	}
#else	//!WIN32
	int sig;
	if(graceful)
		sig = SIGTERM;
	else
		sig = SIGKILL;
	if(kill(proc, sig) == -1) {
		return(false);
	}
#endif	//!WIN32
	return(true);
}

void ProcLauncher::TerminateAll(bool final) {
	if(!final) {
		//send a nice terminate to each process, with intention of waiting for them
		std::map<ProcRef, Spec *>::iterator cur, end;
		cur = m_running.begin();
		end = m_running.end();
		for(; cur != end; ++cur) {
			Terminate(cur->first, true);
		}
	} else {
		//kill each process and remove it from the list
		std::map<ProcRef, Spec *> running(m_running);
		m_running.clear();

		std::map<ProcRef, Spec *>::iterator cur, end;
		cur = running.begin();
		end = running.end();
		for(; cur != end; ++cur) {
			Terminate(cur->first, true);
			safe_delete(cur->second);
		}
	}
}


#ifndef WIN32
void ProcLauncher::HandleSigChild(int signum) {
	if(signum == SIGCHLD) {
		ProcLauncher::get()->m_signalCount++;
	}
}
#endif



ProcLauncher::Spec::Spec() {
	handler = nullptr;
}

ProcLauncher::Spec::Spec(const Spec &other) {
	program = other.program;
	args = other.args;
	handler = other.handler;
	logFile = other.logFile;
}

ProcLauncher::Spec &ProcLauncher::Spec::operator=(const Spec &other) {
	program = other.program;
	args = other.args;
	handler = other.handler;
	logFile = other.logFile;
	return(*this);
}
