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
