#pragma once

#include <memory>

// fwd
struct ToxClient;

struct StateI {
	public: // general interface
		StateI(ToxClient& tcl) : _tcl(tcl) {}
		virtual ~StateI(void) = default;

		// returns true if we should call nextState()
		virtual bool iterate(float delta) = 0;
		// returns the new state (transition)
		virtual std::unique_ptr<StateI> nextState(void) = 0;

	public: // callbacks
		// sha1_info
		virtual void onFT1ReceiveRequestSHA1Info(uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size) = 0;
		virtual bool onFT1ReceiveInitSHA1Info(uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size, const uint8_t transfer_id, const size_t file_size) = 0;
		virtual void onFT1ReceiveDataSHA1Info(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, const uint8_t* data, size_t data_size) = 0;
		virtual void onFT1SendDataSHA1Info(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, uint8_t* data, size_t data_size) = 0;

		// sha1_chunk
		virtual void onFT1ReceiveRequestSHA1Chunk(uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size) = 0;
		virtual bool onFT1ReceiveInitSHA1Chunk(uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size, const uint8_t transfer_id, const size_t file_size) = 0;
		virtual void onFT1ReceiveDataSHA1Chunk(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, const uint8_t* data, size_t data_size) = 0;
		virtual void onFT1SendDataSHA1Chunk(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, uint8_t* data, size_t data_size) = 0;

	protected:
		ToxClient& _tcl;
};

