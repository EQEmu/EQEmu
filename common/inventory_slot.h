/*	EQEMu: Everquest Server Emulator
	
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#pragma once

#include "common/emu_constants.h"


namespace EQ
{
	class InventorySlot;

	namespace inventory {
		int8 ConvertEquipmentIndexToTextureIndex(int16 slot_index);
		int8 ConvertEquipmentSlotToTextureIndex(const InventorySlot& inventory_slot);
		int16 ConvertTextureIndexToEquipmentIndex(int8 texture_index);
	}
	
	class InventorySlot {
	public:
		InventorySlot() : _type_index(invtype::TYPE_INVALID), _slot_index(invslot::SLOT_INVALID), _container_index(invbag::SLOT_INVALID), _socket_index(invaug::SOCKET_INVALID), _typeless(false) { }
		InventorySlot(int16 type_index) : _type_index(type_index), _slot_index(invslot::SLOT_INVALID), _container_index(invbag::SLOT_INVALID), _socket_index(invaug::SOCKET_INVALID), _typeless(false) { }
		InventorySlot(int16 type_index, int16 parent_index) : _type_index(type_index), _slot_index(parent_index), _container_index(invbag::SLOT_INVALID), _socket_index(invaug::SOCKET_INVALID), _typeless(false) { }
		InventorySlot(int16 type_index, int16 parent_index, int16 bag_index) : _type_index(type_index), _slot_index(parent_index), _container_index(bag_index), _socket_index(invaug::SOCKET_INVALID), _typeless(false) { }
		InventorySlot(int16 type_index, int16 parent_index, int16 bag_index, int16 aug_index) : _type_index(type_index), _slot_index(parent_index), _container_index(bag_index), _socket_index(aug_index), _typeless(false) { }
		InventorySlot(const InventorySlot& r) : _type_index(r._type_index), _slot_index(r._slot_index), _container_index(r._container_index), _socket_index(r._socket_index), _typeless(r._typeless) { }
		InventorySlot(int16 type_index, const InventorySlot& r) : _type_index(type_index), _slot_index(r._slot_index), _container_index(r._container_index), _socket_index(r._socket_index), _typeless(false) { }

		inline int16 TypeIndex() const { return _type_index; }
		inline int16 SlotIndex() const { return _slot_index; }
		inline int16 ContainerIndex() const { return _container_index; }
		inline int16 SocketIndex() const { return _socket_index; }

		bool Typeless() const { return _typeless; }

		bool IsValidSlot() const;
		bool IsDeleteSlot() const;

		static bool IsEquipmentIndex(int16 slot_index);
		static bool IsGeneralIndex(int16 slot_index);
		static bool IsCursorIndex(int16 slot_index);
		static bool IsWeaponIndex(int16 slot_index);
		static bool IsTextureIndex(int16 slot_index);
		static bool IsTintableIndex(int16 slot_index);

		bool IsEquipmentSlot() const;
		bool IsGeneralSlot() const;
		bool IsCursorSlot() const;
		bool IsWeaponSlot() const;
		bool IsTextureSlot() const;
		bool IsTintableSlot() const;

		bool IsSlot() const;
		bool IsSlotSocket() const;
		bool IsContainer() const;
		bool IsContainerSocket() const;

		InventorySlot ToTopOwner() const;
		InventorySlot ToOwner() const;

		const std::string ToString() const;
		const std::string ToName() const;

		bool IsTypeIndex(int16 type_index) const { return (_type_index == type_index); }
		bool IsSlotIndex(int16 slot_index) const { return (_slot_index == slot_index); }
		bool IsContainerIndex(int16 container_index) const { return (_container_index == container_index); }
		bool IsSocketIndex(int16 socket_index) const { return (_socket_index == socket_index); }

		void SetType(int16 type_index) { _type_index = type_index; }
		void SetSlot(int16 slot_index) { _slot_index = slot_index; }
		void SetContainer(int16 container_index) { _container_index = container_index; }
		void SetSocket(int16 socket_index) { _socket_index = socket_index; }

		void SetInvalidSlot();

		void SetTypeInvalid() { _type_index = invtype::TYPE_INVALID; }
		void SetSlotInvalid() { _slot_index = invslot::SLOT_INVALID; }
		void SetContainerInvalid() { _container_index = invbag::SLOT_INVALID; }
		void SetSocketInvalid() { _socket_index = invaug::SOCKET_INVALID; }

		void SetTypeBegin() { _type_index = invtype::TYPE_BEGIN; }
		void SetSlotBegin() { _slot_index = invslot::SLOT_BEGIN; }
		void SetContainerBegin() { _container_index = invbag::SLOT_BEGIN; }
		void SetSocketBegin() { _socket_index = invaug::SOCKET_BEGIN; }

		void IncrementType() { ++_type_index; }
		void IncrementSlot() { ++_slot_index; }
		void IncrementContainer() { ++_container_index; }
		void IncrementSocket() { ++_socket_index; }

		void SetTypeless() { _typeless = true; }
		void ClearTypeless() { _typeless = false; }

		// these two methods should really check for all bonus-valid slots..currently checks for equipment only (rework needed)
		//static bool IsBonusIndex(int16 slot_index);
		//bool IsBonusSlot() const;

		bool operator<(const InventorySlot& rhs) const;

	private:
		int16 _type_index;
		//int16 _unknown2; // not implemented
		int16 _slot_index;
		int16 _container_index;
		int16 _socket_index;
		//int16 _unknown1; // not implemented

		bool _typeless;
	};

	bool operator==(const InventorySlot& lhs, const InventorySlot& rhs);
	bool operator!=(const InventorySlot& lhs, const InventorySlot& rhs) { return (!(lhs == rhs)); }

} /*EQEmu*/
