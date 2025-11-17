#include "uuid.h"

#include <ios>
#include <fmt/format.h>
#include <fmt/ranges.h>

#ifdef _WIN32
#include <objbase.h>
#endif

#ifdef __linux__
#include <uuid/uuid.h>
#endif

#ifdef __APPLE__
#include <CoreFoundation/CFUUID.h>
#endif

#ifdef __FreeBSD__
#include <uuid.h>
#endif

unsigned char hexDigitToChar(char ch)
{
	if (ch > 47 && ch < 58)
		return ch - 48;

	if (ch > 96 && ch < 103)
		return ch - 87;

	if (ch > 64 && ch < 71)
		return ch - 55;

	return 0;
}

unsigned char hexPairToChar(char a, char b)
{
	return hexDigitToChar(a) * 16 + hexDigitToChar(b);
}

EQ::Util::UUID::UUID()
{
	m_bytes = std::vector<char>(16, 0);
}

EQ::Util::UUID::UUID(const unsigned char *bytes)
{
	m_bytes.assign(bytes, bytes + 16);
}

EQ::Util::UUID::UUID(const UUID &o)
{
	m_bytes = o.m_bytes;
}

EQ::Util::UUID::UUID(UUID &&o)
{
	std::swap(m_bytes, o.m_bytes);
}

EQ::Util::UUID& EQ::Util::UUID::operator=(const UUID &o)
{
	m_bytes = o.m_bytes;
	return *this;
}

EQ::Util::UUID::~UUID()
{
}

EQ::Util::UUID EQ::Util::UUID::Generate()
{
#ifdef _WIN32
	GUID id;
	CoCreateGuid(&id);

	const unsigned char buffer[16] =
	{
		static_cast<unsigned char>((id.Data1 >> 24) & 0xFF),
		static_cast<unsigned char>((id.Data1 >> 16) & 0xFF),
		static_cast<unsigned char>((id.Data1 >> 8) & 0xFF),
		static_cast<unsigned char>((id.Data1) & 0xff),
		static_cast<unsigned char>((id.Data2 >> 8) & 0xFF),
		static_cast<unsigned char>((id.Data2) & 0xff),
		static_cast<unsigned char>((id.Data3 >> 8) & 0xFF),
		static_cast<unsigned char>((id.Data3) & 0xFF),
		id.Data4[0],
		id.Data4[1],
		id.Data4[2],
		id.Data4[3],
		id.Data4[4],
		id.Data4[5],
		id.Data4[6],
		id.Data4[7]
	};

	return buffer;
#endif

#ifdef __linux__
	uuid_t id;
	uuid_generate(id);
	return id;
#endif

#ifdef __APPLE__
	auto id = CFUUIDCreate(nullptr);
	auto bytes = CFUUIDGetUUIDBytes(id);

	const unsigned char buffer[16] =
	{
		bytes.byte0,
		bytes.byte1,
		bytes.byte2,
		bytes.byte3,
		bytes.byte4,
		bytes.byte5,
		bytes.byte6,
		bytes.byte7,
		bytes.byte8,
		bytes.byte9,
		bytes.byte10,
		bytes.byte11,
		bytes.byte12,
		bytes.byte13,
		bytes.byte14,
		bytes.byte15
	};

	CFRelease(id);

	return buffer;
#endif

#ifdef __FreeBSD__
	uuid_t l_id;
	char l_uuid[37];
	uint32_t l_ignored;
	uuid_create(&l_id, &l_ignored);
	uuid_to_string(&l_id, (char**) &l_uuid, &l_ignored);
	return FromString(l_uuid);
#endif
}

EQ::Util::UUID EQ::Util::UUID::FromString(const std::string &str)
{
	UUID ret;
	ret.m_bytes.clear();

	size_t i = 0;
	ret.m_bytes.push_back(hexPairToChar(str[i], str[i + 1])); ++i;
	ret.m_bytes.push_back(hexPairToChar(str[i], str[i + 1])); ++i;
	ret.m_bytes.push_back(hexPairToChar(str[i], str[i + 1])); ++i;
	ret.m_bytes.push_back(hexPairToChar(str[i], str[i + 1])); ++i;
	++i;
	ret.m_bytes.push_back(hexPairToChar(str[i], str[i + 1])); ++i;
	ret.m_bytes.push_back(hexPairToChar(str[i], str[i + 1])); ++i;
	++i;
	ret.m_bytes.push_back(hexPairToChar(str[i], str[i + 1])); ++i;
	ret.m_bytes.push_back(hexPairToChar(str[i], str[i + 1])); ++i;
	++i;
	ret.m_bytes.push_back(hexPairToChar(str[i], str[i + 1])); ++i;
	ret.m_bytes.push_back(hexPairToChar(str[i], str[i + 1])); ++i;
	++i;
	ret.m_bytes.push_back(hexPairToChar(str[i], str[i + 1])); ++i;
	ret.m_bytes.push_back(hexPairToChar(str[i], str[i + 1])); ++i;
	ret.m_bytes.push_back(hexPairToChar(str[i], str[i + 1])); ++i;
	ret.m_bytes.push_back(hexPairToChar(str[i], str[i + 1])); ++i;
	ret.m_bytes.push_back(hexPairToChar(str[i], str[i + 1])); ++i;
	ret.m_bytes.push_back(hexPairToChar(str[i], str[i + 1])); ++i;
	return ret;
}

EQ::Util::UUID EQ::Util::UUID::FromByteArray(const char *buffer)
{
	return UUID((unsigned char*)buffer);
}

std::string EQ::Util::UUID::ToString() const
{
	return fmt::format("{:0<2x}{:0<2x}{:0<2x}{:0<2x}-{:0<2x}{:0<2x}-{:0<2x}{:0<2x}-{:0<2x}{:0<2x}-{:0<2x}{:0<2x}{:0<2x}{:0<2x}{:0<2x}{:0<2x}",
		(unsigned char)m_bytes[0],
		(unsigned char)m_bytes[1],
		(unsigned char)m_bytes[2],
		(unsigned char)m_bytes[3],
		(unsigned char)m_bytes[4],
		(unsigned char)m_bytes[5],
		(unsigned char)m_bytes[6],
		(unsigned char)m_bytes[7],
		(unsigned char)m_bytes[8],
		(unsigned char)m_bytes[9],
		(unsigned char)m_bytes[10],
		(unsigned char)m_bytes[11],
		(unsigned char)m_bytes[12],
		(unsigned char)m_bytes[13],
		(unsigned char)m_bytes[14],
		(unsigned char)m_bytes[15]);
}

const std::vector<char>& EQ::Util::UUID::ToByteArray() const
{
	return m_bytes;
}
