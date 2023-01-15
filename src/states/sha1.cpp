#include "./sha1.hpp"

#include <iostream>

namespace States {

SHA1::SHA1(
	ToxClient& tcl,
	mio::mmap_source&& file_map,
	const FTInfoSHA1&& sha1_info,
	const std::vector<uint8_t>&& sha1_info_data,
	const std::vector<uint8_t>&& sha1_info_hash,
	std::vector<bool>&& have_chunk
) :
	StateI(tcl),
	_file_map(std::move(file_map)),
	_sha1_info(std::move(sha1_info)),
	_sha1_info_data(std::move(sha1_info_data)),
	_sha1_info_hash(std::move(sha1_info_hash)),
	_have_chunk(std::move(have_chunk))
{
	_have_all = true;
	for (const bool it : _have_chunk) {
		if (!it) {
			_have_all = false;
			break;
		}
	}
}

bool SHA1::iterate(void) {
	// TODO: unmap and remap the file every couple of minutes to keep ram usage down?
	// TODO: when to stop?
	return false;
}

std::unique_ptr<StateI> SHA1::nextState(void) {
	return nullptr;
}

// sha1_info
void SHA1::onFT1ReceiveRequestSHA1Info(uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size) {
	// start tf (init) for sha1_info
	if (file_id_size != _sha1_info_hash.size()) {
		std::cerr << "SHA1 got request for sha1_info of wrong size!!\n";
		return;
	}

	SHA1Digest requested_hash(file_id, file_id_size);

	if (requested_hash != _sha1_info_hash) {
		std::cout << "SHA1 ignoring diffenrent info request " << requested_hash << "\n";
	}

	// same hash, should respond
	// prio higher then chunks?
}

bool SHA1::onFT1ReceiveInitSHA1Info(uint32_t, uint32_t, const uint8_t*, size_t, const uint8_t, const size_t) {
	// no, in this state we have init
	return false;
}

void SHA1::onFT1ReceiveDataSHA1Info(uint32_t, uint32_t, uint8_t, size_t, const uint8_t*, size_t) {
	// no, in this state we have init
}

void SHA1::onFT1SendDataSHA1Info(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, uint8_t* data, size_t data_size) {
	// should all be fine
	(void)group_number;
	(void)peer_number;
	(void)transfer_id;

	for (size_t i = 0; i < data_size; i++) {
		data[i] = _sha1_info_data.at(data_offset+i);
	}
	// knowing when to end might be important
}

// sha1_chunk
void SHA1::onFT1ReceiveRequestSHA1Chunk(uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size) {
#if 0
	bool have {false};
	if (_have_all) {
		have = _have_all;
	} else if (haveChunk(xxx)) {
		have = true;
	}
#endif
}

bool SHA1::onFT1ReceiveInitSHA1Chunk(uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size, const uint8_t transfer_id, const size_t file_size) {
	return false;
}

void SHA1::onFT1ReceiveDataSHA1Chunk(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, const uint8_t* data, size_t data_size) {
}

void SHA1::onFT1SendDataSHA1Chunk(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, uint8_t* data, size_t data_size) {
}

} // States

