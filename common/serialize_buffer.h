
#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <utility>

class SerializeBuffer
{
public:
	SerializeBuffer() : m_buffer(nullptr), m_capacity(0), m_pos(0) {}

	explicit SerializeBuffer(size_t size) : m_capacity(size), m_pos(0)
	{
		m_buffer = new unsigned char[size];
		memset(m_buffer, 0, size);
	}

	SerializeBuffer(const SerializeBuffer &rhs)
	    : m_buffer(new unsigned char[rhs.m_capacity]), m_capacity(rhs.m_capacity), m_pos(rhs.m_pos)
	{
		memcpy(m_buffer, rhs.m_buffer, rhs.m_capacity);
	}

	SerializeBuffer &operator=(const SerializeBuffer &rhs)
	{
		if (this != &rhs) {
			delete[] m_buffer;
			m_buffer = new unsigned char[rhs.m_capacity];
			m_capacity = rhs.m_capacity;
			m_pos = rhs.m_pos;
			memcpy(m_buffer, rhs.m_buffer, m_capacity);
		}
		return *this;
	}

	SerializeBuffer(SerializeBuffer &&rhs) : m_buffer(rhs.m_buffer), m_capacity(rhs.m_capacity), m_pos(rhs.m_pos)
	{
		rhs.m_buffer = nullptr;
		rhs.m_capacity = 0;
		rhs.m_pos = 0;
	}

	SerializeBuffer &operator=(SerializeBuffer &&rhs)
	{
		if (this != &rhs) {
			delete[] m_buffer;

			m_buffer = rhs.m_buffer;
			m_capacity = rhs.m_capacity;
			m_pos = rhs.m_pos;

			rhs.m_buffer = nullptr;
			rhs.m_capacity = 0;
			rhs.m_pos = 0;
		}
		return *this;
	}

	~SerializeBuffer() { delete[] m_buffer; }

	void WriteUInt8(uint8_t v)
	{
		if (m_pos + sizeof(uint8_t) > m_capacity)
			Grow(m_capacity + sizeof(uint8_t));
		*(uint8_t *)(m_buffer + m_pos) = v;
		m_pos += sizeof(uint8_t);
	}

	void WriteUInt16(uint16_t v)
	{
		if (m_pos + sizeof(uint16_t) > m_capacity)
			Grow(m_capacity + sizeof(uint16_t));
		*(uint16_t *)(m_buffer + m_pos) = v;
		m_pos += sizeof(uint16_t);
	}

	void WriteUInt32(uint32_t v)
	{
		if (m_pos + sizeof(uint32_t) > m_capacity)
			Grow(m_capacity + sizeof(uint32_t));
		*(uint32_t *)(m_buffer + m_pos) = v;
		m_pos += sizeof(uint32_t);
	}

	void WriteUInt64(uint64_t v)
	{
		if (m_pos + sizeof(uint64_t) > m_capacity)
			Grow(m_capacity + sizeof(uint64_t));
		*(uint64_t *)(m_buffer + m_pos) = v;
		m_pos += sizeof(uint64_t);
	}

	void WriteInt8(int8_t v)
	{
		if (m_pos + sizeof(int8_t) > m_capacity)
			Grow(m_capacity + sizeof(int8_t));
		*(int8_t *)(m_buffer + m_pos) = v;
		m_pos += sizeof(int8_t);
	}

	void WriteInt16(int16_t v)
	{
		if (m_pos + sizeof(int16_t) > m_capacity)
			Grow(m_capacity + sizeof(int16_t));
		*(int16_t *)(m_buffer + m_pos) = v;
		m_pos += sizeof(int16_t);
	}

	void WriteInt32(int32_t v)
	{
		if (m_pos + sizeof(int32_t) > m_capacity)
			Grow(m_capacity + sizeof(int32_t));
		*(int32_t *)(m_buffer + m_pos) = v;
		m_pos += sizeof(int32_t);
	}

	void WriteInt64(int64_t v)
	{
		if (m_pos + sizeof(int64_t) > m_capacity)
			Grow(m_capacity + sizeof(int64_t));
		*(int64_t *)(m_buffer + m_pos) = v;
		m_pos += sizeof(int64_t);
	}

	void WriteFloat(float v)
	{
		if (m_pos + sizeof(float) > m_capacity)
			Grow(m_capacity + sizeof(float));
		*(float *)(m_buffer + m_pos) = v;
		m_pos += sizeof(float);
	}

	void WriteDouble(double v)
	{
		if (m_pos + sizeof(double) > m_capacity)
			Grow(m_capacity + sizeof(double));
		*(double *)(m_buffer + m_pos) = v;
		m_pos += sizeof(double);
	}

	void WriteString(const char *str)
	{
		assert(str != nullptr);
		auto len = std::char_traits<char>::length(str) + 1;
		if (m_pos + len > m_capacity)
			Grow(m_capacity + len);
		memcpy(m_buffer + m_pos, str, len);
		m_pos += len;
	}

	void WriteString(const std::string &str)
	{
		auto len = str.length() + 1;
		if (m_pos + len > m_capacity)
			Grow(m_capacity + len);
		memcpy(m_buffer + m_pos, str.c_str(), len);
		m_pos += len;
	}

	void WriteLengthString(uint32_t len, const char *str)
	{
		assert(str != nullptr);
		if (m_pos + len + sizeof(uint32_t) > m_capacity)
			Grow(m_capacity + len + sizeof(uint32_t));
		*(uint32_t *)(m_buffer + m_pos) = len;
		m_pos += sizeof(uint32_t);
		memcpy(m_buffer + m_pos, str, len);
		m_pos += len;
	}

	void WriteLengthString(const std::string &str)
	{
		uint32_t len = str.length();
		if (m_pos + len + sizeof(uint32_t) > m_capacity)
			Grow(m_capacity + len + sizeof(uint32_t));
		*(uint32_t *)(m_buffer + m_pos) = len;
		m_pos += sizeof(uint32_t);
		memcpy(m_buffer + m_pos, str.c_str(), len);
		m_pos += len;
	}

	size_t size() const { return m_pos; }
	size_t length() const { return size(); }
	size_t capacity() const { return m_capacity; }
	const unsigned char *buffer() const { return m_buffer; }

	friend class BasePacket;

private:
	void Grow(size_t new_size);
	void Reset();
	unsigned char *m_buffer;
	size_t m_capacity;
	size_t m_pos;
};
