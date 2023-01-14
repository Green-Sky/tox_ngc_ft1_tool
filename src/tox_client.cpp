#include "./tox_client.hpp"

#include "./tox_utils.hpp"
#include "./tox_callbacks.hpp"
#include "toxcore/tox.h"

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
	tox_options_set_local_discovery_enabled(options, true);
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

#define CALLBACK_REG(x) tox_callback_##x(_tox, x##_cb)
	CALLBACK_REG(self_connection_status);

	//CALLBACK_REG(friend_name);
	//CALLBACK_REG(friend_status_message);
	//CALLBACK_REG(friend_status);
	//CALLBACK_REG(friend_connection_status);
	//CALLBACK_REG(friend_typing);
	//CALLBACK_REG(friend_read_receipt);
	CALLBACK_REG(friend_request);
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

	CALLBACK_REG(group_custom_packet);
	CALLBACK_REG(group_custom_private_packet);
	CALLBACK_REG(group_invite);
#undef CALLBACK_REG

	if (_self_name.empty()) {
		_self_name = "tox_ngc_ft1_tool";
	}
	tox_self_set_name(_tox, reinterpret_cast<const uint8_t*>(_self_name.data()), _self_name.size(), nullptr);

	_tox_profile_dirty = true;
}

void ToxClient::iterate(void) {
	tox_iterate(_tox, this);

	if (_tox_profile_dirty) {
		saveToxProfile();
	}
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
}

void ToxClient::onToxGroupCustomPrivatePacket(uint32_t group_number, uint32_t peer_id, const uint8_t *data, size_t length) {
}

void ToxClient::onToxGroupInvite(uint32_t friend_number, const uint8_t* invite_data, size_t invite_length, std::string_view group_name) {
	std::cout << "TCL accepting group invite (" << group_name << ")\n";

	tox_group_invite_accept(_tox, friend_number, invite_data, invite_length, reinterpret_cast<const uint8_t*>(_self_name.data()), _self_name.size(), nullptr, 0, nullptr);
	_tox_profile_dirty = true;
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

