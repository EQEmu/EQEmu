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

#include "common/eqemu_exception.h"
#include "common/types.h"

#include <string.h>

namespace EQ {

	/*! Simple HashSet designed to be used in fixed memory that may be difficult to use an
	allocator for (shared memory), we assume all keys are unsigned int, values are a pointer and size
	*/
	template<class T>
	class FixedMemoryVariableHashSet {
		typedef uint32 key_type;
		typedef T value_type;
		typedef uint8 byte;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef uint32 size_type;
	public:
		/*!
			Constructor which initializes the hash set
		\param data Raw data
		\param size Raw data size
		\param max_element_id Number of offsets to store: eg highest "key" that will be used.
		*/
		FixedMemoryVariableHashSet(byte *data, size_type size, key_type max_element_id) {
			data_ = data;
			size_ = size;
			remaining_size_ = size_ - (sizeof(key_type) * 3) - (sizeof(key_type) * (max_element_id + 1));

			byte *ptr = data;
			*reinterpret_cast<key_type*>(ptr) = max_element_id + 1;
			offset_count_ = max_element_id + 1;
			ptr += sizeof(key_type);

			*reinterpret_cast<key_type*>(ptr) = remaining_size_;
			ptr += sizeof(key_type);

			*reinterpret_cast<key_type*>(ptr) = 0;
			current_offset_ = 0;
			ptr += sizeof(key_type);


			offsets_ = reinterpret_cast<key_type*>(ptr);
			memset(ptr, 0xFFFFFFFFU, sizeof(key_type) * (max_element_id + 1));
			ptr += sizeof(key_type) * (max_element_id + 1);

			elements_ = reinterpret_cast<byte*>(ptr);
		}

		/*!
			Constructor which does not initialize the hash set. Builds the hash set from what data is
			stored in the data pointer passed.
		\param data Raw data
		\param size Raw data size
		*/
		FixedMemoryVariableHashSet(byte *data, size_type size) {
			data_ = data;
			size_ = size;

			byte *ptr = data;
			offset_count_ = *reinterpret_cast<key_type*>(ptr);
			ptr += sizeof(key_type);

			remaining_size_ = *reinterpret_cast<key_type*>(ptr);
			ptr += sizeof(key_type);

			current_offset_ = *reinterpret_cast<key_type*>(ptr);
			ptr += sizeof(key_type);

			offsets_ = reinterpret_cast<key_type*>(ptr);
			ptr += sizeof(key_type) * offset_count_;

			elements_ = reinterpret_cast<byte*>(ptr);
		}

		//! Copy Constructor
		FixedMemoryVariableHashSet(const FixedMemoryVariableHashSet& other) :
			data_(other.data_),
			size_(other.size_),
			offset_count_(other.offset_count_),
			remaining_size_(other.remaining_size_),
			current_offset_(other.current_offset_),
			offsets_(other.offsets_),
			elements_(other.elements_)
		{
		}

		//! RValue-Move Constructor
#ifdef EQEMU_RVALUE_MOVE
		FixedMemoryVariableHashSet(FixedMemoryVariableHashSet&& other) :
			data_(other.data_),
			size_(other.size_),
			offset_count_(other.offset_count_),
			remaining_size_(other.remaining_size_),
			current_offset_(other.current_offset_),
			offsets_(other.offsets_),
			elements_(other.elements_)
		{
		}
#endif

		//! Destructor
		~FixedMemoryVariableHashSet() {
		}

		//! Assignment operator
		const FixedMemoryVariableHashSet& operator=(const FixedMemoryVariableHashSet& other) {
			data_ = other.data_;
			size_ = other.size_;
			offset_count_ = other.offset_count_;
			remaining_size_ = other.remaining_size_;
			current_offset_ = other.current_offset_;
			offsets_ = other.offsets_;
			elements_ = other.elements_;
			return *this;
		}

		//! Returns the number of bytes in the set currently
		size_type size() const {
			return size_ - remaining_size_;
		}

		//! Returns the maximum number of bytes one can insert into the set.
		size_type max_size() const {
			return size_ - (sizeof(key_type) * 2);
		}

		//! Returns the maximum key one can use with the set.
		key_type max_key() const {
			return offset_count_ > 0 ? (offset_count_ - 1) : 0;
		}

		/*!
			Retrieve value operator
		\param i Index to retrieve the value from
		*/
		reference operator[](const key_type& i) {
			if(i >= offset_count_) {
				EQ_EXCEPT("Fixed Memory Variable Hash Set", "Index out of range");
			}

			if(offsets_[i] == 0xFFFFFFFFU) {
				EQ_EXCEPT("Fixed Memory Variable Hash Set", "Element not found.");
			}

			return *reinterpret_cast<value_type*>(&elements_[offsets_[i]]);
		}

		/*!
			Retrieve value function
		\param i Index to retrieve the value from
		*/
		reference at(const key_type& i) {
			if(i >= offset_count_) {
				EQ_EXCEPT("Fixed Memory Variable Hash Set", "Index out of range");
			}

			if(offsets_[i] == 0xFFFFFFFFU) {
				EQ_EXCEPT("Fixed Memory Variable Hash Set", "Element not found.");
			}

			return *reinterpret_cast<value_type*>(&elements_[offsets_[i]]);
		}

		/*!
			Checks if there is a value at a certain index
		\param i Index to check for a value
		*/
		bool exists(const key_type& i) const {
			if(i >= offset_count_) {
				return false;
			}

			if(offsets_[i] == 0xFFFFFFFFU) {
				return false;
			}

			return true;
		}

		/*!
			Inserts a value into the set at a specific index
		\param i Index to insert the value at
		\param v Value to insert
		*/
		void insert(const key_type& i, byte *data, size_type size) {
			if(i >= offset_count_) {
				EQ_EXCEPT("Fixed Memory Variable Hash Set", "Index out of range.");
			}

			if(size > remaining_size_) {
				EQ_EXCEPT("Fixed Memory Hash Set", "Not enough room in hash set to insert this value.");
			}

			if(offsets_[i] != 0xFFFFFFFFU) {
				EQ_EXCEPT("Fixed Memory Hash Set", "Could not insert a repeat value at this index.");
			} else {
				offsets_[i] = current_offset_;
				memcpy(&elements_[current_offset_], data, size);

				remaining_size_ -= size;
				*reinterpret_cast<key_type*>(data_ + sizeof(key_type)) = remaining_size_;

				current_offset_ += size;
				*reinterpret_cast<key_type*>(data_ + (sizeof(key_type) * 2)) = current_offset_;
			}
		}

	private:
		unsigned char *data_;
		size_type size_;
		size_type remaining_size_;
		key_type current_offset_;
		key_type offset_count_;
		key_type *offsets_;
		byte *elements_;
	};
} // EQEmu
