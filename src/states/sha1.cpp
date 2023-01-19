#include "./sha1.hpp"

#include "../tox_client.hpp"

#include "../hash_utils.hpp"

#include <algorithm>
#include <iostream>
#include <tuple>
#include <random>

namespace States {

SHA1::SHA1(
	ToxClient& tcl,
	const CommandLine& cl,
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
	assert(_have_chunk.size() == _sha1_info.chunks.size());

	_max_concurrent_in = cl.max_incoming_transfers;
	_max_concurrent_out = cl.max_incoming_transfers;

	// build lookup table
	for (size_t i = _sha1_info.chunks.size(); i > 0; i--) {
		// chunks can have more then 1 index ..., for now, build reverse and have the first index be the real index
		_chunk_hash_to_index[_sha1_info.chunks.at(i-1)] = i-1;
	}

	_have_all = true;
	_have_count = 0;
	for (size_t i = 0; i < _have_chunk.size(); i++) {
		if (_have_chunk[i]) {
			_have_count++;
		} else {
			_have_all = false;

			// avoid same chunk hash dups
			if (_chunk_hash_to_index.at(_sha1_info.chunks.at(i)) == i) {
				_chunk_want_queue.push_back(i);
			}
		}
	}

	if (!_have_all) {
		assert(_chunk_want_queue.size() > 0);

		{ // load last chunk first :)
			size_t tmp = _chunk_want_queue.back();
			_chunk_want_queue.push_front(tmp);
			_chunk_want_queue.pop_back();
		}

		// if not sequential, shuffle _chunk_want_queue
	}
}

bool SHA1::iterate(float delta) {
	{ // timer and timeouts
		// info
		for (auto it = _transfers_requested_info.begin(); it != _transfers_requested_info.end();) {
			float& time_since_remove_activity = std::get<float>(*it);
			time_since_remove_activity += delta;

			// if we have not heard for 10sec, timeout
			if (time_since_remove_activity >= 10.f) {
				std::cerr << "SHA1 info tansfer timed out " << std::get<0>(*it) << ":" << std::get<1>(*it) << "." << int(std::get<2>(*it)) << "\n";
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
				std::cerr << "SHA1 sending chunk tansfer timed out " << std::get<0>(*it) << ":" << std::get<1>(*it) << "." << int(std::get<2>(*it)) << "\n";
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
				std::cerr << "SHA1 receiving chunk tansfer timed out " << std::get<0>(*it) << ":" << std::get<1>(*it) << "." << int(std::get<2>(*it)) << "\n";
				_chunk_want_queue.push_front(std::get<4>(*it)); // put it back
				it = _transfers_receiving_chunk.erase(it);
			} else {
				it++;
			}
		}
		// sent requests
		for (auto it = _chunks_requested.begin(); it != _chunks_requested.end();) {
			it->second += delta;

			// if we have not heard for 15sec, timeout
			if (it->second >= 15.f) {
				_chunk_want_queue.push_front(it->first); // put it back
				it = _chunks_requested.erase(it);
			} else {
				it++;
			}
		}

		// queued requests
		for (auto it = _queue_requested_chunk.begin(); it != _queue_requested_chunk.end();) {
			float& timer = std::get<float>(*it);
			timer += delta;

			if (timer >= 10.f) {
				it = _queue_requested_chunk.erase(it);
			} else {
				it++;
			}
		}
	}

	// if we have not reached the total cap for transfers
	if (_transfers_requested_info.size() + _transfers_sending_chunk.size() < _max_concurrent_out) {
		// for each peer? transfer cap per peer?

		// first check requests for info
		if (!_queue_requested_info.empty()) {
			// send init to _queue_requested_info
			const auto [group_number, peer_number] = _queue_requested_info.front();

			uint8_t transfer_id {0};

			if (_tcl.sendFT1InitPrivate(
				group_number, peer_number,
				NGC_FT1_file_kind::HASH_SHA1_INFO,
				_sha1_info_hash.data.data(), _sha1_info_hash.size(), // id (info hash)
				_sha1_info_data.size(), // "file_size"
				transfer_id
			)) {
				_transfers_requested_info.push_back({
					group_number, peer_number,
					transfer_id,
					0.f
				});

				_queue_requested_info.pop_front();
			}
		} else if (!_queue_requested_chunk.empty()) { // then check for chunk requests
			const auto [group_number, peer_number, chunk_hash, _] = _queue_requested_chunk.front();

			size_t chunk_index = chunkIndex(chunk_hash).value();
			size_t chunk_file_size = chunkSize(chunk_index);

			uint8_t transfer_id {0};

			if (_tcl.sendFT1InitPrivate(
				group_number, peer_number,
				NGC_FT1_file_kind::HASH_SHA1_CHUNK,
				chunk_hash.data.data(), chunk_hash.size(), // id (chunk hash)
				chunk_file_size,
				transfer_id
			)) {
				_transfers_sending_chunk.push_back({
					group_number, peer_number,
					transfer_id,
					0.f,
					chunk_index
				});

				_queue_requested_chunk.pop_front();
			}
		}
	}

	// update speeds and targets
	_peer_speed_mesurement_interval_timer += delta;
	if (_peer_speed_mesurement_interval_timer >= _peer_speed_mesurement_interval) {
		_peer_speed_mesurement_interval_timer = 0.f; // we lose some time here, but precision is not the issue

		_peer_in_bytes_array_index = (_peer_in_bytes_array_index + 1) % _peer_speed_mesurement_interval_count;
		for (const auto& [peer, array] : _peer_in_bytes_array) {
			float avg {0.f};
			for (size_t i = 0; i < array.size(); i++) {
				avg += array[i];
			}

			// if 6 mesurment every 0.5sec -> avg is over 3sec -> /3 for /s
			avg /= _peer_speed_mesurement_interval * _peer_speed_mesurement_interval_count;

			// reset byte count for next round
			_peer_in_bytes_array[peer][_peer_in_bytes_array_index] = 0;

			_peer_in_speed[peer] = avg;
		}

		_peer_in_targets.clear();
		_tcl.forEachGroup([this](uint32_t group_number) {
			_tcl.forEachGroupPeer(group_number, [group_number, this](uint32_t peer_number, Tox_Connection connection_status) {
				if (connection_status == Tox_Connection::TOX_CONNECTION_UDP || !_udp_only) {
					_peer_in_targets.push_back({group_number, peer_number});
				}
			});
		});

		if (!_peer_in_targets.empty()) {
			std::vector<double> weights;
			for (const auto& peer : _peer_in_targets) {
				if (_peer_in_speed.count(peer)) {
					weights.push_back(
						std::clamp(
							(_peer_in_speed.at(peer) / 1024.f) // KiB/s
							* (20.f/500.f), // map to a range from 0 to 20, max at 500KiB/s
							1.f,
							20.f
						)
					);
				} else {
					weights.push_back(1.f);
				}
			}

			std::discrete_distribution<size_t> tmp_dist{weights.cbegin(), weights.cend()};
			_peer_in_targets_dist.param(tmp_dist.param());
		}
	}

	if (!_have_all && !_peer_in_targets.empty() && !_chunk_want_queue.empty() && _chunks_requested.size() + _transfers_receiving_chunk.size() < _max_concurrent_in) {
		// send out request, no burst tho
		uint32_t group_number;
		uint32_t peer_number;

		//size_t target_index = _rng()%target_peers.size();
		size_t target_index = _peer_in_targets_dist(_rng);
		std::tie(group_number, peer_number) = _peer_in_targets.at(target_index);

		size_t chunk_index = _chunk_want_queue.front();
		_chunks_requested[chunk_index] = 0.f;
		_chunk_want_queue.pop_front();

		_tcl.sendFT1RequestPrivate(group_number, peer_number, NGC_FT1_file_kind::HASH_SHA1_CHUNK, _sha1_info.chunks[chunk_index].data.data(), 20);
		//std::cout << "sent request " << group_number << ":" << peer_number << "\n";
	}

	// log
	_io_log_timer += delta;
	static const float log_interval {10.f};
	if (_io_log_timer >= log_interval) {
		_io_log_timer = 0.f;

		size_t bytes_up_since {_bytes_up - _bytes_up_last_log};
		_bytes_up_last_log = _bytes_up;

		size_t bytes_down_since {_bytes_down - _bytes_down_last_log};
		_bytes_down_last_log = _bytes_down;

		float up_kibs {(bytes_up_since / 1024.f) / log_interval};
		float down_kibs {(bytes_down_since / 1024.f) / log_interval};

		std::cout << "SHA1 speed down: " << down_kibs << "KiB/s up: " << up_kibs << "KiB/s\n";
		std::cout << "SHA1 total down: " << _bytes_down / 1024 << "KiB   up: " << _bytes_up / 1024 << "KiB\n";

		std::cout << "SHA1 cwq:" << _chunk_want_queue.size() << " cwqr:" << _chunks_requested.size() << " trc:" << _transfers_receiving_chunk.size() << " tsc:" << _transfers_sending_chunk.size() << "\n";
		std::cout << "SHA1 peer down speeds:\n";
		for (const auto& [peer, speed] : _peer_in_speed) {
			std::cout << "    " << peer.first << ":" << peer.second << "(" << _tcl.getGroupPeerName(peer.first, peer.second) << ")" << "\t" << speed / 1024.f << "KiB/s\n";
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
	assert(false && "ft should have dropped this for us!");
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
				std::cout << "SHA1 info tansfer finished " << std::get<0>(*it) << ":" << std::get<1>(*it) << "." << int(std::get<2>(*it)) << "\n";
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
	if (_transfers_receiving_chunk.size() >= _max_concurrent_in) {
		// reject, max tf in
		return false;
	}

	if (file_id_size != 20) {
		std::cerr << "SHA1 got request for sha1_chunk of wrong size!!\n";
		return false;
	}

	SHA1Digest incomming_hash(file_id, file_id_size);

	if (haveChunk(incomming_hash)) {
		std::cout << "SHA1 ignoring init for chunk we already have " << incomming_hash << "\n";
		return false;
	}

	auto chunk_i_opt = chunkIndex(incomming_hash);

	if (!chunk_i_opt.has_value()) {
		std::cout << "SHA1 ignoring init for unrelated chunk " << incomming_hash << "\n";
		return false;
	}

	size_t chunk_index = chunk_i_opt.value();

	// check transfers
	for (const auto& it : _transfers_receiving_chunk) {
		if (std::get<4>(it) == chunk_index) {
			// already in transition
			return false;
		}
	}

	_transfers_receiving_chunk.push_back(
		std::make_tuple(
			group_number, peer_number,
			transfer_id,
			0.f,
			chunk_index
		)
	);

	// remove form requests
	_chunks_requested.erase(chunk_index);

	return true;
}

void SHA1::onFT1ReceiveDataSHA1Chunk(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, const uint8_t* data, size_t data_size) {
	// check transfers
	for (auto it =  _transfers_receiving_chunk.begin(); it != _transfers_receiving_chunk.end(); it++) {
		if (std::get<0>(*it) == group_number && std::get<1>(*it) == peer_number && std::get<2>(*it) == transfer_id) {
			_bytes_down += data_size;

			_peer_in_bytes_array[std::make_pair(group_number, peer_number)][_peer_in_bytes_array_index] += data_size;

			std::get<float>(*it) = 0.f; // time

			const size_t chunk_index = std::get<4>(*it);

			size_t file_offset = chunk_index * _sha1_info.chunk_size;

			// TODO: optimize
			for (size_t i = 0; i < data_size; i++) {
				_file_map[file_offset+data_offset+i] = data[i];
			}

			size_t chunk_file_size = chunkSize(chunk_index);

			// if last data block
			if (data_offset + data_size == chunk_file_size) {
				// hash and verify
				SHA1Digest test_hash = hash_sha1(_file_map.data()+file_offset, chunk_file_size);
				if (test_hash != _sha1_info.chunks[chunk_index]) {
					std::cerr << "SHA1 received chunks's hash does not match!, discarding\n";
					_bytes_down -= chunk_file_size; // penalize wrong data
					_transfers_receiving_chunk.erase(it);
					_chunk_want_queue.push_front(chunk_index); // put back in queue
					break;
				}

				_have_chunk[chunk_index] = true;
				_have_count++;
				_have_all = _have_count == _sha1_info.chunks.size();

				std::cout << "SHA1 chunk received " << std::get<0>(*it) << ":" << std::get<1>(*it) << "." << int(std::get<2>(*it)) << " " << chunk_index << " (" <<  100.f * float(_have_count) / _sha1_info.chunks.size() << "%)\n";
				_transfers_receiving_chunk.erase(it);
			}
			break;
		}
	}
}

void SHA1::onFT1SendDataSHA1Chunk(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, uint8_t* data, size_t data_size) {
	// TODO: sub optimal
	for (auto it = _transfers_sending_chunk.begin(); it != _transfers_sending_chunk.end(); it++) {
		if (std::get<0>(*it) == group_number && std::get<1>(*it) == peer_number && std::get<2>(*it) == transfer_id) {
			_bytes_up += data_size;

			std::get<float>(*it) = 0.f; // time

			const size_t chunk_index = std::get<4>(*it);

			size_t file_offset = chunk_index * _sha1_info.chunk_size;

			// TODO: optimize
			for (size_t i = 0; i < data_size; i++) {
				data[i] = _file_map[file_offset+data_offset+i];
			}

			// if last data block
			if (data_offset + data_size == chunkSize(chunk_index)) {
				std::cout << "SHA1 chunk sent " << std::get<0>(*it) << ":" << std::get<1>(*it) << "." << int(std::get<2>(*it)) << " " << chunk_index << "\n";
				_transfers_sending_chunk.erase(it);
			}

			break;
		}
	}
}

void SHA1::queueUpRequestInfo(uint32_t group_number, uint32_t peer_number) {
	// check ongoing transfers for dup
	for (const auto& it : _transfers_requested_info) {
		// if already in queue
		if (std::get<0>(it) == group_number && std::get<1>(it) == peer_number) {
			return;
		}
	}

	for (auto& [i_g, i_p] : _queue_requested_info) {
		// if already in queue
		if (i_g == group_number && i_p == peer_number) {
			return;
		}
	}

	// not in queue yet
	_queue_requested_info.push_back(std::make_pair(group_number, peer_number));
}

void SHA1::queueUpRequestChunk(uint32_t group_number, uint32_t peer_number, const SHA1Digest& hash) {
	// TODO: transfers

	for (auto& [i_g, i_p, i_h, i_t] : _queue_requested_chunk) {
		// if already in queue
		if (i_g == group_number && i_p == peer_number && i_h == hash) {
			// update timer
			i_t = 0.f;
			return;
		}
	}

	// not in queue yet
	_queue_requested_chunk.push_back(std::make_tuple(group_number, peer_number, hash, 0.f));
}

std::optional<size_t> SHA1::chunkIndex(const SHA1Digest& hash) const {
	const auto it = _chunk_hash_to_index.find(hash);
	if (it != _chunk_hash_to_index.cend()) {
		return it->second;
	} else {
		return std::nullopt;
	}
}

size_t SHA1::chunkSize(size_t chunk_index) const {
	if (chunk_index+1 == _sha1_info.chunks.size()) {
		// last chunk
		return _sha1_info.file_size - chunk_index * _sha1_info.chunk_size;
	} else {
		return _sha1_info.chunk_size;
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

