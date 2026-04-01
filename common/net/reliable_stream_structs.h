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

#include "common/net/endian.h"

#include "cereal/cereal.hpp"
#include <cstdint>

namespace EQ
{
	namespace Net
	{
		struct ReliableStreamHeader
		{
			static size_t size() { return 2; }
			uint8_t zero;
			uint8_t opcode;

			template <class Archive>
			void serialize(Archive & archive)
			{
				archive(zero,
					opcode);
			}
		};

		struct ReliableStreamConnect
		{
			static size_t size() { return 14; }
			uint8_t zero;
			uint8_t opcode;
			uint32_t protocol_version;
			uint32_t connect_code;
			uint32_t max_packet_size;

			template <class Archive>
			void serialize(Archive & archive)
			{
				archive(zero,
					opcode,
					protocol_version,
					connect_code,
					max_packet_size);
			}
		};

		struct ReliableStreamConnectReply
		{
			static size_t size() { return 17; }
			uint8_t zero;
			uint8_t opcode;
			uint32_t connect_code;
			uint32_t encode_key;
			uint8_t crc_bytes;
			uint8_t encode_pass1;
			uint8_t encode_pass2;
			uint32_t max_packet_size;

			template <class Archive>
			void serialize(Archive & archive)
			{
				archive(zero,
					opcode,
					connect_code,
					encode_key,
					crc_bytes,
					encode_pass1,
					encode_pass2,
					max_packet_size);
			}
		};

		struct ReliableStreamDisconnect
		{
			static size_t size() { return 8; }
			uint8_t zero;
			uint8_t opcode;
			uint32_t connect_code;

			template <class Archive>
			void serialize(Archive & archive)
			{
				archive(zero,
					opcode,
					connect_code);
			}
		};

		struct ReliableStreamReliableHeader
		{
			static size_t size() { return 4; }
			uint8_t zero;
			uint8_t opcode;
			uint16_t sequence;

			template <class Archive>
			void serialize(Archive & archive)
			{
				archive(zero,
					opcode,
					sequence);
			}
		};

		struct ReliableStreamReliableFragmentHeader
		{
			static size_t size() { return 4 + ReliableStreamReliableHeader::size(); }
			ReliableStreamReliableHeader reliable;
			uint32_t total_size;

			template <class Archive>
			void serialize(Archive & archive)
			{
				archive(reliable,
					total_size);
			}
		};

		struct ReliableStreamSessionStatRequest
		{
			static size_t size() { return 40; }
			uint8_t zero;
			uint8_t opcode;
			uint16_t timestamp;
			uint32_t stat_ping;
			uint32_t avg_ping;
			uint32_t min_ping;
			uint32_t max_ping;
			uint32_t last_ping;
			uint64_t packets_sent;
			uint64_t packets_recv;

			template <class Archive>
			void serialize(Archive & archive)
			{
				archive(zero,
					opcode,
					timestamp,
					stat_ping,
					avg_ping,
					min_ping,
					max_ping,
					last_ping,
					packets_sent,
					packets_recv);
			}
		};

		struct ReliableStreamSessionStatResponse
		{
			static size_t size() { return 40; }
			uint8_t zero;
			uint8_t opcode;
			uint16_t timestamp;
			uint32_t our_timestamp;
			uint64_t client_sent;
			uint64_t client_recv;
			uint64_t server_sent;
			uint64_t server_recv;

			template <class Archive>
			void serialize(Archive & archive)
			{
				archive(zero,
					opcode,
					timestamp,
					our_timestamp,
					client_sent,
					client_recv,
					server_sent,
					server_recv);
			}
		};
	}
}

