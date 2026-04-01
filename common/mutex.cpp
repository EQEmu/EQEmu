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
#include "mutex.h"

#include <iostream>

#define DEBUG_MUTEX_CLASS 0
#if DEBUG_MUTEX_CLASS >= 1
#endif

#ifdef _WINDOWS
bool IsTryLockSupported();
bool TrylockSupported = IsTryLockSupported();

bool IsTryLockSupported() {
	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
	{
		// If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) {
#if DEBUG_MUTEX_CLASS >= 1
			std::cout << "Mutex::trylock() NOT supported" << std::endl;
#endif
			return false;
		}
	}

	// Tests for Windows NT product family.
	if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.dwMajorVersion >= 4) {
#if DEBUG_MUTEX_CLASS >= 1
		std::cout << "Mutex::trylock() SUPPORTED" << std::endl;
#endif
		return true;
	}
	else {
#if DEBUG_MUTEX_CLASS >= 1
		std::cout << "Mutex::trylock() NOT supported" << std::endl;
#endif
		return false;
	}
}
#endif

Mutex::Mutex() {

#if DEBUG_MUTEX_CLASS >= 7
	std::cout << "Constructing Mutex" << std::endl;
#endif
#ifdef _WINDOWS
    InitializeCriticalSection(&CSMutex);
#else
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
#if defined(__CYGWIN__) || defined(__APPLE__) || defined(FREEBSD)
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#else
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
#endif    
    pthread_mutex_init(&CSMutex, &attr);
    pthread_mutexattr_destroy(&attr);
#endif
}

Mutex::~Mutex() {
#ifdef _WINDOWS
	DeleteCriticalSection(&CSMutex);
#else
#endif
}

void Mutex::lock() {
#if DEBUG_MUTEX_CLASS >= 5
	if (!trylock()) {
		std::cout << "Locking Mutex: Having to wait" << std::endl;
		#ifdef _WINDOWS
			EnterCriticalSection(&CSMutex);
		#else
			pthread_mutex_lock(&CSMutex);
		#endif
	}
#else
	#ifdef _WINDOWS
		EnterCriticalSection(&CSMutex);
	#else
		pthread_mutex_lock(&CSMutex);
	#endif
#endif
}

bool Mutex::trylock() {
#ifdef _WINDOWS
	#if(_WIN32_WINNT >= 0x0400)
		if (TrylockSupported)
			return TryEnterCriticalSection(&CSMutex);
		else {
			EnterCriticalSection(&CSMutex);
			return true;
		}
	#else
		EnterCriticalSection(&CSMutex);
		return true;
	#endif
#else
	return (pthread_mutex_trylock(&CSMutex) == 0);
#endif
}

void Mutex::unlock() {
#ifdef _WINDOWS
	LeaveCriticalSection(&CSMutex);
#else
	pthread_mutex_unlock(&CSMutex);
#endif
}


LockMutex::LockMutex(Mutex* in_mut, bool iLock) {
	mut = in_mut;
	locked = iLock;
	if (locked) {
		mut->lock();
	}
}

LockMutex::~LockMutex() {
	if (locked) {
		mut->unlock();
	}
}

void LockMutex::unlock() {
	if (locked)
		mut->unlock();
	locked = false;
}

void LockMutex::lock() {
	if (!locked)
		mut->lock();
	locked = true;
}
