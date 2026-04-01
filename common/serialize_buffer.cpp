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
#include "serialize_buffer.h"

void SerializeBuffer::Grow(size_t new_size)
{
	assert(new_size > m_capacity);
	auto new_buffer = new unsigned char[new_size * 2];
	memset(new_buffer, 0, new_size * 2);

	if (m_buffer)
		memcpy(new_buffer, m_buffer, m_capacity);
	m_capacity = new_size * 2;
	delete[] m_buffer;
	m_buffer = new_buffer;
}

void SerializeBuffer::Reset()
{
	delete[] m_buffer;
	m_buffer = nullptr;
	m_capacity = 0;
	m_pos = 0;
}

