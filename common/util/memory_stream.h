#pragma once

#include <istream>
#include <ostream>
#include <streambuf>

namespace EQ
{
	namespace Util {
		class MemoryBuffer : public std::streambuf
		{
		public:
			MemoryBuffer(char* base, size_t size) {
				setg(base, base, base + size);
				setp(base, base + size);
			}
		};

		class MemoryStreamWriter : virtual MemoryBuffer, public std::ostream
		{
		public:
			MemoryStreamWriter(char* base, size_t size) : MemoryBuffer(base, size), std::ostream(static_cast<std::streambuf*>(this)) { }
		};

		class MemoryStreamReader : virtual MemoryBuffer, public std::istream
		{
		public:
			MemoryStreamReader(char* base, size_t size) : MemoryBuffer(base, size), std::istream(static_cast<std::streambuf*>(this)) { }
		};
	}
}