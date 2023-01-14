#include "./tox_callbacks.hpp"

#include "./tox_client.hpp"

#include <string_view>
#include <iostream>

// TODO: maybe a dispatch table is better?

// logging
void log_cb(Tox*, TOX_LOG_LEVEL level, const char *file, uint32_t line, const char *func, const char *message, void *user_data) {
	//ToxClient* client = static_cast<ToxClient*>(user_data);
	std::cerr << "TOX " << level << " " << file << ":" << line << "(" << func << ") " << message << "\n";
}

// self
void self_connection_status_cb(Tox*, TOX_CONNECTION connection_status, void *user_data) {
	static_cast<ToxClient*>(user_data)->onToxSelfConnectionStatus(connection_status);
}

void friend_request_cb(Tox*, const uint8_t *public_key, const uint8_t *message, size_t length, void *user_data) {
	static_cast<ToxClient*>(user_data)->onToxFriendRequest(public_key, std::string_view{reinterpret_cast<const char*>(message), length});
}
void group_custom_packet_cb(Tox*, uint32_t group_number, uint32_t peer_id, const uint8_t *data, size_t length, void *user_data) {
	static_cast<ToxClient*>(user_data)->onToxGroupCustomPacket(group_number, peer_id, data, length);
}

void group_custom_private_packet_cb(Tox*, uint32_t group_number, uint32_t peer_id, const uint8_t *data, size_t length, void *user_data) {
	static_cast<ToxClient*>(user_data)->onToxGroupCustomPrivatePacket(group_number, peer_id, data, length);
}

void group_invite_cb(Tox*, uint32_t friend_number, const uint8_t *invite_data, size_t length, const uint8_t *group_name, size_t group_name_length, void *user_data) {
	static_cast<ToxClient*>(user_data)->onToxGroupInvite(friend_number, invite_data, length, std::string_view{reinterpret_cast<const char*>(group_name), group_name_length});
}

void group_peer_join_cb(Tox*, uint32_t group_number, uint32_t peer_id, void *user_data) {
	static_cast<ToxClient*>(user_data)->onToxGroupPeerJoin(group_number, peer_id);
}

void group_peer_exit_cb(Tox*, uint32_t group_number, uint32_t peer_id, Tox_Group_Exit_Type exit_type, const uint8_t *name, size_t name_length, const uint8_t *part_message, size_t part_message_length, void *user_data) {
	static_cast<ToxClient*>(user_data)->onToxGroupPeerExit(group_number, peer_id, exit_type, std::string_view{reinterpret_cast<const char*>(name), name_length}, std::string_view{reinterpret_cast<const char*>(part_message), part_message_length});
}

void group_self_join_cb(Tox*, uint32_t group_number, void *user_data) {
	static_cast<ToxClient*>(user_data)->onToxGroupSelfJoin(group_number);
}

// ==================== sha1_info ====================

void ft1_recv_request_sha1_info_cb(Tox*, uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size, void* user_data) {
	static_cast<ToxClient*>(user_data)->getState().onFT1ReceiveRequestSHA1Info(group_number, peer_number, file_id, file_id_size);
}

bool ft1_recv_init_sha1_info_cb(Tox*, uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size, const uint8_t transfer_id, const size_t file_size, void* user_data) {
	return static_cast<ToxClient*>(user_data)->getState().onFT1ReceiveInitSHA1Info(group_number, peer_number, file_id, file_id_size, transfer_id, file_size);
}

void ft1_recv_data_sha1_info_cb(Tox*, uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, const uint8_t* data, size_t data_size, void* user_data) {
	static_cast<ToxClient*>(user_data)->getState().onFT1ReceiveDataSHA1Info(group_number, peer_number, transfer_id, data_offset, data, data_size);
}

void ft1_send_data_sha1_info_cb(Tox*, uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, uint8_t* data, size_t data_size, void* user_data) {
	static_cast<ToxClient*>(user_data)->getState().onFT1SendDataSHA1Info(group_number, peer_number, transfer_id, data_offset, data, data_size);
}

// ==================== sha1_chunk ====================

void ft1_recv_request_sha1_chunk_cb(Tox*, uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size, void* user_data) {
	static_cast<ToxClient*>(user_data)->getState().onFT1ReceiveRequestSHA1Chunk(group_number, peer_number, file_id, file_id_size);
}

bool ft1_recv_init_sha1_chunk_cb(Tox*, uint32_t group_number, uint32_t peer_number, const uint8_t* file_id, size_t file_id_size, const uint8_t transfer_id, const size_t file_size, void* user_data) {
	return static_cast<ToxClient*>(user_data)->getState().onFT1ReceiveInitSHA1Chunk(group_number, peer_number, file_id, file_id_size, transfer_id, file_size);
}

void ft1_recv_data_sha1_chunk_cb(Tox*, uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, const uint8_t* data, size_t data_size, void* user_data) {
	static_cast<ToxClient*>(user_data)->getState().onFT1ReceiveDataSHA1Chunk(group_number, peer_number, transfer_id, data_offset, data, data_size);
}

void ft1_send_data_sha1_chunk_cb(Tox*, uint32_t group_number, uint32_t peer_number, uint8_t transfer_id, size_t data_offset, uint8_t* data, size_t data_size, void* user_data) {
	static_cast<ToxClient*>(user_data)->getState().onFT1SendDataSHA1Chunk(group_number, peer_number, transfer_id, data_offset, data, data_size);
}

