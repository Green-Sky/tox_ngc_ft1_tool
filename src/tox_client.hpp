#pragma once

#include "./command_line.hpp"

#include "./state.hpp"

#include <tox/tox.h>
#include <ngc_ext.h>
#include <ngc_ft1.h>

#include <chrono>
#include <string>
#include <string_view>
#include <map>
#include <memory>
#include <set>
#include <vector>

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

		std::string_view getGroupPeerName(uint32_t group_number, uint32_t peer_number) const;
		TOX_CONNECTION getGroupPeerConnectionStatus(uint32_t group_number, uint32_t peer_number) const;

		template<typename FN>
		void forEachGroup(FN&& fn) const {
			for (const auto& it : _groups) {
				fn(it.first);
			}
		}

		template<typename FN>
		void forEachGroupPeer(uint32_t group_number, FN&& fn) const {
			if (_groups.count(group_number)) {
				for (const auto& [peer_number, peer] : _groups.at(group_number)) {
					fn(peer_number);
				}
			}
		}

	public: // tox callbacks
		void onToxSelfConnectionStatus(TOX_CONNECTION connection_status);
		void onToxFriendRequest(const uint8_t* public_key, std::string_view message);
		void onToxGroupPeerName(uint32_t group_number, uint32_t peer_id, std::string_view name);
		//void onToxGroupPeerConnection(uint32_t group_number, uint32_t peer_id, TOX_CONNECTION connection_status);
		void onToxGroupCustomPacket(uint32_t group_number, uint32_t peer_id, const uint8_t *data, size_t length);
		void onToxGroupCustomPrivatePacket(uint32_t group_number, uint32_t peer_id, const uint8_t *data, size_t length);
		void onToxGroupInvite(uint32_t friend_number, const uint8_t* invite_data, size_t invite_length, std::string_view group_name);
		void onToxGroupPeerJoin(uint32_t group_number, uint32_t peer_id);
		void onToxGroupPeerExit(uint32_t group_number, uint32_t peer_id, Tox_Group_Exit_Type exit_type, std::string_view name, std::string_view part_message);
		void onToxGroupSelfJoin(uint32_t group_number);

	public: // FT1 callbacks
		StateI& getState(void); // public accessor for callbacks

	public: // FT1 sends
		bool sendFT1RequestPrivate(uint32_t group_number, uint32_t peer_number, uint32_t file_kind, const uint8_t* file_id, size_t file_id_size);
		bool sendFT1InitPrivate(uint32_t group_number, uint32_t peer_number, uint32_t file_kind, const uint8_t* file_id, size_t file_id_size, uint64_t file_size, uint8_t& transfer_id);

	private:
		void saveToxProfile(void);

	private:
		Tox* _tox {nullptr};
		NGC_EXT_CTX* _ext_ctx {nullptr};
		NGC_FT1* _ft1_ctx {nullptr};

		std::chrono::time_point<std::chrono::high_resolution_clock> _last_time {std::chrono::high_resolution_clock::now()};

		std::string _self_name;

		std::string _tox_profile_path;
		bool _tox_profile_dirty {false}; // set in callbacks

		std::vector<uint8_t> _join_group_after_dht_connect;

		std::unique_ptr<StateI> _state;

		struct Peer {
			std::string name;
		};
		// key groupid, key peerid
		std::map<uint32_t, std::map<uint32_t, Peer>> _groups;
};

