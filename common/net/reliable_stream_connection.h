#pragma once

#include "common/random.h"
#include "common/net/packet.h"
#include "common/net/reliable_stream_structs.h"
#include "common/net/reliable_stream_pooling.h"

#include "uv.h"

#include <chrono>
#include <functional>
#include <list>
#include <map>
#include <memory>

namespace EQ
{
	namespace Net
	{
		enum ReliableStreamProtocolOpcode
		{
			OP_Padding = 0x00,
			OP_SessionRequest = 0x01,
			OP_SessionResponse = 0x02,
			OP_Combined = 0x03,
			OP_SessionDisconnect = 0x05,
			OP_KeepAlive = 0x06,
			OP_SessionStatRequest = 0x07,
			OP_SessionStatResponse = 0x08,
			OP_Packet = 0x09,
			OP_Packet2 = 0x0a,
			OP_Packet3 = 0x0b,
			OP_Packet4 = 0x0c,
			OP_Fragment = 0x0d,
			OP_Fragment2 = 0x0e,
			OP_Fragment3 = 0x0f,
			OP_Fragment4 = 0x10,
			OP_OutOfOrderAck = 0x11,
			OP_OutOfOrderAck2 = 0x12,
			OP_OutOfOrderAck3 = 0x13,
			OP_OutOfOrderAck4 = 0x14,
			OP_Ack = 0x15,
			OP_Ack2 = 0x16,
			OP_Ack3 = 0x17,
			OP_Ack4 = 0x18,
			OP_AppCombined = 0x19,
			OP_OutboundPing = 0x1c,
			OP_OutOfSession = 0x1d
		};

		enum DbProtocolStatus
		{
			StatusConnecting,
			StatusConnected,
			StatusDisconnecting,
			StatusDisconnected
		};

		enum ReliableStreamEncodeType
		{
			EncodeNone = 0,
			EncodeCompression = 1,
			EncodeXOR = 4,
		};

		enum SequenceOrder
		{
			SequenceCurrent,
			SequenceFuture,
			SequencePast
		};

		typedef std::chrono::steady_clock::time_point Timestamp;
		typedef std::chrono::steady_clock Clock;

		struct ReliableStreamConnectionStats
		{
			ReliableStreamConnectionStats() {
				recv_bytes = 0;
				sent_bytes = 0;
				recv_packets = 0;
				sent_packets = 0;
				sync_recv_packets = 0;
				sync_sent_packets = 0;
				sync_remote_recv_packets = 0;
				sync_remote_sent_packets = 0;
				min_ping = 0xFFFFFFFFFFFFFFFFUL;
				max_ping = 0;
				avg_ping = 0;
				created = Clock::now();
				dropped_datarate_packets = 0;
				resent_packets = 0;
				resent_fragments = 0;
				resent_full = 0;
				datarate_remaining = 0.0;
				bytes_after_decode = 0;
				bytes_before_encode = 0;
			}

			void Reset() {
				recv_bytes = 0;
				sent_bytes = 0;
				min_ping = 0xFFFFFFFFFFFFFFFFUL;
				max_ping = 0;
				avg_ping = 0;
				created = Clock::now();
				dropped_datarate_packets = 0;
				resent_packets = 0;
				resent_fragments = 0;
				resent_full = 0;
				datarate_remaining = 0.0;
				bytes_after_decode = 0;
				bytes_before_encode = 0;
			}

			uint64_t recv_bytes;
			uint64_t sent_bytes;
			uint64_t recv_packets;
			uint64_t sent_packets;
			uint64_t sync_recv_packets;
			uint64_t sync_sent_packets;
			uint64_t sync_remote_recv_packets;
			uint64_t sync_remote_sent_packets;
			uint64_t min_ping;
			uint64_t max_ping;
			uint64_t avg_ping;
			uint64_t last_ping;
			Timestamp created;
			uint64_t dropped_datarate_packets; //packets dropped due to datarate limit, couldn't think of a great name
			uint64_t resent_packets;
			uint64_t resent_fragments;
			uint64_t resent_full;
			double datarate_remaining;
			uint64_t bytes_after_decode;
			uint64_t bytes_before_encode;
		};

		class ReliableStreamConnectionManager;
		class ReliableStreamConnection;
		class ReliableStreamConnection
		{
		public:
			ReliableStreamConnection(ReliableStreamConnectionManager *owner, const ReliableStreamConnect &connect, const std::string &endpoint, int port);
			ReliableStreamConnection(ReliableStreamConnectionManager *owner, const std::string &endpoint, int port);
			~ReliableStreamConnection();

			const std::string& RemoteEndpoint() const { return m_endpoint; }
			int RemotePort() const { return m_port; }

			void Close();
			void QueuePacket(Packet &p);
			void QueuePacket(Packet &p, int stream);
			void QueuePacket(Packet &p, int stream, bool reliable);

			ReliableStreamConnectionStats GetStats();
			void ResetStats();
			size_t GetRollingPing() const { return m_rolling_ping; }
			DbProtocolStatus GetStatus() const { return m_status; }

			const ReliableStreamEncodeType* GetEncodePasses() const { return m_encode_passes; }
			const ReliableStreamConnectionManager* GetManager() const { return m_owner; }
			ReliableStreamConnectionManager* GetManager() { return m_owner; }
		private:
			ReliableStreamConnectionManager *m_owner;
			std::string m_endpoint;
			int m_port;
			uint32_t m_connect_code;
			uint32_t m_encode_key;
			uint32_t m_max_packet_size;
			uint32_t m_crc_bytes;
			ReliableStreamEncodeType m_encode_passes[2];

			Timestamp m_last_send;
			Timestamp m_last_recv;
			DbProtocolStatus m_status;
			Timestamp m_hold_time;
			std::list<DynamicPacket> m_buffered_packets;
			size_t m_buffered_packets_length;
			std::unique_ptr<char[]> m_combined;
			ReliableStreamConnectionStats m_stats;
			Timestamp m_last_session_stats;
			size_t m_rolling_ping;
			Timestamp m_close_time;
			double m_outgoing_budget;

			// resend tracking
			size_t m_resend_packets_sent = 0;
			size_t m_resend_bytes_sent = 0;
			bool m_acked_since_last_resend = false;
			Timestamp m_last_ack;

			struct ReliableStreamSentPacket
			{
				DynamicPacket packet;
				Timestamp last_sent;
				Timestamp first_sent;
				size_t times_resent;
				size_t resend_delay;
			};

			struct ReliableStream
			{
				ReliableStream() {
					sequence_in = 0;
					sequence_out = 0;
					fragment_current_bytes = 0;
					fragment_total_bytes = 0;
				}

				uint16_t sequence_in;
				uint16_t sequence_out;
				std::map<uint16_t, Packet*> packet_queue;

				DynamicPacket fragment_packet;
				uint32_t fragment_current_bytes;
				uint32_t fragment_total_bytes;

				std::map<uint16_t, ReliableStreamSentPacket> sent_packets;
			};

			ReliableStream m_streams[4];
			std::weak_ptr<ReliableStreamConnection> m_self;

			void Process();
			void ProcessPacket(Packet &p);
			void ProcessQueue();
			void RemoveFromQueue(int stream, uint16_t seq);
			void AddToQueue(int stream, uint16_t seq, const Packet &p);
			void ProcessDecodedPacket(const Packet &p);
			void ChangeStatus(DbProtocolStatus new_status);
			bool ValidateCRC(Packet &p);
			void AppendCRC(Packet &p);
			bool PacketCanBeEncoded(Packet &p) const;
			void Decode(Packet &p, size_t offset, size_t length);
			void Encode(Packet &p, size_t offset, size_t length);
			void Decompress(Packet &p, size_t offset, size_t length);
			void Compress(Packet &p, size_t offset, size_t length);
			void ProcessResend();
			void ProcessResend(int stream);
			void Ack(int stream, uint16_t seq);
			void OutOfOrderAck(int stream, uint16_t seq);
			void UpdateDataBudget(double budget_add);

			void SendConnect();
			void SendKeepAlive();
			void SendAck(int stream, uint16_t seq);
			void SendOutOfOrderAck(int stream, uint16_t seq);
			void SendDisconnect();
			void InternalBufferedSend(Packet &p);
			void InternalSend(Packet &p);
			void InternalQueuePacket(Packet &p, int stream_id, bool reliable);
			void FlushBuffer();
			SequenceOrder CompareSequence(uint16_t expected, uint16_t actual) const;

			friend class ReliableStreamConnectionManager;
		};

		struct ReliableStreamConnectionManagerOptions
		{
			ReliableStreamConnectionManagerOptions() {
				max_connection_count = 0;
				keepalive_delay_ms = 9000;
				resend_delay_ms = 30;
				resend_delay_factor = 1.25;
				resend_delay_min = 150;
				resend_delay_max = 5000;
				connect_delay_ms = 500;
				stale_connection_ms = 60000;
				connect_stale_ms = 5000;
				crc_length = 2;
				max_packet_size = 512;
				encode_passes[0] = ReliableStreamEncodeType::EncodeNone;
				encode_passes[1] = ReliableStreamEncodeType::EncodeNone;
				port = 0;
				hold_size = 512;
				hold_length_ms = 50;
				simulated_in_packet_loss = 0;
				simulated_out_packet_loss = 0;
				tic_rate_hertz = 60.0;
				resend_timeout = 30000;
				connection_close_time = 2000;
				outgoing_data_rate = 0.0;
			}

			size_t max_packet_size;
			size_t max_connection_count;
			size_t keepalive_delay_ms;
			double resend_delay_factor;
			size_t resend_delay_ms;
			size_t resend_delay_min;
			size_t resend_delay_max;
			size_t connect_delay_ms;
			size_t connect_stale_ms;
			size_t stale_connection_ms;
			size_t crc_length;
			size_t hold_size;
			size_t hold_length_ms;
			size_t simulated_in_packet_loss;
			size_t simulated_out_packet_loss;
			double tic_rate_hertz;
			size_t resend_timeout;
			size_t connection_close_time;
			ReliableStreamEncodeType encode_passes[2];
			int port;
			double outgoing_data_rate;
		};

		class ReliableStreamConnectionManager
		{
		public:
			ReliableStreamConnectionManager();
			ReliableStreamConnectionManager(const ReliableStreamConnectionManagerOptions &opts);
			~ReliableStreamConnectionManager();

			void Connect(const std::string &addr, int port);
			void Process();
			void UpdateDataBudget();
			void ProcessResend();
			void OnNewConnection(std::function<void(std::shared_ptr<ReliableStreamConnection>)> func) { m_on_new_connection = func; }
			void OnConnectionStateChange(std::function<void(std::shared_ptr<ReliableStreamConnection>, DbProtocolStatus, DbProtocolStatus)> func) { m_on_connection_state_change = func; }
			void OnPacketRecv(std::function<void(std::shared_ptr<ReliableStreamConnection>, const Packet &)> func) { m_on_packet_recv = func; }
			void OnErrorMessage(std::function<void(const std::string&)> func) { m_on_error_message = func; }

			ReliableStreamConnectionManagerOptions& GetOptions() { return m_options; }
		private:
			void Attach(uv_loop_t *loop);
			void Detach();

			EQ::Random m_rand;
			uv_timer_t m_timer;
			uv_udp_t m_socket;
			uv_loop_t *m_attached;
			ReliableStreamConnectionManagerOptions m_options;
			std::function<void(std::shared_ptr<ReliableStreamConnection>)> m_on_new_connection;
			std::function<void(std::shared_ptr<ReliableStreamConnection>, DbProtocolStatus, DbProtocolStatus)> m_on_connection_state_change;
			std::function<void(std::shared_ptr<ReliableStreamConnection>, const Packet&)> m_on_packet_recv;
			std::function<void(const std::string&)> m_on_error_message;
			std::map<std::pair<std::string, int>, std::shared_ptr<ReliableStreamConnection>> m_connections;

			void ProcessPacket(const std::string &endpoint, int port, const char *data, size_t size);
			std::shared_ptr<ReliableStreamConnection> FindConnectionByEndpoint(std::string addr, int port);
			void SendDisconnect(const std::string &addr, int port);

			friend class ReliableStreamConnection;
		};
	}
}
