#include "./tox_client.hpp"

#include "./tox_utils.hpp"

#include <vector>
#include <fstream>
#include <cassert>
#include <iostream>
#include <stdexcept>

ToxClient::ToxClient(const CommandLine& cl) :
	_tox_profile_path(cl.profile_path)
{
	TOX_ERR_OPTIONS_NEW err_opt_new;
	Tox_Options* options = tox_options_new(&err_opt_new);
	assert(err_opt_new == TOX_ERR_OPTIONS_NEW::TOX_ERR_OPTIONS_NEW_OK);

	// use cl for options
	//tox_options_set_log_callback(options, log_cb);
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

