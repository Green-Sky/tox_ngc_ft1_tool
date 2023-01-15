#include "./sha1.hpp"

#include "../tox_client.hpp"

#include <iostream>

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
	for (const bool it : _have_chunk) {
		if (!it) {
			_have_all = false;
			break;
		}
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
		} else if (false) { // then check for chunk requests
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

} // States

