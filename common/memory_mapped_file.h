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

#pragma once

#include "common/types.h"

#include <string>

namespace EQ {

	//! Memory Backed Shared Memory
	/*!
		Allows us to create shared memory that is backed by a file on both windows and unix platforms that
		works in a consistent manner. Non-copyable.
	*/
	class MemoryMappedFile {
		struct Implementation;
		struct shared_memory_struct;

		//! Underlying data structure.
		struct shared_memory_struct {
			uint32 size;
			unsigned char data[1];
		};
	public:
		//! Constructor
		/*!
			Creates a mmf for the given filename and of size.
		\param filename Actual filename of the mmf.
		\param size Size in bytes of the mmf.
		*/
		MemoryMappedFile(std::string filename, uint32 size);

		//! Constructor
		/*!
			Creates a mmf for the given filename and gets the size based on the existing size.
		\param filename Actual filename of the mmf.
		*/
		MemoryMappedFile(std::string filename);

		//! Destructor
		~MemoryMappedFile();

		//! Get Data Operator
		inline void *operator->() const { return memory_->data; }

		//! Get Data Function
		inline void *Get() const { return memory_->data; }

		//! Get Size Function
		inline uint32 Size() const { return memory_->size; }

		//! Zeros all the memory in the file, and set it to be unloaded
		void ZeroFile();
	private:
		//! Copy Constructor
		MemoryMappedFile(const MemoryMappedFile&);

		//! Assignment Operator
		const MemoryMappedFile& operator=(const MemoryMappedFile&);

		std::string filename_; //!< Filename of this shared memory object
		uint32 size_; //!< Size in bytes of this shared memory object
		shared_memory_struct *memory_; //!< Underlying data of the shared memory object.

		Implementation *imp_; //!< Underlying implementation.
	};
} // EQEmu
