/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2013 EQEMu Development Team (http://eqemulator.net)

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

#include "ipc_mutex.h"

#include "common/eqemu_config.h"
#include "common/eqemu_exception.h"
#include "common/path_manager.h"
#include "common/types.h"

#ifdef _WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace EQ {
	struct IPCMutex::Implementation {
#ifdef _WINDOWS
		HANDLE mut_;
#else
		int fd_;
#endif
	};

	IPCMutex::IPCMutex(const std::string& name) : locked_(false) {
		imp_ = new Implementation;
#ifdef _WINDOWS
		auto Config = EQEmuConfig::get();
		std::string final_name = fmt::format("{}/EQEmuMutex_{}", Config->SharedMemDir, name);

		imp_->mut_ = CreateMutex(nullptr,
			FALSE,
			final_name.c_str());

		if(!imp_->mut_) {
			EQ_EXCEPT("IPC Mutex", "Could not create mutex.");
		}
#else
		std::string final_name = fmt::format("{}/{}.lock", PathManager::Instance()->GetSharedMemoryPath(), name);

#ifdef __DARWIN
#if __DARWIN_C_LEVEL < 200809L
		imp_->fd_ = open(final_name.c_str(),
			O_RDWR | O_CREAT,
			S_IRUSR | S_IWUSR);
#else
		imp_->fd_ = open(final_name.c_str(),
			O_RDWR | O_CREAT | O_CLOEXEC,
			S_IRUSR | S_IWUSR);
#endif
#else
		imp_->fd_ = open(final_name.c_str(),
			O_RDWR | O_CREAT | O_CLOEXEC,
			S_IRUSR | S_IWUSR);
#endif

		if(imp_->fd_ == -1) {
				EQ_EXCEPT("IPC Mutex", "Could not create mutex.");
		}
#endif
	}

	IPCMutex::~IPCMutex() {
#ifdef _WINDOWS
		if(locked_) {
			ReleaseMutex(imp_->mut_);
		}
		CloseHandle(imp_->mut_);

#else
		if(locked_) {
			lockf(imp_->fd_, F_ULOCK, 0);
		}
		close(imp_->fd_);

#endif
		delete imp_;
	}

	bool IPCMutex::Lock() {
		if(locked_) {
			return false;
		}

#ifdef _WINDOWS
		DWORD wait_result = WaitForSingleObject(imp_->mut_, INFINITE);
		if(wait_result != WAIT_OBJECT_0) {
			return false;
		}
#else
		if(lockf(imp_->fd_, F_LOCK, 0) != 0) {
			return false;
		}
#endif
		locked_ = true;
		return true;
	}

	bool IPCMutex::Unlock() {
		if(!locked_) {
			return false;
		}
#ifdef _WINDOWS
		if(!ReleaseMutex(imp_->mut_)) {
			return false;
		}
#else
		if(lockf(imp_->fd_, F_ULOCK, 0) != 0) {
			return false;
		}
#endif
		locked_ = false;
		return true;
	}
}
