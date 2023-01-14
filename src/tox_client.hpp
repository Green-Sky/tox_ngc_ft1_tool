#pragma once

#include "./command_line.hpp"

#include <string_view>
#include <tox/tox.h>
#include <ngc_ext.h>
#include <ngc_ft1.h>

#include <string>

struct ToxClient {
	public:
		ToxClient(const CommandLine& cl);

		void iterate(void);

		void setToxProfilePath(const std::string& new_path) { _tox_profile_path = new_path; }

		std::string getOwnAddress(void) const;

	public: // tox callbacks
		void onToxSelfConnectionStatus(TOX_CONNECTION connection_status);
		void onToxFriendRequest(const uint8_t* public_key, std::string_view message);
		void onToxGroupCustomPacket(uint32_t group_number, uint32_t peer_id, const uint8_t *data, size_t length);
		void onToxGroupCustomPrivatePacket(uint32_t group_number, uint32_t peer_id, const uint8_t *data, size_t length);
		void onToxGroupInvite(uint32_t friend_number, const uint8_t* invite_data, size_t invite_length, std::string_view group_name);


	private:
		void saveToxProfile(void);

	private:
		Tox* _tox {nullptr};
		NGC_EXT_CTX* _ext_ctx {nullptr};
		NGC_FT1* _ft1_ctx {nullptr};

		std::string _self_name;

		std::string _tox_profile_path;
		bool _tox_profile_dirty {false}; // set in callbacks
};

