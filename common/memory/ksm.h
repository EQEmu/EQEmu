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

#include "common/eqemu_logsys.h"

class PageAlignedAllocatorBase
{
protected:
	void* allocateInternal(size_t amount, size_t alignment);
	size_t getPageSize() const;
};

// Page-aligned allocator for std::vector
template <typename T>
class PageAlignedAllocator : public PageAlignedAllocatorBase
{
public:
	using value_type = T;

	PageAlignedAllocator() noexcept = default;

	template <typename U>
	PageAlignedAllocator(const PageAlignedAllocator<U>&) noexcept {}

	T* allocate(std::size_t n)
	{
		size_t size = n * sizeof(T);

		return static_cast<T*>(allocateInternal(size, getPageSize()));
	}

	void deallocate(T* p, std::size_t) noexcept
	{
		free(p);
	}
};

template <typename T, typename U>
bool operator==(const PageAlignedAllocator<T>&, const PageAlignedAllocator<U>&) noexcept {
	return true;
}

template <typename T, typename U>
bool operator!=(const PageAlignedAllocator<T>&, const PageAlignedAllocator<U>&) noexcept {
	return false;
}

// Kernel Samepage Merging (KSM) functionality
namespace KSM {

void CheckPageAlignment(void* ptr);
void* AllocatePageAligned(size_t size);
void MarkMemoryForKSM(void* start, size_t size);
void AlignHeapToPageBoundary();
void* MarkHeapStart();
size_t MeasureHeapUsage(void* start);

} // namespace KSM
