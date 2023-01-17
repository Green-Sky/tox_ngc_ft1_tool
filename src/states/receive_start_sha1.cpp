#include "./receive_start_sha1.hpp"

#include "./sha1.hpp"

#include "../hash_utils.hpp"
#include "../tox_utils.hpp"
#include "../ft_sha1_info.hpp"

#include "../tox_client.hpp"

#include <filesystem>
#include <fstream>
#include <mio/mio.hpp>

#include <iostream>
#include <exception>
#include <memory>
#include <system_error>
#include <tuple>

namespace States {

ReceiveStartSHA1::ReceiveStartSHA1(ToxClient& tcl, const CommandLine& cl) : StateI(tcl), _cl(cl), _dump_dir(cl.receive_dump_dir) {
	if (cl.receive_id.empty()) {
		throw std::runtime_error("receiver missing id");
	}

	_sha1_info_hash = hex2bin(cl.receive_id);
	assert(_sha1_info_hash.size() == 20);

	std::cout << "ReceiveStartSHA1 downloading info for " << cl.receive_id << "\n";
}

bool ReceiveStartSHA1::iterate(float delta) {
	_time_since_last_request += delta;

	// iterate and timeout
	if (_transfer.has_value()) {
		float& time_since_remote_activity = std::get<float>(_transfer.value());
		time_since_remote_activity += delta;

		// timout if not heard after 10s
		if (time_since_remote_activity >= 10.f) {
			std::cerr << "ReceiveStartSHA1 info tansfer timed out " << std::get<0>(*_transfer) << ":" << std::get<1>(*_transfer) << "." << int(std::get<2>(*_transfer)) << "\n";

			_transfer.reset();
		}
	} else if (_time_since_last_request >= 15.f) { // blast ever 15sec
		_time_since_last_request = 0.f;
		// TODO: select random and try, not blas
		// ... and we are blasing
		_tcl.forEachGroup([this](const uint32_t group_number) {
			_tcl.forEachGroupPeer(group_number, [this, group_number](uint32_t peer_number) {
				_tcl.sendFT1RequestPrivate(
					group_number, peer_number,
					NGC_FT1_file_kind::HASH_SHA1_INFO,
					_sha1_info_hash.data.data(), _sha1_info_hash.size()
				);
				std::cout << "ReceiveStartSHA1 sendig info request to " << group_number << ":" << peer_number << "\n";
			});
		});
	}

	// if not transfer, request from random peer (equal dist!!)
	// TODO: return true if done
	return _done;
}

std::unique_ptr<StateI> ReceiveStartSHA1::nextState(void) {
	FTInfoSHA1 sha1_info;
	sha1_info.fromBuffer(_sha1_info_data);

	std::cout << "ReceiveStartSHA1 info is: \n" << sha1_info;

	auto file_path = std::filesystem::path{_dump_dir} / sha1_info.file_name;

	bool file_existed = std::filesystem::exists(file_path);
	if (!file_existed) {
		if (!_dump_dir.empty()) {
			std::filesystem::create_directories(_dump_dir);
		}
		std::ofstream(file_path) << '\0'; // create the file
	}
	std::filesystem::resize_file(file_path, sha1_info.file_size);

	// open file for writing (pre allocate?)
	std::error_code err;
	mio::mmap_sink file_map = mio::make_mmap_sink(file_path.string(), 0, sha1_info.file_size, err);

	std::vector<bool> have_chunk(sha1_info.chunks.size(), false);

	// dont overwrite correct existing data
	if (file_existed) {
		std::cout << "ReceiveStartSHA1 checking existing file\n";
		size_t f_i {0};
		size_t tmp_have_count {0};
		for (size_t c_i = 0; f_i + sha1_info.chunk_size < file_map.length(); f_i += sha1_info.chunk_size, c_i++) {
			if (sha1_info.chunks[c_i] == hash_sha1(file_map.data()+f_i, sha1_info.chunk_size)) {
				have_chunk[c_i] = true;
				tmp_have_count++;
			}
		}

		if (f_i < file_map.length()) {
			if (sha1_info.chunks.back() == hash_sha1(file_map.data()+f_i, file_map.length()-f_i)) {
				have_chunk.back() = true;
				tmp_have_count++;
			}
		}

		std::cout << "ReceiveStartSHA1 have " << tmp_have_count << "/" << sha1_info.chunks.size() << " chunks (" << float(tmp_have_count)/sha1_info.chunks.size() << "%)\n";
	}

	std::cout << "ReceiveStartSHA1 switching state to SHA1\n";
	return std::make_unique<SHA1>(
		_tcl,
		_cl,
		std::move(file_map),
		std::move(sha1_info),
		std::move(_sha1_info_data),
		std::move(_sha1_info_hash),
		std::move(have_chunk)
	);
}

// sha1_info
void ReceiveStartSHA1::onFT1ReceiveRequestSHA1Info(uint32_t, uint32_t, const uint8_t*, size_t) {
	// shrug, we dont have it either
}

bool ReceiveStartSHA1::onFT1ReceiveInitSHA1Info(uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size, const uint8_t transfer_id, const size_t file_size) {
	if (file_id_size != _sha1_info_hash.size()) {
		std::cerr << "ReceiveStartSHA1 got request for sha1_info of wrong size!!\n";
		return false;
	}

	SHA1Digest requested_hash(file_id, file_id_size);

	if (requested_hash != _sha1_info_hash) {
		std::cout << "ReceiveStartSHA1 ignoring different info request " << requested_hash << "\n";
		return false;
	}

	if (_transfer.has_value()) {
		// TODO: log?
		return false; // already in progress
	}

	_sha1_info_data.resize(file_size);

	_transfer = std::make_tuple(group_number, peer_number, transfer_id, 0.f);
	std::cout << "ReceiveStartSHA1 accepted info transfer" << group_number << ":" << peer_number << "." << int(transfer_id) << "\n";

	// accept
	return true;
}

void ReceiveStartSHA1::onFT1ReceiveDataSHA1Info(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, const uint8_t* data, size_t data_size) {
	// TODO: test if not current transfer

	for (size_t i = 0; i < data_size; i++) {
		_sha1_info_data[data_offset+i] = data[i];
	}

	std::get<float>(_transfer.value()) = 0.f;

	std::cout << "ReceiveStartSHA1 " << data_offset+data_size << "/" << _sha1_info_data.size() << " (" << float(data_offset+data_size) / _sha1_info_data.size() * 100.f << "%)\n";

	if (data_offset + data_size == _sha1_info_data.size()) {
		// hash and verify
		SHA1Digest test_hash = hash_sha1(_sha1_info_data.data(), _sha1_info_data.size());
		if (test_hash != _sha1_info_hash) {
			std::cerr << "ReceiveStartSHA1 received info's hash does not match!, discarding\n";
			_transfer.reset();
			_sha1_info_data.clear();
		}

		std::cout << "ReceiveStartSHA1 info tansfer finished " << group_number << ":" << peer_number << "." << int(transfer_id) << "\n";
		_done = true;
	}
}

void ReceiveStartSHA1::onFT1SendDataSHA1Info(uint32_t, uint32_t, uint8_t, size_t, uint8_t*, size_t) {
	// we cant send what we dont have
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

