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

#include <ostream>
#include <string>
#include <vector>

namespace EQ
{
	namespace Util
	{
		class UUID
		{
		public:
			UUID(const UUID &o);
			UUID(UUID &&o);
			UUID& operator=(const UUID &o);
			~UUID();

			static UUID Generate();
			static UUID FromString(const std::string &str);
			static UUID FromByteArray(const char *buffer);

			std::string ToString() const;
			const std::vector<char>& ToByteArray() const;

			friend std::ostream &operator<<(std::ostream &os, const UUID &id) {
				return os << id.ToString();
			}
		private:
			UUID();
			UUID(const unsigned char *bytes);
			std::vector<char> m_bytes;
		};
	}
}
