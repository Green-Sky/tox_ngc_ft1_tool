#include "./receive_start_sha1.hpp"

#include "./sha1.hpp"

#include "../tox_utils.hpp"
#include "../ft_sha1_info.hpp"

#include <mio/mio.hpp>

#include <iostream>
#include <exception>
#include <memory>

namespace States {

ReceiveStartSHA1::ReceiveStartSHA1(ToxClient& tcl, const CommandLine& cl) : StateI(tcl) {
	if (cl.receive_id.empty()) {
		throw std::runtime_error("receiver missing id");
	}

	_sha1_info_hash = hex2bin(cl.receive_id);
	assert(_sha1_info_hash.size() == 20);

	std::cout << "ReceiveStartSHA1 downloading info for " << cl.receive_id << "\n";
}

bool ReceiveStartSHA1::iterate(float delta) {
	// TODO: return true if done
	return false;
}

std::unique_ptr<StateI> ReceiveStartSHA1::nextState(void) {
	std::cout << "ReceiveStartSHA1 switching state to SHA1\n";

	FTInfoSHA1 sha1_info;
	// from buffer

	// open file for writing (pre allocate?)
	mio::mmap_sink file_map;

	std::vector<bool> have_chunk(sha1_info.chunks.size(), false);

	return std::make_unique<SHA1>(
		_tcl,
		std::move(file_map),
		std::move(sha1_info),
		std::move(_sha1_info_data),
		std::move(_sha1_info_hash),
		std::move(have_chunk)
	);
}

// sha1_info
void ReceiveStartSHA1::onFT1ReceiveRequestSHA1Info(uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size) {
}

bool ReceiveStartSHA1::onFT1ReceiveInitSHA1Info(uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size, const uint8_t transfer_id, const size_t file_size) {
	// accept
	return true;
}

void ReceiveStartSHA1::onFT1ReceiveDataSHA1Info(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, const uint8_t* data, size_t data_size) {
}

void ReceiveStartSHA1::onFT1SendDataSHA1Info(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, uint8_t* data, size_t data_size) {
}

// sha1_chunk
void ReceiveStartSHA1::onFT1ReceiveRequestSHA1Chunk(uint32_t, uint32_t, const uint8_t*, size_t) {
}

bool ReceiveStartSHA1::onFT1ReceiveInitSHA1Chunk(uint32_t, uint32_t, const uint8_t*, size_t, const uint8_t, const size_t) {
	return false; // nope, not interested in chunks yet
}

void ReceiveStartSHA1::onFT1ReceiveDataSHA1Chunk(uint32_t, uint32_t, uint8_t, size_t, const uint8_t*, size_t) {
}

void ReceiveStartSHA1::onFT1SendDataSHA1Chunk(uint32_t, uint32_t, uint8_t, size_t, uint8_t*, size_t) {
}

} // States

