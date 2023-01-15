#pragma once

#include "../state.hpp"

#include "../ft_sha1_info.hpp"

#include <mio/mio.hpp>

#include <vector>
#include <deque>

namespace States {

// we are either sending or receiving
// we have full info
struct SHA1 final : public StateI {
	public: // general interface
		SHA1(
			ToxClient& tcl,
			mio::mmap_source&& file_map,
			const FTInfoSHA1&& sha1_info,
			const std::vector<uint8_t>&& sha1_info_data,
			const std::vector<uint8_t>&& sha1_info_hash,
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

	private:
		mio::mmap_source _file_map;
		const FTInfoSHA1 _sha1_info;
		const std::vector<uint8_t> _sha1_info_data;
		const SHA1Digest _sha1_info_hash;

		// index is the same as for info
		std::vector<bool> _have_chunk;
		bool _have_all {false};

		// group_number, peer_number
		std::deque<std::pair<uint32_t, uint32_t>> _queue_requested_info;
		// group_number, peer_number, transfer_id, second since (remote) activity
		std::vector<std::tuple<uint32_t, uint32_t, uint8_t, float>> _transfers_requested_info;
};

} // States

