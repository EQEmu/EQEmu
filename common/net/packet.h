#pragma once

#include "common/util/memory_stream.h"

#include "cereal/archives/binary.hpp"
#include "cereal/cereal.hpp"

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>

namespace EQ {
	namespace Net {
		class Packet
		{
		public:
			Packet() : m_stream(std::ios::out | std::ios::binary) { }
			virtual ~Packet() { }

			virtual const void *Data() const = 0;
			virtual void *Data() = 0;
			virtual size_t Length() const = 0;
			virtual size_t Length() = 0;
			virtual bool Clear() = 0;
			virtual bool Resize(size_t new_size) = 0;
			virtual void Reserve(size_t new_size) = 0;

			template<typename T>
			T GetSerialize(size_t offset) const
			{
				T ret;
				Util::MemoryStreamReader reader(((char*)Data() + offset), Length());
				cereal::BinaryInputArchive input(reader);
				input(ret);
				return ret;
			}

			template<typename T>
			void PutSerialize(size_t offset, const T &value) {
				m_stream.clear();
				cereal::BinaryOutputArchive output(m_stream);
				output(value);

				auto str = m_stream.str();
				if (Length() < offset + str.length()) {
					if (!Resize(offset + str.length())) {
						throw std::out_of_range("Packet::PutSerialize(), could not resize packet and would of written past the end.");
					}
				}

				memcpy((char*)Data() + offset, &str[0], str.length());
			}

			void PutInt8(size_t offset, int8_t value);
			void PutInt16(size_t offset, int16_t value);
			void PutInt32(size_t offset, int32_t value);
			void PutInt64(size_t offset, int64_t value);
			void PutUInt8(size_t offset, uint8_t value);
			void PutUInt16(size_t offset, uint16_t value);
			void PutUInt32(size_t offset, uint32_t value);
			void PutUInt64(size_t offset, uint64_t value);
			void PutFloat(size_t offset, float value);
			void PutDouble(size_t offset, double value);
			void PutString(size_t offset, const std::string &str);
			void PutCString(size_t offset, const char *str);
			void PutPacket(size_t offset, const Packet &p);
			void PutData(size_t offset, void *data, size_t length);

			int8_t GetInt8(size_t offset) const;
			int16_t GetInt16(size_t offset) const;
			int32_t GetInt32(size_t offset) const;
			int64_t GetInt64(size_t offset) const;
			uint8_t GetUInt8(size_t offset) const;
			uint16_t GetUInt16(size_t offset) const;
			uint32_t GetUInt32(size_t offset) const;
			uint64_t GetUInt64(size_t offset) const;
			float GetFloat(size_t offset) const;
			double GetDouble(size_t offset) const;
			std::string GetString(size_t offset, size_t length) const;
			std::string GetCString(size_t offset) const;

			std::string ToString() const;
			std::string ToString(size_t line_length) const;
		protected:
			std::stringstream m_stream;
		};

		class StaticPacket : public Packet
		{
		public:
			StaticPacket(void *data, size_t size) { m_data = data; m_data_length = size; m_max_data_length = size; }
			virtual ~StaticPacket() { }
			StaticPacket(const StaticPacket &o) { m_data = o.m_data; m_data_length = o.m_data_length; m_max_data_length = o.m_max_data_length; }
			StaticPacket& operator=(const StaticPacket &o) { m_data = o.m_data; m_data_length = o.m_data_length; return *this; }
			StaticPacket(StaticPacket &&o) noexcept { m_data = o.m_data; m_data_length = o.m_data_length; }

			virtual const void *Data() const { return m_data; }
			virtual void *Data() { return m_data; }
			virtual size_t Length() const { return m_data_length; }
			virtual size_t Length() { return m_data_length; }
			virtual bool Clear() { return false; }
			virtual bool Resize(size_t new_size);
			virtual void Reserve(size_t new_size) { }

		protected:
			void *m_data;
			size_t m_data_length;
			size_t m_max_data_length;
		};

		class DynamicPacket : public Packet
		{
		public:
			DynamicPacket() { }
			virtual ~DynamicPacket() { }
			DynamicPacket(DynamicPacket &&o) noexcept { m_data = std::move(o.m_data); }
			DynamicPacket(const DynamicPacket &o) { m_data = o.m_data; }
			DynamicPacket& operator=(const DynamicPacket &o) { m_data = o.m_data; return *this; }

			virtual const void *Data() const { return &m_data[0]; }
			virtual void *Data() { return &m_data[0]; }
			virtual size_t Length() const { return m_data.size(); }
			virtual size_t Length() { return m_data.size(); }
			virtual bool Clear() { m_data.clear(); return true; }
			virtual bool Resize(size_t new_size) { m_data.resize(new_size, 0); return true; }
			virtual void Reserve(size_t new_size) { m_data.reserve(new_size); }
		protected:
			std::vector<char> m_data;
		};
	}
}
