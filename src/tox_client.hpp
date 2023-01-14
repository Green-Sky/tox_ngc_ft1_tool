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
namespace SendStates {
	struct SHA1Start;
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
#if 0
		// sha1_info
		void onFT1ReceiveRequestSHA1Info(uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size);
		bool onFT1ReceiveInitSHA1Info(uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size, const uint8_t transfer_id, const size_t file_size);
		void onFT1ReceiveDataSHA1Info(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, const uint8_t* data, size_t data_size);
		void onFT1SendDataSHA1Info(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, uint8_t* data, size_t data_size);

		// sha1_chunk
		void onFT1ReceiveRequestSHA1Chunk(uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size);
		bool onFT1ReceiveInitSHA1Chunk(uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size, const uint8_t transfer_id, const size_t file_size);
		void onFT1ReceiveDataSHA1Chunk(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, const uint8_t* data, size_t data_size);
		void onFT1SendDataSHA1Chunk(uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, uint8_t* data, size_t data_size);
#endif

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

		// TODO: this is a hack, make better?
		friend SendStates::SHA1Start;
};

