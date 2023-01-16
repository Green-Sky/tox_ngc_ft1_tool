#include "./sha1.hpp"

#include "../tox_client.hpp"

#include <iostream>
#include <tuple>

namespace States {

SHA1::SHA1(
	ToxClient& tcl,
	mio::mmap_sink&& file_map,
	const FTInfoSHA1&& sha1_info,
	const std::vector<uint8_t>&& sha1_info_data,
	//const std::vector<uint8_t>&& sha1_info_hash,
	const SHA1Digest&& sha1_info_hash,
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
	_have_count = 0;
	for (const bool it : _have_chunk) {
		if (!it) {
			_have_all = false;
		} else {
			_have_count++;
		}
	}

	// build lookup table
	for (size_t i = 0; i < _sha1_info.chunks.size(); i++) {
		_chunk_hash_to_index[_sha1_info.chunks[i]] = i;
	}
}

bool SHA1::iterate(float delta) {
	// do ongoing transfers, send data?, timeout
	// info
	for (auto it = _transfers_requested_info.begin(); it != _transfers_requested_info.end();) {
		float& time_since_remove_activity = std::get<float>(*it);
		time_since_remove_activity += delta;

		// if we have not heard for 10sec, timeout
		if (time_since_remove_activity >= 10.f) {
			std::cerr << "SHA1 info tansfer timed out " << std::get<0>(*it) << ":" << std::get<1>(*it) << "." << std::get<2>(*it) << "\n";
			it = _transfers_requested_info.erase(it);
		} else {
			it++;
		}
	}
	// chunk sending
	for (auto it = _transfers_sending_chunk.begin(); it != _transfers_sending_chunk.end();) {
		float& time_since_remove_activity = std::get<float>(*it);
		time_since_remove_activity += delta;

		// if we have not heard for 10sec, timeout
		if (time_since_remove_activity >= 10.f) {
			std::cerr << "SHA1 sending chunk tansfer timed out " << std::get<0>(*it) << ":" << std::get<1>(*it) << "." << std::get<2>(*it) << "\n";
			it = _transfers_sending_chunk.erase(it);
		} else {
			it++;
		}
	}
	// chunk receiving
	for (auto it = _transfers_receiving_chunk.begin(); it != _transfers_receiving_chunk.end();) {
		float& time_since_remove_activity = std::get<float>(*it);
		time_since_remove_activity += delta;

		// if we have not heard for 10sec, timeout
		if (time_since_remove_activity >= 10.f) {
			std::cerr << "SHA1 receiving chunk tansfer timed out " << std::get<0>(*it) << ":" << std::get<1>(*it) << "." << std::get<2>(*it) << "\n";
			it = _transfers_receiving_chunk.erase(it);
		} else {
			it++;
		}
	}

	// if we have not reached the total cap for transfers
	if (true) {
		// for each peer? transfer cap per peer?

		// first check requests for info
		if (!_queue_requested_info.empty()) {
			// send init to _queue_requested_info
			const auto [group_number, peer_number] = _queue_requested_info.front();

			uint8_t transfer_id {0};

			_tcl.sendFT1InitPrivate(
				group_number, peer_number,
				NGC_FT1_file_kind::HASH_SHA1_INFO,
				_sha1_info_hash.data.data(), _sha1_info_hash.size(), // id (info hash)
				_sha1_info_data.size(), // "file_size"
				transfer_id
			);

			_transfers_requested_info.push_back({
				group_number, peer_number,
				transfer_id,
				0.f
			});

			_queue_requested_info.pop_front();
		} else if (!_queue_requested_chunk.empty()) { // then check for chunk requests
			const auto [group_number, peer_number, chunk_hash] = _queue_requested_chunk.front();

			size_t chunk_index = chunkIndex(chunk_hash).value();
			size_t chunk_file_size = _sha1_info.chunk_size;
			if (chunk_index+1 == _sha1_info.chunks.size()) {
				// last chunk
				chunk_file_size = _sha1_info.file_size - chunk_index * _sha1_info.chunk_size;
			}

			uint8_t transfer_id {0};

			_tcl.sendFT1InitPrivate(
				group_number, peer_number,
				NGC_FT1_file_kind::HASH_SHA1_CHUNK,
				chunk_hash.data.data(), chunk_hash.size(), // id (chunk hash)
				chunk_file_size,
				transfer_id
			);

			_transfers_sending_chunk.push_back({
				group_number, peer_number,
				transfer_id,
				0.f,
				chunk_index
			});

			_queue_requested_chunk.pop_front();
		}
	}

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
		std::cout << "SHA1 ignoring different info request " << requested_hash << "\n";
		return;
	}

	// same hash, should respond
	// prio higher then chunks?

	// add to requested queue
	queueUpRequestInfo(group_number, peer_number);
}

bool SHA1::onFT1ReceiveInitSHA1Info(uint32_t, uint32_t, const uint8_t*, size_t, const uint8_t, const size_t) {
	// no, in this state we have init
	return false;
}

void SHA1::onFT1ReceiveDataSHA1Info(uint32_t, uint32_t, uint8_t, size_t, const uint8_t*, size_t) {
	// no, in this state we have init
	assert(false && "ft should have said dropped this for us!");
}

void SHA1::onFT1SendDataSHA1Info(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, uint8_t* data, size_t data_size) {
	// should all be fine
	(void)group_number;
	(void)peer_number;
	(void)transfer_id;

	for (size_t i = 0; i < data_size; i++) {
		data[i] = _sha1_info_data.at(data_offset+i);
	}

	// TODO: sub optimal
	for (auto it = _transfers_requested_info.begin(); it != _transfers_requested_info.end(); it++) {
		if (std::get<0>(*it) == group_number && std::get<1>(*it) == peer_number && std::get<2>(*it) == transfer_id) {
			std::get<float>(*it) = 0.f;
			break;
		}
	}

	// if last data block
	if (data_offset + data_size == _sha1_info_data.size()) {
		// this transfer is "done" (ft still could have to retransfer)
		for (auto it = _transfers_requested_info.cbegin(); it != _transfers_requested_info.cend(); it++) {
			if (std::get<0>(*it) == group_number && std::get<1>(*it) == peer_number && std::get<2>(*it) == transfer_id) {
				std::cout << "SHA1 info tansfer finished " << std::get<0>(*it) << ":" << std::get<1>(*it) << "." << std::get<2>(*it) << "\n";
				_transfers_requested_info.erase(it);
				break;
			}
		}
	}
}

// sha1_chunk
void SHA1::onFT1ReceiveRequestSHA1Chunk(uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size) {
	if (file_id_size != 20) {
		std::cerr << "SHA1 got request for sha1_chunk of wrong size!!\n";
		return;
	}

	SHA1Digest requested_hash(file_id, file_id_size);
	if (!haveChunk(requested_hash)) {
		// we dont have, ignore
		return;
	}

	queueUpRequestChunk(group_number, peer_number, requested_hash);
}

bool SHA1::onFT1ReceiveInitSHA1Chunk(uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size, const uint8_t transfer_id, const size_t file_size) {
	return false;
}

void SHA1::onFT1ReceiveDataSHA1Chunk(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, const uint8_t* data, size_t data_size) {
}

void SHA1::onFT1SendDataSHA1Chunk(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, uint8_t* data, size_t data_size) {
	// TODO: sub optimal
	for (auto it = _transfers_sending_chunk.begin(); it != _transfers_sending_chunk.end(); it++) {
		if (std::get<0>(*it) == group_number && std::get<1>(*it) == peer_number && std::get<2>(*it) == transfer_id) {
			std::get<float>(*it) = 0.f; // time

			const size_t chunk_index = std::get<4>(*it);

			size_t file_offset = chunk_index * _sha1_info.chunk_size;

			// TODO: optimize
			for (size_t i = 0; i < data_size; i++) {
				data[i] = _file_map[file_offset+data_offset+i];
			}

			size_t chunk_file_size = _sha1_info.chunk_size;
			if (chunk_index+1 == _sha1_info.chunks.size()) {
				// last chunk
				chunk_file_size = _sha1_info.file_size - chunk_index * _sha1_info.chunk_size;
			}

			// if last data block
			if (data_offset + data_size == chunk_file_size) {
				std::cout << "SHA1 chunk sent " << std::get<0>(*it) << ":" << std::get<1>(*it) << "." << std::get<2>(*it) << " " << chunk_index << "\n";
				_transfers_sending_chunk.erase(it);
			}

			break;
		}
	}
}

void SHA1::queueUpRequestInfo(uint32_t group_number, uint32_t peer_number) {
	// check ongoing transfers for dup
	for (const auto& it : _transfers_requested_info) {
		// if allready in queue
		if (std::get<0>(it) == group_number && std::get<1>(it) == peer_number) {
			return;
		}
	}

	for (auto& [i_g, i_p] : _queue_requested_info) {
		// if allready in queue
		if (i_g == group_number && i_p == peer_number) {
			return;
		}
	}

	// not in queue yet
	_queue_requested_info.push_back(std::make_pair(group_number, peer_number));
}

void SHA1::queueUpRequestChunk(uint32_t group_number, uint32_t peer_number, const SHA1Digest& hash) {
	// TODO: transfers

	for (auto& [i_g, i_p, i_h] : _queue_requested_chunk) {
		// if allready in queue
		if (i_g == group_number && i_p == peer_number && i_h == hash) {
			return;
		}
	}

	// not in queue yet
	_queue_requested_chunk.push_back(std::make_tuple(group_number, peer_number, hash));
}

std::optional<size_t> SHA1::chunkIndex(const SHA1Digest& hash) const {
	const auto it = _chunk_hash_to_index.find(hash);
	if (it != _chunk_hash_to_index.cend()) {
		return it->second;
	} else {
		return std::nullopt;
	}
}

bool SHA1::haveChunk(const SHA1Digest& hash) const {
	if (_have_all) {
		return true;
	}

	if (auto i_opt = chunkIndex(hash); i_opt.has_value()) {
		return _have_chunk[i_opt.value()];
	}

	// not part of this file
	return false;
}

} // States

