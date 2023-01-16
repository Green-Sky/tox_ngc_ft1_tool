#pragma once

#include "../state.hpp"

#include "../command_line.hpp"
#include "../ft_sha1_info.hpp"

#include <vector>
#include <optional>

namespace States {

// we have the info hash and request the info until we have it
struct ReceiveStartSHA1 final : public StateI {
	public: // general interface
		ReceiveStartSHA1(ToxClient& tcl, const CommandLine& cl);
		~ReceiveStartSHA1(void) override = default;

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
		std::string _dump_dir;

		//FTInfoSHA1 _sha1_info;
		std::vector<uint8_t> _sha1_info_data;
		SHA1Digest _sha1_info_hash; // treat as const

		// group_number, peer_number, transfer_id, time_since_remote_activity
		std::optional<std::tuple<uint32_t, uint32_t, uint8_t, float>> _transfer;

		bool _done {false};

		float _time_since_last_request {5.f};
};

} // States

