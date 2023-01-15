#include "./sha1.hpp"

namespace States {

SHA1::SHA1(
	ToxClient& tcl,
	mio::mmap_source&& file_map,
	const FTInfoSHA1&& sha1_info,
	const std::vector<uint8_t>&& sha1_info_data,
	const std::vector<uint8_t>&& sha1_info_hash
) :
	StateI(tcl),
	_file_map(std::move(file_map)),
	_sha1_info(std::move(sha1_info)),
	_sha1_info_data(std::move(sha1_info_data)),
	_sha1_info_hash(std::move(sha1_info_hash))
{
}

bool SHA1::iterate(void) {
	return false;
}

std::unique_ptr<StateI> SHA1::nextState(void) {
	return nullptr;
}

// sha1_info
void SHA1::onFT1ReceiveRequestSHA1Info(uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size) {
}

bool SHA1::onFT1ReceiveInitSHA1Info(uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size, const uint8_t transfer_id, const size_t file_size) {
	return false;
}

void SHA1::onFT1ReceiveDataSHA1Info(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, const uint8_t* data, size_t data_size) {
}

void SHA1::onFT1SendDataSHA1Info(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, uint8_t* data, size_t data_size) {
}

// sha1_chunk
void SHA1::onFT1ReceiveRequestSHA1Chunk(uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size) {
}

bool SHA1::onFT1ReceiveInitSHA1Chunk(uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size, const uint8_t transfer_id, const size_t file_size) {
	return false;
}

void SHA1::onFT1ReceiveDataSHA1Chunk(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, const uint8_t* data, size_t data_size) {
}

void SHA1::onFT1SendDataSHA1Chunk(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, uint8_t* data, size_t data_size) {
}

} // States

