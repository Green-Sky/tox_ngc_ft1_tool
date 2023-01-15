#include "./send_start_sha1.hpp"

#include "../tox_client.hpp"
#include "../tox_utils.hpp"
#include "../hash_utils.hpp"
#include "../ft_sha1_info.hpp"

#include "./sha1.hpp"

#include <memory>
#include <mio/mio.hpp>

#include <iostream>
#include <cassert>
#include <vector>

namespace States {

SendStartSHA1::SendStartSHA1(ToxClient& tcl, const CommandLine& cl) : StateI(tcl) {
	std::cout << "SendStartSHA1 start building sha1_info\n";
	std::error_code err;
	_file_map = mio::make_mmap_source(cl.send_path, 0, mio::map_entire_file, err);

	// TODO: propper error checking

	assert(!_file_map.empty());

	// build info
	_sha1_info.file_name = "testfile.bin";
	_sha1_info.file_size = _file_map.length();

	{ // build chunks
		size_t i = 0;
		for (; i + FTInfoSHA1::chunk_size < _file_map.length(); i += FTInfoSHA1::chunk_size) {
			_sha1_info.chunks.push_back(hash_sha1(_file_map.data()+i, FTInfoSHA1::chunk_size));
		}

		if (i < _file_map.length()) {
			_sha1_info.chunks.push_back(hash_sha1(_file_map.data()+i, _file_map.length()-i));
		}
	}

	{ // unload file and remap (for when the file was large)
		_file_map.unmap();
		_file_map = mio::make_mmap_source(cl.send_path, 0, mio::map_entire_file, err);
	}

	std::cout << "SendStartSHA1 chunks: " << _sha1_info.chunks.size() << "\n";

	_sha1_info_data = _sha1_info.toBuffer();

	std::cout << "SendStartSHA1 sha1_info size: " << _sha1_info_data.size() << "\n";

	_sha1_info_hash = hash_sha1(_sha1_info_data.data(), _sha1_info_data.size());

	std::cout << "SendStartSHA1 sha1_info_hash: " << bin2hex(_sha1_info_hash) << "\n";
}

bool SendStartSHA1::iterate(float) {
	return true; // TODO: change hashing to async
}

std::unique_ptr<StateI> SendStartSHA1::nextState(void) {
	std::cout << "SendStartSHA1 switching state to SHA1\n";
	std::vector<bool> have_chunk(_sha1_info.chunks.size(), true);
	// we are done setting up
	return std::make_unique<SHA1>(
		_tcl,
		std::move(_file_map),
		std::move(_sha1_info),
		std::move(_sha1_info_data),
		std::move(_sha1_info_hash),
		std::move(have_chunk)
	);
}

// sha1_info
void SendStartSHA1::onFT1ReceiveRequestSHA1Info(uint32_t, uint32_t, const uint8_t*, size_t) {
}

bool SendStartSHA1::onFT1ReceiveInitSHA1Info(uint32_t, uint32_t, const uint8_t*, size_t, const uint8_t, const size_t) {
	return false;
}

void SendStartSHA1::onFT1ReceiveDataSHA1Info(uint32_t, uint32_t, uint8_t, size_t, const uint8_t*, size_t) {
}

void SendStartSHA1::onFT1SendDataSHA1Info(uint32_t, uint32_t, uint8_t, size_t, uint8_t*, size_t) {
}

// sha1_chunk
void SendStartSHA1::onFT1ReceiveRequestSHA1Chunk(uint32_t, uint32_t, const uint8_t*, size_t) {
}

bool SendStartSHA1::onFT1ReceiveInitSHA1Chunk(uint32_t, uint32_t, const uint8_t*, size_t, const uint8_t, const size_t) {
	return false;
}

void SendStartSHA1::onFT1ReceiveDataSHA1Chunk(uint32_t, uint32_t, uint8_t, size_t, const uint8_t*, size_t) {
}

void SendStartSHA1::onFT1SendDataSHA1Chunk(uint32_t, uint32_t, uint8_t, size_t, uint8_t*, size_t) {
}

} // States

