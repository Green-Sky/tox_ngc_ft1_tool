#pragma once

#include "../state.hpp"

#include "../ft_sha1_info.hpp"
#include "../command_line.hpp"

#include <mio/mio.hpp>

#include <unordered_map>
#include <map>
#include <vector>
#include <deque>
#include <random>

namespace States {

// we are either sending or receiving
// we have full info
struct SHA1 final : public StateI {
	public: // general interface
		SHA1(
			ToxClient& tcl,
			const CommandLine& cl,
			mio::mmap_sink&& file_map,
			const FTInfoSHA1&& sha1_info,
			const std::vector<uint8_t>&& sha1_info_data,
			//const std::vector<uint8_t>&& sha1_info_hash,
			const SHA1Digest&& sha1_info_hash,
			std::vector<bool>&& have_chunk
		);
		~SHA1(void) override = default;

		bool iterate(float delta) override;
		std::unique_ptr<StateI> nextState(void) override;

	public: // callbacks
		// sha1_info
		void onFT1ReceiveRequestSHA1Info(uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size) override;
		bool onFT1ReceiveInitSHA1Info(uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size, const uint8_t transfer_id, const size_t file_size) override;
		void onFT1ReceiveDataSHA1Info(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, const uint8_t* data, size_t data_size) override;
		void onFT1SendDataSHA1Info(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, uint8_t* data, size_t data_size) override;

		// sha1_chunk
		void onFT1ReceiveRequestSHA1Chunk(uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size) override;
		bool onFT1ReceiveInitSHA1Chunk(uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size, const uint8_t transfer_id, const size_t file_size) override;
		void onFT1ReceiveDataSHA1Chunk(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, const uint8_t* data, size_t data_size) override;
		void onFT1SendDataSHA1Chunk(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, uint8_t* data, size_t data_size) override;

	private:
		// avoids duplicates
		// clears timer if exists
		void queueUpRequestInfo(uint32_t group_number, uint32_t peer_number);
		void queueUpRequestChunk(uint32_t group_number, uint32_t peer_number, const SHA1Digest& hash);

		std::optional<size_t> chunkIndex(const SHA1Digest& hash) const;
		size_t chunkSize(size_t chunk_index) const;
		bool haveChunk(const SHA1Digest& hash) const;

	private:
		bool _udp_only {false};

		mio::mmap_sink _file_map; // writable if not all
		const FTInfoSHA1 _sha1_info;
		const std::vector<uint8_t> _sha1_info_data;
		const SHA1Digest _sha1_info_hash;

		float _io_log_timer {0.f};
		size_t _bytes_up {0};
		size_t _bytes_up_last_log {0};
		size_t _bytes_down {0};
		size_t _bytes_down_last_log {0};

		// index is the same as for info
		std::vector<bool> _have_chunk;
		bool _have_all {false};
		size_t _have_count {0};
		std::deque<size_t> _chunk_want_queue;
		// chunk_index -> time since request
		std::map<size_t, float> _chunks_requested;

		size_t _max_concurrent_in {32};
		size_t _max_concurrent_out {16};

		std::minstd_rand _rng {1337};
		std::uniform_int_distribution<size_t> _distrib;

		std::unordered_map<SHA1Digest, size_t> _chunk_hash_to_index;

		// group_number, peer_number
		std::deque<std::pair<uint32_t, uint32_t>> _queue_requested_info;
		// group_number, peer_number, transfer_id, seconds since (remote) activity
		std::vector<std::tuple<uint32_t, uint32_t, uint8_t, float>> _transfers_requested_info;

		// group_number, peer_number, chunk_hash
		std::deque<std::tuple<uint32_t, uint32_t, SHA1Digest, float>> _queue_requested_chunk;

		// group_number, peer_number, transfer_id(i/o), seconds since (remote) activity, chunk index
		std::vector<std::tuple<uint32_t, uint32_t, uint8_t, float, size_t>> _transfers_sending_chunk;
		std::vector<std::tuple<uint32_t, uint32_t, uint8_t, float, size_t>> _transfers_receiving_chunk;

		static constexpr size_t _peer_speed_mesurement_interval_count {20};
		const float _peer_speed_mesurement_interval {0.5f}; // seconds
		float _peer_speed_mesurement_interval_timer {0.f}; // seconds
		// bytes received for last 6 intervals for peer
		std::map<std::pair<uint32_t, uint32_t>, std::array<int64_t, _peer_speed_mesurement_interval_count>> _peer_in_bytes_array;
		size_t _peer_in_bytes_array_index {0}; // current index into _peer_in_bytes_array. !ringbuffer!
		// when chunk data is received, it is added to _peer_in_bytes_array_index in _peer_in_bytes_array
		// every _peer_speed_mesurement_interval the avg is calculed and written to _peer_in_speed
		// and the _peer_in_bytes_array_index is incremented by 1
		std::map<std::pair<uint32_t, uint32_t>, float> _peer_in_speed;
		// speed might be not the actual speed, since wrong data is removed afterwards (on "completion")
		// so it can get negative. this makes this more useful for peer selection, less for userfacing stats
		// _peer_in_speed feeds directly into _peer_in_targets_dist
		std::vector<std::pair<uint32_t, uint32_t>> _peer_in_targets;
		std::discrete_distribution<size_t> _peer_in_targets_dist;

};

} // States

