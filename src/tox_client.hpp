#pragma once

#include "./command_line.hpp"

#include "./state.hpp"

#include <memory>
#include <tox/tox.h>
#include <ngc_ext.h>
#include <ngc_ft1.h>

#include <string>
#include <string_view>

// fwd
namespace States {
	struct SendStartSHA1;
}

struct ToxClient {
	public:
		ToxClient(const CommandLine& cl);
		~ToxClient(void);

		// returns false when we shoul stop the program
		bool iterate(void);

		void setToxProfilePath(const std::string& new_path) { _tox_profile_path = new_path; }

		std::string getOwnAddress(void) const;

	public: // tox callbacks
		void onToxSelfConnectionStatus(TOX_CONNECTION connection_status);
		void onToxFriendRequest(const uint8_t* public_key, std::string_view message);
		void onToxGroupCustomPacket(uint32_t group_number, uint32_t peer_id, const uint8_t *data, size_t length);
		void onToxGroupCustomPrivatePacket(uint32_t group_number, uint32_t peer_id, const uint8_t *data, size_t length);
		void onToxGroupInvite(uint32_t friend_number, const uint8_t* invite_data, size_t invite_length, std::string_view group_name);
		void onToxGroupPeerJoin(uint32_t group_number, uint32_t peer_id);
		void onToxGroupPeerExit(uint32_t group_number, uint32_t peer_id, Tox_Group_Exit_Type exit_type, std::string_view name, std::string_view part_message);
		void onToxGroupSelfJoin(uint32_t group_number);

	public: // FT1 callbacks
		StateI& getState(void); // public accessor for callbacks

	public: // FT1 sends
		bool sendFT1RequestPrivate(uint32_t group_number, uint32_t peer_number, NGC_FT1_file_kind file_kind, const uint8_t* file_id, size_t file_id_size);
		bool sendFT1InitPrivate(uint32_t group_number, uint32_t peer_number, NGC_FT1_file_kind file_kind, const uint8_t* file_id, size_t file_id_size, uint64_t file_size, uint8_t& transfer_id);

	private:
		void saveToxProfile(void);

	private:
		Tox* _tox {nullptr};
		NGC_EXT_CTX* _ext_ctx {nullptr};
		NGC_FT1* _ft1_ctx {nullptr};

		std::string _self_name;

		std::string _tox_profile_path;
		bool _tox_profile_dirty {false}; // set in callbacks

		std::unique_ptr<StateI> _state;
};

