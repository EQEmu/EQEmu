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
#include "ksm.h"


#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>  // For madvise
#include <unistd.h>    // For sysconf, sbrk
#endif

#ifdef _WIN32
// Windows-specific functionality

void* PageAlignedAllocatorBase::allocateInternal(size_t size, size_t alignment)
{
	void* ptr = malloc(size);

	if (!ptr)
	{
		throw std::bad_alloc();
	}

	return ptr;
}

size_t PageAlignedAllocatorBase::getPageSize() const
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	return sysInfo.dwPageSize; // Page size in bytes
}

namespace KSM {

// Windows-specific placeholder functions (no-op)
void CheckPageAlignment(void* ptr)
{
}

void* AllocatePageAligned(size_t size)
{
	return memset(malloc(size), 0, size);
}

void MarkMemoryForKSM(void* start, size_t size)
{
}

void AlignHeapToPageBoundary()
{
}

void* MarkHeapStart()
{
	return nullptr;
}

size_t MeasureHeapUsage(void* start)
{
	return 0;
}

} // namespace KSM

#else

// Linux-specific functionality

void* PageAlignedAllocatorBase::allocateInternal(size_t size, size_t alignment)
{
	void* ptr = nullptr;

	if (posix_memalign(&ptr, alignment, size) != 0)
	{
		throw std::bad_alloc();
	}

	return ptr;
}

size_t PageAlignedAllocatorBase::getPageSize() const
{
	return static_cast<size_t>(sysconf(_SC_PAGESIZE));
}

namespace KSM {

void CheckPageAlignment(void* ptr)
{
	size_t page_size = sysconf(_SC_PAGESIZE);

	if (reinterpret_cast<uintptr_t>(ptr) % page_size == 0)
	{
		LogKSMDetail("Memory is page-aligned [{}]", ptr);
	}
	else
	{
		LogKSMDetail("Memory is NOT page-aligned [{}]", ptr);
	}
}

void* AllocatePageAligned(size_t size)
{
	size_t page_size = sysconf(_SC_PAGESIZE);
	void* aligned_ptr = nullptr;

	if (posix_memalign(&aligned_ptr, page_size, size) != 0)
	{
		LogKSM("Failed to allocate page-aligned memory on Linux. page_size [{}] size [{}] bytes", page_size, size);
	}

	std::memset(aligned_ptr, 0, size);
	return aligned_ptr;
}

void MarkMemoryForKSM(void* start, size_t size)
{
	if (madvise(start, size, MADV_MERGEABLE) == 0)
	{
		LogKSM("Marked memory for KSM | start [{}] size [{}] bytes", start, size);
	}
	else
	{
		perror("madvise failed");
	}
}

void AlignHeapToPageBoundary()
{
	size_t page_size = sysconf(_SC_PAGESIZE);
	if (page_size == 0)
	{
		LogKSM("Failed to retrieve page size SC_PAGESIZE [{}]", page_size);
		return;
	}

	void* current_break = sbrk(0);
	if (current_break == (void*)-1)
	{
		LogKSM("Failed to retrieve the current program break");
		return;
	}

	uintptr_t current_address = reinterpret_cast<uintptr_t>(current_break);
	size_t misalignment = current_address % page_size;

	if (misalignment != 0)
	{
		size_t adjustment = page_size - misalignment;
		if (sbrk(adjustment) == (void*)-1)
		{
			LogKSM("Failed to align heap to page boundary. adjustment [{}] bytes", adjustment);
			return;
		}
	}

	LogKSMDetail("Heap aligned to next page boundary. Current break [{}]", sbrk(0));
}

void* MarkHeapStart()
{
	void* current_pos = sbrk(0);
	AlignHeapToPageBoundary();
	return current_pos;
}

size_t MeasureHeapUsage(void* start)
{
	void* current_break = sbrk(0);
	return static_cast<char*>(current_break) - static_cast<char*>(start);
}

} // namespace KSM

#endif
