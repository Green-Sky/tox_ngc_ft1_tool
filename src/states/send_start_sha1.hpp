#pragma once

#include "../state.hpp"

#include "../command_line.hpp"
#include "../ft_sha1_info.hpp"

#include <mio/mio.hpp>

namespace States {

// we are hashing the given file
// does not react to any ft events
struct SendStartSHA1 final : public StateI {
	public: // general interface
		SendStartSHA1(ToxClient& tcl, const CommandLine& cl);
		~SendStartSHA1(void) override = default;

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
		const CommandLine& _cl;

		std::string _file_path;
		mio::mmap_source _file_map;
		FTInfoSHA1 _sha1_info;
		std::vector<uint8_t> _sha1_info_data;
		std::vector<uint8_t> _sha1_info_hash;
};

} // States

