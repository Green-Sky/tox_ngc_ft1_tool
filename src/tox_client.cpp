#include "./tox_client.hpp"

#include "./tox_utils.hpp"
#include "./tox_callbacks.hpp"

#include "./states/send_start_sha1.hpp"
#include "./states/receive_start_sha1.hpp"
#include "ngc_ft1.h"

#include <memory>
#include <sodium.h>

#include <vector>
#include <fstream>
#include <cassert>
#include <iostream>
#include <stdexcept>

ToxClient::ToxClient(const CommandLine& cl) :
	_self_name(cl.self_name),
	_tox_profile_path(cl.profile_path)
{
	TOX_ERR_OPTIONS_NEW err_opt_new;
	Tox_Options* options = tox_options_new(&err_opt_new);
	assert(err_opt_new == TOX_ERR_OPTIONS_NEW::TOX_ERR_OPTIONS_NEW_OK);

	// use cl for options
	tox_options_set_log_callback(options, log_cb);
	tox_options_set_local_discovery_enabled(options, false);
	tox_options_set_udp_enabled(options, true);
	tox_options_set_hole_punching_enabled(options, true);

	std::vector<uint8_t> profile_data{};
	if (!_tox_profile_path.empty()) {
		std::ifstream ifile{_tox_profile_path, std::ios::binary};

		if (ifile.is_open()) {
			std::cout << "TOX loading save " << _tox_profile_path << "\n";
			// fill savedata
			while (ifile.good()) {
				auto ch = ifile.get();
				if (ch == EOF) {
					break;
				} else {
					profile_data.push_back(ch);
				}
			}

			if (profile_data.empty()) {
				std::cerr << "empty tox save\n";
			} else {
				// set options
				tox_options_set_savedata_type(options, TOX_SAVEDATA_TYPE_TOX_SAVE);
				tox_options_set_savedata_data(options, profile_data.data(), profile_data.size());
			}

			ifile.close(); // do i need this?
		}
	}

	TOX_ERR_NEW err_new;
	_tox = tox_new(options, &err_new);
	tox_options_free(options);
	if (err_new != TOX_ERR_NEW_OK) {
		std::cerr << "tox_new failed with error code " << err_new << "\n";
		throw std::runtime_error{"tox failed"};
	}

#define TOX_CALLBACK_REG(x) tox_callback_##x(_tox, x##_cb)
	TOX_CALLBACK_REG(self_connection_status);

	//CALLBACK_REG(friend_name);
	//CALLBACK_REG(friend_status_message);
	//CALLBACK_REG(friend_status);
	//CALLBACK_REG(friend_connection_status);
	//CALLBACK_REG(friend_typing);
	//CALLBACK_REG(friend_read_receipt);
	TOX_CALLBACK_REG(friend_request);
	//CALLBACK_REG(friend_message);

	//CALLBACK_REG(file_recv_control);
	//CALLBACK_REG(file_chunk_request);
	//CALLBACK_REG(file_recv);
	//CALLBACK_REG(file_recv_chunk);

	//CALLBACK_REG(conference_invite);
	//CALLBACK_REG(conference_connected);
	//CALLBACK_REG(conference_message);
	//CALLBACK_REG(conference_title);
	//CALLBACK_REG(conference_peer_name);
	//CALLBACK_REG(conference_peer_list_changed);

	//CALLBACK_REG(friend_lossy_packet);
	//CALLBACK_REG(friend_lossless_packet);

	TOX_CALLBACK_REG(group_custom_packet);
	TOX_CALLBACK_REG(group_custom_private_packet);
	TOX_CALLBACK_REG(group_invite);
	TOX_CALLBACK_REG(group_peer_join);
	TOX_CALLBACK_REG(group_peer_exit);
	TOX_CALLBACK_REG(group_self_join);
#undef TOX_CALLBACK_REG

	if (_self_name.empty()) {
		_self_name = "tox_ngc_ft1_tool";
	}
	tox_self_set_name(_tox, reinterpret_cast<const uint8_t*>(_self_name.data()), _self_name.size(), nullptr);

	_ext_ctx = NGC_EXT_new();

	NGC_FT1_options ft1_options {};
	_ft1_ctx = NGC_FT1_new(&ft1_options);
	NGC_FT1_register_ext(_ft1_ctx, _ext_ctx);

	// sha1_info
	NGC_FT1_register_callback_recv_request(_ft1_ctx, NGC_FT1_file_kind::HASH_SHA1_INFO, ft1_recv_request_sha1_info_cb, this);
	NGC_FT1_register_callback_recv_init(_ft1_ctx, NGC_FT1_file_kind::HASH_SHA1_INFO, ft1_recv_init_sha1_info_cb, this);
	NGC_FT1_register_callback_recv_data(_ft1_ctx, NGC_FT1_file_kind::HASH_SHA1_INFO, ft1_recv_data_sha1_info_cb, this);
	NGC_FT1_register_callback_send_data(_ft1_ctx, NGC_FT1_file_kind::HASH_SHA1_INFO, ft1_send_data_sha1_info_cb, this);

	// sha1_chunk
	NGC_FT1_register_callback_recv_request(_ft1_ctx, NGC_FT1_file_kind::HASH_SHA1_CHUNK, ft1_recv_request_sha1_chunk_cb, this);
	NGC_FT1_register_callback_recv_init(_ft1_ctx, NGC_FT1_file_kind::HASH_SHA1_CHUNK, ft1_recv_init_sha1_chunk_cb, this);
	NGC_FT1_register_callback_recv_data(_ft1_ctx, NGC_FT1_file_kind::HASH_SHA1_CHUNK, ft1_recv_data_sha1_chunk_cb, this);
	NGC_FT1_register_callback_send_data(_ft1_ctx, NGC_FT1_file_kind::HASH_SHA1_CHUNK, ft1_send_data_sha1_chunk_cb, this);

	{ // state factory // TODO: extract
		// sender
		if (!cl.send_path.empty()) {
			_state = std::make_unique<States::SendStartSHA1>(*this, cl);
		} else { // receiver
			_state = std::make_unique<States::ReceiveStartSHA1>(*this, cl);
		}
	}

	// dht bootstrap
	{
		struct DHT_node {
			const char *ip;
			uint16_t port;
			const char key_hex[TOX_PUBLIC_KEY_SIZE*2 + 1]; // 1 for null terminator
			unsigned char key_bin[TOX_PUBLIC_KEY_SIZE];
		};

		DHT_node nodes[] =
		{
			// you can change or add your own bs and tcprelays here, ideally closer to you
			{"tox.plastiras.org",	443,	"8E8B63299B3D520FB377FE5100E65E3322F7AE5B20A0ACED2981769FC5B43725", {}}, // LU tha14
			{"tox2.plastiras.org",	33445,	"B6626D386BE7E3ACA107B46F48A5C4D522D29281750D44A0CBA6A2721E79C951", {}}, // DE tha14

		};

		for (size_t i = 0; i < sizeof(nodes)/sizeof(DHT_node); i ++) {
			sodium_hex2bin(
				nodes[i].key_bin, sizeof(nodes[i].key_bin),
				nodes[i].key_hex, sizeof(nodes[i].key_hex)-1,
				NULL, NULL, NULL
			);
			tox_bootstrap(_tox, nodes[i].ip, nodes[i].port, nodes[i].key_bin, NULL);
			// TODO: use extra tcp option to avoid error msgs
			// ... this is hardcore
			tox_add_tcp_relay(_tox, nodes[i].ip, nodes[i].port, nodes[i].key_bin, NULL);
		}
	}

	_tox_profile_dirty = true;
}

ToxClient::~ToxClient(void) {
	NGC_FT1_kill(_ft1_ctx);
	NGC_EXT_kill(_ext_ctx);

	tox_kill(_tox);
}

bool ToxClient::iterate(void) {
	tox_iterate(_tox, this);
	NGC_FT1_iterate(_tox, _ft1_ctx);

	// HACK: hardcoded 5ms sleep in main
	if (_state->iterate(0.005f)) {
		_state = _state->nextState();

		if (!_state) {
			// exit program
			return false;
		}
	}

	if (_tox_profile_dirty) {
		saveToxProfile();
	}

	return true;
}

std::string ToxClient::getOwnAddress(void) const {
	std::vector<uint8_t> self_addr{};
	self_addr.resize(TOX_ADDRESS_SIZE);

	tox_self_get_address(_tox, self_addr.data());

	return bin2hex(self_addr);
}

void ToxClient::onToxSelfConnectionStatus(TOX_CONNECTION connection_status) {
	std::cout << "TCL self status: ";
	switch (connection_status) {
		case TOX_CONNECTION::TOX_CONNECTION_NONE: std::cout << "offline\n"; break;
		case TOX_CONNECTION::TOX_CONNECTION_TCP: std::cout << "TCP-relayed\n"; break;
		case TOX_CONNECTION::TOX_CONNECTION_UDP: std::cout << "UDP-direct\n"; break;
	}
	_tox_profile_dirty = true;
}

void ToxClient::onToxFriendRequest(const uint8_t* public_key, std::string_view message) {
	std::vector<uint8_t> key(public_key, public_key + TOX_PUBLIC_KEY_SIZE);
	std::cout << "TCL adding friend " << bin2hex(key) << " (" << message << ")\n";

	tox_friend_add_norequest(_tox, public_key, nullptr);
	_tox_profile_dirty = true;
}

void ToxClient::onToxGroupCustomPacket(uint32_t group_number, uint32_t peer_id, const uint8_t *data, size_t length) {
	// TODO: signal private?
	NGC_EXT_handle_group_custom_packet(_tox, _ext_ctx, group_number, peer_id, data, length);
}

void ToxClient::onToxGroupCustomPrivatePacket(uint32_t group_number, uint32_t peer_id, const uint8_t *data, size_t length) {
	NGC_EXT_handle_group_custom_packet(_tox, _ext_ctx, group_number, peer_id, data, length);
}

void ToxClient::onToxGroupInvite(uint32_t friend_number, const uint8_t* invite_data, size_t invite_length, std::string_view group_name) {
	std::cout << "TCL accepting group invite (" << group_name << ")\n";

	uint32_t new_group_number = tox_group_invite_accept(_tox, friend_number, invite_data, invite_length, reinterpret_cast<const uint8_t*>(_self_name.data()), _self_name.size(), nullptr, 0, nullptr);
	_groups[new_group_number] = {};
	_tox_profile_dirty = true;
}

void ToxClient::onToxGroupPeerJoin(uint32_t group_number, uint32_t peer_id) {
	std::cout << "TCL group peer join " << group_number << ":" << peer_id << "\n";
	_groups[group_number].emplace(peer_id);
	_tox_profile_dirty = true;
}

void ToxClient::onToxGroupPeerExit(uint32_t group_number, uint32_t peer_id, Tox_Group_Exit_Type exit_type, std::string_view name, std::string_view part_message) {
	std::cout << "TCL group peer exit " << group_number << ":" << peer_id << "\n";
	_groups[group_number].erase(peer_id);
	_tox_profile_dirty = true;
}

void ToxClient::onToxGroupSelfJoin(uint32_t group_number) {
	std::cout << "TCL group self join " << group_number << "\n";
	// ???
	// can be triggered after other peers allready joined o.o
	_tox_profile_dirty = true;
}


StateI& ToxClient::getState(void) {
	assert(_state.get());
	return *_state.get();
}

bool ToxClient::sendFT1RequestPrivate(uint32_t group_number, uint32_t peer_number, NGC_FT1_file_kind file_kind, const uint8_t* file_id, size_t file_id_size) {
	NGC_FT1_send_request_private(
		_tox, _ft1_ctx,
		group_number, peer_number,
		file_kind,
		file_id, file_id_size
	);

	// TODO: return value
	return true;
}

bool ToxClient::sendFT1InitPrivate(uint32_t group_number, uint32_t peer_number, NGC_FT1_file_kind file_kind, const uint8_t* file_id, size_t file_id_size, uint64_t file_size, uint8_t& transfer_id) {
	return NGC_FT1_send_init_private(
		_tox, _ft1_ctx,
		group_number, peer_number,
		file_kind,
		file_id, file_id_size,
		file_size,
		&transfer_id
	);
}

void ToxClient::saveToxProfile(void) {
	if (_tox_profile_path.empty()) {
		return;
	}

	std::vector<uint8_t> data{};
	data.resize(tox_get_savedata_size(_tox));
	tox_get_savedata(_tox, data.data());

	std::ofstream ofile{_tox_profile_path, std::ios::binary};
	// TODO: improve
	for (const auto& ch : data) {
		ofile.put(ch);
	}
	ofile.close(); // TODO: do i need this

	_tox_profile_dirty = false;
}

